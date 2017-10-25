#include "roofitUtils.hpp"

#include <iostream>
#include <vector>

#include <TMatrixTSym.h>
#include <TVectorT.h>

#include <RooArgList.h>
#include <RooAbsArg.h>
#include <RooRealVar.h>
#include <RooAbsReal.h>
#include <RooPlot.h>

namespace RU {

  Double_t  getPropagatedError(RooAbsReal& var, const RooFitResult& fr, const RooArgSet* params, bool linMethod) {
    // Calculate error on self by propagated errors on parameters with correlations as given by fit result
    // The linearly propagated error is calculated as follows
    //                                    T
    // error(x) = F_a(x) * Corr(a,a') F_a'(x)
    //
    // where     F_a(x) = [ f(x,a+da) - f(x,a-da) ] / 2, with f(x) this function and 'da' taken from the fit result
    //       Corr(a,a') = the correlation matrix from the fit result
    //

    if(!linMethod) {
      std::cout << "Sampling method not implemented yet ! Use linear propagation." << std::endl;
    }

    RooAbsReal* cloneFunc =  &var ;
    RooArgSet* cloneParams = cloneFunc->getObservables(fr.floatParsFinal()) ;
    RooArgSet* errorParams = params?((RooArgSet*)cloneParams->selectCommon(*params)):cloneParams ;
    RooArgSet* nset = cloneFunc->getParameters(*cloneParams) ;

    // Make list of parameter instances of cloneFunc in order of error matrix
    RooArgList paramList ;
    const RooArgList& fpf = fr.floatParsFinal() ;
    std::vector<int> fpf_idx ;
    for (Int_t i=0 ; i<fpf.getSize() ; i++) {
      RooAbsArg* par = errorParams->find(fpf[i].GetName()) ;
      if (par) {
        paramList.add(*par) ;
        fpf_idx.push_back(i) ;
      }
    }

    std::vector<Double_t> plusVar, minusVar ;

    // Create vector of plus,minus variations for each parameter
    TMatrixDSym V(paramList.getSize()==fr.floatParsFinal().getSize()?
                  fr.covarianceMatrix():
                  fr.reducedCovarianceMatrix(paramList)) ;

    for (Int_t ivar=0 ; ivar<paramList.getSize() ; ivar++) {

      RooRealVar& rrv = (RooRealVar&)fpf[fpf_idx[ivar]] ;

      Double_t cenVal = rrv.getVal() ;
      Double_t errVal = sqrt(V(ivar,ivar)) ;

      // Make Plus variation
      ((RooRealVar*)paramList.at(ivar))->setVal(cenVal+errVal) ;
      plusVar.push_back(cloneFunc->getVal(nset)) ;

      // Make Minus variation
      ((RooRealVar*)paramList.at(ivar))->setVal(cenVal-errVal) ;
      minusVar.push_back(cloneFunc->getVal(nset)) ;

      ((RooRealVar*)paramList.at(ivar))->setVal(cenVal) ;
    }

    TMatrixDSym C(paramList.getSize()) ;
    std::vector<double> errVec(paramList.getSize()) ;
    for (int i=0 ; i<paramList.getSize() ; i++) {
      errVec[i] = sqrt(V(i,i)) ;
      for (int j=i ; j<paramList.getSize() ; j++) {
        C(i,j) = V(i,j)/sqrt(V(i,i)*V(j,j)) ;
        C(j,i) = C(i,j) ;
      }
    }

    // Make vector of variations
    TVectorD F(plusVar.size()) ;
    for (unsigned int j=0 ; j<plusVar.size() ; j++) {
      F[j] = (plusVar[j]-minusVar[j])/2 ;
    }

    // Calculate error in linear approximation from variations and correlation coefficient
    Double_t sum = F*(C*F) ;

    delete errorParams ;
    delete nset ;

    return sqrt(sum) ;
  }


