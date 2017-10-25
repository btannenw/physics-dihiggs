#!/usr/bin/env/python

import os
import os.path
import re
import sys
import math
from array import array
import argparse
import copy
import logging

from ROOT import *
from ROOT import RooFit as RF
import plotMaker as mkplots
import analysisPlottingConfig
gROOT.ProcessLine("#include \"include/roofitUtils.hpp\"")

# TODO support toying with several workspaces
# TODO check for memleaks etc...
# TODO reorganize this script into sections:
#       dealing with / moving in workspaces
#       dealing with yields and everythgin that is integrated
#       plot retrieving and making
# FIXME script can't run with varios options and single option is required.
#       if options config: 0,1 => option 0 crash at the end producing the plots. option 1 => do not produce plots
#       if options config: 1,0 => option 1 produce the plots. option 0 plots are equal to option 1
#       O_o
#       option 0 crash at the end needs to be fix


# Spyros: Added mass as argument
def main(cfg, version, mass, doSum):
    """ main function

    Parameters:
    * version: version of the workspace which we want to make plots with
    * directory: where to find FitCrossChecks.root that contains the RooFitResult
    * is_conditional, is_asimov, mu: describe the type of fit chosen for the FitResult
    """

    ws, rfr, suffix, plotdir, g, binDir = initialize(cfg, version, mass)
    save_h = False

    cfg._main_suffix = suffix
    cfg._main_plotdir = plotdir
    cfg._main_save_hists = save_h

    logging.info("Now start plotting")
    if doSum: makePlotsSums(cfg, ws, rfr, mass)
    else: makePlots(cfg, ws, rfr, mass, restrict_to = cfg.restrict_to, excludes = cfg.excludes, bin_dir = binDir)
    for plotFunc in cfg.additionalPlots:
        plotFunc(cfg, ws, rfr, mass)
    logging.info("Plots made. Now exiting")
    cfg._save_plot_objs()
    cfg._reset()
    g.Close()


# Spyros: Added mass as argument
def initialize(cfg, version, mass):
    RooMsgService.instance().setGlobalKillBelow(RF.ERROR)
    gROOT.SetBatch(True)
    gSystem.Load("libPlotUtils.so")

    ws,g = getWorkspace(version, mass)

    logging.info("Preparing NP to the requested values")
    # Make postfit plots
    if not cfg._main_is_prefit:
        rfr, suffix = getFitResult(cfg)
        transferResults(cfg, ws, rfr)
        plotdir = "plots/{0}/postfit".format(version)

    # Make prefit plots
    else:
        plotdir = "plots/{0}/prefit".format(version)
        rfr = getInitialFitRes(cfg, ws)
        ws.loadSnapshot("vars_initial")
        suffix = "Prefit"

    # GetBinning
    binHist = getBinningDir(g)

    os.system("mkdir -vp "+plotdir)
    cfg._yieldsfile = os.path.join(plotdir, "Yields_{0}.yik".format(suffix))
    cfg._plot_objs_file = os.path.join(plotdir, "plotobjs_{0}.yik".format(suffix))

    return ws, rfr, suffix, plotdir, g, binHist


def getWorkspace(version, mass):
    wsf = os.path.join("workspaces", str(version), "combined", str(mass))
    wsf += ".root"
    print "$$$$$$: ",wsf
    g = TFile.Open(wsf)
    if not g.IsOpen():
        raise RuntimeError("Couldn't find file {0}".format(wsf))
    ws = g.combined
    return ws,g


def getInitialFitRes(cfg, ws):
    """ Create a prefit RooExpandedFitResult for this workspace """
    mc = ws.obj("ModelConfig")
    force_mu, mu_val = cfg.force_mu_value()
    if force_mu:
        mc.GetParametersOfInterest().first().setVal(mu_val)
    np = RooArgList(mc.GetNuisanceParameters())
    np.add(mc.GetParametersOfInterest())
    #removeGamma(np)
    it = np.createIterator()
    n = it.Next()
    while n:
        if "alpha" in n.GetName():
            n.setError(1)
        else:
            n.setError(1.e-4)
        #if "alpha_SysJetEResol" in n.GetName():
            #n.setVal(-1)
        n = it.Next()
    ws.saveSnapshot("vars_initial", RooArgSet(np))
    re = RooExpandedFitResult(np)
    cfg._muhat = mc.GetParametersOfInterest().first().getVal()
    if logging.getLogger().isEnabledFor(logging.DEBUG):
        logging.debug("Expanded RooFit results")
        re.Print("v")
    return re


def transferResults(cfg, ws, rfr):
    """ Transfer results from the RooFitResult to the workspace, and make relevant snapshots """
    mc = ws.obj("ModelConfig")
    np = RooArgSet(mc.GetNuisanceParameters(), mc.GetParametersOfInterest())
    if logging.getLogger().isEnabledFor(logging.DEBUG):
        logging.debug("Expanded RooFit results")
        np.Print("v")
    # we want to be sure that the snapshot contains all we need, including POI
    ws.saveSnapshot("vars_initial", np)
    fpf = rfr.floatParsFinal().Clone()
    cp = rfr.constPars()
    fpf.add(cp) # add all const parameters of the RooFitResult to the floating ones
    if logging.getLogger().isEnabledFor(logging.DEBUG):
        logging.debug("Expanded RooFit results")
        fpf.Print()
    np.assignValueOnly(fpf) # only sets the central value. Should be ok as VisualizeError uses errors from the RooFitResult, according to ROOT doc
    force_mu, mu_val = cfg.force_mu_value()
    if force_mu:
        # is_higgs_fit = True
        # it = np.createIterator()
        # n = it.Next()
        # while n:
        #     if cfg.transferResults_fitName in n.GetName():
        #         n.setVal(0)
        #         is_higgs_fit = False
        #         break
        #     n = it.Next()
        # cfg._muhat = 1
        # if is_higgs_fit:
        #     mc.GetParametersOfInterest().first().setVal(1)

        # cfg._muhat = mu_val
        mc.GetParametersOfInterest().first().setVal(mu_val)
    # else:
    #     cfg._muhat = mc.GetParametersOfInterest().first().getVal()
    cfg._muhat = mc.GetParametersOfInterest().first().getVal()
    if logging.getLogger().isEnabledFor(logging.DEBUG):
        logging.debug("Expanded RooFit results")
        np.Print("v")
    ws.saveSnapshot("vars_final", np)
    ws.loadSnapshot("vars_final")


