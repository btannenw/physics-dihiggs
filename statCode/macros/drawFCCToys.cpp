
#include "TObject.h"
#include "TObjArray.h"
#include "TString.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TMatrix.h"
#include "TStyle.h"

#include "iostream"
#include "iomanip"
#include "fstream"

using namespace std;

void drawFCCToys(TString wsName) {

  // some config
  TString inFileName = "FitCrossChecks_" + wsName + "_combined/mytoys.root";
  TString plotDir = "plots/" + wsName + "/fccToys/";
  TString gfxExt = ".eps";
  TString prefix = "SD_TS0_fitUncond_";
  TString prefixNom = "globObs_nom_";
  TString cutString = prefix + "covQual == 3 && " + prefix + "fitStatus == 0";
  // + " && " + prefix + "SigXsecOverSM_err > 0.1";
  int rebin = 2;

  // get results tree
  TFile* file = new TFile(inFileName);
  if (file->IsZombie()) {
    cout << "ERROR: file not found: " << inFileName.Data() << endl;
    return;
  }
  TObject* toyTreeObj = file -> Get("toyTree");
  if (!toyTreeObj) {
    cout << "ERROR: toyTree not found in file: " << inFileName.Data() << endl;
    return;
  }
  TTree* toyTree = (TTree*) toyTreeObj;
  TObjArray* branches = toyTree->GetListOfBranches();

  // read names of parameters
  vector<TString> names;
  for (int i = 0; i < branches -> GetSize(); i++) {
    TString name = branches -> At(i) -> GetName();
    if (!name.BeginsWith(prefix)) continue;
    if (name.EndsWith("_err")) continue;
    name.ReplaceAll(prefix, "");
    //cout << "Found: " << name << endl;
    names.push_back(name);
  }

  // prepare plotting
  int nPar = names.size();
  system(("mkdir -vp " + plotDir).Data());
  TCanvas* c1 = new TCanvas();

  // move SigXsec to first place
  for (int i = 0; i < nPar; i++) {
    TString name = names[i];
    if (name.Contains("SigXsec")) {
      if (i > 0) swap(names[i], names[0]);
      break;
    }
  }

  // draw 1D pull distributions
  TString pullFileName = plotDir + "pulls.txt";
  TString resultsFileName = plotDir + "results.txt";
  ofstream pullFile(pullFileName.Data());
  ofstream resultsFile(resultsFileName.Data());
  c1 -> SetRightMargin(0.05);
  gStyle -> SetOptStat(1);
  gStyle -> SetOptFit(1);
  pullFile << "Cuts: " << cutString << endl;
  pullFile << "Mean and RMS of post-fit parameters (directly from histograms):" << endl;
  resultsFile << "Gaussian fits to histograms:" << endl;
  resultsFile << "    parameter       = fit to post-fit distribution" << endl;
  resultsFile << "    parameter_err   = fit to post-fit error distribution" << endl;
  resultsFile << "    parameter_pull  = (value - external_constraint) / error" << endl;
  resultsFile << "    parameter_pull2 = (value - nominal_value) / error" << endl;
  resultsFile << setw(40) << " "
          << setw(13) << "parameter"
          << setw(53) << "parameter_err"
          << setw(50) << "parameter_pull"
          << setw(50) << "parameter_pull2"
          << endl;
  resultsFile << setw(40) << " "
          << setw(8) << "mean" << setw(22) << "width" << setw(19) << "chi2/ndf"
          << setw(8) << "mean" << setw(22) << "width" << setw(19) << "chi2/ndf"
          << setw(8) << "mean" << setw(22) << "width" << setw(19) << "chi2/ndf"
          << setw(8) << "mean" << setw(22) << "width" << setw(19) << "chi2/ndf"
          << endl;
  for (int i = 0; i < nPar; i++) {
    // err = 0: value, 1: err, 2: pull, 3: value/err
    for (int err = 0; err <= 3; err++) {

      // variable name
      TString name = names[i];
      TString nameErr = name + "_err";
      TString outName = name;
      if (err == 1) outName += "_err";
      if (err == 2) outName += "_pull";
      if (err == 3) outName += "_pull2";
      if (err && !toyTree -> FindBranch(prefix + nameErr))
        continue;

      // nominal value
      bool floating = name.Contains("SigXsec") || name.Contains("norm_");
      TString nominal = "0";
      if (floating) nominal = "1";
      if (!floating && err == 2) {
        if (toyTree -> FindBranch(prefixNom + name))
          nominal = prefixNom + name;
      }

      // draw string
      TString drawString = prefix + name;
      if (err == 1) drawString = prefix + nameErr;
      if (err == 2 || err == 3) drawString = "(" + prefix + name + "-" + nominal + ")/" + prefix + nameErr;
      drawString += ">>htemp()";
      cout << "Draw string: " << drawString.Data() << endl;

      // draw
      toyTree -> Draw("1>>htemp(100, 0, 1)"); // set nbins to 100
      toyTree -> Draw(drawString, cutString);
      TH1* htemp = (TH1*) gDirectory -> Get("htemp");
      htemp -> Rebin(rebin);
      double mean = htemp -> GetMean();
      double rms = htemp -> GetRMS();
      double max = htemp -> GetMaximum();

      // adjust x-axis range (draw again)
      int bins = htemp -> GetNbinsX();
      float xMin = htemp -> GetBinLowEdge(1);
      float xMax = htemp -> GetBinLowEdge(bins);
      float xMaxMin = mean + 1.5 * (mean - xMin);
      if (xMax < xMaxMin) {
        bins = (int) (bins * (xMaxMin - xMin) / (xMax - xMin));
        xMax = xMaxMin;
      }
      TString binning = TString::Format("histo(%i,%f,%f)", bins, xMin, xMax);
      drawString.ReplaceAll("htemp()", binning);
      toyTree -> Draw(drawString, cutString);
      delete htemp;
      htemp = (TH1*) gDirectory -> Get("histo");
      htemp -> GetXaxis() -> SetTitle(outName);
      htemp -> GetYaxis() -> SetTitle("Entries");

      // fit
      TF1* func = new TF1("func", "gaus(0)", mean - 5 * rms, mean + 5 * rms);
      func -> SetParameter(0, max);
      func -> SetParameter(1, mean);
      func -> SetParameter(2, rms);
      func -> SetParName(0, "G_const");
      func -> SetParName(1, "G_mean");
      func -> SetParName(2, "G_sigma");
      func -> SetLineColor(2);
      func -> Draw("same");
      htemp -> Fit("func");

      // print
      TString outFileName = plotDir + outName + gfxExt;
      c1 -> Print(outFileName);

      // results log
      if (!err) {
        pullFile << setw(40) << name
                << setw(12) << TString::Format("%.5f", mean) << "  +/-"
                << setw(12) << TString::Format("%.5f", rms) << endl;
      }
      if (err == 0) resultsFile << setw(40) << name;
      resultsFile << setw(10) << TString::Format("%.4f", func -> GetParameter(1)) << " +/-"
              << setw(7) << TString::Format("%.4f", func -> GetParError(1)) << "  "
              << setw(8) << TString::Format("%.4f", func -> GetParameter(2)) << " +/-"
              << setw(7) << TString::Format("%.4f", func -> GetParError(2)) << "  "
              << setw(5) << TString::Format("%.2f", func -> GetChisquare() / func -> GetNDF());

      // clean up
      delete htemp;
    }
    resultsFile << endl;
  }
  system(("cat " + pullFileName).Data());
  system(("cat " + resultsFileName).Data());
  cout << "Pulls written to:       " << pullFileName.Data() << endl;
  cout << "Fit results written to: " << resultsFileName.Data() << endl;
  
  // draw correlations from 2D scatter histograms
  TMatrix corrMatrix(nPar, nPar);
  TH2* hCorr = new TH2F("hCorr", "hCorr", nPar, 0, nPar, nPar, 0, nPar);
  cout << "Reading correlations...";
  c1 -> SetRightMargin(0.12);
  gStyle -> SetOptStat(0);
  gStyle -> SetOptFit(0);
  for (int i = 0; i < nPar; i++) {
    cout << ".";
    flush(cout);
    TString nameX = names[i];
    hCorr -> GetXaxis() -> SetBinLabel(i + 1, nameX);
    hCorr -> GetYaxis() -> SetBinLabel(nPar - i, nameX);
    for (int j = i; j < nPar; j++) {
      TString nameY = names[j];
      TString drawString = prefix + nameY + ":" + prefix + nameX + ">>htemp()";
      toyTree -> Draw(drawString, cutString, "goff");
      TH2* htemp = (TH2*) gDirectory -> Get("htemp");
      double corr = htemp -> GetCorrelationFactor();
      corrMatrix[i][j] = corr;
      hCorr -> SetBinContent(i + 1, nPar - j, corr);
      hCorr -> SetBinContent(j + 1, nPar - i, corr);
      //clean up
      delete htemp;
    }
  }
  cout << endl;
  hCorr -> GetXaxis() -> SetLabelSize(0.015);
  hCorr -> GetYaxis() -> SetLabelSize(0.015);
  hCorr -> GetZaxis() -> SetRangeUser(-1, 1);
  hCorr -> Draw("colz");
  c1 -> Print((plotDir + "corrs" + gfxExt).Data());

  // clean up
  delete c1;
  delete toyTree;
  delete hCorr;
  file -> Close();
  delete file;
}

