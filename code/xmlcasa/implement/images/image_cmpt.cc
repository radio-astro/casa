/***
 * Framework independent implementation file for image...
 *
 * Implement the image component here.
 * 
 * // image_cmpt.cc: defines image class which implements functionality
 * // for the image component
 *
 * @author
 * @version 
 ***/

#include <iostream>
#include <sys/wait.h>
#include <xmlcasa/images/image_cmpt.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/fstream.h>
#include <casa/Logging/LogFilter.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/SymLink.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Utilities/Assert.h>
#include <components/ComponentModels/ComponentList.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/GaussianShape.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/SkyCompRep.h>
#include <components/ComponentModels/TwoSidedShape.h>
#include <components/SpectralComponents/SpectralElement.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/GaussianConvert.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <images/Images/ComponentImager.h>
#include <images/Images/Image2DConvolver.h>
#include <images/Images/ImageConcat.h>
#include <images/Images/ImageConvolver.h>
#include <images/Images/ImageDecomposer.h>
#include <images/Images/ImageExpr.h>
#include <images/Images/ImageExprParse.h>
#include <images/Images/ImageFFT.h>
#include <images/Images/ImageFITSConverter.h>
#include <images/Images/ImageFitter.h>
#include <images/Images/ImageHistograms.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageMoments.h>
#include <images/Images/ImageRegrid.h>
#include <images/Images/ImageSourceFinder.h>
#include <images/Images/ImageStatistics.h>
#include <images/Images/ImageSummary.h>
#include <images/Images/ImageTwoPtCorr.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/LELImageCoord.h>
#include <images/Images/PagedImage.h>
#include <images/Images/RebinImage.h>
#include <images/Images/SepImageConvolver.h>
#include <images/Images/SubImage.h>
#include <images/Images/TempImage.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/WCLELMask.h>
#include <lattices/LatticeMath/Fit2D.h>
#include <lattices/LatticeMath/LatticeFit.h>
#include <lattices/Lattices/LatticeAddNoise.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <lattices/Lattices/LatticeExprNode.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeRegion.h>
#include <lattices/Lattices/LatticeSlice1D.h>
#include <lattices/Lattices/LatticeUtilities.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LCSlicer.h>
#include <lattices/Lattices/MaskedLatticeIterator.h>
#include <lattices/Lattices/PixelCurve1D.h>
#include <lattices/Lattices/RegionType.h>
#include <lattices/Lattices/TiledLineStepper.h>
#include <measures/Measures/Stokes.h>
#include <scimath/Fitting/LinearFitSVD.h>
#include <scimath/Functionals/Polynomial.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <tables/LogTables/NewFile.h>

#include <casa/namespace.h>

using namespace std;

namespace casac {

image::image()
{
  try {
    itsLog = new LogIO();
    *itsLog << LogOrigin("image", "image");
    itsImage= new ImageAnalysis();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
}

// private ImageInterface constructor for on the fly components
image::image(const casa::ImageInterface<casa::Float>* inImage)
{
  try {
    itsLog = new LogIO();
    *itsLog << LogOrigin("image", "image");
    itsImage= new ImageAnalysis(inImage);
   
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
}


image::~image()
{
  try {
    if (itsImage != 0) {
      delete itsImage;
      itsImage = 0;
    }
    delete itsLog;
    itsLog=0;
  } catch (AipsError x) {
    if(itsLog){
    *itsLog << LogOrigin("image", "~image");
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    } else {
	    cerr << "Exception Reported: " << x.getMesg() << endl;
    }
    // RETHROW(x);
  }
}

/*
Bool unset(const ::std::vector<bool> &par) {
  if (par.size() == 1 && par[0] == false) {
    return true;
  } else {
    return false;
  }
}

Bool unset(const ::std::vector<int> &par) {
  if (par.size() == 1 && par[0]==-1) {
    return true;
  } else {
    return false;
  }
}

Bool unset(const ::casac::record &theRec) {
  Bool rstat(True);
  for(::casac::rec_map::const_iterator rec_it = theRec.begin();
      rec_it != theRec.end(); rec_it++){
    rstat = False;
    break;
  }
  return rstat;
}

Bool unset(const ::casac::variant &theVar) {
  Bool rstat(False);
  if ( (theVar.type() == ::casac::variant::BOOLVEC)
       && (theVar.size() == 0) ) rstat = True;
  return rstat;
}
*/

Bool isunset(const ::casac::variant &theVar) {
  Bool rstat(False);
  if ( (theVar.type() == ::casac::variant::BOOLVEC)
       && (theVar.size() == 0) ) rstat = True;
  return rstat;
}

::casac::record*
image::torecord(){
  ::casac::record *rstat = new ::casac::record();
  *itsLog << LogOrigin("image", "torecord");
  if (detached()) return rstat;
  try {
    Record rec;
    if (!itsImage->toRecord(rec)) {
      *itsLog << "Could not convert to record "
	      << LogIO::EXCEPTION;
    }


    // Put it in a ::casac::record
    delete rstat;
    rstat=0;
    rstat = fromRecord(rec);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
    	    << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;  

}
bool
image::fromrecord(const ::casac::record& imrecord, 
		  const std::string& outfile){
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "fromrecord");
    Record *tmpRecord = toRecord(imrecord);
    if(itsImage == 0)
      itsImage= new ImageAnalysis();
    rstat=itsImage->fromRecord(*tmpRecord, casa::String(outfile));
    delete tmpRecord;
    if (!rstat) {
      *itsLog << "Failed to create a valid image from this record"
	      << LogIO::EXCEPTION;
    }
  } catch (AipsError x) {
    RETHROW(x);
  }
  return rstat;
}


bool
image::addnoise(const std::string& type, const std::vector<double>& pars,
		const ::casac::record& region, const bool zeroIt)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "addnoise");
    if (detached()) return rstat;

    Record *pRegion = toRecord(region);

