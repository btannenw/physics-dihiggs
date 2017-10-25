#include "inputshandler_run2.hpp"

#include <sstream>
#include <map>
#include <string>
#include <iostream>
#include <memory>

#include <TFile.h>
#include <TString.h>
#include <TH1.h>

#include "sample.hpp"
#include "configuration.hpp"
#include "inputshandler.hpp"
#include "properties.hpp"

InputsHandlerRun2::InputsHandlerRun2(Configuration& conf, const PropertiesSet& pset):
  InputsHandlerPaper()
{
  TString fname = InputsHandlerRun2::forgeFileName(conf, pset);
  m_file = std::unique_ptr<TFile>(TFile::Open(fname));
  findBaseDir();
}

InputsHandlerRun2::~InputsHandlerRun2() {
  if(m_file != nullptr)
    m_file->Close();
}

TString InputsHandlerRun2::forgeFileName(Configuration& conf, const PropertiesSet& pset) {
  std::map<int, std::string> cms { {2011, "7TeV_"}, {2012, "8TeV_"}, {2015, "13TeV_"}, {4031, "13TeV_"} };
  std::map<int, std::string> typeTag { {0, "CUT_"}, {1, "MVA_"} };
  std::map<int, std::string> nLepName { {0, "ZeroLepton_"}, {1, "OneLepton_"}, {2, "TwoLepton_"} };
  std::map<int, std::string> lepFlavTag { {0, "Mu_"}, {1, "El_"} };

  using P = Property;

  std::stringstream ss;
  ss << "inputs/";
  ss << conf.getValue("InputVersion", "10000") << "/";

  // year
  ss << cms[pset[P::year]];

  // channel

  if( pset.hasProperty(Property::spec) && pset(Property::spec).Contains("Tau") ) {
    ss << pset(Property::spec) << "_";
  } else {
    ss << nLepName[pset[P::nLep]];
  }
  if( pset.hasProperty(P::lepFlav) ) {
    ss << lepFlavTag[pset[P::lepFlav]];
  }
  if( pset.hasProperty(P::type) ) {
    bool isMVA = pset.getIntProp(P::type);
    ss << typeTag[isMVA];
  }

  // tag
  ss << pset[P::nTag];
  if( pset.hasProperty(P::tagType) ) { ss << pset(P::tagType); }
  if( pset.hasProperty(P::incTag) ) { ss << "p"; }
  ss << pset(P::tagType) << "tag";

  // jet
  if( pset.hasProperty(P::nFatJet) ) {
    ss << pset[P::nFatJet];
    if( pset.hasProperty(P::incFat) ) { ss << "p"; }
    ss << "fat";
  };
  ss << pset[P::nJet];
  if( pset.hasProperty(P::incJet) ) { ss << "p"; }
  ss << "jet_";

  // ptv
  ss << pset[P::binMin];
  if( pset.hasProperty(P::binMax) ) { ss << "_" << pset[P::binMax]; }
  ss << "ptv_";

  // additional track jet
  if( pset.hasProperty(P::nAddTag) ) {
    ss << pset[P::nAddTag];
    if( pset.hasProperty(P::incAddTag) ) { ss << "p"; }
    ss << "btrkjetunmatched_";
  }

  // descr
  const TString& descr = pset(P::descr);
  if(descr.Length()) { ss << descr << "_"; }

  // dist
  ss << pset(P::dist) << ".root";

  std::cout << "filename = " << ss.str() << std::endl;

  return ss.str().c_str();
}


TH1* InputsHandlerRun2::getHist(const Sample& sample, const TString& systname) {

  TH1* res = nullptr;
  if(! sample.hasSubsamples())
    res = getHistByName(sample.name(), systname, "Systematics");
  else {
    for(const auto& subs : sample.subsamples()) {
      TH1* tmp = getHistByName(subs, systname, "Systematics");
      if(res == nullptr) { // case of first subsample
        if(tmp != nullptr) { // but ensure that the first subsample actually exists
          res = tmp;
          res->SetName(sample.name());
        }
      }
      else
        res->Add(tmp); // TODO add check for compatibility of binnings
    }
  }

  if(res == nullptr) {
    std::cout << "WARNING::InputsHandlerRun2::getHist:" << std::endl
	      << "Histo for sample " << sample.name() << " in systematic " << systname << std::endl
	      << "does not exist in file " << m_file->GetName() << std::endl;
  }

  return res;
}
