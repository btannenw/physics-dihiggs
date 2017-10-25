#include "sampleincategory.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <numeric>

#include <TH1.h>
#include <TString.h>

#include "sample.hpp"
#include "category.hpp"
#include "binning.hpp"
#include "outputhandler.hpp"
#include "configuration.hpp"
#include "utility.hpp"
#include "properties.hpp"
#include "systematic.hpp"

SampleInCategory::SampleInCategory(Category& cat, Sample& s, TH1* hist, TH2* smoothedCorrHist) :
  m_cat(cat), m_sample(s), m_nomHist(hist), m_systs(), m_systsStatus(), m_originalNomHist((TH1*) hist->Clone( (hist->GetName() + std::string("_orig")).c_str() )), m_smoothedNomCorrHist(smoothedCorrHist ? (TH2*) smoothedCorrHist->Clone() : nullptr)
{}

// merge constructor
SampleInCategory::SampleInCategory(Category& cat, Sample& s,
                                   const std::vector<SampleInCategory*>& sicToMerge) :
  m_cat(cat), m_sample(s), m_nomHist(nullptr), m_smoothedNomCorrHist(nullptr), m_systs(), m_systsStatus()
{
  // 1. sum the nominal histograms
  for(const auto p : sicToMerge) {
    if(m_nomHist == nullptr) {
      m_nomHist.reset((TH1*)(p->getNomHist()->Clone(m_sample.name())));
    }
    else {
      m_nomHist->Add(p->getNomHist());
    }
  }

  m_originalNomHist.reset((TH1*) m_nomHist->Clone( (m_nomHist->GetName() + std::string("_orig")).c_str() ));

  // 2. compute binning for systs (needed ?)
  computeBinningForSysts();

  // 3. systematics.
  for(auto sic_it = sicToMerge.begin(); sic_it != sicToMerge.end(); ++sic_it) {
    SampleInCategory& sic = *(*sic_it);
    for(SysIterator sys = sic.m_systs.begin(); sys != sic.m_systs.end(); ++sys) {
      const SysKey& key = sys->first;
      const TString& sysname = key.second;
      // case of floating norm: we have to check that all merged samples have it !
      if(key.first == SysType::flt) {
        if(std::any_of(sic_it+1, sicToMerge.end(),
                       [&key](const SampleInCategory* s) {
                       return !(s->hasSyst(key.first, key.second));
                       }))
        {
          std::cerr << "SampleInCategory::SampleInCategory: Not all merged SiC have a "
            << "NormFactor named " << sysname << std::endl;
          throw;
        }
        m_systs.emplace(key, sys->second);
        m_systsStatus.emplace(key, SysStatus::Used);
        for(auto sic2_it = sic_it+1; sic2_it != sicToMerge.end(); ++sic2_it) {
          (*sic2_it)->deleteSyst(key);
        }
      }
      else {
        // we cannot delete systs of sic on-the-fly. Well, we could, but that would
        // be troublesome. So, instead, just do a lookup to avoid double-putting a systematic.
        // The case that is then avoided is when a syst has both norm and shape components
        // in sic, where we would find it twice.
        if(hasSyst(SysType::norm, sysname) || hasSyst(SysType::shape, sysname)) {
          continue;
        }
        // compute the shifted up/do summed histograms for this syst
        TH1* hup = nullptr;
        TH1* hdo = nullptr;

        std::shared_ptr<TH1> hup_unsmoothed;
        std::shared_ptr<TH1> hdo_unsmoothed;
        const bool needToMergeUnsmoothed = std::any_of(sicToMerge.begin(), sicToMerge.end(),
            [&sysname](const SampleInCategory* s) {
              std::map<SysKey, Systematic, SysOrdering>::const_iterator foundItem =
                  s->m_systs.find(std::make_pair(SysType::shape, sysname));
              if (foundItem != s->m_systs.end()) {
                return foundItem->second.shape_do_unsmoothed || foundItem->second.shape_up_unsmoothed;
              }
              return false;
            }
        );

        for(auto psic2 : sicToMerge) {
          SampleInCategory& sic2 = *psic2;
          TH1* tmp = sic2.getFullSystShape(sysname, Systematic::Side::up);
          if(hup == nullptr) {
            hup = tmp;
            hup->SetName(m_nomHist->GetName());
          }
          else {
            hup->Add(tmp);
            delete tmp;
          }
          tmp = sic2.getFullSystShape(sysname, Systematic::Side::down);
          if(hdo == nullptr) {
            hdo = tmp;
            hdo->SetName(m_nomHist->GetName());
          }
          else {
            hdo->Add(tmp);
            delete tmp;
          }

          if (needToMergeUnsmoothed) {
            std::shared_ptr<TH1> src_up;
            std::shared_ptr<TH1> src_do;
            std::map<SysKey, Systematic, SysOrdering>::const_iterator foundSys =
                sic2.m_systs.find(std::make_pair(SysType::shape, sysname));
            if (foundSys != sic2.m_systs.end()) {
              //This sample does have a shape sys
              src_up = foundSys->second.shape_up_unsmoothed;
              src_do = foundSys->second.shape_do_unsmoothed;
            }

            //add, steal or clone a sys histo
            if (hup_unsmoothed) {
              hup_unsmoothed->Add(src_up ? src_up.get() : sic2.getNomHist());
            }
            else if (src_up) {
              hup_unsmoothed = src_up;
              hup_unsmoothed->SetName((m_nomHist->GetName() + std::string("_unsmoothed")).c_str());
            }
            else {
              TH1* nomClone = static_cast<TH1*>(sic2.getNomHist()->Clone((m_nomHist->GetName() + std::string("_up_unsmoothed")).c_str()));
              nomClone->SetDirectory(0);
              hup_unsmoothed.reset(nomClone);
            }

            if (hdo_unsmoothed) {
              hdo_unsmoothed->Add(src_do ? src_do.get() : sic2.getNomHist());
            }
            else if (src_do) {
              hdo_unsmoothed = src_do;
              hdo_unsmoothed->SetName((m_nomHist->GetName() + std::string("_unsmoothed")).c_str());
            }
            else {
              TH1* nomClone = static_cast<TH1*>(sic2.getNomHist()->Clone((m_nomHist->GetName() + std::string("_do_unsmoothed")).c_str()));
              nomClone->SetDirectory(0);
              hdo_unsmoothed.reset(nomClone);
            }
          }

          // delete this syst from the other SiC to avoid retrying the addition.
          if(&sic != &sic2) { // compare pointers
            sic2.deleteSyst(std::make_pair(SysType::norm, sysname));
            sic2.deleteSyst(std::make_pair(SysType::shape, sysname));
          }
        }
        // then create the appropriate Systematics
        bool isSmoothed = sys->second.isSmoothed;
        addSyst(sysname, Systematic::Side::up, STreat::shape, hup, isSmoothed, hup_unsmoothed);
        addSyst(sysname, Systematic::Side::down, STreat::shape, hdo, isSmoothed, hdo_unsmoothed);
      }
    }
  }

  // 4. Remove old nominal histograms.
  for(auto& sic : sicToMerge) {
    sic->m_nomHist.reset();
  }
}

