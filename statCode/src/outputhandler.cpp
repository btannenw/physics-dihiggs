#include "outputhandler.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <TH1.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TString.h>

#include "properties.hpp"
#include "category.hpp"
#include "configuration.hpp"
#include "systematic.hpp"

OutputConfig* OutputConfig::the_instance = nullptr;

OutputConfig::OutputConfig(Configuration& conf) {
  deleteFiles = conf.getValue("DeleteNormFiles", true);

  TString fullVersion(conf.getValue("InputVersion", "10000") + "." + conf.version());

  std::stringstream ss;
  if(deleteFiles)
    ss << "/tmp/" << std::getenv("USER") << "/normfiles/";
  else
    ss << "inputs/";
  ss << fullVersion << "/";
  normDir = ss.str().c_str();
  // delete and recreate that directory
  std::system(("rm -rf "+normDir).Data());
  std::system(("mkdir -vp "+normDir).Data());

  TString massPoint = conf.getValue("MassPoint", "125");
  xmlDir = TString("xml/") + fullVersion + "/" + massPoint;
  // TODO do we want to delete any possibly existing xml directory ? currently WSMaker does
  // not
  //std::system(("rm -rf "+xmlDir).Data());
  std::system(("mkdir -vp "+xmlDir).Data());

  workspaceDir = TString("workspaces/") + fullVersion + "/combined";
  // TODO do we want to delete any possibly existing workspaces directory ? currently
  // WSMaker does not
  //std::system(("rm -rf "+workspaceDir).Data());
  std::system(("mkdir -vp "+workspaceDir).Data());

  plotDir = TString("plots/") + fullVersion;
  // TODO do we want to delete any possibly existing plots directory ? currently
  // WSMaker does not
  //std::system(("rm -rf "+plotDir).Data());
  std::system(("mkdir -vp "+plotDir).Data());

  std::system(("cp HistFactorySchema.dtd "+xmlDir).Data());
}

OutputConfig::~OutputConfig()
{}

OutputConfig* OutputConfig::createInstance(Configuration& conf) {
  if( !the_instance ) {
    std::cout << "INFO:    OutputConfig::createInstance() OutputConfig pointer is NULL." << std::endl;
    std::cout << "         Will instanciate the OutputConfig service first." << std::endl;
    the_instance = new OutputConfig(conf);
  }
  return the_instance;
}

OutputConfig* OutputConfig::getInstance() {
  if( !the_instance ) {
    std::cout << "ERROR:    OutputConfig::getInstance() OutputConfig pointer is NULL." << std::endl;
    std::cout << "          It should be initialized with a Configuration first." << std::endl;
    throw;
  }
  return the_instance;
}

void OutputConfig::destruct() {
  if( the_instance != nullptr) {
    delete the_instance;
    the_instance = nullptr;
  }
}

OutputHandler::OutputHandler() :
  m_file(), m_xml(), m_xmlName(), m_fname(), m_status(Status::NotInit),
  m_deleteFile(false)
{}

OutputHandler::OutputHandler(const Category& cat) :
  m_file(), m_xml(), m_xmlName(), m_fname(), m_status(Status::NotInit),
  m_deleteFile(false)
{

  const PropertiesSet& pset = cat.properties();

  OutputConfig* conf = OutputConfig::getInstance();

  m_fname = conf->normDir+"/Norm"+pset.getPropertiesTag()+".root";
  m_file = std::unique_ptr<TFile>( TFile::Open(m_fname, "recreate"));
  m_xmlName = conf->xmlDir+"/"+cat.name()+".xml";
  //m_xml = std::ofstream(m_xmlName);
  //std::ofstream tmpfile(m_xmlName);
  //m_xml.swap(tmpfile);
  m_xml = std::unique_ptr<std::ofstream>(new std::ofstream(m_xmlName));

  *m_xml << "<!DOCTYPE Channel SYSTEM 'HistFactorySchema.dtd'>\n\n"
    << "<Channel Name=\"" << cat.name() << "\" HistoPath=\"\">\n\n";

  m_deleteFile = conf->deleteFiles;

  m_status = Status::Ready;
}

OutputHandler::~OutputHandler() {
  // if init not even done, then nothing is to be deleted
  if(m_status == Status::NotInit) {
    return;
  }
  if(m_status == Status::Ready) {
    closeFiles();
  }
  if(m_status != Status::Closed) {
    std::cout << std::endl << "ERROR::OutputHandler::~OutputHandler:" << std::endl;
    std::cout << "  Files were left in an unspecified state." << std::endl;
    std::cout << "  This is not expected !" << std::endl << std::endl;
  }
  if(m_deleteFile) {
    std::system(("rm -rf "+m_fname).Data());
  }
}

