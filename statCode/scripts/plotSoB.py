#!/usr/bin/env python

import os
import sys
import doPlotFromWS
import plotMaker
import ROOT
import math
import analysisPlottingConfig
import logging

def main(version, directory=None, is_conditional=False, is_asimov=False, mu=None):

    cfg = analysisPlottingConfig.Config([])
    cfg._is_conditional = is_conditional
    cfg._is_asimov = is_asimov
    cfg._mu = mu
    cfg._main_is_prefit = (directory is None)
    cfg._fcc_directory = directory

    ws, rfr,  suffix, plotdir, g, binHist = doPlotFromWS.initialize(cfg, version, 125)
    #makeSoBPlot(ws, rfr, is_prefit=is_prefit, suffix=suffix, plotdir=plotdir)

    for ratioType in [0, 1, 2]:
        makeSoBPlot(cfg, ws, rfr, is_prefit=cfg._main_is_prefit, suffix=suffix, plotdir=plotdir, restrict_to=[], ratioType=ratioType)

    print "Plots made. Now exiting"
    g.Close()
    cfg._save_plot_objs()
    cfg._reset()

def makeSoBPlot(cfg, w, rfr, is_prefit, suffix, plotdir, restrict_to=[], ratioType = 0):
    """ Plot distributions for each subchannel """

    os.system("mkdir -vp "+plotdir)
    objs_dict = doPlotFromWS.getAllPlotObjects(cfg, w, rfr, is_prefit, suffix, plotdir, restrict_to)

    print "Plotting Distributions for each subchannel"

    xmin = -4.25
    xmax = 0.75
    xwidth = 0.5
    nbins = (int) ((xmax - xmin + 1e-3) / xwidth)

    yMinRatio = 0.5
    yMaxRatio = 1.9
    if ratioType is not 0:
        yMinRatio = -2.5
        yMaxRatio = 3.5

    hmodel = ROOT.TH1F("h","h",nbins,xmin,xmax)
    hmodel.SetXTitle("log_{10}(S/B)")
    hmodel.SetYTitle("Events / 0.5")

    # need several copies of these hists if we want to superimpose the plots for different mus
    hdata = hmodel.Clone("hdata")
    hdata.Sumw2(False)
    hsig = hmodel.Clone("hsig")
    hbkg = hmodel.Clone("hbkg")
    hsig.Sumw2(True)
    hbkg.Sumw2(True)
    #hsig_exp = hmodel.Clone("hsigexp")
    origin = [[] for i in range(nbins)]
    bkg_list = {}

    for ttname, objs in objs_dict.iteritems():
        if ttname.endswith("error"): # FIXME what's that for already ? looks useless nowadys
            continue
        # Now do the plots
        #sm = plotMaker.SetupMaker(cfg, ttname, 125, muhat = cfg._muhat)
        sm = plotMaker.SetupMaker(cfg, ttname, 0, muhat = cfg._muhat)
        if 'mass' in objs:
            sm.add('mass', objs['mass'])
        for k,v in objs.iteritems():
            print "...", k
            sm.add(doPlotFromWS.getCompName(k), v)
        res = sm.setup.finish_initialize()
        data = sm.setup.data.hist
        sig = sm.setup.sig_to_use.h
        bkg = sm.setup.hsum
        #sig_exp = sm.setup.exp_sig.h
        # complete list of backgrounds
        bkgs = sm.setup.bkgs
        for b in bkgs:
            if not b.name in bkg_list:
                bkg_list[b.name] = hmodel.Clone(b.name)
                bkg_list[b.name].Sumw2(True)

        for i in range(1, data.GetNbinsX()+1):
            if bkg.GetBinContent(i) == 0:
                print "ERROR: 0 background content for some bin of category", ttname
                print "Signal content there is", sig.GetBinContent(i)
                if sig.GetBinContent(i) == 0:
                    continue
                else:
                    raise ZeroDivisionError
            sob = sig.GetBinContent(i) / bkg.GetBinContent(i)
            if sob == 0:
                print "WARNING: signal is 0 in some bin of category", ttname
            else:
                logsob = math.log10(sob)
                # keep track what we put in
                bnumber = hdata.FindFixBin(logsob)
                if bnumber == 15:
                    print "WARNING"
                    print ttname, i
                if bnumber == 0:
                    bnumber = 1
                origin[bnumber-1].append((ttname+"_"+str(i), sob, {}))
                # now fill histograms
                hdata.Fill(logsob, data.GetBinContent(i))
                hsig.Fill(logsob, sig.GetBinContent(i))
                hbkg.Fill(logsob, bkg.GetBinContent(i))
                for b in bkgs:
                    bkg_list[b.name].Fill(logsob, b.h.GetBinContent(i))
                    origin[bnumber-1][-1][2][b.name] = b.h.GetBinContent(i)

    hdata.AddBinContent(1, hdata.GetBinContent(0))
    hsig.AddBinContent(1, hsig.GetBinContent(0))
    hbkg.AddBinContent(1, hbkg.GetBinContent(0))
    for b in bkg_list.values():
        b.AddBinContent(1, b.GetBinContent(0))

    for i in range(hdata.GetNbinsX() + 2):
        hdata.SetBinError(i, hdata.GetBinContent(i)**.5)


    # Do the pretty printing
    cname = "Global_SoverB_2015"
    if ratioType is 1:
        cname = "Global_SoverB_2015_pulls"
    if ratioType is 2:
        cname = "Global_SoverB_2015_mu"
    #plot_setup = plotMaker.SetupMaker(cfg, cname, 125, muhat = 1, guess_properties=False)
    plot_setup = plotMaker.SetupMaker(cfg, cname, 0, muhat = 1, guess_properties=False)
    plot_setup.add("data", hdata)
    plot_setup.add("VH125", hsig) # just cheating to have it accepted as signal
    #plot_setup.add("VZ", hsig) # just cheating to have it accepted as signal
    plot_setup.add("bkg", hbkg)

    can = plot_setup.setup.make_SoB_plot(cname,do_ratio = True, Ratioybounds=(yMinRatio, yMaxRatio), ratioType=ratioType)
    plotname = "{0}/{1}".format(plotdir, can.GetName())
    for f in cfg.formats:
        can.Print(plotname+'.'+f)
    can.Close()
    plotMaker.purge()

    # Do the colorful plot
    cname = "Global_SoverB_2015_details"
    if ratioType is 1:
        cname = "Global_SoverB_2015_details_pulls"
    if ratioType is 2:
        cname = "Global_SoverB_2015_details_mu"
    #plot_setup = plotMaker.SetupMaker(cfg, cname, 125, muhat = 1, guess_properties=False)
    plot_setup = plotMaker.SetupMaker(cfg, cname, 0, muhat = 1, guess_properties=False)
    plot_setup.add("data", hdata)
    plot_setup.add("VH125", hsig) # just cheating to have it accepted as signal
    #plot_setup.add("VZ", hsig) # just cheating to have it accepted as signal
    for k,v in bkg_list.iteritems():
        plot_setup.add(k, v)

    can = plot_setup.setup.make_SoB_plot(cname,do_ratio = True, Ratioybounds=(yMinRatio, yMaxRatio), ratioType=ratioType)
    plotname = "{0}/{1}".format(plotdir, can.GetName())
    for f in cfg.formats:
        can.Print(plotname+'.'+f)
    can.Close()
    plotMaker.purge()

    cfg._save_yields()
    print "End plotting S/B plot !"

    print "Printing origin of bins"
    for i,b in enumerate(origin):
        print "Analysis bins entering bin", i+1, "of the final plot:"
        for s in sorted(b):
            print "   ", s[0]
            print "      S/B:", s[1]
            for k,v in s[2].iteritems():
                print "      {0} : {1:.2f}".format(k, v)

    print "\n\n\n\n\n\n"
    print "Printing total background composition of the final plot"
    for i in range(1, hdata.GetNbinsX()+1):
        print "Composition of bin", i, "of the final plot:"
        for b,h in bkg_list.iteritems():
            print "    {0} : {1:.3f}".format(b, h.GetBinContent(i))
        bkgval = hbkg.GetBinContent(i)
        sigval = hsig.GetBinContent(i)
        print "    {0} : {1:.3f}".format("Background", bkgval)
        print "    {0} : {1:.3f}".format("Signal", sigval)
        print "    {0} : {1:d}".format("Data", int(hdata.GetBinContent(i)))
        if bkgval > 0:
            sob = sigval / bkgval
        else:
            sob = 0
        print "    {0} : {1:.3g}".format("S/B", sob)


if __name__ == "__main__":

    ROOT.gROOT.LoadMacro("$WORKDIR/macros/AtlasStyle.C") 
    ROOT.SetAtlasStyle()

    wsname = sys.argv[1]
    modes = [int(s) for s in sys.argv[2].split(',')]
    # modes:
    # 0 is prefit
    # 1 is bkg-only fit
    # 2 is S+B fit
    if len(sys.argv)>3:
        fitres = sys.argv[3]
    else:
        fitres = wsname
    fcc = "fccs/FitCrossChecks_"+fitres+"_combined"

    logging.basicConfig(level=logging.DEBUG)

    for mode in modes:
        if mode == 0:
            print "Doing S/B prefit plot"
            main(wsname)
        elif mode == 1:
            print "Doing S/B bkg-only postfit plot"
            main(wsname, fcc, is_conditional=True, is_asimov=False, mu=0)
        elif mode == 2:
            print "Doing S/B s+b postfit plot"
            main(wsname, fcc, is_conditional=False, is_asimov=False, mu=1)
        elif mode == 3:
            print "Doing S/B mu=1 postfit plot"
            main(wsname, fcc, is_conditional=True, is_asimov=False, mu=1)
        else:
            print "Mode", mode, "is not recognized !"


