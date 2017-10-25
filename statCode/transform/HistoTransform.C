#include <HistoTransform.h>

#include <cmath>
#include <iostream>

#include <TClass.h>
#include <TROOT.h>
#include <TIterator.h>
#include <TKey.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TH1.h>
#include <TSystem.h>
#include <TFile.h>

using namespace std;

HistoTransform::HistoTransform() {
    doMergeKFolds = true;
    doTransformBeforeMerging = false;
    transformAlgorithm = 6;
    trafoTwoRebin = 5;
    trafoFiveY = 5.;
    trafoSixY = 10.;
    trafoSixZ = 10.;
    trafoSevenNBins = 20;
    trafoEightRebin = 20;
    trafoEzSig = 16;
    trafoEzBkg = 4;
    trafoFzSig = 5;
    trafoFzBkg = 1;
    transformBkgBDTs = true;
}

HistoTransform::HistoTransform(TString inFileName, TString outFileName) :
  HistoTransform() {
    m_inFileName = inFileName;
    m_outFileName = outFileName;

    std::cout << "INFO: reading from file '" << m_inFileName.Data() << "'" << std::endl;
    m_inFile = new TFile(m_inFileName, "read");
    if (!m_inFile -> IsOpen()) {
      gSystem -> Exit(1);
    }

    if (m_outFileName == "") {
      m_outFileName = m_inFileName;
      m_outFileName.ReplaceAll(".root", ".transformed.root");
    }
    std::cout << "INFO: writing to file '" << m_outFileName.Data() << "'" << std::endl;
    m_outFile = new TFile(m_outFileName, "recreate");
}

// get a vector of bins where to rebin to get an uncertainty <= 5% per bin.
// starting from highest bin!
// the numbers give the lowest bin included in the new bin
// overflowbin+1 and underflow bins are returned as the first and last element in the vector, respectively.

