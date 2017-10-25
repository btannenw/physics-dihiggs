#ifndef sampleincategory_hpp
#define sampleincategory_hpp

#include <utility>
#include <map>
#include <vector>
#include <set>
#include <functional>
#include <memory>

#include <TString.h>
#include <TH1.h>
#include <TH2.h>

#include "systematic.hpp"

class Sample;
class Category;
class OutputHandler;

/**
 * @brief A sample in a given category
 *
 * This class is the basic block of the internal representation of the analysis.
 * In the output it correspdonds to a block in the XML.
 *
 * It holds a nominal histogram, and all its associated systematics, whatever their
 * types.
 *
 * This class holds all sorts of smoothing and pruning methods, as long as they work
 * at this level.
 *
 * @c SampleInCategory objects that belong to the same category can be merged together
 *
 * @todo Maybe all the base stuff (~nominal stuff) can be kept here, and all the handling
 * of the systematics can be exported elsewhere to better separate them ?
 */
class SampleInCategory
{

  private:
    /// Shortcut for an element ni the map of systematics
    using SysMapElement = std::pair<const SysKey, Systematic>;

    /// Shortcut for iterator on the map of systematics
    using SysIterator = std::map<SysKey, Systematic>::iterator;

    /// Parent category of this @c SampleInCategory
    Category& m_cat;

    /// Parent sample of this @c SampleInCategory
    Sample& m_sample;

    /// Nominal histogram
    std::unique_ptr<TH1> m_nomHist;

    /// Map of systematics affecting this @c SampleInCategory
    std::map<SysKey, Systematic, SysOrdering> m_systs; // use correct ordering for XML

    /// Map of status of systematics wrt pruning
    std::map<SysKey, SysStatus, SysOrdering> m_systsStatus;

    /// Map to cache the chi2 smooth vs unsmoothed results
    mutable std::map<SysKey, std::pair<float, float>, SysOrdering> m_systsChi2Results;

    /// Rebinning vector for a 5% stat uncertainty
    std::vector<int> m_sysBins;

    /// Original nominal histogram
    std::unique_ptr<TH1> m_originalNomHist;

    /// Smoothed nominal correlation histogram
    std::unique_ptr<TH2> m_smoothedNomCorrHist;

    /**
     * @brief Remove systematics that match a given condition
     *
     * This function is called internally by all pruning algorithms.
     *
     * It goes through all systematics attached to this @c SampleInCategory, and tests
     * them against a condition. If the condition is met, all variation histograms are
     * destructed if they were present, the systematic is removed from the map, and the
     * @c reason for the removal is registered in @c m_systsStatus for logging pruposes.
     *
     * @param  pred	Condition against which all systematics are tested
     * @param reason Type of pruning that triggered the condition
     */
    void removeSysts(std::function<bool(const Systematic&)>& pred, SysStatus reason = SysStatus::PruneOther);

    /**
     * @brief Remove a given systematic in the map
     *
     * This function looks for a specific @c SysKey in the systematics map. All
     * variation histograms are destructed if they were present, the systematic is
     * removed from the map, and the @c reason for the removal is registered in
     * @c m_systsStatus for logging pruposes.
     *
     * @param  key Name and type of the systematic to remove
     * @param reason Type of pruning that triggered the condition
     */
    void deleteSyst(const SysKey& key, SysStatus reason = SysStatus::PruneOther);

    /**
     * @brief Delete a histogram if its shape variation wrt nominal is below a threshold
     *
     * If the histogram has enough shape variation, it is kept. If not, it is destructed, and
     * @c false is returned.
     *
     * @see @c hasEnoughShape for the description of what "have sufficient variation wrt nominal"
     * means
     *
     * @param  h	Histogram to test against the nominal. Pointer will be reset if not enough shape.
     * @param thresh Threshold on the variation wrt nominal to consider that @c h has enough shape
     * @return @c true if the histogram is kept, @c false if it is destroyed
     */
    bool deleteIfNoShape(std::shared_ptr<TH1>& h, float thresh);

