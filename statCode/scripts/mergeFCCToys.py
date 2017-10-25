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

if nJobs > 1:
    print "Merging jobs: " + str(nJobs)
    outdir = "fccs/FitCrossChecks_" + ws + "_combined"
    os.system( "mkdir -vp " + outdir)
    jobStr = "job%iof%i" % (nJobs, nJobs)
    jobStr2 = "job*of%i" % (nJobs)
    ws2 = ws.replace(jobStr, jobStr2)
    outdir2 = "fccs/FitCrossChecks_" + ws2 + "_combined"
    os.system( "rm -v " + outdir + "/mytoys.root" )
    os.system( "hadd " + outdir + "/mytoys.root " + outdir2 + "/mytoys.root" )
else:
    print "nJobs not larger than 1, no merging to be done!"

ROOT.gROOT.LoadMacro("$WORKDIR/macros/AtlasStyle.C")
ROOT.SetAtlasStyle()

ROOT.gROOT.SetBatch(True)
ROOT.gROOT.ProcessLine(".L $WORKDIR/macros/drawFCCToys.cpp+")

print "\n    Running drawFCCToys:\n"
ROOT.drawFCCToys(ws)
