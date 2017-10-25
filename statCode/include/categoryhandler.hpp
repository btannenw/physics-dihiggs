#ifndef categoryhandler_hpp
#define categoryhandler_hpp

#include <vector>
#include <utility>
#include <set>
#include <memory>

#include <TString.h>

#include "category.hpp"
#include "configuration.hpp"
#include "plotting.hpp"

class RegionTracker;

class RegionNamesParser;

/**
 * @brief Class that manages all the categories in the analysis
 *
 * This class builds then holds the categories used in the analysis.
 *
 * It can make plots of the categories.
 *
 * It has statistics / global info to what it holds through a @c RegionTracker
 *
 * Finally, it controls the creation of 'normalized' root files and of the XML files
 * used by @c HistFactory
 */
class CategoryHandler
{
  public:
  private:
    /// Pointer to the global configuration of the analysis
    Configuration m_config;

    /// Pointer to the tool that knows naming conventions of the config files
    std::unique_ptr<RegionNamesParser> m_namesParser;

    /// Storage of the categories
    std::vector<Category> m_categories;

    /// The region tracker of the analysis
    std::unique_ptr<RegionTracker> m_regTrk;

    /// Plotting tool used to make various plots at intermediate steps
    Plotting m_plotting;

    /// The list of XML files created for all categories
    std::vector<TString> m_xmlFiles;

    // the following is in private: only CategoryHandler knows how to populate
    // itself, based on config file switches
    /**
     * @brief Builds the categories based on the config file
     *
     * The configuration is read, and based on it categories are created for the analysis with
     * the correct properties (channel, pTV bin, number of jets, etc...)
     *
     * It simply loops over declared years, channels, and regions for cut-based and MVA,
     * then calls @c addCategory to actually create the categories
     *
     * The @c CategoryIndex is filled at the end with all categories added.
     *
     * This method is private because only @c CategoryHandler knows how to populate itself
     * based on the config file
     *
     * The switches read in the configuration file are:
     * - Years: list of years to run on
     * - Regions: list of regions to use
     *
     * @see @c NamingTool for the precise syntax of the config file
     *
     */
    void populate();

    /**
     * @brief Try to build and add a @c Category to this @c CategoryHandler
     *
     * First, passes @c regName to the @c RegionNamesParser to get
     * the properties of the to-be-built @c Category. If the configuration is
     * not consistent, stop here.
     *
     * Then build a @c Category, and call @c Category::exist to check that an input
     * root file exists for this category. If yes, add the @c Category to @c
     * m_categories
     *
     * @param regName String describing a region in the config file
     */
    void addCategory(const TString& regName);

  public:
    /// No default constructor
    CategoryHandler() = delete;

    /**
     * @brief Standard constructor
     *
     * Standard constructor with a copy of the @c Configuration.
     *
     * @param conf	The reference configuration
     */
    CategoryHandler(const Configuration& conf);

    /// Destructor
    ~CategoryHandler();

    /// Accessor to the @c RegionTracker
    const RegionTracker& regionTracker() const { return *m_regTrk; }

    /**
     * @brief Function called when all nominal histograms are loaded
     *
     * - Propagate the call to all categories.
     * - Fill the @c RegionTracker with information from the categories
     * - If one has requested a default fit config (for instance to make
     *   post-fit plots of the full fit model only in a few categories), fill
     *   the @c RegionTracker with 0, 1, 2 leptons, 2 and 3 jets, 1 and 2 tags.
     *
     * The switches read in the configuration file are:
     * @conf{DefaultFitConfig,false} request the standard fit configuration for systematics,
     *   even if not all required categories are present
     */
    void finalizeNominal();

    /**
     * @brief Make control plots per category
     *
     * Make basic control plots for each category, using the plotting tool.
     */
    void makeControlPlots();

    /**
     * @brief Make the 2D plots showing the status of systematics
     *
     * Use the plotting tool to make the plots showing the status of systematics.
     * Plots are made showing the status of norm systs, the status of shape systs,
     * and the size of norm systs.
     *
     * The plots either show all systematics for a given region, or show all categories
     * for a given systematic uncertainty.
     */
    void makeSystStatusPlots() const;

    /**
     * @brief Make plots of the systematic shapes
     *
     * Loop over categories to make plots showing the systematics shapes
     */
    void makeSystShapePlots() const;

    /**
     * @brief Make plots of the systematic shapes
     *
     * Use the plotting tool to make the plots of systematics shape for a given
     * category
     *
     * @param cat The category to make plots of
     */
    void makeSystShapePlots(const Category& cat) const;

    /**
     * @brief Write all normalized root files and XML files
     *
     * First, loop over categories, to have them write their xml file and their
     * normalized root file.
     *
     * Then, write the driver.xml file that list all categories, the POIs, and the
     * floating normalization NP that should stay constant
     *
     * @param constNormFacts	List of floating normalization NP that should stay constant
     * (i.e not used in the profiling)
     * @todo if a constNormFact was asked to be decorrelated among regions ,this is not
     * reflected here. As such the param will NOT be const. This is a bug.
     * @param poiNames List of actual POI names, i.e with decorrelation tags added
     * @return A pair with the pathname of the driver.xml file, and the pathname of
     * the workspace root file
     */
    std::pair<TString, TString> writeNormAndXML(const std::vector<TString>& constNormFacts,
                                                const std::set<TString>& poiNames);

    /**
     * @brief Write the normalized root file and the xml file for a given category
     *
     * Call for writing the normalized root file and the xml file for a given category.
     * Retrieve the path to the xml file and store it into @c m_xmlFiles
     *
     * @param cat The category we call @c writeNormAndXML on
     */
    void writeNormAndXMLForCat(Category& cat);

    /**
     * @brief Clear the map with the syst histograms for given category.
     */
    void clearSystHistos(Category& cat) {
      cat.clearSystHistos();
    }

    /**
     * @brief Clear the map with the syst histograms in all categories.
     */
    void clearAllSystHistos() {
      for (auto& cat : m_categories) {
        cat.clearSystHistos();
      }
    }

    /**
     * @brief Make chi2 tests of smoothed vs unsmoothed systematics.
     *
     * @param cat Category to run chi2 tests on.
     */
    void makeChi2TestsForSysts(Category& cat) {
      cat.makeChi2TestsForSysts();
    }

    /// @name Yay, syntaxic sugar to easily iterate over categories
    /// @{

    /// const iterator
    typedef std::vector<Category>::const_iterator const_iterator;

    /// const begin
    const_iterator begin() const { return m_categories.begin(); }

    /// const end
    const_iterator end() const { return m_categories.end(); }

    /// iterator
    typedef std::vector<Category>::iterator iterator;

    /// begin
    iterator begin() { return m_categories.begin(); }

    /// end
    iterator end() { return m_categories.end(); }

    /// @}
};

#endif
