#include "splitter_Run2.hpp"

#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <sstream>
#include <algorithm>

#include <TSystem.h>
#include <TPRegexp.h>
#include "TClass.h"       // for TClass
#include "TCollection.h"  // for TIter
#include "TDirectory.h"   // for TDirectory
#include "TFile.h"        // for TFile
#include "TH1.h"          // for TH1
#include "TH2.h"          // for TH2
#include "TH3.h"          // for TH3
#include "TKey.h"         // for TKey
#include "TList.h"        // for TList
#include "TObjArray.h"    // for TObjArray
#include "TObjString.h"   // for TObjString
#include "TObject.h"      // for TObject
#include "utility.hpp"


/*
 * See header file for description
 *
 */

using namespace std;

splitter_Run2::splitter_Run2(TString version, TString inDir) {
  m_debug = 0;
  // make sure all directories end on "/"
  if (!inDir.EndsWith("/"))
    inDir += "/";
  m_inDir = inDir;
  m_configFile = "inputConfigs/" + version + ".txt";
  m_outDirLocal = "inputs/" + version + "/";
  // on lxplus //
  m_outDirTmp = TString("/tmp/" + TString(getlogin()) + "/" + m_outDirLocal);
  // on lyon //
  //m_outDirTmp = TString("/scratch/" + TString(getlogin()) + "/" + m_outDirLocal);
  system("mkdir -vp " + m_outDirTmp);
  system("mkdir -vp " + m_outDirLocal);
  m_inputFiles.clear();
  m_outputFiles.clear();
  m_histoList.clear();
  m_onlyCoreRegions = false;
}; // splitter_Run2

void splitter_Run2::SplitIt() {

  /// Assuming nominal histograms are either stored in the file root directory
  /// or in channel sub directories.
  /// Also directories containing systematic histograms are expected there.
  /// The histograms are assumed to have the format:
  /// sample_tagRegion_VpTbin_distribution_anything
  /// The VpT bin may contain a '_' followed by a '1', '2 or '9'
  /// The sample 'data' is expected to determine tagRegions and VpT bins, e.g.:
  /// data_2tttag3jet_vpt120_160_mva150_SysBla

  ReadConfig();

  for (vector<InputFile_R2>::iterator inputFile(m_inputFiles.begin()); inputFile != m_inputFiles.end(); ++inputFile){
    TString fileName = (*inputFile).fileName;
    // just link output files from previous version for non '.root' files
    if (!fileName.EndsWith(".root")) {
      int energyTagPos = fileName.Index('_');
      TString version =  fileName(0, energyTagPos);
      cout << "Linking output files from version '" << version << endl;
      TString pwd = gSystem -> pwd();
      // FIXME we lost the ability to link inputs from one c.m.e only. Resurrect if needed someday
      TString outFiles = "*_" + (*inputFile).channel + "_*.root";
      TString cmd = "cd " + pwd + "/" + m_outDirLocal + "; ";
      cmd += "ln -sv ../" + version + "/" + outFiles + " ./; ";
      cmd += "cd " + pwd;
      system(cmd);
      cout << endl;
      continue;
    }
    // copy new input file
    cout << "Trying to get input file: " << (*inputFile).fileName << endl;
    copyInputFileToTmp(*inputFile);
    // open file here to check availability
    TString fullName = m_outDirTmp + (*inputFile).fileName;
    (*inputFile).file = TFile::Open(fullName, "READ");
    if (!(*inputFile).file) {
      cout << "ERROR: Missing file: " << fullName << endl;
      exit(-1);
    }
    cout << endl;
  }

  for (vector<InputFile_R2>::iterator inputFile(m_inputFiles.begin()); inputFile != m_inputFiles.end(); ++inputFile) {
    // skip non '.root' files
    if (!(*inputFile).fileName.EndsWith(".root")) {
      continue;
    }
    cout << "Splitting file: " << (*inputFile).fileName << endl;
    cout << "Channel: " << (*inputFile).channel << endl;
    SplitFile(*inputFile);
    // close file
    (*inputFile).file->Close();
    delete (*inputFile).file;
    (*inputFile).file = 0;
    (*inputFile).dir = 0;
    cout << endl;
  }

  moveOutputFromTmp();

  cout << "Leaving original input files in " << m_outDirTmp << endl;
  cout << "Done!" << endl;
}; // SplitIt

