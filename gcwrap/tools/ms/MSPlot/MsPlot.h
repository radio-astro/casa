//# MsPlot.h: Plotting facilities for Measurement Sets
//# Copyright (C) 2003-2008
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
//# $Id: MsPlot.h,v 1.7.2.27 2006/10/12 18:22:01 sjaeger Exp $
//#
//# -------------------------------------------------------------------------
//# Change Log
//# Date	Name	    Description
//# 12/05/2006  S. Jaeger   Added unflagging capabilities.
//# 09/08/2007  S. Jaeger   Added callback functions for TablePlot
//#                         button handling.
//# 11/14/2007  S. Jaeger   Added class documentation and allowing
//#                         time expressions in the plotrange option


#if !defined CASA_MSPLOT_H
#define CASA_MSPLOT_H

//#! Includes go here
#include <casa/BasicSL/String.h>

//
#include <tables/Tables/Table.h>
#include <tools/tables/TablePlot/TablePlot.h>
#include <tools/tables/TablePlot/PanelParams.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>

//# MsPlot includes
#include <measures/Measures/MPosition.h>
#include <flagging/Flagging/SLog.h>

#include <msvis/MSVis/MsAverager.h>


namespace casa { //#! NAMESPACE CASA - BEGIN

//#//////////////////////////////////////////////////////////////////////////
//# All the wonderful docs, that will show up in the user reference
//# manual for this class.
    
// <summary>
// This class does, oh a little of this and a little of that. This class
// understands Measurement Sets and provides support for plotting various
// bits of data in the measurment set.
// </summary>

//# <use visibility=local>   or   <use visibility=export>
// <use visibility=export>


// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
//#! for example:
//#!  <reviewed reviewer="pshannon@nrao.edu" date="1994/10/10" tests="tMyClass, t1MyClass" demos="dMyClass, d1MyClass">
//#!  </reviewed>
// </reviewed>

//#! Classes or concepts you should understand before using this class.
// <prerequisite>
// <ul>    
//   <li> <linkto class="MeasurementSet:description">Measurement Set
//        </linkto> class,
//   <li> <linkto class="TablePlot">TablePlot</linkto>class, 
//   <li> <linkto class="Table">Table</linkto> class,
//       in particular TaQL expressions.
//   in with the table code.
// </ul>    
// </prerequisite>
//    
// <etymology>
// The implementation of the Measurement Set plotting facilities
// hence its called MS Plot.
// </etymology>
//
// <synopsis>
// This class implements the interface to the MS plotting tool. See the
// <linkto module="MeasurementSet:description">Measurement Set</linkto>
// description to get a good understanding of measurement sets and the
// <linkto class="TablePlot">TablePlot</linkto>class description for
// further details on plotting.
// 
// There are four basic activities that can be performed with the
// MS Plot tool, as follows:
// <ul>
//   <li>Selecting data to be plotted,
//   <li>Specify a number of plotting options,
//   <li>Making a plot, and
//   <li>(un)flagging and querying the plotted data
// </ul>
// 
// The data selection is done via the <linkto class="MSSelection">
// MSSelection</linkto> class, with some additions.  Although the
// added selections in MS plot will be in the MSSelection class in
// the future. The data selections made determine which data in the
// MeasurementSet is to be plotted.  Selections can be made on the
// following data:
// <ul>
//    <li>Spectral windows, include channels for each one
//    <li>Correlations
//    <li>Fields
//    <li>Baselines
//    <li>Scan number
//    <li>Array identifiers
//    <li>Time ranges
//    <li>UV distances
// </ul>
// See the <linkto class=MSSelection">MSSelection</linkto> class
// for details on the syntax.
//
// MS Plot uses all of the plotting options as defined by the
// <linkto class="PanelParam">PanelParam</linkto> class in the
// TablePlot module.  The plot options control the plot colours, 
// window size, overplotting, number of plots to display, labels
// on the axes, and many more options.  The plotrange option in
// MS Plot is specified as a String, rather then a list of doubles.
// This allows time ranges to be given in time values as well as numeric
// values, unlike <linkto class="TablePlot>TablePlot</linkto> which 
// allows numeric values only.
//
// The plotting of the data is where most of the tricky bits occur
// in this class. A good understanding of the process is important
// in understanding this class.  The general process is:
// <ol>
//    <li> verify the plot options, setting some of the internally
//         depending on some of choices the user has made for plotting
//         This includes setting
//         up a callback class or two if they are needed. The callback
//         classes are used to allow MS Plot some control over flagging,
//         displaying information about selected plotted points, resets,
//         and others.  See <linkto class="MsPlotHooks">MsPlotHooks</linkto>
//         class.
//    <li> Some of the plots require each data point to be altered, for
//         these plots "convert" functions have been set up.  These are
//         set up at this point in the plotting process.  An example of
//         a conversion class is, the UVDist plot converts all points from
//         m to klamba.  See <linkto class="MsPlotConvert">MsPlotConvert</linkto>
//         class.
//    <li> Next the set of table(s) to be plotted is constructed.  The
//         table may be one of:
//         <ul>
//             <li>The whole measurement set, ie. main table
//             <li>A sub table of the measurement set, such as the
//                 Antenna table
//             <li>The main table sorted by some criteria.  This step
//                 uses the <linkto class="MSIter">MSIter</linkto> class
//                 to divide and sort the main table into a number of
//                 smaller tables. This process is done if the measurment
//                 set contains spectral windows with differently shaped
//                 data, if different pieces of the data have different
//                 correlations, or if the user has selected time averaging.
//         </ul>
//         Once it is determined what tables are to be used these are given
//	   to TablePlot to store and keep track of.
//    <li>Create the Table Query expressions (TaQL) string.  In MS Plot
//        only the "select" portion of the TaQLs are constructed. The
//        TaQL strings created can be viewed in the log messages, if the
//        messaging level is set to INFO3.  The TaQL strings are designed
//        to do as much of the work as possible, for example the channel
//        averaging is done through careful construction of the TaQLs.
//        See AIPS++ note 199 for a good description of TaQL expressions.
    
//    <li>Finally the data is plotted by calling the table plot
//        <linkto method="TablePlot::plotData">plotData</linkto> method
//        for single plots or the
//        <linkto class="TablePlot">iterMultiPlotStart</linkto>
//#!  The method linkto arg doesn't seem to work properly    
//#!  <linkto method="TablePlot::iterMultiPlotStart">iterMultiPlotStart</linkto>
//        method for iterative plots.  Iterative plots allow users to circulate
//        over some data property.  For example, plotting each field
//        separately.
//    </ol>
//
//  All flagging facilities are currently handled by the
//  <linkto class="TablePlot">TablePlot</linkto>.  MS Plot
//  just hands all information given to the table plot class, this
//  may change in the future, for example to properly support flagging
//  of antenna's MS Plot will need to flag all data in the main table
//  as well as the Antenna subtable.
//
// The current model between <linkto class="TablePlot">TablePlot</linkto>
// and MSPlot has all of the GUI events being funnelled through TablePlot,
// but all of the command-line commands from the Python interpreter being
// funnelled through MSPlot to TablePlot.  This makes the design tricky!
//    
// The approach taken has been for the command-line flow, MsPlot accesses
// the <linkto class="TablePlot">TablePlot</linkto> through and instance
// of the class. For the GUI flow, MsPlot provides a set of classes which,
// are given to TablePlot, TablePlot then calls methods in these classes
// to give control to MsPlot. These classes are:
// <ul>
//    <li> <linkto class="MSPlotReset">MsPlotReset</linkto>, which
//         handles selection of the reset button on the GUI.</li>
//    <li> <linkto class="MSPlotMainMSCallBack">MsPlotMainMSCallBack</linkto>,
//        which handles the next, and locate buttons when data from the
//        main table has been plotted.</li>
//    <li> <linkto class="MSPlotAntennaCallBack">MsPlotAntennaCallBack</linkto>,
//        which handles the next, and locate buttons when data from the
//        Antenna has been plotted.</li>
//    <li> <linkto class="MSPlotAntennaCallBack">MsPlotUVWCallBack</linkto>,
//        which handles the next, and locate buttons when data from the
//        UVW column has been plotted.</li>
// </ol>    
// </synopsis>
//
// <example>
// This is a simple example which plots some Measurement Set data.
// <srcblock>
// // Creat a MSPlot object that will plot the file given.
// MsPlot myPlotter( "/dir1/dir2/myMeasurementSet.ms" );
//
// // Set some of the plot options.
// Record optionRec;
// optionRec.define( RecordFieldId( "nrows" ), Int(2) );
// optionRec.define( RecordFieldId( "ncols" ), Int(1) );
// optionRec.define( RecordFieldId( "panel" ), Int(1) );
// optionRec.define( RecordFieldId( "color" ), String( "green" ) );
// optionRec.define( RecordFieldId( "fontsize" ), Double( 7.0 ) );
// optionRec.define( RecordFieldId( "title" ), String( "My Plot" ) );
// if ( ! myPloter.setplotoptions( optionRec )
//    cout << "An error occured when setting the plot options" << endl;
//    return;
//
// // Select from which data to plot.  In this case we are selecting
// // field 0, spectral window 2, channels 10 through 120 with averaging.
// if ( ! myPlotter.setData( "", "0", "", "", "", "2:10~120^3", "vectorchunk", "", "", "", "" ) )
//    cout << "An error occured when setting th data" << endl;
//    return;
//
// // First check to see if the UVDistance plot is okay and if it is 
// // plot it.
// if ( myPlotter.ploxy( True, "uvdist", "data", "", "", "", "AMPLITUDE", "" ) )
//     myPlotter.ploxy( False, "uvdist", "data", "", "", "", "AMPLITUDE", "" ) );
//
// // Plot the visibility frequencies from the corrected data column
// mp.plotter.plotxy( False, "chan_freq", "data", "", "corrected", "", "", "" );
//
// // Clear all plotted data off of the window.
// myPlotter.clearPlot();
//
// // Close the measurment set we were using and switch to a new one
// myPlot.reset( True );
// myPlotter.open( "/dir1/dir2/myCoolMeasurmentSet.ms" );
//
// // Do an iterative plot on baselines, ie. plot the data for each baseline
// // in its own panel.  Each panel will show the weights.vs. uvdistance.
// optionRec.define( RecordFieldId( "nrows" ), Int(2) );
// optionRec.define( RecordFieldId( "ncols" ), Int(3) );
// optionRec.define( RecordFieldId( "panel" ), Int(1) );
// optionRec.define( RecordFieldId( "fontsize" ), Double( 7.0 ) );
// optionRec.define( RecordFieldId( "title" ), String( "My Plot" ) );
// if ( ! myPlotter.setplotoptions( optionRec )
//    cout << "An error occured when setting the plot options" << endl;
//    return;
// myPlotter.plotxy( False, "default_weight", "uvdist", "", "", "", "", "baseline" );
// myPlotter.iterPlotNext();
// myPlotter.iterPlotNext();
// myPlotter.iterPlotStop();
//
// // List the flag versions, load one up and plot the parallactic angle plot
// // and flag some data on the plot.
// myPlotter.getFlagVersionList();
// myPlotter.restoreFlagVersion( "testFlags", "" );
// myPlotter.plotxy( False, "hourangle", "data", "", "", "", "Phase", "" );
// Vector<Double> flagRegion(4);
// flagRegion[0]=3; flagRegion[1]=10; flagRegion[2]=-50; flagRegion[3]=0;
// myPlotter.flagData( FLAG );
// 
// </srcblock>
// </example>
//
// <motivation>
// To provide a plotting tool for Measurement Sets that is very
// flexible, but fairly easy to use.
// </motivation>
//
// <thrown>
//    <li> AIPSError
// </thrown>
//
// <todo asof="2007/10/29">
//   <li> Look at it making it generic for all CASA 
//   <li> Perhaps provide ways of "streaming" the error messages
//        instead of making it all a single String.    
// </todo>

//# End of documentation.    
///////////////////////////////////////////////////////////////////////////////

class MsPlot 
{
 public:
    // Constructor; creates an MsPlot object with no Measurement Set
    // or TablePlot object associated with it at this point.
    MsPlot();
    
