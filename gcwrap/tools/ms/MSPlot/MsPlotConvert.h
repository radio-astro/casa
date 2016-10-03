//# MsPlotConvert.h: Defines the convert classes used by MsPlot.  These
//#                  classes are called by TablePlot when the data points
//#                  being plotted need to be changed -- good for supporting
//#                  different units
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
//# $Id:$
//#
//# -------------------------------------------------------------------------
//# Change Log
//# Date   Name       Description
//# 11/14/2007  S. Jaeger   Fixed up the message handling.


#if !defined CASA_MSPLOT_CONVERT_H
#define CASA_MSPLOT_CONVERT_H


//# General CASA includes
#include <casa/BasicSL/String.h>

//# Measurement and table related includes
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MSOper/MSDerivedValues.h>

#include <measures/Measures/MeasTable.h>

#include <flagging/Flagging/SLog.h>

//# How frequently we display debugging message, we've set it at every
//# 500 000 points.
#define CASA_MPC_DBG_FREQ 50000


//#!//////////////////////////////////////////////////////////////////////////
//#! 
//#!       CONVERSION CLASSES - All classes used with TablePlot to convert 
//#!                            point values when plotting.
//#! 
//#!//////////////////////////////////////////////////////////////////////////

// Some method parameters are only used when LOG0 is set to one which means that
// normally they generate compile warnings.  This macro will comment outthe enclosed
// parameter when LOG0 is not one and will leave it alone when LOG0 == 1. (jjacobs)

#if LOG0
#    define IfLOG0(x)\
    x
#else
#    define IfLOG0(x)\
    /*x*/
#endif

namespace casa { //# NAMESPACE CASA - BEGIN


///////////////////////////////////////////////////////////////////////////////
//#! All the wonderful docs, that will show up in the user reference
//#! manual for this class.
    
//#!//////////////////////////////////////////////////////////////////////////
//#! All the wonderful docs, that will show up in the user reference
//#! manual for this class.
//#!    
// <summary>
// Class used by TablePlot. The Xconvert and Yconvert methods are called
// for each data point that is plotted.  This class is used when plotting
// the "azimuth" plot of a measurement set.
// </summary>
//#!
//#! <use visibility=local>   or   <use visibility=export>
// <use visibility=local>    
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
//#! for example:
//#!  <reviewed reviewer="pshannon@nrao.edu" date="1994/10/10" tests="tMyClass, t1MyClass" demos="dMyClass, d1MyClass">
//#!  </reviewed>
// </reviewed>
//#!
//#! Classes or concepts you should understand before using this class.
// <prerequisite>
// <ul>    
//   <li> <linkto class="MSPlot">MSPlot</linkto>class,     
//   <li> <linkto class="TablePlot">TablePlot</linkto>class,
//   <li> What an azimuth plot is plotting,
//   <li> <linkto class="casacore::MeasurementSet:description">Measurement Set
//        </linkto> class,
//#!    
//   <li> <linkto class="casacore::Table">casacore::Table</linkto> class,
//       in particular TaQL expressions
//   in with the table code.
// </ul>    
// </prerequisite>
//#!        
// <etymology>
// The conversion class that takes points from a Measuremen Set's TIME
// column and converts them to azimuth angless.
// </etymology>
//#!
// <synopsis>
// </synopsis>
//#!
//<example>    
//<example>
//#!
// <motivation>
// Provide a plot of azimuth values to the user doing the conversion
// for them; the conversion could be done with the facilities provided
// in Python as well as the <linkto class="casacore::Table">casacore::Table</linkto> class.    
// </motivation>
//#!
// <thrown>
//    <li> AIPSError
//    <li> TableError
// </thrown>
//#!
// <todo asof="2007/12/12">
//   <li> Make it more efficient by having the
//        <linkto class="TablePlot">TablePlot</linkto> class send
//        a full row of data rather then a single point. This will reduce
//        the number of calculations substantially and speed things up!   
//   </li>
// </todo>
//#!    
//#! End of documentation.    
//#!///////////////////////////////////////////////////////////////////////////
    
class MSPlotConvertAzimuth : public TPConvertBase
{
    public: 
   // Constructor
   MSPlotConvertAzimuth( casacore::MSDerivedValues * derived,
               casacore::ROMSColumns * columns,
               casacore::Bool onXaxis )
        {
            log = SLog::slog();
       // For some reason message loggin, even when its not being
       // displayed really slows things -- the Logger needs to do
       // things a little faster.
       numPlotted = 0;
       casacore::String fnname =  "MSPlotConvertAzimuth";
       log->FnEnter(fnname, clname);
       
       // Determine wether the converted value is along the x or
       // y axis.
       convertOnX = onXaxis;

       // Store the data bits we need.  We could get away with
       // keeping less info around.  It's the old speed versus
       // space trade of.
       itsMsDerived = derived;
       itsFieldIds = columns->fieldId().getColumn();
       itsColumns = columns;
       // We keep track of the lastFieldId seen by these class.
       // This saves us some calculation time, if the current one
       // is the same as the last we use the stored value rather
       // then recalculate.
       lastFieldId = -1;

       log->FnExit(fnname, clname);
   };
   
   // Destructor
   ~MSPlotConvertAzimuth() {
       casacore::String fnname = "~MSPlotConvertAzimuth";
       log->FnEnter(fnname, clname);

       log->FnExit(fnname, clname);
   };
   
   // casacore::Conversion along the Xaxis
   casacore::Double Xconvert( casacore::Double x, casacore::Int row, casacore::Int tblNum )
   {
       casacore::String fnname = "Xconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
	   log->FnEnter(fnname + "(x, row, tblNum)", clname);
	   log->out(casacore::String("Value: ") + casacore::String::toString(x)
	      + " Row= " + casacore::String::toString(row)
	      + " casacore::Table Counter=" + casacore::String::toString(tblNum),
	      fnname, clname, casacore::LogMessage::DEBUG1); 
       }
       
       if ( convertOnX ) {
	   numPlotted++;
	   if ( row >= 0 && itsFieldIds.nelements() > (casacore::uInt)row )
	   {
	       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
		   log->FnExit(fnname, clname);
	       return findValue( x, row );
	   } else {
	       casacore::String err =  casacore::String( "Internal Error: Unable to convert" )
		   + casacore::String( "azimuth value, row value was: " )
		   + casacore::String::toString( row )
		   + casacore::String( ". Row values need to be between 0 and "  )
		   + casacore::String::toString( itsFieldIds.nelements() );
	       log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
	       // Should never get here, and exception occurs in logMessage()
	       return x;   
	   }
       } else {
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	       log->FnExit(fnname, clname);
	   return x;
       }
   };
   

   // casacore::Conversion along the Yaxis
   casacore::Double Yconvert( casacore::Double y, casacore::Int row, casacore::Int tblNum )
   {
       casacore::String fnname = "Yconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
	   log->FnEnter(fnname + "(y, row, tblNum)", clname);
	   log->out(casacore::String("Value: ") + casacore::String::toString(y)
		   + " Row= " + casacore::String::toString(row)
		   + " casacore::Table Counter=" + casacore::String::toString(tblNum),
		   fnname, clname, casacore::LogMessage::DEBUG1); 
       }	
       
