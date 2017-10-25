from AnalysisMgr import WorkspaceConfig

class WorkspaceConfig_Htautau(WorkspaceConfig):
    def __init__(self, *args, **kwargs):
        kwargs["Analysis"] = "Htautau"
        super(WorkspaceConfig_Htautau, self).__init__(*args, **kwargs)
        return

    def set_regions(self, inclusive=True):

        if inclusive:
            self["Regions"] = [
                "13TeV_OneLepton_CUT_5tag0jet_0ptv_ElHad1p_mvis2",
                "13TeV_OneLepton_CUT_5tag0jet_0ptv_ElHad3p_mvis2",
                "13TeV_OneLepton_CUT_5tag0jet_0ptv_MuHad1p_mvis2",
                "13TeV_OneLepton_CUT_5tag0jet_0ptv_MuHad3p_mvis2",
                ]
        else:
            self["Regions"] = [
                "13TeV_OneLepton_CUT_0tag0jet_0ptv_ElHad1p_mvis2",
                "13TeV_OneLepton_CUT_0tag0jet_0ptv_ElHad3p_mvis2",
                "13TeV_OneLepton_CUT_0tag0jet_0ptv_MuHad1p_mvis2",
                "13TeV_OneLepton_CUT_0tag0jet_0ptv_MuHad3p_mvis2",
                "13TeV_OneLepton_CUT_1tag0jet_0ptv_ElHad1p_mvis2",
                "13TeV_OneLepton_CUT_1tag0jet_0ptv_ElHad3p_mvis2",
                "13TeV_OneLepton_CUT_1tag0jet_0ptv_MuHad1p_mvis2",
                "13TeV_OneLepton_CUT_1tag0jet_0ptv_MuHad3p_mvis2",
                ]

        self.check_regions()
