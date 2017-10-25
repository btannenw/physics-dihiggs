#ifndef samplesbuilder
#define samplesbuilder

#include <vector>
#include <unordered_map>
#include <set>
#include <map>

#include "configuration.hpp"

class Sample;
class TString;

/**
 * @brief Class that builds all the samples in the analysis
 *
 * This class acts as a base delegate of @c SampleHandler to do the actual building
 * of samples, as well as declaring which ones to merge.
 *
 * It also allows the declaration of user-defined keywords to refer to groups of samples.
 */
class SamplesBuilder
{

  protected:
    /// Pointer to the global configuration of the analysis
    Configuration m_config;

    /// Reference to storage of the samples, in maps indexed by their name
    std::unordered_map<TString, Sample>& m_samples;

    /// Reference to all defined keywords, stored as a map of keyword -> list of sample names
    std::unordered_map<TString, std::set<TString>>& m_keywords;

    /// Reference to lists of samples to be merged, stored as a map of output sample -> input samples
    std::map<Sample*, std::vector<Sample*>>& m_samplesToMerge;

    /**
     * @brief Stupid helper function to push the @c Sample in the storage
     *
     * @param sample	The @c Sample to put into @c m_samples
     */
    void addSample(Sample&& sample);

    /**
     * @brief Stupid helper function to add several @c Sample as @c Bkg in one go
     *
     * @param bkgs Map of @c sample_name -> @c sample_color
     */
    void addBkgs(const std::unordered_map<TString, int>& bkgs); // convenience function. Syntaxic sugar is good.

    /**
     * @brief Simple helper function to add data
     *
     * Switches read in the configuration file:
     * @conf{UsePseudoData,false} use pseudo-data replica instead of real data
     * @conf{ReplicaCount,0} Index of the replica to use
     */
    void addData();

    /**
     * @brief Declare the merging of a bunch of samples into a new one
     *
     * Declare that a list of samples should be merged into a new one.
     *
     * The new sample is typically built in-place (it is moved). The samples to be
     * merged can be referred to either by name, or by any keyword. So shortcuts
     * like "Higgs" can be used for instance.
     *
     * @param newS	The output, new sample (moved)
     * @param names The names / keywords describing the samples to merge
     */
    void declareMerging(Sample&& newS, const std::vector<TString>& names);

  public:

    /// No default constructor
    SamplesBuilder() = delete;

    /**
     * @brief Standard constructor
     *
     * Standard constructor with a copy of the @c Configuration.
     *
     * @param conf	The reference configuration
     * @param samples Reference to the map of samples
     * @param keywords Reference to the map of keywords
     * @param samplesToMerge Reference to the map of samples to be merged together
     *
     */
    SamplesBuilder(const Configuration& conf, std::unordered_map<TString, Sample>& samples,
                  std::unordered_map<TString, std::set<TString>>& keywords,
                  std::map<Sample*, std::vector<Sample*>>& samplesToMerge);

    /// Destructor
    virtual ~SamplesBuilder();

    /**
     * @brief Builds the declared samples and add them to the list
     *
     * This is called from @c SampleHandler
     *
     * The configuration is read, and based on it samples are built for the analysis with
     * the correct type (signal, background, etc...)
     *
     * This must be implemented by the subclasses
     *
     */
    virtual void declareSamples() = 0;

    /**
     * @brief Fill @c m_keywords
     *
     * Here one must put all the keywords that are then used to easily refer to groups
     * of samples.
     */
    virtual void declareKeywords() = 0;

    /**
     * @brief Main function to declare the merging of samples
     *
     * This is the main function that should be tuned to describe what samples
     * should be merged into what samples, based on the configuration.
     *
     * This is called from @c SampleHandler
     */
    virtual void declareSamplesToMerge() = 0;

};


#endif
