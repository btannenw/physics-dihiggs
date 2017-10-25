#include "systematic.hpp"

#include <iostream>

#include <TString.h>

#include "sample.hpp"
#include "properties.hpp"
#include <TH1.h>

bool Systematic::operator<(const Systematic& other) const {
  SysOrdering ordering;
  return ordering(std::make_pair(type, name), std::make_pair(other.type, other.name));
}

bool Systematic::operator==(const Systematic& other) {
  return((other.type == type) && (other.name.EqualTo(name)));
}

std::pair<TString, Systematic::Side> Systematic::interpretSysName(const TString& name) {
  Side side = Side::symmetrize;
  TString shortname(name);
  if(name.EndsWith("__1up")) {
    side = Side::up;
    // special case: JetEResol
    if(name.Contains("JER") ||
       name.Contains("TTbar_RadLo") || // BBT, Sept 7 2016
       name.Contains("TTbar_RadHi") || // BBT, Sept 7 2016
       name.Contains("TTbar_PS") || // BBT, Sept 7 2016
       name.Contains("TTbar_ME") || // BBT, Sept 7 2016
       name.Contains("SoftCalo_Reso")||
       name.Contains("ResoPerp") ||
       name.Contains("ResoPara") ||
       name.Contains("VjetsMVH") ||
       name.Contains("MVH_ME") ||
       name.Contains("MVH_radiation"))   {
      side = Side::symmetrize;
    }
    shortname = name(0, name.Length()-5);
  }
  else if(name.EndsWith("__1down")) {
    side = Side::down;
    shortname = name(0, name.Length()-7);
  } 
  // Hacks for now
  else if(name.Contains("SoftCalo_Reso")) {
    side = Side::symmetrize;
  }
  else if(name.EndsWith("_ScaleUp")) {
    side = Side::up;
    shortname = name(0, name.Length()-2);
  }
  else if(name.EndsWith("_ScaleDown")) {
    side = Side::down;
    shortname = name(0, name.Length()-4);
  }
  else if(name.Contains("ResoPerp") ||
          name.Contains("ResoPara")) {
    side = Side::symmetrize;
  }
  else if(name.Contains("MJ_El_METstr") ||
          name.Contains("MJ_Mu_METstr") ||
          name.Contains("MJ_El_EWK")    ||
          name.Contains("MJ_El_flavor")) {
    side = Side::symmetrize;
  }

    
  return std::make_pair(shortname, side);
}

void Systematic::merge(const Systematic& other) {
  if(other.type != type || other.name != name) {
    std::cerr << "ERROR:  Systematic::merge" << std::endl;
    std::cerr << "Attempt to merge systematic " << name
      << " " << static_cast<int>(type) << std::endl;
    std::cerr << "With an incompatible " << other.name
      << " " << static_cast<int>(other.type) << std::endl;
    throw;
  }
  if(other.var_up != 1) {
    if(var_up != 1) {
      std::cerr << "WARNING:  Systematic::merge" << std::endl;
      std::cerr << "Merge of systematic " << name
        << " " << static_cast<int>(type)
        << " will overwrite var_up = " << var_up
        << " with var_up = " << other.var_up << std::endl;
    }
    var_up = other.var_up;
  }
  if(other.var_do != 1) {
    if(var_do != 1) {
      std::cerr << "WARNING:  Systematic::merge" << std::endl;
      std::cerr << "Merge of systematic " << name
        << " " << static_cast<int>(type)
        << " will overwrite var_do = " << var_do
        << " with var_do = " << other.var_do << std::endl;
    }
    var_do = other.var_do;
  }
  if(other.shape_up != nullptr) {
    if(shape_up != nullptr) {
      std::cerr << "WARNING:  Systematic::merge" << std::endl;
      std::cerr << "Merge of systematic " << name
        << " " << static_cast<int>(type)
        << " will overwrite shape_up = " << shape_up
        << " with shape_up = " << other.shape_up << std::endl;
    }
    shape_up = other.shape_up;
    shape_up_unsmoothed = other.shape_up_unsmoothed;
  }
  if(other.shape_do != nullptr) {
    if(shape_do != nullptr) {
      std::cerr << "WARNING:  Systematic::merge" << std::endl;
      std::cerr << "Merge of systematic " << name
        << " " << static_cast<int>(type)
        << " will overwrite shape_do = " << shape_do
        << " with shape_do = " << other.shape_do << std::endl;
    }
    shape_do = other.shape_do;
    shape_do_unsmoothed = other.shape_do_unsmoothed;
  }
  if(other.init != 0) {
    if(init != 0) {
      std::cerr << "WARNING:  Systematic::merge" << std::endl;
      std::cerr << "Merge of systematic " << name
        << " " << static_cast<int>(type)
        << " will overwrite init = " << init
        << " with init = " << other.init << std::endl;
    }
    init = other.init;
  }
  isConst = other.isConst;
  isSmoothed = other.isSmoothed;
}

