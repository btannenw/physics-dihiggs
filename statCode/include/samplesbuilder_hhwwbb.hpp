#ifndef samplesbuilder_hhwwbb
#define samplesbuilder_hhwwbb

#include "samplesbuilder.hpp"
#include "samplesbuilder_vhbb.hpp"

/**
 * @brief Implementation of @c SamplesBuilder for the Run1 VHbb analysis
 *
 * We derive from SamplesBuilder_VHbb because we share almost everything !
 */
class SamplesBuilder_HHWWbb : public SamplesBuilder_VHbb {
public:
  /// Inherit the constructors
  using SamplesBuilder_VHbb::SamplesBuilder_VHbb;
  
  /// Destructor
  virtual ~SamplesBuilder_HHWWbb() {}

    /*
     * @brief Builds the declared samples and add them to the list
     *
     * We only change the signal (based on @c Configuration::analysisType() )
     * and the backgrounds (SM Higgs, diboson) compared to the VHbb analysis
     *
     * The switches read in the configuration file are:
     * - MassPoint: Resonance mass to use (default, 125)
     *
     * @see SamplesBuilder_VHbb
    */
    virtual void declareSamples();

    /**
     * @brief Fill @c m_keywords
     *
     * Only HVT and AZh are added
     *
     * @see SamplesBuilder_VHbb
     */
    virtual void declareKeywords();

    /**
     * @brief Main function to declare the merging of samples
     *
     * The switches read in the configuration file are:
     * - MassPoint: Resonance mass to use (default, 125)
     *
     * @see SamplesBuilder_VHbb
     */
    virtual void declareSamplesToMerge();

};

#endif
