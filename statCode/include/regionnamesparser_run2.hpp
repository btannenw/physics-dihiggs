#ifndef regionnamesparser_run2
#define regionnamesparser_run2

#include "regionnamesparser.hpp"
#include <utility>
#include <TString.h>
#include <TPRegexp.h>

struct PropertiesSet;

/**
 * @brief Implementation of config naming conventions for Run2 analysis
 *
 * It relies on the naming conventions defined for the HVT/AZh analyses
 */
class RegionNamesParser_Run2 : public RegionNamesParser
{

  private:
    /// Regexp describing the expected names of regions (order of blocks)
    TPRegexp m_regionRegexp;

    /// Regexp describing the 'tag' block of the naming convention
    TPRegexp m_tagRegexp;

    /// Regexp describing the 'jet' block of the naming convention
    TPRegexp m_jetRegexp;

    /// Regexp describing the 'descr' block of the naming convention
    TPRegexp m_descrRegexp;

  public:

    /// Constructor
    RegionNamesParser_Run2();

    /// Default destructor
    virtual ~RegionNamesParser_Run2() = default;

    /**
     * @brief Creates a @c PropertiesSet for the given @c regConfig line
     *
     * Parsing @c regConfig allows to set the number of jets, the number of tags, the tag type,
     * special flags, etc...
     *
     * @param regConfig The configuration line describing a region
     * @return A pair: @c false, empty @c PropertiesSet if the region is not valid ;
     * @c true, @c PropertiesSet filled with the parsed properties if the region is valid.
     */
    virtual std::pair<bool, PropertiesSet> parseRegion(const TString& regConfig);

};

#endif
