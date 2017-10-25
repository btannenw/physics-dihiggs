import ROOT as R
from array import array

def xsect(scale=False, G=True):
    if G:
        #xsectstr ="""
        #303395   1.0                1.0            0.029493       Ghhbbtautau260c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M260  
        #303396   1.3199	            1.0            0.027967       Ghhbbtautau300c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M300  
        #303397   1.901 	            1.0            0.027167       Ghhbbtautau400c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M400  
        #303398   0.8924             1.0            0.0275         Ghhbbtautau500c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M500  
        #303399   0.4104             1.0            0.026519       Ghhbbtautau600c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M600  
        #303400   0.20148            1.0            0.027525       Ghhbbtautau700c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M700  
        #303401   0.10549            1.0            0.028584       Ghhbbtautau800c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M800  
        #303402   0.05835            1.0            0.027054       Ghhbbtautau900c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M900  
        #303403   0.03368            1.0            0.027192       Ghhbbtautau1000c10       MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M1000
        xsectstr ="""
        342054  0.044           1.000   1.000000     Xhh700    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m700_WWbb
        """
    else:
        xsectstr ="""
        342053  0.59            1.000   1.000000     XhhSM     aMcAtNloHerwigppEG_UEEE5_CTEQ6L1_CT10ME_hh_m137_WWbb
        342053  0.59            1.000   1.000000     XhhSM     aMcAtNloHerwigppEG_UEEE5_CTEQ6L1_CT10ME_hh_m250_WWbb
        342054  0.044           1.000   1.000000     Xhh700    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m700_WWbb
        342055  0.041           1.000   1.000000     Xhh2000   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m2000_WWbb

        342056  0.044           1.000   1.000000     Xhh300    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m300_WWbb
        342057  0.044           1.000   1.000000     Xhh400    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m400_WWbb
        342058  0.044           1.000   1.000000     Xhh500    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m500_WWbb
        342059  0.044           1.000   1.000000     Xhh600    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m600_WWbb
        342060  0.044           1.000   1.000000     Xhh750    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m750_WWbb
        342061  0.044           1.000   1.000000     Xhh800    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m800_WWbb
        342061  0.044           1.000   1.000000     Xhh900    MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m900_WWbb
        342062  0.044           1.000   1.000000     Xhh1000   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m1000_WWbb
        342063  0.044           1.000   1.000000     Xhh1100   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m1100_WWbb
        342063  0.044           1.000   1.000000     Xhh1200   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m1200_WWbb
        342064  0.044           1.000   1.000000     Xhh1300   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m1300_WWbb
        342065  0.044           1.000   1.000000     Xhh1400   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m1400_WWbb
        342066  0.044           1.000   1.000000     Xhh1500   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m1500_WWbb
        342066  0.044           1.000   1.000000     Xhh1600   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m1600_WWbb
        342066  0.044           1.000   1.000000     Xhh1800   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m1800_WWbb
        342066  0.044           1.000   1.000000     Xhh2250   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m2250_WWbb
        342066  0.044           1.000   1.000000     Xhh2500   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m2500_WWbb
        342066  0.044           1.000   1.000000     Xhh2750   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m2750_WWbb
        342066  0.044           1.000   1.000000     Xhh3000   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m3000_WWbb
        342066  0.044           1.000   1.000000     Xhh5000   MGHerwigppEG_UEEE5_CTEQ6L1_CT10ME_Xhh_m5000_WWbb
        """
        #342626	   441.86     1.0	 0.026818        Hhhbbtautau260			MGPy8EG_A14NNPDF23LO_X260hh_bbtautau_lephad 
        #342627	   634.98     1.0	 0.026978        Hhhbbtautau300			MGPy8EG_A14NNPDF23LO_X300hh_bbtautau_lephad   
        #342628	   646.17     1.0	 0.026946        Hhhbbtautau400			MGPy8EG_A14NNPDF23LO_X400hh_bbtautau_lephad   
        #342629	   276.9      1.0	 0.027029        Hhhbbtautau500			MGPy8EG_A14NNPDF23LO_X500hh_bbtautau_lephad   	
        #342630	   107.99     1.0	 0.026863        Hhhbbtautau600			MGPy8EG_A14NNPDF23LO_X600hh_bbtautau_lephad   
        #342631	   44.137     1.0	 0.026815        Hhhbbtautau700			MGPy8EG_A14NNPDF23LO_X700hh_bbtautau_lephad   	 
        #342632	   19.26      1.0	 0.02696         Hhhbbtautau800			MGPy8EG_A14NNPDF23LO_X800hh_bbtautau_lephad   
        #342633	   8.9208     1.0	 0.026963        Hhhbbtautau900			MGPy8EG_A14NNPDF23LO_X900hh_bbtautau_lephad   
        #342634	   4.3747     1.0	 0.026865        Hhhbbtautau1000		MGPy8EG_A14NNPDF23LO_X1000hh_bbtautau_lephad  
        #"""

    #BR = 1 #* 0.15/0.458  #0.577 * 0.0632 * 2 * 0.458
    BR = 0.577 * 0.213 * 2 

    scaling = {}
    if scale:
        f = open("scripts/HHbbtautau/xsratios8_13.txt")
        for l in f:
            tok = l.split()
            m = float(tok[0])
            r = float(tok[1])
            scaling[m] = r

    result = {}
    for l in xsectstr.split('\n'):
        tok = l.split()
        if not tok: continue        
        if G:
            m = int(tok[-1].split("_")[-1].replace("M", ""))
        else:
            #m = int(tok[-1].split("_")[2].replace("X", "").replace("hh", ""))
            m = int(tok[-1].split("_")[5].replace("X", "").replace("hh", "").replace("m","")) # BBT, May 18 2016
        xs = float(tok[1]) / BR
        if scale:
            xs /= scaling[m]
        result[m] = xs
    return result

def run1(scale=False):

    m = array('f', [260, 300, 350, 400, 500, 800, 1000])
    e = array('f', [2.03, 2.57, 1.73, 0.87, 0.48, 0.30, 0.27])    

    if not scale:
        return m, e

    scaling = {}
    f = open("scripts/HHbbtautau/xsratios8_13.txt")
    for l in f:
        tok = l.split()
        mass = float(tok[0])
        r = float(tok[1])
        scaling[mass] = r

    escale = array('f', [])
    for mass, lim in zip(m, e):
        escale.append(lim * scaling[mass])

    return m, escale

    
