#!/usr/bin/env python2
import os
import sys
import ROOT


def main(): 
    ROOT.gROOT.SetBatch(True)
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetOptTitle(0)
    ROOT.gStyle.SetMarkerStyle(6)
    ROOT.gStyle.SetMarkerSize(1.25)

    ROOT.gStyle.SetFrameBorderMode(0)
    ROOT.gStyle.SetFrameFillColor(0)
    ROOT.gStyle.SetCanvasBorderMode(0)
    ROOT.gStyle.SetCanvasColor(0)
    ROOT.gStyle.SetPadBorderMode(0)
    ROOT.gStyle.SetPadColor(0)
    ROOT.gStyle.SetStatColor(0)
    ROOT.gStyle.SetTitleYOffset(1.3)

    ROOT.gStyle.SetPadTopMargin(0.05)
    ROOT.gStyle.SetPadRightMargin(0.05)
    ROOT.gStyle.SetPadBottomMargin(0.10)
    ROOT.gStyle.SetPadLeftMargin(0.10)


    global imgform, points
    #imgform = ".pdf" 
    imgform = ".eps"
    points = 1000
    #Lep = ["2"]
    #Lep = ["1"]
    Lep = ["0","1","2","012"]
    #make all the plots with variable numbers of replicas - check for bias
    for p in range(0,10):
        points = 100*(p+1)
        for lep in Lep:
            reducedchannelplots(lep)



def reducedchannelplots(NLep = "2"):
    """ Reduced set of plots. Add parameters wanted here """
    makeplots("SigXsec", "#hat{#mu}", NLep, True, True)
    #return;
    if ("0" in NLep) or ("2" in NLep):
        makeall("norm_Zbb","Zbb Norm", NLep)
    if "2" in NLep:
        #makeall("norm_ttbar_L0","t#bar{t} Norm L0", NLep)
        #makeall("norm_ttbar_L1","t#bar{t} Norm L1", NLep)
        makeall("norm_ttbar_L2","t#bar{t} Norm L2", NLep)
    if ("0" in NLep) or ("1" in NLep):
        makeall("norm_ttbar", "t#bar{t} Norm L0 L1", NLep)
    if "2" in NLep:
        makeall("norm_Zcl", "Zcl Norm", NLep)
    if "1" in NLep:
        makeall("norm_Wbb", "Wbb Norm", NLep)
        makeall("norm_Wcl", "Wcl Norm", NLep)
    #makeall("SysMETScaleSoftTerms","#slash{E}_{T} Scale Soft Terms", NLep)
    #makeall("SysJetNP1", "Jet NP1", NLep)
    #makeall("SysJetNP2", "Jet NP2", NLep)
    #makeall("SysJetNP3", "Jet NP3", NLep)
    #makeall("SysJetNP4", "Jet NP4", NLep)
    #makeall("SysJetNP5", "Jet NP5", NLep)
    #makeall("SysJetNP6_rest", "Jet NP6", NLep)
    