SampleInCategory::~SampleInCategory() {

}

bool SampleInCategory::hasSyst(SysType type, const TString& name) const {
  return m_systs.count(std::make_pair(type, name));
}

Systematic& SampleInCategory::getSyst(SysType type, const TString& name) {
  return m_systs.at(std::make_pair(type, name));
}

void SampleInCategory::addSyst(const TString& sysname, Systematic::Side side, STreat treat, TH1* hsys,
                               bool isSmoothed, std::shared_ptr<TH1> hsys_unsmoothed) {
  // look at norm effect
  float varNorm = hsys->Integral();
  float nomNorm = m_nomHist->Integral();
  float var = varNorm/nomNorm;
  using P = Property;
  if (treat != STreat::shapeonly && (var < 0.01 || var > 2.01))
  {
    std::cerr << "ERROR::SampleInCategory::addSyst" << std::endl;
    std::cerr << "Variation too drastic: " << name() << " = " << var << std::endl;
    return; // FIXME it is return because of some low stat samples. Should make it more clever.
    //throw;
  }

  if(Configuration::analysisType() == AnalysisType::VHbbRun2) {                                  
     if((m_sample.name() == "ttbar" ) && m_cat.match({P::nLep, 1})) {                             
        if(sysname.Contains("TTbarMBB")) { 
           if(side == Systematic::Side::up) {                                                       
              side = Systematic::Side::down;                                                         
           } else { 
              side = Systematic::Side::up;                                                           
           } 
        } 
     } 
  }

  // we do care about norm
  if(treat == STreat::norm || treat == STreat::shape) {
    // check that norm is not null with a super small threshold
    // threshold for real pruning is defined later
    if(fabs(var - 1) > 0.001) {
      Systematic sys(SysType::norm, sysname);
      switch(side) {
        case Systematic::Side::up:
          sys.var_up = var;
        break;
        case Systematic::Side::down:
          sys.var_do = var;
        break;
        case Systematic::Side::symmetrize:
          sys.var_up = var;
          sys.var_do = 2 - var; // FIXME should it be 1/var ?
        break;
      }
      sys.isSmoothed = isSmoothed;
      addSyst(std::move(sys));
    }
  }

  // now look at shape effect
  if(treat == STreat::shapeonly || treat == STreat::shape) {
    hsys->Scale(1./var);
    if (hsys_unsmoothed) {
       hsys_unsmoothed->Scale(nomNorm/hsys_unsmoothed->Integral());
    }
    // check that shape is not null with a super small threshold
    // threshold for real pruning is defined later
    if(hasEnoughShape(hsys, 0.001)) {
      // ok. So now create a Systematic object and add it to the SampleInCategory
      Systematic sys(SysType::shape, sysname);
      switch(side) {
        case Systematic::Side::up:
          sys.shape_up.reset(hsys);
          sys.shape_up_unsmoothed = hsys_unsmoothed;
        break;
        case Systematic::Side::down:
          sys.shape_do.reset(hsys);
          sys.shape_do_unsmoothed = hsys_unsmoothed;
        break;
        case Systematic::Side::symmetrize:
          sys.shape_up.reset(hsys);
          sys.shape_up_unsmoothed = hsys_unsmoothed;
          sys.shape_do.reset( Utils::symmetrize(hsys, m_nomHist.get()) );
          sys.shape_do_unsmoothed.reset( hsys_unsmoothed ? Utils::symmetrize(hsys_unsmoothed.get(), m_nomHist.get()) : nullptr);
        break;
      }
      sys.isSmoothed = isSmoothed;
      //We take over memory management from root here
      hsys->SetDirectory(0);
      addSyst(std::move(sys));
    }
    else {
      delete hsys;
    }
  }
}

