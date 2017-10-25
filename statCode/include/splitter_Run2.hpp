#ifndef splitter_Run2_h
#define splitter_Run2_h

//#include <iomanip>
#include <vector>
#include <map>
#include <set>
#include <utility>

// root
#include "TString.h"
#include "TKey.h"

class TDirectory;
class TFile;


/**
 * @brief Configuration of an input file for Run2
 *
 * Struct holding the configuration of a raw input file, plus a pointer to the actual file.
 */
struct InputFile_R2 {
  TString energy; ///< 7TeV or 8TeV or 13TeV
  TString channel; ///< @c Zero, @c One or @c TwoLepton
  TString fileName; ///< fileName as given in config (i.e. without preceeding @c m_inDir)
  TString distros; ///< optional: distributions to read from input file
  TFile* file; ///< the actual file
  TDirectory* dir; ///< the directory containing the histograms (the file itself or e.g. @c ZeroLepton)
  std::set<TString> v_distros;
};


/**
 * @brief Class doing all the heavy lifting for splitting inputs. Run2 variant.
 *
 * Class responsible for:
 * - reading a configuration file describing what to split
 * - copying the raw inputs on @c /tmp
 * - doing the split
 * - calling post-split scripts (merging of regions...)
 * - copying the results in @c inputs/version
 *
 * WARNING: The @c CoreRegions feature has not been implemented yet
 */
class splitter_Run2 {

   public:

      /// Destructor
      ~splitter_Run2(){};

      /// Constructor
      splitter_Run2(TString version, TString inDir);

      /// Turn debug on
      void SetDebug(int i) { m_debug = i; }

      /// Main function
      void SplitIt();

   private:

      /// Empty constructor is private
      splitter_Run2(){};

      int m_debug;
      bool m_onlyCoreRegions;

      TString m_inDir;
      TString m_configFile;
      TString m_outDirLocal;
      TString m_outDirTmp;

      std::vector<InputFile_R2> m_inputFiles;
      std::vector<TString> m_outputFiles; // full names of output files
      std::map<TString, std::vector<std::pair<TString, TKey*> > > m_histoList;

      /// Reading configuration
      void ReadConfig();
      /// Reading input data
      void readHistoList(TDirectory* source, TString subdir, const std::set<TString>& distros);

      // splitting
      void SplitFile(InputFile_R2 &inputFile);
      void writeOutFile(InputFile_R2 &inputFile, const TString& regName);
      void copyHistos(TDirectory* source, TDirectory* target, TString match);

      // moving input / output files
      void copyInputFileToTmp(InputFile_R2 &inputFile);
      void moveOutputFromTmp();

      // add data histo if missing
      void addEmptyDataHistoIfMissing(const TString& fileName);

};// close splitter_Run2 class

#endif //splitter_Run2_HPP_

