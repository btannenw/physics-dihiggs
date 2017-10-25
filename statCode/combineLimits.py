#!/usr/bin/env python

import os,sys

# *** 1. Ask for name of target directory
combDir = raw_input("Enter tag of combination dir (e.g. ): ")
combDir = 'root-files/combinedX.' + combDir

sel700dir  = 'root-files/8000X.260916-v0-sep23inputs_fullRange_classicSelections_HH_13TeV_260916-v0-sep23inputs_fullRange_classicSelections_Systs_'
sel2000dir = 'root-files/9000X.260916-v0-sep23inputs_fullRange_classicSelections_HH_13TeV_260916-v0-sep23inputs_fullRange_classicSelections_Systs_'

masses700  = [500, 600, 700, 750, 800]
masses2000 = [900, 1000, 1100, 1200, 1300, 1400, 1500, 1600, 1800, 2000, 2250, 2500, 2750, 3000]

for m in masses700:
    os.system("cp -r {0} {1}".format(sel700dir+str(m) + '_obs', combDir + '_' + str(m) + '_obs'))

for m in masses2000:
    os.system("cp -r {0} {1}".format(sel2000dir+str(m) + '_obs', combDir + '_' + str(m) + '_obs'))