void SampleInCategory::addSyst(const Systematic& sys) {
  SysKey key = std::make_pair(sys.type, sys.name);
  const auto& pos = m_systs.find(key);
  if(pos == m_systs.end()) {
    m_systs.emplace(key, sys);
    const auto& posStatus = m_systsStatus.find(key);
    if(posStatus == m_systsStatus.end()) {
      m_systsStatus.emplace(key, SysStatus::Used);
    }
    else {
      posStatus->second = SysStatus::Used;
    }
  }
  else {
    pos->second.merge(sys);
  }
}

void SampleInCategory::addSyst(Systematic&& sys) {
  // TODO add some pretty logging
  SysKey key = std::make_pair(sys.type, sys.name);
  const auto& pos = m_systs.find(key);
  if(pos == m_systs.end()) {
    // TODO check definition of move to see if needed in that case
    m_systs.emplace(key, std::move(sys));
    const auto& posStatus = m_systsStatus.find(key);
    if(posStatus == m_systsStatus.end()) {
      m_systsStatus.emplace(key, SysStatus::Used);
    }
    else {
      posStatus->second = SysStatus::Used;
    }
  }
  else {
    pos->second.merge(sys);
  }
}

TH1* SampleInCategory::getFullSystShape(const TString& name, Systematic::Side side) {
  TH1* hist = nullptr;
  if(hasSyst(SysType::shape, name)) {
    switch(side) {
      case Systematic::Side::up:
        if(getSyst(SysType::shape, name).shape_up != nullptr) {
          hist = (TH1*)(getSyst(SysType::shape, name).shape_up->Clone());
        }
      break;
      case Systematic::Side::down:
        if(getSyst(SysType::shape, name).shape_do != nullptr) {
          hist = (TH1*)(getSyst(SysType::shape, name).shape_do->Clone());
        }
      break;
      default:
        throw;
    }
  }
  if(hist == nullptr) {
    hist = (TH1*)(m_nomHist->Clone());
  }

  if(hasSyst(SysType::norm, name)) {
    switch(side) {
      case Systematic::Side::up:
        hist->Scale(getSyst(SysType::norm, name).var_up);
      break;
      case Systematic::Side::down:
        hist->Scale(getSyst(SysType::norm, name).var_do);
      break;
      default:
        throw;
    }
  }

  return hist;
}

void SampleInCategory::computeBinningForSysts() {
  BinningTool& binning = BinningTool::getInstance();
  m_sysBins = binning.getBinningForSysts(m_nomHist.get());
  // getBinningForSysts gives rebinning in reverse order.
  // smoothing function expects bins from left to right.
  std::sort(m_sysBins.begin(), m_sysBins.end());
  if(Configuration::debug()) {
    std::cout << "INFO::SampleInCategory::computeBinningForSysts: Binning for SiC " << name() << ":"
      << std::endl;
    for(int b : m_sysBins) {
      std::cout << b << " ";
    }
    std::cout << std::endl;
  }
}

