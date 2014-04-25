//# ImageAnalysis.h: Image analysis and handling tool
//# Copyright (C) 2007
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA

#ifndef _IMAGEANALYSIS__H__
#define _IMAGEANALYSIS__H__

// PLEASE DO *NOT* ADD ADDITIONAL METHODS TO THIS CLASS

//# put includes here

#include <casa/Quanta/Quantum.h>

#include <imageanalysis/ImageTypedefs.h>

namespace std {
template<class T> class auto_ptr;
}

namespace casa {

class CoordinateSystem;
class ImageMomentsProgressMonitor;
class ImageRegion;
class LatticeExprNode;
class LELImageCoord;
class RecordInterface;

template<class T> class ImageHistograms;

// <summary>
// Image analysis and handling tool
// </summary>

// <synopsis>
// This the casapy image tool.
// One time it should be merged with pyrap's image tool ImageProxy.
// </synopsis>

// NOTE: NEW METHODS SHOULD NOT BE ADDED TO THIS CLASS. PLEASE USE THE ImageTask.h
// architecture for adding new functionality for image analysis. If you do not understand,
// please consult with me, dmehring@nrao.edu. If you add new methods to ImageAnalysis, I will contact
// you to remove them. Please save us both the annoyance of that.

class ImageAnalysis
{
  public:

    ImageAnalysis();

    ImageAnalysis(SPIIF image);

    ImageAnalysis(SPIIC image);

    virtual ~ImageAnalysis();

    Bool addnoise(const String& type, const Vector<Double>& pars,
                  Record& region, const Bool zero = False);

    // The current object is replaced by the result of the calculation
    void imagecalc(
    	const String& outfile,
    	const String& pixels,
    	const Bool overwrite = False
    );

    SPIIF imageconcat(const String& outfile,
                                        const Vector<String>& infiles, 
                                        const Int axis, 
                                        const Bool relax = False, 
                                        const Bool tempclose = True, 
                                        const Bool overwrite = False);

    Bool imagefromarray(const String& outfile, Array<Float>& pixels, 
                        const Record& csys, const Bool linear = False, 
                        const Bool overwrite = False, const Bool log = True);

    Bool imagefromascii(const String& outfile, const String& infile, 
                        const Vector<Int>& shape, const String& sep, 
                        const Record& csys, const Bool linear = False, 
                        const Bool overwrite = False);

    Bool imagefromfits(const String& outfile, const String& infile, 
                       const Int whichrep = 0, const Int whichhdu = 0, 
                       const Bool zeroblanks = False, 
                       const Bool overwrite = False);

    Bool imagefromimage(const String& outfile, const String& infile, 
                        Record& region, const String& mask, 
                        const Bool dropdeg = False, 
                        const Bool overwrite = False);

    ImageInterface<Float> * convolve(
    	const String& outfile,
        Array<Float>& kernel,
        const String& kernImage,
        const Double scale,
        Record& region, String& mask,
        const Bool overwrite=False,
        const Bool async=False,
        const Bool stretch=False
    );

    Record* boundingbox(const Record& region) const;

    String brightnessunit() const;

    void calc(const String& pixels);

    // regions should be a Record of Records having different regions

    Bool calcmask(
    	const String& mask, Record& regions,
    	const String& name, const Bool asdefault = True
    );

    SPIIF continuumsub(const String& outline,
                                         const String& outcont, Record& region,
                                         const Vector<int>& channels, 
                                         const String& pol = "", 
                                         const Int fitorder = 0, 
                                         const Bool overwrite = false);

    SPIIF convolve2d(
    		const String& outfile, const Vector<Int>& axes,
            const String& type, const Quantity& major,
            const Quantity& minor, const Quantity& pa,
            Double scale, Record& region, const String& mask,
            const Bool overwrite = False, const Bool stretch=False,
            const Bool targetres = False
    );

    CoordinateSystem coordsys(const Vector<int>& axes);

    Record* coordmeasures(Quantity& intensity, Record& direction, 
                          Record& frequency, Record& velocity, 
                          const Vector<double>& pixel);

   Matrix<Float> decompose(
		   Record& region, const String& mask,
           const Bool simple = false,
           const Double threshold = -1,
           const Int ncontour = 11,
           const Int minrange = 1,
           const Int naxis = 2,
           const Bool fit = True,
           const Double maxrms = -1,
           const Int maxretry = -1,
           const Int maxiter = 256,
           const Double convcriteria = 0.0001
   );

