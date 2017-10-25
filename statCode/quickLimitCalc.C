#include <TFeldmanCousins>
#include <TMultiGraph>
#include <TGraph>
#include <TCanvas>
#include <TLegend>
#include <iostream>

void quickLimitCalc()
{ 
  TCanvas *c1 = new TCanvas("c1","multigraph",700,500);


  TFile *f0 = new TFile("bbWWinput/may18_rootfiles/hist-data_ttbar_singletop_wjets_zjets_diboson.root","READ");
  TH1D* h_xSM_ttbar = (TH1D*)f0->Get("ttbar_SR_optNoRes_bbpt230_drbb12_drww11_mbb_mww_LogLikelihood_ttbar");
  TH1D* h_x700_ttbar = (TH1D*)f0->Get("ttbar_SR_opt700_bbpt150_drbb11_drww09_mbb_mww_mhh_LogLikelihood_ttbar");
  TH1D* h_x2000_ttbar = (TH1D*)f0->Get("ttbar_SR_opt2000_bbpt350_wwpt360_drww20_mbb_mhh_LogLikelihood_ttbar");
  TH1D* h_xSM_sig = (TH1D*)f0->Get("XhhSM_SR_optNoRes_bbpt230_drbb12_drww11_mbb_mww_LogLikelihood_ttbar");
  TH1D* h_x700_sig = (TH1D*)f0->Get("Xhh700_SR_opt700_bbpt150_drbb11_drww09_mbb_mww_mhh_LogLikelihood_ttbar");
  TH1D* h_x2000_sig = (TH1D*)f0->Get("Xhh2000_SR_opt2000_bbpt350_wwpt360_drww20_mbb_mhh_LogLikelihood_ttbar");

  int xMax = 17;
  //optimal xMax for 
  //                 xSM: 19 (corresponds to < -56)
  //                x700: 18 (corresponds to < -58)
  //               x2000: 17 (corresponds to < -60)
  double optNoPlusLH_xSM_ttbar   = h_xSM_ttbar->Integral(0,19);
  double optNoPlusLH_x700_ttbar  = h_x700_ttbar->Integral(0,18);
  double optNoPlusLH_x2000_ttbar = h_x2000_ttbar->Integral(0,17);
  double optNoPlusLH_xSM_sig   = h_xSM_sig->Integral(0,19);
  double optNoPlusLH_x700_sig  = h_x700_sig->Integral(0,18);
  double optNoPlusLH_x2000_sig = h_x2000_sig->Integral(0,17);

  //                 optNoLH:  nonres,  x700,  x2000  |  optLH:  nonres,  x700,  x2000  |   optNo+LH:  nonres,             x700,                   x2000
  double arrBkg[9] = {           6.88 , 1.366, 0.752,              2.174,  0.350, 0.265,   optNoPlusLH_xSM_ttbar, optNoPlusLH_x700_ttbar, optNoPlusLH_x2000_ttbar};
  double arrSig[9] = {           2.50 , 0.654, 0.522,             1.4577, 0.4176, 0.277,   optNoPlusLH_xSM_sig,   optNoPlusLH_x700_sig,   optNoPlusLH_x2000_sig};
  std::string arrRegion[9] = {"optNoLH-nonres","optNoLH-X700","optNoLH-X2000","optWithLH-nonres","optWithLH-X700","optWithLH-X2000", "optNoLH(+LH)-nonres","optNoLH(+LH)-X700","optNoLH(+LH)-X2000"}
    
  TFeldmanCousins f(0.95);

  const Int_t n = 3;
  Float_t y_noLH[n] = {0.0, 0.0, 0.0};
  Float_t yerr_noLH[n] = {0.0, 0.0, 0.0};
  Float_t y_noLH_plusLH[n] = {0.0, 0.0, 0.0};
  Float_t yerr_noLH_plusLH[n] = {0.0, 0.0, 0.0};
  Float_t y_LH[n] = = {0.0, 0.0, 0.0};
  Float_t yerr_LH[n] = {0.0, 0.0, 0.0};
  Float_t x[n] = {250, 700, 2000};
  Float_t xerr[n] = {0, 0, 0};
  
  double harris = 1;
  double ul_fc, ul_hh = 0;
  double nBkg, nSig, nObs = 0;
  for (int i =0; i<9 ; i++){
    nBkg = arrBkg[i] * harris;
    nSig = arrSig[i] * harris;
    nObs = nBkg ;
    /*    if(nBkg <= 1){
      nObs = 0;
      //nBkg = 0;
      }*/

    ul_fc = f.CalculateUpperLimit(nObs, nBkg);
    //ul_hh  = biagioLimit(nSig, nBkg);

    //std::cout<<arrRegion[i]<<": for nBkg= "<<nBkg<<", nSig= "<<nSig<<", and nObs = "<<nObs<<", the 95% Feldman-Cousins <mu> U.L. is: "<<ul_fc/nSig<<std::endl;

    float eff = 0;
    if (i == 0 || i == 3 || i == 6){ // non-res
      //    N    / (xsec * pb -> fb * BR(h->WW) * BR(h->bb)* comb * lumi) 
      eff = nSig / (2.38 *    1e3   *  0.213    * 0.577    *  2  * 3.2);
    }
    if (i == 1 || i == 4 || i == 7){ // X700
      //    N    / (xsec * pb -> fb * BR(h->WW) * BR(h->bb) * comb * lumi) 
      eff = nSig / (0.18 *    1e3   *  0.213    * 0.577     *  2   * 3.2);
    }
    if (i == 2 || i == 5 || i == 8){ // X2000
      //    N    / (xsec * pb -> fb * BR(h->WW) * BR(h->bb) * comb * lumi) 
      eff = nSig / (0.16 *    1e3   *  0.213    * 0.577     *  2   * 3.2);
    }

    double BR = 0.213*0.577*2; // h->WW* x h->bb x 2(combinatoric)
    if (i < 3){ //       * eff / (lumi   * Br)
      //y_noLH[i] = ul_fc * 2e-2/ (3.2 * harris * 0.0715);
      y_noLH[i] = 1e-3 * ul_fc / (3.2 * harris * eff) / BR;
      //std::cout<<"y_noLH["<<i<<"]: "<<y_noLH[i]<<std::endl;
    }
    else if (i >= 3 && i < 6) {
      //y_LH[i-3] = ul_fc * 2e-2/ (3.2 * harris * 0.0715);
      y_LH[i-3] = 1e-3 * ul_fc / (3.2 * harris * eff) / BR;
      //std::cout<<"y_LH["<<i<<"]: "<<y_LH[i-3]<<std::endl;
    }
    else  {
      //std::cout<<"i: "<<i<<std::endl;
      //y_LH[i-3] = ul_fc * 2e-2/ (3.2 * harris * 0.0715);
      y_noLH_plusLH[i-6] = 1e-3 * ul_fc / (3.2 * harris * eff) / BR;
      //std::cout<<"y_noLH_plusLH["<<i<<"]: "<<y_noLH_plusLH[i-6]<<std::endl;
      std::cout<<arrRegion[i]<<": for nBkg= "<<nBkg<<", nSig= "<<nSig<<", and nObs = "<<nObs<<", the Xsec U.L. is: "<<y_noLH_plusLH[i-6]<<std::endl;

    }
  }

  TGraphErrors *g_noLH = new TGraphErrors(n, x, y_noLH, xerr, yerr_noLH);
  TGraphErrors *g_LH = new TGraphErrors(n, x, y_LH, xerr, yerr_LH);
  TGraphErrors *g_noLH_plusLH = new TGraphErrors(n, x, y_noLH_plusLH, xerr, yerr_noLH_plusLH);

  g_noLH->SetTitle("");
  g_noLH->SetLineColor(kBlue);
  g_noLH->SetMarkerColor(kBlue);
  g_noLH->SetMarkerStyle(20);
  g_LH->SetTitle("LH");
  g_LH->SetLineColor(kBlack);
  g_LH->SetMarkerColor(kBlack);
  g_LH->SetMarkerStyle(21);
  g_noLH_plusLH->SetTitle("NoLH_plusLH");
  g_noLH_plusLH->SetLineColor(kBlue);
  g_noLH_plusLH->SetMarkerColor(kGreen+3);
  g_noLH_plusLH->SetMarkerStyle(22);
  
  c1->SetLogy();
  g_LH->GetYaxis()->SetRangeUser(0.3,15);
  g_noLH->GetYaxis()->SetRangeUser(0.3,15);
  g_noLH->GetYaxis()->SetTitle("95% CL Limit on #sigma_{pp#rightarrow X} X BR(X#rightarrow hh) [pb]");
  g_noLH->GetXaxis()->SetTitle("X Mass [GeV]");
  
  TLegend *l1 = new TLegend(0.55,0.65,0.83,0.85);
  l1->AddEntry(g_noLH,"NoLH (opt)","ap");
  l1->AddEntry(g_LH,"LH (opt)","ap");
  l1->AddEntry(g_noLH_plusLH,"NoLH (opt) + LH","ap");

  c1->cd();  
  g_noLH->Draw("ALP");
  g_LH->Draw("same LP");
  g_noLH_plusLH->Draw("same LP");
  l1->Draw("same");
  
  //TCanvas *c2 = new TCanvas("c2","FC_functional",700,500);
  //functionalFC(c2);
  
  return;
}

