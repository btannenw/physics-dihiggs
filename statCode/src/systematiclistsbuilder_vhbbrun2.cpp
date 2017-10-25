#include "systematiclistsbuilder_vhbbrun2.hpp"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>

#include <TString.h>

#include "configuration.hpp"
#include "containerhelpers.hpp"
#include "properties.hpp"
#include "regiontracker.hpp"
#include "sample.hpp"
#include "systematic.hpp"

void SystematicListsBuilder_VHbbRun2::fillHistoSystsRenaming() {
  // FIXME temporary rewrite rules, so we can do 0+2 fits
  //m_renameHistoSysts.emplace( "SysTTbarPTVMbb" , "SysTTbarPTVMBB");
  //
  m_renameHistoSysts.emplace( "SysFT_EFF_extrapolation_from_charm_AntiKt4EMTopoJets" , "SysFT_EFF_extrapolation_from_charm");
  m_renameHistoSysts.emplace( "SysFT_EFF_extrapolation_AntiKt4EMTopoJets" , "SysFT_EFF_extrapolation");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_0_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_0");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_1_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_1");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_2_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_2");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_3_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_3");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_4_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_4");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_5_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_5");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_6_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_6");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_7_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_7");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_8_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_8");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_9_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_9");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_10_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_10");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_11_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_11");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_12_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_12");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_Light_13_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_Light_13");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_C_0_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_C_0");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_C_1_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_C_1");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_C_2_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_C_2");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_C_3_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_C_3");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_B_0_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_B_0");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_B_1_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_B_1");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_B_2_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_B_2");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_B_3_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_B_3");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_B_4_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_B_4");
  m_renameHistoSysts.emplace( "SysFT_EFF_Eigen_B_5_AntiKt4EMTopoJets" , "SysFT_EFF_Eigen_B_5");
}

