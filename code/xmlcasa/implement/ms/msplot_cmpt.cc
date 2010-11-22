//# msplot_cmpt.cc:  this defines MsPlot, which handles the plotting of
//#             measurement sets.
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
//# @author  Gary Li & Shannon Jaeger
//# @version 
//////////////////////////////////////////////////////////////////////////////
//Standard Include files
#include <stdio.h>
#include <iostream>
#include <iomanip>


// CASA include files
#include <casa/Logging/LogIO.h>
#include <casa/Exceptions/Error.h>
#include <casa/namespace.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Quanta/Quantum.h>

// msplot include files
//#include <xmlcasa/ms/msplot_private.h>
#include <msplot_cmpt.h>

using namespace std;

namespace casac {


msplot::msplot():
    itsLog(0),
    itsMsPlot(0)
{
    itsLog = new casa::LogIO();
    
    itsMsPlot = new casa::MsPlot();
}

////////////////////////////////////////////////////////////////////////////
msplot::~msplot()
{
  debugFnEntry( "~msplot()" );

  if ( itsLog != NULL ) { delete itsLog; itsLog = NULL; }

  if ( itsMsPlot != NULL ) { delete itsMsPlot; itsMsPlot = NULL; }
  
  debugFnExit( "~msplot()" );
}

/////////////////////////////////////////////////////////////////////////
bool 
msplot::open(const std::string& msname, const bool dovel,
              const std::string& restfreq,
              const std::string& frame,
              const std::string& doppler)

{
  String Fn = "open( const string& msName )";
  debugFnEntry( Fn );
  Bool rstat(False);

  try {
      if ( itsMsPlot == NULL ) 
	  itsMsPlot = new MsPlot();
      
      if ( itsMsPlot->open( String( msname ), dovel, restfreq, frame, doppler ) )
	  rstat = casa::True; 	
  } 
  catch ( casa::AipsError ae) {
      // TODO decide whether we print another message or
      // if we shoule assume a message was already displayed.
      *itsLog << casa::LogIO::SEVERE 
	      << "[ msplot::open()] Exception Reported: " 
	      << ae.getMesg() << casa::LogIO::POST;
      //RETHROW( ae );
  }

  debugFnExit( Fn );

  return rstat;
}

/////////////////////////////////////////////////////////////////////////////////
bool
msplot::clearplot( const int subplot )
{
    String Fn = "clearplot( subplot )";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if ( subplot == 0 )
	{
	    if ( itsMsPlot->clearPlot( 0, 0, 0 ) )
		rstat = True;
	} else {
	    casa::Int ncols, nrows, panel;
	    if ( parseSubplot( subplot, nrows, ncols, panel ) )
		if( itsMsPlot->clearPlot( nrows, ncols, panel ) )
		    rstat = True;
	}
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	    	<< "[msplot::clearplot()] Exception Reported... \n " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}


/////////////////////////////////////////////////////////////////////////////////
bool
msplot::emperorsNewClose()
{
    String Fn = "reset()";
    debugFnEntry( Fn );

    debugFnExit( Fn );
    return True;
}


/////////////////////////////////////////////////////////////////////////////////
bool
msplot::reset( )
{
    String Fn = "reset()";
    debugFnEntry( Fn );

    casa::Bool rstat(casa::False);
    if ( ! checkForOpenMS() ) return rstat;

    try {
	
	if( itsMsPlot != NULL ) {
	    itsMsPlot->reset( False );
	}
	rstat = True;
    } catch (casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	    	<< "msplot::locatedata()] Exception Reported: " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}



/////////////////////////////////////////////////////////////////////////////////
// Close the measurement set.
// Reset the MsPlot object back to its initial state.  The behaviour may need
// to change in the future.  The user might want to maintain the plot options
// that have been set.  They would probably like to keep anything that was
// set via setspectral or setdata, but this will be nearly impossible and 
// it probably doesn't make sense since these selections are based on the data
// and if they've closed the data set then why would the selections stay.
//
// Anyway to keep the plotoptions MsPlot::reset() will need to be split up.
bool
msplot::closeMS( )
{
    String Fn = "resetMS()";
    debugFnEntry( Fn );

    casa::Bool rstat(casa::False);
    if ( ! checkForOpenMS() ) return rstat;

    try {
	// We want to reset the MS so we send True into this method.
	if( itsMsPlot != NULL ) {
	    itsMsPlot->reset( True );
	}
	
	rstat = True;
    } catch (casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	    	<< "msplot::locatedata()] Exception Reported: " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}


/////////////////////////////////////////////////////////////////////////////////
// Destroy the MsPlot object forcing the plotter and everything else to
// go away.
bool
msplot::done()
{
  String Fn = "done()";
  debugFnEntry( Fn );
 
  //done this way can crash subsequent task (plotxy/plotcal)
  //
  //casa::Bool rstat( casa::True );
  //if( itsMsPlot != NULL ) {
  //    //itsMsPlot->reset( True );
  //    delete itsMsPlot;
  //    itsMsPlot = NULL;
  //}
  debugFnExit( Fn );
  //return rstat;

  return closeMS();
}

/////////////////////////////////////////////////////////////////////////////////
// For now the same as done.
bool
msplot::close()
{
  String Fn = "close()";
  debugFnEntry( Fn );
  
  debugFnExit( Fn );
  //done or close cause switch between tool/task crash
  //return done();

  return closeMS();
}

//////////////////////////////////////////////////////////////////////////////
//
bool
msplot::plotoptions(const int subplot, 
	const std::string& plotsymbol, 
	const std::string& plotcolor, 
	const std::string& multicolor, 
	const std::string& plotrange, 
	const char timeplot,
	const double markersize, 
	const double linewidth, 
	const bool overplot, 
	const bool replacetopplot,
	const bool removeoldpanels, 
        const int skipnrows,
        const int averagenrows,
        const std::string& extendflag,
	const std::string& connect,
	const bool showflags, 
        const std::string& title, 
	const std::string& xlabel, 
	const std::string& ylabel, 
	const double fontsize, 
	const double windowsize, 
	const double aspectratio )
{
    String Fn = "msplot::plotoptions()";
    debugFnEntry( Fn );
    
    casa::Bool rstat( casa::True );
    if ( ! checkForOpenMS() ) return rstat=False;
    
    // The record that will hold the full suite of options.
    casa::Record theOptions;
    

    //////////////////////////////////////////////////////////////////////
    // Set the number of panels: number of rows, columns and which one
    // we are to use for plotting.
    // Number of the panels starts in top, left corner and
    casa::Int ncols =1, nrows = 1, panel = 1;
    if ( subplot > 0 && parseSubplot( subplot, nrows, ncols, panel ) )
    {
	theOptions.define( RecordFieldId( "nrows" ), nrows );
	theOptions.define( RecordFieldId( "ncols" ), ncols );
	theOptions.define( RecordFieldId( "panel" ), panel );
	if ( nrows * ncols > 1 )
	{
	    theOptions.define( RecordFieldId( "multipanel" ), True );
	} else {
	    theOptions.define( RecordFieldId( "multipanel" ), False );
	}
    } else if ( subplot != 0 ) {
	// If we are here the parseSubplot failed.
	rstat = False;
    }
    
    
    //////////////////////////////////////////////////////////////////////
    // Plotting information: multicolor if true channels/polarizations
    // appear in different colour, and plot symbol see TablePlot
    // for the allowed syntax.
    
    if ( plotsymbol.length() )
	theOptions.define( RecordFieldId( "plotsymbol" ), plotsymbol );
    if ( plotcolor.length() )
	theOptions.define( RecordFieldId( "color" ), plotcolor );

    if(upcase(multicolor).matches("CHAN"))
            theOptions.define(  RecordFieldId( "multicolour" ), "cellcol" );
    else if(upcase(multicolor).matches("CORR"))
            theOptions.define(  RecordFieldId( "multicolour" ), "cellrow" );
    else theOptions.define(  RecordFieldId( "multicolour" ), multicolor );


    //////////////////////////////////////////////////////////////////////
    // Deal with plotrange and timeplot.
    String timeplot_l;
    switch ( timeplot ) 
    {
	case 'o': timeplot_l = "o"; break;
	case 'x': timeplot_l = "x"; break;
	case 'y': timeplot_l = "y"; break;
	case 'b': timeplot_l = "b"; break;	    
	default: timeplot_l = "o"; break;
    }
    theOptions.define( RecordFieldId( "timeplot" ),timeplot_l );
    if ( plotrange.size() > 0 )
	theOptions.define( RecordFieldId( "plotrange" ), String( plotrange ) );
    

    //////////////////////////////////////////////////////////////////////
    // The size of the lines, or markers that are plotted.
    if ( markersize > 0 )
	theOptions.define( RecordFieldId( "markersize" ), markersize );
    if ( linewidth > 0 )
	theOptions.define( RecordFieldId( "linewidth" ), linewidth );

    //////////////////////////////////////////////////////////////////////
    // An option to all users to select skipping of point plotting.
    // Allows the user to plot every nth point.
    if ( skipnrows > 0 )
	theOptions.define( RecordFieldId( "skipnrows" ), skipnrows );


    //////////////////////////////////////////////////////////////////////
    // An option to all users to select averaging of points
    // other than spectral or correlation averaging.
    // Allows the user to average across baselines and/or time.
    if ( averagenrows > 0 )
	theOptions.define( RecordFieldId( "averagenrows" ), averagenrows );

    //////////////////////////////////////////////////////////////////////
    // An option to all users to select flagging extensions.

    String extIn = String(extendflag);
    String extStr = String(""); 
    //if (downcase(extIn).contains("time")){
    //	extStr += "T";
    //} 
    if (downcase(extIn).contains("chan")) {
	extStr += "C";
    }
    if (downcase(extIn).contains("corr")) {
	extStr += "P";
    }
    //if (downcase(extIn).contains("base")) {
    //	extStr += "B";
    //}
    //if (downcase(extIn).contains("sp")) {
    //	extStr += "W";
    //}
    if (downcase(extIn).contains("true")) {
	extStr += "TRUE";
    }
    theOptions.define(RecordFieldId("extendflag"), extStr);
    //looks ugly on screen, do this in python instead
    //if (extStr.length() > 0) {
    //   *itsLog << casa::LogIO::WARN 
    //	      << "Extended Flagging "  << casa::LogIO::POST;
    //}


    //////////////////////////////////////////////////////////////////////
    // An option to decide how to sort points while plotting.
    // connect = 'row' makes "NChan x NCorr" plots of "NRows" points each.
    // connect = 'chan' makes "NRows" plots of "NChan x NCorr" points each.
    // This is to be used in conjunction with 'averagenrows' and/or
    // 'skipnrows' because it is inefficient if NRows >> (NChan x NCorr)
    // This option allows plotsymbol='-' to join up points in the 
    // correct direction. The default is 'row'.
    String connectStr = String( connect );
    if ( downcase( connectStr ).matches( "time" ) ) {
	theOptions.define( RecordFieldId( "connect" ), String( "tablerow" ) );
    } else if ( downcase( connectStr ).matches( "chan" ) ) {
	theOptions.define( RecordFieldId( "connect" ), String( "cellcol" ) );
    } else if ( downcase( connectStr ).matches( "chan" ) ) {
	theOptions.define( RecordFieldId( "connect" ), String( "cellrow" ) );
    } else {
	theOptions.define( RecordFieldId( "connect" ), connect );
    }



    //////////////////////////////////////////////////////////////////////
    // Overplotting and time plotting, time plot is set automatically
    // based on what the user choses to plot.
    theOptions.define( RecordFieldId( "overplot" ), overplot );
    
    //////////////////////////////////////////////////////////////////////
    // Flagging    
    theOptions.define( RecordFieldId( "showflags" ), showflags );

    // replace-top-plot and remove-old-plot options.
    theOptions.define( RecordFieldId( "replacetopplot" ), replacetopplot );
    theOptions.define( RecordFieldId( "removeoldpanels" ), removeoldpanels );

    //////////////////////////////////////////////////////////////////////
    // Window type options -- size, aspectratio
    if ( windowsize > 0 )
	theOptions.define( RecordFieldId( "windowsize" ), Double(windowsize) );
    if ( aspectratio > 0 )
	theOptions.define( RecordFieldId( "aspectratio" ), Double(aspectratio) );
    //    if ( fontsize > 0 )
	theOptions.define( RecordFieldId( "fontsize" ), Double(fontsize) );


    // The options that control the contents of the title and 
    // labels along the x and y axies
    //if ( !title.compare( "" ) )
	theOptions.define( RecordFieldId( "title" ), title );
    //if ( !xlabel.compare( "" ) )
	theOptions.define( RecordFieldId( "xlabel" ), xlabel );
    //if ( !ylabel.compare( "" ) )
	theOptions.define( RecordFieldId( "ylabel" ), ylabel );

    try {
	if ( rstat && itsMsPlot->setplotoptions( theOptions ) )
	    rstat = casa::True;
	else
	    rstat = casa::False;
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
		<< "[msplot:: plotoptions()] Exception Reported: " 
		<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}

/////////////////////////////////////////////////////////////////////////////////
// Call the MsPlot class to display a summary, data description of
// the open measurment set.  We either show what the "selected" data
// is or what is contained in the MS.
//
bool
msplot::summary( bool selected )
{
    casa::Bool rstat(casa::True);

    String Fn = "summary( selected? )";
    debugFnEntry( Fn );

    if ( ! checkForOpenMS() ) return rstat;
    
    try {
	if ( rstat && !itsMsPlot->summary( selected ) )
	    rstat = casa::False;
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
		<< "[msplot:: summary()] Exception Reported: " 
		<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}
    

////////////////////////////////////////////////////////////////////////////
bool
msplot::extendflag( const std::string& extendcorr,
		 const std::string& extendchan,
		 const std::string& extendspw,
		 const std::string& extendant,
		 const std::string& extendtime)
{
  casa::Bool rstat(casa::False);

  String Fn = "extendflag( .... )";
  debugFnEntry( Fn );

  if (!checkForOpenMS()) 
     return rstat;

  //cout << "extendcorr=" << extendcorr << " extendchan=" << extendchan
  //       << " extendspw=" << extendant << " extendant=" << extendant
  //       << " extendtime=" << extendtime << endl;
  try {
     if (!itsMsPlot->setFlagExtension( String( extendcorr ),
			       String( extendchan ),
			       String( extendspw ),
			       String( extendant ),
                               String( extendtime )))
     {
	  rstat = False;
      } else {
	  rstat = True;	
      }
  } catch ( casa::AipsError ae ) {
      *itsLog << casa::LogIO::SEVERE 
	      << "[msplot::extendflat()] Exception Reported: " 
	      << ae.getMesg() << casa::LogIO::POST;
      //RETHROW( ae );
      rstat = False;
  }

  
  debugFnExit( Fn );
  return rstat;
}

////////////////////////////////////////////////////////////////////////////
bool
msplot::setdata( const std::string& baseline,
		 const std::string& field,
		 const std::string& scan,
		 const std::string& uvrange,
		 const std::string& array,
		 const std::string& feed,
		 const std::string& spw,
                 const std::string& correlation,
		 const std::string& time)
{
  casa::Bool rstat(casa::False);

  String Fn = "setdata( baseline, field, .... )";
  debugFnEntry( Fn );

  if (!checkForOpenMS()) 
     return rstat;

  //cout << "scan=" << scan << " uvrange=" << uvrange << endl;
  try {
     if (!itsMsPlot->setData( String( baseline ),
			       String( field ),
			       String( uvrange ),
			       String( scan ),
                               String( array ),
			       String( feed ),
       			       String( spw ),
       			       String( correlation ),
 				String( time ) ) )
     {
	  // Note we are assuming that any error msgs have been
	  // printed by MsPlot::setdata(), it may be useful to
	  // put one here just incase there isn't one.
	  
	  rstat = False;
      } else {
	  rstat = True;	
      }
  } catch ( casa::AipsError ae ) {
      *itsLog << casa::LogIO::SEVERE 
	      << "[msplot::setdata()] Exception Reported: " 
	      << ae.getMesg() << casa::LogIO::POST;
      //RETHROW( ae );
      rstat = False;
  }

  
  debugFnExit( Fn );
  return rstat;
}


////////////////////////////////////////////////////////////////////////////
// SDJ Why do we need a data column here?
bool
msplot::avedata( const std::string& chanavemode,
                 const std::string& corravemode,
		 const std::string& datacolumn,
                 const std::string& averagemode,
                 const std::string& averagechan,
                 const std::string& averagetime,
                 const bool averageflagged,
                 const bool averagescan,
                 const bool averagebl,
                 const bool averagearray,
                 const bool averagechanid,
                 const bool averagevel)
{
  casa::Bool rstat(casa::False);

  String Fn = "average( datacolumn, averagemode, ... )";
  debugFnEntry( Fn );

  if (!checkForOpenMS()) 
     return rstat;

  try {
     if (!itsMsPlot->average(  String( chanavemode ),
                               String( corravemode ),
                               String( datacolumn ),
                               String( averagemode ),
                               String( averagechan ),
			       String( averagetime ),
                               averageflagged,
                               averagescan,
                               averagebl,
                               averagearray,
                               averagechanid,
                               averagevel))
			      
      {
	  // Note we are assuming that any error msgs have been
	  // printed by MsPlot::setdata(), it may be useful to
	  // put one here just incase there isn't one.
	  
	  rstat = False;
      } else {
	  rstat = True;	
      }
  } catch ( casa::AipsError ae ) {
      *itsLog << casa::LogIO::SEVERE 
	      << "[msplot::average()] Exception Reported: " 
	      << ae.getMesg() << casa::LogIO::POST;
      //RETHROW( ae );
      rstat = False;
  }
  
  debugFnExit( Fn );
  return rstat;
}


/////////////////////////////////////////////////////////////////////////////////
bool
msplot::checkplotxy( const std::string& x, 
		const std::string& y, 
                const std::string& xcolumn, 
                const std::string& ycolumn, 
		const std::string& xvalue,
		const std::string& yvalue,
		const std::vector<std::string>& iteration )
{
    String Fn = "checkplotxy( x, y, xcolumn, ycolumn, xvalue, yvalue, itereation )";
    debugFnEntry( Fn );

    casa::Bool rstat(casa::False);
    if ( ! checkForOpenMS() ) return rstat;

    try {
	casa::Vector<casa::String> l_iteration = casa::toVectorString( iteration );
	if ( itsMsPlot->plotxy( True, x, y, xcolumn, ycolumn, xvalue, yvalue, l_iteration ) )
	    rstat = casa::True;
    } catch (casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE << "[ msplot::checkplotxy() ] Exception Reported: " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }
    
    debugFnExit( Fn );
    return rstat;
}


/////////////////////////////////////////////////////////////////////////////////
bool
msplot::plotxy( const std::string& x, 
		const std::string& y, 
                const std::string& xcolumn, 
                const std::string& ycolumn, 
		const std::string& xvalue,
		const std::string& yvalue,
                const std::vector<std::string>& iteration )
{
    String Fn = "plotxy( x, y, xcolumn, ycolumn, xvalue, yvalue, iteration )";
    debugFnEntry( Fn );
   
    casa::Bool rstat(casa::False);
    if ( ! checkForOpenMS() ) return rstat;

    try {
	casa::Vector<casa::String> l_iteration = casa::toVectorString( iteration );
	if ( itsMsPlot->plotxy( False, x, y, xcolumn, ycolumn, xvalue, yvalue, l_iteration ) )
	    rstat = casa::True;
    } catch (casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE << "[ msplot::plotxy() ] Exception Reported: " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }
    
    debugFnExit( Fn );
    return rstat;
}

//////////////////////////////////////////////////////////////////////////////////
bool
msplot::plot( const std::string& type,
    const std::string& column, 
    const std::string& value,
    const std::vector<std::string>& iteration
)
{
    String Fn = "plot( type, column, value, iteration )";
    debugFnEntry( Fn );
    
    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	rstat = plotter( type, column, value, iteration, False );
    } catch ( casa::AipsError ae) {
	*itsLog << casa::LogIO::SEVERE 
	        <<  "[msplot::plot()] Exception Reported: " 
	        << ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}

//////////////////////////////////////////////////////////////////////////////////
bool
msplot::checkplot( const std::string& type,
    const std::string& column, 
    const std::string& value,
    const std::vector<std::string>& iteration 
)
{
    String Fn = "checkplot()";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	rstat = plotter( type, column, value, iteration, True );
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	        <<  "[msplot::checkplot()] Exception Reported: " 
	        << ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}


//////////////////////////////////////////////////////////////////////////////
bool
msplot::iterplotstart( )
{
    String Fn = "iterplotstart( )";
    debugFnEntry( Fn );
    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if( itsMsPlot->iterPlotNext( ) ) 
	    rstat = True; 
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	    	<< "[ msplot::iterplotstart() ] Exception Reported: " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}


/////////////////////////////////////////////////////////////////////////////////
bool
msplot::iterplotnext()
{
    String Fn = "iterplotnext()";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if ( itsMsPlot->iterPlotNext() ) {
	    rstat = True; 
	}
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	    	<< "[ msplot::iterplotnext() ] Exception Reported: " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}

//////////////////////////////////////////////////////////////////////////////////
bool
msplot::iterplotstop( const bool rmplotter )
{
    String Fn = "iterplotstop( rmplotter )";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if ( itsMsPlot->iterPlotStop( rmplotter ) ) {
	    rstat = True; 
	} else {
	    rstat = False;
	}
	
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
		<< "[ msplot::iterplotstop() ] Exception Reported: " 
		<< ae.getMesg() << casa::LogIO::POST;
	rstat = casa::False;
    }

    debugFnExit( Fn );
    return rstat;
}

//////////////////////////////////////////////////////////////////////////////////
bool
msplot::savefig( const std::string& filename,
                const int dpi,
		const std::string& orientation,
		const std::string& papertype,
		const std::string& facecolor,
		const std::string& edgecolor )
{
    String Fn = "savefig( filename, dpi, orientation, papertype, facecolor, edgecolor )" ;
    
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	rstat = itsMsPlot->saveFigure( String( filename ), Int( dpi ),
		   String( orientation ), String( papertype ), 
		   String( facecolor ), String( edgecolor ) );
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
		<< LogOrigin( "msplot", "savefig" )
		<< "Exception Reported: " 
		<< ae.getMesg() << casa::LogIO::POST;
	rstat = casa::False;
    }

    debugFnExit( Fn );
    return rstat;
}
    
/////////////////////////////////////////////////////////////////////////////////
bool
msplot::markregion( const int subplot, 
		   const std::vector<double>& region )
{
    String Fn = "markregion( nrows, ncols, panel, region )";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::True );
    if ( ! checkForOpenMS() ) return rstat;

    Int nrows, ncols, panel;
    if ( !parseSubplot( subplot, nrows, ncols, panel ) )
	rstat = False;

    try {
	casa::Vector<casa::Double> theRegion(region.size());
	for(unsigned int i=0;i<region.size();i++)
	    theRegion[i] = region[i];

	// Note: we assume that if markRegion fails a flag will be thrown
	// as this method always returns 0.
	if( rstat )
	    if ( !itsMsPlot->markRegion( nrows, ncols, panel, theRegion ) )
		rstat = False;
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	    	<< "[ msplot::markregion()] Exception Reported: " << ae.getMesg() 
	    	<< casa::LogIO::POST;
	//RETHROW( ae );
	rstat = False;
    }

  debugFnExit( Fn );
  return rstat;
}

/////////////////////////////////////////////////////////////////////////////////
bool
msplot::flagdata()
{
    String Fn = "flagdata()";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if ( itsMsPlot->flagData( casa::FLAG ) )
	    rstat = True;
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	    	<< "[msplot::flagdata()] Exception Reported: " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}

/////////////////////////////////////////////////////////////////////////////////
bool
msplot::unflagdata()
{
    String Fn = "clearflags()";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if ( itsMsPlot->flagData( casa::UNFLAG ) )
	    rstat = True;
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE 
	    	<< "[msplot::flagdata()] Exception Reported: " 
	    	<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

  debugFnExit( Fn );
  return rstat;
}

/////////////////////////////////////////////////////////////////////////////////
bool
msplot::clearflags()
{
    String Fn = "clearflags()";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) 
        return rstat;

    //cout << "Really ?!" << endl;

    try {/*
	if ( itsMsPlot->clearAllFlags() )
	rstat = True;*/
	//cout << "Sorry! Not implemented yet." << endl;
	*itsLog << casa::LogIO::SEVERE << "Sorry! Not implemented yet." 
	    	<< casa::LogIO::POST;
    } catch ( casa::AipsError ae ) {
	*itsLog << casa::LogIO::SEVERE << "Exception Reported: " 
                << ae.getMesg() 
	    	<< casa::LogIO::POST;
	//RETHROW( ae );
    }

  debugFnExit( Fn );
  return rstat;
}

/////////////////////////////////////////////////////////////////////////////////
// For now this method accepts no arguments.  This is likely to change!
// This is a first version of this method.
bool
msplot::locatedata()
{
    String Fn = "locatedata()";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if ( itsMsPlot->locateData() )
	    rstat = True;
    } catch (AipsError ae) {
	*itsLog << casa::LogIO::SEVERE << "Exception Reported: " 
		<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}

/////////////////////////////////////////////////////////////////////////////////


bool
msplot::saveflagversion(const std::string& versionname, const std::string& comment, const std::string& merge)
{
    String Fn = "saveFlagVersion( versionname, comment, merge)";
    debugFnEntry( Fn );
    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if ( itsMsPlot->saveFlagVersion( String(versionname), String(comment), String(merge) ) )
	    rstat = True;
    } catch (AipsError ae) {
	*itsLog << casa::LogIO::SEVERE << "Exception Reported: " 
		<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;
}

bool
msplot::restoreflagversion(const std::vector<std::string>& versionname, const std::string& merge)
{
    String Fn = "restoreFlagVersion( versionname, merge)";
    debugFnEntry( Fn );
    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	casa::Vector<casa::String> l_versionname = casa::toVectorString( versionname );
	if ( itsMsPlot->restoreFlagVersion( l_versionname, String(merge) ) )
	    rstat = True;
    } catch (AipsError ae) {
	*itsLog << casa::LogIO::SEVERE << "Exception Reported: " 
		<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;

}

bool
msplot::deleteflagversion(const std::vector<std::string>& versionname)
{
    String Fn = "deleteFlagVersion( versionname )";
    debugFnEntry( Fn );
    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	casa::Vector<casa::String> l_versionname = casa::toVectorString( versionname );
	if ( itsMsPlot->deleteFlagVersion( l_versionname ) )
	    rstat = True;
    } catch (AipsError ae) {
	*itsLog << casa::LogIO::SEVERE << "Exception Reported: " 
		<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;

}

bool
msplot::getflagversionlist()
{
    String Fn = "getFlagVersionList( )";
    debugFnEntry( Fn );
    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    try {
	if ( itsMsPlot->getFlagVersionList() )
	    rstat = True;
    } catch (AipsError ae) {
	*itsLog << casa::LogIO::SEVERE << "Exception Reported: " 	
		<< ae.getMesg() << casa::LogIO::POST;
	//RETHROW( ae );
    }

    debugFnExit( Fn );
    return rstat;

}

// private methods:

//////////////////////////////////////////////////////////////////////////////////
// TODO add a way of flipping the axis.  Easy to do, for the most part
// just send in the values to the ycolumns, yvalues instead of the x ones
//
casa::Bool
msplot::plotter( const std::string& type,
    const std::string& column, 
    const std::string& value,
    const std::vector<std::string>& iteration,
    casa::Bool checkOnly
)
{
    String Fn = "plotter()";
    debugFnEntry( Fn );
    //cout << "msplot::plotter----type=" << type << " column=" << column 
    //     << " value=" << value << endl;
    casa::Bool rstat( casa::False );
    if ( ! checkForOpenMS() ) return rstat;

    // Create local vars that are easy to handle
    casa::Vector<casa::String> l_iteration = casa::toVectorString( iteration );
    casa::String l_type = String( type );
    casa::String l_column = String( column );
    casa::String l_value = String( value );
    
    // Check the params to verify usage with given plot type
    // Note that itsMsPlot::plotxy calls the check routine so to avoid doing
    // the checks twice we call plotxy only when we are plotting and not
    // just checking things.
    checkPlotParams( l_type, l_column, l_value, l_iteration );
        
    if ( downcase( type ).matches( String( "array" ) ) )
    {
	if ( itsMsPlot->plotxy( checkOnly, "XEAST", "YNORTH", "array", "array", "", "", l_iteration ) )
	    rstat = True;

    } else if ( downcase(l_type).matches( String( "azimuth" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "azimuth", "data", l_column, l_column, l_value, l_value,  l_iteration ) )
	    rstat = casa::True;

    } else if ( downcase(l_type).matches( String( "baseline" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "baseline", "data", l_column, l_column, l_value, l_value, l_iteration ) )
	    rstat = True;

    } else if ( downcase(l_type).matches( String( "elevation" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "elevation", "data", l_column, l_column, l_value, l_value, l_iteration ) )
		rstat = casa::True;

    } else if ( downcase(l_type).matches( String( "hourangle" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "hourangle", "data", l_column, l_column, l_value, l_value, l_iteration ) )
	    rstat = casa::True;

    } else if ( downcase(l_type).matches( String( "parallacticangle" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "parallacticangle", "data", l_column, l_column, l_value, l_value, l_iteration ) )
	    rstat = casa::True;

    } else if ( downcase(l_type).matches( String( "uvcoverage" ) ) ||
         downcase(l_type).matches( String( "uvcover" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "u-u", "v-v", "", "", "", "", l_iteration ) )
		rstat = casa::True;

    } else if ( downcase(l_type).matches( String( "uvdist" ) ) ||
          downcase(l_type).matches( String( "uvdistance" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "uvdist", "data", l_column, l_column, l_value, l_value, l_iteration ) )
		rstat = True;

    } else if ( downcase(l_type).matches( String( "vischannel" ) ) ||
            downcase(l_type).matches( String( "vischan" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "channel", "data", l_column, l_column, l_value, l_value, l_iteration ) )
		rstat = True;
    } else if ( downcase(l_type).matches( String( "viscorrelation" ) ) 
                 || downcase(l_type).matches( String( "viscorr" ) ) ) { 
	if ( itsMsPlot->plotxy( checkOnly, "correlation", "data", l_column, l_column, l_value, l_value, l_iteration ) )
		rstat = True;
    } else if ( downcase(l_type).matches( String( "visfrequency" ) ) ||
            downcase(l_type).matches( String( "visfreq" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "chan_freq", "data", l_column, l_column, l_value, l_value, l_iteration ) )
		rstat = True;
    } else if ( downcase(l_type).matches( String( "visvelocity" ) ) ||
            downcase(l_type).matches( String( "visvelocity" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "chan_velocity", "data", l_column, l_column, l_value, l_value, l_iteration ) )
		rstat = True;

    } else if ( downcase(l_type).matches( String( "vistime" ) ) ) {
	if ( itsMsPlot->plotxy( checkOnly, "time", "data", l_column, l_column, l_value, l_value, l_iteration ) )
		rstat = casa::True;
    } else if ( downcase(l_type).matches( String( "weight" ) ) ) {
	// TODO add an error, warning if a column is given
	if ( itsMsPlot->plotxy( checkOnly, "default_weight", "data", "", l_column, "", l_value, l_iteration ) )
		rstat = casa::True;
    } else {
	*itsLog << LogIO::SEVERE
		<< "Unrecognized plot type: " << type
		<< ". Valid values are: array, azimuth, baseline, "
		<< "elevation, hourangle, parallacticangle, "
		<< " uvcoverage, uvdist, viscorr, vischannel, visfreq, "
                << " vistime, visvelocity, and weight."
		<< LogIO::POST;
	rstat = False;
    }

    debugFnExit( Fn );
    return rstat;
}

/////////////////////////////////////////////////////////////////////////////////
// Some of the plot types don't have full support.  This method warns
// user about using parameters that aren't supported for a particular
// plot type.
//
casa::Bool 
msplot::checkPlotParams( casa::String& type,
    casa::String &column, 
    casa::String &value,
    Vector<casa::String>& iteration 
)
{
    String Fn = "checkPlotParams( type, column, value, iteration )";
    debugFnEntry( Fn );

    casa::Bool rstat( casa::True );
    if ( ! checkForOpenMS() ) return rstat=False;

    // TODO this checks should not be done if the plot command is used.
    // For the time being they are commented out.
    if ( type.matches( "array" ) ||
          type.matches( "uvcoverage" ) )
    {
    /*

	if ( column.length() > 0 )
	{
	    *itsLog << LogIO::WARN
		    << "'column' parameter is not supported with the "
		    << type << " plot type."
		    << LogIO::POST;
	    rstat = False;
	}

	if ( value.length() > 0 )
	{
	    *itsLog << LogIO::WARN
		    << "'value' parameter is not supported with the "
		    << type << " plot type."
		    << LogIO::POST;
	    rstat = False;
	}
	    
	if ( iteration.nelements() > 0 && iteration[0].length() > 0 ) 
	{
	    *itsLog << LogIO::WARN
		    << "'iteration' parameter is not supported with the "
		    << type << " plot type."
		    << LogIO::POST;
	    rstat = False;
	}
    */
    } else {
	if ( column.length() < 1 )
	{
	    column.resize( 4 );
	    column = String( "data" );
	}

	if ( value.length() < 1 )
	{
	    value = String( "amp" );
	}
	
    }

    
    debugFnExit( Fn );
    return rstat;
}

/////////////////////////////////////////////////////////////////////////////////
// This is used to parse the subplot parameter, which is a three digit 
// integer, in plotoption(s). The format currently supported is as follows:
// subplot=rcp
//         where r = the number of plot rows
//               c = the number of plot columns
//               p = the panel number, panel numbers start at 1 (top/left)
//                   increase across the rows first.
//
// Note that we are currently limited, r, c, and p must all be less then
//      or equal to 9.  We may generalize it if needed to support more then
//      9 panels.
casa::Bool 
msplot::parseSubplot( const casa::Int subplot, 
		     casa::Int& nrows, 
		     casa::Int& ncols, 
		     casa::Int& panel )
{
  String Fn = "parseSubplot( subplot, nrows, ncols, panel )";
  debugFnEntry( Fn );

  Bool rstat( True );

  casa::String threeChars = casa::String::toString( subplot ); 
  nrows = atoi((threeChars.at(0,1)).chars());
  ncols = atoi((threeChars.at(1,1)).chars());
  panel = atoi((threeChars.substr(2)).chars());

  // Check to make sure r, c, and p are all values between 1 and 9
  if ( subplot < 111 || subplot > 9981 ) 
  {
    *itsLog << casa::LogIO::SEVERE 
	    << "Invalid value for subplot: " << threeChars
	    << ". Subplot values must be\nbetween 111 and 9981."
	    << casa::LogIO::POST;
    rstat = False;
  }

  if( ncols > 9) {
    *itsLog << casa::LogIO::SEVERE
            << "The maximum number of columns (the second digit) is 9."
            << casa::LogIO::POST;
    rstat = False;
  }
  if ( nrows > 9 )
  {
    *itsLog << casa::LogIO::SEVERE
            << "The maximum number of rows (the first digit is 9."
            << casa::LogIO::POST;
    rstat = False;
  }
    //*itsLog << casa::LogIO::SEVERE 
    //	    << "The total number of panels (nrows x ncols) is: "
    //	    << ncols*nrows
    //	    << " which is\n greater then 9.  Sorry msplot supports"
    //	    << " only 9 panels currently."
    //	    << casa::LogIO::POST;

  debugFnExit( Fn );
  return rstat;
}
//////////////////////////////////////////////////////////////////////////////
Bool
msplot::checkForOpenMS()
{
  String Fn = "checkForOpenMS()";
  debugFnEntry( Fn );  

  // TODO this needs to query itsMsPlot to see if it has
  // an open measurement set?
  // Or maybe we don't need this method at all.
  casa::Bool rstat( casa::True );

  if( itsMsPlot == NULL )
  {
    *itsLog << casa::LogIO::SEVERE 
	    <<  "No measurement set has been opened, please run mp.open()." 
	    << casa::LogIO::POST;
    rstat = False;
  }

  debugFnExit( Fn );
  return rstat;
}

//////////////////////////////////////////////////////////////////////////////
void
msplot::debugFnEntry( String Fn )
{
    if(itsLog){
	*itsLog << LogIO::DEBUGGING
		<< "Entered Fn: msplot::" << Fn;
	if ( itsMsPlot != NULL )
	    *itsLog << "\n\tworking on Measurment Set "; //<< .tableName();

	*itsLog << LogIO::POST;  
    }
}
    //TODO Need to save the measurment set name so we can print it out.


//////////////////////////////////////////////////////////////////////////////
void
msplot::debugFnExit( String Fn )
{
    if(itsLog){
	*itsLog << LogIO::DEBUGGING
		<< "Leaving Fn: msplot::" << Fn;
	if (  itsMsPlot != NULL )
	    *itsLog << "\n\tworking on Measurment Set ";// << m_ms.tableName();
  
	*itsLog << LogIO::POST;
    }
}

/////////////////////////////////////////////////////////////////////////////////
} // casac namespace


