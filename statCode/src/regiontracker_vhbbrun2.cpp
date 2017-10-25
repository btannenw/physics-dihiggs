#include "regiontracker_vhbbrun2.hpp"

#include <string>
#include <unordered_map>

#include "properties.hpp"
#include "category.hpp"

void RegionTracker_VHbbRun2::addInfo(int nlep, int njet, int ntag, bool topcr, bool mergedCat, bool mBBcr, const TString& spec) {
  if(nlep == 2) { m_bits["hasTwoLep"] = true; }
  if(nlep == 1) { m_bits["hasOneLep"] = true; }
  if(nlep == 0) { m_bits["hasZeroLep"] = true; }
  if(njet == 2) { m_bits["has2jet"] = true; }
  if(njet == 3) { m_bits["has3jet"] = true; }
  if(ntag == 0) { m_bits["has0tag"] = true; }
  if(ntag == 1) { m_bits["has1tag"] = true; }
  if(ntag == 2) { m_bits["has2tag"] = true; }
}

void RegionTracker_VHbbRun2::trackCategory(const Category& c) {
  int nlep = c.requestIntProp(Property::nLep);
  int njet = c.requestIntProp(Property::nJet);
  int ntag = c.requestIntProp(Property::nTag);
  int nFatJet = 0; nFatJet = c[Property::nFatJet];
  int incFat = 0; incFat = c.properties()[Property::incFat];
  bool mergedCat = false;
  if ( nFatJet > 0 || incFat > 0 ) { mergedCat = true; }
  TString descr = c.requestStringProp(Property::descr);
  TString spec = c.getStringProp(Property::spec);
  addInfo(nlep, njet, ntag, descr.Contains("top"), mergedCat, descr.Contains("mBBcr"), spec);
}

void RegionTracker_VHbbRun2::setDefaultFitConfig() {
  addInfo(0, 2, 2, false, false, false);
  addInfo(1, 3, 2, false, false, false);
  addInfo(2, 3, 2, false, false, false);
}
