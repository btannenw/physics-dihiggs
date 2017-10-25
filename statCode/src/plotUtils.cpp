#include "plotUtils.hpp"

#include <iostream>
#include <cstdlib>
#include <algorithm>

#include <TObjString.h>
#include <TObjArray.h>
#include <THashList.h>
#include <TGraphAsymmErrors.h>
#include <TAxis.h>
#include <TGraph.h>
#include <TH1.h>
#include <TString.h>


namespace PU {

  // global variables are extern'd in the header file
  std::map<TString, int> samplesPriority = setPriorities();

  std::map<TString, int> setPriorities() {
    std::map<TString, int> samplesPriority;
    samplesPriority["WlvH"] = 100;
    samplesPriority["ZllH"] = 101;
    samplesPriority["ZvvH"] = 102;
    samplesPriority["diboson"] = 50;
    samplesPriority["WW"] = 50;
    samplesPriority["ZZ"] = 51;
    samplesPriority["WZ"] = 52;
    samplesPriority["WZ"] = 52;
    samplesPriority["Top"] = 40;
    samplesPriority["stop"] = 40;
    samplesPriority["ttbar"] = 41;
    samplesPriority["W"] = 30;
    samplesPriority["Wl"] = 30;
    samplesPriority["Wcc"] = 31;
    samplesPriority["Wc"] = 32;
    samplesPriority["Wb"] = 33;
    samplesPriority["Z"] = 20;
    samplesPriority["Zl"] = 20;
    samplesPriority["Zc"] = 21;
    samplesPriority["Zb"] = 22;
    return samplesPriority;
  }

  std::map<TString, int> sysGroups = setSysGroups();

  std::map<TString, int> setSysGroups() {
    std::map<TString, int> sysGroups;
    sysGroups["SigXsec"] = 0;
    sysGroups["norm_"] = 5;
    sysGroups["LUMI"] = 10;
    sysGroups["BTag"] = 20;
    sysGroups["SysJet"] = 30;
    sysGroups["MET"] = 30;
    sysGroups["Elec"] = 40;
    sysGroups["Muon"] = 40;
    sysGroups["Theory"] = 50;
    sysGroups["VV"] = 60;
    sysGroups["WDPhi"] = 70;
    sysGroups["WMbb"] = 70;
    sysGroups["WbbMbb"] = 70;
    sysGroups["WPt"] = 70;
    sysGroups["ZDPhi"] = 80;
    sysGroups["ZMbb"] = 80;
    sysGroups["ZPt"] = 80;
    sysGroups["TopP"] = 90;
    sysGroups["Ttbar"] = 90;
    sysGroups["TChan"] = 100;
    sysGroups["SChan"] = 100;
    sysGroups["WtChan"] = 100;
    return sysGroups;
  }


  void SetBinLabels(TAxis* axis, const std::set<TString>& names) {
    if(static_cast<unsigned int>(axis->GetNbins()) != names.size()) {
      std::cout << "ERROR::SetBinLabels: axis and names sizes are incompatible !" << std::endl;
      exit(-1);
    }

    std::set<TString>::iterator it = names.begin();
    int count = 1;
    for(; it!=names.end(); ++it) {
      axis->SetBinLabel(count, (*it).Data());
      count++;
    }
  }


  void SetBinLabels(TAxis* axis, const std::vector<TString>& names) {
    if(static_cast<unsigned int>(axis->GetNbins()) != names.size()) {
      std::cout << "ERROR::SetBinLabels: axis and names sizes are incompatible !" << std::endl;
      exit(-1);
    }

    std::vector<TString>::const_iterator it = names.begin();
    int count = 1;
    for(; it!=names.end(); ++it) {
      axis->SetBinLabel(count, (*it).Data());
      count++;
    }
  }


  void GetBinLabels(const TAxis* axis, v_lab_pos& result) {
    result.clear();
    int nbins = axis->GetNbins();
    for(int i=1; i<nbins+1; i++) {
      result.push_back(std::make_pair(axis->GetBinLabel(i), i));
    }
  }