       if ( !convertOnX ) {
	   numPlotted++;
	   if ( row >= 0 && itsFieldIds.nelements() > (casacore::uInt)row )
	   {
	       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
		   log->FnExit(fnname, clname);
	       return findValue( y, row );
	   } else {
	       casacore::String err =  casacore::String( "Internal Error: Unable to convert" )
		   + casacore::String( "azimuth value, row value was: " )
		   + casacore::String::toString( row )
		   + casacore::String( ". Row values need to be between 0 and "  )
		   + casacore::String::toString( itsFieldIds.nelements() );
	       log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
	       // Should never get here, and exception occurs in logMessage()
	       return y;
	   }
       } else {
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	       log->FnExit(fnname, clname);
	   return y;
       }
   };
  
   casacore::Double findValue( casacore::Double value, casacore::Int row )
   {
       casacore::String fnname = "findValue";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	   log->FnEnter(fnname, clname);
       
       curFieldId = itsFieldIds( row );    
       if ( curFieldId < 0 ||
	       (casacore::uInt)curFieldId >= itsColumns->field().numPoly().getColumn().nelements() )
       {
	   casacore::String err = casacore::String( "Internal Error: Current field ID, " )
	       + casacore::String::toString( curFieldId )
	       + casacore::String( " is greater then the size of fields polygon column" );
	   log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
       }
       if ( curFieldId != lastFieldId )
       {
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	       log->out( casacore::String( "Field ID: ")+casacore::String::toString(curFieldId));
	   
	   if ( itsColumns->field().numPoly()(curFieldId) == 0 )
	       itsMsDerived->setFieldCenter( itsColumns->field().phaseDirMeas( curFieldId ) );
	   if ( itsColumns->field().numPoly()( curFieldId ) > 0 )
	       itsMsDerived->setFieldCenter( itsColumns->field().phaseDirMeas( curFieldId, value ) );
	    }	
       casacore::Quantity qt( value, "s" );
       casacore::MEpoch mep( qt );
       itsMsDerived->setEpoch( mep );
       
       lastFieldId = curFieldId;
       
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )
	   log->FnExit(fnname, clname);
       return itsMsDerived->azel().getAngle("deg").getValue("deg") (0);
   };
   

 private:
	// Variables
        casacore::Bool convertOnX;
	casacore::MSDerivedValues * itsMsDerived;
        casacore::Int curFieldId, lastFieldId;
        casacore::Vector<casacore::Int> itsFieldIds;
        casacore::ROMSColumns * itsColumns;
	
	// Message logger
	SLog* log;
        static const casacore::String clname;
	// A counter for messaging.  We really don't need to display
	// millions of messages for debugging purposes.  This seems
	// to slow things down a lot if we do anyway -- even if they
	// aren't printed to the logger.
	casacore::uInt numPlotted;
};

const casacore::String MSPlotConvertAzimuth::clname = "MSPlotConvertAzimuth"; 

///////////////////////////////////////////////////////////////////////////////
// Class used by TablePlot. The Xconvert and Yconvert methods are called
// for each data point that is plotted.  This class is used when doing the
// "elevation" plot

class MSPlotConvertElevation : public TPConvertBase
{
    public: 
   // Constructor
   MSPlotConvertElevation( casacore::MSDerivedValues * derived,
               casacore::ROMSColumns * columns,
               casacore::Bool onXaxis )
        {
            log = SLog::slog();
            casacore::String fnname = "MSPlotConvertElevation";
       numPlotted = 0;
       log->FnEnter(fnname + "(derived, columns, onXaxis)", clname);
       
       // Determine wether the converted value is along the x or
       // y axis.
       convertOnX = onXaxis;

       // Store the data bits we need.  We could get away with
       // keeping less info around.  It's the old speed versus
       // space trade of.
       itsMsDerived = derived;
       itsFieldIds = columns->fieldId().getColumn();
       itsColumns = columns;

       // We keep track of the lastFieldId seen by these class.
       // This saves us some calculation time, if the current one
       // is the same as the last we use the stored value rather
       // then recalculate.
       lastFieldId = -1;
       log->FnExit(fnname, clname);
   };
   
   // Destructor
   ~MSPlotConvertElevation() {
            casacore::String fnname = "~MSPlotConvertElevation";
       log->FnEnter(fnname, clname);

       log->FnExit(fnname, clname);
   };
   
   // casacore::Conversion along the Xaxis
   casacore::Double Xconvert( casacore::Double x, casacore::Int row, casacore::Int tblNum )
        {
       //cout << "NUM PLOTTED: " << numPlotted << endl;
       //cout << "mod value is: " << numPlotted % CASA_MPC_DBG_FREQ << endl;
            casacore::String fnname = "Xconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
       //cout << "SENDING LOG MESSAGE" << endl;
      log->FnEnter(fnname + "(x, row, tblNum)", clname );
      log->out(casacore::String("Value: ") + casacore::String::toString(x)
                      + " Row= " + casacore::String::toString(row)
                      + " casacore::Table Counter=" + casacore::String::toString(tblNum),
                      fnname, clname, casacore::LogMessage::DEBUG1); 
       }
       
       if ( convertOnX ) {
      numPlotted++;
      if ( row >= 0 && itsFieldIds.nelements() > (casacore::uInt)row ) 
      {
          if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
         log->FnExit(fnname, clname);
          return findValue( x, row );
      } else {
          casacore::String err =  casacore::String( "Internal Error: Unable to convert" )
         + casacore::String( "elevation value, row value was: " )
         + casacore::String::toString( row )
         + casacore::String( ". Row values need to be between 0 and "  )
         + casacore::String::toString( itsFieldIds.nelements() );
          log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
          // Should never get here, and exception occurs in logMessage()
          return x;   
      }
       } else{
      if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
          log->FnExit(fnname, clname);
      return x;
       }
   };
   

   // casacore::Conversion along the Yaxis
   casacore::Double Yconvert( casacore::Double y, casacore::Int row, casacore::Int tblNum )
        {
       //cout << "NUM PLOTTED: " << numPlotted << endl;
       //cout << "mod value is: " << numPlotted % CASA_MPC_DBG_FREQ << endl;
            casacore::String fnname = "Yconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
      //cout << "SENDING LOG MESSAGE" << endl;
      log->FnEnter(fnname, clname);
      log->out(casacore::String("Value: ") + casacore::String::toString(y)
                      + " Row= " + casacore::String::toString(row)
                      + " casacore::Table Counter=" + casacore::String::toString(tblNum),
                      fnname, clname, casacore::LogMessage::DEBUG1); 
       }
       
       if ( !convertOnX ) {
      numPlotted++;
      if ( row >= 0 && itsFieldIds.nelements() > (casacore::uInt)row )
      {
          if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
         log->FnExit(fnname, clname);
          return findValue( y, row );
      } else {
          casacore::String err =  casacore::String( "Internal Error: Unable to convert" )
         + casacore::String( "elevation value, row value was: " )
         + casacore::String::toString( row )
         + casacore::String( ". Row values need to be between 0 and "  )
         + casacore::String::toString( itsFieldIds.nelements() );
          log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
          // Should never get here, and exception occurs in logMessage()
          return y;
      }
       } else {
      if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
          log->FnExit(fnname, clname);
      return y;
       }
   };

    protected:
        casacore::Double findValue( casacore::Double value, casacore::Int row )
        {
            casacore::String fnname = "findValue";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnEnter(fnname + "(value, row)", clname);

       curFieldId = itsFieldIds( row );
       if ( curFieldId < 0 ||
          (casacore::uInt)curFieldId >= itsColumns->field().numPoly().getColumn().nelements() )
       {
      casacore::String err = casacore::String( "Internal Error: Current field ID, " )
          + casacore::String::toString( curFieldId )
          + casacore::String( " is greater then the size of fields polygon column" );
      log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
         }
         if ( curFieldId != lastFieldId )
         {
        if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
            log->out( casacore::String( "Field ID: ")+casacore::String::toString(curFieldId));
        if ( itsColumns->field().numPoly()(curFieldId) == 0 )
            itsMsDerived->setFieldCenter( itsColumns->field().phaseDirMeas( curFieldId ) );
        if ( itsColumns->field().numPoly()( curFieldId ) > 0 )
            itsMsDerived->setFieldCenter( itsColumns->field().phaseDirMeas( curFieldId, value ) );
         }
         casacore::Quantity qt( value, "s" );
         casacore::MEpoch mep( qt );
         itsMsDerived->setEpoch( mep );
        
         lastFieldId = curFieldId;

         if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
        log->FnExit(fnname, clname);
         return itsMsDerived->azel().getAngle("deg").getValue("deg")(1);
   }

     private:
   // Variables
        casacore::Bool convertOnX;
   casacore::MSDerivedValues * itsMsDerived;
        casacore::Int curFieldId, lastFieldId;
        casacore::Vector<casacore::Int> itsFieldIds;
        casacore::ROMSColumns * itsColumns;

   // Message logger
   SLog* log;
        const static casacore::String clname;
   // A counter for messaging.  We really don't need to display
   // millions of messages for debugging purposes.  This seems
   // to slow things down a lot if we do anyway -- even if they
   // aren't printed to the logger.
   casacore::uInt numPlotted;
};

