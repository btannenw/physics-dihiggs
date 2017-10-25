/*
Author: Romain Madar & Gabriel Facini
Date:   2012-02-16
Email:  romain.madar@cern.ch, gabriel.facini@cern.ch


Description : This code allows the check quality of fit performed in the limit derivation.
It works on a generic workspace produced by hist2workspace command. It performs
a global fit and a fit per subchannel automatically. Various control plots (pull
distribution, correlation matrix, distribution before and after fit, ...) are
stored in a the rootfile FitCrossChecks.root.


Updates:

- 2012-09-20 G. Facini
 * Get Histograms to plot systematic shapes
 * Add plotRelative flag to plot the relative shape difference for a given systematic
 * Add drawPlots flag to make eps files
 * Toys (still in developpement)
 * components post-fit

 - 2012-10 R. Madar
 * Post-fit NP versus subchannel
 * Morphing control plots for each syst x process x subchannel
 * Add the stack of different backgrounds in plot after profiling

 - 2012-11 R. Madar
 * Add the asymmetric error given by minos for the NPs
 * Add the -2Log(L) versus each NP
 * Add a red color for/summary of suspicious NPs

*/


// C++
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>

// Root
#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TList.h"
#include "TMath.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TGaxis.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLeaf.h"
#include "TMarker.h"
#include "TDecompLU.h"
#include "TPRegexp.h"
#include "TObjArray.h"
// RooFit
#include "RooWorkspace.h"
#include "RooRealVar.h"
#include "RooPlot.h"
#include "RooAbsData.h"
#include "RooHist.h"
#include "RooSimultaneous.h"
#include "RooCategory.h"
#include "RooFitResult.h"
#include "RooAbsPdf.h"
#include "RooExtendPdf.h"
#include "RooRealSumPdf.h"
#include "Roo1DTable.h"
#include "RooConstVar.h"
#include "RooProduct.h"
#include "RooRandom.h"
#include "TStopwatch.h"
#include "RooNLLVar.h"
#include "RooMsgService.h"
#include "TMatrixDSymEigen.h"

// RooStat
#include "RooStats/ModelConfig.h"
#include "RooStats/ProfileInspector.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/SamplingDistribution.h"
#include "RooStats/SamplingDistPlot.h"
#include "RooStats/ToyMCSampler.h"
#include "RooStats/RooStatsUtils.h"
#include "RooStats/MinNLLTestStat.h"
#include "RooStats/AsymptoticCalculator.h"

using namespace std;
using namespace RooFit;
using namespace RooStats;

struct NPContainer{
  TString NPname;
  double  NPvalue;
  double  NPerrorHi;
  double  NPerrorLo;
  TString WhichFit;
};

static bool comp_second_abs_decend( const pair< RooRealVar*, float >& i, const pair< RooRealVar*, float >& j ) {
  return fabs(i.second) > fabs(j.second);
}


namespace LimitCrossCheck{

  // Global variables;
  // User configuration one
  bool drawPlots(false);                    // create eps & png files and creat a webpage
  bool plotRelative(false);                 // plot % shift of systematic
  bool draw1DResponse(false);		            // draw 1D response for each NP
  bool UseMinosError(false);                // compute minos error (if false : use minuit error)
  bool blind(false);                        // blind di-jet mass from 100-150 GeV
  bool makePostFitPlots(false);             // takes a lot of time and the error band is wrong
  double PullMaxAcceptable(1.5);            // Threshold to consider a NP[central value] as suspicious
  double ErrorMinAcceptable(0.2);           // Threshold to consider a NP[error] as suspicious
  TString xAxisLabel("Final Distribution"); // set what the x-axis of the distribution is
  int nJobs(1);                             // number of subjobs for parallel processing
  int iJob(0);                              // index of subjob: i = 0..n-1

  // not switches
  RooWorkspace *w         ;
  ModelConfig  *mc        ;
  RooAbsData   *data      ;
  TFile        *outputfile;
  double        LumiRelError;
  TDirectory   *MainDirSyst;
  TDirectory   *MainDirMorphing;
  TDirectory   *MainDirFitEachSubChannel;
  TDirectory   *MainDirFitGlobal;
  TDirectory   *MainDirModelInspector;
  TDirectory   *MainDirStatTest;
  TDirectory   *MainDirFitAsimov;
  map <string,double> MapNuisanceParamNom;
  vector<NPContainer> AllNPafterEachFit;
  TString OutputDir;

  //Global functions
  RooFitResult* FitPDF( ModelConfig* model, RooAbsPdf* fitpdf, RooAbsData* fitdata, TString minimType = "Minuit2" );
  void     PlotHistosBeforeFit(double nSigmaToVary, double mu);
  void     PlotMorphingControlPlots();
  void     PlotHistosAfterFitEachSubChannel(bool IsConditionnal , double mu);
  void     PlotHistosAfterFitGlobal(bool IsConditionnal , double mu, bool isAsimov=false);
  void     PlotsNuisanceParametersVSmu();
  void     PlotsStatisticalTest(double mu_pe, double mu_hyp, int nToyMC = 100, int rndmSeed = 0);
  void 	   Plot1DResponse(RooAbsReal* nll, RooRealVar* var, TString cname, TCanvas* can, TF1* poly, bool IsFloating, TLatex* latex, TDirectory* tdir, RooArgSet* SliceSet = 0);
  void 	   Plot1DResponseNew(RooAbsReal* nll, RooRealVar* var, TString cname, TCanvas* can, bool IsFloating, TLatex* latex, TDirectory* tdir, TString snapshotName);
  TTree*   createObservableTree(ModelConfig* model);
  void     setObservableTreeValues(ModelConfig* model, TTree* tree);
  double   FindMuUpperLimit();
  void     PrintModelObservables();
  void     PrintNuisanceParameters();
  void     PrintAllParametersAndValues(RooArgSet para);
  void     PrintNumberOfEvents(RooAbsPdf *pdf);
  void     FindConstants(RooAbsPdf *pdf);
  void     PrintSubChannels();
  void     PrintSuspiciousNPs();
  bool     IsSimultaneousPdfOK();
  bool     IsChannelNameOK();
  void     SetAllNuisanceParaToSigma(double Nsigma);
  void     SetAllStatErrorToSigma(double Nsigma);
  void     SetNuisanceParaToSigma(RooRealVar *var, double Nsigma);
  void     GetNominalValueNuisancePara();
  void     SetNominalValueNuisancePara();
  void     SetPOI(double mu);
  void     SetStyle();
  void     LegendStyle(TLegend* l);
  int 	   GetPosition(RooRealVar* var, TH2D* corrMatrix);
  list< pair<RooRealVar*, float> > GetOrderedCorrelations(RooRealVar* var, RooFitResult* fitres);
  TCanvas* DrawShift(TString channel, TString var, TString comp, double mu, TH1* d, TH1* n, TH1* p1s, TH1* m1s);
  TH1F*    MakeHist(TString name, RooCurve *curve);
  TH1F*    ConvertGraphToHisto(TGraph *pGraph);
  TH2D*    GetSubsetOfCorrMatrix(RooRealVar* var, list< pair<RooRealVar*,float> >& pairs, RooFitResult* fitres, int size);
  void     FillGraphIntoHisto(TGraph *pGraph,TH1F *pHisto);
  void     Initialize(const char* infile , const char* outputdir, const char* workspaceName, const char* modelConfigName, const char* ObsDataName);
  void     Finalize();
  RooArgList getFloatParList( const RooAbsPdf& pdf, const RooArgSet& obsSet = RooArgSet() );


  //======================================================
  // ================= Main function =====================
  //======================================================
  void PlotFitCrossChecks(const char* infile          = "WorkspaceForTest1.root",
                          const char* outputdir       = "./results/",
                          const char* workspaceName   = "combined",
                          const char* modelConfigName = "ModelConfig",
                          const char* ObsDataName     = "obsData"){

    Initialize(infile, outputdir, workspaceName, modelConfigName, ObsDataName);

    // -----------------------------------------------------------------------------------
    // 1 - Plot nominal and +/- Nsigma (for each nuisance paramater) for Data, signal+bkg
    // -----------------------------------------------------------------------------------
    //PlotHistosBeforeFit(1.0,0.0); // (nSigma,mu)


    // -----------------------------------------------------------------------------------
    // 2 - Control plots for morphing (ie, -1/0/+1 sigma --> continuous NP)
    // -----------------------------------------------------------------------------------
    //PlotMorphingControlPlots();


    // ----------------------------------------------------------------------------------
    // 3 - Plot histograms after unconditional fit (theta and mu fitted at the same time)
    // ----------------------------------------------------------------------------------
    //bool IsConditional = false;
    //PlotHistosAfterFitEachSubChannel(IsConditional,0.0);
    //PlotHistosAfterFitGlobal(IsConditional,1.0);


    // --------------------------------------------------------------------------------------------
    // 4 - Plot the unconditionnal fitted nuisance paramters value (theta fitted while mu is fixed)
    // -------------------------------------------------------------------------------------------
    //IsConditional = true;
    //PlotHistosAfterFitEachSubChannel(IsConditional, 0.0);
    //PlotHistosAfterFitGlobal(IsConditional,0.0,true);
    //PlotHistosAfterFitGlobal(IsConditional,0.0,false);


    // -------------------------------------------
    // 5 - Plot the nuisance parameters versus mu
    // -------------------------------------------
    //PlotsNuisanceParametersVSmu(); // This can take time


    // -------------------------------------------
    // 6 - Plot the pulls and stat test from toys
    // -------------------------------------------
    PlotsStatisticalTest(0,0);

    // ----------------------------------------------------
    // 7 - Fit asimov dataset and plot histograms after fit
    // ----------------------------------------------------
    //PlotHistosAfterFitGlobal(true, 1.0, true); // conditionnal mu=1
    //PlotHistosAfterFitGlobal(false, 1.0, true); // unconditionnal mu=1
    //PlotHistosAfterFitGlobal(true, 0.0, true); // conditionnal mu=0
    //PlotHistosAfterFitGlobal(false, 0.0, true); // unconditionnal mu=0

    Finalize();
    return;

  }




  // ============================================================
  // ============ Definition of all the functions ===============
  // ============================================================







  // ============================================================
  // ============ Definition of Fitting Function ================
  // ============================================================


  RooFitResult* FitPDF( ModelConfig* model, RooAbsPdf* fitpdf, RooAbsData* fitdata, TString minimType ) {
    bool fancy(false);
    bool retryOnHesseFailure(false); // a bit less fancy

    /*
    FindConstants(fitpdf);
    cout << "The following parameters are fixed (constant)" << endl;
    TIterator *nItr = mc->GetNuisanceParameters()->createIterator();
    RooRealVar* arg = 0;
    while ((arg=(RooRealVar*)nItr->Next())) {
      if(!arg) { continue; }
      if(!arg->isConstant()) { continue; }
      cout << "\t" << arg->GetName() << endl;
    }
    */

    RooMsgService::instance().getStream(1).removeTopic(NumIntegration);

    model->Print();


    // let the black magic begin
    RooArgSet* constrainedParams = fitpdf->getParameters(*data);
    RemoveConstantParameters(constrainedParams);
    Constrain(*constrainedParams);

    const RooArgSet* glbObs = mc->GetGlobalObservables();

    // fix poi to zero
    RooRealVar * poi = (RooRealVar*) model->GetParametersOfInterest()->first();
    if(poi) {
      cout << "Constatnt POI ";
      if(poi->isConstant()) { cout << "YES" << endl; } else { cout << "NO" << endl; }
      cout << "Value of POI  " << poi->getVal() << endl;
    } else {
      cout << "No POI - assuming background only fit" << endl;
    }
    TString nCPU_str = getenv("NCORE");
    int nCPU = nCPU_str.Atoi();
    RooAbsReal * nll = fitpdf->createNLL(*fitdata, Constrain(*constrainedParams), GlobalObservables(*glbObs), Offset(1), NumCPU(nCPU,3), Optimize(2) );
    double nllval = nll->getVal();
    cout << "Starting NLL value " << nllval << endl;

    double nllnom(0);
    double nllup(0);
    double nlldn(0);

    cout << "Change to the LHood" << endl;
    TIterator *nItr = mc->GetNuisanceParameters()->createIterator();
    RooRealVar* arg = 0;
    while ((arg=(RooRealVar*)nItr->Next())) {
      if (!arg) continue;
      if(TString(arg->GetName()).Contains("ATLAS_norm")) { arg->setVal(1); continue; }
      if(TString(arg->GetName()).Contains("ATLAS_renorm")) { arg->setVal(1); continue; }
      if(TString(arg->GetName()).Contains("gamma_stat")) { arg->setVal(1); continue; }
      //if(TString(arg->GetName()).Contains("JVF")) { arg->setConstant(1); continue; }
      arg->setVal(5);
      nllup = nll->getVal();
      arg->setVal(-5);
      nlldn = nll->getVal();
      arg->setVal(0); // must re-set before fitting!!
      nllnom = nll->getVal();
      cout << "\t" << arg->GetName() << "\t" << nllup-nllnom << "\t" << nlldn-nllnom << endl;
    }

    std::cout << "initial parameters" << std::endl;
    constrainedParams->Print("v");

    std::cout << "INITIAL NLL = " << nll->getVal() << std::endl;

    //int printLevel = 0;
    //int minimPrintLevel = printLevel; // ROOT::Math::MinimizerOptions::DefaultPrintLevel();

    RooMinimizer minim(*nll);
    int strategy = ROOT::Math::MinimizerOptions::DefaultStrategy();
    minim.setStrategy( strategy );

    //minim.setErrorLevel(0.5);

    double tol =  ROOT::Math::MinimizerOptions::DefaultTolerance();
    tol = std::min(tol,0.1);
    // epsilon gets multiplied by e-3 somewhere
    minim.setEps( tol );
    minim.setEps( 1 );

    int status = -1;

    int optConstFlag = 2;
    minim.optimizeConst(optConstFlag);

    TStopwatch sw; sw.Start();

    TString minimizer = ROOT::Math::MinimizerOptions::DefaultMinimizerType();
    minimizer = minimType;
    TString algorithm = ROOT::Math::MinimizerOptions::DefaultMinimizerAlgo();

    int maxCalls = ROOT::Math::MinimizerOptions::DefaultMaxFunctionCalls();
    cout << "MAX CALLS " << maxCalls << "\t" << ROOT::Math::MinimizerOptions::DefaultMaxFunctionCalls() << endl;
    cout << "MAX Interations " << ROOT::Math::MinimizerOptions::DefaultMaxIterations() << endl;

    cout << "FitPDF" << endl;
    cout << "\t minimizer " << minimizer << endl;
    cout << "\t algorithm " << algorithm << endl;
    cout << "\t strategy  " << strategy << endl;
    cout << "\t tolerance " << tol << endl;
    // cout << "\t maxFCN    " << minim.maxFCN() << endl;

    // HistFitter
    if(fancy) {
      bool kickApplied(false);
      for (int tries = 1, maxtries = 4; tries <= maxtries; ++tries) {
        cout << "try " << tries << " " << kickApplied << endl;
        sw.Print();
        std::cout << "prefit values" << std::endl;
        constrainedParams->Print("v");
        status = minim.minimize(minimizer, algorithm);
        cout << "minimizer status = " << status << endl;
        if (status%1000 == 0) {  // ignore erros from Improve
          break;
        } else {
          if (tries == 1) {
            cout << "    ----> Doing a re-scan first" << endl;
            status = minim.minimize(minimizer,"Scan");
          }
          if (tries == 2) {
            if (ROOT::Math::MinimizerOptions::DefaultStrategy() == 1 ) {
              cout << "    ----> trying with strategy = 2" << endl;
              minim.setStrategy(2);
            }
            else { tries++; } // skip this trial if strategy
          }
          if (tries == 3) {
            cout << "    ----> trying with improve" << endl;
            minimizer = "Minuit2";
            algorithm = "migradimproved";
          }
          if (tries == 4 && !kickApplied) {
            cout << "    ----> trying fit with different starting values" << endl;
            RooFitResult* tmpResult = minim.save();
            const RooArgList& randList = tmpResult->randomizePars();
            *constrainedParams = randList;
            delete tmpResult;
            tries=0;          // reset the fit cycle
            kickApplied=true; // do kick only once
            minim.setStrategy(ROOT::Math::MinimizerOptions::DefaultStrategy());
          }
        }
      }
    } else if (retryOnHesseFailure) {
      int maxtries = 3;
      int tries = 1;
      while (true) {
	cout << "Starting minization, try number " << tries << " of " << maxtries << endl;
	status = minim.minimize(minimizer, algorithm);
	cout << "Calling Hesse ..."  << endl;
	int statusH = minim.hesse();
	cout << "Hesse Status : " << statusH << endl;
	if (statusH == 0 || tries >= maxtries) {
	  break;
	}
	cout << "    ----> trying fit with different starting values" << endl;
	RooFitResult* tmpResult = minim.save();
	const RooArgList& randList = tmpResult->randomizePars();
	*constrainedParams = randList;
	delete tmpResult;
	tries++;
      }
    } else {
      status = minim.minimize(minimizer, algorithm);
    }
    cout << "Minimize Status : " << status << endl;


    //cout << endl;
    RooFitResult * tmpResult = minim.save();
    const TMatrixDSym covarMat = tmpResult->covarianceMatrix();
    Double_t det = covarMat.Determinant();
    cout << "Determinant " << det << endl;
    if(det < 0) { cout << "Determinant negative" << endl; }
    const RooArgList& parFinal = tmpResult->floatParsFinal();
    //Int_t n = covarMat.GetNcols();

    if(blind) {
      bool isHiggsFit = true;
      TIterator* it = parFinal.createIterator();
      while( RooRealVar* o = (RooRealVar*)(it->Next()) ) {
        if(TString(o->GetName()).Contains("HiggsNorm")) {
          isHiggsFit = false;
          o->setVal(0);
          break;
        }
      }
      if(isHiggsFit) {
        it = parFinal.createIterator();
        while( RooRealVar* o = (RooRealVar*)(it->Next()) ) {
          if(TString(o->GetName()).Contains("SigXsec")) {
            o->setVal(1);
            break;
          }
        }
        ((RooRealVar*)(model->GetParametersOfInterest()->first()))->setVal(1);
      }
    }

    // get eigenvectors and eigenvalues
    TMatrixDSymEigen eigenValueMaker(covarMat);
    TVectorT<double> eigenValues   = eigenValueMaker.GetEigenValues();
    TMatrixT<double> eigenVectors  = eigenValueMaker.GetEigenVectors();
    cout << endl << "Eigenvalues  " << endl;
    for( int l=0; l<eigenValues.GetNrows(); l++ ) {
      cout << "\t" << l << "\t" << eigenValues[l] << endl;
//      for( int m=0; m<eigenValues.GetNrows(); m++) {
//        cout << "\t\t" << l << ", " << m << " " << eigenVectors[l][m] << endl;
//      }
    }
    cout << endl;


    //cout << "Covariance matrix:" << endl;
    //covarMat.Print();

    /*
    for (int i = 0 ; i<n ; i++) {
      for (int j = 0 ; j<n; j++) {
         cout << covarMat(i,j) << endl;
      }
    }
    for (int i = 0 ; i<n ; i++) {
      cout << parFinal.at(i)->GetName() << "\t" << covarMat(i,i) << endl;
    }
    */

    if (status%100 == 0) { // ignore errors in Hesse or in Improve
      if (!retryOnHesseFailure) {
        cout << "Calling Hesse ..."  << endl;
        status = minim.hesse();
        cout << "Hesse Status : " << status << endl;
      }
    } else {
      cout << "FIT FAILED !" << endl;
    }

    sw.Print();

    sw.Stop();

    RooFitResult * r = minim.save();

    if(blind) {
      bool isHiggsFit = true;
      const RooArgList& parFinal2 = r->floatParsFinal();
      TIterator* it = parFinal2.createIterator();
      while( RooRealVar* o = (RooRealVar*)(it->Next()) ) {
        if(TString(o->GetName()).Contains("HiggsNorm")) {
          isHiggsFit = false;
          o->setVal(0);
          break;
        }
      }
      if(isHiggsFit) {
        it = parFinal2.createIterator();
        while( RooRealVar* o = (RooRealVar*)(it->Next()) ) {
          if(TString(o->GetName()).Contains("SigXsec")) {
            o->setVal(1);
            break;
          }
        }
        ((RooRealVar*)(model->GetParametersOfInterest()->first()))->setVal(1);
      }
    }
    if(poi) {
      std::cout << "final poi parameters" << std::endl;
      model->GetParametersOfInterest()->Print("v");
    }

    typedef std::numeric_limits< double > dbl;
    cout.precision(dbl::digits10);
    std::cout << "FINAL NLL = " << nll->getVal() << std::endl;

    if(poi) {
      cout << "PRINTING FIT RESULT " << poi->isConstant() << "\t" << poi->getVal() << endl;
    } else {
      cout << "PRINTING FIT RESULT " << endl;
    }
    r->Print();

    sw.Print();

    return r;
  } // FitPDF


