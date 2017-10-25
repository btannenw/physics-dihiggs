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
        #self.force_mu = (True, 0.226395333518) # 600 GeV
        self.force_mu = (False, 1.0)
        
        # for child classes to use
        # self.loggingLvl = logging.INFO
        self.loggingLvl = logging.DEBUG
        self.verbose = False
        self.formats = [ 'eps', 'pdf', 'png', 'root', 'C' ]
        self.blind = False
        self.thresh_drop_legend = 0.01
        self.restrict_to = []
        self.excludes = []
        self.additionalPlots = []
        self.add_sig_to_ratio_plot = False
        self.use_exp_sig = False
        # self.transferResults_fitName = "HiggsNorm"
        # self.get_binning_hist_removal = ["_meas2l2q2v2q"]
        self.bkg_substr_name = "None"
        self.bkg_substr_list = []
        self.file_tags = ["year","chan","cat","reg","sel"]
        self.weight_tags = []
        self.sig_names = ["VBFH125", "ggH125","ttH125","WH125","ZH125"]
        self.signal = ["Signal (best fit)", self._STACK, ROOT.kRed + 1, 1] # last = mult factor
        self.expected_signal = ["Signal (#mu=1)", self._STACK, ROOT.kRed +1, 1] # last = expected mu
        # self.additional_signal = ["A#rightarrow Zh", self._OVERPRINT, ROOT.kRed +1, 1.]
        self.bkg_tuple = {
            "Ztt" : ("Ztautau", 25, ROOT.kAzure + 1, []),
            "Others_hh" : ("Others", 24, ROOT.kMagenta + 1, []),
            "Zll" : ("Zellell", 23, ROOT.kMagenta + 3, []),
            "W" : ("W", 22, ROOT.kMagenta + 2, []),
            "VV" : ("Diboson", 21, ROOT.kMagenta, []),
            "Top" : ("top", 20, ROOT.kBlue + 3, []),
            "Fake" : ("Fake", 19, ROOT.kYellow, []),
            "Zttmm" : ("Ztautau(mm)", 25, ROOT.kAzure + 1, []),
            "Zttem" : ("Ztautau(em)", 25, ROOT.kAzure + 1, []),
            "Zttee" : ("Ztautau(ee)", 25, ROOT.kAzure + 1, []),
            "Zttme" : ("Ztautau(me)", 25, ROOT.kAzure + 1, []),
            "VBFHWW" :("VBF HWW", 25, ROOT.kGreen + 1, []),
            "ggHWW" :("ggH HWW", 25, ROOT.kGreen + 1, []),
            "ZHWW" :("ZHWW", 25, ROOT.kGreen + 1, []),
            "WHWW" :("WHWW", 25, ROOT.kGreen + 1, []),
            "Others" : ("Other", 25, ROOT.kMagenta + 1, []),
            "OtherRun1" : ("other", 25, ROOT.kMagenta + 1, []),
            'Background': ("bkg", 2, ROOT.kOrange, [])}
        self.ATLAS_suffix = "Internal"
        # self.ATLAS_suffix = "Simulation"
        #self.ATLAS_suffix = "Preliminary"
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
        "ggH125": 69,
        "VBFH125" : 68,
        "WH125":67,
        "ZH125":66,
        "ttH125":65,
        "Ztt" : 49,
        "Others_hh" : 48,
        "Zll" : 47,
        "W" : 46,
        "VV" : 45,
        "Top" : 44,
        "Fake" : 43,
        "Others" : 42,
        "Zttee" : 41,
        "Zttmm": 40,
        "Zttem": 39,
        "Zttme" : 38,
        "VBFHWW" : 37,
        "ZHWW" : 36,
        "WHWW" : 35,
        "ggHWW" : 34,
        "OtherRun1" : 33,

        # added by BBT, Sep 29 2016
        "ttbar" : 45,
        "SingleTop" : 40,
        "Z" : 20,
        "Dibosons" : 50,
        "dibosons" : 50,
        "QCD" : 35,
        "Wv22" : 37,
        "Xhh500" : 70,
        "Xhh600" : 70,
        "Xhh700" : 70,
        "Xhh750" : 70,
        "Xhh800" : 70,
        "Xhh900" : 70,
        "Xhh1000" : 70,
        "Xhh1100" : 70,
        "Xhh1200" : 70,
        "Xhh1300" : 70,
        "Xhh1400" : 70,
        "Xhh1500" : 70,
        "Xhh1600" : 70,
        "Xhh1800" : 70,
        "Xhh2000" : 70,
        "Xhh2250" : 70,
        "Xhh250" : 70,
        "Xhh2750" : 70,
        "Xhh3000" : 70,

        }
        # for reduced diag plots only
        self.exclude_str = 'HiggsNorm'
        self.cov_classification = {
        "BTag": [False, ["btag_","BTag_"], []],
        "norm": [False, ["norm","Norm"], []],
        "Lepton": [False, ["el_","mu_","tau_","EL_","MU_","TAU_","TES_"], []],
        "Jet": [False, ["jet_","JES_","JER_"], []],
        "MET": [False, ["met_","pu_","MET_","PU_"], []],
        "LUMI": [False, ["LUMI"], []],
        "Theo": [False, ["QCDscale","NLO_","Gen","Matching", "BR","pdf"], []]
        }
        self.cov_special = {
        "noMCStat": [[], ["gamma"]],
        "JES": [["SigX", "XS", "norm_", "jet_","JES_","JER_"], []],
        "TESJESTNorm": [["SigX", "XS", "norm_", "jet_","tau_","JES_","JER_","TAU_","TES_"], []],
        "noMCnoBTag": [[],["gamma","btag_","BTag_"]],
        "BTag": [["SigX", "XS", "norm_", "bTag","BTag_"], []],
        "LL": [["SigX", "XS", "LL_","chanll_"],[]],
        "LH": [["SigX", "XS", "LH_","chanlh_"],[]],
        "HH": [["SigX", "XS", "HH_","chanhh_"],[]],
        "NFs": [["SigX", "XS", "norm_"], []],
        "POIs": [["SigX", "XS"], []],
        }
        self.syst_to_study = ["norm_boost_Ztt","norm_vbf_Ztt"]
        self.suspicious_syst = ["el_eff_trigger","MadgraphVsSherpa"]
        self.chanlabels = {
            "lh" : "#tau_{l}#tau_{h}",
            "ll" : "#tau_{l}#tau_{l}",
            "hh" : "#tau_{h}#tau_{h}"
            }
        self.catlabels = {
            "vbf" : "VBF",
            "boost" : "Boost",
            "rest" : "Rest",
            "vbfloose" : "VBF Lse cat.",
            "vbftight" : "VBF Tgt cat.",
            "boosttight" : "Bst Tgt cat.",
            "boostloose" : "Bst Lse cat.",
            "boosthighpth" : "Bst h-pTH cat.",
            "boostlowpth" : "Bst l-pTH cat.",
            "vbfhighdrtight" : "VBF H-DR Tgt cat.",
            "vbfhighdrloose" : "VBF H-DR Lse cat.",
            "vbflowdr" : "VBF L-DR cat."
            }
        # for yield ratios only
        self.category_condenser = {
        # "_HistSyst": ["_Exp", False],
        # "_dist(mva|mjj)": ["_dist", False],
        # "_distMV1cBTag": ["_dist", False],
        #"_distmV": ["_dist", False],
        # "_isMVA[01]": ["_isMVA", False],
        # "_B[0-5]_": ["_B9_", False],
        #"_B(Max500_BMin0|BMin500)_": ["_Bresolvedmerged_", False],
        # "_TType(ll|mm|tt|xx)": ["_TType", False],
        #"_T[012]": ["_Tx", False],
        #"_(incJet1_J|incFat1_Fat|J)[1235]": ["_Jx", False],
        # "_Spc[0-9a-z]*top[a-z]*cr": ["_TType", False],
        # "(multijet)(.*_L)([0123])(.*)": [r'MJ\3lep\2\3\4', False],
        #"_L[012]": ["_Lx", False],
        #"_D(SR|topemucr)": ["_DallRegions", False],
        # "_W(bb|bl|bc|cc)_": ["_Whf_", True],
        # "_Z(bb|bl|bc|cc)_": ["_Zhf_", True]
        }

        logging.basicConfig(format='%(levelname)s in %(module)s: %(message)s', level=self.loggingLvl)

    def do_rebinning (self, prop):
        # If we implement non-equidistant bins, we can use this function to decide 
        # if a histogram should be rebinned or not.
        return True

    def is_signal(self, compname):
        """ Check if a component is Higgs. If yes, return mass """
        #Hard-coded 125 GeV mass /EC
        signames = self.sig_names
        has_mass = False
        mass = ""
        
        # remove mVH from compname
        #compname = re.sub('mVH', '', compname)
        for sg in signames:
            if sg in compname:
                has_mass = True
                mass = "125"
                break
        return has_mass, mass

    def blind_data (self, setup):
        def _do_blinding (title):
            #return False, []
            return "T2" in title and "SR" in title, [0, 0]


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
        if self._year == "2016":
            lumi["2016"] = ["5.1", 13]
        if self._year == "1516":
            lumi["1516"] = ["8.3", 13]
        if self._year == "All":
            lumi["All"] = ["8.3", 13]
        return lumi

    def get_title_height (self):
        return 3.5 if self._year == "4023" else 2


    def getTableTitle(self, name, mode):
            properties = {}
            blocks = name.split('_')[1:] # remove "Htt_"
            tags = self.file_tags
            title = name
            i=0
            val = blocks
            print name
            for block in blocks:
                matching_tags = [t for t in tags if block.startswith(t)]
                the_tag = max(matching_tags, key=len)
                val[i] = block.replace(the_tag, '', 1)
                i+=1
                
            if mode==1:
                title = val[0] + " $"+ self.chanlabels[val[1]].replace('#','\\' )+"$"  #year and channel name 
            elif mode==2:
                title= self.catlabels[val[2]].replace('#',' ' )
                if(val[3]!='sig'):
                    title+=" ("+val[3]+" CR)"

            return title


    def draw_category_ids (self, props, l, pos, nf):


        nf += 0.25*nf # a bit more vertical spacing
        print "In draw category ID"
        region = ""
