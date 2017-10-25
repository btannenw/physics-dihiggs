#include "binning_vhbbrun2.hpp"

#include <iostream>
#include <cmath>
#include <map>
#include <memory>

#include <TH1.h>
#include <TString.h>

#include "binning.hpp"
#include "configuration.hpp"
#include "category.hpp"
#include "properties.hpp"

#include "HistoTransform.h"

#include <iostream>

class Sample;

BinningTool_VHbbRun2::BinningTool_VHbbRun2(const Configuration& conf) :
  BinningTool(conf)
{

}

void BinningTool_VHbbRun2::createInstance(const Configuration& conf) {
  if( !the_instance ) {
    std::cout << "INFO:    BinningTool_VHbbRun2::createInstance() BinningTool pointer is NULL." << std::endl;
    std::cout << "         Will instanciate the BinningTool service first." << std::endl;
    the_instance.reset(new BinningTool_VHbbRun2(conf));
  }
  else {
    std::cout << "WARNING: BinningTool_VHbbRun2::createInstance() BinningTool pointer already exists." << std::endl;
    std::cout << "         Don't do anything !" << std::endl;
  }
}

std::vector<int> BinningTool_VHbbRun2::getMVABinning(const Category& cat) {
  TH1* sig = cat.getSigHist();
  TH1* bkg = cat.getBkgHist();
 
  if ( cat(Property::dist) == "mvadiboson" ) {
     m_htrafo->trafoSixY = 5;
     m_htrafo->trafoSixZ = 5;
  } else {
     m_htrafo->trafoSixY = 10;
     m_htrafo->trafoSixZ = 5;
  }
  std::vector<int> bins = m_htrafo->getRebinBins(bkg, sig, 6, 0);
  delete sig;
  delete bkg;
  return bins;
}

std::vector<int> BinningTool_VHbbRun2::getCategoryBinning(const Category& c) {

  int forceRebin = m_config.getValue("ForceBinning", 0);
  if(forceRebin > 0) {
    return {forceRebin};
  }

  std::vector<int> res{};
  TH1* hist = nullptr;
  TH1* sig = c.getSigHist();
  if(sig) { hist = sig; }
  else {
    TH1* bkg = c.getBkgHist();
    hist = bkg;
  }

  if(c(Property::dist) == "mBB"){
       //res = {1}; // 100 bins in [0, 500]
       res = {4}; // bins de 20 GeV
  }

  if(c(Property::dist) == "dRBB"){
       res = {5}; // bins de 0.25 rad
  }

  if(c(Property::dist) == "mva" || c(Property::dist) == "mvadiboson"){
       //res = {50}; // 20 bins entre -1 et 1 pour 0/1/2 lep
       //delete hist;
       return getMVABinning(c);
  }

  delete hist;
  return res;

}

void BinningTool_VHbbRun2::changeRange(TH1* h, const Category& c) {

  if(c(Property::dist).Contains("mBB")){
     //changeRangeImpl(h,0,500); // full range
     changeRangeImpl(h,15,315); // centered on 125 GeV
     //changeRangeImpl(h,0,250);
     //changeRangeImpl(h,20,260);
  }
  if(c(Property::dist).Contains("dRBB")){
     changeRangeImpl(h,0,5);
  }
  if(c(Property::dist).Contains("pTV")){
     changeRangeImpl(h, 0,1000);
  }
  //if(c(Property::dist).Contains("pTV") && is_resolved == false){
  //   changeRangeImpl(h, 500,1000);
  //}
  if(c(Property::dist).Contains("MET") && c[Property::nLep] == 0){
     changeRangeImpl(h, 0,1000);
  }
  //if(c(Property::dist) == "MET" && c(Property::nLep) == 0 && is_resolved == false){
  //   changeRangeImpl(h, 500,1500);
  //}
  if(c(Property::dist) == "MET" && c(Property::nLep) == 2){
     changeRangeImpl(h, 0,100);
  }
  if(c(Property::dist).Contains("dEtaBB")){
     changeRangeImpl(h, 0,5);
  }
  if(c(Property::dist).Contains("dPhiBB")){
     changeRangeImpl(h, 0, 3.15);
  }
  if(c(Property::dist).Contains("METHT")){
     changeRangeImpl(h, 0,10);
  }
  if(c(Property::dist).Contains("MFatJ1") || c(Property::dist).Contains("MLeadFatJet")){
     changeRangeImpl(h, 50,200);
  }
  if(c(Property::dist).Contains("pTB1")){
     changeRangeImpl(h, 0,500);
  }
  if(c(Property::dist).Contains("pTB2")){
     changeRangeImpl(h, 0,500);
  }
  if(c(Property::dist).Contains("PtFatJ1") || c(Property::dist).Contains("pTLeadFatJet")){
     changeRangeImpl(h, 200,1000);
  }
  if(c(Property::dist).Contains("EtaB1") || c(Property::dist).Contains("EtaB2") || c(Property::dist).Contains("etaLeadFatJet")){
     changeRangeImpl(h, -3,3);
  }

}