# TODO: delete?
def removeGamma(arglist):
    to_remove = RooArgList()
    it = arglist.createIterator()
    n = it.Next()
    while n:
        if n.GetName().startswith("gamma"):
            to_remove.add(n)
        n = it.Next()
    print "Will remove"
    to_remove.Print()
    arglist.remove(to_remove)


def getFitResult(cfg):
    """ Go and fetch RooFitResult in a FitCrossChecks.root file """

    f = TFile.Open("{0}/FitCrossChecks.root".format(cfg._fcc_directory))
    if not f.IsOpen():
        raise RuntimeError("Couldn't find file {0}".format("{0}/FitCrossChecks.root".format(cfg._fcc_directory)))
    if cfg._is_asimov:
        f.cd("PlotsAfterFitToAsimov")
    else:
        f.cd("PlotsAfterGlobalFit")
    if cfg._is_conditional:
        gDirectory.cd("conditionnal_MuIsEqualTo_{0}".format(cfg._mu))
    else:
        gDirectory.cd("unconditionnal")
    rfr = gDirectory.Get("fitResult").Clone()
    # bonus: get a suffix corresponding to the setup
    suffix = getPostfitSuffix(cfg)
    f.Close()
    return rfr, suffix


def getComponents(cfg, w):
    """ Fetch all components (data, MC pdfs...) for a given workspace
    Organize all of this in a map
    """
    mc = w.obj("ModelConfig")
    data = w.data("obsData")
    simPdf = w.pdf("simPdf")
    channelCat = simPdf.indexCat()
    chanName = channelCat.GetName()

    comps = {}

    for tt in categories(channelCat):
        ttname = tt.GetName()
        pdftmp  = simPdf.getPdf( ttname )
        datatmp = data.reduce("{0}=={1}::{2}".format(chanName,chanName,ttname))
        obs  = pdftmp.getObservables( mc.GetObservables() ).first()
        obs_set = RooArgSet(obs)
        binWidth = pdftmp.getVariables().find("binWidth_obs_x_{0}_0".format(ttname))
        logging.debug("    Bin Width : {}".format(binWidth.getVal()))
        comps[ttname] = [obs, binWidth, datatmp, {}, pdftmp]

        binning = obs.getBinning()
        stepsize = binning.averageBinWidth()
        low = binning.lowBound()
        high = binning.highBound()
        m = low
        while m<(high-1e-6):
            obs.setRange("bin"+str(m), m, m+stepsize)
            m += stepsize

        bkgList = RooArgList()
        sigList = RooArgList()
        totList = RooArgList()
        for c in components(pdftmp, ttname):
            compname = c.GetName()
            has_mass, res = cfg.is_signal(compname)
            if has_mass:
                sigList.add(c)
            else:
                bkgList.add(c)
            totList.add(c)
            comps[ttname][3][compname] = c
        sigSum = RooAddition( "Signal", "sig_Sum", sigList)
        comps[ttname][3]["Signal"] = sigSum
        bkgSum = RooAddition( "Bkg", "bkg_Sum", bkgList)
        comps[ttname][3]["Bkg"] = bkgSum
        totSum = RooAddition( "MC", "tot_Sum", totList)
        comps[ttname][3]["MC"] = totSum

    return comps


def getYields(cfg, w, rfr=None, onlyTot=False, window=None):
    """ Give map of yields (each category and total) for current snapshot of workspace
    If RooFitResult is given, the errors are computed
    If onlyTot = True, error is computed only on the sum of MC
    """
    if cfg._comps is None:
        cfg._comps = getComponents(cfg, w)

    comps = cfg._comps

    if window:
        logging.info("Will compute weights in window {}".format(window))

    cfg._read_yields()
    yields = cfg._yields
    if len(cfg._yields)>0 and rfr is None:
        return cfg._yields

    if onlyTot:
        comp_rfr = None
    else:
        comp_rfr = rfr
    for ttname,comptt in comps.iteritems():
        logging.info("    Computing yields for category : {}".format(ttname))
        obs_set = RooArgSet(comptt[0])
        yieldsChan = {}
        if window:
            rname = comptt[0].GetName()+"_range"
            comptt[0].setRange(rname, window[0], window[1])
            yieldsChan["data"] = comptt[2].sumEntries("1", rname)
        else:
            yieldsChan["data"] = comptt[2].sumEntries()

        for compname, comp in comptt[3].iteritems():
            if compname != "MC":
                yieldsChan[compname] = getValueAndError(cfg, comptt[0], comp, comptt[1], comp_rfr, ttname, window)
            else:
                yieldsChan[compname] = getValueAndError(cfg, comptt[0], comp, comptt[1], rfr, ttname, window)
            if compname == "Signal" and cfg._muhat:
                yieldsChan["SignalExpected"] = [y/cfg._muhat for y in yieldsChan[compname]]

        yieldsChan["S/B"] = yieldsChan["Signal"][0] / yieldsChan["Bkg"][0]
        yieldsChan["S/sqrt(S+B)"] = yieldsChan["Signal"][0] / (yieldsChan["Bkg"][0]+yieldsChan["Signal"][0])**.5
        yields[ttname] = yieldsChan
    cfg._yields = yields
    cfg._save_yields()

    return yields


