//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
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
//#
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGES_IMAGEFITTER_H
#define IMAGES_IMAGEFITTER_H

#include <measures/Measures/Stokes.h>
#include <lattices/LatticeMath/Fit2D.h>
#include <casa/Logging/LogIO.h>
#include <components/ComponentModels/ComponentList.h>
#include <images/Images/ImageInterface.h>
#include <components/ComponentModels/ComponentType.h>
#include <casa/namespace.h>

namespace casa {

    class ImageFitter {
        // <summary>
        // Top level interface to ImageAnalysis::fitsky to handle inputs, bookkeeping etc and
        // ultimately call fitsky to do fitting
        // </summary>

        // <reviewed reviewer="" date="" tests="" demos="">
        // </reviewed>

        // <prerequisite>
        // </prerequisite>

        // <etymology>
        // Fits components to sources in images (ImageSourceComponentFitter was deemed to be to long
        // of a name)
        // </etymology>

        // <synopsis>
        // ImageFitter is the top level interface for fitting image source components. It handles most
        // of the inputs, bookkeeping etc. It can be instantiated and its one public method, fit,
        // run from either a C++ app or python.
        // </synopsis>

        // <example>
        // <srcblock>
        // ImageFitter fitter(...)
        // fitter.fit()
        // </srcblock>
        // </example>

        public:

            // constructor appropriate for processing command line arguments (from Unix shell).
            ImageFitter(Int argc, char *argv[]);
        
            // constructor approprate for API calls.
            // Parameters:
            // <ul>
            // <li>imagename - the name of the input image in which to fit the models</li>
            // <li>box - A 2-D rectangular box in which to use pixels for the fitting, eg box=100,120,200,230
            // In cases where both box and region are specified, box, not region, is used.</li>
            // <li>region - Named region to use for fitting</li>
            // <li>ngaussInp - Number of gaussians to attempt to fit</li>
            // <li>chanInp - Zero-based channel number on which to do the fit. Only a single channel can be
            // specified.</li>
            // <li>stokes - Stokes plane on which to do the fit. Only a single Stokes parameter can be
            // specified.</li>
            // <li> maskInp - Mask (as LEL) to use as a way to specify which pixels to use </li>
            // <li> includepix - Pixel value range to include in the fit. includepix and excludepix
            // cannot be specified simultaneously. </li>
            // <li> excludepix - Pixel value range to exclude from fit</li>
            // <li> residualInp - Name of residual image to save. Blank means do not save residual image</li>
            ImageFitter(
                const String& imagename, const String& box="", const String& region="",
                const uInt ngaussInp=1, const uInt chanInp=0, const String& stokes="I",
                const String& maskInp="",
                const Vector<Float>& includepix = Vector<Float>(0),
                const Vector<Float>& excludepix = Vector<Float>(0)
                // , const String& residualInp=""
            ); 

            // destructor
            ~ImageFitter();

            // constructor appropriate for API calls
            // Do the fit. If componentList is specified, store the fitted components in that object.
            ComponentList fit() const; 

        private:

            LogIO *itsLog;
            ImageInterface<Float> *image;
            ImageRegion imRegion;
            uInt ngauss, chan;
            String stokesString, mask; //residual;
            Vector<Float> includePixelRange, excludePixelRange;

            // does the lion's share of constructing the object, ie checks validity of
            // inputs, etc.
            void _construct(
                const String& imagename, const String& box, const String& region
            );

            // determine the region based on the inputs
            void _doRegion(const String& box, const String& region);

            // process the 'box' command line argument and return the associated region as
            // a record.
            void _processBox(const String& box);

            // check the validity of the image-related parameters.
            void _checkImageParameterValidity() const;
  
    };
}

#endif
