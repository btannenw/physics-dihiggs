#!/usr/bin/env python
""" A small script to run the different algorithms of FitCrossCheckForLimits in parallel

Author: Nicolas Morange
Date:   2013-01-17
Email:  nicolas.morange@cern.ch

Description:
    This script is a simple wrapper to FitCrossCheckForLimits.
    The user sets the standard input and output variables, as well as the number of jobs
    to be launched in parallel.
    The user then indicates which algorithms to execute, with their parameters. For technical
    reasons some parameters to be provided here are dummy.
    Read FitCrossCheckForLimits documentation to find which ones are relevant to each algorithm.
    The script then executes the different algorithms in parallel, and merge the outputs

"""

import subprocess
import os
import shutil
import time
import sys

from ROOT import gROOT
from ROOT import TFileMerger



###############################################################
###################  User configuration  ######################
###############################################################

NCORES          = 6 # maximum number of jobs to launch in parallel
workspace       = "workspaces/0lep_cut_stat/combined/125.root"
outputdir       = "fccs/FitCrossChecks_0lep_cut_stat_combined/"
workspaceName   = "combined"
modelConfigName = "ModelConfig"
ObsDataName     = "obsData"

# syntax: ["AlgName", mu, sigma, "IsConditional"]
# some parameters are not used by some algorithms. See the C++ clas for description.
# mu and sigma have different meanings for PlotsStatisticalTest
algs_to_execute = [
    # -----------------------------------------------------------------------------------
    # - Plot nominal and +/- Nsigma (for each nuisance paramater) for Data, signal+bkg
    # -----------------------------------------------------------------------------------
    #["PlotHistosBeforeFit",0.0,1.0,"false"],

    # -----------------------------------------------------------------------------------
    # - Control plots for morphing (ie, -1/0/+1 sigma --> continuous NP)
    # -----------------------------------------------------------------------------------
    #["PlotMorphingControlPlots",0.0,1.0,"false"],

    # ----------------------------------------------------------------------------------
    # - Plot histograms after unconditional fit (theta and mu fitted at the same time)
    # ----------------------------------------------------------------------------------
    #["PlotHistosAfterFitEachSubChannel",0.0, 1.0, "false"],
    ["PlotHistosAfterFitGlobal",0.0, 1.0, "false"],

    # -----------------------------------------------------------------------------------------
    # - Plot the conditionnal fitted nuisance parameters value (theta fitted while mu is fixed)
    # -----------------------------------------------------------------------------------------
    #["PlotHistosAfterFitEachSubChannel", 0.0, 1.0, "true"],
    ["PlotHistosAfterFitGlobal",0.0, 1.0, "true"],

    # -------------------------------------------
    # - Plot the nuisance parameters versus mu
    # -------------------------------------------
    #["PlotsNuisanceParametersVSmu",0.0, 1.0, "false"],

    # -------------------------------------------
    # - Plot the pulls and stat test from toys
    # -------------------------------------------
    #["PlotsStatisticalTest",1.0, 0.0, "false"],

    # -------------------------------------------
    # - Plot the pulls from fit to asimov dataset
    # -------------------------------------------
    ["FitToAsimov",1.0, 0.0, "false"], # mu=1, unconditional fit
    #["FitToAsimov",0.0, 0.0, "false"], # mu=0, unconditional fit
    ["FitToAsimov",1.0, 0.0, "true"], # mu=1, conditional fit mu=1
    #["FitToAsimov",0.0, 0.0, "true"], # mu=0, conditional fit mu=0

]

available_algs = [
    ["PlotHistosBeforeFit",0.0,1.0,"false"],                    #0 , first integer is value of mu, set to mu=0
    ["PlotHistosAfterFitEachSubChannel",1.0, 1.0, "false"],     #1
    ["PlotHistosAfterFitGlobal",1.0, 1.0, "false"],             #2 - unconditional ( start with mu=1 )
    ["PlotHistosAfterFitEachSubChannel", 0.0, 1.0, "true"],     #3
    ["PlotHistosAfterFitGlobal",0.0, 1.0, "true"],              #4 - conditional mu = 0
    ["PlotHistosAfterFitGlobal",1.0, 1.0, "true"],              #5 - conditional mu = 1
    ["PlotsStatisticalTest",1.0, 0.0, "false"],                 #6 - run Asimov mu = 1 toys: randomize Poisson term
    ["FitToAsimov",1.0, 0.0, "false"],                          #7 - unconditional fit to asimov where asimov is built with mu=1
    ["FitToAsimov",0.0, 0.0, "false"],                          #8 - unconditional fit to asimov where asimov is built with mu=0
    ["FitToAsimov",1.0, 0.0, "true"],                           #9 -   conditional fit to asimov where asimov is built with mu=1
    ["FitToAsimov",0.0, 0.0, "true"],                           #10-   conditional fit to asimov where asimov is built with mu=0
    ["PlotsNuisanceParametersVSmu",0.0, 1.0, "false"],          #11
    ["PlotHistosAfterFitGlobal",0.0, 1.0, "false"],             #12 - unconditional ( start with mu=0, sigma = 1 )
    ["PlotHistosAfterFitEachSubChannel", 0.0, 1.0, "false"],    #13 - unconditional ( start with mu=0, sigma = 1 )
    ["PlotsStatisticalTest",0.0, 0.0, "false"],                 #14 - run Asimov mu = 0 toys: randomize Poisson term
    ["PlotHistosAfterFitGlobal",0.0, 1.0, "false"],             #15 - unconditional ( start with mu=0 )
    ["PlotHistosAfterFitGlobal",0.0, 1.0, "true"],              #16 - conditional ( start with mu=0 )
]