    // Constructor; creates an MsPlot object with a Measurement Set
    // associated with it. A TablePlot object will also be created
    // with the associated main table in Measurement Set as the table.
    MsPlot( const String& MSPath );

    // Destructor, destroy the MsPlot object, destroying the TablePlot
    // and Measurement Set object with it.
    ~MsPlot();


    //#! General Member Functions for dealing with closing/opening/reset.

    // Open the Measurement Set (table) to be plotted, creating
    // a <linkto class="MeasurementSet">Measurement Set</linkto> object
    // as well as as <linto class"TablePlot">TablePlot</linkto> object.
    // The TablePlot object is initialize with the main table of the
    // Measurement Set 
    Bool open( const String& MSPath, Bool doVel = False,
               const String& restfreq = "", const String& frame = "",
               const String& doppler = "");
    
    // Initialize the class variables. 
    Bool initialize();

    // Reset the MsPlot class back to its initial state.
    Bool reset( Bool resetMS );

    // Free memory, reset defaults, and destroy the TablePlot and
    // MeasurementSet objects
    Bool done();
    

    //#! General Member Functions for dealing with plotting options.
    // Do all of the necessary setup required to prepare for plotting.
    // This incluces:
    // <ol>
    //   <li>Get the plotoptions set by the user</li>
    //   <li>Check correctness of the iteration value given, if one is given</li> 
    //   <li>Set up the callback classes responsible for converting data
    //       points to different units</li>
    //   <li>Figure out what Table(s) we are plotting, this may require some
    //       virtual manipulation of main table to create the set we need.
    //       Note that if any data selection was done by the user, we start
    //       with the resulting table from an MSSelection call.</li>
    //   <li>Create the TaQL (Table Query Language) expressions that will
    //       select the desired data from the tables.</li>
    //   <li>Apply any internal setting of plot options, see updatePlotOptions
    //       for more details on what options can be set internally.</li>
    // </ol>
    Bool setupPlotxy( const String& x,
	    const String& y,
	    const String& xcolumn,
	    const String& ycolumn,
	    const String& xvalue,
	    const String& yvalue,
	    Vector<String>& iteration );

