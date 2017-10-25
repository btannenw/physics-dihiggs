#!/usr/bin/env python

import sys
import argparse
import os
import os.path
import shutil
import re
import subprocess
import tarfile
import fnmatch

def main():
    args = parse_input()
    args.func(args)

def getItems(args):
    dirs_to_use = dirs_to_archive(args.version, args.restrict)
    tar_arguments = forge_tar_command(args)
    print dirs_to_use
    print tar_arguments

    if args.archive:
        cur_dir = os.getcwd()
        # Setup the paths
        username = os.environ['USER']
        tmp_dir = os.path.join('/tmp', username, 'tmp_plots')
        # Create our work area
        if os.path.exists(tmp_dir):
            shutil.rmtree(tmp_dir)
        os.makedirs(tmp_dir)
        os.chdir(tmp_dir)

    # Do the extraction
    for d in dirs_to_use:
        os.system(" ".join(["tar", "zxvf", os.path.join(d, 'output.tar.gz')] + tar_arguments))

    if args.archive:
        archive = shutil.make_archive('_'.join(args.version), "gztar", root_dir=".")
        shutil.copy(archive, cur_dir)
        os.chdir(cur_dir)

def printItems(args):
    dirs_to_use = dirs_to_archive(args.version, args.restrict)
    files = [tarfile.open(os.path.join(d, 'output.tar.gz'), "r:gz") for d in dirs_to_use if os.path.exists(os.path.join(d, 'output.tar.gz'))]
    if args.limit:
        print_results(files, True)
    if args.sig:
        print_results(files, False)

def print_results(tarfiles, is_limit):
    if is_limit:
        the_string = "limit"
        logfile = "getLimit"
        median = "Expected"
    else:
        the_string = "significance"
        logfile = "getSig"
        median = "Median"
    print "\n\n\nExpected and Observed {0}s:".format(the_string)
    for f in tarfiles:
        full_version_tag = "/".join(f.name.split('/')[-3:-1])
        lognames = fnmatch.filter(f.getnames(), "logs/*{0}.log".format(logfile))
        if len(lognames) != 1:
            print "{0}.log not found in archive:".format(logfile)
            print f.name
            continue
        log = f.extractfile(lognames[0])
        log.seek(-500, 2) # some default value
        lines = log.readlines()
        med_lim = fnmatch.filter(lines, "{0} {1}*".format(median, the_string))[0].rstrip('\n')
        obs_lim = fnmatch.filter(lines, "Observed {0}*".format(the_string))[0].rstrip('\n')
        print "  * {0}".format(full_version_tag)
        print "    - {0}".format(med_lim)
        print "    - {0}".format(obs_lim)
        print ''


def listVersions(args):
    bdir = batch_dir()
    dirs = [ d for d in os.listdir(bdir) if os.path.isdir(os.path.join(bdir, d)) ]
    dirs.sort(key = lambda d: os.path.getmtime(os.path.join(bdir, d)))
    if args.all:
        for d in dirs:
            print d
    else:
        for d in dirs[-10:]:
            print d

def forge_tar_command(content):
    arguments = []
    if content.logs:
        arguments.append("\"logs/*getLimit.log\"")
        arguments.append("\"logs/*getSig.log\"")
        arguments.append("\"logs/*runMuHat.log\"")
    if content.plots:
        arguments.append("plots")
    if content.tables:
        arguments.append("tables")
    if content.ranking:
        arguments.append("root-files/")
    if content.NPs:
        arguments.append("\"fccs/FitCrossChecks*/output*.log\"")
        # TODO add the tables of NP in tex files
    if content.workspaces:
        arguments.append("workspaces/")
    if content.xmls:
        arguments.append("\"xml/*/*/*.xml\"")
    if content.fccs:
        arguments.append("\"fccs/FitCrossChecks*/FitCrossChecks.root\"")
    return arguments

def batch_dir():
    username = os.environ['USER']
    bdir = os.path.join('/afs', 'cern.ch', 'work', username[0], username, "analysis", "statistics", "batch")
    return bdir

def dirs_to_archive(version, pattern):
    # Now look for what we have to archive
    dirs_to_use = []
    for v in version:
        v_dir = os.path.join(batch_dir(), v)
        if not os.path.exists(v_dir):
            raise OSError("No such version in batch directory: {0}".format(v))
        dirs_of_v = [ os.path.join(v_dir, d) for d in os.listdir(v_dir) if os.path.isdir(os.path.join(v_dir, d)) ]
        if pattern:
            patt = re.compile(pattern)
            dirs_of_v = [ d for d in dirs_of_v if patt.search(d) ]
        dirs_to_use += dirs_of_v
    print dirs_to_use
    return dirs_to_use

def parse_input():
    parser = argparse.ArgumentParser(
        description='Collate or archive plots and results for a given production')
    subparsers = parser.add_subparsers(dest='action')

    lister = subparsers.add_parser('list')
    lister.set_defaults(func = listVersions)
    lister.add_argument('--all', help='Show all possibilities (default is last 10)',
                      action='store_true')

    getter = subparsers.add_parser('get')
    getter.set_defaults(func = getItems)
    getter.add_argument('version', nargs='+', help='list of versions, specified as InputVersion.OutputVersion')
    getter.add_argument('--restrict-to', dest='restrict', help='Pattern for workspaces to keep')
    getter.add_argument('--archive', help='Create a tar.gz archive instead of putting into work dir',
                        action='store_true')
    getter.add_argument('--logs', help='Retrieve log files', action='store_true')
    getter.add_argument('--plots', help='Retrieve plots', action='store_true')
    getter.add_argument('--tables', help='Retrieve tables', action='store_true')
    getter.add_argument('--NPs', help='Retrieve FCC logs', action='store_true')
    getter.add_argument('--fccs', help='Retrieve FCC results (root files)', action='store_true')
    getter.add_argument('--workspaces', help='Retrieve workspaces', action='store_true')
    getter.add_argument('--xmls', help='Retrieve xml files', action='store_true')
    getter.add_argument('--ranking', help='Retrieve root files for ranking plots', action='store_true')
    getter.add_argument('--all', help='Retrieve everything', action='store_true')

    printer = subparsers.add_parser('print')
    printer.set_defaults(func = printItems)
    printer.add_argument('version', nargs='+', help='list of versions, specified as InputVersion.OutputVersion')
    printer.add_argument('--restrict-to', dest='restrict', help='Pattern for workspaces to keep')
    printer.add_argument('--limit', help='Print limit', action='store_true')
    printer.add_argument('--sig', help='Print significance', action='store_true')

    args = parser.parse_args()
    if args.action == "get":
        if not (args.logs or args.plots or args.tables or args.NPs or args.workspaces or args.xmls or args.fccs or args.ranking):
            args.all = True
        if args.all is True:
            args.logs = True
            args.plots = True
            args.tables = True
            args.NPs = True
            args.workspaces = True
            args.xmls = True
            args.fccs = True
            args.ranking = True
    return args

if __name__ == "__main__":
    main()
