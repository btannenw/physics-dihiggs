#include <iostream>
#include <TFile.h>
#include <TCanvas.h>
#include <TDirectory.h>
#include <TLatex.h>

void runDump(int mass, std::string filename, std::string label);

void dumpPullPlotCorrMatrix()
{

  // Sept 26 (unblinded)
  runDump(3000, "fccs/FitCrossChecks_90017.260916-v3-observedPullPlots-x3000_HH_13TeV_260916-v3-observedPullPlots-x3000_Systs_3000_combined/FitCrossChecks.root", "X3000");
  runDump(2000, "fccs/FitCrossChecks_90014.260916-v3-observedPullPlots-x2000_HH_13TeV_260916-v3-observedPullPlots-x2000_Systs_2000_combined/FitCrossChecks.root", "X2000");
  runDump(1000, "fccs/FitCrossChecks_80009.260916-v3-observedPullPlots-x1000_HH_13TeV_260916-v3-observedPullPlots-x1000_Systs_1000_combined/FitCrossChecks.root", "X1000");
  runDump(600, "fccs/FitCrossChecks_80005.260916-v3-observedPullPlots-x600_HH_13TeV_260916-v3-observedPullPlots-x600_Systs_600_combined/FitCrossChecks.root", "X600");

  // Sept 26 (unblinded)
  //runDump(3000, "fccs/FitCrossChecks_90017.260916-v0-sep23inputs_fullRange_classicSelections-x3000_HH_13TeV_260916-v0-sep23inputs_fullRange_classicSelections-x3000_Systs_3000_combined/FitCrossChecks.root", "X3000");
  //runDump(2750, "fccs/FitCrossChecks_90016.260916-v0-sep23inputs_fullRange_classicSelections-x2750_HH_13TeV_260916-v0-sep23inputs_fullRange_classicSelections-x2750_Systs_2750_combined/FitCrossChecks.root", "X2750");
  //runDump(2000, "fccs/FitCrossChecks_90014.260916-v0-sep23inputs_fullRange_classicSelections-x2000_HH_13TeV_260916-v0-sep23inputs_fullRange_classicSelections-x2000_Systs_2000_combined/FitCrossChecks.root", "X2000");
  //runDump(1300, "fccs/FitCrossChecks_80011.260916-v1-opt700allMasses_fullRange-x1300_HH_13TeV_260916-v1-opt700allMasses_fullRange-x1300_Systs_1300_combined/FitCrossChecks.root", "X1300");
  //runDump(750, "fccs/FitCrossChecks_80007.260916-v0-sep23inputs_fullRange_classicSelections-x750_HH_13TeV_260916-v0-sep23inputs_fullRange_classicSelections-x750_Systs_750_combined/FitCrossChecks.root", "X750");
  //runDump(600, "fccs/FitCrossChecks_80005.260916-v0-sep23inputs_fullRange_classicSelections-x600_HH_13TeV_260916-v0-sep23inputs_fullRange_classicSelections-x600_Systs_600_combined/FitCrossChecks.root", "X600");
 
  // Sept 20 (unblinded)
  //runDump(3000, "fccs/FitCrossChecks_90017.200916-v0-unblindedResults-x3000_HH_13TeV_200916-v0-unblindedResults-x3000_Systs_3000_combined/FitCrossChecks.root", "X3000");
  //runDump(2750, "fccs/FitCrossChecks_90016.200916-v0-unblindedResults-x2750_HH_13TeV_200916-v0-unblindedResults-x2750_Systs_2750_combined/FitCrossChecks.root", "X2750");
  
  // Sept 15 (50% single top unc indepedent in SR and CR)
  //runDump(600, "fccs/FitCrossChecks_80005.190916-v11-fullRange_singleTop50percentIndependentInSRandCR-x600_HH_13TeV_190916-v11-fullRange_singleTop50percentIndependentInSRandCR-x600_Systs_600_combined/FitCrossChecks.root", "X600");
  //runDump(750, "fccs/FitCrossChecks_80007.190916-v11-fullRange_singleTop50percentIndependentInSRandCR-x750_HH_13TeV_190916-v11-fullRange_singleTop50percentIndependentInSRandCR-x750_Systs_750_combined/FitCrossChecks.root", "X750");
  //runDump(1300, "fccs/FitCrossChecks_90011.190916-v11-fullRange_singleTop50percentIndependentInSRandCR-x1300_HH_13TeV_190916-v11-fullRange_singleTop50percentIndependentInSRandCR-x1300_Systs_1300_combined/FitCrossChecks.root", "X1300");
//runDump(2000, "fccs/FitCrossChecks_90014.190916-v11-fullRange_singleTop50percentIndependentInSRandCR-x2000_HH_13TeV_190916-v11-fullRange_singleTop50percentIndependentInSRandCR-x2000_Systs_2000_combined/FitCrossChecks.root", "X2000");


  // Sept 15 (50% single top unc)
  //runDump(600, "fccs/FitCrossChecks_80005.150916-fullLimits_normalSelections_50percentSingleTopUnc-x600_HH_13TeV_150916-fullLimits_normalSelections_50percentSingleTopUnc-x600_Systs_600_combined/FitCrossChecks.root", "X600");
  //runDump(750, "fccs/FitCrossChecks_80007.150916-fullLimits_normalSelections_50percentSingleTopUnc-x750_HH_13TeV_150916-fullLimits_normalSelections_50percentSingleTopUnc-x750_Systs_750_combined/FitCrossChecks.root", "X750");
  //runDump(1300, "fccs/FitCrossChecks_90011.150916-fullLimits_normalSelections_50percentSingleTopUnc-x1300_HH_13TeV_150916-fullLimits_normalSelections_50percentSingleTopUnc-x1300_Systs_1300_combined/FitCrossChecks.root", "X1300");
  //runDump(2000, "fccs/FitCrossChecks_90014.150916-fullLimits_normalSelections_50percentSingleTopUnc-x2000_HH_13TeV_150916-fullLimits_normalSelections_50percentSingleTopUnc-x2000_Systs_2000_combined/FitCrossChecks.root", "X2000");
  
  // Sept 14
  //runDump(500, "fccs/FitCrossChecks_80005.140916-v0-lowMassRankingAndPulls-x600_HH_13TeV_140916-v0-lowMassRankingAndPulls-x600_Systs_600_combined/FitCrossChecks.root", "X600");
  //runDump(750, "fccs/FitCrossChecks_80007.140916-v0-lowMassRankingAndPulls-x750_HH_13TeV_140916-v0-lowMassRankingAndPulls-x750_Systs_750_combined/FitCrossChecks.root", "X750");
  //runDump(1300, "fccs/FitCrossChecks_90011.140916-v0-hiMassRankingAndPulls-x1300_HH_13TeV_140916-v0-hiMassRankingAndPulls-x1300_Systs_1300_combined/FitCrossChecks.root", "X1300");
  //runDump(2000, "fccs/FitCrossChecks_90014.140916-v0-hiMassRankingAndPulls-x2000_HH_13TeV_140916-v0-hiMassRankingAndPulls-x2000_Systs_2000_combined/FitCrossChecks.root", "X2000");

 // Sept 13
 //runDump(500, "fccs/FitCrossChecks_80004.130916-lowMass_onePercentPruning-x500_HH_13TeV_130916-lowMass_onePercentPruning-x500_Systs_500_combined/FitCrossChecks.root", "X500");
 //runDump(600, "fccs/FitCrossChecks_80005.130916-lowMass_onePercentPruning-x600_HH_13TeV_130916-lowMass_onePercentPruning-x600_Systs_600_combined/FitCrossChecks.root", "X600");
 //runDump(800, "fccs/FitCrossChecks_80008.130916-lowMass_onePercentPruning-x800_HH_13TeV_130916-lowMass_onePercentPruning-x800_Systs_800_combined/FitCrossChecks.root", "X800");

  //runDump(700);
  //runDump(750);
  //runDump(1300);
  //runDump(2000);

}

