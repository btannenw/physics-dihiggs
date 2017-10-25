#ifndef inputshandler_paper_hpp
#define inputshandler_paper_hpp

#include <vector>
#include <TString.h>

#include "inputshandler.hpp"

class TH1;
struct PropertiesSet;
class Configuration;
class Sample;


/**
 * @brief Implementation of @c InputsHandler with conventions for VHbb Run1 paper
 *
 * This class implements the @c InputsHandler interface with the naming conventions
 * defined for the Run1 VHbb paper:
 *
 * * One input file per category
 * * So the base directory is the top-level of the file
 * * The names of histograms are the names of the samples (no suffixes)
 */
class InputsHandlerPaper : public InputsHandler
{
  protected:
    /// Empty constructor useable only by derived classes
    InputsHandlerPaper() = default;

    /**
     * @brief Set @c m_baseDir
     *
     * @c m_baseDir is directly @c m_file
     */
    virtual void findBaseDir();

    /**
     * @brief Determine the name of the @c TFile from a set of properties
     *
     * This function is called by the constructor to find the name of the @c TFile
     * that is attached to this instance.
     *
     * This function implements the naming conventions for input files after splitting
     * to determine the correct file name from a set of properties and a configuration
     * (which typically gives an input version number)
     *
     * @param conf A @c Configuration object for the analysis
     * @param pset A @c PropertiesSet that define the category which this InputsHandler
     * will be attached to.
     * @return The name of the @c TFile to attach to this @c InputsHandlerPaper
     */
    static TString forgeFileName(Configuration& conf, const PropertiesSet& pset);

    /**
     * @brief Check that data exists in the input file
     *
     * This is used to check that the category is not empty / exists at all.
     * Data histogram is named "data"
     *
     * @return @c true if a data histogram exists, @c false otherwise
     */
    virtual bool checkData();

  public:
    /**
     * @brief Standard constructor
     *
     * The name of the underlying @c TFile is obtained with @c forgeFileName.
     * m_baseDir is then set.
     *
     * @param conf A @c Configuration object for the analysis
     * @param pset A @c PropertiesSet that define the category which this InputsHandler
     * will be attached to.
     */
    InputsHandlerPaper(Configuration& conf, const PropertiesSet& pset);

    /**
     * @brief Destructor
     *
     * The underlying @c TFile is properly closed.
     */
    ~InputsHandlerPaper();

    /**
     * @brief List all samples (nominal histograms) for this category
     *
     * Sample names are the names of the histograms in the base directory.
     *
     * @return A list of sample names
     */
    virtual std::vector<TString> listSamples();

    /**
     * @brief Get the histogram for a given sample and a given systematic
     *
     * Histogram names are the names of the samples in most of the cases.
     * In the case of samples made of subsamples, we go through all of them and the
     * resulting histogram is the sum of all subsamples histograms.
     *
     * @c getHistByName is used internally to get the histograms once the names are known.
     *
     * @param sample Sample for which the histogram has to be found
     * @param systname Name of the systematic variation to retrieve, or empty for nominal.
     * @return Null pointer if the histo does not exist. A pointer to the histo otherwise.
     * The underlying @c TFile remains the owner of the returned histogram (to be checked)
     */
  virtual TH1* getHist(const Sample& sample, const TString& systname = "");
};



#endif
