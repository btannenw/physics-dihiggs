#!/usr/bin/env python

import sys
import os
import argparse

import ROOT
from ROOT import gDirectory, gROOT

import runFitCrossCheck
import makeReducedDiagPlots
import analysisPlottingConfig

# TODO: JWH cfg
# def main(cfg, versions, basedir_fcc, plotdir = '.', set_to_0 = False, specialGroup="None"):
def main(cfg, versions, basedir_fcc, is_conditional=False, is_asimov = False, plotdir = '.', set_to_0 = False, specialGroup="None", mu_asimov=1):
    # should pass as parameters the region and type of fit we want...
    # and filename oo
    
    if len(versions)<1:
        print "Nothing to merge, aborting"
        return
    os.system("mkdir -vp "+plotdir)
    gROOT.SetBatch(True)
    ROOT.gSystem.Load("libPlotUtils.so")
    pulls = []
    axiss = []
    nuiss = []

    for v in versions:
        directory = basedir_fcc.format(v)
        fname = "{0}/FitCrossChecks.root".format(directory)
        # TODO: JWH cfg
        # pull, corr = get_nuis_corr(cfg, fname, cfg._is_conditional, cfg._is_asimov, cfg._mu)
        pull, corr = get_nuis_corr(fname, is_conditional, is_asimov, mu_asimov)
        pulls.append(pull)
        nuis = initial_sort(pull)
        nuiss.append(nuis)

        #BBT, comment out for blinded data: Aug 17, 2016
        #if len(versions) is 1:
        #    # TODO: JWH cfg
        #    # pull, corr = get_nuis_corr(fname, cfg._is_conditional, not cfg._is_asimov)
        #    pull, corr = get_nuis_corr(fname, is_conditional, not is_asimov)
        #    pulls.append(pull)
        #    nuis = initial_sort(pull)
        #    nuiss.append(nuis)

    res = ROOT.std.vector('TGraph*')()
    for n in nuiss:
        res.push_back(n)
    ROOT.PU.mergeTGraphAxis(res, True)
    for n in nuiss:
        axiss.append(n.GetXaxis())

    # set pulls to 0 if requested
    if set_to_0:
        new_nuiss = []
        for nuis,axis in zip(nuiss, axiss):
            new_nuiss.append(ROOT.PU.shiftTGraphToZero(nuis, axis, True))
        nuiss = new_nuiss

    # play with canvas of NP
    # reference:
    g_2s = pulls[0].GetListOfPrimitives().At(1)
    g_1s = pulls[0].GetListOfPrimitives().At(2)
    amin = axiss[0].GetXmin()
    amax = axiss[0].GetXmax()
    length = int(amax-amin)+1

    for i in range(length):
        g_2s.SetPoint(i,amin+i,-2)
        g_1s.SetPoint(i,amin+i,-1)
    for i in range(length):
        g_2s.SetPoint(2*length-i-1,amin+i,2)
        g_1s.SetPoint(2*length-i-1,amin+i,1)

    # then plot only interesting things
    # TODO: JWH cfg customize
    res = reduce_all(nuiss, g_2s, g_1s, axiss, excludes=makeReducedDiagPlots.vector_TString("HiggsNorm"))
    nuiss = [n.Clone() for n in res[0]]
    axiss = [nuis.GetXaxis().Clone() for nuis in res[0]]
    nuis_plot(plotdir, "all", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss,
                     #includes=makeReducedDiagPlots.vector_TString("SysJetERe"))
    #nuis_plot(plotdir, "JetEResol", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss,
    #                 includes=makeReducedDiagPlots.vector_TString("SysJetE", "SysJetF", "SysJetM",
    #                                                              "SysBJet", "SysJetBE"))
    #nuis_plot(plotdir, "Jet1", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss,
    #                 includes=makeReducedDiagPlots.vector_TString("SysJetN", "SysJetP", "SysJVF",
    #                                                               "MET"))
    #nuis_plot(plotdir, "Jet2", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("TruthTagDR", "BTagB"))
    #nuis_plot(plotdir, "BTagB", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("BTagC"))
    #nuis_plot(plotdir, "BTagC", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("BTagL"))
    #nuis_plot(plotdir, "BTagL", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("norm"))
    #nuis_plot(plotdir, "Norm1", *res, hmin=0, hmax=2.5)

    #res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("DPhi", "Mbb"))
    #nuis_plot(plotdir, "Model", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss,
    #                 includes=makeReducedDiagPlots.vector_TString("DPhi", "WMbb", "WbbMbb", "ZMbb", "ZPt", "WPt"))
    #nuis_plot(plotdir, "ModelWZ", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss,
    #                 includes=makeReducedDiagPlots.vector_TString("VVJet", "VVMbb", "TopP", "Ttbar", "ttbarHigh", "TChan",
    #                                                             "WtChan", "SChan"))
    #nuis_plot(plotdir, "ModelTopVV", *res, hmin=-3, hmax=3)
    #res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("Elec", "Muon",
    #                                                                                        "Lep"))
    #nuis_plot(plotdir, "Lepton", *res, hmin=-3, hmax=3)

    #res = reduce_all(nuiss, g_2s, g_1s, axiss,
    #                 excludes=makeReducedDiagPlots.vector_TString("SysJetE", "SysJetF", "SysJetM", "SysBJet",
    #								  "SysJetBE", "SysJetN",
    #                                                              "SysJetP", "SysJVF", "MET", "TruthTagDR", "BTag", "norm", "Norm",
    #                                                              "Ratio", "Elec", "Muon", "Lep", "DPhi", "WMbb",
    #                                                              "WbbMbb", "ZMbb", "ZPt", "WPt", "VVJet",
    #                                                              "VVMbb", "TopP", "Ttbar", "ttbarHigh", "TChan", "WtChan",
    #                                                              "SChan"))
    #nuis_plot(plotdir, "Rest", *res, hmin=-3, hmax=3)

    #res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("Norm", "Ratio"))
    #nuis_plot(plotdir, "Norm2", *res, hmin=-3, hmax=3)

    #New classification


    # TODO: JWH cfg customize
    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysFT_EFF_Eigen", "SysFT_EFF_extrapolation", "SysFT_EFF_ALT_Eigen", "SysFT_EFF_ALT_extrapolation"))
    nuis_plot(plotdir, "BTag", *res,hmin=-5, hmax=5)


    #res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysWt", "SysTop", "SysTtbar", "SysMVH", "SysTTbar"))
    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("ttbar", "Stop", "TTbar", "stop"))
    nuis_plot(plotdir, "Top", *res,hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("VV", "WW","WZ","ZZ","VZ"))
    nuis_plot(plotdir, "Diboson", *res, hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("ZMbb", "ZPtV","Zb","Zc", "Zl"))
    nuis_plot(plotdir, "Zjets", *res, hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("WMbb", "WPtV","Wb","Wc", "Wl"))
    nuis_plot(plotdir, "Wjets", *res, hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("MJ"))
    nuis_plot(plotdir, "MJ", *res)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysMUON","SysEL","SysEG"))
    nuis_plot(plotdir, "Lepton", *res, hmin=-5, hmax=5)

    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysTAUS"))
    nuis_plot(plotdir, "Tau", *res, hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysJET","FATJET"))
    nuis_plot(plotdir, "Jet", *res, hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysAZh"))
    nuis_plot(plotdir, "TheorySignal", *res, hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysMET"))
    nuis_plot(plotdir, "MET", *res, hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("LUMI"))
    nuis_plot(plotdir, "LUMI", *res)

    ## OLD DEFS
    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysWt", "SysTop", "SysTtbar", "SysMVH", "SysTTbar"))
    nuis_plot(plotdir, "ModelTop", *res,hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("SysVV", "SysWM","SysZM","SysWD","SysZD","SysWP","SysZP", "SysVj"))
    nuis_plot(plotdir, "ModelBoson", *res, hmin=-5, hmax=5)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("Norm","Ratio"))
    nuis_plot(plotdir, "Norm", *res)


    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString("norm"))
    nuis_plot(plotdir, "FloatNorm", *res)


    suspicious_NP = []
    for nuis,axis in zip(nuiss, axiss):
        suspicious_NP.extend(makeReducedDiagPlots.flag_suspicious_NP(cfg, nuis, axis, .5, .5))
    res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString(*suspicious_NP))
    nuis_plot(plotdir, "Suspicious", *res, hmin=-3, hmax=3)


    if specialGroup is not "None":
        specialGroup = specialGroup.replace("ATLAS_", "")
        print "Making plot for special category: " + specialGroup
        res = reduce_all(nuiss, g_2s, g_1s, axiss, includes=makeReducedDiagPlots.vector_TString(specialGroup))
        nuis_plot(plotdir, "Special", *res, hmin=-3, hmax=3)



