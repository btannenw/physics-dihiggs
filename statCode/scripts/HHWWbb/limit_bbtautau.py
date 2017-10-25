import ROOT as R
from array import array

def xsect(scale=False, G=True):
    if G:
        xsectstr ="""
        303395   1.0                1.0            0.029493       Ghhbbtautau260c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M260  
        303396   1.3199	            1.0            0.027967       Ghhbbtautau300c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M300  
        303397   1.901 	            1.0            0.027167       Ghhbbtautau400c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M400  
        303398   0.8924             1.0            0.0275         Ghhbbtautau500c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M500  
        303399   0.4104             1.0            0.026519       Ghhbbtautau600c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M600  
        303400   0.20148            1.0            0.027525       Ghhbbtautau700c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M700  
        303401   0.10549            1.0            0.028584       Ghhbbtautau800c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M800  
        303402   0.05835            1.0            0.027054       Ghhbbtautau900c10        MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M900  
        303403   0.03368            1.0            0.027192       Ghhbbtautau1000c10       MGPy8EG_A14NNPDF23LO_RS_G_hh_bbtt_lh_c10_M1000
        """
    else:
        xsectstr ="""
        342626	   441.86     1.0	 0.026818        Hhhbbtautau260			MGPy8EG_A14NNPDF23LO_X260hh_bbtautau_lephad 
        342627	   634.98     1.0	 0.026978        Hhhbbtautau300			MGPy8EG_A14NNPDF23LO_X300hh_bbtautau_lephad   
        342628	   646.17     1.0	 0.026946        Hhhbbtautau400			MGPy8EG_A14NNPDF23LO_X400hh_bbtautau_lephad   
        342629	   276.9      1.0	 0.027029        Hhhbbtautau500			MGPy8EG_A14NNPDF23LO_X500hh_bbtautau_lephad   	
        342630	   107.99     1.0	 0.026863        Hhhbbtautau600			MGPy8EG_A14NNPDF23LO_X600hh_bbtautau_lephad   
        342631	   44.137     1.0	 0.026815        Hhhbbtautau700			MGPy8EG_A14NNPDF23LO_X700hh_bbtautau_lephad   	 
        342632	   19.26      1.0	 0.02696         Hhhbbtautau800			MGPy8EG_A14NNPDF23LO_X800hh_bbtautau_lephad   
        342633	   8.9208     1.0	 0.026963        Hhhbbtautau900			MGPy8EG_A14NNPDF23LO_X900hh_bbtautau_lephad   
        342634	   4.3747     1.0	 0.026865        Hhhbbtautau1000		MGPy8EG_A14NNPDF23LO_X1000hh_bbtautau_lephad  
        """

    BR = 1 #* 0.15/0.458  #0.577 * 0.0632 * 2 * 0.458

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
            m = int(tok[-1].split("_")[2].replace("X", "").replace("hh", ""))
        xs = float(tok[1]) * BR
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

    
def plot(name, masses, same=False, color=R.kMagenta, legend="exp (G, SLT, 10 fb-1)"):

    m = array('f', [])
    o = array('f', []);e = array('f', [])
    m2 = array('f', []);m1 = array('f', [])
    p1 = array('f', []);p2 = array('f', [])

    xs = xsect(scale=False, G=True if 'G' in legend else False)

    for mass in masses:
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

    ge = R.TGraphAsymmErrors(len(m), m, e, z, z, z, z)
    if same:
        ge.SetLineColor(color);ge.SetMarkerColor(color)    
    else:
        ge.SetLineColor(R.kBlack);ge.SetMarkerColor(R.kBlack)    
    
    g1 = R.TGraphAsymmErrors(len(m), m, e, z, z, m1, p1)
    g1.SetFillColor(R.kGreen)

    g2 = R.TGraphAsymmErrors(len(m), m, e, z, z, m2, p2)
    g2.SetFillColor(R.kYellow)

    mr1, er1 = run1(scale=True)
    gr1 = R.TGraphAsymmErrors(len(mr), mr1, er1, z, z, z, z)
    gr1.SetLineColor(R.kBlue);gr1.SetMarkerColor(R.kBlue)    
    

    g = R.TMultiGraph("", name.format("mass") + ";m_{X} [GeV]; Limit [pb]")
    if not same:
        g.Add(g2, "E3")
        g.Add(g1, "E3")

    g.Add(ge, "PL*")
    if same:
        leg.AddEntry(ge, legend, "l")
    else:
        g.Add(gr1, "PL*")
        leg.AddEntry(gr1, "run 1 (scaled)", "l")        
        leg.AddEntry(ge, legend, "l")        

    R.gPad.SetLogy(True)
    g.Draw("P" if same else "AP")
    c._hists.append(g)
    c._hists.append(ge)

    return

R.gROOT.SetBatch(True)
                    
if __name__ == '__main__':

    c = R.TCanvas()
    c._hists = []
    leg = R.TLegend(0.58,0.60,0.89,0.89)
    leg.SetBorderSize(0)
    leg.SetFillColor(0)

    masses = [300, 500, 600, 700, 800, 900, 1000]
    plot("HH270416.270416g_HH_13TeV_270416g_StatOnly_{0}_exp", masses, legend="exp (G, SLT, 3.2 fb-1)")
    plot("HH270416.270416h_HH_13TeV_270416h_FloatOnly_{0}_exp", masses, same=True, legend="exp (G, SLT, 10 fb-1)")

    masses = [300, 400, 600, 700, 800, 900, 1000]
    plot("HH270416.290416c_HH_13TeV_290416c_StatOnly_{0}_exp", masses, same=True, color=R.kGray+1, legend="exp (H, SLT, 3.2 fb-1)")
    plot("HH270416.290416d_HH_13TeV_290416d_FloatOnly_{0}_exp", masses, same=True, color=R.kRed, legend="exp (H, SLT, 10 fb-1)")

    name = "limit"
    leg.Draw()
    c.Print(name + ".png")
    c.Print(name + ".eps")    
