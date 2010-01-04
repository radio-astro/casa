
/***
 * Framework independent implementation file for sdreader...
 *
 * Implement the sdreader component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <xmlcasa/singledish/sdreader_cmpt.h>
#include <casa/aips.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

using namespace std;
using namespace casa;

namespace casac {

/* Default constructor */
sdreader::sdreader()
{
  itsSDReaderWrapper = new asap::SDReaderWrapper();
  itsSDMemTableWrapper = new asap::SDMemTableWrapper();
  itsLog = new LogIO();
  fopenstat = False;
  rdatastat = False;
}

sdreader::sdreader(const std::string& filename, const int theif, const int thebeam)
{
  itsSDReaderWrapper = new asap::SDReaderWrapper(filename, theif, thebeam);
  itsSDMemTableWrapper = new asap::SDMemTableWrapper();
  itsLog = new LogIO();
  fopenstat = False;
  rdatastat = False;
}

/* destructor */
sdreader::~sdreader()
{
  delete itsSDReaderWrapper;
  delete itsSDMemTableWrapper;
  delete itsLog;
}

bool
sdreader::open(const std::string& filename, const int theif, const int thebeam)
{

    // TODO : IMPLEMENT ME HERE !
  Bool rstat(False);
  try {
     if(fopenstat) {
       cerr<<"close first"<<endl;
       delete itsSDReaderWrapper;
       itsSDReaderWrapper = new asap::SDReaderWrapper();
       fopenstat = False;
     }
     itsSDReaderWrapper->open(filename, theif, thebeam);
     rstat = True;
     fopenstat = True;
  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST; 
     fopenstat = False;
  } 
  return rstat;
}

int
sdreader::read(const std::vector<int>& integrations)
{
  int rstat = 0;
    // TODO : IMPLEMENT ME HERE !
  try {
    if(fopenstat) {
      rstat = itsSDReaderWrapper->read(integrations);      
      rdatastat = True;
    } else {
      *itsLog << LogIO::WARN << "No file specified, please open first" << LogIO::POST;
    }

  } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdreader::close()
{
  Bool rstat(False);
    // TODO : IMPLEMENT ME HERE !
  try {
     delete itsSDReaderWrapper;
     itsSDReaderWrapper = new asap::SDReaderWrapper();
     fopenstat = False;
     rstat = True;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

bool
sdreader::reset()
{
  Bool rstat(False);
    // TODO : IMPLEMENT ME HERE !
  try {
    if(fopenstat) {
     itsSDReaderWrapper->reset();
     rstat = True;
    } else {
      *itsLog << LogIO::WARN << "No file specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::
POST;
  }
  return rstat;
}

casac::sdtable*
sdreader::getdata()
{
    // TODO : IMPLEMENT ME HERE !
  casac::sdtable *rstat(0); 
  try {
    if(fopenstat) {
      if(rdatastat) {
        cerr<<"call getSDMemTable() method"<<endl;
        rstat = new casac::sdtable(itsSDReaderWrapper->getSDMemTable());
      } else {
        *itsLog <<LogIO::WARN << "No data read, please read first" << LogIO::POST;
      }
       
    } else {
      *itsLog << LogIO::WARN << "No file specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "<<x.getMesg() << LogIO::POST;
  }
  return rstat;
}

std::vector<int>
sdreader::header()
{
    // TODO : IMPLEMENT ME HERE !
  std::vector<int> rstat;
  try {
    if(fopenstat) {
      rstat = itsSDReaderWrapper->pseudoHeader();
    } else {
      *itsLog << LogIO::WARN << "No file specified, please open first" << LogIO::POST;
    }
  } catch (AipsError x) {
   *itsLog << LogIO::SEVERE << "Exception Reported: "<< x.getMesg() << LogIO::POST;
  }
  return rstat;
}

} // casac namespace
