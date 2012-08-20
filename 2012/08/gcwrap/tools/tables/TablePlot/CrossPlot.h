//# CrossPlot.h: Basic table access class for the TablePlot (tableplot) tool
//#              to plot across rows for an arrya-column.
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


#ifndef CROSSPLOT_H
#define CROSSPLOT_H

//# Includes

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/OS/Timer.h>

#include <tools/tables/TablePlot/BasePlot.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Basic table access class for the TablePlot (tableplot) tool
// - derived from BasePlot. Used for plotting across rows when 
// a table column is an arraycolumn.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="">
// </reviewed>

// <prerequisite>
//#! Classes or concepts you should understand before using this class.
// BasePlot
// </prerequisite>

// <etymology>
// CrossPlot is a class derived from BasePlot, and allows plots in a direction
// transposed to that allowed by BasePlot. 
// </etymology>

// <synopsis>
// Class CrossPlot holds the same data structures as does BasePlot, but
// provides a transposed view of arraycolumn data to the TPPlotter class
// which queries it to accumulate data to plot.
// 
// </synopsis>

// <example>
// <srcblock>
// // Instantiate BasePlot 
// CrossPlot<T> CP();
// CP.Init(Table&);
// CP.CreateTENS(Vector<String> &TaQL);
// CP.GetData();
// ... followed by TPPlotter 'setPlotRange' and 'plotData' calls
// </srcblock>
// </example>

// <motivation>
// This class was written to allow transposed plots for array-column data,
// where the 'x-axis' of the plot is by default the column index of the
// arraycolumn.
// For a measurement set type table, this corresponds to plotting data as
// a function of spectral channel. It is derived from BasePlot because the
// data read/write/storage mechanisms and data structures are identical to that
// in BasePlot. The only difference is the view presented to the TPPlotter
// class. The TPPlotter class does not distinguish between BasePlot and
// CrossPlot. In a future version, Histogram plots will also be implemented
// in this manner.
// </motivation>

// <templating arg=T>
//    <li>
// </templating>

// <thrown>
//    <li>
//    <li>
// </thrown>


// <todo asof="$DATE:$">
//# A List of bugs, limitations, extensions or planned refinements.
// </todo>


class CrossPlot : public BasePlot {
   public:
      // Constructor
      CrossPlot();  

      // Destructor
      ~CrossPlot();

      // This function is called from TPPlotter::setPlotRange().
      // Set plot range (all plots for this table).
      // Scan the data storage arrays to compute data ranges. In the case 
      // of overlay plots (due to Array TpDouble TaQL results), combined
      // data ranges for this tables data are computed.
      // This function requires that all stored data arrays be traversed.
      // This can get expensive for large number of data points. It is
      // assumed that for such a large number of data points, plotting
      // could broken down into chunks using an iteration axis.
      Int setPlotRange(Double &xmin, Double &xmax, Double &ymin, Double &ymax, 
         Bool showflags, Bool columnsxaxis, String flagversion, 
         Int averagenrows, String connectpoints=String("tablerow"), 
         Bool doscalingcorrection=True, String multicolour=String("none"),
         Bool honourxflags=False); 
      
      // Return True or False = if a point is selected
      Bool selectedPoint(Int np, Int nr);

      // Create TableExprNodes from input TAQL strings.
      Int createXTENS(Vector<String> &datastr);   
      
      // Query the internal structures for X,Y data and Flag values
      // These functions are called by TPPlotter
      Double getXVal(Int pnum, Int col);
      Double getYVal(Int pnum, Int col);
      Bool getYFlags(Int pnum, Int col);
      Int getNumRows();
      Int getNumPlots();
      
   private:
      
      // Read X data from the table. For CrossPlots this corresponds
      // to filling the X data arrays with channel indices.
      // For 'tid' set to row 0, the x storage arrays are created 
      // and filled. Subsequent calls to getXData are ineffectual,
      // since the accessed channel indices can be filled in after
      // reading only the first row of the table, 
      // but are required to preserve the format used in BasePlot
      // which reads a value from each row in the table.
      //Int getXData(TableExprId &tid);
      Int getXData(TPConvertBase* conv, Bool dummyread=False);

      // Average the channel numbers ! not really used...
      Int computeXAverages(Int averagenrows, Int remrows);

      void CrossPlotError(String msg);

      // These hold indices for cell rows, and cell cols.
      // i.e. enumerated values from TaQL indices.
      // i.e. for [1:2,4:8],
      // xpdrow => [0,1]
      // xpdcol => [3,4,5,6,7]
      Vector<Vector<Double> > xpdrow_p, xpdcol_p;
      Vector<Int> ncellrows_p,ncellcols_p;
      Bool columnsxaxis_p;
      Bool plotperrow_p;
      Bool honourxflags_p;

      // Variables from Base class.
      using BasePlot::ReductionType_p;
      using BasePlot::Xshape_p;
      using BasePlot::Yshape_p;
      using BasePlot::Map_p;
      using BasePlot::TENslices_p;
      using BasePlot::TENRowColFlag_p;
      using BasePlot::conv_p;
      using BasePlot::callbackhooks_p;
      using BasePlot::getTableName;
      
      using BasePlot::nflagmarks_p;
      using BasePlot::locflagmarks_p;
      using BasePlot::nTens_p;
      using BasePlot::NRows_p;
      using BasePlot::NPlots_p;
      using BasePlot::yplotdata_p;
      using BasePlot::theflags_p;
      using BasePlot::rowflags_p;
      using BasePlot::pType_p;
      
      using BasePlot::computeAverages;
      using BasePlot::Average_p;
      using BasePlot::NAvgRows_p;
      using BasePlot::avgyplotdata_p;
      using BasePlot::avgtheflags_p;
      using BasePlot::avgrowflags_p;
      using BasePlot::avgindices_p;
      
      using BasePlot::updateFlagHistory;
      using BasePlot::setFlags;
      using BasePlot::getFlags;
      using BasePlot::SelTab_p;
      using BasePlot::flagdirection_p;
      using BasePlot::showflags_p;
      using BasePlot::doscalingcorrection_p;
      using BasePlot::multicolour_p;
      using BasePlot::getColourAddOn;
      
      using BasePlot::log;
      using BasePlot::dbg;
      using BasePlot::ddbg;
      using BasePlot::adbg;
      using BasePlot::tmr;
     
      static String clname; 
};

} //# NAMESPACE CASA - END 

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <tools/tables/TablePlot/CrossPlot.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif

