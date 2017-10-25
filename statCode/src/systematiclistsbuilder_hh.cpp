#include "systematiclistsbuilder_hh.hpp"

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

void SystematicListsBuilder_HH::fillHistoSystsRenaming() {
}

void SystematicListsBuilder_HH::listAllUserSystematics(const RegionTracker& regTrk, bool useFltNorms){
  using P = Property;

  //normFact("All", {{"MC", "DataDriven"}}, (10.0/3.2), 0, 10, true);
  std::vector<TString> sig_decorr = m_config.getStringList("DecorrPOI");

  TString type = m_config.getValue("Type", "RSG");
  if (type.Contains("SM")) {
    normFact("SigScale", {"Sig"}, 1000, 0, 2000, true);  // to improve fit convergence
  }

  // list them all !
  if(sig_decorr.size() != 0) { // TODO or just extend decorrSysForCategories for the POIs ?
    std::vector<Property> decorrelations;
    for(auto& d : sig_decorr) {
      decorrelations.push_back(Properties::props_from_names.at(d));
    }
    addPOI("SigXsecOverSM", {"Sig", {}, std::move(decorrelations)}, 1, -40, 40);
  }
  else {
    addPOI("SigXsecOverSM", {"Sig"}, 1, -40, 40);
  }

  //return;  // HACK to allow scaling of lumi only for stat limits

  // Syst norm yield: get from Nicolina
  //normSys("SysTheorySig", 0.20, {"Sig"});


  // ttbar ...

  // Cross-section
  bool fltTop = ((regTrk["has1tag"] || regTrk["has2tag"]) && regTrk["topcr"]);
  if(useFltNorms && fltTop) {
    normFact("ttbar", {"ttbar"});
  } else {
    sampleNormSys("ttbar", 0.06);

    // Generate acceptance variations for mMMC or mHH (norm only)
    if (m_config.getValue("Type", "RSG") == "SM") {
      normSys("SysTtbarAcc0Tag", 1-0.135, 1+0.084, {"ttbar", {{P::nTag, 0}}});
      normSys("SysTtbarAcc1Tag", 1-0.257, 1+0.212, {"ttbar", {{P::nTag, 1}}});
      normSys("SysTtbarAcc2Tag", 1-0.223, 1+0.196, {"ttbar", {{P::nTag, 2}}}); 
    } else {
      normSys("SysTtbarAcc0Tag", 1-0.178, 1+0.093, {"ttbar", {{P::nTag, 0}}});
      normSys("SysTtbarAcc1Tag", 1-0.197, 1+0.169, {"ttbar", {{P::nTag, 1}}});
      normSys("SysTtbarAcc2Tag", 1-0.286, 1+0.233, {"ttbar", {{P::nTag, 2}}});
    }
  }

  // single top .... 

  // cross-sections from Top MC Twiki: https://twiki.cern.ch/twiki/bin/view/LHCPhysics/SingleTopRefXsec
  sampleNormSys("stops", 0.037);
  sampleNormSys("stopt", 0.042);
  sampleNormSys("stopWt", 0.054);

  // From VH for extra jets, do we need?
//   normSys("SysstoptAcc", 0.16, {"stopt", {{P::nJet, 2}} });//PF
//   normSys("SysstoptAcc", 0.19, {"stopt", {{P::nJet, 3}} });//PF
//   normSys("SysstopWtAcc", 0.25, {"stopWt", {{P::nJet, 2}} });//PF
//   normSys("SysstopWtAcc", 0.32, {"stopWt", {{P::nJet, 3}} });//PF

  // V+jets ....
  // TODO: need to simpify and decide what we can take and if we need separate HF norms for all
  bool floatHFOnlyForW = m_config.getValue("FloatHFOnlyForW", false);

  // Wl
  if(useFltNorms && regTrk["has0tag"] && (regTrk["hasOneLep"]) && !floatHFOnlyForW){ // || regTrk["hasZeroLep"])) {
    normFact("Wl", {"Wl"});
  }
  else {
    sampleNormSys("Wl", 0.32);
  }

  bool floatHFOnlyForZ = m_config.getValue("FloatHFOnlyForZ", false);

  // Zl
  if(useFltNorms && regTrk["has0tag"] && (regTrk["hasZeroLep"] || regTrk["hasTwoLep"]) && !floatHFOnlyForZ) {
    normFact("Zl", {"Zl"});

  }
  else {
    sampleNormSys("Zl", 0.18);
  }

  // Wcl and Zcl
  if(useFltNorms && regTrk["has1tag"] && (regTrk["hasOneLep"]) && !floatHFOnlyForW) {
    normFact("Wcl", {"Wcl"});
  }
  else {
    sampleNormSys("Wcl", 0.37);
  }
  if(useFltNorms && regTrk["has1tag"] && (regTrk["hasTwoLep"]) && !floatHFOnlyForZ) {
    normFact("Zcl", {"Zcl"});
  }
  else {
    sampleNormSys("Zcl", 0.23);
  }

  // floating Wbb+Wbc+Wbl+Wcc - put extra systs on Wbl and Wcc to allow ratios to change
//   if(useFltNorms && regTrk["has2tag"] && (regTrk["hasOneLep"])) {
//     normFact("Wbb", {{"Wbb", "Wbc", "Wbl", "Wcc"}});
//   }
//   else {
    normSys("SysWbbNorm", 0.33, {{"Wbb", "Wbc", "Wbl", "Wcc"}});
    //  }

  // Flavour composition
  normSys("SysWblWbbRatio", 0.35, {"Wbl", {}, {}, {{P::bin, 0}, {P::bin, 1}}});
  normSys("SysWccWbbRatio", 0.12, {"Wcc"});
  normSys("SysWbcWbbRatio", 0.12, {"Wbc"});

  // floating Zbb+Zbc+Zbl+Zcc - put extra systs on Zbl and Zcc to allow ratios to change
  if(useFltNorms && regTrk["has2tag"] && (regTrk["hasTwoLep"])){ // || regTrk["hasZeroLep"])) {
    normFact("Zbb", {{"Zbb", "Zbc", "Zbl", "Zcc"}});

  }
  else {
    normSys("SysZbbNorm", 0.48, {{"Zbb", "Zbc", "Zbl", "Zcc"}});
  }

  // Flavour composition
  normSys("SysZblZbbRatio", 0.12, {"Zbl", {}, {P::nJet}});
  normSys("SysZccZbbRatio", 0.12, {"Zcc"});
  normSys("SysZbcZbbRatio", 0.12, {"Zbc"});


  // Fakes ...
  // Conservative 50% for now.  TODO: estimate
  sampleNormSys("Fakes", 0.5);

  // Diboson ...
  
  // Cross section from A/H->tautau and H+->taunu
  sampleNormSys("diboson", 0.06);

  // SM Higgs ...

  //add Higgs normalisation systematic of 50% when running resonance fits
  normSys("SysHiggsNorm", 0.50, {"Higgs"});


  // multijet
  // TODO

  normSys("ATLAS_LUMI_2015_2016", 0.029, {"MC", {{P::year, 2015}}});

}