void SampleInCategory::scaleHistoSys(TH1* hsys, float factor) {
  BinningTool& binning = BinningTool::getInstance();
  binning.scaleHistoSys(m_nomHist.get(), hsys, factor);
}

void SampleInCategory::smoothHisto(TH1* hsys, SysConfig::Smooth sm) {
  BinningTool& binning = BinningTool::getInstance();
  // old behaviour
  //binning->smoothHisto(m_nomHist, hsys, m_sysBins, false);

  // new algorithm
  std::vector<int> bins;
  // allow parabolic shape for Mjj
  if(m_cat.requestStringProp(Property::dist) == "mjj" || m_cat.requestStringProp(Property::dist) == "mVH") {
    bins = binning.getLocalExtremaBinning(m_nomHist.get(), hsys, 1);
  }
  // only monotonic cases for the other variables
  else {
    bins = binning.getLocalExtremaBinning(m_nomHist.get(), hsys, 0);
  }
  binning.smoothHisto(m_nomHist.get(), hsys, bins, false);
}

void SampleInCategory::deleteSyst(const SysKey& key, SysStatus reason) {
  auto it = m_systs.find(key);
  if(it != m_systs.end()) {
     //The blessing of shared_ptr makes this unnecessary:
//    Systematic& sys = it->second;
//    if(sys.shape_up != nullptr) {
//      delete sys.shape_up;
//    }
//    if(sys.shape_do != nullptr) {
//      delete sys.shape_do;
//    }
//    if (sys.shape_up_unsmoothed) {
//       delete sys.shape_up_unsmoothed;
//    }
//    if (sys.shape_do_unsmoothed) {
//       delete sys.shape_do_unsmoothed;
//    }
    m_systs.erase(it);
    m_systsStatus.at(key) = reason;
  }
}

void SampleInCategory::removeSysts(std::function<bool(const Systematic&)>& pred, SysStatus reason) {
  for(auto it = m_systs.begin(); it != m_systs.end(); ) {
    Systematic& sys = it->second;
    if(pred(sys)) {
//      if(sys.shape_up != nullptr) {
//        delete sys.shape_up;
//        sys.shape_up = nullptr;
//      }
//      if(sys.shape_do != nullptr) {
//        delete sys.shape_do;
//        sys.shape_do = nullptr;
//      }
      m_systsStatus.at(it->first) = reason;
      it = m_systs.erase(it);
    }
    else {
      ++it;
    }
  }
}

void SampleInCategory::pruneSmallShapeSysts(float thresh) {
  for(auto& sysobj : m_systs) {
    if(sysobj.first.first != SysType::shape) {
      continue;
    }
    Systematic& sys = sysobj.second;
    deleteIfNoShape(sys.shape_up, thresh);
    deleteIfNoShape(sys.shape_do, thresh);
  }

  // collect and remove shape systs that don't have any surviving histogram
  std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
    if (sys.type == SysType::shape)
      std::cout<<"%%%% pruneSmallShapeSysts: sys.name = "<<sys.name<<", sys.var_up = "<<sys.var_up<<", sys.var_down = "<<sys.var_do<<std::endl; // BBT Sept 13 2016

    return ((sys.type == SysType::shape) &&
            ((sys.shape_up == nullptr) && (sys.shape_do == nullptr)));};
  removeSysts(f, SysStatus::PruneSmall);
  //if (f) std::cout<<"Removed sys (in pruneSmallShapeSysts): "<<sys.name<<std::endl;
}

// keep it that way (a very small function that calls another one
// to allow easier future developments with different algorithms
bool SampleInCategory::deleteIfNoShape(std::shared_ptr<TH1>& h, float thresh) {
  if(h && !hasEnoughShape(h.get(), thresh) ) {
    h.reset();
    return true;
  }
  return false;
}

bool SampleInCategory::hasEnoughShape(TH1* h, float thresh) {
  int nrBins = h->GetNbinsX();
  for (int ibin=1;ibin<nrBins+1;ibin++)
  {
    float varBin = h->GetBinContent(ibin);
    float nomBin = m_nomHist->GetBinContent(ibin);
    if (varBin == 0 || nomBin == 0) continue;
    if ( (nomBin <= pow(10,-9) && varBin > 10*pow(10,-9)) || (fabs(varBin/nomBin-1) > thresh))
    {
      return true;
    }
  }
  return false;

}

void SampleInCategory::pruneOneSideShapeSysts() {
  std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
    return ((sys.type == SysType::shape) &&
            ((sys.shape_up == nullptr) || (sys.shape_do == nullptr))); };
  removeSysts(f, SysStatus::PruneOneSide);
}

