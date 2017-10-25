// Date:    Feb 8, 2017
// Author:  Benjamin Tannenwald
// Purpose: Header file containing general input reading functions and histogram arrays used in code for ABCD estimation of QCD

TH1D* returnArrayHisto(TObjArray* array, std::string hname)
{
  TH1D* temp = new TH1D();
  if( array->FindObject( hname.c_str()) )
    temp = (TH1D*)array->FindObject( hname.c_str() );
  else {
    if(verbose) cout<<"returnArrayHisto, NOT FOUND: "<<hname<<endl;
  }

  TH1D* hist = (TH1D*)temp->Clone();

  return hist;
}

void create1DAddress(TObjArray* array, TH1D*& histo, std::string sample, std::string cutLevel, std::string var, std::string xname, std::string yname, std::string syst)
{
  std::string merge=(sample + "_" + cutLevel + "_" + var).c_str() ;
  if(lepType=="TightMM"){
    merge = (merge + "_" + abcdSystematic).c_str() ;
    var = (var + "_" + abcdSystematic).c_str() ;
  }
  //TH1D *mBBcr = (TH1D*)array->FindObject( (sample + "_mBBcr_" + cutLevel + "_" + var).c_str() );
  //TH1D *sr    = (TH1D*)array->FindObject( (sample + "_SR_" + cutLevel + "_" + var).c_str() );
  //if(verbose) cout << "create1Daddress: cutLevel = " << cutLevel << endl;
  if(syst!="" && syst.find("QCD_Norm")==string::npos){
    var = (var + "_" + syst).c_str();
    merge = (merge + "_" + syst).c_str() ;
    //if( !(TH1D*)array->FindObject( (sample + "_mBBcr_" + cutLevel + "_" + var).c_str() ) ||
    //	!(TH1D*)array->FindObject( (sample + "_SR_" + cutLevel + "_" + var).c_str() ) )
    //  return;
  }
  if(verbose) cout << "create1Daddress, searching for " << (sample + "_mBBcr_" + cutLevel + "_" + var).c_str() << endl;
  TH1D *mBBcr =  returnArrayHisto(array, (sample + "_mBBcr_" + cutLevel + "_" + var).c_str() );
  TH1D *sr =  returnArrayHisto(array, (sample + "_SR_" + cutLevel + "_" + var).c_str() );

  //if(verbose && syst!="") cout << " create1Daddress, " << sr->GetName() << " has " << sr->GetEntries() << " entries." << endl;
  //if(syst!="" && sr->GetEntries()== 0)
  //  return;
  
  //cout << "Creating sum of " << mBBcr->GetName() << " and " << sr->GetName() << endl;
  histo=(TH1D*)mBBcr->Clone();
  histo->Add(sr);

  // normal label stuff
  histo->SetName(merge.c_str());
  histo->SetTitle(merge.c_str());
  histo->SetXTitle(xname.c_str());
  histo->SetYTitle(yname.c_str());
 
  Color_t c_col;
  
  if (merge.find("Wv221_")!=string::npos)           c_col = kGreen+1;
  else if (merge.find("SingleTop_")!=string::npos) c_col = kYellow+2;
  else if (merge.find("Dibosonsv221_")!=string::npos)  c_col = kBlue;
  else if (merge.find("Dibosons_")!=string::npos)  c_col = kBlue;
  else if (merge.find("Zv221_")!=string::npos)         c_col = kRed;
  else if (merge.find("Z_")!=string::npos)         c_col = kRed;
  //else if (merge.find("QCD_")!=string::npos)       c_col = kGray+1;
  else if (merge.find("ttbar_")!=string::npos)     c_col = kOrange;
  
  if(sample!="data"){
    histo->SetLineColor(c_col);
    histo->SetFillColor(c_col);
  }
  histo->Draw();
  
  array->AddLast(histo); // add histo to TObjArray
  
  //cout << "set1Daddress, Added " << histo->GetName() << endl;

  return;
}