def plot(name, masses, same=False, color=R.kMagenta, legend="exp (G, SLT, 10 fb-1)", skipLegend = False):

    m = array('f', [])
    o = array('f', []);e = array('f', [])
    m2 = array('f', []);m1 = array('f', [])
    p1 = array('f', []);p2 = array('f', [])

    xs = xsect(scale=False, G=True if 'G' in legend else False)

    for mass in masses:
        xs[mass] = xs[mass] 
        
        f = R.TFile.Open("root-files/" + name.format(mass) + "/" + str(mass) + ".root")
        info = f.Get("limit")
        vo  = info.GetBinContent(1) * xs[mass]
        ve  = info.GetBinContent(2) * xs[mass] 
        vm1 = ve - info.GetBinContent(5) * xs[mass] 
        vp1 = info.GetBinContent(4) * xs[mass] - ve
        vm2 = ve - info.GetBinContent(6) * xs[mass]
        vp2 = info.GetBinContent(3) * xs[mass] - ve
        
        m.append(mass)
        o.append(vo)
        e.append(ve)
        m1.append(vm1)
        m2.append(vm2)
        p1.append(vp1)
        p2.append(vp2)
        
        f.Close()

        
    z = array('f', [0]*len(m))

    print m, e
    ge = R.TGraphAsymmErrors(len(m), m, e, z, z, z, z)

    if same:
        ge.SetLineColor(color);ge.SetMarkerColor(color);ge.SetLineWidth(2);ge.SetLineStyle(2);    
    else:
        ge.SetLineColor(R.kBlack);ge.SetMarkerColor(R.kBlack);ge.SetLineWidth(2);ge.SetLineStyle(2);    
    
    if 'obs' in legend:
        ge.SetLineStyle(1)

    g1 = R.TGraphAsymmErrors(len(m), m, e, z, z, m1, p1)
    g1.SetFillColor(R.kGreen)

    g2 = R.TGraphAsymmErrors(len(m), m, e, z, z, m2, p2)
    g2.SetFillColor(R.kYellow)

    mr1, er1 = run1(scale=True)
    gr1 = R.TGraphAsymmErrors(len(mr1), mr1, er1, z, z, z, z)
    gr1.SetLineColor(R.kBlue);gr1.SetMarkerColor(R.kBlue)    
    

    #g = R.TMultiGraph("", name.format("mass") + ";m_{X} [GeV]; Limit [pb]")
    g = R.TMultiGraph("", "Limits for hh#rightarrow bbWW" + ";m_{X} [GeV]; 95% CL Limit on #sigma(pp#rightarrow X#rightarrow hh) [pb]")
    #ge.SetMarkerStyle(20) # BBT (large circles)
    ge.SetMarkerStyle(1) # BBT (invisible)


    #if not same: # ORIGINAL
    if "exp" in legend:
        g.Add(g2, "E3")
        g.Add(g1, "E3")

    #g.Add(ge, "PL*") # BBT comment out
    g.Add(ge, "PL")
    if same:
        if not skipLegend:
            leg.AddEntry(ge, legend, "l")
    else:
        #g.Add(gr1, "PL*")
        #leg.AddEntry(gr1, "Run 1 Combined (scaled)", "l")        
        #print selection
        if not skipLegend:
            leg.AddEntry(ge, legend, "l")        


    R.gPad.SetLogy(True)
    R.gPad.SetTickx(True)
    R.gPad.SetTicky(True)
    g.Draw("P" if same else "AP")
    if same == False:
        print "AHHHH"
        g.GetYaxis().SetRangeUser(0.1, 100) 
        g.GetXaxis().SetLimits(450, 3050)
        g.GetXaxis().SetNdivisions(505)
        line1 = R.TLine(900,0.2,901,10)

    g.Draw("P" if same else "AP")

    c._hists.append(g)
    c._hists.append(ge)

    return

R.gROOT.SetBatch(True)
                    
