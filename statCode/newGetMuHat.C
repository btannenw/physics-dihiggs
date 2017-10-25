#include "RooWorkspace.h"
#include "RooStats/ModelConfig.h"
#include "RooDataSet.h"
#include "RooMinimizerFcn.h"
#include "RooNLLVar.h"
#include "RooRealVar.h"
#include "RooSimultaneous.h"
#include "RooCategory.h"
#include "TFile.h"
#include "TCanvas.h"
#include "RooPlot.h"

// C++
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <time.h>
#include <limits>

// Root
#include "TROOT.h"
#include "TSystem.h"


#include "macros/makeAsimovData.C"
#include "macros/minimize.C"
#include "macros/findSigma.C"

bool blind(false);

double subtractError(double err12, double err1)
{
  double check(err12*err12-err1*err1);
  //if(check < 0) { cout << "NEGATIVE" << endl; }
  return sqrt(fabs(check));
}

double subtractFractionalError(double err12, double err1)
{
  double result = 1 - err1*err1/(err12*err12);
  return result;
}

double fractionalError(double err12, double err1)
{
    double result = err1*err1/(err12*err12);
    return result;
}

void SubtractAndSay(TString say, double err1_hi, double err1_lo, double err12_hi, double err12_lo)
{
  cout << say << " : + " << 1 - err1_hi*err1_hi/(err12_hi*err12_hi) << " /- " << 1 - err1_lo*err1_lo/(err12_lo*err12_lo) << endl;
  cout << " \t  : + " << subtractError(err12_hi, err1_hi) << " / - " << subtractError(err12_lo, err1_lo) << endl;
}

