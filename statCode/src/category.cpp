#include "category.hpp"

#include <iostream>
#include <utility>
#include <set>
#include <memory>
#include <cmath>
#include <algorithm>
#include <unordered_map>

#include <TH1.h>
#include <TAxis.h>
#include <TDirectory.h>
#include <TString.h>

#include "sample.hpp"
#include "binning.hpp"
#include "systematic.hpp"
#include "systematichandler.hpp"
#include "finder.hpp"
#include "analysishandler.hpp"
#include "analysis.hpp"
#include "configuration.hpp"
#include "inputshandler.hpp"
#include "outputhandler.hpp"
#include "properties.hpp"
#include "sampleincategory.hpp"

Category::Category(const Configuration& conf, PropertiesSet&& pset) :
  m_config(conf),
  m_properties(std::move(pset)),
  m_input(AnalysisHandler::analysis().inputsHandler(m_properties)),
  m_out(),
  m_hsob(nullptr),
  m_binHist(nullptr)
{
  if(Configuration::debug()) {
    std::cout << "INFO::Category: Building new Category with given properties tag:" << std::endl;
    std::cout << name() << std::endl << std::endl;
  }
}

int Category::getIntProp(const Property p) const {
  return m_properties.getIntProp(p);
}

TString Category::getStringProp(const Property p) const {
  return m_properties.getStringProp(p);
}

int Category::requestIntProp(const Property p) const {
  return m_properties.requestIntProp(p);
}

TString Category::requestStringProp(const Property p) const {
  return m_properties.requestStringProp(p);
}

void Category::setIntProp(Property p, int i) {
  m_properties.setIntProp(p, i);
}

void Category::setStringProp(Property p, const TString& s) {
  m_properties.setStringProp(p, s);
}

TString Category::name() const {
  TString res = "Region";
  res += m_properties.getPropertiesTag();
  return res;
}

bool Category::exists() {
  return m_input->exists();
}

void Category::tryAddSample(Sample& s) {
  TH1* hist = m_input->getHist(s, "");
  if(hist == nullptr) {
    // TODO add some logging
    return;
  }
  if (TString(hist->GetName()).Contains("correlation_histogram")) return;
  TH2* corr = (TH2*)m_input->getHist(Sample(Form("%s_correlation_histogram", hist->GetName()), Sample::Type::None, 0), "");
  if(Configuration::analysisType() == AnalysisType::VHbb) {
    // FIXME
    // hack for bug in CorrsAndSysts. 6% scaling applied on Wl and Wcl is wrong.
    if( s.name() == "Wcl" || s.name() == "Wl") {
      if( getIntProp(Property::bin) < 2 && getIntProp(Property::nJet) == 3 ) {
        hist->Scale(0.94);
      }
    }
  }

  double err;
  float sum = hist->IntegralAndError(1, hist->GetNbinsX(), err);

  if(s.type() != SType::Data && sum < 1.e-9) {
    // TODO add some proper logging
    std::cout << "WARNING::Nominal rate very small: " << s.name() << std::endl;

    std::cout << "Ignoring this sample in this channel" << std::endl;
    return;
  }

  if(s.type() != SType::Data && fabs(err) > 1.01 * fabs(sum)) {
    // TODO add some proper logging
    std::cout << "WARNING::Nominal rate compatible with 0 within MC stat error: " << s.name() << std::endl;
    //BBT, Aug 16 2016: remove stat unc check to allow QCD to survive (currently ON)
    std::cout << "Ignoring this sample in this channel" << std::endl;
    return;
  }

  // there is an histogram for this sample, so just use it
  // SampleInCategory sic(*this, s, hist);
  SampleInCategory sic(*this, s, hist, corr);
  auto sicpair = samples().emplace(&s, std::move(sic));
  s.categories().emplace(this, &(sicpair.first->second)); // add pointer to the SiC in Sample s
}

bool Category::match(const PropertiesSet& pset) const {
  return m_properties.match(pset);
}

bool Category::isControlRegion() const {
  if( m_properties[Property::nTag] < 2)
    return true;
  if( m_properties(Property::spec) == "topcr" || m_properties(Property::spec) == "topemucr")
    return true;
  return false;
}

