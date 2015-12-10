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

    void calc(const String& expr, Bool verbose);

    // regions should be a Record of Records having different regions

    Bool calcmask(
    	const String& mask, Record& regions,
    	const String& name, const Bool asdefault = True
    );

    CoordinateSystem coordsys(const Vector<int>& axes);

    Bool open(const String& infile);

    inline static String className() {const static String x = "ImageAnalysis"; return x; }

    Bool detached();

    // Return a record of the associates ImageInterface 
    Bool toRecord(RecordInterface& rec);

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