  bool comp_simple(const lab_pos& p1, const lab_pos& p2) {
    return p1.first < p2.first;
  }


  bool comp_sysNames(const lab_pos& p1, const lab_pos& p2) {
    int group1 = 1000;
    int group2 = 1000;
    for(std::map<TString, int>::iterator it = sysGroups.begin(); it!=sysGroups.end(); ++it) {
      if(p1.first.Contains(it->first)) { group1 = it->second; }
      if(p2.first.Contains(it->first)) { group2 = it->second; }
    }
    if(group1 != group2)
      return group1<group2;
    else
      return p1.first<p2.first;
  }


  bool comp_fullSamplesNames(const lab_pos& p1, const lab_pos& p2) {
    TString y1, y2, c1, c2, r1, r2, b1, b2;
    int pr1, pr2;
    tokenize(p1.first, y1, c1, r1, b1, pr1);
    tokenize(p2.first, y2, c2, r2, b2, pr2);
    // comparison of years
    if(y1 != y2)
      return y1<y2;
    // comparison of channels
    if(c1 != c2)
      return c1<c2;
    // comparison of regions
    if(r1 != r2)
      return r1<r2;
    // comparison of bins
    if(b1 != b2)
      return b1<b2;
    // comparison of samples
    if(pr1 != pr2)
      return pr1<pr2;

    // should arrive here only if signal samples of different masses
    return false;
  }


  void tokenize(const TString& s, TString& year, TString& channel, TString& region, TString&bin, int& priority) {
    TObjArray* tokens = s.Tokenize("_");
    year = ((TObjString*)tokens->At(3))->GetString();
    channel = ((TObjString*)tokens->At(0))->GetString();
    region = ((TObjString*)tokens->At(1))->GetString();
    bin = ((TObjString*)tokens->At(2))->GetString();

    TString sample = ((TObjString*)tokens->At(4))->GetString();
    if(sample.Contains("WlvH") || sample.Contains("ZllH") || sample.Contains("ZvvH"))
      priority = samplesPriority[sample(0, sample.Length()-3)];
    else
      priority = samplesPriority[sample];
  }


  void sortTHAxis(TH1* h, const bool isXaxis, bool (*fcn)(const lab_pos&, const lab_pos&), const bool reverse) {
    TAxis* axis = 0;
    if(isXaxis) {
      axis = h->GetXaxis();
    }
    else {
      axis = h->GetYaxis();
    }

    v_lab_pos bins;
    GetBinLabels(axis, bins);

    std::stable_sort(bins.begin(), bins.end(), fcn);
    if(reverse)
      std::reverse(bins.begin(), bins.end());
    shuffleTHAxis(h, isXaxis, bins);
  }


  void shuffleTHAxis(TH1* h, const bool isXaxis, const v_lab_pos& bins) {
    std::vector<int> binorder(bins.size());
    for(unsigned int i=0; i<bins.size(); i++)
      binorder[i] = bins[i].second;
    shuffleTHAxis(h, isXaxis, binorder);
  }


  void shuffleTHAxis(TH1* h, const bool isXaxis, const std::vector<int>& bins) {
    TAxis* axis = 0;
    TAxis* otherAxis = 0;
    if(isXaxis) {
      axis = h->GetXaxis();
      otherAxis = h->GetYaxis();
    }
    else {
      axis = h->GetYaxis();
      otherAxis = h->GetXaxis();
    }

    TH1* hinit = (TH1*)(h->Clone("tmp"));
    TAxis* a_init = (TAxis*)(axis->Clone());

    for(unsigned int i=0; i<bins.size(); i++) {
      int b = i+1;
      int b_init = bins[i];
      axis->SetBinLabel(b, a_init->GetBinLabel(b_init));
      for(int j=1; j<otherAxis->GetNbins()+1; j++) {
        if(isXaxis) {
          h->SetBinContent(b, j, hinit->GetBinContent(b_init, j));
          h->SetBinError(b, j, hinit->GetBinError(b_init, j));
        }
        else {
          h->SetBinContent(j, b, hinit->GetBinContent(j, b_init));
          h->SetBinError(j, b, hinit->GetBinError(j, b_init));
        }
      }
    }

    delete a_init;
    delete hinit;
  }


  TH1* reduceTHAxisExclude(const TH1* h, const bool isXaxis, const std::vector<TString>& excludes) {
    return reduceTHAxis(h, isXaxis, excludes, false);
  }


  TH1* reduceTHAxisInclude(const TH1* h, const bool isXaxis, const std::vector<TString>& includes) {
    return reduceTHAxis(h, isXaxis, includes, true);
  }


  TH1* reduceTHAxis(const TH1* h, const bool isXaxis, const std::vector<TString>& patterns, const bool includes) {
    TH1* hnew = (TH1*)(h->Clone("tmp"));
    TH1* hfinal = (TH1*)(hnew->Clone(TString(h->GetName())+"_reduced"));
    TAxis* axis = 0;
    TAxis* otherAxis = 0;
    TAxis* a_final = 0;
    if(isXaxis) {
      axis = hnew->GetXaxis();
      a_final = hfinal->GetXaxis();
      otherAxis = hnew->GetYaxis();
    }
    else {
      axis = hnew->GetYaxis();
      a_final = hfinal->GetYaxis();
      otherAxis = hnew->GetXaxis();
    }

    int nPresent;
    v_lab_pos bins = findAndSortBins(axis, patterns, includes, nPresent);
    shuffleTHAxis(hnew, isXaxis, bins);

    // then restrict the histo to the selected region
    int nToKeep = includes ? nPresent : bins.size() - nPresent;
    a_final->Set(nToKeep, 0, nToKeep);
    for(int i=1; i<nToKeep+1; i++) {
      a_final->SetBinLabel(i, axis->GetBinLabel(i));
      for(int j=1; j<otherAxis->GetNbins()+1; j++) {
        if(isXaxis) {
          hfinal->SetBinContent(i, j, hnew->GetBinContent(i, j));
          hfinal->SetBinError(i, j, hnew->GetBinError(i, j));
        }
        else {
          hfinal->SetBinContent(j, i, hnew->GetBinContent(j, i));
          hfinal->SetBinError(j, i, hnew->GetBinError(j, i));
        }
      }
    }

    delete hnew;
    return hfinal;
  }


  void shuffleTGraphAxis(TGraph* g, TAxis* a, const bool isXaxis, const v_lab_pos& bins) {
    std::vector<int> binorder(bins.size());
    for(unsigned int i=0; i<bins.size(); i++)
      binorder[i] = bins[i].second;
    shuffleTGraphAxis(g, a, isXaxis, binorder);
  }

  void shuffleTGraphAxis(TGraph* g, TAxis* a, const bool isXaxis, const std::vector<int>& bins) {
    TAxis* a_init = (TAxis*)(a->Clone("tmptmp"));

    for(unsigned int i=0; i<bins.size(); i++) {
      int b = i+1;
      int b_init = bins[i];
      a->SetBinLabel(b, a_init->GetBinLabel(b_init));
    }
    // workaround bug in TAxis
    a->GetLabels()->Rehash(a->GetNbins()+2);

    for(int i=0; i<g->GetN(); i++) {
      double x,y;
      g->GetPoint(i,x,y);
      double coord;
      if(isXaxis)
        coord = x;
      else
        coord = y;
      int b_init = a_init->FindBin(coord);
      const TString label = a_init->GetBinLabel(b_init);

      int b_new = a->FindBin(label);
      coord += (a->GetBinCenter(b_new) - a->GetBinCenter(b_init));
      if(isXaxis)
        g->SetPoint(i,coord,y);
      else
        g->SetPoint(i,x,coord);
    }
    delete a_init;
    //setTGraphAxis(g, a, isXaxis);
  }


