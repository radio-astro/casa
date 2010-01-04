//# DLFont.cc : Class to provide encapsulation for fonts
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

#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <display/Display/DLFont.h>

namespace casa { //# NAMESPACE CASA - BEGIN

DLFont::DLFont() {
  fillArrays();
  itsSize = 12;
  itsCurrentFont = 0;
}

DLFont::DLFont(const String& description, 
	       const DLFont::FontDescription whatType,
	       const Int& size) :
  itsSize(size),
  itsCurrentFont(-1) {
  
  fillArrays();

  if (!lookUp(description, whatType, itsCurrentFont)) {
    throw(AipsError(String("Couldn't find the requested font (" 
			   +description+ ") in list of known fonts")));
  }
  
}

Bool DLFont::lookUp(const String& description, 
		    const DLFont::FontDescription whatType,
		    Int& returnIndex) {
  uInt nNames = itsNames.nelements();

  if (nNames <= 0 ||
      itsXFonts.nelements() != nNames ||
      itsPSFonts.nelements() != nNames) {

    throw (AipsError("Bad arrays being used to search for font index!"));
  }
  
  Bool found = False;
  uInt i=0;

  while(!found && i < itsNames.nelements()) {
    switch(whatType) {
    case DLFont::Name:
      found = (description == itsNames(i));
      break;
    case DLFont::XFontDescription:
      found = (description == itsXFonts(i));
      break;
    case DLFont::PSFontName:
      found = (description == itsPSFonts(i));
      break;
    default: 
      throw(AipsError("Bad font \'type\' selected"));
      break;
    }
    
    if (found) {
      returnIndex = i;
    } else {
      i++;
    }
    
  }
  
  return found;
}

DLFont::DLFont(const DLFont& other) :
  itsXFonts(other.itsXFonts),
  itsPSFonts(other.itsPSFonts),
  itsNames(other.itsNames),
  itsSize(other.itsSize),
  itsCurrentFont(other.itsCurrentFont) {

}

DLFont::~DLFont() {

}

DLFont &DLFont::operator=(const DLFont &other) {
  itsXFonts = other.itsXFonts;
  itsPSFonts = other.itsPSFonts;
  itsNames = other.itsNames;
  itsSize = other.itsSize;
  return *this;
}

String DLFont::getXValueNoSize() {
  String tempArray[14];
  String toReturn = itsXFonts(itsCurrentFont);
  Int numberSplits = split(toReturn, tempArray, 14, '-');

  // Could it be an alias?
  if (numberSplits <= 1) return toReturn; 
  
  // else pad it out to 14 fields (XLFD)
  for (Int i = numberSplits; i < 14 ; i++) 
    toReturn += "-*";
  
  return toReturn;
}

String DLFont::getName() {
  return itsNames(itsCurrentFont);
}

//String DLFont::getPSValueNoSize() {
//  return itsPSFont;
//}

Int DLFont::getSize() {
  return itsSize;
}

String DLFont::getXValue() {
  String beforeSize = getXValueNoSize();
  Int numberFields = beforeSize.freq('-');

  String fontComponents[14];

  beforeSize.erase(0,1);
  if (split(beforeSize, fontComponents, numberFields, '-') <=1 )
    // I guess it could be an alias...
    return getXValueNoSize();
    //throw(AipsError("Badly formed XLFD"));
  
  String afterSize;
  
  for (Int i=0 ; i<numberFields ; i++) {
    afterSize += '-';
    // 6 = Pixel size,
    // 7 = Point size. 
    if (i == 7) 
      afterSize += afterSize.toString((getSize()*10));
    else 
      afterSize += fontComponents[i];
  }

  // Fill out the fields we dont have info for with wildcards
  for (Int i=numberFields; i < 14; i++)
    afterSize += "-*";

  return afterSize;
}

String DLFont::getPSValue() {
  //  String utils;
  return itsPSFonts(itsCurrentFont);
  //  return (getPSValueNoSize() + utils.toString(getSize()));
}

void DLFont::setName(const String& newName) {
  //itsName = newName;

  if (!lookUp(newName, DLFont::Name, itsCurrentFont)) {
    throw(AipsError("Unkown font name used to try and set font!"));
  }
  
}

void DLFont::setSize(const Int newSize) {
  itsSize = newSize;
}

void DLFont::setXValue(const String& newX11) {
  if (!lookUp(newX11, DLFont::XFontDescription, itsCurrentFont)) {
    throw(AipsError("Unkown X11 LFD used to try and set font!"));
  }

  //  itsXFont = newX11;
}

void DLFont::setPSValue(const String& newPS) {

  if (!lookUp(newPS, DLFont::PSFontName, itsCurrentFont)) {
    throw(AipsError("Unkown PS Font name used to try and set font!"));
  }

//  itsPSFont = newPS;
}


void DLFont::fillArrays() {
  itsXFonts.resize(13);
  itsPSFonts.resize(13);
  itsNames.resize(13);

  itsNames[0] = "Default";
  itsNames[1] = "Times - Roman";
  itsNames[2] = "Times - Bold";
  itsNames[3] = "Times - Italic";
  itsNames[4] = "Times - Bold, Italic";
  itsNames[5] = "Courier";  
  itsNames[6] = "Courier - Bold";
  itsNames[7] = "Courier - Oblique";
  itsNames[8] = "Helvetica";
  itsNames[9] = "Helvetica - Bold";
  itsNames[10] = "Helvetica - Oblique";
  itsNames[11] = "Symbol (Greek)";
  itsNames[12] = "ZapfChancery - Medium, Italic";

  itsXFonts[0] = "-misc-fixed-medium-r-semicondensed--*";
  itsXFonts[1] = "-adobe-times-medium-r-*--*";
  itsXFonts[2] = "-adobe-times-bold-r-*--*";
  itsXFonts[3] = "-adobe-times-medium-i-*--*";
  itsXFonts[4] = "-adobe-times-bold-i-*--*";
  itsXFonts[5] = "-adobe-courier-medium-r-*--*";  
  itsXFonts[6] = "-adobe-courier-bold-r-*--*";
  itsXFonts[7] = "-adobe-courier-medium-o-*--*";
  itsXFonts[8] = "-adobe-helvetica-medium-r-*--*";
  itsXFonts[9] = "-adobe-helvetica-bold-r-*--*";
  itsXFonts[10] = "-adobe-helvetica-medium-o-*--*";
  itsXFonts[11] = "*-symbol-medium-r-*--*";
  itsXFonts[12] = "-*-zapfchancery-medium-i-*--*";
  

  itsPSFonts[0] = "Default";
  itsPSFonts[1] = "Times-Roman";
  itsPSFonts[2] = "Times-Bold";
  itsPSFonts[3] = "Times-Italic";
  itsPSFonts[4] = "Times-BoldItalic";
  itsPSFonts[5] = "Courier";  
  itsPSFonts[6] = "Courier-Bold";
  itsPSFonts[7] = "Courier-Oblique";
  itsPSFonts[8] = "Helvetica";
  itsPSFonts[9] = "Helvetica-Bold";
  itsPSFonts[10] = "Helvetica-Oblique";
  itsPSFonts[11] = "Symbol";
  itsPSFonts[12] = "ZapfChancery-MediumItalic";

}






} //# NAMESPACE CASA - END

