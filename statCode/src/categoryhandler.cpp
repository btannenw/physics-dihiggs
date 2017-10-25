#include "categoryhandler.hpp"

#include <iostream>

#include <TString.h>

#include "properties.hpp"
#include "outputhandler.hpp"
#include "finder.hpp"
#include "regionnamesparser.hpp"
#include "analysishandler.hpp"
#include "analysis.hpp"
#include "category.hpp"
#include "configuration.hpp"
#include "plotting.hpp"
#include "regiontracker.hpp"

using namespace std;

CategoryHandler::CategoryHandler(const Configuration& conf):
  m_config(conf),
  m_namesParser(AnalysisHandler::analysis().regionNamesParser()),
  m_categories(),
  m_regTrk(AnalysisHandler::analysis().regionTracker()),
  m_plotting(conf),
  m_xmlFiles()
{
  populate();
}

CategoryHandler::~CategoryHandler()
{}


void CategoryHandler::populate() {
  vector<TString> regNames = m_config.getRegions();
  for(auto& r : regNames) {
    addCategory(r);
  }
  std::cout << std::endl << "We have the following categories:" << std::endl;
  for(auto& c : m_categories) {
    std::cout << c.name() << std::endl;
    CategoryIndex::feed(&c);
  }
}

void CategoryHandler::addCategory(const TString& regName) {
  pair<bool, PropertiesSet> res = m_namesParser->parseRegion(regName);
  if(! res.first)
    return; // bad formatting of the name of the region
  PropertiesSet& pset = res.second;
  Category cat(m_config, std::move(pset));
  if(! cat.exists())
    // TODO add logging to keep track of which ones are discarded
    return;
  m_categories.emplace_back(std::move(cat));
}

void CategoryHandler::finalizeNominal() {
  bool defltFitConfig = m_config.getValue("DefaultFitConfig", false);
  for(Category& cat : m_categories) {
    m_regTrk->trackCategory(cat);
    cat.finalizeNominal();
  }
  if(defltFitConfig) {
    m_regTrk->setDefaultFitConfig();
  }
}

void CategoryHandler::makeControlPlots()
{
  m_plotting.prepare();
  for(auto& c : m_categories) {
    m_plotting.makeCategoryPlots(c);
  }
  m_plotting.finish();
}

void CategoryHandler::makeSystStatusPlots() const {
  for(const auto& c : m_categories) {
    m_plotting.makeSystStatusPlots(c);
  }
  m_plotting.makeOverallSystStatusPlots(*this);
}

void CategoryHandler::makeSystShapePlots() const {
  for(const auto& c : m_categories) {
    makeSystShapePlots(c);
  }
}

void CategoryHandler::makeSystShapePlots(const Category& cat) const {
  m_plotting.makeSystShapePlots(cat);
}

void CategoryHandler::writeNormAndXMLForCat(Category& cat) {
  m_xmlFiles.emplace_back(cat.writeNormAndXML());
}

std::pair<TString, TString> CategoryHandler::writeNormAndXML(const std::vector<TString>& constNormFacts,
                                                             const std::set<TString>& poiNames) {
  OutputConfig* outconfig = OutputConfig::getInstance();
  if(m_xmlFiles.empty()) {
    for(auto& c : m_categories) {
      writeNormAndXMLForCat(c);
    }
  }

  // now write the main driver file

  TString driverName = outconfig->xmlDir+"/driver.xml";
  TString wsname = outconfig->xmlDir+"/output_combined_VH_model.root";

  std::ofstream driver(driverName);
  driver << "<!DOCTYPE Combination  SYSTEM 'HistFactorySchema.dtd'>\n\n"
    << "<Combination OutputFilePrefix=\"" << outconfig->xmlDir << "/output\">\n\n";
  for(const auto& xmlfile : m_xmlFiles) {
    driver << "<Input>" << xmlfile << "</Input>\n";
  }
  driver << std::endl;
  driver << "<Measurement Name=\"VH\" Lumi=\"1\" LumiRelErr=\"0.0001\" ExportOnly=\"True\">\n";
  driver << "<POI>";
  for(const auto& name : poiNames) {
    driver << name << " ";
  }
  driver << "</POI>\n";
  // now floating norm params that must be kept constant
  driver << "<ParamSetting Const=\"True\">Lumi ";
  for(const auto& n : constNormFacts) {
    driver << n << " ";
  }
  driver << "</ParamSetting>\n";

  driver << "</Measurement>\n"
    << "</Combination>\n";

  driver.close();

  return std::make_pair(driverName, wsname);
}

