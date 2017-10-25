#include "systematichandler.hpp"

#include <utility>
#include <iterator>
#include <algorithm>
#include <iostream>
#include <functional>

#include <TH1.h>
#include <TString.h>

#include "systematic.hpp"
#include "sample.hpp"
#include "category.hpp"
#include "sampleincategory.hpp"
#include "analysishandler.hpp"
#include "analysis.hpp"
#include "systematiclistsbuilder.hpp"
#include "properties.hpp"

class Configuration;
class RegionTracker;

SystematicHandler::SystematicHandler(const Configuration& conf) :
  m_config(conf), m_userSysts(), m_pois(), m_32ratios(), m_histoSysts(),
  m_renameHistoSysts(), m_constNormFacts(),
  m_systListsBuilder(AnalysisHandler::analysis().systListsBuilder(conf, m_userSysts, m_pois,
                                                                  m_32ratios, m_histoSysts,
                                                                  m_renameHistoSysts, m_constNormFacts))
{}

void SystematicHandler::listAllUserSystematics(const RegionTracker& regTrk, bool useFltNorms) {
  m_systListsBuilder->listAllUserSystematics(regTrk, useFltNorms);
}


void SystematicHandler::listAllHistoSystematics(const RegionTracker& regTrk) {
  m_systListsBuilder->listAllHistoSystematics(regTrk);
  m_systListsBuilder->fillHistoSystsRenaming();
}



bool SystematicHandler::match(const SampleInCategory& s, const SysConfig& sys) {
  return (match(s.sample(), sys) && match(s.category(), sys));
}

bool SystematicHandler::match(const Sample& s, const SysConfig& sys) {
  if(sys.sampleNames.empty())
    return true;
  return std::any_of(std::begin(sys.sampleNames), std::end(sys.sampleNames),
                     [&s](const TString& name){ return s.hasKW(name); } );
}

bool SystematicHandler::match(const Category& c, const SysConfig& sys) {
  if(sys.categories.empty())
    return true;
  return std::any_of(std::begin(sys.categories), std::end(sys.categories),
                     [&c](const PropertiesSet& pset){ return c.match(pset); } );
}


std::pair<TString, const SysConfig&> SystematicHandler::getHistoSysConfig(const TString& name) const {
  TString newName(name);
  const auto rename = m_renameHistoSysts.find(name);
  if(rename != m_renameHistoSysts.end()) {
    // TODO add logging in that case, to report to analyses to change their names
    newName = rename->second;
  }
  if(!m_histoSysts.count(newName)) {
    std::cout << "ERROR:  SystematicHandler::getHistoSysConfig" << std::endl;
    std::cout << "No configuration defined for systematic |" << name << "|\t Looking for newName: |"<<newName<<"|"<<std::endl;
    std::cout << "Please define a configuration for it !" << std::endl;
    throw;
  }
  std::cout << "Configuration defined for systematic |" << name << "|\t Looking for newName: |"<<newName<<"|"<<std::endl;

  return std::make_pair(newName, std::cref(m_histoSysts.at(newName)));
}

bool SystematicHandler::hasHistoSysConfig(const TString& name) const {
  return ((m_histoSysts.count(name) + m_renameHistoSysts.count(name)) > 0);
}

void SystematicHandler::applyRatioSys(const TString& name, float size, SampleInCategory& sic1,
                                      SampleInCategory& sic2, float weight1, float weight2) {
  // simple case is to preserve the sum of the two categories
  if(weight1 <= 0) {
    weight1 = sic1.getNomHist()->Integral();
  }
  if(weight2 <= 0) {
    weight2 = sic2.getNomHist()->Integral();
  }
  // well-defined case
  if(weight2 > 0) {
    float r = weight1/weight2;
    float err1 = size / (1+r+r*size);
    float err2 =  -r * err1;
    sic1.addSyst(Systematic(name, 1-err1, 1+err1));
    sic2.addSyst(Systematic(name, 1-err2, 1+err2));
  }
  // then just put a syst on the first
  else {
    sic1.addSyst(Systematic(name, 1-size, 1+size));
  }
}

void SystematicHandler::decorrSysForCategories(const TString& name, const std::vector<Properties::Property>& props, bool replace) {
  // user systs
  for(auto hsys = m_userSysts.begin(); hsys != m_userSysts.end(); hsys++) {
    std::cout << "#" << hsys->first.name.Data() << ";" << std::endl;
    if (hsys->first.name == name) {
      std::cout << "INFO: Found and decorrelating: " << name << std::endl;
      if (replace) {
        hsys->second.decorrelations.clear();
        hsys->second.cplxDecorrelations.clear();
        hsys->second.decorrelations = props;
      } else {
        for (auto& prop : props)
          hsys->second.decorrelations.push_back(prop);
      }
    }
  }
  // histo systs
  for(auto hsys = m_histoSysts.begin(); hsys != m_histoSysts.end(); hsys++) {
    std::cout << "#" << hsys->first.Data() << ";" << std::endl;
    if (hsys->first == name) {
      std::cout << "INFO: Found and decorrelating: " << name << std::endl;
      if (replace) {
        hsys->second.decorrelations.clear();
        hsys->second.cplxDecorrelations.clear();
        hsys->second.decorrelations = props;
      } else {
        for (auto& prop : props)
          hsys->second.decorrelations.push_back(prop);
      }
    }
  }
}

void SystematicHandler::decorrSysForAllCategories(const TString& name) {
  decorrSysForCategories(name, {Property::nLep, Property::nJet, Property::nTag, Property::tagType, Property::bin});
}
