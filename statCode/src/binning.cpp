#include "binning.hpp"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <forward_list>

#include <TMath.h>
#include <TAxis.h>
#include <TH1.h>
#include <TH2.h>
#include <TMathBase.h>

#include "category.hpp"
#include "configuration.hpp"

#include "HistoTransform.h"

using namespace std;

std::unique_ptr<BinningTool> BinningTool::the_instance(nullptr);

BinningTool::BinningTool(const Configuration& conf) :
  m_config(conf),
  m_htrafo(new HistoTransform())
{}

void BinningTool::createInstance(const Configuration& conf) {
  if( !the_instance ) {
    cout << "INFO:    BinningTool::createInstance() BinningTool pointer is NULL." << endl;
    cout << "         Will instanciate the BinningTool service first." << endl;
    the_instance.reset(new BinningTool(conf));
  }
  else {
    cout << "WARNING: BinningTool::createInstance() BinningTool pointer already exists." << endl;
    cout << "         Don't do anything !" << endl;
  }
}

BinningTool& BinningTool::getInstance() {
  if( the_instance  == nullptr ) {
    cout << "ERROR:    BinningTool::getInstance() BinningTool pointer is NULL." << endl;
    cout << "          It should be initialized with a Configuration first." << endl;
    throw;
  }
  return *the_instance;
}

std::vector<int> BinningTool::getCategoryBinning(const Category& c) {

  int forceRebin = m_config.getValue("ForceBinning", 0);
  if(forceRebin > 0) {
    return {forceRebin};
  }

  // do not do anything
  return std::vector<int>();

}

void BinningTool::changeRange(TH1* h, const Category& c)
{}

void BinningTool::changeRangeAfter(TH1* h, const Category& c)
{}


std::vector<int> BinningTool::collapseEmptyBins(const Category& c) {
  TH1* sig = c.getSigHist();
  TH1* bkg = c.getBkgHist();
  bkg->Add(sig);
  vector<int> bins;
  int nbins = bkg->GetNbinsX();
  bins.push_back(nbins + 1);
  for(int b = nbins; b > 0; b--) {
    if(bkg->GetBinContent(b) > 1e-6) {
      bins.push_back(b);
    }
  }
  delete sig;
  delete bkg;
  return bins;
}

void BinningTool::emptyNegativeBins(TH1* h) {
  for(int i=1; i<h->GetNbinsX()+1; i++) {
    if(h->GetBinContent(i) < 0) {
      h->SetBinContent(i, 0);
    }
  }
}

std::vector<int> BinningTool::getBinsFromEdges(const TH1* histo, const std::vector<float> &edges) {

  if(edges.size()<2){
    cout << "BinningTool::ERROR : At lest one bin is required (Two edges) " << endl;
    throw;
  }

  if(edges.at(edges.size()-1)!=histo->GetBinLowEdge(1)) {
    cout << "BinningTool::ERROR : Provided low edge and histogram edge don't match" << endl;
    cout << edges.at(edges.size()-1) << " " << histo->GetBinLowEdge(1) << endl;
    throw;
  }

  if(edges.at(0)!=histo->GetBinLowEdge(histo->GetNbinsX()+1 )) {
    cout << "BinningTool::ERROR : Provided high and histogram edge don't match" << endl;
    cout << edges.at(0) << " " << histo->GetBinLowEdge(histo->GetNbinsX()+1 ) << endl;
    throw;
  }
  

  for(unsigned int i=0;i<edges.size()-1;i++){
    if(edges.at(i) < edges.at(i+1) ){
      cout << "BinningTool::ERROR : Edges in decreasing order require " << endl;
      throw;
    }
  }

  std::vector<int> bins;  
  for(unsigned int i=0;i<edges.size();i++){
    bins.push_back(histo->FindFixBin(edges.at(i)));
  }
  
  return bins;

}

std::vector<int> BinningTool::getBinningForSysts(const TH1* h) {
  // for now, use rebinning with 5% MC

  // HistoTransform implement it, yay !
  // shortcomings:
  // * cannot chose direction (left to right or right to left)
  // * need to pass 2 histogram pointers, one of them will not be accessed
  // * use TH1*, not const TH1*

  TH1* h2 = const_cast<TH1*>(h);
  return m_htrafo->getRebinBins(h2, h2, 1, 0.05);
}