const casacore::String MSPlotConvertElevation::clname = "MSPlotConvertElevation";

///////////////////////////////////////////////////////////////////////////////
// Class used by TablePlot. The Xconvert and Yconvert methods are called
// for each data point that is plotted.  This class is used when plotting
// the Hourangle plot.

class MSPlotConvertHourangle : public TPConvertBase
{
    public: 
   // Constructor
   MSPlotConvertHourangle( casacore::MSDerivedValues * derived,
            casacore::ROMSColumns * columns,
            casacore::Bool onXaxis )
        {
            log = SLog::slog();
       numPlotted = 0;
       casacore::String fnname = "MSPlotConvertHourangle";
       log->FnEnter(fnname, clname);
       // Determine wether the converted value is along the x or
       // y axis.
       convertOnX = onXaxis;

       // Store the data bits we need.  We could get away with
       // keeping less info around.  It's the old speed versus
       // space trade of.
       itsMsDerived = derived;
       itsColumns = columns;
       
       // Set the antenna info in the casacore::MS Derived
       const casacore::ROMSAntennaColumns & antColumns = itsColumns->antenna();

       // this is the average of all antennas. Use observatory 
       // position instead?
       itsMsDerived->setAntennas( antColumns ); 

       // the following block is needed by parAngle()
       casacore::Int numAnts = antColumns.nrow();
       casacore::Vector<casacore::String> mount( numAnts );
       for (casacore::Int i=0; i < numAnts; i++) {
         mount(i) = antColumns.mount()(i);
       }
       itsMsDerived->setAntennaMount( mount );

       itsFieldIds = columns->fieldId().getColumn();

       lastFieldId = -1;
       log->FnExit(fnname, clname);
   };
   
   // Destructor
   ~MSPlotConvertHourangle() {
            casacore::String fnname = "~MSPlotConvertHourangle";
       log->FnEnter(fnname, clname);

       log->FnExit(fnname, clname);
   };
   
   // casacore::Conversion along the Xaxis
   casacore::Double Xconvert( casacore::Double x, casacore::Int row, casacore::Int tblNum )
        {
            casacore::String fnname = "Xconver";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
      log->FnEnter(fnname + "(x, row, tblNum)", clname);   
      log->out(casacore::String("Value: ") + casacore::String::toString(x)
                      + " Row= " + casacore::String::toString(row)
                      + " casacore::Table Counter=" + casacore::String::toString(tblNum),
                      fnname, clname, casacore::LogMessage::DEBUG1); 
       }
       
       if ( convertOnX ) {
      numPlotted++;
      if ( row >= 0 && itsFieldIds.nelements() > (casacore::uInt)row )
      {
          if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
         log->FnExit(fnname, clname);
          return findValue( x, row );
      } else {
          casacore::String err =  casacore::String( "Internal Error: Unable to convert" )
         + casacore::String( "hourangle value, row value was: " )
         + casacore::String::toString( row )
         + casacore::String( ". Row values need to be between 0 and "  )
         + casacore::String::toString( itsFieldIds.nelements() );
          log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
          // Should never get here, and exception occurs in logMessage()
          return x;   
      }
       } else {
      if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
          log->FnExit(fnname, clname);
      return x;
       }
   };
   

   // casacore::Conversion along the Yaxis
   casacore::Double Yconvert( casacore::Double y, casacore::Int row, casacore::Int tblNum )
        {
            casacore::String fnname = "Yconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
      log->FnEnter(fnname + "(y, row, tblNum)", clname );
      log->out(casacore::String("Value: ") + casacore::String::toString(y)
                      + " Row= " + casacore::String::toString(row)
                      + " casacore::Table Counter=" + casacore::String::toString(tblNum),
                      fnname, clname, casacore::LogMessage::DEBUG1); 
       }
       

       if ( !convertOnX )
      if ( row >= 0 && itsFieldIds.nelements() > (casacore::uInt)row )
      {
          if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
         log->FnExit(fnname, clname);
          return findValue( y, row );
      } else {
          casacore::String err =  casacore::String( "Internal Error: Unable to convert" )
         + casacore::String( "hourangle value, row value was: " )
         + casacore::String::toString( row )
         + casacore::String( ". Row values need to be between 0 and "  )
         + casacore::String::toString( itsFieldIds.nelements() );
          log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
          // Should never get here, and exception occurs in logMessage()
          return y;
      }
       else {
      if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
          log->FnExit(fnname, clname);
      return y;
       }
   };
  

    protected:
        casacore::Double findValue( casacore::Double value, casacore::Int row )
   {
            casacore::String fnname = "findValue";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnEnter(fnname + "(value, row)", clname);
       curFieldId = itsFieldIds( row );
       if ( curFieldId < 0 ||
          (casacore::uInt)curFieldId >= itsColumns->field().numPoly().getColumn().nelements() )
       {
      casacore::String err = casacore::String( "Internal Error: Current field ID, " )
          + casacore::String::toString( curFieldId )
          + casacore::String( " is greater then the size of fields polygon column" );
      log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
       }
       if ( curFieldId != lastFieldId )
       {
      if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
          log->out( casacore::String( "Field ID: ")+casacore::String::toString(curFieldId));
      if ( itsColumns->field().numPoly()(curFieldId) == 0 )
          itsMsDerived->setFieldCenter( itsColumns->field().phaseDirMeas( curFieldId ) );
      if ( itsColumns->field().numPoly()( curFieldId ) > 0 )
          itsMsDerived->setFieldCenter( itsColumns->field().phaseDirMeas( curFieldId, value ) );
       }
       casacore::Quantity qt( value, "s" );
       casacore::MEpoch mep( qt );
       itsMsDerived->setEpoch( mep );
       
       lastFieldId = curFieldId;

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);
       // itsMsDerived->hourAngle()/C::_2pi*C::day;
       return itsMsDerived->hourAngle()*360.0/casacore::C::_2pi;
   }

     private:
   // Variables
        casacore::Bool convertOnX;
   casacore::MSDerivedValues * itsMsDerived;
        casacore::Int curFieldId, lastFieldId;
        casacore::Vector<casacore::Int> itsFieldIds;
        casacore::ROMSColumns * itsColumns;

   // Message logger
   SLog* log;
        const static casacore::String clname;
   // A counter for messaging.  We really don't need to display
   // millions of messages for debugging purposes.  This seems
   // to slow things down a lot if we do anyway -- even if they
   // aren't printed to the logger.
   casacore::uInt numPlotted;
};

const casacore::String MSPlotConvertHourangle::clname = "MSPlotConvertHourangle";
///////////////////////////////////////////////////////////////////////////////
// Class used by TablePlot. The Xconvert and Yconvert methods are called
// for each data point that is plotted.  This class use used with the
// "parallacticangle" plot.
//
// The values are plotted in degrees.

class MSPlotConvertParallacticangle : public TPConvertBase
{
    public: 
   // Constructor
   MSPlotConvertParallacticangle( casacore::MSDerivedValues * derived,
               casacore::ROMSColumns * columns,
               casacore::Bool onXaxis )
        {
            log = SLog::slog();
            casacore::String fnname = "MSPlotConvertParallacticangle";
       numPlotted=0;
       log->FnEnter(fnname + "(derived, columns, onXaxis)", clname );
       
       // Determine wether the converted value is along the x or
       // y axis.
       convertOnX = onXaxis;

       // Store the data bits we need.  We could get away with
       // keeping less info around.  It's the old speed versus
       // space trade of.
       itsMsDerived = derived;
       itsFieldIds = columns->fieldId().getColumn();
       itsColumns = columns;

       // We keep track of the lastFieldId seen by these class.
       // This saves us some calculation time, if the current one
       // is the same as the last we use the stored value rather
       // then recalculate.
       lastFieldId = -1;

       // Set the antenna info in the casacore::MS Derived
       const casacore::ROMSAntennaColumns & antColumns = itsColumns->antenna();

       // this is the average of all antennas. Use observatory 
       // position instead?
       itsMsDerived->setAntennas( antColumns ); 

       // the following block is needed by parAngle()
       casacore::Int numAnts = antColumns.nrow();
       casacore::Vector<casacore::String> mount( numAnts );
       for (casacore::Int i=0; i < numAnts; i++) {
         mount(i) = antColumns.mount()(i);
       }
       itsMsDerived->setAntennaMount( mount );

       log->FnExit(fnname, clname);
   };
   
