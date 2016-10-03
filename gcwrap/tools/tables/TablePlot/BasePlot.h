//# BasePlot.h: Basic table access class for the TablePlot (tableplot) tool
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003-2008
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
//# 01/10/2007   Urvashi R.V.     Added a locateData function here and to CrossPlot
//# 08/24/2007   Urvashi R.V.     Modified documentation for all recent changes.


#ifndef BASEPLOT_H
#define BASEPLOT_H

//# Includes

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/OS/Timer.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Slicer.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/TableColumn.h>

#include <tables/TaQL/ExprNode.h>
#include <tables/TaQL/RecordGram.h>

#include <flagging/Flagging/SLog.h>

#include <tools/tables/TablePlot/TPCallBackHooks.h>
#include <flagging/Flagging/FlagVersion.h>

#include <ms/MeasurementSets/MeasurementSet.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Basic table access class for the TablePlot (tableplot) tool
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="">
// </reviewed>

// <prerequisite>
//#! Classes or concepts you should understand before using this class.
// </prerequisite>

// <etymology>
// BasePlot is the basic class that accesses one table/subtable and extracts
// data to be plotted. It provides the interface between the TablePlot class
// and the actual tables.
// </etymology>

// <synopsis>
// Class BasePlot is the basic table access class for the purpose of plotting
// data via TaQL expressions. A BasePlot object can operate on one table/subtable
// and handles the extraction of data corresponding to TaQL expressions for the
// purpose of plotting. It is to be used in conjunction with the TPPlotter class
// via the TablePlot class, to connect the data from a table to a plotting device.
// 
// </synopsis>

// <example>
// <srcblock>
// ( see TablePlot.cc )
// // Instantiate BasePlot 
// BasePlot<T> BP();
// BP.Init(casacore::Table&);
// BP.getData();
// ... followed by TPPlotter 'setPlotRange' and 'plotData' calls
// </srcblock>
// </example>

// <motivation>
// This class was written so that each table/subtable can be handled
// independant of each other. This allows simple bookkeeping and clean
// relationships between tables and data sets when multiple tables are
// being simultaneously accessed (plotting/editing). Issues like different 
// numbers of data points to be plotted from different tables, and 
// simultaneous editing on multiple tables/subtables (on one or more
// plot panels) are handled by this class organization.
// </motivation>

// <templating arg=T>
//    <li>
// </templating>

// <thrown>
//    <li>
//    <li>
// </thrown>


// <todo asof="$DATE:$">
//# A casacore::List of bugs, limitations, extensions or planned refinements.
//   <li> Provide the option to flag all channels (or a range) for a chosen range of stokes, or to flag all (or range of) stokes for the chosen channel range.
//   <li> At some point, use TempArray instead of casacore::Matrix, to hold all the data to plot. Then,
// one can limit the memory consumption, at the expense of having TempArrays on disk.
// </todo>

enum { XYPLOT, CROSSPLOT, HISTPLOT };
enum { FLAG=1, UNFLAG=0 };
 
class BasePlot 
{
   public:
      // Constructor
      BasePlot();  

      // Destructor
      virtual ~BasePlot();

      // Operator=
      // Equate by reference.
      BasePlot& operator=(const BasePlot&){return *this;}

      // Attach the BasePlot object to a table/subtable.
      // This function also checks if the FLAG and/or 
      // FLAG_ROW column names exist.
      casacore::Int init(casacore::Table &tab, casacore::Int &tableNumber,casacore::String &rootTabName, 
               casacore::String &tableSelection, casacore::String &dataFlagColName, 
               casacore::String &rowFlagColName);   
      
