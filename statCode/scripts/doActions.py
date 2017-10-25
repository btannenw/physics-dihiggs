#!/usr/bin/env python

import optparse
import commands
import os
import subprocess
import BatchMgr as Mgr
import re

def main():
    usage = "Usage: %prog [options] configfile outputversion"
    parser = optparse.OptionParser(usage)
    parser.add_option("-w", "--workspace", dest="ws", action="store_true",
                      help="create workspace", default=False)
    parser.add_option("-k", "--wslink", dest="WSlink", metavar="link",
                      help="link workspace, requires -w", default="")
    parser.add_option("-l", "--limit", dest="limit", metavar="isExpected",
                      help="compute exp (1) or obs (0) limit")
    parser.add_option("-f", "--fcc", dest="fcc", metavar="algs",
                      help="run FitCrossChecks")
    parser.add_option("-m", "--makeReduced", dest="makered", metavar="algs",
                      help="run makeReducedDiagPlots")
    parser.add_option("-u", "--muhat", dest="muhat", metavar="isExpected",
                      help="run GetMuHat")
    parser.add_option("-t", "--tables", dest="tables", metavar="modes",
                      help="make pre/postfit tables")
    parser.add_option("-a", "--ratios", dest="ratios", metavar="modes",
                      help="make pre/postfit ratio tables")
    parser.add_option("-p", "--plots", dest="plots", metavar="modes",
                      help="make pre/postfit plots")
    parser.add_option("-b", "--btagPlots", dest="btagPlots", metavar="modes",
                      help="make unfolded b-tag plots")
    parser.add_option("-s", "--sig", dest="sig", metavar="isExpected",
                      help="compute exp/obs significances and p0")
    parser.add_option("-r", "--NPranking", dest="NPranking", metavar="mass",
                      help="compute nuissance parameter ranking")
    parser.add_option("-n", "--NPrankingPlots", dest="NPrankingPlots", metavar="mass",
                      help="plot nuissance parameter ranking")
    (options, args) = parser.parse_args()
    print args
    if len(args) < 2:
        print "ERROR: must provide at least a config file and an output version"
        parser.print_help()
        return

    conf_file = args[0]
    outputversion = args[1]

    cutv, outv = Mgr.get_ws_name(conf_file, outputversion)
    ws_name = cutv+'.'+outv
    print "Working on workspace", ws_name
    print "Python version", os.system("python --version")

    if options.ws:
        make_workspace(conf_file, outv, ws_name, str(options.WSlink))

    launch_programs(ws_name, options)


def make_workspace(conf, outver, ws_name, WSlink = ""):
    """run MakeWorkspace. Throw an exception and exit if anything goes wrong"""
    logdir = "logs"
    os.system("mkdir -p " + logdir)
    if WSlink is not "":
        os.system("mkdir -vp workspaces/" + ws_name + "/combined")
        os.system("rm -v workspaces/" + ws_name + "/combined/*.root")
        os.system("ln -sv " + WSlink + "  workspaces/" + ws_name + "/combined/.")

        return
    logfilename = "{0}/output_{1}.log".format(logdir, ws_name)
    os.system("rm -f "+logfilename)
    output_f=open(logfilename, 'w')
    subprocess.check_call(["./MakeWorkspace", str(conf), str(outver)], stderr=output_f, stdout = output_f)

