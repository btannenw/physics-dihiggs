{

  gROOT->ProcessLine(".L FitCrossCheckForLimits.C+");
  gROOT->ProcessLine(".L runAsymptoticsCLs.C+");
  gROOT->ProcessLine(".L newGetMuHat.C+");
  gROOT->ProcessLine(".L runSig.C+");
  gROOT->ProcessLine(".L macros/drawPlot_pulls.C+");

}
