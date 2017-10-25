//include statements
#include "AtlasStyle.h"
#include "TROOT.h"
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
#include <fstream>
#include <vector>

bool scaleTTbar, onlyTTbar, addSR, normPlots, verbose, qcdMM, printPlots, useVBSMethod, useScalingMethod, useOldCutflow, doSignalInjection, addSystematics, doBDRegionStudy, doTtbarNFvsQCDstudy, dumpTexFiles, doStabilityTest, doMbbShapeStudy, doSecondRnp;
std::string topDir;
ofstream yieldTXT, ttbarNFvsQCDstudyTXT, rNonPromptTXT, abcdStabilityTXT, mbbShapeTXT;
std::string xhhmass, optStrategy, lepType, lepChannel, varABCD, abcdSystematic;

vector<std::string> cutflow;

void setCutflowVector()
{
  if (optStrategy == "opt500") {
    cutflow.push_back("mww");
    cutflow.push_back("bbpt170");
    cutflow.push_back("drbb20");
    cutflow.push_back("wwpt160");
    cutflow.push_back("drww15");
    cutflow.push_back("mindrlb15");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");
  }
  else if (optStrategy == "opt700ichep") {
    /*cutflow.push_back("Rnp_mww");
    cutflow.push_back("drww09");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");*/
    cutflow.push_back("mww");
    cutflow.push_back("bbpt150");
    cutflow.push_back("drbb11");
    cutflow.push_back("drww09");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");
  }
  else if (optStrategy == "opt700") {
    cutflow.push_back("mww");
    cutflow.push_back("bbpt150");
    cutflow.push_back("bbpt260");
    cutflow.push_back("drbb15");
    cutflow.push_back("wwpt200");
    cutflow.push_back("mindrlb20");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");
  }
  else if (optStrategy == "opt1400") {
    cutflow.push_back("mhh_bbpt300");
    cutflow.push_back("wwpt400");
    cutflow.push_back("mindrlb25");
    cutflow.push_back("dphilmet08");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");
  }
  else if (optStrategy == "opt2000ichep") {
    cutflow.push_back("bbpt350");
    cutflow.push_back("wwpt360");
    cutflow.push_back("drww20");
    /*if( xhhmass.find("hh2")!=string::npos || xhhmass.find("hh3")!=string::npos)
      cutflow.push_back("hh2000above");      
      else*/
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");
  }
  else if (optStrategy == "opt2000") {
    cutflow.push_back("mhh_bbpt350");
    cutflow.push_back("drbb10");
    cutflow.push_back("wwpt400");
    cutflow.push_back("mindrlb25");
    cutflow.push_back("dphilmet08");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");

  }
  else if (optStrategy == "optNoRes" && useOldCutflow) {
    // pre Mar 27, 2017 (re-optimization not done with IsoOR leptons)
    //cutflow.push_back("Rnp");
    cutflow.push_back("mww");
    cutflow.push_back("bbpt150");
    cutflow.push_back("bbpt230");
    cutflow.push_back("drbb12");  
    cutflow.push_back("drww11");
    cutflow.push_back("mbb");
  }
  else if (optStrategy == "optNoRes") {
    //optNoRes_mww_bbpt150_wwpt160_drww09_dphilmet20_mbb
    cutflow.push_back("mww");
    cutflow.push_back("bbpt150");
    cutflow.push_back("wwpt160");
    cutflow.push_back("drww09");
    cutflow.push_back("dphilmet20");
    cutflow.push_back("mbb");
  }
  else if(optStrategy == "reOptNonRes"){
    cutflow.push_back("mww");
    cutflow.push_back("bbpt210");
    cutflow.push_back("bbpt300");
    cutflow.push_back("wwpt250");
    cutflow.push_back("mbb");
  }
  else if(optStrategy == "reOpt500"){
    cutflow.push_back("mww");
    cutflow.push_back("bbpt210");
    cutflow.push_back("wwpt150");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");
  }
  else if(optStrategy == "reOpt700"){
    cutflow.push_back("mww");
    cutflow.push_back("bbpt210");
    cutflow.push_back("wwpt250");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");
  }
  else if(optStrategy == "reOpt2000"){
    cutflow.push_back("bbpt350");
    cutflow.push_back("wwpt250");
    cutflow.push_back("drww15");
    cutflow.push_back(xhhmass);
    cutflow.push_back("mbb");
  }
  else if(optStrategy == "reOptUnBlind"){
    cutflow.push_back("mww130");
    cutflow.push_back("bbpt150");
    cutflow.push_back("bbpt260");
    cutflow.push_back("drbb15");
    cutflow.push_back("wwpt250");
    cutflow.push_back("mindrlb25");
    cutflow.push_back("mbb");
  }
  else {
    cout << "Honestly, I don't know how this is possible, but you passed setRunOptions but made it to setCutflowVector with a non-recognizex optStrategy. \n ######## EXITING NOW ######## " << endl; 
    exit(0);
  }
  
  return;
}