def launch_programs(ws_name, options):
    logdir = "logs"
    try:
        os.mkdir(logdir)
    except OSError:
        pass
    pids_primary=[]
    pids_secondary=[]
    logfiles=[]

    if options.fcc is not None:
        if '@' in options.fcc:
            conf = str(options.fcc).split('@')
            obsDataName = "obsData"
            #obsDataName = "asimovData"
            if len(conf) > 2:
                obsDataName = conf[2]
            for mass in conf[1].split(','):
                res = submit_program(["python", "scripts/runFitCrossCheck.py", ws_name, str(conf[0]), str(mass),str(obsDataName)],
                                     ws_name, "runFCC", logdir, pids_primary, logfiles)
        else:
            res = submit_program(["python", "scripts/runFitCrossCheck.py", ws_name, str(options.fcc), "125"],
                                 ws_name, "runFCC", logdir, pids_primary, logfiles)

    if options.limit is not None:
        conf = str(options.limit).split(',')
        if len(conf)>1:
            for mass in conf[1:]:
                res = submit_program(["python", "scripts/getLimit.py", ws_name, str(conf[0]), str(mass)],
                                     ws_name, "getLimit_"+str(mass), logdir, pids_secondary, logfiles)
        else:
            res = submit_program(["python", "scripts/getLimit.py", ws_name, str(options.limit), "125"],
                                 ws_name, "getLimit", logdir, pids_secondary, logfiles)

    if options.sig is not None:
        conf = str(options.sig).split(',')
        if len(conf)>1:
            for mass in conf[1:]:
                res = submit_program(["python", "scripts/getSig.py", ws_name, str(conf[0]), str(mass)],
                                     ws_name, "getSig_"+str(mass), logdir, pids_secondary, logfiles)
        else:
            res = submit_program(["python", "scripts/getSig.py", ws_name, str(options.sig), "125"],
                                 ws_name, "getSig", logdir, pids_secondary, logfiles)

    if options.muhat is not None:
        print "Running muhat with options: ",options.muhat
        muhatopts = str(options.muhat).split(',')
        is_expected = muhatopts[0]
        mass = "125"
        if len(muhatopts) > 1:
            mass = muhatopts[1]
        if int(is_expected) == 2:
            Mgr.wait_all(pids_secondary)
        res = submit_program(["python", "scripts/runMuHat.py", ws_name, is_expected, "1", mass],
                             ws_name, "runMuHat", logdir, pids_secondary, logfiles)

    if options.NPranking is not None:
        npopts = str(options.NPranking).split(',')
        mass = npopts[0]
        dataName = "obsData"
        if len(npopts) > 1:
            dataName=npopts[1]
        if dataName == "asimovDataAtLimit":
            Mgr.wait_all(pids_secondary)
        res = submit_program(["python", "scripts/runNPranking.py", ws_name, mass, "ModelConfig", "SigXsecOverSM", dataName], ws_name, "runNPranking", logdir, pids_secondary, logfiles)

    # Wait for end of FCC, which is a prerequisite for other things
    Mgr.wait_all(pids_primary)

    # When it is finished, can launch other things
    if options.makered is not None:
        tokens = re.split('(;|@)', options.makered)
        fcc_types = tokens[0]
        pass_to_user = tokens[-1]
        if pass_to_user == fcc_types: pass_to_user = ''

        a = ["python", "scripts/makeReducedDiagPlots.py", '-p', fcc_types, ws_name]
        if not pass_to_user == '':
            for user_arg in re.split('(,|;)', pass_to_user):
                if not user_arg == ',' or not user_arg == ';':
                    a.append(user_arg)

        res = submit_program(a,
                             ws_name, "makeReducedDiagPlots", logdir, pids_secondary, logfiles)

    if options.tables is not None:
        tokens = re.split('(;|@)', options.tables)
        mass = tokens[tokens.index('@')+1] if '@' in tokens else ''
        # plot_options = tokens[tokens.index(';')+1] if ';' in tokens else ''
        table_types = tokens[0]
        pass_to_user = tokens[-1]
        if pass_to_user == str(mass): pass_to_user = ''

        a = ["python", "scripts/makeTables.py", '-t', table_types, '-m', mass, ws_name]
        if not pass_to_user == '':
            for user_arg in re.split('(,|;)', pass_to_user):
                if not user_arg == ',' or not user_arg == ';':
                    a.append(user_arg)
        res = submit_program(a,
                             ws_name, "makeTables", logdir, pids_secondary, logfiles)

    if options.ratios is not None:
        tokens = re.split('(;|@)', options.ratios)
        ws_name2 = tokens[0]
        if ws_name2.lower() == "current": ws_name2 = ws_name
        fcc_types = tokens[2]
        pass_to_user = tokens[-1]
        if pass_to_user == fcc_types: pass_to_user = ''

        a = ["python", "scripts/makeRatioTables.py", '-t', fcc_types, ws_name, ws_name2]
        if not pass_to_user == '':
            for user_arg in re.split('(,|;)', pass_to_user):
                if not user_arg == ',' or not user_arg == ';':
                    a.append(user_arg)

        res = submit_program(a,
                             ws_name, "makeRatioTables", logdir, pids_secondary, logfiles)

    if options.plots is not None:
        # FIXME script can't run with varios options and single option is required.
        #       if options config: 0,1 => option 0 crash at the end producing the plots. option 1 => do not produce plots
        #       if options config: 1,0 => option 1 produce the plots. option 0 plots are equal to option 1
        #       O_o
        #       option 0 crash at the end needs to be fix
        # res = submit_program(["python", "scripts/doPlotFromWS.py", ws_name, str(options.plots)],
        #                      ws_name, "makePlots", logdir, pids_secondary, logfiles)
        # solution run the script for each option

        #plot_options = options.plots.partition(';')

        tokens = re.split('(;|@|!|&)', options.plots)
        mass = tokens[tokens.index('@')+1] if '@' in tokens else ''
        fccdir = tokens[tokens.index('!')+1] if '!' in tokens else ''
        # plot_options = tokens[tokens.index(';')+1] if ';' in tokens else ''
        doSum = '&' in tokens
        plot_types = tokens[0]
        pass_to_user = tokens[-1]
        if pass_to_user == str(mass): pass_to_user = ''

        # FIXME
        for opt in plot_types.split(','):
            a = ["python", "scripts/doPlotFromWS.py", '-p', str(opt), '-m', mass, ws_name]
            if not fccdir == '':
                a.insert(2, fccdir)
                a.insert(2, '-f')
            if doSum:
                a.insert(2, '-s')
            # else:
            #     a = ["python", "scripts/doPlotFromWS.py", '-f', fccdir, '-p', str(opt), '-m', mass, ws_name]
            if not pass_to_user == '':
                for user_arg in re.split(',', pass_to_user):
                    if not user_arg == ',' or not user_arg == ';':
                        a.append(user_arg)
            print a
            res = submit_program(a,
                                 ws_name, "makePlotsOption"+str(opt), logdir, pids_secondary, logfiles)



    if options.btagPlots is not None:
        res = submit_program(["python", "scripts/makeUnfoldedBTagPlots.py", ws_name, str(options.btagPlots)],
                             ws_name, "makeBtagPlots", logdir, pids_secondary, logfiles)

    # Wait for completion of other algorithms
    Mgr.wait_all(pids_secondary)

    if options.NPrankingPlots is not None:
        res = submit_program(["python", "scripts/makeNPrankPlots.py", ws_name, str(options.NPrankingPlots)], ws_name, "NPrankPlots", logdir, pids_secondary, logfiles)

    for f in logfiles:
        f.close()

def submit_program(exec_sequence, ws_name, logfile_suffix, logdir, pids, logfiles):
    logfile = "{0}/output_{1}_{2}.log".format(logdir, ws_name, logfile_suffix)
    res = Mgr.submit_local_job(exec_sequence, logfile)
    pids.append(res[0])
    logfiles.append(res[1])

if __name__ == "__main__":
    main()
