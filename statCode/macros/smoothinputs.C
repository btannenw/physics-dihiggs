#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <tuple>
#include <string>
#include <algorithm>
#include <numeric>
#include <memory>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>

#include "TError.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TF1.h"
#include "TMath.h"
#include "TKey.h"
#include "TDirectory.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"

// int print_lvl = 1;

// NOTE: command line arguments:
//       first (required): path to directory of split/merged inputs
//       second: vector of strings formatted in a special way that indicates what and how to smooth
//               string is a semicolon-separated list of the following (only first is required)
//                  comma-separated list of samples to merge and smooth
//                  comma-separated list of file name keyword(s) to logically "and" together in order to include to determining whether to smooth or not
//                  comma-separated list of file name keyword(s) to logically "or" together in order to exclude in determining whether to smooth or not
//                  bool that is true if the full histogram range is left untouched on left side (false to truncate at first bin with positive data) - default = true
//                  bool that is true if the full histogram range is left untouched on right side (false to truncate at last bin with positive data) - default = true
//                  double representing a KNN (in fraction of events) that scales the bandwidth (never decreases) - default = 0.0
//                  double representing an event factor that scales the bandwidth (increase for narrower bandwidths and decrease for larger bandwidths) - default = 1.0
//                  double representing the KNN parameter fractional variation (used in creating a systematic associated with smoothing) - default = 0.05
//                  double representing the event-factor parameter fractional variation (used in creating a systematic associated with smoothing) - default = 0.05
//                  string representing whether to use a logarithmic (log) or linear (lin) scale - default = lin
//                  double representing by which factor to reduce the number of grid points to use - default = 1.0
//      third (optional): Boolean value indicating whether or not to write new inputs
// Example call (from command line):
//  root -l -q -b "smoothinputs.C+(\"/Users/jhetherly/Documents/Graduate_Work/ATLAS/HSG5/Zbb_Smoothing/21009\", {\"Zcc,Zbc,Zbb;TwoLepton,SR;fat;false;true;0.005;1.0\"})"

bool check_for_NaNs_and_Infs (const TH1 *h);

bool DetermineSmoothFromFileName(const TString &name, const std::vector<std::string> &all_include, const std::vector<std::string> &any_exclude)
{
  bool result = true;
  for (auto &s : all_include) result &= name.Contains(s.c_str());
  if (result) for (auto &s : any_exclude) result &= !name.Contains(s.c_str());
  return result;
}

// bool DetermineCombineFromFileName (const TString &/*name*/)
// { return true; }

// function that controls the smoothing - this histogram passed in is deleted and TH1* returned assumes it's name and title
std::pair<TH1*, TH2*> smooth_histo(TH1*, const double&, const double&, const bool&, const bool&, const std::string& = "lin", const double& = 1.0, const bool& = false);

double DetermineLowerBound (TH1*, bool absolute = false);
double DetermineUpperBound (TH1*, bool absolute = false);
unsigned DetermineNumberOfGridPoints (TH1*, double, double);
void DrawHistogramBand (TLegend *leg, const char *leg_entry, TH1 *h, EColor mean = kBlue, EColor error = kWhite);
void DrawHistogramCircleE1 (TLegend *leg, const char *leg_entry, TH1 *h, EColor mean = kBlack, EColor error = kBlack);