vector<int> HistoTransform::getRebinBins(TH1* histoBkg, TH1* histoSig, int method, double maxUnc) {

  vector<int> bins;
  bins.clear();

  if (!histoBkg || !histoSig) {
    cout << "ERROR: please provide signal and background histograms!" << endl;
    gSystem -> Exit(1);
  }

  //parameters requires for cases 9/10
  int nBinsDesired= 20; //Target number of final bins
  double minSB = 3; //Minimum total number of events per bin
  double minBkg = 0.5; //Minimum number of background events per bin
  int iBin2 = 0;
  
  int nBins = histoBkg -> GetNbinsX();
  int iBin = nBins + 1; // start with overflow bin
  bins.push_back(nBins + 2);

  float nBkg = histoBkg -> Integral(0, nBins + 1);
  float nSig = histoSig -> Integral(0, nBins + 1);

  double nLLRs = getLLR(histoSig, histoBkg);
  double nLLRb = getLLR(histoBkg, histoSig);
  
  while (iBin > 0) {

    double sumBkg = 0;
    double sumBkgL = 0;
    double sumSigBkg = 0;
    double sumSig = 0;
    double sumSigL = 0;
    double err2Bkg = 0;
    double err2SigBkg = 0;
    bool pass = false;
    int binCount = 1;

    double dist = 1e10;
    double distPrev = 1e10;

    while (!pass && iBin >= 0) {
      double nBkgBin = histoBkg -> GetBinContent(iBin);
      double nSigBin = histoSig -> GetBinContent(iBin);
      sumBkg += nBkgBin;
      sumSig += nSigBin;
      sumSigBkg += nBkgBin + nSigBin;
      if (nBkgBin > 0 && nSigBin > 0) {
        sumBkgL += nBkgBin * log(1 + nBkgBin / nSigBin);
        sumSigL += nSigBin * log(1 + nSigBin / nBkgBin);
      }
      err2Bkg += pow(histoBkg -> GetBinError(iBin), 2);
      err2SigBkg += pow(histoBkg -> GetBinError(iBin), 2) + pow(histoSig->GetBinError(iBin),2);
 
      double err2RelBkg = 1;
      double err2RelSigBkg = 1;
      if (sumBkg != 0) {
        err2RelBkg = err2Bkg / pow(sumBkg, 2);
        err2RelSigBkg = err2SigBkg / pow(sumSigBkg, 2);
      }
 
      float err2Rel = 1;
      float err2MaxBkg = pow(maxUnc, 2);

      switch (method) {
        case 1:
          // METHOD 1 : err(Bkg) < XX%
          pass = sqrt(err2RelBkg) < maxUnc;
          break;
        case 2:
          // similar to TH1:Rebin() + err(Bkg) < XX%
          pass = (binCount >= trafoTwoRebin);
	  pass &= sqrt(err2RelBkg) < maxUnc;
          break;
        case 5:
          if (sumBkg != 0 && sumSig != 0)
            err2Rel = 1 / (err2MaxBkg / err2RelBkg + sumSig / (nSig / trafoFiveY));
          else if (sumBkg != 0)
            err2Rel = err2RelBkg / err2MaxBkg;
          else if (sumSig != 0)
            err2Rel = (nSig / trafoFiveY) / sumSig;
          pass = sqrt(err2Rel) < 1;
          break;
        case 6:
          // "trafo D"
          if (sumBkg != 0 && sumSig != 0)
            err2Rel = 1 / (sumBkg / (nBkg / trafoSixZ) + sumSig / (nSig / trafoSixY));
          else if (sumBkg != 0)
            err2Rel = (nBkg / trafoSixZ) / sumBkg;
          else if (sumSig != 0)
            err2Rel = (nSig / trafoSixY) / sumSig;
          pass = sqrt(err2Rel) < 1;
	  // distance
	  dist = fabs(err2Rel - 1);
          break;
        case 7:
          // flat bkg
          pass = sumBkg > nBkg / trafoSevenNBins;
          break;
        case 8:
          // similar to TH1:Rebin()
          pass = (binCount >= trafoEightRebin);
          break;
      case 9:
	//Rebin based only on statistical uncertainty and bin content
	pass = (sqrt(err2RelSigBkg)<0.2 && sqrt(err2RelBkg)<0.2 && sumBkg>minBkg && sumSigBkg>minSB);
	break;
      case 10:
	//Keep highest MVA bin as is, and rebin rest with quadratic requirement on bin content.
	//Quadratic term depends on number if target bins. Should rebin first depending on statistical uncertainty
	//and bin content so that high MVA bin is reasonable (ie, run case 9 before case 10).
	pass = iBin==nBins ? 1 : (sumSigBkg>(minSB*1.5+iBin2*iBin2*sqrt(nBinsDesired)/2) && sumBkg>minBkg && sqrt(err2RelBkg)<0.2 && sqrt(err2RelSigBkg)<0.2);
	break;
        case 11:
          // "trafo E" with 5% bkg stat unc
          if (sumBkgL != 0 && sumSigL != 0)
            err2Rel = 1 / (sqrt(sumBkgL / (nLLRb / trafoEzBkg)) + sqrt(sumSigL / (nLLRs / trafoEzSig)));
          else if (sumBkgL != 0)
            err2Rel = sqrt((nLLRb / trafoEzBkg) / sumBkgL);
          else if (sumSigL != 0)
            err2Rel = sqrt((nLLRs / trafoEzSig) / sumSigL);
          pass = sqrt(err2Rel) < 1 && sqrt(err2RelBkg) < 0.05;
          break;
        case 12:
          // "trafo F" with 5% bkg stat unc
          if (sumBkg != 0 && sumSigL != 0)
            err2Rel = 1 / (sqrt(sumBkg / (nBkg / trafoFzBkg)) + sqrt(sumSigL / (1 / trafoFzSig)));
          else if (sumBkg != 0)
            err2Rel = sqrt((nBkg / trafoFzBkg) / sumBkg);
          else if (sumSigL != 0)
            err2Rel = sqrt((1 / trafoFzSig) / sumSigL);
          pass = sqrt(err2Rel) < 1 && sqrt(err2RelBkg) < 0.10;
          break;
      default:
	cout << "ERROR: transformation method '" << method << "' unknown!" << endl;
	gSystem -> Exit(1);
	break;
      }
      if (!(pass && dist > distPrev)) {
	binCount++;
	iBin--;
      } // else use previous bin
      distPrev = dist;
    }
    iBin2++;
    // remove last bin
    if (iBin + 1 == 0 && bins.size() > 1) {
      if (method != 6 && method != 7) {
        bins.pop_back();
      } else if (method == 6 && bins.size() > trafoSixY + trafoSixZ + 0.01) {
        // remove last bin if Nbin > Zsig + Zbkg
        // (1% threshold to capture rounding issues)
        bins.pop_back();
      }
    }
    bins.push_back(iBin + 1);
  }

  return bins;
}

