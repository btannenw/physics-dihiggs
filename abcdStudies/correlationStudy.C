#include <TFile.h>
#include <TTree.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TLatex.h>
#include "AtlasStyle.h"

#include <iostream>
//#include "include/commonFunctions.h"

void draw2Dplot(TCanvas* c0, TH2D* h0, std::string xtitle, std::string ytitle, std::string filename);
void printProgBar( int percent );
void SetAtlasStyle ();
TStyle* AtlasStyle() ;

void correlationStudy()
{
  // *** 1. Set style, set file, set output directory
  // Load ATLAS style
  SetAtlasStyle();
  
  TFile *fInput = new TFile("dec14_abcd/ttbar-MVA-d0sigBL_MET-add_dphiLepNu-v4-tightMM-141216.root","READ");
  TTree *fTree = (TTree*)fInput->Get("Nominal");

  std::string topDir = "dec14_abcd/correlations/";

  // *** 2. Define histograms and canvasses
  TCanvas* c1 = new TCanvas("c1", "c1", 800, 800);
  TCanvas* c2 = new TCanvas("c2", "c2", 800, 800);
  TCanvas* c3 = new TCanvas("c3", "c3", 800, 800);
  TCanvas* c4 = new TCanvas("c4", "c4", 1000, 800);

  TH2D* h_d0_dphilepnu = new TH2D( "h_d0_dphilepnu", "h_d0_dphilepnu", 100, -5, 5, 100, -3.5, 3.5);
  TH2D* h_d0_drlepnu   = new TH2D( "h_d0_drlepnu", "h_d0_drlepnu",     100, -5, 5, 100, 0, 5);
  TH2D* h_d0_bbpt    = new TH2D( "h_d0_bbpt", "h_d0_bbpt",       100, -5, 5, 100, 0, 1000);
  TH2D* h_d0_drbb    = new TH2D( "h_d0_drbb", "h_d0_drbb",       100, -5, 5, 100, 0, 5);
  TH2D* h_d0_drww    = new TH2D( "h_d0_drww", "h_d0_drww",       100, -5, 5, 100, 0, 5);
  TH2D* h_d0_wwmass  = new TH2D( "h_d0_wwmass", "h_d0_wwmass",   100, -5, 5, 100, 0, 1000);
  TH2D* h_d0_hhmass  = new TH2D( "h_d0_hhmass", "h_d0_hhmass",   100, -5, 5, 100, 0, 2000);
  TH2D* h_d0_bbmass  = new TH2D( "h_d0_bbmass", "h_d0_bbmass",   100, -5, 5, 100, 0, 500);

  TH2D* h_dphilepnu_bbpt    = new TH2D( "h_dphilepnu_bbpt", "h_dphilepnu_bbpt",       100, -3.5, 3.5, 100, 0, 1000);
  TH2D* h_dphilepnu_drbb    = new TH2D( "h_dphilepnu_drbb", "h_dphilepnu_drbb",       100, -3.5, 3.5, 100, 0, 5);
  TH2D* h_dphilepnu_drww    = new TH2D( "h_dphilepnu_drww", "h_dphilepnu_drww",       100, -3.5, 3.5, 100, 0, 5);
  TH2D* h_dphilepnu_wwmass  = new TH2D( "h_dphilepnu_wwmass", "h_dphilepnu_wwmass",   100, -3.5, 3.5, 100, 0, 1000);
  TH2D* h_dphilepnu_hhmass  = new TH2D( "h_dphilepnu_hhmass", "h_dphilepnu_hhmass",   100, -3.5, 3.5, 100, 0, 2000);
  TH2D* h_dphilepnu_bbmass  = new TH2D( "h_dphilepnu_bbmass", "h_dphilepnu_bbmass",   100, -3.5, 3.5, 100, 0, 500);

  TH2D* h_drlepnu_bbpt    = new TH2D( "h_drlepnu_bbpt", "h_drlepnu_bbpt",       100, 0, 5, 100, 0, 1000);
  TH2D* h_drlepnu_drbb    = new TH2D( "h_drlepnu_drbb", "h_drlepnu_drbb",       100, 0, 5, 100, 0, 5);
  TH2D* h_drlepnu_drww    = new TH2D( "h_drlepnu_drww", "h_drlepnu_drww",       100, 0, 5, 100, 0, 5);
  TH2D* h_drlepnu_wwmass  = new TH2D( "h_drlepnu_wwmass", "h_drlepnu_wwmass",   100, 0, 5, 100, 0, 1000);
  TH2D* h_drlepnu_hhmass  = new TH2D( "h_drlepnu_hhmass", "h_drlepnu_hhmass",   100, 0, 5, 100, 0, 2000);
  TH2D* h_drlepnu_bbmass  = new TH2D( "h_drlepnu_bbmass", "h_drlepnu_bbmass",   100, 0, 5, 100, 0, 500);

  TH2D* h_met_bbpt    = new TH2D( "h_met_bbpt", "h_met_bbpt",       100, 0, 1000e3, 100, 0, 1000);
  TH2D* h_met_drbb    = new TH2D( "h_met_drbb", "h_met_drbb",       100, 0, 1000e3, 100, 0, 5);
  TH2D* h_met_drww    = new TH2D( "h_met_drww", "h_met_drww",       100, 0, 1000e3, 100, 0, 5);
  TH2D* h_met_wwmass  = new TH2D( "h_met_wwmass", "h_met_wwmass",   100, 0, 1000e3, 100, 0, 1000);
  TH2D* h_met_hhmass  = new TH2D( "h_met_hhmass", "h_met_hhmass",   100, 0, 1000e3, 100, 0, 2000);
  TH2D* h_met_bbmass  = new TH2D( "h_met_bbmass", "h_met_bbmass",   100, 0, 1000e3, 100, 0, 500);

  // *** 3. Set tree structure and variables to read
  float bbpt;
  float drbb;
  float drww;
  float wwmass;
  float hhmass;
  float bbmass;
  float drlepnu;
  float dphilepnu;
  float lep_d0sigBL;
  float met;
  
  fTree->SetBranchAddress("bbpt", &bbpt);
  fTree->SetBranchAddress("drbb", &drbb);
  fTree->SetBranchAddress("drww", &drww);
  fTree->SetBranchAddress("wwmass", &wwmass);
  fTree->SetBranchAddress("hhmass", &hhmass);
  fTree->SetBranchAddress("bbmass", &bbmass);
  fTree->SetBranchAddress("lep_d0sigBL", &lep_d0sigBL);
  fTree->SetBranchAddress("drlepnu", &drlepnu);
  fTree->SetBranchAddress("dphilepnu", &dphilepnu);
  fTree->SetBranchAddress("nupt", &met);
  
  // *** 4. Start looping! 
  long t_entries = fTree->GetEntries();
  cout << "Tree entries: " << t_entries << endl;

  for(int i = 0; i < fTree->GetEntries(); i++) {
    if ( t_entries > 100) {
      if ((i+1)%(5*t_entries/100)==0)  printProgBar(100*i/t_entries +1); }
    if (i == t_entries-1) {printProgBar(100); cout << endl;}
    
    fTree->GetEntry(i);

    h_d0_dphilepnu->Fill(lep_d0sigBL, dphilepnu);
    h_d0_drlepnu  ->Fill(lep_d0sigBL, drlepnu);
    h_d0_bbpt     ->Fill(lep_d0sigBL, bbpt/1e3); 
    h_d0_drbb     ->Fill(lep_d0sigBL, drbb); 
    h_d0_drww     ->Fill(lep_d0sigBL, drww);  
    h_d0_wwmass   ->Fill(lep_d0sigBL, wwmass/1e3);
    h_d0_hhmass   ->Fill(lep_d0sigBL, hhmass/1e3);
    h_d0_bbmass   ->Fill(lep_d0sigBL, bbmass/1e3);

    h_dphilepnu_bbpt     ->Fill(dphilepnu, bbpt/1e3); 
    h_dphilepnu_drbb     ->Fill(dphilepnu, drbb); 
    h_dphilepnu_drww     ->Fill(dphilepnu, drww);  
    h_dphilepnu_wwmass   ->Fill(dphilepnu, wwmass/1e3);
    h_dphilepnu_hhmass   ->Fill(dphilepnu, hhmass/1e3);
    h_dphilepnu_bbmass   ->Fill(dphilepnu, bbmass/1e3);

    h_drlepnu_bbpt     ->Fill(drlepnu, bbpt/1e3); 
    h_drlepnu_drbb     ->Fill(drlepnu, drbb); 
    h_drlepnu_drww     ->Fill(drlepnu, drww);  
    h_drlepnu_wwmass   ->Fill(drlepnu, wwmass/1e3);
    h_drlepnu_hhmass   ->Fill(drlepnu, hhmass/1e3);
    h_drlepnu_bbmass   ->Fill(drlepnu, bbmass/1e3);

    h_met_bbpt     ->Fill(met, bbpt/1e3); 
    h_met_drbb     ->Fill(met, drbb); 
    h_met_drww     ->Fill(met, drww);  
    h_met_wwmass   ->Fill(met, wwmass/1e3);
    h_met_hhmass   ->Fill(met, hhmass/1e3);
    h_met_bbmass   ->Fill(met, bbmass/1e3);

  }
  
  std::string level = "presel";
  draw2Dplot(c1, h_d0_dphilepnu, "#sigma_{d_{0}}", "#Delta#phi(l,#nu)", (topDir + "d0_philepnu_" + level).c_str());
  
  draw2Dplot(c2, h_d0_bbpt,      "#sigma_{d_{0}}", "bb p_{T} [GeV]", (topDir + "d0_bbpt_" + level).c_str());
  draw2Dplot(c2, h_d0_drbb,      "#sigma_{d_{0}}", "#DeltaR(bb)", (topDir + "d0_drbb_" + level).c_str());
  draw2Dplot(c2, h_d0_drww,      "#sigma_{d_{0}}", "#DeltaR(WW)", (topDir + "d0_drww_" + level).c_str());
  draw2Dplot(c2, h_d0_wwmass,    "#sigma_{d_{0}}", "WW Invariant Mass [GeV]", (topDir + "d0_wwmass_" + level).c_str());
  draw2Dplot(c2, h_d0_hhmass,    "#sigma_{d_{0}}", "hh Invariant Mass [GeV]", (topDir + "d0_hhmass_" + level).c_str());
  draw2Dplot(c2, h_d0_bbmass,    "#sigma_{d_{0}}", "bb Invariant Mass [GeV]", (topDir + "d0_wwmass_" + level).c_str());
  
  draw2Dplot(c3, h_dphilepnu_bbpt,   "#Delta#phi(l,#nu)", "bb p_{T} [GeV]", (topDir + "dphilepnu_bbpt_" + level).c_str());
  draw2Dplot(c3, h_dphilepnu_drbb,   "#Delta#phi(l,#nu)", "#DeltaR(bb)", (topDir + "dphilepnu_drbb_" + level).c_str());
  draw2Dplot(c3, h_dphilepnu_drww,   "#Delta#phi(l,#nu)", "#DeltaR(WW)", (topDir + "dphilepnu_drww_" + level).c_str());
  draw2Dplot(c3, h_dphilepnu_wwmass, "#Delta#phi(l,#nu)", "WW Invariant Mass [GeV]", (topDir + "dphilepnu_wwmass_" + level).c_str());
  draw2Dplot(c3, h_dphilepnu_hhmass, "#Delta#phi(l,#nu)", "hh Invariant Mass [GeV]", (topDir + "dphilepnu_hhmass_" + level).c_str());
  draw2Dplot(c3, h_dphilepnu_bbmass, "#Delta#phi(l,#nu)", "bb Invariant Mass [GeV]", (topDir + "dphilepnu_wwmass_" + level).c_str());
  
  draw2Dplot(c4, h_met_bbpt,   "Missing E_{T} [GeV]", "bb p_{T} [GeV]", (topDir + "met_bbpt_" + level).c_str());
  draw2Dplot(c4, h_met_bbpt,   "Missing E_{T} [GeV]", "bb p_{T} [GeV]", (topDir + "met_bbpt_" + level).c_str());
  draw2Dplot(c4, h_met_drbb,   "Missing E_{T} [GeV]", "#DeltaR(bb)", (topDir + "met_drbb_" + level).c_str());
  draw2Dplot(c4, h_met_drww,   "Missing E_{T} [GeV]", "#DeltaR(WW)", (topDir + "met_drww_" + level).c_str());
  draw2Dplot(c4, h_met_wwmass, "Missing E_{T} [GeV]", "WW Invariant Mass [GeV]", (topDir + "met_wwmass_" + level).c_str());
  draw2Dplot(c4, h_met_hhmass, "Missing E_{T} [GeV]", "hh Invariant Mass [GeV]", (topDir + "met_hhmass_" + level).c_str());
  draw2Dplot(c4, h_met_bbmass, "Missing E_{T} [GeV]", "bb Invariant Mass [GeV]", (topDir + "met_wwmass_" + level).c_str());

}


