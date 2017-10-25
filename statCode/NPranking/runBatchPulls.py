# Author      : Stefan Gadatsch
# Email       : gadatsch@nikhef.nl
# Date        : 2013-04-24
# Description : Submit jobs to LSF for computing pulls

import os
from ROOT import *

import argparse
parser = argparse.ArgumentParser()
parser.add_argument("workspace", type=str, help="Path to workspace to run on.")
parser.add_argument("--folder", type=str, default="test", help="Identifier for the workspace")
parser.add_argument("--compileRunPulls", help="Compile runPulls.C", action="store_true")
parser.add_argument("--queue", type=str, default="8nh", help="Queue to submit to.")
parser.add_argument("--workspaceName", type=str, default="combWS", help="Name of the workspace.")
parser.add_argument("--ModelConfigName", type=str, default="ModelConfig", help="Name of the ModelConfig.")
parser.add_argument("--dataName", type=str, default="combData", help="Name of the dataset.")
parser.add_argument("--poiName", type=str, default="mu", help="Name of the POI.")
parser.add_argument("--precision", type=str, default="0.005", help="Precision of uncertainty evaluation.")
parser.add_argument("--useMinos", type=str, default="0", help="Use Minos to compute uncertainties, default is findSigma.")
parser.add_argument("--loglevel", type=str, default="DEBUG", help="Control the printout.")

args = parser.parse_args()

workspace = args.workspace
print "This is the workspace", workspace
folder = args.folder
compileRunPulls = args.compileRunPulls
queue = args.queue
workspaceName = args.workspaceName
ModelConfigName = args.ModelConfigName
dataName = args.dataName
poiName = args.poiName
precision = args.precision
useMinos = args.useMinos
loglevel = args.loglevel


def submitJob(variable):
    bsubFile = open("bsub/"+folder+"/"+variable+".sh", "w")
    text = getJobDef(variable)
    bsubFile.write(text)
    bsubFile.close()
    os.system("chmod -R 775 bsub/"+folder)
    command = "bsub < bsub/"+folder+"/"+variable+".sh"
    print
    print command
    os.system(command)


def getJobDef(variable):

    text = """
#!/bin/bash

#BSUB -J %s
#BSUB -o bsub/%s/stdout_%s.out
#BSUB -q %s
#BSUB -u $USER@cern.ch

WORKDIR=$TMPDIR/LSF_$LSB_JOBID
HOMEDIR=%s
OUTDIR=$HOMEDIR
FOLDER=%s

stagein()
{
  uname -a

  mkdir -vp ${WORKDIR}
  cd ${HOMEDIR} 2> /dev/null || { echo "The directory does not exist."; exit -1; }
  echo Current folder is
  pwd
  ls -l

  source /afs/cern.ch/sw/lcg/contrib/gcc/4.8.1/x86_64-slc6/setup.sh
  export PATH=/afs/cern.ch/sw/lcg/external/Python/2.7.4/x86_64-slc6-gcc48-opt/bin:$PATH

  source /afs/cern.ch/work/n/nmorange/public/root-v5-34/bin/thisroot.sh
}

runcode()
{
  ulimit -S -s 20000
""" % (variable, folder, variable, queue, home_folder, folder)

    text += """
  root -b -q macros/runPulls.C+\\(\\"%s\\",\\"%s\\",\\"%s\\",\\"%s\\",\\"%s\\",\\"%s\\",\\"%s\\",%s,%s,\\"%s\\"\\)
    """ % (workspace, poiName, workspaceName, ModelConfigName, dataName, folder, variable, precision, useMinos, loglevel)

    text += """
}

stageout()
{
  cd ${WORKDIR}

  cd ${OUTDIR}; ls -l
}

stagein
runcode
stageout

exit
"""
    return text

os.system("mkdir -vp bsub/%s" % folder)
os.system("mkdir -vp root-files/%s" % folder)

home_folder = os.getcwd()

ret = 0
if compileRunPulls:
    os.system("cp macros/runPulls.C bsub/%s/." % folder)
#    ret += os.system("root -b -q bsub/%s/runPulls.C+" % folder)

if ret != 0:
    print "Error in compilation"
else:
    file = TFile(workspace)

    ws = file.Get(workspaceName)
    mc = ws.obj(ModelConfigName)
    nitr = mc.GetNuisanceParameters().createIterator()

    while (nitr.Next()):
        varName = str(nitr.GetName())

        if "gamma_stat" in varName:
            continue

        submitJob(varName)