void SampleInCategory::pruneSmallNormSysts(float thresh) {
  std::function<bool(const Systematic&)> f = [thresh](const Systematic& sys) {
    if (sys.type == SysType::norm)
      std::cout<<"%%%% pruneSmallNormSysts: sys.name = "<<sys.name<<", sys.var_up = "<<sys.var_up<<", sys.var_down = "<<sys.var_do<<std::endl; // BBT Sept 13 2016
    return ((sys.type == SysType::norm) &&
            (!sys.name.Contains("LUMI")) &&
            ((std::fabs(sys.var_up-1) < thresh) || (std::fabs(sys.var_do-1) < thresh))); };
  
  removeSysts(f, SysStatus::PruneSmall);
  //if (f) std::cout<<"Removed sys (in pruneSmallNormSysts): "<<sys.name<<std::endl;

}

void SampleInCategory::pruneSameSignSysts() {
  //std::cout<<"çççç% pruneSameSignSysts 1"<<std::endl; // BBT Sept 7 2016
 
  std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
    //std::cout<<"%%%% pruneSameSignSysts: sys.name = "<<sys.name<<", sys.var_up = "<<sys.var_up<<", sys.var_down = "<<sys.var_do<<std::endl; // BBT Sept 7 2016
    return ((sys.type == SysType::norm) &&
            ((sys.var_up-1) * (sys.var_do-1) >= 0 )); };
  //std::cout<<"çççç% pruneSameSignSysts 2"<<std::endl; // BBT Sept 7 2016
  removeSysts(f, SysStatus::PruneOneSide);
  //std::cout<<"çççç% pruneSameSignSysts 3"<<std::endl; // BBT Sept 7 2016
}

void SampleInCategory::pruneSpecialTests() {

// remove JetEResol in 2 lepton 1tag 3jet low pTV

  using P = Property;

  // The following things are specific to VHbb Run1 analysis
  if(Configuration::analysisType() == AnalysisType::VHbb) {
    if(m_sample.name() == "Zbl" &&
       m_cat.match({{{P::nLep, 0}, {P::nTag, 2}, {P::nJet, 2}}, {{}}})) {
      std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
        return (sys.name.Contains("SysJET_19NP_JET_EtaIntercalibration_Modelling")); };
      removeSysts(f, SysStatus::PruneOther);
    }
    if((m_sample.name() == "Zcl" || m_sample.name() == "Zl") &&
       // m_cat.match({{{P::nLep, 2}, {P::nTag, 1}, {P::bin, 0}, {P::nJet, 3}}, {}})) {
       m_cat.match({{{P::nLep, 2}, {P::nTag, 1}, {P::bin, 0}, {P::nJet, 3}}, {{}}})) {
      std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
        return (sys.name.Contains("JetEResol")); };
      removeSysts(f, SysStatus::PruneOther);
    }
    // remove all systs except MJ systs on multijet samples
    if(m_sample.type() == SType::DataDriven) {
      std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
        return (!sys.name.Contains("MJ") && sys.type != SysType::flt); };
      removeSysts(f, SysStatus::PruneOther);
    }
    /*
       if((m_sample.hasKW("WbbORcc")) && m_cat.match({P::bin, 0})) {
       std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
       return (sys.name.Contains("WMbb")); };
       removeSysts(f, SysStatus::PruneOther);
       }
       */
  }
  // The following things are specific to VHbb Run2 analysis
  if(Configuration::analysisType() == AnalysisType::VHbbRun2) {
   //   std::cout << "PIPPO: in VHbbRun2 analysistype" << std::endl;
   //   std::cout << "PIPPO: " << m_sample.name() << std::endl;
   // if( (m_sample.name()).Contains("Zbb") &&
   //    m_cat.match({{{P::nLep, 2}, {P::nTag, 2}, {P::nJet, 2}}, {{}}})) {
   //   std::cout << "PIPPO: got hold of the Zbb sample" << std::endl;
   //   std::cout << "PIPPO: category " << m_cat.name() << std::endl;
   //   std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
   //     return (sys.name.Contains("SysJET_19NP_JET_EffectiveNP_1")); };
   //   removeSysts(f, SysStatus::PruneOther);
   //   std::cout << "PIPPO: removing sys SysJET_19NP_JET_EffectiveNP_1" << std::endl;
   // }
    //if(m_sample.name() == "Zbl" &&
    //   m_cat.match({{{P::nLep, 0}, {P::nTag, 2}, {P::nJet, 2}}, {{}}})) {
    //  std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
    //    return (sys.name.Contains("SysJET_19NP_JET_EtaIntercalibration_Modelling")); };
    //  removeSysts(f, SysStatus::PruneOther);
    //}
    // remove all systs except MJ systs on multijet samples
    if(m_sample.type() == SType::DataDriven) {
      std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
        return (!sys.name.Contains("MJ") && sys.type != SysType::flt); };
      removeSysts(f, SysStatus::PruneOther);
    }
  }

}

