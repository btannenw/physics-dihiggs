#ifndef analysis_impl_hpp
#define analysis_impl_hpp

#include "analysis.hpp"

#include <TString.h>

struct PropertiesSet;

// Factory: builds the necessary bits, but gives ownership to the actual users

/**
 * @brief Generic implementation of the interface of @c Analysis
 *
 * This implementation is supposed to be sufficient for all cases,
 * but another subclass of @c Analysis could be created if needed.
 *
 * @see Analysis for a description of the functions (not repeated here)
 * @see AnalysisHandler for usage of this class
 */
template<typename RegParser, typename InputH, typename RegTrk,
  typename SysBuilder, typename SampBuilder, typename BinTool>
class Analysis_Impl : public Analysis
{
  public:
    Analysis_Impl() = delete;
    Analysis_Impl(const Configuration& conf) : Analysis(conf) {}
    virtual ~Analysis_Impl() = default;

    virtual std::unique_ptr<RegionTracker> regionTracker() {
      std::unique_ptr<RegionTracker> p(new RegTrk());
      return p;
    }

    virtual std::unique_ptr<SystematicListsBuilder> systListsBuilder(const Configuration& conf,
                                                                     std::vector<std::pair<Systematic, SysConfig>>& userSysts,
                                                                     std::vector<std::pair<Systematic, SysConfig>>& pois,
                                                                     std::vector<Systematic32>& r32ratios,
                                                                     std::unordered_map<TString, SysConfig>& histoSysts,
                                                                     std::unordered_map<TString, TString>& renameHistoSysts,
                                                                     std::vector<TString>& constNormFacts) {
      std::unique_ptr<SystematicListsBuilder> p(new SysBuilder(conf, userSysts, pois, r32ratios,
                                                               histoSysts, renameHistoSysts, constNormFacts));
      return p;
    }

    virtual std::unique_ptr<InputsHandler> inputsHandler(const PropertiesSet& pset) {
      std::unique_ptr<InputsHandler> p(new InputH(m_conf, pset));
      return p;
    }

    virtual std::unique_ptr<RegionNamesParser> regionNamesParser() {
      std::unique_ptr<RegionNamesParser> p(new RegParser());
      return p;
    }

    virtual std::unique_ptr<SamplesBuilder> samplesBuilder(const Configuration& conf,
                                                           std::unordered_map<TString, Sample>& samples,
                                                           std::unordered_map<TString, std::set<TString>>& keywords,
                                                           std::map<Sample*, std::vector<Sample*>>& samplesToMerge) {
      std::unique_ptr<SamplesBuilder> p(new SampBuilder(conf, samples, keywords, samplesToMerge));
      return p;
    }

    virtual void createBinningTool(const Configuration& conf) {
      BinTool::createInstance(conf);
    }
};


#endif
