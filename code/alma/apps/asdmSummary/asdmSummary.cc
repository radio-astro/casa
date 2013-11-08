#include "ASDM.h"
using namespace asdm;

#include "AntennaRow.h"
#include "AntennaTable.h"
#include "ConfigDescriptionRow.h"
#include "ConfigDescriptionTable.h"
#include "DataDescriptionRow.h"
#include "DataDescriptionTable.h"
#include "ExecBlockRow.h"
#include "ExecBlockTable.h"
#include "MainRow.h"
#include "MainTable.h"
#include "PolarizationRow.h"
#include "PolarizationTable.h"
#include "ScanRow.h"
#include "ScanTable.h"
#include "SpectralWindowRow.h"
#include "SpectralWindowTable.h"
#include "StationRow.h"
#include "StationTable.h"
#include "SubscanRow.h"
#include "SubscanTable.h"


#include "CAntennaMake.h"
using namespace AntennaMakeMod;

#include "CAtmPhaseCorrection.h"
using namespace AtmPhaseCorrectionMod;

#include "CCorrelationMode.h"
using namespace CorrelationModeMod;

#include "CStokesParameter.h"
using namespace StokesParameterMod;

#include "CFrequencyReferenceCode.h";
using namespace FrequencyReferenceCodeMod;

#include "CScanIntent.h"
using namespace ScanIntentMod;

#include "CSpectralResolutionType.h"
using namespace SpectralResolutionTypeMod;

#include "CSubscanIntent.h"
using namespace SubscanIntentMod;

#include "CTimeSampling.h"
using namespace TimeSamplingMod;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/foreach.hpp>

#include <casa/Logging/StreamLogSink.h>
#include <casa/Logging/LogSink.h>
using namespace casa;

using namespace std;
string appName;

// A facility to get rid of blanks at start and end of a string.
// 
string lrtrim(std::string& s,const std::string& drop = " ")
{
  std::string r=s.erase(s.find_last_not_of(drop)+1);
  return r.erase(0,r.find_first_not_of(drop));
}

void info (const string& message) {
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
}

void error(const string& message) {
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
  //os << LogIO::POST;
  exit(1);
}

#include <iostream>
#include <sstream>

ostringstream errstream;
ostringstream infostream;
using namespace std;

void antennaSummary(const ExecBlockRow* eb_p) {
  ASDM& ds = eb_p->getTable().getContainer();
  AntennaTable& aT = ds.getAntenna();
  StationTable& sT = ds.getStation();

  infostream.str("");
  const vector<Tag> antennaIds = eb_p->getAntennaId();
  AntennaRow * antenna_p = NULL ;
  StationRow * station_p = NULL ;
  infostream << endl;
  infostream << antennaIds.size() << " antennas have been used in this exec block." << endl;
  infostream << "        Id     Name         Make Station    Diameter         X              Y             Z" << endl;
  for (unsigned int i = 0; i < antennaIds.size(); i++) {
    antenna_p = aT.getRowByKey(antennaIds[i]);
    station_p = sT.getRowByKey(antenna_p->getStationId());
    vector<Length> position = station_p->getPosition();
    //infostream.fill(''); 
    infostream.width(12);infostream << antenna_p->getAntennaId() ;
    infostream.width(6); infostream.setf(ios::right); infostream   << antenna_p->getName() ;
    infostream.width(13); infostream  << CAntennaMake::name(antenna_p->getAntennaMake()) ;
    infostream.width(6); infostream   << station_p->getName() ;
    infostream.width(10); infostream.precision(10); infostream << antenna_p->getDishDiameter() ;
    infostream.width(15); infostream.precision(10); infostream << position[0] ;
    infostream.width(15); infostream.precision(10); infostream << position[1] ;
    infostream.width(15); infostream.precision(10); infostream << position[2] << endl;
  }
    info(infostream.str());
}

template<typename Enum, typename EnumHelper> 
void output1 (typename  vector<Enum>::iterator begin, typename vector<Enum>::iterator end, ostringstream & oss) {
  if (begin == end) return;
  oss << ',' << EnumHelper::name(*begin);
  output1<Enum, EnumHelper>(begin+1, end, oss);    
} 

