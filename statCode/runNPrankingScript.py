#!/usr/bin/env python

import os,sys

def runNPrankingMacro(ws, mass):
    #print "root -l -q -b 'macros/runBreakdown.C(\"workspaces/{0}/combined/{1}.root\",\"combined\",\"ModelConfig\",\"obsData\",\"SigXsecOverSM\",\"config/breakdown.xml\",\"add\",\"total\", 0.005, 0.0, \"{0}\", \"DEBUG\")'".format(ws, mass)
    os.system("python scripts/runNPranking.py {0} {1}".format(ws, mass))

    os.system("root -l -q -b 'macros/runBreakdown.C(\"workspaces/{0}/combined/{1}.root\",\"combined\",\"ModelConfig\",\"obsData\",\"SigXsecOverSM\",\"config/breakdown.xml\",\"add\",\"total\", 0.005, 0.0, \"{0}\", \"DEBUG\")'".format(ws, mass))
    #os.system("root -l -q -b 'macros/runBreakdown.C(\"workspaces/{0}/combined/{1}.root\",\"combined\",\"ModelConfig\",\"asimovData\",\"SigXsecOverSM\",\"config/breakdown.xml\",\"add\",\"total\", 0.005, 0.0, \"{0}\", \"DEBUG\")'".format(ws, mass))
    
    os.system("python scripts/makeNPrankPlots.py {0} {1}".format(ws, mass))
    
    return

################################################################
#####                        MAIN CODE                    ######
################################################################
# Sept 27, conf note inputs
runNPrankingMacro("90013.290916-hiMass_npRankingPlots-x1500_HH_13TeV_290916-hiMass_npRankingPlots-x1500_Systs_1500","1500");
runNPrankingMacro("90014.290916-hiMass_npRankingPlots-x1800_HH_13TeV_290916-hiMass_npRankingPlots-x1800_Systs_1800","1800");

# Sept 27, conf note inputs
#runNPrankingMacro("80005.260916-v3-observedPullPlots-x600_HH_13TeV_260916-v3-observedPullPlots-x600_Systs_600","600");
#runNPrankingMacro("80009.260916-v3-observedPullPlots-x1000_HH_13TeV_260916-v3-observedPullPlots-x1000_Systs_1000","1000");
#runNPrankingMacro("90014.260916-v3-observedPullPlots-x2000_HH_13TeV_260916-v3-observedPullPlots-x2000_Systs_2000","2000");
#runNPrankingMacro("90017.260916-v3-observedPullPlots-x3000_HH_13TeV_260916-v3-observedPullPlots-x3000_Systs_3000","3000");

# Sept 26 Unblinded results
#runNPrankingMacro("80005.260916-v1-opt700allMasses_fullRange-x600_HH_13TeV_260916-v1-opt700allMasses_fullRange-x600_Systs_600","600");
#runNPrankingMacro("80011.260916-v1-opt700allMasses_fullRange-x1300_HH_13TeV_260916-v1-opt700allMasses_fullRange-x1300_Systs_1300","1300");
#runNPrankingMacro("90014.260916-v2-opt2000allMasses_fullRange-x2000_HH_13TeV_260916-v2-opt2000allMasses_fullRange-x2000_Systs_2000","2000");
#runNPrankingMacro("90017.260916-v2-opt2000allMasses_fullRange-x3000_HH_13TeV_260916-v2-opt2000allMasses_fullRange-x3000_Systs_3000","3000");

# Sept 22 Unblinded results
#runNPrankingMacro("90017.200916-v0-unblindedResults-x3000_HH_13TeV_200916-v0-unblindedResults-x3000_Systs_3000","3000");

# Sept 19 Higgs Plenary (50% unc for single top independent in SR and CR)
#runNPrankingMacro("80005.190916-v12-runNPranking_higgsPlenary_lowMass-x600_HH_13TeV_190916-v12-runNPranking_higgsPlenary_lowMass-x600_Systs_600","600");
#runNPrankingMacro("80007.190916-v12-runNPranking_higgsPlenary_lowMass-x750_HH_13TeV_190916-v12-runNPranking_higgsPlenary_lowMass-x750_Systs_750","750");
#runNPrankingMacro("90011.190916-v12-runNPranking_higgsPlenary_hiMas-x1300_HH_13TeV_190916-v12-runNPranking_higgsPlenary_hiMas-x1300_Systs_1300","1300");
#runNPrankingMacro("90014.190916-v12-runNPranking_higgsPlenary_hiMas-x2000_HH_13TeV_190916-v12-runNPranking_higgsPlenary_hiMas-x2000_Systs_2000","2000");