    /**
     * @brief Check if the impact of a given systematic in a certain range is below a given reference
     *
     * This function typically answers the question:
     * Is the impact of the systematic (@c sys) on @c this background less than 2% (@c thresh) of the
     * signal (@c ref) in the significant bins (@c bins) of the analysis ?
     *
     * If @c sys if of type @c SysType::flt, then we return @c false.
     *
     * The @c SysType::norm and @c SysType::shape are treated separately, but the meaning of the
     * function is always the same, as descirbed above.
     *
     * @param  bins List of bins (potentially discontinuous) over which to check the impact of the syst
     * @param sys Systematic to check
     * @param ref Reference histogram to check the impact against
     * @param thresh Multiplicative factor to the reference histogram
     * @return @c true if the impact of the systematic is always below the reference times the threshold,
     * @c false otherwise
     */
    bool isSysBelow(const std::vector<int>& bins, const Systematic& sys, TH1* ref, float thresh);

    /**
     * @brief Test unsmoothed against smoothed for specified systematic.
     *
     * @param key Systematic to be tested
     * @param side Up or Down
     * @param opts = "WW CHI2/NDF" Options to be passed to TH1::Chi2Test
     * @param setErrorsOfSysSmoothedToZero = true Set errors of the smoothed histogram to 0
     * @return Result of TH1::Chi2Test or -1 if histograms before smoothing not present
     */
    double doChi2SmoothTest(const SysKey& key, Systematic::Side side, const std::string& opts = "WW CHI2/NDF", bool setErrorsOfSysSmoothedToZero = true) const;

  public:
    /// No default constructor
    SampleInCategory() = delete;

    /// Default move constructor
    SampleInCategory(SampleInCategory&&) = default;

    /**
     * @brief Standard constructor
     *
     * Standard constructor used when building the internal representation of the analysis
     * in the nominal case
     *
     * @param  cat	Parent category
     * @param s Parent sample
     * @param hist Nominal histogram
     * @param smoothedCorr If histogram is smoothed, this should be the correlation matrix between the smoothed bins
     */
    SampleInCategory(Category& cat, Sample& s, TH1* hist, TH2* smoothedCorr = nullptr);

    /**
     * @brief Merge constructor
     *
     * Don't be fooled: that is one of the most complex functions of the workspace-making code !
     *
     * It creates a @c SampleInCategory that is the sum of a list of others. The tricky part is
     * the handling of systematics.
     *
     * The merging is done through the following steps:
     * * The nominal histogram is the sum of the nominal histos of the @c sicToMerge
     * * The "binning for systematics" (not used at the moment) is computed
     * * The systematics are merged (see below)
     * * The nominal histograms of the @c sicToMerge are destructed
     *
     * The merging of systematics follows:
     * * For @c SysType::flt, it *must* be present in all @c sicToMerge. It is simply copied
     * to the new @c SampleInCategory
     * * In the other cases, one has to compute the sum over all the @c sicToMerge of the full
     * systematic variation, i.e taking both shape and norm effects when they are present. The
     * result is added to the new @c SampleInCategory through @c addSyst
     *
     * In order not to forget any systematic, the implementation loops over all @c sicToMerge,
     * and over all systematics in each. Then for each syst, one loops over all @c sicToMerge
     * to compute the sum. When a systematic does not exist for a given @c SampleInCategory, the
     * nominal histogram is added. The systematic is then deleted in all @c sicToMerge to avoid
     * re-doing the same addition several times. This implementation is probably not the most
     * optimal one, but speed is not really an issue here, and it is a good balance between speed
     * and simplicity of the implementation.
     *
     * @param cat Parent category
     * @param s Parent (merged) sample
     * @param sicToMerge List of original @c SampleInCategory that are merged into @c this
     */
    SampleInCategory(Category& cat, Sample& s,
                     const std::vector<SampleInCategory*>& sicToMerge);

    /**
     * @brief Destructor
     *
     * The nominal histogram and the systematic ones are not deleted, as they will be written to
     * an output file.
     * @todo double-check that the pointer are indeed invalid after writing, and that we are not
     * leaking memory.
     */
    ~SampleInCategory();

    /**
     * @brief Name of this @c SampleInCategory
     *
     * It is samplename_in_categoryname
     *
     * @return Name of this @c SampleInCategory
     */
    TString name() const;

    /// Accessor to the parent category
    const Category& category() const { return m_cat; }

    /// Accessor to the parent sample
    const Sample& sample() const { return m_sample; }

    /// Accessor to the nominal histogram
    TH1* getNomHist() const { return m_nomHist.get(); }

    /// Accessor to the smoothed correlation histogram
    TH2* getCorrHist() const { return m_smoothedNomCorrHist.get(); }

