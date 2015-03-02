/***
 * Framework independent implementation file for linearmosaic...
 *
 *   Implemention of the linearmosaic component
 *    linearmosaic.xml.
 * 
 *
 * @author Kumar Golap
 * @version $Id$
 ***/

#include <iostream>
#include <linearmosaic_cmpt.h>
#include <images/Images/PagedImage.h>
#include <synthesis/MeasurementEquations/LinearMosaic.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>


using namespace std;
using namespace casa;

namespace casac {

linearmosaic::linearmosaic() 
 
{
  // Default constructor
  itsLog = new casa::LogIO();
  itsMos = new casa::LinearMosaic();
}

linearmosaic::~linearmosaic()
{
  delete itsMos;
  delete itsLog;
}


bool 
linearmosaic::defineimage(const int nx,
		      const int ny, 
		      const ::casac::variant& cellx,
		      const ::casac::variant& celly,
		      const ::casac::variant& imagecenter,
		      const std::string& outputimage, const std::string& outputweight) {

  Bool rstat(False);
  try {
    if(itsMos) delete itsMos;
    casa::MDirection imcen;
    if(!casaMDirection(imagecenter, imcen)){
    	    throw(AipsError("Could not interprete phasecenter parameter"));
    }
    Int nX, nY;
    nX=nx;
    nY=ny;
    if(nY < 1)
    nY=nx;
    casa::Quantity cellX=casaQuantity(cellx);
    casa::Quantity cellY=casaQuantity(celly);
    itsMos=new LinearMosaic(outputimage, outputweight, imcen, nX, nY,
    			  cellX, cellY);
    rstat = True;
  } catch  (AipsError x) {

    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}
  // ---------------------------------------------------------------------
  bool linearmosaic::makemosaic( const std::vector<std::string>& images,  const std::vector<std::string>& weightimages){

 Bool rstat(True);
  try {

	  Vector<CountedPtr<ImageInterface<Float> > > im(1), wgt(1);
	  for (int k=0; images.size(); ++k){

		  im[0]=new casa::PagedImage<Float>(images[k]);
		  wgt[0]=new casa::PagedImage<Float>(weightimages[k]);
		  rstat=rstat && itsMos->makeMosaic(im, wgt);
	  }
    
  } catch  (AipsError x) {

    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;

  }

//

//----------------------------------------------------------------------------

} // casac namespace