void setRunOptions(std::string s_optStrategy, std::string s_xhhmass, std::string s_lepType, std::string s_lepChannel, std::string s_varABCD)
{
  // *** 1. Set opt strategy
  if ( !(s_optStrategy == "opt500" || s_optStrategy == "opt700" || s_optStrategy == "opt700ichep" || s_optStrategy == "opt1400" || s_optStrategy == "optNoResichep" || s_optStrategy == "optNoRes" || s_optStrategy == "opt2000" || s_optStrategy == "opt2000ichep" || s_optStrategy == "reOptNonRes" || s_optStrategy == "reOpt500" || s_optStrategy == "reOpt700" || s_optStrategy == "reOpt2000" || s_optStrategy == "reOptUnBlind") ) {
    cout << "ERROR, optStrategy of " << s_optStrategy << "not recognized. EXITING NOW..." << endl;
    exit(0);
  }
  else
    optStrategy = s_optStrategy;

  // *** 2. Set xhh case
  if ( !(s_xhhmass == "hh500" || s_xhhmass == "hh600" || 
	 s_xhhmass == "hh700" || s_xhhmass == "hh700a" || s_xhhmass == "hh700b" || s_xhhmass == "hh700c" || s_xhhmass == "hh700d" ||
	 s_xhhmass == "hh750" || s_xhhmass == "hh750a" || s_xhhmass == "hh750b" || s_xhhmass == "hh750c" || s_xhhmass == "hh750d" || 
	 s_xhhmass == "hh800" || s_xhhmass == "hh800a" || s_xhhmass == "hh800b" || s_xhhmass == "hh800c" || s_xhhmass == "hh800d" ||
	 s_xhhmass == "hh900" || s_xhhmass == "hh900a" || s_xhhmass == "hh900b" || s_xhhmass == "hh900c" || s_xhhmass == "hh900d" ||  
	 s_xhhmass == "hh1000" || s_xhhmass == "hh1000a" || s_xhhmass == "hh1000b" || s_xhhmass == "hh1000c" || s_xhhmass == "hh1000d" ||
	 s_xhhmass == "hh1100" || s_xhhmass == "hh1100a" || s_xhhmass == "hh1100b" || s_xhhmass == "hh1100c" || s_xhhmass == "hh1100d" ||
	 s_xhhmass == "hh1200" || s_xhhmass == "hh1200a" || s_xhhmass == "hh1200b" || s_xhhmass == "hh1200c" || s_xhhmass == "hh1200d" ||
	 s_xhhmass == "hh1300" || s_xhhmass == "hh1300a" || s_xhhmass == "hh1300b" || s_xhhmass == "hh1300c" || s_xhhmass == "hh1300d" ||
	 s_xhhmass == "hh1400" || s_xhhmass == "hh1400a" || 
	 s_xhhmass == "hh1500" || s_xhhmass == "hh1500a" || 
	 s_xhhmass == "hh1600" || s_xhhmass == "hh1600a" || 
	 s_xhhmass == "hh1800" || s_xhhmass == "hh2000" || s_xhhmass == "hh2250" || 
	 s_xhhmass == "hh2500" || s_xhhmass == "hh2750" || s_xhhmass == "hh3000" || s_xhhmass == "hhSM") ) {
    cout << "ERROR, xhhmass of " << s_xhhmass << "not recognized. EXITING NOW..." << endl;
    exit(0);
  }
  else
    xhhmass = s_xhhmass;
  
  // *** 3. Set lepton type
  if ( !(s_lepType == "TightMM" || s_lepType == "IsoOR") ) {
    cout << "ERROR, lepType of " << s_lepType << "not recognized. EXITING NOW..." << endl;
    exit(0);
  }
  else
    lepType = s_lepType;
  
  // *** 4. Set lepton channel
  if ( !(s_lepChannel == "el" || s_lepChannel == "mu" || s_lepChannel == "el_mu") ) {
    cout << "ERROR, lepChannel of " << s_lepChannel << "not recognized. EXITING NOW..." << endl;
    exit(0);
  }
  else
    lepChannel = s_lepChannel;

  // *** 5. Set ABCD variable
  if ( s_varABCD == "")
    varABCD = "bbMass";
  else
    varABCD = s_varABCD;

  // *** 6. Set ABCD default region
  abcdSystematic = "met25d020";

  cout << "\n\n**************************************************************************" << endl;
  cout << "*********************          Run Options           *********************" << endl;
  cout << "optStrategy: " << optStrategy << endl;
  cout << "xhhmass: " << xhhmass << endl;
  cout << "lepType: " << lepType << endl;
  cout << "lepChannel: " << lepChannel << endl;
  cout << "varABCD: " << varABCD << endl;
  cout << "doStabilityTest: " << doStabilityTest << endl;
  cout << "addSystematics: " << addSystematics << endl;
  cout << "**************************************************************************" << endl;
  cout << "**************************************************************************\n\n" << endl;
  
  // *** 7. Set ABCD variable
  if (lepChannel == "el")
    varABCD = ("el_" + varABCD).c_str();
  if (lepChannel == "mu")
    varABCD = ("mu_" + varABCD).c_str();

  
  // set cutflow vectors
  setCutflowVector();

  return;
}

