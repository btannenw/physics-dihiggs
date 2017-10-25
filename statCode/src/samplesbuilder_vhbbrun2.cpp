#include "samplesbuilder_vhbbrun2.hpp"

#include <set>
#include <unordered_map>
#include <utility>
#include <iostream>

#include <TString.h>

#include "configuration.hpp"
#include "containerhelpers.hpp"
#include "sample.hpp"
#include "samplesbuilder_vhbb.hpp"


void SamplesBuilder_VHbbRun2::declareSamples() {


  TString massPoint = m_config.getValue("MassPoint", "125");
  int injection     = m_config.getValue("DoInjection", 0);
  TString massInjection = TString::Itoa(injection, 10);

  // data or pseudo-data should always be there
  addData();


  // Signal samples and corresponding backgrounds
  if(m_config.getValue("DoDiboson", false)){ // considering diboson as the signal
    addSample(Sample("WZ", SType::Sig, kGray));
    addSample(Sample("ZZ", SType::Sig, kGray + 1));
    // higgs background at 125GeV
    addBkgs( { {"WlvH125", kRed}, {"qqWlvH125", kRed}, {"ggZvvH125", kRed}, {"qqZvvH125", kRed},
             {"ggZllH125", kRed}, {"qqZllH125", kRed}, {"ZllH125", kRed} } );
    addBkgs( { {"WW", kGray + 3} } );
  }
  else { // H125 signal
    // signal
    addSample(Sample("ZllH"+massPoint, SType::Sig,  kRed));
    addSample(Sample("ggZllH"+massPoint, SType::Sig,  kRed));
    addSample(Sample("qqZllH"+massPoint, SType::Sig,  kRed));
    addSample(Sample("WlvH"+massPoint, SType::Sig,  kRed));
    addSample(Sample("qqWlvH"+massPoint, SType::Sig,  kRed));
    addSample(Sample("ggZvvH"+massPoint, SType::Sig,  kRed));
    addSample(Sample("qqZvvH"+massPoint, SType::Sig,  kRed));

    // diboson bkg
    addBkgs( { {"WZ", kGray}, {"ZZ", kGray + 1}, {"WW", kGray + 3} } );

    // signal injection
    if(injection>0 && massInjection != massPoint) {
      addSample(Sample("ZllH"+massInjection, SType::None,  kBlue));
      addSample(Sample("ggZllH"+massInjection, SType::None,  kBlue));
      addSample(Sample("qqZllH"+massInjection, SType::None,  kBlue));
      addSample(Sample("qqWlvH"+massInjection, SType::None,  kBlue));
      addSample(Sample("WlvH"+massInjection, SType::None,  kBlue));
      addSample(Sample("ggZvvH"+massInjection, SType::None,  kBlue));
      addSample(Sample("qqZvvH"+massInjection, SType::None,  kBlue));
    }
  }

  // Top, single-top, W/Z+jets and multijet
  declareWZtopMJBkgs();

}


void SamplesBuilder_VHbbRun2::declareKeywords() {

  TString massPoint = m_config.getValue("MassPoint", "125");
  // from run1
  // First, some user-definitions
  m_keywords = {
    {"Diboson", {"WZ", "ZZ", "WW"}},
    {"Zjets", {"Zcl", "Zcc", "Zbl", "Zbb", "Zl", "Zbc"}},
    {"Zc", {"Zcl", "Zcc"}},
    {"Zb", {"Zbl", "Zbb", "Zbc"}},
    {"Zhf", {"Zbl", "Zbb", "Zbc", "Zcc"}},
    {"ZbORc", {"Zbl", "Zbb", "Zbc", "Zcl", "Zcc"}},
    {"Wjets", {"Wcl", "Wcc", "Wbl", "Wbb", "Wl", "Wbc"}},
    {"Wb", {"Wbl", "Wbb", "Wbc"}},
    {"Whf", {"Wcc", "Wbl", "Wbb", "Wbc"}},
    {"WbbORcc", {"Wcc", "Wbb"}},
    {"WbcORbl", {"Wbl", "Wbc"}},
    {"Stop", {"stops", "stopt", "stopWt"}},
    {"Top", {"ttbar", "stops", "stopt", "stopWt"}}
  };

  for (const auto& spair : m_samples) {
    const Sample& s = spair.second;
    // Higgs samples deserve shorthands too
    if(s.name().Contains("ZvvH")) {
      m_keywords["ZvvH"].insert(s.name());
      m_keywords["Higgs"].insert(s.name());
      m_keywords["ZH"].insert(s.name());
    }
    if(s.name().Contains("WlvH")) {
      m_keywords["WlvH"].insert(s.name());
      m_keywords["Higgs"].insert(s.name());
      m_keywords["WH"].insert(s.name());
    }
    if(s.name().Contains("ZllH")) {
      m_keywords["ZllH"].insert(s.name());
      m_keywords["Higgs"].insert(s.name());
      m_keywords["ZH"].insert(s.name());
    }
    if(s.name().BeginsWith("qq") || s.name().BeginsWith("Wlv")) {
      m_keywords["qqVH"].insert(s.name());
    }
    if(s.name().BeginsWith("qqZ")) {
      m_keywords["qqZH"].insert(s.name());
    }
    if(s.name().BeginsWith("ggZ")) {
      m_keywords["ggZH"].insert(s.name());
    }
  }

}

void SamplesBuilder_VHbbRun2::declareSamplesToMerge() {
  bool doDiboson = m_config.getValue("DoDiboson", false);
  bool doWHZH = m_config.getValue("FitWHZH", false);
  bool doVHVZ = m_config.getValue("FitVHVZ", false);
  TString massPoint = m_config.getValue("MassPoint", "125");
  int injection     = m_config.getValue("DoInjection", 0);
  TString massInjection = TString::Itoa(injection, 10);

  // VH sample merging
  if(doWHZH) {
    declareMerging(Sample("ZH"+massPoint, SType::Sig, kRed), {"ZH"});
    declareMerging(Sample("WH"+massPoint, SType::Sig, kRed), {"WH"});
  }
  else if (doDiboson) {
    declareMerging(Sample("VH"+massPoint, SType::Bkg, kRed), {"Higgs"});
  }
  else {
    declareMerging(Sample("VH"+massPoint, SType::Sig, kRed), {"Higgs"});
  }

  if(injection>0 && massInjection != massPoint) {
    declareMerging(Sample("VH"+massInjection, SType::None, kBlue), {"Higgs"});
  }

  // diboson sample merging
  if(doDiboson) {
    declareMerging(Sample("VZ", SType::Sig, kGray), {"Sig"});
  }
  else if(doVHVZ) {
    declareMerging(Sample("VZ", SType::Bkg, kGray), {"WZ", "ZZ"});
  }
  else {
    declareMerging(Sample("diboson", SType::Bkg, kGray), {"Diboson"});
  }

  declareMerging(Sample("stop", SType::Bkg, kYellow - 7), {"Stop"});
  declareMerging(Sample("Whf", SType::Bkg, kGreen + 4), {"Wbb", "Wbc", "Wbl", "Wcc"});
  declareMerging(Sample("Zhf", SType::Bkg, kAzure + 4), {"Zbb", "Zbc", "Zbl", "Zcc"});
}

