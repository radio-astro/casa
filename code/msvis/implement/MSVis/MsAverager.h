//# MsAverager.h: Measurement Sets (Visiablity Averager)
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

#if !defined MSAVERAGER_H
#define MSAVERAGER_H


#include <casa/aips.h>

#include <tables/Tables/SetupNewTab.h>
//#include <tables/Tables/Table.h>
#include <tables/Tables/MemoryTable.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/StManAipsIO.h>

#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <casa/Containers/List.h>

#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/SubMS.h>
#include <msvis/MSVis/SelectAverageSpw.h>

#include <tableplot/TablePlot/SLog.h>


///////////////////////////////////////////////////////////////////////////////
//# Start of documentation.    
//
// <summary>
// A class to average a visibilty data set in time and/or channel
// </summary>
//
// <use visibility=export>
//
// <reviewed reviewer="Me" date="2007/12/25" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> VisBuffer
//   <li> MeasurementSet
// </prerequisite>
//
// <etymology>
// From "visibility", "time", "channel", "polarization" and "averaging".
// </etymology>
//
// <synopsis>
// This class averages VisBuffers in time, channel.
// </synopsis>
//
// <example>
//   MS ms("ngc5921.ms" Table::Update);
//   MsAverager msa(ms);
//   //chanlist is matrix of int its rows [spwid, start, end, step] 
//   msa.setAverager(chanlist, 90, 32, "data", "SCALAR");
//   if (msa.ok()) { 
//      MS avems;
//      msa.getMS(avems);   
//   }
//   
// </example>
//
// <motivation>
// Provide time/channel averaging capabilities for plotxy. 
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="2007/12/25">
//   <li> everything, and
//   <li> everything else
// </todo>

//# End of documentation.    
///////////////////////////////////////////////////////////////////////////////

namespace casa {


class MsAverager
{

public:
   enum OutputMode {
     TableMS = 1,   
     ListBuffer
   };

   MsAverager(MS*, OutputMode = MsAverager::TableMS);
   ~MsAverager();

   void reset(MS*, OutputMode = MsAverager::TableMS);
   void getMS(MS& ms);
   void getXY(Vector<Double>& x, Vector<Double>& y, 
              Vector<Int>& f, Int pol);
   void getMap(Matrix<Int>& rowMap, Matrix<Int>& chanMap);
    
   //average engine and its status 
   void setAverager(
              const Matrix<Int>& chanList,
              const Matrix<Int>& baselines,
              Double aveTime, Int aveChan,
              const String& column = "DATA",
              const String& aveMode = "VECTOR",
              const Bool& aveFlag = False,
              const Bool& aveScan = False,
              const Bool& aveBline = False,
              const Bool& aveArray = False,
              const Bool& aveVelo = False,
              const String& restfreq = "",
              const String& frame = "",
              const String& doppler = "");
   Bool ok() {return aveOK;}
   Int outputFormat() {return outputMode;}

   void showColumnNames();
   Bool isDataColumn(const String& colNmae);
   Bool hasColumn(const String& colName);
   static Int baselineRow(const Int& nAnt, const Int& a, const Int& b); 
   Int baselineRow(const Int& a = -1, const Int& b = -1); 

   void putAveBuffer(Double bufTime, Int bufField, Int bufScan,
                     Int bufArray, VisBuffer& p, Int nTime);
   void initAveBuffer(Double bufTime, VisBuffer& p, Int nAnt, Int nChan);
   void putAveTable(Double bufTime, Int bufField, Int bufScan,
                    Int bufArray, Int bufSpw, VisBuffer& p, Int nTime, 
                    Double timeShift);
   void showVisRow(Cube<Complex>& vc, Int row);
   void showMsRow(MSMainColumns* msc, Int row);
   void showAveMap(Matrix<Int>& rmap, Matrix<Int>& cmap);

   const static String DataColumn[4];
   Int npol() {return nAvePol;}
   Int nchan() {return nAveChan;}
   Int nAnt() {return nAntenna;}

private:
   //to be averaged
   void cleanup();
   MS* pMS;
   VisSet* vs;
   String msName;
   //MSDerivedValues *msdv;

   //averaged, store as MS
   MS aMS;
   Int msRow;
   MSMainColumns *msc;
   //MSColumns *msc;
   
   //averaged, store as VBs 
   VisBuffer* pAveBuff;
   List<VisBuffer*> aveList;

   //averaging inputs, ideally, calculate only needed column
   String column;

   //selected channels for each piece for spw expression
   //each row contains [spwid, start, end, stride]
   //may contains multiple rows of same spwid
   Matrix<Int> chanList;

   Vector<SAS> spw;

   //map between averaged and original 
   Matrix<Int> aveChanMap;
   //Matrix<Double> aveTimeMap;
   Matrix<Int> aveRowMap;

   //averaging requirements
   String aveMode;
   Double aveTime;
   Int aveChan;
   Bool aveFlag;
   Bool crossScans;
   Bool crossBlines;
   Bool crossArrays;
   Bool crossSpws;
   Bool aveVel;
   Bool sorryVel;
   Matrix<Int> baselines;

   //size of the averaged
   Int nAveChan;
   Int nAveTime;
   Int nAveRow;
   Int nAvePol;
   Int nAntenna;

   String restfreq;
   String frame;
   String doppler;

   Bool aveOK;

   //utility
   static const String clname;
   //SLog *log;

   int outputMode;

  
};


} 
#endif


