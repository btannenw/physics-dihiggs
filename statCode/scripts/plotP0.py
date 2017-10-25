#!/bin/python

#########################################################################################
#
#   This is an example of a standard p0 plot
#   Copy it to an analysis-specific directory, then edit as needed
#
#########################################################################################

from __future__ import division
import sys
import os
import os.path
import commands
from datetime import date

import ROOT
ROOT.gROOT.SetBatch(True)
from ROOT import *
from math import *
from array import array

##==== read inputs ===##

channels = sys.argv[1]
inputVersion = sys.argv[5]
version = sys.argv[6]
isExpected = sys.argv[7]
isInjection = sys.argv[8]

masses = range(110, 145, 5)
#masses = [110,115,120,125,130,140]

print "Print Inputs = " , channels, inputVersion, version, isExpected, isInjection

plotdir = "plots/final/"

fout = open(plotdir + 'p0_Systs_MVA_' + channels + '_' + inputVersion + '_t.tex', 'w')                

fout.write('\\documentclass[11pt,a4paper]{article} \n');
fout.write('\\begin{document} \n');
if(isInjection == 'True'):
    fout.write('\\begin{tabular}{c|c|c|c} \\hline \n')    
    fout.write('mass (GeV) & Exp $\\rm{p_{0}}$ & Obs $\\rm{p_{0}}$ & Inj $\\rm{p_{0}}$ \\\\ \\hline \n')
else:
    fout.write('\\begin{tabular}{c|c|c} \\hline \n')    
    fout.write('mass (GeV) & Exp $\\rm{p_{0}}$ & Obs $\\rm{p_{0}}$ \\\\ \\hline \n')    

if isExpected == 'True':
    model_8 = inputVersion + "."+version+"_higgs_8TeV_{0}_"+version+"_Systs_MVA_" + channels + "_exp_p0"
else:
    model_8 = inputVersion + "."+version+"_higgs_8TeV_{0}_"+version+"_Systs_MVA_" + channels + "_obs_p0"                
location_8 = "../analysis/statistics/batch/"+inputVersion+"."+version+"/higgs_8TeV_{0}_"+version+"_Systs_MVA_"+channels+"/root-files/"

        
print "LOCATION = " , location_8
print "MODEL = " , model_8

rootfiles_8 = []
#rootfiles_7 = []
#rootfiles_78 = []

for m in masses:
    rootfiles_8.append(os.path.join(location_8.format(m), model_8.format(m), "{0}.root".format(m)))
    #rootfiles_7.append(os.path.join("root-files", model_7.format(m), "{0}.root".format(m)))
    #rootfiles_78.append(os.path.join("root-files", model_78.format(m), "{0}.root".format(m)))

print rootfiles_8
#print rootfiles_7
#print rootfiles_78

##====variable definitions===##

nPoints = len(masses)

obs_p0s_8TeV = array("d")
exp_p0s_8TeV = array("d")
inj_p0s_8TeV = array("d")
#obs_p0s_7TeV = array("d")
#exp_p0s_7TeV = array("d")
#obs_p0s_78TeV = array("d")
#exp_p0s_78TeV = array("d")
massPoints = array("d", masses)

#for m,f in zip(masses,rootfiles_7):
#    rootf = TFile.Open(f)
#    hyp = rootf.Get("hypo")
#    obs_p0s_7TeV.append(hyp.GetBinContent(3))
#    exp_p0s_7TeV.append(hyp.GetBinContent(4))

for m,f in zip(masses,rootfiles_8):
    print 'OPENING ' , f
    rootf = TFile.Open(f)
    hyp = rootf.Get("hypo")
    if(isInjection == 'True'):
        inj_p0s_8TeV.append(hyp.GetBinContent(6))
        obs_p0s_8TeV.append(hyp.GetBinContent(4))
        exp_p0s_8TeV.append(hyp.GetBinContent(5))
    else:
        obs_p0s_8TeV.append(hyp.GetBinContent(3))
        exp_p0s_8TeV.append(hyp.GetBinContent(4))
    
    if(isInjection == 'True'):
        fout.write(str(int(m)) + ' & ' + str(float(round(hyp.GetBinContent(5),5))) + ' & ' + str(float(round(hyp.GetBinContent(4),5))) + ' & ' + str(float(round(hyp.GetBinContent(6),5))) + ' \\\\ \n')
    else:
        fout.write(str(int(m)) + ' & ' + str(float(round(hyp.GetBinContent(4),5))) + ' & ' + str(float(round(hyp.GetBinContent(3),5))) + ' \\\\ \n')                
#for m,f in zip(masses,rootfiles_78):
#    rootf = TFile.Open(f)
#    hyp = rootf.Get("hypo")
#    obs_p0s_78TeV.append(hyp.GetBinContent(3))
#    exp_p0s_78TeV.append(hyp.GetBinContent(4))

