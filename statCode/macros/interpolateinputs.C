#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include <TCanvas.h>
#include <TFile.h>
#include <TGraph.h>
#include <TH1.h>
#include <TKey.h>
#include <TMath.h>
#include <TF1.h>
#include <TDirectory.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TRegexp.h>
#include <RooMsgService.h>

void compute_initial_bin_error_graphs (std::vector<std::pair<TGraph*, TGraph*>> &bin_errors,
                                       const std::vector<std::pair<double, TH1*>> &mass_histo_pairs,
                                       const Double_t *masses);

void compute_interpolated_errors (const std::vector<std::pair<TGraph*, TGraph*>> &bin_errors,
                                  TH1 *h,
                                  const double &mass_to_interpolate);

bool check_for_NaNs_and_Infs (const TH1 *h);

bool check_for_all_zeros (const TH1 *h);

bool file_exists (const char *filename)
{
  std::ifstream ifile(filename);
  return ifile;
}

void interpolateinputs ()
{
  using namespace std;
  // NOTE Remove stream for data handling and integration
  RooMsgService::instance().getStream(1).removeTopic(RooFit::MsgTopic::DataHandling);
  RooMsgService::instance().getStream(1).removeTopic(RooFit::MsgTopic::InputArguments);
  RooMsgService::instance().getStream(1).removeTopic(RooFit::MsgTopic::NumIntegration);
  RooMsgService::instance().getStream(0).removeTopic(RooFit::MsgTopic::Plotting);
  RooMsgService::instance().getStream(1).removeTopic(RooFit::MsgTopic::Plotting);
  // NOTE Set global message level
  gErrorIgnoreLevel = kInfo;
  // gErrorIgnoreLevel = kWarning;
  // NOTE Set global debug level
  gDebug = 0; // >0 debug, >5 verbose
  TH1::SetDefaultSumw2(kTRUE);

  vector<string>                               signal_prefixes = {"AZhllbb", "AZhvvbb"};
  // NOTE only interpolate for 1 & 2 tag regions
  const vector<pair<TRegexp, vector<double>>>  masses_to_interpolate = {make_pair("13TeV_TwoLepton_[1-2]+p?tag[0-9]+p?jet_.*_.*ptv_[a-zA-Z]*_mVH.*:AZhllbb",
                                                                                  vector<double>{/*230., 250.,*/ 270., 290.,
                                                                                                 310., 330., 350., 370., 390.,
                                                                                                 410., 430., 450., 470., 490.}),
                                                                        make_pair("13TeV_ZeroLepton_[1-2]+p?tag[0-9]+p?jet_.*_.*ptv_[a-zA-Z]*_mVH.*:AZhvvbb",
                                                                                  vector<double>{310., 330., 350., 370., 390.,
                                                                                                 410., 430., 450., 470., 490.})
                                                                        };
  const vector<double>                         masses_to_ignore = {200.};
  TString                                      src_dir("/afs/cern.ch/user/c/cpandini/public/AZhInputs/21018");
  TString                                      new_directory = "/afs/cern.ch/work/j/jhetherl/public/interpolated_inputs/21018";
  TString                                      plots_directory = new_directory + "/plots";
  std::vector<TString> files_to_interpolate;

  TSystemDirectory dir(src_dir.Data(), src_dir.Data());
  TList *files = dir.GetListOfFiles();

  if (files) {
    TSystemFile *file;
    TString fname;
    TString extension(".root");
    TIter next(files);
    while ((file = (TSystemFile*)next())) {
      fname = file->GetName();
      if (!file->IsDirectory() && fname.EndsWith(extension.Data())) {
        files_to_interpolate.push_back(fname);
      }
    }
  }

  gSystem->mkdir(new_directory.Data(), kTRUE);
  gSystem->mkdir(plots_directory.Data(), kTRUE);

  for (size_t j = 0; j < files_to_interpolate.size(); ++j) {
    // // FIXME JWH testing
    // if (//files_to_interpolate[j] != "13TeV_ZeroLepton_2tag2pjet_150_500ptv_SR_mVH.root" &&
    //     files_to_interpolate[j] != "13TeV_TwoLepton_2tag2pjet_0_500ptv_SR_mVH.root") continue;
    TString original_full_file = src_dir + "/" + files_to_interpolate[j],
            destination_file = new_directory + "/" + files_to_interpolate[j],
            plots_file = plots_directory + "/" + files_to_interpolate[j];
    TFile* _file0 = nullptr;
    TFile* _file1 = nullptr;
    TFile* _file2 = nullptr;

    TString original_file = original_full_file(original_full_file.Last('/') + 1, original_full_file.Length() - (original_full_file.Last('/') + 1));
    TString new_full_file = new_directory + "/" + original_file;
    TString plots_full_file = plots_directory + "/" + original_file;
    TString syst_dir_name;
    if (file_exists(new_full_file.Data())) {
      Info("interpolateinputs", "Found preexisting file %s. Will not recreate.", new_full_file.Data());
      continue;
    }
    Info("interpolateinputs", "Creating inputs file %s", new_full_file.Data());
    _file0 =  TFile::Open(original_full_file.Data());
    _file1 =  TFile::Open(new_full_file.Data(), "RECREATE");
    // NOTE signal prefix, syst. tag ("" for nominal), mass, histogram
    unordered_map<string, unordered_map<string, vector<pair<double, TH1*>>>>  signal_mass_histo_pairs;

    TIter nextkey(_file0->GetListOfKeys());
    TKey *key;
    while ( (key = (TKey*)nextkey()) ) {

      TString keyname = key->GetName();
      if(keyname.Contains("Sys")) {
        syst_dir_name = keyname;
        TDirectory *local_dir = (TDirectory*)key->ReadObj();
        //Systematic list
        TString dirname = local_dir->GetName();
        _file1->cd();
        _file1->mkdir(dirname,dirname);
        // _file1->cd(dirname);
        TIter nextkey2(local_dir->GetListOfKeys());
        TKey *key2;
        while ( (key2 = (TKey*)nextkey2()) ) {
          TH1 *h = (TH1F*)key2->ReadObj();
          TString origname = key2->GetName();
          // // FIXME JWH testing
          // if (!origname.Contains("FT_EFF_Eigen_B_0_AntiKt4EMTopoJets__1up")) continue;
          for (auto &signal_prefix : signal_prefixes) {
            if(origname.BeginsWith(signal_prefix.c_str())) {
              if (check_for_NaNs_and_Infs(h)) {
                Warning("interpolateinputs", "%s:%s/%s contains a NaN or Inf. Will not include this histogram in the interpolation...", original_full_file.Data(), syst_dir_name.Data(), h->GetName());
                continue;
              }
              if (check_for_all_zeros(h)) {
                Warning("interpolateinputs", "%s:%s/%s contains all zeros. Will not include this histogram in the interpolation...", original_full_file.Data(), syst_dir_name.Data(), h->GetName());
                continue;
              }
              if (gDebug > 0) Info("interpolateinputs", "\tFound systematic named %s/%s", dirname.Data(), origname.Data());
              auto syst_name = origname;
              syst_name.Remove(0, syst_name.First('_'));
              auto mass_name = origname;
              mass_name.Remove(mass_name.First('_'), mass_name.Length());
              mass_name.ReplaceAll(signal_prefix.c_str(), "");
              auto mass = stod(mass_name.Data());
              if (!any_of(masses_to_ignore.begin(), masses_to_ignore.end(), [&mass](const double &s){return s==mass;}))
                signal_mass_histo_pairs[signal_prefix][syst_name.Data()].emplace_back(make_pair(mass, h));
            }
          }

          _file1->cd(dirname);
          h->Write(origname);
          _file0->cd();
        }
      } else {
        TH1 *h = (TH1*)key->ReadObj();
        TString origname = key->GetName();
        for (auto &signal_prefix : signal_prefixes) {
          if(origname.BeginsWith(signal_prefix.c_str())) {
            if (check_for_NaNs_and_Infs(h)) {
              Warning("interpolateinputs", "%s:%s contains a NaN or Inf. Will not include this histogram in the interpolation...", original_full_file.Data(), h->GetName());
              continue;
            }
            if (check_for_all_zeros(h)) {
              Warning("interpolateinputs", "%s:%s contains all zeros. Will not include this histogram in the interpolation...", original_full_file.Data(), h->GetName());
              continue;
            }
            if (gDebug > 0) Info("interpolateinputs", "\tFound nominal named %s", origname.Data());
            auto mass_name = origname;
            mass_name.ReplaceAll(signal_prefix.c_str(), "");
            auto mass = stod(mass_name.Data());
            if (!any_of(masses_to_ignore.begin(), masses_to_ignore.end(), [&mass](const double &s){return s==mass;})) {
              signal_mass_histo_pairs[signal_prefix][""].emplace_back(make_pair(mass, h));
            }
          }
        }

        _file1->cd();
        h->Write(origname);
        _file0->cd();
      }
    }

    // NOTE do all interpolation here
    if (signal_mass_histo_pairs.size() <= 0) {
      Warning("interpolateinputs", "No mass points found. Skipping file!");
      continue;
    }

    bool was_interpolated = false;
    for (auto &pp : signal_mass_histo_pairs) {
      auto signal_prefix = pp.first;
      auto &mass_histo_pairs = pp.second;
      int interpolate_masses_index = -1;

      for (int index = 0; static_cast<unsigned>(index) < masses_to_interpolate.size(); ++index)
        if (TString(Form("%s:%s", files_to_interpolate[j].Data(), signal_prefix.c_str())).Contains(masses_to_interpolate[index].first))
          interpolate_masses_index = index;
      if (interpolate_masses_index == -1) continue;
      was_interpolated = true;
      auto &masses_to_interp = masses_to_interpolate[interpolate_masses_index].second;
      Info("interpolateinputs", "Interpolating signal %s", signal_prefix.c_str());

      for (auto &p : mass_histo_pairs) {
        auto tag = p.first;
        auto pairs = p.second;

        // NOTE sort by mass points
        sort(pairs.begin(), pairs.end(), [](const pair<double, TH1*> &i, const pair<double, TH1*> &j){return (i.first < j.first);});
        auto template_mass_lowest = pairs[0].first,
             template_mass_highest = pairs.back().first;

        // NOTE determine bin boundaries
        auto bin_boundaries = new Double_t[pairs[0].second->GetNbinsX() + 1];
        for (unsigned i = 0; i <= pairs[0].second->GetNbinsX(); ++i)
          bin_boundaries[i] = pairs[0].second->GetXaxis()->GetBinLowEdge(i);
        RooBinning binning(pairs[0].second->GetNbinsX(), bin_boundaries);

        // NOTE graph for interpolating the normalization
        auto masses = new Double_t[pairs.size()],
             norms = new Double_t[pairs.size()];
        for (unsigned i = 0; i < pairs.size(); ++i) {
          masses[i] = pairs[i].first;
          norms[i] = pairs[i].second->Integral();
        }
        TGraph normalizations(pairs.size(), masses, norms);
        // normalizations.SetBit(TGraph::kIsSortedX);
        delete []norms;

        // NOTE graphs for interpolating the bin errors (left/right of morphing parameter)
        vector<pair<TGraph*, TGraph*>> bin_errors;
        compute_initial_bin_error_graphs(bin_errors, pairs, masses);
        delete []masses;

        RooRealVar m("mass", "m_{Vh}", pairs[0].second->GetXaxis()->GetBinLowEdge(1),
                                       pairs[0].second->GetXaxis()->GetBinUpEdge(pairs[0].second->GetXaxis()->GetNbins()));
        RooRealVar a("morph_mass", "m_{Vh}^{morph}", masses_to_interp[0], masses_to_interp.back());
        TVectorD m_i(pairs.size());
        RooArgList pdf_list;
        vector<RooDataHist*> dataHists;
        vector<RooHistPdf*> histPdfs;
        for (unsigned i = 0; i < pairs.size(); ++i) {
          m_i[i] = pairs[i].first;
          // NOTE the file name within the TNamed here is b/c ROOT/RooFit can't handle multiple files otherwise
          auto rh_name = Form("rh%d_%s", int(m_i[i]), files_to_interpolate[j].Data());
          auto rhPDF_name = Form("rhPDF%d_%s", int(m_i[i]), files_to_interpolate[j].Data());
          auto rh = new RooDataHist(rh_name, rh_name, m, pairs[i].second);
          auto rhPDF = new RooHistPdf(rhPDF_name, rhPDF_name, m, *rh);
          pdf_list.add(*rhPDF);
          dataHists.push_back(rh);
          histPdfs.push_back(rhPDF);
        }

        auto setting = RooMomentMorph::Linear;
        // auto setting = RooMomentMorph::NonLinear;
        // auto setting = RooMomentMorph::NonLinearLinFractions;
        // auto setting = RooMomentMorph::NonLinearPosFractions;
        // auto setting = RooMomentMorph::SineLinear;
        RooMomentMorph morph("morph", "morph", a, RooArgList(m), pdf_list, m_i, setting);

        for (unsigned i = 0; i < masses_to_interp.size(); ++i) {
          if (gDebug > 5)
            Info("interpolateinputs", "\tProcessing interpolated histogram %s", Form("%s%d%s", signal_prefix.c_str(), int(masses_to_interp[i]), tag.c_str()));
          TH1 *h = nullptr;
          TString h_name;
          if (template_mass_lowest > masses_to_interp[i] || template_mass_highest < masses_to_interp[i]) {
            h_name = Form("%s%d%s", signal_prefix.c_str(), int(masses_to_interp[i]), tag.c_str());
            h = (TH1*)pairs[0].second->Clone();
            TString h_title(pairs[0].second->GetTitle());
            h->SetName(h_name.Data());
            h_title.Remove(0, h_title.First('_'));
            h_title = Form("%s%d%s", signal_prefix.c_str(), int(masses_to_interp[i]), h_title.Data());
            h->SetTitle(h_title.Data());
            h->GetXaxis()->SetTitle(pairs[0].second->GetXaxis()->GetTitle());
            h->GetYaxis()->SetTitle(pairs[0].second->GetYaxis()->GetTitle());
            for (Int_t ibin = 1; ibin <= h->GetNbinsX(); ++ibin) {
              h->GetBinContent(ibin, 0.0);
              h->GetBinError(ibin, 0.0);
            }
          }
          else {
            a.setVal(masses_to_interp[i]);
            h = morph.createHistogram(Form("%s%d%s", signal_prefix.c_str(), int(masses_to_interp[i]), tag.c_str()), m, RooFit::Binning(binning));
            h_name = h->GetName();
            TString h_title(pairs[0].second->GetTitle());
            h_name.ReplaceAll(Form("__%s", m.GetName()), "");
            h->SetName(h_name.Data());
            h_title.Remove(0, h_title.First('_'));
            h_title = Form("%s%d%s", signal_prefix.c_str(), int(masses_to_interp[i]), h_title.Data());
            h->SetTitle(h_title.Data());
            h->GetXaxis()->SetTitle(pairs[0].second->GetXaxis()->GetTitle());
            h->GetYaxis()->SetTitle(pairs[0].second->GetYaxis()->GetTitle());
            h->Scale(normalizations.Eval(masses_to_interp[i], 0, "S"));
            compute_interpolated_errors(bin_errors, h, masses_to_interp[i]);
          }
          _file1->cd();
          if (tag.size() > 0) {
            _file1->cd(syst_dir_name);
            if (gDebug > 0)
              Info("interpolateinputs", "\tWriting interpolated histogram %s/%s", syst_dir_name.Data(), h_name.Data());
          }
          else {
            _file1->cd();
            if (gDebug > 0)
              Info("interpolateinputs", "\tWriting interpolated histogram %s", h_name.Data());
          }
          h->Write(h_name);
          h->SetDirectory(0);
          delete h;
        }

        for (auto &p : histPdfs) delete p;
        for (auto &p : dataHists) delete p;
        for (auto &p : bin_errors) {delete p.first; delete p.second;}
        delete []bin_boundaries;
      }
    }

    _file1->cd();

    _file1->Close();
    _file0->Close();

    delete _file1;
    delete _file0;

    // for (auto &p1 : signal_mass_histo_pairs)
    //   for (auto &p2 : p1.second)
    //     for (auto &p3 : p2.second) {
    //       p3.second->SetDirectory(0);
    //       delete p3.second;
    //     }

    signal_mass_histo_pairs.clear();

    // NOTE all plotting here
    if (was_interpolated) {
      if (gDebug > 0) Info("interpolateinputs", "\tCreating plots...");
      _file1 = TFile::Open(new_full_file.Data());
      _file2 = TFile::Open(plots_full_file.Data(), "RECREATE");
      nextkey = _file1->GetListOfKeys();
      while ( (key = (TKey*)nextkey()) ) {
        TString keyname = key->GetName();
        if(keyname.Contains("Sys")) {
          TDirectory *local_dir = (TDirectory*)key->ReadObj();
          //Systematic list
          TString dirname = local_dir->GetName();
          _file2->cd();
          _file2->mkdir(dirname, dirname);
          TIter nextkey2(local_dir->GetListOfKeys());
          TKey *key2;
          while ( (key2 = (TKey*)nextkey2()) ) {
            TH1 *h = (TH1F*)key2->ReadObj();
            TString origname = key2->GetName();
            for (auto &signal_prefix : signal_prefixes) {
              if(origname.BeginsWith(signal_prefix.c_str())) {
                if (check_for_NaNs_and_Infs(h)) continue;
                if (check_for_all_zeros(h)) continue;
                auto syst_name = origname;
                syst_name.Remove(0, syst_name.First('_'));
                auto mass_name = origname;
                mass_name.Remove(mass_name.First('_'), mass_name.Length());
                mass_name.ReplaceAll(signal_prefix.c_str(), "");
                auto mass = stod(mass_name.Data());
                if (!any_of(masses_to_ignore.begin(), masses_to_ignore.end(), [&mass](const double &s){return s==mass;}))
                  signal_mass_histo_pairs[signal_prefix][syst_name.Data()].emplace_back(make_pair(mass, h));
              }
            }
          }
        } else {
          TH1 *h = (TH1*)key->ReadObj();
          TString origname = key->GetName();
          for (auto &signal_prefix : signal_prefixes) {
            if(origname.BeginsWith(signal_prefix.c_str())) {
              if (check_for_NaNs_and_Infs(h)) continue;
              if (check_for_all_zeros(h)) continue;
              auto mass_name = origname;
              mass_name.ReplaceAll(signal_prefix.c_str(), "");
              auto mass = stod(mass_name.Data());
              if (!any_of(masses_to_ignore.begin(), masses_to_ignore.end(), [&mass](const double &s){return s==mass;})) {
                signal_mass_histo_pairs[signal_prefix][""].emplace_back(make_pair(mass, h));
              }
            }
          }
        }
      }
      for (auto &pp : signal_mass_histo_pairs) {
        auto signal_prefix = pp.first;
        auto &mass_histo_pairs = pp.second;
        int interpolate_masses_index = -1;

        for (int index = 0; static_cast<unsigned>(index) < masses_to_interpolate.size(); ++index)
          if (TString(Form("%s:%s", files_to_interpolate[j].Data(), signal_prefix.c_str())).Contains(masses_to_interpolate[index].first))
            interpolate_masses_index = index;
        if (interpolate_masses_index == -1) continue;
        auto &masses_to_interp = masses_to_interpolate[interpolate_masses_index].second;

        for (auto &p : mass_histo_pairs) {
          auto tag = p.first;
          auto &pairs = p.second;

          // NOTE sort by mass points
          sort(pairs.begin(), pairs.end(), [](const pair<double, TH1*> &i, const pair<double, TH1*> &j){return (i.first < j.first);});

          auto canvas_hist = new TCanvas(Form("%s%s_signal_histograms", signal_prefix.c_str(), tag.c_str()), "Signal Interpolation", 600, 600);
          auto canvas_pdf = new TCanvas(Form("%s%s_signal_pdfs", signal_prefix.c_str(), tag.c_str()), "Signal Interpolation", 600, 600);
          bool is_first = true;
          for (auto &ps : pairs) {
            auto mass = ps.first;
            auto h = ps.second;

            bool is_interpolated = any_of(masses_to_interp.begin(), masses_to_interp.end(), [&mass](const double &m){return mass==m;});

            h->SetLineColor(is_interpolated ? kRed : kBlack);
            h->SetMarkerColor(is_interpolated ? kRed : kBlack);
            h->SetMarkerStyle(20);
            h->SetMarkerSize(is_interpolated ? 0.75 : 0.5);

            canvas_hist->cd();
            if (is_first) h->Draw("E1X0");
            else h->Draw("E1X0 same");

            auto h_pdf = static_cast<TH1*>(h->Clone());
            h_pdf->Scale(1.0/h_pdf->Integral("width"));
            h_pdf->SetLineWidth(is_interpolated ? 2 : 1);
            canvas_pdf->cd();
            if (is_first) h_pdf->Draw("hist");
            else h_pdf->Draw("hist same");

            is_first = false;
          }
          if (tag.size() > 0) _file2->cd(syst_dir_name);
          else _file2->cd();
          canvas_hist->Write();
          canvas_pdf->Write();

          gSystem->ProcessEvents();
          delete canvas_hist;
          delete canvas_pdf;
        }
      }

      _file1->Close();
      _file2->Close();

      delete _file1;
      delete _file2;

      // for (auto &p1 : signal_mass_histo_pairs)
      //   for (auto &p2 : p1.second)
      //     for (auto &p3 : p2.second) {
      //       p3.second->SetDirectory(0);
      //       delete p3.second;
      //     }

      signal_mass_histo_pairs.clear();
    }
  }
}

