#ifndef HistoTransform_h
#define HistoTransform_h

class TDirectory;
class TH1;
class TFile;

#include <TString.h>
#include <vector>
#include <string>

class HistoTransform {
 public:
  struct Region {
    std::string name;
    float maxUnc;
    TH1* bkg;
    TH1* sig;
    std::vector<int> bins;
  };
  struct KFold {
    std::string name;
    int nFold;
    std::vector<Region*> regions;
  };
  struct SubDirectory {
    TDirectory* dir;
    std::vector<std::string> backgrounds;
    std::string signal;
    //std::vector<Region*> regions;
    std::vector<KFold*> kFolds;
  };

 private:

  TString m_inFileName;
  TString m_outFileName;
  TFile* m_inFile;
  TFile* m_outFile;

  std::vector<SubDirectory*> m_subDirs;

  KFold* getKFold(SubDirectory* subDir, std::string histName);
  Region* getRegion(SubDirectory* subDir, std::string histName);
  double getLLR(TH1* sig, TH1* bkg);
  bool isFirstOfKFold(KFold* kFold, std::string histName);
  void readRebinBins(SubDirectory* subDir);
  void rebinAllHistos(TDirectory* source, SubDirectory* subDir);
  void mergeKFolds(TDirectory* source, SubDirectory* subDir);
  void readTotalBkg(SubDirectory* source);
  void readSignal(SubDirectory* source);
  void findRegions(SubDirectory* subDir, std::string anyHistoName, float maxUnc = 0.05, std::string containRegion = "", int nFold = 1);
  SubDirectory* findSubDirs(std::string subDirName, bool identical = false);
  void skimKFoldTrafos(SubDirectory * subDir);
  void addRegion1(SubDirectory* subDir, std::string regionName, float maxUnc = 0.05, int nFold = 1);
  std::string getNameField(std::string histoName, int field);
  std::string getSampleName(std::string histoName);
  std::string getRegionName(std::string histoName);
  std::string getMVATag(std::string histoName);
  std::string getSysName(std::string histoName);

public:

  std::vector<int> getRebinBins(TH1* histoBkg, TH1* histoSig, int method, double maxUnc = 0.05);
  void rebinHisto(TH1* histo, std::vector<int>* bins, bool equidist=true, bool relabel=false);

  bool doMergeKFolds;
  bool doTransformBeforeMerging;
  bool transformBkgBDTs;
  int transformAlgorithm;
  float trafoTwoRebin;
  float trafoFiveY;
  float trafoSixY;
  float trafoSixZ;
  unsigned int trafoSevenNBins;
  int trafoEightRebin;
  float trafoEzSig;
  float trafoEzBkg;
  float trafoFzSig;
  float trafoFzBkg;

  SubDirectory* addSubDirectory(std::string subDirName);
  void addBackground(SubDirectory* subDir, std::string bkgName);
  void setSignal(SubDirectory* subDir, std::string bkgName);
  void addRegion(SubDirectory* subDir, std::string regionName, float maxUnc = 0.05, int nFold = 1);
  void run();

  HistoTransform();

  HistoTransform(TString inFileName, TString outFileName = "");

};

#endif