def getValueAndError(cfg, obs, comp, binWidth, rfr, ttname, window=None):
    """ Try to be clever and not re-compute something that has already been computed """
    obs_set = RooArgSet(obs)
    compname = comp.GetName()
    bwidth = binWidth.getVal()
    compInt = None
    if ttname in cfg._yields and compname in cfg._yields[ttname]:
        Ntemp = cfg._yields[ttname][compname][0]
    else:
        if window:
            obs.setRange("myrange", window[0], window[1])
            compInt = comp.createIntegral(obs_set, RF.Range("myrange"))
        else:
            compInt = comp.createIntegral(obs_set)
        Ntemp=compInt.getVal() * bwidth
    error = -1
    if rfr:
        if ttname in cfg._yields and compname in cfg._yields[ttname] \
           and cfg._yields[ttname][compname][1] != -1:
            error = cfg._yields[ttname][compname][1]
        else:
            if not compInt:
                if window:
                    obs.setRange("myrange", window[0], window[1])
                    compInt = comp.createIntegral(obs_set, RF.Range("myrange"))
                else:
                    compInt = comp.createIntegral(obs_set)
            error = RU.getPropagatedError(compInt, rfr) * bwidth
    logging.info("Found {} +/- {} for {} in region {}".format(Ntemp, error, compname, ttname))
    return [Ntemp, error]


# TODO: delete or move
def getSumAndError(list_comps, rfr, window=None):
    """ list_comps: list of tuples (obs, bwidth, comp)
    """
    complist = RooArgList()
    widthlist = RooArgList()
    logging.debug("{}".format(list_comps))
    for l in list_comps:
        if window:
            l[0].setRange("myrange", window[0], window[1])
            inte = l[2].createIntegral(RooArgSet(l[0]), RF.Range("myrange"))
        else:
            inte = l[2].createIntegral(RooArgSet(l[0]))
        complist.add(inte)
        widthlist.add(RF.RooConst(l[1]))
    roosum = RooAddition("sum", "sum", complist, widthlist)
    val = roosum.getVal()
    if rfr is not None:
        error = RU.getPropagatedError(roosum, rfr)
    else:
        error = -1
    return [val, error]


def makePlots(cfg, w, rfr, mass, restrict_to=[], excludes=[], bin_dir=None):
    """ Plot distributions for each subchannel """

    is_prefit = cfg._main_is_prefit
    suffix = cfg._main_suffix
    plotdir = cfg._main_plotdir
    save_hists = cfg._main_save_hists

    os.system("mkdir -vp "+plotdir)
    objs_dict = getAllPlotObjects(cfg, w, rfr, is_prefit, suffix, plotdir, restrict_to, excludes)

    logging.info("Plotting Distributions for each subchannel")

    for ttname, objs in objs_dict.iteritems():
        if ttname.endswith("error"):
            continue
        plot(cfg, objs, ttname, mass, plot_bkgsub = False, bin_dir=bin_dir)
        #if 'mBB' in ttname: # extra mBB hists with bkg subtraction
         #   print ttname
          #  plot(cfg, objs, ttname, mass, plot_bkgsub = True, bin_dir=bin_dir)

    cfg._save_yields()
    # TODO: move this to the cfg.additionalPlots
    #if makeVpT:
    #    makepTbinsPlots(w, rfr, is_prefit, suffix, plotdir, cfg._yields, save_hists)
    logging.info("End plotting distributions!")


