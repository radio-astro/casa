//# ImageMetaData.h: Meta information for Images
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef IMAGES_IMAGEINPUTPARAMETERS_H
#define IMAGES_IMAGEINPUTPARAMETERS_H

#include <images/Images/ImageMetaData.h>
#include <casa/aips.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class to validate and store common input parameters for image tasks.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=ImageInterface>ImageInterface</linkto>
// </prerequisite>

// <etymology>
// Validates and stores input parameters common to many image analysis tasks.
// </etymology>

// <synopsis> 
// Class to validate and store common input parameters for image tasks.
// </synopsis>

// <example>
// </example>

// <motivation> 
// This class is meant to provide an object-oriented interface for validating
// and accessing input parameters common to many image analysis tasks.
// </motivation>
// <todo>
// add storage and access of position-like inputs
// add storage and access of polarization inputs
// </todo>


class ImageInputParameters {

    public:
        ImageInputParameters(const ImageInterface<Float>& image);


        // Sets the spectral ranges. Returns True if the input specification is valid,
        // False otherwise. If invalid, no ranges are set; if previously successfully set,
        // those ranges remain. If invalid, a user-friendly
        // error message is returned in errmsg, else the returned value of errmsg is
        // the empty string.
        Bool setSpectralRanges (String& errmsg, const String& specification);

        // Return the currently set spectral ranges or an empty vector if they have
        // not yet been successfully set.
        Vector<uInt> getSpectralRanges() const;

    private:
        ImageMetaData _metaData;
        Vector<uInt> _spectralRanges;

        Vector<uInt> _consolidateAndOrderRanges(const Vector<uInt>& ranges) const;
 


};

} //# NAMESPACE CASA - END
#endif