def channelplots(NLep = "2"):
    """ call making of plots for all NP. Not advisable unless you really want them all """
    makeplots("SigXsec", "#hat{#mu}", NLep, True, True) 

    makeall("norm_Zbb","Zbb Norm", NLep)
    makeall("norm_ttbar", "t#bar{t} Norm", NLep)
    makeall("LUMI", "Luminosity", NLep)
    makeall("SysBJetReso", "B-Jet Resolution", NLep)
    makeall("SysBTagB0Effic", "B-Tag B0 Efficiency", NLep)
    makeall("SysBTagB1Effic", "B-Tag B1 Efficiency", NLep)
    makeall("SysBTagB2Effic", "B-Tag B2 Efficiency", NLep)
    makeall("SysBTagB3Effic", "B-Tag B3 Efficiency", NLep)
    makeall("SysBTagB4Effic", "B-Tag B4 Efficiency", NLep)
    makeall("SysBTagB5Effic", "B-Tag B5 Efficiency", NLep)
    makeall("SysBTagB6Effic", "B-Tag B6 Efficiency", NLep)
    makeall("SysBTagB7Effic", "B-Tag B7 Efficiency", NLep)
    makeall("SysBTagB8Effic", "B-Tag B8 Efficiency", NLep)
    makeall("SysBTagB9Effic", "B-Tag B9 Efficiency", NLep)
    makeall("SysBTagC0Effic", "B-Tag C0 Efficiency", NLep)
    makeall("SysBTagC1Effic", "B-Tag C1 Efficiency", NLep)
    makeall("SysBTagC2Effic", "B-Tag C2 Efficiency", NLep)
    makeall("SysBTagC3Effic", "B-Tag C3 Efficiency", NLep)
    makeall("SysBTagC4Effic", "B-Tag C4 Efficiency", NLep)
    makeall("SysBTagC5Effic", "B-Tag C5 Efficiency", NLep)
    makeall("SysBTagC6Effic", "B-Tag C6 Efficiency", NLep)
    makeall("SysBTagC7Effic", "B-Tag C7 Efficiency", NLep)
    makeall("SysBTagC8Effic", "B-Tag C8 Efficiency", NLep)
    makeall("SysBTagC9Effic", "B-Tag C9 Efficiency", NLep)
    makeall("SysBTagC10Effic", "B-Tag C10 Efficiency", NLep)
    makeall("SysBTagC11Effic", "B-Tag C11 Efficiency", NLep)
    makeall("SysBTagC12Effic", "B-Tag C12 Efficiency", NLep)
    makeall("SysBTagC13Effic", "B-Tag C13 Efficiency", NLep)
    makeall("SysBTagC14Effic", "B-Tag C14 Efficiency", NLep)
    makeall("SysBTagL0Effic", "B-Tag L0 Efficiency", NLep)
    makeall("SysBTagL1Effic", "B-Tag L1 Efficiency", NLep)
    makeall("SysBTagL2Effic", "B-Tag L2 Efficiency", NLep)
    makeall("SysBTagL3Effic", "B-Tag L3 Efficiency", NLep)
    makeall("SysBTagL4Effic", "B-Tag L4 Efficiency", NLep)
    makeall("SysBTagL5Effic", "B-Tag L5 Efficiency", NLep)
    makeall("SysBTagL6Effic", "B-Tag L6 Efficiency", NLep)
    makeall("SysBTagL7Effic", "B-Tag L7 Efficiency", NLep)
    makeall("SysBTagL8Effic", "B-Tag L8 Efficiency", NLep)
    makeall("SysBTagL9Effic", "B-Tag L9 Efficiency", NLep)
    makeall("SysElecEResol", "Electron E Resolution", NLep)
    makeall("SysJVF", "JVF", NLep)
    makeall("SysJetBE", "Jet B-Jet E",NLep)
    makeall("SysJetEResol", "Jet E Resolution", NLep)
    makeall("SysJetEtaModel", "Jet #eta Model", NLep)
    makeall("SysJetEtaStat", "Jet #eta Stat", NLep)
    makeall("SysJetFlavB", "Jet Flav B", NLep)
    makeall("SysJetFlavComp_Top", "Jet Flavour Comp Top", NLep)
    makeall("SysJetFlavComp_VHVV", "Jet Flavour Comp VHVV", NLep)
    makeall("SysJetFlavComp_Wjets", "Jet Flavour Comp Wjets", NLep)
    makeall("SysJetFlavComp_Zjets", "Jet Flavour Comp Zjets", NLep)
    makeall("SysJetFlavResp_Top", "Jet Flavour Resp Top", NLep)
    makeall("SysJetFlavResp_VHVV", "Jet Flavour Resp VHVV", NLep)
    makeall("SysJetFlavResp_Wjets", "Jet Flavour Resp Wjets", NLep)
    makeall("SysJetFlavResp_Zjets", "Jet Flavour Resp Zjets", NLep)
    makeall("SysJetMu", "Jet Mu", NLep)
    makeall("SysJetNP1", "Jet NP1", NLep)
    makeall("SysJetNP2", "Jet NP2", NLep)
    makeall("SysJetNP3", "Jet NP3", NLep)
    makeall("SysJetNP4", "Jet NP4", NLep)
    makeall("SysJetNP5", "Jet NP5", NLep)
    makeall("SysJetNP6_rest", "Jet NP6", NLep)
    makeall("SysJetNPV", "Jet NPV", NLep)
    makeall("SysJetNonClos", "Jet Non-Closure", NLep)
    makeall("SysJetPilePt", "Jet Pileup Pt", NLep)
    makeall("SysJetPileRho", "Jet Pileup Rho", NLep)
    makeall("SysMETResoSoftTerms", "#slash{E}_{T} Resolution Soft Terms", NLep)
    makeall("SysMETScaleSoftTerms", "#slash{E}_{T} Scale Soft Terms", NLep)
    makeall("SysMuonEResolID", "Muon E Resolution ID", NLep)
    makeall("SysTChanPtB2", "TChan PtB2", NLep)
    makeall("SysTheoryAccPDF_ggZH","TheoryAccPDF ggZH", NLep)
    makeall("SysTheoryAccPDF_qqVH", "TheoryAccPDF qqVH", NLep)
    makeall("SysTheoryAcc_J2_ggZH", "TheoryAcc J2 ggZH", NLep)
    makeall("SysTheoryAcc_J2_qqVH", "TheoryAcc J2 qqVH", NLep)
    makeall("SysTheoryAcc_J3_ggZH", "TheoryAcc J3 ggZH", NLep)
    makeall("SysTheoryAcc_J3_qqVH", "TheoryAcc J3 qqVH", NLep)
    makeall("SysTheoryPDF_ggZH", "TheoryPDF ggZH", NLep)
    makeall("SysTheoryPDF_qqVH", "TheoryPDF qqVH", NLep)
    makeall("SysTheoryQCDscale_ggZH", "TheoryQCDScale ggZH", NLep)
    makeall("SysTheoryQCDscale_qqVH", "TheoryQCDScale qqVH", NLep)
    makeall("SysTheoryVHPt", "Theory VHPt", NLep)
    makeall("SysTopPt", "Top Pt", NLep)
    makeall("SysTtbarMBBCont", "t#bar{t} M_{bb} Cont", NLep)
    makeall("SysVVJetPDFAlphaPt", "VV Jet PDF Alpha Pt", NLep)
    makeall("SysVVJetScalePtST1", "VV Jet Scale Pt ST1", NLep)
    makeall("SysVVJetScalePtST2", "VV Jet Scale Pt ST2", NLep)
    makeall("SysVVMbb_WZ", "VV M_{bb} WZ", NLep)
    makeall("SysVVMbb_ZZ", "VV M_{bb} ZZ", NLep)
    makeall("SysWbbNorm", "Wbb Norm", NLep)
    makeall("SysWhfNorm_J3", "Whf Norm J3", NLep)
    makeall("SysWtChanAcerMC", "WtChan AcerMC", NLep)
    makeall("SysWtChanPythiaHerwig", "WtChan PythiaHerwig", NLep)
    makeall("SysZDPhi_J2_ZbORc", "ZD#phi J2 ZbORc", NLep)
    makeall("SysZDPhi_J2_Zl", "ZD#phi J2 Zl", NLep)
    makeall("SysZDPhi_J3_ZbORc", "ZD#phi J3 ZbORc", NLep)
    makeall("SysZDPhi_J3_Zl", "ZD#phi J3 Zl", NLep)
    makeall("SysZMbb_ZbORc", "ZM_{bb} ZbORc", NLep)
    makeall("SysZMbb_Zl", "ZM_{bb} Zl", NLep)
    makeall("SysZPtV_ZbORc", "ZPtV ZbORc", NLep)
    makeall("SysZPtV_Zl", "ZPtV Zl", NLep)
    makeall("SysZbbNorm_J3", "Zbb Norm J3", NLep)
    makeall("SysZbcZbbRatio", "Zbc/Zbb", NLep)
    makeall("SysZblZbbRatio", "Zbl/Zbb", NLep)
    makeall("SysZccZbbRatio", "Zcc/Zbb", NLep)
    makeall("SysZclNorm_J3", "Zcl Norm J3", NLep)
    makeall("SysZlNorm", "Zl Norm", NLep)
    makeall("SysZlNorm_J3", "Zl Norm J3", NLep)
    makeall("SysstopWtNorm", "stopWt Norm", NLep)
    makeall("SysttbarHighPtV", "t#bar{t} High PtV", NLep)

    if "2" in NLep:
        makeall("norm_Zcl", "Zcl Norm", NLep)
        makeall("SysElecE", "Electron E", NLep)
        makeall("SysMultijet_L2", "Multijet", NLep)
        makeall("SysMultijet_L2_Spctopemucr", "Multijet topemu", NLep)
        makeall("SysMuonEResolMS", "Muon E Resolution MS", NLep)
        makeall("SysVVMbb_WW", "VV M_{bb} WW", NLep)
        makeall("SysttbarNorm_J3_L2", "t#bar{t} Norm J3", NLep)

    elif "0" in NLep:
        makeall("SysLepVeto", "Lepton Veto", NLep)
        makeall("SysMultijet_J2_T1_L0", "Multijet J2 T1", NLep)
        makeall("SysMultijet_J2_T2_L0", "Multijet J2 T2", NLep)
        makeall("SysMultijet_J3_T1_L0", "Multijet J3 T1", NLep)
        makeall("SysMultijet_J3_T2_L0", "Multijet J3 T2", NLep)
        makeall("SysSChanAcerMC", "SChan AcerMC", NLep)
        makeall("SysSChanAcerMCPS", "SChan AcerMCPS", NLep)
        makeall("SysTheoryVPtQCD", "Theory VPt QCD", NLep)
        makeall("SysTheoryVPtQCD_qqVH", "Theory VPt QCD qqVH", NLep)
        makeall("SysWDPhi_J2_Wcl", "WDPhi J2 Wcl", NLep)
        makeall("SysWDPhi_J2_Whf", "WDPhi J2 Whf", NLep)
        makeall("SysWDPhi_J2_Wl", "WDPhi J2 Wl", NLep)
        makeall("SysWDPhi_J3_Wcl", "WDPhi J3 Wcl", NLep)
        makeall("SysWDPhi_J3_Whf", "WDPhi J3 Whf", NLep)
        makeall("SysWDPhi_J3_Wl", "WDPhi J3 Wl", NLep)
        makeall("SysWMbb_Wcl", "WM_{bb} Wcl", NLep)
        makeall("SysWMbb_Whf", "WM_{bb} Whf", NLep)
        makeall("SysWMbb_Wl", "WM_{bb} Wl", NLep)
        makeall("SysWPtV_Whf", "WPtV Whf", NLep)
        makeall("SysWbbMbbGS", "Wbb M_{bb} GS", NLep)
        makeall("SysWbcWbbRatio", "Wbc/Wbb", NLep)
        makeall("SysWblWbbRatio", "Wbl/Wbb", NLep)
        makeall("SysWccWbbRatio", "Wcc/Wbb", NLep)
        makeall("SysWclNorm", "Wcl Norm", NLep)
        makeall("SysWclNorm_J3", "Wcl Norm J3", NLep)
        makeall("SysWlNorm", "Wl Norm", NLep)
        makeall("SysWlNorm_J3", "Wl Norm J3", NLep)
        makeall("SysZclNorm", "Zcl Norm", NLep)
        makeall("SysstopsNorm", "stops Norm", NLep)
        makeall("SysstoptNorm", "stopt Norm", NLep)
        makeall("SysttbarNorm_J3", "t#bar{t} Norm J3", NLep)

