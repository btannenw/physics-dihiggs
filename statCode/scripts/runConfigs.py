#!/usr/bin/env python

import sys
import os
import BatchMgr as Mgr


if __name__ == "__main__":

    if len(sys.argv) < 3:
        print "Usage: runConfigs.py outversion config1,config2,... [mass]"
        exit(0)

    outversion = sys.argv[1]
    configfiles = sys.argv[2].split(',')
    
    if len(sys.argv)>3 : 
        mass = sys.argv[3]
    else :
        mass="125"

    print "Config files:"
    for fi in configfiles:
        print fi
        
    ########## define tasks

    tasks = ["-w"]             # create workspace (do always)
    #tasks += ["-k", WSlink]    # link workspace instead of creating one (requires -w)
    tasks += ["-l", "1,"+mass]         # limit (1: exp, 0: obs)
    #tasks += ["-s", 0]         # significance
    tasks += ["--fcc", "7,2@"+mass]  # fitCrossChecks
    #tasks += ["-u", 0]         # breakdown of muhat errors stat/syst/theory
    # requires fitCrossChecks:
    tasks += ["-m", "7,2"]     # reduced diag plots (quick)
    #tasks += ["-b", "2"]       # unfolded b-tag plots (quick)
    #tasks += ["-p", "2,0"]       # post-fit plots (CPU intense)
    #tasks += ["-t", "0,2"]     # yield tables (CPU intense)
    
    ########## submit jobs

    # in case you want to run something locally
    Mgr.run_local_batch(configfiles, outversion, tasks)
    #Mgr.run_local_batch(configfiles_stat, outversion, ["-w", "-l", 1])
    #    Mgr.run_lyon_batch(configfiles, outversion, tasks,"name@lal.in2p3.fr")

    # adjust to recover failed ranking subjobs
    redoSubJob = -1

    if redoSubJob < 0:
        print "Submitting stat only jobs..."
        #Mgr.run_lxplus_batch(configfiles_stat, outversion, ["-w", "-l", 1,"-s",1], '2nd')
        print "Submitting systs jobs..."
        #Mgr.run_lxplus_batch(configfiles, outversion, tasks, '2nd')
    print "Submitting rankings..."
    #Mgr.run_lxplus_batch(configfiles, outversion, ["-w", "-r", "125"], '2nd', jobs=20, subJob=redoSubJob)
    #Mgr.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "-r", "125"], '2nd', jobs=20, subJob=redoSubJob)

    ########## non-default stuff. Warning: don't submit rankings / toys / NLL scans with the same outversion and nJobs!
    #print "Submitting toys..."
    #Mgr.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "--fcc", "6"], '2nd', jobs=50, subJob=redoSubJob)
    #print "Submitting NLL scans..."
    #Mgr.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "--fcc", "7,2,doNLLscan"], '2nd', jobs=50, subJob=redoSubJob)
