#ifndef binning_azh
#define binning_azh

#include "binning.hpp"

#include <vector>

class Category;
class Configuration;
class TH1;

/**
 * @brief Implementation of Binning for Run2 VH resonances searches
 *
 * It holds all the specific configuration to rebin the individual analysis regions,
 * and change the ranges of histograms.
 *
 */
class BinningTool_AZh : public BinningTool
{

  protected:

    /// No default constructor
    BinningTool_AZh() = delete;

    /**
     * @brief Default constructor with a @c Configuration object
     *
     * Set a number of internal parameters based on the configuration.
     *
     * @param conf The @c Configuration of the analysis
     */
    BinningTool_AZh(const Configuration& conf);

  public:

    /// Destructor
    virtual ~BinningTool_AZh() = default;

    /**
     * @brief Return static pointer to the instance after building it
     *
     * @param conf A @c Configuration object
     * @return Pointer to the instance of @c BinningTool
     */
    static void createInstance(const Configuration& conf);


    /**
     * @brief Main function to compute the rebinning used in a given category
     *
     * By default, do a flat 20 GeV rebinning for everyone.
     *
     * @param c Category in which to compute the rebinning
     * @return A vector of bins numbers defining the new bin boundaries, in reverse order.
     */
    virtual std::vector<int> getCategoryBinning(const Category& c);

  /**
   * @brief Change the range of a histogram belonging to a certain @c Category
   *
   * This function restricts the range of @c histograms to [200 - 1000 ] GeV
   *
   * @param h Histogram whose range can be restricted
   * @param c The @c Category it belongs to
   */

  void changeRange(TH1* h, const Category& c);

};

#endif
