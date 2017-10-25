import re
import logging
import ROOT
import plottingConfig as cfg

class Config(cfg.PlottingConfig):

    def __init__ (self, options):
        self.options = options
        super(Config, self).__init__()

        sigma = 1 # at mu=1 (arbitrary for AZh)
        sigma_units = 'fb'
        # self.force_mu = (True, 0.16) # 700 GeV
        self.force_mu = (True, 10) # 600 GeV

        # for child classes to use
        # self.loggingLvl = logging.INFO
        self.loggingLvl = logging.DEBUG
        self.verbose = False
        self.formats = [ 'eps', 'pdf', 'png', 'root', 'C' ]
        self.blind = True
        self.thresh_drop_legend = 0.01
        self.restrict_to = []
        self.excludes = []
        self.additionalPlots = []
        self.add_sig_to_ratio_plot = True
        self.use_exp_sig = True
        # self.transferResults_fitName = "HiggsNorm"
        # self.get_binning_hist_removal = ["_meas2l2q2v2q"]
        self.bkg_substr_name = "Diboson"
        self.bkg_substr_list = ["diboson", "Diboson", "WZ", "ZZ", "VZ"]
        self.file_tags = ["Y", "L", "J", "T", "TType", "Flv", "Sgn", "isMVA", "dist", "Spc", "D", "nAddTag", "BMax", "BMin", "Fat", "incFat", "incJet", "incAddTag"]
        self.weight_tags = ["Higgsweighted", "Dibosonweighted"]
        self.sig_names = ["VH"]
        self.signal = ["A#rightarrow Zh (best fit)", self._STACK, ROOT.kRed + 1, 1] # last = mult factor
        self.expected_signal = ["VHbb", self._STACK, ROOT.kRed +1, self.force_mu[1]] # last = expected mu
        #self.expected_signal = ["A#rightarrow Zh (#sigma={0} {1})".format(int(sigma*self.force_mu[1]), sigma_units), self._STACK, ROOT.kRed +1, self.force_mu[1]] # last = expected mu
        # self.additional_signal = ["A#rightarrow Zh", self._OVERPRINT, ROOT.kRed +1, 1.]
        self.bkg_tuple = {'ttbar': ("t#bar{t}", 42, ROOT.kOrange, []),
                'stopt': ("t, s+t chan", 41, ROOT.kOrange - 1, ["stops"]),
                'stops': ("t, s+t chan", 41, ROOT.kOrange - 1, ["stopt"]),
                'stopWt': ("Wt", 40, ROOT.kYellow - 7, []),
                'stop': ("Single top", 40, ROOT.kOrange - 1, []),
                'Zbb': ("Z+bb", 25, ROOT.kAzure + 3, []),
                'Zbc': ("Z+bc", 24, ROOT.kAzure + 2, []),
                'Zclbl': ("Z+(bl,cl)", 23, ROOT.kAzure + 1, []),
                'Zbl': ("Z+bl", 23, ROOT.kAzure + 1, []),
                'Zcl': ("Z+cl", 21, ROOT.kAzure - 8, []),
                'Zcc': ("Z+cc", 22, ROOT.kAzure - 4, []),
                'Zhf': ("Z+(bb,bc,cc)", 22, ROOT.kAzure + 2, []),
                'Zl': ("Z+l", 20, ROOT.kAzure - 9, []),
                'Wbl': ("W+bl", 33, ROOT.kGreen + 2, []),
                'Wbb': ("W+bb", 35, ROOT.kGreen + 4, []),
                'Wbc': ("W+bc", 34, ROOT.kGreen + 3, []),
                'Wcc': ("W+cc", 32, ROOT.kGreen + 1, []),
                'Whf': ("W+(bb,bc,cc,bl)", 32, ROOT.kGreen + 3, []),
                'Wcl': ("W+cl", 31, ROOT.kGreen - 6, []),
                'Wl': ("W+l", 30, ROOT.kGreen - 9, []),
                'WZ': ("WZ", 53, ROOT.kGray + 1, ["ZZ"]),
                'ZZ': ("ZZ", 52, ROOT.kGray + 1, ["WZ"]),
                'VZ': ("VZ", 51, ROOT.kGray + 1, []),
                'diboson': ("Diboson", 51, ROOT.kGray + 1, []),
                'WW': ("WW", 50, ROOT.kGray + 3, []),
                'Diboson': ("Diboson", 50, ROOT.kGray + 1, []),
                #'VH125': ("Vh", 49, ROOT.kRed - 6, []),
                'multijet': ("Multijet", 39, ROOT.kViolet-9, ["multijetMu", "multijetEl"]),
                'multijetEl': ("Multijet", 39, ROOT.kViolet-9, ["multijetMu", "multijet"]),
                'multijetMu': ("Multijet", 39, ROOT.kViolet-9, ["multijetEl", "multijet"])}
        # self.ATLAS_suffix = "Internal"
        # self.ATLAS_suffix = "Simulation"
        self.ATLAS_suffix = "Preliminary"
        # self.ATLAS_suffix = ""
        # for yields
        self.make_slides = False
        self.window = None
        self.priorities = {
        "data" : 80,
        "S/sqrt(S+B)" : 73,
        "S/B" : 72,
        "Bkg" : 60,
        "MC" : 75,
        "SignalExpected" : 71,
        "Signal" : 70,
        "VH125" : 57,
        "ZvvH125" : 67,
        "ggZvvH125" : 67,
        "qqZvvH125" : 67,
        "WlvH125" : 68,
        "ZllH125" : 69,
        "ggZllH125" : 69,
        "qqZllH125" : 69,
        "ZvvH150" : 67,
        "ggZvvH150" : 67,
        "qqZvvH150" : 67,
        "WlvH150" : 68,
        "ZllH150" : 69,
        "AZhllbb1200" : 70,
        "AZhvvbb1200" : 70,
        "AZhllbb1000" : 70,
        "AZhvvbb1000" : 70,
        "AZhllbb400" : 70,
        "AZhvvbb400" : 70,
        "AZhllbb300" : 70,
        "AZhvvbb300" : 70,
        "AZhllbb600" : 70,
        "AZhvvbb600" : 70,
        "bbAZhllbb600" : 70,
        "bbAZhvvbb600" : 70,
        "ggZllH150" : 69,
        "qqZllH150" : 69,
        "ttbar" : 45,
        "stops" : 43,
        "stopt" : 42,
        "stopst" : 41,
        "stopWt" : 40,
        "stop" : 40,
        "Zhf" : 27,
        "Zb" : 24,
        "Zbl" : 25,
        "Zbb" : 27,
        "Zbc" : 26,
        "Zc" : 21,
        "Zcl" : 100,
        "Zclbl" : 22,
        "Zcc" : 23,
        "Zl" : 20,
        "Whf" : 37,
        "Wb" : 34,
        "Wbl" : 35,
        "Wbb" : 37,
        "Wbc" : 36,
        "Wcc" : 33,
        "Wc" : 31,
        "Wcl" : 32,
        "Wl" : 30,
        "WZ" : 53,
        "ZZ" : 52,
        "VZ" : 51,
        "WW" : 50,
        "Diboson" : 50,
        "diboson" : 50,
        "multijet" : 45,
        "multijetEl" : 45,
        "multijetMu" : 45,
        "MJ0lep" : 45,
        "MJ1lep" : 45,
        "MJ2lep" : 45,
        "MJ2lepEl" : 45,
        "MJ1lepEl" : 45,
        "MJ1lepMu" : 45,
        }
        # for reduced diag plots only
        self.exclude_str = 'HiggsNorm'
        self.cov_classification = {
        "BTag": [False, ["SysFT_EFF_Eigen", "SysFT_EFF_extrapolation"], []],
        "Top": [False, ["SysWt", "SysTop", "SysTtbar", "SysMVH"], []],
        "ModelBoson": [False, ["SysVV", "SysWM","SysZM","SysWD","SysZD","SysWP","SysZP","SysVj"], []],
        "Norm": [False, ["Norm","Ratio"], []],
        "norm": [False, ["norm"], []],
        "Lepton": [False, ["SysMUON","SysEL","SysEG"], []],
        "Jet": [False, ["SysJET","FATJET"], []],
        "MET": [False, ["SysMET"], []],
        "LUMI": [False, ["LUMI"], []],
        "Shifted": [True, [], ["blablabla"]]
        }
        self.cov_special = {
        "noMCStat": [[], ["gamma"]],
        "JES": [["SigX", "norm_", "Jet"], []],
        "BTag": [["SigX", "norm_", "BTag"], []],
        "Mbb": [["SigX", "norm_", "Mbb"], []],
        "Modelling": [["SigX", "norm_", "Norm", "Ratio", "PtBi"], []],
        "SF": [["SigX", "norm_"], []],
        "Norm": [["3JNorm", "norm_", "Norm", "Ratio"], []]
        }
        self.syst_to_study = ["JetEResol", "Mbb_Whf", "V_Whf", "METScale", "TChanP",
        "ttbarHigh", "BJetReso", "ZblZbb", "BTagB1", "norm_Wbb", "WblWbbRatio"]
        self.suspicious_syst = ["norm_"]
        # for yield ratios only
        self.category_condenser = {
        # "_HistSyst": ["_Exp", False],
        # "_dist(mva|mjj)": ["_dist", False],
        # "_distMV1cBTag": ["_dist", False],
        "_distmV": ["_dist", False],
        # "_isMVA[01]": ["_isMVA", False],
        # "_B[0-5]_": ["_B9_", False],
        "_B(Max500_BMin0|BMin500)_": ["_Bresolvedmerged_", False],
        # "_TType(ll|mm|tt|xx)": ["_TType", False],
        "_T[012]": ["_Tx", False],
        "_(incJet1_J|incFat1_Fat|J)[1235]": ["_Jx", False],
        # "_Spc[0-9a-z]*top[a-z]*cr": ["_TType", False],
        # "(multijet)(.*_L)([0123])(.*)": [r'MJ\3lep\2\3\4', False],
        "_L[012]": ["_Lx", False],
        "_D(SR|topemucr)": ["_DallRegions", False],
        # "_W(bb|bl|bc|cc)_": ["_Whf_", True],
        # "_Z(bb|bl|bc|cc)_": ["_Zhf_", True]
        }

        logging.basicConfig(format='%(levelname)s in %(module)s: %(message)s', level=self.loggingLvl)

    def do_rebinning (self, prop):
        # NOTE: JWH - ED board requests
        if prop["dist"] == "mVH":
            if "mBBcr" in prop["D"] or "topemucr" in prop["D"]:
                if prop["L"] == "2" or prop["L"] == "0":
                    if prop.get("incFat", "-1") == "1" or prop.get("incJet", "-1") == "1":
                        return False
            if "SR" in prop["D"]:
                if prop["L"] == "2" or prop["L"] == "0":
                    if prop.get("incFat", "-1") == "1":
                        return False
                    if prop["L"] == "0":
                        return False
        return True

    def is_signal(self, compname):
        """ Check if a component is Higgs. If yes, return mass """
        # Spyros: Add ggA to list of signal names - has to be first in list otherwise we get problems
        signames = self.sig_names
        has_mass = False
        mass = ""
        # Spyros: if sg in compname matches also mVH so doesn't work for resonance analyses
        # remove mVH from compname
        compname = re.sub('mVH', '', compname)
        for sg in signames:
            if sg in compname:
                has_mass = True
                pos = compname.find(sg) + len(sg)
                mass = int(re.sub("[^0-9]", "", compname[pos:pos + compname[pos:].find('_')]))
                break
        return has_mass, mass

    def blind_data (self, setup):
        def _do_blinding (title):
            #return False, []
            return "T2" in title, [110, 140]


        do_blinding, blind_range = _do_blinding(setup.title)
        if do_blinding:
            # blind entire range
            if blind_range[0] == 0 and blind_range[1] == 0:
                blind_range[0] = setup.data.h.GetXaxis().GetXmin()
                blind_range[1] = setup.data.h.GetXaxis().GetXmax()
            setup.data.blind(blind_range[0], blind_range[1])
        #else:
        # # Add general blinding at 2% S/B
        # for i in range(1, setup.hsum.GetNbinsX()+1):
        #     if setup.hsum.GetBinContent(i) > 0:
        #         sob = setup.exp_sig.h.GetBinContent(i) / ( setup.hsum.GetBinContent(i) )
        #         if sob > 0.02:
        #             setup.data.blind(setup.hsum.GetBinLowEdge(i), setup.hsum.GetBinLowEdge(i+1))
        #     elif setup.exp_sig.h.GetBinContent(i) > 0:
        #         setup.data.blind(setup.hsum.GetBinLowEdge(i), setup.hsum.GetBinLowEdge(i+1))

    def preprocess_main_content_histogram (self, hist, setupMaker):
        return hist
        # def change_MeV_GeV(hist):
        #     if isinstance(hist, ROOT.TH1):
        #         new_hist = hist.Clone()
        #         bins = new_hist.GetXaxis().GetXbins()
        #         for i in range(bins.GetSize()):
        #             bins[i] /= 1000.
        #         new_hist.SetBins(bins.GetSize()-1, bins.GetArray())
        #         for i in range(new_hist.GetNbinsX()+2):
        #             new_hist.SetBinContent(i, hist.GetBinContent(i))
        #             new_hist.SetBinError(i, hist.GetBinError(i))
        #     elif isinstance(hist, ROOT.TGraph):
        #         new_hist = hist
        #         xbins = new_hist.GetX()
        #         for i in range(new_hist.GetN()):
        #             xbins[i] /= 1000.
        #         if isinstance(hist, ROOT.TGraphAsymmErrors):
        #             xbinsup = new_hist.GetEXhigh()
        #             xbinsdo = new_hist.GetEXlow()
        #             for i in range(new_hist.GetN()):
        #                 xbinsup[i] /= 1000.
        #                 xbinsdo[i] /= 1000.
        #     return new_hist
        #
        # new_hist = hist
        # props = sm.setup.properties
        # if props:
        #     # Changes for MeV/GeV
        #     affected_dists = ["MEff", "MEff3", "MET", "mLL", "mTW", "pTB1", "pTB2", "pTJ3", "pTV", "mBB", "mBBJ"]
        #     if props["L"] == "1" and props["dist"] in affected_dists:
        #         new_hist = change_MeV_GeV(hist)
        #
        # return new_hist

    def make_sum_plots (self, func):
        #add MET for 0 lepton merged+resolved signal region
        #add mBB for 0 mbbcr+SR
        for tag_i in ["1", "2"] :
                func("Region_BMax500_BMin0_incJet1_J2_T"+tag_i+"_L2_Y2015_distmBB_Dtopemucr",
                     rt=["_L2", "_T"+tag_i, "_distmBB", "_Dtopemucr"], ea=[])
                func("Region_BMax500_BMin0_incJet1_J2_T"+tag_i+"_L2_Y2015_distmBB",
                     rt=["_L2", "_T"+tag_i, "_distmBB"], ea=["_Dtopemucr"])
                func("Region_BMax500_BMin150_incJet1_J2_T"+tag_i+"_L0_Y2015_distmBB",
                     rt=["_L0", "_T"+tag_i, "_distmBB"], ea=[])
                func("Region_BMin150_T"+tag_i+"_L0_Y2015_distMET_DSR",
                     rt=["_L0","_T"+tag_i, "_distMET","_DSR"], ea=["_L2","_DmBBcr","_Dtopemucr"])
                func("Region_BMin0_T"+tag_i+"_L2_Y2015_distpTV_DSR",
                     rt=["_L2","_T"+tag_i, "_distpTV","_DSR"], ea=["_DmBBcr","_Dtopemucr"])

    def get_run_info (self):
        lumi = {}
        if self._year == "4023":
            lumi["2011"] = ["4.7", 7]
            lumi["2012"] = ["20.3", 8]
        if self._year == "2011":
            lumi["2011"] = ["4.7", 7]
        if self._year == "2012":
            lumi["2012"] = ["20.3", 8]
        if self._year == "2015":
            lumi["2015"] = ["3.2", 13]
        return lumi

    def get_title_height (self):
        return 3.5 if self._year == "4023" else 2

    def draw_category_ids (self, props, l, pos, nf):

        merged = False
        plural_jets = False
        nf += 0.25*nf # a bit more vertical spacing

        nleps = props.get("L", "-100")
        if nleps == '3':
            nleps = "0+1+2"
        njets = props.get("J", "-1")
        nincjets = props.get("incJet", "-1")
        if njets == "23":
            plural_jets = True
            njets = "2+3"
        elif nincjets == '1':
            plural_jets = True
            # njets += '+'
            njets = '#geq {}'.format(njets)
        elif int(njets) > 1:
            plural_jets = True
        nfatjets = props.get("Fat", "-1")
        nincfatjets = props.get("incFat", "-1")
        if int(nfatjets) > 0 and nincfatjets == '1':
            plural_jets = True
            merged = True
            # nfatjets += '+'
            nfatjets = '#geq {}'.format(nfatjets)
            # nfatjets += ' #leq'
        elif int(nfatjets) > 1:
            plural_jets = True
        ntags = props.get("T", "-100")

        region = ""
        if not nleps == '-100':
            if len(region) > 0:
                region += ', '
            region += "{} lep.".format(nleps)
        if not njets == '-1' or not nfatjets == '-1':
            if len(region) > 0:
                region += ', '
            region += "{} {}jet{}".format(nfatjets if merged else njets,
                                             "large-R " if merged else "",
                                             "s" if plural_jets else "")
        if not ntags == '-100':
            if len(region) > 0:
                region += ', '
            region += "{} tag{}".format(ntags,
                                        "s" if not int(ntags) == 1 else "")


        pTVBin = ""
        pTVmin = props.get("BMin", "-999")
        pTVmax = props.get("BMax", "-999")
        if not pTVmin == "-999" and pTVmax == "-999" and not pTVmin == "0":
            pTVBin = "{0} GeV #leq p_{{T}}^{{V}}".format(pTVmin)
        elif (pTVmin == "0" or pTVmin == "-999") and not pTVmax == "-999":
            pTVBin = "p_{{T}}^{{V}} < {0} GeV".format(pTVmax)
        elif not pTVmin == "-999" and not pTVmax == "-999":
            pTVBin = "{0} GeV #leq p_{{T}}^{{V}} < {1} GeV".format(pTVmin, pTVmax)

        signalControl = props.get("D", "")
        if not signalControl == "":
            def add_strings (base, addition):
                if base == "":
                    return addition
                else:
                    return base + ", " + addition
            temp = signalControl
            signalControl = ""
            reduce_SR_CR_mBB = props["dist"] == "pTV" or props["dist"] == "MET"
            if temp.find('SR') == 0:
                if reduce_SR_CR_mBB: signalControl = "m_{b#bar{b}} SR"
                elif merged: signalControl = add_strings(signalControl, "75 GeV #leq m_{b#bar{b}} < 145 GeV")
                else: signalControl = add_strings(signalControl, "110 GeV #leq m_{b#bar{b}} < 140 GeV")
                temp = temp[2:]
            if "highmBBcr" in temp:
                if reduce_SR_CR_mBB: signalControl = "m_{b#bar{b}} upper CR"
                elif merged: signalControl = add_strings(signalControl, "145 GeV #leq m_{b#bar{b}}")
                else: signalControl = add_strings(signalControl, "140 GeV #leq m_{b#bar{b}}")
                temp = temp.replace("highmBBcr", "")
            if "lowmBBcr" in temp:
                if reduce_SR_CR_mBB: signalControl = "m_{b#bar{b}} lower CR"
                elif merged: signalControl = add_strings(signalControl, "m_{b#bar{b}} < 75 GeV")
                else: signalControl = add_strings(signalControl, "m_{b#bar{b}} < 110 GeV")
                temp = temp.replace("lowmBBcr", "")
            if "mBBcr" in temp:
                if reduce_SR_CR_mBB: signalControl = "m_{b#bar{b}} CR"
                elif merged: signalControl = add_strings(signalControl, "m_{b#bar{b}} #leq 75 GeV, 145 GeV < m_{b#bar{b}}")
                else: signalControl = add_strings(signalControl, "m_{b#bar{b}} #leq 110 GeV, 140 GeV < m_{b#bar{b}}")
                temp = temp.replace("mBBcr", "")
            if "topemucr" in temp:
                signalControl = add_strings(signalControl, "e#mu")
                temp = temp.replace("topemucr", "")
            if "topaddbjetcr" in temp:
                signalControl = add_strings(signalControl, "+1 b-jet")
                temp = temp.replace("topaddbjetcr", "")



        pos_next = pos[1] - 0.1*nf # a bit more spacing

        l.DrawLatex(pos[0], pos_next, region)
        if not pTVBin == "":
            pos_next -= nf
            l.DrawLatex(pos[0], pos_next, pTVBin)
        if not signalControl == "":
            pos_next -= nf
            l.DrawLatex(pos[0], pos_next, signalControl)

        pos_next -= nf
        return (pos[0], pos_next)

    def force_mu_value (self):
        return self.force_mu

    def get_year_str (self):
        return self._year if int(self._year) < 2015 else ""

    def get_xbound_from_properties (self, prop):
        return (40, 400) if prop["dist"] == "pTB1" else None

    def get_legend_pos_from_properties (self, prop):
        result = None
        if prop["L"] == '0' and prop["dist"] == "VpT":
            result = [0.155, 0.13, 0.375, 0.65]
        if prop["dist"] == "dPhiVBB":
            result = [0.16, 0.16, 0.38, 0.68]
        return result

    def get_yscale_factor_from_properties (self, prop, logy):
        # if prop["dist"] == "MV1cB1" or prop["dist"] == "MV1cB2" or prop["dist"] == "MV1cBTag":
        #     if not logy: return 1.5
        # if prop["dist"] == "dPhiVBB" :
        #     if logy: return 5
        #     else : return 0.7
        # if prop["dist"] == "dPhiLBmin" :
        #     if not logy: return 1.3
        # if prop["dist"] == "mjj" :
        #     if not logy: return 1.1
        # if prop["dist"] == "dRBB" :
        #     if logy: return 500
        # if prop["dist"] == "MV1cBTag" :
        #     if not logy: return 0.75
        # if prop["L"] == "0" :
        #     if prop["dist"] == "MV1cB1" or prop["dist"] == "MV1cB2" or prop["dist"] == "mjj" :
        #         if not logy: return 1.1
        #     if prop["dist"] == "MET" :
        #         if not logy: return 1.0/1.15
        return 1.0

    def postprocess_main_content_histogram (self, prop, hist):
        # draw line denoting the transition of merged and resolved
        if prop["dist"] == "MET" or prop["dist"] == "pTV":
            max_value = hist.GetMaximum()
            min_value = 0#hist.GetYaxis().GetXmin()
            x_value = hist.GetXaxis().GetBinLowEdge(hist.GetXaxis().FindBin(500))
            l = ROOT.TLine(x_value, min_value, x_value, max_value)
            l.SetLineStyle(2)
            l.SetLineWidth(4)
            l.SetNDC(False)
            l.DrawLine(x_value, min_value, x_value, max_value)
            logging.debug("drawing line with endpoint coordinates ({},{}) and ({},{})".format(x_value, min_value, x_value, max_value))
        return hist

    def get_xTitle (self, prop, data_hist):
        """ get title of X-axis from properties """
        if not prop:
            return ""
        varname = prop["dist"]
        result = varname
        labels = {
            # new
            "MV1cB1": "MV1c(b_{1}) OP",
            "MV1cB2": "MV1c(b_{2}) OP",
            "MV1cBTag": "MV1c(b) OP",
            "dEtaBB": "#Delta#eta(b_{1},b_{2})",
            "dEtaVBB": "#Delta#eta(V,bb)",
            "dPhiLBmin": "#Delta#phi(lep,b)_{min}",
            "dPhiVBB": "#Delta#phi(V,bb)",
            "dRBB": "#DeltaR(b_{1},b_{2})",
            #"MEff": "M_{eff} [GeV]",
            #"MEff3": "M_{eff3} [GeV]",
            "MEff": "H_{T} [GeV]",
            "MEff3": "H_{T} [GeV]",
            "MET": "E_{T}^{miss} [GeV]",
            "mLL": "M_{ll} [GeV]",
            "mTW": "m_{T}(W) [GeV]",
            "mva": "BDT_{VH}",
            "mvaVZ": "BDT_{VZ}",
            "pTB1": "p_{T}(b_{1}) [GeV]",
            "pTB2": "p_{T}(b_{2}) [GeV]",
            "pTJ3": "p_{T}(j_{3}) [GeV]",
            "pTV": "p_{T}^{V} [GeV]",
            "VpT": "p_{T}^{V} [GeV]",
            "mVH": "m_{T}(Vh) [GeV]"
            }
        if "mjj" in varname:
            # nominal
            tmp_extra = ""
            tmp_extra2 = " [GeV]"
            # hack for mjj trafo D
            #tmp_extra = "Transformed "
            #tmp_extra2 = ""
            #
            if prop["T"] == "2":
                result = tmp_extra+"m_{bb}"+tmp_extra2
            elif prop["T"] == "1":
                result = tmp_extra+"m_{bj}"+tmp_extra2
            else:
                result = tmp_extra+"m_{jj}"+tmp_extra2
        elif "mBBJ" in varname:
            if prop["T"] == "2":
                result = "m_{bbj} [GeV]"
            elif prop["T"] == "1":
                result = "m_{bjj} [GeV]"
            else:
                result = "m_{jjj} [GeV]"
        elif "mBB" in varname:
            if prop["T"] == "2":
                result = "m_{bb} [GeV]"
            elif prop["T"] == "1":
                result = "m_{bj} [GeV]"
            else:
                result = "m_{jj} [GeV]"
        elif "mVH" in varname:
            if prop["L"] == "1" or prop["L"] == "0":
                result = "m_{T}(Vh) [GeV]"
            else:
                result = "m(Vh) [GeV]"
        elif varname in labels:
            result = labels[varname]
            #for k in labels:
                #if k in varname:
                    #return labels[k]
        return result

    def get_yTitle_tag (self, prop, data_hist):
        extra_unit = ""
        if prop["dist"] == "MEff" :  extra_unit = " GeV"
        if prop["dist"] == "MEff3" : extra_unit = " GeV"
        if prop["dist"] == "MET" :   extra_unit = " GeV"
        if prop["dist"] == "mLL" :   extra_unit = " GeV"
        if prop["dist"] == "mTW" :   extra_unit = " GeV"
        if prop["dist"] == "pTB1" :  extra_unit = " GeV"
        if prop["dist"] == "pTB2" :  extra_unit = " GeV"
        if prop["dist"] == "pTJ3" :  extra_unit = " GeV"
        if prop["dist"] == "pTV" :   extra_unit = " GeV"
        #if prop["dist"] == "VpT" :   extra_unit = " GeV" # new
        if prop["dist"] == "mjj" :   extra_unit = " GeV" # hack -> comment when trafoD
        if prop["dist"] == "mBB" :   extra_unit = " GeV"
        if prop["dist"] == "mBBJ" :  extra_unit = " GeV"
        if prop["dist"] == "mVH" :  extra_unit = " GeV"

        # NOTE: JWH - ED board requests
        if not self.do_rebinning(prop):
        # if not (prop["dist"] == "mVH" and prop.get("incFat", "-1") == "-1" and
        #     prop.get("D", "") == "SR" and prop.get("L", "0") == "2") :
            extra_number = str(data_hist.GetBinWidth(1))
            if not extra_number.find('.') == -1: extra_number = extra_number[:extra_number.find('.')]
            extra_unit = " " + extra_number + extra_unit

        y_ratio = round(data_hist.GetBinWidth(1), 2)
        if (y_ratio*10) % 10 == 0 and (y_ratio*100) % 100 == 0: y_ratio = int(y_ratio)
        if prop["dist"] == "VpT": extra_str = " / bin" # new
        elif prop["dist"] == "mVH":  extra_str = " /" + extra_unit
        else: extra_str = " / " + str(y_ratio) + extra_unit # new

        if prop["dist"] == "MV1cB1":   extra_str = ""
        if prop["dist"] == "MV1cB2":   extra_str = ""
        if prop["dist"] == "MV1cBTag": extra_str = ""

        return extra_str

    def set_y_range (self, hist, nlegend_items, miny, maxy, log_scale, prop):
        # if log_scale and prop["dist"] == "mVH":
        #     hist.SetMaximum(maxy * 100)
        #     hist.SetMinimum(0.001)
        #     return

        bottom_padding = 1.0/16.0
        content_faction = 4.0/7.0 if nlegend_items <= 8 else 3.0/7.0
        if prop["dist"] == "mVH":
            # figures 2)a-d in conf note
            if (prop["L"] == "0" or prop["L"] == "2") and log_scale:
                if prop["T"] == "1" or prop["T"] == "2":
                    if prop["D"] == "mBBcr":
                        if prop.get("BMax", "-999") == "500":
                            content_faction *= 1.25
            # figures 3)a,b in conf note
            if prop["D"] == "topemucr" and log_scale:
                if prop["T"] == "1":
                    content_faction *= 1.15
                if prop["T"] == "2":
                    content_faction *= 1.25
            if "SR" in prop["D"]:
                # figures 6)a-d in conf note
                if prop.get("BMax", "-999") == "500" and log_scale:
                    if prop["L"] == "0":
                        if prop["T"] == "1":
                            content_faction *= 1.15
                        if prop["T"] == "2":
                            content_faction *= 1.25
                    if prop["L"] == "2":
                        content_faction *= 1.25
                # figures 7)a,c,d in conf note
                if prop.get("BMin", "-999") == "500" and not log_scale:
                    if prop["L"] == "0":
                        if prop["T"] == "1":
                            content_faction *= 1.5
                    if prop["L"] == "2":
                        if prop["T"] == "1":
                            content_faction *= 2.15
                        if prop["T"] == "2":
                            content_faction *= 1.15
        # figures 4)a-d in conf note
        if prop["dist"] == "mBB" and not log_scale:
            if prop.get("BMax", "-999") == "500" and not (prop.get("D", "") == "topemucr"):
            # if prop["L"] == "0":
            #     if prop["T"] == "1":
                content_faction *= 1.5
            if prop.get("BMax", "-999") == "500" and prop.get("D", "") == "topemucr":
                content_faction *= 1.15
        # figures 10)a-d in conf note
        if (prop["dist"] == "MET" or prop["dist"] == "pTV") and log_scale:
            content_faction *= 1.25
        if not log_scale:
            if miny < 1e-6: miny = 0
            plot_scale = (maxy - miny)
            bottom = miny - bottom_padding*plot_scale
            top = bottom + plot_scale/content_faction

            # hist.SetMinimum(bottom)
            # hist.SetMaximum(top)

            hist.GetYaxis().SetLimits(bottom, top)

            # hist.GetHistogram().GetYaxis().SetRangeUser(bottom, top)
            logging.debug("setting plot y-range to ({0}, {1})".format(hist.GetHistogram().GetYaxis().GetXmin(), hist.GetHistogram().GetYaxis().GetXmax()))
            return
        else:
            log_miny = ROOT.TMath.Log10(miny)
            log_maxy = ROOT.TMath.Log10(maxy)
            plot_scale = (log_maxy - log_miny)
            # 0.25 is just fine tuning
            # bottom = log_miny - 0.25*bottom_padding*plot_scale
            bottom = log_miny
            top = bottom + plot_scale/content_faction

            # hist.SetMinimum(ROOT.TMath.Power(10, bottom))
            # hist.SetMaximum(ROOT.TMath.Power(10, top))

            hist.GetYaxis().SetLimits(ROOT.TMath.Power(10, bottom), ROOT.TMath.Power(10, top))

            # hist.GetHistogram().GetYaxis().SetRangeUser(ROOT.TMath.Power(10, bottom), ROOT.TMath.Power(10, top))
            logging.debug("setting log scale plot y-range to ({0}, {1})".format(hist.GetHistogram().GetYaxis().GetXmin(), hist.GetHistogram().GetYaxis().GetXmax()))
            return

        # if not log_scale and miny > 0:
        #     miny = 0
        # if log_scale and miny <= 1:
        #     miny = 0.25
        # mini = miny
        #
        # if mini < 0:
        #     hist.SetMinimum(mini*1.25)
        # else:
        #     mini = 0
        #     # fix 0 cut in the Y axis
        #     #hist.SetMinimum(0.01)
        # if log_scale:
        #     hist.SetMaximum(maxy * 100)
        #     hist.SetMinimum(miny / 2.5)
        # else:
        #     hist.SetMaximum(mini + (maxy - mini) * 1.5)

    def auto_compute_ratio_yscale_from_properties (self, prop):
        return (prop["dist"] == "mva" or prop["dist"] == "mvaVZ")

    def scale_all_yvals(self, prop):
        return prop["dist"] == "mva", 0.05

    def postprocess_dataMC_ratio_histogram (self, prop, hist):
        return hist

    def determine_year_from_title (self, title):
        if "2015" in title:
            return "2015"
        elif "2012" in title:
            return "2012"
        elif "2011" in title:
            return "2011"
        elif "both" in title:
            return "4023"

    def add_additional_signal_info_to_legend (self, legend, signal):
        if signal.mode == self._STACK:
            legend.AddEntry(ROOT.NULL, "m_{H}=" + str(signal.mass) + " GeV", "")
        else:
            legend.AddEntry(ROOT.NULL, "m_{H}=" + str(signal.mass) + " GeV", "")
