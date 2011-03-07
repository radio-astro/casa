//# PassiveTableDD.h: PassiveCachingDD class with Table-specific support
//# Copyright (C) 2000
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

#ifndef TRIALDISPLAY_PASSIVETABLEDD_H
#define TRIALDISPLAY_PASSIVETABLEDD_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/DataType.h>
#include <display/DisplayDatas/PassiveCachingDD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Table;
class Regex;

// <summary>
// Class adding Table-specific support functions to PassiveCachingDD.
// </summary>

// <synopsis>
// This class adds support functions useful for dealing with Tables
// to the PassiveCachingDD class.
// </synopsis>

class PassiveTableDD : public PassiveCachingDD {

 public:

  // Constructor taking a pointer to an already constructed Table.
  // The Table will be deleted by this class.
  PassiveTableDD(Table *table);

  // Constructor taking a String which describes the full pathname
  // of a Table on disk.
  PassiveTableDD(const String tablename);

  // Destructor.
  virtual ~PassiveTableDD();

  // Install the default options for this DisplayData.
  virtual void setDefaultOptions();

  // Apply options stored in <src>rec</src> to the DisplayData.  A
  // return value of <src>True</src> means a refresh is needed.
  // <src>recOut</src> contains any fields which were implicitly 
  // changed as a result of the call to this function.
  virtual Bool setOptions(Record &rec, Record &recOut);

  // Retrieve the current and default options and parameter types.
  virtual Record getOptions();

  // Return the current options of this DisplayData as an
  // AttributeBuffer.
  virtual AttributeBuffer optionsAsAttributes();

 protected:

  // Get the value of the named keyword, or the first keyword matching
  // <src>regex</src>, and return it in <src>value</src>.  The return
  // value is <src>True</src> for success, and <src>False</src> for
  // failure, which is the result if the wrong type <src>T</src> is 
  // requested.
  // <group>
  template <class T> Bool getTableKeyword(T &value, 
					  const String keyword) const;
  template <class T> Bool getTableKeyword(T &value, const Regex &regex) const;
  // </group>

  // Get the value of the named keyword, or the first keyword matching
  // <src>regex</src> for the named column, and return it in
  // <src>value</src>. The return value is <src>True</src> for
  // success, and <src>False</src> for failure, which is the result if
  // the wrong type <src>T</src> is requested, or if the keyword
  // doesn't exist.
  // <group>
  template <class T> Bool getColumnKeyword(T &value, const String column,
					   const String keyword) const;
  template <class T> Bool getColumnKeyword(T &value, const String column,
					   const Regex &regex) const;
  // </group>

  // Get the names of the columns of the table having the given type.
  Vector<String> getColumnNamesOfType(const DataType type);

  // Return the Table pointer for use by derived classes.  If a query
  // is active (via <src>setOptions</src>), then the result of the
  // query is returned, otherwise the construction Table is returned.
  Table *table();

  // (Required) default constructor.
  PassiveTableDD();

  // (Required) copy constructor.
  PassiveTableDD(const PassiveTableDD &other);

  // (Required) copy assignment.
  void operator=(const PassiveTableDD &other);

 private:

  // Store the primary table here.
  Table *itsTable;

  // Store the result of a query on the table here.
  Table *itsQueryTable;

  // Option: what is the query string, or is it unset?
  String itsOptQueryString;
  Bool itsOptQueryStringUnset;

  // Install the default options for this DisplayData.
  void installDefaultOptions();

  // Arrange the query table (generally called after installing new
  // options).
  Bool arrangeQueryTable();

};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/DisplayDatas/PassiveTableDDTemplates.tcc>
#endif

#endif
