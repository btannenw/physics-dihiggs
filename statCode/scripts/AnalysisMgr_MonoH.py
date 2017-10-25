from AnalysisMgr import WorkspaceConfig

class WorkspaceConfig_MonoH(WorkspaceConfig):
    def __init__(self, *args, **kwargs):
        kwargs["Analysis"] = "MonoH"
        super(WorkspaceConfig_MonoH, self).__init__(*args, **kwargs)
        return

    def set_regions(self, merged_only=False, inclusive=True, one_lepton_cr=True, two_lepton_cr=True, top_emu_cr=True, sam=False):
        if sam:
            self["Regions"] = ["13TeV_ZeroLepton_CUT_0ptag0pjet_0ptv_preselection_METFatJetLeadTranMass"]
            self.check_regions()
            return

        if merged_only:
             self["Regions"] = ["13TeV_ZeroLepton_CUT_1ptag1pfatjet_500ptv_SR_mBB"]
        elif inclusive:
            self["Regions"] = [
                "13TeV_ZeroLepton_CUT_0tag2pjet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag2pjet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag2pjet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag2pjet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag2pjet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag2pjet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag2pjet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag2pjet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag2pjet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag1pfat0pjet_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag1pfat0pjet_500ptv_SR_mBB",                
                ]

            if two_lepton_cr:
                self["Regions"] += [
                    "13TeV_TwoLepton_CUT_0tag2pjet_150_200ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_0tag2pjet_200_350ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_0tag2pjet_350_500ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_1tag2pjet_150_200ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_1tag2pjet_200_350ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_1tag2pjet_350_500ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_2tag2pjet_150_200ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_2tag2pjet_200_350ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_2tag2pjet_350_500ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_1tag1pfat0pjet_500ptv_SR_mBB",
                    "13TeV_TwoLepton_CUT_2tag1pfat0pjet_500ptv_SR_mBB"
                    ]
            if top_emu_cr:
                self["Regions"] += [                
                    #"13TeV_TwoLepton_CUT_1tag2pjet_150_200ptv_SR_topemucr_mBB",
                    #"13TeV_TwoLepton_CUT_1tag2pjet_200_350ptv_SR_topemucr_mBB",
                    #"13TeV_TwoLepton_CUT_1tag2pjet_350_500ptv_SR_topemucr_mBB",
                    "13TeV_TwoLepton_CUT_2tag2pjet_150_200ptv_SR_topemucr_mBB",
                    "13TeV_TwoLepton_CUT_2tag2pjet_200_350ptv_SR_topemucr_mBB",
                    "13TeV_TwoLepton_CUT_2tag2pjet_350_500ptv_SR_topemucr_mBB",
                    #"13TeV_TwoLepton_CUT_1tag1pfat0pjet_500ptv_SR_topemucr_mBB",
                    "13TeV_TwoLepton_CUT_2tag1pfat0pjet_500ptv_SR_topemucr_mBB",                
                    ]
            if one_lepton_cr:
                self["Regions"] += [
                    "13TeV_OneLepton_CUT_0tag2pjet_150_200ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_0tag2pjet_200_350ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_0tag2pjet_350_500ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_1tag2pjet_150_200ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_1tag2pjet_200_350ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_1tag2pjet_350_500ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_2tag2pjet_150_200ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_2tag2pjet_200_350ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_2tag2pjet_350_500ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_1tag1pfat0pjet_500ptv_SR_mBB",
                    "13TeV_OneLepton_CUT_2tag1pfat0pjet_500ptv_SR_mBB"
                    ]
            
        else:
            self["Regions"] = [
                "13TeV_ZeroLepton_CUT_0tag2jet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag2jet_150ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag2jet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag2jet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag3pjet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag3pjet_150ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag3pjet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag3pjet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag2jet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag2jet_150ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag2jet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag2jet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag3pjet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag3pjet_150ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag3pjet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag3pjet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag2jet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag2jet_150ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag2jet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag2jet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag3pjet_150_200ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag3pjet_150ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag3pjet_200_350ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag3pjet_350_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_0tag1pfat0pjet_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_1tag1pfat0pjet_500ptv_SR_mBB",
                "13TeV_ZeroLepton_CUT_2tag1pfat0pjet_500ptv_SR_mBB"
                ]

            if one_lepton_cr:
                pass

            if two_lepton_cr:            
                pass

        
        self.check_regions()
