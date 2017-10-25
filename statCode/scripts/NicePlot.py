##########################################################################
#
#   This is an example script to draw a measurement in several channels
#   Stolen shamelessly from our ttH colleagues
#   Please copy this script to analysis-specific directories, then
#   edit as you need
#
##########################################################################



import glob, os, sys
import commands
from ROOT import *
from glob import *
import time

onlyOne=False
ROOT.gROOT.LoadMacro("macros/AtlasStyle.C")
ROOT.gROOT.LoadMacro("macros/AtlasUtils.C")
SetAtlasStyle()

def transformName(inputname):
    return inputname.replace("_8TeV","").replace("8TeV","").replace("GBREAK","G_BREAK")

#some injection
##3.897  : From Javier  (combination)
##3.32196: From Valerio (combination) S+B asimov
##3.839  : From Valerio (l+jets)      S+B asimov

def main():
    gStyle.SetEndErrorSize(7.0);

# file2 is red
# file1 is black

#           NAME     index  up2sig, up1sig, down1sig, down2sig, nomin, obs, inj, mu, mu err 
    value=[
        #["ttH-->#gamma#gamma",0, 16.2, 9.9, 4.6, 3.4, 5.3, 6.4 , 0.0    , 0.0, 0.0, 0.0  ],
        ["Combination",0, 4.79379, 3.5759, 1.85143, 1.37909, 2.56945, 4.13966, 3.38938 , 1.74, 1.37, 0.666 ],
#    ["Lepton+jets",1, 5.98854, 4.3701, 2.2393 , 1.668  , 3.10774, 4.21064, 3.91    , 1.28, 1.62, 0.75  ],
#    ["Dilepton"   ,2, 8.41   , 6.05  , 3.07   , 2.29   , 4.27   , 6.95231, 5.09    , 2.88, 2.29, 1.45  ]

###########to be updated #######################

        ["0 lepton",1, 5.98854, 4.3701, 2.2393 , 1.668  , 3.10774, 4.21064, 3.91    , 1.28, 1.62, 0.75  ],
        ["1 lepton"   ,2, 8.41   , 6.05  , 3.07   , 2.29   , 4.27   , 6.95231, 5.09    , 2.88, 2.29, 1.45  ],
        ["2 lepton"   ,3, 8.41   , 6.05  , 3.07   , 2.29   , 4.27   , 6.95231, 5.09    , 2.88, 2.29, 1.45  ]
        ]

    plots(value, "channels")

#WH mu = 2.03 +0.7346  -0.6640   (Av. = 0.6993) DataStat error  +  0.53  - 0.44 (Av. = 0.48)
#ZH  mu = 0.06  +0.5502  -0.6350 (Av. = 0.5926) DataStat error  + 0.29  -  0.46 (Av. = 0.38)
    value=[
        ["Combination",0, 4.79379, 3.5759, 1.85143, 1.37909, 2.56945, 4.13966, 3.38938 , 1.74, 1.37, 0.666 ],
        ["WH",1, 5.98854, 4.3701, 2.2393 , 1.668  , 3.10774, 4.21064, 3.91    , 2.03, 0.669, 0.48  ],
        ["ZH"   ,2, 8.41   , 6.05  , 3.07   , 2.29   , 4.27   , 6.95231, 5.09    , 0.06, 0.5926, 0.38  ]
    ]

    plots(value, "WHZH")

def plots(value, name):

    theHisto=TH2D("plot","plot",20,0,12,len(value),-0.5,len(value)-0.5)
    theHisto.GetXaxis().SetTitle("95% CL limit on #sigma/#sigma_{SM} at m_{H}=125 GeV")
    theHisto.GetYaxis().SetLabelSize(0.06)
    for obj in value:
        print "setting: "+str(obj[1])
        theHisto.GetYaxis().SetBinLabel(obj[1]+1,obj[0])

