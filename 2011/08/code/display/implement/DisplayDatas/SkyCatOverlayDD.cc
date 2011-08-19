//# SkyCatOverlay.cc: sky catalogue overlay displaydata
//# Copyright (C) 1999,2000,2001,2002,2004
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

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Utilities/Regex.h>
#include <display/DisplayDatas/SkyCatOverlayDD.h>
#include <display/DisplayDatas/SkyCatOverlayDM.h>
#include <display/Display/WorldCanvas.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SkyCatOverlayDD::SkyCatOverlayDD(Table *table) :
  PassiveTableDD(table),
  itsParamNameColumn(0),
  itsParamLineWidth(0),
  itsParamMarkerType(0),
  itsParamMarkerSize(0),
  itsParamMarkerColor(0),
  itsParamMapColumn(0),
  itsParamCharacterFont(0),
  itsParamCharacterSize(0),
  itsParamCharacterColor(0),
  itsParamCharacterAngle(0),
  itsParamLabelXOffset(0),
  itsParamLabelYOffset(0) {
  if (!determineDirectionColumnMapping()) {
    throw(AipsError("Invalid sky catalogue table; cannot determine coordinate "
		    "column mapping"));
  }
  constructParameters();
}

SkyCatOverlayDD::SkyCatOverlayDD(const String tablename) :
  PassiveTableDD(tablename),
  itsParamNameColumn(0),
  itsParamLineWidth(0),
  itsParamMarkerType(0),
  itsParamMarkerSize(0),
  itsParamMarkerColor(0),
  itsParamMapColumn(0),
  itsParamCharacterFont(0),
  itsParamCharacterSize(0),
  itsParamCharacterColor(0),
  itsParamCharacterAngle(0),
  itsParamLabelXOffset(0),
  itsParamLabelYOffset(0) {
  if (!determineDirectionColumnMapping()) {
    throw(AipsError("Invalid sky catalogue table; cannot determine coordinate "
		    "column mapping"));
  }
  constructParameters();
}

SkyCatOverlayDD::~SkyCatOverlayDD() {
  destructParameters();
}

void SkyCatOverlayDD::setDefaultOptions() {
  PassiveTableDD::setDefaultOptions();
}

Bool SkyCatOverlayDD::setOptions(Record &rec, Record &recOut) {
  Bool ret = PassiveTableDD::setOptions(rec, recOut);
  Bool localchange = False;
  localchange = (itsParamNameColumn->fromRecord(rec) || localchange);
  localchange = (itsParamLineWidth->fromRecord(rec) || localchange);
		
  localchange = (itsParamMarkerType->fromRecord(rec) || localchange);
  localchange = (itsParamMarkerSize->fromRecord(rec) || localchange);
  localchange = (itsParamMarkerColor->fromRecord(rec) || localchange);
  localchange = (itsParamMapColumn->fromRecord(rec) || localchange);
		
  localchange = (itsParamCharacterFont->fromRecord(rec) || localchange);
  localchange = (itsParamCharacterSize->fromRecord(rec) || localchange);
  localchange = (itsParamCharacterColor->fromRecord(rec) || localchange);
  localchange = (itsParamCharacterAngle->fromRecord(rec) || localchange);
  localchange = (itsParamLabelXOffset->fromRecord(rec) || localchange);
  localchange = (itsParamLabelYOffset->fromRecord(rec) || localchange);
  
  ret = ret || localchange;
  return ret;
}

Record SkyCatOverlayDD::getOptions() {
  Record rec = PassiveTableDD::getOptions();

  itsParamNameColumn->toRecord(rec);
  
  itsParamLineWidth->toRecord(rec);

  itsParamMarkerType->toRecord(rec);
  itsParamMarkerSize->toRecord(rec);
  itsParamMarkerColor->toRecord(rec);
  itsParamMapColumn->toRecord(rec);

  itsParamCharacterFont->toRecord(rec);
  itsParamCharacterSize->toRecord(rec);
  itsParamCharacterColor->toRecord(rec);
  itsParamCharacterAngle->toRecord(rec);
  itsParamLabelXOffset->toRecord(rec);
  itsParamLabelYOffset->toRecord(rec);

  return rec;
}