    itsImage->addnoise(type, pars, *pRegion, zeroIt);
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::imagecalc(const std::string& outfile, const std::string& pixels,
		 const bool overwrite)
{
  ::casac::image *rstat = 0;
  try {
    if (itsLog==0)
      itsLog = new LogIO();
    *itsLog << LogOrigin("image", "imagecalc");
    if (itsImage) delete itsImage;
    itsImage = new ImageAnalysis();
    rstat = new ::casac::image(itsImage->imagecalc(outfile, pixels, overwrite));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::imageconcat(const std::string& outfile, const ::casac::variant& infiles,
		   const int axis, const bool relax, const bool tempclose,
		   const bool overwrite)
{
  ::casac::image *rstat = 0;
  try {
    if (itsLog==0)
      itsLog=new LogIO();
    *itsLog << LogOrigin("image", "imageconcat");

    if (itsImage) delete itsImage;
    itsImage = new ImageAnalysis();

    Vector<String> inFiles;
    if (infiles.type() == ::casac::variant::BOOLVEC) {
      inFiles.resize(0);      // unset
    } else if (infiles.type() == ::casac::variant::STRING) {
      sepCommaEmptyToVectorStrings(inFiles, infiles.toString());
    } else if (infiles.type() == ::casac::variant::STRINGVEC) {
      inFiles = toVectorString(infiles.toStringVec());
    } else {
      *itsLog << LogIO::WARN << "Unrecognized infiles datatype"
	      << LogIO::POST;
    }

    rstat = new ::casac::image(itsImage->imageconcat(outfile, inFiles, 
						     axis, relax, tempclose, 
						     overwrite));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
        RETHROW(x);
  }
  return rstat;
}

bool
image::fromarray(const std::string& outfile, const ::casac::variant& pixels, const ::casac::record& csys, const bool linear, const bool overwrite, const bool log)
{
  bool rstat = false;
  try {
    if(itsLog==0)
      itsLog=new LogIO();
    if(itsImage) delete itsImage;
    itsImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "fromarray");

    // Some protection.  Note that a Glish array, [], will
    // propagate through to here to have ndim=1 and shape=0
    Vector<Int> shape = pixels.arrayshape();
    uInt ndim = shape.size();
    if (ndim == 0) {
      *itsLog << "The pixels array is empty" << LogIO::EXCEPTION;
    }
    for (uInt i=0; i<ndim; i++) {
      if (shape(i) <=0) {
	*itsLog << "The shape of the pixels array is invalid" << LogIO::EXCEPTION;
      }
    }

    Array<Float> pixelsArray;
    if(pixels.type() == ::casac::variant::DOUBLEVEC ){
      std::vector<double> pixelVector = pixels.getDoubleVec();
      Vector<Int> shape = pixels.arrayshape();
      pixelsArray.resize(IPosition(shape));
      Vector<Double> localpix(pixelVector);
      casa::convertArray(pixelsArray,localpix.reform(IPosition(shape)));
    }
    else if(pixels.type()== ::casac::variant::INTVEC){
      std::vector<int> pixelVector = pixels.getIntVec();
      Vector<Int> shape = pixels.arrayshape();
      pixelsArray.resize(IPosition(shape));
      Vector<Int> localpix(pixelVector);
      casa::convertArray(pixelsArray,localpix.reform(IPosition(shape)));
    }  
    else{
      *itsLog << LogIO::SEVERE 
	      << "pixels is not understood, try using an array " 
	      << LogIO::POST;
      return rstat;
    }

    Record *coordinates = toRecord(csys);
    rstat=itsImage->imagefromarray(outfile, pixelsArray, *coordinates, linear, 
			     overwrite, log);
    delete coordinates;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::fromascii(const std::string& outfile, const std::string& infile,
		      const std::vector<int>& shape, const std::string& sep,
		      const ::casac::record& csys, const bool linear,
		      const bool overwrite)
{

  // The glish code ignored sep (assumed to be ' ') so will we to
  bool rstat=false;

  try {
    if(itsLog==0)
      itsLog=new LogIO();
    *itsLog << LogOrigin("image", "fromascii");

    if (infile=="") {
      *itsLog << LogIO::SEVERE << "infile must be specified" << LogIO::POST;
      return false;
    }
    if (shape.size()==1 && shape[0]==-1) {
      *itsLog << LogIO::SEVERE << "Image shape must be specified"
	      << LogIO::POST;
      return false;
    }

    if(itsImage==0)
      itsImage=new ImageAnalysis();
    Record *coordsys=toRecord(csys);
    rstat = itsImage->imagefromascii(outfile, infile , Vector<Int>(shape), sep,
				     *coordsys, linear, overwrite);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::fromfits(const std::string& outfile, const std::string& fitsfile,
		     const int whichrep, const int whichhdu,
		     const bool zeroBlanks, const bool overwrite)
{
  bool rstat = false;
  try {
    if(itsLog==0) itsLog=new LogIO();
    if(itsImage) delete itsImage;
    itsImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "fromfits");
       rstat= 
         itsImage->imagefromfits(outfile, fitsfile, whichrep, whichhdu, 
	         		      zeroBlanks, overwrite);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::fromimage(const std::string& outfile, const std::string& infile, const ::casac::record& region, const ::casac::variant& mask, const bool dropdeg, const bool overwrite)
{
  bool rstat = false;
  try {
    if(itsLog==0)
      itsLog=new LogIO();
    if(itsImage) delete itsImage;
    itsImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "fromimage");
    String theMask;
    Record *theMaskRegion;
    if(mask.type() == ::casac::variant::BOOLVEC){
      theMask="";
    }
    else if(mask.type() == ::casac::variant::STRING){
      theMask=mask.toString();
    }
    else if(mask.type() == ::casac::variant::RECORD){
      theMaskRegion=toRecord(mask.getRecord());
      *itsLog << LogIO::SEVERE 
	      << "Don't support region masking yet, only valid LEL " 
	      << LogIO::POST;
      return False;
    } 
    else{
      *itsLog << LogIO::SEVERE 
	      << "Mask is not understood, try a valid LEL string " 
	      << LogIO::POST;
      return False;
    }

    Record *Region = toRecord(region);
    rstat = itsImage->imagefromimage(outfile, infile, *Region,
				     theMask, dropdeg ,overwrite);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool 
image::fromshape(const std::string& outfile, const std::vector<int>& shape, const ::casac::record& csys, const bool linear, const bool overwrite, const bool log)
{
  bool rstat = false;
  try {
    if(itsLog==0)
      itsLog=new LogIO();
    if(itsImage) delete itsImage;
    itsImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "fromshape");
    Record *coordinates = toRecord(csys);
    rstat=itsImage->imagefromshape(outfile, Vector<Int>(shape), *coordinates, 
				   linear, overwrite, log);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::adddegaxes(const std::string& outfile, const bool direction, const bool spectral, const std::string& stokes, const bool linear, const bool tabular, const bool overwrite)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "adddegaxes");
    if (detached()) return rstat;


    PtrHolder<ImageInterface<Float> > outimage;
    itsImage->adddegaxes(outfile, outimage, direction, 
			 spectral, stokes, linear, 
			 tabular, overwrite);
    rstat = new ::casac::image(outimage.ptr());
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::convolve(const std::string& outFile, const ::casac::variant& kernel,
		const double in_scale, const ::casac::record& region,
		const ::casac::variant& vmask, const bool overwrite,
		const bool async)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "convolve");
    if (detached()) return rstat;


    Array<Float> kernelArray;
    String kernelFileName="";
    if(kernel.type() == ::casac::variant::DOUBLEVEC ){
      std::vector<double> kernelVector = kernel.toDoubleVec();
      Vector<Int> shape = kernel.arrayshape();
      kernelArray.resize(IPosition(shape));
      Vector<Double> localkern(kernelVector);
      casa::convertArray(kernelArray,localkern.reform(IPosition(shape)));
    }
    else if(kernel.type()== ::casac::variant::INTVEC){
      std::vector<int> kernelVector = kernel.toIntVec();
      Vector<Int> shape = kernel.arrayshape();
      kernelArray.resize(IPosition(shape));
      Vector<Int> localkern(kernelVector);
      casa::convertArray(kernelArray,localkern.reform(IPosition(shape)));
    } 
    else if(kernel.type()== ::casac::variant::STRING || kernel.type()== ::casac::variant::STRINGVEC){
      
      kernelFileName = kernel.toString();
    }
    else{
      *itsLog << LogIO::SEVERE 
	      << "kernel is not understood, try using an array or an image " 
	      << LogIO::POST;
      return rstat;
    }
 
    String theMask;
    Record *theMaskRegion;
    if(vmask.type() == ::casac::variant::BOOLVEC) {
      theMask="";
    }
    else if(vmask.type() == ::casac::variant::STRING || 
       vmask.type() == ::casac::variant::STRINGVEC){
      theMask=vmask.toString();
    }
    else if(vmask.type()== ::casac::variant::RECORD){
      ::casac::variant localvar(vmask);
      theMaskRegion=toRecord(localvar.asRecord());
      *itsLog << LogIO::SEVERE 
	      << "Don't support region masking yet, only valid LEL " 
	      << LogIO::POST;
      return rstat;
    } 
    else{
      *itsLog << LogIO::SEVERE 
	      << "Mask is not understood, try a valid LEL string " 
	      << LogIO::POST;
      return rstat;
    }

    Record *Region= toRecord(region);

    ImageInterface<Float> * tmpIm=
      itsImage->convolve(outFile, kernelArray, kernelFileName, in_scale, 
			 *Region, theMask, overwrite, async);
    rstat = new ::casac::image(tmpIm);
    delete tmpIm;
		       
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::boundingbox(const ::casac::record& region)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "boundingbox");
    if (detached()) return rstat;
    Record *Region = toRecord(region);
    rstat = fromRecord(*itsImage->boundingbox(*Region));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

std::string
image::brightnessunit()
{
  std::string rstat;
  try {
    *itsLog << LogOrigin("image", "brightnessunit");
    if (! detached()) {
      rstat = itsImage->brightnessunit();
    } else {
      rstat = "";
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::calc(const std::string& expr)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "calc");
    if (detached()) return rstat;

 
    rstat = itsImage->calc(expr);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::calcmask(const std::string& mask, const std::string& maskName,
		const bool makeDefault)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "calcmask");
    if (detached()) return rstat;

    //For now not passing the Regions record...when this is done then 
    //replace this temporary Record  here... it should be a Record of Records..
    //each element a given region
    Record regions;

   rstat = itsImage->calcmask(mask, regions, maskName, makeDefault);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::close()
{
   bool rstat(false);
   try {
     *itsLog << LogOrigin("image", "close");
     if (itsImage != 0 ) {
       *itsLog << LogIO::NORMAL3 << "Detaching from image" <<  LogIO::POST;
       delete itsImage;
     } else {
       *itsLog << LogIO::WARN << "Image is already closed" << LogIO::POST;
     }
     itsImage = 0;
     //                                      
     rstat = true;
   } catch (AipsError x) {
     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
     RETHROW(x);
   }
   return rstat;
}

::casac::image *
image::continuumsub(const std::string& outline, const std::string& outcont,
		    const ::casac::record& region,
		    const std::vector<int>& channels, const std::string& pol,
		    const int in_fitorder, const bool overwrite)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "continuumsub");
    if (detached()) return rstat;

    Record * leRegion=toRecord(region);
    Vector<Int> theChannels(channels);
    if (theChannels.size()==1 && theChannels[0]==-1) {
      theChannels.resize(0);
    }
    ImageInterface<Float> * theResid=
      itsImage->continuumsub(outline, outcont, *leRegion, 
			     theChannels, pol, in_fitorder, 
			     overwrite);
    if(!theResid) {
      *itsLog << LogIO::SEVERE << "continuumsub failed "
	      << LogIO::POST;
    } else {
      rstat = new ::casac::image(theResid);
      delete theResid;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
	    << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record *
image::convertflux(const ::casac::variant& qvalue, const ::casac::variant& major, const ::casac::variant& minor, const std::string& type, const bool toPeak)
{
  ::casac::record *rstat = 0;

  try {
    *itsLog << LogOrigin("image", "convertflux");
    if (detached()) return rstat;

    Quantum<Double> value = casaQuantity(qvalue);
    Quantum<Double> majorAxis = casaQuantity(major);
    Quantum<Double> minorAxis = casaQuantity(minor);

    //
    
    casa::Quantity rtn =
      itsImage->convertflux(value, majorAxis,
			    minorAxis, type, toPeak);
    rstat = recordFromQuantity(rtn);
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
	    << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::convolve2d(const std::string& outFile, const std::vector<int>& axes,
		  const std::string& type, const ::casac::variant& major,
		  const ::casac::variant& minor, const ::casac::variant& pa,
		  const double in_scale, const ::casac::record& region,
		  const ::casac::variant& vmask, const bool overwrite,
		  const bool async)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "convolve2d");
    if (detached()) return rstat;

    UnitMap::putUser("pix",UnitVal(1.0), "pixel units");
    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";
    String kernel(type);
    Quantum<Double> majorKernel = casaQuantityFromVar(major);
    Quantum<Double> minorKernel = casaQuantityFromVar(minor);
    Quantum<Double> paKernel = casaQuantityFromVar(pa);

    Vector<Int> Axes(axes);
    if (Axes.size() == 0) {
      Axes.resize(2);
      Axes[0]=0;
      Axes[1]=1;
    }

    // Return image
    ImageInterface<Float> * tmpIm= 
      itsImage->convolve2d(outFile, Axes, type, majorKernel, 
			   minorKernel, paKernel, in_scale, *Region, mask,
			   overwrite, async);
    rstat = new ::casac::image(tmpIm);
    delete tmpIm;
   
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::coordsys *
image::coordsys(const std::vector<int>& pixelAxes)
{
  ::casac::coordsys *rstat = 0;

  try {
    *itsLog << LogOrigin("image", "coordsys");
    if (detached()) return rstat;

    
    // Return coordsys object
    rstat = new ::casac::coordsys();
    CoordinateSystem csys=itsImage->coordsys(Vector<Int>(pixelAxes));
    rstat->setcoordsys(csys);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

/*
::casac::coordsys *
image::csys(const std::vector<int>& axes)
{

    // See image::coordsys for actual implementation.
    // This is Wes' method for heurestics group.
   ::casac::coordsys *rstat = 0;
   try {
   *itsLog << LogOrigin("image", "csys");
   if(!detached()){
      rstat = new ::casac::coordsys();
      CoordinateSystem csys=itsImage->coordsys(Vector<Int>(axes));
      rstat->setcoordsys(csys);
      //::casac::record *csysRec2 = fromRecord(csysRec.asRecord(RecordFieldId("CoordinateSystem")));
      //rstat->fromrecord(*csysRec2);
      //delete csysRec2;
   }
  } catch (AipsError x) {
        *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
   return rstat;
}
*/

::casac::record*
image::coordmeasures(const std::vector<double>&pixel)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "coordmeasures");
    if (detached()) return rstat;

    casa::Record theDir;
    casa::Record theFreq;
    casa::Record theVel;
    casa::Record* retval;
    casa::Quantity theInt;
    Vector<Double> vpixel;
    if ( !(pixel.size()==1 && pixel[0] == -1) ) {
      vpixel = pixel;
    }
    retval=itsImage->coordmeasures(theInt, theDir, theFreq, theVel, vpixel);

    String error;
    Record R;
    if (QuantumHolder(theInt).toRecord(error, R)) {
      retval->defineRecord(RecordFieldId("intensity"), R);
    } else {
      *itsLog << LogIO::SEVERE << "Could not convert intensity to record. "
	      << error << LogIO::POST;
    }
    rstat = fromRecord(*retval);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::decompose(const ::casac::record& region, const ::casac::variant& vmask,
		 const bool simple, const double Threshold, const int nContour,
		 const int minRange, const int nAxis, const bool fit,
		 const double maxrms, const int maxRetry, const int maxIter,
		 const double convCriteria)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "decompose");
    if (detached()) return rstat;
    
    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";

    Matrix<Int> blcs;
    Matrix<Int> trcs;

    Matrix<Float> cl= itsImage->decompose(blcs, trcs, *Region, mask, simple, Threshold, 
					  nContour, minRange, nAxis, fit,
					  maxrms, maxRetry, maxIter, 
					  convCriteria);

    /*std::vector<float> cl_v;
    cl.tovector(cl_v);
    int nelem = cl_v.size();
    std::vector<double> cl_dv(nelem);
    for (int n=0; n < nelem; n++) {
      cl_dv[n] = cl_v[n];
    }
    std::vector<int> cl_shape;
    cl.shape().asVector().tovector(cl_shape);
    rstat = new ::casac::variant(cl_dv, cl_shape);
    */
    casa::Record outrec1;
    outrec1.define("components", cl);
    outrec1.define("blc", blcs);
    outrec1.define("trc", trcs);
    rstat = fromRecord(outrec1);


  } catch (AipsError x) {
        *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
   return rstat;
}

::casac::record *
image::deconvolvecomponentlist(const ::casac::record& complist)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "decovolvecomponentlist");
    if (detached()) return rstat;

    String error;
    Record *compList = toRecord(complist);
    Record outRec=itsImage->deconvolvecomponentlist(*compList);
    rstat = fromRecord(outRec);
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record *
image::deconvolvefrombeam(const ::casac::variant& source, 
			  const ::casac::variant& beam){

::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "decovolvefrombeam");
    casa::Vector<casa::Quantity> sourceParam;
    casa::Vector<casa::Quantity> beamParam;
    if(!toCasaVectorQuantity(source, sourceParam) || 
       (sourceParam.nelements()==0)){
      throw(AipsError("Cannot understand source values"));
    }
    else{
      if(sourceParam.nelements()==1){
	sourceParam.resize(3, True);
	sourceParam[1]=sourceParam[0];
	sourceParam[2]=casa::Quantity(0.0, "deg");
      }
      if(sourceParam.nelements()==2){
	sourceParam.resize(3, True);
	sourceParam[2]=casa::Quantity(0.0, "deg");
      }

    }
    if(!toCasaVectorQuantity(beam, beamParam) || 
       (beamParam.nelements()==0)){
      throw(AipsError("Cannot understand beam values"));
    }
    else{
      if(beamParam.nelements()==1){
	beamParam.resize(3, True);
	beamParam[1]=beamParam[0];
	beamParam[2]=casa::Quantity(0.0, "deg");
      }
      if(beamParam.nelements()==2){
	beamParam.resize(3, True);
	beamParam[2]=casa::Quantity(0.0, "deg");
      }

    }

    Bool retval=itsImage->deconvolveFromBeam(sourceParam[0], sourceParam[1], 
					     sourceParam[2], beamParam);
    

    casa::Record outrec1;
    casa::Record deconval;
    casa::Record tmprec;
    casa::String error;
    QuantumHolder(sourceParam[0]).toRecord(error, tmprec);
    deconval.defineRecord("major", tmprec);
    QuantumHolder(sourceParam[1]).toRecord(error, tmprec);
    deconval.defineRecord("minor", tmprec);
    QuantumHolder(sourceParam[2]).toRecord(error, tmprec);
    deconval.defineRecord("pa", tmprec);
    outrec1.define("return", retval);
    outrec1.defineRecord("fit", deconval);
    rstat = fromRecord(outrec1);


  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;


}
bool
image::remove(const bool finished, const bool verbose)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "remove");

    if (detached()) return rstat;

    if(itsImage->remove(verbose)){
      // Now done the image tool if desired.
      if (finished) done();
      rstat = true;
    } else {
      rstat=false;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::removefile(const std::string& filename)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "removefile");

    String fileName(filename);
    if (fileName.empty()) {
      *itsLog << LogIO::WARN << "Empty filename" << LogIO::POST;
      return rstat;
    }
    File f(fileName);
    if (! f.exists()) {
      *itsLog << LogIO::WARN << fileName << " does not exist." << LogIO::POST;
      return rstat;
    }

    // Now try and blow it away.  If it's open, tabledelete won't delete it.
    String message;
    if (Table::canDeleteTable(message, fileName, True)) {
      Table::deleteTable(fileName, True);
      rstat = true;
    } else {
      *itsLog << LogIO::WARN << "Cannot delete file " << fileName
              << " because " << message << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}


bool
image::done(const bool remove, const bool verbose)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "done");

    if (remove && !detached()) {
      if(!itsImage->remove(verbose)){
	*itsLog << LogIO::WARN << "Failed to remove image file" << LogIO::POST;
      }
    }

    if (itsImage != 0) {
      delete itsImage;
      itsImage = 0;
    }

    /*
       Don't delete this! bad things happen if you do, because the component is
       not released from memory...i.e done == close

    if (itsLog != 0) {
      delete itsLog;
      itsLog=0;
    }
    */

    rstat = true;

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::fft(const std::string& realOut, const std::string& imagOut,
	   const std::string& ampOut, const std::string& phaseOut,
	   const std::vector<int>& axes, const ::casac::record& region,
	   const ::casac::variant& vmask)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "fft");
    if (detached()) return rstat;

    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";

    // if default value change it to empty vector
    Vector<Int> leAxes(axes);
    if(leAxes.size()==1){
      if(leAxes[0]==-1)
	leAxes.resize();
    }

    rstat= itsImage->fft(realOut, imagOut, ampOut, phaseOut, leAxes,
			 *Region, mask);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::findsources(const int nMax, const double cutoff,
		   const ::casac::record& region,
		   const ::casac::variant& vmask,
		   const bool point, const int width, const bool absFind)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "findsources");
    if (detached()) return rstat;

    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";

