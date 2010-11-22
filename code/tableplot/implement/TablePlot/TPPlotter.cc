//# TPPlotter.cc: Plotter class for the TablePlot (tableplot) tool
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

/* Code Documentation : TPPlotter */
/*
  Handles the actual plotting.

  - User Plot options are sent in via TablePlot.
  - BasePlots are also sent in via TablePlot.
  - TPPlotter calls the BasePlot data access routines, to extract and populate
    plot arrays (whose format depends on the plotting package being used).
    
  Currently Two plotting packages are supported. 
  
  MATPLOTLIB : All documented operations.  The interaction with python is via direct 
          binding to the internal python interpreter. A new python module is
          created with functions to read X and Y data from C++, and to 
          send in flag-region co-ordinates. Commands are run within this
          interpreter, using the PyRun_String() command.
          The event-handling and flag-region creation is done by customized
          matplotlib TkAgg backend code. (TablePlotTkAgg.py).

          The data arrays are transferred from C++ to python via memory mapping.
                    
  The main persistant data member in TPPlotter is :

  [Ppar_p] : List of PanelParams objects. These hold plot information for each panel.
             These params are documented in PanelParams.h

  setPlotRange, SetFlagRegions, and plotData are the only functions of TPPlotter that
  directly interact with BasePlot/CrossPlot. Everything else is to manage the plotting
  parameters, and the interface between C++ and the plotting package.

*/

//# Includes

#include <cmath>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/OS/Timer.h>
#include <casa/iomanip.h>

#include <tableplot/TablePlot/TPPlotter.h>
#include <numpy/arrayobject.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/DataType.h>
#include <casa/Logging/LogIO.h>


namespace casa {  //# NAMESPACE CASA - BEGIN

#define TMR(a) "[User: " << a.user() << "] [System: " << a.system() << "] [Real: " << a.real() << "]"
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))

#define NYSEC (Double(86400.0))
  
#define LOG0 0 
#define LOG2 1 
/*********************************************************************************/
/********************************* MatPlotlib stuff ******************************/
/*********************************************************************************/

/* The python binding code is all global. */

extern PyObject* plotx_p;
extern PyObject* ploty_p;
//extern void initPyBind(void);

/*********************************************************************************/
/********************************* TPPPlotter Class ******************************/
/*********************************************************************************/

String TPPlotter::clname = "TPPlotter";

/* Default Constructor */
TPPlotter::TPPlotter()
{
   //extern casa::TPPlotter *GBTPLP;
   log = SLog::slog();
   String fnname= "TPPlotter";
#if LOG0
   log->FnEnter(fnname + "()", clname);
#endif
   
   setupPlotter();
   
   usegui_p = True;
#if LOG0
   log->FnExit(fnname, clname);
#endif
}
/*********************************************************************************/

/* No-GUI Constructor */
TPPlotter::TPPlotter(Bool usegui)
{
   log = SLog::slog();
   //extern casa::TPPlotter *GBTPLP;
   String fnname= "TPPlotter";
#if LOG0
   log->FnEnter(fnname + "(" + String::toString(usegui) + ")", clname);
#endif
   
   setupPlotter();
   usegui_p = usegui;
#if LOG0
   log->FnExit(fnname, clname);
#endif
}

/*********************************************************************************/
Int TPPlotter::setupPlotter()
{
   PlotPackage_p = MATPLOTLIB;

   px_p=NULL;
   py_p=NULL;
   PyInterp_p=NULL;

   PPar_p.resize(0,True);
   nflagmarks_p=0;
   nelem_p = 0;

   oldwindowsize_p = 8.0;
   oldaspectratio_p = 1.0;

   layernplots_p.resize(0);
   
   tcount1=0.0;
   tcount2=0.0;
   tcount3=0.0;
   tcount4=0.0;

   return 0;
}

/*********************************************************************************/

/* Destructor */
// TODO : Find a way to cleanly free the px_p, py_p  PyArrayObjects.
TPPlotter::~TPPlotter()
{
   String fnname= "~TPPlotter";
#if LOG0
   log->FnEnter(fnname + "()", clname);
#endif
   
   closePlot();
        
   if(PyInterp_p)
        delete PyInterp_p;
   PyInterp_p=NULL;
#if LOG0
   log->FnExit(fnname, clname);
#endif
   
}

/*********************************************************************************/
/* Query all BasePlots in the list and consolidate all plot ranges */
Int TPPlotter::setPlotRange(PtrBlock<BasePlot* > &PBP,Int panel)
{
   log = SLog::slog();
   String fnname= "setPlotRange";
#if LOG0
   log->FnEnter(fnname + "(PBP, panel)", clname);
#endif

#if LOG2
   log->out(String("nBP : ")+String::toString(PBP.nelements()), 
        fnname, clname, LogMessage::DEBUG1);
#endif
  
   if(PBP.nelements() != (PPar_p[panel-1]->LayerShowFlags).nelements()) 
      TPPlotterError(String("panel-bp mismatch"));
   
   /* Query all BasePlots in the list for individual plot ranges */
   Int i = 0;
   try {
   Xmin_p=1e+30; Xmax_p=-1e+30; Ymin_p=1e+30; Ymax_p=-1e+30;
   Double xmin=0,xmax=0,ymin=0,ymax=0;
   //cout << "PBP.nelements=" << PBP.nelements() << endl;
   for(i=0;i<(Int)PBP.nelements();i++)       
   {
      //os.DebugMessage("Getting ranges  ");
      PBP[i]->setPlotRange(xmin,xmax,ymin,ymax,
             (PPar_p[panel-1]->LayerShowFlags)[i],
             PPar_p[panel-1]->Plop.ColumnsXaxis,
             (PPar_p[panel-1]->LayerFlagVersions)[i],
             (PPar_p[panel-1]->LayerAverageNRows)[i],
             (PPar_p[panel-1]->LayerConnects)[i],
             PPar_p[panel-1]->Plop.DoScalingCorrection,
             PPar_p[panel-1]->LayerMultiColours[i],
             PPar_p[panel-1]->Plop.HonourXFlags);
      
      Xmin_p = MIN(Xmin_p,xmin);
      Xmax_p = MAX(Xmax_p,xmax);
      Ymin_p = MIN(Ymin_p,ymin);
      Ymax_p = MAX(Ymax_p,ymax);
      (PPar_p[panel-1]->PanelZrange)[0] = Xmin_p;
      (PPar_p[panel-1]->PanelZrange)[1] = Xmax_p;      
      (PPar_p[panel-1]->PanelZrange)[2] = Ymin_p;
      (PPar_p[panel-1]->PanelZrange)[3] = Ymax_p;   
   }
   }
   catch (...) {
      if (PPar_p[panel-1]->Plop.OverPlot) {
         log->out("Trying to overplot...",
                  fnname, clname, LogMessage::NORMAL4, True);
      }
      else {
           //cout << "do nothing" << endl;
      } 
   }

   /* Store plot range for this panel*/
   //(PPar_p[panel-1]->PanelZrange)[0] = Xmin_p;
   //(PPar_p[panel-1]->PanelZrange)[1] = Xmax_p;      
   //(PPar_p[panel-1]->PanelZrange)[2] = Ymin_p;
   //(PPar_p[panel-1]->PanelZrange)[3] = Ymax_p;   

   ///Apply user defined plotrange if supplied, and take care of zero ranges 
   adjustPlotRange(panel);      
#if LOG0
   log->FnExit(fnname, clname);   
#endif
   return 0;
}


/*********************************************************************************/

/* Plot data */
/* Plot parameters are stored in Ppar_p.
   Parameters for new plots need to be filled into Ppar_p.
   Parameters for lower plot layers need to be read out from Ppar_p.
*/