template<typename Enum, typename EnumHelper> 
void output (typename std::vector<Enum>::iterator begin, typename std::vector<Enum>::iterator end, std::ostringstream & oss) {
  if (begin == end) return;
  oss << EnumHelper::name(*begin);
  output1<Enum, EnumHelper>(begin+1, end, oss);    
} 


template<typename T>
void output1 (typename vector<T>::iterator begin, typename vector<T>::iterator end, ostringstream& oss) {
  if (begin == end) return;
  oss << "," << *begin;
  output1<T> (begin+1, end, oss);
}

template<typename T>
void output (typename vector<T>::iterator begin, typename vector<T>::iterator end, ostringstream& oss) {
  if (begin == end) return;
  oss << *begin;
  output1<T> (begin+1, end, oss);
}

typedef struct SpectralWindowSummaryStruct {
  int		numChan;
  string	measFreqRef;
  Frequency     firstChan;
  Frequency	chanWidth;
  Frequency	refFreq;
} SpectralWindowSummary;

SpectralWindowSummary spectralWindowSummary(SpectralWindowRow * spw_p) {
  SpectralWindowSummary result;
  
  result.numChan = spw_p->getNumChan();
  
  if (spw_p->isChanFreqStartExists()) 
    result.firstChan = spw_p->getChanFreqStart();
  else 
    if (spw_p->isChanFreqArrayExists())
      result.firstChan = spw_p->getChanFreqArray()[0];
    else
      result.firstChan = Frequency(0.0);

  if (spw_p->isChanWidthArrayExists())
    result.chanWidth = spw_p->getChanWidthArray()[0];
  else 
    if (spw_p->isChanWidthExists())
      result.chanWidth = spw_p->getChanWidth();
    else
      result.chanWidth = Frequency(0.0);

  if (spw_p->isMeasFreqRefExists())
    result.measFreqRef = CFrequencyReferenceCode::name(spw_p->getMeasFreqRef());
  else
    result.measFreqRef = "TOPO";

  result.refFreq = spw_p->getRefFreq();
  
  return result;
}

bool notNull(int n) { return n != 0 ; }

void mainSummary(ExecBlockRow* eb_p, int scanNumber, int subscanNumber) {

  ASDM& ds = eb_p->getTable().getContainer();
  
  Tag ebId = eb_p->getExecBlockId();

  const vector<MainRow *>& mains = ds.getMain().get();
  vector<MainRow *> eb_mains;

  BOOST_FOREACH(MainRow* main_p, mains) {
    if ( main_p->getExecBlockId() == ebId && main_p->getScanNumber() == scanNumber && main_p->getSubscanNumber() == subscanNumber )
      eb_mains.push_back(main_p);
  }

  DataDescriptionTable& ddT = ds.getDataDescription();
  PolarizationTable& polT = ds.getPolarization();
  SpectralWindowTable& spwT = ds.getSpectralWindow();
  ConfigDescriptionTable& cfgDescT = ds.getConfigDescription();

  BOOST_FOREACH ( MainRow* main_p, eb_mains ) {
    infostream.str("");
    infostream << endl;
    infostream << "\t\t Binary data in " << main_p->getDataUID().getEntityId() << endl;
    infostream << "\t\t Number of integrations : " << main_p->getNumIntegration() << endl;
    infostream << "\t\t Time sampling : " << CTimeSampling::name(main_p->getTimeSampling()) << endl;
    ConfigDescriptionRow* cfgDesc_p = cfgDescT.getRowByKey(main_p->getConfigDescriptionId());
    infostream << "\t\t Correlation Mode : " << CCorrelationMode::name(cfgDesc_p->getCorrelationMode()) << endl;
    infostream << "\t\t Spectral resolution type : " << CSpectralResolutionType::name(cfgDesc_p->getSpectralType()) << endl;
    infostream << "\t\t Atmospheric phase correction : " ;
    vector<AtmPhaseCorrection> apcs = cfgDesc_p->getAtmPhaseCorrection();
    output<AtmPhaseCorrection, CAtmPhaseCorrection>(apcs.begin(), apcs.end(), infostream);
    infostream << endl;
    info(infostream.str());

    vector<Tag> ddIds = cfgDesc_p->getDataDescriptionId();
    BOOST_FOREACH ( Tag ddId, ddIds ) {
      DataDescriptionRow * dd_p = ddT.getRowByKey(ddId);
      SpectralWindowRow * spw_p = spwT.getRowByKey(dd_p->getSpectralWindowId());
      PolarizationRow * p_p = polT.getRowByKey(dd_p->getPolOrHoloId());
      infostream.str("");
      SpectralWindowSummary spwSummary = spectralWindowSummary(spw_p);
      infostream << "\t\t " << spw_p->getSpectralWindowId() << " : numChan = " << spwSummary.numChan
		 << ", frame = " << spwSummary.measFreqRef
		 << ", firstChan = " << spwSummary.firstChan
		 << ", chandWidth = " << spwSummary.chanWidth
		 << " x " 
		 << p_p->getPolarizationId() << " : corr = " ; 
      vector<StokesParameter> corrType = p_p->getCorrType();
      output<StokesParameter, CStokesParameter>(corrType.begin(), corrType.end(), infostream);
      infostream << endl;
      info(infostream.str());
    }
  }
}

void subscanSummary(ExecBlockRow* eb_p, int scanNumber) {
      
  ASDM& ds = eb_p->getTable().getContainer();
  Tag ebId = eb_p->getExecBlockId();

  const vector<SubscanRow *>& subscans = ds.getSubscan().get();
  vector<SubscanRow *> eb_subscans;
  BOOST_FOREACH (SubscanRow * sscan_p, subscans) {
    if (sscan_p->getExecBlockId() == ebId && sscan_p->getScanNumber() == scanNumber) 
      eb_subscans.push_back(sscan_p);
  }

  BOOST_FOREACH (SubscanRow* sscan_p, eb_subscans) {
    infostream.str("");
    infostream << "\tSubscan #" << sscan_p->getSubscanNumber()
	       << " from " << sscan_p->getStartTime().toFITS()
	       << " to " << sscan_p->getEndTime().toFITS()
	       << endl;
    infostream << "\t\tIntent : " << CSubscanIntent::name(sscan_p->getSubscanIntent()) << endl;
    infostream << "\t\tNumber of integrations : " << sscan_p->getNumIntegration() << endl;
    vector<int> numSubintegration = sscan_p->getNumSubintegration();
    if (find_if(numSubintegration.begin(), numSubintegration.end(), notNull) != numSubintegration.end()) {
      infostream << "\t\tNumber of subintegrations per integration : ";
      output<int>(numSubintegration.begin(), numSubintegration.end(), infostream);
      infostream << endl;
    }
    info(infostream.str()); 

    mainSummary(eb_p, scanNumber, sscan_p->getSubscanNumber());
  }

}


void scanSummary(ExecBlockRow* eb_p) {

  ASDM& ds = eb_p->getTable().getContainer();  
  Tag ebId = eb_p->getExecBlockId();

  const vector<MainRow *>& mains = ds.getMain().get();
  vector<MainRow *> eb_mains;

  BOOST_FOREACH(MainRow* main, mains) {
    if ( main->getExecBlockId() == ebId) eb_mains.push_back(main);
  }
  
  const vector<ScanRow*>& scans = ds.getScan().get();
  vector<ScanRow *> eb_scans;
  BOOST_FOREACH(ScanRow* scan, scans) {
    if ( scan->getExecBlockId() == ebId) eb_scans.push_back(scan);
  }

  infostream.str("");
  infostream << endl;
  infostream << "Number of scans in this exec Block : " << eb_scans.size() << endl;
  info(infostream.str());
  if (eb_scans.size() > 0) {
    BOOST_FOREACH (ScanRow* scan_p, eb_scans) {
      infostream.str("");
      infostream << endl;
      infostream << "scan #" << scan_p->getScanNumber()
		 << " from " << scan_p->getStartTime().toFITS()
		 << " to " <<  scan_p->getEndTime().toFITS()
		 << endl;
      
      vector<ScanIntent> scis = scan_p->getScanIntent();
      if (scis.size() > 0) {
	infostream << "\tIntents : ";
	output<ScanIntent, CScanIntent>(scis.begin(), scis.end(), infostream);
	infostream << endl;
      }
      
      if ( scan_p->isFieldNameExists() ) {
	vector<string> fields = scan_p->getFieldName();
	if (fields.size() > 0) {
	  infostream << "\tFields : ";
	  output<string>(fields.begin(), fields.end(), infostream);
	  infostream << endl;
	}
      }
      
      if ( scan_p->isSourceNameExists() ) {
	infostream << "\tSources : " << scan_p->getSourceName() << endl; 
      }
      info(infostream.str());
      subscanSummary(eb_p, scan_p->getScanNumber());
    }
  }
}

