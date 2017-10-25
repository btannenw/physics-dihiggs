#include <iostream>
#include <cstdlib>
#include <file.h>
#include <utils.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TH1F.h>

int main(int argc, char** argv) {
  gROOT->SetBatch(true);
  if(argc<2) {
    std::cout << "Usage: addData inputFileName" << std::endl;
    return 0;

  }

  std::string fname = argv[1];
  File inFile(fname, "update");
  if(inFile.getObject<TH1F>("data") != nullptr) {
    // data is there: do nothing
    return 0;
  }

  // Spyros: Add warning 
  std::cout << "WARNING:addData : Data histogram does not exist in " << fname << std::endl;

  auto plots = inFile.getObjects<TH1F>();
  // Spyros: following method segfaults if ttbar does not exist
  //auto ttbar = inFile.getObject<TH1F>("ttbar");
  //TH1F* data = dynamic_cast<TH1F*>(ttbar->Clone("data"));
  TH1F* data = dynamic_cast<TH1F*>(plots.at(0)->Clone("data"));
  data->Reset();

  for(auto p : plots) {
    TString pname = p->GetName();
    if(! pname.Contains("HVT") && ! pname.Contains("AZh")) { data->Add(p); }
  }
  data->Write();

  return 0;
}
