#!/usr/bin/env python

import sys
import ROOT
import os

ws = sys.argv[1]

# if the WS name contains "_jobYofY" merge jobs:
# assuming "_jobXofY" with X = 0..Y-1

nJobs = 1
pat = ROOT.TPRegexp(".*_job(\\d+)of(\\d+).*");
res = ROOT.TObjArray(pat.MatchS(ws));
if res.GetSize() >= 3:
    if res.At(1) and res.At(2):
        nJobsTmp = int(((ROOT.TObjString)(res.At(2))).GetString().Data());
        iJobTmp = int(((ROOT.TObjString)(res.At(1))).GetString().Data());
        if nJobsTmp == iJobTmp and nJobsTmp > 1:
            nJobs = nJobsTmp

miss = 0
if nJobs > 1:
    print "Merging jobs: " + str(nJobs)
    outdir = "fccs/FitCrossChecks_" + ws + "_combined"
    os.system( "mkdir -vp " + outdir)
    jobStr = "job%iof%i" % (nJobs, nJobs)
    # check for missing
    for iJob in range(0, nJobs):
        jobStr2 = "job%iof%i" % (iJob, nJobs)
        ws2 = ws.replace(jobStr, jobStr2)
        outdir2 = "fccs/FitCrossChecks_" + ws2 + "_combined"
        #print "Copying files for job " + str(iJob)
        status = os.system( "ls -l " + outdir2 + "/FitCrossChecks.root" )
        if status is not 0:
            miss += 1
    # hadd FitCrossChecks.root:
    jobStr2 = "job*of%i" % (nJobs)
    ws2 = ws.replace(jobStr, jobStr2)
    outdir2 = "fccs/FitCrossChecks_" + ws2 + "_combined"
    os.system( "rm -i " + outdir + "/FitCrossChecks.root" )
    os.system( "hadd " + outdir + "/FitCrossChecks.root " + outdir2 + "/FitCrossChecks.root" )
else:
    print "nJobs not larger than 1, no merging to be done!"
    
# redo the plotting

outdir = "fccs/FitCrossChecks_" + ws + "_combined"
os.system( "mkdir -vp " + outdir + "/PlotsAfterFitToAsimov/unconditionnal/AllNNLProjections" )
os.system( "mkdir -vp " + outdir + "/PlotsAfterGlobalFit/conditionnal_MuIsEqualTo_0/AllNNLProjections" )
os.system( "mkdir -vp " + outdir + "/PlotsAfterGlobalFit/unconditionnal/AllNNLProjections" )

ROOT.gROOT.LoadMacro("$WORKDIR/macros/AtlasStyle.C")
ROOT.SetAtlasStyle()
ROOT.gROOT.SetBatch(True)
ROOT.gROOT.ProcessLine(".L $WORKDIR/macros/drawNLLscan.cpp+")
print "\n    Running drawNLLscan:\n"
ROOT.drawNLLscan(ws)
        
if miss is not 0:
    print ""
    print "WARNING: Missing " + str(miss) + " of " + str(nJobs) + " jobs. Corresponding Nuisance parameter will be missing!"
    print ""
