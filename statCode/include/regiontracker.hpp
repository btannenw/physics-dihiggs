#ifndef RegionTracker_hpp
#define RegionTracker_hpp

#include <unordered_map>
#include <string>

class Category;

/**
 * @brief Class to hold high-level summary information about analysis regions
 *
 * This class can hold custom sets of information refering to what analysis
 * regions are present in the fit. This information can be used when setting up
 * the systematics. An example can be to allow Zbb to float if a 2 lepton region
 * is present, but to put a normalization prior if only 1 lepton regions are
 * used.
 *
 * The information stored is based on arbitrary keywords with an attached
 * boolean. This allows for maximal flexibility in use.
 *
 * It also has a function to set the configuration in a default, predetermined
 * state. This is useful for instance to create workspaces with other analysis
 * regions or distributions than usual, but with the standard fit configuration,
 * to allow a correct propagation of fit results in order to make post-fit plots.
 *
 * This is a pure virtual base class, that all analyses must implement.
 */
class RegionTracker
{
  protected:
    /// Storage of the information. Map of yes/no properties.
    std::unordered_map<std::string, bool> m_bits;

  public:
    /// Default constructor
    RegionTracker() = default;

    /// Default destructor
    virtual ~RegionTracker() = default;

    /// Const accessor to a property
    bool operator[](const std::string& prop) const;

    /// Accessor to a property (can be used to modify it)
    bool& operator[](const std::string& prop);

    /**
     * @brief Extract information from a @c Category
     *
     * This function is called once per category in @c CategoryHandler.
     * Analyses must implement this function to tell what information to
     * extract from the categories
     *
     * @param c The @c Category to extract information from
     */
    virtual void trackCategory(const Category& c) = 0;

    /**
     * @brief Set all properties to predefined values
     *
     * Can be used by the analyses to setup systematics in a default state.
     */
    virtual void setDefaultFitConfig() = 0;

};

#endif
