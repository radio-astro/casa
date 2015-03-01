/***
 * Framework independent implementation file for linearmosaic...
 *
 *   Implemention of the linearmosaic component
 *    linearmosaic.xml.
 * 
 *
 * @author Kumar Golap
 * @version $Id$
 ***/

#include <iostream>
#include <linearmosaic_cmpt.h>
#include <synthesis/MeasurementEquations/LinearMosaic.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <iostream>

using namespace std;
using namespace casa;

namespace casac {

linearmosaic::linearmosaic() 
 
{
  // Default constructor
  itsLog = new casa::LogIO();
  itsMos = new casa::LinearMosaic();
}

linearmosaic::~linearmosaic()
{
  delete itsMos;
  delete itsLog;
}


bool 
linearmosaic::defineimage(const int nx,
		      const int ny, 
		      const ::casac::variant& cellx,
		      const ::casac::variant& celly,
		      const ::casac::variant& imagecenter,
		      const std::string& outputimage) {

  Bool rstat(False);
  try {
    if(itsMos) delete itsMos;
    rstat = True;
  } catch  (AipsError x) {

    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}
  // ---------------------------------------------------------------------
  bool linearmosaic::makemosaic( const std::vector<std::string>& images,  const std::vector<std::string>& weightimages){

 Bool rstat(False);
  try {
    
    rstat = True;
  } catch  (AipsError x) {

    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;

  }

//

//----------------------------------------------------------------------------

} // casac namespace
