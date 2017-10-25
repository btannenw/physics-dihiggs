
#include "TObject.h"
#include "TString.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TROOT.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TGraph.h"
#include "TLine.h"
#include "TStyle.h"

#include "iostream"
#include "iomanip"
#include "fstream"

using namespace std;

void drawNLLscan(TString inFileName, TString dirName, TString plotDir) {

  // some config
  TString gfxExt = ".png";
  float yMin = -0.5;
  float yMax = 3;

  // get results tree
  TFile* file = new TFile(inFileName);
  if (file->IsZombie()) {
    cout << "ERROR: file not found: " << inFileName.Data() << endl;
    return;
  }
  if (!file -> cd(dirName)) {
    cout << "ERROR: could not change to directory: " << dirName.Data() << endl;
    return;
  }

  TKey *key;
  TIter nextkey(gDirectory -> GetListOfKeys());
  while ((key = (TKey*) nextkey())) {
    // get canvas
    TString name = key -> GetName();
    TString nameFull = name;
    name.ReplaceAll("alpha_", "");
    name.ReplaceAll("gamma_", "");
    name.ReplaceAll("ATLAS_", "");

    //cout << name.Data() << endl;
    TDirectory* dir = (TDirectory*) key->ReadObj();
    TObject* canvasObj = dir -> Get("NLLscan_" + name);
    if (!canvasObj) continue;
    TCanvas* canvas = (TCanvas*) canvasObj;
    //canvas -> Draw();

    // get graphs
    TList* list = canvas -> GetListOfPrimitives();
    TGraph* graphProfile = 0;
    TGraph* graphFull = 0;
    TF1* pol2 = 0;
    for (int i = 0; i < list -> GetEntries(); i++) {
      TString name2 = list -> At(i) -> GetName();
      //cout << name2.Data() << endl;
      if (name2 == "Graph")
        graphProfile = (TGraph*) list -> At(i);
      if (name2 == "FullLHood")
        graphFull = (TGraph*) list -> At(i);
      if (name2 == "pol2")
        pol2 = (TF1*) list -> At(i);
    }

    // draw
    double center = 0;
    if (graphProfile && graphFull) {
      double xmin, xmax, ytmp;
      int n = graphProfile -> GetN();
      graphProfile -> GetPoint(0, xmin, ytmp);
      graphProfile -> GetPoint(n - 1, xmax, ytmp);
      graphProfile -> GetPoint((n-1)/2, center, ytmp);
      graphProfile -> GetXaxis() -> SetRangeUser(xmin, xmax);
      graphProfile -> GetYaxis() -> SetRangeUser(yMin, yMax);
      graphProfile -> GetXaxis() -> SetTitle(name);
      graphProfile -> GetYaxis() -> SetTitle("-#DeltaLog(L)");
      graphProfile -> SetLineColor(2);
      graphProfile -> SetLineWidth(3);
      graphFull -> SetLineColor(4);
      graphFull -> SetLineWidth(3);

      /*
      // correct for an offset at the center
      double xtmp, yoffset;
      graphFull -> GetPoint(50, xtmp, yoffset);
      cout << "OFFSET = " << yoffset << endl;
      for (int i = 0; i < graphFull -> GetN(); i++) {
	double x,y;
	graphFull-> GetPoint(i, x, y);
	graphFull -> SetPoint(i, x, y - yoffset);
      }
      pol2 -> SetParameter(0, 0);
      */

      TCanvas* canvas2 = new TCanvas();
      graphProfile -> Draw("AL");
      if (pol2) pol2 -> Draw("same");
      graphProfile -> Draw("sameL");
      graphFull -> Draw("sameL");
      TLine line;
      line.SetLineStyle(2);
      line.DrawLine(xmin, 0, xmax, 0);
      line.DrawLine(xmin, 0.5, xmax, 0.5);
      line.DrawLine(xmin, 2, xmax, 2);
      if (-1 > xmin && -1 < xmax) line.DrawLine(-1, yMin, -1, yMax);
      if ( 1 > xmin &&  1 < xmax) line.DrawLine( 1, yMin,  1, yMax);
      if ( 0 > xmin &&  0 < xmax) line.DrawLine( 0, yMin,  0, yMax);
      TString plotName = plotDir + name + gfxExt;
      canvas2 -> Print(plotName);
      // clean up
      delete canvas2;
    }
    
    //continue;
    //if (!name.Contains("SigXsec")) continue;
    //if (!name.Contains("METReso")) continue;

    // get tree
    TObject* treeObj = dir -> Get("NLLscanTree_" + name);
    if (!treeObj) continue;
    TTree* tree = (TTree*) treeObj;

    TCanvas* canvas3 = new TCanvas();
    TObjArray* branches = tree -> GetListOfBranches();
    for (int i = 0; i < branches -> GetEntries(); i++) {
      TString nameY = branches -> At(i) -> GetName();
      if (nameY.EndsWith("_err")) continue;

      TString drawString = nameY + ":" + nameFull + ">>htemp()";
      tree -> Draw(drawString, "", "colz");
      
      TH2* htemp = (TH2*) gDirectory -> Get("htemp");
      htemp -> GetXaxis() -> SetTitle(nameFull);
      htemp -> GetYaxis() -> SetTitle(nameY);
      
      int points = tree->GetSelectedRows();
      
      TGraph* graphL = new TGraph(points, tree->GetV2(), tree->GetV1());
      graphL -> GetXaxis() -> SetTitle(nameFull);
      graphL -> GetYaxis() -> SetTitle(nameY);
      graphL -> SetMarkerStyle(7);
      graphL -> SetLineColor(2);
      graphL -> SetLineWidth(3);
      graphL -> Draw("AP");

      TString plotName = plotDir + nameFull + "_" + nameY + gfxExt;
      canvas3 -> Print(plotName);
      //clean up
      delete htemp;
    }
    // clean up
    delete canvas3;
  }
}

void drawNLLscan(TString wsName) {

  // some config
  TString dirName = "FitCrossChecks_" + wsName + "_combined/";
  TString inFileName = dirName + "FitCrossChecks.root";
  TString dataDirName = "PlotsAfterGlobalFit/conditionnal_MuIsEqualTo_0/AllNNLProjections/";
  TString dataDirNameUnc = "PlotsAfterGlobalFit/unconditionnal/AllNNLProjections/";
  TString asimovDirName = "PlotsAfterFitToAsimov/unconditionnal/AllNNLProjections/";

  drawNLLscan(inFileName, dataDirName, dirName + dataDirName);
  drawNLLscan(inFileName, dataDirNameUnc, dirName + dataDirNameUnc);
  drawNLLscan(inFileName, asimovDirName, dirName + asimovDirName);
}