OutputHandler& OutputHandler::operator=(OutputHandler&& other) {
  m_file = std::move(other.m_file);
  m_xml = std::move(other.m_xml);
  m_xmlName = other.m_xmlName;
  m_fname = other.m_fname;
  m_status = other.m_status;
  m_deleteFile = other.m_deleteFile;
  other.m_status = Status::NotInit;
  other.m_deleteFile = false;
  return *this;
}

TString OutputHandler::closeFiles() {
  assertStatus(Status::Ready);
  m_file->Close("R");
  *m_xml << "</Channel>\n";
  m_xml->close();
  m_status = Status::Closed;
  return m_xmlName;
}

void OutputHandler::addMCStatThresh(float thresh) {
  assertStatus(Status::Ready);
  TString constraintType = "Poisson"; // TODO some flag to restore here ?
  *m_xml << "<StatErrorConfig RelErrorThreshold=\"" << thresh;
  *m_xml << "\" ConstraintType=\"" << constraintType << "\" />\n\n";
}

void OutputHandler::addData(const TString& name, const TH1* hdata) {
  assertStatus(Status::Ready);
  writeHisto(name, hdata);
  *m_xml << "<Data InputFile=\"" << m_fname
    << "\" HistoName=\"" << name << "\" HistoPath=\"\" />\n\n";
}

void OutputHandler::beginSample(const TString& name, const TH1* hnom) {
  assertStatus(Status::Ready);
  writeHisto(name, hnom);
  *m_xml << "<Sample Name=\"" << name << "\" InputFile=\"" << m_fname
    << "\" HistoName=\"" << name << "\" NormalizeByTheory=\"False\" >\n";
  m_status = Status::InSample;
}

void OutputHandler::addSyst(const Systematic& sys) {
  assertStatus(Status::InSample);
  switch(sys.type) {
    case SysType::norm:
      addNormSyst(sys);
      break;
    case SysType::shape:
      addShapeSyst(sys);
      break;
    case SysType::flt:
      addFltSyst(sys);
      break;
  }
}

void OutputHandler::addNormSyst(const Systematic& sys) const {
  *m_xml << "<OverallSys Name=\"" << sys.name << "\" High=\""
    << sys.var_up << "\" Low=\"" << sys.var_do << "\"/>\n";
}

void OutputHandler::addShapeSyst(const Systematic& sys) const {
  if(sys.shape_up == nullptr || sys.shape_do == nullptr) {
    std::cout << std::endl << "ERROR::OutputHandler::addShapeSyst:" << std::endl;
    std::cout << "  Function cannot handle one-sided shape systs yet" << std::endl;
    std::cout << "  Please FIXME ! I promise it is not hard" << std::endl << std::endl;
    throw;
    // FIXME
  }

  *m_xml << "<HistoSys Name=\"" << sys.name << "\" HistoNameLow=\""
    << sys.shape_do->GetName() << "\" HistoNameHigh=\"" << sys.shape_up->GetName()
    << "\" HistoPathLow=\"/" << sys.name << "__1down/\" HistoPathHigh=\"/"
    << sys.name << "__1up/\"/>\n";

  writeHisto(sys.shape_do->GetName(), sys.shape_do.get(), sys.name+"__1down");
  writeHisto(sys.shape_up->GetName(), sys.shape_up.get(), sys.name+"__1up");
}

void OutputHandler::addFltSyst(const Systematic& sys) const {
  *m_xml << "<NormFactor Name=\"" << sys.name << "\" Val=\"" << sys.init
    << "\" Low=\"" << sys.var_do << "\" High=\"" << sys.var_up << "\" Const=\"True\" />\n";
}

void OutputHandler::closeSample() {
  assertStatus(Status::InSample);
  *m_xml << "</Sample>\n\n";
  m_status = Status::Ready;
}

void OutputHandler::activateStatErr(bool activate) {
  assertStatus(Status::InSample);
  if(activate) {
    *m_xml << "<StatError Activate=\"True\"/>\n";
  }
  else {
    *m_xml << "<StatError Activate=\"False\"/>\n";
  }
}

void OutputHandler::writeHisto(const TString& name, const TH1* h, const TString& sysname) const {
  TDirectory* targetDir = m_file->GetDirectory(sysname);
  if(targetDir == nullptr) {
    targetDir = m_file->mkdir(sysname);
  }
  targetDir->cd();
  h->Write(name);
}

void OutputHandler::assertStatus(Status s) {
  if(m_status != s) {
    std::cout << std::endl << "ERROR::OutputHandler:" << std::endl;
    std::cout << "  Was expected status " << static_cast<int>(s) << std::endl;
    std::cout << "  Got status " << static_cast<int>(m_status) << std::endl << std::endl;
    throw;
  }
}