void SystematicListsBuilder_VHbbRun2::listAllUserSystematics(const RegionTracker& regTrk, bool useFltNorms) {
  using P = Property;

  bool doDiboson = m_config.getValue("DoDiboson", false);
  bool doWHZH = m_config.getValue("FitWHZH", false);
  bool doVHVZ = m_config.getValue("FitVHVZ", false);
  std::vector<TString> sig_decorr = m_config.getStringList("DecorrPOI");

  // for injection at an arbitrary mass setup in the config file
  int injection     = m_config.getValue("DoInjection", 0);
  TString massInjection = TString::Itoa(injection, 10);

  TString massPoint = m_config.getValue("MassPoint", "125");
  double xsec_injected = 1.;
  if(injection>0 && massInjection != massPoint) {
    normFact("muInjection", {"None"}, xsec_injected, -10, 10, true); // this one is constant
  }

  // DO NOT APPLY ! RESCALE ALL MC TO A DIFFERENT INTEGRATED LUMINOSITY
//  normFact("lumirescale", {"MC"}, 5.81/13.18, -10, 10, true); // this one is constant
//  normFact("lumirescale", {"MC"}, 7.37/13.18, -10, 10, true); // this one is constant

  // list them all !

  if(doWHZH) {
    addPOI("SigXsecOverSMWH", {"WH"}, 1, -40, 40);
    addPOI("SigXsecOverSMZH", {"ZH"}, 1, -40, 40);
  }
  else if(sig_decorr.size() != 0) { // TODO or just extend decorrSysForCategories for the POIs ?
    std::vector<Property> decorrelations;
    for(auto& d : sig_decorr) {
      decorrelations.push_back(Properties::props_from_names.at(d));
    }
    addPOI("SigXsecOverSM", {"Sig", {}, std::move(decorrelations)}, 1, -40, 40);
  } else if ( injection>0 ) {
    addPOI("SigXsecOverSM", {"Sig"}, xsec_injected, -40, 40);
  } else {
    //addPOI("SigXsecOverSM", {"Sig", {{}}, {P::nLep}}, 1, -40, 40);
    addPOI("SigXsecOverSM", {"Sig"}, 1, -40, 40);
  }

  // MODELLING MAINLY FROM VHBB RUN 1
  // ttbar
  // everywhere
  bool fltTop = (regTrk["has1tag"] || regTrk["has2tag"]);
  if(useFltNorms && fltTop) {
    //// From Elisabeth's study 27/06/2016
    //// floating parameters decorrelated between 2 and (0+1) lepton channels
    normFact("ttbar", {"ttbar", {}, {}, {{P::nLep, 2}}});
  }
  else {
    sampleNormSys("ttbar", 0.05);
  }

  //// From Elisabeth's study 27/06/2016
  //// 3/2J uncertainty
  // decorrelated between 2 and (0+1) lepton channels
  // 24% prior for 2-lepton, 9% prior for (0+1) lepton
  normSys("SysttbarNorm", 0.09, {"ttbar", {{ {{P::nJet, 3},{P::nLep, 0}}, {{}} },{ {{P::nJet, 3},{P::nLep, 1}}, {{}} }}, {P::nJet}});//PF
  //normSys("SysttbarNorm", 0.10, {"ttbar", {{P::nJet, 3}}, {P::nJet}});//SVN 17/07/2017
  normSys("SysttbarNorm", 0.24, {"ttbar", { { {{P::nJet, 3},{P::nLep, 2}}, {{}} } }, {P::nJet}, {{P::nLep, 2}}});


  // single top
  // cross-sections from Top MC Twiki
  sampleNormSys("stops", 0.046);
  sampleNormSys("stopt", 0.044);//PF
  sampleNormSys("stopWt", 0.062);//PF
  normSys("SysstoptAcc", 0.16, {"stopt", {{P::nJet, 2}} });//PF
  normSys("SysstoptAcc", 0.19, {"stopt", {{P::nJet, 3}} });//PF
  normSys("SysstopWtAcc", 0.25, {"stopWt", {{P::nJet, 2}} });//PF
  normSys("SysstopWtAcc", 0.32, {"stopWt", {{P::nJet, 3}} });//PF
  //normSys("SysstoptNorm", 0.16, {"stopt", {{P::nJet, 2}} });//SVN 17/07/2017
  //normSys("SysstoptNorm", 0.19, {"stopt", {{P::nJet, 3}} });//SVN 17/07/2017
  //normSys("SysstopWtNorm", 0.25, {"stopWt", {{P::nJet, 2}} });//SVN 17/07/2017
  //normSys("SysstopWtNorm", 0.32, {"stopWt", {{P::nJet, 3}} });//SVN 17/07/2017
  // Directly from VHbb Run1 code:
  // acceptance from MC comparisons
  //sampleNormSysByRegion("SysTChanPtB2", "stopt", 0.52, 0.25, 0.12, -0.18);
  //sampleNormSysByRegion("SysWtChanAcerMC", "stopWt", 0.01, -0.02, 0.04, -0.15);
  //sampleNormSysByRegion("SysWtChanPythiaHerwig", "stopWt", 0.05, 0.03, 0.05, -0.03);
  //sampleNormSysByRegion("SysSChanAcerMC", "stops", 0.13, 0.22, 0.18, 0.30);
  //sampleNormSysByRegion("SysSChanAcerMCPS", "stops", 0.06, 0.08, 0.04, 0.04);


  // V+jets

  // Wl
  if(useFltNorms && regTrk["has0tag"] && (regTrk["hasOneLep"] || regTrk["hasZeroLep"])) {
    normFact("Wl", {"Wl"});
  }
  else {
    sampleNormSys("Wl", 0.32);
  }

  // Zl
  if(useFltNorms && regTrk["has0tag"] && (regTrk["hasZeroLep"] || regTrk["hasTwoLep"])) {
    normFact("Zl", {"Zl"});
  }
  else {
    sampleNormSys("Zl", 0.18);
  }

  // Wcl and Zcl
  if(useFltNorms && regTrk["has1tag"] && (regTrk["hasOneLep"])) {
    normFact("Wcl", {"Wcl"});
  }
  else {
    sampleNormSys("Wcl", 0.37);
  }
  if(useFltNorms && regTrk["has1tag"] && (regTrk["hasTwoLep"])) {
    normFact("Zcl", {"Zcl"});
  }
  else {
    sampleNormSys("Zcl", 0.23);
  }

  // floating Wbb+Wbc+Wbl+Wcc - put extra systs on Wbl and Wcc to allow ratios to change
  // DO I NEED THIS?
  if(useFltNorms && regTrk["has2tag"] && (regTrk["hasOneLep"])) {
    normFact("Wbb", {{"Wbb", "Wbc", "Wbl", "Wcc"}});
  }
  else {
    normSys("SysWbbNorm", 0.33, {{"Wbb", "Wbc", "Wbl", "Wcc"}});
  }
  normSys("SysWbbNorm", 0.23, {"Wbb", {{P::nJet, 2}}, {P::nJet}});
  // 0/1L
  if(regTrk["has2tag"] && (regTrk["hasOneLep"])) {
     normSys("SysWbbNorm", 0.17, {"Wbb", {{P::nLep, 0}}, {P::nLep}});
  }

  // FLAVOUR COMPOSITION 
  normSys("SysWblWbbRatio", 0.17, {"Wbl", {{P::nLep, 0}}});//, {P::nJet}, {{P::nLep, 0}}});
  normSys("SysWblWbbRatio", 0.31, {"Wbl", {{P::nLep, 1}}});//, {P::nJet}, {{P::nLep, 1}}});
  //
  normSys("SysWccWbbRatio", 0.24, {"Wcc", {{P::nLep, 0}}});//, {P::nJet}, {{P::nLep, 0}}});
  normSys("SysWccWbbRatio", 0.13, {"Wcc", {{P::nLep, 1}}});//, {P::nJet}, {{P::nLep, 1}}});
  //
  normSys("SysWbcWbbRatio", 0.43, {"Wbc", {{P::nLep, 0}}});//, {P::nJet}, {{P::nLep, 0}}});
  normSys("SysWbcWbbRatio", 0.21, {"Wbc", {{P::nLep, 1}}});//, {P::nJet}, {{P::nLep, 1}}});

  // floating Zbb+Zbc+Zbl+Zcc - put extra systs on Zbl and Zcc to allow ratios to change
  if(useFltNorms && regTrk["has2tag"] && (regTrk["hasTwoLep"] || regTrk["hasZeroLep"])) {
    normFact("Zbb", {{"Zbb", "Zbc", "Zbl", "Zcc"}});
  }
  else {
    normSys("SysZbbNorm", 0.48, {{"Zbb", "Zbc", "Zbl", "Zcc"}});
  }
  // 2/3J regions
  normSys("SysZbbNorm", 0.28, {"Zbb", {{ {{P::nLep,0},{P::nJet, 2}}, {{}} }}, {P::nJet}});
  normSys("SysZbbNorm", 0.25, {"Zbb", {{ {{P::nLep,2},{P::nJet, 2}}, {{}} }}, {P::nJet}});
  // 0/2L channel
  if(regTrk["has2tag"] && (regTrk["hasTwoLep"])) {
     normSys("SysZbbNorm", 0.26, {"Zbb", {{P::nLep, 0}}, {P::nLep}});
  }

  // Flavour composition
  normSys("SysZblZbbRatio", 0.38, {"Zbl", {{ {{P::nJet, 2},{P::nLep, 2}}, {{}} }} });
  normSys("SysZblZbbRatio", 0.15, {"Zbl", {{ {{P::nJet, 3},{P::nLep, 2}}, {{}} }} });
  normSys("SysZblZbbRatio", 0.17, {"Zbl", {{P::nLep, 0}}});
  //
  normSys("SysZccZbbRatio", 0.31, {"Zcc", {{ {{P::nJet, 2},{P::nLep, 2}}, {{}} }} });
  normSys("SysZccZbbRatio", 0.07, {"Zcc", {{ {{P::nJet, 3},{P::nLep, 2}}, {{}} }} });
  normSys("SysZccZbbRatio", 0.12, {"Zcc", {{P::nLep, 0}}});
  //
  normSys("SysZbcZbbRatio", 0.27, {"Zbc", {{ {{P::nJet, 2},{P::nLep, 2}}, {{}} }} });
  normSys("SysZbcZbbRatio", 0.16, {"Zbc", {{ {{P::nJet, 3},{P::nLep, 2}}, {{}} }} });
  normSys("SysZbcZbbRatio", 0.14, {"Zbc", {{P::nLep, 0}}});

  // Diboson
  // Overall norm
  normSys("SysWWNorm", 0.25, {"WW"});
  if ( doDiboson ) { //TODO: CHECK THIS
     // Constraints across channel/region
     // 2/3J
     normSys("SysVZNorm" , 0.19, {"ZZ", {{P::nJet, 2}}, {P::nJet}});
     normSys("SysVZNorm" , 0.14, {"WZ", {{ {{P::nJet, 2},{P::nLep, 0}}, {{}} }}, {P::nJet}});
     normSys("SysVZNorm" , 0.11, {"WZ", {{ {{P::nJet, 2},{P::nLep, 1}}, {{}} }}, {P::nJet}});
     // 0/2L
     normSys("SysZZNorm" , 0.30, {"ZZ", {{P::nLep, 0}}, {}, {{P::nLep, 0}}});
     normSys("SysWZNorm" , 0.12, {"WZ", {{P::nLep, 0}}, {}, {{P::nLep, 0}}});
  } else if ( doVHVZ ) {
     // do VH fit using floating VZ
     normFact("VZ", {{"WZ", "ZZ"}});
     // Constraints across channel/region
     // 2/3J
     normSys("SysVZNorm" , 0.19, {"ZZ", {{P::nJet, 2}}, {P::nJet}});
     normSys("SysVZNorm" , 0.14, {"WZ", {{ {{P::nJet, 2},{P::nLep, 0}}, {{}} }}, {P::nJet}});
     normSys("SysVZNorm" , 0.11, {"WZ", {{ {{P::nJet, 2},{P::nLep, 1}}, {{}} }}, {P::nJet}});
     // 0/2L
     normSys("SysZZNorm" , 0.30, {"ZZ", {{P::nLep, 0}}, {}, {{P::nLep, 0}}});
     normSys("SysWZNorm" , 0.12, {"WZ", {{P::nLep, 0}}, {}, {{P::nLep, 0}}});
  } else { //TODO: CHECK THIS
     normSys("SysZZNorm", 0.20, {"ZZ"});
     normSys("SysWZNorm", 0.26, {"WZ"});
     // Constraints across channel/region
     normSys("SysZZNorm" , 0.19, {"ZZ", {{P::nJet, 2}}, {P::nJet}});
     normSys("SysZZNorm" , 0.30, {"ZZ", {{P::nLep, 0}}, {}, {{P::nLep, 0}}});
     //
     normSys("SysWZNorm" , 0.14, {"WZ", {{ {{P::nJet, 2},{P::nLep, 0}}, {{}} }}, {P::nJet}});
     normSys("SysWZNorm" , 0.11, {"WZ", {{ {{P::nJet, 2},{P::nLep, 1}}, {{}} }}, {P::nJet}});
     normSys("SysWZNorm" , 0.12, {"WZ", {{P::nLep, 0}}, {}, {{P::nLep, 0}}});
  }

  // Higgs
  //add Higgs normalisation systematic of 50% when running diboson fit
  //if(doDiboson) {
  //  normSys("SysHiggsNorm", 0.50, {"Higgs"});
  //}
  // FROM VHbb run 1 (directly)
  // Branching ratio
  // numbers from Peyton, 2014/04/02
  //std::map<TString, float> BRunc = {//SVN 17/07/2017
  //  {"100", 0.018}, {"105", 0.02}, {"110", 0.022}, {"115", 0.024}, {"120", 0.028},//SVN 17/07/2017
  //  {"125", 0.032}, {"130", 0.037}, {"135", 0.042}, {"140", 0.048}, {"145", 0.053},//SVN 17/07/2017
  //  {"150", 0.057}};//SVN 17/07/2017
  // numbers from Paolo, 2016/07/19 PF
    std::map<TString, float> BRunc = {{"125", 0.017}};//PF
  normSys("SysTheoryBRbb", BRunc.at(massPoint), {"Higgs"});

  // signal acceptance uncertainty
  // 2-jet only numbers
  //normSys("SysTheoryAcc_J2_qqVH" , 0.03, {"WlvH", {{P::nJet, 2}}});//SVN 17/07/2017
  //normSys("SysTheoryAcc_J2_qqVH" , 0.034, {"qqZH", {{P::nJet, 2}}});//SVN 17/07/2017
  //normSys("SysTheoryAcc_J2_ggZH" , 0.015, {"ggZH", {{P::nJet, 2}}});//SVN 17/07/2017
  // numbers anticorrelated in 2J and 3J
  //sampleNormSysByRegion("SysTheoryAcc_J3_qqVH", "WlvH", -0.011, -0.011, 0.041, 0.041);
  //sampleNormSysByRegion("SysTheoryAcc_J3_qqVH", "qqZH", -0.009, -0.009, 0.035, 0.035);
  //sampleNormSysByRegion("SysTheoryAcc_J3_ggZH", "ggZH", -0.019, -0.019, 0.033, 0.033);
  // PDF impact on acceptance
  //sampleNormSysByRegion("SysTheoryAccPDF_qqVH", "WlvH", 0.035, 0.035, 0.028, 0.028);
  //sampleNormSysByRegion("SysTheoryAccPDF_qqVH", "qqZH", 0.03, 0.03, 0.05, 0.05);
  //sampleNormSysByRegion("SysTheoryAccPDF_ggZH", "ggZH", 0.021, 0.021, 0.034, 0.034);
  //
  // SIGNAL SYS
  normSys("SysTheoryAcc_J2_qqVH" , 0.04, {"WlvH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J2_qqVH" , 0.006, {"qqZllH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J2_qqVH" , 0.036, {"qqZvvH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J2_ggVH" , 0.006, {"ggZllH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J2_ggVH" , 0.036, {"ggZvvH", {{P::nJet, 2}}});//PF
    
  normSys("SysTheoryAcc_J3_qqVH" , -0.030, {"WlvH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J3_qqVH" , 0.036, {"WlvH", {{P::nJet, 3}}});//PF
  normSys("SysTheoryAcc_J3_qqVH" , -0.013, {"qqZllH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J3_qqVH" , 0.014, {"qqZllH", {{P::nJet, 3}}});//PF
  normSys("SysTheoryAcc_J3_qqVH" , -0.018, {"qqZvvH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J3_qqVH" , 0.025, {"qqZvvH", {{P::nJet, 3}}});//PF
  normSys("SysTheoryAcc_J3_ggVH" , -0.013, {"ggZllH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J3_ggVH" , 0.014, {"ggZllH", {{P::nJet, 3}}});//PF
  normSys("SysTheoryAcc_J3_ggVH" , -0.018, {"ggZvvH", {{P::nJet, 2}}});//PF
  normSys("SysTheoryAcc_J3_ggVH" , 0.025, {"ggZvvH", {{P::nJet, 3}}});//PF
    
  normSys("SysTheoryAcc_JVeto_qqVH", 0.030, {"WlvH", {{P::nJet, 3}}});//PF
  normSys("SysTheoryAcc_JVeto_qqVH", 0.011, {"qqZvvH", {{P::nJet, 3}}});//PF
  normSys("SysTheoryAcc_JVeto_ggVH", 0.011, {"ggZvvH", {{P::nJet, 3}}});//PF
    
    
  normSys("SysTheoryPDFAcc_qqVH", 0.007, {"WlvH"});//PF
  normSys("SysTheoryPDFAcc_qqVH", 0.006, {"qqZvvH"});//PF
  normSys("SysTheoryPDFAcc_qqVH", 0.003, {"qqZllH"});//PF
  normSys("SysTheoryPDFAcc_ggZH", 0.006, {"ggZvvH"});//PF
  normSys("SysTheoryPDFAcc_ggZH", 0.003, {"ggZllH"});//PF
    
  normSys("SysTheoryPDFAccPS", 0.075, {"WlvH"});//PF
  normSys("SysTheoryPDFAccPS", 0.065, {"ZllH"});//PF
  normSys("SysTheoryPDFAccPS", 0.075, {"ZvvH"});//PF
  normSys("SysTheoryPDFAccPS", 0.06, {"WlvH", {{P::nJet, 3}}, {P::nJet}});//PF
  normSys("SysTheoryPDFAccPS", 0.04, {"ZllH", {{P::nJet, 3}}, {P::nJet}});//PF
  normSys("SysTheoryPDFAccPS", 0.05, {"ZvvH", {{P::nJet, 3}}, {P::nJet}});//PF
    
  // PDF and scales for total x-sec uncertainty
  //normSys("SysTheoryQCDscale", 0.01, {"qqVH", {}, {}, {}, {"qqVH"}});//SVN 17/07/2017
  //normSys("SysTheoryQCDscale", 0.5380, {"ggZH", {}, {}, {}, {"ggZH"}});//SVN 17/07/2017
  //normSys("SysTheoryPDF", 0.023, {"qqVH", {}, {}, {}, {"qqVH"}});//SVN 17/07/2017
  //normSys("SysTheoryPDF", 0.17, {"ggZH", {}, {}, {}, {"ggZH"}});//SVN 17/07/2017

  normSys("SysTheoryQCDscale", 0.007, {"qqVH", {}, {}, {}, {"qqVH"}});//PF
  normSys("SysTheoryQCDscale", 0.27, {"ggZH", {}, {}, {}, {"ggZH"}});//PF
    
  normSys("SysTheoryPDF_qqVH", 0.019, {"WlvH"});//PF
  normSys("SysTheoryPDF_qqVH", 0.016, {"qqZH"});//PF
  normSys("SysTheoryPDF_ggZH", 0.05, {"ggZH"});//PF
    
  // parton shower acceptance
  //sampleNormSysByRegion("SysTheoryAccPS", "Higgs", 0.08, 0.07, 0.13, 0.07);

  // multijet
  normSys("SysMJNorm", 0.50, {"multijet"});//BACK IN UNTIL THE SHAPE SYS ARE FIXED
  
  normSys("ATLAS_LUMI_2015_2016", 0.029, {"MC", {{P::year, 2015}}});

}


void SystematicListsBuilder_VHbbRun2::listAllHistoSystematics(const RegionTracker& regTrk) {
  using T = SysConfig::Treat;
  using S = SysConfig::Smooth;
  using P = Property;
  bool yes(true);     bool no(false); // correlate years ?

  SysConfig noSmoothConfig { T::shape, S::noSmooth };
  // TODO smoothConfig is noSmooth for the moment for tests
  //  SysConfig smoothConfig { T::shape, S::noSmooth };
                
  SysConfig smoothConfig { T::shape, S::smooth };

  // AT THE MOMENT ONLY RECO SYSTEMATIC CONSIDERED
  m_histoSysts.insert({ "SysEG_RESOLUTION_ALL" , smoothConfig});
  m_histoSysts.insert({ "SysEG_SCALE_ALL" , smoothConfig});
  m_histoSysts.insert({ "ttbar_SysEL_EFF_Reco_TOTAL_1NPCOR_PLUS_UNCOR" , noSmoothConfig});
  m_histoSysts.insert({ "ttbar_SysEL_EFF_ID_TOTAL_1NPCOR_PLUS_UNCOR" , noSmoothConfig});
  m_histoSysts.insert({ "ttbar_SysEL_EFF_Trigger_TOTAL_1NPCOR_PLUS_UNCOR" , noSmoothConfig});
  m_histoSysts.insert({ "ttbar_SysEL_EFF_Iso_TOTAL_1NPCOR_PLUS_UNCOR" , noSmoothConfig});
  //m_histoSysts.insert({ "SysEL_EFF_ID_TotalCorrUncertainty" , noSmoothConfig});
  //m_histoSysts.insert({ "SysEL_EFF_Iso_TotalCorrUncertainty" , noSmoothConfig});
  //m_histoSysts.insert({ "SysEL_EFF_Reco_TotalCorrUncertainty" , noSmoothConfig});

  m_histoSysts.insert({ "SysJET_JER_SINGLE_NP" , smoothConfig});

  m_histoSysts.insert({ "SysMETTrigStat" , smoothConfig});
  m_histoSysts.insert({ "SysMETTrigTop" , smoothConfig});
  m_histoSysts.insert({ "SysMETTrigZ" , smoothConfig});
  m_histoSysts.insert({ "SysMET_JetTrk_Scale" , smoothConfig});
  m_histoSysts.insert({ "SysMET_SoftTrk_ResoPerp" , smoothConfig});
  m_histoSysts.insert({ "SysMET_SoftTrk_ResoPara" , smoothConfig});
  m_histoSysts.insert({ "SysMET_SoftTrk_Scale" , smoothConfig});

  m_histoSysts.insert({ "SysMUON_EFF_SYS" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_EFF_STAT" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_ISO_SYS" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_ISO_STAT" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_TTVA_SYS" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_TTVA_STAT" , noSmoothConfig});
  
  m_histoSysts.insert({ "SysMUONS_SCALE" , smoothConfig});
  m_histoSysts.insert({ "SysMUONS_MS" , smoothConfig});
  m_histoSysts.insert({ "SysMUONS_ID" , smoothConfig});
  //m_histoSysts.insert({ "SysMUON_EFF_STAT_LOWPT" , smoothConfig});
  //m_histoSysts.insert({ "SysMUON_EFF_SYST_LOWPT" , smoothConfig});
 
  bool use_jet_19np = m_config.getValue("UseJet19NP",true);
  if(use_jet_19np){
    // 19NP Scheme
    m_histoSysts.insert({ "SysJET_19NP_JET_EffectiveNP_1" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EffectiveNP_2" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EffectiveNP_3" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EffectiveNP_4" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EffectiveNP_5" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EffectiveNP_6restTerm" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EtaIntercalibration_Modelling" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EtaIntercalibration_TotalStat" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EtaIntercalibration_NonClosure" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_Pileup_OffsetMu" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_Pileup_OffsetNPV" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_Pileup_PtTerm" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_Pileup_RhoTopology" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_BJES_Response" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_PunchThrough_MCTYPE" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_SingleParticle_HighPt" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_Flavor_Response" , smoothConfig});

    // Correlate
    // m_histoSysts.insert({ "SysJET_19NP_JET_Flavor_Composition" , smoothConfig});

    // Decorrelate
    m_histoSysts.insert( {"SysJET_19NP_JET_Flavor_Composition" ,  SysConfig{T::shape, S::smooth, yes, {}, {},
                                              {"Top", "Zjets", "Wjets"}}});
  }
  else{
    // 3NP Scheme
    m_histoSysts.insert({ "SysJET_SR1_JET_GroupedNP_1" , smoothConfig});
    m_histoSysts.insert({ "SysJET_SR1_JET_GroupedNP_2" , smoothConfig});
    m_histoSysts.insert({ "SysJET_SR1_JET_GroupedNP_3" , smoothConfig});
    m_histoSysts.insert({ "SysJET_19NP_JET_EtaIntercalibration_NonClosure" , smoothConfig});
  }

 
  // B-tag
  m_histoSysts.insert({ "SysFT_EFF_extrapolation_from_charm" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_extrapolation" , noSmoothConfig});
 
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_0" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_1" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_2" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_3" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_4" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_5" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_6" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_7" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_8" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_9" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_10" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_11" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_12" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_13" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_C_0" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_C_1" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_C_2" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_C_3" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_0" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_1" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_2" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_3" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_4" , noSmoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_5" , noSmoothConfig});
 
  // Background modelling
  // 1D
  m_histoSysts.insert( {"SysTTbarPTV", SysConfig{T::shape, S::noSmooth, "ttbar", {}, {}, {{P::nLep, 2}} }});
  m_histoSysts.insert( {"SysTTbarMBB", SysConfig{T::shape, S::noSmooth, "ttbar", {}, {}, {{P::nLep, 2}} }});
  // 2D
  //m_histoSysts.insert( {"SysTTbarPTVMBB", SysConfig{T::shape, S::noSmooth, "ttbar", {}, {}, {{P::nLep, 2}} }});
  //
  m_histoSysts.insert({ "SysWPtV" , noSmoothConfig});//Test
  m_histoSysts.insert({ "SysZPtV" , noSmoothConfig});//Test
  m_histoSysts.insert({ "SysWMbb" , noSmoothConfig});//Test
  m_histoSysts.insert({ "SysZMbb" , noSmoothConfig});//Test
  //
  // 2D
  //m_histoSysts.insert( {"SysWPtVMbb",  noSmoothConfig});
  //m_histoSysts.insert( {"SysZPtVMbb",  noSmoothConfig});
    
  //m_histoSysts.insert({ "SysJET_Hbb_Run1_pT" , smoothConfig});
  //m_histoSysts.insert({ "SysJET_Hbb_Run1_mass" , smoothConfig});
  //m_histoSysts.insert({ "SysJET_Hbb_Run1_D2" , { T::skip, S::smooth }});
  //m_histoSysts.insert({ "SysJET_Hbb_CrossCalib" , smoothConfig});

  //m_histoSysts.insert({ "SysVVJetScalePtST1" , noSmoothConfig});
  m_histoSysts.insert({ "SysVVMbbME" , noSmoothConfig});
  m_histoSysts.insert({ "SysVVPTVME" , noSmoothConfig});
  //m_histoSysts.insert({ "SysVVJetPDFAlphaPt" , noSmoothConfig});
  //m_histoSysts.insert({ "SysVVJetScalePtST2" , noSmoothConfig});

  
  // dropped when introducing mVH V+jets systematics
  //m_histoSysts.insert({ "SysWMbb" , noSmoothConfig});
  //m_histoSysts.insert({ "SysWDPhi" , noSmoothConfig});
  //m_histoSysts.insert({ "SysWPtV" , noSmoothConfig});
  
  //m_histoSysts.insert({ "SysZMbb" , noSmoothConfig});
  //m_histoSysts.insert({ "SysZDPhi" , noSmoothConfig});
  //m_histoSysts.insert({ "SysZPtV" , noSmoothConfig});
  
  //m_histoSysts.insert({ "SysVjetsMVH" , SysConfig{ T::shape, S::noSmooth, {"Wjets", "Zjets"}, {{P::descr,"SR"}, {P::descr,"mBBcr"}}, {P::descr},  {},  {"Zhf", "Zclbl", "Zl", "Whf", "Wcl", "Wl"}} });
  
  //m_histoSysts.insert({ "SysVjetsMVH" , SysConfig{ T::shapeonly, S::noSmooth, {"Wjets", "Zjets"}, {{P::descr,"SR"}, {P::descr,"mBBcr"}}, {},  {},  {"Zhf", "Zclbl", "Zl", "Whf", "Wcl", "Wl"}} }); // shapeonly implementation for alternative V+jets syst. scheme
  // TODO: PUT BACK WITH NUTPLES AVAILABLE
  m_histoSysts.insert({ "SysVHQCDscalePTV" , noSmoothConfig});
  m_histoSysts.insert({ "SysVHQCDscaleMbb" , noSmoothConfig});
  m_histoSysts.insert({ "SysVHPDFPTV" , noSmoothConfig});
  m_histoSysts.insert({ "SysVHQCDscalePTV_ggZH" , noSmoothConfig});
  m_histoSysts.insert({ "SysVHQCDscaleMbb_ggZH" , noSmoothConfig});
  m_histoSysts.insert({ "SysVHPDFPTV_ggZH" , noSmoothConfig});
  m_histoSysts.insert({ "SysVHUEPSPTV" , noSmoothConfig});
  m_histoSysts.insert({ "SysVHUEPSMbb" , noSmoothConfig});
  m_histoSysts.insert({ "SysVHNLOEWK" , noSmoothConfig});

  m_histoSysts.insert({ "SysStoptPTV" , noSmoothConfig}); 
  m_histoSysts.insert({ "SysStoptMBB" , noSmoothConfig});
  m_histoSysts.insert({ "SysStopWtPTV" , noSmoothConfig});
  m_histoSysts.insert({ "SysStopWtMBB" , noSmoothConfig});
  //m_histoSysts.insert({ "SysWtChanPythiaHerwig" , noSmoothConfig});
  //m_histoSysts.insert({ "SysWtChanAcerMC" , noSmoothConfig});
  //m_histoSysts.insert({ "SysTopPt" , noSmoothConfig});
  //m_histoSysts.insert({ "SysTTbarPt" , noSmoothConfig});

  //m_histoSysts.insert({ "SysMVH_ME", SysConfig{T::shapeonly, S::noSmooth, "ttbar", {}, {{P::nLep}}} });
  //m_histoSysts.insert({ "SysMVH_radiation", SysConfig{T::shapeonly, S::noSmooth, "ttbar", {}, {{P::nLep}}} });  
  
  //m_histoSysts.insert({ "SysTtbarMetCont" , noSmoothConfig});
  //m_histoSysts.insert({ "SysTtbarMBBCont" , noSmoothConfig});
    m_histoSysts.insert({ "SysMJ_El_METstr" , smoothConfig});//PF
    m_histoSysts.insert({ "SysMJ_Mu_METstr" , smoothConfig});//PF
    m_histoSysts.insert({ "SysMJ_El_EWK" , SysConfig{ T::shapeonly, S::smooth }});
    //m_histoSysts.insert({ "SysMJ_El_EWK" , smoothConfig});//SysConfig{ T::shapeonly, S::smooth }});
    m_histoSysts.insert({ "SysMJ_El_flavor" , smoothConfig});//PF

 
  /////////////////////////////////////////////////////////////////////////////////////////
  //
  //                          Tweaks, if needed
  //
  /////////////////////////////////////////////////////////////////////////////////////////

  // later can add some switches, e.g looping through m_histoSysts and
  // putting to T::skip all the JES NP

}