   Matrix<Float> decompose(
		   Matrix<Int>& blcs, Matrix<Int>& trcs,
		   Record& region, const String& mask,
		   const Bool simple = false,
		   const Double threshold = -1,
		   const Int ncontour = 11,
		   const Int minrange = 1,
		   const Int naxis = 2,
		   const Bool fit = True,
		   const Double maxrms = -1,
		   const Int maxretry = -1,
		   const Int maxiter = 256,
		   const Double convcriteria = 0.0001,
		   const Bool stretch=False
   );

    Record deconvolvecomponentlist(
    	const Record& complist, const Int channel, const Int polarization
    );

    Bool remove(Bool verbose=true);

    Record findsources(const Int nmax, const Double cutoff, Record& region, 
                        const String& mask, const Bool point = True, 
                        const Int width = 5, const Bool negfind = False);

    Record* getslice(const Vector<Double>& x, const Vector<Double>& y, 
                     const Vector<Int>& axes, const Vector<Int>& coord, 
                     const Int npts = 0, const String& method = "linear");

    Vector<Bool> haslock();

    Record histograms(
    	const Vector<Int>& axes, Record& region,
        const String& mask, const Int nbins,
        const Vector<Double>& includepix, const Bool gauss,
        const Bool cumu, const Bool log, const Bool list,
        const Bool force=False,
        const Bool disk=False, const Bool extendMask=False
    );

    Bool insert(
    	const String& infile, Record& region,
    	const Vector<double>& locate, Bool verbose
    );

    Bool ispersistent();

    Bool lock(const Bool writelock = False, const Int nattempts = 0);

    Bool makecomplex(const String& outfile, const String& imag, Record& region,
                     const Bool overwrite = False);

    Vector<String> maskhandler(const String& op,const Vector<String>& nam);

    Record miscinfo();

    Bool modify(
    	Record& model, Record& region , const String& mask,
        const Bool subtract = True, const Bool list=True, const Bool extendMask=False
    );

    Record maxfit(Record& region, const Bool point, const Int width = 5, 
                   const Bool negfind = False, const Bool list = True);

    ImageInterface<Float> * moments(
    	const Vector<Int>& moments, const Int axis, Record& region,
    	const String& mask, const Vector<String>& method,
    	const Vector<Int>& smoothaxes,
    	const Vector<String>& smoothtypes,
        const Vector<Quantity>& smoothwidths,
        const Vector<Float>& includepix,
        const Vector<Float>& excludepix,
        const Double peaksnr, const Double stddev,
        const String& doppler = "RADIO",  const String& outfile = "",
        const String& smoothout="", const String& plotter="/NULL",
        const Int nx=1, const Int ny=1,  const Bool yind=False,
        const Bool overwrite=False, const Bool drop=True,
        const Bool stretchMask=False
    );

    void setMomentsProgressMonitor( ImageMomentsProgressMonitor* progressMonitor );

    Bool open(const String& infile);

    Record* pixelvalue(const Vector<Int>& pixel);
    void pixelValue (Bool& offImage, Quantum<Double>& value, Bool& mask,
                     Vector<Int>& pos) const;

    Bool putchunk(
    	const Array<Float>& pixels, const Vector<Int>& blc,
    	const Vector<Int>& inc, const Bool list = False,
    	const Bool locking = True, const Bool replicate = False
    );

    Bool putchunk(
    	const Array<Complex>& pixels, const Vector<Int>& blc,
    	const Vector<Int>& inc, const Bool list = False,
    	const Bool locking = True, const Bool replicate = False
    );

    Bool putregion(const Array<Float>& pixels, const Array<Bool>& pixelmask, 
                   Record& region, const Bool list = False, 
                   const Bool usemask = True, 
                   const Bool locking = True, const Bool replicate = False);

    ImageInterface<Float>* rotate(
    	const String& outfile,
        const Vector<int>& shape,
        const Quantity& pa, Record& region,
        const String& mask,
        const String& method="cubic",
        const Int decimate=0,
        const Bool replicate=False,
        const Bool dropdeg=False,
        const Bool overwrite=False,
        const Bool extendMask=False
    );

    Bool rename(const String& name, const Bool overwrite = False);

