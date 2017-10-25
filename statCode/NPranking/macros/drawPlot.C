#ifndef DRAWPLOT
#define DRAWPLOT

#include "macros/atlasstyle-00-03-05/AtlasLabels.C"

#include "TGraph.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TAxis.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TButton.h"

#include "macros/fileHolder.C"
#include "macros/saveAscii.C"
#include "macros/parseString.C"

#include <fstream>
#include <string>
#include <vector>

using namespace std;

TCanvas* c_saved;
string saveName_saved;
string labelTxt  = "Internal";

bool doInj       = 0;
bool doLogx      = 0;
bool doSave      = 1;
bool forApproval = 0;
bool showLabel   = 1;
double labelPosX = 0.2;
double labelPosY = 0.87;
double textSize  = 0.04;
double txtPosX   = 0.6;
double txtPosY   = 0.3;
double channelPosX = 0.15;
double channelPosY = 0.87;

string lumi_2011     = "4.8";
string lumi_2012     = "3.2";
string lumi          = "1.0-5.0";
string energy        = "8";
string year          = "2012";
string channel_label = "";

string overlay  = "";
string overlay2 = "";
string overlay3 = "";

double xmin_leg  = 0.6;
double xdiff_leg = 0.22;
double ymax_leg  = 0.85;
double ydiff_leg = 0.13;

double markerSize = 0.8;

int minMass = 90;
int maxMass = 600;

bool overridden   = 0;
bool overrideMass = 0;

bool dozoom = 0;
bool do2011 = 0;
bool do2012 = 0;

bool dogg_4l_lvlv = 0;
bool dogg_4l      = 0;
bool dogg         = 0;
bool dollll       = 0;
bool dollqq       = 0;
bool dollvv       = 0;
bool dolvqq       = 0;
bool dolvlv       = 0;
bool dott         = 0;
bool doll         = 0;
bool dolh         = 0;
bool dowh         = 0;
bool dozh         = 0;
bool dovh         = 0;
bool dozz         = 0;
bool doww         = 0;
bool docb         = 0;
bool dolog        = 0;
bool drawBands    = 0;

void applyOverlay(string in, string& out, string suf) {
  if (in == "") return;
  vector<string> parsed = parseString(in,":");
  if (parsed.size() > 1) {
    in = parsed[0];
  }
  out = in+suf;
  for (int i=1;i<(int)parsed.size();i++) {
    out += ":"+parsed[i];
  }
}

void applyOverlay(string in, string& out) {
  vector<string> parsed = parseString(in,":");
  if (parsed.size() > 1) {
    out = parsed[1];
  }
}