void execBlockSummary(const ASDM& ds) {
  infostream.str("");

  const vector<ExecBlockRow*>& ebs = ds.getExecBlock().get();
  for (unsigned int i = 0; i < ebs.size(); i++) {
    ExecBlockRow* eb_p = ebs[i];
    infostream << "\n";
    infostream << "Exec Block : " << eb_p->getExecBlockId() << endl;
    infostream << "Telescope : " << eb_p->getTelescopeName() << endl;
    infostream << "Configuration name : " << eb_p->getConfigName() << endl;
    infostream << "Observer name : " << eb_p->getObserverName() << endl;
    infostream << "The exec block started on " << eb_p->getStartTime().toFITS() << " and ended on " << eb_p->getEndTime().toFITS() << endl;
    if (eb_p->getAborted())
      infostream << "It was aborted." << endl;
    info(infostream.str());

    antennaSummary(eb_p);
    scanSummary(eb_p);
  }
}


void summary(const ASDM& ds, const string& dsPath) {
  infostream.str("");
  infostream << "========================================================================================" << endl;
  infostream << "ASDM dataset :" << dsPath << endl;
  infostream << "========================================================================================" << endl;
  info(infostream.str());

  execBlockSummary(ds);

}

int main (int argC, char* argV[]) {
  string dsName;
  string appName = string(argV[0]);
  ofstream ofs;

  LogSinkInterface& lsif = LogSink::globalSink();
  po::variables_map vm;

  try {
    po::options_description generic("Displays a summary of the content of an ASDM dataset .\n"
				    "Usage : " + appName +" asdm-directory \n\n"
				    "Command parameters: \n"
				    " asdm-directory : the pathname to the ASDM dataset to be reported on. \n\n"
				    "Allowed options:");
    generic.add_options()
      ("logfile,l", po::value<string>(), "specifies the log filename. If the option is not used then the logged informations are written to the standard error stream.")
      ("help", "produces help message.");

    po::options_description hidden("Hidden options");
    hidden.add_options()
      ("asdm-directory", po::value< string >(), "asdm directory")
      ;

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);
    
    po::positional_options_description p;
    p.add("asdm-directory", 1);

    po::store(po::command_line_parser(argC, argV).options(cmdline_options).positional(p).run(), vm);
    po::notify(vm);
  
    if (vm.count("help")) {
      errstream.str("");
      errstream << generic << "\n" ;
      error(errstream.str());
    }

    // Where do the log messages should go ?
    if (vm.count("logfile")) {
      //LogSinkInterface *theSink;
      ofs.open(vm["logfile"].as<string>().c_str(), ios_base::app);
      LogSinkInterface *theSink = new casa::StreamLogSink(&ofs);
      LogSink::globalSink(theSink);
    }

    if (vm.count("asdm-directory")) {
      string dummy = vm["asdm-directory"].as< string >();
      dsName = lrtrim(dummy) ;
      if (boost::algorithm::ends_with(dsName,"/")) dsName.erase(dsName.size()-1);
    }
    else {
      errstream.str("");
      errstream << generic ;
      error(errstream.str());
    }
  }
  catch (std::exception& e) {
    errstream.str("");
    errstream << e.what();
    error(errstream.str());
  }

  try {
    infostream.str("");
    infostream << "Input ASDM dataset : " << dsName << endl;
    info(infostream.str());
    
    ASDM ds ;
    ds.setFromFile(dsName, ASDMParseOptions().loadTablesOnDemand(true).checkRowUniqueness(false));
    summary(ds, dsName);
  }
  catch (ConversionException e) {
    errstream.str("");
    errstream << e.getMessage();
    error(errstream.str());
  }
  catch (std::exception e) {
    errstream.str("");
    errstream << e.what();
    error(errstream.str());
  }
  catch (...) {
    errstream.str("");
    errstream << "Uncaught exception !" << endl;
    error(errstream.str());
  }
  return 0;
}

  