#        chanlabels = {
#            "lh" : "#tau_{l}#tau_{h}",
#            "ll" : "#tau_{l}#tau_{l}",
#            "hh" : "#tau_{h}#tau_{h}"
#            }
#
#        catlabels = {
#            "vbf" : "VBF category",
#            "boost" : "Boosted category",
#            "rest" : "Rest",
#            "vbfloose" : "VBF Loose cat.",
#            "vbftight" : "VBF Tight cat.",
#            "boosttight" : "Boosted Tight cat.",
#            "boostloose" : "Boosted Loose cat.",
#            "boosthighpth" : "Boosted high pTH cat.",
#            "boostlowpth" : "Boosted low pTH cat.",
#            "vbfhighdrtight" : "VBF High #Delta R Tight cat.",
#            "vbfhighdrloose" : "VBF High #Delta R Loose cat.",
#            "vbflowdr" : "VBF Low #Delta R cat."
#            }
        region = self.chanlabels[props["chan"]] + " " + self.catlabels[props["cat"]]
        print region

        if(props["sel"]=="CBA"):
            region += " (CBA)"

        signalControl = ""
        reglabels = {
            "sig" : "Signal region",
            "top" : "Top Control Region",
            "zll" : "Z#rightarrow ll Control Region",
            "ztt" : "Z Control Region",
            "control" :  "Control region"
            }
        signalControl = reglabels[props["reg"]] + " (Post-fit)"




        pos_next = pos[1] - 0.1*nf # a bit more spacing

        l.DrawLatex(pos[0], pos_next, region)
        if not signalControl == "":
            pos_next -= nf
            l.DrawLatex(pos[0], pos_next, signalControl)

        pos_next -= nf
        return (pos[0], pos_next)

    def force_mu_value (self):


        return self.force_mu

    def get_year_str (self):
        return self._year #if int(self._year) < 2015 else ""

    def get_xbound_from_properties (self, prop):
        
        return None #(0, 300) #if prop["dist"] == "pTB1" else None

    def get_legend_pos_from_properties (self, prop):
        result = None
        #if prop["L"] == '0' and prop["dist"] == "VpT":
        #    result = [0.155, 0.13, 0.375, 0.65]
        #if prop["dist"] == "dPhiVBB":
        #    result = [0.16, 0.16, 0.38, 0.68]
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
        # Hbb specific, so removed all content, just return hist!
        return hist

    def get_xTitle (self, prop, data_hist):
        """ get title of X-axis from properties """
        if not prop:
            return ""
        title=" "
        if prop["sel"]=="MVA":
            title="MMC"
        if prop["sel"]=="CBA":
            title="MMC"
        if prop["reg"]=="ztt":
            title="pT(Z)"
        
        return title

    def get_yTitle_tag (self, prop, data_hist):
        logging.debug("in get_yTitle_tag")
        extra_unit =" "
        if prop["sel"]=="CBA":
            extra_unit=" GeV"
        if prop["reg"]=="ztt":
            extra_unit=" MeV"
        #if prop["dist"] == "MEff" :  extra_unit = " GeV"
        
        y_ratio = round(data_hist.GetBinWidth(1), 2)
        if (y_ratio*10) % 10 == 0 and (y_ratio*100) % 100 == 0: y_ratio = int(y_ratio)
        extra_str = " / " + str(y_ratio) + extra_unit # new
        if data_hist.GetXaxis().GetXbins().GetSize()!=0:
            extra_str = " / "+extra_unit
        
        logging.debug(extra_str)
        return extra_str

    def set_y_range (self, hist, nlegend_items, miny, maxy, log_scale, prop):
        # if log_scale and prop["dist"] == "mVH":
        #     hist.SetMaximum(maxy * 100)
        #     hist.SetMinimum(0.001)
        #     return
        #logging.debug("In set_y_range: {0}, {1}".format(nlegend_items, log_scale))
        bottom_padding = 1.0/16.0
        content_faction = 4.0/7.0 if nlegend_items <= 8 else 3.0/7.0
               
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
        #return (prop["dist"] == "mva" or prop["dist"] == "mvaVZ")
        return False

    def scale_all_yvals(self, prop):
        #return prop["dist"] == "mva", 0.05
        return False, 0.05

    def postprocess_dataMC_ratio_histogram (self, prop, hist):
        return hist

    def determine_year_from_title (self, title):
        if "2015" in title:
            return "2015"
        elif "2016" in title:
            return "2016"
        elif "1516" in title:
            return "1516"
        elif "All" in title:
            return "All"
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