    // Main plot function.
    // For iteration plots, this initializes the iterations 
    // and makes the first iterplot.
    Bool plotxy( const Bool checkOnly,
	    const String& x,
	    const String& y,
	    const String& xcolumn,
	    const String& ycolumn,
	    const String& xvalue,
	    const String& yvalue,
	    Vector<String>& iteration );

    // Iteration plotting.
    Bool iterPlotNext();
    Bool iterPlotStop( const Bool rmplotter );

    Bool clearPlot( const Int nrows,
	    const Int ncols,
	    const Int panel );
    
    Bool setplotoptions( Record );

    Bool saveFigure( const String& filename, 
	    const Int dpi, 
	    const String& orientation,
	    const String& papertype,
	    const String& facecolor,
	    const String& edgecolor );

    Bool summary( Bool selected );
    
    Bool setData( const String& baselineExpr,
	    const String& fieldExpr, 
	    const String& scanExpr,
	    const String& uvDistExpr, 
	    const String& arrayExpr,
	    const String& feedExpr,
	    const String& spwExpr,
	    const String& corrExpr,
	    const String& timeExpr );

    Bool setFlagExtension( const String& extendcorr = "",
	    const String& extendchan = "", 
	    const String& extendspw = "",
	    const String& extendant = "", 
	    const String& extendtime = "");

