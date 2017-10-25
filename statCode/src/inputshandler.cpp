#include "inputshandler.hpp"

#include <TDirectory.h>
#include <TFile.h>
#include <TList.h>
#include <TCollection.h>
#include <TClass.h>
#include <TKey.h>
#include <TH1.h>
#include <TString.h>

#include <set>
#include <iostream>

class Configuration;
struct PropertiesSet;


InputsHandler::InputsHandler() :
  m_file(nullptr), m_baseDir(nullptr)
{}

InputsHandler::InputsHandler(Configuration& conf, const PropertiesSet& pset) :
  m_file(nullptr), m_baseDir(nullptr)
{}

InputsHandler::~InputsHandler() {}

bool InputsHandler::exists() {
  if(m_file == nullptr || m_baseDir == nullptr)
    return false;
  return checkData();
}

std::vector<TString> InputsHandler::listSystematics() {
  std::vector<TString> res;
  TList* keys = m_baseDir->GetListOfKeys();
  TIter iter(keys);
  while(TKey* key = (TKey*)(iter())) {
    if(TClass::GetClass(key->GetClassName()) -> InheritsFrom(TDirectory::Class())) {
      TString keyname(key->GetName());
      if (keyname == "Systematics") {
        TDirectory* sysdir = (TDirectory*)key->ReadObj();
        TList* syskeys = sysdir->GetListOfKeys();
        TIter sysiter(syskeys);
        while(TKey* syskey = (TKey*)(sysiter())) {
          TString name(syskey->GetName());
          TString nameSys(name);
          if (name.Index("_Sys") > 0) {
            nameSys.Remove(0,name.Index("_Sys")+1);
            res.push_back(nameSys);
          }
        }
      } else if (keyname.BeginsWith("Sys")) {
        res.push_back(keyname);
      }
    }
  }

  // Uniquify the systematics list
  std::set<TString> resset(res.begin(), res.end());
  std::vector<TString> resuniq(resset.begin(), resset.end());


  return resuniq;
}

TH1* InputsHandler::getHistByName(const TString& name, const TString& systname, const TString& dirname) {
  TH1* res = nullptr;
  if(systname == "") {
    m_baseDir->GetObject(name, res);
  }  else {
    TString dir = dirname.Length()==0 ? systname : dirname;
    TString hname = dirname.Length()==0 ? name : name + "_" + systname;
    m_baseDir->GetObject(dirname+"/"+hname, res);
  }
  return res;
}

