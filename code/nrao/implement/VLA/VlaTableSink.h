//# VlatableSink.h:
//# Copyright (C) 1997,1999
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
//#
//# $Id$

#ifndef NRAO_VLATABLESINK_H
#define NRAO_VLATABLESINK_H

#include <nrao/VLA/VlaSink.h>
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRow.h>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
namespace casa { //# NAMESPACE CASA - BEGIN
class String;
class TableDesc;
class TableRecord;
} //# NAMESPACE CASA - END

struct ADAFlags;         
struct ContinuumCDAFlags;
struct RCAFlags;
struct SDAFlags;
struct SpectralLineCDAFlags;

// <summary>
// VLA Table Sink
// </summary>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <templating arg=T>
//    <li>
//    <li>
// </templating>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>


class VlaTableSink :public VlaSink {
public:
  VlaTableSink();
  VlaTableSink(const String& a, const String& b);
  ~VlaTableSink();
  //
  Int writeVisibilityRecord(const Char* buffer);
  
private :
  //# Private and inaccessible
  VlaTableSink(const VlaTableSink& other);
  VlaTableSink& operator=(const VlaTableSink& other);

  // Some table stuff
  void           createTable(const String& tableName);
  void           addColumns(TableDesc& td, const RCAFlags* flags);
  void           addColumns(TableDesc& td, const SDAFlags* flags);
  void           addColumns(TableDesc& td, const ADAFlags* flags);
  void           addColumns(TableDesc& td, const ContinuumCDAFlags* flags);
  void           addColumns(TableDesc& td, const SpectralLineCDAFlags* flags);
  //
  void           writeTable(const RCAFlags* flags);
  void           writeTable(const SDAFlags* flags);
  void           writeTable(const ADAFlags* flags);
  void           writeTable(const ContinuumCDAFlags* flags);
  void           writeTable(const SpectralLineCDAFlags* flags);
  //
  Table          data;
  TableRow       dataRow;
  Vector<String> rcaNames;
  Vector<String> sdaNames;
  Vector<String> adaNames;
  Vector<String> cdaNames;
  Int            rowCount;
};
#endif