void SampleInCategory::removeLumiWhenFloat() {
  if(m_sample.type() != SType::Bkg) {
    return;
  }
  bool isFloated = std::any_of(m_systs.begin(), m_systs.end(),
                               [](const std::pair<SysKey, Systematic>& syspair)
                               { return (syspair.first.first == SysType::flt) && (syspair.second.isConst == false);
                              });
  if(!isFloated) {
    return;
  }

  std::function<bool(const Systematic&)> f = [](const Systematic& sys) {
    return (sys.name.Contains("ATLAS_LUMI")); };
  removeSysts(f, SysStatus::PruneOther);
}

void SampleInCategory::printSystematics() const {
  std::cout << std::endl << "INFO::SampleInCategory::printSystematics:" << std::endl
    << "  Printing systematics for sample " << m_sample.name() << std::endl
    << "    in category " << m_cat.name() << std::endl;
  for(const auto& sysobj : m_systs) {
    sysobj.second.print();
  }
}

void SampleInCategory::writeNormAndXML(OutputHandler& out, bool statSys) const {
  //std::cout << std::endl << "INFO::SampleInCategory::writeNormAndXML:" << std::endl
  //<< "  Writing systematics for sample " << m_sample.name() << std::endl
  //<< "    in category " << m_cat.name() << std::endl;
  if(m_sample.type() == SType::Data) {
    out.addData(m_sample.name(), m_nomHist.get());
  }
  else {
    out.beginSample(m_sample.name(), m_nomHist.get());
    for(auto& syspair : m_systs) {
      out.addSyst(syspair.second);
    }
    // StatSys activated only for bkg or MJ
    bool activate = (statSys && (m_sample.type() == SType::Bkg || m_sample.type() == SType::DataDriven));
    out.activateStatErr(activate);
    out.closeSample();
  }
}

TString SampleInCategory::name() const {
  return m_sample.name() + "_in_" + m_cat.name();
}

bool SampleInCategory::isSmoothed() {
  return (m_nomHist && m_smoothedNomCorrHist);
}

float SampleInCategory::integral() {
  if(m_nomHist == nullptr) {
    return 0;
  }
  return m_nomHist->Integral(0, m_nomHist->GetNbinsX()+1);
}

void SampleInCategory::pruneSmallSysts_Yao(TH1* hsig, TH1* hbkg, bool isSensitiveRegion,
                                           const std::vector<int>& sensitiveBins) {
  SType stype = m_sample.type();
  // pruning applies to backgrounds only
  if(! (stype == SType::DataDriven || stype == SType::Bkg) ) {
    return;
  }
  // pruning is for small bkgs. Threshold is 1% of total bkg.
  // If above that threshold, then do nothing.
  if(integral() > 0.01 * hbkg->Integral(0, hbkg->GetNbinsX()+1)) {
    return;
  }

  // ok, now we look at the systs in details.

  // if sensitive region, then threshold is 2% of signal in the sensitive bins
  if(isSensitiveRegion) {
    std::function<bool(const Systematic&)> f = [&](const Systematic& sys) {
      return this->isSysBelow(sensitiveBins, sys, hsig, 0.02) &&
                   (!sys.name.Contains("LUMI"));
    };
    removeSysts(f, SysStatus::PruneYao);
  }

  // else threshold is 5 per mille of total background
  else {
    // need to fill vector with all bin numbers
    std::vector<int> bins(hbkg->GetNbinsX());
    std::iota(bins.begin(), bins.end(), 1);
    std::function<bool(const Systematic&)> f = [&](const Systematic& sys) {
      return this->isSysBelow(bins, sys, hbkg, 0.005) &&
                   (!sys.name.Contains("LUMI"));
    };
    removeSysts(f, SysStatus::PruneYao);
  }
}

