#ifndef regionnamesparser_hpp
#define regionnamesparser_hpp

#include <utility>

#include <TString.h>

struct PropertiesSet;

/**
 * @brief Class specialized in naming conventions of the config file to declare analysis categories
 *
 * This class holds the logic to translate the definition of analysis regions in the config
 * files into sets of properties that characterize the categories.
 *
 * This is a pure virtual base class, that must be subclassed by the different analyses.
 */
class RegionNamesParser
{
  public:

    /// Default constructor
    RegionNamesParser() = default;

    /// Destructor
    virtual ~RegionNamesParser() = default;


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
    virtual std::pair<bool, PropertiesSet> parseRegion(const TString& regConfig) = 0;

};

#endif
