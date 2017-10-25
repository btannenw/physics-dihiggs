#include <iostream>
#include <TString.h>
#include "engine.hpp"

/**
 * @file MakeWorkspace.C
 *
 * Take input files and create a RooFit workspace with
 * all systematics included, after some smoothing/pruning,
 * with samples merged together, etc...
 *
 * This is UI for the WSMaking code
 *
 */


using namespace std;

/**
 * @brief Main workspace-making function
 *
 * Executable for the creation of workspaces. Workspaces are created according
 * to a configuration file, written in TEnv-readable format, stored by convention in
 * @c configs/ directory.
 * This executable is basically empty: just creates an instance of Engine, and run it.
 *
 * The options of the config file are described where they are needed in the code.
 * For an example of a full config file, and for the automatic generation of them,
 * see @c scripts/defaultAnalysis.py and @c scripts/AnalysisMgr.py
 *
 * @param configfile @c /path/to/configfile, e.g @c configs/higgs_8TeV_1_Systs_CUT_012.config
 * @param version An output version number/tag, e.g @c 42, or @c HSG7Final
 *
 */
int main(int argc, char* argv[]) {

   if(argc != 3) {
      cerr << "Usage: " << argv[0] << " <configfile> <version>" << endl;
      cerr << "<configfile>  = path/to/config/file.conf" << endl;
      cerr << "<version>    = i.e v0.0" << endl;
      return 1;
   }

   TString config(argv[1]);
   TString version(argv[2]);

   cout << "Configuration for " << argv[0] << " : " << endl;
   cout << "<configfile> = " << config << endl;
   cout << "<version>   = " << version << endl;
   cout << "additional one" << endl;

   Engine myEngine(config, version);
   std::cout << "configured the engine" << std::endl;

   myEngine.MakeIt();

   std::cout << "Made the engine" << std::endl;

   return 0;
}