  void PlotHistosBeforeFit(double nSigmaToVary, double mu){
    cout << endl << "Plotting Histos Before Fit " << endl;
    cout << "\t Plotting relative " << plotRelative << endl;

    RooMsgService::instance().setGlobalKillBelow(RooFit::WARNING);

    TString MaindirName("MuIsEqualTo_");
    MaindirName += mu;
    if(plotRelative) { MaindirName.Append("_relative"); }
    TDirectory *MainDir = (TDirectory*) MainDirSyst->mkdir(MaindirName);
    gROOT->cd();

    // Get the RooSimultaneous PDF
    RooSimultaneous *simPdf = (RooSimultaneous*)(mc->GetPdf());
    RooRealVar * firstPOI = dynamic_cast<RooRealVar*>(mc->GetParametersOfInterest()->first());
    firstPOI->setVal(mu);

    RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;
    TString dirName("");
    while((tt=(RooCatType*) iter->Next()) ){

      cout << endl;
      cout << endl;
      cout << " -- On category " << tt->GetName() << " " << endl;
      ostringstream SubdirName;
      SubdirName << tt->GetName();
      TDirectory *SubDirChannel = (TDirectory*) MainDir->mkdir(SubdirName.str().c_str());
      gROOT->cd();

      // Get pdf associated with state from simpdf
      RooAbsPdf  *pdftmp  = simPdf->getPdf(tt->GetName()) ;
      RooArgSet  *obstmp  = pdftmp->getObservables( *mc->GetObservables() ) ;
      RooAbsData *datatmp = data->reduce(Form("%s==%s::%s",channelCat->GetName(),channelCat->GetName(),tt->GetName()));
      RooRealVar *obs     = ((RooRealVar*) obstmp->first());

      // Get the bin width
      RooRealVar* binWidth = ((RooRealVar*) pdftmp->getVariables()->find(Form("binWidth_obs_x_%s_0",tt->GetName()))) ;
      if(!binWidth) { cout << "No bin width!" << tt->GetName() << endl; return; }
      cout << "    Bin Width : " << binWidth->getVal() << endl;

      // First be sure that all nuisance parameters are nominal
      w->loadSnapshot("snapshot_paramsVals_initial");

      // Look at each component
      cout << "    Contains the following components : " << endl;
      TString modelName(tt->GetName());
      modelName.Append("_model");
      RooRealSumPdf *pdfmodel = (RooRealSumPdf*) (pdftmp->getComponents())->find(modelName);
      RooArgList funcList =  pdfmodel->funcList();
      RooLinkedListIter funcIter = funcList.iterator() ;
      RooProduct* comp = 0;
      float total(0);
      SetPOI(1);	 // want to see signal
      map<TString, TH1*> nominals;
      while( (comp = (RooProduct*) funcIter.Next())) {
        cout << " Component : " << comp->GetName() << endl;
        cout << "\t" << ( comp->createIntegral(*obs) )->getVal() * binWidth->getVal() << endl;
        total += ( comp->createIntegral(*obs) )->getVal() * binWidth->getVal();
      }
      SetPOI(mu);
      cout << " Total (mu = 1) : " << total << endl;

      // Loop over nuisance params
      TIterator* it = mc->GetNuisanceParameters()->createIterator();
      RooRealVar* var = NULL;
      bool IsAllStatDone = false;
      TString chanName(tt->GetName());
      while( (var = (RooRealVar*) it->Next()) ){

        string varname = (string) var->GetName();
        if ( varname.find("gamma_stat")!=string::npos ){
          continue;
        }

        // one sigma not defined for floating parameters
        // is there a more general way of getting to this fact?
        if ( varname.find("ATLAS_norm")!=string::npos ){
          continue;
        }
        if ( varname.find("ATLAS_sampleNorm")!=string::npos ){
          continue;
        }

        // user firendly label / name
        TString varName(var->GetName());
        varName.ReplaceAll("alpha_Sys","");
        varName.ReplaceAll("alpha_","");

        dirName = OutputDir+"/"+MainDirSyst->GetName()+"/"+MaindirName+"/"+chanName+"/"+varName;

        // Not consider nuisance parameter being not assocaited to systematics
        if (MapNuisanceParamNom[varname]!=0.0 &&
            MapNuisanceParamNom[varname]!=1.0 ) continue;

        cout << endl;
        cout << "  -- On nuisance parameter : " << var->GetName() << endl;

        TString histName("");

        // -1 sigma
        SetNuisanceParaToSigma(var,-nSigmaToVary);
        SetPOI(mu);
        histName = chanName+"_"+varName+"_"+TString(plotRelative)+"_m1sigma";
        TH1* hm1sigma = pdftmp->createHistogram(histName,*obs, Extended(), Scaling(false));
        hm1sigma->Scale(pdftmp->expectedEvents(*obs) / hm1sigma->Integral());

        // +1 sigma
        SetNuisanceParaToSigma(var,+nSigmaToVary);
        SetPOI(mu);
        histName.ReplaceAll("m1sigma","p1sigma");
        TH1* hp1sigma = pdftmp->createHistogram(histName,*obs, Extended(), Scaling(false));
        hp1sigma->Scale(pdftmp->expectedEvents(*obs) / hp1sigma->Integral());

        // Nominal
        SetNuisanceParaToSigma(var,0.0);
        SetPOI(mu);
        histName.ReplaceAll("p1sigma","nominal");
        TH1* hnominal = pdftmp->createHistogram(histName,*obs, Extended(), Scaling(false));
        hnominal->Scale(pdftmp->expectedEvents(*obs) / hnominal->Integral());

        // Data
        histName.ReplaceAll("nominal","data");
        TH1* hdata = datatmp->createHistogram(histName,*obs);
        for (int ib=0 ; ib<hdata->GetNbinsX()+1 ; ib++) hdata->SetBinError(ib, sqrt(hdata->GetBinContent(ib)));


        TString expName("AllBkg(#mu=");
        expName += mu;
        expName.Append(")");
        TCanvas* c2 = DrawShift(chanName,(TString)var->GetName(),expName,mu,hdata,hnominal,hp1sigma,hm1sigma);
        /*
           if(!c2) { continue; }
           */


        if(!SubDirChannel) {
          SubDirChannel = (TDirectory*) MainDir->mkdir(SubdirName.str().c_str());
        }
        //gROOT->cd();

        SubDirChannel->cd();
        c2->Write();
        if(drawPlots) {
          system(TString("mkdir -vp "+dirName));
          c2->Print(dirName+"/totalExpected.eps");
          c2->Print(dirName+"/totalExpected.png");
        }
        c2->Close();
        gROOT->cd();

        // reset pointer
        hdata->~TH1();
        hnominal->~TH1();
        hp1sigma->~TH1();
        hm1sigma->~TH1();
        hdata = 0;
        hnominal = 0;
        hp1sigma = 0;
        hm1sigma = 0;


        // Loop over components and make these plots for each one
        funcIter = funcList.iterator();
        while( (comp = (RooProduct*) funcIter.Next()) ) {
          if(!comp->dependsOn(*var)) { continue; }
          TString compName(comp->GetName());
          compName.ReplaceAll("L_x_","");
          compName.ReplaceAll(chanName,"");
          compName.ReplaceAll("__overallSyst_x_StatUncert","");
          compName.ReplaceAll("__overallSyst_x_HistSyst","");
          compName.ReplaceAll("__overallSyst_x_Exp","");

          // Fisrt be sure that all nuisance parameters are nominal
          w->loadSnapshot("snapshot_paramsVals_initial");
          SetPOI(1);	// set to one so do not ignore signal

          // -1 sigma
          SetNuisanceParaToSigma(var,-nSigmaToVary);
          SetPOI(1);
          histName = chanName+"_"+varName+"_"+compName+"_"+TString(plotRelative)+"_m1sigma";
          hm1sigma = comp->createHistogram(histName,*obs);
          hm1sigma->Scale( binWidth->getVal() );

          // +1 sigma
          SetNuisanceParaToSigma(var,+nSigmaToVary);
          SetPOI(1);
          histName.ReplaceAll("m1sigma","p1sigma");
          hp1sigma = comp->createHistogram(histName,*obs);
          hp1sigma->Scale( binWidth->getVal() );

          // nominal
          SetNuisanceParaToSigma(var,0.0);
          SetPOI(1);
          histName.ReplaceAll("p1sigma","nominal");
          hnominal = comp->createHistogram(histName,*obs);
          hnominal->Scale( binWidth->getVal() );

          /*
             if(hnominal->Integral()<0.001) { continue; }

             if( fabs((hp1sigma->Integral() / hnominal->Integral()) - 1) < 0.0005
             && fabs((hm1sigma->Integral() / hnominal->Integral()) - 1) < 0.0005 ) { continue; }
             */

          /*
          // skip components which are not affected by this nuisance parameter
          if(hp1sigma->Integral() == 0 || hm1sigma->Integral() == 0) {
          cout << "Integral 0 " << varName << " on " << compName << " in " << chanName
          << " ( " << hp1sigma->Integral() << ", " << hm1sigma->Integral() << " ) " << endl;
          continue;
          }
          */

          c2 = DrawShift(chanName,(TString)var->GetName(),compName,mu,0,hnominal,hp1sigma,hm1sigma);

          SubDirChannel->cd();
          c2->Write();
          if(drawPlots) {
            //system(TString("mkdir -vp "+dirName));
            c2->Print(dirName+"/"+compName+".eps");
            c2->Print(dirName+"/"+compName+".png");
          }
          c2->Close();
          gROOT->cd();

          // Put everything back to the nominal
          SetAllNuisanceParaToSigma(0.0);
          SetPOI(mu);

          //
          hnominal->~TH1();
          hp1sigma->~TH1();
          hm1sigma->~TH1();
          hnominal = 0;
          hp1sigma = 0;
          hm1sigma = 0;
        } // loop over components



        // Put everything back to the nominal
        SetAllNuisanceParaToSigma(0.0);
        SetPOI(mu);

        // Stat uncertainty
        if (!IsAllStatDone){

          // reset pointer
          hdata = 0;
          hnominal = 0;
          hp1sigma = 0;
          hm1sigma = 0;

          // -1 sigma
          SetAllStatErrorToSigma(-nSigmaToVary);
          SetAllNuisanceParaToSigma(0.0);
          SetPOI(mu);
          histName = chanName+"_Stat_"+TString(plotRelative)+"_m1sigma";
          hm1sigma = pdftmp->createHistogram(histName,*obs);
          hm1sigma->Scale(pdftmp->expectedEvents(*obs));

          // +1 sigma
          SetAllStatErrorToSigma(+nSigmaToVary);
          SetAllNuisanceParaToSigma(0.0);
          SetPOI(mu);
          histName.ReplaceAll("m1sigma","p1sigma");
          hp1sigma = pdftmp->createHistogram(histName,*obs);
          hp1sigma->Scale(pdftmp->expectedEvents(*obs));

          // Nominal
          SetAllStatErrorToSigma(0.0);
          SetNuisanceParaToSigma(var,0.0);
          SetPOI(mu);
          histName.ReplaceAll("p1sigma","nominal");
          hnominal = pdftmp->createHistogram(histName,*obs);
          hnominal->Scale(pdftmp->expectedEvents(*obs));

          // Data
          histName.ReplaceAll("nominal","data");
          hdata = datatmp->createHistogram(histName,*obs);
          for (int ib=0 ; ib<hdata->GetNbinsX()+1 ; ib++) hdata->SetBinError(ib, sqrt(hdata->GetBinContent(ib)));

          cout << endl;
          cout << " - stat uncertainty : " << endl;
          TCanvas* c4 = DrawShift(chanName,"Stat",expName,mu,hdata,hnominal,hp1sigma,hm1sigma);

          dirName = OutputDir+"/"+MainDirSyst->GetName()+"/"+MaindirName+"/"+chanName+"/Stat";
          SubDirChannel->cd();
          c4->Write();
          if(drawPlots) {
            system(TString("mkdir -vp "+dirName));
            c4->Print(dirName+"/totalExpected.eps");
            c4->Print(dirName+"/totalExpected.png");
          }
          c4->Close();
          gROOT->cd();

          IsAllStatDone=true;
        }

      }

    }

    return;
  }


  // create the canvas and put stuff on it
  // to be used when plotting the +/- 1 sigma shifts
  TCanvas* DrawShift(TString channel, TString var, TString comp, double mu, TH1* d, TH1* n, TH1* p1s, TH1* m1s) {
    cout << " " << comp << endl;
    cout << "N(-sigma) = " << m1s->Integral() << endl;
    cout << "N(+sigma) = " << p1s->Integral() << endl;
    cout << "N(nominal) = " << n->Integral() << endl;
    if(d) { cout << "N(Observed) = " << d->Integral() << endl; }

    var.ReplaceAll("alpha_Sys","");
    var.ReplaceAll("alpha_","");

    TString cname = "can_" + channel + "_" + comp + "_" + var + "_mu";
    cname += mu;
    if(plotRelative) { cname.Append("_relative"); }
    cname.ReplaceAll("#","");
    cname.ReplaceAll("(","");
    cname.ReplaceAll(")","");
    cname.ReplaceAll("=","");
    TCanvas *canvas = new TCanvas(cname,cname,700,550);
    canvas->cd();
    TPad *pad1 = new TPad("pad1","pad1",0,0.25,1,1);
    pad1->SetBottomMargin(0.009);
    pad1->Draw();
    TPad *pad2 = new TPad("pad2","pad2",0,0,1,0.25);
    pad2->SetTopMargin(0.009);
    pad2->SetBottomMargin(0.5);
    pad2->Draw();

    // style
    if(d) {
      d->SetLineColor(1);
      d->SetLineWidth(1);
      d->SetMarkerColor(1);
      d->SetMarkerSize(0.9);
      d->SetMarkerStyle(20);
    }
    n->SetLineWidth(2);
    p1s->SetLineColor(kRed);
    p1s->SetLineWidth(2);
    p1s->SetLineStyle(2);
    m1s->SetLineColor(kGreen);
    m1s->SetLineWidth(2);
    m1s->SetLineStyle(2);

    float max(0);
    float min(0);

    // put averages on the plot
    float avgUp = ( p1s->Integral() - n->Integral() ) / n->Integral();
    float avgDn = ( m1s->Integral() - n->Integral() ) / n->Integral();

    // Distribution in the upper pad
    pad1->cd();
    n->SetTitle(channel);
    n->GetXaxis()->SetTitle(xAxisLabel);
    max = p1s->GetMaximum();
    if(m1s->GetMaximum() > max) { max = m1s->GetMaximum(); }
    if(n->GetMaximum() > max) { max = n->GetMaximum(); }
    if(d) { if(d->GetMaximum() > max) { max = d->GetMaximum(); } }
    n->SetMaximum( 1.2*max );
    n->Draw("hist");
    if(d) { d->Draw("E1 same"); }
    p1s->Draw("hist same");
    m1s->Draw("hist same");

    // Distribution of the ratio in %
    pad2->cd();
    TH1F *p1s_ratio = (TH1F*) p1s->Clone();
    p1s_ratio->Add(n,-1); p1s_ratio->Divide(n); p1s_ratio->Scale(100);
    p1s_ratio->SetLineStyle(1);
    TH1F *m1s_ratio = (TH1F*) m1s->Clone();
    m1s_ratio->Add(n,-1); m1s_ratio->Divide(n); m1s_ratio->Scale(100);
    m1s_ratio->SetLineStyle(1);
    max = p1s_ratio->GetMaximum();
    if(m1s_ratio->GetMaximum() > max) { max = m1s_ratio->GetMaximum(); }
    min = p1s_ratio->GetMinimum();
    if(m1s_ratio->GetMinimum() < min) { min = m1s_ratio->GetMinimum(); }
    p1s_ratio->SetMaximum( 1.5*max );
    p1s_ratio->SetMinimum( min - 0.5*fabs(min) );
    p1s_ratio->GetYaxis()->SetNdivisions(004);
    p1s_ratio->GetXaxis()->SetTitleFont(43);
    p1s_ratio->GetXaxis()->SetTitleSize(16);
    p1s_ratio->GetXaxis()->SetTitleOffset(4);
    p1s_ratio->GetYaxis()->SetTitleOffset(1.1);
    p1s_ratio->GetYaxis()->SetTitleFont(43);
    p1s_ratio->GetYaxis()->SetTitleSize(13);
    p1s_ratio->GetXaxis()->SetLabelFont(43);
    p1s_ratio->GetXaxis()->SetLabelSize(13);
    p1s_ratio->GetYaxis()->SetLabelFont(43);
    p1s_ratio->GetYaxis()->SetLabelSize(13);
    p1s_ratio->SetTitle("");
    p1s_ratio->GetYaxis()->SetTitle("Rel. unc. (%)");
    p1s_ratio->Draw("hist");
    m1s_ratio->Draw("hist same");
    if (d){
      TH1F *d_ratio = (TH1F*) d->Clone();
      d_ratio->Add(n,-1); d_ratio->Divide(n); d_ratio->Scale(100);
      d_ratio->Draw("E1 same");
    }


    // get max and min and draw
    //  -- old way, just keep for book-keeping --
    if(plotRelative) {
      // draw percent error bands
      p1s->Add(n,-1); p1s->Divide(n); p1s->Scale(100);
      m1s->Add(n,-1); m1s->Divide(n); m1s->Scale(100);
      p1s->GetYaxis()->SetTitle("Percent Error");
      p1s->SetTitle(channel);
      p1s->GetXaxis()->SetTitle(xAxisLabel);
      max = p1s->GetMaximum();
      if(m1s->GetMaximum() > max) { max = m1s->GetMaximum(); }
      min = p1s->GetMinimum();
      if(m1s->GetMinimum() < min) { min = m1s->GetMinimum(); }
      p1s->SetMaximum( 1.5*max );
      p1s->SetMinimum( min - 0.5*fabs(min) );
      canvas->cd();
      p1s->Draw("hist");
      m1s->Draw("hist same");
      // draw nominal with a seperate axis on a transparent pad
      TPad *pad = new TPad("pad","pad",0,0,1,1);
      pad->SetFillStyle(4000); //will be transparent
      pad->SetFrameFillStyle(4000);
      pad->SetLeftMargin(canvas->GetLeftMargin());
      pad->SetRightMargin(canvas->GetRightMargin());
      pad->SetTopMargin(canvas->GetTopMargin());
      pad->SetBottomMargin(canvas->GetBottomMargin());
      pad->Draw();
      pad->cd();
      // new axis
      float xloc = n->GetXaxis()->GetBinLowEdge( n->GetNbinsX()+1 );
      TGaxis *axis = new TGaxis(xloc,0,xloc,p1s->GetMaximum(),0,n->GetMaximum(),510,"+L");
      axis->SetTitle(n->GetYaxis()->GetTitle());
      axis->SetTitleColor(kBlue);
      axis->SetLabelColor(kBlue);
      //axis->SetTitleFont(mFont);
      n->SetLineColor(kBlue);
      n->SetLineStyle(kDashed);
      n->GetYaxis()->SetTitle(""); n->SetTitle(""); n->GetXaxis()->SetTitle("");
      //n->Draw("hist ah e");
      n->Draw("hist ah");
      axis->Draw("same");
      pad->Update();
    }

    // write average shift on canvas
    if (!plotRelative) pad1->cd();
    TString info(var+" "+comp);
    info.Append(Form(" %5.2f, %5.2f",avgUp*100,avgDn*100));
    info.Append('%');
    TLatex *niceinfo = new TLatex(0.12, 0.85, info);
    niceinfo->SetNDC();
    niceinfo->SetTextSize(0.045);
    niceinfo->Draw("same");

    // legend
    TLegend *leg = new TLegend(0.67, 0.64, 0.87, 0.86);
    LegendStyle(leg);
    TString varLegName(var);
    varLegName.ReplaceAll("alpha_Sys","");
    varLegName.ReplaceAll("alpha_","");
    if(!plotRelative && d) { leg->AddEntry( d, "Data", "p" ); }
    leg->AddEntry( n, comp, "l" );
    leg->AddEntry( p1s, "+#sigma", "l" );
    leg->AddEntry( m1s, "-#sigma", "l" );
    leg->Draw();

    return canvas;
  } // DrawShift





  void PlotMorphingControlPlots(){

    cout << endl << "Plotting Systematic morphing control plots" << endl;
    RooMsgService::instance().setGlobalKillBelow(ERROR);

    // Get the RooSimultaneous PDF
    RooSimultaneous *simPdf = (RooSimultaneous*)(mc->GetPdf());
    RooRealVar * firstPOI = dynamic_cast<RooRealVar*>(mc->GetParametersOfInterest()->first());
    double mu=0;
    firstPOI->setVal(mu);

    RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;
    TString dirName("");
    while((tt=(RooCatType*) iter->Next()) ){

      cout << endl;
      cout << endl;
      cout << " -- On category " << tt->GetName() << " " << endl;
      ostringstream SubdirName;
      SubdirName << tt->GetName();
      TDirectory *SubDirChannel = (TDirectory*) MainDirMorphing->mkdir(SubdirName.str().c_str());
      gROOT->cd();

      // Get pdf associated with state from simpdf
      RooAbsPdf  *pdftmp  = simPdf->getPdf(tt->GetName()) ;
      RooArgSet  *obstmp  = pdftmp->getObservables( *mc->GetObservables() ) ;
      RooRealVar *obs     = ((RooRealVar*) obstmp->first());

      // First be sure that all nuisance parameters are nominal
      if(!w->loadSnapshot("snapshot_paramsVals_initial")) {
        SetAllStatErrorToSigma(0.0);
        SetAllNuisanceParaToSigma(0.0);
      }

      // Loop over nuisance params
      TIterator* it = mc->GetNuisanceParameters()->createIterator();
      RooRealVar* var = NULL;
      TString chanName(tt->GetName());
      while( (var = (RooRealVar*) it->Next()) ){

        string varname = (string) var->GetName();
        if ( varname.find("gamma_stat")!=string::npos ){
          continue;
        }

        if ( varname.find("ATLAS_norm")!=string::npos ){
          continue;
        }
        if ( varname.find("ATLAS_sampleNorm")!=string::npos ){
          continue;
        }

        // user friendly label / name
        TString varName(var->GetName());
        varName.ReplaceAll("alpha_Sys","");
        varName.ReplaceAll("alpha_","");

        // Not consider nuisance parameter being not assocaited to systematics
        if (MapNuisanceParamNom[varname]!=0.0 &&
            MapNuisanceParamNom[varname]!=1.0 ) continue;

        cout << endl;
        cout << "  -- On nuisance parameter : " << var->GetName() << endl;
        TDirectory *SubDirNP = (TDirectory*) SubDirChannel->mkdir(varName);
        gROOT->cd();

        TString cname = "can_" + (TString)tt->GetName() + "_" + varName;
        TCanvas* c2 = new TCanvas( cname );
        c2->cd();
        TH1* hh = pdftmp->createHistogram("hh_"+cname,*obs,YVar(*var,Binning(60)) ) ;
        hh->SetLineColor(kBlue) ;
        hh->GetZaxis()->SetTitleOffset(2.5) ; hh->Draw("surf") ;

        SubDirNP->cd();
        c2->Write();
        c2->Close();
        gROOT->cd();

        // Loop over components and make these plots for each one
        TString modelName(tt->GetName());
        modelName.Append("_model");
        RooRealSumPdf *pdfmodel = (RooRealSumPdf*) (pdftmp->getComponents())->find(modelName);
        RooArgList funcList =  pdfmodel->funcList();
        RooLinkedListIter funcIter = funcList.iterator() ;
        RooProduct* comp = 0;
        while( (comp = (RooProduct*) funcIter.Next()) ) {
          TString compName(comp->GetName());
          compName.ReplaceAll("L_x_","");
          compName.ReplaceAll(chanName,"");
          compName.ReplaceAll("__overallSyst_x_StatUncert","");
          compName.ReplaceAll("__overallSyst_x_HistSyst","");
          compName.ReplaceAll("__overallSyst_x_Exp","");

          // Fisrt be sure that all nuisance parameters are nominal
          if(!w->loadSnapshot("snapshot_paramsVals_initial")) {
            SetAllStatErrorToSigma(0.0);
            SetAllNuisanceParaToSigma(0.0);
            SetPOI(1);	// set to one so do not ignore signal
          }

          cname = "can_" + (TString)tt->GetName() + "_" + compName + "_" + varName;
          TCanvas* c3 = new TCanvas( cname );
          c2->cd();

          hh = comp->createHistogram("hh_"+cname,*obs,YVar(*var,Binning(60)) ) ;
          hh->SetLineColor(kBlue) ;
          hh->GetZaxis()->SetTitleOffset(2.5) ; hh->Draw("surf") ;

          SubDirNP->cd();
          c3->Write();
          c3->Close();
          gROOT->cd();

          // Put everything back to the nominal
          SetAllNuisanceParaToSigma(0.0);
          SetPOI(mu);

        } // loop over components


        // Put everything back to the nominal
        SetAllNuisanceParaToSigma(0.0);
        SetPOI(mu);

      }
    }

    return;
  }



