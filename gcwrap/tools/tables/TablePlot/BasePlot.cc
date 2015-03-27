//# BasePlot.cc: Basic table access class for the TablePlot (tableplot) tool
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
//# $Id$


//# Includes


#include <cmath>

#include <casa/Exceptions.h>

#include <tables/TaQL/TableParse.h>
#include <tables/TaQL/TableGram.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableLock.h>
#include <tables/Tables/TableIter.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/MatrixMath.h>
#include <casa/Arrays/ArrayError.h>
#include <casa/Arrays/Matrix.h>
#include <casa/IO/LargeRegularFileIO.h>

#include <tables/TaQL/ExprMathNode.h>
#include <tables/TaQL/ExprMathNodeArray.h>
#include <tables/TaQL/ExprDerNode.h>
#include <tables/TaQL/ExprDerNodeArray.h>
#include <tables/TaQL/ExprFuncNode.h>
#include <tables/TaQL/ExprFuncNodeArray.h>
#include <tables/TaQL/ExprLogicNode.h>
#include <tables/TaQL/ExprLogicNodeArray.h>
#include <tables/TaQL/ExprNodeArray.h>
#include <tables/TaQL/ExprNodeSet.h>
#include <tables/TaQL/ExprNodeRep.h>
#include <tables/TaQL/ExprNodeRecord.h>
#include <tables/TaQL/ExprRange.h>
#include <tables/TaQL/RecordGram.h>

#include <casa/Utilities/DataType.h>
#include <casa/Quanta/MVTime.h>
#include <casa/System/ProgressMeter.h>

#include <tools/tables/TablePlot/BasePlot.h>

#include <casa/iomanip.h>
#include <casa/iostream.h> 
#include <casa/fstream.h> 
/*

   NOTE : PLEASE make sure you understand the behaviour of this class and it's
       data structures before changing too much !!!
*/

namespace casa { //# NAMESPACE CASA - BEGIN

#define TMR(a) "[User: " << a.user() << "] [System: " << a.system() << "] [Real: " << a.real() << "]"
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))

#define LOG0 0
#define LOG2 1

String BasePlot::clname = "BasePlot";

/* Default Constructor */
BasePlot::BasePlot()
{
   dbg=0;   
   ddbg=0;   adbg=0;
   String fnname = "BasePlot";
   log = SLog::slog();
#if LOG0
   log->FnEnter(fnname, clname);
#endif
   nip_p=0;
   nflagmarks_p=0;
   xtens_p.resize(0); ytens_p.resize(0); 
   colnames_p.resize(3); ipslice_p.resize(0);
   IndCnt_p.resize(0);
   locflagmarks_p.resize(0);
   DataStr_p.resize(0);
   TableTouch_p=0;
   Average_p = False;
   
   FlagColName_p = "FLAG"; fcol_p = False;
   FlagRowName_p = "FLAG_ROW"; frcol_p = False;
   ReductionType_p.resize(0);
   Layer_p=0;
   firsttime_p=True;
   
   Map_p.resize(0,0);
   FV = NULL;
   currentflagversion_p = String("main");
   flagdirection_p = 0;
   numflagpoints_p = 0;
   showflags_p = False;
        doscalingcorrection_p = False;
   
   pType_p=XYPLOT; 

   itsTab_p = NULL;
#if LOG0
   log->FnExit(fnname, clname);
#endif
}

/*********************************************************************************/

/* Destructor */
BasePlot::~BasePlot()
{
#if LOG0
   log->FnPass("~BasePlot", clname);
#endif
   if(FV) 
     delete FV;
}

/*********************************************************************************/

/* Attach a BasePlot to a table/subtable */
Int BasePlot::init(Table &tab, Int &tableNumber,String &rootTabName, 
            String &tableSelection, String &dataFlagColName, 
            String &rowFlagColName)
{
   String fnname = "init";
   //cout << "tableNumber=" << tableNumber
   //     << " rootTabName=" << rootTabName
   //     << " tableSelection=" << tableSelection
   //     << " dataFlagColName=" << dataFlagColName
   //     << " rowFlagColName=" << rowFlagColName
   //     << endl;
#if LOG0
   log->FnEnter(fnname, clname);
#endif
   //cout << "BasePlot tableNumber=" << tableNumber << endl;
   /* Attach table */
   
   SelTab_p = tab;
   tabNum_p = tableNumber;
   rootTabName_p = rootTabName;
   tabSelString_p = tableSelection;
   FlagColName_p = dataFlagColName;
   FlagRowName_p = rowFlagColName;
    
   if(SelTab_p.isNull()) BasePlotError(String("Null Table"));
   
   /* Check for the existence of Flag column names */
   /* FLAG and FLAG_ROW */
   fcol_p = SelTab_p.tableDesc().isColumn(FlagColName_p);
   frcol_p = SelTab_p.tableDesc().isColumn(FlagRowName_p);
   
   String msg = String( "Column : " ) + FlagColName_p;  
   if(fcol_p) 
      msg = msg + String( " exists " ); 
   else 
      msg = msg + String( " does not exist " );
#if LOG0
   log->out(msg, fnname, clname, LogMessage::DEBUGGING);
#endif

   msg = String( "Column : " ) + FlagRowName_p;
   if(frcol_p) 
      msg = msg + String( " exists " );
   else 
      msg = msg + String( " does not exist " );
#if LOG0
   log->out(msg, fnname, clname, LogMessage::DEBUGGING);
#endif
   
   
   /* Attach flag columns */
   /* Create TableColumns for the FLAG and FLAG_ROW columns */
   /* Do this only once.. */
   if(fcol_p) 
   {
      Flags_p.attach(SelTab_p,FlagColName_p);
      FlagColShape_p = Flags_p.shape(0);
   }
   if(frcol_p)
   {
      RowFlags_p.attach(SelTab_p,FlagRowName_p);
   }
#if LOG0
   log->out(String("FlagColName : ")+FlagColName_p + 
         String("FlagRowName : ")+FlagRowName_p,
         fnname, clname, LogMessage::DEBUGGING);
#endif

   /* clear bookkeeping arrays between tables */
   cleanUp();

   TableTouch_p=1;

   // Need to always create this with the root table !
   
   if(FV) 
   {
      delete FV;
      FV = NULL;
   }
   
   if(rootTabName.length()>0)
   {
      try
      {
         FV = new FlagVersion(rootTabName,FlagColName_p,FlagRowName_p);
         FV->attachFlagColumns("main", RowFlags_p, Flags_p, SelTab_p);
      }
      catch (AipsError x)
      {
         log->out("Cannot use Flag Versions.",
               fnname, clname, LogMessage::WARN);
         FV = NULL;
      }
      currentflagversion_p = String("main");
   }
   
   //if(fcol_p) FV->attachDataFlag(Flags_p,"main");
   //if(frcol_p) FV->attachRowFlag(RowFlags_p,"main");
#if LOG0   
   log->out(fnname, clname);
#endif
   return 0;
}
    
   

/*********************************************************************************/
String BasePlot::getTableName()
{
      return SelTab_p.tableName();  
}

/*********************************************************************************/

/* Create TableExprNodes from TaQL strings and obtain TaQL indices/column names */
Int BasePlot::createTENS(Vector<String> &datastr)
{
   String fnname = "createTENS";
#if LOG2
   log->FnEnter(fnname, clname);
#endif 
      
   /* Check the number of input strings */
   Int nTStr = datastr.nelements();
#if LOG0
   log->out(String("Number of strings : ") + String::toString(nTStr),
            fnname, clname, LogMessage::DEBUGGING);
#endif
   
   if(nTStr%2 != 0) 
       BasePlotError(String("Need an even number of TaQL strings"));
   
   nTens_p = nTStr/2; 

   IndCnt_p.resize(nTens_p);
   colnames_p.resize(0);
   ipslice_p.resize(0);
   try
   {
      createXTENS(datastr); 
      createYTENS(datastr);
   }
   catch(AipsError &x)
   {
      BasePlotError(String(x.getMesg()));
   }
   
   /* Fill in default shapes, if not specified in the TaQL */
   Bool tst=False;
   Vector<IPosition> colshapes;
   colshapes.resize(nip_p);
   Vector<Bool> isarraycol;
   isarraycol.resize(nip_p);
   
#if LOG0 
   log->out("Validate the TaQL slices", fnname, clname,
            LogMessage::DEBUGGING);
#endif 
   for(Int i=0;i<nip_p;i++)   
   {
      tst = SelTab_p.tableDesc().isColumn(colnames_p(i));
#if LOG0
      log->out(String("column ")+colnames_p(i),
            fnname, clname, LogMessage::DEBUGGING);
#endif
      ROTableColumn rot(SelTab_p,colnames_p(i));
                if(!rot.isNull())
      {
          ColumnDesc cdesc = rot.columnDesc();
          Slicer tempslice;
          if(cdesc.isArray()) // This is an arrayColumn.
          {
             colshapes[i] = rot.shape(0);
         isarraycol[i] = True;

#if LOG0
         log->out(String("COLUMN : ") + colnames_p(i) +
                  " with shape : " + String::toString(colshapes[i]),
                  fnname, clname, LogMessage::DEBUGGING);
         log->out( String("Is arraycol : ") +
                 String::toString(cdesc.isArray()),
                  fnname, clname, LogMessage::DEBUGGING);
         log->out( String("slice -> ") +
                 String::toString(ipslice-p[i])),
                  fnname, clname, LogMessage::DEBUGGING);
#endif
          }
          else 
          {
             colshapes[i] = IPosition(1,0); // This is a Scalar column
             isarraycol[i] = False;
          }
      }
   }


   /* Fill in one slice per TEN */
#if LOG0 
   log->out("Fill in max col shapes for each column ",
            fnname, clname, LogMessage::DEBUGGING);
#endif 
   
   Slicer tempslicer;
   IPosition istart,iend,istride,sliceshape;
   TENslices_p.resize(0);
   TENslices_p.resize(nTens_p);
   TENRowColFlag_p.resize(0);
   TENRowColFlag_p.resize(nTens_p);
   TENcolshapes_p.resize(0);
   TENcolshapes_p.resize(nTens_p);
   isArrayCol_p.resize(nTens_p);

   for(Int z=0;z<nTens_p;z++)
   {
      TENcolshapes_p[z] = colshapes[IndCnt_p[z]];
      isArrayCol_p[z] = isarraycol[IndCnt_p[z]];
      
      if( ! isArrayCol_p[z] ) // This is a Scalar column
      {
         istart = IPosition(1,0);
         iend = IPosition(1,0);
         istride = IPosition(1,1);
      
         //tempslicer = Slicer();
         tempslicer = Slicer(istart,iend,istride);
#if LOG0 
         ostringstream os;
         os << " Scalar -> " << istart <<  " "  
            << iend <<  " " << istride;
         log->out(os, fnname, clname, LogMessage::DEBUGGING);
#endif 
      }
      else 
      {
         //This is an array column and it makes sense to define a proper slice.
         sliceshape = ipslice_p[IndCnt_p[z]].inferShapeFromSource(
           TENcolshapes_p[z],istart,iend,istride);
         // this is crazy... the Slicer documentation says that
         // the second param is the "end" but it treats it as "length" !!
         tempslicer = Slicer(istart,sliceshape,istride);
#if LOG0 
         ostringstream os;
         os << " Sliceshape -> " << istart <<  " "  
            << iend <<  " " << istride;
         log->out(os, fnname, clname, LogMessage::DEBUGGING);
#endif 
      }
      
      TENslices_p[z] = tempslicer;
#if LOG0 
      ostringstream os;
      os << "z " << z << " slice" << TENslices_p[z];
      log->out(os, fnname, clname, LogMessage::DEBUGGING);
#endif
      // resize to an array of the shape defined by the slicer.
      TENRowColFlag_p[z].resize(TENslices_p[z].length().product());
      TENRowColFlag_p[z].set(False);
   }

#if LOG0 
   log->out(String("Number of col slices : ") +
            String::toString(colnames_p.nelements()),
            fnname, clname, LogMessage::DEBUGGING);
#endif 

#if LOG0 
   {
      ostringstream os;
      os << "Number of TENs : " << nTens_p;
      log->out(os, fnname, clname, LogMessage::DEBUGGING);
   }

   {
      ostringstream os;
      os << "IndCnt: " << IndCnt_p;
      log->out(os, fnname, clname, LogMessage::DEBUGGING);
   }
#endif 

#if LOG2
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/
/* Extract data from the table */
Int BasePlot::getData(Vector<String> &datastr, Int layer, 
     TPConvertBase* conv, TPGuiCallBackHooks* callbackhooks)
{
   
  // cout << "getData: datastr=" << datastr << " layer=" << layer
  //      << " conv=" << conv << " callbackhooks=" << callbackhooks << endl;
   /******************* Check whether or not to re-read the data **************************/
   String fnname ="getData";
#if LOG2 
   log->FnEnter(fnname, clname);
#endif 
   
   /* A bunch of debugging info. */
   String msg;
#if LOG0 
   msg = String(" BP layer: ")+String::toString( Layer_p)
       + String(" input layer: ")+String::toString(layer);
   log->out(msg, fnname, clname, LogMessage::DEBUGGING);
#endif 

#if LOG0 
   ostringstream os;
   os << "BP DataStr: " << DataStr_p << " input datastr: " << datastr ;
   log->out(os, fnname, clname, LogMessage::DEBUGGING);
#endif 

#if LOG0 
   msg = String("old layer=")+String::toString(Layer_p)
       + String(", new layer=")+String::toString(layer)
       + String(", table-touch=")+String::toString(TableTouch_p);
   log->out(msg, fnname, clname, LogMessage::DEBUGGING);
#endif 

   /* if different layer , just return */
   if(Layer_p==0) 
      Layer_p = layer;
   else if(Layer_p != layer) {
#if LOG0
      log->out("Not re-reading data (1).",
             fnname, clname, LogMessage::NORMAL2);
#endif
   }

   
   /* if same layer, then check if datastr or table has changed. */
   Int TQchange=1;
   if(DataStr_p.nelements() == datastr.nelements())
   {
      TQchange=0;
      for(Int j=0;j<(Int)datastr.nelements();j++)
         if( !datastr[j].matches(DataStr_p[j]) ) TQchange=1;
   }

   //cout << "datastr tqchange : " << TQchange << endl;
   //cout << "tabletouch : " << TableTouch_p << endl;

   TQchange = TableTouch_p ? 1 : TQchange ;
   
   /* If there is a convert function, then re-read the data */
   /* Make this smarter later, to not always do this */
   /* Do a dynamic cast. If it's the base class, then do NOT set TQchange=1 */
   
   //if(conv != NULL) TQchange = 1;
   if(conv == NULL) BasePlotError(String("The convert function is null !"));
#if LOG0
   log->out(String("BP TQchange: ")+ String::toString(TQchange),
            fnname, clname, LogMessage::DEBUGGING);
#endif
   if(TQchange==0) { 
       log->out("Not re-reading data (2).", 
           fnname, clname, LogMessage::NORMAL2);

#if LOG2
       log->FnExit(fnname, clname);
#endif 
       return 0;
   }
   //   cout << "Reading data..." << endl;
   
   /******************************* Reading the data **************************/
   
   TableTouch_p=0;
   DataStr_p.resize(datastr.nelements());
   DataStr_p = datastr;
   
   callbackhooks_p = callbackhooks;
   /******************************** Create TENs ******************************/
#if LOG0
   log->out("Create TENS ", fnname, clname, LogMessage::DEBUGGING);
#endif
   
   createTENS(DataStr_p);
   
#if LOG0
   {
      ostringstream os;
      os << "Get Data into storage arrays. "
         << "SelTab_p=" << SelTab_p 
         << "DataStr_p=" << DataStr_p;
      log->out(os, fnname, clname, LogMessage::DEBUGGING);
   }
#endif
   NRows_p = (SelTab_p).nrow();
   NAvgRows_p = 0;

   /******************* Read only the first row and get shape information ******************/
   Yshape_p.resize(0);
   Xshape_p.resize(0);
   //for( Int i=0;i<(Int)Yshape_p.nelements(); i++ )
   //{
   //   Yshape_p[i] = IPosition();
   //   Xshape_p[i] = IPosition();
   //}
   Yshape_p.resize(nTens_p);
   Xshape_p.resize(nTens_p);
   
   /* Setup the flagsum vector */
   firsttime_p=True;
   flagsum_p.resize(SelTab_p.nrow(),nTens_p); 
   flagsum_p.set(0);

   conv_p = conv;
   
   tmr.mark();
   
   try
   {
      /// TODO - clean this up to one createMap call.
      getYData(conv,True);  
      createMap();
      //cout << "Ydata ------------fine-------" << endl;
      getXData(conv,True);  
      //cout << "Xdata ------------fine-------" << endl;
      createMap();
   }
   catch(ArraySlicerError &x){
      BasePlotError(String("Error in TaQL indices : ") + x.getMesg());
   }
   catch(AipsError &x){
      BasePlotError(String("TaQL string Error : ") + x.getMesg());
   }
   
   log->out( String("Time to allocate mem : ")+ String::toString(tmr.all()),
       fnname, clname, LogMessage::NORMAL5);

   /* Check for shape. If the X axis is more than one, complain */
   /* If X shape is more than one, it needs to be the SAME as Y */
   for(Int z=0;z<nTens_p;z++) 
   {
      if(Xshape_p[z].product() > 1 && Xshape_p[z] != Yshape_p[z])
         BasePlotError(String("X-axis TaQL should only result in a ") +
                       String("Scalar, or must be exactly the same shape") +
                       String("as the Y-axis TaQL."));
   }//end of for z
      
#if LOG0 
   {
      ostringstream os;
      os << "xplotdata shape : " << xplotdata_p.shape() 
          << "\nyplotdata shape : " << yplotdata_p.shape()
          << "\nMap_p : " << Map_p
          << "\nReductionType : " << ReductionType_p 
          << "\nNPlots : " << NPlots_p;
      log->out(os, fnname, clname, LogMessage::DEBUG2);
   }
#endif 

   /******************************** Extract Data from Table ***************************/
#if LOG0
   log->out("Extracting data using TEN ... ",
           fnname, clname, LogMessage::DEBUGGING );
#endif
   

   /* read all the data now */
   
   try
   {
      //ProgressMeter pm(1.0,(Double)NRows_p,"","","","",False,NRows_p/10);
      //pm.update((Double)rc);
      
      if(NRows_p>0)
      {
         tmr.mark();
         getXData(conv);
#if LOG0 
         {
            ostringstream os;
            os << "Time to extract X data : " << tmr.all() << endl;
            log->out(os, fnname, clname, LogMessage::NORMAL5);
         }
#endif 

         tmr.mark();
         getYData(conv);
#if LOG0 
         {
            ostringstream os;
            os << "Time to extract Y data : " << tmr.all() << endl; 
            log->out(os, fnname, clname, LogMessage::NORMAL5);
         }
#endif 

	 
         //tmr.mark();
         //getFlags(True);
         //{
         //   ostringstream os;
         //   os << "Time to extract Flags : " << tmr.all(); 
         //   log->out(os, fnname, clname, LogMessage::NORMAL4);
         //}
      }//end of for rc
   }
   catch(AipsError &x)
   {
      BasePlotError(String("getData : ") + x.getMesg());
   }
   
   
#if LOG2
   log->FnExit(fnname, clname);
#endif 
   return 0;
}


