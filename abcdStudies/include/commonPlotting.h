// Date:    Feb 8, 2017
// Author:  Benjamin Tannenwald
// Purpose: Header file containing 'general' plotting functions used in code for ABCD estimation of QCD

TH1D* getErrorHisto(THStack *hs)
{
  //TH1D* h_err = new TH1D();

  TListIter next(hs->GetHists());
  TObject *object;

  //set first histogram
  object=next();
  TH1D* h0=(TH1D*)object;
  TH1D* sum=(TH1D*)h0->Clone();
  //cout << "getErrorHisto: creating sum using " << h0->GetName() << endl;
  
  //loop over rest
  while( (object=next()) ) 
    {
      h0=(TH1D*)object;
      //cout << "getErrorHisto: creating sum using " << h0->GetName() << endl;
      sum->Add(h0);
    }

  /*  h_err=sum->Clone()
  for (int i =0; i < sum->GetNbinsX(); i++)
    h_err->SetBinContent(i, sum->GetBinError(i));
  */

  sum->SetLineColor(0);
  sum->SetMarkerStyle(0);
  sum->SetFillColor(kBlack);
  sum->SetFillStyle(3004);

  return sum;
}

TH1D* returnRatioHistogram(TH1D* data, TH1D* pred)
{
  TH1D* h_ratio = (TH1D*)data->Clone();
  double binError, ratio, errData, errPred = 0;

  for(int b = 0; b < h_ratio->GetNbinsX() + 1; b++) {
    binError = 0;
    errPred  = 0;
    errData  = 0;
    ratio = 0.15;
      
    if (pred->GetBinContent(b) > 0 ) {
      ratio =  data->GetBinContent(b) / pred->GetBinContent(b);
      errData = data->GetBinContent(b) != 0 ? (data->GetBinError(b)/data->GetBinContent(b))*(data->GetBinError(b)/data->GetBinContent(b)) : 0;
      errPred = pred->GetBinContent(b) != 0 ? (pred->GetBinError(b)/pred->GetBinContent(b))*(pred->GetBinError(b)/pred->GetBinContent(b)) : 0;
      binError = ratio * sqrt( errData + errPred);
      if(data->GetBinContent(b) != 0) {
	h_ratio->SetBinContent(b, ratio);
	h_ratio->SetBinError(b, binError);
      }
    }
    
  }
  return h_ratio;
}