      // Create TableExprNodes for all TaQL strings.
      // The casacore::RecordGram::parse() function is used to 
      // create TableExprNodes for each of the TaQL strings in the
      // input vector. The parse tree for each expression is also
      // traversed to extract table column names and corresponding
      // index ranges that are accessed. This is required while flagging
      // based on the result of a TaQL expression. For instance, in a MS
      // type table, if the 'MEAN' TaQL function is used to average data 
      // over several channels/stokes, only one set of data is plotted,
      // but while flagging all accessed channels/stokes must be flagged.
      // The function returns -1 if there is a TaQL syntax error, or if
      // the data-type of the expression result is not TpDouble, or if
      // an odd number of TaQL strings in sent in the input vector.
      // (Only expressions that return a TpDouble scalar or array can
      // be directly plotted).
      casacore::Int createTENS(casacore::Vector<casacore::String> &datastr);   
      
      // Read data from the table and fill up storage arrays.
      // This function reads the results of all TableExprNodes
      // from the first row of the table/subtable, to obtain the
      // shapes of the TaQL results. casacore::Data storage arrays are
      // accordingly resized and the reading continues for all
      // other rows.
      // Flags are read using the getFlags function.
      // If the shape of the data column being accessed matches that of
      // the FLAG column, a one-to-one mapping of flags is done.
      // If the shapes mis-match (or if only one flag exists per row), 
      // then the FLAG_ROW column is read. If neither FLAG nor FLAG_ROW
      // exist, all flags are assumed as false.
      // Errors in TaQL indices are caught and -1 is returned.
      casacore::Int getData(casacore::Vector<casacore::String> &datastr, casacore::Int layer, 
         TPConvertBase* conv, TPGuiCallBackHooks* callbackhooks_p);      
      
      // This function is callse from TPPlotter::setPlotRange().
      // Set plot range (all plots for this table).
      // Scan the data storage arrays to compute data ranges. In the case 
      // of overlay plots (due to casacore::Array TpDouble TaQL results), combined
      // data ranges for this tables data are computed.
      // This function requires that all stored data arrays be traversed.
      // This can get expensive for large number of data points. It is
      // assumed that for such a large number of data points, plotting
      // could broken down into chunks using an iteration axis.
      virtual casacore::Int setPlotRange(casacore::Double &xmin, casacore::Double &xmax, 
              casacore::Double &ymin, casacore::Double &ymax, 
              casacore::Bool showflags, casacore::Bool columnsxaxis, casacore::String flagversion, 
              casacore::Int averagenrows, casacore::String connectpoints=casacore::String("tablerow"),
              casacore::Bool doscalingcorrection=true,casacore::String multicolour=casacore::String("none"),
              casacore::Bool honourxflags=false); 

      
      // This function is called from TPPlotter::setFlagRegions().
      // The list of regions that have been marked for flagging
      // via TPPlotter::markFlags() is passed into BasePlot and 
      // stored.
      casacore::Int convertCoords(casacore::Vector<casacore::Vector<casacore::Double> > &flagmarks);
      
      // Fill in flags in the storage arrays.
      // The data storage arrays are traversed and flags for all
      // data values falling within the chosen flag regions are
      // set to true. If diskwrite=1, updated flags are written to disk.
      // If diskwrite=0, the flags are not written to disk, but will
      // be applied to plots that use the current instance of BasePlot.
      // If rowflag=1, a the FLAG_ROW column is set (if it exists) in
      // addition to the individual flags in the FLAG column (if it exists).
      casacore::Int flagData(casacore::Int diskwrite, casacore::Int rowflag, casacore::Int direction);     

      virtual casacore::Bool selectedPoint(casacore::Int np, casacore::Int nr);     

      casacore::Int flagData(casacore::Int direction, casacore::String msname, 
                   casacore::String ext);     
      casacore::Int flagData(casacore::Int direction, casacore::String msname, casacore::String spwexpr, 
                   casacore::Matrix<casacore::Int>& rowMap, casacore::Matrix<casacore::Int>& chanMap, casacore::String ext);     
      casacore::Int getFlags(casacore::String versionname, casacore::String msname);
      ////casacore::Int setFlags(casacore::Int direction, casacore::Int setrowflag, casacore::String msname);
      
      // Clear all flags (FLAG and FLAG_ROW) from the current
      // table/subtable.
      casacore::Int clearFlags();   
      
      casacore::Bool saveData(const casacore::String& filename);

