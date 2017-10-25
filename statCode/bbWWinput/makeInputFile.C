#include "TFile.h"
#include "TH1.h"
#include "TObjArray.h"

#include "string"
#include "iostream"

void addSample(std::string sample, std::string region, TFile* f0, TH1D*& h0, TObjArray*& array, std::string name, std::string optStrategy, std::string syst = "", std::string plotVar = "");
void addRegion(std::string region, TFile* f0, TH1D*& h0, TObjArray*& array, std::string name, std::string optStrategy, std::string syst = "", std::string plotVar = "");

bool addPseudoData = false;
bool doScan = false;

void makeInputFile(std::string name, std::string optStrategy)
{
  bool controlRegion = true;
  bool runSystematics = false;

  std::string topDir = "/afs/cern.ch/work/s/suyogs/diHiggs/data/histograms/apr23_hhWindows/IsoOR/el_mu/mcYields/"; // run over histograms produced at OSU. Unblinded and 2015+2016
  
  //std::string name = "non-res", "Xhh300", "Xhh400", "Xhh500", "Xhh600", "Xhh700", "Xhh750", "Xhh800", "Xhh1000", "Xhh1100", "Xhh1300", "Xhh1400", "Xhh1500", "Xhh2000"
  //std::string optStrategy = "non-res", "Xhh700, "Xhh2000"

  if ( optStrategy != "non-res" && optStrategy != "Xhh700" && optStrategy != "Xhh2000"){
    std::cout<<"optStrategy: "<<optStrategy<<" NOT RECOGNIZED. ABORT!!!"<<std::endl;
    return;
  }
  
  if ( name != "non-res" && name != "Xhh300" && name != "Xhh400" && name != "Xhh500" && name != "Xhh600" && 
       name != "Xhh700" && name != "Xhh750" && name != "Xhh800" && name != "Xhh900" && name != "Xhh1000" && name != "Xhh1100" && name != "Xhh1200" && 
       name != "Xhh1300" && name != "Xhh1400" && name != "Xhh1500" && name != "Xhh1600" && name != "Xhh1800" && name != "Xhh2000" && name != "Xhh2250" && name != "Xhh2500" && 
       name != "Xhh2750" && name != "Xhh3000" && name != "Xhh5000"){
    std::cout<<"name: "<<name<<" NOT RECOGNIZED. ABORT!!!"<<std::endl;
    return;
  }

  //TFile *fIn = new TFile( (topDir+"hist-kinFit-data_ttbar_wjets_qcd_singletop_zjets_diboson_signal.root").c_str(),"READ"); // stat+syst histograms for all masses in both selections
  TFile *fIn = new TFile( (topDir+"qcdHistograms_optNoRes_hhSM_bbMass.root").c_str(),"READ"); // stat+syst histograms for all masses in both selections
  
  std::string optStrat;
  if ( optStrategy.find("Xhh")!=string::npos)
    optStrat = "opt"+optStrategy.substr(3,optStrategy.size());
  else
    optStrat = "optNonRes";

  TFile *fNew;
  if( name == "non-res") 
    fNew = new TFile( (topDir+"13TeV.CUT.XhhSM.hist-all."+ optStrat +".root").c_str() ,"RECREATE");
  else if (name.find("Xhh")!=string::npos && (name.find("300")!=string::npos || name.find("400")!=string::npos || name.find("500")!=string::npos || name.find("600")!=string::npos || name.find("700")!=string::npos || name.find("750")!=string::npos || name.find("800")!=string::npos || name.find("900")!=string::npos || name.find("1000")!=string::npos || name.find("1100")!=string::npos || name.find("1200")!=string::npos || name.find("1300")!=string::npos || name.find("1400")!=string::npos || name.find("1500")!=string::npos || name.find("1600")!=string::npos || name.find("1800")!=string::npos || name.find("2000")!=string::npos || name.find("2250")!=string::npos || name.find("2500")!=string::npos || name.find("2750")!=string::npos || name.find("3000")!=string::npos || name.find("5000")!=string::npos )) 
    fNew = new TFile( (topDir+"13TeV.CUT."+ name +".hist-all."+ optStrat +".root").c_str() ,"RECREATE");
  else{
    std::cout<<"Something's wrong. ABORT."<<std::endl;
    return;
  }

  TObjArray *hists = new TObjArray();

  TH1D *h_noLH = new TH1D();
  TH1D *h_cr = new TH1D();
  TH1D *h_scan = new TH1D();
  TH1D *h_sys = new TH1D();
 
  //OLD: 'XhhSM', 'Xhh2000', 'Xhh5000', 'Xhh700', 'ttbar', 'Z', 'Wenu+C', 'Wenu+B', 'Wenu+l', 'Wmunu+l', 'Wmunu+B', 'Wmunu+C', 'Wtaunu+l', 'Wtaunu+B', 'Wtaunu+C', 'stop', 'WZ', 'WW', 'ZZ'
  //Jun 27: 'XhhSM', 'Xhh2000', 'Xhh5000', 'Xhh700', 'ttbar', 'Z', 'Wjets', 'stop'

  addRegion("SR", fIn, h_noLH, hists, name, optStrategy);
  if(controlRegion)  {addRegion("mBBcr", fIn, h_cr, hists, name, optStrategy);}
  if(doScan)         {addRegion("SR", fIn, h_scan, hists, name, optStrategy, "", "LogLikelihood_ttbar");
                      addRegion("SR", fIn, h_scan, hists, name, optStrategy, "", "LogLikelihood_dihiggs");
  }

  // Add all systematics
  if(runSystematics){
    // A. By Hand
    //addSystematic("SR", fIn, h_sys, hists, name, "SysJET_JER_SINGLE_NP__1up"); 
    
    // B. Automatically
    TIter next(fIn->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
      TClass *cl = gROOT->GetClass(key->GetClassName());
      if (cl->InheritsFrom("TDirectory")){
	std::string systName = key->GetName();
	
	if( !(systName.find("SysJET_SR2")!=string::npos ||
	      systName.find("SysJET_SR3")!=string::npos ||
	      systName.find("SysJET_SR4")!=string::npos ||
	      systName.find("SysMODEL_ZJETS_Norm")!=string::npos ||
	      systName.find("SysMODEL_WJETS_Norm")!=string::npos ||
	      systName.find("SysMODEL_TTbar_PS")!=string::npos ||
	      systName.find("SysMODEL_TTbar_ME")!=string::npos ||
	      systName.find("SysMODEL_TTbar_RadHi")!=string::npos ||
	      systName.find("SysMODEL_TTbar_RadLo")!=string::npos ||
	      systName.find("SysMODEL_TTbar_Scale")!=string::npos ||
	      systName.find("SysMODEL_TTbar_PDF")!=string::npos 

	      )){  // Keep all systematics except for ones listed(Aug 17, 2016)
	
	  std::cout<<"name: "<<key->GetName()<<std::endl;
	  addRegion("SR", fIn, h_sys, hists, name, optStrategy, systName);
	  addRegion("mBBcr", fIn, h_sys, hists, name, optStrategy, systName);
	}
      }
    }
  }
  
  
  std::cout<<"DONE ADDING"<<std::endl;
  
  fNew->cd();
  
  //std::cout<<"Move to file"<<std::endl;
  
  TObjArrayIter next(hists);
  TObject* object;
  while( (object = next()) ){
    //std::cout<<"0... | "<<std::endl;
    TH1D* htemp = (TH1D*)object;
    //std::cout<<"writing "<<htemp->GetTitle()<<std::endl;
    //if (htemp->Integral() < 0)
    //  htemp->Scale(0);
    
    std::string systCheck = htemp->GetTitle();
    if ( systCheck.find("Sys")!=string::npos ){
      if( !fNew->GetDirectory("Systematics") )
	fNew->mkdir("Systematics");
      fNew->cd("Systematics");
    }
    object->Write();
    //std::cout<<"wrote "<<htemp->GetTitle()<<std::endl;
    fNew->cd("");
  }
  
  //std::cout<<"LOOPED"<<std::endl;
  
  fNew->Close();
  
  std::cout<<"LOOPED"<<std::endl;
  
  return;
}

