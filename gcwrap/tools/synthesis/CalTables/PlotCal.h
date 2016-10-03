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
#include <tools/tables/TablePlot/TablePlot.h>
#include <synthesis/CalTables/NewCalTable.h>

#ifndef CALIBRATION_PLOTCAL_H
#define CALIBRATION_PLOTCAL_H


namespace casacore{

template <class T> class PtrBlock;
class Record;
}

namespace casa { //# NAMESPACE CASA - BEGIN

// forward declarations:
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
// From "Plot" and "Calibration casacore::Table"
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
    casacore::Bool open(casacore::String tabName);

    //close...detach from tables but keep tablePlot alive
    void close();

    //Clear the plot or panel
    casacore::Bool clearPlot(casacore::Int panel=0);

    // Setting the plot look and feel
    casacore::Bool setPlotParam(casacore::Int subplot=111,
		      const casacore::Bool overplot=false,
		      const casacore::String& iteration="",
		      const casacore::Vector<casacore::Double>& plotrange=casacore::Vector<casacore::Double>(0),
		      const casacore::Bool& showflags=false,
		      const casacore::String& plotsymbol="ro",
		      const casacore::String& plotcolor="green",
		      const casacore::Double& markersize=5.0,
		      const casacore::Double& fontsize=10.0);
    
    // Select on caltable using casacore::MSSelection helpers
    casacore::Bool selectCal(const casacore::String& antenna="",
		   const casacore::String& field="",
		   const casacore::String& spw="",
		   const casacore::String& time="",
		   const casacore::String& poln="");

    //Plot the selection and table
    casacore::Bool oldplot(casacore::String plottype=casacore::String("PHASE"),
		 casacore::String poltype=casacore::String(""));

    casacore::Bool plot(casacore::String xaxis="",casacore::String yaxis="");

    // Save the plot to an image file.
    casacore::Bool saveFigure( const casacore::String& filename, 
		     const casacore::Int dpi, 
		     const casacore::String& orientation,
		     const casacore::String& papertype,
		     const casacore::String& facecolor,
		     const casacore::String& edgecolor );
    
    // Trigger next iteration
    casacore::Bool iterPlotNext();

    // stop iterations
    casacore::Bool iterPlotStop( casacore::Bool rmplotter );


    // mark a region
    casacore::Bool markRegion(casacore::Int nrows, casacore::Int ncols, casacore::Int panel, casacore::Vector<casacore::Double>& region);

    // flag the data
    casacore::Bool flagData(casacore::Bool direction);

    // Print Location of data
    casacore::Bool  locateData();
      
  private:

    void createCalTab(casacore::String& tabName);

    void getAxisTaQL(const casacore::String& axis,
		     casacore::String& taql,
		     casacore::String& label);
    casacore::Bool doPlot();

    casacore::Int multiTables(const casacore::Table& tablein,
		    casacore::Vector<casacore::Table>& tablesout,
		    casacore::Vector<casacore::String>& tablesoutnames,
		    casacore::Vector<casacore::Int>& cdlist,
		    casacore::Block<casacore::String> colNames);

    void subplotToPanel(const casacore::Int& subplot, casacore::Int& nrows, casacore::Int& ncols,
			casacore::Int& panel);
    //One more function which is very matplotlib specific
    void setPlotSymbols(const casacore::String& plotsymbol);

    //Fill the option record into plotoption 
    void fillPlotOptions();

    // Get casacore::MS Meta data for labels and locating
    void getMetaRecord();

    casacore::Vector<casacore::Int> getAntIdx(const casacore::String& antenna);
    casacore::Vector<casacore::Int> getFieldIdx(const casacore::String& field);
    casacore::Vector<casacore::Int> getSpwIdx(const casacore::String& spw,casacore::Matrix<casacore::Int>& chanId);
    casacore::Vector<casacore::Int> getCalDescIds(const casacore::Vector<casacore::Int> selspws);
    casacore::Matrix<casacore::Double> getTimeRange(const casacore::String& time);

    // get channel freq info for frequency labelling
    void getFreqInfo();

    // Handle BPOLY
    void virtualBPoly( casacore::Table& tabG, casacore::Int& nchan);
    casacore::Double getChebVal(const casacore::Vector<casacore::Double>& coeff,
		      const casacore::Double& xinit, const casacore::Double& xfinal,
		      const casacore::Double& x);

    // Handle GSPLINE
    void virtualGSpline( casacore::Table& tabG );
    casacore::Double getSplineVal (casacore::Double x, 
			 casacore::Vector<casacore::Double>& knots,
			 casacore::Vector<casacore::Double>& coeff);
    // Handle K 
    casacore::Bool timePlotK();
    void virtualKTab( casacore::Table& tabB, casacore::Int& nAnt, 
			       casacore::Vector<casacore::Int>& ant1hash, casacore::Vector<casacore::Int>& ant2hash );

    // Return which MAIN table column to return for "CAL_DESC_ID",
    //  depending on whether we are plotting a new or old table
    inline casacore::String CDIcol() { return (isNCT_p ? "SPECTRAL_WINDOW_ID" : "CAL_DESC_ID"); };
    inline casacore::String GAINcol() { return (isNCT_p ? "CPARAM" : "GAIN"); };

    // Private data
    TablePlot *tp_p;

    casacore::String tabName_p;
    casacore::String msName_p;

    casacore::Table tab_p;
    casacore::Table tabSel_p;

    casacore::String calType_p;


    casacore::Int nxPanels_p, nyPanels_p; 
    casacore::Bool multiPlot_p;
    casacore::Bool overPlot_p;
    
    casacore::Bool antSel_p, fldSel_p,spwSel_p;

    casacore::Record plotopts_p;
    PlotOptions itsPlotOptions;

    casacore::String plotType_p, polType_p;
    casacore::String xAxis_p,yAxis_p;

    // This is the one that will have a flag column
    casacore::Table virtualTab_p;
    casacore::PtrBlock<casacore::Table *> overlayTabs_p;
    casacore::Vector<casacore::String> plotTaQL_p;
    casacore::Vector<casacore::String> label_p;

    casacore::Vector<casacore::String> iterAxes_p;
    casacore::Bool iterating_p;

    casacore::String title_p;
    casacore::String titleMain_p, titlePoln_p, titleSpw_p, titleFld_p, titleAnt_p;
    // whichPol_p determines which pol to plot
    // 0 = "R" or "X"
    // 1 = "L" or "Y"
    // 2 = "diff of phase " or "ratio of amplitude" between R and L or X and Y
    // 3 = "mean" of  "R" and "L" or "X" and "Y"  
    casacore::Int whichPol_p;

    casacore::Int nCalDesc_p;
    casacore::Matrix<casacore::Int> tabSpws_p,chanId_p;
    casacore::Vector<casacore::Double> startFreq_p;
    casacore::Vector<casacore::Double> stepFreq_p;
    casacore::Vector<casacore::Int> nchan_p;

    // The ResetCallBack
    casa::TPResetCallBack *resetCallBack_p;

    // A record containing meta info for callbacks
    casacore::Record metaRec_p;

    casacore::Vector< casacore::Vector<casacore::Int> > fldlist_p;

    casacore::Bool noMS_p;

    casacore::Vector<casacore::Int> MSstartChan_p;

    // If true, we are plotting a NewCalTable, else the old kind
    casacore::Bool isNCT_p;

    NewCalTable ct_p;

  };


} //# NAMESPACE CASA - END

#endif
   