      // Query the internal structures for X data values
      virtual casacore::Double getXVal(casacore::Int pnum, casacore::Int col);
      
      // Query the internal structures for Y data values
      virtual casacore::Double getYVal(casacore::Int pnum, casacore::Int col);

      // Query the internal structures for flags
      virtual casacore::Bool getYFlags(casacore::Int pnum, casacore::Int col);
      void showFlags();

      // Query for the number of points per plot
      virtual casacore::Int getNumRows();

      // Query for the number of plots
      virtual casacore::Int getNumPlots();

      // Get the plot colour to use - based on "multicolour"
      // This number is added to the "color" in PlotOption, to
      // generate the different colours for different cellrows/cellcols
      virtual casacore::Int getColourAddOn(casacore::Int pnum);

      // Query for the type of plot (BASEPLOT)
      // For BasePlot, this is "XYPLOT".
      // For CrossPlot, this is "CROSS".
      casacore::Int getPlotType();

      // Get some casacore::Table Info
      casacore::Vector<casacore::String> getBasePlotInfo();
      
      // Locate Data
      // Step through the data, check if each point has been selected.
      // If it has, then read out the row number, and index into the
      // casacore::Table and pull out the values of each of the LocateColumns
      // for each selected row number.  Channel/Corr (cellcol/cellrow)
      // indices are also found, and concatenated into a [row,col] string
      // to be returned for printing.
      virtual casacore::Int locateData(casacore::Vector<casacore::String> collist, 
           casacore::Matrix<casacore::Double> &info, casacore::Vector<casacore::String> &cpol);

      // Make this different from the above just in case
      // somebody else also use this function
      // make TablePlot to know which one to call
      virtual casacore::Int locateData(casacore::Vector<casacore::String> collist, 
           casacore::Matrix<casacore::Double> &info, casacore::Vector<casacore::String> &cpol,
           casacore::Matrix<casacore::Int>& rmap, casacore::Matrix<casacore::Int> &cmap);
      
      // Update Flag History
      // Fill in these parameters with a list of flag regions, 
      // flag or unflag, and the
      // number of points selected in these regions.
      casacore::Int updateFlagHistory(casacore::Vector<casacore::Vector<casacore::Double> > &flagmarks,
           casacore::Int &direction, casacore::Int &numflags);

      //Number of rows in the BP's Table.
      casacore::Int NRows_p;
                
      // A pointer to the CallBack class. One of them is held
      // for each BP.
      TPGuiCallBackHooks* callbackhooks_p;

       // Return the name of the casacore::Table being held by BP.
       casacore::String getTableName();

   protected:
      
      // Create TableExprNodes from input TAQL strings
      virtual casacore::Int createXTENS(casacore::Vector<casacore::String> &datastr);   
      casacore::Int createYTENS(casacore::Vector<casacore::String> &datastr);   
      
      // Read in the values ( after TaQL evaluations ) into
      // the storage arrays.
      virtual casacore::Int getXData(TPConvertBase* conv,casacore::Bool dummyread=false);
      casacore::Int getYData(TPConvertBase* conv,casacore::Bool dummyread=false);

      // Read flags from the table into theflags_p
      // This reconciles the original indices in the FLAG column, with the
      // indices selected via TaQL. It also take care of getting the flags
      // in when a TaQL Scalar or casacore::Vector reduction has been done.
      // Also, it checks with flagsum_p, to see if anyone else has
      // updated flags for this casacore::Table, and if so, recomputes the TaQL so that
      // channel averages take into account updated flags.
      // And lots more !!
      casacore::Int getFlags( casacore::String versionname, casacore::Bool showflags );

      // Recompute TaQLs in case another BP has changed the flags
      // for this row. 
      casacore::Int reGetYData(casacore::Int tenid, casacore::Int row, TPConvertBase* conv);

      // Get TaQL incides
      casacore::Int getIndices(casacore::TableExprNode &ten);
      
      // Traverse the TaQL parse tree and collect TaQL index ranges
      void ptTraverse(const casacore::TableExprNodeRep *tenr);
      