def nuis_plot(plotdir, name, nuiss, yellow, green, h=None, hmin=-5, hmax=5):
    c = ROOT.TCanvas("c","c",1000,600)
    if hmin < -1:
        hmax = hmax + 4
        hmin = hmin - 1
    else:
        hmax = hmax + 1
    if h is not None:
        h.SetMinimum(hmin)
        h.SetMaximum(hmax)
        ya = h.GetYaxis()
        h.Draw()
        nuiss[0].Draw("pz")
    else:
        nuiss[0].SetMinimum(hmin)
        nuiss[0].SetMaximum(hmax)
        nuiss[0].Draw("pza")
        ya = nuiss[0].GetYaxis()
    nuiss[0].SetTitle("")
    ya.SetTitle("pull")
    ya.SetTitleOffset(.5)
    ya.SetTickLength(0)
    yellow.Draw("f")
    green.Draw("f")
    colors=[1,2,ROOT.kBlue-4,6,ROOT.kGray+1]
    markerSize = 1
    lineWidth = 3
    if name is "all":
        markerSize = 0.5
        lineWidth = 1
    markerShift = 0.25


    for num,nuis in enumerate(nuiss):
        if num == 0:
            continue
        if num == 1:
            ROOT.PU.shiftTGraph(nuis, markerShift, True)
        if num == 2:
            ROOT.PU.shiftTGraph(nuis, -markerShift, True)
        if num > 2:
            print "WARNING: this is going to be ugly and hard to read"
            ROOT.PU.shiftTGraph(nuis, (num-1)*markerShift, True)
        col = colors[num]
        nuis.SetMarkerSize(markerSize)
        nuis.SetLineWidth(lineWidth)
        nuis.SetMarkerColor(col)
        nuis.SetLineColor(col)
        nuis.Draw("pz")
    nuiss[0].SetMarkerSize(markerSize)
    nuiss[0].SetLineWidth(lineWidth)
    nuiss[0].Draw("pz")
    nuiss[0].GetXaxis().LabelsOption("v")


    label = ROOT.TLatex()
    label.SetTextSize(0.045)
    offset = 0
    if  nuiss[0].GetXaxis().GetNbins() > 0:
        offset = 0.2 * nuiss[0].GetXaxis().GetNbins() * label.GetTextSize()
    if name is "all":
        label.SetTextSize(0.023)
        offset = 0.4
    label.SetTextAngle(90)
    label.SetTextFont(42)
    for i in range(1, nuiss[0].GetXaxis().GetNbins() + 1):
        labelStr = ROOT.TString(nuiss[0].GetXaxis().GetBinLabel(i))
        nuiss[0].GetXaxis().SetBinLabel(i, "")
        xpos = nuiss[0].GetXaxis().GetBinCenter(i) + offset
        # TODO: JWH cfg customize
        labelStr.ReplaceAll("Sys","")
        labelStr.ReplaceAll("FT_EFF_Eigen","FT")
        labelStr.ReplaceAll("extrapolation","extrap")
        labelStr.ReplaceAll("JET_JER_SINGLE_NP","JER")
        labelStr.ReplaceAll("JET_GroupedNP","JET")
        labelStr.ReplaceAll("SoftTerms","ST")
        labelStr.ReplaceAll("multijet","MJ")
        labelStr.ReplaceAll("Multijet","MJ")
        label.DrawText(xpos, 2.2, labelStr.Data())


    label.SetTextSize(0.03)
    for num,nuis in enumerate(nuiss):
        # add values in all cases by commenting the following 2 lines
        #if name is "all":
            #continue
        xVals = nuis.GetX()
        yVals = nuis.GetY()
        yErr = nuis.GetEYhigh()
        for i in range(0, nuis.GetN()):
            yPos = hmin + 0.2
            labelStr = "%.2f #pm %.2f" % (abs(yVals[i]), yErr[i])
            if name is "all":
                label.SetTextSize(0.018)
            label.DrawLatex(xVals[i], yPos, labelStr)
            if yVals[i] < 0:
                label.DrawLatex(xVals[i], yPos-0.11, "-")


    ROOT.gPad.SetTopMargin(ROOT.gPad.GetTopMargin()*0.15)
    ROOT.gPad.SetBottomMargin(ROOT.gPad.GetBottomMargin()*0.15)
    ROOT.gPad.SetLeftMargin(ROOT.gPad.GetLeftMargin()*.4)
    ROOT.gPad.SetRightMargin(ROOT.gPad.GetRightMargin()*.05)
    ROOT.gPad.Update()
    for ext in cfg.formats:
        c.Print("{2}/NP_{0}.{1}".format(name, ext, plotdir))

