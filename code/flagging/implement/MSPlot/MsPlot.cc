//# MsPlot.cc:  Plotting of measurement sets.
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
//# $Id: MsPlot.cc,v 1.9.2.61 2006/10/12 18:22:01 sjaeger Exp $
//#////////////////////////////////////////////////////////////////////////////
//# -------------------------------------------------------------------------
//# Change Log
//# 10/31/2007  S. Jaeger   Added message leves, new logger controls
//# 11/09/2007  S. Jaeger   Added support for giving times in the plotrange
//#                         Changed the setData() method in two.  One that
//#                         that deals with MSSelection and one for 
//#                         printing the selected data.
//# 11/14/2007  S. Jaeger   Added class documentation and allowing
//#                         time expressions in the plotrange option
//

//# Table and TablePlot includes
#include <tableplot/TablePlot/TablePlot.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/RefRows.h>

//# Local (MsPlot) includes
#include <flagging/MSPlot/MsPlot.h>
#include <flagging/MSPlot/MsPlotConvert.h>
#include <flagging/MSPlot/MsPlotHooks.h>

//# Measurment and table related includes
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSDerivedValues.h>
#include <ms/MeasurementSets/MSRange.h>
#include <ms/MeasurementSets/MSIter.h>

#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MFrequency.h>

#include <msvis/MSVis/MsAverager.h>

//# General CASA includes
#include <casa/Exceptions.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Arrays/ArrayMath.h>

//# System Includes
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <cmath>


#define LOG0 0
#define LOG2 1
//#////////////////////////////////////////////////////////////////////
//#
//# Some Notes on data structures!!!
//# 
//# The Main Characters:
//# itsTableVector - A rather fundamental fellow that really is the
//#                  backbone of MsPot as he holds the data for 
//#                  plotting, but be careful as he has a split 
//#                  personality. His current personality influences many 
//#                  of the other characters. His personalities are:
//#                  1. When not iterating on SPW and not doing any time
//#                     averging.  This fellow is a potentially long Vector 
//#                     of Tables all in the first position of this
//#              Vector<Vector<Table> >
//#                  2. He is a list of several vectors of Tables.  This
//#                     is only in a couple of special cases, such as when
//#                     we are iterating over SPW (possibly SCAN) and doing
//#                     time averaging.
//# itsTableVecNames - This character is rather full of himself, as all he 
//#                 contains is the name of the measurement set, over and
//#                 over again (with one exception).  The single exception
//#                 is when plotting the antenna array he develops a bit
//#                 of amnesia and has no name!  He is kind of a split
//#                 personality two as his structure is the exact same as
//#                 itsTableVector.
//# itsTableVecSpwIds - This fellow is also a split personality in that his
//#                 shape is the same as itsTableVector. But instead of 
//#         being filled with tables he's filled with the Spectral
//#                 window indexes that correspond to the values in the
//#                 itsTableVector. When he is a single column of SPWs, he
//#                 will have different SPWs in that column, but when he is
//#                 several columns of SPWs each column will contain the same
//#                 SPW index all the way down, when the MS has been split
//#                 on spectral window.
//# itsTaqlStrings - Oh, what a story this fellow has to say.  He has much
//#                 influence on itsVectorTables, even if itsVectorTables
//#                 doesn't know it.  The Strings contained in him control
//#                 which bits and pieces of information are plotted in each
//#                 of the tables.  He is shaped the same as itsTableVectors,
//#                 so that each Table has defined for it a Vector of Strings
//#                 that define that data to plot.  Some little tidbits to
//#                 keep in mind when analysing this one.  When he is used
//#                 to specify data the TaQL string looks like:
//#                       "DATA[b:e:s,b:e:s]"
//#                             |---| |---|                   
//#       Specifies correlation  /      \ Specifies channels
//#                (polarization)
//#                             b = beginning
//#                             e = end         
//#                             s = step
//#

