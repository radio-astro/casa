//# CrossPlot.cc: Basic table access class for the TablePlot (tableplot) tool
//#               to plot across rows for an array-column.
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003
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

#include <tools/tables/TablePlot/CrossPlot.h>

#include <casa/iomanip.h>

namespace casa { //# NAMESPACE CASA - BEGIN

#define TMR(a) "[User: " << a.user() << "] [System: " << a.system() << "] [Real: " << a.real() << "]"
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))


#define LOG0 0
#define LOG2 1

String CrossPlot::clname = "CrossPlot";

/* Default Constructor */
CrossPlot::CrossPlot() : BasePlot()
{
   String fnname= "CrossPlot";
   dbg=0;   ddbg=0;   adbg=0;
   if (!log)
      log = SLog::slog();
#if LOG0 
   log->FnEnter(fnname, clname);
#endif 
   nflagmarks_p=0;
   Map_p.resize(0,0); 
   locflagmarks_p.resize(0);
   xpdrow_p.resize(); xpdcol_p.resize();
   columnsxaxis_p=0;
   plotperrow_p = False;
   honourxflags_p=False;

   pType_p = CROSSPLOT; 
}

/*********************************************************************************/

/* Destructor */
CrossPlot::~CrossPlot()
{
    String fnname= "~CrossPlot";
#if LOG0 
    log->FnEnter(fnname, clname);
#endif
}

/*********************************************************************************/


/*********************************************************************************/
Int CrossPlot::createXTENS(Vector<String> &/*datastr*/)
{
   String fnname= "createXTENS";
#if LOG0
   log->FnEnter(fnname, clname);
#endif 
   return 0;
}
/*********************************************************************************/

/*********************************************************************************/

