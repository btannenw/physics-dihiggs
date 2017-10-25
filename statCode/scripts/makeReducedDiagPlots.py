#!/usr/bin/env python

import sys
import os
import argparse

import ROOT
from ROOT import gDirectory, gROOT
#from ROOT import PU

import runFitCrossCheck
import analysisPlottingConfig


def main(cfg, version):#, directory, is_conditional=False, is_asimov=False, mu=None):
    # TODO this initialization is UGLY
    plotdir = "plots/{0}".format(version)
    try:
        os.mkdir(plotdir)
    except OSError:
        pass
    plotdir = "plots/{0}/fcc".format(version)
    try:
        os.mkdir(plotdir)
    except OSError:
        pass
    if cfg._is_asimov:
        dirname = "AsimovFit_"
        if cfg._mu is None:
            cfg._mu = 1 # default value
    else:
        dirname = "GlobalFit_"
        if cfg._mu is None:
            cfg._mu = 0 # default value
    if cfg._is_conditional:
        condname = "conditionnal_"
    else:
        condname = "unconditionnal_"
    muname = "mu{0}".format(cfg._mu)
    mu = str(cfg._mu)

    plotdir = "plots/{0}/fcc/{1}{2}{3}".format(version, dirname, condname, muname)
    try:
        os.mkdir(plotdir)
    except OSError:
        pass

    gROOT.SetBatch(True)
    ROOT.gSystem.Load("libPlotUtils.so")
    f = ROOT.TFile.Open("{0}/FitCrossChecks.root".format(cfg._fcc_directory))
    if cfg._is_asimov:
        f.cd("PlotsAfterFitToAsimov")
    else:
        f.cd("PlotsAfterGlobalFit")
    if cfg._is_conditional:
        gDirectory.cd("conditionnal_MuIsEqualTo_{0}".format(mu))
    else:
        gDirectory.cd("unconditionnal")

    p_chi2 = gDirectory.Get("Chi2PerChannel")
    if p_chi2 != None:
        hchi2 = p_chi2.GetListOfPrimitives().At(0)
        c2 = ROOT.TCanvas("c2","c2")
        hchi2.Draw()
        for ext in cfg.formats:
            c2.Print("{1}/Chi2PerChannel.{0}".format(ext, plotdir))

    p_nuis = gDirectory.Get("can_NuisPara_{0}{1}{2}".format(dirname, condname, muname))
    p_corr = gDirectory.Get("can_CorrMatrix_{0}{1}{2}".format(dirname, condname, muname))

    # play with canvas of NP
    h = p_nuis.GetListOfPrimitives().At(0)
    axis = h.GetXaxis()
    g_2s = p_nuis.GetListOfPrimitives().At(1)
    g_1s = p_nuis.GetListOfPrimitives().At(2)
    nuis = p_nuis.GetListOfPrimitives().At(3)
    # first try to reproduce the existing plot
    #nuis_plot("origin", nuis.Clone(), g_2s.Clone(), g_2s.Clone(), h.Clone())
    # then sort the NP
    # NOTE: this #include statement is needed for ROOT v6 
    #       (ROOT v5 is fine without it)
    gROOT.ProcessLine("#include \"include/plotUtils.hpp\"")
    # function pointers not supported in PyROOT... have to workaround
    gROOT.ProcessLine("n = (TGraph*)"+str(ROOT.AddressOf(nuis)[0]))
    gROOT.ProcessLine("a = (TAxis*)"+str(ROOT.AddressOf(axis)[0]))
    gROOT.ProcessLine("PU::sortTGraphAxis(n, a, true, PU::comp_sysNames)")
    #nuis_plot("sorted", nuis.Clone(), g_2s.Clone(), g_2s.Clone(), h.Clone())
    # then plot only interesting things
    res = reduce_all(nuis, g_2s, g_1s, axis, excludes=vector_TString(cfg.exclude_str))
    nuis_plot(cfg, plotdir, "all", *res)
    nuis = res[0]
    g_2s = res[1]
    g_1s = res[2]
    axis = nuis.GetXaxis().Clone()
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("SysJetE", "SysJetF", "SysJetM",
    #                                                                  "SysBJet", "SysJetBE"))
    #    nuis_plot(plotdir, "Jet1", *res)
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("SysJetN", "SysJetP", "SysJVF",
    #                                                                   "METReso", "METTrig"))
    #    nuis_plot(plotdir, "Jet2", *res)
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("TruthTagDR", "BTagB"))
    #    nuis_plot(plotdir, "BTagB", *res)
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("BTagC"))
    #    nuis_plot(plotdir, "BTagC", *res)
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("BTagL"))
    #    nuis_plot(plotdir, "BTagL", *res)
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("Elec", "Muon","Lep"))
    #    nuis_plot(plotdir, "Lepton", *res)
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("norm"))
    #    nuis_plot(plotdir, "Norm1", *res)
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("Norm","Ratio"))
    #    nuis_plot(plotdir, "Norm2", *res)
    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("DPhi", "WMbb", "WbbMbb", "ZMbb", "ZPt", "WPt"))
    #    nuis_plot(plotdir, "ModelWZ", *res)

    #    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("VVJet", "VVMbb", "TopP", "Ttbar", "ttbarHigh", "TChan",
    #                                                                 "WtChan", "SChan"))
    #    nuis_plot(plotdir, "ModelTopVV", *res)
    #res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("norm"))
    #nuis_plot(plotdir, "norm", *res, hmin=0, hmax=2.5)
    #res = reduce_all(nuis, g_2s, g_1s, axis, excludes=vector_TString("SysJetE", "SysJetF", "SysJetM", "SysBJet",
    #                                                              "SysJetBE", "SysJetN",
    #                                                              "SysJetP", "SysJVF", "MET",
    #                                                              "TruthTagDR", "BTag", "norm", "Norm",
    #                                                              "Ratio", "Elec", "Muon", "Lep", "DPhi", "WMbb",
    #                                                              "WbbMbb", "ZMbb", "ZPt", "WPt", "VVJet",
    #                                                              "VVMbb", "TopP", "Ttbar", "ttbarHigh", "TChan", "WtChan",
    #                                                              "SChan"))
    #nuis_plot(plotdir, "Rest", *res)

    # New clasification
    for key in cfg.cov_classification:
        name = key
        zero = cfg.cov_classification[key][0]
        inc_args = cfg.cov_classification[key][1]
        exc_args = cfg.cov_classification[key][2]

        if zero:
            tmp = shift_to_zero(nuis, axis)
        else:
            tmp = nuis

        if len(inc_args) > 0 and len(exc_args) > 0:
            res = reduce_all(tmp, g_2s, g_1s, axis, includes=vector_TString(*inc_args), excludes=vector_TString(*exc_args))
        elif len(inc_args) > 0:
            res = reduce_all(tmp, g_2s, g_1s, axis, includes=vector_TString(*inc_args))
        elif len(exc_args) > 0:
            res = reduce_all(tmp, g_2s, g_1s, axis, excludes=vector_TString(*exc_args))
        else:
            res = reduce_all(tmp, g_2s, g_1s, axis)

        nuis_plot(cfg, plotdir, name, *res)

    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("SysFT_EFF_Eigen", "SysFT_EFF_extrapolation"))
    # nuis_plot(plotdir, "BTag", *res)
    #
    #
    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("SysWt", "SysTop", "SysTtbar"))
    # nuis_plot(plotdir, "Top", *res)
    #
    #
    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("SysVV", "SysWM","SysZM","SysWD","SysZD","SysWP","SysZP"))
    # nuis_plot(plotdir, "ModelBoson", *res)
    #
    #
    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("Norm","Ratio"))
    # nuis_plot(plotdir, "Norm", *res)
    #
    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("norm"))
    # nuis_plot(plotdir, "norm", *res)
    #
    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("SysMUON","SysEL","SysEG"))
    # nuis_plot(plotdir, "Lepton", *res)
    #
    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("SysJET","FATJET"))
    # nuis_plot(plotdir, "Jet", *res)
    #
    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("SysMET"))
    # nuis_plot(plotdir, "MET", *res)
    #
    #
    # res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString("LUMI"))
    # nuis_plot(plotdir, "LUMI", *res)
    #
    #
    # tmp = shift_to_zero(nuis, axis)
    # res = reduce_all(tmp, g_2s, g_1s, axis, excludes=vector_TString("blablabla"))
    # nuis_plot(plotdir, "Shifted", *res)


    suspicious_NP = []
    suspicious_NP.extend(flag_suspicious_NP(cfg, nuis, axis, .5, .5))
    res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString(*suspicious_NP))
    #res = reduce_all(nuis, g_2s, g_1s, axis, includes=vector_TString(*flag_suspicious_NP(nuis, axis)))
    nuis_plot(cfg, plotdir, "Suspicious", *res)

    # play with correlation matrix
    h2 = p_corr.GetListOfPrimitives().At(0)
    corr_plot(cfg, plotdir, "origin", h2.Clone())
    # function pointers not supported in PyROOT... have to workaround
    gROOT.ProcessLine("c = (TH2*)"+str(ROOT.AddressOf(h2)[0]))
    gROOT.ProcessLine("PU::sortTHAxis(c, true, PU::comp_sysNames)")
    gROOT.ProcessLine("PU::sortTHAxis(c, false, PU::comp_sysNames, true)")
    corr_plot(cfg, plotdir, "sorted", h2.Clone())
    for key in cfg.cov_special:
        name = key
        inc_args = cfg.cov_special[key][0]
        exc_args = cfg.cov_special[key][1]

        if len(inc_args) > 0 and len(exc_args) > 0:
            reduce_and_plot(cfg, plotdir, key, h2, includes=vector_TString(*inc_args), excludes=vector_TString(*exc_args))
        elif len(inc_args) > 0:
            reduce_and_plot(cfg, plotdir, key, h2, includes=vector_TString(*inc_args))
        elif len(exc_args) > 0:
            reduce_and_plot(cfg, plotdir, key, h2, excludes=vector_TString(*exc_args))
        else:
            reduce_and_plot(cfg, plotdir, key, h2)

    # reduce_and_plot(plotdir, "noMCStat", h2, excludes=vector_TString("gamma"))
    # reduce_and_plot(plotdir, "JES", h2, includes=vector_TString("SigX", "norm_", "Jet"))
    # reduce_and_plot(plotdir, "BTag", h2, includes=vector_TString("SigX", "norm_", "BTag"))
    # reduce_and_plot(plotdir, "Mbb", h2, includes=vector_TString("SigX", "norm_", "Mbb"))
    # reduce_and_plot(plotdir, "Modelling", h2, includes=vector_TString("SigX", "norm_", "Norm", "Ratio", "PtBi"))
    # reduce_and_plot(plotdir, "SF", h2, includes=vector_TString("SigX", "norm_"))
    # reduce_and_plot(plotdir, "Norm", h2, includes=vector_TString("3JNorm", "norm_", "Norm", "Ratio"))
    # find all correlations > 25%
    systs = set()
    systs.add("SigX")
    nbins = h2.GetNbinsX()
    hasOneBin = False

    for i in range(1, nbins+1):
        for j in range(1, nbins+1):
            if i+j == nbins+1: # diagonal
                continue
            if abs(h2.GetBinContent(i,j))>0.25:
                hasOneBin = True
                systs.add(h2.GetXaxis().GetBinLabel(i))
                systs.add(h2.GetYaxis().GetBinLabel(j))
    if hasOneBin :
        reduce_and_plot(cfg, plotdir, "HighCorr", h2, includes=vector_TString(*systs))

    #systs = set()
    #systs.add("SigX")
    #nbins = h2.GetNbinsX()
    #for i in range(1, nbins+1):
    #    if not "LUMI" in h2.GetXaxis().GetBinLabel(i):
    #        continue
    #    for j in range(1, nbins+1):
    #        if i+j == nbins+1: # diagonal
    #            continue
    #        if abs(h2.GetBinContent(i,j))>0.1:
    #            systs.add(h2.GetXaxis().GetBinLabel(i))
    #            systs.add(h2.GetYaxis().GetBinLabel(j))
    #reduce_and_plot(plotdir, "Lumi", h2, includes=vector_TString(*systs))

    systsX = set()
    systsX.add("SigX")
    for s in cfg.syst_to_study:
        systsX.add(s)
    systsY = set()
    systsY.add("SigX")
    for s in cfg.syst_to_study:
        systsY.add(s)
    nbins = h2.GetNbinsX()
    for j in range(1, nbins+1):
        found = False
        sysname = h2.GetYaxis().GetBinLabel(j)
        for s in systsY:
            if s in sysname:
                found = True
                break
        if not found:
            continue
        for i in range(1, nbins+1):
            if i+j == nbins+1: # diagonal
                continue
            if abs(h2.GetBinContent(i,j))>0.15:
                systsX.add(h2.GetXaxis().GetBinLabel(i))
    reduce_and_plot_2D(cfg, plotdir, "HighSysts", h2, includesX=vector_TString(*systsX), includesY=vector_TString(*systsY))




