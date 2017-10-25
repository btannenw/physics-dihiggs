#!/usr/bin/env python

import sys
import os
import AnalysisMgr_AZh as Mgr
import BatchMgr as Batch

# InputVersion = "inputs_forPostfit"
InputVersion = "30000"

def add_config(mass, lumi, var = 'mVH', unc = "Systs", merged=False, one_bin=False,mcstats=True, resolved=True,
               ZeroLep=True, OneLep=True, TwoLep=True, debug=False, inclusiveResolved=False,
              splitLowHighCR=False):
    conf = Mgr.WorkspaceConfig_AZh(unc, InputVersion=InputVersion, MassPoint=mass, OneBin=one_bin,
                                   UseStatSystematics=mcstats, Debug=debug,
                                   SkipUnkownSysts=True,
                                   DoShapePlots=False,DoSystsPlots=False,DeleteNormFiles=True ,
				   DoLumirescale=lumi,
                                   )
                                     #DecorrSys="SysJET_GroupedNP_1", DecorrTypes="T,L,D,J")
    #conf.set_channels(channel)
    conf.set_regions(var=var, merged=merged, resolved=resolved, zero_lepton=ZeroLep, one_lepton=OneLep, two_lepton=TwoLep,
                    inclusiveResolved=inclusiveResolved, splitLowHighCR=splitLowHighCR)
    fullversion = "VHbbRun2_13TeV_" + outversion + "_" + unc + "_" + mass + "_" + str(lumi) + "_" + var
    if var == "pTV": conf["ForceBinning"] = 4
    return conf.write_configs(fullversion)

