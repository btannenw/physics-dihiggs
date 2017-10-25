#include "engine.hpp"

#include <vector>
#include <algorithm>
#include <utility>
#include <iostream>
#include <cstdlib>
#include <set>
#include <iterator>

#include <RooWorkspace.h>
#include <RooRealVar.h>
#include <RooStats/ModelConfig.h>
#include <RooRealSumPdf.h>
#include <RooAbsArg.h>
#include <RooAbsPdf.h>
#include <RooArgSet.h>
#include <RooLinkedListIter.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TIterator.h>
#include <TObject.h>
#include <TString.h>

#include "systematic.hpp"
#include "systematichandler.hpp"
#include "properties.hpp"
#include "outputhandler.hpp"
#include "analysis.hpp"
#include "analysishandler.hpp"
#include "category.hpp"
#include "categoryhandler.hpp"
#include "configuration.hpp"
#include "samplehandler.hpp"
#include "sampleincategory.hpp"

class RooAbsData;
class Sample;


void Engine::MakeIt() {

  // first run through everything to get nominal histograms
  std::cout<<"made it to MakeIt()"<<std::endl;
  // add potentially user-defined vetos, i.e not use some samples for some categories
  // SysConfig objects can be used for that aim
  std::vector<SysConfig> vetos;
  std::cout<<"MakeIt(): 1"<<std::endl;
  // do not use multijet in 1 lepton inputs, but use multijetEl and multijetMu
  if ( Configuration::analysisType() != AnalysisType::VHbbRun2 )
     vetos.emplace_back(SysConfig("multijet", {{Property::nLep, 1}}));
  //vetos.emplace_back(SysConfig("Sig", {{Property::nTag, 1}}));
  std::cout<<"MakeIt(): 2"<<std::endl;
  for(auto& cat : m_categories) {
    for(auto& spair : m_samples) {
      Sample& s = spair.second;
      bool veto = std::any_of(std::begin(vetos), std::end(vetos),
                              [&s, &cat](const SysConfig& v){ return
                              SystematicHandler::match(s, v)
                              && SystematicHandler::match(cat, v);
                              });
      if(!veto)
        cat.tryAddSample(s);
    }
  }
  std::cout<<"MakeIt(): 3"<<std::endl;
  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  All nominal samples are added." << std::endl;
  std::cout << "  Now will compute binning and some statistics." << std::endl;

  // setup things: compute rebinnings, and whatever values may be useful
  m_categories.finalizeNominal();
  m_samples.finalizeNominal();

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  Binnings and statistics computed." << std::endl;
  std::cout << "  Now will make control plots." << std::endl;

  m_categories.makeControlPlots();

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  Plots made." << std::endl;

  bool mergeSamples = m_config.getValue("MergeSamples", false);
  if(mergeSamples) {
    std::cout << "  Now will prepare the merging of some samples." << std::endl;
    m_samples.declareSamplesToMerge();
    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  Preparation done.." << std::endl;
  }
  std::cout << "  Now will fill lists of systematics." << std::endl;

  bool useSysts = m_config.getValue("UseSystematics", true);
  bool useFloatingNorms = m_config.getValue("UseFloatNorms", true);

  // now define all systematics we will need
  m_systematics.listAllHistoSystematics(m_categories.regionTracker());
  // note the following one also defines the POIs
  m_systematics.listAllUserSystematics(m_categories.regionTracker(), useFloatingNorms);

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  Systematics lists complete." << std::endl;
  std::cout << "  Now will apply the POIs." << std::endl;

  std::set<TString> POIsNames;
  for(const auto& sysconfig : m_systematics.POIs()) {
    auto poiNames = m_samples.addUserSyst(sysconfig.first, sysconfig.second);
    POIsNames.insert(poiNames.begin(), poiNames.end());
  }

  TString decorrSys = m_config.getValue("DecorrSys", "None");
  if (decorrSys != "None") {
    std::cout << "INFO: Trying to decorrelate systematic: " << decorrSys.Data() << std::endl;
    std::vector<TString> decorrTypes = m_config.getStringList("DecorrTypes");
    std::vector<Properties::Property> props;
    for(auto& d : decorrTypes) {
      props.push_back(Properties::props_from_names.at(d));
    }
    if (props.size() > 0) {
      m_systematics.decorrSysForCategories(decorrSys, props, false);
    } else {
      std::cout << "WARNING: No DecorrTypes filled to decorrelate systematic: " << decorrSys.Data() << std::endl;
    }
  }

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  POIs have been applied to relevant samples." << std::endl;

  if(useSysts || useFloatingNorms) {

    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  Now will apply user-defined systematics." << std::endl;

    // apply user-defined systematics
    for(const auto& sysconfig : m_systematics.userSysts()) {
      const Systematic& sys = sysconfig.first;
      // if useFloatingNorms only, drop everything except flt systs
      std::cout<<"@@@@ userSysts, useSysts: "<<useSysts<<std::endl;
      if(useFloatingNorms && !useSysts && sys.type != SysType::flt) {
        continue;
      }
      m_samples.addUserSyst(sys, sysconfig.second);
    }

    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  User-defined systematics applied, except 3/2 ratios." << std::endl;

    if(useSysts) {

      for(const auto& sysconfig : m_systematics.ratios32()) {
        m_samples.add32Syst(sysconfig);
      }

      std::cout << "INFO::Engine::MakeIt:" << std::endl;
      std::cout << "  3/2 systematics applied." << std::endl;
    }
  }

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  Will now loop on categories." << std::endl;

  bool lumiWhenFloat = m_config.getValue("LumiForAll", true);
  bool skipUnknown = m_config.getValue("SkipUnkownSysts", false);
  bool doShapePlots = m_config.getValue("DoShapePlots", false);
  for(auto& c : m_categories) {
    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  In category " << c.name() << " ..." << std::endl;
    if(useSysts || useFloatingNorms) {
      std::cout << "INFO::Engine::MakeIt:" << std::endl;
      std::cout << "  Now will apply histo systematics." << std::endl;
      // apply histo systs
      c.applyHistoSysts(m_systematics, skipUnknown);
     
      // BBT: Aug 16, 2016- turn off pruning for the moment (currently ON)
      // First, simple pruning sample by sample
      std::cout << "INFO::Engine::MakeIt:" << std::endl;
      std::cout << "  Now will prune systematics for this category." << std::endl;
      if(Configuration::debug()) {
        std::cout << "   * Starting with simple pruning." << std::endl;
      }
      for(auto& sampleitr : c.samples()) {
        SampleInCategory& sic = sampleitr.second;
        sic.pruneSmallShapeSysts(0.005);
	sic.pruneOneSideShapeSysts();
        sic.pruneSmallNormSysts(0.005); // [ORIGINAL] 
	sic.pruneSameSignSysts();// [ORIGINAL] 
        //sic.pruneSmallNormSysts(0.01); // BBT, Sept 13 2016, this threshold still prunes
        if(!lumiWhenFloat) {
           sic.removeLumiWhenFloat();
        }
        //sic.pruneSmallShapeSysts_chi2();
        sic.pruneSmallShapeSysts_chi2_samesign();
        sic.pruneSpecialTests();
      }
      // Then complex pruning
      if(Configuration::debug()) {
        std::cout << "   * Moving to complex pruning." << std::endl;
      }
      if(Configuration::analysisType() == AnalysisType::VHbb || Configuration::analysisType() == AnalysisType::VHbbRun2) {
        c.pruneSmallSysts_Yao();
      }
      if(m_config.getValue("HTauTauPruning", false)) {
        c.pruneSmallShapeSysts_Htautau();
      }
      
    }
    
    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  Now will merge samples in this category." << std::endl;
    // then merging of samples
    m_samples.mergeSamplesInCategory(c);
    if(useSysts || useFloatingNorms) {
      for(auto& sampleitr : c.samples()) {
        SampleInCategory& sic = sampleitr.second;
        sic.pruneOneSideShapeSysts(); // should happen in very few circumstances
      }
    }

    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  All systematics collected and pruned in category " << c.name() <<"." << std::endl;
    if(Configuration::debug()) {
      std::cout << "INFO::Engine::MakeIt:" << std::endl;
      std::cout << "  Dump of all systematics after pruning." << std::endl;
      // Some pretty print of all systs that remain
      for(auto& sampleitr : c.samples()) {
        SampleInCategory& sic = sampleitr.second;
        sic.printSystematics();
      }
    }

    if(doShapePlots) {
      std::cout << "INFO::Engine::MakeIt:" << std::endl;
      std::cout << "  Plotting of systematic shapes requested! So doing it..." << std::endl;
      std::cout << "  for category " << c.name() << std::endl;
      m_categories.makeSystShapePlots(c);
      std::cout << "INFO::Engine::MakeIt:" << std::endl;
      std::cout << "  Systematic shape plots done." << std::endl;
    }
    else if (m_config.getValue("SmoothingControlPlots", false)) {
      std::cout << "INFO::Engine::MakeIt:" << std::endl;
      std::cout << "  Chi2 of sys vs. nom are being cached." << std::endl;
      m_categories.makeChi2TestsForSysts(c);
    }

    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  Now will write all the stuff to XML and norm files." << std::endl;
    m_categories.writeNormAndXMLForCat(c);

    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  End of treatment for category " << c.name() << "." << std::endl;
    m_categories.clearSystHistos(c);
  } // Loop on categories

  m_samples.finishMerging();

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  Done for all systematics." << std::endl;

  if (m_config.getValue("DoSystsPlots", false)) {
    std::cout << "INFO::Engine::MakeIt:" << std::endl;
    std::cout << "  Now will print summary plots of systematics." << std::endl;
    m_categories.makeSystStatusPlots();
  }

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  Plots made." << std::endl;
  std::cout << "  Now will collect the XML and write the driver file." << std::endl;

  // now save everything in norm file
  // and write XML at the same time
  // needs to delegate this to m_categories so that it produces a nice driver file
  auto driverAndWS = m_categories.writeNormAndXML(m_systematics.constNormFacts(), POIsNames);

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  Writing done." << std::endl;
  std::cout << "  Now will call hist2workspace." << std::endl;

  std::system(("hist2workspace -standard_form " + driverAndWS.first +
          " | grep -v \"INFO\" |grep -v \"RooHistFunc\" | grep -v \"RooGaussian\" | sed -e \"/Making OverallSys/,+1D\"|sed -e \"/Making HistoSys/,+1D\"|grep -v \"Writing sample\" | sed -e\"/Getting histogram/,+1D\" "
         ).Data() );

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  Workspace generated." << std::endl;
  std::cout << "  Now will do some modifications and move stuff around." << std::endl;

  PimpMyWS(driverAndWS.second);

  std::cout << "INFO::Engine::MakeIt:" << std::endl;
  std::cout << "  All Done !" << std::endl;
  std::cout << "  Now will clean our mess." << std::endl;

  // remove useless temporary root files in xml dir
  OutputConfig* outConf = OutputConfig::getInstance();
  std::system(("rm -rf "+outConf->xmlDir+"/*.root").Data());

}




