#!/usr/bin/env python

import os,sys

def runWorkspace(ws, mass):

    #os.system("python scripts/runFitCrossCheck.py {0} 2,6 {1}".format(ws, mass)) # run fccs
    #os.system("python scripts/comparePulls.py 0 1 {0} 0 \"\" {0}".format(ws)) # compare pulls
    #os.system("python scripts/runNPranking.py {0} {1}".format(ws,mass)) # run NP ranking
    os.system("python scripts/makeNPrankPlots.py {0} {1}".format(ws,mass)) # plot NP ranking

    return

################################################################
#####                        MAIN CODE                    ######
################################################################

#runWorkspace("80006.170816-v25-objSRandCR_modSRandCR_ttbarModSR_floatCR_runFccs2only-x700_HH_13TeV_170816-v25-objSRandCR_modSRandCR_ttbarModSR_floatCR_runFccs2only-x700_Systs_700","700")
#runWorkspace("80006.180816-v1-CRonly_objCR_floatCR_modCR_addTTbarModandSignalTheoryUnc-x700_HH_13TeV_180816-v1-CRonly_objCR_floatCR_modCR_addTTbarModandSignalTheoryUnc-x700_Systs_700","700")
runWorkspace("80006.180816-v14-CRonly_forReal-x700_HH_13TeV_180816-v14-CRonly_forReal-x700_FloatOnly_700","700")