# TODO: delete or move
def makepTbinsPlots(w, rfr, is_prefit, suffix, plotdir, yields = None, save_hists = False):
    """ Plot VpT distributions in each tag region """

    logging.info("Plotting VpT Distributions")
    if is_prefit:
        w.loadSnapshot("vars_initial")
    else:
        w.loadSnapshot("vars_final")

    if not yields:
        logging.debug("Yields not provided. Compute them")
        getYields(w, rfr, True)
        yields = Config.yields

    os.system("mkdir -vp "+plotdir)

    ptbins_cut = array('d', [0, 90, 120, 160, 200, 250])
    hmodel_cut = TH1F("hmodel_cut", "hmodel_cut", 5, ptbins_cut)
    errmodel_x_cut = array('d', [0, 90, 90, 120, 120, 160, 160, 200, 200, 250,
                             250, 200, 200, 160, 160, 120, 120, 90, 90, 0])
    errmodel_y_cut = array('d', [0 for i in range(20)])
    errmodel_cut = TGraph(20, errmodel_x_cut, errmodel_y_cut)

    ptbins_mva = array('d', [0, 120, 250])
    hmodel_mva = TH1F("hmodel_mva", "hmodel_mva", 2, ptbins_mva)
    errmodel_x_mva = array('d', [0, 120, 120, 250, 250, 120, 120, 0])
    errmodel_y_mva = array('d', [0 for i in range(8)])
    errmodel_mva = TGraph(8, errmodel_x_mva, errmodel_y_mva)

    ptbins_cut_0lep = array('d', [0, 100, 120, 160, 200, 250])
    hmodel_cut_0lep = TH1F("hmodel_cut_0lep", "hmodel_cut_0lep", 5, ptbins_cut_0lep)
    errmodel_x_cut_0lep = array('d', [0, 100, 100, 120, 120, 160, 160, 200, 200, 250,
                             250, 200, 200, 160, 160, 120, 120, 100, 100, 0])
    errmodel_y_cut_0lep = array('d', [0 for i in range(20)])
    errmodel_cut_0lep = TGraph(20, errmodel_x_cut_0lep, errmodel_y_cut_0lep)

    ptbins_mva_0lep = array('d', [0, 100, 120, 250])
    hmodel_mva_0lep = TH1F("hmodel_mva_0lep", "hmodel_mva_0lep", 3, ptbins_mva_0lep)
    errmodel_x_mva_0lep = array('d', [0, 100, 100, 120, 120, 250, 250, 120, 120, 100, 100, 0])
    errmodel_y_mva_0lep = array('d', [0 for i in range(12)])
    errmodel_mva_0lep = TGraph(12, errmodel_x_mva_0lep, errmodel_y_mva_0lep)

    histos = {}
    mass = None

    for k,y in yields.iteritems():
        parts = k.split('_')
        # k: Region_Y2012_isMVA1_B2_J2_T2_L2_distmva_TType
        # find the bin:
        pos1 = k.find("_B")
        pos2 = k.find("_", pos1+1)
        if pos2 - pos1 != 3:
            logging.error("name of the region {} does not seem to be a standard one".format(k))
            logging.error("_B should match the bin name")
            return
        # TODO: horrible. Need to take care of 0 lepton low MET bin properly, especially in MVA case where lowMET is still CUT
        regname = k[:pos1+2]+'9'+k[pos2:]
        bin = int(k[pos1+2])
        pos1 = regname.find("_dist")
        pos2 = regname.find("_", pos1+1)
        regname = regname[:pos1+5]+"VpT"+regname[pos2:]
        pos1 = regname.find("_isMVA")
        isMVA = int(regname[pos1+6])
        pos1 = k.find("_L")
        pos2 = k.find("_", pos1+1)
        nlep = int(k[pos1+2])
        logging.info("Accumulating for region {}".format(regname))
        ibin = bin + 1
        if isMVA and nlep != 0 and ibin == 3: # MVA has no B1
            ibin = 2
        if isMVA:
            if nlep == 0:
                hmodel = hmodel_mva_0lep
                errmodel = errmodel_mva_0lep
            else:
                hmodel = hmodel_mva
                errmodel = errmodel_mva
        else:
            if nlep == 0:
                hmodel = hmodel_cut_0lep
                errmodel = errmodel_cut_0lep
            else:
                hmodel = hmodel_cut
                errmodel = errmodel_cut
        if not regname in histos:
            histos[regname] = {}
            for s, v in y.iteritems():
                if s.startswith("L_x"):
                    sname = getCompName(s)
                    histos[regname][sname] = hmodel.Clone(sname)
                    if mass == None:
                        res = is_signal(sname)
                        if res:
                            mass = res
            histos[regname]["data"] = hmodel.Clone("data")
            histos[regname]["error"] = errmodel.Clone("error")
            histos[regname]["prefit"] = [hmodel.Clone("prefit"), ""]
        # now, fill the histos
        for s, v in y.iteritems():
            if s.startswith("L_x"):
                sname = getCompName(s)
                try:
                    histos[regname][getCompName(s)].SetBinContent(ibin, v[0])
                except KeyError: # case when a given component is 0 in the first found VpT bin, but !=0 elsewhere
                    sname = getCompName(s)
                    histos[regname][sname] = hmodel.Clone(sname)
                    histos[regname][sname].SetBinContent(ibin, v[0])
        histos[regname]["data"].SetBinContent(ibin, y["data"])
        (mcval, mcerr) = y["MC"]
        if mcerr == -1: # recompute case by case if not already computed somewhere
            mcerr = getMCerror(w, rfr, k)
            y["MC"] = [mcval, mcerr]
        X = Double()
        Y = Double()
        for i in [2*ibin-2, 2*ibin-1]:
            histos[regname]["error"].GetPoint(i, X, Y)
            histos[regname]["error"].SetPoint(i, X, mcval - mcerr)
        npoints = histos[regname]["error"].GetN()
        for i in [npoints+1-2*ibin, npoints-2*ibin]:
            histos[regname]["error"].GetPoint(i, X, Y)
            histos[regname]["error"].SetPoint(i, X, mcval + mcerr)
        # if doing postfit, add the prefit line
        if not is_prefit:
            histo, mulegend = getPrefitCurve(w, regname = k)
            histos[regname]["prefit"][0].SetBinContent(ibin, histo.Integral())
            print "PREFIT ", mulegend
            logging.info("PREFIT {}".format(mulegend))
            histos[regname]["prefit"][1] = mulegend

    for reg,h in histos.iteritems():
        h["data"] = TGraphAsymmErrors(h["data"]) # TODO use Poisson errors ?
        sm = mkplots.SetupMaker(reg, mass, muhat = Config.muhat)
        for k,v in h.iteritems():
            sm.add(k, v)

        cname = reg +"_"+suffix
        can = sm.setup.make_complete_plot(cname, True, ybounds=(0.85, 1.15))
        plotname = "{0}/{1}".format(plotdir, can.GetName())
        can2 = sm.setup.make_complete_plot(cname+"_logy", True, True, ybounds=(0.85, 1.15))
        plotname2 = "{0}/{1}".format(plotdir, can2.GetName())
        for f in Config.formats:
            can.Print(plotname+'.'+f)
            can2.Print(plotname2+'.'+f)

        # save histograms if requested
        if save_hists:
            afile = TFile.Open(plotname+".root", "recreate")
            can.Write(can.GetName())
            can2.Write(can2.GetName())
            for k,v in h.iteritems():
                if isinstance(v, TObject):
                    v.Write(v.GetName())
                if k == "prefit":
                    v[0].Write(v[0].GetName())

        can.Close()
        can2.Close()
        # free memory for some objects used in the plotting...
        mkplots.purge()
        # TODO check for things not deleted
    Config.save_yields()


# TODO: Elisabeth's stuff
def plotSumOfCats(cfg, rfr, mass, suffix, plotdir, save_hists, list_objs, list_comps, name, weights=None):
# def plotSumOfCats(rfr, suffix, plotdir, save_hists, list_objs, list_comps, name, weights=None):
    if len(list_objs)>0:
        res = sumPlotObjects(list_objs, weights)
        error_name = name+"_error"
        # print "JWH: error name = {}".format(error_name)
        # print "JWH: objs names = {}".format(cfg._plot_objs.keys())
        if error_name in cfg._plot_objs:
            # print "JWH: found in cfg._plot_objs"
            res["error"] = cfg._plot_objs[error_name]
        else:
            #print "JWH: did not find in cfg._plot_objs"
            res["error"] = getSumErrorBand(list_comps, rfr, weights)
            cfg._plot_objs[error_name] = res["error"]
            # print res["error"]
            # for i in range(res["error"].GetN()):
            #     # print "(x,y,err_y) = ({},{},{})".format(res["error"].GetX()[i], res["error"].GetY()[i], res["error"].GetEY()[i])
            #     print "JWH: res[\"error\"](x,y) = ({},{})".format(res["error"].GetX()[i], res["error"].GetY()[i])

        plot(cfg, res, name, mass, False)
        if 'mBB' in name:
            plot(cfg, res, name, mass, True) 
    # if len(list_objs)>0:
    #     res = sumPlotObjects(list_objs, weights)
    #     error_name = name+"_error"
    #     if error_name in Config.plot_objs:
    #         res["error"] = Config.plot_objs[error_name]
    #     else:
    #         res["error"] = getSumErrorBand(list_comps, rfr, weights)
    #         Config.plot_objs[error_name] = res["error"]
    #     plot(res, name, suffix, plotdir, save_hists)


# TODO: Elisabeth's stuff
def makeWeightedSumPlot(cfg, rfr, mass, suffix, plotdir, save_hists, objs_dict, yields, comps, name, restrict_to=[], exclude=[]):
# def makeWeightedSumPlot(rfr, suffix, plotdir, save_hists, objs_dict, yields, comps, name, restrict_to=[], exclude=[]):
    """ restrict_to is to be understand with AND """
    list_tot = []
    comps_tot = []
    weights_tot = []
    weights_dib_tot = []
    sob_rabbit_hole=False
    for k,v in objs_dict.iteritems():
        if "error" in k:
            continue
        if len(restrict_to)>0:
            if False in (r in k for r in restrict_to):
                continue
        if True in (r in k for r in exclude):
            continue
        s_o_b = abs(yields[k]["Signal"][0]) / abs(yields[k]["Bkg"][0])
        diboson = 0
        for r in yields[k]:
            if "WZ" in r or "ZZ" in r or "VZ" in r or "Diboson" in r or "diboson" in r:
                diboson += yields[k][r][0]
        if diboson:
            d_o_b = abs(diboson) / abs(yields[k]["Bkg"][0])
        else:
            d_o_b = 0
        list_tot.append(v)
        comps_tot.append(comps[k])
        weights_tot.append(s_o_b)
        weights_dib_tot.append(d_o_b)
        print k,' (S/B) weight=',s_o_b
    for wlist in [ weights_tot, weights_dib_tot]:
        sum_list = math.fsum(wlist)
        for w in wlist:
            w /= sum_list


    # print weights_tot
    # print weights_dib_tot

    plotSumOfCats(cfg, rfr, mass, suffix, plotdir, save_hists, list_tot, comps_tot, name)
    plotSumOfCats(cfg, rfr, mass, suffix, plotdir, save_hists, list_tot, comps_tot, name+"_Higgsweighted", weights_tot)
#    plotSumOfCats(cfg, rfr, mass, plotdir, save_hists, list_tot, comps_tot, name+"_Dibosonweighted", weights_dib_tot) # To-do: figure this out


# TODO: Elisabeth's stuff
def makePlotsSums(cfg, w, rfr, mass) : #is_prefit, suffix, plotdir, save_hists = False):
# def makePlotsSums(w, rfr, is_prefit, suffix, plotdir, save_hists = False):

    is_prefit = cfg._main_is_prefit
    suffix = cfg._main_suffix
    plotdir = cfg._main_plotdir
    save_hists = cfg._main_save_hists

    os.system("mkdir -vp "+plotdir)
    objs_dict = getAllPlotObjects(cfg, w, rfr, is_prefit, suffix, plotdir)
    logging.info("got all plot objects for sum plots")
    yields = cfg._yields
    if cfg._comps is None:
        cfg._comps = getComponents(cfg, w)
    comps = cfg._comps

    logging.info("Plotting summed Distributions")

    # resulting name, restrict-to list, exclude-any list
    plottingFunc = lambda name, rt, ea : makeWeightedSumPlot(cfg, rfr, mass,
                                                             suffix, plotdir,
                                                             save_hists,
                                                             objs_dict, yields,
                                                             comps, name = name,
                                                             restrict_to = rt,
                                                             exclude = ea)

    cfg.make_sum_plots(plottingFunc)

    cfg._save_yields()
    logging.info("End plotting summed distributions!")


# TODO: delete or move
def makePlotsSumsCR(w, rfr, is_prefit, suffix, plotdir, save_hists = False):
    """ Plot distributions for each subchannel, summing 7 and 8 TeV """

    os.system("mkdir -vp "+plotdir)
    objs_dict = getAllPlotObjects(w, None, is_prefit, suffix, plotdir, restrict_to=["_T0_", "_T1_", "Spctop"])

    if Config.comps is None:
        getComponents(w)
    comps = Config.comps

    print "Plotting summed Distributions of backgrounds"

    # FIXME update with new conventions. Low priority since we are not updating 2011
    for flav in ["Zero", "One", "Two"]:
        for reg in ["0T2J", "0T3J", "1T2J", "1T3J", "topcr", "topemucr"]:
            list_cr =[]
            list_comps = []
            for k,v in objs_dict.iteritems():
                if k.endswith("error"):
                    continue
                if reg in k and flav in k:
                    list_cr.append(v)
                    list_comps.append(comps[k])

            if len(list_cr)>0:
                name = "{0}Lepton_{1}_B9_both_mjj".format(flav, reg)
                res = sumPlotObjects(list_cr)
                error_name = name+"_error"
                if error_name in Config.plot_objs:
                    res["error"] = Config.plot_objs[error_name]
                else:
                    res["error"] = getSumErrorBand(list_comps, rfr)
                    Config.plot_objs[error_name] = res["error"]
                plot(res, name, suffix, plotdir, save_hists, plot_bkgsub = False,
                    ybounds = (0.76, 1.24))

    Config.save_yields()
    print "End plotting distributions !"