// rebin histogram with provided vector of bins

void HistoTransform::rebinHisto(TH1* histo, vector<int>* bins, bool equidist, bool relabel) {

  TH1* histoOld = (TH1*) histo -> Clone();
  histo -> Reset();
  int newNBins = bins -> size() - 1;

  if(equidist) {
    histo -> SetBins(newNBins, -1, 1);
  } else {
   
    const int nBinEdges = bins->size(); // add one more bin for plotting
    Double_t* binEdges = new Double_t[nBinEdges];
    for (int iBinEdge = 0; iBinEdge < nBinEdges; iBinEdge++) {
      int iBin = (*bins)[nBinEdges - iBinEdge - 1]; // vector is reverse-ordered
      if (iBin == 0)
        iBin = 1;
      if (iBin == histoOld -> GetNbinsX() + 2)
        iBin = histoOld -> GetNbinsX() + 1;
      binEdges[iBinEdge] = histoOld -> GetBinLowEdge(iBin);
    }
    //    binEdges[nBinEdges - 1] = 1.8; // additional bin for plotting
    histo -> SetBins(newNBins, binEdges);
  }

  for (int iBinNew = 1; iBinNew <= newNBins; iBinNew++) {
    int iBinLow = bins -> at(newNBins - iBinNew + 1); // vector is reverse-ordered
    int iBinHigh = bins -> at(newNBins - iBinNew) - 1;

    double err = 0;
    double sum = histoOld -> IntegralAndError(iBinLow, iBinHigh, err);
    histo -> SetBinContent(iBinNew, sum);
    histo -> SetBinError(iBinNew, err);
    if(relabel) {
      float xlow = histoOld -> GetXaxis() -> GetBinLowEdge(iBinLow);
      float xhigh = histoOld -> GetXaxis() -> GetBinLowEdge(iBinHigh+1);
      histo-> GetXaxis() -> SetBinLabel(iBinNew, TString::Format("%i to %i", (int)xlow, (int)xhigh));
    }
  }
  int oldNbins = histoOld -> GetNbinsX();
  double diff = 1 - histo -> Integral(0, newNBins + 1) / histoOld -> Integral(0, oldNbins + 1);
  //  double diff = 1 - histo -> GetSumOfWeights() / histoOld -> GetSumOfWeights();
  if (TMath::Abs(diff) > 1e-7) {
    cout << "WARNING: sizeable difference in transformation of '" << histo -> GetName() << "' found. Integrals: (old-new)/old = " << diff << endl;
  }
  delete histoOld;
}

double HistoTransform::getLLR(TH1* sig, TH1* bkg) {
  int nBin = sig -> GetNbinsX();
  double llr = 0;
  for (int i = 0; i <= nBin + 1; i++) {
    double nSig = sig -> GetBinContent(i);
    double nBkg = bkg -> GetBinContent(i);
    if (nBkg <= 0 || nSig <= 0)
      continue;
    llr += nSig * log(1 + nSig / nBkg);
  }
  return llr;
}


void HistoTransform::readRebinBins(SubDirectory* subDir) {
  for (unsigned int iKFold = 0; iKFold < subDir -> kFolds.size(); iKFold++) {
    KFold* kFold = subDir -> kFolds[iKFold];
    for (unsigned int iRegion = 0; iRegion < kFold -> regions.size(); iRegion++) {
      Region* region = kFold -> regions[iRegion];
      cout << "INFO: building transformation for region '" << region -> name << "':";
      region -> bins = getRebinBins(region -> bkg, region -> sig, transformAlgorithm, region -> maxUnc);
      cout << " Nbins = " << region -> bins.size() - 1 << endl;
    }
  }
}

HistoTransform::KFold* HistoTransform::getKFold(SubDirectory* subDir, string histName) {
  for (unsigned int iKFold = 0; iKFold < subDir -> kFolds.size(); iKFold++) {
    KFold* kFold = subDir -> kFolds[iKFold];
    if (kFold -> name == getRegionName(histName))
      return kFold;
  }
  //  cout << "WARNING: kFold for histo '" << histName << "' not found!" << endl;
  return 0;
}