## just for the legend
    tmp1=TH1D("tmp1", "tmp1", 1,1,2)
    tmp2=TH1D("tmp2", "tmp2", 1,1,2)
    tmp3=TH1D("tmp3", "tmp3", 1,1,2)
    tmp4=TH1D("tmp4", "tmp4", 1,1,2)

    tmp1.SetFillColor(3)
    tmp1.SetFillStyle(1001)
    tmp1.SetLineColor(1)
    tmp1.SetLineStyle(2)
    tmp1.SetLineWidth(3)

    tmp2.SetFillStyle(1001)
    tmp2.SetFillColor(5)
    tmp2.SetLineColor(1)
    tmp2.SetLineStyle(2)
    tmp2.SetLineWidth(3)

    tmp4.SetLineColor(2)
    tmp4.SetLineStyle(2)
    tmp4.SetLineWidth(3)

    tmp3.SetLineColor(1)
    tmp3.SetLineStyle(1)
    tmp3.SetLineWidth(3)


    c = TCanvas("test","test",800,600)
    gPad.SetLeftMargin(0.20)
    gPad.SetTopMargin(0.10)
    theHisto.Draw()
    listPave=[]
    listLines=[]

    for obj in value:
        sig1= TPave(obj[5],obj[1]-0.5,obj[2],obj[1]+0.5)
        sig1.SetFillColor(5)
        sig1.Draw("SAME")
        sig2= TPave(obj[4],obj[1]-0.5,obj[3],obj[1]+0.5)
        sig2.SetFillColor(3)
        sig2.Draw("SAME")
        listPave.append(sig1)
        listPave.append(sig2)
        l = TLine(obj[6],obj[1]-0.5,obj[6],obj[1]+0.5)
        l.SetLineStyle(2)
        l.SetLineWidth(4)
        l.SetLineColor(1)
        l.Draw("SAME")
        l2 = TLine(obj[7],obj[1]-0.5,obj[7],obj[1]+0.5)
        l2.SetLineStyle(1)
        l2.SetLineWidth(4)
        l2.SetLineColor(1)
        l2.Draw("SAME")
        l3 = TLine(obj[8],obj[1]-0.5,obj[8],obj[1]+0.5)
        l3.SetLineStyle(2)
        l3.SetLineWidth(4)
        l3.SetLineColor(2)
        l3.Draw("SAME")

        listLines.append(l)
        listLines.append(l2)
        listLines.append(l3)
         
        c.Update()
        ####time.sleep(1)
    theHisto.Draw("SAMEAXIS")

    lall = TLine(1.0,-0.5,1.0,len(value)-0.5)
    lall.SetLineWidth(3)
    lall.SetLineColor(kGray)
    lall.Draw("SAME")

    ATLAS_LABEL(0.15,0.93,1)
    myText(     0.30,0.93,1,"Internal")
    myText(     0.58,0.93,1,"#sqrt{s}=8 TeV, #intL dt=20.3 fb^{-1}",0.045)

    legend3=TLegend(0.60,0.2,0.95,0.5)
    legend3.SetTextFont(42)
    legend3.SetTextSize(0.04)
    legend3.SetFillColor(0)
    legend3.SetLineColor(0)
    legend3.SetFillStyle(0)
    legend3.SetBorderSize(0)
    legend3.AddEntry(tmp1,"Expected #pm 1#sigma","fl")
    legend3.AddEntry(tmp2,"Expected #pm 2#sigma","fl")
    legend3.AddEntry(tmp3,"Observed","l")
#legend3.AddEntry(tmp4,"Injected sig. x1 SM","l")
    legend3.AddEntry(tmp4,"Injected signal (\mu=1)","l")
    legend3.Draw("SAME")

    c.SaveAs("Limits_{0}.png".format(name))
    c.Update()
