#include "binning_hh.hpp"

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

class Sample;

BinningTool_HH::BinningTool_HH(const Configuration& conf) :
  BinningTool(conf)
{

}

void BinningTool_HH::createInstance(const Configuration& conf) {
  if( !the_instance ) {
    std::cout << "INFO:    BinningTool_HH::createInstance() BinningTool pointer is NULL." << std::endl;
    std::cout << "         Will instanciate the BinningTool service first." << std::endl;
    the_instance.reset(new BinningTool_HH(conf));
  }
  else {
    std::cout << "WARNING: BinningTool_HH::createInstance() BinningTool pointer already exists." << std::endl;
    std::cout << "         Don't do anything !" << std::endl;
  }
}


std::vector<int> BinningTool_HH::getCategoryBinning(const Category& c) {

  int forceRebin = m_config.getValue("ForceBinning", 0);
  if(forceRebin > 0) {
    return {forceRebin};
  }

  if (m_config.getValue("OneBin", false)) {
    return oneBin(c);
  }

  if (c[Property::nTag] == 0) {
    return oneBin(c);
  }

  if (c(Property::dist) == "mMMC") {
    return {10, 44, 54, 62, 69, 79, 89, 94, 101, 106, 111, 119, 125, 131, 138, 145, 152, 159, 167, 175, 183, 194, 208, 224, 243, 266, 298, 361, 600};
    //return {50}; //0, 80, 90, 100, 105, 110, 120, 125, 130, 135, 140, 160, 600};
  }

  if (c(Property::dist) == "TauPt") {
    return {21, 22, 25, 28, 32, 37, 44, 54, 70, 133, 200};

    //return {20, 25, 30, 35, 40, 60, 100, 200};
  }

  //std::vector<int> res{20};
  //std::vector<int> res{220, 246, 272, 298, 324, 350, 376, 402, 454, 550, 1200};
  std::vector<int> res{220, 240, 260, 280, 300, 320, 340, 360, 380, 400, 450, 550, 1200};
  //std::vector<int> res{0, 257, 260, 270, 280, 290, 300, 310, 320, 400, 450, 2000};



  return res;

}

void BinningTool_HH::changeRange(TH1* h, const Category& c) {

  TString dist = c(Property::dist);

  // special case: norm-only workspaces
  if( m_config.getValue("OneBin", false)) {
    return;
  }

  if(dist == "mMMC") {
    return changeRangeImpl(h, 0., 600., false);
  }
}