void BinningTool::applyBinning(TH1* h, const std::vector<int>& bins, TH2* corr) {

  for (int i = 0; i <= h -> GetNbinsX() + 1; i++) {
    double sum = h -> GetBinContent(i);
    //if (sum < 0) {
    //  std::cout << "BinningTool::WARNING: negative bin content before binning! histo = "
    //		<< h -> GetName() << " bin = " << i << " content = " << sum
    //		<< std::endl;
    //}
    if (sum != 0 && sum/sum != sum/sum) {
      std::cout << "BinningTool::WARNING: infinite bin content before binning! histo = "
		<< h -> GetName() << " bin = " << i << " content = " << sum
		<< std::endl;
    }
    if (sum != sum) {
      std::cout << "BinningTool::WARNING: NaN bin content before binning! histo = "
		<< h -> GetName() << " bin = " << i << " content = " << sum
		<< std::endl;
    }
  }

  if(bins.empty()) { // then no rebinning necessary ; do nothing !
    emptyNegativeBins(h);
    return;
  }

  TH1* histoOld = (TH1*) h-> Clone();
  std::forward_list<std::pair<int, int>> bins_merged;

  if(bins.size() == 1) { // only 1 number: use simple Rebin
    int rebinSize = bins[0];
    for (int iOldBin = 1; iOldBin <= histoOld->GetXaxis()->GetNbins(); iOldBin += rebinSize + 1) {
      int upperOldBin = iOldBin + rebinSize;
      bins_merged.push_front(std::make_pair(iOldBin, upperOldBin > histoOld->GetXaxis()->GetNbins() ? histoOld->GetXaxis()->GetNbins() : upperOldBin));
    }
    h->Rebin(rebinSize);
  }
  else {
    // then copy from makeWS
    TAxis* xaxis = h->GetXaxis();
    double xmin = xaxis->GetXmin();
    double xmax = xaxis->GetXmax();

    h-> Reset();
    int newNBins = bins.size() - 1;
    h-> SetBins(newNBins, xmin, xmax);

    for (int iBinNew = 1; iBinNew <= newNBins; ++iBinNew) {
      int iBinLow = bins.at(newNBins - iBinNew + 1); // vector is reverse-ordered
      int iBinHigh = bins.at(newNBins - iBinNew) - 1;
      bins_merged.push_front(std::make_pair(iBinLow, iBinHigh));

      double err = 0; // root requires this to be a double...jeez
      float sum = histoOld -> IntegralAndError(iBinLow, iBinHigh, err);
      h-> SetBinContent(iBinNew, sum);
      h-> SetBinError(iBinNew, err);
      //float xlow = histoOld -> GetXaxis() -> GetBinLowEdge(iBinLow);
      //float xhigh = histoOld -> GetXaxis() -> GetBinLowEdge(iBinHigh+1);
      //h-> GetXaxis() -> SetBinLabel(iBinNew, TString::Format("%i to %i", (int)xlow, (int)xhigh));
    }
    int oldNbins = histoOld -> GetNbinsX();
    double diff = 1 - h -> Integral(0, newNBins + 1) / histoOld -> Integral(0, oldNbins + 1);
    //  double diff = 1 - histo -> GetSumOfWeights() / histoOld -> GetSumOfWeights();
    if (TMath::Abs(diff) > 1e-7) {
      cout << "BinningTool::WARNING: sizeable difference in transformation of '"
        << h -> GetName() << "' found. Integrals: (old-new)/old = " << diff << endl;
    }
  }

  if (corr) {
    // account for correlations in error computation 
    // the correlation histogram may need an offset due to the histogram range being previously altered
    int corrBinOffset = 0;
    for (int iCorrBin = 1; iCorrBin <= corr->GetXaxis()->GetNbins(); ++iCorrBin) {
      double corrBinCenter = corr->GetXaxis()->GetBinCenter(iCorrBin);
      double oldBinLow = histoOld->GetXaxis()->GetBinLowEdge(1);

      if (corrBinCenter > oldBinLow) {
        corrBinOffset = iCorrBin;
        break;
      }
    }
    // assumes the bin number pairs are iterated in reverse order
    int iBinNew = h->GetXaxis()->GetNbins();
    // int iBinNew = 1;
    for (auto &p : bins_merged) {
      // calculate the new bin error as:
      //    (new error)^2 = a.Sig.aT
      //    where a = [f1 ... fn]
      //          and Sig is the correlation matrix with the bin errors 
      //          appropriately multiplied with each element to yield the 
      //          covariance matrix
      //          (fn is the fraction of the new bin width the old bin width
      //          takes up, NOT NEEDED HERE SINCE HISTOGRAM IS NOT A PDF)
      double newError = 0.0;
      double totWidth = 0.0;
      for (int iOldBin = p.first; iOldBin <= p.second; ++iOldBin) {
        double iError = histoOld->GetBinError(iOldBin);
        double iWidth = histoOld->GetBinWidth(iOldBin);
        totWidth += iWidth;
        for (int jOldBin = iOldBin; jOldBin <= p.second; ++jOldBin) {
          double jError = histoOld->GetBinError(jOldBin);
          double jWidth = histoOld->GetBinWidth(jOldBin);

          newError += ((iOldBin == jOldBin) ? 1.0 : 2.0)*corr->GetBinContent(iOldBin + corrBinOffset, jOldBin + corrBinOffset)*iError*jError;//*iWidth*jWidth;
        }
      }
      newError = TMath::Sqrt(newError);///totWidth;
      h->SetBinError(iBinNew, newError);

      --iBinNew;
      // ++iBinNew;
    }
  }

  delete histoOld;
  emptyNegativeBins(h);

}