  void PlotHistosAfterFitEachSubChannel(bool IsConditionnal, double mu){
    cout << endl << "Plotting Histos After Fit in Each Subchannel" << endl;

    // Prepare NP_i[after fit] vs category histogram
    map < TString,vector<NPContainer> > MapChannelNPs;

    // Put all parameters to their iniital values
    if(!w->loadSnapshot("snapshot_paramsVals_initial")) {
      cout << "Cannot load " <<  "snapshot_paramsVals_initial" << endl;
      exit(-1);
    }
    SetAllNuisanceParaToSigma(0);

    // Conditionnal or unconditional fit
    TString TS_IsConditionnal;
    if (IsConditionnal) TS_IsConditionnal="conditionnal";
    else                TS_IsConditionnal="unconditionnal";
    RooRealVar * firstPOI = dynamic_cast<RooRealVar*>(mc->GetParametersOfInterest()->first());
    firstPOI->setVal(mu);

    ostringstream MaindirName;
    if (IsConditionnal) MaindirName << TS_IsConditionnal << "_MuIsEqualTo_" << mu;
    else                MaindirName << TS_IsConditionnal;
    TDirectory *MainDir =  (TDirectory*) MainDirFitEachSubChannel->mkdir(MaindirName.str().c_str());
    gROOT->cd();

    // Get the RooSimultaneous PDF
    RooSimultaneous *simPdf = (RooSimultaneous*)(mc->GetPdf());

    RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
    TIterator *iter = channelCat->typeIterator() ;
    RooCatType *tt  = NULL;
    while((tt=(RooCatType*) iter->Next()) ){

      if(!w->loadSnapshot("snapshot_paramsVals_initial")) {
        cout << "Cannot load " <<  "snapshot_paramsVals_initial" << endl;
        exit(-1);
      }
      SetAllNuisanceParaToSigma(0);

      cout << endl;
      cout << endl;
      cout << " -- On category " << tt->GetName() << " " << endl;
      ostringstream SubdirName;
      SubdirName << tt->GetName();
      TDirectory *SubDirChannel = (TDirectory*) MainDir->mkdir(SubdirName.str().c_str());
      gROOT->cd();

      // Get pdf and datset associated to the studied channel
      RooAbsPdf  *pdftmp  = simPdf->getPdf( tt->GetName() );
      RooArgSet  *obstmp  = pdftmp->getObservables( *mc->GetObservables() ) ;
      RooAbsData *datatmp = data->reduce(Form("%s==%s::%s",channelCat->GetName(),channelCat->GetName(),tt->GetName()));
      RooRealVar *obs = ((RooRealVar*) obstmp->first());

      // Fit
      if (IsConditionnal) firstPOI->setConstant();
      ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
      ROOT::Math::MinimizerOptions::SetDefaultStrategy(1);
      RooFitResult *fitres = FitPDF( mc, pdftmp, datatmp, "Minuit2" );
      const RooArgSet* obsSet = mc->GetObservables();
      RooArgList floatPars = getFloatParList( *pdftmp, *obsSet);
      if (IsConditionnal) cout << "Conditionnal fit : mu is fixed at " << mu << endl;
      else                cout << "Unconditionnal fit : mu is fitted" << endl;
      double muhat = firstPOI->getVal();
      firstPOI->setConstant(kFALSE);

      // Plotting the nuisance paramaters correlations during the fit
      TString cname = "can_NuisPara_" + (TString)tt->GetName() + "_" + TS_IsConditionnal + "_mu";
      cname += mu;
      TCanvas* c1 = new TCanvas( cname, cname, 1260, 500);
      c1->Divide(2,1);
      TH2D *h2Dcorrelation = (TH2D*) fitres->correlationHist(cname);
      TString hname = "Corr_NuisPara_"+ (TString)tt->GetName() + "_"  + TS_IsConditionnal + "_mu";
      h2Dcorrelation->SetName(hname);
      c1->cd(1); h2Dcorrelation->Draw("colz");

      // Plotting the nuisance paramaters after fit
      TString h1name = "h_NuisParaPull_" + (TString)tt->GetName() + "_" + TS_IsConditionnal + "_mu";
      h1name += mu;
      TIterator* it1 = mc->GetNuisanceParameters()->createIterator();
      RooRealVar* var = NULL;
      int Npar=0;
      int NparNotStat=0;
      while( (var = (RooRealVar*) it1->Next()) ) {
        Npar++;
        string varname = (string) var->GetName();
        if (varname.find("gamma_stat")==string::npos) NparNotStat++;
      }
      // assignment is irrelevant - comment out to remove warning: AJB
      //      NparNotStat=NparNotStat;
      TH1F * h1Dpull_axis = new TH1F(h1name,h1name,NparNotStat,0,NparNotStat);
      TGraphAsymmErrors *h1Dpull = new TGraphAsymmErrors(NparNotStat);
      h1Dpull->SetLineWidth(2);
      h1Dpull->SetLineColor(1);
      h1Dpull->SetMarkerColor(1);
      h1Dpull->SetMarkerStyle(21);
      h1Dpull->SetMarkerSize(1.2);
      h1Dpull_axis->GetYaxis()->SetRangeUser(-5.5,5.5);
      h1Dpull_axis->GetXaxis()->SetTitle("#theta");
      h1Dpull_axis->GetYaxis()->SetTitle("(#theta_{fit} - #theta_{0}) / #Delta#theta");

      // Create a latex table of NPs after fit
      vector <NPContainer> MyNPContainerVector; MyNPContainerVector.clear();
      TString fname = OutputDir + "/LatexFileNPs/Fit"+(TString)tt->GetName()+"_nuisPar_"+TS_IsConditionnal+"_mu";
      fname += mu;
      fname += ".tex";
      ofstream fnuisPar(fname.Data());
      TString fnuiscorr = OutputDir + "/TextFileFitResult/Fit"+(TString)tt->GetName()+"_fitres_"+TS_IsConditionnal+"_mu";
      fnuiscorr += mu;
      fnuiscorr += ".txt";
      ofstream fnuisParAndCorr(fnuiscorr.Data());
      fnuisParAndCorr << "NUISANCE_PARAMETERS" << endl;

      fnuisPar << endl;
      fnuisPar << "\\begin{tabular}{|l|c|}" << endl;
      fnuisPar << "\\hline" << endl;
      fnuisPar << "Nuisance parameter & postfit value (in $\\sigma$ unit) \\\\\\hline" << endl;

      if(!IsConditionnal){
        fnuisPar.precision(3);
        fnuisPar << "$\\mu$ & $" << firstPOI->getVal() << "^{+" << firstPOI->getErrorHi() << "}_{-" << fabs(firstPOI->getErrorLo()) << "}$ \\\\" << endl;

        NPContainer MyNPTemp;
        MyNPTemp.NPname  = "#mu";
        MyNPTemp.NPvalue = firstPOI->getVal();
        MyNPTemp.NPerrorLo = firstPOI->getErrorLo();
        MyNPTemp.NPerrorHi = firstPOI->getErrorHi();
        MyNPContainerVector.push_back(MyNPTemp);
      }


      vector<TGraphAsymmErrors*> vec_MyGraph;
      vec_MyGraph.clear();
      int ib=0;
      TIterator* it2 = mc->GetNuisanceParameters()->createIterator();
      while( (var = (RooRealVar*) it2->Next()) ){

        // Not consider nuisance parameter being not associated to syst
        string varname = (string) var->GetName();
        if ( (varname.find("gamma_stat")!=string::npos) ) continue;

        double pull  = var->getVal() / 1.0 ; // GetValue() return value in unit of sigma
        double errorHi = var->getErrorHi() / 1.0;
        double errorLo = var->getErrorLo() / 1.0;

        if(strcmp(var->GetName(),"Lumi")==0){
          pull  = (var->getVal() - w->var("nominalLumi")->getVal() ) / (w->var("nominalLumi")->getVal() * LumiRelError );
          errorHi = var->getErrorHi() / (w->var("nominalLumi")->getVal() * LumiRelError);
          errorLo = var->getErrorLo() / (w->var("nominalLumi")->getVal() * LumiRelError);
        }

        TString vname=var->GetName();
        vname.ReplaceAll("alpha_","");
        vname.ReplaceAll("gamma_","");
        vname.ReplaceAll("Lumi","Luminosity");
        vname.ReplaceAll("_","\\_");
        fnuisPar.precision(3);
        fnuisPar << vname << " & $" << pull << "^{+" << fabs(errorHi) << "}_{-" << fabs(errorLo)  << "}$ \\\\" << endl;
        fnuisParAndCorr << vname << " & $" << pull << "^{+ " << fabs(errorHi) << "}_{-" << fabs(errorLo)  << "}$ \\\\" << endl;

        NPContainer MyNPsTemp;
        MyNPsTemp.NPname  = vname;
        MyNPsTemp.NPvalue = pull;
        MyNPsTemp.NPerrorHi = errorHi;
        MyNPsTemp.NPerrorLo = errorLo;
        MyNPContainerVector.push_back(MyNPsTemp);

        MyNPsTemp.WhichFit = TS_IsConditionnal+"_FitOnChannel_"+(TString)tt->GetName()+"_Mu";
        MyNPsTemp.WhichFit += mu;
        AllNPafterEachFit.push_back(MyNPsTemp);

        ib++;
        double xc = h1Dpull_axis->GetBinCenter(ib);
        TString vname2=var->GetName();
        vname2.ReplaceAll("alpha_","");
        vname2.ReplaceAll("gamma_","");
        vname2.ReplaceAll("ATLAS_","");
        h1Dpull_axis->GetXaxis()->SetBinLabel(ib,vname2);

        h1Dpull->SetPoint(ib-1,xc,pull);
        h1Dpull->SetPointEXlow(ib-1 ,h1Dpull_axis->GetBinWidth(ib)/3.0);
        h1Dpull->SetPointEXhigh(ib-1,h1Dpull_axis->GetBinWidth(ib)/3.0);
        h1Dpull->SetPointEYlow(ib-1 ,fabs(errorLo));
        h1Dpull->SetPointEYhigh(ib-1,fabs(errorHi));


        // Put in red NP with pull>1.5 OR Err<0.2, in black otherwise
        bool IsTooPulled = fabs(pull)>PullMaxAcceptable;
        bool IsOverConst = (fabs(errorLo)+fabs(errorHi))/2.0<ErrorMinAcceptable;
        bool IsOnesided  = UseMinosError && (errorHi==0 || errorLo==0);
        if (IsTooPulled || IsOverConst || IsOnesided){
          TGraphAsymmErrors *myGraph = new TGraphAsymmErrors(1);
          myGraph->SetLineWidth(2);
          myGraph->SetLineColor(kRed+1);
          myGraph->SetMarkerColor(kRed+1);
          myGraph->SetMarkerStyle(21);
          myGraph->SetMarkerSize(1.2);
          myGraph->SetPoint(0,xc,pull);
          myGraph->SetPointEXlow(0 ,h1Dpull_axis->GetBinWidth(ib)/3.0);
          myGraph->SetPointEXhigh(0,h1Dpull_axis->GetBinWidth(ib)/3.0);
          myGraph->SetPointEYlow(0 ,fabs(errorLo));
          myGraph->SetPointEYhigh(0,fabs(errorHi));
          vec_MyGraph.push_back(myGraph);
        }
      }

      MapChannelNPs[(TString)tt->GetName()] = MyNPContainerVector;

      fnuisPar << "\\hline" << endl;
      fnuisPar << "\\end{tabular}" << endl;
      fnuisPar.close();

      fnuisParAndCorr << endl << endl << "CORRELATION_MATRIX" << endl;
      fnuisParAndCorr << h2Dcorrelation->GetNbinsX() << "   " << h2Dcorrelation->GetNbinsY() << endl;
      for(int kk=1; kk < h2Dcorrelation->GetNbinsX()+1; kk++) {
        for(int ll=1; ll < h2Dcorrelation->GetNbinsY()+1; ll++) {
          fnuisParAndCorr << h2Dcorrelation->GetBinContent(kk,ll) << "   ";
        }
        fnuisParAndCorr << endl;
      }
      fnuisParAndCorr << endl;
      fnuisParAndCorr.close();


      double _1SigmaValue[1000];
      double _2SigmaValue[1000];
      double NuisParamValue[1000];
      for (int i=0 ; i<NparNotStat+1 ; i++){
        _1SigmaValue[i] = 1.0;
        _1SigmaValue[2*NparNotStat-i] = -1;
        _2SigmaValue[i] = 2;
        _2SigmaValue[2*NparNotStat-i] = -2;
        NuisParamValue[i] = i;
        NuisParamValue[2*NparNotStat-1-i] = i;
      }

      TGraph *_1sigma = new TGraph(2*NparNotStat,NuisParamValue,_1SigmaValue);
      TGraph *_2sigma = new TGraph(2*NparNotStat,NuisParamValue,_2SigmaValue);
      c1->cd(2);
      _2sigma->SetFillColor(5);
      _2sigma->SetLineColor(5);
      _2sigma->SetMarkerColor(5);
      _1sigma->SetFillColor(3);
      _1sigma->SetLineColor(3);
      _1sigma->SetMarkerColor(3);
      h1Dpull_axis->Draw("hist");
      _2sigma->Draw("F");
      _1sigma->Draw("F");
      h1Dpull->Draw("P");
      for (unsigned i=0 ; i<vec_MyGraph.size() ; i++) vec_MyGraph[i]->Draw("P");
      h1Dpull_axis->GetYaxis()->DrawClone();


      TLatex text;
      text.SetNDC();
      text.SetTextSize( 0.054);
      text.SetTextAlign(31);
      TString WritDownMuValue;
      if(!IsConditionnal) WritDownMuValue = "#mu_{best} = ";
      else                WritDownMuValue = "#mu_{fixed} = ";
      WritDownMuValue += Form("%2.2f",firstPOI->getVal());
      c1->cd(2);
      if(!blind) { text.DrawLatex( 0.87,0.81, WritDownMuValue ); }


      // Plotting the likelihood projection in each NP direction
      TDirectory *NLLprojection = (TDirectory*) SubDirChannel->mkdir("AllNNLProjections");
      gROOT->cd();
      RooAbsReal* nll = pdftmp->createNLL(*datatmp);
      if (!IsConditionnal){
        var = (RooRealVar*) firstPOI;
        TString vname=var->GetName();
        vname.ReplaceAll("alpha_","");
        vname.ReplaceAll("gamma_","");
        vname.ReplaceAll("ATLAS_","");
        RooPlot* frame2 = var->frame(Title("-log(L) vs "+vname));
        nll->plotOn(frame2,LineColor(kRed),ShiftToZero()) ;
        frame2->GetYaxis()->SetRangeUser(0.0,5.0);
        frame2->GetYaxis()->SetTitle("#Delta [-2Log(L)]");
        TCanvas *can2 = new TCanvas( "NLLscan_"+vname );
        can2->cd();
        frame2->Draw();
        NLLprojection->cd();
        can2->Write();
        gROOT->cd();
      }
      TIterator* it3 = mc->GetNuisanceParameters()->createIterator();
      while( (var = (RooRealVar*) it3->Next()) ){
        var->setRange(-2,2); // FIXME
        TString vname=var->GetName();
        if (vname.Contains("gamma_stat")) continue;
        vname.ReplaceAll("alpha_","");
        vname.ReplaceAll("gamma_","");
        vname.ReplaceAll("ATLAS_","");
        RooPlot* frame2 = var->frame(Title("-log(L) vs "+vname)) ;
        nll->plotOn(frame2,LineColor(kRed),ShiftToZero()) ;
        frame2->GetYaxis()->SetRangeUser(0.0,5.0);
        frame2->GetYaxis()->SetTitle("#Delta [-2Log(L)]");
        TCanvas *can2 = new TCanvas( "NLLscan_"+vname );
        can2->cd();
        frame2->Draw();
        NLLprojection->cd();
        can2->Write();
        gROOT->cd();
      }

      if(!makePostFitPlots) { return; }

      // Bin Width
      RooRealVar* binWidth = ((RooRealVar*) pdftmp->getVariables()->find(Form("binWidth_obs_x_%s_0",tt->GetName()))) ;
      if(!binWidth) { cout << "No bin width " << tt->GetName() << endl; }
      cout << "    Bin Width : " << binWidth->getVal() << endl;

      // Plotting the distributions
      cname = "can_DistriAfterFit_" + (TString)tt->GetName() + "_" + TS_IsConditionnal + "_mu";
      cname += mu;
      TCanvas* c2 = new TCanvas( cname );
      RooPlot* frame = obs->frame();
      TString FrameName = "Plot_" + (TString)tt->GetName() + "_FitIsconditional" + (TString) IsConditionnal;
      frame->SetName( FrameName );
      frame->SetYTitle("EVENTS");
      float postFitIntegral = pdftmp->expectedEvents(*obs);
      pdftmp->plotOn(frame,FillColor(kOrange),LineWidth(2),LineColor(kBlue),VisualizeError(*fitres,1),
                     Normalization(postFitIntegral,RooAbsReal::NumEvent),Name("FitError_NotAppears"));
      pdftmp->plotOn(frame,LineWidth(2),Normalization(postFitIntegral,RooAbsReal::NumEvent),Name("CentralFit_NotAppears"));
      double chi2 = frame->chiSquare();
      if(blind) { // zero out the signal region
        TString histName(tt->GetName());
        histName.Append("_data");
        TH1* hdata = datatmp->createHistogram(histName,*obs);
        if(hdata->GetNbinsX()>1) {
          float width(hdata->GetBinWidth(1));
          for(int b=1; b<hdata->GetNbinsX()+1; b++) {
            hdata->SetBinError(b, sqrt(hdata->GetBinContent(b)));
            if( (hdata->GetBinLowEdge(b) > 100 || (hdata->GetBinLowEdge(b)+width) > 100) && hdata->GetBinLowEdge(b) < 150) {
              hdata->SetBinContent(b,0);
              hdata->SetBinError(b,0);
            }
          }
        }
        hdata->SetMarkerSize(2);
        frame->addTH1(hdata);
      } else {
        datatmp->plotOn(frame,MarkerSize(1),Name("Data"),DataError(RooAbsData::Poisson) );
      }

      // miniloop over componant to make a bkg stack
      TString modelName1(tt->GetName());
      modelName1.Append("_model");
      RooRealSumPdf *pdfmodel1 = (RooRealSumPdf*) (pdftmp->getComponents())->find(modelName1);
      RooArgList funcList1 =  pdfmodel1->funcList();
      RooLinkedListIter funcIter1 = funcList1.iterator() ;
      RooProduct* comp1 = 0;
      firstPOI->setVal(0.);
      int ibkg=0;
      int icolor=0;
      TString previous="";
      while( (comp1 = (RooProduct*) funcIter1.Next()) ) {
        ibkg++;

        int color= kGray;

        TString compname(comp1->GetName());
        compname.ReplaceAll("L_x_","");
        compname.ReplaceAll(tt->GetName(),"");
        compname.ReplaceAll("_overallSyst_x_StatUncert","");
        compname.ReplaceAll("_overallSyst_x_HistSyst","");
        compname.ReplaceAll("_overallSyst_x_Exp","");
        compname.ReplaceAll("_","");
        double Ntemp = (comp1->createIntegral(*obs))->getVal() * binWidth->getVal();
        if (Ntemp!=0){
          icolor++;
          if(icolor==1) { color = kOrange - 3; }
          else if(icolor==2) { color = kCyan + 1; }
          else if(icolor==3) { color = kGreen - 9; }
          else if(icolor==4) { color = kAzure - 9; }
          else if(icolor==5) { color = kOrange + 10;}
          else if(icolor==6) { color = kGreen - 6; }
          else if(icolor==7) { color = kAzure - 4; }
          else if(icolor==8) { color = kOrange + 6; }
          else if(icolor==9) { color = kGreen + 1; }
          else if(icolor==10) { color = kAzure + 2; }
          else if(icolor==11) { color = kOrange; }
          else if(icolor==12) { color = kGreen + 3; }
          else if(icolor==13) { color = kAzure - 4; }
          else if(icolor==14) { color = kOrange; }
          else if(icolor==15) { color = kGreen + 1; }
          else if(icolor==16) { color = kOrange - 7; }
          else if(icolor==17) { color = kPink + 1; }
          else   color=icolor;

          // Get the signal x 1 in white (for further purposes)
          if (IsConditionnal) firstPOI->setVal(1.0);
          Ntemp=(comp1->createIntegral(*obs))->getVal() * binWidth->getVal();
          pdfmodel1->plotOn(frame,LineWidth(0),Components(*comp1),LineColor(0), LineStyle(3), Normalization(Ntemp,RooAbsReal::NumEvent),Name("NoStacked_"+compname));
          if (IsConditionnal) firstPOI->setVal(mu);

          // Get the stack of background
          Ntemp=(comp1->createIntegral(*obs))->getVal() * binWidth->getVal();
          pdfmodel1->plotOn(frame,LineWidth(0),Components(*comp1),LineColor(0), Normalization(Ntemp,RooAbsReal::NumEvent),Name("NoStacked"+compname));
          if (ibkg==0) pdfmodel1->plotOn(frame,LineWidth(2),Components(*comp1),LineColor(color), Normalization(Ntemp,RooAbsReal::NumEvent),Name("Stacked_"+compname));
          else         pdfmodel1->plotOn(frame,LineWidth(2),Components(*comp1),LineColor(color), Normalization(Ntemp,RooAbsReal::NumEvent),Name("Stacked_"+compname),AddTo(previous));
          previous="Stacked_"+compname;
        }
      }
      firstPOI->setVal(muhat);
      pdftmp->plotOn(frame,FillColor(kOrange),LineWidth(2),LineColor(kBlue),VisualizeError(*fitres,1),
                     Normalization(postFitIntegral,RooAbsReal::NumEvent),Name("FitError_AfterFit"));
      pdftmp->plotOn(frame,LineWidth(2),Normalization(postFitIntegral,RooAbsReal::NumEvent),Name("FitCentral2_NotAppears"));
      if(blind) { // zero out the signal region
        TString histName(tt->GetName());
        histName.Append("_data");
        TH1* hdata = datatmp->createHistogram(histName,*obs);
        if(hdata->GetNbinsX()>1) {
          float width(hdata->GetBinWidth(1));
          for(int b=1; b<hdata->GetNbinsX()+1; b++) {
            hdata->SetBinError(b, sqrt(hdata->GetBinContent(b)));
            if( (hdata->GetBinLowEdge(b) > 100 || (hdata->GetBinLowEdge(b)+width) > 100) && hdata->GetBinLowEdge(b) < 150) {
              hdata->SetBinContent(b,0);
              hdata->SetBinError(b,0);
            }
          }
        }
        hdata->SetMarkerSize(2);
        frame->addTH1(hdata);
      } else {
        datatmp->plotOn(frame,MarkerSize(1),Name("Dat_NotAppears"),DataError(RooAbsData::Poisson));
      }

      // Putting nuisance parameter at the central value and draw the nominal distri
      SetAllStatErrorToSigma(0.0);
      SetAllNuisanceParaToSigma(0.0);
      if (!IsConditionnal) SetPOI(0.0);
      else                 SetPOI(mu);
      TString muValueBeforeFitLegend = Form("Before fit (#mu=%2.2f)",firstPOI->getVal());
      pdftmp->plotOn(frame,LineWidth(2),Name("BeforeFit"),LineStyle(kDashed),Normalization(pdftmp->expectedEvents(*obs),RooAbsReal::NumEvent));

      TString modelName2(tt->GetName());
      modelName2.Append("_model");
      RooRealSumPdf *pdfmodel2 = (RooRealSumPdf*) (pdftmp->getComponents())->find(modelName2);
      RooArgList funcList2 =  pdfmodel2->funcList();
      RooLinkedListIter funcIter2 = funcList2.iterator() ;
      RooProduct* comp2 = 0;
      if (!IsConditionnal) firstPOI->setVal(muhat);
      else                 firstPOI->setVal(mu);
      ibkg=0;
      icolor=0;
      while( (comp2 = (RooProduct*) funcIter2.Next()) ) {
        ibkg++;
        int color=kGray;
        TString compname(comp2->GetName());
        compname.ReplaceAll("L_x_","");
        compname.ReplaceAll(tt->GetName(),"");
        compname.ReplaceAll("_overallSyst_x_StatUncert","");
        compname.ReplaceAll("_overallSyst_x_HistSyst","");
        compname.ReplaceAll("_overallSyst_x_Exp","");
        compname.ReplaceAll("_","");
        double Ntemp = (comp2->createIntegral(*obs))->getVal() * binWidth->getVal();
        if (Ntemp!=0){
          icolor++;
          if(icolor==1) { color = kOrange - 3; }
          else if(icolor==2) { color = kCyan + 1; }
          else if(icolor==3) { color = kGreen - 9; }
          else if(icolor==4) { color = kAzure - 9; }
          else if(icolor==5) { color = kOrange + 10;}
          else if(icolor==6) { color = kGreen - 6; }
          else if(icolor==7) { color = kAzure - 4; }
          else if(icolor==8) { color = kOrange + 6; }
          else if(icolor==9) { color = kGreen + 1; }
          else if(icolor==10) { color = kAzure + 2; }
          else if(icolor==11) { color = kOrange; }
          else if(icolor==12) { color = kGreen + 3; }
          else if(icolor==13) { color = kAzure - 4; }
          else if(icolor==14) { color = kOrange; }
          else if(icolor==15) { color = kGreen + 1; }
          else if(icolor==16) { color = kOrange - 7; }
          else if(icolor==17) { color = kPink + 1; }
          else   color=icolor;
          //i//pdfmodel2->plotOn(frame,LineWidth(0),Components(*comp2),LineColor(0), LineStyle(3), Normalization(Ntemp,RooAbsReal::NumEvent),Name("NoStacked_BkgBeforeFit_"+compname));
        }
      }
      if (!IsConditionnal) firstPOI->setVal(muhat);
      else                 firstPOI->setVal(mu);

      c2->cd();
      frame->Draw();
      if(!blind) { text.DrawLatex( 0.84,0.81, WritDownMuValue ); }
      TString ts_chi2 = Form("#chi^{2}=%1.1f",chi2);
      text.DrawLatex( 0.22, 0.83, ts_chi2 );

      TLegend *leg = new TLegend(0.54,0.40,0.77,0.76);
      leg->SetBorderSize(0);
      leg->SetFillColor(0);
      leg->SetTextFont(62);
      leg->SetTextSize(0.050);
      for (int i=0; i<frame->numItems(); i++) {
        TString obj_name=frame->nameOf(i);
        if (obj_name=="" || obj_name.Contains("NotAppears") || obj_name.Contains("NoStacked") ) continue;
        TObject *obj = frame->findObject(obj_name.Data());
        if (((string)obj_name).find("Data")   !=string::npos) { leg->AddEntry( obj , "Data" , "p"); continue; }
        TString legname;
        if (!IsConditionnal) legname = "After fit (#mu[best] S+B)";
        else                 legname = "After fit (#mu[fixed] S+B)";
        if(!blind) { if (((string)obj_name).find("AfterFit")!=string::npos){ leg->AddEntry( obj , legname , "lf"); continue; } }
        legname = muValueBeforeFitLegend;
        if (((string)obj_name).find("BeforeFit")!=string::npos){leg->AddEntry( obj ,legname , "l"); continue;}

        obj_name.ReplaceAll("Stacked_","");
        leg->AddEntry(obj,obj_name,"l");
      }
      leg->Draw();


      // Save plots in outputfile
      SubDirChannel->cd();
      c1->Write();
      c2->Write();
      c1->Close();
      c2->Close();
      gROOT->cd();
    }


    // Plots of each NP[posfit] vs subchannel
    int Nchannel=0;
    int Nsyst;
    vector<NPContainer> NPvecForName;
    TIterator *iter1 = channelCat->typeIterator() ;
    RooCatType *tt1  = NULL;
    while((tt1=(RooCatType*) iter1->Next()) ){
      Nchannel++;
      if (Nchannel==1) NPvecForName = MapChannelNPs[(TString)tt1->GetName()];
    }
    Nsyst = NPvecForName.size();
    for (int isys=0 ; isys<Nsyst ; isys++){
      TString hname = NPvecForName[isys].NPname;
      hname.ReplaceAll("\\_","_");

      TH1F * hsys_axis = new TH1F(hname,hname,Nchannel,0,Nchannel);
      TGraphAsymmErrors *hsys = new TGraphAsymmErrors(Nchannel);
      hsys_axis->GetYaxis()->SetRangeUser(-5.5,5.5);
      hsys_axis->GetXaxis()->SetTitle("#theta");
      hsys_axis->GetYaxis()->SetTitle("(#theta_{fit} - #theta_{0}) / #Delta#theta");

      TString cname = "can_"+ hname;
      cname.ReplaceAll("#","");
      cname.ReplaceAll("(","");
      cname.ReplaceAll(")","");
      cname.ReplaceAll("=","");
      TCanvas* can = new TCanvas( cname, cname , 600, 500);

      TIterator *iter2 = channelCat->typeIterator() ;
      RooCatType *tt2  = NULL;
      int ich=0;
      while((tt2=(RooCatType*) iter2->Next()) ){
        ich++;
        vector<NPContainer> NPvec = MapChannelNPs[(TString)tt2->GetName()];
        double NPval   = NPvec[isys].NPvalue;
        double NPerrHi = NPvec[isys].NPerrorHi;
        double NPerrLo = NPvec[isys].NPerrorLo;

        double xc = hsys_axis->GetBinCenter(ich);
        hsys->SetPoint(ich-1,xc,NPval);
        hsys->SetPointEXlow(ich-1,hsys_axis->GetBinWidth(ich)/3.0);
        hsys->SetPointEXhigh(ich-1,hsys_axis->GetBinWidth(ich)/3.0);
        hsys->SetPointEYlow(ich-1,fabs(NPerrLo));
        hsys->SetPointEYhigh(ich-1,fabs(NPerrHi));
        hsys_axis->GetXaxis()->SetBinLabel(ich,tt2->GetName());
      }

      double _1SigmaValue[1000];
      double _2SigmaValue[1000];
      double NuisParamValue[1000];
      for (int i=0 ; i<Nchannel+1 ; i++){
        _1SigmaValue[i] = 1.0;
        _1SigmaValue[2*Nchannel-i] = -1;
        _2SigmaValue[i] = 2;
        _2SigmaValue[2*Nchannel-i] = -2;
        NuisParamValue[i] = i;
        NuisParamValue[2*Nchannel-1-i] = i;
      }

      TGraph *_1sigma = new TGraph(2*Nchannel,NuisParamValue,_1SigmaValue);
      TGraph *_2sigma = new TGraph(2*Nchannel,NuisParamValue,_2SigmaValue);
      can->cd();
      hsys->SetLineWidth(2);
      hsys->SetLineColor(1);
      hsys->SetMarkerColor(1);
      hsys->SetMarkerStyle(21);
      hsys->SetMarkerSize(1.2);
      _2sigma->SetFillColor(5);
      _2sigma->SetLineColor(5);
      _2sigma->SetMarkerColor(5);
      _1sigma->SetFillColor(3);
      _1sigma->SetLineColor(3);
      _1sigma->SetMarkerColor(3);
      hsys_axis->GetYaxis()->SetRangeUser(-5.0,5.0);
      hsys_axis->Draw("hist");
      if (NPvecForName[isys].NPname != "#mu"){
        _2sigma->Draw("F");
        _1sigma->Draw("F");
      }
      hsys->Draw("P");
      MainDir->cd();
      can->Write();
      can->Close();
      gROOT->cd();
    }

    return;
  }


