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

#include <synthesis/ImagerObjects/SynthesisDeconvolver.h>

#include <synthesisdeconvolver_cmpt.h>

using namespace std;
using namespace casa;

     
namespace casac {

synthesisdeconvolver::synthesisdeconvolver()
{
  itsDeconvolver = new SynthesisDeconvolver() ;
}

synthesisdeconvolver::~synthesisdeconvolver()
{
  done();
}

  bool synthesisdeconvolver::setupdeconvolution(const casac::record& decpars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( decpars );

      SynthesisParamsDeconv decpars;
      decpars.fromRecord( rec );
      itsDeconvolver->setupDeconvolution( decpars );
      //      itsDeconvolver->setupDeconvolution( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

casac::record* synthesisdeconvolver::initminorcycle()
{
  casac::record* rstat(0);
  try {
    rstat = fromRecord(itsDeconvolver->initMinorCycle( ));
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

casac::record* synthesisdeconvolver::interactivegui(const casac::record& iterbot)
{
  casac::record* rstat(0);
  try {
    casa::Record recpars = *toRecord( iterbot );
    rstat = fromRecord(itsDeconvolver->interactiveGUI( recpars ));
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

casac::record* synthesisdeconvolver::executeminorcycle(const casac::record& iterbot)
{
  casac::record* rstat(0);
  try {
    casa::Record recpars = *toRecord( iterbot );
    rstat = fromRecord(itsDeconvolver->executeMinorCycle( recpars ));
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool synthesisdeconvolver::restore()
{
  casac::record* rstat(0);
  try {
    itsDeconvolver->restore();
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

  /*
  bool synthesisdeconvolver::testsummary(const casac::image *imt)
{
  casac::record* rstat(False);
  try {

    const_cast<casac::image *>(imt)->_image->summary();

  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}
  */


bool
synthesisdeconvolver::done()
{
  Bool rstat(False);

  try 
    {
      if (itsDeconvolver)
	{
	  delete itsDeconvolver;
	  itsDeconvolver=NULL;
	}
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}



} // casac namespace
