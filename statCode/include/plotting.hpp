#ifndef plotting_hpp
#define plotting_hpp

#include <vector>
#include <fstream>
#include <TString.h>

#include "configuration.hpp"

class CategoryHandler;
class Category;
class TH1;
class TH2;
class TLegend;

/**
 * @brief Manage control plots created with the workspaces
 *
 * This class is responsible to draw different types of control plots
 * that are created when making the workspaces.
 *
 * Some of them are drawn for each category individually:
 * * Standard stack plot
 * * MC stat uncertainty on each MC and on the sum
 * * S/B for each B and for the sum
 * * DLLR (TODO)
 * * Fractions (TODO)
 * * Write small table with yields
 *
 * There are also control plots for the systematics:
 * * Plots of all the shape systematics after smoothing (before smoothing superimposed)
 * * Summary plots of the systematics:
 *   * Reason for pruning for shape and norm parts separately
 *   * Mean size of norm part
 *   * Chi^2/NDF comparing before/after smoothing. Enable with "SmoothingControlPlots"=true
 */
class Plotting
{
  private:
    /// Pointer to the global configuration of the analysis
    Configuration m_conf;

    /// Flag used to get correct blinding of Mjj distributions
    bool m_specialMjjBlinding;

    /// Format in which plots are saved: png, eps...
    TString m_format;

    /// File used to print event yields
    std::ofstream* m_evtFile;

    /// @name The following should be in a struct passed from function to function
    /// @{

    /// Current data histogram
    TH1* m_hData;

    /// Current data histogram, blinded
    TH1* m_hDataBlind;

    /// Current signal histogram (sum)
    TH1* m_hSig;

    /// Current backgrounds
    std::vector<TH1*> m_hBkgs;

    /// Stacked backgrounds (last is full sum)
    std::vector<TH1*> m_hStack;

    /// Current signals
    std::vector<TH1*> m_hSignals;

    /// @}

    /// Count the total number of bins in the analysis
    int m_binCount;

  public:
    /// No default constructor
    Plotting() = delete;

    /**
     * @brief Standard constructor
     *
     * Standard constructor with a copy of the @c Configuration.
     *
     * @conf{PlotsFormat, eps} format used to save the plots
     *
     * @param conf	The reference configuration
     */
    Plotting(const Configuration& conf);

    /// Destructor
    ~Plotting();

    /**
     * @brief Initialize stuff to make category plots
     *
     * Creates directories for plots and open the file to write out yields
     *
     * Makes use of the @c OutputConfig to create the directories in the right place
     *
     * This function must be called before calling any other function that
     * actually makes plots.
     */
    void prepare();

    /**
     * @brief Close file with event yields
     *
     * This function must be called after all calls to @c makeCategoryPlots
     */
    void finish();

    /**
     * @brief Main function to make control plots for a given category
     *
     * The calls @c readHistosFromCat, then successively the function to write the
     * event yields and make the different types of plots for a given category.
     *
     * @param c	Category to make plots of
     */
    void makeCategoryPlots(Category& c);

    /**
     * @brief Make pTV plot of yields for a set of similar categories
     *
     * @todo This function is empty at the moment
     *
     * @param c	Similar categories used to make the plot
     */
    void makeVpTbinsPlot(std::vector<Category*> c) {}

    /**
     * @brief Make plots of status of systematics for a given category
     *
     * This makes the plots of status of norm and shape systematics, as well
     * as mean value of norm systematics.
     *
     * 2D plots of (systematic, sample) are made. Given the number of systematics,
     * the plots are split in Jet/MET, BTag, other.
     *
     * @param c	Category to make plots of
     */
    void makeSystStatusPlots(const Category& c) const;

    /**
     * @brief Make plots of status of systematics for the full analysis
     *
     * This makes the plots of status of norm and shape systematics, as well
     * as mean value of norm systematics.
     *
     * This function takes the full @c CategoryHandler because it creates one set of
     * plots per systematic, with the 2D plots having axes (category, sample)
     *
     * @param handler @c CategoryHandler of the analysis
     */
    void makeOverallSystStatusPlots(const CategoryHandler& handler) const;
  
  /**
   * @brief Make plots of MC stat relative error (GetBinError/GetBinContent)
   *
   * Make plots showing the relative MC stat error per bin 
   *
   * @param c Category to make plots of
   */
  void makeMCStatRelErrorPlots(const Category& c);

    /**
     * @brief Make plots of shape systematics
     *
     * Make plots showing the effect of shape systematics for a given category.
     *
     * The mean norm impact is also printed on the plot.
     *
     * @conf{OverlayNominalForSystShapes, true} overlay the nominal distribution
     *
     * @param c	Category to make plots of
     */
    void makeSystShapePlots(const Category& c) const;

  private:

    /**
     * @brief Fill member variables with histos for current category
     *
     * Get the histograms out of the current category and fill member
     * variables with them.
     *
     * Colors ans names are set.
     *
     * Blinding of data is commented out at the moment !
     *
     * @param c	Category to make plots of
     */
    void readHistosFromCat(Category& c);

    /**
     * @brief Delete histo copies in the member variables
     *
     * This is called after all types of category histos have been made
     */
    void deleteHistos();

    /**
     * @brief Write event yields in the output file
     *
     * Write event yields for the current category in the common output text file
     * Total background is also written, as well as data/MC ratio
     *
     * @param c	Category to write yields of
     */
    void writeYields(Category& c);

    /**
     * @brief Make the standard stack plot for the current category
     *
     * Make standard stack plot. Blinding is still controled somewhow by hand.
     *
     * @conf{LogScalePlots, false} display the plots in log scale
     *
     * @param c Category to make the plot of
     */
    void makeStackPlot(Category& c);

    /**
     * @brief Make the plot of MC stat for the current category
     *
     * Show MC stat uncertainty for all backgrounds individually and for the sum
     *
     * @param c Category to make the plot of
     */
    void makeStatPlot(Category& c);

    /**
     * @brief Make the plot of S/B for the current category
     *
     * Show S/B for all backgrounds individually and for the sum
     *
     * @param c Category to make the plot of
     */
    void makeSoBPlot(Category& c);

    /**
     * @brief Make the plot of DLLR for the current category
     *
     * @todo This function is empty at the moment
     *
     * @param c Category to make the plot of
     */
    void makeDllrPlot(Category& c) {}

    /**
     * @brief Make the plot of fractions of the total for the current category
     *
     * @todo This function is empty at the moment
     *
     * @param c Category to make the plot of
     */
    void makeFractionsPlot(Category& c) {}

    /**
     * @brief Set decent properties for a legend
     *
     * Set colors, borders, etc... for a legend
     *
     * @param leg Legend to beautify
     */
    void setLegendStyle(TLegend* leg);

    /**
     * @brief Split the summary plots of systematics in Jet/MET, BTag, and other
     *
     * Given the number of systematics, summary plots for a given category are split
     * in Jet/MET, BTag, and other.
     *
     * This is achieved through simple calls to @c PlotUtils functions.
     *
     * @param h	One of the summary plots
     * @return A set of subplots
     */
    std::vector<TH1*> split2Dplot(const TH2* h) const;
};

#endif
