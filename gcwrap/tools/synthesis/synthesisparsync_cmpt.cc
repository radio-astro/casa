/***
 * Framework independent implementation file for imager...
 *
 * Implement the imager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 
 * @author Wes Young
 * @version 
 ***/

#include <iostream>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/Assert.h>
#include <ms/MeasurementSets.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <casa/Logging/LogIO.h>

#include <synthesis/ImagerObjects/SynthesisParSync.h>

#include <synthesisparsync_cmpt.h>

using namespace std;
using namespace casa;

     
namespace casac {

synthesisparsync::synthesisparsync()
{
  itsParSync = new SynthesisParSync() ;
}

synthesisparsync::~synthesisparsync()
{
  done();
}

  bool synthesisparsync::setupparsync(const casac::record& syncpars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( syncpars );
      itsParSync->setupParSync( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

  bool synthesisparsync::setimstore(const casac::synthesisimstore *imstore)
{
  Bool rstat(False);
  try {
    itsParSync->setImageStore( const_cast<casac::synthesisimstore *>(imstore)->getImStore() );
    rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

synthesisimstore* synthesisparsync::getimstore()
{
  synthesisimstore *rstat=NULL;
  try {

    rstat = new synthesisimstore( &*(itsParSync->getImageStore()) );

  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}



bool synthesisparsync::gatherpsfweight()
{
  Bool rstat(False);
  try {
    itsParSync->gatherImages( /*dopsf*/True, /*doresidual*/False );
    rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisparsync::gatherresidual()
{
  Bool rstat(False);
  try {
    itsParSync->gatherImages( /*dopsf*/False, /*doresidual*/True );
    rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisparsync::dividepsfbyweight()
{
  Bool rstat(False);
  try {
	itsParSync->dividePSFByWeight( );
	rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisparsync::divideresidualbyweight()
{
  Bool rstat(False);
  try {
	itsParSync->divideResidualByWeight( );
	rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisparsync::scattermodel()
{
  Bool rstat(False);
  try {
    itsParSync->scatterModel( );
    rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool
synthesisparsync::done()
{
  Bool rstat(False);

  try 
    {
      if (itsParSync)
	{
	  delete itsParSync;
	  itsParSync=NULL;
	}
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}



} // casac namespace