    Bool average(
                 const String& chanAveMode,
                 const String& corrAveMode,
                 const String& aveData,
                 const String& aveMode,
                 const String& aveChan,
                 const String& aveTime,
                 const Bool& aveFlag,
                 const Bool& aveScan,
                 const Bool& aveBl,
                 const Bool& aveArray,
                 const Bool& aveChanId,
                 const Bool& aveVel);
    
    // nchan: how many channels you want the selected data to be averagged to.
    // start: start channel number;
    // width: number of channels to average;
    // colName: the data column name -- DATA, CORRECTED_DATA or MODEL_DATA
    Bool setSpectral( const Vector<Int>& start,
	    const Vector<Int>& end,
	    const Vector<Int>& step,
	    const Bool width,
	    const String& average,
	    const String& frequencyExpr,
	    const String& correlationExpr );

    Bool markRegion( Int nrows, Int ncols, Int panel, Vector<Double> regionvec );
    Bool flagData( Int direction );
    Bool locateData();

    // Functions for flag version control.
    Bool saveFlagVersion(String versionname, 
                         String comment, 
			 String merge);
    Bool restoreFlagVersion(Vector<String> versionname, 
                            String merge);
    Bool deleteFlagVersion(Vector<String> versionname);
    Bool getFlagVersionList();

