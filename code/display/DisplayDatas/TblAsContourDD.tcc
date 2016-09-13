//# TblAsContourDD.cc:  Display Data for contour displays of data from a table
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
#include <casa/Utilities/Regex.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/TaQL/TableParse.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Containers/Record.h>
#include <display/Display/Attribute.h>
#include <casa/Utilities/DataType.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <display/DisplayDatas/TblAsContourDD.h>
#include <display/DisplayDatas/TblAsContourDM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// constructors
// given an already constructed table
	TblAsContourDD::TblAsContourDD(Table *table):
		// NEED TO CLONE/COPY THE TABLE AND PUT THE CLONE IN ITSTABLE
		// SO THAT NO ONE ELSE CAN REMOVE THE TABLE ON US
		itsTable(table),
		itsQueryTable(0),
		itsXColumnName(0),
		itsYColumnName(0),
		itsMColumnName(0),
		itsMColumnSet(0),
		itsLevels(0),
		itsScale(0),
		itsLine(0),
		itsDash(True),
		itsColor("foreground"),
		itsType("frac") {
		// get the names of the columns from the table
		getTableColumnNames();
		// construct the parameter set for axis (column) selection
		constructParameters();
		// install the default option values
		installDefaultOptions();
		// setup the coordinate system
		getCoordinateSystem();
		ActiveCaching2dDD::setCoordinateSystem( itsCoord, itsLinblc, itsLintrc);
	}

// given a string which gives the full pathname and filename of a table
// on disk
	TblAsContourDD::TblAsContourDD(const String tablename):
		itsTable(0),
		itsQueryTable(0),
		itsXColumnName(0),
		itsYColumnName(0),
		itsMColumnName(0),
		itsMColumnSet(0),
		itsLevels(0),
		itsScale(0),
		itsLine(0),
		itsDash(True),
		itsColor("foreground"),
		itsType("frac") {
		// open the table file - throw and error if there is a problem
		itsTable = new Table(tablename);
		if (!itsTable) {
			throw(AipsError("Cannot open named table"));
		}
		// get the names of the columns from the table
		getTableColumnNames();
		// construct the parameter set for axis (column) selection
		constructParameters();
		// install the default option values
		installDefaultOptions();
		// setup the coordinate system
		getCoordinateSystem();
		ActiveCaching2dDD::setCoordinateSystem( itsCoord, itsLinblc, itsLintrc);
	}

//destuctor
	TblAsContourDD::~TblAsContourDD() {
		destructParameters();
		if (itsQueryTable) {
			itsQueryTable->markForDelete();
			delete itsQueryTable;
		}
		if (itsTable) {
			delete itsTable;
		}
	}

	const Unit TblAsContourDD::dataUnit() {
		String value = "_";
		return value;
	}

// get the units of the columns being displayed
	const Unit TblAsContourDD::dataUnit(const String column) {
		static Regex rxUnit("^[uU][nN][iI][tT]$");
		String value;
		if (getColumnKeyword(value, column, rxUnit)) {
		} else {
			value = "_";
		}
		return value;
	}

// set the default options for the display data
	void TblAsContourDD::setDefaultOptions() {
		ActiveCaching2dDD::setDefaultOptions();
		installDefaultOptions();
		getCoordinateSystem();
		setCoordinateSystem();

	}

