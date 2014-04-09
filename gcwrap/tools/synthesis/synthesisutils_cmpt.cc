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

  casac::record* synthesisutils::cubedatapartition(const casac::record& selpars, const int npart, const double fstart, const double fend, const string& frame)
{
  casac::record* rstat(0);

  try 
    {
      casa::Record recpars = *toRecord( selpars );
      casa::MFrequency::Types eltype;
      casa::MFrequency::getType(eltype, frame);
      rstat = fromRecord(  itsUtils->cubeDataPartition( recpars , npart, fstart, fend, eltype ) );
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

/***
 bool 
 synthesisutils::makeimage(const casac::record& impars, const casac::record& selpars, const string& msname)
{
  bool rstat(0);

  try 
    {
      // Construct parameter object, and verify params.
      casa::Record recpars = *toRecord( impars );
      SynthesisParamsImage pars;
      pars.fromRecord( recpars );  // will throw exception if parameters are invalid

      // Construct Coordinate system and make image. 
      MeasurementSet ms;
      //if( msname.length() > 0 && (Directory(msname)).exists() ) { ms = MeasurementSet(msname); }
      ms = MeasurementSet(msname);

      cout << "Making image : " << pars.imageName << " of shape : " << pars.shp() << endl;
      PagedImage<Float> diskimage( pars.shp(), pars.buildCoordinateSystem(ms), pars.imageName );

    } 
  catch  (AipsError x) 
    {
      RETHROW(x);
    }

  return rstat;
}
***/





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
