#ifndef finder_hpp
#define finder_hpp

#include <unordered_map>
#include <map>
#include <set>

#include <TString.h>

#include <sample.hpp>

struct PropertiesSet;

class Category;

/**
 * @file finder.hpp
 * @brief Finder: two small classes that index existing samples and categories
 *
 * The implementation uses classes with only static members and methods...
 * Is this the correct way ? Alternatives could be namespaces, or singleton classes
 *
 */

/**
 * @brief An index of all existing samples
 *
 * An index that can be globally accessible, to quickly find samples that match
 * some keywords.
 *
 * Typical use is in @c SystematicHandler to assign systematics to specific samples
 */
class SampleIndex
{
  private:
    /// Map linking sample names to samples, and the keywords to all relevant samples
    static std::unordered_map<TString, std::set<Sample*> > m_samples;
    /// Map linking the types of samples to all relevant samples
    static std::map<SType, std::set<Sample*> > m_samplesByType;

  public:
    /// Clear all maps
    static void reset();

    /**
     * @brief Add a sample in the index
     *
     * For all keyword in @c kw, add @c s to the keyword entry in @c m_samples.
     * Add @c s to the entry of @c s->type() in @c m_samplesByType
     *
     * @param s	The @c Sample to add
     * @param kw A set containing the name of @c s and all keywords attached to it
     */
    static void feed(Sample* s, const std::set<TString>& kw);

    /**
     * @brief Find a given sample through its name
     *
     * @param k	The name of the sample to find.
     * @return A pointer to the sample if found. @c nullptr if nothing matches or if several
     * matches are found.
     */
    static Sample* findOrNull(const TString& k);  // never throws, but returns nullptr

    /**
     * @brief Find a given sample through its name
     *
     * @param k	The name of the sample to find.
     * @return A pointer to the sample if found. Throws an exception if nothing matches 
     * or if several matches are found.
     */
    static Sample* find(const TString& k);        // throw if k not valid

    /**
     * @brief Find all samples matched by name/keyword
     *
     * @param k	The name/keyword describing the samples to find.
     * @return A set of pointers of @c Sample matching @c k. Throws an exception if nothing
     * matches
     */
    /// @todo make a non-throwing version of findAll, returning an empty set ?
    static const std::set<Sample*>& findAll(const TString& k);

    /**
     * @brief Find all samples of a given type
     *
     * @param t	The type of samples to retrieve
     * @return A set of pointers of @c Sample of type @c t.
     */
    static const std::set<Sample*>& findAll(SType t) { return m_samplesByType.at(t); }

    /**
     * @brief Check existence of samples of a given type
     *
     * @param t	The type of samples to retrieve
     * @return @c true if there are samples of this type, @c false otherwise
     */
    static bool has(SType t) { return m_samplesByType.count(t); }
};

/**
 * @brief An index of all existing categories
 *
 * An index that can be globally accessible, to quickly find a precise category based on its
 * properties.
 *
 * The categories are indexed by their name.
 *
 * Use-case can be to find a 3 jet region corresponding to a given 2 jet region
 */
class CategoryIndex
{
  private:
    /// Map linking names of categories to the categories themselves
    static std::unordered_map<TString, Category*> m_categories;

  public:
    /// Add a @c Category to the index
    static void feed(Category* c);

    /**
     * @brief Find a @c Category based on its set of properties
     *
     * This method is not very robust if the @c pset does not match anything
     *
     * @param pset	The set of properties describing the @c Category to find
     * @return A pointer to the @c Category.
     */
    static Category* find(const PropertiesSet& pset);
};

#endif
