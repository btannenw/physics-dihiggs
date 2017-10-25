#!/usr/bin/env python

import sys
import ROOT
import os


ws = sys.argv[1]
if len(sys.argv)>2:
    mass = sys.argv[2]
else:
    mass = "125"
if len(sys.argv)>3:
    modelConfig = sys.argv[3]
else:
    modelConfig = "ModelConfig"
if len(sys.argv)>4:
    poi = sys.argv[4]
else:
    poi = "SigXsecOverSM"
if len(sys.argv)>5:
   dataName = sys.argv[5]
else:
   dataName = "obsData"
if len(sys.argv)>6:
   folder = sys.argv[6]
else:
   folder = "test"
if len(sys.argv)>7:
   variable = sys.argv[7]
else:
   variable = "NULL"
if len(sys.argv)>8:
   precision = sys.argv[8]
else:
   precision = 0.005
if len(sys.argv)>9:
   minos = sys.argv[9]
else:
   minos = 0
if len(sys.argv)>10:
   loglevel = sys.argv[10]
else:
   loglevel = "INFO"
ROOT.gROOT.SetBatch(True)
ROOT.gROOT.ProcessLine(".L macros/runPulls.C+")

print "$$%%^^ Running runPulls"
ROOT.runPulls("workspaces/"+ws+"/combined/"+mass+".root", poi, "combined",modelConfig, dataName, ws)

loglevel = "DEBUG"
print "$$%%^^ Running breakdown"
print "&&& ws: ", ws
print "&&& mass: ", mass
print "&&& modelConfig: ", modelConfig
print "&&& dataName: ", dataName
print "&&& poi: ", poi
print "&&& precision: ", precision
print "&&& loglevel: ", loglevel


ROOT.gROOT.ProcessLine(".L macros/runBreakdown.C+")
ROOT.runBreakdown("workspaces/"+ws+"/combined/"+mass+".root", "combined",modelConfig, dataName, poi, "config/breakdown.xml","add","total", precision, 0.0, ws, loglevel)
