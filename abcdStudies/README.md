Instructions for running ABCD code for QCD estimation

1) Go to ABCDcode directory. This should be included in any updated version of CxAODCode-00-10-01-branch. If you're reading this README, you already have everything you need

2) The macros will produce plots and subdirectories in the topDir directory set on line 45 of abcdPlotter.C . This top-level directory needs to be created by the user and correctly specified in abcdPlotter.C

3) Several booleans in abcdPlotter.C [lines 48-58] 

   - normPlots               --> draws normalized plots (probably never need to set as true)
   - scaleTTbar              --> scale all ttbar distributions by the value set on line 203 of include/commonFileInput.h
   - onlyTTbar               --> use ttbar as only background (probably never need to set as true)
   - addSR                   --> THIS IS FALSE UNLESS PRODUCING UNBLINDED RESULTS
   - qcdMM                   --> calculate QCD via ABCD if false, use top group Matrix Method for QCD when true (must be provided in histogram file)
   - verbose                 --> when == true, lots and lots of comments dumped to screen. Really, a lot of comments.
   - useMixedFiles           --> Used to specify alternative input files (probably never need to set as true)
   - useVBSMethod            --> Use method for QCD estimation taken from VBS analysis (probably never need to set as true)
   - doSignalInjection       --> Do signal injection tests when == true
   - printPlots              --> When == true, print all plots. Printing plots takes time, so set to false if just running for numbers or debugging
   - dumpRootFile            --> When == true, produce .root file with QCD histograms usable in limit setting, plotting, etc.
   - doSystematics           --> When == true, evaluate nominal ABCD regions as well as d0 and MET variations to caluclate sysetmatics for ABCD method
   - doTtbarNFvsQCDstudy     --> When == true, set QCD to zero, evaluate ttbar normalization factor, return to beginning, scale ttbar, calculate QCD, and repeat until ttbar NF is stable
   - dumpTexFiles            --> When == true, produce ready-to-compile .tex files in latex/ subdirectory for yield tables, R_MC table, ABCD systematic table (when doSystematics == true), and ttbar NF vs QCD table (when doTtbarNFvsQCDstudy == true)
   
For most running, the only booleans a user will need to change should be printPlots, dumpRootFile, or dumpTexFiles.

4) Depending on the chosen options, set the input file to be read in the block between lines 107-123 in abcdPlotter.C 

5) the code can be run from the command line using root 'abcdPlotter.C("<optimizationStrategy>", "<signalSample>", "<lepType>", "<lepChannel>", "<variable>)' where the following are acceptable options
   - <optimizationStrategy> = optNoRes, opt700ichep, opt2000ichep, opt700, opt2000
   - <signalSample> = hhSM, hh500, hh600, hh700, ... , hh2000, ...
   - <lepType> = TightMM, IsoOR
   - <lepChannel> = el, mu, el_mu
   - <variable> = bbMass, bbPt, WWPt, WWMass, drbb, drww, hhMass, wlepmtben, MET

 the <variable> option can be left out and the code will run the ABCD estimation using bbMass by default. All other options must be specified. Inputs are checked against a stored list of strings, and the code exits with a relevant error message if an option is unknown. The optimization strategies opt700ichep and opt2000ichep refer to cutflows from the 2016 CONF note while opt700 and opt2000 refer to the cutflows re-optimized by Suyog in spring 2017. The lepType options set the input file used to read in histograms [see lines 107-123 in abcdPlotter.C]. The lepChannel option specifies whether the code plots electron channel only (el), muon channel only (mu), or the combined electron+muon channel (el_mu).

And that's it. Sourcing the script runAllPointsAllVars.sh will produce plots for all variables in the electron channel only using the optNoRes, opt700ichep, and opt2000ichep cutflows. The CP-updated input file (updated Mar 31, 2017) is provided on AFS at

AFS:  /afs/cern.ch/work/s/suyogs/diHiggs/data/histograms/hist-trunkProduction-data-bkgMC-allsignals.root

For any questions/help, please contact Ben Tannenwald (benjamin.tannenwald@cern.ch).

=============================================
       How-to's for common operations
=============================================

A) Adding a new variable:
   i) Add line for producing dummy histogram when not found in input file [block starts at line 79 in commonFileInput.h]. N_bins and range can be taken from makeMainHistogramsABCD in CxAODReader_VHbb/Root/AnalysisReader_VHbb.cxx
   ii) Add if statement for rebinning (if necessary/desired) in block from line 139-175 in include/commonFileInput.h. Rebinning can be either uniform or variable and examples exist for both

B) Change cuflow for given optimization
   i) Go to function setCutflowVector() in include/abcdPlotter.h and find the if statement corresponding to the optimization strategy you'd like to change. The cutflows are set as a vector of strings in include/abcdPlotter.h and the vector is used throughout the rest of the code, so changing it here is the only step necessary.
   ii) Add a line, change a line, change order to the push_back of the cutflow vector

C) ... if you come across something else and don't know how to do it, just ask Ben.  