void BinningTool::changeRangeImpl(TH1* histo, float min, float max, bool useOverflow) { // copy from makeWS

  if( min == -1e8 ) { min = histo->GetBinLowEdge(1); }
  if( max ==  1e8 ) { max = histo->GetBinLowEdge( histo->GetNbinsX()+1 ); }
  float width = histo->GetBinWidth(0);
  int nbin( ceil( (max - min)/width ) ); // round up

  TH1* histoOld = (TH1*) histo -> Clone();
  histo->Reset();
  histo->SetBins(nbin, min, min + nbin*width);

  double err = 0; // root requires this to be a double...jeez
  float sum = 0;

  if(nbin<2) {
    cout << "BinningTool::changeRangeImpl" << endl;
    cout << "Change range requires 2 bins in resultant histogram " << endl;
    cout << "Requested range " << min << " " << max << " for histo " << histo->GetName() << endl;
    cout << "Which has bin width " << histo->GetBinWidth(0) << endl;
    throw;
  }

  // get content in old histo below and including the first bin of new histo
  int first, last;
  if(useOverflow) {
    first = 0;
    last = histoOld->GetNbinsX()+1;
  }
  else {
    first = histoOld->FindBin(histo->GetBinCenter(1));
    last = histoOld->FindBin(histo->GetBinCenter(nbin));
  }
  sum = histoOld -> IntegralAndError(first, histoOld->FindBin( histo->GetBinCenter(1) ), err);
  histo->SetBinContent( 1, sum );
  histo->SetBinError( 1, err );

  // get content in old histo above and including the last bin of new histo
  sum = histoOld -> IntegralAndError(histoOld->FindBin( histo->GetBinCenter(nbin) ), last, err);
  histo->SetBinContent( nbin, sum );
  histo->SetBinError( nbin, err );

  // for the rest it is a simple 1-to-1 matching of bins
  for( int b=2; b<nbin; b++ ) {
    int bin = histoOld->FindBin( histo->GetBinCenter(b) );
    histo->SetBinContent( b, histoOld->GetBinContent( bin ) );
    histo->SetBinError( b, histoOld->GetBinError( bin ) );
  }

  // check
  float sum2;
  double err2;
  sum = histoOld->IntegralAndError(first, last, err);
  sum2 = histo->IntegralAndError(1, nbin, err2);
  if( fabs(sum2-sum)/sum > 0.01 || fabs(err2-err)/err > 0.01 ) { // 1% tolerance
    cout << "BinningTool::changeRange" << endl;
    cout << "ChangeRange ERROR" << endl;
    cout << "Old Histo : " << sum << "\t" << err << endl;
    cout << "New Histo : " << sum2 << "\t" << err2 << endl;
    throw;
  }

  delete histoOld;

}

