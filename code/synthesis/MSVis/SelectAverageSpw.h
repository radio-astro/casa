//# SelecteAverageSpwChan.h: 
//# Copyright (C) 2003
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
//#
//# -------------------------------------------------------------------------


#if !defined SAS_H
#define SAS_H


#include <casa/aips.h>

#include <tables/Tables/SetupNewTab.h>
//#include <tables/Tables/Table.h>
#include <tables/Tables/MemoryTable.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/StManAipsIO.h>

#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <casa/Containers/List.h>

#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/VisSet.h>



///////////////////////////////////////////////////////////////////////////////
//# Start of documentation.    
//
// <summary>
// A class to select/average spectral windows and associated channels
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="Me" date="2008/07/07" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> MeasurementSet
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
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="2008/07/06">
//   <li> everything, and
//   <li> everything else
// </todo>

//# End of documentation.    
///////////////////////////////////////////////////////////////////////////////

namespace casa {

class SelectAverageSpw;

typedef SelectAverageSpw SAS;

class SelectAverageSpw
{
public:
   Int spwid;
   Int desc;
   Double rFreq;
   Int measFreqRef;

   //selected channels
   Vector<Int> chans;

   //averaged channels
   Vector<Double> aveFreqs;
   Vector<Int> aveChans;
   Vector<Int> sxsChans;
   Vector<String> aveChanNames; 
   Matrix<Int> aveChanMaps; 

   //velocity depends on both freq and field and
   //can not be a member of this


   static Int nextSelected(Int spw, Int currId, Matrix<Int>& cList);
   static Int selectAverageChan(MS*, const Matrix<Int>& chanList,
                                Vector<SAS>& sp, const Int& aveChan = 1);
   static void averageVelocity(Bool &sorry,
                               MS*, Vector<SAS>& sp, Vector<Double>& velo,
                               const Int& spwidx, const Int& filed = 0,
                               const String& restfreq = "",
                               const String& frame = "",
                               const String& doppler = "");
   static void showSASC(const Vector<SAS>& sp);
   static void chanMap(Matrix<Int>& cmap, const Vector<SAS>& sp);
   static void showSASC();
   static Int descBySpw(const Int& spid, const Vector<SAS>& sp);
   static Int spwByDesc(const Int& desc, const Vector<SAS>& sp);
   static Int spwIndexByDesc(const Int& desc, const Vector<SAS>& sp);
   static Int spwIndexBySpw(const Int& spid, const Vector<SAS>& sp);

   static const Int maxChan;

  
};


} 
#endif