def nuis_plot(cfg, plotdir, name, nuis, yellow, green, h=None, hmin=-5, hmax=5):
    c = ROOT.TCanvas("c","c",1000,400)
    if h is not None:
        h.SetMinimum(hmin)
        h.SetMaximum(hmax)
        ya = h.GetYaxis()
        h.Draw()
        nuis.Draw("p")
    else:
        nuis.SetMinimum(hmin)
        nuis.SetMaximum(hmax)
        nuis.Draw("pa")
        ya = nuis.GetYaxis()
    ya.SetTitle("pull")
    ya.SetTitleOffset(.5)
    yellow.SetFillColor(ROOT.kYellow)
    green.SetFillColor(ROOT.kGreen)
    yellow.Draw("f")
    green.Draw("f")
    nuis.Draw("p")
    ROOT.gPad.SetBottomMargin(ROOT.gPad.GetBottomMargin()*1.8)
    ROOT.gPad.SetLeftMargin(ROOT.gPad.GetLeftMargin()*.5)
    ROOT.gPad.SetRightMargin(ROOT.gPad.GetRightMargin()*.3)
    ROOT.gPad.Update()
    for ext in cfg.formats:
        c.Print("{2}/NP_{0}.{1}".format(name, ext, plotdir))

def corr_plot(cfg, plotdir, name, hist):
    c = ROOT.TCanvas("c","c")
    hist.GetXaxis().LabelsOption("v")
    if hist.GetNbinsX() > 0:
        hist.GetXaxis().SetLabelSize(min(1.6/hist.GetNbinsX(), 0.03))
    if hist.GetNbinsY() > 0:
        hist.GetYaxis().SetLabelSize(min(1.4/hist.GetNbinsY(), 0.03))
    purge_label_names(hist.GetXaxis())
    purge_label_names(hist.GetYaxis())
    hist.Draw("colz")
    ROOT.gPad.SetBottomMargin(ROOT.gPad.GetBottomMargin()*1.4)
    ROOT.gPad.SetLeftMargin(ROOT.gPad.GetLeftMargin()*1.2)
    ROOT.gPad.Update()
    for ext in cfg.formats:
        c.Print("{2}/corr_{0}.{1}".format(name, ext, plotdir))