void computeFlags(string cardName) {
  if (!overridden) {
    dogg_4l_lvlv = 0;
    dogg_4l      = 0;
    dogg         = 0;
    dollll       = 0;
    dollqq       = 0;
    dollvv       = 0;
    dolvqq       = 0;
    dolvlv       = 0;
    doll         = 0;
    dolh         = 0;
    dowh         = 0;
    dozh         = 0;
    dovh         = 0;
    dozz         = 0;
    doww         = 0;
    dott         = 0;
    docb         = 0;
    dolog        = 1;
    drawBands    = 0;
    showLabel    = 1;
    // labelTxt = "";
    // if (1) docb = 1;

    doLogx = !dozoom && docb;

    if (cardName.find("2012") != string::npos || cardName.find("12_") != string::npos || cardName.find("2013") != string::npos) {
      do2012 = true;
    }
    else if (cardName.find("2011") != string::npos || cardName.find("11_") != string::npos) {
      do2011 = true;
    }

    if (do2011) energy = "7";
    if (do2012) energy = "8";
    if (do2011 && do2012) lumi += "+";

    if (cardName.find("gg_4l_lvlv") != string::npos) {
      dogg_4l_lvlv = true;
    }
    else if (cardName.find("gg_4l") != string::npos) {
      if (do2011) {
        lumi   = "4.9";
        energy = "7";
        year   = "2011";
      }
      else if (do2012) {
        lumi   = "3.4";
        energy = "8";
        year   = "2012";
      }
      else {
        lumi   = "4.9+3.2";
        energy = "7+8";
        year   = "2011+2012";
      }
      channel_label = "#gamma#gamma+4l";
      dogg_4l = true;
    }
    else if (cardName.find("gg") != string::npos) {
      lumi_2011 = "4.8";
      lumi_2012 = "5.9";
      if (do2011) {
        lumi   = "4.9";
        energy = "7";
        year   = "2011";
      }
      else if (do2012) {
        lumi   = "5.9";
        energy = "8";
        year   = "2012";
      }
      else {
        lumi   = "4.9+3.2";
        energy = "7+8";
        year   = "2011+2012";
      }
      dogg = true;
    }
    else if (cardName.find("llll") != string::npos) {
      lumi_2011 = "4.8";
      lumi_2012 = "5.8";
      if (do2011) {
        lumi   = "4.8";
        energy = "7";
        year   = "2011";
      }
      else if (do2012) {
        lumi   = "5.8";
        energy = "8";
        year   = "2012";
      }
      else {
        lumi   = "4.8+3.2";
        energy = "7+8";
        year   = "2011+2012";
      }
      dollll = true;
    }
    else if (cardName.find("_llqq_") != string::npos) {
      dollqq = true;
    }
    else if (cardName.find("_llvv_") != string::npos) {
      dollvv = true;
    }
    else if (cardName.find("_lvqq_") != string::npos) {
      dolvqq = true;
    }
    else if (cardName.find("lvlv") != string::npos) {
      lumi_2011 = "4.7";
      lumi_2012 = "20.7";
      lumi      = "4.7+20.7";
      year      = "2011+2012";
      if (do2011) {
        lumi   = "4.7";
        energy = "7";
      }
      if (do2011 && do2012) {
        lumi += "+";
      }
      if (do2012) {
        lumi   = "20.7";
        energy = "8";
      }
      doLogx = false;
      dolvlv = true;
    }
    else if (cardName.find("_tt_") != string::npos) {
      dott = true;
    }
    else if (cardName.find("_lh_") != string::npos) {
      dolh = true;
    }
    else if (cardName.find("_wh_") != string::npos) {
      dowh = true;
    }
    else if (cardName.find("_zh_") != string::npos) {
      dozh = true;
    }
    else if (cardName.find("_vh_") != string::npos) {
      dovh = true;
    }
    else if (cardName.find("_ww_") != string::npos) {
      doww = true;
    }
    else if (cardName.find("_zz_") != string::npos) {
      dozz = true;
    }
    else if (cardName.find("_tt_") != string::npos) {
      dott = true;
    }
    else {
      lumi_2011 = "4.6-4.8";
      lumi_2012 = "5.8-5.9";
      if (do2011) {
        lumi = "4.6-4.9";
      }
      else if (do2012) {
        lumi = "5.8-5.9";
      }
      else {
        lumi = "4.9+5.8";
      }
      if (do2011) {
        year = "2011";
      }
      else if (do2012) {
        year = "2012";
      }
      else {
        year = "2011+2012";
      }
      docb = true;
    }
  }
}

void drawPlot(string cardName, int nrCols, fileHolder& numbers) {
  numbers.setNrCols(nrCols);
  numbers.readInFile(cardName);
  vector<double> vec_massPoints_tmp = numbers.massPoints;
  int nrNumbers = vec_massPoints_tmp.size();
  for (int i=0;i<nrNumbers;i++) {
    if (vec_massPoints_tmp[i] < minMass || vec_massPoints_tmp[i] > maxMass) {
      numbers.removeMass(vec_massPoints_tmp[i]);
    }
  }
}

double* getAry(vector<double> numbers) {
  int nrPoints = numbers.size();
  double* ary = new double[nrPoints];
  for (int i=0;i<nrPoints;i++) {
    ary[i] = numbers[i];
  }
  return ary;
}

TGraph* makeGraph(string title, int n, double* x_ary, double* y_ary) {
  TGraph* graph = new TGraph(n, x_ary, y_ary);
  graph->SetTitle("");
  graph->GetXaxis()->SetTitle("m_{H} [GeV]");
  graph->GetYaxis()->SetTitle(title.c_str());
  return graph;
}

TGraphAsymmErrors* makeGraphErr(string title, int n, double* x_ary, double* central, double* errlo, double* errhi) {
  TGraphAsymmErrors* graph = new TGraphAsymmErrors(n, x_ary, central, NULL, NULL, errlo, errhi);
  graph->SetTitle("");
  graph->GetXaxis()->SetTitle("m_{H} [GeV]");
  graph->GetYaxis()->SetTitle(title.c_str());
  return graph;
}