print massPoints
#print obs_p0s_7TeV
#print exp_p0s_7TeV
print obs_p0s_8TeV
print exp_p0s_8TeV
if(isInjection == 'True'):
    print inj_p0s_8TeV
#print obs_p0s_78TeV
#print exp_p0s_78TeV


##====helper functions====##

def defineCanvas(name, title) :

    canv = TCanvas(name)
    #gROOT.SetMacroPath(os.environ["WORKDIR"]+"/macros")
    gROOT.SetMacroPath("/macros")
    ROOT.gROOT.LoadMacro("AtlasStyle.C")
    #SetAtlasStyle()
    canv.SetTitle(title)

    return canv

##========##

def setLegend(leg) :

    leg.SetFillColor(kWhite)
    leg.SetLineColor(kBlack)
    leg.SetTextSize(0.05*0.8)
    leg.SetTextFont(42)
    leg.SetLineStyle(1)
    leg.SetLineWidth(1)
    leg.SetShadowColor(0)
    #    leg.SetEntrySeparation(2.)

##========##

def setTextPads(textsize) :

    t = TLatex()
    t.SetTextSize(textsize)
    t.SetNDC()
    t.SetTextFont(72)
    t.SetTextColor(1)
    p = TLatex()
    p.SetTextSize(textsize)
    p.SetNDC()
    p.SetTextFont(42)
    p.SetTextColor(1)
    info = TLatex()
    info.SetTextSize(textsize*0.8)
    info.SetNDC()
    info.SetTextFont(42)
    info.SetTextColor(1)
    lumi = TLatex()
    lumi.SetTextSize(textsize*0.8)
    lumi.SetNDC()
    lumi.SetTextFont(42)
    lumi.SetTextColor(1)
    zeroS = TLatex()
    zeroS.SetTextSize(textsize*1.3)
    zeroS.SetNDC()
    zeroS.SetTextFont(42)
    zeroS.SetTextColor(kRed)
    oneS = TLatex()
    oneS.SetTextSize(textsize*1.3)
    oneS.SetNDC()
    oneS.SetTextFont(42)
    oneS.SetTextColor(kRed)

    return t, p, info, lumi, zeroS, oneS

def drawNsigma(n, x, y):
    t = TLatex()
    t.SetTextSize(.03)
    t.SetNDC()
    t.SetTextFont(42)
    t.SetTextColor(kGray+1)
t.DrawLatex(x, y, "{0} #sigma".format(n))

##====main====##

totGr = TMultiGraph()
obsGraph_8 = TGraph(nPoints, massPoints, obs_p0s_8TeV)
expGraph_8 = TGraph(nPoints, massPoints, exp_p0s_8TeV)
if(isInjection == 'True'):
    injGraph_8 = TGraph(nPoints, massPoints, inj_p0s_8TeV)    
#obsGraph_78 = TGraph(nPoints, massPoints, obs_p0s_78TeV)
#expGraph_78 = TGraph(nPoints, massPoints, exp_p0s_78TeV)
#obsGraph_7 = TGraph(nPoints, massPoints, obs_p0s_7TeV)
#expGraph_7 = TGraph(nPoints, massPoints, exp_p0s_7TeV)
#now draw
canv = defineCanvas("p0_graph", "p0_graph")
#leg_7 = TLegend(0.14, 0.30, 0.31, 0.41)
if(channels == '012'):
    leg_8 = TLegend(0.15, 0.65, 0.46, 0.81)
else:
    leg_8 = TLegend(0.15, 0.15, 0.46, 0.31)

#leg_78 = TLegend(0.38, 0.30, 0.56, 0.41)
#setLegend(leg_7)
setLegend(leg_8)
#setLegend(leg_78)
canv.SetLogy()
t, p, info, lumi, zeroS, oneS = setTextPads(0.05)
#obsGraph_8.SetMarkerStyle(7)
#obsGraph_8.SetMarkerColor(kBlack)
obsGraph_8.SetLineStyle(1)
obsGraph_8.SetLineWidth(2)
obsGraph_8.SetLineColor(1)

if(isInjection == 'True'):
    injGraph_8.UseCurrentStyle()    
    injGraph_8.SetLineStyle(3)
    injGraph_8.SetLineWidth(3)
    injGraph_8.SetLineColor(kBlue+3)    

expGraph_8.UseCurrentStyle()
expGraph_8.SetLineStyle(7)
expGraph_8.SetLineWidth(2)
expGraph_8.SetLineColor(kBlue)
leg_8.AddEntry(obsGraph_8, "Observed", "l")
leg_8.AddEntry(expGraph_8, "Expected", "l")
    
if(isInjection == 'True'):
    leg_8.AddEntry(injGraph_8, "Injected 125 GeV", "l")