// set a record
	Bool TblAsContourDD::setOptions(Record &rec, Record &recOut) {
		Bool ret = ActiveCaching2dDD::setOptions(rec,recOut);
		Bool localchange = False, coordchange = False, error;

		if (readOptionRecord(itsOptQueryString, itsOptQueryStringUnset,
		                     error, rec, "querystring")) {

			arrangeQueryTable();
			localchange = True;
		}

		// set options for apperance of contours
		localchange = readOptionRecord(itsScale, error, rec, "scale") ||
		              localchange;
		localchange = readOptionRecord(itsType, error, rec, "type") ||
		              localchange;
		localchange = readOptionRecord(itsLine, error, rec, "line") ||
		              localchange;
		localchange = readOptionRecord(itsDash, error, rec, "dash") ||
		              localchange;
		localchange = readOptionRecord(itsColor, error, rec, "color") ||
		              localchange;

		// set options for contour levels
		Vector<Float> newLevels;
		if (rec.isDefined("levels")) {
			rec.get("levels",newLevels);
			Bool diff = (newLevels.nelements() != itsLevels.nelements());
			if (!diff) {
				for (uInt i = 0; i < newLevels.nelements(); i++) {
					diff = (newLevels(i) != itsLevels(i));
					if (diff) break;
				}
			}
			if (diff) {
				itsLevels.resize(newLevels.nelements());
				for (uInt i = 0; i < newLevels.nelements(); i++) {
					itsLevels(i) = newLevels(i);
				}
				ret = True;
			}
		} else {
			// throw(AipsError("no levels option set in TblAsContourDD"));
		}


		// set the DParmeter values which have information on
		// the axis (columns) used
		coordchange = (itsXColumnName->fromRecord(rec) || coordchange);
		coordchange = (itsYColumnName->fromRecord(rec) || coordchange);
		coordchange = (itsMColumnName->fromRecord(rec) || coordchange);
		coordchange = (itsMColumnSet->fromRecord(rec)  || coordchange);

		// if the axis (columns of table) are changed then we need to
		// update the coordinate system
		if (coordchange) {
			getCoordinateSystem();
			setCoordinateSystem();
		}

		return (ret || localchange || coordchange);
	}

	Record TblAsContourDD::getOptions( bool scrub ) const {
		Record rec = ActiveCaching2dDD::getOptions(scrub);

		Record querystring;
		querystring.define("dlformat", "querystring");
		querystring.define("listname", "\"WHERE\" query");
		querystring.define("ptype", "string");
		querystring.defineRecord("default", unset());
		if (itsOptQueryStringUnset) {
			querystring.defineRecord("value", unset());
		} else {
			querystring.define("value", itsOptQueryString);
		}
		querystring.define("allowunset", True);
		rec.defineRecord("querystring", querystring);

		// record for contour levels
		Record levels;
		levels.define("dlformat", "levels");
		levels.define("listname", "Contour levels");
		levels.define("ptype", "array");
		Vector<Float> vlevels(11);
		vlevels(0) = 0.0;
		vlevels(1) = 1.0;
		vlevels(2) = 2.0;
		vlevels(3) = 3.0;
		vlevels(4) = 4.0;
		vlevels(5) = 5.0;
		vlevels(6) = 6.0;
		vlevels(7) = 7.0;
		vlevels(8) = 8.0;
		vlevels(9) = 9.0;
		vlevels(10) = 10.0;
		levels.define("default", vlevels);
		levels.define("value", itsLevels);
		levels.define("allowunset", False);
		rec.defineRecord("levels", levels);

		// record for the scaling of the contour levels
		Record scale;
		Float vscale=0.1;
		scale.define("dlformat", "scale");
		scale.define("listname", "Contour scale factor");
		scale.define("ptype", "scalar");
		scale.define("default", vscale);
		scale.define("value", itsScale);
		scale.define("allowunset", False);
		rec.defineRecord("scale", scale);

		// record for the contour level type: absolute or (fractal?)
		Record type;
		type.define("dlformat", "type");
		type.define("listname", "Level type");
		type.define("ptype", "choice");
		Vector<String> vtype(2);
		vtype(0) = "frac";
		vtype(1) = "abs";
		type.define("popt", vtype);
		type.define("default", "frac");
		type.define("value", itsType);
		type.define("allowunset", False);
		rec.defineRecord("type", type);

		// record for contour line parameters
		Record line;
		line.define("dlformat", "line");
		line.define("listname", "Line width");
		line.define("ptype", "floatrange");
		line.define("pmin", Float(0.0));
		line.define("pmax", Float(5.0));
		line.define("presolution", Float(0.1));
		line.define("default", Float(0.5));
		line.define("value", itsLine);
		line.define("allowunset", False);
		rec.defineRecord("line", line);

		// record for type of contour line for negative values: dash or solid
		Record dash;
		dash.define("dlformat", "dash");
		dash.define("listname", "Dash negative contours?");
		dash.define("ptype", "boolean");
		dash.define("default", Bool(True));
		dash.define("value", itsDash);
		dash.define("allowunset", False);
		rec.defineRecord("dash", dash);

		// record for color of contour lines
		Record color;
		color.define("dlformat", "color");
		color.define("listname", "Line color");
		color.define("ptype", "userchoice");
		Vector<String> vcolor(8);
		vcolor(0) = "foreground";
		vcolor(1) = "background";
		vcolor(2) = "black";
		vcolor(3) = "white";
		vcolor(4) = "red";
		vcolor(5) = "green";
		vcolor(6) = "blue";
		vcolor(7) = "yellow";
		color.define("popt", vcolor);
		color.define("default", "foreground");
		color.define("value", itsColor);
		color.define("allowunset", False);
		rec.defineRecord("color", color);

		// get DParameter values which have information on the axis (columns) used
		itsXColumnName->toRecord(rec);
		itsYColumnName->toRecord(rec);
		itsMColumnName->toRecord(rec);
		itsMColumnSet->toRecord(rec);

		return rec;
	}



	CachingDisplayMethod *TblAsContourDD::newDisplayMethod(
	    WorldCanvas *worldCanvas,
	    AttributeBuffer *wchAttributes,
	    AttributeBuffer *ddAttributes,
	    CachingDisplayData *dd) {
		return new TblAsContourDM(worldCanvas,wchAttributes,ddAttributes,dd);
	}

//get the current options of the DD in a Attribute Buffer
	AttributeBuffer TblAsContourDD::optionsAsAttributes() {
		AttributeBuffer buffer = ActiveCaching2dDD::optionsAsAttributes();

		buffer.set("querystringunset", itsOptQueryStringUnset);
		buffer.set("querystring", itsOptQueryString);

		//now add DParmeter values
		buffer.set(itsXColumnName->name(), itsXColumnName->value());
		buffer.set(itsYColumnName->name(), itsYColumnName->value());
		buffer.set(itsMColumnName->name(), itsMColumnName->value());
		buffer.set(itsMColumnSet->name(),  itsMColumnSet->value() );

		return buffer;
	}

// obtain a pointer to the table to be displayed
	Table *TblAsContourDD::table() {
		if (itsQueryTable) {
			return itsQueryTable;
		} else {
			return itsTable;
		}
	}

	void TblAsContourDD::cleanup() {
	}

// (Required) default constructor
	TblAsContourDD::TblAsContourDD() :
		ActiveCaching2dDD() {
	}

// (Required) copy constructor
	TblAsContourDD::TblAsContourDD(const TblAsContourDD &) {
	}

// (Required) copy assignment
	void TblAsContourDD::operator=(const TblAsContourDD &) {
	}

	void TblAsContourDD::installDefaultOptions() {

		// setup values for query options
		itsOptQueryString = "";
		itsOptQueryStringUnset = True;
		arrangeQueryTable();

		// set contour level and apperance options
		itsLevels.resize(11);
		itsLevels(0) = 0.0;
		itsLevels(1) = 1.0;
		itsLevels(2) = 2.0;
		itsLevels(3) = 3.0;
		itsLevels(4) = 4.0;
		itsLevels(5) = 5.0;
		itsLevels(6) = 6.0;
		itsLevels(7) = 7.0;
		itsLevels(8) = 8.0;
		itsLevels(9) = 9.0;
		itsLevels(10) = 10.0;
		itsScale = 0.1;
		itsType = "frac";
		itsLine = 0.5;
		itsDash = True;
		itsColor = "foreground";


	}

	Bool TblAsContourDD::arrangeQueryTable() {
		// remove old version of query table and make ready for new entries
		if (itsQueryTable) {
			itsQueryTable->markForDelete();
			delete itsQueryTable;
		}
		itsQueryTable = 0;

		// now add to new query table if requested
		if (!itsOptQueryStringUnset) {
			String selectStr = "SELECT ";
			String fromStr = "FROM " + String(itsTable->tableName()) + String(" ");
			String whereStr = "WHERE " + itsOptQueryString;
			itsQueryTable = new Table(tableCommand(selectStr + fromStr + whereStr));
			if (itsQueryTable) {
				return True;
			}
		}
		// query table was not set
		return False;
	}

	void TblAsContourDD::getCoordinateSystem() {
		// NEED TO IMPELMENT Movie axis once changed from ActiveCaching2dDD to
		// ActiveCachingDD for n-dimensions


		// linear extent of coordinates
		Vector<Double> linblc(2), lintrc(2), extrema;
		extrema = columnStatistics(itsXColumnName->value());
		linblc(0)=extrema(0);
		lintrc(0)=extrema(1)-1.0;
		extrema = columnStatistics(itsYColumnName->value());
		linblc(1)=extrema(0);
		lintrc(1)=extrema(1)-1.0;

		// coordinate axis names
		Vector<String> names(2);
		names(0) = itsXColumnName->value();
		names(1) = itsYColumnName->value();

		// coordinate axis units
		Vector<String> units(2);
		Unit temp = dataUnit(itsXColumnName->value());
		units(0) = temp.getName();
		if (itsYColumnName->value() == "<row>") {  // row is not a table column
			units(1) = "_";
		} else {
			Unit temp2 = dataUnit(itsYColumnName->value());
			units(1)= temp2.getName();
		}

		Matrix<Double> pc(2,2);
		pc = 0.0;
		pc(0, 0) = pc(1, 1) = 1.0;

		// reference values for mapping for mapping coordinates
		Vector<double> refVal = linblc;

		// coordinate increments
		Vector<double> inc(2);
		inc = 1.0;

		// reference pixel for mapping coordinates
		Vector<double> refPix = linblc;

		LinearCoordinate lc(names, units, refVal, inc, pc, refPix);
		itsCoord.addCoordinate(lc);
		itsLinblc = linblc;
		itsLintrc = lintrc;

	}

	void TblAsContourDD::setCoordinateSystem() {
		ActiveCaching2dDD::setCoordinateSystem( itsCoord, itsLinblc, itsLintrc);
	}

	String TblAsContourDD::showValue(const Vector<Double> &world) {

		// NEED TO IMPLEMENT
		// no examples of this function exist in any other DD but it should be
		// easy to implement?
		String temp="";
		return temp;
	}

// get all of the table column names
	void TblAsContourDD::getTableColumnNames() {

		// make sure there is a table to be read
		if (!table()) {
			throw(AipsError("could not obtain table in TblAsContourDD"));
		}

		// determine the column names
		itsColumnNames = table()->tableDesc().columnNames();

		// check to make sure there are at least two column names
		if (itsColumnNames.nelements() < 2) {
			throw(AipsError("too few columns for TblAsContourDD to plot table"));
		}

	}

// get all of the table columnNames with a certain data type
	Vector<String> TblAsContourDD::getColumnNamesOfType( const Bool isarray) {

		uInt n = 0;

		// get all the table column names available
		// we must do this since a table query may be active
		getTableColumnNames();
		Vector<String> cnames = itsColumnNames;

		// get a description of the columns
		TableDesc tdesc(table()->tableDesc());

		// now keep only columns of specified data types
		Vector<String> retval (cnames.shape());
		for (uInt i = 0; i < cnames.nelements(); i++ ) {
			if (isarray ) {
				// columns with arrays suitable for x axis
				if (tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayUShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayInt ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayUInt ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayFloat ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayDouble ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayComplex ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpArrayDComplex ) {
					retval(n++) = cnames(i);
				}
			} else {
				// columns with scalars suitable for y axis
				if (tdesc.columnDesc(cnames(i)).trueDataType() == TpShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpUShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpInt ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpUInt ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpFloat ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpDouble ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpComplex ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == TpDComplex ) {
					retval(n++) = cnames(i);
				}
			}
		}

		// now resize the selected column names vector
		retval.resize(n, True);

		return retval;
	}


// construct the parameters list
	void TblAsContourDD::constructParameters() {

		Bool isarray = true, notarray = False;

		// get a list of column names with numerical data in arrays
		Vector<String> xstring = getColumnNamesOfType(isarray);

		// if no columns are returned then throw exception
		if (xstring.nelements() < 1) {
			throw(AipsError("no valid columns found in table for a contour plot"));
		}

		// get a list of column names with numerical data in non-arrays
		// ystring can have zero elements since we can plot against "row number"
		Vector<String> ystring = getColumnNamesOfType(notarray);

		// increase the size of the x column string and add the "none" option
		xstring.resize(xstring.nelements() + 1, True);
		xstring(xstring.nelements() - 1) = "<none>";

		// increase the size of the y column string and add the "rows" option
		ystring.resize(ystring.nelements() + 1, True);
		ystring(ystring.nelements() - 1) = "<row>";

		// now set up the X column choice parameters
		// we want the x axis to contain the vector data - this will
		// allow "time, etc." to go on the y axis (for now) to obtain
		// the types of plots people are used to having
		// select first valid table column as default
		itsXColumnName =
		    new DParameterChoice("xcolumn", "X Axis Column",
		                         "Selects table column to be plotted along the x axis"
		                         ,xstring, xstring(0), xstring(0),
		                         "Label_properties");

		// now set up the Y column choice parameters
		// currently restricted to non-array table columns
		// select row number as default
		itsYColumnName =
		    new DParameterChoice("ycolumn", "Y Axis Column",
		                         "Selects table column or `row number' for y axis",
		                         ystring, ystring(ystring.nelements() - 1),
		                         ystring(ystring.nelements() - 1),
		                         "Label_properties");

		// now set up the Movie column choice parameters
		// currently restricted to non-array table columns
		// select row number as default
		itsMColumnName =
		    new DParameterChoice("mcolumn", "Movie Axis Column",
		                         "Selects table column or `row number' for movie axis"
		                         ,ystring, ystring(ystring.nelements() - 1),
		                         ystring(ystring.nelements() - 1),
		                         "Label_properties");

		//now set up the Movie column selected choice parameters
		// default is off
		xstring.resize(2);
		xstring(0)="Off";
		xstring(1)="On";
		itsMColumnSet =
		    new DParameterChoice("mcolumnset", "Movie On/Off",
		                         "Selects table column or `row number' for movie axis"
		                         , xstring, xstring(0), xstring(0),
		                         "Label_properties");
	}

