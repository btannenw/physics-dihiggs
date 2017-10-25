#include "binning_hhwwbb.hpp"

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

BinningTool_HHWWbb::BinningTool_HHWWbb(const Configuration& conf) :
  BinningTool(conf)
{

}

void BinningTool_HHWWbb::createInstance(const Configuration& conf) {
  if( !the_instance ) {
    std::cout << "INFO:    BinningTool_HHWWbb::createInstance() BinningTool pointer is NULL." << std::endl;
    std::cout << "         Will instanciate the BinningTool service first." << std::endl;
    the_instance.reset(new BinningTool_HHWWbb(conf));
  }
  else {
    std::cout << "WARNING: BinningTool_HHWWbb::createInstance() BinningTool pointer already exists." << std::endl;
    std::cout << "         Don't do anything !" << std::endl;
  }
}


std::vector<int> BinningTool_HHWWbb::getCategoryBinning(const Category& c) {

  int forceRebin = m_config.getValue("ForceBinning", 0);
  std::cout << "INFO-Ben:    BinningTool_HHWWbb::getCategoryBinning(), forceRebin = "<<forceRebin<<std::endl;
  if(forceRebin > 0) {
    return {forceRebin};
  }

  if (m_config.getValue("OneBin", false)) {
    std::cout << "INFO-Ben:    BinningTool_HHWWbb::getCategoryBinning(), return oneBin"<<std::endl;
    return oneBin(c);
  }

  if (c[Property::nTag] == 0) {
    return oneBin(c);
  }

  //return oneBin(c); // BBT, May 18 2016: makes all distros 1 bin, e.g. counting experiment

  std::vector<int> res{4};
  //std::vector<int> res{0, 20e3, 40e3, 60e3, 80e3, 100e3, 120e3, 140e3, 160e3, 180e3, 200e3, 220e3, 240e3, 260e3, 280e3, 300e3, 320e3, 340e3, 360e3, 380e3, 400e3, 420e3, 440e3, 460e3, 480e3, 500e3};
  std::cout << "INFO-Ben:    BinningTool_HHWWbb::getCategoryBinning(), doing the thing"<<std::endl;
  return res;

}