   // Destructor
   ~MSPlotConvertParallacticangle() {
            casacore::String fnname = "~MSPlotConvertParallacticangle";
       log->FnEnter(fnname, clname);

       log->FnExit(fnname, clname);
   };

   // casacore::Conversion along the Xaxis
   casacore::Double Xconvert( casacore::Double x, casacore::Int row, casacore::Int tblNum )
        {
            casacore::String fnname = "Xconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
      log->FnEnter(fnname + "(x, row, tableNum)", clname );
                log->out( "Value=" + casacore::String::toString(x) +
                          + " Row=" + casacore::String::toString(row)
                + " casacore::Table Counter=" + casacore::String::toString(tblNum),
                 fnname, clname, casacore::LogMessage::DEBUG1); 
       }
       

       if ( convertOnX ) {
      numPlotted++;      
      if ( row >=  0 && itsFieldIds.nelements() > (casacore::uInt)row ) 
      {
          if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
         log->FnExit(fnname, clname);          
          return findValue( x, row );
      } else {
          casacore::String err =  casacore::String( "Internal Error: Unable to convert" )
         + casacore::String( "parallacticangle   value, row value was: " )
         + casacore::String::toString( row )
         + casacore::String( ". Row values need to be between 0 and "  )
         + casacore::String::toString( itsFieldIds.nelements() );
          log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
          // Should never get here, and exception occurs in logMessage()
          return x;   
      }
       } else {
      if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
          log->FnExit(fnname, clname);      
      return x;
       }
   };
   

   // casacore::Conversion along the Yaxis
   casacore::Double Yconvert( casacore::Double y, casacore::Int row, casacore::Int tblNum )
        {
            casacore::String fnname = "Yconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {      
      log->FnEnter(fnname + "(y, row, tblNum)", clname );
                log->out( "Value=" + casacore::String::toString(y) +
                          + " Row=" + casacore::String::toString(row)
                + " casacore::Table Counter=" + casacore::String::toString(tblNum),
                 fnname, clname, casacore::LogMessage::DEBUG1); 
       }
       
       if ( !convertOnX ) {
      numPlotted++;       
      if ( row >= 0 && itsFieldIds.nelements() > (casacore::uInt)row )
      {
          if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )       
         log->FnExit(fnname, clname);          
          return findValue( y, row );
      } else {        
          casacore::String err =  casacore::String( "Internal Error: Unable to convert" )
         + casacore::String( "parallacticangle   value, row value was: " )
         + casacore::String::toString( row )
         + casacore::String( ". Row values need to be between 0 and "  )
         + casacore::String::toString( itsFieldIds.nelements() );
          log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
          // Should never get here, and exception occurs in logMessage()
          return y;          
      }
       } else {
      if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )       
          log->FnExit(fnname, clname);      
      return y;
       }
   };

    protected:
        casacore::Double findValue( casacore::Double value, casacore::Int row )
        {
            casacore::String fnname = "findValue";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )       
      log->FnEnter( "findValue", "findValue( value, row )" );
       if ( itsColumns != NULL )
      curFieldId = itsFieldIds( row );
       if ( curFieldId < 0 ||
          (casacore::uInt)curFieldId >= itsColumns->field().numPoly().getColumn().nelements() )
       {
      casacore::String err = casacore::String( "Internal Error: Current field ID, " )
          + casacore::String::toString( curFieldId )
          + casacore::String( " is greater then the size of fields polygon column, " )
          + casacore::String::toString( itsColumns->field().numPoly().getColumn().nelements() );
      log->out(err, fnname, clname, casacore::LogMessage::SEVERE, true );
       }
         
       if ( curFieldId != lastFieldId )
       {
      if ( itsColumns->field().numPoly()(curFieldId) == 0 )
          itsMsDerived->setFieldCenter( itsColumns->field().phaseDirMeas( curFieldId ) );
      if ( itsColumns->field().numPoly()( curFieldId ) > 0 )
          itsMsDerived->setFieldCenter( itsColumns->field().phaseDirMeas( curFieldId, value ) );
       }
       casacore::Quantity qt( value, "s" );
       casacore::MEpoch mep( qt );
       itsMsDerived->setEpoch( mep );
        
       lastFieldId = curFieldId;

       // itsMsDerived->parAngle()/C::_2pi*C::day;
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);       
       return itsMsDerived->parAngle()*360.0/casacore::C::_2pi;
   }
   

     private:
   // Variables
        casacore::Bool convertOnX;
        casacore::MSDerivedValues * itsMsDerived;
        casacore::Int curFieldId, lastFieldId;
        casacore::Vector<casacore::Int> itsFieldIds;
        casacore::ROMSColumns * itsColumns;

   // Message logger
   SLog* log;
        const static casacore::String clname;
   // A counter for messaging.  We really don't need to display
   // millions of messages for debugging purposes.  This seems
   // to slow things down a lot if we do anyway -- even if they
   // aren't printed to the logger.
   casacore::uInt numPlotted;
};

const casacore::String MSPlotConvertParallacticangle::clname = 
             "MSPlotConvertParallacticangle";
///////////////////////////////////////////////////////////////////////////////
// Class used by TablePlot. The Xconvert and Yconvert methods are called
// for each data point that is plotted.
//
// This class converts values from geocentric to topcentric frame for
// the antenna position (array) plot.  The conversion routine logic
// is based on Gary Li's original implementation of the MsPlot class,
// although it wasn't done in a Covert class then.
//
// The values are plotted in meters, note if we wish to change this
// to handle kilometers then just divide the results by 1000.


class MSPlotConvertArrayPositions : public TPConvertBase
{
    public: 
   // Constructor
   MSPlotConvertArrayPositions( casacore::ROMSColumns * msColumn )
        {
            log = SLog::slog();            
       numPlotted=0;
       casacore::String fnname = "MSPlotConvertArrayPositions";
       log->FnEnter(fnname, clname);
       
       // Calculate the converted X and Y values that will 
       // replace those stored in the table.
       itsMsColumn = msColumn;
       getTopocentricAnts( msColumn );

       log->FnExit(fnname, clname);       
   };
   
   // Destructor
   ~MSPlotConvertArrayPositions() {
            casacore::String fnname = "~MSPlotConvertParallacticangle";
       log->FnEnter(fnname, clname);
       
       delete itsMsColumn; itsMsColumn = NULL;

       log->FnExit(fnname, clname);       
   };
   
   // casacore::Conversion along the Xaxis
   casacore::Double Xconvert( casacore::Double x, casacore::Int row, casacore::Int tblNum )
        {
            casacore::String fnname = "Xconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
      log->FnEnter(fnname + "(x, row, tblNum)", clname );
      log->out(casacore::String("Value: ") + casacore::String::toString(x)
                      + " Row= " + casacore::String::toString(row)
                      + " casacore::Table Counter=" + casacore::String::toString(tblNum),
                      fnname, clname, casacore::LogMessage::DEBUG1); 
       }
              
       if ( (casacore::uInt)row >= itsXValues.nelements() )
       {
      casacore::String err = casacore::String( "Internal Error: Invalid array row value: " )
          + casacore::String::toString( row )
          + casacore::String( ". Expected a value less then ")
          + casacore::String::toString( itsXValues.nelements() );
          log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
          // Should never get here, and exception occurs in logMessage()
          return x;   
       }

       numPlotted++;
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);
       return itsXValues[ row ];
   };


   // casacore::Conversion along the Yaxis
   casacore::Double Yconvert( casacore::Double y, casacore::Int row, casacore::Int tblNum )
        {
            casacore::String fnname = "Yconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
      log->FnEnter(fnname + "(y, row, tblNum)", clname);
      log->out(casacore::String("Value: ") + casacore::String::toString(y)
                      + " Row= " + casacore::String::toString(row)
                      + " casacore::Table Counter=" + casacore::String::toString(tblNum),
                      fnname, clname, casacore::LogMessage::DEBUG1); 
       }
       
       
       if ( (casacore::uInt)row >= itsYValues.nelements() )
       {
      casacore::String err = casacore::String( "Internal Error: Invalid array row value: " )
          + casacore::String::toString( row )
          + casacore::String( ". Expected a value less then ")
          + casacore::String::toString( itsYValues.nelements() );
      log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
      // Should never get here, and exception occurs in logMessage()
      return y;                
       }
       
       numPlotted++;
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);             
       return itsYValues[ row ];
   };
  

    protected:
   casacore::MPosition 
   getObservatory( casacore::ROMSColumns * msColumn )
        {
            casacore::String fnname = "getObservatory";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnEnter(fnname, clname);
       casacore::MPosition retValue;
       
       // Get the observatory details from the telescope information.
       casacore::String telescope = msColumn->observation().telescopeName()(0);
       if ( casacore::MeasTable::Observatory( retValue, telescope ) )
       {
      if ( retValue.type() != casacore::MPosition::ITRF )
      {
          casacore::MPosition::Convert toItrf( retValue, casacore::MPosition::ITRF );
          retValue = toItrf( retValue );
      }
       }  else {
	 casacore::String err = casacore::String( "Error: Unable to plot telescope array" );
	 if (telescope.length()<=0)
	   err += casacore::String( " the name of the telescope is not stored in " )
	     + casacore::String( " the measurement set. " );
	 else
	   err += casacore::String(" - ")+telescope+casacore::String(" unknown to CASA.  Please contact the helpdesk");
      log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true );
      // Should never get here, and exception occurs in logMessage()
      return retValue;          
       }
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);      
            return retValue;
   }
   
   casacore::Vector<casacore::MPosition>
   getAntennas( casacore::ROMSColumns * msColumn )
   {
            casacore::String fnname = "getAntennas";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnEnter(fnname, clname);
       
       casacore::Vector<casacore::MPosition> retValue( msColumn->antenna().nrow() );
       for( casacore::uInt i=0; i < retValue.nelements(); i++ )
      //retValue[i] = antPositions(i);   
      retValue[i] = msColumn->antenna().positionMeas()(i);   

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);             
       return retValue;
   }
       
   void
   getTopocentricAnts( casacore::ROMSColumns * msColumn )
        {
            casacore::String fnname = "getTopocentricAnts";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnEnter(fnname, clname);
       // Get the observatory information.
       casacore::MPosition observatory = getObservatory( msColumn );

       // Get the anenna information.
       casacore::Vector<casacore::MPosition> antPositions = getAntennas( msColumn );
       
       // Resize the vectors we need to store the values in.
       casacore::uInt numAnts = antPositions.nelements();
       itsXValues.resize( numAnts );
       itsYValues.resize( numAnts );
       
       // Define some storage variables used in the loop
       casacore::Vector<casacore::Double> xTrans(numAnts), yTrans(numAnts), zTrans(numAnts);
       casacore::Vector<casacore::Double> antXYZ(3);
       casacore::Vector<casacore::Double> obsXYZ = observatory.get("m").getValue();

       // Store the cos and sin of both the longitude and
       // latitude of the observatory.  This is needed to 
       // rotate the antenna positions.
       casacore::Vector<casacore::Double> angles    = observatory.getAngle("rad").getValue();
       casacore::Double obsLongitude      = angles[0];
       casacore::Double obsLatitude       = angles[1];
       casacore::Double obsCosLongitude   = cos( obsLongitude );
       casacore::Double obsSinLongitude   = sin( obsLongitude );
       casacore::Double obsCosLatitude    = cos( obsLatitude );
       casacore::Double obsSinLatitude    = sin( obsLatitude );

       //       cout << endl << "observatory: " << observatory << endl << 
       //	 "lonlat = " << obsLongitude*180/3.1415 << " , " << obsLatitude*180/3.1415 << endl;
       //       cout << "obsXYZ = " << obsXYZ << endl;
       
       // Now do the conversion.
       for( casacore::uInt i=0; i < numAnts; i++ )
       {
	 //	 if (i<3)
	 //	   cout << i << ": " << antPositions[i] << " from " <<  (casacore::MPosition::Types)antPositions[i].type() << " to " << observatory.type() << ": ";

      if ( antPositions[i].type() != observatory.type() )
          antPositions[i] = casacore::MPosition::Convert( antPositions[i],
                     (casacore::MPosition::Types)observatory.type() )();
       
      // Translate the global coords to local components
      // without rotating.
      antXYZ = antPositions[i].get("m").getValue();
      xTrans[i] = antXYZ[0] - obsXYZ[0];
      yTrans[i] = antXYZ[1] - obsXYZ[1];
      zTrans[i] = antXYZ[2] - obsXYZ[2];
      
      //      if (i<3) cout << "Trans = " << xTrans[i] << "," << yTrans[i] << "," << zTrans[i] << endl;

      // Now rotate and store the new position information
      itsXValues[i] = ( -obsSinLongitude * xTrans[i] )
          + ( obsCosLongitude * yTrans[i] );
      itsYValues[i] = ( -obsSinLatitude * obsCosLongitude * xTrans[i] )
          - ( obsSinLatitude * obsSinLongitude * yTrans[i] )
          + obsCosLatitude * zTrans[i];
       }

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);      
   }
   
     private:
   // Variables
   casacore::ROMSColumns * itsMsColumn;
        casacore::Bool convertOnX;
   casacore::Vector<casacore::Double> itsXValues;
   casacore::Vector<casacore::Double> itsYValues;

   // Message logger
   SLog* log;
        static const casacore::String clname;

   // A counter for messaging.  We really don't need to display
   // millions of messages for debugging purposes.  This seems
   // to slow things down a lot if we do anyway -- even if they
   // aren't printed to the logger.
   casacore::uInt numPlotted;
};

const casacore::String MSPlotConvertArrayPositions::clname = "MSPlotConvertArrayPositions";

///////////////////////////////////////////////////////////////////////////////
// Class used by TablePlot. The Xconvert and Yconvert methods are called
// for each data point that is plotted.
//
// This class converts channel indices (0-based) to frequency.
//
// Note that if the values coming to the convert functions are non-integer
// this means that we've done some averaging and we need to average the
// frequencies for this value. The values could be integers as well,
// with averaging if choosen correctly
//
// The values are plotted in GHz,

class MSPlotConvertChanToFreq : public TPConvertBase
{
    public: 
   // Constructor
   MSPlotConvertChanToFreq( casacore::Vector<casacore::Int> inIds,
      MsPlot *inMsPlot,
      casacore::Bool inAveOn,
      casacore::Vector< casacore::Vector<casacore::Int> > &inStartChans,
      casacore::Vector< casacore::Vector<casacore::Int> > &inEndChans,
      casacore::Vector< casacore::Vector<casacore::Int> > &inStepChans,
      casacore::Bool onXAxis )
        {
            log = SLog::slog();
       numPlotted=0;
       casacore::String fnname = "MSPlotConvertChanToFreq";
       log->FnEnter(fnname + 
              "(Ids, casacore::MS, average, startChans, endChans, stepChans)", clname);
       // Store the list of Ids for each table locally.
       // The Ids are currently SPWs, but in the future
       // the will be casacore::Data Desc Ids.
       itsSpwIds = inIds;

       // Store the Measurement Set data locally
       itsMsPlot = inMsPlot;

       // Store the channel averaging information.
       itsAveragingOn = inAveOn;
       itsStartChans = inStartChans;
       itsEndChans = inEndChans;
       itsStepChans = inStepChans;
       
       // Store which axis we are converting on.
       convertOnX = onXAxis;
       
       //showConverter();
       log->FnExit(fnname, clname);
   };

