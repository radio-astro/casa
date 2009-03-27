//# FlagVersions.h: Maintain and manage different flag versions.
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003
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
//# You should have receied a copy of the GNU Library General Public License
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
//#
//# ----------------------------------------------------------------------------
//# Change Log
//# ----------------------------------------------------------------------------
//# Date         Name             Comments
//# 


#ifndef FLAGVERSION_H
#define FLAGVERSION_H

//# Includes

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/OS/Timer.h>
#include <casa/OS/File.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableColumn.h>

#include <tableplot/TablePlot/SLog.h>

//# FlagVersion Change Log
//# =======================
//# Date              Name      Description
//# April 20 2007    Urvashi R.V.       Created this class.
//# Aug 23 2007      Urvashi R.V.       Added Documentation.
//# Oct 29 2007      Shannon J.         Switched how msg logging was done.

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class to generate and manage flag versions for Tables containing flag columns. 
// </summary>

// <reviewed reviewer="" date="" tests="">
// </reviewed>

// <prerequisite>
//   <li> Table 
// </prerequisite>

// <etymology>
// Manages Flag Versions.
// </etymology>

// <synopsis>
// This class creates and manages flag versions. A flag version as defined in this
// class has the following structure.
//
//    xxxx.ms
//    xxxx.ms.flagversions
//
// When first opened, a flagversions directory is created parallel to the original Table.
// This directory contains a text file : FLAG_VERSION_LIST that holds a list of string pairs
//      version name  :  comment for this version.
//
// The FLAG and FLAG_ROW columns of the main table are copied into new Tables with only two
// columns, whenever a new flag version is created.  Flag versions can be merged using
// logical 'and', logical 'or', and 'replace' semantics. Flag versions can be restored to
// the main table of the MS.
//
// At the end of a "save" or "restore" operation, the latest flags are always also in
// the main table.
// 
// </synopsis>

// <motivation>
//  
// </motivation>

// <thrown>
//    <li>
//    <li>
// </thrown>


// <todo asof="$DATE:$">
//   <li> 
// </todo>

       
class FlagVersion 
{
   public:
      // Constructor
      FlagVersion(String intab, String dataflagcolname, String rowflagcolname);  

      // Destructor
      virtual ~FlagVersion();

      // Operator=
      // Equate by reference.
      FlagVersion& operator=(const FlagVersion&){return *this;}
      
      // Get a list of entries from the version-list file for this table 
      Vector<String> getVersionList();

      // Specify the Table column names to use as flag columns.
      // For example, for a MS, they are "FLAG" and "FLAG_ROW". 
      Bool attachFlagColumns(String version, ScalarColumn<Bool> &rowflag, 
                 ArrayColumn<Bool> &flags, Table &subtab);

      // Save current main table flags into a separate version 
      // These keeps a copy in the main table too. 
      // "merge" can be one of 'or','and','replace'
      Bool saveFlagVersion( String versionname, String comment, 
                  String merge=String("replace") );

      // Copy flags from a flag version, into the main table 
      Bool restoreFlagVersion(String versionname, 
                     String merge=String("replace") );

      // Delete a version. This does not touch or update the main table 
      Bool deleteFlagVersion( String versionname );

      // Clear all main table flags 
      Bool clearAllFlags();

   private:
      void FlagVersionError( String msg );
      
      Bool readVersionList();
      Bool saveFlagsInto(Table &fromFTab, Table &toFTab, 
                String merge = String("replace"));
      Bool doesVersionExist( String versionname );

      /* Variables to be maintained for the root Table */
      String verlistfile_p;
      String flagtablename_p;
      String dataflagcolname_p;
      String rowflagcolname_p;

      Table tab_p;
      String tabname_p;
      Vector<String> versionlist_p;
      Vector<String> commentlist_p;
      Bool fcol_p, frcol_p;
      Int nrows_p;

      Table subflagtable_p;

      File file_p;
      
      SLog* log;
      static String clname;
};

} //# NAMESPACE CASA - END 

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <tableplot/TablePlot/FlagVersion.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif

