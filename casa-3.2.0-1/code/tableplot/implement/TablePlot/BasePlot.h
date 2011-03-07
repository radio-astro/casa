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

#include <tables/Tables/ExprNode.h>
#include <tables/Tables/RecordGram.h>

#include <tableplot/TablePlot/SLog.h>

#include <tableplot/TablePlot/TPCallBackHooks.h>
#include <tableplot/TablePlot/FlagVersion.h>

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
// BP.Init(Table&);
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
//# A List of bugs, limitations, extensions or planned refinements.
//   <li> Provide the option to flag all channels (or a range) for a chosen range of stokes, or to flag all (or range of) stokes for the chosen channel range.
//   <li> At some point, use TempArray instead of Matrix, to hold all the data to plot. Then,
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
      Int init(Table &tab, Int &tableNumber,String &rootTabName, 
               String &tableSelection, String &dataFlagColName, 
               String &rowFlagColName);   
      
      // Create TableExprNodes for all TaQL strings.
      // The RecordGram::parse() function is used to 
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
      Int createTENS(Vector<String> &datastr);   
      
      // Read data from the table and fill up storage arrays.
      // This function reads the results of all TableExprNodes
      // from the first row of the table/subtable, to obtain the
      // shapes of the TaQL results. Data storage arrays are
      // accordingly resized and the reading continues for all
      // other rows.
      // Flags are read using the getFlags function.
      // If the shape of the data column being accessed matches that of
      // the FLAG column, a one-to-one mapping of flags is done.
      // If the shapes mis-match (or if only one flag exists per row), 
      // then the FLAG_ROW column is read. If neither FLAG nor FLAG_ROW
      // exist, all flags are assumed as False.
      // Errors in TaQL indices are caught and -1 is returned.
      Int getData(Vector<String> &datastr, Int layer, 
         TPConvertBase* conv, TPGuiCallBackHooks* callbackhooks_p);      
      
      // This function is callse from TPPlotter::setPlotRange().
      // Set plot range (all plots for this table).
      // Scan the data storage arrays to compute data ranges. In the case 
      // of overlay plots (due to Array TpDouble TaQL results), combined
      // data ranges for this tables data are computed.
      // This function requires that all stored data arrays be traversed.
      // This can get expensive for large number of data points. It is
      // assumed that for such a large number of data points, plotting
      // could broken down into chunks using an iteration axis.
      virtual Int setPlotRange(Double &xmin, Double &xmax, 
              Double &ymin, Double &ymax, 
              Bool showflags, Bool columnsxaxis, String flagversion, 
              Int averagenrows, String connectpoints=String("tablerow"),
              Bool doscalingcorrection=True,String multicolour=String("none"),
              Bool honourxflags=False); 

      
      // This function is called from TPPlotter::setFlagRegions().
      // The list of regions that have been marked for flagging
      // via TPPlotter::markFlags() is passed into BasePlot and 
      // stored.
      Int convertCoords(Vector<Vector<Double> > &flagmarks);
      
      // Fill in flags in the storage arrays.
      // The data storage arrays are traversed and flags for all
      // data values falling within the chosen flag regions are
      // set to true. If diskwrite=1, updated flags are written to disk.
      // If diskwrite=0, the flags are not written to disk, but will
      // be applied to plots that use the current instance of BasePlot.
      // If rowflag=1, a the FLAG_ROW column is set (if it exists) in
      // addition to the individual flags in the FLAG column (if it exists).
      Int flagData(Int diskwrite, Int rowflag, Int direction);     

      virtual Bool selectedPoint(Int np, Int nr);     

      Int flagData(Int direction, String msname, 
                   String ext);     
      Int flagData(Int direction, String msname, String spwexpr, 
                   Matrix<Int>& rowMap, Matrix<Int>& chanMap, String ext);     
      Int getFlags(String versionname, String msname);
      ////Int setFlags(Int direction, Int setrowflag, String msname);
      
      // Clear all flags (FLAG and FLAG_ROW) from the current
      // table/subtable.
      Int clearFlags();   
      
      Bool saveData(const String& filename);

      // Query the internal structures for X data values
      virtual Double getXVal(Int pnum, Int col);
      
      // Query the internal structures for Y data values
      virtual Double getYVal(Int pnum, Int col);

      // Query the internal structures for flags
      virtual Bool getYFlags(Int pnum, Int col);
      void showFlags();

      // Query for the number of points per plot
      virtual Int getNumRows();

      // Query for the number of plots
      virtual Int getNumPlots();

      // Get the plot colour to use - based on "multicolour"
      // This number is added to the "color" in PlotOption, to
      // generate the different colours for different cellrows/cellcols
      virtual Int getColourAddOn(Int pnum);

      // Query for the type of plot (BASEPLOT)
      // For BasePlot, this is "XYPLOT".
      // For CrossPlot, this is "CROSS".
      Int getPlotType();

      // Get some Table Info
      Vector<String> getBasePlotInfo();
      
      // Locate Data
      // Step through the data, check if each point has been selected.
      // If it has, then read out the row number, and index into the
      // Table and pull out the values of each of the LocateColumns
      // for each selected row number.  Channel/Corr (cellcol/cellrow)
      // indices are also found, and concatenated into a [row,col] string
      // to be returned for printing.
      virtual Int locateData(Vector<String> collist, 
           Matrix<Double> &info, Vector<String> &cpol);

      // Make this different from the above just in case
      // somebody else also use this function
      // make TablePlot to know which one to call
      virtual Int locateData(Vector<String> collist, 
           Matrix<Double> &info, Vector<String> &cpol,
           Matrix<Int>& rmap, Matrix<Int> &cmap);
      
      // Update Flag History
      // Fill in these parameters with a list of flag regions, 
      // flag or unflag, and the
      // number of points selected in these regions.
      Int updateFlagHistory(Vector<Vector<Double> > &flagmarks, 
           Int &direction, Int &numflags);

      //Number of rows in the BP's Table.
      Int NRows_p;
                
      // A pointer to the CallBack class. One of them is held
      // for each BP.
      TPGuiCallBackHooks* callbackhooks_p;

       // Return the name of the Table being held by BP.
       String getTableName();

   protected:
      
      // Create TableExprNodes from input TAQL strings
      virtual Int createXTENS(Vector<String> &datastr);   
      Int createYTENS(Vector<String> &datastr);   
      
      // Read in the values ( after TaQL evaluations ) into
      // the storage arrays.
      virtual Int getXData(TPConvertBase* conv,Bool dummyread=False);
      Int getYData(TPConvertBase* conv,Bool dummyread=False);

      // Read flags from the table into theflags_p
      // This reconciles the original indices in the FLAG column, with the
      // indices selected via TaQL. It also take care of getting the flags
      // in when a TaQL Scalar or Vector reduction has been done.
      // Also, it checks with flagsum_p, to see if anyone else has
      // updated flags for this Table, and if so, recomputes the TaQL so that
      // channel averages take into account updated flags.
      // And lots more !!
      Int getFlags( String versionname, Bool showflags );

      // Recompute TaQLs in case another BP has changed the flags
      // for this row. 
      Int reGetYData(Int tenid, Int row, TPConvertBase* conv);

      // Get TaQL incides
      Int getIndices(TableExprNode &ten);
      
      // Traverse the TaQL parse tree and collect TaQL index ranges
      void ptTraverse(const TableExprNodeRep *tenr);
      
      // Write flags to disk.
      // This does the inverse of getFlags.
      // Flag expansion for TaQL scalar/vector reduction happens here.
      Int setFlags(Int direction, Int setrowflag);
   
      // Clears currently held flag region lists and TaQL index lists.
      Int cleanUp();      

      // Get the chan, pol from np
      Matrix<String> getLocateIndices();
      Matrix<String> getLocateIndices(Matrix<Int>& cmap);

      // Get the chan, pol from np
      Matrix<Int> getLocatePolChan();
      
      // Create Map_p = a matrix of important indices.
      // nrows : NPlots_p. So each cellcol/cellrow gets
      //         its own Map_p row.
      // ncols : 5
      // Col 0 : index into xplotdata 
      // Col 1 : index into yplotdata
      // Col 2 : index into tens (z)
      // Col 3 : yplotdata row index
      // Col 4 : yplotdata col index
      Int createMap();

      // Compute averages of rows.
      Int computeAverages(Int averagenrows);      

      // Compute averages of X values.. this is just a place-holder.
      virtual Int computeXAverages(Int averagenrows, Int remrows);

      // Exceptions..
      void BasePlotError(String msg);

      
      // The single Table to be accessed.
      Table SelTab_p;

      Table* itsTab_p;
      String itsTabName_p;

      // The internal number assigned to the table by
      // TablePlot
      uInt tabNum_p;
      // The name of the root Table on disk.
      // If a memory Table, this should be "".
      String rootTabName_p;
      // A string with selection information.
      String tabSelString_p;

      // List of TaQL strings.
      Vector<String> DataStr_p;
      // Flag to signal a new table being attached
      Int TableTouch_p; 
      // Remember the "layer" of the plot from this basePlot.
      // This is used to discard "getData" calls for other layers.
      Int Layer_p;

      // Flag to signal the use of averages or not.
      // This is for "averagenrows" and averaging across rows.
      // If True, then points are averaged, before being sent to TPLP.
      Bool Average_p; 

      // number of TaQL string pairs.
      Int nTens_p; 
      // TableExprnodes created from these TaQL string pairs
      Vector<TableExprNode> xtens_p;
      Vector<TableExprNode> ytens_p;

      // Matrices to hold the actual data to be plotted.
      // i.e. the values obtained after TaQL evaluation.
      Matrix<Double> xplotdata_p;
      Matrix<Float> yplotdata_p;

      // Matrix to hold the data flags
      Matrix<Bool> theflags_p;
      // Vector to hold the row flags
      Vector<Bool> rowflags_p;
                
      // Vector to hold the number of flagged cell rows/cols per row.
      // For an MS, this is the number of corrs/chans flagged per row.
      // This is used to check when another BasePlot has changed the
      // flags. ( remember - multiple BPs can work on the same Table
      // at once...) and this ensures that channel averages are 
      // recomputed for rows for which the flags have changed.
      // This is how flags changed through one plot, are immediately
      // updated in all other plots from the same Table.
      Matrix<Int> flagsum_p;

      // Flag to distinguish between loading flagsum_p for the first
      // time, and using it to check for "changed" rows.
      Bool firsttime_p;

      // If averaging is done across rows, the averaged
      // values are stored in these variables.
      // This is in addition to "xplotdata_p, etc.. "
      // Since this is filled on the fly, one can generate
      // plots with different 'averagenrows', without
      // having to re-read data from disk.
      ArrayColumn<Bool> AvgFlags_p;
      ScalarColumn<Bool> AvgRowFlags_p;

      Matrix<Double> avgxplotdata_p;
      Matrix<Float> avgyplotdata_p;
      Matrix<Bool> avgtheflags_p;
      Vector<Bool> avgrowflags_p;
      Vector<Int> avgindices_p;
      
      // Number of plots
      // sum (ncellrows x ncellcols from each TaQL XY pair )
      // TPLP sees this number, and asks for data to create
      // this number of plots.
      Int NPlots_p;

       // Number of rows left after averaging across rows
       // ( averagenrows )
       Int NAvgRows_p; 

      // a VERY important data structure that holds 
      // very useful index information. This is used
      // all over the place. A description of this
      // Matrix is in the code of BasePlot::createMap().
      Matrix<Int> Map_p;

      // The first TaQL indices for each Y-TaQL.
      Vector<Slicer> TENslices_p;

      // The Column shapes for columns accessed by each Y-TaQL.
      // This is used to decide if a scalar/vector TaQL reduction
      // has happened or not. This info goes into ReductionType_p
      Vector<IPosition> TENcolshapes_p;

      // Holds info about which cellrows/cellcols are
      // completely flagged per row. This is for use in
      // averaging channel numbers -- not really used.
      Vector<Vector<Bool> > TENRowColFlag_p;
                
      // List of reduction-types for each Y-TaQL.
      // 0 : no reduction
      // 1 : scalar reduction (SUM(..))
      // 2 : vector reduction (SUM(..,1))
      // 3 : vector reduction (SUM(..,2))
      Vector<Int> ReductionType_p;
      
      // Signal for each TaQL using an ArrayCol or ScalarCol.
      Vector<Bool> isArrayCol_p;

      // Shapes for Xdata and Ydata.
      Vector<IPosition> Yshape_p;
      Vector<IPosition> Xshape_p;
                
      // accessed column names
      Vector<String> colnames_p;
      // accessed column slices
      Vector<Slicer> ipslice_p;
      // number of pairs of colnames and indices.
      Int nip_p; 
      // mapping from yplotdata index to colnames_p indices.
      Vector<Int> IndCnt_p; 
      
      // Flag holders...
      ArrayColumn<Bool> Flags_p;
      ScalarColumn<Bool> RowFlags_p;
      String FlagColName_p,FlagRowName_p;
      IPosition FlagColShape_p;
      // flags of FLAG,FLAG_ROW existence.
      Bool fcol_p,frcol_p; 
      
      // list of flag regions ( or locate regions )
      Vector<Vector<Double> > locflagmarks_p; 
      // # flag regions
      Int nflagmarks_p; 
      // 1 : FLAG, 0 : UNFLAG
      Int flagdirection_p; 
      // number of selected points.
      Int numflagpoints_p;

      // Plot Options that need to be used here.
      Bool showflags_p;
      Bool doscalingcorrection_p;
      String multicolour_p;

      // Pointer to the Convert function
      TPConvertBase* conv_p;

      // Dear ol' FlagVersion pointer.
      FlagVersion *FV;
      String currentflagversion_p;

      Int dbg,ddbg,adbg;
      Timer tmr;

      Int pType_p;

      SLog *log;
      static String clname;
};

} //# NAMESPACE CASA - END 

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <tableplot/TablePlot/BasePlot.cc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif

