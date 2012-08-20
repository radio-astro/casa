//# SkyCatOverlayDM.cc: catalogue overlay drawing for SkyCatOverlayDD
//# Copyright (C) 1999,2000,2001,2002
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
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/Quantum.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableDesc.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/SkyCatOverlayDM.h>
#include <display/DisplayDatas/SkyCatOverlayDD.h>
#include <cpgplot.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SkyCatOverlayDM::SkyCatOverlayDM(WorldCanvas *worldCanvas, 
				     AttributeBuffer *wchAttributes,
				     AttributeBuffer *ddAttributes,
				     CachingDisplayData *dd) :
  CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
}

SkyCatOverlayDM::~SkyCatOverlayDM() {
  cleanup();
}


Bool SkyCatOverlayDM::drawIntoList(Display::RefreshReason reason,
				    WorldCanvasHolder &wcHolder) {
  // Locate the WorldCanvas to draw upon
  WorldCanvas *wc = wcHolder.worldCanvas();

  SkyCatOverlayDD *parent = dynamic_cast<SkyCatOverlayDD *>
    (parentDisplayData());
  if (!parent) {
    throw(AipsError("invalid parent of SkyCatOverlayDM"));
  }

  TableDesc tdesc = parent->table()->tableDesc();
  ColumnDesc cdesc = tdesc.columnDesc(parent->itsLongitudeColumn);
  DataType dtype = cdesc.dataType();
  Vector<Float> longVec, latVec;
  Vector<String> dirtypeVec;
  if (dtype == TpFloat) {
    ROScalarColumn<Float> longCol(*(parent->table()), 
				  parent->itsLongitudeColumn);
    longCol.getColumn(longVec, True);
  } else if (dtype == TpDouble) {
    ROScalarColumn<Double> longCol(*(parent->table()),
				   parent->itsLongitudeColumn);
    Vector<Double> dlongVec;
    longCol.getColumn(dlongVec, True);
    longVec.resize(dlongVec.shape());
    convertArray(longVec, dlongVec);
  }

  cdesc = tdesc.columnDesc(parent->itsLatitudeColumn);
  dtype = cdesc.dataType();
  if (dtype == TpFloat) {
    ROScalarColumn<Float> latCol(*(parent->table()), 
				  parent->itsLatitudeColumn);
    latCol.getColumn(latVec, True);
  } else if (dtype == TpDouble) {
    ROScalarColumn<Double> latCol(*(parent->table()),
				   parent->itsLatitudeColumn);
    Vector<Double> dlatVec;
    latCol.getColumn(dlatVec, True);
    latVec.resize(dlatVec.shape());
    convertArray(latVec, dlatVec);
  }

  if (latVec.nelements() != longVec.nelements()) {
    throw(AipsError("Non-conformant Long/Lat columns"));
  }

  Unit longUnit(parent->columnUnit(parent->itsLongitudeColumn));
  Unit latUnit(parent->columnUnit(parent->itsLatitudeColumn));

  cdesc = tdesc.columnDesc(parent->itsDirectionTypeColumn);
  ROScalarColumn<String> dirtypeCol(*(parent->table()), 
				  parent->itsDirectionTypeColumn);
  dirtypeCol.getColumn(dirtypeVec, True);
    

  String xaxiscode, yaxiscode;
  String xattString("xaxiscode (required match)");
  String yattString("yaxiscode (required match)");

  if (!wc->getAttributeValue(xattString, xaxiscode) ||
      !wc->getAttributeValue(yattString, yaxiscode)) {
    throw(AipsError("Unable to determine WorldCanvas axiscodes"));
  }	// (shouldn't happen anymore, since conformsTo[CS]() is now tested
  	// for direction axis codes before allowing draw() to be called..)
  
  // ASSUMPTION: axiscodes are the same for x and y

  String wcxaxisrefcode = "";
  String wcyaxisrefcode = "";  

  if (xaxiscode.contains("Direction") && yaxiscode.contains("Direction")) {
    String tmp = xaxiscode.after("Direction");
    wcxaxisrefcode = tmp.at(0,tmp.length()-1);
    tmp = yaxiscode.after("Direction");
    wcyaxisrefcode = tmp.at(0,tmp.length()-1);
  }
  if (wcxaxisrefcode == "" || wcyaxisrefcode == "") {
    throw(AipsError("Overlays only possible using direction (lon,lat) on WorldCanvas"));
  }

  MDirection::Types dirtype;
  if (!MDirection::getType(dirtype, wcxaxisrefcode) ||
      !MDirection::getType(dirtype, wcyaxisrefcode)) {
    throw(AipsError("Unable to parse Direction type"));
  }

  MDirection mdir;
  MDirection::Ref mref;
  MDirection::Ref min;
  mdir.giveMe(mref,wcxaxisrefcode);
  Vector<Float> long_rad,lat_rad;
  long_rad.resize(longVec.nelements());
  lat_rad.resize(latVec.nelements());

  // ASSUMPTION: assume that WorldCanvas units are radians - this 
  // will be fixed when WorldCanvas learns about CoordinateSystem.

  Quantity longQnt,latQnt;
  Unit wcUnit("rad");
  for (uInt i=0; i< longVec.nelements(); i++) {
    mdir.giveMe(min,dirtypeVec(i));
    longQnt = Quantity(longVec(i),longUnit);
    latQnt = Quantity(latVec(i),latUnit);
    if (wcxaxisrefcode != dirtypeVec(i) ) {
      mdir = MDirection::Convert(MDirection(longQnt,latQnt,min),
				 mref) ();
      long_rad(i) = (mdir.getAngle(wcUnit)).getValue()(0);
      lat_rad(i) = (mdir.getAngle(wcUnit)).getValue()(1);
    } else {
      long_rad(i) = longQnt.getValue(wcUnit);
      lat_rad(i) = latQnt.getValue(wcUnit);
    }
  }

  // finally draw the markers
  wc->setLineWidth(parent->lineWidth());
  wc->setColor(parent->markerColor());

  if (parent->mapColumn() != "<none>") {
    ROTableColumn mapCol(*(parent->table()),
			 parent->mapColumn());
    Vector<Float> mapVec(parent->table()->nrow());
    for (uInt i=0; i < mapVec.nelements(); ++i) {
      mapVec(i) = mapCol.asfloat(i);
    }
    wc->drawMappedMarkers(long_rad, lat_rad, mapVec, 1, 20,
			  parent->markerType());
  } else {    
    wc->drawMarkers(long_rad, lat_rad, parent->markerType(),
		    parent->markerSize());
  }
  if (parent->nameColumn() != "<none>") {
    ROScalarColumn<String> nameCol(*(parent->table()),
				   parent->nameColumn());
    Vector<String> nameVec;
    nameCol.getColumn(nameVec, True);
    wc->setColor(parent->charColor());
    cpgsch(parent->charSize());
    if (parent->charFont() == "roman") {
      cpgscf(2);
    } else if (parent->charFont() == "italic") {
      cpgscf(3); 
    } else if (parent->charFont() == "script") {
      cpgscf(4);
    } else {
      cpgscf(1);
    }
    wc->drawTextStrings(long_rad, lat_rad, nameVec, parent->charAngle(), 
                        parent->labelXOffset(), parent->labelYOffset());    
  }
  return True;
}

void SkyCatOverlayDM::cleanup() {
}

SkyCatOverlayDM::SkyCatOverlayDM() {
}

SkyCatOverlayDM::SkyCatOverlayDM(const SkyCatOverlayDM &other) :
  CachingDisplayMethod(other) {
}

void SkyCatOverlayDM::operator=(const SkyCatOverlayDM &) {
}


} //# NAMESPACE CASA - END