#obsGraph_7.SetLineStyle(1)
#obsGraph_7.SetLineWidth(2)
#obsGraph_7.SetLineColor(kBlue)
#expGraph_7.UseCurrentStyle()
#expGraph_7.SetLineStyle(7)
#expGraph_7.SetLineWidth(2)
#expGraph_7.SetLineColor(kBlue)
#leg_7.AddEntry(obsGraph_7, "2011 Obs.", "l")
#leg_7.AddEntry(expGraph_7, "2011 Exp.", "l")
#obsGraph_78.SetLineStyle(1)
#obsGraph_78.SetLineWidth(2)
#obsGraph_78.SetLineColor(kBlack)
#expGraph_78.UseCurrentStyle()
#expGraph_78.SetLineStyle(7)
#expGraph_78.SetLineWidth(2)
#expGraph_78.SetLineColor(kBlack)
#leg_78.AddEntry(obsGraph_78, "2011+2012 Obs.", "l")
#leg_78.AddEntry(expGraph_78, "2011+2012 Exp.", "l")
#zeroSigma = TF1("zeroSigma", "0.5", massPoints[0]-5, massPoints[-1]+5)
#zeroSigma.SetLineColor(kRed)
#zeroSigma.SetLineStyle(4)
#zeroSigma.SetLineWidth(2)
line = TF1("one", "1", massPoints[0]-5, massPoints[-1]+5)
line.SetLineColor(kGray+1)
line.SetLineStyle(7)
line.SetLineWidth(2)
# use RooStats::SignificanceToPValue()
oneSigma = TF1("oneSigma", "1.58655253931457074e-01", massPoints[0]-5, massPoints[-1]+5)
oneSigma.SetLineColor(kGray+1)
oneSigma.SetLineStyle(7)
oneSigma.SetLineWidth(2)
twoSigma = TF1("twoSigma", "2.27501319481792086e-02", massPoints[0]-5, massPoints[-1]+5)
twoSigma.SetLineColor(kGray+1)
twoSigma.SetLineStyle(7)
twoSigma.SetLineWidth(2)
threeSigma = TF1("threeSigma", "1.34989803163009588e-03", massPoints[0]-5, massPoints[-1]+5)
threeSigma.SetLineColor(kGray+1)
threeSigma.SetLineStyle(7)
threeSigma.SetLineWidth(2)
#expGraph_7.SetDrawOption("C")
#obsGraph_7.SetDrawOption("L")
expGraph_8.SetDrawOption("C")
obsGraph_8.SetDrawOption("L")
if(isInjection == 'True'):
    injGraph_8.SetDrawOption("L")
#expGraph_78.SetDrawOption("C")
#obsGraph_78.SetDrawOption("L")
#line.SetDrawOption("")
#zeroSigma.SetDrawOption("")
#oneSigma.SetDrawOption("")
#twoSigma.SetDrawOption("")
#totGr.Add(obsGraph_7)
#totGr.Add(expGraph_7)
totGr.Add(obsGraph_8)
totGr.Add(expGraph_8)
if(isInjection == 'True'):
    totGr.Add(injGraph_8)
#totGr.Add(obsGraph_78)
#totGr.Add(expGraph_78)
totGr.Draw("al")
totGr.GetXaxis().SetTitle("m_{H} [GeV]")
totGr.GetYaxis().SetTitle("Local p_{0}")
totGr.GetYaxis().SetTitleOffset(1)
totGr.GetXaxis().SetTitleOffset(0.9)
totGr.GetYaxis().SetRangeUser(1.e-4, 20)
totGr.GetXaxis().SetRangeUser(massPoints[0], massPoints[-1])
line.Draw("samel")
#zeroSigma.Draw("samel")
oneSigma.Draw("samel")
twoSigma.Draw("samel")
threeSigma.Draw("samel")
totGr.Draw("l")
#leg_78.Draw("same")
leg_8.Draw("same")
#leg_7.Draw("same")
t.DrawLatex(0.15, 0.83, "ATLAS")
p.DrawLatex(0.27, 0.83, "Preliminary")
#info.DrawLatex(0.65, 0.85, "VH (b#bar{b})")
lumi.DrawLatex(0.57, 0.82, "#sqrt{s}=8 TeV #scale[0.65]{#int}Ldt = 20.3 fb^{-1}")    
drawNsigma(1, .8, .605)
drawNsigma(2, .8, .475)
drawNsigma(3, .8, .285)

# print canvas
os.system("mkdir -vp "+plotdir)
for ext in [".root", "_prelim.eps", ".png","_prelim.pdf"]:

canv.Print(plotdir + 'p0_Systs_MVA_' + channels + '_' + inputVersion + '.' + ext)                    

fout.write('\\hline \n')
fout.write('\\end{tabular} \n')
fout.write('\\end{document} \n')
fout.close()
