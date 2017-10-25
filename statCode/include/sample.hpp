#ifndef sample_hpp
#define sample_hpp

#include <unordered_map>
#include <vector>
#include <set>

#include <TString.h>

#include "containerhelpers.hpp"

class Category;
class SampleInCategory;
struct Systematic;
struct Systematic32;
struct SysConfig;

/**
 * @brief Class that describes a sample of the analysis
 *
 * The class describes a sample of the analysis.
 *
 * A sample is characterized by a name, a type, a color (for plotting),
 * and a list of associated keywords that refer to it.
 *
 * A sample holds pointers to associated @c SampleInCategory, indexed by the
 * categories. It is NOT the owner of the @c SampleInCategory objects.
 *
 * A sample can be made of several subsamples. The concept of subsamples is
 * present mostly for historical reasons and is not very well tested.
 *
 * When a sample is made of subsamples, basically everytime a histogram has
 * to be fetched, the histos from all the subsamples are fetched and added
 * together.
 *
 * The difference with the concept of merging samples as explained in
 * @c SampleHandler is that here the histos are summed before any smoothing
 * or pruning of systematics, while the merging of samples occurs after this
 * treatment has been applied on the individual samples.
 *
 * The concept of merging samples is preferred, because it provides the same
 * fit model mathematically speaking in merged and unmerged modes.
 *
 */
class Sample
{
  public:
    /**
     * @brief Possible types for a sample
     *
     * A sample can be data, data-driven (multijet), signal, or background.
     * A keyword matching the type of a sample is always present, so that
     * one can always easily refer to e.g all signal samples.
     */
    enum class Type : std::int8_t { None, Data, DataDriven, Sig, Bkg };

  private:
    /// The name of the sample
    TString m_name;

    /// The list of keywords that refer to this sample
    std::set<TString> m_keywords;

    /// The type fo the sample
    Type m_type;

    /// The color in which the sample will be displayed in plots
    int m_color;

    /// The list of subsamples this sample is made of
    std::vector<TString> m_subsamples;

    /// Pointers to @c SampleInCategory that are children of this sample
    std::unordered_map<Category*, SampleInCategory*> m_categories;

  public:
    /// No default constructor
    Sample() = delete;

    /**
     * @brief Simple constructor with a name, a type, and a color
     *
     * @param name the name of the sample
     * @param type the type of the sample
     * @param color the color of the sample
     */
    Sample(const TString& name, const Type type, const int color);

    /**
     * @brief Constructor in case of subsamples
     *
     * @param name the name of the sample
     * @param subsamples the list of subsamples this one is made of
     * @param type the type of the sample
     * @param color the color of the sample
     */
    Sample(const TString& name, std::vector<TString>&& subsamples, const Type type, const int color);

    /// Default destructor
    ~Sample() {}

    /**
     * @brief Equality operator
     *
     * Equality is simply tested with the names of the samples.
     *
     * @param other	the sample to be tested against
     * @return @c true if samples have the same name. @c false otherwise
     */
    bool operator==(const Sample& other);

    /// Accessor to the name
    inline const TString& name() const { return m_name; }

    /// Accessor to the keywords
    const std::set<TString>& keywords() const { return m_keywords; }

    /**
     * @brief Test if this sample has a given keyword attached
     *
     * @param kw	the keyword to look for
     * @return @c true if @c kw is a keyword attached to @c this
     */
    inline bool hasKW(const TString& kw) const { return m_keywords.count(kw); }

    /**
     * @brief Append a keyword to this sample
     *
     * @param kw	The keyword to add to the list
     */
    inline void addKW(const TString& kw) { m_keywords.insert(kw); }

    /// Accessor to the type
    inline Type type() const { return m_type; }

    /// Accessor to the color
    inline int color() const { return m_color; }

    /// Check if this sample is made of subsamples
    inline bool hasSubsamples() const { return ! m_subsamples.empty(); }

    /// Accessor to the list of subsamples
    inline std::vector<TString>& subsamples() { return m_subsamples; }

    /// const accessor to the list of subsamples
    inline const std::vector<TString>& subsamples() const { return m_subsamples; }

    /**
     * @brief Function called after the nominal histos have all be read
     *
     * This function is currently empty. Could be used for logging purposes,
     * of to display some useful statistics to the user.
     *
     */
    void finalizeNominal();

    /// Accessor to the map storing the pointers of the children @c SampleInCategory
    inline std::unordered_map<Category*, SampleInCategory*>& categories() { return m_categories; }

    /// const accessor to the map storing the pointers of the children @c SampleInCategory
    inline const std::unordered_map<Category*, SampleInCategory*>& categories() const { return m_categories; }

    /**
     * @brief Add a user-defined systematic to this sample
     *
     * Loop over categories, and add the systematic with the correct
     * decorrelation tags for the categories that match the given
     * configuration.
     *
     * @param sys	The systematic to add
     * @param conf The configuration for this systematic
     * @return List of full systematic names (i.e with decorrelation tags) as they have
     * been added
     */
    std::set<TString> addUserSyst(const Systematic& sys, const SysConfig& conf);

    /**
     * @brief Add a 3-to-2 systematic to this sample
     *
     * Find all 3-jet categories that match the configuration associated
     * to @c sys32, then find the corresponding 2-jet categories.
     * Then call @c SystematicHandker::applyRatioSys to apply a systematic of the
     * correct size to the 2 jet and 3 jet regions.
     *
     * @param sys32	The @c Systematic32 to add
     */
    void add32Syst(const Systematic32& sys32);

};

/// Simple alias of sample type for easier use
using SType = Sample::Type;

#endif
