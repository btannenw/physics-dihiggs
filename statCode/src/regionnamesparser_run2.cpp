#include "regionnamesparser_run2.hpp"

#include "TObjArray.h"
#include "utility.hpp"
#include <TPRegexp.h>
#include <TString.h>

#include "properties.hpp"

#include <iostream>
#include <cstdlib>

RegionNamesParser_Run2::RegionNamesParser_Run2() :
  m_regionRegexp("^(\\d+)TeV_(\\w+)_(\\w+)tag(\\w+)jet_(\\d+)(_\\d+)?ptv(_\\w+)?_(\\w+)(\\s*)$"),
  m_tagRegexp("(\\d+)([lmtx]{2})?(p)?"),
  m_jetRegexp("((\\d+)(p)?fat)?(\\d+)(p)?"),
  m_descrRegexp("(_([a-zA-Z0-9]+))+")
{}

std::pair<bool, PropertiesSet> RegionNamesParser_Run2::parseRegion(const TString& regConfig) {
  PropertiesSet pset;
  // First parsing of the string
  TObjArray* res = m_regionRegexp.MatchS(regConfig);
  if(res->GetEntriesFast() == 0) { // no match
    std::cout << "ERROR (run2): ParseRegions: region badly formatted " << regConfig << std::endl;
    throw;
  }

  TString cme = Utils::group(res, 1);
  if(cme == "13") {
    pset[Property::year] = 2015;
  }
  else if(cme == "8") {
    pset[Property::year] = 2012;
  }
  else {
    pset[Property::year] = 2011;
  }


  TString channel = Utils::group(res, 2);
  // our config does make use of this category, so now get all its properties
  if(channel.Contains("Zero")) pset[Property::nLep] = 0;
  if(channel.Contains("One")) pset[Property::nLep] = 1;
  if(channel.Contains("Two")) pset[Property::nLep] = 2;
  if(channel.Contains("TauLH")) {
    pset(Property::spec) = "TauLH";
    pset[Property::nLep] = 1;
  }
  if(channel.Contains("TauHH")) {
    pset(Property::spec) = "TauHH";
    pset[Property::nLep] = 0;
  }
  if(channel.Contains("MET")) pset(Property::spec) = "MET";
  if(channel.Contains("_Mu")) pset[Property::lepFlav] = 0;
  if(channel.Contains("_El")) pset[Property::lepFlav] = 1;
  if(channel.Contains("_MVA")) pset[Property::type] = 1;
  if(channel.Contains("_CUT")) pset[Property::type] = 0;


  TString sr_OR_cr = Utils::group(res, 6);

  if(regConfig.Contains("_SR_")){
    pset(Property::spec) = "SR";
    std::cout<<"^^^^^^ SR! channel: "<<sr_OR_cr<<std::endl;
  }
  else{
    pset(Property::spec) = "mBBcr";
    std::cout<<"^^^^^^ mBBcr! regConfig<<: "<<regConfig
	     <<"\t reg(1): "<<Utils::group(res, 1)
	     <<"\t reg(2): "<<Utils::group(res, 2)
	     <<"\t reg(3): "<<Utils::group(res, 3)
	     <<"\t reg(4): "<<Utils::group(res, 4)
	     <<"\t reg(5): "<<Utils::group(res, 5)
	     <<"\t reg(6): "<<Utils::group(res, 6)
	     <<std::endl;
  }
  //if(channel.Contains("SR")) pset[Property::SR] = 1;
  //if(channel.Contains("mBBcr")) pset[Property::SR] = 0;

  /*  if(regConfig.Contains("_Xhh700_")){
    pset(Property::hiLoMass) = "loMass";
    std::cout<<"!!!!! loMass: "<<regConfig<<std::endl;
  }
  else{
    pset(Property::hiLoMass) = "hiMass";
    std::cout<<"!!!!! hiMass: "<<regConfig<<std::endl;
  }*/


  TObjArray* res_tag = m_tagRegexp.MatchS(Utils::group(res, 3));
  if(res_tag->GetEntriesFast() == 0) { // no match
    std::cout << "ERROR: ParseRegions: tag block badly formatted " << Utils::group(res, 3) << std::endl;
    throw;
  }
  pset[Property::nTag] = atoi(Utils::group(res_tag, 1));
  TString tagType = Utils::group(res_tag, 2);
  if(tagType.Length()) { pset(Property::tagType) = tagType; }
  TString incTag = Utils::group(res_tag, 3);
  if(incTag.Length()) { pset[Property::incTag] = 1; }

  TObjArray* res_jet = m_jetRegexp.MatchS(Utils::group(res, 4));
  if(res_jet->GetEntriesFast() == 0) { // no match
    std::cout << "ERROR: ParseRegions: jet block badly formatted " << Utils::group(res, 4) << std::endl;
    throw;
  }
  pset[Property::nJet] = atoi(Utils::group(res_jet, 4));
  TString incJet = Utils::group(res_jet, 5);
  if(incJet.Length()) { pset[Property::incJet] = 1; }
  TString nFatJet = Utils::group(res_jet, 1);
  if(nFatJet.Length()) { pset[Property::nFatJet] = atoi(nFatJet); }
  TString incFat = Utils::group(res_jet, 2);
  if(incFat.Length()) { pset[Property::incFat] = 1; }

  pset[Property::binMin] = atoi(Utils::group(res, 5));
  TString ptvmax = Utils::group(res, 6);
  if(ptvmax.Length()) { pset[Property::binMax] = atoi(TString(ptvmax.Strip(TString::EStripType::kLeading, '_'))); }

  TString descr = Utils::group(res, 7);
  pset(Property::descr) = descr.Strip(TString::EStripType::kLeading, '_');

  pset(Property::dist) = Utils::group(res, 8);

  res->Delete();
  delete res;
  res_tag->Delete();
  delete res_tag;
  res_jet->Delete();
  delete res_jet;

  return {true, pset};
}