    Record listOut= itsImage->findsources(nMax, cutoff, *Region, mask, point, 
					  width, absFind);
    rstat = fromRecord(listOut);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::fitallprofiles(const ::casac::record& region, const int axis,
		      const ::casac::variant& vmask, const int nGauss,
		      const int poly, const std::string& sigmaFileName,
		      const std::string& fitFileName,
		      const std::string& residFileName)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "fitallprofiles");
    if (detached()) return rstat;

    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
      mask = "";
    rstat=itsImage->fitallprofiles(*Region, axis, mask, nGauss, poly, 
				  sigmaFileName, fitFileName, residFileName);
    

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::fitprofile(std::vector<double>& d_values, std::vector<double>& d_resid,
		  const ::casac::record& region, int axis,
		  const ::casac::variant& vmask,
		  const ::casac::record& estimate, const int ngauss,
		  const int poly, const bool fitIt,
		  const std::string& sigmaFileName)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "fitprofile");
    if (detached()) return rstat;
    
    Vector<Float> values;
    Vector<Float> residual;
    Record *Region = toRecord(region);
     Record *Estimate = toRecord(estimate);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";

    Record recOut2=itsImage->fitprofile(values, residual, *Region, axis, mask,
					 *Estimate, ngauss, poly, fitIt, 
					 sigmaFileName);
    // Generate output parameters d_values and d_resid
    int v_elem = values.nelements();
    d_values.resize(v_elem);
    for (int i = 0; i < v_elem; i++) d_values[i]=values[i];
    int r_elem = residual.nelements();
    d_resid.resize(r_elem);
    for (int i = 0; i < r_elem; i++) d_resid[i]=residual[i];
   

    rstat = fromRecord(recOut2);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::fitpolynomial(const std::string& residFile, const std::string& fitFile,
		     const std::string& sigmaFile, const int axis,
		     const int order, const ::casac::record& region,
		     const ::casac::variant& vmask, const bool overwrite)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "fitpolynomial");
    if (detached()) return rstat;

    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";

    ImageInterface<Float>* tmpIm=
      itsImage->fitpolynomial(residFile, fitFile,
			      sigmaFile, axis, order, 
			      *Region, mask, 
			      overwrite);
    rstat = new ::casac::image(tmpIm);
    delete tmpIm;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}