bool Category::hasSensitiveBinAbove(float thresh) const {
  // compare max of S/B histo with thresh
  float maxSoB = getSoBHist()->GetBinContent(getSoBHist()->GetMaximumBin());
  return (maxSoB > thresh);
}

TH1* Category::getSigHist() const {
  return getHist(SType::Sig);
}

TH1* Category::getBkgHist() const {
  TH1* hbkg = getHist(SType::Bkg);
  TH1* hdd = getHist(SType::DataDriven);
  if(hbkg != nullptr && hdd != nullptr) {
    hbkg->Add(hdd);
    delete hdd;
    return hbkg;
  }
  if(hbkg == nullptr)
    return hdd;
  if(hdd == nullptr)
    return hbkg;
  return nullptr;
}

TH1* Category::getDataHist() const {
  return getHist(SType::Data);
}

TH1* Category::getSoBHist() const {
  if(m_hsob == nullptr) {
    TH1* hsig = getSigHist();
    TH1* hbkg = getBkgHist();
    if(hbkg == nullptr) {
      std::cout << "ERROR::Category::getSoBHist: Background is null !" << std::endl
        << "  That should not happen ! Will now exit. Please investigate the reason !"
        << std::endl;
    }
    if(hsig == nullptr) {
      hbkg->Reset();
      const_cast<Category*>(this)->m_hsob = hbkg; // returns histo with 0 everywhere
    }
    else {
      hsig->Divide(hbkg);
      delete hbkg;
      const_cast<Category*>(this)->m_hsob = hsig;
    }
  }

  return m_hsob;
}

TH1* Category::getHist(SType type) const {
  TH1* hist = nullptr;
  if(! SampleIndex::has(type) ) { return hist; }
  const std::set<Sample*>& samples = SampleIndex::findAll(type);
  for(const auto& s : samples) {
    const auto& pos = m_samples.find(s);
    if(pos == m_samples.end()) { // this sample is not present for this category
      continue;
    }
    const SampleInCategory& sic = pos->second;
    if(hist == nullptr) {
      hist = (TH1*)(sic.getNomHist()->Clone());
    }
    else {
      hist->Add( sic.getNomHist() );
    }
  }
  return hist;
}

void Category::finalizeNominal() {
  if(Configuration::debug()) {
    std::cout << "INFO::Category: In category " << name() << std::endl << std::endl;
  }

  BinningTool& binning = BinningTool::getInstance();
  // adapt range
  for(auto& sample : m_samples) {
    binning.changeRange(sample.second.getNomHist(), *this);
  }

  // then compute binning
  m_binning = binning.getCategoryBinning(*this);
  setBinningHist();

  // then apply it
  for(auto& sample : m_samples) {
    binning.applyBinning(sample.second.getNomHist(), m_binning, sample.second.getCorrHist());
    binning.changeRangeAfter(sample.second.getNomHist(), *this);
    sample.second.computeBinningForSysts();
  }
}

void Category::setBinningHist() {
  // only 1 number: using a simple Rebin - no need for this histo
  if(m_binning.size() <= 1) {
    return;
  }

  // default is to rebin right to left so need to reverse order
  // first we need to make a copy
  std::vector<int> binning = m_binning;
  // check first if necessary
  if( binning.size() > 1 && ( binning.at(0) > binning.at(1) ) ) {
    std::reverse(binning.begin(),binning.end());
  }
  // fill a vector of doubles containing the *low-edge* of each bin
  // should have n+1 elements to get a n bin histogram
  TH1* data = getDataHist();
  int nbin(data->GetNbinsX());
  std::cout << "nbin " << nbin << std::endl;
  std::vector<Double_t> bins;
  for(auto& b: binning) {
    if( b == 0 ) { 
      bins.push_back( data->GetXaxis()->GetBinLowEdge(1) );
    } else if( b > nbin ) {
      bins.push_back( data->GetXaxis()->GetBinLowEdge(b-1) );
    } else {
      bins.push_back( data->GetXaxis()->GetBinLowEdge(b) );
    }
  }
  Double_t* binarray = bins.data(); // make array from vector
  m_binHist = data->Rebin( m_binning.size()-1, TString("bins_"+this->name()), binarray );
  for(int i=0; i<m_binHist->GetNbinsX()+2; i++) {
    float width = m_binHist->GetBinWidth(i);
    if(width == 0) { continue; }
    m_binHist->SetBinContent( i, m_binHist->GetBinContent(i) / width );
    m_binHist->SetBinError( i, m_binHist->GetBinError(i) / width );
    // now need to do m_binHist->Integral("width") to get the correct integral
  }
//  delete data;
//  delete binarray;
}