void makeStack(TObjArray *array, TCanvas *c0, std::string region, std::string phaseSpace, std::string label, bool doNorm, bool addQCD, bool drawUnblinded = false)
{
  //c0->Clear();
  std::string hname = (region + "_" + phaseSpace).c_str();

  if(drawUnblinded)  {
    if (region.find("opt700ichep")!=string::npos) 
	hname = ("opt700ichep_" +  phaseSpace).c_str();
    else if (region.find("opt2000ichep")!=string::npos) 
	hname = ("opt2000ichep_" +  phaseSpace).c_str();
    else if (region.find("reOpt500")!=string::npos)
        hname = ("reOpt500_" +  phaseSpace).c_str();
    else if (region.find("reOpt700")!=string::npos) 
	hname = ("reOpt700_" +  phaseSpace).c_str();
    else if (region.find("reOpt2000")!=string::npos) 
	hname = ("reOpt2000_" +  phaseSpace).c_str();
    else if (region.find("reOptNonRes")!=string::npos) 
	hname = ("reOptNonRes_" +  phaseSpace).c_str();
  }

  THStack *hs0 = new THStack("hs0","hs0");
  //TH1D *ttbar = (TH1D*)array->FindObject( ("ttbar_" + hname).c_str() );
  TH1D *ttbar = returnArrayHisto(array, ("ttbar_" + hname).c_str() );
  TH1D *data = new TH1D();
  TH1D *QCD = new TH1D();
  TH1D *Wv221 = new TH1D();
  TH1D *Z = new TH1D();
  TH1D *SingleTop = new TH1D();
  TH1D *Dibosons = new TH1D();

  if(verbose) cout<<"makeStack: post TH1D declare"<<endl;
  if(verbose) cout << "makeStack: post TH1D declare, addQCD: " << addQCD << endl;
  if(addQCD)
    QCD = returnArrayHisto(array, ("QCD_" + hname).c_str() );

  if(!onlyTTbar) {
    Wv221 = returnArrayHisto(array, ("Wv221_" + hname).c_str() );

    if(lepType=="IsoOR"){
      Dibosons = returnArrayHisto(array, ("Dibosonsv221_" + hname).c_str() );
      Z = returnArrayHisto(array, ("Zv221_" + hname).c_str() );
    }
    else if(lepType=="TightMM"){
      Dibosons = returnArrayHisto(array, ("Dibosons_" + hname).c_str() );
      Z = returnArrayHisto(array, ("Z_" + hname).c_str() );
    }
    SingleTop = returnArrayHisto(array, ("SingleTop_" + hname).c_str() );
  }
  if(verbose) cout<<"makeStack: post other backgrounds"<<endl;
  //cout <<"łłłł ttbar histo: " << ("ttbar_" + hname).c_str() <<endl;

  hname = (region + "_" + phaseSpace).c_str(); 
  std::string dataname = hname;
  if (!addSR && hname.find("mBBcr")==string::npos && hname.find("regionA_" + abcdSystematic)!=string::npos)
    dataname = ("mBBcr_" + region + "_" + phaseSpace).c_str(); // FIXME: 17-2-17, will this give me ABCD region plots with better naming?
  if(verbose)
    cout <<"łłłł data histo: " << ("data_" + dataname).c_str() <<endl;
  data = returnArrayHisto(array, ("data_" + dataname).c_str() );
    
  std::string xtitle =ttbar->GetXaxis()->GetTitle();
  std::string ytitle =ttbar->GetYaxis()->GetTitle();
  if(verbose) cout<<"makeStack: pre scaling and coloring"<<endl;

  ttbar->SetLineColor(kOrange);
  ttbar->SetFillColor(kOrange);
 
  if(addQCD){
    QCD->SetLineColor(kGray+1);
    QCD->SetFillColor(kGray+1);
  }
  
  if(!onlyTTbar) {
    Wv221->SetLineColor(kGreen+1);
    Wv221->SetFillColor(kGreen+1);
    Z->SetLineColor(kRed);
    Z->SetFillColor(kRed);
    Dibosons->SetLineColor(kBlue);
    Dibosons->SetFillColor(kBlue);
    SingleTop->SetLineColor(kYellow+2);
    SingleTop->SetFillColor(kYellow+2);
  }
  if(verbose) cout<<"makeStack: post coloring"<<endl;

  data->SetMarkerStyle(20);
  data->SetMarkerColor(kBlack);
  data->SetLineColor(kBlack);
  if(verbose) cout<<"makeStack: post data style"<<endl;

  TH1D* hTot = (TH1D*)ttbar->Clone();
  if(verbose) cout<<"makeStack: ttbar->GetNbinsX(): "<< ttbar->GetNbinsX() << endl; 
  if(verbose) cout<<"makeStack: QCD->GetNbinsX(): "<< QCD->GetNbinsX() << endl; 
  if(addQCD )  hTot->Add( QCD );
  if(verbose) cout<<"makeStack: post QCD add"<<endl; 
  if(!onlyTTbar){
    hTot->Add( Wv221 );
    if(verbose) cout<<"makeStack: Wv221->GetNbinsX(): "<< Wv221->GetNbinsX() << endl; 
    if(verbose) cout<<"makeStack: post W+jets add"<<endl; 
    hTot->Add( Z );
    if(verbose) cout<<"makeStack: Z->GetNbinsX(): "<< Z->GetNbinsX() << "\t Z->GetXaxis()->GetXmin(): "<< Z->GetXaxis()->GetXmin() << "\t Z->GetXaxis()->GetXmax(): "<< Z->GetXaxis()->GetXmax() << endl; 
    if(verbose) cout<<"makeStack: post Z+jets add"<<endl; 
    hTot->Add( Dibosons );
    if(verbose) cout<<"makeStack: Dibosons->GetNbinsX(): "<< Dibosons->GetNbinsX() << endl; 
    if(verbose) cout<<"makeStack: Dibosons->GetNbinsX(): "<< Dibosons->GetNbinsX() << "\t Dibosons->GetXaxis()->GetXmin(): "<< Dibosons->GetXaxis()->GetXmin() << "\t Dibosons->GetXaxis()->GetXmax(): "<< Dibosons->GetXaxis()->GetXmax() << endl; 
    if(verbose) cout<<"makeStack: post Dibosons add"<<endl; 
    hTot->Add( SingleTop );
    if(verbose) cout<<"makeStack: post single top add"<<endl; 
  }
  double tot = hTot->Integral();
  if(verbose) cout<<"makeStack: post totaling"<<endl;
 
  double sigLow = 0;
  double sigHigh = 0;

  if (doNorm){
    ttbar->Scale(1/tot);
    if(addQCD) QCD->Scale(1/tot);
    if(!onlyTTbar){
      Wv221->Scale(1/tot);
      Z->Scale(1/tot);
      Dibosons->Scale(1/tot);
      SingleTop->Scale(1/tot);
    }
    ytitle = ("Normalized " + ytitle).c_str();
  
  }
  if(verbose) cout<<"makeStack: post doNorm"<<endl;

  hs0->Add(ttbar);
  if(!onlyTTbar){
    hs0->Add(Wv221);
    hs0->Add(Z);
    hs0->Add(Dibosons);
    hs0->Add(SingleTop);
  }
  if (addQCD) hs0->Add(QCD);
  TH1D* hs0_error = getErrorHisto(hs0); 
  //hs0_error->Print("all");
  //THStack *hsErr = (THStack*)hs0->Clone();
  //hsErr->Add(hs0_error);

  if(verbose) cout<<"makeStack: post stacking"<<endl;
  
  //cout<<"hs0->GetMaximum(): "<<hs0->GetMaximum()<<"\tXhh700->GetMaximum(): "<<Xhh700->GetMaximum()/Xhh700->Integral()<<endl;
  double stackMax = hs0_error->GetBinContent(hs0_error->GetMaximumBin()) + hs0_error->GetBinError(hs0_error->GetMaximumBin());

  //if (hs0_error->GetMaximum() < data->GetMaximum())
  if (stackMax < data->GetMaximum())
    hs0->SetMaximum( data->GetMaximum()*1.3);
  else
    //hs0->SetMaximum( hs0_error->GetMaximum()*1.2);
    hs0->SetMaximum( stackMax*1.3);    
  TLegend *leg1 = new TLegend();  
  if(!onlyTTbar){
    if(addQCD) leg1=new TLegend(.65,.60,.85,.90);
    else       leg1=new TLegend(.58,.53,.73,.83);

    if(varABCD.find("d0sigBL")!=string::npos)
      leg1=new TLegend(.68,.53,.83,.83);
  }
  else{
    if(addQCD) leg1=new TLegend(.65,.64,.85,.82);
    else       leg1=new TLegend(.58,.69,.73,.82);
  }
  leg1->SetFillStyle(0);
  leg1->SetLineWidth(0);
  leg1->AddEntry(data,"Data","ep");
  leg1->AddEntry(ttbar,"t#bar{t}","f");
  if(addQCD) leg1->AddEntry(QCD,"DD Bkg","f");
  if(!onlyTTbar){
    leg1->AddEntry(Wv221,"W+jets","f");
    leg1->AddEntry(SingleTop,"Single Top","f");
    leg1->AddEntry(Dibosons,"Dibosons","f");
    leg1->AddEntry(Z,"Z+jets","f");
  }
  leg1->AddEntry(hs0_error,"MC stat err","f");

  TLatex lat2;
  lat2.SetTextAlign(9);
  lat2.SetTextFont(62);
  lat2.SetTextSize(0.05);
  lat2.SetNDC();

  TLatex lat4;
  lat4.SetTextAlign(9);
  lat4.SetTextFont(62);
  lat4.SetTextSize(0.04);
  lat4.SetNDC();

  TLatex lat5;
  lat5.SetTextAlign(9);
  lat5.SetTextFont(62);
  lat5.SetTextSize(0.038);
  lat5.SetNDC();

  c0->cd();
  TPad *pad1 =new TPad("pad1","pad1",0,0.2,1,1);
  pad1->Draw();
  pad1->cd();
 
  hs0->Draw("HIST");
  hs0->GetXaxis()->SetLabelSize(0);
  hs0->GetYaxis()->SetTitleOffset(1.7);
  hs0->GetXaxis()->SetTitle(xtitle.c_str());
  hs0->GetYaxis()->SetTitle(ytitle.c_str());
  
  hs0->Draw("HIST");
  hs0_error->Draw("E2 same");
  data->Draw("ep same");
  
  leg1->Draw("same");
  lat2.DrawLatex(0.4, 0.86, label.c_str());
  if(scaleTTbar)
    lat2.DrawLatex(0.58, 0.78, "t#bar{t} scaled by 1.18");

  // Intelligent plot labeling
  if(verbose)    std::cout << " phaseSpace: " << phaseSpace << '\n';
  std::size_t found = phaseSpace.find_last_of("_");
  std::string tempPhase = phaseSpace.substr(0,found);
  found = tempPhase.find_last_of("_");
  if(verbose)    std::cout << " tempPhase: " << tempPhase << '\n';
  
  std::string var = tempPhase.substr(found+1);
  found = tempPhase.find_last_of("_");
  std::string phaseNoVar = tempPhase.substr(0,found);
  found = phaseNoVar.find_last_of("_");
  std::string checkLep = phaseNoVar.substr(found+1);

  if( checkLep == "el" || checkLep == "mu"){
    std::size_t lepClean = phaseNoVar.find_last_of("_");
    phaseNoVar = tempPhase.substr(0,lepClean);
  }

  found = phaseNoVar.find_last_of("_");
  //std::string lastCut = phaseNoVar.substr(found+1);
  std::string lastCuts = phaseNoVar.substr(0,found);
  found = lastCuts.find_last_of("_");
  std::string lastCut = lastCuts.substr(found+1);
  
  if(verbose)
    {
      std::cout << " var: " << var << '\n';
      std::cout << " phaseNoVar: " << phaseNoVar << '\n';
      std::cout << " checkLep: " << checkLep << '\n';
      std::cout << " lastCut: " << lastCut << '\n';
    }
  
  if(lastCut == cutflow.at(0))
    lat4.DrawLatex(0.62, 0.48, (optStrategy + ": " + lastCut).c_str());
  else
    lat4.DrawLatex(0.62, 0.48, (optStrategy + ": '' + " + lastCut).c_str());
  
  if(lepChannel == "el")
    lat5.DrawLatex(.2, .9, "Electron Channel");
  else if(lepChannel == "mu")
    lat5.DrawLatex(.2, .9, "Muon Channel");
  else if(lepChannel == "el_mu")
    lat5.DrawLatex(.2, .9, "Combined Channel");

  // ********* RATIO PLOT ************
  c0->cd();
  TPad *pad2=new TPad("pad2","pad2",0,0,1,0.2*1.55);
  pad2->SetTopMargin(0);
  pad2->SetBottomMargin(0.5);

  pad2->Draw();
  pad2->cd();
  TH1D* ratio=(TH1D*)data->Clone();

  //set first histogram for predicion sum (MC + ABCD estimation)
  TListIter next(hs0->GetHists());
  TObject *object;
  object=next();
  TH1D* h0=(TH1D*)object;
  TH1D* sum=(TH1D*)h0->Clone();
  //cout << "đđđđđđ ADDING " << h0->GetName() << " , sum->Integral() = " << sum->Integral() << endl;
  //loop over rest
  while( (object=next()) ) 
    {
      h0=(TH1D*)object;
      //cout << "h0->GetName() = " << h0->GetName() << " , h0->GetNbinsX() = " << h0->GetNbinsX() << " , h0->Integral() = " << h0->Integral() <<  ", isnan(h0->Integral()) = " << isnan(h0->Integral()) << endl;
      if( !TMath::IsNaN(h0->Integral()) ) 
	sum->Add(h0);
      //cout << "đđđđđđ ADDING " << h0->GetName() << " , h0->Integral() = " << sum->Integral() << endl;
      //cout << " ↓↓↓↓↓↓↓ h0->Print) ↓↓↓↓↓ " << endl;
      //h0->Print("all");
    }

  /*cout << " ↓↓↓↓↓↓↓ data->Print) ↓↓↓↓↓ " << endl;
  ratio->Print("all");
  cout << " ↓↓↓↓↓↓↓ sum->Print) ↓↓↓↓↓ " << endl;
  sum->Print("all");*/
  ratio->SetLineColor(kBlack);
  ratio->Divide(sum);

  ratio = returnRatioHistogram(data, sum); // test error calculation function

  ratio->GetYaxis()->SetRangeUser(.1,1.9);
  //ratio->GetYaxis()->SetRangeUser(.3,1.7);
  //if(addQCD) ratio->GetYaxis()->SetRangeUser(.55,1.45);
  ratio->GetYaxis()->SetNdivisions(5);
  ratio->GetYaxis()->SetLabelSize(0.095);
  ratio->SetYTitle("Data/Pred");
  ratio->GetYaxis()->SetTitleSize(0.13);
  ratio->GetYaxis()->SetTitleOffset(0.6);
  ratio->GetXaxis()->SetLabelSize(0.12);
  ratio->SetXTitle(xtitle.c_str());
  ratio->GetXaxis()->SetTitleSize(0.13);
  ratio->GetXaxis()->SetTitleOffset(1.2);
  ratio->Draw();
 
  TLine *line1=new TLine(ratio->GetXaxis()->GetXmin(),1,ratio->GetXaxis()->GetXmax(),1);
  line1->SetLineStyle(2);
  line1->SetLineWidth(3);
  line1->SetLineColor(kBlue+2);
  line1->Draw("same");

  c0->Update();
 
  if(label.find("A:")!=string::npos)      hname = hname ;
  else if(label.find("B:")!=string::npos) hname = hname ;
  else if(label.find("C:")!=string::npos) hname = hname ;
  else if(label.find("D:")!=string::npos) hname = hname ;

  if(printPlots) {
    if(drawUnblinded) {
      if(addSR) {
	c0->Print( (topDir + "unblinded/" + hname + ".eps").c_str());
	c0->Print( (topDir + "unblinded/" + hname + ".png").c_str());
      }
      else {
	c0->Print( (topDir + hname + ".eps").c_str());
	c0->Print( (topDir + hname + ".png").c_str());
      }
    }
    else{
      c0->Print( (topDir + hname + ".eps").c_str());
      c0->Print( (topDir + hname + ".png").c_str());
    }
    cout<<" ============= "<<label.c_str()<<" ============= "<<endl;
  }


  /*cout<<"ttbar integral:\t\t"<<ttbar->Integral()<<endl;
  if(addQCD) cout<<"QCD integral:\t\t"<<QCD->Integral()*100/tot<<endl;
  if(!onlyTTbar){
    cout<<"Wv221 integral:\t\t"<<Wv221->Integral()<<endl;
    cout<<"Z integral:\t\t"<<Z->Integral()<<endl;
    cout<<"Dibosons integral:\t"<<Dibosons->Integral()<<endl;
    cout<<"SingeTop integral:\t"<<SingleTop->Integral()<<endl;
  }
  cout<<"Total Bkg integral:\t"<<hTot->Integral()<<endl;
  cout<<"Data integral:\t\t"<<data->Integral()<<endl;
  cout<<"Data - Bkg:\t\t"<<data->Integral() - hTot->Integral()<<endl;
  //cout<<"ttbar NF:\t\t"<< (ttbar->Integral() + (data->Integral() - hTot->Integral()) ) / ttbar->Integral()<<endl;
  */
}