  RooCurve* plotOnWithErrorBand(const std::vector<RooAbsPdf*>& pdfs, const std::vector<RooRealVar*>& observables,
                                const std::vector<double>& weights, const RooFitResult& fr,
                                const std::vector<RooAbsData*>& data, const RooArgSet* params)
  {
    // Plot function or p.d.f. on frame with support for visualization of the uncertainty encoded in the given fit result fr.
    // If params is non-zero, only the subset of the parameters in fr that occur in params is considered for the error evaluation
    // Argument argList can contain any RooCmdArg named argument that can be applied to a regular plotOn() operation
    //
    // By default (linMethod=kTRUE) a linearized error is shown which is calculated as follows
    //                                    T
    // error(x) = Z* F_a(x) * Corr(a,a') F_a'(x)
    //
    // where     F_a(x) = [ f(x,a+da) - f(x,a-da) ] / 2, with f(x) the plotted curve and 'da' taken from the fit result
    //       Corr(a,a') = the correlation matrix from the fit result
    //                Z = requested signifance 'Z sigma band'
    //
    // The linear method is fast (required 2*N evaluations of the curve, where N is the number of parameters), but may
    // not be accurate in the presence of strong correlations (~>0.9) and at Z>2 due to linear and Gaussian approximations made
    //


    if(pdfs.size() != observables.size()) {
      std::cout << "ERROR::plotOnWithErrorBand: input PDF and Observables vectors have different sizes !" << std::endl;
      return 0;
    }

    if(pdfs.size() != data.size()) {
      std::cout << "ERROR::plotOnWithErrorBand: input PDF and data vectors have different sizes !" << std::endl;
      return 0;
    }

    int nPdfs = pdfs.size();

    if(pdfs.size() == 0) {
      std::cout << "ERROR::plotOnWithErrorBand: hold on a second, why are you sending empty inputs ?" << std::endl;
      return 0;
    }

    if(pdfs.size() == 1) {
      std::cout << "WARNING::plotOnWithErrorBand: please do not use the complicated way to do simple things !" << std::endl;
      std::cout << "Anyway, I'll take care." << std::endl;
      RooPlot* frm = observables[0]->frame();
      data[0]->plotOn(frm);
      pdfs[0]->plotOn(frm, RooFit::VisualizeError(fr, 1), RooFit::Normalization(1, RooAbsReal::RelativeExpected));
      RooCurve* c = (RooCurve*)(frm->getCurve()->Clone());
      delete frm;
      return c;
    }

    std::vector<RooPlot*> frames;
    std::vector<RooCurve*> cenCurves;
    for(int i=0; i<nPdfs; i++) {
      RooPlot* frame = observables[i]->frame();
      frames.push_back(frame);
      data[i]->plotOn(frame);
      pdfs[i]->plotOn(frame, RooFit::Normalization(1, RooAbsReal::RelativeExpected)) ;
      cenCurves.push_back(frame->getCurve());
    }

    // Generate central value curve
    RooCurve* cenCurve = addCurves(cenCurves, weights);
    for(int i=0; i<nPdfs; i++) {
      frames[i]->remove(0, true);
    }

    RooCurve* band(0) ;

    // *** Linear Method ***

    // Clone self for internal use
    RooArgSet* cloneParams = new RooArgSet();
    for(int i=0; i<nPdfs; i++)
      cloneParams->add(*(pdfs[i]->getObservables(fr.floatParsFinal())));
    RooArgSet* errorParams = params?((RooArgSet*)cloneParams->selectCommon(*params)):cloneParams ;


    // Make list of parameter instances of cloneFunc in order of error matrix
    RooArgList paramList ;
    const RooArgList& fpf = fr.floatParsFinal() ;
    std::vector<int> fpf_idx ;
    for (Int_t i=0 ; i<fpf.getSize() ; i++) {
      RooAbsArg* par = errorParams->find(fpf[i].GetName()) ;
      if (par) {
        paramList.add(*par) ;
        fpf_idx.push_back(i) ;
      }
    }

    std::vector<RooCurve*> plusVar, minusVar ;

    // Create vector of plus,minus variations for each parameter

    TMatrixDSym V(paramList.getSize()==fr.floatParsFinal().getSize()?
                  fr.covarianceMatrix():
                  fr.reducedCovarianceMatrix(paramList)) ;

    for (Int_t ivar=0 ; ivar<paramList.getSize() ; ivar++) {

      RooRealVar& rrv = (RooRealVar&)fpf[fpf_idx[ivar]] ;

      Double_t cenVal = rrv.getVal() ;
      Double_t errVal = sqrt(V(ivar,ivar)) ;

      std::vector<RooCurve*> plusVars, minusVars ;

      // Make Plus variation
      ((RooRealVar*)paramList.at(ivar))->setVal(cenVal+errVal) ;
      for(int i=0; i<nPdfs; i++) {
        pdfs[i]->plotOn(frames[i], RooFit::Normalization(1, RooAbsReal::RelativeExpected)) ;
        plusVars.push_back(frames[i]->getCurve());
      }
      plusVar.push_back(addCurves(plusVars, weights));
      for(int i=0; i<nPdfs; i++) {
        frames[i]->remove(0, true);
      }

      // Make Minus variation
      ((RooRealVar*)paramList.at(ivar))->setVal(cenVal-errVal) ;
      for(int i=0; i<nPdfs; i++) {
        pdfs[i]->plotOn(frames[i], RooFit::Normalization(1, RooAbsReal::RelativeExpected)) ;
        minusVars.push_back(frames[i]->getCurve());
      }
      minusVar.push_back(addCurves(minusVars, weights));
      for(int i=0; i<nPdfs; i++) {
        frames[i]->remove(0, true);
      }

      ((RooRealVar*)paramList.at(ivar))->setVal(cenVal) ;

    }

    TMatrixDSym C(paramList.getSize()) ;
    std::vector<double> errVec(paramList.getSize()) ;
    for (int i=0 ; i<paramList.getSize() ; i++) {
      errVec[i] = sqrt(V(i,i)) ;
      for (int j=i ; j<paramList.getSize() ; j++) {
        C(i,j) = V(i,j)/sqrt(V(i,i)*V(j,j)) ;
        C(j,i) = C(i,j) ;
      }
    }

    band = cenCurve->makeErrorBand(plusVar,minusVar,C,1) ;

    for (std::vector<RooCurve*>::iterator i=plusVar.begin() ; i!=plusVar.end() ; i++) {
      delete (*i) ;
    }
    for (std::vector<RooCurve*>::iterator i=minusVar.begin() ; i!=minusVar.end() ; i++) {
      delete (*i) ;
    }

    delete cenCurve ;
    if (!band) {
      std::cout << "ERROR::plotOnWithErrorBand: a problem has occurred somewhere. Return 0." << std::endl;
      return 0;
    }

    for(int i=0; i<nPdfs; i++) {
      delete frames[i];
    }

    return band ;
  }

