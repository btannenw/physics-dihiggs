#!/usr/bin/env python

#InputVersion = "HH181115LTT
#InputVersion = "HH270416"
#InputVersion = "11821" # XhhSM (non-res)
#InputVersion = "11822" # Xhh700
#InputVersion = "11823" # Xhh2000

import os,sys

def runMassPoint(configNumber, massPoint, binNumber, dirName):
    
    # *** A. Run splitInputs just to be safe
    #os.system("./SplitInputs -v {0} -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/may18_rootfiles/".format(str(configNumber)))

    # *** B. See if directory exists already 
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_StatOnly_{2}_exp'.format(str(configNumber), dirName, massPoint)
    headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_FloatOnly_{2}_exp'.format(str(configNumber), dirName, massPoint)
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_FloatOnly_{2}_obs'.format(str(configNumber), dirName, massPoint)
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
        # get input version right
        if 'InputVersion = \"' in line:
            if str(configNumber) in line and line[0] == "#":
                runFile.write(line.split('#')[1])
            elif str(configNumber) in line and line[0] != "#":
                runFile.write('#' + line)
            elif str(configNumber) not in line and line[0] != "#":
                runFile.write('#' + line)
            elif str(configNumber) not in line and line[0] == "#":
                runFile.write(line)
        else:
            runFile.write(line)   

    runFile.close()

    # *** D. grep/sed for mass point
    runFile = open ('scripts/HHWWbb/AnalysisMgr_HH.py','r')
    lines = runFile.readlines()
    runFile.close()
    runFile = open ('scripts/HHWWbb/AnalysisMgr_HH.py','w')

    for line in lines:
        #print line
        # get input version right
        if '13TeV_OneLepton' in line and 'mBBcr' not in line:
            if str("SRscan"+str(binNumber)+"_") in line and '#' in line:
                runFile.write( line.split('#')[0] + line.split('#')[1] )
            elif str("SRscan"+str(binNumber)+"_") not in line and '#' not in line:
                runFile.write( line.split('"13TeV')[0] + '#"13TeV' + line.split('"13TeV')[1])
            else:
                runFile.write(line)   
        else:
            runFile.write(line)   

    runFile.close()

    # *** E. Run the script
    #print 'python scripts/HHWWbb/Analysis_HH.py {0}-x{1}'.format(dirName, massPoint)
    os.system('python scripts/HHWWbb/Analysis_HH.py {0}-x{1}'.format(dirName, massPoint))

    # *** F. Move results to sanitized directory
    #newDir = 'root-files/1182X.{0}_HH_13TeV_{0}_StatOnly_{1}_exp'.format(dirName, massPoint)
    newDir = 'root-files/1182X.{0}_HH_13TeV_{0}_FloatOnly_{1}_exp'.format(dirName, massPoint)
    #newDir = 'root-files/1182X.{0}_HH_13TeV_{0}_FloatOnly_{1}_obs'.format(dirName, massPoint)
    os.system('mkdir {0}'.format(newDir))
    os.system('cp {0}/* {1}/'.format(headDir, newDir))

    return


def runScan(binNumber):
    inputDir = "060716-v2-LHscan"+str(binNumber)

    runMassPoint(11821, '250', binNumber, inputDir) # non-res
    runMassPoint(11822, '700', binNumber, inputDir)
    runMassPoint(11823, '2000', binNumber, inputDir)

################################################################
#####                        MAIN CODE                    ######
################################################################


# *** 1. Scan
#runScan(1)
runScan(2)
runScan(3)
runScan(4)
runScan(5)
runScan(6)
runScan(7)
runScan(8)
runScan(9)
runScan(10)
runScan(11)
runScan(12)
runScan(13)
runScan(14)
runScan(15)
runScan(16)
runScan(17)
runScan(18)
runScan(19)
runScan(20)
runScan(21)
runScan(22)
runScan(23)
runScan(24)
runScan(25)
runScan(26)
runScan(27)
runScan(28)
runScan(29)