    ImageInterface<Float>* sepconvolve(
    	const String& outfile,
        const Vector<Int>& axes,
        const Vector<String>& types,
        const Vector<Quantity>& widths,
        Double scale,
        Record& region,
        const String& mask,
        const bool overwrite=False,
        const bool extendMask=False
    );

    Bool set(const String& pixels, const Int pixelmask, 
             Record& region, const Bool list = false);

    bool setcoordsys(const Record& csys);

    inline static String className() {const static String x = "ImageAnalysis"; return x; }

    bool twopointcorrelation(
    	const String& outfile, Record& region,
        const String& mask, const Vector<Int>& axes,
        const String& method="structurefunction",
        const Bool overwrite=False, const Bool stretch=False
    );

    Record summary(
    	const String& doppler = "RADIO",
    	const Bool list = True,
    	const Bool pixelorder = True,
    	const Bool verbose = False
    );

    Bool tofits(
    	const String& outfile, const Bool velocity, const Bool optical,
        const Int bitpix, const Double minpix, const Double maxpix,
        Record& region, const String& mask,
        const Bool overwrite=False,
        const Bool dropdeg=False, const Bool deglast=False,
        const Bool dropstokes=False, const Bool stokeslast=False,
        const Bool wavelength=False, const Bool airWavelength=False,
        const String& origin="", Bool stretch=False,
	const Bool history=True
    );

    Bool toASCII(
    	const String& outfile, Record& region, const String& mask,
        const String& sep=" ", const String& format="%e",
        const Double maskvalue=-999, const Bool overwrite=False,
        const Bool extendMask=False
    );

    Record toworld(const Vector<double>& value, const String& format = "n") const;

    Bool detached();

    Record setregion(const Vector<Int>& blc, const Vector<Int>& trc, 
                      const String& infile = "");

    Record setboxregion(const Vector<Double>& blc, const Vector<Double>& trc,
                        const Bool frac = False, const String& infile = "");

    //make test image...cube or 2d (default)
    bool maketestimage(const String& outfile="", const Bool overwrite=False, 
                       const String& imagetype="2d");

    ImageInterface<Float> * newimage(const String& infile,
                                     const String& outfile,
                                     Record& region,
                                     const String& Mask,
                                     const bool dropdeg = False,
                                     const bool overwrite = False);

    ImageInterface<Float> * newimagefromfile(const String& fileName);

    ImageInterface<Float> * newimagefromarray(const String& outfile,
                                              Array<Float> & pixelsArray,
                                              const Record& csys,
                                              const Bool linear = False,
                                              const Bool overwrite = False,
                                              const Bool log = True);

    ImageInterface<Float> * newimagefromfits(const String& outfile,
                                             const String& infile, 
                                             const Int whichrep = 0,
                                             const Int whichhdu = 0, 
                                             const Bool zeroblanks = False, 
                                             const Bool overwrite = False);