def getPlotObjects(cfg, w, obs, pdf, data, components, ttyields, fitres=None, objs={}):
    frm = obs.frame()

    for comp in components:
        compname = comp.GetName()
        if compname in objs:
            continue
        h = comp.createHistogram(compname, obs)
        if h.Integral()!=0:
            h.Scale(1/h.Integral())
            if compname in ttyields:
                h.Scale(ttyields[compname][0])
        objs[compname] = h
        if not "mass" in objs:
            has_mass, res = cfg.is_signal(compname)
            if has_mass:
                objs["mass"] = res

    if not "data" in objs:
        data.plotOn(frm,RF.DataError(RooAbsData.Poisson))
        tgdata = frm.getHist()
        objs["data"] = tgdata
        #pdfSum.plotOn(frm)
        pdf.plotOn(frm, RF.Normalization(1, RooAbsReal.RelativeExpected))
        chi2 = frm.chiSquare()
        objs["chi2"] = chi2

    if fitres is not None and "error" not in objs:
        data.plotOn(frm,RF.DataError(RooAbsData.Poisson))
        mc = w.obj("ModelConfig")
        prev_mu = mc.GetParametersOfInterest().first().getVal()
        mc.GetParametersOfInterest().first().setVal(0)
        pdf.plotOn(frm, RF.VisualizeError(fitres,1), RF.Name("FitError_AfterFit"),
                   RF.Normalization(1, RooAbsReal.RelativeExpected))
        mc.GetParametersOfInterest().first().setVal(prev_mu)
        c = frm.getCurve()
        objs["error"] = c

    return objs


# TODO: Elisabeth's stuff
def sumPlotObjects(objs_list, weights=None):
    res = {}
    aux = {}
    if weights is None:
        weights = [1]*len(objs_list)
    if len(weights) != len(objs_list):
        logging.error("Numbers of regions and weights given do not match!")
        return

    for objs,w in zip(objs_list, weights):
        for k,v in objs.iteritems():
            if k == "error" or k == "chi2": # meaningless to sum them
                continue
            short = getCompName(k)
            if not short in aux:
                aux[short] = k
                # pay attention not to modify the objects we are working with. They might be reused elsewhere.
                if isinstance(v, TObject):
                    res[k] = v.Clone()
                    if isinstance(v, TGraphAsymmErrors): # data
                        scaleTGraphAsymmErrors(res[k], w)
                    else:
                        res[k].Scale(w)
                elif k == "prefit":
                    res[k] = [v[0].Clone(), v[1]]
                    res[k][0].Scale(w)
                else:
                    res[k] = v
            else:
                idx = aux[short]
                if short != k: # that's one standard component
                    res[idx].Add(v, w)
                elif k == "prefit":
                    res[idx][0].Add(v[0], w)
                elif k == "data":
                    if w == 1:
                        res[idx] = RooHist(res[idx], v)
                    else:
                        res[idx] = RooHist(res[idx], v, 1, w, RooAbsData.SumW2)
    return res


# TODO: Elisabeth's stuff
def getSumErrorBand(comps_list, rfr, weights=None):
    if rfr is None:
        return None
    if weights is None:
        weights = [1]*len(comps_list)
    if len(weights) != len(comps_list):
        logging.error("Numbers of regions and weights given do not match!")
        return

    #v_pdfs = vector('RooAbsPdf*')()
    #v_obs = vector('RooRealVar*')()
    #v_weights = vector(Double)()
    #v_data = vector('RooAbsData*')()

    #print "Summing:"
    #for c,w in zip(comps_list, weights):
        #print c[4]
        #v_obs.push_back(c[0])
        #v_weights.push_back(w)
        #v_pdfs.push_back(c[4])
        #v_data.push_back(c[2])

    #curve = RU.plotOnWithErrorBand(v_pdfs, v_obs, v_weights, rfr, v_data)

    v_pdfs = []
    v_obs = []
    v_bw = []
    #print "Summing:"
    for c in comps_list:
        v_pdfs.append(c[3]["MC"])
        v_obs.append(c[0])
        v_bw.append(c[1])
    curve = getBinByBinErrorBand(v_pdfs, v_obs, v_bw, rfr, weights)

    return curve


# TODO: Elisabeth's stuff
def getBinByBinErrorBand(mc_comps, observables, binWidths, rfr, weights=None):
    if rfr is None:
        return None
    bins = []
    binning = observables[0].getBinning()
    stepsize = binning.averageBinWidth()
    low = binning.lowBound()
    high = binning.highBound()
    real_weights = RooArgList()
    if weights is not None:
        for bw, w in zip(binWidths, weights):
            real_weights.add(RF.RooConst(bw.getVal()*w))
    else:
        for bw in binWidths:
            real_weights.add(RF.RooConst(bw.getVal()))
    m = low
    while m < high - 1e-6:
        rname = "bin"+str(m)
        intes = RooArgList()
        for obs, mc in zip(observables, mc_comps):
            intes.add(mc.createIntegral(RooArgSet(obs), RF.Range(rname)))
        totbin = RooAddition("sumbin", "sumbin", intes, real_weights)
        val = totbin.getVal()
        err = RU.getPropagatedError(totbin, rfr)
        bins.append((val, err))
        logging.debug("Found error of {}".format(err))
        m += stepsize
    yvals = []
    xvals = []
    for i,b in enumerate(bins):
        yvals.extend([b[0]+b[1], b[0]+b[1]])
        xvals.extend([binning.binLow(i), binning.binLow(i)])
    xvals.append(high)
    xvals = xvals[1:]
    xvals.extend(reversed(xvals))
    for b in reversed(bins):
        yvals.extend([b[0]-b[1], b[0]-b[1]])
    # # FIXME JWH
    # for x,y in zip(xvals,yvals):
    #     print "(x,y) = ({},{})".format(x,y)
    yvals_a = array('d', yvals)
    xvals_a = array('d', xvals)
    curve = TGraph(len(xvals), xvals_a, yvals_a)
    return curve