void Category::writeBinHist(TDirectory* dir) { 
  if(m_binHist == nullptr) { return; }
  dir->cd();
  m_binHist->Write();
}

void Category::applyHistoSysts(const SystematicHandler& systs, bool skipUnknown) {
  std::vector<TString> systnames = m_input->listSystematics();
  // loop on the names of the TDirectories
  for(const auto& name : systnames) {
    //std::cout << "Now working on syst: " << name << std::endl;
    auto splitSysName = Systematic::interpretSysName(name);
    // check if some config is defined for that one
    if( skipUnknown && !systs.hasHistoSysConfig(splitSysName.first)) {
      continue;
    }
    // getHistoSysConfig returns the configuration, and also the correct syst name
    // if there is a rule to correct for splitSysName.first (e.g typo in some input file)
    auto nameconfig = systs.getHistoSysConfig(splitSysName.first);
    // check if not skip, and if this category matches the requirements
    if((nameconfig.second.treat != STreat::skip) && SystematicHandler::match(*this, nameconfig.second)) {
      applyHistoSysts(nameconfig.first, splitSysName.second, name, nameconfig.second);
    }
  }

  // then apply the 'smoothing' of MV1c distributions. Can be done only now that all individual
  // shapes have been added side after side
  if(getStringProp(Property::dist).Contains("MV1c")) {
    for(auto& pairsample : m_samples) {
      SampleInCategory& sic = pairsample.second;
      sic.removeSameSignBinsSysts();
    }
  }
}

void Category::applyHistoSysts(const TString& name, const Systematic::Side side,
                               const TString& dirname, const SysConfig& conf) {
  BinningTool& binning = BinningTool::getInstance();

  // loop on samples for that specific syst
  for(auto& pairsample : m_samples) {
    Sample& sample = *(pairsample.first);
    SampleInCategory& sic = pairsample.second;
    // check that the sample matches the requirements
    if( sample.type() == SType::Data ) {
      continue;
    }
    if( !SystematicHandler::match(sample, conf)) {
      continue;
    }
    // try to get the shifted hist
    TH1* hsys = m_input->getHist(sample, dirname);
    // if not exist, skip
    if(hsys == nullptr)
      continue;

    if(Configuration::analysisType() == AnalysisType::VHbb) {
      // FIXME
      //hack for WDPhi systematic in 3 jet low pTV
      // Note: it would be safer for this hack to be in inputshandler_paper
      // But I checked that syst histograms are accessed only from this codepath.
      // for UP, we must scale down 6%, for DOWN we must scale up 6%
      // for Wl and Wcl, it is 3%

      if( getIntProp(Property::bin) < 2 && getIntProp(Property::nJet) == 3 ) {
        // first scale nominal
        if( sample.name() == "Wcl" || sample.name() == "Wl") {
          hsys->Scale(0.94);
        }

        // then scale systematics
        if( name.Contains("WDPhi") ) {
          float scale = 0.06;
          if(side == Systematic::Side::up) {
            scale = - scale;
          }
          if( sample.name() == "Wl" || sample.name() == "Wcl") {
            scale /= 2.;
          }
          hsys->Scale(1+scale);
        }
      }
    }

    std::shared_ptr<TH1> hsys_unsmoothed;
    if (m_config.getValue("SmoothingControlPlots", false)) {
       hsys_unsmoothed.reset( (TH1*) hsys->Clone( (hsys->GetName() + std::string("_unsmoothed")).c_str() ) );
       hsys_unsmoothed->SetDirectory(0);
    }

    // then rebin the histo for this category
    binning.changeRange(hsys, *this);
    // FIXME: JWH - should pass in correlation hist?
    binning.applyBinning(hsys, m_binning);
    binning.changeRangeAfter(hsys, *this);
    if (hsys_unsmoothed) {
       binning.changeRange(hsys_unsmoothed.get(), *this);
       // FIXME: JWH - should pass in correlation hist?
       binning.applyBinning(hsys_unsmoothed.get(), m_binning);
       binning.changeRangeAfter(hsys_unsmoothed.get(), *this);
    }
      
    // smooth histo if requested
    if(conf.doSmooth != SysConfig::Smooth::noSmooth &&
      ! getStringProp(Property::dist).Contains("MV1c")) { // do not smooth MV1c distributions !
      //  TODO should the logic be here or elsewhere ?
      if(Configuration::debug()) {
        std::cout << "INFO::Category::applyHistoSysts::Smoothing of " << name << std::endl;
        std::cout << "In " << sic.name() << std::endl << std::endl;
      }
      sic.smoothHisto(hsys, conf.doSmooth);
    }

    // We have a clean histo. Now the SiC knows how to translate it into proper systematics
    TString finalSysName = name + conf.getDecorrelationTag(m_properties, sample);
    sic.addSyst(finalSysName, side, conf.treat, hsys, (conf.doSmooth != SysConfig::Smooth::noSmooth), hsys_unsmoothed);
    // std::cout << "before addSyst" << std::endl;
    // std::cout << m_config.getValue("SmoothingControlPlots", false) << std::endl;
    // std::cout << finalSysName << std::endl;
    // sic.addSyst(finalSysName, side, conf.treat, hsys, m_config.getValue("SmoothingControlPlots", false), hsys_unsmoothed);
    // std::cout << "after addSyst" << std::endl;
  }
}

