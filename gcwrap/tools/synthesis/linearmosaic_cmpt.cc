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
#include <images/Images/TempImage.h>
#include <synthesis/MeasurementEquations/LinearMosaic.h>
#include <synthesis/MeasurementEquations/Imager.h>
#include <casa/Logging/LogIO.h>
#include <casa/Utilities/Assert.h>

#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>


using namespace std;
using namespace casacore;
using namespace casa;

using namespace casacore;
namespace casac {

linearmosaic::linearmosaic() 
 
{
  // Default constructor
  itsLog = new casacore::LogIO();
  itsMos = new casa::LinearMosaic();
}

linearmosaic::~linearmosaic()
{
  delete itsMos;
  itsMos=NULL;
  delete itsLog;
  itsLog=NULL;
}


bool 
linearmosaic::defineoutputimage(const int nx,
		      const int ny, 
		      const ::casac::variant& cellx,
		      const ::casac::variant& celly,
		      const ::casac::variant& imagecenter,
		      const std::string& outputimage, const std::string& outputweight) {

  Bool rstat(false);
  try {
    if(itsMos) delete itsMos;
    itsMos=NULL;
    casacore::MDirection imcen;
    if(!casaMDirection(imagecenter, imcen)){
    	    throw(AipsError("Could not interprete phasecenter parameter"));
    }
    Int nX, nY;
    nX=nx;
    nY=ny;
    if(nY < 1)
    nY=nx;
    casacore::Quantity cellX=casaQuantity(cellx);
    casacore::Quantity cellY=casaQuantity(celly);
    itsMos=new LinearMosaic(outputimage, outputweight, imcen, nX, nY,
    			  cellX, cellY);
    rstat = true;
  } catch  (AipsError x) {

    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}
//-------------------------------------------------------------------------
bool linearmosaic::setoutputimage(const  string& outputimage, const  string& outputweight,  int  imageweight, int weighttype){
	 Bool rstat(false);
	  try {
		  	  itsMos->setOutImages(outputimage, outputweight, imageweight, weighttype);

		  	  rstat = true;
		  } catch  (AipsError x) {

		    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		    RETHROW(x);
		  }
		  return rstat;

}
///---------------------------------------------------------------------------
bool linearmosaic::setlinmostype(const  string& linmostype){
	 Bool rstat(false);
	  try {
		  	  Int mostype=2;
		  	  if(casacore::String(linmostype)==casacore::String("pbweight"))
		  		  mostype=1;
		  	  itsMos->setlinmostype(mostype);

		  	  rstat = true;
		  } catch  (AipsError x) {

		    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		    RETHROW(x);
		  }
		  return rstat;

}

  // ---------------------------------------------------------------------
  bool linearmosaic::makemosaic(const casac::variant& inimages,  const casac::variant& inweightimages, const int imagewgttype, const int wgttype){

 Bool rstat(true);
  try {

	  casacore::Vector<String> images, weightimages;
	  images=toVectorString(toVectorString(inimages, "images"));
	  weightimages=toVectorString(toVectorString(inweightimages, "weightimages"));

	  if(images.nelements() != weightimages.nelements())
		  throw(AipsError("number of images and weightimages not equal"));
	  Vector<CountedPtr<ImageInterface<Float> > > im(images.nelements()), wgt(images.nelements());
	  for (uInt k=0; k < images.nelements(); ++k){

          if(wgttype==1){
        	  wgt[k]=new casacore::PagedImage<Float>(weightimages[k]);
          }
          else if(wgttype==2){
        	  PagedImage<Float>tmp(weightimages[k]);
        	  wgt[k]=new casacore::TempImage<Float>(tmp.shape(), tmp.coordinates());
        	  wgt[k]->copyData((LatticeExpr<Float>)(sqrt(abs(tmp))));
		  if(tmp.getDefaultMask() != "")
		    casa::Imager::copyMask(*wgt[k], tmp,  tmp.getDefaultMask());
          }
	  if(imagewgttype==1)
	    im[k]=new casacore::PagedImage<Float>(images[k]);
	  else if(imagewgttype==0){
	    PagedImage<Float>tmp(images[k]);
	    im[k]=new casacore::TempImage<Float>(tmp.shape(), tmp.coordinates());
	    im[k]->copyData((LatticeExpr<Float>)(tmp*(*wgt[k])));
	    if(tmp.getDefaultMask() != "")
	      casa::Imager::copyMask(*im[k], tmp,  tmp.getDefaultMask());
	  }
	  else if(imagewgttype==2){
	    PagedImage<Float>tmp(images[k]);
	    im[k]=new casacore::TempImage<Float>(tmp.shape(), tmp.coordinates());
	    im[k]->copyData((LatticeExpr<Float>)(iif(*(wgt[k]) > (0.0),
						     (tmp/(*wgt[k])), 0)));
	    if(tmp.getDefaultMask() != "")
	      casa::Imager::copyMask(*im[k], tmp,  tmp.getDefaultMask());
	  }
	  else
	    throw(AipsError("image is weighted in an unknown fashion"));
	  //rstat=rstat && itsMos->makeMosaic(im, wgt);
	  }
	  rstat=rstat && itsMos->makeMosaic(im, wgt);
    
  } catch  (AipsError x) {

    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;

  }
  bool linearmosaic::saultweightimage(const  string& outputimage,double fracpeak){
	  Bool rstat(false);
	 	  try {
	 		  	  itsMos->saultWeightImage(outputimage, Float(fracpeak));

	 		  	  rstat = true;
	 		  } catch  (AipsError x) {

	 		    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
	 		    RETHROW(x);
	 		  }
	 		  return rstat;

  }


//

//----------------------------------------------------------------------------

} // casac namespace
