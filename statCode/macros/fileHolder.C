#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <set>

using namespace std;

struct fileHolder
{

  fileHolder()
  :
  nrCols(8),
  skipFirstLine(false),
  useStrings(false)
  {

  }

  void setUseStrings(bool flag=true) {useStrings=flag;}

  void setNrCols(int n) {nrCols = n;}

  bool readInFile(string name)
  {
    cout << "Reading in file: " << name << endl;
    ifstream inFile(name.c_str());
    _name=name;
    if (inFile.fail())
    {
      cout << "ERROR::Couldn't open file: " << name << endl;
      return false;
    }

    if (skipFirstLine)
    {
      string junk;
      getline(inFile, junk);
    }

    int nrItr = 0;
    while (!inFile.eof())
    {
      double mass;
      string massS;

      if (useStrings)
      {
        inFile >> massS;
      }
      else
      {
        inFile >> mass;
      }

      if (inFile.eof()) break;
      //cout << "Checking: " << mass << endl;
      bool fill=false;
      if (!massPoints.size()/*bug in inputs*/) fill=true;
      else if (mass != massPoints.back()) fill=true;
     //cout << "Passed" << endl;

      vector<double> numbers;
      double number;
      for (int i=0;i<nrCols;i++)
      {
        inFile >> number;
        numbers.push_back(number);
      }

      if (fill)
      {
        massPoints.push_back(mass);
        rates.push_back(numbers);
      }

      nrItr++;
      //cout << "mass = " << mass << endl;
      if (nrItr > 500) 
      {
        cout << "ERROR::Line overflow detected. Exiting." << endl;
        return false;
      }
    }
    cout << "Done" << endl;
    inFile.close();
    return true;
  }

  void writeOutFile(string name)
  {
    cout << "Writing file: " << name << endl;
    ofstream outFile(name.c_str());
    if (outFile.fail())
    {
      cout << "Error writing to file: " << name << endl;
      return;
    }

    int nrPoints = massPoints.size();
    for (int i=0;i<nrPoints;i++)
    {
      //cout << "adding point: " << massPoints[i] << endl;
      outFile << massPoints[i];
      for (int j=0;j<nrCols;j++)
      {
        //cout << "->rate=" << rates[i][j] << endl;
        outFile << " " << rates[i][j];
      }
      outFile << "\n";
    }
    outFile.close();
  }

  vector<double> getCol(int col)
  {
    vector<double> vec_col;
    int nrPoints = massPoints.size();
    for (int i=0;i<nrPoints;i++)
    {
      vec_col.push_back(rates[i][col]);
    }
    return vec_col;
  }

  double getRate(int row, int col)
  {
    return rates[row][col];
  }

  double getRateByMass(double mass, int col)
  {
    int nrPoints = massPoints.size();
    for (int im=0;im<nrPoints;im++)
    {
      if (mass == massPoints[im])
      {
        return rates[im][col];
      }
    }
    return 0.;
  }

  void setRate(double mass, int col, double val)
  {
    for (int imass=0;imass<(int)massPoints.size();imass++)
    {
      if (mass == massPoints[imass])
      {
        rates[imass][col] = val;
        break;
      }
    }
  }

  void copy(fileHolder& alt)
  {
    alt.nrCols=nrCols;
    alt.rates=rates;
    alt.massPoints=massPoints;
  }

  void removeMass(double mass)
  {
    vector<double> newPoints;
    vector<vector<double> > newRates;
    int nrPoints = massPoints.size();
    for (int i=0;i<nrPoints;i++)
    {
      if (massPoints[i] == mass) continue;
      newPoints.push_back(massPoints[i]);
      newRates.push_back(rates[i]);
    }
    massPoints = newPoints;
    rates = newRates;
  }

  void addMass(double mass, vector<double>& rate)
  {
    //cout << "adding mass: " << mass << endl;
    vector<double> newPoints;
    vector<vector<double> > newRates;

    bool found = false;
    int nrPoints = massPoints.size();
    for (int i=0;i<nrPoints;i++)
    {
      if ((massPoints[i] < mass && !found) || (massPoints[i] > mass && found))
      {
        newPoints.push_back(massPoints[i]);
        newRates.push_back(rates[i]);
      }
      else if (massPoints[i] > mass && !found) 
      {
        //cout << "found" << endl;

        newPoints.push_back(mass);
        newRates.push_back(rate);

        newPoints.push_back(massPoints[i]);
        newRates.push_back(rates[i]);

        found = true;
      }
    }
    massPoints = newPoints;
    rates = newRates;
  }

  void addFile(fileHolder& other)
  {
    vector<double> otherPoints = other.massPoints;
    vector<vector<double> > otherRates = other.rates;
    for (int i=0;i<(int)otherPoints.size();i++)
    {
      addMass(otherPoints[i], otherRates[i]);
    }
  }

  void addCol(vector<double>& addRates, int col)
  {
    int nrPoints = massPoints.size();
    for (int i=0;i<nrPoints;i++)
    {
      vector<double> newRates;
      for (int j=0;j<col;j++)
      {
        newRates.push_back(rates[i][j]);
      }
      newRates.push_back(addRates[i]);
      for (int j=col;j<(int)rates[i].size();j++)
      {
        newRates.push_back(rates[i][j]);
      }
      rates[i]=newRates;
    }
    nrCols++;
  }

  void useMedian(int col)
  {
    //cout << "in use median" << endl;
    set<double> vals;
    int nrRates = rates.size();
    for (int i=0;i<nrRates;i++)
    {
      vals.insert(rates[i][col]);
    }
    set<double>::iterator itr=vals.begin();
    int nrVals = vals.size();
    for (int i=0;i<nrVals/2;i++)
    {
      itr++;
    }

    for (int i=0;i<nrRates;i++)
    {
      rates[i][col] = *itr;
    }
    //cout << "out use median" << endl;
  }

  void skipFirst() {skipFirstLine=true;}

  int nrCols;
  bool skipFirstLine;
  vector<vector<double> > rates; // row: mass, col: sample
  vector<double> massPoints;
  vector<string> massPointsS;
  string _name;
  bool useStrings;
};
