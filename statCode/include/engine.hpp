#ifndef engine_hpp
#define engine_hpp

#include <TString.h>

#include "configuration.hpp"
#include "analysishandler.hpp"
#include "samplehandler.hpp"
#include "categoryhandler.hpp"
#include "systematichandler.hpp"

/**
 * @brief Main class for workspace creation
 *
 * This class is the top-level for creation of workspaces.  It is meant to be called by
 * MakeWorkspace.C only.
 *
 * It is responsible for setting up the handlers of samples, categories, and systematics.
 *
 * The global workflow is also controlled from here. Flags are read from the configuration
 * file to do so. An example is making a stat-only workspace, as all loops on systematics will
 * be skipped.  @see MakeIt for the full list of options.
 *
 * It also tunes a bit the resulting workspace, by ensuring all starting values of NP are
 * correct, and enforcing the BinnedLikelihood option on the PDF.
 *
 */
class Engine
{

  public:

    /// No empty constructor
    Engine() = delete;

    /**
     * @brief Constructor
     *
     * @see MakeWorkspace.C for description of the parameters
     *
     * In this constructor, all handlers are initialized.
     * The configuration is also read and setup.
     *
     * @param confFileName	@c /path/to/configfile.conf. The config file follows @c TEnv
     * syntax
     * @param version	A version number or tag.
     */
    Engine(const TString& confFileName, const TString& version);

    /// Destructor
    ~Engine();

    /**
     * @brief Top-level algorithm
     *
     * Actions performed
     * =================
     *
     * - define vetos for some samples in some categories
     *   - example: no @c multijet in 1 lepton, as we have @c multijetEl and @c multijetMu
     * - add samples to categories if corresponding histos do exist in the root files
     * - call finalizeNominal on @c SampleHandler and @c CategoryHandler. Almost empty these
     *   days.
     * - make control plots
     * - prepare the merging of samples, if requested
     * - prepare the list of systematics to add
     * - add the POIs
     * - if needed, tune the decorrelation scheme for specific systematics (used for fit
     *   studies, not for baseline)
     * - add the user-defined systematics (i.e flat systs added in this code)
     * - add the specific 3/2 ratio systs that preserve 2+3 normalization (empty these days)
     * - loop on categories
     *   - apply histo systematics, as defined in the root files
     *   - Prune systs.
     *     - prune small shape systs
     *     - prune one-sided shape systs
     *     - prune small norm
     *     - prune same sign
     *     - prune small shape systs with chi2 test
     *     - special pruning (e.g remove buggy syst in a specific case)
     *     - prune small systs on small samples (Tuned by Yao)
     *     - Simple chi2 pruning à la Htautau (OFF by default)
     *   - merge samples if requested
     *   - print systematics if debug is ON
     *   - make plots of shape systematics if requested
     *   - write XML file and normalized ROOT file with surviving systematics
     * - clean up remainings of merging of samples
     * - make summary plots of systematics if requested
     * - write driver.xml file
     * - call hist2workspace
     * - call @c PimpMyWS
     *
     * @see SampleInCategory for further information on pruning
     *
     * List of relevant configuration options
     * ======================================
     *
     * @conf{MergeSamples,false} merging of samples that can go together, like all single top.
     * @conf{UseSystematics, true} make a workspace with systematics.
     * @conf{UseFloatNorms, true} use floating normalizations for some samples.
     * @conf{DecorrSys, None} decorrelate a given syst for fit studies.
     * @conf{DecorrTypes, 0} decorrelate only some properties of the syst.
     * @conf{LumiForAll, true} keep lumi syst even on floated backgrounds.
     * @conf{SkipUnkownSysts, false} if true, just skip histo systematics which are present in
     * a rootfile but not in the list. If false, throw an exception in such a case.
     * @conf{DoShapePlots,false} make plots of all shape systematics after pruning.
     * @conf{HTauTauPruning, false} brutal chi2 pruning à la Htautau.
     * @conf{DoSystsPlots, false} make the summary plots of systematics (values of norm systs, and
     * summary of pruning).
     *
     */
    void MakeIt();

    /**
     * @brief Some tuning of the workspace resulting from hist2workspace
     *
     * Perform the following actions on the workspace created by hist2workspace
     * - Set the starting point for all gamma stat NP to 1
     * - Set the starting point for all ATLAS_norm NP to 1
     * - Set the BinnedLikelihood attribute to the PDF for faster likelihood evaluation
     * - Save an initial snapshot
     * - Save an initial RooExpandedFitResult
     * - Rename the file to @c higgsmass.root and move it to correct directory
     * - Write binning histograms in the same file, when histos with variable bin
     *   sizes are used
     *
     * @param wsfilename	Path to the workspace file
     *
     * @todo should we really set starting point of all floating norm NP to 1 ?
     *
     */
    void PimpMyWS(const TString& wsfilename);

  private:
    Configuration m_config;
    AnalysisHandler m_analysis;
    SampleHandler m_samples;
    CategoryHandler m_categories;
    SystematicHandler m_systematics;

};

#endif