void Systematic::print() const {
  std::cout << "Print systematic:" << std::endl
    << "  - name : " << name << std::endl
    << "  - type : " << static_cast<int>(type) << std::endl;
  if(type == Type::norm || type == Type::flt) {
    std::cout << "  - var_up : " << var_up << std::endl
      << "  - var_do : " << var_do << std::endl;
  }
  if(type == Type::flt) {
    std::cout << "  - init : " << init << std::endl;
    std::cout << "  - isConst : " << isConst << std::endl;
  }
  if(type == Type::shape) {
    std::cout << "  - has shape_up : " << (shape_up != nullptr) << std::endl
      << "  - has shape_do : " << (shape_do != nullptr) << std::endl;
  }
  std::cout << "  - isSmoothed : " << isSmoothed << std::endl;
}



// those are mostly useful for histo systs

SysConfig::SysConfig(Treat t, Smooth sm, std::vector<TString>&& s,
                     std::vector<PropertiesSet>&& c, std::vector<Property>&& d,
                     std::vector<PropertiesSet>&& cd, std::vector<TString>&& sd,
                     std::function<TString(const PropertiesSet&, const Sample&)>&& fun) :
  treat(t), doSmooth(sm), sampleNames(std::move(s)), categories(std::move(c)),
  decorrelations(std::move(d)), cplxDecorrelations(std::move(cd)),
  sampleDecorrelations(std::move(sd)), freeDecorrelation(std::move(fun))
{}

SysConfig::SysConfig(Treat t, Smooth sm, const TString& s, std::vector<PropertiesSet>&& c,
                     std::vector<Property>&& d, std::vector<PropertiesSet>&& cd, std::vector<TString>&& sd,
                     std::function<TString(const PropertiesSet&, const Sample&)>&& fun) :
  treat(t), doSmooth(sm), sampleNames(), categories(std::move(c)),
  decorrelations(std::move(d)), cplxDecorrelations(std::move(cd)),
  sampleDecorrelations(std::move(sd)), freeDecorrelation(std::move(fun))

{
  if( ! s.IsNull() ){
    sampleNames.push_back(s);
  }
}

// shorthand to decorrelate years
SysConfig::SysConfig(Treat t, Smooth sm, bool corr_years, std::vector<TString>&& s,
                     std::vector<PropertiesSet>&& c, std::vector<TString>&& sd,
                     std::function<TString(const PropertiesSet&, const Sample&)>&& fun) :
  treat(t), doSmooth(sm), sampleNames(std::move(s)), categories(std::move(c)),
  decorrelations(), cplxDecorrelations(), sampleDecorrelations(std::move(sd)),
  freeDecorrelation(std::move(fun))

{
  if(!corr_years)
    decorrelations.push_back(Property::year);
}

//SysConfig::SysConfig(Treat t, Smooth sm, bool corr_years, const TString& s,
                     //std::vector<PropertiesSet>&& c, std::vector<TString>&& sd,
                     //std::function<TString(const PropertiesSet&)>&& fun) :
  //treat(t), doSmooth(sm), sampleNames({s}), categories(std::move(c)),
  //decorrelations(), cplxDecorrelations(), sampleDecorrelations(std::move(sd)),
  //freeDecorrelation(std::move(fun))

//{
  //if(!corr_years)
    //decorrelations.push_back(Property::year);
//}


// those are mostly useful for user-defined systs

SysConfig::SysConfig(std::vector<TString>&& s, std::vector<PropertiesSet>&& c,
                     std::vector<Property>&& d, std::vector<PropertiesSet>&& cd, std::vector<TString>&& sd,
                     std::function<TString(const PropertiesSet&, const Sample&)>&& fun) :
  treat(Treat::skip), doSmooth(Smooth::noSmooth), sampleNames(std::move(s)), categories(std::move(c)),
  decorrelations(std::move(d)), cplxDecorrelations(std::move(cd)),
  sampleDecorrelations(std::move(sd)), freeDecorrelation(std::move(fun))

{}

SysConfig::SysConfig(const TString& s, std::vector<PropertiesSet>&& c,
                     std::vector<Property>&& d, std::vector<PropertiesSet>&& cd, std::vector<TString>&& sd,
                     std::function<TString(const PropertiesSet&, const Sample&)>&& fun) :
  treat(Treat::skip), doSmooth(Smooth::noSmooth), sampleNames({s}), categories(std::move(c)),
  decorrelations(std::move(d)), cplxDecorrelations(std::move(cd)),
  sampleDecorrelations(std::move(sd)), freeDecorrelation(std::move(fun))

{}


TString SysConfig::getDecorrelationTag(const PropertiesSet& pset, const Sample& s) const {
  TString res("");
  // simple decorrelation rules, if defined
  for(const auto& p : decorrelations) {
    if(pset.hasProperty(p)) {
      res += pset.getPropertyTag(p);
    }
  }
  // more complex decorrelation rules, if defined
  for(const auto& psetDecorr : cplxDecorrelations) {
    if(pset.match(psetDecorr)) {
      res += psetDecorr.getPropertiesTag();
    }
  }
  // free decorrelation rules, if defined
  if(freeDecorrelation != nullptr) {
    res += freeDecorrelation(pset, s);
  }
  for(const auto& sd : sampleDecorrelations) {
    if(s.hasKW(sd)) {
      res += "_" + sd;
    }
  }
  return res;
}

