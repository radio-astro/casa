/***
 * Framework independent implementation file for imager...
 *
 * Implement the imager component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 
 ***/

#include <iostream>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/Assert.h>
#include <ms/MeasurementSets.h>
#include <ms/MeasurementSets/MSHistoryHandler.h>
#include <casa/Logging/LogIO.h>

#include <synthesis/MeasurementEquations/SynthesisImager.h>

#include <synthesisimager_cmpt.h>

using namespace std;
using namespace casa;

     
namespace casac {

synthesisimager::synthesisimager()
{
  itsImager = new SynthesisImager();
}

synthesisimager::~synthesisimager()
{
  if(itsImager)
    {
      delete itsImager;
    }
}


  // This function should read in a variant, and convert it to a list of MSs and selpars for itsImager
bool
synthesisimager::selectdata(const casac::record& selpars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( selpars );
      itsImager->selectData( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

bool
synthesisimager::defineimage(const casac::record& impars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( impars );
      itsImager->defineImage( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

  bool synthesisimager::setupimaging(const casac::record& gridpars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( gridpars );
      itsImager->setupImaging( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}


bool synthesisimager::initmapper()
{
  Bool rstat(False);

  try 
    {
      itsImager->initMapper( );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

bool synthesisimager::executemajorcycle(const casac::record& controls)
{
  Bool rstat(False);
  try {
    casa::Record recpars = *toRecord( controls );
    itsImager->executeMajorCycle( recpars );
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}


bool
synthesisimager::done()
{
  Bool rstat(False);

  try 
    {
      if (itsImager)
	{
	  delete itsImager;
	}
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}



/*
  bool synthesisimager::setupdeconvolution(const casac::record& decpars)
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
*/
 /*
casac::record* synthesisimager::getiterationdetails()
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
 */

  /*
casac::record* synthesisimager::getiterationsummary()
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
  */

   /*
casac::record* synthesisimager::setupiteration(const casac::record& iterpars)
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
   */

/*
bool synthesisimager::cleanComplete()
{
  Bool rstat(False);

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
*/

  /*
   casac::record* synthesisimager::endloops()
  {
  try 
    {
      itsImager->endLoops();
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return getiterationdetails();
  }
  */

 /*
bool synthesisimager::runmajorcycle()
{
  Bool rstat(False);

  try 
    {
      // This is a convenience function for tool-level usage, for the non-parallel case.
      // Duplicates the code from getmajorcyclecontrols(), executemajorcycle(), endmajorcycle().
      casa::Record recpars; // = itsImager->getMajorCycleControls();
      itsImager->executeMajorCycle( recpars );
      itsIterBot->endMajorCycle();
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}
 */
  /*
casac::record* synthesisimager::getmajorcyclecontrols()
{
  casac::record* rstat(0);
  try {
    rstat=fromRecord(itsImager->getMajorCycleControls());
  } catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}  
  */

/*
bool synthesisimager::endmajorcycle()
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
*/

 /*
bool synthesisimager::runminorcycle()
{
  Bool rstat(False);
  
  try 
    {
      // This is a convenience function for tool-level usage, for the non-parallel case.
      // Duplicates the code from getsubiterbot(), executeminorcycle(), endminorcycle().
      casa::Record iterbotrec = itsIterBot->getSubIterBot();
      iterbotrec = itsDeconvolver->executeMinorCycle(iterbotrec);
      itsImager->endMinorCycle(iterbotrec);
     } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}
 */

  /*
casac::record* synthesisimager::getsubiterbot()
{
  casac::record* rstat(0);
  try {
    rstat=fromRecord(itsImager->getSubIterBot());
  } catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}  
  */

   /*
casac::record* synthesisimager::executeminorcycle(const casac::record& iterbot)
{
  casac::record* rstat(False);
  try {
    casa::Record recpars = *toRecord( iterbot );
    rstat = fromRecord(itsImager->executeMinorCycle( recpars ));
  } catch  (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}
   */
    /*
bool synthesisimager::endminorcycle(const casac::record& iterbot)
{
  Bool rstat(False);
  
  try 
    {
      casa::Record recpars = *toRecord( iterbot );
      itsImager->endMinorCycle(recpars);
     } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}
    */


} // casac namespace