/* Extract X data from the table */
// Please don't change anything in this function - without testing it out
// thoroughly. It's VERY hard to debug...
Int CrossPlot::getXData(TPConvertBase* conv, Bool dummyread)
{
   //cout << "CrossPlot: conv=" << conv << endl;
   String fnname= "getXData";
   Int  rc=0;
#if LOG0 
   log->FnEnter(fnname + "(conv, dummyread)", clname );
#endif 
   if(dummyread)
   {
      Int rownum=1,colnum=1,rowstride=0,colstride=0, nrows=0,ncols=0;
      //Double midrowval=1,midcolval=1;
      Vector<Double> midrowval,midcolval;

      xpdrow_p.resize(nTens_p);
      xpdcol_p.resize(nTens_p);
      ncellrows_p.resize(nTens_p);
      ncellcols_p.resize(nTens_p);
      IPosition istart;
      IPosition iend;
      IPosition istride;
      IPosition tlength;
      Slicer tempslicer;

      for(Int z=0;z<nTens_p;z++)
      {
         Xshape_p[z] = IPosition(2,1,1);

         tempslicer = TENslices_p[z];
         istart = tempslicer.start();
         iend = tempslicer.end();
         istride = tempslicer.stride();
         tlength = tempslicer.length();
         
#if LOG0 
         ostringstream os;
         os  << "z : " << z << "  start : " << istart 
             << "  end : " << iend << "  stride : " << istride 
             << "    length : " << tlength;
         log->out(os, fnname, clname, LogMessage::DEBUG1); 
#endif 
 
         nrows = tlength[0];
         rownum = istart[0];
         rowstride = istride[0];

         if(tlength.nelements()>1) 
         {   
            ncols = tlength[1];
            colnum = istart[1];
            colstride = istride[1];
         }
         else
         {
            ncols = 1;
            colnum = 1;
            colstride = 1;
         }
                        
         Vector<Int> rowcnt,colcnt;
         midrowval.resize(ncols); midrowval.set(0.0);
         midcolval.resize(nrows); midcolval.set(0.0);
         rowcnt.resize(ncols);rowcnt.set(0);
         colcnt.resize(nrows);colcnt.set(0);
                        
         /* Correct averaged 'mid' values for flagged rows/cols*/
                        
         Int cnt=0;
                        
         for(Int c=0;c<ncols;c++) // cols,chans
         {
            for(Int r=0;r<nrows;r++) // rows,corrs
            {
               if(!honourxflags_p || !TENRowColFlag_p[z][cnt])
               {
                   midcolval[r] += colnum + c * colstride;
                   colcnt[r]++ ;
                   midrowval[c] += rownum + r * rowstride;
                   rowcnt[c]++ ;
               }
               cnt++;
            }
         }
         for(Int c=0;c<ncols;c++) // cols,chans
            if(rowcnt[c]) 
               midrowval[c] /= rowcnt[c];
         for(Int r=0;r<nrows;r++) // cols,chans
            if(colcnt[r]) 
               midcolval[r] /= colcnt[r];
                        
#if LOG0 
         {
            ostringstream os;
            os << "midrowval : " << midrowval
               << "\nmidcolval : " << midcolval;
            log->out(os, fnname, clname, LogMessage::DEBUG1); 
         }
#endif 

         /* Fill in xpdrow and xpdcol depending on the type of reduction */
         switch(ReductionType_p[z])
         {
         
         case 0: /* No reduction. enumerate all touched chans,pols */
            xpdrow_p[z].resize(nrows*ncols); xpdcol_p[z].resize(ncols*nrows);
            ncellrows_p[z]=nrows; ncellcols_p[z]=ncols;
#if LOG0 
            log->out(String("No reduction : nrow,ncol : ") +
                      nrows + ", " + ncols,
                      fnname, clname, LogMessage::DEBUG1); 
#endif 
            /* Enumerate for row */
            for(Int j=0;j<nrows;j++) //pol, row
            {
               for(Int k=0;k<ncols;k++)
                   xpdrow_p[z][k*nrows + j] = 
                     conv->Xconvert_row((Double)(rownum),rc,BasePlot::tabNum_p);
               rownum += rowstride;
            }
            
            /* Enumerate for col */
            for(Int j=0;j<ncols;j++) //chan, col
            {
               for(Int k=0;k<nrows;k++)
                  xpdcol_p[z][j*nrows+k] = 
                     conv->Xconvert_col((Double)(colnum),rc,BasePlot::tabNum_p);
	       colnum += colstride;
            }
            
            break;
         case 1: /* Scalar reduction : Only one value will 
                    exist in xpdrow and xpdcol. */
            nrows = 1;
            ncols = 1;
            xpdrow_p[z].resize(nrows*ncols); xpdcol_p[z].resize(ncols*nrows);
                                ncellrows_p[z]=nrows; ncellcols_p[z]=ncols;
#if LOG0 
            log->out(String("Scalar reduction : nrow,ncol : ") +
                 nrows + ", " + ncols,
                 fnname, clname, LogMessage::DEBUG1); 
#endif 
            /* Use middle value for row */
            for(Int k=0;k<ncols;k++)
               xpdrow_p[z][k] = 
                 conv->Xconvert_row(midrowval[k],rc,BasePlot::tabNum_p);
            /* Use middle value for col */
            for(Int k=0;k<nrows;k++)
               xpdcol_p[z][k] = 
                 conv->Xconvert_col(midcolval[k],rc,BasePlot::tabNum_p);
            break;
         case 2: /* Vector reduction : collape along cols. Remaining 
                    shape = nrows */
            ncols = 1;
            xpdrow_p[z].resize(nrows*ncols); xpdcol_p[z].resize(ncols*nrows);
                                ncellrows_p[z]=nrows; ncellcols_p[z]=ncols;
#if LOG0 
            log->out("Vector reduction along cols : nrow,ncol : " +
                     String::toString(nrows) + ", " + String::toString(ncols),
                     fnname, clname, LogMessage::DEBUG1); 
#endif 
            /* Enumerate for row */
            for(Int j=0;j<nrows;j++) //pol, row
            {
               for(Int k=0;k<ncols;k++)
                  xpdrow_p[z][k*nrows + j] = 
                    conv->Xconvert_row((Double)(rownum),rc,BasePlot::tabNum_p);
               rownum += rowstride;
            }
            /* Use middle value for col */
            for(Int k=0;k<nrows;k++)
                xpdcol_p[z][k] = 
                  conv->Xconvert_col(midcolval[k],rc,BasePlot::tabNum_p);
            break;
         case 3: /* Vector reduction : collape along rows. 
                    Remaining shape = nCrossCols */
            nrows = 1;
            xpdrow_p[z].resize(nrows*ncols); xpdcol_p[z].resize(ncols*nrows);
                                ncellrows_p[z]=nrows; ncellcols_p[z]=ncols;
#if LOG0 
            log->out(String("Vector reduction along rows : nrow,ncol : ") +
                  nrows + ", " + ncols,
                  fnname, clname, LogMessage::DEBUG1); 
#endif 

            /* Use middle value for row */
            for(Int k=0;k<ncols;k++)
               xpdrow_p[z][k] = 
                  conv->Xconvert_row(midrowval[k],rc,BasePlot::tabNum_p);
            /* Enumerate for col */
            for(Int j=0;j<ncols;j++) //chan, col
            {
               for(Int k=0;k<nrows;k++)
                   xpdcol_p[z][j*nrows + k] = 
                     conv->Xconvert_col((Double)(colnum),rc,BasePlot::tabNum_p);
               colnum += colstride;
            }
            break;
         default: CrossPlotError(String("Unsupported TaQL reduction"));

         }// end of switch

         
      }//end of for tens..
#if LOG0 
      ostringstream os;
      for(Int i=0;i<(Int)xpdrow_p.nelements();i++)
          os << "xpdrow[" << i << "] : " << xpdrow_p[i] << "\n";
      for(Int i=0;i<(Int)xpdcol_p.nelements();i++)
          os << "xpdcol[" << i << "] : " << xpdcol_p[i] << "\n";
      log->out(os, fnname, clname, LogMessage::DEBUG1); 
#endif 
   }
   else
   {
      // do nothing - xpdrow_p and xpdcol_ values have already been filled.
   }

   return 0;
}


