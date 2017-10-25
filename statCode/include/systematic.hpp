#ifndef systematic_hpp
#define systematic_hpp

#include <vector>
#include <functional>
#include <utility>
#include <memory>

#include <TString.h>

#include "properties.hpp"

class TH1;
class Sample;

/**
 * @file systematic.hpp
 * @brief Contains the two basic blocks for the incorporation of systematics
 *
 * Systematic is the description of 1 systematic (or floating norm) on 1 sample
 * in 1 category.
 *
 * SysConfig is the mechanism used to describe treatments that need to be applied on
 * systematics (smoothing...), as well as to decide the regions and samples of application,
 * and setup the decorrelations between the categories and samples.
 *
 * @todo A better structure could be the following:
 * - A namespace Sys. The enums of @c Systematic should be directly there. @c interpretSysName
 *   should be there too.
 * - So @c Systematic would be in that namespace and would have only the info directly used to
 *   describe a given syst
 * - The rest of the contents of the file should be in the namespace too, and their names
 *   simplified, e.g @c SysConfig -> @c Sys::Config
 *
 */



/**
 * @brief A systematic of any type, that applies on 1 sample in 1 category
 *
 * The internal (in @c WSMaker) representation of a systematic of any type (histo, normalization,
 * even floating normalization). This information is easily translated into @c HistFactory language,
 * with all numbers written in the XML files and variation histograms written in 'normalized'
 * @c ROOT files.
 *
 * @see OutputHandler for the translation in @c HistFactory language
 *
 * When building systematics from histograms, one loops over the input files, and will encounter
 * a @c MySystUp and a @c MySystDo directories, in any order. So typically partial @c Systematic
 * objects will be created with only @c up or @c down variation, then they will be merged into
 * a complete object.
 *
 * @see @c Category and @c SampleInCategory for the logic of the construction of systematics
 * from input histograms
 *
 */
struct Systematic
{
  /// Possible sides of a systematic when built from histograms.
  enum class Side {up, down, symmetrize /**< Only 1 histo given, to be symmetrized (e.g @c JetEResol).
                                          By convention the histo given is considered as the
                                          @c up variation. */};

  /**
   * @brief A syst can be a shape syst, a norm syst, or a floating norm syst.
   *
   * The order of the enum is important to follow the HistFactory XML DTD.
   *
   */
  enum class Type { shape, norm, flt };

  /**
   * @brief Status of a systematic wrt pruning
   *
   * If the systematic has been pruned at some stage, keep track of it.
   *
   * @todo Should we keep this enum here ?
   *
   */
  enum Status { NotConsidered = 0, PruneSmall, PruneOneSide, PruneYao, PruneOther, Used };

  Type type; ///< Type of the systematic
  TString name; ///< Name of the systematic, e.g SysBTagB3
  float var_up; ///< Up variation for a norm systematic, and upper bound for floating norm
  float var_do; ///< Down variation for a norm systematic, and lower bound for floating norm
  float init; ///< Initial value for floating norm
  std::shared_ptr<TH1> shape_up; ///< Histo with Up variation for shape syst
  std::shared_ptr<TH1> shape_do; ///< Histo with Down variation for shape syst
  bool isConst; ///< For a floating norm, should it be constant ?
  bool isSmoothed; ///< For a shape syst, has it been smoothed ?
  std::shared_ptr<TH1> shape_up_unsmoothed; ///< Histo with Up variation for shape syst before smoothing
  std::shared_ptr<TH1> shape_do_unsmoothed; ///< Histo with Down variation for shape syst before smoothing

  /// No empty constructor
  Systematic() = delete;

  /// Default copy constructor
  Systematic(const Systematic&) = default;

  /// Default move constructor
  Systematic(Systematic&&) = default;

  /**
   * @brief Generic constructor for an empty syst
   *
   * @param t Type of the systematic
   * @param aname Name of the systematic
   */
  Systematic(Type t, const TString& aname):
    type(t), name(aname), var_up(1), var_do(1), init(0),
    shape_up(nullptr), shape_do(nullptr), isConst(false), isSmoothed(false), shape_up_unsmoothed(nullptr), shape_do_unsmoothed(nullptr) {}