CachingDisplayMethod *SkyCatOverlayDD::newDisplayMethod(
    WorldCanvas *worldCanvas,
    AttributeBuffer *wchAttributes,
    AttributeBuffer *ddAttributes,
    CachingDisplayData *dd) {
  return new SkyCatOverlayDM(worldCanvas, wchAttributes, ddAttributes, dd);
}

AttributeBuffer SkyCatOverlayDD::optionsAsAttributes() {
  AttributeBuffer buffer = PassiveTableDD::optionsAsAttributes();

  buffer.set(itsParamNameColumn->name(), itsParamNameColumn->value());
  //new
  buffer.set("coordinatetype", itsDirectionTypeColumn);
  //new

  buffer.set("longitudecolumn", itsLongitudeColumn);
  buffer.set("latitudecolumn", itsLatitudeColumn);

  buffer.set(itsParamLineWidth->name(), itsParamLineWidth->value());

  buffer.set(itsParamMarkerType->name(), itsParamMarkerType->value());
  buffer.set(itsParamMarkerSize->name(), itsParamMarkerSize->value());
  buffer.set(itsParamMarkerColor->name(), itsParamMarkerColor->value());
  buffer.set(itsParamMapColumn->name(), itsParamMapColumn->value());

  buffer.set(itsParamCharacterFont->name(), itsParamCharacterFont->value());
  buffer.set(itsParamCharacterSize->name(), itsParamCharacterSize->value());
  buffer.set(itsParamCharacterColor->name(), itsParamCharacterColor->value());
  buffer.set(itsParamCharacterAngle->name(), itsParamCharacterAngle->value());
  buffer.set(itsParamLabelXOffset->name(), itsParamLabelXOffset->value());
  buffer.set(itsParamLabelYOffset->name(), itsParamLabelYOffset->value());

  return buffer;
}

SkyCatOverlayDD::SkyCatOverlayDD() :
  PassiveTableDD() {
}

SkyCatOverlayDD::SkyCatOverlayDD(const SkyCatOverlayDD &other) :
  PassiveTableDD(other) {
}

void SkyCatOverlayDD::operator=(const SkyCatOverlayDD &) {
}