  void PlotHistosAfterFitGlobal(bool IsConditionnal, double mu, bool isAsimov){
    // isAsimov switch canvas names and the like
    TString globOrAsim;
    if(isAsimov)
      globOrAsim = "Asimov";
    else
      globOrAsim = "Global";

    cout << "Global Fit ";
    if(IsConditionnal) { cout << "conditional " << endl; } else { cout << "unconditional " << endl; }
    cout << " mu = " << mu;
    if(isAsimov) { cout << " Asimov" << endl; }

    // Conditionnal or unconditional fit
    TString TS_IsConditionnal;
    if (IsConditionnal) TS_IsConditionnal="conditionnal";
    else                TS_IsConditionnal="unconditionnal";

    RooRealVar * firstPOI = dynamic_cast<RooRealVar*>(mc->GetParametersOfInterest()->first());
    if(firstPOI && IsConditionnal) { firstPOI->setVal(mu); }
    else {
      if(!firstPOI && (!IsConditionnal || (IsConditionnal && mu != 0)) ) { cout << "Cannot find POI" << endl; exit(-1); }
    }

    ostringstream MaindirName;
    if (IsConditionnal) MaindirName << TS_IsConditionnal << "_MuIsEqualTo_" << mu;
    else                MaindirName << TS_IsConditionnal;
    TDirectory *MainDir =  0;
    if(isAsimov) {
      MainDir = (TDirectory*) MainDirFitAsimov->mkdir(MaindirName.str().c_str());
    }
    else {
      MainDir = (TDirectory*) MainDirFitGlobal->mkdir(MaindirName.str().c_str());
    }
    gROOT->cd();

    // Get the RooSimultaneous PDF
    RooSimultaneous *simPdf = (RooSimultaneous*)(mc->GetPdf());

    TString snapshotName("snapshot_paramsVals_"+globOrAsim+"Fit");
    if(IsConditionnal) { snapshotName.Append("_mu"+TString(Form("%4.2f",mu))); }
    else { snapshotName.Append("_unCond"); }

    RooAbsData* localData = 0;

    // make Asimov Data if needed
    if(isAsimov) {
      if(firstPOI) { firstPOI->setVal(mu); }
      RooArgSet *  allParams = mc->GetPdf()->getParameters(*data);
      RooStats::RemoveConstantParameters( allParams );
      RooArgSet globObs("globObs");

      RooAbsData* asimov_data = AsymptoticCalculator::MakeAsimovData(*mc, *allParams, globObs);
      localData = asimov_data;
    }
    else {
      localData = data;
    }

    // kick in Asimov fits
    if (!IsConditionnal && firstPOI && isAsimov) firstPOI->setVal(1.1); // kick muhat in Asimov fits
    if (IsConditionnal && firstPOI && isAsimov) {
      const RooArgSet *np = mc->GetNuisanceParameters();
      RooRealVar* var = (RooRealVar*)(np->first());
      var->setVal(var->getVal() + 0.1);
    }

    // Fit
    if (IsConditionnal && firstPOI) firstPOI->setConstant();
//    ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
//    ROOT::Math::MinimizerOptions::SetDefaultStrategy(2);
    RooFitResult    *fitresGlobal  = 0;
    if(!w->loadSnapshot(snapshotName)) {
      fitresGlobal = FitPDF( mc, simPdf, localData, "Minuit2" );
      const RooArgSet *ParaGlobalFit = mc->GetNuisanceParameters();
      w->saveSnapshot(snapshotName,*ParaGlobalFit);
      const RooArgSet* obsSet = mc->GetObservables();
      RooArgList floatPars = getFloatParList( *simPdf, *obsSet);
    }
    // do something witht eh snapshot
//    else {
//      cout << "No snapshot called " << snapshotName << endl;
//      exit(-1);
//    }
    //const RooArgSet *ParaGlobalFit = mc->GetNuisanceParameters();

    // write fit result to file
    MainDir->cd();
    fitresGlobal->Write("fitResult");
    gROOT->cd();

    double muhat = 0;
    if(firstPOI) {
      muhat = firstPOI->getVal();
      //firstPOI->setConstant(kFALSE);
    }


    if (IsConditionnal) cout << "Conditionnal fit : mu is fixed at " << mu << endl;
    else                cout << "Unconditionnal fit : mu is fitted" << endl;

    // PLotting the nuisance paramaters correlations during the fit
    TString cname = "can_CorrMatrix_"+globOrAsim+"Fit_" + TS_IsConditionnal + "_mu";
    cname += mu;

    cout << "==========================   " << cname << endl;

    //TCanvas* c1 = new TCanvas( cname, cname, 1260, 500);
    TCanvas* c1 = new TCanvas( cname );
    c1->SetLeftMargin( 2.0 * c1->GetLeftMargin() );
    c1->SetBottomMargin( 2.0 * c1->GetBottomMargin() );
    //c1->Divide(2,1);
    TH2D *h2Dcorrelation = (TH2D*) fitresGlobal->correlationHist();
    TString hname = "Corr_CorrMatrix_"+globOrAsim+"Fit_" + TS_IsConditionnal + "_mu";
    h2Dcorrelation->SetName(hname);
    h2Dcorrelation->GetXaxis()-> LabelsOption("v");
    //c1->cd(1);
    h2Dcorrelation->Draw("colz");
    MainDir->cd();
    c1->Write();
    gROOT->cd();

    //    cout << "EIGEN" << endl;
    //    const TMatrixDSym corrMatrix = fitresGlobal->correlationMatrix();
    //    TVectorT<double>* eigenVals;
    //    corrMatrix.EigenVectors( *eigenVals );

    // PLotting the nuisance paramaters correlations during the fit
    cname.ReplaceAll("CorrMatrix","NuisPara");
    TCanvas* c1a = new TCanvas( cname );
    TString h1name = "h_NuisPara_"+globOrAsim+"Fit_" + TS_IsConditionnal + "_mu";
    h1name += mu;
    TIterator* it1 = mc->GetNuisanceParameters()->createIterator();
    RooRealVar* var = NULL;
    int Npar=0;
    int NparNotStat=0;
    while( (var = (RooRealVar*) it1->Next()) ) {
      Npar++;
      string varname = (string) var->GetName();
      if (varname.find("gamma_stat")==string::npos) NparNotStat++;
    }

    TH1F * h1Dpull_axis = new TH1F(h1name,h1name,NparNotStat,0,NparNotStat);
    TGraphAsymmErrors *h1Dpull = new TGraphAsymmErrors(NparNotStat);
    h1Dpull_axis->GetYaxis()->SetRangeUser(-5.5,5.5);
    h1Dpull_axis->GetXaxis()->SetTitle("#theta");
    h1Dpull_axis->GetYaxis()->SetTitle("(#theta_{fit} - #theta_{0}) / #Delta#theta");

    // Create a latex table of NPs after fit
    TString fname = OutputDir + "/LatexFileNPs/"+globOrAsim+"Fit_nuisPar_"+TS_IsConditionnal+"_mu";
    fname += mu;
    fname += ".tex";
    ofstream fnuisPar(fname.Data());
    TString fnuiscorr = OutputDir + "/TextFileFitResult/"+globOrAsim+"Fit_fitres_"+TS_IsConditionnal+"_mu";
    fnuiscorr += mu;
    fnuiscorr += ".txt";
    ofstream fnuisParAndCorr(fnuiscorr.Data());
    fnuisParAndCorr << "NUISANCE_PARAMETERS" << endl;

    fnuisPar << endl;
    fnuisPar << "\\begin{tabular}{|l|c|}" << endl;
    fnuisPar << "\\hline" << endl;
    fnuisPar << "Nuisance parameter & postfit value (in $\\sigma$ unit) \\\\\\hline" << endl;

    if(!IsConditionnal){
      fnuisPar.precision(3);
      fnuisPar << "$\\mu$ & $" << firstPOI->getVal() << "^{+" << firstPOI->getErrorHi() << "}_{-" << fabs(firstPOI->getErrorLo()) << "}$ \\\\" << endl;
    }

    vector<TGraphAsymmErrors*> vec_MyGraph;
    vec_MyGraph.clear();
    int ib=0;
    TIterator* it2 = mc->GetNuisanceParameters()->createIterator();
    while( (var = (RooRealVar*) it2->Next()) ){

      // Not consider nuisance parameter being not associated to syst
      string varname = (string) var->GetName();
      if ((varname.find("gamma_stat")!=string::npos)) continue;

      double pull  = var->getVal() / 1.0 ; // GetValue() return value in unit of sigma
      double errorHi = var->getErrorHi() / 1.0;
      double errorLo = var->getErrorLo() / 1.0;

      if(strcmp(var->GetName(),"Lumi")==0){
        pull  = (var->getVal() - w->var("nominalLumi")->getVal() ) / (w->var("nominalLumi")->getVal() * LumiRelError );
        errorHi = var->getErrorHi() / (w->var("nominalLumi")->getVal() * LumiRelError);
        errorLo = var->getErrorLo() / (w->var("nominalLumi")->getVal() * LumiRelError);
      }

      TString vname=var->GetName();
      vname.ReplaceAll("alpha_","");
      vname.ReplaceAll("Lumi","Luminosity");
      vname.ReplaceAll("_","\\_");
      fnuisPar.precision(3);
      fnuisPar << vname << " & $" << pull << "^{+" << fabs(errorHi) << "}_{-" << fabs(errorLo)  << "}$ \\\\" << endl;
      fnuisParAndCorr << vname << " & $" << pull << "^{+ " << fabs(errorHi) << "}_{-" << fabs(errorLo)  << "}$ \\\\" << endl;

      NPContainer MyNPsTemp;
      MyNPsTemp.NPname  = vname;
      MyNPsTemp.NPvalue = pull;
      MyNPsTemp.NPerrorHi = errorHi;
      MyNPsTemp.NPerrorLo = errorLo;
      MyNPsTemp.WhichFit = TS_IsConditionnal+"_Fit"+globOrAsim+"_Mu";
      MyNPsTemp.WhichFit += mu;
      AllNPafterEachFit.push_back(MyNPsTemp);

      ib++;
      double xc = h1Dpull_axis->GetBinCenter(ib);
      TString vname2=var->GetName();
      vname2.ReplaceAll("alpha_","");
      vname2.ReplaceAll("gamma_","");
      vname2.ReplaceAll("ATLAS_","");
      h1Dpull_axis->GetXaxis()->SetBinLabel(ib,vname2);
      h1Dpull->SetPoint(ib-1,xc,pull);
      h1Dpull->SetPointEXlow(ib-1,h1Dpull_axis->GetBinWidth(ib)/3.0);
      h1Dpull->SetPointEXhigh(ib-1,h1Dpull_axis->GetBinWidth(ib)/3.0);
      h1Dpull->SetPointEYlow(ib-1,fabs(errorLo));
      h1Dpull->SetPointEYhigh(ib-1,fabs(errorHi));

      // Put in red NP with pull>1.5 OR Err<0.2, in black otherwise
      bool IsTooPulled = fabs(pull)>PullMaxAcceptable;
      bool IsOverConst = (fabs(errorLo)+fabs(errorHi))/2.0<ErrorMinAcceptable;
      bool IsOnesided  = UseMinosError && (errorHi==0 || errorLo==0);
      if (IsTooPulled || IsOverConst || IsOnesided){
        TGraphAsymmErrors *myGraph = new TGraphAsymmErrors(1);
        myGraph->SetName("TGraph_"+vname2);
        myGraph->SetLineWidth(2);
        myGraph->SetLineColor(kRed+1);
        myGraph->SetMarkerColor(kRed+1);
        myGraph->SetMarkerStyle(21);
        myGraph->SetMarkerSize(1.2);
        myGraph->SetPoint(0,xc,pull);
        myGraph->SetPointEXlow(0,h1Dpull_axis->GetBinWidth(ib)/3.0);
        myGraph->SetPointEXhigh(0,h1Dpull_axis->GetBinWidth(ib)/3.0);
        myGraph->SetPointEYlow(0,fabs(errorLo));
        myGraph->SetPointEYhigh(0,fabs(errorHi));
        vec_MyGraph.push_back(myGraph);
      }
    }

    fnuisPar << "\\hline" << endl;
    fnuisPar << "\\end{tabular}" << endl;
    fnuisPar.close();

    fnuisParAndCorr << endl << endl << "CORRELATION_MATRIX" << endl;
    fnuisParAndCorr << h2Dcorrelation->GetNbinsX() << "   " << h2Dcorrelation->GetNbinsY() << endl;
    for(int kk=1; kk < h2Dcorrelation->GetNbinsX()+1; kk++) {
      for(int ll=1; ll < h2Dcorrelation->GetNbinsY()+1; ll++) {
        fnuisParAndCorr << h2Dcorrelation->GetBinContent(kk,ll) << "   ";
      }
      fnuisParAndCorr << endl;
    }
    fnuisParAndCorr << endl;
    fnuisParAndCorr.close();

    double _1SigmaValue[1000];
    double _2SigmaValue[1000];
    double NuisParamValue[1000];
    for (int i=0 ; i<NparNotStat+1 ; i++){
      _1SigmaValue[i] = 1.0;
      _1SigmaValue[2*NparNotStat-i] = -1;
      _2SigmaValue[i] = 2;
      _2SigmaValue[2*NparNotStat-i] = -2;
      NuisParamValue[i] = i;
      NuisParamValue[2*NparNotStat-1-i] = i;
    }
    TGraph *_1sigma = new TGraph(2*NparNotStat,NuisParamValue,_1SigmaValue);
    TGraph *_2sigma = new TGraph(2*NparNotStat,NuisParamValue,_2SigmaValue);
    //c1->cd(2);
    c1a->SetLeftMargin( 2.0 * c1a->GetLeftMargin() );
    c1a->SetBottomMargin( 2.0 * c1a->GetBottomMargin() );
    c1a->cd();
    h1Dpull->SetLineWidth(2);
    h1Dpull->SetLineColor(1);
    h1Dpull->SetMarkerColor(1);
    h1Dpull->SetMarkerStyle(21);
    h1Dpull->SetMarkerSize(1.2);
    _2sigma->SetFillColor(5);
    _2sigma->SetLineColor(5);
    _2sigma->SetMarkerColor(5);
    _1sigma->SetFillColor(3);
    _1sigma->SetLineColor(3);
    _1sigma->SetMarkerColor(3);
    h1Dpull_axis->Draw("hist");
    _2sigma->GetXaxis()->LabelsOption("v");
    _1sigma->GetXaxis()->LabelsOption("v");
    h1Dpull->GetXaxis()->LabelsOption("v");
    _2sigma->Draw("F");
    _1sigma->Draw("F");
    h1Dpull->Draw("P");
    for (unsigned i=0 ; i<vec_MyGraph.size() ; i++) vec_MyGraph[i]->Draw("P");
    h1Dpull_axis->GetYaxis()->DrawClone();


    TLatex text;
    text.SetNDC();
    text.SetTextSize( 0.054);
    text.SetTextAlign(31);
    TString WritDownMuValue;
    if(!IsConditionnal) WritDownMuValue = "#mu_{best} = ";
    else                WritDownMuValue = "#mu_{fixed} = ";
    if(firstPOI) {
      WritDownMuValue += Form("%2.2f",firstPOI->getVal());
    }
    //c1a->cd(2);
    if(!blind) { text.DrawLatex( 0.87,0.81, WritDownMuValue ); }

    MainDir->cd();
    c1a->Write();
    gROOT->cd();

    if(draw1DResponse) {
      bool doSingleCategories = false;
      // Plotting the likelihood projection in each NP direction
      cout << "Plotting 1D Response for each NP" << endl;
      TDirectory *NLLprojection = (TDirectory*) MainDir->mkdir("AllNNLProjections");
      gROOT->cd();
      TDirectory *maxCorrNPs = (TDirectory*) MainDir->mkdir("AllMaxCorrNPs");
      gROOT->cd();
      RooAbsReal* nll = simPdf->createNLL(*localData);

      TLatex* latex = new TLatex();
      latex->SetNDC(); latex->SetTextSize(0.055); latex->SetTextAlign(32);
      //TF1* poly5 = new TF1("poly5","[0]+[1]*x+[2]*x*x",-5,5);
      //TF1* poly2 = new TF1("poly2","[0]+[1]*x+[2]*x*x",0,10);
      TCanvas* can2 = new TCanvas("NLLscan");
      list< pair<RooRealVar*, float> > pairs;
      TH2D* redMat = NULL;

      if (!IsConditionnal && iJob == 0){
        var = (RooRealVar*) firstPOI;

        // find NPs with highest correlation
        pairs.clear();
        pairs = GetOrderedCorrelations(var, fitresGlobal);
        redMat = GetSubsetOfCorrMatrix(var, pairs, fitresGlobal, 4);
        can2->SetTitle(redMat->GetName());
        can2->SetName(redMat->GetName());
        can2->cd();
        redMat->Draw("colz text");
        maxCorrNPs->cd();
        TDirectory* saveDir = maxCorrNPs->GetDirectory(var->GetName());
        if(!saveDir) { saveDir = maxCorrNPs->mkdir(var->GetName()); }
        saveDir->cd();
        can2->Write(redMat->GetName());
        gROOT->cd();

        //Plot1DResponse(nll, var, "", can2, poly2, true, latex, NLLprojection);
        Plot1DResponseNew(nll, var, "", can2, true, latex, NLLprojection, snapshotName);

        RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
        TIterator *iter = channelCat->typeIterator() ;
        RooCatType *tt  = NULL;
        while((tt=(RooCatType*) iter->Next()) && doSingleCategories ){
          TString chanName(tt->GetName());
	  cout << "Category: " << chanName << endl;
          RooAbsPdf  *pdftmp  = simPdf->getPdf( tt->GetName() );
          RooAbsData *datatmp = localData->reduce(Form("%s==%s::%s",channelCat->GetName(),channelCat->GetName(),tt->GetName()));
          RooAbsReal* catNll = pdftmp->createNLL(*datatmp);
          if(!catNll->dependsOn(*var)) { continue; }
          //Plot1DResponse(catNll, var, chanName, can2, poly2, true, latex, NLLprojection);
          Plot1DResponseNew(catNll, var, chanName, can2, true, latex, NLLprojection, snapshotName);
        }

      } // if is *not* conditional fit
      //return;

      TIterator* it3 = mc->GetNuisanceParameters()->createIterator();
      //TIterator* it3 = mc->GetParametersOfInterest()->createIterator();
      int countNP = 0; // index 0 is POI
      while( (var = (RooRealVar*) it3->Next()) ){
	countNP++;
	if ( countNP % nJobs != iJob ) continue;
        TString vname=var->GetName();
        if (vname.Contains("gamma_stat")) { continue; }
        //if (!vname.Contains("ATLAS_norm")) { continue; }
        bool floating(false);
        if(vname.Contains("ATLAS_norm")) { floating = true; }

        /* if want to make a slice instead of a projection
           RooArgSet* SliceNPs = new RooArgSet( *(mc->GetNuisanceParameters()), TString("sliceNP_")+TString(var->GetName()));
           SliceNPs->remove(*var, true, true);
           */

        // find NPs with highest correlation
        pairs.clear();
        pairs = GetOrderedCorrelations(var, fitresGlobal);
        redMat = GetSubsetOfCorrMatrix(var, pairs, fitresGlobal, 4);
        can2->SetTitle(redMat->GetName());
        can2->SetName(redMat->GetName());
        can2->cd();
        redMat->Draw("colz text");
        maxCorrNPs->cd();
        TDirectory* saveDir = maxCorrNPs->GetDirectory(var->GetName());
        if(!saveDir) { saveDir = maxCorrNPs->mkdir(var->GetName()); }
        saveDir->cd();
        can2->Write(redMat->GetName());
        gROOT->cd();

        Plot1DResponseNew(nll, var, "", can2, floating, latex, NLLprojection, snapshotName);
        if(floating) {
          //Plot1DResponse(nll, var, "", can2, poly2, floating, latex, NLLprojection);
        } else {
          //Plot1DResponse(nll, var, "", can2, poly5, floating, latex, NLLprojection);
        }

        RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
        TIterator *iter = channelCat->typeIterator() ;
        RooCatType *tt  = NULL;
        while((tt=(RooCatType*) iter->Next()) && doSingleCategories ){
          //cout << " -- On category " << tt->GetName() << " " << endl;
          TString chanName(tt->GetName());
          if(!chanName.Contains("B0")) { continue; }
          RooAbsPdf  *pdftmp  = simPdf->getPdf( tt->GetName() );
          RooAbsData *datatmp = localData->reduce(Form("%s==%s::%s",channelCat->GetName(),channelCat->GetName(),tt->GetName()));
          RooAbsReal* catNll = pdftmp->createNLL(*datatmp);
          if(!catNll->dependsOn(*var)) { continue; }
          Plot1DResponseNew(catNll, var, chanName, can2, floating, latex, NLLprojection, snapshotName);
          if(floating) {
            //Plot1DResponse(catNll, var, chanName, can2, poly2, floating, latex, NLLprojection);
          } else {
            //Plot1DResponse(catNll, var, chanName, can2, poly5, floating, latex, NLLprojection);
          }
        } // loop over sub channels
      } // loop over NPs
      delete can2;
      cout << "Finished Plotting 1D Response for each NP" << endl;
    }

    if(!makePostFitPlots) { return; }

    cout << "Plotting Distributions for each subchannel" << endl;

    // Plotting the distributions for each subchannel
    RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
    TIterator *iter = channelCat->typeIterator() ;
    RooCatType *tt  = NULL;
    vector<double> Chi2Channel;Chi2Channel.clear();
    vector<TString> NameChannel;NameChannel.clear();
    TString dirName(OutputDir+"/PlotsAfter"+globOrAsim+"Fit");
    if(drawPlots) { system(TString("mkdir -vp "+dirName)); }

    TString nFileName = OutputDir+"/globNorm_"+TS_IsConditionnal;
    if(IsConditionnal) { nFileName += "_mu"; nFileName += mu; }
    nFileName += ".txt";
    ofstream normFile(nFileName.Data());
    normFile << "Normalizations - Per Channel and Global" << endl << endl;

    map<TString, float> preFits;
    map<TString, float> postFits;

    while((tt=(RooCatType*) iter->Next()) ){

      RooAbsPdf  *pdftmp  = simPdf->getPdf( tt->GetName() );
      RooAbsData *datatmp = localData->reduce(Form("%s==%s::%s",channelCat->GetName(),channelCat->GetName(),tt->GetName()));
      RooArgSet  *obstmp  = pdftmp->getObservables( *mc->GetObservables() ) ;
      RooRealVar *obs     = ((RooRealVar*) obstmp->first());

      // Bin Width
      RooRealVar* binWidth = ((RooRealVar*) pdftmp->getVariables()->find(Form("binWidth_obs_x_%s_0",tt->GetName()))) ;
      if(!binWidth) { cout << "No bin width " << tt->GetName() << endl; }
      cout << "    Bin Width : " << binWidth->getVal() << endl;

      map<TString, float> preFitsChan;
      map<TString, float> postFitsChan;

      // Load the value from the global fit
      if(!w->loadSnapshot(snapshotName)) {
        cout << "Cannot load " <<  snapshotName << endl;
        exit(-1);
      }
      if(firstPOI) {
        if   (!IsConditionnal) firstPOI->setVal(muhat);
        else                   firstPOI->setVal(mu);
      }

      TString modelName(tt->GetName());
      modelName.Append("_model");
      RooRealSumPdf *pdfmodel = (RooRealSumPdf*) (pdftmp->getComponents())->find(modelName);
      RooArgList funcList =  pdfmodel->funcList();
      RooProduct* comp = 0;
      RooLinkedListIter funcIter = funcList.iterator() ;
      cout << "Post Fit " << endl;
      while( (comp = (RooProduct*) funcIter.Next()) ) {
        cout << "\t" << comp->GetName() << "\t" << (comp->createIntegral(*obs))->getVal() * binWidth->getVal() << endl;
      }

      cname = "can_DistriAfterFit_"+ (TString) tt->GetName() +"_"+globOrAsim+"Fit_" + TS_IsConditionnal + "_mu";
      cname += mu;
      TCanvas* c2 = new TCanvas( cname );
      RooPlot* frame = obs->frame();
      TString FrameName = "Plot_Distri"+globOrAsim+"_" + (TString) IsConditionnal;
      frame->SetName( FrameName );
      frame->SetYTitle("EVENTS");

      float postFitIntegral = pdftmp->expectedEvents(*obs);
      pdftmp->plotOn(frame,FillColor(kOrange),LineWidth(2),LineColor(kBlue),VisualizeError(*fitresGlobal,1),
                     Normalization(postFitIntegral,RooAbsReal::NumEvent),Name("FitError"));
      pdftmp->plotOn(frame,LineWidth(2),Normalization(postFitIntegral,RooAbsReal::NumEvent),Name("CentralFit"));
      if(blind) { // zero out the signal region
        TString histName(tt->GetName());
        histName.Append("_data");
        TH1* hdata = datatmp->createHistogram(histName,*obs);
        if(hdata->GetNbinsX()>1) {
          float width(hdata->GetBinWidth(1));
          for(int b=1; b<hdata->GetNbinsX()+1; b++) {
            hdata->SetBinError(b, sqrt(hdata->GetBinContent(b)));
            if( (hdata->GetBinLowEdge(b) > 100 || (hdata->GetBinLowEdge(b)+width) > 100) && hdata->GetBinLowEdge(b) < 150) {
              hdata->SetBinContent(b,0);
              hdata->SetBinError(b,0);
            }
          }
        }
        hdata->SetMarkerSize(2);
        frame->addTH1(hdata);
      } else {
        datatmp->plotOn(frame,MarkerSize(1),Name("Data"),DataError(RooAbsData::Poisson));
      }
      double chi2 = frame->chiSquare();
      Chi2Channel.push_back( chi2 );
      NameChannel.push_back( (TString)tt->GetName() );

      /* Total Prefit distribution */
      // Putting nuisance parameter at the central value and draw the nominal distri
      if(!w->loadSnapshot("snapshot_paramsVals_initial")) {
        cout << "Cannot load " <<  "snapshot_paramsVals_initial" << endl;
        exit(-1);
      }
      if(firstPOI) { SetPOI(1.0); }
      float preFitIntegral = pdftmp->expectedEvents(*obs);
      pdftmp->plotOn(frame,LineWidth(2.),Name("BeforeFit"),LineStyle(kDashed),Normalization(preFitIntegral,RooAbsReal::NumEvent));

      c2->cd();
      frame->Draw();
      MainDir->cd();
      c2->Write();

      delete frame;

      RooPlot* frm = obs->frame();
      FrameName = "Plot_Distri"+globOrAsim+"_" + (TString) IsConditionnal + TString("_stack");
      frm->SetName( FrameName );
      frm->SetYTitle("EVENTS");

      // Load the value from the global fit
      if(!w->loadSnapshot(snapshotName)) {
        cout << "Cannot load " <<  snapshotName << endl;
        exit(-1);
      }
      if(firstPOI) {
        if   (!IsConditionnal) firstPOI->setVal(muhat);
        else                   firstPOI->setVal(mu);
      }


      // miniloop over componant to make a bkg stack
      TString modelName1(tt->GetName());
      modelName1.Append("_model");
      RooRealSumPdf *pdfmodel1 = (RooRealSumPdf*) (pdftmp->getComponents())->find(modelName1);
      RooArgList funcList1 =  pdfmodel1->funcList();
      RooLinkedListIter funcIter1 = funcList1.iterator() ;
      RooProduct* comp1 = 0;
      if(firstPOI) {
        if (!IsConditionnal) firstPOI->setVal(muhat);
        else                 firstPOI->setVal(mu);
      }
      int ibkg=0;
      //int icolor=0;
      TString previous="";
      while( (comp1 = (RooProduct*) funcIter1.Next()) ) {
        ibkg++;

        int color=kGray;

        TString compname(comp1->GetName());
        compname.ReplaceAll("L_x_","");
        compname.ReplaceAll(tt->GetName(),"");
        compname.ReplaceAll("_overallSyst_x_StatUncert","");
        compname.ReplaceAll("_overallSyst_x_HistSyst","");
        compname.ReplaceAll("_overallSyst_x_Exp","");
        compname.ReplaceAll("_","");

        /*
           icolor++;
           if(icolor==1) { color = kOrange - 3; }
           else if(icolor==2) { color = kCyan + 1; }
           else if(icolor==3) { color = kGreen - 9; }
           else if(icolor==4) { color = kAzure - 9; }
           else if(icolor==5) { color = kOrange + 10;}
           else if(icolor==6) { color = kGreen - 6; }
           else if(icolor==7) { color = kAzure - 4; }
           else if(icolor==8) { color = kOrange + 6; }
           else if(icolor==9) { color = kGreen + 1; }
           else if(icolor==10) { color = kAzure + 2; }
           else if(icolor==11) { color = kOrange; }
           else if(icolor==12) { color = kGreen + 3; }
           else if(icolor==13) { color = kAzure - 4; }
           else if(icolor==14) { color = kOrange; }
           else if(icolor==15) { color = kGreen + 1; }
           else if(icolor==16) { color = kOrange - 7; }
           else if(icolor==17) { color = kPink + 1; }
           else   color=icolor;
           */

        color = kBlack;
        if(compname.Contains("DibosonSign"))    { color = kRed; }
        else if(compname.Contains("WH"))   { color = kRed; }
        else if(compname.Contains("ZH"))   { color = kRed; }
        else if(compname.Contains("Diboson"))   { color = kOrange - 3; }
        else if(compname.Contains("WW"))        { color = kOrange + 7; }
        else if(compname.Contains("WZ"))        { color = kOrange + 10;}
        else if(compname.Contains("ZZ"))        { color = kOrange + 6; }
        else if(compname.Contains("Top"))       { color = kOrange;     }
        else if(compname.Contains("ttbar"))     { color = kOrange;     }
        else if(compname.Contains("stop"))      { color = kOrange - 7; }
        else if(compname.Contains("Wl"))        { color = kGreen - 9;  }
        else if(compname.Contains("Wc"))        { color = kGreen - 6;  }
        else if(compname.Contains("Wcc"))       { color = kGreen + 1;  }
        else if(compname.Contains("Wb"))        { color = kGreen + 3;  }
        else if(compname.Contains("W"))         { color = kGreen + 1;  }
        else if(compname.Contains("Zl"))        { color = kAzure - 9;  }
        else if(compname.Contains("Zc"))        { color = kAzure - 4;  }
        else if(compname.Contains("Zb"))        { color = kAzure + 2;  }
        else if(compname.Contains("Z"))         { color = kAzure - 4;  }
        else if(compname.Contains("multijet"))  { color = kPink + 1;   }
        else { cout << "NO color for  " << compname << endl; exit(-1); }

        // Get the signal x 1 in white (for further purposes)
        if (IsConditionnal && firstPOI) firstPOI->setVal(1.0);
        double Ntemp=(comp1->createIntegral(*obs))->getVal() * binWidth->getVal();
        //pdfmodel1->plotOn(frame,LineWidth(0),Components(*comp1),LineColor(0), LineStyle(3), Normalization(Ntemp,RooAbsReal::NumEvent),Name("NoStacked_"+compname));
        if (IsConditionnal && firstPOI) firstPOI->setVal(mu);


        if(preFits.find(compname) == preFits.end()) {
          preFits[compname]  = 0.;
          postFits[compname] = 0.;
        }
        if(preFitsChan.find(compname) == preFitsChan.end()) {
          preFitsChan[compname]  = 0.;
          postFitsChan[compname] = 0.;
        }

        // Stack bkg
        Ntemp=(comp1->createIntegral(*obs))->getVal() * binWidth->getVal();

        postFits[compname] += Ntemp;
        postFitsChan[compname] += Ntemp;

        //        if (ibkg==0) pdfmodel1->plotOn(frame,LineWidth(2),Components(*comp1),LineColor(color), Normalization(Ntemp,RooAbsReal::NumEvent),Name("Stacked_"+compname));
        //        else         pdfmodel1->plotOn(frame,LineWidth(2),Components(*comp1),LineColor(color), Normalization(Ntemp,RooAbsReal::NumEvent),Name("Stacked_"+compname),AddTo(previous));
        if (previous.Length()==0) { pdfmodel1->plotOn(frm,LineWidth(2),Components(*comp1),LineColor(color), Normalization(Ntemp,RooAbsReal::NumEvent),FillColor(color),DrawOption("F"),FillStyle(1001),Name("Stacked_"+compname)); }
        else         { pdfmodel1->plotOn(frm,LineWidth(2),Components(*comp1),LineColor(color), Normalization(Ntemp,RooAbsReal::NumEvent),FillColor(color),DrawOption("F"),FillStyle(1001),Name("Stacked_"+compname),MoveToBack(),AddTo(previous)); }
        previous="Stacked_"+compname;


      }

      if(firstPOI) {
        if (!IsConditionnal) firstPOI->setVal(muhat);
        else                 firstPOI->setVal(mu);
      }
      pdftmp->plotOn(frm,FillColor(kBlue),LineWidth(2),LineColor(kBlue),VisualizeError(*fitresGlobal,1),
                     Normalization(postFitIntegral,RooAbsReal::NumEvent),Name("FitError_AfterFit"));
      //pdftmp->plotOn(frm,LineWidth(2),Normalization(postFitIntegral,RooAbsReal::NumEvent),Name("FitCentral2_NotAppears"));
      if(blind) { // zero out the signal region
        TString histName(tt->GetName());
        histName.Append("_data");
        TH1* hdata = datatmp->createHistogram(histName,*obs);
        if(hdata->GetNbinsX()>1) {
          float width(hdata->GetBinWidth(1));
          for(int b=1; b<hdata->GetNbinsX()+1; b++) {
            hdata->SetBinError(b, sqrt(hdata->GetBinContent(b)));
            if( (hdata->GetBinLowEdge(b) > 100 || (hdata->GetBinLowEdge(b)+width) > 100) && hdata->GetBinLowEdge(b) < 150) {
              hdata->SetBinContent(b,0);
              hdata->SetBinError(b,0);
            }
          }
        }
        hdata->SetMarkerSize(2);
        frm->addTH1(hdata);
      } else {
        datatmp->plotOn(frm,MarkerSize(1),Name("Data"),DataError(RooAbsData::Poisson));
      }

      // HERE

      preFits["data"] += datatmp->sumEntries();
      postFits["data"] += datatmp->sumEntries();
      preFitsChan["data"] += datatmp->sumEntries();
      postFitsChan["data"] += datatmp->sumEntries();

      // Putting nuisance parameter at the central value and draw the nominal distri
      SetAllStatErrorToSigma(0.0);
      SetAllNuisanceParaToSigma(0.0);
      if(!w->loadSnapshot("snapshot_paramsVals_initial")) {
        cout << "Cannot load " <<  "snapshot_paramsVals_initial" << endl;
        exit(-1);
      }
      if(firstPOI) { SetPOI(1.0); }
      //if (!IsConditionnal) SetPOI(0.0);
      //else                 SetPOI(mu);
      float mubeforefit(0);
      if(firstPOI) { mubeforefit = firstPOI->getVal(); }
      TString muValueBeforeFitLegend = Form("Before fit (#mu=%2.2f)",mubeforefit);
      funcIter = funcList.iterator() ;
      cout << "Pre Fit " << endl;
      while( (comp = (RooProduct*) funcIter.Next()) ) {
        cout << "\t" << comp->GetName() << "\t" << (comp->createIntegral(*obs))->getVal() * binWidth->getVal() << endl;
      }
      preFitIntegral = pdftmp->expectedEvents(*obs);
      pdftmp->plotOn(frm,LineWidth(2),Name("BeforeFit"),LineStyle(kDashed),Normalization(preFitIntegral,RooAbsReal::NumEvent));
      c2->cd();
      frm->Draw();
      c2->cd();

      TString modelName2(tt->GetName());
      modelName2.Append("_model");
      RooRealSumPdf *pdfmodel2 = (RooRealSumPdf*) (pdftmp->getComponents())->find(modelName2);
      RooArgList funcList2 =  pdfmodel2->funcList();
      RooLinkedListIter funcIter2 = funcList2.iterator() ;
      RooProduct* comp2 = 0;
      if(firstPOI) {
        if (!IsConditionnal) firstPOI->setVal(muhat);
        else                 firstPOI->setVal(mu);
      }
      ibkg=0;
      //icolor=0;
      while( (comp2 = (RooProduct*) funcIter2.Next()) ) {
        ibkg++;
        int color=kGray;
        TString compname(comp2->GetName());
        compname.ReplaceAll("L_x_","");
        compname.ReplaceAll(tt->GetName(),"");
        compname.ReplaceAll("_overallSyst_x_StatUncert","");
        compname.ReplaceAll("_overallSyst_x_HistSyst","");
        compname.ReplaceAll("_overallSyst_x_Exp","");
        compname.ReplaceAll("_","");
        if (IsConditionnal && firstPOI) firstPOI->setVal(1.0);

        //        icolor++;
        //        if(icolor==1) { color = kOrange - 3; }
        //        else if(icolor==2) { color = kCyan + 1; }
        //        else if(icolor==3) { color = kGreen - 9; }
        //        else if(icolor==4) { color = kAzure - 9; }
        //        else if(icolor==5) { color = kOrange + 10;}
        //        else if(icolor==6) { color = kGreen - 6; }
        //        else if(icolor==7) { color = kAzure - 4; }
        //        else if(icolor==8) { color = kOrange + 6; }
        //        else if(icolor==9) { color = kGreen + 1; }
        //        else if(icolor==10) { color = kAzure + 2; }
        //        else if(icolor==11) { color = kOrange; }
        //        else if(icolor==12) { color = kGreen + 3; }
        //        else if(icolor==13) { color = kAzure - 4; }
        //        else if(icolor==14) { color = kOrange; }
        //        else if(icolor==15) { color = kGreen + 1; }
        //        else if(icolor==16) { color = kOrange - 7; }
        //        else if(icolor==17) { color = kPink + 1; }
        //        else   color=icolor;

        color = kBlack;
        if(compname.Contains("DibosonSign"))    { color = kRed; }
        else if(compname.Contains("WH"))   { color = kRed; }
        else if(compname.Contains("ZH"))   { color = kRed; }
        else if(compname.Contains("Diboson"))   { color = kOrange - 3; }
        else if(compname.Contains("WW"))        { color = kOrange + 7; }
        else if(compname.Contains("WZ"))        { color = kOrange + 10;}
        else if(compname.Contains("ZZ"))        { color = kOrange + 6; }
        else if(compname.Contains("Top"))       { color = kOrange;     }
        else if(compname.Contains("ttbar"))     { color = kOrange;     }
        else if(compname.Contains("stop"))      { color = kOrange - 7; }
        else if(compname.Contains("Wl"))        { color = kGreen - 9;  }
        else if(compname.Contains("Wc"))        { color = kGreen - 6;  }
        else if(compname.Contains("Wcc"))       { color = kGreen + 1;  }
        else if(compname.Contains("Wb"))        { color = kGreen + 3;  }
        else if(compname.Contains("W"))         { color = kGreen + 1;  }
        else if(compname.Contains("Zl"))        { color = kAzure - 9;  }
        else if(compname.Contains("Zc"))        { color = kAzure - 4;  }
        else if(compname.Contains("Zb"))        { color = kAzure + 2;  }
        else if(compname.Contains("Z"))         { color = kAzure - 4;  }
        else if(compname.Contains("multijet"))  { color = kPink + 1;   }

        double Ntemp=(comp2->createIntegral(*obs))->getVal() * binWidth->getVal();
        preFits[compname] += Ntemp;
        preFitsChan[compname] += Ntemp;
        //pdfmodel2->plotOn(frame,LineWidth(0),Components(*comp2),LineColor(0), LineStyle(3), Normalization(Ntemp,RooAbsReal::NumEvent),Name("NoStacked_BkgBeforeFit_"+compname));

      }
      if(firstPOI) {
        if (!IsConditionnal) firstPOI->setVal(muhat);
        else                 firstPOI->setVal(mu);
      }

      c2->cd();
      frm->Draw();
      if(!blind) { text.DrawLatex( 0.73,0.81, WritDownMuValue ); }
      TString ts_chi2 = Form("#chi^{2}=%1.1f", chi2 );
      text.DrawLatex( 0.22, 0.83, ts_chi2 );

      TLegend *leg = new TLegend(0.54,0.40,0.77,0.76);
      leg->SetBorderSize(0);
      leg->SetFillColor(0);
      leg->SetTextFont(62);
      leg->SetTextSize(0.050);
      for (int i=0; i<frm->numItems(); i++) {
        TString obj_name=frm->nameOf(i);
        if (obj_name=="" || obj_name.Contains("NotAppears") || obj_name.Contains("NoStacked") ) continue;
        TObject *obj = frm->findObject(obj_name.Data());
        if (((string)obj_name).find("Data")   !=string::npos) { leg->AddEntry( obj , "Data" , "p"); continue; }
        TString legname;
        if (!IsConditionnal) legname = "After fit (#mu[best] S+B)";
        else                 legname = "After fit (#mu[fixed] S+B)";
        if (((string)obj_name).find("AfterFit")!=string::npos){ leg->AddEntry( obj , legname , "lf"); continue; }
        legname = muValueBeforeFitLegend;
        if (((string)obj_name).find("BeforeFit")!=string::npos){leg->AddEntry( obj ,legname , "l"); continue;}

        obj_name.ReplaceAll("Stacked_","");
        leg->AddEntry(obj,obj_name,"l");
      }

      //leg->Draw();
      c2->cd();
      frm->Draw();
      text.DrawLatex( 0.22, 0.83, ts_chi2 );


      // Save the plots
      MainDir->cd();
      c2->Write();
      if(drawPlots) {
        c2->Print(dirName+"/"+c2->GetName()+".eps");
        c2->Print(dirName+"/"+c2->GetName()+".png");
      }
      c2->Close();
      gROOT->cd();


      cout << "Number of items in frame " << frame->numItems() << endl;
      frm->Clear();
      cout << "Number of items in frame " << frame->numItems() << endl;
      delete frm;
      frm = 0;


      /*
      // plot each component
      TString chanName(tt->GetName());
      funcIter = funcList.iterator() ;
      while( (comp = (RooProduct*) funcIter.Next()) ) {

      // Load the value from the global fit
      if(!w->loadSnapshot(snapshotName)) {
      cout << "Cannot load " <<  snapshotName << endl;
      exit(-1);
      }

      cname = "can_DistriAfterFit_"+ (TString) tt->GetName() +"_GlobalFit_" + TS_IsConditionnal + "_mu";
      cname += mu;
      TString compName(comp->GetName());
      compName.ReplaceAll("L_x_","");
      compName.ReplaceAll(chanName,"");
      compName.ReplaceAll("__overallSyst_x_StatUncert","");
      compName.ReplaceAll("__overallSyst_x_HistSyst","");
      compName.ReplaceAll("__overallSyst_x_Exp","");
      cname.Append("_"+compName);

      TCanvas* c3 = new TCanvas( cname );
      RooPlot* compFrame = obs->frame();
      cout << "COMP FRAMCE " << compFrame->numItems() << endl;
      FrameName = "Plot_" + compName + "_Global_" + (TString) IsConditionnal;
      compFrame->SetName( FrameName );
      compFrame->SetYTitle("EVENTS");
      postFitIntegral = ( (comp->createIntegral(*obs))->getVal() * binWidth->getVal() );
      comp->plotOn(compFrame,FillColor(kOrange),LineWidth(2),LineColor(kBlue),VisualizeError(*fitresGlobal,1),
      Normalization(postFitIntegral),Name("AfterFit"));
      comp->plotOn(compFrame,LineWidth(2),Normalization(postFitIntegral));
      c3->cd();
      compFrame->Draw();

      // Putting nuisance parameter at the central value and draw the nominal distribution
      if(!w->loadSnapshot("snapshot_paramsVals_initial")) {
      cout << "Cannot load " <<  "snapshot_paramsVals_initial" << endl;
      exit(-1);
      }
      preFitIntegral = ( (comp->createIntegral(*obs))->getVal() * binWidth->getVal() );
      //comp->plotOn(compFrame,LineWidth(2),Name("BeforeFit"),LineStyle(kDashed),Normalization(preFitIntegral));
      //comp->plotOn(compFrame,LineWidth(2),Name("BeforeFit"),LineStyle(kDashed),Normalization(preFitIntegral,RooAbsReal::NumEvent));
      cout << tt->GetName() << "\t" << compName << "\t" << preFitIntegral << " " << postFitIntegral <<  endl;
      c3->cd();
      compFrame->Draw();
      c3->cd();
      TString normChange = Form("Norm %5.2f -> %5.2f = %5.2f",preFitIntegral, postFitIntegral, postFitIntegral/preFitIntegral);
      text.DrawLatex( 0.84,0.83, normChange);

      leg->Clear();
      leg->SetBorderSize(0);
      leg->SetFillColor(0);
      leg->SetTextFont(62);
      leg->SetTextSize(0.050);
      for (int i=0; i<compFrame->numItems(); i++) {
      TString obj_name=compFrame->nameOf(i);
      if (obj_name=="") continue;
      TObject *obj = compFrame->findObject(obj_name.Data());
      if (((string)obj_name).find("Data")   !=string::npos) leg->AddEntry( obj , "Data" , "p");
      if (((string)obj_name).find("AfterFit")!=string::npos) leg->AddEntry( obj , "After fit" , "lf");
      TString legname = Form("Before fit (#mu=%2.2f)",firstPOI->getVal());
      if (((string)obj_name).find("BeforeFit")!=string::npos) leg->AddEntry( obj ,legname , "l");
      }
      leg->Draw();


      // Save the plots
      MainDir->cd();
      c3->Write();
      if(drawPlots) {
      c3->Print(dirName+"/"+c3->GetName()+".eps");
      c3->Print(dirName+"/"+c3->GetName()+".png");
    }
    c3->Close();
    gROOT->cd();
    delete compFrame;
    compFrame = 0;

    // make a webpage
    if(drawPlots) {
      system("./createHTMLpageAllPlots.sh "+dirName+" &");
    }

    } // loop over components
    */

      normFile << endl << tt->GetName() << endl;
    normFile << "Component \t PreFit \t PostFit \t Change" << endl;
    float totalPre(0);
    float totalPost(0);
    for(map<TString,float>::iterator ipre(preFitsChan.begin()); ipre!=preFitsChan.end(); ipre++) {
      totalPre += ipre->second;
      totalPost += postFitsChan[ipre->first];
    }
    for(map<TString,float>::iterator ipre(preFitsChan.begin()); ipre!=preFitsChan.end(); ipre++) {
      normFile << ipre->first << "\t" << ipre->second << "\t" << postFitsChan[ipre->first] << "\t" << postFitsChan[ipre->first]/ipre->second << endl;
      normFile << "\t\t" << ipre->second/totalPre << "\t" << postFitsChan[ipre->first]/totalPost << endl;
    }
    normFile << "total \t" << totalPre << "\t" << totalPost << "\t" << totalPost/totalPre << endl;
    //normFile << "total Bkgd Frac Error +/- " << (totalUp-totalNom)/totalNom << "\t" << (totalDn-totalNom)/totalNom << endl;
    //normFile << "\t\t" << totalDn << "\t" << totalNom << "\t" << totalUp << endl;
    normFile << endl;

    } // loop over channels

    normFile << globOrAsim << " Normalizations" << endl;
    normFile << "Component \t PreFit \t PostFit \t Change" << endl;
    for(map<TString,float>::iterator ipre(preFits.begin()); ipre!=preFits.end(); ipre++) {
      normFile << ipre->first << "\t" << ipre->second << "\t" << postFits[ipre->first] << "\t" << postFits[ipre->first]/ipre->second << endl;
    }
    normFile << endl << endl;
    normFile << "Component \t Change" << endl;
    for(map<TString,float>::iterator ipre(preFits.begin()); ipre!=preFits.end(); ipre++) {
      normFile << ipre->first << "\t" << postFits[ipre->first]/ipre->second << endl;
    }
    normFile << endl << endl;
    normFile.close();

    int Nchannel = NameChannel.size();
    TH1F *hChi2 = new TH1F("Chi2PerChannel","Chi2PerChannel",Nchannel,0,Nchannel);
    for (int jb=0 ; jb<Nchannel ; jb++){
      hChi2->SetBinContent(jb+1,Chi2Channel[jb]);
      NameChannel[jb].ReplaceAll("2012","12");
      NameChannel[jb].ReplaceAll("ZeroLepton","0L");
      NameChannel[jb].ReplaceAll("OneLepton","1L");
      NameChannel[jb].ReplaceAll("TwoLepton","2L");
      NameChannel[jb].ReplaceAll("1tag","1T");
      NameChannel[jb].ReplaceAll("0tag","0T");
      NameChannel[jb].ReplaceAll("bin","");
      hChi2->GetXaxis()->SetBinLabel(jb+1,NameChannel[jb]);
    }
    sort( Chi2Channel.begin(), Chi2Channel.end());
    hChi2->GetYaxis()->SetRangeUser(0.0,Chi2Channel[Chi2Channel.size()-1]*1.50);
    hChi2->SetTitle("#chi^{2} overview among channels");
    hChi2->SetLineColor(1);
    hChi2->SetMarkerColor(1);
    hChi2->SetLineWidth(2);
    hChi2->GetXaxis()->LabelsOption("v");


    TCanvas* chi2can = new TCanvas(hChi2->GetName());
    chi2can->SetBottomMargin( 1.5 * chi2can->GetBottomMargin() );
    chi2can->cd();
    hChi2->Draw();

    MainDir->cd();
    //hChi2->Write();
    chi2can->Write();
    gROOT->cd();

    return;
  } ; // PlotHistosAfterFitGlobal