###time.sleep(10)

    value.append( ["Fake"   ,2, 8.41   , 6.05  , 3.07   , 2.29   , 4.27   , 6.95231, 5.09    , 2.88, 2.29, 1.45  ] )
     
    theHisto2=TH2D("plot2","plot2",20,-1,11,len(value),-0.5,len(value)-0.5)
    theHisto2.GetXaxis().SetTitle("best fit #mu=#sigma/#sigma_{SM} for m_{H}=125 GeV")
    theHisto2.GetYaxis().SetLabelSize(0.06)
    for obj in value:
        if "Fake" in obj[0]: continue
        print "setting: "+str(obj[1])
        theHisto2.GetYaxis().SetBinLabel(obj[1]+1,obj[0])

    c2 = TCanvas("test2","test2",800,600)
    gPad.SetLeftMargin(0.20)
    gPad.SetTopMargin(0.10)
    theHisto2.Draw()

    tmp5=TH1D("tmp5", "tmp5", 1,1,2)
    tmp6=TH1D("tmp6", "tmp6", 1,1,2)
    tmp5.SetLineColor(1)
    tmp6.SetLineWidth(4)
    tmp5.SetLineWidth(3)
    tmp6.SetLineColor(8)


    g = TGraphErrors()
    g.SetLineWidth(4)
    g.SetMarkerColor(2)
    g.SetMarkerStyle(20)
    g.SetMarkerSize(1.6)

    g2 = TGraphErrors()
    g2.SetLineWidth(3)
    g2.SetMarkerColor(2)
    g2.SetMarkerStyle(20)
    g2.SetMarkerSize(1.6)
    g2.SetLineColor(8)


    for obj in value:
        if "Fake" in obj[0]: continue
        g.SetPoint( obj[1]     , obj[9]    , float(obj[1]) )
        g.SetPointError( obj[1], obj[10]   , float(0) )
        g2.SetPoint( obj[1]     , obj[9]    , float(obj[1]) )
        g2.SetPointError( obj[1], obj[11]   , float(0) )
        print " --> Setting point: "+str(obj[9])+"  +/- "+str(obj[10])
        mystring1 = "#mu= %.1f #pm %.1f" % (obj[9], obj[10])
        mystring2 = " (stat. #pm %.1f)" % (obj[11])

        mystring1 = "%.1f" % (obj[9])
        mystring2 = " #pm %.1f" % (obj[10])
        mystring3 = "( %.1f )" % (obj[11])
        
        #myText(0.60,0.28+0.25*obj[1],1, mystring1, 0.06)  # 0.042
        #myText(0.75,0.28+0.25*obj[1],419, mystring2, 0.042) ##8
        sca = 0.20 
        if(len(value)==4): sca = 0.20
        elif(len(value)==5): sca = 0.15

        myText(0.64,0.22+sca*obj[1],1, mystring1, 0.05)  # 0.042
        myText(0.69,0.22+sca*obj[1],1, mystring2, 0.05) ##8
        myText(0.82,0.22+sca*obj[1],1, mystring3, 0.05) ##8

    myText(0.70,0.12+0.20*3,1, " ( tot )", 0.05) ##8
    myText(0.82,0.12+0.20*3,1, "( stat )", 0.05) ##8

    legend4=TLegend(0.25,0.73,0.60,0.85)
    legend4.SetTextFont(42)
    legend4.SetTextSize(0.04)
    legend4.SetFillColor(0)
    legend4.SetLineColor(0)
    legend4.SetFillStyle(0)
    legend4.SetBorderSize(0)
    legend4.AddEntry(tmp5,"tot." ,"l")
    legend4.AddEntry(tmp6,"stat.","l")
    legend4.Draw("SAME")

    lall.Draw("SAME")
    g.Draw("SAMEP")
    g2.Draw("SAMEP")

    ATLAS_LABEL(0.15,0.93,1)
    myText(     0.30,0.93,1,"Internal")
    myText(     0.58,0.93,1,"#sqrt{s}=8 TeV, #intL dt=20.3 fb^{-1}",0.045)

    c2.SaveAs("Muhat_{0}.png".format(name))
    c2.Update()



if __name__ == "__main__":
    gROOT.SetBatch(True)
    main()
