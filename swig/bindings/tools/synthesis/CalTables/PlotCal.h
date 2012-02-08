//# PlotCal.h Class to plot calibration tables
//# Copyright (C) 1996-2007
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#include <casa/aips.h>
#include <tableplot/TablePlot/TablePlot.h>


#ifndef CALIBRATION_PLOTCAL_H
#define CALIBRATION_PLOTCAL_H


namespace casa { //# NAMESPACE CASA - BEGIN

// forward declarations:
template <class T> class PtrBlock;
class Record;
class PlotOptions;
class TPConvertTimeX;

class PlotCal
  {

// <summary> 
// PlotCal: Class to plot calibration tables.
// </summary>
// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class="TablePlot">TablePlot</linkto> module
// </prerequisite>
//
// <etymology>
// From "Plot" and "Calibration Table"
// </etymology>
//
// <synopsis>
// To plot calibration tables ...encapsulates knowledge of 
// different cal tables 
// </synopsis>

// <todo asof="2005/12/31">
// (i) make use of cal-iterators
// </todo>

  public:
    //Default constructor
    PlotCal();

    //Destructor 
    virtual ~PlotCal();

    //open a calibration table
    Bool open(String tabName);

    //close...detach from tables but keep tablePlot alive
    void close();

    //Clear the plot or panel
    Bool clearPlot(Int panel=0);

    // Setting the plot look and feel
    Bool setPlotParam(Int subplot=111,
		      const Bool overplot=False,
		      const String& iteration="",
		      const Vector<Double>& plotrange=Vector<Double>(0),
		      const Bool& showflags=False,
		      const String& plotsymbol="ro",
		      const String& plotcolor="green",
		      const Double& markersize=5.0,
		      const Double& fontsize=10.0);
    
    // Select on caltable using MSSelection helpers
    Bool selectCal(const String& antenna="",
		   const String& field="",
		   const String& spw="",
		   const String& time="",
		   const String& poln="");

    //Plot the selection and table
    Bool oldplot(String plottype=String("PHASE"),
		 String poltype=String(""));

    Bool plot(String xaxis="",String yaxis="");

    // Save the plot to an image file.
    Bool saveFigure( const String& filename, 
		     const Int dpi, 
		     const String& orientation,
		     const String& papertype,
		     const String& facecolor,
		     const String& edgecolor );
    
    // Trigger next iteration
    Bool iterPlotNext();

    // stop iterations
    Bool iterPlotStop( Bool rmplotter );


    // mark a region
    Bool markRegion(Int nrows, Int ncols, Int panel, Vector<Double>& region);

    // flag the data
    Bool flagData(Bool direction);

    // Print Location of data
    Bool  locateData();
      
  private:

    void createCalTab(String& tabName);

    void getAxisTaQL(const String& axis,
		     String& taql,
		     String& label);
    Bool doPlot();

    Int multiTables(const Table& tablein,
		    Vector<Table>& tablesout,
		    Vector<String>& tablesoutnames,
		    Vector<Int>& cdlist,
		    Block<String> colNames);

    void subplotToPanel(const Int& subplot, Int& nrows, Int& ncols,
			Int& panel);
    //One more function which is very matplotlib specific
    void setPlotSymbols(const String& plotsymbol);

    //Fill the option record into plotoption 
    void fillPlotOptions();

    // Get MS Meta data for labels and locating
    void getMetaRecord();

    Vector<Int> getAntIdx(const String& antenna);
    Vector<Int> getFieldIdx(const String& field);
    Vector<Int> getSpwIdx(const String& spw,Matrix<Int>& chanId);
    Vector<Int> getCalDescIds(const Vector<Int> selspws);
    Matrix<Double> getTimeRange(const String& time);

    // get channel freq info for frequency labelling
    void getFreqInfo();

    // Handle BPOLY
    void virtualBPoly( Table& tabG, Int& nchan);
    Double getChebVal(const Vector<Double>& coeff,
		      const Double& xinit, const Double& xfinal,
		      const Double& x);

    // Handle GSPLINE
    void virtualGSpline( Table& tabG );
    Double getSplineVal (Double x, 
			 Vector<Double>& knots,
			 Vector<Double>& coeff);
    // Handle K 
    Bool timePlotK();
    void virtualKTab( Table& tabB, Int& nAnt, 
			       Vector<Int>& ant1hash, Vector<Int>& ant2hash );


    // Private data
    TablePlot *tp_p;

    String tabName_p;
    String msName_p;

    Table tab_p;
    Table tabSel_p;

    String calType_p;


    Int nxPanels_p, nyPanels_p; 
    Bool multiPlot_p;
    Bool overPlot_p;
    
    Bool antSel_p, fldSel_p,spwSel_p;

    Record plotopts_p;
    PlotOptions itsPlotOptions;

    String plotType_p, polType_p;
    String xAxis_p,yAxis_p;

    // This is the one that will have a flag column
    Table virtualTab_p;
    PtrBlock<Table *> overlayTabs_p;
    Vector<String> plotTaQL_p;
    Vector<String> label_p;

    Vector<String> iterAxes_p;
    Bool iterating_p;

    String title_p;
    String titleMain_p, titlePoln_p, titleSpw_p, titleFld_p, titleAnt_p;
    // whichPol_p determines which pol to plot
    // 0 = "R" or "X"
    // 1 = "L" or "Y"
    // 2 = "diff of phase " or "ratio of amplitude" between R and L or X and Y
    // 3 = "mean" of  "R" and "L" or "X" and "Y"  
    Int whichPol_p;

    Int nCalDesc_p;
    Matrix<Int> tabSpws_p,chanId_p;
    Vector<Double> startFreq_p;
    Vector<Double> stepFreq_p;
    Vector<Int> nchan_p;

    // The ResetCallBack
    casa::TPResetCallBack *resetCallBack_p;

    // A record containing meta info for callbacks
    Record metaRec_p;

    Vector< Vector<Int> > fldlist_p;

    Bool noMS_p;

    Vector<Int> MSstartChan_p;

  };


} //# NAMESPACE CASA - END

#endif
   