/*********************************************************************************/
/* Compute averages. */
/* Called from ::setPlotRange */
/* All the unaveraged data and flags are ready by now */
Int BasePlot::computeAverages(Int averagenrows)
{
   // this is old time average roution. 
   // It is not used for chan-only
   // The new time/chan ave go through the non-ave branch 
   // to save mem and to speed it up 
   // This whole thing should be removed
   // keep this as-is, because it cost little with the
   // proper averagnrows flag 
   
   IPosition yshp;
   String fnname = "computeAverages";
   log->FnEnter(fnname, clname);
   /* Check if averaging needs to be done ! */
   if (averagenrows > 1 ) 
      Average_p = True;
   else 
      Average_p = False;

   /* If data is being averaged, compute various averages */
   if( Average_p )
   {
      tmr.mark();

      /* Compute the reduced number of rows */
      Int remrows=0;
      if( averagenrows >= NRows_p )
      {
         NAvgRows_p = 1;
         remrows=0;
         averagenrows = NRows_p;
      }
      else
      {
         NAvgRows_p = NRows_p / averagenrows;
         remrows = (NRows_p%averagenrows);
         if (remrows > 0 ) 
            NAvgRows_p += 1; // the end case.
      }

      log->out(String("NAVGROWS : ") + String::toString(NAvgRows_p) + 
               " and Rem : " + String::toString(remrows),
               fnname, clname, LogMessage::DEBUG2);

      /* Record the unaveraged shapes.. */
      yshp = yplotdata_p.shape();
#if LOG0
      ostringstream os;
      os << "Current yshape : " << yshp;
      log->out(os, fnname, clname, LogMessage::DEBUGGING);
#endif
      
      if( yshp[1] != NRows_p ) 
        BasePlotError(String("Internal Error in BasePlot::computeAverages"));
      
      /* Resize all the avg data arrays - if not already the correct size ! */
      if( NRows_p != (Int)avgindices_p.nelements() )   
         avgindices_p.resize(NRows_p);
      if( NAvgRows_p != (Int)avgrowflags_p.nelements() || 
          yshp[0] != (avgyplotdata_p.shape())[0] )
      {
#if LOG0
         log->out("about to resize", fnname, clname, LogMessage::DEBUGGING );
#endif
         avgyplotdata_p.resize(yshp[0],NAvgRows_p);
         avgtheflags_p.resize(yshp[0],NAvgRows_p);
         avgrowflags_p.resize(NAvgRows_p);
#if LOG0
         log->out("done with resize", fnname, clname, LogMessage::DEBUGGING );
#endif
         
         Int avcount=0;
         for(Int row=0; row<NRows_p-remrows; row+=averagenrows)
         {
            for(Int cnt=row; cnt<row+averagenrows; cnt++)
               avgindices_p[cnt] = avcount;
            avcount++;
         }
         if(remrows>0)
         {
            for(Int row=NRows_p-remrows; row<NRows_p; row++)
               avgindices_p[row] = avcount;
         }
      }

      /* Fill in averages for X and Y data and data and row flags. */
      computeXAverages(averagenrows, remrows);

      /* Y values : fill in avgyplotdata_p, avgtheflags_p */
      /* Honour flags, while averaging the Y-data */
      Bool tflag=True, avflag=True;
      Double yval=0.0;
                Int fcnt=0;
      Int AvgCounter=0;
      for(Int num=0; num<yshp[0]; num++)
      {
         /* Fill in all complete averages */
         AvgCounter=0;
         for(Int row=0; row<NRows_p-remrows; row+=averagenrows)
         {
            yval=0.0;
            avflag = ! showflags_p;
            fcnt = 0;
            for(Int cnt=row; cnt<row+averagenrows; cnt++)
            {
               tflag = theflags_p(num,cnt);

               if( showflags_p == False ) 
                   avflag &= tflag;
               else 
                   avflag |= tflag;

               if( tflag == showflags_p ) 
                {
                    // average only flagged or unflagged points.
                    fcnt++;
                    yval += yplotdata_p(num,cnt);
                }
            }
            if(fcnt==0) 
                avgyplotdata_p(num,AvgCounter) = 0;
            else 
                avgyplotdata_p(num,AvgCounter) = yval/fcnt;
            avgtheflags_p(num,AvgCounter) = avflag;
            AvgCounter++;
         }
         if(remrows>0 && AvgCounter+1 != NAvgRows_p) 
            BasePlotError(String("Internal ERROR IN Counting NAvgRows."));

         /* Fill in the end value */
         if(remrows>0)
         {
            yval=0.0;
            avflag = ! showflags_p;
            fcnt = 0;
            for(Int row=NRows_p-remrows; row<NRows_p; row++)
            {
               tflag = theflags_p(num,row);
               if( showflags_p == False ) 
                   avflag &= tflag;
               else 
                   avflag |= tflag;
               if( tflag == showflags_p ) // average only unflagged points.
               {
                  fcnt++;
                  yval += yplotdata_p(num,row);
               }
            }
            if(fcnt==0) 
                avgyplotdata_p(num,AvgCounter) = 0;
            else 
                avgyplotdata_p(num,AvgCounter) = yval/fcnt;
            avgtheflags_p(num,AvgCounter) = avflag;
         }
      }
      /* Y values : Done fill in avgyplotdata_p, avgtheflags_p */
      
      /* Do the row-flags : avgrowflags_p */
      AvgCounter=0;
      Bool rflag=True;
      for(Int row=0; row<NRows_p-remrows; row+=averagenrows)
      {
         rflag=True;
         for(Int cnt=row; cnt<row+averagenrows; cnt++) 
             rflag &= rowflags_p(cnt);
         avgrowflags_p[AvgCounter] = rflag;
         AvgCounter++;
      }
      if(remrows>0)
      {
         rflag=True;
         for(Int row=NRows_p-remrows; row<NRows_p; row++)
         {
            rflag &= rowflags_p[row]; 
         }
         avgrowflags_p[AvgCounter] = rflag; 
      }
      /* Done the row-flags : avgrowflags_p */

#if LOG0
      {
         ostringstream os;
         os <<"Shapes !!" 
            << "\n\tX : " << avgxplotdata_p.shape()   
            << "\n\tY : " << avgyplotdata_p.shape()
            << "\n\tF : " << avgtheflags_p.shape()
            << "\n\tRF : " << avgrowflags_p.shape();
            //<< avgindices_p << endl;
         log->out(os, fnname, clname, LogMessage::DEBUGGING);
      }
#endif

      log->out(String("Time to compute Average : ")
               + String::toString( tmr.all() ), 
               fnname, clname, LogMessage::NORMAL5 );
   }
   else
   {
      //cout << "Do not computer average-------------" << endl;
      /* Free up this memory if it is not going to be used. */
#if LOG0
      log->out("resizing to zero", fnname, clname, LogMessage::DEBUGGING);
#endif
      avgxplotdata_p.resize(0,0);
      avgyplotdata_p.resize(0,0);
      avgtheflags_p.resize(0,0);
      avgrowflags_p.resize(0);
      avgindices_p.resize(0);
      
   }
   
   log->FnExit(fnname, clname);
   return 0;
}

/*********************************************************************************/
Int BasePlot::computeXAverages(Int averagenrows, Int remrows)
{
   String fnname = "computeXAverages";
   IPosition xshp;
   
   xshp = xplotdata_p.shape();
   avgxplotdata_p.resize(xshp[0],NAvgRows_p);
   Int AvgCounter=0;
   
   /* X values : fill in avgxplotdata_p */
   Double xval=0.0;
   for(Int num=0; num<xshp[0]; num++)
   {
      /* Fill in all complete averages */
      AvgCounter=0;
      for(Int row=0; row<NRows_p-remrows; row+=averagenrows)
      {
         xval=0.0;
         for(Int cnt=row; cnt<row+averagenrows; cnt++)
            xval += xplotdata_p(num,cnt);
         avgxplotdata_p(num,AvgCounter) = xval/averagenrows;
         AvgCounter++;
      }
      if(remrows>0 && AvgCounter+1 != NAvgRows_p) 
         BasePlotError(String("Internal ERROR IN Counting NAvgRows."));
#if LOG0
      log->out(String("Rem Rows : ") +String::toString(remrows), 
            fnname, clname, LogMessage::DEBUGGING);
#endif
      
      /* Fill in the end value */
      if(remrows>0)
      {
         xval=0.0;
         for(Int row=NRows_p-remrows; row<NRows_p; row++)
            xval += xplotdata_p(num,row);
         avgxplotdata_p(num,AvgCounter) = xval/(remrows);
      }
   }
   
   return 0;
}