# Sept 15 HBSM presentation + 50% unc for single top
#runNPrankingMacro("80005.150916-fullLimits_normalSelections_50percentSingleTopUnc-x600_HH_13TeV_150916-fullLimits_normalSelections_50percentSingleTopUnc-x600_Systs_600","600");
#runNPrankingMacro("80007.150916-fullLimits_normalSelections_50percentSingleTopUnc-x750_HH_13TeV_150916-fullLimits_normalSelections_50percentSingleTopUnc-x750_Systs_750","750");
#runNPrankingMacro("90011.150916-fullLimits_normalSelections_50percentSingleTopUnc-x1300_HH_13TeV_150916-fullLimits_normalSelections_50percentSingleTopUnc-x1300_Systs_1300","1300");
#runNPrankingMacro("90014.150916-fullLimits_normalSelections_50percentSingleTopUnc-x2000_HH_13TeV_150916-fullLimits_normalSelections_50percentSingleTopUnc-x2000_Systs_2000","2000");

# Sept 14 (HBSM presentation)
#runNPrankingMacro("80005.140916-v0-lowMassRankingAndPulls-x600_HH_13TeV_140916-v0-lowMassRankingAndPulls-x600_Systs_600","600");
#runNPrankingMacro("80007.140916-v0-lowMassRankingAndPulls-x750_HH_13TeV_140916-v0-lowMassRankingAndPulls-x750_Systs_750","750");
#runNPrankingMacro("90011.140916-v0-hiMassRankingAndPulls-x1300_HH_13TeV_140916-v0-hiMassRankingAndPulls-x1300_Systs_1300","1300");
#runNPrankingMacro("90014.140916-v0-hiMassRankingAndPulls-x2000_HH_13TeV_140916-v0-hiMassRankingAndPulls-x2000_Systs_2000","2000");

# Sept 12 (Suyog numbers, both radHi and radLo from ratio calc)
#runNPrankingMacro("80006.120916-v6-npRanking-x700_HH_13TeV_120916-v6-npRanking-x700_Systs_700","700")
#runNPrankingMacro("80007.120916-v6-npRanking-x750_HH_13TeV_120916-v6-npRanking-x750_Systs_750","750")
#runNPrankingMacro("90011.120916-v6-npRanking-x1300_HH_13TeV_120916-v6-npRanking-x1300_Systs_1300","1300")
#runNPrankingMacro("90014.120916-v6-npRanking-x2000_HH_13TeV_120916-v6-npRanking-x2000_Systs_2000","2000")

# Sept 8 (dropping radLo, presentation)
#runNPrankingMacro("80006.080916-v7-runNPranking_lowMass-x700_HH_13TeV_080916-v7-runNPranking_lowMass-x700_Systs_700","700")
#runNPrankingMacro("80007.080916-v7-runNPranking_lowMass-x750_HH_13TeV_080916-v7-runNPranking_lowMass-x750_Systs_750","750")
#runNPrankingMacro("90011.080916-v7-runNPranking_hiMass-x1300_HH_13TeV_080916-v7-runNPranking_hiMass-x1300_Systs_1300","1300")
#runNPrankingMacro("90014.080916-v7-runNPranking_hiMass-x2000_HH_13TeV_080916-v7-runNPranking_hiMass-x2000_Systs_2000","2000")

# Sept 8 (ttbar modeling from histos)
#runNPrankingMacro("80006.080916-v1-ttbarModelingFromHisto_npRanking-x700_HH_13TeV_080916-v1-ttbarModelingFromHisto_npRanking-x700_Systs_700","700")
#runNPrankingMacro("80007.080916-v1-ttbarModelingFromHisto_npRanking-x750_HH_13TeV_080916-v1-ttbarModelingFromHisto_npRanking-x750_Systs_750","750")
#runNPrankingMacro("90011.080916-v1-ttbarModelingFromHisto_npRanking-x1300_HH_13TeV_080916-v1-ttbarModelingFromHisto_npRanking-x1300_Systs_1300","1300")
#runNPrankingMacro("90014.080916-v1-ttbarModelingFromHisto_npRanking-x2000_HH_13TeV_080916-v1-ttbarModelingFromHisto_npRanking-x2000_Systs_2000","2000")

