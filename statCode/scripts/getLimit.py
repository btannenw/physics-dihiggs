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
ROOT.gROOT.ProcessLine(".L runAsymptoticsCLs.C+")

if is_expected:
    ROOT.doExpected(True)
    suff = "_exp"
else:
    ROOT.doExpected(False)
    suff = "_obs"

#FIXME should be an option
ROOT.doBetterBands(False)
#ROOT.doBetterBands(True)

ROOT.doInjection(False)#original
#ROOT.doInjection(True)# Sept 1

outdir = ws+suff

ROOT.runAsymptoticsCLs("workspaces/"+ws+"/combined/"+mass+".root", "combined",
                       "ModelConfig", "obsData", "", outdir, mass, 0.95)
#ROOT.runAsymptoticsCLs("workspaces/"+ws+"/combined/"+mass+".root", "combined",
#                       "ModelConfig", "asimovData", "", outdir, mass, 0.95)

f=ROOT.TFile("root-files/"+outdir+"/"+mass+".root")
lim = f.Get("limit")
med = lim.GetBinContent(2)
p2 = lim.GetBinContent(3)
p1 = lim.GetBinContent(4)
m1 = lim.GetBinContent(5)
m2 = lim.GetBinContent(6)
obs = lim.GetBinContent(1)
inj = lim.GetBinContent(7)

print "Injected limit:", inj
print "Expected limit:", med, "+", p1-med, "-", med-m1
print "Observed limit:", obs
print "{0:.2f}".format(med)

outfname = 'root-files/'+outdir+'/limit_'+mass+'.txt'
os.system('echo "Injected limit: '+str(inj)+'" > '+ outfname)
os.system('echo "Expected limit: '+str(med)+' +'+str(p1-med)+' -'+str(med-m1)+'" >> '+ outfname)
os.system('echo "Observed limit: '+str(obs)+'" >> '+outfname)
os.system('echo "Expected limit: {0:.2f}^{{+{1:.2f}}}_{{-{2:.2f}}}" >> {3}'.format(med, p1-med, med-m1, outfname))
os.system('echo "Observed limit: {0:.2f}" >> {1}'.format(obs, outfname))
os.system('echo " " >> {0}'.format(outfname))
os.system('echo "obs -2s -1s exp +1s _2s" >> {0}'.format(outfname))
os.system('echo "{0:.2f} & {1:.2f} &  {2:.2f} & {3:.2f} & {4:.2f} & {5:.2f} \\\\" >> {6}'.format(obs, m2, m1, med, p1, p2, outfname))

print "Now adding asimov data with mu at expected limit."
ROOT.RooWorkspace.rfimport = getattr(ROOT.RooWorkspace, 'import')

f = ROOT.TFile("workspaces/"+ws+"/combined/"+mass+".root")
wsm = f.Get("combined")

print "Data contained in the original workspace: "
allData = wsm.allData()
alreadyExists = False
while allData.size() > 0:
    data = allData.front()
    allData.pop_front()
    print "   ==>",data.GetName()
    if data.GetName() == "asimovDataAtLimit":
        alreadyExists = True

if alreadyExists:
    print "asimovDataAtLimit already exists"
else:
    data = wsm.data("obsData")

    mc = wsm.obj("ModelConfig")

    poi = mc.GetParametersOfInterest().first();

    poi.setVal(med);

    allParams = mc.GetPdf().getParameters(data);
    ROOT.RooStats.RemoveConstantParameters(allParams)

    globObs = ROOT.RooArgSet("globObs")
    asimov_data = ROOT.RooStats.AsymptoticCalculator.MakeAsimovData(mc, allParams, globObs)

    asimov_data.SetName("asimovDataAtLimit")
    wsm.rfimport(asimov_data)

    print "Data contained in the modified workspace: "
    allData = wsm.allData()
    while allData.size() > 0:
        data = allData.front()
        allData.pop_front()
        print "   ==>",data.GetName()
    
    wsm.writeToFile("workspaces/"+ws+"/combined/"+mass+".root")