std::vector<int> BinningTool::getLocalExtremaBinning(TH1* hnom, TH1* hsys, unsigned int nmax) {
  // This implementation is iterative.
  // A faster (say analytic) implementation is possible if this one proves to be
  // too slow. This one is however easier to write and read
  std::vector<int> res;
  double err = 0;
  float sum = hnom->IntegralAndError(0, hnom->GetNbinsX()+1, err);
  // too large stat unc: no shape
  if(fabs(err / sum) > 0.05) {
    res.push_back(1);
    res.push_back(hnom->GetNbinsX()+1);
    return res;
  }

  // normal case. Then, beginning with no rebinning
  for(int i = 1 ; i < hnom->GetNbinsX()+2 ; i++) {
    res.push_back(i);
  }

  TH1* ratio = getRatioHist(hnom, hsys, res);
  std::vector<int> extrema = findExtrema(ratio);

  while(extrema.size() > nmax+2) {
    int pos = findSmallerChi2(hnom, hsys, extrema);
    mergeBins(extrema[pos], extrema[pos+1], res);
    getRatioHist(hnom, hsys, res, ratio);
    extrema = findExtrema(ratio);
  }

  // second pass to avoid bins with too large stat uncertainty
  std::vector<int>::iterator fst = res.end();
  std::vector<int>::iterator lst = res.end();
  std::vector<int> to_remove;
  --lst;
  --fst;
  while(fst != res.begin()) {
    if(fst == lst) {
      --fst;
    }
    else {
      float statE = statError(hnom, *fst, *lst);
      if(statE > 0.05) {
        to_remove.push_back(fst - res.begin());
        --fst;
      }
      else {
        lst=fst;
      }
    }
  }
  for(unsigned int i = 0; i < to_remove.size(); i++) {
    res.erase(res.begin()+to_remove[i]);
  }

  delete ratio;
  return res;
}


TH1* BinningTool::getRatioHist(TH1* hnom, TH1* hsys, const std::vector<int>& bins) {
  TH1* res = (TH1*) hsys->Clone();
  getRatioHist(hnom, hsys, bins, res);
  return res;
}

void BinningTool::getRatioHist(TH1* hnom, TH1* hsys, const std::vector<int>& bins, TH1* res) {
  for(unsigned int iRefBin = 0; iRefBin<bins.size()-1; iRefBin++) {
    float nomInt = hnom->Integral(bins.at(iRefBin), bins.at(iRefBin+1) - 1);
    float varInt = hsys->Integral(bins.at(iRefBin), bins.at(iRefBin+1) - 1);
    for(int b=bins.at(iRefBin); b<bins.at(iRefBin+1); b++) {
      if(nomInt != 0) {
        res->SetBinContent( b, varInt / nomInt );
      }
      else {
        res->SetBinContent( b, 0 );
      }
    }
  }
}

void BinningTool::scaleHistoSys(const TH1* hnom, TH1* hsys, float factor) {
  for(int b = 0; b <= hnom->GetNbinsX() + 1; b++) {
    float nom = hnom->GetBinContent(b);
    float var = hsys->GetBinContent(b);
    if(nom != 0) {
      std::cout << "Bin " << b << ": " << var / nom << " -> " << factor * (var / nom - 1) + 1 << std::endl;
      float scale = factor * (var / nom - 1) + 1;
      hsys->SetBinContent( b, scale * var );
    }
  }
}

std::vector<int> BinningTool::findExtrema(TH1* h) {
  std::vector<int> res;
  res.push_back(1);
  int status = 0; // 1: potential max, -1: potential min
  int k = 1;
  for(int i=2; i<h->GetNbinsX()+1; i++) {
    // special rule for bins with 0 stat. Keep going on, until one finds another bin to compare to
    if(h->GetBinContent(i) < 1.e-6) {
      continue;
    }
    if(status == 1 && h->GetBinContent(i) < h->GetBinContent(k) - 1.e-6) {
      res.push_back(i-1);
      status = -1;
    }
    if(status == -1 && h->GetBinContent(i) > h->GetBinContent(k) + 1.e-6) {
      res.push_back(i-1);
      status = 1;
    }
    if(status == 0 && h->GetBinContent(i) < h->GetBinContent(k) - 1.e-6) {
      status = -1;
    }
    if(status == 0 && h->GetBinContent(i) > h->GetBinContent(k) + 1.e-6) {
      status = 1;
    }
    k = i;
  }
  res.push_back(h->GetNbinsX());

  return res;
}

