//# MS2asdm.cc 
//#
//#  ALMA - Atacama Large Millimeter Array
//#  (c) European Southern Observatory, 2002
//#  (c) Associated Universities Inc., 2002
//#  Copyright by ESO (in the framework of the ALMA collaboration),
//#  Copyright by AUI (in the framework of the ALMA collaboration),
//#  All rights reserved.
//#  
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#  
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#  
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <cmath>

#include <boost/algorithm/string.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/algorithm/string.hpp>
using namespace boost;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <alma/ASDM/ASDMAll.h>

#include <alma/MS2ASDM/MS2ASDM.h>

#include <exception>
#include <casa/Logging/StreamLogSink.h>
#include <casa/Logging/LogSink.h>

string appName;
bool verbose = true;
using namespace casa;

void info(const string& message) {  
  if(!verbose){
    return;
  }
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
}

void error(const string& message) {
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::SEVERE));
  exit(1);
}

// A facility to get rid of blanks at start and end of a string.
// 
string lrtrim(std::string& s,const std::string& drop = " ")
{
  std::string r=s.erase(s.find_last_not_of(drop)+1);
  return r.erase(0,r.find_first_not_of(drop));
}


#include <iostream>
#include <sstream>


/**
 * The main function.
 */
int main(int argc, char *argv[]) {

  String asdmfile = "";
  String msfile = "";
  String datacolumn = "DATA";
  String archiveid = "S0";
  String rangeid = "X1"; 
  bool verbose = False;
  bool showversion = False;
  double subscanduration = 24.*3600.; // default is one day
  double schedblockduration = 2700.; // default is 45 minutes
  bool apcorrected = True;

  boost::filesystem::path msPath;

  appName = string(argv[0]);
  ofstream ofs;

  //   Process command line options and parameters.
  po::variables_map vm;
 
  // Declare the supported options.
  
  po::options_description generic("Converts an ASDM dataset into a CASA measurement set.\n"
				  "Usage : " + appName +" [options] ms-directory asdm-directory\n\n"
				  "Allowed options:");
  generic.add_options()
    ("datacolumn,d", po::value<string>(), "specifies the datacolumn.")
    ("archiveid,a", po::value<string>(), "specifies the log filename. If the option is not used then the logged informations are written to the standard error stream.")
    ("rangeid,g", po::value<string>(), "specifies the log filename. If the option is not used then the logged informations are written to the standard error stream.")
    ("subscanduration,s", po::value<double>(), "specifies the maximum duration of a subscan in the output ASDM (seconds). Default: 86400")
    ("schedblockduration,s", po::value<double>(), "specifies the maximum duration of a scheduling block in the output ASDM (seconds). Default: 2700")
    ("logfile,l", po::value<string>(), "specifies the log filename. If the option is not used then the logged informations are written to the standard error stream.")
    ("apuncorrected,u", "the data given by datacolumn should be regarded as not having an atmospheric phase correction. Default: data is AP corrected.")
    ("verbose,v", "logs numerous informations as the filler is working.")
    ("revision,r", "logs information about the revision of this application.");
  
  
  // Hidden options, will be allowed both on command line and
  // in config file, but will not be shown to the user.
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("ms-directory", po::value< string >(), "ms directory")
    ;
  hidden.add_options()
    ("asdm-directory", po::value< string >(), "asdm directory")
    ;
  
  po::options_description cmdline_options;
  cmdline_options.add(generic).add(hidden);
  
  po::positional_options_description p;
  p.add("ms-directory", 1);
  p.add("asdm-directory", 1);
  
  // Parse the command line and retrieve the options and parameters.
  po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
  
  po::notify(vm);
  
  // Where do the log messages go ?
  if (vm.count("logfile")) {
    //LogSinkInterface *theSink;
    ofs.open(vm["logfile"].as<string>().c_str(), ios_base::app);
    LogSinkInterface *theSink = new casa::StreamLogSink(&ofs);
    LogSink::globalSink(theSink);
  }
  
  // AP corrected?
  apcorrected = !(vm.count("apuncorrected") > 0);
  // Verbose or quiet ?
  verbose = vm.count("verbose") > 0;
  // showversion ?
  showversion = vm.count("revision") > 0;

  if (vm.count("datacolumn")) {
    datacolumn = String(vm["datacolumn"].as< string >());
  }

  if (vm.count("archiveid")) {
    archiveid = String(vm["archiveid"].as< string >());
  }

  if (vm.count("rangeid")) {
    msfile = String(vm["rangeid"].as< string >());
  }

  if (vm.count("subscanduration")) {
    subscanduration = vm["subscanduration"].as< double >();
  }

  if (vm.count("schedblockduration")) {
    schedblockduration = vm["schedblockduration"].as< double >();
  }

  if (vm.count("ms-directory")) {
    msfile = String(vm["ms-directory"].as< string >());
  }
  else if(!showversion){
    error("Error: Need to provide name of input Measurement Set."); 
  }

  if (vm.count("asdm-directory")) {
    asdmfile = String(vm["asdm-directory"].as< string >());
  }
  else if(!showversion){
    error("Error: Need to provide name of output ASDM."); 
  }

  MeasurementSet* itsMS=0;
   
  int rstat = 0; // return value 0 means "everything OK"
  MS2ASDM* m2a=0;
  try{
    if(showversion && msfile==""){
      MeasurementSet ms;
      m2a = new MS2ASDM(ms);
      error("Using ASDM version "+ m2a->showversion());
      delete m2a;
    }
    else{
      itsMS = new MeasurementSet(msfile);
      m2a = new MS2ASDM(*itsMS);
      info("Using ASDM version " + m2a->showversion());
      if (!m2a->writeASDM(asdmfile, datacolumn, archiveid, rangeid, verbose,
			  subscanduration, schedblockduration, apcorrected)) {
	delete m2a;
	delete itsMS;
	error("Conversion to ASDM failed.");
      }
      delete m2a;
      delete itsMS;
    }
  } catch (AipsError x) {
    if(m2a){
      delete m2a;
    }
    if(itsMS){
      delete itsMS;
    }
    Table::relinquishAutoLocks();
    error("Exception Reported: " + x.getMesg());
  }
  Table::relinquishAutoLocks();
  exit(rstat); // note: "return rstat" does not give the correct return value when used inside Python os.system()
  
}