void set1DAddress(TObjArray* array, TH1D*& histo, TFile* file, std::string sample, std::string cutLevel, std::string var, std::string xname, std::string yname, std::string syst)
{
  std::string merge=(sample + "_" + cutLevel + "_" + var).c_str() ;

  TH1D* rebin = new TH1D();

  // revert to empty hist if IS data and SR and region A
  bool skipSR = false;     
  if( !addSR && (merge.find("SR")!=string::npos && merge.find("data")!=string::npos &&merge.find("regionA")!=string::npos) )
    skipSR = true;

  if(verbose) cout << "set1Daddress, histogram: " << merge << " , skipSR: " << skipSR << endl;

  // *** 1. Check if histogram in file, and get from file if present
  if(syst==""){
    if( file->GetListOfKeys()->Contains(merge.c_str()) && !skipSR) { 
      histo=(TH1D*)file->Get(merge.c_str()); //get histogram from input file
  
      if(lepType=="TightMM")
	merge = (merge + "_" + abcdSystematic).c_str();
      
      histo->SetName(merge.c_str());
      histo->SetXTitle(xname.c_str());
      histo->SetYTitle(yname.c_str());
      
      // put overflow bin at end
      int maxBin = histo->GetNbinsX();
      histo->SetBinContent( maxBin, histo->GetBinContent( maxBin ) + histo->GetBinContent( maxBin+1 ) );
      histo->SetBinError  ( maxBin, sqrt( histo->GetBinError(maxBin)*histo->GetBinError(maxBin) + histo->GetBinError(maxBin+1)*histo->GetBinError(maxBin+1) ) );
      histo->SetBinContent( maxBin + 1, 0 );
      histo->SetBinError( maxBin + 1, 0 );
      
    }
    // *** 2. Produce dummy histogram if not found in file
    else {  
      
      if(lepType=="TightMM")
	merge = (merge + "_" + abcdSystematic).c_str();
      
      //if( !doStabilityTest && !(merge.find("_mbb")!=string::npos && merge.find("mBBcr")!=string::npos))  { cout<<"set1DAddress, Histogram "<<merge<<" NOT FOUND."<<endl; }
      if( verbose )  { cout<<"set1DAddress, Histogram "<<merge<<" NOT FOUND."<<endl; }
      if( varABCD.find("bbMass")!=string::npos)       histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 1000);
      if( varABCD.find("bbPt")!=string::npos)         histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 1000);
      if( varABCD.find("drbb")!=string::npos)         histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 5);
      if( varABCD.find("drww")!=string::npos)         histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 10);
      if( varABCD.find("WWMass")!=string::npos)       histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 1000);
      if( varABCD.find("WWPt")!=string::npos)         histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 1000);
      if( varABCD.find("hhMass")!=string::npos)       histo = new TH1D(merge.c_str(), merge.c_str(), 500, 0, 5000);
      if( varABCD.find("MET")!=string::npos)          histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 200);
      if( varABCD.find("wlepmtben")!=string::npos)    histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 200);
      if( varABCD.find("d0sigBL")!=string::npos)      histo = new TH1D(merge.c_str(), merge.c_str(), 100, -10, 10);
    }
  }
  else { // *** 3. Systematic histograms
    if(syst.find("QCD_Norm")==string::npos) {
      if(verbose) cout << "set1Daddress, " << merge << " goes to " ;
      merge = (merge + "_" + syst).c_str();    
      if(verbose) cout << merge << endl;
    }

    if( (gDirectory->GetListOfKeys()->Contains(merge.c_str()) && !skipSR) && merge.find("data")==string::npos) { 

      if(verbose && syst !="")    cout << merge << endl;
      histo=(TH1D*)gDirectory->Get(merge.c_str()); //get histogram from input file
      if(verbose && syst !="")    cout << "got syst histo" << endl;
      histo->SetName(merge.c_str());
      if(verbose && syst !="")    cout << "A";
      histo->SetXTitle(xname.c_str());
      if(verbose && syst !="")    cout << "B";
      histo->SetYTitle(yname.c_str());
      if(verbose && syst !="")    cout << "C" << endl;

      if(verbose && syst !="")    cout << histo->GetName() << "has " << histo->GetNbinsX() << " bins." << endl;

      // put overflow bin at end
      int maxBin = histo->GetNbinsX();
      histo->SetBinContent( maxBin, histo->GetBinContent( maxBin ) + histo->GetBinContent( maxBin+1 ) );
      histo->SetBinError  ( maxBin, sqrt( histo->GetBinError(maxBin)*histo->GetBinError(maxBin) + histo->GetBinError(maxBin+1)*histo->GetBinError(maxBin+1) ) );
      histo->SetBinContent( maxBin + 1, 0 );
      histo->SetBinError( maxBin + 1, 0 );

    }
    else{
      if( varABCD.find("bbMass")!=string::npos)       histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 1000);
      if( varABCD.find("bbPt")!=string::npos)         histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 1000);
      if( varABCD.find("drbb")!=string::npos)         histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 5);
      if( varABCD.find("drww")!=string::npos)         histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 10);
      if( varABCD.find("WWMass")!=string::npos)       histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 1000);
      if( varABCD.find("WWPt")!=string::npos)         histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 1000);
      if( varABCD.find("hhMass")!=string::npos)       histo = new TH1D(merge.c_str(), merge.c_str(), 500, 0, 5000);
      if( varABCD.find("MET")!=string::npos)          histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 200);
      if( varABCD.find("wlepmtben")!=string::npos)    histo = new TH1D(merge.c_str(), merge.c_str(), 100, 0, 200);
      if( varABCD.find("d0sigBL")!=string::npos)      histo = new TH1D(merge.c_str(), merge.c_str(), 100, -10, 10);
      if(verbose)    cout << merge << " NOT FOUND." << endl;
      //return;
    }
  }
  // *** 4. Rebinning
  // *** A. bbMass
  if( varABCD.find("bbMass")!=string::npos) {
    if(optStrategy == "optNoRes" || optStrategy == "reOptNonRes"){
      //Double_t xbins[10] = {0, 25, 50, 75, 150, 225, 300, 375, 450, 500}; // array of low-edges (last number is upper edge of last bin)
      //rebin = (TH1D*)histo->Rebin(9, merge.c_str(), xbins);  //creates a new variable bin size histogram rebin
      Double_t xbins[13] = {0, 20, 60, 100, 140, 180, 220, 260, 320, 400, 600, 800, 1000}; // array of low-edges (last number is upper edge of last bin)
      rebin = (TH1D*)histo->Rebin(12, merge.c_str(), xbins);  //creates a new variable bin size histogram rebin
      /*if(lepChannel != "el_mu"){
	//Double_t xbins2[9] = {0, 20, 100, 140, 220, 300, 400, 700, 1000}; // array of low-edges (last number is upper edge of last bin)
	//rebin = (TH1D*)histo->Rebin(8, merge.c_str(), xbins2);  //creates a new variable bin size histogram rebin
	Double_t xbins2[10] = {0, 20, 60, 100, 140, 220, 300, 400, 700, 1000}; // array of low-edges (last number is upper edge of last bin)
	rebin = (TH1D*)histo->Rebin(9, merge.c_str(), xbins2);  //creates a new variable bin size histogram rebin      }
      }*/
    }
    else if(optStrategy == "opt500"){
      Double_t xbins2[12] = {0, 20, 60, 100, 140, 180, 220, 260, 320, 400, 600, 1000}; // array of low-edges (last number is upper edge of last bin)
      rebin = (TH1D*)histo->Rebin(11, merge.c_str(), xbins2);  //creates a new variable bin size histogram rebin
    }
    else if(optStrategy == "opt700"){
      Double_t xbins2[11] = {0, 20, 60, 100, 140, 180, 220, 260, 320, 400, 1000}; // array of low-edges (last number is upper edge of last bin)
      rebin = (TH1D*)histo->Rebin(10, merge.c_str(), xbins2);  //creates a new variable bin size histogram rebin
    }
    else if(optStrategy == "opt700ichep" || optStrategy == "reOpt500" || optStrategy == "reOpt700"){
      Double_t xbins[13] = {0, 20, 60, 100, 140, 180, 220, 260, 320, 400, 600, 800, 1000}; // array of low-edges (last number is upper edge of last bin)
      rebin = (TH1D*)histo->Rebin(12, merge.c_str(), xbins);  //creates a new variable bin size histogram rebin
      if(lepChannel != "el_mu"){
	Double_t xbins2[12] = {0, 20, 60, 100, 140, 180, 220, 260, 320, 400, 600, 1000}; // array of low-edges (last number is upper edge of last bin)
	rebin = (TH1D*)histo->Rebin(11, merge.c_str(), xbins2);  //creates a new variable bin size histogram rebin
      }
    }
    else if(optStrategy == "opt1400" || optStrategy == "opt2000" ){
      Double_t xbins2[11]  = {0, 20, 60, 100, 140, 200, 300, 450, 600, 800, 1000}; // array of low-edges (last number is upper edge of last bin)
      rebin = (TH1D*)histo->Rebin(10, merge.c_str(), xbins2);  //creates a new variable bin size histogram rebin
    }
    else if(optStrategy == "opt2000ichep" || optStrategy == "reOpt2000" ){
      //Double_t xbins[13]  = {0, 25, 50, 75, 100, 125, 150, 200, 250, 300, 350, 500, 1000}; // array of low-edges (last number is upper edge of last bin)
      //rebin = (TH1D*)histo->Rebin(12, merge.c_str(), xbins);  //creates a new variable bin size histogram rebin
      Double_t xbins2[11]  = {0, 20, 60, 100, 140, 200, 300, 450, 600, 800, 1000}; // array of low-edges (last number is upper edge of last bin)
      rebin = (TH1D*)histo->Rebin(10, merge.c_str(), xbins2);  //creates a new variable bin size histogram rebin
    }
  }
  // *** B. MET
  if( varABCD.find("MET")!=string::npos) {
    //Double_t xbins[8]  = {0, 20, 40, 60, 80, 100, 150, 200};
    //rebin = (TH1D*)histo->Rebin(7, merge.c_str(), xbins);  //creates a new variable bin size histogram rebin
    rebin = (TH1D*)histo->Rebin(4);
  }
  if( varABCD.find("wlepmtben")!=string::npos) {
    Double_t xbins[8]  = {0, 20, 40, 60, 80, 100, 150, 200};
    rebin = (TH1D*)histo->Rebin(7, merge.c_str(), xbins);  //creates a new variable bin size histogram rebin
  }
  if( varABCD.find("d0sigBL")!=string::npos) {
    rebin = (TH1D*)histo->Rebin(2);
    //rebin = (TH1D*)histo->Clone();
  }
  if( varABCD.find("drbb")!=string::npos ) {
    rebin = (TH1D*)histo->Rebin(10);
  }  
  if( varABCD.find("drww")!=string::npos ) {
    Double_t xbins2[17]  = {0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0}; // array of low-edges (last number is upper edge of last bin)
    rebin = (TH1D*)histo->Rebin(16, merge.c_str(), xbins2);  //creates a new variable bin size histo
    //rebin = (TH1D*)histo->Rebin(5);
  }  
  if( varABCD.find("bbPt")!=string::npos ) {
    Double_t xbins2[11]  = {0, 50, 100, 150, 200, 250, 300, 400, 600, 800, 1000}; // array of low-edges (last number is upper edge of last bin)
    rebin = (TH1D*)histo->Rebin(10, merge.c_str(), xbins2);  //creates a new variable bin size histo
    //rebin = (TH1D*)histo->Rebin(10);
  }  
  if( varABCD.find("WWPt")!=string::npos ) {
    Double_t xbins2[11]  = {0, 50, 100, 150, 200, 250, 300, 400, 600, 800, 1000}; // array of low-edges (last number is upper edge of last bin)
    rebin = (TH1D*)histo->Rebin(10, merge.c_str(), xbins2);  //creates a new variable bin size histo
    //rebin = (TH1D*)histo->Rebin(10);
  }  
  if( varABCD.find("hhMass")!=string::npos ) {
    rebin = (TH1D*)histo->Rebin(50);
  }  
  if( varABCD.find("WWMass")!=string::npos ) {
    rebin = (TH1D*)histo->Rebin(10);
  }
    
  // *** 5. Do general things like set color
  Color_t c_col;
  if (merge.find("Wv221_")!=string::npos)           c_col = kGreen+1;
  else if (merge.find("SingleTop_")!=string::npos) c_col = kYellow+2;
  else if (merge.find("Dibosonsv221_")!=string::npos)  c_col = kBlue;
  else if (merge.find("Dibosons_")!=string::npos)  c_col = kBlue;
  else if (merge.find("Zv221_")!=string::npos)         c_col = kRed;
  else if (merge.find("Z_")!=string::npos)         c_col = kRed;
  //else if (merge.find("QCD_")!=string::npos)       c_col = kGray+1;
  else if (merge.find("ttbar_")!=string::npos)     c_col = kOrange;
  
  if(sample!="data"){
    rebin->SetLineColor(c_col);
    rebin->SetFillColor(c_col);
  }
  //histo->Sumw2();
  rebin->Draw();
  
  // ** A. Scale ttbar if found in file and scaleTTbar set to true
  if(scaleTTbar && merge.find("ttbar_")!=string::npos){
    if(optStrategy == "optNoRes" || optStrategy == "opt700ichep")
      rebin->Scale(0.94);
    else if(optStrategy == "opt2000ichep")
      rebin->Scale(1.03);
  }

  // add rebin to TObjArray
  array->AddLast(rebin); 
  
  return;
}

