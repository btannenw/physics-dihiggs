#ifndef systematichandler_hpp
#define systematichandler_hpp

#include <vector>
#include <unordered_map>
#include <utility>
#include <memory>

#include <TString.h>

#include "systematic.hpp"
#include "configuration.hpp"
#include "systematiclistsbuilder.hpp"
#include "properties.hpp"
#include "containerhelpers.hpp"

class SampleInCategory;
class Category;
class Sample;
class RegionTracker;

/**
 * @brief Class to manage the configuration of all systematics
 *
 * The main role of this class is to store the configuration for all sytematics
 * of the analysis:
 * * user-defined norm systematics
 * * POIs, norm factors
 * * histo-based systematics
 * * 3-to-2 systematics
 *
 * It has two main functions:
 * * @c listAllHistoSystematics defines the configuration for all histo-based systs
 * * @c listAllUserSystematics is used to set all user-defined values, normfactors, and POIs.
 *
 * The actual filling of the lists of systematics is delegated to @c SystematicListsBuilder
 *
 * Other functions can alter the configurations for test purposes (@c decorrSysForCategories),
 * or the names to workaround naming bugs in the inputs (@c m_renameHistoSysts).
 *
 * This class was also found the best place to define the (static) matching functions between
 * @c SysConfig objects, and samples or categories.
 */
class SystematicHandler
{
  private:
    /// Configuration of the analysis
    Configuration m_config;

    /**
     * @brief Storage of the user-defined systematics (norm systs and norm factors)
     *
     * A @c vector<pair> is more adapted than a @c map because can have several entries for 
     * same thing, but with different configs,
     * e.g have separately ttbar in 3j and 2j with different priors
     *
     * We also don't need any fast random access.
     */
    std::vector<std::pair<Systematic, SysConfig>> m_userSysts;

    /// Storage of the POIs
    std::vector<std::pair<Systematic, SysConfig>> m_pois;

    /// Storage of the 3-to-2 systematics
    std::vector<Systematic32> m_32ratios;

    /**
     * @brief Storage of the hist-based systematics
     *
     * Here we need fast random acces, so @c unordered_map is used
     */
    std::unordered_map<TString, SysConfig> m_histoSysts;

    /// Storage for renaming rules of systematics
    std::unordered_map<TString, TString> m_renameHistoSysts;

    /// List of NormFactors that should be constant in the fit
    std::vector<TString> m_constNormFacts;

    /**
     * @brief Pointer to the class that builds the lists of systematics
     *
     * The listing of systematics is delegated to a virtual specialized class
     * that must be subclassed by the analyses
     */
    std::unique_ptr<SystematicListsBuilder> m_systListsBuilder;


  public:

    /// No empty constructor
    SystematicHandler() = delete;

    /**
     * @brief Standard contructor
     *
     * @param conf Configuration of the analysis
     */
    SystematicHandler(const Configuration& conf);

    /// Empty destructor
    ~SystematicHandler() {}

    /// @name Accessors
    /// @{

    /// Accessor to user-defined systematics
    std::vector<std::pair<Systematic, SysConfig>>& userSysts() { return m_userSysts; }

    /// Accessor to the list of NormFactors that should be constant
    const std::vector<TString>& constNormFacts() { return m_constNormFacts; }

    /// Accessor to the list of POIs
    std::vector<std::pair<Systematic, SysConfig>>& POIs() { return m_pois; }

    /// Accessor to 3-to-2- systematics
    std::vector<Systematic32>& ratios32() { return m_32ratios; }

    /// @}

    /**
     * @brief Check if a configuration has been defined for this histo-based systematic
     *
     * Check that the name of the systematic exists in @c m_histoSysts or in @c m_renameHistoSysts
     *
     * @todo There is a small bug. If a rule exists in @c m_renameHistoSysts, it should be checked
     * that a rule exists in @c m_histoSysts for the rewritten name
     *
     * @param name Name of the systematic to look for
     * @return @c true if a configuration exists. @c false otherwise.
     */
    bool hasHistoSysConfig(const TString& name) const;

    /**
     * @brief Get the configuration that corresponds to a given histo systematic
     *
     * The function returns a pair with the name of the systematic: in case there has been a
     * correction of the name (rewriting rule defined in @c m_renameHistoSysts), the output is
     * the correct name of the systematic to write in e.g the XML file.
     *
     * This function throws an exception if no configuration exists for the systematic.
     *
     * @param name	Name of the systematic to look for. It must be the syst name, i.e Up or Do
     * suffixes should have been stripped beforehand
     * @return A pair with the name of the systematic, and the corresponding configuration.
     */
    std::pair<TString, const SysConfig&> getHistoSysConfig(const TString& name) const;