  RooCurve* addCurves(const std::vector<RooCurve*>& added, const std::vector<double>& weights) {
    int ncurves = added.size();
    if(ncurves == 0){
      std::cout << "ERROR::addCurves: must provide at least 1 curve !" << std::endl;
      return 0;
    }

    int csize = added[0]->GetN();

    if(ncurves == 1){
      RooCurve* newcurve = (RooCurve*)(added[0]->Clone());
      double* ysum = newcurve->GetY();
      for(int j=0; j<csize; j++) {
        ysum[j] *= weights[0];
      }
      return newcurve;
    }

    if(weights.size() != ncurves) {
      std::cout << "ERROR::addCurves: numbers of curves and weights differ !" << std::endl;
      return 0;
    }

    for(int i=0; i<ncurves; i++) {
      if(added[i]->GetN() != csize) {
        std::cout << "ERROR::addCurves: All curves don't have the same size !" << std::endl;
        std::cout << "Check your original binnings." << std::endl;
        return 0;
      }
    }

    // now we have done all the checks. Can start adding things.
    // it should be as simple as the following
    RooCurve* sum = (RooCurve*)(added[0]->Clone());
    double* ysum = sum->GetY();
    for(int j=0; j<csize; j++) {
      ysum[j] *= weights[0];
    }
    for(int i=1; i<ncurves; i++) {
      double* toAdd = added[i]->GetY();
      for(int j=0; j<csize; j++)
        ysum[j] += toAdd[j] * weights[i];
    }

    return sum;
  }

  void addCurves(RooCurve* sum, const RooCurve* added) {} // FIXME implement if needed

}