void functionalFC(TCanvas* c0)
{
  const Int_t n2 =15 ;
  Float_t y2[n2] = {3.09, 4.14, 5.22, 5.25, 5.76, 6.26, 6.75, 6.81, 7.29, 7.77, 7.82, 8.29, 8.34, 8.80, 8.94};
  Float_t yerr2[n2] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  Float_t x2[n2] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
  Float_t xerr2[n2] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  
  const Int_t n = 25 ;
  Float_t y[n] = {};
  Float_t yerr[n] = {};
  Float_t x[n] = {};
  Float_t xerr[n] = {};

  TFeldmanCousins g(0.95);

  for(int i=0; i< n; i++){
    float b = i;
    y[i]= g.CalculateUpperLimit(b,b);
    std::cout<<"b: "<<b<<"\tFC U.L: "<<g.CalculateUpperLimit(b,b)<<std::endl;
    yerr[i] = 0;
    x[i] = b;
    xerr[i] = 0;
  }

  TGraphErrors *g_FC = new TGraphErrors(n, x, y, xerr, yerr);
  TGraphErrors *g_FC2 = new TGraphErrors(n, x2, y2, xerr2, yerr2);
  g_FC->SetMarkerStyle(20);
  g_FC2->SetMarkerStyle(21);
  g_FC2->SetMarkerColor(kBlue);
  c2->cd();
  g_FC->Draw("AP");
  g_FC2->Draw("same P");

  return;
}


