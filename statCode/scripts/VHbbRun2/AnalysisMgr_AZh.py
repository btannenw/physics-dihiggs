from AnalysisMgr import WorkspaceConfig

class WorkspaceConfig_AZh(WorkspaceConfig):
    def __init__(self, *args, **kwargs):
        kwargs["Analysis"] = "VHbbRun2"
        super(WorkspaceConfig_AZh, self).__init__(*args, **kwargs)
        self["Regions"] = []
        return

    def set_regions(self, var='mVH', merged=True, resolved=True, zero_lepton=True, one_lepton=True, two_lepton=True,
                   inclusiveResolved=False, splitLowHighCR=False):
        self["Regions"] = [ ]
        self.append_regions(var, merged, resolved, False, zero_lepton, one_lepton, two_lepton, inclusiveResolved, splitLowHighCR)

    def append_regions(self, var='mVH', merged=True, resolved=True, CRs=False, zero_lepton=True, one_lepton=True, two_lepton=True,
                   inclusiveResolved=False, splitLowHighCR=False):
        if zero_lepton:
            if merged:
                self["Regions"] += [
                    "13TeV_ZeroLepton_2tag1pfat0pjet_500ptv_SR_noaddbjetsr_" + var,
                    "13TeV_ZeroLepton_1tag1pfat0pjet_500ptv_SR_noaddbjetsr_" + var,
                    ]
            if resolved:
                tags = ["2"]
                jets = ["2","3"]
                #if inclusiveResolved:
                #    ptv = "150"
                #else:
                #    ptv = "150_500"
                ptvs = ["150_200","200" ]
                for t in tags:
                    for j in jets:
                        for p in ptvs:
                            # if t == "2" and d == "SR": continue
                            # if (t == "1" or t == "2") and d == "SR": continue
                            self["Regions"].append("13TeV_ZeroLepton_{0}tag{1}jet_{2}ptv_SR_{3}".format(t, j, p, var))
            if CRs:
                tags = ["2"]
                jets = ["2","3"]
                if splitLowHighCR:
                    descrs = ["highmBBcr", "lowmBBcr"]
                else:
                    descrs = ["mBBcr"]
                if inclusiveResolved:
                    ptv = "150"
                else:
                    ptv = "150_500"
                for t in tags:
                    for j in jets:
                        for d in descrs:
                            # if t == "2" and d == "SR": continue
                            # if (t == "1" or t == "2") and d == "SR": continue
                            self["Regions"].append("13TeV_ZeroLepton_{0}tag{1}jet_{2}ptv_{3}_{4}".format(t, j, ptv, d, var))

        if one_lepton:
            if merged:
                self["Regions"] += [
                    "13TeV_OneLepton_1tag1pfat0pjet_500ptv_0btrkjetunmatched_SR_{}".format(var),
                    ]
            if resolved:
                self["Regions"] += [
                    "13TeV_OneLepton_1tag2jet_150_500ptv_SR_{}".format(var),
                    ]
        if two_lepton:
            if merged:
                self["Regions"] += [
                    "13TeV_TwoLepton_1tag1pfat0pjet_500ptv_SR_{}".format(var),
                    "13TeV_TwoLepton_2tag1pfat0pjet_500ptv_SR_{}".format(var),
                    ]
            if resolved:
                tags = ["2", "1"]
                jets = ["2p"]
                if inclusiveResolved:
                    ptv = "0"
                else:
                    ptv = "0_500"
                for t in tags:
                    for j in jets:
                        # if t == "2" and d == "SR": continue
                        # if (t == "1" or t == "2") and d == "SR": continue
                        self["Regions"].append("13TeV_TwoLepton_{0}tag{1}jet_{2}ptv_SR_{3}".format(t, j, ptv, var))

            if CRs:
                tags = ["2", "1"]
                jets = ["2p"]
                if splitLowHighCR:
                    descrs = ["highmBBcr", "lowmBBcr", "topemucr"]
                else:
                    descrs = ["mBBcr", "topemucr"]
                if inclusiveResolved:
                    ptv = "0"
                else:
                    ptv = "0_500"
                for t in tags:
                    for j in jets:
                        for d in descrs:
                            self["Regions"].append("13TeV_TwoLepton_{0}tag{1}jet_{2}ptv_{3}_{4}".format(t, j, ptv, d, var))
        self.check_regions()