void beginLatexTable(ofstream& txt, std::string title)
{
  txt.open(title);

  txt << "\\documentclass{article}" << endl;
  txt << "\\usepackage{multirow, xspace, amsmath}" << endl;
  txt << "\\usepackage{geometry}\n" << endl;
  txt << " \\geometry{" << endl;
  txt << " a4paper," << endl;
  txt << " total={170mm,257mm}," << endl;
  txt << " left=20mm," << endl;
  txt << " top=20mm," << endl;
  txt << " }\n" << endl;
  txt << "\\newcommand*{\\ttbar}{\\ensuremath{t\\bar{t}}\\xspace}" << endl;
  txt << "\\newcommand*{\\mbb}{\\ensuremath{\\text{m}_{bb}}\\xspace}" << endl;
  txt << "\\newcommand*{\\dsig}{\\ensuremath{|\\sigma_{d_{0}}|}\\xspace} \n" << endl;

  txt << "\\begin{document}" << endl;
  txt << "\\begin{center}" << endl;
  txt << "\\begin{table}\\fontsize{7}{8}\\selectfont" << endl;
  
 return;
}

void endLatexTable(ofstream& txt)
{
  txt << "\\end{tabular}" << endl;
  txt << "\n\\end{table}" << endl;
  txt << "\n\\end{center}" << endl;
  txt << "\\end{document}" << endl;

  txt.close();
  
  return;
}

TH1D* returnArrayHistoLocal(TObjArray* array, std::string hname)
{
  TH1D* temp = new TH1D();

  if( array->FindObject( hname.c_str()) )
    temp = (TH1D*)array->FindObject( hname.c_str() );
  else {
    if(verbose) cout<<"returnArrayHistoLocal, NOT FOUND: "<<hname<<endl;
  }

  TH1D* hist = (TH1D*)temp->Clone();
  
  return hist;
}