    // A number of methods for getting specific values for the
    // open measurement set.
    Double getRefFrequency( uInt spwId=0 );
    Double getChanFrequency( uInt spwId=0, uInt channel=0 );
    Double getChanVelocity( uInt spwId=0, uInt fieldId=0, uInt channel=0 );


//
protected:
    //#! Data Members

    //#! Constructors

    //#! Inheritable Member Functions

private:

    //  Message log, used to report warning, errors, and debugging information.
    SLog* log;
    static String clname;

    // The table plotter, a measurement set is a table and as such
    // can be plotted by the TablePlot class.
    TablePlot* itsTablePlot;

    // The Table that is to be plotted, this may or may not be the
    // full measurement set, depending on the values given to
    // setdata()
    Table* itsTable;

    // The measurement set that that is plotted.
    MeasurementSet* itsMS;
    
    // The reference MS created after data selection.
    // This is passed into TablePlot via setTableT().
    MeasurementSet itsSelectedMS;
    MSSelection itsMSSelectionObj;
    MeasurementSet itsAveragedMS;


    // In case an extra sub-selection needs to be done to account for
    // varying shapes in the main table (due to different spectral window shapes)
    // store the internally sub-selected MSs.
    // Store the list of tables to be plotted.
    //Vector<Table> itsTableVec;

    Vector<Vector<Table> > itsTableVectors;
    Vector<Vector<String> > itsTableVecNames;
    Vector<Vector<Int> >itsTableVecSpwIds;
    Vector<Vector<Int> >itsTableVecPolIds;
    //Vector<uInt> itsTableVecSpwIdNChans;

    
    // Info about currently selected MS.
    // A string with information about the current data selection.
    //String itsSelectString;
    Vector<String> itsSelectString;
    String itsDataSelectString;
    Vector<Int> itsSelectedSpwIds;
    Vector<Int> itsSpwIdNChans;
    Vector<Vector<Int> > itsSelectedCorrIds;
    Vector<Int> itsSelectedPolIds;
    Vector<Int> itsSelectedNPols;
    String itsTimeSelectString;
    Int itsTimeStep;

    MsAverager *msa;
    String itsDataColumn;
    String itsAveMode;
    Vector<String> itsAveCorr;
    Int itsAveChan;
    Double itsAveTime;
    Double itsMinTimeInterval;
    Bool itsAveFlag;
    Bool itsAveScan;
    Bool itsAveBl;
    Bool itsAveArray;
    Int nAveChan;
    Int nAvePol;
    Matrix<Int> chanList;
    Matrix<Int> aveRowMap;
    Matrix<Int> aveChanMap;
    Matrix<Double> aveTimeMap;
    Bool doAverage;
    Bool useAveChanId;
    Bool doVel;

    String itsRestFreq;
    String itsFrame;
    String itsDoppler;

    String itsExtendCorr;
    String itsExtendChan;
    String itsExtendSpw;
    String itsExtendAnt;
    String itsExtendTime;

    // Blarg
    uInt itsMaxPolId;	
    Matrix<Int> itsBaselines;
    Vector<Vector<String> > itsCorrelationNames;
    Vector<Vector<Int> > itsCorrelationIds;
    Vector<String> itsFieldNames;
    Vector<String> itsAntennaNames;
    Vector<Double> itsRefFrequencies;
    Vector<Vector<Double> > itsChanFrequencies;
    // x=spw id, y= field id, z = channel num
    Vector< Vector<Vector<Double> > > itsChanVelocities;
    Vector<Int> itsScanNumbers;
    MVTime itsStartTime;
    MVTime itsEndTime;
    uInt itsMaxSpwId;
    Vector<Bool> itsUsedSpws;
    
    Array<Double> itsPlotRange;
    
    // The various plot options that can be set by the user. The current
    // list of options includes: TODO : update this list from PanelParams.h
    //
    //      ncols, nrows, panel, windowsize, aspectratio,
    //      fontsize, plotstyle, plotcolour, plotsymbol, pointlabels,
    //      plotrange, linewidth, overplot, multicolour, timeplot, 
    //      crossdirection, and useflags,
    // @see TablePlot::parsePOption() for the list of options supported by
    // table plot and their descriptions
    // @see TPPlotter::parsePlotOptions() for more of the options and their
    // descriptions.
    PlotOptions itsPlotOptions;

