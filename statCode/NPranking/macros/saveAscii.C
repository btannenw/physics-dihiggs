#include <set>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <vector>

#include "TFile.h"
#include "TH1D.h"
#include "TMath.h"
#include "Math/ProbFuncMathCore.h"

using namespace std;

// mode
// 0 = sig
// 1 = limit
// 2 = muhat
// 3 = cls
// 4 = llr
// 5 = hypo_tev
// 6 = hypo_llr
// 7 = CLsb (mu_up)
// 8 = CLb( mu_up)
// 9 = CLsb (mu=1)
// 10 = CLb (mu=1)

void saveAscii(string folder)
{
  bool useOld = 0;

  system("mkdir -vp ascii");

  bool ignoreInfNan = 1;

  set<double> massPoints;
  // for (int i=0;i<100;i++)
  // {
  //   massPoints.insert(i);
  // }

  // double counter = 110;
  double counter = 90;
  while (counter < 120)
  {
    massPoints.insert(counter);
    counter+=1;
  }
  while (counter < 130)
  {
    massPoints.insert(counter);
    counter+=0.5;
  }
  while (counter < 150)
  {
    massPoints.insert(counter);
    counter+=1;
  }
  while (counter < 290)
  {
    massPoints.insert(counter);
    counter+=2;
  }
  while (counter < 350)
  {
    massPoints.insert(counter);
    counter+=5;
  }
  while (counter < 400)
  {
    massPoints.insert(counter);
    counter+=10;
  }
  while (counter <= 600)
  {
    massPoints.insert(counter);
    counter+=20;
  }
  massPoints.insert(155);
  massPoints.insert(165);
  massPoints.insert(175);
  massPoints.insert(185);
  massPoints.insert(195); 

  stringstream outFileName;
  outFileName << "ascii/" << folder << ".txt";
  ofstream outFile(outFileName.str().c_str());
  int nrPoints = massPoints.size();
  vector<double> infMasses;
  vector<double> nanMasses;
  //cout << "nrpoints = " << massPoints.size() << endl;
  for (set<double>::iterator mass = massPoints.begin();mass!=massPoints.end();mass++)
  {
    stringstream fileName;
    fileName << "root-files/" << folder << "/" << *mass << ".root";
    //cout << "fileName: " << fileName.str() << endl;
    bool success = true;
    if (system(("ls " + fileName.str() + " > /dev/null 2>&1").c_str())) success = false;
    
    TFile* f = NULL;
    if (success) f = new TFile(fileName.str().c_str());
    if (f && !f->IsOpen()) success = false;

    if (success) cout << "O";
    else cout << "X";
    cout << " File: " << fileName.str() << endl;
    if (!success) continue;

    TH1D* hist;
    int mode = 0;
    hist = (TH1D*)f->Get("hypo");
    if (!hist)
    {
      hist = useOld ? (TH1D*)f->Get("limit_old") : (TH1D*)f->Get("limit");
      mode = 1;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("muhat");
      mode = 2;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("cls");
      mode = 3;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("llr");
      mode = 4;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("hypo_tev");
      mode = 5;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("hypo_llr");
      mode = 6;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("clsb");
      mode = 7;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("clb");
      mode = 8;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("pv_clsb");
      mode = 9;
    }
    if (!hist)
    {
      hist = (TH1D*)f->Get("pv_clb");
      mode = 10;
    }
    if (!hist)
    {
      cout << "ERROR::Couldn't find hist" << endl;
    }
      
    // cout << "mode = " << mode << endl;

    int nrBins = hist->GetNbinsX();
    if (mode == 1) nrBins--;

    bool isInfNan = false;
    vector<double> numbers;
    numbers.push_back(*mass);

    for (int bin=1;bin<=nrBins;bin++)
    {
      double number = hist->GetBinContent(bin);
      if (number > 10e9) // check inf
      {
        infMasses.push_back(*mass);
        number = 1;
        if (ignoreInfNan) 
        {
          isInfNan = true;
        }
      }
      if (number != number) // check nan
      {
        nanMasses.push_back(*mass);
        number = 1;
        if (ignoreInfNan) 
        {
          isInfNan = true;
        }
      }

      if (mode == 0)
      {
        number = 1-ROOT::Math::gaussian_cdf(number);
      }
      numbers.push_back(number);
    }
    if (isInfNan && ignoreInfNan) continue;

    nrBins=numbers.size();
    for (int bin=0;bin<nrBins;bin++)
    {
      outFile << numbers[bin];
      if (bin < nrBins-1)
      {
        outFile << " ";
      }
      else
      {
        outFile << "\n";
      }
    }
    f->Close();
  }
  for (int i=0;i<(int)infMasses.size();i++)
  {
    cout << "WARNING::mass " << infMasses[i] << " gave inf" << endl;
  }
  for (int i=0;i<(int)nanMasses.size();i++)
  {
    cout << "WARNING::mass " << nanMasses[i] << " gave nan" << endl;
  }

  cout << "Writing to file: " << outFileName.str() << endl;
  outFile.close();
}
