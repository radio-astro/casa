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

#ifndef IMAGEANALYSIS_IMAGEHEADER_H
#define IMAGEANALYSIS_IMAGEHEADER_H

#include <images/Images/ImageInterface.h>

#include <casa/namespace.h>

#include <tr1/memory>

namespace casa {

template <class T> class ImageHeader {
	// <summary>
	// Top level class for accessing and mutating image "header" items.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Accesses image "header" items.
	// </etymology>

	// <synopsis>
	// High level interface for accessing image "header" items.
	// </synopsis>

	// <example>
	// <srcblock>
	// PagedImage<Float> pm("my.im");
	// ImageHeader(pm);
	// collapser.collapse();
	// </srcblock>
	// </example>

public:

	// The caller should not delete the reference to <src>image</src>
	// before the newly created ImageHeader object is destroyed or a segfault
	// will ensue.
	ImageHeader(const std::tr1::shared_ptr<ImageInterface<T> >& image);

	// convert the header info to a Record and list to logger if verbose=True
	Record toRecord(Bool verbose);

private:

	const static String _BEAMMAJOR, _BEAMMINOR, _BEAMPA,
		_CTYPE, _DATAMAX, _DATAMIN, _EQUINOX, _IMTYPE,
		_MASKS, _MAXPIXPOS, _MAXPOS, _MINPIXPOS, _MINPOS,
		_OBJECT, _OBSDATE, _OBSERVER, _PROJECTION,
		_RESTFREQ, _REFFREQTYPE, _SHAPE, _TELESCOPE;

	const std::tr1::shared_ptr<ImageInterface<T> > _image;
	std::auto_ptr<LogIO> _log;
	Record _header;

	ImageHeader() : _image(0), _log(0) {}

	void _toLog() const;

	void _fieldToLog(const String& field) const;

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageHeader.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC


#endif