// TODO: test for multiple smoothed samples
void smoothinputs (const TString &src_dir,
                   const std::vector<std::string> &settings,
                   const bool &write_new_inputs = false)
{
  gDebug = 0;
  using namespace std;
  auto n_samples = settings.size();
  bool should_combine_samples = false;
  vector<vector<string>> samples_to_smooth;//(n_samples);
  vector<string>         combined_samples_to_smooth;//(n_samples);
  vector<vector<string>> catagories_all_include;//(n_samples);
  vector<vector<string>> catagories_any_exclude;//(n_samples);
  vector<bool>           full_low_ranges;
  vector<bool>           full_high_ranges;
  vector<double>         knn_parameters;
  vector<double>         ef_parameters;
  vector<double>         knn_variation_factors;
  vector<double>         ef_variation_factors;
  vector<string>         scales;
  vector<double>         grid_factors;

  // parse smoothing commands
  for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
    auto setting = settings[isetting];
    vector<string> tokens;

    tokens.push_back("");
    for (char c : setting) {
      if (c == ';') {
        tokens.push_back("");
        continue;
      }
      tokens.back().push_back(c);
    }

    samples_to_smooth.push_back({});
    combined_samples_to_smooth.push_back("");
    if (tokens[0].size() != 0) {
      samples_to_smooth.back().push_back("");
      for (char c : tokens[0]) {
        if (c == ',') {
          samples_to_smooth.back().push_back("");
          continue;
        }
        samples_to_smooth.back().back().push_back(c);
      }
      for (auto &s : samples_to_smooth.back()) combined_samples_to_smooth.back() += s;
      should_combine_samples |= (samples_to_smooth.back().size() > 1);
    }

    catagories_all_include.push_back({});
    if (tokens.size() >= 2) {
      if (tokens[1].size() != 0) {
        catagories_all_include.back().push_back("");
        for (char c : tokens[1]) {
          if (c == ',') {
            catagories_all_include.back().push_back("");
            continue;
          }
          catagories_all_include.back().back().push_back(c);
        }
      }
    }

    catagories_any_exclude.push_back({});
    if (tokens.size() >= 3) {
      if (tokens[2].size() != 0) {
        catagories_any_exclude.back().push_back("");
        for (char c : tokens[2]) {
          if (c == ',') {
            catagories_any_exclude.back().push_back("");
            continue;
          }
          catagories_any_exclude.back().back().push_back(c);
        }
      }
    }

    if (tokens.size() >= 4) full_low_ranges.push_back((tokens[3] == "true" || tokens[3] == ""));
    else full_low_ranges.push_back(true);

    if (tokens.size() >= 5) full_high_ranges.push_back((tokens[4] == "true" || tokens[4] == ""));
    else full_high_ranges.push_back(true);

    if (tokens.size() >= 6) knn_parameters.push_back((tokens[5] != "") ? stod(tokens[5]) : 0.0);
    else knn_parameters.push_back(0.0);
    if (knn_parameters.back() > 1.0 || knn_parameters.back() < 0.0)
      knn_parameters.back() = 0.0;

    if (tokens.size() >= 7) ef_parameters.push_back((tokens[6] != "") ? stod(tokens[6]) : 1.0);
    else ef_parameters.push_back(1.0);

    if (tokens.size() >= 8) knn_variation_factors.push_back((tokens[7] != "") ? stod(tokens[7]) : 0.0);
    else knn_variation_factors.push_back(0.0);
    if (knn_variation_factors.back() >= 1.0 || knn_variation_factors.back() < 0.0)
      knn_variation_factors.back() = 0.0;

    if (tokens.size() >= 9) ef_variation_factors.push_back((tokens[8] != "") ? stod(tokens[8]) : 0.0);
    else ef_variation_factors.push_back(0.0);
    if (ef_variation_factors.back() >= 1.0 || ef_variation_factors.back() < 0.0)
      ef_variation_factors.back() = 0.0;

    if (tokens.size() >= 10) scales.push_back((tokens[9] != "") ? tokens[9] : "lin");
    else scales.push_back("lin");

    if (tokens.size() >= 11) grid_factors.push_back((tokens[10] != "") ? stod(tokens[10]) : 1.0);
    else grid_factors.push_back(1.0);
    if (grid_factors.back() < 1.0)
      grid_factors.back() = 1.0;
  }

  TH1::SetDefaultSumw2(kTRUE);
  TSystemDirectory dir(src_dir.Data(), src_dir.Data());
  TList *files = dir.GetListOfFiles();
  vector<TString> files_to_copy, files_to_combine, files_to_smooth;


  if (files) {
    TSystemFile *file;
    TString fname;
    TString extension(".root");
    TIter next(files);
    while ((file=(TSystemFile*)next())) {
      fname = file->GetName();
      if (!file->IsDirectory() && fname.EndsWith(extension.Data()) && !fname.Contains("ORIGINAL_") && !fname.Contains("TEMP_")) {
        bool should_smooth = false;
        for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting)
          should_smooth |= DetermineSmoothFromFileName(fname, catagories_all_include[isetting], catagories_any_exclude[isetting]);
        if (should_smooth || should_combine_samples)
          files_to_smooth.push_back(fname);
        else
          files_to_copy.push_back(fname);
      }
    }
  }

  TString new_dir = src_dir;

  // NOTE: this is for control plots
  TString new_dir_control = Form("%s/plots", new_dir.Data());
  gSystem->mkdir(new_dir_control.Data(), kTRUE);


  for (size_t j = 0; j < files_to_smooth.size(); ++j) {
    TString original_full_file = src_dir + "/" + files_to_smooth[j],
            destination_file = new_dir + "/" + files_to_smooth[j],
            temp_file_name = src_dir + "/TEMP_" + files_to_smooth[j],
            copied_file_name = src_dir + "/ORIGINAL_" + files_to_smooth[j];
    TFile *_file0, // original file
          *_file1, // destination file
          *_file2; // control plots file
    TDirectory *file0_sysdir;
    TString dirname;

    TString systematics_dir_name;
    TString original_file = original_full_file(original_full_file.Last('/') + 1, original_full_file.Length() - (original_full_file.Last('/') + 1));
    TString new_full_file = new_dir + "/" + original_file;
    TString new_full_file_control = new_dir_control + "/" + original_file;
    if (gDebug >= 0 && write_new_inputs) Info("smoothinputs", "Creating newly combined input %s", new_full_file.Data());
    auto copied_file = "ORIGINAL_" + files_to_smooth[j];
    auto prev_error_lvl = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kSysError;
    _file0 =  TFile::Open(copied_file_name.Data());
    gErrorIgnoreLevel = prev_error_lvl;
    auto file0_file_name_no_write = copied_file_name;
    if (_file0 == nullptr) {
      _file0 =  TFile::Open(original_full_file.Data());
      file0_file_name_no_write = original_full_file;
      if (write_new_inputs) {
        _file0->Cp(original_full_file.Data(), copied_file_name.Data());
        _file0->Close();
        _file0 =  TFile::Open(copied_file_name.Data());
      }
    }
    if (write_new_inputs) {
      _file1 =  TFile::Open(new_full_file.Data(), "RECREATE");
    }
    else {
      _file1 =  TFile::Open(temp_file_name.Data(), "RECREATE");
    }

    TIter nextkey(_file0->GetListOfKeys());
    TKey *key;
    vector<TH1*> nominal_hists_to_smooth(n_samples, nullptr);
    vector<map<string, TH1*>> syst_histos(n_samples);
    while ( (key = (TKey*)nextkey()) ) {

      TString keyname = key->GetName();
      if (key && keyname.Contains("Sys")) {
        if (systematics_dir_name == "") systematics_dir_name = keyname;
        file0_sysdir = (TDirectory*)key->ReadObj();
        //Systematic list
        dirname = file0_sysdir->GetName();
        _file1->mkdir(dirname, dirname);
        _file1->cd(dirname);
        TIter nextkey2(file0_sysdir->GetListOfKeys());
        TKey *key2;
        while ( (key2 = (TKey*)nextkey2()) ) {
          TH1 *h = (TH1F*)key2->ReadObj();
          TString origname = key2->GetName();
          TString systname = origname;
          systname.Remove(0, systname.First('_'));
          for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting)
            if (syst_histos[isetting].count(systname.Data()) == 0) syst_histos[isetting][systname.Data()] = nullptr;
          bool is_histo = false;
          decltype(n_samples) iisetting = 0;
          for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
            if (!DetermineSmoothFromFileName(_file0->GetName(), catagories_all_include[isetting], catagories_any_exclude[isetting])) continue;
            for (unsigned i = 0; i < samples_to_smooth[isetting].size(); ++i) {
              is_histo |= origname.BeginsWith((samples_to_smooth[isetting][i]+"_").c_str());
              if (is_histo) break;
            }
            if (is_histo) {iisetting = isetting; break;}
          }
          _file1->cd(dirname);
          if(is_histo) {
            if (syst_histos[iisetting][systname.Data()] == 0)
              syst_histos[iisetting][systname.Data()] = static_cast<TH1*>(h->Clone());
            else
              syst_histos[iisetting][systname.Data()]->Add(h);
          }
          else {
            h->Write();
          }
          _file0->cd();
        }
        typedef map<string, TH1*>::iterator it_type;
        for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
          for (it_type iterator = syst_histos[isetting].begin(); iterator != syst_histos[isetting].end(); ++iterator) {
            TH1 *h_to_smooth = iterator->second;
            if (h_to_smooth != 0) {
              string hist_name(h_to_smooth->GetName());
              string hist_title(h_to_smooth->GetTitle());
              decltype(n_samples) i_sample = 0;
              // for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
              //   bool found = false;
                for (unsigned i = 0; i < samples_to_smooth[isetting].size(); ++i) {
                  if (hist_name.find(samples_to_smooth[isetting][i]) == 0) {
                    i_sample = isetting;
                    // found = true;
                    break;
                  }
                }
              //   if (found) break;
              // }
              h_to_smooth->SetName(hist_name.replace(0, hist_name.find_first_of('_'), combined_samples_to_smooth[i_sample]).c_str());
              h_to_smooth->SetTitle(hist_title.replace(0, hist_title.find_first_of('_'), combined_samples_to_smooth[i_sample]).c_str());

              TH1 *smoothed_histo = 0;
              if (DetermineSmoothFromFileName(_file0->GetName(), catagories_all_include[i_sample], catagories_any_exclude[i_sample]) &&
                  !check_for_NaNs_and_Infs(h_to_smooth)) {
                _file1->cd(dirname);
                if (gDebug >= 1) Info("smoothinputs", "Smoothing histogram %s", h_to_smooth->GetName());
                smoothed_histo = smooth_histo(h_to_smooth, knn_parameters[i_sample], ef_parameters[i_sample], full_low_ranges[i_sample], full_high_ranges[i_sample], scales[i_sample], grid_factors[i_sample]).first;
              }
              else {
                smoothed_histo = h_to_smooth;
              }

              _file1->cd(dirname);
              if (smoothed_histo != 0) {
                TDirectory::Cd(Form("%s:%s", _file1->GetName(), dirname.Data()));
                smoothed_histo->Write();
              }
              else {
                Warning("smoothinputs", "Couldn't smooth histogram! Saving un-smoothed histogram instead.");
                TDirectory::Cd(Form("%s:%s", _file1->GetName(), dirname.Data()));
                h_to_smooth->Write();
              }
              _file0->cd();
            }
          }
          syst_histos[isetting].clear();
        }
      } else if (key) {
        TH1 *h = (TH1*)key->ReadObj();
        TString origname=key->GetName();
        _file1->cd();
        bool is_histo = false;
        decltype(n_samples) i_sample = 0;
        for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
          if (!DetermineSmoothFromFileName(_file0->GetName(), catagories_all_include[isetting], catagories_any_exclude[isetting])) continue;
          for (unsigned i = 0; i < samples_to_smooth[isetting].size(); ++i) {
            is_histo |= (origname == samples_to_smooth[isetting][i]);
            if (is_histo) break;
          }
          if (is_histo) {i_sample = isetting; break;}
        }
        // for (unsigned i = 0; i < histos_to_smooth.size(); ++i) {
        //   is_histo |= (origname == histos_to_smooth[i]);
        // }
        if(is_histo) {
          if (nominal_hists_to_smooth[i_sample] == nullptr)
            nominal_hists_to_smooth[i_sample] = static_cast<TH1*>(h->Clone());
          else
            nominal_hists_to_smooth[i_sample]->Add(h);
        }
        else {
          h->Write(origname);
        }

        _file0->cd();
      }
    }

    vector<TH2*> smoothed_nominal_corrs(n_samples, nullptr);
    vector<TH1*> smoothed_nominal_histos(n_samples, nullptr);
    vector<TH1*> smoothed_nominal_histos_syst_up(n_samples, nullptr);
    vector<TH1*> smoothed_nominal_histos_syst_down(n_samples, nullptr);
    vector<TH1*> unsmoothed_nominal_histos(n_samples, nullptr);
    if (static_cast<decltype(nominal_hists_to_smooth.size())>(count(nominal_hists_to_smooth.begin(), nominal_hists_to_smooth.end(), nullptr)) != nominal_hists_to_smooth.size()) {
      for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
        if (nominal_hists_to_smooth[isetting] == nullptr) continue;
        nominal_hists_to_smooth[isetting]->SetName(combined_samples_to_smooth[isetting].c_str());
        std::string hist_title(nominal_hists_to_smooth[isetting]->GetTitle());
        nominal_hists_to_smooth[isetting]->SetTitle(hist_title.replace(0, hist_title.find_first_of('_'), combined_samples_to_smooth[isetting]).c_str());
        TH1 *smoothed_histo = nullptr,
            *smoothed_histo_syst_up = nullptr,
            *smoothed_histo_syst_down = nullptr;
        TH2D *rebinned_corr = nullptr;

        _file1->cd();
        if (DetermineSmoothFromFileName(_file0->GetName(), catagories_all_include[isetting], catagories_any_exclude[isetting]) &&
            !check_for_NaNs_and_Infs(nominal_hists_to_smooth[isetting])) {
          if (gDebug >= 1) Info("smoothinputs", "Smoothing histogram %s", nominal_hists_to_smooth[isetting]->GetName());

          auto nominal_down_down = (TH1*)nominal_hists_to_smooth[isetting]->Clone();
          nominal_down_down->SetDirectory(0);
          nominal_down_down->SetName("nominal_down_down");
          auto nominal_down_up = (TH1*)nominal_hists_to_smooth[isetting]->Clone();
          nominal_down_up->SetDirectory(0);
          nominal_down_up->SetName("nominal_down_up");
          auto nominal_up_down = (TH1*)nominal_hists_to_smooth[isetting]->Clone();
          nominal_up_down->SetDirectory(0);
          nominal_up_down->SetName("nominal_up_down");
          auto nominal_up_up = (TH1*)nominal_hists_to_smooth[isetting]->Clone();
          nominal_up_up->SetDirectory(0);
          nominal_up_up->SetName("nominal_up_up");
          unsmoothed_nominal_histos[isetting] = static_cast<TH1*>(nominal_hists_to_smooth[isetting]->Clone());
          unsmoothed_nominal_histos[isetting]->SetDirectory(0);
          auto nominal_h_unsmoothed = static_cast<TH1*>(nominal_hists_to_smooth[isetting]->Clone());
          auto smoothed_result = smooth_histo(nominal_hists_to_smooth[isetting], knn_parameters[isetting], ef_parameters[isetting], full_low_ranges[isetting], full_high_ranges[isetting], scales[isetting], grid_factors[isetting], true);
          smoothed_histo = smoothed_result.first;
          smoothed_nominal_histos[isetting] = (TH1*)smoothed_histo->Clone();
          smoothed_nominal_histos[isetting]->SetDirectory(0);

          // systematic variations for smoothing
          // JWH
          // std::pair<double, double> knn_variations(knn_parameters[isetting]*(1.0 - knn_variation_factors[isetting]), knn_parameters[isetting]*(1.0 + knn_variation_factors[isetting])),
          //                           ef_variations(ef_parameters[isetting]*(1.0 - ef_variation_factors[isetting]), ef_parameters[isetting]*(1.0 + ef_variation_factors[isetting]));
          std::pair<double, double> knn_variations(TMath::Max(knn_parameters[isetting] - knn_variation_factors[isetting], 0.0), TMath::Min(knn_parameters[isetting] + knn_variation_factors[isetting], 0.999)),
                                    ef_variations(ef_parameters[isetting]*(TMath::Power(10.0, -10.0*ef_variation_factors[isetting])), ef_parameters[isetting]*(TMath::Power(10.0, 10.0*ef_variation_factors[isetting])));
          auto smoothed_histo_syst_down_down = smooth_histo(nominal_down_down, knn_variations.first, ef_variations.first, full_low_ranges[isetting], full_high_ranges[isetting], scales[isetting], grid_factors[isetting]).first;
          auto smoothed_histo_syst_down_up = smooth_histo(nominal_down_up, knn_variations.first, ef_variations.second, full_low_ranges[isetting], full_high_ranges[isetting], scales[isetting], grid_factors[isetting]).first;
          auto smoothed_histo_syst_up_down = smooth_histo(nominal_up_down, knn_variations.second, ef_variations.first, full_low_ranges[isetting], full_high_ranges[isetting], scales[isetting], grid_factors[isetting]).first;
          auto smoothed_histo_syst_up_up = smooth_histo(nominal_up_up, knn_variations.second, ef_variations.second, full_low_ranges[isetting], full_high_ranges[isetting], scales[isetting], grid_factors[isetting]).first;
          auto smoothed_histo_syst_down_down_delta = (TH1*)smoothed_histo_syst_down_down->Clone();
          smoothed_histo_syst_down_down_delta->SetDirectory(0);
          auto smoothed_histo_syst_down_up_delta = (TH1*)smoothed_histo_syst_down_up->Clone();
          smoothed_histo_syst_down_up_delta->SetDirectory(0);
          auto smoothed_histo_syst_up_down_delta = (TH1*)smoothed_histo_syst_up_down->Clone();
          smoothed_histo_syst_up_down_delta->SetDirectory(0);
          auto smoothed_histo_syst_up_up_delta = (TH1*)smoothed_histo_syst_up_up->Clone();
          smoothed_histo_syst_up_up_delta->SetDirectory(0);
          // NOTE: compute relative differences between variations and nominal
          for (Int_t ibin = 1; ibin <= smoothed_histo->GetXaxis()->GetNbins(); ++ibin) {
            auto nominal_bin_content = smoothed_histo->GetBinContent(ibin);
            auto down_down_bin_content = smoothed_histo_syst_down_down->GetBinContent(ibin);
            auto down_up_bin_content = smoothed_histo_syst_down_up->GetBinContent(ibin);
            auto up_down_bin_content = smoothed_histo_syst_up_down->GetBinContent(ibin);
            auto up_up_bin_content = smoothed_histo_syst_up_up->GetBinContent(ibin);
            if (nominal_bin_content != 0.0) {
              smoothed_histo_syst_down_down_delta->SetBinContent(ibin, (down_down_bin_content - nominal_bin_content)/nominal_bin_content);
              smoothed_histo_syst_down_up_delta->SetBinContent(ibin, (down_up_bin_content - nominal_bin_content)/nominal_bin_content);
              smoothed_histo_syst_up_down_delta->SetBinContent(ibin, (up_down_bin_content - nominal_bin_content)/nominal_bin_content);
              smoothed_histo_syst_up_up_delta->SetBinContent(ibin, (up_up_bin_content - nominal_bin_content)/nominal_bin_content);
            }
            else {
              smoothed_histo_syst_down_down_delta->SetBinContent(ibin, 0.0);
              smoothed_histo_syst_down_up_delta->SetBinContent(ibin, 0.0);
              smoothed_histo_syst_up_down_delta->SetBinContent(ibin, 0.0);
              smoothed_histo_syst_up_up_delta->SetBinContent(ibin, 0.0);
            }
          }
          auto smoothed_histo_syst_down_down_integral = smoothed_histo_syst_down_down_delta->Integral();
          auto smoothed_histo_syst_down_up_integral = smoothed_histo_syst_down_up_delta->Integral();
          auto smoothed_histo_syst_up_down_integral = smoothed_histo_syst_up_down_delta->Integral();
          auto smoothed_histo_syst_up_up_integral = smoothed_histo_syst_up_up_delta->Integral();
          bool smoothed_histo_syst_down_down_keep = false,
               smoothed_histo_syst_down_up_keep = false,
               smoothed_histo_syst_up_down_keep = false,
               smoothed_histo_syst_up_up_keep = false;
          // find up variation
          if (smoothed_histo_syst_down_down_integral >= smoothed_histo_syst_down_up_integral &&
              smoothed_histo_syst_down_down_integral >= smoothed_histo_syst_up_down_integral &&
              smoothed_histo_syst_down_down_integral >= smoothed_histo_syst_up_up_integral) {
            smoothed_histo_syst_up = smoothed_histo_syst_down_down;
            smoothed_histo_syst_down_down_keep = true;
          }
          else if (smoothed_histo_syst_down_up_integral >= smoothed_histo_syst_down_down_integral &&
              smoothed_histo_syst_down_up_integral >= smoothed_histo_syst_up_down_integral &&
              smoothed_histo_syst_down_up_integral >= smoothed_histo_syst_up_up_integral) {
            smoothed_histo_syst_up = smoothed_histo_syst_down_up;
            smoothed_histo_syst_down_up_keep = true;
          }
          else if (smoothed_histo_syst_up_down_integral >= smoothed_histo_syst_down_up_integral &&
              smoothed_histo_syst_up_down_integral >= smoothed_histo_syst_down_down_integral &&
              smoothed_histo_syst_up_down_integral >= smoothed_histo_syst_up_up_integral) {
            smoothed_histo_syst_up = smoothed_histo_syst_up_down;
            smoothed_histo_syst_up_down_keep = true;
          }
          else if (smoothed_histo_syst_up_up_integral >= smoothed_histo_syst_down_up_integral &&
              smoothed_histo_syst_up_up_integral >= smoothed_histo_syst_up_down_integral &&
              smoothed_histo_syst_up_up_integral >= smoothed_histo_syst_down_down_integral) {
            smoothed_histo_syst_up = smoothed_histo_syst_up_up;
            smoothed_histo_syst_up_up_keep = true;
          }
          // find down variation
          if (smoothed_histo_syst_down_down_integral <= smoothed_histo_syst_down_up_integral &&
              smoothed_histo_syst_down_down_integral <= smoothed_histo_syst_up_down_integral &&
              smoothed_histo_syst_down_down_integral <= smoothed_histo_syst_up_up_integral) {
            smoothed_histo_syst_down = smoothed_histo_syst_down_down;
            smoothed_histo_syst_down_down_keep = true;
          }
          else if (smoothed_histo_syst_down_up_integral <= smoothed_histo_syst_down_down_integral &&
              smoothed_histo_syst_down_up_integral <= smoothed_histo_syst_up_down_integral &&
              smoothed_histo_syst_down_up_integral <= smoothed_histo_syst_up_up_integral) {
            smoothed_histo_syst_down = smoothed_histo_syst_down_up;
            smoothed_histo_syst_down_up_keep = true;
          }
          else if (smoothed_histo_syst_up_down_integral <= smoothed_histo_syst_down_up_integral &&
              smoothed_histo_syst_up_down_integral <= smoothed_histo_syst_down_down_integral &&
              smoothed_histo_syst_up_down_integral <= smoothed_histo_syst_up_up_integral) {
            smoothed_histo_syst_down = smoothed_histo_syst_up_down;
            smoothed_histo_syst_up_down_keep = true;
          }
          else if (smoothed_histo_syst_up_up_integral <= smoothed_histo_syst_down_up_integral &&
              smoothed_histo_syst_up_up_integral <= smoothed_histo_syst_up_down_integral &&
              smoothed_histo_syst_up_up_integral <= smoothed_histo_syst_down_down_integral) {
            smoothed_histo_syst_down = smoothed_histo_syst_up_up;
            smoothed_histo_syst_up_up_keep = true;
          }
          if (!smoothed_histo_syst_down_down_keep) {
            smoothed_histo_syst_down_down->SetDirectory(0);
            delete smoothed_histo_syst_down_down;
            smoothed_histo_syst_down_down = nullptr;
          }
          if (!smoothed_histo_syst_down_up_keep) {
            smoothed_histo_syst_down_up->SetDirectory(0);
            delete smoothed_histo_syst_down_up;
            smoothed_histo_syst_down_up = nullptr;
          }
          if (!smoothed_histo_syst_up_down_keep) {
            smoothed_histo_syst_up_down->SetDirectory(0);
            delete smoothed_histo_syst_up_down;
            smoothed_histo_syst_up_down = nullptr;
          }
          if (!smoothed_histo_syst_up_up_keep) {
            smoothed_histo_syst_up_up->SetDirectory(0);
            delete smoothed_histo_syst_up_up;
            smoothed_histo_syst_up_up = nullptr;
          }
          delete smoothed_histo_syst_down_down_delta;
          delete smoothed_histo_syst_down_up_delta;
          delete smoothed_histo_syst_up_down_delta;
          delete smoothed_histo_syst_up_up_delta;
          smoothed_histo_syst_up->SetName(Form("%s_SysWSMaker_Smoothing__1up", combined_samples_to_smooth[isetting].c_str()));
          if (smoothed_histo_syst_up == smoothed_histo_syst_down)
            smoothed_histo_syst_down = (TH1*)smoothed_histo_syst_up->Clone();
          smoothed_histo_syst_down->SetName(Form("%s_SysWSMaker_Smoothing__1down", combined_samples_to_smooth[isetting].c_str()));
          bool up_nominal = true,
               down_nominal = true;
          for (Int_t ibin = 1; ibin <= smoothed_histo_syst_up->GetXaxis()->GetNbins(); ++ibin) {
            if (!up_nominal && !down_nominal) break;
            if (smoothed_histo_syst_up->GetBinContent(ibin) != smoothed_histo->GetBinContent(ibin))
              up_nominal = false;
            if (smoothed_histo_syst_down->GetBinContent(ibin) != smoothed_histo->GetBinContent(ibin))
              down_nominal = false;
          }
          if (!up_nominal) {
            smoothed_nominal_histos_syst_up[isetting] = (TH1*)smoothed_histo_syst_up->Clone();
            smoothed_nominal_histos_syst_up[isetting]->SetDirectory(0);
          }
          else
            smoothed_nominal_histos_syst_up[isetting] = nullptr;
          if (!down_nominal) {
            smoothed_nominal_histos_syst_down[isetting] = (TH1*)smoothed_histo_syst_down->Clone();
            smoothed_nominal_histos_syst_down[isetting]->SetDirectory(0);
          }
          else
            smoothed_nominal_histos_syst_down[isetting] = nullptr;

          // correlations for smoothing
          auto corr = smoothed_result.second;
          double *xbins = new double[smoothed_histo->GetXaxis()->GetNbins()];
          for (int i = 1; i <= smoothed_histo->GetXaxis()->GetNbins(); ++i) {
            xbins[i - 1] = smoothed_histo->GetXaxis()->GetBinLowEdge(i);
          }
          xbins[smoothed_histo->GetXaxis()->GetNbins()] = smoothed_histo->GetXaxis()->GetBinUpEdge(smoothed_histo->GetXaxis()->GetNbins());
          rebinned_corr = new TH2D(Form("%s_correlation_histogram", smoothed_histo->GetName()), "",
                                   smoothed_histo->GetXaxis()->GetNbins(), xbins,
                                   smoothed_histo->GetXaxis()->GetNbins(), xbins);
          for (auto ixbin = 1; ixbin <= smoothed_histo->GetXaxis()->GetNbins(); ++ixbin) {
            for (auto iybin = ixbin; iybin <= smoothed_histo->GetXaxis()->GetNbins(); ++iybin) {
              rebinned_corr->SetBinContent(ixbin, iybin, corr->GetBinContent(ixbin, iybin));
              rebinned_corr->SetBinContent(iybin, ixbin, corr->GetBinContent(ixbin, iybin));
            }
          }
          smoothed_nominal_corrs[isetting] = (TH2*)rebinned_corr->Clone();
          smoothed_nominal_corrs[isetting]->SetTitle(smoothed_histo->GetTitle());
          smoothed_nominal_corrs[isetting]->SetDirectory(0);
          smoothed_nominal_corrs[isetting]->SetOption("COLZ");
          // rebinned_corr->SetName(Form("%s_correlation_histogram", smoothed_histo->GetName()));
          rebinned_corr->SetOption("COLZ");
          // rebinned_corr->Write();

          corr->SetDirectory(0);
          delete corr;
        }
        else
          smoothed_histo = nominal_hists_to_smooth[isetting];

        if (smoothed_histo != nullptr &&
            rebinned_corr != nullptr &&
            smoothed_histo_syst_up != nullptr &&
            smoothed_histo_syst_down != nullptr) {
          smoothed_histo->Write();
          rebinned_corr->Write();
          _file1->cd(systematics_dir_name);
          smoothed_histo_syst_up->Write();
          smoothed_histo_syst_down->Write();
        }
        else {
          Warning("smoothinputs", "Couldn't smooth histogram! Saving un-smoothed histogram instead.");
          smoothed_histo->Write();
        }
      }
    }

    _file0->Close();
    _file1->Close();
    for (auto &h : nominal_hists_to_smooth) h = nullptr;

    // _file2 = (smoothed_nominal_histo != nullptr && unsmoothed_nominal_histo != nullptr) ? TFile::Open(new_full_file_control.Data(), "RECREATE") : nullptr;
    _file2 = (static_cast<decltype(smoothed_nominal_histos.size())>(count(smoothed_nominal_histos.begin(), smoothed_nominal_histos.end(), nullptr)) != smoothed_nominal_histos.size() &&
              static_cast<decltype(unsmoothed_nominal_histos.size())>(count(unsmoothed_nominal_histos.begin(), unsmoothed_nominal_histos.end(), nullptr)) != unsmoothed_nominal_histos.size()) ? TFile::Open(new_full_file_control.Data(), "RECREATE") : nullptr;

    if (_file2 != nullptr) {
      _file2->mkdir(dirname, dirname);
      if (write_new_inputs) {
        _file0 = TFile::Open(copied_file_name.Data());
        _file1 = TFile::Open(new_full_file.Data());
      }
      else {
        _file0 = TFile::Open(file0_file_name_no_write.Data());
        _file1 = TFile::Open(temp_file_name.Data());
      }

      nextkey = _file0->GetListOfKeys();
      while ( (key = (TKey*)nextkey()) ) {

        TString keyname = key->GetName();
        if (key && keyname.Contains("Sys")) {
          file0_sysdir = (TDirectory*)key->ReadObj();
          //Systematic list
          TString dirname = file0_sysdir->GetName();
          TIter nextkey2(file0_sysdir->GetListOfKeys());
          TKey *key2;
          _file0->cd();
          while ( (key2 = (TKey*)nextkey2()) ) {
            TH1 *h = (TH1F*)key2->ReadObj();
            TString origname = key2->GetName();
            TString systname = origname;
            systname.Remove(0, systname.First('_'));
            for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting)
              if (syst_histos[isetting].count(systname.Data()) == 0) syst_histos[isetting][systname.Data()] = nullptr;
            bool is_histo = false;
            decltype(n_samples) iisetting = 0;
            for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
              if (!DetermineSmoothFromFileName(_file0->GetName(), catagories_all_include[isetting], catagories_any_exclude[isetting])) continue;
              for (unsigned i = 0; i < samples_to_smooth[isetting].size(); ++i) {
                is_histo |= origname.BeginsWith((samples_to_smooth[isetting][i]+"_").c_str());
                if (is_histo) break;
              }
              if (is_histo) {iisetting = isetting; break;}
            }
            // for (unsigned i = 0; i < histos_to_smooth.size(); ++i) {
            //   is_histo |= origname.BeginsWith((histos_to_smooth[i]+"_").c_str());
            // }
            if(is_histo) {
              if (syst_histos[iisetting][systname.Data()] == nullptr)
                syst_histos[iisetting][systname.Data()] = static_cast<TH1*>(h->Clone());
              else
                syst_histos[iisetting][systname.Data()]->Add(h);
            }
          }
          typedef map<string, TH1*>::iterator it_type;
          for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
            for (it_type iterator = syst_histos[isetting].begin(); iterator != syst_histos[isetting].end(); ++iterator) {
              TH1 *h_to_smooth = iterator->second;
              if (h_to_smooth != nullptr) {
                _file2->cd(dirname);
                string hist_name(h_to_smooth->GetName());
                string hist_title(h_to_smooth->GetTitle());
                decltype(n_samples) i_sample = 0;
                // for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
                  for (unsigned i = 0; i < samples_to_smooth[isetting].size(); ++i) {
                    if (hist_name.find(samples_to_smooth[isetting][i]) == 0) i_sample = isetting;
                  }
                // }
                string smoothed_hist_name(hist_name.replace(0, hist_name.find_first_of('_'), combined_samples_to_smooth[i_sample]));
                auto smoothed_histo = (TH1*)_file1->Get(Form("%s/%s", dirname.Data(), smoothed_hist_name.c_str()));

                auto unsmoothed_delta = (TH1*)h_to_smooth->Clone();
                auto smoothed_delta = (TH1*)smoothed_histo->Clone();

                unsmoothed_delta->Add(unsmoothed_nominal_histos[i_sample], -1.0);
                smoothed_delta->Add(smoothed_nominal_histos[i_sample], -1.0);

                auto canvas = new TCanvas(smoothed_hist_name.c_str(), Form("%s #Delta comparison", smoothed_histo->GetName()), 600, 600);
                unsmoothed_delta->SetTitle(Form("%s #Delta comparison", smoothed_hist_name.c_str()));
                unsmoothed_delta->SetStats(kFALSE);

                auto leg = new TLegend(0.6, 0.6, 0.9, 0.9);
                leg->SetHeader("Kernel Density Estimation");
                leg->SetBorderSize(0);
                leg->SetFillStyle(0);

                DrawHistogramCircleE1(leg, "Raw", unsmoothed_delta);
                DrawHistogramBand(leg, "Smoothed", smoothed_delta, kBlue);

                leg->Draw();

                canvas->Write();

                delete canvas;

                // unsmoothed_delta->Chi2Test(smoothed_delta, "WW P");

                _file0->cd();
              }
            }
          }
          _file2->cd(dirname);
          for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
            if (smoothed_nominal_histos[isetting] == nullptr) continue;
            auto smoothed_histo = smoothed_nominal_histos[isetting];
            auto smoothed_histo_up = smoothed_nominal_histos_syst_up[isetting];
            // string smoothed_hist_name(hist_name.replace(0, hist_name.find_first_of('_'), combined_samples_to_smooth[i_sample]));

            auto smoothed_delta_up = (TH1*)smoothed_histo->Clone();

            smoothed_delta_up->Add(smoothed_histo_up, -1.0);

            auto canvas = new TCanvas(smoothed_histo_up->GetName(), Form("%s #Delta comparison", smoothed_histo->GetName()), 600, 600);
            smoothed_delta_up->SetTitle(Form("%s #Delta comparison", smoothed_histo_up->GetName()));
            smoothed_delta_up->SetStats(kFALSE);

            auto leg = new TLegend(0.6, 0.6, 0.9, 0.9);
            leg->SetHeader("Kernel Density Estimation");
            leg->SetBorderSize(0);
            leg->SetFillStyle(0);

            DrawHistogramBand(leg, "Smoothed", smoothed_delta_up, kBlue);

            leg->Draw();

            canvas->Write();

            delete canvas;

            auto smoothed_histo_down = smoothed_nominal_histos_syst_down[isetting];

            auto smoothed_delta_down = (TH1*)smoothed_histo->Clone();

            smoothed_delta_down->Add(smoothed_histo_down, -1.0);

            canvas = new TCanvas(smoothed_histo_down->GetName(), Form("%s #Delta comparison", smoothed_histo->GetName()), 600, 600);
            smoothed_delta_down->SetTitle(Form("%s #Delta comparison", smoothed_histo_down->GetName()));
            smoothed_delta_down->SetStats(kFALSE);

            leg = new TLegend(0.6, 0.6, 0.9, 0.9);
            leg->SetHeader("Kernel Density Estimation");
            leg->SetBorderSize(0);
            leg->SetFillStyle(0);

            DrawHistogramBand(leg, "Smoothed", smoothed_delta_down, kBlue);

            leg->Draw();

            canvas->Write();

            delete canvas;
          }
          _file0->cd();

          for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting)
            syst_histos[isetting].clear();
        }
        _file0->cd();
      }

      // NOTE: nominal comparison control plot
      _file2->cd();
      auto canvas = new TCanvas("nominal_smoothing_comp", "Unsmoothed vs. smoothed nominal", 600, 600);
      for (decltype(n_samples) isetting = 0; isetting < n_samples; ++isetting) {
        if (unsmoothed_nominal_histos[isetting] == nullptr) continue;
        canvas->Clear();
        canvas->SetName(Form("%s_nominal_smoothing_comp", combined_samples_to_smooth[isetting].c_str()));
        unsmoothed_nominal_histos[isetting]->SetStats(kFALSE);

        auto leg = new TLegend(0.6, 0.6, 0.9, 0.9);
        leg->SetHeader("Kernel Density Estimation");
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);

        DrawHistogramCircleE1(leg, "Raw", unsmoothed_nominal_histos[isetting]);
        DrawHistogramBand(leg, "Smoothed", smoothed_nominal_histos[isetting], kBlue, kGreen);
        if (smoothed_nominal_histos_syst_up[isetting] != nullptr) {
          smoothed_nominal_histos_syst_up[isetting]->SetLineColor(kRed);
          smoothed_nominal_histos_syst_up[isetting]->Draw("hist same");
          leg->AddEntry(smoothed_nominal_histos_syst_up[isetting], "Systematic Up", "L");
        }
        if (smoothed_nominal_histos_syst_down[isetting] != nullptr) {
          smoothed_nominal_histos_syst_down[isetting]->SetLineColor(kMagenta + 2);
          smoothed_nominal_histos_syst_down[isetting]->Draw("hist same");
          leg->AddEntry(smoothed_nominal_histos_syst_down[isetting], "Systematic Down", "L");
        }

        leg->Draw();

        canvas->Write();

        canvas->Clear();

        canvas->SetName(Form("%s_smoothed_nominal_correlation_matrix", combined_samples_to_smooth[isetting].c_str()));
        smoothed_nominal_corrs[isetting]->SetStats(kFALSE);
        smoothed_nominal_corrs[isetting]->Draw("COLZ");

        canvas->Write();
      }

      delete canvas;

      _file0->Close();
      _file1->Close();
      _file2->Close();
    }

    // JWH
    for (auto &p : smoothed_nominal_histos) {
      if (p != nullptr) {
        delete p;
        p = nullptr;
      }
    }
    for (auto &p : smoothed_nominal_histos_syst_up) {
      if (p != nullptr) {
        delete p;
        p = nullptr;
      }
    }
    for (auto &p : smoothed_nominal_histos_syst_down) {
      if (p != nullptr) {
        delete p;
        p = nullptr;
      }
    }
    for (auto &p : unsmoothed_nominal_histos) {
      if (p != nullptr) {
        delete p;
        p = nullptr;
      }
    }
    for (auto &p : smoothed_nominal_corrs) {
      if (p != nullptr) {
        delete p;
        p = nullptr;
      }
    }

    // NOTE for exploration only
    if (!write_new_inputs) {
      remove(temp_file_name.Data());
    }
  }
}


