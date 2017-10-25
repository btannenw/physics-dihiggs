#ifndef regiontracker_azh_hpp
#define regiontracker_azh_hpp

#include "regiontracker.hpp"

#include <TString.h>

class Category;

/**
 * @brief Implementation of the @c RegionTracker for the AZh analysis
 *
 */
class RegionTracker_AZh : public RegionTracker
{

  public:
    /// Default constructor
    RegionTracker_AZh() = default;

    /// Default destructor
    virtual ~RegionTracker_AZh() = default;

    /**
     * @brief Extract information from a @c Category
     *
     * The following properties are used:
     * * @c hasZeroLep, @c hasOneLep, @c hasTwoLep
     * * @c has2jet, @c has3jet
     * * @c has0tag, @c has1tag, @c has2tag
     *
     * @param c The @c Category to extract information from
     */
    virtual void trackCategory(const Category& c);

    /**
     * @brief Set all properties to predefined values
     *
     * Sets 0+1+2 lepton, 2+3 jets, 1+2 tags.
     *
     */
    virtual void setDefaultFitConfig();

  protected:
    /**
     * @brief Convenience function to add properties in @c m_bits
     *
     * @param nlep number of leptons in current category
     * @param njet number of jets in current category
     * @param ntag number of tags in current category
     * @param topcr: is this the top control region?
     * @param mergedCat: true is the merged-regime is included in the analysis
     * @param mBBcr: true if mBB sidebands are included in the fit
     * @param spec: any special flags
     */
  void addInfo(int nlep, int njet, int ntag, bool topcr, bool mergedCat, bool mBBcr, const TString& spec = "");

};

#endif
