#ifndef outputhandler_hpp
#define outputhandler_hpp

#include <memory>
#include <fstream>

#include <TString.h>

class TH1;
class TFile;
class Configuration;
class Category;
struct Systematic;

// FIXME this whole class assumes that we have only 1 signal mass
// do we want to keep the functionality of treating several masses
// simultaneously ?

/**
 * @brief Analysis-level configuration for the output files
 *
 * The singleton class is responsible to manage the paths where output
 * files of different types should be held. It also has a couple of flags
 * related to them, such as if the intermediate files should be deleted.
 */
class OutputConfig {
  private:
    /// Static instance of the singleton
    static OutputConfig* the_instance;

    /// No empty constructor
    OutputConfig() = delete;

    /// Standard constructor with the analysis configuration
    OutputConfig(Configuration& conf);

    /// Destructor
    ~OutputConfig();

  public:
    /// Static accessor to the instance
    static OutputConfig* getInstance();

    /**
     * @brief Static accessor to the instance with configuration
     *
     * Used to instantiate the class. Ugly !
     *
     * Configuration parameters read:
     * @conf{InputVersion,10000} version of the (split) input files
     * @conf{DeleteNormFiles,true} delete the temporary normalized root files
     * @conf{MassPoint,125} Higgs mass considered
     *
     * @c conf.version() is also used in creating the paths for the output files
     *
     * @param conf @c Configuration object to instantiate the paths
     * @return Pointer to the instance
     */
    static OutputConfig* createInstance(Configuration& conf);

    /// Static call to the destructor
    static void destruct();

    /// Whether to delete temporary normalized root files
    bool deleteFiles;

    /**
     * @brief Directory where normalized root files are kept
     *
     * /tmp/$USER/normfiles/input.output/ if files are to be deleted
     *
     * inputs/input.output/ otherwise
     */
    TString normDir;

    /**
     * @brief Directory for the XML files
     *
     * xml/input.output/masspoint
     */
    TString xmlDir;

    /**
     * @brief Directory for the workspaces
     *
     * workspaces/input.output/combined
     */
    TString workspaceDir;

    /**
     * @brief Directory for the plots
     *
     * plots/input.output/
     */
    TString plotDir;
};

/**
 * @brief Class to handle the output files corresponding to a given category
 *
 * This class is typically used by a @c Category to handle the writing of its
 * corersponding XML file and root file with "normalized" histograms (histos after
 * rebinning, smoothing, pruning, merging)
 *
 * A poor man's state machine is used to help with the consistency and correctness
 * of the XML file. The status are checked and the transitions are performed by the
 * functions that act on the XML file.
 */
class OutputHandler
{
  private:
    /// Possible status of the writing in the XML file
    enum class Status {
      NotInit, ///< files not yet open
      Ready, ///< can add category-level lines, or open a new sample
      InSample, ///< a sample is open. Can add systematics to it
      Closed ///< files are closed. It is impossible to add anything to them
    };

    /// (Unique) pointer to the output normalized root file
    std::unique_ptr<TFile> m_file;

    /// (Unique) pointer to the XML file
    std::unique_ptr<std::ofstream> m_xml;

    /// Path to the XML file
    TString m_xmlName;

    /// Path to the normalized root file
    TString m_fname;

    /// Status of the writing in the XML file
    Status m_status;

    /// Whether the normalized root file should be deleted
    bool m_deleteFile;

    /**
     * @brief Check the current status of the writing
     *
     * Helper function to check that the internal status (@c m_status) corresponds to
     * what we expect (@c s).
     *
     * If the status doesn't match, an exception is thrown.
     *
     * @param s	The @c Status we should have
     */
    void assertStatus(Status s);

    /**
     * @brief Add a norm-only systematic to the output
     *
     * Function called by @c addSyst.
     * Hence, no check is performed on the current status, neither on the type
     * of @c sys
     * Simply add a NormSys line in the XML
     *
     * @param sys	Systematic to add
     */
    void addNormSyst(const Systematic& sys) const;