def makeall(param="SigXsec", label="#hat{#mu}", NLep = "2"):
    """ call makeplots for parameter and error. Don't want significance for most """
    makeplots(param, label, NLep, True, False)  


def makeplots(param="SigXsec", label="#hat{#mu}", NLep="2", doError=False, doSignificance=False):
    """ Run requested plots of correlation,correlation as func of number of replicas and mean """
    os.system("mkdir -vp {0}".format(param))
    print "Correlation plots on parameter" , param
    corrplot(param, label, NLep)
    if doError:
        corrplot(param, label+" Error", NLep, True)
    if doSignificance:
        corrplot(param,label+"/("+label+" Error)", NLep, False, True)
    print "Correlation as function of replicas on parameter", param
    corrrep(param, label, NLep)
    if doError:
       corrrep(param,label+" Error", NLep, True)
    if doSignificance:
       corrrep(param,label+"/("+label+" Error)", NLep, False, True)
    print "Mean as function of replicas on parameter", param
    meanplot(param, label, NLep)
    if doError:
        meanplot(param,label+" Error", NLep, True)
    if doSignificance:
        meanplot(param,label+"/("+label+" Error)", NLep, False, True)

def corrplot(param="SigXsec", label="#hat{#mu}", NLep="2" , Error=False, Significance=False):
    """ Draw plot of the correlation of a parameter,error or significance(param/error) """

    if Error and Significance:
        print "Error, can't do significance of an error"
        return

    npoints = points

    rebin = 2

    if npoints < 500:
        rebin = 4

    if npoints < 400:
        rebin = 5
    
    if npoints < 200:
        rebin = 10

    filename="{0}Lepton".format(NLep)     
    if Significance:
        filename+="Significance"
    elif Error:
        filename+="Error"


    if Significance:
        inpar = param+"Results{0}.txt".format(NLep)
        inerr = param+"ErrResults{0}.txt".format(NLep)
        if not os.path.isfile(inpar) or not os.path.isfile(inerr):
            print "Parameter or Error file not found, exiting"
            return    
        fplength = len(open(inpar).readlines())
        felength = len(open(inerr).readlines())
        if fplength != felength:
            print "Parameter and Error file not of same size, exiting"
            return

        if fplength < npoints:
            npoints=fplength

    elif Error:
        infile = param+"ErrResults{0}.txt".format(NLep)
        if not os.path.isfile(infile):
            print "Error file not found, exiting"
            return
        flength = len(open(infile).readlines())
        if flength < npoints:
            npoints=flength
    else:
        infile = param+"Results{0}.txt".format(NLep)
        print "loading file", infile
        if not os.path.isfile(infile):
            print "Parameter file not found, exiting"
            return
        flength = len(open(infile).readlines())
        if flength < npoints:
            npoints=flength

    outfile = ROOT.TFile("{0}/{1}correlation_CUT_MVA{2}.root".format(param,filename,npoints),"recreate")
    tg = ROOT.TGraph(npoints)

    if Significance:
        inpar = param+"Results{0}.txt".format(NLep)
        inerr = param+"ErrResults{0}.txt".format(NLep)
        with open(inpar,"r") as fp, open(inerr,"r") as fe:
            for i in range(0,npoints):
                vals = fp.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                errs = fe.readline().split()           
                ecut = float(errs[0])
                emva = float(errs[1])
                scut = vcut/ecut
                smva = vmva/emva
                tg.SetPoint(i,scut,smva)

    elif Error:
        infile = param+"ErrResults{0}.txt".format(NLep)
        with open(infile,"r") as f:
            for i in range(0,npoints):
                vals = f.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                tg.SetPoint(i,vcut,vmva)

    else:
        infile = param+"Results{0}.txt".format(NLep)
        with open(infile,"r") as f:
            for i in range(0,npoints):
                vals = f.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                tg.SetPoint(i,vcut,vmva)         

    tg.GetXaxis().SetTitle(label+" cut-based")
    tg.GetYaxis().SetTitle(label+" MVA")
    tg.SetName("correlationGraph")
    c = ROOT.TCanvas(label+" correlation")
    tg.Draw("ap")
    tg.Write()
    xvals = tg.GetX()
    yvals = tg.GetY()
    minp=999.
    maxp=-999.
    mindiff=999.
    maxdiff=-999.
    for i in range(tg.GetN()):
        if xvals[i] < minp:
            minp = xvals[i]
        if yvals[i] < minp:
            minp = yvals[i]
        if xvals[i] > maxp:
            maxp = xvals[i]
        if yvals[i] > maxp:
            maxp = yvals[i]
        if xvals[i] - yvals[i] < mindiff:
            mindiff = xvals[i] - yvals[i]
        if xvals[i] - yvals[i] > maxdiff:
            maxdiff = xvals[i] - yvals[i]

    #stretch limits to be 1% either side min and max 
    spread = maxp - minp
    minp*=1.-((minp/abs(minp))*0.01*spread)
    maxp*=1.+((maxp/abs(maxp))*0.01*spread)
    diffspread = maxdiff - mindiff
    mindiff*=1.-((mindiff/abs(mindiff))*0.01*diffspread)
    maxdiff*=1.+((maxdiff/abs(maxdiff))*0.01*diffspread)

    hcorr = ROOT.TH2F("correlationHist", label+" Correlation", 100, minp, maxp, 100, minp, maxp)
    hdiff = ROOT.TH1F("Diff",label+" Cut-MVA",100, mindiff, maxdiff)
    
    for i in range(tg.GetN()):
        hcorr.Fill(xvals[i], yvals[i])
        hdiff.Fill(xvals[i]-yvals[i])

    hdiff.SetTitle(label+" cut - mva")
    hdiff.GetXaxis().SetTitle(label+" cut - "+label+" mva")
    hdiff.GetYaxis().SetTitle("Entries")

    hcorr.GetXaxis().SetTitle(label+" cut-based")
    hcorr.GetYaxis().SetTitle(label+" MVA")
    hcorr.Draw()   

    corr = tg.GetCorrelationFactor()
    cov = tg.GetCovariance()

    draw_ATLAS_label((0.2,0.8))
    l1 = new_TLatex()
    l1.DrawLatex(0.6, 0.2, "Correlation: {0:.3f}".format(corr))
    #l2 = new_TLatex()
    #l2.DrawLatex(0.6, 0.15, "Covariance: {0:.3f}".format(cov))

    c.SaveAs("{0}/{1}correlation{2}{3}".format(param,filename,npoints,imgform))

    hcorr.Write()       

    hcut = hcorr.ProjectionX("{0} CUT".format(param))
    hcut.SetTitle(label+" cut-based")
    hcut.GetXaxis().SetTitle(label+" cut-based")
    hcut.GetYaxis().SetTitle("Entries")
    hcut.Rebin(rebin)
    hcut.SetLineColor(ROOT.kBlack)
    hcut.Draw()
    
    fcut = ROOT.TF1("cutfit","gaus",minp,maxp)
    fcut.SetLineColor(ROOT.kBlue)
    fcut.SetLineWidth(3)
    hcut.Fit(fcut)
    hcut.Draw("same")
    cutl1 = new_TLatex()
    cutl1.DrawLatex(0.63, 0.90, "fit Mean: {m:.3f} +/- {e:.3f}".format(m=fcut.GetParameter(1),e=fcut.GetParError(1)))
    cutl2 = new_TLatex()
    cutl2.DrawLatex(0.63, 0.85, "fit Sigma: {s:.3f} +/- {e:.3f}".format(s=fcut.GetParameter(2),e=fcut.GetParError(2)))
    draw_ATLAS_label((0.2,0.8))
    cutchi = new_TLatex()
    cutchi.DrawLatex(0.63,0.80,"#chi^{{2}}/NDF={0:.2f}".format(fcut.GetChisquare()/fcut.GetNDF()))
    cutl3 = new_TLatex()
    cutl3.DrawLatex(0.63, 0.75, "hist Mean: {m:.3f} +/- {e:.3f}".format(m=hcut.GetMean(),e=hcut.GetMeanError()))
    cutl4 = new_TLatex()
    cutl4.DrawLatex(0.63, 0.70, "hist RMS: {s:.3f} +/- {e:.3f}".format(s=hcut.GetRMS(),e=hcut.GetRMSError()))
    
    c.SaveAs("{0}/{1}cut{2}{3}".format(param,filename,npoints,imgform))

    hcut.Write()

    hmva = hcorr.ProjectionY("{0} MVA".format(param)) 
    hmva.SetTitle(label+" mva") 
    hmva.GetXaxis().SetTitle(label+ " mva")
    hmva.GetYaxis().SetTitle("Entries")
    hmva.Rebin(rebin)
    hmva.SetLineColor(ROOT.kBlack)
    hmva.Draw()

    fmva = ROOT.TF1("mvafit","gaus",minp,maxp)
    fmva.SetLineColor(ROOT.kRed)
    fmva.SetLineWidth(3)
    hmva.Fit(fmva)
    fmva.Draw("same")
    mval1 = new_TLatex()
    mval1.DrawLatex(0.63, 0.90, "fit Mean: {m:.3f} +/- {e:.3f}".format(m=fmva.GetParameter(1),e=fmva.GetParError(1)))
    mval2 = new_TLatex()
    mval2.DrawLatex(0.63, 0.85, "fit Sigma: {s:.3f} +/- {e:.3f}".format(s=fmva.GetParameter(2),e=fmva.GetParError(2)))
    draw_ATLAS_label((0.2,0.8))
    mvachi = new_TLatex()
    mvachi.DrawLatex(0.63,0.80,"#chi^{{2}}/NDF={0:.2f}".format(fmva.GetChisquare()/fmva.GetNDF()))
    mval3 = new_TLatex()
    mval3.DrawLatex(0.63, 0.75, "hist Mean: {m:.3f} +/- {e:.3f}".format(m=hmva.GetMean(),e=hmva.GetMeanError()))
    mval4 = new_TLatex()
    mval4.DrawLatex(0.63, 0.70, "hist RMS: {s:.3f} +/- {e:.3f}".format(s=hmva.GetRMS(),e=hmva.GetRMSError()))

    c.SaveAs("{0}/{1}mva{2}{3}".format(param,filename,npoints,imgform))

    hmva.Write()

    hdiff.SetTitle(label+" cut - mva")
    hdiff.GetXaxis().SetTitle(label+" cut - "+label+" mva")
    hdiff.GetYaxis().SetTitle("Entries")
    hdiff.Rebin(rebin)
    hdiff.SetLineColor(ROOT.kBlack)
    hdiff.Draw()

    fdiff = ROOT.TF1("difffit","gaus",mindiff,maxdiff)
    fdiff.SetLineColor(ROOT.kGreen)
    fdiff.SetLineWidth(3)
    hdiff.Fit(fdiff)
    fdiff.Draw("same")
    diffl1 = new_TLatex()
    diffl1.DrawLatex(0.63, 0.90, "fit Mean: {m:.3f} +/- {e:.3f}".format(m=fdiff.GetParameter(1),e=fdiff.GetParError(1)))
    diffl2 = new_TLatex()
    diffl2.DrawLatex(0.63, 0.85, "fit Sigma: {s:.3f} +/- {e:.3f}".format(s=fdiff.GetParameter(2),e=fdiff.GetParError(2)))
    draw_ATLAS_label((0.2,0.8))
    diffchi = new_TLatex()
    diffchi.DrawLatex(0.63,0.80,"#chi^{{2}}/NDF={0:.2f}".format(fdiff.GetChisquare()/fdiff.GetNDF()))
    diffl3 = new_TLatex()
    diffl3.DrawLatex(0.63, 0.75, "hist Mean: {m:.3f} +/- {e:.3f}".format(m=hdiff.GetMean(),e=hdiff.GetMeanError()))
    diffl4 = new_TLatex()
    diffl4.DrawLatex(0.63, 0.70, "hist RMS: {s:.3f} +/- {e:.3f}".format(s=hdiff.GetRMS(),e=hdiff.GetRMSError()))

    c.SaveAs("{0}/{1}diff{2}{3}".format(param,filename,npoints,imgform))

    hdiff.Write()

    outfile.Close()

    print param, filename, "correlation:", corr
    print param, filename, "covariance:", cov