void makeControlPlots(TObjArray *array, TCanvas *c0, std::string region, std::string phaseSpace, std::string variable, bool doNorm, bool onlyAregion = false, bool drawUnblinded = false)
{
  std::string s_plotname = (phaseSpace + "_" + variable).c_str();

  if(verbose) cout<<"makeControlPlots: "<<s_plotname << endl;
  std::string labelA = !onlyAregion ? "A: |#sigma_{d_{0}}| < 2.0 and MET > 25 GeV" : " " ;
  std::string labelB = "B: |#sigma_{d_{0}}| < 2.0 and MET < 25 GeV";
  std::string labelC = "C: |#sigma_{d_{0}}| > 2.0 and MET > 25 GeV";
  std::string labelD = "D: |#sigma_{d_{0}}| > 2.0 and MET < 25 GeV";
  //std::string label  = "No |#sigma_{d_{0}}| or MET cuts";

  // make A and C region plots                                                    boolean for adding QCD
  makeStack(array, c0, region, (s_plotname + "_regionA_" + abcdSystematic).c_str(), labelA, doNorm, onlyAregion, drawUnblinded);
  if(onlyAregion)
    return;
  makeStack(array, c0, region, (s_plotname + "_regionC_" + abcdSystematic).c_str(), labelC, doNorm, false, drawUnblinded);

  // make B and D region histograms
  if(verbose) cout<<"makeControlPlots: "<<("ABCD_low" + s_plotname).c_str()<< endl;
  makeStack(array, c0, region, (s_plotname + "_regionB_" + abcdSystematic).c_str(), labelB, doNorm, false, drawUnblinded); 
  makeStack(array, c0, region, (s_plotname + "_regionD_" + abcdSystematic).c_str(), labelD, doNorm, false, drawUnblinded); 

}

