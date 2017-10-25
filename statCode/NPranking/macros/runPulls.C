// Author      : Stefan Gadatsch
// Email       : gadatsch@nikhef.nl
// Date        : 2013-04-24
// Description : Compute pulls and impact on the POI

#include <string>
#include <vector>

#include "TFile.h"
#include "TH1D.h"
#include "Math/MinimizerOptions.h"
#include "TStopwatch.h"

#include "RooWorkspace.h"
#include "RooStats/ModelConfig.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooRealVar.h"
#include "RooNLLVar.h"
#include "RooFitResult.h"

#include "macros/log.C"
#include "macros/parseString.C"
#include "macros/minimize.C"
#include "macros/findSigma.C"

using namespace std;
using namespace RooFit;
using namespace RooStats;

// ____________________________________________________________________________|__________
//void runPulls() {
//    // for compiling only
//}

// ____________________________________________________________________________|__________
// compute pulls of the nuisance parameters and store them in text files. norm and syst parameters will be split among different files
void runPulls(const char* inFileName = "workspace.root",
    const char* poiName = "SigXsecOverSM",
    const char* wsName = "combined",
    const char* modelConfigName = "ModelConfig",
    const char* dataName = "obsData",
    const char* folder = "test",
    const char* variable = NULL,
    double precision = 0.005,
    bool useMinos = 0,
    string loglevel = "DEBUG")
{
	cout<<"start doing "<<endl;

    //TStopwatch timer_pulls;
    //timer_pulls.Start();

    // DEBUG OUTPUT
    // - ERROR
    // - WARNING
    // - INFO
    // - DEBUG

	LOG::FromString(loglevel);

	cout<<"start doing 1"<<endl;

    //LOG::ReportingLevel() = LOG::FromString(loglevel);

	cout<<"start doing 2"<<endl;

    // some settings
    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);
    if (loglevel == "DEBUG") {
        ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);
    } else {
        ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(-1);
        RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
    }

        cout<<"start doing 3"<<endl;


    // loading the workspace etc.
    LOG(logINFO) << "Running over workspace: " << inFileName;
        cout<<"start doing 3.1"<<endl;
    system(("mkdir -vp root-files/" + string(folder) + "_pulls").c_str());
    vector<string> parsed = parseString(poiName, ",");

    TFile* file = new TFile(inFileName);

    RooWorkspace* ws = (RooWorkspace*)file->Get(wsName);
    if (!ws) {
        LOG(logERROR) << "Workspace: " << wsName << " doesn't exist!";
        exit(1);
    }

    ModelConfig* mc = (ModelConfig*)ws->obj(modelConfigName);
    if (!mc) {
        LOG(logERROR) << "ModelConfig: " << modelConfigName << " doesn't exist!";
        exit(1);
    }

    RooDataSet* data = (RooDataSet*)ws->data(dataName);
    if (!data) {
        LOG(logERROR) << "Dataset: " << dataName << " doesn't exist!";
        exit(1);
    }

        cout<<"start doing 4"<<endl;

    vector<RooRealVar*> pois;
    for (int i = 0; i < parsed.size(); i++) {
        RooRealVar* poi = (RooRealVar*)ws->var(parsed[i].c_str());
        LOG(logINFO) << "Getting POI " << poi->GetName();
        if (!poi) {
            LOG(logERROR) << "POI: " << poi->GetName() << " doesn't exist!";
            exit(1);
        }
        poi->setVal(1);
        poi->setRange(-5.,5.);
        poi->setConstant(0);
        pois.push_back(poi);
    }
        cout<<"start doing 5"<<endl;

    RooArgSet* nuis = (RooArgSet*)mc->GetNuisanceParameters();
    if (!nuis) {
        LOG(logERROR) << "Nuisance parameter set doesn't exist!";
        exit(1);
    }
    TIterator* itr = nuis->createIterator();
    RooRealVar* var;

    RooArgSet* globs = (RooArgSet*)mc->GetGlobalObservables();
    if (!globs) {
        LOG(logERROR) << "GetGlobal observables don't exist!";
        exit(1);
    }
        cout<<"start doing 6"<<endl;

    // collect nuisance parameters
    vector<string> vec_nuis;

    while ((var = (RooRealVar*)itr->Next())) {
        string varName(var->GetName());

        // skipping MC stat, lvlv pacman efficiencies
        if (varName.find("gamma_stat") != string::npos && varName.find("alpha") == string::npos) {
            LOG(logDEBUG) << "Skipping " << varName;
            continue;
        }

        // all remaining nuisance parameters
        LOG(logDEBUG) << "pushed back " << varName;
        vec_nuis.push_back(string(var->GetName()));
    }

    itr->Reset();
    int nrNuis = vec_nuis.size();

        cout<<"start doing 7"<<endl;

    // create nll and do unconditional fit
    // ws->loadSnapshot("ucmles");
    for (int i = 0; i < pois.size(); i++) {
    	pois[i]->setConstant(0);
        pois[i]->setRange(-5., 5.);
    }

    //int numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    int numCPU = 1;
    if (useMinos) numCPU = 1; // Minos doesn't like our NLL with multiple cores
    RooNLLVar* nll = (RooNLLVar*)mc->GetPdf()->createNLL(*data, Constrain(*nuis), GlobalObservables(*globs), Offset(1), NumCPU(numCPU, RooFit::Hybrid));

    RooFitResult* fitresult; minimize(nll);
    double nll_hat = nll->getVal();
    vector<double> pois_hat;
    for (int i = 0; i < pois.size(); i++) {
        pois_hat.push_back(pois[i]->getVal());
    }

    // set all nuisance parameters floating and save snapshot
    while ((var = (RooRealVar*)itr->Next())) {
        var->setConstant(0);
    }
    itr->Reset();
    for (int i = 0; i < pois.size(); i++) {
        pois[i]->setConstant(0);
    }
    ws->saveSnapshot("tmp_snapshot", *mc->GetPdf()->getParameters(data));

        cout<<"start doing 8"<<endl;

    LOG(logDEBUG) << "Made unconditional snapshot";

    for (int in = 0; in < nrNuis; in++) {

        ws->loadSnapshot("tmp_snapshot");

        RooRealVar* nuip = (RooRealVar*)nuis->find(vec_nuis[in].c_str());
        string nuipName(nuip->GetName());

	cout<<" doing nuip name "<< nuipName <<endl;
        if (variable != NULL && nuipName != string(variable)) continue;

        // find all unconstrained NFs etc.
        bool isNorm = 0;
        // if (nuipName.find("ATLAS_norm") != string::npos) isNorm = 1;
        // if (nuipName.find("atlas_nbkg_") != string::npos) isNorm = true;
        // if (nuipName.find("slope_") != string::npos) isNorm = true;
        // if (nuipName.find("p0_") != string::npos) isNorm = true;
        // if (nuipName.find("p1_") != string::npos) isNorm = true;
        // if (nuipName.find("p2_") != string::npos) isNorm = true;
        // if (nuipName.find("p3_") != string::npos) isNorm = true;
        // if (nuipName.find("ATLAS_Hbb_norm_") != string::npos) isNorm = true;
        // if (nuipName.find("ATLAS_PM_EFF_") != string::npos) isNorm = true;
        if (nuipName.find("scale_") != string::npos && nuipName.find("QCDscale_") == string::npos) isNorm = true;

        // setting all parameters but the one NF to look at constant at the best fit value when computing the error for the NF
        if (isNorm) {
            LOG(logDEBUG) << nuipName << " is unconstrained.";
            while ((var = (RooRealVar*)itr->Next())) {
                string varName(var->GetName());
                if (varName.find(nuipName) == string::npos) var->setConstant(1);
                LOG(logDEBUG) << varName << " is constant -> " << var->isConstant();
            }
            itr->Reset();
        }

        double nuip_hat = nuip->getVal();
        nuip->setConstant(0);

        ws->saveSnapshot("tmp_snapshot2", *mc->GetPdf()->getParameters(data));

        LOG(logDEBUG) << "Computing error for var " << nuip->GetName() << " at " << nuip->getVal();

        double nuip_errup;
        double nuip_errdown;

        if (useMinos) {
            RooArgSet* minosSet = new RooArgSet(*nuip);
            minimize(nll);
            nuip_errup = nuip->getErrorHi();
            nuip_errdown = nuip->getErrorLo();
        } else {
            ws->loadSnapshot("tmp_snapshot2");
            nuip_errup = findSigma(nll, nll_hat, nuip, nuip_hat+fabs(nuip->getErrorHi()), nuip_hat, +1, precision);
            ws->loadSnapshot("tmp_snapshot2");
            nuip_errdown = findSigma(nll, nll_hat, nuip, nuip_hat-fabs(nuip->getErrorLo()), nuip_hat, -1, precision);
        }

        LOG(logINFO) << nuip->GetName() << " = " << nuip_hat << " +" << fabs(nuip_errup) << " /  -" << fabs(nuip_errdown);

        // fix theta at the MLE value +/- postfit uncertainty and minimize again to estimate the change in the POI
        ws->loadSnapshot("tmp_snapshot2");
        nuip->setVal(nuip_hat+fabs(nuip_errup));
        nuip->setConstant(1);
        minimize(nll);
        vector<double> pois_up;
        for (int i = 0; i < pois.size(); i++) {
            pois_up.push_back(pois[i]->getVal());
        }

        ws->loadSnapshot("tmp_snapshot2");
        nuip->setVal(nuip_hat-fabs(nuip_errdown));
        nuip->setConstant(1);
        minimize(nll);
        vector<double> pois_down;
        for (int i = 0; i < pois.size(); i++) {
            pois_down.push_back(pois[i]->getVal());
        }

        // fix theta at the MLE value +/- postfit uncertainty and minimize again to estimate the change in the POI
        ws->loadSnapshot("tmp_snapshot2");
        nuip->setVal(nuip_hat+1.0);
        nuip->setConstant(1);
        minimize(nll);
        vector<double> pois_nom_up;
        for (int i = 0; i < pois.size(); i++) {
            pois_nom_up.push_back(pois[i]->getVal());
        }

        ws->loadSnapshot("tmp_snapshot2");
        nuip->setVal(nuip_hat-1.0);
        nuip->setConstant(1);
        minimize(nll);
        vector<double> pois_nom_down;
        for (int i = 0; i < pois.size(); i++) {
            pois_nom_down.push_back(pois[i]->getVal());
        }

        for (int i = 0; i < pois.size(); i++) {
            LOG(logINFO) << "Variation of " << pois[i]->GetName() << " = " << pois_up[i] << " (" << pois_nom_up[i] << ") / " << pois_down[i] << " (" << pois_nom_down[i] << ")";
        }

        // store result in root file
        stringstream fileName;
        fileName << "root-files/" << folder << "_pulls/" << nuipName << ".root";
        TFile fout(fileName.str().c_str(), "recreate");

        TH1D* h_out = new TH1D(nuipName.c_str(), nuipName.c_str(), 3 + 5 * pois.size(), 0, 3 + 5 * pois.size());

        h_out->SetBinContent(1, nuip_hat);
        h_out->SetBinContent(2, fabs(nuip_errup));
        h_out->SetBinContent(3, fabs(nuip_errdown));

        h_out->GetXaxis()->SetBinLabel(1, "nuip_hat");
        h_out->GetXaxis()->SetBinLabel(2, "nuip_up");
        h_out->GetXaxis()->SetBinLabel(3, "nuip_down");

        int bin = 4;
        for (int i = 0; i < pois.size(); i++) {
            h_out->SetBinContent(bin, pois_hat[i]);
            h_out->SetBinContent(bin+1, pois_up[i]);
            h_out->SetBinContent(bin+2, pois_down[i]);
            h_out->SetBinContent(bin+3, pois_nom_up[i]);
            h_out->SetBinContent(bin+4, pois_nom_down[i]);

            h_out->GetXaxis()->SetBinLabel(bin, pois[i]->GetName());
            h_out->GetXaxis()->SetBinLabel(bin+1, "poi_up");
            h_out->GetXaxis()->SetBinLabel(bin+2, "poi_down");
            h_out->GetXaxis()->SetBinLabel(bin+3, "poi_nom_up");
            h_out->GetXaxis()->SetBinLabel(bin+4, "poi_nom_down");

            bin += 5;
        }

        fout.Write();
        fout.Close();
    }

    //timer_pulls.Stop();
    //timer_pulls.Print();
}