    /**
     * @brief Determine whether the sample has been smoothed
     *
     * @return true if smoothed, false otherwise
     */
    bool isSmoothed();

    /**
     * @brief Integral of the nominal histogram
     *
     * Includes underflow and overflow.
     *
     * @return 0 if nominal histogram is @c nullptr. Its integral otherwise
     */
    float integral();

    /**
     * @brief Compute the value of @c m_sysBins
     *
     * Simply use the binning tool to get the value. The result has to be sorted for
     * later use.
     *
     * @c m_sysBins is not used at the moment for the systematics treatment.
     */
    void computeBinningForSysts();

    /**
     * @brief Scale the impact of a systematic histo by some factor
     *
     * Use the binning tool to scale a systematic histo wrt the nominal:
     *
     *     hsys -> hnom + factor * (hsys - hnom)
     *
     * @param  hsys	Histo to scale wrt nominal
     * @param factor Enhancement factor
     */
    void scaleHistoSys(TH1* hsys, float factor); // call e.g. from Category::applyHistoSyst

    /**
     * @brief Smooth a systematic histogram
     *
     * Use the localExtremaBinning algorithm from the binning tool.
     *
     * Allow 0 local extrema for all distributions, except mjj, where 1 local
     * extremum is allowed.
     *
     * @see BinningTool::getLocalExtremaBinning for a description of the algorithm
     *
     * @param  hsys	Systematic histo to smooth
     * @param sm Smoothing algorithm to use. Not used at the moment
     */
    void smoothHisto(TH1* hsys, SysConfig::Smooth sm);

    /**
     * @brief Add of a systematic described by an histogram
     *
     * If @c treat is full shape or norm-only, this function creates a norm systematic with the
     * variation, if it passes a pre-pruning cut of one-per-mille effect.
     *
     * If @c treat is full shape or shape-only, this function creates a shape-only systematic with
     * the variation, if it passes a pre-pruning cut of one-per-mille effect on any bin.
     *
     * The created systematic(s) are passed to @c addSyst(Systematic&&) to be effectively inserted
     * or merged in the systematic map of this @c SampleInCategory
     *
     * @param sysname	  Full name of the systematic. All decorrelation tags should already be there
     * @param side      Whether it is the up or down variation of the systematic, or if it should be
     * symmetrized
     * @param treat     If it should be treated norm-only, shape-only, or norm+shape
     * @param hsys      Systematic histogram to deal with
     * @param isSmoothed Whether the histo should be subject to the smoothing of systematics
     * @param hsys_unsmoothed Add an optional extra histo for shape comparisons before / after syst smoothing
     */
    void addSyst(const TString& sysname, Systematic::Side side, STreat treat, TH1* hsys, bool isSmoothed, std::shared_ptr<TH1> hsys_unsmoothed = nullptr );

    /**
     * @brief Insert or merge a systematic into the systematics map
     *
     * If a systematic with the same @c SysKey (type, name) is already present in the map,
     * they are merged. Otherwise, the systematic is simply inserted into the table
     *
     * @see Systematic::merge for a description of the merging
     *
     * @todo This version with @c const @c Systematic& could probably be removed. The
     * @c Systematic&& signature should be sufficient, provided there are the correct
     * forwards
     *
     * @param  sys	Systematic to insert
     */
    void addSyst(const Systematic& sys);

    /**
     * @brief Insert or merge a systematic into the systematics map
     *
     * If a systematic with the same @c SysKey (type, name) is already present in the map,
     * they are merged. Otherwise, the systematic is simply inserted into the table
     *
     * @see Systematic::merge for a description of the merging
     *
     * @param  sys	Systematic to insert
     */
    void addSyst(Systematic&& sys);

    /**
     * @brief Check if a systematic is already present in the systematics map
     *
     * Check for the @c SysKey formed from @c (type,name)
     *
     * @param  type	Type of the systematic to look for
     * @param name Full name of the systematic to look for
     * @return @c true if the systematic is already present, @c false otherwise
     */
    bool hasSyst(SysType type, const TString& name) const;

    /**
     * @brief Look for and return a systematic
     *
     * Accessor to a systematic in the map. Throws an exception if the systematic does
     * not exist
     *
     * @param  type	Type of the systematic to look for
     * @param name Full name of the systematic to look for
     * @return The corresponding systematic. Throws an exception if it does not exist
     */
    Systematic& getSyst(SysType type, const TString& name);

