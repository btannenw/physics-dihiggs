#include "finder.hpp"

#include <iostream>
#include <utility>

#include <TString.h>

#include "category.hpp"
#include "containerhelpers.hpp"
#include "properties.hpp"
#include "sample.hpp"

std::unordered_map<TString, std::set<Sample*> > SampleIndex::m_samples{};
std::map<SType, std::set<Sample*> > SampleIndex::m_samplesByType{};

void SampleIndex::reset() {
  m_samplesByType.clear();
  m_samples.clear();
}

void SampleIndex::feed(Sample* s, const std::set<TString>& kw) {
  m_samplesByType[s->type()].insert(s);
  for(const auto& k : kw)
    m_samples[k].insert(s);
}

Sample* SampleIndex::find(const TString& k) {
  const auto& res = findAll(k);
  if(res.size() != 1) {
    std::cout << "ERROR::SampleFinder::find:" << std::endl
      << "  Key " << k << " designates several samples." << std::endl
      << "  Consider using findAll instead." << std::endl;
    throw;
  }
  return *(res.begin());
}

Sample* SampleIndex::findOrNull(const TString& k) {
  const auto& res = m_samples.find(k);
  if(res == m_samples.end()) {
    return nullptr;
  }
  if(res->second.size() != 1) {
    return nullptr;
  }
  return *(res->second.begin());
}

const std::set<Sample*>& SampleIndex::findAll(const TString& k) {
  std::cout << "INFO::SampleIndex::findAll: Looking for sample" << k << "in list."<<std::endl;
  return m_samples.at(k); // throw if not found
}


std::unordered_map<TString, Category*> CategoryIndex::m_categories{};

void CategoryIndex::feed(Category* c) {
  m_categories.emplace(c->properties().getPropertiesTag(), c);
}

Category* CategoryIndex::find(const PropertiesSet& pset) {
  std::cout << "Looking for category with properties:" << std::endl
    << "Region" << pset.getPropertiesTag() << std::endl;
  return m_categories[pset.getPropertiesTag()];
}

