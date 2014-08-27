/***
 * Tool bindings of the test single dish module that work on an MS
 *
 * @author kana
 * @version 
 ***/
#include <sdms_cmpt.h>
#include <string>
#include <iostream>

#include <singledish/SingleDish/SingleDishMS.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Exceptions/Error.h>

#include <casa/namespace.h> // using casa namespace

using namespace std;

namespace casac {

sdms::sdms()
{
  itsSd = 0;
  itsLog = new LogIO();
}

sdms::~sdms()
{
  if (itsSd != 0) delete itsSd;
  delete itsLog;
}

bool
sdms::open(const string& ms_name)
{
  Bool rstat(False);
  try {
    // In case already open, close it!
    close();
    // create instanse
    itsSd = new SingleDishMS(ms_name);
    if (itsSd != 0) rstat = True;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
sdms::close()
{
  Bool rstat(False);
  try {
    if(itsSd != 0) delete itsSd;
    itsSd = 0;
    rstat = True;
  } catch  (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() 
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
sdms::done()
{
   return close();
}

string
sdms::name()
{
  if(itsSd != 0) return itsSd->name();
  *itsLog << LogIO::SEVERE << "MS is not yet assigned." << LogIO::POST;
  return "";
}

} // end of casac namespace