struct TKernelDensityEstimatorResult : public TNamed {
  TH1 *SmoothedHistogram = nullptr;
  TH1 *CorrelationHistogram = nullptr;
  ClassDef(TKernelDensityEstimatorResult, 1);
};

ClassImp(TKernelDensityEstimatorResult)

// these are technical details for the n-dimensional kernel estimator
namespace details_
{

// relies on std::allocator_traits to fill in the rest
template <class T>
struct BigAllocator /*: public std::allocator<T>*/ {
public:
  using value_type = T;
  using difference_type = long long;
  using size_type = unsigned long long;
  BigAllocator () = default;
  template <class U>
  BigAllocator (const BigAllocator<U>&) {}

  T* allocate(size_type n) {
    if (n <= std::numeric_limits<size_type>::max() / sizeof(T)) {
      if (auto ptr = std::malloc(n * sizeof(T))) {
        return static_cast<T*>(ptr);
      }
    }
    throw std::bad_alloc();
  }
  void deallocate(T* ptr, size_type /*n*/) {
    std::free(ptr);
  }
};

template <typename T, typename U>
inline bool operator == (const BigAllocator<T>&, const BigAllocator<U>&) {
  return true;
}

template <typename T, typename U>
inline bool operator != (const BigAllocator<T>& a, const BigAllocator<U>& b) {
  return !(a == b);
}


// TODO: redo the GetIndex to use a const-to-type helper that would clear up the logic
//       (correct as written, just not easy to follow)
// inner loop
template <unsigned J>
struct GetIndex_impl_impl {
  template <unsigned DIM, typename T, typename U>
  static void apply (T &p, const U &ngp) {
    p *= std::get<DIM-J>(ngp);
    GetIndex_impl_impl<J-1>::template apply<DIM>(p, ngp);
  }
};

template <>
struct GetIndex_impl_impl<1> {
  template <unsigned DIM, typename T, typename U>
  static void apply (T &p, const U &ngp) {
    p *= std::get<DIM-1>(ngp);
  }
};

// outer loop
template <unsigned I>
struct GetIndex_impl {
  template <unsigned DIM, typename T, typename U>
  static void apply (T &r, const U &indices, const U &ngp) {
    T product = 1;
    GetIndex_impl_impl<I-1>::template apply<DIM>(product, ngp);
    // NOTE: this is the loop the template above unrolls
    // for (unsigned j = DIM - I + 1; j < DIM; ++j) {
    //   product *= ngp[j];
    // }
    r += product*std::get<DIM-I>(indices);
    GetIndex_impl<I-1>::template apply<DIM>(r, indices, ngp);
  }
};

template <>
struct GetIndex_impl<1> {
  template <unsigned DIM, typename T, typename U>
  static void apply (T &r, const U &indices, const U &/*ngp*/) {
    r += std::get<DIM-1>(indices);
  }
};

template <unsigned I>
struct GetIndices_impl {
  template <unsigned DIM, typename T, typename U, typename V>
  static void apply (T &r, const U &index, const V &ngp) {
    std::get<I>(r) = index % std::get<I>(ngp);
    GetIndices_impl<I-1>::template apply<DIM>(r, (index - std::get<I>(r))/std::get<I>(ngp), ngp);
  }
};

template <>
struct GetIndices_impl<0> {
  template <unsigned DIM, typename T, typename U, typename V>
  static void apply (T &r, const U &index, const V &ngp) {
    std::get<0>(r) = index % std::get<0>(ngp);
  }
};

template<class T>
inline constexpr T pow(const T base, unsigned const exponent)
{
  // (parentheses not required in next line)
  return (exponent == 0)     ? 1 :
         (exponent % 2 == 0) ? pow(base, exponent/2)*pow(base, exponent/2) :
         base * pow(base, (exponent-1)/2) * pow(base, (exponent-1)/2);
}

}



template <unsigned DIMENSION = 1>
class DensityEstimator {
  // NOTE: all coordinates are linearized (row-major)
  using CoordIndex = unsigned long long;
  // used for all arrays of length DIMENSION
  template <typename T> using DimArray = std::array<T, DIMENSION>;
  // used for all sparcely populated coordinates (class assumes incoming data is sparce)
  template <typename T> using SparceCoordList = std::unordered_map<CoordIndex, T>;
  // used for coordinates that aren't sparce
  template <typename T> using BigVector = std::vector<T, details_::BigAllocator<T>>;

