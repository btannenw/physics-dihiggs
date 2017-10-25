
#include "TString.h"
#include "TFile.h"
#include "RooFitResult.h"
#include "TMatrixDSym.h"
#include "TMatrixDSymEigen.h"
#include "TVectorT.h"
#include "TMatrixT.h"
#include "RooArgList.h"
#include "TIterator.h"
#include "RooRealVar.h"

#include "iostream"
#include "iomanip"

using namespace std;

void readEVsFromFCC(TString WSName, bool asimov = false) {

  // open file and get fit results

  TString fileName = "FitCrossChecks_" + WSName + "_combined/FitCrossChecks.root";
  TFile* file = TFile::Open(fileName, "read");
  if (file) {
    cout << "INFO: file opened: " << fileName.Data() << endl;
  } else {
    cout << "ERROR: could not open file: " << fileName.Data() << endl;
    return;
  }
  TString resultName = "PlotsAfterGlobalFit/unconditionnal/fitResult";
  if (asimov) {
    resultName = "PlotsAfterFitToAsimov/unconditionnal/fitResult";
  }
  RooFitResult* tmpResult = (RooFitResult*) file -> Get(resultName);
  if (tmpResult) {
    cout << "INFO: found fit result: " << resultName.Data() << endl;
  } else {
    cout << "ERROR: could not find fit result: " << resultName.Data() << endl;
    return;
  }

  // config for printout

  double minEigenValue = 0.6;
  double minEigenVecVal = 0.05;
  int maxNEigenValues = 10;

  cout << "minEigenValue   = " << minEigenValue << endl;
  cout << "minEigenVecVal  = " << minEigenVecVal << endl;
  cout << "maxNEigenValues = " << maxNEigenValues << endl;

  // get parameters, covariance matrix, ...

  const RooArgList& parFinal = tmpResult->floatParsFinal();
  //TIterator* it = parFinal.createIterator();
  //while( RooRealVar* o = (RooRealVar*)(it->Next()) ) {
  //cout << "VarName = " << o->GetName() << endl;
  //}

  const TMatrixDSym covarMat = tmpResult->covarianceMatrix();
  TMatrixDSymEigen eigenValueMaker(covarMat);
  TVectorT<double> eigenValues   = eigenValueMaker.GetEigenValues();
  TMatrixT<double> eigenVectors  = eigenValueMaker.GetEigenVectors();

  int N = eigenValues.GetNrows();
  Double_t det = covarMat.Determinant();

  if (parFinal.getSize() != N) {
    cout << "ERROR: matrix size does not match number of fit parameters!" << endl;
    return;
  }

  cout << endl;
  cout << "Parameter   = " << N << endl;
  cout << "Determinant = " << det << endl;
  cout << endl;

  // list eigenvalues and corresponding eigenvectors (above thresholds)

  for (int l = 0; l < N; l++) { 
    if (eigenValues[l] < minEigenValue || l >= maxNEigenValues) break;
    cout << "Eigenvalue " << l << " = " << eigenValues[l] << endl;
    double curr = 1e99;
    while (fabs(curr) >= minEigenVecVal) {
      double max = 0;
      double maxInd = -1;
      for(int m = 0; m < N; m++) {
	double val = eigenVectors[l][m];
	if (fabs(val) < fabs(curr) && fabs(val) >= fabs(max)) {
	  max = val;
	  maxInd = m;
	}
      }
      if (maxInd < 0) break;
      curr = eigenVectors[l][maxInd];
      // FIXME: is it really reverse ordered??
      TString parName = parFinal.at(N - maxInd - 1) -> GetName();
      TString currString = TString::Format("%.3f", curr);
      cout << setw(70) << parName.Data();
      cout << setw(10) << currString.Data() << endl;
    }
  }

  //eigenVectors.Draw();
}