def corrrep(param="SigXsec", label="#hat{#mu}", NLep="2", Error=False, Significance=False):
    """ Draw plot of the correlation of a parameter,error or significance(param/error) as function of number of replicas"""

    if Error and Significance:
        print "Error, can't do significance of an error"
        return

    npoints = points

    filename="{0}Lepton".format(NLep)     
    if Significance:
        filename+="Significance"
    elif Error:
        filename+="Error"


    if Significance:
        inpar = param+"Results{0}.txt".format(NLep)
        inerr = param+"ErrResults{0}.txt".format(NLep)
        if not os.path.isfile(inpar) or not os.path.isfile(inerr):
            print "Parameter or Error file not found, exiting"
            return    
        fplength = len(open(inpar).readlines())
        felength = len(open(inerr).readlines())
        if fplength != felength:
            print "Parameter and Error file not of same size, exiting"
            return

        if fplength < npoints:
            npoints=fplength

    elif Error:
        infile = param+"ErrResults{0}.txt".format(NLep)
        if not os.path.isfile(infile):
            print "Error file not found, exiting"
            return
        flength = len(open(infile).readlines())
        if flength < npoints:
            npoints=flength
    else:
        infile = param+"Results{0}.txt".format(NLep)
        if not os.path.isfile(infile):
            print "Parameter file not found, exiting"
            return
        flength = len(open(infile).readlines())
        if flength < npoints:
            npoints=flength

    outfile = ROOT.TFile("{0}/{1}correlation_replicas{2}.root".format(param,filename,npoints),"recreate")
    
    tg = ROOT.TGraph()

    hrep = ROOT.TH1F("NRep",label+"Replicas", npoints, 1., npoints+1.)

    if Significance:
        inpar = param+"Results{0}.txt".format(NLep)
        inerr = param+"ErrResults{0}.txt".format(NLep)
        with open(inpar,"r") as fp, open(inerr,"r") as fe:
            for i in range(0,npoints):
                vals = fp.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                errs = fe.readline().split()           
                ecut = float(errs[0])
                emva = float(errs[1])
                scut = vcut/ecut
                smva = vmva/emva
                tg.Set(i+1)
                tg.SetPoint(i,scut,smva)
                co = tg.GetCorrelationFactor()
                hrep.Fill(i+1,co)

    elif Error:
        infile = param+"ErrResults{0}.txt".format(NLep)
        with open(infile,"r") as f:
            for i in range(0,npoints):
                vals = f.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                tg.Set(i+1)
                tg.SetPoint(i,vcut,vmva)
                co = tg.GetCorrelationFactor()
                hrep.Fill(i+1,co)

    else:
        infile = param+"Results{0}.txt".format(NLep)
        with open(infile,"r") as f:
            for i in range(0,npoints):
                vals = f.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                tg.Set(i+1)
                tg.SetPoint(i,vcut,vmva)  
                co = tg.GetCorrelationFactor()
                hrep.Fill(i+1,co)

    tg.GetXaxis().SetTitle(label+" cut-based")
    tg.GetYaxis().SetTitle(label+" MVA")
    tg.SetName("correlationGraph")
    c = ROOT.TCanvas(label+" correlation")
    tg.Draw("ap")
    tg.Write()
    xvals = tg.GetX()
    yvals = tg.GetY()
    minp=999.
    maxp=-999.
    for i in range(tg.GetN()):
        if xvals[i] < minp:
            minp = xvals[i]
        if yvals[i] < minp:
            minp = yvals[i]
        if xvals[i] > maxp:
            maxp = xvals[i]
        if yvals[i] > maxp:
            maxp = yvals[i]

    #stretch limits to be 1% either side min and max 
    spread = maxp - minp
    minp*=1.-((minp/abs(minp))*0.01*spread)
    maxp*=1.+((maxp/abs(maxp))*0.01*spread)

    hcorr = ROOT.TH2F("correlationHist", label+" Correlation", 100, minp, maxp, 100, minp, maxp)
    
    for i in range(tg.GetN()):
        hcorr.Fill(xvals[i], yvals[i])

    hcorr.GetXaxis().SetTitle(label+" cut-based")
    hcorr.GetYaxis().SetTitle(label+" MVA")
    hcorr.Draw()   

    corr = tg.GetCorrelationFactor()
    cov = tg.GetCovariance()

    draw_ATLAS_label((0.2,0.8))
    l1 = new_TLatex()
    l1.DrawLatex(0.6, 0.2, "Correlation: {0:.3f}".format(corr))

    c.SaveAs("{0}/{1}corrfullrep{2}{3}".format(param,filename,npoints,imgform))

    hcorr.Write()     

    hrep.SetLineColor(ROOT.kRed)
    hrep.GetXaxis().SetTitle("Number of Replicas")
    hrep.GetYaxis().SetTitle(label+" Correlation")
    hrep.Draw()
    draw_ATLAS_label((0.2,0.8))

    c.SaveAs("{0}/{1}corrnrep{2}{3}".format(param,filename,npoints,imgform))

    hrep.Write()

    outfile.Close()