void printSample(TObjArray* array, std::string region, std::string sample, double totalBkg[], double errBkg[], std::string options = "")
{
  cout<< std::fixed << std::setprecision(1);
  yieldTXT << std::fixed << std::setprecision(1);

  int nCuts = cutflow.size();
  //if (!addSR) nCuts--;
  
  if( sample == "data" && !addSR && region.find("SR")!=string::npos) {
    cout<< "\\hline"<<endl;
    cout << "Data";
    if(dumpTexFiles) yieldTXT << "\\hline"<<endl;
    if(dumpTexFiles) yieldTXT << "Data";
    
    for (int c = 0; c < nCuts; c++) {
      cout << " \t& --- ";
      if(dumpTexFiles) yieldTXT << " \t& --- ";
    }
    
    cout<< "\t\\\\\\hline "<<endl;
    cout<< "\\hline"<<endl;
    if(dumpTexFiles) yieldTXT << "\t\\\\\\hline "<<endl;
    if(dumpTexFiles) yieldTXT << "\\hline"<<endl;
    
    return;
  }
  
  if( sample == "Background Sum") {
    cout<< "\\hline"<<endl;
    cout<< sample;
    if(dumpTexFiles) yieldTXT << "\\hline"<<endl;
    if(dumpTexFiles) yieldTXT << sample;
  
    for (int c = 0; c < nCuts; c++) {
      cout << " \t& "<< totalBkg[c] << "$\\pm$ " << sqrt(errBkg[c]);
      if(dumpTexFiles) yieldTXT << " \t& "<< totalBkg[c] << "$\\pm$ " << sqrt(errBkg[c]);
    }
    
    cout << "\t\\\\\\hline "<<endl;
    cout << "\\hline"<<endl;
    if(dumpTexFiles) yieldTXT << "\t\\\\\\hline "<<endl;
    if(dumpTexFiles) yieldTXT << "\\hline"<<endl;
    
    return;
  }
  
  std::string var = varABCD;
  
  if(sample == "DD Background") 
    sample = "QCD";
  
  TH1D *h0 = new TH1D();
  std::string s_cutLevel = "";
  double integral, err = 0;
  if(sample != "data" && sample != "ttbar" && (sample=="QCD" || options.find("bdOnly")==string::npos) ){
    cout << sample;
    if(dumpTexFiles) yieldTXT << sample;
    if(options.find("abcdStability")!=string::npos)
      cout << (" (" + abcdSystematic + ")").c_str() ;
  }
  else if (sample == "data"){
    cout << "Data";
    if(dumpTexFiles) yieldTXT << "Data";
  }
  else if (sample == "ttbar" && options.find("bdOnly")==string::npos){
    cout << "\\ttbar";
    if(dumpTexFiles) yieldTXT << "\\ttbar";
  }

 // now do normal table stuff
  for (int c = 0; c < nCuts; c++) {
    integral = 0;
    err = 0;

    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  

    // ** B. Read histogram
    h0          = returnArrayHistoLocal(array, (sample + "_" + region + "_" + s_cutLevel + "_" + var + "_regionA_" + abcdSystematic).c_str() );
    //h0          = returnArrayHistoLocal(array, (sample + "_" + region + "_" + s_cutLevel + "_" + var + "_regionC_" + abcdSystematic).c_str() );
    integral    = h0->IntegralAndError(1, h0->GetNbinsX(), err);
    /*
    totalBkg[c] += integral;
    errBkg[c]   += err;
    cout << " \t& " << integral;
    if (sample != "data") cout << " $\\pm$ " << sqrt( err );
    */
    totalBkg[c] += h0->Integral();
    err = 0;
    if(sample!="QCD")
      err         = h0->GetEntries() > 0    ? ( h0->Integral()*h0->Integral()) / h0->GetEntries() : 0;
    else{
      //h0->Print("all");
      for(int h = 1; h < h0->GetNbinsX() + 1; h++){
	err += h0->GetBinError(h)*h0->GetBinError(h);
	//if( verbose)
	//cout << "QCD bin " << h << " has content " << h0->GetBinContent(h) << " and error of " << h0->GetBinError(h) << endl;
      }
    }
    errBkg[c]   += err;
    
    if( (sample=="QCD" || sample=="data") || options.find("bdOnly")==string::npos) {
      cout << " \t& " << integral;
      if(dumpTexFiles) yieldTXT << " \t& " << integral;
      if (sample != "data"){
	cout << " $\\pm$ " << sqrt( err );
	if(dumpTexFiles) yieldTXT << " $\\pm$ " << sqrt( err );
      }
    }
  }
  
  if( sample=="QCD" || options.find("bdOnly")==string::npos) {
    cout << "\t\\\\\\hline " << endl;
    if(dumpTexFiles) yieldTXT << "\t\\\\\\hline " << endl;
  }
  
}