###############################################################
################  End of User configuration  ##################
###############################################################


def main(doNLLscan = False):
    """Parallelize the execution of algorithms in FitCrossCheckForLimits
    """

    print NCORES
    # cleaning
    outdir=outputdir.rstrip('/')
    outdir += '/'
    try:
        os.makedirs(outdir)
    except:
        pass

    # first, compile
    if compile():
        print "Compilation failed !"
        print "Aborting..."
        return

    # then, execute the different algorithms
    pids=[]
    logfiles=[]
    directories=[]

    for i,alg in enumerate(algs_to_execute):
        print i, alg
        if len(pids) >= NCORES: # manage number of jobs running
            wait_completion(pids)
        print "Launching job",i,":",alg
        output_f=open(outdir+"/output_"+str(i)+".log", 'w')
        #output_f=open(os.devnull, 'w')
        logfiles.append(output_f)
        directory = outdir + str(i)
        directories.append(directory)
        formatted_args = alg[0] + "," + str(alg[1]) + "," + str(alg[2]) + "," + alg[3] \
                              + ",\"" + workspace + "\",\"" + directory + "\",\"" \
                              + workspaceName + "\",\"" + modelConfigName + "\",\"" + ObsDataName + "\""
        if doNLLscan:
            formatted_args += ",true"
        print formatted_args
        pids.append(subprocess.Popen(["root", "-l", "-b", "-q",
                                      "FitCrossCheckForLimits.C+("+formatted_args+")"],
                                     stderr=output_f, stdout = output_f))

    # Now just wait for completion of all jobs
    wait_all(pids)
    for f in logfiles:
        f.close()
        #os.system("grep -v \"^SigXsec\" {0} | grep -v \"^alpha_SysHiggsNorm\" > {0}_a".format(f.name))
        #os.rename(f.name+"_a", f.name)

    # and merge outputs
    print "Merging results..."

    for directory in directories:
        #print subprocess.check_output(["ls", directory])
        things = os.listdir(directory)
        things.remove('FitCrossChecks.root')
        for thing in things:
            if thing in os.listdir(outdir):
                subprocess.check_call(["cp", "-r", directory+"/"+thing, outdir])
            else:
                subprocess.check_call(["mv", directory+"/"+thing, outdir])
    tfm = TFileMerger()
    for directory in directories:
        tfm.AddFile(directory+"/FitCrossChecks.root")
    tfm.OutputFile(outdir+"/FitCrossChecks.root")
    tfm.Merge()
    print "Merging done !"

    # finally, remove the splitted files
    for directory in directories:
        shutil.rmtree(directory)
    print "All OK !"

def wait_all(pids):
    """Wait until completion of all launched jobs"""
    while len(pids)>0:
        wait_completion(pids)
    print "All jobs finished !"

def wait_completion(pids):
    """Wait until completion of one of the launched jobs"""
    while True:
        for pid in pids:
            if pid.poll() is not None:
                print "Process", pid.pid, "has completed"
                pids.remove(pid)
                return
        print "Waiting completion of jobs..."
        time.sleep(15) # wait 15 seconds before retrying


def compile():
    """Compile ROOT class"""
    return gROOT.ProcessLine(".L FitCrossCheckForLimits.C+")

if __name__ == "__main__":
    print sys.argv
    if len(sys.argv)>1:
        mass = sys.argv[3] if len(sys.argv) > 3 else '125'
        print "Mass = ", mass        
        workspace       = "workspaces/"+sys.argv[1]+"/combined/"+mass+".root"
        outputdir       = "fccs/FitCrossChecks_"+sys.argv[1]+"_combined/"
    if len(sys.argv) > 4:
        ObsDataName = sys.argv[4]
    doNLLscan = False
    if len(sys.argv)>2:
        algnums = sys.argv[2]
        algs_to_execute = []
        for algnum in algnums.split(','):
            if "doNLLscan" in algnum:
                doNLLscan = True
            else:
                algs_to_execute.append(available_algs[int(algnum)])
    main(doNLLscan)
