#include "splitter.hpp"

#include <ctime>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <sstream>

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


/*
 * See header file for description
 *
 */

using namespace std;

splitter::splitter(TString version, TString inDir) {
  m_debug = 0;
  // make sure all directories end on "/"
  if (!inDir.EndsWith("/"))
    inDir += "/";
  m_inDir = inDir;
  m_configFile = "inputConfigs/" + version + ".txt";
  m_outDirLocal = "inputs/" + version + "/";
  m_outDirTmp = TString("/tmp/" + TString(getlogin()) + "/" + m_outDirLocal);
  system("mkdir -vp " + m_outDirTmp);
  system("mkdir -vp " + m_outDirLocal);
  m_inputFiles.clear();
  m_outputFiles.clear();
  m_histoList.clear();
  m_onlyCoreRegions = false;
}; // splitter

void splitter::SplitIt() {

  /// Assuming nominal histograms are either stored in the file root directory
  /// or in channel sub directories.
  /// Also directories containing systematic histograms are expected there.
  /// The histograms are assumed to have the format:
  /// sample_tagRegion_VpTbin_distribution_anything
  /// The VpT bin may contain a '_' followed by a '1', '2 or '9'
  /// The sample 'data' is expected to determine tagRegions and VpT bins, e.g.:
  /// data_2tttag3jet_vpt120_160_mva150_SysBla

  ReadConfig();

  for (vector<InputFile>::iterator inputFile(m_inputFiles.begin()); inputFile != m_inputFiles.end(); ++inputFile){
    TString fileName = (*inputFile).fileName;
    // just link output files from previous version for non '.root' files
    if (!fileName.EndsWith(".root")) {
      int length = fileName.Length();
      TPRegexp regex(".[78]TeV");
      if (!regex.Match(fileName(length-5, length))) {
	cout << "ERROR: version for linking not recognized: " << fileName << endl;
	exit(-1);
      }
      TString version =  fileName(0, length-5);
      TString energy =  fileName(length-4, length);
      cout << "Linking output files from version '" << version << "' (" << energy << ")" << endl;
      TString pwd = gSystem -> pwd();
      TString outFiles = energy + "_" + (*inputFile).channel + "_" + (*inputFile).type + "_*.root";
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

  for (vector<InputFile>::iterator inputFile(m_inputFiles.begin()); inputFile != m_inputFiles.end(); ++inputFile) {
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

  cout << "Hadd regions that may be used in  fits" << endl;
  system("hadd_inputs.sh "+m_outDirLocal);

  cout << "Leaving original input files in " << m_outDirTmp << endl;
  cout << "Done!" << endl;
}; // SplitIt

/**
 * Read a configuration file for splitter.
 *
 * The syntax is:
 * - a line beginning by # is a comment
 * - CoreRegions means that only 1 or 2 tag, 2 or 3 jet and topemu will be split
 *   MV1c will be extracted only in 1 tag
 * - Main construct is "Channel Type Path", where
 *   - Channel is ZeroLepton, OneLepton, TwoLepton, OneLeptonMET,
 *     OneLepton_El, OneLepton_Mu, TwoTauLH, TwoTauHH
 *   - Type is CUT or MVA
 *   - Path is path to input file, relative to inDir
 *   - Path can also be of the form previousVersion.XTeV. In that case, no splitting will
 *     be performed, and the files split from previousVersion will be linked in the new
 *     inputs/version/ directory.
 *
 */
void splitter::ReadConfig() {
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
    InputFile inputFile;
    iss >> inputFile.channel >> inputFile.type >> inputFile.fileName >> inputFile.distros;
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
    } else {
      cout << "ERROR: Energy tag in input file name not found: "
              << inputFile.fileName << endl;
      exit(-1);
    }
    inputFile.file = 0;
    cout << "Found:	"
            << inputFile.energy << "	"
            << inputFile.channel << "	"
            << inputFile.type << "	"
	    << inputFile.fileName;
    if (inputFile.distros != "")
      cout << "\t (Distributions: " << inputFile.distros << ")";
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

void splitter::readHistoList(TDirectory* source, TString subdir) {
  TKey* key;
  TIter nextkey(source->GetListOfKeys());
  TPRegexp tagpattern("\\w+_(((\\w*tag\\w*)|(top\\w+))_vpt\\d+(_\\d+)?_[a-zA-Z0-9]+)(_\\w+)?");
  TPRegexp stricttagpattern("\\w+_((((2|1)\\w*tag(2|3)jet)|(topemu\\w+))_vpt\\d+(_\\d+)?_([a-zA-Z0-9]+))(_\\w+)?");
  while ((key = (TKey*) nextkey())) {
    TString name = key->GetName();
    TClass* objclass = TClass::GetClass(key->GetClassName());
    if (objclass->InheritsFrom(TDirectory::Class())) {
      if (m_debug) cout << "Going into " << key->GetName() << endl;
      cout << ".";
      flush(cout);
      TDirectory* source1 = (TDirectory*) key->ReadObj();
      readHistoList(source1, subdir + name + "/");
      delete source1;
    } else if (objclass->InheritsFrom(TH1::Class()) &&
            !objclass->InheritsFrom(TH2::Class()) &&
            !objclass->InheritsFrom(TH3::Class())) {
      TObjArray* res = tagpattern.MatchS(name);
      if(res->GetEntries() < 2){
        cout << "ERROR: found an histo with non-matching name !" << endl;
        cout << name << endl;
        exit(-1);
      }

      // if CoreRegions is switched, remove useless histos.
      TObjArray* res2 = stricttagpattern.MatchS(name);
      if(m_onlyCoreRegions) {
        if(res2->GetEntries() < 2) continue;
        TString ntag = ((TObjString*)(res2->At(4)))->GetString();
        TString var = ((TObjString*)(res2->At(8)))->GetString();
        if( ntag == "2" && var.BeginsWith("MV1")) continue;
        //cout << "INFO: Ignoring histogram : " << name << endl;
      }

      m_histoList[((TObjString*)(res->At(1)))->GetString()].push_back(subdir + name);
      delete res;
      delete res2;
    }
  }
}

void splitter::SplitFile(InputFile &inputFile) {

  // try to get channel sub-directory
  inputFile.dir = inputFile.file;
  TObject* subDir = inputFile.file->Get(inputFile.channel);
  if (subDir)
    inputFile.dir = (TDirectory*) subDir;

  vector<TString> tags;
  vector<TString> bins;
  vector<TString> distros;
  findRegions(inputFile.dir, tags, bins, distros, inputFile.distros);

  cout << "Tag regions   :";
  for (vector<TString>::iterator tag(tags.begin()); tag != tags.end(); ++tag)
    cout << " " << *tag;
  cout << endl;

  cout << "VpT bins      :";
  for (vector<TString>::iterator bin(bins.begin()); bin != bins.end(); ++bin)
    cout << " " << *bin;
  cout << endl;

  cout << "Distributions :";
  for (vector<TString>::iterator dist(distros.begin()); dist != distros.end(); ++dist)
    cout << " " << *dist;
  cout << endl;

  if (tags.size() == 0 || bins.size() == 0 || distros.size() == 0) {
    cout << "WARNING: regions not properly identified! Skipping input file." << endl;
    return;
  }

  cout << "Reading full list of histograms...";
  m_histoList.clear();
  readHistoList(inputFile.dir, "");
  cout << endl;
  for (map<TString, vector<TString> >::iterator it(m_histoList.begin()); it != m_histoList.end(); ++it) {
    cout << "Found " << it->second.size() << " histograms for " << it->first << endl;
  }

  for (vector<TString>::iterator tag(tags.begin()); tag != tags.end(); ++tag) {
    for (vector<TString>::iterator bin(bins.begin()); bin != bins.end(); ++bin) {
      for (vector<TString>::iterator dist(distros.begin()); dist != distros.end(); ++dist) {
        writeOutFile(inputFile, *tag, *bin, *dist);
      }
    }
  }
}

void splitter::writeOutFile(InputFile &inputFile,
        TString tagRegion,
        TString VpTbin,
        TString dist) {
  TString match = tagRegion + "_" + VpTbin + "_" + dist;
  TString outFileName = m_outDirTmp
          + inputFile.energy + "_"
          + inputFile.channel + "_"
          + inputFile.type + "_"
          + match + ".root";

  // hack for inconsistent tag region naming
  outFileName.ReplaceAll("2ltag","2lltag");
  outFileName.ReplaceAll("2mtag","2mmtag");
  outFileName.ReplaceAll("2ttag","2tttag");

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

void splitter::copyHistos(TDirectory* source, TDirectory* target, TString match) {
  // assumptions:
  // just one layer of sub folders (Sys...)
  // m_histoList to be ordered in sub folders (given by readHistoList)

  TString subDirPrev = "";
  // linear looping over all histogram names.
  // not the most efficient way, but uses negligible amount of total runtime.
  for (vector<TString>::iterator hName(m_histoList[match].begin()); hName != m_histoList[match].end(); ++hName) {
    // getting each object from the file by name
    // *hName includes the sub directory name, eg "SysBla/sample_tags_SysBla"
    // also not very efficient, but again uses negligible amount of total runtime
    TObject* obj = source -> Get(*hName);
    if (!obj) {
      cout << "ERROR: histogram '" << *hName << "' in list, but not found in file again!" << endl;
      exit(-1);
    }

    // split name
    TString writeName = *hName;
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

void splitter::copyInputFileToTmp(InputFile &inputFile) {
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

    cmd = "/afs/cern.ch/project/eos/installation/0.3.1-22/bin/eos.select cp -k ";
    inDir.ReplaceAll("root://eosatlas//", "/");
  }
  system(cmd + inDir + inputFile.fileName + " " + m_outDirTmp + subDir);
}

void splitter::moveOutputFromTmp() {
  cout << "Moving output files from " << m_outDirTmp << " to " << m_outDirLocal << endl;
  for (vector<TString>::iterator fileName(m_outputFiles.begin()); fileName != m_outputFiles.end(); ++fileName) {
    // only move non-empty files
    system("((`/usr/bin/stat -c%s " + *fileName + "` < 1000)) || mv -v " + *fileName + " " + m_outDirLocal);
  }
}

bool splitter::pushUnique(vector<TString> &vec, TString &str) {
  bool found = false;
  for (unsigned int i = 0; i < vec.size(); i++)
    found |= (vec[i] == str);
  if (!found) vec.push_back(str);
  return found;
}

TString splitter::getNameField(TString histoName, int field) {
  TObjArray* nameArray = histoName.Tokenize("_");
  if (field >= nameArray->GetEntries())
    return "ERR";
  TObjString * objString = (TObjString*) nameArray->At(field);
  TString fieldString = objString->String();
  return fieldString;
}

void splitter::findRegions(TDirectory* dir,
          vector<TString> &tagRegions,
          vector<TString> &vptBins,
          vector<TString> &distributions,
	  TString distToUse,
	  TString anySample) {
  cout << "Reading regions for sample '" << anySample << "'..." << endl;
  if (distToUse != "")
    cout << "Limit distributions to : "<< distToUse << endl;
  tagRegions.clear();
  vptBins.clear();
  distributions.clear();
  TKey* key;
  TIter nextkey(dir->GetListOfKeys());
  while ((key = (TKey*) nextkey())) {
    if (TClass::GetClass(key->GetClassName())->InheritsFrom(TH1::Class())) {
      string name = key->GetName();
      if (getNameField(name, 0).BeginsWith(anySample)) {
        // check upper vpt bound
        // TODO: better way?
        TString testField = getNameField(name, 3);
        bool hasUpperVpt = (testField.BeginsWith("90") ||
                            testField.BeginsWith("12") ||
                            testField.BeginsWith("16") ||
                            testField.BeginsWith("20"));
        // tag regions
        TString regionName = getNameField(name, 1);
        pushUnique(tagRegions, regionName);
        // vpt bins
        regionName = getNameField(name, 2);
        if (hasUpperVpt)
          regionName += "_" + getNameField(name, 3);
        pushUnique(vptBins, regionName);
        // distributions
        regionName = getNameField(name, 3);
        if (hasUpperVpt)
          regionName = getNameField(name, 4);
	if (distToUse == "" || distToUse.Contains(regionName))
	  pushUnique(distributions, regionName);
      }
    }
  }
}