Int TPPlotter::plotData(PtrBlock<BasePlot* > &PBP,Int panel, Int newplot)
{
   log = SLog::slog();   
   String fnname= "plotData";
#if LOG2
   log->FnEnter(fnname + "(PBP, panel, newplot)", clname);
#endif
   tcount1=0; 
   tcount2=0; 
   tcount3=0; 
   tcount4=0;

   //Timer tmr;
   /* Set the plot range */
   setPlotRange(PBP,panel);

   /* Initialize the plotter */
   startPlot(panel,newplot);

   /* Set the plot labels */
   setPlotLabels(panel);

   /* Call 'thePlot' for all tables within each BasePlot in the list PBP. 
      This list
      corresponds to one TablePlot ATBPS entry (all layers for one panel).*/

#if LOG2
   ostringstream os;
   os << PPar_p[panel-1]->Plop.print() 
      << "\nNum layers : " << PBP.nelements() << "\n"
      << PPar_p[panel-1]->LayerNumbers <<" " 
      <<  PPar_p[panel-1]->MaxLayer; 
   log->out(os, fnname, clname, LogMessage::DEBUG1);
#endif

#if LOG2
   String msg = "Plotting data ...      ";
   msg += String::toString(PBP.nelements()) + " layers (thePlot)";
   log->out(msg, fnname, clname, LogMessage::DEBUG1);
#endif

   /* For overplot mode, GcGall thePlot only for new layers, and do not
      clear the plot in startPlot - so that the prev layers remain... */
      /// i.e.  only for "layerNumber" = maxlayer...
   for(Int layer=0;layer<(Int)PBP.nelements();layer++)
      if(!newplot ||  
         PPar_p[panel-1]->LayerNumbers[layer] == PPar_p[panel-1]->MaxLayer )
             thePlot(*PBP[layer],panel,layer);
   
   /* Finish plotting to window */
   endPlot(panel);
   
   /* Record the nplots in each layer */
   layernplots_p[panel-1].resize(PBP.nelements());
        for(Int layer=0;layer<(Int)PBP.nelements();layer++)
      (layernplots_p[panel-1])[layer] = PBP[layer]->getNumPlots();
#if LOG2
   log->out(String("Plotter timing : Allocation : ")
            + String::toString(tcount1) +  " sec, Filling : " 
            + String::toString(tcount2) +  " sec, Resizing : " 
            + String::toString(tcount3) +  " sec, Plot to Python : " 
            + String::toString(tcount4) +  " sec.", 
            fnname, clname, LogMessage::NORMAL5);
#endif
   //tmr.show("Plot: ");

#if LOG2
   log->FnExit(fnname, clname);
#endif
   return 0;
}

/*********************************************************************************/
/* Mark Flag Region */
/* Called by whoever has the co-ordinates of a flag region */

/* If called without the region vector, then it triggers the user-interaction 
   to create a region.
   The related Matplotlib code then calls this function again along with the 
   "region" */
Int TPPlotter::markRegions(Int nrows, Int ncols, Int panel,
                           Vector<Double> &region)
{
   String fnname= "markRegion";
#if LOG0 
   {
      ostringstream os;
      os << fnname << "(nrows=" << nrows << " ncols=" << ncols 
         << " panel=" << panel << ", region=" << region; 
      log->FnEnter(os, clname);
   }
#endif 

   /* If a region is not supplied, this 
      Triggers matplotlib user-interaction to mark regions.
      Returns 0. When the user marks a region, this function is 
      called again with the region values set */
   if(region.nelements() != 4)
   {
      markInteractiveRegion();
      return 0;
   }
   
   Int panindex=-1;
   Int cpanel=1;

   /* Check if this region is in a valid panel */
   if(nrows>0 && ncols>0)
   {
      panindex=-1;
      for(Int i=0;i<(Int)PPar_p.nelements();i++)
      {
#if LOG0 
         ostringstream os;
         os << "PanelMap for " << i << " is " 
            << PPar_p[i]->Plop.PanelMap;
         log->out(os, fnname, clname, LogMessage::DEBUG1);
#endif 
         if(PPar_p[i] != NULL && 
            PPar_p[i]->Plop.PanelMap[0]==nrows && 
            PPar_p[i]->Plop.PanelMap[1]==ncols && 
            PPar_p[i]->Plop.PanelMap[2]==panel) 
         {panindex = i; break;}
      }
      if(panindex==-1) {
         //TPPlotterError(String("Invalid nrows,ncols,panel : ")+
         //String::toString(nrows) + String(",") + 
         //String::toString(ncols) + String(",") + String::toString(panel));
         return 0;
      }
      else 
         cpanel = panindex+1;
   }
   else cpanel = 1;

#if LOG0 
   log->out("Finished marking region. Now updating flaglist",
            fnname, clname, LogMessage::DEBUG1);
#endif 
   
   /* Append to the list of marked flag regions for this panel */
   nflagmarks_p = (PPar_p[cpanel-1]->FlagList).nelements();
   (PPar_p[cpanel-1]->FlagList).resize(nflagmarks_p+1,True);
   (PPar_p[cpanel-1]->FlagList)[nflagmarks_p] = Vector<Double>(region);

#if LOG0 
   log->out(String("nflagmarks_p in panel ") 
             + String::toString(cpanel)
             + String(" is ")+String::toString(nflagmarks_p+1),
            fnname, clname, LogMessage::DEBUG1);
#endif 

#if LOG0
    ostringstream  os;
    for(Int i=0;i<nflagmarks_p+1;i++)
       os << (PPar_p[cpanel-1]->FlagList)[i] << "\n";
    log->out(os, fnname, clname, LogMessage::DEBUG1); 
#endif

#if LOG0 
   log->FnExit(fnname, clname);   
#endif 
   return 0;
}

/*********************************************************************************/

/* Adjust plot ranges */
/* PanelZrange holds the data ranges obtained from the BasePlot objects.
   It is expanded a bit if the data range = 0 in either dimension.
   User supplied ranges are then applied */