double handFeldmanCousins(double nSig, double nBkg)
{
 
  Float_t n0[8]   = {   0,    1,    2,    3,    4,     5,     6,     7};
  Float_t b0[8]   = {3.09, 5.14, 6.72, 8.25, 9.76, 11.26, 12.75, 13.81};
  Float_t b0p5[8] = {2.63, 4.64, 6.22, 7.75, 9.26, 10.76, 12.25, 13.31};
  Float_t b1[8]   = {2.33, 4.14, 5.72, 7.25, 8.76, 10.26, 11.75, 12.81};
  Float_t b1p5[8] = {2.05, 3.69, 5.22, 6.75, 8.26,  9.76, 11.25, 12.31};
  Float_t b2[8]   = {1.78, 3.30, 4.72, 6.25, 7.76,  9.26, 10.75, 11.81};
  Float_t b2p5[8] = {1.78, 2.95, 4.25, 5.75, 7.26,  8.76, 10.25, 11.31};
  Float_t b3[8]   = {1.63, 2.63, 3.84, 5.25, 6.76,  8.26,  9.75, 10.81};
  Float_t b3p5[8] = {1.63, 2.33, 3.46, 4.78, 6.26,  7.76,  9.25, 10.31};
  Float_t b4[8]   = {1.57, 2.08, 3.11, 4.35, 5.76,  7.26,  8.75,  9.81};
  Float_t b5[8]   = {1.54, 1.88, 2.49, 3.58, 4.84,  6.26,  7.75,  8.81};
  Float_t b6[8]   = {1.52, 1.78, 2.28, 2.91, 4.05,  5.33,  6.75,  7.81};
  Float_t b7[8]   = {1.51, 1.73, 2.11, 2.69, 3.35,  4.52,  5.82,  6.81};


}