      // Write flags to disk.
      // This does the inverse of getFlags.
      // Flag expansion for TaQL scalar/vector reduction happens here.
      casacore::Int setFlags(casacore::Int direction, casacore::Int setrowflag);
   
      // Clears currently held flag region lists and TaQL index lists.
      casacore::Int cleanUp();      

      // Get the chan, pol from np
      casacore::Matrix<casacore::String> getLocateIndices();
      casacore::Matrix<casacore::String> getLocateIndices(casacore::Matrix<casacore::Int>& cmap);

      // Get the chan, pol from np
      casacore::Matrix<casacore::Int> getLocatePolChan();
      
      // Create Map_p = a matrix of important indices.
      // nrows : NPlots_p. So each cellcol/cellrow gets
      //         its own Map_p row.
      // ncols : 5
      // Col 0 : index into xplotdata 
      // Col 1 : index into yplotdata
      // Col 2 : index into tens (z)
      // Col 3 : yplotdata row index
      // Col 4 : yplotdata col index
      casacore::Int createMap();

      // Compute averages of rows.
      casacore::Int computeAverages(casacore::Int averagenrows);      

      // Compute averages of X values.. this is just a place-holder.
      virtual casacore::Int computeXAverages(casacore::Int averagenrows, casacore::Int remrows);

      // Exceptions..
      void BasePlotError(casacore::String msg);

      
      // The single casacore::Table to be accessed.
      casacore::Table SelTab_p;

      casacore::Table* itsTab_p;
      casacore::String itsTabName_p;

      // The internal number assigned to the table by
      // TablePlot
      casacore::uInt tabNum_p;
      // The name of the root casacore::Table on disk.
      // If a memory casacore::Table, this should be "".
      casacore::String rootTabName_p;
      // A string with selection information.
      casacore::String tabSelString_p;

      // casacore::List of TaQL strings.
      casacore::Vector<casacore::String> DataStr_p;
      // Flag to signal a new table being attached
      casacore::Int TableTouch_p; 
      // Remember the "layer" of the plot from this basePlot.
      // This is used to discard "getData" calls for other layers.
      casacore::Int Layer_p;

      // Flag to signal the use of averages or not.
      // This is for "averagenrows" and averaging across rows.
      // If true, then points are averaged, before being sent to TPLP.
      casacore::Bool Average_p; 

      // number of TaQL string pairs.
      casacore::Int nTens_p; 
      // TableExprnodes created from these TaQL string pairs
      casacore::Vector<casacore::TableExprNode> xtens_p;
      casacore::Vector<casacore::TableExprNode> ytens_p;

      // Matrices to hold the actual data to be plotted.
      // i.e. the values obtained after TaQL evaluation.
      casacore::Matrix<casacore::Double> xplotdata_p;
      casacore::Matrix<casacore::Float> yplotdata_p;

      // casacore::Matrix to hold the data flags
      casacore::Matrix<casacore::Bool> theflags_p;
      // casacore::Vector to hold the row flags
      casacore::Vector<casacore::Bool> rowflags_p;
                
      // casacore::Vector to hold the number of flagged cell rows/cols per row.
      // For an casacore::MS, this is the number of corrs/chans flagged per row.
      // This is used to check when another BasePlot has changed the
      // flags. ( remember - multiple BPs can work on the same Table
      // at once...) and this ensures that channel averages are 
      // recomputed for rows for which the flags have changed.
      // This is how flags changed through one plot, are immediately
      // updated in all other plots from the same Table.
      casacore::Matrix<casacore::Int> flagsum_p;

      // Flag to distinguish between loading flagsum_p for the first
      // time, and using it to check for "changed" rows.
      casacore::Bool firsttime_p;

      // If averaging is done across rows, the averaged
      // values are stored in these variables.
      // This is in addition to "xplotdata_p, etc.. "
      // Since this is filled on the fly, one can generate
      // plots with different 'averagenrows', without
      // having to re-read data from disk.
      casacore::ArrayColumn<casacore::Bool> AvgFlags_p;
      casacore::ScalarColumn<casacore::Bool> AvgRowFlags_p;

