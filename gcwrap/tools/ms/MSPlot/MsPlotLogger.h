//# MsPlot.h: this defines a message logger for the casacore::MS plotting classes.
//# Copyright (C) 2007
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
//# 11/14/2007  S. Jaeger   Fixed a typo.


#ifndef CASA_MSPLOT_LOGGER_H
#define CASA_MSPLOT_LOGGER_H

//# Include files
#include <casa/Logging/LogIO.h>
#include <casa/BasicSL/String.h>
  
#define LOG0 0
// <summary>
// A wrapper around the CASA logging facilities for the casacore::MS Plotter
// </summary>

// <use visibility=local>   or   <use visibility=export>
// <use visibility=local>


// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
//#! for example:
//#!  <reviewed reviewer="pshannon@nrao.edu" date="1994/10/10" tests="tMyClass, t1MyClass" demos="dMyClass, d1MyClass">
//#!  </reviewed>
// </reviewed>

// <prerequisite>
//#! Classes or concepts you should understand before using this class.
//   <li> CASA::LogIO
// </prerequisite>
//
// <etymology>
// This class simplifies some of the typing needed in the casacore::MS Plot classes
// to make the code more readable.
// </etymology>
//
// <synopsis>
// This class provides some set debugging information for the entry and
// exit of each method, this can be very useful in determining where an
// error is occuring.
//
// It also provides a method reduces some of the details that are
// needed for messages to the logger in general.
//
// It might be useful to put this class at a higher-level for all CASA
// classes to use, but for now it is here.
// </synopsis>
//
// <example>
// This example shows how to create and use the MSLogger class.
// <srcblock>
// //Create the MSLogger class
// MsPlotLogger *log = new MsPlotLogger( "MsPlot" );
//
// // Register a debug message indicating entering and exiting a method.
// log->fnEnter( "myFunc", "myFunc( arg1, arg2 )" );
// log->setFnName( "myFunction" );
//
// // Log an error, standard, and debug message
// log->logMessage( "Error: incorrect input", "casacore::LogMessage::ERROR", true );
// log->logMessage( "Some earth shattering results", casacore::LogMessage::NORMAL );
// log->debugMessage( "Cool we've found a result!" );
//
// Use the << operator to log a INFO3 message.
// *log << casacore::LogOrigin( "myClass", "myMethod" )
//      << casacore::LogMessage::NORMAL3
//      << "Created casacore::Table Query Strings: " << tblQueryStrs
//      << casacore::LogIO::POST;
// </srcblock>
// </example>
//
// <motivation>
// I got tired of typing the same things over and over again.
//
// Also wanted to add some consistency to the messaging, for example to
// make sure the casacore::LogOrigin was set.    
// </motivation>
//
// <templating arg=T>
// </templating>
//
// <thrown>
//    <li> AIPSError
//    <li>
// </thrown>
//
// <todo asof="2007/10/29">
//   <li> Look at it making it generic for all CASA 
//   <li> Perhaps provide ways of "streaming" the error messages
//        instead of making it all a single String.    
// </todo>
namespace casa { //# NAMESPACE CASA - BEGIN
  

class MsPlotLogger : public casa::LogIO
{
 public:
    //#! Constructors
    // Simple constructor, assumes there is no Class to be added to the
    // message origin.
    MsPlotLogger( const casacore::String& classNm, const casacore::String FnName )
    {
	itsClassNm=classNm;
	itsFnName=FnName;
#if LOG0
	origin( casacore::LogOrigin( "MsPlotLogger", "constructor" ) );
	priority( casacore::LogMessage::DEBUG1 );
	output() << "Created MsPlotLogger for class: " << itsClassNm;
	post();
#endif
    };

    // Constructor. The Class name is given, which will be used for
    // all messages created.
    MsPlotLogger( const casacore::String& classNm )
    {
	itsClassNm=classNm;
	itsFnName="";
#if LOG0
	origin( casacore::LogOrigin( "MsPlotLogger", "constructor" ) );
	priority( casacore::LogMessage::DEBUG1 );
	output() << "Created MsPlotLogger for class: " << itsClassNm;
	post();
#endif
    };

    // Simple constructor, assumes there is no Class to be added to the
    // message origin.
    MsPlotLogger() 
    {
	itsClassNm="";
	itsFnName="";
#if LOG0
	origin( casacore::LogOrigin( "MsPlotLogger", "constructor" ) );
	priority( casacore::LogMessage::DEBUG1 );
	output() << "Created MsPlotLogger";
	post();
#endif
    };

