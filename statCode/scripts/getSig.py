#!/usr/bin/env python

import sys
import ROOT
import os

if len(sys.argv)<2:
    print """
Usage:
  python %prog [workspace] [exp/obs] [mass]

    expected = 1
    observed = 0
    default mass point = 125
"""
    sys.exit()

ws = sys.argv[1]
if len(sys.argv)>2:
    is_expected = bool(int(sys.argv[2]))
else:
    is_expected = False
if len(sys.argv)>3:
    mass = sys.argv[3]
else:
    mass = "125"
ROOT.gROOT.SetBatch(True)
ROOT.gROOT.ProcessLine(".L runSig.C+")

if is_expected:
    suff = "_exp"
else:
    suff = "_obs"

outdir = ws+suff+"_p0"
print outdir

ROOT.runSig("workspaces/"+ws+"/combined/"+mass+".root", "combined", "ModelConfig",
            "obsData", "asimovData_1", "conditionalGlobs_1", "nominalGlobs",
            mass, outdir, is_expected)

