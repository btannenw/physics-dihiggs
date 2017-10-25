#!/usr/bin/env/python

import sys
import doPlotFromWS as plots


def main(directory=None, is_conditional=False, is_asimov=False, mu=None):
    if directory:
        rfr, suffix = plots.getFitResult(directory, is_conditional, is_asimov, mu)
        rfr.Print()

if __name__ == "__main__":
    fitres = sys.argv[1]
    modes = [int(s) for s in sys.argv[2].split(',')]
    # modes:
    # 1 is bkg-only fit
    # 2 is S+B fit
    fcc = "fccs/FitCrossChecks_"+fitres+"_combined"
    for mode in modes:
        if mode == 1:
            print "Print result for bkg-only fit"
            main(fcc, is_conditional=True, is_asimov=False, mu=0)
        elif mode == 2:
            print "Print result from unconditional fit"
            main(fcc, is_conditional=False, is_asimov=False, mu=1)
