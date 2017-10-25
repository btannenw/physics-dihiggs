#include <HistoTransform.C>

using namespace std;

void transform(TString inFileName, TString outFileName = "") {

  /* This is an example of using the HistoTransform Class. You can compile and
   * run this script e.g. with:
   *
   * root 'transform.C+("pathToLimitHistograms/LimitHistograms.llbb.8TeV.MVA_mH125.LiverpoolBmham.v10.root")'
   *
   */

  /* Create HistoTransform instance.
   * The transformation is built on the background defined below.
   * The histogram names are assumed to be of the form:
   *      sample_region[_systematic_[IofK]]
   * Examples:
   *      Wb_2jet2tag_mva
   *      Wb_2jet2tag_mva_0of2
   *      Wb_2jet2tag_mva_SysBTagB0EfficDo_0of2
   * The samples and systematics do not have to be defined,
   * they will be searched for.
   */
  HistoTransform histoTrafo(inFileName, outFileName);

  /* Define if the background BDTs should be transformed. A distinct transformation
   * for each bkg BDT is formed.
   * Default is true.
   */
  histoTrafo.transformBkgBDTs = true;

  /* Define if the kFolds should be merged.
   * Default is false. Not supported at the moment!
   */
  histoTrafo.doMergeKFolds = false;

  /* Define if the histograms should be transformed before the kFolds
   * are merged.
   * Technical note: The transformation is applied in any case before merging,
   * but the transformation is built from the merged background when this is set to true.
   * Default is false.
   */
  histoTrafo.doTransformBeforeMerging = false;

  /* Choose the algorithm for transformation.
   * Available are:
   * 1: bkgUnc < maxUnc
   * 5: optimized for signal bins
   * 6: "trafoD"
   * 11: "trafoE"
   * 12: "trafoF"
   * Default is 6
   */

  histoTrafo.transformAlgorithm = 6;


  /* Add subdirectories, e.g. "OneLepton".
   * If the histograms are not stored in a subdirectory add one named "".
   * If no subdirectory is defined, those containing "Lepton" will be searched for
   * and the a histogram of name "bkg" is used as total background.
   */

  //HistoTransform::SubDirectory* subDir = histoTrafo.addSubDirectory("OneLepton");
  HistoTransform::SubDirectory* subDir = histoTrafo.addSubDirectory("");

  /* Define backgrounds for building the transformation for each subdirectory.
   */

  histoTrafo.addBackground(subDir, "bkg");

  //  histoTrafo.addBackground(subDir, "WW");
  //  histoTrafo.addBackground(subDir, "WZ");
  //  histoTrafo.addBackground(subDir, "ZZ");
  //  histoTrafo.addBackground(subDir, "Wbb");
  //  histoTrafo.addBackground(subDir, "Wbl");
  //  histoTrafo.addBackground(subDir, "Wcc");
  //  histoTrafo.addBackground(subDir, "Wcl");
  //  histoTrafo.addBackground(subDir, "Wl");
  //  histoTrafo.addBackground(subDir, "multijet");
  //  histoTrafo.addBackground(subDir, "Zbb");
  //  histoTrafo.addBackground(subDir, "Zbl");
  //  histoTrafo.addBackground(subDir, "Zcc");
  //  histoTrafo.addBackground(subDir, "Zcl");
  //  histoTrafo.addBackground(subDir, "Zl");
  //  histoTrafo.addBackground(subDir, "ttbar");
  //  histoTrafo.addBackground(subDir, "stop_s");
  //  histoTrafo.addBackground(subDir, "stop_t");
  //  histoTrafo.addBackground(subDir, "stop_Wt");

  /* Set the signal histogram name. Needed for algorithm 5 and 6.
   * Only one signal supported at the moment. */
  histoTrafo.setSignal(subDir, "ZllH125");

  /* Set the Y (= NBins for signal) for algorithm 5. Default is 5. */
  //histoTrafo.trafoFiveY = 5.;

  /* Set the number of bins for trafoD (algorithm 6). Default is 10 + 10. */
  histoTrafo.trafoSixY = 10.; // also called z_sig
  histoTrafo.trafoSixZ = 10.; // also called z_bkg

  /* Set the number of bins for algorithm 7. Default is 20. */
  //histoTrafo.trafoSevenNBins = 20;

  /* Set the number of bins for trafoE (algorithm 11). Default is 16 + 4. */
  histoTrafo.trafoEzSig = 16.;
  histoTrafo.trafoEzBkg = 4.;

  /* Set the number of bins for trafoF (algorithm 12). Default is 5 + 1. */
  histoTrafo.trafoFzSig = 5.;
  histoTrafo.trafoFzBkg = 1.;

  /* Add regions, e.g. "_2tag2jet_mva" to each subdirectory.
   * For each region a maximum uncertainty on the background stats can be defined,
   * default is 0.05 = 5%.
   * Kfolds with K > 1 can be added by providing the nFold (= K) argument.
   * The suffix _IofK (e.g. _0of2) will be added automatically to the histogram names.
   * If transformation is applied before merging of the kFolds, the max. unc. refers to
   * the background of each fold, otherwise to the combined background.
   * The maxUncFactor can be used to reach about the same number of bins in both cases.
   * 
   * WARNING: kFolds NOT supported with the current file format!
   */

  //  int nFold = 2;
  //  float maxUncFactor = 1;
  //  if (histoTrafo.doTransformBeforeMerging)
  //    maxUncFactor = sqrt(nFold);

  //  histoTrafo.addRegion(subDir, "pretag2jet_mva", 0.02);
  //  histoTrafo.addRegion(subDir, "0tag2jet_mva", 0.02);
  //histoTrafo.addRegion(subDir, "1tag2jet_mva", 0.03 * maxUncFactor, nFold);
  //histoTrafo.addRegion(subDir, "2tag2jet_mva", 0.05 * maxUncFactor, nFold);
  //  histoTrafo.addRegion(subDir, "2tag3jet_mva", 0.05);

  histoTrafo.run();

  cout << "Done!" << endl;
  gSystem -> Exit(0);

}
