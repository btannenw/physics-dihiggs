#!/usr/bin/env python

import sys
import ROOT
import os

if len(sys.argv)<2:
    print """
Usage:
  python %prog [workspace] [exp/obs] [mode] [mass] [Mu_Asimov]

    expected = 2: Asimov with mu = mu_limit
    expected = 1: Asimov
    observed = 0
    default mode value = 2
    default mass point = 125
    default mu_asimov = 1 
"""
    sys.exit()

ws = sys.argv[1]
if len(sys.argv)>2:
    is_expected = int(sys.argv[2])
else:
    is_expected = 0
if len(sys.argv)>3:
    mode = int(sys.argv[3])
else:
    mode = 2
if len(sys.argv)>4:
    mass = sys.argv[4]
else:
    mass = "125"
if len(sys.argv)>5:
    mu_asimov = float(sys.argv[5])
else:
    mu_asimov = 1

dataName = "obsData"
if is_expected == 2:
    dataName = "asimovDataAtLimit"
    
ROOT.gROOT.SetBatch(True)
ROOT.gROOT.ProcessLine(".L newGetMuHat.C+")

#Hack mode = 2 to only run tot and stat
#mode = 2
print "Will open workspace", ws
f = ROOT.TFile.Open("workspaces/{0}/combined/{1}.root".format(ws,mass))
w = f.Get("combined")
mc = w.obj("ModelConfig")
pois = mc.GetParametersOfInterest()
print "Number of POIs:", pois.getSize()
it = pois.createIterator()
n = it.Next()
while n:
    print "The workspace has POI named", n.GetName()
    print "Running GetMuHat, using expected ?", is_expected, ", with mode", mode
    print "Use as POI:", n.GetName()
    ROOT.newGetMuHat(ws, is_expected==1, mode, mass, n.GetName(),"combined","ModelConfig",dataName,False,False,mu_asimov)
    n = it.Next()

