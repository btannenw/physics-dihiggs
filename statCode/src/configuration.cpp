#include "configuration.hpp"

#include <TEnv.h>
#include <iostream>
#include <TString.h>

#include "utility.hpp"
#include "containerhelpers.hpp"

bool Configuration::m_debug = false;
AnalysisType Configuration::m_analysis = AnalysisType::None;

const std::unordered_map<TString, AnalysisType> AnalysesTypes::names({
    {"None", AnalysisType::None}, {"VHbb", AnalysisType::VHbb}, {"AZh", AnalysisType::AZh}, {"VHbbRun2", AnalysisType::VHbbRun2},
    {"HVT", AnalysisType::HVT}, {"MonoH", AnalysisType::MonoH}, {"Htautau", AnalysisType::Htautau}, 
    {"HH", AnalysisType::HH}, {"VBFGamma", AnalysisType::VBFGamma},
    {"HHWWbb", AnalysisType::HHWWbb}
    });

Configuration::Configuration(const TString& fname, const TString& version) :
  m_env(std::make_shared<TEnv>(fname.Data())),
  m_version(version)
{
  m_debug = m_env->GetValue("Debug", false);
  TString analysis_name = m_env->GetValue("Analysis", "None");
  // Spyros: if Analysis contains trailing spaces accidentally the map throws out_of_range
  // Protect against this case
  analysis_name.ReplaceAll(" ", "");
  m_analysis = AnalysesTypes::names.at(analysis_name);
}

std::vector<int> Configuration::getIntList(const char* name) {
  std::vector<TString> string_res = Utils::splitString(getValue(name, ""), ',');
  std::vector<int> res;
  if(! string_res.empty()) {
    for(const auto& s : string_res)
      res.push_back(s.Atoi());
  }
  return res;
}

std::vector<int> Configuration::getIntList(const char* name, const int dflt) {
  std::vector<int> res = getIntList(name);
  if(res.empty())
    res.push_back(dflt);
  return res;
}

std::vector<double> Configuration::getDoubleList(const char* name) {
  std::vector<TString> string_res = Utils::splitString(getValue(name, ""), ',');
  std::vector<double> res;
  if(! string_res.empty()) {
    for(const auto& s : string_res)
      res.push_back(s.Atof());
  }
  return res;
}

std::vector<double> Configuration::getDoubleList(const char* name, const double dflt) {
  std::vector<double> res = getDoubleList(name);
  if(res.empty())
    res.push_back(dflt);
  return res;
}

std::vector<TString> Configuration::getStringList(const char* name) {
  return Utils::splitString(getValue(name, ""), ',');
}

std::vector<TString> Configuration::getRegions() {
  return Utils::splitString(getValue("Regions", ""), ' ');
}