if __name__ == '__main__':

    c = R.TCanvas('c1','c1',800,800)
    c._hists = []
    #leg = R.TLegend(0.18,0.77,0.39,0.89)
    leg = R.TLegend(0.14,0.75,0.35,0.87)
    #leg = R.TLegend(0.14,0.70,0.35,0.82)
    leg.SetBorderSize(0)
    leg.SetFillColor(0)
    leg.SetTextSize(0.025)


    #masses = [300, 500, 600, 700, 800, 900, 1000]
    #masses = [250, 750, 2000]
    #masses = [700]
    #masses = [300, 400, 500, 600, 700, 750, 800, 1000, 1100, 1300, 1400, 1500]
    massesComb_final   = [500, 600, 750, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750] # (2015+2016) + modeling
    massesComb_mod   = [500, 600, 700, 750, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750] # (2015+2016) + modeling
    massesComb_lo   = [500, 600, 750, 800, 900] # (2015+2016) + modeling
    massesComb_hi   = [900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750] # (2015+2016) + modeling

    massesComb_opt700   = [500, 600, 750, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 2000] # (2015+2016) + modeling
    massesComb_opt2000  = [750, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750] # (2015+2016) + modeling

    massesHi1   = [900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750, 3000]
    massesHi1_2   = [900, 1000, 1100, 1300]
    massesRun_3   = [2250, 2500, 2750, 3000]
    massesComb   = [500, 600, 700, 750, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750] # (2015+2016)
    massesComb_2015   = [600, 700, 750, 800, 1000, 1100, 1300, 1400, 1500, 2000, 2500, 2750] # opt700 survivors
    massesComb_2016   = [500, 600, 700, 750, 800, 1000, 1100, 1300, 1400, 1500, 2000, 2500, 2750] # opt700 survivors
    masses2000   = [900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750] # opt2000 survivors
    masses700    = [500, 600, 700, 750, 800] # opt700 survivors
    massesNonRes = [600, 750, 800, 1000, 1100] # nonRes survivors
    massTest     = [700]

    #plot("1182X.240516-3-withLH_HH_13TeV_240516-3-withLH-StatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), with LH")
    #plot("1182X.240516-3-noLH_HH_13TeV_240516-3-noLH-StatOnly_{0}_exp", masses, same=True, legend = "exp (H, SLT, 3.2 fb-1), w/o LH")

    # Jul 04, 2016
    #plot("1182X.040716-v4-jun29inputs_asimovFit_HH_13TeV_040716-v4-jun29inputs_asimovFit_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float, all systs
    #plot("1182X.040716-v5-jun29inputs_statOnly_HH_13TeV_040716-v5-jun29inputs_statOnly_StatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # stat only
    #plot("1182X.040716-v7-jun29inputs_ttbarNormOnly_HH_13TeV_040716-v7-jun29inputs_ttbarNormOnly_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm only
    #plot("1182X.040716-v8-floatOnly_ttbarNorm_JER_HH_13TeV_040716-v8-floatOnly_ttbarNorm_JER_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + JER
    #plot("1182X.040716-v9-floatOnly_ttbarNorm_allJetSys_HH_13TeV_040716-v9-floatOnly_ttbarNorm_allJetSys_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + all SysJet
    #plot("1182X.040716-v10-floatOnly_ttbarNorm_SysJET_SysFT-EFF-Eigen_HH_13TeV_040716-v10-floatOnly_ttbarNorm_SysJET_SysFT-EFF-Eigen_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + all SysJet + all b-tagging syst
    #plot("1182X.040716-v11-floatOnly_ttbarNorm_SysJET-FT-MUON-MUONS_HH_13TeV_040716-v11-floatOnly_ttbarNorm_SysJET-FT-MUON-MUONS_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + SysJet, SysFT, SysMUON, SysMUONS
    #plot("1182X.040716-v2-floatOnly_ttbarNorm_SysJET-FT-MUON-MUONS-MET_HH_13TeV_040716-v2-floatOnly_ttbarNorm_SysJET-FT-MUON-MUONS-MET_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + SysJet, SysFT, SysMUON, SysMUONS, SysMET
    #plot("1182X.040716-v13-floatOnly_ttbarNorm_SysJET-FT-MUON-MUONS-MET-EL_HH_13TeV_040716-v13-floatOnly_ttbarNorm_SysJET-FT-MUON-MUONS-MET-EL_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + SysJet, SysFT, SysMUON, SysMUONS, SysMET, SysEL
    #plot("1182X.040716-v14-floatOnly_ttbarNorm_SysJET-FT-MUON-MUONS-MET-EG_HH_13TeV_040716-v14-floatOnly_ttbarNorm_SysJET-FT-MUON-MUONS-MET-EG_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + SysJet, SysFT, SysMUON, SysMUONS, SysMET, SysEG
    #plot("1182X.040716-v15-floatOnly_ttbarNorm_allSystsBut-SysEL-test1_HH_13TeV_040716-v15-floatOnly_ttbarNorm_allSystsBut-SysEL-test1_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + SysJet, SysFT, SysMUON, SysMUONS, SysMET, SysEG, SysEL_iso, SysEL_reco
    #plot("1182X.040716-v16-floatOnly_ttbarNorm_allSystsBut-SysEL-ID-Reco_HH_13TeV_040716-v16-floatOnly_ttbarNorm_allSystsBut-SysEL-ID-Reco_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + SysJet, SysFT, SysMUON, SysMUONS, SysMET, SysEG, SysEL_trigger, SysEL_iso
    #plot("1182X.040716-v18-floatOnly_ttbarNorm_allSystsBut_SysEL-Iso-Trigger_HH_13TeV_040716-v18-floatOnly_ttbarNorm_allSystsBut_SysEL-Iso-Trigger_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # ttbar norm + SysJet, SysFT, SysMUON, SysMUONS, SysMET, SysEG, SysEL_ID, SysEL_reco

    # Jul 06, 2016
    #plot("1182X.060716-v3-floatOnly_ttbarNorm_HH_13TeV_060716-v3-floatOnly_ttbarNorm_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-floatOnly_ttbarNorm_SRwithLH_HH_13TeV_060716-v2-floatOnly_ttbarNorm_SRwithLH_FloatOnly_{0}_exp", masses, same=True, color=R.kRed, legend = "exp (H, SLT, 3.2 fb-1), optNoLH + LH") # float ttbar, use LH cut on top of selection
    #plot("1182X.060716-v5-floatOnly_ttbarNorm_allSystButFailEL_HH_13TeV_060716-v5-floatOnly_ttbarNorm_allSystButFailEL_FloatOnly_{0}_exp", masses, same=True, color=R.kBlue, legend = "exp (H, SLT, 3.2 fb-1), optNoLH + Systs") # float ttbar, all systs but EL_Trigger and EL_iso

    # ####### LH Scan $$$$$$$
    #plot("1182X.060716-v3-floatOnly_ttbarNorm_HH_13TeV_060716-v3-floatOnly_ttbarNorm_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v20-LHscan1_HH_13TeV_060716-v20-LHscan1_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-sanity_HH_13TeV_060716-sanity_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-LHscan1_HH_13TeV_060716-LHscan1_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan1_HH_13TeV_060716-v2-LHscan1_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan2_HH_13TeV_060716-v2-LHscan2_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan3_HH_13TeV_060716-v2-LHscan3_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan4_HH_13TeV_060716-v2-LHscan4_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan5_HH_13TeV_060716-v2-LHscan5_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan6_HH_13TeV_060716-v2-LHscan6_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan7_HH_13TeV_060716-v2-LHscan7_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan8_HH_13TeV_060716-v2-LHscan8_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan9_HH_13TeV_060716-v2-LHscan9_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan10_HH_13TeV_060716-v2-LHscan10_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan11_HH_13TeV_060716-v2-LHscan11_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    ##plot("1182X.060716-v2-LHscan12_HH_13TeV_060716-v2-LHscan12_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan13_HH_13TeV_060716-v2-LHscan13_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan14_HH_13TeV_060716-v2-LHscan14_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan15_HH_13TeV_060716-v2-LHscan15_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan16_HH_13TeV_060716-v2-LHscan16_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan17_HH_13TeV_060716-v2-LHscan17_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan18_HH_13TeV_060716-v2-LHscan18_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan19_HH_13TeV_060716-v2-LHscan19_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan20_HH_13TeV_060716-v2-LHscan20_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan21_HH_13TeV_060716-v2-LHscan21_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan22_HH_13TeV_060716-v2-LHscan22_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan23_HH_13TeV_060716-v2-LHscan23_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan24_HH_13TeV_060716-v2-LHscan24_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan25_HH_13TeV_060716-v2-LHscan25_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan26_HH_13TeV_060716-v2-LHscan26_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan27_HH_13TeV_060716-v2-LHscan27_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan28_HH_13TeV_060716-v2-LHscan28_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization
    #plot("1182X.060716-v2-LHscan29_HH_13TeV_060716-v2-LHscan29_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # float ttbar, nonLH optimization

    # Jul 7, 2016
    #plot("1182X.060716-v5-floatOnly_ttbarNorm_allSystButFailEL_HH_13TeV_060716-v5-floatOnly_ttbarNorm_allSystButFailEL_FloatOnly_{0}_exp", masses, same=True, color=R.kBlue, legend = "exp (H, SLT, 3.2 fb-1), optNoLH + Systs") # float ttbar, all systs but EL_Trigger and EL_iso
    #plot("11822.070716-Unblinded700-v10-SRandCRdata-x700_HH_13TeV_070716-Unblinded700-v10-SRandCRdata-x700_FloatOnly_{0}_obs", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH")

    # Jul 12, 2016
    #plot("8000X.120716-v4-runAllPoints_allOpts_floatOnly_noSysts_HH_13TeV_120716-v4-runAllPoints_allOpts_floatOnly_noSysts_FloatOnly_{0}_exp", masses700, same=False, legend = "exp (H, stat only), sel700")
    #plot("7000X.120716-v4-runAllPoints_allOpts_floatOnly_noSysts_HH_13TeV_120716-v4-runAllPoints_allOpts_floatOnly_noSysts_FloatOnly_{0}_exp", massesNonRes, same=True, color=R.kRed, legend = "exp (H, stat only), optNonRes")
    #plot("9000X.120716-v4-runAllPoints_allOpts_floatOnly_noSysts_HH_13TeV_120716-v4-runAllPoints_allOpts_floatOnly_noSysts_FloatOnly_{0}_exp", masses2000, same=True, color=R.kBlue, legend = "exp (H, stat only), sel2000")

   # Aug 6, 2015+2016 with systs!
    #plot("combinedX.060816-v13-statOnly_2015plus2016_{0}_obs", massesComb_mod, same=False , legend = "exp (H, stat only)")
    #plot("combinedX.040816-v12-noModeling_2015plus2016_{0}_obs", massesComb_mod, same=True, color = R.kBlue , legend = "exp (H, stat only) bad singleTop")
    #plot("combinedX.060816-v14-objectAndModelingSysts_2015plus2016_SRandCRsysts_{0}_obs",massesComb_mod, same=True, color = R.kBlue , legend = "exp (H, stat+systs [SR+CR])") # modeling and object systs in SR+CR
    #plot("combinedX.060816-v14-objectAndModelingSysts_2015plus2016_SRandCRsysts_{0}_obs",massesComb_mod, same=False , legend = "exp (H, stat + systs [SR+CR])")
    #plot("combinedX.060815-v15-objectAndModelingSysts_2015plus2016_SRsystsOnly_{0}_obs",massesComb_mod, same=True, color = R.kBlue, legend = "exp (H, stat + systs [SR only])")
    #plot("combinedX.060815-v15-objectAndModelingSysts_2015plus2016_SRsystsOnly_{0}_obs", massesComb_mod, same=False, legend = "exp (H, stat + systs [SR only])")

    # Sept 16, 2016: full limits- objSR+CR, modSR+CR, ttbar ME,PS,PDF,scale all SR, floatCR+SR (ttbar ME, PS, and ISR/FSR from gaussian)
    #plot("combinedX.140916-v1-fullLimits_lumiIncluded_{0}_obs",massesComb_final, same=False, legend = "obs (H, stat+systs [SR+CR])")
    #plot("combinedX.140916-v4-fullLimits_{0}_exp",massesComb_hi, same=False, legend = "exp (H, stat+systs), 5% Single Top", skipLegend = False)
    #plot("combinedX.140916-v5-fullLimits_{0}_exp",massesComb_lo, same=True, color = R.kBlack, legend = "exp (H, stat+systs), 5% Single Top", skipLegend = True)
    #plot("combinedX.150916-fullLimits_normalSelections_50percentSingleTopUnc_{0}_exp",massesComb_final, same=True, color = R.kRed, legend = "exp (H, stat+systs), 50% Single Top")
    #plot("combinedX.150916-fullLimits_normalSelections_50percentSingleTopUnc_{0}_exp",massesComb_hi, same=False, legend = "exp (H, stat+systs), 50% Single Top corr.", skipLegend = False)
    #plot("combinedX.150916-fullLimits_normalSelections_50percentSingleTopUnc_lowMass900_{0}_exp",massesComb_lo, same=True, color = R.kBlack, legend = "exp (H, stat+systs), 50% Single Top corr.", skipLegend = True)
    #plot("combinedX.190916-v11-fullRange_singleTop50percentIndependentInSRandCR_{0}_exp",massesComb_hi, same=False, legend = "exp (H, stat+systs)", skipLegend = False)
    #plot("combinedX.190916-v11-fullRange_singleTop50percentIndependentInSRandCR_low900_{0}_exp",massesComb_lo, same=True, color = R.kBlack, legend = "exp (H, stat+systs)", skipLegend = True)

    # Sept 21, 2016: 
    #plot("combinedX.200916-v2-expectedLimitsWithSignalUnc_{0}_exp",massesComb_hi, same=False, legend = "exp (H, stat+systs)", skipLegend = False)
    #plot("combinedX.200916-v2-expectedLimitsWithSignalUnc_low900_{0}_exp",massesComb_lo, same=True, color = R.kBlack, legend = "exp (H, stat+systs)", skipLegend = True)
    #plot("combinedX.200916-v0-unblindedResults_{0}_obs",massesComb_hi, same=True, color = R.kBlack, legend = "obs (H, stat+systs)", skipLegend = False)
    #plot("combinedX.200916-v0-unblindedResults_low900_{0}_obs",massesComb_lo, same=True, color = R.kBlack, legend = "obs (H, stat+systs)", skipLegend = True)

    # Sept 21, 2016: 
    plot("combinedX.200916-v2-expectedLimitsWithSignalUnc_{0}_exp",massesComb_hi, same=False, legend = "exp (H, stat+systs)", skipLegend = False)
    plot("combinedX.200916-v2-expectedLimitsWithSignalUnc_low900_{0}_exp",massesComb_lo, same=True, color = R.kBlack, legend = "exp (H, stat+systs)", skipLegend = True)
    plot("combinedX.260916-v0-sep23inputs_fullRange_classicSelections_{0}_obs",massesComb_hi, same=True, color = R.kRed, legend = "exp (H, stat+systs), Sep23", skipLegend = False)
    plot("combinedX.260916-v0-sep23inputs_fullRange_classicSelections_{0}_obs",massesComb_lo, same=True, color = R.kRed, legend = "exp (H, stat+systs), Sep23", skipLegend = True)

    #plot("8000X.150916-v1-opt700_fullRange_HH_13TeV_150916-v1-opt700_fullRange_FloatOnly_{0}_exp",massesComb_opt700, same=True, color = R.kBlue, legend = "exp (H, stat only), opt700")
    #plot("9000X.150916-v2-opt2000_fullRange_HH_13TeV_150916-v2-opt2000_fullRange_FloatOnly_{0}_exp",massesComb_opt2000, same=True, color = R.kRed, legend = "exp (H, stat only), opt2000")

    # Sept 9, 2016: full limits- objSR+CR, modSR+CR, ttbar ME,PS,PDF,scale all SR, floatCR+SR (ttbar ME, PS, and ISR/FSR from gaussian)
    #plot("combinedX.090916-v1-fullRange_useSuyogNumbersTTbarMod_{0}_exp",massesComb_mod, same=False, legend = "exp (H, stat+systs [SR+CR]), Asimov")
    #plot("9000X.090916-v1-hiMass_fullRange_useSuyogNumbersTTbarMod_HH_13TeV_090916-v1-hiMass_fullRange_useSuyogNumbersTTbarMod_Systs_{0}_exp",massesComb_hi, same=False, legend = "exp (H, stat+systs [SR+CR]), Asimov", selection="Hi")
    #plot("8000X.090916-v1-lowMass_fullRange_useSuyogNumbersTTbarMod_HH_13TeV_090916-v1-lowMass_fullRange_useSuyogNumbersTTbarMod_Systs_{0}_exp",massesComb_lo, same=True, color = R.kBlack, legend = "exp (H, stat+systs [SR+CR]), Asimov", selection="Lo")

    # Sept 7, 2016: full limits- objSR+CR, modSR+CR, ttbar ME,PS,PDF,scale all SR, floatCR+SR
    ###plot("combinedX.070916-v11-fullRange_dataInCR_psuedoDataInSRwithScaledTTbar_{0}_exp",massesComb_mod, same=False, legend = "exp (H, stat+systs [SR+CR]), Asimov")
    #plot("combinedX.070916-v11-fullRange_dataInCR_psuedoDataInSRwithScaledTTbar_{0}_exp",massesComb_hi, same=False, legend = "exp (H, stat+systs [SR+CR]), Asimov", selection="Hi")
    #plot("combinedX.070916-v11-fullRange_dataInCR_psuedoDataInSRwithScaledTTbar_{0}_exp",massesComb_lo, same=True, color = R.kBlack, legend = "exp (H, stat+systs [SR+CR]), Asimov", selection="Lo")
    
    #plot("combinedX.080916-v0-ttbarModelingHandledByHistos_{0}_exp",massesComb_mod, same=True, color = R.kBlue, legend = "exp (H, stat+systs [SR+CR]), Asimov, ttbarMod from Histos")
    #plot("combinedX.080916-v4-noRadLo_fullMassRange_{0}_exp",massesComb_mod, same=True, color = R.kRed, legend = "exp (H, stat+systs [SR+CR]), Asimov, ttbarMod from Histos, no radLo")
   
    #plot("combinedX.070916-v10-fullRange_dataInCR_psuedoDataInSRwithScaledTTbar_{0}_obs",massesComb_mod, same=True, color = R.kBlue , legend = "exp (H, stat+systs [SR+CR]), data in CR + pseudo in SR")
    #plot("combinedX.070916-v12-100percentTTbarPSandME_{0}_exp",massesComb_mod, same=True, color = R.kBlue, legend = "exp (H, stat+systs [SR+CR]), Asimov, 100% PS and ME")
    
    # Sept 5, 2016: full limits- objSR+CR, modSR+CR, combined ttbar ME+PS 15%, floatCR+SR
    #plot("combinedX.050916-v40-floatCRandSR_allModSR_allObjSR_ttbarScalePDFinSRandCR_ttbarPSandMStogether15_{0}_exp",massesComb_mod, same=False, legend = "exp (H, stat+systs [SR+CR]), Asimov")
    #plot("combinedX.060816-v14-objectAndModelingSysts_2015plus2016_SRandCRsysts_{0}_obs",massesComb_mod, same=True, color = R.kBlue , legend = "exp (H, stat+systs [SR+CR], Aug6)") # modeling and object systs in SR+CR
    #plot("combinedX.060916-v1-test700stratFor900point_{0}_exp",massesComb_mod, same=True, color = R.kRed, legend = "exp (H, stat+systs [SR+CR]), switch 900")
    #plot("combinedX.070916-v10-fullRange_dataInCR_psuedoDataInSRwithScaledTTbar_{0}_obs",massesComb_mod, same=True, color = R.kRed , legend = "exp (H, stat+systs [SR+CR]), data in CR + pseudo in SR")

    # Sept 1, 2016: full limits- objSR+CR, modSR+CR, ttbar ME and PS from alpha, floatCR+SR
    #plot("combinedX.020916-v12-floatCRandSR_allObjSRandCR_allModSRandCR_ttbarPSandMEbyAlpha_ttbarModSR_{0}_obs",massesComb_mod, same=False, legend = "exp (H, stat+systs [SR+CR])") # modeling and object systs in SR+CR
    #plot("combinedX.060816-v14-objectAndModelingSysts_2015plus2016_SRandCRsysts_{0}_obs",massesComb_mod, same=True, color = R.kBlue , legend = "exp (H, stat+systs [SR+CR], Aug6)") # modeling and object systs in SR+CR
    ####plot("9000X.020916-v15-floatCRandSR_allObjSRandCR_noJER_allModSRandCR_ttbarPSandMEbyAlpha_ttbarModSR_HH_13TeV_020916-v15-floatCRandSR_allObjSRandCR_noJER_allModSRandCR_ttbarPSandMEbyAlpha_ttbarModSR_Systs_{0}_exp",massesRun_3, same=True, color = R.kRed , legend = "exp (H, stat+systs [SR+CR], noJER)")
    ####plot("9000X.020916-v14-floatCR_allObjSRandCR_allModSRandCR_ttbarPSandMEbyAlpha_ttbarModSR_HH_13TeV_020916-v14-floatCR_allObjSRandCR_allModSRandCR_ttbarPSandMEbyAlpha_ttbarModSR_Systs_{0}_exp",massesRun_3, same=True, color = R.kMagenta , legend = "exp (H, stat+systs [SR+CR], exp, floatCR)")
    #plot("9000X.050916-v1-floatCRandSR_allObjSRandCR_noJER_allModSRandCR_noTtbarPSorME_HH_13TeV_050916-v1-floatCRandSR_allObjSRandCR_noJER_allModSRandCR_noTtbarPSorME_Systs_{0}_exp",massesRun_3, same=True, color = R.kMagenta , legend = "exp (H, stat+systs [SR+CR], exp, noJER, no ttbar ME/PS)")
    #plot("9000X.050916-v3-floatCRandSR_allObjSRandCR_noJER_allModSRandCR_noTtbarPSorME_HH_13TeV_050916-v3-floatCRandSR_allObjSRandCR_noJER_allModSRandCR_noTtbarPSorME_Systs_{0}_obs",massesRun_3, same=True, color = R.kOrange , legend = "obs (H, stat+systs [SR+CR], exp, noJER, no ttbar ME/PS)")
    #plot("9000X.050916-v4-sameAs_v3_noPseudoDataInSR_HH_13TeV_050916-v4-sameAs_v3_noPseudoDataInSR_Systs_{0}_obs",massesRun_3, same=True, color = R.kCyan , legend = "obs (H, stat+systs [SR+CR], exp, noJER, no ttbar ME/PS, no SR pseudo)")
    #plot("9000X.050916-v5-floatCRandSR_allObjSRandCR_allModSRandCR_ttbarModSR_ttbarPEandMSfromAlpha_HH_13TeV_050916-v5-floatCRandSR_allObjSRandCR_allModSRandCR_ttbarModSR_ttbarPEandMSfromAlpha_Systs_{0}_obs",massesRun_3, same=True, color = R.kRed , legend = "obs (H, stat+systs [SR+CR], no SR pseudo)")
    

    # Aug 24, 2016: full limits- objSR, modSR+CR, ttbar ME and ttbar PS from histograms
    #plot("combinedX.240816-v20-floatCR_objSR_modSRandCR_ttbarPEandMSfromHisto_{0}_obs", massesComb_mod, same=False, legend = "exp (H, stat+syst)")
    #plot("combinedX.240816-v21-floatCR_objSR_modSR_ttbarPEandMSfromHisto_{0}_obs", massesComb_mod, same=True, color = R.kBlue, legend = "exp (H, stat+syst) [modSR]")
    #plot("9000X.240816-v22-floatCR_objSR_modSRandCR_mcStatFalse_HH_13TeV_240816-v22-floatCR_objSR_modSRandCR_mcStatFalse_Systs_{0}_obs",massesHi1, same=True, color = R.kBlue , legend = "exp (H, stat+syst [modSR only])")

    # Aug 17, 2016 syst tests
    #plot("combinedX.170816-v20-objSRandCR_modSRandCR_ttbarModSR_floatCR_{0}_obs", massesComb_mod, same=False, legend = "exp (H, stat+syst)")
    #plot("combinedX.060816-v13-statOnly_2015plus2016_{0}_obs", massesComb_mod, same=True, color=R.kBlue, legend = "exp (H, stat only), Aug6")
    ##plot("combinedX.170816-v21-objSRandCR_modSRandCR_ttbarModSR_floatCR_QCDzeroCheck_{0}_obs", massesComb_mod, same=True, color=R.kBlue, legend = "exp (H, stat+syst), no QCD")
    #plot("combinedX.060816-v14-objectAndModelingSysts_2015plus2016_SRandCRsysts_{0}_obs",massesComb_mod, same=True, color = R.kRed , legend = "exp (H, stat+syst), Aug6")# modeling and object systs in SR+CR
    #plot("9000X.170816-v26-statOnly_hiMass1_HH_13TeV_170816-v26-statOnly_hiMass1_StatOnly_{0}_obs",massesHi1, same=True, color = R.kMagenta , legend = "exp (H, stat)")
    ##plot("9000X.180816-v4-SRandCRfit_aug6systs_HH_13TeV_180816-v4-SRandCRfit_aug6systs_Systs_{0}_obs",massesHi1, same=True, color = R.kOrange , legend = "exp (H, stat+ Aug6 syst)")
    ##plot("9000X.180816-v5-SRandCRfit_aug6systs_removeLargeStatUncSamples_HH_13TeV_180816-v5-SRandCRfit_aug6systs_removeLargeStatUncSamples_Systs_{0}_obs",massesHi1, same=True, color = R.kCyan , legend = "exp (H, stat+ Aug6 syst), removeStatUncSamples")
    #plot("9000X.180816-v7-objSRandCR_modAllSRandCR_floatSRandCR_HH_13TeV_180816-v7-objSRandCR_modAllSRandCR_floatSRandCR_Systs_{0}_obs",massesHi1, same=True, color = R.kCyan , legend = "exp (H, stat+ Aug6 syst), floatTTbarSR+CR")
    #plot("9000X.180816-v8-objSR_allModSRandCR_floatCR_HH_13TeV_180816-v8-objSR_allModSRandCR_floatCR_Systs_{0}_obs",massesHi1, same=True, color = R.kOrange , legend = "exp (H, stat+ Aug6 syst), floatSR_modSR+CR_objSR")
    #plot("9000X.180816-v9-objSR_allModSR_floatCR_HH_13TeV_180816-v9-objSR_allModSR_floatCR_Systs_{0}_obs",massesHi1, same=True, color = R.kOrange+10 , legend = "exp (H, stat+ Aug6 syst), floatSR_modSR_objSR")
    #plot("9000X.180816-v10-objSR_allModSR_floatCR_HH_13TeV_180816-v10-objSR_allModSR_floatCR_Systs_{0}_exp",massesHi1, same=True, color = R.kBlack , legend = "exp (H, stat + Aug6 syst), Exp only, floatSR_modSR_objSR")


    # Aug 10, 2016 syst config tests
    #plot("80006.pres-100816-v0-objSRandCR_modSRandCR_ttbarModSRandCR_floatSRandCR-x700_HH_13TeV_pres-100816-v0-objSRandCR_modSRandCR_ttbarModSRandCR_floatSRandCR-x700_Systs_{0}_obs", massTest, same=False, legend = "exp (H, stat only)")
    #plot("80006.pres-100816-v1-objSRandCR_modSRandCR_ttbarModSRandCR_floatCR-x700_HH_13TeV_pres-100816-v1-objSRandCR_modSRandCR_ttbarModSRandCR_floatCR-x700_Systs_{0}_obs", massTest, same=False, legend = "exp (H, stat only)")
    #plot("80006.pres-100816-v2-objSRandCR_modSRandCR_ttbarModSR_floatCR-x700_HH_13TeV_pres-100816-v2-objSRandCR_modSRandCR_ttbarModSR_floatCR-x700_Systs_{0}_obs", massTest, same=False, legend = "exp (H, stat only)")
    #plot("80006.pres-100816-v3-objSRandCR_modSR_ttbarModSR_floatCR-x700_HH_13TeV_pres-100816-v3-objSRandCR_modSR_ttbarModSR_floatCR-x700_Systs_{0}_obs", massTest, same=False, legend = "exp (H, stat only)")
    #plot("80006.pres-100816-v4-objSR_modSR_ttbarModSR_floatCR-x700_HH_13TeV_pres-100816-v4-objSR_modSR_ttbarModSR_floatCR-x700_Systs_{0}_obs", massTest, same=False, legend = "exp (H, stat only)")

    # Aug 4, 2015 vs 2016 vs 2015+2016 
    ###plot("combinedX.040816-v12-noModeling_2015plus2016_{0}_obs", massesComb_mod, same=False , legend = "exp (H, stat only), 2015+216, 13.2 fb^{-1}")
    #plot("combinedX.050816-v11-allObjectSysts_noModeling_{0}_obs", massesComb_mod, same=True , color = R.kBlue, legend = "exp (H, stat + SR object systs)")
    #plot("combinedX.050816-v12-allObjectSysts_SRandCR_noModeling_{0}_obs", massesComb_mod, same=True , color = R.kRed, legend = "exp (H, stat + SRandCR object systs)")
    #plot("combinedX.050816-v9-noModeling_runJetSystsSR1_{0}_obs", massesComb_mod, same=True , color = R.kBlue, legend = "exp (H, stat + Jet Systs SR1)")
    #plot("combinedX.050816-v7-noModeling_runJetSystsSR2_{0}_obs", massesComb_mod, same=True , color = R.kRed, legend = "exp (H, stat + Jet Systs SR2)")
    #plot("combinedX.050816-v8-noModeling_runJetSystsSR3_{0}_obs", massesComb_mod, same=True , color = R.kOrange-3, legend = "exp (H, stat + Jet Systs SR3)")
    #plot("combinedX.050816-v10-noModeling_runJetSystsSR4_{0}_obs", massesComb_mod, same=True , legend = "exp (H, stat + Jet Systs SR4)")
    #plot("combinedX.030816-v0-allPoints_sanityCheck_noModeling_{0}_exp", massesComb_mod, same=True , color = R.kBlue, legend = "exp (H, stat only), 2015+2016, wrong Bkgs")
    ###plot("combinedX.050816-v0-noModeling_2016only_{0}_obs", massesComb_2016, same=True , color = R.kRed, legend = "exp (H, stat only), 2016, 10.1 fb^{-1}")
    ###plot("combinedX.050816-v1-noModeling_2015only_{0}_obs", massesComb_2015, same=True , legend = "exp (H, stat only), 2015, 3.2 fb^{-1}")

    # Aug 2, 2015+2016
    #plot("8000X.020816-v2-run700points_modelingSysts_2015and2016_HH_13TeV_020816-v2-run700points_modelingSysts_2015and2016_Systs_{0}_exp", masses700, same=False, legend = "exp (H, stat only), sel700")
    #plot("9000X.020816-v2-run2000points_modelingSysts_2015and2016_HH_13TeV_020816-v2-run2000points_modelingSysts_2015and2016_Systs_{0}_exp", masses2000, same=True, color=R.kBlue, legend = "exp (H, stat only), sel2000")
    #plot("combinedX.020816-v2-x700_and_x2000_2015and2016_modelingSysts_{0}_exp", massesComb_mod, same=False , legend = "exp (H, stat + modeling syst)")
    #plot("combinedX.030816-v0-allPoints_sanityCheck_noModeling_{0}_exp", massesComb_mod, same=True , color = R.kBlue, legend = "exp (H, stat only)")
    #plot("combinedX.010816-v1-x700_and_x200_combom_2015plus2016_{0}_exp", massesComb, same=True, color = R.kBlue, legend = "exp (H, stat only), 2015+2016, 13.3 fb^{-1}")

    # Aug 1, 2015+2016
    #plot("8000X.010816-v1-nominal_ttbarFloatOnly_2015plus2016_HH_13TeV_010816-v1-nominal_ttbarFloatOnly_2015plus2016_FloatOnly_{0}_exp", masses700, same=False, legend = "exp (H, stat only), sel700") 
    #plot("9000X.010816-v1-nominal_ttbarFloatOnly_2015plus2016_HH_13TeV_010816-v1-nominal_ttbarFloatOnly_2015plus2016_FloatOnly_{0}_exp", masses2000, same=True, color=R.kBlue, legend = "exp (H, stat only), sel2000") 
    #plot("combinedX.010816-v1-x700_and_x200_combom_2015plus2016_{0}_exp", massesComb, same=False, legend = "exp (H, stat only), 2015+2016, 13.3 fb^{-1}")
    #plot("combinedX.290716-v3-ttbarFloatOnly_2016only_x700_and_x2000_combo_{0}_exp", massesComb_2016, same=True, color=R.kRed, legend = "exp (H, stat only), 2016, 10.1 fb^{-1}")
    #plot("combinedX.270716-cern-v1-x700_and_x2000_combo_{0}_exp", massesComb_2015, same=True, color=R.kBlue, legend = "exp (H, stat only), 2015, 3.2 fb^{-1}")
 
    # Jul 29, 2016 (2016 only, 20.7)
    #plot("8000X.290716-v3-allPoints_ttbarFloatOnly_2016only_HH_13TeV_290716-v3-allPoints_ttbarFloatOnly_2016only_FloatOnly_{0}_exp", masses700, same=False, legend = "exp (H, stat only), sel700") 
    #plot("9000X.290716-v3-allPoints_ttbarFloatOnly_2016only_HH_13TeV_290716-v3-allPoints_ttbarFloatOnly_2016only_FloatOnly_{0}_exp", masses2000, same=True, color=R.kBlue, legend = "exp (H, stat only), sel2000") 
    #plot("combinedX.290716-v3-ttbarFloatOnly_2016only_x700_and_x2000_combo_{0}_exp", massesComb, same=False, legend = "exp (H, stat only), 2016, 10.1 fb^{-1}")
    #plot("combinedX.270716-cern-v1-x700_and_x2000_combo_{0}_exp", massesComb, same=True, color=R.kRed, legend = "exp (H, stat only), 2015, 3.2 fb^{-1}")

    # Jul 27, 2016  (2015 only, 20.7)
    #plot("8000X.270716-cern-v1-runAllPoints_noModelingObjSysts_ttbarFloatOnly_HH_13TeV_270716-cern-v1-runAllPoints_noModelingObjSysts_ttbarFloatOnly_FloatOnly_{0}_exp", masses700, same=False, legend = "exp (H, stat only), sel700")
    #plot("9000X.270716-cern-v1-runAllPoints_noModelingObjSysts_ttbarFloatOnly_HH_13TeV_270716-cern-v1-runAllPoints_noModelingObjSysts_ttbarFloatOnly_FloatOnly_{0}_exp", masses2000, same=True, color=R.kBlue, legend = "exp (H, stat only), sel2000")
    #plot("combinedX.270716-cern-v1-x700_and_x2000_combo_{0}_exp", massesComb, same=False, legend = "exp (H, stat only), bestSel")

    # Jul 14, 2016
    #plot("combinedX.140716-v0-x700_and_x2000_combo_hbsmPresentation_{0}_exp", massesComb, same=True, color=R.kRed, legend = "exp (H, stat only), OLD bestSel")


    # Old
    #plot("1182X.280616-v2-negQCDfix_asimovFit_HH_13TeV_280616-v2-negQCDfix_asimovFit_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH") # Negative QCD fix
    #plot("1182X.040716-jun29inputs_floatOnly_v2_HH_13TeV_040716-jun29inputs_floatOnly_v2_FloatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH")
    #plot("1182X.040716-jun29inputs_SRandCR_HH_13TeV_040716-jun29inputs_SRandCR_StatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH")
    #plot("1182X.040716-beforeDoingAnything_HH_13TeV_040716-beforeDoingAnything_StatOnly_{0}_exp", masses, same=False, legend = "exp (H, SLT, 3.2 fb-1), optNoLH")
    
    #plot("1182X.310516-noLH-2_HH_13TeV_310516-noLH-2_StatOnly_{0}_exp", masses, same=True,  color=R.kBlue, legend = "exp (H, SLT, 3.2 fb-1), optNoLH (t#bar{t} only)")
    #plot("1182X.010616-noLH-CR_HH_13TeV_010616-noLH-CR_StatOnly_{0}_exp", masses, same=True,  color=R.kRed, legend = "exp (H, SLT, 3.2 fb-1), optNoLH + CR")
    #plot("1182X.080616-NormFact2-addCrossChecks_HH_13TeV_080616-NormFact2-addCrossChecks_FloatOnly_{0}_exp", masses, same=True,  color=R.kBlue, legend = "exp (H, SLT, 3.2 fb-1), floating CR t#bar{t}")
    #plot("1182X.310516-addLH-3_HH_13TeV_310516-addLH-3_StatOnly_{0}_exp", masses, same=True, legend = "exp (H, SLT, 3.2 fb-1), optNoLH + LH")
    #plot("1182X.310516-withLH-2_HH_13TeV_310516-withLH-2_StatOnly_{0}_exp", masses, same=True, color=R.kBlue, legend = "exp (H, SLT, 3.2 fb-1), optWithLH")

    #plot("11822.240516-eta_noLH_HH_13TeV_240516-eta_noLH_StatOnly_{0}_exp", masses, same=False, legend="exp (H, SLT, 3.2 fb-1)")
    #plot("HH270416.270416g_HH_13TeV_270416g_StatOnly_{0}_exp", masses, legend="exp (G, SLT, 3.2 fb-1)")
    #plot("HH270416.270416h_HH_13TeV_270416h_FloatOnly_{0}_exp", masses, same=True, legend="exp (G, SLT, 10 fb-1)")

    #masses = [300, 400, 600, 700, 800, 900, 1000]
    #plot("HH270416.290416c_HH_13TeV_290416c_StatOnly_{0}_exp", masses, same=True, color=R.kGray+1, legend="exp (H, SLT, 3.2 fb-1)")
    #plot("HH270416.290416d_HH_13TeV_290416d_FloatOnly_{0}_exp", masses, same=True, color=R.kRed, legend="exp (H, SLT, 10 fb-1)")

    name = "limit"
    leg.Draw()

    line1 = R.TLine(900,0.16,900,10)
    #line1.SetLineStyle(2)
    line1.SetLineColor(R.kBlue + 2)
    line1.SetLineWidth(2)
    line1.Draw("same")
    hiLabel = R.TLatex()
    hiLabel.SetNDC()
    hiLabel.SetTextFont(42)
    hiLabel.SetTextColor(R.kBlack)
    hiLabel.SetTextSize(0.0175)
    hiLabel.DrawLatex(.252,.603,"High-mass")
    loLabel = R.TLatex()
    loLabel.SetNDC()
    loLabel.SetTextFont(42)
    loLabel.SetTextColor(R.kBlack)
    loLabel.SetTextSize(0.0175)
    loLabel.DrawLatex(.147,.603,"Low-mass")
    
    x = 0.60
    y = 0.845
    l = R.TLatex()
    l.SetNDC()
    l.SetTextFont(72)
    l.SetTextColor(R.kBlack)
    l.SetTextSize(0.035)
    l.DrawLatex(x,y,"ATLAS")
    p = R.TLatex()
    p.SetNDC()
    p.SetTextFont(42)
    p.SetTextColor(R.kBlack)
    p.SetTextSize(0.035)
    p.DrawLatex(x+0.115,y,"Internal")
    s = R.TLatex() 
    s.SetNDC()
    s.SetTextFont(42)
    s.SetTextSize(0.025)
    s.SetTextColor(R.kBlack)
    s.DrawLatex(x+0.01,y-0.05,"#sqrt{s}= 13 TeV, 13.2 fb^{-1}")
    #s.DrawLatex(x+0.01,y-0.05,"#sqrt{s}= 13 TeV")

    c.Print(name + ".png")
    c.Print(name + ".eps")    