    /**
     * @brief Compute the total variation of a syst, including both norm and shape effects
     *
     * Computes the +/-1-sigma variation of the nominal, including both norma and shape
     * effects.
     *
     * The caller gets the ownership of the returned histogram
     *
     * @param name Full name of the systematic to look for
     * @param side Compute the up or the down variation
     * @return New histogram showing the total systematic variation
     */
    TH1* getFullSystShape(const TString& name, Systematic::Side side);

    /**
     * @brief Remove bins with same-sign variation for smoothed systematics
     *
     * This function is typically called for MV1c distributions, where MC-limited
     * systematics like JES create very small bin migration, but still can lead to
     * undesireable unphysical effects like same-sign variations in a given bin.
     *
     * One loops over all systematics declared as smoothed (EScale/EResol...). The full up
     * and down shape are computed. In bins where they are same-sign wrt nominal, their
     * variations are set to 1. They are then updated in the systematics map with this
     * "smoothed" version.
     */
    void removeSameSignBinsSysts();

    /**
     * @brief Pruning method to remove systs with too small a shape effect
     *
     * Prune separately the up and down shape variations. Remove if the shape effect
     * is less than the threshold in all bins.
     *
     * @see deleteIfNoShape
     *
     * @param  thresh	Threshold for pruning
     */
    void pruneSmallShapeSysts(float thresh);

    /**
     * @brief Prune away a shape systematic if either up or down variation is missing
     *
     * Remove shape systematics when one side of the variation is missing (or has been
     * pruned by @c pruneSmallShapeSysts)
     */
    void pruneOneSideShapeSysts();

    /**
     * @brief Prune systematics that have too small a norm effect
     *
     * If either the up or down variation is small than the threshold, the systematic
     * is pruned.
     *
     * "LUMI" systematics are excepted.
     *
     * @param  thresh	Threshold for pruning
     */
    void pruneSmallNormSysts(float thresh);

    /**
     * @brief Prune norm systematics with same-sign variations
     */
    void pruneSameSignSysts();

    /**
     * @brief Special pruning function that can be used to hardcode some tests
     *
     * At present 2 cases are defined:
     * * @c JetEResol is removed from 2 lepton 1tag 3jet low pTV on Zl and Zcl sample.
     * The effect was found to be crazy (lack of statistics) and lead to a strong pull
     * of the @c JetEResol NP, and still passing all other pruning algorithms
     * * Remove all systematics on multijet samples, except the ones that contain "MJ"
     * in their names. This is used to work around existing bugs in some inputs.
     */
    void pruneSpecialTests();

    /**
     * @brief Remove LUMI NP if this @c SampleInCategory is floated
     *
     * The sample is floated if at least one @c SysType::flt systematic is attached,
     * which is not declared as const.
     */
    void removeLumiWhenFloat();

    /**
     * @brief Apply complex pruning scheme of systematics on this category
     *
     * This one is based on studies by Yao.
     * See his presentation, HSG5 meeting 25/09/2013
     * Remove systs for samples that are small wrt signal and small wrt other backgrounds.
     *
     * This applies to background samples only, and only if their contribution to the total
     * background is above 1% in the parent category.
     *
     * If there are bins with S/B>0.02, systematics are pruned if their effect is less than 2%
     * of the signal in all those bins.
     *
     * Otherwise, systematics are pruned if their effect is less than 5 per mille of the total
     * background in the full distribution.
     *
     * @param  hsig Signal histogram for the parent category
     * @param  hbkg Total background histogram for the parent category
     * @param isSensitiveRegion Whether this category has bins with S/B>0.02
     * @param sensitiveBins List of bins with S/B>0.02
     */
    void pruneSmallSysts_Yao(TH1* hsig, TH1* hbkg, bool isSensitiveRegion,
                             const std::vector<int>& sensitiveBins);

    /**
     * @brief Pruning of systematics small wrt error on background, à la Htautau
     *
     * Prune shape systematics on background samples, when in all bins the difference between
     * their up and down variations is less than 10% of the total MC stat error.
     *
     * @param  hbkg Total background histogram for the parent category
     */
    void pruneSmallShapeSysts_Htautau(TH1* hbkg);