   void showConverter() {
       cout << "itsSpwIds=" << itsSpwIds << endl;
       cout << "itsAveragingOn=" << itsAveragingOn << endl;
       for (casacore::uInt i = 0; i < itsStartChans.nelements(); i++) {
          cout << "itsStartChans=" << itsStartChans(i) << endl;
          cout << "itsEndChans=" << itsEndChans(i) << endl;
          cout << "itsStepChans=" << itsStepChans(i) << endl;
       }
       cout << "convertOnX=" << convertOnX;
   }
   
   // Destructor
   ~MSPlotConvertChanToFreq() {
            casacore::String fnname = "~MSPlotConvertChanToFreq";
       log->FnEnter(fnname, clname);

       log->FnExit(fnname, clname);
   };

   // This is a MAJOR HACK to support multiple spws!!!
   // Just a word of warning that this really needs to
   // be cleaned up at some point.
   void setSpwIds( casacore::Vector<casacore::Int> inIds )  { itsSpwIds = inIds; };
   
   
   // casacore::Conversion along the Xaxis
   casacore::Double
   Xconvert_col( casacore::Double x, casacore::Int row, casacore::Int tblnum )
   {
      casacore::String fnname = "Xconvert_col";

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )
          log->FnEnter(fnname + "(x, row, tblnum)", clname );
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {      
	   log->out(casacore::String("Value=") + casacore::String::toString(x) +
		   " Row=" + casacore::String::toString(row) +
		   " casacore::Table Counter=" + casacore::String::toString(tblnum),
                   fnname, clname, casacore::LogMessage::DEBUG1);   
       }
       
       if ( ! convertOnX )
       {
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )
	       log->FnExit(fnname, clname);
	   return x;
       } else {
	   numPlotted++;      
	   // Check table number with our list of SPW Ids and get the
	   // spwId for the table
	   casacore::Int spwId = getSpwId( tblnum );

	   if ( ! itsAveragingOn )
	   {
	       // Look up the frequency from our stored information.
	       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )
		   log->FnExit(fnname, clname);

	       return itsMsPlot->getChanFrequency( spwId, casacore::Int(x) ) / 1.0e9;
	   } else {
	       // If channels are averaged then
	       
	       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )
		   log->FnExit(fnname, clname);
	       casacore::Vector<casacore::Int> chanRange = getChannelRange( x, spwId );
	       return getAveFreq( chanRange, tblnum );
	   }
       }
   };


   // casacore::Conversion along the Yaxis
   casacore::Double
   Yconvert_col( casacore::Double y, casacore::Int row, casacore::Int tblnum )
   {
       casacore::String fnname = "Yconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	   log->FnEnter(fnname, clname);
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
	   log->out(casacore::String("Value=") + casacore::String::toString(y) +
		   " Row=" + casacore::String::toString(row) +
		   " casacore::Table Counter=" + casacore::String::toString(tblnum),
                   fnname, clname, casacore::LogMessage::DEBUG1);   
       }
                
       
       if ( convertOnX )
       {
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	       log->FnExit(fnname, clname);
	   return y;
       } else {
	   numPlotted++;      
	   // Check table number with our list of SPW Ids and get the
	   // spwId for the table
	   casacore::Int spwId = getSpwId( tblnum );
      
	   if ( ! itsAveragingOn )
	   {
	       // Look up the frequency from our stored information.
	       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
		   log->FnExit(fnname, clname);
	       return itsMsPlot->getChanFrequency( spwId, casacore::Int(y) ) / 1.0e9;
	   } else {
	       // If channels are averaged then
	       casacore::Vector<casacore::Int> chanRange = getChannelRange( y, spwId );
	       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
		   log->FnExit(fnname, clname);
	       return getAveFreq( chanRange, tblnum );
	   }
       }
   };
  

    protected:
   // Check if the supplied table ID is smaller then our
   // list of Ids.  If it's larger and exception is thrown.
   casacore::Int
   getSpwId( casacore::Int tblNum )
   {
       casacore::String fnname = "getSpwId";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
                log->FnEnter(fnname, clname);

       casacore::Int retValue = -1;

       // Check that our tblNum is with in range.
       if ( tblNum < 0 || (casacore::uInt)tblNum > itsSpwIds.nelements() )
       {
	   casacore::String err = casacore::String( "Internal Error: Invalid table number: " )
	       + casacore::String::toString( tblNum )
	       + casacore::String( " given to MsPlotConvertChanToFreq.\n" )
	       + casacore::String( " Expected values are from 0 to " )
	       + casacore::String::toString( itsSpwIds.nelements() );
	   log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true);
	   // Should never get here, and exception occurs in logMessage()
	   return retValue;                      
       }
       
             
       // If we are here all is good, and we can simply look up the
       // Spw ID from our Id list.
            // What?-----------------
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	   log->FnExit(fnname, clname);
       retValue = itsSpwIds[tblNum];

       return retValue;
   };

   // If the averaging is on then the channel value passed in
   // is an averaged channel value.  We need to find the channel
   // range that this value came from, ie. the value is between
   // the start and end values.
   //
   // Comment, it might be useful to pre-compute the averaged values
   // in the constructor.  They could be computed if the start, end,
   // and step values are given.  Then we'd just need to search for
   // the matching number.  However, this may not work if we compute
   // the average differently then TablePlot.
   casacore::Vector<casacore::Int>
   getChannelRange( casacore::Double aveChanValue, casacore::Int spwId )
   {
       casacore::String fnname = "getChannelRange";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnEnter(fnname + "(aveChanValue, spwId)", clname);
       
       casacore::Vector<int> retValue;

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->out(casacore::String(" Finding a freqency range for: ")
                         +casacore::String::toString(aveChanValue));

       // Loop through your list of channel start and end values
       // looking for an averaging range that our channel value is
       // between.
       // TODO deal with min/max values.  ie. if there is no
       // start or end value.
       // TODO if stepchan < 2 then we aren't averaging, may want
       // to make this a special case.
       casacore::Int start=-1; casacore::Int end=-1;
       for ( casacore::uInt i=0; i < itsEndChans[spwId].nelements(); i++ )
       {
      if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) { 
         ostringstream os;
         os  << "Checking " << i << " set of channel info: "
         << itsStartChans[spwId][i]
         << "~" << itsEndChans[spwId][i]
         << "^"  << itsStepChans[spwId][i];
                    log->out(os, fnname, clname, casacore::LogMessage::DEBUG1);
                }
      
      for ( casacore::Int chan = itsStartChans[spwId][i];
            chan < itsEndChans[spwId][i];
            chan += itsStepChans[spwId][i] )
      {
          if ( chan < 0 )
         chan = 0;
          if ( chan <= aveChanValue
             && chan+itsStepChans[spwId][i] >= aveChanValue )
          {
         start = chan;
         end = chan + itsStepChans[spwId][i];
         break;
          }
      }
       }
       

       if ( start < 0 && end < 0 )
      start = end = casacore::Int( aveChanValue );

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->out(casacore::String("Averaging range is from ") + 
                      casacore::String::toString(start)
          + casacore::String(" to ") + casacore::String::toString(end), 
                    fnname, clname, casacore::LogMessage::DEBUG1 );
       retValue.resize( end - start + 1 );
       for( casacore::uInt i=0; i < (casacore::uInt)(end-start+1); i++ )
      retValue[i] = start + i;

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);
       return retValue;
   };

   // Get the averaged frequency for the averaged channel
   // value.
   casacore::Double
   getAveFreq( casacore::Vector<casacore::Int> chanRange, casacore::Int tblNum )
   {
            casacore::String fnname = "getAveFreq";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnEnter(fnname + "(chanRange, tblNum)", clname);
       
       casacore::Double sum = 0.0;

       for ( casacore::uInt i=0; i < chanRange.nelements(); i++ )
       {
	   //sum += getFreq( chanRange[i], tblNum );
	   sum += 
	       itsMsPlot->getChanFrequency( tblNum, chanRange[i] ) / 1.0e9;
       }

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
      log->FnExit(fnname, clname);
       return sum / chanRange.nelements();
   };


   // Get the frequency associated with this channel
   // Routine based on demo4 in tTablePlot.cc, written by
   // Urvashi R.V.
   casacore::Double
   getFreq( casacore::Int channel, casacore::Int spwId )
   {
       casacore::String fnname = "getFreq";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	   log->FnEnter(fnname + "(channel, spwId)", clname);
       
       return itsMsPlot->getChanFrequency( spwId, channel ) / 1.0e9;
   };
   
     private:
   // Measurement set the data belongs to.
   MsPlot *itsMsPlot;

   // casacore::List of Ids assoc. with each table given to TablePlot.
   // May not need this,  Its needed if different tables passed
   // in have different channel ranges.  Not that index 0, is
   // the SPW ID of table 0, ...
   casacore::Vector<casacore::Int> itsSpwIds;

   // Averaging information, the list of averaged ranges in
   // two lists.  The list in which each range starts on and
   // the list each range ends on.
   casacore::Bool itsAveragingOn;
   casacore::Vector< casacore::Vector< casacore::Int > > itsStartChans;
   casacore::Vector< casacore::Vector< casacore::Int > > itsStepChans;
   casacore::Vector< casacore::Vector< casacore::Int > > itsEndChans;

   // Indicate if we are averaging on the X or Y axis.
   casacore::Bool convertOnX;

   // Message logger
   SLog* log;
        static const casacore::String clname;
   // A counter for messaging.  We really don't need to display
   // millions of messages for debugging purposes.  This seems
   // to slow things down a lot if we do anyway -- even if they
   // aren't printed to the logger.
   casacore::uInt numPlotted;
};