  bool m_use_adaptive_bandwidth = true,
       m_full_correlations = false;
  double m_normalization = 1.0;
  DimArray<int> m_number_of_grid_points;
  SparceCoordList<DimArray<double>> m_xs;
  SparceCoordList<double>           m_ws,
                                    m_ues,
                                    m_des;
  DimArray<std::pair<double, double>> m_bounds,
                                      m_selection_bounds;
  DimArray<double> m_default_hs,
                   m_taus,
                   m_variance_fractions,
                   m_event_factors;
  DimArray<TF1> m_scaling_funcs,
                m_scaling_func_derivatives,
                m_inverse_scaling_funcs,
                m_kernels;

  enum class ErrorEstimationType {None, Prop, Jackknife, Bootstrap} m_error_type = ErrorEstimationType::None;
  enum class KNNType {Standard, Large, Small};

  // DimArray<unsigned> m_knn_factor_distances;
  DimArray<double> m_knn_factor_distances;
  DimArray<KNNType> m_knn_factor_settings;



  // NOTE: only 1D KNN for now
  int KNNDistance (const BigVector<double> &c, const double &sum_c,
                   const int &i, const unsigned &dim, const double &d,
                   const KNNType &t) const
  {
    int distance = 0;
    unsigned count = 0;
    double dist_sum = d*sum_c - c[i];

    if (dist_sum <= 0.0) return 0.0;
    // normal KNN
    if (t == KNNType::Standard) {
      int left_index = i - 1,
          right_index = i + 1;
      double left_sum = dist_sum,
             right_sum = dist_sum;
      auto left_distance = 0,
           right_distance = 0;
      // // NOTE: find first non-zero index to the (wrapped) left of the starting index
      // while (left_index == i - 1) {
      //   ++count;
      //   ++left_distance;
      //   auto wrapped = GetWrappedIndex(left_index - count, dim);
      //   if (c[wrapped] != 0.0) {
      //     left_index = left_index - count;
      //     left_sum -= c[wrapped];
      //   }
      // }
      // count = 0;
      // // NOTE: find first non-zero index to the (wrapped) right of the starting index
      // while (right_index == i + 1) {
      //   ++count;
      //   ++right_distance;
      //   auto wrapped = GetWrappedIndex(right_index + count, dim);
      //   if (c[wrapped] > 0.0) {
      //     right_index = right_index + count;
      //     right_sum -= c[wrapped];
      //   }
      // }
      while (dist_sum > 0.0) {
        // determine the nearest distance
        if (left_sum < right_sum) {
          distance = left_distance;
          dist_sum = left_sum;
        }
        else {
          distance = right_distance;
          dist_sum = right_sum;
        }
        // compute next indicies
        // (less-than-equal is important)
        bool first_block = (left_sum >= right_sum),
             second_block = (left_sum <= right_sum);
        if (first_block) {
          bool should_break = false;
          while (!should_break) {
            --left_index;
            ++left_distance;
            auto wrapped = GetWrappedIndex(left_index, dim);
            left_sum -= c[wrapped];
            should_break = (c[wrapped] != 0.0);
          }
        }
        else if (second_block) {
          bool should_break = false;
          while (!should_break) {
            ++right_index;
            ++right_distance;
            auto wrapped = GetWrappedIndex(right_index, dim);
            right_sum -= c[wrapped];
            should_break = (c[wrapped] != 0.0);
          }
        }
        // auto left_distance = i - left_index,
        //      right_distance = right_index - i;
        // // determine the nearest distance
        // if (left_distance < right_distance) {
        //   ++count;
        //   distance = left_distance;
        // }
        // else {
        //   ++count;
        //   distance = right_distance;
        // }
        // // compute next indicies
        // // (less-than-equal is important)
        // if (left_distance <= right_distance) {
        //   bool should_break = false;
        //   while (!should_break) {
        //     --left_index;
        //     should_break = (c[GetWrappedIndex(left_index, dim)] > 0.0);
        //   }
        // }
        // else if (left_distance >= right_distance) {
        //   bool should_break = false;
        //   while (!should_break) {
        //     ++right_index;
        //     should_break = (c[GetWrappedIndex(right_index, dim)] > 0.0);
        //   }
        // }
      }
    }
    // take the largest/smallest of the kth neighbor on left or right
    else if (t == KNNType::Large || t == KNNType::Small) {
      int left_index = i,
          right_index = i;
      double left_sum = dist_sum,
             right_sum = dist_sum;
      auto left_distance = 0,
           right_distance = 0;
      // left
      while (left_sum > 0.0) {
        --left_index;
        ++left_distance;
        left_sum -= c[GetWrappedIndex(left_index, dim)];
      }
      // right
      while (right_sum > 0.0) {
        ++right_index;
        ++right_distance;
        right_sum -= c[GetWrappedIndex(right_index, dim)];
      }
      if ((left_distance > right_distance && t == KNNType::Large) ||
          (left_distance < right_distance && t == KNNType::Small)) {
        distance = left_distance;
      }
      else {
        distance = right_distance;
      }
    }
    return distance;
  }

  // in normalized coordinates
  int DetermineKNNFactor (const BigVector<double> &c, const double &sum_c, const int &i, const unsigned &dim) const
  {
    if (m_knn_factor_distances[dim] > 0.0) {
      return KNNDistance(c, sum_c, i, dim, m_knn_factor_distances[dim], m_knn_factor_settings[dim]);
    }
    // else return 1;
    else return 0;
  }

  // in normalized coordinates
  // NOTE: only 1D for now
  double ComputeSigma (const BigVector<double> &c, const int &l, const int &u, const unsigned &dim) const
  {
    double result = 0;
    double _ci2 = 0,
           _c = 0,
           _ci = 0,
           _c2 = 0;

    for (int i = l; i <= u; ++i) {
      auto ii = GetWrappedIndex(i, dim);
      _ci2 += c[ii]*i*i;
      _c += c[ii];
      _ci += c[ii]*i;
      _c2 += c[ii]*c[ii];
    }
    if ((_ci2*_c - _ci*_ci) <= 0.0)
      // return TMath::Sqrt(m_number_of_grid_points[dim]);
      return 0.5*m_number_of_grid_points[dim];
    if ((_c*_c - _c2) <= 0.0)
      return 1.0;

    result = TMath::Sqrt((_ci2*_c - _ci*_ci)/(_c*_c - _c2));
    return result;
  }

  int GetWrappedIndex (const int &i, const unsigned &dim) const
  {
    if (i >= 0 && i < m_number_of_grid_points[dim]) return i;
    if (i < 0 && i >= -m_number_of_grid_points[dim]) return -i - 1;
    if (i >= m_number_of_grid_points[dim] && i < 2*m_number_of_grid_points[dim]) return m_number_of_grid_points[dim] - 1 - i%m_number_of_grid_points[dim];
    Error("DensityEstimator::GetWrappedIndex", "Index %d is out of wrapped range", i);
    return -1;
  }

  BigVector<std::pair<CoordIndex, double>>
  ComputeGridOfKernelEvaluations(const CoordIndex &index,
                                 const DimArray<SparceCoordList<int>> &Ls,
                                 const DimArray<SparceCoordList<std::vector<double>>> &ks) const
  {
    CoordIndex total_storage = 1;
    auto indices = this->GetIndices(index);
    for (unsigned dim = 0; dim < DIMENSION; ++dim) {
      if (m_use_adaptive_bandwidth)
        total_storage *= 2*Ls[dim].at(indices[dim]) + 1;
      else
        total_storage *= 2*Ls[dim].at(0) + 1;
    }
    BigVector<std::pair<DimArray<int>, double>> result_(total_storage);
    BigVector<std::pair<CoordIndex, double>> result(total_storage);
    CoordIndex dimension_delta = 1;

    for (unsigned dim = 0; dim < DIMENSION; ++dim) {
      auto L = m_use_adaptive_bandwidth ? Ls[dim].at(indices[dim]): Ls[dim].at(0);
      for (unsigned repeat_delta = 0; repeat_delta < total_storage; repeat_delta += (2*L + 1)*dimension_delta) {
        CoordIndex counter = 0;
        for (typename std::make_signed<CoordIndex>::type l = -L; l <= L; ++l) {
          auto grid_index = GetWrappedIndex(l + indices[dim], dim);
          // FIXME compute inverse coordinate transform here
          for (CoordIndex i = 0; i < dimension_delta; ++i) {
            result_[counter + repeat_delta].first[dim] = grid_index;
            if (m_use_adaptive_bandwidth) {
              if (dim == 0) result_[counter + repeat_delta].second = ks[dim].at(indices[dim])[l < 0 ? -l : l];
              else result_[counter + repeat_delta].second *= ks[dim].at(indices[dim])[l < 0 ? -l : l];
            }
            else {
              if (dim == 0) result_[counter + repeat_delta].second = ks[dim].at(0)[l < 0 ? -l : l];
              else result_[counter + repeat_delta].second *= ks[dim].at(0)[l < 0 ? -l : l];
            }
            ++counter;
          }
        }
      }
      dimension_delta *= 2*L + 1;
    }

    for (CoordIndex i = 0; i < result_.size(); ++i) {
      result[i].first = this->GetIndex(result_[i].first);
      result[i].second = result_[i].second;
    }


    return result;
  }

  bool AreNeighbors(const CoordIndex &k, const CoordIndex &l) const
  {
    auto k_indices = this->GetIndices(k),
         l_indices = this->GetIndices(l);

    for (unsigned dim = 0; dim < DIMENSION; ++dim)
      if (TMath::Abs(k_indices[dim] - l_indices[dim]) > 1) return false;
    return true;
  }

  void ComputeSmoothedValues(BigVector<double> &fs,
                             const DimArray<SparceCoordList<int>> &Ls,
                             const DimArray<SparceCoordList<std::vector<double>>> &ks,
                             const decltype(m_ws) &c,
                             const decltype(m_bounds) &extremes, const DimArray<double> &deltas) const
  {
    // compute smoothed values and errors at grid points
    if (m_use_adaptive_bandwidth) {
      for (auto &p : c) {
        const auto &index = p.first;
        const auto &weight = p.second;
        // skip negative weights and errors
        if (weight <= 0.0) continue;
        auto kernel_grid = ComputeGridOfKernelEvaluations(index, Ls, ks);
        for (auto &pp : kernel_grid) {
          auto this_index = pp.first;
          auto this_kernel = pp.second;

          fs[this_index] += weight*this_kernel;
        }
      }
      for (CoordIndex j = 0; j < fs.size(); ++j) {
        double reverse_scale = 1.0;
        for (unsigned dim = 0; dim < DIMENSION; ++dim)
          reverse_scale *= m_scaling_func_derivatives[dim](m_inverse_scaling_funcs[dim](extremes[dim].first + j*deltas[dim]));
        fs[j] *= reverse_scale;
      }
    }
    else {
      // TODO: make this work whenever global bandwidth is specified
      // // construct c and kappa FFT input
      // auto P = PowerOf2RoundUp(m_number_of_grid_points + L);
      // double *c_input = new double[P],
      //        *kappa_input = new double[P];
      // for (unsigned i = 0; i < c.size(); ++i) c_input[i] = c[i];
      // for (int i = m_number_of_grid_points; i < P; ++i) c_input[i] = 0.0;
      // for (int i = 0; i < P - L; ++i) {
      //   if (i == 0)
      //     kappa_input[i] = m_kernel(0.0);
      //   else {
      //     if (i <= L) {
      //       double h_inv = 1.0/hs[i],
      //              val = h_inv*m_kernel(i*delta*h_inv)/m_xs.size();
      //       kappa_input[i] = val;
      //       kappa_input[P - i] = val;
      //     }
      //     else {
      //       kappa_input[i] = 0.0;
      //     }
      //   }
      // }

      // TODO: perform FFT and inverse FFT (and multiply by derivative of transform)
    }
  }


  void ComputeSmoothedValuesAndErrors(BigVector<double> &fs, BigVector<BigVector<double>> &fs_cov,
  // void ComputeSmoothedValuesAndErrors(BigVector<double> &fs, BigVector<double> &fs_error,
                                      const DimArray<SparceCoordList<int>> &Ls,
                                      const DimArray<SparceCoordList<std::vector<double>>> &ks,
                                      const decltype(m_ws) &c, const decltype(m_ws) &c_error,
                                      // const double &sum_weights,
                                      const decltype(m_bounds) &extremes, const DimArray<double> &deltas) const
  {
    // compute smoothed values and errors at grid points
    BigVector<BigVector<double>> fs_a(fs_cov);
    for (const auto &p : c) {
      const auto &index = p.first;
      const auto &weight = p.second;
      // skip negative weights and errors
      if (weight <= 0.0 || c_error.at(index) <= 0.0) continue;
      auto kernel_grid = ComputeGridOfKernelEvaluations(index, Ls, ks);
      for (auto &pp : kernel_grid) {
        auto this_index = pp.first;
        auto this_kernel = pp.second;

        fs_a[this_index][index] += this_kernel;
        fs[this_index] += weight*this_kernel;
      }
      // FIXME: real error propagation
      // for (auto &pp : c) {
      //   const auto &this_index = p.first;
      //   const auto &this_weight = p.second;
      //   auto this_kernel = ks[][];
      //
      //   if (this_index == index) {
      //     fs_a[this_index][index] += (1-)*this_kernel;
      //   }
      //   else {
      //     fs_a[this_index][index] -= this_kernel;
      //   }
      // }
    }
    for (CoordIndex j = 0; j < fs.size(); ++j) {
      double reverse_scale = 1.0;
      for (unsigned dim = 0; dim < DIMENSION; ++dim)
        reverse_scale *= m_scaling_func_derivatives[dim](m_inverse_scaling_funcs[dim](extremes[dim].first + j*deltas[dim]));
      fs[j] *= reverse_scale;
      for (CoordIndex i = 0; i < fs.size(); ++i) {
        fs_a[j][i] *= reverse_scale;
      }
    }
    if (m_full_correlations) Info("DensityEstimator::ComputeSmoothedValuesAndErrors", "Computing full covariance matrix (patience)");
    for (CoordIndex i = 0; i < fs_cov.size(); ++i) {
      auto j_limit = m_full_correlations ? fs_cov[i].size() : i + 1;
      for (CoordIndex j = i; j < j_limit; ++j) {
        for (typename std::make_signed<CoordIndex>::type k = 0; static_cast<CoordIndex>(k) < fs_a.size(); ++k) {
          if (c_error.count(k) == 0) continue;
          double cek = c_error.at(k);
          if (cek <= 0.0 || fs_a[i][k] <= 0.0) continue;
          for (typename std::make_signed<CoordIndex>::type l = 0; static_cast<CoordIndex>(l) < fs_a[k].size(); ++l) {
            if (c_error.count(l) == 0 || !this->AreNeighbors(k, l)) continue; // covariance matrix is almost diagonal
            fs_cov[i][j] += fs_a[i][k]*cek*c_error.at(l)*fs_a[j][l];
          }
        }
        fs_cov[j][i] = fs_cov[i][j];
      }
    }
  }

  bool BinningEqual(const TH1 *h1, const TH1 *h2) const
  {
    if (h1->GetXaxis()->GetNbins() != h2->GetXaxis()->GetNbins()) return false;
    for (int ibin = 1; ibin <= h1->GetXaxis()->GetNbins(); ++ibin) {
      if (h1->GetXaxis()->GetBinCenter(ibin) != h2->GetXaxis()->GetBinCenter(ibin)) return false;
      if (h1->GetXaxis()->GetBinUpEdge(ibin) != h2->GetXaxis()->GetBinUpEdge(ibin)) return false;
      if (h1->GetXaxis()->GetBinLowEdge(ibin) != h2->GetXaxis()->GetBinLowEdge(ibin)) return false;
    }
    return true;
  }