HistoTransform::Region* HistoTransform::getRegion(SubDirectory* subDir, string histName) {
  KFold* kFold = getKFold(subDir, histName);
  if (!kFold)
    return 0;
  if (kFold -> regions.size() == 1)
    return kFold -> regions[0];
  TString name = histName;
  for (unsigned int iRegion = 0; iRegion < kFold -> regions.size(); iRegion++) {
    TString stringIofK = TString::Format("_%iof%i", iRegion, (int) kFold -> regions.size());
    if (name.Contains(stringIofK))
      return kFold -> regions[iRegion];
  }
  //  cout << "WARNING: region for histo '" << histName << "' not found!" << endl;
  return 0;
}

bool HistoTransform::isFirstOfKFold(KFold* kFold, string histName) {
  if (!kFold)
    return false;
  TString string0ofK = TString::Format("_0of%i", (int) kFold -> regions.size());
  TString name = histName.c_str();
  if (name.Contains(string0ofK))
    return true;
  return false;
}

// go recursively through directories in source.
// rebin all histos found on the way and save same to gDirectory.

void HistoTransform::rebinAllHistos(TDirectory* source, SubDirectory* subDir) {
  TDirectory* savdir = gDirectory;
  TDirectory* adir = savdir;
  if (source != m_inFile) {
    adir = savdir -> mkdir(source -> GetName());
  }
  adir -> cd();
  //loop on all entries of this directory
  TKey* key;
  TIter nextkey(source -> GetListOfKeys());
  while ((key = (TKey*) nextkey())) {
    TClass* cl = gROOT -> GetClass(key -> GetClassName());
    if (!cl) continue;
    if (cl -> InheritsFrom(TDirectory::Class())) {
      cout << "INFO: going into directory '" << key -> GetName() << "'" << endl;
      source -> cd(key -> GetName());
      TDirectory* subdir = gDirectory;
      adir -> cd();
      rebinAllHistos(subdir, subDir);
      adir -> cd();
    } else if (cl -> InheritsFrom(TH1::Class())) {
      string name = key -> GetName();
      TH1* histo = (TH1*) source -> Get(name.c_str());
      adir -> cd();
      Region* region = getRegion(subDir, name);
      if (region) {
        //        cout << "INFO: applying transformation for region '" << region -> name << "' to histogram '" << name << "'" << endl;
        //        cout << getSampleName(name) << " : " << getRegionName(name) << " : " << getMVATag(name) << " : " << getSysName(name) << endl;
        rebinHisto(histo, &region -> bins);
        histo -> Write();
      }
      delete histo;
    }
  }
  adir -> SaveSelf(kTRUE);
  savdir -> cd();
}

void HistoTransform::mergeKFolds(TDirectory* source, SubDirectory* subDir) {
  TDirectory* savdir = gDirectory;
  TDirectory* adir = source;
  //    if (source != m_inFile) {
  //      adir = savdir -> mkdir(source -> GetName());
  //    }
  adir -> cd();
  //loop on all entries of this directory
  TKey* key;
  TIter nextkey(source -> GetListOfKeys());
  while ((key = (TKey*) nextkey())) {
    TClass* cl = gROOT -> GetClass(key -> GetClassName());
    if (!cl) continue;
    if (cl -> InheritsFrom(TDirectory::Class())) {
      cout << "INFO: going into directory '" << key -> GetName() << "'" << endl;
      source -> cd(key -> GetName());
      TDirectory* subdir = gDirectory;
      adir -> cd();
      mergeKFolds(subdir, subDir);
      adir -> cd();
    } else if (cl -> InheritsFrom(TH1::Class())) {
      string name = key -> GetName();
      TH1* histo = (TH1*) source -> Get(name.c_str());
      adir -> cd();
      KFold* kFold = getKFold(subDir, name);
      if (isFirstOfKFold(kFold, name)) {
        TString string0ofK = TString::Format("_0of%i", (int) kFold -> regions.size());
        TString nameMerged = name.c_str();
        nameMerged.ReplaceAll(string0ofK, "");
        TH1* histoMerged = (TH1*) histo -> Clone(nameMerged);
        bool foundAllFolds = true;
        for (unsigned int iRegion = 1; iRegion < kFold -> regions.size(); iRegion++) {
          //region = kFold -> regions[iRegion];
          TString foldName = name.c_str();
          //foldName.ReplaceAll(nameFirstRegion, region -> name.c_str());
          TString stringIofK = TString::Format("_%iof%i", iRegion, (int) kFold -> regions.size());
          foldName.ReplaceAll(string0ofK, stringIofK);
          TObject* objHisto = source -> Get(foldName);
          if (objHisto) {
            histo = (TH1*) objHisto;
            histoMerged -> Add(histo);
            delete histo;
            //cout << "added " << iRegion << endl;
          } else {
            cout << "WARNING: histogram '" << foldName << "' not found! Not writing merged output." << endl;
            foundAllFolds = false;
          }
        }
        if (foundAllFolds)
          histoMerged -> Write();
        delete histoMerged;
      }
    }
  }
  adir -> SaveSelf(kTRUE);
  savdir -> cd();
}