/**
 * Read a configuration file for splitter_Run2.
 *
 * The syntax is:
 * - a line beginning by # is a comment
 * - CoreRegions means that only 1 or 2 tag, 2 or 3 jet and topemu will be split
 *   MV1c will be extracted only in 1 tag
 * - Main construct is "Channel Type Path", where
 *   - Channel is ZeroLepton, OneLepton, TwoLepton, OneLeptonMET,
 *     OneLepton_El, OneLepton_Mu, TwoTauLH, TwoTauHH, OnePhoton
 *   - Type is CUT or MVA
 *   - Path is path to input file, relative to inDir
 *   - Path can also be of the form previousVersion.XTeV. In that case, no splitting will
 *     be performed, and the files split from previousVersion will be linked in the new
 *     inputs/version/ directory.
 *
 */
void splitter_Run2::ReadConfig() {
  TString configFileName = m_configFile;
  cout << "Reading config file: " << configFileName << endl;
  ifstream configFile(configFileName);
  if (!configFile.good()) {
    configFileName = m_outDirLocal + "input.txt";
    cout << "WARNING: Config file not found. Trying old convention: " << configFileName << endl;
    configFile.open(configFileName);
  }
  if (!configFile.good()) {
    cout << "ERROR: Missing config file!" << endl;
    exit(-1);
  }
  m_inputFiles.clear();


  string line;
  while (getline(configFile, line)) {
    istringstream iss(line);
    InputFile_R2 inputFile;
    iss >> inputFile.channel >> inputFile.fileName >> inputFile.distros;
    if (inputFile.channel.BeginsWith("#"))
      continue;
    if (inputFile.channel == "")
      continue;
    if (inputFile.channel.BeginsWith("CoreRegions")) {
      m_onlyCoreRegions = true;
      cout << "INFO: Only core regions ( 1|2 tag, 2|3 jet, + topemu) will be split" << endl;
      continue;
    } else if (inputFile.fileName.Contains("7TeV")) {
      inputFile.energy = "7TeV";
    } else if (inputFile.fileName.Contains("8TeV")) {
      inputFile.energy = "8TeV";
    } else if (inputFile.fileName.Contains("13TeV")) {
      inputFile.energy = "13TeV";
    } else {
      cout << "ERROR: Energy tag in input file name not found: "
              << inputFile.fileName << endl;
      exit(-1);
    }
    inputFile.file = 0;
    cout << "Found:	"
            << inputFile.energy << "	"
            << inputFile.channel << "	"
	    << inputFile.fileName;
    if (inputFile.distros != "") {
      cout << "\t (Distributions: " << inputFile.distros << ")";
      for(const auto& s : Utils::splitString(inputFile.distros, ','))
        inputFile.v_distros.insert(s);
    }
    cout << endl;
    m_inputFiles.push_back(inputFile);
  }
  if (m_inputFiles.size() == 0) {
    cout << "ERROR: no input files defined in config!" << endl;
    exit(-1);
  }
  cout << endl;
  configFile.close();
}