def purge_label_names(axis):
    for i in range(1, axis.GetNbins()+1):
        label = axis.GetBinLabel(i)
        axis.SetBinLabel(i, label.replace("alpha_","").replace("ATLAS_","").replace("Sys",""))

def reduce_all(nuis, yellow, green, axis, excludes=None, includes=None):
    if excludes is not None:
        new_nuis = ROOT.PU.reduceTGraphAxisExclude(nuis, axis, True, excludes)
    elif includes is not None:
        new_nuis = ROOT.PU.reduceTGraphAxisInclude(nuis, axis, True, includes)
    max_axis = new_nuis.GetXaxis().GetXmax()
    new_y = yellow.Clone(yellow.GetName()+"_reduced")
    new_g = green.Clone(yellow.GetName()+"_reduced")
    ROOT.PU.removeTGraphPointsAbove(new_y, True, max_axis)
    ROOT.PU.removeTGraphPointsAbove(new_g, True, max_axis)
    return [new_nuis, new_y, new_g]

def shift_to_zero(nuis, axis):
    return ROOT.PU.shiftTGraphToZero(nuis, axis, True)

def reduce_and_plot(cfg, plotdir, name, hist, excludes=None, includes=None):
    if excludes is not None:
        tmp_h = ROOT.PU.reduceTHAxisExclude(hist, True, excludes)
        final_h = ROOT.PU.reduceTHAxisExclude(tmp_h, False, excludes)
    elif includes is not None:
        tmp_h = ROOT.PU.reduceTHAxisInclude(hist, True, includes)
        final_h = ROOT.PU.reduceTHAxisInclude(tmp_h, False, includes)
    corr_plot(cfg, plotdir, name, final_h)

