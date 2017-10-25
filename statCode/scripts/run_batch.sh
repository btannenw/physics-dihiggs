#!/bin/bash
#
# G. Facini
# Tue Aug 14 19:51:02 CEST 2012
#
# purpose:  run selection code on lxbatch
#
# usage: configure and set to submit to grid
#	 with submit.sh in same directory
#

source /afs/cern.ch/project/eos/installation/atlas/etc/setup.sh
export EOS_MGM_URL=root://eosatlas.cern.ch
export PATH=/afs/cern.ch/project/eos/installation/0.2.5/bin:$PATH

#mytar=${1}
#input=${2}
#myafs=${3}
#mycmd=${4}

mytar=XXX_MYTAR
input=XXX_INPUTS
myafs=XXX_MYAFS
mycmd=XXX_MYCMD

echo "Job ID	    : ${JOBID}"
echo "Tar file in : ${mytar}"
echo "Inputs      : ${input}"
echo "Output dir  : ${myafs}"
echo "Running     : ${mycmd}"

#go to lxplus workdir (this is unique for this job)
####cd $WORKDIR
mkdir MyWorkingDir
cd MyWorkingDir
# Which directory are we in?
echo "Working directory"
echo `pwd`

# Copy tar file
cp -r ${mytar}/stuff.tar.gz .
tar -xzvf stuff.tar.gz
rm stuff.tar.gz

# link input file directory
ln -s ${input} .

echo ""
source setup.sh
export WORKDIR=$PWD
echo "WORKDIR=$WORKDIR"
echo ""
gmake -j 4
make -j 2 scripts
echo ""

echo "Running"
START=$(date +%s)
`${mycmd}`
echo "Done"
END=$(date +%s)
DIFF=$(( $END - $START ))
echo "Run time in seconds : ${DIFF}"

echo ""
echo ""

echo "copying output back"
echo "${myafs}"

# List content of the working directory for debugging purposes
"ls" -l

tar -czvf output.tar.gz plots fccs workspaces xml logs root-files eps-files pdf-files tables
cp -v output.tar.gz ${myafs}

# Clean workspace before exit
cd ..
#####rm -fR MyWorkingDir
echo "goodbye"

