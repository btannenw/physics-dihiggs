// Author      : Stefan Gadatsch
// Email       : gadatsch@nikhef.nl
// Date        : 2013-04-13
// Description : Draw pulls of nuisance parameters, rank by importance

#include "TCanvas.h"
#include "TH2F.h"
#include "TGraphAsymmErrors.h"
#include "TLatex.h"
#include "TLine.h"
#include "TGaxis.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TSystemFile.h"
#include "TStyle.h"
#include "TString.h"
#include "TROOT.h"
#include "drawPlot.C"

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

// global style options
TString energy1 = "8";
TString energy2 = "13";
TString lumi1 = "20.3";
TString lumi2 = "13.2";
bool doHorizontal          = false; // produce a horizontal plot
bool drawInset             = false; // will cover legend but show the normalisation factors which are a priori unconstrained
bool drawErrorBars         = false; // draw bars visualising the total, stat and syst uncertainty
//bool drawErrorBars         = true; // draw bars visualising the total, stat and syst uncertainty
bool drawHatchedBands      = false; // draw hatched bands around delta muhat = 0 to visualise the total, stat and syst uncertainty
bool drawParamNames        = true;  // show the nuisance parameter labels
bool drawPostfitImpactBand = true;  // && (mode != error); // draw a band with the variation of muhat due to each theta (postfit uncertainty)
bool drawPrefitImpactBand  = false;  // && (mode != error); // draw a band with the variation of muhat due to each theta (prefit uncertainty)
bool drawStandardBand      = false;  // draw 1 sigma standard deviation bar
bool useRelativeImpact     = false;  // switch to delta muhat / delta muhat tot for the top axis
int useBreakdown           = 0;     // 0 = add, 1 = sub
double scale_poi           = 2.0;  // zoom the impact axis
double scale_theta         = 1.75;  // zoom the pull axis; 3.4 for 1lep...1.7 for everybody else
//double scale_poi           = 1.25;  // zoom the impact axis
//double scale_theta         = 2.20;  // zoom the pull axis
bool removeHbb             = false; // remove Hbb parameters from the plot
bool removeHtt             = false; // remove Htt parameters from the plot
//int showTopParameters      = 25;    // -1 to show all parameters
int showTopParameters      = -1;    // -1 to show all parameters
double showHighImpact      = 0.0;   // sigma_comp / sigma_tot threshold
double UserlabelSize       = 0.03; // label size
double hatch_width         = 2.5; // postfit hatch width
Color_t color_standardband    = kRed-4;
Color_t color_standardband_ol = kBlue-4;
Color_t color_totalerror   = kBlue-4;
Color_t color_staterror    = kGreen+1;
Color_t color_systerror    = kMagenta-4;
Color_t color_pulls        = kGray+2;
Color_t color_normalization = kRed-4;
Color_t color_prefit       = kYellow-7;
Color_t color_prefit_ol    = kGreen-8;
Color_t color_postfit      = kBlue-4;
Color_t color_postfit_ol   = kYellow-7;
bool rankNuis              = true; // sort the nuisance parameters by impact on POI
bool m_postFitOrder          = true;