  // useful for FFT (global bandwidth)
  int PowerOf2RoundUp (int x) const
  {
    if (x < 0) return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
  }

  // useful for FFT (global bandwidth)
  void ComplexAdd (const double &a, const double &b,
                   const double &c, const double &d,
                   double &re, double &im) const
  {
    double k1 = a*(c + d),
           k2 = d*(a + b),
           k3 = c*(b - a);
    re = k1 - k2;
    im = k1 + k3;
  }

  TH1* MakeCorrelationHistogram(const BigVector<BigVector<double>> &final_cov) const
  {
    static int counter = 0;
    auto *result = new TH2D(Form("DensityEstimator_correlation_matrix_n%d", ++counter), "", final_cov.size(), 1, final_cov.size(), final_cov.size(), 1, final_cov.size());
    for (int ibin = 1; ibin <= result->GetXaxis()->GetNbins(); ++ibin) {
      auto jbin_limit = m_full_correlations ? result->GetYaxis()->GetNbins() : ibin;
      for (int jbin = ibin; jbin <= jbin_limit; ++jbin) {
        auto ii = ibin - 1,
             jj = jbin - 1;
        if (final_cov[ii][ii] == 0.0 ||
            final_cov[jj][jj] == 0.0)
          result->SetBinContent(ibin, jbin, 0.0);
        else
          result->SetBinContent(ibin, jbin, final_cov[ii][jj]/(TMath::Sqrt(final_cov[ii][ii]*final_cov[jj][jj])));
        result->SetBinContent(jbin, ibin, result->GetBinContent(ibin, jbin));
      }
    }
    return result;
  }

  decltype(m_xs) GetTransformedCoordinates () const
  {
    decltype(m_xs) txs(m_xs);
    for (unsigned i = 0; i < DIMENSION; ++i)
      std::for_each(txs.begin(), txs.end(),
                    [this, &i](typename decltype(m_xs)::value_type &p) {p.second[i] = this->m_scaling_funcs[i](p.second[i]);}
                    );
    return std::move(txs);
  }

  std::tuple<decltype(m_ws), decltype(m_ws)>
  ComputeGridWeightsAndErrors (const decltype(m_bounds) &a,
                               const decltype(m_bounds) &extremes,
                               const DimArray<double> &deltas) const
  {
    decltype(m_ws) c, c_error;
    auto txs = this->GetTransformedCoordinates();
    // array of surrounding grid points with their "grid wieghts/errors" (along their dimension)
    std::array<std::pair<std::array<unsigned, 2>, std::array<double, 4>>, DIMENSION> grid_weights;

    for (const auto &x_ : txs) {
      auto &index = x_.first;
      auto &x = x_.second;

      // check for bounds
      {
        bool out_of_bounds = false;
        for (unsigned i = 0; i < DIMENSION; ++i) {
          if (x[i] < a[i].first || x[i] > a[i].second) out_of_bounds = true;
          if (out_of_bounds) break;
        }
        if (out_of_bounds) continue;
      }

      auto weight = m_ws.at(index),
           up_error = m_ues.at(index),
           down_error = m_des.at(index),
           ave_error = 0.5*(up_error + down_error);

      for (unsigned i = 0; i < DIMENSION; ++i) {
        auto D = (x[i] - extremes[i].first)/deltas[i];
        auto lower_index = TMath::Max(0, TMath::FloorNint(D)),
             upper_index = TMath::Min(TMath::CeilNint(D), m_number_of_grid_points[i] - 1);
        if (x[i] >= extremes[i].first && x[i] <= extremes[i].second) {
          if (lower_index == upper_index && upper_index == m_number_of_grid_points[i] - 1) --lower_index;
          else if (lower_index == upper_index) ++upper_index;
          // this is a special case of a point lying on the lower bound, but numerical precision reduces it just enough to fall
          // in the previous bin's range
          else if (TMath::AreEqualAbs(D, static_cast<double>(upper_index), 1e-10) && upper_index != m_number_of_grid_points[i] - 1) {
            ++lower_index;
            ++upper_index;
          }
        }
        grid_weights[i].first[0] = lower_index;
        grid_weights[i].first[1] = upper_index;

        auto lower_bound = TMath::Max(extremes[i].first, extremes[i].first + deltas[i]*lower_index),
             upper_bound = TMath::Min(extremes[i].first + deltas[i]*upper_index, extremes[i].second);

        double lower_portion = (upper_bound - x[i])/deltas[i],
               upper_portion = (x[i] - lower_bound)/deltas[i];
        if (x[i] < extremes[i].first) {
          grid_weights[i].second[0] = 1.0;
          grid_weights[i].second[1] = 0.0;
          grid_weights[i].second[2] = 1.0;
          grid_weights[i].second[3] = 0.0;
        }
        else if (x[i] > extremes[i].second) {
          grid_weights[i].second[0] = 0.0;
          grid_weights[i].second[1] = 1.0;
          grid_weights[i].second[2] = 0.0;
          grid_weights[i].second[3] = 1.0;
        }
        else {
          grid_weights[i].second[0] = lower_portion;
          grid_weights[i].second[1] = upper_portion;
          grid_weights[i].second[2] = lower_portion*lower_portion;
          grid_weights[i].second[3] = upper_portion*upper_portion;
        }
      }

      // this effectively loops over all the possible combinations of grid-box bounds
      DimArray<int> extents;
      for (auto &e : extents) e = 2;
      for (unsigned i = 0; i < pow(2, DIMENSION); ++i) {
        DimArray<int> grid_weights_indices;
        details_::GetIndices_impl<DIMENSION-1>::template apply<DIMENSION>(grid_weights_indices, i, extents);

        DimArray<int> grid_indices;
        double grid_weight = 1.0,
               grid_error = 1.0;
        for (unsigned j = 0; j < DIMENSION; ++j) {
          grid_indices[j] = grid_weights[j].first[grid_weights_indices[j]];
          grid_weight *= grid_weights[j].second[grid_weights_indices[j]];
          grid_error *= grid_weights[j].second[2+grid_weights_indices[j]];
        }

        auto grid_index = this->GetIndex(grid_indices);
        if (c.count(grid_index) == 0) {
          c[grid_index] = 0.0;
          c_error[grid_index] = 0.0;
        }
        c[grid_index] += weight*grid_weight;
        c_error[grid_index] += ave_error*ave_error*grid_error;
      }
    }

    for (auto &e : c_error) e.second = TMath::Sqrt(e.second);

    return std::make_tuple(c, c_error);
  }

  double GetSelectionNormalization () const
  {
    double selection_normalization = 0.0;
    for (unsigned i = 0; i < DIMENSION; ++i) {
      if (m_selection_bounds[i].first != m_selection_bounds[i].second) {
        for (auto &p : m_xs) {
          auto &key = p.first;
          auto &x = p.second;
          if (m_selection_bounds[i].first <= x[i] && x[i] <= m_selection_bounds[i].second)
            selection_normalization += m_ws.at(key);
        }
        selection_normalization *= m_normalization;
      }
    }
    if (selection_normalization == 0.0) selection_normalization = m_normalization;
    return selection_normalization;
  }

  BigVector<double> MakeProjection (const decltype(m_ws) &c,
                                 const unsigned &dim,
                                 const DimArray<double> &/*deltas*/) const
  {
    BigVector<double> result(m_number_of_grid_points[dim], 0.0);
    // NOTE: JWH for NDSparceSystHistoSet
    // auto collapsed_volume = 1.0;
    // for (unsigned i = 0; i < deltas.size(); ++i) if (i != dim) collapsed_volume *= deltas[i];

    for (auto &p : c) {
      auto indices = this->GetIndices(p.first);
      auto index = indices[dim];
      result[index] += p.second;
    }
    // NOTE: JWH for NDSparceSystHistoSet
    // for (auto &e : result) e *= collapsed_volume;

    return result;
  }

  std::tuple<DimArray<SparceCoordList<double>>, DimArray<SparceCoordList<int>>, DimArray<SparceCoordList<std::vector<double>>>>
  ComputeBandwidthsRangesAndKernelEvals(const double &selection_normalization,
                                        const decltype(m_ws) &c,
                                        const decltype(m_ws) &/*c_error*/,
                                        const DimArray<double> &deltas) const
  {
    DimArray<SparceCoordList<double>> hs;
    DimArray<SparceCoordList<int>> Ls;
    DimArray<SparceCoordList<std::vector<double>>> ks;
    auto weight_sum = 0.0;
    for (auto &p : c) weight_sum += p.second;

    for (unsigned dim = 0; dim < DIMENSION; ++dim) {
      auto projection = MakeProjection(c, dim, deltas);
      auto projection_sum_of_weights = std::accumulate(projection.begin(), projection.end(), 0.0);
      auto max_h_range = 0.1*m_number_of_grid_points[dim]*deltas[dim]; // TODO: make 0.1 configurable

      // compute global sigma
      double sigma = ComputeSigma(projection, 0, m_number_of_grid_points[dim] - 1, dim);

      if (m_use_adaptive_bandwidth) {
        // TODO: (maybe) change this to reflect RooNDKeysPDF multidimensional bandwidth (be sure to normalize the kernel evals appropriately)
        int sigma_range = TMath::CeilNint(TMath::Power(m_number_of_grid_points[dim], 1./3.)); // TODO: make power configurable
        double constant = TMath::Power(4./(m_event_factors[dim]*3.*selection_normalization), 1./5.);
        for (unsigned i = 0; i < projection.size(); ++i) {
          // FIXME
          // auto local_density = projection[i];
          auto local_density = projection[i]/weight_sum;
          auto knn = 1 + DetermineKNNFactor(projection, projection_sum_of_weights, i, dim);
          double local_sigma = ComputeSigma(projection, i - sigma_range, i + sigma_range, dim),
                 this_sigma = m_variance_fractions[dim]*sigma + (1 - m_variance_fractions[dim])*local_sigma;
          if (local_sigma <= 0.01*this_sigma) this_sigma = local_sigma; // TODO: make 0.01 configurable
          // correct units
          // this_sigma *= delta;
          // local_density /= delta;
          // NOTE: cannot ignore zero and negative densities here because this is a projection
          hs[dim][i] = local_density > 0.0 ? knn*constant*TMath::Sqrt(this_sigma/local_density)*deltas[dim] : m_default_hs[dim]*deltas[dim];
          if (hs[dim][i] > max_h_range) hs[dim][i] = max_h_range;
          Ls[dim][i] = TMath::Min(TMath::FloorNint(m_taus[dim]*hs[dim][i]/deltas[dim]), m_number_of_grid_points[dim] - 1);
          if (Ls[dim][i] <= 0) Ls[dim][i] = m_number_of_grid_points[dim] - 1;

          // NOTE: since the resulting smoothed density will be put in a binned histogram,
          //       the kernel evaluations need to be normalized w.r.t. the bin width
          //       (else the normalization is off)
          // if (c[i] == 0.0 && c_error[i] <= 0.0) continue;
          auto ks_sum = 0.0;
          for (unsigned j = 0; j <= static_cast<unsigned>(Ls[dim][i]); ++j) {
            auto inv_h = 1.0/hs[dim][i];
            ks[dim][i].emplace_back(m_kernels[dim]((static_cast<double>(j))*deltas[dim]*inv_h)*inv_h);
            ks_sum += (j == 0 ? 1.0 : 2.0)*ks[dim][i].back();
          }
          auto inv_factor = 1.0/(ks_sum*deltas[dim]);
          for (unsigned j = 0; j <= static_cast<unsigned>(Ls[dim][i]); ++j) {
            ks[dim][i][j] *= inv_factor;
          }
        }
      }
      else {
        hs[dim][0] = m_default_hs[dim]*deltas[dim];
        Ls[dim][0] = TMath::Min(TMath::FloorNint(m_taus[dim]*m_default_hs[dim]), m_number_of_grid_points[dim] - 1);
        // NOTE: since the resulting smoothed density will be put in a binned histogram,
        //       the kernel evaluations need to be normalized w.r.t. the bin width
        //       (else the normalization is off)
        auto ks_sum = 0.0,
             inv_h = 1.0/hs[dim][0];
        for (unsigned j = 0; j <= static_cast<unsigned>(Ls[dim][0]); ++j) {
          ks[dim][0].emplace_back(m_kernels[dim]((static_cast<double>(j))*deltas[dim]*inv_h)*inv_h);
          ks_sum += (j == 0 ? 1.0 : 2.0)*ks[dim][0].back();
        }
        auto inv_factor = 1.0/(ks_sum*deltas[dim]);
        for (unsigned j = 0; j <= static_cast<unsigned>(Ls[dim][0]); ++j) {
          ks[dim][0][j] *= inv_factor;
        }
      }
    }

    return std::make_tuple(hs, Ls, ks);
  }

  void ComputeJackknifeErrors(decltype(m_ws) &c,
                              BigVector<double> &fs, BigVector<BigVector<double>> &fs_cov,
                              const DimArray<SparceCoordList<int>> &Ls, const DimArray<SparceCoordList<std::vector<double>>> &ks,
                              const decltype(m_bounds) &extremes, const DimArray<double> &deltas) const
  {
    Info("DensityEstimator::ComputeJackknifeErrors", "Computing error associated with estimation using the jackknife technique.");
    SparceCoordList<BigVector<double>> estimates;
    for (auto &p : c) {
      double missing = p.second;
      p.second = 0.0;
      BigVector<double> e(fs.size(), 0.0);
      ComputeSmoothedValues(e, Ls, ks, c, extremes, deltas);
      p.second = missing;
      estimates[p.first] = e;
    }
    double variance_factor = static_cast<double>(estimates.size() - 1)/static_cast<double>(estimates.size());
    double sum_grid_point = 0.0;
    BigVector<double> fs_estimates(fs.size(), 0.0);
    for (unsigned i = 0; i < fs.size(); ++i) {
      sum_grid_point = 0.0;
      for (auto &p : estimates) {
        sum_grid_point += p.second[i];
      }
      fs_estimates[i] = sum_grid_point/static_cast<double>(estimates.size());
    }
    for (unsigned i = 0; i < fs.size(); ++i) {
      auto j_limit = m_full_correlations ? fs.size() : i + 1;
      for (unsigned j = i; j < j_limit; ++j) {
        for (auto &p : estimates) {
          fs_cov[i][j] += (p.second[i] - fs_estimates[i])*(p.second[j] - fs_estimates[j]);
        }
        fs_cov[i][j] *= variance_factor;
        fs_cov[j][i] = fs_cov[i][j];
      }
    }
  }

