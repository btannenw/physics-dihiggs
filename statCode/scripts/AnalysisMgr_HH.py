from AnalysisMgr import WorkspaceConfig

class WorkspaceConfig_HH(WorkspaceConfig):
    def __init__(self, *args, **kwargs):
        kwargs["Analysis"] = "HH"
        super(WorkspaceConfig_HH, self).__init__(*args, **kwargs)
        return

    def set_regions(self):
        self["Regions"] = [
            "13TeV_TauLH_CUT_2tag2pjet_0ptv_SR_METCent_mHH"
#             "13TeV_TwoTauLH_CUT_0tag2jet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_0tag2pjet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_0tag3pjet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_1tag2jet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_1tag2pjet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_1tag3pjet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_2tag2jet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_2tag2pjet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_2tag3pjet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_3ptag2pjet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_3ptag3pjet_0_500ptv_SR_mVH",
#             "13TeV_TwoTauLH_CUT_0tag1pfat0pjet_500ptv_SR_mVH"                
            ]
        
        self.check_regions()