void addSample(std::string sample, std::string region, TFile* f0, TH1D*& h0, TObjArray*& array, std::string name, std::string optStrategy, std::string syst, std::string plotVar)
{
  f0->cd("");

  h0 = new TH1D();
  TH1D* temp = new TH1D();
  TH1D* temp2 = new TH1D();
  TH1D* tempScan = new TH1D();
  
  double xbins = 0;
  double xmax = 0;
  double xmin = 0;

  if(plotVar==""){
    plotVar = "bbMass";
    xbins = 100; //bbMass
    xmax = 500e3; //bbMass
    xmin = 0; //bbMass
    //plotVar = "LepEta";
    //plotVar = "bbPt";
    //double xbins = 100; //bbPt
    //double xmax = 1000e3; //bbPt
    //double xmin = 0; //bbPt
    //double xbins = 100; //LepEta
    //double xmax = 5; //LepEta
    //double xmin = -5; //LepEta
  }
  else{
    if (plotVar.find("Likelihood_ttbar")!=string::npos){
      xbins = 50; //LogLikelihood_ttbar
      xmax = -40; //LogLikelihood_ttbar
      xmin = -90; //LogLikelihood_ttbar
    }
    else if (plotVar.find("Likelihood_dihiggs")!=string::npos){
      xbins = 50; //LogLikelihood_dihiggs
      xmax = -10; //LogLikelihood_dihiggs
      xmin = -60; //LogLikelihood_dihiggs
    }
  }

  std::string mass;
  std::string region1 = "";

  if(syst!=""){
    if( !f0->GetDirectory(syst.c_str())){
      std::cout<<"WARNING!! Histogram directory "<<syst<<" NOT FOUND! ABORTING."<<std::endl;
      return;
    }
  }

  if (optStrategy == "non-res" && region == "mBBcr")
    region1 = "_opt700_bbpt150_";
  else if (optStrategy == "Xhh700" && region == "mBBcr") 
    region1 = "_opt700_bbpt150_";
  else if (optStrategy == "Xhh2000" && region == "mBBcr") 
    region1 = "_opt2000_bbpt350_";
  else if (optStrategy == "non-res" && region == "SR") {
    region1 = "_optNoRes_bbpt230_drbb12_drww11_mww_hh"+name.substr(3,name.size())+"_mbb_";
  }
  else if (optStrategy == "Xhh700" && region == "SR") {
    region1 = "_opt700_bbpt150_drbb11_drww09_mww_hh"+name.substr(3,name.size())+"_mbb_";
    //region1 = "_opt700_bbpt150_drbb11_drww09_mww_mbb_hh"+name.substr(3,name.size())+"_"; // for 2015only and/or 2016only
  }
  else if (optStrategy == "Xhh2000" && region == "SR") {
    if (!(name.find("2000")!=string::npos || name.find("2250")!=string::npos || name.find("2500")!=string::npos || name.find("2750")!=string::npos || name.find("3000")!=string::npos || name.find("5000")!=string::npos))
      region1 = "_opt2000_bbpt350_wwpt360_drww20_hh"+name.substr(3,name.size())+"_mbb_"; 
    else
      region1 = "_opt2000_bbpt350_wwpt360_drww20_hh2000above_mbb_";
    
    /* // for 2015only and/or 2016only
    if (!(name.find("2000")!=string::npos || name.find("2250")!=string::npos || name.find("2500")!=string::npos || name.find("2750")!=string::npos || name.find("3000")!=string::npos || name.find("5000")!=string::npos))
      region1 = "_opt2000_bbpt350_wwpt360_drww20_mbb_hh"+name.substr(3,name.size())+"_"; // FIXME
    else
      region1 = "_opt2000_bbpt350_wwpt360_drww20_mhh_mbb_";
    */
 }   
  else{
    std::cout<<"2### name: "<<name<<"\t### region: "<<region<<" NOT RECOGNIZED. Abort."<<std::endl;
    return;
  }

  
  if (sample == "non-res")
    sample = "XhhSM";

  std::string histName = (sample+"_"+region + region1+ plotVar + syst).c_str();
  if (syst!=""){
    histName = (sample+"_"+region + region1+ plotVar + "_" + syst).c_str();
    //std::cout<<"Syst histo name: "<<histName<<std::endl;
    f0->cd(syst.c_str());
    syst = ("_"+syst).c_str();
  }
  
  //cout<<"CHECK |"<<histName<<"|"<<endl;
  //f0->ls();
  if( gDirectory->GetListOfKeys()->Contains( histName.c_str() ) ){
    //cout<<"I exist "<<histName<<endl;
    //if (histName.find("QCD_SR_opt2000_bbpt350_wwpt360_drww20_mhh_mbb_")!=string::npos)
    //if (histName.find("QCD_SR_")!=string::npos)
    //  h0 = new TH1D(histName.c_str(), histName.c_str(), xbins, xmin, xmax);
    //else{
      temp = (TH1D*)gDirectory->Get( histName.c_str() );
      temp->SetTitle("h0");
      temp->SetName("h0");
      if( temp->Integral() < 0 ) {
	h0 = new TH1D(histName.c_str(), histName.c_str(), xbins, xmin, xmax);
	h0->SetBinContent(12,0);
	h0->SetBinContent(13,0);
      }
      else
	h0 = temp;
      //}
  }
  else if(sample=="data" && region=="SR" && addPseudoData){
    //cout<<"data SR"<<endl;
    std::vector<string> samples = {"ttbar","Z","Wv22","SingleTop","QCD","Dibosons"};

    for (int i = 0; i < samples.size(); i++){
      std::string histName2 = (samples[i]+"_"+region + region1+ plotVar + syst).c_str();
      if( gDirectory->GetListOfKeys()->Contains( histName2.c_str() ) ){
	//cout<<histName2<<endl;
	if (i==0){
	  temp = (TH1D*)gDirectory->Get( histName2.c_str() );
	  temp->SetTitle(histName.c_str());
	  temp->SetName(histName.c_str());
	}
	else{
	  temp2 = (TH1D*)gDirectory->Get( histName2.c_str() );
	  temp->Add(temp2);
	} 
      }
    }
    h0 = temp;
  }
  else{
    //cout<<"data SR DNE"<<endl;
    h0 = new TH1D(histName.c_str(), histName.c_str(), xbins, xmin, xmax);
  }

  if (sample == "XhhSM")
    sample = "Xhh250";

  //if(sample == "data" && region == "mBBcr")
  //  cout<<"$$$$$$$$ histName: |"<<histName<<"|\t Integral(): "<<h0->Integral()<<endl;
  
  //h0->SetName( (sample+"_2tag4pjet_0ptv_"+region+"_"+optStrategy+"_"+plotVar +  syst).c_str());
  //h0->SetTitle( (sample+"_2tag4pjet_0ptv_"+region+"_"+optStrategy+"_"+plotVar + syst).c_str());

  h0->SetName( (sample+"_2tag4pjet_0ptv_"+region+"_"+plotVar +  syst).c_str());
  h0->SetTitle( (sample+"_2tag4pjet_0ptv_"+region+"_"+plotVar + syst).c_str());

  //if(sample.find("Xhh800")!=string::npos)
  //  h0->Scale(42./30);
  /*
  if (sample == "ttbar" && region=="SR" && optStrategy == "Xhh700"){
    h0->Scale(1.12);
    cout<<"!!! 1.12 : "<<(sample+"_2tag4pjet_0ptv_"+region+"_"+plotVar + syst).c_str()<<endl;
  }
  if (sample == "ttbar" && region=="SR" && optStrategy == "Xhh2000"){
    h0->Scale(1.05);
    cout<<"!!! 1.05 : "<< (sample+"_2tag4pjet_0ptv_"+region+"_"+plotVar + syst).c_str()<<endl;
    }
  */

  // store overflow in final bin
  h0->SetBinContent(xbins, h0->GetBinContent(xbins) + h0->GetBinContent(xbins+1));
  h0->SetBinContent(xbins+1, h0->GetBinContent(xbins+1), 0);

  
  array->AddLast(h0); // normal filling
  if (doScan){
    if (plotVar.find("Likelihood")!=string::npos){ // add histograms scanned via cut on likelihood

      if (plotVar.find("Likelihood_ttbar")!=string::npos){ // add histograms scanned via cut on likelihood
	for(int maxBin = 1; maxBin < xbins; maxBin++){ 
	  tempScan = (TH1D*)h0->Clone();
	  for (int i = maxBin; i < tempScan->GetNbinsX(); i++){
	    tempScan->SetBinContent(i,0);
	    tempScan->SetBinError(i,0);   
	  }
	  
	  string sMax;          // string which will contain the result
	  ostringstream convert;   // stream used for the conversion
	  convert << maxBin;      // insert the textual representation of 'Number' in the characters in the stream
	  sMax = convert.str(); // set 'Result' to the contents of the stream
	  //std::cout<<"sMax: "<<sMax<<std::endl;
	  tempScan->SetName( (sample+"_2tag4pjet_0ptv_"+region+"_" + plotVar + "_scan" + sMax + syst).c_str());
	  tempScan->SetTitle( (sample+"_2tag4pjet_0ptv_"+region+"_" + plotVar + "_scan" + sMax + syst).c_str());
	  
	  array->AddLast(tempScan);
	}
      } // end ttbar logLikelihood

      // start with dihiggs log(likelihood)
      if (plotVar.find("Likelihood_dihiggs")!=string::npos){ // add histograms scanned via cut on likelihood
	for(int minBin = 1; minBin < xbins; minBin++){ 
	  tempScan = (TH1D*)h0->Clone();
	  for (int i = 0; i < minBin; i++){
	    tempScan->SetBinContent(i,0);
	    tempScan->SetBinError(i,0);   
	  }
	  
	  string sMax;          // string which will contain the result
	  ostringstream convert;   // stream used for the conversion
	  convert << minBin;      // insert the textual representation of 'Number' in the characters in the stream
	  sMax = convert.str(); // set 'Result' to the contents of the stream
	  //std::cout<<"sMax: "<<sMax<<std::endl;
	  tempScan->SetName( (sample+"_2tag4pjet_0ptv_"+region+"_" + plotVar + "_scan" + sMax + syst).c_str());
	  tempScan->SetTitle( (sample+"_2tag4pjet_0ptv_"+region+"_" + plotVar + "_scan" + sMax + syst).c_str());
	  
	  array->AddLast(tempScan);
	}
      } // end dihiggs logLikelihood

    }
  }

  f0->cd("");

  return;
}

void addRegion(std::string region, TFile* f0, TH1D*& h0, TObjArray*& array, std::string name, std::string optStrategy, std::string syst, std::string plotVar)
{
  addSample("data", region, f0, h0, array, name, optStrategy, syst, plotVar);
  addSample("ttbar",region,f0, h0, array, name, optStrategy, syst, plotVar);
  addSample("SingleTop", region, f0, h0, array, name, optStrategy, syst, plotVar);
  addSample("Wv22", region, f0, h0, array, name, optStrategy, syst, plotVar);
  addSample("Z", region, f0, h0, array, name, optStrategy, syst, plotVar);
  addSample("QCD", region, f0, h0, array, name, optStrategy, syst, plotVar);
  addSample("Dibosons", region, f0, h0, array, name, optStrategy, syst, plotVar);
  addSample(name, region, f0, h0, array, name, optStrategy, syst, plotVar);
}