    // This record is the persistant record of parameters.
    // Required so that any plot-type-dependant parameters
    // can be reset, while retaining all user-parameters.
    // Every MsPlot::plotxy call, refills itsPlotOptions from
    // this record, and then calls MsPlot::updatePlotOptions to
    // augment the params for plot-type dependant parameters.
    // SDJ This is probably not needed
    Record itsOptionRecord;

    // Stores the TaQL string used by TablePlot to select the data
    // to be plotted.
    Vector< Vector<Vector<String> > > itsTaqlStrings;
    

    // Store the iteration axes information.
    Vector<String> itsIterationAxes;


    // The time range selected as a pair of double values.  Time
    // ranges are applied to the x-axis only, and only for timeplot.
    Vector< Double > itsTimeRange;
    

    // Values to keep track of iterative plots.
    //<group>
    Bool itsIterPlotOn;
    //Bool itsIterPlot;
    Vector<String> itsIterLabels;
    Vector<String> itsIterAxes;
    //</group>

    // Variables to keep track of the current data selection
    // for channels and correlations.
    // These are used inside the dataTaQL function to create
    // TaQL strings that reflect this chan, corr selection.
    //<group>
    Vector<Vector<Int> > itsStartChan;
    Vector<Vector<Int> > itsEndChan;
    Vector<Vector<Int> > itsStepChan;
    Vector<String> itsAverage;
    String itsStepOrSlice;
    Vector<Vector<Int> > itsStartCorr;
    Vector<Vector<Int> > itsEndCorr;
    Vector<Vector<Int> > itsStepCorr;
    //</group>


  //#! Private Member Functions
    TPGuiCallBackHooks *itsCallBack;
    TPResetCallBack *itsResetCallBack;
            
    Bool itsInitialized;
    Bool itsMsIsOpen;
        
    
    // Flags to decide when to send a Table into TablePlot.
    // For performance reasons, do not re-set a Table
    // if there is no need to.
    Bool itsIsNewSelection;
    Bool itsIsSubTablePlot;
    Bool itsIsIterationPlot;
    Bool itsIsInIterPlot;
    Bool itsConvertClassSet;
    Bool itsSpwIterOn;
    Bool itsIsUVWPlot;
    
    
    String defaultDataSelectString();
    String defaultSpectralSelectString();
    

    void resetDataSelection();
    void resetTableSelection();

    Bool makeDataSelectStr( MSSelection& MSSelecObj,
	    const Vector< Vector<String> >& selectedCorrs,  
	    const String& selectedTimes );
    Bool corrSelection( const String& correlation, Vector< Vector<String> >& corrNames );
    Bool corrAndStokesParser( const String& corrExpr, Vector< Vector<String> >& names );
    Bool validateCorrAndStokes( Vector< Vector<String> >& names );
    Bool makeCorrIdLists( Vector< Vector<Int> >& ids );
    Bool getChannelInfo( Matrix<Int> chanSelectList );
    Bool setSingleChannelSet( uInt spwId, Int start, Int stop, Int step );
    String getTimeExprStr( const String& times, const String& aveOrStep );
    String getTimeExprStr( const String& times );

    /////////////////////////////////////////////////////////////////////
    // In order to make thins faster when we get a new Measurement Set we
    // store a bunch of information in some internal variables.
    // <group>
    void  initAllLists();
    void  getAllAntennaNames();
    void  getAllFieldNames();
    void  getAllCorrelations();
    void  getAllSpwsAndFreqs();
    void  getAllSpwChanNums();
    void  getAllScanNumbers();
    void  getAllTimes();
    //</group>

    // Parse the input provided by the user for the plotrange option, if
    // double/int values are given the values are taken as they are.  If
    // the values are strings then it's assumed that they are time ranges
    // The plotrange is expected to be a string in the form:
    //     [xmin,xmax,ymin,ymax]
    //     where each is either a double value or in the from YY/MM/DD/hh:mm:ss
    //
    // RETURN: returns error/warning messages.  If no mesasges are returned
    //         then there were no errors.  The warnings are in retValue[1]
    //         and the errors in retValue[0];
    //
    // <group>
    Vector<String> parsePlotRange();
    Vector<String> plotrangeToDbls( Vector<String> rangeStrVec,
	    Vector<Double> rangeVector,
	    Vector<Bool> rangeSet );
    // </group>

