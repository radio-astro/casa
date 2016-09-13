//# TblAsXYDM.cc:  Display Method for xy displays of data from tables
//# Copyright (C) 2000,2001,2002
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
#include <tables/TaQL/TableParse.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <display/Display/Attribute.h>
#include <casa/Utilities/DataType.h>
#include <casa/Arrays/ArrayIter.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <casa/Arrays/ArrayMath.h>
#include <display/DisplayDatas/TblAsXYDD.h>
#include <display/DisplayDatas/TblAsXYDM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// constructor
	TblAsXYDM::TblAsXYDM(WorldCanvas *worldCanvas,
	                     AttributeBuffer *wchAttributes,
	                     AttributeBuffer *ddAttributes,
	                     CachingDisplayData *dd) :
		CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
	}

// destructor
	TblAsXYDM::~TblAsXYDM() {
		cleanup();
	}

// cleanup function
	void TblAsXYDM::cleanup() {
	}

	Bool TblAsXYDM::drawIntoList(Display::RefreshReason /*reason*/,
	                             WorldCanvasHolder &wcHolder) {

		// which world canvas do we draw on?
		WorldCanvas *wc = wcHolder.worldCanvas();

		// can we do a dynamic cast to the correct data type?
		TblAsXYDD *parent = dynamic_cast<TblAsXYDD *>
		                    (parentDisplayData());
		if (!parent) {
			throw(AipsError("invalid parent of TblAsXYDM"));
		}

		// get the table column data type
		TableDesc tdesc(parent->table()->tableDesc());
		DataType type =
		    tdesc.columnDesc(parent->itsXColumnName->value()).trueDataType();

		// array to contain data to be plotted
		Array<Float> xData,yData;
		Table *theTable = parent->table();

		cout << "Getting the data to plot" << endl;

		// get the x column data
		if (type == TpDouble) {
			Vector<double> typedata;
			// read the column into an array
			ROScalarColumn<double>
			dataCol(*theTable,parent->itsXColumnName->value());
			dataCol.getColumn(typedata,True);
			// now convert array to type double
			xData.resize(typedata.shape());
			convertArray(xData,typedata);
		}
		if (type == TpArrayFloat) {
			Vector<float> typedata;
			ROScalarColumn<float>
			dataCol(*theTable,parent->itsXColumnName->value());
			dataCol.getColumn(typedata,True);
			xData.resize(typedata.shape());
			xData=typedata;
		}
		if (type == TpArrayUShort) {
			Vector<ushort> typedata;
			ROScalarColumn<ushort>
			dataCol(*theTable,parent->itsXColumnName->value());
			dataCol.getColumn(typedata,True);
			xData.resize(typedata.shape());
			convertArray(xData,typedata);
		}
		if (type == TpArrayInt) {
			Vector<int> typedata;
			ROScalarColumn<int>
			dataCol(*theTable,parent->itsXColumnName->value());
			dataCol.getColumn(typedata,True);
			xData.resize(typedata.shape());
			convertArray(xData,typedata);
		}
		if (type == TpArrayUInt) {
			Vector<uInt> typedata;
			ROScalarColumn<uInt>
			dataCol(*theTable,parent->itsXColumnName->value());
			dataCol.getColumn(typedata,True);
			xData.resize(typedata.shape());
			convertArray(xData,typedata);
		}

		cout << "getting the y column data" << endl;

		// get the y column data
		if (parent->itsYColumnName->value()=="<row>") {
			cout << "y column is a row value" << endl;
			yData.resize(xData.shape());
			indgen(yData); // set yData(k)=k
		} else {
			if (type == TpDouble) {
				Vector<double> typedata;
				// read the column into an array
				ROScalarColumn<double>
				dataCol(*theTable,parent->itsYColumnName->value());
				dataCol.getColumn(typedata,True);
				// now convert array to type double
				yData.resize(typedata.shape());
				convertArray(yData,typedata);
			}
			if (type == TpArrayFloat) {
				Vector<float> typedata;
				ROScalarColumn<float>
				dataCol(*theTable,parent->itsYColumnName->value());
				dataCol.getColumn(typedata,True);
				yData.resize(typedata.shape());
				yData=typedata;
			}
			if (type == TpArrayUShort) {
				Vector<ushort> typedata;
				ROScalarColumn<ushort>
				dataCol(*theTable,parent->itsYColumnName->value());
				dataCol.getColumn(typedata,True);
				yData.resize(typedata.shape());
				convertArray(yData,typedata);
			}
			if (type == TpArrayInt) {
				Vector<int> typedata;
				ROScalarColumn<int>
				dataCol(*theTable,parent->itsYColumnName->value());
				dataCol.getColumn(typedata,True);
				yData.resize(typedata.shape());
				convertArray(yData,typedata);
			}
			if (type == TpArrayUInt) {
				Vector<uInt> typedata;
				ROScalarColumn<uInt>
				dataCol(*theTable,parent->itsYColumnName->value());
				dataCol.getColumn(typedata,True);
				yData.resize(typedata.shape());
				convertArray(yData,typedata);
			}
		}

		cout << xData << endl;
		cout << yData << endl;
		cout << parent->itsXColumnName->value() << "  " <<
		     parent->itsYColumnName->value();
		cout << endl;

		// define several things for drawing
		// Bool usePixelEdges = False;  // don't use this yet

		// now plot the data
		wc->drawMarkers(xData, yData);
		return True;
	}

// (required) default constructor
	TblAsXYDM::TblAsXYDM() {
	}

// (required) copy constructor
	TblAsXYDM::TblAsXYDM(const TblAsXYDM & other) :
		CachingDisplayMethod(other) {
	}

// (required) copy assignment
	void TblAsXYDM::operator=(const TblAsXYDM &) {
	}


} //# NAMESPACE CASA - END

