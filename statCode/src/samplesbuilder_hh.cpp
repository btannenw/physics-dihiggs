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
#include "samplesbuilder_hh.hpp"

void SamplesBuilder_HH::declareSamples() {
  TString massPoint = m_config.getValue("MassPoint", "1000");

  TString type = m_config.getValue("Type", "RSG");

  // data or pseudo-data should always be there
  addData();

  // signal
  if (type == "RSG") {
    addSample(Sample("Ghhbbtautau" + massPoint + "c10", SType::Sig, kViolet));
  } else if (type == "2HDM") {
    addSample(Sample("Hhhbbtautau"+massPoint, SType::Sig, kViolet));
    addSample(Sample("X"+massPoint, SType::Sig, kViolet));
  } else if (type == "SM") {
    addSample(Sample("hhttbb", SType::Sig, kViolet));
  }

  // TODO add spectator samples

  // SM Higgs
  addBkgs( { {"WlvH125", kRed}, {"ggZvvH125", kRed}, {"qqZvvH125", kRed},
           {"ggZllH125", kRed}, {"qqZllH125", kRed} } );

  // Z->tautau + jets
  addBkgs( { {"Zttl", kAzure - 9} } );
  addBkgs( { {"Zttcl", kAzure - 4}, {"Zttcc", kAzure - 2}, {"Zttbl", kAzure + 2}, {"Zttbc", kAzure + 3}, {"Zttbb", kAzure + 4} } );

  // W->taunu + jets
  addBkgs( { {"Wttl", kGreen} } );
  addBkgs( { {"Wttcl", kGreen}, {"Wttcc", kGreen}, {"Wttbl", kGreen}, {"Wttbc", kGreen}, {"Wttbb", kGreen} } );

  // diboson
  addBkgs( { {"WZ", kGray}, {"ZZ", kGray + 1}, {"WW", kGray + 3} } );

  // DY
  addBkgs( { {"DY", kOrange}, {"DYtt", kOrange + 1} } );

  // ttbar, stop, Zll, Wlnu
  declareWZtopMJBkgs();

  addSample(Sample("Fake", SType::DataDriven, kPink + 1));

}

void SamplesBuilder_HH::declareKeywords() {
  // use standard keywords
  SamplesBuilder_VHbb::declareKeywords();

  // and add a few more
  for (const auto& spair : m_samples) {
    const Sample& s = spair.second;

    // Higgs samples deserve shorthands too
    if(s.name().Contains("hhbbtautau")) {
      m_keywords["hh"].insert(s.name());
    }
  }
}

void SamplesBuilder_HH::declareSamplesToMerge() {
  TString massPoint = m_config.getValue("MassPoint", "1000");

  declareMerging(Sample("VH125", SType::Bkg, kRed), {"Higgs"});
  declareMerging(Sample("diboson", SType::Bkg, kGray), {"Diboson"});
  declareMerging(Sample("stop", SType::Bkg, kYellow - 7), {"Stop"});
  declareMerging(Sample("Whf", SType::Bkg, kGreen + 4), {"Wbb", "Wbc", "Wbl", "Wcc"});
  declareMerging(Sample("Zhf", SType::Bkg, kAzure + 4), {"Zbb", "Zbc", "Zbl", "Zcc"});
}

