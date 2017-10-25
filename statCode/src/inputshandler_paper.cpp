#include "inputshandler_paper.hpp"

#include <sstream>
#include <map>
#include <string>
#include <iostream>
#include <memory>

#include <TFile.h>
#include <TDirectory.h>
#include <TList.h>
#include <TCollection.h>
#include <TClass.h>
#include <TKey.h>
#include <TH1.h>
#include <TString.h>

#include "sample.hpp"
#include "configuration.hpp"
#include "inputshandler.hpp"
#include "properties.hpp"

InputsHandlerPaper::InputsHandlerPaper(Configuration& conf, const PropertiesSet& pset):
  InputsHandler()
{
  TString fname = InputsHandlerPaper::forgeFileName(conf, pset);
  m_file = std::unique_ptr<TFile>(TFile::Open(fname));
  findBaseDir();
}

InputsHandlerPaper::~InputsHandlerPaper() {
  if(m_file != nullptr)
    m_file->Close();
}

TString InputsHandlerPaper::forgeFileName(Configuration& conf, const PropertiesSet& pset) {
  // inputs/10002/8TeV_TwoLepton_MVA_2tag3xjet_vpt0_120_mva.root
  // 8TeV_TwoLepton_MVA_topemucr_vpt120_mva.root
  bool isMVA = pset.getIntProp(Property::type);
  std::map<int, std::string> cms { {2011, "7TeV_"}, {2012, "8TeV_"} };
  std::map<int, std::string> typeTag { {0, "CUT_"}, {1, "MVA_"} };
  std::map<int, std::string> nLepName { {0, "ZeroLepton_"}, {1, "OneLepton_"}, {2, "TwoLepton_"} };
  std::map<int, std::string> lepFlavTag { {0, "Mu_"}, {1, "El_"} };
  std::map<int, std::map<int, std::string> > binBounds {
    { 0, { {0, "0_90"}, {1, "90_120"}, {2, "120_160"}, {3, "160_200"}, {4, "200"}, {5, "100_120"} } } ,
    { 1, { {0, "0_120"}, {1, "90_120"}, {2, "120"} } }
  };

  std::stringstream ss;
  ss << "inputs/";
  ss << conf.getValue("InputVersion", "10000") << "/";
  ss << cms[pset[Property::year]];
  ss << nLepName[pset[Property::nLep]];
  if( pset.hasProperty(Property::lepFlav) ) {
    ss << lepFlavTag[pset[Property::lepFlav]];
  }
  ss << typeTag[isMVA];
  // special case of topemucr and topcr
  if( pset.hasProperty(Property::spec) && pset(Property::spec).Contains("top") ) {
    ss << pset(Property::spec);
  }
  else {
    ss << pset[Property::nTag] << pset(Property::tagType) << "tag";
    if (pset.hasProperty(Property::highVhf)) {
      if (pset[Property::highVhf]) ss << "highHF";
      else ss << "lowHF";
    }
    ss << pset[Property::nJet] << "jet";
  }
  if( pset.hasProperty(Property::spec) && pset(Property::spec) == "WbbCR" ) {
    ss << "WbbCR";
  }
  ss<< "_vpt";
  //If 1tag cut base we mimic MVA MV1c regions.
  if (isMVA==false &&  pset[Property::nTag] == 1){
    ss << binBounds[1][pset[Property::bin]]; // We force to mimic MVA convention in cut based
  }
  else {
    ss << binBounds[isMVA][pset[Property::bin]];
  }
  ss << "_" << pset(Property::dist) << ".root";

  std::cout << "filename = " << ss.str() << std::endl;

  return ss.str().c_str();
}

void InputsHandlerPaper::findBaseDir() {
  m_baseDir = m_file.get();
}

bool InputsHandlerPaper::checkData() {
  TH1* data = nullptr;
  m_baseDir->GetObject("data", data);
  return data != nullptr;
}

std::vector<TString> InputsHandlerPaper::listSamples() {
  std::vector<TString> res;
  TList* keys = m_baseDir->GetListOfKeys();
  TIter iter(keys);
  while(TKey* key = (TKey*)(iter())) {
    if(TClass::GetClass(key->GetClassName()) -> InheritsFrom(TH1::Class()))
      res.push_back(key->GetName());
  }

  return res;
}

TH1* InputsHandlerPaper::getHist(const Sample& sample, const TString& systname) {

  TH1* res = nullptr;
  if(! sample.hasSubsamples())
    res = getHistByName(sample.name(), systname);
  else {
    for(const auto& subs : sample.subsamples()) {
      TH1* tmp = getHistByName(subs, systname);
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
    std::cout << "WARNING::InputsHandlerPaper::getHist:" << std::endl
	      << "Histo for sample " << sample.name() << " in systematic " << systname << std::endl
	      << "does not exist in file " << m_file->GetName() << std::endl;
  }

  return res;
}

