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
#include <ms/MSSel/MSSelection.h>

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
//   <li> <linkto class="casacore::MeasurementSet:description">Measurement Set
//        </linkto> class,
//   <li> <linkto class="TablePlot">TablePlot</linkto>class, 
//   <li> <linkto class="casacore::Table">casacore::Table</linkto> class,
//       in particular TaQL expressions.
//   in with the table code.
// </ul>    
// </prerequisite>
//    
// <etymology>
// The implementation of the Measurement Set plotting facilities
// hence its called casacore::MS Plot.
// </etymology>
//
// <synopsis>
// This class implements the interface to the casacore::MS plotting tool. See the
// <linkto module="casacore::MeasurementSet:description">Measurement Set</linkto>
// description to get a good understanding of measurement sets and the
// <linkto class="TablePlot">TablePlot</linkto>class description for
// further details on plotting.
// 
// There are four basic activities that can be performed with the
// casacore::MS Plot tool, as follows:
// <ul>
//   <li>Selecting data to be plotted,
//   <li>Specify a number of plotting options,
//   <li>Making a plot, and
//   <li>(un)flagging and querying the plotted data
// </ul>
// 
// The data selection is done via the <linkto class="casacore::MSSelection">
// casacore::MSSelection</linkto> class, with some additions.  Although the
// added selections in casacore::MS plot will be in the casacore::MSSelection class in
// the future. The data selections made determine which data in the
// casacore::MeasurementSet is to be plotted.  Selections can be made on the
// following data:
// <ul>
//    <li>Spectral windows, include channels for each one
//    <li>Correlations
//    <li>Fields
//    <li>Baselines
//    <li>Scan number
//    <li>casacore::Array identifiers
//    <li>casacore::Time ranges
//    <li>UV distances
// </ul>
// See the <linkto class=casacore::MSSelection">casacore::MSSelection</linkto> class
// for details on the syntax.
//
// casacore::MS Plot uses all of the plotting options as defined by the
// <linkto class="PanelParam">PanelParam</linkto> class in the
// TablePlot module.  The plot options control the plot colours, 
// window size, overplotting, number of plots to display, labels
// on the axes, and many more options.  The plotrange option in
// casacore::MS Plot is specified as a casacore::String, rather then a list of doubles.
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
//         classes are used to allow casacore::MS Plot some control over flagging,
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
//    <li>Create the casacore::Table Query expressions (TaQL) string.  In casacore::MS Plot
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
//  <linkto class="TablePlot">TablePlot</linkto>.  casacore::MS Plot
//  just hands all information given to the table plot class, this
//  may change in the future, for example to properly support flagging
//  of antenna's casacore::MS Plot will need to flag all data in the main table
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
// casacore::Record optionRec;
// optionRec.define( casacore::RecordFieldId( "nrows" ), casacore::Int(2) );
// optionRec.define( casacore::RecordFieldId( "ncols" ), casacore::Int(1) );
// optionRec.define( casacore::RecordFieldId( "panel" ), casacore::Int(1) );
// optionRec.define( casacore::RecordFieldId( "color" ), casacore::String( "green" ) );
// optionRec.define( casacore::RecordFieldId( "fontsize" ), casacore::Double( 7.0 ) );
// optionRec.define( casacore::RecordFieldId( "title" ), casacore::String( "My Plot" ) );
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
// if ( myPlotter.ploxy( true, "uvdist", "data", "", "", "", "AMPLITUDE", "" ) )
//     myPlotter.ploxy( false, "uvdist", "data", "", "", "", "AMPLITUDE", "" ) );
//
// // Plot the visibility frequencies from the corrected data column
// mp.plotter.plotxy( false, "chan_freq", "data", "", "corrected", "", "", "" );
//
// // Clear all plotted data off of the window.
// myPlotter.clearPlot();
//
// // Close the measurment set we were using and switch to a new one
// myPlot.reset( true );
// myPlotter.open( "/dir1/dir2/myCoolMeasurmentSet.ms" );
//
// // Do an iterative plot on baselines, ie. plot the data for each baseline
// // in its own panel.  Each panel will show the weights.vs. uvdistance.
// optionRec.define( casacore::RecordFieldId( "nrows" ), casacore::Int(2) );
// optionRec.define( casacore::RecordFieldId( "ncols" ), casacore::Int(3) );
// optionRec.define( casacore::RecordFieldId( "panel" ), casacore::Int(1) );
// optionRec.define( casacore::RecordFieldId( "fontsize" ), casacore::Double( 7.0 ) );
// optionRec.define( casacore::RecordFieldId( "title" ), casacore::String( "My Plot" ) );
// if ( ! myPlotter.setplotoptions( optionRec )
//    cout << "An error occured when setting the plot options" << endl;
//    return;
// myPlotter.plotxy( false, "default_weight", "uvdist", "", "", "", "", "baseline" );
// myPlotter.iterPlotNext();
// myPlotter.iterPlotNext();
// myPlotter.iterPlotStop();
//
// // casacore::List the flag versions, load one up and plot the parallactic angle plot
// // and flag some data on the plot.
// myPlotter.getFlagVersionList();
// myPlotter.restoreFlagVersion( "testFlags", "" );
// myPlotter.plotxy( false, "hourangle", "data", "", "", "", "Phase", "" );
// casacore::Vector<casacore::Double> flagRegion(4);
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
    MsPlot( const casacore::String& MSPath );

    // Destructor, destroy the MsPlot object, destroying the TablePlot
    // and Measurement Set object with it.
    ~MsPlot();


    //#! General Member Functions for dealing with closing/opening/reset.

    // Open the Measurement Set (table) to be plotted, creating
    // a <linkto class="casacore::MeasurementSet">Measurement Set</linkto> object
    // as well as as <linto class"TablePlot">TablePlot</linkto> object.
    // The TablePlot object is initialize with the main table of the
    // Measurement Set 
    casacore::Bool open( const casacore::String& MSPath, casacore::Bool doVel = false,
               const casacore::String& restfreq = "", const casacore::String& frame = "",
               const casacore::String& doppler = "");
    
    // Initialize the class variables. 
    casacore::Bool initialize();

    // Reset the MsPlot class back to its initial state.
    casacore::Bool reset( casacore::Bool resetMS );

    // Free memory, reset defaults, and destroy the TablePlot and
    // casacore::MeasurementSet objects
    casacore::Bool done();
    

    //#! General Member Functions for dealing with plotting options.
    // Do all of the necessary setup required to prepare for plotting.
    // This incluces:
    // <ol>
    //   <li>Get the plotoptions set by the user</li>
    //   <li>Check correctness of the iteration value given, if one is given</li> 
    //   <li>Set up the callback classes responsible for converting data
    //       points to different units</li>
    //   <li>Figure out what casacore::Table(s) we are plotting, this may require some
    //       virtual manipulation of main table to create the set we need.
    //       Note that if any data selection was done by the user, we start
    //       with the resulting table from an casacore::MSSelection call.</li>
    //   <li>Create the TaQL (casacore::Table Query Language) expressions that will
    //       select the desired data from the tables.</li>
    //   <li>Apply any internal setting of plot options, see updatePlotOptions
    //       for more details on what options can be set internally.</li>
    // </ol>
    casacore::Bool setupPlotxy( const casacore::String& x,
	    const casacore::String& y,
	    const casacore::String& xcolumn,
	    const casacore::String& ycolumn,
	    const casacore::String& xvalue,
	    const casacore::String& yvalue,
	    casacore::Vector<casacore::String>& iteration );

    // Main plot function.
    // For iteration plots, this initializes the iterations 
    // and makes the first iterplot.
    casacore::Bool plotxy( const casacore::Bool checkOnly,
	    const casacore::String& x,
	    const casacore::String& y,
	    const casacore::String& xcolumn,
	    const casacore::String& ycolumn,
	    const casacore::String& xvalue,
	    const casacore::String& yvalue,
	    casacore::Vector<casacore::String>& iteration );

    // Iteration plotting.
    casacore::Bool iterPlotNext();
    casacore::Bool iterPlotStop( const casacore::Bool rmplotter );

    casacore::Bool clearPlot( const casacore::Int nrows,
	    const casacore::Int ncols,
	    const casacore::Int panel );
    
    casacore::Bool setplotoptions( casacore::Record );

    casacore::Bool saveFigure( const casacore::String& filename, 
	    const casacore::Int dpi, 
	    const casacore::String& orientation,
	    const casacore::String& papertype,
	    const casacore::String& facecolor,
	    const casacore::String& edgecolor );

    casacore::Bool summary( casacore::Bool selected );
    
    casacore::Bool setData( const casacore::String& baselineExpr,
	    const casacore::String& fieldExpr, 
	    const casacore::String& scanExpr,
	    const casacore::String& uvDistExpr, 
	    const casacore::String& arrayExpr,
	    const casacore::String& feedExpr,
	    const casacore::String& spwExpr,
	    const casacore::String& corrExpr,
	    const casacore::String& timeExpr );

    casacore::Bool setFlagExtension( const casacore::String& extendcorr = "",
	    const casacore::String& extendchan = "", 
	    const casacore::String& extendspw = "",
	    const casacore::String& extendant = "", 
	    const casacore::String& extendtime = "");

    casacore::Bool average(
                 const casacore::String& chanAveMode,
                 const casacore::String& corrAveMode,
                 const casacore::String& aveData,
                 const casacore::String& aveMode,
                 const casacore::String& aveChan,
                 const casacore::String& aveTime,
                 const casacore::Bool& aveFlag,
                 const casacore::Bool& aveScan,
                 const casacore::Bool& aveBl,
                 const casacore::Bool& aveArray,
                 const casacore::Bool& aveChanId,
                 const casacore::Bool& aveVel);
    
    // nchan: how many channels you want the selected data to be averagged to.
    // start: start channel number;
    // width: number of channels to average;
    // colName: the data column name -- DATA, CORRECTED_DATA or MODEL_DATA
    casacore::Bool setSpectral( const casacore::Vector<casacore::Int>& start,
	    const casacore::Vector<casacore::Int>& end,
	    const casacore::Vector<casacore::Int>& step,
	    const casacore::Bool width,
	    const casacore::String& average,
	    const casacore::String& frequencyExpr,
	    const casacore::String& correlationExpr );

    casacore::Bool markRegion( casacore::Int nrows, casacore::Int ncols, casacore::Int panel, casacore::Vector<casacore::Double> regionvec );
    casacore::Bool flagData( casacore::Int direction );
    casacore::Bool locateData();

    // Functions for flag version control.
    casacore::Bool saveFlagVersion(casacore::String versionname, 
                         casacore::String comment, 
			 casacore::String merge);
    casacore::Bool restoreFlagVersion(casacore::Vector<casacore::String> versionname, 
                            casacore::String merge);
    casacore::Bool deleteFlagVersion(casacore::Vector<casacore::String> versionname);
    casacore::Bool getFlagVersionList();

    // A number of methods for getting specific values for the
    // open measurement set.
    casacore::Double getRefFrequency( casacore::uInt spwId=0 );
    casacore::Double getChanFrequency( casacore::uInt spwId=0, casacore::uInt channel=0 );
    casacore::Double getChanVelocity( casacore::uInt spwId=0, casacore::uInt fieldId=0, casacore::uInt channel=0 );


//
protected:
    //#! casacore::Data Members

    //#! Constructors

    //#! Inheritable Member Functions

private:

    //  Message log, used to report warning, errors, and debugging information.
    SLog* log;
    static casacore::String clname;

    // The table plotter, a measurement set is a table and as such
    // can be plotted by the TablePlot class.
    TablePlot* itsTablePlot;

    // The casacore::Table that is to be plotted, this may or may not be the
    // full measurement set, depending on the values given to
    // setdata()
    casacore::Table* itsTable;

    // The measurement set that that is plotted.
    casacore::MeasurementSet* itsMS;
    
    // The reference casacore::MS created after data selection.
    // This is passed into TablePlot via setTableT().
    casacore::MeasurementSet itsSelectedMS;
    casacore::MSSelection itsMSSelectionObj;
    casacore::MeasurementSet itsAveragedMS;


    // In case an extra sub-selection needs to be done to account for
    // varying shapes in the main table (due to different spectral window shapes)
    // store the internally sub-selected MSs.
    // Store the list of tables to be plotted.
    //casacore::Vector<casacore::Table> itsTableVec;

    casacore::Vector<casacore::Vector<casacore::Table> > itsTableVectors;
    casacore::Vector<casacore::Vector<casacore::String> > itsTableVecNames;
    casacore::Vector<casacore::Vector<casacore::Int> >itsTableVecSpwIds;
    casacore::Vector<casacore::Vector<casacore::Int> >itsTableVecPolIds;
    //casacore::Vector<casacore::uInt> itsTableVecSpwIdNChans;

    
    // Info about currently selected MS.
    // A string with information about the current data selection.
    //casacore::String itsSelectString;
    casacore::Vector<casacore::String> itsSelectString;
    casacore::String itsDataSelectString;
    casacore::Vector<casacore::Int> itsSelectedSpwIds;
    casacore::Vector<casacore::Int> itsSpwIdNChans;
    casacore::Vector<casacore::Vector<casacore::Int> > itsSelectedCorrIds;
    casacore::Vector<casacore::Int> itsSelectedPolIds;
    casacore::Vector<casacore::Int> itsSelectedNPols;
    casacore::String itsTimeSelectString;
    casacore::Int itsTimeStep;

    MsAverager *msa;
    casacore::String itsDataColumn;
    casacore::String itsAveMode;
    casacore::Vector<casacore::String> itsAveCorr;
    casacore::Int itsAveChan;
    casacore::Double itsAveTime;
    casacore::Double itsMinTimeInterval;
    casacore::Bool itsAveFlag;
    casacore::Bool itsAveScan;
    casacore::Bool itsAveBl;
    casacore::Bool itsAveArray;
    casacore::Int nAveChan;
    casacore::Int nAvePol;
    casacore::Matrix<casacore::Int> chanList;
    casacore::Matrix<casacore::Int> aveRowMap;
    casacore::Matrix<casacore::Int> aveChanMap;
    casacore::Matrix<casacore::Double> aveTimeMap;
    casacore::Bool doAverage;
    casacore::Bool useAveChanId;
    casacore::Bool doVel;

    casacore::String itsRestFreq;
    casacore::String itsFrame;
    casacore::String itsDoppler;

    casacore::String itsExtendCorr;
    casacore::String itsExtendChan;
    casacore::String itsExtendSpw;
    casacore::String itsExtendAnt;
    casacore::String itsExtendTime;

    // Blarg
    casacore::uInt itsMaxPolId;	
    casacore::Matrix<casacore::Int> itsBaselines;
    casacore::Vector<casacore::Vector<casacore::String> > itsCorrelationNames;
    casacore::Vector<casacore::Vector<casacore::Int> > itsCorrelationIds;
    casacore::Vector<casacore::String> itsFieldNames;
    casacore::Vector<casacore::String> itsAntennaNames;
    casacore::Vector<casacore::Double> itsRefFrequencies;
    casacore::Vector<casacore::Vector<casacore::Double> > itsChanFrequencies;
    // x=spw id, y= field id, z = channel num
    casacore::Vector< casacore::Vector<casacore::Vector<casacore::Double> > > itsChanVelocities;
    casacore::Vector<casacore::Int> itsScanNumbers;
    casacore::MVTime itsStartTime;
    casacore::MVTime itsEndTime;
    casacore::uInt itsMaxSpwId;
    casacore::Vector<casacore::Bool> itsUsedSpws;
    
    casacore::Array<casacore::Double> itsPlotRange;
    
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
    casacore::Record itsOptionRecord;

    // Stores the TaQL string used by TablePlot to select the data
    // to be plotted.
    casacore::Vector< casacore::Vector<casacore::Vector<casacore::String> > > itsTaqlStrings;
    

    // Store the iteration axes information.
    casacore::Vector<casacore::String> itsIterationAxes;


    // The time range selected as a pair of double values.  Time
    // ranges are applied to the x-axis only, and only for timeplot.
    casacore::Vector< casacore::Double > itsTimeRange;
    

    // Values to keep track of iterative plots.
    //<group>
    casacore::Bool itsIterPlotOn;
    //casacore::Bool itsIterPlot;
    casacore::Vector<casacore::String> itsIterLabels;
    casacore::Vector<casacore::String> itsIterAxes;
    //</group>

    // Variables to keep track of the current data selection
    // for channels and correlations.
    // These are used inside the dataTaQL function to create
    // TaQL strings that reflect this chan, corr selection.
    //<group>
    casacore::Vector<casacore::Vector<casacore::Int> > itsStartChan;
    casacore::Vector<casacore::Vector<casacore::Int> > itsEndChan;
    casacore::Vector<casacore::Vector<casacore::Int> > itsStepChan;
    casacore::Vector<casacore::String> itsAverage;
    casacore::String itsStepOrSlice;
    casacore::Vector<casacore::Vector<casacore::Int> > itsStartCorr;
    casacore::Vector<casacore::Vector<casacore::Int> > itsEndCorr;
    casacore::Vector<casacore::Vector<casacore::Int> > itsStepCorr;
    //</group>


  //#! Private Member Functions
    TPGuiCallBackHooks *itsCallBack;
    TPResetCallBack *itsResetCallBack;
            
    casacore::Bool itsInitialized;
    casacore::Bool itsMsIsOpen;
        
    
    // Flags to decide when to send a casacore::Table into TablePlot.
    // For performance reasons, do not re-set a Table
    // if there is no need to.
    casacore::Bool itsIsNewSelection;
    casacore::Bool itsIsSubTablePlot;
    casacore::Bool itsIsIterationPlot;
    casacore::Bool itsIsInIterPlot;
    casacore::Bool itsConvertClassSet;
    casacore::Bool itsSpwIterOn;
    casacore::Bool itsIsUVWPlot;
    
    
    casacore::String defaultDataSelectString();
    casacore::String defaultSpectralSelectString();
    

    void resetDataSelection();
    void resetTableSelection();

    casacore::Bool makeDataSelectStr( casacore::MSSelection& MSSelecObj,
	    const casacore::Vector< casacore::Vector<casacore::String> >& selectedCorrs,  
	    const casacore::String& selectedTimes );
    casacore::Bool corrSelection( const casacore::String& correlation, casacore::Vector< casacore::Vector<casacore::String> >& corrNames );
    casacore::Bool corrAndStokesParser( const casacore::String& corrExpr, casacore::Vector< casacore::Vector<casacore::String> >& names );
    casacore::Bool validateCorrAndStokes( casacore::Vector< casacore::Vector<casacore::String> >& names );
    casacore::Bool makeCorrIdLists( casacore::Vector< casacore::Vector<casacore::Int> >& ids );
    casacore::Bool getChannelInfo( casacore::Matrix<casacore::Int> chanSelectList );
    casacore::Bool setSingleChannelSet( casacore::uInt spwId, casacore::Int start, casacore::Int stop, casacore::Int step );
    casacore::String getTimeExprStr( const casacore::String& times, const casacore::String& aveOrStep );
    casacore::String getTimeExprStr( const casacore::String& times );

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
    casacore::Vector<casacore::String> parsePlotRange();
    casacore::Vector<casacore::String> plotrangeToDbls( casacore::Vector<casacore::String> rangeStrVec,
	    casacore::Vector<casacore::Double> rangeVector,
	    casacore::Vector<casacore::Bool> rangeSet );
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
    casacore::Vector<casacore::String> parseStrToList( const casacore::String& inString, casacore::Vector<casacore::String>& outVector );
    

    
    /////////////////////////////////////////////////////////////////////
    // Methods used to figure out what needs to be plotted.

    // Find which table to give to casacore::Table plot, it will be one of three:
    //     1. The whole measurement set
    //     2. The SubMS created with casacore::MSSelection values
    //     3. A SubMS with channel averaging
    //casacore::Table* getTable();

    // Create the TaQL string from the various parameters given
    // to plotxy
    casacore::Bool
	createTaQL( const casacore::String& x,
		const casacore::String& y,
		const casacore::String& xcolumn,
		const casacore::String& ycolumn,
		const casacore::String& xvalue,
		const casacore::String& yvalue,
		casacore::String& title,
		casacore::String& xlabel,
		casacore::String& ylabel );

   casacore::Bool
	createSaQL(
		const casacore::String& x,
		const casacore::String& y,
		const casacore::String& xvalue,
		const casacore::String& yvalue,
		casacore::String& title,
		casacore::String& xlabel,
		casacore::String& ylabel);

    casacore::Vector<casacore::String>
	getTaQL( const casacore::String& axisStr,
	    const casacore::String& column,
	    const casacore::String& value,
	    const casacore::uInt spwId,
	    const casacore::uInt polId,
	    const char axis,
	    casacore::String& label);

    casacore::Bool checkAverageMode( casacore::String optName, casacore::String mode, casacore::String selectExpr );
    casacore::Bool checkAverageMode( casacore::String optName, casacore::String mode );
    
    casacore::Bool checkIterationAxes( const casacore::Vector<casacore::String>& iteration );

    casacore::Bool setSingleInputTable( const casacore::Vector<casacore::String>& sortOrder,
	    const casacore::uInt spwId,
	    const casacore::uInt polId,
	    const casacore::Table table,
	    const casacore::String tableName );
    casacore::Bool setInputTable( const casacore::String& xcolumn, const casacore::String& ycolumn );

    casacore::Bool updatePlotOptions( const casacore::String& title, 
	    const casacore::String& xlabel,
	    const casacore::String& ylabel,
	    const casacore::String& xcolumn,
	    const casacore::String& ycolumn,
	    const casacore::String& x,
	    const casacore::String& y );
    void cleanupOptions();
			      
    //casacore::Bool setSpwLists( casacore::Bool checkShape );
    casacore::Bool checkSpwShapes();
    casacore::Bool checkPolShapes();
    //casacore::Bool setSpwChanNums();

    casacore::Vector<casacore::String> dataTaQL( const casacore::String& column,
                                         const casacore::String& value,
	    				 const casacore::uInt spwId,
	    				 const casacore::uInt polId,
					 casacore::String& label );
    
    
    // Create the vector of XTaQL, YTaQL pairs that TablePlot likes...
    casacore::String getIndices( const casacore::Int& startcorr,
	    const casacore::Int& endcorr,
	    const casacore::Int& stepcorr,
	    const casacore::Int& startchan,
	    const casacore::Int& endchan,
	    const casacore::Int& stepchan,
	    const casacore::String& column,
	    const casacore::String& value,
	    casacore::String& label );

    // Find the conversion value for converting uvdists from meters
    // to kilometers
    casacore::Double getMeterToKlambdaConvertValue( casacore::uInt spwIndex );
    casacore::Bool getDerivedConvertVectors();
    casacore::Bool setupConvertFn( const casacore::String& axisStr, const casacore::Char& axis );
    
    // casacore::Conversion routines which use MSDerived to calculate the
    // conversion values.
    casacore::Vector<casacore::Double>
	msDerivedConvert( const casacore::String& convertType );
    
    
    /////////////////////////////////////////////////////////////////////
    // Misceleanouse methods.
    
    // Verify that the class variables have been iniiatlized.  If they
    // haven't been then initialize them.
    casacore::Bool checkInit();

    // This is needed for flagging.  We need to know if we've made an
    // array plot, or more precisely a subtable plot.
    casacore::Int checkForArrayPlot( casacore::Int subplot );
    

    // Verify that we have openen a measurement set.
    casacore::Bool checkOpenMS();

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


