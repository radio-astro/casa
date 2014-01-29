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
#include <casa/Logging/LogIO.h>

#include <synthesis/ImagerObjects/SynthesisUtilMethods.h>

#include <synthesisutils_cmpt.h>

using namespace std;
using namespace casa;

     
namespace casac {

synthesisutils::synthesisutils() 
{
  itsUtils = new SynthesisUtilMethods();
}

synthesisutils::~synthesisutils()
{
  done();
}

  casac::record* synthesisutils::contdatapartition(const casac::record& selpars, const int npart)
{
  casac::record* rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( selpars );
      rstat = fromRecord(  itsUtils->continuumDataPartition( recpars , npart ) );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}

  casac::record* synthesisutils::cubedatapartition(const casac::record& selpars, const int npart)
{
  casac::record* rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( selpars );
      rstat = fromRecord(  itsUtils->cubeDataPartition( recpars , npart ) );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}

  casac::record* synthesisutils::cubeimagepartition(const casac::record& selpars, const int npart)
{
  casac::record* rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( selpars );
      rstat = fromRecord(  itsUtils->cubeImagePartition( recpars , npart ) );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}


  casac::record* synthesisutils::checkselectionparams(const casac::record& selpars)
{
  casac::record* rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( selpars );
      SynthesisParamsSelect pars;
      pars.setValues( recpars );
      rstat = fromRecord(  pars.toRecord()  );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}





bool
synthesisutils::done()
{
  Bool rstat(False);

  try 
    {
      if (itsUtils)
	{
	  delete itsUtils;
	  itsUtils=NULL;
	}
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }
  
  return rstat;
}

} // casac namespace
