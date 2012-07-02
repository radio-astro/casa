//# TPPlotter.h: Plotter class for the TablePlot (tableplot) tool
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
//# You should have receied a copy of the GNU Library General Public License
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
//# -------------------------------------------------------------------------
//# CHANGE LOG
//# Date         Name          Comments
//# 11/11/2006   S. Jaeger     Switched to use Matplotlib by default, by
//#                            removing the AIPS_USEMATPLOTLIB compile flag.
//#                            This may need to be added back in later, since
//#                            those using pgplot will be compiling both in.
//# 11/29/2006    S. Jaeger   Changed clearplot to allow flagged and 
//#            unflagged data to be displayed together when
//#            the overplot option is set to True.
//#            Added pointlabels param to thePlot() and plotXY()
//#            to allow point labels to be applied to each layer
//#            being plotted.
//# 12/22/2006  Urvashi R.V.   Added/Updated code documentation.
//#                            Modified condition in clearPlot, to allow for
//#                            correct 'memory' of plot parameters, when a
//#                            plot is replotted, after flagging.
//# 01/11/2007  Urvashi R.V.    Removed PGPLOT. Refactored the data structure
//#                             that deals with plot parameters per panel and
//#                             layer. 
//# 08/24/2007  Urvashi R.V.   Lots of changes + Removed the Python interpreter handing
//#                            from here into the CasaPyInterpreter class.

#ifndef TPPLOTTER_H
#define TPPLOTTER_H

//# Includes
#include <Python.h>

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Vector.h>

#include <tableplot/TablePlot/BasePlot.h>
#include <tableplot/TablePlot/PanelParams.h>
#include <tableplot/TablePlot/CasaPyInterpreter.h>
#include <tableplot/TablePlot/SLog.h>

//#include <CCM_Python/PySub.h>


// <summary>
// Plotter class for the TablePlot (tableplot) tool
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="">
// </reviewed>

// <prerequisite>
//#! Classes or concepts you should understand before using this class.
//   <li> BasePlot
//   <li> PanelParams
//   <li> PlotOptions
//   <li> CasaPyInterpreter
//   <li> Matplotlib, Python-Binding (numarray)
// </prerequisite>

// <etymology>
// TPPlotter is the class that controls all plot operations
// for the TablePlot (TP) class.
// </etymology>

// <synopsis>
// Class TPPlotter is the plotting interface that the TablePlot class
// uses. It has to be used along with the BasePlot and TablePlot classes.
// It currently manages direct binding to Python for the
// matplotlib plotting package.
//
// Other plotting packages can be attached to TPPlotter.
// For this, a set of functions need to be implemented using commands
// pertaining to this other plotting package. The set of functions that need
// to be duplicated are listed in TPPlotter.cc after the heading
// "PLOTTING PACKAGE DEPENDANT FUNCTIONS " and in this file, with the symbol "PPDF"
// </synopsis>

// <example>
// See TablePlot.cc
// </example>

// <motivation>
// This class was written to keep all plotting function calls and
// display window management issues independant of the BasePlot class
// that handles the actual tables. Also if the current plotting 
// package has to be replaced, changes need be made only in this one
// class. The TablePlot manipulates the TPPlotter objects.
// </motivation>

// <templating arg=T>
//    <li>
// </templating>

// <thrown>
//    <li>
//    <li>
// </thrown>


// <todo asof="$DATE:$">
//# A List of bugs, limitations, extensions or planned refinements.
//   <li> Figure out how to properly free the PyArrayObject memory : in the destructor.
// </todo>


namespace CCM_Python
{
  class Py;
  class PySub;
}

namespace casa { //# NAMESPACE CASA - BEGIN

enum {PGPLOT=0, MATPLOTLIB=1};

class TPPlotter 
{
   public:
      // Default Constructor
      TPPlotter(); 

      // Constructor that is to be used to decide between
      // GUI + command-line  vs only commmand-line mode.
      TPPlotter(Bool usegui); 
      
      // Destructor
      ~TPPlotter();
      
      // Set Plotting options from an input record of plot options.
      Int setPlotOptions(PtrBlock<PanelParams* > &ppar);
      
      // Read data from Baseplots and plot the data.
      // For the chosen panel, this function reads the data from
      // all BasePlots in the input list, applies the stored flags,
      // and 
      // (Will be improved to optionally plot with/without applying
      // flags, or plotting only flagged or unflagged data)
      Int plotData(PtrBlock<BasePlot* > &PBP,Int panel,Int newplot); 
      