void drawABCDplot(std::string fname, TCanvas*& c0, TH1D* nA_QCD, TH1D* nB_QCD, TH1D* nC_QCD, TH1D* nD_QCD, double nA_yield, bool scaleSideband = false)
{
  TH1D* nA = (TH1D*)nA_QCD->Clone();
  TH1D* nB = (TH1D*)nB_QCD->Clone();
  TH1D* nC = (TH1D*)nC_QCD->Clone();
  TH1D* nD = (TH1D*)nD_QCD->Clone();

  if (verbose) cout << "drawABCDplot, nA->GetName(): " << nA->GetName() << endl;

  c0->cd();
  nA->SetLineColor(kBlack);
  nB->SetLineColor(kRed);
  nC->SetLineColor(kBlue);
  nD->SetLineColor(kGreen);
  
  nA->SetLineColor(kBlack);
  nB->SetLineColor(kRed);
  nC->SetLineColor(kBlue);
  nD->SetLineColor(kGreen);
  nA->SetFillColor(0);
  nB->SetFillColor(0);
  nC->SetFillColor(0);
  nD->SetFillColor(0);
  nA->SetLineWidth(3);
  nB->SetLineWidth(3);
  nC->SetLineWidth(3);
  nD->SetLineWidth(3);

  /*for (int i = 4; i <= 6; i++){
    nA->SetBinContent(i, 0);
    nB->SetBinContent(i, 0);
    nC->SetBinContent(i, 0);
    nD->SetBinContent(i, 0);
    nA->SetBinError(i, 0);
    nB->SetBinError(i, 0);
    nC->SetBinError(i, 0);
    nD->SetBinError(i, 0);
  }
  */
  
  double sidebandB = nB->Integral() - nB->Integral(4,6);
  double sidebandC = nC->Integral() - nC->Integral(4,6);
  double sidebandD = nD->Integral() - nD->Integral(4,6);
  
  double nA_qcd_abcd = (sidebandB * sidebandC) / sidebandD;
  double nA_qcd_abcd_sideband = 0;

  /*
  if(scaleSideband){
    nA_qcd_abcd_sideband = nA_qcd_abcd;
    TH1D* qcdTemplate = (TH1D*)nC->Clone();
    double nA_qcd_abcd_unblinded = nB->Integral() * nC->Integral() / nD->Integral();
    qcdTemplate->Scale( nA_qcd_abcd_unblinded / nC->Integral() ); // scale region C to B*C/D (traditional ABCD)

    double nA_qcd_abcd_unscaled = qcdTemplate->Integral() - qcdTemplate->Integral(4,6);
    qcdTemplate->Scale( nA_qcd_abcd_sideband  / nA_qcd_abcd_unscaled ); // scale region so that unblinded sideband yield matches blinded sideband
    
    nA_qcd_abcd = qcdTemplate->Integral() - qcdTemplate->Integral(4,6);
  }*/
  
  
  TLegend *leg2 = new TLegend();  
  leg2=new TLegend(.555,.6,.705,.86);
  leg2->SetTextSize(0.04);
  leg2->AddEntry(nA,"N_{A}^{Data} - N_{A}^{MC Bkgs}", "l");
  leg2->AddEntry(nB,"N_{B}^{Data} - N_{B}^{MC Bkgs}", "l");
  leg2->AddEntry(nC,"N_{C}^{Data} - N_{C}^{MC Bkgs}", "l");
  leg2->AddEntry(nD,"N_{D}^{Data} - N_{D}^{MC Bkgs}", "l");
    
  nB->SetYTitle("Normalized Entries / 25 GeV");
  nB->GetXaxis()->SetLabelSize(0.04);
  nB->GetYaxis()->SetLabelSize(0.04);

  double max = 1.2*nB->GetMaximum();
  if( nB->GetMaximum() / nB->Integral() < nA->GetMaximum() / nA->Integral())
    max = (1 + 3*(nA->GetMaximum() / nA->Integral()) ) * nB->GetMaximum();

  nA->SetMarkerStyle( 0 );
  nB->SetMarkerStyle( 0 );
  nC->SetMarkerStyle( 0 );
  nD->SetMarkerStyle( 0 );

  nB->SetMaximum( max );
  nB->SetMinimum( 0 );
  nB->DrawNormalized("HIST E");
  nD->DrawNormalized("HIST E same");
  nC->DrawNormalized("HIST E same");
  nA->DrawNormalized("HIST E same");
  leg2->Draw("same");
  
  TLatex lat3;
  lat3.SetTextAlign(9);
  lat3.SetTextFont(62);
  lat3.SetTextSize(0.04);
  lat3.SetNDC();
  
  /*string s_rMC;          // string which will contain the result
    ostringstream os_rMC;   // stream used for the conversion
    os_rMC << std::fixed;
    os_rMC << setprecision(2) << rMC ;      
    s_rMC = os_rMC.str(); // set 'Result' to the contents of the stream
    lat3.DrawLatex(0.65, 0.55, ("R_{MC} = " + s_rMC).c_str());
  */

  string s_nAqcd;          // string which will contain the result
  ostringstream os_nAqcd;   // stream used for the conversion
  os_nAqcd << std::fixed;
  //os_nAqcd << setprecision(1) << nA_qcd_abcd; // old B*C/D method
  os_nAqcd << setprecision(1) << nA_yield;      
  s_nAqcd = os_nAqcd.str(); // set 'Result' to the contents of the stream
  if(fname.find("opt700")!=string::npos)
    lat3.DrawLatex(0.578, 0.48, ("N_{A, Sideband}^{DD, Est.} = " + s_nAqcd).c_str());
  //else if(fname.find("opt2000")!=string::npos)
  else if(fname.find("reOpt2000")!=string::npos)
    lat3.DrawLatex(0.25, 0.86, ("N_{A, Sideband}^{DD, Est.} = " + s_nAqcd).c_str());
  
  /*string s_ttbarNF;          // string which will contain the result
    ostringstream os_ttbarNF;   // stream used for the conversion
    os_ttbarNF << std::fixed;
    os_ttbarNF << setprecision(2) << ttbarNF;      
    s_ttbarNF = os_ttbarNF.str(); // set 'Result' to the contents of the stream
    lat3.DrawLatex(0.632, 0.42, ("t#bar{t} NF = " + s_ttbarNF).c_str());
  */

  TLatex lat2;
  lat2.SetTextAlign(9);
  lat2.SetTextFont(62);
  lat2.SetTextSize(0.03);
  lat2.SetNDC();
  if(scaleTTbar)
    lat2.DrawLatex(0.58, 0.89, "t#bar{t} scaled by 1.18");
  
  TLatex lat4;
  lat4.SetTextAlign(9);
  lat4.SetTextFont(62);
  lat4.SetTextSize(0.033);
  lat4.SetNDC();

  std::string hname = nA->GetName();
  if(fname.find("opt700")!=string::npos){
    if(fname.find("drww")!=string::npos)
      lat4.DrawLatex(0.42, 0.36, "After m_{WW}<130, #DeltaR_{bb}<1.1, #DeltaR_{WW}<0.9");
    else if(fname.find("drbb")!=string::npos)
      lat4.DrawLatex(0.42, 0.36, "After m_{WW}<130, #DeltaR_{bb}<1.1");
    else if(fname.find("mww")!=string::npos)
      lat4.DrawLatex(0.42, 0.36, "After m_{WW}<130");
  }
  //else if (fname.find("opt2000")!=string::npos) { 
  else if (fname.find("reOpt2000")!=string::npos) { 
    if(fname.find("drww")!=string::npos)
      lat4.DrawLatex(0.25, 0.55, "After p_{T}^{WW}>360, #DeltaR_{WW}<2.0");
    else if(fname.find("wwpt")!=string::npos)
      lat4.DrawLatex(0.25, 0.55, "After p_{T}^{WW}>360");
  }

  if(printPlots) {
    c0->Print( (topDir + fname + "_abcd_QCD_shapes.eps").c_str());
    c0->Print( (topDir + fname + "_abcd_QCD_shapes.png").c_str());
  }
}
