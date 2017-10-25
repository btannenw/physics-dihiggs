#include <iostream>
#include "splitter.hpp"
#include "splitter_Run2.hpp"
#include <TString.h>

/**
 * @file SplitInputs.C
 *
 * Split file to for limit setting
 * none of this one histogram nonsense
 *
 * This is UI to splitter class
 *
 */


using namespace std;

void printUsage() {
  cerr << "Usage:\n\t ./SplitInputs " << " -v|version [version] -r|run [Run1|Run2] -inDir|indir [inDir]" << endl;
}

/**
 * @brief Main splitter function
 *
 * Main function for splitting of inputs. The inputs will be split according to a
 * configuration file named @c inputConfigs/version.txt
 *
 * An old convention is still useable: the first parameter is the path to the @c inputs.txt config file,
 * typically of the form @c inputs/version/
 *
 * @param version	Version number of the inputs to split.
 *                    Or with the old convention, @c /path/to/inputs.txt (without @c inputs.txt)
 * @param type	Optional type of the splitter to run. Should be @c Run1 or @c Run2. Default is @c Run1
 * @param inDir Optional parameter, where the raw inputs are stored.
 *                  Default location is @c root://eosatlas//eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run1Paper/
 */
int main(int argc, char* argv[]) {

   if(argc < 2) {
      printUsage();
      //cerr << "" << endl;
      return 1;
   }

   /*TString version(argv[1]);

   TString type="Run1";
   if (argc >= 3)
     type = argv[2];

   TString inDir("root://eosatlas//eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run1Paper/");
   if (argc == 4)
     inDir = argv[3];*/
     
   // Spyros
   TString version = "";
   TString type    = "Run2";
   TString inDir   = "root://eosatlas//eos/atlas/atlascerngroupdisk/phys-higgs/HSG5/Run2/Moriond2016/";
   int arg = 1;
   while (arg < argc) {
     if     ( strcmp(argv[arg], "-v")     * strcmp(argv[arg], "-version") == 0 ) version = argv[arg+1];
     else if ( strcmp(argv[arg], "-r")     * strcmp(argv[arg], "-run")     == 0 ) type    = argv[arg+1];
     else if ( strcmp(argv[arg], "-inDir") * strcmp(argv[arg], "-indir")   == 0 ) inDir   = argv[arg+1];
     else    {
       cerr << "================================" << endl;
       cerr << "ERROR: Argument unrecognized" << endl;
       printUsage();
       return 1;
     }
     arg += 2; 
   }  

   if ( version.CompareTo("") == 0 ) {
     cerr << "================================" << endl;
     cerr << "ERROR: Version not provided" << endl;
     printUsage();
     return 1;
   }

   cout << "Configuration for " << argv[0] << " : " << endl;
   cout << "version	= " << version << endl;
   cout << "inDir	= " << inDir << endl;
   cout << endl;

   if(type == "Run1") {
     splitter mySplitter(version, inDir);
     mySplitter.SplitIt();
   }
   else if(type == "Run2") {
     splitter_Run2 mySplitter(version, inDir);
     mySplitter.SplitIt();
   }
   else {
     cerr << "Type " << type << " not known ! Please use Run1 or Run2 !" << endl;
   }

   return 0;
}