  /**
   * @brief Constructor for a normalization systematic
   *
   * @param aname Name of the systematic
   * @param down Normalization factor for down variation (e.g 0.8 for -20%)
   * @param up Normalization factor for up variation (e.g 0.8 for -20%)
   */
  Systematic(const TString& aname, float down, float up):
    type(Type::norm), name(aname), var_up(up), var_do(down), init(0),
    shape_up(nullptr), shape_do(nullptr), isConst(false), isSmoothed(false), shape_up_unsmoothed(nullptr), shape_do_unsmoothed(nullptr) {}

  /**
   * @brief Constructor for a floating norm systematic
   *
   * @param aname Name of the systematic
   * @param init Initial value of the scale factor
   * @param down Lower bound
   * @param up Upper bound
   * @param constness Should this normalization be floated or not in the fits ?
   */
  Systematic(const TString& aname, float init, float down, float up, bool constness):
    type(Type::flt), name(aname), var_up(up), var_do(down), init(init),
    shape_up(nullptr), shape_do(nullptr), isConst(constness), isSmoothed(false), shape_up_unsmoothed(nullptr), shape_do_unsmoothed(nullptr) {}

  /**
   * @brief Find the real name and @c Side of the syst from its directory name in input files
   *
   * HSG5 conventions are:
   * - @c XXXXUp -> name is @c XXXX, @c Side is @c up
   * - @c XXXXDo -> name is @c XXXX, @c Side is @c down
   * - @c XXXX   -> name is @c XXXX, @c Side is @c symmetrize
   *
   * But there are exceptions in some inputs... so we take care of them too.
   * @c JetEResolUp that is present sometimes should be interpreted as JetEResol.
   *
   * @param name	The directory name in the input file
   * @return A pair with the name of the systematic and its @c Side
   */
  static std::pair<TString, Side> interpretSysName(const TString& name);

  /**
   * @brief Merge @c other @c Systematic into @c this
   *
   * Merge 2 complementary systematics into a single one. Typical use-case is for
   * histo-based systematics, where the up and down variations are built separately,
   * then merged.
   *
   * @c this and @c other must be of the same @c Type, otherwise an exception is thrown.
   *
   * In case a property defined in @c other already exists in @c this, the value from
   * @c other is used. A warning is then printed on @c stdout.
   *
   * @param other	The @c Systematic to merge into @c this
   */
  void merge(const Systematic& other);

  /**
   * @brief Comparison of @c Systematic
   *
   * Ordering of @c Systematic is ensures they are written in correct order in the XML files.
   * We have to comply with the @c HistFactory DTD format.
   *
   * @see @c SysOrdering for the description of the ordering scheme
   *
   * @param other The @c Systematic to compare to
   * @return @c true if @c this is lower than @c other. @c false otherwise.
   */
  bool operator<(const Systematic& other) const;

  /**
   * @brief Equality of @c Systematic
   *
   * Both @c type and @c name are tested for equality.
   *
   * @param other	The @c Systematic to compare to
   * @return @c true if @c this and @c other are equal in type and name
   */
  bool operator==(const Systematic& other);

  /**
   * @brief Print some info on @c this
   *
   * The content printed depend on the type. Useful for debugging purpose,
   * but can quickly fill the ouput if used on all systematics of a workspace
   *
   */
  void print() const;
};

using SysType = Systematic::Type;
using SysKey = std::pair<SysType, TString>;
using SysStatus = Systematic::Status;


/**
 * @brief Ordering scheme of @c Systematic
 *
 * Systematics are first ordered according to their @c Type. The order of the enum is
 * chosen to comply with @c HistFactory DTD.
 *
 * Then systematics are ordered with their name, in plain alphabetical order.
 */
struct SysOrdering {
  bool operator()(const SysKey& s1, const SysKey& s2) const {
    int itype = static_cast<int>(s1.first);
    int other_itype = static_cast<int>(s2.first);

    if(itype == other_itype)
      return (s1.second.CompareTo(s2.second) > 0);
    return (itype < other_itype);
  }
};