    ///////////////////////////////////////////////////////////////////////////
    // Parse a string assuming to be a list of values. The list of values is
    // expected to be in one of the following formats:
    // 	[item1, item2, ..., itemN]
    // 	(item1, item2, ..., itemN)
    //
    //      If no item is given between two commas an empty string is returned
    //      for this item in the string vector.       	
    //
    // RETURN: returns error/warning messages.  If no mesasges are returned 
    //         then there were no errors.  The warnings are in retValue[1] and 
    //         the errors in retValue[0];
    Vector<String> parseStrToList( const String& inString, Vector<String>& outVector );
    

    
    /////////////////////////////////////////////////////////////////////
    // Methods used to figure out what needs to be plotted.

    // Find which table to give to Table plot, it will be one of three:
    //     1. The whole measurement set
    //     2. The SubMS created with MSSelection values
    //     3. A SubMS with channel averaging
    //Table* getTable();

    // Create the TaQL string from the various parameters given
    // to plotxy
    Bool
	createTaQL( const String& x,
		const String& y,
		const String& xcolumn,
		const String& ycolumn,
		const String& xvalue,
		const String& yvalue,
		String& title,
		String& xlabel,
		String& ylabel );

   Bool
	createSaQL(
		const String& x,
		const String& y,
		const String& xvalue,
		const String& yvalue,
		String& title,
		String& xlabel,
		String& ylabel);

    Vector<String>
	getTaQL( const String& axisStr,
	    const String& column,
	    const String& value,
	    const uInt spwId,
	    const uInt polId,
	    const char axis,
	    String& label);

    Bool checkAverageMode( String optName, String mode, String selectExpr );
    Bool checkAverageMode( String optName, String mode );
    
    Bool checkIterationAxes( const Vector<String>& iteration );

    Bool setSingleInputTable( const Vector<String>& sortOrder,
	    const uInt spwId,
	    const uInt polId,
	    const Table table,
	    const String tableName );
    Bool setInputTable( const String& xcolumn, const String& ycolumn );

    Bool updatePlotOptions( const String& title, 
	    const String& xlabel,
	    const String& ylabel,
	    const String& xcolumn,
	    const String& ycolumn,
	    const String& x,
	    const String& y );
    void cleanupOptions();
			      
    //Bool setSpwLists( Bool checkShape );
    Bool checkSpwShapes();
    Bool checkPolShapes();
    //Bool setSpwChanNums();

    Vector<String> dataTaQL( const String& column,
                                         const String& value,
	    				 const uInt spwId,
	    				 const uInt polId,
					 String& label );
    
    
    // Create the vector of XTaQL, YTaQL pairs that TablePlot likes...
    String getIndices( const Int& startcorr,
	    const Int& endcorr,
	    const Int& stepcorr,
	    const Int& startchan,
	    const Int& endchan,
	    const Int& stepchan,
	    const String& column,
	    const String& value,
	    String& label );

    // Find the conversion value for converting uvdists from meters
    // to kilometers
    Double getMeterToKlambdaConvertValue( uInt spwIndex );
    Bool getDerivedConvertVectors();
    Bool setupConvertFn( const String& axisStr, const Char& axis );
    
    // Conversion routines which use MSDerived to calculate the
    // conversion values.
    Vector<Double>
	msDerivedConvert( const String& convertType );
    
    
    /////////////////////////////////////////////////////////////////////
    // Misceleanouse methods.
    
    // Verify that the class variables have been iniiatlized.  If they
    // haven't been then initialize them.
    Bool checkInit();

    // This is needed for flagging.  We need to know if we've made an
    // array plot, or more precisely a subtable plot.
    Int checkForArrayPlot( Int subplot );
    

    // Verify that we have openen a measurement set.
    Bool checkOpenMS();

    //	 Initialize the plot title, x-axis label, and y-axis label to their 
    // default settings, the empty  sting.  This method can also be used
    // to reset the labels to their default values.
    void initializeLabels();

    // Enable/Disable all buttons.
    void disableAllButtons();
    void enableAllButtons();


};

 
  //template class MsPlot;
} //# NAMESPACE CASA - END

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <msvis/MSPlot/MsPlot.cc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif


