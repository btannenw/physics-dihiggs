#include "systematiclistsbuilder.hpp"

#include <utility>
#include <type_traits>

#include <TString.h>

#include "properties.hpp"
#include "systematic.hpp"

#include <iostream>

class Configuration;

SystematicListsBuilder::SystematicListsBuilder(const Configuration& conf,
                                               std::vector<std::pair<Systematic, SysConfig>>& userSysts,
                                               std::vector<std::pair<Systematic, SysConfig>>& pois,
                                               std::vector<Systematic32>& r32ratios,
                                               std::unordered_map<TString, SysConfig>& histoSysts,
                                               std::unordered_map<TString, TString>& renameHistoSysts,
                                               std::vector<TString>& constNormFacts) :
  m_config(conf), m_userSysts(userSysts), m_pois(pois), m_32ratios(r32ratios),
  m_histoSysts(histoSysts), m_renameHistoSysts(renameHistoSysts),
  m_constNormFacts(constNormFacts)
{}

// most generic one

void SystematicListsBuilder::applySys(Systematic&& sys, SysConfig&& conf) {
  m_userSysts.emplace_back(std::make_pair(std::move(sys), std::move(conf)));
}

// basic ones

void SystematicListsBuilder::normSys(const TString& name, float min, float max, SysConfig&& conf) {
  m_userSysts.emplace_back(std::make_pair(Systematic(name, min, max), std::move(conf)));
}

void SystematicListsBuilder::normSys(const TString& name, float size, SysConfig&& conf) {
  m_userSysts.emplace_back(std::make_pair(Systematic(name, 1-size, 1+size), std::move(conf)));
  std::cout<<"normSys^^^: "<<name<<std::endl;
}

void SystematicListsBuilder::normFact(const TString& name, SysConfig&& conf, float init, float down, float up, bool isConst) {
  TString sysname = "ATLAS_norm_"+name;
  m_userSysts.emplace_back(std::make_pair(Systematic(sysname, init, down, up, isConst), std::move(conf)));
  if(isConst) {
    m_constNormFacts.push_back(sysname);
  }
}

void SystematicListsBuilder::addPOI(const TString& name, SysConfig&& conf, float init, float down, float up) {
  m_pois.emplace_back(std::make_pair(Systematic(name, init, down, up, false), std::move(conf)));
}

void SystematicListsBuilder::ratio32(const TString& name, float size, SysConfig&& conf) {
  m_32ratios.emplace_back(Systematic32{name, size, conf});
}

void SystematicListsBuilder::sampleNormSys(const TString& name, float size) {
  normSys("Sys"+name+"Norm", size, {name});
}

void SystematicListsBuilder::sampleNorm3JSys(const TString& name, float size) {
  normSys("Sys"+name+"Norm", size, {name, {{Property::nJet, 3}}, {Property::nJet}});
}

void SystematicListsBuilder::sampleNormSysByRegion(const TString& sysName, const TString& sampleName,
                                              float loPt2Jet, float hiPt2Jet, float loPt3Jet, float hiPt3Jet) {
  using P = Property;
  // normSys(sysName, loPt2Jet, {sampleName, {{{{P::nJet, 2}, {P::bin, 0}}, {}},
  //                                          {{{P::nJet, 2}, {P::bin, 1}}, {}}}});
  // normSys(sysName, hiPt2Jet, {sampleName, {{{{P::nJet, 2}, {P::bin, 2}}, {}},
  //                                          {{{P::nJet, 2}, {P::bin, 3}}, {}},
  //                                          {{{P::nJet, 2}, {P::bin, 4}}, {}}}});
  // normSys(sysName, loPt3Jet, {sampleName, {{{{P::nJet, 3}, {P::bin, 0}}, {}},
  //                                          {{{P::nJet, 3}, {P::bin, 1}}, {}}}});
  // normSys(sysName, hiPt3Jet, {sampleName, {{{{P::nJet, 3}, {P::bin, 2}}, {}},
  //                                          {{{P::nJet, 3}, {P::bin, 3}}, {}},
  //                                          {{{P::nJet, 3}, {P::bin, 4}}, {}}}});
  normSys(sysName, loPt2Jet, {sampleName, {{{{P::nJet, 2}, {P::bin, 0}}, {{}}},
                                           {{{P::nJet, 2}, {P::bin, 1}}, {{}}}}});
  normSys(sysName, hiPt2Jet, {sampleName, {{{{P::nJet, 2}, {P::bin, 2}}, {{}}},
                                           {{{P::nJet, 2}, {P::bin, 3}}, {{}}},
                                           {{{P::nJet, 2}, {P::bin, 4}}, {{}}}}});
  normSys(sysName, loPt3Jet, {sampleName, {{{{P::nJet, 3}, {P::bin, 0}}, {{}}},
                                           {{{P::nJet, 3}, {P::bin, 1}}, {{}}}}});
  normSys(sysName, hiPt3Jet, {sampleName, {{{{P::nJet, 3}, {P::bin, 2}}, {{}}},
                                           {{{P::nJet, 3}, {P::bin, 3}}, {{}}},
                                           {{{P::nJet, 3}, {P::bin, 4}}, {{}}}}});
}

