// Author      : Stefan Gadatsch
// Email       : gadatsch@nikhef.nl
// Date        : 2013-04-26
// Description : Compute uncertainty due to different groups of parameters specified in a XML

#include <string>
#include <vector>

#include "TFile.h"
#include "TH1D.h"
#include "Math/MinimizerOptions.h"
#include "TStopwatch.h"
#include "TXMLEngine.h"

#include "RooWorkspace.h"
#include "RooStats/ModelConfig.h"
#include "RooDataSet.h"
#include "RooArgSet.h"
#include "RooRealVar.h"
#include "RooNLLVar.h"
#include "RooFitResult.h"

#include "log.C"
#include "parseString.C"
#include "minimize.C"
#include "findSigma.C"

using namespace std;
using namespace RooFit;
using namespace RooStats;

struct settings {
    string inFileName;
    string wsName;
    string modelConfigName;
    string dataName;
    string poiName;
    string xmlName;
    string technique;
    string catecory2eval;
    double precision;
    double corrCutoff;
    bool useMinos;
    string folder;
    string loglevel;
};

void setParams(RooArgSet* nuis, list<string> tmp_list, string technique, RooFitResult* fitresult, RooRealVar* poi, double corrCutoff);
list<string> addParams(settings* config, string catecory2eval);
void writeTmpXML (string systName, settings* config);

// ____________________________________________________________________________|__________
//void runBreakdown() {
    // for compiling only
//}

