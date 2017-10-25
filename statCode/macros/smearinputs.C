#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TKey.h>
#include <TMath.h>
#include <TF1.h>
#include <TDirectory.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>

double RelBW (double* x, double* par);
TH1* smearsignal (TH1* h, float smearf, const TString& origname, const TString& signal);
TH1* smearhisto (TH1* orighisto, float mass, float smear_faction);

int print_lvl = 1;

bool DetermineSmearFromFileName (const TString &name)
{ return (name.Contains("TwoLepton") && name.Contains("SR")); }

void smearinputs (int percent_max = 5, int smear = 5)
{
  TH1::SetDefaultSumw2(kTRUE);
  std::vector<int> smear_percents;
  TString src_dir("/afs/cern.ch/work/c/cpandini/AZhInputs/21017");
  TString signal = "AZhllbb";
  TString new_directory_form = "/afs/cern.ch/work/j/jhetherl/public/smeared_inputs/21017_SmearBW%d";
  TSystemDirectory dir(src_dir.Data(), src_dir.Data());
  TList *files = dir.GetListOfFiles();
  std::vector<TString> files_to_copy, files_to_smear;

  for (int percent = 1; percent <= percent_max; ++percent)
    smear_percents.push_back(percent);

  if (files) {
    TSystemFile *file;
    TString fname;
    TString extension(".root");
    TIter next(files);
    while ((file=(TSystemFile*)next())) {
      fname = file->GetName();
      if (!file->IsDirectory() && fname.EndsWith(extension.Data())) {
        if (DetermineSmearFromFileName(fname))
          files_to_smear.push_back(fname);
        else
          files_to_copy.push_back(fname);
      }
    }
  }

  for (size_t i = 0; i < smear_percents.size(); ++i) {
    float smearf = (float)smear_percents[i]/100.0 + (float)smear/1000.0;
    TString new_dir = Form(new_directory_form.Data(), smear_percents[i]);

    gSystem->mkdir(new_dir.Data());

    for (size_t j = 0; j < files_to_copy.size(); ++j) {
      TString file_to_copy = src_dir + "/" + files_to_copy[j],
              destination_file = new_dir + "/" + files_to_copy[j];
      if (print_lvl >= 1) cout << "Copying input: " << destination_file.Data() << endl;
      gSystem->CopyFile(file_to_copy.Data(), destination_file.Data(), kTRUE);
    }

    for (size_t j = 0; j < files_to_smear.size(); ++j) {
      TString original_full_file = src_dir + "/" + files_to_smear[j],
              destination_file = new_dir + "/" + files_to_smear[j];
      TFile* _file0;
      TFile* _file1;

      TString original_file = original_full_file(original_full_file.Last('/') + 1, original_full_file.Length() - (original_full_file.Last('/') + 1));
      TString new_full_file = new_dir + "/" + original_file;
      if (print_lvl >= 1) cout << "Creating newly smeared input: " << new_full_file.Data() << endl;
      _file0 =  TFile::Open(original_full_file.Data());
      _file1 =  TFile::Open(new_full_file.Data(), "RECREATE");

      TIter nextkey(_file0->GetListOfKeys());
      TKey *key;
      while ( (key = (TKey*)nextkey()) ) {

        TString keyname=key->GetName();
        if(key&&keyname.Contains("Sys")) {
          TDirectory *local_dir = (TDirectory*)key->ReadObj();
          //Systematic list
          TString dirname=local_dir->GetName();
          _file1->cd();
          _file1->mkdir(dirname,dirname);
          // _file1->cd(dirname);
          TIter nextkey2(local_dir->GetListOfKeys());
          TKey *key2;
          while ( (key2 = (TKey*)nextkey2()) ) {
            TH1 *h = (TH1F*)key2->ReadObj();
            TString origname=key2->GetName();
            if(origname.Contains(signal)) h = smearsignal(h, smearf, origname, signal);

            _file1->cd(dirname);
            h->Write();
            _file0->cd();
          }
        }  else if(key) {
          TH1 *h = (TH1*)key->ReadObj();
          TString origname=key->GetName();
          if(origname.Contains(signal)) h = smearsignal(h, smearf, origname, signal);

          _file1->cd();
          h->Write(origname);
          _file0->cd();
        }
      }

      _file1->cd();

      _file1->Close();
      _file0->Close();
    }
  }
}