void splitter_Run2::readHistoList(TDirectory* source, TString subdir, const std::set<TString>& distros) {
  TKey* key;
  TIter nextkey(source->GetListOfKeys());
  TString stripped_subdir(subdir.Strip(TString::EStripType::kTrailing, '/'));
  std::cout << "subdir is " << stripped_subdir << std::endl;
  TPRegexp stricttagpattern("\\w+_((((2|1)\\w*tag(2|3)jet)|(topemu\\w+))_vpt\\d+(_\\d+)?_([a-zA-Z0-9]+))(_\\w+)?");
  while ((key = (TKey*) nextkey())) {
    TString name = key->GetName();
    if (m_debug) cout << "Name::" << name << "::" << endl;
    TClass* objclass = TClass::GetClass(key->GetClassName());
    if (objclass->InheritsFrom(TDirectory::Class())) {
      if (name.Contains("Systematics") || name.BeginsWith("Sys")) {
        if (m_debug) cout << "Going into " << key->GetName() << endl;
        cout << ".";
        flush(cout);
        TDirectory* source1 = (TDirectory*) key->ReadObj();
        readHistoList(source1, subdir + name + "/" , distros);
        delete source1;
      } else {
        cout << "Don't go into " << name << endl; 
      }
    } else if (objclass->InheritsFrom(TH1::Class()) &&
               !objclass->InheritsFrom(TH2::Class()) &&
               !objclass->InheritsFrom(TH3::Class())) {

      TString nameSys(name);
      if (name.Index("_Sys") >0 ) nameSys.Remove(0,name.Index("_Sys")+1);
      TPRegexp tagpattern("\\w+_((\\w*tag\\w*jet)_(\\d+)(_\\d+)?(?:ptv|ptbb|BDT)(_[a-zA-Z0-9]+)+_(\\w+))"+(stripped_subdir=="" ? "" : "_"+nameSys));
      TObjArray* res = tagpattern.MatchS(name);

      if(res->GetEntries() < 2){
        cout << "WARNING: found an histo with non-matching name " << name << " for dir " << (subdir=="" ? "" : subdir) << " !" << endl;
        delete res;
        continue;
      }

      // if CoreRegions is switched, remove useless histos.
      if(m_onlyCoreRegions) {
        TObjArray* res2 = stricttagpattern.MatchS(name);
        if(res2->GetEntries() < 2) {
          delete res;
          delete res2;
          continue;
        }
        TString ntag = Utils::group(res2, 4);
        TString var = Utils::group(res2, 8);
        if( ntag == "2" && var.BeginsWith("MV1")) {
          delete res;
          delete res2;
          continue;
        }
        //cout << "INFO: Ignoring histogram : " << name << endl;
        delete res2;
      }

      if(distros.size()) {
        TString dist = Utils::group(res, 6);
        if( ! distros.count(dist)) {
          delete res;
          continue;
        }
      }

      m_histoList[Utils::group(res, 1)].push_back(std::make_pair(subdir + name, (TKey*)key->Clone() ));
      delete res;
    }
  }
}

void splitter_Run2::SplitFile(InputFile_R2 &inputFile) {

  // try to get channel sub-directory
  inputFile.dir = inputFile.file;
  TObject* subDir = inputFile.file->Get(inputFile.channel);
  if (subDir)
    inputFile.dir = (TDirectory*) subDir;

  cout << "Reading full list of histograms...";
  m_histoList.clear();
  readHistoList(inputFile.dir, "", inputFile.v_distros);
  cout << endl;
  for (auto it(m_histoList.begin()); it != m_histoList.end(); ++it) {
    cout << "Found " << it->second.size() << " histograms for " << it->first << endl;
    writeOutFile(inputFile, it->first);
  }

}

void splitter_Run2::writeOutFile(InputFile_R2 &inputFile, const TString& regName) {
  TString match = regName;
  TString outFileName = m_outDirTmp
          + inputFile.energy + "_"
          + inputFile.channel + "_"
          + match + ".root";

  cout << "Writing " << outFileName << " ";
  flush(cout);
  int timeStart = time(0);
  TFile* outFile = new TFile(outFileName, "RECREATE");
  // creating of channel directories skipped (included in file name)
  //TDirectory* outDir = outFile->mkdir(inputFile.channel);
  copyHistos(inputFile.dir, outFile, match);
  outFile->Close();
  delete outFile;
  m_outputFiles.push_back(outFileName);
  cout << time(0) - timeStart << "s" << endl;
}

void splitter_Run2::copyHistos(TDirectory* source, TDirectory* target, TString match) {
  // assumptions:
  // just one layer of sub folders (Sys...)
  // m_histoList to be ordered in sub folders (given by readHistoList)

  TString subDirPrev = "";
  // linear looping over all histogram names.
  // not the most efficient way, but uses negligible amount of total runtime.
  for (auto hName(m_histoList[match].begin()); hName != m_histoList[match].end(); ++hName) {
    // *hName includes the sub directory name, eg "SysBla/sample_tags_SysBla"
    //TObject* obj = source -> Get(*hName);
    source->cd();
    TObject* obj = hName->second->ReadObj();
    if (!obj) {
      cout << "ERROR: histogram '" << hName->first << "' in list, but not found in file again!" << endl;
      exit(-1);
    }

    // split name
    TString writeName = hName->first;
    TString subDir = "";
    if (writeName.Contains('/')) {
      subDir = writeName;
      subDir.Remove(subDir.First('/'), subDir.Length());
      writeName.Remove(0, writeName.First('/') + 1);
    }

    if (writeName.Contains('/')) {
      cout << "ERROR: only one layer of sub directories supported!" << endl;
    }

    // go into sub directory
    bool success = target->cd();
    if (subDir != "") {
      // assuming ordering
      if (subDir != subDirPrev) {
        if (m_debug) cout << "create " << subDir << endl;
        target -> mkdir(subDir);
        subDirPrev = subDir;
      }
      success &= target->cd(subDirPrev);
    }
    if (!success) {
      cout << "ERROR: could not change to output directory!" << endl;
      exit(-1);
    }

    // remove tags from histogram names to be left with the sample name
    TString title = writeName;
    writeName.ReplaceAll("_" + match, ""); // remove region
    if (subDir != "")
      writeName.ReplaceAll("_" + subDir, ""); // remove systematic
    // remove '_' from stop samples
    writeName.ReplaceAll("stop_Wt", "stopWt");
    writeName.ReplaceAll("stop_s", "stops");
    writeName.ReplaceAll("stop_t", "stopt");
    TH1* histo = (TH1*) obj;
    histo->SetTitle(title);
    histo->SetName(writeName);

    // writing objects to the new file consumes about 1/2 of the runtime
    obj->Write();
    // deleting objects consumes the other half (might be optimized?)
    delete obj;
  }
  //target->SaveSelf(true);
}