if __name__ == "__main__":

    if len(sys.argv) is not 2:
        print "Usage: Analysis_AZh.py <outversion>"
        exit(0)

    outversion = sys.argv[1]


    # For running on existing workspaces (-k option):
    #WSlink = "/afs/cern.ch/user/g/gwilliam/workspace/WSMaker/workspaces/MonoH0708152p.180815_MonoH_13TeV_180815_Systs_Scalar_2000/combined/2000.root"
    #os.system("ls -lh " + WSlink)

    ########## create config files

    print "Adding config files..."
    configfiles_stat = []
    configfiles = []
    mass = [125]
    #vs = ['mVH']
    # vs = ['mBB', 'MET', 'pTV']
    # vs = ['mBB', 'MET']
    vs = ['mBB']
    lumi = [3.2, 10., 25.,]

    for l in lumi:
        for m in mass:
            for var in vs:
                # configfiles_stat += add_config(str(m), "StatOnly", one_bin=False, mcstats=True, merged=True, resolved=True,
                #                                ZeroLep=True, OneLep=False, TwoLep=True, debug=True)
                # configfiles      += add_config(str(m), "Systs", one_bin=False, mcstats=True, merged=True, resolved=True,
                #                                ZeroLep=True, OneLep=False, TwoLep=True, inclusiveResolved=False,
                #                               splitLowHighCR=False)
                if var == "mBB":
                    configfiles      += add_config(str(m), l, var=var, unc="NormSys", one_bin=False, mcstats=True, merged=False, resolved=True,
                                                   ZeroLep=True, OneLep=False, TwoLep=False, inclusiveResolved=False,
                                                   splitLowHighCR=False )
                elif var == "mVH":
                    configfiles      += add_config(str(m), var=var, unc="Systs", one_bin=False, mcstats=True, merged=True, resolved=True,
                                                   ZeroLep=True, OneLep=False, TwoLep=True, inclusiveResolved=False,
                                                   splitLowHighCR=False)
                elif var == "MET":
                    configfiles      += add_config(str(m), var=var, unc="Systs", one_bin=False, mcstats=True, merged=True, resolved=True,
                                                   ZeroLep=True, OneLep=False, TwoLep=True, inclusiveResolved=False,
                                                   splitLowHighCR=False)
                elif var == "pTV":
                    configfiles      += add_config(str(m), var=var, unc="Systs", one_bin=False, mcstats=True, merged=True, resolved=True,
                                                   ZeroLep=False, OneLep=False, TwoLep=True, inclusiveResolved=False,
                                                   splitLowHighCR=False)

                # configfiles_stat.append("configs/AZh_13TeV_{0}_StatOnly_{1}_SPLIT.config".format(InputVersion, m))
                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT.config".format(InputVersion, m))
                # configfiles_stat.append("configs/AZh_13TeV_{0}_StatOnly_{1}_SPLIT_NO_T.config".format(InputVersion, m))
                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_NO_T.config".format(InputVersion, m))
                #configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}.config".format(InputVersion, m))
                #configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_B_0.config".format(InputVersion, m))
                #configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_NO_T_B_0.config".format(InputVersion, m))
                #configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_JER.config".format(InputVersion, m))
                #configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_JES_1.config".format(InputVersion, m))

                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}.config".format(InputVersion, m))
                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_B_0.config".format(InputVersion, m))
                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_NO_T_B_0.config".format(InputVersion, m))
                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_JER.config".format(InputVersion, m))
                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_JES_1.config".format(InputVersion, m))

                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_Zhf_MODELING.config".format(InputVersion, m))
                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_SPLIT_Zcl.config".format(InputVersion, m))

                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_NO_SR.config".format(InputVersion, m))
                # configfiles.append("configs/AZh_13TeV_{0}_Systs_{1}_JES1_SAMPLE_SPLIT.config".format(InputVersion, m))

    for fi in configfiles_stat:
        print fi
    for fi in configfiles:
        print fi

    ########## define tasks

    tasks = ["-w"]             # create workspace (do always)
    # tasks += ["-k", WSlink]
    tasks += ["-l", 1]         # limit
    tasks += ["-s", 1]         # significance
    tasks += ["--fcc", "4,10@{MassPoint}"]  # fitCrossChecks
    tasks += ["-u", 1]         # breakdown of muhat errors stat/syst/theory
    # # requires fitCrossChecks:
    # tasks += ["-m", "4,10"]     # reduced diag plots (quick)
    # # tasks += ["-b", "2"]       # unfolded b-tag plots (quick)
    # the '!' signifies the reference workspace, & signifies to do sum plots
    # tasks += ["-p", "0,1@{MassPoint}&!21017.21017_AZh_13TeV_21017_Systs_600_mVH"]       # post-fit plots (CPU intense)
    # tasks += ["-p", "0,1@{MassPoint}"]       # post-fit plots (CPU intense)
    # # tasks += ["-p", "0,1;@{MassPoint};blah,blah"]       # post-fit plots (CPU intense)
    # # tasks += ["-r", "{MassPoint}", "-n", "{MassPoint}"]   # NP ranking
    # tasks += ["-t", "0,1@{MassPoint}"]       # yield tables (CPU intense)
    # tasks += ["-a", "current;p,4,10"]       # yield ratio tables

    ########## submit jobs

    # in case you want to run something locally
    Batch.run_local_batch(configfiles, outversion, tasks)
    # Batch.run_lxplus_batch(configfiles, outversion, tasks, '8nh')

    # adjust to recover failed ranking subjobs
    redoSubJob = -1

    #if redoSubJob < 0:
        #print "Submitting stat only jobs..."
        #Batch.run_lxplus_batch(configfiles_stat, outversion, ["-w", "-l", 1,"-s",1], '2nd')
        #print "Submitting systs jobs..."
        #Batch.run_lxplus_batch(configfiles, outversion, tasks, '2nd')

    #print "Submitting rankings..."
    #for m in mass:
    #WSlink = "/afs/cern.ch/user/n/nmorange/Hbb/WSMaker/workspaces/21013.testJERsamplesFine_AZh_13TeV_testJERsamplesFine_Systs_500/combined/500.root"
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "-r", "{MassPoint}"], '8nh', jobs=20, subJob=redoSubJob)
    #Batch.run_local_batch(configfiles, outversion, ["-w", "-k", WSlink, "-l", "0,{MassPoint}"])
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "-r", "{0}".format(m)], '1nd', jobs=20, subJob=redoSubJob)
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-r", "{MassPoint}"], '2nd', jobs=20, subJob=redoSubJob)
    #Batch.run_local_batch(configfiles, outversion, ["-w", "-k", WSlink, "-n", "{MassPoint}"])

    ########## non-default stuff. Warning: don't submit rankings / toys / NLL scans with the same outversion and nJobs!
    #print "Submitting toys..."
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "--fcc", "6"], '2nd', jobs=50, subJob=redoSubJob)
    #print "Submitting NLL scans..."
    #Batch.run_lxplus_batch(configfiles, outversion, ["-w", "-k", WSlink, "--fcc", "7,2,doNLLscan"], '2nd', jobs=50, subJob=redoSubJob)
