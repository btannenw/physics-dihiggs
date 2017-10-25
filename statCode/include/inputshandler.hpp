#ifndef inputshandler_hpp
#define inputshandler_hpp

#include <vector>
#include <memory>

#include <TString.h>
#include <TFile.h>

class TDirectory;
class TH1;
class Sample;
class Configuration;
struct PropertiesSet;

/**
 * @brief Base class for management of input files for a given category
 *
 * It assumes that all histograms for a given category are in one file,
 * and in one directory inside that file.
 *
 * Another asumption is that nominal histograms will be in the base directory,
 * and that histograms for systematics will be in specific directories.
 *
 * It needs to be subclassed for all practical cases.
 * Only implementation for 2014 paper split inputs is provided so far
 * Implementations could be provided for EPS inputs, or raw (before split)
 * paper inputs
 */
class InputsHandler
{
  protected:
    /// (Unique) pointer to the underlying @c TFile holding the histograms
    std::unique_ptr<TFile> m_file;

    /// Pointer to the base directory in which all histograms for this category are present.
    TDirectory* m_baseDir;

    /**
     * @brief Find a histogram from its name, and the name of a systematic
     *
     * Simple @c GetObject(name) or @c GetObject(systname/name) or @c GetObject(dirname/name_systname)
     *
     * @param name Name of the histogram to find
     * @param systname Name of the systematic for which the histo should be found
     * @param dirname  Name of the directory in which systematics are kept, if there is 1 dir for all systs
     * @return Null pointer if the histo does not exist. A pointer to the histo otherwise.
     * The underlying @c TFile remains the owner of the returned histogram (to be checked)
     */
    TH1* getHistByName(const TString& name, const TString& systname = "", const TString& dirname = "");

    /// Set @c m_baseDir
    virtual void findBaseDir() = 0;

    /**
     * @brief Check that data exists in the input file
     *
     * This is used to check that the category is not empty / exists at all.
     *
     * @return @c true if a data histogram exists, @c false otherwise
     */
    virtual bool checkData() = 0;

  public:
    /// Default constructor
    InputsHandler();

    /**
     * @brief Standard constructor
     *
     * @param conf A @c Configuration object for the analysis
     * @param pset A @c PropertiesSet that define the category which this InputsHandler
     * will be attached to.
     */
    InputsHandler(Configuration& conf, const PropertiesSet& pset);

    /// Destructor
    virtual ~InputsHandler();

    /**
     * @brief Check that the category exists and is not empty
     *
     * Checks that the pointers to the @c TFile and @c TDirectory are valid, then
     * check that the data histogram in this category can be accessed.
     *
     * @return @c true if histograms for this category can be found, @c false otherwise
     */
    virtual bool exists();

    /**
     * @brief List the directories containing systematic variations
     *
     * Go through the subdirectories of the base directory, and assume that all of them
     * hold systematic variations of the histograms
     *
     * @return A list of directory names
     */
    std::vector<TString> listSystematics();

    /**
     * @brief List all samples (nominal histograms) for this category
     *
     * @return A list of sample names
     */
    virtual std::vector<TString> listSamples() = 0;

    /**
     * @brief Get the histogram for a given sample and a given systematic
     *
     * @param sample Sample for which the histogram has to be found
     * @param systname Name of the systematic variation to retrieve, or empty for nominal.
     * @return Null pointer if the histo does not exist. A pointer to the histo otherwise.
     * The underlying @c TFile remains the owner of the returned histogram (to be checked)
     */
  virtual TH1* getHist(const Sample& sample, const TString& systname = "") = 0;

};

#endif