def reduce_and_plot_2D(cfg, plotdir, name, hist, excludesX=None, includesX=None, excludesY=None, includesY=None):
    if excludesX is not None:
        tmp_h = ROOT.PU.reduceTHAxisExclude(hist, True, excludesX)
    elif includesX is not None:
        tmp_h = ROOT.PU.reduceTHAxisInclude(hist, True, includesX)
    if excludesY is not None:
        final_h = ROOT.PU.reduceTHAxisExclude(tmp_h, False, excludesY)
    elif includesY is not None:
        final_h = ROOT.PU.reduceTHAxisInclude(tmp_h, False, includesY)
    corr_plot(cfg, plotdir, name, final_h)

def flag_suspicious_NP(cfg, nuis, axis, thresh_y=.5, thresh_err=.5):
    names = []
    x = ROOT.Double()
    y = ROOT.Double()
    for i in range(nuis.GetN()):
        err = nuis.GetErrorY(i)
        nuis.GetPoint(i, x, y)
        label = axis.GetBinLabel(axis.FindBin(x))
        flag = False
        for syst in cfg.suspicious_syst:
            if label.startswith(syst): flag = True
        if flag or abs(y)>thresh_y or err<thresh_err:
            names.append(label)
            #print label, y, err
    return names

def vector_TString(*names):
    res = ROOT.std.vector(ROOT.TString)()
    for n in names:
        res.push_back(n)
    return res

