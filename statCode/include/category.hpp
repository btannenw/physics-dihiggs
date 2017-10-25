#ifndef category_hpp
#define category_hpp

#include <map>
#include <memory>
#include <vector>

#include <TString.h>

#include "properties.hpp"
#include "inputshandler.hpp"
#include "configuration.hpp"
#include "sampleincategory.hpp"
#include "sample.hpp"
#include "outputhandler.hpp"
#include "systematic.hpp"

class TH1;
class SystematicHandler;
class TDirectory;

/**
 * @brief Class that describes a category (region) of the analysis
 *
 * The class describes a category of the analysis
 *
 * A category is characterized by a set of properties that describe it
 * uniquely, like the number of leptons, number of jets, of b-tags, etc...
 * Attached is also the rebinning used for all the histograms of the region.
 *
 * A category holds the @c SampleInCategory that are present in this category,
 * indexed by the @c Sample objects. It IS the owner of the @c SampleInCategory
 * objects.
 *
 * As such, it is in control of the creation of the @c SampleInCategory
 *
 * As input files are split by categories, each category owns an @c InputsHandler
 * to get all the relevant histograms.
 *
 * Similarly, it owns an @c OutputHandler to perform the writing of the XML files
 * and of the normalized ROOT files used by @c HistFactory.
 *
 *
 */
class Category
{

  private:
    /// Pointer to the @c Configuration of the analysis
    Configuration m_config; // TODO needed ?

    /// Set of properties that describe this category
    PropertiesSet m_properties;

    /// Pointer to the @c InputsHandler that takes care of histograms for this category
    std::unique_ptr<InputsHandler> m_input;

    /// Vector describing the rebinning used for this category
    std::vector<int> m_binning;

    /// Storage of the associated @c SampleInCategory, indexed by the samples
    std::map<Sample*, SampleInCategory> m_samples;

    /// @c OutputHandler for this category
    OutputHandler m_out;

    /// S/B histogram
    TH1* m_hsob;

    /// Rebinning stored as an histogram. Can be exported to make plots with variable bin sizes
    TH1* m_binHist;

    /**
     * @brief Get a histogram, sum of all samples of a given type
     *
     * Loop over all samples of type @c type that are present in this category,
     * and gives the sum of their nominal histograms.
     *
     * The caller takes ownership of the returned histogram
     *
     * @param type	The @c Sample::Type for which the histo is requested
     * @return Pointer to the sum histogram. @c nullptr is no sample in this category
     * is of type @c type
     */
    TH1* getHist(SType type) const;

    // name is the name of the syst; side is Up, Do or symmetrize;
    // dirname is the name of the TDirectory where it can be found;
    // conf if the configuraiton for treatment and correlations
    /**
     * @brief Apply a histo-based systematic to samples in this category
     *
     * For all samples in this category:
     * * check if they are concerned by @c conf
     * * check if the syst histo exist for them
     * * rebin the histo to fit this category
     * * call for smoothing if requested in @c conf
     * * forge the syst name with correct decorrelation tags (for category and sample)
     * * call @c addSyst on the @c SampleInCategory
     *
     * @todo Smoothing is explicitely not called for MV1c distributions. The logic for this
     * should probably be in SampleInCategory::smoothHisto ?
     *
     * @todo There is currently a hack for the size of WDPhi systematic in 3 jet low pTV
     * (which is wrong in @c CorrsAndSysts)
     *
     * @param name	Name of the systematic, e.g SysBTagB6
     * @param side  Is it up, down, or symmetrize ?
     * @param dirname Name of the @c TDirectory that contains the histos for this syst
     * @param conf @c SysConfig object associated to this systematic
     */
    void applyHistoSysts(const TString& name, const Systematic::Side side,
                         const TString& dirname, const SysConfig& conf);


    /**
     * @brief Set @c m_binHist from @c m_binning
     *
     * @c m_binHist is useful to export in workspaces, to later make plots with
     * bins of variable size.
     *
     */
    void setBinningHist();


  public:
    /// No default constructor
    Category() = delete;

    /// Standard constructor, with configuration and a set of properties
    Category(const Configuration& conf, PropertiesSet&& pset);

    /// Default move constructor
    Category(Category&&) = default;

    /// Default destructor
    ~Category() = default;


    /// Accessor to the map of @c SampleInCategory
    inline std::map<Sample*, SampleInCategory>& samples() { return m_samples; }

    /// Const accessor to the map of @c SampleInCategory
    inline const std::map<Sample*, SampleInCategory>& samples() const { return m_samples; }

    /// Const accessor to the properties
    const PropertiesSet& properties() const { return m_properties; }

    /// Query an integer property
    int getIntProp(const Property p) const;

    /// Query a string property
    TString getStringProp(const Property p) const;

    /// Query an integer property. Throwing version.
    int requestIntProp(const Property p) const;

    /// Query a string property. Throwing version.
    TString requestStringProp(const Property p) const;

    /// Set an integer property. Should that be here ?
    void setIntProp(Property p, int i);

    /// Set a string property. Should that be here ?
    void setStringProp(Property p, const TString& s);

    /// Shortcut to query an integer property
    inline int operator[](const Property p) const { return getIntProp(p); }

    /// Shortcut to query a string property
    inline TString operator()(const Property p) const { return getStringProp(p); }

    /**
     * @brief Const accessor to the name of the category
     *
     * The name is @c Region_ followed by the tags made from the properties
     */
    TString name() const;