void drawPlot_pulls2(string cardName, string mass, TCanvas* c1, TPad* pad1, TPad* pad2);
void ROOT2Ascii(string folder);
void loadFile(const char* fileName, int cols, fileHolder file);
vector<string> getLabel(const char* fileName, int nrPars);
TString translateNPname(TString internalName, bool isMVA);
void drawPlot_pulls(string mass = "125", string cardName = "", bool remakeAscii = 0, string overlayCard="", bool postFitOrder = true) {
  gStyle->SetHatchesLineWidth(hatch_width);

  std::cout<<"^^^ a"<<std::endl;
    m_postFitOrder = postFitOrder;
    vector<string> parsed = parseString(cardName, ":");
    string cardOpts;
    if (parsed.size() > 1) {
        cardOpts = parsed[1];
    }
    cardName = parsed[0];
    computeFlags(cardName);
    std::cout<<"^^^ b"<<std::endl;

    applyOverlay(overlayCard , overlay , "");
    std::cout<<"c"<<std::endl;
    if (remakeAscii) {
      ROOT2Ascii(parsed[0]+"_pulls");
      ROOT2Ascii(parsed[0]+"_breakdown_add");

        if (overlay != "") {
            ROOT2Ascii(overlay+"_pulls");
            ROOT2Ascii(overlay+"_breakdown_add");
        }
    }
    std::cout<<"^^^ c"<<std::endl;
    showLabel = 1;

    TCanvas* c1 = new TCanvas("c1","c1",1024,1448);

    TPad *pad1 = new TPad("pad1", "pad1", 0.0  , 0.0  , 1.0 , 1.0  , 0);
    TPad *pad2 = new TPad("pad2", "pad2", 0.63, 0.1, 0.94, 0.22, 0);
    
    std::cout<<"^^^ d"<<std::endl;
    if (drawParamNames) pad1->SetLeftMargin(0.30);//Graph size
    else pad1->SetLeftMargin(0.05);
    pad1->SetRightMargin(0.05);
    pad1->SetBottomMargin(0.09);
    if (drawErrorBars) pad1->SetTopMargin(0.10);
    else pad1->SetTopMargin(0.09);
    std::cout<<"^^^ e"<<std::endl;
    pad2->SetLeftMargin(0.325);
    pad2->SetRightMargin(0.01);

    pad1->Draw();
    if (drawInset) pad2->Draw();
    std::cout<<"^^^ f"<<std::endl;
    ydiff_leg = 0.15;

    labelPosX = 0.06;
    channelPosX = 0.33;
    channelPosY = 0.19;
  std::cout<<"^^^ g"<<std::endl;
    if (dolvlv) {
        markerSize = 0.8;

        minMass = 0;
        maxMass = 500;

        drawPlot_pulls2(cardName, mass, c1, pad1, pad2);

        pad1->cd();

        TLatex t;
        t.SetTextSize(0.03);
        t.SetNDC();
    
        if (cardName.find("_ee_") != string::npos)          t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu");
        else if (cardName.find("_em_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu");
        else if (cardName.find("_me_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nue#nu");
        else if (cardName.find("_mm_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nu#mu#nu");
        else if (cardName.find("_0j_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu (0 jets)");
        else if (cardName.find("_1j_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu (1 jet)");
        else if (cardName.find("_2j_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu (2 jets)");
        else if (cardName.find("_BK_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu B-K");
        else if (cardName.find("_LM_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu L-M");
        else if (cardName.find("_ee0j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu (0 jets)");
        else if (cardName.find("_em0j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu (0 jets)");
        else if (cardName.find("_mm0j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nu#mu#nu (0 jets)");
        else if (cardName.find("_me0j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nue#nu (0 jets)");
        else if (cardName.find("_ee1j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu (1 jet)");
        else if (cardName.find("_em1j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu (1 jet)");
        else if (cardName.find("_me1j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nue#nu (1 jet)");
        else if (cardName.find("_mm1j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nu#mu#nu (1 jet)");
        else if (cardName.find("_ee01j_")  != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu (0/1 jets)");
        else if (cardName.find("_em01j_")  != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu (0/1 jets)");
        else if (cardName.find("_mm01j_")  != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nu#mu#nu (0/1 jets)");
        else if (cardName.find("_me01j_")  != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nue#nu (0/1jets)");
        else if (cardName.find("_ee2j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu (2 jets)");
        else if (cardName.find("_em2j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu (2 jets)");
        else if (cardName.find("_me2j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nue#nu (2 jets)");
        else if (cardName.find("_mm2j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrow#mu#nu#mu#nu (2 jets)");
        else if (cardName.find("_em0jBK_") != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu BK (0 jets)");
        else if (cardName.find("_em0jLM_") != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu LM (0 jets)");
        else if (cardName.find("_em1jBK_") != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu BK (1 jet)");
        else if (cardName.find("_em1jLM_") != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu LM (1 jet)");
        else if (cardName.find("_01j_")    != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu (0/1 jets)");
        else if (cardName.find("_SF_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu/#mu#nu#mu#nu");
        else if (cardName.find("_SF0j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu/#mu#nu#mu#nu (0 jets)");
        else if (cardName.find("_SF1j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu/#mu#nu#mu#nu (1 jet)");
        else if (cardName.find("_SF01j_")  != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu/#mu#nu#mu#nu (0/1 jets)");
        else if (cardName.find("_SF2j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nue#nu/#mu#nu#mu#nu (2 jets)");
        else if (cardName.find("_OF_")     != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu/#mu#nue#nu");
        else if (cardName.find("_OF0j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu/#mu#nue#nu (0 jets)");
        else if (cardName.find("_OF1j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu/#mu#nue#nu (1 jet)");
        else if (cardName.find("_OF01j_")  != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu/#mu#nue#nu (0/1 jets)");
        else if (cardName.find("_OF2j_")   != string::npos) t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowe#nu#mu#nu/#mu#nue#nu (2 jets)");
        else if (cardName.find("_cuts_") != string::npos) {
            t.DrawLatex(channelPosX + 0.4, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu");
            t.DrawLatex(0.67, 0.2, "m_{T} Cut");
        }
        else if (cardName.find("_lpt_") != string::npos) {
            t.DrawLatex(channelPosX + 0.4, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu");
            t.DrawLatex(0.67, 0.2, "Low pT alone");
        }
        else if (cardName.find("_hpt_") != string::npos) {
            t.DrawLatex(channelPosX + 0.4, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu");
            t.DrawLatex(0.67, 0.2, "High pT alone");
        } else {
            t.DrawLatex(channelPosX, channelPosY, "H#rightarrowWW^{(*)}#rightarrowl#nul#nu");
        }
    } else {
        markerSize = 0.8;

        minMass = 0;
        maxMass = 500;

        drawPlot_pulls2(cardName, mass, c1, pad1, pad2);

        pad1->cd();

//        TLatex t;
//        t.SetTextSize(0.03);
//        t.SetNDC();
//
//        t.DrawLatex(channelPosX, channelPosY, cardName.c_str());
//        if (overlay != "") t.DrawLatex(channelPosX, channelPosY-0.025, overlay.c_str());
    }

    labelPosY = channelPosY-0.02;
    //ATLASLabel(labelPosX,labelPosY,"\nPreliminary",1);
    ATLASLabel(labelPosX,labelPosY,"\nInternal",1);

    TLatex p;
    p.SetNDC();
    p.SetTextFont(42);
    p.DrawLatex(labelPosX,labelPosY-0.04,labelTxt.c_str());

//    TLatex t3;
//    t3.SetNDC();
//    t3.SetTextSize(0.03);
//    stringstream lumiLatex;
//    TString energy2 = "8";
//    TString lumi2 = "20.3";
//    lumiLatex << "#sqrt{s} = " << energy2 << " TeV: #lower[-0.2]{#scale[0.6]{#int}}Ldt = " << lumi2 << " fb^{-1}";
//     t3.DrawLatex(channelPosX,channelPosY-0.02,lumiLatex.str().c_str());

    TLatex t3;
    t3.SetTextSize(0.025);
    t3.SetNDC();
    TString stex = "#sqrt{s} = " + energy2 + " TeV";//, #int Ldt = " + lumi2 + " fb^{-1}";
    TString lumitex2 = "#int Ldt = " + lumi2 + " fb^{-1}";
    t3.DrawLatex(channelPosX-0.020, channelPosY, stex.Data());
    t3.DrawLatex(channelPosX-0.020, channelPosY-0.04, lumitex2.Data());
    // for combined plot
//    TLatex t4;
//    t4.SetTextSize(0.025);
//    t4.SetNDC();
//    TString energy1 = "7";
//    TString lumi1 = "4.7";
//    TString lumitex1 = "#sqrt{s} = " + energy1 + " TeV, #int Ldt = " + lumi1 + " fb^{-1}";
//    t4.DrawLatex(channelPosX-0.020, channelPosY-0.00, lumitex1.Data());

    TLatex t2;
    t2.SetTextSize(0.025);
    t2.SetNDC();
    t2.DrawLatex(channelPosX-0.020, channelPosY-0.075, ("m_{H}="+mass+" GeV").c_str());
    //Chiara: modified for diboson
    //t2.DrawLatex(channelPosX, channelPosY-0.07, ("m_{Z}="+mass+" GeV").c_str());
//    TLatex norm_label;
//    norm_label.SetTextSize(0.034);
//    norm_label.SetNDC();
//    norm_label.SetTextFont(42);
////    norm_label.Set
////    norm_label.SetTextColor(kRed);
////    norm_label.DrawLatex(channelPosX-0.020, channelPosY-0.17,"Normalization,");
//    norm_label.DrawLatex(channelPosX+0.310, channelPosY-0.17,"#color[2]{Normalization}#color[1]{ and Pull}");

    stringstream saveName;
    saveName << cardName << "_pulls_";
    if (!m_postFitOrder) saveName << "prefit_";
    saveName << mass;
    cout<<"PRE eps"<<endl;
    save(saveName.str(), "eps", c1);
    cout<<"PRE pdf"<<endl;
    save(saveName.str(), "pdf", c1);
    save(saveName.str(), "png", c1);
    cout<<"DONE"<<endl;

    //save(saveName.str(), "C", c1);

}

// ____________________________________________________________________________|__________
// The actual plotting goes on here
void drawPlot_pulls2(string cardName, string mass, TCanvas* c1, TPad* pad1, TPad* pad2) {
  gStyle->SetHatchesLineWidth(hatch_width);
    cout << "INFO::Drawing pulls: " << cardName << " for mH = " << mass << " GeV";
    std::cout<<"$$$ a"<<std::endl;
    // load and initialize ascii files
    ifstream testFile(("ascii/"+cardName+"_pulls.txt").c_str());
    if (testFile.fail()) {
        cout << "ERROR::file " << ("ascii/"+cardName+"_pulls.txt").c_str() << "does not exist.";
        exit(1);
    }
    fileHolder pulls;
    drawPlot("ascii/"+cardName+"_pulls.txt", 8, pulls);
    std::cout<<"$$$ b"<<std::endl;
    // overlay
    if (overlay != "") {
        ifstream testFile_ol(("ascii/"+overlay+"_pulls.txt").c_str());
        if (testFile_ol.fail()) {
            cout << "ERROR::file " << ("ascii/"+overlay+"_pulls.txt").c_str() << "does not exist.";
            exit(1);
        }
    }
    fileHolder pulls_ol;
    if (overlay != "") drawPlot("ascii/"+overlay+"_pulls.txt", 8, pulls_ol);

    // load and initialize the normalisation factor ascii file
    ifstream testFile2(("ascii/"+cardName+"_pulls_nf.txt").c_str());
    if (testFile2.fail()) {
        cout << "ERROR::file " << ("ascii/"+cardName+"_pulls_nf.txt").c_str() << "does not exist.";
        exit(1);
    }
    fileHolder nfs;
    drawPlot("ascii/"+cardName+"_pulls_nf.txt", 8, nfs);

    // overlay
    if (overlay != "") {
        ifstream testFile2_ol(("ascii/"+overlay+"_pulls_nf.txt").c_str());
        if (testFile2_ol.fail()) {
            cout << "ERROR::file " << ("ascii/"+overlay+"_pulls_nf.txt").c_str() << "does not exist.";
            exit(1);
        }
    }
    fileHolder nfs_ol;
    if (overlay != "") drawPlot("ascii/"+overlay+"_pulls_nf.txt", 8, nfs_ol);

   // load and initialize the category uncertainties
    ifstream testFile3(("ascii/"+cardName+"_breakdown_add.txt").c_str());
    if (testFile3.fail()) {
        cout << "ERROR::file " << ("ascii/"+cardName+"_breakdown_add.txt").c_str() << "does not exist.";
        exit(1);
    }
    fileHolder cats;
    cout<<"@@ cardName: "<<cardName<<endl;
    drawPlot("ascii/"+cardName+"_breakdown_add.txt", 3, cats);
    cout<<"CHECK OUT "<<"ascii/"+cardName+"_breakdown_add.txt"<<endl;
    // overlay
    if (overlay != "") {
        ifstream testFile3_ol(("ascii/"+overlay+"_breakdown_add.txt").c_str());
        if (testFile3_ol.fail()) {
            cout << "ERROR::file " << ("ascii/"+overlay+"_breakdown_add.txt").c_str() << "does not exist.";
            exit(1);
        }
    }
    fileHolder cats_ol;
    if (overlay != "") drawPlot("ascii/"+overlay+"_breakdown_add.txt", 3, cats_ol);

    // get the values from the ascii files
    int nrNuis = pulls.massPoints.size();
    int nrNFs = nfs.massPoints.size();
    int nrCats = cats.massPoints.size();

    //int nrNuis_ol = 30;
    int nrNuis_ol = pulls_ol.massPoints.size();
    int nrNFs_ol = nfs_ol.massPoints.size();
    int nrCats_ol = cats_ol.massPoints.size();
    
    vector<double> points_nuis = pulls.massPoints;
    vector<double> points_nf = nfs.massPoints;
    vector<double> points_cats = cats.massPoints;

    vector<double> points_nuis_ol = pulls_ol.massPoints;
    vector<double> points_nf_ol = nfs_ol.massPoints;
    vector<double> points_cats_ol = cats_ol.massPoints;

    for (int i = 0; i < nrNuis; i++) points_nuis[i]   = i + 0.5;
    for (int i = 0; i < nrNFs; i++) points_nf[i] = i + 0.5;

    for (int i = 0; i < nrNuis_ol; i++) points_nuis_ol[i]   = i + 0.25;
    for (int i = 0; i < nrNFs_ol; i++) points_nf_ol[i] = i + 0.25;

    if (overlay != ""){
        for (int i = 0; i < nrNuis; i++) points_nuis[i]   = i + 0.75;
        // TODO: nfs not needed at the moment
    }

    vector<double> val          = pulls.getCol(0);
    vector<double> up           = pulls.getCol(1);
    vector<double> down         = pulls.getCol(2);
    vector<double> poi_hat      = pulls.getCol(3);
    vector<double> poi_up       = pulls.getCol(4);
    vector<double> poi_down     = pulls.getCol(5);
    vector<double> poi_nom_up   = pulls.getCol(6);
    vector<double> poi_nom_down = pulls.getCol(7);
    //
    vector<double> poi_up_sign   = pulls.getCol(4);
    vector<double> poi_down_sign = pulls.getCol(5);
    vector<double> poi_nom_up_sign   = pulls.getCol(6);
    vector<double> poi_nom_down_sign = pulls.getCol(7);

    vector<double> val_ol          = pulls_ol.getCol(0);
    vector<double> up_ol           = pulls_ol.getCol(1);
    vector<double> down_ol         = pulls_ol.getCol(2);
    vector<double> poi_hat_ol      = pulls_ol.getCol(3);
    vector<double> poi_up_ol       = pulls_ol.getCol(4);
    vector<double> poi_down_ol     = pulls_ol.getCol(5);
    vector<double> poi_nom_up_ol   = pulls_ol.getCol(6);
    vector<double> poi_nom_down_ol = pulls_ol.getCol(7);

    vector<double> nf_val          = nfs.getCol(0);
    vector<double> nf_up           = nfs.getCol(1);
    vector<double> nf_down         = nfs.getCol(2);
    vector<double> nf_poi_hat      = nfs.getCol(3);
    vector<double> nf_poi_up       = nfs.getCol(4);
    vector<double> nf_poi_down     = nfs.getCol(5);
    vector<double> nf_poi_nom_up   = nfs.getCol(6);
    vector<double> nf_poi_nom_down = nfs.getCol(7);

    vector<double> nf_val_ol          = nfs_ol.getCol(0);
    vector<double> nf_up_ol           = nfs_ol.getCol(1);
    vector<double> nf_down_ol         = nfs_ol.getCol(2);
    vector<double> nf_poi_hat_ol      = nfs_ol.getCol(3);
    vector<double> nf_poi_up_ol       = nfs_ol.getCol(4);
    vector<double> nf_poi_down_ol     = nfs_ol.getCol(5);
    vector<double> nf_poi_nom_up_ol   = nfs_ol.getCol(6);
    vector<double> nf_poi_nom_down_ol = nfs_ol.getCol(7);

    vector<double> cats_val  = cats.getCol(0);
    vector<double> cats_up   = cats.getCol(1);
    vector<double> cats_down = cats.getCol(2);

    vector<double> cats_val_ol  = cats_ol.getCol(0);
    vector<double> cats_up_ol   = cats_ol.getCol(1);
    vector<double> cats_down_ol = cats_ol.getCol(2);

    // set correct values for the poi
    for (int i = 0; i < nrNuis; i++) {
        val[i] *= scale_theta;

        poi_up[i]   = poi_up[i] - poi_hat[i];
        poi_down[i] = poi_down[i] - poi_hat[i];

        poi_nom_up[i]   = poi_nom_up[i] - poi_hat[i];
        poi_nom_down[i] = poi_nom_down[i] - poi_hat[i];
        //
        if (poi_up[i] < 0){
            poi_up_sign[i]   = 0.;
            poi_down_sign[i] = fabs(poi_up[i]);
            poi_nom_up_sign[i]   = 0.;
            poi_nom_down_sign[i] = fabs(poi_nom_up[i]);
        }else{
            poi_up_sign[i]   = fabs(poi_up[i]);
            poi_down_sign[i] = 0.;
            poi_nom_up_sign[i]   = fabs(poi_nom_up[i]);
            poi_nom_down_sign[i] = 0.;
        }
        //
        if (poi_up[i] < 0) swap(poi_up[i], poi_down[i]);
        if (poi_nom_up[i] < 0) swap(poi_nom_up[i], poi_nom_down[i]);
        //
        poi_up[i]   = fabs(poi_up[i]);
        poi_down[i] = fabs(poi_down[i]);

        poi_nom_up[i]   = fabs(poi_nom_up[i]);
        poi_nom_down[i] = fabs(poi_nom_down[i]);
        
        poi_hat[i] = 0;
    }

    for (int i = 0; i < nrNuis_ol; i++) {
        val_ol[i] *= scale_theta;

        poi_up_ol[i]   = poi_up_ol[i] - poi_hat_ol[i];
        poi_down_ol[i] = poi_down_ol[i] - poi_hat_ol[i];

        poi_nom_up_ol[i]   = poi_nom_up_ol[i] - poi_hat_ol[i];
        poi_nom_down_ol[i] = poi_nom_down_ol[i] - poi_hat_ol[i];
        
        if (poi_up_ol[i] < 0) swap(poi_up_ol[i], poi_down_ol[i]);
        if (poi_nom_up_ol[i] < 0) swap(poi_nom_up_ol[i], poi_nom_down_ol[i]);
        
        poi_up_ol[i]   = fabs(poi_up_ol[i]);
        poi_down_ol[i] = fabs(poi_down_ol[i]);

        poi_nom_up_ol[i]   = fabs(poi_nom_up_ol[i]);
        poi_nom_down_ol[i] = fabs(poi_nom_down_ol[i]);
        
        poi_hat_ol[i] = 0;
    }

    // do the sum for printout at the end
    double sum_poi2 = 0.;
    for (int i = 0; i < nrNuis; ++i) {  
      double up = fabs(poi_up[i] - poi_hat[i]);
      double down = fabs(poi_up[i] - poi_hat[i]);
      sum_poi2 += pow((up+down)/2, 2);
    }

    // find maximal error due to a single nuisance parameter
    double max_poi = 0.;
    for (int i = 0; i < nrNuis; ++i) {
        if (poi_up[i] > max_poi) max_poi = poi_up[i];
        if (poi_down[i] > max_poi) max_poi = poi_down[i];
    }
    // TODO: for overlay as well? maybe get rid of this...

    // get labels
    vector<std::string> labels;
    Int_t nlines = 0;
    ifstream idFile(("ascii/"+cardName+"_pulls_id.txt").c_str());
    while (1) {
        if (!idFile.good() || nlines > nrNuis-1) break;
        string label;
        idFile >> label;
        labels.push_back(label);
        cout << "added: " << label << endl;
        nlines++;
    }
    cout<<"$"<<endl;
    vector<string> labels_ol;
    if (overlay != "") {
        Int_t nlines_ol = 0;
        ifstream idFile_ol(("ascii/"+overlay+"_pulls_id.txt").c_str());
        while (1) {
            if (!idFile_ol.good() || nlines_ol > nrNuis_ol-1) break;
            string label;
            idFile_ol >> label;
            labels_ol.push_back(label);
            nlines_ol++;
        }
    }

    cout<<"$"<<endl;
    vector<string> nf_labels;
    Int_t nf_nlines = 0;
    ifstream idFile2(("ascii/"+cardName+"_pulls_nf_id.txt").c_str());
    while (1) {
        if (!idFile2.good() || nf_nlines > nrNFs-1) break;
        string nf_label;
        idFile2 >> nf_label;
        nf_labels.push_back(nf_label);
        nf_nlines++;
    }

    cout<<"$"<<endl;
    vector<string> nf_labels_ol;
    if (overlay != "") {
        Int_t nf_nlines_ol = 0;
        ifstream idFile2_ol(("ascii/"+overlay+"_pulls_nf_id.txt").c_str());
        while (1) {
            if (!idFile2_ol.good() || nf_nlines_ol > nrNFs_ol-1) break;
            string nf_label;
            idFile2_ol >> nf_label;
            nf_labels_ol.push_back(nf_label);
            nf_nlines_ol++;
        }
    }

    cout<<"$"<<endl;
    vector<string> cats_labels;
    Int_t cats_nlines = 0;
    ifstream idFile3(("ascii/"+cardName+"_breakdown_add_id.txt").c_str());
    while (1) {
        if (!idFile3.good() || cats_nlines > nrCats-1) break;
        string cat_label;
        idFile3 >> cat_label;
        cats_labels.push_back(cat_label);
        cats_nlines++;
    }

    cout<<"$"<<endl;
    vector<string> cats_labels_ol;
    if (overlay != "") {
        Int_t cats_nlines_ol = 0;
        ifstream idFile3_ol(("ascii/"+overlay+"_breakdown_add_id.txt").c_str());
        while (1) {
            if (!idFile3_ol.good() || cats_nlines_ol > nrCats_ol-1) break;
            string cat_label;
            idFile3_ol >> cat_label;
            cats_labels_ol.push_back(cat_label);
            cats_nlines_ol++;
        }
    }

    cout<<"$ nrCats: "<<nrCats<<endl;
    // map of category uncertainties
    map<string, vector<double> > cat_uncerts;
    for (int i = 0; i < nrCats; i++) {
        string index = cats_labels[i];
        cout << i << " " << index << " " << cats_val[i] << " " << cats_up[i] << " " << cats_down[i] << endl;
        cat_uncerts[index].push_back(cats_val[i]);
        cat_uncerts[index].push_back(cats_up[i]);
        cat_uncerts[index].push_back(cats_down[i]);
    }

    cout<<"$"<<endl;
    map<string, vector<double> > cat_uncerts_ol;
    if (overlay != "") {
        for (int i = 0; i < nrCats_ol; i++) {
            string index = cats_labels_ol[i];
            cat_uncerts_ol[index].push_back(cats_val_ol[i]);
            cat_uncerts_ol[index].push_back(cats_up_ol[i]);
            cat_uncerts_ol[index].push_back(cats_down_ol[i]);
        }
    }

    for(auto a:cat_uncerts)cout<<a.first<<" "<<a.second.size()<<endl;
    double sigma_tot_hi  = cat_uncerts["total"][1];
    double sigma_tot_lo  = cat_uncerts["total"][2];
    double sigma_tot_ol_hi;
    double sigma_tot_ol_lo;
    cout<<"$^"<<endl;
    if (overlay != "") sigma_tot_ol_hi = cat_uncerts_ol["total"][1];
    if (overlay != "") sigma_tot_ol_lo = cat_uncerts_ol["total"][2];

    cout<<"$^"<<endl;

    // hardcoded for now
  // double sigma_tot_hi  = 0.;
  //     double sigma_tot_lo  = 0.;
    double sigma_stat_hi = 0.;
    double sigma_stat_lo = 0.;
    double sigma_syst_hi = 0.;
    double sigma_syst_lo = 0.;
    // TODO: can probably drop this

    // dump everything in maps
    map<string, vector<double> > nuis_map;
    for (int i = 0; i < nrNuis; i++) {
        string index = labels[i];
        nuis_map[index].push_back(val[i]);
        nuis_map[index].push_back(up[i]);
        nuis_map[index].push_back(down[i]);
        nuis_map[index].push_back(poi_hat[i]);
        nuis_map[index].push_back(poi_up[i]);
        nuis_map[index].push_back(poi_down[i]);
        nuis_map[index].push_back(poi_nom_up[i]);
        nuis_map[index].push_back(poi_nom_down[i]);
    }

    cout<<"$"<<endl;
    // dump everything in maps
    map<string, vector<double> > nuis_map_sign;
    for (int i = 0; i < nrNuis; i++) {
        string index = labels[i];
        nuis_map_sign[index].push_back(val[i]);
        nuis_map_sign[index].push_back(up[i]);
        nuis_map_sign[index].push_back(down[i]);
        nuis_map_sign[index].push_back(poi_hat[i]);
        nuis_map_sign[index].push_back(poi_up_sign[i]);
        nuis_map_sign[index].push_back(poi_down_sign[i]);
        nuis_map_sign[index].push_back(poi_nom_up_sign[i]);
        nuis_map_sign[index].push_back(poi_nom_down_sign[i]);
    }

    cout<<"$"<<endl;
    map<string, vector<double> > nuis_map_ol;
    if (overlay != "") {
        for (int i = 0; i < nrNuis_ol; i++) {
            string index = labels_ol[i];
            nuis_map_ol[index].push_back(val_ol[i]);
            nuis_map_ol[index].push_back(up_ol[i]);
            nuis_map_ol[index].push_back(down_ol[i]);
            nuis_map_ol[index].push_back(poi_hat_ol[i]);
            nuis_map_ol[index].push_back(poi_up_ol[i]);
            nuis_map_ol[index].push_back(poi_down_ol[i]);
            nuis_map_ol[index].push_back(poi_nom_up_ol[i]);
            nuis_map_ol[index].push_back(poi_nom_down_ol[i]);
        }
    }

    // check that we have in both maps the same keys
    if (overlay != "") {

        std::vector<string> tmpstring;

        int counter = nrNuis;
        int counter_ol = nrNuis_ol;

        for (int i = 0; i < nrNuis; i++) {
            bool found = 0;
            for (int ii = 0; ii < nrNuis_ol; ii++) {
                if (labels[i] == labels_ol[ii]) found = 1;
            }
            if (!found) {
                nuis_map_ol[labels[i]].push_back(-999); val_ol.push_back(-999);
                nuis_map_ol[labels[i]].push_back(0); up_ol.push_back(0);
                nuis_map_ol[labels[i]].push_back(0); down_ol.push_back(0);
                nuis_map_ol[labels[i]].push_back(0); poi_hat_ol.push_back(0);
                nuis_map_ol[labels[i]].push_back(0); poi_up_ol.push_back(0);
                nuis_map_ol[labels[i]].push_back(0); poi_down_ol.push_back(0);
                nuis_map_ol[labels[i]].push_back(0); poi_nom_up_ol.push_back(0);
                nuis_map_ol[labels[i]].push_back(0); poi_nom_down_ol.push_back(0);
                labels_ol.push_back(labels[i]);
                points_nuis_ol.push_back(counter_ol + 0.25); counter_ol++;
            }
        }

        for (int i = 0; i < nrNuis_ol; i++) {
            bool found = 0;
            for (int ii = 0; ii < nrNuis; ii++) {
                if (labels_ol[i] == labels[ii]) found = 1;
            }
            if (!found) {
                nuis_map[labels_ol[i]].push_back(-999); val.push_back(-999);
                nuis_map[labels_ol[i]].push_back(0); up.push_back(0);
                nuis_map[labels_ol[i]].push_back(0); down.push_back(0);
                nuis_map[labels_ol[i]].push_back(0); poi_hat.push_back(0);
                nuis_map[labels_ol[i]].push_back(0); poi_up.push_back(0);
                nuis_map[labels_ol[i]].push_back(0); poi_down.push_back(0);
                nuis_map[labels_ol[i]].push_back(0); poi_nom_up.push_back(0);
                nuis_map[labels_ol[i]].push_back(0); poi_nom_down.push_back(0);
                labels.push_back(labels_ol[i]);
                points_nuis.push_back(counter + 0.75); counter++;
            }
        }
    }

    // Getting the vectors back
    nrNuis    = labels.size();
    if (overlay != "") nrNuis_ol = labels_ol.size();

    for (int i = 0; i < nrNuis-1; i++) {
        for (int j = 0; j < nrNuis-1-i; j++) {
            if (strcmp(labels[i].c_str(),labels[i+1].c_str())) {
                swap(labels[j], labels[j+1]);
                if (overlay != "") labels_ol[j+1] = labels[j+1];
                if (overlay != "") labels_ol[j] = labels[j];
            }
        }
    }

    for (int i = 0; i < nrNuis; i++) {
        val[i] = nuis_map[labels[i]][0];
        up[i] = nuis_map[labels[i]][1];
        down[i] = nuis_map[labels[i]][2];
        poi_hat[i] = nuis_map[labels[i]][3];
        poi_up[i] = nuis_map[labels[i]][4];
        poi_down[i] = nuis_map[labels[i]][5];
        poi_nom_up[i] = nuis_map[labels[i]][6];
        poi_nom_down[i] = nuis_map[labels[i]][7];
    }
    for (int i = 0; i < nrNuis; i++) {
        poi_up_sign[i] = nuis_map_sign[labels[i]][4];
        poi_down_sign[i] = nuis_map_sign[labels[i]][5];
        poi_nom_up_sign[i] = nuis_map_sign[labels[i]][6];
        poi_nom_down_sign[i] = nuis_map_sign[labels[i]][7];
    }

    for (int i = 0; i < nrNuis_ol; i++) {
        val_ol[i] = nuis_map_ol[labels_ol[i]][0];
        up_ol[i] = nuis_map_ol[labels_ol[i]][1];
        down_ol[i] = nuis_map_ol[labels_ol[i]][2];
        poi_hat_ol[i] = nuis_map_ol[labels_ol[i]][3];
        poi_up_ol[i] = nuis_map_ol[labels_ol[i]][4];
        poi_down_ol[i] = nuis_map_ol[labels_ol[i]][5];
        poi_nom_up_ol[i] = nuis_map_ol[labels_ol[i]][6];
        poi_nom_down_ol[i] = nuis_map_ol[labels_ol[i]][7];
    }

    if (overlay != "") {
        for (int i = 0; i < nrNuis; i++) {
            cout << "Matches: " << labels[i] << " " << labels_ol[i] << endl;
        }
    }

    // sort poi values by variation size
    if (rankNuis) {
        for (int i = 0; i < nrNuis-1; i++) {
            for (int j = 0; j < nrNuis-1-i; j++) {
                bool doSwap = false;
		if (m_postFitOrder) {
                    doSwap = poi_up[j]+poi_down[j] > poi_up[j+1]+poi_down[j+1];
                } else {
                    doSwap = poi_nom_up[j]+poi_nom_down[j] > poi_nom_up[j+1]+poi_nom_down[j+1];
		}
                if (doSwap) {
                // swap postfit poi
                    swap(poi_up[j], poi_up[j+1]);
                    swap(poi_down[j], poi_down[j+1]);
                    swap(poi_up_sign[j], poi_up_sign[j+1]);
                    swap(poi_down_sign[j], poi_down_sign[j+1]);
                    if (overlay != "") {
                        swap(poi_up_ol[j], poi_up_ol[j+1]);
                        swap(poi_down_ol[j], poi_down_ol[j+1]);
                    }

                // swap prefit poi
                    swap(poi_nom_up[j], poi_nom_up[j+1]);
                    swap(poi_nom_down[j], poi_nom_down[j+1]);
                    if (overlay != "") {
                        swap(poi_nom_up_ol[j], poi_nom_up_ol[j+1]);
                        swap(poi_nom_down_ol[j], poi_nom_down_ol[j+1]);
                    }

                // swap pulls
                    swap(up[j], up[j+1]);
                    swap(down[j], down[j+1]);
                    swap(val[j], val[j+1]);
                    if (overlay != "") {
                        swap(up_ol[j], up_ol[j+1]);
                        swap(down_ol[j], down_ol[j+1]);
                        swap(val_ol[j], val_ol[j+1]);
                    }

                // swap names
                    swap(labels[j], labels[j+1]);
                    if (overlay != "") {
                        swap(labels_ol[j], labels_ol[j+1]);
                    }
                }
            }
        }
    }

    // make the 1 sigma boxes
    vector<double> boxup;
    vector<double> boxdown;
    vector<double> cenup;
    vector<double> cendown;
    vector<double> boxup_ol;
    vector<double> boxdown_ol;
    vector<double> cenup_ol;
    vector<double> cendown_ol;

    for (int i = 0; i < nrNuis; i++) {
        boxup.push_back(1.*scale_theta);
        boxdown.push_back(1.*scale_theta);
        double height = 0.5;
        if (overlay != "") height = 0.25;
        cenup.push_back(height);
        cendown.push_back(height);
    }

    for (int i = 0; i < nrNuis_ol; i++) {
        boxup_ol.push_back(1.*scale_theta);
        boxdown_ol.push_back(1.*scale_theta);
        cenup_ol.push_back(0.25);
        cendown_ol.push_back(0.25);
    }

    // make the 1 sigma boxes
    double* statboxup   = new double[nrNuis];
    double* statboxdown = new double[nrNuis];
    double* systboxup   = new double[nrNuis];
    double* systboxdown = new double[nrNuis];

    for (int i = 0; i < nrNuis; i++) {
        statboxup[i]   = sigma_stat_hi * scale_poi / max_poi;
        statboxdown[i] = sigma_stat_lo * scale_poi / max_poi;

        systboxup[i]   = sigma_syst_hi * scale_poi / max_poi;
        systboxdown[i] = sigma_syst_lo * scale_poi / max_poi;
    }
    // TODO: same for overlay, not really needed though

    // find boundaries for NF box
    double max = 1.;
    double min = 1.;
    for (int i = 0; i < nrNFs; ++i) {
        if (nf_val[i] - nf_down[i] < min) min = nf_val[i] - nf_down[i];
        if (nf_val[i] + nf_up[i] > max) max = nf_val[i] + nf_up[i];
    }
    // TODO: same for overlay, not really needed though

    // make the final arrays for plotting, in particular remove parameters
    int nrNuis2remove = 0;
    for (int i = 0; i < nrNuis; i++) {
      // pring average effect
      //double varDo = fabs(poi_down[i]-poi_hat[i]);
      //double varUp = fabs(poi_up[i]-poi_hat[i]);
      //double var = (varDo + varUp) / 2.;
      //cout << "Rank " << nrNuis - i << ":  \t" << var << "  \t" << labels[i] << endl;

      // print up and down effect
      cout << "Rank " << nrNuis - i << ":  \t" << fabs(poi_down[i]-poi_hat[i]) << "  \t" << fabs(poi_up[i]-poi_hat[i]) << "  \t" << labels[i] << endl;

        if ((fabs(poi_down[i]) + fabs(poi_up[i])) / (sigma_tot_lo + sigma_tot_hi) < showHighImpact) {
            cout << "WARNING::Removing " << labels[i] << ". Below threshold." << endl;
            nrNuis2remove++;
        }
    }

    if (showTopParameters != -1) nrNuis2remove = std::max(0, nrNuis - showTopParameters);

    labels.erase(labels.begin(), labels.begin() + nrNuis2remove);
    points_nuis.erase(points_nuis.end() - nrNuis2remove, points_nuis.end());

    if (overlay != "") {
        labels_ol.erase(labels_ol.begin(), labels_ol.begin() + nrNuis2remove);
        points_nuis_ol.erase(points_nuis_ol.end() - nrNuis2remove, points_nuis_ol.end());
    }

    val.erase(val.begin(), val.begin() + nrNuis2remove);
    down.erase(down.begin(), down.begin() + nrNuis2remove);
    up.erase(up.begin(), up.begin() + nrNuis2remove);

    if (overlay != "") {
        val_ol.erase(val_ol.begin(), val_ol.begin() + nrNuis2remove);
        down_ol.erase(down_ol.begin(), down_ol.begin() + nrNuis2remove);
        up_ol.erase(up_ol.begin(), up_ol.begin() + nrNuis2remove);
    }

    poi_hat.erase(poi_hat.begin(), poi_hat.begin() + nrNuis2remove);
    poi_down.erase(poi_down.begin(), poi_down.begin() + nrNuis2remove);
    poi_up.erase(poi_up.begin(), poi_up.begin() + nrNuis2remove);
    poi_down_sign.erase(poi_down_sign.begin(), poi_down_sign.begin() + nrNuis2remove);
    poi_up_sign.erase(poi_up_sign.begin(), poi_up_sign.begin() + nrNuis2remove);

    if (overlay != "") {
        poi_hat_ol.erase(poi_hat_ol.begin(), poi_hat_ol.begin() + nrNuis2remove);
        poi_down_ol.erase(poi_down_ol.begin(), poi_down_ol.begin() + nrNuis2remove);
        poi_up_ol.erase(poi_up_ol.begin(), poi_up_ol.begin() + nrNuis2remove);
    }

    poi_nom_down.erase(poi_nom_down.begin(), poi_nom_down.begin() + nrNuis2remove);
    poi_nom_up.erase(poi_nom_up.begin(), poi_nom_up.begin() + nrNuis2remove);

    if (overlay != "") {
        poi_nom_down_ol.erase(poi_nom_down_ol.begin(), poi_nom_down_ol.begin() + nrNuis2remove);
        poi_nom_up_ol.erase(poi_nom_up_ol.begin(), poi_nom_up_ol.begin() + nrNuis2remove);
    }

    boxdown.erase(boxdown.begin(), boxdown.begin() + nrNuis2remove);
    boxup.erase(boxup.begin(), boxup.begin() + nrNuis2remove);
    cendown.erase(cendown.begin(), cendown.begin() + nrNuis2remove);
    cenup.erase(cenup.begin(), cenup.begin() + nrNuis2remove);

    if (overlay != "") {
        boxdown_ol.erase(boxdown_ol.begin(), boxdown_ol.begin() + nrNuis2remove);
        boxup_ol.erase(boxup_ol.begin(), boxup_ol.begin() + nrNuis2remove);
        cendown_ol.erase(cendown_ol.begin(), cendown_ol.begin() + nrNuis2remove);
        cenup_ol.erase(cenup_ol.begin(), cenup_ol.begin() + nrNuis2remove);
    }

    nrNuis -= nrNuis2remove;
    nrNuis_ol -= nrNuis2remove;
    cout << "INFO::" << nrNuis << " " << nrNuis_ol << " nuisance paramters remaining." << endl;

    int offset = ceil(2 * nrNuis / 10); // used for space to plot the labels and legend

    for (int i = 0; i < nrNuis; i++) {
        poi_up[i] = fabs(poi_up[i]) * scale_poi / max_poi;
        poi_down[i] = fabs(poi_down[i]) * scale_poi / max_poi;
        poi_up_sign[i] = fabs(poi_up_sign[i]) * scale_poi / max_poi;
        poi_down_sign[i] = fabs(poi_down_sign[i]) * scale_poi / max_poi;

        if (overlay != "") {
            poi_up_ol[i] = fabs(poi_up_ol[i]) * scale_poi / max_poi;
            poi_down_ol[i] = fabs(poi_down_ol[i]) * scale_poi / max_poi;
        }

        poi_nom_up[i] = fabs(poi_nom_up[i]) * scale_poi / max_poi;
        poi_nom_down[i] = fabs(poi_nom_down[i]) * scale_poi / max_poi;

        if (overlay != "") {
            poi_nom_up_ol[i] = fabs(poi_nom_up_ol[i]) * scale_poi / max_poi;
            poi_nom_down_ol[i] = fabs(poi_nom_down_ol[i]) * scale_poi / max_poi;
        }

        if (useRelativeImpact) {
            poi_up[i] /= sigma_tot_hi;
            poi_down[i] /= sigma_tot_lo;
            poi_up_sign[i] /= sigma_tot_hi;
            poi_down_sign[i] /= sigma_tot_lo;

            if (overlay != "") {
                poi_up_ol[i] /= sigma_tot_ol_hi;
                poi_down_ol[i] /= sigma_tot_ol_lo;
            }

            poi_nom_up[i] /= sigma_tot_hi;
            poi_nom_down[i] /= sigma_tot_lo;

            if (overlay != "") {
                poi_nom_up_ol[i] /= sigma_tot_ol_hi;
                poi_nom_down_ol[i] /= sigma_tot_ol_lo;
            }
        }

        up[i] = fabs(up[i]) * scale_theta;
        down[i] = fabs(down[i]) * scale_theta;

        if (overlay != "") {
            up_ol[i] = fabs(up_ol[i]) * scale_theta;
            down_ol[i] = fabs(down_ol[i]) * scale_theta;
        }
    }
    // change to the right pad
    pad1->cd();
    // make plot of pulls for nuisance parameters
    markerSize = 2;
    //Added
//    TH2F *h = new TH2F("h", "", 1, border_lo, border_hi, nrNuis+offset+1, -offset, nrNuis+1);
//    vector<int> isNorm;
//    for (int i = offset; i < nrNuis+offset; i++){
//        if(labels[i-offset].find("norm")!= string::npos)
//            isNorm.push_back(1);
//        else
//            isNorm.push_back(0);
//        bool isBDT = false;
//        if(cardName.find("MVA") != string::npos)
//            isBDT = true;
//        TString tmpName = labels[i-offset];
//        bool is8TeV = tmpName.Contains("8TeV");
//        tmpName = tmpName.ReplaceAll("_8TeV","");
//        TString newLabels = translateNPname(tmpName, isBDT);
////        if(is8TeV)
////            newLabels+=" 8TeV";
//        cout<<(i-offset)<<"th NP Renamed "<<labels[i-offset]<<" -> "<<newLabels<<endl;
////        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?labels[i-offset].c_str():"");
////        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?newLabels.c_str():"");
//        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?newLabels.Data():"");
//    }
//
//    vector<double> points_nuis_norm;
//    vector<double> val_norm;
//    vector<double> up_norm;
//    vector<double> down_norm;
//    vector<double> points_nuis_pull;
//    vector<double> val_pull;
//    vector<double> up_pull;
//    vector<double> down_pull;
//    for(int i = 0;i < nrNuis; i++){
//        if(isNorm.at(i) == 1){
//            points_nuis_norm.push_back(points_nuis.at(i));
//            val_norm.push_back(val.at(i));
//            up_norm.push_back(up.at(i));
//            down_norm.push_back(down.at(i));
//        }
//        else{
//            points_nuis_pull.push_back(points_nuis.at(i));
//            val_pull.push_back(val.at(i));
//            up_pull.push_back(up.at(i));
//            down_pull.push_back(down.at(i));
//        }
//    }
    //
//    TGraphAsymmErrors* gr_norm = makeGraphErr("", val_norm.size(), getAry(val_norm), getAry(points_nuis_norm), getAry(down_norm), getAry(up_norm), NULL, NULL);
//    TGraphAsymmErrors* gr      = makeGraphErr("", val_pull.size(), getAry(val_pull), getAry(points_nuis_pull), getAry(down_pull), getAry(up_pull), NULL, NULL);
//    gr->SetLineColor(kBlack);
//    gr->SetMarkerColor(kBlack);
//    gr->SetMarkerStyle(24);
//    gr->SetLineStyle(1);
//    gr->SetLineWidth(3);
//    gr->SetMarkerSize(markerSize);
//    gr->GetXaxis()->SetTitleOffset(1.2);


    TGraphAsymmErrors* gr_ol;
    if (overlay != "") {
        gr_ol = makeGraphErr("", nrNuis_ol, getAry(val_ol), getAry(points_nuis_ol), getAry(down_ol), getAry(up_ol), NULL, NULL);
        gr_ol->SetLineColor(kBlack);
        gr_ol->SetMarkerColor(kBlack);
        gr_ol->SetMarkerStyle(24);
        gr_ol->SetLineStyle(1);
        gr_ol->SetLineWidth(2);
        gr_ol->SetMarkerSize(markerSize);
        gr_ol->GetXaxis()->SetTitleOffset(1.2);
    }

    // make plot of 1 sigma boxes
    TGraphAsymmErrors* gr1s = makeGraphErr("", nrNuis, getAry(val), getAry(points_nuis), getAry(boxdown), getAry(boxup), NULL, NULL);
    gr1s->SetLineColor(color_standardband);
    gr1s->SetMarkerColor(color_standardband);
    gr1s->SetLineStyle(1);
    gr1s->SetLineWidth(2);
    gr1s->SetMarkerSize(markerSize);
    gr1s->GetXaxis()->SetTitleOffset(1.2);

    TGraphAsymmErrors* gr1s_ol;
    if (overlay != "") {
        gr1s_ol = makeGraphErr("", nrNuis_ol, getAry(val_ol), getAry(points_nuis_ol), getAry(boxdown_ol), getAry(boxup_ol), NULL, NULL);
        gr1s_ol->SetLineColor(color_standardband_ol);
        gr1s_ol->SetMarkerColor(color_standardband_ol);
        gr1s_ol->SetLineStyle(1);
        gr1s_ol->SetLineWidth(3);
        gr1s_ol->SetMarkerSize(markerSize*1.25);
        gr1s_ol->GetXaxis()->SetTitleOffset(1.2);
    }

    // make plot of normalisation parameters
    TGraphAsymmErrors* gr_nf = makeGraphErr("", nrNFs, getAry(nf_val), getAry(points_nf), getAry(nf_down), getAry(nf_up), NULL, NULL);
    gr_nf->SetLineColor(1);
    gr_nf->SetMarkerColor(1);
    gr_nf->SetMarkerStyle(20);
    gr_nf->SetLineStyle(1);
    gr_nf->SetLineWidth(1);
    gr_nf->SetMarkerSize(markerSize);
    gr_nf->GetXaxis()->SetTitleOffset(1.2);

    if (overlay != "") {
        // TODO: not needed at the moment
    }
    // make plot for the POI change for postfit uncertainties
    double hatch_width=4;
    TGraphAsymmErrors* gr_poi = makeGraphErr("", nrNuis, getAry(poi_hat), getAry(points_nuis), getAry(poi_down), getAry(poi_up), getAry(cenup), getAry(cendown));
    gr_poi->SetLineColor(color_postfit);
    gr_poi->SetFillColor(color_postfit);
    gr_poi->SetFillStyle(0);
    gr_poi->SetLineWidth(2);
    gr_poi->SetMarkerSize(0);
    //
    // make plot for the POI change for postfit uncertainties positive side
    vector<double> poi_zero;
    for(int ii=0;ii<nrNuis;ii++)
        poi_zero.push_back(0.);
    TGraphAsymmErrors* gr_poi_pos = makeGraphErr("", nrNuis, getAry(poi_hat), getAry(points_nuis), getAry(poi_down_sign), getAry(poi_up_sign), getAry(cenup), getAry(cendown));
    gr_poi_pos->SetLineColor(color_postfit);
    gr_poi_pos->SetFillColor(color_postfit);
    gr_poi_pos->SetFillStyle(3354);//3004);
    gr_poi_pos->SetLineWidth(2);
    gr_poi_pos->SetMarkerSize(0);

    TGraphAsymmErrors* gr_poi_ol;
    if (overlay != "") {
        gr_poi_ol = makeGraphErr("", nrNuis_ol, getAry(poi_hat_ol), getAry(points_nuis_ol), getAry(poi_down_ol), getAry(poi_up_ol), getAry(cenup_ol), getAry(cendown_ol));
        gr_poi_ol->SetLineColor(color_postfit_ol);
        gr_poi_ol->SetFillColor(color_postfit_ol);
        gr_poi_ol->SetFillStyle(3345);
        gr_poi_ol->SetLineWidth(0);
        gr_poi_ol->SetMarkerSize(0);
    }

    // make plot for the POI change for prefit uncertainties
    TGraphAsymmErrors* gr_poi_nom = makeGraphErr("", nrNuis, getAry(poi_hat), getAry(points_nuis), getAry(poi_nom_down), getAry(poi_nom_up), getAry(cenup), getAry(cendown));
    gr_poi_nom->SetLineColor(color_prefit);
    gr_poi_nom->SetFillColor(color_prefit);
    gr_poi_nom->SetLineWidth(1);
    gr_poi_nom->SetMarkerSize(0);

    TGraphAsymmErrors* gr_poi_nom_ol;
    if (overlay != "") {
        gr_poi_nom_ol = makeGraphErr("", nrNuis_ol, getAry(poi_hat_ol), getAry(points_nuis_ol), getAry(poi_nom_down_ol), getAry(poi_nom_up_ol), getAry(cenup_ol), getAry(cendown_ol));
        gr_poi_nom_ol->SetLineColor(color_prefit_ol);
        gr_poi_nom_ol->SetFillColor(color_prefit_ol);
        gr_poi_nom_ol->SetLineWidth(1);
        gr_poi_nom_ol->SetMarkerSize(0);
    }

    double border_lo = -sigma_tot_lo / max_poi;
    double border_hi = sigma_tot_hi / max_poi;
    // different shades for better readability
    int nrShades = ceil((nrNuis+1)/2);
    std::vector<double> shadeCenter;
    std::vector<double>  shadePoints;
    std::vector<double>  shadeWidth;
    std::vector<double>  shadeHeight;
    for (int ishade = 0; ishade < nrShades; ishade++) {
        shadeCenter.push_back(0.0);
        shadePoints.push_back(2.0*ishade+0.5);
        shadeWidth.push_back(10.); // TODO: should not be hardcoded
        shadeHeight.push_back(0.5);
    }

    TGraphAsymmErrors* gr_shades = makeGraphErr("", nrShades, getAry(shadeCenter), getAry(shadePoints), getAry(shadeWidth), getAry(shadeWidth), getAry(shadeHeight), getAry(shadeHeight));
    gr_shades->SetLineColor(18);
    gr_shades->SetFillColor(18);
    gr_shades->SetFillStyle(1001);
    gr_shades->SetLineWidth(1);
    gr_shades->SetMarkerSize(0);

    // histogram to get the nuisance parameter labels correct
//    TH2F *h = new TH2F("h", "", 1, border_lo, border_hi, nrNuis+offset+1, -offset, nrNuis+1);
//    vector<int> isNorm;
//    for (int i = offset; i < nrNuis+offset; i++){
//        if(labels[i-offset].find("norm")!= string::npos)
//            isNorm.push_back(1);
//        else
//            isNorm.push_back(0);
//        bool isBDT = false;
//        if(cardName.find("MVA") != string::npos)
//            isBDT = true;
//        TString tmpName = labels[i-offset];
//        bool is8TeV = tmpName.Contains("8TeV");
//        tmpName = tmpName.ReplaceAll("_8TeV","");
//        TString newLabels = translateNPname(tmpName, isBDT);
////        if(is8TeV)
////            newLabels+=" 8TeV";
//        cout<<(i-offset)<<"th NP Renamed "<<labels[i-offset]<<" -> "<<newLabels<<endl;
////        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?labels[i-offset].c_str():"");
////        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?newLabels.c_str():"");
//        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?newLabels.Data():"");
//    }
    //Added
    TH2F *h = new TH2F("h", "", 1, border_lo, border_hi, nrNuis+offset+1, -offset, nrNuis+1);
    vector<int> isNorm;
    for (int i = offset; i < nrNuis+offset; i++){
        if(labels[i-offset].find("norm")!= string::npos)
            isNorm.push_back(1);
        else
            isNorm.push_back(0);
        bool isBDT = false;
        if(cardName.find("MVA") != string::npos)
            isBDT = true;
        TString tmpName = labels[i-offset];
        bool is8TeV = tmpName.Contains("8TeV");
        tmpName = tmpName.ReplaceAll("_8TeV","");
        TString newLabels = translateNPname(tmpName, isBDT);
//        if(is8TeV)
//            newLabels+=" 8TeV";
        cout<<(i-offset)<<"th NP Renamed "<<labels[i-offset]<<" -> "<<newLabels<<endl;
//        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?labels[i-offset].c_str():"");
//        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?newLabels.c_str():"");
        h->GetYaxis()->SetBinLabel(i+1, drawParamNames?newLabels.Data():"");
    }

    vector<double> points_nuis_norm;
    vector<double> val_norm;
    vector<double> up_norm;
    vector<double> down_norm;
    vector<double> points_nuis_pull;
    vector<double> val_pull;
    vector<double> up_pull;
    vector<double> down_pull;
    for(int i = 0;i < nrNuis; i++){
        if(isNorm.at(i) == 1){
            points_nuis_norm.push_back(points_nuis.at(i));
            val_norm.push_back(val.at(i));
            up_norm.push_back(up.at(i));
            down_norm.push_back(down.at(i));
        }
        else{
            points_nuis_pull.push_back(points_nuis.at(i));
            val_pull.push_back(val.at(i));
            up_pull.push_back(up.at(i));
            down_pull.push_back(down.at(i));
        }
    }
    //
//    TGraphAsymmErrors* gr_norm = makeGraphErr("", val_norm.size(), getAry(val_norm), getAry(points_nuis_norm), getAry(down_norm), getAry(up_norm), NULL, NULL);
    TGraphAsymmErrors* gr      = makeGraphErr("", val_pull.size(), getAry(val_pull), getAry(points_nuis_pull), getAry(down_pull), getAry(up_pull), NULL, NULL);
    gr->SetLineColor(kBlack);
    gr->SetMarkerColor(kBlack);
    gr->SetMarkerStyle(20);
    gr->SetLineStyle(1);
    gr->SetLineWidth(3);
    gr->SetMarkerSize(markerSize);
    gr->GetXaxis()->SetTitleOffset(1.2);
    
    TGraphAsymmErrors* gr_norm = makeGraphErr("", val_norm.size(), getAry(val_norm), getAry(points_nuis_norm), getAry(down_norm), getAry(up_norm), NULL, NULL);
    gr_norm->SetLineColor(kRed);
    gr_norm->SetMarkerColor(kRed);
    gr_norm->SetMarkerStyle(24);
    gr_norm->SetLineStyle(1);
    gr_norm->SetLineWidth(3);
    gr_norm->SetMarkerSize(markerSize);
    gr_norm->GetXaxis()->SetTitleOffset(1.2);
    //
    h->LabelsOption("h");
    double labelSize = 1./nrNuis;
    h->SetLabelSize(labelSize>UserlabelSize?UserlabelSize:labelSize,"Y");//label size
    h->GetXaxis()->SetLabelColor(kWhite);
    h->GetXaxis()->SetAxisColor(kWhite);
    h->GetXaxis()->SetRangeUser(-2,2);
    h->GetYaxis()->SetLabelColor(kBlack);
    h->GetYaxis()->SetAxisColor(kBlack);
    h->GetYaxis()->SetTickLength(0.);
    h->SetStats(0);
    // h->LabelsDeflate();
    h->Draw("h");
    // TODO: order should be the same for overlay, so just do it once

    // histogram to get the normalisation parameters labels correct
    TH2F *h2 = new TH2F("h2", "", 1, min-0.05, max+0.05, nrNFs, 0, nrNFs);
    for (int i = 0; i < nrNFs; i++)
        h2->GetYaxis()->SetBinLabel(i+1, drawParamNames?nf_labels[i].c_str():"");
    h2->SetStats(0);
    h2->SetLabelSize(0.1, "X");
    h2->SetLabelSize(0.1, "Y");
    // TODO: not needed at the moment

    // axis for the POI correlation
    TGaxis *axis_poi = new TGaxis(border_lo, nrNuis+1, border_hi, nrNuis+1, (-sigma_tot_lo) / scale_poi, (sigma_tot_hi) / scale_poi, 510, "-L");
    axis_poi->ImportAxisAttributes(h->GetXaxis());
    axis_poi->SetName("axis_poi");
    // if (useRelativeImpact) axis_poi->SetTitle("#Delta#hat{#mu}/#Delta#hat{#mu}_{tot}");
    // else axis_poi->SetTitle("#Delta#hat{#mu}");
    if (useRelativeImpact) axis_poi->SetTitle("#Delta#mu/#Delta#mu_{tot}");
    else axis_poi->SetTitle("#Delta#mu");
    axis_poi->SetTitleOffset(1.1);
//    axis_poi->SetLineColor(kBlack);
//    axis_poi->SetLabelColor(kBlack);
//    axis_poi->SetTitleColor(kBlack);
    axis_poi->SetLineColor(kBlue);
    axis_poi->SetLabelColor(kBlue);
    axis_poi->SetTitleColor(kBlue);
    axis_poi->SetLabelSize(0.034);
    axis_poi->SetTitleSize(0.034);

    // axis for the nuisance parameter pull
    TGaxis *axis_theta = new TGaxis(border_lo, -offset, border_hi, -offset, (-sigma_tot_lo / max_poi) / scale_theta, (sigma_tot_hi / max_poi) / scale_theta, 510, "+R");
    axis_theta->ImportAxisAttributes(h->GetXaxis());
    axis_theta->SetName("axis_theta");
//    axis_theta->SetTitle("Pull");
//    axis_theta->SetTitle("#color[1]{Pull, }#color[2]{Normalization}");
    axis_theta->SetTitle("");
    axis_theta->SetTitleOffset(1.1);
    axis_theta->SetLineColor(kBlack);
    axis_theta->SetLabelColor(kBlack);
    axis_theta->SetTitleColor(kBlack);
    axis_theta->SetLabelSize(0.034);
    axis_theta->SetTitleSize(0.034);

    // axis for the nuisance parameter labels
    TGaxis *axis_label = new TGaxis(border_lo, 0, border_lo, nrNuis+1, 0, nrNuis+1, 0, "-R");
    axis_label->SetLineColor(kBlack);
    axis_label->SetTitleColor(kWhite);
    axis_label->SetLabelSize(0);
    axis_label->SetNdivisions(0);

    // some line definitions
    TLine l;
    l.SetLineWidth(2);
    l.SetLineColor(color_pulls);
    l.SetLineStyle(2);

    TLine l_stat;
    l_stat.SetLineWidth(2);
    l_stat.SetLineColor(color_staterror);
    l_stat.SetLineStyle(2);

    TLine l_syst;
    l_syst.SetLineWidth(2);
    l_syst.SetLineColor(color_systerror);
    l_syst.SetLineStyle(2);

    TLine l_tot;
    l_tot.SetLineWidth(2);
    l_tot.SetLineColor(color_totalerror);
    l_tot.SetLineStyle(2);

    // draw the nuisance parameter pulls including error bands and impact on poi
    gr_shades->Draw("p2");
    gr_shades->GetXaxis()->SetRange(-2,2);

    if (drawPrefitImpactBand) {
        gr_poi_nom->Draw("p2");
        if (overlay != "") gr_poi_nom_ol->Draw("p2");
    }
    if (drawPostfitImpactBand) {
        gr_poi->Draw("p2");
        gr_poi_pos->Draw("p2 same");
        if (overlay != "") gr_poi_ol->Draw("p2");
    }
    // draw axes
    if (drawPrefitImpactBand || drawPostfitImpactBand || drawErrorBars) axis_poi->Draw();
    axis_theta->Draw();
    axis_label->Draw();

    // draw +-1 and 0 sigma lines for pulls
    l.DrawLine( 0.              , 0.,  0.              , nrNuis);
    l.DrawLine( 1. * scale_theta, 0.,  1. * scale_theta, nrNuis);
    l.DrawLine(-1. * scale_theta, 0., -1. * scale_theta, nrNuis);
    gStyle->SetEndErrorSize(5.0);
    if(drawStandardBand){
        gr1s->Draw("p");
        if (overlay != "") gr1s_ol->Draw("p");
    }
    // draw syst and stat errors
    if (drawErrorBars) {
        l_stat.SetLineStyle(1);
        l_syst.SetLineStyle(1);
        l_tot.SetLineStyle(1);

        l_stat.DrawLine(-sigma_stat_lo * scale_poi / max_poi, 1.07*nrNuis,  sigma_stat_hi * scale_poi / max_poi, 1.07*nrNuis);
        l_stat.DrawLine( sigma_stat_hi * scale_poi / max_poi, 1.06*nrNuis,  sigma_stat_hi * scale_poi / max_poi, 1.08*nrNuis);
        l_stat.DrawLine(-sigma_stat_lo * scale_poi / max_poi, 1.06*nrNuis, -sigma_stat_lo * scale_poi / max_poi, 1.08*nrNuis);

        l_syst.DrawLine(-sigma_syst_lo * scale_poi / max_poi, 1.10*nrNuis,  sigma_syst_hi * scale_poi / max_poi, 1.10*nrNuis);
        l_syst.DrawLine( sigma_syst_hi * scale_poi / max_poi, 1.09*nrNuis,  sigma_syst_hi * scale_poi / max_poi, 1.11*nrNuis);
        l_syst.DrawLine(-sigma_syst_lo * scale_poi / max_poi, 1.09*nrNuis, -sigma_syst_lo * scale_poi / max_poi, 1.11*nrNuis);

        l_tot.DrawLine(-sigma_tot_lo * scale_poi / max_poi, 1.13*nrNuis,  sigma_tot_hi * scale_poi / max_poi, 1.13*nrNuis);
        l_tot.DrawLine( sigma_tot_hi * scale_poi / max_poi, 1.12*nrNuis,  sigma_tot_hi * scale_poi / max_poi, 1.14*nrNuis);
        l_tot.DrawLine(-sigma_tot_lo * scale_poi / max_poi, 1.12*nrNuis, -sigma_tot_lo * scale_poi / max_poi, 1.14*nrNuis);

        TLatex t_stat;
        TLatex t_syst;
        TLatex t_tot;

        t_stat.SetTextSize(0.03);
        t_stat.SetTextAlign(32);
        t_stat.SetTextColor(color_staterror);
        t_stat.DrawLatex((-sigma_stat_lo-0.025) * scale_poi / max_poi, 1.07*nrNuis, "statistics");

        t_syst.SetTextSize(0.03);
        t_syst.SetTextAlign(32);
        t_syst.SetTextColor(color_systerror);
        t_syst.DrawLatex((-sigma_syst_lo-0.025) * scale_poi / max_poi, 1.10*nrNuis, "systematics");

        t_tot.SetTextSize(0.03);
        t_tot.SetTextAlign(32);
        t_tot.SetTextColor(color_totalerror);
        t_tot.DrawLatex((-sigma_tot_lo-0.025) * scale_poi / max_poi, 1.13*nrNuis, "total");

        t_stat.Draw();
        t_syst.Draw();
        t_tot.Draw();
    }
    // gr->GetXaxis()->SetRangeUser(-1.7,1.7);//AXIS
    gr->Draw("p");
    gr_norm->Draw("p same");
    if (overlay != "") gr_ol->Draw("p");

    pad1->SetTicks(0, 0);

    c1->SetLogy(0);

    TLegend* leg = makeLeg();
    leg->SetX1(channelPosX + 0.27);
    leg->SetY1(channelPosY-0.0775);
    leg->SetX2(channelPosX + 0.77);
    leg->SetY2(channelPosY+0.02);
    leg->SetTextSize(0.0225);

    leg->AddEntry(gr, "Pull: (#hat{#theta} - #theta_{0})/#Delta#theta","lp");
    leg->AddEntry(gr_norm, "Normalisation","lp");
    if (nrNuis_ol > 0) leg->AddEntry(gr_ol, "Alt pull","lp");
    if(drawStandardBand){
        leg->AddEntry(gr1s, "1 standard deviation","l");
        if (nrNuis_ol > 0) leg->AddEntry(gr1s_ol, "Alt 1 standard deviation","l");
    }
    if (drawPostfitImpactBand) {
      leg->AddEntry(gr_poi_pos, "+1#sigma Postfit Impact on #mu", "f");//LOST YOUR HAT?
        leg->AddEntry(gr_poi, "-1#sigma Postfit Impact on #mu", "f");
        if (nrNuis_ol > 0) leg->AddEntry(gr_poi_ol, "Alt Postfit Impact on #mu", "f");
    }

    leg->Draw();

    // draw the normalisations
    if (drawInset) {
        pad2->cd();

        TLine l2;
        l2.SetLineWidth(2);
        l2.SetLineColor(13);
        l2.SetLineStyle(2);

        h2->Draw();
        l2.DrawLine(1., 0., 1., nrNFs);
        gr_nf->Draw("p");
    }

    cout << "total unc = " << (fabs(sigma_tot_hi) + fabs(sigma_tot_lo)) / 2 << endl;
    cout << "sum of sq = " << sqrt(sum_poi2) << endl;
}

// ____________________________________________________________________________|__________
// dump ROOT file to ascii table
void ROOT2Ascii(string folder) {
    system("mkdir -vp ascii");

    enum ConvertMode {pulls, breakdown};

    ConvertMode mode;
    if (folder.find("pulls") != string::npos) {
        mode = pulls;
    } else if (folder.find("breakdown") != string::npos) {
        mode = pulls;
    } else {
        cout << "ERROR::Something went wrong." << endl;
        exit(1);
    }

    bool ignoreInfNan = 1;

    std::vector<TString> list;

    TSystemDirectory  dire(("root-files/"+folder).c_str(), ("root-files/"+folder).c_str());

    TList *files = dire.GetListOfFiles();
    TIter next(files);
    TSystemFile *file;
    TString fname;
    while((file = (TSystemFile*)next())) {
        fname = file->GetName();
        if(file->IsDirectory()) continue;

        if (removeHbb && fname.Contains("Hbb")) continue;
        if (removeHtt && (fname.Contains("Htt") || fname.Contains("ATLAS_TAU"))) continue;

        list.push_back(fname.Data());
    }
    
    stringstream outFileName;
    outFileName << "ascii/" << folder;
    
    ofstream outFile((outFileName.str()+".txt").c_str());
    ofstream outFile_id((outFileName.str()+"_id.txt").c_str());
    ofstream outFile_nf((outFileName.str()+"_nf.txt").c_str());
    ofstream outFile_nf_id((outFileName.str()+"_nf_id.txt").c_str());

    int nrNPs = 0;
    int nrNFs = 0;

    for(vector<TString>::iterator it = list.begin(); it != list.end(); it++) {
        stringstream fileName;
        fileName << "root-files/" << folder << "/" << *it;

        bool isNorm = false;
        bool success = true;
        bool isInfNan = false;
    
        TFile* f = NULL;
        f = new TFile(fileName.str().c_str());
        if (f && !f->IsOpen()) success = false;

        if (!success) {
	  cout << "ERROR: Could not open file " << fileName.str() << endl;
          continue;
	}

        TString histoName = it->ReplaceAll(".root", "");

        // if (histoName.Contains("atlas_nbkg_")) isNorm = true;
        // if (histoName.Contains("slope_")) isNorm = true;
        // if (histoName.Contains("p0_")) isNorm = true;
        // if (histoName.Contains("p1_")) isNorm = true;
        // if (histoName.Contains("p2_")) isNorm = true;
        // if (histoName.Contains("p3_")) isNorm = true;
        //if (histoName.Contains("ATLAS_norm") /*&& histoName.Contains("lvlv")*/) isNorm = true;
        // if (histoName.Contains("ATLAS_Hbb_norm_")) isNorm = true;
        // if (histoName.Contains("ATLAS_PM_EFF_")) isNorm = true;
        // if (histoName.Contains("scale_norm")) isNorm = true;
	//	if (histoName.Contains("VH_")) isNorm = true;

        TH1D* hist = (TH1D*)f->Get(histoName);

        int nrBins = hist->GetNbinsX();

        for (int bin = 1; bin <= nrBins; bin++) {
            double number = hist->GetBinContent(bin);
            
            // check inf
            if (number > 10e9) isInfNan = true;

            // check nan
            if (number != number) isInfNan = true;
        }
    
        if (ignoreInfNan && isInfNan) {
            cout << "WARNING::Skipping " << *it << " because of inf/nan" << endl;
            continue;
        }

        (isNorm?outFile_nf:outFile) << (isNorm?nrNPs++:nrNFs++) << " ";

        for (int bin = 1; bin <= nrBins; bin++) {
            double number = hist->GetBinContent(bin);
            (isNorm?outFile_nf:outFile) << number << ((bin < nrBins)?" ":"\n");
        }

        if (isNorm) outFile_nf_id << * it << "\n";
        else outFile_id << * it << "\n";
        if(mode==breakdown) outFile_id<<"~~*~*~*~*~*~*~*~*~*~ " << * it << "\n";

        f->Close();
    }

    cout << "Writing to file: " << outFileName.str() << "*.txt" << endl;

    outFile.close();
    outFile_id.close();
    
    if (mode == pulls) {
        outFile_nf.close();
        outFile_nf_id.close();
    }
}

// ____________________________________________________________________________|__________
// initialize file
void loadFile(const char* fileName, int cols, fileHolder file) {
    ifstream testFile(fileName);
    if (testFile.fail()) {
        cout << "ERROR::file " << fileName << "does not exist.";
        exit(1);
    }
    drawPlot(fileName, cols, file);
}

// ____________________________________________________________________________|__________
// Return vector of strings from textfile
vector<string> getLabel(const char* fileName, int nrPars) {
    vector<string> tmp_labels;
    Int_t nlines = 0;
    ifstream idFile(fileName);
    while (1) {
        if (!idFile.good() || nlines > nrPars) break;
        string label;
        idFile >> label;
        tmp_labels.push_back(label);
        nlines++;
    }
    return tmp_labels;
}
TString translateNPname(TString internalName, bool isMVA){
  if(internalName == "ATLAS_norm_Wbb") return "W+HF normalisation";//b#bar{b}
if(internalName == "ATLAS_norm_Wcl") return "W+cl normalisation";
 if(internalName == "ATLAS_norm_Zbb") return "Z+HF normalisation";;//"Z+b#bar{b} normalisation";
if(internalName == "ATLAS_norm_Zcl") return "Z+cl normalisation";
if(internalName == "ATLAS_norm_ttbar") return "t#bar{t} normalisation";
if(internalName == "ATLAS_norm_ttbar_L0") return "0-lepton t#bar{t} normalisation";
if(internalName == "ATLAS_norm_ttbar_L1") return "1-lepton t#bar{t} normalisation";
if(internalName == "ATLAS_norm_ttbar_L2") return "2-lepton t#bar{t} normalisation";
if(internalName == "alpha_ATLAS_LUMI_2012") return "Luminosity";
if(internalName == "alpha_ATLAS_LUMI_2015") return "Luminosity";
if(internalName == "alpha_ATLAS_LUMI_2015_2016") return "Luminosity";
// radiation
if(internalName == "alpha_ttbar_radHi") return "t#bar{t} ISR/FSR";
if(internalName == "alpha_ttbar_radLo") return "t#bar{t} ISR/FSR";


//Run 2 b-tagging
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_0")return "Light-flavour tagging efficiency 0";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_1")return "Light-flavour tagging efficiency 1";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_2")return "Light-flavour tagging efficiency 2";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_3")return "Light-flavour tagging efficiency 3";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_4")return "Light-flavour tagging efficiency 4";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_5")return "Light-flavour tagging efficiency 5";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_6")return "Light-flavour tagging efficiency 6";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_7")return "Light-flavour tagging efficiency 7";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_8")return "Light-flavour tagging efficiency 8";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_9")return "Light-flavour tagging efficiency 9";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_10")return "Light-flavour tagging efficiency 10";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_11")return "Light-flavour tagging efficiency 11";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_12")return "Light-flavour tagging efficiency 12";
 if(internalName == "alpha_SysFT_EFF_Eigen_Light_13")return "Light-flavour tagging efficiency 13";
 if(internalName == "alpha_SysFT_EFF_Eigen_C_0")return "c-jet tagging efficiency 0";
 if(internalName == "alpha_SysFT_EFF_Eigen_C_1")return "c-jet tagging efficiency 1";
 if(internalName == "alpha_SysFT_EFF_Eigen_C_2")return "c-jet tagging efficiency 2";
 if(internalName == "alpha_SysFT_EFF_Eigen_C_3")return "c-jet tagging efficiency 3";
 if(internalName == "alpha_SysFT_EFF_Eigen_B_0")return "b-jet tagging efficiency 0";
 if(internalName == "alpha_SysFT_EFF_Eigen_B_1")return "b-jet tagging efficiency 1";
 if(internalName == "alpha_SysFT_EFF_Eigen_B_2")return "b-jet tagging efficiency 2";
 if(internalName == "alpha_SysFT_EFF_Eigen_B_3")return "b-jet tagging efficiency 3";
 if(internalName == "alpha_SysFT_EFF_Eigen_B_4")return "b-jet tagging efficiency 4";
 if(internalName == "alpha_SysFT_EFF_Eigen_B_5")return "b-jet tagging efficiency 5";
 if(internalName == "alpha_SysFT_EFF_extrapolation")return "b-jet tagging extrapolation";

 // 19NP Scheme
 if(internalName == "alpha_SysJET_19NP_JET_EffectiveNP_1")return"JES NP 1";
 if(internalName == "alpha_SysJET_19NP_JET_EffectiveNP_2")return"JES NP 2";
 if(internalName == "alpha_SysJET_19NP_JET_EffectiveNP_3")return"JES NP 3";
 if(internalName == "alpha_SysJET_19NP_JET_EffectiveNP_4")return"JES NP 4";
 if(internalName == "alpha_SysJET_19NP_JET_EffectiveNP_5")return"JES NP 5";
 if(internalName == "alpha_SysJET_19NP_JET_EffectiveNP_6restTerm")return"JES NP 6";
 if(internalName == "alpha_SysJET_19NP_JET_EtaIntercalibration_Modelling")return"JES #eta intercalibration modelling";
 if(internalName == "alpha_SysJET_19NP_JET_EtaIntercalibration_TotalStat")return"JES #eta intercalibration stat.";
 if(internalName == "alpha_SysJET_19NP_JET_EtaIntercalibration_NonClosure")return"JES #eta intercalibration non-closure";
 if(internalName == "alpha_SysJET_19NP_JET_Pileup_OffsetMu")return"JES PU offset(#mu)";
 if(internalName == "alpha_SysJET_19NP_JET_Pileup_OffsetNPV")return"JES PU offset(N_{PV})";
 if(internalName == "alpha_SysJET_19NP_JET_Pileup_PtTerm")return"JES PU p_{T} term";
 if(internalName == "alpha_SysJET_19NP_JET_Pileup_RhoTopology")return"JES PU #rho topology";
 if(internalName == "alpha_SysJET_19NP_JET_BJES_Response")return"b-jets response";
 if(internalName == "alpha_SysJET_19NP_JET_PunchThrough_MCTYPE")return"JES punch-through MC Type";
 if(internalName == "alpha_SysJET_19NP_JET_SingleParticle_HighPt")return"JES single particle hi-p_{T}";
 if(internalName == "alpha_SysJET_19NP_JET_Flavor_Response")return"JES flavour response";
 if(internalName == "alpha_SysJET_19NP_JET_Flavor_Composition_Top")return"JES top flavour composition";
 if(internalName == "alpha_SysJET_19NP_JET_Flavor_Composition_Zjets")return"JES Z+jets flavour composition";
 if(internalName == "alpha_SysJET_19NP_JET_Flavor_Composition_Wjets")return"JES W+jets flavour composition";

 if(internalName == "alpha_SysJET_JER_SINGLE_NP")return"Jet energy resolution";
    
 // Correlate
 if(internalName == "alpha_SysJET_19NP_JET_Flavor_Composition")return"JES flavour composition";

if(internalName == "alpha_SysBJetReso") return "b-jet energy resolution";
if(internalName == "alpha_SysFTBTagB0Effic_Y2012") return "b-jet tagging efficiency 0";
if(internalName == "alpha_SysBTagB0Effic_Y2012") return "b-jet tagging efficiency 0";
if(internalName == "alpha_SysBTagB1Effic_Y2012") return "b-jet tagging efficiency 1";
if(internalName == "alpha_SysBTagB2Effic_Y2012") return "b-jet tagging efficiency 2";
if(internalName == "alpha_SysBTagB3Effic_Y2012") return "b-jet tagging efficiency 3";
if(internalName == "alpha_SysBTagB4Effic_Y2012") return "b-jet tagging efficiency 4";
if(internalName == "alpha_SysBTagB5Effic_Y2012") return "b-jet tagging efficiency 5";
if(internalName == "alpha_SysBTagB6Effic_Y2012") return "b-jet tagging efficiency 6";
if(internalName == "alpha_SysBTagB7Effic_Y2012") return "b-jet tagging efficiency 7";
if(internalName == "alpha_SysBTagB8Effic_Y2012") return "b-jet tagging efficiency 8";
if(internalName == "alpha_SysBTagB9Effic_Y2012") return "b-jet tagging efficiency 9";
if(internalName == "alpha_SysBTagBPythia8_Y2012") return "#splitline{Generator dependence of}{    tagging efficiency}";
if(internalName == "alpha_SysBTagBSherpa_Y2012") return "Generator dependence of tagging eff.(for Sherpa)";
if(internalName == "alpha_SysBTagC0Effic_Y2012") return "c-jet tagging efficiency 0";
if(internalName == "alpha_SysBTagC1Effic_Y2012") return "c-jet tagging efficiency 1";
if(internalName == "alpha_SysBTagC2Effic_Y2012") return "c-jet tagging efficiency 2";
if(internalName == "alpha_SysBTagC3Effic_Y2012") return "c-jet tagging efficiency 3";
if(internalName == "alpha_SysBTagC4Effic_Y2012") return "c-jet tagging efficiency 4";
if(internalName == "alpha_SysBTagC5Effic_Y2012") return "c-jet tagging efficiency 5";
if(internalName == "alpha_SysBTagC6Effic_Y2012") return "c-jet tagging efficiency 6";
if(internalName == "alpha_SysBTagC7Effic_Y2012") return "c-jet tagging efficiency 7";
if(internalName == "alpha_SysBTagC8Effic_Y2012") return "c-jet tagging efficiency 8";
if(internalName == "alpha_SysBTagC9Effic_Y2012") return "c-jet tagging efficiency 9";
if(internalName == "alpha_SysBTagC10Effic_Y2012") return "c-jet tagging efficiency 10";
if(internalName == "alpha_SysBTagC11Effic_Y2012") return "c-jet tagging efficiency 11";
if(internalName == "alpha_SysBTagC12Effic_Y2012") return "c-jet tagging efficiency 12";
if(internalName == "alpha_SysBTagC13Effic_Y2012") return "c-jet tagging efficiency 13";
if(internalName == "alpha_SysBTagC14Effic_Y2012") return "c-jet tagging efficiency 14";
if(internalName == "alpha_SysBTagCPythia8_Y2012") return "Generator dependence of tagging eff. (for charm)";
if(internalName == "alpha_SysBTagCSherpa_Y2012") return "Generator dependence of tagging eff. (for Sherpa, charm)";
if(internalName == "alpha_SysBTagL0Effic_Y2012") return "l-jet tagging efficiency 0";
if(internalName == "alpha_SysBTagL1Effic_Y2012") return "l-jet tagging efficiency 1";
if(internalName == "alpha_SysBTagL2Effic_Y2012") return "l-jet tagging efficiency 2";
if(internalName == "alpha_SysBTagL3Effic_Y2012") return "l-jet tagging efficiency 3";
if(internalName == "alpha_SysBTagL4Effic_Y2012") return "l-jet tagging efficiency 4";
if(internalName == "alpha_SysBTagL5Effic_Y2012") return "l-jet tagging efficiency 5";
if(internalName == "alpha_SysBTagL6Effic_Y2012") return "l-jet tagging efficiency 6";
if(internalName == "alpha_SysBTagL7Effic_Y2012") return "l-jet tagging efficiency 7";
if(internalName == "alpha_SysBTagL8Effic_Y2012") return "l-jet tagging efficiency 8";
if(internalName == "alpha_SysBTagL9Effic_Y2012") return "l-jet tagging efficiency 9";
if(internalName == "alpha_SysElecE") return "Electron energy";
//if(internalName == "alpha_SysElecEffic") return "Electron efficiency";
if(internalName == "alpha_SysJVF_Y2012") return "jet vertex fraction";
if(internalName == "alpha_SysJetBE") return "b-jet energy";//
if(internalName == "alpha_SysJetEResol_Y2012") return "Jet energy resolution";
if(internalName == "alpha_SysJetEtaModel") return "Jet energy scale eta modeling";
//if(internalName == "alpha_SysJetEtaStat_Y2012")
if(internalName == "alpha_SysJetFlavB") return "Modeling of b-jet response";
//if(internalName == "alpha_SysJetFlavComp_Top")
//if(internalName == "alpha_SysJetFlavComp_VHVV")
//if(internalName == "alpha_SysJetFlavComp_Wjets")
//if(internalName == "alpha_SysJetFlavComp_Zjets")
//if(internalName == "alpha_SysJetFlavResp_Top")
//if(internalName == "alpha_SysJetFlavResp_VHVV")
//if(internalName == "alpha_SysJetFlavResp_Wjets")
//if(internalName == "alpha_SysJetFlavResp_Zjets")
//if(internalName == "alpha_SysJetMu")
if(internalName == "alpha_SysJetNP1_Y2012") return "Jet energy scale 1";
if(internalName == "alpha_SysJetNP2_Y2012") return "Jet energy scale 2";
if(internalName == "alpha_SysJetNP3_Y2012") return "Jet energy scale 3";
if(internalName == "alpha_SysJetNP4_Y2012") return "Jet energy scale 4";
if(internalName == "alpha_SysJetNP5_Y2012") return "Jet energy scale 5";
if(internalName == "alpha_SysJetNP6_rest_Y2012") return "Jet energy scale 6 and rest";
//if(internalName == "alpha_SysJetNPV")
if(internalName == "alpha_SysJetNonClos") return "Jet energy scale MC non-closure";
//if(internalName == "alpha_SysJetPilePt_Y2012")
//if(internalName == "alpha_SysJetPileRho_Y2012")
//if(internalName == "alpha_SysLepVeto")
if(internalName == "alpha_SysMETResoSoftTerms_Y2012") return "resolution E_{T}^{miss,SoftTerm}";
if(internalName == "alpha_SysMET_SoftTrk_Scale") return "resolution E_{T}^{miss,SoftTrk Term}";
 if(internalName == "alpha_SysMET_SoftTrk_ResoPara") return "E_{T}^{miss} soft term resolution (parallel)";// #parallel";
 if(internalName == "alpha_SysMET_SoftTrk_ResoPerp") return "E_{T}^{miss} soft term resolution (perp)";// #perp";
if(internalName == "alpha_SysMETScaleSoftTerms_Y2012") return "scale E_{T}^{miss,SoftTerm}";
if(internalName == "alpha_SysMETTrigTop") return "E_{T}^{miss} top trigger";
if(internalName == "alpha_SysMJ_El_METstr") return "Multijet template";
// if(internalName == "alpha_SysMJ_El_flavor") return "Multijet flavor (electron)";
// if(internalName == "alpha_SysMUONS_MS") return "Muon MS";
// if(internalName == "alpha_SysEG_SCALE_ALL") return "Electron scale";
//if(internalName == "alpha_SysMJElCaloIso_T1")
//if(internalName == "alpha_SysMJElCaloIso_T2")
//if(internalName == "alpha_SysMJElDR")
//if(internalName == "alpha_SysMJElNorm_J2_T1")
//if(internalName == "alpha_SysMJElNorm_J2_TTypell")
//if(internalName == "alpha_SysMJElNorm_J2_TTypemm")
//if(internalName == "alpha_SysMJElNorm_J2_TTypett")
//if(internalName == "alpha_SysMJElNorm_J3_T1")
//if(internalName == "alpha_SysMJElNorm_J3_T2")
//if(internalName == "alpha_SysMJElPtV")
//if(internalName == "alpha_SysMJElTrkIso_T1_J2")
//if(internalName == "alpha_SysMJElTrkIso_T1_J3")
//if(internalName == "alpha_SysMJElTrkIso_T2_J2")
//if(internalName == "alpha_SysMJElTrkIso_T2_J3")
//if(internalName == "alpha_SysMJMuNorm_J2_T1")
//if(internalName == "alpha_SysMJMuNorm_J2_TTypell")
//if(internalName == "alpha_SysMJMuNorm_J2_TTypemm")
//if(internalName == "alpha_SysMJMuNorm_J2_TTypett")
//if(internalName == "alpha_SysMJMuNorm_J3_T1")
//if(internalName == "alpha_SysMJMuNorm_J3_T2")
//if(internalName == "alpha_SysMJMuTrkIso_T1_J2")
//if(internalName == "alpha_SysMJMuTrkIso_T1_J3")
//if(internalName == "alpha_SysMJMuTrkIso_T2_J2")
//if(internalName == "alpha_SysMJMuTrkIso_T2_J3")
//if(internalName == "alpha_SysMJ_J2_T1_L0_Y2012")
//if(internalName == "alpha_SysMJ_J2_T1_L0_Y2012_B1")
if(internalName == "alpha_SysMJNorm") return "Multijet normalisation)";
if(internalName == "alpha_SysMJ_J2_T2_L0_Y2012") return "Zerolepton Multijet (2-jet, p_{T}^{V} > 120 GeV)";
if(internalName == "alpha_SysMJ_J2_T2_L0_Y2012_B1") return "Zerolepton Multijet (2-jet, p_{T}^{V} < 120 GeV)";
//if(internalName == "alpha_SysMJ_J3_T1_L0_Y2012")
//if(internalName == "alpha_SysMJ_J3_T2_L0_Y2012")
if(internalName == "alpha_SysMJ_L2_Y2012") return "2-lepton multijet";
//if(internalName == "alpha_SysMuonEffic")
//if(internalName == "alpha_SysSChanAcerMC")
//if(internalName == "alpha_SysSChanAcerMCPS")
if(internalName == "alpha_SysTChanPtB2") return "Single top t-channel acceptance";
if(internalName == "alpha_SysTheoryAccPS") return "Signal acceptance (parton shower)";
if(internalName == "alpha_SysTheoryPDFAccPS") return "VH acc. for PS/UE tunes";
if(internalName == "alpha_SysstoptAcc") return "Single top t-ch. acc.";
if(internalName == "alpha_SysstopWtAcc") return "Singe top Wt acc.";
//if(internalName == "alpha_SysTheoryAccPDF_ggZH")
//if(internalName == "alpha_SysTheoryAccPDF_qqVH")
//if(internalName == "alpha_SysTheoryAcc_J2_ggZH")
//if(internalName == "alpha_SysTheoryAcc_J2_qqVH")
//if(internalName == "alpha_SysTheoryAcc_J3_ggZH")
//if(internalName == "alpha_SysTheoryAcc_J3_qqVH")
//if(internalName == "alpha_SysTheoryBRbb")
//if(internalName == "alpha_SysTheoryPDF_ggZH")
//if(internalName == "alpha_SysTheoryPDF_qqVH")
if(internalName == "alpha_SysTheoryQCDscale_ggZH") return "QCD scale for ggZH";
if(internalName == "alpha_SysTheoryQCDscale_qqVH") return "QCD scale for qqVH";
//if(internalName == "alpha_SysTheoryVHPt")
if(internalName == "alpha_SysTheoryVPtQCD") return "#splitline{signal shape uncertainty}{    from QCD/PDF}";
if(internalName == "alpha_SysTheoryVPtQCD_ggZH") return "#splitline{ggZH shape uncertainty}{    from QCD/PDF}";
if(internalName == "alpha_SysTheoryVPtQCD_qqVH") return "#splitline{qqVH shape uncertainty}{    from QCD/PDF}";
if(internalName == "alpha_SysTopPt") return "t#bar{t} truth avg p_{T} modeling";
if(internalName == "alpha_SysTruthTagDR_Y2012") return "truth tagging DR modeling";
if(internalName == "alpha_SysTtbarMBBCont") return "t#bar{t} m_{jj} shape";
//if(internalName == "alpha_SysTtbarMetCont")
//if(internalName == "alpha_SysVVJetPDFAlphaPt")
//if(internalName == "alpha_SysVVJetScalePtST1")
//if(internalName == "alpha_SysVVJetScalePtST2")
//if(internalName == "alpha_SysVVMbb_WW")
//if(internalName == "alpha_SysVVMbb_WZ")
//if(internalName == "alpha_SysVVMbb_ZZ")
//if(internalName == "alpha_SysWDPhi_J2_Wcl")
//if(internalName == "alpha_SysWDPhi_J2_Whf")
//if(internalName == "alpha_SysWDPhi_J2_Wl")
//if(internalName == "alpha_SysWDPhi_J3_Wcl")
//if(internalName == "alpha_SysWDPhi_J3_Whf")
//if(internalName == "alpha_SysWDPhi_J3_Wl")
if(internalName == "alpha_SysWMbb_B0_WbbORcc"){
    if(isMVA)
        return "#splitline{W+b#bar{b}, W+c#bar{c} m_{jj} shape}{    (p_{T}^{V} < 120 GeV)}";
    else
        return "#splitline{W+b#bar{b}, W+c#bar{c} m_{jj} shape}{    (p_{T}^{V} < 90 GeV)}";
}
if(internalName == "alpha_SysWMbb_B1_WbbORcc") return "#splitline{W+b#bar{b}, W+c#bar{c} m_{jj} shape}{    (90 < p_{T}^{V} < 120 GeV)}";
if(internalName == "alpha_SysWMbb_WbbORcc")    return "#splitline{W+b#bar{b}, W+c#bar{c} m_{jj} shape}{    (p_{T}^{V} > 120 GeV)}";
if(internalName == "alpha_SysWMbb_WbcORbl") return "W+bc, W+bl m_{jj} shape";
//if(internalName == "alpha_SysWMbb_Wcl")
//if(internalName == "alpha_SysWMbb_Wl")
if(internalName == "alpha_SysWPtV_J2_Whf") return "W+HF p_{T}^{V} shape (2-jet) ";
if(internalName == "alpha_SysWPtV_J3_Whf") return "W+HF p_{T}^{V} shape (3-jet) ";
if(internalName == "alpha_SysWbcWbbRatio") return "W+bc to W+b#bar{b} normalisation";
if(internalName == "alpha_SysWblWbbRatio") return "#splitline{W+bl to W+b#bar{b} normalisation}{    (p_{T}^{V} > 120 GeV)}";
if(internalName == "alpha_SysWblWbbRatio_B0"){
    if(isMVA)
        return "#splitline{W+bl to W+b#bar{b} normalisation}{    (p_{T}^{V} < 120 GeV)}";
    else
        return "#splitline{W+bl to W+b#bar{b} normalisation}{    (p_{T}^{V} < 90 GeV)}";
}
if(internalName == "alpha_SysWblWbbRatio_B1") return "#splitline{W+bl to W+b#bar{b} normalisation}{    (90 GeV < p_{T}^{V} < 120 GeV)}";
//if(internalName == "alpha_SysWccWbbRatio")
//if(internalName == "alpha_SysWclNorm_J3")
//if(internalName == "alpha_SysWhfNorm_J3")
//if(internalName == "alpha_SysWlNorm")
//if(internalName == "alpha_SysWlNorm_J3")
//if(internalName == "alpha_SysWtChanAcerMC")
//if(internalName == "alpha_SysWtChanPythiaHerwig")
if(internalName == "alpha_SysZDPhi_J2_ZbORc") return "Z+b#bar{b}, Z+c#bar{c} d#phi shape (2-jet)";
if(internalName == "alpha_SysZDPhi_J2_Zl") return "Z+l d#phi shape (2-jet)";
if(internalName == "alpha_SysZDPhi_J3_ZbORc") return "Z+b#bar{b}, Z+c#bar{c} d#phi shape (3-jet)";
if(internalName == "alpha_SysZDPhi_J3_Zl") return "Z+l d#phi shape (3-jet)";
if(internalName == "alpha_SysZMbb_ZbORc") return "Z+b#bar{b}, Z+c#bar{c} m_{jj} shape";
if(internalName == "alpha_SysZMbb_Zl") return "Z+light m_{jj} shape";
 if(internalName == "alpha_SysZMbb") return "Z+jets m_{bb} shape";
 if(internalName == "alpha_SysWMbb") return "W+jets m_{bb} shape";
 if(internalName == "alpha_SysVVMbbME") return "Diboson m_{bb} shape";
 if(internalName == "alpha_SysTTbarMBB") return "t#bar{t} m_{bb} shape";
 if(internalName == "alpha_SysTTbarMBB_L0") return "0-lepton t#bar{t} m_{bb} shape";
 if(internalName == "alpha_SysTTbarMBB_L2") return "2-lepton t#bar{t} m_{bb} shape";
 if(internalName == "alpha_SysStoptMBB") return "Single top t-ch. m_{bb} shape";
 if(internalName == "alpha_SysStopWtMBB") return "Single top Wt m_{bb} shape";
if(internalName == "alpha_SysZPtV_ZbORc") return "Z+b#bar{b}, Z+c#bar{c} p_{T}^{V}";
if(internalName == "alpha_SysZPtV_Zl") return "Z+light p_{T}^{V}";
 if(internalName == "alpha_SysZPtV") return "Z+jets p_{T}^{V}";
 if(internalName == "alpha_SysWPtV") return "W+jets p_{T}^{V}";
 if(internalName == "alpha_SysTTbarPTV") return "t#bar{t} p_{T}^{V}";
 if(internalName == "alpha_SysTTbarPTV_L2") return "2-lepton t#bar{t} p_{T}^{V}";
 if(internalName == "alpha_SysStoptPTV") return "Single top t-ch. p_{T}^{V}";
 if(internalName == "alpha_SysStopWtPTV") return "Single top Wt p_{T}^{V}";
 if(internalName == "alpha_SysZbbNorm_L0") return "0-lepton Z+HF normalisation";//b#bar{b}
 if(internalName == "alpha_SysZbbNorm_L2") return "2-lepton Z+HF normalisation";//b#bar{b}
 if(internalName == "alpha_SysZbbNorm") return "Z+HF normalisation";//b#bar{b}
 if(internalName == "alpha_SysWbbNorm_L1") return "1-lepton W+HF normalisation";//b#bar{b}
 if(internalName == "alpha_SysWbbNorm") return "W+HF normalisation";//b#bar{b}
 if(internalName == "alpha_SysZZNorm_L0") return "0-lepton ZZ normalisation";
 if(internalName == "alpha_SysZZNorm_L2") return "2-lepton ZZ normalisation";
 if(internalName == "alpha_SysZZNorm") return "ZZ normalisation";
 if(internalName == "alpha_SysWZNorm_L0") return "0-lepton WZ normalisation";
 if(internalName == "alpha_SysWZNorm_L2") return "2-lepton WZ normalisation";
 if(internalName == "alpha_SysWZNorm") return "WZ normalisation";
 if(internalName == "alpha_SysVZNorm_J2") return "VZ normalisation (2-jet)";
 if(internalName == "alpha_SysVZNorm_J3") return "VZ normalisation (3-jet)";
 if(internalName == "alpha_SysVZNorm") return "VZ normalisation";
 if(internalName == "alpha_SysZbbNorm_J2") return "Z+HF normalisation (2-jet)";//b#bar{b}
 if(internalName == "alpha_SysZbbNorm_J3") return "Z+HF normalisation (3-jet)";//b#bar{b}
 if(internalName == "alpha_SysWbbNorm_J2") return "W+HF normalisation (2-jet)";//b#bar{b}
 if(internalName == "alpha_SysWbbNorm_J3") return "W+HF normalisation (3-jet)";//b#bar{b}
 if(internalName == "alpha_SysWbbNorm") return "W+HF normalisation";//b#bar{b}
if(internalName == "alpha_SysZbcZbbRatio") return "Z+bc to Z+b#bar{b} normalisation";
if(internalName == "alpha_SysZblZbbRatio") return "Z+bl to Z+HF normalisation";
if(internalName == "alpha_SysZblZbbRatio_J2") return "Z+bl to Z+b#bar{b} normalisation (2-jet)";
if(internalName == "alpha_SysZblZbbRatio_J3") return "Z+bl to Z+b#bar{b} normalisation (3-jet)";
if(internalName == "alpha_SysZccZbbRatio") return "Z+bc to Z+b#bar{b} normalisation";
if(internalName == "alpha_SysZclNorm") return "Z+cl normalisation";
//if(internalName == "alpha_SysZlNorm")
//if(internalName == "alpha_SysZlNorm_J3")
//if(internalName == "alpha_SysstopWtNorm")
//if(internalName == "alpha_SysstopsNorm")
//if(internalName == "alpha_SysstoptNorm")
if(internalName == "alpha_SysttbarHighPtV") return "t#bar{t} high p_{T}^{V} normalisation";
if(internalName == "alpha_SysttbarNorm_J3") return "t#bar{t} normalisation (3-jet)";
if(internalName == "alpha_SysttbarNorm_J3_L2") return "t#bar{t} normalisation (3-jet, dilepton)";
if(internalName == "alpha_SysMJ_L2_Y2012_Spctopemucr") return "2-lepton Multijet (top C.R.)";
    return internalName;
}