    /**
     * @brief Pruning based on simple chi2, à la Htautau
     *
     * Prune smoothed shape systematics on background samples, when the chi2 probability between the
     * nominal and both up and down variations is above 0.99.
     */
    void pruneSmallShapeSysts_chi2();

    /**
     * @brief Pruning of "same-sign" shape systematics, based on a chi2 test
     *
     * Prune smoothed shape systematics on background samples, when up and down are more similar
     * between each other than with the nominal histo.
     *
     * The implementation uses simple chi2 tests: when the chi2 probability between the up and
     * down is larger than the chi2 prob of both up wrt nominal and down wrt nominal, the
     * systematic is removed.
     */
    void pruneSmallShapeSysts_chi2_samesign();

    /**
     * @brief Check if a histo has a shape sufficiently different from the nominal
     *
     * The shape is sufficiently different if at least in one bin the ratio between h and nominal
     * is above the threshold.
     *
     * e.g: thresh 0.02 means that h and nominal should differ by at least 2% in a bin.
     *
     * @param  h Histogram to test against the nominal
     * @param thresh Threshold on the variation
     * @return @c true if the shape is sufficiently different. @c false otherwise
     */
    bool hasEnoughShape(TH1* h, float thresh);

    /**
     * @brief Print the systematics attached to this @c SampleInCategory
     *
     * Warning: this is super verbose.
     */
    void printSystematics() const;

    /**
     * @brief Get all names of systematics of a given type
     *
     * @param  type	@c SysType of the systematics to get
     * @return List of names of the systematics of the given type
     */
    const std::set<TString> getConsideredSysts(const SysType type) const;

    /**
     * @brief Accessor to the @c SysStatus of a given systematic
     *
     * @param  key Name and type of the systematic to look for
     * @return The status for the systematic, or @c SysStatus::NotConsidered if it is not present
     */
    SysStatus getSystStatus(const SysKey& key) const;

    /**
     * @brief Get the mean normalization effect (absolute) of a systematic
     *
     *     (|up-nom| + |down-nom|)/2
     *
     * @param  key Name and type of the systematic to look for
     * @return Mean absolute effect. 0 if the systematic is not present
     */
    float getMeanNormEffect(const SysKey& key) const;

    /**
     * @brief Accessor to a shape systematic
     *
     * @param  key Name and type of the systematic to look for
     * @param side Get the up or the down variation
     * @param getUnsmoothed Retrieve syst histogram before smoothing
     * @return Pointer to the systematic histogram. @c nullptr if the systematic is not present
     */
    TH1* getShapeSyst(const SysKey& key, Systematic::Side side, bool getUnsmoothed = false) const;

    /**
     * @brief Test smoothed against unsmoothed systematic histogram.
     *
     * Will run a chi2 test for up and down using private function getChi2SmoothTest().
     * Results are cached for repeated use.
     * 
     * @param key Systematic to be tested
     * @param side up or down
     * @return Result of TH1::Chi2Test or -1 if histograms before smoothing not present
     */
    double getChi2SmoothTest(const SysKey& key, Systematic::Side side) const;

    /**
     * @brief Fill the XML file and the root file with normalized histos for this @c SampleInCategory
     *
     * Use the @c OutputHandler to add the nominal histo, then (if not of type data) all the
     * systematics. Activate stat error for this @c SampleInCategory if it is requested and the parent
     * sample is a background.
     *
     * @param  out @c OutputHandler object of the parent category
     * @param statSys Whether MC stat NP should be activated
     */
    void writeNormAndXML(OutputHandler& out, bool statSys) const;

    /**
     * @brief Run the chi2 tests of systematics before and after smoothing.
     *
     * Results will be cached and can be retrieved using getChi2SmoothTest(). Systematic
     * histograms can be deleted after this function call.
     */
    void makeChi2TestsForSysts() {
      if (m_systsChi2Results.empty()) {
        const std::set<TString> shapeNames = getConsideredSysts(SysType::shape);
        for (const auto& name : shapeNames) {
          SysKey key(Systematic::Type::shape, name);
          getChi2SmoothTest(key, Systematic::Side::up);
        }
      }
    }

    /**
     * @brief Clear map with all systematic histograms.
     *
     * Note: Functions like hasSyst() and getSyst() won't return valid systematics.
     */
    void clearSystHistos() {
      m_systs.clear();
    }
};

#endif