void compute_initial_bin_error_graphs(std::vector<std::pair<TGraph*, TGraph*>> &bin_errors,
                                      const std::vector<std::pair<double, TH1*>> &mass_histo_pairs,
                                      const Double_t *masses)
{
  // NOTE index convention: 2i = up, 2i + 1 = down
  // NOTE for center bin only fill left
  using namespace std;

  int bin_counter = 0;
  auto  left_exit = false,
        right_exit = false;

  while (!left_exit || !right_exit) {
    left_exit = true;
    right_exit = true;
    auto left_up = new Double_t[mass_histo_pairs.size()],
         left_down = new Double_t[mass_histo_pairs.size()],
         right_up = new Double_t[mass_histo_pairs.size()],
         right_down = new Double_t[mass_histo_pairs.size()];
    unsigned imass_bin = 0;
    for (const auto &p : mass_histo_pairs) {
      auto mass = p.first;
      auto histo = p.second;
      auto center_bin = histo->FindBin(mass);
      if (bin_counter == 0) {
        left_up[imass_bin] = histo->GetBinErrorUp(center_bin);
        left_down[imass_bin] = histo->GetBinErrorLow(center_bin);
        left_exit = false;
        right_exit = false;
      }
      else {
        auto left_bin = center_bin - bin_counter;
        auto right_bin = center_bin + bin_counter;
        if (left_bin >= 1) {
          left_up[imass_bin] = histo->GetBinErrorUp(left_bin);
          left_down[imass_bin] = histo->GetBinErrorLow(left_bin);
          left_exit = false;
        }
        else {
          left_up[imass_bin] = 0.0;
          left_down[imass_bin] = 0.0;
        }
        if (right_bin <= histo->GetNbinsX()) {
          right_up[imass_bin] = histo->GetBinErrorUp(right_bin);
          right_down[imass_bin] = histo->GetBinErrorLow(right_bin);
          right_exit = false;
        }
        else {
          right_up[imass_bin] = 0.0;
          right_down[imass_bin] = 0.0;
        }
      }
      ++imass_bin;
    }

    if (bin_counter == 0) {
      auto  left_up_graph = new TGraph(mass_histo_pairs.size(), masses, left_up),
            left_down_graph = new TGraph(mass_histo_pairs.size(), masses, left_down);
      // left_up_graph->SetBit(TGraph::kIsSortedX);
      // left_down_graph->SetBit(TGraph::kIsSortedX);
      bin_errors.push_back(make_pair(left_up_graph, nullptr));
      bin_errors.push_back(make_pair(left_down_graph, nullptr));
    }
    else if (!left_exit || !right_exit) {
      auto  left_up_graph = new TGraph(mass_histo_pairs.size(), masses, left_up),
            left_down_graph = new TGraph(mass_histo_pairs.size(), masses, left_down),
            right_up_graph = new TGraph(mass_histo_pairs.size(), masses, right_up),
            right_down_graph = new TGraph(mass_histo_pairs.size(), masses, right_down);
      // left_up_graph->SetBit(TGraph::kIsSortedX);
      // left_down_graph->SetBit(TGraph::kIsSortedX);
      // right_up_graph->SetBit(TGraph::kIsSortedX);
      // right_down_graph->SetBit(TGraph::kIsSortedX);
      bin_errors.push_back(make_pair(left_up_graph, right_up_graph));
      bin_errors.push_back(make_pair(left_down_graph, right_down_graph));
    }

    delete []left_up;
    delete []left_down;
    delete []right_up;
    delete []right_down;

    ++bin_counter;
  }
}