void draw2Dplot(TCanvas* c0, TH2D* h0, std::string xtitle, std::string ytitle, std::string filename)
{
  c0->cd();

  h0->SetXTitle( xtitle.c_str() );
  h0->SetYTitle( ytitle.c_str() );

  h0->Draw("colz");
  cout<<"ABCD correlation = "<<h0->GetCorrelationFactor()<<endl;

  TLatex lat3;
  lat3.SetTextAlign(9);
  lat3.SetTextFont(62);
  lat3.SetTextSize(0.05);
  lat3.SetNDC();
  
  string s_corr;          // string which will contain the result
  ostringstream os_corr;   // stream used for the conversion
  os_corr << std::fixed;
  os_corr << setprecision(3) << h0->GetCorrelationFactor() ;      
  s_corr = os_corr.str(); // set 'Result' to the contents of the stream
  lat3.DrawLatex(0.35, 0.75, ("Corr. = " + s_corr).c_str());

  c0->Print( (filename + ".png").c_str() );
  c0->Print( (filename + ".eps").c_str() );
}



void printProgBar( int percent )
{
  string bar;
  
  for(int i = 0; i < 50; i++){
    if( i < (percent/2)){
      bar.replace(i,1,"=");
    }else if( i == (percent/2)){
      bar.replace(i,1,">");
    }else{
      bar.replace(i,1," ");
    }
  }
  
  cout<< "\r" "[" << bar << "] ";
  cout.width( 3 );
  cout<< percent << "%     " << std::flush;
}