void newGetMuHat(const char* workspace,
        const bool doAsimov = 0,
          //0 = do observed error
          //1 = do expected mu=1 error
        const int mode = 1,
          //0 = standard
          //1 = cb
        const char* mass = "125",
        TString poiStr              = "SigXsecOverSM",
        const char* wsName          = "combined",
        const char* modelConfigName = "ModelConfig",
        const char* dataName        = "obsData",
        const bool doDummy          = false,
        // false = normal running
        // true =don't call findSigma, run just to get set names
	const bool doHesse          = false,
	double mu_asimov = 1)
        // for calling and printing Hesse for each NP set
{


  TString inFileName = "workspaces/";
  inFileName += workspace;  inFileName += "/combined/";
  inFileName += mass;       inFileName += ".root";
  TFile* file = new TFile(inFileName.Data());
  RooWorkspace* ws = (RooWorkspace*)file->Get(wsName);
  if (!ws)
  {

    cout << "ERROR::Workspace: " << wsName << " doesn't exist!" << endl;
    return;
  }
  ModelConfig* mc = (ModelConfig*)ws->obj(modelConfigName);
  if (!mc)
  {
    cout << "ERROR::ModelConfig: " << modelConfigName << " doesn't exist!" << endl;
    return;
  }


  RooDataSet* data = (RooDataSet*)ws->data(dataName);
  if (!data && doAsimov == 0)
  {
    cout << "ERROR::Dataset: " << dataName << " doesn't exist!" << endl;
    return;
  }

  cout << "Loaded Workspace "<< workspace << "  - "<< ws->GetName()<<" , configuration and dataset." << endl;
  //ws->loadSnapshot("nominalNuis");

  RooRealVar* poi           = 0;
  RooDataSet* asimovData    = 0;
  RooArgSet globObs;
  RooNLLVar* nll            = 0;
  double nll_val_true       = 0;
  double muhat              = 0;
  double err_guess          = 0;

  if (!doDummy)
  {
      ws->loadSnapshot("vars_final");

      poi = (RooRealVar*)mc->GetParametersOfInterest()->first();
      if( poiStr != "SigXsecOverSM" ) {
          poi->setConstant(0);
          cout << "SWITCHING POI TO " << poiStr << endl;
          poi = ws->var(poiStr.Data());
          if(!poi) {
              cout << "Cannot find " << poiStr << " in workspace" << endl;
              return;
          }
      }

      //RooDataSet* asimovData = makeAsimovData(mc, 0, ws, mc->GetPdf(), data, 1); // unconditional fit and build asimov with mu=1
      mc->GetGlobalObservables()->Print("v");
      globObs = *mc->GetGlobalObservables();
      if (doAsimov && !doDummy) {
	asimovData = makeAsimovData(mc, 0, ws, mc->GetPdf(), data, mu_asimov); // unconditional fit and build asimov with mu=mu_asimov
	data = asimovData;
      }
      cout << "Made Asimov data. with mu = " << mu_asimov << endl;

      if (!doDummy) {
	TString nCPU_str = getenv("NCORE");
	int nCPU = nCPU_str.Atoi();
	nll = (RooNLLVar*)mc->GetPdf()->createNLL(*data, GlobalObservables(globObs), Offset(1), Optimize(2), NumCPU(nCPU,3)/*, Constrain(nuis)*/);
      }
      //  nll->enableOffsetting(kTRUE);

      cout << "Created NLL." << endl;

      ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2");
      ROOT::Math::MinimizerOptions::SetDefaultStrategy(1);
      ROOT::Math::MinimizerOptions::SetDefaultPrintLevel(-1);

      poi->setConstant(0);
      cout << "POI" << endl;
      cout << poi->getMin() << endl;
      cout << poi->getMax() << endl;

      cout << "setting min to -max" << endl;
      poi->setMin( -1 * poi->getMax() );

      minimize(nll);
      cout << "Minimum2 " << nll->getVal() << endl;

      RooArgSet nuisAndPOI(*mc->GetNuisanceParameters(),*mc->GetParametersOfInterest());
      ws->saveSnapshot("tmp_shot",nuisAndPOI);

      nll_val_true = nll->getVal();
      muhat = poi->getVal();
      err_guess = poi->getError();

      typedef std::numeric_limits< double > dbl;
      cout.precision(dbl::digits10);
      cout << "nll_val_true " << nll_val_true << endl;
      if(!blind) { cout << "muhat " << muhat << endl; }
      cout << "err_guess " << err_guess << endl;
  }



  const RooArgSet* nuis = mc->GetNuisanceParameters();
  TIterator* nitr = nuis->createIterator();
  RooRealVar* var;

  const int npsets_max  = 30;
  const int nstrmax = 30;
  TString *npsetname[npsets_max];
  TString *npsetstring[npsets_max][nstrmax];
  int npnstr[npsets_max];
  int npsets = 0;
  bool npsetexclude[npsets_max];

  double err_raw_hi[npsets_max];
  double err_raw_lo[npsets_max];
  double err_npset_hi[npsets_max];
  double err_npset_lo[npsets_max];

  for (int iset = 0; iset < npsets_max; iset++){
      npsetname[iset]  = new TString("xxxxx");
      for (int istr = 0; istr < nstrmax; istr++){
          npsetstring[iset][istr]  = new TString("xxxxx");
      }
      npsetexclude[iset]  = false;
      err_raw_hi[iset]  = 0.0;
      err_raw_lo[iset]  = 0.0;
      err_npset_hi[iset]  = 0.0;
      err_npset_lo[iset]  = 0.0;
  }

  //*********************************************************************************
  //*********************************************************************************
  //                           Common to all modes
  //*********************************************************************************
  //*********************************************************************************

    npsetname[0] = new TString("Total");

    npsetname[1]        = new TString("DataStat");
    npsetexclude[1]     = true;

    npsetname[2]        = new TString("FullSyst");

  // Warning: Start all NP set indexes with 2 in the mode-specific section!
    
  //*********************************************************************************
  //*********************************************************************************
  //                           Mode 0
  //*********************************************************************************
  //*********************************************************************************


  if (mode == 0)
  {

    npsetname[3]        = new TString("Floating norms");
    npsetstring[3][0]   = new TString("norm");

    npsetname[4]        = new TString("All Norms");
    npsetstring[4][0]   = new TString("norm");
    npsetstring[4][1]   = new TString("Norm");

    npsetname[5]        = new TString("AllOtherSysts");
    npsetstring[5][0]   = new TString("norm");
    npsetstring[5][1]   = new TString("Norm");
    npsetexclude[5]     = true;

    npsetname[6]        = new TString("ThH");
    npsetstring[6][0]   = new TString("Theory");

    npsetname[7]        = new TString("ThD");
    npsetstring[7][0]   = new TString("VV");




  //*********************************************************************************
  //*********************************************************************************
  //                           Mode 1
  //*********************************************************************************
  //*********************************************************************************
  }
  else if (mode == 1)
  {

    npsetname[3]        = new TString("Floating normalizations");
    npsetstring[3][0]     = new TString("norm");

    npsetname[4]        = new TString("All normalizations");
    npsetstring[4][0]   = new TString("norm");
    npsetstring[4][1]   = new TString("Norm");

    npsetname[5]        = new TString("All but normalizations");
    npsetstring[5][0]   = new TString("norm");
    npsetstring[5][1]   = new TString("Norm");
    npsetexclude[5]     = true;

    npsetname[6]        = new TString("Jets MET");
    npsetstring[6][0]   = new TString("SysJET");
    npsetstring[6][1]   = new TString("SysMET");
    npsetstring[6][2]   = new TString("SysFATJET");
    npsetstring[6][3]   = new TString("SysJER");

    npsetname[7]        = new TString("BTag");
    npsetstring[7][0]   = new TString("SysFT");

    npsetname[8]        = new TString("Leptons");
    npsetstring[8][0]   = new TString("SysEG");
    npsetstring[8][1]   = new TString("SysEL");
    npsetstring[8][2]   = new TString("SysMUONS");
      
    npsetname[9]        = new TString("Luminosity");
    npsetstring[9][0]     = new TString("LUMI");

    npsetname[10]        = new TString("Diboson");
    npsetstring[10][0]   = new TString("VV");
    npsetstring[10][1]   = new TString("WW");
    npsetstring[10][2]   = new TString("ZZ");
    npsetstring[10][3]   = new TString("WZ");

    npsetname[11]        = new TString("Zjets");
    npsetstring[11][0]   = new TString("ZDPhi");
    npsetstring[11][1]   = new TString("ZMbb");
    npsetstring[11][2]   = new TString("ZPt");
    npsetstring[11][3]   = new TString("Zcl");
    npsetstring[11][4]   = new TString("Zbb");
    npsetstring[11][5]   = new TString("Zl");
    npsetstring[11][6]   = new TString("Zbc");
    npsetstring[11][7]   = new TString("Zbl");
    npsetstring[11][8]   = new TString("Zcc");

    npsetname[12]        = new TString("Wjets");
    npsetstring[12][0]   = new TString("WDPhi");
    npsetstring[12][1]   = new TString("WMbb");
    npsetstring[12][2]   = new TString("WbbMbb");
    npsetstring[12][3]   = new TString("WPt");
    npsetstring[12][4]   = new TString("Wbb");
    npsetstring[12][5]   = new TString("Wbc");
    npsetstring[12][6]   = new TString("Wbl");
    npsetstring[12][7]   = new TString("Wcc");
    npsetstring[12][8]   = new TString("Wcl");
    npsetstring[12][9]   = new TString("Wl");

    npsetname[13]        = new TString("Model ttbar");
    npsetstring[13][0]   = new TString("TtBar");
    npsetstring[13][1]   = new TString("ttBarHigh");
    npsetstring[13][2]   = new TString("TopPt");
    npsetstring[13][3]   = new TString("TTbarPTV");
    npsetstring[13][4]   = new TString("TTbarMBB");
    npsetstring[13][5]   = new TString("ttbar");

    npsetname[14]        = new TString("Model Single Top");
    npsetstring[14][0]   = new TString("stop");
    npsetstring[14][1]   = new TString("Stop");
      
    npsetname[15]        = new TString("Model Multi Jet");
    npsetstring[15][0]   = new TString("MJ");
      
    npsetname[16]        = new TString("Signal Systematics");
    npsetstring[16][0]     = new TString("Theory");
    npsetstring[16][1]     = new TString("VH");
      

    npsetname[17]        = new TString("MC stat");
    npsetstring[17][0]     = new TString("gamma");




  //*********************************************************************************
  //*********************************************************************************
  //                           Mode 2
  //*********************************************************************************
  //*********************************************************************************
  }

  if (mode == 2)
  {

    npsetname[3]        = new TString("Floating norms");
    npsetstring[3][0]   = new TString("norm");

    npsetname[4]        = new TString("All Norms");
    npsetstring[4][0]   = new TString("norm");
    npsetstring[4][1]   = new TString("Norm");

    npsetname[5]        = new TString("AllOtherSysts");
    npsetstring[5][0]   = new TString("norm");
    npsetstring[5][1]   = new TString("Norm");
    npsetexclude[5]     = true;

    npsetname[6]        = new TString("ThH");
    npsetstring[6][0]   = new TString("Theory");

    npsetname[7]        = new TString("ThD");
    npsetstring[7][0]   = new TString("VV");

    //npsetname[8]        = new TString("b-tagging");
    //npsetstring[8][0]   = new TString("SysFT_EFF_Eigen_B_0_AntiKt4EMTopoJets");
    //npsetstring[8][1]   = new TString("SysFT_EFF_Eigen_B_1_AntiKt4EMTopoJets");
    //npsetstring[8][2]   = new TString("SysFT_EFF_Eigen_B_2_AntiKt4EMTopoJets");
    //npsetstring[8][3]   = new TString("SysFT_EFF_Eigen_B_3_AntiKt4EMTopoJets");
    npsetname[8]        = new TString("b-tagging: charm");
    npsetstring[8][0]   = new TString("SysFT_EFF_Eigen_C_0_AntiKt4EMTopoJets");
    npsetstring[8][1]   = new TString("SysFT_EFF_Eigen_C_1_AntiKt4EMTopoJets");
    npsetstring[8][2]   = new TString("SysFT_EFF_Eigen_C_2_AntiKt4EMTopoJets");
    npsetstring[8][3]   = new TString("SysFT_EFF_Eigen_C_3_AntiKt4EMTopoJets");
    npsetname[9]        = new TString("b-tagging; light");
    npsetstring[9][0]   = new TString("SysFT_EFF_Eigen_Light_0_AntiKt4EMTopoJets");
    npsetstring[9][1]   = new TString("SysFT_EFF_Eigen_Light_1_AntiKt4EMTopoJets");
    npsetstring[9][2]   = new TString("SysFT_EFF_extrapolation_AntiKt4EMTopoJets");
    npsetstring[9][3]   = new TString("SysFT_EFF_extrapolation_from_charm_AntiKt4EMTopoJets");

    npsetname[10]        = new TString("JES");
    npsetstring[10][0]   = new TString("alpha_SysJET_SR1_JET_GroupedNP_1");
    npsetstring[10][1]   = new TString("alpha_SysJET_SR1_JET_GroupedNP_2");
    npsetstring[10][2]   = new TString("alpha_SysJET_SR1_JET_GroupedNP_3");

    npsetname[11]        = new TString("ttbar mod: scale");
    npsetstring[11][0]   = new TString("alpha_ttbar_scale");
  
    npsetname[12]        = new TString("ttbar mod: PS");
    npsetstring[12][0]   = new TString("alpha_ttbar_PS");

    npsetname[13]        = new TString("ttbar mod: ISR/FSR");
    npsetstring[13][0]   = new TString("alpha_ttbar_radLo");
    npsetstring[13][1]   = new TString("alpha_ttbar_radHi");

    npsetname[14]        = new TString("ttbar mod: ME");
    npsetstring[14][0]   = new TString("alpha_ttbar_ME");

    npsetname[15]        = new TString("Wjets Mod");
    npsetstring[15][0]   = new TString("SysWv22Norm");

    npsetname[16]        = new TString("JER");
    npsetstring[16][0]   = new TString("SysJET_JER_SINGLE_NP");

  //*********************************************************************************
  //*********************************************************************************
  //                           Mode 3
  //                      VH run 1 paper table
  //*********************************************************************************
  //*********************************************************************************
  }  else if (mode == 3)
  {

    int i = 2;
    int j = 0;

    i++; j = 0;
    npsetname[i]        = new TString("W+jets");
    npsetstring[i][j++]     = new TString("WPtV");
    npsetstring[i][j++]     = new TString("WDPhi");
    npsetstring[i][j++]     = new TString("WMbb");
    npsetstring[i][j++]     = new TString("WbbRatio");
    npsetstring[i][j++]     = new TString("WlNorm");
    npsetstring[i][j++]     = new TString("WclNorm");
    npsetstring[i][j++]     = new TString("WhfNorm");
    npsetstring[i][j++]     = new TString("norm_W");

    i++; j = 0;
    npsetname[i]        = new TString("W+jets norm");
    npsetstring[i][j++]     = new TString("norm_W");

    i++; j = 0;
    npsetname[i]        = new TString("W+jets model");
    npsetstring[i][j++]     = new TString("WPtV");
    npsetstring[i][j++]     = new TString("WDPhi");
    npsetstring[i][j++]     = new TString("WMbb");
    npsetstring[i][j++]     = new TString("WbbRatio");
    npsetstring[i][j++]     = new TString("WlNorm");
    npsetstring[i][j++]     = new TString("WclNorm");
    npsetstring[i][j++]     = new TString("WhfNorm");

    i++; j = 0;
    npsetname[i]        = new TString("Z+jets");
    npsetstring[i][j++]     = new TString("ZPtV");
    npsetstring[i][j++]     = new TString("ZDPhi");
    npsetstring[i][j++]     = new TString("ZMbb");
    npsetstring[i][j++]     = new TString("ZbbRatio");
    npsetstring[i][j++]     = new TString("ZlNorm");
    npsetstring[i][j++]     = new TString("ZclNorm");
    npsetstring[i][j++]     = new TString("ZbbNorm");
    npsetstring[i][j++]     = new TString("norm_Z");

    i++; j = 0;
    npsetname[i]        = new TString("Z+jets norm");
    npsetstring[i][j++]     = new TString("norm_Z");

    i++; j = 0;
    npsetname[i]        = new TString("Z+jets modeling");
    npsetstring[i][j++]     = new TString("ZPtV");
    npsetstring[i][j++]     = new TString("ZDPhi");
    npsetstring[i][j++]     = new TString("ZMbb");
    npsetstring[i][j++]     = new TString("ZbbRatio");
    npsetstring[i][j++]     = new TString("ZlNorm");
    npsetstring[i][j++]     = new TString("ZclNorm");
    npsetstring[i][j++]     = new TString("ZbbNorm");

    i++; j = 0;
    npsetname[i]        = new TString("Signal Theory");
    npsetstring[i][j++]     = new TString("Theory");

    i++; j = 0;
    npsetname[i]        = new TString("All b-tagging");
    npsetstring[i][j++]     = new TString("BTag");
    npsetstring[i][j++]     = new TString("TruthTagDR");

    i++; j = 0;
    npsetname[i]        = new TString("Diboson");
    npsetstring[i][j++]     = new TString("VVMbb");
    npsetstring[i][j++]     = new TString("VVJet");

    i++; j = 0;
    npsetname[i]        = new TString("b-tagging (c-jets)");
    npsetstring[i][j++]     = new TString("BTagC");
    npsetstring[i][j++]     = new TString("TruthTagDR");

    i++; j = 0;
    npsetname[i]        = new TString("b-tagging (b-jets)");
    npsetstring[i][j++]     = new TString("BTagB");

    i++; j = 0;
    npsetname[i]        = new TString("b-tagging (light-jets)");
    npsetstring[i][j++]     = new TString("BTagL");

    i++; j = 0;
    npsetname[i]        = new TString("MJ");
    npsetstring[i][j++]     = new TString("MJ");

    i++; j = 0;
    npsetname[i]        = new TString("Jets");
    npsetstring[i][j++]     = new TString("JVF");
    npsetstring[i][j++]     = new TString("JetN");
    npsetstring[i][j++]     = new TString("JetE");
    npsetstring[i][j++]     = new TString("JetF");
    npsetstring[i][j++]     = new TString("JetPile");
    npsetstring[i][j++]     = new TString("JetB");
    npsetstring[i][j++]     = new TString("JetM");
    npsetstring[i][j++]     = new TString("JetR");

    i++; j = 0;
    npsetname[i]        = new TString("MET");
    npsetstring[i][j++]     = new TString("MET");

    i++; j = 0;
    npsetname[i]        = new TString("Jets&MET");
    npsetstring[i][j++]     = new TString("JVF");
    npsetstring[i][j++]     = new TString("JetN");
    npsetstring[i][j++]     = new TString("JetE");
    npsetstring[i][j++]     = new TString("JetF");
    npsetstring[i][j++]     = new TString("JetPile");
    npsetstring[i][j++]     = new TString("JetB");
    npsetstring[i][j++]     = new TString("JetM");
    npsetstring[i][j++]     = new TString("JetR");
    npsetstring[i][j++]     = new TString("MET");

    i++; j = 0;
    npsetname[i]        = new TString("Leptons");
    npsetstring[i][j++]     = new TString("Elec");
    npsetstring[i][j++]     = new TString("Muon");
    npsetstring[i][j++]     = new TString("LepVeto");

    i++; j = 0;
    npsetname[i]        = new TString("Single Top");
    npsetstring[i][j++]     = new TString("stop");
    npsetstring[i][j++]     = new TString("Chan");

    i++; j = 0;
    npsetname[i]        = new TString("ttbar");
    npsetstring[i][j++]     = new TString("norm_ttbar");
    npsetstring[i][j++]     = new TString("TopPt");
    npsetstring[i][j++]     = new TString("Ttbar");
    npsetstring[i][j++]     = new TString("ttbarHighPtV");
    npsetstring[i][j++]     = new TString("ttbarNorm");

    i++; j = 0;
    npsetname[i]        = new TString("ttbar Norm");
    npsetstring[i][j++]     = new TString("norm_ttbar");

    i++; j = 0;
    npsetname[i]        = new TString("ttbar Model");
    npsetstring[i][j++]     = new TString("TopPt");
    npsetstring[i][j++]     = new TString("Ttbar");
    npsetstring[i][j++]     = new TString("ttbarHighPtV");
    npsetstring[i][j++]     = new TString("ttbarNorm");

    i++; j = 0;
    npsetname[i]        = new TString("Float + Norm");
    npsetstring[i][j++]     = new TString("norm");
    npsetstring[i][j++]     = new TString("Norm");
    npsetstring[i][j++]     = new TString("Ratio");
    npsetstring[i][j++]     = new TString("MJ_");

    i++; j = 0;
    npsetname[i]        = new TString("Float");
    npsetstring[i][j++]     = new TString("norm");

    i++; j = 0;
    npsetname[i]        = new TString("Model Norm");
    npsetstring[i][j++]     = new TString("Norm");
    npsetstring[i][j++]     = new TString("Ratio");
    npsetstring[i][j++]     = new TString("MJ_");

    i++; j = 0;
    npsetname[i]        = new TString("Lumi");
    npsetstring[i][j++]     = new TString("LUMI_2012");

  }
  
  // STUDYING JES SYSTEMATICS 
  // comparing 19NP and 3NP scheme for SM VH analysis
   else if (mode == 4)
  {
    npsetname[3]         = new TString("19NP JES systematics");
    npsetstring[3][0]    = new TString("JET_19NP_JET_EffectiveNP_1");
    npsetstring[3][1]    = new TString("JET_19NP_JET_EffectiveNP_2");
    npsetstring[3][2]    = new TString("JET_19NP_JET_EffectiveNP_3");
    npsetstring[3][3]    = new TString("JET_19NP_JET_EffectiveNP_4");
    npsetstring[3][4]    = new TString("JET_19NP_JET_EffectiveNP_5");
    npsetstring[3][5]    = new TString("JET_19NP_JET_EffectiveNP_6restTerm");
    npsetstring[3][6]    = new TString("JET_19NP_JET_EtaIntercalibration_Modelling");
    npsetstring[3][7]    = new TString("JET_19NP_JET_EtaIntercalibration_TotalStat");
    npsetstring[3][8]    = new TString("JET_19NP_JET_EtaIntercalibration_NonClosure");
    npsetstring[3][9]    = new TString("JET_19NP_JET_Pileup_OffsetMu");
    npsetstring[3][10]   = new TString("JET_19NP_JET_Pileup_OffsetNPV");
    npsetstring[3][11]   = new TString("JET_19NP_JET_Pileup_PtTerm");
    npsetstring[3][12]   = new TString("JET_19NP_JET_Pileup_RhoTopology");
    npsetstring[3][13]   = new TString("JET_19NP_JET_Flavor_Composition");
    npsetstring[3][14]   = new TString("JET_19NP_JET_Flavor_Response");
    npsetstring[3][15]   = new TString("JET_19NP_JET_BJES_Response");
    npsetstring[3][16]   = new TString("JET_19NP_JET_PunchThrough_MCTYPE");
    npsetstring[3][17]   = new TString("JET_19NP_JET_SingleParticle_HighPt");

    npsetname[4]         = new TString("19NP JES systematics: Eff NP");
    npsetstring[4][0]    = new TString("JET_19NP_JET_EffectiveNP_1");
    npsetstring[4][1]    = new TString("JET_19NP_JET_EffectiveNP_2");
    npsetstring[4][2]    = new TString("JET_19NP_JET_EffectiveNP_3");
    npsetstring[4][3]    = new TString("JET_19NP_JET_EffectiveNP_4");
    npsetstring[4][4]    = new TString("JET_19NP_JET_EffectiveNP_5");
    npsetstring[4][5]    = new TString("JET_19NP_JET_EffectiveNP_6restTerm");

    //npsetname[3]        = new TString("3NP JES systematics");
    //npsetstring[3][0]   = new TString("JET_SR1_JET_GroupedNP_1");
    //npsetstring[3][1]   = new TString("JET_SR1_JET_GroupedNP_2");
    //npsetstring[3][2]   = new TString("JET_SR1_JET_GroupedNP_3");
    //npsetstring[3][3]   = new TString("JET_19NP_JET_EtaIntercalibration_NonClosure");

    npsetname[5]        = new TString("All but JES systematics");
    npsetstring[5][0]    = new TString("JET_19NP_JET_EffectiveNP_1");
    npsetstring[5][1]    = new TString("JET_19NP_JET_EffectiveNP_2");
    npsetstring[5][2]    = new TString("JET_19NP_JET_EffectiveNP_3");
    npsetstring[5][3]    = new TString("JET_19NP_JET_EffectiveNP_4");
    npsetstring[5][4]    = new TString("JET_19NP_JET_EffectiveNP_5");
    npsetstring[5][6]    = new TString("JET_19NP_JET_EffectiveNP_6restTerm");
    npsetstring[5][6]    = new TString("JET_19NP_JET_EtaIntercalibration_Modelling");
    npsetstring[5][7]    = new TString("JET_19NP_JET_EtaIntercalibration_TotalStat");
    npsetstring[5][8]    = new TString("JET_19NP_JET_EtaIntercalibration_NonClosure");
    npsetstring[5][9]    = new TString("JET_19NP_JET_Pileup_OffsetMu");
    npsetstring[5][10]   = new TString("JET_19NP_JET_Pileup_OffsetNPV");
    npsetstring[5][11]   = new TString("JET_19NP_JET_Pileup_PtTerm");
    npsetstring[5][12]   = new TString("JET_19NP_JET_Pileup_RhoTopology");
    npsetstring[5][13]   = new TString("JET_19NP_JET_Flavor_Composition");
    npsetstring[5][14]   = new TString("JET_19NP_JET_Flavor_Response");
    npsetstring[5][15]   = new TString("JET_19NP_JET_BJES_Response");
    npsetstring[5][16]   = new TString("JET_19NP_JET_PunchThrough_MCTYPE");
    npsetstring[5][17]   = new TString("JET_19NP_JET_SingleParticle_HighPt");
    //npsetstring[4][0]   = new TString("JET_SR1_JET_GroupedNP_1");
    //npsetstring[4][1]   = new TString("JET_SR1_JET_GroupedNP_2");
    //npsetstring[4][2]   = new TString("JET_SR1_JET_GroupedNP_3");
    //npsetstring[4][3]   = new TString("JET_19NP_JET_EtaIntercalibration_NonClosure");
    npsetexclude[5]     = true;
  }

   else if (mode == 5)
  {
    npsetname[3]         = new TString("Top shape systematics");
    //npsetstring[3][0]    = new TString("SysTTbarPTV");
    //npsetstring[3][1]    = new TString("SysTTbarMBB");
    npsetstring[3][0]    = new TString("SysTTbarPTVMBB");
    //
    npsetname[4]         = new TString("All but top shape systematics");
    //npsetstring[4][0]    = new TString("SysTTbarPTV");
    //npsetstring[4][1]    = new TString("SysTTbarMBB");
    npsetstring[4][0]    = new TString("SysTTbarPTVMBB");
    npsetexclude[4]     = true;
  }

    else if (mode == 5)
    {
        npsetname[3]        = new TString("MC stat");
        npsetstring[3][0]     = new TString("gamma");
        
        npsetname[4]        = new TString("All Sys");
        npsetstring[4][0]     = new TString("gamma");
        npsetexclude[4]     = true;
    }
    
    else if (mode == 6)
    {
        npsetname[3]        = new TString("TTbar syst");
        npsetstring[3][0]     = new TString("TTbar");
        
        npsetname[4]        = new TString("TTbar syst and ttbar norm");
        npsetstring[4][0]     = new TString("TTbar");
        npsetstring[4][1]     = new TString("ttbar");
        
        npsetname[4]        = new TString("All but TTBar syst");
        npsetstring[4][0]     = new TString("TTbar");
        npsetexclude[4]     = true;
        
        npsetname[5]        = new TString("All but TTBar syst and ttbar norm");
        npsetstring[5][0]     = new TString("TTbar");
        npsetstring[5][1]     = new TString("ttbar");
        npsetexclude[5]     = true;
        
    }
    else if (mode == 7)
    {
        npsetname[3]        = new TString("V syst");
        npsetstring[3][0]     = new TString("ZPtV");
        npsetstring[3][1]     = new TString("WPtV");
        npsetstring[3][2]     = new TString("ZMbb");
        npsetstring[3][3]     = new TString("WMbb");

        
        npsetname[4]        = new TString("All but V syst");
        npsetstring[4][0]     = new TString("ZPtV");
        npsetstring[4][1]     = new TString("WPtV");
        npsetstring[4][2]     = new TString("ZMbb");
        npsetstring[4][3]     = new TString("WMbb");
        npsetexclude[4]     = true;
        
    }
  //*********************************************************************************
  //*********************************************************************************
  //                           End of mode-specific settings
  //*********************************************************************************
  //*********************************************************************************


  // This must come **after** setting npnstr
  int setnamemax = 0;
  for (int iset = 0; iset < npsets_max; iset++){
      npnstr[iset]        = 0;
      for (int istr = 0; istr < nstrmax; istr++){
          if (npsetstring[iset][istr]->CompareTo("xxxxx")) npnstr[iset]++;
      }
       if (npsetname[iset]->CompareTo("xxxxx")) npsets++;

  }

  for (int iset = 0; iset < npsets; iset++)
  {
      if (setnamemax < npsetname[iset]->Length()) setnamemax = npsetname[iset]->Length();
      if (iset == 2) {// FullSyst is just Total -Stat (quadratically)
          err_raw_hi[iset] = subtractError(err_raw_hi[0], err_raw_hi[1]);
          err_raw_lo[iset] = subtractError(err_raw_lo[0], err_raw_lo[1]);
          err_npset_lo[iset] = err_raw_lo[iset];
          err_npset_hi[iset] = err_raw_hi[iset];
          cout << npsetname[iset]->Data() << " errors from quadratical subrtraction of Total and Stat." <<endl;
          cout << npsetname[iset]->Data() << " _err_npset_hi same as raw: " << err_npset_hi[iset] <<endl;
          cout << npsetname[iset]->Data() << " _err_npset_lo same as raw:  " << err_npset_lo[iset] <<endl;
          continue; 
      }
      
      if (!doDummy) ws->loadSnapshot("tmp_shot");
      nitr->Reset();
      cout << endl << "Holding constant the "<<npsetname[iset]->Data() <<
      " nuisance parameter set, which is: "<< endl;

      while ((var = (RooRealVar*)nitr->Next()))
      {
          bool aux_isnp = false;
          for (int istr = 0; istr < npnstr[iset]; istr++)
          {
              aux_isnp = aux_isnp ||
              (string(var->GetName()).find(npsetstring[iset][istr]->Data())
              != string::npos);// is this var the iset _th NP?
          }

          if (npsetexclude[iset]) aux_isnp = !aux_isnp;
          if (aux_isnp) var->setConstant(1);
          else { continue; }
          cout << "\t" << var->GetName() << endl;
      }

      if (!doDummy){

	if (doHesse) {
          minimize(nll, true);
          if(!blind) { cout << "muhat " << poi->getVal() << endl; }
          cout << "err_hesse " << poi->getError() << endl;
	}

          err_raw_hi[iset] = findSigma(nll, nll_val_true, poi, muhat+err_guess, muhat, 1, 0.01);
          err_raw_lo[iset] = findSigma(nll, nll_val_true, poi, muhat+err_guess, muhat, -1, 0.01);
      }

      cout << npsetname[iset]->Data() << " constant"<< endl;
      cout << npsetname[iset]->Data() << " _err_raw_hi straight from fit: " << err_raw_hi[iset] <<endl;
      cout << npsetname[iset]->Data() << " _err_raw_lo straight from fit: " << err_raw_lo[iset] <<endl;
      
      err_npset_lo[iset] = err_raw_lo[iset];
      err_npset_hi[iset] = err_raw_hi[iset];
      // if (iset != 0 && npsetexclude[iset] == false) {
      if (iset != 0 && iset != 1) {
          err_npset_hi[iset] = subtractError(err_raw_hi[0], err_raw_hi[iset]);
          err_npset_lo[iset] = subtractError(err_raw_lo[0], err_raw_lo[iset]);
          cout << npsetname[iset]->Data() << " _err_npset_hi quadratically subtracted from tot: " << err_npset_hi[iset] <<endl;
          cout << npsetname[iset]->Data() << " _err_npset_lo quadratically subtracted from tot:  " << err_npset_lo[iset] <<endl;
      } else {
          cout << npsetname[iset]->Data() << " _err_npset_hi same as raw: " << err_npset_hi[iset] <<endl;
          cout << npsetname[iset]->Data() << " _err_npset_lo same as raw:  " << err_npset_lo[iset] <<endl;
      }
      

  }
  
   TString dirName = "./plots/";
   dirName += workspace; dirName += "/breakdown";
   system(TString("mkdir -vp "+dirName));
 
  TString fname = dirName;
  fname += "/muHatTable_";  fname += TString(workspace).ReplaceAll("/","_");
  fname += "_mode";         fname += mode;
  if (doAsimov) fname += "_Asimov1"; else fname += "_Asimov0";
  fname += "_";             fname += poiStr;
  TString fnameTex = fname;
  TString fnameTxt = fname;
  fnameTex += ".tex";
  fnameTxt += ".txt";
  ofstream fmuHatTex(fnameTex.Data());  
    fmuHatTex << endl;
  fmuHatTex << "\\begin{tabular}{|l|c|}" << endl;
  fmuHatTex << "\\hline" << endl;
  fmuHatTex << "Set of nuisance & Impact on error  \\\\\\hline" << endl;
  fmuHatTex << " parameters     &   \\\\\\hline" << endl;
  //fmuHat << "\\hline" << endl;
  
  
  time_t rawtime;  time (&rawtime);
  FILE *fmuHatTxt = fopen(fnameTxt.Data(),"w");
  fprintf(fmuHatTxt,"\n\tOutput of newGetMuHat ran at: %s\n",ctime (&rawtime));
  fprintf(fmuHatTxt,"----------------------------------------------------------\n\tOptions:\n");
  fprintf(fmuHatTxt,"Workspace: %s\n",workspace);
  if (doAsimov) fprintf(fmuHatTxt,"Asimov: 1, ");  else fprintf(fmuHatTxt,"Asimov: 0, ");
  if (doDummy)  fprintf(fmuHatTxt,"Dummy ,");
  fprintf(fmuHatTxt,"Mode: %d, \n",mode);  
  fprintf(fmuHatTxt,"Mass: %s GeV; poi: %s; wsName: %s\n",mass,poiStr.Data(),wsName);
  fprintf(fmuHatTxt,"ModelConfigName: %s ; dataName: %s\n",modelConfigName,dataName);
  fprintf(fmuHatTxt,"----------------------------------------------------------\n\n");

  if(!blind) { fprintf(fmuHatTxt, "muhat: %7.5f\n\n",muhat); }
  if(!blind) { cout << "muhat: " << muhat << endl; }
  fprintf(fmuHatTxt, "nll_val_true: %f\n\n",nll_val_true); 
  
  
  
  fprintf(fmuHatTxt,"  Set of nuisance          Impact on error\n");
  fprintf(fmuHatTxt,"       parameters          \n");
  fprintf(fmuHatTxt,"----------------------------------------------------------\n");

  cout << "Raw impact of nuisance parameter sets, quadratically substracted from total." <<endl;

   fmuHatTex.precision(3);

  for (int iset  = 0; iset < npsets; iset++){
    //if (iset == 1) continue;
      
        fmuHatTex << npsetname[iset]->Data() 
            << " & $^{+ " << err_npset_hi[iset]
            << "}_{-" << err_npset_lo[iset] <<"}$ \\\\" << endl;
        cout << npsetname[iset]->Data() << " : + " 
            << err_npset_hi[iset] << " / - "
            << err_npset_lo[iset] << endl;
            
        fprintf(fmuHatTxt,"%25s  + %6.3f  - %6.3f\n",
            npsetname[iset]->Data(), 
            err_npset_hi[iset],
            err_npset_lo[iset] );

  }
  
  fmuHatTex << "\\hline" << endl;
  fmuHatTex << "\\end{tabular}" << endl;
  fmuHatTex << endl;

  fmuHatTex << "\\begin{tabular}{|l|c|}" << endl;
  fmuHatTex << "\\hline" << endl;
  fmuHatTex << "Set of nuisance & Fractional impact on error  \\\\\\hline" << endl;
  fmuHatTex << " parameters     &  (square of fraction of total) \\\\\\hline" << endl;
  fmuHatTex << "\\hline" << endl;

  fprintf(fmuHatTxt,"Impact on error quadratically subtracted from total, except for:\n");
  for (int iset  = 0; iset < npsets; iset++){
        if (npsetexclude[iset]) fprintf(fmuHatTxt,"%s ",npsetname[iset]->Data());
    }
  fprintf(fmuHatTxt,"\n----------------------------------------------------------\n\n");

  fprintf(fmuHatTxt,"  Set of nuisance          Fractional impact on error\n");
  fprintf(fmuHatTxt,"       parameters          (square of fraction of total)\n");
  fprintf(fmuHatTxt,"----------------------------------------------------------\n");

  
  cout << endl;
  cout << "Fractional impact of nuisance parameter sets, quadratically substracted from total." <<endl;
  for (int iset  = 1; iset < npsets; iset++){
    //if (iset == 1) continue;
 
        fmuHatTex.precision(2);
        fmuHatTex << npsetname[iset]->Data() 
            << " & $^{+ " << fractionalError(err_npset_hi[0], err_npset_hi[iset])
            << "}_{-" << fractionalError(err_npset_lo[0], err_npset_lo[iset]) <<"}$ \\\\" << endl;
        cout << npsetname[iset]->Data() << " : + " 
            << fractionalError(err_npset_hi[0], err_npset_hi[iset]) << " / - "
            << fractionalError(err_npset_lo[0], err_npset_lo[iset]) << endl;

        fprintf(fmuHatTxt,"%25s  + %5.2f  - %5.2f\n",
            npsetname[iset]->Data(), 
            fractionalError(err_npset_hi[0], err_npset_hi[iset]),
            fractionalError(err_npset_lo[0], err_npset_lo[iset]) );
      
  }
  
  fmuHatTex << "\\hline" << endl;
  fmuHatTex << "\\end{tabular}" << endl;
  fmuHatTex.close();
  
  fprintf(fmuHatTxt,"----------------------------------------------------------\n\n");
  fclose(fmuHatTxt); 

  //cout << "Data - no NPs  : +" << 1 -err_npset_hi[1]/err_npset_hi[0] << " / -"
  //<< 1 - err_npset_lo[1]/err_npset_lo[0] << endl;
  cout << "DataStat - no NPs  : +" << err_npset_hi[1] << " / -" << err_npset_lo[1] << endl;
  cout << "FullSyst = Tot -(quad) Stat  : +" << err_npset_hi[2] << " / -" << err_npset_lo[2] << endl;
  cout << "TOT  : +" << err_npset_hi[0] << " / -" << err_npset_lo[0] << endl;




}
