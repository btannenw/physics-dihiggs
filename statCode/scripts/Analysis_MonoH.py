#!/usr/bin/env python

import sys
import os
import AnalysisMgr_MonoH as Mgr
import BatchMgr as Batch

InputVersion = "MonoH170715b"
InputVersion = "MonoH070815"
InputVersion = "MonoH0708152p"
InputVersion = "MonoH091115"
InputVersion = "MonoH20151222a"
InputVersion = "MonoH080116"

def add_config(mass, dm, mediator, unc = "Systs", merged_only=False, one_bin=False,mcstats=True, inclusive=True, floatHFOnlyForZ=False, floatHFOnlyForW=False, sam=False,
               OneLep=True, TwoLep=True, EMu=True, debug=False):
    conf = Mgr.WorkspaceConfig_MonoH(unc, InputVersion=InputVersion, MassPoint=mass, DMType=dm, MediatorMass=mediator, OneBin=one_bin,
                                     UseStatSystematics=mcstats, FloatHFOnlyForZ=floatHFOnlyForZ, FloatHFOnlyForW=floatHFOnlyForW, Debug=debug,
                                     DoShapePlots=True,DoSystsPlots=True) #,DeleteNormFiles=False)
    #conf.set_channels(channel)
    conf.set_regions(merged_only=merged_only, inclusive=inclusive, sam=sam, one_lepton_cr=OneLep, two_lepton_cr=TwoLep, top_emu_cr=EMu)
    fullversion = "MonoH_13TeV_" + outversion + "_" + unc + "_" + dm + "_" + mediator + "_" + mass
    return conf.write_configs(fullversion)

if __name__ == "__main__":

    if len(sys.argv) is not 2:
        print "Usage: Analysis_MonoH.py <outversion>"
        exit(0)

    outversion = sys.argv[1]


    # For running on existing workspaces (-k option):
    #WSlink = "/afs/cern.ch/user/g/gwilliam/workspace/WSMaker/workspaces/MonoH0708152p.180815_MonoH_13TeV_180815_Systs_Scalar_2000/combined/2000.root" 
    #os.system("ls -lh " + WSlink)

    ########## create config files

    print "Adding config files..."
    configfiles_stat = []
    configfiles = []
    dmmasses = [1]
    medmasses = [2000]
    dmtypes = ["Scalar"] #, "Fermion", "2HDM"]

    for m in dmmasses:
        for mm in medmasses:        
            for dm in dmtypes:
                configfiles_stat += add_config(str(m), dm, str(mm), "StatOnly")
                configfiles      += add_config(str(m), dm, str(mm), "Systs", sam=False, one_bin=False, mcstats=False, merged_only=False, inclusive=True,
                                               floatHFOnlyForZ=True, floatHFOnlyForW=True, OneLep=True, TwoLep=True, EMu=True, debug=True)

    for fi in configfiles_stat:
        print fi
    for fi in configfiles:
        print fi

    #sys.exit()

    ########## define tasks

    tasks = ["-w"]             # create workspace (do always)
    try:
        tasks += ["-k", WSlink]    # link workspace instead of creating one (requires -w)
    except NameError:
        pass
    tasks += ["-l", "1,{MassPoint}"]         # limit
#     tasks += ["-s", 0]         # significance
    tasks += ["--fcc", "2,7@{MassPoint}"]  # fitCrossChecks
#     tasks += ["-u", 0]         # breakdown of muhat errors stat/syst/theory
#     # requires fitCrossChecks:
#     tasks += ["-m", "7,2"]     # reduced diag plots (quick)
#     tasks += ["-b", "2"]       # unfolded b-tag plots (quick)
#     tasks += ["-p", "2,0"]       # post-fit plots (CPU intense)
#     tasks += ["-t", "0,2"]     # yield tables (CPU intense)
    
    ########## submit jobs

    # in case you want to run something locally
    Batch.run_local_batch(configfiles, outversion, tasks)
    sys.exit()
    #Batch.run_local_batch(configfiles_stat, outversion, ["-w", "-l", 1])

    # adjust to recover failed ranking subjobs
    redoSubJob = -1

    if redoSubJob < 0:
        print "Submitting stat only jobs..."
        Batch.run_lxplus_batch(configfiles_stat, outversion, ["-w", "-l", 1,"-s",1], '2nd')
        print "Submitting systs jobs..."
        Batch.run_lxplus_batch(configfiles, outversion, tasks, '2nd')
    print "Submitting rankings..."
    Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-r", "125"], '2nd', jobs=20, subJob=redoSubJob)
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "-r", "125"], '2nd', jobs=20, subJob=redoSubJob)

    ########## non-default stuff. Warning: don't submit rankings / toys / NLL scans with the same outversion and nJobs!
    #print "Submitting toys..."
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "--fcc", "6"], '2nd', jobs=50, subJob=redoSubJob)
    #print "Submitting NLL scans..."
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "--fcc", "7,2,doNLLscan"], '2nd', jobs=50, subJob=redoSubJob)
