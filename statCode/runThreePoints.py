#!/usr/bin/env python

#InputVersion = "HH181115LTT
#InputVersion = "HH270416"
#InputVersion = "11821" # XhhSM (non-res)
#InputVersion = "11822" # Xhh700
#InputVersion = "11823" # Xhh2000

import os,sys

def runMassPoint(configNumber, massPoint, dirName):
    
    # *** A. Run splitInputs just to be safe
    #os.system("./SplitInputs -v {0} -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/may18_rootfiles/".format(str(configNumber)))

    # *** B. See if directory exists already 
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_StatOnly_{2}_exp'.format(str(configNumber), dirName, massPoint)
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_FloatOnly_{2}_exp'.format(str(configNumber), dirName, massPoint)
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_FloatOnly_{2}_obs'.format(str(configNumber), dirName, massPoint)
    headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_Systs_{2}_exp'.format(str(configNumber), dirName, massPoint)
    #print headDir
    
    while os.path.exists(headDir):
        print '!!!!! Sorry, "+headDir" exists.'
        print 'ABORTING.'
        return

    # *** C. grep/sed for mass point
    runFile = open ('scripts/HHWWbb/Analysis_HH.py','r')
    lines = runFile.readlines()
    runFile.close()
    runFile = open ('scripts/HHWWbb/Analysis_HH.py','w')

    for line in lines:
        if 'InputVersion = \"' in line:
            if str(configNumber) in line and line[0] == "#":
                runFile.write(line.split('#')[1])
            elif str(configNumber) in line and line[0] != "#":
                runFile.write(line)
            elif str(configNumber) not in line and line[0] != "#":
                runFile.write('#' + line)
            elif str(configNumber) not in line and line[0] == "#":
                runFile.write(line)
        else:
            runFile.write(line)

    runFile.close()

    # *** D. Run the script
    #print 'python scripts/HHWWbb/Analysis_HH.py {0}-x{1}'.format(dirName, massPoint)
    os.system('python scripts/HHWWbb/Analysis_HH.py {0}-x{1}'.format(dirName, massPoint))

    # *** E. Move results to sanitized directory
    #newDir = 'root-files/1182X.{0}_HH_13TeV_{0}_StatOnly_{1}_exp'.format(dirName, massPoint)
    #newDir = 'root-files/1182X.{0}_HH_13TeV_{0}_FloatOnly_{1}_exp'.format(dirName, massPoint)
    #newDir = 'root-files/1182X.{0}_HH_13TeV_{0}_FloatOnly_{1}_obs'.format(dirName, massPoint)
    newDir = 'root-files/1182X.{0}_HH_13TeV_{0}_Systs_{1}_exp'.format(dirName, massPoint)
    os.system('mkdir {0}'.format(newDir))
    os.system('cp {0}/* {1}/'.format(headDir, newDir))

    return


################################################################
#####                        MAIN CODE                    ######
################################################################


# *** 1. Ask for name of target directory
inputDir = raw_input("Enter name of target dir (e.g. ): ")

# *** 2. Run mass points
#runMassPoint(11821, '250', inputDir) # non-res
#runMassPoint(11822, '700', inputDir)
#runMassPoint(11823, '2000', inputDir)

runMassPoint(48007, '700', inputDir)
runMassPoint(48002, '2000', inputDir)