::casac::record* image::fitcomponents(
	const std::string& box, const ::casac::variant& region, const int chan,
	const std::string& stokes, const ::casac::variant& vmask,
	const std::vector<double>& in_includepix,
	const std::vector<double>& in_excludepix,
	const std::string& residual, const std::string& model,
	const std::string& estimates, const std::string& logfile,
	const bool append, const std::string& newestimates
) {
    if (detached()) {
    	return 0;
    }
    int num = in_includepix.size();
    Vector<Float> includepix(num);
    num = in_excludepix.size();
    Vector<Float> excludepix(num);
    convertArray(includepix, Vector<Double>(in_includepix));
    convertArray(excludepix, Vector<Double>(in_excludepix));
    if (includepix.size() == 1 && includepix[0] == -1) {
        includepix.resize();
    }
    if (excludepix.size() == 1 && excludepix[0] == -1) {
        excludepix.resize();
    }
    ::casac::record *rstat = 0;
	*itsLog << LogOrigin("image", "fitcomponent");
    String mask = vmask.toString();
    if(mask == "[]") {
	    mask = "";
    }
	try {
		ImageFitter *fitter = 0;
		String imName = itsImage->name();
		if (region.type() == ::casac::variant::STRING || region.size() == 0) {
			String regionString = (region.size() == 0) ? "" : region.toString();
			fitter = new ImageFitter(
				imName, regionString, box, chan, stokes, mask, includepix, excludepix,
				residual, model, estimates, logfile, append, newestimates
			);
		}
		else if (region.type() == ::casac::variant::RECORD) {
			::casac::variant regionCopy = region;
			Record *regionRecord = toRecord(regionCopy.asRecord());
			fitter = new ImageFitter(
				imName, regionRecord, box, chan, stokes, mask, includepix, excludepix,
				residual, model, estimates, logfile, append, newestimates
			);
		}
		else {
			*itsLog << "Unsupported type for region " << region.type() << LogIO::EXCEPTION;
		}
		ComponentList compList = fitter->fit();
		bool converged = fitter->converged();
		delete fitter;
		Record returnRecord, compListRecord;
		String error;
	    if (! compList.toRecord(error, compListRecord)) {
	        *itsLog << "Failed to generate output record from result. " << error
	                << LogIO::POST;
	    }
		returnRecord.defineRecord("results", compListRecord);
		returnRecord.define("converged", converged);
	    rstat = fromRecord(returnRecord);

	}catch (AipsError x) {
		*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
		RETHROW(x);
	}

	return rstat;
}

::casac::record*
image::fitsky(const ::casac::record& region,
          const int chan, const std::string& stokes, 
	      const ::casac::variant& vmask,
	      const std::vector<std::string>& in_models,
	      const ::casac::record& in_estimate,
	      const std::vector<std::string>& fixedparams,
	      const std::vector<double>& in_includepix,
	      const std::vector<double>& in_excludepix, const bool fitIt,
	      const bool deconvolveIt, const bool list)
{
  // !!! NOTE THAT CASAPY IS NOT RETURNING VARIANT CORRECTLY.  ALWAYS !!!
  // !!! RETURNS BOOLEAN TRUE RATHER THAN SPECIFIED VALUE.            !!!
  ::casac::record * rstat = 0;
  try {
    *itsLog << LogOrigin("image", "fitsky");
    if (detached()) return rstat;

    Array<Float> residPixels;  // output in out_pixels
    Array<Bool> residMask;     // output in out_pixelmask

    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";
    Vector<String> models = toVectorString(in_models);
    Record *Estimate = toRecord(in_estimate);
    Vector<String> fixed = toVectorString(fixedparams);
    //
    int num = in_includepix.size();
    Vector<Float> includepix(num);
    convertArray(includepix, Vector<Double>(in_includepix));
    //
    num = in_excludepix.size();
    Vector<Float> excludepix(num);
    convertArray(excludepix, Vector<Double>(in_excludepix));
    // Deal with those pesky -1 default
    // if default value change it to empty vector
    if(includepix.size()==1){
      if(includepix[0]<0)
	includepix.resize();
    }
    if(excludepix.size()==1){
      if(excludepix[0]<0)
	excludepix.resize();
    }

    //call the fitsky
    Bool converged;
    ComponentList compList = itsImage->fitsky(
        residPixels, residMask, converged, *Region,
        chan, String(stokes), mask, models, *Estimate, fixed, 
		includepix, excludepix, fitIt, deconvolveIt, list
    );


    /*
    // Marshal residMask into output parameter out_pixelmask
    std::vector<bool> v_residMask;
    std::vector<int> v_shape;
    residMask.tovector(v_residMask);
    residMask.shape().asVector().tovector(v_shape);
    out_pixelmask = new ::casac::variant(v_residMask, v_shape);

    // Marshal residPixels into output parameter out_pixels
    std::vector<float> vf_residPixels;
    residPixels.tovector(vf_residPixels);
    int nelem = vf_residPixels.size();
    std::vector<double> vd_residPixels(nelem);
    for (int i=0; i < nelem; i++) vd_residPixels[i] = vf_residPixels[i];
    std::vector<int> v_pshape;
    residPixels.shape().asVector().tovector(v_pshape);
    out_pixels = new ::casac::variant(vd_residPixels, v_pshape);
    */

    Record outrec;
    String error;
    if (! compList.toRecord(error, outrec)) {
        *itsLog << "Failed to generate output record from result. " << error
                << LogIO::POST;
    }    
 
    casa::Record outrec1;
    outrec1.define("converged", converged);
    outrec1.define("pixelmask", residMask);
    outrec1.defineRecord("return", outrec);
    outrec1.define("pixels", residPixels);
    rstat = fromRecord(outrec1);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::variant*
image::getchunk(const std::vector<int>& blc, const std::vector<int>& trc, const std::vector<int>& inc, const std::vector<int>& axes, const bool list, const bool dropdeg, const bool getmask)
{

  // Recover some pixels from the image from a simple strided box
  ::casac::variant *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "getchunk");
    if (detached()) return rstat;

    
    //
    Array<Float> pixels;
    Array<Bool> pixelMask;
    Vector<Int> iaxes(axes);
     // if default value change it to empty vector
    if(iaxes.size()==1){
      if(iaxes[0]<0)
	iaxes.resize();
    }
    itsImage->getchunk(pixels, pixelMask, Vector<Int>(blc), Vector<Int>(trc),
		       Vector<Int>(inc), iaxes, list, dropdeg, getmask);

    if (getmask) {
      std::vector<bool> s_pixelmask;
      std::vector<int> s_shape;
      pixelMask.tovector(s_pixelmask);
      pixels.shape().asVector().tovector(s_shape);
      rstat = new ::casac::variant(s_pixelmask, s_shape);
    } else {
      std::vector<int> s_shape;
      pixels.shape().asVector().tovector(s_shape);
      std::vector<double> d_pixels(pixels.nelements());
      int i(0);
      for(Array<Float>::iterator iter = pixels.begin();
	  iter != pixels.end(); iter++)
	d_pixels[i++]=*iter;
      rstat = new ::casac::variant(d_pixels, s_shape);
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::variant*
image::getregion(const ::casac::record& region, const std::vector<int>& axes, const ::casac::variant& mask, const bool list, const bool dropdeg, const bool getmask)
{
  // Recover some pixels and their mask from a region in the image
  ::casac::variant *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "getregion");
    if (detached()) return false;

    Array<Float> pixels;
    Array<Bool> pixelmask;

    Record *Region = toRecord(region);
    String Mask;
    if(mask.type()== ::casac::variant::BOOLVEC) {
      Mask="";
    }
    else if(mask.type()== ::casac::variant::STRING 
       || mask.type()== ::casac::variant::STRINGVEC  ){
      Mask = mask.toString();
    }
    else{
      *itsLog << LogIO::WARN 
	      << "Only LEL string handled for mask...region is yet to come" 
	      << LogIO::POST;
      Mask="";
    }
    Vector<Int> iaxes(axes);
     // if default value change it to empty vector
    if(iaxes.size()==1){
      if(iaxes[0]<0)
	iaxes.resize();
    }
    itsImage->getregion(pixels, pixelmask, *Region, 
			iaxes, Mask, list, dropdeg,getmask); 
    if (getmask) {
      std::vector<bool> s_pixelmask;
      std::vector<int> s_shape;
      pixelmask.tovector(s_pixelmask);
      pixels.shape().asVector().tovector(s_shape);
      rstat = new ::casac::variant(s_pixelmask, s_shape);
    } else {
      std::vector<int> s_shape;
      pixels.shape().asVector().tovector(s_shape);
      std::vector<double> d_pixels(pixels.nelements());
      int i(0);
      for(Array<Float>::iterator iter = pixels.begin();
	  iter != pixels.end(); iter++)
	d_pixels[i++]=*iter;
      rstat = new ::casac::variant(d_pixels, s_shape);
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::getslice(const std::vector<double>& x, const std::vector<double>& y, const std::vector<int>& axes, const std::vector<int>& coord, const int npts, const std::string& method)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "getslice");
    if (detached()) return rstat;

    // handle default coord
    std::vector<int> ncoord(coord);
    if (ncoord.size() == 1 && ncoord[0] == -1) {
      int n = (itsImage->shape()).size();
      ncoord.resize(n);
      for (int i=0; i < n; i++) {
	//ncoord[i]=i;
	ncoord[i]=0;
      }
    }

    Record *outRec=itsImage->getslice(Vector<Double>(x), Vector<Double>(y), 
				       Vector<Int>(axes), Vector<Int>(ncoord),
				       npts, method);
    rstat =fromRecord(*outRec);
    delete outRec;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::hanning(const std::string& outFile, const ::casac::record& region,
	       const ::casac::variant& vmask, const int axis, const bool drop,
	       const bool overwrite, const bool async)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "hanning");
    if (detached()) return rstat;

    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";
    
    ImageInterface<Float> * pImOut=
      itsImage->hanning(outFile, *Region, mask, axis, drop, overwrite);
    // Return handle to new file
    rstat = new ::casac::image(pImOut);
    delete pImOut;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

std::vector<bool>
image::haslock()
{
  std::vector<bool> rstat;
  try {
    *itsLog << LogOrigin("image", "haslock");
    if (detached()) return rstat;

    itsImage->haslock().tovector(rstat);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::histograms(::casac::record& histout, const std::vector<int>& axes, const ::casac::record& region, const ::casac::variant& mask, const int nbins, const std::vector<double>& includepix, const bool gauss, const bool cumu, const bool log, const bool list, const std::string& plotter, const int nx, const int ny, const std::vector<int>& size, const bool force, const bool disk, const bool async)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "histograms");
    if (detached()) return rstat;

    Record *regionRec = toRecord(region);
    String Mask;
    if(mask.type() == ::casac::variant::BOOLVEC) {
      Mask = "";
    }
    else if(mask.type() == ::casac::variant::STRING ||
       mask.type() == ::casac::variant::STRINGVEC ){
      Mask = mask.toString();
    }
    else{
      *itsLog << LogIO::WARN 
	      << "Only LEL string handled for mask...region is yet to come" 
	      << LogIO::POST;
      Mask="";
    }
    Record retval;
    Vector<Int> naxes;
    if ( !(axes.size() == 1 && axes[0] == -1) ) {
      naxes.resize(axes.size());
      naxes=Vector<Int>(axes);
    }
    Vector<Double> includePix;
    if ( !(includepix.size() == 1 && includepix[0] == -1) ) {
      includePix.resize(includepix.size());
      includePix=Vector<Double>(includepix);
    }
    itsImage->histograms(retval, naxes, *regionRec, Mask, nbins, 
			 includePix, gauss, cumu, log, list,
			 plotter, nx, ny, Vector<Int>(size), force, disk);
     
    delete regionRec;
    casac::record *tmp = fromRecord(retval); // memory leak???
    histout = *tmp;
    //  Cleanup
    delete tmp;
    rstat = true;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

std::vector<std::string>
image::history(const bool list, const bool browse)
{
  std::vector<string> rstat;
  try {
    *itsLog << LogOrigin("image", "history");
    if (detached()) return rstat;
   
    rstat = fromVectorString(itsImage->history(list, browse));
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::insert(const std::string& infile, const ::casac::record& region, const std::vector<double>& locate)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "insert");
    if (detached()) return rstat;

    Vector<Double> locatePixel(locate);
    if(locatePixel.size()==1){
      if(locatePixel[0]<0)
	locatePixel.resize(0);
    }
    Record *Region=toRecord(region);
    rstat = new ::casac::image(itsImage->insert(infile, *Region, locatePixel));
    delete Region;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::isopen()
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "isopen");

    if (itsImage != 0 && !itsImage->detached()) rstat = true;

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::ispersistent()
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "ispersistent");
    if (detached()) return rstat;
    
    rstat = itsImage->ispersistent();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::lock(const bool writelock, const int nattempts)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "lock");
    if (detached()) return rstat;
    rstat = itsImage->lock(writelock, nattempts);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::makecomplex(const std::string& outFile, const std::string& imagFile, const ::casac::record& region, const bool overwrite)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "makecomplex");
    if (detached()) return rstat;

    Record *Region = toRecord(region);

    rstat=itsImage->makecomplex(outFile, imagFile, *Region, overwrite);
    delete Region;

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