# def init():
#     if len(sys.argv)<=1:
#         print "You must provide a workspace version number !"
#         return
#
#     ROOT.gROOT.LoadMacro("$WORKDIR/macros/AtlasStyle.C")
#     ROOT.SetAtlasStyle(0.03)
#     ROOT.gStyle.SetPadRightMargin(0.12)
#
#     version = sys.argv[1]
#     directory = "fccs/FitCrossChecks_{0}_combined".format(version)
#     print "Running makeReducedDiagPlots for workspace", version
#     if len(sys.argv)>2:
#         algnums = sys.argv[2]
#         for algnum in algnums.split(','):
#             alg = runFitCrossCheck.available_algs[int(algnum)]
#             mu = int(alg[1])
#             is_conditional = False
#             if alg[3] == "true":
#                 is_conditional = True
#             is_asimov = False
#             if "Asimov" in alg[0]:
#                 is_asimov = True
#             print "Running for conditional ?", is_conditional, "for asimov ?", is_asimov, "What mu ?", mu
#             main(version, directory, is_conditional, is_asimov, mu)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Create covariance matrix plots from a given workspace.')
    parser.add_argument('workspace', help = 'workspace/{name}/{something}/{mass}.root -> pass {name}')
    parser.add_argument('-p', '--plot_modes', default = '0',
                        help = """Comma-separated list of FCC options:
    2: unconditional ( start with mu=1 )
    4: conditional mu = 0
    5: conditional mu = 1
    6: run Asimov mu = 1 toys: randomize Poisson term
    7: unconditional fit to asimov where asimov is built with mu=1
    8: unconditional fit to asimov where asimov is built with mu=0
    9: conditional fit to asimov where asimov is built with mu=1
    10: conditional fit to asimov where asimov is built with mu=0""", dest = 'mode')
    parser.add_argument('-f', '--fitres', help = "", default = None, dest = 'fitres')
    parser.add_argument('pass_to_user', nargs = argparse.REMAINDER, default = [])
    args = parser.parse_args()

    cfg = analysisPlottingConfig.Config(args.pass_to_user)


    ROOT.gROOT.LoadMacro("$WORKDIR/macros/AtlasStyle.C")
    ROOT.SetAtlasStyle(0.03)
    ROOT.gStyle.SetPadRightMargin(0.12)
    ROOT.gSystem.Load("libPlotUtils.so")

    version = args.workspace
    modes = [int(s) for s in args.mode.split(',')]

    fitres = args.fitres
    if fitres is None:
        fitres = version

    if os.path.sep in fitres:
        fcc = fitres
    else:
        fcc = "fccs/FitCrossChecks_"+fitres+"_combined"

    cfg._fcc_directory = fcc

    # NOTE: need to declare these variables out here since whenever multiple 
    #       sets of of plots are created C++ could complain of multiple 
    #       declarations
    gROOT.ProcessLine("TGraph* n;")
    gROOT.ProcessLine("TAxis* a;")
    gROOT.ProcessLine("TH2* c;")

    print "Running makeReducedDiagPlots for workspace ", version
    for algnum in modes:
        alg = runFitCrossCheck.available_algs[int(algnum)]
        mu = int(alg[1])
        is_conditional = False
        if alg[3] == "true":
            is_conditional = True
        is_asimov = False
        if "Asimov" in alg[0]:
            is_asimov = True
        cfg._is_conditional = is_conditional
        cfg._is_asimov = is_asimov
        cfg._mu = mu
        print "Running for conditional? ", is_conditional, " for asimov? ", is_asimov, " what mu? ", mu
        main(cfg, version)