def meanplot(param="SigXsec", label="#hat{#mu}", NLep = "2", Error=False, Significance=False):
    """ Draw plot of the mean of a parameter,error or significance(param/error) as function of number of replicas"""

    if Error and Significance:
        print "Error, can't do significance of an error"
        return

    npoints = points

    filename="{0}Lepton".format(NLep)    
    if Significance:
        filename+="Significance"
    elif Error:
        filename+="Error"


    if Significance:
        inpar = param+"Results{0}.txt".format(NLep)
        inerr = param+"ErrResults{0}.txt".format(NLep)

        if not os.path.isfile(inpar) or not os.path.isfile(inerr):
            print "Parameter or Error file not found, exiting"
            return   
        fplength = len(open(inpar).readlines())
        felength = len(open(inerr).readlines())
        if fplength != felength:
            print "Parameter and Error file not of same size, exiting"
            return
        if fplength < npoints:
            npoints=fplength

    elif Error:
        infile = param+"ErrResults{0}.txt".format(NLep)
        if not os.path.isfile(infile):
            print "Error file not found, exiting"
            return
        flength = len(open(infile).readlines())
        if flength < npoints:
            npoints=flength
    else:
        infile = param+"Results{0}.txt".format(NLep)
        if not os.path.isfile(infile):
            print "Parameter file not found, exiting"
            return
        flength = len(open(infile).readlines())
        if flength < npoints:
            npoints=flength

    outfile = ROOT.TFile("{0}/{1}mean_replicas{2}.root".format(param,filename,npoints),"recreate")

    hcutm = ROOT.TH1F("CutMean",label+"CutReplicas", npoints, 1., npoints+1.)
    hmvam = ROOT.TH1F("MVAMean",label+"MVAReplicas", npoints, 1., npoints+1.)

    cutsum = 0
    mvasum = 0

    if Significance:
        inpar = param+"Results{0}.txt".format(NLep)
        inerr = param+"ErrResults{0}.txt".format(NLep)
        with open(inpar,"r") as fp, open(inerr,"r") as fe:
            for i in range(0,npoints):
                vals = fp.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                errs = fe.readline().split()           
                ecut = float(errs[0])
                emva = float(errs[1])
                scut = vcut/ecut
                smva = vmva/emva
                cutsum += scut
                mvasum += smva
                hcutm.Fill(i+1,(cutsum/(i+1)))
                hmvam.Fill(i+1,(mvasum/(i+1)))

    elif Error:
        infile = param+"ErrResults{0}.txt".format(NLep)
        with open(infile,"r") as f:
            for i in range(0,npoints):
                vals = f.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                cutsum += vcut
                mvasum += vmva
                hcutm.Fill(i+1,(cutsum/(i+1)))
                hmvam.Fill(i+1,(mvasum/(i+1)))

    else:
        infile = param+"Results{0}.txt".format(NLep)
        with open(infile,"r") as f:
            for i in range(0,npoints):
                vals = f.readline().split()
                vcut = float(vals[0])
                vmva = float(vals[1])
                cutsum += vcut
                mvasum += vmva
                hcutm.Fill(i+1,(cutsum/(i+1)))
                hmvam.Fill(i+1,(mvasum/(i+1)))

    c = ROOT.TCanvas(label+" correlation")

    meanmax = max(hcutm.GetMaximum(),hmvam.GetMaximum())
    meanmin = min(hcutm.GetMinimum(),hmvam.GetMinimum())

    spread = meanmax - meanmin
    meanmin *= 1.-((meanmin/abs(meanmin))*0.05*spread)
    meanmax *= 1.+((meanmax/abs(meanmax))*0.05*spread)

    hcutm.SetLineColor(ROOT.kRed)
    hmvam.SetLineColor(ROOT.kBlue)
    hcutm.GetXaxis().SetTitle("Number of Replicas")
    hmvam.GetXaxis().SetTitle("Number of Replicas")
    hcutm.GetYaxis().SetTitle(label+" Mean")
    hmvam.GetYaxis().SetTitle(label+" Mean")
    hcutm.Draw()
    hcutm.SetMinimum(meanmin)    
    hcutm.SetMaximum(meanmax)
    hmvam.Draw("same")
    draw_ATLAS_label((0.2,0.8))

    legend=ROOT.TLegend(0.6,0.74,0.8,0.83)
    legend.SetFillColor(ROOT.kWhite)
    legend.SetLineColor(ROOT.kBlack)
    legend.SetTextFont(42)
    legend.SetTextSize(0.04)
    legend.SetLineStyle(1)
    legend.SetLineWidth(1)
    legend.SetShadowColor(0)
    legend.SetBorderSize(0)
    legend.AddEntry(hcutm, label+ " Cut", "l")
    legend.AddEntry(hmvam, label+ " MVA", "l")
    legend.Draw()

    c.SaveAs("{0}/{1}meanrep{2}{3}".format(param,filename,npoints,imgform))

    hcutm.Write()
    hmvam.Write()

    outfile.Close()



def new_TLatex():
    """ Create a simple TLatex object with correct properties """
    l = ROOT.TLatex()
    l.SetNDC()
    l.SetTextFont(72)
    l.SetTextSize(0.04)
    l.SetTextColor(1)
    l.SetTextAlign(11)
    return l

def draw_ATLAS_label(pos):
    """ Reimplementation of the official macro with better spacing """
    x = pos[0]
    y = pos[1]
    l = new_TLatex()
    delx = 0.13
    l.DrawLatex(x, y, "ATLAS")
    p = new_TLatex()
    p.SetTextFont(42)
    p.DrawLatex(x+delx, y, "Internal")

if __name__ == "__main__":
    #ROOT.gROOT.LoadMacro("./../macros/AtlasStyle.C") 
    #ROOT.SetAtlasStyle()
    main()
