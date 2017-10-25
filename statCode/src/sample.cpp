#include "sample.hpp"

#include <iostream>
#include <cstdlib>
#include <utility>
#include <TString.h>

#include "category.hpp"
#include "sampleincategory.hpp"
#include "properties.hpp"
#include "systematic.hpp"
#include "systematichandler.hpp"
#include "finder.hpp"

Sample::Sample(const TString& name, std::vector<TString>&& subsamples, const Type type, const int color) :
  m_name(name), m_keywords(), m_type(type), m_color(color), m_subsamples(std::move(subsamples))
{}

Sample::Sample(const TString& name, const Type type, const int color) :
  Sample(name, {}, type, color)
{}

bool Sample::operator==(const Sample& other) {
  return m_name == other.m_name;
}

std::set<TString> Sample::addUserSyst(const Systematic& sys, const SysConfig& conf) {
  std::set<TString> result;
  for(const auto& c : categories()) {
    if( !SystematicHandler::match(*(c.first), conf) )
      continue;
    // Change name of syst if some decorrelations apply
    Systematic syscopy(sys);
    syscopy.name += conf.getDecorrelationTag(c.first->properties(), *this);
    result.insert(syscopy.name);
    c.second->addSyst(std::move(syscopy));
  }
  return result;
}

void Sample::add32Syst(const Systematic32& sys32) {
  for(const auto& c : categories()) {
    Category* cat3J = c.first;
    if( !SystematicHandler::match(*(cat3J), sys32.conf) )
      continue;
    // then look at 3J only
    if( !(cat3J->match({Property::nJet, 3})) )
      continue;
    // ok, now we have a 3J category that matches
    SampleInCategory& sic3J = *(c.second);
    // now find the corresponding 2J
    Category* cat2J = CategoryIndex::find(cat3J->properties().copyExcept(Property::nJet, 2));
    if (cat2J == nullptr) {
      std::cout << "Sample::add32Syst::ERROR: corresponding 2J category not found. Cannot apply 3/2-jet ratio!" << std::endl;
      exit(-1);
    }
    SampleInCategory& sic2J = *(m_categories.at(cat2J));
    TString sysname(sys32.name);
    sysname += sys32.conf.getDecorrelationTag(cat3J->properties(), *this);
    // default 3/2 ratio is to keep 3+2 constant in every bin. So use default weights of applyRatioSys
    SystematicHandler::applyRatioSys(sysname, sys32.size, sic3J, sic2J);
  }
}

void Sample::finalizeNominal() {
  //std::cout << "Sample::finalizeNominal()" << std::endl;
  //std::cout << "Sample " << name() << " is in following categories:" << std::endl;
  //for(const auto& c : categories()) {
  //std::cout << c.first->name() << " at " << c.first << std::endl;
  //}
}