      casacore::Matrix<casacore::Double> avgxplotdata_p;
      casacore::Matrix<casacore::Float> avgyplotdata_p;
      casacore::Matrix<casacore::Bool> avgtheflags_p;
      casacore::Vector<casacore::Bool> avgrowflags_p;
      casacore::Vector<casacore::Int> avgindices_p;
      
      // Number of plots
      // sum (ncellrows x ncellcols from each TaQL XY pair )
      // TPLP sees this number, and asks for data to create
      // this number of plots.
      casacore::Int NPlots_p;

       // Number of rows left after averaging across rows
       // ( averagenrows )
       casacore::Int NAvgRows_p; 

      // a VERY important data structure that holds 
      // very useful index information. This is used
      // all over the place. A description of this
      // casacore::Matrix is in the code of BasePlot::createMap().
      casacore::Matrix<casacore::Int> Map_p;

      // The first TaQL indices for each Y-TaQL.
      casacore::Vector<casacore::Slicer> TENslices_p;

      // The Column shapes for columns accessed by each Y-TaQL.
      // This is used to decide if a scalar/vector TaQL reduction
      // has happened or not. This info goes into ReductionType_p
      casacore::Vector<casacore::IPosition> TENcolshapes_p;

      // Holds info about which cellrows/cellcols are
      // completely flagged per row. This is for use in
      // averaging channel numbers -- not really used.
      casacore::Vector<casacore::Vector<casacore::Bool> > TENRowColFlag_p;
                
      // casacore::List of reduction-types for each Y-TaQL.
      // 0 : no reduction
      // 1 : scalar reduction (SUM(..))
      // 2 : vector reduction (SUM(..,1))
      // 3 : vector reduction (SUM(..,2))
      casacore::Vector<casacore::Int> ReductionType_p;
      
      // Signal for each TaQL using an ArrayCol or ScalarCol.
      casacore::Vector<casacore::Bool> isArrayCol_p;

      // Shapes for Xdata and Ydata.
      casacore::Vector<casacore::IPosition> Yshape_p;
      casacore::Vector<casacore::IPosition> Xshape_p;
                
      // accessed column names
      casacore::Vector<casacore::String> colnames_p;
      // accessed column slices
      casacore::Vector<casacore::Slicer> ipslice_p;
      // number of pairs of colnames and indices.
      casacore::Int nip_p; 
      // mapping from yplotdata index to colnames_p indices.
      casacore::Vector<casacore::Int> IndCnt_p; 
      
      // Flag holders...
      casacore::ArrayColumn<casacore::Bool> Flags_p;
      casacore::ScalarColumn<casacore::Bool> RowFlags_p;
      casacore::String FlagColName_p,FlagRowName_p;
      casacore::IPosition FlagColShape_p;
      // flags of FLAG,FLAG_ROW existence.
      casacore::Bool fcol_p,frcol_p; 
      
      // list of flag regions ( or locate regions )
      casacore::Vector<casacore::Vector<casacore::Double> > locflagmarks_p;
      // # flag regions
      casacore::Int nflagmarks_p; 
      // 1 : FLAG, 0 : UNFLAG
      casacore::Int flagdirection_p; 
      // number of selected points.
      casacore::Int numflagpoints_p;

      // Plot Options that need to be used here.
      casacore::Bool showflags_p;
      casacore::Bool doscalingcorrection_p;
      casacore::String multicolour_p;

      // Pointer to the Convert function
      TPConvertBase* conv_p;

      // Dear ol' FlagVersion pointer.
      FlagVersion *FV;
      casacore::String currentflagversion_p;

      casacore::Int dbg,ddbg,adbg;
      casacore::Timer tmr;

      casacore::Int pType_p;

      SLog *log;
      static casacore::String clname;
};

} //# NAMESPACE CASA - END 

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <tools/tables/TablePlot/BasePlot.cc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif

