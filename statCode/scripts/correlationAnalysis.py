#!/usr/bin/env python

import sys
import AnalysisMgr as Mgr
import AnalysisMgr_VHbbRun2 as Mgr2
import BatchMgr as Batch

InputVersion = "bootstrap"

def fit_higgs_13TeV(unc ='Systs', channel='0', var2tag='mBB', var1tag='mBB', use1tag=False, use4pjet=False, doptvbin=False, nreplica=10):
    configfiles = []

    InputVersion = "SMVH_mva_v14puw_bootstrapfulldataset"
    signalInjection = 0 # DEFAULT 0: no injection
    mass = str(125)

    baseline_configs = {'UseJet19NP':True,'DoDiboson':False,'FitWHZH':False, 'FitVHVZ':False,'DoSystsPlots':False,"DoShapePlots":False}#, 'DecorrPOI':'L'}
    baseline_configs.update({'InputVersion':InputVersion,'MassPoint':mass,'DoInjection':signalInjection,'SkipUnkownSysts':True})
    baseline_configs.update({'UsePseudoData':True, 'ReplicaCount':nreplica, 'cplxRebinCut':True})

    conf = Mgr2.WorkspaceConfig_VHbbRun2(unc, **baseline_configs)

    if "0" in channel:
        conf.append_regions(var2tag, var1tag, True, False, False,
                            use1tag, use4pjet, doptvbin)
    if "1" in channel:
        conf.append_regions(var2tag, var1tag, False, True, False,
                            use1tag, use4pjet, doptvbin)
    if "2" in channel:
        conf.append_regions(var2tag, var1tag, False, False, True,
                            use1tag, use4pjet, doptvbin)

    if (var2tag is not var1tag) and (use1tag is True) :
        fullversion = "VHbbRun2_13TeV_" + outversion + "_" + channel + "_" + mass + "_" + unc + "_" + "use1tag" + str(use1tag) + "_use4pjet" + str(use4pjet) + "_" + var2tag + "2tag_" + var1tag + "1tag"
    else:
        fullversion = "VHbbRun2_13TeV_" + outversion + "_" + channel + "_" + mass + "_" + unc + "_" + "use1tag" + str(use1tag) + "_use4pjet" + str(use4pjet) + "_" + var2tag

    return conf.write_configs(fullversion)

def fit_higgs_8TeV(unc = "Systs", dist = "CUT", use1tag = True, trafo1 = 6, Zbkg = 10, Zsig = 10, useTopCR = False, useLowMET = False, usemv1c = False, splitb = False, channels = [], nreplica = 10):
    configfiles = []

    conf = Mgr.WorkspacesConfig(unc, "8TeV", version=InputVersion, versionCUT=InputVersion, versionMVA=InputVersion, trafo=trafo1,
                                MVAZbkg = Zbkg, MVAZsig = Zsig, merge = "true", usePseudoData="true", nreplicas = nreplica, cplxRebinCut="true")
    conf.set_channels(*channels)

    regCut = []
    regMVA = []

    if dist == "CUT":
        regCut += ["2T2J", "2T3J"]
        if use1tag:
            regCut += ["1T2J"]
        if use1tag:
            regCut += ["1T3J"]
        if useTopCR:
            regCut += ["topemu"]
    else:
        regMVA += ["2J","3J"]
        if use1tag:
            regMVA += ["1T2J"]
        if use1tag:
            regMVA += ["1T3J"]
        if useTopCR:
            regMVA += ["topemu"]
        if usemv1c:
            regMVA += ["MV1cBTag_1T"]
        if splitb:
            regMVA += ["BTagSplit"]

    if useLowMET:
        regCut += ["lowMET"]

    conf.set_regions(CUT = regCut, MVA = regMVA)

    print "CUT: "+conf["regsCUT"]
    print "MVA: "+conf["regsMVA"]

    shape = "shape"
    regs="2T"
    if use1tag:
        regs += "1T"
    if useTopCR:
        regs += "top"

    fullversion = "higgs_8TeV_correlation_" + outversion + "_" + unc + "_" + dist + "_" + regs + "_" + shape

    configfiles.extend(conf.write_configs(fullversion))
    return configfiles


if __name__ == "__main__":
    outversion = sys.argv[1]
    configfiles = []

    # 8 TEV ANALYSIS
    trafo = 12
    Zbkg = 4.5
    Zsig = 4.5
    #no low met bs yet
    useLowMET = False
    usemv1c = True
    splitb = True
    #channels = ["0"]
    #channels = ["1"]    
    #channels = ["2"]
    channels = ["012"]    
    unc = "Systs"
    useTopCR = True
    use1tag = True
    #nreplica = 1000
    nreplica = 4

    #for dist in ["CUT","MVA"]:
    #    configfiles.extend(fit_higgs_8TeV(unc, dist, use1tag, trafo, Zbkg, Zsig,
    #                                      useTopCR, useLowMET, usemv1c, splitb, channels, nreplica))
    #print configfiles

    #Mgr.run_lxplus_batch(configfiles, outversion, ["-w", "--fcc", "2", "-m", "2"], queue='1nd')


    # SETUP FOR 13 TEV ANALYSIS
    use1tag = False
    use4pjet = False
    doptvbin = False # automatically set to the std setup for mBB and mva
    nreplica = 1
    channels = ["0", "1", "2", "012"]    

    for dist in ["mva","mBB"]:
        for chan in channels:
            configfiles.extend(fit_higgs_13TeV(unc, chan, dist, 'dRBB', use1tag, use4pjet, doptvbin, nreplica)) 

    print configfiles

    Batch.run_local_batch(configfiles, outversion, ["-w", "--fcc", "2", "-m", "2"])
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "--fcc", "2", "-m", "2"], queue='1nh')