void SystematicListsBuilder_HH::listAllHistoSystematics(const RegionTracker& regTrk) {
  using T = SysConfig::Treat;
  using S = SysConfig::Smooth;
  using P = Property;
  bool yes(true);     bool no(false); // correlate years ?

  SysConfig noSmoothConfig { T::shape, S::noSmooth };
  SysConfig smoothConfig { T::shape, S::smooth };
  SysConfig skipConfig { T::skip, S::noSmooth };

  m_histoSysts.insert({ "SysMUONS_SCALE" , smoothConfig});
  m_histoSysts.insert({ "SysMUONS_MS" , smoothConfig});
  m_histoSysts.insert({ "SysMUONS_ID" , smoothConfig});

  m_histoSysts.insert({ "SysMUON_TTVA_SYS" , skipConfig});
  m_histoSysts.insert({ "SysMUON_TTVA_STAT" , skipConfig});
  m_histoSysts.insert({ "SysMUON_EFF_SYS" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_EFF_STAT" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_EFF_TrigSystUncertainty" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_EFF_TrigStatUncertainty" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_ISO_SYS" , noSmoothConfig});
  m_histoSysts.insert({ "SysMUON_ISO_STAT" , noSmoothConfig});
  
  m_histoSysts.insert({ "SysEG_SCALE_ALL" , smoothConfig});
  m_histoSysts.insert({ "SysEG_RESOLUTION_ALL" , smoothConfig});

  m_histoSysts.insert({ "SysEL_EFF_Reco_TotalCorrUncertainty" , skipConfig});
  m_histoSysts.insert({ "SysEL_EFF_ID_TotalCorrUncertainty" , skipConfig});
  m_histoSysts.insert({ "SysEL_EFF_Iso_TotalCorrUncertainty" , skipConfig});
  m_histoSysts.insert({ "SysEL_EFF_Trigger_TotalCorrUncertainty" , skipConfig});

  m_histoSysts.insert({ "SysEL_EFF_Reco_TOTAL_1NPCOR_PLUS_UNCOR" , noSmoothConfig});
  m_histoSysts.insert({ "SysEL_EFF_ID_TOTAL_1NPCOR_PLUS_UNCOR" , noSmoothConfig});
  m_histoSysts.insert({ "SysEL_EFF_Trigger_TOTAL_1NPCOR_PLUS_UNCOR" , noSmoothConfig});
  m_histoSysts.insert({ "SysEL_EFF_Iso_TOTAL_1NPCOR_PLUS_UNCOR" , noSmoothConfig});

  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_SME_TES_DETECTOR",  smoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_SME_TES_DETECTOR",  smoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_SME_TES_INSITU",   smoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_SME_TES_INSITU", smoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_SME_TES_MODEL",   smoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_SME_TES_MODEL", smoothConfig});   
  m_histoSysts.insert({ "SysANTITAU_BDT_CUT",   smoothConfig});
  
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_ELEOLR_TOTAL", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_ELEOLR_TOTAL", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_JETID_TOTAL", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_JETID_TOTAL", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_JETID_HIGHPT", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_JETID_HIGHPT", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_RECO_TOTAL", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_RECO_TOTAL", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_RECO_HIGHPT", noSmoothConfig});
  m_histoSysts.insert({ "SysTAUS_TRUEHADTAU_EFF_RECO_HIGHPT", noSmoothConfig});

  m_histoSysts.insert({ "SysJET_GroupedNP_1" , smoothConfig});
  m_histoSysts.insert({ "SysJET_GroupedNP_2" , smoothConfig});
  m_histoSysts.insert({ "SysJET_GroupedNP_3" , smoothConfig});

  m_histoSysts.insert({ "SysJET_JER_SINGLE_NP" , smoothConfig});

  m_histoSysts.insert({ "SysFT_EFF_extrapolation" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_extrapolation_from_charm" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_0" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_1" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_2" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_3" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_Light_4" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_0" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_1" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_B_2" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_C_0" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_C_1" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_C_2" , smoothConfig});
  m_histoSysts.insert({ "SysFT_EFF_Eigen_C_3" , smoothConfig});

  m_histoSysts.insert({ "SysMET_SoftTrk_Scale" , smoothConfig});
  m_histoSysts.insert({ "SysMET_JetTrk_Scale" , smoothConfig});
  m_histoSysts.insert({ "SysMET_SoftTrk_ResoPerp" , smoothConfig});
  m_histoSysts.insert({ "SysMET_SoftTrk_ResoPara" , smoothConfig});


  /////////////////////////////////////////////////////////////////////////////////////////
  //
  //                          Tweaks, if needed
  //
  /////////////////////////////////////////////////////////////////////////////////////////

  // later can add some switches, e.g looping through m_histom_histoSysts.emplace( "Systs and
  // putting to T::skip all the JES NP



}