// ____________________________________________________________________________|__________
// Compute ranking of systematics specified in xml
void runBreakdown(string inFileName = "/afs/cern.ch/user/g/gfacini/public/0625Workspaces/workspaces/12.TT.10_otT_0/combined/125.root",//"workspace.root",
    string wsName          = "combined",
    string modelConfigName = "ModelConfig",
    string dataName        = "obsData",
    string poiName         = "SigXsecOverSM",
    string xmlName         = "config/breakdown.xml",
    string technique       = "add",
    string catecory2eval   = "total",
    double precision       = 0.005,
    double corrCutoff      = 0.0,
    string folder          = "12.TT.10_otT_0",
    string loglevel        = "DEBUG")
{
    //TStopwatch timer_breakdown;
    //timer_breakdown.Start();
  std::cout<<"$$ a"<<std::endl;
    // store all settings for passing to other functions
    settings* config = new settings();
    config->inFileName = inFileName;
    config->wsName = wsName;
    config->modelConfigName = modelConfigName;
    config->dataName = dataName;
    config->poiName = poiName;
    config->xmlName = xmlName;
    config->technique = technique;
    config->catecory2eval = catecory2eval;
    config->precision = precision;
    config->corrCutoff = corrCutoff;
    config->folder = folder;
    config->loglevel = loglevel;

    std::cout<<"$$ b"<<std::endl;
    // DEBUG OUTPUT
    // - ERROR
    // - WARNING
    // - INFO
    // - DEBUG
    //LOG::ReportingLevel() = LOG::FromString(config->loglevel);

    // some settings
    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(1);
    if (config->loglevel == "DEBUG") {
        ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(1);
    } else {
        ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(-1);
        RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);
    }

    std::cout<<"$$ c"<<std::endl;

    // loading the workspace etc.
    LOG(logINFO) << "Running over workspace: " << config->inFileName;

    vector<string> parsed = parseString(config->poiName, ",");

    TFile* file = new TFile(config->inFileName.c_str());

    std::cout<<"$$ d"<<std::endl;  
    RooWorkspace* ws = (RooWorkspace*)file->Get(config->wsName.c_str());
    if (!ws) {
        LOG(logERROR) << "Workspace: " << config->wsName << " doesn't exist!";
        exit(1);
    }
  
    std::cout<<"$$ e"<<std::endl;
    ModelConfig* mc = (ModelConfig*)ws->obj(config->modelConfigName.c_str());
    if (!mc) {
        LOG(logERROR) << "ModelConfig: " << config->modelConfigName << " doesn't exist!";
        exit(1);
    }
    std::cout<<"$$ f"<<std::endl;
  
    RooDataSet* data = (RooDataSet*)ws->data(config->dataName.c_str());
    if (!data) {
        LOG(logERROR) << "Dataset: " << config->dataName << " doesn't exist!";
        exit(1);
    }
    std::cout<<"$$ g"<<std::endl;
    vector<RooRealVar*> pois;
    for (int i = 0; i < parsed.size(); i++) {
        RooRealVar* poi = (RooRealVar*)ws->var(parsed[i].c_str());
        if (!poi) {
            LOG(logERROR) << "POI: " << parsed[i] << " doesn't exist!";
            exit(1);
        }
        poi->setVal(1);
        poi->setRange(-10.,10.);
        poi->setConstant(1);
        pois.push_back(poi);
    }
    std::cout<<"$$ h"<<std::endl;    

    RooArgSet* nuis = (RooArgSet*)mc->GetNuisanceParameters();
    if (!nuis) {
        LOG(logERROR) << "Nuisance parameter set doesn't exist!";
        exit(1);
    }
    TIterator* nitr = nuis->createIterator();
    RooRealVar* var;
    
    std::cout<<"$$ i"<<std::endl;
    RooArgSet* globs = (RooArgSet*)mc->GetGlobalObservables();
    if (!globs) {
        LOG(logERROR) << "GetGlobal observables don't exist!";
        exit(1);
    }

    std::cout<<"$$ j"<<std::endl;
    ws->loadSnapshot("nominalNuis");
    for (int i = 0; i < pois.size(); i++) {
        pois[i]->setRange(-10., 10.);
        pois[i]->setConstant(0);
        pois[i]->setVal(1.1); // Kick !
    }

    std::cout<<"$$ k"<<std::endl;
    //int numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    int numCPU = 4; 
    RooNLLVar* nll = (RooNLLVar*)mc->GetPdf()->createNLL(*data, Constrain(*nuis), GlobalObservables(*globs), Offset(1), NumCPU(numCPU, RooFit::Hybrid), Optimize(2));

    RooFitResult* fitresult; minimize(nll);
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(1);

    RooArgSet nuisAndPOI(*mc->GetNuisanceParameters(), *mc->GetParametersOfInterest());
    ws->saveSnapshot("tmp_shot", nuisAndPOI);

    std::cout<<"$$ l"<<std::endl;

    double nll_val_true = nll->getVal();
    vector<double> pois_hat;
    for (int i = 0; i < pois.size(); i++) {
        pois_hat.push_back(pois[i]->getVal());
    }

    vector<double> pois_up;
    vector<double> pois_down;

    std::cout<<"$$ m"<<std::endl;
    if (config->catecory2eval == "total") {
        for (int i = 0; i < pois.size(); i++) {
            ws->loadSnapshot("tmp_shot");
            pois_up.push_back(findSigma(nll, nll_val_true, pois[i], pois_hat[i]+pois[i]->getErrorHi(), pois_hat[i], +1, config->precision));
            ws->loadSnapshot("tmp_shot");
            pois_down.push_back(findSigma(nll, nll_val_true, pois[i], pois_hat[i]-fabs(pois[i]->getErrorLo()), pois_hat[i], -1, config->precision));
        }
    } else if (config->catecory2eval == "srstat") {
        // setting everything constant, so that just data statistics is left
        ws->loadSnapshot("tmp_shot");
        nitr->Reset();
        while ((var = (RooRealVar*)nitr->Next())) {
            var->setConstant(1);
        }
        ws->saveSnapshot("tmp_shot_srstat",nuisAndPOI);

        for (int i = 0; i < pois.size(); i++) {
            ws->loadSnapshot("tmp_shot_srstat");
            pois_up.push_back(findSigma(nll, nll_val_true, pois[i], pois_hat[i]+pois[i]->getErrorHi(), pois_hat[i], +1, config->precision));
            ws->loadSnapshot("tmp_shot_srstat");
            pois_down.push_back(findSigma(nll, nll_val_true, pois[i], pois_hat[i]-fabs(pois[i]->getErrorLo()), pois_hat[i], -1, config->precision));
        }
    } else if (config->catecory2eval == "mcstat") {
        for (int i = 0; i < pois.size(); i++) {
            ws->loadSnapshot("tmp_shot");
            list<string> stat_list = addParams(config, "statistical");
            setParams(nuis, stat_list, config->technique, fitresult, pois[i], config->corrCutoff);
            nitr->Reset();
            while ((var = (RooRealVar*)nitr->Next())) {
                if (string(var->GetName()).find("gamma_stat") != string::npos) {
                    var->setConstant(0);
                }
            }
            ws->saveSnapshot("tmp_shot_mcstat",nuisAndPOI);

            ws->loadSnapshot("tmp_shot_mcstat");
            pois_up.push_back(findSigma(nll, nll_val_true, pois[i], pois_hat[i]+pois[i]->getErrorHi(), pois_hat[i], +1, config->precision));
            ws->loadSnapshot("tmp_shot_mcstat");
            pois_down.push_back(findSigma(nll, nll_val_true, pois[i], pois_hat[i]-fabs(pois[i]->getErrorLo()), pois_hat[i], -1, config->precision));
        }
    } else {
        list<string> nuis_list = addParams(config, config->catecory2eval);

        for (int i = 0; i < pois.size(); i++) {
            ws->loadSnapshot("tmp_shot");
            setParams(nuis, nuis_list, config->technique, fitresult, pois[i], config->corrCutoff);
            pois_up.push_back(findSigma(nll, nll_val_true, pois[i], pois_hat[i]+pois[i]->getErrorHi(), pois_hat[i], +1, config->precision));
            ws->loadSnapshot("tmp_shot");
            setParams(nuis, nuis_list, config->technique, fitresult, pois[i], config->corrCutoff);
            pois_down.push_back(findSigma(nll, nll_val_true, pois[i], pois_hat[i]-fabs(pois[i]->getErrorLo()), pois_hat[i], -1, config->precision));
        }
    }

    std::cout<<"$$ n"<<std::endl;

    for (int i = 0; i < pois.size(); i++) {
        LOG(logINFO) << config->catecory2eval << " gives " << pois[i]->GetName() << " = " << pois_hat[i] << " +" << pois_up[i] << " / -" << pois_down[i];
    }

    system(("mkdir -vp root-files/" + string(config->folder) + "_breakdown_" + string(technique)).c_str());
    stringstream fileName;
    fileName << "root-files/" << config->folder << "_breakdown_" << technique << "/" << config->catecory2eval << ".root";
    TFile fout(fileName.str().c_str(), "recreate");

    TH1D* h_out = new TH1D(config->catecory2eval.c_str(), config->catecory2eval.c_str(), 3 * pois.size(), 0, 3 * pois.size());

    int bin = 1;
    for (int i = 0; i < pois.size(); i++) {
        h_out->SetBinContent(bin, pois_hat[i]);
        h_out->SetBinContent(bin+1, fabs(pois_up[i]));
        h_out->SetBinContent(bin+2, fabs(pois_down[i]));

        h_out->GetXaxis()->SetBinLabel(bin, pois[i]->GetName());
        h_out->GetXaxis()->SetBinLabel(bin+1, "poi_up");
        h_out->GetXaxis()->SetBinLabel(bin+2, "poi_down");
        bin += 3;
    }

    std::cout<<"$$ o"<<std::endl;

    fout.Write();
    fout.Close();

    //timer_breakdown.Stop();
    //timer_breakdown.Print();
}

