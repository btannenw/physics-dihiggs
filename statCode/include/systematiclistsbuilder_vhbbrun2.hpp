#ifndef systematiclistsbuilder_vhbbrun2_hpp
#define systematiclistsbuilder_vhbbrun2_hpp

#include "systematiclistsbuilder.hpp"

class RegionTracker;

/**
 * @brief Implementation of @c SystematicListsBuilder for the Run2 HVT/AZh analyses
 *
 */
class SystematicListsBuilder_VHbbRun2 : public SystematicListsBuilder
{
  public:

    /// Inherit the constructors
    using SystematicListsBuilder::SystematicListsBuilder;

    /// Empty destructor
    virtual ~SystematicListsBuilder_VHbbRun2() {}

    /**
     * @brief Fill the map of renaming rules for systematics names
     *
     * This is typically used to correct for bugs in syst names in some inputs
     */
    virtual void fillHistoSystsRenaming();

    // list them all !
    /**
     * @brief Populate the list of user-defined systematics
     *
     * At the moment, store a stripped-down copy of the VHbb Run1 function
     *
     * Configuration flags used:
     * @conf{DoDiboson,false} use @c WZ+ZZ as signal
     * @conf{DecorrPOI,""} decorrelate POI using the provided comma-separated list of Property names
     * @conf{DoInjection,0} mass of the signal to inject
     * @conf{MassPoint,125} Higgs mass considered
     *
     * @see @c SysConfig for the possibilities to configure systematics
     *
     * @param regTrk	Object tracking what type of regions are present in the analysis
     * @param useFltNorms Whether some backgrounds should be let free, or be constrained to a prior
     */
    virtual void listAllUserSystematics(const RegionTracker& regTrk, bool useFltNorms);

    /**
     * @brief Populate the list of expected histo-based systematics
     *
     * Empty at the moment
     *
     * @see @c SysConfig for the possibilities to configure systematics
     *
     * @param regTrk	Object tracking what type of regions are present in the analysis
     */
    virtual void listAllHistoSystematics(const RegionTracker& regTrk);

};

#endif