TGraphAsymmErrors* makeGraphErr(string title, int n, double* x_ary, double* central, double* cenlo, double* cenhi, double* errlo, double* errhi) {
  TGraphAsymmErrors* graph = new TGraphAsymmErrors(n, x_ary, central, cenlo, cenhi, errlo, errhi);
  graph->SetTitle("");
  graph->GetXaxis()->SetTitle("m_{H} [GeV]");
  graph->GetYaxis()->SetTitle(title.c_str());
  return graph;
}

TLegend* makeLeg() {
  xmin_leg  = labelPosX;
  xdiff_leg = 0.22;
  ymax_leg  = 0.74;

  if (overlay != "") {
    ymax_leg = 0.74;
    ydiff_leg += 0.005;
  }
  if (overlay2 != "") {
    ydiff_leg += 0.005;
  }
  if (overlay3 != "") {
    ydiff_leg += 0.005;
  }

  TLegend* leg = new TLegend(xmin_leg, ymax_leg-ydiff_leg, xmin_leg+xdiff_leg, ymax_leg, "", "NDC");
  leg->SetFillStyle(0);
  leg->SetTextSize(textSize);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  return leg;
}

TLegend* makeLeg2()
{
  xmin_leg  = 0.32;
  xdiff_leg = 0.22;
  ymax_leg  = 0.72;

  if (overlay != "") {
    ymax_leg = 0.74;
    ydiff_leg += 0.005;
  }
  if (overlay2 != "") {
    ydiff_leg += 0.005;
  }
  if (overlay3 != "") {
    ydiff_leg += 0.005;
  }

  TLegend* leg = new TLegend(xmin_leg+xdiff_leg, ymax_leg-ydiff_leg/2, xmin_leg+2*xdiff_leg, ymax_leg, "", "NDC");
  leg->SetFillStyle(0);
  leg->SetTextSize(textSize);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  return leg;
}

void save(string baseName, string type, TCanvas* c1) {
  c_saved = c1;
  saveName_saved = baseName;
  system(("mkdir -vp " + type + "-files").c_str());
  stringstream saveName;
  saveName << type << "-files/" << baseName << "." << type;
  if (doSave) c1->SaveAs(saveName.str().c_str());
}

void save() {
  bool doSave_save = doSave;
  doSave = 1;
  save(saveName_saved,"eps",c_saved);
  save(saveName_saved,"pdf",c_saved);
  save(saveName_saved,"C",c_saved);
  doSave=doSave_save;
}

void addButton() {
  TButton *but1 = new TButton("For Approval","34+56",labelPosX,labelPosY+0.04,labelPosX+0.2,labelPosY+0.1);
  but1->Draw();
}

void drawInsert() {
  txtPosY = 0.742;
  txtPosX = 0.51;

  if (doInj) {
    txtPosY -= 0.05;
  }
  if (dolvlv) {
    txtPosX += 0.05;
  }

  TLatex t;
  t.SetNDC();
  t.SetTextSize(textSize);

  if (do2011 || do2012) {
    stringstream lumiLatex;
    lumiLatex << "#sqrt{s} = " << energy << " TeV: #lower[-0.2]{#scale[0.6]{#int}}Ldt = " << lumi << " fb^{-1}";
    t.DrawLatex(labelPosX,0.75,lumiLatex.str().c_str());
  }
  else {
    stringstream lumiLatex1;
    lumiLatex1 << "#splitline{#sqrt{s} = 7 TeV:  #lower[-0.2]{#scale[0.6]{#int}}Ldt = " << lumi_2011 << " fb^{-1}}{";
    lumiLatex1 << "#sqrt{s} = 8 TeV:  #lower[-0.2]{#scale[0.6]{#int}}Ldt = " << lumi_2012 << " fb^{-1}}";
    t.DrawLatex(txtPosX,txtPosY,lumiLatex1.str().c_str());
  }

  if (showLabel) {
    ATLASLabel(labelPosX,labelPosY,labelTxt.c_str(),1);
  }
  if (forApproval) {
    addButton();
  }
  // if (!dozoom) {
  //   if (doLogx) {
  //     c1->SetLogx(1);
  //   }
  // }
}

void drawTopRight(string s) {
  TLatex t;
  t.SetTextSize(textSize);
  t.SetNDC();
  t.DrawLatex(labelPosX + 0.455, labelPosY, s.c_str());
}

#endif
