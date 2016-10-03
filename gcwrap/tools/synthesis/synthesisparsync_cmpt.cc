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
using namespace casacore;
using namespace casa;

     
using namespace casacore;
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
  Bool rstat(false);

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
  Bool rstat(false);
  try {
    itsParSync->setImageStore( const_cast<casac::synthesisimstore *>(imstore)->getImStore() );
    rstat=true;
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
  Bool rstat(false);
  try {
    itsParSync->gatherImages( /*dopsf*/true ); //, /*doresidual*/false );
    rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisparsync::gatherresidual()
{
  Bool rstat(false);
  try {
    itsParSync->gatherImages( /*dopsf*/false) ; //, /*doresidual*/true );
    rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisparsync::dividepsfbyweight()
{
  Bool rstat(false);
  try {
	itsParSync->dividePSFByWeight( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisparsync::divideresidualbyweight()
{
  Bool rstat(false);
  try {
	itsParSync->divideResidualByWeight( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisparsync::dividemodelbyweight()
{
  Bool rstat(false);
  try {
	itsParSync->divideModelByWeight( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}
  bool synthesisparsync::multiplymodelbyweight()
{
  Bool rstat(false);
  try {
	itsParSync->multiplyModelByWeight( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisparsync::scattermodel()
{
  Bool rstat(false);
  try {
    itsParSync->scatterModel( );
    rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool
synthesisparsync::done()
{
  Bool rstat(false);

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
