#ifndef binning_hpp
#define binning_hpp

#include <memory>
#include <vector>

#include "configuration.hpp"
#include "HistoTransform.h"

class Category;
class TH1;
class TH2;

/**
 * @brief Class to handle all rebinning operations
 *
 * This class is a singleton that is the place to handle all rebinning operations.
 * More specifically, it holds functions to:
 * * perform the rebinning in each analysis category, be it a simple @c TH1::Rebin,
 * or a complex rebinning. For the latter it owns a private instance of Daniel's
 * tool transformHisto.C
 * * Remove empty bins in e.g MV1c distributions (RooFit does not like empty bins)
 * * Perform the rebinning to smooth a specific systematic histogram
 *
 * This base class has all the technical stuff to actually compute and perform
 * rebinning, but holds no configuration for any category rebinning or change of
 * range. This has to be defined in the derived classes.
 *
 * For consistency rebinning is handled through a vector<int> bins for everyone.
 * By convention, this vector will hold only 1 number when a simple @ TH1::Rebin()
 * is going to be used.
 * By convention, this vector will be empty when no rebinning is to be applied.
 *
 */
class BinningTool
{
  protected:
    /// The instance of the singleton
    static std::unique_ptr<BinningTool> the_instance;

    /// Pointer to the @c Configuration of the analysis
    Configuration m_config;

    /// (Unique) Pointer to Daniel's class
    std::unique_ptr<HistoTransform> m_htrafo;

    /// No default constructor
    BinningTool() = delete;

    /**
     * @brief Default constructor with a @c Configuration object
     *
     * Set a number of internal parameters based on the configuration.
     *
     * @param conf The @c Configuration of the analysis
     */
    BinningTool(const Configuration& conf);

    /**
     * @brief Restrict the range of a histogram
     *
     * Change the x range of a histogram in-place. The under and overflows can be either
     * added in the first and last bin respectively, or ignored.
     *
     * @param histo	Pointer to the histo to modify
     * @param min Minimum x-value for the new histogram
     * @param max Maximum x-value for the new histogram
     * @param useOverflow Put overflow/underflow in last/first bin, or just ignore them
     */
    void changeRangeImpl(TH1* histo, float min=-1e8, float max=1e8, bool useOverflow = true);

    /**
     * @brief Compute rebinning of a distribution with empty bins collapsed into bins with content
     *
     * Used for MV1c distributions, which have 5 bins filled and 95 empty bins.
     * Necessary because RooFit does not like distributions with empty bins.
     *
     * @param c	The @c Category in which to compute the binning
     * @return A vector of bins numbers defining the new bin boundaries, in reverse order.
     */
    std::vector<int> collapseEmptyBins(const Category& c);

    /**
     * @brief Put negative bins of a distribution to 0
     *
     * @param h	Histogram of which negative bins should be set to 0
     */
    void emptyNegativeBins(TH1* h);

    /**
     * @brief Compute a ratio histogram with ratios computed over ranges of bins
     *
     * Auxiliary functions for getLocalExtremaBinning.
     * Returns a new TH1 which is the ratio of nom and sys.
     * It is user's reseponsibility to delete the returned TH1
     *
     * @param hnom	Denominator histogram ("Nominal")
     * @param hsys	Numerator histogram ("Systematic variation")
     * @param bins  Edges of bins that are merged to compute the ratios
     * @return The ratio histogram. It has the same number of bins as the input histograms, but
     * adjacent bins can have the same value. The number of different values is equal to
     * @c bins.size()-1
     */
    TH1* getRatioHist(TH1* hnom, TH1* hsys, const std::vector<int>& bins);

    /**
     * @brief Compute a ratio histogram with ratios computed over ranges of bins
     *
     * Auxiliary functions for getLocalExtremaBinning.
     * The histo @c res is filled with the ratio of sys over nom. It needs to have
     * the same number of bins as the input histograms
     *
     * @param hnom	Denominator histogram ("Nominal")
     * @param hsys	Numerator histogram ("Systematic variation")
     * @param bins  Edges of bins that are merged to compute the ratios
     * @param res   The ratio histogram. It has the same number of bins as the input histograms, but
     * adjacent bins can have the same value. The number of different values is equal to
     * @c bins.size()-1
     */
    void getRatioHist(TH1* hnom, TH1* hsys, const std::vector<int>& bins, TH1* res);

