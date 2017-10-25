//include statements
#include <stdlib.h>
#include <TFile.h>
#include <TCanvas.h>
#include <THStack.h>
#include <TH1.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TObject.h>
#include <TObjArray.h>
#include "TVector.h"
#include <cmath>
#include <iostream>
#include <string>
#include <cstddef>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "include/abcdPlotter.h"
#include "include/commonFileInput.h"
#include "include/commonPlotting.h"

void calculateABCD(TCanvas*& c0, TObjArray* array, std::string analysisRegion, std::string highPtRegion, std::string lowPtRegion, std::string fname, bool drawPlot = false, std::string userBDregion = "", bool printInformation = true);
void getIntegralAndError(double& integral, double& error, TH1D* data, TH1D* ttbar, TH1D* Wv221, TH1D* SingleTop, TH1D* Dibosons, TH1D* Z, bool ttbarOnly);
void runABCD(TCanvas *&c0, TObjArray *array, std::string region, std::string cuts, std::string variable, bool drawControlPlots = true, std::string userBDregion = "", bool printInformation = true);
void makeRatioTable(TObjArray* array, std::string sample);
void returnRegionStrings(std::string opt, std::string phase, std::string& s_A, std::string& s_B, std::string& s_C, std::string &s_D);
void getRegionRatios(TObjArray* array, std::string sample, std::string s_A, std::string s_B, std::string s_C, std::string s_D);
void printLeakageCoefficients(TH1D* A, TH1D* B, TH1D* C, TH1D* D, std::string sample, bool dumpIntegralAndError = true);
void returnRegionHistograms(TObjArray* array, std::string srORcr, std::string region, TH1D*& data, TH1D*& prompt, TH1D*& ewk, TH1D*& ttbar, TH1D*& Wv221, TH1D*& SingleTop, TH1D*& Dibosons, TH1D*& Z, TH1D*& Xhh);
TH1D* returnABCDestimation(TObjArray* array, std::string analysisRegion, std::string highPtRegion, std::string lowPtRegion, bool drawPlot = false, std::string userBDregion = "", bool printInformation = true);
void makeBetterLeakageTable(TObjArray* array, std::string sample);
void getBetterSampleCoefficients(TObjArray* array, std::string cutLevel, std::string s_A, std::string s_B, std::string s_C, std::string s_D);
std::string returnNonPromptName(TH1D* h0);
void storeNonPromptHistos(TObjArray* array, TH1D* nA, TH1D* nB, TH1D* nC, TH1D* nD);
void dumpQCDhistograms(TFile *& fOutput, TObjArray* array, std::string region);
void dumpQCDhistograms_monica(TFile *& fOutput, TObjArray* array, std::string region);
void makeRTable(TObjArray* array, std::string sample);
void cleanRegions(TH1D*& sr, TH1D*& cr);
void runSystematic(TFile* fInput, TObjArray* a_hists, std::string abcdVariation, bool printSystPlots = false);
void runBDstudy(TObjArray* array, std::string BDregion);
void iterateTTbarNF(TObjArray* array, int iteration, double& n_QCD_first, double& n_QCD_nf, double& n_QCD_sr, double& ttbarNF);