  void sortTGraphAxis(TGraph* g, TAxis* a, const bool isXaxis, bool (*fcn)(const lab_pos&, const lab_pos&), bool reverse) {
    v_lab_pos bins;
    GetBinLabels(a, bins);

    std::stable_sort(bins.begin(), bins.end(), fcn);
    if(reverse)
      std::reverse(bins.begin(), bins.end());
    shuffleTGraphAxis(g, a, isXaxis, bins);
  }

  TGraph* reduceTGraphAxis(const TGraph* g, const TAxis* a, const bool isXaxis, const std::vector<TString>& patterns, bool includes) {
    TGraph* gnew = (TGraph*)(g->Clone("tmp"));
    TAxis* a_new = (TAxis*)(a->Clone("tmpa"));

    int nPresent;
    v_lab_pos bins = findAndSortBins(a, patterns, includes, nPresent);
    shuffleTGraphAxis(gnew, a_new, isXaxis, bins);
    TGraph* gfinal = (TGraph*)(gnew->Clone(TString(g->GetName())+"_reduced"));

    // then restrict the histo to the selected region
    int nToKeep = includes ? nPresent : bins.size() - nPresent;
    double min = a->GetXmin();
    TAxis* gaxis = gfinal->GetXaxis();
    gaxis->Set(nToKeep, min, min+nToKeep*a_new->GetBinWidth(1));
    for(int i=1; i<gaxis->GetNbins()+1; i++)
      gaxis->SetBinLabel(i, a_new->GetBinLabel(i));

    float max = gaxis->GetXmax();
    removeTGraphPointsAbove(gfinal, isXaxis, max);

    delete gnew;
    delete a_new;
    return gfinal;
  }

  void removeTGraphPointsAbove(TGraph* g, const bool isXaxis, double val) {
    int i = 0;
    double x,y;
    while(g->GetPoint(i,x,y)>-1) {
      if(isXaxis && x>val)
        g->RemovePoint(i);
      else if(!isXaxis && y>val)
        g->RemovePoint(i);
      else i++;
    }
  }

  v_lab_pos findAndSortBins(const TAxis* a, const std::vector<TString>& patterns, bool
  includes, int& nPresent) {
    TString yes, no;
    if(includes) {
      yes = "0";
      no = "1";
    }
    else {
      yes = "1";
      no = "0";
    }

    // find out which bins are concerned by the patterns
    nPresent = 0;
    v_lab_pos bins;
    GetBinLabels(a, bins);
    for(unsigned int i=0; i<bins.size(); i++) {
      TString& label = bins[i].first;
      TString isIn = no;
      for(std::vector<TString>::const_iterator it = patterns.begin(); it!=patterns.end(); ++it) {
        if(label.Contains(*it)) {
          isIn = yes;
          nPresent++;
          break;
        }
      }
      label = isIn;
    }

    // move the selected bins at the beginning or at the end
    std::stable_sort(bins.begin(), bins.end(), comp_simple);
    // bins to be kept are at the beginning now
    return bins;
  }

  TGraph* reduceTGraphAxisExclude(const TGraph* g, const TAxis* a, const bool isXaxis, const std::vector<TString>& excludes) {
    return reduceTGraphAxis(g, a, isXaxis, excludes, false);
  }

  TGraph* reduceTGraphAxisInclude(const TGraph* g, const TAxis* a, const bool isXaxis, const std::vector<TString>& includes) {
    return reduceTGraphAxis(g, a, isXaxis, includes, true);
  }

  void shiftTGraph(TGraph* g, const float shift, const bool isXaxis) {
    for(int i=0; i<g->GetN(); i++) {
      double x,y;
      g->GetPoint(i, x, y);
      if(isXaxis)
        g->SetPoint(i, x+shift, y);
      else
        g->SetPoint(i, x, y+shift);
    }
  }