    /**
      * Populates two vectors, zxaxisval and zyaxisval, representing
      * values along an axis (usually the spectral axis) with a corresponding
      * summary intensity computed based on the region passed in.
      *
      * @param x a Vector of x-coordinates describing a region.
      * @param y a Vector of y-coordinates describing a region.  Together, (x,y)
      * 		describe the region.  So for a point region, both vectors would have
      * 		length one.  For a rectangular reqion, they would describe the blc and
      * 		trc points of the rectangle.  For a polygonal region, the vectors would
      * 		have undetermined length and together be the coordinates of the corners
      * 		of the polygon.
      * @param zxaxisval contains the x-coordinates of the frequency profile.
      * @param zyaxisval contains the y-coordinates of the frequency profile.
      * @param xytype the coordinate system used by the input vectors x,y.  "world"
      *      is the default, but an alternative might be "pixel".
      * @param specaxis -values include "pixel", "frequency", "radio velocity"
      *      "optical velocity", "wavelength" or "air wavelength"
      * @param whichStokes - a parameter that is currently not being used by the
      * 		profiler (not sure of the purpose).
      * @param whichTabular - find a frequency profile along a tabular axis instead of
      * 		the spectral axis.  This one was put in because people wanted to profile
      * 		images that had tabular axes, but not spectral axes.  The idea is that if
      * 		it is not at its default value of -1, then the method will use the specified
      * 		tabular axis.
      * @param whichLinear - another one that is currently not being used (not sure of the purpose).
      * @param xunits - the units for the spectral (z) axis.  Possible values are
      * 	    Hz, MHz, MHz, GHz, m/s, km/s, mm, um, nm, Angstrom
      * @param specframe - String form of the MFrequency types such as TOPO, BARY, etc
      * @param combineType - Method used for combining pixels.  Current values are an enum
      * 	    in the PlotType enum of QtProfile.qo.h: PMEAN, PMEDIAN, PSUM, PFLUX.  For error
      * 	    plotting and overplotting, the profiler also seems to be passing values in from
      * 	    the ExtrType enum:  MEAN, MEDIAN, SUM, MSE, RMSE, SQRTSUM, NSQRTSUM, FLUX, EFLUX
      * @param whichQuality - the profiler is not making any use of this one (passing in a
      * 	    default value).  Not sure what it is there for.
      * @param restValue - Specify a different rest frequency.  Currently the user can reset
      * 	    the rest frequency for the image, and when they do this, a new frequency profile
      * 	    is computed with the new rest value.
      * @param beamChannel - currently this is only used when the combine type is FLUX.
      * @param shape - this was added to distinguish 'rectangle' regions from 'ellipse'
      * 	       regions (when the vectors x and y both have size 2).  Other types of region
      * 	       can be distinguished from the size of the x,y vectors.
      */
    Bool getFreqProfile(const Vector<Double>& x,
			const Vector<Double>& y,
			Vector<Float>& zxaxisval, Vector<Float>& zyaxisval,
			const String& xytype="world",
			const String& specaxis="freq",
			const Int& whichStokes=0,
			const Int& whichTabular=-1,
			const Int& whichLinear=0,
			const String& xunits="",
			const String& specframe="",
			const Int &combineType=0,
			const Int& whichQuality=0,
			const String& restValue="",
			Int beamChannel = -1,
			const String& shape="rectangle");

    // Return a record of the associates ImageInterface 
    Bool toRecord(RecordInterface& rec);
    // Create a pagedimage if imagename is not "" else create a tempimage
    Bool fromRecord(const RecordInterface& rec, const String& imagename="");

    // get the associated ImageInterface object
    SPCIIF getImage() const;
    SPIIF getImage();

    SPCIIC getComplexImage() const;
    SPIIC getComplexImage();


    // If file name empty make TempImage (allowTemp=T) or do nothing.
    // Otherwise, make a PagedImage from file name and copy mask and
    // misc from inimage.   Returns T if image made, F if not
    static SPIIF makeExternalImage (
    	const String& fileName,
    	const CoordinateSystem& cSys,
    	const IPosition& shape,
    	const ImageInterface<Float>& inImage,
    	LogIO& os, Bool overwrite=False,
    	Bool allowTemp=False,
    	Bool copyMask=True
    );

    Bool isFloat() const { return _imageFloat; }

 private:

    //Used for single point extraction.
    //Functions to get you back a spectral profile at direction position x, y.
     //x, y are to be in the world coord value or pixel value...user specifies
     //by parameter xytype ("world" or "pixel").
     //On success returns true
     //return value of profile is in zyaxisval, zxaxisval contains the spectral
     //values at which zyaxisval is evaluated its in the spectral type
     //specified by specaxis...possibilities are "pixel", "frequency", "radio velocity"
     //"optical velocity", "wavelength" or "air wavelength" (the code checks for the
     //keywords "pixel", "freq", "vel", "optical", and "radio" in the string)
     // if "vel" is found but no "radio" or "optical", the full relativistic velocity
     // is generated (MFrequency::RELATIVISTIC)
     // xunits determines the units of the x-axis values...default is "GHz" for
     // freq and "km/s" for vel, "mm" for wavelength and "um" for "air wavelength"
     //PLEASE note that the returned value of zyaxisval are the units of the image
     //specframe can be a valid frame from MFrequency...i.e LSRK, LSRD etc...
     Bool getFreqProfile(const Vector<Double>& xy,
    		 Vector<Float>& zxaxisval, Vector<Float>& zyaxisval,
    		 const String& xytype="world",
    		 const String& specaxis="freq",
    		 const Int& whichStokes=0,
    		 const Int& whichTabular=-1,
    		 const Int& whichLinear=0,
    		 const String& xunits="",
    		 const String& specframe="",
    		 const Int& whichQuality=0,
    		 const String& restValue="");
    