  void Plot1DResponse(RooAbsReal* nll, RooRealVar* var, TString cname, TCanvas* can, TF1* poly, bool IsFloating, TLatex* latex, TDirectory* tdir, RooArgSet* SliceSet) {
    TString vname=var->GetName();
    cout << "Varname: " << vname << endl;
    vname.ReplaceAll("alpha_","");
    vname.ReplaceAll("gamma_","");
    vname.ReplaceAll("ATLAS_","");
    RooPlot* frame2 = var->frame(Title("-log(L) vs "+vname+" "+cname));
    TString tag("");
    //     if(SliceSet) {
    //	// plot a slice instead of a projection
    //	nll->plotOn(frame2,LineColor(kRed),ShiftToZero(),Slice(*SliceSet)) ;
    //	tag.Append("_slice");
    //     } else {
    //	nll->plotOn(frame2,LineColor(kRed),ShiftToZero()) ;
    //     }

    nll->plotOn(frame2,Name("FullLHood"),LineColor(kRed),ShiftToZero()) ;
    RooCurve* curve = frame2->getCurve();

    if(SliceSet) {
      // plot a slice instead of a projection
      nll->plotOn(frame2,Name("SliceLHood"),LineColor(kGreen),ShiftToZero(),MoveToBack(),Slice(*SliceSet)) ;
    }

    float min(0);
    if(!IsFloating) {
      RooCurve* poissonTerm = new RooCurve(*curve);
      for(int i=1; i<curve->GetN()-2; i++) {
        poissonTerm->SetPoint(i, curve->GetX()[i], curve->GetY()[i] - curve->GetX()[i]*curve->GetX()[i]/2.);
        if(poissonTerm->GetY()[i] < min) { min = poissonTerm->GetY()[i]; }
      }
      poissonTerm->SetLineColor(kBlue);
      frame2->addPlotable(poissonTerm,"same",false,false);
      //frame2->GetXaxis()->SetRangeUser(-5.0,5.0);
      frame2->GetXaxis()->SetRangeUser(-2.0,2.0);
      curve = poissonTerm;
    } else {
      float val = var->getVal();
      float minVal = 0;
      float maxVal = 2;
      if(val>1/5) {
        minVal = val - 2; if(minVal<0) { minVal = 0; }
        maxVal = val + 2;
      }
      //frame2->GetXaxis()->SetRangeUser(0.0,2.0);
      frame2->GetXaxis()->SetRangeUser(minVal,maxVal);
    }

    // fit
    for( int par(0); par<3; par++) { poly->SetParameter(par,0); }
    curve->Fit(poly,"RQN"); // R=range, Q=quiet, N=do not draw
    TString fitStr = Form("%5.2f + %5.2fx + %5.2fx^{2}", poly->GetParameter(0), poly->GetParameter(1), poly->GetParameter(2));
    latex->SetText(0.925,0.925, fitStr);

    // y axis
    //    frame2->updateYAxis(min,5.0);
    //    frame2->GetYaxis()->SetRangeUser(min,5.0);
    // frame2->updateYAxis(min,1.0);
    frame2->SetMinimum(min);
    frame2->SetMaximum(1.0);
    frame2->GetYaxis()->SetRangeUser(min,1.0);
    frame2->GetYaxis()->SetTitle("#Delta [-Log(L)]");

    if(cname.Length()>0) { cname.Append("_"); } // it is 0 if plotting nll for full PDF - it is not if plotting for 1 channel
    cname.Prepend("NLLscan_");
    cname.Append(vname);
    cname.Append(tag);
    can->SetTitle(cname);
    can->SetName(cname);
    can->cd();
    frame2->Draw();
    latex->Draw("same");
    TDirectory* saveDir = tdir->GetDirectory(var->GetName());
    if(!saveDir) { saveDir = tdir->mkdir(var->GetName()); }
    saveDir->cd();
    can->Write(cname);
    gROOT->cd();
    if(drawPlots) {
      TString epsDir("LHoodPlots/"+vname+"/");
      system(TString("mkdir -vp ")+epsDir);
      epsDir.Append(cname); epsDir.Append(".eps");
      can->Print(epsDir);
    }
  } // Plot1DResponse