    /// @name Definition of matches between syst configurations and samples or categories
    /// @{

    /**
     * @brief Matching for @c SampleInCategory objects
     *
     * Matches if both the sample and the category @c s belongs to match @c sys.
     *
     * @param s	Sample in category
     * @param sys Systematic configuration
     * @return @c true if @c s matches the configuration, @c false otherwise
     */
    static bool match(const SampleInCategory& s, const SysConfig& sys);

    /**
     * @brief Matching for samples
     *
     * A sample matches a systematic configuration if it has any keyword associated
     * to the configuration.
     *
     * @param s Sample to test
     * @param sys Systematic configuration
     * @return @c true if @c s matches the configuration, @c false otherwise
     */
    static bool match(const Sample& s, const SysConfig& sys);

    /**
     * @brief Matching for categories
     *
     * A category matches a systematic configuration if it matches any @c PropertiesSet
     * defined in @c sys.categories.
     *
     * @see Category::match for the description of matching rules of a category with a
     * set of properties
     *
     * @param c Category to test
     * @param sys Systematic configuration
     * @return @c true if @c c matches the configuration, @c false otherwise
     */
    static bool match(const Category& c, const SysConfig& sys);

    /// @}

    // list them all !
    /**
     * @brief Populate the list of user-defined systematics
     *
     * This is the main function where all user-defined systematics and flaoting norm NP should
     * be defined with their configurations.
     *
     * The POI(s) is also defined here.
     *
     * The @c RegionTracker object can be  used to decide to float or put a prior on some backgrounds.
     * Example: W+jets backgrounds floated when 1 lepton channel is present, have priors when 1
     * lepton is not there.
     *
     * The actual filling of the lists is delegated to @c SystematicListsBuilder
     *
     * @see @c SystematicListsBuilder for the actual filling
     *
     * @param regTrk	Object tracking what type of regions are present in the analysis
     * @param useFltNorms Whether some backgrounds should be let free, or be constrained to a prior
     */
    void listAllUserSystematics(const RegionTracker& regTrk, bool useFltNorms);

    /**
     * @brief Populate the list of expected histo-based systematics
     *
     * This is the main function where all the configuration for histo-based systematics
     * is defined.
     *
     * It also triggers the filling of the map with renaming rules of systematics
     *
     * The actual filling of the lists is delegated to @c SystematicListsBuilder
     *
     * @see @c SystematicListsBuilder for the actual filling
     *
     * @param regTrk	Object tracking what type of regions are present in the analysis
     */
    void listAllHistoSystematics(const RegionTracker& regTrk);

    /**
     * @brief Put coherent systematics on two samples, to match a given syst size on their ratio
     *
     * This function is used to put a systematic on a ratio, while preserving the sum of the 2
     * @c SampleInCategory involved.
     *
     * This is typically used to put a systematic on a 3/2 jet ratio, while preserving the
     * 2+3 integral in each couple of categories.
     *
     * @param name	Full name of the systematic (with decorrelation tags already added)
     * @param size  Size of the systematic (e.g 0.1 means 10%)
     * @param sic1  First @c SampleInCategory (numerator)
     * @param sic2  Second @c SampleInCategory (denominator)
     * @param weight1 Weight to be given to the first @c SampleInCategory. Integral of the SiC if
     * value <=0 given
     * @param weight2 Weight to be given to the second @c SampleInCategory. Integral of the SiC if
     * value <=0 given
     */
    static void applyRatioSys(const TString& name, float size, SampleInCategory& sic1, SampleInCategory& sic2,
                              float weight1=0, float weight2=0);

    /**
     * @brief Change configuration of a systematic to decorrelate it between some categories
     *
     * This function allows to modify the configuration of an existing systematic, either user-defined
     * or histo-based, after the initial filling of the configuration lists.
     *
     * This is typically used to study the origin of pulls/constraints of NPs: it can indeed be triggered
     * for a specific NP through the config file of the analysis (@c DecorrTypes, @c DecorrSys)
     *
     * @param name	Name of the systematic which to change the configuration of
     * @param props Properties that are to be decorrelated
     * @param replace If @c true, wipe current configuration and decorrelate @c props. If false, @c props are
     * decorrelated on top of the existing configuration
     */
    void decorrSysForCategories(const TString& name, const std::vector<Properties::Property>& props, bool replace = true);

    /**
     * @brief Change configuration of a systematic to decorrelate it between all categories
     *
     * This function allows to replace the configuration of an existing systematic, either user-defined
     * or histo-based, after the initial filling of the configuration lists, so that it is fully
     * decorrelated between all analysis regions.
     *
     * @param name	Name of the systematic which should be fully decorrelated
     */
    void decorrSysForAllCategories(const TString& name);

};

#endif