bool SampleInCategory::isSysBelow(const std::vector<int>& bins, const Systematic& sys,
                                  TH1* ref, float thresh) {
  if(sys.type == SysType::flt) {
    return false;
  }
  if(sys.type == SysType::norm) {
    return std::none_of(bins.begin(), bins.end(),
                        [&](int b){ return
                        ( fabs(this->m_nomHist->GetBinContent(b) * (1-sys.var_up)) >
                          fabs(ref->GetBinContent(b)) * thresh ) ||
                        ( fabs(this->m_nomHist->GetBinContent(b) * (1-sys.var_do)) >
                          fabs(ref->GetBinContent(b)) * thresh ) ;
                        });
  }
  // else shape
  bool shape_up_below = true;
  bool shape_do_below = true;
  if(sys.shape_up != nullptr) {
    shape_up_below = std::none_of(bins.begin(), bins.end(),
                                  [&](int b){ return
                                  fabs(this->m_nomHist->GetBinContent(b) -
                                       sys.shape_up->GetBinContent(b))
                                  > fabs(ref->GetBinContent(b)) * thresh;
                                  });
  }
  if(sys.shape_do != nullptr) {
    shape_do_below = std::none_of(bins.begin(), bins.end(),
                                  [&](int b){ return
                                  fabs(this->m_nomHist->GetBinContent(b) -
                                       sys.shape_do->GetBinContent(b))
                                  > fabs(ref->GetBinContent(b)) * thresh;
                                  });
  }
  return shape_up_below && shape_do_below;
}

void SampleInCategory::pruneSmallShapeSysts_Htautau(TH1* hbkg) {
  float thresh = 0.1;

  SType stype = m_sample.type();
  // pruning applies to backgrounds only
  if(stype == SType::Data || stype == SType::Sig) {
    return;
  }
  std::vector<int> bins(hbkg->GetNbinsX());
  std::iota(bins.begin(), bins.end(), 1);
  std::function<bool(const Systematic&)> f = [&](const Systematic& sys) {
    if(sys.type != SysType::shape) { return false; }
    return std::none_of(bins.begin(), bins.end(),
                        [&](int b){ return
                        fabs(sys.shape_up->GetBinContent(b) - sys.shape_do->GetBinContent(b))
                        > fabs(hbkg->GetBinError(b)) * thresh;
                        });
  };
  removeSysts(f, SysStatus::PruneOther);
}

void SampleInCategory::pruneSmallShapeSysts_chi2() {
  float thresh = 0.99;

  SType stype = m_sample.type();
  // pruning applies to MC only
  if(stype == SType::Data || stype == SType::DataDriven) {
    return;
  }
  std::function<bool(const Systematic&)> f = [&](const Systematic& sys) {
    if(sys.type != SysType::shape) { return false; }
    if(! sys.isSmoothed) { return false; }
    return (this->m_nomHist->Chi2Test(sys.shape_up.get(), "WW") > thresh) &&
      (this->m_nomHist->Chi2Test(sys.shape_do.get(), "WW") > thresh);
  };
  removeSysts(f, SysStatus::PruneOther);
}

void SampleInCategory::pruneSmallShapeSysts_chi2_samesign() {
  SType stype = m_sample.type();
  // pruning applies to MC only
  if(stype == SType::Data || stype == SType::DataDriven) {
    return;
  }
  std::function<bool(const Systematic&)> f = [&](const Systematic& sys) {
    if(sys.type != SysType::shape) { return false; }
    // this pruning only applies to smoothed systematics
    if(! sys.isSmoothed) { return false; }
    double chi2ref = std::max(this->m_nomHist->Chi2Test(sys.shape_up.get(), "WW"),
                              this->m_nomHist->Chi2Test(sys.shape_do.get(), "WW"));
    for(int i=0 ; i<sys.shape_up->GetNbinsX()+2; i++) {
      sys.shape_up->SetBinError(i, this->m_nomHist->GetBinError(i));
    }
    double chi2test = sys.shape_up->Chi2Test(sys.shape_do.get(), "WW");
    for(int i=0 ; i<sys.shape_up->GetNbinsX()+2; i++) {
      sys.shape_up->SetBinError(i, 0);
    }
    return chi2test > chi2ref;
  };
  removeSysts(f, SysStatus::PruneOther);
}

const std::set<TString> SampleInCategory::getConsideredSysts(const SysType type) const {
  std::set<TString> res;
  for(auto& statPair : m_systsStatus) {
    if(statPair.first.first == type) {
      res.insert(statPair.first.second);
    }
  }
  return res;
}

SysStatus SampleInCategory::getSystStatus(const SysKey& key) const {
  auto it = m_systsStatus.find(key);
  if(it == m_systsStatus.end()) {
    return SysStatus::NotConsidered;
  }
  return it->second;
}

float SampleInCategory::getMeanNormEffect(const SysKey& key) const {
  if(key.first != SysType::norm) {
    std::cerr << "ERROR::SampleInCategory::getMeanNormEffect" << std::endl;
    std::cerr << "Requested systematic " << key.second << " is not of type norm" << std::endl;
    throw;
  }
  auto it = m_systs.find(key);
  if(it == m_systs.end()) {
    return 0;
  }
  else {
    auto& sys = it->second;
    return (fabs(sys.var_up - 1) + fabs(sys.var_do - 1)) / 2;
  }
}

