#ifndef roofitUtils_hpp
#define roofitUtils_hpp

#include <vector>

#include <RooAbsReal.h>
#include <RooFitResult.h>
#include <RooArgSet.h>
#include <RooCurve.h>
#include <RooAbsPdf.h>
#include <RooRealVar.h>
#include <RooAbsData.h>

/**
 * @file roofitUtils.hpp
 * @brief A few functions used for postfit plots
 *
 * They compensate for featuers that are strangely missing by default in RooFit
 */

namespace RU {

  /**
   * @brief Propagate the error on a variable from results stored in a @c RooFitResult
   *
   * Do error propagation from a set of parameters to a variable.
   *
   * This function augments the standard function with the same name in @c
   * RooFit::RooAbsReal, with the option (that exist only in plotOn for no reason) to limit
   * the propagation to a subset of the parameters in the fit result.
   *
   * @param var	Variable on which to compute the error
   * @param fr  RooFitResult that stores the errors and the correlations between variables
   * which @c var depends on
   * @param params Set of parameters present in var that the error propagation should be restricted to
   * @param linMethod Use linear propagation (@c true) or sampling method (@c false). ONLY the linear
   * propagation is implemented at the moment
   * @return Error on @c var
   */
  Double_t  getPropagatedError(RooAbsReal& var, const RooFitResult& fr, const RooArgSet* params = 0, bool linMethod = true);

  /**
   * @brief Compute the error band on the sum of PDFs with similar x-axis
   *
   * This function has the same interface as the one in RooFit, except that it takes vectors of
   * PDFs, observables, and data, to compute the total error curve.
   *
   * Basically:
   * * for each parameter in the fit result, compute the error curve on each pdf
   * * add the curves linearly and store the result
   * * at the end, add the curves in quadrature
   *
   * This method works but it is slow. It has been found quicker to compute the error bin-by-bin
   * by calling @c getPropagatedError on @c RooAddition of the results of @c createIntegral on bins
   * that have to be merged together.
   *
   * It is this second method that is used in practice in @c doPlotFromWS.py
   */
  RooCurve* plotOnWithErrorBand(const std::vector<RooAbsPdf*>& pdfs, const std::vector<RooRealVar*>& observables,
                                const std::vector<double>& weights, const RooFitResult& fr,
                                const std::vector<RooAbsData*>& data, const RooArgSet* params = 0);

  /// Not implemented
  void addCurves(RooCurve* sum, const RooCurve* added);

  /**
   * @brief Perform a weighted linear sum of several curves
   *
   * Bin-by-bin weighted sum of curves
   *
   * @param added Set of curves to add together
   * @param weights Corresponding set of weights to apply to the curves
   * @return The sum of curves*weights
   */
  RooCurve* addCurves(const std::vector<RooCurve*>& added, const std::vector<double>& weights);

}


#endif