def reduce_all(nuiss, yellow, green, axiss, excludes=None, includes=None):
    new_nuis = []
    for nuis,axis in zip(nuiss, axiss):
        if excludes is not None:
            new_nuis.append(ROOT.PU.reduceTGraphAxisExclude(nuis, axis, True, excludes))
        elif includes is not None:
            new_nuis.append(ROOT.PU.reduceTGraphAxisInclude(nuis, axis, True, includes))
    max_axis = new_nuis[0].GetXaxis().GetXmax()
    new_y = yellow.Clone(yellow.GetName()+"_reduced")
    new_g = green.Clone(yellow.GetName()+"_reduced")
    ROOT.PU.removeTGraphPointsAbove(new_y, True, max_axis)
    ROOT.PU.removeTGraphPointsAbove(new_g, True, max_axis)
    return [new_nuis, new_y, new_g]

def get_nuis_corr(tfile, is_conditional, is_asimov, poi=1):
    f = ROOT.TFile.Open(tfile)

    mu = "1"
    if is_asimov:
        stub = "Asimov"
        f.cd("PlotsAfterFitToAsimov")
        mu = str(poi)
    else:
        stub = "Global"
        f.cd("PlotsAfterGlobalFit")
        mu = str(poi)
    if is_conditional:
        gDirectory.cd("conditionnal_MuIsEqualTo_"+mu)
        #if is_asimov:
        #    gDirectory.cd("conditionnal_MuIsEqualTo_"+mu)
        #else:
        #    gDirectory.cd("conditionnal_MuIsEqualTo_0")
        #p_nuis = gDirectory.Get("can_NuisPara_"+stub+"Fit_conditionnal_mu0")
        #p_corr = gDirectory.Get("can_CorrMatrix_"+stub+"Fit_conditionnal_mu0")
        p_nuis = gDirectory.Get("can_NuisPara_"+stub+"Fit_conditionnal_mu"+mu)
        p_corr = gDirectory.Get("can_CorrMatrix_"+stub+"Fit_conditionnal_mu"+mu)
    else:
        gDirectory.cd("unconditionnal")
        p_nuis = gDirectory.Get("can_NuisPara_"+stub+"Fit_unconditionnal_mu"+mu)
        p_corr = gDirectory.Get("can_CorrMatrix_"+stub+"Fit_unconditionnal_mu"+mu)
    return p_nuis, p_corr

