#ifndef inputshandler_run2_hpp
#define inputshandler_run2_hpp

#include <vector>
#include <TString.h>

#include "inputshandler.hpp"
#include "inputshandler_paper.hpp"

class TH1;
struct PropertiesSet;
class Configuration;
class Sample;


/**
 * @brief Implementation of @c InputsHandler with conventions for VHbb/AZh/HVT Run2
 *
 * This class implements the @c InputsHandler interface with the naming conventions
 * defined for the Run2 VHbb:
 *
 * * One input file per category
 * * So the base directory is the top-level of the file
 * * The names of histograms are the names of the samples (no suffixes)
 */
class InputsHandlerRun2 : public InputsHandlerPaper
{
  protected:
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
     * @return The name of the @c TFile to attach to this @c InputsHandlerRun2
     */
    static TString forgeFileName(Configuration& conf, const PropertiesSet& pset);

  public:
    /// No empty constructor
    InputsHandlerRun2() = delete;

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
    InputsHandlerRun2(Configuration& conf, const PropertiesSet& pset);

    /**
     * @brief Destructor
     *
     * The underlying @c TFile is properly closed.
     */
    ~InputsHandlerRun2();

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