  TTree* createObservableTree(ModelConfig* model) {
    cout << "Creating observable tree ..." << endl;
    TTree* tree = new TTree();
    RooArgSet* allParams = (RooArgSet*) model->GetParametersOfInterest() -> Clone();
    allParams -> add(*mc->GetNuisanceParameters());
    TIterator* paramIter = allParams->createIterator();
    RooRealVar* param = 0;
    while ((param = (RooRealVar*) paramIter->Next())) {
      TString name = param -> GetName();
      //cout << "register branch: " << name.Data() << endl;
      Double_t* branch = new Double_t();
      Double_t* branchErr = new Double_t();
      tree -> Branch(name, branch);
      tree -> Branch(name + "_err", branchErr);
    }
    return tree;
  }

  void setObservableTreeValues(ModelConfig* model, TTree* tree) {
    RooArgSet* allParams = (RooArgSet*) model->GetParametersOfInterest() -> Clone();
    allParams -> add(*mc->GetNuisanceParameters());
    TIterator* paramIter = allParams->createIterator();
    RooRealVar* param = 0;
    while ((param = (RooRealVar*) paramIter->Next())) {
      //cout << "Fill branch: ";
      //param -> Print();
      TString name = param -> GetName();
      TBranch* branch = tree -> GetBranch(name);
      TBranch* branchErr = tree -> GetBranch(name + "_err");
      if (!branch || !branchErr) continue;
      double* branchVal = (double*) branch -> GetAddress();
      double* branchValErr = (double*) branchErr -> GetAddress();
      if (!branchVal || !branchValErr) continue;
      float val = param -> getVal();
      //float err = (fabs(param -> getErrorHi()) + fabs(param -> getErrorLo())) / 2;
      float err = param -> getError();
      *branchVal = val;
      *branchValErr = err;
    }
    //tree -> Fill();
  }

  void Plot1DResponseNew(RooAbsReal* nll, RooRealVar* var, TString cname, TCanvas* can, bool IsFloating, TLatex* latex, TDirectory* tdir, TString snapshotName) {

    cout << "Load snapshot '" << snapshotName << "': ";
    if(!w->loadSnapshot(snapshotName)) {
      cout << "ERROR!" << endl;
      return;
    }
    cout << "OK!" << endl;

    TString vname=var->GetName();
    //if (!vname.Contains("JetEResol")) return;
    //if (!vname.Contains("SigXsec")) return;
    vname.ReplaceAll("alpha_","");
    vname.ReplaceAll("gamma_","");
    vname.ReplaceAll("ATLAS_","");
    int nPoints1D = 101;
    int nPoints = 51;
    int maxDLL = 2;
    float center = var->getVal();
    float err = (fabs(var->getErrorHi()) + fabs(var->getErrorLo())) / 2;
    float delta = 1.5;
    if (IsFloating) delta = 0.3;
    if (IsFloating && vname.Contains("ultijet")) delta = 1.0;
    if (vname.Contains("SigXsec")) delta = 1.0;
    //float delta = 3 * err;
    float min = center - delta;
    float max = center + delta;
    float step = 2 * delta / (nPoints - 1);

    cout << "Running NLL scan ..." << endl;
    cout << "Varname = " << vname << endl;
    cout << "Value   = " << center << " +/- " << err << endl;
    cout << "nPoints = " << nPoints1D << " / " << nPoints << endl;

    RooPlot* frame2 = var->frame(Title("-log(L) vs "+vname+" "+cname),Bins(nPoints1D-1),Range(min,max));
    frame2 -> SetTitle("");
    TString tag("");

    cout << "Scanning parameter (others fixed)..." << endl;
    nll->plotOn(frame2,Name("FullLHood"),LineColor(kAzure + 1),ShiftToZero(),Precision(-1)) ;
    RooCurve* curve = frame2->getCurve();

//    frame2->updateYAxis(0,1.0);

// Plot the profile likelihood in frac
    //RooAbsReal* pll_frac = nll->createProfile(*var, Offset(1), NumCPU(4,3), Optimize(2));
    RooAbsReal* pll_frac = nll->createProfile(*var);
    pll_frac -> enableOffsetting(kTRUE);
    pll_frac -> constOptimizeTestStatistic(RooAbsArg::Activate, 2);
    ROOT::Math::MinimizerOptions::SetDefaultStrategy(1);

    // start a timer
    TStopwatch *mn_t = new TStopwatch;
    mn_t->Start();

    // this is the easy way:
    //pll_frac->plotOn(frame2,LineColor(kRed),Precision(-1));

    // try to be a bit smarter: go left and right from minimum
    TGraph* curve2 = new TGraph(nPoints);
    TTree* resultTree = createObservableTree(mc);
    resultTree -> SetName("NLLscanTree_" + vname);
    Double_t branch_minNLL = 0;
    resultTree -> Branch("minNLL", &branch_minNLL);
    cout << "Scanning parameter (others re-fitted)..." << endl;
    cout << "Reload snapshot!" << endl;
    w->loadSnapshot(snapshotName);
    for (int sign = -1; sign <= 1; sign += 2) {
      // iterate over points
      for (int i = 0; i <= (nPoints-1)/2; i++) {
        // create profile for each point (force re-fitting)
        //    RooAbsReal* pll_frac = nll->createProfile(*var);
        //    pll_frac -> enableOffsetting(kTRUE);
        //    pll_frac -> constOptimizeTestStatistic(RooAbsArg::Activate, 2);
        //    ROOT::Math::MinimizerOptions::SetDefaultStrategy(0);

	// get paramter value and pll
	double val = center + sign * i * step;
        //double val  = 0.3; // fixed value
	//double val = center + floor(sign * i * step * 10) / 10.; // jumping value
        var -> setVal(val);
        double pll = pll_frac -> getVal(*var);
        double time = mn_t -> CpuTime() / mn_t -> Counter();
        mn_t -> Start(false);
        cout << "Point " << sign * i << ":  " << vname.Data() << " = " << val << "  pll = " << pll << "  time/s = " << time << endl;
	int point = (nPoints-1)/2 + sign * i;
        curve2 -> SetPoint(point, val, pll);

        // fill tree
        setObservableTreeValues(mc, resultTree);
        branch_minNLL = pll;
        resultTree -> Fill();

	// check if NLL exceeds range and set remaining points to current value
        if (fabs(pll) > maxDLL) {
	  cout << "NLL exceeding allowed range of +-" << maxDLL << " -> break iteration." << endl;
	  while (point >= 0 && point <= nPoints) {
            curve2 -> SetPoint(point, val, pll);
	    point += sign;
	  }
	  break;
        }
        //        delete pll_frac;
      }
      cout << "Reload snapshot!" << endl;
      w->loadSnapshot(snapshotName);
    }

    // stop timing
    mn_t->Stop();
    cout << " total CPU time: " << mn_t->CpuTime() << endl;
    cout << " total real time: " << mn_t->RealTime() << endl;

    // some plotting
    frame2->GetYaxis()->SetRangeUser(-0.5, 3);
    frame2->GetYaxis()->SetTitle("-#DeltaLog(L)");

    if(cname.Length()>0) { cname.Append("_"); } // it is 0 if plotting nll for full PDF - it is not if plotting for 1 channel
    cname.Prepend("NLLscan_");
    cname.Append(vname);
    cname.Append(tag);
    can->SetTitle(cname);
    can->SetName(cname);
    can->SetTopMargin(0.05);
    can->SetRightMargin(0.05);
    can->cd();
    frame2->Draw();

    TF1* pol2 = new TF1("pol2", "[0]+0.5*pow((x-[1])/[2], 2)", min-1, max+1);
    pol2 -> SetParameter(0, curve->interpolate(center));
    pol2 -> SetParameter(1, center);
    pol2 -> SetParameter(2, err);
    pol2 -> SetLineColor(1);
    pol2 -> SetLineWidth(3);
    pol2 -> SetLineStyle(1);
    pol2 -> Draw("sameL");

    curve2 -> SetLineColor(2);
    curve2 -> SetLineWidth(3);
    curve2 -> Draw("sameL");

    latex->Draw("same");
    TDirectory* saveDir = tdir->GetDirectory(var->GetName());
    if(!saveDir) { saveDir = tdir->mkdir(var->GetName()); }
    saveDir->cd();
    can->Write(cname);
    resultTree -> Write();
    gROOT->cd();
    if(drawPlots) {
      TString epsDir = tdir->GetPath();
      epsDir.ReplaceAll("/FitCrossChecks.root:","");
      system(TString("mkdir -vp ")+epsDir);
      epsDir.Append("/" + vname + ".eps");
      can->Print(epsDir);
    }
  } // Plot1DResponse

