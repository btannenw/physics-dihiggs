//
// This script is kept here as reference
// It creates the usual limit plot
// Copy in analysis-specific directory then edit as needed
//

void ATLASLabel2(Double_t x,Double_t y,int Preliminary,Color_t color);

void plotLimit(Int_t iv=62, Int_t NTOT=2, char* wsname, bool isExpected) {

  gROOT->SetMacroPath((TString(getenv("WORKDIR"))+"/macros").Data());
  gROOT->LoadMacro("AtlasStyle.C");
  gROOT->LoadMacro("AtlasLabels.C");
  SetAtlasStyle();

  const Int_t Tot =  7; 

  Double_t FMSA[Tot]        = {  0.0  }; 
  Double_t likelihd_Observed[Tot]  = {0.0}; 
  Double_t likelihd_Observed_PCL[Tot]  = {0.0}; 

  Int_t NPROBLEM = 0;
  Int_t mass_problem[1]={110};

  // if(iv == 22) NPROBLEM = 9;
  //  if (NTOT == 0 ) { NPROBLEM =1; mass_problem[0] = 155; }
  //  if (NTOT == 1 ) { NPROBLEM =4; mass_problem[0] = 110; mass_problem[1] = 130; mass_problem[2] = 180; mass_problem[3] = 560; }


  char* finalstate[5]={"WH","ZH","VH", "ZvvH", "VH3"};
  Double_t likelihd_c[Tot]  = {  0.0  }; 
  Double_t likelihd_p1[Tot] = {  0.0  };
  Double_t likelihd_m1[Tot] = {  0.0  };
  Double_t likelihd_p2[Tot] = {  0.0  };
  Double_t likelihd_m2[Tot] = {  0.0  };

  Double_t likelihd_p1_err[Tot] = {0.0};
  Double_t likelihd_m1_err[Tot] = {0.0};
  Double_t likelihd_p2_err[Tot] = {0.0};
  Double_t likelihd_m2_err[Tot] = {0.0};

  // Int_t NTOT=3; //0 ee; 1 emu; 2 mumu; 3 combined

  TString filesdir = getenv("WORKDIR");
  filesdir += "/root-files/";
  filesdir += wsname;
  if(isExpected)
    filesdir += "_exp/";
  else
    filesdir += "_obs/";

  TString plotdir = getenv("WORKDIR");
  plotdir+="/plots/";
  plotdir+=wsname;
  plotdir+="/final/";
  system((TString("mkdir -vp ")+plotdir).Data());

  if(iv == 27) NPROBLEM = 0;

  //FILE *pFile4 = fopen(Form("limit_tex/Limit_v%i_%s.tex", iv, finalstate[NTOT]), "w");
  FILE *pFile4 = fopen(Form("%s/Limit_v%i_%s.tex", plotdir.Data(), iv, finalstate[NTOT]), "w");
  fprintf(pFile4,"  \\begin{tabular}{c|c|c|c|c|c} \\hline \n ");
  fprintf(pFile4,"%12s  & %12s & %12s & %12s &  %12s & %12s \\\\ \\hline \n", "mass", "Exp.", "$+2\\sigma$",  "$+1\\sigma$",  "$-1\\sigma$", "$-2\\sigma$" );
  Double_t t_mass, t_tanB, observed, upperlimit, upperlimit_p1, upperlimit_m1, upperlimit_p2, upperlimit_m2, tmp1, tmp2;
  Int_t iline = 0;

  for(int i=0; i< Tot; i++) {
    int mass = 110 + i*5;
    cout << "Collecting for mass " << mass << endl;
    TString fileName(Form("%s/%i.root", filesdir.Data(), mass));
    cout << fileName << endl;
    TFile* file = new TFile(fileName,"READ");
    TH1D* info = (TH1D*)file->Get("limit");
    // position
    // value
    t_mass = mass;
    observed  = info->GetBinContent(1);
    upperlimit  = info->GetBinContent(2);
    upperlimit_m1 = info->GetBinContent(5);
    upperlimit_p1 = info->GetBinContent(4);
    upperlimit_m2 = info->GetBinContent(6);
    upperlimit_p2 = info->GetBinContent(3);

    cout<<t_mass<<"  "<<observed<<"  "<<"  "<<upperlimit<<"  "<< upperlimit_m2<<"  "<< upperlimit_m1<<"  "<<  upperlimit_p1<<"  "<< upperlimit_p2
      <<endl;

    FMSA[iline] = t_mass;
    likelihd_Observed[iline] = observed; 
    likelihd_c[iline] = upperlimit;
    likelihd_p1[iline] = upperlimit_p1;
    likelihd_m1[iline] = upperlimit_m1;
    likelihd_p2[iline] = upperlimit_p2;
    likelihd_m2[iline] = upperlimit_m2;

    fprintf(pFile4,"%12i  & %12.2f & %12.2f & %12.2f &  %12.2f & %12.2f \\\\ \n ",t_mass,   upperlimit,
        upperlimit_p2, upperlimit_p1, upperlimit_m1,  upperlimit_m2);

    likelihd_Observed_PCL[iline] = observed; 

    if( observed < upperlimit_m1) likelihd_Observed_PCL[iline] = upperlimit_m1;

    if( fabs(likelihd_m2[iline] -likelihd_m1[iline])<0.0001) likelihd_m2[iline] = 0;

    iline++;
    if(iline == Tot) break;

  }

  fprintf(pFile4,"  \\hline \n");
  fprintf(pFile4,"  \\end{tabular} \n");
  fclose(pFile4); //file storing result

  likelihd_p2[Tot-1] = likelihd_p2[Tot-2] + (likelihd_p1[Tot-1] - likelihd_p1[Tot-2]);
  //Get the errors for 1 or 2 sigma deviation
  for(Int_t i=0; i<Tot; i++) {

    likelihd_p1_err[i] = likelihd_p1[i]-likelihd_c[i];
    likelihd_p2_err[i] = likelihd_p2[i]-likelihd_c[i];
    likelihd_m1_err[i] = likelihd_c[i]-likelihd_m1[i];
    likelihd_m2_err[i] = likelihd_c[i]-likelihd_m2[i];



    // cout<<likelihd_p2_err[i]<<"  "<<likelihd_p1_err[i]<<"  "<< likelihd_m1_err[i]<<"  " <<likelihd_m2_err[i]<<endl;

  }

  //////////////////////count how many mass points have been sucessfully calculated
  Int_t NPLOT= 0;

  for(Int_t i=0; i<Tot; i++) if ( likelihd_c[i] >0.0)  NPLOT++;
  cout<<"NPLOT: "<<NPLOT<<" iline "<<iline-1<<endl;



  for(Int_t i=0; i<NPLOT; i++) cout<<FMSA[i]<<"  "<<likelihd_c[i]<<"  "<<likelihd_Observed[i]<<endl;
  Double_t zero[Tot]={0.0};

  TGraphAsymmErrors *likelihd_limit_c0j = new TGraphAsymmErrors(NPLOT,FMSA,likelihd_c,zero,zero,zero,zero);
  likelihd_limit_c0j->SetLineWidth(2.0);
  likelihd_limit_c0j->SetLineStyle(2);
  likelihd_limit_c0j->SetLineColor(1);

  TGraphAsymmErrors *likelihd_limit_Observed_PCL = new TGraphAsymmErrors(NPLOT,FMSA,likelihd_Observed_PCL,zero,zero,zero,zero);
  likelihd_limit_Observed_PCL->SetMarkerStyle(20);
  likelihd_limit_Observed_PCL->SetMarkerSize(1.0);
  likelihd_limit_Observed_PCL->SetLineStyle(1);
  likelihd_limit_Observed_PCL->SetLineColor(1);
  likelihd_limit_Observed_PCL->SetLineWidth(2.0);

  TGraphAsymmErrors *likelihd_limit_Observed = new TGraphAsymmErrors(NPLOT,FMSA,likelihd_Observed,zero,zero,zero,zero);
  likelihd_limit_Observed->SetMarkerStyle(20);
  likelihd_limit_Observed->SetMarkerSize(1.0);
  likelihd_limit_Observed->SetLineStyle(1);
  likelihd_limit_Observed->SetLineColor(1);
  likelihd_limit_Observed->SetLineWidth(2.0);




  TGraphAsymmErrors *likelihd_limit_1sigma0j = new TGraphAsymmErrors(NPLOT,FMSA,likelihd_c,zero,zero,likelihd_m1_err,likelihd_p1_err);
  likelihd_limit_1sigma0j->SetFillColor(kGreen);

  TGraphAsymmErrors *likelihd_limit_2sigma0j = new TGraphAsymmErrors(NPLOT,FMSA,likelihd_c,zero,zero,likelihd_m2_err,likelihd_p2_err);
  likelihd_limit_2sigma0j->SetFillColor(kYellow);


  TCanvas *MyC = new TCanvas("MyC", "",430,430,600,500);
  MyC->Divide();

  MyC->cd(1); gPad->SetLogy(0);

  TMultiGraph *likelihd_limit_0j = new
    TMultiGraph("","; m_{H} [GeV]; 95% C.L. limit on #sigma/#sigma_{SM}");
  likelihd_limit_0j->Add(likelihd_limit_2sigma0j,"E3");
  likelihd_limit_0j->Add(likelihd_limit_1sigma0j,"E3");
  likelihd_limit_0j->Add(likelihd_limit_c0j, "L");
  likelihd_limit_0j->Add(likelihd_limit_Observed, "PL");
  Double_t ymax=20.0;

  if(NTOT == 4) { ymax = 8; };
  if(NTOT == 1) { ymax = 25; };

  Double_t text2Y=ymax*0.83;
  Double_t text1Y=ymax*0.90;
  Double_t text3Y=ymax*0.90;

  likelihd_limit_0j->SetMaximum(ymax);
  // if(NTOT == 1)  likelihd_limit_0j->SetMaximum(40.0);
  likelihd_limit_0j->SetMinimum(0.0);
  likelihd_limit_0j->Draw("AP");

  gPad->RedrawAxis();

  TLegend *legend = new TLegend(.2,0.65,0.55,0.85);
  legend->SetBorderSize(0);
  legend->AddEntry(likelihd_limit_Observed,"Observed (CLs)","PL");
  legend->AddEntry(likelihd_limit_c0j,"Expected (CLs)","L");
  legend->AddEntry(likelihd_limit_1sigma0j, "#pm 1#sigma","F");
  legend->AddEntry(likelihd_limit_2sigma0j, "#pm 2#sigma","F");

  legend->SetFillColor(0);
  legend->Draw();

  TLatex *text2;



  if(iv == 30 ){
    if(NTOT==0) text2 =      new TLatex(124, text2Y, "WH#rightarrow l#nub#bar{b}  (StatsOnly)");
    else if(NTOT==1) text2 = new TLatex(124, text2Y, "ZH#rightarrow l^{+}l^{-} b#bar{b}  (StatsOnly)");
    else if(NTOT==2) text2 = new TLatex(124, text2Y, "WH/ZH, H#rightarrow b#bar{b}  (StatsOnly)");
    else if(NTOT==3) text2 = new TLatex(124, text2Y, "ZH#rightarrow #nu#bar{#nu} b#bar{b}  (StatsOnly)");
    else if(NTOT==4) text2 = new TLatex(124, text2Y, "VH(b#bar{b}), combined  (StatsOnly)");
  }else if(iv == 43){					  
    if(NTOT==0) text2 =      new TLatex(125, text2Y, "WH, H#rightarrow bb,  (Overall)");
    else if(NTOT==1) text2 = new TLatex(125, text2Y, "ZH, H#rightarrow bb,  (Overall )");
    else if(NTOT==2) text2 = new TLatex(125, text2Y, "WH/ZH, H#rightarrow bb,  (Overall)");
    else if(NTOT==3) text2 = new TLatex(125, text2Y, "ZvvH, H#rightarrow bb,  (Overall)");
    else if(NTOT==4) text2 = new TLatex(125, text2Y, "VH#rightarrow bb,  (Overall)");
  }else if(iv == 62){		
    if(NTOT==0)      text2 = new TLatex(125, text2Y, "WH#rightarrow l#nub#bar{b} ");
    else if(NTOT==1) text2 = new TLatex(125, text2Y, "ZH#rightarrow l^{+}l^{-} b#bar{b}");
    else if(NTOT==2) text2 = new TLatex(125, text2Y, "WH/ZH, H#rightarrow b#bar{b}");
    else if(NTOT==3) text2 = new TLatex(125, text2Y, "ZH#rightarrow #nu#bar{#nu} b#bar{b}");
    else if(NTOT==4) text2 = new TLatex(124.5, text2Y, "VH(b#bar{b}), combined");			  

  }else{
    if(NTOT==0)      text2 = new TLatex(125, text2Y, "WH#rightarrow l#nub#bar{b} ");
    else if(NTOT==1) text2 = new TLatex(125, text2Y, "ZH#rightarrow l^{+}l^{-} b#bar{b}");
    else if(NTOT==2) text2 = new TLatex(125, text2Y, "WH/ZH, H#rightarrow b#bar{b}");
    else if(NTOT==3) text2 = new TLatex(125, text2Y, "ZH#rightarrow #nu#bar{#nu} b#bar{b}");
    else if(NTOT==4) text2 = new TLatex(124.5, text2Y, "VH(b#bar{b}), combined");

  }

  text2->SetTextSize(0.03);
  text2->Draw();

  ATLASLabel2(110.5, text3Y, 2, 1);


  Double_t lumi=4.7;
  if (NTOT ==1 ) lumi = 4.7;
  if (NTOT ==3 ) lumi = 4.6;
  TLatex *text1 = new TLatex(125, text1Y, Form("#sqrt{s} = 7 TeV,  #int Ldt = %3.1f fb^{-1}", lumi));
  if(NTOT==4) text1 = new TLatex(124.5, text1Y, "#sqrt{s} = 7, 8 TeV,  #int Ldt = 4.6-4.7, 13 fb^{-1}" );
  text1->SetTextSize(0.03);
  text1->Draw();

  TLine* l1 = new TLine(FMSA[0], 1.0, FMSA[NPLOT-1], 1.0);
  l1->SetLineWidth(2.0);
  l1->SetLineStyle(2);
  l1->SetLineColor(33);
  l1->Draw();


//  MyC->Print(Form("figure/SMHiggs_%s_v%i.pdf", finalstate[NTOT], iv));
//  MyC->Print(Form("figure/SMHiggs_%s_v%i.png", finalstate[NTOT], iv));
//  MyC->Print(Form("figure/SMHiggs_%s_v%i.eps", finalstate[NTOT], iv));

  MyC->Print(Form("%s/SMHiggs_%s_v%i.pdf", plotdir.Data(), finalstate[NTOT], iv));
  MyC->Print(Form("%s/SMHiggs_%s_v%i.eps", plotdir.Data(), finalstate[NTOT], iv));
  MyC->Print(Form("%s/SMHiggs_%s_v%i.png", plotdir.Data(), finalstate[NTOT], iv));
  MyC->Print(Form("%s/SMHiggs_%s_v%i.root", plotdir.Data(), finalstate[NTOT], iv));

}

void ATLASLabel2(Double_t x,Double_t y,int Preliminary,Color_t color)
{
  TLatex l; //l.SetTextAlign(12); l.SetTextSize(tsize);
  //l.SetNDC();
  l.SetTextFont(72);
  l.SetTextColor(color);

  //  double delx = 0.115*696*gPad->GetWh()/(472*gPad->GetWw());
  double delx = 6;

  l.DrawLatex(x,y,"ATLAS");
  if (Preliminary) {
    TLatex p;
    p.SetTextFont(42);
    p.SetTextColor(color);
    if (Preliminary==2) p.DrawLatex(x+delx,y,"Internal"); 
    if (Preliminary==1) p.DrawLatex(x+delx,y,"Preliminary");
    if (Preliminary==3) p.DrawLatex(x+delx,y,"Prelim.");
    //    p.DrawLatex(x+delx,y,"Preliminary (Simulation)");
    //    p.DrawLatex(x,y,"#sqrt{s}=900GeV");
  }
}

