#!/usr/bin/env python

import sys
import os
import AnalysisMgr_HH as Mgr
import BatchMgr as Batch

# what goes in SplitInputs

#InputVersion = "HH181115LTT
#InputVersion = "HH270416"
#InputVersion = "11821" 
#InputVersion = "11822" 
#InputVersion = "11823" 

InputVersion = "" 


def add_config(mass, unc = "Systs", mcstats=True, one_bin=False, usePseudoData=False, replicaCount=0):
    conf = Mgr.WorkspaceConfig_HH(unc, InputVersion=InputVersion, MassPoint=mass, OneBin=one_bin,
                                  UseStatSystematics=mcstats, Debug=True,
                                  UsePseudoData=False, ReplicaCount=0) 

    #conf.set_channels(channel)
    conf.set_regions()
    #fullversion = "HHbbWW_13TeV_" + outversion + "_" + unc + "_" + mass # BBT, May 17 2016
    fullversion = "HH_13TeV_" + outversion + "_" + unc + "_" + mass
    return conf.write_configs(fullversion)

if __name__ == "__main__":

    if len(sys.argv) is not 4:
        print "Usage: Analysis_HH.py <InputVersion> <outversion> <massPoint>"
        exit(0)

    InputVersion = sys.argv[1]
    outversion   = sys.argv[2]
    masses       = [int(sys.argv[3])]

    # For running on existing workspaces (-k option):
    #WSlink = "/afs/cern.ch/user/g/gwilliam/workspace/WSMaker/workspaces/MonoH0708152p.180815_MonoH_13TeV_180815_Systs_Scalar_2000/combined/2000.root" 
    #os.system("ls -lh " + WSlink)

    ########## create config files

    print "Adding config files..."
    configfiles_stat = []
    configfiles = []
    #masses = [260, 300, 500, 600, 700, 800, 900, 1000] 
    #masses = [250, 700, 2000] 
    #masses = [250, 300, 400, 500, 600, 700, 750, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750, 3000, 5000] 
    #Aug 04
    #masses = [500, 600, 700, 750, 800, 900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750, 3000] 

    for m in masses:
        #configfiles_stat += add_config(str(m), "FloatOnly", one_bin=True) #"StatOnly") #FloatONly manes let floating samples float
        #configfiles_stat += add_config(str(m), "FloatOnly", one_bin=False, usePseudoData=True, replicaCount=2) #try some pseudoData
        #configfiles_stat += add_config(str(m), "FloatOnly") #"StatOnly") #FloatONly manes let floating samples float
        #configfiles_stat += add_config(str(m), "StatOnly")
        #configfiles      += add_config(str(m), "Systs", one_bin=False, mcstats=False)
        configfiles_stat  += add_config(str(m), "Systs", one_bin=True, mcstats=True)
        
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

    #tasks += ["-l", "2,{MassPoint}"]         # limit (1 means expected limit, 0 runs expected and observed)
    #tasks += ["-l", "1,{MassPoint}"]         # limit (1 means expected limit, 0 runs expected and observed)
    tasks += ["-l", "0,{MassPoint}"]         # limit (1 means expected limit, 0 runs expected and observed)
    #tasks += ["--fcc", "0,2,7", "{MassPoint}", "asmiovData"]  # fitCrossChecks # produce pull plots, rankings, etc
    #tasks += ["--fcc", "0,2,7@{MassPoint}"]  # fitCrossChecks # produce pull plots, rankings, etc
    #tasks += ["--fcc", "0,8,11,12,13,14@{MassPoint}"]  # fitCrossChecks # produce pull plots, rankings, etc
    #####tasks += ["--fcc", "doNLLscan,0,8,10,11@{MassPoint}"]  # fitCrossChecks # produce pull plots, rankings, etc
    tasks += ["--fcc", "2,8,10,15,16@{MassPoint}"]  # fitCrossChecks # produce pull plots, rankings, etc
    #tasks += ["--fcc", "doNLLscan,0,10@{MassPoint}"]  # fitCrossChecks # produce pull plots, rankings, etc
    #tasks += ["--fcc", "2@{MassPoint}"]  # fitCrossChecks # produce pull plots, rankings, etc
    #tasks += ["--fcc", "0,2,6,7,9,11@{MassPoint}"]  # fitCrossChecks # produce pull plots, rankings, etc
    #tasks += ["-s", "7,2,{MassPoint}"]         # significance
    #tasks += ["-s",  "0@{MassPoint}"]         # significance
    #tasks += ["-u", "2,7{MassPoint}"]         # breakdown of muhat errors stat/syst/theory
    tasks += ["-u", "0@{MassPoint}"]         # breakdown of muhat errors stat/syst/theory
    # requires fitCrossChecks:
    #tasks += ["-m", "7,2"]     # reduced diag plots (quick)
    #tasks += ["-r", "{MassPoint}", "-n", "{MassPoint}"]   # NP ranking
    #     tasks += ["-b", "2"]       # unfolded b-tag plots (quick)
    #tasks += ["-p", "2,0@{MassPoint}"]       # post-fit plots (CPU intense)
    #tasks += ["-t", "1@{MassPoint}"]     # yield tables (CPU intense)
    
    ########## submit jobs

    # in case you want to run something locally
    Batch.run_local_batch(configfiles_stat, outversion, tasks)
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
