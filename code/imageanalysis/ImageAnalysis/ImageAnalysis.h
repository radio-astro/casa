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

#include <utility>

using namespace std;

namespace casa {

class CoordinateSystem;
class ImageMomentsProgressMonitor;
class ImageRegion;
class LatticeExprNode;
class LELImageCoord;
class RecordInterface;

//template<class T> class ImageHistograms;

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

    void calc(const String& expr, Bool verbose);

    // regions should be a Record of Records having different regions

    Bool calcmask(
    	const String& mask, Record& regions,
    	const String& name, const Bool asdefault = True
    );

    CoordinateSystem coordsys(const Vector<int>& axes);

    Record* coordmeasures(Quantum<Float>& intensity, Record& direction,
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


    Bool open(const String& infile);

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

    inline static String className() {const static String x = "ImageAnalysis"; return x; }

    bool twopointcorrelation(
    	const String& outfile, Record& region,
        const String& mask, const Vector<Int>& axes,
        const String& method="structurefunction",
        const Bool overwrite=False, const Bool stretch=False
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

    Bool detached();

    ImageInterface<Float> * newimage(const String& infile,
                                     const String& outfile,
                                     Record& region,
                                     const String& Mask,
                                     const bool dropdeg = False,
                                     const bool overwrite = False);

    // Return a record of the associates ImageInterface 
    Bool toRecord(RecordInterface& rec);
    // Create a pagedimage if imagename is not "" else create a tempimage
    //Bool fromRecord(const RecordInterface& rec, const String& imagename="");

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

    IPosition last_chunk_shape_p;

    //ImageMomentsProgressMonitor* imageMomentsProgressMonitor;

   
    // Center refpix apart from STokes
    void centreRefPix (casa::CoordinateSystem& cSys,
                       const casa::IPosition& shape) const;
    

    void _onlyFloat(const String& method) const;

    template<class T> static void _destruct(ImageInterface<T>& image);

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

}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageAnalysis2.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
