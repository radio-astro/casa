#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
#include <assert.h>
#include <cmath>
#include <complex>
#include <string>

#include <boost/algorithm/string.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/algorithm/string.hpp>
using namespace boost;

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
using namespace boost::filesystem;

#include <boost/regex.hpp> 

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/casts.hpp>
using namespace boost::lambda;

#include <boost/foreach.hpp>

#include <ASDMAll.h>

#include "SDMBinData.h"
using namespace sdmbin;

#include <exception>
using namespace asdm;
#include "IllegalAccessException.h"

#include "SDMDataObjectReader.h"
#include "SDMDataObject.h"

#include "asdmstman/AsdmStMan.h"


void scanBDF(const string & pathToBDF) {
  SDMDataObjectStreamReader sdosr;
  
  sdosr.open(pathToBDF);
  ProcessorType processorType = sdosr.processorType();
  if (processorType == RADIOMETER) {
    const SDMDataSubset& sdmDataSubset = sdosr.getSubset();
  }
  else if (processorType == CORRELATOR) {
    while (sdosr.hasSubset()) {
      const SDMDataSubset& sdmDataSubset = sdosr.getSubset();
    }
  }
  else 
    cout << "Processor not supported in lazy mode." << endl;
  
  sdosr.close(); 
}


int main ( int argc, char * argv[] ) {
  string dsName;
  string appName = string(argv[0]);

  po::variables_map vm;

  try {
    // Declare the supported options.

    po::options_description generic("Read sequentially all the BDFs of  ASDM dataset without any processing. It's just an application to measure the time reading the BDFs by using the class SDMDataObjectStreamReader. \n"
				    "Usage : " + appName +" asdm-directory \n\n"
				    "Command parameters: \n"
				    " asdm-directory : the pathname to the ASDM dataset containing the BFDs to be read. \n"
				    
				    ".\n\n"
				    "Allowed options:");
    generic.add_options()
      ("help", "produces help message.");
    
    // Hidden options, will be allowed both on command line and
    // in config file, but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()
      ("asdm-directory", po::value< string >(), "asdm directory")
      ;
    
    po::options_description cmdline_options;
    cmdline_options.add(generic).add(hidden);
    
    po::positional_options_description p;
    p.add("asdm-directory", 1);
    
    // Parse the command line and retrieve the options and parameters.
    po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
    
    po::notify(vm);

    if (vm.count("asdm-directory")) {
      dsName = vm["asdm-directory"].as< string >();
      cout << "dsName = " << dsName << endl;
      boost::algorithm::trim(dsName);
      if (boost::algorithm::ends_with(dsName,"/")) dsName.erase(dsName.size()-1);
    }
    else {
      cout << generic << endl;
      exit (1);
    }
  }
  catch (std::exception& e) {
    cout << e.what() << endl;
  }

  ASDM* ds_p = new ASDM();
  try {
    cout << "Input ASDM dataset : " << dsName << endl;
    
    ds_p->setFromFile(dsName, ASDMParseOptions().loadTablesOnDemand(true));
  }
  catch (ConversionException e) {
    cout << e.getMessage() << endl;
  }
  catch (std::exception e) {
    cout << e.what() << endl;
  }
  catch (...) {
    cout << "Uncaught exception !" << endl;
  } 

  const MainTable& mainT = ds_p->getMain();
  const vector<MainRow *>& v = mainT.get();

  vector<string> bdfNames;
  for ( vector<MainRow *>::const_iterator iter_v = v.begin(); iter_v != v.end(); iter_v++) {
    string abspath = complete(path(dsName)).string() + "/ASDMBinary/" + replace_all_copy(replace_all_copy((*iter_v)->getDataUID().getEntityId().toString(), ":", "_"), "/", "_");
    cout << abspath << endl;
    bdfNames.push_back(abspath);
  }

  for (unsigned int iBDF = 0; iBDF < bdfNames.size(); iBDF++) {
    scanBDF(bdfNames[iBDF]);
    cout << iBDF+1 << "/" << bdfNames.size() << endl;
  }
  exit (0);
}
