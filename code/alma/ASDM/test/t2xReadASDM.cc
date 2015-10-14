#include <iostream>
#include <sstream>
#include <vector>
#include <exception>

#include "ASDMAll.h"

#include <casa/Logging/StreamLogSink.h>
#include <casa/Logging/LogSink.h>

using namespace std;
using namespace asdm;
using namespace casa;

ostringstream errstream;
ostringstream infostream;

string appName;
bool verbose = true;
bool checkRowUniqueness = true;

void info (const string& message) {
  
  if (!verbose){
    return;
  }

  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
}

void warning (const string& message) {
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
}

void error(const string& message, int status=1) {
  LogSink::postGlobally(LogMessage(message, LogOrigin(appName,WHERE), LogMessage::NORMAL));
  //os << LogIO::POST;
  exit(status);
}

ASDM* readASDM(string asdmPath) {
  ASDM* ds_p;
  ds_p = new ASDM();

  try {
    ASDMParseOptions parse = ASDMParseOptions().loadTablesOnDemand(true).checkRowUniqueness(checkRowUniqueness);
    ds_p->setFromFile(asdmPath, parse);
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

  cout << asdmPath << " successfully loaded with load tables on demand == true" << endl;

  return ds_p;
}


int main(int argC, char *argV[]) {
  if (argC < 2) {
    cout << "Usage : TestTableStreamReader <asdm-directory>" << endl;
    exit (1);
  }

  ASDM* ds_p = readASDM(argV[1]);
  if (ds_p->getAntenna().size() > 0) {
    const vector<AntennaRow *>&	aR_v = ds_p->getAntenna().get();
    vector<PointingRow *>* pR_v	     = ds_p->getPointing().getByContext(aR_v[0]->getAntennaId());
    cout << "getByContext returned " << pR_v->size() << " rows." << endl;
  }
  delete ds_p; ds_p = NULL;

  ds_p = readASDM(argV[1]);
  if (ds_p->getAntenna().size() > 0) {
    const vector<AntennaRow *>&	aR_v = ds_p->getAntenna().get();
    vector<PointingRow *>* pR_v	     = ds_p->getPointing().getByContext(aR_v[0]->getAntennaId());
    cout << "getByContext returned " << pR_v->size() << " rows." << endl;
  }
  delete ds_p; ds_p = NULL;
}