def getPrefitCurve(cfg, w, obs=None, pdf=None, regname=None):
    w.loadSnapshot("vars_initial")
    mc = w.obj("ModelConfig")
    mc.GetParametersOfInterest().first().setVal( 0 );
    mubeforefit = mc.GetParametersOfInterest().first().getVal()
    muValueBeforeFitLegend = "Pre-fit background"
    if regname is not None:
        simPdf = w.pdf("simPdf")
        pdf  = simPdf.getPdf( regname )
        obs  = pdf.getObservables( mc.GetObservables() ).first()
        preFitIntegral = pdf.expectedEvents(RooArgSet(obs))
        h = pdf.createHistogram(pdf.GetName(), obs)
        h.Scale(preFitIntegral/h.Integral())
    else:
        preFitIntegral = pdf.expectedEvents(RooArgSet(obs))
        h = pdf.createHistogram(pdf.GetName(), obs)
        h.Scale(preFitIntegral/h.Integral())
    w.loadSnapshot("vars_final")
    return h, muValueBeforeFitLegend


def getCompName(name):
    if name.startswith("L_x_"):
        return name.split('_')[2]
    return name


# TODO: Elisabeth's stuff
def categories(chanCat):
    it = chanCat.typeIterator()
    tt = it.Next()
    while tt:
        yield tt
        tt = it.Next()


def components(pdf, ttname):
    modelName1 = ttname + "_model"
    pdfmodel1 = pdf.getComponents().find(modelName1)
    funcList1 =  pdfmodel1.funcList()
    funcIter1 = funcList1.iterator()
    comp1 = funcIter1.Next()
    while comp1 != None:
        yield comp1
        comp1 = funcIter1.Next()


def getPostfitSuffix(cfg):

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
    #mu = str(mu)
    postfit_suffix = "{0}{1}{2}".format(dirname, condname, muname)
    return postfit_suffix


# TODO: delete, move, or refactor
def getMCerror(w, rfr, regname):
    if Config.comps is None:
        getComponents(w)
    comps = Config.comps
    obs  = comps[regname][0]
    binWidth = comps[regname][1]
    MC = comps[regname][3]["MC"]
    val, error = getValueAndError(obs, MC, binWidth, rfr, regname)
    return error


def getAllPlotObjects(cfg, w, rfr, is_prefit, suffix, plotdir, restrict_to=[], excludes=[]):
    """ returns a map of plot objects, indexed by category name """
    if is_prefit:
        logging.info("Loading initial snapshot")
        w.loadSnapshot("vars_initial")
    else:
        logging.info("Loading final snapshot")
        w.loadSnapshot("vars_final")
    getYields(cfg, w)
    yields = cfg._yields
    if cfg._comps is None:
        cfg._comps = getComponents(cfg, w)
    comps = cfg._comps
    if cfg._plot_objs is None:
        cfg._read_plot_objs()
    objs_dict = cfg._plot_objs

    logging.info("Getting Distributions for each subchannel")

    for ttname, comptt in comps.iteritems():
        if len(restrict_to)>0:
            if not True in (r in ttname for r in restrict_to):
                continue
        if len(excludes)>0:
            if True in (r in ttname for r in excludes):
                continue
        objs = objs_dict.get(ttname, {})

        logging.info("Gathering plot primitives for region {}".format(ttname))
        pdftmp  = comptt[4]
        datatmp = comptt[2]
        obs  = comptt[0]

        individual_comps = {k:v for k,v in comptt[3].iteritems()
                            if k!="MC" and k!="Signal" and k!="Bkg"
                            and k!="SignalExpected" and k!="S/B"
                            and k!="S/sqrt(S+B)"}
        # objs = getPlotObjects(cfg, obs, pdftmp, datatmp, individual_comps.values(), yields[ttname], rfr, objs)
        # FIXME: JWH test
        objs = getPlotObjects(cfg, w, obs, pdftmp, datatmp, individual_comps.values(), yields[ttname], rfr, objs)
        # if doing postfit, add the prefit line
        if not is_prefit and "prefit" not in objs:
            histo, mulegend = getPrefitCurve(cfg, w, obs, pdftmp)
            objs["prefit"] = [histo, mulegend]

        logging.debug("plot objects for {0} = \n{1}".format(ttname, objs))
        objs_dict[ttname] = objs

        # # FIXME: JWH test
        # if True:
        #     import json
        #     print json.dumps(objs_dict, sort_keys=True, indent=2)

    cfg._save_plot_objs()

    to_remove = []
    if len(restrict_to)>0:
        for k in objs_dict:
            if not True in (r in k for r in restrict_to):
                to_remove.append(k)
            if True in (r in k for r in excludes):
                to_remove.append(k)
    objs_final = copy.deepcopy(objs_dict)
    for k in to_remove:
        objs_final.pop(k)
    return objs_final


# TODO: Elisabet's stuff
def scaleTGraphAsymmErrors(tg, weight):
    yval = tg.GetY()
    errhi = tg.GetEYhigh()
    errlo = tg.GetEYlow()
    for i in range(tg.GetN()):
        yval[i] *= weight
        errhi[i] *= weight
        errlo[i] *= weight