  TGraph* shiftTGraphToZero(const TGraph* g, TAxis* a, const bool isXaxis) {
    TGraph* gfinal = (TGraph*)(g->Clone(TString(g->GetName())+"_shifted"));

    double x,y;
    for(int i=0; i<gfinal->GetN(); i++) {
      gfinal->GetPoint(i,x,y);
      float coord = 0;
      if(isXaxis)
        coord = x;
      else
        coord = y;
      TString label = a->GetBinLabel(a->FindBin(coord));
      float other = 0;
      if(label.BeginsWith("norm_"))
        other = 1;
      if(isXaxis)
        gfinal->SetPoint(i,coord,other);
      else
        gfinal->SetPoint(i,other,coord);
    }
    return gfinal;
  }

  TAxis* mergeTGraphAxis(const std::vector<TGraph*>& v, const bool isXaxis) {
    // find all existing labels
    std::set<TString> labels;
    for(unsigned int i=0; i<v.size(); i++) {
      TAxis* axis=0;
      if(isXaxis)
        axis = v[i]->GetXaxis();
      else
        axis = v[i]->GetYaxis();
      for(int j=1; j<axis->GetNbins()+1; j++) {
        labels.insert(axis->GetBinLabel(j));
      }
    }

    // create new axis with all labels
    TAxis* newAxis = new TAxis(labels.size(), 0, labels.size());
    std::set<TString>::iterator it = labels.begin();
    int count = 1;
    for(; it!=labels.end(); ++it) {
      newAxis->SetBinLabel(count, *it);
      count++;
    }

    // now change the TGraphs to match the new axis
    for(unsigned int i=0; i<v.size(); i++) {
      TGraphAsymmErrors* tge = 0;
      if(v[i]->InheritsFrom("TGraphAsymmErrors"))
        tge = (TGraphAsymmErrors*)(v[i]);
      TAxis* axis=0;
      if(isXaxis)
        axis = (TAxis*)(v[i]->GetXaxis()->Clone("tmp"));
      else
        axis = (TAxis*)(v[i]->GetYaxis()->Clone("tmp"));
      for(int j=0; j<v[i]->GetN(); j++) {
        double x,y;
        v[i]->GetPoint(j, x, y);
        if(isXaxis) {
          TString label = axis->GetBinLabel(axis->FindBin(x));
          float newx = newAxis->GetBinCenter(newAxis->FindBin(label));
          v[i]->SetPoint(j, newx, y);
          if(tge) {
            tge->SetPointEXhigh(j, .1);
            tge->SetPointEXlow(j, .1);
          }
        }
        else {
          TString label = axis->GetBinLabel(axis->FindBin(y));
          float newy = newAxis->GetBinCenter(newAxis->FindBin(label));
          v[i]->SetPoint(j, x, newy);
          if(tge){
            tge->SetPointEYhigh(j, .1);
            tge->SetPointEYlow(j, .1);
          }
        }
      }
      setTGraphAxis(v[i], newAxis, isXaxis);
      TAxis* tgaxis=0;
      if(isXaxis)
        tgaxis = v[i]->GetXaxis();
      else
        tgaxis = v[i]->GetYaxis();
      tgaxis->GetLabels()->Rehash(tgaxis->GetNbins()+2);
      delete axis;
      TAxis* tmpaxis = (TAxis*)(tgaxis->Clone("axistmp"));
      PU::sortTGraphAxis(v[i], tmpaxis, isXaxis, *PU::comp_sysNames);
      setTGraphAxis(v[i], tmpaxis, isXaxis);
      delete tmpaxis;
    }
    return newAxis;
  }

  void setTGraphAxis(TGraph* g, const TAxis* a, const bool isXaxis) {
    TAxis* axis=0;
    if(isXaxis)
      axis = g->GetXaxis();
    else
      axis = g->GetYaxis();
    a->Copy(*axis);
  }

}