void SetAtlasStyle ()
{
  static TStyle* atlasStyle = 0;
  std::cout << "\nApplying ATLAS style settings...\n" << std::endl ;
  if ( atlasStyle==0 ) atlasStyle = AtlasStyle();
  gROOT->SetStyle("ATLAS");
  gROOT->ForceStyle();
}

TStyle* AtlasStyle() 
{
  TStyle *atlasStyle = new TStyle("ATLAS","Atlas style");

  // use plain black on white colors
  Int_t icol=0; // WHITE
  atlasStyle->SetFrameBorderMode(icol);
  atlasStyle->SetFrameFillColor(icol);
  atlasStyle->SetCanvasBorderMode(icol);
  atlasStyle->SetCanvasColor(icol);
  atlasStyle->SetPadBorderMode(icol);
  atlasStyle->SetPadColor(icol);
  atlasStyle->SetStatColor(icol);
  //atlasStyle->SetFillColor(icol); // don't use: white fill color for *all* objects

  // set the paper & margin sizes
  atlasStyle->SetPaperSize(20,26);

  // set margin sizes
  atlasStyle->SetPadTopMargin(0.05);
  atlasStyle->SetPadRightMargin(0.14);
  atlasStyle->SetPadBottomMargin(0.16);
  atlasStyle->SetPadLeftMargin(0.16);

  // set title offsets (for axis label)
  atlasStyle->SetTitleXOffset(1.4);
  atlasStyle->SetTitleYOffset(1.4);

  // use large fonts
  //Int_t font=72; // Helvetica italics
  Int_t font=42; // Helvetica
  Double_t tsize=0.05;
  atlasStyle->SetTextFont(font);

  atlasStyle->SetTextSize(tsize);
  atlasStyle->SetLabelFont(font,"x");
  atlasStyle->SetTitleFont(font,"x");
  atlasStyle->SetLabelFont(font,"y");
  atlasStyle->SetTitleFont(font,"y");
  atlasStyle->SetLabelFont(font,"z");
  atlasStyle->SetTitleFont(font,"z");
  
  atlasStyle->SetLabelSize(tsize,"x");
  atlasStyle->SetTitleSize(tsize,"x");
  atlasStyle->SetLabelSize(tsize,"y");
  atlasStyle->SetTitleSize(tsize,"y");
  atlasStyle->SetLabelSize(tsize,"z");
  atlasStyle->SetTitleSize(tsize,"z");

  // use bold lines and markers
  //atlasStyle->SetMarkerStyle(20);
  atlasStyle->SetMarkerStyle(0);
  atlasStyle->SetMarkerSize(1.2);
  atlasStyle->SetHistLineWidth(2.);
  atlasStyle->SetLineStyleString(2,"[12 12]"); // postscript dashes

  // get rid of X error bars 
  //atlasStyle->SetErrorX(0.001);
  // get rid of error bar caps
  atlasStyle->SetEndErrorSize(0.);

  // do not display any of the standard histogram decorations
  atlasStyle->SetOptTitle(0);
  //atlasStyle->SetOptStat(1111);
  atlasStyle->SetOptStat(0);
  //atlasStyle->SetOptFit(1111);
  atlasStyle->SetOptFit(0);

  // put tick marks on top and RHS of plots
  atlasStyle->SetPadTickX(1);
  atlasStyle->SetPadTickY(1);

  return atlasStyle;

}

