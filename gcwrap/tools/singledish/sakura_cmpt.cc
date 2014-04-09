/***
 * Tool bindings of sakura module
 *
 * @author kana
 * @version 
 ***/
#include <string>
#include <iostream>

#include <casa/Logging/LogIO.h>
#include <casa/Exceptions/Error.h>

#include <sakura_cmpt.h>
#include <stdcasa/StdCasa/CasacSupport.h>

using namespace std;
using namespace casa;

namespace casac {

sakura::sakura()
{
  try {
    itsLog = new LogIO(LogOrigin("sakura",""));
    // do initialization here
    *itsLog << "Constructor of sakura module is called" << LogIO::POST;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
}

sakura::~sakura()
{
  try {
    *itsLog << LogOrigin("sakura", "");
    // finalizations
    *itsLog << "Destructor of sakura module is called" << LogIO::POST;
    if (itsLog) {
      delete itsLog;
      itsLog = NULL;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
}

void
sakura::welcome(int number) {
  *itsLog << LogOrigin("sakura", "welcome");
  try {
    if (number < 0)
      throw(AipsError("The input number should be >= 0"));
    // do something here
    *itsLog << "sakura::welcome is called with the input value, "
	    << number << ". This function just returns the input value."
	    << LogIO::POST;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
}

}  // casac namespace
