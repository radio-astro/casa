
/***
 * Framework independent implementation file for sdwriter...
 *
 * Implement the sdwriter component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <xmlcasa/singledish/sdwriter_cmpt.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>


using namespace std;
using namespace casa;

namespace casac {

/* Default constructor */
sdwriter::sdwriter()
{
  itsSDWriterWrapper = new asap::SDWriterWrapper();
  itsSDMemTableWrapper = new asap::SDMemTableWrapper();
  itsLog = new LogIO();
}

/* destructor */
sdwriter::~sdwriter()
{
  delete itsSDWriterWrapper;
  delete itsSDMemTableWrapper;
  delete itsLog;
}

int
sdwriter::setformat(const std::string& format)
{

  int rstat = 0;
  try {
    rstat = itsSDWriterWrapper->setFormat(format);
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;

}

int
sdwriter::write(const std::string& intable, const std::string& filename, const bool tostokes)
{

  int rstat = 0;
  try {
    rstat = itsSDWriterWrapper->write(intable, filename, tostokes);
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;


}

} // casac namespace