// ____________________________________________________________________________|__________
// Set parameters constant or floating, depending on technique
void setParams(RooArgSet* nuis, list<string> tmp_list, string technique, RooFitResult* fitresult, RooRealVar* poi, double corrCutoff) {
    RooRealVar* var;
    TIterator* nitr = nuis->createIterator();

    while ((var = (RooRealVar*)nitr->Next())) {
        string varName = string(var->GetName());

        if (technique.find("sub") != string::npos) {
            var->setConstant(0);
        } else {
            var->setConstant(1);
        }

        bool found = 0;
        if (find(tmp_list.begin(), tmp_list.end(), varName) != tmp_list.end()) {
            LOG(logDEBUG) << "Found " << varName;
            found = 1;
        }

        if (found) {
            if (technique.find("sub") != string::npos) {
                var->setConstant(1);
            } else {
                var->setConstant(0);
            }
        }

        LOG(logDEBUG) << varName << " is constant -> " << var->isConstant();

        double correlation = fitresult->correlation(varName.c_str(), poi->GetName());

        LOG(logDEBUG) << "Correlation between poi and " << varName << " is " << correlation;

        if (abs(correlation) < corrCutoff) {
            LOG(logWARNING) << "Setting " << varName << " constant because it's not correlated to the POI.";
            var->setConstant(1);
        }
    }
}