    /**
     * @brief Add a shape systematic to the output
     *
     * Function called by @c addSyst.
     * Hence, no check is performed on the current status, neither on the type
     * of @c sys
     * Write the up and down histograms in the output root file, and link them in the XML
     * file (HistoSys line)
     *
     * @param sys	Systematic to add
     */
    void addShapeSyst(const Systematic& sys) const;

    /**
     * @brief Add a floating normalization to the output
     *
     * Function called by @c addSyst.
     * Hence, no check is performed on the current status, neither on the type
     * of @c sys
     * Simply add a NormFactor line in the XML
     *
     * @param sys	Systematic to add
     */
    void addFltSyst(const Systematic& sys) const;

    /**
     * @brief Write a histogram in the output file
     *
     * Write in the output normalized root file an histogram, either nominal or of
     * a systematic
     *
     * @param name	Name of the histogram to write
     * @param h Histogram to write
     * @param sysname Name of the systematic the histo belongs to (if any. "" otherwise)
     */
    void writeHisto(const TString& name, const TH1* h, const TString& sysname = "") const;

  public:
    /**
     * @brief Empty constructor
     *
     * Changes to: @c Status::NotInit
     */
    OutputHandler();

    /**
     * @brief Constructor from a category
     *
     * From the properties of the category, and the configuration stored in the
     * @c OutputConfig, the XML file and the normalized root file are created and
     * initialized.
     *
     * Changes to: @c Status::Ready
     *
     * @param cat	Parent category
     */
    OutputHandler(const Category& cat);

    /// Default move constructor
    OutputHandler(OutputHandler&&) = default;

    /**
     * @brief Destructor
     *
     * The output files are closed if they are not already.
     * The normalized root file is deleted if the flag has been set.
     */
    ~OutputHandler();

    /// Move assignment operator. Used somewhere
    OutputHandler& operator=(OutputHandler&& other);

    /**
     * @brief Add the configuration for MC stat uncertainty
     *
     * Add the StatErrorConfig line
     *
     * Requires: @c Status::Ready
     *
     * @param thresh	Threshold for MC stat NP
     */
    void addMCStatThresh(float thresh = 0.05);

    /**
     * @brief Add the data corresponding to this category
     *
     * Write the data histogram to the output root file, then add the data line in
     * the XML file.
     *
     * The data histogram is NOT destroyed after being written.
     *
     * Requires: @c Status::Ready
     *
     * @param name	Name of the data histogram
     * @param hdata Data histogram
     */
    void addData(const TString& name, const TH1* hdata);

    /**
     * @brief Starts adding info about a sample
     *
     * Write the nominal histogram to the output file, then add the sample line
     * in the XML file.
     *
     * The nominal histogram is NOT destroyed after being written.
     *
     * Requires: @c Status::Ready
     *
     * Changes to: @c Status::InSample
     *
     * @param name	Name of the sample to add
     * @param hnom  Nominal histogram for this sample
     */
    void beginSample(const TString& name, const TH1* hnom);

    /**
     * @brief Add a systematic to a sample
     *
     * Depending on the @c type of the systematic, call one of the specialized
     * functions to add Norm, Shape, or floating norm systematics.
     *
     * Shape syst histograms ARE destroyed after being written.
     *
     * Requires: @c Status::InSample
     *
     * @param sys	Systematic to add
     */
    void addSyst(const Systematic& sys);

    /**
     * @brief Use this sample for MC stat accounting
     *
     * Requires: @c Status::InSample
     *
     * @param activate use this sample for MC stat error
     */
    void activateStatErr(bool activate);

    /**
     * @brief End the description of the current sample
     *
     * Add @c </Sample> to the XML.
     *
     * Requires: @c Status::InSample
     *
     * Changes to: @c Status::Ready
     */
    void closeSample();

    /**
     * @brief Write and close the XML and root files
     *
     * Properly write and close the files.
     *
     * Requires: @c Status::Ready
     *
     * Changes to: @c Status::Closed
     *
     * @return Path to the XML file
     */
    TString closeFiles();

};

#endif
