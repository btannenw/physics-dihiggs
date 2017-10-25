cd bbWWinput/

#rlqb 'makeInputFile_noHH.C("Xhh700","Xhh700")'
#rlqb 'makeInputFile_noHH.C("Xhh750","Xhh700")'
rlqb 'makeInputFile_noHH.C("Xhh800","Xhh700")'
#rlqb 'makeInputFile_noHH.C("Xhh700","Xhh2000")'
#rlqb 'makeInputFile_noHH.C("Xhh750","Xhh2000")'
rlqb 'makeInputFile_noHH.C("Xhh800","Xhh2000")'

cd ..

#./SplitInputs -v 07007 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul12_rootfiles/
#./SplitInputs -v 07507 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul12_rootfiles/
./SplitInputs -v 08007 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul12_rootfiles/

#./SplitInputs -v 07002 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul12_rootfiles/
#./SplitInputs -v 07502 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul12_rootfiles/
./SplitInputs -v 08002 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul12_rootfiles/