/**
 * @brief Configuration of area of application and decorrelations for systematics
 *
 * This structure is heavily used when adding systematics in the workspaces.
 * Through a series of matching rules, it encodes the configuration of systematics:
 * - how to treat them (shape, norm...)
 * - if they should be smoothed
 * - where do they apply, and on what samples
 * - what regions should be decorrelated or grouped together
 * - what samples should be decorrelated or grouped together
 *
 * The decorrelations are translated into 'tags' appended to the systematics names,
 * so that in the end everything that should be grouped together has the same tag,
 * and things that are decorrelated have different tags.
 */
struct SysConfig
{
  /// Types of treatment to be applied on hist systs: skip norm, shape, shapeonly
  enum class Treat { skip, norm, shape, shapeonly };

  /// @brief Type of smoothing to apply on histo systs.
  ///
  /// None, or potentially one among several
  /// algs. The enum just needs to be extended to accomodate additional algs.
  enum class Smooth { noSmooth, smooth, smooth2 };

  /// Type of treatment to apply
  Treat treat;

  /// Type of smoothign to apply
  Smooth doSmooth;

  /**
   * @brief Samples that the systematic applies on
   *
   * Both sample names and keywords as defined in @c SampleHandler can be used
   */
  std::vector<TString> sampleNames;

  /**
   * @brief Categories the systematic applies on
   *
   * The way to make the list of affected categories is to use a vector of
   * @c PropertiesSet, that are used asa matching rules.
   *
   * So a category is matched if its properties match at least one of the @c PropertiesSet
   * in this vector
   *
   * e.g if one defines @c categories with
   *
   *     { {nTag, 2}, { { {nLep, 2}, {nJet, 3} }, {} } }
   *
   * then all 2-tag categories will be used, as well as all 2 lepton 3 jet categories.
   *
   */
  std::vector<PropertiesSet> categories;

  /**
   * @brief Simple decorrelation rules for categories
   *
   * Here one can define simple rules like: I want this systematic to be decorrelated
   * in number of jets, number of leptons, and tag type
   *
   *     {nLep, nJet, tagType}
   *
   * The resulting tag will be e.g for all 0 lepton 3 jet ll tag region:
   *
   *     _L0_J3_TTypell
   *
   */
  std::vector<Property> decorrelations;

  /**
   * @brief More complex decorrelation rules for categories
   *
   * Sometime one needs something a bit more complex than the above simple decorrelation
   * schemes. In that case one must define the precise regions that should be on their own.
   *
   * They way to describe precisely the affected regions is through @c PropertiesSet
   *
   * An example is ttbar 3jet systematic, which is correlated in 0 and 1 lepton, but
   * decorrelated in 2 lepton.
   *
   *     { {nLep, 2} }
   *
   * Then all 0 and 1 lepton categories won't have any tag, while all 2 lepton categories
   * will have a _L2 tag
   */
  std::vector<PropertiesSet> cplxDecorrelations;

  /**
   * @brief Simple decorrelation rules for samples
   *
   * This simply works through sample names or keywords as definde in @c SampleHandler.
   *
   * An example is SysJetFlavComp, which should be decorrelated based on sample type:
   *
   *     {"Top", "Zjets", "Wjets", "VHVV"}
   *
   * All samples with Zjet keyword attached will have a _Zjets tag appended to the syst name,
   * and similarly for others.
   *
   * If a sample does not match any keyword in this vector, it simply won't have any tag appended.
   */
  std::vector<TString> sampleDecorrelations;

  /**
   * @brief Completely free decorrelation for desperate cases
   *
   * Ultimate freedom for the most complex schemes. One can pass a function to write tags based
   * on a @c Sample and a @c PropertiesSet. Typically one will write lambdas in the constructor.
   *
   * Example: SysWMbb. For Wbb and Wcc only, one wants to decorrelate pTV bins. Plus, in cut-based,
   * bins 2, 3 and 4 should be merged together
   *
   *                    [](const PropertiesSet& pset, const Sample& s) {
   *                      if(! s.hasKW("WbbORcc")) return "";
   *                      if(pset[P::bin] == 0) return "_B0";
   *                      if(pset[P::bin] == 1) return "_B1";
   *                      return "";
   *                    }
   *
   */
  std::function<TString(const PropertiesSet&, const Sample&)> freeDecorrelation;

