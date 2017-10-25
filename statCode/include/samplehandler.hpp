#ifndef samplehandler_hpp
#define samplehandler_hpp

#include <vector>
#include <unordered_map>
#include <set>
#include <map>
#include <memory>

#include <TString.h>

#include "sample.hpp"
#include "configuration.hpp"
#include "containerhelpers.hpp"

struct SysConfig;
struct Systematic;
struct Systematic32;
class Category;
class SamplesBuilder;

/**
 * @brief Class that manages all the samples in the analysis
 *
 * This class holds the samples used in the analysis.
 *
 * The actual creation of the samples is delegated to @c SamplesBuilder, that can be
 * subclassed by different analyses
 *
 * It also knows how to refer to them through keywords.
 *
 * Finally, it knows what samples should be merged, and has the high-level alg to merge them
 */
class SampleHandler
{
  private:
    /// Pointer to the global configuration of the analysis
    Configuration m_config;

    /// Storage of the samples, in maps indexed by their name
    std::unordered_map<TString, Sample> m_samples;

    /// All defined keywords, stored as a map of keyword -> list of sample names
    std::unordered_map<TString, std::set<TString>> m_keywords;

    /// Lists of samples to be merged, stored as a map of output sample -> input samples
    std::map<Sample*, std::vector<Sample*>> m_samplesToMerge;

    /// Pointer to SamplesBuilder
    std::unique_ptr<SamplesBuilder> m_builder;

    // the following is in private: only SampleHandler knows how to populate
    // itself, based on config file switches
    /**
     * @brief Builds the declared samples and add them to the list
     *
     * Delegates the creation of the samples to @c SamplesBuilder
     *
     * Then calls the filling of the keywords, then updates the global samples index.
     *
     * This method is private because only @c SampleHandler knows how to populate itself
     * based on the config file
     *
     */
    void populate();

    /**
     * @brief Fill @c m_keywords and add the keywords to the @c Sample themselves
     *
     * Analysis-depedent keywords must be declared in @c SamplesBuilder.
     * Here, generic keywords are filled: each sample can be referenced by his name,
     * as well as by his type
     *
     * The keywords are inserted as properties of the @c Sample
     * themselves
     */
    void fillKeywords();

    /**
     * @brief Fill the global @c SampleIndex finder with all keywords
     *
     * Go through all keywords and fill the @c SampleIndex with pairs of
     * @c Sample, @c keywords
     */
    void feedSampleIndex();

    /**
     * @brief Simply calls @c fillKeywords() and @c feedSampleIndex
     */
    void update();

    /**
     * @brief Internal function for merging some samples in a given category
     *
     * Do the merging of a bunch of samples in one category.
     * - First, find all samples from the list that are indeed present in the category
     * - Then create the merged @c SampleInCategory
     * - Assign the @c SampleInCategory to its parents
     * - Delete the individual original @c SampleInCategory and remove links to them everywhere
     *
     * @see Merging constructor of @c SampleInCategory for how the actual merging works
     *
     * @param cat The category in which the merging is done
     * @param target The big sample that is result of the merging
     * @param samplesToMerge The list of samples that will be merged
     */
    void mergeSamplesInCategory(Category& cat, Sample& target, const std::vector<Sample*>& samplesToMerge);

  public:
    /// No default contructor
    SampleHandler() = delete;

    /**
     * @brief Standard constructor
     *
     * Standard constructor with a copy of the @c Configuration.
     *
     * @param conf	The reference configuration
     */
    SampleHandler(const Configuration& conf);

    /// Destructor
    ~SampleHandler();

    /**
     * @brief Function called when all nominal histograms are loaded
     *
     * Currently does nothing except propagating the call to all samples.
     * Could be expanded to add some logging, collect statistics...
     *
     */
    void finalizeNominal();

    /**
     * @brief Apply a user-defined systematic on all samples that match
     *
     * Loop over all samples, and apply @c sys to all samples that match @c conf
     * @see SystematicHandler::match for description of how matching works
     *
     * @param sys	The @c Systematic to be applied
     * @param conf	The @c SysConfig describing the systematic
     * @return List of full systematic names (i.e with decorrelation tags) as they have
     * been added
     */
    std::set<TString> addUserSyst(const Systematic& sys, const SysConfig& conf);

    /**
     * @brief Apply a 3-to-2 systematic on all samples that match
     *
     * Loop over all samples, and apply @c sys32 to all samples that match its
     * associated configuration
     * @see SystematicHandler::match for description of how matching works
     *
     * @param sys32	The @c Systematic32 to be applied
     */
    void add32Syst(const Systematic32& sys32);

    /**
     * @brief Merge all samples declared for merging in a given category
     *
     * Simply loop over all merging cases that have been previously declared, and
     * call for the actual merging of these cases in the given category.
     *
     * @param cat	The @c Category in which the samples should be merged
     */
    void mergeSamplesInCategory(Category& cat);

    /**
     * @brief Remove samples that were subject to merging
     *
     * Function to be called when the merging of samples has been called in all
     * analysis categories.
     *
     * The samples that have been merged can now be safely deleted. The lists of keywords
     * and the @c SampleIndex are updated to reflect the changes.
     *
     */
    void finishMerging();

    /**
     * @brief Basic check of internal consistency
     *
     * Simply check that all defined samples have different names, and print
     * some information about them.
     *
     */
    void checkContent() const; // basic check of internal consistency

    /**
     * @brief Main function to declare the merging of samples
     *
     * The declaration is delegated to @c SamplesBuilder
     *
     * This is the main function that should be tuned to describe what samples
     * should be merged into what samples, based on the configuration. This function
     * is typically called towards the beginning of @c Engine::MakeIt
     *
     */
    void declareSamplesToMerge();

    /// @name Yay, syntaxic sugar to easily iterate over samples
    /// @{

    /// const iterator
    typedef std::unordered_map<TString, Sample>::const_iterator const_iterator;

    /// const begin
    const_iterator begin() const { return m_samples.begin(); }

    /// const end
    const_iterator end() const { return m_samples.end(); }

    /// iterator
    typedef std::unordered_map<TString, Sample>::iterator iterator;

    /// begin
    iterator begin() { return m_samples.begin(); }

    /// end
    iterator end() { return m_samples.end(); }

    /// @}
};

#endif