void splitter_Run2::copyInputFileToTmp(InputFile_R2 &inputFile) {
  cout << "Copy from " << m_inDir << " to " << m_outDirTmp << endl;
  TString subDir = "";
  TString inDir = m_inDir;
  if (inputFile.fileName.Contains("root://eosatlas/")) {
    inDir = inputFile.fileName;
    inDir.Remove(inDir.Last('/') + 1, inDir.Length());
    inputFile.fileName.Remove(0, inputFile.fileName.Last('/'));
    subDir = "";
  }
  else if (inputFile.fileName.Contains("/")) {
    subDir = inputFile.fileName;
    subDir.Remove(subDir.Last('/') + 1, subDir.Length());
  }

  system("mkdir -vp " + m_outDirTmp + subDir);
  TString cmd = "cp -uv ";
  if (inDir.Contains("/eos/")) {

    cmd = "/afs/cern.ch/project/eos/installation/0.3.84-aquamarine/bin/eos.select cp -k ";
    inDir.ReplaceAll("root://eosatlas//", "/");
  }
  system(cmd + inDir + inputFile.fileName + " " + m_outDirTmp + subDir);
}

void splitter_Run2::moveOutputFromTmp() {
  cout << "Moving output files from " << m_outDirTmp << " to " << m_outDirLocal << endl;
  for (vector<TString>::iterator fileName(m_outputFiles.begin()); fileName != m_outputFiles.end(); ++fileName) {
    // add empty Data histogram if missing
    addEmptyDataHistoIfMissing(*fileName);
    // add Data when necessary
    //system("((`/usr/bin/stat -c%s " + *fileName + "` < 1000)) || addData " + *fileName );
    // only move non-empty files
    system("((`/usr/bin/stat -c%s " + *fileName + "` < 1000)) || mv -v " + *fileName + " " + m_outDirLocal);
  }
}

void splitter_Run2::addEmptyDataHistoIfMissing(const TString& fileName) {

  TFile* outfile = TFile::Open( fileName , "update" );

  // check if a data histogram already exists //
  if ( outfile->FindObjectAny("data") ) {
    return;
  } 

  // if it doesn't loop on the histograms in the file                              //
  // this is to get characteristics of the first TH1F to give to a empty data hist // 
  TH1F* data = NULL;
  TKey* key;
  TIter nextkey(outfile->GetListOfKeys());
  while ((key = (TKey*) nextkey())) {

    TClass* objclass = TClass::GetClass(key->GetClassName());

    // check that we are getting a TH1F and not a dir or other stuff //
    if (objclass->InheritsFrom(TH1::Class()) &&
        !objclass->InheritsFrom(TH2::Class()) &&
        !objclass->InheritsFrom(TH3::Class())) {

       // save needed info //
       TObject* obj = key->ReadObj();
       if (!obj) {
         cout << "ERROR: histogram '" << key->GetClassName() << "' in list, but not found in file again!" << endl;
         continue;
       }
       data = (TH1F*)obj->Clone("data"); // add region name to name
       data->SetTitle("data");
       int Nbins = data->GetSize(); // also emptying overflow and underflow bins from count
       for ( int i = 0; i < Nbins; i++ ) {
          data->SetBinContent(i, 0.);
          data->SetBinError(i, 0.);
       }

       break;

    }
  }

  // add data histo to existing file //
  if (data) {
    data->Write();
    delete data;
  }

  outfile->Close();

  return;
}