// inclusive in lo and hi
void BinningTool::mergeBins(int lo, int hi, std::vector<int>& bins) {
  std::vector<int>::iterator beg = std::upper_bound(bins.begin(), bins.end(), lo);
  // +1 because inclusive merge
  std::vector<int>::iterator last = std::lower_bound(bins.begin(), bins.end(), hi+1);
  bins.erase(beg, last);
}

void BinningTool::smoothHisto(TH1* hnom, TH1* hsys, const std::vector<int>& bins, bool smooth) {
  float norm_init = hsys->Integral();
  TH1* ratio = getRatioHist(hnom, hsys, bins);

  if(smooth && ratio->GetNbinsX() > 2) {
    std::vector<float> vals(ratio->GetNbinsX() - 2);
    for(int i=2 ; i<ratio->GetNbinsX(); i++) {
      vals[i-2] = (2. * ratio->GetBinContent(i) + ratio->GetBinContent(i-1) + ratio->GetBinContent(i+1)) / 4.;
    }
    for(int i=2 ; i<ratio->GetNbinsX(); i++) {
      ratio->SetBinContent(i, vals[i-2]);
    }
  }

  for(int i=1; i<hsys->GetNbinsX()+1; i++) {
    if(hnom->GetBinContent(i) != 0) {
      hsys->SetBinContent(i, ratio->GetBinContent(i) * hnom->GetBinContent(i));
    }
    else {
      hsys->SetBinContent(i, 0);
    }
  }
  hsys->Scale(norm_init / hsys->Integral());
  delete ratio;
  // set bin errors to 0 for systematics. Easier later when doing chi2 tests
  for(int i=0 ; i<hsys->GetNbinsX()+2; i++) {
    hsys->SetBinError(i, 0);
  }
}

int BinningTool::findSmallerChi2(TH1* hnom, TH1* hsys, const std::vector<int>& extrema) {
  int pos = 0;
  float minval = 99999;
  for(unsigned int i=0; i<extrema.size()-2; i++) {
    float chi2 = computeChi2(hnom, hsys, extrema[i], extrema[i+1]);
    if(chi2 < minval) {
      pos = i;
    }
  }
  return pos;
}

float BinningTool::computeChi2(TH1* hnom, TH1* hsys, int beg, int end) {
  float ratio = hsys->Integral(beg, end) / hnom->Integral(beg, end);
  float chi2 = 0;
  for(int i = beg; i < end+1; i++) {
    if(hnom->GetBinContent(i) != 0) {
    float iratio = hsys->GetBinContent(i) / hnom->GetBinContent(i);
    float err = hnom->GetBinError(i) / hnom->GetBinContent(i);
    chi2 += ((iratio-ratio)/err) * ((iratio-ratio)/err);
    }
  }
  return chi2;
}

float BinningTool::statError(TH1* hnom, int beg, int end) { // end is excluded
  double err = 0;
  float nomInt = hnom->IntegralAndError(beg, end-1, err);
  return fabs(err / nomInt);
}

std::vector<int> BinningTool::oneBin(const Category& c) {
  // put everything into 1 bin
  TH1* data = c.getDataHist();
  std::vector<int> res { data-> GetNbinsX() };
  delete data;
  return res;
}


std::vector<int> BinningTool::makeForcedBinning(const Category& c, const std::vector<double>& user_bins) {

  std::vector<int> res;

  // No rebin
  if (user_bins.empty()) return res;

  // Simple rebin
  if (user_bins.size()==1) {

    res.push_back(user_bins.front());
    return res;
  }

  TH1* h = c.getDataHist();

  // Complex rebin
  for (auto b = user_bins.rbegin(); b != user_bins.rend(); ++b) {
    res.push_back(h->FindBin(*b));
  }

  return res;
}