Int TPPlotter::adjustPlotRange(Int panel)
{
   String fnname= "adjustPlotRange";
#if LOG0 
   log->FnEnter(fnname + "(" + String::toString(panel) + ")", clname);
#endif 
    
   /* If zero range, expand the range a bit */
   Double expand=0.0;
   if( (PPar_p[panel-1]->PanelZrange)[0] == (PPar_p[panel-1]->PanelZrange)[1] )
   {
      if((PPar_p[panel-1]->PanelZrange)[0] >= 0) 
         expand = 0.1;
      else 
         expand = (PPar_p[panel-1]->PanelZrange)[0]*0.1;
      (PPar_p[panel-1]->PanelZrange)[0] -= expand;
      (PPar_p[panel-1]->PanelZrange)[1] += expand;
   }
   if( (PPar_p[panel-1]->PanelZrange)[2] == (PPar_p[panel-1]->PanelZrange)[3] )
   {
      if( (PPar_p[panel-1]->PanelZrange)[2] >= 0 ) expand = 0.1;
      else expand = (PPar_p[panel-1]->PanelZrange)[2]*0.1;
      (PPar_p[panel-1]->PanelZrange)[2] -= expand;
      (PPar_p[panel-1]->PanelZrange)[3] += expand;
   }
   

   /* If user plotrange is supplied, and if the min != max, and */
   /* plotrangesset is True then use it with the data ranges */
   if((Int)PPar_p[panel-1]->Plop.PlotRange.nelements() == 4)
   {
      if ((PPar_p[panel-1]->Plop.PlotRange)[1] != 
          (PPar_p[panel-1]->Plop.PlotRange)[0])
      {
         if ((PPar_p[panel-1]->Plop.PlotRangesSet)[0]) 
             (PPar_p[panel-1]->PanelZrange)[0] = 
                (PPar_p[panel-1]->Plop.PlotRange)[0];
         if ((PPar_p[panel-1]->Plop.PlotRangesSet)[1])
             (PPar_p[panel-1]->PanelZrange)[1] = 
                (PPar_p[panel-1]->Plop.PlotRange)[1];
      }
      
          
      if ((PPar_p[panel-1]->Plop.PlotRange)[3] != 
          (PPar_p[panel-1]->Plop.PlotRange)[2])
      {
         if ((PPar_p[panel-1]->Plop.PlotRangesSet)[2])
             (PPar_p[panel-1]->PanelZrange)[2] = 
                (PPar_p[panel-1]->Plop.PlotRange)[2];
         if ((PPar_p[panel-1]->Plop.PlotRangesSet)[3])
             (PPar_p[panel-1]->PanelZrange)[3] = 
               (PPar_p[panel-1]->Plop.PlotRange)[3];
      }

#if 0
      /* Trying to be a bit too smart... */
      /* If the user range is larger than the data range, use the data range */
      if((PPar_p[panel-1]->Plop.PlotRange)[1] != 
          (PPar_p[panel-1]->Plop.PlotRange)[0])
      {
         (PPar_p[panel-1]->PanelZrange)[0] = 
            MAX((PPar_p[panel-1]->PanelZrange)[0],
               (PPar_p[panel-1]->Plop.PlotRange)[0]);
         (PPar_p[panel-1]->PanelZrange)[1] = 
            MIN((PPar_p[panel-1]->PanelZrange)[1],
               (PPar_p[panel-1]->Plop.PlotRange)[1]);
      }
      if((PPar_p[panel-1]->Plop.PlotRange)[3] != 
         (PPar_p[panel-1]->Plop.PlotRange)[2])
      {
         (PPar_p[panel-1]->PanelZrange)[2] = 
            MAX((PPar_p[panel-1]->PanelZrange)[2],
              (PPar_p[panel-1]->Plop.PlotRange)[2]);
         (PPar_p[panel-1]->PanelZrange)[3] = 
            MIN((PPar_p[panel-1]->PanelZrange)[3],
            (PPar_p[panel-1]->Plop.PlotRange)[3]);
      }
#endif
   }
      
#if LOG0 
   ostringstream os;
      os  << "data range for panel " << panel << ": " 
      << (PPar_p[panel-1]->PanelZrange)[0] << " , " 
      << (PPar_p[panel-1]->PanelZrange)[1] << " , " 
      << (PPar_p[panel-1]->PanelZrange)[2] << " , " 
      << (PPar_p[panel-1]->PanelZrange)[3] ;
   log->out(os, fnname, clname, LogMessage::DEBUG1);
#endif 

#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/

/* Send region list to all BasePlots 
   so that it then knows what flags to write to the disk */
/* To ensure that this list is sent only once, clean up FlagList after 
   passing it into BasePlot */
Vector<Int> TPPlotter::setMarkedRegions(PtrBlock<PtrBlock<BasePlot* >* > &PPBP)
{
   String fnname= "setMarkedRegions";
#if LOG0 
   log->FnEnter(fnname + "(PPBP)", clname );
#endif 

   /* Read in the list of regions from matplotlib */
   //readInRegions();
   Vector<Int> numregions(PPar_p.nelements());
   numregions = 0;
   
   /* Send marked region info into all the BasePlots */
   for(Int panel=0;panel<(Int)PPBP.nelements();panel++)
   {
      if(PPBP[panel]!=NULL)
      {
         for(Int i=0;i<(Int)(*PPBP[panel]).nelements();i++)
            (*PPBP[panel])[i]->convertCoords(PPar_p[panel]->FlagList);
         
         /* Store the number of regions per panel, to return */
         numregions[panel] = (PPar_p[panel]->FlagList).nelements();
   
         /* Clean up the regionlist for each panel, after sending it into the BasePlots */
         (PPar_p[panel]->FlagList).resize(0);
      }
   }
#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   return numregions;
}

/*********************************************************************************/

/*********************************************************************************/
Int TPPlotter::setPlotOptions(PtrBlock<PanelParams* > &ppar)
{
   String fnname= "setPlotOptions";
#if LOG2 
   log->FnEnter(fnname + "(ppar)", clname);
#endif 
   PPar_p.resize(ppar.nelements());
   PPar_p = ppar;

   layernplots_p.resize(ppar.nelements(),True);
#if LOG0 
   log->out(String("NPPar_pelem here ! : ")
        + String::toString(PPar_p.nelements()),
        fnname, clname, LogMessage::DEBUG1);
#endif 
   
   /* Initialize the plotter */
#if LOG2
   log->out("Calling initPlot from setPlotOptions",
            fnname, clname, LogMessage::DEBUG1);
#endif
   initPlot();

#if LOG2 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}
/*********************************************************************************/
/*********************************************************************************/

/* Read data from BasePlots, apply flags and do the actual plotting */
/* If all overlays (chans,pols) are to be in the same colour, 
   then read them all out at once and plot using a single plot
   command. Otherwise, do them separately */
Int 
TPPlotter::thePlot( BasePlot &BP,
             Int panel,   // panel number
             Int layer)   // layer number (which PBP.)
{
   String fnname= "thePlot";
#if LOG0
   log->FnEnter(fnname, clname);
#endif
   Double xval,yval;

   /* Get the number of data points in each plot (nrows) */
   Int NRows = BP.getNumRows();
   /* Get the number of data points in this plot (nchans of npols)*/
   Int NPlots = BP.getNumPlots();
#if LOG2
   log->out(String("NRows=")+String::toString(NRows) 
            + String(" NPlots=")+String::toString(NPlots),
        fnname, clname, LogMessage::DEBUG1);
#endif

   Bool flagged = (PPar_p[panel-1]->LayerShowFlags)[layer];
   Int startcolour = PPar_p[panel-1]->LayerColours[layer];
   if (!PPar_p[panel-1]->Plop.useLayerColor)
      startcolour = PPar_p[panel-1]->LayerColours[0];
   //cout << "useLayerColor=" << PPar_p[panel-1]->Plop.useLayerColor << endl;
   //Int colourinc = 0;
   Int fpcnt = 0;
   Int allpcnt = 0;
   Bool chosen=False;
      
   /* Setup the mem. for numpoints to be plotted for each Plot */
   /* Attempts to be memory-smart.. and does not re-allocate if you need a
      smaller chunk than last time. If you need a larger chunk, then it
      re-allocates afresh */
   allocPlotArrays(NRows);

   /* Iterate over nchans or npols */
   for(Int Pnum=0;Pnum<NPlots;Pnum++)
   {
      
      tmr1.mark();
      
      pcnt_p=0;
      fpcnt=0;
      allpcnt=0;
            
      Int nplabels = PPar_p[panel-1]->LayerPointLabels[layer].nelements();
      pointlabels_p.resize(nplabels);
         
      tcount1 += tmr1.all();
      
      //cout << "panel=" << panel << " layer=" << layer
      //     << " xmin=" << (PPar_p[panel-1]->PanelZrange)[0]
      //     << " xmax=" << (PPar_p[panel-1]->PanelZrange)[1]
      //     << " ymin=" << (PPar_p[panel-1]->PanelZrange)[2]
      //     << " ymax=" << (PPar_p[panel-1]->PanelZrange)[3]
      //     << endl;

      tmr.mark();
      /* Fill up the Plot Arrays with data-to-be-plotted. */
      for(Int rc=0;rc<NRows;rc++)
      {
         /* Check for flags while reading values */
         // 'flagged' = True --> plot flagged data
         // 'flagged' = False --> plot unflagged data
         chosen = False;
         
         Bool flg = BP.getYFlags(Pnum,rc);
         //if (flg == (Bool)flagged)
         //   cout << "Pnum=" << Pnum << " rc=" << rc 
         //        << " flag=" << flg << endl;
         /* Choose points that match the required flag-state */
         if(flg == (Bool)flagged) 
         {
            xval = BP.getXVal(Pnum,rc);
            yval = BP.getYVal(Pnum,rc);
            //if (yval != 0.)
            //cout << " row " << std::setprecision(20) << rc 
            //     << " " << xval << "," << yval << endl;
         
            if(xval >= (PPar_p[panel-1]->PanelZrange)[0] && 
               xval <= (PPar_p[panel-1]->PanelZrange)[1] &&
               yval >= (PPar_p[panel-1]->PanelZrange)[2] && 
               yval <= (PPar_p[panel-1]->PanelZrange)[3])
            {
               //Skip N rows - applied to the final bunch of points to plot 
               if( allpcnt % PPar_p[panel-1]->LayerSkipNRows[layer] == 0 )
               {
                  
                  /* Fill the point into the python array */
                  fillPlotArrays(xval,yval,pcnt_p);
                  pcnt_p++;
                  
                  chosen = True;
                  
                  /* Attach a point label, if provided */
                  if(pcnt_p <= nplabels) 
                     pointlabels_p[pcnt_p-1] = 
                        PPar_p[panel-1]->LayerPointLabels[layer][rc];
               }
               
               /* Count the number of chosen points (without skipping) */
               allpcnt++;
            }
          }
          if( !chosen ) fpcnt++; 
      }
      tcount2 += tmr.all();
   
      totalpcnt_p += pcnt_p;
      totalflagpcnt_p += fpcnt;
         
      //cout << "totalpcnt_p=" << totalpcnt_p
      //     << " pcnt_p=" << pcnt_p
      //     << " totalflagpcnt_p=" << totalflagpcnt_p << endl;
      if(pcnt_p>0)
      {
         /* Resize python arrays before plotting */
         tmr2.mark();
         
         /* This reduces the size of the array to account for
            points not filled in due to being flaggged. It re-uses
            the same memory - and is therefore efficient */
         resizePlotArrays(pcnt_p);

         /* If number of points plotted is smaller than nplabels, resize
            to make the pointlabels match.. */
         if(pcnt_p < nplabels) pointlabels_p.resize(pcnt_p,True);
         
         tcount3 += tmr2.all();
         
         /* Plot */
         /* If plotting flagged points, do them in purple = -1 */
         tmr.mark();

         /* Get the colour add-on from BasePlot */
         plotXY(panel, layer, 
            flagged ? -1 :
       ((startcolour+BP.getColourAddOn(Pnum))%PPar_p[panel-1]->Plop.NColours));

         
         tcount4 += tmr.all();
      }// if(pcnt_p>0)
      
   }// for Pnum
#if LOG0
   log->FnExit(fnname, clname);
#endif
   return 0;
}

/*********************************************************************************/


/*********************************************************************************/


/*********************************************************************************/
/*********************************************************************************/
/******************* PLOTTING PACKAGE DEPENDANT FUNCTIONS ************************/
/*********************************************************************************/

/*********************************************************************************/

static void initialize_numpy( ) {
      import_array();
}

Int TPPlotter::initPlot()
{
   String fnname= "initPlot";
#if LOG2 
   log->FnEnter(fnname + "()", clname);
#endif 
      
   if(PlotPackage_p==MATPLOTLIB && !PyInterp_p)
   {

      initialize_numpy( );
                
#if LOG2 
      log->out("new interpreter.............", 
               fnname, clname, LogMessage::DEBUG1);
#endif 
      PyInterp_p = new CasaPyInterpreter(usegui_p);

      PyInterp_p->setupCustomGuiFeatures();
      PyInterp_p->pyrunString("pl.clf()\n" ); //DP
      PyInterp_p->pyrunString("pl.ioff()\n" ); //RI
#if LOG2 
      log->out(String("usegui : ")+String::toString(usegui_p),
               fnname, clname, LogMessage::DEBUG1);
#endif 
   }

#if LOG2 
   log->FnExit(fnname, clname);
#endif 
   
   return 0;
}
/*********************************************************************************/
Int TPPlotter::closePlot()
{
   String fnname= "closePlot";
#if LOG0 
   log->FnEnter(fnname + "()", clname);
#endif 
   if(PlotPackage_p==MATPLOTLIB)
   {
      if(PyInterp_p)
      {
         //cout << "TPLP::closePlot start" << endl;
         //PyInterp_p->pyrunString("pl.close('all')\n\n" ); 
         PyInterp_p->pyrunString("del pf\n" ); 
         PyInterp_p->pyrunString("del PlotFlag\n" ); 
         PyInterp_p->pyrunString("del PyBind\n" ); 
         //cout << "TPLP::closePlot end" << endl;
      }
   }

   return 0;
}

/*********************************************************************************/
Bool TPPlotter::closeWindow()
{
   String fnname= "closeWindow";
#if LOG0 
   log->FnEnter(fnname + "()", clname);
#endif 
   if(PlotPackage_p==MATPLOTLIB)
   {
      if(PyInterp_p)
      {
         //PyInterp_p->pyrunString("pl.close('all')\n\n" ); 
         PyInterp_p->pyrunString("pl.close()\n\n" ); 
      }
   }
   return True;
}

/*********************************************************************************/
Int TPPlotter::startPlot(Int panel, Int newplot)
{
   String fnname= "startPlot";
#if LOG2 
   log->FnEnter(fnname + "(" + String::toString(panel) +
                ", " + String::toString(newplot) + ")", clname);
#endif 
   totalpcnt_p=0;
   totalflagpcnt_p=0;

   
   /* Initialize the plotter */
#if LOG2
   log->out("Calling initPlot from startPlot",
            fnname, clname, LogMessage::DEBUG1);
#endif
   initPlot();
   PyInterp_p->pyrunString("pl.ioff()\n" ); //RI
   
   if (PlotPackage_p==MATPLOTLIB)
   {
      if (oldwindowsize_p != PPar_p[panel-1]->Plop.WindowSize || 
          oldaspectratio_p != PPar_p[panel-1]->Plop.AspectRatio)
      {
         oldwindowsize_p = PPar_p[panel-1]->Plop.WindowSize;
         oldaspectratio_p = PPar_p[panel-1]->Plop.AspectRatio;
         //the windowsize and aspect ratio has never worked.
         //yet, it involves a draw, be careful not to slow it down
         //setWindowOptions(oldwindowsize_p, oldaspectratio_p); //RI
      }

      ostringstream buf;
      buf << "currAxes = figman.canvas.figure.add_subplot("
         << PPar_p[panel-1]->Plop.PanelMap[0]<<","
         << PPar_p[panel-1]->Plop.PanelMap[1]<<","
         << PPar_p[panel-1]->Plop.PanelMap[2] <<")\n";

      PyInterp_p->pyrunString(buf.str().data() );

      PyInterp_p->pyrunString("figman.canvas.figure.subplots_adjust(top=0.9,bottom=0.1,left=0.125,right=0.9,wspace=0.3,hspace=0.7)\n" );      

      /* If it's replaceTopPlot... then pop the final layer ! */
      /* For each layer... pop for NPlots for that layer :-| */
      
      for(Int layer=0;layer<PPar_p[panel-1]->nBP;layer++)
         if (newplot && 
             PPar_p[panel-1]->Plop.ReplaceTopPlot && 
             PPar_p[panel-1]->Plop.OverPlot &&
             PPar_p[panel-1]->LayerNumbers[layer] == PPar_p[panel-1]->MaxLayer )
         {
            //cout << "Want to POP " << (layernplots_p[panel-1])[layer] 
            //     << " times." << endl;
            for(Int np=0;np<(layernplots_p[panel-1])[layer];np++)
            {
               //PyInterp_p->pyrunString("if(currAxes is not None):print 'lines=',len(currAxes.lines);\n\n");
               //PyInterp_p->pyrunString("if((currAxes is not None) and (len(currAxes.lines) > 0)):print 'OK to try...';\n\n");
               //try {
               PyInterp_p->pyrunString("if((currAxes is not None) and (len(currAxes.lines)>0)):currAxes.lines.pop();\n\n");
               //}
               //catch(...){}
               //PyInterp_p->pyrunString("if(currAxes is not None):print 'after...',currAxes.lines;\n\n");
            }
         }
         
      if (!newplot) 
         PyInterp_p->pyrunString("pl.cla()\n" );
      // PyInterp_p->pyrunString("pl.ioff()\n" );  //RI
      PyInterp_p->pyrunString("starttime = time.time()\n" );

   }
   
#if LOG2 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/
Int TPPlotter::show() //RI
{
  if(usegui_p) PyInterp_p->pyrunString("pl.show()\n" );  // show sets ion()
  return 0;
}

/*********************************************************************************/
Int TPPlotter::endPlot(Int panel)
{
   String fnname= "endPlot";
#if LOG0 
   log->FnEnter(fnname + "(" + String::toString(panel) + ")", clname);
#endif 

   log->out(String("Number of points being plotted : ")
               +String::toString(totalpcnt_p),
               fnname, clname, LogMessage::NORMAL2, True);
   log->out(String("Number of points NOT being plotted : ")
               +String::toString(totalflagpcnt_p),
               fnname, clname, LogMessage::NORMAL2);

   //if(totalpcnt_p == 0) {cout << "Not Plotting. " << endl; return 0;}

   if(PlotPackage_p==MATPLOTLIB) {

      if( totalpcnt_p > 0 )
      {
         /* Expand the plot window range by 5% */
         //Double expand = 0.02;
         Double expand = 0.05;
         Double xrange = (PPar_p[panel-1]->PanelZrange)[1] 
                       - (PPar_p[panel-1]->PanelZrange)[0];
         Double yrange = (PPar_p[panel-1]->PanelZrange)[3] 
                       - (PPar_p[panel-1]->PanelZrange)[2];
         Double xmin = (PPar_p[panel-1]->PanelZrange)[0] - expand * xrange;
         Double xmax = (PPar_p[panel-1]->PanelZrange)[1] + expand * xrange;
         Double ymin = (PPar_p[panel-1]->PanelZrange)[2] - expand * yrange;
         Double ymax = (PPar_p[panel-1]->PanelZrange)[3] + expand * yrange;
            
         ostringstream span;
         span.precision(20);
         span << "\n"
              << "spann = (" << xmin << "," << xmax << "," 
              << ymin << "," << ymax << ");\n"
              << "pl.axis(spann);\n";
         //cout << "Setting Plot range to: "
         //<< xmin << "," << xmax << "," << ymin << "," << ymax 
         //<< endl;
   
         /* expand axis */
         PyInterp_p->pyrunString(span.str().data() ); 
         
         /* Attach again to the correct panel */
         ostringstream buf;
         buf << "ax = figman.canvas.figure.add_subplot("
            << PPar_p[panel-1]->Plop.PanelMap[0]<<","
            << PPar_p[panel-1]->Plop.PanelMap[1]<<","
            << PPar_p[panel-1]->Plop.PanelMap[2] <<")\n";
         PyInterp_p->pyrunString(buf.str().data() );
         Int row = PPar_p[panel-1]->Plop.PanelMap[0];
         Int col = PPar_p[panel-1]->Plop.PanelMap[1];
   
         //cout << "row=" << row << " col=" << col << endl;
         if (row > 5 || col > 5) {
            //cout << "PPar_p[panel-1]->TimePlot=" << PPar_p[panel-1]->TimePlot << endl;
            //PyInterp_p->pyrunString("figman.canvas.figure.subplots_adjust(top=1,bottom=0.,left=0.,right=1.,wspace=0.,hspace=0.)\n" );      
            PyInterp_p->pyrunString("figman.canvas.figure.subplots_adjust(top=0.96,bottom=0.01,left=0.01,right=0.99,wspace=0.1,hspace=0.1)\n" );      
      
            /* For timeplots, rotate the tick labels, and add cursor-box formatting */
            if(PPar_p[panel-1]->TimePlot)
            {
               ostringstream fmt;
               
               /* Setup the cursor tracking label format */
               
               fmt << "pl.xlabel('')\n";
               fmt << "pl.ylabel('')\n";
               //fmt << "pl.title('')\n";
               
               fmt << "\ndef yearconv(x):";
               fmt << "\n  dt = pl.num2date(x);";
               fmt << "\n  return time.strftime('%d-%b-%Y %H:%M:%S',(dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second,0,0,0))+'.%d'%(int(dt.microsecond/1e5))";
               fmt << "\n\n";
      
               fmt << "\ndef timeconv(x, pos=0):";
               fmt << "\n  dt = pl.num2date(x);";
               fmt << "\n  return '%02d:%02d:%02d.%d ' % "
                      "(dt.hour,dt.minute,dt.second,(int(dt.microsecond/1e5)));";
               fmt << "\n\n";
      
               fmt << "\ndef conv(x):";
               fmt << "\n  return '%(p).5f'%{'p':x};";
               fmt << "\n\n";
               
               fmt << "\ntimeformatter = pl.FuncFormatter(timeconv);";
               fmt << "\n";
               
               
               /* Setup the tick label format */
               if(PPar_p[panel-1]->TimePlot==1 || PPar_p[panel-1]->TimePlot==3)
               {
                 fmt << "pl.setp( pl.gca().get_xticklabels(), Visible=False )\n";
                 fmt << "pl.setp( pl.gca().get_yticklabels(), visible=False )\n";
                  
                  fmt << "ax.fmt_xdata=yearconv\n";
                  fmt << "ax.fmt_ydata=conv\n";
      
                  fmt << "ax.xaxis.set_major_locator(MaxNLocator(6));\n";
                  //fmt << "ax.xaxis.set_major_formatter(pl.DateFormatter('%d-%b %H:%M:%S'));\n";
                  //fmt << "ax.xaxis.set_major_formatter(pl.DateFormatter(' %H:%M:%S '));\n";
                  fmt << "ax.xaxis.set_major_formatter(timeformatter)\n";
      
                  fmt << "labelFormatter = "
                         "pl.ScalarFormatter(useOffset=False,useMathText=True)\n";
                  fmt << "ax.yaxis.set_major_formatter(labelFormatter)\n";
      
                  //fmt << "pl.subplots_adjust(bottom=0.2);\n";
                  //fmt << "figman.canvas.figure.subplots_adjust(bottom=0.2);\n";
                  
                 
               }
               if(PPar_p[panel-1]->TimePlot==2 || PPar_p[panel-1]->TimePlot==3)
               {
                  fmt << "pl.setp( pl.gca().get_xticklabels(), Visible=False )\n";
                  fmt << "pl.setp( pl.gca().get_yticklabels(), visible=False )\n";
                   
                  fmt << "ax.fmt_xdata=conv\n";
                  fmt << "ax.fmt_ydata=yearconv\n";
      
                  //fmt << "ax.yaxis.set_major_formatter"
                  //        "(pl.DateFormatter('%d-%b %H:%M:%S'));\n";
                  //fmt << "ax.yaxis.set_major_formatter(pl.DateFormatter(' %H:%M:%S '));\n";
                  fmt << "ax.yaxis.set_major_formatter(timeformatter)\n";
      
                  fmt << "labelFormatter = "
                         "pl.ScalarFormatter(useOffset=False,useMathText=True)\n";
                  fmt << "ax.xaxis.set_major_formatter(labelFormatter)\n";
      
                  //fmt << "pl.subplots_adjust(left=0.2);\n";
                  //fmt << "figman.canvas.figure.subplots_adjust(left=0.2);\n";
                  
                  
               }
               
               PyInterp_p->pyrunString(fmt.str().data() ); 
            }
            else {
               ostringstream fmt;
               
                  fmt << "pl.setp( pl.gca().get_xticklabels(), Visible=False )\n";
                  fmt << "pl.setp( pl.gca().get_yticklabels(), visible=False )\n";
               
               /* Setup the cursor tracking label format */
                
               fmt << "\ndef conv(x):";
               fmt << "\n  return '%(p).5f'%{'p':x};";
               fmt << "\n\n";
               
                
               /* Setup the tick label format */
                
               fmt << "ax.fmt_xdata=conv\n";
               fmt << "ax.fmt_ydata=conv\n";
               
      
               fmt << "labelFormatter = pl.ScalarFormatter(useOffset=False,useMathText=True)\n";
               fmt << "ax.xaxis.set_major_formatter(labelFormatter)\n";
               fmt << "ax.yaxis.set_major_formatter(labelFormatter)\n";
               
               
               /* If the tick locations are to be altered, do this here */
               
               PyInterp_p->pyrunString(fmt.str().data() ); 
               
            }
      
   
         }
         else { 
            //cout << "PPar_p[panel-1]->TimePlot=" << PPar_p[panel-1]->TimePlot << endl;
      
            /* For timeplots, rotate the tick labels, and add cursor-box formatting */
            if(PPar_p[panel-1]->TimePlot)
            {
               ostringstream fmt;
               
               /* Setup the cursor tracking label format */
               fmt << "\ndef yearconv(x):";
               fmt << "\n  dt = pl.num2date(x);";
               fmt << "\n  return time.strftime('%d-%b-%Y %H:%M:%S',(dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second,0,0,0))+'.%d'%(int(dt.microsecond/1e5))";
               fmt << "\n\n";
      
               fmt << "\ndef timeconv(x, pos=0):";
               fmt << "\n  dt = pl.num2date(x);";
               fmt << "\n  return '%02d:%02d:%02d.%d ' % "
                      "(dt.hour,dt.minute,dt.second,(int(dt.microsecond/1e5)));";
               fmt << "\n\n";
      
               fmt << "\ndef conv(x):";
               fmt << "\n  return '%(p).5f'%{'p':x};";
               fmt << "\n\n";
               
               //fmt << "\nclass DsDateFormatter(pl.DateFormatter):";
               //fmt << "\n  def __call__(self, val, pos=0):";
               //fmt << "\n    fx = int(np.log(abs(val))/np.log(self._base) +0.5)
               //fmt << "\n    isDecade = self.is_decade(fx)
               //fmt << "\n   if not isDecade and self.labelOnlyBase:
               //fmt << "\n   return ''
               //fmt << "\n   return '%d'%fx 
      
               //fmt << "\ndef timeformat(x, pos):";
               //fmt << "\n  return "%G %%" % (100*(1-exp(-exp(y))))
      
               fmt << "\ntimeformatter = pl.FuncFormatter(timeconv);";
               fmt << "\n";
               
               /* Setup the tick label format */
               if(PPar_p[panel-1]->TimePlot==1 || PPar_p[panel-1]->TimePlot==3)
               {
                  fmt << "pl.setp( pl.gca().get_xticklabels(), rotation=0, "
                         "horizontalalignment='center', fontsize=" 
                      << PPar_p[panel-1]->Plop.FontSize*0.8 << ")\n";
                  fmt << "pl.setp( pl.gca().get_yticklabels(), fontsize=" 
                      << PPar_p[panel-1]->Plop.FontSize*0.8 << ")\n";
      
                  fmt << "ax.fmt_xdata=yearconv\n";
                  fmt << "ax.fmt_ydata=conv\n";
      
                  fmt << "ax.xaxis.set_major_locator(MaxNLocator(6));\n";
                  //fmt << "ax.xaxis.set_major_formatter(pl.DateFormatter('%d-%b %H:%M:%S'));\n";
                  //fmt << "ax.xaxis.set_major_formatter(pl.DateFormatter(' %H:%M:%S '));\n";
                  fmt << "ax.xaxis.set_major_formatter(timeformatter)\n";
      
                  fmt << "labelFormatter = "
                         "pl.ScalarFormatter(useOffset=False,useMathText=True)\n";
                  fmt << "ax.yaxis.set_major_formatter(labelFormatter)\n";
      
                  //fmt << "pl.subplots_adjust(bottom=0.2);\n";
                  fmt << "figman.canvas.figure.subplots_adjust(bottom=0.2);\n";
               }
               if(PPar_p[panel-1]->TimePlot==2 || PPar_p[panel-1]->TimePlot==3)
               {
                  fmt << "pl.setp( pl.gca().get_xticklabels(), fontsize=" 
                      << PPar_p[panel-1]->Plop.FontSize*0.8 << ")\n";
                  fmt << "pl.setp( pl.gca().get_yticklabels(), fontsize=" 
                      << PPar_p[panel-1]->Plop.FontSize*0.8 << ")\n";
      
                  fmt << "ax.fmt_xdata=conv\n";
                  fmt << "ax.fmt_ydata=yearconv\n";
      
                  //fmt << "ax.yaxis.set_major_formatter"
                  //        "(pl.DateFormatter('%d-%b %H:%M:%S'));\n";
                  //fmt << "ax.yaxis.set_major_formatter(pl.DateFormatter(' %H:%M:%S '));\n";
                  fmt << "ax.yaxis.set_major_formatter(timeformatter)\n";
      
                  fmt << "labelFormatter = "
                         "pl.ScalarFormatter(useOffset=False,useMathText=True)\n";
                  fmt << "ax.xaxis.set_major_formatter(labelFormatter)\n";
      
                  //fmt << "pl.subplots_adjust(left=0.2);\n";
                  fmt << "figman.canvas.figure.subplots_adjust(left=0.2);\n";
               }
               
               PyInterp_p->pyrunString(fmt.str().data() ); 
            }
            else {
               ostringstream fmt;
               
               fmt << "pl.setp( pl.gca().get_xticklabels(), fontsize=" 
                   << PPar_p[panel-1]->Plop.FontSize*0.8 << ")\n";
               fmt << "pl.setp( pl.gca().get_yticklabels(), fontsize=" 
                   << PPar_p[panel-1]->Plop.FontSize*0.8 << ")\n";
      
               /* Setup the cursor tracking label format */
               fmt << "\ndef conv(x):";
               fmt << "\n  return '%(p).5f'%{'p':x};";
               fmt << "\n\n";
               
               /* Setup the tick label format */
               fmt << "ax.fmt_xdata=conv\n";
               fmt << "ax.fmt_ydata=conv\n";
      
               //fmt << "\ndef sensible(value):";
               //fmt << "\n   if abs(value) > 1e5 or abs(value)<1e-4:";
               //fmt << "\n      return '%1.5e'% value ";
               //fmt << "\n      s = '%1.4e'% value ";
               //fmt << "\n      return self._formatSciNotation(s) ";
               //fmt << "\n   else:";
               //fmt << "\n      return '%4.3f' % value";
               //fmt << "\n\n";
      
               //fmt << "ax.fmt_xdata=sensible\n";
               //fmt << "ax.fmt_ydata=sensible\n";
               
               /*override default default + range label */
               //fmt << "majorFormatter = pl.FormatStrFormatter('%0.5f')\n";
               //fmt << "ax.xaxis.set_major_formatter(majorFormatter)\n";
               //fmt << "ax.yaxis.set_major_formatter(majorFormatter)\n";
      
               fmt << "labelFormatter = pl.ScalarFormatter(useOffset=False,useMathText=True)\n";
               fmt << "ax.xaxis.set_major_formatter(labelFormatter)\n";
               fmt << "ax.yaxis.set_major_formatter(labelFormatter)\n";
      
      
               /* If the tick locations are to be altered, do this here */
               
               PyInterp_p->pyrunString(fmt.str().data() ); 
            }
         }
      
      }
      
      //Now do the actual plotting
      if(usegui_p) PyInterp_p->pyrunString("pl.draw()\n" );

      //this ion may not be necessary, but it is a quick 
      //if(usegui_p) PyInterp_p->pyrunString("pl.ion()\n" );  //RI
      //log->out("Done Plotting data.",  fnname, clname, LogMessage::DEBUG2 );
      
   }
#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/
/* Change the state of a GUI button. */
Bool TPPlotter::changeGuiButtonState(String button, String state)
{
   String fnname= "changeGuiButtonsState"; 
#if LOG0 
   log->FnEnter(fnname + "(" + button + ", " + state + ")", clname);
#endif 
   initPlot();
   
   //cout << "Changing " << button << " to " << state << endl;

   if ( button.matches("iternext") ) {
      if( state.matches("enabled") ) 
          PyInterp_p->pyrunString("pf.enable_iter_button();\n");
      if( state.matches("disabled") ) 
          PyInterp_p->pyrunString("pf.disable_iter_button();\n");
      return True;
   }

   if( button.matches("markregion") ) {
      if( state.matches("enabled") ) 
         PyInterp_p->pyrunString("pf.enable_markregion_button();\n");
      if( state.matches("disabled") ) 
         PyInterp_p->pyrunString("pf.disable_markregion_button();\n");
      return True;
   }

   if( button.matches("flag") ) {
      if( state.matches("enabled") ) 
         PyInterp_p->pyrunString("pf.enable_flag_button();\n");
      if( state.matches("disabled") ) 
         PyInterp_p->pyrunString("pf.disable_flag_button();\n");
      return True;
   }

   if( button.matches("unflag") ) {
      if( state.matches("enabled") ) 
         PyInterp_p->pyrunString("pf.enable_unflag_button();\n");
      if( state.matches("disabled") ) 
         PyInterp_p->pyrunString("pf.disable_unflag_button();\n");
      return True;
   }

   if( button.matches("locate") ) {
     if( state.matches("enabled") ) 
         PyInterp_p->pyrunString("pf.enable_locate_button();\n");
     if( state.matches("disabled") ) 
         PyInterp_p->pyrunString("pf.disable_locate_button();\n");
     return True;
   }

   if( button.matches("clear") ) {
      if( state.matches("enabled") ) 
         PyInterp_p->pyrunString("pf.enable_clear_button();\n");
      if( state.matches("disabled") ) 
         PyInterp_p->pyrunString("pf.disable_clear_button();\n");
      return True;
   }

   if( button.matches("quit") ) {
      if( state.matches("enabled") ) 
         PyInterp_p->pyrunString("pf.enable_quit_button();\n");
      if( state.matches("disabled") ) 
         PyInterp_p->pyrunString("pf.disable_quit_button();\n");
      return True;
   }

   /* If control gets here, then the button/state is not valid */
   log->out("Internal Error : Invalid button name and/or state", 
             fnname, clname, LogMessage::SEVERE );
#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   return False;
}
/*********************************************************************************/

/*********************************************************************************/

Int TPPlotter::allocPlotArrays(Int size)
{
   //do not put too much debug messages!
   //log->FnEnter( "allocPlotArrays", "allocPlotArrays( size )" );
   
   if(PlotPackage_p==MATPLOTLIB)
   {
       npy_intp dim[1]; dim[0] = size;

      if(nelem_p < size) 
      /* can't resize back to full size :-( memory leak !! */
      {
         //log->out( "Need to make larger");
         px_p = PyArray_SimpleNew(1, dim, PyArray_DOUBLE);
         py_p = PyArray_SimpleNew(1, dim, PyArray_DOUBLE);
	 //Py_INCREF(px_p);
	 //Py_INCREF(py_p);
         nelem_p = size;

         //log->out(String("Mem alloc for pyarray : ")
         //     +String::toString(2*size*sizeof(double)/(1024.0*1024.0))
         //     +String(" MB "));
      }
      
      //log->out("PyArrayObj size (after) : " 
      //    +String::toString((((PyArrayObject *)px_p)->dimensions)[0]));
   }
   return 0;
}

/*********************************************************************************/
/* This needs to be very very light-weight */
Int TPPlotter::fillPlotArrays(Double xvalue, Double yvalue, Int index)
{
   static double *xtemp, *ytemp;
   if(PlotPackage_p==MATPLOTLIB)
   {
      ytemp = (double *)PyArray_DATA((PyArrayObject *)py_p);
      xtemp = (double *)PyArray_DATA((PyArrayObject *)px_p);
      xtemp[index] = xvalue;
      ytemp[index] = yvalue;
      //Py_INCREF((PyArrayObject *)px_p);
      //Py_INCREF((PyArrayObject *)py_p);
   }
   return 0;
}
   
/*********************************************************************************/
/* Resize. Re-Use memory. This is possible, because the number of flagged 
   or unflagged 
   points plotted is always less than or equal to the number of 
   "flagged + unflagged" points. */
Int TPPlotter::resizePlotArrays(Int numpoints)
{
   if(PlotPackage_p==MATPLOTLIB)
   {
      npy_intp dim[1]; dim[0] = numpoints;
      // These are needed for numpy arrays.
      plotx_p = PyArray_Copy((PyArrayObject *)PyArray_SimpleNewFromData(1,dim,PyArray_DOUBLE,
                PyArray_DATA((PyArrayObject *)px_p)));
      ploty_p = PyArray_Copy((PyArrayObject *)PyArray_SimpleNewFromData(1,dim,PyArray_DOUBLE,
                PyArray_DATA((PyArrayObject *)py_p)));
      
      //Py_INCREF((PyArrayObject *)plotx_p);
      //Py_INCREF((PyArrayObject *)plotx_p);
      // These are needed for numeric arrays.
      //plotx_p = PyArray_FromDimsAndData(1,dim,PyArray_DOUBLE,
      //          ((PyArrayObject *)px_p)->data);
      //ploty_p = PyArray_FromDimsAndData(1,dim,PyArray_DOUBLE,
      //          ((PyArrayObject *)py_p)->data);
      
      //os.DebugMessage("PyArrayObj size (after) : " 
      //        +String::toString((((PyArrayObject *)px_p)->dimensions)[0]));
   }
   return 0;
}
/*********************************************************************************/
/* Do the actual plotting. */
/* Take in colour here, only because colour is allowed to change within a BP. 
   i.e. for the same panel, layer */
/* NOTE: debugging messages commented out in this method, since there is */
/*       a number of them produced for each plot.                        */

Int 
TPPlotter::plotXY(Int panel, Int layer, Int colour)
{
   //log->FnEnter("plotXY", "plotXY( panel, layer, colour )" );
   //initPlot();
   if(PlotPackage_p==MATPLOTLIB)
   {
      ostringstream buf,tbuff;

      if(!PPar_p[panel-1]->TimePlot) 
          buf << "pl.plot(datx,daty";
      else 
          buf << "pl.plot_date(datx,daty";

      buf << ",'" << (PPar_p[panel-1]->LayerSymbols[layer]).c_str() << "'"
          << ", markersize=" << PPar_p[panel-1]->LayerMarkerSizes[layer]
          << ", linewidth=" << PPar_p[panel-1]->LayerLineWidths[layer] ;
   
      if( colour < -1 || colour >= PPar_p[panel-1]->Plop.NColours) 
          colour=1;
      if( colour == -1 ) 
          buf << ", color='magenta'";
      else 
          buf << ", color=" << PPar_p[panel-1]->Plop.ColourList[colour];

      /* 1 : x-axis, 2 : y axis, 3 : both. */
      if(PPar_p[panel-1]->TimePlot) 
      {
         if(PPar_p[panel-1]->TimePlot==1) buf << ", xdate=True, ydate=False";
         if(PPar_p[panel-1]->TimePlot==2) buf << ", xdate=False, ydate=True";
         if(PPar_p[panel-1]->TimePlot==3) buf << ", xdate=True, ydate=True";
      }
      
      buf << ");\n";

      //cout << buf.str() << endl;

      /* Map the NumArrays on disk - to lists in python */
      tbuff << "datx = PyBind.readXdata()\ndaty = PyBind.readYdata()\n";
      PyInterp_p->pyrunString(tbuff.str().data() );

      //PyInterp_p->pyrunString("print len(datx), len(daty);\n");

      /* Call the matplotlib 'plot' function */
      PyInterp_p->pyrunString(buf.str().data() );
      
      /* If labels for the first n point is specified */
      if(pointlabels_p.nelements())
      {
              double *ytemp = (double *)PyArray_DATA((PyArrayObject *)py_p);
              double *xtemp = (double *)PyArray_DATA((PyArrayObject *)px_p);
      //Py_INCREF((PyArrayObject *)px_p);
      //Py_INCREF((PyArrayObject *)py_p);

         for(Int i=0;i<(Int)pointlabels_p.nelements();i++)
         {
            if(i==pcnt_p)break;
            ostringstream dbuf;
            dbuf << "pl.text(" << xtemp[i] << "," 
                 << ytemp[i] << ",'" << pointlabels_p[i] << "')\n";
            PyInterp_p->pyrunString(dbuf.str().data() );
         }
      }
      
   }   
   
    //log->FnExit();
    return 0;
}

   
/*********************************************************************************/
Int TPPlotter::markInteractiveRegion()
{
   String fnname= "markinteractiveRegion";
#if LOG0 
   log->FnEnter(fnname + "()", clname);
#endif 
   initPlot();
   if(PlotPackage_p==MATPLOTLIB) {
      /* Register all the event handlers */
      PyInterp_p->pyrunString("print 'Mark regions on the plot...'\n" );
   }
#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/

Int TPPlotter::setWindowOptions(Double windowsize, Double aspectratio)
{
   String fnname= "setWindowOptions";
#if LOG0 
   log->FnEnter(fnname + "(windowsize, aspectratio)", clname);
#endif 
   initPlot();
   if(PlotPackage_p==MATPLOTLIB)
   {
      ostringstream buf;
      buf << "pl.figure(num=1, figsize=("<<windowsize<<","
	// <<windowsize*aspectratio<<"))\n";
          <<windowsize*aspectratio<<"))\npl.draw()\n";  //RI
      PyInterp_p->pyrunString(buf.str().data() ); 
   }
#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/

Int 
TPPlotter::setPlotLabels(Int panel)
{
   String fnname= "setPlotLabels";
#if LOG0 
   log->FnEnter(fnname + "(" + String::toString(panel) + ")", clname);
#endif 
 
   Int row = PPar_p[panel-1]->Plop.PanelMap[0];
   Int col = PPar_p[panel-1]->Plop.PanelMap[1];
   //cout << "row=" << row << " col=" << col << endl;



   /* TODO : move the plot around if the title has lots of \n in it */
   if(PlotPackage_p==MATPLOTLIB)
   {
      
      ostringstream tbuf,xbuf,ybuf;
      tbuf <<"pl.title('" << PPar_p[panel-1]->Plop.Title 
           <<"', fontsize=" << PPar_p[panel-1]->Plop.FontSize << ")\n";
      PyInterp_p->pyrunString(tbuf.str().data() ); 
      if (row > 5 || col > 5) {
         xbuf <<"pl.xlabel('" 
              <<"', fontsize=" << PPar_p[panel-1]->Plop.FontSize*0.9 
              << ")\n";
      }
      else {
      xbuf <<"pl.xlabel('" << PPar_p[panel-1]->Plop.XLabel 
           <<"', fontsize=" << PPar_p[panel-1]->Plop.FontSize*0.9 << ")\n";
      }

      PyInterp_p->pyrunString(xbuf.str().data() ); 
      if (row > 5 || col > 5) {
         ybuf <<"pl.ylabel('" 
              <<"', fontsize=" << PPar_p[panel-1]->Plop.FontSize*0.9 
              << ")\n";
      }
      else {
         ybuf <<"pl.ylabel('" << PPar_p[panel-1]->Plop.YLabel 
              <<"', fontsize=" << PPar_p[panel-1]->Plop.FontSize*0.9 << ")\n";
      }

      PyInterp_p->pyrunString(ybuf.str().data() ); 
      
      
      if (row > 5 || col > 5) {
         ostringstream fmt;      
         fmt << "pl.setp( pl.gca().get_xticklabels(), Visible=False )\n";
         fmt << "pl.setp( pl.gca().get_yticklabels(), visible=False )\n";
         PyInterp_p->pyrunString(fmt.str().data()); 
      }


      
      PyInterp_p->pyrunString(String("thetitle = '") 
                     + PPar_p[panel-1]->Plop.Title + String("';\n"));
      PyInterp_p->pyrunString("nlines = thetitle.count('\\n');\n");
      PyInterp_p->pyrunString("pl.subplots_adjust(top=0.9-0.04*nlines);\n");
   }
#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/
/* Place holder for plotting packages other than matplotlib... */
Int TPPlotter::readInRegions()
{
   if(PlotPackage_p==MATPLOTLIB)
   {
   
      //os.DebugMessage("TPLP : Reading in the region list");
      /** markRegions(xxxx) will get called from clickCoords **/
      //PyInterp_p->pyrunString( "if(not pf.flagmode):pf.start();\n" );
      //PyInterp_p->pyrunString("print 'reg list : ', pf.flagregionlist\n");
      //PyInterp_p->pyrunString("TPLP.clickCoords(pf.flagregionlist)\n");
      //PyInterp_p->pyrunString("pf.erase_rects()\n");
      //PyInterp_p->pyrunString("pf.close();\n" );
   
   }
   
   return 0;
}
/*********************************************************************************/
/* Either clear the whole window, or only a specific panel.
   For a simple replot on a panel, only clear the panel but don't clean
   up PPar_p for that panel (for use with TP.flagData */
Int TPPlotter::clearPlot(Int panel, Bool delaxes)
{
   String fnname= "clearPlot";
#if LOG0 
   log->FnEnter(fnname +  "(" + String::toString(panel) +
               ", " + String::toString(delaxes) + ")", clname);
#endif 
   //initPlot();

   //TODO -make this a tighter check.
   if(!PyInterp_p) 
   {
      log->out("No plot to clear !", fnname, clname, LogMessage::DEBUG1 );
      return 0;
   }

   if(panel==0)
   {
      /* Clear the whole window */
      if(PlotPackage_p==MATPLOTLIB)
      {

	//PyInterp_p->pyrunString( "pf.erase_rects();\n" );  //RI
         PyInterp_p->pyrunString("sp = figman.canvas.figure.get_axes();\n");
         
         //PyInterp_p->pyrunString("for ss in sp:\n  figman.canvas.figure.add_subplot(ss._rows,ss._cols,ss._num+1);\n  pl.cla();\n\n"); //RI

         //PyInterp_p->pyrunString( "pl.cla()\n" );  //RI
         PyInterp_p->pyrunString( "pl.clf()\n" ); 
         //PyInterp_p->pyrunString( "pf.close()\n" );
      }
   }
   else
   {
      /* Clear only specified panel */
      if(PlotPackage_p==MATPLOTLIB)
      {
#if LOG0 
         ostringstream os;
         os << "TPLP::Clearing (" 
            << PPar_p[panel-1]->Plop.PanelMap[0] << ","
            << PPar_p[panel-1]->Plop.PanelMap[1] << ","
            << PPar_p[panel-1]->Plop.PanelMap[2] << ")" ;
         log->out(os, fnname, clname, LogMessage::DEBUG1);
#endif 

         ostringstream buf;
         
         buf << "figman.canvas.figure.add_subplot("
             << PPar_p[panel-1]->Plop.PanelMap[0]
             <<","<< PPar_p[panel-1]->Plop.PanelMap[1]
             <<","<< PPar_p[panel-1]->Plop.PanelMap[2] <<")\n";
         
         PyInterp_p->pyrunString(buf.str().data() );
         
         PyInterp_p->pyrunString("figman.canvas.figure.subplots_adjust(top=0.9,bottom=0.1,left=0.125,right=0.9,wspace=0.3,hspace=0.7)\n" );      
         //PyInterp_p->pyrunString("pl.subplots_adjust(top=0.9,bottom=0.1,left=0.125,right=0.9,wspace=0.3,hspace=0.7)\n" );      
         
         //if(usegui_p)   //RI
	   //PyInterp_p->pyrunString("pl.ion()\n" );
         PyInterp_p->pyrunString("pl.cla()\n" ); 
         if(delaxes) 
            PyInterp_p->pyrunString("pl.delaxes()\n" ); 
         PyInterp_p->pyrunString("pl.ioff()\n" ); 
         //PyInterp_p->pyrunString("pf.erase_rects()\n" );
         
      }
   }
#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   return 0;
}

/*********************************************************************************/

/* Run a matplotlib command */
Int TPPlotter::runPlotCommand(String command)
{
   String fnname= "runPlotCommand";
#if LOG0 
   log->FnEnter(fnname + "(" + command + ")", clname);
#endif 
   if(PlotPackage_p==MATPLOTLIB)
   {
#if LOG2
      log->out("Calling initPlot from runplotcommand",
              fnname, clname, LogMessage::DEBUG1);
#endif
      initPlot();

      ostringstream buf;
      
      buf << command << endl;
      PyInterp_p->pyrunString(buf.str().data() ); 
      
   }
#if LOG0 
   log->FnExit(fnname, clname);
#endif 
   
   return 0;
}

/*********************************************************************************/
/*********************************************************************************/
void TPPlotter::TPPlotterError(String msg)
{
   throw AipsError("TPPlotter: " + msg);
}

}//# NAMESPACE CASA - END 