    SPIIF _imageFloat;
    SPIIC _imageComplex;

    std::auto_ptr<LogIO> _log;

    // Having private version of IS and IH means that they will
    // only recreate storage images if they have to

    std::tr1::shared_ptr<ImageHistograms<Float> > _histograms;
    IPosition last_chunk_shape_p;

    std::tr1::shared_ptr<ImageRegion> pOldHistRegionRegion_p, pOldHistMaskRegion_p;
    Bool oldHistStorageForce_p;
    ImageMomentsProgressMonitor* imageMomentsProgressMonitor;

   
    // Center refpix apart from STokes
    void centreRefPix (casa::CoordinateSystem& cSys,
                       const casa::IPosition& shape) const;
    
    // Convert types
    //casa::ComponentType::Shape convertModelType (casa::Fit2D::Types typeIn) const;
   
    // Delete private ImageHistograms objects
    void deleteHist();
   
    Bool _haveRegionsChanged (
    	ImageRegion* pNewRegionRegion,
    	ImageRegion* pNewMaskRegion
    );
    
// Make a new image with given CS
    void _make_image(
    	const String &image,
    	const CoordinateSystem& cSys,
    	const IPosition& shape,
    	Bool log=True,
    	Bool overwrite=False, const String& type="f"
    );

// Convert a Record to a CoordinateSystem
    casa::CoordinateSystem*
      makeCoordinateSystem(const casa::Record& cSys,
                           const casa::IPosition& shape) const;
    
    // Make a block of regions from a Record
    void makeRegionBlock(casa::PtrBlock<const casa::ImageRegion*>& regions,
                         const casa::Record& Regions,
                         casa::LogIO& logger);
    // Set the cache
    void set_cache(const casa::IPosition& chunk_shape) const;
    

    // Some helper functions that needs to be in casa namespace coordsys

    Record worldVectorToRecord (const Vector<Double>& world, 
                                Int c, const String& format, 
                                Bool isAbsolute, Bool showAsAbsolute) const;

    Record worldVectorToMeasures(const Vector<Double>& world, 
                                 Int c, Bool abs) const;

    //return a vector of the spectral axis values in units requested
    //e.g "vel", "fre" or "pix"..specVal has to be sized already.  If a
    //valid tabular axis is specified (>=0) it takes precedence over the
    //spectral axis.
    Bool getSpectralAxisVal(const String& specaxis, Vector<Float>& specVal, 
                            const CoordinateSystem& cSys, const String& xunits, 
                            const String& freqFrame="", const String& restValue="",
                            int tabularAxisIndex = -1);
    //return a vector of the spectral axis values in units requested
    //e.g "vel", "fre" or "pix"..specVal has to be sized already

    SPIIF _fitpolynomial(
    	const String& residfile,
    	const String& fitfile,
    	const String& sigmafile,
    	const Int axis, const Int order,
    	Record& region, const String& mask,
    	const bool overwrite = false
    );

    void _onlyFloat(const String& method) const;

    template<class T> Bool _putchunk(
        ImageInterface<T>& image,
    	const Array<T>& pixels, const Vector<Int>& blc,
    	const Vector<Int>& inc, const Bool list,
    	const Bool locking, const Bool replicate
    );

    template<class T> static void _destruct(ImageInterface<T>& image);

    template<class T> Bool _setrestoringbeam(
    	SPIIT image,
    	const Quantity& major, const Quantity& minor,
    	const Quantity& pa, const Record& rec,
    	const bool deleteIt, const bool log,
        Int channel, Int polarization
    );

    template<class T> Record _summary(
    	const ImageInterface<T>& image,
    	const String& doppler, const Bool list,
    	const Bool pixelorder, const Bool verbose
    );

    template<class T> SPIIT _imagecalc(
    	const LatticeExprNode& node, const IPosition& shape,
    	const CoordinateSystem& csys, const LELImageCoord* const imCoord,
    	const String& outfile,
    	Bool overwrite, const String& expr
    );

    template<class T> void _calc(
    	SPIIT image,
    	const LatticeExprNode& node
    );

    template<class T> Bool _calcmask(
    	SPIIT image,
    	const LatticeExprNode& node,
    	const String& name, const Bool makedefault
    );

};

} // casac namespace

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageAnalysis2.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif

