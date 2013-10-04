//# TblAsRasterDDTemplates.cc: member templates for TblAsRasterDD
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

#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Utilities/DataType.h>
#include <casa/Utilities/Regex.h>
#include <display/DisplayDatas/TblAsRasterDD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T>
	Bool TblAsRasterDD::getTableKeyword(T &value, const String keyword) const {
		TableRecord trec(itsTable->keywordSet());
		if (!trec.isDefined(keyword)) {
			return False;
		}
		DataType dtype = whatType(&value);
		if (dtype != trec.dataType(keyword)) {
			return False;
		}
		try {
			trec.get(keyword, value);
		} catch (const AipsError &x) {
			if (&x) {
				return False;
			}
		}
		return True;
	}

	template <class T>
	Bool TblAsRasterDD::getTableKeyword(T &value, const Regex &regex) const {
		TableRecord trec(itsTable->keywordSet());
		for (uInt j = 0; j < trec.nfields(); j++) {
			if (trec.name(j).matches(regex)) {
				return getTableKeyword(value, trec.name(j));
			}
		}
		return False;
	}

	template <class T>
	Bool TblAsRasterDD::getColumnKeyword(T &value, const String column,
	                                     const String keyword) const {
		TableDesc tdesc(itsTable->tableDesc());
		TableRecord trec(tdesc.columnDesc(column).keywordSet());
		if (!trec.isDefined(keyword)) {
			return False;
		}
		DataType dtype = whatType(&value);
		if (dtype != trec.dataType(keyword)) {
			return False;
		}
		try {
			trec.get(keyword, value);
		} catch (const AipsError &x) {
			if (&x) {
				return False;
			}
		}
		return True;
	}

	template <class T>
	Bool TblAsRasterDD::getColumnKeyword(T &value, const String column,
	                                     const Regex &regex) const {
		TableDesc tdesc(itsTable->tableDesc());
		TableRecord trec(tdesc.columnDesc(column).keywordSet());
		for (uInt j = 0; j < trec.nfields(); j++) {
			if (trec.name(j).matches(regex)) {
				return getColumnKeyword(value, column, trec.name(j));
			}
		}
		return False;
	}


} //# NAMESPACE CASA - END