  TH1* FillSmoothedHistogram(const decltype(m_bounds) &a, const DimArray<double> &deltas,
                             const BigVector<double> &fs,
                            //  const BigVector<double> &fs_error) const
                             const BigVector<BigVector<double>> &fs_cov) const
  {
    if (DIMENSION > 3) return nullptr; // TODO: error message here
    TH1 *result = nullptr;
    double *bin_low_edges[DIMENSION];

    for (unsigned dim = 0; dim < DIMENSION; ++dim) {
      double scaled_start = a[dim].first;
      bin_low_edges[dim] = new double[m_number_of_grid_points[dim] + 1];
      for (int i = 0; i <= m_number_of_grid_points[dim]; ++i)
        bin_low_edges[dim][i] = m_inverse_scaling_funcs[dim](scaled_start + i*deltas[dim]);
    }

    if (DIMENSION == 1) {
      result = new TH1D("DensityEstimator_smoothed_histogram", "",
                             m_number_of_grid_points[0], bin_low_edges[0]);
    }
    else if (DIMENSION == 2) {
      result = new TH2D("DensityEstimator_smoothed_histogram", "",
                             m_number_of_grid_points[0], bin_low_edges[0],
                             m_number_of_grid_points[1], bin_low_edges[1]);
    }
    else if (DIMENSION == 3) {
      result = new TH3D("DensityEstimator_smoothed_histogram", "",
                             m_number_of_grid_points[0], bin_low_edges[0],
                             m_number_of_grid_points[1], bin_low_edges[1],
                             m_number_of_grid_points[2], bin_low_edges[2]);
    }

    // NOTE: not sure this is required
    for (unsigned dim = 0; dim < DIMENSION; ++dim) {
      delete[] bin_low_edges[dim];
    }

    if (fs.size() == 0) {
      return result;
    }

    DimArray<int> coord;
    auto total_bins = (result->GetNbinsX() + 2)*
                      (result->GetDimension() >= 1 ? result->GetNbinsY() + 2 : 1)*
                      (result->GetDimension() >= 2 ? result->GetNbinsZ() + 2 : 1);
    for (int ibin = 0; ibin < total_bins; ++ibin) {
      if (result->IsBinOverflow(ibin) || result->IsBinUnderflow(ibin)) continue;
      Int_t binx, biny, binz;
      result->GetBinXYZ(ibin, binx, biny, binz);
      coord[0] = binx - 1;
      if (DIMENSION >= 2) coord[1] = biny - 1;
      if (DIMENSION >= 3) coord[2] = binz - 1;
      auto index = this->GetIndex(coord);

      // TODO: try TH1::Fill instead and remove manual bin-by-bin normalization in ComputeBandwidthsRangesAndKernelEvals
      result->SetBinContent(ibin, fs[index]);

      // add errors to histogram
      /*if (m_error_type == ErrorEstimationType::None) {
        result->SetBinError(ibin, TMath::Sqrt(fs[index]));
      }
      else */if (m_error_type == ErrorEstimationType::Prop || m_error_type == ErrorEstimationType::Jackknife) {
        // FIXME error matrix far too large
        result->SetBinError(ibin, TMath::Sqrt(fs_cov[index][index]));
        // result->SetBinError(ibin, TMath::Sqrt(fs_error[index]));
      }
      else if (m_error_type == ErrorEstimationType::Bootstrap) {
      }
    }

    return result;
  }

  std::tuple<TH1*, BigVector<BigVector<double>>>
  RebinAndComputeErrors(TH1 *result, TH1 *binning_histogram,
                       const BigVector<BigVector<double>> &fs_cov,
                       const double &sum_weights) const
  {
    result->SetName("DensityEstimator_smoothed_histogram_old");
    auto new_result = static_cast<TH1*>(binning_histogram->Clone("DensityEstimator_smoothed_histogram"));
    result->SetDirectory(0);
    new_result->Reset();
    auto new_total_storage = new_result->GetXaxis()->GetNbins();
    if (new_result->GetDimension() >= 2) new_total_storage *= new_result->GetYaxis()->GetNbins();
    if (new_result->GetDimension() >= 3) new_total_storage *= new_result->GetZaxis()->GetNbins();
    auto compute_error = (new_total_storage <= 1000); // TODO: make '1000' configurable
    BigVector<BigVector<double>> final_cov;
    if (compute_error)
      final_cov = BigVector<BigVector<double>>(new_total_storage,
                                               BigVector<double>(new_total_storage, 0.0));
    // auto a_matrix = final_cov;
    SparceCoordList<SparceCoordList<double>> a_matrix;

    auto old_ixbin = 1;
    for (int ixbin = 1; ixbin <= new_result->GetXaxis()->GetNbins(); ++ixbin) {
      // account for old range that extends to lower values than the binning histogram
      while (result->GetXaxis()->GetBinUpEdge(old_ixbin) < new_result->GetXaxis()->GetBinLowEdge(1)) ++old_ixbin;
      // account for the reverse of above
      if (new_result->GetXaxis()->GetBinUpEdge(ixbin) < result->GetXaxis()->GetBinLowEdge(1)) continue;

      double new_up_x = new_result->GetXaxis()->GetBinUpEdge(ixbin),
             new_low_x = new_result->GetXaxis()->GetBinLowEdge(ixbin),
             old_up_x = result->GetXaxis()->GetBinUpEdge(old_ixbin),
             old_low_x = result->GetXaxis()->GetBinLowEdge(old_ixbin);

      if (new_result->GetDimension() == 1) {
        auto new_global_bin = new_result->GetBin(ixbin),
             old_global_bin = result->GetBin(old_ixbin);

        SparceCoordList<double> a;
        double new_weight_sum = 0.0;

        // is old bin within new bin's range?
        while ((old_up_x > new_low_x && old_up_x <= new_up_x) ||
               (old_low_x >= new_low_x && old_low_x < new_up_x) ||
               (old_low_x <= new_low_x && old_up_x >= new_up_x)) {
          double a_weight = 0.0;

          if (old_low_x >= new_low_x && old_up_x <= new_up_x) a_weight = 1.0;
          else if (old_low_x <= new_low_x && old_up_x >= new_up_x) a_weight = (new_up_x - new_low_x)/(old_up_x - old_low_x);
          else if (old_low_x < new_low_x && old_up_x > new_low_x && old_up_x <= new_up_x) a_weight = (old_up_x - new_low_x)/(old_up_x - old_low_x);
          else if (old_low_x >= new_low_x && old_low_x < new_up_x && old_up_x > new_up_x) a_weight = (new_up_x - old_low_x)/(old_up_x - old_low_x);

          a_weight *= (old_up_x - old_low_x)/(new_up_x - new_low_x);
          a[old_global_bin] = a_weight;

          new_weight_sum += a[old_global_bin]*result->GetBinContent(old_global_bin);

          if (old_up_x > new_up_x) break;
          else ++old_ixbin;
          if (old_ixbin > result->GetXaxis()->GetNbins()) break;

          old_global_bin = result->GetBin(old_ixbin);
          old_up_x = result->GetXaxis()->GetBinUpEdge(old_ixbin);
          old_low_x = result->GetXaxis()->GetBinLowEdge(old_ixbin);
        }

        a_matrix[new_global_bin] = a;

        new_result->SetBinContent(new_global_bin, new_weight_sum);

        if (old_ixbin > result->GetXaxis()->GetNbins()) break;
      }
      else {
        auto old_ixbin_origin = old_ixbin;
        auto old_iybin = 1;

        for (int iybin = 1; iybin <= new_result->GetYaxis()->GetNbins(); ++iybin) {
          // account for old range that extends to lower values than the binning histogram
          while (result->GetYaxis()->GetBinUpEdge(old_iybin) < new_result->GetYaxis()->GetBinLowEdge(1)) ++old_iybin;
          // account for the reverse of above
          if (new_result->GetYaxis()->GetBinUpEdge(iybin) < result->GetYaxis()->GetBinLowEdge(1)) continue;

          double new_up_y = new_result->GetYaxis()->GetBinUpEdge(iybin),
                 new_low_y = new_result->GetYaxis()->GetBinLowEdge(iybin),
                 old_up_y = result->GetYaxis()->GetBinUpEdge(old_iybin),
                 old_low_y = result->GetYaxis()->GetBinLowEdge(old_iybin);

          old_ixbin = old_ixbin_origin;

          if (new_result->GetDimension() == 2) {
            auto new_global_bin = new_result->GetBin(ixbin, iybin),
                 old_global_bin = result->GetBin(old_ixbin, old_iybin);

            SparceCoordList<double> a;
            double new_weight_sum = 0.0;

            // is old bin within new bin's range?
            auto old_iybin_origin = old_iybin;
            while (((old_up_x > new_low_x && old_up_x <= new_up_x) ||
                    (old_low_x >= new_low_x && old_low_x < new_up_x) ||
                    (old_low_x <= new_low_x && old_up_x >= new_up_x)) &&
                   ((old_up_y > new_low_y && old_up_y <= new_up_y) ||
                    (old_low_y >= new_low_y && old_low_y < new_up_y) ||
                    (old_low_y <= new_low_y && old_up_y >= new_up_y))) {
              double a_weight = 0.0;

              if (old_low_x >= new_low_x && old_up_x <= new_up_x) a_weight = 1.0;
              else if (old_low_x <= new_low_x && old_up_x >= new_up_x) a_weight = (new_up_x - new_low_x)/(old_up_x - old_low_x);
              else if (old_low_x < new_low_x && old_up_x > new_low_x && old_up_x <= new_up_x) a_weight = (old_up_x - new_low_x)/(old_up_x - old_low_x);
              else if (old_low_x >= new_low_x && old_low_x < new_up_x && old_up_x > new_up_x) a_weight = (new_up_x - old_low_x)/(old_up_x - old_low_x);
              if (old_low_y >= new_low_y && old_up_y <= new_up_y) a_weight *= 1.0;
              else if (old_low_y <= new_low_y && old_up_y >= new_up_y) a_weight *= (new_up_y - new_low_y)/(old_up_y - old_low_y);
              else if (old_low_y < new_low_y && old_up_y > new_low_y && old_up_y <= new_up_y) a_weight *= (old_up_y - new_low_y)/(old_up_y - old_low_y);
              else if (old_low_y >= new_low_y && old_low_y < new_up_y && old_up_y > new_up_y) a_weight *= (new_up_y - old_low_y)/(old_up_y - old_low_y);

              a_weight *= (old_up_x - old_low_x)/(new_up_x - new_low_x);
              a_weight *= (old_up_y - old_low_y)/(new_up_y - new_low_y);
              old_global_bin = result->GetBin(old_ixbin, old_iybin);
              a[old_global_bin] = a_weight;

              new_weight_sum += a[old_global_bin]*result->GetBinContent(old_global_bin);

              ++old_iybin;
              old_up_y = result->GetYaxis()->GetBinUpEdge(old_iybin);
              old_low_y = result->GetYaxis()->GetBinLowEdge(old_iybin);
              if (old_low_y > new_up_y) {
                ++old_ixbin;
                old_up_x = result->GetXaxis()->GetBinUpEdge(old_ixbin);
                old_low_x = result->GetXaxis()->GetBinLowEdge(old_ixbin);
                if (old_low_x <= new_up_x) {
                  old_iybin = old_iybin_origin;
                  old_up_y = result->GetYaxis()->GetBinUpEdge(old_iybin);
                  old_low_y = result->GetYaxis()->GetBinLowEdge(old_iybin);
                }
              }
            }


            a_matrix[new_global_bin] = a;

            new_result->SetBinContent(new_global_bin, new_weight_sum);

            if (old_ixbin > result->GetXaxis()->GetNbins() &&
                old_iybin > result->GetYaxis()->GetNbins()) break;
          }
          else {
            auto old_ixbin_origin = old_ixbin;
            auto old_iybin_origin = old_iybin;
            auto old_izbin = 1;

            for (int izbin = 1; izbin <= new_result->GetZaxis()->GetNbins(); ++izbin) {
              // account for old range that extends to lower values than the binning histogram
              while (result->GetZaxis()->GetBinUpEdge(old_izbin) < new_result->GetZaxis()->GetBinLowEdge(1)) ++old_izbin;
              // account for the reverse of above
              if (new_result->GetZaxis()->GetBinUpEdge(izbin) < result->GetZaxis()->GetBinLowEdge(1)) continue;

              double new_up_z = new_result->GetZaxis()->GetBinUpEdge(izbin),
                     new_low_z = new_result->GetZaxis()->GetBinLowEdge(izbin),
                     old_up_z = result->GetZaxis()->GetBinUpEdge(old_izbin),
                     old_low_z = result->GetZaxis()->GetBinLowEdge(old_izbin);

              old_ixbin = old_ixbin_origin;
              old_iybin = old_iybin_origin;

              auto new_global_bin = new_result->GetBin(ixbin, iybin, izbin),
                   old_global_bin = result->GetBin(old_ixbin, old_iybin, old_izbin);

              SparceCoordList<double> a;
              double new_weight_sum = 0.0;

              // is old bin within new bin's range?
              auto old_izbin_origin = old_izbin;
              while (((old_up_x > new_low_x && old_up_x <= new_up_x) ||
                      (old_low_x >= new_low_x && old_low_x < new_up_x) ||
                      (old_low_x <= new_low_x && old_up_x >= new_up_x)) &&
                     ((old_up_y > new_low_y && old_up_y <= new_up_y) ||
                      (old_low_y >= new_low_y && old_low_y < new_up_y) ||
                      (old_low_y <= new_low_y && old_up_y >= new_up_y)) &&
                     ((old_up_z > new_low_z && old_up_z <= new_up_z) ||
                      (old_low_z >= new_low_z && old_low_z < new_up_z) ||
                      (old_low_z <= new_low_z && old_up_z >= new_up_z))) {
                double a_weight = 0.0;

                if (old_low_x >= new_low_x && old_up_x <= new_up_x) a_weight = 1.0;
                else if (old_low_x <= new_low_x && old_up_x >= new_up_x) a_weight = (new_up_x - new_low_x)/(old_up_x - old_low_x);
                else if (old_low_x < new_low_x && old_up_x > new_low_x && old_up_x <= new_up_x) a_weight = (old_up_x - new_low_x)/(old_up_x - old_low_x);
                else if (old_low_x >= new_low_x && old_low_x < new_up_x && old_up_x > new_up_x) a_weight = (new_up_x - old_low_x)/(old_up_x - old_low_x);
                if (old_low_y >= new_low_y && old_up_y <= new_up_y) a_weight *= 1.0;
                else if (old_low_y <= new_low_y && old_up_y >= new_up_y) a_weight *= (new_up_y - new_low_y)/(old_up_y - old_low_y);
                else if (old_low_y < new_low_y && old_up_y > new_low_y && old_up_y <= new_up_y) a_weight *= (old_up_y - new_low_y)/(old_up_y - old_low_y);
                else if (old_low_y >= new_low_y && old_low_y < new_up_y && old_up_y > new_up_y) a_weight *= (new_up_y - old_low_y)/(old_up_y - old_low_y);
                if (old_low_z >= new_low_z && old_up_z <= new_up_z) a_weight *= 1.0;
                else if (old_low_z <= new_low_z && old_up_z >= new_up_z) a_weight *= (new_up_z - new_low_z)/(old_up_z - old_low_z);
                else if (old_low_z < new_low_z && old_up_z > new_low_z && old_up_z <= new_up_z) a_weight *= (old_up_z - new_low_z)/(old_up_z - old_low_z);
                else if (old_low_z >= new_low_z && old_low_z < new_up_z && old_up_z > new_up_z) a_weight *= (new_up_z - old_low_z)/(old_up_z - old_low_z);

                a_weight *= (old_up_x - old_low_x)/(new_up_x - new_low_x);
                a_weight *= (old_up_y - old_low_y)/(new_up_y - new_low_y);
                a_weight *= (old_up_z - old_low_z)/(new_up_z - new_low_z);
                old_global_bin = result->GetBin(old_ixbin, old_iybin, old_izbin);
                a[old_global_bin] = a_weight;

                new_weight_sum += a[old_global_bin]*result->GetBinContent(old_global_bin);

                ++old_izbin;
                old_up_z = result->GetZaxis()->GetBinUpEdge(old_izbin);
                old_low_z = result->GetZaxis()->GetBinLowEdge(old_izbin);
                if (old_low_z > new_up_z) {
                  ++old_iybin;
                  old_up_y = result->GetYaxis()->GetBinUpEdge(old_iybin);
                  old_low_y = result->GetYaxis()->GetBinLowEdge(old_iybin);
                  if (old_low_y <= new_up_y) {
                    old_izbin = old_izbin_origin;
                    old_up_z = result->GetZaxis()->GetBinUpEdge(old_izbin);
                    old_low_z = result->GetZaxis()->GetBinLowEdge(old_izbin);
                  }
                  else {
                    ++old_ixbin;
                    old_up_x = result->GetXaxis()->GetBinUpEdge(old_ixbin);
                    old_low_x = result->GetXaxis()->GetBinLowEdge(old_ixbin);
                    if (old_low_x <= new_up_x) {
                      old_iybin = old_iybin_origin;
                      old_izbin = old_izbin_origin;
                      old_up_y = result->GetYaxis()->GetBinUpEdge(old_iybin);
                      old_low_y = result->GetYaxis()->GetBinLowEdge(old_iybin);
                      old_up_z = result->GetZaxis()->GetBinUpEdge(old_izbin);
                      old_low_z = result->GetZaxis()->GetBinLowEdge(old_izbin);
                    }
                  }
                }
              }

              a_matrix[new_global_bin] = a;

              new_result->SetBinContent(new_global_bin, new_weight_sum);

              if (old_ixbin > result->GetXaxis()->GetNbins() &&
                  old_iybin > result->GetYaxis()->GetNbins() &&
                  old_izbin > result->GetZaxis()->GetNbins()) break;
            }
            ++old_iybin;
          }
        }
        ++old_ixbin;
      }
    }

    if (compute_error && m_error_type != ErrorEstimationType::None) {
      for (unsigned i = 0; i < final_cov.size(); ++i) {
        auto j_limit = m_full_correlations ? final_cov.size() : i + 1;
        for (unsigned j = i; j < j_limit; ++j) {
          double error = 0.0;
          for (unsigned k = 0; k < fs_cov.size(); ++k) {
            if (a_matrix[i + 1][k + 1] <= 0.0) continue;
            for (unsigned l = 0; l < fs_cov.size(); ++l) {
              // calculate errors in new binning through error propagation
              error = fs_cov[k][l];
              if (error <= 0.0) continue;
              final_cov[i][j] += a_matrix[i + 1][k + 1]*error*a_matrix[j + 1][l + 1];
            }
          }
          final_cov[j][i] = final_cov[i][j];
        }
        new_result->SetBinError(i + 1, TMath::Sqrt(final_cov[i][i]));
      }
    }

    auto rebinning_normalization = new_result->Integral("width");
    if (rebinning_normalization != sum_weights) {
      Info("DensityEstimator::RebinAndComputeErrors", "Normalization has been altered during rebinning (%g). Will force proper normalization to %g.", rebinning_normalization, sum_weights);
      new_result->Scale(sum_weights/rebinning_normalization);
    }
    delete result;
    result = new_result;

    return std::make_tuple(result, final_cov);
  }

