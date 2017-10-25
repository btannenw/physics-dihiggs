#include "properties.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <utility>

#include <TString.h>

#include "containerhelpers.hpp"

const std::unordered_map<Property, TString> Properties::names({
  {Property::year, "Y"}, {Property::nLep, "L"}, {Property::nJet, "J"}, {Property::nFatJet, "Fat"}, {Property::nTag, "T"},
    {Property::tagType, "TType"}, {Property::bin, "B"}, {Property::lepFlav, "Flv"}, {Property::lepSign, "Sgn"},
    {Property::type, "isMVA"}, {Property::dist, "dist"}, {Property::spec, "Spc"}, {Property::highVhf, "Vhf"},
    {Property::binMin, "BMin"}, {Property::binMax, "BMax"}, {Property::descr, "D"}, {Property::incTag, "incTag"},
    {Property::incJet, "incJet"}, {Property::incFat, "incFat"}, {Property::incAddTag, "incAddTag"},
    {Property::nAddTag, "nAddTag"}, {Property::nPh, "Ph"}//, {Property::hiLoMass, "hiLoMass"}
});

const std::unordered_map<TString, Property> Properties::props_from_names(Utils::reverseMap<Property, TString>(Properties::names));

bool PropertiesSet::hasProperty(const Property p) const {
  if(intProperties.find(p) == intProperties.end() && stringProperties.find(p) == stringProperties.end())
    return false;
  return true;
}

int PropertiesSet::getIntProp(const Property p) const {
  const auto& pos = intProperties.find(p);
  if(pos != intProperties.end())
    return pos->second;
  return -1;
}

TString PropertiesSet::getStringProp(const Property p) const {
  const auto& pos = stringProperties.find(p);
  if(pos != stringProperties.end())
    return pos->second;
  return "";
}

void PropertiesSet::setIntProp(Property p, int i) {
  intProperties[p] = i;
}

void PropertiesSet::setStringProp(Property p, const TString& s) {
  stringProperties[p] = s;
}

bool PropertiesSet::match(const PropertiesSet& pset) const {
  // check if all defined intProps are present and have same value
  bool res = std::all_of(std::begin(pset.intProperties), std::end(pset.intProperties),
                         [this](const std::pair<Property, int>& p) {
                           return this->hasProperty(p.first) &&
                             (*this)[p.first] == p.second;
                         });

  if( !res )
    return false;

  // check if all defined stringProps are present and have same value
  res = std::all_of(std::begin(pset.stringProperties), std::end(pset.stringProperties),
                    [this](const std::pair<Property, TString>& p) {
                      return this->hasProperty(p.first) &&
                        (*this)(p.first).EqualTo(p.second);
                    });
  return res;
}

void PropertiesSet::merge(const PropertiesSet& other) {
  for(const auto& p : other.intProperties)
    intProperties[p.first] = p.second;
  for(const auto& p : other.stringProperties)
    stringProperties[p.first] = p.second;
}

void PropertiesSet::print() const {
  std::cout << std::endl << "Printing properties:" << std::endl;
  std::cout << "  * int values:" << std::endl;
  for(const auto& p : intProperties)
    std::cout << "    - " << Properties::names.at(p.first) << " = " <<  p.second << std::endl;
  std::cout << "  * string values:" << std::endl;
  for(const auto& p : stringProperties)
    std::cout << "    - " << Properties::names.at(p.first) << " = " <<  p.second << std::endl;
  std::cout << std::endl;
}

TString PropertiesSet::getPropertyTag(const Property p) const {
  std::stringstream ss;
  ss << "_";
  const auto& pos = intProperties.find(p);
  if(pos != intProperties.end()) {
    ss << Properties::names.at(pos->first) << pos->second;
  }
  else {
    const auto& pos2 = stringProperties.find(p);
    if(pos2 != stringProperties.end()) {
      TString tmp = pos2->second;
      tmp.ReplaceAll("_", "");
      ss << Properties::names.at(pos2->first) << tmp;
    }
    else {
      std::cout << "ERROR:   PropertiesSet::printProperty" << std::endl;
      std::cout << "Property " << static_cast<int>(p) << " does not exist here" << std::endl;
      throw;
    }
  }
  return ss.str().c_str();
}

TString PropertiesSet::getPropertiesTag() const {
  std::stringstream ss;
  for(const auto& p : intProperties) {
    ss << "_" << Properties::names.at(p.first) << p.second;
  }
  for(const auto& p : stringProperties) {
    TString tmp = p.second;
    tmp.ReplaceAll("_", "");
    ss << "_" << Properties::names.at(p.first) << tmp;
  }
  return ss.str().c_str();
}

PropertiesSet PropertiesSet::copyExcept(const Property p, int i) const {
  PropertiesSet pset(*this);
  pset[p] = i;
  return pset;
}

PropertiesSet PropertiesSet::copyExcept(const Property p, const TString& s) const {
  PropertiesSet pset(*this);
  pset(p) = s;
  return pset;
}


