from AnalysisMgr import WorkspaceConfig

class WorkspaceConfig_HH(WorkspaceConfig):
    def __init__(self, *args, **kwargs):
        #kwargs["Analysis"] = "HH"
        kwargs["Analysis"] = "HHWWbb" # BBT, May 17 2016
        super(WorkspaceConfig_HH, self).__init__(*args, **kwargs)
        return

    def set_regions(self):
        self["Regions"] = [
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_mBBcr_MET",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_mBBcr_bbPt",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_mBBcr_bbMass",

            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_mBBcr_Xhh700_bbMass",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SR_Xhh700_bbMass",

            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_mBBcr_Xhh2000_bbMass",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SR_Xhh2000_bbMass",

            "13TeV_OneLepton_CUT_2tag4pjet_0ptv_mBBcr_bbMass",
            "13TeV_OneLepton_CUT_2tag4pjet_0ptv_SR_bbMass",

            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_mBBcr_LepEta",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SR_LepEta",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRwithLH_LepEta",

            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRpre_LepEta",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRwithLH_LepEta"           
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRaddLH_LogLikelihood_ttbar",
            
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan1_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan2_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan3_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan4_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan5_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan6_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan7_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan8_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan9_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan10_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan11_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan12_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan13_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan14_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan15_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan16_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan17_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan18_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan19_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan20_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan21_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan22_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan23_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan24_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan25_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan26_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan27_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan28_LogLikelihood_ttbar",
            #"13TeV_OneLepton_CUT_2tag4pjet_0ptv_SRscan29_LogLikelihood_ttbar",
            
            #"13TeV_TauLH_CUT_2tag2pjet_0ptv_SR_mHH" 
            # BBT , select histograms to fit
            ]
        
        self.check_regions()