/*********************************************************************************/

/* Compute the combined plot range */
Int CrossPlot::setPlotRange(Double &xmin, Double &xmax, Double &ymin, 
        Double &ymax, Bool showflags, Bool columnsxaxis, String flagversion, 
        Int averagenrows, String connectpoints, Bool doscalingcorrection, 
        String multicolour, Bool honourxflags)
{
   String fnname= "setPlotRange";
#if LOG0 
   log->FnEnter(fnname + "(xmin, xmax, ymin, ... )", clname);
#endif 
   //cout << "CrossPlot::setPlotRange" << endl;
   Matrix<Double> xprange_p,yprange_p;
   Double xvalue=0.0, yvalue=0.0;
   Bool flag=False, rflag=False;

   /* Record the showflags state */
    showflags_p = showflags;
        doscalingcorrection_p = doscalingcorrection;
        multicolour_p = multicolour;
        honourxflags_p = honourxflags;

   xprange_p.resize(NPlots_p,2);
   yprange_p.resize(NPlots_p,2);

   columnsxaxis_p = columnsxaxis;
   plotperrow_p = False;
   if( connectpoints.matches(String("cellcol")) || 
       connectpoints.matches(String("cellrow")) )
      plotperrow_p = True;
   
   //if(columnsxaxis_p) // Use xpdcol as the x axis values
   //else // Use xpdrow as the x axis values
   
   /* compute min and max for each Plot */
   for(int i=0;i<NPlots_p;i++)
   {
     xprange_p(i,0) = 1e+30;
     xprange_p(i,1) = -1e+30;
     yprange_p(i,0) = 1e+30;
     yprange_p(i,1) = -1e+30;
   }
     
   tmr.mark();
   getFlags(flagversion,showflags);
#if LOG0 
   ostringstream os;
   os << tmr.all() << " sec.";
   log->out(os, fnname, clname, LogMessage::NORMAL4); 
#endif 
   

   /* Update the xpdrow and xpdcol values to account
      for completely flagged cell rows or cell cols. */
   getXData(conv_p,True);
        
   /* Update the averages. 
      This function knows when it should be a no-op */
   computeAverages(averagenrows);

   Bool choosepoint=False;
   Int NR=0;
   for(int i=0;i<NPlots_p;i++)
   {
      if( ! Average_p ) NR = NRows_p; /* Use un-averaged data. */
      else NR = NAvgRows_p; /* Use the averaged data */
      
      for(int rc=0;rc<NR;rc++)
      {
         choosepoint = False;

         if( columnsxaxis_p ) 
          xvalue = 
          xpdcol_p[Map_p(i,2)][Map_p(i,4)*ncellrows_p[Map_p(i,2)] + Map_p(i,3)];
         else 
          xvalue = 
          xpdrow_p[Map_p(i,2)][Map_p(i,4)*ncellrows_p[Map_p(i,2)] + Map_p(i,3)];
         
         if( ! Average_p )
         {
            yvalue = yplotdata_p(Map_p(i,1),rc);
            flag = theflags_p(Map_p(i,1),rc);
            rflag = rowflags_p[rc];
         }
         else
         {
            yvalue = avgyplotdata_p(Map_p(i,1),rc);
            flag = avgtheflags_p(Map_p(i,1),rc);
            rflag = avgrowflags_p[rc];
         }
         
         if( showflags == False )
         {
            if( (flag == False) && (rflag == False) ) choosepoint = True;
         }
         else
         {
            if( (flag == True) || (rflag == True) ) choosepoint = True;
         }
         
         if( choosepoint ) 
         {
            if(xvalue <= xprange_p(i,0)) xprange_p(i,0) = xvalue;
            if(xvalue >= xprange_p(i,1)) xprange_p(i,1) = xvalue;
            if(yvalue <= yprange_p(i,0)) yprange_p(i,0) = yvalue;
            if(yvalue >= yprange_p(i,1)) yprange_p(i,1) = yvalue;
         }
         
         
      }// end of for rows
      
   }// end of for nplots
   
   xmin=0;xmax=0;ymin=0;ymax=0;
   xmin = xprange_p(0,0);
   xmax = xprange_p(0,1);
   ymin = yprange_p(0,0);
   ymax = yprange_p(0,1);
   
#if LOG0 
   log->out(String(" initial Ranges : [") + String::toString(xmin) + 
         "," + String::toString(xmax) + "] [" + 
         String::toString(ymin) + "," + String::toString(ymax) + "]",
       fnname, clname, LogMessage::DEBUG1); 
#endif 

   /* get a comnined min,max */

   for(int qq=1;qq<NPlots_p;qq++)
   {
      xmin = MIN(xmin,xprange_p(qq,0));
      xmax = MAX(xmax,xprange_p(qq,1));
   }
   for(int qq=1;qq<NPlots_p;qq++)
   {
      ymin = MIN(ymin,yprange_p(qq,0));
      ymax = MAX(ymax,yprange_p(qq,1));
   }
   
#if LOG0 
   log->out(String(" Final Ranges : [") +  String::toString(xmin) + 
       "," + String::toString(xmax) + "] [" 
       + String::toString(ymin) + "," + String::toString(ymax) + "]",
       fnname, clname, LogMessage::DEBUG1); 
#endif 

   return 0;
}