    /**
     * @brief Check the existence of this category
     *
     * A category is said to exist if there is an input file that corresponds
     * to the set of properties attached to the category.
     *
     * @return @c true of this category exists in the inputs. @c false otherwise
     */
    bool exists();

    /**
     * @brief Build a @c SampleInCategory from this category and a given sample
     *
     * Try to build a @c SampleInCategory for this category and @c s.
     * Nothing is created if:
     * * no histogram is present in the input file
     * * the integral is extremely small (10^-9)
     * * the integral is compatible with 0 within MC stat error (cases with 1 event,
     * or many events with small weight and 1 event with big weight)
     *
     * In the other cases, a @c SampleInCategory is built and added to the map
     *
     * @param s	@c Sample that should be added
     */
    void tryAddSample(Sample& s);

    /**
     * @brief Perform operations once all nominal histograms have been read
     *
     * Use the binning tool to perform all range cutting and rebinning operations
     * for this category, on all samples.
     *
     * In particular store the binning for future use, and create the binning histogram.
     */
    void finalizeNominal();

    /**
     * @brief Apply all histo-based systematics in this category
     *
     * First, loop through all systematics directories that exist in the input file for
     * this category.
     *
     * Check if a configuration has been defined for the systematic. If not, continue or
     * fail depending on @c skipUnknown.
     *
     * Check if this category matches the configuration.
     *
     * Call the private @c applyHistoSysts with information on the given syst.
     *
     * On a second step, if this category is a MV1c distribution, remove same-sign
     * bins in all the samples.
     *
     * @param systs	Instance of @c SystematicHandler
     * @param skipUnknown continue or fail on unknown systematics
     */
    void applyHistoSysts(const SystematicHandler& systs, bool skipUnknown);

    /**
     * @brief Is this category a control region
     *
     * Control regions are defined as regions with:
     * * less than 2 tags
     * * @c spec property set to "topcr" or "topemucr"
     *
     * @return @c true if it is a control region, @c false otherwise
     */
    bool isControlRegion() const;

    /// A signal region is not a control region
    bool isSignalRegion() const { return !isControlRegion(); }

    /// Check if one of the bins has a S/B over the threshold @c thresh
    bool hasSensitiveBinAbove(float thresh) const;

    /**
     * @brief Check the properties of this category match the requirements in @c pset
     *
     * @see PropertiesSet::match() for a description of matching
     *
     * @return @c true if match, @c false otherwise
     */
    bool match(const PropertiesSet& pset) const;

    /**
     * @brief Get the total signal histogram
     *
     * Get the sum of samples that are of @c Sample::Type @c Sig.
     *
     * The caller takes the ownership of the returned histogram.
     *
     * @return Pointer to the summed signal histogram
     */
    TH1* getSigHist() const;

    /**
     * @brief Get the total background histogram
     *
     * Get the sum of samples that are of @c Sample::Type @c Bkg and @c DataDriven
     *
     * The caller takes the ownership of the returned histogram.
     *
     * @return Pointer to the summed background histogram
     */
    TH1* getBkgHist() const;

    /**
     * @brief Get the data histogram
     *
     * The caller takes the ownership of the returned histogram.
     *
     * @return Pointer to the data histogram
     */
    TH1* getDataHist() const;

    /**
     * @brief Get the S/B histogram for this category
     *
     * The caller DOES NOT take ownership of this histogram, which is cached in @c this
     *
     * @todo return a shared_ptr to avoid ownership issues, or maybe weak_ptr.
     *
     * @return Pointer to the S/B histogram
     */
    TH1* getSoBHist() const;

    /**
     * @brief Write the binning histogram
     *
     * If the binning histogram has been defined, write it in @c dir.
     *
     * @param dir	The directory in which the histogram should be written
     */
    void writeBinHist(TDirectory* dir);

    /**
     * @brief Call the writing of the XML and the normalized ROOT files for this category
     *
     * Create the @c OutputHandler object for this category.
     * Call @c SampleInCategory::writeNormAndXML on all samples in this category. Also add
     * the configuration for MC stat uncertainty in the XML. Finally close all open files.
     *
     * Configuration options read:
     * @conf{UseStatSystematics,true} use or not MC stat systs
     * @conf{StatThreshold,0.05} threshold for MC stat uncertainty
     *
     * @return /path/to/xmlFile.xml
     */
    TString writeNormAndXML();

    /**
     * @brief Apply complex pruning scheme of systematics on this category
     *
     *  This one is based on studies by Yao.
     *  See his presentation, HSG5 meeting 25/09/2013
     *  Remove systs for samples that are small wrt signal and small wrt other backgrounds.
     *
     *  More precisely:
     *  * Get the bins with S/B > 0.02
     *  * Call @c pruneSmallSysts_Yao on all samples with the list of bins as a parameter
     */
    void pruneSmallSysts_Yao();

    /**
     * @brief Apply pruning scheme à la Htautau to this category
     *
     * * Get the total background histogram
     * * Call pruneSmallShapeSysts_Htautau to all samples with the histo as a parameter
     */
    void pruneSmallShapeSysts_Htautau();

    /**
     * @brief Clear systematic histos in the sample map.
     */
    void clearSystHistos() {
      for (auto& sample_and_sic : m_samples) {
        sample_and_sic.second.clearSystHistos();
      }
    }

    /**
     * @brief Precalculate all chi2 tests between syst histograms before and after smoothing.
     */
    void makeChi2TestsForSysts() {
      for (auto& sample_and_SiC : m_samples) {
        sample_and_SiC.second.makeChi2TestsForSysts();
      }
    }
};

#endif
