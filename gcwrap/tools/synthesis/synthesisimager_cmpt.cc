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

#include <synthesis/MeasurementEquations/SynthesisImager.h>

#include <synthesisimager_cmpt.h>

using namespace std;
using namespace casa;

     
namespace casac {

synthesisimager::synthesisimager()
{
  itsImager = new SynthesisImager() ;
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

  bool synthesisimager::setupdeconvolution(const casac::record& decpars)
{
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord( decpars );
      itsImager->setupDeconvolution( rec );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

casac::record* synthesisimager::setupiteration(const casac::record& iterpars)
{
  //Bool rstat(False);
  casac::record* rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( iterpars );
      casa::SIIterBot loopcontrols = itsImager->setupIteration( recpars );
      rstat=fromRecord(loopcontrols);
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

bool synthesisimager::initcycles()
{
  Bool rstat(False);

  try 
    {
      itsImager->initCycles( );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

   /*
casac::record* synthesisimager::initloops()
{
  casac::record* rstat(0);

  try 
    {
      Record loopcontrols = itsImager->initLoops();
      rstat=fromRecord(loopcontrols);
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}
   */

  bool synthesisimager::endloops(const casac::record& loopcontrols)
  {
  Bool rstat(False);

  try 
    {
      casa::Record rec = *toRecord(loopcontrols);
      casa::SIIterBot lcrec(rec);
      itsImager->endLoops(lcrec);
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
  }

  casac::record* synthesisimager::runmajorcycle(const ::casac::record& loopcontrols)
{
    casac::record* rstat(0);
    //bool rstat(0);

  try 
    {
      casa::Record inrec = *toRecord(loopcontrols);
      casa::SIIterBot lcrec(inrec);
      itsImager->runMajorCycle(lcrec);
      rstat=fromRecord(lcrec);
      //      loopcontrols = *fromRecord(inrec);

    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

  casac::record* synthesisimager::runminorcycle(const ::casac::record& loopcontrols)
{
   casac::record* rstat(0);
   //bool rstat(0);

  try 
    {
      casa::Record inrec = *toRecord(loopcontrols);
      casa::SIIterBot lcrec(inrec);
      itsImager->runMinorCycle(lcrec);
      rstat=fromRecord(lcrec);
      //      loopcontrols = *fromRecord(inrec);
    } 
  catch  (AipsError x) 
    {
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



} // casac namespace