  // and now for dozens of constructors

  /// @name Constructors
  /// @{

  /// Default copy constructor
  SysConfig(const SysConfig&) = default;

  /// Default move constructor
  SysConfig(SysConfig&&) = default;


  /// @name Constructors useful mostly for histo systs
  /// @{

  /// Full constructor
  SysConfig(Treat t = Treat::skip, Smooth sm = Smooth::noSmooth, std::vector<TString>&& s = {},
            std::vector<PropertiesSet>&& c = {}, std::vector<Property>&& d = {},
            std::vector<PropertiesSet>&& cd = {}, std::vector<TString>&& sd = {},
            std::function<TString(const PropertiesSet&, const Sample&)>&& fun = nullptr);

  /// Shorthand with only 1 sample name or keyword to define the affected samples
  SysConfig(Treat t, Smooth sm, const TString& s, std::vector<PropertiesSet>&& c = {},
            std::vector<Property>&& d = {}, std::vector<PropertiesSet>&& cd = {},
            std::vector<TString>&& sd = {},
            std::function<TString(const PropertiesSet&, const Sample&)>&& fun = nullptr);

  /// Shorthand to decorrelate years
  SysConfig(Treat t, Smooth sm, bool corr_years, std::vector<TString>&& s = {},
            std::vector<PropertiesSet>&& c = {}, std::vector<TString>&& sd = {},
            std::function<TString(const PropertiesSet&, const Sample&)>&& fun = nullptr);

  //SysConfig(Treat t, Smooth sm, bool corr_years, const TString& s,
  //std::vector<PropertiesSet>&& c = {}, std::vector<TString>&& sd = {},
  //std::function<TString(const PropertiesSet&)>&& fun = nullptr);

  /// @}

  /// @name Constructors useful mostly for user-defined systs
  /// @{

  /// Full constructor. Treat and Smooth are useless for user-defined systs.
  SysConfig(std::vector<TString>&& s, std::vector<PropertiesSet>&& c = {},
            std::vector<Property>&& d = {}, std::vector<PropertiesSet>&& cd = {},
            std::vector<TString>&& sd = {},
            std::function<TString(const PropertiesSet&, const Sample&)>&& fun = nullptr);

  /// Shorthand with only 1 sample name or keyword to define the affected samples
  SysConfig(const TString& s, std::vector<PropertiesSet>&& c = {},
            std::vector<Property>&& d = {}, std::vector<PropertiesSet>&& cd = {},
            std::vector<TString>&& sd = {},
            std::function<TString(const PropertiesSet&, const Sample&)>&& fun = nullptr);

  /// @}

  /// @}

  /**
   * @brief Compute the decorrelation tag corresponding to @c pset and @c s
   *
   * Computation of the decorrelation tag of the @c SampleInCategory characterized by
   * @c pset and @c s, based on the rules defined in this @c SysConfig.
   *
   * See the description of the class members for the details about how the decorrelation
   * tags are built.
   *
   * @param pset	A @c PropertiesSet typically coming from some @c Category
   * @param s A @c Sample
   * @return The decorrelation tag, that will have to be appended to a systematic name
   */
  TString getDecorrelationTag(const PropertiesSet& pset, const Sample& s) const;

};

/// Short version of @c SysConfig::Treat for use in other classes
using STreat = SysConfig::Treat;

/**
 * @brief Structure to hold configuration for 3-to-2 systematics
 *
 * Describe 3-to-2 systematics à la ttbar EPS 2013:
 * A systematic uncertainty is put on the 3/2 ratio, with 2+3 constant in each category.
 * So for every 2jet category, its exact correspondance in 3jet must exist.
 * Then the anticorrelated uncertainties in the 2jet and the 3jet categories are computed,
 * so that the uncetainty on the ratio matches the desired number.
 *
 */
struct Systematic32
{
  TString name; ///< Name of the systematic
  float size; ///< Size of the 3-to-2 systematic uncertainty
  SysConfig conf; ///< Configuration: where it applies, what should be decorrelated
};


#endif