TH1* smearhisto(TH1* orighisto, float mass ,float smear) {
  //TF1* gaus=new TF1("fun1", "gaus(0)",0,2500);

  //  TF1* gaus=new TF1("fun1", "gaus(0)",0,2500);
  TF1* relbw=new TF1("RelBW", RelBW,0,2500,2);

  //Loop over all bins in original histogram
  TH1* smearedhisto=(TH1*)orighisto->Clone();
  // smearedhisto->Scale(0);
  smearedhisto->Reset("ICES");
  float width = mass*smear;
  int nbins=orighisto->GetNbinsX();
  for(int i = 1; i <= nbins; i++){
    float nevt=orighisto->GetBinContent(i);
    if(nevt == 0) continue;
    float binc=orighisto->GetBinCenter(i);
    float enevt=orighisto->GetBinError(i);
    //    gaus->SetParameters(1/width/sqrt(2*TMath::Pi()),binc,width);
    relbw->SetParameters(binc,width);
    //Now loop over new histogram and out the smeared content of this bin into the new histogram
    for(int j = 1; j <= nbins; j++){
      float bincn=smearedhisto->GetBinCenter(j);
      float cont=smearedhisto->GetBinContent(j);
      float econt=smearedhisto->GetBinError(j);
      //  float weight=gaus->Eval(bincn);
      float weight=relbw->Eval(bincn);
      smearedhisto->SetBinContent(j, cont + nevt*weight);
      econt = sqrt(econt*econt + enevt*enevt*weight);
      smearedhisto->SetBinError(j,econt);
    }
  }

  if (print_lvl >= 2) {
    cout << "For histogram " << orighisto->GetName() << " and mass " << mass << " GeV:" << endl;
    cout << "\toriginal histo area = " << orighisto->Integral() << endl;
    cout << "\tsmeared histo area before renormalization = " << smearedhisto->Integral() << endl;
  }

  // smearedhisto->Sumw2();
  if (smearedhisto->Integral() != 0) smearedhisto->Scale(orighisto->Integral()/smearedhisto->Integral());

  if (print_lvl >= 2) {
    cout << "\tsmeared histo area after renormalization = " << smearedhisto->Integral() << endl;
  }

  return smearedhisto;
}

double RelBW(double* x, double* par){
  //Relativistic BW
  double mass=par[0];
  double width=par[1];

  double width2=width*width;
  double mass2=mass*mass;

  double gamma=sqrt(mass2*(mass2+width2));
  double k=2*sqrt(2)*mass*width*gamma/(TMath::Pi()*sqrt(mass2+gamma));

  double relBW=k/(pow(x[0]*x[0]-mass2,2)+mass2*width2);

  return relBW;
}

float GetMassFromName (const TString &name, const TString &signame)
{
  if (name.Contains(signame)) {
    int i = name.First(signame.Data()) + signame.Length();
    int start = i,
        end = i;
    while (true) {
      TString c = TString(name[i]);
      if (!c.IsDec() && start == end) {
        ++i;
        continue;
      }
      if (c.IsDec() && start == end) {
        start = i;
        end = 0;
        ++i;
        continue;
      }
      if (!c.IsDec()) {
        end = i;
        break;
      }
      ++i;
    }
    TString mass_str = name(start, end - start);
    return mass_str.Atof();
  }
  return 0.0;
}

TH1* smearsignal(TH1* h, float smearf, const TString& origname, const TString& signal){
  h->SetNameTitle(origname, h->GetTitle());
  float mass = GetMassFromName(origname, signal);
  return smearhisto(h, mass, smearf);
}
