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

#include <casa/Quanta/Quantum.h>

#include <imageanalysis/ImageTypedefs.h>

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

class ImageAnalysis {
public:

    ImageAnalysis();

    ImageAnalysis(SPIIF image);

    ImageAnalysis(SPIIC image);

    virtual ~ImageAnalysis();

    void addnoise(
    	const String& type, const Vector<Double>& pars,
    	const Record& region, const Bool zero,
    	const std::pair<Int, Int> *const &seeds
    );

    // The current object is replaced by the result of the calculation
    void imagecalc(
    	const String& outfile,
    	const String& pixels,
    	const Bool overwrite = False
    );

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

    void calc(const String& expr, Bool verbose);

    // regions should be a Record of Records having different regions

    Bool calcmask(
    	const String& mask, Record& regions,
    	const String& name, const Bool asdefault = True
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
        const String& smoothout="",
        const Bool overwrite=False, const Bool drop=True,
        const Bool stretchMask=False
    );

    void setMomentsProgressMonitor( ImageMomentsProgressMonitor* progressMonitor );

    Bool open(const String& infile);

    Record* pixelvalue(const Vector<Int>& pixel);
    void pixelValue (Bool& offImage, Quantum<Double>& value, Bool& mask,
                     Vector<Int>& pos) const;

    Bool putregion(const Array<Float>& pixels, const Array<Bool>& pixelmask, 
                   Record& region, const Bool list = False, 
                   const Bool usemask = True, 
                   const Bool locking = True, const Bool replicate = False);

    SPIIF rotate(
    	const String& outfile, const Vector<int>& shape,
        const Quantity& pa, Record& region,
        const String& mask, const String& method="cubic",
        const Int decimate=0, const Bool replicate=False,
        const Bool dropdeg=False, const Bool overwrite=False,
        const Bool extendMask=False
    ) const;

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

    // if doVelocity=True, compute spectral velocities as well as frequencies, if False, only
    // compute frequencies.
    Record toworld(const Vector<double>& value, const String& format = "n", Bool doVelocity=True) const;

    Bool detached();

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

    // Return a record of the associates ImageInterface 
    Bool toRecord(RecordInterface& rec);
    // Create a pagedimage if imagename is not "" else create a tempimage
    Bool fromRecord(const RecordInterface& rec, const String& imagename="");

    // get the associated ImageInterface object
    SPCIIF getImage() const;
    SPIIF getImage();

    SPCIIC getComplexImage() const;
    SPIIC getComplexImage();

    Bool isFloat() const { return _imageFloat ? true : false; }

 private:
    SPIIF _imageFloat;
    SPIIC _imageComplex;

    std::unique_ptr<LogIO> _log;

    // Having private version of IS and IH means that they will
    // only recreate storage images if they have to

    SHARED_PTR<ImageHistograms<Float> > _histograms;
    IPosition last_chunk_shape_p;

    CountedPtr<ImageRegion> pOldHistRegionRegion_p, pOldHistMaskRegion_p;
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

// Convert a Record to a CoordinateSystem
    casa::CoordinateSystem*
      makeCoordinateSystem(const casa::Record& cSys,
                           const casa::IPosition& shape) const;
    
    // Make a block of regions from a Record
    static void _makeRegionBlock(
    	PtrBlock<const ImageRegion*>& regions,
    	const Record& Regions
    );

    // Some helper functions that needs to be in casa namespace coordsys

    Record _worldVectorToRecord(
        const Vector<Double>& world, Int c, const String& format, 
        Bool isAbsolute, Bool showAsAbsolute, Bool doVelocity
    ) const;

    Record _worldVectorToMeasures(
        const Vector<Double>& world, Int c, Bool abs, Bool doVelocity
    ) const;

    void _onlyFloat(const String& method) const;

    template<class T> static void _destruct(ImageInterface<T>& image);

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

