cd bbWWinput/
rlqb 'makeInputFile.C("non-res")'
rlqb 'makeInputFile.C("Xhh700")'
rlqb 'makeInputFile.C("Xhh2000")'
cd ..
./SplitInputs -v 11821 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul07_rootfiles/
./SplitInputs -v 11822 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul07_rootfiles/
./SplitInputs -v 11823 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/jul07_rootfiles/