/*********************************************************************************/

/*********************************************************************************/
Bool CrossPlot::selectedPoint(Int np,Int nr)
{
   static Double xvalue=0.0,yvalue=0.0;

   /* Fill in xvalue, yvalue */
        // Use xpdcol_p for x data
        // Use xpdrow_p for x data
   if( columnsxaxis_p ) 
       xvalue = 
        xpdcol_p[Map_p(np,2)][Map_p(np,4)*ncellrows_p[Map_p(np,2)]+Map_p(np,3)];
   else 
      xvalue = 
        xpdrow_p[Map_p(np,2)][Map_p(np,4)*ncellrows_p[Map_p(np,2)]+Map_p(np,3)];
   
   if( ! Average_p ) 
      yvalue = yplotdata_p(Map_p(np,1),nr); 
      // use unaveraged values
   else 
      yvalue = avgyplotdata_p(Map_p(np,1),avgindices_p[nr]); 
      // use averaged values
   
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
Int CrossPlot::computeXAverages(Int /*averagenrows*/, Int /*remrows*/)
{
   //Ahaaa - do nothing !!! Because nothing happens to
   // channel and correlation indices, when row-averaging is done.
   return 0;
}
/*********************************************************************************/
/*********************************************************************************/
/*********************************************************************************/
/** Make this return a flipped value for crossplot **/
/*
Double CrossPlot::getXVal(Int pnum, Int col)
{
   if(columnsxaxis_p)// x axis is chans. use xpdcol
      return xpdcol_p[Map_p(pnum,2)][Map_p(pnum,4)];
   else
      return xpdrow_p[Map_p(pnum,2)][Map_p(pnum,3)];
}
*/

/*********************************************************************************/

Double CrossPlot::getXVal(Int pnum, Int col)
{
   //if( (!Average_p && NRows_p == 1) || (Average_p && NAvgRows_p==1) )
   if( plotperrow_p == True )
   {
      if(columnsxaxis_p)// x axis is chans. use xpdcol
         return xpdcol_p[Map_p(col,2)][Map_p(col,4)*ncellrows_p[Map_p(col,2)]+Map_p(col,3)];
      else
         return xpdrow_p[Map_p(col,2)][Map_p(col,4)*ncellrows_p[Map_p(col,2)]+Map_p(col,3)];
   }
   else
   {
      if(columnsxaxis_p)// x axis is chans. use xpdcol
         return xpdcol_p[Map_p(pnum,2)][Map_p(pnum,4)*ncellrows_p[Map_p(pnum,2)]+Map_p(pnum,3)];
      else
         return xpdrow_p[Map_p(pnum,2)][Map_p(pnum,4)*ncellrows_p[Map_p(pnum,2)]+Map_p(pnum,3)];
   }
}

/*********************************************************************************/

Double CrossPlot::getYVal(Int pnum, Int col)
{
   if( ! Average_p )
   {
      //if( NRows_p == 1 ) return yplotdata_p(Map_p(col,1),pnum);
      if( plotperrow_p == True ) return yplotdata_p(Map_p(col,1),pnum);
      else return yplotdata_p(Map_p(pnum,1),col);
   }
   else
   {
      //if( NAvgRows_p == 1) return avgyplotdata_p(Map_p(col,1),pnum);
      if( plotperrow_p == True ) return avgyplotdata_p(Map_p(col,1),pnum);
      else return avgyplotdata_p(Map_p(pnum,1),col);
   }
}

/*********************************************************************************/
Bool CrossPlot::getYFlags(Int pnum, Int col)
{
   if( ! Average_p )
   {
      //if( NRows_p == 1 ) return theflags_p(Map_p(col,1),pnum) | rowflags_p[pnum];
      if( plotperrow_p == True ) return theflags_p(Map_p(col,1),pnum) | rowflags_p[pnum];
      else return theflags_p(Map_p(pnum,1),col) | rowflags_p[col];
   }
   else
   {
      //if( NAvgRows_p == 1 ) return avgtheflags_p(Map_p(col,1),pnum) | avgrowflags_p[pnum];
      if( plotperrow_p == True ) return avgtheflags_p(Map_p(col,1),pnum) | avgrowflags_p[pnum];
      else return avgtheflags_p(Map_p(pnum,1),col) | avgrowflags_p[col];
   }
}

/*********************************************************************************/

Int CrossPlot::getNumPlots()
{
   if( ! Average_p )
   {
      //if( NRows_p == 1 ) return NRows_p;
      if( plotperrow_p == True ) return NRows_p;
      else return NPlots_p;
   }
   else 
   {
      //if( NAvgRows_p == 1 ) return NAvgRows_p;
      if( plotperrow_p == True ) return NAvgRows_p;
      else return NPlots_p;
   }
}

/*********************************************************************************/

Int CrossPlot::getNumRows()
{
   if( ! Average_p )
   {
      //if( NRows_p == 1 ) return NPlots_p;
      if( plotperrow_p == True ) return NPlots_p;
      else return NRows_p;
   }
   else
   {
      //if( NAvgRows_p == 1 ) return NPlots_p;
      if( plotperrow_p == True ) return NPlots_p;
      else return NAvgRows_p;
   }
}
/*********************************************************************************/


/*********************************************************************************/
void CrossPlot::CrossPlotError(String msg)
{
   throw AipsError("CrossPlot :: " + msg);
}

/*********************************************************************************/
/*********************************************************************************/

} //# NAMESPACE CASA - END 

