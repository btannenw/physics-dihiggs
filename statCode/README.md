Instructions for running the fit

1) go to whatever directory where you have the CxAODCode checked out
2) $> source rcSetup.sh
3) run the command
   $> svn co svn+ssh://svn.cern.ch/reps/atlasphys-hsg6/Physics/Higgs/HSG6/HDibosons/Analysis/DiHiggsWWbb/StatCode/trunk WSMaker
4) cd WSMaker
5) cp -r /afs/cern.ch/work/b/btannenw/hhbbWW_Analysis/CxAODCode-trunk_Apr8/WSMaker/bbWWinput/sep23_unblinded ./bbWWinput
6) Run (this needs to be done each time as it tells the fitter which scripts to use)
   $> source setup.sh HHWWbb
7) Run
   $> gmake -j5

8) To create histograms with the correct naming scheme for the fitter, you need to run the makeInputFile.C macro contained in subdir bbWWinput
9) Running the fit is a two step process: 
   i) first the inputs by region have to be made using the histogram files produced by the makeInputFile.C macro
  ii) second, the fit is run

example commands for both of these are given in simpleCommands.txt (found in the svn repo)

10) To produce inputs for all three mass points, run the following
  $> ./SplitInputs -v 80007 -r Run2 -inDir <global-address-to-your-CxAODCode>/bbWWinput/sep23_unblinded/
  $> ./SplitInputs -v 90011 -r Run2 -inDir <global-address-to-your-CxAODCode>/bbWWinput/sep23_unblinded/
  $> ./SplitInputs -v 90014 -r Run2 -inDir <global-address-to-your-CxAODCode>/bbWWinput/sep23_unblinded/

where 80007 is the 750 GeV sample in the low-mass selection, 90011 is the 1300 GeV sample in the high-mass selection, and 90014 is the 2000 GeV sample in the high-mass selection. These config numbers were a random choice by Ben. Numbers starting with 800* indiciate the low-mass selection while numbers starting with 900* indicate the high-mass selection. Again, random choice by Ben.

11) There is a python script written to evaulate points simultaneously, runAllPoints.py . Before personal use, line 21 must be edited to change the global path to wherever your local copy of CxAODCode is. The option stored in svn is runs the expected and observed limits with all object and modelling systematics applied. By changing options in scripts/HHWWbb/Ananlysis_HH.py, systematics can be turned on/off and running expected only vs expected+observed can be specificed. Once you have the desired configuration, simply run
    $> python runAllPoints.py <option>
where <option> is either 0,1,2,3, or test. These can be edited in the python script for your mass point of choice. Once run, you will be prompted for an directory name. If you enter a name already chosen, the code will abort. I suggest naming structures like dd-mm-yy-<run_descriptor>, but listen to your heart. The best naming convention is the one you already follow.

12) The code will produce individual runs and then merge the results in a conveniently named directory for the limit setting plot to run. Log files are produced under the logs/ directory. Root files are produced under root_files/ . Some plots by region can be found under plots/ .

13) To produce the limit plots, you need to use the scripts/HHWWbb/limitObs.py script. Here, line 271 must be edited to point to whatever naming scheme you used when running the code in step 10. Adding second limit lines is done by copying the same line, changing the name to the appropriate directory, and changing same=False --> same=True . Other options exist for overlaying combined ATLAS Run 1 results and CMS Run2 hh->bbWW results. Creating the plots is then as simple as
    $> python scripts/HHWWbb/limitObs.py

limit.png and limit.eps will be produced containing the famous Brazilian limit plots.

More information about the framework available at https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/WorkspaceMaker

Any questions? Email benjamin.tannenwald@cern.ch
