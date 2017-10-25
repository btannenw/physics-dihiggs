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

    
def plot(name, masses, same=False, color=R.kMagenta, legend="exp (G, SLT, 10 fb-1)", skipLegend = False, plotRun1 = False, plotCMS = False):

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

    if 'exp' in legend:
        ge = R.TGraphAsymmErrors(len(m), m, e, z, z, z, z)
        print 'exp', m, e
    else:
        ge = R.TGraphAsymmErrors(len(m), m, o, z, z, z, z)
        print 'obs', m, o

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


    m1CMS = array('f', [260, 270, 300, 350, 400, 450])
    e1CMS = array('f', [1.69, 1.13, 6.21e-1, 3.26e-1, 1.91e-1, 1.45e-1])    
    e1CMS_hh = array('f', [x * 89 for x in e1CMS])
    g1CMS = R.TGraphAsymmErrors(len(m1CMS), m1CMS, e1CMS_hh, z, z, z, z)
    g1CMS.SetLineColor(R.kRed);g1CMS.SetMarkerColor(R.kRed)    
    
    m2CMS = array('f', [450, 500, 550, 600, 650, 700, 800, 900])
    e2CMS = array('f', [2.27e-1, 1.74e-1, 1.48e-1, 1.33e-1, 1.21e-1, 1.13e-1, 1.05e-1, 1.01e-1])
    e2CMS_hh = array('f', [x * 89 for x in e2CMS])
    g2CMS = R.TGraphAsymmErrors(len(m2CMS), m2CMS, e2CMS_hh, z, z, z, z)
    g2CMS.SetLineColor(R.kRed);g2CMS.SetMarkerColor(R.kRed)    

    #g = R.TMultiGraph("", name.format("mass") + ";m_{X} [GeV]; Limit [pb]")
    g = R.TMultiGraph("", "Limits for hh#rightarrow bbWW" + ";m_{X} [GeV]; 95% CL Limit on #sigma(pp#rightarrow X#rightarrow hh) [pb]")
    #ge.SetMarkerStyle(20) # BBT (large circles)
    ge.SetMarkerStyle(1) # BBT (invisible)


    #if not same: # ORIGINAL
    #if "sel" not in legend:
    if "exp" in legend and 'stat+systs' in legend and 'Unc' not in legend:
        g.Add(g2, "E3")
        g.Add(g1, "E3")

    #g.Add(ge, "PL*") # BBT comment out
    g.Add(ge, "PL")

    if same:
        if not skipLegend:
            leg.AddEntry(ge, legend, "l")
    else:
        if plotCMS:
            g.Add(g1CMS, "PL*")
            g.Add(g2CMS, "PL*")
            leg.AddEntry(g1CMS, "CMS Run 2", "l")        
        if plotRun1:
            g.Add(gr1, "PL*")
            leg.AddEntry(gr1, "Run 1 Combined (scaled)", "l")        
        #print selection
        if not skipLegend:
            leg.AddEntry(ge, legend, "l")        


    R.gPad.SetLogy(True)
    R.gPad.SetTickx(True)
    R.gPad.SetTicky(True)
    g.Draw("P" if same else "AP")
    if same == False:
        print "AHHHH"
        g.GetYaxis().SetRangeUser(0.1, 1000) 
        g.GetXaxis().SetLimits(250, 2800)
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

    massesComb_lo2  = [500, 600, 750, 800, 900, 1000, 1100, 1200, 1300] # (2015+2016) + modeling
    massesComb_hi2  = [1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750, 2750] # (2015+2016) + modeling

    massesComb_opt700   = [500, 600, 750, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600] # (2015+2016) + modeling
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

  
    # Sept 30, 2016 [final] 
    plot("8000X.061016-v1-checkSVN_withSys_HH_13TeV_061016-v1-checkSVN_withSys_Systs_{0}_obs",massesComb_lo2, same=False, color = R.kBlack, legend = "exp (H, stat+systs)", skipLegend=False, plotRun1=False, plotCMS=False)
    plot("9000X.061016-v1-checkSVN_withSys_HH_13TeV_061016-v1-checkSVN_withSys_Systs_{0}_obs",massesComb_hi2, same=True, color = R.kBlack, legend = "exp (H, stat+systs)", skipLegend=True)
    plot("8000X.061016-v1-checkSVN_withSys_HH_13TeV_061016-v1-checkSVN_withSys_Systs_{0}_obs",massesComb_lo2, same=True, color = R.kBlack, legend = "obs (H, stat+systs)", skipLegend=False)
    plot("9000X.061016-v1-checkSVN_withSys_HH_13TeV_061016-v1-checkSVN_withSys_Systs_{0}_obs",massesComb_hi2, same=True, color = R.kBlack, legend = "obs (H, stat+systs)", skipLegend=True)
    #plot("8000X.300916-v0-50percentPSuncOnSignal_HH_13TeV_300916-v0-50percentPSuncOnSignal_Systs_{0}_obs",massesComb_lo2, same=True, color = R.kRed, legend = "exp (H, stat+systs), 50% Sig PS Unc", skipLegend=False)
    #plot("9000X.300916-v0-50percentPSuncOnSignal_HH_13TeV_300916-v0-50percentPSuncOnSignal_Systs_{0}_obs",massesComb_hi2, same=True, color = R.kRed, legend = "exp (H, stat+systs), 50% Sig PS Unc", skipLegend=True)
  

    name = "limit"
    leg.Draw()

    #line1 = R.TLine(900,0.16,900,10)
    line1 = R.TLine(1300,0.16,1300,10)
    #line1.SetLineStyle(2)
    line1.SetLineColor(R.kBlue + 2)
    line1.SetLineWidth(2)
    line1.Draw("same")
    hiLabel = R.TLatex()
    hiLabel.SetNDC()
    hiLabel.SetTextFont(42)
    hiLabel.SetTextColor(R.kBlack)
    hiLabel.SetTextSize(0.0175)
    x = .337
    y = .483
    hiLabel.DrawLatex(x+ .105, y, "High-mass")
    loLabel = R.TLatex()
    loLabel.SetNDC()
    loLabel.SetTextFont(42)
    loLabel.SetTextColor(R.kBlack)
    loLabel.SetTextSize(0.0175)
    loLabel.DrawLatex(x, y, "Low-mass")
    
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
