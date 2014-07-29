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

#include <synthesis/ImagerObjects/SynthesisNormalizer.h>

#include <synthesisnormalizer_cmpt.h>

using namespace std;
using namespace casa;

     
namespace casac {

synthesisnormalizer::synthesisnormalizer()
{
  itsNormalizer = new SynthesisNormalizer() ;
}

synthesisnormalizer::~synthesisnormalizer()
{
  done();
}

  bool synthesisnormalizer::setupnormalizer(const casac::record& normpars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( normpars );
      itsNormalizer->setupNormalizer( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

bool synthesisnormalizer::setimstore(const casac::synthesisimstore */*imstore*/)
{
  Bool rstat(False);
  try {
    ////    itsNormalizer->setImageStore( const_cast<casac::synthesisimstore *>(imstore)->getImStore() );
    rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

synthesisimstore* synthesisnormalizer::getimstore()
{
  synthesisimstore *rstat=NULL;
  try {

    /////    rstat = new synthesisimstore( &*(itsNormalizer->getImageStore()) );

  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}



bool synthesisnormalizer::gatherpsfweight()
{
  Bool rstat(False);
  try {
    itsNormalizer->gatherImages( /*dopsf*/True ); //, /*doresidual*/False );
    rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisnormalizer::gatherresidual()
{
  Bool rstat(False);
  try {
    itsNormalizer->gatherImages( /*dopsf*/False) ; //, /*doresidual*/True );
    rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisnormalizer::dividepsfbyweight()
{
  Bool rstat(False);
  try {
	itsNormalizer->dividePSFByWeight( );
	rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisnormalizer::divideresidualbyweight()
{
  Bool rstat(False);
  try {
	itsNormalizer->divideResidualByWeight( );
	rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisnormalizer::dividemodelbyweight()
{
  Bool rstat(False);
  try {
	itsNormalizer->divideModelByWeight( );
	rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}
  bool synthesisnormalizer::multiplymodelbyweight()
{
  Bool rstat(False);
  try {
	itsNormalizer->multiplyModelByWeight( );
	rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisnormalizer::scattermodel()
{
  Bool rstat(False);
  try {
    itsNormalizer->scatterModel( );
    rstat=True;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool
synthesisnormalizer::done()
{
  Bool rstat(False);

  try 
    {
      if (itsNormalizer)
	{
	  delete itsNormalizer;
	  itsNormalizer=NULL;
	}
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}



} // casac namespace
