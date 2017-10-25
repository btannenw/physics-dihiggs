#include "samplesbuilder_vhres.hpp"

#include <set>
#include <unordered_map>
#include <utility>
#include <iostream>
#include <map>

#include <TString.h>

#include "configuration.hpp"
#include "containerhelpers.hpp"
#include "sample.hpp"
#include "samplesbuilder_hhwwbb.hpp"

void SamplesBuilder_HHWWbb::declareSamples() {
  TString massPoint = m_config.getValue("MassPoint", "1000");

  std::cout<<"samplesbuilder_hhwwbb.cpp: declareSamples arrival"<<std::endl;
  // data or pseudo-data should always be there
  addData();
  std::cout<<"samplesbuilder_hhwwbb.cpp: after addData()"<<std::endl;
  // signal
  addSample(Sample("Xhh"+massPoint, SType::Sig, kViolet)); //BBT May 11, 2016
  //addSample(Sample("Hhhbbtautau"+massPoint, SType::Sig, kViolet));
  //addSample(Sample("Ghhbbtautau" + massPoint + "c10", SType::Sig, kViolet));
  std::cout<<"samplesbuilder_hhwwbb.cpp: after addSample(Xhh)"<<std::endl;
  // TODO add spectator samples

  // SM Higgs
  //addBkgs( { {"WlvH125", kRed}, {"ggZvvH125", kRed}, {"qqZvvH125", kRed},
  //         {"ggZllH125", kRed}, {"qqZllH125", kRed} } );

  // Z->tautau + jets
  //addBkgs( { {"Zttl", kAzure - 9} } );
  //addBkgs( { {"Zttcl", kAzure - 4}, {"Zttcc", kAzure - 2}, {"Zttbl", kAzure + 2}, {"Zttbc", kAzure + 3}, {"Zttbb", kAzure + 4} } );

  // W->taunu + jets
  //addBkgs( { {"Wttl", kGreen} } );
  //addBkgs( { {"Wttcl", kGreen}, {"Wttcc", kGreen}, {"Wttbl", kGreen}, {"Wttbc", kGreen}, {"Wttbb", kGreen} } );
  
  // DY
  //addBkgs( { {"DY", kOrange}, {"DYtt", kOrange + 1} } );
  
  
  // ttbar, stop, Zll, Wlnu
  //declareWZtopMJBkgs(); // BBT May 19, 2016: Following lines copied and slightly modified from declareWZtopMJBkgs in samplesbuilder_vhbb.cpp
  
  // diboson
  addBkgs( { {"Dibosons", kGray} } );
  //addBkgs( { {"WZ", kGray} } );
  //addBkgs( { {"WW", kGray} } );
  //addBkgs( { {"ZZ", kGray} } );
  
  // Top and single-top
  addBkgs( { {"ttbar", kOrange} } );
  addBkgs( { {"SingleTop", kYellow - 7} } );
  //addBkgs( { {"stopt", kYellow - 7} } );
  //addBkgs( { {"stops", kYellow - 5} } );
  //addBkgs( { {"stopWt", kOrange + 3} } );
  
  // W+jets
  addBkgs( { {"Wv22", kGreen + 2} } );
  //addBkgs( { {"Wenu+l", kGreen + 2}, {"Wmunu+l", kGreen + 3}, {"Wtaunu+l", kGreen + 4} } );
  //addBkgs( { {"Wenu+C", kGreen - 2}, {"Wmunu+C", kGreen - 3}, {"Wtaunu+C", kGreen - 4} } );
  //addBkgs( { {"Wenu+B", kGreen - 6}, {"Wmunu+B", kGreen - 7}, {"Wtaunu+B", kGreen - 8} } );

  // Z+jets
  addBkgs( { {"Z", kRed} } );

  // QCD
  addBkgs( { {"QCD", 29} } );
  

  std::cout<<"samplesbuilder_hhwwbb.cpp: after declareWZtopMJBkgs()"<<std::endl;
}

void SamplesBuilder_HHWWbb::declareKeywords() {
  // use standard keywords
  SamplesBuilder_VHbb::declareKeywords();

  // and add a few more
  for (const auto& spair : m_samples) {
    const Sample& s = spair.second;

    // Higgs samples deserve shorthands too
    // BBT, May 11 2016
    if(s.name().Contains("Xhh")) {
      m_keywords["hh"].insert(s.name());
    }
    //if(s.name().Contains("hhbbtautau")) {
    //  m_keywords["hh"].insert(s.name());
    //}
  }
}

void SamplesBuilder_HHWWbb::declareSamplesToMerge() {
  TString massPoint = m_config.getValue("MassPoint", "1000");

  //declareMerging(Sample("VH125", SType::Bkg, kRed), {"Higgs"});
  //declareMerging(Sample("diboson", SType::Bkg, kGray), {"Diboson"});
  //declareMerging(Sample("Whf", SType::Bkg, kGreen + 4), {"Wbb", "Wbc", "Wbl", "Wcc"});
  //declareMerging(Sample("Whf", SType::Bkg, kGreen + 4), {"Wbb", "Wbc", "Wbl", "Wcc"});
  //declareMerging(Sample("Zhf", SType::Bkg, kAzure + 4), {"Zbb", "Zbc", "Zbl", "Zcc"});

  //declareMerging(Sample("Diboson", SType::Bkg, kBlue), {"WW", "ZZ", "WZ"});
  //declareMerging(Sample("stop", SType::Bkg, kYellow - 7), {"stops", "stopt", "stopWt"});
  //declareMerging(Sample("W+light", SType::Bkg, kGreen + 2), {"Wenu+l", "Wmunu+l", "Wtaunu+l"});
  //declareMerging(Sample("W+c", SType::Bkg, kGreen + 3), {"Wenu+C", "Wmunu+C", "Wtaunu+C"});
  //declareMerging(Sample("W+b", SType::Bkg, kGreen + 4), {"Wenu+B", "Wmunu+B", "Wtaunu+B"});

}