void HistoTransform::readTotalBkg(SubDirectory* subDir) {

  for (unsigned int iKFold = 0; iKFold < subDir -> kFolds.size(); iKFold++) {
    KFold* kFold = subDir -> kFolds[iKFold];
    TH1* kFoldBkg = 0;
    for (unsigned int iRegion = 0; iRegion < kFold -> regions.size(); iRegion++) {
      Region* region = kFold -> regions[iRegion];
      cout << "INFO: reading total background for region '" << region -> name << "'" << endl;
      TH1* histoBkg = 0;
      for (unsigned int iBkg = 0; iBkg < subDir -> backgrounds.size(); iBkg++) {

        string histoName = subDir -> backgrounds[iBkg] + "_" + region -> name;
        TObject* objHisto = subDir -> dir -> Get(histoName.c_str());
        if (objHisto == 0) {
          cout << "ERROR: background '" << subDir -> backgrounds[iBkg] << "' not found!" << endl;
          gSystem -> Exit(1);
        }
        if (!histoBkg) {
          histoBkg = (TH1*) objHisto -> Clone("totalBkg");
          histoBkg -> SetDirectory(0);
        } else {
          histoBkg -> Add((TH1*) objHisto);
        }
        if (!kFoldBkg) {
          kFoldBkg = (TH1*) objHisto -> Clone("kFoldBkg");
          kFoldBkg -> SetDirectory(0);
        } else {
          kFoldBkg -> Add((TH1*) objHisto);
        }
      }

      //  histoBkg -> Draw();
      if (!histoBkg) {
        cout << "ERROR: total background not found!" << endl;
        gSystem -> Exit(1);
      }
      region -> bkg = histoBkg;
    }
    if (doMergeKFolds && !doTransformBeforeMerging) {
      for (unsigned int iRegion = 0; iRegion < kFold -> regions.size(); iRegion++) {
        Region* region = kFold -> regions[iRegion];
        region -> bkg = kFoldBkg;
      }
    }
  }
}

void HistoTransform::readSignal(SubDirectory* subDir) {

  if (subDir -> signal == "") {
    cout << "WARNING: signal (possibly used for transformation) not defined!" << endl;
    return;
  }
  for (unsigned int iKFold = 0; iKFold < subDir -> kFolds.size(); iKFold++) {
    KFold* kFold = subDir -> kFolds[iKFold];
    TH1* kFoldSignal = 0;
    for (unsigned int iRegion = 0; iRegion < kFold -> regions.size(); iRegion++) {
      Region* region = kFold -> regions[iRegion];
      cout << "INFO: reading signal for region '" << region -> name << "'" << endl;
      TH1* histoSignal = 0;
      string histoName = subDir -> signal + "_" + region -> name;
      TObject* objHisto = subDir -> dir -> Get(histoName.c_str());
      if (objHisto == 0) {
        cout << "ERROR: signal histogram not found!" << endl;
        gSystem -> Exit(1);
      }
      histoSignal = (TH1*) objHisto -> Clone("signal");
      histoSignal -> SetDirectory(0);
      if (!kFoldSignal) {
        kFoldSignal = (TH1*) objHisto -> Clone("kFoldSignal");
        kFoldSignal -> SetDirectory(0);
      } else {
        kFoldSignal -> Add((TH1*) objHisto);
      }
      region -> sig = histoSignal;
    }
    if (doMergeKFolds && !doTransformBeforeMerging) {
      for (unsigned int iRegion = 0; iRegion < kFold -> regions.size(); iRegion++) {
        Region* region = kFold -> regions[iRegion];
        region -> sig = kFoldSignal;
      }
    }
  }
}