# Sept 7
#runNPrankingMacro("80006.070916-v2-lowMass_dataInCR_unscaledPseudodataInSR_forTable-x700_HH_13TeV_070916-v2-lowMass_dataInCR_unscaledPseudodataInSR_forTable-x700_Systs_700","700")
#runNPrankingMacro("80007.070916-v2-lowMass_dataInCR_unscaledPseudodataInSR_forTable-x750_HH_13TeV_070916-v2-lowMass_dataInCR_unscaledPseudodataInSR_forTable-x750_Systs_750","750")
#runNPrankingMacro("90011.070916-v3-highMass_dataInCR_unscaledPseudodataInSR_forTable-x1300_HH_13TeV_070916-v3-highMass_dataInCR_unscaledPseudodataInSR_forTable-x1300_Systs_1300","1300")
#runNPrankingMacro("90014.070916-v3-highMass_dataInCR_unscaledPseudodataInSR_forTable-x2000_HH_13TeV_070916-v3-highMass_dataInCR_unscaledPseudodataInSR_forTable-x2000_Systs_2000","2000")

# Sept 6
#runNPrankingMacro("80006.lowMass_obs_14percentTTbarPSandME_npRanking-x700_HH_13TeV_lowMass_obs_14percentTTbarPSandME_npRanking-x700_Systs_700","700")
#runNPrankingMacro("80007.lowMass_obs_14percentTTbarPSandME_npRanking-x750_HH_13TeV_lowMass_obs_14percentTTbarPSandME_npRanking-x750_Systs_750","750")
#runNPrankingMacro("90011.highMass_obs_14percentTTbarPSandME_npRanking-x1300_HH_13TeV_highMass_obs_14percentTTbarPSandME_npRanking-x1300_Systs_1300","1300")
#runNPrankingMacro("90014.highMass_obs_14percentTTbarPSandME_npRanking-x2000_HH_13TeV_highMass_obs_14percentTTbarPSandME_npRanking-x2000_Systs_2000","2000")

# Aug 31
#runNPrankingMacro("80006.310816-v8-noFloatTTbar_applyNFbyHand_fitSRonly-x700_HH_13TeV_310816-v8-noFloatTTbar_applyNFbyHand_fitSRonly-x700_Systs_700","700")
#runNPrankingMacro("80006.310816-v7-noFloatTTbar_fitSRandCR_fccs027-x700_HH_13TeV_310816-v7-noFloatTTbar_fitSRandCR_fccs027-x700_Systs_700","700")
#runNPrankingMacro("80006.310816-v6-floatSRandCR_allObjSRandCR_allModSRandCR_ttbarPSandMEfromAlpha_fccs027-x700_HH_13TeV_310816-v6-floatSRandCR_allObjSRandCR_allModSRandCR_ttbarPSandMEfromAlpha_fccs027-x700_Systs_700","700")

# Aug 24
#runNPrankingMacro("90011.240816-v16-floatCR_objSRandCR_modSRandCR_exp-x1300_HH_13TeV_240816-v16-floatCR_objSRandCR_modSRandCR_exp-x1300_Systs_1300","1300")
#runNPrankingMacro("80006.240816-v21-floatCR_objSR_modSR_ttbarPEandMSfromHisto-x700_HH_13TeV_240816-v21-floatCR_objSR_modSR_ttbarPEandMSfromHisto-x700_Systs_700","700")
#runNPrankingMacro("80006.240816-v20-floatCR_objSR_modSRandCR_ttbarPEandMSfromHisto-x700_HH_13TeV_240816-v20-floatCR_objSR_modSRandCR_ttbarPEandMSfromHisto-x700_Systs_700","700")
#runNPrankingMacro("90011.240816-v20-floatCR_objSR_modSRandCR_ttbarPEandMSfromHisto-x1300_HH_13TeV_240816-v20-floatCR_objSR_modSRandCR_ttbarPEandMSfromHisto-x1300_Systs_1300","1300")

#runNPrankingMacro("80006.230816-v4-floatCR_allObjModSR_TTbarPEandMSfromHisto-x700_HH_13TeV_230816-v4-floatCR_allObjModSR_TTbarPEandMSfromHisto-x700_Systs_700","700")
#runNPrankingMacro("80006.220816-v8-floatCR_allModSR_allObjSR_AnalysisHHopts-x700_HH_13TeV_220816-v8-floatCR_allModSR_allObjSR_AnalysisHHopts-x700_Systs_700","700")