    // Destructor
    ~MsPlotLogger()
    {
#if LOG0
	origin( casacore::LogOrigin( "MsPlotLogger", "constructor" ) );
	priority( casacore::LogMessage::DEBUG1 );
	output() << "Destroying MSLogger for class " << itsClassNm;
	post();
#endif
    };

    //#!//////////////////////////////////////////////////////////////////////
    // <group>
    // Methods to set and get the class name used with this logger.
    void setClassName( casacore::String classNm ) { itsClassNm=classNm; };
    casacore::String getClassName() { return itsClassNm; };
    //</group>


    //#!//////////////////////////////////////////////////////////////////////
    // <group>
    // Methods to set and get the function name associated with logged messages.
    void setFnName( casacore::String fnName ) { itsFnName=fnName; };
    casacore::String getFnName() { return itsFnName; };
    //</group>


    //#  ///////////////////////////////////////////////////////////////////
    // Generic debug message added to log when entering a function
    void
    fnEnter( casacore::String fnName, casacore::String fnCall )
    {
	itsFnName = fnName;
	origin( casacore::LogOrigin( itsClassNm, fnName ) );
	priority( casacore::LogMessage::DEBUG1 );
	output() << "Entered casacore::Function: ";
	output() << (const char*)fnCall.c_str();
	post();
    };

    //# ///////////////////////////////////////////////////////////////////
    // <group>
    // Generic debug message added to log when exiting a function
    void
    fnExit( casacore::String fnName)
    {
	origin( casacore::LogOrigin( itsClassNm, fnName ) );
	priority( casacore::LogMessage::DEBUG1 );
	output() << "Exiting casacore::Function: ";
	output() << (const char*)fnName.c_str();
	post();
	itsFnName="";
    };
    
    void
    fnExit()
    {
	fnExit( itsFnName );
    };
    // </group>


    //#!//////////////////////////////////////////////////////////////////////
    //<group>  
    // Log a message to the CASA logging facilities.
    void
    logMessage( const casacore::String &msg, const casacore::String& classname, const casacore::String& fnname,
	    casacore::LogMessage::Priority msglevel=casacore::LogMessage::NORMAL2,
	    casacore::Bool onconsole=false,
	    casacore::Bool throwexcep=false )
    {
	itsClassNm=classname;
	itsFnName=fnname;
	origin( casacore::LogOrigin( classname, fnname ) );
	priority( msglevel );
	output() << (const char *)msg.c_str();
	if ( onconsole )
	    localSink().cerrToo( true );
	
	if ( throwexcep  )
	    postThenThrow();
	else
	    post();

	if ( onconsole )
	    localSink().cerrToo( false );
    };

    void
    logMessage( const casacore::String &msg, const casacore::String& fnname,
	    casacore::LogMessage::Priority msglevel=casacore::LogMessage::NORMAL2,
	    casacore::Bool onconsole=false,
	    casacore::Bool throwexcep=false )
    {
	logMessage( msg, itsClassNm, fnname, msglevel, onconsole, throwexcep );
    };

    void
    logMessage( const casacore::String &msg,
	    casacore::LogMessage::Priority msglevel=casacore::LogMessage::NORMAL2,
	    casacore::Bool onconsole=false,
	    casacore::Bool throwexcep=false )
    {
	logMessage( msg, itsClassNm, itsFnName, msglevel, onconsole, throwexcep );
    };
    // </group>

    //#!//////////////////////////////////////////////////////////////////////
    // <group>
    // Log a debugging message to the CASA logging facilities.
    void
    debugMessage( const casacore::String &msg, const casacore::String& classname,
	    const casacore::String& fnname,
	    casacore::LogMessage::Priority msglevel=casacore::LogMessage::DEBUGGING )
    {
	itsClassNm=classname;
	itsFnName=fnname;

	logMessage( msg, classname, fnname, msglevel, false, false );
    };

    void
    debugMessage( const casacore::String &msg, const casacore::String& fnname,
	    casacore::LogMessage::Priority msglevel=casacore::LogMessage::DEBUGGING )
    {
	debugMessage( msg, itsClassNm, fnname, msglevel );
    };
    
    void
    debugMessage( const casacore::String &msg,
	    casacore::LogMessage::Priority msglevel=casacore::LogMessage::DEBUGGING )
    {
	debugMessage( msg, itsClassNm, itsFnName, msglevel );
    };
    // </group>
    
 private:
    //#! casacore::Data Members
    // Name of the class the msgs. belong to.
    casacore::String itsClassNm;

    // Name of the function/method the msgs. belong to.
    casacore::String itsFnName;

    
};  // class MsPlotLogger;
} //# NAMESPACE CASA - END


#endif
