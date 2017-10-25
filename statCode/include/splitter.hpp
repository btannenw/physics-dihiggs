#ifndef splitter_h
#define splitter_h

//#include <iomanip>
#include <vector>
#include <map>

// root
#include "TString.h"

class TDirectory;
class TFile;


/**
 * @brief Configuration of an input file
 *
 * Struct holding the configuration of a raw input file, plus a pointer to the actual file.
 */
struct InputFile {
  TString energy; ///< 7TeV or 8TeV
  TString type; ///< @c CUT or @c MVA
  TString channel; ///< @c Zero, @c One or @c TwoLepton
  TString fileName; ///< fileName as given in config (i.e. without preceeding @c m_inDir)
  TString distros; ///< optional: distributions to read from input file
  TFile* file; ///< the actual file
  TDirectory* dir; ///< the directory containing the histograms (the file itself or e.g. @c ZeroLepton)
};


/**
 * @brief Class doing all the heavy lifting for splitting inputs
 *
 * Class responsible for:
 * - reading a configuration file describing what to split
 * - copying the raw inputs on @c /tmp
 * - doing the split
 * - calling post-split scripts (merging of regions...)
 * - copying the results in @c inputs/version
 */
class splitter {

   public:

      /// Destructor
      ~splitter(){};

      /// Constructor
      splitter(TString version, TString inDir);

      /// Turn debug on
      void SetDebug(int i) { m_debug = i; }

      /// Main function
      void SplitIt();

   private:

      /// Empty constructor is private
      splitter(){};

      int m_debug;
      bool m_onlyCoreRegions;

      TString m_inDir;
      TString m_configFile;
      TString m_outDirLocal;
      TString m_outDirTmp;

      std::vector<InputFile> m_inputFiles;
      std::vector<TString> m_outputFiles; // full names of output files
      std::map<TString, std::vector<TString> > m_histoList;

      /// Reading configuration
      void ReadConfig();
      /// Reading input data
      void readHistoList(TDirectory* source, TString subdir);

      // splitting
      void SplitFile(InputFile &inputFile);
      void writeOutFile(InputFile &inputFile,
                        TString tagRegion,
                        TString VpTbin,
                        TString dist);
      void copyHistos(TDirectory* source, TDirectory* target, TString match);

      // moving input / output files
      void copyInputFileToTmp(InputFile &inputFile);
      void moveOutputFromTmp();

      // tools
      bool pushUnique(std::vector<TString> &vec, TString &str);
      TString getNameField(TString histoName, int field);
      void findRegions(TDirectory* dir,
          std::vector<TString> &tagRegions,
          std::vector<TString> &vptBins,
          std::vector<TString> &distributions,
	  TString distToUse = "",
          TString anySample = "data");

};// close splitter class

#endif //splitter_HPP_

