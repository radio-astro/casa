//# Copyright (C) 2007-2008
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
//# Change Log
//# Date   Name       Description
//# 29/10/2007  S. Jaeger   Added new messaging.

#ifndef CALLBACKHOOKS_H
#define CALLBACKHOOKS_H

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/OS/Time.h>
#include <casa/IO/AipsIO.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/QC.h>

#include <flagging/Flagging/SLog.h>

namespace casa { //# NAMESPACE CASA - BEGIN

#define NSECINYEAR (86400.0)
#define MJDZERO (678575.0+1.0)

#define LOG0 0

/* The maximum possible value that matplotlib can handle for Time Formatting is : 
   3.652e+06 -> 11/02/9999 
   The minumum possible is 1.0 -> 01/01/0001
   
   CASA <29>: pl.num2date(3.652e+06)
   Out[29]: datetime.datetime(9999, 11, 2, 0, 0, tzinfo=<UTC>)
*/


// <summary>
// Base class for computing derived quantities that cannot be
// computed directly via TaQL. 
// </summary>


/* Base Class for conversion functions for derived quantities */
class TPConvertBase
{
   public :
      TPConvertBase(){};
      virtual ~TPConvertBase(){};

      // Notes on these convert functions.
      // 1. For BasePlot only the XConvert and YConvert methods are used
      //    but for CROSS plots the _row and _col methods are used.
      //    Why? BasePlots you all values in a cell. no distinction,
      //    however CROSS plots only plot one of the row or column indexes
      //    on the data itself, so we really only need one Convert method.
      //    Also its important for the caller to know if they have a row
      //    or column index.
      //
      // 2. the _row convert method values are the row indexes for the
      //    data array found in the table cell.  This is typically 
      //    related to the polarization information in the DATA column
      //    Similarily _col convert methodes pass the col indicies through
      //    the value parameter and are typically associated with channels
      //    in DATA columns.
      //
      // 3. Don't forget about ColumnsXaxis Panel parameter it controls
      //    whether row or column indeces are used for plotting in CROSS
      //    plots, this means that only one of the _row and _col methods
      //    will be used/need to be defined and not both.
      //
      // 4. tblRow is the row number where the data resides in the Table
      //    tblNum is the index when a list of Tables was supplied to
      //    TablePlot::setTable(T/S)
      //

      // X-axis convert methods
      virtual inline Double Xconvert(Double x, Int /*tblRow*/, Int /*tblNum*/)
          {return x;};
      
      virtual inline Double Xconvert_row(Double x, Int /*tblRow*/, Int /*tblNum*/)
          {return x;};

      virtual inline Double Xconvert_col(Double x, Int /*tblRow*/, Int /*tblNum*/)
          {return x;};

      // Y-axis convert methods
      virtual inline Double Yconvert(Double y, Int /*tblRow*/, Int /*tblNum*/)
          {return y;};
      
      virtual inline Double Yconvert_row(Double y, Int /*tblRow*/, Int /*tblNum*/)
          {return y;};
      
      virtual inline Double Yconvert_col(Double y, Int /*tblRow*/, Int /*tblNum*/)
          {return y;};
};


// <summary>
// Hooks for applications to write custom code that follows the standard TablePlot behaviour 
// </summary>

// <reviewed reviewer="" date="" tests="">
// </reviewed>

// <prerequisite>
//   <li> TablePlot
// </prerequisite>

// <etymology>
// Hooks into TablePlot callback functions that are triggered from the GUI. 
// </etymology>

// <synopsis>
// TablePlot functions are called directly via the Python-C++ binding, when
// GUI buttons are pressed. 
// These callbacks provide a way for the user-application to customize the
// response to GUI-initiated operations. These callbacks are called from
// fixed points in the TablePlot code.  If a user application needs to 
// implement any callbacks, an instance of this class (or one derived from it)
// needs to be sent in to TablePlot as a Plot Option.
//
// Hooks have been provided for
//  <li>   - (un)flagdata
//  <li>   - locatedata
//  <li>   - clearplot
//   These functions are stored in, and called from each BasePlot.

// </synopsis>

// <motivation>
// User apps need to be able to customize some behaviour that is initiated by GUI events. 
// </motivation>

// <thrown>
//    <li>
//    <li>
// </thrown>


// <todo asof="$DATE:$">
//   <li> 
// </todo>



class TPGuiCallBackHooks
{
   public:
      //Constructor
      TPGuiCallBackHooks(){LocateColumns.resize(0);};

      //Destructor
      virtual ~TPGuiCallBackHooks(){};

      // Specify a list of Table column names to use for the "locate" function 
      // This function is used by TablePlot to decide which columns to 
      // "locate" on.
       // Called from PanelParams.cc : PlotOption::validataParams();
       virtual Vector<String> getLocateColumns(){return LocateColumns;};

