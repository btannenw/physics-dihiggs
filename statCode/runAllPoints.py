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
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_StatOnly_{2}_obs'.format(str(configNumber), dirName, massPoint)
    headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_FloatOnly_{2}_exp'.format(str(configNumber), dirName, massPoint)
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_FloatOnly_{2}_obs'.format(str(configNumber), dirName, massPoint)
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_Systs_{2}_exp'.format(str(configNumber), dirName, massPoint)
    #headDir = '/afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/root-files/{0}.{1}-x{2}_HH_13TeV_{1}-x{2}_Systs_{2}_obs'.format(str(configNumber), dirName, massPoint)
    #print headDir
    
    while os.path.exists(headDir):
        print '!!!!! Sorry, "+headDir" exists.'
        print 'ABORTING.'
        return

    # *** C. grep/sed for mass point
    #runFile = open ('scripts/HHWWbb/Analysis_HH.py','r')
    #lines = runFile.readlines()
    #runFile.close()
    #runFile = open ('scripts/HHWWbb/Analysis_HH.py','w')

    #for line in lines:
    #    if 'InputVersion = \"' in line:
    #        if str(configNumber) in line and line[0] == "#":
    #            runFile.write(line.split('#')[1])
    #        elif str(configNumber) in line and line[0] != "#":
    #            runFile.write(line)
    #        elif str(configNumber) not in line and line[0] != "#":
    #            runFile.write('#' + line)
    #        elif str(configNumber) not in line and line[0] == "#":
    #            runFile.write(line)
    #    else:
    #        runFile.write(line)

    #runFile.close()

    # *** D. Run the script
    #print 'python scripts/HHWWbb/Analysis_HH.py {0}-x{1}'.format(dirName, massPoint)
    os.system('python scripts/HHWWbb/Analysis_HH.py {0} {1}-x{2} {2}'.format(configNumber, dirName, massPoint))

    # *** E. Move results to sanitized directory
    #newDir = 'root-files/1182X.{0}_HH_13TeV_{0}_StatOnly_{1}_exp'.format(dirName, massPoint)
    if configNumber/10000 == 7:
        xDir = '7000X'
    if configNumber/10000 == 8:
        xDir = '8000X'
    if configNumber/10000 == 9:
        xDir = '9000X'
        
    #newDir = 'root-files/{0}.{1}_HH_13TeV_{1}_StatOnly_{2}_obs'.format(xDir, dirName, massPoint)
    newDir = 'root-files/{0}.{1}_HH_13TeV_{1}_FloatOnly_{2}_exp'.format(xDir, dirName, massPoint)
    #newDir = 'root-files/{0}.{1}_HH_13TeV_{1}_FloatOnly_{2}_obs'.format(xDir, dirName, massPoint)
    #newDir = 'root-files/{0}.{1}_HH_13TeV_{1}_Systs_{2}_exp'.format(xDir, dirName, massPoint)
    #newDir = 'root-files/{0}.{1}_HH_13TeV_{1}_Systs_{2}_obs'.format(xDir, dirName, massPoint)

    os.system('mkdir {0}'.format(newDir))
    os.system('cp {0}/* {1}/'.format(headDir, newDir))

    return

def getConfigNumbers(massPoint):
    a = []
    if massPoint == '250':
        a = [70001, 80001, 90001]
    if massPoint == '300':
        a = [70002, 80002, 90002]
    if massPoint == '400':
        a = [70003, 80003, 90003]
    if massPoint == '500':
        a = [70004, 80004, 90004]
    if massPoint == '600':
        a = [70005, 80005, 90005]
    if massPoint == '700':
        a = [70006, 80006, 90006]
    if massPoint == '750':
        a = [70007, 80007, 90007]
    if massPoint == '800':
        a = [70008, 80008, 90008]
    if massPoint == '900':
        a = [70019, 80019, 90019]
    if massPoint == '1000':
        a = [70009, 80009, 90009]
    if massPoint == '1100':
        a = [70010, 80010, 90010]
    if massPoint == '1200':
        a = [70020, 80020, 90020]
    if massPoint == '1300':
        a = [70011, 80011, 90011]
    if massPoint == '1400':
        a = [70012, 80012, 90012]
    if massPoint == '1500':
        a = [70013, 80013, 90013]
    if massPoint == '1600':
        a = [70021, 80021, 90021]
    if massPoint == '1800':
        a = [70022, 80022, 90022]
    if massPoint == '2000':
        a = [70014, 80014, 90014]
    if massPoint == '2250':
        a = [70023, 80023, 90023]
    if massPoint == '2500':
        a = [70015, 80015, 90015]
    if massPoint == '2750':
        a = [70016, 80016, 90016]
    if massPoint == '3000':
        a = [70017, 80017, 90017]
    if massPoint == '5000':
        a = [70018, 80018, 90018]
 
    return a

def runSingleMass(massPoint, dirName):
    
    configs = getConfigNumbers(massPoint)
    
    if len(configs) != 3:
        print "massPoint", massPoint, "not recognized. ABORT!!!!!!!"

    ####runMassPoint(configs[0], massPoint, dirName) # non-res
    if float(massPoint) < 1350:
    ##if float(massPoint) < 1050:
        runMassPoint(configs[1], massPoint, dirName) # Xhh700
    if float(massPoint) > 1350:
    ##if float(massPoint) > 1050:
        runMassPoint(configs[2], massPoint, dirName) # Xhh2000


def is_float(input):
  try:
    num = float(input)
  except ValueError:
    return False
  return True

################################################################
#####                        MAIN CODE                    ######
################################################################

if len(sys.argv) is not 2:
    print "Usage: runAllPoints <test/0/1/2/3/all>"
    exit(0)

runType = sys.argv[1]

# *** 1. Ask for name of target directory
inputDir = raw_input("Enter name of target dir (e.g. ): ")

# *** 2. Run mass points
#runSingleMass('250', inputDir)
#runSingleMass('300', inputDir)
#runSingleMass('400', inputDir)

if is_float(runType) and float(runType) >100:
    runSingleMass(runType, inputDir)

if runType == 'test': 
    runSingleMass('500', inputDir)
    #runSingleMass('600', inputDir)
    #runSingleMass('700', inputDir)
    #runSingleMass('750', inputDir)
    #runSingleMass('1300', inputDir)
    #runSingleMass('1500', inputDir)
    #runSingleMass('1800', inputDir)
    #runSingleMass('2000', inputDir)
    #runSingleMass('3000', inputDir)

if runType == '0' or runType == 'all':
    runSingleMass('500', inputDir)
    runSingleMass('600', inputDir)
    #runSingleMass('700', inputDir)
    runSingleMass('750', inputDir)
    runSingleMass('800', inputDir)

if runType == '1' or runType == 'all':      
    runSingleMass('900', inputDir)
    runSingleMass('1000', inputDir)
    runSingleMass('1100', inputDir)
    runSingleMass('1200', inputDir)
    runSingleMass('1300', inputDir)

if runType == '2' or runType == 'all':
    runSingleMass('1300', inputDir)
    runSingleMass('1400', inputDir)
    runSingleMass('1500', inputDir)
    runSingleMass('1600', inputDir)
    runSingleMass('1800', inputDir)

if runType == '3' or runType == 'all':
    runSingleMass('2000', inputDir)
    runSingleMass('2250', inputDir)
    runSingleMass('2500', inputDir)
    runSingleMass('2750', inputDir)
    runSingleMass('3000', inputDir)

#runSingleMass('5000', inputDir)

#runMassPoint(11822, '700', inputDir)
#runMassPoint(11823, '2000', inputDir)