Bool SkyCatOverlayDD::conformsToCS(const WorldCanvasHolder& wch) {
  // Determine whether DD can draw on the current coordinate system of the
  // given WC[H].  This DD requires two sky coordinates; it tests that
  // direction coordinates are encoded into both axis codes.
  //
  // (12/04: The whole DD till needs work to function correctly in some
  // cases, such as transposed RA and Dec...).
  
  WorldCanvas *wc = wch.worldCanvas();
  String xAxis = "xaxiscode (required match)",
         yAxis = "yaxiscode (required match)";
  String xcode, ycode;
  
  return csConformed_  =
	  wc->getAttributeValue(xAxis, xcode) && xcode.contains("Direction")
       && wc->getAttributeValue(yAxis, ycode) && ycode.contains("Direction");
}

 
Bool SkyCatOverlayDD::determineDirectionColumnMapping() {
  if (!table()) {
    return False;
  }

  itsColumnNames = table()->tableDesc().columnNames();
  if (itsColumnNames.nelements() < 3) {
    return False;
  }

  // logic is case insensitive, and is as follows:
  // - first column called 'long*', or if not present, first column having
  //   units which map to degrees or hours is longitude column
  // - first column called 'lat*', or if not present, first column *beyond
  //   latitude column* having units which map to degrees is latitude column.

  itsLongitudeColumn = "";
  itsLatitudeColumn = "";
  itsDirectionTypeColumn ="";

  Regex rxLong("^[lL][oO][nN][gG].*");
  Regex rxLat("^[lL][aA][tT].*");
  Regex rxDir("^[tT][yY][pP][eE].*");

  // first attempt basic column name pattern matching
  uInt ncols = itsColumnNames.nelements();
  uInt i;
  for (i = 0; (i < ncols) && (itsLongitudeColumn == ""); i++) {
    if (itsColumnNames(i).matches(rxLong)) {
      itsLongitudeColumn = itsColumnNames(i);
    }
  }
  for (i = 0; (i < ncols) && (itsLatitudeColumn == ""); i++) {
    if (itsColumnNames(i).matches(rxLat)) {
      itsLatitudeColumn = itsColumnNames(i);
    }
  }
  for (i = 0; (i < ncols) && (itsDirectionTypeColumn == ""); i++) {
    if (itsColumnNames(i).matches(rxDir)) {
       itsDirectionTypeColumn = itsColumnNames(i);
    }
  }


  // and return if we at least determined long,lat and type columns
  if ((itsLongitudeColumn != "") && (itsLatitudeColumn != "") &&
      (itsDirectionTypeColumn !="")) {
    return True;
  }

  // ok, we have more work to do.
  Unit degUnit("deg");
  Unit hUnit("h");
  Int storeIndex = -1;

  if (itsLongitudeColumn == "") {
    // need to find first column with UNIT keyword dimensionally
    // equivalent to degrees or hours
    for (i = 0; (i < ncols) && (itsLongitudeColumn == ""); i++) {
      Unit tunit = columnUnit(itsColumnNames(i));
      if ((tunit == degUnit) || (tunit == hUnit)) {
	itsLongitudeColumn = itsColumnNames(i);
	storeIndex = Int(i);
      }
    }
  }
  if (itsLongitudeColumn == "") {
    // haven't found anything suitable for longitude, so cannot go further.
    return False;
  }

  if (itsLatitudeColumn == "") {
    // need to find first column *beyond long column* with UNIT keyword
    // dimensionally equivalent to degrees
    for (i = storeIndex; (i < ncols) && (itsLatitudeColumn == ""); i++) {
      Unit tunit = columnUnit(itsColumnNames(i));
      if (tunit == degUnit) {
	itsLatitudeColumn = itsColumnNames(i);
      }

    }
  }
  if (itsLatitudeColumn == "") {
    // haven't found anything suitable for latitude, so cannot go further.
    return False;
  }

  // we made it, so return True!
  return True;
}

Unit SkyCatOverlayDD::columnUnit(const String columnName) const {
  static Regex rxUnit("^[uU][nN][iI][tT]$");
  String value;
  if (getColumnKeyword(value, columnName, rxUnit)) {
    return Unit(value);
  } else {
    return Unit("_");
  }
}