    /**
     * @brief Find local extrema in a histogram
     *
     * A local extrema is defined in bin @c i by either:
     * * h[i-1] <= h[i] > h[i+1]
     * * h[i-1] >= h[i] < h[i+1]
     * By convention bins 1 and @c GetNbinsX() are added in the result
     *
     * @todo micro-bug to fix (?) : will return {1, 1} for a 1-bin histogram
     *
     * @param h	The histogram in which to find the extrema
     * @return Bin numbers of the extrema
     */
    std::vector<int> findExtrema(TH1* h);

    /**
     * @brief Find the position of an extremum in a sys/nom ratio where the chi2
     * between nom and sys between this extremum and the next is smallest
     *
     * When smoothing systematics histograms, one 'merges' together bins between
     * consecutive extrema of the ratio. In order to chose what bins to merge,
     * we start where at the place of 'lower resistance', i.e where nominal and
     * systematic histograms are already the most similar. This is obtained by
     * computing chi2 between nominal and systematic in the ranges between consecutive
     * extrema (@c computeChi2), then chosing the place where the chi2 is lowest
     *
     * @param hnom	Denominator histogram ("Nominal")
     * @param hsys  Numerator histogram ("Systematic")
     * @param extrema A vector of extrema positions in @c hsys/hnorm ratio, that can
     * be obtained with @c findExtrema
     * @return The position in @c extrema for which the chi2 in the range
     * @c [extrema[pos], extrema[pos+1]] is smallest
     */
    int findSmallerChi2(TH1* hnom, TH1* hsys, const std::vector<int>& extrema);

    /**
     * @brief Compute a compatibility chi2 between two histograms in some range
     *
     * Compute a chi2 indicating how much the syst/nom ratio is compatible with a
     * flat ratio.
     *
     * First the mean ratio over the range is computed. Then the chi2 is evaluated
     * by summing over the bins the difference between the ratio and the mean ratio,
     * where the error considered is the BinError in the nominal histogram.
     *
     * @param hnom  Denominator histograms ("Nominal")
     * @param hsys  Numerator histograms ("Systematic")
     * @param beg First bin (inclusive) for the calculation
     * @param end   Last bin (inclusive) for the calculation
     * @return The chi2 value
     */
    float computeChi2(TH1* hnom, TH1* hsys, int beg, int end);

    /**
     * @brief Compute the relative error of a histogram over a range
     *
     * Return error / value computed over a chosen range.
     *
     * @param hnom The histogram on which the error is computed
     * @param beg First bin (inclusive) for the calculation
     * @param end Last bin (exclusive) for the calculation
     * @return The relative error over the range
     */
    float statError(TH1* hnom, int beg, int end);

    /**
     * @brief Remove values within a range in a sorted vector;
     *
     * In a sorted vector of values (typically bin numbers),
     * remove the values in the [lo, hi] range.
     *
     * @param lo Lower value to remove (inclusive)
     * @param hi Highest value to remove (inclusive)
     * @param bins Sorted vector of values
     */
    void mergeBins(int lo, int hi, std::vector<int>& bins);


  /**
   * @brief Return the bins from the bin edges
   *
   * Return the bins from the bins edges. If the edges of the histograms doesn't match, the code crashes.
   *
   * @param histo Pointer to the histo to modify
   * @param edges vector which contains in reverse order the edges of the bins ie of size n+1 bins 
   * @return A vector of bins numbers defining the new bin boundaries in reverse order.
   */
  std::vector<int> getBinsFromEdges(const TH1* histo, const std::vector<float>& edges) ;
  
  public:

    /// Destructor
    virtual ~BinningTool() = default;

    /**
     * @brief Return static pointer to the instance
     *
     * @return Pointer to the instance of @c BinningTool
     */
    static BinningTool& getInstance();

    /**
     * @brief Return static pointer to the instance after building it
     *
     * A similar function must be implemented by all derived subclasses.
     *
     * @param conf A @c Configuration object
     * @return Pointer to the instance of @c BinningTool
     */
    static void createInstance(const Configuration& conf);

    /**
     * @brief Change the range of a histogram belonging to a certain @c Category
     *
     * Do nothing by default. Should be implemented in derived classes.
     *
     * @param h	Histogram whose range can be restricted
     * @param c The @c Category it belongs to
     */
    virtual void changeRange(TH1* h, const Category& c);