def plot(cfg, objs, ttname, mass, plot_bkgsub=True, ybounds=(0.4, 1.6), bin_dir=None): 
    # Now do the plots
    suffix = cfg._main_suffix
    plotdir = cfg._main_plotdir
    save_hists = cfg._main_save_hists
    sm = mkplots.SetupMaker(cfg, ttname, mass, muhat = cfg._muhat, guess_properties = True, bin_dir=bin_dir)
    #for i in range(objs["error"].GetN()):
        #print "JWH: objs[\"error\"](x,y) = ({},{})".format(objs["error"].GetX()[i], objs["error"].GetY()[i])
    if 'mass' in objs:
        sm.add('mass', objs['mass'])
    for k,v in objs.iteritems():
        logging.info("... {}".format(k))
        sm.add(getCompName(k), v)

    # first, standard plot
    cname = ttname +"_"+suffix
    if ttname.endswith("weighted"):
        can = sm.setup.make_complete_plot(cname, True, ytitle = "Weighted events",ybounds = ybounds)
        canlog = sm.setup.make_complete_plot(cname+'_logy', True,True, ytitle = "Weighted events",ybounds = ybounds)
    else:
        can = sm.setup.make_complete_plot(cname, True, ybounds = ybounds)
        canlog = sm.setup.make_complete_plot(cname+'_logy', True,True, ybounds = ybounds)
    plotname = "{0}/{1}".format(plotdir, can.GetName())

    if plot_bkgsub:
        # then, bkg-subtracted plot
        cname2 = ttname +"_BkgSub_"+suffix
        can2 = sm.setup.make_bkg_substr_plot(cname2)
        plotname2 = "{0}/{1}".format(plotdir, can2.GetName())
    for f in cfg.formats:
        print plotname+'.'+f
        can.Print(plotname+'.'+f)
        canlog.Print(plotname+'log.'+f)
        if plot_bkgsub:
            can2.Print(plotname2+'.'+f)
    # save histograms if requested
    if save_hists:
        afile = TFile.Open(plotname+".root", "recreate")
        can.Write(can.GetName())
        canlog.Write(canlog.GetName())
        if plot_bkgsub:
            can2.Write(can.GetName())
        for k,v in objs.iteritems():
            if isinstance(v, TObject):
                v.Write(v.GetName())
            if k == "prefit":
                v[0].Write(v[0].GetName())
    can.Close()
    if plot_bkgsub:
        can2.Close()
    # free memory for some objects used in the plotting...
    mkplots.purge()
    # TODO check for things not deleted


def getBinningDir(f):
    binHist = f.GetDirectory('binning')
    if not binHist:
        binHist = None
    return binHist


if __name__ == "__main__":

    gROOT.LoadMacro("$WORKDIR/macros/AtlasStyle.C")
    # gSystem.Load("libMatrix.so")
    # gSystem.Load("libRooFit.so")
    gSystem.Load("libPlotUtils.so")
    gROOT.ProcessLine("#include \"include/roofitUtils.hpp\"")
    # ROOT.gROOT.LoadMacro("$WORKDIR/macros/AtlasStyle.C")
    SetAtlasStyle()

    class MyParser(argparse.ArgumentParser):
        def error(self, message=None):
            sys.stderr.write('error: %s\n' % message)
            self.print_help()
            sys.exit(2)

    parser = MyParser(description='Create plots from a given workspace.', formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('workspace', help = 'workspace/{name}/{something}/{mass}.root -> pass {name}')
    parser.add_argument('-m', '--mass', type = int, default = 125,
                        help = 'workspace/{name}/{something}/{mass}.root -> pass {mass}', dest = 'mass')
    parser.add_argument('-p', '--plot_modes', default = '0',
                        help = """Comma-separated list of plots to create:
    0: prefit plots
    1: bkg-only postfit plots
    2: s+b postfit plots
    3: s+b conditional postfit plots""", dest = 'mode')
    parser.add_argument('-f', '--fitres', help = "fit results to use (fcc directory)", default = None, dest = 'fitres')
    parser.add_argument('-s', '--sum', help = "make sum plots",
                        dest = 'sum', action = 'store_true')
    parser.set_defaults(sum = False)
    parser.add_argument('pass_to_user', nargs = argparse.REMAINDER, default = [])
    args = parser.parse_args()

    cfg = analysisPlottingConfig.Config(args.pass_to_user)

    wsname = args.workspace
    modes = [int(s) for s in args.mode.split(',')]

    mass = args.mass
    doSum = args.sum

    fitres = args.fitres
    if fitres is None:
        fitres = wsname

    if os.path.sep in fitres:
        fcc = fitres
    else:
        fcc = "fccs/FitCrossChecks_"+fitres+"_combined"

    cfg._fcc_directory = fcc

    for mode in modes:
        if mode == 0:
            logging.info("Doing prefit plots")
            cfg._main_is_prefit = True
            try:
                main(cfg, wsname, mass, doSum)
            except TypeError, te:
                logging.critical(str(te))
                print TypeError
        elif mode == 1:
            logging.info("Doing bkg-only postfit plots")
            cfg._is_conditional = True
            cfg._is_asimov = False
            cfg._mu = 0
            cfg._main_is_prefit = False
            try:
                main(cfg, wsname, mass, doSum)
            except TypeError, te:
                logging.critical(str(te))
                print TypeError
        elif mode == 2:
            logging.info("Doing s+b postfit plots")
            cfg._is_asimov = False
            cfg._mu = 1
            cfg._main_is_prefit = False
            try:
                main(cfg, wsname, mass, doSum)
            except TypeError, te:
                logging.critical(str(te))
                print TypeError
        elif mode == 3:
            logging.info("Doing bkg-only postfit plots")
            cfg._is_conditional = True
            cfg._is_asimov = False
            cfg._mu = 1
            cfg._main_is_prefit = False
            try:
                main(cfg, wsname, mass, doSum)
            except TypeError, te:
                logging.critical(str(te))
                print TypeError
        else:
            print "Mode", mode, "is not recognized !"
            logging.warning("Plotting mode {} is not recognized!".format(mode))