void HistoTransform::findRegions(SubDirectory* subDir, string anyHistoName, float maxUnc, string containRegion, int nFold) {

  cout << "INFO: looking for regions in directory '" << subDir -> dir -> GetName() << "'" << endl;
  TKey* key;
  TIter nextkey(subDir -> dir -> GetListOfKeys());
  while ((key = (TKey*) nextkey())) {
    TClass* cl = gROOT -> GetClass(key -> GetClassName());
    if (!cl) continue;
    if (cl -> InheritsFrom(TH1::Class())) {
      string name = key -> GetName();
      if (getSampleName(name) == anyHistoName) {
        string regionName = getRegionName(name);
        TString regionNameT(regionName.c_str());
        bool addThis = containRegion == "" || regionNameT.Contains(containRegion);
        //        addThis &= regionNameT.Contains("_mva");
        if (addThis) {
          cout << "INFO: found region '" << regionName << "'" << endl;
          addRegion1(subDir, regionName, maxUnc, nFold);
        }
      }
    }
  }

  if (subDir -> kFolds.size() == 0) {
    cout << "ERROR: no histogram found! maybe there is no histgram named '" << anyHistoName << "'" << endl;
    gSystem -> Exit(1);
  }
}

HistoTransform::SubDirectory* HistoTransform::findSubDirs(string subDirName, bool identical) {

  if (identical && subDirName == "") {
    SubDirectory* subDir = new SubDirectory();
    subDir -> dir = m_inFile;
    m_subDirs.push_back(subDir);
    return subDir;
  }

  TKey* key;
  TIter nextkey(m_inFile -> GetListOfKeys());
  while ((key = (TKey*) nextkey())) {
    TClass* cl = gROOT -> GetClass(key -> GetClassName());
    if (!cl) continue;
    if (cl -> InheritsFrom(TDirectory::Class())) {
      TString name = key -> GetName();
      if ((name.Contains(subDirName) && !identical) || name.EqualTo(subDirName)) {
        cout << "INFO: found subdirectory '" << name.Data() << "'" << endl;
        SubDirectory* subDir = new SubDirectory();
        m_inFile -> cd(name);
        subDir -> dir = gDirectory;
        m_subDirs.push_back(subDir);
        if (identical)
          return subDir;
        else
          addBackground(subDir, "bkg");
      }
    }
  }

  if (m_subDirs.size() == 0 && !identical) {
    cout << "INFO: no subdirectory found, using root directory" << endl;
    SubDirectory* subDir = new SubDirectory();
    subDir -> dir = m_inFile;
    m_subDirs.push_back(subDir);
    addBackground(subDir, "bkg");
    return subDir;
  }
  if (identical) {
    cout << "WARNING: subdirectory '" << subDirName << "' not found!" << endl;
  }
  return 0;
}

void HistoTransform::skimKFoldTrafos(SubDirectory * subDir) {
  cout << "INFO: skimming kFold transformations (for same number of bins in each fold)" << endl;
  for (unsigned int iKFold = 0; iKFold < subDir -> kFolds.size(); iKFold++) {
    KFold* kFold = subDir -> kFolds[iKFold];
    int nFold = kFold -> regions.size();
    if (nFold <= 1)
      continue;
    int minBins = -1;
    for (int iFold = 0; iFold < nFold; iFold++) {
      int nBins = kFold -> regions[iFold] -> bins.size() - 1;
      if (minBins > nBins || minBins == -1)
        minBins = nBins;
    }
    cout << "INFO: minimum number of bins for kFold '" << kFold -> name << "':" << minBins << endl;
    for (int iFold = 0; iFold < nFold; iFold++) {
      vector<int>* bins = &kFold -> regions[iFold] -> bins;
      while ((int) bins -> size() - 1 > minBins) {
        bins -> pop_back();
        bins -> pop_back();
        bins -> push_back(0);
      }
    }
  }
}

void HistoTransform::addRegion1(SubDirectory* subDir, string regionName, float maxUnc, int nFold) {

  KFold* kFold = new KFold();
  for (int iFold = 0; iFold < nFold; iFold++) {
    Region* region = new Region();
    region -> name = regionName;
    region -> sig = 0;
    if (nFold > 1)
      region -> name += TString::Format("_%iof%i", iFold, nFold);
    region -> maxUnc = maxUnc;
    kFold -> regions.push_back(region);
    kFold -> name = regionName;
    //subDir -> regions.push_back(region);
  }
  //if (nFold > 1)
  subDir -> kFolds.push_back(kFold);
}

