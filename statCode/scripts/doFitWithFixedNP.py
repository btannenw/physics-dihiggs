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

# read a pre-existing roofitresults 
# loop on the NP values of a ws and fix them to the roofitresults values/uncertainties
# rerun mu-hat free fit

# Spyros: Added mass as argument
def main(cfg, version, mass, doSum):
    """ main function

    Parameters:
    * version: version of the workspace which we want to make plots with
    * directory: where to find FitCrossChecks.root that contains the RooFitResult
    * is_conditional, is_asimov, mu: describe the type of fit chosen for the FitResult
    """

    ws, rfr, suffix, g, binDir = initialize(cfg, version, mass)

    cfg._main_suffix = suffix
    ws.Print()
    fitPDF(cfg, ws, rfr)

    g.Close()
    cfg._reset()

# NEEDS TO UPDATE MACRO LAUNCHING:
# GET ONE WS NAME, THEN ONE ROOFITRES NAME

def initialize(cfg, version, mass):
    RooMsgService.instance().setGlobalKillBelow(RF.ERROR)
    gROOT.SetBatch(True)
    gSystem.Load("libPlotUtils.so")

    # GET WORKSPACE
    ws,g = getWorkspace(version, mass)

    # GET FIT NPS SETUP FROM FIT RESULTS
    logging.info("Preparing NP to the requested values")
    rfr, suffix = getFitResult(cfg)
    transferResults(cfg, ws, rfr)

    # GetBinning
    binHist = getBinningDir(g)

    return ws, rfr, suffix, g, binHist

def getBinningDir(f):
    binHist = f.GetDirectory('binning')
    if not binHist:
        binHist = None
    return binHist

def fitPDF(cfg, ws, rfr):
    # Prepare input arguments
    print '%s' % ws.Print()
    mc = ws.obj("ModelConfig")
    print '%s' % mc.Print()
    simPdf = mc.GetPdf()
    print '%s' % simPdf.Print()
    fitdata = ws.data("obsData")
    print '%s' % fitdata.Print()
    glbObs = mc.GetGlobalObservables()
    print '%s' % glbObs.Print()

    # Create NLL
    #nCPU_str = environ['NCORE']
    #nCPU = int(nCPU_str)
    nll = simPdf.createNLL(fitdata, RF.GlobalObservables(glbObs), RF.Offset(1), RF.NumCPU(3), RF.Optimize(2) )
    nllval = nll.getVal()
    print 'Starting NLL value %s' % nllval 

    # Minimization setup
    minim = RooMinimizer(nll)
    strategy = ROOT.Math.MinimizerOptions.DefaultStrategy()
    minim.setStrategy( strategy )
    tol =  ROOT.Math.MinimizerOptions.DefaultTolerance()
    tol = min(tol,0.1)
    # epsilon gets multiplied by e-3 somewhere
    minim.setEps( tol )
    minim.setEps( 1 )
    status = -1

    optConstFlag = 2
    minim.optimizeConst(optConstFlag)

    sw = TStopwatch()
    sw.Start()

    minimizer = ROOT.Math.MinimizerOptions.DefaultMinimizerType()
    minimizer = 'Minuit2'
    algorithm = ROOT.Math.MinimizerOptions.DefaultMinimizerAlgo()

    maxCalls = ROOT.Math.MinimizerOptions.DefaultMaxFunctionCalls()
    print 'MAX CALLS %s\t%s' % (maxCalls, ROOT.Math.MinimizerOptions.DefaultMaxFunctionCalls())
    print 'MAX Interations %s' % ROOT.Math.MinimizerOptions.DefaultMaxIterations()

    print 'FitPDF'
    print '\t minimizer %s' % minimizer
    print '\t algorithm %s' % algorithm
    print '\t strategy  %s' % strategy
    print '\t tolerance %s' % tol

    # MINIMIZING
    status = minim.minimize(minimizer, algorithm)
    print 'Minimize Status : %s' % status

    # PRINT OUT SOME RESULTS
    tmpResult = minim.save()
    covarMat = tmpResult.covarianceMatrix()
    det = covarMat.Determinant()
    print 'Determinant %f' % det
    if(det < 0): 
        print 'ATTENTION: Determinant negative'
    parFinal = tmpResult.floatParsFinal()

    # GET EIGENVECTORS AND EIGENVALUES
    eigenValueMaker = TMatrixDSymEigen(covarMat)
    eigenValues   = eigenValueMaker.GetEigenValues()
    eigenVectors  = eigenValueMaker.GetEigenVectors()
    print 'Eigenvalues:  '
    for l in range(0, eigenValues.GetNrows()):
        print '\t %d\t%lf' % (l, eigenValues[l])

    if (status%100 is 0):
        print 'Calling Hesse ...'
        status = minim.hesse()
        print 'Hesse Status : %d' % status

    # STOPWATCH
    sw.Print()
    sw.Stop()

    # PRINTING FINAL RESULTS OVERVIEW
    r = minim.save()
    print 'Final POI parameters'
    mc.GetParametersOfInterest().Print("v");

    print 'FINAL NLL = %lf' % nll.getVal()

    poi = mc.GetParametersOfInterest().first()
    print 'PRINTING FIT RESULT %d\t%lf' % (poi.isConstant(), poi.getVal())
    r.Print()

    sw.Print()

    return

def getWorkspace(version, mass):
    wsf = os.path.join("workspaces", str(version), "combined", str(mass))
    wsf += ".root"
    g = TFile.Open(wsf)
    if not g.IsOpen():
        raise RuntimeError("Couldn't find file {0}".format(wsf))
    ws = g.combined
    return ws,g


def transferResults(cfg, ws, rfr):
    """ Transfer results from the RooFitResult to the workspace, and make relevant snapshots """
    mc = ws.obj("ModelConfig")
    np = RooArgSet(mc.GetNuisanceParameters(), mc.GetParametersOfInterest())
    removeGamma(np)
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
    np_iterator = np.createIterator()
    inp = np_iterator.Next()
    while inp:
        print ' setting parameter %s to constant' % inp.GetName()
        inp.setConstant(kTRUE)
        inp = np_iterator.Next()
    # MU set to 1
    #force_mu, mu_val = cfg.force_mu_value()
    #if force_mu:
    #    mc.GetParametersOfInterest().first().setVal(mu_val)
    mc.GetParametersOfInterest().first().setVal(1) # CROSS CHECK
    mc.GetParametersOfInterest().first().setConstant(kFALSE)
    #cfg._muhat = mc.GetParametersOfInterest().first().getVal()
    if logging.getLogger().isEnabledFor(logging.DEBUG):
        logging.debug("Expanded RooFit results")
        np.Print("v")
    ws.saveSnapshot("vars_final", np)
    ws.loadSnapshot("vars_final")


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
    parser.add_argument('-p', '--fit_modes', default = '0',
                        help = """Comma-separated list of plots to create:
    0: prefit plots
    2: unconditional fit (start with mu=1)
    5: conditional mu=1 fit""", dest = 'mode')
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
        elif mode == 2:
            logging.info("Doing unconditional fit (start with mu=1)")
            cfg._is_conditional = False
            cfg._is_asimov = False
            cfg._mu = 1
            cfg._main_is_prefit = False
            try:
                main(cfg, wsname, mass, doSum)
            except TypeError, te:
                logging.critical(str(te))
                print TypeError
        elif mode == 5:
            logging.info("Doing conditional mu = 1 fit")
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
