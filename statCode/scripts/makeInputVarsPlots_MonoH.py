#!/usr/bin/env python


import sys
import subprocess
import os
import os.path
import shutil

import ROOT
import AnalysisMgr_MonoH as Mgr
import BatchMgr as Batch
import doPlotFromWS as Plotter


# Input with extra variables
version_nb="MonoH141115Vars" 

# output tag is whatever you want. If you have several variables in your input file and want to run this script
# several times, then update this tag each time
output_tag = "1"

# Fit result
fitdir_cut="/afs/cern.ch/user/g/gwilliam/workspace/WSMaker/FitCrossChecks_MonoH20151110c.20151112aBlind_MonoH_13TeV_20151112aBlind_Systs_Scalar_2000_1_combined"

# the variables you want to plot
run_list = [
    ["mjjMask_MET", "5"],
]

def main():

    configs_list_cut = []

    # Signal setup
    dm = "Scalar"
    mass = 1
    mediator = 2000

    for i_run in run_list:

        # definition of needed variables
        variable=str(i_run[0])
        choice_rebin=str(i_run[1]) if len(i_run) > 1 else 0

        # some rebin
        my_forceBinning=choice_rebin

        """ Create a workspace with the given inputs """
        conf = Mgr.WorkspaceConfig_MonoH("Systs", "8TeV", InputVersion=version_nb,
                                         UseStatSystematics=False, MassPoint=mass, DMType=dm, MediatorMass=mediator, OneBin=False,
                                         FloatHFOnlyForZ=True, FloatHFOnlyForW=True,
                                         ForceBinning=my_forceBinning, DefaultFitConfig=True)  # DefaultFitConfig makes sure fit is setup as for nominal case even if don't plot all regions

        print "Will create a config file to make a workspace for variable", variable

        # Regions to plot
        conf["Regions"] = [
             "13TeV_ZeroLepton_CUT_2tag2pjet_150_500ptv_" + variable,

#             "13TeV_ZeroLepton_CUT_2tag2jet_150_200ptv_" + variable,
#             "13TeV_ZeroLepton_CUT_2tag2jet_150ptv_" + variable,
#             "13TeV_ZeroLepton_CUT_2tag2jet_200_350ptv_" + variable,
#             "13TeV_ZeroLepton_CUT_2tag2jet_350_500ptv_" + variable,
#             "13TeV_ZeroLepton_CUT_2tag3pjet_150_200ptv_" + variable,
#             "13TeV_ZeroLepton_CUT_2tag3pjet_150ptv_" + variable,
#             "13TeV_ZeroLepton_CUT_2tag3pjet_200_350ptv_" + variable,
#             "13TeV_ZeroLepton_CUT_2tag3pjet_350_500ptv_" + variable,
            ]

        # Make WS for a given variable 
        #conf.set_regions_for_plots(var = variable)
        configs_list_cut += conf.write_configs("MonoH_13TeV_" + version_nb + "_" + "Systs" + "_" + dm + "_" + str(mediator) + "_" + str(mass) + "_" + variable)
        
    print "configs_list_cut:"
    print configs_list_cut

    # Run WS and post fit plots
    if len(configs_list_cut) > 0:
        Batch.run_local_batch(configs_list_cut, output_tag, ["-w", "-p", "2;"+fitdir_cut+"@"+str(mass)])

if __name__ == "__main__":
    print "start"
    main()
    print "finished"