string HistoTransform::getNameField(string histoName, int field) {

  TString histNameT(histoName.c_str());
  TObjArray* nameArray = histNameT.Tokenize("_");
  string fieldString = "";
//  bool tagFieldFound = false;
  for (int i = 0; i < nameArray -> GetEntries(); i++) {
    TObjString* objString = (TObjString*) nameArray -> At(i);
//    bool isTagField = false; //objString -> String().Contains("jet");
//    isTagField |= objString -> String().Contains("tag");
//    isTagField |= objString -> String().Contains("topcr");
//    isTagField |= objString -> String().Contains("topemucr");
//    tagFieldFound |= isTagField;
//    if (!tagFieldFound && i > 0) {
//      field++;
//      continue;
//    }
    if (i == field) {
      fieldString = objString -> String().Data();
      break;
    }
  }
  nameArray -> Delete();
//  if (!tagFieldFound) {
//    cout << "WARNING: tag field in '" << histoName << "' not found!" << endl;
//  }
  return fieldString;
}

string HistoTransform::getSampleName(string histoName) {

  string name = getNameField(histoName, 0);
  if (name == "stop") {
    name += getNameField(histoName, 1);
  }
  return name;
}

string HistoTransform::getRegionName(string histoName) {

  int start = 1;
  string name = getNameField(histoName, 0);
  if (name == "stop") {
    start = 2;
  }

  string region = "";
  bool go = true;
  while (go) {
    TString test = getNameField(histoName, start);
    if (test == "" || test.BeginsWith("Sys")) {
      go = false;
    } else {
      if (region != "")
        region += "_";
      region += test;
    }
    start++;
  }
  return region;
}

string HistoTransform::getMVATag(string histoName) {
  cout << "WARNING: function deprecated! Called with: " << histoName << endl;
  return "";
}

string HistoTransform::getSysName(string histoName) {
  
  bool go = true;
  int start = 0;
  while (go) {
    TString test = getNameField(histoName, start);
    if (test == "") {
      go = false;
    } else if (test.BeginsWith("Sys")) {
      return test.Data();
    }
    start++;
  }
  return "";
}

// PUBLIC

HistoTransform::SubDirectory* HistoTransform::addSubDirectory(string subDirName) {
  return findSubDirs(subDirName, true);
}

void HistoTransform::addBackground(SubDirectory* subDir, string bkgName) {
  subDir -> backgrounds.push_back(bkgName);
}

void HistoTransform::setSignal(SubDirectory* subDir, string signalName) {
  subDir -> signal = signalName;
}

void HistoTransform::addRegion(SubDirectory* subDir, string regionName, float maxUnc, int nFold) {
  
  if (transformBkgBDTs)
    findRegions(subDir, "data", maxUnc, regionName);
  else
    addRegion1(subDir, regionName, maxUnc, nFold);
}

void HistoTransform::run() {

  if (m_subDirs.size() == 0) {
    string subDirName = "Lepton";
    cout << "INFO: no subdirectories defined, looking for those containing '" << subDirName << "'" << endl;
    findSubDirs(subDirName, false);
  }

  for (unsigned int iSubDir = 0; iSubDir < m_subDirs.size(); iSubDir++) {
    SubDirectory* subDir = m_subDirs[iSubDir];
    if (subDir -> kFolds.size() == 0)
      findRegions(subDir, "data");
    readTotalBkg(subDir);
    readSignal(subDir);
    cout << "INFO: using transformation algorithm '" << transformAlgorithm << "'" << endl;
    readRebinBins(subDir);
    skimKFoldTrafos(subDir);
    cout << "INFO: applying transformations" << endl;
    m_outFile -> cd();
    rebinAllHistos(subDir -> dir, subDir);
    if (doMergeKFolds) {
      cout << "INFO: merging KFolds" << endl;
      m_outFile -> cd();
      mergeKFolds(m_outFile, subDir);
    }
  }
  //cout << "INFO: closing input file" << endl;
  //m_inFile -> Close();
  //cout << "INFO: closing output file" << endl;
  //m_outFile -> Close();
}