  TH1* RescaleHistogram (TH1 *result, const double &selection_normalization) const
  {
    double new_normalization = 1.0;

    DimArray<std::pair<int, int>> result_bin_extremes;

    for (unsigned dim = 0; dim < DIMENSION; ++dim) {
      result_bin_extremes[dim].first = -1;
      result_bin_extremes[dim].second = -1;
      TH1 *h;
      bool is_projection = false;

      if (DIMENSION == 1) {
        h = result;
      }
      else if (DIMENSION == 2) {
        if (dim == 0) {
          h = static_cast<TH2*>(result)->ProjectionX();
          is_projection = true;
        }
        if (dim == 1) {
          h = static_cast<TH2*>(result)->ProjectionY();
          is_projection = true;
        }
      }
      else if (DIMENSION == 3) {
        if (dim == 0) {
          h = static_cast<TH3*>(result)->ProjectionX();
          is_projection = true;
        }
        if (dim == 1) {
          h = static_cast<TH3*>(result)->ProjectionY();
          is_projection = true;
        }
        if (dim == 2) {
          h = static_cast<TH3*>(result)->ProjectionZ();
          is_projection = true;
        }
      }

      if (result_bin_extremes[dim].first == result_bin_extremes[dim].second) {
        result_bin_extremes[dim].first = 1;
        result_bin_extremes[dim].second = h->GetNbinsX();
      }
      else {
        for (int ibin = 1; ibin <= h->GetNbinsX(); ++ibin) {
          auto low_edge = h->GetXaxis()->GetBinLowEdge(ibin),
               up_edge = h->GetXaxis()->GetBinUpEdge(ibin);

          if (result_bin_extremes[dim].first == -1) {
            if (low_edge <= m_selection_bounds[dim].first &&
                up_edge > m_selection_bounds[dim].first)
            result_bin_extremes[dim].first = ibin;
          }
          else if (result_bin_extremes[dim].second == -1) {
            if (low_edge < m_selection_bounds[dim].second &&
                up_edge >= m_selection_bounds[dim].second)
            result_bin_extremes[dim].second = ibin;
          }
          else break;
        }
      }

      if (is_projection) {
        h->SetDirectory(0);
        delete h;
      }
    }

    if (DIMENSION == 1) {
      new_normalization = selection_normalization*
                          result->Integral("width")/result->Integral(result_bin_extremes[0].first,
                                                                     result_bin_extremes[0].second, "width");
    }
    if (DIMENSION == 2) {
      new_normalization = selection_normalization*
                          result->Integral("width")/static_cast<TH2*>(result)->Integral(result_bin_extremes[0].first,
                                                                                        result_bin_extremes[0].second,
                                                                                        result_bin_extremes[1].first,
                                                                                        result_bin_extremes[1].second, "width");
    }
    if (DIMENSION == 3) {
      new_normalization = selection_normalization*
                          result->Integral("width")/static_cast<TH3*>(result)->Integral(result_bin_extremes[0].first,
                                                                                        result_bin_extremes[0].second,
                                                                                        result_bin_extremes[1].first,
                                                                                        result_bin_extremes[1].second,
                                                                                        result_bin_extremes[2].first,
                                                                                        result_bin_extremes[2].second, "width");
    }

    // FIXME
    // result->Scale(new_normalization);
    result->Scale(new_normalization/result->Integral("width"));
    Info("DensityEstimator::RescaleHistogram", "Integral of smoothed histogram after rescaling (should be close to %.3g): %.3g", new_normalization, result->Integral("width"));

    return result;
  }


public:
  DensityEstimator ()
  {
    // assign default values to all the static arrays
    for (auto &e : m_number_of_grid_points) e = 50;
    for (auto &p : m_bounds) { p.first = 0.0; p.second = 0.0; }
    for (auto &p : m_selection_bounds) { p.first = 0.0; p.second = 0.0; }
    for (auto &e : m_default_hs) e = 40;
    for (auto &e : m_taus) e = 4.0;
    for (auto &e : m_variance_fractions) e = 0.0;
    for (auto &e : m_event_factors) e = 1.0;
    for (auto &e : m_scaling_funcs) e = TF1("DensityEstimator_default_scale", "x");
    for (auto &e : m_scaling_func_derivatives) e = TF1("DensityEstimator_default_scale_dervative", "1");
    for (auto &e : m_inverse_scaling_funcs) e = TF1("DensityEstimator_scale_inverse", "x");
    for (auto &e : m_kernels) e = TF1("DensityEstimator_kernel", "TMath::Gaus(x, 0, 1, 1)");
    for (auto &e : m_knn_factor_distances) e = 0.0;
    for (auto &e : m_knn_factor_settings) e = KNNType::Standard;
  }

  void LoadData (const TH1 *h, bool auto_determine_parameters = true)
  {
    if (h == nullptr) return; // TODO: error message here
    if (static_cast<unsigned>(h->GetDimension()) > DIMENSION) return; // TODO: error message here

    auto temp = static_cast<TH1*>(h->Clone());
    auto dim = temp->GetDimension();
    temp->SetDirectory(0);
    temp->Sumw2();
    m_normalization = temp->Integral("width");
    if (m_normalization <= 0) return; // TODO: error message here
    // FIXME
    // temp->Scale(1.0/m_normalization);

    m_xs.clear();
    m_ws.clear();
    m_ues.clear();
    m_des.clear();

    if (auto_determine_parameters) {
      m_bounds[0].first = temp->GetXaxis()->GetXmin();
      m_bounds[0].second = temp->GetXaxis()->GetXmax();
      m_number_of_grid_points[0] = temp->GetXaxis()->GetNbins();
      if (dim >= 2) {
        m_bounds[1].first = temp->GetYaxis()->GetXmin();
        m_bounds[1].second = temp->GetYaxis()->GetXmax();
        m_number_of_grid_points[1] = temp->GetYaxis()->GetNbins();
      }
      if (dim >= 3) {
        m_bounds[2].first = temp->GetZaxis()->GetXmin();
        m_bounds[2].second = temp->GetZaxis()->GetXmax();
        m_number_of_grid_points[2] = temp->GetZaxis()->GetNbins();
      }
    }

    for (decltype(temp->GetNbinsX()) ibinx = 1; ibinx <= temp->GetNbinsX(); ++ibinx) {
      for (decltype(temp->GetNbinsY()) ibiny = 1; ibiny <= temp->GetNbinsY(); ++ibiny) {
        for (decltype(temp->GetNbinsZ()) ibinz = 1; ibinz <= temp->GetNbinsZ(); ++ibinz) {
          auto global_bin = temp->GetBin(ibinx, ibiny, ibinz);
          if (temp->GetBinContent(global_bin) == 0.0 &&
              temp->GetBinError(global_bin) == 0.0) continue; // only makes sense for sparce data
          DimArray<int> indices;
          indices[0] = ibinx - 1;
          if (dim >= 2) indices[1] = ibiny - 1;
          if (dim >= 3) indices[2] = ibinz - 1;
          CoordIndex index = this->GetIndex(indices);
          auto bin_volume = temp->GetXaxis()->GetBinWidth(ibinx);
          if (dim > 1) bin_volume *= temp->GetYaxis()->GetBinWidth(ibiny);
          if (dim > 2) bin_volume *= temp->GetZaxis()->GetBinWidth(ibinz);
          DimArray<double> x;

          // fill x coordinates
          x[0] = temp->GetXaxis()->GetBinCenter(ibinx);
          if (dim >= 2) x[1] = temp->GetYaxis()->GetBinCenter(ibiny);
          if (dim >= 3) x[2] = temp->GetZaxis()->GetBinCenter(ibinz);

          m_xs.emplace(index, x);
          m_ws.emplace(index, temp->GetBinContent(global_bin)*bin_volume);
          m_ues.emplace(index, temp->GetBinErrorUp(global_bin)*bin_volume);
          m_des.emplace(index, temp->GetBinErrorLow(global_bin)*bin_volume);
        }
      }
    }

    double sum = 0.0;
    for (auto &e : m_ws) sum += e.second;
    // Info("DensityEstimator::LoadData", "Input histogram has normalization of %.3g. After rescaling, internal histogram has normalization of %.3g.", m_normalization, sum);

    // clear error if all are zero
    if (std::all_of(m_ues.cbegin(), m_ues.cend(), [](const typename decltype(m_ues)::value_type &p){ return p.second == 0.0; }) &&
        std::all_of(m_des.cbegin(), m_des.cend(), [](const typename decltype(m_ues)::value_type &p){ return p.second == 0.0; })) {
      m_ues.clear();
      m_des.clear();
    }

    delete temp;
  }
  void LoadData (const decltype(m_xs) &xs,
                 bool auto_determine_parameters = true)
  {
    const decltype(m_ws) ws;
    for (auto &p : xs) ws.emplace(p.first, 1.0);
    this->LoadData(xs, ws, {}, {}, auto_determine_parameters);
  }
  void LoadData (const decltype(m_xs) &xs, const decltype(m_ws) &ws,
                 bool auto_determine_parameters = true)
  {
    this->LoadData(xs, ws, {}, {}, auto_determine_parameters);
  }
  void LoadData (const decltype(m_xs) &xs, const decltype(m_ws) &ws,
                 const decltype(m_ues) &es,
                 bool auto_determine_parameters = true)
  {
    this->LoadData(xs, ws, es, es, auto_determine_parameters);
  }
  void LoadData (const decltype(m_xs) &xs, const decltype(m_ws) &ws,
                 const decltype(m_ues) &ues, const decltype(m_des) &des,
                 bool auto_determine_parameters = true)
  {
    m_xs = xs;
    m_ws = ws;
    m_ues = ues;
    m_des = des;
    m_normalization = 0.0;
    for (const auto &p : ws) m_normalization += p.second;
    if (m_normalization > 0.0) for (auto &p : m_ws) p.second /= m_normalization;
    else return; // TODO: error message here

    if (auto_determine_parameters) {
      // FIXME
      // if (DIMENSION >= 1) {
      //   m_number_of_grid_points[0] = m_xs.size();
      //   m_bound[0].first = m_xs[0][0];
      //   m_bound[0].second = m_xs[m_number_of_grid_points[0] - 1][0];
      //   m_selection_bound[0].first = m_bound[0].first;
      //   m_selection_bound[0].second = m_bound[0].second;
      // }
      // if (DIMENSION >= 2) {
      // }
    }
  }

  // void AddData (const double &x, const double &w = 1.0)
  // { m_xs.emplace_back(x); m_ws.emplace_back(w); m_state_changed = true; }


  template <typename... Args_t>
  typename std::enable_if<sizeof...(Args_t) >= 2, CoordIndex>::type
  GetIndex (const Args_t&... args) const
  {
    DimArray<int> indices = {args...};
    return this->GetIndex(indices);
  }

  template <typename... Args_t>
  typename std::enable_if<sizeof...(Args_t) == 1 && std::is_integral<Args_t...>::value, CoordIndex>::type
  GetIndex (const Args_t&... args) const
  {
    DimArray<int> indices = {args...};
    return this->GetIndex(indices);
  }

  CoordIndex GetIndex (const DimArray<int> &indices) const
  {
    CoordIndex result = 0;
    details_::GetIndex_impl<DIMENSION>::template apply<DIMENSION>(result, indices, m_number_of_grid_points);
    return result;
  }

  DimArray<int> GetIndices (const CoordIndex &i) const
  {
    DimArray<int> result;
    details_::GetIndices_impl<DIMENSION-1>::template apply<DIMENSION>(result, i, m_number_of_grid_points);
    return result;
  }



  template <typename DimArray<int>::size_type DIM>
  void SetNumberOfGridPoints (const int &m) { std::get<DIM>(m_number_of_grid_points) = m; }
  void SetNumberOfGridPoints (const typename DimArray<int>::size_type &dim, const int &m) { m_number_of_grid_points[dim] = m; }

  template <typename DimArray<int>::size_type DIM>
  void SetLowerBound (const double &l) { std::get<DIM>(m_bounds).first = l; }
  void SetLowerBound (const typename DimArray<int>::size_type &dim, const double &l) { m_bounds[dim].first = l; }
  template <typename DimArray<int>::size_type DIM>
  void SetUpperBound (const double &u) { std::get<DIM>(m_bounds).second = u; }
  void SetUpperBound (const typename DimArray<int>::size_type &dim, const double &u) { m_bounds[dim].second = u; }

  void UseGlobalBandwidth () { m_use_adaptive_bandwidth = false; }
  template <typename DimArray<int>::size_type DIM>
  void SetGlobalBandwidth (const double &h)
  { m_use_adaptive_bandwidth = false; std::get<DIM>(m_default_hs) = h; }
  void SetGlobalBandwidth (const typename DimArray<int>::size_type &dim, const double &h)
  { m_use_adaptive_bandwidth = false; m_default_hs[dim] = h; }