    /**
     * @brief Change the range of a histogram belonging to a certain @c Category. Called after rebinning
     *
     * This function is similar to @c changeRange, but is typically called after a
     * rebinning has been performed.
     *
     * It does nothing by default. Should be implemented in derived classes.
     *
     * @param h	Histogram whose range can be restricted
     * @param c The @c Category it belongs to
     */
    virtual void changeRangeAfter(TH1* h, const Category& c);

    /**
     * @brief Main function to compute the rebinning used in a given category
     *
     * Depending on the configuration and on the category itself, this function can perform
     * various rebinnings.
     *
     * It does nothing by default. Should be implemented in derived classes.
     *
     * @param c Category in which to compute the rebinning
     * @return A vector of bins numbers defining the new bin boundaries, in reverse order.
     */
    virtual std::vector<int> getCategoryBinning(const Category& c);

    /**
     * @brief Compute the rebinning for 5% MC stat error for a given histo
     *
     * Simply calls transformHisto with the correct parameters to get the
     * rebinning vector for a 5% MC stat uncertainty per bin.
     *
     * This is typically called once per @c SampleInCategory.
     *
     * This smoothing algo is not in use any longer in VHbb
     *
     * @param h Histogram on which to compute the rebinning
     * @return A vector of bins numbers defining the bin boundaries, in reverse order.
     */
    std::vector<int> getBinningForSysts(const TH1* h);

    /**
     * @brief Compute the rebinning needed for a syst histo so that its ratio to
     * nominal has at most @c nmax local extrema, not counting first and last bins
     *
     * This function implements a rebinning algorithm so that the ratio of @c hsys
     * over @c hnom contains at most @c nmax local extrema. The local extrema are removed
     * iteratively starting at the places where the ratio is more compatible with a flat
     * one, which is done by computing a simple chi2.
     *
     * A second pass is then performed, to merge the remaining bins so that the MC stat
     * uncertainty is less than 5% in all the final bins after merging.
     *
     * @param hnom Denominator histogram ("Nominal")
     * @param hsys Numerator histogram ("Systematic")
     * @param nmax Maximum number of local extra allowed in the ratio, not counting
     * the first and last bins
     * @return A vector of bins numbers defining the bin boundaries, in reverse order.
     */
    std::vector<int> getLocalExtremaBinning(TH1* hnom, TH1* hsys, unsigned int nmax);

    /**
     * @brief Scale the size of a systematic by some factor
     *
     * This function can be used to e.g inflate a systematic variation by some amount,
     * like doubling the size of a systematic wrt nominal, or decrease it by half...
     *
     * @param hnom Nominal histogram
     * @param hsys Systematic histogram
     * @param factor Scale applied to the systematic
     */
    void scaleHistoSys(const TH1* hnom, TH1* hsys, float factor);

    /**
     * @brief Smooth a syst histogram using some rebinning
     *
     * This function performs the actual "smoothing" of a systematic histogram based on
     * some pre-defined rebinning scheme.
     *
     * The ratio histogram is computed for the binning defined with @c bins.
     * If @c smooth is @c true, an additional avering of neighbouring bins is performed
     * in addition.
     * Then the syst histogram is replaced by ratio times nominal, and its integral is
     * forced to be equal to its original one.
     *
     * @param hnom Nominal histogram
     * @param hsys Systematic histogram
     * @param bins Vector of bin edges for the rebinning used to smooth
     * @param smooth Whether to also apply a simple averaging on top of the rebinning
     */
    void smoothHisto(TH1* hnom, TH1* hsys, const std::vector<int>& bins, bool smooth);

    /**
     * @brief Performs the actual rebinning of a histogram based on a pre-defined
     * rebinning scheme
     *
     * The contents of the original histogram are replaced :
     * * if @c bins is of size 0, do nothing
     * * if @c bins is of size 1, apply simple @c Rebin
     * * else number of bins is bins.size()-1
     * * bin content is the integral of the original histo between the bin edges
     *
     * Important: Bins with negative content are set to 0
     *
     * @param h Histogram to rebin in-place
     * @param bins Vector of bin edges for the rebinning, given in reverse order
     */
    void applyBinning(TH1* h, const std::vector<int>& bins, TH2* corr = nullptr);

    /// Accessor for the use of complex rebinning scheme in mjj regions
    /// @todo generalize and improve
    virtual bool useMjjVariableRebin() const {return false;}

    /// Return binning for a single bin
    std::vector<int> oneBin(const Category& c);

    /// Make simple or non-equidisttant binning for `forceBinning`
    std::vector<int> makeForcedBinning(const Category& c, const std::vector<double>& user_bins);

};

#endif
