#ifndef samplesbuilder_azh
#define samplesbuilder_azh

#include "samplesbuilder.hpp"
#include "samplesbuilder_vhbb.hpp"

/**
 * @brief Implementation of @c SamplesBuilder for the Run1 VHbb analysis
 *
 * We derive from SamplesBuilder_VHbb because we share almost everything !
 */
class SamplesBuilder_AZh : public SamplesBuilder_VHbb {
  public:
    /// Inherit the constructors
    using SamplesBuilder_VHbb::SamplesBuilder_VHbb;

    /// Destructor
    virtual ~SamplesBuilder_AZh() {}
   /**
     * @brief Stupid helper function to declare many common backgrounds
     *
     * W+jets, Z+jets, ttbar, single-top and multijet backgrounds are all
     * declared in one go
     */
    virtual void declareWZtopMJBkgs();
    /*
     * @brief Builds the declared samples and add them to the list
     *
     * We only change the signal (based on @c Configuration::analysisType() )
     * and the backgrounds (SM Higgs, diboson) compared to the VHbb analysis
     *
     * The switches read in the configuration file are:
     * @conf{MassPoint,125} Resonance mass to use
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
     * @conf{MassPoint,125} Resonance mass to use
     *
     * @see SamplesBuilder_VHbb
     */
    virtual void declareSamplesToMerge();

};

#endif