void makeYieldTable(TObjArray* array, std::string region, std::string BDregion = "")
{
  double totalBkg[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  double errBkg[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int nCuts = cutflow.size();
  //if (!addSR) nCuts--;
  
  cout<<" ========== Yield Table for "<<region<<" ========== "<<endl;
  if( BDregion != "")  cout<<" ========== B,D Evaluated at " << BDregion <<" ========== "<<endl;
  if(dumpTexFiles){
    if( region.find("mBBcr")!=string::npos) {
      if(dumpTexFiles) yieldTXT <<  "\\begin{tabular}{l";
      for (int c = 0; c < nCuts; c++)
	if(dumpTexFiles) yieldTXT << "|c";
      if(dumpTexFiles) yieldTXT << "}" << endl;
      if(dumpTexFiles) yieldTXT << "\\hline\\hline" << endl;
      if(dumpTexFiles) yieldTXT << "\\multicolumn{" << nCuts << "}{c}{\\textbf{CR}: \\mbb Sideband}\\\\\\hline\\hline" << endl;
    }
    else if( region.find("SR")!=string::npos) {
      if(dumpTexFiles) yieldTXT << "\\hline" << endl;
      if(dumpTexFiles) yieldTXT << "\\multicolumn{" << nCuts << "}{c}{\\textbf{SR}: 100 $<$ \\mbb $<$ 140 GeV}\\\\\\hline\\hline" << endl;
      //if(dumpTexFiles) yieldTXT << "\\multicolumn{" << nCuts << "}{c}{\\textbf{SR}: 105 $<$ \\mbb $<$ 135 GeV}\\\\\\hline\\hline" << endl;
    }
  }
  cout<<"Sample ";
  if(dumpTexFiles) yieldTXT << "Sample ";

  for (int c = 0; c < nCuts; c++) {
    cout << " \t& " << cutflow.at(c) ;
    if(dumpTexFiles) yieldTXT << " \t& " << cutflow.at(c) ;
  }
  cout<< "  \\\\\\hline"<<endl;
  if(dumpTexFiles) yieldTXT<< "  \\\\\\hline"<<endl;
  
  //if( BDregion == "")
  std::string printOpts = "";
  if(BDregion!="")
    printOpts="bdOnly";
  
  printSample(array, region, "ttbar", totalBkg, errBkg, printOpts);
  printSample(array, region, "DD Background", totalBkg, errBkg, printOpts);
  if(!onlyTTbar){// && BDregion == "" ) {
    printSample(array, region, "Wv221", totalBkg, errBkg, printOpts);
    printSample(array, region, "SingleTop", totalBkg, errBkg, printOpts);
   
    if(lepType == "IsoOR"){
      printSample(array, region, "Dibosonsv221", totalBkg, errBkg, printOpts);
      printSample(array, region, "Zv221", totalBkg, errBkg, printOpts);
    }
    else if(lepType == "TightMM"){
      printSample(array, region, "Dibosons", totalBkg, errBkg, printOpts);
      printSample(array, region, "Z", totalBkg, errBkg, printOpts);
    }
  }
  printSample(array, region, "Background Sum", totalBkg, errBkg, printOpts);
  if(xhhmass.find("a") !=string::npos || xhhmass.find("b") !=string::npos || xhhmass.find("c") !=string::npos || xhhmass.find("d") !=string::npos) 
    printSample(array, region, ("X" + xhhmass.substr(0,xhhmass.size()-1)).c_str(), totalBkg, errBkg, printOpts);  
  else printSample(array, region, ("X" + xhhmass).c_str(), totalBkg, errBkg, printOpts);
  printSample(array, region, "data", totalBkg, errBkg, printOpts);
  
  cout<<" ================================ "<<endl;
  
}

void printBDregionYields(TObjArray* array, std::string BDregion)
{

  makeYieldTable(array, ("mBBcr_" + optStrategy).c_str(), BDregion);
  makeYieldTable(array, ("SR_" + optStrategy).c_str(), BDregion);

  return;
}

void compareABCDvariations(TObjArray* nom, TObjArray* metVar, TObjArray* d0Var, std::string region)
{
  double qcd_nom[10], qcd_d0Var[10], qcd_metVar[10], err_nom[10], err_d0Var[10], err_metVar[10];// = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  for (int s = 0; s < 10; s++) {
    qcd_nom[s]    = 0.0;
    qcd_metVar[s] = 0.0;
    qcd_d0Var[s]  = 0.0;
    err_nom[s]    = 0.0;
    err_metVar[s] = 0.0;
    err_d0Var[s]  = 0.0;
  }
  int nCuts = cutflow.size();
  
  cout<<" ========== Comparing ABCD Stability Variations for "<<region<<" ========== "<<endl;
  cout<<"Sample \t\t";
  
  for (int c = 0; c < nCuts; c++) {
    cout << " \t& " << cutflow.at(c) ;
  }
  cout<< "  \\\\\\hline"<<endl;

  bool storeDumpTexFiles = dumpTexFiles;
  dumpTexFiles = false;
  std::string storeNominalABCD = abcdSystematic;
  std::string printOpts = "abcdStability";
  printSample(nom, region, "DD Background", qcd_nom, err_nom, printOpts);
  //printSample(nom, region, "Wv221", totalBkg, errBkg, true);
  //printSample(nom, region, "ttbar", totalBkg, errBkg, true);
  
  abcdSystematic = "met25d022";
  printSample(d0Var, region, "DD Background", qcd_d0Var, err_d0Var, printOpts);
  //printSample(d0var, region, "Wv221", totalBkg, errBkg, true);
  //printSample(d0Var, region, "ttbar", totalBkg, errBkg, true);
  //abcdSystematic = "met225d020";
  abcdSystematic = "met15d020";
  printSample(metVar, region, "DD Background", qcd_metVar, err_metVar, printOpts);
  //printSample(metVar, region, "Wv221", totalBkg, errBkg, true);
  //printSample(metVar, region, "ttbar", totalBkg, errBkg, true);
  abcdSystematic = storeNominalABCD;
  dumpTexFiles = storeDumpTexFiles;
  
  cout << " ================= Systematic Error Summary ================= " << endl;
  cout << " Cut \t & Nominal \t & Nom. -- \\dsig$_{2.0\\rightarrow 2.2}$ \t & Nom. -- MET$_{25\\rightarrow 22.5}$ \t & Nom. $\\pm$ Stat $\\pm$ Syst \t\\\\\\hline " << endl;

  if(dumpTexFiles) {
    abcdStabilityTXT << std::fixed << std::setprecision(1);
      
    if( region.find("mBBcr")!=string::npos) {
      abcdStabilityTXT <<  "\\begin{tabular}{l|c|c|c|c}" << endl;
      abcdStabilityTXT << "\\hline\\hline" << endl;
      abcdStabilityTXT << "\\multicolumn{5}{c}{\\textbf{CR}: \\mbb Sideband}\\\\\\hline\\hline" << endl;
    }
    else if( region.find("SR")!=string::npos) {
      abcdStabilityTXT << "\\hline" << endl;
      abcdStabilityTXT << "\\multicolumn{5}{c}{\\textbf{SR}: 100 $<$ \\mbb $<$ 140 GeV}\\\\\\hline\\hline" << endl;
    }
    abcdStabilityTXT << " Cut \t & Nominal \t & Nom. -- \\dsig$_{2.0\\rightarrow 2.2}$ \t & Nom. -- MET$_{25\\rightarrow 22.5}$ \t & Nom. $\\pm$ Stat $\\pm$ Syst \t\\\\\\hline " << endl;
  }
  
  double totSyst = 0;
  for (int c = 0; c < cutflow.size(); c++) {
    totSyst = sqrt( (qcd_nom[c] - qcd_d0Var[c])*(qcd_nom[c] - qcd_d0Var[c]) + (qcd_nom[c] - qcd_metVar[c])*(qcd_nom[c] - qcd_metVar[c]) );
    cout << cutflow.at(c) << " \t & " << qcd_nom[c] << " \t & " << qcd_nom[c] - qcd_d0Var[c] << " \t & " << qcd_nom[c] - qcd_metVar[c] << " \t & " << qcd_nom[c] << " $\\pm$ " << sqrt(err_nom[c]) << " $\\pm$ " << totSyst<< " \t \\\\\\hline " << endl;
    if(dumpTexFiles)
      abcdStabilityTXT << cutflow.at(c) << " \t & " << qcd_nom[c] << " \t & " << qcd_nom[c] - qcd_d0Var[c] << " \t & " << qcd_nom[c] - qcd_metVar[c] << " \t & " << qcd_nom[c] << " $\\pm$ " << sqrt(err_nom[c]) << " $\\pm$ " << totSyst<< " \t \\\\\\hline " << endl;
  }
  if(dumpTexFiles)
    abcdStabilityTXT << "\\hline" << endl;
  
  cout << " ============================================================ " << endl;

  return;
}
