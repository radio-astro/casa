//# DLFont.h : Class to provide encapsulation for fonts
//# Copyright (C) 2000,2002
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
//# $Id:


#ifndef TRIALDISPLAY_DLFONT_H
#define TRIALDISPLAY_DLFONT_H

#include <casa/BasicSL/String.h>
#include <casa/Exceptions.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// A class designed to manage several representations of the same font
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" test="" demos="">
// </reviewed>

// <prerequisite>
// </prerequisite>

// <etymology>
// DLFont is a class designed to manage multiple representations of the
// same font.
// </etymology>

// <motivation>
// Different DL media (e.g. X11 and postscript) have different ways of
// representing fonts. The user, however should be presented with a single
// representation across all media types. This class is designed to represent
// a single font, as observed by the user, but also to contain different
// representations of the font for each media.
// </motivation>

// <thrown>
// <li> None.
// </thrown>

// <todo asof="2002/05/13">
// <li> Nothing known.
// </todo>

	class DLFont {

	public:
		enum FontDescription { Name, XFontDescription, PSFontName };

		// (Required) default constructor.
		DLFont();

		// The constructor accepts any description of the font. By default it is
		// the 'name' of the font. You can also construct a DLFont by either of
		// its other descriptions, by using the appropriate 'FontDescription'.
		// e.g to select Times by its PostScript name:
		// <srcblock>
		// DLFont myFont("Times-Roman", DLFont::PSFontName);
		// </srcblock>
		// is equivalent to :
		// <srcblock>
		// DLFont myFont("-adobe-times-medium-r-*--*", DLFont::XFontDescription);
		// </srcblock>
		// or, more simply :
		// <srcblock>
		// DLFont myFont("Times - Roman");
		// </srcblock>
		DLFont(const String& description,
		       const DLFont::FontDescription whatType = DLFont::Name,
		       const Int& size = 12);

		// Copy constructor using copy semantics.
		DLFont(const DLFont& other);

		// Destructor.
		virtual ~DLFont();

		// Copy assignment.
		DLFont &operator=(const DLFont &other);

		// Return the current font as a XLFD. Although it states "noSize", it may
		// actually return a size. e.g. a call to "setXValue", providing
		// an entry in the size field will result in a XLFD with a valid size field.
		// However, if you do a "setSize", then call this method, the change in
		// size will not be reflected.
		String getXValueNoSize();

		// Return the current font as a post-script recognisable name
		//  String getPSValueNoSize();

		// Return the current fonts desired size
		Int getSize();

		// Return the current font as a XLFD, with the specific size.
		// NB If there is any matrix transform in the pixel size field
		// of the XLFD, this will not preserve it. To do operations
		// e.g. rotation, extract the desired font size (getSize) and the
		// font info (getXFontNoSize) seperately before applying required
		// transforms
		String getXValue();

		// Return the current font as a PS recognisable name, with the specific size
		// 'tagged onto' the end of it.
		String getPSValue();

		// Return the name (as it should be presented to users) of the font.
		String getName();

		// Set the name of the font
		void setName(const String& newName);

		// Set the desired size (pixelSize)
		void setSize(const Int newSize);

		// Set the XLFD for this font (if you wish to set at matrix for any fields
		// e.g. set the size field as a rotation matrix to rotate text, you will
		// need to handle the size of the text independantly of this class.
		// You can use the getXValueNoSize for that. i.e. class will not
		// preserve rotated/scaled text via matrix transforms
		void setXValue(const String& newX11);

		// Set the Postscript representation of this font
		void setPSValue(const String& newPS);

		Vector<String> getAllNames() {
			return itsNames;
		}

	protected:


	private:
		void fillArrays();
		Bool lookUp(const String& desc, const DLFont::FontDescription,
		            Int& returnIndex);

		Vector<String> itsXFonts;
		Vector<String> itsPSFonts;
		Vector<String> itsNames;

		Int itsSize;
		Int itsCurrentFont;
	};


} //# NAMESPACE CASA - END

#endif