  int GetPosition(RooRealVar* var, TH2D* corrMatrix) {
    int position(0);
    for(int b=0; b<corrMatrix->GetNbinsX()+2; b++) {
      if(TString(corrMatrix->GetXaxis()->GetBinLabel(b)).CompareTo(var->GetName())) { continue; } // 0 = match, non-zero = not match
      position = b;
      break;
    }
    return position;
  } // Plot1DResponseNew

  list< pair<RooRealVar*,float> > GetOrderedCorrelations(RooRealVar* var, RooFitResult* fitres) {
    list< pair<RooRealVar*, float> > pairs;
    TIterator* inp = mc->GetNuisanceParameters()->createIterator();
    RooRealVar* np = NULL;
    float corr(0);
    // save all into a list to be sorted
    while( (np = (RooRealVar*) inp->Next()) ){
      if(TString(np->GetName()).Contains("gamma")) { continue; }
      if(np->GetName() == var->GetName()) { continue; }
      corr = fitres->correlation(var->GetName(),np->GetName());
      pairs.push_back( make_pair( np, corr ) );
    }
    pairs.sort(comp_second_abs_decend);
    //     for(list< pair<RooRealVar* ,float> >::iterator p=pairs.begin(); p!=pairs.end(); p++) {
    //	cout << p->first->GetName() << "\t" << p->second << endl;
    //     }
    return pairs;
  } // GetOrderedCorrelations

  TH2D* GetSubsetOfCorrMatrix(RooRealVar* var, list< pair<RooRealVar*,float> >& pairs, RooFitResult* fitres, int size) {
    RooArgList corrNPs = RooArgList("corrNPs");
    // retrieve top "size" NPs and draw correlations in a 2D hist
    for(list< pair<RooRealVar* ,float> >::iterator p=pairs.begin(); p!=pairs.end(); p++) {
      if(corrNPs.getSize()==size) { break; }
      corrNPs.add( *(p->first) );
      //cout << p->first->GetName() << "\t" << p->second << endl;
    }
    corrNPs.add(*var); // add variable of interest
    // this does a decomposition
    //TMatrixDSym redCM = fitresGlobal->reducedCovarianceMatrix( corrNPs );

    // build a matrix by hand
    TString hname("Top_"+TString(Form("%i",size))+"_Correlated_NPs_for_"+TString(var->GetName()));
    size = corrNPs.getSize(); // redefine size to include variable of interest
    TH2D* redMat = new TH2D(hname,hname, size,0,size, size,0,size);
    for(int i=0; i<size; i++) {
      TString vname(corrNPs.at(i)->GetName());
      vname.ReplaceAll("alpha_Sys","");
      vname.ReplaceAll("alpha_","");
      vname.ReplaceAll("alpha_","");
      redMat->GetXaxis()->SetBinLabel(i+1, vname);
      redMat->GetYaxis()->SetBinLabel(i+1, vname);
      for(int j=0; j<size; j++) {
        redMat->SetBinContent(i+1, j+1, fitres->correlation( corrNPs.at(i)->GetName(), corrNPs.at(j)->GetName() ) );
      }
    }
    redMat->SetMinimum(-1);
    redMat->SetMaximum(1);
    return redMat;
  } // GetSubsetOfCorrMatrix


  void PlotsNuisanceParametersVSmu(){
    cout << endl;
    cout << endl;
    cout << "Performing a global fit for mu : can take time ..." << endl;
    cout << endl;

    ProfileInspector p;
    TList* list = p.GetListOfProfilePlots(*data,mc);

    for(int i=0; i<list->GetSize(); ++i){

      TString cname = "ProfileInspector_" + (TString) list->At(i)->GetName();
      TCanvas* c1 = new TCanvas(cname);
      c1->cd();
      list->At(i)->Draw("al");

      MainDirModelInspector->cd();
      c1->Write();
      c1->Close();
      gROOT->cd();
    }
    return;
  }


  void PlotsStatisticalTest(double mu_pe, double mu_hyp, int nToyMC, int rndmSeed){

    RooRandom::randomGenerator()->SetSeed(rndmSeed);

    //RooAbsData* tmpData = w->data("obsData") ;

    cout << endl;
    cout << endl;
    cout << "Will generate " << nToyMC << " pseudo-experiments for : " << endl;
    cout << " - mu[pseudo-data] = " << mu_pe  << endl;
    cout << " - mu[stat-test]   = " << mu_hyp << endl;
    cout << endl;

    // Check number of POI (for Wald approx)
    RooArgSet *ParamOfInterest = (RooArgSet*) mc->GetParametersOfInterest();
    //int nPOI = ParamOfInterest->getSize();
    RooRealVar* firstPOI    = (RooRealVar*) ParamOfInterest->first();
    //RooSimultaneous *simPdf = (RooSimultaneous*)(mc->GetPdf());
    if(firstPOI) { firstPOI->setVal(mu_pe); }

    // unconditional fit to data:
    //RooFitResult *fitres = FitPDF( mc, simPdf, tmpData, "Minuit2" );

    /*
    for (int i = 0; i < nToyMC; i++) {
      RooDataSet* asimovData = mc->GetPdf()->generate(*mc->GetObservables(), Extended(kFALSE));
      RooFitResult* fitres = FitPDF(mc, simPdf, asimovData, "Minuit2");
    }
    return;

    RooRealVar* norm = new RooRealVar("norm", "norm", 1000, 100, 10000);
    RooAbsPdf* pdf = mc->GetPdf();
    RooExtendPdf* pdfe = new RooExtendPdf("pdfe", "extended pdf", *pdf, *norm);
    //pdf = pdfe;
    cout << "NORM = " << norm->getVal() << endl;
    */

    // set up the sampler
    ToyMCSampler sampler;
    sampler.SetPdf(*mc->GetPdf());
    sampler.SetObservables(*mc->GetObservables());
    sampler.SetNToys(nToyMC);
    //sampler.SetGlobalObservables(*mc->GetGlobalObservables()); // enable randomization of NPs
    sampler.SetParametersForTestStat(*mc->GetParametersOfInterest());
    RooArgSet* poiset = dynamic_cast<RooArgSet*>(w->set("ModelConfig_POI")->Clone());

    // only unconditional fit
    MinNLLTestStat *minNll = new MinNLLTestStat(*mc->GetPdf());
    minNll->SetMinimizer("Minuit2");
    minNll->SetStrategy(1);
    minNll->SetLOffset(kTRUE);
    minNll->SetPrintLevel(3);
    minNll->SetReuseNLL(kTRUE);
    minNll->EnableDetailedOutput(kTRUE);
    sampler.AddTestStatistic(minNll);

    //ProfileLikelihoodTestStat ts(*simPdf);


    // enable PROOF if desired
    //ProofConfig pc(*w, 8, "workers=8", kFALSE);
    //sampler.SetProofConfig(&pc);

    // evaluate the test statistics - this is where most of our time will be spent
    cout << "Generating " << nToyMC << " toys...this will take a few minutes" << endl;
    TStopwatch *mn_t = new TStopwatch;
    mn_t->Start();
    RooDataSet* sd = sampler.GetSamplingDistributions(*poiset);
    cout << "Toy generation complete :" << endl;
    // stop timing
    mn_t->Stop();
    cout << " total CPU time: " << mn_t->CpuTime() << endl;
    cout << " total real time: " << mn_t->RealTime() << endl;

    // now sd contains all information about our test statistics, including detailed output
    // we might eg. want to explore the results either directly, or first converting to a TTree
    // do the conversion
    TFile f(OutputDir + "/mytoys.root", "RECREATE");
    TTree *toyTree = RooStats::GetAsTTree("toyTree", "TTree created from test statistics", *sd);
    // save result to file, but in general do whatever you like
    f.cd();
    toyTree->Write();
    f.Close();

    TFile* tmpFile = new TFile(OutputDir + "/mytoys.root", "READ");
    TTree* myTree = (TTree*)tmpFile->Get("toyTree");

    // get boundaries for histograms
    TIter nextLeaf( (myTree->GetListOfLeaves())->MakeIterator() );
    TObject* leafObj(0);
    map<TString, float> xMaxs;
    map<TString, float> xMins;
    for(int i(0); i<myTree->GetEntries(); i++) {
      myTree->GetEntry(i);
      nextLeaf = ( (myTree->GetListOfLeaves())->MakeIterator() );
      while( (leafObj = nextLeaf.Next()) ) {
        TString name(leafObj->GetName());
        float value(myTree->GetLeaf( leafObj->GetName() )->GetValue());
        if(value > xMaxs[name]) { xMaxs[name] = value; }
        if(value < xMins[name]) { xMins[name] = value; }
      } // loop over leaves
    } // loop over tree entries

    // plot everything in the tree
    myTree->GetEntry(0);
    nextLeaf = ( (myTree->GetListOfLeaves())->MakeIterator() );
    leafObj = 0;
    // make a histogram per leaf
    map<TString, TH1F*> hists;
    myTree->GetEntry(0);
    while( (leafObj = nextLeaf.Next()) ) {
      if(!leafObj) { continue; }
      //cout << leafObj->GetName() << endl;
      TString name(leafObj->GetName());
      // special ones : fit related things
      if(name.Contains("covQual"))   { hists[name] = new TH1F(name,name,5,0,5); continue; }
      if(name.Contains("fitStatus")) { hists[name] = new TH1F(name,name,5,0,5); continue; }
      int nbin(500);
      float histMin( xMins[name] - 0.1*fabs(xMins[name]) );
      float histMax( xMaxs[name] + 0.1*fabs(xMaxs[name]) );
      if(name.Contains("ATLAS_norm")) { // floating normalization factors
        histMin = 0; histMax = 10;
      }
      else if(name.Contains("gamma_stat")) { // statistical nus param
        if(name.Contains("globObs")) {  // get custom range for sampling
          histMin = int( xMins[name] - 0.1*fabs(xMins[name]) );
          histMax = int( xMaxs[name] + 0.1*fabs(xMaxs[name]) );
        } // use small range for pull and error
        else { nbin = 100; histMin = 0.0; histMax = 2.0; }
      }
      else if(name.Contains("_err")) { // errors on nus param
        nbin = 100; histMin = 0.0; histMax = 2.0;
      }
      else if(name.Contains("fitCond") || name.Contains("fitUncond") || name.Contains("globObs")) { // fit pulls
        nbin = 500; histMin = -5; histMax = 5;
      }
      hists[name] = new TH1F(name,name,nbin,histMin,histMax);
    } // loop over leaves to declare histos

    // loop over entries and fill histograms
    for(int i(0); i<myTree->GetEntries(); i++) {
      myTree->GetEntry(i);
      nextLeaf = ( (myTree->GetListOfLeaves())->MakeIterator() );
      while( (leafObj = nextLeaf.Next()) ) {
        TString name(leafObj->GetName());
        if(hists.find(name) == hists.end()) { continue; }
        hists[name]->Fill( myTree->GetLeaf( leafObj->GetName() )->GetValue() );
      } // loop over leaves
    } // loop over tree entries

    // overflow and underflow
    for(map<TString,TH1F*>::iterator ihist(hists.begin()); ihist!=hists.end(); ihist++) {
      if(ihist->second->GetBinContent(0)>0) {
        ihist->second->SetBinContent(1, ihist->second->GetBinContent(0) + ihist->second->GetBinContent(1) );
        // fix err
      }
      int nBinx = ihist->second->GetNbinsX();
      if(ihist->second->GetBinContent(nBinx)>0) {
        ihist->second->SetBinContent(nBinx-1, ihist->second->GetBinContent(nBinx) + ihist->second->GetBinContent(nBinx-1) );
        // fix err
      }
    }

    // save the results
    TString dirName(OutputDir+"/PlotsStatisticalTest/GlobalFit");
    if(drawPlots) {
      system(TString("mkdir -vp "+dirName));
    }
    TCanvas* canvas = new TCanvas("pulls");
    TLegend *leg = new TLegend(0.67, 0.64, 0.87, 0.86);
    LegendStyle(leg);
    for(map<TString,TH1F*>::iterator ihist(hists.begin()); ihist!=hists.end(); ihist++) {
      if( (ihist->first).Contains("fitCond_") ) { continue; } // skip unconditional fit - get it explicitly
      canvas->Clear();
      leg->Clear();
      TString niceName(ihist->first);
      niceName.ReplaceAll("fitUncond_","");
      //niceName.ReplaceAll("SD_TS0_",""); // not good if have multiple test statistics
      // conditional fit information
      ihist->second->SetLineColor(kGray+2);
      ihist->second->SetTitle(niceName);
      ihist->second->SetLineStyle(kSolid);
      ihist->second->SetLineWidth(2);
      if((ihist->first).Contains("fit") && !(ihist->first).Contains("_err")
          && !(ihist->first).Contains("Qual") && !(ihist->first).Contains("Status")) {
        ihist->second->Rebin(4);
      }

      //      ihist->second->GetXaxis()->SetTitle("");
      //      ihist->second->GetYaxis()->SetTitle("");

      if(niceName.Contains("globObs")) {
        leg->AddEntry( ihist->second, "Sampling", "l" ); // add value of mu
      } else {
        leg->AddEntry( ihist->second, "Unconditional Fit", "l" ); // add value of mu
      }
      TString condName(ihist->first);
      condName.ReplaceAll("fitUncond","fitCond");
      // uncomditional fit information
      if(hists.find(condName) != hists.end() && condName != ihist->first) {
        hists[condName]->SetLineColor(kGray+2);
        hists[condName]->SetLineStyle(kDashed);
        hists[condName]->SetLineWidth(2);
        if(!(ihist->first).Contains("_err")) { hists[condName]->Rebin(4); }
        leg->AddEntry( hists[condName], "Conditional Fit", "l" );
        if( hists[condName]->GetMaximum() > ihist->second->GetMaximum() ) {
          ihist->second->SetMaximum( hists[condName]->GetMaximum() );
        }
      }
      ihist->second->SetMaximum( 1.2 * ihist->second->GetMaximum() );
      canvas->cd();
      ihist->second->Draw();
      leg->Draw();
      if(hists[condName] && condName != ihist->first) { hists[condName]->Draw("same"); }
      if(drawPlots) {
        canvas->Print(dirName+"/"+niceName+".eps");
        canvas->Print(dirName+"/"+niceName+".png");
      }

      MainDirStatTest->cd();
      canvas->Write();
      ihist->second->Write();
      gROOT->cd();
    } // loop over hists

    // make a webpage
    if(drawPlots) {
      system("./createHTMLpageAllPlots.sh "+dirName+" &");
    }

    // save result to file, but in general do whatever you like
    //    f.cd();
    //    toyTree->Write();
    //    f.Close();
    return;


  /*
  ProfileLikelihoodTestStat ts(*simPdf);
  double ObsPLHR = ts.Evaluate( *data , *ParamOfInterest );

  // Make pseudo experiment and plot the TestStat distribution
  TString hname = Form("Distribution of q_{#mu=%1.1f} under pseudodata with #mu=%1.1f",mu_hyp,mu_pe);
  TH1F *hStatTest = new TH1F(hname,hname,100,0,10);
  double Npe_disagreed = 0;
  for (int ipe=0 ; ipe<nToyMC ; ipe++){
  firstPOI->setVal(mu_pe);
  RooAbsData *SampledDataSet = sampler.GenerateToyData(allParameters);
  firstPOI->setVal(mu_hyp);
  double StatTest = ts.Evaluate( *SampledDataSet , *ParamOfInterest );
  hStatTest->Fill( StatTest );
  if (ipe%10==0){
  cout << "pseudo-exp " << ipe << ", StatTest = " << StatTest << endl;
  TCanvas *ctemp = PlotData(SampledDataSet,simPdf);
  TString ctempname = (TString) ctemp->GetName() + (TString) "_PseudoExp";
  ctempname += ipe;
  ctemp->SetName(ctempname);
  MainDirStatTest->cd();
  ctemp->Write();
  gROOT->cd();
  }
  }
  */

    return;
  }



  void PrintSuspiciousNPs(){

    cout.precision(3);

    cout << endl;
    cout << endl;
    cout << endl;
    cout << "==================================================================" << endl;
    cout << "     List of nuisance parameters which deserve more attention     " << endl;
    cout << "===================================================================" << endl;
    cout << endl;
    cout << endl;
    cout << " === Tension between the NP central value and CP measurement ===" << endl;
    cout << " ===============================================================" << endl;
    cout << endl;
    for (unsigned i=0 ; i<AllNPafterEachFit.size() ; i++){
      NPContainer MyNPsTemp = AllNPafterEachFit[i];
      TString name = MyNPsTemp.NPname;
      double value = MyNPsTemp.NPvalue;
      double errHi = MyNPsTemp.NPerrorHi;
      double errLo = MyNPsTemp.NPerrorLo;
      TString Fit  = MyNPsTemp.WhichFit;

      name.ReplaceAll("\\","");

      if (fabs(value)>PullMaxAcceptable){
        cout << name << "\t : \t" << value << " \t +" << errHi << "\t -" << fabs(errLo) << "  \t Fit : " << Fit << endl;
      }

    }
    cout << endl;
    cout << endl;
    cout << " === Overconstraint of profiled NP wrt CP measurement ===" << endl;
    cout << " ========================================================" << endl;
    cout << endl;
    for (unsigned i=0 ; i<AllNPafterEachFit.size() ; i++){
      NPContainer MyNPsTemp = AllNPafterEachFit[i];
      TString name = MyNPsTemp.NPname;
      double value = MyNPsTemp.NPvalue;
      double errHi = MyNPsTemp.NPerrorHi;
      double errLo = MyNPsTemp.NPerrorLo;
      TString Fit  = MyNPsTemp.WhichFit;

      name.ReplaceAll("\\","");
      if ( (fabs(errHi)+fabs(errLo))/2.0<ErrorMinAcceptable ){
        cout << name << "\t : \t" << value << " \t +" << errHi << "\t -" << fabs(errLo) << "  \t Fit : " << Fit << endl;
      }
    }

    if (UseMinosError){
      cout << endl;
      cout << endl;
      cout << " === Nuisance parameters with one sided Minos error ===" << endl;
      cout << " ======================================================" << endl;
      cout << endl;
      for (unsigned i=0 ; i<AllNPafterEachFit.size() ; i++){
        NPContainer MyNPsTemp = AllNPafterEachFit[i];
        TString name = MyNPsTemp.NPname;
        double value = MyNPsTemp.NPvalue;
        double errHi = MyNPsTemp.NPerrorHi;
        double errLo = MyNPsTemp.NPerrorLo;
        TString Fit  = MyNPsTemp.WhichFit;

        name.ReplaceAll("\\","");
        if ( errHi==0 || errLo==0 ){
          cout << name << "\t : \t" << value << " \t +" << errHi << "\t -" << fabs(errLo) << "  \t Fit : " << Fit << endl;
        }
      }
    }

    cout << endl;
    cout << endl;
    cout << "===================================================================" << endl;
    cout << endl;
    cout << endl;
    cout << endl;

    return;
  }



  double FindMuUpperLimit(){

    //RooMsgService::instance().setGlobalKillBelow(ERROR);

    RooRealVar* firstPOI = (RooRealVar*) mc->GetParametersOfInterest()->first();
    ProfileLikelihoodCalculator plc(*data,*mc);
    LikelihoodInterval* interval = plc.GetInterval();
    double UpperLimit = interval->UpperLimit(*firstPOI);

    TCanvas* c2 = new TCanvas( "Likelihood_vs_mu" );
    LikelihoodIntervalPlot plot(interval);
    plot.SetNPoints(50);
    c2->cd();
    plot.Draw("");
    delete interval;

    if(!w->loadSnapshot("snapshot_paramsVals_initial")) { // what?
    }

    outputfile->cd();
    c2->Write();
    gROOT->cd();

    return UpperLimit;

  }


  void GetNominalValueNuisancePara(){
    TIterator *it = mc->GetNuisanceParameters()->createIterator();
    RooRealVar *var = NULL;
    if (MapNuisanceParamNom.size() > 0) MapNuisanceParamNom.clear();
    std::cout << "Nuisance parameter names and values" << std::endl;
    while ((var = (RooRealVar*)it->Next()) != NULL){
      const double val = var->getVal();
      MapNuisanceParamNom[(string)var->GetName()] = val;
    }
    return;
  }


  void SetNominalValueNuisancePara(){
    TIterator *it = mc->GetNuisanceParameters()->createIterator();
    RooRealVar *var = NULL;
    while ((var = (RooRealVar*)it->Next()) != NULL){
      const double val =  MapNuisanceParamNom[(string)var->GetName()];
      var->setVal(val);
    }
    return;
  }


  void SetAllStatErrorToSigma(double Nsigma){

    TIterator* it = mc->GetNuisanceParameters()->createIterator();
    RooRealVar* var = NULL;
    while( (var = (RooRealVar*) it->Next()) ){
      string varname = (string) var->GetName();
      if ( varname.find("gamma_stat")!=string::npos ){
        RooAbsReal* nom_gamma = (RooConstVar*) w->obj( ("nom_" + varname).c_str() );
        double nom_gamma_val = nom_gamma->getVal();
        double sigma = 1/TMath::Sqrt( nom_gamma_val );
        var->setVal(1 + Nsigma*sigma);
      }
    }

    return;
  }