// ____________________________________________________________________________|__________
// Add parameters to a list of nuisance parameters
list<string> addParams(settings* config, string catecory2eval) {
    list<string> tmp_list;

    TXMLEngine* xml = new TXMLEngine;
    XMLDocPointer_t xmldoc = xml->ParseFile(config->xmlName.c_str());
    if (!xmldoc) {
        LOG(logERROR) << "XML " << config->xmlName << " doesn't exist!";
        exit(1);
    }

    XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);
    XMLNodePointer_t category = xml->GetChild(mainnode);

    while (category != 0) {
        string categoryName = xml->GetNodeName(category);

        if (categoryName.find(catecory2eval) == string::npos) {
            LOG(logINFO) << "skipping " << categoryName;
            category = xml->GetNext(category);
        } else {
            bool breakdown = (string(xml->GetAttr(category, "breakdown")).find("yes") != string::npos) ? 1 : 0;

            XMLNodePointer_t systematic = xml->GetChild(category);
        
            if (config->technique.find("add") != string::npos && catecory2eval.find("statistical") == string::npos) {
                LOG(logINFO) << "Adding statistical parameters";
                tmp_list = addParams(config, "statistical");
            }

            while (systematic != 0) {
                XMLAttrPointer_t attr_syst = xml->GetFirstAttr(systematic);    
                while (attr_syst != 0) {
                    string systName = xml->GetAttrValue(attr_syst);

                    tmp_list.push_back(systName);
                    
                    if (breakdown) {
                        LOG(logINFO) << "Doing breakdown: " << systName;
                        writeTmpXML(systName, config);
                        runBreakdown(config->inFileName, config->wsName, config->modelConfigName, config->dataName, config->poiName, "config/tmp_"+systName+".xml", config->technique, systName, config->precision, config->corrCutoff, config->folder, config->loglevel);
                    }
                    attr_syst = xml->GetNextAttr(attr_syst);  
                }
                systematic = xml->GetNext(systematic);
            }
            category = xml->GetNext(category);
        }
    }
    return tmp_list;
}

// ___________________________________________________________________________|__________
// Write temporary XML for a single parameter
void writeTmpXML (string systName, settings* config) {
    // add the interesting category
    TXMLEngine* xml = new TXMLEngine;

    XMLNodePointer_t mainnode = xml->NewChild(0, 0, "breakdown");
    XMLAttrPointer_t description_main = xml->NewAttr(mainnode, 0, "description", "map of tmp uncertainties");

    XMLNodePointer_t child = xml->NewChild(mainnode, 0, systName.c_str());
    XMLAttrPointer_t description_child = xml->NewAttr(child, 0, "description", systName.c_str());
    XMLAttrPointer_t breakdown_child = xml->NewAttr(child, 0, "breakdown", "no");
    XMLAttrPointer_t scan_child = xml->NewAttr(child, 0, "scan", "no");

    XMLNodePointer_t subchild = xml->NewChild(child, 0, "systematic");
    XMLAttrPointer_t name_subchild = xml->NewAttr(subchild, 0, "name", systName.c_str());
 
    // add the statistical parameters as defined in top level xml
    XMLNodePointer_t child_stat = xml->NewChild(mainnode, 0, "statistical");
    XMLAttrPointer_t description_child_stat = xml->NewAttr(child_stat, 0, "description", "statistical uncertainties");
    XMLAttrPointer_t breakdown_child_stat = xml->NewAttr(child_stat, 0, "breakdown", "no");
    XMLAttrPointer_t scan_child_stat = xml->NewAttr(child_stat, 0, "scan", "no");

    TXMLEngine* xml_top = new TXMLEngine;
    XMLDocPointer_t xmldoc_top = xml_top->ParseFile(config->xmlName.c_str());

    XMLNodePointer_t mainnode_top = xml_top->DocGetRootElement(xmldoc_top);
    XMLNodePointer_t category_top = xml_top->GetChild(mainnode_top);

    while (category_top != 0) {
        string categoryName = xml_top->GetNodeName(category_top);

        if (categoryName.find("statistical") != string::npos) {
            xml->AddChild(child_stat, xml->GetChild(category_top));
        }
        category_top = xml_top->GetNext(category_top);
    }

    // save the new tmp xml
    XMLDocPointer_t xmldoc = xml->NewDoc();
    xml->DocSetRootElement(xmldoc, mainnode);
   
    xml->SaveDoc(xmldoc, ("config/tmp_"+systName+".xml").c_str());
    
    xml->FreeDoc(xmldoc);
    delete xml;
}
