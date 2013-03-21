//# CTDesc.h: Define the format of NewCalTable
//# Copyright (C) 2011 
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef CALIBRATION_CTDESC_H
#define CALIBRATION_CTDESC_H

#include <casa/aips.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> 
// CTDesc: Define the format of calibration tables
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="CalTable">CalTable</linkto> module
// </prerequisite>
//
// <etymology>
// From "Calibration table" and "descriptor".
// </etymology>
//
// <synopsis>
// CTDesc defines the format of NewCalTable in terms
// of table descriptors <linkto class="TableDesc">TableDesc</linkto>, as
// defined in the Table system. This is a base class and defines the
// overall calibration table structure. Specializations for both
// antenna-based (ViJones and SkyJones) and baseline-based (MJones)
// calibration tables are provided through inheritance. At present this 
// set of classes returns the default calibration table descriptors for a 
// given calibration table type.
// </etymology>
//
// <example>
// <srcblock>
// </srcblock>
// </example>
//
// <motivation>
// This inheritance tree defines the format of all calibration
// table types, both antenna- and baseline-based, to provide
// a unified and consistent interface to their representation
// as aips++ tables. An overall structure common to all calibration
// tables is maintained as far as possible, with common column
// names throughout. This class tree is primarily used by
// the <linkto class="CalTable"> CalTable</linkto> classes.
// </motivation>
//
// <todo asof="98/01/01">
// Generate the calibration table descriptors from a VisSet or VisBuffer object
// </todo>

class CTDesc 
{
 public:
   // Default null constructor, and destructor
   CTDesc();
   virtual ~CTDesc() {};

   // Alternative ctor that enables OBS_ID opt-out
   CTDesc(Bool addObsId);

   // Construct from some external info
   CTDesc (const String& partype,
	   const String& msname="none",
	   const String& viscal="unknown",
	   const String& polbasis="circ");

   // Return the table descriptors for the main calibration table
   // and the cal_history and cal_desc sub-tables
   virtual TableDesc calMainDesc();
   //virtual TableDesc calMainDesc();
   //virtual TableDesc calHistoryDesc();

 protected:
   // a specified column name.
   TableDesc insertDesc (const TableDesc& tableDesc, const TableDesc& insert,
			 const String& insertAfter);

 private:
   // Generate the default sub-table descriptors
   TableDesc defaultCalMain (const String& partype="",
			     const String& msname="none",
			     const String& viscal="unknown",
			     const String& polbasis="circ");
   //TableDesc defaultCalHistory();


   // Support (discouraged) ctor that can opt out of OBSERVATION_ID
   Bool addObsId_;

   // Table descriptors
   TableDesc itsCalMainDesc;
   //TableDesc itsCalHistoryDesc;
   String MSName_p;
   String PolBasis_p;


 };


} //# NAMESPACE CASA - END

#endif
