//# TblAsRasterDD.cc:  Display casacore::Data for raster displays of data from a table
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
#include <display/DisplayDatas/TblAsRasterDD.h>
#include <display/DisplayDatas/TblAsRasterDM.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// constructors
// given an already constructed table
	TblAsRasterDD::TblAsRasterDD(casacore::Table *table):
		// NEED TO CLONE/COPY THE TABLE AND PUT THE CLONE IN ITSTABLE
		// SO THAT NO ONE ELSE CAN REMOVE THE TABLE ON US
		itsTable(table),
		itsQueryTable(0),
		itsXColumnName(0),
		itsYColumnName(0),
		itsMColumnName(0),
		itsMColumnSet(0) {
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
	TblAsRasterDD::TblAsRasterDD(const casacore::String tablename):
		itsTable(0),
		itsQueryTable(0),
		itsXColumnName(0),
		itsYColumnName(0),
		itsMColumnName(0),
		itsMColumnSet(0) {
		// open the table file - throw and error if there is a problem
		itsTable = new casacore::Table(tablename);
		if (!itsTable) {
			throw(casacore::AipsError("Cannot open named table"));
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
	TblAsRasterDD::~TblAsRasterDD() {
		destructParameters();
		if (itsQueryTable) {
			itsQueryTable->markForDelete();
			delete itsQueryTable;
		}
		if (itsTable) {
			delete itsTable;
		}
	}

	const casacore::Unit TblAsRasterDD::dataUnit() {
		casacore::String value = "_";
		return value;
	}

// get the units of the columns being displayed
	const casacore::Unit TblAsRasterDD::dataUnit(const casacore::String column) {
		static casacore::Regex rxUnit("^[uU][nN][iI][tT]$");
		casacore::String value;
		if (getColumnKeyword(value, column, rxUnit)) {
		} else {
			value = "_";
		}
		return value;
	}

// set the default options for the display data
	void TblAsRasterDD::setDefaultOptions() {
		ActiveCaching2dDD::setDefaultOptions();
		installDefaultOptions();
		getCoordinateSystem();
		setCoordinateSystem();
	}

// set a record
	casacore::Bool TblAsRasterDD::setOptions(casacore::Record &rec, casacore::Record &recOut) {
		casacore::Bool ret = ActiveCaching2dDD::setOptions(rec,recOut);
		casacore::Bool localchange = false, coordchange = false, error;

		if (readOptionRecord(itsOptQueryString, itsOptQueryStringUnset,
		                     error, rec, "querystring")) {

			arrangeQueryTable();
			localchange = true;
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

	casacore::Record TblAsRasterDD::getOptions( bool scrub ) const {
		casacore::Record rec = ActiveCaching2dDD::getOptions(scrub);

		casacore::Record querystring;
		querystring.define("dlformat", "querystring");
		querystring.define("listname", "\"WHERE\" query");
		querystring.define("ptype", "string");
		querystring.defineRecord("default", unset());
		if (itsOptQueryStringUnset) {
			querystring.defineRecord("value", unset());
		} else {
			querystring.define("value", itsOptQueryString);
		}
		querystring.define("allowunset", true);
		rec.defineRecord("querystring", querystring);

		// get DParameter values which have information on the axis (columns) used
		itsXColumnName->toRecord(rec);
		itsYColumnName->toRecord(rec);
		itsMColumnName->toRecord(rec);
		itsMColumnSet->toRecord(rec);

		return rec;
	}



	CachingDisplayMethod *TblAsRasterDD::newDisplayMethod(
	    WorldCanvas *worldCanvas,
	    AttributeBuffer *wchAttributes,
	    AttributeBuffer *ddAttributes,
	    CachingDisplayData *dd) {
		return new TblAsRasterDM(worldCanvas,wchAttributes,ddAttributes,dd);
	}

//get the current options of the DD in a Attribute Buffer
	AttributeBuffer TblAsRasterDD::optionsAsAttributes() {
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
	casacore::Table *TblAsRasterDD::table() {
		if (itsQueryTable) {
			return itsQueryTable;
		} else {
			return itsTable;
		}
	}

	void TblAsRasterDD::cleanup() {
	}

// (Required) default constructor
	TblAsRasterDD::TblAsRasterDD() :
		ActiveCaching2dDD() {
	}

// (Required) copy constructor
	TblAsRasterDD::TblAsRasterDD(const TblAsRasterDD &) {
	}

// (Required) copy assignment
	void TblAsRasterDD::operator=(const TblAsRasterDD &) {
	}

	void TblAsRasterDD::installDefaultOptions() {

		// setup values for query options
		itsOptQueryString = "";
		itsOptQueryStringUnset = true;
		arrangeQueryTable();

	}

	casacore::Bool TblAsRasterDD::arrangeQueryTable() {
		// remove old version of query table and make ready for new entries
		if (itsQueryTable) {
			itsQueryTable->markForDelete();
			delete itsQueryTable;
		}
		itsQueryTable = 0;

		// now add to new query table if requested
		if (!itsOptQueryStringUnset) {
			casacore::String selectStr = "SELECT ";
			casacore::String fromStr = "FROM " + casacore::String(itsTable->tableName()) + casacore::String(" ");
			casacore::String whereStr = "WHERE " + itsOptQueryString;
			itsQueryTable = new casacore::Table(tableCommand(selectStr + fromStr + whereStr));
			if (itsQueryTable) {
				return true;
			}
		}
		// query table was not set
		return false;
	}

	void TblAsRasterDD::getCoordinateSystem() {
		// NEED TO IMPELMENT Movie axis once changed from ActiveCaching2dDD to
		// ActiveCachingDD for n-dimensions


		// linear extent of coordinates
		casacore::Vector<casacore::Double> linblc(2), lintrc(2), extrema;
		extrema = columnStatistics(itsXColumnName->value());
		linblc(0)=extrema(0);
		lintrc(0)=extrema(1)-1.0;
		extrema = columnStatistics(itsYColumnName->value());
		linblc(1)=extrema(0);
		lintrc(1)=extrema(1)-1.0;

		// coordinate axis names
		casacore::Vector<casacore::String> names(2);
		names(0) = itsXColumnName->value();
		names(1) = itsYColumnName->value();

		// coordinate axis units
		casacore::Vector<casacore::String> units(2);
		casacore::Unit temp = dataUnit(itsXColumnName->value());
		units(0) = temp.getName();
		if (itsYColumnName->value() == "<row>") {  // row is not a table column
			units(1) = "_";
		} else {
			casacore::Unit temp2 = dataUnit(itsYColumnName->value());
			units(1)= temp2.getName();
		}

		casacore::Matrix<casacore::Double> pc(2,2);
		pc = 0.0;
		pc(0, 0) = pc(1, 1) = 1.0;

		// reference values for mapping for mapping coordinates
		casacore::Vector<double> refVal = linblc;

		// coordinate increments
		casacore::Vector<double> inc(2);
		inc = 1.0;

		// reference pixel for mapping coordinates
		casacore::Vector<double> refPix = linblc;

		casacore::LinearCoordinate lc(names, units, refVal, inc, pc, refPix);
		itsCoord.addCoordinate(lc);
		itsLinblc = linblc;
		itsLintrc = lintrc;

	}

	void TblAsRasterDD::setCoordinateSystem() {
		ActiveCaching2dDD::setCoordinateSystem( itsCoord, itsLinblc, itsLintrc);
	}

	casacore::String TblAsRasterDD::showValue(const casacore::Vector<casacore::Double> &world) {

		// NEED TO IMPLEMENT
		// no examples of this function exist in any other DD but it should be
		// easy to implement?
		casacore::String temp="";
		return temp;
	}

// get all of the table column names
	void TblAsRasterDD::getTableColumnNames() {

		// make sure there is a table to be read
		if (!table()) {
			throw(casacore::AipsError("could not obtain table in TblAsRasterDD"));
		}

		// determine the column names
		itsColumnNames = table()->tableDesc().columnNames();

		// check to make sure there are at least two column names
		if (itsColumnNames.nelements() < 2) {
			throw(casacore::AipsError("too few columns for TblAsRasterDD to plot table"));
		}

	}

// get all of the table columnNames with a certain data type
	casacore::Vector<casacore::String> TblAsRasterDD::getColumnNamesOfType( const casacore::Bool isarray) {

		casacore::uInt n = 0;

		// get all the table column names available
		// we must do this since a table query may be active
		getTableColumnNames();
		casacore::Vector<casacore::String> cnames = itsColumnNames;

		// get a description of the columns
		casacore::TableDesc tdesc(table()->tableDesc());

		// now keep only columns of specified data types
		casacore::Vector<casacore::String> retval (cnames.shape());
		for (casacore::uInt i = 0; i < cnames.nelements(); i++ ) {
			if (isarray ) {
				// columns with arrays suitable for x axis
				if (tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayUShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayInt ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayUInt ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayFloat ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayDouble ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayComplex ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpArrayDComplex ) {
					retval(n++) = cnames(i);
				}
			} else {
				// columns with scalars suitable for y axis
				if (tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpUShort ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpInt ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpUInt ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpFloat ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpDouble ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpComplex ||
				        tdesc.columnDesc(cnames(i)).trueDataType() == casacore::TpDComplex ) {
					retval(n++) = cnames(i);
				}
			}
		}

		// now resize the selected column names vector
		retval.resize(n, true);

		return retval;
	}


// construct the parameters list
	void TblAsRasterDD::constructParameters() {

		casacore::Bool isarray = true, notarray = false;

		// get a list of column names with numerical data in arrays
		casacore::Vector<casacore::String> xstring = getColumnNamesOfType(isarray);

		// if no columns are returned then throw exception
		if (xstring.nelements() < 1) {
			throw(casacore::AipsError("no valid columns found in table for a raster plot"));
		}

		// get a list of column names with numerical data in non-arrays
		// ystring can have zero elements since we can plot against "row number"
		casacore::Vector<casacore::String> ystring = getColumnNamesOfType(notarray);

		// increase the size of the x column string and add the "none" option
		xstring.resize(xstring.nelements() + 1, true);
		xstring(xstring.nelements() - 1) = "<none>";

		// increase the size of the y column string and add the "rows" option
		ystring.resize(ystring.nelements() + 1, true);
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
	void TblAsRasterDD::destructParameters() {
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
	casacore::Vector<double> TblAsRasterDD::columnStatistics(const casacore::String& columnName) {

		casacore::Vector<double> extrema(2);  // first value is minima second is maxima

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
		casacore::TableDesc tdesc(table()->tableDesc());
		casacore::DataType type=tdesc.columnDesc(columnName).trueDataType();

		if (type == casacore::TpArrayDouble) {
			// array to contain data from column in columns data type
			casacore::Array<double> typedata;
			// read the column into an array
			casacore::ROArrayColumn<double> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			// let the maximum value be the number of elements in the array
			// typedata is ndim+row so we need to look at its shape the get
			// the number of pixels in a column row (nx)
			casacore::IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
			// for use in the future or TableAsXY
			// find the minimum and maximum in the array
			//minMax(extrema(0),extrema(1),data);
		}
		if (type == casacore::TpArrayFloat) {
			casacore::Array<float> typedata;
			casacore::ROArrayColumn<float> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			// let the maximum value be the number of elements - assume 1-D array
			casacore::IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
			// for use in the future or TableAsXY
			// now convert array to type double
			// casacore::Array<double> data;
			//data.resize(typedata.shape());
			//convertArray(data,typedata);
			// find the minimum and maximum in the array
			//minMax(extrema(0),extrema(1),data);
		}
		if (type == casacore::TpArrayShort) {
			casacore::Array<short> typedata;
			casacore::ROArrayColumn<short> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
		}
		if (type == casacore::TpArrayUShort) {
			casacore::Array<casacore::uShort> typedata;
			casacore::ROArrayColumn<casacore::uShort> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
		}
		if (type == casacore::TpArrayInt) {
			casacore::Array<int> typedata;
			casacore::ROArrayColumn<int> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
		}
		if (type == casacore::TpArrayUInt) {
			casacore::Array<casacore::uInt> typedata;
			casacore::ROArrayColumn<casacore::uInt> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::IPosition length = typedata.shape();
			extrema(1) = length(0); // get the length of the first axis which is nx
		}
		//
		// scalar column cases
		//
		if (type == casacore::TpDouble) {
			// array to contain data from column in columns data type
			casacore::Vector<double> typedata;
			// read the scalar column into an array
			casacore::ROScalarColumn<double> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),typedata);
		}
		if (type == casacore::TpFloat) {
			casacore::Vector<float> typedata;
			casacore::ROScalarColumn<float> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::Array<double> data;
			data.resize(typedata.shape());
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}
		if (type == casacore::TpShort) {
			casacore::Vector<short> typedata;
			casacore::ROScalarColumn<short> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::Array<double> data;
			data.resize(typedata.shape());
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}
		if (type == casacore::TpUShort) {
			casacore::Vector<casacore::uShort> typedata;
			casacore::ROScalarColumn<casacore::uShort> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::Array<double> data;
			data.resize(typedata.shape());
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}
		if (type == casacore::TpInt) {
			casacore::Vector<int> typedata;
			casacore::ROScalarColumn<int> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::Array<double> data;
			data.resize(typedata.shape());
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}
		if (type == casacore::TpUInt) {
			casacore::Vector<casacore::uInt> typedata;
			casacore::ROScalarColumn<casacore::uInt> dataCol(*table(),columnName);
			dataCol.getColumn(typedata,true);
			casacore::Array<double> data;
			data.resize(typedata.shape());
			// have to change template file
			convertArray(data,typedata);
			// minima and maxima of data are world coordinate min and max
			minMax(extrema(0),extrema(1),data);
		}


		return extrema;
	}

} //# NAMESPACE CASA - END