///////////////////////////////////////////////////////////////////////////////
// Class used by TablePlot. The Xconvert and Yconvert methods are called
// for each data point that is plotted.
//
// This class converts channel indices (0-based) to velocity (km/s).
//
// Note that if the values coming to the convert functions are non-integer
// this means that we've done some averaging and we need to average the
// frequencies for this value. The values could be integers as well,
// with averaging if choosen correctly
//
// The values are plotted in Km/s,

class MSPlotConvertChanToVelocity : public TPConvertBase
{
    public: 
    // Constructor
    MSPlotConvertChanToVelocity( casacore::Vector<casacore::Int> inSpwIds, casacore::ROMSColumns * columns,
	   MsPlot *inMsPlot, casacore::Bool onXAxis )
   {
       log = SLog::slog();
       numPlotted=0;
       casacore::String fnname = "MSPlotConvertChanToVelocity";
       log->FnEnter(fnname + 
              "(Ids, casacore::MS, average, startChans, endChans, stepChans)", clname);

       // Store the list of Ids for each table locally.
       // The Ids are currently SPWs, but in the future
       // the will be casacore::Data Desc Ids.
       itsSpwIds = inSpwIds;


       // Store the list of field Ids for each table row locally.
       // If in the future we have DATA DESC IDs we can likely
       // retrieve this using the measurement set.
       itsFieldIds = columns->fieldId().getColumn();

       // Store the Measurement Set data locally
       itsMsPlot = inMsPlot;

       // Store which axis we are converting on.
       convertOnX = onXAxis;
       
       log->FnExit(fnname, clname);
   };
   
   // Destructor
   ~MSPlotConvertChanToVelocity() {
       casacore::String fnname = "~MSPlotConvertChanToVelocity";
       log->FnEnter(fnname, clname);

       log->FnExit(fnname, clname);
   };

   // This is a MAJOR HACK to support multiple spws!!!
   // Just a word of warning that this really needs to
   // be cleaned up at some point.
   void setSpwIds( casacore::Vector<casacore::Int> inIds )  { itsSpwIds = inIds; };
   
   // casacore::Conversion along the Xaxis
   casacore::Double
   Xconvert_col( casacore::Double x, casacore::Int row, casacore::Int tblnum )
   {
       casacore::String fnname = "Xconvert_col";

       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )
	   log->FnEnter(fnname + "(x, row, tblnum)", clname );
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {      
	   log->out(casacore::String("Value=") + casacore::String::toString(x) +
		   " Row=" + casacore::String::toString(row) +
		   " casacore::Table Counter=" + casacore::String::toString(tblnum),
                   fnname, clname, casacore::LogMessage::DEBUG1);   
       }

       numPlotted++;      
       if ( ! convertOnX )
       {
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )
	       log->FnExit(fnname, clname);
	   return x;
       } else {
	   // Check table number with our list of SPW Ids and get the
	   // spwId for the table
	   casacore::Int spwId = getSpwId( tblnum );

	   // Find the field Number associated with this Row of data.
	   casacore::Int fieldId = getFieldId( row );

	   // Look up the frequency from our stored information.
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 )
	       log->FnExit(fnname, clname);

	   return itsMsPlot->getChanVelocity( spwId, fieldId, casacore::uInt(x) );
       }
   };


   // casacore::Conversion along the Yaxis
   casacore::Double
   Yconvert_col( casacore::Double y, casacore::Int row, casacore::Int tblnum )
   {
       casacore::String fnname = "Yconvert";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	   log->FnEnter(fnname, clname);
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) {
	   log->out(casacore::String("Value=") + casacore::String::toString(y) +
		   " Row=" + casacore::String::toString(row) +
		   " casacore::Table Counter=" + casacore::String::toString(tblnum),
                   fnname, clname, casacore::LogMessage::DEBUG1);   
       }
                
       numPlotted++;      
       if ( convertOnX )
       {
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	       log->FnExit(fnname, clname);
	   return y;
       } else {
	   // Check table number with our list of SPW Ids and get the
	   // spwId for the table
	   casacore::Int spwId = getSpwId( tblnum );

	   // Find the field Number associated with this Row of data.
	   casacore::Int fieldId = getFieldId( row );

	   // Look up the frequency from our stored information.
	   if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	       log->FnExit(fnname, clname);
	   return itsMsPlot->getChanVelocity( spwId, fieldId, casacore::uInt(y) );
       }
   };

 protected:
   // Check if the supplied table ID is smaller then our
   // list of Ids.  If it's larger and exception is thrown.
   casacore::Int
   getSpwId( casacore::Int tblNum )
   {
       casacore::String fnname = "getSpwId";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
                log->FnEnter(fnname, clname);

       casacore::Int retValue = -1;

       // Check that our tblNum is with in range.
       if ( tblNum < 0 || (casacore::uInt)tblNum > itsSpwIds.nelements() )
       {
	   casacore::String err = casacore::String( "Internal Error: Invalid table number: " )
	       + casacore::String::toString( tblNum )
	       + casacore::String( " given to MsPlotConvertChanToVelocity.\n" )
	       + casacore::String( " Expected values are from 0 to " )
	       + casacore::String::toString( itsSpwIds.nelements() );
	   log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true);
	   // Should never get here, and exception occurs in logMessage()
	   return retValue;                      
       }
       
             
       // If we are here all is good, and we can simply look up the
       // Spw ID from our stored list.
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	   log->FnExit(fnname, clname);
       retValue = itsSpwIds[tblNum];

       return retValue;
   };


   // Check if the supplied row number and if valid then get
   // the field ID.
   casacore::Int
   getFieldId( casacore::Int rowNum )
   {
       casacore::String fnname = "getFieldId";
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
                log->FnEnter(fnname, clname);

       casacore::Int retValue = -1;

       // Check that our rowNum is with in range.
       if ( rowNum < 0 || (casacore::uInt)rowNum > itsFieldIds.nelements() )
       {
	   casacore::String err = casacore::String( "Internal Error: Invalid row number: " )
	       + casacore::String::toString( rowNum )
	       + casacore::String( " given to MsPlotConvertChanToVelocity.\n" )
	       + casacore::String( " Expected values are from 0 to " )
	       + casacore::String::toString( itsFieldIds.nelements() );
	   log->out( err, fnname, clname, casacore::LogMessage::SEVERE, true);
	   // Should never get here, and exception occurs in logMessage()
	   return retValue;                      
       }
       
             
       // If we are here all is good, and we can simply look up the
       // Field ID from our stored list.
       if ( numPlotted % CASA_MPC_DBG_FREQ == 0 ) 
	   log->FnExit(fnname, clname);
       retValue = itsFieldIds[rowNum];

       return retValue;
   };

 private:
   // Measurement set the data belongs to.
   MsPlot *itsMsPlot;

   // casacore::List of Ids assoc. with each table given to TablePlot.
   // May not need this,  Its needed if different tables passed
   // in have different channel ranges.  Not that index 0, is
   // the SPW ID of table 0, ...
   casacore::Vector<casacore::Int> itsSpwIds;


   // casacore::List of Field Ids assoc. with each row of data given to TablePlot.
   casacore::Vector<casacore::Int> itsFieldIds;

   // Averaging information, the list of averaged ranges in
   // two lists.  The list in which each range starts on and
   // the list each range ends on.
   casacore::Bool itsAveragingOn;
   casacore::Vector< casacore::Vector< casacore::Int > > itsStartChans;
   casacore::Vector< casacore::Vector< casacore::Int > > itsStepChans;
   casacore::Vector< casacore::Vector< casacore::Int > > itsEndChans;

   // Indicate if we are averaging on the X or Y axis.
   casacore::Bool convertOnX;

   // Message logger
   SLog* log;
   static const casacore::String clname;
   
   // A counter for messaging.  We really don't need to display
   // millions of messages for debugging purposes.  This seems
   // to slow things down a lot if we do anyway -- even if they
   // aren't printed to the logger.
   casacore::uInt numPlotted;
};


