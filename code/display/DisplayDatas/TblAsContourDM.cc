//# TblAsContourDM.cc:  Display Method for contour displays of data from tables
//# Copyright (C) 2000,2001
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
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/TableParse.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <display/Display/Attribute.h>
#include <casa/Utilities/DataType.h>
#include <tables/Tables/ArrayColumn.h> 
#include <casa/Arrays/ArrayMath.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/TblAsContourDD.h>
#include <display/DisplayDatas/TblAsContourDM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// constructor
TblAsContourDM::TblAsContourDM(WorldCanvas *worldCanvas, 
				 AttributeBuffer *wchAttributes,
				 AttributeBuffer *ddAttributes,
				 CachingDisplayData *dd) :
  CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
}
 
// destructor
TblAsContourDM::~TblAsContourDM() {
  cleanup();
}
 
// cleanup function
void TblAsContourDM::cleanup() {
}

Bool TblAsContourDM::drawIntoList(Display::RefreshReason reason,
				   WorldCanvasHolder &wcHolder) {

  // which world canvas do we draw on?
  WorldCanvas *wc = wcHolder.worldCanvas();
 
  // can we do a dynamic cast to the correct data type?
  TblAsContourDD *parent = dynamic_cast<TblAsContourDD *>
    (parentDisplayData());                                            
  if (!parent) {
    throw(AipsError("invalid parent of TblAsContourDM"));
  }                

  // get the table column data type
  TableDesc tdesc(parent->table()->tableDesc());
  DataType type = 
    tdesc.columnDesc(parent->itsXColumnName->value()).trueDataType();

  // array to contain data to be plotted
  Array<float> data;
  Table *theTable = parent->table();

  if (type == TpArrayDouble) {
    Array<double> typedata;
    // read the column into an array
    ROArrayColumn<double> 
      dataCol(*theTable,parent->itsXColumnName->value());
    dataCol.getColumn(typedata,True);
    // now convert array to type double
    data.resize(typedata.shape());
    convertArray(data,typedata);
  }
  if (type == TpArrayFloat) {
    ROArrayColumn<float> 
      dataCol(*theTable,parent->itsXColumnName->value());
    dataCol.getColumn(data,True);
  }
  if (type == TpArrayUShort) {
    Array<ushort> typedata;
    ROArrayColumn<ushort> 
      dataCol(*theTable,parent->itsXColumnName->value());
    dataCol.getColumn(typedata,True);
    data.resize(typedata.shape());
    convertArray(data,typedata);
  }
  if (type == TpArrayInt) {
    Array<int> typedata;
    ROArrayColumn<int> 
      dataCol(*theTable,parent->itsXColumnName->value());
    dataCol.getColumn(typedata,True);
    data.resize(typedata.shape());
    convertArray(data,typedata);
  }
  if (type == TpArrayUInt) {
    Array<uInt> typedata;
    ROArrayColumn<uInt> 
      dataCol(*theTable,parent->itsXColumnName->value());
    dataCol.getColumn(typedata,True);
    data.resize(typedata.shape());
    convertArray(data,typedata);
  }
  // put the data into the matrix
  const Matrix<float> theData = data;

  // define several things for drawing
  Bool usePixelEdges = True;  // Better than center (should be user choice) dk

  // find data min and max
  float dmin=0.,dmax=0.;
  minMax(dmin,dmax,data);

  // determine the contour levels to be used in the plot
  Vector<Float> levels(parent->itsLevels.nelements());
  for (uInt i = 0; i < levels.nelements(); i++) {
    if (parent->itsType == "abs") {
      levels(i) = parent->itsLevels(i) * parent->itsScale;
    } else {
      levels(i) = dmin + parent->itsLevels(i) * parent->itsScale *
        (dmax - dmin);
    }
  }

  // set properties for apperance of contour lines
  wc->setColor(parent->itsColor);
  wc->pixelCanvas()->setLineWidth(parent->itsLine);
  Attribute dashem("dashNegativeContours", Bool(parent->itsDash));
  wc->setAttribute(dashem);

  // now plot the data
  wc->drawContourMap(parent->itsLinblc, parent->itsLintrc, theData, 
			levels, usePixelEdges);
  return True;
}

// (required) default constructor
TblAsContourDM::TblAsContourDM(){
}

// (required) copy constructor
TblAsContourDM::TblAsContourDM(const TblAsContourDM & other) :
  CachingDisplayMethod(other) {
}

// (required) copy assignment
void TblAsContourDM::operator=(const TblAsContourDM &){
}


} //# NAMESPACE CASA - END