      // Mark a rectangular region on the chosen panel with two
      // mouse clicks at diagonally opposite corners.
      // For matplotlib
      // This functions responds to a user-controlled rubber-band box
      // on the plot, to mark the flag region.
      // For pgplot
      // This function grabs two mouse click events via the plotting
      // package,and draws a hatched rectangle on the plot panel. 
      //
      // It then 
      // appends the area co-ordinates to a list of flag regions
      // maintained per panel in the TPPlotter class.
      Int markRegions(Int nrows, Int ncols, Int panel,Vector<Double> &region);
      
      // Passes on the list of marked flag regions (created by single
      // or multiple calls to MarkFlags) to each of the tables/subtables
      // in the input list of BasePlot objects.
      // Returns a vector of the number of flags per panel.
      Vector<Int> setMarkedRegions(PtrBlock<PtrBlock<BasePlot* >* > &PPBP);
      
      // PPDF : Back door for issuing commands to matplotlib
      Int runPlotCommand(String command);

      // PPDF : Clear the plot window
      Int clearPlot(Int panel=0, Bool delaxes=True);

      // Set up the plotter
      Int setupPlotter();

      // Change the state of a button
      // button : 'iternext','markregion','flag','unflag','locate'
      // PPDF : state : 'enable','disable'
      Bool changeGuiButtonState(String button, String state);

      // function to close the plot window......
      // PPDF : needs to be called just before the destructor.
      Bool closeWindow();
      
   private:
      // Consolidate ranges from all BasePlots and set plot range.
      // For the chosen panel, this function queries all the
      // BasePlot objects held in the input list for the ranges
      // of data being plotted from each BasePlot object. It then
      // combines these ranges along with zooming ranges (if applicable)
      // to create the final world-coordinate plot range for the
      // panel.
      Int setPlotRange(PtrBlock<BasePlot* > &PBP,Int panel); 
      
      // Do the actual plotting. Read xdata,ydata,flags from the input
      // BasePlot object, accumulate data-to-be-plotted (usually unflagged
      // data) into plot arrays and call the plotting package draw routines.
      // (Modify (if possible) to not have to accumulate data into
      // separate plot arrays before calling the plot draw routines. This
      // would be trivial if the plotting package draw routines could work
      // with a (Bool) mask.)
      Int thePlot(BasePlot &BP,Int panel, Int layer);

      // PPDF : Set plot environment parameters for a panel.
      // Sets up the viewport and window (world) coordinates from SetFlagRegions
      Int startPlot(Int panel, Int newplot);
      Int endPlot(Int panel);

      // The following are Plotting Package Specific functions.
      // If a new plotting package is to be used, these are the 
      // functions that need
      // to be modified.
      
      // PPDF : Initialize the plotter object
      Int initPlot();

      // PPDF : Clean up leftovers from the python interpreter.
      Int closePlot();
      
      // PPDF : Set windowsize, aspectratio
      Int setWindowOptions(Double windowsize, Double aspectratio);
      
      // PPDF : Set Plot labels from internally stored Vector of label strings.
      Int setPlotLabels(Int panel);

      // PPDF :Mark a Rectangular region (currently via 2 mouse clicks) and
      // draw a hatched rectangle to mark the region.
      Int markInteractiveRegion();

      // PPDF : Read in the user-selected flag regions from the plotting device.
      Int readInRegions();

      // PPDF : Manage memory for plot arrays into which data-to-be-plotted is
      // accumulated while plotting.
      Int allocPlotArrays(Int size);
      Int fillPlotArrays(Double xvalue, Double yvalue, Int index);
      Int resizePlotArrays(Int numpoints);
      
      // PPDF : Call the plotting function draw routines.
      Int plotXY(Int panel, Int layer, Int colour);

      // Adjust the plot range
      Int adjustPlotRange(Int panel);

      // Exceptions !
      void TPPlotterError(String msg);
      
      Int PlotPackage_p;

      // Execute and check python commands
      CasaPyInterpreter *PyInterp_p;

      // Pointers for PyArrayObject.
      PyObject *px_p,*py_p;
      Int nelem_p;
      Vector<String> pointlabels_p;
      Vector<Vector<Int> > layernplots_p;

      Double Xmin_p, Xmax_p, Ymin_p, Ymax_p;
      Int pcnt_p, totalpcnt_p, totalflagpcnt_p;
      Double oldwindowsize_p,oldaspectratio_p;
      
      Int nflagmarks_p;

      Timer tmr,tmr1,tmr2;
      Double tcount1,tcount2,tcount3,tcount4,tcount5;

      SLog* log;
      static String clname;

      // PlotOption and Panel-specific parameters.
      PtrBlock<PanelParams*> PPar_p;
      
      Bool usegui_p;
      
};


} //# NAMESPACE CASA - END 

#endif