  void SetAllNuisanceParaToSigma(double Nsigma){

    TIterator* it = mc->GetNuisanceParameters()->createIterator();
    RooRealVar* var = NULL;
    while( (var = (RooRealVar*) it->Next()) ){
      string varname = (string) var->GetName();
      if ( varname.find("gamma_stat")!=string::npos ) continue;
      if ( varname.find("ATLAS_norm")!=string::npos ) {
        var->setVal(1);
        continue;
      }
      if(strcmp(var->GetName(),"Lumi")==0){
        var->setVal(w->var("nominalLumi")->getVal()*(1+Nsigma*LumiRelError));
      }
      else if(varname.find("ATLAS_norm")!=string::npos || varname.find("ATLAS_norm")!=string::npos) {
        var->setVal(1+Nsigma);
      }
      else {
        var->setVal(Nsigma);
      }
    }

    return;
  }


  void SetNuisanceParaToSigma(RooRealVar *var, double Nsigma){

    string varname = (string) var->GetName();
    if ( varname.find("gamma_stat")!=string::npos ) return;

    if(strcmp(var->GetName(),"Lumi")==0){
      var->setVal(w->var("nominalLumi")->getVal()*(1+Nsigma*LumiRelError));
    } else{
      var->setVal(Nsigma);
    }

    return;
  }


  void SetPOI(double mu){
    RooRealVar * firstPOI = dynamic_cast<RooRealVar*>(mc->GetParametersOfInterest()->first());
    firstPOI->setVal(mu);
    return  ;
  }


  bool IsSimultaneousPdfOK(){

    bool IsOK=true;

    bool IsSimultaneousPDF = strcmp(mc->GetPdf()->ClassName(),"RooSimultaneous")==0;
    if (!IsSimultaneousPDF){
      cout << " ERROR : no Simultaneous PDF was found, will stop here." << endl;
      cout << " You need to investigate your input histogramms." << endl;
      IsOK = false;
    }

    return IsOK;

  }


  bool IsChannelNameOK(){

    bool IsOK=true;
    if( !IsSimultaneousPdfOK() ) return false;

    RooSimultaneous* simPdf = (RooSimultaneous*)(mc->GetPdf());
    RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;
    while((tt=(RooCatType*) iter->Next()) ){
      string channelName =  tt->GetName();
      if (channelName.find("/")!=string::npos){
        cout << endl;
        cout << "One of the channel name contain a caracter \"/\" : " << endl;
        cout << "  - "  << channelName << endl;
        cout << "This is mis-intrepreted by roofit in the reading of the workspace. " << endl;
        cout << "Please change the channel name in the xml file to run this code." << endl;
        cout << endl;
        IsOK = false;
      }
    }


    return IsOK;
  }


  void PrintModelObservables(){

    RooArgSet* AllObservables = (RooArgSet*) mc->GetObservables();
    TIterator* iter = AllObservables->createIterator() ;
    RooAbsArg* MyObs = NULL;
    cout << endl;
    cout << "List of model Observables : "  << endl;
    cout << "----------------------------"  << endl;
    while( (MyObs = (RooAbsArg*) iter->Next()) )
      MyObs->Print();

    return;
  }


  void PrintNuisanceParameters(){

    RooArgSet nuis = *mc->GetNuisanceParameters();
    TIterator* itr = nuis.createIterator();
    RooRealVar* arg;
    cout << endl;
    cout << "List of nuisance parameters : "  << endl;
    cout << "----------------------------"  << endl;
    while ((arg=(RooRealVar*)itr->Next())) {
      if (!arg) continue;
      cout << arg->GetName()  << " : " << arg->getVal() << "+/-" << arg->getError() << endl;
    }
    return;
  }


  void PrintAllParametersAndValues(RooArgSet para){
    TIterator* itr = para.createIterator();
    RooRealVar* arg;
    cout << endl;
    cout << "List of parameters : "  << endl;
    cout << "----------------------------"  << endl;
    while ((arg=(RooRealVar*)itr->Next())) {
      if (!arg) continue;
      cout << arg->GetName() << " = " << arg->getVal() << endl;
    }
    return;
  }

  void PrintSubChannels(){

    RooMsgService::instance().setGlobalKillBelow(ERROR);

    if( !IsSimultaneousPdfOK() ) return;

    RooSimultaneous* simPdf = (RooSimultaneous*)(mc->GetPdf());
    RooCategory* channelCat = (RooCategory*) (&simPdf->indexCat());
    TIterator* iter = channelCat->typeIterator() ;
    RooCatType* tt = NULL;

    while((tt=(RooCatType*) iter->Next()) ){

      RooAbsPdf  *pdftmp  = simPdf->getPdf( tt->GetName() );
      RooAbsData *datatmp = data->reduce(Form("%s==%s::%s",channelCat->GetName(),channelCat->GetName(),tt->GetName()));

      cout << endl;
      cout << endl;
      cout << "Details on channel " << tt->GetName() << " : "  << endl;
      cout << "----------------------------------------------------------" << endl;
      datatmp->Print();
      pdftmp->Print();
      PrintNumberOfEvents(pdftmp);
      //FindConstants(pdftmp);

    }

    return;
  }

  TH1F* MakeHist(TString name, RooCurve *curve) {
    int nbin = (curve->GetN()-4)/2;
    cout << "Create histogram of " << nbin << " bins from " << curve->GetX()[1] << " to " << curve->GetX()[curve->GetN()-2] << endl;
    TH1F* hist = new TH1F(name,name, nbin, curve->GetX()[1], curve->GetX()[curve->GetN()-2]);
    // find point at lower edge of bin and use value - there are two and we want the second
    for(int b=1; b<hist->GetNbinsX()+1; b++) {
      bool sawFirst(false);
      cout << "Looking for " << hist->GetBinLowEdge(b) << endl;
      for(int i=0; i<curve->GetN(); i++) {
        cout << "\t" << curve->GetX()[i] << endl;
        if(fabs(curve->GetX()[i] - hist->GetBinLowEdge(b)) < 0.001 ) {
          cout << " got one" << endl;
          if(sawFirst) {
            hist->SetBinContent(b, curve->GetY()[i]);
            break;
          }
          if(!sawFirst) { sawFirst = true; }
        }
      } // loop over curve points
    } // loop over bins
    return hist;
  } // MakeHist


  // thanks desy
  void FillGraphIntoHisto(TGraph *pGraph,TH1F *pHisto){
    // takes data from a graph and fills it into a pre-binned histogram
    for (Int_t k =0 ; k<pHisto->GetNbinsX();k++){
      Double_t Sum = 0.;
      Int_t MatchingPoints = 0;
      for (Int_t i = 0; i<pGraph->GetN();i++){
        Double_t x,y;
        pGraph->GetPoint(i,x,y);
        // check if this point falls in the limit of the current histo bin
        // and add y value if the case;
        if ((x > pHisto->GetBinLowEdge(k)) && (x < ( pHisto->GetBinLowEdge(k)+pHisto->GetBinWidth(k)))) {
          Sum += y;MatchingPoints++;
        }
      }
      //average:
      if (MatchingPoints > 0) Sum = Sum/(Double_t) MatchingPoints;
      pHisto->SetBinContent(k,Sum);
    }
  }


  TH1F* ConvertGraphToHisto(TGraph *pGraph){
    // takes data from a graph, determines binning and fills data into histogram
    Int_t NPoints = pGraph->GetN();
    Double_t BinLimits[NPoints+1];
    // sort graph
    pGraph->Sort();
    // determine lower limit of histogram: half the distance to next point
    Double_t x0,x1,y;
    pGraph->GetPoint(0,x0,y);
    pGraph->GetPoint(1,x1,y);
    Double_t Distance = TMath::Abs(x0-x1);
    BinLimits[0] = x0 - Distance/2.;
    // now set upper limits for all the other points
    for (Int_t k = 0 ; k<NPoints-1;k++){
      pGraph->GetPoint(k,x0,y);
      pGraph->GetPoint(k+1,x1,y);
      Distance = TMath::Abs(x0-x1);
      BinLimits[k+1] = x0 + Distance/2.;
    }
    // for the last point set upper limit similar to first point:
    pGraph->GetPoint(NPoints-2,x0,y);
    pGraph->GetPoint(NPoints-1,x1,y);
    Distance = TMath::Abs(x0-x1);
    BinLimits[NPoints] = x1 + Distance/2.;
    // now we know the binning and can create the histogram:
    TString Name = "ConvertedHisto";
    // make name unique
    Name+= rand();
    TH1F *ThisHist = new TH1F(Name,"Converted Histogram",NPoints,BinLimits);
    // now fill the histogram
    for (Int_t i = 0; i<pGraph->GetN();i++){
      Double_t a,b;
      pGraph->GetPoint(i,a,b);
      ThisHist->SetBinContent(i+1,b);
    }
    return ThisHist;
  }

  void PrintNumberOfEvents(RooAbsPdf *pdf){

    RooRealVar* firstPOI = (RooRealVar*) mc->GetParametersOfInterest()->first();
    double val_sym=1;
    cout
      << Form(" %3s |","")
      << Form(" %-32s |","Nuisance Parameter")
      << Form(" %18s |","Signal events")
      << Form(" %18s |","% Change (+1sig)")
      << Form(" %18s |","% Change (-1sig)")
      << Form(" %18s |","Background events")
      << Form(" %18s |","% Change (+1sig)")
      << Form(" %18s |","% Change (-1sig)")
      << endl;

    int inuis=-1;
    RooArgSet  *obstmp  = pdf->getObservables( *mc->GetObservables() ) ;
    RooRealVar *myobs   = ((RooRealVar*) obstmp->first());

    RooArgSet nuis = *mc->GetNuisanceParameters();
    TIterator* itr = nuis.createIterator();
    RooRealVar* arg;
    while ((arg=(RooRealVar*)itr->Next())) {
      if (!arg) continue;
      //
      ++inuis;
      //

      double val_hi = val_sym;
      double val_lo = -val_sym;
      double val_nom = arg->getVal();
      if (string(arg->GetName()) == "Lumi"){
        val_nom = w->var("nominalLumi")->getVal();
        val_hi  = w->var("nominalLumi")->getVal() * (1+LumiRelError);
        val_lo  = w->var("nominalLumi")->getVal() * (1-LumiRelError);
      }
      //
      arg->setVal(val_hi);
      firstPOI->setVal(0);
      double b_hi = pdf->expectedEvents(*myobs);
      firstPOI->setVal(1);
      double s_hi = pdf->expectedEvents(*myobs)-b_hi;
      //
      arg->setVal(val_lo);
      firstPOI->setVal(0);
      double b_lo = pdf->expectedEvents(*myobs);
      firstPOI->setVal(1);
      double s_lo = pdf->expectedEvents(*myobs)-b_lo;
      //
      arg->setVal(val_nom);
      firstPOI->setVal(0);
      double b_nom = pdf->expectedEvents(*myobs);
      firstPOI->setVal(1);
      double s_nom = pdf->expectedEvents(*myobs)-b_nom;
      //
      double x_nom = s_nom ;
      double x_hi  = 0; if (s_nom) x_hi = (s_hi-s_nom)/s_nom;
      double x_lo  = 0; if (s_nom) x_lo = (s_lo-s_nom)/s_nom;
      double y_nom = b_nom ;
      double y_hi  = 0; if (b_nom) y_hi = (b_hi-b_nom)/b_nom;
      double y_lo  = 0; if (b_nom) y_lo = (b_lo-b_nom)/b_nom;

      cout
        << Form(" %3d |",inuis)
        << Form(" %-32s |",arg->GetName())
        << Form(" %18.2f |",x_nom)
        << Form(" %18.2f |",100*x_hi)
        << Form(" %18.2f |",100*x_lo)
        << Form(" %18.2f |",y_nom)
        << Form(" %18.2f |",100*y_hi)
        << Form(" %18.2f |",100*y_lo)
        << endl;
    }

    return;
  }

  void FindConstants(RooAbsPdf *pdf){
    cout << "Looking for constant parameters in " << pdf->GetName() << endl;
    cout << "The following will be set to constant " << endl;

    double val_sym=1;
    RooArgSet  *obstmp  = pdf->getObservables( *mc->GetObservables() ) ;
    RooRealVar *myobs   = ((RooRealVar*) obstmp->first());

    RooArgSet nuis = *mc->GetNuisanceParameters();
    TIterator* itr = nuis.createIterator();
    RooRealVar* arg = 0;
    while ((arg=(RooRealVar*)itr->Next())) {
      if (!arg) continue;

      double val_hi = val_sym;
      double val_lo = -val_sym;
      double val_nom = arg->getVal();
      //if (TString(arg->GetName()).Contains("Lumi") || TString(arg->GetName()).Contains("LUMI")) { continue; }
      arg->setVal(val_hi);
      double exp_hi = pdf->expectedEvents(*myobs);
      arg->setVal(val_lo);
      double exp_lo = pdf->expectedEvents(*myobs);
      arg->setVal(val_nom);
      double exp_nom = pdf->expectedEvents(*myobs);

      //cout << "\t" << arg->GetName() << "\t" << 1-exp_hi/exp_nom << "\t" << 1-exp_lo/exp_nom << endl;
      if(fabs(1-exp_hi/exp_nom) > 0.005 && fabs(1-exp_lo/exp_nom) > 0.005) { continue; }
      cout << "\t" << arg->GetName() << "\t" << fabs(1-exp_hi/exp_nom) << "\t" << fabs(1-exp_lo/exp_nom) << endl;
      arg->setConstant(kTRUE);

    }

    return;
  }

  void SetStyle(){
    gStyle->SetOptStat(0);

    return;
  }

  void LegendStyle(TLegend* l) {
    l->SetBorderSize(0);
    l->SetFillColor(0);
    l->SetLineColor(0);
    l->SetFillStyle(0);
    //l->SetTextFont(62);
    l->SetTextSize(0.050);
    return;
  }

  void Initialize(const char* infile , const char* outputdir, const char* workspaceName, const char* modelConfigName, const char* ObsDataName) {

    // Cosmetics
    SetStyle();

    // Lumi error hard-coded;
    LumiRelError = 0.037;

    /*
    cout << "epsAbs " << RooAbsReal::defaultIntegratorConfig()->epsAbs() << endl;
    cout << "epsRel " << RooAbsReal::defaultIntegratorConfig()->epsRel() << endl;
    RooAbsReal::defaultIntegratorConfig()->setEpsRel(1e-9);
    RooAbsReal::defaultIntegratorConfig()->setEpsAbs(1e-9);
    */

    // Container for the plots
    OutputDir = (TString) outputdir;
    gSystem->Exec("mkdir -p " + OutputDir);
    gSystem->Exec("mkdir -p " + OutputDir + "/LatexFileNPs");
    gSystem->Exec("mkdir -p " + OutputDir + "/TextFileFitResult");
    outputfile = new TFile(OutputDir+"/FitCrossChecks.root","RECREATE");

    // Load workspace, model and data
    TFile *file = TFile::Open(infile);
    if (!file) {
      cout << "The file " << infile << " is not found/created, will stop here." << endl;
      return;
    }
    if(!(RooWorkspace*) file->Get(workspaceName)){
      cout <<"workspace not found" << endl;
      return;
    }

    w      = (RooWorkspace*) file->Get(workspaceName);
    mc     = (ModelConfig*) w->obj(modelConfigName);
    data   = w->data(ObsDataName);

    w->SetName("w");
    w->SetTitle("w");
    // save snapshot before any fit has been done
    RooSimultaneous* pdf = (RooSimultaneous*) w->pdf("simPdf");
    if(!pdf) pdf = (RooSimultaneous*) w->pdf("combPdf");
    RooArgSet* params = (RooArgSet*) pdf->getParameters(*data) ;
    if(!w->loadSnapshot("snapshot_paramsVals_initial"))  w->saveSnapshot("snapshot_paramsVals_initial",*params);
    else cout << endl << " Snapshot 'snapshot_paramsVals_initial' already exists in  workspace, will not overwrite it" << endl;
    if(!data || !mc){
      w->Print();
      cout << "data or ModelConfig was not found" <<endl;
      return;
    }

    // Some sanity checks on the workspace
    if ( !IsSimultaneousPdfOK() ) return;
    if (   !IsChannelNameOK()   ) return;
    GetNominalValueNuisancePara();
    AllNPafterEachFit.clear();

    // Print some information
    //bool Verbose(false); // original
    bool Verbose(true); // BBT, Aug 30 2016
    if(Verbose) {
      PrintModelObservables();
      PrintNuisanceParameters();
      PrintSubChannels();
    }

    // Prepare the directory structure of the outputfile
    MainDirSyst              = (TDirectory*) outputfile->mkdir("PlotsBeforeFit");
    MainDirMorphing          = (TDirectory*) outputfile->mkdir("PlotsMorphing");
    MainDirFitEachSubChannel = (TDirectory*) outputfile->mkdir("PlotsAfterFitOnSubChannel");
    MainDirFitGlobal         = (TDirectory*) outputfile->mkdir("PlotsAfterGlobalFit");
    MainDirModelInspector    = (TDirectory*) outputfile->mkdir("PlotsNuisanceParamVSmu");
    MainDirStatTest          = (TDirectory*) outputfile->mkdir("PlotsStatisticalTest");
    MainDirFitAsimov         = (TDirectory*) outputfile->mkdir("PlotsAfterFitToAsimov");
    gROOT->cd();

    /*
    int maxIter = ROOT::Math::MinimizerOptions::DefaultMaxIterations();
    ROOT::Math::MinimizerOptions::SetDefaultMaxIterations( maxIter*2 );
    cout << "Changing max number of iterations from " << maxIter << " to " << ROOT::Math::MinimizerOptions::DefaultMaxIterations() << endl;
    int maxFC( ROOT::Math::MinimizerOptions::DefaultMaxFunctionCalls() );
    ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls( maxFC*2 );
    cout << "Changing max number of function calls from " << maxFC << " to " << ROOT::Math::MinimizerOptions::DefaultMaxFunctionCalls() << endl;
    */

  }

  void Finalize() {
    outputfile->Close();

    PrintSuspiciousNPs();
  }

  RooArgList getFloatParList( const RooAbsPdf& pdf, const RooArgSet& obsSet ) {
    RooArgList floatParList;

    const RooArgSet* pars = pdf.getParameters( obsSet );
    if (pars==0) { return floatParList; }

    TIterator* iter = pars->createIterator() ;
    RooAbsArg* arg ;
    while( (arg=(RooAbsArg*)iter->Next()) ) {
      if(arg->InheritsFrom("RooRealVar") && !arg->isConstant()){
        floatParList.add( *arg );
      }
    }
    delete iter;

    return floatParList;
  }

}




//============================================================
// ================= Executable function =====================
//============================================================


enum Algs { PlotHistosBeforeFit=0, PlotMorphingControlPlots, PlotHistosAfterFitEachSubChannel, PlotHistosAfterFitGlobal, PlotsNuisanceParametersVSmu, PlotsStatisticalTest, FitToAsimov };

void FitCrossCheckForLimits(const Algs algorithm         = PlotHistosBeforeFit,
                            float mu                    = 0,
                            float sigma                 = 1,
                            bool IsConditional          = true,
                            const char* infile          = "WorkspaceForTest1.root",
                            const char* outputdir       = "./results/",
                            const char* workspaceName   = "combined",
                            const char* modelConfigName = "ModelConfig",
                            const char* ObsDataName     = "obsData",
                            bool draw1DResponse         = false) {

  LimitCrossCheck::Initialize(infile, outputdir, workspaceName, modelConfigName, ObsDataName);

  // set number of toys per job
  //int nToys = 20; // original
  //int nToys = 100; // BBT, Aug 30 2016 --> trying to understand why ttbar NF != in getLimit
  int nToys = 500; // BBT, Aug 30 2016 --> trying to understand why ttbar NF != in getLimit
  

  // enable NLL scan plots (does not make sense for all algorithms)
  LimitCrossCheck::draw1DResponse = draw1DResponse;

  // determine job id to set random seed for toys and parallel processing of toys / NLL scans
  LimitCrossCheck::nJobs = 1;
  LimitCrossCheck::iJob = 0;
  TPRegexp pat(".*_job(\\d+)of(\\d+).*");
  TObjArray* res = pat.MatchS(infile);
  if (res->GetSize() >= 3) {
    if (res->At(1) && res->At(2)) {
      int nJobsTmp = atoi(((TObjString*) (res->At(2)))->GetString());
      int iJobTmp = atoi(((TObjString*) (res->At(1)))->GetString());
      if (nJobsTmp > 0 && iJobTmp >= 0 && iJobTmp < nJobsTmp) {
	LimitCrossCheck::nJobs = nJobsTmp;
	LimitCrossCheck::iJob = iJobTmp;
      }
    }
  }
  cout << "Running job " << LimitCrossCheck::iJob << " of " << LimitCrossCheck::nJobs << endl;

  switch(algorithm) {
    // -----------------------------------------------------------------------------------
    // - Plot nominal and +/- Nsigma (for each nuisance paramater) for Data, signal+bkg
    // -----------------------------------------------------------------------------------
    case PlotHistosBeforeFit:
      LimitCrossCheck::PlotHistosBeforeFit(sigma,mu); // (nSigma,mu)
    break;

    // -----------------------------------------------------------------------------------
    // - Control plots for morphing (ie, -1/0/+1 sigma --> continuous NP)
    // -----------------------------------------------------------------------------------
    case PlotMorphingControlPlots:
      LimitCrossCheck::PlotMorphingControlPlots();
    break;

    // ----------------------------------------------------------------------------------
    // - Plot histograms after unconditional fit (theta and mu fitted at the same time)
    // - Or
    // - Plot the unconditionnal fitted nuisance paramters value (theta fitted while mu is fixed)
    // ----------------------------------------------------------------------------------
    case PlotHistosAfterFitEachSubChannel:
      LimitCrossCheck::PlotHistosAfterFitEachSubChannel(IsConditional, mu);
    break;
    case PlotHistosAfterFitGlobal:
      LimitCrossCheck::PlotHistosAfterFitGlobal(IsConditional, mu, false);
    break;

    // -------------------------------------------
    // - Plot the nuisance parameters versus mu
    // -------------------------------------------
    case PlotsNuisanceParametersVSmu:
      LimitCrossCheck::PlotsNuisanceParametersVSmu();
    break;

    // -------------------------------------------
    // - Plot the pulls and stat test from toys
    // -------------------------------------------
    case PlotsStatisticalTest:
      // For this algorithm mu is the mu used for pseudo-data
      // sigma is the hypothetized mu for the statistical tests
      LimitCrossCheck::PlotsStatisticalTest(mu, sigma, nToys, LimitCrossCheck::iJob + 1000 * LimitCrossCheck::nJobs);
    break;

    // -------------------------------------------
    // - Do a global fit to the asimov dataset
    // -------------------------------------------
    case FitToAsimov:
      // For this algorithm mu is the mu used for pseudo-data, and the fitting if IsConditional is true
      LimitCrossCheck::PlotHistosAfterFitGlobal(IsConditional, mu, true);
    break;

    default:
      cout << "FitCrossChecksForLimits:: ERROR: unknown Algorithm requested" << endl;
    break;
  }
  LimitCrossCheck::Finalize();
  return;

}
