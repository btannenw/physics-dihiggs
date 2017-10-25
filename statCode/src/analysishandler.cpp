#include "analysishandler.hpp"

#include <iostream>

#include "analysis_impl.hpp"
#include "analysis.hpp"
#include "configuration.hpp"

// include all existing derived classes here !

#include "regionnamesparser_vhbb.hpp"
#include "systematiclistsbuilder_vhbb.hpp"
#include "inputshandler_paper.hpp"
#include "regiontracker_vhbb.hpp"
#include "samplesbuilder_vhbb.hpp"
#include "binning_vhbb.hpp"

#include "inputshandler_run2.hpp"
#include "regionnamesparser_run2.hpp"
// #include "samplesbuilder_vhres.hpp"
// #include "binning_vhres.hpp"
// #include "systematiclistsbuilder_vhres.hpp"
// #include "regionnamesparser_vhres.hpp"
// #include "samplesbuilder_hvt.hpp"
#include "binning_hvt.hpp"
// #include "systematiclistsbuilder_hvt.hpp"
// #include "regionnamesparser_hvt.hpp"

#include "samplesbuilder_vhbbrun2.hpp"
#include "binning_vhbbrun2.hpp"
#include "systematiclistsbuilder_vhbbrun2.hpp"
#include "regiontracker_vhbbrun2.hpp"

#include "regiontracker_azh.hpp"
#include "samplesbuilder_azh.hpp"
#include "binning_azh.hpp"
#include "systematiclistsbuilder_azh.hpp"

#include "samplesbuilder_monoh.hpp"
#include "binning_monoh.hpp"
#include "systematiclistsbuilder_monoh.hpp"
#include "regiontracker_monoh.hpp"

#include "samplesbuilder_hh.hpp"
#include "binning_hh.hpp"
#include "systematiclistsbuilder_hh.hpp"

#include "samplesbuilder_htt.hpp"
#include "binning_htt.hpp"
#include "systematiclistsbuilder_htt.hpp"

#include "samplesbuilder_vbfgamma.hpp"
#include "binning_vbfgamma.hpp"
#include "systematiclistsbuilder_vbfgamma.hpp"
#include "regiontracker_vbfgamma.hpp"
#include "regionnamesparser_vbfgamma.hpp"
#include "inputshandler_vbfgamma.hpp"

#include "samplesbuilder_hhwwbb.hpp"
#include "binning_hhwwbb.hpp"
#include "systematiclistsbuilder_hhwwbb.hpp"

std::unique_ptr<Analysis> AnalysisHandler::m_analysis(nullptr);

AnalysisHandler::~AnalysisHandler() {}