  void UseAdaptiveBandwidth () { m_use_adaptive_bandwidth = false; }
  template <typename DimArray<int>::size_type DIM>
  void SetDefaultBandwidth (const double &h)
  { m_use_adaptive_bandwidth = true; std::get<DIM>(m_default_hs) = h; }
  void SetDefaultBandwidth (const typename DimArray<int>::size_type &dim, const double &h)
  { m_use_adaptive_bandwidth = true; m_default_hs[dim] = h; }

  template <typename DimArray<int>::size_type DIM>
  void SetVarianceFraction (const double &f)
  {
    if (f < 0.0) {
      Error("DensityEstimator::SetVarianceFraction", "Tried to set a negative variance fraction. Keeping it at %f.", std::get<DIM>(m_variance_fractions));
      return;
    }
    std::get<DIM>(m_variance_fractions) = f;
  }
  void SetVarianceFraction (const typename DimArray<int>::size_type &dim, const double &f)
  {
    if (f < 0.0) {
      Error("DensityEstimator::SetVarianceFraction", "Tried to set a negative variance fraction. Keeping it at %f.", m_variance_fractions[dim]);
      return;
    }
    m_variance_fractions[dim] = f;
  }
  template <typename DimArray<int>::size_type DIM>
  void SetRescaleRange (const double &l, const double &h)
  { std::get<DIM>(m_selection_bounds) = l; std::get<DIM>(m_selection_bounds) = h; }
  void SetRescaleRange (const typename DimArray<int>::size_type &dim, const double &l, const double &h)
  { m_selection_bounds[dim] = l; m_selection_bounds[dim] = h; }
  void ComputeCorrelations (const bool &flag = true) { m_full_correlations = flag; }
  void UseErrorPropagation () { m_error_type = ErrorEstimationType::Prop; }
  void UseJackknife () { m_error_type = ErrorEstimationType::Jackknife; }
  void UseBootstrap () { m_error_type = ErrorEstimationType::Bootstrap; }
  void UseNoEstimationError () { m_error_type = ErrorEstimationType::None; }
  template <typename DimArray<int>::size_type DIM>
  void SetEventFactor (const double &f) { std::get<DIM>(m_event_factors) = f; }
  void SetEventFactor (const typename DimArray<int>::size_type &dim, const double &f) { m_event_factors[dim] = f; }
  template <typename DimArray<int>::size_type DIM>
  void SetStandardKNN (const double &f)
  { std::get<DIM>(m_knn_factor_distances) = f; std::get<DIM>(m_knn_factor_settings) = KNNType::Standard; }
  void SetStandardKNN (const typename DimArray<int>::size_type &dim, const double &f)
  { m_knn_factor_distances[dim] = f; m_knn_factor_settings[dim] = KNNType::Standard; }
  template <typename DimArray<int>::size_type DIM>
  void SetLargeKNN (const double &f)
  { std::get<DIM>(m_knn_factor_distances) = f; std::get<DIM>(m_knn_factor_settings) = KNNType::Large; }
  void SetLargeKNN (const typename DimArray<int>::size_type &dim, const double &f)
  { m_knn_factor_distances[dim] = f; m_knn_factor_settings[dim] = KNNType::Large; }
  template <typename DimArray<int>::size_type DIM>
  void SetSmallKNN (const double &f)
  { std::get<DIM>(m_knn_factor_distances) = f; std::get<DIM>(m_knn_factor_settings) = KNNType::Small; }
  void SetSmallKNN (const typename DimArray<int>::size_type &dim, const double &f)
  { m_knn_factor_distances[dim] = f; m_knn_factor_settings[dim] = KNNType::Small; }

  void UseAdaptiveBandwidth (const bool &f = true)
  { m_use_adaptive_bandwidth = f; }

  template <typename DimArray<int>::size_type DIM>
  void SetScalingFunctionsAndDerivative (const TF1 &f, const TF1 &i, const TF1 &d)
  {
    std::get<DIM>(m_scaling_funcs) = f;
    std::get<DIM>(m_inverse_scaling_funcs) = i;
    std::get<DIM>(m_scaling_func_derivatives) = d;
  }
  void SetScalingFunctionsAndDerivative (const typename DimArray<int>::size_type &dim, const TF1 &f, const TF1 &i, const TF1 &d)
  {
    m_scaling_funcs[dim] = f;
    m_inverse_scaling_funcs[dim] = i;
    m_scaling_func_derivatives[dim] = d;
  }

  template <typename DimArray<int>::size_type DIM>
  void SetKernelFunction (const TF1 &f, const double &tau = 1.0)
  { std::get<DIM>(m_kernels) = f; std::get<DIM>(m_taus) = tau; }
  void SetKernelFunction (const typename DimArray<int>::size_type &dim, const TF1 &f, const double &tau = 1.0)
  { m_kernels[dim] = f; m_taus[dim] = tau; }

  TKernelDensityEstimatorResult CreateSmoothedHistogram (TH1 *binning_histogram = nullptr, bool rescale = true) const
  {
    TKernelDensityEstimatorResult _result;
    TH1 *result = nullptr;

    // transform coordinates
    // and construct grid (grid = g (not precomputed), with weights = c)
    int total_storage = 1;
    for (const auto &N : m_number_of_grid_points) total_storage *= N;
    decltype(m_ws) c, c_error;
    decltype(m_bounds) a, extremes;
    DimArray<double> deltas;
    for (unsigned i = 0; i < DIMENSION; ++i) {
      a[i].first = m_scaling_funcs[i](m_bounds[i].first);
      a[i].second = m_scaling_funcs[i](m_bounds[i].second);
      deltas[i] = (a[i].second - a[i].first)/static_cast<double>(m_number_of_grid_points[i]);
      extremes[i].first = 0.5*deltas[i] + a[i].first;
      extremes[i].second = a[i].second - 0.5*deltas[i];
    }

    if (m_normalization <= 0.0) {
      // TODO error message here
      BigVector<double> fs;
      BigVector<decltype(fs)> fs_cov;
      result = FillSmoothedHistogram(a, deltas, fs, fs_cov);
      _result.SmoothedHistogram = result;

      return _result;
    }

    std::tie(c, c_error) = ComputeGridWeightsAndErrors(a, extremes, deltas);
    double sum_weights = 0.0;
    for (auto &p : c) sum_weights += p.second;

    // compute normalization
    auto selection_normalization = GetSelectionNormalization();

    // compute vectors of h's, L's, and (properly normalized) kernel evaluations
    // dimension, grid point in that dimension, value
    DimArray<SparceCoordList<double>> hs;
    DimArray<SparceCoordList<int>> Ls;
    // dimension, grid point in that dimension, list of values
    DimArray<SparceCoordList<std::vector<double>>> ks;

    std::tie(hs, Ls, ks) = ComputeBandwidthsRangesAndKernelEvals(selection_normalization, c, c_error, deltas);

    auto compute_errors = (total_storage <= 1000); // TODO: make '1000' configurable
    // compute smoothed grid weights
    BigVector<double> fs(total_storage, 0.0);
    // NOTE: covariance matrix for fs
    // FIXME error matrix far too large
    BigVector<decltype(fs)> fs_cov;
    if (compute_errors)
      fs_cov = std::move(BigVector<decltype(fs)>(fs.size(), decltype(fs)(fs.size(), 0.0)));
    // // NOTE: errors on fs (diagonals of covariance matrix)
    // BigVector<double> fs_error(total_storage, 0.0);
    if (m_error_type == ErrorEstimationType::Prop && compute_errors)
      // FIXME error matrix far too large
      ComputeSmoothedValuesAndErrors(fs, fs_cov, Ls, ks, c, c_error, /*sum_weights,*/ extremes, deltas);
      // ComputeSmoothedValuesAndErrors(fs, fs_error, Ls, ks, c, c_error, extremes, deltas);
    else
      ComputeSmoothedValues(fs, Ls, ks, c, extremes, deltas);

    // proper error propagation for errors on estimation
    if (m_error_type == ErrorEstimationType::Prop && compute_errors) {
      Info("DensityEstimator::CreateSmoothedHistogram", "Computing error associated with estimation using full error propagation.");
    }
    else if (m_error_type == ErrorEstimationType::Jackknife && compute_errors) {
      // FIXME error matrix far too large
      ComputeJackknifeErrors(c, fs, fs_cov, Ls, ks, extremes, deltas);
      // ComputeJackknifeErrors(c, fs, fs_error, Ls, ks, extremes, deltas);
    }
    // TODO: do bootstrap for errors on estimation
    else if (m_error_type == ErrorEstimationType::Bootstrap && compute_errors) {
      // Info("DensityEstimator::CreateSmoothedHistogram", "Computing error associated with estimation using the bootstrap technique.");
    }
    else if (compute_errors)
      Info("DensityEstimator::CreateSmoothedHistogram", "Not computing error associated with estimation.");
    else
      Info("DensityEstimator::CreateSmoothedHistogram", "Not computing error associated with estimation due to computational limitations.");

    // FIXME error matrix far too large
    // construct histogram
    result = FillSmoothedHistogram(a, deltas, fs, fs_cov);
    // result = FillSmoothedHistogram(a, deltas, fs, fs_error);

    // create histogram with requested binning
    decltype(fs_cov) final_cov;
    // decltype(fs_error) final_error;

    if (binning_histogram != nullptr && !BinningEqual(binning_histogram, result)) {
      // TODO:  FIXME
      // final_cov = fs_cov;
      // final_error = fs_error;
      std::tie(result, final_cov) = RebinAndComputeErrors(result, binning_histogram, fs_cov, sum_weights);
    }
    else {
      final_cov = fs_cov;
      // final_error = fs_error;
    }

    // FIXME error matrix far too large
    if (compute_errors) _result.CorrelationHistogram = MakeCorrelationHistogram(final_cov);

    Info("DensityEstimator::CreateSmoothedHistogram", "Integral of smoothed histogram (should be close to %.3g): %.3g", sum_weights, result->Integral("width"));

    // do selection or full rescaling
    if (rescale) {
      result = RescaleHistogram(result, selection_normalization);
    }

    _result.SmoothedHistogram = result;

    return _result;
  }
};



void DrawHistogramBand (TLegend *leg, const char *leg_entry, TH1 *h, EColor mean, EColor error)
{
  std::string name(h->GetName());
  auto h_center = static_cast<TH1*>(h->Clone((name + "_center").c_str())),
       h_low_error = static_cast<TH1*>(h->Clone((name + "_low_error").c_str())),
       h_high_error = static_cast<TH1*>(h->Clone((name + "_high_error").c_str()));

  for (int i = 1; i <= h->GetXaxis()->GetNbins(); ++i) {
    h_center->SetBinError(i, 0.0);
    h_low_error->SetBinContent(i, h->GetBinContent(i) - h->GetBinError(i));
    h_low_error->SetBinError(i, 0.0);
    h_high_error->SetBinContent(i, h->GetBinContent(i) + h->GetBinError(i));
    h_high_error->SetBinError(i, 0.0);
  }
  h_center->SetLineColor(mean);
  h_center->SetLineWidth(2);
  h_center->Draw("hist same");
  if (error != kWhite) {
    h_low_error->SetLineColor(error);
    h_low_error->Draw("hist same");
    h_high_error->SetLineColor(error);
    h_high_error->Draw("hist same");
  }
  else {
    h_low_error->SetDirectory(0);
    delete h_low_error;
    h_high_error->SetDirectory(0);
    delete h_high_error;
  }
  leg->AddEntry(h_center, leg_entry, "L");
}

void DrawHistogramCircleE1 (TLegend *leg, const char *leg_entry, TH1 *h, EColor mean, EColor error)
{
  h->SetMarkerColor(mean);
  h->SetLineColor(error);
  h->SetMarkerStyle(kFullDotLarge);
  // h->SetMarkerStyle(kCircle);
  h->SetMarkerSize(0.5);
  h->Draw("e1 x0 same");
  leg->AddEntry(h, leg_entry, "PE");
}

double DetermineLowerBound (TH1 *raw_hist, bool absolute)
{
  double result = 0.0;

  for (int ibin = 1; ibin <= raw_hist->GetXaxis()->GetNbins(); ++ibin) {
    if (raw_hist->GetBinContent(ibin) > 0.0 || absolute) {
      result = raw_hist->GetBinLowEdge(ibin);
      break;
    }
  }

  return result;
}

double DetermineUpperBound (TH1 *raw_hist, bool absolute)
{
  double result = 0.0;

  if (absolute)
    result = raw_hist->GetXaxis()->GetBinUpEdge(raw_hist->GetXaxis()->GetNbins());
  else {
    for (int ibin = 1; ibin <= raw_hist->GetXaxis()->GetNbins(); ++ibin) {
      if (raw_hist->GetBinContent(ibin) > 0.0) {
        result = raw_hist->GetXaxis()->GetBinUpEdge(ibin);
      }
    }
  }

  return result;
}

unsigned DetermineNumberOfGridPoints (TH1 *raw_hist, double low, double high)
{
  auto low_bin = raw_hist->GetXaxis()->FindBin(low);
  auto high_bin = raw_hist->GetXaxis()->FindBin(high);
  unsigned result = high_bin - low_bin;

  return result;
}



std::pair<TH1*, TH2*> smooth_histo(TH1 *orighisto, const double &knn, const double &ef, const bool &low_full, const bool &high_full, const std::string &scale, const double &grid_factor, const bool &return_corr)
{
  std::pair<TH1*, TH2*> result(nullptr, nullptr);
  // TH1 *result = nullptr;
  TString orighisto_name(orighisto->GetName()),
          orighisto_title(orighisto->GetTitle());
  DensityEstimator<1> de;

  de.LoadData(orighisto);

  auto lower_bound = DetermineLowerBound(orighisto, low_full);
  auto upper_bound = DetermineUpperBound(orighisto, high_full);
  auto number_of_grid_points = DetermineNumberOfGridPoints(orighisto, lower_bound, upper_bound);

  // NOTE: default is linear scale, so I'm not checking for it explicitly here
  if (scale == "log")
    de.SetScalingFunctionsAndDerivative(0, TF1("scale", "TMath::Log(x)"),
                                           TF1("scale_inv", "TMath::Exp(x)"),
                                           TF1("scale_prime", "1.0/x"));

  de.SetLowerBound(0, lower_bound);
  de.SetUpperBound(0, upper_bound);
  de.SetNumberOfGridPoints(0, number_of_grid_points/grid_factor);
  de.SetStandardKNN(0, knn);
  de.SetEventFactor(0, ef);

  // de.UseJackknife();
  de.UseErrorPropagation();

  de.ComputeCorrelations(return_corr);

  auto smoothed_result = de.CreateSmoothedHistogram(orighisto);
  result.first = smoothed_result.SmoothedHistogram;

  if (result.first != nullptr) {
    orighisto->SetDirectory(0);
    delete orighisto;
    result.first->SetName(orighisto_name.Data());
    result.first->SetTitle(orighisto_title.Data());
  }
  if (smoothed_result.CorrelationHistogram != nullptr && !return_corr) {
    smoothed_result.CorrelationHistogram->SetDirectory(0);
    delete smoothed_result.CorrelationHistogram;
    smoothed_result.CorrelationHistogram = nullptr;
  }
  else if (smoothed_result.CorrelationHistogram != nullptr && return_corr)
    result.second = (TH2*)smoothed_result.CorrelationHistogram;

  return result;
}

bool check_for_NaNs_and_Infs (const TH1 *h)
{
  for (Int_t ibin = 1; ibin <= h->GetNbinsX(); ++ibin) {
    if (std::isnan(h->GetBinContent(ibin)) || std::isinf(h->GetBinContent(ibin))) return true;
    if (std::isnan(h->GetBinErrorUp(ibin)) || std::isinf(h->GetBinErrorUp(ibin))) return true;
    if (std::isnan(h->GetBinErrorLow(ibin)) || std::isinf(h->GetBinErrorLow(ibin))) return true;
  }
  return false;
}