// destruct the parameters list
	void TblAsContourDD::destructParameters() {
		if (itsXColumnName) {
			delete itsXColumnName;
		}
		if (itsYColumnName) {
			delete itsYColumnName;
		}
		if (itsMColumnName) {
			delete itsMColumnName;
		}
		if (itsMColumnSet) {
			delete itsMColumnSet;
		}
	}

// this is a wrapper to read a table column
//
// for now we will assume that the table does not have information
// which tells how the "row" maps to world coordinates and we will
// just use the number of rows for the world coordinate
//
// we will also assume that the column is made up of a scalar value
// or a one dimensional array  -  we will need to extend this to
// n-dimensional arrays in the future
//
// we need to add support for complex values
//
	Vector<double> TblAsContourDD::columnStatistics(const String& columnName) {

		Vector<double> extrema(2);  // first value is minima second is maxima

		// for now the min is allows zero - until we can determine if a
		// table measures exists to tell us the world coordinate values
		extrema = 0.;

		// if column not selected then return
		if (columnName == "<none>") {
			return extrema;
		}

		// if column is selected as a row
		if (columnName == "<row>") {
			extrema(1) = table()->nrow();
			return extrema;
		}

		// get the table column data type
		TableDesc tdesc(table()->tableDesc());
		DataType type=tdesc.columnDesc(columnName).trueDataType();

		if (type == TpArrayDouble) {
			// array to contain data from column in columns data type
			Array<double> typedata;
			// read the column into an array
			ROArrayColumn<double> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			// let the maximum value be the number of elements in the array
			// typedata is ndim+row so we need to look at its shape the get
			// the number of pixels in a column row (nx)
			IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
			// for use in the future or TableAsXY
			// find the minimum and maximum in the array
			//minMax(extrema(0),extrema(1),data);
		}
		if (type == TpArrayFloat) {
			Array<float> typedata;
			ROArrayColumn<float> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			// let the maximum value be the number of elements - assume 1-D array
			IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
			// for use in the future or TableAsXY
			// now convert array to type double
			// Array<double> data;
			//data.resize(typedata.shape());
			//convertArray(data,typedata);
			// find the minimum and maximum in the array
			//minMax(extrema(0),extrema(1),data);
		}
		if (type == TpArrayShort) {
			Array<short> typedata;
			ROArrayColumn<short> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
		}
		if (type == TpArrayUShort) {
			Array<uShort> typedata;
			ROArrayColumn<uShort> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
		}
		if (type == TpArrayInt) {
			Array<int> typedata;
			ROArrayColumn<int> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
		}
		if (type == TpArrayUInt) {
			Array<uInt> typedata;
			ROArrayColumn<uInt> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
		}
		//
		// scalar column cases
		//
		if (type == TpDouble) {
			// array to contain data from column in columns data type
			Vector<double> typedata;
			// read the scalar column into an array
			ROScalarColumn<double> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),typedata);
		}
		if (type == TpFloat) {
			Vector<float> typedata;
			ROScalarColumn<float> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			Array<double> data;
			data.resize(typedata.shape());
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}
		if (type == TpShort) {
			Vector<short> typedata;
			ROScalarColumn<short> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			Array<double> data;
			data.resize(typedata.shape());
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}
		if (type == TpUShort) {
			Vector<uShort> typedata;
			ROScalarColumn<uShort> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			Array<double> data;
			data.resize(typedata.shape());
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}
		if (type == TpInt) {
			Vector<int> typedata;
			ROScalarColumn<int> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			Array<double> data;
			data.resize(typedata.shape());
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}
		if (type == TpUInt) {
			Vector<uInt> typedata;
			ROScalarColumn<uInt> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,True);
			Array<double> data;
			data.resize(typedata.shape());
			// have to change template file
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}


		return extrema;
	}

} //# NAMESPACE CASA - END