AnalysisHandler::AnalysisHandler(const Configuration& conf)
{
  std::cout << "start AnalysisHandler 1" << std::endl;

  // don't do anything if the analysis has been already instanciated
  if(m_analysis != nullptr) {
    return;
  }
  std::cout << "start AnalysisHandler 2" << std::endl;

  using AnalysisVHbb = Analysis_Impl<RegionNamesParser_VHbb, InputsHandlerPaper,
        RegionTracker_VHbb, SystematicListsBuilder_VHbb, SamplesBuilder_VHbb, BinningTool_VHbb>;

  // using AnalysisHVT = Analysis_Impl<RegionNamesParser_VHRes, InputsHandlerRun2,
        // RegionTracker_VHbb, SystematicListsBuilder_VHRes, SamplesBuilder_VHRes, BinningTool_VHRes>;
  using AnalysisHVT = Analysis_Impl<RegionNamesParser_Run2, InputsHandlerRun2,
        RegionTracker_VHbb, SystematicListsBuilder_AZh, SamplesBuilder_AZh, BinningTool_AZh>;
        // RegionTracker_VHbb, SystematicListsBuilder_AZh, SamplesBuilder_AZh, BinningTool_HVT>;

  using AnalysisAZh = Analysis_Impl<RegionNamesParser_Run2, InputsHandlerRun2,
        RegionTracker_AZh, SystematicListsBuilder_AZh, SamplesBuilder_AZh, BinningTool_AZh>;

  using AnalysisVHbbRun2 = Analysis_Impl<RegionNamesParser_Run2, InputsHandlerRun2,
        RegionTracker_VHbbRun2, SystematicListsBuilder_VHbbRun2, SamplesBuilder_VHbbRun2, BinningTool_VHbbRun2>;

  using AnalysisMonoH = Analysis_Impl<RegionNamesParser_Run2, InputsHandlerRun2,
        RegionTracker_MonoH, SystematicListsBuilder_MonoH, SamplesBuilder_MonoH, BinningTool_MonoH>;

  using AnalysisHH = Analysis_Impl<RegionNamesParser_Run2, InputsHandlerRun2,
    RegionTracker_VHbb, SystematicListsBuilder_HH, SamplesBuilder_HH, BinningTool_HH>;
    //RegionTracker_VHbb, SystematicListsBuilder_HH, SamplesBuilder_HHWWbb, BinningTool_HH>; // BBT, may 17 2016: testing bbWW tools in HH configuration
    //RegionTracker_VHbb, SystematicListsBuilder_HHWWbb, SamplesBuilder_HHWWbb, BinningTool_HH>; // BBT, may 17 2016: testing bbWW tools in HH configuration
  
  using AnalysisHtautau = Analysis_Impl<RegionNamesParser_Run2, InputsHandlerRun2,
    RegionTracker_VHbb, SystematicListsBuilder_Htautau, SamplesBuilder_Htautau, BinningTool_Htautau>;

  using AnalysisHtautau = Analysis_Impl<RegionNamesParser_Run2, InputsHandlerRun2,
        RegionTracker_VHbb, SystematicListsBuilder_Htautau, SamplesBuilder_Htautau, BinningTool_Htautau>;

  using AnalysisVBFGamma = Analysis_Impl<RegionNamesParser_VBFGamma, InputsHandlerVBFGamma,
        RegionTracker_VBFGamma, SystematicListsBuilder_VBFGamma, SamplesBuilder_VBFGamma, BinningTool_VBFGamma>;

  //BBT make new objects from Systematics on, can copy Carl's probably...
  using AnalysisHHWWbb = Analysis_Impl<RegionNamesParser_Run2, InputsHandlerRun2,
    RegionTracker_VHbb, SystematicListsBuilder_HHWWbb, SamplesBuilder_HHWWbb, BinningTool_HHWWbb>;
  
  switch(Configuration::analysisType()) {
  case AnalysisType::None:
    std::cout << "ERROR:  AnalysisHandler::AnalysisHandler" << std::endl;
    std::cout << "Configuration::AnalysisType has not been defined !" << std::endl;
    throw;
    break;
  case AnalysisType::VHbb:
      m_analysis.reset(new AnalysisVHbb(conf));
      break;
    case AnalysisType::HVT:
      m_analysis.reset(new AnalysisHVT(conf));
      break;
    case AnalysisType::AZh:
      m_analysis.reset(new AnalysisAZh(conf));
      break;
    case AnalysisType::VHbbRun2:
      m_analysis.reset(new AnalysisVHbbRun2(conf));
      break;
    case AnalysisType::MonoH:
      m_analysis.reset(new AnalysisMonoH(conf));
      break;
    case AnalysisType::HH:
      m_analysis.reset(new AnalysisHH(conf));
      break;
    case AnalysisType::Htautau:
      m_analysis.reset(new AnalysisHtautau(conf));
      break;
    case AnalysisType::VBFGamma:
      m_analysis.reset(new AnalysisVBFGamma(conf));
      break;
      // BBT May 10, make new for bbww
    case AnalysisType::HHWWbb:
      m_analysis.reset(new AnalysisHHWWbb(conf));
      std::cout << "################ Configuragion::analysisType : HHWWbb" << std::endl;
      break;
    default:
      std::cout << "ERROR:  AnalysisHandler::AnalysisHandler" << std::endl;
      std::cout << "No derived analysis has been implemented for this AnalysisType yet !" << std::endl;
      std::cout << "Please provide one." << std::endl;
      throw;
      break;
  }
  
}