       // After the standard TP Locate function call, this method 
       //is called during printing
       //   the results, to accomodate custom formatting. 
       // Called from TablePlot::dumpLocateInfo().
       // The number of rows in the Matrix are the number of table 
       // rows selected by the
       // locate regions.
       // The Matrix (infomat) contains n+2 columns, 
       // where n = LocateColumns.nelements().
       // column 0 contains the row number.
       // column 1 contains the number of points selected per row
       // The rest of the columns are values for LocateColumns.
       // The String (cpol) contains information about selected 
       // cellrows/cellcols. (chans/pols).
      virtual Bool printlocateinfo(Vector<String> collist,
          Matrix<Double> infomat,Vector<String> cpol)
      {
         ostringstream os;
         
         IPosition mshape = infomat.shape();
         for(Int j=0;j<mshape[1];j++) {
            for(Int k=0;k<mshape[0];k++) {
               if(collist[k].contains("TIME")) {
                   os << collist[k] << ":" 
                      << MVTime( infomat(k,j)/C::day).string( MVTime::DMY,7) 
                      << ", " ; 
               }
               else
                   os << collist[k] << ":" << infomat(k,j) << ", " ; 
            }
            os << "[cellrow,cellcol] : " << cpol[j] << "\n";
         }
         SLog::slog()->out(os, "printlocateinfo", "TPCallBackHooks",
                  LogMessage::NORMAL4);
                        
         return True;
      };
      

       // After the standard TP flagdata function, 
       // this method is called (per BasePlot/Table)
       //   so any external flag-propagation can be carried out
       // Called from TablePlot::flagData.
       virtual Bool flagdata(String /*tablename*/){return True;};
       virtual Bool flagdata(Int /*f*/, Vector<String> /*collist*/,
                            Matrix<Double> /*infomat*/,Vector<String> /*cpol*/,
                            Bool /*ave*/ = False){
                  return True;
       }

       // During the standard TP clearPlot, this function 
       // is called immediately after
       //   each BasePlot destructor.
       // Called from TablePlot::deleteBasePlot.
       virtual Bool releasetable(Int /*nrows*/, Int /*ncols*/, Int /*panel*/,
           String /*tablename*/)
       {return True;};
       
       // A function to allow the creation of customized labels for
       // iteration plot panels.
       // Called from TablePlot::iterMultiPlotNext.
       virtual Bool createiterplotlabels(Vector<String> iteraxes, 
                Vector<Double> values, String &titlestring)
       {
          if(iteraxes.nelements() != values.nelements()) {
             titlestring = String("error");
             return True;
           }
           titlestring = String("");
           for(uInt i=0;i<iteraxes.nelements();i++) {
              titlestring += String(" : ") 
                       + iteraxes[i] + String(" : ") 
                       + String::toString((Int)values[i]) + String("  ");
           }
           return True;
       };
       
  protected:
       Vector<String> LocateColumns;
};

// <summary>
// Base Class for TablePlot "reset" callback
// </summary>

class TPResetCallBack
{
   public :
      //Constructor
      TPResetCallBack(){};

      //Destructor
      virtual ~TPResetCallBack(){};

      // Callback to signal full internal cleanup of TablePlot.
      // This callback is called from TablePlot::resetTP.
      virtual Bool reset(){
#if LOG0
        SLog::slog()->out("reset callback !",
           "reset", "TPResetCallBack", LogMessage::DEBUG1);
#endif
        return True;
      };

};


// Derived Class for conversion functions for TIME FORMATTING 
// This can also be done via a TaQL... which is best... 

// TRY NOT TO USE CONVERSION FUNCTIONS for TIME 
class TPConvertTimeX : public TPConvertBase
{
   public :
      TPConvertTimeX(){};
      ~TPConvertTimeX(){};

      inline Double Xconvert(Double x,Int /*tblRow*/,Int /*tblNum*/){
          return x/NSECINYEAR + MJDZERO;
      };
};

class TPConvertTimeY : public TPConvertBase
{
   public :
      TPConvertTimeY(){};
      ~TPConvertTimeY(){};
      inline Double Yconvert(Double y,Int /*tblRow*/,Int /*tblNum*/){
         return y/NSECINYEAR + MJDZERO;
      };
};

class TPConvertTimeXY : public TPConvertBase
{
   public :
      TPConvertTimeXY(){};
      ~TPConvertTimeXY(){};
      inline Double Xconvert_row(Double x,Int /*tblRow*/,Int /*tblNum*/){
         return x/NSECINYEAR + MJDZERO;
      };
      inline Double Yconvert_col(Double x,Int /*tblRow*/,Int /*tblNum*/){
         return x/NSECINYEAR + MJDZERO;
      };
};

// Convert Channel numbers to frequencies
class TPConvertChanToFreq : public TPConvertBase
{
   public :
      TPConvertChanToFreq(){offset=1400, interval=40;};
      ~TPConvertChanToFreq(){};
      Double offset,interval;

      // This plots the column numbers of the data array found in
      // the DATA column of. 'x' is the column index.  Note that
      // we don't need to convert the row values (polarizations)
      // Setting ColumnsXaxis to False forces only the column values
      // to be plotted.
      inline Double Xconvert_col(Double x,Int /*tblRow*/,Int /*tblNum*/){
         return x*interval + offset;
      };

};

} //# NAMESPACE CASA - END 


#endif