std::vector<std::string>
image::maskhandler(const std::string& op, const std::vector<std::string>& name)
{
  std::vector<string> rstat;
  try {
    *itsLog << LogOrigin("image", "maskhandler");
    if (detached()) return rstat;

    Vector<String> namesOut;
    Vector<String> namesIn = toVectorString(name);
    namesOut=itsImage->maskhandler(op, namesIn);
    if (namesOut.size() == 0) {
      namesOut.resize(1);
      namesOut[0]="T";
    }
    rstat = fromVectorString(namesOut);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::miscinfo()
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "miscinfo");
    if (detached()) return rstat;

    rstat = fromRecord(itsImage->miscinfo());
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::modify(const ::casac::record& model, const ::casac::record& region,
	      const ::casac::variant& vmask, const bool subtract,
	      const bool list, const bool async)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "modify");
    if (detached()) return rstat;

    //
    String error;
    Record *Model = toRecord(model);
    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";

    rstat=itsImage->modify(*Model, *Region, mask, subtract,list);
    delete Region;
    delete Model;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::maxfit(const ::casac::record& region, const bool doPoint,
	      const int width, const bool absFind, const bool list)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "maxfit");
    if (detached()) return rstat;

    Record *Region = toRecord(region);
    rstat = fromRecord(itsImage->maxfit(*Region, doPoint,width,absFind,list));
   
    delete Region;
   
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::moments(const std::vector<int>& moments, const int axis,
	       const ::casac::record& region, const ::casac::variant& vmask,
	       const std::vector<std::string>& in_method,
	       const std::vector<int>& smoothaxes,
	       const ::casac::variant& smoothtypes,
	       const std::vector<double>& smoothwidths,
	       const std::vector<double>& d_includepix,
	       const std::vector<double>& d_excludepix,
	       const double peaksnr, const double stddev,
	       const std::string& velocityType, const std::string& out,
	       const std::string& smoothout, const std::string& pgdevice,
	       const int nx, const int ny, const bool yind,
	       const bool overwrite, const bool removeAxis, const bool async)
{
  ::casac::image *rstat = 0;
    try {
    *itsLog << LogOrigin("image", "moments");
    if (detached()) return rstat;

    UnitMap::putUser("pix",UnitVal(1.0), "pixel units");
    Vector<Int> whichmoments(moments);
    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";
    Vector<String> method = toVectorString(in_method);

    Vector<String> kernels;
    if (smoothtypes.type() == ::casac::variant::BOOLVEC) {
      kernels.resize(0);      // unset
    } else if (smoothtypes.type() == ::casac::variant::STRING) {
      sepCommaEmptyToVectorStrings(kernels, smoothtypes.toString());
    } else if (smoothtypes.type() == ::casac::variant::STRINGVEC) {
      kernels = toVectorString(smoothtypes.toStringVec());
    } else {
      *itsLog << LogIO::WARN << "Unrecognized smoothtypes datatype"
	      << LogIO::POST;
    }
    int num = kernels.size();

    Vector<Quantum<Double> > kernelwidths(num);
    Unit u("pix");
    for (int i = 0; i < num; i++) {
      kernelwidths[i] = casa::Quantity(smoothwidths[i],u);
    }
    //
    Vector<Float> includepix;
    num = d_includepix.size();
    if ( !(num == 1 && d_includepix[0]==-1) ) {
      includepix.resize(num);
      for (int i = 0; i < num; i++) includepix[i]=d_includepix[i];
    }
    //
    Vector<Float> excludepix;
    num = d_excludepix.size();
    if ( !(num == 1 && d_excludepix[0]==-1) ) {
      excludepix.resize(num);
      for (int i = 0; i < num; i++) excludepix[i]=d_excludepix[i];
    }

    ImageInterface<Float>* outIm=
      itsImage->moments(whichmoments, axis, *Region, mask, method, 
			Vector<Int>(smoothaxes), kernels,
			kernelwidths, includepix, excludepix, peaksnr, stddev, 
			velocityType, out, smoothout, pgdevice, nx, ny, yind, 
			overwrite, removeAxis); 
    //
    delete Region;
    rstat = new ::casac::image(outIm);
    delete outIm;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

std::string
image::name(const bool strippath)
{
  std::string rstat;
  try {
   *itsLog << LogOrigin("image", "name");
   if (detached()) {
     rstat="none";
   } else {
     rstat = itsImage->name(strippath);
   }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool image::open(const std::string& infile)
{
  bool rstat(false);
  try {
    if(itsLog==0) {
      itsLog=new LogIO();
    }

    // Remove the old itsImage if it still exists
    // I found that when you run on a different image with out deleting it
    // you get core dumps because it can't delete some temp file, wky
    //
    if(itsImage) delete itsImage;

    itsImage=new ImageAnalysis();

    *itsLog << LogOrigin("image", "open");
    
    // Open input image.  We don't handle an Image tool because
    // we would get a bit confused as to who owns the pointer
    rstat= itsImage->open(infile);

    if(rstat) {
	*itsLog << "Opened image " << infile << LogIO::NORMAL2 << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool image::open(const casa::ImageInterface<casa::Float>* inImage)
{
  try {
    if(!itsLog)
      itsLog = new LogIO();

    *itsLog << LogOrigin("image", "open2");

    if(itsImage)
      delete itsImage;
    itsImage= new ImageAnalysis(inImage);
   
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return True;
}


::casac::record*
image::pixelvalue(const std::vector<int>& pixel)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "pixelvalue");
    if (detached()) return rstat;

 
    Record *outRec = itsImage->pixelvalue(Vector<Int>(pixel));
    rstat = fromRecord(*outRec);
    delete outRec;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::putchunk(const ::casac::variant& pixels, const std::vector<int>& blc,
		const std::vector<int>& inc, const bool list,
		const bool locking, const bool replicate)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "putchunk");
    if (detached()) return rstat;

    Array<Float> pixelsArray;
 
    if(pixels.type() == ::casac::variant::DOUBLEVEC ){
      std::vector<double> pixelVector = pixels.getDoubleVec();
      Vector<Int> shape = pixels.arrayshape();
      pixelsArray.resize(IPosition(shape));
      Vector<Double> localpix(pixelVector);
      casa::convertArray(pixelsArray,localpix.reform(IPosition(shape)));
    }
    else if(pixels.type()== ::casac::variant::INTVEC){
      std::vector<int> pixelVector = pixels.getIntVec();
      Vector<Int> shape = pixels.arrayshape();
      pixelsArray.resize(IPosition(shape));
      Vector<Int> localpix(pixelVector);
      casa::convertArray(pixelsArray,localpix.reform(IPosition(shape)));
    }  
    else{
      *itsLog << LogIO::SEVERE 
	      << "pixels is not understood, try using an array " 
	      << LogIO::POST;
      return rstat;
    }

    rstat=itsImage->putchunk(pixelsArray,Vector<Int>(blc), Vector<Int>(inc), 
			     list,locking, replicate);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::putregion(const ::casac::variant& v_pixels,
		 const ::casac::variant& v_pixelmask,
		 const ::casac::record& region,
		 const bool list, const bool usemask,
		 const bool locking, const bool replicateArray)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "putregion");
    if (detached()) return rstat;

    // create Array<Float> pixels
    Array<Float> pixels;
    if (isunset(v_pixels)) {
      // do nothing
    }
    else if(v_pixels.type() == ::casac::variant::DOUBLEVEC ) {
      std::vector<double> pixelVector = v_pixels.getDoubleVec();
      Vector<Int> shape = v_pixels.arrayshape();
      pixels.resize(IPosition(shape));
      Vector<Double> localpix(pixelVector);
      casa::convertArray(pixels,localpix.reform(IPosition(shape)));
    }
    else if(v_pixels.type()== ::casac::variant::INTVEC){
      std::vector<int> pixelVector = v_pixels.getIntVec();
      Vector<Int> shape = v_pixels.arrayshape();
      pixels.resize(IPosition(shape));
      Vector<Int> localpix(pixelVector);
      casa::convertArray(pixels,localpix.reform(IPosition(shape)));
    }  
    else{
      *itsLog << LogIO::SEVERE 
	      << "pixels is not understood, try using an array " 
	      << LogIO::POST;
      return rstat;
    }

    // create Array<Bool> mask
    Array<Bool> mask;
    if (isunset(v_pixelmask)) {
      // do nothing
    }
    else if(v_pixelmask.type() == ::casac::variant::DOUBLEVEC ) {
      std::vector<double> maskVector = v_pixelmask.getDoubleVec();
      Vector<Int> shape = v_pixelmask.arrayshape();
      mask.resize(IPosition(shape));
      Vector<Double> localmask(maskVector);
      casa::convertArray(mask,localmask.reform(IPosition(shape)));
    }
    else if (v_pixelmask.type() == ::casac::variant::INTVEC) {
      std::vector<int> maskVector = v_pixelmask.getIntVec();
      Vector<Int> shape = v_pixelmask.arrayshape();
      mask.resize(IPosition(shape));
      Vector<Int> localmask(maskVector);
      casa::convertArray(mask,localmask.reform(IPosition(shape)));
    }
    else if (v_pixelmask.type() == ::casac::variant::BOOLVEC) {
      std::vector<bool> maskVector = v_pixelmask.getBoolVec();
      Vector<Int> shape = v_pixelmask.arrayshape();
      mask.resize(IPosition(shape));
      Vector<Bool> localmask(maskVector);
      // casa::convertArray(mask,localmask.reform(IPosition(shape)));
      mask=localmask.reform(IPosition(shape));
    }
    else{
      *itsLog << LogIO::SEVERE 
	      << "mask is not understood, try using an array " 
	      << LogIO::POST;
      return rstat;
    }

    if (pixels.size()==0 && mask.size()==0) {
      *itsLog << "You must specify at least either the pixels or the mask"
	      << LogIO::POST;
      return rstat;
    }

    Record * theRegion= toRecord(region);
    rstat=itsImage->putregion(pixels, mask, *theRegion, list, usemask, 
			      locking, replicateArray);
    delete theRegion;

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::rebin(const std::string& outfile, const std::vector<int>& bin, const ::casac::record& region, const ::casac::variant& vmask, const bool dropdeg, const bool overwrite, const bool async)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "rebin");
    if (detached()) return rstat;

    String outFile(outfile);
    Vector<Int> factors(bin);
    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";

    ImageInterface<Float> *pImOut=itsImage->rebin(outFile, factors, *Region, 
						 mask, dropdeg, overwrite);
    rstat = new ::casac::image(pImOut);
    delete pImOut;
    delete Region;
    //Need to delete if the above object makes a copy

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::regrid(const std::string& outfile, const std::vector<int>& inshape,
	      const ::casac::record& csys, const std::vector<int>& inaxes,
	      const ::casac::record& region, const ::casac::variant& vmask,
	      const std::string& method, const int decimate,
	      const bool replicate, const bool doRefChange,
	      const bool dropDegenerateAxes, const bool overwrite,
	      const bool forceRegrid, const bool async)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "regrid");
    if (detached()) return rstat;

    String outFile(outfile);
    Record *coordinates = toRecord(csys);
    
    String methodU(method);
    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]") mask = "";

    Vector<Int> axes;
    if ( !((inaxes.size()==1) && (inaxes[0]==-1)) ) {
      axes = inaxes;
    }

    ImageInterface<Float> * pImOut;

    pImOut= itsImage->regrid(outFile, Vector<Int>(inshape),
			     *coordinates, axes, *Region, 
			     mask, methodU, decimate, replicate, doRefChange,
			     dropDegenerateAxes, overwrite, forceRegrid);

    delete Region;
    delete coordinates;

    rstat = new ::casac::image(pImOut);
    delete pImOut;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::rotate(const std::string& outfile, const std::vector<int>& inshape,
	      const ::casac::variant& inpa, const ::casac::record& region,
	      const ::casac::variant& vmask, const std::string& method,
	      const int decimate, const bool replicate, const bool dropdeg,
	      const bool overwrite, const bool async)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "rotate");
    if (detached()) return rstat;

    String outFile(outfile);
    Vector<Int> shape(inshape);
    Quantum<Double> pa(casaQuantityFromVar(inpa));
    String methodU(method);
    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";
 
    ImageInterface<Float> *pImOut= 
      itsImage->rotate(outFile, shape, pa, *Region, mask, methodU, decimate,
		       replicate, dropdeg, overwrite);

    delete Region;
    rstat = new ::casac::image(pImOut);
    delete pImOut;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::rename(const std::string& name, const bool overwrite)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "rename");
    if (detached()) return rstat;

    rstat = itsImage->rename(name, overwrite);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::replacemaskedpixels(const ::casac::variant& vpixels,
			   const ::casac::record& region,
			   const ::casac::variant& vmask,
			   const bool updateMask, const bool list)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "replacemaskedpixels");
    if (detached()) return rstat;

    String pixels = vpixels.toString();
    Record *pRegion = toRecord(region);
    String maskRegion = vmask.toString();
    if(maskRegion == "[]") maskRegion = "";
      
    rstat = itsImage->replacemaskedpixels(pixels, *pRegion, maskRegion, 
					  updateMask, list) ;
    delete pRegion;

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::restoringbeam()
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "restoringbeam");
    if (detached()) return rstat;

    rstat=fromRecord(itsImage->restoringbeam());
    return rstat;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::sepconvolve(const std::string& outFile, const std::vector<int>& axes,
		   const std::vector<std::string>& types,
		   const ::casac::variant& widths,
		   const double Scale, const ::casac::record& region,
		   const ::casac::variant& vmask, const bool overwrite,
		   const bool async)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "sepconvolve");
    if (detached()) return rstat;
    UnitMap::putUser("pix",UnitVal(1.0), "pixel units");
    Record *pRegion = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]") mask = "";
    Vector<Int> smoothaxes(axes);
    Vector<String> kernels = toVectorString(types);

    int num = 0;
    Vector<Quantum<Double> > kernelwidths;
    if (widths.type() == ::casac::variant::INTVEC) {
      std::vector<int> widthsIVec = widths.toIntVec();
      num = widthsIVec.size();
      std::vector<double> widthsVec(num);
      for (int i = 0; i < num; i++) widthsVec[i] = widthsIVec[i];
      kernelwidths.resize(num);
      Unit u("pix");
      for (int i = 0; i < num; i++) {
	kernelwidths[i] = casa::Quantity(widthsVec[i],u);
      }
    } else if (widths.type() == ::casac::variant::DOUBLEVEC) {
      std::vector<double> widthsVec = widths.toDoubleVec();
      num = widthsVec.size();
      kernelwidths.resize(num);
      Unit u("pix");
      for (int i = 0; i < num; i++) {
	kernelwidths[i] = casa::Quantity(widthsVec[i],u);
      }
    } else if (widths.type() == ::casac::variant::STRING ||
	       widths.type() == ::casac::variant::STRINGVEC) {
      toCasaVectorQuantity(widths, kernelwidths);
      num = kernelwidths.size();
    } else {
      *itsLog << LogIO::WARN << "Unrecognized kernelwidth datatype"
	      << LogIO::POST;
      return rstat;
    }
    if (kernels.size() == 1 && kernels[0]=="") {
      kernels.resize(num);
      for (int i = 0; i < num; i++) kernels[i]="gauss";
    }
    if (smoothaxes.size() == 0 ||
	( (smoothaxes.size() == 1) && (smoothaxes[0]=-1) ) ) {
      smoothaxes.resize(num);
      for (int i = 0; i < num; i++) smoothaxes[i]=i;
    }

    ImageInterface<Float> * pImOut=
      itsImage->sepconvolve(outFile, smoothaxes, kernels, kernelwidths, Scale,
			    *pRegion, mask, overwrite);
    delete pRegion;
    // Return image
    rstat = new ::casac::image(pImOut);
    delete pImOut;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::set(const ::casac::variant& vpixels, const int pixelmask,
	   const ::casac::record& region, const bool list)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "set");
    if (detached()) return rstat;
    
    String pixels = vpixels.toString();
    if(pixels == "[]") pixels = "";
    Record *pRegion = toRecord(region);

    if (pixels=="" && pixelmask==-1) {
      *itsLog << LogIO::WARN
	      << "You must specify at least either the pixels or the mask to set"
	      << LogIO::POST;
      return rstat;
    }
    rstat = itsImage->set(pixels, pixelmask, *pRegion, list);
    delete pRegion;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::setbrightnessunit(const std::string& unit)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "setbrightnessunit");
    if (detached()) return rstat;

    rstat =itsImage->setbrightnessunit(unit);
    if (!rstat) {
      *itsLog << "Unable to set brightness units" << LogIO::EXCEPTION;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::setcoordsys(const ::casac::record& csys)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "setcoordsys");
    if (detached()) return rstat;

    Record *coordinates = toRecord(csys);
    rstat = itsImage->setcoordsys(*coordinates);
    delete coordinates; 
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::sethistory(const std::string& origin, const std::vector<std::string>& history)
{
  bool rstat(false);
  try {
    if (detached()) {
      return false;
    }
    if ( (history.size() == 1) && (history[0].size() == 0) ) {
      LogOrigin lor("image", "sethistory");
      *itsLog << lor << "history string is empty" << LogIO::POST;
    } else {
      Vector<String> History = toVectorString(history);
      rstat=itsImage->sethistory(origin, History);
    }
  } catch (AipsError x) {
    LogOrigin lor("image", "sethistory");
    *itsLog << lor << LogIO::SEVERE << "Exception Reported: "
	    << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::setmiscinfo(const ::casac::record& info)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "setmiscinfo");
    if (detached()) return rstat;

    Record *tmp = toRecord(info);
    rstat = itsImage->setmiscinfo(*tmp);
    delete tmp;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

std::vector<int>
image::shape()
{
  std::vector<int> rstat(0);
  *itsLog << LogOrigin("image", "shape");
  if (detached()) return rstat;
  try {
    Vector<Int> dummy = itsImage->shape();
    dummy.tovector(rstat);
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::setrestoringbeam(const ::casac::variant& major,
			const ::casac::variant& minor,
			const ::casac::variant& pa,
			const ::casac::record& beam, const bool deleteIt,
			const bool log)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "setrestoringbeam");
    if (detached()) return rstat;

    //
    casa::Quantity beam0(casaQuantityFromVar(major));
    casa::Quantity beam1(casaQuantityFromVar(minor));
    casa::Quantity beam2(casaQuantityFromVar(pa));

    Record *rec = toRecord(beam);
    
    rstat =itsImage->setrestoringbeam(beam0,beam1,beam2,*rec,deleteIt,log);
    delete rec;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::statistics(const std::vector<int>& axes,
		  const ::casac::record& region, const ::casac::variant& mask,
		  const std::vector<std::string>& plotstats,
		  const std::vector<double>& includepix,
		  const std::vector<double>& excludepix,
		  const std::string& plotter, const int nx, const int ny,
		  const bool list, const bool force, const bool disk,
		  const bool robust, const bool verbose, const bool async)
{
   ::casac::record *rstat = 0;
   try {
   *itsLog << LogOrigin("image", "statistics");
    if (detached()){
	    *itsLog << "Image not attached" << LogIO::POST;
	    return rstat;
    }

// Convert region from Glish record to ImageRegion. Convert mask to ImageRegion
// and make SubImage.    

    Record *regionRec = toRecord(region);
    String mtmp = mask.toString();
    if(mtmp == "false" || mtmp == "[]")
	    mtmp = "";

    Vector<String> plotStats = toVectorString(plotstats);
    if (plotStats.size() == 0) {
      plotStats.resize(2);
      plotStats[0]="mean";
      plotStats[1]="sigma";
    }

    //cout << "CMPT in AXES: ";
    //copy (axes.begin(),axes.end(), ostream_iterator<int>(cout," "));
    //cout << endl;
    Vector<Int> tmpaxes(axes);
    if (tmpaxes.size()==1 && tmpaxes[0]==-1) {
      tmpaxes.resize(0);
    }
    Vector<Float> tmpinclude;
    Vector<Float> tmpexclude;
    if(!(includepix.size() == 1 && includepix[0] == -1)){
	tmpinclude.resize(includepix.size());
        for(uInt i=0;i<includepix.size();i++)
            tmpinclude[i] = includepix[i];
    }
    if(!(excludepix.size() == 1 && excludepix[0] == -1)){
	tmpexclude.resize(excludepix.size());
        for(uInt i=0;i<excludepix.size();i++)
            tmpexclude[i] = excludepix[i];
    }

    Record retval;
    Bool status;
    status = itsImage->statistics(retval, tmpaxes, *regionRec, mtmp, 
				  plotStats, tmpinclude,
				  tmpexclude, plotter, nx, ny, list, force, 
				  disk,robust, verbose);
    if (status) {
      rstat = fromRecord(retval);
    }
    delete regionRec;
  } catch (AipsError x) {
        *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
   return rstat;
}

bool
image::twopointcorrelation(const std::string& outfile,
			   const ::casac::record& region,
			   const ::casac::variant& vmask,
			   const std::vector<int>& axes,
			   const std::string& method, const bool overwrite)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "twopointcorrelation");
    if (detached()) {
      return rstat;
    }

    String outFile(outfile);
    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";
    Vector<Int> iAxes;
    if ( !(axes.size()==1 && axes[0]==-1) ) {
      iAxes = axes;
    }
 
    rstat = itsImage->twopointcorrelation(outFile, *Region, mask, 
					  iAxes, method, overwrite) ;
    delete Region;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::image *
image::subimage(const std::string& outfile, const ::casac::record& region,
		const ::casac::variant& vmask, const bool dropDegenerateAxes,
		const bool overwrite, const bool list)
{
  ::casac::image *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "subimage");
    if (detached()) return rstat;

    Record *Region = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]")
	    mask = "";
    ImageInterface<Float> * tmpIm = 
      itsImage->subimage(outfile, *Region, mask, 
			 dropDegenerateAxes,
			 overwrite, list);
    rstat = new ::casac::image(tmpIm);
    delete tmpIm;
    
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

std::vector<std::string>
image::summary(casac::record& header, const std::string& doppler, const bool list, const bool pixelorder)
{
  std::vector<string> rstat;
  try {
    *itsLog << LogOrigin("image", "summary");
    if (detached()) {
      return rstat;
    }        
    Record retval;
    rstat = fromVectorString(itsImage->summary(retval, doppler, list, 
					       pixelorder));   
    header = *fromRecord(retval);


  } catch (AipsError x) {
        *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
   return rstat;
}

bool
image::tofits(const std::string& fitsfile, const bool velocity,
	      const bool optical, const int bitpix,
	      const double minpix, const double maxpix,
	      const ::casac::record& region, const ::casac::variant& vmask,
	      const bool overwrite, 
	      const bool dropdeg, const bool deglast,
	      const bool dropstokes, const bool stokeslast, const bool async)
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "tofits");
    if (detached()) return rstat;

    Record *pRegion = toRecord(region);
    String mask = vmask.toString();
    if(mask == "[]") {
      mask = "";
    }
    
    rstat=itsImage->tofits(fitsfile, velocity, optical, bitpix, minpix, 
			   maxpix, *pRegion, mask, overwrite, 
			   dropdeg, deglast,
			   dropstokes, stokeslast);
    delete pRegion;
    //
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::toASCII(const std::string& outfile, const ::casac::record& region,
	       const ::casac::variant& mask, const std::string& sep,
	       const std::string& format, const double maskvalue,
	       const bool overwrite)
{
  // sep is hard-wired as ' ' which is what imagefromascii expects
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "toASCII");
    if (detached()) return rstat;


    String Mask;
    if(mask.type()== ::casac::variant::BOOLVEC) {
      Mask = "";
    }
    else if(mask.type()== ::casac::variant::STRING || 
       mask.type()== ::casac::variant::STRINGVEC){
      Mask = mask.toString();
    }
    Record* pRegion=toRecord(region);
    rstat= itsImage->toASCII(outfile, *pRegion, Mask, sep, format, maskvalue, 
			     overwrite);
    delete pRegion;

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

std::string
image::type()
{
  return "image";
}

//std::vector<double>
::casac::record*
image::topixel(const ::casac::variant& value)
{
  //  std::vector<double> rstat;
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "topixel");
    if (detached()) return rstat;

    Vector<Int> bla;
    CoordinateSystem cSys = itsImage->coordsys(bla);
    ::casac::coordsys mycoords;
    //NOT using itsImage->toworld as most of the math is in casac namespace
    //in coordsys...should revisit this when casac::coordsys is cleaned
    mycoords.setcoordsys(cSys);
    rstat = mycoords.topixel(value);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

::casac::record*
image::toworld(const ::casac::variant& value, const std::string& format)
{
  ::casac::record *rstat = 0;
  try {
    *itsLog << LogOrigin("image", "toworld");
    if (detached()) return rstat;

    Vector<Double> pixel;
    if (isunset(value)) {
      pixel.resize(0);
    } else if (value.type() == ::casac::variant::DOUBLEVEC) {
      pixel = value.getDoubleVec();
    } else if (value.type() == ::casac::variant::INTVEC) {
      Vector<Int> ipixel = value.getIntVec();
      Int n = ipixel.size();
      pixel.resize(n);
      for (int i=0 ; i < n; i++) pixel[i]=ipixel[i];
    } else if (value.type() == ::casac::variant::RECORD) {
      ::casac::variant localvar(value);
      Record *tmp = toRecord(localvar.asRecord());
      if (tmp->isDefined("numeric")) {
	pixel = tmp->asArrayDouble("numeric");
      } else {
	*itsLog << LogIO::SEVERE << "unsupported record type for value"
		<< LogIO::EXCEPTION;
	return rstat;
      }
      delete tmp;
    } else {
      *itsLog << LogIO::SEVERE << "unsupported data type for value"
	      << LogIO::EXCEPTION;
      return rstat;
    }
    rstat = fromRecord(itsImage->toworld(pixel, format));

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool
image::unlock()
{
  bool rstat(false);
  try {
    *itsLog << LogOrigin("image", "unlock");
    if (detached()) return rstat;

    rstat=itsImage->unlock();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}


bool image::detached() const
{
  bool rstat(false);
  if (itsImage == 0 || itsImage->detached()) {
    *itsLog << LogOrigin("image", "detached");
    *itsLog << LogIO::SEVERE
	    << "Image is detached - cannot perform operation." << endl
	    << "Call image.open('filename') to reattach." << LogIO::POST;
    //    *itsLog << "Can not continue" << LogIO::EXCEPTION;
    rstat = true;
  }
  return rstat;
}                                            

::casac::record*
image::setboxregion(const std::vector<double>& blc,
		    const std::vector<double>& trc,
		    const bool frac, const std::string& infile)
{
  casac::record* rstat=0;
  try {

    *itsLog << LogOrigin("image", "setboxregion");
    if (detached()) return rstat;  

    Record tempR(itsImage->setboxregion(Vector<Double>(blc),
				     Vector<Double>(trc), frac, infile));
    rstat=fromRecord(tempR);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

bool image::maketestimage(const std::string& outfile, const bool overwrite)
{
  bool rstat(false);
  try {
    if(itsLog==0) itsLog=new LogIO();
    if(itsImage) delete itsImage;
    itsImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "maketestimage");
    rstat = itsImage->maketestimage(outfile, overwrite);

  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }

  return rstat;
  
}

::casac::image *
image::newimagefromimage(const std::string& infile, const std::string& outfile,
			 const ::casac::record& region,
			 const ::casac::variant& vmask, const bool dropdeg,
			 const bool overwrite)
{
  ::casac::image *outImg = 0;
  try {
    if (itsLog==0)
      itsLog=new LogIO();
    //itsImage=new ImageAnalysis();
    casa::ImageAnalysis *newImage = new ImageAnalysis();
    *itsLog << LogOrigin("image", "newimage");

    String mask;

    if(vmask.type() == ::casac::variant::BOOLVEC) {
      mask = "";
    }
    else if(vmask.type() == ::casac::variant::STRING ||
	    vmask.type() == ::casac::variant::STRINGVEC){
      mask=vmask.toString();
    }
    else if(vmask.type() == ::casac::variant::RECORD){
      Record *theMaskRegion;
      theMaskRegion=toRecord(vmask.getRecord());
      *itsLog << LogIO::SEVERE 
	      << "Don't support region masking yet, only valid LEL " 
	      << LogIO::POST;
      return outImg;
    } 
    else{
      *itsLog << LogIO::SEVERE 
	      << "Mask is not understood, try a valid LEL string " 
	      << LogIO::POST;
      return outImg;
    }

    Record *Region = toRecord(region);
    ImageInterface<Float>* outIm =
      newImage->newimage(infile, outfile, *Region,mask, dropdeg, overwrite);
    delete Region;
    if (outIm) {
      outImg = new ::casac::image(outIm);
      delete outIm;
    } else {
      outImg = new ::casac::image();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outImg;
}

::casac::image *
image::newimagefromfile(const std::string& fileName)
{
  ::casac::image *outImg = 0;
  try {
    if (itsLog==0)
      itsLog=new LogIO();
    casa::ImageAnalysis *newImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "newimagefromfile");

    ImageInterface<Float>* outIm =
      newImage->newimagefromfile(fileName);
    if (outIm) {
      outImg = new ::casac::image(outIm);
      delete outIm;
    } else {
      outImg = new ::casac::image();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outImg;
}

::casac::image *
image::newimage(const std::string& fileName)
{
  ::casac::image *outImg = 0;
  try {
    if (itsLog==0)
      itsLog=new LogIO();
    casa::ImageAnalysis *newImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "newimagefromfile");

    ImageInterface<Float>* outIm =
      newImage->newimagefromfile(fileName);
    if (outIm) {
      outImg = new ::casac::image(outIm);
      delete outIm;
    } else {
      outImg = new ::casac::image();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outImg;
}

::casac::image *
image::newimagefromarray(const std::string& outfile, const ::casac::variant& pixels, const ::casac::record& csys, const bool linear, const bool overwrite, const bool log)
{
  ::casac::image *outImg = 0;
  try {
    if(itsLog==0)
      itsLog=new LogIO();
    //if(itsImage) delete itsImage;
    casa::ImageAnalysis *newImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "newimagefromarray");

    // Some protection.  Note that a Glish array, [], will
    // propagate through to here to have ndim=1 and shape=0
    Vector<Int> shape = pixels.arrayshape();
    uInt ndim = shape.size();
    if (ndim == 0) {
      *itsLog << "The pixels array is empty" << LogIO::EXCEPTION;
    }
    for (uInt i=0; i<ndim; i++) {
      if (shape(i) <=0) {
	*itsLog << "The shape of the pixels array is invalid" << LogIO::EXCEPTION;
      }
    }

    Array<Float> pixelsArray;
    if(pixels.type() == ::casac::variant::DOUBLEVEC ){
      std::vector<double> pixelVector = pixels.getDoubleVec();
      Vector<Int> shape = pixels.arrayshape();
      pixelsArray.resize(IPosition(shape));
      Vector<Double> localpix(pixelVector);
      casa::convertArray(pixelsArray,localpix.reform(IPosition(shape)));
    }
    else if(pixels.type()== ::casac::variant::INTVEC){
      std::vector<int> pixelVector = pixels.getIntVec();
      Vector<Int> shape = pixels.arrayshape();
      pixelsArray.resize(IPosition(shape));
      Vector<Int> localpix(pixelVector);
      casa::convertArray(pixelsArray,localpix.reform(IPosition(shape)));
    }  
    else{
      *itsLog << LogIO::SEVERE 
	      << "pixels is not understood, try using an array " 
	      << LogIO::POST;
      outImg = new::casac::image();
      return outImg;
    }

    Record *coordinates = toRecord(csys);
    ImageInterface<Float>* outIm =
      newImage->newimagefromarray(outfile, pixelsArray, *coordinates,
				  linear, overwrite, log);
    delete coordinates;
    if (outIm) {
      outImg = new::casac::image(outIm);
      delete outIm;
    } else {
      outImg = new::casac::image();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outImg;
}

::casac::image *
image::newimagefromshape(const std::string& outfile, const std::vector<int>& shape, const ::casac::record& csys, const bool linear, const bool overwrite, const bool log)
{
  ::casac::image *outImg = 0;
  try {
    if(itsLog==0)
      itsLog=new LogIO();
    //if(itsImage) delete itsImage;
    casa::ImageAnalysis *newImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "newimagefromshape");
    Record *coordinates = toRecord(csys);
    ImageInterface<Float>* outIm =
      newImage->newimagefromshape(outfile, Vector<Int>(shape), *coordinates, 
				  linear, overwrite, log);
    if (outIm) {
      outImg = new::casac::image(outIm);
      delete outIm;
    } else {
      outImg = new::casac::image();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
	    << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outImg;
}

::casac::image * 
image::newimagefromfits(const std::string& outfile, const std::string& fitsfile,
			const int whichrep, const int whichhdu,
			const bool zeroBlanks, const bool overwrite)
{
  ::casac::image *outImg = 0;
  try {
    if (itsLog==0)
      itsLog=new LogIO();
    casa::ImageAnalysis *newImage=new ImageAnalysis();
    *itsLog << LogOrigin("image", "newimagefromfits");

    ImageInterface<Float>* outIm =
      newImage->newimagefromfits(outfile, fitsfile, whichrep, whichhdu, 
				 zeroBlanks, overwrite);
    if (outIm) {
      outImg = new ::casac::image(outIm);
      delete outIm;
    } else {
      outImg = new ::casac::image();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return outImg;
}

::casac::variant*
image::makearray(const double v, const std::vector<int>& shape)
{
  int ndim = shape.size();
  int nelem = 1;
  for (int i = 0; i < ndim; i++)
    nelem *= shape[i];
  std::vector<double> element(nelem);
  for (int i = 0; i < nelem; i++)
    element[i] = v;
  return new ::casac::variant(element, shape);
}

::casac::record* image::echo(const ::casac::record& v, const bool godeep)
{
casac::record* rstat=0;
  try {
    Record *tempo=toRecord(v);
    rstat=fromRecord(*(itsImage->echo(*tempo, godeep)));
							 
    delete tempo;

  } catch (AipsError x) {
    *itsLog << LogOrigin("image", "echo");
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }

  return rstat;
}

void image::outputvariant(::casac::variant& v)
{
  try {
    int len=5;
    std::vector<int> vi(len);
    for (uInt i = 0 ; i < vi.size(); i++) {
      vi[i]=i;
    }
    std::vector<int> vi_shape = *new std::vector<int>(1);
    vi_shape[0]=len;
    v = new ::casac::variant(vi,vi_shape);
  } catch (AipsError x) {
    *itsLog << LogOrigin("image", "outputvariant");
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  cout << "all is well so far" << endl;
}

casac::record*
image::recordFromQuantity(const casa::Quantity q)
{
  ::casac::record *r=0;
  try {
    *itsLog << LogOrigin("image", "recordFromQuantity");
    String error;
    casa::Record R;
    if (QuantumHolder(q).toRecord(error, R)) {
      r = fromRecord(R);
    } else {
      *itsLog << LogIO::SEVERE << "Could not convert quantity to record."
              << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
            << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

::casac::record*
image::recordFromQuantity(const Quantum<Vector<Double> >& q)
{
  ::casac::record *r=0;
  try {
    *itsLog << LogOrigin("image", "recordFromQuantity");
    String error;
    casa::Record R;
    if (QuantumHolder(q).toRecord(error, R)) {
      r = fromRecord(R);
    } else {
      *itsLog << LogIO::SEVERE << "Could not convert quantity to record."
              << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: "
            << x.getMesg() << LogIO::POST;
    RETHROW(x);
  }
  return r;
}

casa::Quantity
image::casaQuantityFromVar(const ::casac::variant& theVar){
  casa::Quantity retval;
  try {
    *itsLog << LogOrigin("image", "casaQuantityFromVar");
    casa::QuantumHolder qh;
    String error;
    if(theVar.type()== ::casac::variant::STRING ||
       theVar.type()== ::casac::variant::STRINGVEC){
      if(!qh.fromString(error, theVar.toString())){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting quantity "<< LogIO::POST;
      }
      retval=qh.asQuantity();
    }
    if(theVar.type()== ::casac::variant::RECORD){
      //NOW the record has to be compatible with QuantumHolder::toRecord
      ::casac::variant localvar(theVar); //cause its const
      Record * ptrRec = toRecord(localvar.asRecord());
      if(!qh.fromRecord(error, *ptrRec)){
        *itsLog << LogIO::SEVERE << "Error " << error
                << " in converting quantity "<< LogIO::POST;
      }
      delete ptrRec;
      retval=qh.asQuantity();
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
            << LogIO::POST;
    RETHROW(x);
  }
  return retval;
}

} // casac namespace