void abcdPlotter_new(std::string s_optStrategy = "", std::string s_xhhmass = "", std::string s_lepType = "", std::string s_lepChannel = "", std::string s_varABCD = "")
{
  // set top directory manually. subdirectories created automatically
  topDir = "aug14_systYields/";

  // some global booleans. These should probably be set in some config file in the future
  normPlots           = false;
  scaleTTbar          = false;
  onlyTTbar           = false;
  addSR               = false;
  qcdMM               = false;
  verbose             = false;
  useOldCutflow       = true;
  useVBSMethod        = false;
  useScalingMethod    = true;
  doSignalInjection   = false;
  printPlots          = false;
  dumpRootFile        = true;
  doStabilityTest     = false;
  doBDRegionStudy     = false;
  doTtbarNFvsQCDstudy = false;
  doMbbShapeStudy     = false;
  doSecondRnp         = false;
  addSystematics      = true;
  dumpTexFiles        = true;
  
  // Sanitation of inputs
  setRunOptions(s_optStrategy, s_xhhmass, s_lepType, s_lepChannel, s_varABCD);
  
  // check subdirectory structure for requested options and create directories if necessary
  struct stat sb;
  if (!(stat(topDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
    cout << "TopDir, " << topDir << " , DNE. EXITING" << endl;
    exit(0);
  }
  topDir = (topDir + lepType + "/").c_str();
  if (!(stat(topDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
    cout << "lepType subdirectory , " << topDir << " , DNE. Creating now" << endl;
    mkdir(topDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
  }
  topDir = (topDir + lepChannel + "/").c_str();
  if (!(stat(topDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
    cout << "lepChannel subdirectory , " << topDir << " , DNE. Creating now" << endl;
    mkdir(topDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
  }

  // additional subdirectory level
  if (scaleTTbar)
    topDir  += "ttbarScaled/";
  else if (!onlyTTbar)
    topDir += "mcYields/";
  else
    topDir += "onlyTTbar/";
  if (!(stat(topDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
    cout << "run type subdirectory , " << topDir << " , DNE. Creating now" << endl;
    mkdir(topDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
  }
  
  // add subdirectory for .tex files if necessary
  std::string tempDir = (topDir + "latex/").c_str();
  if (!(stat(tempDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
    cout << "latex subdirectory , " << tempDir << " , DNE. Creating now" << endl;
    mkdir(tempDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
  }
  if(dumpTexFiles) {
    // create latex files
    std::string base = (optStrategy + "_" + xhhmass + "_" + varABCD + ".tex").c_str();

    beginLatexTable(yieldTXT, (tempDir + "yieldTable_" + base).c_str() );
    beginLatexTable(rNonPromptTXT, (tempDir + "rNonPrompt_" + base).c_str() );
    if(doTtbarNFvsQCDstudy)    beginLatexTable(ttbarNFvsQCDstudyTXT, (tempDir + "ttbarNFvsQCD_" + base).c_str());
    if(doStabilityTest)        beginLatexTable(abcdStabilityTXT, (tempDir + "abcdStabilityTest_" + base).c_str());
    if(doMbbShapeStudy)        beginLatexTable(mbbShapeTXT, (tempDir + "mbbShapeStudy_" + base).c_str());
  }

  // add subdirectory for unblinded results if necessary
  tempDir = (topDir + "unblinded/").c_str();
  if (!(stat(tempDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
    cout << "unblinded subdirectory , " << tempDir << " , DNE. Creating now" << endl;
    mkdir(tempDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
  }
  
  // Load ATLAS style
  SetAtlasStyle();
  
  // lep d0sigBL15_20_25_30_50, large d0, el+mu only, IsoOR
  //TFile *fInput = new TFile("jan26_abcd/data_allBkgs_Sherpa221_WandZjets_threeSignal-all_d0sigBLopts-PrivCxAOD-v5-IsoOR-230117.root", "READ");

  TFile* fInput;
  if(lepType == "IsoOR") {
    //fInput = new TFile("jul19_fullSysts/hist-all-7-18.root","READ");
    //fInput = new TFile("jul22_mwwX/hist-nominal-7-20.root","READ");
    //fInput = new TFile("jul16_mbbHistos/hist-reOpt-nominal-mbbStudy.root","READ");
    fInput = new TFile("aug14_systYields/hist-allselection-fullsyst-bbMass_v2.root", "READ");
    //cout << "****** gfsdgasdfasd asfékljs :)) ;;))= " << endl;
  }
  else if(lepType== "TightMM"){
    fInput = new TFile("apr18_abcd/hist-36ifb_TightMM_unblindedBCD_METriggerCommented.root", "READ");
  }
  else {
    cout << "Oops, you're using TightMM and don't have any files set. EXITING ----------->" << endl;
    return;
  }
  
  TCanvas* c1 = new TCanvas("c1","c1", 800, 800);
  
  //TObjArrays
  TObjArray* a_hists =new TObjArray();
  TObjArray* a_hists_nom =new TObjArray();
  
  // *** 2. Store histograms in arrays
  loopHists(fInput, a_hists);
  a_hists_nom = (TObjArray*)a_hists->Clone();
    
  // *** 3. Produce control plots, calculate ABCD contribution, draw blinded control plot with QCD added, draw unblinded control plot (one-stop shop)
  // ** i. Enormous kluge for using reOpt500 Rnp value for reOptNonRes and reOpt700
  if( doSecondRnp && (optStrategy == "reOptNonRes" || optStrategy == "reOpt700"))
    runABCD(c1, a_hists_nom, "reOpt500", "mww_bbpt210_wwpt150", varABCD);
    //runABCD(c1, a_hists_nom, "reOpt500", "mww_wwpt150", varABCD);   

  std::string s_cutLevel = "";
  for (int c = 0; c < cutflow.size(); c++) {
    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    // ** B. Produce ABCD estimation
    runABCD(c1, a_hists_nom, optStrategy, s_cutLevel, varABCD);   
    if(c!= (cutflow.size() -1)) {
      if(c!=0) runABCD(c1, a_hists_nom, optStrategy, cutflow.at(c), varABCD);         // individual cuts
      runABCD(c1, a_hists_nom, optStrategy, (s_cutLevel + "_mbb").c_str(), varABCD);  // cutflow + mbb
    }
  }
  
  // *** 4. Making yield tables
  makeYieldTable(a_hists_nom, ("mBBcr_" + optStrategy).c_str());
  makeYieldTable(a_hists_nom, ("SR_" + optStrategy).c_str());
  //makeYieldTable(a_hists, (optStrategy).c_str());

  // *** 5. Print tables of leakage coefficients for multiple X resonance masses
  //makeBetterLeakageTable(a_hists, "ttbar");
  //makeBetterLeakageTable(a_hists, "nonprompt");

  // *** 6. Dump tables of R_MC for desired samples
  makeRTable(a_hists_nom, "nonprompt");
  makeRTable(a_hists_nom, "ttbar");
  makeRTable(a_hists_nom, "data");
  //makeRatioTable(a_hists_nom, "nonprompt");

  // *** 7. Study relationship between ttbar NF and QCD estimate --> iterative procedure
  if (doTtbarNFvsQCDstudy) {

    double n_QCD_first, n_QCD_nf, n_QCD_sr, ttbarNF = 0;
    for (int i = 0; i < 8; i ++)
      iterateTTbarNF(a_hists, i, n_QCD_first, n_QCD_nf, n_QCD_sr, ttbarNF);
  }
  
  // *** 8. Do B,D region study
  if (doBDRegionStudy) {

    if(optStrategy == "optNoRes"){
      TObjArray* a_hists_drww11 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_drbb12 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_bbpt230 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_bbpt150 = (TObjArray*)a_hists->Clone();
      runBDstudy(a_hists_drww11, "drww11");
      runBDstudy(a_hists_drbb12, "drbb12");
      runBDstudy(a_hists_bbpt230, "bbpt230");
      runBDstudy(a_hists_bbpt150, "bbpt150");
    }
    else if(optStrategy == "opt700ichep"){
      TObjArray* a_hists_mhh = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_drww09 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_drbb11 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_bbpt150 = (TObjArray*)a_hists->Clone();
      runBDstudy(a_hists_mhh, "mhh");
      runBDstudy(a_hists_drww09, "drww09");
      runBDstudy(a_hists_drbb11, "drbb11");
      runBDstudy(a_hists_bbpt150, "bbpt150");
    }
    else if(optStrategy == "opt2000ichep"){
      TObjArray* a_hists_mhh = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_drww20 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_wwpt360 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_bbpt350 = (TObjArray*)a_hists->Clone();
      runBDstudy(a_hists_mhh, "hh2000above");
      runBDstudy(a_hists_drww20, "drww20");
      runBDstudy(a_hists_wwpt360, "wwpt360");
      runBDstudy(a_hists_bbpt350, "bbpt350");
    }
    else if(optStrategy == "reOptNonRes"){
      TObjArray* a_hists_wwpt250 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_bbpt300 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_bbpt210 = (TObjArray*)a_hists->Clone();
      runBDstudy(a_hists_wwpt250, "wwpt250");
      runBDstudy(a_hists_bbpt300, "bbpt300");
      runBDstudy(a_hists_bbpt210, "bbpt210");
    }
    else if(optStrategy == "reOpt700"){
      TObjArray* a_hists_wwpt250 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_hh700 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_bbpt210 = (TObjArray*)a_hists->Clone();
      runBDstudy(a_hists_hh700, "hh700");
      runBDstudy(a_hists_wwpt250, "wwpt250");
      runBDstudy(a_hists_bbpt210, "bbpt210");
    }
    else if(optStrategy == "reOpt2000"){
      TObjArray* a_hists_wwpt250 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_hh2000 = (TObjArray*)a_hists->Clone();
      TObjArray* a_hists_drww15 = (TObjArray*)a_hists->Clone();
      runBDstudy(a_hists_hh2000, "hh2000");
      runBDstudy(a_hists_drww15, "drww15");
      runBDstudy(a_hists_wwpt250, "wwpt250");
    }

  }

  // *** 9. Do Stability Test
  if (doStabilityTest) {

    TObjArray* a_hists_met25d022 = new TObjArray();
    TObjArray* a_hists_met225d020 = new TObjArray();
    // ** A. Do d0sig > 2.0 --> d0sig > 2.2
    runSystematic(fInput, a_hists_met25d022, "met25d022");
    // ** B. Do MET < 25 GeV --> MET < 15
    //runSystematic(fInput, a_hists_met225d020, "met225d020");
    runSystematic(fInput, a_hists_met225d020, "met15d020");
    // ** C. Print table comparing QCD yields for nominal ABCD and variations
    compareABCDvariations(a_hists_nom, a_hists_met225d020, a_hists_met25d022, ("mBBcr_" + optStrategy).c_str() );
    compareABCDvariations(a_hists_nom, a_hists_met225d020, a_hists_met25d022, ("SR_" + optStrategy).c_str() );
  }

  // *** 10. Study shape of mbb spectrum as function of individual cuts
  if (doMbbShapeStudy) {
    cout<< std::fixed << std::setprecision(2);
    mbbShapeTXT << std::fixed << std::setprecision(2);

    cout<<" ========== mBB Shape Table for "<<optStrategy<<" ========== "<<endl;
    int nCuts = cutflow.size();

    if(dumpTexFiles) {
      mbbShapeTXT << "\\begin{tabular}{c";
      for(int c = 1; c < nCuts-1; c++)
	mbbShapeTXT << "|c";
      mbbShapeTXT << "}" << endl;
      mbbShapeTXT << "\\hline\\hline" << endl;
      mbbShapeTXT << "\\multicolumn{" << nCuts-1 << "}{c}{" << optStrategy << ": \\mbb SR/CR Ratios For Individual Cuts}\\\\\\hline\\hline " << endl;
    }
    
    cout << cutflow.at(0) ;
    mbbShapeTXT << cutflow.at(0) ;
    for (int c = 1; c < nCuts -1; c++) {
      cout << " \t& " << cutflow.at(c) ;
      mbbShapeTXT << " \t& " << cutflow.at(c) ;
    }
    mbbShapeTXT << " \t\\\\\\hline " << endl;
    TH1D *hSR = new TH1D();
    TH1D *hCR = new TH1D();
    std::string s_SR = "";
    std::string s_CR = "";
    double iSR, iCR, eSR, eCR, ratio, ratio_err = 0;
    for (int c = 0; c < nCuts -1; c++) {
      // ** A. Read histograms
      s_SR = ("QCD_SR_" + optStrategy + "_" + cutflow.at(c) + "_" + varABCD + "_regionA_" + abcdSystematic).c_str();
      s_CR = ("QCD_mBBcr_" + optStrategy + "_" + cutflow.at(c) + "_" + varABCD + "_regionA_" + abcdSystematic).c_str();
      
      hSR = returnArrayHistoLocal(a_hists_nom, s_SR );
      hCR = returnArrayHistoLocal(a_hists_nom, s_CR );
      iSR = hSR->IntegralAndError(1, hSR->GetNbinsX()+1, eSR);
      iCR = hCR->IntegralAndError(1, hCR->GetNbinsX()+1, eCR);

      // with SR in denominator
      ratio = iSR / (iCR + iSR);
      ratio_err = ratio * sqrt( 2*(eSR/iSR)*(eSR/iSR) + (eCR/iCR)*(eCR/iCR));
      // without SR in denominator
      //ratio = iSR / iCR;
      //ratio_err = ratio * sqrt( (eSR/iSR)*(eSR/iSR) + (eCR/iCR)*(eCR/iCR));

      if(c!=0){
	cout << " \t& ";
	mbbShapeTXT << " \t& ";
      }
      cout << ratio << " $\\pm$ " << ratio_err;
      mbbShapeTXT << ratio << " $\\pm$ " << ratio_err;
            
      //cout << "doMbbShapeStudy, " << cutflow.at(c) << ", SR name: " << s_SR << " ,  CR name: " << s_CR << endl;
      //cout << "doMbbShapeStudy, " << cutflow.at(c) << ", SR yield: " << iSR << " +/- " << eSR << ",  CR yield: " << iCR <<  " +/- " << eCR << endl;
    }
    cout << "\n";
    mbbShapeTXT << " \t\\\\\\hline " << endl;
   }
  
  // *** 11. Dump output .root file if requested (with or without systematics depending on addSystematics)
  if (dumpRootFile) {
    TFile* fOutput = new TFile((topDir + "qcdHistograms_" + optStrategy + "_" + xhhmass + "_" + varABCD + ".root").c_str(),"RECREATE");
    dumpQCDhistograms_monica(fOutput, a_hists_nom, ("mBBcr_" + optStrategy).c_str());
    dumpQCDhistograms_monica(fOutput, a_hists_nom, ("SR_" + optStrategy).c_str());
    dumpQCDhistograms_monica(fOutput, a_hists_nom, optStrategy);
    
    // *** 11.5 Add systematic histograms (re-binned for consistency and plotting
    if(addSystematics) {
      cout << "systeematics!" << endl;
      loopSystematics(fInput, fOutput, a_hists_nom);
    } 
  }

  // *** 12. Dump .tex tables if if requested
  if(dumpTexFiles) {
    endLatexTable(yieldTXT);
    endLatexTable(rNonPromptTXT);
    if(doTtbarNFvsQCDstudy)  endLatexTable(ttbarNFvsQCDstudyTXT);
    if(doStabilityTest)      endLatexTable(abcdStabilityTXT);
    if(doMbbShapeStudy)      endLatexTable(mbbShapeTXT);
  }
  
  return;
}

void iterateTTbarNF(TObjArray* array, int iteration, double& n_QCD_first, double& n_QCD_nf, double& n_QCD_sr, double& ttbarNF)
{
  if(iteration == 0 ) {
    cout << " ==================================================================== " << endl;
    cout << " ===========    Iterating ttbar NF and QCD Estimation    ============ " << endl;
    cout << " ==================================================================== " << endl;
    cout << "Iteration & \t nQCD_first & \t n_QCD_nf & \t nQCD_sr & \t ttbar_first & \t  ttbar_sr & \t  ttbarNF \t\\\\\\hline " <<endl;

    if(dumpTexFiles){
      ttbarNFvsQCDstudyTXT <<  "\\begin{tabular}{l|c|c|c|c|c|c}";
      ttbarNFvsQCDstudyTXT << "\\hline\\hline" << endl;
      ttbarNFvsQCDstudyTXT << "Iteration & \t nQCD\\_first & \t n\\_QCD\\_nf & \t nQCD\\_sr & \t ttbar\\_first & \t  ttbar\\_sr & \t  ttbarNF \t\\\\\\hline " <<endl;
    }

  }
  
  cout<< std::fixed << std::setprecision(2);
  ttbarNFvsQCDstudyTXT << std::fixed << std::setprecision(2);

  TCanvas* c0 = new TCanvas("c0", "c0", 800, 800);

  TObjArray* array_it = (TObjArray*)array->Clone();

  // ** A. Reset histogram array and scale all ttbar histograms (if not first iteration)
  array_it = (TObjArray*)array->Clone();

  if (iteration!=0) {
    TIter next(array_it);
    TObject *object;
    TH1D* h0= new TH1D();
    
    //loop over array
    while( (object=next()) ) { 
      h0=(TH1D*)object;
      std::string hTitle = h0->GetTitle();
      //cout << "h0->GetTitle(): " << hTitle << endl;
      if (hTitle.find("ttbar_")!=string::npos)
	h0->Scale(ttbarNF);
    }
  }
  
  // *** B. Get region strings for cut level of QCD estimation, ttbar NF calculation, and final mbb SR
  int nCuts = cutflow.size();
  std::string s_cutLevel, firstCut, ttbarNFCut, mbbCut = "";
  for (int c = 0; c < nCuts; c++) {
    // ** i. Create cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    // ** ii. Produce QCD estimate
    runABCD(c0, array_it, optStrategy, s_cutLevel, varABCD, false, "", false);
    
    // ** iii. Store cut string when appropriate
    if (c==0)                                                       firstCut   = s_cutLevel;
    if (optStrategy == "optNoRes" && cutflow.at(c) == "bbpt150")    ttbarNFCut = s_cutLevel;
    if (optStrategy == "opt700ichep" && cutflow.at(c) == "bbpt150") ttbarNFCut = s_cutLevel;
    if (optStrategy == "opt700" && cutflow.at(c) == "bbpt150") ttbarNFCut = s_cutLevel;
    if (optStrategy == "opt2000ichep" && cutflow.at(c) == "drww20") ttbarNFCut = s_cutLevel;
    if (optStrategy == "opt2000" && cutflow.at(c) == "drbb10") ttbarNFCut = s_cutLevel;
    if (optStrategy == "reOptNonRes" && cutflow.at(c) == "bbpt300" ) ttbarNFCut = s_cutLevel;
    if (optStrategy == "reOpt500" && cutflow.at(c) == "wwpt150" ) ttbarNFCut = s_cutLevel;
    if (optStrategy == "reOpt700" && cutflow.at(c) == "wwpt250" ) ttbarNFCut = s_cutLevel;
    if (optStrategy == "reOpt2000" && cutflow.at(c) == "drww15" ) ttbarNFCut = s_cutLevel;
    if (cutflow.at(c) == "mbb")                                     mbbCut     = s_cutLevel;
  }

  // *** C. Read histograms of first cut (QCD estimation), cut for ttbar NF calculation, and final SR yields
  //cout << ("QCD_mBBcr_" + optStrategy + "_"+ firstCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() << endl;
  TH1D* qcd_first    = iteration > 0 ? returnArrayHisto(array_it,  ("QCD_mBBcr_" + optStrategy + "_" + firstCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() ) : new TH1D();
  TH1D* qcd_nf       = iteration > 0 ? returnArrayHisto(array_it,  ("QCD_mBBcr_" + optStrategy + "_" + ttbarNFCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() ) : new TH1D();
  TH1D* qcd_sr       = iteration > 0 ? returnArrayHisto(array_it,  ("QCD_SR_" + optStrategy + "_" + mbbCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() ) : new TH1D();
  
  TH1D* ttbar_first  = returnArrayHisto(array_it,  ("ttbar_mBBcr_" + optStrategy + "_" + firstCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
  TH1D* ttbar_nf     = returnArrayHisto(array_it,  ("ttbar_mBBcr_" + optStrategy + "_" + ttbarNFCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
  TH1D* ttbar_sr     = returnArrayHisto(array_it,  ("ttbar_SR_" + optStrategy + "_" + mbbCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );

  TH1D* Wv221_nf     = returnArrayHisto(array_it,  ("Wv221_mBBcr_" + optStrategy + "_" + ttbarNFCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
  std::string zVersion = "Zv221";
  if(lepType == "TightMM")
    zVersion = "Z";
  TH1D* Z_nf         = returnArrayHisto(array_it,  (zVersion + "_mBBcr_" + optStrategy + "_" + ttbarNFCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
  std::string dibosonVersion = "Dibosonsv221";
  if(lepType == "TightMM")
    dibosonVersion = "Dibosons";
  TH1D* dibosons_nf  = returnArrayHisto(array_it,  (dibosonVersion + "_mBBcr_" + optStrategy + "_" + ttbarNFCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
  TH1D* singleTop_nf = returnArrayHisto(array_it,  ("SingleTop_mBBcr_" + optStrategy + "_" + ttbarNFCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
  TH1D* data_nf      = returnArrayHisto(array_it,  ("data_mBBcr_" + optStrategy + "_" + ttbarNFCut + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
  TH1D* totBkg_nf    = (TH1D*)ttbar_nf->Clone();

  if(iteration > 0) totBkg_nf->Add(qcd_nf);
  totBkg_nf->Add(Wv221_nf);
  totBkg_nf->Add(Z_nf);
  totBkg_nf->Add(dibosons_nf);
  totBkg_nf->Add(singleTop_nf);

  /*cout << " QCD (mBBcr) at first cut: \t\t" << qcd_first->Integral() << endl;
  cout << " QCD (mBBcr) at ttbarNF cut: \t\t" << qcd_nf->Integral() << endl;
  cout << " QCD (SR) at mbb cut: \t\t\t" << qcd_sr->Integral() << endl;
  cout << " ttbar (mBBcr) at first cut: \t\t" << ttbar_first->Integral() << endl;
  cout << " ttbar (mBBcr) at ttbarNF cut: \t\t" << ttbar_nf->Integral() << endl;
  cout << " ttbar (SR) at mbb cut: \t\t" << ttbar_sr->Integral() << endl;
  cout << " Total Bkg (mBBcr) at ttbarNF cut: \t" << totBkg_nf->Integral() << endl;
  cout << " Data (mBBcr) at ttbarNF cut: \t\t" << data_nf->Integral() << endl;*/

  // ** D. Store and print some information
  double n_ttbar_first = ttbar_first->Integral();
  double n_ttbar_sr    = ttbar_sr->Integral();
  n_QCD_first = qcd_first->Integral();
  n_QCD_nf    = qcd_nf->Integral();
  n_QCD_sr    = qcd_sr->Integral();
  ttbarNF     = 1 + (data_nf->Integral() - totBkg_nf->Integral())/ttbar_nf->Integral();
  double ttbarNF_err   = (totBkg_nf->Integral()/ttbar_nf->Integral()) * sqrt( 1/totBkg_nf->GetEntries() + 1/ttbar_nf->GetEntries());
  //cout << "Iteration " << iteration << ", nQCD_first = " << n_QCD_first << ", nQCD_nf = " << n_QCD_nf << ", nQCD_sr = " << n_QCD_sr << ", ttbar_first = " << n_ttbar_first << ", ttbar_sr = " << n_ttbar_sr << ", ttbarNF = " << ttbarNF << endl;

  cout << iteration << " \t & \t " << n_QCD_first << " & \t "  << n_QCD_nf << " & \t "  << n_QCD_sr << " & \t "  << n_ttbar_first << " & \t "  << n_ttbar_sr << " & \t "  << ttbarNF << " $\\pm$ " << ttbarNF_err << " \t\\\\\\hline " << endl;
  if(dumpTexFiles)
    ttbarNFvsQCDstudyTXT << iteration << " \t & \t " << n_QCD_first << " & \t "  << n_QCD_nf << " & \t "  << n_QCD_sr << " & \t "  << n_ttbar_first << " & \t "  << n_ttbar_sr << " & \t "  << ttbarNF << " $\\pm$ " << ttbarNF_err << " \t\\\\\\hline " << endl;

  // ** E. Repeat X times or until ttbarNF_(n-1) - ttbarNF_(n) < threshold
  
  delete c0;
  return;
}

void runBDstudy(TObjArray* array, std::string BDregion)
{
  TCanvas* c0 = new TCanvas("c0", "c0", 800, 800);
  
  cout << " ========================================================== " << endl;
  cout << " ===========    Evaluating B,D Study Region    ============ " << endl;
  cout << " ===========          " << optStrategy << ", " << BDregion << "         ============" << endl;
  cout << " ========================================================== " << endl;
  
  std::string s_cutLevel = "";
  for (int c = 0; c < cutflow.size(); c++) {
    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    // ** B. Produce ABCD estimation
    runABCD(c0, array, optStrategy, s_cutLevel, varABCD, false, BDregion);   
  }
  
  printBDregionYields(array, BDregion);

  delete c0;

  return;
}

void runSystematic(TFile* fInput, TObjArray* a_hists, std::string abcdVariation, bool printSystPlots)
{
  TCanvas* c0 = new TCanvas("c0","c0", 800, 800);

  // *** 0. Store nominal topDir and abcdSystematic
  std::string storeABCDsystematic = abcdSystematic;
  std::string storeTopDir = topDir;
  bool storeDumpTexFiles = dumpTexFiles;
  
  // *** 1. Set topDir and abcdSystematic for variation
  cout << " ========================================================== " << endl;
  cout << " ===========       Evaluating Systematics      ============ " << endl;
  cout << " ===========              " << abcdVariation << "            ============" << endl;
  cout << " ========================================================== " << endl;
  
  abcdSystematic = abcdVariation;
  topDir = (topDir + abcdSystematic + "/" ).c_str();
  dumpTexFiles = false;
  struct stat sb;
  if (!(stat(topDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
    cout << "systematic subdirectory , " << topDir << " , DNE. Creating now" << endl;
    mkdir(topDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
  }
  std::string tempDir = (topDir + "unblinded/").c_str();
  if (!(stat(tempDir.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode))){
    cout << "unblinded subdirectory , " << tempDir << " , DNE. Creating now" << endl;
    mkdir(tempDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH);
  }


  // *** 2. Store histograms in arrays
  loopHists(fInput, a_hists);   

  // *** 3. Calculate ABCD contribution
  std::string s_cutLevel = "";
  for (int c = 0; c < cutflow.size(); c++) {
    // ** A) Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  
    // ** B) Produce ABCD estimation
    runABCD(c0, a_hists, optStrategy, s_cutLevel, varABCD, printSystPlots);   
  }

  // *** 4. Produce yield tables
  makeYieldTable(a_hists, ("mBBcr_" + optStrategy).c_str());
  makeYieldTable(a_hists, ("SR_" + optStrategy).c_str());
  makeRTable(a_hists, "nonprompt");

  // *** 5. Reset topDir and abcdSystematic to nominal values
  topDir = storeTopDir;
  abcdSystematic = storeABCDsystematic;
  dumpTexFiles = storeDumpTexFiles;
  
  delete c0;
  
  return;
}
   
void runABCD(TCanvas *&c0, TObjArray *array, std::string region, std::string cuts, std::string variable, bool drawControlPlots, std::string userBDregion, bool printInformation)
{
  // *** 1. Draw plots with ratio and stuff
  if(drawControlPlots) makeControlPlots(array, c0, ("mBBcr_" + region).c_str(), cuts, variable, normPlots);
  if(drawControlPlots) makeControlPlots(array, c0, region, cuts, variable, normPlots); // ABCD region plots using full mbb spectrum?
  
  // *** 2. Do ABCD calculations
  if(verbose) 
    cout<<"runABCD, pre string manip,  cuts: "<<cuts<<endl;

  if(!qcdMM) // calculate QCD contribution using ABCD method if not obtaining QCD from MM
    //calculateABCD(c0, array, region, (cuts + "_" + variable).c_str(), (cuts + "_" + variable).c_str(), (cuts + "_" + variable).c_str(), true);
    calculateABCD(c0, array, region, (cuts + "_" + variable).c_str(), (cuts + "_" + variable).c_str(), (cuts + "_" + variable).c_str(), drawControlPlots, userBDregion, printInformation);
  
  // *** 3. Draw blinded plot with QCD added
  if(drawControlPlots) makeControlPlots(array, c0, region, cuts, variable, normPlots, true);
  if(drawControlPlots) makeControlPlots(array, c0, ("mBBcr_" + region).c_str(), cuts, variable, normPlots, true);
  
}


void returnRegionStrings(std::string opt, std::string phase, std::string& s_A, std::string& s_B, std::string& s_C, std::string &s_D)
{
  std::string regionA = (opt + "_" + phase + "_" + varABCD + "_regionA_" + abcdSystematic).c_str();
  std::string regionB = (opt + "_" + phase + "_" + varABCD + "_regionB_" + abcdSystematic).c_str();
  std::string regionC = (opt + "_" + phase + "_" + varABCD + "_regionC_" + abcdSystematic).c_str();
  std::string regionD = (opt + "_" + phase + "_" + varABCD + "_regionD_" + abcdSystematic).c_str();

  //cout << "POST: regionA: " << regionA << "\tregionB: " << regionB << "\tregionC: " << regionC << "\tregionD: " << regionD << endl;

  s_A = regionA;
  s_B = regionB;
  s_C = regionC;
  s_D = regionD;
}

void printRatioLine(TObjArray* array, std::string sample, std::string cutLevel)
{
  std::string regionA, regionB, regionC, regionD;

  returnRegionStrings(optStrategy, cutLevel, regionA, regionB, regionC, regionD);
  cout<< cutLevel << " \t\t\t\t& ";
  getRegionRatios(array, sample, regionA, regionB, regionC, regionD);

  return;
}

void makeRatioTable(TObjArray* array, std::string sample)
{
  std::string regionA, regionB, regionC, regionD;
 
  cout<< " ============== "<< optStrategy << " ============== " <<endl;
  cout<< sample <<" \t\t\t\t\t& A/C ratio \t\t& B/D ratio \t\t\\\\\\hline"<<endl;

  std::string s_cutLevel = "";
  for (int c = 0; c < cutflow.size(); c++) {
    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    printRatioLine(array, sample, s_cutLevel);
  }

  return;
}

void getRegionRatios(TObjArray* array, std::string sample, std::string s_A, std::string s_B, std::string s_C, std::string s_D)
{
  TH1D* h_A = returnArrayHisto(array, (sample + "_" + s_A).c_str() );
  TH1D* h_B = returnArrayHisto(array, (sample + "_" + s_B).c_str() );
  TH1D* h_C = returnArrayHisto(array, (sample + "_" + s_C).c_str() );
  TH1D* h_D = returnArrayHisto(array, (sample + "_" + s_D).c_str() );

  double intA = h_A->Integral();
  double intB = h_B->Integral();
  double intC = h_C->Integral();
  double intD = h_D->Integral();

  double errAC = sqrt( 1/h_A->GetEntries() + 1/h_C->GetEntries()) * (intA / intC);
  //double errBD = sqrt( 1/h_B->GetEntries() + 1/h_D->GetEntries()) * (intB / intD);
  double errBD = sqrt( 1/h_B->GetEntries() + 1/h_D->GetEntries()) * (intD / intB);

  cout<< std::fixed << std::setprecision(2);

  //cout << intA / intC << " \\pm " << errAC << " \t& " << intB / intD << " \\pm " << errBD  <<" \t\\\\\\hline"<<endl;

  cout << intA / intC << " \\pm " << errAC << " \t& " << intD / intB << " \\pm " << errBD  <<" \t\\\\\\hline"<<endl;
}

void makeBetterLeakageTable(TObjArray* array, std::string sample)
{
  std::string regionA, regionB, regionC, regionD;
  cout<< " ============== "<< sample << " , " << optStrategy << " ============== " <<endl;
  cout<<"Cut \t& B/A ratio \t& C/A ratio \t& D/A ratio \t\\\\\\hline"<<endl;

  std::string s_cutLevel = "";
  for (int c = 0; c < cutflow.size(); c++) {
    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    returnRegionStrings(optStrategy, s_cutLevel, regionA, regionB, regionC, regionD);
    getBetterSampleCoefficients(array, s_cutLevel, (sample + "_" + regionA).c_str(), (sample + "_" + regionB).c_str(), (sample + "_" + regionC).c_str(), (sample + "_" + regionD).c_str()); 
  }

  return;
}

void getBetterSampleCoefficients(TObjArray* array, std::string cutLevel, std::string s_A, std::string s_B, std::string s_C, std::string s_D)
{
  TH1D* h_A = returnArrayHisto(array, s_A);
  TH1D* h_B = returnArrayHisto(array, s_B);
  TH1D* h_C = returnArrayHisto(array, s_C);
  TH1D* h_D = returnArrayHisto(array, s_D);

  printLeakageCoefficients(h_A, h_B, h_C, h_D, cutLevel, false);
}

void calculateABCD(TCanvas*& c0, TObjArray* array, std::string analysisRegion, std::string highPtRegion, std::string lowPtRegion, std::string fname, bool drawPlot, std::string userBDregion, bool printInformation)
{
  if (userBDregion == "" && printInformation) cout<<" ============= ABCD Estimation , " << analysisRegion << "_" << highPtRegion << " ============= "<<endl;

  // ***** I. Do ABCD in m_bb sideband window always *****
  TH1D* nA_QCD_CR_temp = returnABCDestimation(array, ("mBBcr_" + analysisRegion).c_str(), highPtRegion, lowPtRegion, drawPlot, userBDregion, printInformation);
  TH1D* nA_QCD         = returnABCDestimation(array, analysisRegion, highPtRegion, lowPtRegion, drawPlot, userBDregion, printInformation);
  TH1D* nA_QCD_SR_temp = returnABCDestimation(array, ("SR_" + analysisRegion).c_str(), highPtRegion, lowPtRegion, drawPlot, userBDregion, printInformation);

  if(useScalingMethod && varABCD=="bbMass"){
    std::string sX = ("nonprompt_" + analysisRegion).c_str();
    std::string trim = "";
    
    //if(optStrategy == "reOptNonRes")       trim = "mww";
    if(optStrategy == "reOptNonRes")       trim = "mww_bbpt210";
    else if(optStrategy == "reOpt500")     trim = "mww_bbpt210";
    else if(optStrategy == "reOpt700")     trim = "mww_bbpt210";
    else if(optStrategy == "reOpt2000")    trim = "bbpt350_wwpt250";
    //else if(optStrategy == "reOpt2000")    trim = "bbpt350";
    
    if (highPtRegion.find(trim) != string::npos) {
      sX = (sX + "_" + trim + "_" + varABCD + "_regionC_" + abcdSystematic).c_str();
      //if(verbose)
      cout << "useScalingMethod, histo string : " << sX << endl;
      
      // ** Get shape from earlier point in cutflow
      TH1D* nA_QCD_shape = returnArrayHisto(array, sX);
      TH1D* nA_QCD_lessMbb = new TH1D();
      if(highPtRegion.find("_mbb")!=string::npos) {
	// * i. Normalization tricks for final mbb cut
	//TH1D* nA_QCD_full  = returnArrayHisto(array, sX);
	cout << "mbb name: " << ("nonprompt_SR_" + optStrategy + "_" + trim + "_mbb_" + varABCD + "_regionC_" + abcdSystematic).c_str() << endl;
	TH1D* nA_QCD_mbb    = returnArrayHisto(array, ("nonprompt_SR_" + optStrategy + "_" + trim + "_mbb_" + varABCD + "_regionC_" + abcdSystematic).c_str());
	TH1D* nA_QCD_trimSR = returnArrayHisto(array, ("nonprompt_SR_" + optStrategy + "_" + trim + "_" + varABCD + "_regionC_" + abcdSystematic).c_str());
	double iFull, iMbb, eFull, eMbb = 0;
	//iFull = nA_QCD_shape->IntegralAndError(1, nA_QCD_shape->GetNbinsX()+1, eFull);
	iFull = nA_QCD_trimSR->IntegralAndError(1, nA_QCD_trimSR->GetNbinsX()+1, eFull);
	iMbb = nA_QCD_mbb->IntegralAndError(1, nA_QCD_mbb->GetNbinsX()+1, eMbb);

	double Cratio = iMbb/iFull;	  
	double Cratio_err = Cratio*sqrt( (eFull/iFull)*(eFull/iFull) + (eMbb/iMbb)*(eMbb/iMbb) );
	//if(verbose)
	cout << nA_QCD_trimSR->GetName() << " has " << iFull << " +/- " << eFull << endl;
	cout << nA_QCD_mbb->GetName()    << " has " << iMbb  << " +/- " << eMbb  << endl;
	cout << "C ratio (2) = " << Cratio << " +/- " << Cratio_err << endl;
	
	// Intelligent plot labeling
	if(verbose)  std::cout << " sX: " << sX << endl;
	std::size_t found  = highPtRegion.find_last_of("mbb");
	std::string frontX = highPtRegion.substr(0,found-5);
	std::string backX  = highPtRegion.substr(found-1);
	std::string lastBack = ("QCD_" + analysisRegion + "_" +  frontX + varABCD + "_regionA_" + abcdSystematic).c_str();
	//if(verbose)
	  std::cout << " highPtRegion: " << highPtRegion << " , frontX: " << frontX << " , backX: " << backX << endl;
	  //if(verbose)
	  std::cout << " lastBack: " << lastBack << endl;
	nA_QCD_lessMbb = returnArrayHisto(array, lastBack);
	//nA_QCD->Scale ( nA_QCD_lessMbb->Integral() * Cratio / nA_QCD->Integral() ); // original
	nA_QCD_shape->Scale ( Cratio*nA_QCD_lessMbb->Integral() / nA_QCD_shape->Integral() ); // trying something?
	double newErr = nA_QCD_shape->GetBinContent(4)*Cratio*sqrt( (nA_QCD_shape->GetBinError(4)/nA_QCD_shape->GetBinContent(4))*(nA_QCD_shape->GetBinError(4)/nA_QCD_shape->GetBinContent(4)) +
								    (Cratio_err/Cratio)*(Cratio_err/Cratio) );
	//nA_QCD_shape->Scale ( Cratio ); // jul 17, 2017: comment out BBT. probably need to revisit
	
	// * ii. Protection for final mbb cut
	for(int z = 1; z < nA_QCD_shape->GetNbinsX() + 1; z++) {
	  if(z != 4) {
	    nA_QCD_shape->SetBinContent(z, 0);
	    nA_QCD_shape->SetBinError(z, 0);
	  }
	  else // add error on Cratio to mbb QCD estimate
	    nA_QCD_shape->SetBinError(z, newErr);
	}

      }
      else{
	cout << "useScalingMethod, " << nA_QCD_shape->GetName() << "->Integral() = " << nA_QCD_shape->Integral() << " , " << nA_QCD->GetName() << "->Integral() = " << nA_QCD->Integral() << endl;
	
	//if(verbose)
	cout << "useScalingMethod, (pre-scale) nA_QCD_shape->Integral(): " << nA_QCD_shape->Integral() << " , nA_QCD->Integral() : " << nA_QCD->Integral()<< endl;
	nA_QCD_shape->Scale( nA_QCD->Integral() / nA_QCD_shape->Integral() );
	//if(verbose)
	cout << "useScalingMethod, (post-scale) nA_QCD_shape->Integral(): " << nA_QCD_shape->Integral() << " , nA_QCD->Integral() : " << nA_QCD->Integral()<< endl;
      }	

      /*// ** iii. Simple check that sets QCD shape bins to zero if data and MC have zero entries there
      TH1D *data_A, *prompt_A, *ewk_A, *ttbar_A, *Wv221_A, *SingleTop_A, *Dibosons_A, *Z_A, *Xhh_A = new TH1D();
      returnRegionHistograms(array, analysisRegion, (highPtRegion + "_regionA_" + abcdSystematic).c_str(), data_A, prompt_A, ewk_A, ttbar_A, Wv221_A, SingleTop_A, Dibosons_A, Z_A, Xhh_A);
      for(int z = 1; z < nA_QCD_shape->GetNbinsX() + 1; z++) {
	if ( data_A->GetBinContent(z)==0 && prompt_A->GetBinContent(z)==0 && ewk_A->GetBinContent(z)==0) {
	  nA_QCD_shape->SetBinContent(z, 0);
	  nA_QCD_shape->SetBinError(z, 0);
	}
      }
      */
      
      // ** iv. Set normalized shape histogram to final QCD histogram
      nA_QCD = (TH1D*)nA_QCD_shape->Clone();

      
    }
  }
  
  TH1D* nA_QCD_CR = new TH1D();
  TH1D* nA_QCD_SR = new TH1D();
  if(varABCD.find("bbMass")!=string::npos){
    nA_QCD_CR = (TH1D*)nA_QCD->Clone();
    nA_QCD_SR = (TH1D*)nA_QCD->Clone();
    cleanRegions(nA_QCD_SR, nA_QCD_CR);
  }
  else {
    nA_QCD_SR = (TH1D*)nA_QCD_SR_temp->Clone();
    nA_QCD_CR = (TH1D*)nA_QCD->Clone();
    if( !std::isnan(nA_QCD_SR->Integral()) ) 
      nA_QCD_CR->Add(nA_QCD_SR, -1);
      
    // ** Take care of bin-by-bin corrections
    for(int i = 1; i < nA_QCD_CR->GetNbinsX() + 1; i++) {
      // i) set bins with negative yield to zero
      if (nA_QCD_CR->GetBinContent(i) < 0)   nA_QCD_CR->SetBinContent(i, 0);
      // ii) don't count SR error in mBBcr plots
      nA_QCD_CR->SetBinError(i, nA_QCD->GetBinError(i));
    }

  }
  //cout << "[SR included]\t CR Yield (no scaling) =\t" << nA_QCD_CR->Integral() <<endl;
  //cout << "[SR included]\t SR Yield (no scaling) =\t" << nA_QCD_SR->Integral() <<endl;
  //cout << "[SR included]\t SR+CR Yield (no scaling) =\t" << nA_QCD->Integral() <<endl;
  
  // *** 3. Add QCD estimate to array
  std::string qcd_arrayname, qcd_sr_arrayname, qcd_mbbcr_arrayname;
  qcd_arrayname = ("QCD_" + optStrategy + "_" + highPtRegion + "_regionA_" + abcdSystematic).c_str();
  qcd_sr_arrayname = ("QCD_SR_" + optStrategy + "_" + highPtRegion + "_regionA_" + abcdSystematic).c_str();
  qcd_mbbcr_arrayname = ("QCD_mBBcr_" + optStrategy + "_" + highPtRegion + "_regionA_" + abcdSystematic).c_str();
  if(verbose){
    cout<<"&&&&&&&&&&&&&&  qcd_arrayname: "<<qcd_arrayname<<endl;
    cout<<"&&&&&&&&&&&&&&  qcd_sr_arrayname: "<<qcd_sr_arrayname<<endl;
    cout<<"&&&&&&&&&&&&&&  qcd_mbbcr_arrayname: "<<qcd_mbbcr_arrayname<<endl;
  }
  nA_QCD_CR->SetName(qcd_mbbcr_arrayname.c_str());
  nA_QCD_CR->SetTitle(qcd_mbbcr_arrayname.c_str());
  nA_QCD_CR->Draw();
  array->AddLast(nA_QCD_CR); // add histo to TObjArray

  nA_QCD_SR->SetName(qcd_sr_arrayname.c_str());
  nA_QCD_SR->SetTitle(qcd_sr_arrayname.c_str());
  nA_QCD_SR->Draw();
  array->AddLast(nA_QCD_SR); // add histo to TObjArray

  nA_QCD->SetName(qcd_arrayname.c_str());
  nA_QCD->SetTitle(qcd_arrayname.c_str());
  nA_QCD->Draw();
  array->AddLast(nA_QCD); // add histo to TObjArray

  return;
}

void cleanRegions(TH1D*& sr, TH1D*& cr)
{
  if(varABCD.find("bbMass")!=string::npos) {

    for(int z = 1; z < sr->GetNbinsX() + 1; z++) {
      if(z != 4) {
	sr->SetBinContent(z, 0);
	sr->SetBinError(z, 0);
      }
      else {
	cr->SetBinContent(z, 0);
	cr->SetBinError(z, 0);
      }
    }
    
  }
}

void getIntegralAndError(double &integral, double &error, TH1D* data, TH1D* ttbar, TH1D* Wv221, TH1D* SingleTop, TH1D* Dibosons, TH1D* Z, bool ttbarOnly)
{
  double intData      = data->Integral();
  double intTTbar     = ttbar->Integral();
  double intWv221      = Wv221->Integral();
  double intSingleTop = SingleTop->Integral();
  double intDibosons  = Dibosons->Integral();
  double intZ         = Z->Integral();

  if(!ttbarOnly)
    integral = intData - intTTbar - intWv221 - intSingleTop - intDibosons - intZ;
  else
    integral = intData - intTTbar;
    
  double errWv221, errTTbar, errSingleTop, errDibosons, errZ, errData = 0;
  /*
    // === This should only be for adding bin-by-bin systematic errors. Stat error comes from GetEffectiveEntries() [below] ===
  for (int i = 0; i < ttbar->GetNbinsX(); i++) {
    if(!onlyTTbar) {
      errWv221      += Wv221->GetBinError(i)*Wv221->GetBinError(i);// + 0.48*Wv221->GetBinContent(i)*0.48*Wv221->GetBinContent(i);
      errZ         += Z->GetBinError(i)*Z->GetBinError(i);//       + 0.48*Z->GetBinContent(i)*0.48*Z->GetBinContent(i);
      errDibosons  += Dibosons->GetBinError(i)*Dibosons->GetBinError(i);//       + 0.48*Dibosons->GetBinContent(i)*0.48*Dibosons->GetBinContent(i);
      errSingleTop += SingleTop->GetBinError(i)*SingleTop->GetBinError(i);//       + 0.05*SingleTop->GetBinContent(i)*0.05*SingleTop->GetBinContent(i);  
    }
    errTTbar     += ttbar->GetBinError(i)*ttbar->GetBinError(i);//       + 0.05*SingleTop->GetBinContent(i)*0.05*SingleTop->GetBinContent(i);  
    errData     += data->GetBinError(i)*data->GetBinError(i);//       + 0.05*SingleTop->GetBinContent(i)*0.05*SingleTop->GetBinContent(i);  
  }
  */

  errData      = intData      / sqrt( data->GetEntries() );
  errTTbar     = intTTbar     / sqrt( ttbar->GetEntries() );
  errWv221      = intWv221      / sqrt( Wv221->GetEntries() );
  errZ         = intZ         / sqrt( Z->GetEntries() );
  errSingleTop = intSingleTop / sqrt( SingleTop->GetEntries() );
  errDibosons  = intDibosons  / sqrt( Dibosons->GetEntries() );
  
  if(!ttbarOnly)  
    error = sqrt( errData*errData + errWv221*errWv221 + errSingleTop*errSingleTop + errZ*errZ + errDibosons*errDibosons + errTTbar*errTTbar);
    //error = sqrt( errWv221 + errSingleTop + errZ + errDibosons + errTTbar);
  else
    error = sqrt( errTTbar*errTTbar + errData*errData);
    //error = sqrt( errTTbar );
  
}


void printLeakageCoefficients(TH1D* A, TH1D* B, TH1D* C, TH1D* D, std::string sample, bool dumpIntegralAndError)
{
  double intA = A->Integral() ;
  double intB = B->Integral() ;
  double intC = C->Integral() ;
  double intD = D->Integral() ;

  double errBA = B->GetEntries() > 0 ? sqrt( 1/A->GetEntries() + 1/B->GetEntries()) * (intB / intA) : 0;
  double errCA = C->GetEntries() > 0 ? sqrt( 1/A->GetEntries() + 1/C->GetEntries()) * (intC / intA) : 0;
  double errDA = D->GetEntries() > 0 ? sqrt( 1/A->GetEntries() + 1/D->GetEntries()) * (intD / intA) : 0;

  cout<<std::fixed<<std::setprecision(3);
  
  if(dumpIntegralAndError)
    cout<<"çççç "<<A->GetName()<<"\t B/A ratio: "<< intB / intA<<"\t C/A ratio: "<< intC / intA<<"\t D/A ratio: "<< intD / intA<<"\t A integral: "<<intA<<"\t B integral: "<<intB<<"\t C integral: "<<intC<<"\t D integral: "<<intD<<endl;
  else
    cout<<sample<<"\t& "<< intB / intA << " $\\pm$ " << errBA <<"\t& "<< intC / intA << " $\\pm$ " << errCA <<"\t& "<< intD / intA << " $\\pm$ " << errDA <<"\t\\\\\\hline "<<endl;
  

}

void returnRegionHistograms(TObjArray* array, std::string srORcr, std::string region, TH1D*& data, TH1D*& prompt, TH1D*& ewk, TH1D*& ttbar, TH1D*& Wv221, TH1D*& SingleTop, TH1D*& Dibosons, TH1D*& Z, TH1D*& Xhh)
{
  if (verbose) cout << "returnRegionHistogram, srORcr: " << srORcr << " , region: " << region << endl;
  ttbar = returnArrayHisto(array,    ("ttbar_" + srORcr + "_" +region).c_str() );
  data = returnArrayHisto(array,     ("data_" + srORcr + "_" +region).c_str() );
  Wv221 = returnArrayHisto(array,     ("Wv221_" + srORcr + "_" +region).c_str() );
  if(lepType == "IsoOR"){
    Dibosons = returnArrayHisto(array, ("Dibosonsv221_" + srORcr + "_" +region).c_str() );
    Z = returnArrayHisto(array,        ("Zv221_" + srORcr + "_" +region).c_str() );
  }
  else if(lepType == "TightMM"){
    Dibosons = returnArrayHisto(array, ("Dibosons_" + srORcr + "_" +region).c_str() );
    Z = returnArrayHisto(array,        ("Z_" + srORcr + "_" +region).c_str() );
  }
  SingleTop = returnArrayHisto(array,("SingleTop_" + srORcr + "_" +region).c_str() );
  Xhh = returnArrayHisto(array,      ("X" + xhhmass + "_" + srORcr + "_" +region).c_str() );

  ewk = (TH1D*)Dibosons->Clone();
  ewk->Add(Wv221);
  ewk->Add(SingleTop);
  ewk->Add(Z);

  prompt = (TH1D*)ttbar->Clone();
    
  return;
}

void trimRegionNames(std::string trim, std::string& sB, std::string& sD )
{
  //cout << " ---> Trimming B,D region names for " << optStrategy << " at " << trim << endl;
  //if(optStrategy=="opt700ichep" && (lepChannel == "el" || lepChannel == "mu") ){
  if(sD=="Do_Ceff_trim"){//stuff we want
   	if(sB.find( trim.c_str() )!=string::npos) {
            size_t f = sB.find( trim.c_str() );
            //sB.replace(f, std::string( trim.c_str() ).length(), "");
            if(verbose) cout << " ½½½½½ sC (pre .replace) : " << sB << endl;
            sB.replace(f, std::string( sB.c_str() ).length()-1, "");
	    std::string sTemp = sB;
            if(verbose) cout << " ½½½½½ sC (post .replace) : " << sB << endl;
            sB = ( trim + "_mbb_" + varABCD + "_regionC_" + abcdSystematic).c_str();
            if(verbose) cout << " ½½½½½ sC (final name) : " << sB << endl;
            sD = ( trim + "_" + varABCD + "_regionC_" + abcdSystematic).c_str();
            if(verbose) cout << " ½½½½½ sC no (post .replace) : " << sD << endl;
            if(verbose) cout << " ½½½½½ sC no (final name) : " << sD << endl;
        }
	
  }
  else{
	  if(sB.find( trim.c_str() )!=string::npos) {
	    size_t f = sB.find( trim.c_str() );
	    //sB.replace(f, std::string( trim.c_str() ).length(), "");
	    if(verbose) cout << " ½½½½½ sB (pre .replace) : " << sB << endl; 
            string region="";
	    if(sB.find("_regionB_")!=string::npos){ 
		region = "_regionB_";
	        sB.replace(f, std::string( sB.c_str() ).length()-1, "");
	        if(verbose) cout << " ½½½½½ sB (post .replace) : " << sB << endl; 
	        sB = (sB + trim + "_" + varABCD + region + abcdSystematic).c_str();
	        if(verbose) cout << " ½½½½½ sB (final name) : " << sB << endl; 
            }

            else if(sB.find("_regionC_")!=string::npos){ 
	        region = "_regionC_";
	    	sB.replace(f, std::string( sB.c_str() ).length()-1, "");
	    	if(verbose) cout << " ½½½½½ sB (post .replace) : " << sB << endl; 
	    	sB = (sB +  varABCD + region + abcdSystematic).c_str();
	    	if(verbose) cout << " ½½½½½ sB (final name) : " << sB << endl; 
		}
	    //sB.replace(f, std::string( sB.c_str() ).length()-1, "");
	    //if(verbose) cout << " ½½½½½ sB (post .replace) : " << sB << endl; 
	    //sB = (sB + trim + "_" + varABCD + region + abcdSystematic).c_str();
	    //if(verbose) cout << " ½½½½½ sB (final name) : " << sB << endl; 
	  }

	  if(sD.find( trim.c_str() )!=string::npos) {
	    size_t f = sD.find( trim.c_str() );
	    //sD.replace(f, std::string( trim.c_str() ).length(), "");
	    sD.replace(f, std::string( sD.c_str() ).length()-1, "");
	    sD = (sD + trim + "_" + varABCD + "_regionD_" + abcdSystematic).c_str();
	  }
  }

  return;
}

void returnTrimmedString(std::string trim, std::string& sX, std::string region, bool mBBratio)
{
  if(mBBratio){
    if(sX.find("regionB")!=string::npos){

    }
    if (verbose) cout << "return trimed strgin (mBB) : " << sX << " , " << trim << " , " << varABCD << " , " << region << " , " << abcdSystematic << endl;
    sX = (trim + "_" + varABCD + "_" + region + "_" + abcdSystematic).c_str();

    return;
  }
  
  if(sX.find( trim.c_str() )!=string::npos) {
    size_t f = sX.find( trim.c_str() );

    if(verbose)
      cout << " ½½½½½ sX (pre .replace) : " << sX << endl; 
    string s_temp = sX;
    sX.replace(f, std::string( sX.c_str() ).length()-1, "");
    if(sX == "0"){ // protection for mbb shape study
      sX = s_temp;
      return;
    }
    if(verbose)
      cout << " ½½½½½ sX (post .replace) : " << sX << endl;
    sX = (sX + trim + "_" + varABCD + "_" + region + "_" + abcdSystematic).c_str();
    
    if(verbose)
      cout << " ½½½½½ sX (final name) : " << sX << endl; 
  }

}

void trimRegionNames2(std::string trim, std::string& sB, std::string& sC, std::string& sD, bool mBBratio = false)
{
  //cout << " ---> Trimming B, C, D region names for " << optStrategy << " at " << trim << endl;
  //if(optStrategy=="opt700ichep" && (lepChannel == "el" || lepChannel == "mu") ){

  returnTrimmedString(trim, sB, "regionB", mBBratio);
  returnTrimmedString(trim, sC, "regionC", mBBratio);
  returnTrimmedString(trim, sD, "regionD", mBBratio);

  return;
}

void mergeSRandCR(TH1D*& spectrum_A, TH1D* hSR_A, TH1D* hCR_A, TH1D*& spectrum_B, TH1D* hSR_B, TH1D* hCR_B, TH1D*& spectrum_C, TH1D* hSR_C, TH1D* hCR_C, TH1D*& spectrum_D, TH1D* hSR_D, TH1D* hCR_D)
{
  spectrum_A = (TH1D*)hSR_A->Clone();
  spectrum_A->Add(hCR_A);
  spectrum_B = (TH1D*)hSR_B->Clone();
  spectrum_B->Add(hCR_B);
  spectrum_C = (TH1D*)hSR_C->Clone();
  spectrum_C->Add(hCR_C);
  spectrum_D = (TH1D*)hSR_D->Clone();
  spectrum_D->Add(hCR_D);
}

TH1D* returnABCDestimation(TObjArray* array, std::string analysisRegion, std::string highPtRegion, std::string lowPtRegion, bool drawPlot, std::string userBDregion, bool printInformation)
{
  // Approach A: Really simple estimation
  // Approach B: maths from VBS Z+jet estimation 

  double nA_QCD_calc = 0;  // normalization of QCD in A region
  
  // *** 1. Create strings for A, B, C, and D regions
  std::string Aregion = (highPtRegion + "_regionA_" + abcdSystematic).c_str();
  std::string Bregion = (lowPtRegion + "_regionB_" + abcdSystematic).c_str();
  std::string Cregion = (highPtRegion + "_regionC_" + abcdSystematic).c_str();
  std::string Cregion_temp = (highPtRegion + "_regionC_" + abcdSystematic).c_str();
  std::string Dregion = (lowPtRegion + "_regionD_" + abcdSystematic).c_str();

  //cout<<"Pre trim region names A,B,C,D,Ceff : "<<Aregion<<", "<<Bregion<<", "<<Cregion<<", "<<Dregion<<", "<<Ceff_mbb<<endl;
  // really stupid way of taking care of non-standard way opt2000 hh window cut is named
  std::string t_xhhmass = xhhmass;

  if ( verbose ) //|| (analysisRegion.find("SR")==string::npos && analysisRegion.find("mBBcr")==string::npos) )
    {
      cout << "**** Aregion: " << Aregion << endl;
      cout << "**** Bregion: " << Bregion << endl;
      cout << "**** Cregion: " << Cregion << endl;
      cout << "**** Dregion: " << Dregion << endl;
    }
  if(verbose) cout<<"returnABCDestimation, analysisRegion: "<<analysisRegion<<"\t highPtRegion: "<<highPtRegion << "\t lowPtRegion: "<<lowPtRegion<<endl;
  
  
  // *** 2. Read appropriate histograms
  TH1D *data_A, *prompt_A, *ewk_A, *ttbar_A, *Wv221_A, *SingleTop_A, *Dibosons_A, *Z_A, *Xhh_A = new TH1D();
  TH1D *data_B, *prompt_B, *ewk_B, *ttbar_B, *Wv221_B, *SingleTop_B, *Dibosons_B, *Z_B, *Xhh_B = new TH1D();
  TH1D *data_C, *prompt_C, *ewk_C, *ttbar_C, *Wv221_C, *SingleTop_C, *Dibosons_C, *Z_C, *Xhh_C = new TH1D();
  TH1D *data_D, *prompt_D, *ewk_D, *ttbar_D, *Wv221_D, *SingleTop_D, *Dibosons_D, *Z_D, *Xhh_D = new TH1D();
  returnRegionHistograms(array, analysisRegion, Aregion, data_A, prompt_A, ewk_A, ttbar_A, Wv221_A, SingleTop_A, Dibosons_A, Z_A, Xhh_A);
  returnRegionHistograms(array, analysisRegion, Bregion, data_B, prompt_B, ewk_B, ttbar_B, Wv221_B, SingleTop_B, Dibosons_B, Z_B, Xhh_B);
  returnRegionHistograms(array, analysisRegion, Cregion, data_C, prompt_C, ewk_C, ttbar_C, Wv221_C, SingleTop_C, Dibosons_C, Z_C, Xhh_C);
  returnRegionHistograms(array, analysisRegion, Dregion, data_D, prompt_D, ewk_D, ttbar_D, Wv221_D, SingleTop_D, Dibosons_D, Z_D, Xhh_D);
  if (verbose) cout << "doScaling method, (PRE) prompt_A yield: " << prompt_A->Integral(1,500) << endl;

  // Lines to check effect of using mBBcr for B/D ratio in SR calculation
  //returnRegionHistograms(array, ("mBBcr_" + optStrategy).c_str(), Bregion, data_B, prompt_B, ewk_B, ttbar_B, Wv221_B, SingleTop_B, Dibosons_B, Z_B, Xhh_B);
  //returnRegionHistograms(array, ("mBBcr_" + optStrategy).c_str(), Dregion, data_D, prompt_D, ewk_D, ttbar_D, Wv221_D, SingleTop_D, Dibosons_D, Z_D, Xhh_D);

  if(verbose) cout<<"returnABCDestimation, post FindObject"<<endl;
  
  // *** 3. Build ABCD regions
  TH1D* nA = (TH1D*)data_A->Clone();
  TH1D* nB = (TH1D*)data_B->Clone();
  TH1D* nC = (TH1D*)data_C->Clone();
  TH1D* nD = (TH1D*)data_D->Clone(); 

  if(doSignalInjection) {

    TH1D* hhA = (TH1D*)Xhh_A->Clone();
    TH1D* hhB = (TH1D*)Xhh_B->Clone();
    TH1D* hhC = (TH1D*)Xhh_C->Clone();
    TH1D* hhD = (TH1D*)Xhh_D->Clone();
    
    nA->Add(hhA);
    nB->Add(hhB);
    nC->Add(hhC);
    nD->Add(hhD);
  }
    
  // *** 4. now do normal subtraction
  nA->Add(ewk_A, -1);
  nA->Add(prompt_A, -1);
  nB->Add(ewk_B, -1);
  nB->Add(prompt_B, -1);
  nC->Add(ewk_C, -1);  
  nC->Add(prompt_C, -1);
  nD->Add(ewk_D, -1);
  nD->Add(prompt_D, -1);
  if(verbose) std::cout << "Post MC subtraction for the usual suspects" << std::endl;
  if(verbose) cout << "doScaling method, (SANITY) prompt_A yield: " << prompt_A->Integral(1,500) << endl;

  // set bins with negative yield to zero
  for(int i = 1; i < nA->GetNbinsX() + 1; i++) {
    if (nA->GetBinContent(i) < 0)   nA->SetBinContent(i, 0);
    if (nB->GetBinContent(i) < 0)   nB->SetBinContent(i, 0);
    if (nC->GetBinContent(i) < 0)   nC->SetBinContent(i, 0);
    if (nD->GetBinContent(i) < 0)   nD->SetBinContent(i, 0);
  }
  if(verbose) std::cout << "Post setting negative yields to zero" << std::endl;

  // *** 5. Store data - MC for non-prompt leakage coefficients
  storeNonPromptHistos(array, nA, nB, nC, nD);
if(verbose) std::cout << "Post storing non-prompt histograms" << std::endl;
 
  // *** 6. Now grab stopped B/D if past point, walk back event selections used for B and D regions
  if (userBDregion != "") { // pass stopping point as argument --> just for tests
    trimRegionNames(userBDregion, Bregion, Dregion);
  }
  else{ // use default stopping points
    // ** i. Replacement for reOptNonRes strings            [ mww | bbpt210 | bbpt300 | wwpt250 | mbb ]
    if (optStrategy.find("reOptNonRes")!=string::npos) 
      trimRegionNames2("bbpt210", Bregion, Cregion_temp, Dregion);
    // ** ii. Replacement for reOpt500 strings              [ mww | bbpt210 | wwpt150 | mhh | mbb ]
    else if (optStrategy.find("reOpt500")!=string::npos) 
      trimRegionNames2("bbpt210", Bregion, Cregion_temp, Dregion);
    // ** iii. Replacement for reOpt700 strings             [ mww | bbpt210 | wwpt250 | mhh | mbb ]
    else if (optStrategy.find("reOpt700")!=string::npos) 
      trimRegionNames2("bbpt210", Bregion, Cregion_temp, Dregion);
    // ** iv. Replacement for reOpt2000 strings             [ bbpt350 | wwpt250 | drww15 | mhh | mbb ]
    else if (optStrategy.find("reOpt2000")!=string::npos) 
      trimRegionNames2("wwpt250", Bregion, Cregion_temp, Dregion);

    nB = returnArrayHisto(array, ( "nonprompt_" + analysisRegion + "_" + Bregion).c_str() );
    nD = returnArrayHisto(array, ( "nonprompt_" + analysisRegion + "_" + Dregion).c_str() );

  }

  
  if(verbose){
    cout<<"returnABCDestimation, pre subtraction [SR] nA->Integral() = "<<nA->Integral(4,6)<<endl;
    cout<<"returnABCDestimation, pre subtraction [SR] nB->Integral() = "<<nB->Integral(4,6)<<endl;
    cout<<"returnABCDestimation, pre subtraction [SR] nC->Integral() = "<<nC->Integral(4,6)<<endl;
    cout<<"returnABCDestimation, pre subtraction [SR] nD->Integral() = "<<nD->Integral(4,6)<<endl;
    cout<<"returnABCDestimation, post subtraction [SR] nA->Integral() = "<<nA->Integral(4,6)<<endl;
    cout<<"returnABCDestimation, post subtraction [SR] nB->Integral() = "<<nB->Integral(4,6)<<endl;
    cout<<"returnABCDestimation, post subtraction [SR] nC->Integral() = "<<nC->Integral(4,6)<<endl;
  }
  
  // *** 7. Choose which approach to calculate ABCD normalization
  double rMC = 0;
  double rMC2, rMC2_err = 0;
  // *** Approach A: Really simple estimation
  if(!useVBSMethod) {

    // ** I. Gets mBBcr histogram names after first cut
    std::string sA, sB, sC, sD = "";
    returnRegionStrings( ("mBBcr_" + optStrategy).c_str(), cutflow.at(0), sA, sB, sC, sD);

    // ** a. Re-calculate R_non-prompt after user-specified point in cutflow
    if(doSecondRnp){
      /*if( optStrategy == "reOpt500" && highPtRegion.find("wwpt150") !=string::npos )
	returnRegionStrings( ("mBBcr_" + optStrategy).c_str(), "mww_wwpt150" , sA, sB, sC, sD);
      if( optStrategy == "reOpt700" && highPtRegion.find("wwpt250") !=string::npos )
	returnRegionStrings( "mBBcr_reOpt500", "mww_wwpt150" , sA, sB, sC, sD);
      if( optStrategy == "reOptNonRes" && highPtRegion.find("wwpt250") !=string::npos )
      returnRegionStrings( "mBBcr_reOpt500", "mww_wwpt150" , sA, sB, sC, sD);*/

      // mww_bbpt210_wwpt150
      if( optStrategy == "reOpt500" && highPtRegion.find("wwpt150") !=string::npos )
	returnRegionStrings( ("mBBcr_" + optStrategy).c_str(), "mww_bbpt210_wwpt150" , sA, sB, sC, sD);
      if( optStrategy == "reOpt700" && highPtRegion.find("wwpt250") !=string::npos )
	returnRegionStrings( "mBBcr_reOpt500", "mww_bbpt210_wwpt150" , sA, sB, sC, sD);
      if( optStrategy == "reOptNonRes" && highPtRegion.find("wwpt250") !=string::npos )
	returnRegionStrings( "mBBcr_reOpt500", "mww_bbpt210_wwpt150" , sA, sB, sC, sD);
      
    }
    
    //returnRegionStrings( ("mBBcr_" + optStrategy).c_str(), (cutflow.at(0) + "_" + cutflow.at(1)).c_str(), sA, sB, sC, sD);
    //returnRegionStrings( optStrategy, cutflow.at(0), sA, sB, sC, sD);
    TH1D* nA_cr = returnArrayHisto(array, ( "nonprompt_" + sA).c_str() );
    TH1D* nB_cr = returnArrayHisto(array, ( "nonprompt_" + sB).c_str() );
    TH1D* nC_cr = returnArrayHisto(array, ( "nonprompt_" + sC).c_str() );
    TH1D* nD_cr = returnArrayHisto(array, ( "nonprompt_" + sD).c_str() );
    /*
    // ** II. Gets mBBcr histogram names at each cut
    TH1D* nA_cr = returnArrayHisto(array, ( "nonprompt_mBBcr_" + optStrategy + "_" + Aregion).c_str() );
    TH1D* nB_cr = returnArrayHisto(array, ( "nonprompt_mBBcr_" + optStrategy + "_" + Bregion).c_str() );
    TH1D* nC_cr = returnArrayHisto(array, ( "nonprompt_mBBcr_" + optStrategy + "_" + Cregion).c_str() );
    TH1D* nD_cr = returnArrayHisto(array, ( "nonprompt_mBBcr_" + optStrategy + "_" + Dregion).c_str() );
    */
    rMC = ( nA_cr->Integral()*nD_cr->Integral() ) / ( nB_cr->Integral()*nC_cr->Integral() );
    double iA, eA, iB, eB, iC, eC, iD, eD, fracA, fracB, fracC, fracD = 0;
    iA = nA_cr->IntegralAndError(1, nA_cr->GetNbinsX(), eA);
    iB = nB_cr->IntegralAndError(1, nB_cr->GetNbinsX(), eB);
    iC = nC_cr->IntegralAndError(1, nC_cr->GetNbinsX(), eC);
    iD = nD_cr->IntegralAndError(1, nD_cr->GetNbinsX(), eD);
    fracA = eA/iA;
    fracB = eB/iB;
    fracC = eC/iC;
    fracD = eD/iD;
    rMC2 = ( iA*iD ) / ( iB*iC );
    rMC2_err = rMC2 * sqrt( fracA*fracA + fracB*fracB + fracC*fracC + fracD*fracD );


    // ====**** CALCULATION OF QCD NORMALIZATION ****====
    nA_QCD_calc = rMC* nB->Integral() * nC->Integral() / nD->Integral() ;//Here we need to * by mbb efficiency
    
    if( verbose && analysisRegion.find("mBBcr")==string::npos && analysisRegion.find("SR")==string::npos) {
      cout << "[SIMPLE] returnABCDestimation: nA: " << nA->Integral() << "\tnB: " << nB->Integral() << "\tnC: " << nC->Integral() << "\tnD: " << nD->Integral() << endl;
      //cout << "[SIMPLE] returnABCDestimation: iA: " << iA             << "\tiB: " << iB             << "\tiC: " << iC             << "\tiD: " << iD             << endl;
      //cout << "[SIMPLE] returnABCDestimation: eA: " << eA             << "\teB: " << eB             << "\teC: " << eC             << "\teD: " << eD             << endl;
      cout << "[SIMPLE] returnABCDestimation: rMC = " << rMC << endl;
      cout << "[SIMPLE] returnABCDestimation: rMC2 = " << rMC2 << " +/- " << rMC2_err << endl;
      cout << "[SIMPLE] returnABCDestimation: rMC2_err : eA/iA =" << fracA << "\t eB/iB =" << fracB <<  "\t eC/iC =" << fracC <<  "\t eD/iD =" << fracD << endl;
      cout << "[SIMPLE] returnABCDestimation: nA_QCD_calc = " <<  nA_QCD_calc << endl;
    }
  }
  
  // *** Approach B: maths from VBS Z+jet estimation
  else if(useVBSMethod) {   
    // ** I. leakage coefficients
    double cB, cC, cD = 0;
    cout << "returnABCDestimation, ttbar_A = " << ("ttbar_SR_opt700_" + Aregion).c_str() << endl;
    cB = prompt_B->Integral() / prompt_A->Integral(); 
    cC = prompt_C->Integral() / prompt_A->Integral(); 
    cD = prompt_D->Integral() / prompt_A->Integral(); 
    cout<<"returnABCDestimation, cB = "<<cB<<"\tcC = "<<cC<<"\tcD = "<<cD<<endl;
    
    
    // ** II. Store yields
    double nA_calc = data_A->Integral() - ewk_A->Integral();
    double nB_calc = data_B->Integral() - ewk_B->Integral();
    double nC_calc = data_C->Integral() - ewk_C->Integral();
    double nD_calc = data_D->Integral() - ewk_D->Integral();
    //if (nD_calc <= 0) nD_calc = 1;

    // ** III. R_MC
    if(verbose) {
      cout<<"returnABCDestimation, c_i names: " << ("X" + xhhmass + "_" + analysisRegion + "_" +Aregion).c_str()<<endl;
      
      cout << "returnABCDestimation, X" + xhhmass + "_A = " << Xhh_A->Integral() <<
	" , X" + xhhmass + "_B = " << Xhh_B->Integral() <<
	" , X" + xhhmass + "_C = " << Xhh_C->Integral() <<
	" , X" + xhhmass + "_D = " << Xhh_D->Integral() << endl;
    }
    //double rMC = ( Xhh_A_SR->Integral(1, Xhh_A_SR->GetNbinsX() + 1) * Xhh_D_SR->Integral(1, Xhh_D_SR->GetNbinsX() + 1) ) / ( Xhh_C_SR->Integral(1, Xhh_C_SR->GetNbinsX() + 1) * Xhh_B_SR->Integral(1, Xhh_B_SR->GetNbinsX() + 1) );
    //double rMC = ( Xhh_A->Integral(1, Xhh_A->GetNbinsX() + 1) * Xhh_D->Integral(1, Xhh_D->GetNbinsX() + 1) ) / ( Xhh_C->Integral(1, Xhh_C->GetNbinsX() + 1) * Xhh_B->Integral(1, Xhh_B->GetNbinsX() + 1) );
    rMC = 1;
    //double rMC = ( nA_calc * nD_calc ) / ( nC_calc * nB_calc );
    cout<<"returnABCDestimation, R_MC = "<<rMC<<endl;
    
    
    // ** IV. Do calculation
    // Assume that N_a = N_a,ewkMC + N_a,Xhh + N_a,QCD -->
    //             N_a,QCD = N_a - N_a,ewkMC - N_a,Xhh
    
    double EE = nD_calc + cD*nA_calc + cC*rMC*nB_calc + cB*rMC*nC_calc;
    double GG = 2 * (cB*cC*rMC - cD);
    double FF = ( (2*GG) / (EE*EE) ) * ( nA_calc*nD_calc - rMC*nB_calc*nD_calc);
    
    cout<<"returnABCDestimation, E = "<<EE<<"\tG = "<<GG<<"\tF = "<<FF<<endl;
    double nA_ewk_calc =  (EE * ( sqrt(1 + FF) - 1)) / GG ; 
    cout<<"returnABCDestimation, nA_ewk_calc = "<<nA_ewk_calc<<"\tprompt_A = "<<prompt_A->Integral()<<"\tdata_A = "<<data_A->Integral()<<endl;

    // ================== calculation of normalization in A region """"""""""+++++++++++*ççççççççç%&/
    nA_QCD_calc = data_A->Integral() - nA_ewk_calc - prompt_A->Integral();// - Wv221_A->Integral(1, Wv221_A->GetNbinsX() + 1);
    //double nA_QCD_calc    = data_A->Integral() - nA_ewk_calc - ewk_A->Integral();// - Wv221_A->Integral(1, Wv221_A->GetNbinsX() + 1);
    
    cout<<"returnABCDestimation, nA_QCD_ABCD = "<<nA_QCD_calc<<endl;
  }



  if (verbose) cout<<"returnABCDestimation, pre clone [SR] nC->Integral() = "<<nC->GetBinContent(4)<<endl;
  TH1D* h_nA_QCD_calc = (TH1D*)nC->Clone();
  //cout<<"returnABCDestimation, post clone [SR] nC->Integral() = "<<nC->GetBinContent(4)<<endl;
  if (verbose) cout<<"returnABCDestimation, post clone [SR] h_nA_QCD_calc->Integral() = "<<h_nA_QCD_calc->GetBinContent(4)<<endl;
  if (verbose) cout<<"returnABCDestimation, post clone [SR+CR] h_nA_QCD_calc->Integral() = "<<h_nA_QCD_calc->Integral()<<endl;
  if( nC->Integral() != 0)
    h_nA_QCD_calc->Scale( nA_QCD_calc / nC->Integral() );
  if (verbose) cout<<"returnABCDestimation, post-scale [SR+CR] h_nA_QCD_ABCD->Integral() = "<<h_nA_QCD_calc->Integral()<<endl;
  if (verbose) cout<<"returnABCDestimation, [SR] h_nA_QCD_ABCD->Integral() = "<<h_nA_QCD_calc->GetBinContent(4)<<endl;
  if (verbose) cout<<"returnABCDestimation, [CR] h_nA_QCD_ABCD->Integral() = "<<h_nA_QCD_calc->Integral() - h_nA_QCD_calc->GetBinContent(4)<<endl;
  
  // set bins with negative yield to zero
  //double norm = h_nA_QCD_calc->Integral();
  for(int i = 1; i < h_nA_QCD_calc->GetNbinsX() + 2; i++) {
    if (h_nA_QCD_calc->GetBinContent(i) < 0)
      h_nA_QCD_calc->SetBinContent(i, 0);
  }
  //h_nA_QCD_calc->Scale( norm / h_nA_QCD_calc->Integral() );
  
  // ** 6. Draw shapes plot if requested
  TCanvas *c_temp = new TCanvas("c_temp", "c_temp", 800, 800);
  //cout<<"returnABCDestimation, pre draw [SR] nC->Integral() = "<<nC->Integral(4,6)<<endl;
  if(drawPlot && highPtRegion.find("mbb")==string::npos )
    drawABCDplot((analysisRegion + "_" + Aregion).c_str(), c_temp, nA, nB, nC, nD, h_nA_QCD_calc->Integral());
  //cout<<"returnABCDestimation, post draw [SR] nC->Integral() = "<<nC->Integral(4,6)<<endl;
  delete c_temp;
  
  // ** 7. Calculate bin-by-bin stat errors for QCD estimate
  double intA, intB, intC, intD, errA, errB, errC, errD, err = 0;
  for(int b = 1; b < h_nA_QCD_calc->GetNbinsX(); b++){
    /*intA = data_A->GetBinContent(b) - prompt_A->GetBinContent(b) - ewk_A->GetBinContent(b);
    intB = data_B->GetBinContent(b) - prompt_B->GetBinContent(b) - ewk_B->GetBinContent(b);
    intC = data_C->GetBinContent(b) - prompt_C->GetBinContent(b) - ewk_C->GetBinContent(b);
    intD = data_D->GetBinContent(b) - prompt_D->GetBinContent(b) - ewk_D->GetBinContent(b);
    errA = sqrt( prompt_A->GetBinError(b)*prompt_A->GetBinError(b) + ewk_A->GetBinError(b)*ewk_A->GetBinError(b) + data_A->GetBinError(b)*data_A->GetBinError(b));
    errB = sqrt( prompt_B->GetBinError(b)*prompt_B->GetBinError(b) + ewk_B->GetBinError(b)*ewk_B->GetBinError(b) + data_B->GetBinError(b)*data_B->GetBinError(b));
    errC = sqrt( prompt_C->GetBinError(b)*prompt_C->GetBinError(b) + ewk_C->GetBinError(b)*ewk_C->GetBinError(b) + data_C->GetBinError(b)*data_C->GetBinError(b));
    errD = sqrt( prompt_D->GetBinError(b)*prompt_D->GetBinError(b) + ewk_D->GetBinError(b)*ewk_D->GetBinError(b) + data_D->GetBinError(b)*data_D->GetBinError(b));
    */
    intA = nA->GetBinContent(b);
    intB = nB->GetBinContent(b);
    intC = nC->GetBinContent(b);
    intD = nD->GetBinContent(b);
    errA = nA->GetBinError(b);
    errB = nB->GetBinError(b);
    errC = nC->GetBinError(b);
    errD = nD->GetBinError(b);
    
    if(intA < 0) {
      intA = 0;
      errA = 0;
    }
    if(intB < 0) {
      intB = 0;
      errB = 0;
    }
    if(intC < 0) {
      intC = 0;
      errC = 0;
    }
    if(intD < 0) {
      intD = 0;
      errD = 0;
    }
    
    //if(verbose) cout<<"çççç intA: "<< intA <<" +/- " << errA << "\t intB: "<< intB  <<" +/- " << errB <<"\t intC: "<< intC  <<" +/- " << errC <<"\t intD: "<< intD  <<" +/- " << errD << endl;

    if(useVBSMethod)
      err = errC;
    else {
      double compB = intB != 0 ? (errB/intB)*(errB/intB) : 0;
      double compC = intC != 0 ? (errC/intC)*(errC/intC) : 0;
      double compD = intD != 0 ? (errD/intD)*(errD/intD) : 0;
      /*// limit fractional stat error on B, C, and D regions to 100%
      if(compB > 1) compB = 1;
      if(compC > 1) compC = 1;
      if(compD > 1) compD = 1;*/
      err =  h_nA_QCD_calc->GetBinContent(b) * sqrt( compB + compC + compD ) ;
      err = sqrt( err*err + rMC2_err*rMC2_err);
      //err =  intD > 0 ?  sqrt( compB + compC + compD ) : 0;

      if(verbose){
      //if(analysisRegion.find("mBBcr")==string::npos && analysisRegion.find("SR")==string::npos && b==4) {
	cout << "çççç intB: " << intB << "\t intC: " << intC << "\t intD: " << intD << endl;
	cout << "çççç errB: " << errB << "\t errC: " << errC << "\t errD: " << errD << endl;
	cout << "çççç errB/intB: " << sqrt(compB) << "\t errC/intC: " << sqrt(compC) << "\t errD/intD: " << sqrt(compD) << endl;
	cout << "çççç Bin " << b << " , content +/- err: " << h_nA_QCD_calc->GetBinContent(b) << " +/- " << err << endl;
	cout << "çççç %err B: " << compB / (compB + compC + compD) << " %err C: " << compC / (compB + compC + compD) << " %err D: " << compD / (compB + compC + compD) << endl;

	double bdRatio = intD > 0 ? intB/intD : 0;
	double bdError = bdRatio * sqrt(compB + compD);

	cout << "çççç nB->GetName(): " << nB->GetName() << "\t nC->GetName(): " << nC->GetName() << "\t nD->GetName(): " << nD->GetName() << endl;
      }

      //err = intD > 0 ? errC * rMC*intB/(intD) : 0;
      //err = errC * sqrt(nA_QCD_calc);
      //err = errC;
    }
    h_nA_QCD_calc->SetBinError(b, err);
    //if(verbose)
    //cout << "Bin " << b << " has content of " << h_nA_QCD_calc->GetBinContent(b) << " +/- " << h_nA_QCD_calc->GetBinError(b) << endl;
  }
  
  return h_nA_QCD_calc;
  
}

void storeNonPromptHistos(TObjArray* array, TH1D* nA, TH1D* nB, TH1D* nC, TH1D* nD)
{
  if(verbose){ cout << "storeNonPromptHistos 1 " << endl;
    cout << "storeNonPromptHistos, nA->Integral(): " << nA->Integral() << endl;
  }
  std::string nonPromptName = returnNonPromptName(nA);
  if(verbose) cout << "storeNonPromptHistos, nonPromptName (A): " << nonPromptName << endl;
  TH1D* tA = (TH1D*)nA->Clone();
  tA->SetName(nonPromptName.c_str());
  array->AddLast(tA);
  if(verbose) std::cout << "storeNonPromptHistos, post A" << std::endl;
		
  nonPromptName = returnNonPromptName(nB);
  TH1D* tB = (TH1D*)nB->Clone();
  tB->SetName(nonPromptName.c_str());
  array->AddLast(tB);
  if(verbose) std::cout << "storeNonPromptHistos, post B" << std::endl;

  nonPromptName = returnNonPromptName(nC);
  TH1D* tC = (TH1D*)nC->Clone();
  tC->SetName(nonPromptName.c_str());
  array->AddLast(tC);
  if(verbose) std::cout << "storeNonPromptHistos, post C" << std::endl;

  nonPromptName = returnNonPromptName(nD);
  TH1D* tD = (TH1D*)nD->Clone();
  tD->SetName(nonPromptName.c_str());
  array->AddLast(tD);
  if(verbose) std::cout << "storeNonPromptHistos, post D" << std::endl;

  return;
}

std::string returnNonPromptName(TH1D* h0)
{
  if(verbose) cout << "returnNonPromptName 1" << endl;
  if(verbose) cout << "returnNonPromptName, name: " << h0->GetName() << endl;
  std::string name = h0->GetName();
  size_t f = name.find("data");
  name.replace(f, std::string( "data" ).length(), "nonprompt");
  if(verbose) cout << " **** h0->GetName: " << h0->GetName() << " to be " << name << endl;
  
  return name;
}

void makeRTable(TObjArray* array, std::string sample)
{
  cout<< std::fixed << std::setprecision(2);
  rNonPromptTXT << std::fixed << std::setprecision(2);
  int nCuts = cutflow.size() - 1;
  
  std::string printSample = sample;
  if(sample == "nonprompt") 
    printSample = "QCD";
  else if(sample == "ttbar")
    printSample = "\\ttbar";
  else if(sample == "data")
    printSample = "Data";
  
  if(dumpTexFiles) {
    if(printSample=="QCD") {
      if (dumpTexFiles) rNonPromptTXT << "\\begin{tabular}{c";
      for(int c = 1; c < nCuts; c++)
	if (dumpTexFiles) rNonPromptTXT << "|c";
      if (dumpTexFiles) rNonPromptTXT << "}" << endl;
    }

    if (dumpTexFiles) rNonPromptTXT << "\\hline\\hline" << endl;
    if (dumpTexFiles) rNonPromptTXT << "\\multicolumn{" << nCuts << "}{c}{" << printSample << " $R_{non-prompt}$ Values}\\\\\\hline\\hline " << endl;

  }
  cout<<" ========== R Table for "<<optStrategy<<" , " << sample << " ========== "<<endl;
  cout << cutflow.at(0) ;
  if (dumpTexFiles) rNonPromptTXT << cutflow.at(0) ;

  for (int c = 1; c < nCuts; c++) {
    cout << " \t& " << cutflow.at(c) ;
    if (dumpTexFiles) rNonPromptTXT << " \t& " << cutflow.at(c) ;
  }
  cout << "\t\\\\\\hline " << endl;
  if (dumpTexFiles) rNonPromptTXT << "\t\\\\\\hline " << endl;
  
  std::string sA, sB, sC, sD, s_cutLevel = "";
  TH1D *nA_cr, *nB_cr, *nC_cr, *nD_cr = new TH1D();
  double iA, eA, iB, eB, iC, eC, iD, eD, rMC2, rMC2_err, fracA, fracB, fracC, fracD = 0;

  for (int c = 0; c < nCuts; c++) {
    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    returnRegionStrings( ("mBBcr_" + optStrategy).c_str(), s_cutLevel, sA, sB, sC, sD);
    //returnRegionStrings( optStrategy, cutflow.at(0), sA, sB, sC, sD);
    nA_cr = returnArrayHisto(array, ( sample + "_" + sA).c_str() );
    nB_cr = returnArrayHisto(array, ( sample + "_" + sB).c_str() );
    nC_cr = returnArrayHisto(array, ( sample + "_" + sC).c_str() );
    nD_cr = returnArrayHisto(array, ( sample + "_" + sD).c_str() );

    iA = nA_cr->IntegralAndError(1, nA_cr->GetNbinsX(), eA);
    iB = nB_cr->IntegralAndError(1, nB_cr->GetNbinsX(), eB);
    iC = nC_cr->IntegralAndError(1, nC_cr->GetNbinsX(), eC);
    iD = nD_cr->IntegralAndError(1, nD_cr->GetNbinsX(), eD);
    fracA = eA/iA;
    fracB = eB/iB;
    fracC = eC/iC;
    fracD = eD/iD;
    rMC2 = ( iA*iD ) / ( iB*iC );
    rMC2_err = rMC2 * sqrt( fracA*fracA + fracB*fracB + fracC*fracC + fracD*fracD );
    //rMC2 = ( iD ) / ( iB );
    //rMC2_err = rMC2 * sqrt( fracB*fracB + fracD*fracD );

    cout << rMC2 << " $\\pm$ " << rMC2_err;
    if (dumpTexFiles) rNonPromptTXT << rMC2 << " $\\pm$ " << rMC2_err;
    if(c != nCuts -1) {
      cout << " \t& " ;
      if (dumpTexFiles) rNonPromptTXT << " \t& " ;
    }
  }
  cout << "\t\\\\\\hline " << endl;
  if (dumpTexFiles) rNonPromptTXT << "\t\\\\\\hline " << endl;

  return;
}

void dumpQCDhistograms(TFile *& fOutput, TObjArray* array, std::string region)
{
  TH1D *h0 = new TH1D();
  fOutput->cd();
  std::string s_cutLevel = "";
  for (int c = 0; c < cutflow.size(); c++) {
    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    
    if( array->FindObject( ("QCD_" + region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str()) ) {
      h0 = returnArrayHisto(array, ("QCD_" + region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
      h0->Write();
    }
    else{
      cout << ("QCD_" + region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() << " NOT FOUND" << endl;
      cout << "region: " << region << ", s_cutLevel: " << s_cutLevel << endl;
    }
  }
  
  return;
}

  
void dumpQCDhistograms_monica(TFile *& fOutput, TObjArray* array, std::string region)
{
  TH1D *h0 = new TH1D();
  TH1D *httbar = new TH1D();
  TH1D *hdata = new TH1D();
  TH1D *hWv = new TH1D();
  TH1D *hZ = new TH1D();
  TH1D *hDibos = new TH1D();
  TH1D *hSingleT = new TH1D();
  TH1D *hSM = new TH1D();

  TH1D *h0_c = new TH1D();
  TH1D *httbar_c = new TH1D();
  TH1D *hdata_c = new TH1D();
  TH1D *hWv_c = new TH1D();
  TH1D *hZ_c = new TH1D();
  TH1D *hDibos_c = new TH1D();
  TH1D *hSingleT_c = new TH1D();
  TH1D *hSM_c = new TH1D();

  TH1D *h0_indiv = new TH1D();

  fOutput->cd();
  std::string s_cutLevel = "";
  for (int c = 0; c < cutflow.size(); c++) {
    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    h0 = returnArrayHisto(array, ("QCD_" + region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    httbar = returnArrayHisto(array, ("ttbar_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    hWv = returnArrayHisto(array, ("Wv221_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    if(lepType == "IsoOR"){
      hDibos = returnArrayHisto(array, ("Dibosonsv221_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
      hZ = returnArrayHisto(array, ("Zv221_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    }
    else if(lepType == "TightMM"){
      hDibos = returnArrayHisto(array, ("Dibosons_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
      hZ = returnArrayHisto(array, ("Z_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    }
    hSingleT = returnArrayHisto(array, ("SingleTop_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    hSM = returnArrayHisto(array, ("X" + xhhmass + "_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    // some protection against dumping unblinded data
    /*if (!addSR){ 
      if (region.find("SR_")!=string::npos) {
	hdata = returnArrayHisto(array, ("data_" + region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
	}*/
    hdata = returnArrayHisto(array, ("data_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    
    h0_c = returnArrayHisto(array, ("nonprompt_" + region + "_" + s_cutLevel + "_" + varABCD + "_regionC_" + abcdSystematic).c_str() );
    httbar_c = returnArrayHisto(array, ("ttbar_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionC_" + abcdSystematic).c_str() );
    hWv_c = returnArrayHisto(array, ("Wv221_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionC_" + abcdSystematic).c_str() );
    hZ_c = returnArrayHisto(array, ("Zv221_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionC_" + abcdSystematic).c_str() );
    hDibos_c = returnArrayHisto(array, ("Dibosonsv221_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionC_" + abcdSystematic).c_str() );
    hSingleT_c = returnArrayHisto(array, ("SingleTop_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionC_" + abcdSystematic).c_str() );
    hSM_c = returnArrayHisto(array, ("X" + xhhmass + "_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionC_" + abcdSystematic).c_str() );
    hdata_c = returnArrayHisto(array, ("data_" +  region + "_" + s_cutLevel + "_" + varABCD + "_regionC_" + abcdSystematic).c_str() );

    // QCD shape for individual cuts
    if(c!= (cutflow.size()-1) )
      h0_indiv = returnArrayHisto(array, ("QCD_" + region + "_" + cutflow.at(c) + "_" + varABCD + "_regionA_" + abcdSystematic).c_str() );
    
    h0->Write();
    httbar->Write();
    hdata->Write();
    hWv->Write();
    hZ->Write();
    hDibos->Write();
    hSingleT->Write();
    hSM->Write();

    h0_c->Write();
    httbar_c->Write();
    hdata_c->Write();
    hWv_c->Write();
    hZ_c->Write();
    hDibos_c->Write();
    hSingleT_c->Write();
    hSM_c->Write();

    h0_indiv->Write();
  }
  
  return;
}