const casacore::String MSPlotConvertChanToVelocity::clname = "MSPlotConvertChanToVelocity";
const casacore::String MSPlotConvertChanToFreq::clname = "MSPlotConvertChanToFreq";

////////////////////////
class MSPlotConvertAveChanToFreq : public TPConvertBase
{
   public: 
   //It would be ideal to calculate the chanFreq in the averager
   //and pass the map to here. However, in the setupPlotxy,
   //setConverFn is before setInputTable (because a conversion
   //function change may require a reread of the ms), then
   //we have to reset conversion function after averager
   //
   MSPlotConvertAveChanToFreq(casacore::MeasurementSet &inMS, 
                             casacore::Matrix<casacore::Double> &chanFreq, casacore::Bool onXAxis) {
   
      log = SLog::slog();
      casacore::String fnname = "MSPlotConvertAveChanToFreq";
       
      itsMS = inMS;
      itsChanFreq = chanFreq;

      //cout << "itsChanFreq=" << itsChanFreq << endl;
      convertOnX = onXAxis;
   };

   
   ~MSPlotConvertAveChanToFreq() {
      //casacore::String fnname = "~MSPlotConvertAveChanToFreq";
   };

   // casacore::Conversion along the Xaxis
   casacore::Double Xconvert_col(casacore::Double x, casacore::Int IfLOG0(row), casacore::Int IfLOG0(tableId)) {
      casacore::String fnname = "Xconvert_col";
#if LOG0
      static int a = 0;
      casacore::String fncall = casacore::String("Value=") + casacore::String::toString(x) +
               " Row=" + casacore::String::toString(row) +
               " casacore::Table Counter=" + casacore::String::toString(tableId);
      if (x != 0 && a < 10) {
         log->FnPass(fnname + fncall, clname);
         a++;
      }
#endif
       
      if (!convertOnX) {
          return x;
      } 
      else {
         casacore::Int chn = max(0, casacore::Int(x)); 
         chn = min(chn, casacore::Int(itsChanFreq.nrow() - 1));
         return itsChanFreq(chn, 1);
      }
   };

   // casacore::Conversion along the Yaxis
   casacore::Double Yconvert_col(casacore::Double y, casacore::Int IfLOG0(row), casacore::Int IfLOG0(tableId)) {
      casacore::String fnname = "Yconvert";
#if LOG0
      static int b = 0;
      casacore::String fncall = casacore::String("Value=") + casacore::String::toString(y) +
               " Row=" + casacore::String::toString(row) +
               " casacore::Table Counter=" + casacore::String::toString(tableId);
      if (b < 10 &&  y != 0) {
         log->FnPass(fnname + fncall, clname);
         b++;
      }
#endif
                
      if (convertOnX) {
         return y;
      } 
      else {
         casacore::Int chn = max(0, casacore::Int(y)); 
         chn = min(chn, casacore::Int(itsChanFreq.nrow() - 1));
         return itsChanFreq(chn, 1);
      }
   };
  
   private:

      //not used. the frequency calculation is done in the averager.
      casacore::MeasurementSet itsMS;

      //itsChanFreq is of nAveChan x 2. The first column stores
      //the averaged channel Id (hence is equal to the position index.)
      //The second column store averaged frequencies corresponding to
      //the averaged channel Id. 
      //The first column can be used to store the spwids in the future. 
      casacore::Matrix<casacore::Double> itsChanFreq;

      // Indicate if we are averaging on the X or Y axis.
      casacore::Bool convertOnX;

      SLog* log;
      static const casacore::String clname;
};

const casacore::String MSPlotConvertAveChanToFreq::clname = "MSPlotConverAveChanToFreq";

////////////////////////
class MSPlotConvertAveChanToChan : public TPConvertBase
{
   public: 
   MSPlotConvertAveChanToChan(casacore::MeasurementSet &inMS, 
                             casacore::Matrix<casacore::Int> &chanFreq, casacore::Bool onXAxis) {
   
      log = SLog::slog();
      casacore::String fnname = "MSPlotConvertAveChanToChan";
       
      itsMS = inMS;

      casacore::Int nChan = chanFreq.nrow();
      //cout << "chanFreq=" << chanFreq << endl;
      //cout << "nChan=" << nChan << endl;
      itsChanMap.resize(nChan);
      //cout << "itsChanMap.shape()[0]=" << itsChanMap.shape()[0] << endl;
      for (casacore::Int k = 0; k < nChan; k++) {
         itsChanMap(k) = (chanFreq(k, 1) + chanFreq(k, 2) - 1) / 2;
      }
      //cout << "itsChanMap=" << itsChanMap << endl;

      convertOnX = onXAxis;
   };

   
   ~MSPlotConvertAveChanToChan() {
      //casacore::String fnname = "~MSPlotConvertAveChanToChan";
   };

   // casacore::Conversion along the Xaxis
   casacore::Double Xconvert_col(casacore::Double x, casacore::Int IfLOG0(row), casacore::Int IfLOG0(tableId)) {
      casacore::String fnname = "Xconvert";
#if LOG0
      static int a = 0;
      casacore::String fncall = casacore::String("Value=") + casacore::String::toString(x) +
               " Row=" + casacore::String::toString(row) +
               " casacore::Table Counter=" + casacore::String::toString(tableId);
      if (x != 0 && a < 10) {
         log->FnPass(fnname + fncall, clname);
         a++;
      }
#endif
       
      if (!convertOnX) {
          return x;
      } 
      else {
         casacore::Int chn = max(0, casacore::Int(x)); 
         chn = min(chn, casacore::Int(itsChanMap.shape()[0] - 1));
         cout << " " << itsChanMap(chn);
         return itsChanMap(chn);
      }
   };

   // casacore::Conversion along the Yaxis
   casacore::Double Yconvert_col(casacore::Double y, casacore::Int IfLOG0(row), casacore::Int IfLOG0(tableId)) {
      casacore::String fnname = "Yconvert";
#if LOG0
      static int b = 0;
      casacore::String fncall = casacore::String("Value=") + casacore::String::toString(y) +
               " Row=" + casacore::String::toString(row) +
               " casacore::Table Counter=" + casacore::String::toString(tableId);
      if (b < 10 &&  y != 0) {
         log->FnPass(fnname + fncall, clname);
         b++;
      }
#endif
                
      if (convertOnX) {
         return y;
      } 
      else {
         casacore::Int chn = max(0, casacore::Int(y)); 
         chn = min(chn, casacore::Int(itsChanMap.shape()[0] - 1));
         return itsChanMap(chn);
      }
   };
  
   private:

      //not used. the frequency calculation is done in the averager.
      casacore::MeasurementSet itsMS;

      casacore::Vector<casacore::Int> itsChanMap;

      // Indicate if we are averaging on the X or Y axis.
      casacore::Bool convertOnX;

      SLog* log;
      static const casacore::String clname;
};

const casacore::String MSPlotConvertAveChanToChan::clname = "MSPlotConverAveChanToChan";

};
#endif //CASA_MSPLOT__CONVERT_H