TString Category::writeNormAndXML() {
  std::cout << "INFO::Category::writeNormAndXML:" << std::endl;
  std::cout << "Will now write Norm and XML files for category: " << name() << std::endl;
  std::cout << "..." << name() << std::endl;
  m_out = OutputHandler(*this);
  bool useStatSysts = m_config.getValue("UseStatSystematics", true);
  float statThreshold = m_config.getValue("StatThreshold", 0.05);
  // DTD is: data, then staterrorconfig, then other samples
  const std::set<Sample*>& vdata = SampleIndex::findAll(SType::Data);
  for(auto& d : vdata) {
    m_samples.at(d).writeNormAndXML(m_out, useStatSysts);
  }

  if(useStatSysts) {
    m_out.addMCStatThresh(statThreshold);
  }

  for(auto& sicpair : m_samples) {
    if( !(sicpair.first->type() == SType::Data) ) {
      sicpair.second.writeNormAndXML(m_out, useStatSysts);
    }
  }
  std::cout << "Files written." << name() << std::endl;
  return m_out.closeFiles();
}


// Complex pruning schemes
// This one is based on studies by Yao.
// See its presentation, HSG5 meeting 25/09/2013
// Remove systs for samples that are small wrt signal and small wrt other backgrounds.

void Category::pruneSmallSysts_Yao() {
  TH1* bkg(getBkgHist());
  TH1* sig(getSigHist());
  float threshold = 0.02;
  bool isSensitiveRegion = hasSensitiveBinAbove(threshold);
  std::vector<int> sensitiveBins;
  if(isSensitiveRegion) {
    for(int i=1; i<sig->GetNbinsX()+1; ++i) {
      if(sig->GetBinContent(i) > threshold * bkg->GetBinContent(i)) {
        sensitiveBins.push_back(i);
      }
    }
  }

  for(auto & couple : m_samples) {
    couple.second.pruneSmallSysts_Yao(sig, bkg, isSensitiveRegion, sensitiveBins);
  }

  delete bkg;
  delete sig;
}

void Category::pruneSmallShapeSysts_Htautau() {
  TH1* bkg(getBkgHist());
  for(auto & couple : m_samples) {
    couple.second.pruneSmallShapeSysts_Htautau(bkg);
  }
  delete bkg;
}