double SampleInCategory::getChi2SmoothTest(const SysKey& key, Systematic::Side side) const {
  auto foundIt = m_systsChi2Results.find(key);

  if (foundIt == m_systsChi2Results.end()) {
    std::pair<float, float> chi2Result;
    chi2Result.first  = doChi2SmoothTest(key, Systematic::Side::up);
    chi2Result.second = doChi2SmoothTest(key, Systematic::Side::down);
    foundIt = m_systsChi2Results.emplace(key, std::move(chi2Result)).first;
  }

  return side == Systematic::Side::up ? foundIt->second.first : foundIt->second.second;
}

double SampleInCategory::doChi2SmoothTest(const SysKey& key, Systematic::Side side, const std::string& opts, bool setErrorsOfSysSmoothedToZero) const {
  auto it = m_systs.find(key);
  if (it == m_systs.end())
    return -1.;

  const Systematic& sys = it->second;
  const TH1* smooth = side == Systematic::Side::up ? sys.shape_up.get() : sys.shape_do.get();
  const TH1* orig   = side == Systematic::Side::up ? sys.shape_up_unsmoothed.get() : sys.shape_do_unsmoothed.get();
  std::unique_ptr<TH1> clone;

  if (!smooth || !orig)
    return -1.;

  if (setErrorsOfSysSmoothedToZero) {
    clone.reset((TH1*) smooth->Clone("cloneWithoutErrors") );
    for (int bin = 0; bin < sys.shape_up->GetNbinsX() + 2; ++bin) {
      clone->SetBinError(bin, 0.);
    }

    smooth = clone.get();
  }

  return orig->Chi2Test(smooth, opts.c_str());
}

TH1* SampleInCategory::getShapeSyst(const SysKey& key, Systematic::Side side, bool getUnsmoothed) const {
  if(key.first != SysType::shape) {
    std::cerr << "ERROR::SampleInCategory::getShapeSyst" << std::endl;
    std::cerr << "Requested systematic " << key.second << " is not of type shape" << std::endl;
    throw;
  }
  auto it = m_systs.find(key);
  if(it == m_systs.end()) {
    return nullptr;
  }
  auto& sys = it->second;
  if (side == Systematic::Side::up)
    return getUnsmoothed ? sys.shape_up_unsmoothed.get() : sys.shape_up.get();
  else
    return getUnsmoothed ? sys.shape_do_unsmoothed.get() : sys.shape_do.get();

  return nullptr;
}

void SampleInCategory::removeSameSignBinsSysts() {
  std::set<TString> toTreat;
  for(const auto& sysobj : m_systs) {
    if(sysobj.second.isSmoothed) {
      toTreat.insert(sysobj.second.name);
    }
  }

  for(const auto& sysname : toTreat) {
    TH1* hup = getFullSystShape(sysname, Systematic::Side::up);
    hup->Divide(m_nomHist.get()); // get the ratio
    TH1* hdo = getFullSystShape(sysname, Systematic::Side::down);
    hdo->Divide(m_nomHist.get()); // get the ratio
    // remove same-sign bins
    for(int i=1; i < hup->GetNbinsX()+1; i++) {
      if((hup->GetBinContent(i) - 1) * (hdo->GetBinContent(i) - 1) >= 0) {
        hup->SetBinContent(i,1);
        hdo->SetBinContent(i,1);
      }
    }
    // then rebuild the syst
    hup->Multiply(m_nomHist.get());
    hdo->Multiply(m_nomHist.get());

    //save unsmoothed histos
    std::shared_ptr<TH1> unsmoothedUp, unsmoothedDo;
    if (hasSyst(Systematic::Type::shape, sysname)) {
       Systematic& syst = getSyst(Systematic::Type::shape, sysname);
       unsmoothedUp = syst.shape_up_unsmoothed;
       syst.shape_up_unsmoothed = nullptr;
       unsmoothedDo = syst.shape_do_unsmoothed;
       syst.shape_do_unsmoothed = nullptr;
    }
    // remove the existing syst
    deleteSyst(std::make_pair(SysType::norm, sysname));
    deleteSyst(std::make_pair(SysType::shape, sysname));
    // and add the new one
    addSyst(sysname, Systematic::Side::up, STreat::shape, hup, true, unsmoothedUp);
    addSyst(sysname, Systematic::Side::down, STreat::shape, hdo, true, unsmoothedDo);
  }

}

