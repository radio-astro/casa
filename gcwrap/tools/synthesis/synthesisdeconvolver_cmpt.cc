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

#include <synthesis/MeasurementEquations/SynthesisDeconvolver.h>

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
  if(itsDeconvolver)
    {
      delete itsDeconvolver;
    }
}

  bool synthesisdeconvolver::setupdeconvolution(const casac::record& decpars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( decpars );
      itsDeconvolver->setupDeconvolution( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

casac::record* synthesisdeconvolver::initminorcycle(const casac::record& iterbot)
{
  casac::record* rstat(False);
  try {
    casa::Record recpars = *toRecord( iterbot );
    rstat = fromRecord(itsDeconvolver->initMinorCycle( recpars ));
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}


casac::record* synthesisdeconvolver::executeminorcycle(const casac::record& iterbot)
{
  casac::record* rstat(False);
  try {
    casa::Record recpars = *toRecord( iterbot );
    rstat = fromRecord(itsDeconvolver->executeMinorCycle( recpars ));
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}

bool
synthesisdeconvolver::done()
{
  Bool rstat(False);

  try 
    {
      if (itsDeconvolver)
	{
	  delete itsDeconvolver;
	}
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}



} // casac namespace