void Engine::PimpMyWS(const TString& wsfilename) {
  using namespace RooStats;

  TFile* wsFile = TFile::Open(wsfilename);
  if (!wsFile || wsFile->IsZombie()) {
    std::cout << "ERROR::Engine::PimpMyWS: Problem opening file " << wsfilename << std::endl;
    throw;
  }

  RooWorkspace* w = nullptr;
  wsFile->GetObject("combined", w);
  if(!w){
    std::cout << "ERROR::Engine::PimpMyWS: Cannot find RooWorkspace in " << wsfilename << std::endl;
    throw;
  }
  ModelConfig* mc = dynamic_cast<ModelConfig*>(w->obj("ModelConfig"));
  if(!mc){
    std::cout << "ERROR::Engine::PimpMyWS: Cannot find ModelConfig in " << wsfilename << std::endl;
    throw;
  }

  // make sure floating norms are at nominal value of 1
  const RooArgSet* list = mc->GetNuisanceParameters();
  if (list != nullptr) {
    TIterator *itr = list->createIterator();
    RooRealVar* arg = 0;
    while ((arg=(RooRealVar*)itr->Next())) {
      if (!arg) continue;
      if(TString(arg->GetName()).Contains("ATLAS_norm")) { arg->setVal(1); }
      if(TString(arg->GetName()).Contains("gamma_stat")) { arg->setVal(1); }
    }
  }

  // Activate binned likelihood calculation for binned models
  RooFIter iter = w->components().fwdIterator() ;
  RooAbsArg* argu ;
  while((argu=iter.next())) {
    if (argu->IsA()==RooRealSumPdf::Class()) {
      argu->setAttribute("BinnedLikelihood") ;
    }
  }

  // save snapshot before any fit has been done
  // TODO do we really need this ?
  RooAbsPdf* simPdf = w->pdf("simPdf");
  RooAbsData* data = w->data("obsData");
  RooArgSet* params = simPdf->getParameters(*data) ;
  if(!w->loadSnapshot("snapshot_paramsVals_initial")) {
    w->saveSnapshot("snapshot_paramsVals_initial",*params);
  }
  
  // write to file
  // TODO Is this still needed?
  gDirectory->RecursiveRemove(w);
  
  TString massPoint = m_config.getValue("MassPoint", "125");
  TString outFileName(OutputConfig::getInstance()->workspaceDir+"/"+massPoint+".root");
  w->writeToFile(outFileName);

  wsFile->Close();

  TFile* outFile = TFile::Open(outFileName,"UPDATE");
  TDirectory* binDir = outFile->mkdir("binning");
  std::cout << "Write binning hists" << std::endl;
  for(auto& cat : m_categories) {
    std::cout << "\t" << cat.name() << std::endl;
    cat.writeBinHist( binDir );
  }
  binDir->Write();
  outFile->Close();
  //delete binDir;
  //delete outFile;


}



Engine::Engine(const TString& confFileName, const TString& version) :
  m_config(confFileName, version),
  m_analysis(m_config),
  m_samples(m_config),
  m_categories(m_config),
  m_systematics(m_config)
{
  std::cout << "start intialization 2" << std::endl;
  // initialize properly the binning tool once and for all
  AnalysisHandler::analysis().createBinningTool(m_config);
  std::cout << "created binningTool" << std::endl;
  // ... and the output config
  OutputConfig::createInstance(m_config);
  std::cout << "created Instance" << std::endl;
}

Engine::~Engine() {
  // get rid of the singletons
  OutputConfig::destruct();
}