void runDump(int mass, std::string filename, std::string label)
{
  std::string outDir = "sep26_plots";
  /*
  if (mass == 700){
    label = "X700";
    filename = "fccs/FitCrossChecks_80006.130916-v4-700withOnePercentPruning-x700_HH_13TeV_130916-v4-700withOnePercentPruning-x700_Systs_700_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80006.120916-v7-pulls_usingRadSpecificSuyogCalculatedTTbarMod-x700_HH_13TeV_120916-v7-pulls_usingRadSpecificSuyogCalculatedTTbarMod-x700_Systs_700_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80006.090916-v1-lowMass_fullRange_useSuyogNumbersTTbarMod-x700_HH_13TeV_090916-v1-lowMass_fullRange_useSuyogNumbersTTbarMod-x700_Systs_700_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80006.080916-v4-noRadLo_fullMassRange-x700_HH_13TeV_080916-v4-noRadLo_fullMassRange-x700_Systs_700_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80006.080916-v0-ttbarModelingHandledByHistos-x700_HH_13TeV_080916-v0-ttbarModelingHandledByHistos-x700_Systs_700_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80006.070916-v1-lowMass_pullPlots_ttbarMEandPSseperateInSRonly-x700_HH_13TeV_070916-v1-lowMass_pullPlots_ttbarMEandPSseperateInSRonly-x700_Systs_700_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80006.060916-v9_14point1percentTTbarMEandPS-x700_HH_13TeV_060916-v9_14point1percentTTbarMEandPS-x700_Systs_700_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80006.050916-v90-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether14point5-x700_HH_13TeV_050916-v90-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether14point5-x700_Systs_700_combined/FitCrossChecks.root";
  }
  else if (mass == 750){
    label = "X750";
    filename = "fccs/FitCrossChecks_80007.130916-v3-0point1percentPruneSmallNorm-x750_HH_13TeV_130916-v3-0point1percentPruneSmallNorm-x750_Systs_750_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80007.120916-v7-pulls_usingRadSpecificSuyogCalculatedTTbarMod-x750_HH_13TeV_120916-v7-pulls_usingRadSpecificSuyogCalculatedTTbarMod-x750_Systs_750_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80007.090916-v1-lowMass_fullRange_useSuyogNumbersTTbarMod-x750_HH_13TeV_090916-v1-lowMass_fullRange_useSuyogNumbersTTbarMod-x750_Systs_750_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80007.080916-v4-noRadLo_fullMassRange-x750_HH_13TeV_080916-v4-noRadLo_fullMassRange-x750_Systs_750_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80007.080916-v0-ttbarModelingHandledByHistos-x750_HH_13TeV_080916-v0-ttbarModelingHandledByHistos-x750_Systs_750_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80007.070916-v1-lowMass_pullPlots_ttbarMEandPSseperateInSRonly-x750_HH_13TeV_070916-v1-lowMass_pullPlots_ttbarMEandPSseperateInSRonly-x750_Systs_750_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80007.060916-v9_14point1percentTTbarMEandPS-x750_HH_13TeV_060916-v9_14point1percentTTbarMEandPS-x750_Systs_750_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_80007.050916-v90-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether14point5-x750_HH_13TeV_050916-v90-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether14point5-x750_Systs_750_combined/FitCrossChecks.root";
  }
  else if (mass == 1300){
    label = "X1300";
    filename = "fccs/FitCrossChecks_90011.130916-v4-hiMassWithOnePercentPruning-x1300_HH_13TeV_130916-v4-hiMassWithOnePercentPruning-x1300_Systs_1300_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90011.120916-v7-pulls_usingRadSpecificSuyogCalculatedTTbarMod-x1300_HH_13TeV_120916-v7-pulls_usingRadSpecificSuyogCalculatedTTbarMod-x1300_Systs_1300_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90011.090916-v1-hiMass_fullRange_useSuyogNumbersTTbarMod-x1300_HH_13TeV_090916-v1-hiMass_fullRange_useSuyogNumbersTTbarMod-x1300_Systs_1300_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90011.080916-v4-noRadLo_fullMassRange-x1300_HH_13TeV_080916-v4-noRadLo_fullMassRange-x1300_Systs_1300_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90011.080916-v0-ttbarModelingHandledByHistos-x1300_HH_13TeV_080916-v0-ttbarModelingHandledByHistos-x1300_Systs_1300_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90011.070916-v0-highMass_pullPlots_ttbarMEandPSseperateInSRonly-x1300_HH_13TeV_070916-v0-highMass_pullPlots_ttbarMEandPSseperateInSRonly-x1300_Systs_1300_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90011.060916-v9_14point1percentTTbarMEandPS-x1300_HH_13TeV_060916-v9_14point1percentTTbarMEandPS-x1300_Systs_1300_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90011.050916-v90-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether15-x1300_HH_13TeV_050916-v90-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether15-x1300_Systs_1300_combined/FitCrossChecks.root";
  }
  else if (mass == 2000){
    label = "X2000";
    filename = "fccs/FitCrossChecks_90014.130916-v4-hiMassWithOnePercentPruning-x2000_HH_13TeV_130916-v4-hiMassWithOnePercentPruning-x2000_Systs_2000_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90014.120916-v7-pulls_usingRadSpecificSuyogCalculatedTTbarMod-x2000_HH_13TeV_120916-v7-pulls_usingRadSpecificSuyogCalculatedTTbarMod-x2000_Systs_2000_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90014.090916-v1-hiMass_fullRange_useSuyogNumbersTTbarMod-x2000_HH_13TeV_090916-v1-hiMass_fullRange_useSuyogNumbersTTbarMod-x2000_Systs_2000_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90014.080916-v4-noRadLo_fullMassRange-x2000_HH_13TeV_080916-v4-noRadLo_fullMassRange-x2000_Systs_2000_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90014.080916-v0-ttbarModelingHandledByHistos-x2000_HH_13TeV_080916-v0-ttbarModelingHandledByHistos-x2000_Systs_2000_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90014.070916-v0-highMass_pullPlots_ttbarMEandPSseperateInSRonly-x2000_HH_13TeV_070916-v0-highMass_pullPlots_ttbarMEandPSseperateInSRonly-x2000_Systs_2000_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90014.060916-v9_14point1percentTTbarMEandPS-x2000_HH_13TeV_060916-v9_14point1percentTTbarMEandPS-x2000_Systs_2000_combined/FitCrossChecks.root";
    //filename = "fccs/FitCrossChecks_90014point5.050916-v40-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether15-x2000_HH_13TeV_050916-v40-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether15-x2000_Systs_2000_combined/FitCrossChecks.root";
  }
  else{
    cout<<"Mass not recognized. EXITING!!!"<<endl;
    return;
  }
  */
  TFile *fIn = new TFile(filename.c_str(), "READ");
 
  // UNCONDITIONAL
  gDirectory->Cd("PlotsAfterFitToAsimov/unconditionnal");

  TCanvas* c_pull = (TCanvas*)gDirectory->Get("can_NuisPara_AsimovFit_unconditionnal_mu0");
  TCanvas* c_corr = (TCanvas*)gDirectory->Get("can_CorrMatrix_AsimovFit_unconditionnal_mu0");

  TH1D* h1 = (TH1D*)c_pull->GetPrimitive("h_NuisPara_AsimovFit_unconditionnal_mu0");
  h1->GetXaxis()->LabelsOption("v");
  c_pull->SetBottomMargin(0.35);
  c_pull->SetFrameLineColor(0);
  c_pull->SetFrameLineColor(1);

  TLatex l1;
  l1.SetTextAlign(9);
  l1.SetTextFont(62);
  l1.SetTextSize(0.05);
  l1.SetNDC();
  cout<<"A"<<endl;
  c_pull->Update();
  c_pull->Draw();
  l1.DrawLatex(0.4, 0.8, label.c_str());
  cout<<"A"<<endl;
  c_pull->Print((outDir+"/pullPlot_"+label+"_mu0_unconditional.eps").c_str());
  c_corr->Draw();

  l1.DrawLatex(0.4, 0.8, label.c_str());
  c_corr->Print((outDir+"/corrMatrix_"+label+"_mu0_unconditional.eps").c_str());

  // CONDITIONAL
  
  gDirectory->Cd("../conditionnal_MuIsEqualTo_0");

  TCanvas* c2_pull = (TCanvas*)gDirectory->Get("can_NuisPara_AsimovFit_conditionnal_mu0");
  TCanvas* c2_corr = (TCanvas*)gDirectory->Get("can_CorrMatrix_AsimovFit_conditionnal_mu0");

  TH1D* h2 = (TH1D*)c2_pull->GetPrimitive("h_NuisPara_AsimovFit_conditionnal_mu0");
  h2->GetXaxis()->LabelsOption("v");
  c2_pull->SetBottomMargin(0.35);
  c2_pull->SetFrameLineColor(0);
  c2_pull->SetFrameLineColor(1);

  c2_pull->Draw();
  l1.DrawLatex(0.4, 0.8, label.c_str());
  c2_pull->Print((outDir+"/pullPlot_"+label+"_mu0_conditional.eps").c_str());
  c2_corr->Draw();
  l1.DrawLatex(0.4, 0.8, label.c_str());
  c2_corr->Print((outDir+"/corrMatrix_"+label+"_mu0_conditional.eps").c_str());

  // UNCONDITIONAL [needs work]
  
  gDirectory->Cd("../../PlotsAfterGlobalFit/unconditionnal");

  TCanvas* c3_pull = (TCanvas*)gDirectory->Get("can_NuisPara_GlobalFit_unconditionnal_mu0");
  TCanvas* c3_corr = (TCanvas*)gDirectory->Get("can_CorrMatrix_GlobalFit_unconditionnal_mu0");

  TH1D* h3 = (TH1D*)c3_pull->GetPrimitive("h_NuisPara_GlobalFit_unconditionnal_mu0");
  h3->GetXaxis()->LabelsOption("v");
  c3_pull->SetBottomMargin(0.35);
  c3_pull->SetFrameLineColor(0);
  c3_pull->SetFrameLineColor(1);

  c3_pull->Draw();
  l1.DrawLatex(0.4, 0.8, label.c_str());
  c3_pull->Print((outDir+"/pullPlot_"+label+"_globalFit_mu0_unconditional.eps").c_str());
  c3_corr->Draw();
  l1.DrawLatex(0.4, 0.8, label.c_str());
  c3_corr->Print((outDir+"/corrMatrix_"+label+"_globalFit_mu0_unconditional.eps").c_str());


  return;
}