namespace casa { //# NAMESPACE CASA - BEGIN

String MsPlot::clname = "MsPlot";

//#////////////////////////////////////////////////////////////////////////////
//# default constructor. In case for some reason, one need to pass a Table 
//# object, not a MeasurementSet to TABS_P, he can use this constructor.
MsPlot::MsPlot() : msa(0), itsResetCallBack(0)
{
    String fnname = "MsPlot";
    log = SLog::slog();
    itsTable    = NULL;
    itsTablePlot = NULL;
    itsMS   = NULL;

    itsMaxSpwId = 0;
    itsMaxPolId  = 0;

    //#itsPyBinder  = NULL;    
    
    //# Initialize the various variables.
    itsInitialized = False;
    initialize();

    //# Indicate that MS is not open yet.
    itsMsIsOpen = False;
}

//#////////////////////////////////////////////////////////////////////////////
//# Another constructor with a ms object, ???
MsPlot::MsPlot( const String& MSPath ) : msa(0), itsResetCallBack(0)
{
    String fnname = "MsPlot";
    log = SLog::slog();
    itsTablePlot = NULL;
    itsTable     = NULL;
    itsMS    = NULL;

    itsMaxSpwId  = 0;
    itsMaxPolId  = 0;

    //#itsPyBinder  = NULL;    

    //# Set MsPlot to its initialized state.
    itsInitialized = False;
    initialize();

    //# Since we have a measurment set name creat the measurment set.
    open( MSPath );
}

//#////////////////////////////////////////////////////////////////////////////
//# destructor 
MsPlot::~MsPlot()
{
    String fnname = "~MsPlot";

    //#if ( itsPyBinder != NULL ) { delete itsPyBinder; itsPyBinder = NULL; }
    if ( itsTablePlot != NULL ) { 
        //# Tell TablePlot I don't exist anymore
        itsTablePlot->clearResetCallBack("MsPlot");
   /*delete itsTablePlot;*/ 
   itsTablePlot=NULL; 
    }
    if ( itsTable != NULL ) { delete itsTable; itsTable = NULL; }

    if ( itsMS != NULL ) { delete itsMS; itsMS = NULL; }     
}

//#////////////////////////////////////////////////////////////////////////////
//#
//# This method is used to set up and define the convert class to
//# be used by TablePlot. TablePlot uses this class to convert, change
//# each value being plotted. In this case we are using it to change
//# the data values to plot wither the azimuth, elevation, hourangle,
//# and parallacticangle.
//#
Bool
MsPlot::setupConvertFn( const String& axisStr, const Char& axis  )
{
    //cout << "axisStr=" << axisStr << " axis=" << axis << endl;
    String FnCall = String( "(" ) 
	+ axisStr + String(", " ) +  axis + String(  " )" );
    String fnname = "setupConvertFn";
    log->FnEnter(fnname + FnCall, clname);
    
    Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { 
        log->FnExit(fnname, clname); 
        return rstat=False; 
    }

    //# Verify correctness of axis string.
    Bool useXAxis;
    if ( axis == 'x' ) {
       useXAxis = True;
    } else if ( axis == 'y' ) {
       useXAxis = False;
    } else {
       log->FnExit(fnname, clname); 
       return rstat=False;
    }

    //cout << "itsSelectedMS=" << itsSelectedMS.tableName() << endl;
    //# First we need to create the values needed by the convert function.
    //# TODO cleanup the msColumns when done with it.
    MSDerivedValues * msDerived = new MSDerivedValues();
    ROMSColumns * msColumns = new ROMSColumns( itsSelectedMS );
    if (  ! upcase( axisStr ).compare( "AZIMUTH" ) 
        || ! upcase( axisStr ).compare( "ELEVATION" ) 
        || ! upcase( axisStr ).compare( "HOURANGLE" ) 
        || ! upcase( axisStr ).compare( "HOUR_ANGLE" ) 
        || ! upcase( axisStr ).compare( "PARALLACTICANGLE" ) 
        || ! upcase( axisStr ).compare( "PARALLACTIC_ANGLE" ) )
    {
	//# TODO move the MsDerived class set up into the convert
	//# class constructors.

	//# Set the antenna information in the MSDerivedValue class.
	const ROMSAntennaColumns & antColumns = msColumns->antenna();
   
	//# this is the average of all antennas. Use observatory position 
	//# instead?
	msDerived->setAntennas( antColumns ); 

	//# the following block is needed by parAngle()
	Int numAnts = antColumns.nrow();
	Vector<String> mount( numAnts );
	for (Int i=0; i < numAnts; i++) {
	    mount(i) = antColumns.mount()(i);
	}
	msDerived->setAntennaMount( mount );

	//#TODO the fieldIds and msDerived will need to be destroyed
	//# when the conversion function is no longer needed.
	//# BIG TODO to turn of the conversion function.
	//#Vector<Int> fieldIds = msColumns.fieldId().getColumn();
	//#MSFieldColumns & fields = msColumns.field();
    } else if( !upcase(axisStr).compare("XEAST") 
           || !upcase(axisStr).compare( "CHANFREQ" ) 
           || !upcase(axisStr).compare( "CHAN_FREQ" )
           || !upcase(axisStr).compare( "CHANNEL_FREQ" )
           || !upcase(axisStr).compare( "CHANNEL_FREQUENCY" )
           || !upcase(axisStr).compare( "FREQUENCY" )
           || !upcase(axisStr).compare( "CHANVELOCITY" ) 
           || !upcase(axisStr).compare( "CHAN_VELOCITY" ) 
           || !upcase(axisStr).compare( "CHANNEL_VELOCITY" )
           || !upcase(axisStr).compare( "VELOCITY" ) ) {
	//# We've done this above, setting the msColumns.
	//# This is just here so we don't cause the function
	//# to return prematurely.
    } else {
	log->FnExit(fnname, clname); 
	return rstat=False; 
    }
    
    // If we already have a convert Fn defined get rid of it
    if ( itsPlotOptions.Convert != NULL )
    {
	//delete itsPlotOptions.Convert;
	//itsPlotOptions.Convert = NULL;
    }


    //# If we've made it here we know that our inputs are corred.
    //# and we don't need to check them below.
    if ( ! upcase( axisStr ).compare( "AZIMUTH" ) ) {
        itsPlotOptions.Convert = 
	    new MSPlotConvertAzimuth( msDerived, msColumns, useXAxis);
    } else if ( ! upcase( axisStr ).compare( "ELEVATION" ) ) {
        itsPlotOptions.Convert = 
	    new MSPlotConvertElevation( msDerived, msColumns, useXAxis );
    } else if ( ! upcase( axisStr ).compare( "HOURANGLE" ) 
              || ! upcase( axisStr ).compare( "HOUR_ANGLE" ) ) {
        itsPlotOptions.Convert = 
	    new MSPlotConvertHourangle( msDerived, msColumns, useXAxis );
    } else if ( ! upcase( axisStr ).compare( "PARALLACTICANGLE" ) 
               || ! upcase( axisStr ).compare( "PARALLACTIC_ANGLE" ) ) {
	itsPlotOptions.Convert = 
	    new MSPlotConvertParallacticangle( msDerived, msColumns, useXAxis );
	
    } else if ( ! upcase( axisStr ).compare( "XEAST" )  ) {
	//# This is for the antenna position plot.
	itsPlotOptions.Convert = 
	    new MSPlotConvertArrayPositions( msColumns );
    } else if ( !upcase(axisStr).compare( "CHANFREQ" ) 
             || !upcase(axisStr).compare( "CHANNEL_FREQ" )
             || !upcase(axisStr).compare( "CHANNEL_FREQUENCY" )
             || !upcase(axisStr).compare( "FREQUENCY" )
             || !upcase(axisStr).compare( "CHAN_FREQ" ) ) 
    {
	if ( itsAveMode.length() < 1 || 
             ( itsAveChan <= 1 && itsAveTime <= itsMinTimeInterval ) )
	{
	    //# Since this is a cross plot we need to allow plotting
	    //# in both x and y axies.
	    Bool averagingOn = False;
	    if ( ( downcase(itsAverage[0]).matches( "chan" )
       		   || downcase(itsAverage[0]).matches( "both" ) )
                && ( itsAverage[1].length() > 1 
                    &&  !itsAverage[1].matches( "NONE" ) ) )
		averagingOn = True;
	  
	    if ( itsTableVecSpwIds.nelements() == 1 ) {
		itsPlotOptions.Convert = new MSPlotConvertChanToFreq( 
		    itsTableVecSpwIds[0],
		    this,
		    averagingOn,
		    itsStartChan,
		    itsStepChan,
		    itsEndChan, 
		    useXAxis );
	    } else {
		//# We have a vector of tables, each one having the same
		//# SPW.
		Vector< Int > spwIds( itsTableVecSpwIds.nelements() );
		for( uInt j=0; j< itsTableVecSpwIds.nelements(); j++ )
		    spwIds[j] = itsTableVecSpwIds[j][0];
		
		itsPlotOptions.Convert = new MSPlotConvertChanToFreq( 
		    spwIds,
		    this,
		    averagingOn,
		    itsStartChan,
		    itsEndChan,
		    itsStepChan,
		    useXAxis );
	    }
	}	
    } else if ( !upcase(axisStr).compare( "CHANVELOCITY" ) 
             || !upcase(axisStr).compare( "CHANNEL_VELOCITY" )
             || !upcase(axisStr).compare( "CHAN_VELOCITY" )
             || !upcase(axisStr).compare( "VELOCITY" ) ) 
    {
	if ( itsAveMode.length() < 1 || 
             ( itsAveChan <= 1 && itsAveTime <= itsMinTimeInterval ) )
	{
	    //# Setup the function that will convert the data points
	    //# to velocities.	
	    if ( itsTableVecSpwIds.nelements() == 1 ) {
		itsPlotOptions.Convert = 
		    new MSPlotConvertChanToVelocity( itsTableVecSpwIds[0],msColumns, this,useXAxis );
	    } else {
		//# We have a vector of tables, each one having the same
		//# SPW.
		Vector< Int > spwIds( itsTableVecSpwIds.nelements() );
		for( uInt j=0; j< itsTableVecSpwIds.nelements(); j++ )
		    spwIds[j] = itsTableVecSpwIds[j][0];
		
		itsPlotOptions.Convert = 
		    new MSPlotConvertChanToVelocity( spwIds, msColumns, this, useXAxis );
	    }
	}	
    }
    
    //# Indicate that the Convert Class option has been set
    //# We need this to reset the table and force TablePlot to
    //# re-interpret the Convert Class information.
    itsConvertClassSet = True;

    log->FnExit(fnname, clname);
    return rstat;
}


//#////////////////////////////////////////////////////////////////////////////
//# Used by uvdist to convert the x-axis from meters to klambda
//# 
//# Important things to know to do the conversion.
//# TaQL to select the SQRT(SUMSQUARE(UVW[1:2]))
//# c (define in Quanta) is in m/s
//# REF_FREQ is in??
//#
//# For this we need
//# the frequency of the observation.
//# The fomula's are as follows:
//#     lambda = c (speed of light)/nu (frequency of obsrvation)
//#              speed of light defined in QC.h (QC::c m/s)
//#     nu/c is in m/wavelength or m/lambda
//#     We want kilolambda and our data points are in meters so we need to 
//#     convert  as follows:
//#           nu/c * 1000 lambda / klambda 
//#
//# May need to adjust if frequency is not in seconds.
//#
Double
MsPlot::getMeterToKlambdaConvertValue( uInt spwId )
{
    String FnCall = String( "(" )
         + String::toString( spwId ) + String( ")" );
    String fnname = "getMeterToKlambdaConvertValue";
    log->FnEnter(fnname + FnCall, clname);

    if ( ! checkInit() || ! checkOpenMS() )  
    { 
       log->FnExit(fnname, clname);
       AipsError ae( "MsPlot class is not initialized" );
          throw( ae );
    }
    
    Quantum<Double> c = QC::c;

    //# Get the spectral window column since it has the REF_FREQ value
    //# in it.
    //#ROMSSpWindowColumns spwc(itsSelectedMS.spectralWindow());
    //#ROScalarColumn<Double> refFrequencies = spwc.refFrequency();
  

    //#if ( spwId < 0 || spwId >= refFrequencies.nrow() )
    if ( spwId < 0 || spwId >= itsRefFrequencies.nelements() )
    {
        String msg = String( "Unable to obtain reference frequency for " )
             + String( "spectral window " ) + String::toString( spwId )
             + String( ". an not convert UV distance klambda values." );
        log->out(msg, fnname, clname, LogMessage::NORMAL4 );
    }
  
    log->FnExit(fnname, clname);

    return ( c.getValue() / itsRefFrequencies( spwId )  ) * 1000;
}


//#////////////////////////////////////////////////////////////////////////////
//# Open a measurement set to be used with the MS plotter.
Bool
MsPlot::open( const String& MSPath, Bool doV, const String& restfreq,
              const String& frame, const String& doppler)
{   
    String FnCall = String( "(" ) + MSPath + String(")" );
    String fnname = "open";
    log->FnEnter(fnname + FnCall, clname);
    //# Check if we have something open already if we do we need to
    //# shut things down.
    if ( itsMsIsOpen )
    {
       reset( True );
    }
    itsMsIsOpen = False;
    itsResetCallBack = NULL;
    
    doVel = doV;
    //cout << "doVel=" << doVel << endl;

    Bool rstat = True;
    if ( itsMS != NULL && itsMS->tableName().find( MSPath ) > 1  
         && itsMS->tableName().find( MSPath ) < MSPath.length() )
    {
      log->out(String("New Measurement Set not created. The Measurement") +
               " Set name is the same as the current one. ",
              fnname, clname, LogMessage::DEBUG1);
      return rstat;
    }

    if ( itsMS != NULL )
    {
	//# TODO Do some close stuff
	delete itsMS;
	itsMS = NULL;
    }

    try {
	itsMS = new MeasurementSet( MSPath, casa::Table::Update );
	itsSelectedMS = MS(*itsMS);
	itsIsNewSelection = True;

	//# Initialize internal. variables.
	initialize();
	//# Store the full list of possible
	//#      baselines
	//#      field names
	//#      spw names (in the future when they have names)
	//#      correlations
	//# Note that the uvrange, time, scan, feed, array, channel, and
	//# and frequency selection don't us anything besides what the user
	//# has provided.

        itsRestFreq = restfreq;
        itsFrame = frame;
        itsDoppler = doppler; 
         
	//can not init lists for corrupted ms! 
	initAllLists();
	//# Initialize the channel and corr selection.
	//# This needs to go after initAllLists() since we need to know
	//# how many SPWs we have.
	resetDataSelection();
	resetTableSelection();
    
	//# In order for flagging to work properly TablePlot
	//# needs to know what table we are using right away
	//# so we are going to do that.
	setInputTable( String(""), String( "") );
	
	//# Flag to indicate that the MS has been opened.
	itsMsIsOpen = True;
    }
    catch(...) {
	log->out("failed to initialize msplot",  fnname, clname, 
		LogMessage::WARN);
	rstat=False;
    }

    log->FnExit(fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//# Create internal lists for the given measurement set.  This lists
//# save of from recreating them everytime we are plotting someting,
//# as they are used to construct the "selection" list.
//#////////////////////////////////////////////////////////////////////////////
void
MsPlot::initAllLists()
{
    String fnname = "initAllLists";
    log->FnEnter(fnname, clname);
    //# Get all of the field names for this MS
    getAllFieldNames();

    //# Get all of the antenna names for this MS
    getAllAntennaNames();

    //# Get the number of spws in this MS, this also gets the
    //# list of frequencies (reference frequencies), since the
    //# frequencies are stored with the spectral windows.
    //# We need to know the number of fields (ie. have called
    //# getAllFieldNames()) before calling SpwsAndFreqs.
    getAllSpwsAndFreqs();
    getAllSpwChanNums();

    //# Get all of the correlation names associated with this measurement
    //# set.  The correlation names are stored in itsCorrelationNames, and
    //# the ids (indexes) are stored in itsCorrelationIds.  The ids start
    //# at 1.
    getAllCorrelations();

    //# Get the number of scan numbers.
    getAllScanNumbers();
    
    //# Get the start and stop time of the observation(s)
    getAllTimes();
    log->FnExit(fnname, clname);
}

//#////////////////////////////////////////////////////////////////////////////
//# This method originally written by Gary Li.
//#
//# It has been changed a fair bit.  Originally he created two copies of the
//#    "stokes" names and returned them.  Well one was a string with the full 
//# list separated with a space, the other was a vector of string.  
//# Now it just returns the list of correlations(polarizations) used in the MS.
//#
//# The index to the itsCorrelationNames is the SPW id, for example
//# itsCorrelationIds[0] is the porazization Ids for SPW 0.  Different
//# SPWs can have multiple polarization IDs associated with it, and each
//# polarization ID can have different polarizations.
//#
//# Note that it's assumed we know what the maxSpwId is before we 
//#      get to this routine.

 void
 MsPlot::getAllCorrelations()
 {
     String fnname = "getAllCorrelations";
     log->FnEnter( fnname, clname );

     if ( itsMS == NULL )  { 
        //log->out( "Unable to continue, no Measurement Set open.",
        //         fnname, clname, LogMessage::WARN, True);
        //log->FnExit(fnname, clname); 
        return; 
     }

     //# Find out what we have for polarizations.
     ROMSDataDescColumns dataDescColumn( itsMS->dataDescription() );
     ROMSPolarizationColumns polarColumn( itsMS->polarization());

     //# Reserve some memory and set class vars
     itsMaxPolId = polarColumn.nrow();
     itsCorrelationNames.resize( itsMaxPolId );
     itsCorrelationIds.resize( itsMaxPolId );
     
     //# TODO determine if its an error if there are no rows in the
     //# polarization column.

     //# We need to keep track of the contents of the polarization table,
     //# which contains all of the correlation information.  Why this naming
     //# confusion, I do not know.
     Vector<Int> numFound( itsMaxPolId );
     for( uInt polId=0; polId < itsMaxPolId; polId++ )
     {
        itsCorrelationNames[polId].resize( Stokes::NumberOfTypes );
        itsCorrelationIds[polId].resize( Stokes::NumberOfTypes );
        numFound[polId] = 0;
     }

     //# Loop through the list of data description IDs to get the
     //# full list of polarizations for each polarization ID.  Each
     Int polId=-1;
     for( uInt ddId=0; ddId < dataDescColumn.nrow(); ddId++ )
     {   
        polId = dataDescColumn.polarizationId()(ddId);
        const Vector<Int> corrType = polarColumn.corrType()(polId);
        for( uInt corrId=0; corrId < corrType.nelements(); corrId++ )
        {
           String name = Stokes::name( Stokes::StokesTypes(corrType[corrId]) );
           uInt lastPolar = numFound[polId];
           Bool isNewName = True;
           for( uInt i=0; i<lastPolar; i++ )
           {
              if( name.matches( itsCorrelationNames[polId][i] ) )
              { 
                  isNewName = False; 
                 break; 
              }
           }

           if( isNewName )
           {
              itsCorrelationNames[polId][lastPolar] = name;
              itsCorrelationIds[polId][lastPolar] = lastPolar+1;  
               //#IDs start at 1
              numFound[polId] = lastPolar+1;
           }   
        } //# end of for( polType );
     }
    
     //# Loop through the list making sure that we get rid of the
     //# memory we didn't use.  Its assumed that it is faster to
     //# trim memory then to reallocate mem. with each trip through   
     //# the above loop
     Int size;
     for ( uInt polId=0; polId < itsMaxPolId; polId++ )
     {
        size = numFound[polId];
        itsCorrelationNames[polId].resize(size, True );
        itsCorrelationIds[polId].resize(size, True );

        //# Print some debugging details
        String corrNames = "";
   
       if ( size > 0 ) {
           String msg = String("All potential correlations for ID=")
                        + String::toString( polId ) + ": ";
           for( uInt j=0; j < itsCorrelationNames[polId].nelements(); j++ )
           {
              if ( j > 0 )
                 msg += String( ", " );
              msg += String( " " );
              msg += itsCorrelationNames[polId][j];
           }
           log->out( msg, fnname, clname, LogMessage::DEBUG1 );
       } else {
#if LOG2 
           String msg = String( "No valid correlation found for ID=" )
                        + String::toString( polId );
           log->out(msg, fnname, clname, LogMessage::DEBUG1);
#endif 
       }
    }

    log->FnExit(fnname, clname);
}

//#////////////////////////////////////////////////////////////////////////////
//# Store the list of field names in a local vector to be used for constructing
//# the itsSelectString.
//#
//# Note that they are stored in their index order -- fieldName[0] is the name
//# for the first field.
void
MsPlot::getAllFieldNames()
{
   String fnname = "getAllFieldNames";
   log->FnEnter( fnname, clname );

   if ( itsMS == NULL )  { 
       //log->out( "Unable to continue, no Measurement Set open.",
       //        fnname, clname, LogMessage::WARN, True);
       //log->FnExit(fnname, clname); 
       return; 
   }

   //# Gets the field column of the MS
   ROMSFieldColumns fieldColumn( itsMS->field() );
   uInt numFields = fieldColumn.nrow();
   if (numFields==0 )
   {
       log->out( "This Measurement Set has 0 fields!", 
                fnname, clname, LogMessage::WARN);
       return;
   }
     
   //# Store the field names for the MS
   itsFieldNames.resize( numFields );
   for ( uInt i=0; i < numFields; i++ )
       itsFieldNames[i] = (fieldColumn.name())(i);
     
   log->FnExit(fnname, clname);
}

//#////////////////////////////////////////////////////////////////////////////
//# Store the list of antenna names in a local vector to be used for 
//# constructing the itsSelectString.
//#
//# Note that they are stored in their index order -- itsAntennaName[0] is 
//# the name of the first antenna.
void
MsPlot::getAllAntennaNames()
{
     String fnname = "getAllAntennaNames";
     log->FnEnter(fnname, clname);

    if ( itsMS == NULL )  { 
        //log->out( "Unable to continue, no Measurement Set open.",
        //         fnname, clname, LogMessage::WARN, True );
        //log->FnExit(fnname, clname); 
        return; 
    }

     //# Get the antenna column from the MS
     //#ROMSColumns antColumn( *itsMS );
     const ROMSAntennaColumns & antColumn  = itsMS->antenna();
     //#const ROScalarColumn<String> antNames = antColumn.name();
     uInt numAnts= antColumn.position().nrow();
     
     itsAntennaNames.resize(numAnts);
     for( uInt i=0; i<numAnts; i++ )  
     itsAntennaNames[i] = (antColumn.name())(i);

     log->FnExit(fnname, clname);
}

//#////////////////////////////////////////////////////////////////////////////
//# Store the list of spectral window indexes. There are currently empty
//# names associated with the specral windows, this may change for ALMA
//# and we may need to change this method to store the spw names instead
//# of the indexes.
//#
//# The information in this class variable is used in the construction
//# of the itsSelectString, as well as some sanity checking.
//#
//# SDJ I think?
//# Note that the indexing starts at 1 for the user, but 1 for the
//# software.  Very confusing ;).
//#
//# TODO combine this with polarization gather as they both need to
//#      loop through the data description IDs
void
MsPlot::getAllSpwsAndFreqs()
{
    String fnname = "getAllSpwsAndFreqs";
    log->FnEnter(fnname, clname);
    
    if ( itsMS == NULL )  { 
       //log->out( "Unable to continue, no Measurement Set open.",
       //         fnname, clname, LogMessage::WARN, True);
       //log->FnExit(fnname, clname); 
       return; 
    }
    
    //# Find out how many SPWs we have in this MS
    ROMSSpWindowColumns spwColumn(itsSelectedMS.spectralWindow());
    itsMaxSpwId = spwColumn.nrow();
    itsUsedSpws.resize( itsMaxSpwId );
    itsUsedSpws.set( False );
    

    //# Create the list of bools that indicates if we have data for the
    //# SPW or not.  We may have more entries in the SPW column the
    //# spectral windows used. For example, if the only SPWs are 2 and 3
    //# we will also have rows 0 and 1.
    ROMSDataDescColumns dataDescColumn(itsMS->dataDescription());
    MSRange msRange( *itsMS );
    //cout << " be careful a crash here !+++++++++" << endl;
    Record rc = msRange.range(MSS::DATA_DESC_ID);
    Vector<Int> dataDescIds = rc.asArrayInt(RecordFieldId(0));
    //cout << " +++++++++" << endl;
    Vector<uInt> udataDescIds( dataDescIds.nelements() );
    for ( uInt i=0; i < dataDescIds.nelements(); i++ )
       udataDescIds[i]=dataDescIds[i];
    Vector<Int> spwIds = 
       dataDescColumn.spectralWindowId().getColumnCells(udataDescIds);
    Vector<Int> tmpSpwIds = 
	dataDescColumn.spectralWindowId().getColumnCells(udataDescIds);
    
    const Sort::Order order = Sort::Ascending;
    const Int option = Sort::HeapSort | Sort::NoDuplicates;
    Int numSpw = GenSort<Int>::sort(spwIds, order, option );
    Int lastId = -1;
    for( Int i=0; i < numSpw; i++ )
    {   
       uInt spwId = spwIds[i];
       if ( (Int)spwId == lastId )
          continue;
       itsUsedSpws[spwId] = True;
    }
        
    //# Gather up the reference and channel frequencies for each SPW.  Created
    //# so that we can look up the values by SPW ID.  Nots, that
    //# the largest spwId will reside at the end of the spwIds
    //# Vector since it is sorted.
    ROScalarColumn<Double> refFrequencies = spwColumn.refFrequency();
    
    // TODO check that the largest spwId is less then the size of
    // the reference frequencies.
    // uInt numFreq = refFrequencies.nrow();
    itsRefFrequencies.resize( spwIds[numSpw-1]+1 ); // itsRefFrequencies=0;

    itsChanFrequencies.resize( spwIds[numSpw-1]+1 );
    for ( uInt i=0; i < itsChanFrequencies.nelements(); i++ )
	itsChanFrequencies[i].resize(0);

    // The channel frequency has 3 axis: spw, field, and channel.
    // This is becuase the frequency will differ for each field and
    // later we'll need to retrieve the value based on spw and channel.
    // We use the fieldNames vector to find out how many fields we have.
    // The axies are; x=spw, y=field, and z=channel.

    itsChanVelocities.resize( spwIds[numSpw-1]+1 );
    for ( uInt i=0; i < itsChanVelocities.nelements(); i++ )
	itsChanVelocities[i].resize(itsFieldNames.nelements());

    Double cspeed = (QC::c).getValue() / 1000.;
    //cout << "cspeed=" << cspeed << endl;

    // Set up the MSDerived object we'll use to get the velocities.
    MSDerivedValues * msDerived = new MSDerivedValues();
    msDerived->setMeasurementSet(*itsMS);
    msDerived->setVelocityReference( MDoppler::RADIO );

    //cout << "itsRestFreq=" << itsRestFreq
    //     << " itsFrame=" << itsFrame
    //     << " itsDoppler=" << itsDoppler << endl;

    MFrequency::Types freqtp = MFrequency::LSRK;
    MDoppler::Types doptp = MDoppler::RADIO;
    MFrequency trans;
    Quantity qt;
    if (itsRestFreq != "") {
        if (!MFrequency::getType(freqtp, itsFrame))
           freqtp = MFrequency::LSRK; 
        if (!MDoppler::getType(doptp, itsDoppler))
           doptp = MDoppler::RADIO; 

        msDerived->setVelocityReference(doptp);
    	msDerived->setFrequencyReference(freqtp);


        if (MeasTable::Line(trans, itsRestFreq)) {
           qt = trans.get("GHz");
    	   msDerived->setRestFrequency(qt);
           log->out("setRestFrequency: " + String::toString(qt.getValue()) +
                     " " + qt.getUnit(), 
                    fnname, clname, LogMessage::NORMAL5);
        }
        else {
           Double fr = 1;
           String unit = "";
           String freqVal = downcase(itsRestFreq);
           if (freqVal.contains("ghz")) {
              freqVal = freqVal.before("ghz");
              fr = atof(freqVal.chars());
              unit = "GHz";
           }
           else if (freqVal.contains("mhz")) {
              freqVal = freqVal.before("mhz");
              fr = atof(freqVal.chars());
              unit = "MHz";
           }
           //else if (freqVal.contains("khz")) {
           //   freqVal = freqVal.before("khz");
           //   fr = atof(freqVal.chars());
           //   unit = "KHz";
           //}
           else if (freqVal.contains("hz")) {
              freqVal = freqVal.before("hz");
              fr = atof(freqVal.chars());
              unit = "Hz";
           }
           //if (fr < 0)
           //   fr *= -1;

           //cout << "itsRestFreq=" << fr << " " << unit << endl;
           qt =  Quantity(fr, unit);
    	   msDerived->setRestFrequency(qt);
           log->out("setRestFrequency: " + String::toString(qt.getValue()) +
                     " " + qt.getUnit(), 
                    fnname, clname, LogMessage::NORMAL5);
        }

        for (uInt i=0; i < tmpSpwIds.nelements(); i++) {
    	    itsRefFrequencies[tmpSpwIds[i]] = refFrequencies(i);
    	    itsChanFrequencies[tmpSpwIds[i]] = spwColumn.chanFreq()(tmpSpwIds[i]);

            for (uInt fieldId=0; fieldId < itsFieldNames.nelements(); fieldId++) {
    	        msDerived->setFieldCenter( fieldId );
    
    	        itsChanVelocities[tmpSpwIds[i]][fieldId].resize(
                      itsChanFrequencies[tmpSpwIds[i]].nelements() );
    
    	        for (uInt chanNum=0; 
                    chanNum < itsChanVelocities[tmpSpwIds[i]][fieldId].nelements(); 
                    chanNum++ ) {
            
            	    Double tmp =  msDerived->toVelocity(
                         Quantity(itsChanFrequencies[tmpSpwIds[i]][chanNum], "Hz")
                       ).get("km/s").getValue();
    
    		    itsChanVelocities[tmpSpwIds[i]][fieldId][chanNum] = 
                          (tmp < 0) ? max(tmp, -cspeed)
                                    : min(tmp, cspeed);
            
    	        }
    	    }
        }
    } 
    else {

        for (uInt i=0; i < tmpSpwIds.nelements(); i++) {
           itsRefFrequencies[tmpSpwIds[i]] = refFrequencies(i);
    	   // Use the chanFreqMeas() method???
    	   itsChanFrequencies[tmpSpwIds[i]] = spwColumn.chanFreq()(tmpSpwIds[i]);
    
    	   // Now deal with velocities, note that this takes the spw ID
    	   // an field ID.  TODO: We assume the field IDs start at 0 and
    	   // increase by 1, that there is no breaks.  This may not be true
    	   // if we are using a partial MS that's been created with ms.split()
    	   // We need to remove this assumption.
    	   for (uInt fieldId=0; fieldId < itsFieldNames.nelements(); fieldId++) {
    	       if ( msDerived->setRestFrequency(fieldId,tmpSpwIds[i]) ) {
    	          // Has a rest frequency so find its type and set it.
    		  msDerived->setFrequencyReference(
                       MFrequency::castType(spwColumn.measFreqRef()(tmpSpwIds[i])));
    	        } else {
    		    msDerived->setFrequencyReference(MFrequency::LSRK);
    		    msDerived->setRestFrequency(Quantity(1.420, "GHz"));
                    //cout  << " doVel=" << doVel << endl;
                    if (doVel) {
                      SLog::slog()->out(String("No rest frequency found in the MS.")
                         //+ String(" use LSRK 1.420 Ghz for velocity calculation"),
                         + String(" Can't calculate velocity properly."),
                         "", "MsPlot", LogMessage::NORMAL, False);
                       doVel = False;
                    }
    	         }
    	         msDerived->setFieldCenter(fieldId);
    
    	         itsChanVelocities[tmpSpwIds[i]][fieldId].resize(
                      itsChanFrequencies[tmpSpwIds[i]].nelements());
    
    	         for (uInt chanNum=0; 
                    chanNum < itsChanVelocities[tmpSpwIds[i]][fieldId].nelements(); 
                    chanNum++ ) {
           	    //itsChanVelocities[tmpSpwIds[i]][fieldId][chanNum] = 
    	            //msDerived->toVelocity(
                    //    Quantity(itsChanFrequencies[tmpSpwIds[i]][chanNum], "Hz")
                    //   ).get("km/s").getValue();
    
    	            //cout << "converted frequency " 	
    	            //     << itsChanFrequencies[tmpSpwIds[i]][chanNum]
                    //     << " for field " << fieldId << " to velocity " 
    	            //     << itsChanVelocities[tmpSpwIds[i]][fieldId][chanNum]
    	            //     << endl;
    
                    // this is to bound out unreasonable speed values, 
                    //but it results // no point 
            
            	    Double tmp =  msDerived->toVelocity(
                          Quantity(itsChanFrequencies[tmpSpwIds[i]][chanNum], "Hz")
                         ).get("km/s").getValue();
    
    		    itsChanVelocities[tmpSpwIds[i]][fieldId][chanNum] = 
                          (tmp < 0) ? max(tmp, -cspeed)
                                    : min(tmp, cspeed);
            
    	        }
    	    }
        }
    }
    
    log->FnExit( fnname, clname);
}

Double 
MsPlot::getRefFrequency( uInt spwId ) 
{
    Double retValue=0;
    if ( spwId < itsRefFrequencies.nelements() ) 
	retValue = itsRefFrequencies[spwId];
    else
	throw AipsError(
	    String ( "Internal Error: Reference frequency requested for an" )
	    + String( "unknown spectral window: " ) + String( spwId ) );
    return retValue;
}


Double 
MsPlot::getChanFrequency( uInt spwId, uInt channel ) 
{
    Double retValue=0;
    if ( spwId < itsChanFrequencies.nelements() && 
	channel < itsChanFrequencies[spwId].nelements() ) 
	retValue = itsChanFrequencies[spwId][channel];
    else
	throw AipsError(
	    String ( "Internal Error: Channel frequency requested for an" )
	    + String( "unknown\nspectral window/channel combination: " ) 
	    + String( spwId ) + String( "/" ) + String(channel) );
    return retValue;
}

Double 
MsPlot::getChanVelocity( uInt spwId, uInt fieldId, uInt channel ) 
{
    Double retValue=0;
    //cout << "Getting velocity for spw " << spwId << "and field " << fieldId << " and channel " << channel << endl;
    
    if ( spwId < itsChanVelocities.nelements() && 
	 fieldId < itsChanVelocities[spwId].nelements() && 
	 channel < itsChanVelocities[spwId][fieldId].nelements() ) 
	retValue = itsChanVelocities[spwId][fieldId][channel];
    else
	throw AipsError(
	    String ( "Internal Error: Channel velocity requested for an" )
	    + String( " unknown\nspectral window/field/channel combination: " ) 
	    + String( spwId ) + String( "/" ) + String( fieldId )
	    + String( "/" ) + String( channel ) );
    return retValue;
}
    

//#////////////////////////////////////////////////////////////////////////////
//# Store the number of channels for each SPW
//# TODO : Currently this checks only for number of channels.
//#        Need to add in checks for the number of correlations.
//#        -> TablePlot needs Tables that have the same number of
//#           chans and corrs per row, for all rows.
//#////////////////////////////////////////////////////////////////////////////
void
MsPlot::getAllSpwChanNums()
{
    String fnname = "setSpwChanNums";
    log->FnEnter(fnname, clname);

    //# Throw an exception if the MsPlot class has not bee initialized
    //# or if there is no MS associated with the class.
    if ( itsMS == NULL )  { 
       log->out( "Unable to contine, no Measurment Set open.",
          fnname, clname, LogMessage::WARN, True);
       log->FnExit(fnname, clname); 
       return; 
    }

     
    //# Get a list of number-of-channels for all spws.
    ROScalarColumn<Int> numchan( itsSelectedMS.spectralWindow(), "NUM_CHAN" );
    Vector<Int> cnums = numchan.getColumn();

    //# Fill in the shapes for the selected SPWids.
    //# This is needed in dataTaQL, to set the "end chan".
    //# TODO : put a check somewhere to make sure this is filled before 
    //#        using it
    itsSpwIdNChans.resize( itsMaxSpwId );
    for ( uInt i=0; i < itsMaxSpwId; i++ )
    {
   itsSpwIdNChans[i] = cnums[i];
    }

    log->FnExit( fnname, clname);
}


//#////////////////////////////////////////////////////////////////////////////
//# Store the number largest scan number, 
//#
//# SDJ I think?
//# Note that the indexing starts at 1 for the user, but 0 for the
//# software.  Very confusing ;).
void
MsPlot::getAllScanNumbers()
{
    String fnname = "getAllScanNumbers";
    log->FnEnter( fnname, clname );

    if ( itsMS == NULL )  { 
       //log->out( "Unable to continue, no Measurment Set open.",
       //         fnname, clname, LogMessage::WARN, True);
       //log->FnExit( fnname, clname); 
       return; 
    }
        
    ROMSColumns * msColumns = new ROMSColumns( itsSelectedMS );
    ROScalarColumn<Int> scanColumn = msColumns->scanNumber();
    uInt numRows = scanColumn.nrow();
    itsScanNumbers.resize( numRows );

    //# Initialze to a value different then the first value so the if statment
    //# in the for loop passes on the first element.
    Int previous=scanColumn(0)+1; 
    uInt numFound=0;
    Bool newNumber = True;

    for (uInt i=0; i<numRows; i++ )
    {
   //# Do a quick check.  Most of the time the scan numbers are in
   //# order so this quick check should eliminate most of the 
   //# duplicates.  The sort below will get rid of any others.
   if ( previous == scanColumn(i) )
       newNumber = False;
   else
       newNumber = True;
   
   //# Replace above with this loop to eliminate all duplicates,
   //# but given that an MS can have *lots* of scan numbers thiw
   //# can be very time consuming.
   /*
   for ( uInt j=0; j<numFound; j++ )
   {
       if ( itsScanNumbers[i] == scanColumn(i) )
       {
      newNumber = False;
      break;
       }
   }
   */
   if ( newNumber )
   {
       itsScanNumbers[numFound] = scanColumn(i);
       previous = itsScanNumbers[numFound];
       numFound++;
   }
    }

    itsScanNumbers.resize( numFound, True );
    numFound = 
      GenSort<Int>::sort( itsScanNumbers, Sort::Ascending, Sort::NoDuplicates );
    itsScanNumbers.resize( numFound, True );
       
    //# Make sure we don't waste any memory.
    delete msColumns; msColumns=NULL;
    
    log->FnExit( fnname, clname);
}


//#////////////////////////////////////////////////////////////////////////////
//# Store the start and end times of the observation(s)
//#
void
MsPlot::getAllTimes()
{
    String fnname = "getAllTimes";
    log->FnEnter(fnname, clname);

    if ( itsMS == NULL )  { 
       //log->out( "Unable to continue, no Measurment Set open.",
       //         fnname, clname, LogMessage::WARN, True);
       //log->FnExit(fnname, clname); 
       return; 
    }
        
    ROMSColumns msColumns( itsSelectedMS );
    Double startTime, endTime;
    ::casa::minMax( startTime, endTime, msColumns.time().getColumn() );
    
    //# Now store the time as an MVTime value, this allows us to
    //# easily find the year, month, ... of the value later on.
    //# We also need to adjust it a little, since CASA stores data
    //# as MJD.
    //# Also since MVTime doesn't have a set() method to set a var to
    //# a new time we need to use the assignment operator by creating
    //# some local vars here and then assigning a new value - bleck!
    //#MVTime tmpStart(startTime), tmpEnd( endTime);
    MVTime tmpStart(startTime/86400.0), tmpEnd( endTime/86400.0);
    
    itsStartTime = tmpStart;
    itsEndTime = tmpEnd;
    log->FnExit(fnname, clname);
}

//#////////////////////////////////////////////////////////////////////////////
//# Clenup memory used as well as resetting the class variables used
//# when set data or setspectral is called.
//#////////////////////////////////////////////////////////////////////////////
void
MsPlot::resetDataSelection()
{
    String fnname = "resetDataSelection";
    log->FnEnter(fnname, clname);
    
    //# Reset the channel and correlation selection
    //# to the default value of zero, implying "all".
    itsStartChan.resize( itsMaxSpwId );
    itsEndChan.resize( itsMaxSpwId );
    itsStepChan.resize( itsMaxSpwId );
    for ( uInt i=0; i < itsMaxSpwId; i++ )
    {
       itsStartChan[i].resize(1);   itsStartChan[i][0] = -1; 
       itsEndChan[i].resize(1);   itsEndChan[i][0] = -1;  
       itsStepChan[i].resize(1);   itsStepChan[i][0] = -1;
    }

    itsStartCorr.resize( itsMaxPolId );
    itsEndCorr.resize( itsMaxPolId );
    itsStepCorr.resize( itsMaxPolId );
    for ( uInt i=0; i < itsMaxPolId; i++ )
    {
       itsStartCorr[i].resize(1);   itsStartCorr[i][0] = -1; 
       itsEndCorr[i].resize(1);   itsEndCorr[i][0] = -1;  
       itsStepCorr[i].resize(1);   itsStepCorr[i][0] = -1;
    }

    //# reset averaging mode
    itsAverage.resize(2);
    itsAverage[0] = String("");
    itsAverage[1] = String("NONE");
    
    //# Reset the display string showing the currently selected data.
    itsDataSelectString = String( "" );

    //# We have now SPWs selected so we need to empty our record.
    itsSelectedSpwIds.resize( 0 );
    itsSelectedPolIds.resize( 0 );
    
    log->FnExit(fnname, clname);
}


//#////////////////////////////////////////////////////////////////////////////
//# Clenup memory used as well as resetting the class variables used
//# when setting the tables to be used by TablePlot.
//#////////////////////////////////////////////////////////////////////////////
void
MsPlot::resetTableSelection()
{
    String fnname = "resetTableSelection";
    log->FnEnter(fnname, clname);

    if ( itsTableVectors.nelements() > 0 )
    {
       for ( uInt i=0; i<itsTableVectors.nelements(); i++ )
          itsTableVectors[i].resize(0);
       itsTableVectors.resize(0);     
    }
    
    if ( itsTableVecNames.nelements() > 0 )
    {
       for ( uInt i=0; i<itsTableVecNames.nelements(); i++ )
          itsTableVecNames[i].resize(0);
       itsTableVecNames.resize(0);     
    }

    if ( itsTableVecSpwIds.nelements() > 0 )
    {
       for ( uInt i=0; i<itsTableVecSpwIds.nelements(); i++ )
          itsTableVecSpwIds[i].resize(0);
       itsTableVecSpwIds.resize(0);     
    }

    if ( itsTaqlStrings.nelements() > 0 )
    {
       for ( uInt i=0; i<itsTaqlStrings.nelements(); i++ )
       {
          for( uInt j=0; j<itsTaqlStrings[i].nelements(); j++ )
          if ( itsTaqlStrings[i][j].nelements() > 0 )
             itsTaqlStrings[i][j].resize(0);
          itsTaqlStrings[i].resize(0);
        }
        itsTaqlStrings.resize(0);
    }

    log->FnExit(fnname, clname);
}


Bool
MsPlot::done()
{
    String fnname = "done";
    log->FnEnter(fnname, clname);

    //# close the MS and set things back initial state.
    reset( True );
    
    //# Detroy the plotter -- the table plot object
    if ( itsTablePlot != NULL ) 
    { 
       //# Tell TablePlot I don't exist anymore
       itsTablePlot->clearResetCallBack("MsPlot");

       //# TablePlot destructor is now private: hopefully it knows
       //# to delete itself.
       //#delete itsTablePlot; 
       itsTablePlot=NULL; 
    }   

    //# Set the flag that lets us know whether or not an MS is open.
    itsMsIsOpen = False;

    log->FnExit(fnname, clname);
    return True;
}
    
//#////////////////////////////////////////////////////////////////////////////
//# Reset MsPlot to it's initial state.
//#////////////////////////////////////////////////////////////////////////////
Bool
MsPlot::reset( Bool resetMS )
{
    String fnname = "reset";
    log->FnEnter(fnname, clname);

    //# If we are in an iterative plot we need to halt it, otherwise
    //# we could leave things in an odd state.
    if ( itsIsInIterPlot && !resetMS )
        iterPlotStop( False );

    //# If we are resetting the Measurment set, then set the MS to NULL;
    //# TODO make sure that any data structures, tables, subMS, ... are
    //# destroyed.
    if ( resetMS )
    {
       //# Make sure all of the buttons are disabled after a reset.
       //# We no longer have a measurement set, so the buttons will
       //# only cause errors if they are enabled here.
       if ( itsMS != NULL ) { 
          delete itsMS; 
          itsMS = NULL; 
       }

       //# Need to make sure the flagging columns are set to the ususal
       //#ones.
       itsTablePlot->useFlagColumns( "FLAG", "FLAG_ROW" );
    }
    
    //# Reset all of the variables, reset the plot options,
    //# reset the data selection information for both the
    //# setdata and setspectral methods.

    itsInitialized = False;
    initialize();

    //# Reset any of the variables used to select tables for
    //# TablePlot to their initial state
    resetTableSelection();

    log->FnExit( fnname, clname);
    return True;
}

//#////////////////////////////////////////////////////////////////////////////
Bool 
MsPlot::initialize()
{
    String fnname = "initialize";

    //# Create a new message log.
    if (log == NULL) 
       log = SLog::slog();
    log->FnEnter(fnname, clname);

    itsExtendCorr = "";
    itsExtendChan = "";
    itsExtendSpw = "";
    itsExtendAnt = "";
    itsExtendTime = "";
    
    aveRowMap.resize(0, 0);
    aveChanMap.resize(0, 0);
    doAverage = False;
 
    itsRestFreq = "";
    itsFrame = "";
    itsDoppler = "";

    if ( itsTablePlot == NULL ) 
    {
       //# Create a new TablePlot class for plotting
       //#itsTablePlot = new casa::TablePlot();
       itsTablePlot = casa::TablePlot::TablePlotInstance();
#if LOG2 
       log->out("Created TablePlot class", 
                fnname, clname, LogMessage::DEBUG1 );
#endif 
    }

    //# Setup the reset call back with table plot.
    if ( itsResetCallBack == NULL )
    {
       itsResetCallBack = new casa::MSPlotReset( this );
#if LOG2 
       log->out( "Created MS Plot Reset callback class", 
           fnname, clname, LogMessage::DEBUG1 );
#endif 
    }
    itsTablePlot->setResetCallBack( "MsPlot", itsResetCallBack );
    

    //# TODO : Reset the MS variables 
    //# if( itsMS ) { delete itsMS; itsMS = NULL; }
    //# itsSelectedMS = MS(); //# ???? 

    //# Clean up the current Plot Options
    itsOptionRecord = Record();
    itsPlotOptions.reset(); //# resets to defaults.

    //# Flag that signals iteration plot state.
    itsIterPlotOn = casa::False;

    //# Flags that control when a new Selected MS 
    //#  must be sent into itsTablePlot
    itsIsSubTablePlot = casa::False;
    itsIsNewSelection = casa::False;
    if ( itsConvertClassSet && itsPlotOptions.Convert != NULL ) 
    {
	delete itsPlotOptions.Convert;
	itsPlotOptions.Convert = NULL;
    }
    
    itsConvertClassSet = casa::False;
    itsIsInIterPlot = casa::False; //# different from itsIterPlotOn.
    itsSpwIterOn = casa::False;
   
    //# Make sure the data selection values are set to their initial
    //# state. Currently this only resets chan, corr selection to "all".
    resetDataSelection();
    resetTableSelection();

    //# Flag to signal that MsPlot is ready to use.
    itsInitialized = casa::True;

    log->FnExit( fnname, clname);
    return True;
}

void
MsPlot::disableAllButtons( )
{
   itsTablePlot->changeGuiButtonState("markregion","disabled");
   itsTablePlot->changeGuiButtonState("flag","disabled");
   itsTablePlot->changeGuiButtonState("unflag","disabled");
   itsTablePlot->changeGuiButtonState("locate","disabled");
   //#itsTablePlot->changeGuiButtonState("iternext","disabled");
   //#itsTablePlot->changeGuiButtonState("quit","disabled");
   return;
}
void
MsPlot::enableAllButtons( )
{
   itsTablePlot->changeGuiButtonState("markregion","enabled");
   itsTablePlot->changeGuiButtonState("flag","enabled");
   itsTablePlot->changeGuiButtonState("unflag","enabled");
   itsTablePlot->changeGuiButtonState("locate","enabled");
   //#itsTablePlot->changeGuiButtonState("iternext","enabled");
   //#itsTablePlot->changeGuiButtonState("quit","enabled");
   return;
}
        
//#////////////////////////////////////////////////////////////////////////////
//# Take in user input plot options and fill it into itsPlotOptions.
//#////////////////////////////////////////////////////////////////////////////
Bool 
MsPlot::setplotoptions( casa::Record options )
{
    String FnCall = "(options)";
    String fnname = "setplotoptions";
    log->FnEnter( fnname + FnCall, clname);

    Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { 
       log->FnExit(fnname, clname);
       return rstat=False; 
    }

    //# (1) Assign a fresh plot-option holder.
    //#     This resets everything to default.
    //#     Keep a copy of this record.
    //#      - to reset any plot-type specific parameters
    //#        while retaining user-set parameters.
    itsOptionRecord = options;


    //# (2) Need to parse the plotrange option.  MsPlot let user's specify
    //#     strings, but TablePlot wants only double values. The strings 
    //#     are used to allow for time ranges. 
    //# If this record contains "timerange" do the needful top
    //# convert this into Doubles, and then define "plotrange"
    //# in itsOptionRecord, before filling into itsPlotOptions.
    //# After this method is done the "plotrange" and "plotrangesset"
    //# option values are set.
    //# TODO do something with these errors
    Vector<String> prErrors(2);
    prErrors = parsePlotRange();
    if ( prErrors[0].length() > 0 ) {
       log->out(prErrors[0], fnname, clname, LogMessage::SEVERE );
       rstat = False;
    }
    if (prErrors[1].length() > 0 ) {
       //# These are warnings
       log->out( prErrors[1], fnname, clname, LogMessage::WARN );
       rstat = False;
    }
    
    //# (3) Fill itsPlotOptions from itsOptionRecord.
    //#     This first sets defaults for everything and
    //#     then changes only the parameters defined in
    //#     itsOptionRecord.
    Vector<String> errors(2);
    errors[0] = itsPlotOptions.fillFromRecord( itsOptionRecord );
    if ( errors[0].length() > 0 )
    {
       //# Errors have occured!
       log->out( errors[0], fnname, clname, LogMessage::WARN, True);
       rstat = False;
    }
    
    //# (4) We should also validate the plot options.  This will
    //#     include warnings for correct, but probably not desired
    //#     results.

    //cout << "setplotoptions--------" << endl;

    //cas-734, unspecified warnning - fixed 5/11/08
    errors = itsPlotOptions.validateParams();
    //cout << "errors[0]=" << errors[0] << endl;
    //cout << "errors[1]=" << errors[1] << endl;
    if ( errors[0].length() > 0 )
    {
       //# These are errors
       log->out( errors[0], fnname, clname, LogMessage::NORMAL5);
       rstat = False;
    }   
    if ( errors[1].length() > 0 )
    {
       //# These are warnings
       log->out( errors[1], fnname, clname, LogMessage::NORMAL5);
       rstat = False;
    }
    //cout << "setplotoptions========" << endl;
    
    //# Print out the current plot options. (for debugging)
#if LOG2 
    log->out(itsPlotOptions.print(), fnname, clname, LogMessage::DEBUGGING );
#endif 
    log->FnExit(fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//# Parse the input provided by the user for the plotrange option.
//#
//# RETURN: returns error/warning messages.  If no mesasges are returned then
//#         there were no errors.  The warnings are in retValue[1] and the
//#         errors in retValue[0];
//#
Vector<String>
MsPlot::parsePlotRange()
{
    String fnname = "parsePlotRange";
    log->FnEnter(fnname, clname );

    Vector<String> errors(2);
    if ( ! checkInit() || ! checkOpenMS() )  { 
       errors[0] = "There is no measurement set open";
       log->FnExit(fnname, clname); 
       return errors; 
    }

    //# Set the plot options to the default values
    Vector<Double> rangeDblVec(4);
    rangeDblVec.set( 0.0 );
    
    Vector<Bool> rangesGiven(4);
    rangesGiven.set( False );

    
    if ( itsOptionRecord.isDefined( "plotrange" ) ) {
       RecordFieldId rid( "plotrange" );
       DataType type = itsOptionRecord.dataType( rid );
       if ( type != TpString )
            errors[0] += String("\nMsPlot plotrange expected a string value" );
       else {
          //# Get what the user gave us.
          String rangeStr;
          itsOptionRecord.get( rid, rangeStr );

          //# Parse our String into 4 strings
          Vector<String> newErrors(2);
          Vector<String> rangeStrVec(4);
          newErrors = parseStrToList( rangeStr, rangeStrVec );
       
          errors[0] += newErrors[0];
          errors[1] += newErrors[1];
    
          if ( errors[0].length() < 1 )
          {
             //# Convert each of the range values to a double value.
             newErrors = plotrangeToDbls(rangeStrVec, rangeDblVec, rangesGiven);
             errors[0] += newErrors[0];
             errors[1] += newErrors[1];
          }
       
          //# Remove the definition of the plotrange because we need
          //# to set it to what TablePlot expects.
          itsOptionRecord.removeField( rid );
       }
    }
    
    if ( errors[0].length() < 1 )
    {
       itsOptionRecord.define( RecordFieldId("plotrange"), rangeDblVec );
       itsOptionRecord.define( RecordFieldId("plotrangesset"), rangesGiven );
    }

    log->FnExit(fnname, clname);
    return errors;
    
}

//#////////////////////////////////////////////////////////////////////////////
//# Go through the list of strings given for the plotrange option.
//# We are expecting no more then 4 values, [xmin,xmax,ymin,ymax]
//#
//# We first try and convert the values into a double value, if this
//# is not successful then we assume its time expression and try and
//# convert the time expression to string.
//#
//# If no value is given, ie. an empty string we assume the user
//# didn't set the value.
//#
Vector<String>
MsPlot::plotrangeToDbls( Vector<String> rangeStrVec, 
   Vector<Double> rangeVector,
   Vector<Bool>   rangesSet )
{
    String fnname = "plotRangeToDbl";
    log->FnEnter(fnname + "(rangeStrVec, rangeVector, rangesSet)", clname);

    Vector<String> errors(2);
    if ( ! checkInit() || ! checkOpenMS() )  { 
       errors[0] = "There is no measurement set open";
       log->FnExit(fnname, clname); 
       return errors; 
    }

    //# If we don't have any values then return 0 length vectora.
    if ( rangeStrVec.nelements() == 0 ) {
       rangeVector.resize(0);
       rangesSet.resize(0);
       return errors; 
    }
    
    //# Find the default year, month, and day of the observation. If
    //# the start/end values don't match then create a message.
    Int year = -1, month = -1, day = -1;
    String yearError, monthError, dayError;
    
    if ( itsStartTime.year() == itsEndTime.year() )
       year = itsStartTime.year();
    else
       yearError = "the observations start year and end year differ.";
    
    if ( itsStartTime.month() ==  itsEndTime.month() )
       month = itsStartTime.month();
    else
       monthError = "the observations start month and end month differ.";

    if ( itsStartTime.monthday() == itsEndTime.monthday() )
       day = itsStartTime.monthday();
    else
       monthError = "the observations start day and end day differ.";
    
    //# If we have more then 4 strings there could be a problem so
    //# give the user a warning.
    if ( rangeStrVec.nelements() > 4 )
      errors[1] += String( "plotrange option has too many values, " )
          + String::toString( rangeStrVec.nelements() )
          + String( " were given, expected 4. " );

    //# Now process the 4 values
    rangeVector.resize(4); rangeVector.set(0.0);
    rangesSet.resize(4);   rangesSet.set(False);
   for( uInt i=0; i < 4 && i < rangeStrVec.nelements(); i++ )
   {
   //# Skip empty strings, this is range value that isn't set.
   if ( rangeStrVec[i].length()  < 1 )
        continue;

   Double tmpValue;
   //# If we can convert the value to a double without
   //# a problem then we do.  If not then we assume its
   //# a time expression.
   char *end;
   errno = 0;
   tmpValue = strtod(rangeStrVec[i].c_str(), &end );
   Int tmpErrno = errno;
   String endStr(end);
   if ( ( end == NULL && tmpErrno!=ERANGE ) || endStr.length() < 1 ) 
   {
       //# We have a valid double value so store it.
       rangeVector[i] = tmpValue;
       rangesSet[i] = True;
       continue;
   }

   //# If we have made it here then we have a string, and we assume
   //# it is a time value.
   //# We are looking for something in the form of:
   //# YYYY/MM/DD/hh:mm:ss
   //# If bits are missing we add to the missing bits, if
   //# we can.
      
   String timeString;       
   String tmpStr = rangeStrVec[i];
   Bool done = False;
   while ( !done ) {
       //# Loop through collecting what we have and adding either
       //# default values or found values to the TimeString.
       Int pos = tmpStr.find_first_of("/");
       Int nextPos;
       if ( pos >=0 && pos < (Int)tmpStr.length() )
          nextPos = 
             tmpStr.substr(pos+1,tmpStr.length()-1).find_first_of( "/");
       else
          nextPos = -1;
      
       if ( ( pos < 0 || pos >= (Int)tmpStr.length() || pos==0  )
          || ( nextPos > (Int)tmpStr.length() || nextPos < 0 ) ) {
         //# We have found all the bits of the string relating
         //# to the year, month, and day so depending on what
         //# we have so far, add on the default values.
         uInt numSlashes = timeString.freq( "/" );
         if ( pos >= 0 && pos < (Int)tmpStr.length() || pos==0  ) 
         {
             timeString += tmpStr.substr( 0, pos+1 );
             tmpStr=tmpStr.substr( pos+1, tmpStr.length()-1 );
             numSlashes++;
         }
      
         if ( numSlashes < 1 ) 
            if ( day >= 0 )
               timeString =  String::toString( day ) + String( "/" ) 
                  + timeString;
             else 
               errors[0] += 
                 String( "Day not given for plotrange value: " )
                 + rangeStrVec[i]
                 + String(" and the observations start end end days differ.\n");

      if ( numSlashes < 2 ) 
          //# Add Default Month value.
          if ( month >= 0 )
             timeString = String::toString( month ) 
                + String( "/" ) + timeString ;
          else 
             errors[0] += String( "Month not given for plotrange value: " )
              + rangeStrVec[i]
              + String(" and the observations start end end months differ.\n");

      if ( numSlashes < 3 ) 
          //# Add Default year value.
          if ( year >= 0 )
             timeString = String::toString( year ) + String("/") 
                + timeString;
          else 
             errors[0] += String( "Year not given for plotrange value: " )
              + rangeStrVec[i]
              + String(" and the observations start end end years differ.\n" );
      
          done = True;
      } else {
         if ( nextPos == 0 ) {
            //# We have an empty string so fill in with the default
            uInt numSlashes = timeString.freq( "/" );
            if ( numSlashes == 1 ) 
               //# Add Default year value.
               if ( year >= 0 )
                  timeString += String::toString( year ) + String( "/" );
               else 
                  errors[0] += String( "Year not given for plotrange value: " )
                   + rangeStrVec[i]
                   + " and the observations start end end years differ.\n" ;
            if ( numSlashes == 1 ) 
               //# Add Default Month value.
               if ( month >= 0 )
                    timeString += String::toString( month ) + String( "/" );
               else 
                  errors[0] += String("Month not given for plotrange value: " )
                   + rangeStrVec[i]
                   + " and the observations start end end months differ.\n" ;
         
            if ( numSlashes == 2 ) 
               if ( day >= 0 )
                  timeString += String::toString( day ) + String( "/" );
               else 
                 errors[0] += String( "Day not given for plotrange value: " )
                   + rangeStrVec[i]
                   + " and the observations start end end days differ.\n";
         }  else {   
            //# Add the substring we found between the two slashes.
            timeString += tmpStr.substr( 0, pos+1 );
            tmpStr=tmpStr.substr( pos+1, tmpStr.length()-1 );
         }
      
       }
   }

   
   //# At this point the time string has the YYYY/MM/DD in it.
   //# For now we aren't adding on parsing of the HH:mm:ss bit
   //# we assume that it will be different for all, 
   //# TODO add the parsing
   timeString += tmpStr;
       
   casa::Quantity qa;   
   if ( ! MVTime::read( qa, timeString ) )
   {
       errors[0] += String( "Unable to convert plotrange value: " )
             + rangeStrVec[i] + String( ".\n" );
   } else {
       //# CASA dates are in MJD but matplotlib uses 01/01/0001 as its
       //# first day.  So we need to convert the values as follows:
       //#          Time/86400.0 + 658576.0
       MVTime mt( qa );
       rangeVector[i] = mt.second()/86400.0 + 678576.0;
       rangesSet[i] = True;
   }
   }
        
   log->FnExit(fnname, clname);
   return errors;
}
    
Vector<String>
MsPlot::parseStrToList( const String& inString, Vector<String>& outVector )
{
    String fnname = "parseStrToList";
    log->FnEnter(fnname + "(inString, outVector)", clname);

    Vector<String> errors(2);
    if ( ! checkInit() || ! checkOpenMS() )  { 
       errors[0] = "There is no measurement set open";
       log->FnExit(fnname, clname);
       return errors; 
    }

    //# Strip of any "(", ")", "[", or "]" that might be in the string.
    uInt startPos = 0, endPos = inString.length() - 1;
    uInt sbLen = endPos + 1;
    if((inString.chars())[0] == '(' || ( inString.chars())[0]=='[')
    {
       startPos = 1;
       sbLen = sbLen - 1;
    }
    if((inString.chars())[endPos] == ')' || (inString.chars())[endPos] == ']') 
    sbLen = sbLen-1;
    String strippedStr = inString.substr( startPos, sbLen );

    //# Now split it on comma's 
    Int numCommas = strippedStr.freq(",");
    Int numSpaces = strippedStr.freq(" ");
    uInt numFound=0;

    String stringList[numCommas+numSpaces+1];
    if ( numCommas > 0 ) {
       numFound = split( strippedStr, stringList, numCommas+numSpaces+1, "," );
    } else if ( numSpaces > 0 ) {
       //# This bit remains for backward compatibility, but this 
       //# allows spaces between values, but may be deprecated
       numFound = split( strippedStr, stringList, numCommas+numSpaces+1, " " );
    } else {
       //# We only have one item
       numFound = 1;
       stringList[0] = strippedStr;
    }
    
    //# normalize everything by making it upper case and removed
    //# leading and trailing spaces.
    if ( numFound > 0  ) {
       outVector.resize( numFound );
       for( uInt i=0; i < numFound; i++ )
       {
          String tmpName = upcase( stringList[i] );
          Int spaceIndex;
          while( (spaceIndex = tmpName.index(" ")) >= 0 )
          tmpName.del(spaceIndex,1);
          outVector[i] = tmpName;
       }
    } else {
       outVector.resize(0);
       errors[1] += String("Warning: empty string found, nothing to parse");
    }
    

    log->FnExit(fnname, clname);
    return errors;
}


//#////////////////////////////////////////////////////////////////////////////
//#  Create the Selected reference MS and do averaging 
//#  SDJ: OLD ROUTINE, NOT USED I THINK???
//#////////////////////////////////////////////////////////////////////////////
/*
Bool MsPlot::setData( const String& baselineExpr,
            const String& fieldExpr,
            const String& scanExpr,
            const String& uvDistExpr,
            const String& arrayExpr,
            const String& feedExpr,
            const String& spwExpr,
            const String& corrExpr,
            const String& timeExpr,
            const String& aveMode,
            const int& aveChan,
            const double& aveTime)
{
   String FnCall = "(baselineExpr=" + baselineExpr
                   + " fieldExpr=" + fieldExpr
                   + " uvDistExpr=" + uvDistExpr
                   + " scanExpre=" + scanExpr
                   + " \narrayExpr=" + arrayExpr
                   + " feedExpr=" + feedExpr
                   + " spwExpr=" + spwExpr
                   + " corrExpr=" + corrExpr
                   + " timeExpr=" + timeExpr
                   + " \naveragemode=" + aveMode 
                   + " averagechan=" + String::toString(aveChan)
                   + " averagetime=" + String::toString(aveTime)
                   + ")";

   String fnname = "setData";
   log->FnEnter(fnname + FnCall, clname);


   Bool rstat = True;
   if (!checkInit() || !checkOpenMS())  { 
      log->FnExit(fnname, clname); 
      return rstat=False; 
   }

   //# Initialize the selection string and reset internal variables.
   itsDataSelectString = String( "" );
   resetDataSelection();

   //# Reset the selection MS to the original.
   //# This is done because if nothing is specified, then 
   //# msselection returns back an unchanged itsSelctedMS.
   itsSelectedMS = MS(*itsMS);

   //# Use a helper method to get the timeExpr.  
   String msSelectTimeExpr;
   try {   
      msSelectTimeExpr = getTimeExprStr(timeExpr);
   } catch (AipsError ae) {
      //# Any error would already have been logged so don't
      //# report it again.
      log->out(ae.getMesg(), fnname, clname, LogMessage::SEVERE );
      log->FnExit( fnname, clname);
      return rstat = False;
   }
    
   //# Do the selection.
   //# Note that since we do our own correlation parsing, until
   //# MSSelection is able to give us the index values, we pass
   //# in an empty string for it.  We also pass in an empty 
   //# expression for the TaQL expression.
   //#
   //# Note: the two "empty" fields in the MSSecltion object creation
   //# are for taqlExpr and corrExpr respectively.
   log->out(String("Time Expr:     ") + msSelectTimeExpr + 
            String("\nBaseline Expr: ") + baselineExpr + 
            String("\nField Expr:    ") + fieldExpr + 
            String("\nspwExpr:       ") + spwExpr + 
            String("\nuvDistExpr:    ") + uvDistExpr + 
            String("\nscanExpr:      ") + scanExpr + 
            String("\narrayExpr:     ") + arrayExpr, 
            fnname, clname, LogMessage::DEBUG1);
   try {
      String emptyExpr="";
      MSSelection MSSelectionObj(*itsMS, MSSelection::PARSE_NOW,
                                  msSelectTimeExpr, baselineExpr, fieldExpr,
                                  spwExpr, uvDistExpr, emptyExpr, emptyExpr,
                                  scanExpr, arrayExpr );

      MSSelectionObj.getSelectedMS(itsSelectedMS, String(""));
#if LOG2
      {
         ostringstream os;
         os << "selected ms=" << itsSelectedMS << endl;
         log->out(os, fnname, clname, LogMessage::NORMAL3);
      }
#endif


      if (itsSelectedMS.nrow() < 1 && corrExpr.length() < 1) {
         log->out(String("The data selection resulted in no data."),
                  fnname, clname, LogMessage::WARN );
         rstat = False;
      } else {
          log->out( String( "Number of selected rows is: " )
                  + String::toString(itsSelectedMS.nrow())
                  , fnname, fnname, LogMessage::NORMAL3 );
      }
      
      //# Store the selected spectral windows and display a
      //# debugging message
      itsSelectedSpwIds.resize(0);
      itsSelectedSpwIds = MSSelectionObj.getSpwList();
      
      //# Parse and save the channel selection information needed later
      //# when we construct the TaQL strings for data selection.
      //# The channel list is used later when constructing the output
      //# string.
      Matrix<Int> chanList = MSSelectionObj.getChanList();
      getChannelInfo(chanList);
      
      //# Parse and save the correlation information.  We do the parsing
      //# but it will eventually be done in MSSelection, and be similar
      //# to what is done with channel information.
      //# Note: that before correlations can be parsed we need to 
      //#       know what SPWs have been selected.
      itsSelectedPolIds.resize( 0 );
      Vector<Vector<String> > corrNames( itsMaxPolId );
      if (!corrSelection(corrExpr, corrNames))
          rstat = False;
      
      //if (corrNames.nelements() > 0) {
      //   Int nc = corrNames(0).nelements();
      //   itsAveCorr.resize(nc);
      //   for (Int i = 0; i < nc; i++) {
      //      itsAveCorr(i) = corrNames(0)(i);
      //   } 
      //}
      //cout << " corr=" << itsAveCorr << endl;
       
       
      if (!rstat) {
          log->FnExit( fnname, clname);
          return rstat;
      }
   
      //# (2) Create a String that contains information about the selection.
      //#     And display it, to let the user know what they've selected.
      if (makeDataSelectStr(MSSelectionObj, corrNames, msSelectTimeExpr))
      {
          String msg = 
               String("Data to be selected from matches the following: ")
               + itsDataSelectString
               + String( "\n\nNote that NOT all of the points falling with in ")
               + String( "the bounds of the above\nselection may be plotted." )
               + String( " For example, the flagged data is plotted\nonly " )
               + String( " if 'showflags' is set to True." );
          log->out( msg, fnname, clname, LogMessage::NORMAL2);
      } else {
         //# Some error
      }
   
   } catch (AipsError ae) {
       log->out( String( "Parse Error with in MSSelection" ) + ae.getMesg(),
          fnname, clname, LogMessage::WARN, True);
       rstat = False;
   }



   //# Signal a successful new selection.
   if ( rstat )
      itsIsNewSelection = True;

   log->FnExit(fnname, clname);
   return rstat;
}
*/

//#////////////////////////////////////////////////////////////////////////////
//# Method responsible for set flag extension
//#
//#////////////////////////////////////////////////////////////////////////////
Bool 
MsPlot::setFlagExtension( const String& extendcorr,
                 const String& extendchan, 
                 const String& extendspw,
                 const String& extendant,
		 const String& extendtime )
{
   String FnCall = "(extendcorr=" + extendcorr
                   + " extendchan=" + extendchan
                   + " extendspw=" + extendspw
                   + " extendant=" + extendant
                   + " extendtime=" + extendtime
                   + ")";
   String fnname = "setFlagExtension";
   log->FnEnter( fnname + FnCall, clname );

   Bool rstat = True;
   if ( ! checkInit() || ! checkOpenMS() )  { 
      log->FnExit( fnname, clname); 
      return rstat=False; 
   }

   itsExtendCorr = extendcorr;
   itsExtendChan = extendchan;
   itsExtendSpw = extendspw;
   itsExtendAnt = extendant;
   itsExtendTime = extendtime;
   //cout << " itsExtendCorr=" + itsExtendCorr
   //     << " itsExtendChan=" + itsExtendChan
   //     << " itsExtendSpw=" + itsExtendSpw
   //     << " itsExtendAnt=" + itsExtendAnt
   //     << " itsExtendTime=" + itsExtendTime
   //     << endl;

   log->FnExit(fnname, clname);
   return rstat;
}
//#////////////////////////////////////////////////////////////////////////////
//# Method responsible for selecting the data.
//#
//#  Create the Selected reference MS
//#////////////////////////////////////////////////////////////////////////////
Bool 
MsPlot::setData( const String& baselineExpr,
                 const String& fieldExpr, 
                 const String& uvDistExpr,
                 const String& scanExpr,
                 const String& arrayExpr,
                 const String& feedExpr,
                 const String& spwExpr,
		 const String& corrExpr,
		 const String& timeExpr )
{
   String FnCall = "(baselineExpr=" + baselineExpr
                   + " fieldExpr=" + fieldExpr
                   + " scanExpre=" + scanExpr
                   + " uvDistExpr=" + uvDistExpr
                   + " arrayExpr=" + arrayExpr
                   + " feedExpr=" + feedExpr
                   + " spwExpr=" + spwExpr
       		   + " corrExp=" + corrExpr
                   + " timeExpr=" + timeExpr
                   + ")";
   //cout << FnCall << endl;
   String fnname = "setData";
   log->FnEnter( fnname + FnCall, clname );
   Bool rstat = True;
   if ( ! checkInit() || ! checkOpenMS() )  { 
      log->FnExit( fnname, clname); 
      return rstat=False; 
   }

   //Timer tmr;
   // TODO Move this to init.
   ROMSColumns msColumn(*itsMS);
   const ROScalarColumn<Double>& timeInterval = msColumn.interval();
   Int nrow = timeInterval.nrow();
   itsMinTimeInterval = 86400.;
   for (Int i=0; i < nrow; i++) {
       if (itsMinTimeInterval > timeInterval(i)) {
          itsMinTimeInterval = timeInterval(i);
       }
   }

   //# Initialize the selection string and reset internal variables.
   itsDataSelectString = String( "" );
   resetDataSelection();

   //# Reset the selection MS to the original.
   //# This is done because if nothing is specified, then 
   //# msselection returns back an unchanged itsSelctedMS.
   itsSelectedMS = MS(*itsMS);

   //# User a helper method to get the timeExpr.  Note that,
   //# this method also sets up skipnrows or averagenrows
   //# getting set when updatePlotOptions for timeplots.
   String msSelectTimeExpr;

   try {  
       // TODO?? Honglin has one that maybe uses averagint
       // msSelectTimeExpr = getTimeExprStr(timeExpr, aveMode);
       msSelectTimeExpr = getTimeExprStr(timeExpr);
   } 
   catch (AipsError ae) {
      //# Any error would already have been logged so don't
      //# report it again.
      log->out(ae.getMesg(), fnname, clname, LogMessage::SEVERE);
      log->FnExit(fnname, clname);
      return rstat = False;
   }

   //# Do the selection.
   //# Note that since we do our own correlation parsing, until
   //# MSSelection is able to give us the index values, we pass
   //# in an empty string for it.  We also pass in an empty 
   //# expression for the TaQL expression.
   //#
   //# Note: the two "empty" fields in the MSSecltion object creation
   //# are for taqlExpr and corrExpr respectively.
   String sel = String("Time Expr:     ") + msSelectTimeExpr +
            String("\nBaseline Expr: ") + baselineExpr +
            String("\nField Expr:    ") + fieldExpr +
            String("\nspwExpr:       ") + spwExpr +
            String("\nuvDistExpr:    ") + uvDistExpr +
            String("\nscanExpr:      ") + scanExpr +
            String("\narrayExpr:     ") + arrayExpr;
   log->out(sel, fnname, clname, LogMessage::DEBUG1);
   //cout << "sel=" << sel << endl;
   
   try {
      String emptyExpr="";
      MSSelection MSSelectionObj(*itsMS,MSSelection::PARSE_NOW,
                                 msSelectTimeExpr, baselineExpr, fieldExpr,
                                 spwExpr, uvDistExpr, emptyExpr, emptyExpr,
                                 scanExpr, arrayExpr );

      if (itsAveChan==1234567) {
         //cout << "(allspw) spwExpr=  " << spwExpr << endl; 
         Vector<Int> selSpw = MSSelectionObj.getSpwList();
         //cout << "selSpw=" << selSpw << endl;
         String spwstr = "";
         for (uInt i = 0; i < selSpw.nelements(); i++) {
             spwstr += ((i > 0) ? "," : "");
             spwstr += String::toString(selSpw[i]);
         }
         //cout << "spwstr=" << spwstr << endl;
         MSSelectionObj.setSpwExpr(spwstr);
      }

      MSSelectionObj.getSelectedMS(itsSelectedMS,String(""));

      {
         ostringstream os;
         os << "selected ms=" << itsSelectedMS << endl;
         log->out(os, fnname, clname, LogMessage::NORMAL3);
      }
      //cout << "selected ms=" << itsSelectedMS << endl;

      //cout << "itsSelectedMS.nrow()=" << itsSelectedMS.nrow() << endl;
      //cout << "corrExpr=" << corrExpr << endl;
      if (itsSelectedMS.nrow() < 1 && corrExpr.length() < 1) {
         log->out(String("The data selection resulted in no data."),
                  fnname, clname, LogMessage::WARN );
         rstat = False;
      } 
      else {
         log->out(String( "Number of selected rows is: " )
		 + String::toString(itsSelectedMS.nrow()),
                 fnname, fnname, LogMessage::NORMAL3 );
      }
   
      //# Store the selected spectral windows and display a
      //# debugging message
      itsSelectedSpwIds.resize(0);
      itsSelectedSpwIds = MSSelectionObj.getSpwList();
      
      //# Parse and save the channel selection information needed later
      //# when we construct the TaQL strings for data selection.
      //# The channel list is used later when constructing the output
      //# string.
      Matrix<Int> chanList = MSSelectionObj.getChanList();
      getChannelInfo( chanList );
      
      //# Parse and save the correlation information.  We do the parsing
      //# but it will eventually be done in MSSelection, and be similar
      //# to what is done with channel information.
      //# Note: that before correlations can be parsed we need to 
      //#       know what SPWs have been selected.
      itsSelectedPolIds.resize( 0 );
      Vector<Vector<String> > corrNames( itsMaxPolId );
      if ( ! corrSelection( corrExpr, corrNames) )
          rstat = False;
      
      if ( !rstat )
      {
          log->FnExit( fnname, clname);
          return rstat;
      }
          
      //# (2) Create a String that contains information about the selection.
      //#     And display it, to let the user know what they've selected.
      if (makeDataSelectStr(MSSelectionObj, corrNames, msSelectTimeExpr))
      {
          String msg = 
               String("Data to be selected from matches the following: ")
               + itsDataSelectString
             //  + String( "\n\nNote that NOT all of the points falling with in ")
             //  + String( "the bounds of the above\nselection may be plotted." )
             //  + String( " For example, the flagged data is plotted\nonly " )
             //  + String( " if 'showflags' is set to True." )
             ;
          log->out( msg, fnname, clname, LogMessage::NORMAL);
      } 
      else {
         //# Some error
      }
   } catch (AipsError ae) {
      log->out(String("Parse Error with in MSSelection ") + ae.getMesg(),
                fnname, clname, LogMessage::WARN, True);
      rstat = False;
   }

   //# Signal a successful new selection.
   if ( rstat )
      itsIsNewSelection = True;
   //tmr.show("Selection: ");
   log->FnExit(fnname, clname);
   return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//#  Create do averaging on the Selected reference MS 
//#
//# THIS NEEDS MAJOR CLEANUP, OLD AND NEW AVERAGING MIXED IN HERE.
//#
//# FOR THE FUTURE MAYBE
//# Allowable expressions are:
//# operator: number ~ number   (range of values)
//#           > number
//#           >= number
//#           < number
//#           <= number
//# spwExpr: Same as for set data.
//#          [spwId]:channel(s)
//#          spwId:channel(s)[;spwId:channel(s)]
//#          channel(s) = chanNumber[,chanNumber] | operator chanNumber
//#          NOTE: Frequency can also be given but the values must include
//#                the units
//# corrExpr: Same as for set data
//#           some combination of the valid correlation values:
//#           RR, LL, RL, LR, XX, YY, XY, YX, ...
//# timeExpr: Same as for set data
//#           timeExpr[,timeExpr]
//#           timeExpr= operator time (in YY:MM:DD;
//# 
//#////////////////////////////////////////////////////////////////////////////
Bool MsPlot::average(
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
                 const Bool& aveVel)
{
   String FnCall = "( chanAveMode=" + chanAveMode
                   + " corrAveMode=" + corrAveMode
                   + " averagedata=" + aveData
                   + " averagemode=" + aveMode 
                   + " averagechan=" + aveChan
                   + " averagetime=" + aveTime
                   + " aveFlag=" + String::toString(aveFlag)
                   + " aveScan=" + String::toString(aveScan)
                   + " aveBl=" + String::toString(aveBl)
                   + " aveArray=" + String::toString(aveArray)
                   + " aveChanId=" + String::toString(aveChanId)
                   + " aveVel=" + String::toString(aveVel)
                   + ")";

   String fnname = "average";
   log->FnEnter(fnname + FnCall, clname);

   Bool rstat = True;
   if (!checkInit() || !checkOpenMS())  { 
      log->FnExit(fnname, clname); 
      return rstat=False; 
   }	

   //as long as it go through here, the itsSelectedMS 
   //will get resset, a reopen is therefore unnecessary
   //
   //Bool needOpen = (itsAveTime != atof(aveTime.data()) 
   //                 || itsAveChan != atoi(aveChan.data()) 
   //                 || itsAveMode != upcase(aveMode) 
   //                 || itsAveFlag != aveFlag); 
   //if (needOpen) {
   //   //String path = itsMS->tableName();
   //   //itsMsIsOpen = False;
   //   //itsMS->reopenRW();
   //   //open(path);
   //   //itsTablePlot->reset();
   //   ;
   //}   

   // Set everything to its default values.

   //itsAveMode hold averagemode=[vector, scalar, '']
   itsAveMode.resize(0);
   itsAveChan = 1;
   itsAveTime = 0.0;
   itsAveFlag = False;
   itsAveScan = False;
   itsAveBl = False;
   itsAveArray = False;
   itsDataColumn = String( "data" );
   useAveChanId = False;
   

   //itsAveMode hold averagemode=[vector, scalar, '']
   itsAveMode = upcase(aveMode);
   itsAveChan = atoi(aveChan.data());
   itsAveTime = atof(aveTime.data());
   itsAveFlag = aveFlag;
   itsAveScan = aveScan;
   itsAveBl = aveBl;
   itsAveArray = aveArray;
   itsDataColumn = upcase(aveData);
   useAveChanId = aveChanId;
   doVel = aveVel;
 

   ROMSColumns msColumn(*itsMS);
   const ROScalarColumn<Double>& timeInterval = msColumn.interval();
   Int nrow = timeInterval.nrow();
   itsMinTimeInterval = 86400.;
   for (Int i=0; i < nrow; i++) {
       if (itsMinTimeInterval > timeInterval(i)) {
	   itsMinTimeInterval = timeInterval(i);
       }
   }

   //cout << "chanAveMode=" << chanAveMode << endl;
   //if ((upcase(chanAveMode).matches("VECTORCHUNK") ||
   //     upcase(chanAveMode).matches("SCALARCHUNK")) && 
   //    itsAveTime <= itsMinTimeInterval && itsAveChan == 1) { 
   //   itsAveTime = 0.;
   //   log->out(String("Can not average with given averaging parameters"),
   //           fnname, clname, LogMessage::WARN);
   //   return rstat = False;
   //}
 
   //cout << "itsAveChan=" << itsAveChan << " itsAveTime=" << itsAveTime
   //     << " itsAveFlag=" << itsAveFlag << " itsAveMode=" << itsAveMode
   //     << " itsAveScan=" << itsAveScan 
   //     << " itsMinTimeInterval=" << itsMinTimeInterval << endl;

   //# Parse all of the channel and correlation averaging
   //# modes.  The modes are:
   //#    None:   No averaging
   //#    Step:   Select every nth data point
   //#    StepScalar: Do a incoherent average with every nth point
   //#    StepVector: Do a coherent average with every nth point
   //#    ChunkScalar: Do a incoherent average with blocks of n points
   //#    ChunkVector: Do a coherent average with block of n points
   if (!checkAverageMode("chanavemode", chanAveMode ) 
	|| !checkAverageMode("corravemode", corrAveMode ) ) {
       rstat = False;
   }		 
   else if (upcase(chanAveMode).matches("NONE") 
	&& upcase(corrAveMode).matches("NONE")) {
       itsAverage[0] = String("");
       itsAverage[1] = corrAveMode;
   } 
   else if (upcase(corrAveMode).matches(upcase(chanAveMode))) {
       itsAverage[0] = String("both");
       itsAverage[1] = chanAveMode;
   } 
   else if (upcase(chanAveMode).matches("NONE")) {
       itsAverage[0] = String("corr");
       itsAverage[1] = corrAveMode;
   } 
   else if (upcase(corrAveMode).matches("NONE")) {
       itsAverage[0] = String("chan");
       itsAverage[1] = chanAveMode;
   }	 
   else {
       //# If we are here the user has set both the channel and
       //# correlation averaging which we need to be the same type
       //# do to the way we do the averaging. So it is an error if
       //# they aren't the same.
       log->out(String( "Averaging modes do not match. Due to the way "
	"channel and correlation averaging\nare done they"
	" must have the same mode when doing averaging on"
	" both of them." ),
	fnname, clname, LogMessage::SEVERE );
       rstat = False;
      }

   log->FnExit(fnname, clname);
   return rstat;
}



//#////////////////////////////////////////////////////////////////////////////
//# Construct a string describing what was selecteced for display purposes
//#
//#
Bool 
MsPlot::makeDataSelectStr( MSSelection& MSSelectObj, 
   const Vector< Vector<String> >& selectedCorrs,  
   const String& selectedTimes )
{
    String fnname = "makeDataSelectStr";
    log->FnEnter(fnname + "(MSSelectObj, selectedCorrs)", clname);

    //# TODO ADD array information
    
    //# Add Baseline Info.
    //cout << "itsDataSelectString=" << itsDataSelectString << endl;
    itsDataSelectString += String("\n\tBaselines: ");
    Matrix<Int> baselines = MSSelectObj.getBaselineList();
    //cout << "baselines=" << baselines << endl;
    //cout << "itsAntennaNames=" << itsAntennaNames << endl;
    //cout << "baselines.shape()=" << baselines.shape() << endl;
    itsBaselines.resize(baselines.shape()[0], baselines.shape()[1]);
    itsBaselines = baselines;
    if( baselines.shape().nelements() == 2  && baselines.shape()[1] == 2 
        && baselines.shape()[0] != 0) 
    {
       //# Expected shape is (2,N) -- where N is the number of baselines.
       for ( Int i=0; i < (baselines.shape())[0]; i++ )
       {
          if ( i > 0 )
             itsDataSelectString += String( ", " );

          //cout << "i=" << i << " baselines(i,0)=" << baselines(i,0)
          //     << " baselines(i,1)=" << baselines(i,1)  
          //     << " itsBases(i,0)=" << itsBaselines(i,0)
          //     << " itsBaselines(i,1)=" << itsBaselines(i,1) << endl; 
          
          Int a1 = baselines(i,0);
          Int a2 = baselines(i,1); 
          itsDataSelectString += String( "[" ) 
                 + String((a1 < 0) ? "!" : "")
                 + itsAntennaNames(abs(a1))
                 + String( ", " ) 
                 + String((a2 < 0) ? "!" : "")
                 + itsAntennaNames(abs(a2))
                 + String( "]" );
      }
   } else {
      itsDataSelectString += String( "*ALL pairs of* -- " );
      for ( uInt i=0; i < itsAntennaNames.nelements(); i++ )
      {
          if ( i > 0 )
             itsDataSelectString += String( ", " );      
          itsDataSelectString += itsAntennaNames[i];
      }
   }
   //# Add the field names
   itsDataSelectString += String("\n\tFields: ");
   Vector<Int> fields = MSSelectObj.getFieldList();
   if ( fields.nelements() > 0 ) {
      for ( uInt i=0; i<fields.nelements(); i++ )
      { 
         if ( i > 0 )
            itsDataSelectString += String( ", " );
         itsDataSelectString += itsFieldNames( fields[i] );
      }
   } else {
      itsDataSelectString += String(" *ALL* -- ");
      for (uInt i=0; i< itsFieldNames.nelements(); i++ )
      {
         if ( i > 0 )
            itsDataSelectString += String( ", " );      
         itsDataSelectString += itsFieldNames[i];
      }
   }


    //# Add SPW indexes -- this may need to change if spw's ever get
    //# names, which may be the case for ALMA data (TODO)
    //# TODO, verify shape of channel list to make sure its the
    //# expected shape.
       
    Matrix<Int> chanList = MSSelectObj.getChanList();
    IPosition chanShape = chanList.shape();
    itsDataSelectString += String("\n\tSpectral Windows: ");
    if ( chanShape[1] > 0 )
    {
       //Wrong in many ways! 
       //if select multiple chunks from same spw, itsSelectedSpwIds
       //contains same spwid multiple times. also, there may be
       //multiple chanList rows for one spw
       //the result is that some spw:chan chunks are missing while
       //some others repeat multiple times
       /*
       for (uInt i=0; i < itsSelectedSpwIds.nelements(); i++ ) {
          itsDataSelectString += String( "\n\t\tSPW " )
              + String::toString( itsSelectedSpwIds[i] ) + String( ": ");
          Bool found = False;
          for ( Int j=0; j < chanShape[1] && !found; j++ )
          {
             if ( chanList(j,0) == itsSelectedSpwIds[i] )
             {
                //# There are channels selected for this spw.
                //# Note we need to add 1, as TaQLs are 1 based
                //# but users use 0-based selection
                itsDataSelectString += String( "Channels " )
                   + String::toString( chanList(j,1) + 1)
                   + String( " to " )
                   + String::toString( chanList(j,2) + 1)
                   + String( " with a step of " )
                   + String::toString( chanList(j,3) );
                found = True;
                break;
             }
          }
          for (Int j = 0; j < chanShape[1]; j++) {
             if (chanList(j, 0) == itsSelectedSpwIds[i] ) {
                //# There are channels selected for this spw.
                //# Note we need to add 1, as TaQLs are 1 based
                //# but users use 0-based selection
                itsDataSelectString += String("Channels ")
                   + String::toString(chanList(j, 1) + 1)
                   + String(" to ")
                   + String::toString( chanList(j, 2) + 1)
                   + String(" with a step of ")
                   + String::toString(chanList(j, 3));
                found = True;
             }
          }
          if ( ! found )
          {
             itsDataSelectString += String( "Channels *All* -- 1 to " ) 
                + String::toString( itsSpwIdNChans[itsSelectedSpwIds[i]] )
                + String( " with a step of 1" );
          }   
       }
       */

       /* this is correct but why not the simplest !
       //cout << "chanList=" << chanList << endl;
       Vector<Int> spwids(itsSelectedSpwIds.nelements());
       uInt k = 0;
       for (uInt i = 0; i < itsSelectedSpwIds.nelements(); i++) {
          uInt j = 0;
          for (j = 0; j < k; j++) {
             if (itsSelectedSpwIds[i] == spwids(j)) {
                break; 
             }
          }
          if (j == k) {
             spwids(j) = itsSelectedSpwIds(i);
             k++;
          }
       }
       spwids.resize(k, True);
       //cout << "spwids=" << spwids << endl;
       for (uInt i = 0; i < spwids.nelements(); i++) {
          itsDataSelectString += String( "\n\t\tSPW " )
              + String::toString(spwids[i]) + String(": ");
          Bool found = False;
          for (Int j = 0; j < chanShape[1]; j++) {
             if (chanList(j, 0) == spwids[i]) {
                //# There are channels selected for this spw.
                //# Note we need to add 1, as TaQLs are 1 based
                //# but users use 0-based selection
                itsDataSelectString += String("Channels ")
                   + String::toString(chanList(j, 1) + 1)
                   + String(" to ")
                   + String::toString( chanList(j, 2) + 1)
                   + String(" with a step of ")
                   + String::toString(chanList(j, 3));
                found = True;
             }
          }
          if (!found)
          {
             itsDataSelectString += String( "Channels *All* -- 1 to " ) 
                + String::toString( itsSpwIdNChans[itsSelectedSpwIds[i]] )
                + String( " with a step of 1" );
          }   
       }
       */
          if (chanShape[0] == 0) {
            for (uInt i = 0; i < itsSelectedSpwIds.nelements(); i++)
             itsDataSelectString += String( "Channels *All* -- 1 to " ) 
                + String::toString(itsSpwIdNChans[itsSelectedSpwIds[i]])
                + String( " with a step of 1" );
          }   
          else {
             for (Int j = 0; j < chanShape[0]; j++) {
                if (j > 0)
                   itsDataSelectString += String(", ");
                itsDataSelectString += String::toString(chanList(j, 0))
                   + String(":")
                   + String::toString(chanList(j, 1))
                   + String("~")
                   + String::toString(chanList(j, 2))
                   + String("^")
                   + String::toString(chanList(j, 3));
             }
          }
   } else {
      itsDataSelectString += String(" *ALL* -- ");
      for (uInt i=0; i <itsMaxSpwId; i++ )
      {
          if ( ! itsUsedSpws[i] )
              continue;
       
          itsDataSelectString += String( "\n\t\tSPW " )
             + String::toString( i )
             + String( ": *ALL Channels* -- 1 to " )
             + String::toString( itsSpwIdNChans[i] )
             + String( " with a step of 1" );
       }      
   }
    
   //# Earlier in this method we found the correlation names
   //# when we parsed the correlation information. Note that
   //# our parser makes sure that there is somthing in the list,
   //# ie. if the user didn't select any correlations we use them all.
   if ( selectedCorrs.nelements() > 0 )
   {
       itsDataSelectString += "\n\tCorrelations: ";
       for( uInt i=0; i < selectedCorrs.nelements(); i++ )
       {
           itsDataSelectString += String( "\n\t\tCorr. ID " ) + 
                 String::toString( i ) + String( " - " );
           if ( selectedCorrs[i].nelements() < 1 )
           {
              itsDataSelectString += String( "*NONE*" );
           } else {
              for ( uInt j=0; j<selectedCorrs[i].nelements(); j++ )
              {
                 if ( j > 0 )
                    itsDataSelectString += ", ";
                 itsDataSelectString += selectedCorrs[i][j];
              }
           }
       }
   } else {
       itsDataSelectString += String("\n\tCorrelations: *ALL* -- ");
       for (uInt i=0; i < itsCorrelationNames.nelements(); i++ )
       {
           itsDataSelectString += String( "\n\t\tCorr ID " )
                     + String::toString( i )
                     + String( ": *ALL* -- " );
           for ( uInt j=0; j < itsCorrelationNames[i].nelements(); j++ )
           {
               if ( j > 0 )
                   itsDataSelectString += String( ", " );
               itsDataSelectString += itsCorrelationNames[i][j];
           }
           itsDataSelectString += String( "\n" );
       }      
    }    
    
    //#Add time information
    itsDataSelectString += String( "\n\tTime Range ");
    if ( selectedTimes.length() < 1 )
    {   
       itsDataSelectString += String( " *ALL* --  " )
           + String::toString( itsStartTime.year() ) + String( "/" )
           + String::toString( itsStartTime.month() ) + String( "/" )
           + String::toString( itsStartTime.monthday() ) + String( "/" )
           + itsStartTime.string()
           + String( " to " )
           + String::toString( itsEndTime.year() ) + String( "/" )
           + String::toString( itsEndTime.month() ) + String( "/" )
           + String::toString( itsEndTime.monthday() ) + String( "/" )
           + itsEndTime.string();
     } else {
       itsDataSelectString += String( ": " ) + selectedTimes;
     }

    //# Add the Scan information
    itsDataSelectString += String("\n\tScan Numbers: ");
    Vector<Int> scans = MSSelectObj.getScanList();
    if( scans.nelements() ) 
    {
       for (uInt i=0; i< scans.nelements(); i++ )
       {
           if ( i > 0 )
              itsDataSelectString += String( ", " );      
           itsDataSelectString += String::toString( scans[i] );
        }   
    } else {
       itsDataSelectString += String(" *ALL* -- ");
       for( uInt i=0; i<itsScanNumbers.nelements(); i++ )
       {
          if ( i > 0 )
             itsDataSelectString += String( ", " );
          itsDataSelectString += String::toString( itsScanNumbers[i] );
       }
    }

    //# TODO add feed when supported by MSSelection

    //# getUVList() returns a 2xN list of min/max values.
    //# getUVUnitsList() return a 1xN list of units (meters or lambda)
    //# TODO, We assume that uvDistList returns a 2xN matrix of 
    //# min/max values we really should verify its shape before using it.
    //# We also assume that the uvUnits is the assume length, this 
    //# shoule be verified as well.
    
    Matrix<Double> uvDists = MSSelectObj.getUVList();
    Vector<Bool> uvUnits = MSSelectObj.getUVUnitsList();
    if ( (uvDists.shape())[1] < 1 )
        itsDataSelectString += String("\n\tUVRanges: *ALL*");
    else {
       itsDataSelectString += String("\n\tUVRanges: ");
       for ( Int i=0; i < (uvDists.shape())[1]; i++ )
       {
          if ( i > 1 )
             itsDataSelectString += String( ", " );
             itsDataSelectString += String::toString( uvDists(i,0) )
                + String( " to " )
                + String::toString( uvDists(i,1) );
             if ( uvUnits[i] )
                itsDataSelectString += String( " (meters) " );
             else
                itsDataSelectString += String( " (lambda) " );
       }   
    }   
    
    log->FnExit(fnname, clname);
    return True;
}
    

//#////////////////////////////////////////////////////////////////////////////
//# Helper method for getChannelInfo
//# It adds a single set of start,stop,step values for a particular 
//# SPW 
//#
casa::Bool
MsPlot::setSingleChannelSet( uInt spwId, Int start, Int stop, Int step )
{
    String FnCall = String( "( " )
   + String::toString( spwId ) + String( ", " )
   + String::toString( start ) + String( ", " )
   + String::toString( stop ) + String( ", " )
   + String::toString( step ) + String( " )" );
    String fnname = "setSingleChannelSet";
    log->FnEnter( fnname + FnCall, clname );

    Bool rstat = True;
    if ( ! checkInit() || itsMS==NULL )  
    { 
   log->FnExit( fnname, clname); return rstat = False; 
    }

    //# Note that it is assumed that that 
    //#     itsStartChan
    //#     itsEndChan
    //#     itsStepChan
    //#  are all the same length.
    //# 
    //# TODO we should add an assert here so things don't crash if
    //# they are different lengths for some reason.
    

    //# If we already have something at that given position we need
    //# to append the info on to the end of the list.
    uInt last = itsStartChan[spwId].nelements();
    //# -1 is the default value, we are replacing it not adding to
    //# the end if we see this.
    if ( itsStartChan[spwId][last-1] == -1 )
   last -= 1;
   
    itsStartChan[spwId].resize(last+1, True);
    itsStartChan[spwId][last] = start+1;
    
    itsEndChan[spwId].resize(last+1, True);
    itsEndChan[spwId][last] = stop+1;
    
    itsStepChan[spwId].resize(last+1, True);
    itsStepChan[spwId][last] = step;

    log->FnExit( fnname, clname);
    return rstat;
}
//#////////////////////////////////////////////////////////////////////////////
//# Extract the selected channels from the MSSelection object.
//#
//# The channel information is in a 4xN matrix as follows:
//#     0: Spectral window index
//#     1: Start channel
//#     2: Stop Channel
//#     3: Step (or average).
//#
Bool
MsPlot::getChannelInfo( Matrix<Int> chanSelectList  )
{
    String FnCall = "( chanSelectList )";
    String fnname = "getChannelInfo";
    log->FnEnter( fnname + FnCall, clname );

    Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { log->FnExit( fnname, clname); return rstat; }

    //# It's assume that we've already done our selection of information.
    IPosition chanShape = chanSelectList.shape();

    chanList.resize(0, 0);
    chanList = chanSelectList;

    //cout << "chanList=" << chanList << endl;
    
    //# We use the position of the information as the SPW index, for example
    //# itsStartChan[3], itsEndChan[3]. and itsSteChan[3] all contain the
    //# channel selection information for SPW index 3.  
    //# The lists then need to be as big as the number of SPWs we have, we
    //# also initialize them to -1, meaning no selection made.

    //# TODO make sure these are really cleared properly
    itsStartChan.resize( itsMaxSpwId );
    itsEndChan.resize( itsMaxSpwId );
    itsStepChan.resize( itsMaxSpwId );
    for ( uInt i=0; i < itsMaxSpwId; i++ )
    {
   itsStartChan[i].resize(1);   itsStartChan[i][0] = -1; 
   itsEndChan[i].resize(1);   itsEndChan[i][0] = -1;  
   itsStepChan[i].resize(1);   itsStepChan[i][0] = -1;
    }
    
    //# Lets find out what we've got!
    for ( Int i=0; i < chanShape[0]; i++ )
    {
   setSingleChannelSet( chanSelectList(i,0), chanSelectList(i,1),
          chanSelectList(i,2), chanSelectList(i,3) );   
    }

    //# TODO add some verification of stuff here.
    /*    
    for ( uInt i=0; i < itsStartChan.nelements(); i++ )
    {
   cout << "Channel Info for SPW ID " << i << ": "
        << itsStartChan[i] << " -- " << itsEndChan[i]
        << " -- " << itsStepChan[i] << endl;
    }
    */

    log->FnExit( fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//# If the user hasn't called the setdata method then there are no
//# restrictions on the data that is being selected from. This method
//# constructs a string, for the selection information string display
//# by the logger to indicate this.
Bool
MsPlot::summary( Bool selected )
{
    String FnCall = String( "( " ) + String::toString( selected )
         + String( " )" );
    String fnname( "summary" );
    log->FnEnter( fnname + FnCall, clname );

    Bool rstat = True;
    if ( ! checkInit() || itsMS==NULL )  
    { 
       log->FnExit( fnname, clname); 
       return rstat = False; 
    }

    String displayStr = String( "" );
    if ( !selected ||  itsDataSelectString.length() < 1 )
    {
       //# We need to display the default information, which
       //# describes the whole MS.
       displayStr += defaultDataSelectString();
    } else {
       if ( itsDataSelectString.length() < 1 )
          displayStr += defaultDataSelectString();
       else
          displayStr += itsDataSelectString;
    }

    String msg = String( "data selection is: " ) + displayStr;
    log->out( msg, fnname, clname, LogMessage::NORMAL );
    log->FnExit( fnname, clname);

    return rstat;
}
    
//#////////////////////////////////////////////////////////////////////////////
//# If the user hasn't called the setdata method then there are no
//# restrictions on the data that is being selected from. This method
//# constructs a string, for the selection information string display
//# by the logger to indicate this.
String
MsPlot::defaultDataSelectString()
{
    String fnname = "defaultDataSelectString";
    log->FnEnter(fnname, clname);
    
    String retValue;

    //# List all of the antennas that can be used in baseline selection.
    retValue += String("\n\tBaselines: *ALL pairs of* -- ");
    for ( uInt i=0; i<itsAntennaNames.nelements(); i++ ){
       if ( i > 0 )
          retValue += String( ", " );
       retValue += itsAntennaNames[i];
    }

    //# Add the list of Field names
    retValue += String("\n\tFields: *ALL* -- ");
    for ( uInt i=0; i<itsFieldNames.nelements(); i++ ){
       if ( i > 0 )
          retValue += String( ", " );
       retValue += itsFieldNames[i];
    }

    //#  get the list of allowed indexes for spws:
    retValue += String("\n\tSpectral Windows: *ALL* -- ");
    for (uInt i=0; i <itsMaxSpwId; i++ )
    {       
       if ( ! itsUsedSpws[i] )
          continue;

       retValue += String( "\n\t\tSPW " )
          + String::toString( i )
          + String( ": *ALL Channels* -- 1 to " )
          + String::toString( itsSpwIdNChans[i] )
          +  String( " with a step of 1" );
    }

    //# Add the full list of correlations for this MS
    retValue += String( "\n\tCorrelations: *ALL* -- " );   
    for ( uInt polId=0; polId<itsCorrelationNames.nelements(); polId++ )
    {
       retValue += String( "\n\t\tCorr row " ) + String::toString( polId )
            + String( ": " );
       for ( uInt i=0; i < itsCorrelationNames[polId].nelements(); i++ )
       {
           if ( i > 0 )
              retValue += String( ", " );      
           retValue += itsCorrelationNames[polId][i];
       }
    }

    //# Add time range of the observation
    retValue += String( "\n\tTime Range: *ALL* -- " )
       + String::toString( itsStartTime.year() ) + String( "/" )
       + String::toString( itsStartTime.month() ) + String( "/" )
       + String::toString( itsStartTime.monthday() ) + String( "/" )
       + itsStartTime.string()
       + String( " to " )
       + String::toString( itsEndTime.year() ) + String( "/" )
       + String::toString( itsEndTime.month() ) + String( "/" )
       + String::toString( itsEndTime.monthday() ) + String( "/" )
       + itsEndTime.string();

    //# Add the scan number
    retValue += String("\n\tScan Numbers: *ALL* -- ");
    for ( uInt i=0; i<itsScanNumbers.nelements(); i++ )
    {
       if ( i > 0 )
          retValue += String( ", " );
       retValue+= String::toString( itsScanNumbers[i] );
    }

    //# TODO add feed information.

    //# TODO:  It would be nice to find the max/min uvrange.
    retValue += String("\n\tUVRanges: *ALL*");

    log->FnExit( fnname, clname);
    return retValue;
}

//#////////////////////////////////////////////////////////////////////////////
//# Helper method for setInputTable
//# It adds a single table to the class variables (itsTableVector and
//# itsTableVectorName) for use later.
//#
casa::Bool
MsPlot::setSingleInputTable( const Vector<String>& sortOrder, 
      const uInt spwId, 
      const uInt polId, 
      const Table table, 
      const String tableName )
{
    String FnCall  = "(sortOrder, spwId, polId, table, tableName)";
    String fnname = "setSingleInputTable";
#if LOG2
    log->FnEnter( fnname + FnCall, clname );
#endif

    Bool rstat = True;
    if ( ! checkInit() || itsMS==NULL )  
    { 
#if LOG2
       log->FnExit( fnname, clname); 
#endif
       return rstat = False; 
    }

    //# Note that it is assumed that that 
    //#     itsTableVector,
    //#     itsTableVecNames, and
    //#     itsTableVecSpwIds
    //#     itsTableVecPolIds
    //#  are all the same length.
    //# 
    //# TODO we should add an assert here so things don't crash if
    //# they are different lengths for some reason.

    //# First we need to find where this table goes. The first "sort"
    //# field indicates which value, spw or pol, need to match for a set
    //# of tables.
    Int position = -1;
    for ( uInt i=0; i < itsTableVectors.nelements(); i++ )
    {
       if ( itsTableVecSpwIds[i] == spwId && itsTableVecPolIds[i] == polId )
            position = i;
    }
          
    
    if ( position >= 0 )
    {
        //# If we already have something at that given position we need
        //# to append that table onto the list of tables.
        uInt last = itsTableVectors[position].nelements();
        itsTableVectors[position].resize(last+1, True);
        itsTableVectors[position][last] = table;
     
        itsTableVecNames[position].resize(last+1, True);
        itsTableVecNames[position][last] = tableName;
     
        itsTableVecSpwIds[position].resize(last+1, True);
        itsTableVecSpwIds[position][last] = spwId;
     
        itsTableVecPolIds[position].resize(last+1, True);
        itsTableVecPolIds[position][last] = polId;
    } else {
        //# If the table is to be inserted beyond the end of the table
        //# vector we need to make or storage containers bigger.
        position = itsTableVectors.nelements();
        
        itsTableVectors.resize( position+1, True );
        itsTableVectors[position].resize(1);
        itsTableVectors[position][0] = table;
     
        itsTableVecNames.resize(position+1, True );
        itsTableVecNames[position].resize(1);
        itsTableVecNames[position][0] = tableName;
     
        itsTableVecSpwIds.resize(position+1, True );
        itsTableVecSpwIds[position].resize(1);
        itsTableVecSpwIds[position][0] = spwId;
     
        itsTableVecPolIds.resize(position+1, True );
        itsTableVecPolIds[position].resize(1);
        itsTableVecPolIds[position][0] = polId;
    }
    
#if LOG2
    log->FnExit( fnname, clname);
#endif
    return rstat;
}
    
//#////////////////////////////////////////////////////////////////////////////
//# Set the Tables 
//# This works with itsSelectedMS, and itsSelectString and gets
//# root Table names from itsMS.
//# 
//# The following is the logic used to decide when to send a table into TablePlot.
//# This is for performance reasons. If a change is not required, do not
//# force a change. This allows TablePlot to re-use data.
//#
//#  This decision uses itsIsNewSelection, itsIsSubTablePlot, itsIsInIterPlot .
//#
//#  (1) For a Sub-Table plot, always send in the table and set itsIsSubTablePlot = True
//#  (2) If there's a new MS selection - send in when itsIsNewSelection = True
//#
//#  (3) If it's an old MS selection ( itsIsNewSelection = False )
//#        - but we've just moved out of a subtable plot -> when itsIsSubTablePlot = True
//#        - but we've just moved out of an iteration plot -> when itsIsInIterPlot = True 
//#  
//#  (4) If itsTablePlot->isTableSet() = False, then set to the current selection.
//#      (This can occur at the beginning, or after an iterPlotStop, 
//#       or after itsTablePlot.reset() )
//#
//#////////////////////////////////////////////////////////////////////////////
casa::Bool
MsPlot::setInputTable( const String& xcolumn, const String& ycolumn )
{
   String FnCall = String( "( " ) +  xcolumn 
           + String( ", " ) + ycolumn  + String( " )" );
   String fnname = "setInputTable";
#if LOG2
   log->FnEnter( fnname, clname);
#endif
   //cout << "setInputTable-------" << endl;
   Bool rstat = True;
   if (!checkInit() || itsMS==NULL) { 
#if LOG2
      log->FnExit( fnname, clname); 
#endif
      return rstat = False; 
   }
   //cout << "setInputTable========" << endl;


   //# (1) Check the number of rows in the table.
   //#     This does not change the state of itsTablePlot. 
   if( itsSelectedMS.nrow() == 0 ) {
       log->out(String( "No rows in selected Measurement Set " ), 
                fnname, clname, LogMessage::SEVERE);
#if LOG2
       log->FnExit( fnname, clname); 
#endif
       return rstat = False;
   }
    
   //# Some debugging details so see what the various flags are set at
   //# which controls what tables are plotted.
   String msg = String("itsIsSubTablePlot=") 
         + String::toString(itsIsSubTablePlot)
         + String(" itsIsNewSelection=") 
         + String::toString( itsIsNewSelection)
         + String(" itsIsInIterPlot=") 
         + String::toString(itsIsInIterPlot)
         + String(" itsConvertClassSet=") 
         + String::toString(itsConvertClassSet)
         +  String(" itsTableSet=") 
         + String::toString(itsTablePlot->isTableSet());
   log->out(msg, fnname, clname, LogMessage::DEBUG1);

   try {
      //# (2)  Create the Vector<Table> that TablePlot needs.
      //#      This will be a set of tables if the various spectral
      //#      windows have different shapes to them.
      if (!itsMsIsOpen) {
         //cout << "setInputTable------itsMsIsOpen" << endl;
         //# We are initializing the MS so we need not worry about
         //# conditions yet - I think?  This is needed to set up
         //# flagging i the TablePlot so the full MS is ok.
         //# We don't worry about spectral window sizes at this point.
         log->out(String( "Initializing table information for TablePlot" ),
                     fnname, clname, LogMessage::DEBUG1 );

         uInt spwId;
         if (itsSelectedSpwIds.nelements() >= 1)
            spwId = itsSelectedSpwIds[0];
         else 
            //# Assume there is only one spw and its the first one.
            spwId = 0;          
    
         uInt polId;
         if (itsSelectedPolIds.nelements() >= 1)
            polId = itsSelectedPolIds[0];
         else
            //# Assume there is only one spw and its the first one.
            polId = 0;          
    
         Vector<String> sortOrder(1);
         sortOrder[0] = "none";

         setSingleInputTable(sortOrder, spwId, polId, 
                             itsSelectedMS, itsMS->tableName());
           
         //# We need to set the table right away for TablePlot in order
         //# to handle the flagging, and flag versions.
         Vector<String> selectStr(itsTableVectors[0].nelements()); 
         for (uInt i=0; i < itsTableVectors[0].nelements(); i++)
              selectStr[i] = String("");
    
         itsTablePlot->setTableT(itsTableVectors[0], 
                                 itsTableVecNames[0], selectStr );
    
         //# We set this because regardless of what is done from now
         //# until things are plotted we want the table to be reset.
         //# The above is just a place holder.
         itsIsNewSelection = True;
       } 
       else if(!upcase(xcolumn).compare("ARRAY") 
               || !upcase(ycolumn).compare("ARRAY")) {

          //What is the logic here?

          //# Reset the table Information before we start
          //# setting all the information.
          resetTableSelection();
    
          //#  Set the TablePlot table to a SubTable " ANTENNA "
          //#  NOTE : This can be done for any other plot involving a
          //#         subtable. 
       
          //# Set the TablePlot Table to the ANTENNA subtable.
          //# This may get repeated if "array" is called in succession
          //# This is a small table, so this may be okay.
          log->out(String("Changing to subtable ANTENNA for array plot"),
                    fnname, clname, LogMessage::NORMAL4 );
    
          uInt spwId;
          if (itsSelectedSpwIds.nelements() >= 1)
             spwId = itsSelectedSpwIds[0];
          else 
          //# Assume there is only one spw and its the first one.
          spwId = 0;          
    
          uInt polId;
          if (itsSelectedPolIds.nelements() >= 1)
             polId = itsSelectedPolIds[0];
          else
          //# Assume there is only one spw and its the first one.
          polId = 0;          
    
          Vector<String> sortOrder(1);
          sortOrder[0] = "none";
          //#setSingleInputTable( sortOrder, spwId, polId, 
          //#   itsSelectedMS.antenna(), 
          //#   itsMS->tableName()+String( "/ANTENNA" ) );
          setSingleInputTable(sortOrder, spwId, polId, 
                               itsSelectedMS.antenna(), "" );
    
          //# TODO move this to where the TaQL is constructed?
          if (!itsConvertClassSet) {
             //# We didn't successfully setup the convert fucntion
             //# when we were setting the plot options.  report the
             //# error, well throw an exception.
             log->out( 
                 "Failed to setup data conversion function for ARRAY plot.",
                     fnname, clname, LogMessage::WARN, True);
          }
           
          //# We have a subtable plot so set our flag that lets us know!
          itsIsSubTablePlot = True;
       } 
       else if(itsIsSubTablePlot || itsIsNewSelection 
               || itsIsInIterPlot || itsConvertClassSet
               || !itsTablePlot->isTableSet()) {
          //# Reset the table Information before we start
          //# setting all the information.
          resetTableSelection();
    
          //# Check to see if we can use just a single set of
          //# tables or if we need lots of them. False is returned
          //# if they have different shapes.
          Bool spwsAllSameShape = checkSpwShapes();
          Bool polsAllSameShape = checkPolShapes();
	    //cout << "Pols same shape: " << polsAllSameShape << endl;
	  
          //# Output some debugging information
          SLog::slog()->out( String( "spwsAllSameShape=" ) 
                 + String::toString( spwsAllSameShape )
                 + String( " polsAllSameShaper=" ) 
                 + String::toString( polsAllSameShape )
		 + String( " datacolumn='" ) + itsDataColumn + String( "'" )
		 + String( " averagemode='" ) + itsAveMode + String( "'" )
		 + String( " averagetime=" ) + String::toString( itsAveTime )
                 + String( " averagechan=" ) + String::toString( itsAveChan )
                 + String( " chanAveMode=" ) + itsAverage[0] + String(  "," )
                 + itsAverage[1], fnname, clname, LogMessage::DEBUG1 );

	  //cout << "AVERAGE MODE: " << itsAveMode << endl;
	  //cout << "Ave Chan: " << itsAveChan << endl;
	  //cout << "Ave Time: " << itsAveTime << endl;
	  //cout << "Data Column: " << itsDataColumn << endl;
	  
         ROMSMainColumns msColumn(itsSelectedMS);
         Vector<Int> ant1 = msColumn.antenna1().getColumn();
         Vector<Int> ant2 = msColumn.antenna2().getColumn();
         uInt nrrec = ant1.nelements();
         Matrix<Int> bls(nrrec, 2);
         uInt nBase = 0;
         for (uInt i = 0; i < nrrec; i++) {
            Bool in = False;
            for (uInt j = 0; j < nBase; j++) {
               if (bls(j, 0) == ant1(i) && bls(j, 1) == ant2(i)) {
                  in = True;
                  break;
               }
            }
            if (!in) {      
               bls(nBase, 0) = ant1(i);
               bls(nBase, 1) = ant2(i);
               nBase++;
            }
         }
         bls.resize(nBase, 2, True);
         //cout << "bls=" << bls << endl;
         itsBaselines.resize(bls.shape()[0], bls.shape()[1]);
         itsBaselines = bls;


         if (itsAveMode.length() != 0 
             && (itsAveChan > 1 || itsAveTime > itsMinTimeInterval)){
             //cout << "-------ave------" << endl;

            log->out("average on time (and channel if requested)",
                     fnname, clname, LogMessage::DEBUG1);

            if (!msa) 
               msa = new MsAverager(&itsSelectedMS);
            else
               msa->reset(&itsSelectedMS);

            //above works before setdata/avedata splitting
            //for split setdata/avedata, the following crashes
            //  vis='data/ngc5921.ms'
            //  mp.open(vis)
            //  mp.setdata()
            //  mp.plot('visvelocity')
            //  mp.close() ----- this destruct MsPlot thus invalidate averager
            //  averagemode='vector'
            //  timebin='90'
            //  width='4'
            //  plotxy()

            //cout << "itsBaselines=" << itsBaselines << endl;

            //cout << "itsRestFreq=" << itsRestFreq
            //     << " itsFrame=" << itsFrame
            //     << " itsDoppler=" << itsDoppler << endl;

            Timer timer;
            msa->setAverager(chanList, itsBaselines, itsAveTime, itsAveChan, 
                             itsDataColumn, itsAveMode, itsAveFlag,
                             itsAveScan, itsAveBl, itsAveArray, doVel,
                             itsRestFreq, itsFrame, itsDoppler);
            if (msa->ok()) {
               //MSColumns msc(itsSelectedMS);
               //msa.showMsRow(&msc, 0);

               msa->getMS(itsAveragedMS);
#if LOG0
               uInt len = itsAveragedMS.nrow();
               MSColumns msc(itsAveragedMS);
               for (uInt i = 0; i < len; i++) {
                  msa->showMsRow(&msc, i);
               }
#endif
               msa->getMap(aveRowMap, aveChanMap);
#if LOG0
               msa->showAveMap(aveRowMap, aveChanMap);
#endif

                //timer.show("Averaging: ");
                log->out(String("Averaging: ") + 
                     String::toString(timer.all()) + " sec.",
                     fnname, clname, LogMessage::NORMAL5);
            }
            else {
               AipsError ae("Could not average the given MS" );
               throw( ae );
            }
           
            uInt spwId = 0;
            if (itsSelectedSpwIds.nelements() >= 1)
                spwId = itsSelectedSpwIds[0];
    
            uInt polId = 0;
            if (itsSelectedPolIds.nelements() >= 1)
               polId = itsSelectedPolIds[0];
    
            Vector<String> sortOrder(1);
            sortOrder[0] = "none";

            //setSingleInputTable(sortOrder, spwId, polId, 
            //       itsSelectedMS, 
            //       itsSelectedMS.tableName());

            //setSingleInputTable(sortOrder, spwId, polId, itsAveragedMS, "");
            //cout << "itsAveragedMs=" << itsAveragedMS << endl;
            //cout << "itsAveragedMs=" << &itsAveragedMS << endl;

            //do we need a new name
            //static Int tableId = 1;
            //String cpName = String("casapy.scratch-") 
                      //itsSelectedMS.tableName()
            //          + String::toString(tableId++) + String(".table");
            //itsAveragedMS.rename(cpName, Table::New);
            //setSingleInputTable(sortOrder, spwId, polId, itsAveragedMS, cpName);

            //In order to support multi panel flagging, make a deep
            //copy of te averaged ms 
            static Int tableId = 1;
            String cpName = String("casapy.scratch-") 
                      //itsSelectedMS.tableName()
                      + String::toString(tableId++) + String(".table");
            itsAveragedMS.deepCopy(cpName, Table::New, False); 
            MS cpMS(cpName);
            cpMS.markForDelete();
            setSingleInputTable(sortOrder, spwId, polId, cpMS, cpName);
            
#if LOG2 
            {
               ostringstream os;
               os << "averaged ms=" << itsSelectedMS << endl;
               log->out(os, fnname, clname, LogMessage::NORMAL5);
            }
#endif

            nAvePol = msa->npol();
            nAveChan = msa->nchan();

            //cout << "setInputTable------done average" << endl;
         }
         else {
            //cout << "-------------" << endl;
            //no average

            Block<int> sort;

            //for single table
            Vector<String> sortOrder;
            
            //cout << "itsIterationAxes=" << itsIterationAxes << endl;
            /*if (spwsAllSameShape && polsAllSameShape ) { 
               //# All selected spw and pol ids are of the same shape.
               //# so only need a single table
               log->out(String("Plotting from the Main Measrument Set"),
                       fnname, clname, LogMessage::NORMAL2);
                  
               sortOrder.resize(1);
               sortOrder[0] = "none";
               uInt spwId;
               if (itsSelectedSpwIds.nelements() >= 1)
                  spwId = itsSelectedSpwIds[0];
               else
                  //# Assume there is only one spw and its the first one.
                  spwId = 0;          
        
               uInt polId;
               if (itsSelectedPolIds.nelements() >= 1)
                  polId = itsSelectedPolIds[0];
               else
                  //# Assume there is only one spw and its the first one.
                  polId = 0;          

               setSingleInputTable(sortOrder, spwId, polId, 
                                 itsSelectedMS, itsMS->tableName() );
               itsIsSubTablePlot = False;
            } 
            else //if (!spwsAllSameShape || !polsAllSameShape) {*/
               //# We need to divide on the data description ID only,
               //# to all either variable shape SPWs or variable shape
               //# polarizations. 
               log->out(String("Sorting tables on Data Description Id"),
                        fnname, clname, LogMessage::NORMAL2 );
               sort.resize(2);
               sort[0] = MS::ARRAY_ID;
               sort[1] = MS::DATA_DESC_ID;
             

               //# Set what the major sorting axis is.
               //# TODO, if we are iterating on correlation, or some
               //# other field we'll want to change how this is done.
               //# TODO investigate if we can get the spw ID and pol ID
               //# from the table itself.  Or maybe we should be passing
               //# in the MSIter object to the method instead, since it
               //# holds all the info.
	       
               if (spwsAllSameShape && !polsAllSameShape) {
		   sortOrder.resize(1);
		   sortOrder[0] = "pol";
	       } else if ( !spwsAllSameShape && polsAllSameShape ) {
                   sortOrder.resize(1);
                   sortOrder[0] = "spw";
	       } else {
                   //# Sort on spectral windows first because they
		   //# are bigger.
		   sortOrder.resize( 2 );
		   sortOrder[0] = "spw";
		   sortOrder[1] = "pol";       
	       }
   
               //# Pick a huge time interval in seconds.
               //# Why so large you ask?  Well for some reason it needs
               //# to big or else we get lots and lots of tables and we
               //# want as few as possible.
               MSIter msIter( itsSelectedMS, sort, 7.0e9, False );
           
               //# Now create the table vector for TablePlot, note
               //# that the table name is the same for all of them.
           
               //# TODO see if there is a way to get the number of
               //# tables from the iterator and size things properly
               //# before starting?
               Int i=-1;
               Int lastSpwId = -1;
               Int lastPolId = -1;
               uInt totalTbls = 0;

               for (msIter.origin(); msIter.more(); msIter++, totalTbls++) {   
                  //# If we have a new SPW or polar index then we need to
                  //# create a new set of tables.
                  if (lastSpwId != msIter.spectralWindowId() 
                      || lastPolId != msIter.polarizationId()) {
                     lastSpwId = msIter.spectralWindowId();
                     lastPolId = msIter.polarizationId();
                     i++;
                  }
#if LOG2
                  String msgPrefix = String( "\nTable " ) + 
                                        String::toString(i);
                  log->out( msgPrefix + String( ": SPW " ) 
                        + String::toString( lastSpwId ) + msgPrefix 
                        + String( ": Polar " ) 
                        + String::toString( lastPolId ),
                        fnname, clname, LogMessage::DEBUG1 );
#endif
          
                  //# Skip if we don't have any correlation or channel
                  //# information for this table, ie. its empty. 
                  if (itsSelectedCorrIds[lastPolId].nelements() < 1) {   
                     continue;
                  }
                  if (itsSelectedSpwIds.nelements() && 
                      itsSpwIdNChans[lastSpwId] < 0 ) {
                     continue;
                  }

                  setSingleInputTable( sortOrder, lastSpwId, lastPolId, 
                       msIter.table(), itsMS->tableName() );
               }
                 
               //#timer.show("Done table vector creation.  ");
               String msg = String("Created ") 
                     + String::toString(totalTbls);
               log->out( msg, fnname, clname, LogMessage::NORMAL2 );
           
               //# We don't have subtables
               itsIsSubTablePlot = False;
              
               //# TODO verify how these are to be set for both cases above:
               //# one subtable or many subtables.
               itsIsInIterPlot = False;
               itsIsNewSelection = False;
               rstat = True;
	       /*}*/
            //else {
            //   //# TODO We aren't setting the table, does this
            //   //# mean we are using the whole MS or nothting
            //   //# has changed since last plot?
            //} 
         }
      }
   }
   catch (AipsError ae) {
      itsIsNewSelection = True; 
      //# This says that the Table has not been sent
      //# into TablePlot yet...
      
      String aemsg = ae.getMesg();
      //aemsg="Unable to set table for plotting. Invalid Table operation: "
      //      "Table: cannot add a column";
      if (aemsg.length() > 0 && aemsg.contains("cannot add a column")) {
         String msg = "Unable to set table for plotting. The most likely "
                      "reason is that a column in the MS has been "
                      "corrupted. That column has to be removed. " 
                      "Please try running clearcal.";
         log->out(msg, fnname, clname, LogMessage::WARN);
      }
      else {    
         String msg = String("Unable to set table for plotting. ") + aemsg;
         log->out( msg, fnname, clname, LogMessage::WARN);
      }
      
      rstat = False;
   }
#if LOG2
   log->FnExit( fnname, clname);
#endif
   return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//#  Check the iteration axes strings
//#////////////////////////////////////////////////////////////////////////////
//#casa::Vector<casa::String>
casa::Bool
MsPlot::checkIterationAxes( const Vector<String>& iteration )
{
    String fnname = "checkInterationAxes";
    log->FnEnter( fnname, clname);

    //# Init return val.
    Bool rstat=True;
    
    if ( ! checkInit() || ! checkOpenMS() )  { 
       log->FnExit( fnname, clname); 
       return rstat=False; 
    }//#return retValue; }
 
    //# (1) If 'iteration' is the default : [""], recognize this.
    //# TODO : If the xml/cmpt default can be Vector<String>(0), 
    //# that would be great.
    //#        Then this piece can go away.
    if( iteration.nelements() < 1 || 
         (iteration.nelements() == 1 && iteration[0].length() == 0 ) )
    {
       itsIterationAxes.resize(0);
       log->FnExit( fnname, clname); 
       return rstat;
    }
    
    //# (2) Get the iteration axes in terms of Table Column names.
    //#     Map the user-supplied iteration axes to this.
    //# TODO : Provide translations for iterations on all index-able
    //#        parameters that msselection uses to select on.
    Int ncolumns = 0;
    for(Int i=0; i<(Int)iteration.nelements(); i++)
    {
       if ( upcase(iteration[i]).matches("BASELINE") )
       {
           itsIterationAxes.resize( ncolumns + 2, True );
           itsIterationAxes[ ncolumns ] = String("ANTENNA1");
           itsIterationAxes[ ncolumns + 1 ] = String("ANTENNA2");
           ncolumns = itsIterationAxes.nelements();
       }
       else if ( upcase(iteration[i]).matches("ANTENNA")
          || upcase(iteration[i]).matches("ANTENNA1") )
       {
           if ( upcase(iteration[i]).matches("ANTENNA1") )
          log->out(
              "Iterating on ANTENNA1 instead of ANTENNA",
              fnname, clname, LogMessage::NORMAL4 );
           itsIterationAxes.resize( ncolumns + 1, True );
           itsIterationAxes[ ncolumns ] = String("ANTENNA1");
           ncolumns = itsIterationAxes.nelements();
       }
       else    if ( upcase(iteration[i]).matches("FIELD") 
          || upcase(iteration[i]).matches("FIELD_ID") )
       {
           itsIterationAxes.resize( ncolumns + 1, True );
           itsIterationAxes[ ncolumns ] = String("FIELD_ID");
           ncolumns = itsIterationAxes.nelements();
       }
       else if ( upcase(iteration[i]).matches("SPW") 
          || upcase(iteration[i]).matches("SPECTRAL_WINDOW")  )
       {
           itsSpwIterOn = True;
           //# TODO Make this do the SPWid properly....
           //# At least in the label... :-|
           itsIterationAxes.resize( ncolumns + 1, True );
    
           //# CHANGED BY SDJ  Don't we need the spectral window here
           //# and not the data desc id, will need to do by hand at 
           //# some point
           itsIterationAxes[ ncolumns ] = String("DATA_DESC_ID");
           itsIterationAxes[ ncolumns ] = String("SPECTRAL_WINDOW");
           ncolumns = itsIterationAxes.nelements();
       }
       else if ( upcase(iteration[i]).matches("SCAN") )
       {
           itsIterationAxes.resize( ncolumns + 1, True );
           itsIterationAxes[ ncolumns ] = String("SCAN_NUMBER");
           ncolumns = itsIterationAxes.nelements();
       }   
       else if ( upcase(iteration[i]).matches("FEED") 
          || upcase(iteration[i]).matches("FEED1") )
       {   
           if ( upcase(iteration[i]).matches("FEED") )
          log->out( "Iterating on FEED1 instead of FEED",
             fnname, clname, LogMessage::NORMAL4 );
           itsIterationAxes.resize( ncolumns + 1, True );
           itsIterationAxes[ ncolumns ] = String("FEED1");
           ncolumns = itsIterationAxes.nelements();
       }
       else if ( upcase(iteration[i]).matches("ARRAY_ID") 
          || upcase(iteration[i]).matches("ARRAY") )
       {
           itsIterationAxes.resize( ncolumns + 1, True );
           itsIterationAxes[ ncolumns ] = String("ARRAY_ID");
           ncolumns = itsIterationAxes.nelements();
       }
       else //# default.
       {
           ostringstream os;
           os << "Unrecognized iteration value: " << iteration
              << " - continuing anyway ...";
           log->out(os, fnname, clname, LogMessage::WARN);
 
           itsIterationAxes.resize( ncolumns + 1, True );
           itsIterationAxes[ ncolumns ] = upcase(iteration[i]);
           ncolumns = itsIterationAxes.nelements();
    
           //# This may be an error state at some point, for now
           //# we let TablePlot worry about the validity of it.
           //#rstat = False;
       }
    }
      
    log->FnExit( fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//# Check if all selected spectral windows have the same shape, 
//# ie. that the spectral windows all have the same number of
//# channels.
//#////////////////////////////////////////////////////////////////////////////
casa::Bool
MsPlot::checkSpwShapes()
{
    String fnname = "checkSpwShapes";
    log->FnEnter( fnname, clname);

    Bool rstat = True;

    //# Throw an exception if the MsPlot class has not bee initialized
    //# or if there is no MS associated with the class.
    if ( ! checkInit() || ! checkOpenMS() )  
    { 
       //log->out( "Unable to contine, no Measurment Set open.",
       //    fnname, clname, LogMessage::WARN, True);
       return rstat = False; 
    }

    
    //# Need to check only if the MS has more than one Spectral Window..
    if( itsMaxSpwId > 1 )
    {
       //#cout << "Max number of SPWS: " << itsMaxSpwId << endl;
       //#cout << "Number of SPWs: " << itsSelectedSpwIds.nelements() <<endl;
       if ( itsSelectedSpwIds.nelements() < 1 )
       {
           //# We need to set the Start Chan, otherwise things will
           //# fail when we are trying to plot the tables.
           itsSelectedSpwIds.resize( itsMaxSpwId );
           itsStartChan.resize( itsMaxSpwId );
           for (uInt i=0; i<itsMaxSpwId; i++ )
           {
          itsSelectedSpwIds[i] = i;
          itsStartChan[i] = 0;
           }
       }
       //#cout << "SPWs: " << itsSelectedSpwIds <<endl;
       //#cout << "itsSpwIdNChans: " << itsSpwIdNChans << endl;
       
       //# Check if the shapes are the same
       Int shape = itsSpwIdNChans[itsSelectedSpwIds[0] ];
       for(uInt i=1; i < itsSelectedSpwIds.nelements(); i++)
       {
           //# Skip this SPW if this ID is not part of the 
           //# measurment set.
           if ( !itsUsedSpws[itsSelectedSpwIds[i]] )
              continue;
           
           if( shape !=  itsSpwIdNChans[ itsSelectedSpwIds[i] ] )
           {
              //# *itsLog << LogIO::SEVERE
              //#   << "Error: Different Spectral Windows have different shapes."
              //#   << "       Please do a selection on spectral windows."
              //#   << LogIO::POST;
              rstat = False;
          break;
           }
       }
    }



    //# This is a somewhat expensive brute-force check for shapes.
    //# It iterates through the entire MS, pulls out the shape of
    //# the "DATA" column for each row and checks for consistency
    //# across rows.
    //#if ( rstat && !itsTablePlot->checkShapes( intable ) ) complain.©©©

    
    //#cout << "checkSpwShapes is returning: " << rstat << endl;
    log->FnExit( fnname, clname);
    return rstat;
}


//#////////////////////////////////////////////////////////////////////////////
//# Check if all selected polarizations (correlations) have the same shape, 
//# ie. that each row in the polarization table is the same size.
//#////////////////////////////////////////////////////////////////////////////
casa::Bool
MsPlot::checkPolShapes()
{
    String fnname = "checkPolShapes";
    log->FnEnter( fnname, clname);
    Bool rstat = True;

    //# Throw an exception if the MsPlot class has not bee initialized
    //# or if there is no MS associated with the class.
    if ( ! checkInit() || ! checkOpenMS() )  
    { 
	log->FnExit( fnname, clname );
	return False;
    }

    //# If nothing was selected we assume that the user entered
    //# a blank value which by default selects everything. This is
    //# safe to assume since the correlation selection information
    //# would have been parsed already and we wouldn't be here if
    //# errors occured
    if ( itsSelectedPolIds.nelements() < 1 )
    {
	itsSelectedPolIds.resize( itsMaxPolId );
	itsSelectedCorrIds.resize( itsMaxPolId );
	for (uInt i=0; i<itsMaxPolId; i++ )
	{
	    itsSelectedPolIds[i] = i;
	    itsSelectedCorrIds[i] = itsCorrelationIds[i];
	}
    }
    

    //# If there is only 1 row in the polarization table, then this is
    //# a no brainer -- ie. they are all the same.  We do this check here
    //# because we need to fill itsSelectedPolIds if it was empty, before
    //# returning.
    if( itsMaxPolId < 2 )
    {
	log->FnExit( fnname, clname);
	return rstat;
    }

    //# Okay now we can check what the user has selected and the number
    //# of correlation values in each row of the polarization table.
    //# This is only an error if there is no data in the end

    Int shape = itsSelectedCorrIds[itsSelectedPolIds[0]].nelements();
    for( uInt i=1; rstat && i < itsSelectedPolIds.nelements(); i++)
    {
	Int size = itsSelectedCorrIds[itsSelectedPolIds[i]].nelements();
	if( shape != size )
	{
	    rstat = False;
	    break;
	} else {
	    //# Check that if they have the same length that they
	    //# also have the same selections.
	    for( Int corrId=0; corrId < size; corrId++ )
		if ( itsSelectedCorrIds[itsSelectedPolIds[0]][corrId]
			!= itsSelectedCorrIds[itsSelectedPolIds[i]][corrId] )
		{
		    rstat = False;
		    break;
		}
	}
    }
    
    log->FnExit( fnname, clname);
    return rstat;
}
    

//#////////////////////////////////////////////////////////////////////////////
//# Clear the Plotter
casa::Bool
MsPlot::clearPlot( const casa::Int nrows, 
   const casa::Int ncols, 
   const casa::Int panel )
{
    String FnCall = String( "( " ) + String::toString( nrows )
   + String( ", " ) + String::toString( ncols )
   + String( ", " ) + String::toString( panel ) + String( "  )" );
    String fnname = "clearPlot";
    log->FnEnter( fnname + FnCall, clname );

    Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { log->FnExit( fnname, clname); return rstat; }

    itsTablePlot->clearPlot( nrows, ncols, panel );
    
    log->FnExit( fnname, clname);
    return rstat;
}
    
//#////////////////////////////////////////////////////////////////////////////
//# 
//#         PLOTTING ROUTINES
//#
//#////////////////////////////////////////////////////////////////////////////

//#////////////////////////////////////////////////////////////////////////////
//# Plot the specified portion of the measurement set
casa::Bool
MsPlot::setupPlotxy( const casa::String& x,
   const casa::String& y,
   const casa::String& xcolumn,
   const casa::String& ycolumn,
   const casa::String& xvalue,
   const casa::String& yvalue,
   casa::Vector<casa::String>& iteration )
{
   String FnCall = "( x, y, xcolumn, ycolumn, xvalue, yvalue, iteration )";
   String fnname = "setupPlotxy";
   //cout << "x=" << x << " y=" << y << " xcolum=" << xcolumn
   //     << " ycolumn=" << ycolumn 
   //     << " xvalue=" << xvalue << " yvalue=" << yvalue << endl;
   log->FnEnter( fnname + FnCall, clname );
    
   Bool rstat = True;
   if ( ! checkInit() || ! checkOpenMS() )  { 
      log->FnExit( fnname, clname); 
      return rstat; 
   }

   //cout << "setupPlotxy - " 
   //     << "itsAveChan=" << itsAveChan << " itsAvetime=" << itsAveTime
   //     << " itsAveFlag=" << itsAveFlag << " itsAveMode=" << itsAveMode
   //     << endl;

   try {
      //# Check if we need to get out of iterplot mode
      if (itsIterPlotOn && !itsPlotOptions.OverPlot ) 
         iterPlotStop(False);

      //# (1) Re-fill plotoptions instance with the user-set record.
      //# - Yes, this is done in MsPlot::setPlotOptions too.
      //#   It's needed in ::setplotoptions too, because of the ability 
      //#   to validate params before plotting. (user-interaction)
      //# - It's needed here too, to reset any plot-type-specific plot options
      //#   that were set in the previous plot in MsPlot::updatePlotOptions.
      if ( rstat ) {
         String errors = itsPlotOptions.fillFromRecord( itsOptionRecord );
         if ( errors.length() > 0 ) {
            log->out( errors, fnname, clname, LogMessage::SEVERE );
            rstat = False;
         }
      }

      //# (2) Create/check the iteration axes specifications.
      //#     This is done second so that we can set the itsInIterPlot
      //#     flag properly for calling setInputTables.
      if ( rstat )
         rstat = checkIterationAxes( iteration );
      
      // This is a big logic mistake!
      // 1. The conversion function depends on the table to be plotted
      // 2. The table splitting and ordering is in the setInputTable
      // 3. setInputTable needs setupConvertFn
      // 4. when setConvertFn before setInputTable, the setConvertFn
      //    does not have full information needed for conversion
      // Therefore, any conversion done can be wrong.
      // Is it a repare to repeat setupConvertFn after setInputTable? 
      //
      //# (2b) Setup the convert Functions.  This is needed here because
      //# setInputTables needs to know if there have been any new 
      //# convert functions set.
      setupConvertFn( x, 'x' );
      setupConvertFn( y, 'y' );     
   
      //# (3) Setup the Table(s) for Table Plot
      //#     This will put the list of tables to use in itsTableVec,
      //#     and the name of the MS assoc. with each table in itsTableName
      //#     These need to be used later to setup the tables in Table Plot.
      //#     We need to create the tables before the TaQL is created, 
      //#     since some of the TaQL strings depend on SPW IDs
      if ( rstat && ( ! setInputTable( xcolumn, ycolumn ) ) ) {
         log->out( "Unable to set the table used by the plotter", 
                  fnname, clname, LogMessage::SEVERE );
         resetTableSelection();
         rstat = False;
      }

      // see 2b for comments
      if (rstat) {
        //only needed by non-averaged
        if (itsAveMode.length() < 1 || 
            (itsAveChan <= 1 && itsAveTime <= itsMinTimeInterval)) {
           setupConvertFn( x, 'x' );
           setupConvertFn( y, 'y' );     
        }
      }

      //# (4) Create the taql strings.
      //# NOTE: Need to store the taql for plotxy?
      itsTaqlStrings.resize(0);
      String xlabel(""), ylabel(""),title("");
      if ( rstat ) {
         //cout << "itsAveMode=" << itsAveMode << endl;
         if (itsAveMode.length() > 0 && 
             (itsAveChan > 1 || itsAveTime > itsMinTimeInterval)) { 
            //SaQL is for average
            rstat = createSaQL(x, y, xvalue, yvalue, title, xlabel, ylabel);
         }
         else {
            rstat = createTaQL(x, y, xcolumn, ycolumn, 
                         xvalue, yvalue, title, xlabel, ylabel);
         }
	 
         if (!rstat) {
            log->out("Could not create TaQL strings.", 
                      fnname, clname, LogMessage::SEVERE);
         }
         else {
            ostringstream os;
            for (uInt i=0; i<itsTaqlStrings.nelements(); i++) {
               os << "The first of " << itsTaqlStrings[i].nelements()
                  << " TaQL for Table "
                  << i << ": \n";
               //for(uInt j=0; j<itsTaqlStrings[i].nelements(); j++)
               for (uInt j=0; j<1; j++)
                  for (uInt k=0; k+1<itsTaqlStrings[i][j].nelements(); k+=2)
                     os << "[ " << itsTaqlStrings[i][j][k] 
                        << " , " << itsTaqlStrings[i][j][k+1] << " ]\n";
            }
            //log->out(os, fnname, clname, LogMessage::DEBUG1);
	    log->out(os, fnname, clname, LogMessage::NORMAL3);
         }
      }

      //# (5) Modify the PlotOptions according to the specific type of plot.
      //#     Labels and PointLabels and TimePlotChar.....
      //#     This needs to be called before the Tables are created so
      //#     that itsConvertClassSet is set properly.
      if ( rstat ) {
         rstat = updatePlotOptions( title, xlabel, ylabel, 
                                   xcolumn, ycolumn, x, y );
         log->out( itsPlotOptions.print(), 
                  fnname, clname, LogMessage::DEBUG1 );
      }
   } 
   catch ( AipsError ae ) {
      if( iteration.nelements()==0 )   
            cleanupOptions();
      rstat = False;
      throw;
   }

   log->FnExit( fnname, clname);
   return rstat;
}
    

//#////////////////////////////////////////////////////////////////////////////
//# Plot the specified portion of the measurement set
//#
//# Valid x and y values are:
//#            ARRAY, AZIMUTH, BASELINE, CHAN, CORR, DATA, ELEVATION, 
//#              HOURANGLE, PARALLACTICANGLE, TIME, U, UVCOVERAGE, UVDIST,
//#              V, W
//#           any other values are assumed to be TaQL strings and
//#              are allowed to flow through to TablePlot, where they
//#              are examined for accuracy.  Note that it is unlikely
//#              you would specify the column and value parameters if
//#              you give a TaQL string.
//# 
//# Valid column values are:
//#         CORRECTED, MODEL, RESIDUAL, WEIGHTEDDATA, WEIGHTEDCORRECTED,
//#         WEIGHTEDMODEL, WEIGHTEDRESIDUAL (some of these not fully
//#         implemented yet).  Also doesn't error if it is something else
//#         it falls through hoping that it is a recognizable data column.
//#
//# Valid "value" values are:
//#         AMP, AMPLITUDE, PHA, PHASE, RE, REAL, IM, IMAG, and IMAGINARY
//#
//# Valid iteration values -- see checkInterationAxes
//#         BASELINE, ANTENNA, ANTEANNA1, ANTENNA2, ARRAY, ARRAY_ID,
//#         CHAN, CHANNEL, CORR, CORRELATION, DATA_DESC_ID, FEED, 
//#      FEED1, FIELD, FIELD_ID, SCAN, SCAN_NUMBER, SPW, SPECTRAL_WINDOW
//#         also anything that is a column in the Main MS table will work.

casa::Bool
MsPlot::plotxy( const casa::Bool checkOnly,
        const casa::String& x,
   const casa::String& y,
   const casa::String& xcolumn,
   const casa::String& ycolumn,
   const casa::String& xvalue,
   const casa::String& yvalue,
        casa::Vector<casa::String>& iteration )
{
    String FnCall = "(x, y, xcolumn, ycolumn, xvalue, yvalue, iteration, filename)";
    String fnname = "plotxy";
    log->FnEnter( fnname + FnCall, clname );
    //cout << "x=" << x << " y=" << y
    //     << " xcolumn=" << xcolumn << " ycolumn=" << ycolumn
    //     << " xvalue=" << xvalue << " yvalue=" << yvalue
    //     << endl;
    Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { 
        log->FnExit( fnname, clname); 
        return rstat; 
    }
    
    String msg = "Preparing data ...      ";
    log->out( msg, fnname, clname, LogMessage::NORMAL);
    
    try 
        {
       //# Call setupPlotxy.  This method sets any plot options that are
       //# set internally, finds the taql string, and does some sanity
       //# checks on the plot.
       rstat = setupPlotxy( x, y, xcolumn, ycolumn, xvalue, yvalue, iteration );
       if ( !rstat )  { 
           log->FnExit( fnname, clname); 
           return rstat; 
       }

       //# Plot the data if all is okay.
       //# TODO decide if we should not plot if their were warnings.
       //# (??) Setup the tables used by Table Plot for plotting.
        log->out(String("Number of tables SETS to plot: ") +
                 String::toString(itsTableVectors.nelements()),
                 fnname, clname, LogMessage::DEBUG1); 
       
       //# For each table in our list of tables do
       Bool haveMadeFirstPlot = False;
       Bool numTblsToPlot = 0;
       Vector<String> selectionStr;
       for ( uInt tblVecCnt = 0; 
                  rstat && tblVecCnt < itsTableVectors.nelements(); 
             tblVecCnt++ )
       {
           //# Skip if we don't have any correlation or channel information 
           //# for this table, ie. its empty. 
           //# TODO add a check to make sure we plot something in the end.
           Int polId = itsTableVecPolIds[tblVecCnt][0];
           if (  itsSelectedCorrIds.nelements() > 0
              && itsSelectedCorrIds[polId].nelements() < 1 )
           {
                continue;
           }
           
           Int spwId = itsTableVecSpwIds[tblVecCnt][0];
           if ( itsSelectedSpwIds.nelements() >  0 
                       && ( itsSelectedSpwIds.nelements() > 0 
                       && allLT(itsStartChan[spwId], 0) ) )
           {
	       continue;
           }
           numTblsToPlot++;
           String msg = String( "Plotting a total of " )
                 + String::toString( numTblsToPlot ) + String( " tables." );
           log->out( msg, fnname, clname, LogMessage::NORMAL4 );
                  
           //# Note that if we turn it on before we plot the first
           //# set of table we end up overplotting on what was
           //# already plotted.  Not what we want to do!
           //# TODO we need to base this on when we've actully
           //#      plotted something
           if( haveMadeFirstPlot ) {
               itsPlotOptions.OverPlot = True;
           }
           haveMadeFirstPlot = True;
           
    
           //# Find all the tables with the same SPW that are together.
           //# Note that we could send the tables in one at a time,
           //# making the code easier, but this makes it *much* slower.
           //# The downside is that sending multiple tables in, causes
           //# the TablePlot to automatically go into multi-color mode
           //# plotting each table in a different colour. The other
           //# downside is more memory use.
           //# Verify this?  I don't think this is true now.
           selectionStr.resize( itsTableVectors[tblVecCnt].nelements() );
    
           if ( numTblsToPlot < 1 )
           {
              String msg = "There are no tables to plot. Plese check"
                 " your data selection\nvalues to ensure that"
                 " it selects a portion of the MS.\nIn particular check"
                 " the channel and correlation selection.";
              log->out( msg, fnname, clname, LogMessage::SEVERE );
              rstat = False;
           }
           
           if( rstat && itsTableVectors[tblVecCnt].nelements() 
                    && itsTableVecNames[tblVecCnt].nelements() 
                          && selectionStr.nelements() )
           {
               //# We should check for a return value of -1
               itsTablePlot->setTableT( itsTableVectors[tblVecCnt],
                           itsTableVecNames[tblVecCnt], selectionStr );
           } else {
              //# ERROR
           }

           if ( rstat && ! itsTablePlot->isTableSet() )
           {
              //# Houston, we have a problem!
              //# TablePlot thinks that it doesn't have a table the
               String msg = String( "Unable to set plotting tables, sorry" )
                  + String( " but can not continue!" );
               log->out( msg, fnname, clname, LogMessage::SEVERE );
               rstat = False;
           }
	   
	   //# MAJOR HACK ALERT -- in order to get the multiple spw
	   //# plot to work we need to set up the Convert Fn differently
	   //# for each table, we need to make sure the correct spwId is
	   //# in the the first index of the TableList.
	   if ( rstat &&  itsTableVectors.nelements() > 1 ) {
	       if ( !upcase(x).compare( "CHANFREQ" ) 
                    || !upcase(x).compare( "CHANNEL_FREQ" )
                    || !upcase(x).compare( "CHANNEL_FREQUENCY" )
                    || !upcase(x).compare( "CHAN_FREQ" ) 
                    || !upcase(x).compare( "CHANFREQ" ) 
                    || !upcase(x).compare( "CHANNEL_FREQ" )
                    || !upcase(x).compare( "CHANNEL_FREQUENCY" )
                    || !upcase(x).compare( "CHAN_FREQ" ) ) 
	       {
		   if (itsAveMode.length() < 1 || 
                        (itsAveChan <= 1 && itsAveTime <= itsMinTimeInterval))
		   {
			   setupConvertFn( x, 'x' );
			   setupConvertFn( y, 'y' );

			   Vector<int> spws( itsTableVectors.nelements() );
			   for( uInt z=0; z<itsTableVectors.nelements();z ++ )
			       spws[z]=spwId;
			   if ( itsPlotOptions.Convert != NULL )
			   {
			       MSPlotConvertChanToFreq *conv = 
				   (MSPlotConvertChanToFreq*)itsPlotOptions.Convert;
			       conv->setSpwIds( spws );
			   } else {
			       ostringstream os;
			       os << "Internal Error: Unexected NULL for conversion function pointer" 
				  << endl;
			       log->out( os, fnname, clname, LogMessage::SEVERE );
			   }
		   }
	       }

	       if ( !upcase(x).compare( "CHANVELOCITY" ) 
                    || !upcase(x).compare( "CHANNEL_VELOCITY" )
                    || !upcase(x).compare( "CHAN_VELOCITY" ) 
                    || !upcase(x).compare( "CHANVELOCITY" ) )
	       {
		   if ( itsAveMode.length() < 1 || 
                       (itsAveChan <= 1 && itsAveTime <= itsMinTimeInterval ) )
		   {
			   setupConvertFn( x, 'x' );
			   setupConvertFn( y, 'y' );

			   Vector<int> spws( itsTableVectors.nelements() );
			   for( uInt z=0; z<itsTableVectors.nelements();z ++ )
			       spws[z]=spwId;
			   if ( itsPlotOptions.Convert != NULL ) {
			       
			       MSPlotConvertChanToVelocity *conv = 
				   (MSPlotConvertChanToVelocity*)itsPlotOptions.Convert;
			       conv->setSpwIds( spws );
			   } else {
			       ostringstream os;
			       os << "Internal Error: Unexected NULL for conversion function pointer" 
				  << endl;
			       log->out( os, fnname, clname, LogMessage::SEVERE );
			   }
		   }
	       }
	   }
	   
           if ( rstat) {
              //# Do a sanity check on the stored TaQL for plotting.
              //# The check is slightly different depening on the
              //# shape of itsVectorTables, whether we are sending
              //# everything to TablePlot once or grouped be SPWs
#if LOG2 
              {
                 ostringstream os;
                 os << "Checking TaQL: " << itsTaqlStrings[tblVecCnt][0] 
                    << endl;
                 log->out(os, fnname, clname, LogMessage::DEBUG1);
              }
#endif
	      
              Vector<String> errors = 
                  itsTablePlot->checkInputs( itsPlotOptions, 
                     itsTaqlStrings[tblVecCnt], itsIterationAxes );
              if ( errors[0].length() > 0 ) 
              {
                  log->out( errors[0], fnname, clname, LogMessage::SEVERE );
                  rstat =  False;
              }
           
              if ( errors[1].length() > 0 ) {
                  log->out( errors[1], fnname, clname, LogMessage::NORMAL2 );
              }
           }

           if ( rstat && !checkOnly )
           {
              //# All right we are ok to plot the data
              //# Either an iterative (multi) plot or a standard one
              //# shot plot.
              if( rstat && itsIterationAxes.nelements()==0)
              {
                  itsTablePlot->plotData( itsPlotOptions, 
                               itsTaqlStrings[tblVecCnt] );
              } 
              else if ( rstat ) {
                  //cout << "Calling iterMultPlotStart with taql "
                  //    << itsTaqlStrings[tblVecCnt][0]
                  //    << endl;
                  //cout << "tblVecCnt=" << tblVecCnt << endl; 
                  //itsPlotOptions.print();
                  //cout << "itsIterationAxes=" << itsIterationAxes << endl;
                  itsTablePlot->iterMultiPlotStart( itsPlotOptions,
                              itsTaqlStrings[tblVecCnt], itsIterationAxes );
        
                  //# Set the iterplot ON flag.
                  itsIterPlotOn = casa::True;
               
                  //# Do this so got that at the end of iterplot, 
                  //# the TP.setTableT will be
                  //# forced. This needs to be done, because after 
                  //# iteration plots, the
                  //# subtable for the last iteration is all that TP has.
                  //# And the full Table needs to be sent back in.
                  itsIsInIterPlot = True;
                  
                  //# Call MsPlot::iterPlotNext() once, to start the plots.
                  //# Note that this will dis-allow overplots with iterplot.
                  //# TODO Decide which is more important. Have the first plot
                  //#  appear automatically, or to allow overplot with iterplot.
                  //# iterPlotNext();
                  //#cout << "Please call mp.iterplotnext() to make the plot"
                  //#     << " or you may perform an overplot first" << endl;
                  itsTablePlot->changeGuiButtonState( "iternext","enabled" );
              }
           }
       }
   } catch ( AipsError ae ) {
      if( iteration.nelements()==0 )   
         cleanupOptions();
      rstat = False;
      throw;
   }

   if ( !rstat ) 
   {
      //# TODO?? reset various flags because there is a serious
      //# error and we need to redo things??
   }
    

   if(iteration.nelements() == 0)   
       cleanupOptions();

   log->FnExit( fnname, clname);
   return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//# Reset any of the plot options that may have been set internally
void
MsPlot::cleanupOptions()
{
    itsPlotOptions.TimePlotChar = String( "o" );
    itsPlotOptions.ColumnsXaxis = True;
    itsPlotOptions.PointLabels.resize(0);
    
    if ( itsPlotOptions.Convert != NULL )
    {
       delete itsPlotOptions.Convert;
       itsPlotOptions.Convert = NULL;
       itsConvertClassSet = False;
    }    
    if( itsPlotOptions.CallBackHooks != NULL )
    {
        delete itsPlotOptions.CallBackHooks;
        itsPlotOptions.CallBackHooks = NULL;
    }

}
    

//#////////////////////////////////////////////////////////////////////////////
//# Modify TaQL string for time (chan) average plotting
casa::Bool
MsPlot::createSaQL(  const String& x, const String& y,
                     const String& xvalue, const String& yvalue,
                     String& title, String& xlabel, String& ylabel)
{
   // Valid x and y values are:
   //   TIME, DATA,
   //   ARRAY, AZIMUTH, BASELINE, CHAN, CORR, ELEVATION, FREQUENCY 
   //   HOURANGLE, PARALLACTICANGLE, U, V, W, UVCOVERAGE, UVDIST,
   //
   // Valid "value" values are:
   //         AMP, AMPLITUDE, PHA, PHASE, RE, REAL, IM, IMAG, and IMAGINARY
   String FnCall = String("(x=") + x + ", y=" + y +
                   ", xvalue=" + xvalue + ", yvalue=" + yvalue +
                   ", title=" + title + ", xlabel=" + xlabel +
                   ", ylabel=" + ylabel + ")" ;
   //cout << FnCall << endl;
   String fnname = "createSaQL";
   log->FnEnter( fnname +  FnCall, clname );

   Vector<String> axis(2);
   axis(0) = x;
   axis(1) = y;
   Vector<String> value(2);
   value(0) = xvalue;
   value(1) = yvalue;
   Vector<String> label(2);
   label(0) = "";
   label(1) = "";

   itsIsUVWPlot = False;

   //cout << "itsMaxPolId=" << itsMaxPolId << endl;
   //for (uInt i = 0; i < itsMaxPolId; i++) {
   //   cout << "start=" << itsStartCorr(i) << endl;
   //   cout << "end=" << itsEndCorr(i) << endl;
   //   cout << "stride=" << itsStepCorr(i) << endl;
   //}

   Int sect = itsStartCorr(0).nelements();
   itsTaqlStrings.resize(1);
   itsTaqlStrings[0].resize(1);

   Vector<String> curTaql(2);
   curTaql(0) = "";
   curTaql(1) = "";
   Vector<Vector<String> > dTaql(2);
   dTaql(0).resize(0);
   dTaql(1).resize(0);

   for (Int i = 0; i < 2; i++) {
      if (!upcase(axis(i)).compare("TIME")) {
         curTaql[i] = "TIME/86400.0 + 678576.0";
         label[i] = "Time";
      }
      else if (!upcase(axis(i)).compare("DATA")) {
         dTaql(i).resize(sect);
         for (Int s = 0; s < sect; s++) { 
         String data = String("(DATA[") + 
                         String::toString(itsStartCorr(0)(s)) +
                         String(":") +
                         String::toString(itsEndCorr(0)(s)) +
                         String(":") +
                         String::toString(itsStepCorr(0)(s)) +
                         String(",1:") +
                         String::toString(nAveChan) +
                         String(":1])");

         if (!upcase(value(i)).compare("RE") ||
             !upcase(value(i)).compare("REAL")) {
            dTaql[i][s] = String("REAL") + data;
            label[i] = "Real Part Average";
         }
         else if (!upcase(value(i)).compare("AMP") ||
                  !upcase(value(i)).compare("AMPLITUDE")) {
            if (!upcase(itsAveMode).compare("SCALAR")) {
               dTaql[i][s] = String("REAL") + data;
               label[i] = "Scalar Average Amplitude";
            }
            else {
               dTaql[i][s] = String("AMPLITUDE") + data;
               label[i] = "Vector Average Amplitude";
            }
         }
         else if (!upcase(value(i)).compare("IM") ||
                  !upcase(value(i)).compare("IMAG") ||
                  !upcase(value(i)).compare("IMAGINARY")) {
            dTaql[i][s] = String("IMAG") + data;
            label[i] = "Image Part Average";
         }
         else if (upcase(value(i)).matches("PHA") ||  
                  upcase(value(i)).matches("PHASE")) {
            dTaql[i][s] = String("(180/PI())*PHASE") + data; 
            label[i] = "Phase Average";
         }
         }
      } 
      else if (!upcase(axis(i)).compare("DEFAULT_WEIGHT") ||
               !upcase(axis(i)).compare("DEFAULTWEIGHT") ||
               !upcase(axis(i)).compare("WEIGHT") ) { 
          curTaql[i] = "WEIGHT[1]";
          label[i] = "Weights";
      }
      else //if (!upcase(column).compare("ARRAY")) {
         if (upcase(axis(i)).matches("XEAST")) {
            curTaql[i] = String("POSITION[1]");
            label[i] = String("X (m)");
         } 
         else if (upcase(axis(i)).matches("YNORTH")) {
            curTaql[i] = String("POSITION[2]");
            label[i] = String("Y (m)");
      } 
      else if (!upcase(axis(i)).compare("AZIMUTH") || 
            !upcase(axis(i)).compare("ELEVATION") || 
            !upcase(axis(i)).compare("HOURANGLE") || 
            !upcase(axis(i)).compare("HOUR_ANGLE") || 
            !upcase(axis(i)).compare("PARALLACTICANGLE") || 
            !upcase(axis(i)).compare("PARALLACTIC_ANGLE")) {
         curTaql[i] = "TIME";
         if (!upcase(axis(i)).compare("AZIMUTH"))
            label[i] = String("Azimuth (deg)");
         if (!upcase(axis(i)).compare("ELEVATION"))
            label[i] = String("Elevation (deg)");
         if (!upcase(axis(i)).compare("HOURANGLE"))
            label[i] = String("Hour Angle (deg)");
         if (!upcase(axis(i)).compare("PARALLACTICANGLE"))
            label[i] = String("Parallactic Angle (deg)");
      } 
      else if (!upcase(axis(i)).compare("BASELINE")) {
         Int nAnt = 2;
         //if (msa) nAnt = msa->nAnt();
         ROMSColumns msColumn( itsSelectedMS );
         const ROMSAntennaColumns & antColumns = msColumn.antenna();
         nAnt = antColumns.nrow();
         //looks like that a1<=a2 always, so
         //bl = nAnt * a1 - (a1 * (a1 - 1)) / 2 + a2 - a1;
         curTaql[i] = String::toString(nAnt) +
               "*ANTENNA1-(ANTENNA1*(ANTENNA1-1))/2+ANTENNA2-ANTENNA1";
         label[i] = String("Baseline number with ") 
                     + String::toString(nAnt) + String(" antennas");
      } 
      else if (!upcase(axis(i)).compare("UVDIST") || 
               !upcase(axis(i)).compare("UVDISTANCE")) {
         Quantum<Double> c = QC::c;
         Double cVal = c.getValue();
         ostringstream os;
         os << std::setprecision(16) << cVal;
         curTaql[i] = String("SQRT(SUMSQUARE(UVW[1:2])) * SIGMA[1] /")
                     + String(os.str());
         label[i] = String( "UV Distance (klambda )" );
      } 
      else if (!upcase(axis(i)).compare("CHANNEL") || 
               !upcase(axis(i)).compare("CHANNELS") ||
               !upcase(axis(i)).compare("CHAN")) {
         dTaql(i).resize(sect);
         for (Int s = 0; s < sect; s++) { 
            String mdata = String("(CORRECTED_DATA[") + 
                         String::toString(itsStartCorr(0)(s)) +
                         String(":") +
                         String::toString(itsEndCorr(0)(s)) +
                         String(":") +
                         String::toString(itsStepCorr(0)(s)) +
                         String(",1:") +
                         String::toString(nAveChan) +
                         String(":1])");
            if (useAveChanId) {
               dTaql[i][s] = String("REAL") + mdata;
            }
            else {
               dTaql[i][s] = String("IMAG") + mdata;
            }
         }
         label[i] = "Channels";
      } 
      else if (!upcase(axis(i)).compare("CHANFREQ") || 
               !upcase(axis(i)).compare("CHANNEL_FREQ") || 
               !upcase(axis(i)).compare("CHANNEL_FREQUENCY") || 
               !upcase(axis(i)).compare("CHAN_FREQ") || 
               !upcase(axis(i)).compare("FREQUENCY") || 
               !upcase(axis(i)).compare("FREQ")) {
         dTaql(i).resize(sect);
         for (Int s = 0; s < sect; s++) { 
            String mdata = String("(MODEL_DATA[") + 
                         String::toString(itsStartCorr(0)(s)) +
                         String(":") +
                         String::toString(itsEndCorr(0)(s)) +
                         String(":") +
                         String::toString(itsStepCorr(0)(s)) +
                         String(",1:") +
                         String::toString(nAveChan) +
                         String(":1])");
            dTaql[i][s] = String("IMAG") + mdata;
         }
         label[i] = "Channel Frequency (GHz)";
      } 
      else if (!upcase(axis(i)).compare("CHANVELOCITY") ||
               !upcase(axis(i)).compare("CHANNEL_VELOCITY") || 
               !upcase(axis(i)).compare("CHAN_VELOCITY") || 
               !upcase(axis(i)).compare("CHANVELOCITY")) {
         dTaql(i).resize(sect);
         for (Int s = 0; s < sect; s++) { 
            String mdata = String("(MODEL_DATA[") + 
                         String::toString(itsStartCorr(0)(s)) +
                         String(":") +
                         String::toString(itsEndCorr(0)(s)) +
                         String(":") +
                         String::toString(itsStepCorr(0)(s)) +
                         String(",1:") +
                         String::toString(nAveChan) +
                         String(":1])");

            dTaql[i][s] = String("REAL") + mdata;
         }
         label[i] = "Channel Velocity (km/s)";
      }
      else if (!upcase(axis(i)).compare("CORRELATION") || 
               !upcase(axis(i)).compare("CORRELATIONS") || 
               !upcase(axis(i)).compare("CORR")) {
         curTaql[i] = "CROSS";
         label[i] = "Correlation";
      } 
      else if (!upcase(axis(i)).compare("U") ||
               !upcase(axis(i)).compare("U-U")) {
         curTaql[i] = "UVW[1]";
         label[i] = String("U (m)");
      } 
      else if (!upcase(axis(i)).compare("V") ||
               !upcase(axis(i)).compare("V-V")) {
         curTaql[i] = "UVW[2]";
         label[i] = String("V (m)");
      } 
      else if (!upcase(axis(i)).compare("W") ||
               !upcase(axis(i)).compare("W-W")) {
         curTaql[i] = "UVW[3]";
         label[i] = String("W (m)");
      } 
      else {
         curTaql[i] = "";
      }
   }

   //cout << "curTaql=" << curTaql(0) << ", " << curTaql(1) << endl;
   //cout << "dTaql=" << dTaql(0) << ", " << dTaql(1) << endl;

   //if (curTaql[0] == "" || curTaql[1] == "") {
   //   xlabel = "";
   //   ylabel = "";
   //   title = "";
   //   log->out(String("Time averaging of '") + y + " vs " + x +
   //            "' is not supported",
   //            fnname, clname, LogMessage::WARN, True); 
   //   log->FnExit(fnname, clname);
   //   return False;
   //}

   if (curTaql[0] != "" || curTaql[1] != "") {
      itsTaqlStrings[0][0].resize(2);
      itsTaqlStrings[0][0] = curTaql;
   }

   Int nd0 = dTaql(0).nelements();
   if (nd0 > 0 && curTaql[1] != "") {
      itsTaqlStrings[0][0].resize(2 * nd0);
      for (Int s = 0; s < nd0; s++) {
         itsTaqlStrings[0][0][2 * s] = dTaql[0][s];
         itsTaqlStrings[0][0][2 * s + 1] = curTaql[1];
      }
   }

   Int nd1 = dTaql(1).nelements();
   if (nd1 > 0 && curTaql[0] != "") {
      itsTaqlStrings[0][0].resize(2 * nd1);
      for (Int s = 0; s < nd1; s++) {
         itsTaqlStrings[0][0][2 * s] = curTaql[0];
         itsTaqlStrings[0][0][2 * s + 1] = dTaql[1][s];
      }
   }

   if (nd0 > 0 && nd1 > 0 && nd1 == nd0) {
      itsTaqlStrings[0][0].resize(2 * nd1);
      for (Int s = 0; s < nd0; s++) {
         itsTaqlStrings[0][0][2 * s] = dTaql[0][s];
         itsTaqlStrings[0][0][2 * s + 1] = dTaql[1][s];
      }
   }

   xlabel = label(0);
   ylabel = label(1);

   if (itsTaqlStrings[0][0].nelements() == 0) {
      xlabel = "";
      ylabel = "";
      title = "";
      log->out(String("Time averaging of '") + y + " vs " + x +
               "' is not supported",
               fnname, clname, LogMessage::WARN, True); 
      log->FnExit(fnname, clname);
      return False;
   }

   log->FnExit(fnname, clname);
   return True;
}
//#////////////////////////////////////////////////////////////////////////////
//# Create TaQL string which is used to select the data from the
//# table.  Note that there can be conversion information added to
//# the TaQL string as well.
casa::Bool
MsPlot::createTaQL( const String& x, 
   const String& y, 
   const String& xcolumn, 
   const String& ycolumn, 
   const String& xvalue,
   const String& yvalue,
   String& title,
   String& xlabel,
   String& ylabel)
{

   String FnCall = String("(x=") + x + ", y=" + y +
                   ", xcolumn=" + xcolumn + ", ycolumn=" + ycolumn +
                   ", xvalue=" + xvalue + ", yvalue=" + yvalue +
                   ", title=" + title + ", xlabel=" + xlabel +
                   ", ylabel=" + ylabel + ")" ;
   String fnname = "createTaQL";
   log->FnEnter( fnname +  FnCall, clname );
   //cout << fnname << FnCall << endl;
   //# Turn off UVW plot indicator.  If we have a UVW plot we'll find out
   //# soon, when we construct the TaQLs;
   itsIsUVWPlot = False;
    
   //cout << "itsTableVectors.nelement=" 
   //     << itsTableVectors.nelements() << endl;
   //# Loop through the vector of table vectors, creating a set of
   //# TaQLs for each table.
   Int spwId = -1;
   Int polId = -1;
   Vector<String> currentTaql(1);
   currentTaql[0] = String("");
   itsTaqlStrings.resize(itsTableVectors.nelements() );   
   for (uInt tblVecCnt=0; 
        tblVecCnt < itsTableVectors.nelements(); tblVecCnt++)   {
      //cout << "Table SPWs: " << itsTableVecSpwIds[tblVecCnt] << endl;
      //#cout << "Table POLs: " << itsTableVecPolIds[tblVecCnt] << endl;    
      itsTaqlStrings[tblVecCnt].resize(itsTableVectors[tblVecCnt].nelements());
      for (uInt tblCnt=0; 
           tblCnt < itsTableVectors[tblVecCnt].nelements(); tblCnt++ ) {
         //#cout << "Creating TaQL for table set: " << tblVecCnt << endl;
         //#cout << "Creating TaQL for table: " << tblCnt << endl;
         if (spwId == itsTableVecSpwIds[tblVecCnt][tblCnt] 
                 && polId == itsTableVecPolIds[tblVecCnt][tblCnt] ) {
            //# No point in recalculating the TaQL it!  The 
            //# TaQLs will be the same as long as the SPW has
            //# not changed.  
            //# SDJ added continue here, wasn't here before.
            itsTaqlStrings[tblVecCnt][tblCnt] = currentTaql;
            continue;
         } 
         else {
            currentTaql.resize(1);
            currentTaql[0] = String("");
            spwId = itsTableVecSpwIds[tblVecCnt][tblCnt];
            polId = itsTableVecPolIds[tblVecCnt][tblCnt];
         }
   
         //# Not all 3 of x, y, and column can be specified?
       
         //# (1) Get the TaQL strings for X and Y.
         //#     There can be more than one each, for X or Y.
         //#     Defaul atxes labels are also created and returned.
         Vector<String> xTaQL, yTaQL;
         Vector<String> tmpX, tmpY;
         try {
            tmpX = getTaQL( x, xcolumn, xvalue, spwId, polId, 'x', xlabel );
            tmpY = getTaQL( y, ycolumn, yvalue, spwId, polId, 'y', ylabel );
         } catch ( AipsError ae ) {
            throw ae;
         }
       
         //cout << "tmpX: " << tmpX << endl;
         //cout << "tmpY: " << tmpY << endl;
    
         //# (2) UR's attempt to create a sensible title.  Not pretty.
         //#     SDJ: Took off the itsSelectString, what data has been
         //#     selected or not.   
         title = "";
               //ylabel.before("(") + String(" vs ") 
               //+ xlabel.before("(");
       
         //# (3) Match up all pairs of X and Y TaQLs
         //#     TablePlot can take in any number of pairs : [x,y,x,y,x,y,....]
         //#     all which are applied to the same vector<Table> sent in.
         //# The uvcoverage is a special-case plot, where it is a 
         //# one-to-one mapping.

         //#if( upcase(x).matches("U") && upcase(y).matches("V") 
         //#   && tmpX.nelements()==tmpY.nelements() )

         if (itsIsUVWPlot && tmpX.nelements()==tmpY.nelements()) {
            currentTaql.resize( tmpX.nelements() + tmpY.nelements());
            for (uInt cnt=0, j=0; j<tmpX.nelements(); j++ ) {
               currentTaql[cnt] = tmpX[j];
               currentTaql[cnt+1] = tmpY[j];
               cnt += 2;
            } 
         }
         else {
            //This part make bogus point for data-data plotting
            //
            //currentTaql.resize((tmpX.nelements() * tmpY.nelements() * 2));
            //for (uInt cnt=0,j=0; j<tmpX.nelements(); j++ ) 
            //   for (uInt k=0; k<tmpY.nelements(); k++ ) {
            //      currentTaql[cnt] = tmpX[j];
            //      currentTaql[cnt+1] = tmpY[k];
            //      cnt += 2;
            //   }        
            
            uInt ndx = tmpX.nelements();
            uInt ndy = tmpY.nelements();
            //cout << "ndx=" << ndx << " ndy=" << ndy << endl;
            if (ndx > 0 && ndy > 0 && ndx == ndy) {
               currentTaql.resize(ndx + ndy);
               for (uInt cnt = 0, j = 0; j < ndx; j++) { 
                  currentTaql[cnt] = tmpX[j];
                  currentTaql[cnt + 1] = tmpY[j];
                  cnt += 2;
               }        
            }
            else {
               currentTaql.resize((tmpX.nelements() * tmpY.nelements() * 2));
               for (uInt cnt=0,j=0; j<tmpX.nelements(); j++ ) { 
                  for (uInt k=0; k<tmpY.nelements(); k++ ) {
                     currentTaql[cnt] = tmpX[j];
                     currentTaql[cnt+1] = tmpY[k];
                     cnt += 2;
                  }
               }        
            }

            //for the vector-array plot, e.g., weight vs. amp
            //the taql generated this way contains too many points.
            //
            //example: 4 pols, 1 chan,
            // weight=[.1, .2, .3, .4]
            // amp   =[1, 2, 3, 4]
            //it is proper to plot 4 points, 
            //   (.1, 1), (.2, 2), (.3, 3), (.4, 4)
            // currently, it plot 16 points, 12 bogus more
            //
            
         }
       
         //cout << "New TaQL: " << currentTaql << endl;
         //# Now store the created TaQL to be retrieved later.
         itsTaqlStrings[tblVecCnt][tblCnt]= currentTaql;
      }
   }
    
   log->out("Done createTaQL", fnname, clname, LogMessage::DEBUG1);
    
   Bool retValue = True;
   log->FnExit( fnname, clname);
   return retValue;
}

//#////////////////////////////////////////////////////////////////////////////
//# Construct TaQL strings for various cases.
//# Also construct default axis labels here.
//#////////////////////////////////////////////////////////////////////////////
casa::Vector<casa::String>
MsPlot::getTaQL( const String& axisStr, const String& column,   
        const String& value, const uInt spwId, const uInt polId, 
        const char axis, String& label)
{
   //# TODO define the constant strings used for the TaQLs some place.

   String FnCall = String( "( " ) + axisStr + String( ", " ) 
                  + column + String( ", " ) +value + String( ", " ) 
                  + String::toString( spwId ) + String( ", " ) + axis 
                  + String( ", " ) + label + String( " )" );
   String fnname = "getTaQL";
   log->FnEnter( fnname +  FnCall, clname );
    
   Vector<String> retValue;
   if (!axisStr.compare( "" ) 
          || ! upcase(axisStr).compare( "DATA" )
          || ! upcase(axisStr).compare( "RESIDUAL" )
          || ! upcase(axisStr).compare( "CORRECTED" )
          || ! upcase(axisStr).compare( "MODEL" )
          || (upcase(axisStr).contains( "WEIGHTTED" ) 
              && !upcase(axisStr).contains( "DEFAULT" ) ) 
          || ( upcase(axisStr).contains( "WEIGHT" ) 
                && ! upcase(axisStr).contains( "DEFAULT" ))) {   
      //# This must be using one of the data columns so construct
      //# the TaQL from the column and value paramenters
      //# Also use 'average' and 'steporwidth'.
      //# This uses the current channel and correlation selections.
      retValue.resize(0);
      if (upcase(axisStr).contains( "WEIGHTTED" )
          || upcase(axisStr).contains( "WEIGHT" ))
         retValue = dataTaQL( axisStr, value, spwId, polId, label );
      else
         retValue = dataTaQL( column, value, spwId, polId, label );

   } 
   else if (!upcase(axisStr).compare( "TIME" )) {
      //# CASA stores dates in MJD, but matplot lib uses
      //# 01/01/0001 as the first date so we need to convert
      retValue.resize(1);
      retValue[0] = "TIME/86400.0 + 678576.0";
      label = String("Time");

      //# itsPlotOptions needs to set its TimePlotChar.   
      //# This happens in updatePlotOptions.
      //# --> 'timeplot' parameter is not visible to the user
      //#     they will expect that if they say X='time', then
      //#     the time formatting is automatic.
      //# TODO : Allow 'timeplot' option anyway - for TaQL string writers....
    
   } 
   else if (!upcase(column).compare( "ARRAY" )) {
      //# The values ploted are completely handled by the conversion
      //# class MsPlotConvertArray().  So we don't need a TaQL string
      //# here.
      //# and subtract this number from the POSITION...
      //# Clarify if this is needed first
      //#
      //# we flip the axis if axis is they axis.

      retValue.resize(1);
      if (upcase(axisStr).matches("XEAST")) {
         retValue[0] = String("POSITION[1]");
         label = String("X (m)");
      } 
      else if (upcase(axisStr).matches("YNORTH")) {
         retValue[0] = String("POSITION[2]");
         label = String("Y (m)");
      } 
      else {
         String msg = String( "Unrecognized array plot axis " )
             + axisStr + String( ". Will attempt to proceed anyway." );
         log->out( msg, fnname, clname, LogMessage::WARN );
         retValue[0] = axisStr;
         label = axisStr + String(" (m)");
      }
   } 
   else if (!upcase( axisStr).compare( "AZIMUTH" )
            || !upcase(axisStr).compare( "ELEVATION" )
            || !upcase(axisStr).compare( "HOURANGLE" )
            || !upcase(axisStr).compare( "HOUR_ANGLE" )
            || !upcase(axisStr).compare( "PARALLACTICANGLE" ) 
            || !upcase(axisStr).compare( "PARALLACTIC_ANGLE" )) {
      retValue.resize(1);
      retValue[0] = "TIME";
   
      if ( ! itsConvertClassSet ) {
         //# We didn't successfully setup the convert fucntion
         //# when we were setting the plot options.  report the
         //# error, well throw an exception.
         String msg = String("Failed to setup data conversion function for ")
                             +  axisStr +String( "." );
         log->out( msg, fnname, clname, LogMessage::WARN, True);
      }
   
      if (!upcase( axisStr).compare( "AZIMUTH" ) )
         label = String("Azimuth (deg)");
      if (!upcase( axisStr).compare( "ELEVATION" ) )
         label = String("Elevation (deg)");
      if (!upcase( axisStr).compare( "HOURANGLE" ) )
         label = String("Hour Angle (deg)");
      if (!upcase( axisStr).compare( "PARALLACTICANGLE" ) )
         label = String("Parallactic Angle (deg)");
   } 
   else if ( !upcase(axisStr).compare( "BASELINE" ) ) {
      //# TODO, put the calculation of the number of
      //# antennas into a method.  This isn't accurate
      //# because it doen't take into acct. setdata()
      //#  -> This is probably okay as it is... since if the user
      //#     is interested in baseline 5-8 which may be 224 (say..)
      //#     Then, if they do a sub-selection on antennas 4-10 only..
      //#     And then want to look at the same plot, the baseline 5-8
      //#     should retain the same baseline number.....
      ROMSColumns msColumn( itsSelectedMS );
      const ROMSAntennaColumns & antColumns = msColumn.antenna();
      uInt numAnts = antColumns.nrow();
   
      //# Baseline number is calculated as:
      //#   nAnt * ANTENNA1+ANETNNA2-(ANTENNA1-1)*(ANTENNA1+2)/2
      //#
      //# SDJ June 8, 2007
      //# Changed calculation to the same calculation for mapping
      //# 2D arrays into continuous memory.
      //# 
      //# (nAnt * ANTENNA1) + ANTENNA2
      //#
      //# This formula maps the baseline (antenna pairs) uniquely to a 
      //# baseline number.
      retValue.resize(1);
      retValue[0] = String( "(" )  + String::toString( numAnts ) 
                           + String( "*ANTENNA1" ) + String( ")")
                           + String( "+ANTENNA2" );
      label = String("Baseline number with ") 
                     + String::toString( numAnts ) + String(" antennas");
   } 
   else if ( !upcase(axisStr).compare( "CHANNEL" ) 
              || !upcase(axisStr).compare( "CHANNELS" )
              || !upcase(axisStr).compare( "CHAN" ) ) {
      retValue.resize(1);
      retValue[0] = "CROSS";
      label = "Channels";
   } 
   else if ( !upcase(axisStr).compare( "CHANFREQ" ) 
              || !upcase(axisStr).compare( "CHANNEL_FREQ" )
              || !upcase(axisStr).compare( "CHANNEL_FREQUENCY" )
              || !upcase(axisStr).compare( "CHAN_FREQ" ) 
              || !upcase(axisStr).compare( "FREQUENCY" )
              || !upcase(axisStr).compare( "FREQ" ) ) {
      retValue.resize(1);
      retValue[0] = "CROSS";
      label = "Channel Frequency (GHz)";
   
   } 
   else if ( !upcase(axisStr).compare( "CHANVELOCITY" ) 
              || !upcase(axisStr).compare( "CHANNEL_VELOCITY" )
              || !upcase(axisStr).compare( "CHAN_VELOCITY" ) 
              || !upcase(axisStr).compare( "VELOCITY" ) ) {
      retValue.resize(1);
      retValue[0] = "CROSS";
      label = "Channel Velocity (km/s)";
   } 
   else if ( !upcase(axisStr).compare( "CORRELATION" )
               || !upcase(axisStr).compare( "CORRELATIONS" ) 
               || !upcase(axisStr).compare( "CORR" ) ) {
      retValue.resize(1);
      retValue[0] = "CROSS";
      label = "Correlation";
   } 
   else if ( !upcase(axisStr).compare( "DEFAULT_WEIGHT" ) 
              || !upcase(axisStr).compare( "DEFAULTWEIGHT" ) ) {
      // !!! Wrong
      retValue.resize(2);
      retValue[0] = "WEIGHT[1]";
      retValue[1] = "WEIGHT[2]";
      label = "Weights";
   } 
   else if ( !upcase(axisStr).compare( "UVDIST" ) 
              || !upcase(axisStr).compare( "UVDISTANCE" ) ) {
   
      //# Plot the uvdistance, but in kilo-lambda, so we need
      //# to calculate a conversion value.  NOTE: is this good
      //# enough or do we need a Convert class SDJ
      retValue.resize(1);
      Double convertValue = getMeterToKlambdaConvertValue( spwId );
      ostringstream os;
      os << std::setprecision(16) << convertValue;
      
      retValue[0] = String( "SQRT(SUMSQUARE(UVW[1:2])) / " )
                        + String( os.str() );
      label = String( "UV Distance (klambda )" );
   
   } 
   else if ( !upcase(axisStr).compare( "U" ) ) {
      //# create the taQL for the x-axis of the uvcoverage plot
      retValue.resize(1);
      retValue[0] = "UVW[1]";
      label = String("U (m)");
   
      itsIsUVWPlot = True;
   } 
   else if ( !upcase(axisStr).compare( "-U" ) ) {
      //# create the taQL for the x-axis of the uvcoverage plot
      retValue.resize(1);
      retValue[0] = "-UVW[1]";
      label = String("U (m)");
   
      itsIsUVWPlot = True;
   } 
   else if ( !upcase(axisStr).compare( "U-U" ) ) {
      //# create the taQL for the x-axis of the uvcoverage plot
      retValue.resize(2);
      retValue[0] = "UVW[1]";
      retValue[1] = "-UVW[1]";
      label = String("U (m)");
   
      itsIsUVWPlot = True;
   } 
   else if ( !upcase(axisStr).compare( "V" ) ) {
      //# create the taQL for the y-axis of the uvcoverage plot
      retValue.resize(1);
      retValue[0] = "UVW[2]";
      label = String("V (m)");
   
      itsIsUVWPlot = True;
   } 
   else if ( !upcase(axisStr).compare( "-V" ) ) {
      //# create the taQL for the y-axis of the uvcoverage plot
      retValue.resize(1);
      retValue[0] = "-UVW[2]";
      label = String("V (m)");
   
      itsIsUVWPlot = True;
   } 
   else if ( !upcase(axisStr).compare( "V-V" ) ) {
      //# create the taQL for the x-axis of the uvcoverage plot
      retValue.resize(2);
      retValue[0] = "UVW[2]";
      retValue[1] = "-UVW[2]";
      label = String("V (m)");
   
      itsIsUVWPlot = True;
   } 
   else if ( !upcase(axisStr).compare( "W" ) ) {
      //# create the taQL for the y-axis of the uvcoverage plot
      retValue.resize(1);
      retValue[0] = "UVW[3]";
      label = String("W (m)");
   
      itsIsUVWPlot = True;
   } 
   else if ( !upcase(axisStr).compare( "-W" ) ) {
      //# create the taQL for the y-axis of the uvcoverage plot
      retValue.resize(1);
      retValue[0] = "-UVW[3]";
      label = String("W (m)");
   
      itsIsUVWPlot = True;
   } 
   else if ( !upcase(axisStr).compare( "W-W" ) ) {
      //# create the taQL for the x-axis of the uvcoverage plot
      retValue.resize(2);
      retValue[0] = "UVW[3]";
      retValue[1] = "-UVW[3]";
      label = String("W (m)");
   
      itsIsUVWPlot = True;
    } 
    else {
      retValue.resize(1);
      retValue[0] = upcase( axisStr );
      label = axisStr;
    }
    
    log->FnExit( fnname, clname);
    return retValue;
}
    
//#////////////////////////////////////////////////////////////////////////////
//#////////////////////////////////////////////////////////////////////////////
//# 
//#           ITERATIVE PLOTTING ROUTINES
//#
//#                   - need only iterPlotNext() and iterPlotStop().
//#                   - Iteration plots are started from MsPlot::plotxy.
//#
//#////////////////////////////////////////////////////////////////////////////
Bool
MsPlot::iterPlotNext()
{
    String fnname = "iterPlotNext";
    log->FnEnter( fnname, clname );

    casa::Bool rstat( casa::True );
    if ( ! checkInit() || ! checkOpenMS() )  { 
       log->FnExit( fnname, clname); 
       return rstat=False; 
    }

    if( ! itsIterPlotOn ) 
    { 
       log->out( "Please start the iterations first...", 
           fnname, clname, LogMessage::WARN );

   return rstat = False;
    }

    //# These are holders for column names and labels, corresponding to the 
    //# number of plots on the window.
    //# By default, TablePlot creates the title label as follows
    //# 
    //# title = title + : + labcol + : labval --> concatenated for multiple labcol,labval pairs.
    //#
    //# These strings are returned here, in case MsPlot needs to do something different
    //# from the default. 
    //#
    //# Labels can be over-ridden post-facto.... using the TP.runPlotCommand().
    //#
    //# (there may be a better way to do this, but I'm not sure how.....)
    //#
    casa::Vector<casa::String> labcol;
    casa::Vector<casa::Vector<casa::Double> > labval;
    
    //# Call next()
    if ( itsTablePlot->iterMultiPlotNext( labcol, labval ) == -1 )
       iterPlotStop( casa::False );
      
    log->FnExit( fnname, clname);
    return rstat;
}


//#///////////////////////////////////////////////////////////////////////////
//#
Bool
MsPlot::iterPlotStop( const Bool rmplotter )
{
    String FnCall = String( "( " ) + String::toString( rmplotter )
           + String( " ) " );
    String fnname = "iterPlotStop";
    log->FnEnter( fnname + FnCall, clname );
    casa::Bool rstat( casa::True ); 
    if ( ! checkInit() || ! checkOpenMS() )  { 
        log->FnExit( fnname, clname); 
        return rstat=False; 
    }

    if( ! itsIterPlotOn ) 
    { 
       log->out( "Iteration plotting is already off.", fnname, 
                  clname, LogMessage::NORMAL );
       return rstat = False;
    }

    //-----itsTablePlot->iterMultiPlotStop( rmplotter );
    itsTablePlot->changeGuiButtonState("iternext","disabled");
    itsIterPlotOn = casa::False;

    //# Cleanup, undo any internal plot options that may have
    //# been set internally.  

    //--------cleanupOptions();

    //#
    log->out( "Iteration plotting has been turned off.",
      fnname, clname, LogMessage::NORMAL);

    log->FnExit( fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//# This method is based on Gary Li's original version, before the
//# refactoring of MsPlot
//#
Bool
MsPlot::corrSelection( const String& corrExpr, 
               Vector<Vector<String> >& corrNames )
{
    String FnCall = String( "( " ) + corrExpr 
   + String( ", corrNames )" );
    String fnname = "corrSelection";
    log->FnEnter( fnname + FnCall, clname );
    Bool rstat = True;
    
    //# TODO adding an assert that the itsStartCorr, itsEndCorr,
    //# and istStepCorr all have length == itsMaxSpwId

    //# Make sure we have nothing to start.
    corrNames.resize( itsMaxPolId );
    for( uInt i=0; i < itsMaxPolId; i++ )
   corrNames[i].resize(0);
    
    //# Parse the corrExpr and fill in the corrNames for each spw ID
    if (  corrExpr.length() && !corrExpr.matches(  String("") ) ) {
       rstat = corrAndStokesParser( corrExpr, corrNames );
    } else {
   //# Empty correlation String, We've selected all correlations.
   //# So fill the corrNames for all polarizations.  This is done in 
   //# the validator, so we don't do it here.
    }
    
    //# At this point we have an identical list of selected correlations
    //# for all polars.  We now need to go through this list an prune any
    //# that are invalid for a particular correlation, we also need to
    //# remove any duplicates, and we order them.  We also fill in the
    //# itsStartCorr, itsEndCorr, and itsStepCorr values.
    if( rstat ) {
         rstat = validateCorrAndStokes( corrNames );
    } else {
       log->out( String( "Unable to parse correlations given: " ) + corrExpr,
              fnname, clname, LogMessage::SEVERE );
       rstat = False;
    }

    log->FnExit( fnname, clname);
    return rstat;   
}

 //#///////////////////////////////////////////////////////////////////////////
 //# This method originally written by Gary Li, it has been copied over more
 //# or less intact when MsPlot was refactored. 
 //#
 //# corrParser parses the input String of corr parameter in the setData(...) 
 //# method into correlation type names. The input is in the format "RR LL RL", 
 //# or "[RR LL RL]" or "(RR LL RL)" different types of polarizations is
 //# separated by space. "XX", and "XY" are also accepted, as well as the
 //# stokes types "I", "Q", "U", and "V" (maybe?).
 //#
 //# 
 Bool
 MsPlot::corrAndStokesParser( const String& corrExpr, 
          Vector<Vector<String> >& corrNames )
 {
     String FnCall ( String( "( " ) + corrExpr + String( ", corrNames )" ));
     String fnname ( "corrAndStokesParser");
     log->FnEnter( fnname + FnCall, clname );

     //# For now we are just parsing the input, not checking the validity
     //# so we make the list of correlation names the same for all 
     //# polarss, another method will remove or complain about extra values.

     Vector<String> names;
     //# Strip of any "(", ")", "[", or "]" that might be in the string.
     uInt startPos = 0, endPos = corrExpr.length() - 1;
     uInt sbLen = endPos + 1;
     if( corrExpr.length() > 0 && ((corrExpr.chars())[0] == '(' || (corrExpr.chars())[0]=='['))
     {
         startPos = 1;
	 sbLen = sbLen - 1;
     }

     if( corrExpr.length() > 0 && ((corrExpr.chars())[endPos] == ')' || (corrExpr.chars())[endPos] == ']'))
         sbLen = sbLen-1;
     String corrExprPure = corrExpr.substr( startPos, sbLen );
     log->out(String("Purified correlation expression: ")+corrExprPure,
              fnname, clname, LogMessage::NORMAL );
     
     //cout << "corrExprPure=" << corrExprPure << endl;

     //# Now pull out the correlation information.
     Int nmax = (uInt)Stokes::NumberOfTypes;
     String stokesNm[ nmax ];         
     uInt totalStokes = split( corrExprPure, stokesNm, nmax, "," );
     
     //# This bit remains for backward compatibility, but this 
     //# allows spaces between the correlation values.
     if ( totalStokes <= 1 )
    totalStokes = split( corrExprPure, stokesNm, nmax, " " );

     //# normalize everything by making it upper case and removed
     //# leading and trailing spaces.
     names.resize( totalStokes );
     for( uInt i=0; i<totalStokes; i++ ){
    String tmpName = upcase(stokesNm[i]);
    Int spaceIndex;
    while( (spaceIndex = tmpName.index(" ")) >= 0 )
        tmpName.del(spaceIndex,1);
    names[ i ] = tmpName;
     }
     
     //# Now put the list of names into our data structure for all
     //# polarizations, the index is the porization id or row.
     for ( uInt i=0; i<itsMaxPolId; i++ )
     {
    corrNames[i].resize( names.nelements() );
    corrNames[i] = names;
    //cout << "corrNames[" << i << "]=" << names << endl;
     }
     log->FnExit( fnname, clname);
     return True;

 }//# end of corrParser

    

//#////////////////////////////////////////////////////////////////////////////
//# This method originally written by Gary Li, it has been copied over more
//# or less intact when MsPlot was refactored. 
//# 
//# This method examines the correlation information given to the setdata
//# "correlations" paramenter and verifies the correctness of them.  It accepts
//# both correlation and stokes names (RR, RL, XX, YX, U, V, ... )
//#
//# Both the name and id list are by SPW id
//#
Bool
MsPlot::validateCorrAndStokes( Vector<Vector<String> >& names )
{
    String fnname = "validateCorrAndStokes";
    log->FnEnter( fnname + "names", clname);

    casa::Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { log->FnExit( fnname, clname); return rstat=False; }

    //# Make sure our list of correlation IDs is the same size as
    //# our list of names.
    Vector<Vector<Int> > ids( names.nelements() );
    for( uInt i=0; i < ids.nelements(); i++ )
    {
   ids[i].resize( names[i].nelements() );
    }
    

    //# We need to keep track if there are any matches.  Its an error
    //# if there is no matches found.
    Bool foundNoMatches = True;
    Bool allZeroLength = True;
     
    //# Different SPWs may have different correlation values. 
    //# If a user has specified a correlation value that is not
    //# present for a particular SPW it is ignored, but a msg
    //# is written to the logger.
    for( uInt polId=0; polId < itsMaxPolId; polId++ )
    {
   //# If we have a name length > 0 then we have an input.
   if ( names[polId].nelements() > 0 )
       allZeroLength = False;

   //# Find the list of correlations that match the users selection.
   if ( //#names[polId].nelements() < 1 ||
             ( names[polId].nelements() > 0 
             &&  names[polId][0].matches( String('*' ) ) ) )
   {   
       //# If the user gave us '*' the we use the full compliment of
       //# correlations
       foundNoMatches = False;
       names[polId].resize( itsCorrelationNames[polId].nelements() );
       for( uInt j=0; j < itsCorrelationNames[polId].nelements(); j++ )
      names[polId][j] = itsCorrelationNames[polId][j];
       ids[polId].resize( itsCorrelationIds[polId].nelements() );
       for( uInt j=0; j < itsCorrelationIds[polId].nelements(); j++ )
      ids[polId][j] = itsCorrelationIds[polId][j];
       rstat = True;
   } else {
       //# Go through the list making sure we don't have any duplicates.
       //# SDJ I bet the CASA::String class has some way of scanning the
       //# whole vector for a particular value.
       uInt numFound = 0;   
       Bool found = False;
      
       //Vector<String> validNames(names[polId].nelements());
       //Vector<Int> validIds(itsCorrelationIds[polId].nelements());

       
       //do this to avoid crash by correlation='rr lr ll rl rl rr ll'
       Int len = std::max(itsCorrelationIds[polId].nelements(),
                     names[polId].nelements());
       Vector<String> validNames(len);
       Vector<Int> validIds(len);
       //cout << "len=" << len << endl;
       //cout << "names=" << names[polId] << endl;
       //cout << "itsCorrelationIds=" << itsCorrelationIds[polId] << endl;
       for( uInt i=0; rstat && i<names[polId].nelements(); i++ )
       {
      found = False;
      for( uInt j=0; j<itsCorrelationNames[polId].nelements(); j++ )
      {
          if( names[polId][i].matches( itsCorrelationNames[polId][j] ))
          {
         validNames[ numFound ] = names[polId][i];
         validIds[ numFound ] = j+1; //# Add one because TaQLs are 1-based.
         numFound++;
         found = True;
          }
      }      
      if( !found ){
          /*
          *itsLog << casa::LogIO::WARN
             << LogOrigin( "MSPlot", "ValidateCorr" )
             << "Invalid correlation given for polarization row " 
             << String::toString(polId) << " : " 
             << names[polId][i]
             << ". This correlation will be ignored."
             << " The valid correlation values for this MS are: "
             << itsCorrelationNames[polId]
             <<casa::LogIO::POST;
          */
      } 
       }//# end of i loop

   
       //cout << "numFound=" << numFound << endl; 
       //# Resize the name list to the number of valid names found.   
       //cout << "validNames=" << validNames << endl;
       //cout << "validIds=" << validIds << endl;       
       validNames.resize( numFound, True );
       validIds.resize( numFound, True );   
       if ( numFound > 0 )
      foundNoMatches = False;

       //cout << "validNames.nelements()=" << validNames.nelements() << endl;
       //cout << "names.nelements()=" << names.nelements() << endl;
       if ( numFound < 1 )
       {
      //# If there were no valid correlations given then warn
      //# the user.
      //#*itsLog << casa::LogIO::WARN 
      //#   << LogOrigin( "MSPlot", "ValidateCorr" )
      //#   << "No valid correlation values were found for row "
      //#   << String::toString( polId )
      //#   << casa::LogIO::POST;
      names[polId].resize( 0 );
      ids[polId].resize( 0 );
       } else if ( rstat && validNames.nelements() != names.nelements() ) {
      //# If there were some invalid names we need to resize and get
      //# rid of them.
      names[polId].resize( validNames.nelements() );
      names[polId] = validNames;
      ids[polId].resize( validIds.nelements() );
      ids[polId] = validIds;
       } else {
      //# All names were okay, so do nothing.
      ids[polId].resize( validIds.nelements() );   
      ids[polId] = validIds;
       }
       //# Make sure the list is sorted and duplicates are reomved.
       uInt numIds = 
      GenSort<Int>::sort( ids[polId], Sort::Ascending, Sort::NoDuplicates );
       if ( numIds != ids[polId].nelements() )
       {
      ids[polId].resize( numIds, True );
      names[polId].resize( numIds );
      
      ROMSPolarizationColumns polarColumn( itsMS->polarization());
      const Vector<Int> corrType = polarColumn.corrType()(polId);
      for( uInt k=0; k < numIds; k++ ){
          Int corr = ids[polId][k]-1;
          names[polId][k] = Stokes::name( Stokes::StokesTypes(corrType[corr]));
      }
       }
   }   
    }

    if ( foundNoMatches && !allZeroLength ) 
    {
   //# The user supplied arguments have no matches.  This is
   //# a failure state. This log message may be overkill, but
   //# there may be some instances where there are no other 
   //# errors found.
   log->out("There were no matches found in the correlation selection ",
       fnname, clname, LogMessage::SEVERE );
       rstat = False;
    } else if ( foundNoMatches && allZeroLength ) {
   //# User entered nothing so we use all correlations names.
   for ( uInt polId=0; polId < itsMaxPolId; polId++ )
   {
       names[polId].resize( itsCorrelationNames[polId].nelements() );
       names[polId] = itsCorrelationNames[polId];
       ids[polId].resize( itsCorrelationIds[polId].nelements() );
       ids[polId] = itsCorrelationIds[polId];
   }
    }
   
    if ( rstat ) 
    {
   //# We need to make our list of correlation information.
   makeCorrIdLists( ids );
    }

    log->FnExit( fnname, clname );
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//#  Make the list of correlation start, end, and step values.
//#  These list are used in the creation of TaQL string.

Bool
MsPlot::makeCorrIdLists( Vector< Vector<Int> >& ids )
{
    String fnname = "makeCorrIdLists";
    log->FnEnter( fnname + "(ids)",  clname);

   casa::Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { 
        log->FnExit( fnname, clname); 
        return rstat=False; 
    }
    
     //for (uInt k = 0; k < ids.nelements(); k++)
     //     cout << "k=" << k << " ids=" << ids(k) << endl;

    //# Set our global parameters to store the info we need to
    //# check for table splitting and referring back to.
    //# We also want to make sure that the ids are in sorted order.
    itsSelectedCorrIds.resize(ids.nelements() );
    itsSelectedPolIds.resize( itsSelectedCorrIds.nelements() );
    for ( uInt corrId=0,polId=0; corrId < ids.nelements(); corrId++ )
    {
   //#if ( ids[corrId].nelements() > 0 )
       itsSelectedPolIds[polId++] = corrId;
       Int size = GenSort<Int>::sort( ids[corrId], Sort::Ascending, Sort::NoDuplicates );
       itsSelectedCorrIds[corrId].resize( size );
       itsSelectedCorrIds[corrId] = ids[corrId];
    }
    


    //# Set the start, end, and step correlation values.
    //cout << "itsMaxPolId=" << itsMaxPolId << endl;
    itsStartCorr.resize( itsMaxPolId );
    itsEndCorr.resize( itsMaxPolId );
    itsStepCorr.resize( itsMaxPolId );
    for( uInt polId=0; polId < ids.nelements(); polId++ )
    {
   if ( ids[polId].nelements() == 1 )
   {
       itsStartCorr[polId].resize(1); 
       itsStartCorr[polId][0] = ids[polId][0];

       itsEndCorr[polId].resize(1);   
       itsEndCorr[polId][0]   = ids[polId][0];
       
       itsStepCorr[polId].resize(1);  
       itsStepCorr[polId][0]  = 1;
   } else if ( ids[polId].nelements() > 1 ) {
       //itsStartCorr[polId].resize( ids[polId].nelements() - 1 );
       //itsEndCorr[polId].resize( ids[polId].nelements() - 1 );
       //itsStepCorr[polId].resize( ids[polId].nelements() - 1 );
       //
       //for ( uint i=0; i < ids[polId].nelements()-1; i++ )
       //{
       //itsStartCorr[polId][i] = ids[polId][i];
       //itsEndCorr[polId][i]   = ids[polId][i+1];
       //itsStepCorr[polId][i] = ids[polId][i+1] - ids[polId][i];
       //}


       //correct, but for 3 pols, 2 different steps
       /*
       //make it big first
       itsStartCorr[polId].resize(ids[polId].nelements());
       itsEndCorr[polId].resize(ids[polId].nelements());
       itsStepCorr[polId].resize(ids[polId].nelements());
       uInt s = 0;
       uInt i = 0;
       uInt len = ids[polId].nelements();
       while (i < len) {
          if (i == len - 1) {
             itsStartCorr[polId][s] = ids[polId][i];
             itsEndCorr[polId][s] = ids[polId][i];
             itsStepCorr[polId][s] = 1;
             i++;
             s++;
          }
          else {
             itsStartCorr[polId][s] = ids[polId][i];
             itsEndCorr[polId][s] = ids[polId][i + 1];
             Int step = ids[polId][i + 1] - ids[polId][i];
             itsStepCorr[polId][s] = step;
             i += 2;
             s++;
          }
       }
       itsStartCorr[polId].resize(s, True);
       itsEndCorr[polId].resize(s, True);
       itsStepCorr[polId].resize(s, True);
       */


       Int iCnt = ids[polId].nelements();
       if (iCnt == 3) {
          itsStartCorr[polId].resize(iCnt);
          itsEndCorr[polId].resize(iCnt);
          itsStepCorr[polId].resize(iCnt);
          for (Int i=0; i < iCnt; i++) {
             itsStartCorr[polId][i] = ids[polId][i];
             itsEndCorr[polId][i] = ids[polId][i];
             itsStepCorr[polId][i] = 1;
          }
       }
       else {
          /*
          itsStartCorr[polId].resize(ids[polId].nelements());
          itsEndCorr[polId].resize(ids[polId].nelements());
          itsStepCorr[polId].resize(ids[polId].nelements());
          Int cCnt = ids[polId].nelements();
          Int rCnt = 0;
          Int i = 0;
          Int k = 1;
          while (i < cCnt) {
            //cout << "i=" << i << endl;
            Int start = ids[polId][i++];
            Int end = start;
            Int step = 1;
            //cout << "i=" << i << " start=" << start << endl;
            while(i < cCnt) {
               //cout << "----i=" << i << " start=" << start << endl;
               if (k++ == 1) {
                  end = ids[polId][i++];
                  step = end - start;
               }
               else {
                  Int del = ids[polId][i++];
                  if (del - end == step) {
                     end = del;
                  }
                  else {
                     //cout << "rCnt=" << rCnt << " start=" << start
                     //      << " end=" << end << " step=" << step << endl;
                     itsStartCorr[polId][rCnt] = start;
                     itsEndCorr[polId][rCnt] = end;
                     itsStepCorr[polId][rCnt] = step;
                     start = del;
                     end = start;
                     step = 1;
                     k = 1;
                     rCnt++;
                  }
               }
               //cout << "----i=" << i << " start=" << start << endl;
            }
            if (k > 0) {
               //cout << "rCnt=" << rCnt << " start=" << start
               //     << " end=" << end << " step=" << step << endl;
               itsStartCorr[polId][rCnt] = start;
               itsEndCorr[polId][rCnt] = end;
               itsStepCorr[polId][rCnt] = step;
               rCnt++;
            }
   
          }
          itsStartCorr[polId].resize(rCnt, True);
          itsEndCorr[polId].resize(rCnt, True);
          itsStepCorr[polId].resize(rCnt, True);
          */

          itsStartCorr[polId].resize(1);
          itsEndCorr[polId].resize(1);
          itsStepCorr[polId].resize(1);
          itsStartCorr[polId][0] = ids[polId][0];
          itsEndCorr[polId][0] = ids[polId][iCnt - 1];
          itsStepCorr[polId][0] = ids[polId][iCnt - 1] - ids[polId][0];
          if (iCnt == 4)
             itsStepCorr[polId][0] = 1;
       }
    }
    }
    //for (uInt polId=0; polId < itsMaxPolId; polId++) {
    //    cout << "polId=" << polId << endl;
    //    cout << "itsStartCorr " << itsStartCorr[polId] << endl;
    //    cout << "itsEndCorr " << itsEndCorr[polId] << endl;
    //    cout << "itsStepCorr " << itsStepCorr[polId] << endl;
    //}

    log->FnExit( fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//#
//# select the time ranges 
//#
//#///////////////////////////////////////////////////////////////////////////
casa::String
MsPlot::getTimeExprStr( const String& times)
{
    String FnCall = String( "( " ) + times + String( " )" );
    String fnname = "getTimeExprStr";
    log->FnEnter( fnname +  FnCall, clname );

    if ( ! checkInit() || ! checkOpenMS() )  { 
       log->FnExit( fnname, clname );
       return False;
    }
    
    //# Initialize the Time selection time string.
    String timeExpr = "";
    Int timeStep = -1;
    
    //# Construct the timeExpr string for MSSelection.
    //# Looping through all of the time ranges given and stripping of
    //# time step (average) values, and keeping the time step value
    //# locally.
    String timeRanges[ times.length() ];
    uInt numRanges = split( times, timeRanges, times.length(), "," );
  
    for( uInt i=0; i < numRanges; i++ )
    {
   //# Look for the step values. Notice that we only want to see
   //# one "^", but we look for "^" in order to check validity.
   String curTimeRange[3];
   uInt numBits = split( timeRanges[i], curTimeRange, 3, "^" );
   if ( numBits > 2 || numBits < 1 ) 
   {
       //# Error bad time range
       timeExpr = String( "" );
       *log << LogIO::SEVERE
            << LogOrigin( "MsPlot", fnname )
          << "Invalid time range given: " << timeRanges[i]
          << "\nExpected format is: HH\\MM\\DD:hh:mm:ss^step"
          << LogIO::EXCEPTION;
   } else {
       //# We have a valid time range specification keep track of it.
       if ( timeExpr.length() > 0 )
      //# We need to add the comma separator in to separate
      //# the time ranges for MSSelection
      timeExpr += String( "," );
       
       timeExpr += curTimeRange[0];
       
       //# Skip over the storing of the time stamp, there isn't
       //# one for this time range.
       if ( numBits > 1 ) 
       {
      //# We have a  timeStep!!!
      Int curTimeStep = atoi( curTimeRange[1].chars() );
      if ( curTimeStep < 0 ) 
      {
          //# Error, we average/step over postive time values.
          timeExpr = String( "" );
          *log << LogIO::SEVERE
               << LogOrigin( "MsPlot", fnname )
             << "Invalid time step: " << curTimeStep 
             << ", step values must be positive real numbers. "
             << LogIO::EXCEPTION;
      } else {
          //# We have a step value, lets keep it.
          if ( timeStep < 0 )
         timeStep = curTimeStep;
          else if ( timeStep != curTimeStep )
          {
         //# Error, we have to different time steps, we don't
         //# handle this yet.
         timeExpr = String( "" );
         String msg = 
             String( "Sorry, currently time steps of " )
             + String( "different values are not supported." );
         log->out( msg, fnname, clname, LogMessage::WARN, True);
          }
      }   
       }
   }
    }
  
    
    //# We now have a timeExpr that should keep MSSelection happy!
    //# We'll return it and if we found a step value, lets hang on
    //# to what it is, to tell TablePlot about it.
    log->FnExit( fnname, clname);
    itsTimeStep = timeStep; 
    return timeExpr;
}

//#////////////////////////////////////////////////////////////////////////////
//# setTimes();
//#
//# OLD -> MSSelection was used to select the time ranges, but that was all.
//#
//# TODO : Eventually allow scan based time selection, with syntax similar
//#        to that used in SPWs and channel selection.
//# TODO : Be more flexible on the time syntax
//#
//#///////////////////////////////////////////////////////////////////////////
casa::String
MsPlot::getTimeExprStr(const String& times, const String& mode)
{
   //mode can be vector, scalar, step
   String FnCall = String("( ") + times + String(", ") 
          + mode + String(" )");
   String fnname = "getTimeExprStr";
   log->FnEnter(fnname + FnCall, clname);

   if (!checkInit() || !checkOpenMS()) { 
      log->FnExit( fnname, clname );
      return False;
   }
    
   //# Initialize the Time selection time string.
   String timeExpr = "";
   Int timeStep = -1;

   //#Timer timer;
   //# TODO
   //# This will look a lot more like setspectral does now when we support
   //# time selection on a per scan basis.

   //
   //# Check that we have a valid value for the average/step indicator.
   //if (!checkAverageMode("timeavemode", mode, times)) {
   //   timeExpr = "";
   //   AipsError ae(String( "Invalid time averaging mode: ") + mode );
   //   throw( ae );
   //}
   // 
   //if (mode.length() > 0 
   //       && !upcase(mode).matches("SCALARCHUNK") 
   //       && !upcase(mode).matches("STEP") 
   //       && !upcase(mode).matches("NONE")) {
   //   String errMsg = String("Invalid value for timeavemode: ")
   //                  + mode + String(". Must be 'step' or 'scalarchunk'");
   //   AipsError ae( errMsg );   
   //   throw(ae);
   //}

   //itsTimeAveMode hold averagemode=[vector, scalar, step]
   //itsTimeAveMode = upcase(mode);

   //# Construct the timeExpr string for MSSelection.
   //# Looping through all of the time ranges given and stripping of
   //# time step (average) values, and keeping the time step value
   //# locally.
   String timeRanges[times.length()];
   uInt numRanges = split(times, timeRanges, times.length(), ",");
  
   for (uInt i=0; i < numRanges; i++) {
      //# Look for the step values. Notice that we only want to see
      //# one "^", but we look for "^" in order to check validity.
      String curTimeRange[3];
      uInt numBits = split( timeRanges[i], curTimeRange, 3, "^" );
      if (numBits > 2 || numBits < 1) {
         //# Error bad time range
         timeExpr = String( "" );
         *log << LogIO::SEVERE << LogOrigin( "MsPlot", fnname )
            << "Invalid time range given: " << timeRanges[i]
            << "\nExpected format is: HH\\MM\\DD:hh:mm:ss^step"
            << LogIO::EXCEPTION;
      } 
      else {
         //# We have a valid time range specification keep track of it.
         if (timeExpr.length() > 0)
            //# We need to add the comma separator in to separate
            //# the time ranges for MSSelection
            timeExpr += String( "," );
          
         timeExpr += curTimeRange[0];
          
         //# Skip over the storing of the time stamp, there isn't
         //# one for this time range.
         if (numBits > 1) {
             //# We have a  timeStep!!!
            Int curTimeStep = atoi( curTimeRange[1].chars() );
            if (curTimeStep < 0) {
               //# Error, we average/step over postive time values.
               timeExpr = String( "" );
               *log << LogIO::SEVERE
                    << LogOrigin( "MsPlot", fnname )
                    << "Invalid time step: " << curTimeStep 
                    << ", step values must be positive real numbers. "
                    << LogIO::EXCEPTION;
            } 
            else {
               //# We have a step value, lets keep it.
               if (timeStep < 0)
                  timeStep = curTimeStep;
               else if (timeStep != curTimeStep) {
                  //# Error, we have to different time steps, we don't
                  //# handle this yet.
                  timeExpr = String( "" );
                  String msg = String( "Sorry, currently time steps of " )
                           + String( "different values are not supported." );
                  log->out( msg, fnname, clname, LogMessage::WARN, True);
               }
            }   
         }
      }
   }
    
   //# We now have a timeExpr that should keep MSSelection happy!
   //# We'll return it and if we found a step value, lets hang on
   //# to what it is, to tell TablePlot about it.
   log->FnExit( fnname, clname);
   itsTimeStep = timeStep; 
   return timeExpr;
}

//#////////////////////////////////////////////////////////////////////////////
//# A routine for saving the plot to an image file.
//# This uses the matplotlib "pl.savefig" routine to do all the work.
//#
//# Note that the supported file formats depends on the backend used with
//# matplotlib.  Currently with the Tk backend only eps, ps, pdf, and png
//# formats are supported.
//# 
Bool
MsPlot::saveFigure( const String& filename, 
                const Int dpi, 
      const String& orientation,
      const String& papertype,
      const String& facecolor,
      const String& edgecolor )
{
    String FnCall = String( "( " ) + filename + String( ", " ) 
   + String::toString( dpi ) + String( ", " ) + orientation 
   + String( ", " ) + papertype + String( " , " ) + facecolor 
   + String( ", " ) + edgecolor + String( " )" );
    String fnname = "saveFigure";
    log->FnEnter( fnname + FnCall, clname );

    casa::Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { log->FnExit( fnname, clname); return rstat=False; }

    try {
   rstat = itsTablePlot->saveFigure( filename, dpi, orientation, 
      papertype, facecolor, edgecolor );
    } catch ( AipsError ae ) {
   rstat = False;
   throw;
    }

    log->FnExit( fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
//# TODO --> try-catch blocks for all TablePlot calls...
//# SDJ: Probably not needed since the state after a mark region fails is
//# still good.
//# 
Bool
MsPlot::markRegion( Int nrows, Int ncols, Int panel, Vector<Double> regionvec )
{
  String FnCall = String( "( " ) + String::toString( nrows )
   + String( ", " ) + String::toString( ncols ) + String( ", " )
        + String::toString( panel ) + String( ", " ) 
   + String( ", regionvec )" );
  String fnname = "markRegion";
  log->FnEnter( fnname + FnCall, clname );
  casa::Bool rstat = True;
  if ( ! checkInit() || ! checkOpenMS() )  {
      log->FnExit( fnname, clname); return rstat=False;
  }

  if(regionvec.nelements() != 4)
  {
      String msg = 
         String( "Please enter a valid region [xmin,xmax,ymin,ymax]: " );
      log->out( msg, fnname, clname, LogMessage::WARN, True);
      rstat = False;
  }
  
  itsTablePlot->markRegions( nrows, ncols, panel, regionvec );

  log->FnExit( fnname, clname);
  return rstat;
}
//#////////////////////////////////////////////////////////////////////////////
//# 
Bool
MsPlot::flagData( Int direction )
{
    String FnCall = String( "( " ) + String::toString( direction )
       + String ( " )" );
    String fnname = "flagData";
    log->FnEnter( fnname + FnCall, clname );
    
    casa::Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { 
   log->FnExit( fnname, clname); return rstat=False; 
    }

    //#TODO : check if anything needs to be done if antennas are flagged.
    //# need to make these flags apply to the main table flags.
    //# --> for now, it only updates flags in the ANTENNA subtable.
    //# --> Leave this as is for now. No-where in CASA are there any
    //#     functions that reconcile flags between subtables and the
    //#     main table.
    //#     This will hopefully be rectified with the "flag" tool's
    //#     flag extension functions.... so maybe wait for that to appear.
    
    if ( rstat ) 
    {
    itsTablePlot->flagData( direction );

    //# TODO : write this flag history in some special format somewhere...
    //# For now, it just prints to the logger.
    //#PtrBlock<Record*> FLAGHIST;
    //#itsTablePlot->updateFlagHistory(FLAGHIST);
    //#itsTablePlot->dumpFlagHistory(FLAGHIST);
  
    //# Clean it up.
    //#for(Int i=0;i<(Int)FLAGHIST.nelements();i++)
    //#    delete FLAGHIST[i];
    //#FLAGHIST.resize(0);
    }
    

  log->FnExit( fnname, clname);
  return rstat;
}

//#///////////////////////////////////////////////////////////////////////////
Bool 
MsPlot::locateData( )
{
    String fnname = "locateData";
    log->FnEnter( fnname, clname );
    
    Bool rstat( True );
    if ( ! checkInit() || ! checkOpenMS() )  { 
       log->FnExit( fnname, clname); 
       return rstat=False; 
    }

    //# We don't send in anything here, we let the callback functions
    //# handle everything.  We don't even need that data that is returned
    //# back to us!
    Vector<String> selectColumns(1);
    selectColumns[0] = "";

    PtrBlock<Record*> data;
    itsTablePlot->locateData( selectColumns, data );
    
    //# Clenup the memory that was allocated -- table plot doesn't know
    //# when we are done with this info so it doesn't do it for us!
    for ( uInt i=0; i < data.nelements(); i++ ) {
       delete data[i];
       data[i] = NULL;
    }
    data.resize(0);

    log->FnExit( fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////
Bool 
MsPlot::checkInit() 
{
    if ( !itsInitialized )
    {
        return initialize();
    }

    return True;
}

//#////////////////////////////////////////////////////////////////////////////
Bool 
MsPlot::checkOpenMS() 
{
    if ( itsMS == NULL || !itsMsIsOpen )
    {
       log->out( "Unable to continue, no Measurement Set open.",
          "checkOpenMS", clname, LogMessage::WARN);
       return False;
    } else {
       return True;   
    }
}

//#////////////////////////////////////////////////////////////////////////////
//# Check the validity of an averaging mode.  Valid modes are:
//# 'none' - No averaging
//# 'step' - Plot every nth point
//# 'scalarstep' - Scalar (incoherent) average of every nth point
//# 'vectorstep' - Vector (coherent) average of every nth point
//# 'scalarchunk' - Scalar (incoherent) average with blocks of n points
//# 'vectorchunk' - Vector (coherent) average with blocks of n points
Bool
MsPlot::checkAverageMode(String optName, String mode, String selectExpr ) 
{
   String FnCall = String( "( " ) + optName + String( ", " ) + mode 
                     + String( ", " ) + selectExpr + String( ", )" );
   String fnname = "checkAverageMode";
   log->FnEnter(fnname + FnCall, clname);
    
   Bool rstat(False);
   if (!checkInit() || !checkOpenMS())  {
      log->FnExit( fnname, clname); 
      return rstat=False; 
   }

   if (mode.matches("")
        || upcase(mode).matches("NONE")
        || upcase(mode).matches("STEP")
        || upcase(mode).matches("SCALARSTEP")
        || upcase(mode).matches("VECTORSTEP")
        || upcase(mode).matches("SCALARCHUNK")
        || upcase(mode).matches("VECTORCHUNK")) {
      rstat = True;
   } 
   else {
      String msg = String( "Invalid averaging mode: " ) +  mode
          + String(". Valid values are:\n\t'none', 'step', 'scalarstep',")
          + String( " 'vectorstep', 'scalarchunk', and 'vectorchunk'.");
      log->out( msg, fnname, clname, LogMessage::SEVERE );
      rstat = False;
   }

   //
   //if (!mode.matches("") && !upcase(mode).matches("NONE") 
   //                      && selectExpr.length() < 1) {
   //   if ((selectExpr.matches("timeavemode") 
   //        && itsPlotOptions.AverageNRows < 2) 
   //       || !selectExpr.matches( "timeavemode" ) ) 
   //      log->out(String("Averaging mode ") +  mode + " has been selected"
   //            + " for " + optName + " but averagenrows has not been"
   //            + " set yet.", fnname, clname, LogMessage::NORMAL2);
   //   rstat = True;
   //}

   log->FnExit( fnname, clname);
   return rstat;
}


//#////////////////////////////////////////////////////////////////////////////
//# Check the validity of an averaging mode.  Valid modes are:
//# 'none' - No averaging
//# 'step' - Plot every nth point
//# 'scalarstep' - Scalar (incoherent) average of every nth point
//# 'vectorstep' - Vector (coherent) average of every nth point
//# 'scalarchunk' - Scalar (incoherent) average with blocks of n points
//# 'vectorchunk' - Vector (coherent) average with blocks of n points
//#
//# New to support splitting of averaging and setdata
Bool
MsPlot::checkAverageMode(String optName, String mode ) 
{
   String FnCall = String( "( " ) + optName + String( ", " ) + mode 
                     + String( ", )" );
   String fnname = "checkAverageMode";
   log->FnEnter(fnname + FnCall, clname);
    
   Bool rstat(False);
   if (!checkInit() || !checkOpenMS())  {
      log->FnExit( fnname, clname); 
      return rstat=False; 
   }

   if (mode.matches("")
        || upcase(mode).matches("NONE")
        || upcase(mode).matches("STEP")
        || upcase(mode).matches("SCALARSTEP")
        || upcase(mode).matches("VECTORSTEP")
        || upcase(mode).matches("SCALARCHUNK")
        || upcase(mode).matches("VECTORCHUNK")) {
      rstat = True;
   } 
   else {
      String msg = String( "Invalid averaging mode: " ) +  mode
          + String(". Valid values are:\n\t'none', 'step', 'scalarstep',")
          + String( " 'vectorstep', 'scalarchunk', and 'vectorchunk'.");
      log->out( msg, fnname, clname, LogMessage::SEVERE );
      rstat = False;
   }

   log->FnExit( fnname, clname);
   return rstat;
}
    

//#////////////////////////////////////////////////////////////////////////////
//#////////////////////////////////////////////////////////////////////////////
//#////////////////////////////////////////////////////////////////////////////

//# Creates a vector of Y TaQL strings.
//# itsStartChan and itsEndChan MUST be one-based at this point.
//# -1 and 0 are both defaults here.
//#
casa::Vector<casa::String>
MsPlot::dataTaQL( const String& column, const String& value, 
                  const uInt spwId, const uInt polId, String& label ) 
{
   String FnCall = String( "(  " ) + column + String( ", " )
                      + value + String( ", " ) + String::toString( spwId )
                      + String( ", " ) + String::toString( polId ) 
                      + String( ", " ) + label + String( " )" );
   String fnname = "dataTaQL";
   log->FnEnter( fnname + FnCall, clname );
    
   Vector<String> retValue(1);
   retValue[0]="";

   //# TODO an assert that start, end, and step are all the
   //# same length.
   Int ntaqls=0;
   Vector<Int> startChanList = itsStartChan[spwId];
   Vector<Int> endChanList   = itsEndChan[spwId];
   Vector<Int> stepChanList  = itsStepChan[spwId];    

   //# TODO an assert that start, end, and step are all the
   //# same length.
   Vector<Int> startCorrList = itsStartCorr[polId];
   Vector<Int> endCorrList = itsEndCorr[polId];
   Vector<Int> stepCorrList = itsStepCorr[polId];
    
   //# Get the number of specified ranges for corr and chan
   Int nChanRanges = startChanList.nelements();
   Int nCorrRanges = startCorrList.nelements();
    
    
   //# Loop over all supplied channel and correlation ranges, and 
   //#     Make y-taql strings for each pair.
   for (Int corrrange=0; corrrange<nCorrRanges; corrrange++ )
      for (Int chanrange=0; chanrange<nChanRanges; chanrange++ ) {
         //# Record channel indices
         Int startchan = startChanList[chanrange];
         Int endchan = endChanList[chanrange];
         Int stepchan = stepChanList[chanrange];
      
         //# Set default limits.
         if (startchan <= 0) 
            startchan=1;
      
         //# If the endchan is not specified (i.e. endchan==0), then try to
         //# set it to the actual endchan. 
         //# Also, if the specified endchan is too big
         //# resize it to the actual max value.
         if (itsSpwIdNChans.nelements() >= spwId && 
             (endchan <= 0 || 
              (endchan > 0 && endchan > itsSpwIdNChans[spwId]))) {
            endchan = itsSpwIdNChans[spwId]; 
         } 
         else {
         //# TODO some error or does this mean given endchan is ok?
         }

         if(stepchan <= 0 && endchan > 0 ) 
            stepchan=1;

         //# Record correlation indices 
         Int startcorr = startCorrList[corrrange];
         Int endcorr = endCorrList[corrrange];
         Int stepcorr = stepCorrList[corrrange];

         //# Use defaults if no value is given.
         uInt numIds = itsCorrelationIds[polId].nelements();
         if (startcorr < 0 ) 
            startcorr=itsCorrelationIds[polId][0];
         if (endcorr < 0 ) 
            endcorr=itsCorrelationIds[polId][numIds-1];
         if (stepcorr <= 0 && endcorr > 0 ) 
            stepcorr=1;

         String ytaql("");
         //# If we have asked for "chunked" averaging we need to 
         //# produce a set of TaQL strings, one for each chunk.
         //# This chunking is done here.
         //# Otherwise we just need one set of TaQL string.

         //# TODO, if the last tidbit is small (less then a half of
         //# the chunk size) add the last bit to the previous chunk.
        //if (stepchan >= 2 && 
        //    (upcase(itsAverage[1]).matches("SCALARCHUNK")
        //     || upcase(itsAverage[1]).matches("VECTORCHUNK") ) ) {
           //# This is only for "width" in channels.
           //# startchan, endchan, stepchan all need to be valid.
           if (startchan<=0) 
              startchan=1;
           //# We *need* a valid "endchan" here.... so since we're 
           //anyway doing this,
           if (itsSpwIdNChans.nelements() > 0 && endchan<=0 ) { 
              endchan = itsSpwIdNChans[spwId]; 
           }

           //# stepchan > 1  by the time control comes here.
           //# Calculate the number of channel chunks.
           Int nytaqls = (Int)(endchan - startchan + 1)/stepchan;
           if ((endchan - startchan + 1) % stepchan != 0 ) 
              nytaqls += 1;

           //# nytaql y-taql strings must be appended to retValue.
           retValue.resize( ntaqls + nytaqls, True );
       
           Int endch;
           Int cnt=0;
           for (Int ch=startchan; ch<=endchan; ch+=stepchan) { 
              endch = ch + stepchan - 1;
             if (endch > endchan ) 
                endch = endchan;
      
             ytaql = getIndices(startcorr, endcorr, stepcorr,
                    ch, endch, 1, column, value, label);
             retValue[ ntaqls + cnt ] = ytaql;
             cnt++;
          }   
          ntaqls = retValue.nelements();
         /* 
       } 
       else {
          //# Do averaging on every nth point, or stepping, or averaging.
          cout << "entering old average branch----" << endl; 
          ytaql = getIndices(startcorr,endcorr,stepcorr, startchan,
                          endchan,stepchan, column, value, label);
          //# Fill in retValue.
          retValue.resize( ntaqls+1, True );
          retValue[ntaqls] = ytaql;
          ntaqls = retValue.nelements();
      }
       */
   }//# end of for corr and chan range...retVa

   log->FnExit( fnname, clname);
   return retValue;
}

//#////////////////////////////////////////////////////////////////////////////
//# Create a Single Y-TaQL string for a data column.
//#////////////////////////////////////////////////////////////////////////////
casa::String 
    MsPlot::getIndices( const Int& startcorr,
       const Int& endcorr,
       const Int& stepcorr,
       const Int& startchan,
       const Int& endchan,
       const Int& stepchan,
       const String& column,
       const String& value,
       String& label )
{
    String FnCall = String( "( " ) + String::toString( startcorr )
   + String( ", " ) + String::toString( endcorr ) + String( ", " )
   + String::toString( stepcorr ) + String( ", " )
   + String::toString( startchan ) + String( ", " )
   + String::toString( endchan ) + String( ", " ) 
   + String::toString( stepchan ) + String(", ") + column + String( ", " )
   + value + String( ", " ) +  label + String( " )" );
    String fnname = "getIndices";
    log->FnEnter( fnname + FnCall, clname );
   //cout << fnname << FnCall << endl; 

    String indexcorr("");
    String indexchan("");
    String index("");
    String columntaql("");
    String valuetaql("");
    String convtaql("");
    String retValue("");
    label = String("");

    Bool showflags = itsPlotOptions.ShowFlags;

    
    //# (1) Create correlation indices as well as figure out how
    //#     many of them we'll be selecting.
    if( startcorr > 0 ) indexcorr += String::toString(startcorr);
    if( endcorr > 0 ) indexcorr += String(":") + String::toString(endcorr);
    if( stepcorr > 0 && endcorr <= 0 ) indexcorr += String(":");
    if( stepcorr > 0 ) indexcorr += String(":") + String::toString(stepcorr);

    Int totalcorr=1;
    if (startcorr > 0 ) {
   if ( endcorr > 0 ) {
       if ( stepcorr > 1 ) {
      totalcorr = floor( ( endcorr / stepcorr ) + 1 - startcorr );
      totalcorr += 1;   //# Add one because the above calculation
                                  //# is not inclusive, ie. doesn't inlcude
                                  //# both start and end value.
       } else {
      totalcorr = endcorr + 1 - startcorr;
       }
   } else {
       cout << "ARG!!! We can't find how big the correlations"
       << "are, no end corr." << endl;
   }
    } else {
   cout << "ARG!!! We can't find how big the correlations"
        << "are, no start corr." << endl;    
    }
    
    //# (2) Create channel indices and find the number of channels to be
    //#     used.
    if( startchan > 0 ) indexchan += String::toString(startchan);
    if( endchan > 0 ) indexchan += String(":") + String::toString(endchan);
    if( stepchan > 0 && endchan <= 0 ) indexchan += String(":");
    if( stepchan > 0 ) indexchan += String(":") + String::toString(stepchan);

    Int totalchan=1;
    if ( startchan > 0 ) {
   if ( endchan > 0 ) {
       if ( stepchan > 1 ) {
      totalchan = floor( endchan / stepchan ) + 1 - startchan;
      totalchan += 1;
       } else {
      totalchan = endchan + 1 - startchan;
       }
   } else {
       cout << "ARG!!! We can't find how big the channel"
       << "selection is, no end channl." << endl;
   }
    } else {
   cout << "ARG!!! We can't find how big the channel"
        << "selection is, no start channel." << endl;    
    }

    //# (3) Combine the correlation and channel indices
    if(indexcorr.length()==0 && indexchan.length()==0) index = String("");
    else index = String("[") + indexcorr + String(",") + indexchan + String("]");

    //#cout << "TaQL Chan indexes: " << indexchan << endl;
    //#cout << "Total chan: " << totalchan << endl;
    //#cout << "TaQL Corr indexes: " << indexcorr << endl;
    //#cout << "Total corr: " << totalcorr << endl;
    
    //# (4) Combine the column name with indices.
    if( (upcase(column)).matches("DATA") )
    {
       columntaql = String("DATA") + index;
       label = String("Observed Data");
    }
    else if( (upcase(column)).matches("CORRECTED") 
   || (upcase(column)).matches("CORRECTED_DATA")
   || (upcase(column)).matches("CORRECTEDDATA") )
    {
       columntaql = String("CORRECTED_DATA") + index;
       label = String("Corrected Data");
    }
    else if( (upcase(column)).matches("MODEL") 
        || (upcase(column)).matches("MODELDATA") 
        || (upcase(column)).matches("MODEL_DATA") ) 
    {
       columntaql = String("MODEL_DATA") + index;
       label = String("Model Data");
    }
    else if( (upcase(column)).matches("RESIDUAL") 
        || (upcase(column)).matches("RESIDUALCORRECTED") 
             || (upcase(column)).matches("RESIDUAL_CORRECTED") )

    {
       columntaql = String("(CORRECTED_DATA") + index 
                  + String("-MODEL_DATA") + index + String(")");
       label = String("Residual with Corrected Data");
    }
    else if( (upcase(column)).matches("RESIDUAL_DATA") 
          || (upcase(column)).matches("RESIDUALDATA") 
          || (upcase(column)).matches("RESIDUAL_OBS") 
          || (upcase(column)).matches("RESIDUAL_OBSERVED") )
    {
       columntaql = String("(DATA") + index 
                  + String("-MODEL_DATA") + index + String(")");
       label = String("Residual with Observed Data");
    }
    else if( (upcase(column)).matches("WEIGHT") 
   || (upcase(column)).matches("WEIGHTS") )
    {
        //# General Syntax needed is
   //#  IIF(FLAG[<pol>,<chan>],1.0,0.0)
   //#          *ARRAY(WEIGHT[<pol>],[<numPol>,<numChan>])
   String flagStr;
   if ( showflags ) 
       flagStr = String( "IIF( FLAG" ) + index + String( ", 1.0, 0.0 ) " );
   else
       flagStr = String( "IIF( FLAG" ) + index + String( ", 0.0, 1.0 ) " );
   columntaql = flagStr 
       + String( " * ARRAY( WEIGHT[" ) + indexcorr + String( "],[" )
       + String::toString( totalcorr ) + String(",")
       + String::toString( totalchan ) + String("] )");
   label = String("Weights");
    }
    else if( (upcase(column)).matches("WEIGHT_SPECTRUM") 
   || (upcase(column)).matches("SPECTRUM_WEIGHTS") 
   || (upcase(column)).matches("WEIGHTSPECTRUM") )
    {
   //# General Syntax needed is
   //#  WEIGHT_SPECTRUM[<pol>,chan]
   columntaql = String( "WEIGHT_SPECTRUM" ) + index;
   label = String("Spectrum Weights");
    }
    else if( (upcase(column)).matches("IMAGING_WEIGHT") 
   || (upcase(column)).matches("IMAGING_WEIGHTS") 
   || (upcase(column)).matches("IMAGE_WEIGHTS") 
   || (upcase(column)).matches("IMAGEWEIGHTS") )
    {
   //# General Syntax needed is
   //#  IIF(FLAG[<pol>,<chan>],1.0,0.0)
   //#          *ARRAY(IMAGING_WEIGHT[<chan>],[<numPol>,<numChan>])
   String flagStr;
   if ( showflags ) 
       flagStr = String( "IIF( FLAG" ) + index + String( ", 1.0, 0.0 ) " );
   else
       flagStr = String( "IIF( FLAG" ) + index + String( ", 0.0, 1.0 ) " );
   columntaql = flagStr
       + String( "* ARRAY( IMAGING_WEIGHT[" ) + indexchan + String( "],[" )
       + String::toString( totalcorr ) + String(",")
       + String::toString( totalchan ) + String("] )");
   label = String("Imaging Weights");
    }
    else if( (upcase(column)).matches("WEIGHTEDDATA")
             || (upcase(column)).matches("WEIGHTED") 
             || (upcase(column)).matches("WEIGHTED_DATA") )
    {
       columntaql = String("IMAGING_WEIGHT[") + indexchan + String("]") + 
               String("*DATA") + index;
       label = String("Weighted Observed Data");
    }
    else if( (upcase(column)).matches("WEIGHTEDCORRECTED")
             || (upcase(column)).matches("WEIGHTED_CORRECTED")
             || (upcase(column)).matches("WEIGHTED_CORRECTED_DATA") )
    {
       columntaql = String("IMAGING_WEIGHT[") + indexchan + String("]") + 
               String("*CORRECTED_DATA") + index;
       label = String("Weighted Corrected Data");
    }
    else if( (upcase(column)).matches("WEIGHTEDMODEL") 
            || (upcase(column)).matches("WEIGHTED_MODEL") )
    {
       columntaql = String("IMAGING_WEIGHT[") + indexchan + String("]") + 
               String("*MODEL_DATA") + index;
       label = String("Weighted Model Data");
    }
    else
    {
   //# Default is to use the DATA column.
   //#*itsLog << LogIO::WARN
   //#<< "Unrecognized data column: "
   //#<< column << ". Using the DATA column for plotting."
   //#<< LogIO::POST;
   columntaql = String("DATA") + index;
   label = String("Observed Data");
    }

        
    //# (5) Get the value taql and any conversion taqls to
    //# be applied.
    if( upcase(value).matches("AMP") 
        || upcase(value).matches("AMPLITUDE") )  
    {
       valuetaql = String("AMPLITUDE"); 
   label = String("Amplitude of ") + label ;
    } else if ( upcase(value).matches("PHA") 
        ||  upcase(value).matches("PHASE") ) {
       valuetaql = String("(180/PI())*PHASE"); 
   //#convtaql = String( "*180.0 / (2.0*asin(1.0))" );
   label = String("Phase of ") + label + String( "  (deg)" );
    } else if( upcase(value).matches("RE") 
      || upcase(value).matches("REAL") ) {
       valuetaql = String("REAL"); 
   label = String("Real part of  ") + label ;
    } else if( upcase(value).matches("IM") 
   || upcase(value).matches("IMAG") 
   || upcase(value).matches("IMAGINARY") ) {
       valuetaql = String("IMAG"); 
   label = String("Imaginary part of ") + label ;
    } else {
   //#*itsLog << LogIO::SEVERE
   //#<< "Invalid value parameter: " << value
   //#<< ". Must be one of: AMP, PHASE, REAL, or IMAG."
   //#<< LogIO::EXCEPTION;
    }


    //#
    //#     Depending on showflags, average the flagged or unflagged data.
    //#
    //# The basic TaQL construction
    //#--------------------------------
    //# In the below pseudo-taql strings the text "data" can be be one of:
    //# DATA, CORRECTED_DATA, MODEL_DATA, and RESIDUAL (corrected - model).
    //# "value" can be on of: 'PHASE', 'AMPLITUDE', 'REAL', AND 'IMAG'. The
    //# channel and correlation selection go within the '[]' in each of the
    //# psuedo TaQLs below.
    //#
    //# 1. No averaging or stepping (plotting every nth point)
    //#      Life is sweet!  TaQL = value(data[])
    //#
    //# 2. Scalar averaging of channels or correlations only
    //#      TaQL = SUM(IIF(FLAG[],value(data[]),0.0) / SUM(IIF(FLAG[],1.0,0.0))
    //# and for flagged data
    //#      TaQL = SUM(IIF(FLAG[],0.0,value(data[]))) / SUM(IIF(FLAG[],0.0,1.0))
    //#
    //# 3. Vector averaging for channels or correlations only
    //#      TaQL = value(SUM(IIF(FLAG[],data[],0.0))) / SUM(IIF(FLAG[],1.0,0.0))
    //# and for flagged data
    //#      TaQL = value(SUM(IIF(FLAG[],0.0,data[]))) / SUM(IIF(FLAG[],0.0,1.0))
    //#
    //# 4. Scalar averaging of both channels and correlations
    //#      TaQL = SUMS(IIF(FLAG[],value(data[]),0.0),2) / SUMS(IIF(FLAG[],1.0,0.0),2)
    //# and for flagged data
    //#      TaQL = SUMS(IIF(FLAG[],0.0,value(data[])),2) / SUMS(IIF(FLAG[],0.0,1.0),2)
    //#
    //# 5. Vector averaging of both channels and correlations
    //#      TaQL = value(SUMS(IIF(FLAG[],data[],0.0),2)) / SUMS(IIF(FLAG[],1.0,0.0),2)
    //# and for flagged data
    //#      TaQL = value(SUMS(IIF(FLAG[],0.0,data[]),2)) / SUMS(IIF(FLAG[],0.0,1.0),2)
    //#
    //# To add to this we also have chunked and stepped, but this is taken
    //# care of before entering getIndices, the start, end, and step values
    //# are adjusted accordingly.

    //# Set up the portion of the TaQL related to flagging.
    //# If plotting unflagged data we want:
    //#    IIF( flagSelection, data, 0.0 )
    //# and for plotting flagged data
    //#    IIF( flagSelection, 0.0, data )
    String dataFlagStr;
    String sumFlagStr;
    if ( showflags ) {
   dataFlagStr = String( "IIF( FLAG" ) + index + String( ", " )
       + columntaql + String( ", ")
       + String( " 0.0 ) " );
   sumFlagStr = String( "IIF( FLAG" ) + index + String( ", " )
       + String( "1.0, 0.0 ) " );
    } else {
   dataFlagStr = String( "IIF( FLAG" ) + index + String( ", " )
       + String( " 0.0, " )
       + columntaql + String( " ) ");
   sumFlagStr = String( "IIF( FLAG" ) + index + String( ", " )
       + String( "0.0, 1.0 ) " );
    }

    //# Record what we are averaging just to make code more
    //# readable later.
    Bool averageChans = False;
    Bool averageCorrs = False;
    if ( upcase( itsAverage[0] ).matches( "CHAN" ) )
   averageChans = True;
    if ( upcase( itsAverage[0] ).matches( "CORR" ) )
   averageCorrs = True;
    if ( upcase( itsAverage[0] ).matches( "BOTH" ) )
    {
   averageChans = True;   
   averageCorrs = True;
    }
    
    if(     upcase( itsAverage[1] ).matches("")
        ||  upcase( itsAverage[1] ).matches("NONE") 
        ||  upcase( itsAverage[1] ).matches("STEP")
   || ( upcase(itsAverage[1]).matches("SCALARSTEP") && (!averageChans && !averageCorrs) ) 
   || ( upcase(itsAverage[1]).matches("VECTORSTEP") && (!averageChans && !averageCorrs) ) 
   || ( upcase(itsAverage[1]).matches("SCALARCHUNK") && (!averageChans && !averageCorrs) )
        || ( upcase(itsAverage[1]).matches("VECTORCHUNK") && (!averageChans && !averageCorrs) ) )
    {
   //# Do No averaging. Just plot what they've selected.
   retValue = valuetaql + String("( ") + columntaql + String(")");

   if (  upcase(itsAverage[1]).matches("SCALARSTEP") 
      || upcase( itsAverage[1]).matches("VECTORSTEP" )
      || upcase( itsAverage[1]).matches("SCALARCHUNK" )
      || upcase( itsAverage[1]).matches("VECTORCHUNK" ) )
      log->out(String("Averaging mode has been selected, but SPW selection")
         + " did not include\nany averaging information."
         + " Continuing with NO averaging.", 
         fnname, clname, LogMessage::WARN);
    } 
    else if( upcase(itsAverage[1]).matches("SCALARSTEP")
             ||  upcase(itsAverage[1]).matches("SCALARCHUNK") )
    {
   if ( averageChans && averageCorrs )
       //# Scalar ave of both channels and corrs
       retValue = String( "SUM( " ) 
      + valuetaql + String( "( " )
      + dataFlagStr + String( ") ) / " )
      + String( "SUM( " ) + sumFlagStr + String( " )" );
   else if ( averageChans )
       //# Scalar averaging of channels only
       retValue = String( "SUMS( " ) 
      + valuetaql + String( "( " )
           + dataFlagStr + String( "), 2 ) / " )
      + String( "SUMS( " ) + sumFlagStr + String( ",2 )" );
   else if ( averageCorrs )
       //# Scalar averaging of correlations only
       retValue = String( "SUMS( " ) 
      + valuetaql + String( "( " )
           + dataFlagStr + String( "), 1 ) / " )
      + String( "SUMS( " ) + sumFlagStr + String( ",1 )" );
   else {
       //# ???
   }
    } else if( upcase(itsAverage[1]).matches( "VECTORSTEP" ) 
            || upcase(itsAverage[1]).matches( "VECTORCHUNK" ) )
    {    
       if ( averageChans && averageCorrs )
       //# vector average of both channels and corrs
       retValue = valuetaql + String( "( ( " ) 
           + String( "SUM( " ) + dataFlagStr + String( " ) ) / " )
      + String( "SUM( " ) + sumFlagStr + String( " ) )" );
   else if ( averageChans )
       //# vector average of channels only
       retValue = valuetaql + String( "( ( " ) 
      + String( "SUMS( " ) + dataFlagStr + String( ", 2 ) ) / " )
      + String( "SUMS( " ) + sumFlagStr + String( ", 2  ) )" );
   else if ( averageCorrs )
       //# vector average of correlations only
       retValue = valuetaql + String( "( (" ) 
                + String( "SUMS( " ) + dataFlagStr + String( ", 1 ) ) / " )
      + String( "SUMS( " ) + sumFlagStr + String( ", 1 ) )" );
   else {
       //# ???
   }
    } else {
       ostringstream os;
       os << "Internal error, encountered an unexpected averaging value: "
          << itsAverage[1]
          << ". with channel selection (" 
          << startchan << "~" << endchan << "^" << stepchan
          << ") and correlation selection ("
          << startcorr << "~" << endcorr << "^" << stepcorr << ").";
       log->out(os, fnname, clname, LogMessage::SEVERE);
   }

    //#cout << startchan << "-" << endchan << "-" << stepchan << endl;
    //#cout << startcorr << "-" << endcorr << "-" << stepcorr << endl;
    //#cout << "chans : " << indexchan << endl;
    //#cout << "corrs : " << indexcorr << endl;
    //#cout << "index : " << index << endl;
    //#cout << "conv TaQL: " << convtaql << endl;
    
    
    //# Add the conervsion taql to whatever we've constructed so far.
    if ( convtaql.length() > 0 )
   retValue = retValue + convtaql;
    

    log->FnExit( fnname, clname);
    return retValue;
}
//#///////////////////////////////////////////////////////////////////////

//# For special cases, update the Plot Options.
casa::Bool
MsPlot::updatePlotOptions( const String& title, 
                           const String& xlabel,
            const String& ylabel,
            const String& xcolumn,
            const String& ycolumn,
            const String& x,
            const String& y)
{
   String FnCall = String("(")  + title
           + String(", ") + xlabel + String(", ") + ylabel + String(", ")
           + ycolumn + String(", ") + x + String(", ") + y + String("  )");
   String fnname = "updatePlotOptions";

   log->FnEnter( fnname + FnCall, clname );
   casa::Bool retValue = True;

   //# (1) If the user has not set a title, then apply the defaults.
   if (itsPlotOptions.Title.length()==0) 
      itsPlotOptions.Title = title;
   if (itsPlotOptions.XLabel.length()==0) 
      itsPlotOptions.XLabel = xlabel;
   if (itsPlotOptions.YLabel.length()==0) 
      itsPlotOptions.YLabel = ylabel;

   //# (2) Set up the Antenna labels in itsPlotOptions.PointLabels also
   //#     turn off flagging, in order to ensure we turn it on for non-array
   //#     plots we set it to the default values.
   itsTablePlot->useFlagColumns( "FLAG", "FLAG_ROW" );    
   if (!upcase(xcolumn).compare("ARRAY") || !upcase(ycolumn).compare("ARRAY")) {
      //# Turn off flagging, quick and dirty way is to make the 
      //# Flag columns empty strings.
      itsTablePlot->useFlagColumns( "", "" );

      ROScalarColumn<String> rot( itsSelectedMS.antenna(), "NAME" );
      Vector<String> antnames = rot.getColumn();
      if (antnames.nelements() != itsSelectedMS.antenna().nrow()) {
         //# Warn, but go ahead with the plot.
         log->out("Unable to set antenna names as labels",
                    fnname, clname, LogMessage::WARN );
      }
      else 
         itsPlotOptions.PointLabels = antnames;    
   
      //# TODO Maaaaaybe : check if only colour is set. if so, then append "o".
      //# to itsPlotOptions.PlotSymbol for the symbol-type. 
   }

   //# (3) Set timeplot option if 'x' or 'y' is 'time'
   //#     This parameter is not visible to the user right now.
   //#     We also need to use the timeaveraging value if the user
   //#     has set it and we are doing a time plot
   if (downcase(x).contains("time") && !downcase(y).contains("time")) 
      itsPlotOptions.TimePlotChar = String("x");
   if (downcase(y).contains("time") && !downcase(x).contains("time")) 
      itsPlotOptions.TimePlotChar = String("y");
   if (downcase(x).contains("time") && downcase(y).contains("time"))  
      itsPlotOptions.TimePlotChar = String("b");
    
   if (downcase(x).matches("time") ||  downcase(y).matches("time") 
       || downcase(x).contains("data") || downcase(y).contains("data")) {
      if (itsTimeStep > 1 &&
          (itsAveMode.length() < 1 || 
           upcase(itsAveMode).compare(String("STEP")))) 
         itsPlotOptions.SkipNRows = itsTimeStep;
   }

   //# (5)If we are plotting correlations or channels we need to indicate It.
   if (upcase(x).matches( "CORRELATION" ) 
        || upcase(x).matches( "CORRELATIONS" ) 
        || upcase(x).matches( "CORR" ) 
        || upcase(y).matches( "CORRELATION" ) 
        || upcase(y).matches( "CORRELATIONS" ) 
        || upcase(y).matches( "CORR" ) ) {
       itsPlotOptions.ColumnsXaxis = False;
   }

   if (upcase(x).matches( "CHAN" ) 
           || upcase(x).matches( "CHANNEL" ) 
           || upcase(x).matches( "CHANNELS" ) 
           || upcase(y).matches( "CHAN" ) 
           || upcase(y).matches( "CHANNEL" ) 
           || upcase(y).matches( "CHANNELS" )) {
       itsPlotOptions.ColumnsXaxis = True;
   }

   //# (6) We never want multi table plots to be multi-coloured
   //#     we want them to look as if they are a singl plot, so
   //#     we turn off this feature.  It might be a cool feature
   //#     that some may use, but its not available at this point.
   itsPlotOptions.TableMultiColour = False;

   //# (7) Setup the "locate" function parameters.
   //# TODO - for the special case of ARRAY plots, change this.
   if (!upcase(xcolumn).compare("ARRAY") || !upcase(ycolumn).compare("ARRAY")) {
      itsPlotOptions.CallBackHooks = 
         new MSPlotAntennaCallBack(itsAntennaNames, itsFieldNames);
    } 
    else if (itsIsUVWPlot) {
       itsPlotOptions.CallBackHooks =
           new MSPlotUVWCallBack(&itsSelectedMS, itsAntennaNames, 
                  itsFieldNames, itsCorrelationNames, itsRefFrequencies );
    } 
    else {
       itsPlotOptions.CallBackHooks =
            new MSPlotMainMSCallBack( &itsSelectedMS, itsAntennaNames,
          //itsBaselines,
          itsFieldNames, itsCorrelationNames, itsRefFrequencies, 
          itsMS->tableName(),
          itsExtendChan, itsExtendCorr, itsExtendSpw,
          itsExtendAnt, itsExtendTime, itsAveBl, itsAveTime, &itsAveragedMS);
    }

    //cout << "chanList=" << chanList << endl;
    Vector<Int> spwids(itsSelectedSpwIds.nelements());
    uInt k = 0;
    for (uInt i = 0; i < itsSelectedSpwIds.nelements(); i++) {
       uInt j = 0;
       for (j = 0; j < k; j++) {
          if (itsSelectedSpwIds[i] == spwids(j)) {
             break;
          }
       }
       if (j == k) {
          spwids(j) = itsSelectedSpwIds(i);
          k++;
       }
    }
    spwids.resize(k, True);
 
    ROMSDataDescColumns dataDescColumn(itsMS->dataDescription());
    Vector<Int> spwIds = dataDescColumn.spectralWindowId().getColumn();
    //cout << "spwIds=" << spwIds << endl;

    if (spwids.nelements() == 0) {
       //spwids.resize(spwIds.nelements();
       spwids = spwIds;
    } 

    //cout << "spwids=" << spwids << endl;
    //actually, spwExpr is the data_desc_id expr
    String spwExpr = "";
    for (uInt k = 0; k < spwids.nelements(); k++) {
       if (k > 0)
          spwExpr += String(","); 
       for (uInt j = 0; j < spwIds.nelements(); j++) {
          if (spwIds[j] == spwids[k]) {
            spwExpr += String::toString(j);
          }
       }
    }
    //cout << "spwExpr=" << spwExpr << endl;

    if (spwids.nelements() > 1) {
       itsPlotOptions.ReplaceTopPlot = False;
    }
    //cout << "options=" << itsPlotOptions.print() << endl; 

    if (itsAveMode.length() > 0 && 
        ( itsAveChan > 1 ||  itsAveTime > itsMinTimeInterval)) {
       itsPlotOptions.doAverage = True;
       itsPlotOptions.RowMap = aveRowMap;
       itsPlotOptions.ChanMap = aveChanMap;
       itsPlotOptions.MSName = itsMS->tableName();
       itsPlotOptions.spwExpr = spwExpr; 
    }
    else {
       itsPlotOptions.doAverage = False;
       itsPlotOptions.RowMap.resize(0, 0);
       itsPlotOptions.ChanMap.resize(0, 0);
       itsPlotOptions.MSName = "";
       itsPlotOptions.spwExpr = "";
       //itsPlotOptions.useLayerColor = False;
    }
    
    log->FnExit( fnname, clname);
    return retValue;
    //#return retValue;
}
//#////////////////////////////////////////////////////////////////////////////
//#/////////////////  Flag Version Control - start ////////////////////////////
//#////////////////////////////////////////////////////////////////////////////
casa::Bool
MsPlot::saveFlagVersion( String versionname, 
                         String comment, String merge) 
{
    String FnCall = String( "( " ) +  versionname
   + String( ", " ) + comment + String( ", " ) + merge + String( " )");
    String fnname = "saveFlagVersion";
    log->FnEnter( fnname + FnCall, clname );
    casa::Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { log->FnExit( fnname, clname); return rstat=False; }

    if( ! itsTablePlot->saveFlagVersion( versionname, comment, merge ) )
    {
       log->out( String( "Unable to save flag version" ), 
          fnname, clname, LogMessage::WARN );
       rstat = False;
    }
    

    log->FnExit( fnname, clname);
    return rstat;
}
//#////////////////////////////////////////////////////////////////////////////
casa::Bool
MsPlot::restoreFlagVersion(Vector<String> versionname, 
            String merge) 
{
    String FnCall = String( "(  'versionname', " )
         +  merge + String( " )" );
    String fnname = "restoreFlagVersion";
    log->FnEnter( fnname + FnCall, clname );

    casa::Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { 
   log->FnExit( fnname, clname); return rstat=False; 
    }

    if( ! itsTablePlot->restoreFlagVersion( versionname, merge ) )
    {
       log->out( "Unable to restore flag version", 
          fnname, clname, LogMessage::WARN );
       rstat = False;
    }
    

    log->FnExit( fnname, clname);
    return rstat;
}
//#////////////////////////////////////////////////////////////////////////////
casa::Bool
MsPlot::deleteFlagVersion(Vector<String> versionname )
                         {
    String FnCall = String( "( versionname )" );
    String fnname = "deleteFlagVersion";
    log->FnEnter( fnname + FnCall, clname );

    casa::Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  {
    log->FnExit( fnname, clname); return rstat=False;
    }

    if( ! itsTablePlot->deleteFlagVersion( versionname ) )
    {
       log->out( "Unable to delete flag version", 
             fnname, clname, LogMessage::WARN );
       rstat = False;
    }
    

    log->FnExit( fnname, clname);
    return rstat;
}
//#////////////////////////////////////////////////////////////////////////////
casa::Bool
MsPlot::getFlagVersionList() 
{
    String fnname = "getFlagVersionList";
    log->FnEnter(fnname, clname);
    casa::Bool rstat = True;
    if ( ! checkInit() || ! checkOpenMS() )  { 
       log->FnExit( fnname, clname); 
       return rstat=False; 
    }
    Vector<String> versionlist(0);

    itsTablePlot->getFlagVersionList(versionlist);
    
    if( versionlist.nelements() == 0 )
    {
       log->out( "Unable to read flag version list", 
                fnname, clname, LogMessage::WARN );
       rstat = False;
    }
    else
    {   
       String msg = "Flag Version are:";
       for(Int i=0;i<(Int)versionlist.nelements(); i++)
          msg += String( "\t" ) + versionlist[i] + String( "\n" );
          log->out( msg, fnname, clname, LogMessage::NORMAL );
    }

    log->FnExit(fnname, clname);
    return rstat;
}

//#////////////////////////////////////////////////////////////////////////////

//#////////////////////////////////////////////////////////////////////////////
//#/////////////////  Flag Version Control - end ////////////////////////////
//#////////////////////////////////////////////////////////////////////////////


//#////////////////////////////////////////////////////////////////////////////
} //# NAMESPACE CASA - END
//# end of file