def initial_sort(can_pulls):
    h = can_pulls.GetListOfPrimitives().At(0)
    axis = h.GetXaxis()
    nuis = can_pulls.GetListOfPrimitives().At(3)
    # function pointers not supported in PyROOT... have to workaround
    gROOT.ProcessLine("#include \"include/plotUtils.hpp\"")
    gROOT.ProcessLine("n = (TGraph*)"+str(ROOT.AddressOf(nuis)[0]))
    gROOT.ProcessLine("a = (TAxis*)"+str(ROOT.AddressOf(axis)[0]))
    gROOT.ProcessLine("PU::sortTGraphAxis(n, a, true, PU::comp_sysNames)")
    ROOT.PU.setTGraphAxis(nuis, axis, True)
    return nuis


if __name__ == "__main__":

    # TODO: JWH cfg
    # class MyParser(argparse.ArgumentParser):
    #     def error(self, message):
    #         sys.stderr.write('error: %s\n' % message)
    #         self.print_help()
    #         sys.exit(2)
    #
    # parser = MyParser(description='Compare nuisance parameter pulls from different workspaces', formatter_class=argparse.RawTextHelpFormatter)
    # parser.add_argument('workspace', help = 'nominal workspace: workspace/{name}/{something}/{mass}.root -> pass {name}')
    # parser.add_argument('-p', '--plot_modes', default = '0',
    #                     help = """Comma-separated list of FCC options:
    # 2: unconditional ( start with mu=1 )
    # 4: conditional mu = 0
    # 5: conditional mu = 1
    # 6: run Asimov mu = 1 toys: randomize Poisson term
    # 7: unconditional fit to asimov where asimov is built with mu=1
    # 8: unconditional fit to asimov where asimov is built with mu=0
    # 9: conditional fit to asimov where asimov is built with mu=1
    # 10: conditional fit to asimov where asimov is built with mu=0""", dest = 'mode')
    # parser.add_argument('-z', '--zero', help = "force the pulls of nominal workspace to zero",
    #                     dest = 'zero', action = 'store_true')
    # parser.set_defaults(zero = False)
    # parser.add_argument('-v', '--versions',
    #                     help = "Comma-separated list of different workspaces to compare",
    #                     default = [], dest = 'versions')
    # parser.add_argument('pass_to_user', nargs = argparse.REMAINDER, default = [])
    # args = parser.parse_args()
    #
    # cfg = analysisPlottingConfig.Config(args.pass_to_user)
    #
    # basedir_fcc = "fccs/FitCrossChecks_{0}_combined"
    #
    # special = args.workspace
    # modes = [int(s) for s in args.mode.split(',')]
    # versions = args.versions
    # zero = args.zero
    #
    # print "Running comparePulls for nominal workspace ", special
    # for algnum in modes:
    #     alg = runFitCrossCheck.available_algs[int(algnum)]
    #     mu = int(alg[1])
    #     is_conditional = False
    #     if alg[3] == "true":
    #         is_conditional = True
    #     is_asimov = False
    #     if "Asimov" in alg[0]:
    #         is_asimov = True
    #     cfg._is_conditional = is_conditional
    #     cfg._is_asimov = is_asimov
    #     cfg._mu = mu
    #     print "Running for conditional? ", is_conditional, " for asimov? ", is_asimov, " what mu? ", mu
    #     main(cfg, versions, basedir_fcc, "plots/PullComparisions", set_to_0 = zero, specialGroup = special)


    basedir_fcc = "fccs/FitCrossChecks_{0}_combined"
    is_conditional = False
    #if len(sys.argv)>1:

    is_conditional = int(sys.argv[1])
    is_asimov = int(sys.argv[2].split(',')[0])
    mu_asimov = int(sys.argv[2].split(',')[1] if ',' in sys.argv[2] else '1')
    plotdir = sys.argv[3]
    set_to_0 = bool(int(sys.argv[4]))
    special = sys.argv[5]
    versions = []
    for i in range(6, len(sys.argv)):
        versions.append(sys.argv[i])


    cfg = analysisPlottingConfig.Config([])
    cfg._is_conditional = is_conditional
    cfg._is_asimov = is_asimov

    gROOT.ProcessLine("TGraph* n;")
    gROOT.ProcessLine("TAxis* a;")

    main(cfg, versions, basedir_fcc, is_conditional, is_asimov, plotdir, set_to_0 = set_to_0, specialGroup = special, mu_asimov=mu_asimov)
