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

#ifndef IMAGES_IMAGECOLLAPSER_H
#define IMAGES_IMAGECOLLAPSER_H

#include <casa/Logging/LogIO.h>
#include <images/Images/ImageInterface.h>
#include <casa/namespace.h>

namespace casa {

    class ImageCollapser {
        // <summary>
        // Top level interface which allows collapsing of images along a single axis. An aggregate method
    	// (average, sum, etc) is applied to the collapsed pixels.
        // </summary>

        // <reviewed reviewer="" date="" tests="" demos="">
        // </reviewed>

        // <prerequisite>
        // </prerequisite>

        // <etymology>
        // Collapses image.
        // </etymology>

        // <synopsis>
        // High level interface for collapsing an image along a single axis.
        // </synopsis>

        // <example>
        // <srcblock>
        // ImageCollapser collapser();
        // collapser.collapse();
        // </srcblock>
        // </example>

    public:

    	ImageCollapser(
    	    String aggString,const String& imagename,
    	    const String& region, const String& box,
    	    const String& chanInp, const String& stokes,
    	    const String& maskInp, const uInt compressionAxis,
            const String& outname, const Bool overwrite
    	);

        // destructor
        ~ImageCollapser();

        // perform the collapse. If <src>wantReturn</src> is True, return a pointer to the
        // collapsed image. The returned pointer is created via new(); it is the caller's
        // responsibility to delete the returned pointer. If <src>wantReturn</src> is False,
        // a NULL pointer is returned and pointer deletion is performed internally.
        ImageInterface<Float>* collapse(const Bool wantReturn) const;

    private:
        LogIO *_log;
        ImageInterface<Float> *_image;
        Record _regionRecord;
        String _regionString, _stokesString, _chan, _mask, _outname;
        uInt _compressionAxis;
        Bool _overwrite;
        //AggregateType _aggType;
        Float (*_aggregateFunction)(const Array<Float>&);


        // does the lion's share of constructing the object, ie checks validity of
        // inputs, etc.
        void _construct(
        	String& aggString, const String& imagename,
        	const String& box, const String& regionName
        );

        void _setAggregateFunction(String& aggString);
    };
}

#endif
