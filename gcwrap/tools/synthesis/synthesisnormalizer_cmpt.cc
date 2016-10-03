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
using namespace casacore;
using namespace casa;

     
using namespace casacore;
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
  Bool rstat(false);

  try 
    {
      casacore::Record rec = *toRecord( normpars );
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
  Bool rstat(false);
  try {
    ////    itsNormalizer->setImageStore( const_cast<casac::synthesisimstore *>(imstore)->getImStore() );
    rstat=true;
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

bool synthesisnormalizer::gatherweightdensity()
{
  Bool rstat(false);
  try {
    itsNormalizer->gatherImages( /*dopsf*/false, /*doresidual*/false, /*density*/ true );
    rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}
bool synthesisnormalizer::scatterweightdensity()
{
  Bool rstat(false);
  try {
    itsNormalizer->scatterWeightDensity( ); 
    rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}


bool synthesisnormalizer::gatherpsfweight()
{
  Bool rstat(false);
  try {
    itsNormalizer->gatherImages( /*dopsf*/true, /*doresidual*/false, /*density*/ false );
    rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisnormalizer::gatherresidual()
{
  Bool rstat(false);
  try {
    itsNormalizer->gatherImages( /*dopsf*/false, /*doresidual*/true, /*density*/ false );
    rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisnormalizer::dividepsfbyweight()
{
  Bool rstat(false);
  try {
	itsNormalizer->dividePSFByWeight( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisnormalizer::normalizeprimarybeam()
{
  Bool rstat(false);
  try {
	itsNormalizer->normalizePrimaryBeam( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisnormalizer::divideresidualbyweight()
{
  Bool rstat(false);
  try {
	itsNormalizer->divideResidualByWeight( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  bool synthesisnormalizer::dividemodelbyweight()
{
  Bool rstat(false);
  try {
	itsNormalizer->divideModelByWeight( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}
  bool synthesisnormalizer::multiplymodelbyweight()
{
  Bool rstat(false);
  try {
	itsNormalizer->multiplyModelByWeight( );
	rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisnormalizer::scattermodel()
{
  Bool rstat(false);
  try {
    itsNormalizer->scatterModel( );
    rstat=true;
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool
synthesisnormalizer::done()
{
  Bool rstat(false);

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
