#include "samplesbuilder.hpp"

#include <sstream>
#include <iostream>
#include <utility>

#include <TString.h>

#include "containerhelpers.hpp"
#include "sample.hpp"
#include "finder.hpp"

class Configuration;

SamplesBuilder::SamplesBuilder(const Configuration& conf, std::unordered_map<TString, Sample>& samples,
                             std::unordered_map<TString, std::set<TString>>& keywords,
                             std::map<Sample*, std::vector<Sample*>>& samplesToMerge) :
  m_config(conf), m_samples(samples), m_keywords(keywords), m_samplesToMerge(samplesToMerge)
{}

SamplesBuilder::~SamplesBuilder() {}

void SamplesBuilder::addSample(Sample&& sample) {
  m_samples.emplace(sample.name(), std::move(sample));
}

void SamplesBuilder::addBkgs(const std::unordered_map<TString, int>& bkgs) {
  for(const auto& bkg : bkgs) {
    addSample(Sample(bkg.first, SType::Bkg, bkg.second));
  }
}

void SamplesBuilder::addData() {
  bool usePD        = m_config.getValue("UsePseudoData", false);
  int  nReplica     = m_config.getValue("ReplicaCount",      0);

  // data or pseudo-data should always be there
  std::stringstream ss;
  ss << "data";
  //BBT, Aug 25 2016: comment out appending number to "data" for pseudoData. basically grasping at straws here
  //if(usePD) {
  //  ss << nReplica;
  // }
  addSample(Sample(ss.str(), SType::Data, kBlack));
}

void SamplesBuilder::declareMerging(Sample&& newS, const std::vector<TString>& names) {
  std::vector<Sample*> samplesToMerge;
  // fill samplesToMerge from names, if their type matches with the type of newS
  for(const TString& s : names) {
    for(auto p : SampleIndex::findAll(s)) {
      if(p->type() == newS.type()) {
        samplesToMerge.push_back(p);
      }
    }
  }

  if(samplesToMerge.size() < 2) {
    std::cerr << "SamplesBuilder::declareMerging: You need to merge at least two samples ! Abort..." << std::endl;
    throw;
  }

  std::cout <<"INFO::SamplesBuilder:declareMerging: will attempt to merge the following samples:" << std::endl;
  for(auto p : samplesToMerge) {
    std::cout << "    " << p->name() << std::endl;
  }

  TString newSname(newS.name());
  m_samples.emplace(newSname, std::move(newS));
  Sample& sample = m_samples.at(newSname);
  m_samplesToMerge.emplace(&sample, samplesToMerge);
}