void compute_interpolated_errors(const std::vector<std::pair<TGraph*, TGraph*>> &bin_errors,
                                 TH1 *h,
                                 const double &mass_to_interpolate)
{
  // NOTE index convention: 2i = up, 2i + 1 = down
  // NOTE for center bin only left graph is relevant
  using namespace std;
  // static TSpline3 spline;

  int bin_counter = 0;
  auto  left_exit = false,
        right_exit = false;
  auto center_bin = h->FindBin(mass_to_interpolate);

  while (!left_exit || !right_exit) {
    left_exit = true;
    right_exit = true;
    auto up_error_graphs = bin_errors[2*bin_counter];
    auto down_error_graphs = bin_errors[2*bin_counter + 1];

    if (bin_counter == 0) {
      auto up_error_graph = up_error_graphs.first;
      auto down_error_graph = down_error_graphs.first;

      h->SetBinError(center_bin, 0.5*(up_error_graph->Eval(mass_to_interpolate, 0, "S") + down_error_graph->Eval(mass_to_interpolate, 0, "S")));
      // h->SetBinError(center_bin, 0.5*(up_error_graph->Eval(mass_to_interpolate, &spline, "S") + down_error_graph->Eval(mass_to_interpolate, &spline, "S")));

      left_exit = false;
      right_exit = false;
    }
    else {
      auto up_error_graph_left = up_error_graphs.first;
      auto down_error_graph_left = down_error_graphs.first;
      auto up_error_graph_right = up_error_graphs.second;
      auto down_error_graph_right = down_error_graphs.second;
      auto left_bin = center_bin - bin_counter;
      auto right_bin = center_bin + bin_counter;
      if (left_bin >= 1) {
        h->SetBinError(left_bin, 0.5*(up_error_graph_left->Eval(mass_to_interpolate, 0, "S") + down_error_graph_left->Eval(mass_to_interpolate, 0, "S")));
        // h->SetBinError(left_bin, 0.5*(up_error_graph_left->Eval(mass_to_interpolate, &spline, "S") + down_error_graph_left->Eval(mass_to_interpolate, &spline, "S")));
        left_exit = false;
      }
      if (right_bin <= h->GetNbinsX()) {
        h->SetBinError(right_bin, 0.5*(up_error_graph_right->Eval(mass_to_interpolate, 0, "S") + down_error_graph_right->Eval(mass_to_interpolate, 0, "S")));
        // h->SetBinError(right_bin, 0.5*(up_error_graph_right->Eval(mass_to_interpolate, &spline, "S") + down_error_graph_right->Eval(mass_to_interpolate, &spline, "S")));
        right_exit = false;
      }
    }

    ++bin_counter;
  }
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

bool check_for_all_zeros (const TH1 *h)
{
  for (Int_t ibin = 1; ibin <= h->GetNbinsX(); ++ibin) {
    if (!TMath::AreEqualRel(h->GetBinContent(ibin), 0.0, 1.0e-12)) return false;
  }
  return true;
}