void SkyCatOverlayDD::constructParameters() {
  Vector<String> vstring;
  Vector<String> vstring2;
  vstring2 = getColumnNamesOfType(TpString);
  uInt k=0;
  for (uInt i=0; i< vstring2.nelements(); i++) {
    // exclude direction type column since it is not useful
    if (vstring2(i) != itsDirectionTypeColumn) {
      vstring.resize(k+1,True);
      vstring(k) = vstring2(i);
      k++;
    }
  }
  vstring.resize(vstring.nelements() + 1, True);
  vstring(vstring.nelements() - 1) = "<none>";
  itsParamNameColumn
    = new DParameterChoice("namecolumn", "Column listing names", 
			   "", vstring, vstring(vstring.nelements() - 1),
			   vstring(vstring.nelements() - 1),
			   "Label_properties");
  vstring.resize();
  itsParamLineWidth
    = new DParameterRange<Int>("labellinewidth", "Line width",
			       "", 1, 5, 1, 1, 1,
			       "Label_properties");
  
  Vector<String> markerNames(Display::nMarkers);
  Vector<Int> markerValues(Display::nMarkers);
  for (Int i=0; i<Display::nMarkers;i++) {
    ostringstream os;
    markerValues(i) = i;
    Display::Marker m = static_cast<Display::Marker>(i);
    os << m;    
    markerNames(i) = String(os);
  }
  itsParamMarkerType 
    = new DParameterMapKeyChoice("markertype", "Marker type",
				"Select the symbol", markerNames, 
				markerValues, markerNames(0), markerNames(0),
				"Marker_properties");
  itsParamMarkerSize
    = new DParameterRange<Int>("markersize", "Marker size",
			       "Marker size in screen pixel", 0, 20, 1, 5, 5,
			       "Marker_properties");

  itsParamMarkerColor
    = new DParameterColorChoice("markercolor", "Marker color",
			   "", "Marker_properties");

  // gather columns of type Int, Float and Double for mapped markers
  Vector<String> vflt= getColumnNamesOfType(TpFloat);;
  Vector<String> vstring4;
  if ( vflt.nelements() > 0 ) {
    k=0;
    for (uInt i=0; i< vflt.nelements(); i++) {
      // exclude direction type column since it is not useful
      if (vflt(i) != itsLongitudeColumn && 
	  vflt(i) != itsLatitudeColumn) {
	vstring4.resize(k+1,True);
	vstring4(k) = vflt(i);
	k++;
      }
    }
  }
  Vector<String> vdbl(getColumnNamesOfType(TpDouble));
  if (vdbl.nelements()  > 0) { 
    for (uInt i=0; i< vdbl.nelements(); i++) {
      // exclude direction type column since it is not useful
      if (vdbl(i) != itsLongitudeColumn && 
	  vdbl(i) != itsLatitudeColumn) {
	vstring4.resize(k+1,True);
	vstring4(k) = vdbl(i);
	k++;
      }
    }

  }

  Vector<String> vint = getColumnNamesOfType(TpInt);
  if (vint.nelements()  > 0) { 
    for (uInt i=0; i< vint.nelements(); i++) {
      vstring4.resize(k+1,True);
      vstring4(k) = vdbl(i);
      k++;
    }
  }
  vstring4.resize(vstring4.nelements() + 1, True);
  vstring4(vstring4.nelements() - 1) = "<none>";

  itsParamMapColumn
    = new DParameterChoice("mapcolumn", "Marker value column", 
			   "Select a column if you want to map the"
			   " marker size to its's values", 
			   vstring4, vstring4(vstring4.nelements() - 1),
			   vstring4(vstring4.nelements() - 1),
			   "Marker_properties");

  vstring.resize(4);
  vstring(0) = "normal";
  vstring(1) = "roman";
  vstring(2) = "italic";
  vstring(3) = "script";

  itsParamCharacterFont 
    = new DParameterChoice("labelcharfont", "Character font",
                          "", vstring, vstring(0), vstring(0), 
                          "Label_properties");


  vstring.resize();
  itsParamCharacterSize 
    = new DParameterRange<Float>("labelcharsize", "Character size",
			       "Select the font size for text",
			       0.0, 4.0, 0.05, 0.8, 0.8,
			       "Label_properties");
  itsParamCharacterColor
    = new DParameterColorChoice("labelcharcolor", "Character color",
				"", "Label_properties");
  itsParamCharacterAngle
    = new DParameterRange<Int>("labelcharangle", "Label angle",
			       "", 0, 359, 1, 0, 0, 
			       "Label_properties");
  itsParamLabelXOffset
    = new DParameterRange<Float>("labelxoffset", "Label X offset (char units)",
				 "", -2.0, 2.0, 0.05, 0.0, 0.0,
				 "Label_properties");
  itsParamLabelYOffset
    = new DParameterRange<Float>("labelyoffset", "Label Y offset (char units)",
				 "", -2.0, 2.0, 0.05, 0.0, 0.0,
				 "Label_properties");

}

void SkyCatOverlayDD::destructParameters() {
  delete itsParamNameColumn;
  delete itsParamLineWidth;
  delete itsParamMarkerType;
  delete itsParamMarkerSize;
  delete itsParamMarkerColor;
  delete itsParamMapColumn;  
  delete itsParamCharacterFont;
  delete itsParamCharacterSize;
  delete itsParamCharacterColor;
  delete itsParamCharacterAngle;
  delete itsParamLabelXOffset;
  delete itsParamLabelYOffset;
}

} //# NAMESPACE CASA - END