/*********************************************************************************/
/*********************************************************************************/
Int BasePlot::createMap()
{
   //cout << "createMap----------------" << endl;
   String fnname = "createMap";
#if LOG2
   log->FnEnter(fnname, clname);
#endif 
   
   /* We have...    Yshape_p[z], Tenslices_p[z]
      Need to fill in...  ReductionType_p[z], and Map_p.  */

   /* Col 0 : index into xplotdata 
      Col 1 : index into yplotdata
      Col 2 : index into tens (z)
      Col 3 : yplotdata row index
      Col 4 : yplotdata col index
   */
#if LOG0
   log->out("Filling in Map_p", fnname, clname, LogMessage::DEBUGGING);
#endif
   
   Map_p.resize(NPlots_p,5);
   Int xptr=0,yptr=0;
   IPosition fshp, istart, iend,istride;
   
   ReductionType_p.resize(nTens_p);
   for(Int z=0;z<nTens_p;z++) 
   {
      /* Figure out the type of reduction - for each TEN */
      IPosition fshp = TENslices_p[z].length();
#if LOG0 
      ostringstream os;
      os  << "SLICE SHAPE=" << fshp
          << "\nYDATA SHAPE=" << Yshape_p[z]
          << "\nIsArrayCol=" << isArrayCol_p[z] << endl; 
      log->out(os, fnname, clname, LogMessage::DEBUG2);
#endif 

      Int nrow=0,ncol=0;
      if( (! isArrayCol_p[z]) || 
          (fshp.nelements() == Yshape_p[z].nelements() && 
           fshp == Yshape_p[z])) {
         ReductionType_p[z] = 0;
         nrow = Yshape_p[z][0];   
         if (Yshape_p[z].nelements()>1) 
            ncol = Yshape_p[z][1];
         else 
            ncol=1;
      }
      else {
         if (Yshape_p[z].product()==1)
         {
            ReductionType_p[z] = 1; // MEAN
            nrow = 1; 
            ncol = 1;
         }
         else { // MEANS
            if(Yshape_p[z][0] == fshp[0]) 
            {
               ReductionType_p[z] = 2; // averaged over cols (chans).
               nrow = Yshape_p[z][0];   
               ncol = 1;
            }
            else
            {
               if(Yshape_p[z][0] == fshp[1]) 
               {
                  ReductionType_p[z] = 3; // averaged over rows (pols).
                  nrow = 1; 
                  ncol = Yshape_p[z][0];
               }
            }
         }
      }

      /* Fill up Map_p for this TEN */
      /* Use fortran ordering for the row,col indices... 
         to make it compatible with Array 
      */
#if LOG0 
      {
         ostringstream os;
         os << "z=" << z << " Xshape=" << Xshape_p[z] 
            << " Yshape=" << Yshape_p[z] 
            << " ReductionType=" << ReductionType_p[z] << endl;
         log->out(os, fnname, clname, LogMessage::DEBUG2);
      }
#endif 
                
      for(Int col=0;col<ncol;col++)
      for(Int row=0;row<nrow;row++)
      { 
         Map_p(yptr,0) = (Xshape_p[z].product() > 1 && 
                          Xshape_p[z] == Yshape_p[z]) ? yptr : xptr;  
         //Map_p(yptr,0) = xptr;  
         Map_p(yptr,1) = yptr;
         Map_p(yptr,2) = z;
         Map_p(yptr,3) = row;
         Map_p(yptr,4) = col;

         yptr++;
      }
      //xptr++;
      xptr += Xshape_p[z].product();

   }//end of for z
   if(yptr != NPlots_p) 
       log->out("Wrong number of NPlots !!!", 
                fnname, clname, LogMessage::WARN );

   //cout << "Map_p : " << Map_p << endl;
   
   //cout << "createMap---------------2" << endl;
#if LOG2 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/
/*********************************************************************************/

/* Read in marked flag regions */
Int BasePlot::convertCoords(Vector<Vector<Double> > &flagmarks)
{
   String fnname = "convertCoords";
#if LOG0 
   log->FnEnter(fnname, clname);
#endif 
   
   nflagmarks_p = flagmarks.nelements();
   log->out(String( "BP :: nflagmarks_p : " ) + 
            String::toString(nflagmarks_p), 
            fnname, clname, LogMessage::DEBUG2);
   
   // record to a history list somewhere here before destroying...
   locflagmarks_p.resize(flagmarks.nelements());
   
   locflagmarks_p = flagmarks;
   numflagpoints_p = 0;
#if LOG0
   //locflagmarks_p[llcx, llcy, urcx, urcy]
   ostringstream os;
   for(Int i=0;i<nflagmarks_p;i++)
      os << locflagmarks_p[i] << " ";
   log->out(os, fnname, clname, LogMessage::DEBUG2);
#endif
   // change units of locflagmarks_p, if any units changes were 
   //used while plotting..
#if LOG0
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/
/* Read in marked flag regions */
// direction = 1 : True : flag
// direction = 0 : False : unflag
Int BasePlot::updateFlagHistory(Vector<Vector<Double> > &flagmarks, 
         Int &direction, Int &numflags)
{
   flagmarks.resize(0);
   flagmarks = locflagmarks_p;

   direction = flagdirection_p;

   numflags = numflagpoints_p;

   return 0;
}
   

/*********************************************************************************/
/*********************************************************************************/
Bool BasePlot::selectedPoint(Int np,Int nr)
{
   static Double xvalue=0.0, yvalue=0.0;
   //cout << "X,Y" << xplotdata_p(Map_p(np,0),nr) << "," 
   //     << yplotdata_p(Map_p(np,1),nr) << endl;
   //for(int nf=0;nf<nflagmarks_p;nf++) 
   //   cout << locflagmarks_p[nf] << endl;
   
   /* Fill in xvalue, yvalue */
   if( ! Average_p )
   {
      xvalue = xplotdata_p(Map_p(np,0),nr); 
      yvalue = yplotdata_p(Map_p(np,1),nr); 
   }
   else 
   { 
      xvalue = avgxplotdata_p(Map_p(np,0),avgindices_p[nr]); 
      yvalue = avgyplotdata_p(Map_p(np,1),avgindices_p[nr]);
   }
   
   /* Check for selection */
   for(int nf=0;nf<nflagmarks_p;nf++)
   {
      if(xvalue >= (locflagmarks_p[nf])[0] &&
         xvalue <= (locflagmarks_p[nf])[1] && 
         yvalue >= (locflagmarks_p[nf])[2] && 
         yvalue <= (locflagmarks_p[nf])[3]) 
      {
         return True;
      }
   }

   return False;
}
/*********************************************************************************/
//// make this also use fv. - then delete this fn.
Int BasePlot::clearFlags()
{
   String fnname = "clearFlags";
#if LOG0
   log->FnEnter(fnname, clname);
#endif
   
   Array<Bool> flagcol;
   Vector<Bool> rflagcol;
   
   if(fcol_p)
   {
      Flags_p.attach(SelTab_p,FlagColName_p);
   }
   if(frcol_p)
   {
      RowFlags_p.attach(SelTab_p,FlagRowName_p);
   }
      
   if(fcol_p)
   {
      flagcol = (Flags_p.getColumn());  
      ostringstream os;
      os << "flagcol shape : " << flagcol.shape();
      log->out(os, fnname, clname, LogMessage::DEBUGGING);
      flagcol = False;
      Flags_p.putColumn(flagcol);
   }
   
   if(frcol_p)
   {
      rflagcol = RowFlags_p.getColumn();
      ostringstream os;
      os << "rflagcol length : " << rflagcol.nelements();
      log->out(os, fnname, clname, LogMessage::DEBUGGING);
      rflagcol = False;
      RowFlags_p.putColumn(rflagcol);
   }
      
   SelTab_p.flush();
   theflags_p.set(False);

   log->out("All Flags Cleared !!", fnname, clname, LogMessage::DEBUGGING);
#if LOG0
   log->FnExit(fnname, clname);
#endif

   return 0;
}

/*********************************************************************************/
/*                     TPPlotter interaction functions                           */
/*********************************************************************************/
/*********************************************************************************/
/* Compute the combined plot range */
Int BasePlot::setPlotRange(Double &xmin, Double &xmax, Double &ymin, 
     Double &ymax, Bool showflags, Bool /*columnsxaxis*/, String flagversion,
     Int averagenrows, String /*connectpoints*/, Bool doscalingcorrection,
     String multicolour, Bool /*honourxflags*/)
{
   String fnname = "setPlotRange";
#if LOG0
   log->FnEnter(fnname + "(xmin, xmax, ymin, ...)", clname);
#endif

   //cout << "flagversion=" << flagversion << endl;
  // cout << "BasePlot::setPlotRange" << endl;

   Matrix<Double> xprange_p, yprange_p;
   xprange_p.resize(NPlots_p,2);
   yprange_p.resize(NPlots_p,2);
   Double xvalue=0.0, yvalue=0.0;
   Bool flag=False, rflag=False;

   /* Record the showflags state */
   showflags_p = showflags;
   doscalingcorrection_p = doscalingcorrection;
   multicolour_p = multicolour;

        /* Update Map_p(xxx,5) for plot colour to use, 
           depending on multicolour setting */
        /*
        Int base=0,addon=0;
        Map_p(0,5)=0;
        for(Int i=1;i<NPlots_p;i++)
        {
             if(Map_p(i,2)-1 == Map_p(i-1,2)) base = addon+1; 
             // Increment to next TaQL
             if(multicolour.matches("both")) Map_p(i,5)=Map_p(i-1,5)+1;
             else if(multicolour.matches("cellrow"))
             {
                     Map_p(i,5) = base + Map_p(i,3);
             }
             else if(multicolour.matches("cellcol"))
             {
                     Map_p(i,5) = base + Map_p(i,4);
             }
             else Map_p(i,5)=0;
             addon = MAX(addon,Map_p(i,5));
        }

        *os << LogIO::DEBUGGING << Map_p << LogIO::POST
        */

   /* Reset the plot ranges */
   for(int i=0;i<NPlots_p;i++)
   {
      xprange_p(i,0) = 1e+30;
      xprange_p(i,1) = -1e+30;
      yprange_p(i,0) = 1e+30;
      yprange_p(i,1) = -1e+30;
   }
   
   /* Update the flag version ! */
   tmr.mark();
   getFlags(flagversion, showflags);
   log->out(String("time to getFlags") + String::toString(tmr.all()) 
            + " sec.", fnname, clname, LogMessage::DEBUGGING);

   /* Update the averages. 
      This function knows when it should be a no-op */
   computeAverages(averagenrows);


   /* compute min and max for each Plot */
   Bool choosepoint=False;
   Int NR=0;
   for(int i=0;i<NPlots_p;i++)
   {
      if( ! Average_p ) 
          NR = NRows_p; 
          /* No averages. Use the un-averaged arrays */
      else 
          NR = NAvgRows_p; 
          /* Use the averaged data */
      
      //cout << " NR=" << NR << endl;
      for(int rc=0;rc<NR;rc++)
      {
         choosepoint = False;
         
         if( ! Average_p )
         {
            xvalue = xplotdata_p(Map_p(i, 0), rc);
            yvalue = yplotdata_p(Map_p(i, 1), rc);
            flag = theflags_p(Map_p(i, 1), rc);
            rflag = rowflags_p[rc];
         }
         else
         {
            xvalue = avgxplotdata_p(Map_p(i,0),rc);
            yvalue = avgyplotdata_p(Map_p(i,1),rc);
            flag = avgtheflags_p(Map_p(i,1),rc);
            rflag = avgrowflags_p[rc];
         }
         
         if(showflags == False) {
            if ((flag == False) && (rflag == False)) 
               choosepoint = True;
         }
         else {
            if ((flag == True) || (rflag == True)) 
               choosepoint = True;
         }
            
         if(choosepoint) {
            //cout << "xvalue=" << std::setprecision(12) << xvalue 
            //     << " yvalue=" << yvalue
            //     << " flag=" << flag << " rflag=" << rflag << endl;
            if (xvalue <= xprange_p(i,0)) 
               xprange_p(i,0) = xvalue;
            if (xvalue >= xprange_p(i,1)) 
               xprange_p(i,1) = xvalue;
            if (yvalue <= yprange_p(i,0)) 
               yprange_p(i,0) = yvalue;
            if (yvalue >= yprange_p(i,1)) 
               yprange_p(i,1) = yvalue;
         }
      }// end of for rows...
      
   }// end of for nplots
   
   xmin=0; 
   xmax=0;
   ymin=0;
   ymax=0;
   xmin = xprange_p(0,0);
   xmax = xprange_p(0,1);
   ymin = yprange_p(0,0);
   ymax = yprange_p(0,1);
   

   /* get a comnined min,max */

   for(int qq=1;qq<NPlots_p;qq++)
   {
      xmin = MIN(xmin, xprange_p(qq,0));
      xmax = MAX(xmax, xprange_p(qq,1));
   }   
   for(int qq=1;qq<NPlots_p;qq++)
   {
      ymin = MIN(ymin, yprange_p(qq,0));
      ymax = MAX(ymax, yprange_p(qq,1));
   }

   ostringstream os;
   os << "BasePlot::setPlotRange=== Ranges : [" 
        << std::setprecision(12) << xmin << "," 
       << xmax << "] [" << ymin << "," << ymax << "]" << endl;
   log->out(os, fnname, clname, LogMessage::DEBUG2);

#if LOG0
   log->FnExit(fnname, clname);
#endif
   return 0;
}
/*********************************************************************************/
Double BasePlot::getXVal(Int pnum, Int col)
{
   if( ! Average_p ) { 
       //cout << "xval=" << std::setprecision(12) 
       //     << xplotdata_p(Map_p(pnum,0),col) << endl;
       return xplotdata_p(Map_p(pnum,0),col);
   }
   else 
       return avgxplotdata_p(Map_p(pnum,0),col);
}

/*********************************************************************************/

Double BasePlot::getYVal(Int pnum, Int col)
{
   if( ! Average_p ) {
       //cout << "yval=" << std::setprecision(12) 
       //     << yplotdata_p(Map_p(pnum,1),col) << endl;
       return yplotdata_p(Map_p(pnum,1),col);
   }
   else 
       return avgyplotdata_p(Map_p(pnum,1),col);
}

/*********************************************************************************/
/* col means row here ! */
Bool BasePlot::getYFlags(Int pnum, Int col)
{
   //cout << "Average_p" << Average_p << endl;
   //cout << "col=" << col << " rowflag=" << rowflags_p[col] << endl;
   if( ! Average_p ) 
       return theflags_p(Map_p(pnum,1),col) | rowflags_p[col];
   else 
       return avgtheflags_p(Map_p(pnum,1),col) | avgrowflags_p[col];
}

/*********************************************************************************/

Int BasePlot::getColourAddOn(Int pnum)
{
   if(multicolour_p.matches("both")) 
       return pnum;
   else if(multicolour_p.matches("cellrow")) 
       return Map_p(pnum,3);
   else if(multicolour_p.matches("cellcol")) 
       return Map_p(pnum,4);
   else 
       return 0;
}

/*********************************************************************************/

Int BasePlot::getNumPlots()
{
   return NPlots_p;
}

/*********************************************************************************/

Int BasePlot::getNumRows()
{
   if( ! Average_p ) 
       return NRows_p;
   else 
       return NAvgRows_p;
}

/*********************************************************************************/

Int BasePlot::getPlotType()
{
    log->FnEnter( "getPlotType", "getPlotType()" );
    return pType_p;
}

/*********************************************************************************/

/*********************************************************************************/

Vector<String> BasePlot::getBasePlotInfo()
{
   Vector<String> dat(DataStr_p.nelements()+2);
   if(rootTabName_p.length()==0) 
       dat[0] = SelTab_p.tableName();
   else 
       dat[0] = rootTabName_p;

   dat[1] = tabSelString_p;
   
   for(Int i=2;i<(Int)dat.nelements();i++)
      dat[i] = DataStr_p[i-2];
   
   return dat;
}


/****************************************************************************/
/* Locate data. The first row of the matrix is to be filled by the 
   number of points selected in a row. The rest of the rows are the 
   user-supplied columns. */
/* Note that this IGNORES flags.... the selected points are not just 
   unflagged ones.  If this is to be put in, combine the rowcount++ 
   with a check on theflags_p */
Int BasePlot::locateData(Vector<String> collist, Matrix<Double> &info, 
        Vector<String> &cpol)
{
   String fnname = "locateData";
   {
#if LOG0
      ostringstream os;
      os << fnname << "(collist=" << collist 
         << " info=" << info << " cpol=" << cpol << ")";
      log->FnEnter(os, clname);
#endif
   }
   /* Number of columns +2 (for row number and number of selected 
      points in this row ) */
   Int ncoll = collist.nelements();

   Int maxlen = NRows_p;

   numflagpoints_p = 0;
   
   /* Initialize to full size.... - resize back later... */
   info.resize(IPosition(2, ncoll, maxlen));
   info = 0.0;
   cpol.resize(maxlen);
   
   /**********************************/
   
   /* Create TENs for each column - so that expressions will 
      also work.. for selected rows */
   Vector<TableExprNode> ctens;
   ctens.resize(ncoll-2);

#if LOG0
   log->out("Create TENS", fnname, clname, LogMessage::DEBUG2);
#endif

   /* Create TENS */
   for(Int i=0;i<ncoll-2;i++) 
   {
      try
      {
         ctens[i] = RecordGram::parse(SelTab_p, collist[i+2]);
      }
      catch (AipsError x)
      {
#if LOG0
         ostringstream os;
         os << " Column expression " << collist[i+2] << " isn't okay ";
         log->out(os, fnname, clname, LogMessage::SEVERE);
#endif
         BasePlotError(x.getMesg());
      }
      
      /* Check shape of column */
      if(!ctens[i].isScalar()) 
         BasePlotError(String("Need Scaler column for ") + collist[i+2]);
      
      //cout << "2--type=" << ctens[i].dataType() << endl;
      /* Check datatype of column */
      if(!(ctens[i].dataType() == TpDouble 
      	      || ctens[i].dataType() == TpFloat 
      	      || ctens[i].dataType() == TpInt))
            BasePlotError(String("Need Double, Float or Int, and not ") + 
                     ctens[i].dataType() + String(" for ") + collist[i+2]);

         
   }// for ncoll-2

   /**********************************/
   Matrix<String> Lind = getLocateIndices();
   //cout << "Lind=" << Lind << endl;

   log->out("Now start looking at the data", 
            fnname, clname, LogMessage::DEBUG2);
   
   Int nselrows=0;
   Double tmpval=0.0;
   Bool okay=False;
   
#if LOG0
   log->out(String("NRows=") + String::toString(NRows_p) +
            String(" NPlots=") + String::toString(NPlots_p),
            fnname, clname, LogMessage::DEBUG2);
#endif

   if(nflagmarks_p>0)
   {
      for(int nr=0;nr<NRows_p;nr++)
      {

         cpol(nselrows) = String("");

         /* Find out the number of selected points in this row */
         Int rowcount=0;
         for(int np=0; np < NPlots_p; np++) {
            /* np ---> ten,chan,pol 
               fslice from ten - read the start,end,stride
               count with chan,pol into start,end,stride
               to get actual chan and pol */
                
            okay = False;
            if(showflags_p == False) {
               if ((theflags_p(Map_p(np, 1), nr) == False) && 
                   (rowflags_p[nr] == False) ) 
                  okay = True;
            }
            else {
               if ((theflags_p(Map_p(np, 1), nr) == True) || 
                   (rowflags_p[nr] == True) ) 
                  okay = True;
            }
   
            //if(selectedPoint(np,nr) &&  !theflags_p(Map_p(np,1),nr) && 
            //                 !rowflags_p[nr])
            if(okay && selectedPoint(np, nr))
            {   
               rowcount++;
               cpol[nselrows] += String("[") +
                  Lind(np, 0) +
                  String(",") + 
                  Lind(np, 1) +
                  String("] ");
               numflagpoints_p++;
            }
         }

         /* If there are selected points, add an entry to 'info' 
            for this row */
         if(rowcount > 0)
         {
#if LOG0
            log->out(String("nselrows=") + String::toString(nselrows) +
                " nr=" + String::toString(nr),
                fnname, clname, LogMessage::DEBUG2);
#endif
            /* Number of points into first column */
            info(0, nselrows) = nr;
            info(1, nselrows) = rowcount;
   
#if LOG0
            ostringstream os;
            os  << "rowcount : " << info(nselrows, 1);
            log->out(os, fnname, clname, LogMessage::DEBUG2);
#endif
   
            /* Look into the table at row 'nr' and fill in the user columns */
            for(Int i=2; i< ncoll; i++) 
            {
               ctens[i-2].get(nr, tmpval);
               info(i, nselrows) = tmpval;
#if LOG0               
               ostringstream os;
               os << collist[i] << " : " << info(i, nselrows); 
               log->out(os, fnname, clname, LogMessage::DEBUG2);
#endif
            }

            nselrows++;
         }
      }
   }
   
   
    /******************************/
#if LOG0   
   log->out( "Done", fnname, clname, LogMessage::DEBUG2);
#endif

   /* Resize to actually used size... */

   info.resize(IPosition(2,ncoll,nselrows),True);
   cpol.resize(nselrows, True);

#if LOG0
   {
      ostringstream os;
      os << "setlect flag candidate:\n" << cpol;
      log->out(os, fnname, clname, LogMessage::DEBUG2);
   }
#endif

#if LOG0
   {
      ostringstream os;
      os << info;
      log->out(os, fnname, clname, LogMessage::DEBUG2);
   }
#endif

#if LOG0
   log->FnExit(fnname, clname);
#endif
   return 0;
}
/****************************************************************************/
/* Locate data. The first row of the matrix is to be filled by the 
   number of points selected in a row. The rest of the rows are the 
   user-supplied columns. */
/* Note that this IGNORES flags.... the selected points are not just 
   unflagged ones.  If this is to be put in, combine the rowcount++ 
   with a check on theflags_p */
Int BasePlot::locateData(Vector<String> collist, Matrix<Double> &info, 
        Vector<String> &cpol, 
        Matrix<Int>& rmap, Matrix<Int>& cmap)
{
   String fnname = "locateData";
#if LOG0
   {
      log->FnEnter(fnname + "(collist, info, cpol, cmap)", clname);

      ostringstream os;
      os << "collist=" << collist 
         << " info=" << info 
         << " cpol=" << cpol 
         << " rmap=" << rmap 
         << " cmap=" << cmap ;
      log->out(os, fnname, clname, LogMessage::DEBUG2);
   }
#endif

   /* Number of columns +2 (for row number and number of 
      selected points in this row ) */
   Int ncoll = collist.nelements();
   Int maxlen = NRows_p;
   
#if LOG0
   {
      ostringstream os;
      os << "ncoll=" << ncoll << " NRows_p=" << NRows_p << endl;
      log->out(os, fnname, clname, LogMessage::DEBUG2);
   }
#endif
  

   numflagpoints_p = 0;
   
   /* Initialize to full size.... - resize back later... */
   info.resize(IPosition(2, ncoll, maxlen));
   info = 0.0;
   cpol.resize(maxlen);
   
   /**********************************/
   
   /* Create TENs for each column - so that expressions will 
      also work.. for selected rows */
   Vector<TableExprNode> ctens;
   ctens.resize(ncoll-2);

#if LOG0
   log->out("Create TENS", fnname, clname, LogMessage::DEBUG2);
#endif

   //cout << "SelTab_p=" << SelTab_p
   //     << " name=" << SelTab_p.tableName() << endl;

   /* Create TENS */
   for(Int i=0;i<ncoll-2;i++) 
   {
      try
      {
         ctens[i] = RecordGram::parse(SelTab_p, collist[i+2]);
      }
      catch (AipsError x)
      {
#if LOG0
         ostringstream os;
         os << " Column expression " << collist[i+2] << " isn't okay ";
         log->out(os, fnname, clname, LogMessage::SEVERE);
#endif
         BasePlotError(x.getMesg());
      }
      
      /* Check shape of column */
      if (!ctens[i].isScalar()) 
         BasePlotError(String("Need Scaler column for ") + collist[i+2]);

      //cout << "1 ---type=" << ctens[i].dataType() << endl;
      /* Check datatype of column */
      if(!(ctens[i].dataType() == TpDouble 
      	      || ctens[i].dataType() == TpFloat 
      	      || ctens[i].dataType() == TpInt))
            BasePlotError(String("Need Double, Float or Int, and not ") + 
                     ctens[i].dataType() + String(" for ") + collist[i+2]);
         
   }// for ncoll-2

   /**********************************/
   //Matrix<String> Lind = getLocateIndices(cmap);
   //cout << "Lind=" << Lind << endl;

   IPosition istart;
   IPosition iend;
   IPosition istride;
   Slicer fslice;
   Matrix<Int> polids(NPlots_p, 2);
   //cout << "nTens_p=" << nTens_p << endl;
   Int polCnt = 0;
   polids = 0;
   for (int z=0;z<nTens_p;z++) {
      if (isArrayCol_p[z]) {
         fslice = TENslices_p[z];
         istart = fslice.start();
         iend = fslice.end();
         istride = fslice.stride();
         for (Int l = istart[1]; l <= iend[1]; l += istride[1]) {
            for (Int k = istart[0]; k <= iend[0]; k += istride[0]) {
               if (polCnt < NPlots_p) {
                   polids(polCnt, 0) = k;
                   polids(polCnt, 1) = l;
                   polCnt++;
               }
            }
         } 
      }
   }
   //cout << "polCnt=" << polCnt << " polids=" << polids << endl;

   log->out("Now start looking at the data", 
            fnname, clname, LogMessage::DEBUG2);
   
   Int nselrows=0;
   Double tmpval=0.0;
   Bool okay=False;
   
#if LOG0
   log->out(String("NRows=") + String::toString(NRows_p) +
            String(" NPlots=") + String::toString(NPlots_p),
            fnname, clname, LogMessage::DEBUG2);
#endif

   if(nflagmarks_p>0)
   {
      for(int nr=0;nr<NRows_p;nr++)
      {
         cpol(nselrows) = String("");

         /* Find out the number of selected points in this row */
         Int rowcount=0;
         for(int np=0; np < NPlots_p; np++) {
            /* np ---> ten,chan,pol 
               fslice from ten - read the start,end,stride
               count with chan,pol into start,end,stride
               to get actual chan and pol */
                
            okay = False;
            if(showflags_p == False) {
               if ((theflags_p(Map_p(np, 1), nr) == False) && 
                   (rowflags_p[nr] == False) ) 
                  okay = True;
            }
            else {
               if ((theflags_p(Map_p(np, 1), nr) == True) || 
                   (rowflags_p[nr] == True) ) 
                  okay = True;
            }
   
            Int rowCnt = rmap.nrow();

            //if(selectedPoint(np,nr) &&  !theflags_p(Map_p(np,1),nr) && 
            //                 !rowflags_p[nr])
            if(okay && selectedPoint(np, nr))
            {   
               rowcount++;

               //np is the (nAvePol * aveChanId) count 0, 1, ...
               //nr is the aved row count
               //cout << "np=" << np << " nr=" << nr;
               Int sp = 0;
               for (Int s = 0; s < rowCnt; s++) {
                  if (rmap(s, 0) == nr) {
                     sp = rmap(s, 2);
                     break;
                  }
               }
               //cout << " sp=" << sp << endl;

               for (Int t = 0; t < Int(cmap.nrow()); t++) {
                  if (sp == cmap(t, 4) && polids(np, 1) == cmap(t, 3)) {
                     cpol[nselrows] += String("[") +
                           String::toString(polids(np, 0)) +
                           String(",") +
                           String::toString(cmap(t, 0)) +
                           String(":") +
                           String::toString(cmap(t, 1)) +
                           String(":") +
                           String::toString(cmap(t, 2)) +
                           String("]"); 
                  } 
               }

               //cpol[nselrows] += String("[") +
               //   Lind(np, 0) +
               //   String(",") + 
               //   Lind(np, 1) +
               //   String("] ");
               numflagpoints_p++;
            }
         }

         /* If there are selected points, add an entry to 'info' 
            for this row */
    //cout << "np=" << np << " nr=" << nr << " rowcount=" << rowcount << endl;
         if(rowcount > 0)
         {
#if LOG0
            log->out(String("nselrows=") + String::toString(nselrows) +
                " nr=" + String::toString(nr),
                fnname, clname, LogMessage::DEBUG2);
#endif
            /* Number of points into first column */
            info(0, nselrows) = nr;
            info(1, nselrows) = rowcount;
   
#if LOG0
            ostringstream os;
            os  << "rowcount : " << info(nselrows, 1);
            log->out(os, fnname, clname, LogMessage::DEBUG2);
#endif
   
            /* Look into the table at row 'nr' and fill in the user columns */
            for(Int i=2; i< ncoll; i++) 
            {
               ctens[i-2].get(nr, tmpval);
               info(i, nselrows) = tmpval;
#if LOG0               
               ostringstream os;
               os << collist[i] << " : " << info(i, nselrows); 
               log->out(os, fnname, clname, LogMessage::DEBUG2);
#endif
            }
            nselrows++;
         }
      }
   }
   
   
    /******************************/
#if LOG0   
   log->out( "Done", fnname, clname, LogMessage::DEBUG2);
#endif

   /* Resize to actually used size... */

   info.resize(IPosition(2,ncoll,nselrows),True);
   cpol.resize(nselrows, True);

#if LOG0
   {
      ostringstream os;
      os << "setlect flag candidate:\n" << cpol;
      log->out(os, fnname, clname, LogMessage::DEBUG2);
   }
#endif

#if LOG0
   {
      ostringstream os;
      os << info;
      log->out(os, fnname, clname, LogMessage::DEBUG2);
   }
#endif

#if LOG0
   log->FnExit(fnname, clname);
#endif
   return 0;
}

Matrix<Int> BasePlot::getLocatePolChan() {
   IPosition istart;
   IPosition iend;
   IPosition istride;
   Slicer fslice;
   Matrix<Int> polids(NPlots_p, 2);
   //cout << "nTens_p=" << nTens_p << endl;
   Int polCnt = 0;
   polids = 0;
   for (int z=0;z<nTens_p;z++) {
      if (isArrayCol_p[z]) {
         fslice = TENslices_p[z];
         istart = fslice.start();
         iend = fslice.end();
         istride = fslice.stride();
         for (Int l = istart[1]; l <= iend[1]; l += istride[1]) {
            for (Int k = istart[0]; k <= iend[0]; k += istride[0]) {
               if (polCnt < NPlots_p) {
                   polids(polCnt, 0) = k;
                   polids(polCnt, 1) = l;
                   polCnt++;
               }
            }
         } 
      }
   }
   return polids;
}

Matrix<String> BasePlot::getLocateIndices(Matrix<Int>& cmap)
{
   String fnname = "getLocateIndices(Matrix<Int>& cmap)";
   Matrix<String> npmap(NPlots_p, 2);

   IPosition istart;
   IPosition iend;
   IPosition istride;
   Slicer fslice;
   Int plotcounter=0;
   Array<Bool> flagit;

#if LOG0 
   log->out( String("nplots : ")+String::toString(NPlots_p)
             + " nTens_p=" + String::toString(nTens_p), 
            fnname, clname, LogMessage::DEBUG1);
#endif
   
   //cout << "nTens_p=" << nTens_p << endl;
   for(int z=0;z<nTens_p;z++) 
   {
     if( isArrayCol_p[z] )
     {
      fslice = TENslices_p[z];
      
      istart = fslice.start();
      iend = fslice.end();
      istride = fslice.stride();
      
#if LOG0
      ostringstream os;
      os  << "start : " << istart << "   end : " << iend 
          << "   stride : " << istride;
      log->out(os, fnname, clname, LogMessage::DEBUG1);
#endif

      if(istart.nelements()!=2) {
          //cout <<"WRONG DIM" << LogIO::POST
          npmap(plotcounter,0) = String("-");   
          npmap(plotcounter,1) = String("-");
          plotcounter++;
      }
      else {
         for(Int chan=istart[1];chan<=iend[1];chan+=istride[1])
            for(Int pol=istart[0];pol<=iend[0];pol+=istride[0])
            {
               if(plotcounter < NPlots_p)
               {
                  npmap(plotcounter, 0) = String::toString(pol);   
                  npmap(plotcounter, 1) = 
                     String::toString(cmap(chan, 0)) + String(":") +
                     String::toString(cmap(chan, 1)) + String(":") +
                     String::toString(cmap(chan, 2));
                  plotcounter++;
               }
            }
        }
     }
     else {
        npmap(plotcounter,0) = String("-");   
        npmap(plotcounter,1) = String("-");
        plotcounter++;
     }
   } 
#if LOG0
   ostringstream os;
   os << "npmap=\n" << npmap;
   log->out(os, fnname, clname, LogMessage::DEBUG2);
#endif
   return npmap;
}
/*********************************************************************************/

Matrix<String> BasePlot::getLocateIndices()
{
   String fnname = "getLocateIndices";
   Matrix<String> npmap(NPlots_p, 2);

   IPosition istart;
   IPosition iend;
   IPosition istride;
   Slicer fslice;
   Int plotcounter=0;
   Array<Bool> flagit;

#if LOG0 
   log->out( String("nplots : ")+String::toString(NPlots_p)
             + " nTens_p=" + String::toString(nTens_p), 
            fnname, clname, LogMessage::DEBUG1);
#endif
   
   for(int z=0;z<nTens_p;z++) 
   {
     if( isArrayCol_p[z] )
     {
      fslice = TENslices_p[z];
      
      istart = fslice.start();
      iend = fslice.end();
      istride = fslice.stride();
      
#if LOG0
      ostringstream os;
      os  << "start : " << istart << "   end : " << iend 
          << "   stride : " << istride;
      log->out(os, fnname, clname, LogMessage::DEBUG1);
#endif

      // Assuming that these indices are sane. 
      //i.e. both ends exist for all dimensions.
      
      if(istart.nelements()!=2) 
      {
          //cout <<"WRONG DIM" << LogIO::POST
          npmap(plotcounter,0) = String("-");   
          npmap(plotcounter,1) = String("-");
          plotcounter++;
      }
      else
      {
      switch(ReductionType_p[z])
      {
         case 0: /* no reduction */
            for(Int chan=istart[1];chan<=iend[1];chan+=istride[1])
            for(Int pol=istart[0];pol<=iend[0];pol+=istride[0])
            {
               if(plotcounter < NPlots_p)
               {
                  npmap(plotcounter, 0) = String::toString(pol);   
                  npmap(plotcounter, 1) = String::toString(chan);
                  plotcounter++;
               }
            }
            
            break;
         case 1: /* Scalar reduction : MEAN */
            npmap(plotcounter,0) = String::toString(istart[0]) + String(":") +
                         String::toString(iend[0]) + String(":") +
                         String::toString(istride[0]);
            npmap(plotcounter,1) = String::toString(istart[1]) + String(":") +
                         String::toString(iend[1]) + String(":") +
                         String::toString(istride[1]);
            plotcounter++;
            break;
         case 2: /* Vector reduction : MEANS - avg over cols (chan) */
            for(Int pol=istart[0];pol<=iend[0];pol+=istride[0])
            {
               npmap(plotcounter,0) = String::toString(pol);
               npmap(plotcounter,1) = 
                  String::toString(istart[1]) + 
                  String(":") +
                  String::toString(iend[1]) + 
                  String(":") +
                  String::toString(istride[1]);
               plotcounter++;
            }
            break;
         case 3: /* Vector reduction : MEANS - avg over rows (pol) */
            for(Int chan=istart[1];chan<=iend[1];chan+=istride[1])
            {
               npmap(plotcounter,0) = 
                  String::toString(istart[0]) + 
                  String(":") +
                  String::toString(iend[0]) + 
                  String(":") +
                  String::toString(istride[0]);
               npmap(plotcounter,1) = String::toString(chan);
               plotcounter++;
            }
            break;
         default: BasePlotError(String("Unsupported TaQL reduction"));
      }
      }
     }// end of if ArrayCol
     else
     {
      npmap(plotcounter,0) = String("-");   
      npmap(plotcounter,1) = String("-");
      plotcounter++;
        
     }
   } 
#if LOG0
   ostringstream os;
   os << "npmap=\n" << npmap;
   log->out(os, fnname, clname, LogMessage::DEBUG2);
#endif
   return npmap;
}

/*********************************************************************************/
/*********************************************************************************/
/********************** Private(protected) Functions *****************************/
/*********************************************************************************/
Int BasePlot::createXTENS(Vector<String> &datastr)
{
   String fnname = "createXTENS";
#if LOG0
   log->FnEnter(fnname + "(datastr)", clname);
#endif
   xtens_p.resize(nTens_p);

   /* Create TENS and traverse parse trees */
   for(Int i=0;i<nTens_p;i++) 
   {
      xtens_p[i] = RecordGram::parse(SelTab_p,datastr[i*2]);
      
      //if( (xtens_p[i].dataType() != TpDouble)  ) 
      if(!((xtens_p[i].dataType() == TpDouble) ||
           (xtens_p[i].dataType() == TpInt) || 
           (xtens_p[i].dataType() == TpFloat))) 
      {
         ostringstream dtype;
         dtype << xtens_p[i].dataType();
         BasePlotError(String("DataType of TaQL expression (") + 
               dtype + String(") is not plottable"));
      }
      
   }
#if LOG0
   log->FnExit(fnname, clname);
#endif
   return 0;
}
/*********************************************************************************/
Int BasePlot::createYTENS(Vector<String> &datastr)
{
   String fnname = "createYTENS";
#if LOG0
   log->FnEnter(fnname, clname);
#endif
   ytens_p.resize(nTens_p);
   /* Create TENS and traverse parse trees */
   nip_p=0;
   for(Int i=0;i<nTens_p;i++) 
   {
      ytens_p[i] = RecordGram::parse(SelTab_p,datastr[i*2+1]);

      //if( (ytens_p[i].dataType() != TpDouble) ) 
      if(!((ytens_p[i].dataType() == TpDouble) ||
           (ytens_p[i].dataType() == TpInt) ||
           (ytens_p[i].dataType() == TpFloat))) 
      {
         ostringstream dtype;
         dtype << ytens_p[i].dataType();
         BasePlotError(String("DataType of TaQL expression (") + 
              dtype + String(") is not plottable"));
      }
      
      IndCnt_p[i] = nip_p;/* since flags pertain only to y data */
      getIndices(ytens_p[i]);
   }
   
   if(colnames_p.nelements()==0)
   {
      BasePlotError(String("No valid Table columns found for the Y data"));
   }
#if LOG0
   log->FnExit(fnname, clname);
#endif
   return 0;
}

/*********************************************************************************/

/* Extract X data from the table */
Int BasePlot::getXData(TPConvertBase* conv, Bool dummyread)
{
   String fnname = "getXData";
#if LOG0
   log->FnEnter(fnname + "(conv, dummyread)", clname);
#endif
   Double xytemp;
   Array<Double> xtemp;
   TableExprNode *tten;
   Int row=0,z=0;
   
   if(dummyread)
   {
   //cout << "getXData------true" << endl;       
      Int nxplots=0;
      for(z=0;z<nTens_p;z++)
      {
         tten = &(xtens_p[z]);
         if(tten->isScalar())
         {
            tten->get(0,xytemp);
            Xshape_p[z] = IPosition(1,1);
         }
         else
         {
         ////   Xshape_p[z] = IPosition(1,1);
            tten->get(0,xtemp);
            Xshape_p[z] = xtemp.shape();
            // complain !
            //tten.get(0,xtemp);
            //IPosition shp = xtemp.shape();
            //if(shp.nelements()==2) Xshape_p[z] = xtemp.shape();
         }
         nxplots += Xshape_p[z].product();
      }
      xplotdata_p.resize(nxplots,NRows_p);  
      xplotdata_p.set(0.0);

#if LOG0
      log->out(String("getXData: nxplots=") + String::toString(nxplots) +
               " NRows_p=" + String::toString(NRows_p), 
               fnname, clname, LogMessage::DEBUG2);
#endif

#if LOG0
      {
         stringstream os;
         os << "Mem allocated for X: " 
            << nxplots*NRows_p*sizeof(Double)/(1024.0*1024.0) 
            << " MB ";
         log->out(os, fnname, clname, LogMessage::DEBUGGING);
      }
#endif
   }
   else
   {
      Timer ttt;
      ttt.mark();
      Int xp=0;
   //cout << "getXData------------" << endl;       
      for(row=0;row<NRows_p;row++)
      {
         xp=0;
         for(z=0;z<nTens_p;z++) 
         {
            xtemp.resize();
            tten = &(xtens_p[z]);
            if(tten->isScalar())
            {
               tten->get(row,xytemp);
               //xplotdata_p(z,row) = xytemp;
               xplotdata_p(z,row) = 
                     (Double)conv->Xconvert(xytemp,row,tabNum_p);
            }
            else
            {
               tten->get(row,xtemp);
               for (Int i=0;i<Xshape_p[z].product();i++)
                   xplotdata_p(xp++,row) = 
                     conv->Xconvert((xtemp.data()[i]),row,tabNum_p);
            }
         }// end of for z
      }// end of for row
   }
#if LOG0
   log->FnExit(fnname, clname);
#endif
   return 0;
}

/*********************************************************************************/

/* Extract Y data from the table */
Int BasePlot::getYData(TPConvertBase *conv, Bool dummyread)
{
   String fnname = "getYData";
#if LOG0
   log->FnEnter(fnname + "(conv, dummyread)", clname);
#endif
   Double xytemp;
   Array<Double> ytemp;
   TableExprNode *tten;
   Int yp=0, z=0;
   Int row=0;
   
   // Read and update shapes for this row, but do not fill data.
   if(dummyread) 
   {
      Int nyplots=0;
      for(z=0;z<nTens_p;z++)
      {
         ytemp.resize();
         
         tten = &(ytens_p[z]);
         if(tten->isScalar())
         {
	     //#if LOG0 
            log->out( "before scalar get", fnname, clname, 
                     LogMessage::DEBUGGING);
	    //#endif 
            tten->get(0,xytemp);
            Yshape_p[z] = IPosition(1,1);
         }
         else
         {
	     //#if LOG0
            log->out( "before vectro get", fnname, clname, 
                     LogMessage::DEBUGGING);
	    //#endif 
            tten->get(0,ytemp);
            Yshape_p[z] = ytemp.shape();
         }
         nyplots += Yshape_p[z].product();
      }
      yplotdata_p.resize(nyplots,NRows_p);   
      yplotdata_p.set(0.0);
      theflags_p.resize(nyplots,NRows_p);   
      theflags_p.set(False);
      rowflags_p.resize(NRows_p);      
      rowflags_p.set(False);
      NPlots_p = nyplots;

      //#if LOG0
      log->out("getYData: nyplots=" + String::toString(nyplots) +
               " NRows_p=" + String::toString(NRows_p), 
               fnname, clname, LogMessage::DEBUG2);
      //#endif

      //#if LOG0 
      {
         ostringstream os;
         os << "Mem allocated for Y: " 
            << nyplots*NRows_p*sizeof(Float)/(1024.0*1024.0) 
            << " MB\n"
            << "Mem allocated for F: " 
            << nyplots*NRows_p*sizeof(Bool)/(1024.0*1024.0) 
            << " MB ";
         log->out(os, fnname, clname, LogMessage::DEBUGGING);
      } 
      //#endif
   }
   else
   {
      for(row=0;row<NRows_p;row++)
      {
         yp=0; 
         for(z=0;z<nTens_p;z++) // nTens_p : number of TEN pairs
         {
            ytemp.resize();

            tten = &(ytens_p[z]);
            if(tten->isScalar())
            {
               tten->get(row,xytemp);
               yplotdata_p(yp++,row) = conv->Yconvert(xytemp,row,tabNum_p);
            }
            else
            {
               tten->get(row,ytemp);
               for (Int i=0;i<Yshape_p[z].product();i++){
                   yplotdata_p(yp++,row) = 
		       conv->Yconvert((ytemp.data()[i]),row,tabNum_p);
               }
            }
            
         }// end of for z
#if LOG0 
         if(yp != NPlots_p) 
             log->out("Again, NPlots is not correct ! ",
                      fnname, clname, LogMessage::WARN );
#endif 
      }//end of for row
      
   }
#if LOG0
   log->FnExit(fnname, clname);
#endif
   return 0;
}


/*********************************************************************************/
/* Extract Y data from the table for just one TEN and ROW : special case. */
Int BasePlot::reGetYData(Int tenid, Int row, TPConvertBase *conv)
{
   Double xytemp;
   Array<Double> ytemp;
   TableExprNode *tten;
   Int yp=0, z=0,n=0;
   String fnname = "reGetYData";
   
   //if(row<4) cout << row << " : regetY : " << LogIO::POST
   
   yp=0; 
   for(n=0;n<NPlots_p;n++)
      if(Map_p(n,2)==tenid) {
         yp=Map_p(n,1); 
         break;
      }
   
   tten = &(ytens_p[tenid]);
   if(tten->isScalar())
   {
      tten->get(row,xytemp);
      // SDJ: WARNING: This may cause issues sending in table number
      // 0 for all plots. 
      yplotdata_p(yp++,row) = conv->Yconvert(xytemp,row,tabNum_p);
   }
   else
   {
      ytemp.resize();

      tten->get(row,ytemp);
      for (Int i=0;i<Yshape_p[tenid].product();i++)
         // SDJ: WARNING: This may cause issues sending in table number
         // 0 for all plots. 
          
         yplotdata_p(yp++,row) = conv->Yconvert((ytemp.data()[i]),row,tabNum_p);
   }

   if(tenid == nTens_p-1 && yp != NPlots_p) 
        log->out( "wrong indexing", fnname, clname, LogMessage::WARN);
   if(tenid < nTens_p-1)
   {
      z=0;
      for(n=0;n<NPlots_p;n++)   
         if(Map_p(n,2)==tenid+1) {
            z=Map_p(n,1); 
            break;
         }
      if(z != yp) 
         log->out( "wrong indexing for z", fnname, clname, LogMessage::WARN );
   }
   
   return 0;
}


/*********************************************************************************/



/* Clean Up bookkeeping arrays */
Int BasePlot::cleanUp()
{
#if LOG0
   String fnname = "cleanUp";
   log->FnEnter(fnname, clname);
#endif
   nip_p=0;
   ipslice_p.resize(nip_p);
   colnames_p.resize(nip_p);
   nflagmarks_p=0;
   locflagmarks_p.resize(nflagmarks_p);

   callbackhooks_p = NULL;
   
   return 0;
}

/*********************************************************************************/

/* Given a TEN, get an index value - required by mspflagdata */
Int BasePlot::getIndices(TableExprNode &ten)
{
   const TableExprNodeRep* tenroot = (ten.getNodeRep());

//#if LOG0
   String fnname = "getIndices";
   ostringstream os;
   os << "Call ptTravers, tenroot=" << tenroot;
   log->out(os, fnname, clname, LogMessage::DEBUGGING);
//#endif
   ptTraverse(tenroot); 

   return 0;
}

/*********************************************************************************/

/* Recursive Parse Tree traversal */
void BasePlot::ptTraverse(const TableExprNodeRep *tenr)
{
   String fnname ="ptTraverse";
#if LOG0
       log->FnEnter(fnname, clname);
#endif

   if(tenr == (TableExprNodeRep*)NULL) {
#if LOG0
       log->out("NULL tenr", fnname, clname, LogMessage::DEBUGGING);
#endif
       return;
   }
#if LOG0
   ostringstream os;
   os  << "dat : " << tenr->dataType() 
       << " , val : " << tenr->valueType() << " , op : " 
       << tenr->operType();
   log->out(os, fnname, clname, LogMessage::DEBUGGING);
#endif 
   /* Check to see what type this node is */
   Int etype = -1;
   
   if(dynamic_cast<const TableExprNodeBinary*>(tenr)!=NULL) {
      etype=3; 
#if LOG0
      log->out(" ##########***********It's Binary",
          fnname, clname, LogMessage::DEBUGGING);
#endif
   }
   if(dynamic_cast<const TableExprNodeMulti*>(tenr)!=NULL) {
      etype=2; 
#if LOG0
      log->out(" ##########********It's Multi",
          fnname, clname, LogMessage::DEBUGGING);
#endif
   }
   if(dynamic_cast<const TableExprNodeSet*>(tenr)!=NULL) {
      etype=1; 
#if LOG0
      log->out(" ##########***********It's Set",
          fnname, clname, LogMessage::DEBUGGING);
#endif
   }
   if(dynamic_cast<const TableExprNodeSetElem*>(tenr)!=NULL) {
      etype=0; 
#if LOG0
      log->out(" ##########***********It's SetElem",
          fnname, clname, LogMessage::DEBUGGING);
#endif
   }
   if(dynamic_cast<const TableExprFuncNodeArray*>(tenr)!=NULL) {
      etype=4; 
#if LOG0
      log->out(" ##########***********It's Func node",
         fnname, clname, LogMessage::DEBUGGING); 
#endif
   }

   switch(etype)
   {
      case 0: /*  SetElem */
      {
#if LOG0
         /* Act on this node */
         log->out("SetElem Node : ",
                fnname, clname, LogMessage::DEBUGGING); 
        
         const TableExprNodeSetElem *tense = 
              dynamic_cast<const TableExprNodeSetElem*>(tenr);

         /* get children */
         log->out("Start ---> ", fnname, clname, LogMessage::DEBUGGING); 
         ptTraverse(tense->start());
         log->out("Increment ---> ", fnname, clname, LogMessage::DEBUGGING);
         ptTraverse(tense->increment());
         log->out("End ---> ", fnname, clname, LogMessage::DEBUGGING); 
         ptTraverse(tense->end());
#endif
         break;
      }
      case 1: /* Set */
      {
         const TableExprNodeSet *tens=
             dynamic_cast<const TableExprNodeSet*>(tenr);
         /* Act on this node */
#if LOG0 
         log->out("Set Node : ", fnname, clname, LogMessage::DEBUGGING); 
#endif  
         /* get children */
         for(Int i=0;i<(Int)tens->nelements();i++)
         {
            const TableExprNodeSetElem tenser = (*tens)[i];
#if LOG0
            ostringstream os;
            log->out(" Set[" << i << "] start ---> ",
                 fnname, clname, LogMessage::DEBUGGING);   
#endif
            ptTraverse((*tens)[i].start());

#if LOG0
            log->out(" Set[" << i << "] increament ---> ",
                 fnname, clname, LogMessage::DEBUGGING);   
#endif
            ptTraverse((*tens)[i].increment());

#if LOG0
            log->out(" Set[" << i << "] end ---> ",
                 fnname, clname, LogMessage::DEBUGGING);   
#endif
            ptTraverse((*tens)[i].end());
         }
         break;
      }
      case 2: /* Multi */
      {
         const TableExprNodeMulti *tenm=
            dynamic_cast<const TableExprNodeMulti*>(tenr);
         /* Act on this node */
#if LOG0 
         log->out("Multi Node : ", fnname, clname, LogMessage::DEBUGGING);
#endif 
         
         const TableExprNodeIndex* nodePtr = 
              dynamic_cast<const TableExprNodeIndex*>(tenr);
         if(nodePtr!=0)
         {
            if (nodePtr->isConstant())//  &&  nodePtr->isSingle()) 
            {
               const Slicer& indices = nodePtr->getConstantSlicer();
               // Extract the index from it.
#if LOG0
               ostringstream os;
               os << "M Index start: " << indices.start();
               os << "\nM Index end: " << indices.end();
               os << "\nM Index stride: " << indices.stride();
               log->out(os, fnname, clname, LogMessage::DEBUGGING); 
#endif               
               //ipslice_p.resize(nip_p+1,True);
               //ipslice_p[nip_p] = indices;
               //nip_p++;
               
               if((Int) ipslice_p.nelements() == nip_p) 
                  ipslice_p[nip_p-1] = indices;
               
            }

         }
           
         /* get children */
         PtrBlock<TableExprNodeRep*> tenrarr = tenm->getChildren();
#if LOG0
         ostringstream os;
         os << "Num elements : " << tenrarr.nelements();
#endif
         for(Int i=0;i<(Int)tenrarr.nelements();i++)
         {
#if LOG0
            os << "\nChild " << i << " ---> " ; 
#endif
            ptTraverse(tenrarr[i]);
         }
         break;
      }
#if LOG0
         log->out(os, fnname, clname, LogMessage::DEBUGGING); 
#endif
      case 3: /* Binary */
      {
         String cname;
         const TableExprNodeBinary *tenb=
               dynamic_cast<const TableExprNodeBinary*>(tenr);
         /* Act on this node */
#if LOG0
         log->out("Binary Node : ", fnname, clname, LogMessage::DEBUGGING); 
#endif
         
         const TableExprNodeArrayColumn *tenac = 
            dynamic_cast<const TableExprNodeArrayColumn*>(tenb);
         if(tenac != 0){
            cname = tenac->getColumn().columnDesc().name();
#if LOG0 
            ostringstream os;
            os << " Array Column Name : " << cname
                << "\n Num elems : " << colnames_p.nelements();
            log->out(os, fnname, clname, LogMessage::DEBUGGING); 
#endif 
            colnames_p.resize(nip_p+1,True); // small array of strings
            colnames_p[nip_p] = cname;
            ipslice_p.resize(nip_p+1,True);
            ipslice_p[nip_p] = Slicer(IPosition(2,0,0),
                                      IPosition(2,-2147483646,-2147483646));
            nip_p++;
         }

         
         const TableExprNodeColumn *tenc = 
            dynamic_cast<const TableExprNodeColumn*>(tenr);
         if(tenc != 0) {
            cname =  tenc->getColumn().columnDesc().name() ;
#if LOG0 
            log->out(String(" Column Name : ")+cname,
                    fnname, clname, LogMessage::DEBUGGING);
#endif 
            colnames_p.resize(nip_p+1,True); // small array of strings
            colnames_p[nip_p] = cname;
            ipslice_p.resize(nip_p+1,True);
            ipslice_p[nip_p] = Slicer(IPosition(1,0),IPosition(1,0));
            nip_p++;
         }
         

         /*   
         const TableExprNodeArrayPart* nodePtr = 
            dynamic_cast<const TableExprNodeArrayPart*>(tenr);
         if (nodePtr != 0) {
            // The node represents a part of an array; get its index node.
            const TableExprNodeIndex* inxNode = nodePtr->getIndexNode();
            // If a constant index accessing a single element,
            // get the Slicer defining the index.
            if (inxNode->isConstant() )// &&  inxNode->isSingle()) 
            {
               const Slicer& indices = inxNode->getConstantSlicer();
               // Extract the index from it.
               os << LogOrigin( "BasePlot", "ptTraverse", WHERE )
                  << LogIO::NORMAL3
                  << "B Index start: " << indices.start()
                  << "\nB Index end: " << indices.end()
                  << "\nB Index stride: " << indices.stride() << LogIO::POST
            }
         }
         */

           /* get left and right children */
         ptTraverse(tenb->getLeftChild());
         ptTraverse(tenb->getRightChild());
         
         break;
      }
      case 4: /* FuncNodeArray */
      {
         const TableExprFuncNodeArray *tefna=
             dynamic_cast<const TableExprFuncNodeArray*>(tenr);
         const TableExprFuncNode *tefn = tefna->getChild();
         
         //const TableExprNodeMulti *tenm=
         //       dynamic_cast<const TableExprNodeMulti*>(tenr);
         /* Act on this node */
#if LOG0
         log->out("Func Node Array : ", fnname, clname, LogMessage::DEBUGGING); 
#endif 
         //tenm->show(cout,1);
         
         const TableExprNodeIndex* nodePtr = 
            dynamic_cast<const TableExprNodeIndex*>(tefn);
         if(nodePtr!=0)
         {
            if (nodePtr->isConstant())//  &&  nodePtr->isSingle()) 
            {
#if LOG0 
               const Slicer& indices = nodePtr->getConstantSlicer();
               // Extract the index from it.
               ostringstream os;
               os << "F Index start: " << indices.start();
               os << "\nF Index end: " << indices.end();
               os << "\nF Index stride: " << indices.stride();
               log->out(os, fnname, clname, LogMessage::DEBUGGING); 
#endif 
            }

         }
           
         /* get children */
         PtrBlock<TableExprNodeRep*> tenrarr = tefn->getChildren();
         ostringstream os;
#if LOG0
         os << "Num elements : " << tenrarr.nelements();
#endif
         for(Int i=0;i<(Int)tenrarr.nelements();i++)
         {
#if LOG0
            os << " Child " << i << " " ; 
#endif
            ptTraverse(tenrarr[i]);
         }
#if LOG0
         log->out(os, fnname, clname, LogMessage::DEBUGGING); 
#endif
         break;
      }
     }// end of switch
#if LOG0
       log->FnExit(fnname, clname);
#endif
}

void BasePlot::showFlags() {
     //cout << "theflags_p=" << theflags_p << endl;
     //cout << "avgtheflags_p=" << avgtheflags_p << endl;
}

Bool BasePlot::saveData(const String& filename) {
    //cout << "BP saveData " << filename << endl;

    RegularFile dataFile(filename);
    if (dataFile.exists()) {
        dataFile.setPermissions(0666);
        //if (dataFile.isRegular()) {
        //   cout << filename << " exists and is a regular file" << endl;
        //}
    }
    else if (!dataFile.exists()) {
        //if (!dataFile.canCreate()){
        //    cout << "cannot create this file" << endl;
        //}
        dataFile.create(False);
    }

    //LargeRegularFileIO file1(dataFile, ByteIO::Append);
    //cout << "file1 created" << endl;
    //Timer timer;
    //AipsIO stream (&file1);
    //cout << "stream created" << endl;
    //for (Int i=0; i<100000000; i++) {
    //    stream << i ;
    //}

    ofstream out(filename.data(), fstream::app); 
    if (!out) { 
       cout << "Cannot open file.\n"; 
       return 1; 
    } 
    
    //cout << "NRows_p=" << NRows_p << " NPlots_p=" << NPlots_p << endl;
    for (int nr = 0; nr < NRows_p; nr++) {
       for (int np = 0; np < NPlots_p; np++) {
          if (showflags_p == getYFlags(np, nr)) {
             out << "\n" 
                 << std::setprecision(16) << getXVal(np, nr)
                 << " "
                 << std::setprecision(16) << getYVal(np, nr);
              //   << " "
              //   << std::setprecision(16) << getYFlags(np, nr);
              //out << "\n" << xplotdata_p(Map_p(np, 0), nr)
              //    << " " << yplotdata_p(Map_p(np, 1), nr);
          }
       }
    } 
 
    out.close(); 

    //timer.show ("LargeRegularFileIO write");

    return True;
}

/*********************************************************************************/
/* Get Flags */
/* showflags=True MUST be accompanied by the appropriate TaQL with FLAG */
Int BasePlot::getFlags(String flagversion, Bool showflags)
{
   String fnname = "getFlags";
   Int fyp;
   Slicer fslice;
   Array<Bool> flagit; 
   Vector<Float> vfcount;
   Vector<Float> vtcount;
   Vector<Bool> vsflag;
   IPosition fshp;
   Bool sflag;
   Bool row;
   
   Int rc;
   row = False;

   /*** get the right flag version **/
   if(FV)
   {
      // If version has not changed... then don't re-attach.....
      if( flagversion.length() && ! currentflagversion_p.matches(flagversion) )
      {
         log->out(String("getting flag version ")+flagversion,
              fnname, clname, LogMessage::DEBUGGING); 
         FV->attachFlagColumns( flagversion, RowFlags_p, Flags_p, SelTab_p);
         currentflagversion_p = flagversion;
      }
   }
   /** gotten the right flag version **/
   
   //*****/////if(frcol_p) rowflags_p = RowFlags_p.getColumn();
   if(frcol_p)
   {
       rowflags_p = RowFlags_p.getColumn();
       //rowflags_p = False;
   }
        
   fyp=0; 
   for(int z=0;z<nTens_p;z++)
   {
      if( isArrayCol_p[z] && fcol_p && 
          (TENcolshapes_p[z].isEqual(FlagColShape_p)))      
      {
      row = False;
   
      Float fcount=0.0,tcount=0.0;
      Int ff=0;
      Int zp,cnt;
      Bool reget=False;
                Int npol=0;
      
      /* Get the shape of the (sub)Array that has been touched by the TaQL */
         fslice = TENslices_p[z];
         fshp = fslice.length();
      // Use getColumnRange later.
      flagit.resize();

      /////////////////////////////////////////////////

      //cout << "BasePlot::getFlags fslice=" << fslice << endl;

      //cout << "BasePlot::getFlags---Flags_p.getColumn"<< endl;
         // [ row, col, nrows ] : row : innermost.
         flagit = Flags_p.getColumn(fslice); 
      // cout << "BasePlot::getFlags===Flags_p.getColumn"<< endl;


      /////////////////////////////////////////////////


         TENRowColFlag_p[z].set(True);
   
      /* Read in flags, depending on the type of reduction that has happened */
      switch(ReductionType_p[z])
      {
      /* This is for when a subset of chans,pols is
      chosen - but without any reduction. Still a one-to-one mapping. */
      case 0: 
         cnt=0;
         for(rc=0;rc<NRows_p;rc++)
         {
            zp = fyp;
            for (Int i=0;i<fshp.product();i++)
            {
               theflags_p(zp+i,rc) = (flagit.data()[cnt]);
               TENRowColFlag_p[z][i] &= flagit.data()[cnt];
               cnt++;
            }
         }
         fyp += fshp.product();
         break;
          /* A scalar reduction results in the TaQL result being a scalar, but
             the FLAG is still an Array. Collapse flags from this Array to a
             single value */
      case 1:
         log->out(String("z=")+String::toString(z) + " scalar reduction",
                  fnname, clname, LogMessage::DEBUGGING); 
         cnt=0;
         for(rc=0;rc<NRows_p;rc++)
         {
            zp = fyp;
            fcount=0.0,tcount=0.0;
            sflag = ! showflags; // True for showflags = False
            for (Int i=0;i<fshp.product();i++)
            {
               TENRowColFlag_p[z][i] &= flagit.data()[cnt];

               if( showflags == False )
               {
                  /* Count and accumulate for unflagged points */
                  sflag &= flagit.data()[cnt];
                  fcount += (Int)(!(flagit.data()[cnt]));
               }
               else
               {
                  /* Count and accumulate for flagged points */
                  sflag |= flagit.data()[cnt];
                  fcount += (Int)((flagit.data()[cnt]));
               }
               tcount++;
               cnt++;
            }
            theflags_p(zp,rc) = sflag;
            if(firsttime_p) 
            {
               if( showflags == False )flagsum_p(rc,z) = (Int)fcount;
               else flagsum_p(rc,z) = (Int)(tcount - fcount);

               if(doscalingcorrection_p && fcount!=0 && tcount!=0) 
                  //correct yplotdata_p !
                  yplotdata_p(zp,rc) = yplotdata_p(zp,rc)*tcount/fcount;
            }
            else
            {
               reget = False;
               if( showflags == False && flagsum_p(rc,z) != (Int)fcount ) 
                   reget = True;
               if(showflags == True && flagsum_p(rc,z) != (Int)(tcount-fcount))                    reget = True;
               if(reget)
               {
                  //cout << "reget for z : " << z << " and rc : " 
                  //     << rc << LogIO::POST
                  reGetYData(z,rc,conv_p);
                  if(doscalingcorrection_p && fcount!=0 && tcount!=0) 
                      //correct yplotdata_p !
                  {
                     yplotdata_p(zp,rc) = yplotdata_p(zp,rc)*tcount/fcount;
                  }
                  if( showflags == False )flagsum_p(rc,z) = (Int)fcount;
                  else flagsum_p(rc,z) = (Int)(tcount - fcount);
               }
            }
         }
         fyp++;

         // first time = True : fill in flag_sum_p
         // first time = False : compare with flag_sum_p[rc]. 
         //if changed, re-read y.
         
         break;
         /* A vector reduction has happened. The TaQL result and therefore
            shape of theflags_p is both a subset of the FLAG Array, and also
            has one dimension collapsed. 
         */
         /* Flag subsets from from FLAG Array have to be collapsed in one
            dimension, and then filled into theflags_p 
         */
      case 2:
      case 3:
         /* ReductionType_p = 2 -> collapse along 
            chan(col:1) -> shape to accumulate on : (fshp[0])
            ReductionType_p = 3 -> collapse along 
            pol(row:0) -> shape to accumulate on : (fshp[1])
            => shape to accumulate on = ( ReductionType_p - 2 )
         */
         {
           ostringstream os;
           os  <<"z : " << z << " : In case 2,3 for redtype : "
               << ReductionType_p[z];
           log->out(os, fnname, clname, LogMessage::DEBUGGING); 
         }
         
         vfcount.resize(fshp[ReductionType_p[z]-2]); 
         vtcount.resize(fshp[ReductionType_p[z]-2]); 
         vsflag.resize(fshp[ReductionType_p[z]-2]);
         cnt=0;
         npol=fshp[0];
         for(rc=0;rc<NRows_p;rc++)
         {
            zp = fyp;
            vfcount.set(0.0);
            vtcount.set(0.0);
            vsflag.set( !showflags );
            ff=0;
            for (Int i=0;i<fshp.product();i++)
            {
               TENRowColFlag_p[z][i] &= flagit.data()[cnt];

               if( showflags == False )
               {
                  /* Count and accumulate for unflagged points */
                  vsflag[ff] &= flagit.data()[cnt];
                  vfcount[ff] += (Int)(!(flagit.data()[cnt]));
               }
               else
               {
                  /* Count and accumulate for flagged points */
                  vsflag[ff] |= flagit.data()[cnt];
                  vfcount[ff] += (Int)((flagit.data()[cnt]));
               }

               vtcount[ff]++;
               cnt++;
               if(ReductionType_p[z]==2)
               { 
                  if( (i+1)%npol==0 ) 
                      ff=0;
                  else 
                      ff++;
               }
               if(ReductionType_p[z]==3 && (i+1)%npol==0) 
                  ff++;
            }
            
            for(Int i=0;i<(Int)vtcount.nelements();i++) 
               theflags_p(zp+i,rc) = vsflag[i];
#if LOG0               
            if(rc<4) { 
               ostringstream os;
               os << rc << " : about to do flagsum : vfcount : " << vfcount;
               log->out(os, fnname, clname, LogMessage::DEBUGGING);    
            }
#endif
            
            if(firsttime_p) 
            {
               if( showflags == False )flagsum_p(rc,z) = (Int)sum(vfcount);
               else flagsum_p(rc,z) = (Int)(sum(vtcount) - sum(vfcount));
               
               for(Int i=0;i<(Int)vtcount.nelements();i++)
               {
                  if(doscalingcorrection_p && vfcount[i]!=0 && vtcount[i]!=0)
                     yplotdata_p(zp+i,rc) = 
                        yplotdata_p(zp+i,rc)*vtcount[i]/vfcount[i];
               }
            }
            else
            {
               reget = False;
               if( showflags == False && 
                   flagsum_p(rc,z) != (Int)sum(vfcount) ) reget = True;
               if( showflags == True && 
                   flagsum_p(rc,z) != (Int)(sum(vtcount)-sum(vfcount)) ) 
                     reget = True;
               if(reget)
               {
                  reGetYData(z,rc,conv_p);
                  for(Int i=0;i<(Int)vtcount.nelements();i++)
                  {
                     if(doscalingcorrection_p && vfcount[i]!=0 && vtcount[i]!=0)
                        yplotdata_p(zp+i,rc) = 
                          yplotdata_p(zp+i,rc)*vtcount[i]/vfcount[i];
                  }
                  if( showflags == False )
                     flagsum_p(rc,z) = (Int)sum(vfcount);
                  else 
                     flagsum_p(rc,z) = (Int)(sum(vtcount) - sum(vfcount));
               }
            }
         }
         fyp += fshp[ReductionType_p[z]-2];

         break;
      
      default: BasePlotError(String("Unsupported TaQL reduction"));
      
      }// end of switch
      }// end of if FLAG is to be used
      // fill up from the RowFlags. This is triggered if it's a Scalar Col.
      // also triggered when the shape of the y data column does not match 
      // the flag column shape (i.e. for a uvcoverage plot)
      else 
      {
         if( frcol_p )
         {
            for(rc=0;rc<NRows_p;rc++)
              theflags_p(fyp,rc) = rowflags_p[rc];
            fyp++;
         }
      }
             
   }// end of for z

   
   if(fyp > 0 && fyp != NPlots_p) 
      log->out("NPlots is not correct (1)", 
             fnname, clname, LogMessage::WARN );
   ////TODO :  This is never going to be correct - 
   //if fcol_p or frcol_p are False !!!

   /* If *only* ROW_FLAG exists, use it for all vals.. */
   if(frcol_p && !fcol_p)
   {
      fyp=0;
      for(Int z=0;z<nTens_p;z++)
      for(Int i=0;i<NPlots_p;i++) 
      // need this because multiple chans/pols are in the same TEN
      {
         if(Map_p(i,2)==z) // all vals for this TEN
         {
            for(rc=0;rc<NRows_p;rc++)
               theflags_p(fyp,rc) = rowflags_p[rc];
            fyp++;
         }
      }
   }
   if(fyp > 0 && fyp != NPlots_p) 
       log->out( "NPlots is not correct (2)", 
             fnname, clname, LogMessage::WARN );
   
   firsttime_p=False;
   
   return 0;
}


///////////flag data for averaged ///////////////////////////////
//
//Flags_p.putColumn(fslice, flagit); 
//RowFlags_p.putColumn(rowflags_p);
//SelTab_p.flush();
// 
// Flag holders...
//      ArrayColumn<Bool> Flags_p, AvgFlags_p;
//      ScalarColumn<Bool> RowFlags_p, AvgRowFlags_p;
//      String FlagColName_p, FlagRowName_p;
//      IPosition FlagColShape_p;
//      Bool fcol_p,frcol_p;
//////////////////////////////////////////////////////////////////////
Int BasePlot::flagData(Int direction, String msname, String spwexpr,
                       Matrix<Int>& rowMap, Matrix<Int>& chanMap,
                       String ext)
{

   String fnname = "flagData";
   log->FnEnter(fnname + "(direction, msname)", clname);
   // direction:  0=unflag, 1=flag
   flagdirection_p = direction;

   //cout << "msname=" << msname << endl;
   //cout << "extendflag=" << ext << endl;
   //cout << "spwexpr=" << spwexpr << endl;
   
   /////////////////getFlags("main", msname);
   itsTabName_p = msname;
   //if (itsTab_p != NULL) {
   //   delete itsTab_p;
   //   itsTab_p = NULL;
   //}

   /* 
   #include <ms/MSSel/MSSelection.h>
   #include <ms/MeasurementSets/MeasurementSet.h>
   MS itsSelectedMS; 
   try {
      MS itsMS(itsTabName_p, casa::Table::Update);
      itsSelectedMS = MS(itsMS);
      MSSelection MSSelectionObj(itsMS, MSSelection::PARSE_NOW,
                                 "", "", "",
                                 spwexpr, "", "", "","", "");

      MSSelectionObj.getSelectedMS(itsSelectedMS,String(""));
      //cout << "itsSelectedMS.nrow()=" << itsSelectedMS.nrow() << endl; 
      itsTab_p = &itsSelectedMS;
   }
   catch (...) {
      log->out("Can not open MS to write flags",
               fnname, clname, LogMessage::WARN); 
      return 0;
   }
   */

   Table itsSelectedMS; 
   uInt selRows = 0;
   try {
      Table itsMS(itsTabName_p, casa::Table::Update);
      //cout << "itsMS.nrow()=" << itsMS.nrow() << endl; 

      //cout << "spwexpr=" << spwexpr << endl;
      String command = String("select from ") + itsTabName_p +
                       String(" where DATA_DESC_ID in [") + spwexpr +
                       String("]");  
      //cout << "command=" << command << endl;

      itsSelectedMS = tableCommand(command); 
      selRows = itsSelectedMS.nrow();
      //cout << "itsSelectedMS.nrow()=" << selRows << endl; 

      itsTab_p = &itsSelectedMS;
   }
   catch(AipsError &y) {
      log->out(y.getMesg(), fnname, clname, LogMessage::WARN); 
      return 0;
   }
   catch (...) {
      log->out("Can not open MS to write flags",
               fnname, clname, LogMessage::WARN); 
      return 0;
   }

   Vector<uInt> rownrs = itsTab_p->rowNumbers();

   //try {
   //   itsTab_p = new Table(itsTabName_p, casa::Table::Update);
   //}
   //catch (...) {
   //   log->out("Can not open MS to write flags",
   //            fnname, clname, LogMessage::WARN); 
   //   return 0;
   //}
   log->out("MS open for flagging", fnname, clname, LogMessage::DEBUG2); 
   //log->out("MS open for flagging", fnname, clname, LogMessage::NORMAL, True); 

   Array<Bool> avgflagit; 
   Vector<Bool> vsflag;
   IPosition fshp;
   Bool row;
   row = False;

   //if (FV) {
   //   if(flagversion.length() && 
   //        !currentflagversion_p.matches(flagversion))
   //   {
   //      log->out(String("getting flag version ")+flagversion,
   //           fnname, clname, LogMessage::DEBUG2); 
   //      FV->attachFlagColumns(flagversion, AvgRowFlags_p, 
   //                            AvgFlags_p, *itsTab_p);
   //      currentflagversion_p = flagversion;
   //   }
   //}
   //
  
   AvgFlags_p.attach(*itsTab_p, "FLAG");
   AvgRowFlags_p.attach(*itsTab_p, "FLAG_ROW");

   try {
      avgrowflags_p = AvgRowFlags_p.getColumn();
      //cout << "rowflags=" << avgrowfalgs_p <<endl;
   }
   catch(AipsError &e) {
      log->out(String(e.getMesg()),
               fnname, clname, LogMessage::WARN); 
      return 0;
   }
   catch(...) {
      log->out("Can not get flag column for flagging",
               fnname, clname, LogMessage::WARN); 
      return 0;
   }

   try {
      avgflagit.resize();
      avgflagit = AvgFlags_p.getColumn();
      //cout <<"avgflagit=" <<avgflagit.shape() <<endl;
   }
   catch(...) {
      log->out("Can not get flag column for flagging",
               fnname, clname, LogMessage::WARN); 
      return 0;
   }

   IPosition avgIp = avgflagit.shape();
   avgtheflags_p.resize(avgIp(0) * avgIp(1), avgIp(2));   
   //avgtheflags_p.set(False);

   Int cnt = 0;
   for (Int rc = 0; rc < avgIp(2); rc++) {
      for (Int i = 0; i < avgIp(0) * avgIp(1); i++) {
         avgtheflags_p(i, rc) = (avgflagit.data()[cnt]);
         cnt++;
      }
   }
   
   //Int nt = 0;
   //for (Int rc = 0; rc < 2; rc++) {
   //   for (Int i = 0; i < avgIp(0) * avgIp(1); i++) {
   //      cout << "avgflagit.data[" << nt << "]=" 
   //           << (avgflagit.data()[nt]) << endl ;
   //      nt++;
   //   }
   //}

   //Int ct = 0;
   //for (Int rc = 0; rc < 2; rc++) {
   //   for (Int i = 0; i < avgIp(0) * avgIp(1); i++) {
   //      cout << "ct=" << ct
   //           << " avgtheflags_p("<< i << ", " << rc << ")=" 
   //           << avgtheflags_p(i, rc) << endl ;
   //      ct++;
   //   }
   //}

   //////////////////////////////////////////
   numflagpoints_p = 0;
   try {
    Int fyp;
    Slicer fslice;
    Array<Bool> flagit; 
    Bool row;
    IPosition fshp;
 
    //Int ff;
    Int zp, rc;
    //Int cnt;
    row = False;
    //Int npol=0;

    //###ReductionType=[0], nTens_p=1
    //cout << "ReductionType=" << ReductionType_p
    //     << " nTens_p=" << nTens_p
    //     << endl;

    Matrix<Int> polids = getLocatePolChan();
    //cout << "polids=" << polids << endl;
  
    fyp=0;      
    for (int z=0; z < nTens_p; z++) {

     //###fcol_p=1, FLAG column exists
     //###flageColShape_p=[nPol, nChan]
     //cout      << "BasePlot info=" << getBasePlotInfo() 
     //      << " TENcolshapes_p["<< z << "]=" << TENcolshapes_p[z] 
     //      << " fcol_p=" << fcol_p 
     //      << " FlagColShape_p=" << FlagColShape_p 
     //      << " isArrayCol_p["<< z << "]=" << isArrayCol_p[z] << endl; 

     if (isArrayCol_p[z] && fcol_p && 
         TENcolshapes_p[z].isEqual(FlagColShape_p)) {

      fslice = TENslices_p[z];
      fshp = fslice.length();

      flagit.resize();
      flagit = Flags_p.getColumn(fslice); 

      //fslice should [pol_0, chan_0] to [pol_n, chan_m] 
      //with stride [1, 1] length [nPol, mChan]
      //each slice contains an [nPol, mChan] matrix of flags 
      //flagit is a [nPol, nChan, nRow] cube of flags
      //theflags is a [nPol x nChan, nRow] Matrix of flags 
      //cout << " fslice="<< fslice <<  " fshp=" << fshp 
           //<< " flagit=" << flagit 
      //     << " flagit IP=" << flagit.shape() 
           //<< " theflags_p=" << theflags_p 
      //     << endl;

      //cout << "rmap=" << rowMap; 
      //cout << "cmap=" << chanMap;
      //cout << "rmap=" << rowMap.shape() << endl;
      //cout << "cmap=" << chanMap.shape() << endl;
      //cout << "avgtheflags_p=" << avgtheflags_p.shape() << endl;
      //cout << "avgflagit_p=" << avgflagit.shape() << endl;

      if (ReductionType_p[z] == 0) {

        cnt = 0;
        for (rc = 0; rc < NRows_p; rc++) {
		    
         zp = fyp;
         for (Int i = 0; i < fshp.product(); i++) {
           Int zpi = zp + i;

           if (selectedPoint(zpi, rc)) {

             Vector<Int> polt(fshp(0));
             Int pCns = 0;
             if (ext.contains("P")) {
              for (Int i = 0; i < fshp(0); i++) {
               polt(pCns++) = i;
              }
             }
             else {
              polt(pCns++) = polids(zpi, 0);
             }
             polt.resize(pCns, True);
             //cout << "polt=" << polt << endl;

             Vector<Int> chant(fshp(1));
             Int cCns = 0;
             if (ext.contains("C")) {
              for (Int i = 0; i < fshp(1); i++) {
               chant(cCns++) = i;
              }
             }
             else {
              chant(cCns++) = polids(zpi, 1);
             }
             chant.resize(cCns, True);
             //cout << "chnt=" << chant << endl;
              
             for (uInt i = 0; i < chant.nelements(); i++) {
              Int d = chant(i);
              for (uInt j = 0; j < polt.nelements(); j++) {
               Int pol = polt(j);
               Int s = d * fshp(0) + pol % fshp(0);
  
               //cout  
               //  << " pol=" << pol
               //  << " chan=" << d
               //  << " pos=" << s  
                 //<< " flag dir=" << direction
                 //<< " oldflag=" << theflags_p(s, row)
               //  << endl;
  
               theflags_p(s, rc) = (Bool)direction;
               Int srow = rc * fshp.product() + s;
               //cout << "cnt=" << cnt << " srow=" << srow << endl; 
               (flagit.data()[srow]) = direction ? 
                  theflags_p(s, rc) | (flagit.data()[srow]) :
                  theflags_p(s, rc) & (flagit.data()[srow]);
               if (!direction) {
                rowflags_p[rc] = direction;  
               }
               numflagpoints_p++;
              }
             }
             
      
             Int row;
             Int srow;
             Int msrow = rowMap.shape()(0);

             //srow = 0;
             //cout << "msrow=" << msrow << endl;
             //for (Int m = 0; m < msrow; m++) {
             //   if (rowMap(m, 0) == rc) { 
             //      srow++;
             //      cout << srow << " " << rowMap(m, 0) 
             //           << " " << rowMap(m, 1) << endl;
             //   }
             //}

             for (Int m = 0; m < msrow; m++) {
              if (rowMap(m, 0) == rc) { 

               //ask rowMap for row and spwid for this aved row
               uInt rootRow = rowMap(m, 1);
               Int sp = rowMap(m, 2);

               row = -1;
               for (uInt r = 0; r < rownrs.nelements(); r++) {
                   if (rownrs[r] == rootRow) {
                      row = r;
                      break;
                   }
               }

               if (row < 0 || row >= (Int)selRows) {
                  continue;
               }

               if (!direction) {
                  avgrowflags_p(row) = 0;
               }

               //
               //fshp(0) is nPol
               //fshp(1) is mChan
               //Int rChan = zpi / fshp(0);
               //Int rPol = zpi % fshp(0);
               //cout << " aveRow=" << rc << " row=" << row
               //     << " rChan=" << rChan << " rPol=" << rPol
               //     << " sp=" << sp << endl;

               //cout << " \noldFlag=";
               //   for (Int b = 0; b < avgIp(0) * avgIp(1); b++) {
               //      cout << avgtheflags_p(b, row);
               //   } 
               //cout << " \noldflagit="; 
               //   for (Int b = 0; b < avgIp(0) * avgIp(1); b++) {
               //      Int p = row * avgIp(0) * avgIp(1);
               //      cout << avgflagit.data()[p + b]; 
               //   }
               //cout << "\n";

               Vector<Int> pols(avgIp(0));
               Int pCnt = 0;
               if (ext.contains("P")) {
                for (Int i = 0; i < avgIp(0); i++) {
                 pols(pCnt++) = i;
                }
               }
               else {
                pols(pCnt++) = polids(zpi, 0);
               }
               pols.resize(pCnt, True);
               //cout << "pols=" << pols << endl;


               Vector<Int> chans(avgIp(1));
               Int cCnt = 0;
               if (ext.contains("C")) {
                for (Int i = 0; i < avgIp(1); i++) {
                 chans(cCnt++) = i;
                }
               }
               else {
                //for each row in chanMap
                //if spwid match chanMap(*, 4) and 
                //   rChan match chanMap(*, 3)
                for (Int t = 0; t < Int(chanMap.nrow()); t++) {
                 if (sp == chanMap(t, 4) && polids(zpi, 1) == chanMap(t, 3)) {
                  //cout << " chanMap(" << t << ")=" << chanMap.row(t) << endl;
                  for (Int i = chanMap(t, 0);
                       i <= chanMap(t, 1); i += chanMap(t, 2)) {
                   chans(cCnt++) = i;
                  }
                 }
                }
               }
               chans.resize(cCnt, True);
               //cout << "chns=" << chans << endl;

              
               for (uInt i = 0; i < chans.nelements(); i++) {
                Int d = chans(i);
                for (uInt j = 0; j < pols.nelements(); j++) {
                 Int pol = pols(j);
                 Int s = d * avgIp(0) + pol;
  

                 //cout  
                 //    << " pol=" << pol
                 //    << " chan=" << d
                 //    << " pos=" << s  
                     //<< " flag dir=" << direction
                     //<< " oldflag=" << avgtheflags_p(s, row)
                 //    << endl;
  
                 //cout << "row=" << row << endl;
                 avgtheflags_p(s, row) = (Bool)direction;


                 srow = row * avgIp(0) * avgIp(1) + s;
                 avgflagit.data()[srow] = direction ? 
                           avgtheflags_p(s, row) | (avgflagit.data()[srow]) :
                           avgtheflags_p(s, row) & (avgflagit.data()[srow]);
                 numflagpoints_p++;

  
                 //cout << " srow=" << srow << " avgflagit="
                 //     << avgflagit.data()[srow] << endl;  
                }
               }
               //cout << "row=" << row << endl;
               //cout << " \nnewFlag=";
               //   for (Int b = 0; b < avgIp(0) * avgIp(1); b++) {
               //      cout << avgtheflags_p(b, row);
               //   } 
               //cout << " \nnewflagit="; 
               //   for (Int b = 0; b < avgIp(0) * avgIp(1); 
               //                                       b++) {
               //      Int p = row * avgIp(0) * avgIp(1);
               //      cout << avgflagit.data()[p + b]; 
               //   }
               //cout << "\n";


               if (direction) {
                //reconsile after flag cell
                bool allFlagged = 1;
                for (Int b = 0; b < avgIp(0) * avgIp(1); b++) {
                 if (!avgtheflags_p(b, row)) {
                  allFlagged = 0;
                  break;
                 }
                } 
                if (allFlagged) {
                 //cout << "flag row=" << row << endl;
                 avgrowflags_p(row) = 1;
                }
               }

              }
             }
           //cout << "done disk" << endl;
           }
           cnt++;
         }
        }

        fyp += fshp.product();
        Flags_p.putColumn(fslice, flagit); 

        try {
          //AvgFlags_p.putColumn(avgtheflags_p);
          AvgFlags_p.putColumn(avgflagit);

          AvgRowFlags_p.putColumn(avgrowflags_p);

          itsTab_p->flush(True);
        }
          catch(...) {
            log->out("Can not write flags", 
                 fnname, clname, LogMessage::WARN);
        }
      }
     }
     else if (frcol_p) {
      Array<Bool> flagarray;  
      if (fcol_p) { 
         flagarray.resize(Flags_p.shape(0));  
      }

#if LOG0
      {
          ostringstream os;
          os << "FLAGROW\n" 
             << " theflags_p=" << theflags_p << endl; 
          log->out(os, fnname, clname, LogMessage::DEBUG2);
      }
#endif
                          
      for (rc = 0; rc < NRows_p; rc++) {
         if (selectedPoint(fyp, rc)) {
            theflags_p(fyp, rc) = (Bool)direction;
            rowflags_p[rc] = direction ?  
                     theflags_p(fyp,rc) | rowflags_p[rc] :  
                     theflags_p(fyp,rc) & rowflags_p[rc];
            numflagpoints_p++;

            if (fcol_p) {
               flagarray = rowflags_p[rc]; 
               Flags_p.put(rc, flagarray); 
            }
         }
         //else 
         //    rowflags_p[rc] |= theflags_p(fyp,rc);
      }
      fyp++;
     }
    }

    if (fyp > 0 && fyp != NPlots_p) 
       log->out("NPlots is not correct (1)", 
                fnname, clname, LogMessage::WARN );
 
    if (fcol_p && frcol_p) {
        Array<Bool> flagarray;
        flagarray.resize();
        Bool rflag = True;
        for (Int rc = 0; rc < NRows_p; rc++) {
           Flags_p.get(rc,flagarray);
           rflag=True;
           for (Int j = 0; j < flagarray.shape().product(); j++) 
              rflag &= flagarray.data()[j];
                      
           if (rflag) 
              rowflags_p[rc]=True;
           else 
              rowflags_p[rc]=False;
        }
     }
 
#if LOG0
     if (fcol_p && frcol_p) {
        Array<Bool> flagcol;
        flagcol = (Flags_p.getColumn());  
        IPosition tshp = flagcol.shape();
        if (direction == 1) {
           Int count = 0;
           Bool rflag = True;
           for (Int i = 0; i < tshp[2]; i++) {
              rflag=True;
              for (Int j = 0; j < tshp[0] * tshp[1]; j++) { 
                   rflag &= flagcol.data()[count+j];
              }
                          
              if (rowflags_p[i]) { 
                 // If row is flagged, set all data flags to True.
                 for (Int j = 0; j < tshp[0] * tshp[1]; j++)
                    flagcol.data()[count+j] = True;
              }
              else if (rflag) 
                 rowflags_p[i] = True;
                         
              count += tshp[0] * tshp[1];
          }
       }
       else {
          Int count = 0;
          Bool rflag = True;
          for (Int i = 0; i < tshp[2]; i++) {
             rflag=True;
             for (Int j = 0;j < tshp[0] * tshp[1]; j++) 
                rflag &= flagcol.data()[count+j];
                                     
             if (!rowflags_p[i] && rflag) {
                for (Int j = 0; j < tshp[0] * tshp[1]; j++)
                   flagcol.data()[count+j] = False;
              }
              else if(!rflag) 
                 rowflags_p[i]=False;
                         
              count += tshp[0]*tshp[1];
           }
        }
 
        Flags_p.putColumn(flagcol);
     }
#endif
 
     if (frcol_p) 
        RowFlags_p.putColumn(rowflags_p);
 
     if (fyp > 0 && fyp != NPlots_p) 
         log->out( "NPlots is not correct (2)", 
              fnname, clname, LogMessage::WARN );

     //cout << "SelTab_p=" << SelTab_p << endl;
 
     try {
        SelTab_p.flush();
        log->out("Flags written to disk.",
                  fnname, clname, LogMessage::DEBUG2); 
     }
     catch(...) {
        log->out("Failed to write flags..", 
                  fnname, clname, LogMessage::WARN);
     }

   }
   catch (AipsError x) {
      log->out("Failed to write flags...", 
                    fnname, clname, LogMessage::WARN);
   }

   log->FnExit(fnname, clname);
   return 0;
}


/////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////
Int BasePlot::getFlags(String /*flagversion*/, String msname)
{
   String fnname = "getFlags";
   log->FnEnter(fnname + "(flageversion, msname)", clname);

   itsTabName_p = msname;
   if (itsTab_p != NULL) {
      delete itsTab_p;
      itsTab_p = NULL;
   }

   try {
      itsTab_p = new Table(itsTabName_p, casa::Table::Update);
   }
   catch (...) {
      log->out("Can not open MS to write flags",
               fnname, clname, LogMessage::WARN); 
      return 0;
   }
   log->out("MS open for flagging", fnname, clname, LogMessage::DEBUG2); 

   Slicer fslice;
   Array<Bool> flagit; 
   Vector<Float> vfcount;
   Vector<Float> vtcount;
   Vector<Bool> vsflag;
   IPosition fshp;
   //Bool sflag;
   Bool row;
   
   row = False;

   //if (FV) {
   //   if(flagversion.length() && 
   //        !currentflagversion_p.matches(flagversion))
   //   {
   //      log->out(String("getting flag version ")+flagversion,
   //           fnname, clname, LogMessage::DEBUG2); 
   //      FV->attachFlagColumns(flagversion, AvgRowFlags_p, 
   //                            AvgFlags_p, *itsTab_p);
   //      currentflagversion_p = flagversion;
   //   }
   //}
 
   AvgFlags_p.attach(*itsTab_p, "FLAG");
   AvgRowFlags_p.attach(*itsTab_p, "FLAG_ROW");

   if(frcol_p) {
       avgrowflags_p = AvgRowFlags_p.getColumn();
   }

   //cout << "rowflags=" << avgrowflags_p;
   
   flagit.resize();
   flagit = AvgFlags_p.getColumn();
   //cout << "flagit=" << flagit.shape() << endl;
   IPosition ip = flagit.shape();
   avgtheflags_p.resize(ip(1) * ip(0), ip(2));   
   //avgtheflags_p.set(False);

   Int fyp=0; 
   Int cnt=0;
   Int zp;
   for (Int rc = 0; rc < ip(2); rc++) {
      zp = fyp;
      for (Int i = 0; i < ip(0) * ip(1); i++) {
         avgtheflags_p(zp + i, rc) = (flagit.data()[cnt]);
         cnt++;
      }
   }
   fyp += fshp.product();
  
   //cout << avgtheflags_p(0, 0) << endl;
   //cout << avgtheflags_p(125, 22653) << endl;

   //Vector<TableExprNode> ctens(2);
   //ctens[0] = RecordGram::parse(*itsTab_p, "FLAG");
   //ctens[1] = RecordGram::parse(*itsTab_p, "FLAG_ROW");
   //
   //log->out(String("FLAG nrow=") 
   //         + String::toString(ctens[0].nrow()) +
   //         "FLAG_ROW nrow=" 
   //         + String::toString(ctens[1].nrow()),
   //         fnname, clname, LogMessage::DEBUG2);
   return 0;
}

/*********************************************************************************/
/* Set Flags */
/* Directly write to disk. */
Int BasePlot::flagData(Int /*diskwrite*/, Int /*setrowflag*/, Int direction)
{


   String fnname = "flagData";
   log->FnEnter(fnname + "(diskwrite, setrowflag, direction)", clname);

   flagdirection_p = direction;
   numflagpoints_p = 0;
   
   ostringstream os;
   os << "Reduction type : " << ReductionType_p;
   log->out(os, fnname, clname, LogMessage::DEBUG2); 
    
   // direction = 1 --> flag
   // direction = 0 --> unflag
   // setrowflag = 0 --> don't write into RowFlag
   // setrowflag = 1 --> fill in and write RowFlag

   try
   {

   Int fyp;
   Slicer fslice;
   Array<Bool> flagit; 
   Bool row;
   IPosition fshp;
   
   Int ff=0;
   Int zp,cnt,rc;
   row = False;
   Int npol=0;
   
   fyp=0;      
   for(int z=0;z<nTens_p;z++) 
   {
       /* Check that the y-data column shape is SAME from that of FLAG
          and that FLAG exists. */
      if( isArrayCol_p[z] && fcol_p && 
          (TENcolshapes_p[z].isEqual(FlagColShape_p)))
      {
        fslice = TENslices_p[z];
        fshp = fslice.length();

        /* Get the latest flags for this slice */
        /* While filling flags, honour any previous flags as follows */
        /*   -> dir=0 : FLAG   : old OR new
             -> dir=1 : UNFLAG : old AND new
        */
        flagit.resize();
        flagit = Flags_p.getColumn(fslice); 
            // [ row, col, nrows ] : row : innermost.

        //cout << " fslice="<< fslice <<  " fshp=" << fshp 
           //<< " flagit=" << flagit 
        //   << " flagit IP=" << flagit.shape() 
           //<< " theflags_p=" << theflags_p 
        //   << endl;

        switch(ReductionType_p[z])
        {
           
           case 0: /* No reduction */   
              cnt=0;
              for(rc=0;rc<NRows_p;rc++)
              {
                 zp = fyp;
                 for (Int i=0;i<fshp.product();i++)
                 {
                  if(selectedPoint(zp+i,rc))
                  {
                 // cout << "zp=" << zp << " i=" << i << " rc=" << rc 
                 //      << " cnt=" << cnt << endl;
                     theflags_p(zp+i,rc) = (Bool)direction;
                     //?
                     (flagit.data()[cnt]) = direction ? 
                       theflags_p(zp+i,rc) | (flagit.data()[cnt]) :
                       theflags_p(zp+i,rc) & (flagit.data()[cnt]);
                     numflagpoints_p++;
                  }
                  cnt++;
                 }
              }
              fyp += fshp.product();
              break;
           case 1: /* Scalar reduction : MEAN */
              cnt=0;
              for(rc=0;rc<NRows_p;rc++)
              {
               zp = fyp;
               for (Int i=0;i<fshp.product();i++)
               {
                 if(selectedPoint(zp,rc))
                 {
                  theflags_p(zp,rc) = (Bool)direction;
                  (flagit.data()[cnt]) = direction ? 
                    theflags_p(zp,rc) | (flagit.data()[cnt]) :
                    theflags_p(zp,rc) & (flagit.data()[cnt]);
                  numflagpoints_p++;
                 }
                 cnt++;
               }
              }
              fyp++;
              
              break;
           case 2: /* Vector reduction : MEANS */
           case 3:
              npol = fshp[0];
              cnt=0;
              for(rc=0;rc<NRows_p;rc++)
              {
                 zp = fyp;
                 ff=0;
                 for (Int i=0;i<fshp.product();i++)
                 {
                    if(selectedPoint(zp+ff,rc))
                    {
                     theflags_p(zp+ff,rc) = (Bool)direction;
                       (flagit.data()[cnt]) = direction ? 
                       theflags_p(zp+ff,rc) | (flagit.data()[cnt]) :
                       theflags_p(zp+ff,rc) & (flagit.data()[cnt]);
                     numflagpoints_p++;
                    }
                    cnt++;
                    if(ReductionType_p[z]==2)
                    { 
                       if( (i+1)%npol==0 ) ff=0;
                       else ff++;
                       
                    }
                    if(ReductionType_p[z]==3 && (i+1)%npol==0) ff++;
                 }
              }
              fyp += fshp[ReductionType_p[z]-2];
              break;
           default: BasePlotError(String("Unsupported TaQL reduction"));
        }//end of switch
        
        /* Now write these flags to disk */
        //cout << "flagit=" << flagit << endl;

        Flags_p.putColumn(fslice,flagit); 
       // cout << " oldflag=" << theflags_p << endl;
        
      }
      else {
          // Write into rowflags, since this is a scalar column.. // fcol !!
          //cout << "write row flag" << endl; 
          if(frcol_p)
          {
              Array<Bool> flagarray;  
              if(fcol_p) 
                  flagarray.resize(Flags_p.shape(0));  
                                
              for(rc=0;rc<NRows_p;rc++) // all rows
              {
                 /* If FLAG doesn't exist, then 
                     flagging hasn't been done yet */
                 if(selectedPoint(fyp,rc))
                 {
                     theflags_p(fyp,rc) = (Bool)direction;
                     rowflags_p[rc] = 
                            direction ?  theflags_p(fyp,rc) | rowflags_p[rc] 
                                      :  theflags_p(fyp,rc) & rowflags_p[rc];
                     numflagpoints_p++;

                     // write into data flags too..
                     // -> all of them.
                     // get a slice of the whole row and fill it.
                     if(fcol_p)
                     {
                         flagarray = rowflags_p[rc]; 
                         Flags_p.put(rc,flagarray); 
                     }
                 }
                 //else 
                 //    rowflags_p[rc] |= theflags_p(fyp,rc);
               }
               fyp++;
           }
       }
                
    }

    if(fyp>0 && fyp != NPlots_p) 
         log->out( "NPlots is not correct (1)", 
               fnname, clname, LogMessage::WARN );

     /* Reconcile flag and rowflag and write back to disk */
     // Only set RF to &&&& of F
     if(fcol_p && frcol_p)
     {
        Array<Bool> flagarray;
        flagarray.resize();
        Bool rflag=True;
        for(Int rc=0;rc<NRows_p;rc++)
        {
            Flags_p.get(rc,flagarray);
            rflag=True;
            for(Int j=0;j<flagarray.shape().product();j++) 
               rflag &= flagarray.data()[j];
                     
               if(rflag) 
                  rowflags_p[rc]=True;
               else 
                 rowflags_p[rc]=False;
        }
     }

#if LOG0
     if(fcol_p && frcol_p)
     {
        Array<Bool> flagcol;
        flagcol = (Flags_p.getColumn());  
        IPosition tshp=flagcol.shape();
        //cout << "flagcol shape : " << tshp << LogIO::POST
        if(direction==1)//flag
        {
            Int count=0;
            Bool rflag=True;
            for(Int i=0;i<tshp[2];i++)
            {
               rflag=True;
               for(Int j=0;j<tshp[0]*tshp[1];j++) 
                   rflag &= flagcol.data()[count+j];
                         
                if(rowflags_p[i]) 
                // If row is flagged, set all data flags to True.
                {
                   for(Int j=0;j<tshp[0]*tshp[1];j++)
                       flagcol.data()[count+j] = True;
                }
                else if(rflag) 
                   rowflags_p[i]=True;
                         
                count += tshp[0]*tshp[1];
            }
        }
        else //unflag
        {
           Int count=0;
           Bool rflag=True;
           for(Int i=0;i<tshp[2];i++)
           {
              rflag=True;
              for(Int j=0;j<tshp[0]*tshp[1];j++) 
                 rflag &= flagcol.data()[count+j];
                                     
              if(!rowflags_p[i] && rflag)// unflag all data flags
              {
                 for(Int j=0;j<tshp[0]*tshp[1];j++)
                      flagcol.data()[count+j] = False;
              }
              else if(!rflag) 
                 rowflags_p[i]=False;
                         
              count += tshp[0]*tshp[1];
           }
        }

        Flags_p.putColumn(flagcol);
     }
#endif

     if(frcol_p) 
        RowFlags_p.putColumn(rowflags_p);

     if(fyp>0 && fyp != NPlots_p) 
        log->out( "NPlots is not correct (2)", 
             fnname, clname, LogMessage::WARN );

     try {
        SelTab_p.flush();
     }
     catch(...) {
        log->out("Failed to write flags.", 
                 fnname, clname, LogMessage::WARN );
     }

     log->out( "Flags written to disk.",
               fnname, clname, LogMessage::DEBUGGING); 

   }
   catch (AipsError x)
   {
      BasePlotError(String("Unable to write flags to disk"));
   }
   
   log->FnExit(fnname, clname);
   return 0;
}

/*********************************************************************************/
void BasePlot::BasePlotError(String msg)
{
   throw AipsError("BasePlot: " + msg);
}
/*********************************************************************************/

Int BasePlot::flagData(Int direction, String /*msname*/,
                       String ext)
{


   String fnname = "flagData";
   log->FnEnter(fnname + "(diskwrite, setrowflag, direction)", clname);
   //cout << "msname=" << msname << endl;
   //cout << "direction=" << direction << endl;
   //cout << "extendflag=" << ext << endl;


   flagdirection_p = direction;
   numflagpoints_p = 0;
   
   ostringstream os;
   //os << "Reduction type : " << ReductionType_p;
   log->out(os, fnname, clname, LogMessage::DEBUG2); 
    
   // direction = 1 --> flag
   // direction = 0 --> unflag
   // setrowflag = 0 --> don't write into RowFlag
   // setrowflag = 1 --> fill in and write RowFlag

   //cout << " oldflag=" << theflags_p << endl;
   try
   {

   Int fyp;
   Slicer fslice;
   Array<Bool> flagit; 
   Bool row;
   IPosition fshp;
   
   Int ff=0;
   Int zp,cnt,rc;
   row = False;
   Int npol=0;
   
   //get flags
   flagit.resize();
   flagit = Flags_p.getColumn(); 
   IPosition avgIp = flagit.shape();
   //cout << "nPol=" << avgIp(0) << " nChan=" << avgIp(1)
   //     << " nRow=" << avgIp(2) << endl;

   Matrix<Int> polids = getLocatePolChan();
   //cout << "polids=" << polids << endl;

   fyp=0;      
   for(int z=0;z<nTens_p;z++) 
   {
       /* Check that the y-data column shape is SAME from that of FLAG
          and that FLAG exists. */
      if( isArrayCol_p[z] && fcol_p && 
          (TENcolshapes_p[z].isEqual(FlagColShape_p)))
      {
        fslice = TENslices_p[z];
        fshp = fslice.length();

        /* Get the latest flags for this slice */
        /* While filling flags, honour any previous flags as follows */
        /*   -> dir=0 : FLAG   : old OR new
             -> dir=1 : UNFLAG : old AND new
        */
        //flagit.resize();
        //flagit = Flags_p.getColumn(fslice); 
            // [ row, col, nrows ] : row : innermost.

        //cout //<< " fslice="<< fslice <<  " fshp=" << fshp 
        //   << " flagit=" << flagit 
           //   << " flagit IP=" << flagit.shape() 
        //   << " theflags_p=" << theflags_p 
        //   << endl;

        //cout << "fyp=" << fyp << endl;
        switch(ReductionType_p[z])
        {
           
           case 0: /* No reduction */   
              cnt=0;
              for(rc=0;rc<NRows_p;rc++) {
                 zp = fyp;
                 for (Int i=0;i<fshp.product();i++) {
                  Int zpi = zp + i;
                  //cout << "zp=" << zp << " i=" << i << " rc=" << rc 
                  //     << " cnt=" << cnt 
                  //     << " srow=" << rc * fshp.product() + i << endl;
                  if (selectedPoint(zpi,rc)) {
                    Int pol = polids(zpi, 0);
                    Int chn = polids(zpi, 1);
                    //cout << "pol=" << pol << " chn=" << chn << endl;
                     
                    //{
                    //   theflags_p(zpi,rc) = (Bool)direction;
                    //   (flagit.data()[cnt]) = direction ? 
                    //     theflags_p(zpi,rc) | (flagit.data()[cnt]) :
                    //     theflags_p(zpi,rc) & (flagit.data()[cnt]);
                    // }

                     
                     for (Int p = 0; p < avgIp(0); p++) {
                       for (Int c = 0; c < avgIp(1); c++ ) {
                         if (ext.contains("P") && c == chn) {
                            IPosition ip(3, p, c, rc);
                            //cout << "ip=" << ip << endl;
                            theflags_p(zpi,rc) = (Bool)direction;
                            flagit(ip) = direction ? 
                               theflags_p(zpi,rc) | flagit(ip) :
                               theflags_p(zpi,rc) & flagit(ip);
                         }
                         if (ext.contains("C") && p == pol) {
                            IPosition ip(3, p, c, rc);
                            //cout << "ip=" << ip << endl;
                            theflags_p(zpi,rc) = (Bool)direction;
                            flagit(ip) = direction ? 
                               theflags_p(zpi,rc) | flagit(ip) :
                               theflags_p(zpi,rc) & flagit(ip);
                         }
                       }
                     }
                     numflagpoints_p++;
                  }
                  cnt++;
                 }
              }
              fyp += fshp.product();
              break;
           case 1: /* Scalar reduction : MEAN */
              cnt=0;
              for(rc=0;rc<NRows_p;rc++)
              {
               zp = fyp;
               for (Int i=0;i<fshp.product();i++)
               {
                 if(selectedPoint(zp,rc))
                 {
                  theflags_p(zp,rc) = (Bool)direction;
                  (flagit.data()[cnt]) = direction ? 
                    theflags_p(zp,rc) | (flagit.data()[cnt]) :
                    theflags_p(zp,rc) & (flagit.data()[cnt]);
                  numflagpoints_p++;
                 }
                 cnt++;
               }
              }
              fyp++;
              
              break;
           case 2: /* Vector reduction : MEANS */
           case 3:
              npol = fshp[0];
              cnt=0;
              for(rc=0;rc<NRows_p;rc++)
              {
                 zp = fyp;
                 ff=0;
                 for (Int i=0;i<fshp.product();i++)
                 {
                    if(selectedPoint(zp+ff,rc))
                    {
                     theflags_p(zp+ff,rc) = (Bool)direction;
                       (flagit.data()[cnt]) = direction ? 
                       theflags_p(zp+ff,rc) | (flagit.data()[cnt]) :
                       theflags_p(zp+ff,rc) & (flagit.data()[cnt]);
                     numflagpoints_p++;
                    }
                    cnt++;
                    if(ReductionType_p[z]==2)
                    { 
                       if( (i+1)%npol==0 ) ff=0;
                       else ff++;
                       
                    }
                    if(ReductionType_p[z]==3 && (i+1)%npol==0) ff++;
                 }
              }
              fyp += fshp[ReductionType_p[z]-2];
              break;
           default: BasePlotError(String("Unsupported TaQL reduction"));
        }//end of switch
        
        /* Now write these flags to disk */
        //cout << " theflags_p=" << theflags_p << endl;
        //cout << "write flagit=" << flagit << endl;

        Flags_p.putColumn(flagit); 
        
      }
      else {
          // Write into rowflags, since this is a scalar column.. // fcol !!
          //cout << "write row flag" << endl; 
          if(frcol_p)
          {
              Array<Bool> flagarray;  
              if(fcol_p) 
                  flagarray.resize(Flags_p.shape(0));  
                                
              for(rc=0;rc<NRows_p;rc++) // all rows
              {
                 /* If FLAG doesn't exist, then 
                     flagging hasn't been done yet */
                 if(selectedPoint(fyp,rc))
                 {
                     theflags_p(fyp,rc) = (Bool)direction;
                     rowflags_p[rc] = 
                            direction ?  theflags_p(fyp,rc) | rowflags_p[rc] 
                                      :  theflags_p(fyp,rc) & rowflags_p[rc];
                     numflagpoints_p++;

                     // write into data flags too..
                     // -> all of them.
                     // get a slice of the whole row and fill it.
                     if(fcol_p)
                     {
                         flagarray = rowflags_p[rc]; 
                         Flags_p.put(rc,flagarray); 
                     }
                 }
                 //else 
                 //    rowflags_p[rc] |= theflags_p(fyp,rc);
               }
               fyp++;
           }
       }
                
    }

    if(fyp>0 && fyp != NPlots_p) 
         log->out( "NPlots is not correct (1)", 
               fnname, clname, LogMessage::WARN );

     /* Reconcile flag and rowflag and write back to disk */
     // Only set RF to &&&& of F
     if(fcol_p && frcol_p)
     {
        Array<Bool> flagarray;
        flagarray.resize();
        Bool rflag=True;
        for(Int rc=0;rc<NRows_p;rc++)
        {
            Flags_p.get(rc,flagarray);
            rflag=True;
            for(Int j=0;j<flagarray.shape().product();j++) 
               rflag &= flagarray.data()[j];
                     
               if(rflag) 
                  rowflags_p[rc]=True;
               else 
                 rowflags_p[rc]=False;
        }
     }

#if LOG0
     if(fcol_p && frcol_p)
     {
        Array<Bool> flagcol;
        flagcol = (Flags_p.getColumn());  
        IPosition tshp=flagcol.shape();
        //cout << "flagcol shape : " << tshp << LogIO::POST
        if(direction==1)//flag
        {
            Int count=0;
            Bool rflag=True;
            for(Int i=0;i<tshp[2];i++)
            {
               rflag=True;
               for(Int j=0;j<tshp[0]*tshp[1];j++) 
                   rflag &= flagcol.data()[count+j];
                         
                if(rowflags_p[i]) 
                // If row is flagged, set all data flags to True.
                {
                   for(Int j=0;j<tshp[0]*tshp[1];j++)
                       flagcol.data()[count+j] = True;
                }
                else if(rflag) 
                   rowflags_p[i]=True;
                         
                count += tshp[0]*tshp[1];
            }
        }
        else //unflag
        {
           Int count=0;
           Bool rflag=True;
           for(Int i=0;i<tshp[2];i++)
           {
              rflag=True;
              for(Int j=0;j<tshp[0]*tshp[1];j++) 
                 rflag &= flagcol.data()[count+j];
                                     
              if(!rowflags_p[i] && rflag)// unflag all data flags
              {
                 for(Int j=0;j<tshp[0]*tshp[1];j++)
                      flagcol.data()[count+j] = False;
              }
              else if(!rflag) 
                 rowflags_p[i]=False;
                         
              count += tshp[0]*tshp[1];
           }
        }

        Flags_p.putColumn(flagcol);
     }
#endif

     if(frcol_p) 
        RowFlags_p.putColumn(rowflags_p);

     if(fyp>0 && fyp != NPlots_p) 
        log->out( "NPlots is not correct (2)", 
             fnname, clname, LogMessage::WARN );

     try {
        SelTab_p.flush();
     }
     catch(...) {
        log->out("Failed to write flags.", 
                 fnname, clname, LogMessage::WARN );
     }

     log->out( "Flags written to disk.",
               fnname, clname, LogMessage::DEBUGGING); 

   }
   catch (AipsError x)
   {
      BasePlotError(String("Unable to write flags to disk"));
   }
   //cout << " newflag=" << theflags_p << endl;
   
   log->FnExit(fnname, clname);
   return 0;
}
} //# NAMESPACE CASA - END 