void readHists(TFile* file, TObjArray* array, std::string cutLevel, std::string var, std::string ABCDregion, std::string syst)
{
  TH1D *h_arr = new TH1D();
  if(verbose) cout<<"cutlevel hoopla:  "<<cutLevel<<endl; 
  std::string xtitle = "";
  std::string ytitle = "";

  if(var.find("bbMass")!=string::npos){
    xtitle = "m_{bb} [GeV]";
    ytitle = "Entries / 25 GeV";
  }
  else if(var.find("MET")!=string::npos){
    xtitle = "Missing E_{T} [GeV]";
    ytitle = "Entries / 25 GeV";
  }
  else if(var.find("wlepmtben")!=string::npos){
    xtitle = "Leptonic m_{T}^{W} [GeV]";
    ytitle = "Entries / 25 GeV";
  }
  else if(var.find("drbb")!=string::npos){
    xtitle = "#Delta R(bb)";
    ytitle = "Entries / Bin";
  }
  else if(var.find("drww")!=string::npos){
    xtitle = "#Delta R(WW)";
    ytitle = "Entries / Bin";
  }
  else if(var.find("WWPt")!=string::npos){
    xtitle = "p_{T}^{WW} [GeV]";
    ytitle = "Entries / Bin";
  }
  else if(var.find("WWMass")!=string::npos){
    xtitle = "Invariant WW* Mass [GeV]";
    ytitle = "Entries / Bin";
  }
  else if(var.find("hhMass")!=string::npos){
    xtitle = "Invariant hh Mass [GeV]";
    ytitle = "Entries / Bin";
  }
  else if(var.find("bbPt")!=string::npos){
    xtitle = "p_{T}^{bb} [GeV]";
    ytitle = "Entries / Bin";
  }

  if(ABCDregion!="")
    var = (var + "_" + ABCDregion).c_str();
  
  std::string region = "mBBcr_";
  set1DAddress(array, h_arr, file, "ttbar", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);

  set1DAddress(array, h_arr, file, "data", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
  if(xhhmass.find("a") !=string::npos || xhhmass.find("b") !=string::npos || xhhmass.find("c") !=string::npos || xhhmass.find("d") !=string::npos) 
    set1DAddress(array, h_arr, file, ("X" + xhhmass.substr(0,xhhmass.size()-1)).c_str(), ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
  else set1DAddress(array, h_arr, file, ("X" + xhhmass).c_str(), ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
  if(qcdMM) set1DAddress(array, h_arr, file, "QCD", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
  
  if(!onlyTTbar) {
    set1DAddress(array, h_arr, file, "Wv221", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
    set1DAddress(array, h_arr, file, "SingleTop", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
   
    if(lepType=="IsoOR"){
      set1DAddress(array, h_arr, file, "Dibosonsv221", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
      set1DAddress(array, h_arr, file, "Zv221", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
    }
    else if(lepType=="TightMM"){
      set1DAddress(array, h_arr, file, "Dibosons", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
      set1DAddress(array, h_arr, file, "Z", ("mBBcr_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
    }
  }
  // SR plots. Can always read in MC, but fill empty histograms for data if addSR not true
  set1DAddress(array, h_arr, file, "data", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
  set1DAddress(array, h_arr, file, "ttbar", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
  if(xhhmass.find("a") !=string::npos || xhhmass.find("b") !=string::npos || xhhmass.find("c") !=string::npos || xhhmass.find("d") !=string::npos ) 
    set1DAddress(array, h_arr, file, ("X" + xhhmass.substr(0,xhhmass.size()-1)).c_str(), ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
  else set1DAddress(array, h_arr, file, ("X" + xhhmass).c_str(), ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);

  if(qcdMM){
    set1DAddress(array, h_arr, file, "QCD", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
  }
  
  if(!onlyTTbar) {
    set1DAddress(array, h_arr, file, "Wv221", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
    set1DAddress(array, h_arr, file, "SingleTop", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);

    if(lepType=="IsoOR"){
      set1DAddress(array, h_arr, file, "Dibosonsv221", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
      set1DAddress(array, h_arr, file, "Zv221", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
    }
    else if(lepType=="TightMM"){
      set1DAddress(array, h_arr, file, "Dibosons", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
      set1DAddress(array, h_arr, file, "Z", ("SR_" + cutLevel).c_str(), var, xtitle, ytitle, syst);
    }
  }

  if(verbose) cout <<"readHists, starting to merge SR and mBBcr" << endl;
  // creating mBBcr+SR histos
  create1DAddress(array, h_arr, "ttbar", cutLevel, var, xtitle, ytitle, syst);
  create1DAddress(array, h_arr, "data", cutLevel, var, xtitle, ytitle, syst);
  if(xhhmass.find("a") !=string::npos || xhhmass.find("b") !=string::npos || xhhmass.find("c") !=string::npos || xhhmass.find("d") !=string::npos) 
    create1DAddress(array, h_arr, ("X" + xhhmass.substr(0,xhhmass.size()-1)).c_str(), cutLevel, var, xtitle, ytitle, syst);
  else create1DAddress(array, h_arr, ("X" + xhhmass).c_str(), cutLevel, var, xtitle, ytitle, syst);
  if(qcdMM) create1DAddress(array, h_arr, "QCD", cutLevel, var, xtitle, ytitle, syst);
  
  if(cutLevel.find("opt2000")!=string::npos) create1DAddress(array, h_arr, "Xhh2000", cutLevel, var, xtitle, ytitle, syst);
  
  if(!onlyTTbar) {
    create1DAddress(array, h_arr, "Wv221", cutLevel, var, xtitle, ytitle, syst);
    create1DAddress(array, h_arr, "SingleTop", cutLevel, var, xtitle, ytitle, syst);
   
    if(lepType=="IsoOR"){
      create1DAddress(array, h_arr, "Dibosonsv221", cutLevel, var, xtitle, ytitle, syst);
      create1DAddress(array, h_arr, "Zv221", cutLevel, var, xtitle, ytitle, syst);
    }
    else if(lepType=="TightMM"){
      create1DAddress(array, h_arr, "Dibosons", cutLevel, var, xtitle, ytitle, syst);
      create1DAddress(array, h_arr, "Z", cutLevel, var, xtitle, ytitle, syst);
    }	
  }

  return;
}

void loopHists(TFile* f0, TObjArray *array, std::string syst = "")
{
  cout<<"####### Reading " << optStrategy << " Histograms ####### "<<endl;
  if(syst!="")
    cout<<"####### Reading " << syst << " systematic ####### "<<endl;
    
  std::string s_cutLevel = "";
  
  
  for (int c = 0; c < cutflow.size(); c++) {
    // ** A. Set cut string
    if(s_cutLevel == "") s_cutLevel = cutflow.at(c);  
    else                 s_cutLevel = (s_cutLevel + "_" + cutflow.at(c)).c_str();  
    // ** B. read histograms at each cut level
    if(verbose) cout<<"readHists (cutLevel = " << s_cutLevel<< ")"<<endl;
    //readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "");
    if(lepType == "IsoOR") {
      cout << " --> normal, s_cutLevel: " << s_cutLevel << endl;
      readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "regionA_" + abcdSystematic, syst);
      if(syst==""){
	readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "regionB_" + abcdSystematic, syst);
	readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "regionC_" + abcdSystematic, syst);
	readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "regionD_" + abcdSystematic, syst);
      }
      if(verbose) cout<<" John "<<  s_cutLevel  << " s_cutLevel.find(mbb): "<< s_cutLevel.find("mbb") <<endl;
      // * i. For _mbb after each cut
      if( c < (cutflow.size() -2) ){
      //if( s_cutLevel.find("mbb")== string::npos){
	cout << " --> adding mbb, s_cutLevel: " << s_cutLevel << "_mbb" << endl;
	readHists(f0, array, optStrategy + "_" + s_cutLevel + "_mbb", varABCD, "regionA_" + abcdSystematic, syst);

	if(syst==""){
	  readHists(f0, array, optStrategy + "_" + s_cutLevel + "_mbb", varABCD, "regionB_" + abcdSystematic, syst);
	  readHists(f0, array, optStrategy + "_" + s_cutLevel + "_mbb", varABCD, "regionC_" + abcdSystematic, syst);
	  readHists(f0, array, optStrategy + "_" + s_cutLevel + "_mbb", varABCD, "regionD_" + abcdSystematic, syst);
	}
      }
      // * ii. For individual cuts
      if(c>0 && c!= (cutflow.size() -1) ){ // first cut already 'individual'
	cout << " --> individual, s_cutLevel: " << cutflow.at(c) << endl;
	readHists(f0, array, optStrategy + "_" + cutflow.at(c), varABCD, "regionA_" + abcdSystematic, syst);
	if(syst==""){
	  readHists(f0, array, optStrategy + "_" + cutflow.at(c), varABCD, "regionB_" + abcdSystematic, syst);
	  readHists(f0, array, optStrategy + "_" + cutflow.at(c), varABCD, "regionC_" + abcdSystematic, syst);
	  readHists(f0, array, optStrategy + "_" + cutflow.at(c), varABCD, "regionD_" + abcdSystematic, syst);
	}

	cout << " --> individual + mbb, s_cutLevel: " << cutflow.at(c)  << "_mbb" << endl;
	readHists(f0, array, optStrategy + "_" + cutflow.at(c) + "_mbb", varABCD, "regionA_" + abcdSystematic, syst);
	if(syst==""){
	  readHists(f0, array, optStrategy + "_" + cutflow.at(c) + "_mbb", varABCD, "regionB_" + abcdSystematic, syst);
	  readHists(f0, array, optStrategy + "_" + cutflow.at(c) + "_mbb", varABCD, "regionC_" + abcdSystematic, syst);
	  readHists(f0, array, optStrategy + "_" + cutflow.at(c) + "_mbb", varABCD, "regionD_" + abcdSystematic, syst);
	}
      }

    }
    else if(lepType == "TightMM"){
      readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "regionA", syst);
      if(syst==""){
	readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "regionB", syst);
	readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "regionC", syst);
	readHists(f0, array, optStrategy + "_" + s_cutLevel, varABCD, "regionD", syst);
      }
    }
  }

  // ** iii. read reOpt500 string for second Rnp calculation
  if( doSecondRnp && (optStrategy == "reOptNonRes" || optStrategy == "reOpt700")){
    cout << " --> reOpt500 histograms for 2nd Rnp " << endl;
    /*readHists(f0, array, "reOpt500_mww_wwpt150", varABCD, "regionA_" + abcdSystematic, syst);
    if(syst==""){
      readHists(f0, array, "reOpt500_mww_wwpt150", varABCD, "regionB_" + abcdSystematic, syst);
      readHists(f0, array, "reOpt500_mww_wwpt150", varABCD, "regionC_" + abcdSystematic, syst);
      readHists(f0, array, "reOpt500_mww_wwpt150", varABCD, "regionD_" + abcdSystematic, syst);
      }*/

    // mww_bbpt210_wwpt150
    readHists(f0, array, "reOpt500_mww_bbpt210_wwpt150", varABCD, "regionA_" + abcdSystematic, syst);
    if(syst==""){
      readHists(f0, array, "reOpt500_mww_bbpt210_wwpt150", varABCD, "regionB_" + abcdSystematic, syst);
      readHists(f0, array, "reOpt500_mww_bbpt210_wwpt150", varABCD, "regionC_" + abcdSystematic, syst);
      readHists(f0, array, "reOpt500_mww_bbpt210_wwpt150", varABCD, "regionD_" + abcdSystematic, syst);
    }
    
  }

}

void addQCDnormSys(TFile* infile, TFile* outfile, TObjArray* a_temp, std::string upDown)
{
  cout<<"####### Reading " << upDown << " systematic ####### "<<endl;

  // set systematic variation for 
  double systVar = 0.36;

  // add QCD norm systematic
  TObjArray* array = new TObjArray();
  TH1D* htemp = new TH1D();
  TObjArrayIter next(a_temp);
  TObject* object;
  std::string systName = upDown;
  std::string hname = "";
  while( (object = next()) ){
    htemp = (TH1D*)object->Clone();
    hname = htemp->GetName();
    hname = (hname + "_" + systName).c_str();
    htemp->SetName(hname.c_str());
    htemp->SetTitle(hname.c_str());
    if(hname.find("regionA")!=string::npos && hname.find("data")==string::npos && hname.find("nonprompt")==string::npos)
      array->AddLast(htemp);
  }

  outfile->cd("");
  outfile->mkdir(systName.c_str()); 
  outfile->cd(systName.c_str()); 
  TObjArrayIter next2(array);
  TObject* object2;
  hname = "";
  while( (object2 = next2()) ){
    htemp = (TH1D*)object2;
    hname = htemp->GetName();
    if(htemp->GetNbinsX() > 0){
      bool isQCD = (hname.find("QCD_mBBcr")!=string::npos || hname.find("QCD_SR")!=string::npos || hname.find("QCD_opt")!=string::npos) ? true : false;
      if(isQCD && upDown.find("1up")!=string::npos)     htemp->Scale(1. + systVar);
      if(isQCD && upDown.find("1down")!=string::npos)   htemp->Scale(1. - systVar);
      htemp->Write();
    }
  }
  outfile->cd("");

  return;
}

void loopSystematics(TFile* infile, TFile* outfile, TObjArray* a_nom)
{
  //TObjArrays
  a_temp =new TObjArray();
  
  std::string systName = "";
  TIter next(infile->GetListOfKeys());
  TKey *key;
  while ((key = (TKey*)next())) {
    a_temp = new TObjArray();
    TClass *cl = gROOT->GetClass(key->GetClassName());
    if (cl->InheritsFrom("TDirectory")){
      std::string systName = key->GetName();
      
      if( !(systName.find("CutFlow")!=string::npos) ){
	  //&& systName.find("SysRate")!=string::npos ){
	
	infile->cd(systName.c_str());
	loopHists(infile, a_temp, systName);
	infile->cd("");
	if( !outfile->GetDirectory(systName.c_str()) )
	  outfile->mkdir(systName.c_str());
	outfile->cd(systName.c_str());
	//gDirectory->pwd();
	TObjArrayIter next(a_temp);
	TObject* object;
	while( (object = next()) ){
	  TH1D* htemp = (TH1D*)object;
	  if(htemp->GetNbinsX() > 0 //)
	     && !gDirectory->GetListOfKeys()->Contains(htemp->GetName()) )
	    object->Write();
	}
	outfile->cd("");
      }
    }
  }

  // add QCD norm systematic
  addQCDnormSys(infile, outfile, a_nom, "SysMODEL_QCD_Norm__1up");
  addQCDnormSys(infile, outfile, a_nom, "SysMODEL_QCD_Norm__1down");
  
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
  atlasStyle->SetPadRightMargin(0.05);
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

