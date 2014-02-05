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

#include <casa/OS/Directory.h>
#include <images/Images/PagedImage.h>

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
      pars.fromRecord( recpars );
      rstat = fromRecord(  pars.toRecord()  );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}

  casac::record* synthesisutils::checkimageparams(const casac::record& impars)
{
  casac::record* rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( impars );
      SynthesisParamsImage pars;
      pars.fromRecord( recpars );
      rstat = fromRecord(  pars.toRecord()  );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}


  casac::record* synthesisutils::checkgridparams(const casac::record& gridpars)
{
  casac::record* rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( gridpars );
      SynthesisParamsGrid pars;
      pars.fromRecord( recpars );
      rstat = fromRecord(  pars.toRecord()  );
    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}


 bool 
 synthesisutils::makeimage(const casac::record& impars, const string& msname)
{
  bool rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( impars );
      SynthesisParamsImage pars;
      pars.fromRecord( recpars );

      SynthesisUtilMethods su;
      MeasurementSet ms;
      if( msname.length() > 0 && (Directory(msname)).exists() ){ ms = MeasurementSet(msname); }
      CoordinateSystem imcsys = su.buildCoordinateSystem( pars, ms );

      IPosition imshape(4, pars.imsize[0], pars.imsize[1], 
			((imcsys.stokesCoordinate()).stokes()).nelements(), 
			pars.nchan);

      cout << "Making image : " << pars.imageName << " of shape : " << imshape << endl;
      PagedImage<Float> diskimage( imshape, imcsys, pars.imageName );

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
