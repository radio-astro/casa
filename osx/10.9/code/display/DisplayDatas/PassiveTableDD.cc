//# PassiveTableDD.cc: PassiveCachingDD class with Table-specific support
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

#include <casa/Exceptions/Error.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/TableParse.h>
#include <display/DisplayDatas/PassiveTableDD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	PassiveTableDD::PassiveTableDD(Table *table) :
		PassiveCachingDD(),
		itsTable(table),
		itsQueryTable(0) {
		installDefaultOptions();
	}

	PassiveTableDD::PassiveTableDD(const String tablename) :
		PassiveCachingDD(),
		itsTable(0),
		itsQueryTable(0) {
		itsTable = new Table(tablename);
		if (!itsTable) {
			throw(AipsError("Cannot open named table"));
		}
		installDefaultOptions();
	}

	PassiveTableDD::~PassiveTableDD() {
		if (itsQueryTable) {
			itsQueryTable->markForDelete();
			delete itsQueryTable;
		}
		if (itsTable) {
			delete itsTable;
		}
	}

	void PassiveTableDD::setDefaultOptions() {
		PassiveCachingDD::setDefaultOptions();
		installDefaultOptions();
	}

	Bool PassiveTableDD::setOptions(Record &rec, Record &recOut) {
		Bool ret = PassiveCachingDD::setOptions(rec, recOut);

		Bool localchange = False, error, unst;

		if (readOptionRecord(itsOptQueryString, unst,
		                     error, rec, "querystring")) {
			arrangeQueryTable();
			localchange = True;
		}

		return (ret || localchange);
	}

	Record PassiveTableDD::getOptions( bool scrub ) const {
		Record rec = PassiveCachingDD::getOptions(scrub);

		Record querystring;
		querystring.define("dlformat", "querystring");
		querystring.define("listname", "\"WHERE\" query");
		querystring.define("ptype", "string");
		querystring.defineRecord("default", unset());
		querystring.define("value", itsOptQueryString);
		querystring.define("allowunset", True);
		rec.defineRecord("querystring", querystring);

		return rec;
	}

	AttributeBuffer PassiveTableDD::optionsAsAttributes() {
		AttributeBuffer buffer = PassiveCachingDD::optionsAsAttributes();

		buffer.set("querystring", itsOptQueryString);

		return buffer;
	}

	Vector<String> PassiveTableDD::getColumnNamesOfType(const DataType type) {
		uInt n = 0;
		TableDesc tdesc(itsTable->tableDesc());
		Vector<String> cnames = tdesc.columnNames();
		Vector<String> retval(cnames.shape());
		for (uInt i = 0; i < cnames.nelements(); i++) {
			if (tdesc.columnDesc(cnames(i)).trueDataType() == type) {
				retval(n++) = cnames(i);
			}
		}
		retval.resize(n, True);
		return retval;
	}

	Table *PassiveTableDD::table() {
		if (itsQueryTable) {
			return itsQueryTable;
		} else {
			return itsTable;
		}
	}

// (Required) default constructor.
	PassiveTableDD::PassiveTableDD() :
		PassiveCachingDD() {
	}

// (Required) copy constructor.
	PassiveTableDD::PassiveTableDD(const PassiveTableDD &o) : PassiveCachingDD(o) {
	}

// (Required) copy assignment.
	void PassiveTableDD::operator=(const PassiveTableDD &) {
	}

	void PassiveTableDD::installDefaultOptions() {
		itsOptQueryString = "";
		arrangeQueryTable();
	}

	Bool PassiveTableDD::arrangeQueryTable() {
		if (itsQueryTable) {
			itsQueryTable->markForDelete();
			delete itsQueryTable;
		}
		itsQueryTable = 0;
		if (itsOptQueryString != "") {
			String selectStr = "SELECT ";
			String fromStr = "FROM " + String(itsTable->tableName()) + String(" ");
			String whereStr = "WHERE " + itsOptQueryString;

			try {

				itsQueryTable =
				    new Table(tableCommand(selectStr + fromStr + whereStr));
			}

			catch (const AipsError& err) {
				cerr<<"*** "<<err.getMesg()<<" ***"<<endl;
			} catch(...) {
				cerr<<"***Unknown query error***"<<endl;
			}

			if (itsQueryTable) {
				return True;
			}
		}
		return False;
	}


} //# NAMESPACE CASA - END

