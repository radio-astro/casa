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

#include <synthesis/ImagerObjects/SynthesisIterBot.h>

#include <synthesisiterbot_cmpt.h>

using namespace std;
using namespace casa;

     
namespace casac {

  synthesisiterbot::synthesisiterbot():
    itsIterBot(NULL)
{
  itsIterBot = new SynthesisIterBot() ;
  ////  itsIterBot->openDBus( );
}

synthesisiterbot::~synthesisiterbot()
{
  if(itsIterBot)
    {
      delete itsIterBot;
    }
}


casac::record* synthesisiterbot::setupiteration(const casac::record& iterpars)
{

  try 
    {
      casa::Record recpars = *toRecord( iterpars );
      itsIterBot->setupIteration( recpars );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return getiterationdetails();
}


casac::record* synthesisiterbot::getiterationdetails()
{
  casac::record* rstat(0);

  try 
    {
      rstat=fromRecord(itsIterBot->getIterationDetails());
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

casac::record* synthesisiterbot::getiterationsummary()
{
  casac::record* rstat(0);

  try 
    {
      rstat=fromRecord(itsIterBot->getIterationSummary());
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}



  int synthesisiterbot::cleanComplete()
{
  Int rstat=0;

  try 
    {
      rstat = itsIterBot->cleanComplete( );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}



bool synthesisiterbot::endmajorcycle()
{
  Bool rstat(False);
  
  try 
    {
      itsIterBot->endMajorCycle();
     } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}



  casac::record* synthesisiterbot::getminorcyclecontrols()
{
  casac::record* rstat(0);
  try {
    rstat=fromRecord(itsIterBot->getSubIterBot());
  } catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}  

bool synthesisiterbot::mergeinitrecord(const casac::record& initrecord)
{
  Bool rstat(False);
  
  try 
    {
      casa::Record recpars = *toRecord( initrecord );
      itsIterBot->startMinorCycle( recpars );
     } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}

bool synthesisiterbot::mergeexecrecord(const casac::record& execrecord)
{
  Bool rstat(False);
  
  try 
    {
      casa::Record recpars = *toRecord( execrecord );
      itsIterBot->endMinorCycle(recpars);
     } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}


bool
synthesisiterbot::done()
{
  Bool rstat(False);

  try 
    {
      if (itsIterBot)
	{
	  delete itsIterBot;
	  itsIterBot=NULL;
	}
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}



} // casac namespace
