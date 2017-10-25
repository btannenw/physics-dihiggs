cd bbWWinput/

rlqb 'makeInputFile.C("Xhh600","Xhh700")'
rlqb 'makeInputFile.C("Xhh750","Xhh700")'
rlqb 'makeInputFile.C("Xhh1300","Xhh2000")'
rlqb 'makeInputFile.C("Xhh2000","Xhh2000")'

cd -

./SplitInputs -v 80005 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/aug16_rootfiles/
./SplitInputs -v 80007 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/aug16_rootfiles/
./SplitInputs -v 90011 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/aug16_rootfiles/
./SplitInputs -v 90014 -r Run2 -inDir /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/aug16_rootfiles/