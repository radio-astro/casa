//# PlotCal.cc: Implementation of PlotCal.h
//# Copyright (C) 1996,1997,1998,2001,2002,2003
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
//----------------------------------------------------------------------------
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/TableInfo.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ExprNode.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/GenSort.h>
#include <casa/Exceptions/Error.h>
#include <casa/Quanta/MVTime.h>
#include <casa/OS/Time.h>
#include <casa/iostream.h>
#include <tableplot/TablePlot/TablePlot.h>
#include <graphics/Graphics/PGPlotterLocal.h>
#include <graphics/Graphics/PGPLOT.h>
#include <casa/Logging/LogIO.h>
#include <casa/sstream.h>
#include <casa/BasicSL/Constants.h>
#include <casa/System/PGPlotter.h>
#include <calibration/CalTables/BJonesTable.h>
#include <calibration/CalTables/BJonesMCol.h>
#include <calibration/CalTables/GJonesTable.h>
#include <calibration/CalTables/GJonesMBuf.h>
#include <calibration/CalTables/CalIter.h>
#include <calibration/CalTables/PlotCal.h>
#include <calibration/CalTables/PlotCalHooks.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>

namespace casa { //# NAMESPACE CASA - BEGIN

#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define cheb cheb_
#define getbspl getbspl_
#endif

extern "C" {
  void cheb(Int*, Double*, Double*, Int*);
  void getbspl(Int*, Double*, Double*, Double*, Double*, Int*);
}

/******************/

  PlotCal::PlotCal():  
    tp_p(NULL),
    tabName_p(""),
    msName_p(""),
    calType_p(""),
    nxPanels_p(1), 
    nyPanels_p(1),
    multiPlot_p(False), 
    overPlot_p(False), 
    plotType_p("PHASE"),
    polType_p(""),
    xAxis_p(""),
    yAxis_p(""),
    iterAxes_p(),
    iterating_p(False),
    titleMain_p(""), 
    whichPol_p(0),
    nCalDesc_p(0),
    tabSpws_p(),
    startFreq_p(),
    stepFreq_p(),
    resetCallBack_p(NULL),
    noMS_p(True),
    MSstartChan_p()
{

  // Attach to TablePlotInstance
  tp_p = TablePlot::TablePlotInstance();
      
  // The reset callback
  resetCallBack_p = new casa::PlotCalReset( this );
  tp_p->setResetCallBack("PlotCal",resetCallBack_p);

  //    tp_p->changeGuiButtonState("markregion","disabled");
  //    tp_p->changeGuiButtonState("flag","disabled");
  //    tp_p->changeGuiButtonState("unflag","disabled");
  //    tp_p->changeGuiButtonState("locate","disabled");
  tp_p->changeGuiButtonState("iternext","disabled");

}

  PlotCal::~PlotCal(){

    close();

    tp_p->clearResetCallBack("PlotCal");
    tp_p=NULL;
  }

  Bool PlotCal::open(String tabName){

    // This shouldn't happen
    if(tp_p ==NULL)
      throw(AipsError("PlotCal is hopelessly disabled! Restart casapy"));

    // Create/discern the cal table
    createCalTab(tabName);

    // Initialize plot options
    setPlotParam();

    whichPol_p=0;

    return True;
  }

  void  PlotCal::close(){

    //break the link with original table
    tabSel_p=Table();
    tab_p=Table();
    tabName_p="";
    msName_p="";

    virtualTab_p=Table();

  }


  Bool PlotCal::clearPlot(Int subplot){

    Int nx, ny , panel;
    subplotToPanel(subplot, nx, ny, panel);
    
    tp_p->clearPlot(nx,ny,panel);
    return True;
  }
  
  Bool PlotCal::setPlotParam(Int subplot,
                             const Bool overplot,
                             const String& iteration,
                             const Vector<Double>& plotrange,
                             const Bool& showflags,
                             const String& plotsymbol,
                             const String& plotcolor,
                             const Double& markersize,
                             const Double& fontsize) {

   LogIO os(LogOrigin("PlotCal", "setPlotParam", WHERE));

   // Initialize
   plotopts_p = Record();

   // Handle panels
   Int nxPanels, nyPanels, panel;
   subplotToPanel(subplot, nyPanels, nxPanels, panel);
   nxPanels_p=nxPanels;
   if(nxPanels_p < 1)
     nxPanels_p=1;
   nyPanels_p=nyPanels;
   if(nyPanels_p < 1)
     nyPanels_p=1;
   plotopts_p.define("panel", panel);
   plotopts_p.define("ncols", nxPanels_p);
   plotopts_p.define("nrows",nyPanels_p);


   // Handle overplot && iteration
   multiPlot_p=False;
   iterAxes_p.resize();
   if (iteration.length()>0) {

     String ucIter = upcase(iteration);

     iterAxes_p.resize(4);
     Int iter=0;
     if ( ucIter.contains("ANT") || 
	  ucIter.contains("BASE") ) {
       iterAxes_p(iter)="ANTENNA1";
       ++iter;
     }	 
     if ( ucIter.contains("TIME") ) {
       iterAxes_p(iter)="TIME";
       ++iter;
     }
     if ( ucIter.contains("SPW") ) {
       iterAxes_p(iter)="CAL_DESC_ID";
       ++iter;
     }
     if ( ucIter.contains("FIELD") ) {
       iterAxes_p(iter)="FIELD_ID";
       ++iter;
     }
     iterAxes_p.resize(iter,True);
     multiPlot_p=True;
   }

   // Handle overplot (multiplot trumps overplot)
   overPlot_p = overplot;
   if (multiPlot_p && overplot) {
     cout << "NB: Plot iteration requested, turning off overplot" << endl;
     overPlot_p=False;
   }
   plotopts_p.define("overplot", overPlot_p);

   // Clear the existing plot, if we are not overPloting
   //  TBD: don't do this part for now....
   //   if (!overPlot_p) {
   //     tp_p->clearPlot(nyPanels_p,nxPanels_p,panel);
   //
   //     cout << "itsPlotOptions.CallBackHooks = " << itsPlotOptions.CallBackHooks << endl;
   //     // TBD: should we do this?
   //     if (itsPlotOptions.CallBackHooks!=NULL)
   //       delete itsPlotOptions.CallBackHooks;
   //     itsPlotOptions.CallBackHooks=NULL;
   //   }

   // Are we showing flags?
   plotopts_p.define("showflags",showflags);

   // The plot range
   Vector<Double> pltr=plotrange;
   if (plotrange.nelements()<4)
     pltr=Vector<Double>(4,0.0);
   plotopts_p.define("plotrange",pltr);
   plotopts_p.define( RecordFieldId("plotrangesset"), Vector<Bool>(4,True) );

   //Define the plot symbol as defined by matplotlib...
   setPlotSymbols(plotsymbol);
   plotopts_p.define("color", plotcolor);

   plotopts_p.define("markersize",markersize);
   plotopts_p.define("fontsize",fontsize);
   //   plotopts_p.define("windowsize", 10.0);
   //   plotopts_p.define("aspectratio",0.8);


    //    cout << boolalpha << "plotopts_p = " << endl << plotopts_p << endl;

   // Fill the plotoptions object
   fillPlotOptions();
   
   //    cout << itsPlotOptions.print() << endl;
   
   return True;

  }

Bool PlotCal::selectCal(const String& antenna,
			const String& field,
			const String& spw,
			const String& time,
			const String& poln) {

  LogIO os(LogOrigin("plotcal", "selectCal", WHERE)); 
  
  Vector<Int> antId; 
  Vector<Int> fldId;
  Vector<Int> spwId;
  
  Bool antSel=False;
  Bool fldSel=False;
  Bool spwSel=False;
  Bool timeSel=False;
  
  if (msName_p=="" || !Table::isReadable(msName_p)) {
    cout << "Note: Either your CalTable pre-dates name-based selection, or" << endl;
    cout << "      does not (yet) support selection, or the MS associated" << endl;
    cout << "      with this cal table does not exist.  All antennas," << endl;
    cout << "      fields, spws are being selected for plotting." << endl;

    noMS_p=True;
  }
  else {

    noMS_p=False;
    
    // Use MSSelection-assistance
    antId=getAntIdx(antenna);
    if (antenna.length()>0 && antId.nelements()==0)
      throw(AipsError("Specified antenna(s) select no calibration solutions."));
    if(antId.nelements()>0 && antId(0)!=-1)
      antSel=True;
    
    fldId=getFieldIdx(field);
    if (field.length()>0 && fldId.nelements()==0)
      throw(AipsError("Specified field(s) select no calibration solutions."));
    if(fldId.nelements()>0 && fldId(0)!=-1)
      fldSel=True;
    
    spwId=getSpwIdx(spw);
    if (spw.length()>0 && spwId.nelements()==0)
      throw(AipsError("Specified spw(s) select no calibration solutions."));
    if(spwId.nelements()>0 && spwId(0)!=-1) 
      spwSel=True;
    
    if(time.length()>0)
      timeSel=True;

    // Polarization selection (includes '/')
    polType_p=upcase(poln);

  }
  
  if(antSel || fldSel || spwSel || timeSel){
    
    TableExprNode condition;
    
    if(antSel){
      String col="ANTENNA1";
      condition=tab_p.col(col).in(antId);
    }
    if(fldSel){
      String col="FIELD_ID";
      if(condition.isNull())
	condition=tab_p.col(col).in(fldId);
      else
	condition=condition && tab_p.col(col).in(fldId);
    }
    if(spwSel){

      String col="CAL_DESC_ID";
      Vector<Int> caldescids=getCalDescIds(spwId);

      if (caldescids.nelements() > 0) {
	if(condition.isNull())
	  condition=tab_p.col(col).in(caldescids);
	else
	  condition=condition && tab_p.col(col).in(caldescids);
      }
      else {
	throw(AipsError("Selected spws not found in cal table."));
      }
    }
    
    if (timeSel) {
      
      Matrix<Double> timeRng=getTimeRange(time);
      
      if (timeRng.nrow()>0) {
	
	TableExprNode timeTEN;
	
	for (uInt itm=0;itm<timeRng.ncolumn();++itm) {
	  
	  // Form single time selection
	  TableExprNode thistimeTEN;
	  Vector<Double> thistime(timeRng.column(itm));
	  if (thistime(0)!=thistime(1)) {
	    thistimeTEN=(tab_p.col("TIME") >= min(thistime));
	    thistimeTEN=thistimeTEN && (tab_p.col("TIME") <= max(thistime));
	  }
	  else {
	    // Handle single time specification (within +/- 30s
	    thistimeTEN=(tab_p.col("TIME") >= thistime(0)-30.0);
	    thistimeTEN=thistimeTEN && (tab_p.col("TIME") <=thistime(0)+30);
	  }
	  
	  // Combine with global time selection
	  if (!thistimeTEN.isNull()) {
	    if (timeTEN.isNull())
	      timeTEN=thistimeTEN;
	    else
	      timeTEN=timeTEN || thistimeTEN;
	  }
	  
	}
	
	// Combine with global selection
	if (!timeTEN.isNull()) {
	  if (condition.isNull())
	    condition = timeTEN;
	  else
	    condition = condition && timeTEN;
	}
      }
    }
    
    // Only if 
    if (!condition.isNull()) {

      tabSel_p=Table();
      tabSel_p=tab_p(condition);
      tabSel_p.rename(tab_p.tableName()+".plotCal", Table::New);
      tabSel_p.flush();
      tabSel_p.markForDelete();

      if(tabSel_p.nrow()==0) {
	tabSel_p=tab_p;
	throw(AipsError("Combined selection selects nothing."));
	return False;
      }
    }
  }
  else 
    // Select whole cal table for plotting
    tabSel_p = tab_p;
  
  return True;
}

Bool PlotCal::plot(String xaxis, String yaxis) {

  LogIO os(LogOrigin("PlotCal", "plot", WHERE));

  xAxis_p=upcase(xaxis);
  yAxis_p=upcase(yaxis);

  if(tabName_p==""){
    os << LogIO::WARN << "No caltable has been specified."
       << LogIO::POST;
    return False;
  }
  
  // Disable the Next button for now
  if (!multiPlot_p) 
    tp_p->changeGuiButtonState("iternext","disabled");
    
  if(calType_p=="G" || calType_p=="T" || calType_p=="GSPLINE" || 
     calType_p=="B" || calType_p=="BPOLY"  ||
     calType_p=="M" || calType_p=="MF" ||
     calType_p=="D")
    return doPlot();

  else if(calType_p=="K")
    //    return timePlotK();
    throw(AipsError("K plots are disabled for now."));
  
  else if(calType_p=="X")
    throw(AipsError("X plots are disabled for now."));

  else 
    throw(AipsError("The cal table you specified is not supported yet."));
  
  return False;

}


Bool
PlotCal::saveFigure( const String& filename, const Int dpi, 
      const String& orientation, const String& papertype,
      const String& facecolor,	const String& edgecolor )
{
    LogIO os(LogOrigin("PlotCal", "saveFigure", WHERE));
    if(tabName_p==""){
	os << LogIO::WARN << "No caltable has been specified."
	   << LogIO::POST;
	return False;
    }

    Bool ret = 
	tp_p->saveFigure( filename, dpi, orientation, papertype, facecolor, edgecolor );

    return ret;
}

  Bool PlotCal::iterPlotNext(){

    // Do nothing if not iterating
    if(!iterating_p) return False;

    Vector<String> labcol;
    Vector<Vector<Double> > labval;
    Int ret;
    ret=tp_p->iterMultiPlotNext(labcol, labval);

    if (ret==-1) iterPlotStop(False);

    return True;

  }

  Bool PlotCal::iterPlotStop( Bool rmplotter ){

    tp_p->iterMultiPlotStop( rmplotter );
    iterating_p=False;

    // Disable the Next button
    tp_p->changeGuiButtonState("iternext","disabled");

    return True;
  }
  
  Bool PlotCal::markRegion(Int nrows, Int ncols, Int panel, Vector<Double>& region){
    tp_p->markRegions(nrows, ncols, panel, region);
    return True;
  }


  Bool PlotCal::flagData(Bool direction){

    tp_p->flagData( direction );
    return True;      

  }

Bool  PlotCal::locateData()
{
  Vector<String> selectColumns(1);
  selectColumns[0] = "";
  
  PtrBlock<Record*> data;
  tp_p->locateData( selectColumns, data );
  
  // Clenup the memory that was allocated -- table plot doesn't know
  // when we are done with this info so it doesn't do it for us!
  for ( uInt i=0; i < data.nelements(); i++ )
    {
      delete data[i];
      data[i] = NULL;
    }
  data.resize(0);
  
  return True;
}


void PlotCal::getAxisTaQL(const String& axis,
			  String& taql,
			  String& label) {

  LogIO os(LogOrigin("PlotCal", "getAxisTaQL", WHERE));
  
  if (axis.contains("TIME")) {
    taql=String("(TIME/86400.0)+678576.0");
    label = "Time";
  }
  else if (axis.contains("FREQ")) {
    taql="CROSS";
    label= "Frequency (MHz)";
  }
  else if (axis.contains("CHAN")) {
    taql="CROSS";
    label= "Channel";
    if (calType_p=="BPOLY")
      label = "Arbitrary Channel";
  }
  else if (axis.contains("ANTENNA")) {
    taql="ANTENNA1";
    label= "Antenna INDEX";
  }
  else if (axis.contains("PARANG")) {
    taql="TIME";
    label= "Parallactic Angle (deg) (actually Time)";
  }
  else {
    
    if ( polType_p.contains("/") ) {
      // Ratio plots
      if (calType_p == "T" || calType_p == "BPOLY") {
	String errmsg="Polarization ratio plots not supported for "+calType_p+" tables.";
	throw(AipsError(errmsg));
      }
      
      String defval("0.0");
      if (axis.contains("AMP") ) {
	taql = "(AMPLITUDE(GAIN[1,]/GAIN[2,]))";
	label = "Gain Amplitude POLN Ratio";
      }
      else if (axis.contains("REAL") ) {
	taql = "(REAL(GAIN[1,]/GAIN[2,]))";
	label = "Gain: Real Part POLN Ratio";
      }
      else if (axis.contains("IMAG") ) {
	taql = "(IMAG(GAIN[1,]/GAIN[2,]))";
	label = "Gain: Imaginary Part POLN Ratio";
      }
      else if (axis.contains("PHASE")) {
	taql = "((180.0/PI())*ARG(GAIN[1,]/GAIN[2,]))";
	label = "Gain Phase POLN Difference (deg)";
      }
      else if (axis.contains("SNR") ) {
	if (calType_p=="M" || 
	    calType_p=="MF" || 
	    calType_p=="BPOLY" || 
	    calType_p=="GSPLINE")
	  throw(AipsError("Baseline-based calibration types don't support SNR plots."));
	taql = "(SNR[1,]/SNR[2,])";
	label = "Solution SNR POLN Ratio";
      }
      else 
	throw(AipsError("The plot value you requested is not supported."));
      
      // Expand TaQL to "handle" flag info properly
      taql = 
	"IIF(ARRAY(ANYS(FLAG[1:2,],1),[SHAPE(FLAG)[2]]),0.0,ARRAY("+taql+",[SHAPE(FLAG)[2]]))";
      //"IIF(ARRAY(ANYS(FLAG[,],1),[SHAPE(GAIN)[2],SHAPE(GAIN)[1]]),0.0,ARRAY("+taql+",[SHAPE(GAIN)[2],SHAPE(GAIN)[1]]))";
    }
    else {
      
      String polsel("");
      if (polType_p=="R" || polType_p=="X")
	polsel="1";
      if (polType_p=="L" || polType_p=="Y")
	polsel="2";
      
      if (axis.contains("AMP") ) {
	taql = "(AMPLITUDE(GAIN["+polsel+",]))";
	label = "Gain Amplitude";
      }
      else if (axis.contains("REAL") ) {
	taql = "(REAL(GAIN["+polsel+",]))";
	label = "Gain: Real Part";
      }
      else if (axis.contains("IMAG") ) {
	taql = "(IMAG(GAIN["+polsel+",]))";
	label = "Gain: Imaginary Part";
      }
      else if (axis.contains("PHASE")) {
	taql = "((180.0/PI())*ARG(GAIN["+polsel+",]))";
	label = "Gain Phase (deg)";
      }
      else if (axis.contains("SNR") ) {
	if (calType_p=="M" || calType_p=="MF")
	  throw(AipsError("Baseline-based calibration types don't support SNR plots."));
	taql = "(SNR["+polsel+",])";
	label = "Solution SNR";
      }
      else 
	throw(AipsError("The plot value you requested is not supported."));
    }
  }
  
}



Bool PlotCal::doPlot(){

  LogIO os(LogOrigin("PlotCal", "doPlot", WHERE));

  // Catch bad y-axis  
  if (yAxis_p.contains("TIME"))
    throw(AipsError("Plotting time on y-axis if forbidden."));
  
  // Handle unspecified axes  
  if (xAxis_p=="")
    if(calType_p=="G" || 
       calType_p=="T" || 
       calType_p=="M" || 
       calType_p=="GSPLINE") 
      xAxis_p="TIME";
    else if (calType_p=="D")
      xAxis_p="ANTENNA";
    else
      xAxis_p="CHAN";
  
  if (yAxis_p=="")
    if (calType_p=="D")
      yAxis_p="AMP";
    else
      yAxis_p="AMP";


  // Get meta info for labels/locate
  getMetaRecord();

  // The table for plotting (we may slice it later)
  Table plotTab;

  Int nchan;
  if (calType_p=="BPOLY") {
    // form a virtual "B" table
    virtualBPoly(plotTab, nchan);

    // BPOLY can only plot vs. Frequency for now
    if (xAxis_p=="CHAN")
      xAxis_p="FREQ";
    if (xAxis_p=="TIME")
      throw(AipsError("BPOLY plotting does not support xaxis=time"));


    // Disable flagging on BPOLY
    tp_p->changeGuiButtonState("flag","disabled");
    tp_p->changeGuiButtonState("unflag","disabled");
  }
  else if (calType_p=="GSPLINE") {
    // form a virtual "B" table
    virtualGSpline(plotTab);

    if (xAxis_p=="CHAN" || 
	xAxis_p=="FREQ")
      throw(AipsError("GSPLINE plotting does not support xaxis=freq or chan"));

    // Disable flagging on GSPLINE
    tp_p->changeGuiButtonState("flag","disabled");
    tp_p->changeGuiButtonState("unflag","disabled");
  }
  else {
    // just use the selected table
    plotTab=tabSel_p;
    // Flagging is enables for other plots
    tp_p->changeGuiButtonState("flag","enabled");
    tp_p->changeGuiButtonState("unflag","enabled");

    // Only get freq info if we need it
    if (xAxis_p=="FREQ")
      getFreqInfo();
  }

  // Get the TaQLs formation section---------------------
  label_p.resize(3);
  plotTaQL_p.resize(2);


  getAxisTaQL(xAxis_p,plotTaQL_p(0),label_p(1));
  getAxisTaQL(yAxis_p,plotTaQL_p(1),label_p(2));

  titleMain_p = (calType_p + String(" table: ") + tabName_p);
  label_p[0]=titleMain_p;

  // ---------------------------------------------

  try {

    // Stop any prior iteration condition
    tp_p->iterMultiPlotStop();
    iterating_p=False;

    Block<String> layercols;

    // Manage the plotting options
    fillPlotOptions();
    itsPlotOptions.Title=label_p[0];
    itsPlotOptions.XLabel=label_p[1];
    itsPlotOptions.YLabel=label_p[2];
    itsPlotOptions.MultiColour="cellrow";
    itsPlotOptions.CallBackHooks = new PlotCalCallBacks(metaRec_p);

    
    //    if (calType_p=="BPOLY") {
    //      itsPlotOptions.MultiColour="cellrow";
      // itsPlotOptions.Connect = "cellcol";
    //    }
    // Setup time axis
    if (xAxis_p.contains("TIME"))
      itsPlotOptions.TimePlotChar="x";
    else
      itsPlotOptions.TimePlotChar="o";

    if(multiPlot_p){

      Int nlayers=0;
      layercols.resize(nlayers);

      if (iterAxes_p.nelements()>0) {

	//	cout << "Iterating on " << iterAxes_p << endl;
	
   
	if (!anyEQ(iterAxes_p,String("CAL_DESC_ID"))) {
	  nlayers++;
	  layercols.resize(nlayers,True);
	  layercols[nlayers-1]="CAL_DESC_ID";
	}
   
	if (!anyEQ(iterAxes_p,String("ANTENNA1"))) {
	  nlayers++;
	  layercols.resize(nlayers,True);
	  layercols[nlayers-1]="ANTENNA1";
	}

    /*
	if (!anyEQ(iterAxes_p,String("FIELD_ID"))) {
	  nlayers++;
	  layercols.resize(nlayers,True);
	  layercols[nlayers-1]="FIELD_ID";
	}
    */

   /*
	cout << "Layering (multi-color): ";
	for (uInt i=0;i<layercols.nelements();++i)
	  cout << layercols[i] << " ";
	cout << endl;
   */

      }

      Vector<Table> vt;
      Vector<String> tabnames;
      Vector<String> dummysel;
      Vector<Int> cdlist;

      Int numLayer=0;
      numLayer=multiTables(plotTab,
			   vt,tabnames,
			   cdlist,
			   layercols);

      dummysel.resize(numLayer);
      dummysel.set("");

      if (xAxis_p.contains("FREQ") || xAxis_p.contains("CHAN") )
	itsPlotOptions.Convert= new PlotCalFreqAxes(cdlist,startFreq_p,stepFreq_p);
      
      tp_p->setTableT(vt,tabnames,dummysel);
      tp_p->checkInputs(itsPlotOptions,plotTaQL_p,iterAxes_p);
      if(tp_p->iterMultiPlotStart( itsPlotOptions, plotTaQL_p,iterAxes_p)==-1){
	cout << "Error in iteration plot initialization." << endl;
	return False;
      }

      iterating_p=True;
      
      Vector<String> labcol;
      Vector<Vector<Double> > labval;
      tp_p->iterMultiPlotNext(labcol,labval);
      tp_p->changeGuiButtonState("iternext","enabled");

      cout << "NOTE: Click Next button to advance plots" << endl;

    }
    else{

      iterating_p=False;

      layercols.resize(2);
      layercols[0]="CAL_DESC_ID";
      layercols[1]="ANTENNA1";

      Vector<String> iterAxes;
      Vector<Table> vt;
      Vector<String> tabnames;
      Vector<String> dummysel;
      Vector<Int> cdlist;

      Int numAnt=multiTables(plotTab,vt,tabnames,cdlist,layercols);

      dummysel.resize(numAnt);
      dummysel.set("");

      if (xAxis_p.contains("FREQ") || xAxis_p.contains("CHAN") )
	itsPlotOptions.Convert= new PlotCalFreqAxes(cdlist,startFreq_p,stepFreq_p);

      if (xAxis_p.contains("PARANG"))
	itsPlotOptions.Convert= new PlotCalParang(fldlist_p);
      
      tp_p->setTableT(vt,tabnames,dummysel);
      tp_p->checkInputs(itsPlotOptions,plotTaQL_p,iterAxes);
      tp_p->plotData(itsPlotOptions, plotTaQL_p);

    }

  } catch(AipsError x){

    close();
    throw(x);

  }
  
  return True;

}
  
  
  Bool PlotCal::timePlotK(){

    cout << "K plotting disabled for the moment." << endl;

    return False;

 /*
    iterating_p=False;

    label_p.resize(3);
    plotopts_p.define("timeplot", "x");
 
    label_p[2]=String("Phase in degrees"); 
    label_p[0]=string("K solution");
    Vector<String> plotstr(2);
    
    Vector<Int> ant1hash;
    Vector<Int> ant2hash;
    Int nAnt;

    //    Table tabM;
    virtualKTab(virtualTab_p, nAnt, ant1hash, ant2hash);

    Table storeTab=tabSel_p;
    tabSel_p=virtualTab_p;
    try{
      plotstr[0]=String("(TIME/86400.0)+678576.0");
      plotstr[1]=plotType_p;
      
      if(plotType_p=="AMP"){
	label_p[2]="Amplitude";
      }
      if(plotType_p=="SNR"){
	label_p[2]="SNR";
      }
      if(plotType_p=="DELAY"){
	label_p[2]="Delay  (ns)";
      }
      if(plotType_p=="DELAYRATE"){
	label_p[2]="Delay rate (ps/s)";
      }
	
      plotTaQL_p=plotstr;
      

      if(multiPlot_p){
	Vector<String> iterAxes(1);
	iterAxes[0]="BASELINE";

	Int numDesc=0;
	numDesc=multiTablesInt("CAL_DESC_ID");
	Vector<Table> vt(numDesc);
	Vector<String> tabnames(numDesc);
	tabnames.set(tabName_p);
	Vector<String> dummysel(numDesc);
	dummysel.set("");
	
	for (Int k=0; k < numDesc; ++k){
	  vt(k)=*overlayTabs_p[k];
	}
	tp_p->setTableT(vt,dummysel, dummysel);
	fillPlotOptions();
	itsPlotOptions.Title=label_p[0];
	itsPlotOptions.XLabel=label_p[1];
	itsPlotOptions.YLabel=label_p[2];
	if(tp_p->iterMultiPlotStart( itsPlotOptions, plotstr,iterAxes)==-1){
          cout << "Error in iteration plot initialization." << endl;
	  return False;
	}
	
	iterating_p=True;
	iterPlotNext();
      }
      else{
	Int numAnt=multiTablesInt("BASELINE");
	Vector<Table> vt(numAnt);
	Vector<String> tabnames(numAnt);
	tabnames.set(tabName_p);
	Vector<String> dummysel(numAnt);
	dummysel.set("");
	for (Int k=0; k < numAnt; ++k){
	  vt(k)=*overlayTabs_p[k];
	}
	tp_p->setTableT(vt,dummysel,dummysel);
	fillPlotOptions();
	itsPlotOptions.Title=label_p[0];
	itsPlotOptions.XLabel=label_p[1];
	itsPlotOptions.YLabel=label_p[2];
	tp_p->plotData(itsPlotOptions, plotTaQL_p);

      }
    }
    catch (AipsError x){
      tabSel_p=storeTab;
      throw(x);
    }

    tabSel_p=storeTab;

    // be sure non-multiplot case is closed properly (for now)
    if (!multiPlot_p) close();
    
    return True;
 */
  }


Int PlotCal::multiTables(const Table& tablein,
			 Vector<Table>& tablesout,
			 Vector<String>& tablesoutnames,
			 Vector<Int>& cdlist,
			 Block<String> colNames){
  
  TableIterator titer(tablein,colNames);
  
  Int ntab(0);
  while (!titer.pastEnd()) {
    ntab++;
    titer.next();
  }

  fldlist_p.resize(ntab);

  tablesout.resize(ntab);
  tablesoutnames.resize(ntab);
  cdlist.resize(ntab);
  
  titer.reset();

  Int itab=0;
  while (!titer.pastEnd() && itab<ntab) {
    tablesout(itab) = titer.table();
    tablesoutnames(itab)=tablesout(itab).tableName();

    cdlist(itab) = ROScalarColumn<Int>(tablesout(itab),"CAL_DESC_ID")(0);
    if (cdlist(itab)<0) cdlist(itab)=0;

    fldlist_p(itab).resize();
    fldlist_p(itab) = ROScalarColumn<Int>(tablesout(itab),"FIELD_ID").getColumn();

    //    cout << "fldlist_p(itab) = " << fldlist_p(itab) << endl;

    itab++;
    titer.next();
  }

  return ntab;

}


  void PlotCal::createCalTab(String& tabName){
    LogIO os(LogOrigin("plotcal", "createCalTab", WHERE));
    if(!Table::isReadable(tabName)) {
      os << LogIO::SEVERE << "Calibration table " << tabName 
	 << " does not exist " 
	 << LogIO::POST;
    }
    tab_p=Table(tabName, Table::Update); //need to write the flags in 
    if(!tab_p.tableInfo().type().contains("Calibration")){
      os << LogIO::SEVERE << "Table " << tabName 
	 << " is not a calibration table " 
	 << LogIO::POST;
    }

    tabName_p = tabName;

    //default selection is whole
    tabSel_p=tab_p;
   
    String subType[2];
    split(tab_p.tableInfo().subType(), subType, 1, String(" "));
 
    if(subType[0].contains("G")){
      if (tab_p.tableInfo().subType()=="GSPLINE")
	calType_p="GSPLINE";
      else
	calType_p="G";
    }
    else if(subType[0].contains("B")){
      if (tab_p.tableInfo().subType()=="BPOLY")
	calType_p="BPOLY";
      else
	calType_p="B";
    }
    else if(subType[0].contains("D")){
      calType_p="D";
    }
    else if(subType[0].contains("T")){
      calType_p="T";
    }
    else if(subType[0].contains("Mf")){
      calType_p="MF";
    }
    else if(subType[0].contains("M")){
      calType_p="M";
    }
    else if(subType[0].contains("K")){
      calType_p="K";
    } 
    else if(subType[0].contains("X")){
      calType_p="X";
    } 

    // Get meta data from CAL_DESC subtable
    msName_p="";
    String cdtabname=tabName+"/CAL_DESC";
    if (Table::isReadable(cdtabname)) {

      Table cdtab(cdtabname);

      nCalDesc_p=cdtab.nrow();
      if (nCalDesc_p>0) {

	ROArrayColumn<Int> spwcol(cdtab,"SPECTRAL_WINDOW_ID");
	tabSpws_p.resize();
	tabSpws_p = spwcol.getColumn();
	
	// frequencies are channel numbers, for starters
	startFreq_p.resize(nCalDesc_p);
	startFreq_p=0.0;
	stepFreq_p.resize(nCalDesc_p);
	stepFreq_p=1.0;
	
	MSstartChan_p.resize(nCalDesc_p);
	MSstartChan_p=0;
	if (calType_p=="B" || calType_p=="MF") {
	  ROArrayColumn<Int> stchancol(cdtab,"CHAN_RANGE");
	  Array<Int> stchan=stchancol.getColumn();
	  for (Int i=0;i<nCalDesc_p;++i) {
	    MSstartChan_p(i)=stchan(IPosition(4,0,0,0,i));
	    startFreq_p(i)=Double(MSstartChan_p(i));
	  }
	}

	ROScalarColumn<String> msNameCol(cdtab,"MS_NAME");
	msName_p = msNameCol(0);

	// Add cal table relative path to the ms name
	msName_p = Path(tabName_p).dirName() + "/" + msName_p;
	
	// get the full (presumed) absolute path to the MS, so Table doesn't
	//  get it wrong
	msName_p = Path(msName_p).absoluteName();

	//	cout << "msName_p = " << msName_p << endl;

      }
    }
  }

  void PlotCal::virtualKTab( Table& tabB, Int& nAnt,  
			     Vector<Int>& ant1hash, Vector<Int>& ant2hash ){

    TableDesc td("", "1", TableDesc::Scratch);
    td.comment() = "A memory K table to have the array size to satisfy tableplot";
    td.addColumn (ScalarColumnDesc<Int>("ANTENNA1"));
    td.addColumn (ScalarColumnDesc<Int>("ANTENNA2"));
    td.addColumn (ScalarColumnDesc<Int>("BASELINE"));
    td.addColumn (ArrayColumnDesc<Float>("PHASE"));
    td.addColumn (ArrayColumnDesc<Float>("AMP"));
    td.addColumn (ArrayColumnDesc<Float>("DELAY"));
    td.addColumn (ArrayColumnDesc<Float>("DELAYRATE"));
    td.addColumn (ScalarColumnDesc<Int>("CAL_DESC_ID"));
    td.addColumn (ArrayColumnDesc<Bool> ("FLAG"));
    td.addColumn (ScalarColumnDesc<Double> ("TIME"));
    
    Int nrows=tabSel_p.nrow();
    
    
    // Now create a new table from the description.
    SetupNewTable aNewTab("Kscratch", td, Table::New);
    tabB = Table (aNewTab, Table::Memory, nrows);
    
    
    ROArrayColumn<Complex>  origGain(tabSel_p,"GAIN");
    ROArrayColumn<Bool> solnOk(tabSel_p, "SOLUTION_OK") ;
    ROScalarColumn<Int> origBaseline(tabSel_p, "ANTENNA1");
    ROScalarColumn<Int> origCalDesc(tabSel_p, "CAL_DESC_ID");
    ROScalarColumn<Double> origTime(tabSel_p, "TIME");
    Cube<Complex> ydata=origGain.getColumn();
    Vector<Int> baselines=origBaseline.getColumn();
    //Let's determine nAnt now
    Int maxbaseline=max(baselines);
    Int baseId=0;
    nAnt=0;
    while(baseId <= maxbaseline){
      baseId=nAnt*(nAnt+1)/2;
      ++nAnt;
    }
    --nAnt;
    ant1hash.resize(maxbaseline+1);
    ant2hash.resize(maxbaseline+1);
    Int ibl=0;
    for (Int k=0; k < nAnt; ++k){
      for (Int j=k; j < nAnt; ++j){
	ibl=k*nAnt-k*(k+1)/2+j;
	ant1hash(ibl)=k;
	ant2hash(ibl)=j;
      } 
    }
    //===

    Cube<Bool> soln=solnOk.getColumn();
    Cube<Bool> flag(1, 1, nrows);
    Vector<Int> ant1(nrows);
    Vector<Int> ant2(nrows);
    Cube<Float> newPhase(1,1,nrows);
    newPhase.set(0);
    Cube<Float> newAmp(1,1,nrows);
    newAmp.set(0);
    Cube<Float> newDelay(1,1,nrows);
    newDelay.set(0);
    Cube<Float> newDelayRate(1,1,nrows);
    newDelayRate.set(0);

    for (Int k=0; k < nrows ; ++k){
      ant1[k]=ant1hash[baselines[k]];
      ant2[k]=ant2hash[baselines[k]];
      if(whichPol_p==0){
	newPhase(0,0,k)=arg(ydata(0,0,k))*(180.0/C::pi);
	newAmp(0,0,k)=abs(ydata(0,0,k));
	newDelay(0,0,k)=real(ydata(1,0,k));
      }
      else if(whichPol_p==1){
	newPhase(0,0,k)=arg(ydata(2,0,k))*(180.0/C::pi);
	newAmp(0,0,k)=abs(ydata(2,0,k));
	newDelay(0,0,k)=real(ydata(3,0,k));
      }
      else if(whichPol_p==2){
	newPhase(0,0,k)=(arg(ydata(0,0,k))-arg(ydata(2,0,k)))*180.0/C::pi;
	while(newPhase(0,0,k) > 180.0){
	  newPhase(0,0,k)-=360.0;
	}
	while(newPhase(0,0,k) < -180.0){
	  newPhase(0,0,k)+=360.0;
	}
	if(abs(ydata(2,0,k))> 0){
	  newAmp(0,0,k)=abs(ydata(0,0,k))/abs(ydata(2,0,k));

	}
	newDelay(0,0,k)=real(ydata(1,0,k)-ydata(3,0,k));
      }
      else if(whichPol_p==3){
	newPhase(0,0,k)=arg( (ydata(0,0,k) + ydata(2,0,k))/2.0)*180.0/C::pi;
	newAmp(0,0,k)=abs((ydata(0,0,k)+ydata(2,0,k))/2.0);
	newDelay(0,0,k)=real(ydata(3,0,k)+ydata(1,0,k))/2.0;	  
      }
      
      newDelayRate(0,0,k)=real(ydata(4,0,k));
      flag(0,0,k)=!soln(0,0,k);
      
    }
 

    ScalarColumn<Int> newAnt1(tabB, "ANTENNA1");
    newAnt1.putColumn(ant1);
    ScalarColumn<Int> newAnt2(tabB, "ANTENNA2");
    newAnt2.putColumn(ant2);
    ScalarColumn<Int> cal_desc(tabB, "CAL_DESC_ID");
    cal_desc.putColumn(origCalDesc);
    ArrayColumn<Bool> flagCol(tabB, "FLAG");
    flagCol.putColumn(flag);
    label_p[1]="Time";
    plotopts_p.define("timeplot", "x");
    ScalarColumn<Double> newtime(tabB, "TIME");
    newtime.putColumn(origTime);
    ArrayColumn<Float> phase(tabB, "PHASE");
    phase.putColumn(newPhase);
    ArrayColumn<Float> amp(tabB, "AMP");
    amp.putColumn(newAmp);
    ArrayColumn<Float> del(tabB, "DELAY");
    del.putColumn(newDelay);
    ArrayColumn<Float> delrat(tabB, "DELAYRATE");
    delrat.putColumn(newDelayRate);
    ScalarColumn<Int> newBas(tabB, "BASELINE");
    newBas.putColumn(baselines);


  }

  void PlotCal::subplotToPanel(const Int&subplot, Int& nrows, Int& ncols, Int& panel){
    
    LogIO os(LogOrigin("plotcal", "subplotToPanel", WHERE));
    nrows=0;
    ncols=0; 
    panel=0;
    //Stolen from msplot
    if(subplot >999){
      os << LogIO::SEVERE << "Invalid subplot value: "
	 << subplot << ". A value of 0, or a value\n between 111"
	 << "and 999 is expected."
	 << LogIO::POST;
      return;
    }
    String threeChars = String::toString( subplot ); 
    nrows = atoi((threeChars.at(0,1)).chars());
    ncols = atoi((threeChars.at(1,1)).chars());
    panel = atoi((threeChars.at(2,1)).chars());

  }



  void PlotCal::setPlotSymbols(const String& plotSymbol){
    LogIO os(LogOrigin("plotcal", "setplotsymplols", WHERE));
    //Stolen from msplot
    String plotsymbol;
    Int nblanks=plotSymbol.freq(" ");
    if(nblanks > 0){
      String *splitstrings = new String[nblanks+1];
      nblanks=split(plotSymbol, splitstrings, nblanks+1, " ");
      plotsymbol=splitstrings[0];
      for (Int k = 1 ; k < nblanks; ++k)
	plotsymbol=plotsymbol+splitstrings[k];
    }
    else{
      plotsymbol=plotSymbol;
    }

    plotsymbol.downcase();
    plotopts_p.define("plotsymbol", plotsymbol);    
    
  }

  void PlotCal::fillPlotOptions(){
    LogIO os(LogOrigin("PlotCal", "fillPlotOptions", WHERE));
    Vector<String> errors(2);

    errors[0] = itsPlotOptions.fillFromRecord(plotopts_p);

    if ( errors[0].length() > 0 )
      {
	// Errors have occured!
	os  << LogIO::SEVERE
	    << errors[0] << LogIO::EXCEPTION;
      }
  }


  void PlotCal::getMetaRecord() {

    metaRec_p = Record();

    if (tabSpws_p.nelements()>0) {
      metaRec_p.define("spwIds",tabSpws_p.row(0));
    }

    if (msName_p!="" && Table::isReadable(msName_p)) {
      Vector<String> fieldNames;
      Vector<String> antNames;
      MeasurementSet ms(msName_p);
      fieldNames = ROMSFieldColumns(ms.field()).name().getColumn();
      antNames = ROMSAntennaColumns(ms.antenna()).name().getColumn();
     
      if (calType_p.contains("M")) {
	Int nant=antNames.nelements();
	Vector<String> blNames(nant*(nant+1)/2);
	Int k=0;
	for (Int i=0;i<nant;++i)
	  for (Int j=i;j<nant;++j,++k) 
	    blNames(k)=antNames(i)+"-"+antNames(j);
	antNames.resize();
	antNames = blNames;
      }
      if (calType_p.contains("GSPLINE")) 
	fieldNames = "Any";

      metaRec_p.define("fieldNames",fieldNames);
      metaRec_p.define("antNames",antNames);
    }
    else 
      cout << "Associated MS not available.  Locate will report indices exclusively." << endl;

  }



  // Interpret antenna indices (via MSSelection)
  Vector<Int> PlotCal::getAntIdx(const String& antenna) {
    
    if (msName_p!="" && Table::isReadable(msName_p)) {
      MeasurementSet ms(msName_p);
      MSSelection mssel;

      // Handle baseline-based types
      if (calType_p=="M" || calType_p=="MF") {

	// Form the baseline list (which selects from ANTENNA1, currently

	// Punt if given too complex specification
	if (antenna.contains(";")) 
	  throw(AipsError("Antenna/baseline selection only supports single expressions (don't use ;)"));

	mssel.setAntennaExpr(antenna);

	Int nAnt=ms.antenna().nrow();

	Vector<Int> a1=mssel.getAntenna1List(&ms);
	Vector<Int> a2=mssel.getAntenna2List(&ms);
	Vector<Int> bllist;
	Int nbl=0;
	Int i,j;
	for (uInt ii=0;ii<a1.nelements();++ii) {
	  for (uInt jj=0;jj<a2.nelements();++jj) {
	    if (a1(ii)>a2(jj)) {
	      j=a1(ii);
	      i=a2(jj);
	    }
	    else {
	      i=a1(ii);
	      j=a2(jj);
	    }
	    bllist.resize(nbl+1,True);
	    bllist(nbl)=(i+1)*(i+2)/2 + (nAnt-i-1)*i + j-i-1;
	    ++nbl;
	  }
	}
	return bllist;
      }
      else {
	// Antenna-based case is easy
	mssel.setAntennaExpr(antenna);
	return mssel.getAntenna1List(&ms);
      }
    }
    else {
      return Vector<Int>();
    }

  }

  // Interpret field indices (MSSelection)
  Vector<Int> PlotCal::getFieldIdx(const String& field) {
    
    if (msName_p!="" && Table::isReadable(msName_p)) {
      MeasurementSet ms(msName_p);
      MSSelection mssel;
      mssel.setFieldExpr(field);
      return mssel.getFieldList(&ms);
    }
    else {
      return Vector<Int>();
    }
    
  }

  // Get cal_desc indices (via MSSelection on spws)
  Vector<Int> PlotCal::getSpwIdx(const String& spw) {

    if (msName_p!="" && Table::isReadable(msName_p)) {
      MeasurementSet ms(msName_p);
      MSSelection mssel;
      mssel.setSpwExpr(spw);
      return mssel.getSpwList(&ms);
    }
    else {
      return Vector<Int>();
    }

  }

  // Get cal_desc indices (from spw indices)
  Vector<Int> PlotCal::getCalDescIds(const Vector<Int> selspws) {
    
    Vector<Int> allcds(nCalDesc_p);
    Vector<Bool> cdmask(nCalDesc_p,False);
    indgen(allcds);

    for (Int icd=0;icd<nCalDesc_p;++icd) {
      cdmask(icd)=False;
      for (Int ispw=0;ispw<Int(selspws.nelements());++ispw)
	cdmask(icd)|=anyEQ(tabSpws_p.column(icd),selspws(ispw));
    }

    return allcds(cdmask).getCompressedArray();

  }

  // Interpret field indices (MSSelection)
  Matrix<Double> PlotCal::getTimeRange(const String& time) {

    if (msName_p!="" && Table::isReadable(msName_p)) {
      MeasurementSet ms(msName_p);
      MSSelection mssel;
      mssel.setTimeExpr(time);
      return mssel.getTimeList(&ms);
    }
    else {
      return Matrix<Double>();
    }

  }

  void PlotCal::getFreqInfo() {

    if (noMS_p)
      throw(AipsError("MS is not available to get frequencies for freq axis. Please plot channels instead."));

    MeasurementSet ms(msName_p);
    ROArrayColumn<Double> chanfreqcol(ROMSSpWindowColumns(ms.spectralWindow()).chanFreq());

    for (Int icd=0;icd<nCalDesc_p;++icd) {

      Int ispw=tabSpws_p(0,icd);
      if (ispw<0) ispw=0;
      //      cout << icd << " " << tabSpws_p(0,icd) << "  chanfreqcol.shape() = " << chanfreqcol.shape(ispw) << endl;

      Vector<Double> chanfreq = chanfreqcol(ispw);
      Int nchan=chanfreq.nelements();

      startFreq_p(icd) = chanfreq(MSstartChan_p(icd));
      stepFreq_p(icd)= nchan>1 ? (chanfreq(nchan-1)-chanfreq(0))/Double(nchan-1): 0.0;

      //      cout << "    " << startFreq_p(icd) << " " << stepFreq_p(icd) << endl;

    }

    // Render frequencies in MHz:
    startFreq_p/=1.0e6;
    stepFreq_p/=1.0e6;

  }



  void PlotCal::virtualBPoly( Table& tabB, Int& nchan ){

    //    cout << "Evalutating BPOLY solutions...." << endl;

    //    ROScalarColumn<Int> origAnt1(tabSel_p, "ANTENNA1");
    ROScalarColumn<Int> origFld(tabSel_p, "FIELD_ID");
    ROScalarColumn<Int> origCalDesc(tabSel_p, "CAL_DESC_ID");
    ROScalarColumn<Double> origTime(tabSel_p, "TIME");

    // The following is cribbed from MeasurementComponents/BPoly.cc
    //-------------------------------------------------------------
    // Open the selected table as a BJonesPolyTable
    BJonesPolyTable calTable(tabSel_p.tableName(), Table::Update);

    // Attach a calibration table columns accessor
    BJonesPolyMCol col(calTable);
    
    Int ndesc=calTable.nRowDesc();

    startFreq_p.resize(ndesc);
    stepFreq_p.resize(ndesc);

    // Fill the bandpass correction cache
    Int nrow = calTable.nRowMain();

    Int npolin=2;
    //    nchan=100;
    nchan=500;

    IPosition ipin(3,npolin,nchan,nrow);
    
    Cube<Bool> flag(ipin);
    flag=True;
    Cube<Complex> gain(ipin);
    gain.set(Complex(1.0));
    Float Amp(0.0),Pha(0.0);

    Vector<Int> origAnt1(nrow,0);

    IPosition ipos(3,0,0,0);
    IPosition iposOK(2,0,0);

    whichPol_p=0;
    
    for (Int row=0; row < nrow; row++) {
      
      // CalDescId
      Int idesc=col.calDescId().asInt(row);

      // Antenna id.
      Int antennaId = col.antenna1().asInt(row);
      origAnt1(row)=antennaId;
      ipos(2)=antennaId;
      iposOK(1)=antennaId;
      
      // Frequency group name
      String freqGrpName = col.freqGrpName().asString(row);
      
      // Extract the polynomial scale factor
      Complex factor = col.scaleFactor().asComplex(row);
      
      // Extract the polynomial coefficients in amplitude and phase
      Int nAmp = col.nPolyAmp().asInt(row);
      Int nPhase = col.nPolyPhase().asInt(row);
      Matrix<Double> ampCoeff(nAmp,2);
      Matrix<Double> phaseCoeff(nPhase,2);
      Array<Double> ampCoeffArray, phaseCoeffArray;
      col.polyCoeffAmp().get(row, ampCoeffArray);
      col.polyCoeffPhase().get(row, phaseCoeffArray);
      
      IPosition ampPos = ampCoeffArray.shape();
      ampPos = 0;
      for (Int k=0; k < 2*nAmp; k++) {
	ampPos.setLast(IPosition(1,k));
	ampCoeff(k%nAmp,k/nAmp) = ampCoeffArray(ampPos);
      };
      
      IPosition phasePos = phaseCoeffArray.shape();
      phasePos = 0;
      for (Int k=0; k < 2*nPhase; k++) {
	phasePos.setLast(IPosition(1,k));
	phaseCoeff(k%nPhase,k/nPhase) = phaseCoeffArray(phasePos);
      };
      
      // Extract the valid domain for the polynomial
      Vector<Double> freqDomain(2);
      col.validDomain().get(row, freqDomain);
      Double x1 = freqDomain(0);
      Double x2 = freqDomain(1);

      Vector<Double> freq(nchan);
      Double dfreq( (x2-x1)/Double(nchan-1) );
      for (Int ichan=0;ichan<nchan;++ichan)
	freq(ichan)=freqDomain(0)+Double(ichan)*dfreq;

      startFreq_p(idesc)=freqDomain(0)/1.0e6;
      stepFreq_p(idesc)=dfreq/1.0e6;

      for (Int ipol=0;ipol<2;ipol++) {
	
	Vector<Double> ac(ampCoeff.column(ipol));
	Vector<Double> pc(phaseCoeff.column(ipol));
	
	// Only do non-triv calc if coeffs are non-triv
	if (anyNE(ac,Double(0.0)) ||
	    anyNE(pc,Double(0.0)) ) {
	  
	  ipos(0)=ipol;
	  
	  // Loop over frequency channel
	  for (Int chan=0; chan < nchan; chan++) {
	    ipos(1)=chan;
	    iposOK(0)=chan;
	    
	    // only if in domain, calculate Cheby
	    if (freq(chan) >=x1 && freq(chan)<= x2) {
	      Amp = exp(getChebVal(ac, x1, x2, freq(chan)));
	      Pha = getChebVal(pc, x1, x2, freq(chan));
	      gain(ipol,chan,row)=Complex(Amp)*Complex(cos(Pha),sin(Pha));
	      // Set flag for valid cache value
	      flag(ipol,chan,row) = False;
	    }
	  }
	}
      }
      
    };

    // Ensure something to plot
    if (nfalse(flag)==0)
      throw(AipsError("No unflagged solutions to plot."));

    // Handle x-axis issues
    Vector<Double> newtime=origTime.getColumn();

    // A Table Description for the virtual table for TablePlot
    TableDesc td("", "1", TableDesc::Scratch);
    td.comment() = "A memory B table (derived from BPOLY)";
    td.addColumn (ScalarColumnDesc<Double> ("TIME"));
    td.addColumn (ScalarColumnDesc<Int>("ANTENNA1"));
    td.addColumn (ScalarColumnDesc<Int>("FIELD_ID"));
    td.addColumn (ScalarColumnDesc<Int>("CAL_DESC_ID"));
    td.addColumn (ArrayColumnDesc<Complex>("GAIN"));
    td.addColumn (ArrayColumnDesc<Bool> ("FLAG"));
    
    // Now create a new table from the description.
    SetupNewTable aNewTab("BGscratch", td, Table::New);
    tabB = Table (aNewTab, Table::Memory, nrow);
    
    ScalarColumn<Double> newtimecol(tabB, "TIME");
    newtimecol.putColumn(newtime);
    ScalarColumn<Int> newAnt1(tabB, "ANTENNA1");
    newAnt1.putColumn(origAnt1);
    ScalarColumn<Int> fieldid(tabB, "FIELD_ID");
    fieldid.putColumn(origFld);
    ScalarColumn<Int> cal_desc(tabB, "CAL_DESC_ID");
    cal_desc.putColumn(origCalDesc);
    ArrayColumn<Complex> newGain(tabB, "GAIN");
    newGain.putColumn(gain);
    ArrayColumn<Bool> flagCol(tabB, "FLAG");
    flagCol.putColumn(flag);

    //    cout << "...done." << endl;

  }

  Double PlotCal::getChebVal (const Vector<Double>& coeff,
			      const Double& xinit, const Double& xfinal,
			      const Double& x)
  {
    // Compute a Chebyshev polynomial value using the CLIC library
    // Input:
    //    coeff       const Vector<Double>&       Chebyshev coefficients
    //    xinit       const Double&               Domain start
    //    xfinal      const Double&               Domain end
    //    x           const Double&               x-ordinate
    // Output:
    //    getChebVal  Double                      Chebyshev polynomial value
    //
    // Re-scale x-ordinate
    Double xcap=0;
    xcap=((x-xinit)-(xfinal-x))/(xfinal-xinit);
    
    // Compute polynomial
    Int deg=coeff.shape().asVector()(0);
    Vector<Double> val(deg);
    Bool check;
    Int checkval;
    cheb(&deg,
	 &xcap,
	 val.getStorage(check),
	 &checkval);
    
    Double soly=0.0;
    for (Int mm=0; mm< deg; mm++){
      soly+= coeff[mm]*val[mm];
    }
    
    return soly;
  }





//----------------------------------------------------------------------------

void PlotCal::virtualGSpline( Table& tabG ) {
  
  //  cout << "Evalutating GSPLINE solutions...." << endl;

  // Attach a calibration buffer and iterator to the calibration 
  // table containing corrections to be applied
  GJonesSplineTable calTable(tabSel_p.tableName(), Table::Update);
  CalIter calIter(calTable);
  GJonesSplineMBuf calBuffer(calIter);
  // Create the buffer and synchronize with the iterator
  //  calBuffer = GJonesSplineMBuf(calIter);
  calBuffer.synchronize();
  calBuffer.fillCache();

  Double freqRatio=1.0;
  Double vbFreqHz = 1.0; // 1.0e9*mean(currFreq());

  //  cout << "vbFreqHz = " << vbFreqHz << endl;

  Vector<Int> antidlist=calBuffer.antenna1();
  Int nAnt=GenSort<Int>::sort(antidlist,Sort::Ascending,
			      Sort::NoDuplicates);
  antidlist.resize(nAnt,True);

  Int nTime(1000);
  Int nRow(nAnt*nTime);
  Vector<Double> times(nRow,0.0);
  Vector<Int> ant1(nRow,-1);
  Cube<Complex> gainspl(2,1,nRow);
  gainspl.set(Complex(1.0));
  Cube<Bool> flag(2,1,nRow);
  flag.set(True);
    
  // Row counter
  Int irow(0);
  Double dt=-1.0;
  Double mintime=-1.0;
  // Compute splines per antenna
  for (Int iant=0; iant < nAnt; ++iant) {
    
    // Match this antenna id. and the visibility buffer field id. 
    // in the calibration buffer

    Int antid=antidlist(iant);


    Vector<Int> matchingRows = 
      calBuffer.matchAntenna1AndFieldId(antid,0);
    
    // First sample is iant; will increment by nAnt below
    irow = iant;

    if (matchingRows.nelements() > 0) {
	  
      // Matching calibration solution found
      Int row = matchingRows(0);
      
      Complex gain(calBuffer.scaleFactor()(row));
      String mode = calBuffer.polyMode()(row);
      
      //      cout << "gain = " << gain << endl;
      
      // Compute the ratio between the calibration solution 
      // reference frequency and the mean observed frequency
      // of the visibility data to be corrected
      IPosition refFreqPos = calBuffer.refFreqMeas().shape();
      refFreqPos = 0;
      refFreqPos.setLast(IPosition(1,row));
      MFrequency refFreq = calBuffer.refFreqMeas()(refFreqPos);
      Double refFreqHz = refFreq.get("Hz").getValue();
      vbFreqHz = refFreqHz;
      freqRatio = abs(refFreqHz) > 0 ? vbFreqHz / refFreqHz : 1.0;
      //      cout << "freqRatio = " << freqRatio << endl;

      Vector<Double> ampKnots;
      Vector<Double> phaseKnots;
      Vector<Double> ampCoeff;
      Vector<Double> phaseCoeff;
      Int nPolyA(0);
      Int nPolyP(0);

      // Compute amplitude polynomial
      if (mode.contains("AMP") || mode.contains("A&P")) {
	// Extract amplitude spline polynomial knots
	IPosition ampKnotsPos = calBuffer.splineKnotsAmp().shape();
	ampKnotsPos = 0;
	Int nAmpKnotsPos = ampKnotsPos.nelements();
	ampKnotsPos[nAmpKnotsPos-1] = row;
	Int nKnots = calBuffer.nKnotsAmp()(row);
	ampKnots.resize(nKnots);
	for (Int k=0; k < nKnots; k++) {
	  ampKnotsPos[nAmpKnotsPos-2] = k;
	  ampKnots(k) = calBuffer.splineKnotsAmp()(ampKnotsPos);
	};

	if (dt<0.0) {
	  dt=(max(ampKnots)-min(ampKnots))/Double(nTime);
	  mintime=ampKnots(0)+dt/2.0;
	}

	// Extract amplitude spline polynomial coefficients
	IPosition ampCoeffPos = calBuffer.polyCoeffAmp().shape();
	ampCoeffPos = 0;
	Int nAmpCoeffPos = ampCoeffPos.nelements();
	ampCoeffPos[nAmpCoeffPos-1] = row;
	nPolyA = calBuffer.nPolyAmp()(row);
	ampCoeff.resize(2*nPolyA);
	for (Int k=0; k < 2*nPolyA; k++) {
	  ampCoeffPos[nAmpCoeffPos-2] = k;
	  ampCoeff(k) = calBuffer.polyCoeffAmp()(ampCoeffPos);
	};
	
	//	cout << "ampCoeff = " << ampCoeff << endl;
	
      };
      
      // Compute phase polynomial
      if (mode.contains("PHAS") || mode.contains("A&P")) {
	
	// Extract phase spline polynomial knots
	IPosition phaseKnotsPos = calBuffer.splineKnotsPhase().shape();
	phaseKnotsPos = 0;
	Int nPhaseKnotsPos = phaseKnotsPos.nelements();
	phaseKnotsPos[nPhaseKnotsPos-1] = row;
	Int nKnots = calBuffer.nKnotsPhase()(row);
	phaseKnots.resize(nKnots);
	for (Int k=0; k < nKnots; k++) {
	  phaseKnotsPos[nPhaseKnotsPos-2] = k;
	  phaseKnots(k) = calBuffer.splineKnotsPhase()(phaseKnotsPos);
	};

	if (dt<0.0) {
	  dt=(max(phaseKnots)-min(phaseKnots))/Double(nTime);
	  mintime=phaseKnots(0)+dt/2.0;
	}

	// Extract phase spline polynomial coefficients
	IPosition phaseCoeffPos = calBuffer.polyCoeffPhase().shape();
	phaseCoeffPos = 0;
	Int nPhaseCoeffPos = phaseCoeffPos.nelements();
	phaseCoeffPos[nPhaseCoeffPos-1] = row;
	nPolyP = calBuffer.nPolyPhase()(row);
	phaseCoeff.resize(2*nPolyP);
	for (Int k=0; k < 2*nPolyP; k++) {
	  phaseCoeffPos[nPhaseCoeffPos-2] = k;
	  phaseCoeff(k) = calBuffer.polyCoeffPhase()(phaseCoeffPos);
	};
	
      };
      
      
      for (Int itime=0;itime<nTime;++itime) {
	
	times(irow)=mintime + Double(itime)*dt;
	ant1(irow)=antid;

	//	cout << itime << " " << MVTime(times(irow)/C::day).string(MVTime::YMD,7) << endl;

	Vector<Double> ac;
	Vector<Double> pc;
	Vector<Double> ampVal(2,1.0);
	Vector<Double> phaseVal(2,0.0);
	
	// Fill the gainspl array 
	for (Int ipol=0;ipol<2;++ipol) {

	  if (mode.contains("AMP") || mode.contains("A&P")) {
	    ac.reference(ampCoeff(IPosition(1,ipol*nPolyA),
				  IPosition(1,(ipol+1)*nPolyA-1)));
	    ampVal(ipol) *= exp(getSplineVal(times(irow), ampKnots, ac));
	  }


	  if (mode.contains("PHAS") || mode.contains("A&P")) {
	    pc.reference(phaseCoeff(IPosition(1,ipol*nPolyP),
				    IPosition(1,(ipol+1)*nPolyP-1)));
	    phaseVal(ipol) = getSplineVal(times(irow), phaseKnots, pc);
	    
	    // Handle gildas sign convention on spline phases
	    phaseVal(ipol) = -phaseVal(ipol);
	    
	    // Scale by the ratio of the observing frequency of the 
	    // data to be corrected and the reference frequency of the
	    // calibration solution
	    phaseVal(ipol) *= freqRatio;
	  }

	  gainspl(ipol,0,irow) = gain * ampVal(ipol) * Complex(cos(phaseVal(ipol)), 
							       sin(phaseVal(ipol)) );
	  flag(ipol,0,irow) = False;

      /*
	  cout << itime << iant << ipol << " "
	       << gain << " "
	       << ampVal(ipol) << " "
	       << phaseVal(ipol) << " "
	       << gainspl(ipol,0,irow) << " "
	       << endl;
      */
	}
	irow+=nAnt;
      }
    }
  };

  // Ensure something to plot
  if (nfalse(flag)==0)
    throw(AipsError("No unflagged solutions to plot."));

  //  cout << "gainspl = " << gainspl << endl;
  //  cout << "abs(gainspl) = " << amplitude(gainspl) << endl;
  //  cout << "pha(gainspl) = " << phase(gainspl) << endl;
  //  cout << "times = " << times << endl;
  //  cout << "ant1 = " << ant1 << endl;
  
  // A Table Description for the virtual table for TablePlot
  TableDesc td("", "1", TableDesc::Scratch);
  td.comment() = "A memory G table (derived from GSPLINE)";
  td.addColumn (ScalarColumnDesc<Double> ("TIME"));
  td.addColumn (ScalarColumnDesc<Int>("ANTENNA1"));
  td.addColumn (ScalarColumnDesc<Int>("FIELD_ID"));
  td.addColumn (ScalarColumnDesc<Int>("CAL_DESC_ID"));
  td.addColumn (ArrayColumnDesc<Complex>("GAIN"));
  td.addColumn (ArrayColumnDesc<Bool> ("FLAG"));
    
  // Now create a new table from the description.
  SetupNewTable aNewTab("BGscratch", td, Table::New);
  tabG = Table (aNewTab, Table::Memory, nRow);
  
  ScalarColumn<Double> newtimecol(tabG, "TIME");
  newtimecol.putColumn(times);
  ScalarColumn<Int> newAnt1(tabG, "ANTENNA1");
  newAnt1.putColumn(ant1);
  ScalarColumn<Int> fieldid(tabG, "FIELD_ID");
  fieldid.fillColumn(0);
  ScalarColumn<Int> cal_desc(tabG, "CAL_DESC_ID");
  cal_desc.fillColumn(0);
  ArrayColumn<Complex> newGain(tabG, "GAIN");
  newGain.putColumn(gainspl);
  ArrayColumn<Bool> flagCol(tabG, "FLAG");
  flagCol.putColumn(flag);

  //  cout << "...done." << endl;

  return;

};

//----------------------------------------------------------------------------


Double PlotCal::getSplineVal (Double x, 
			      Vector<Double>& knots,
			      Vector<Double>& coeff)
{
// Compute a polynomial spline value using the GILDAS routine getbspl
// Input:
//    x             Double                   Value at which to compute spline
//    knots         Vector<Double>&          Knot locations
//    coeff         Vector<Double>&          Spline coefficients
// Output:
//    getSplineVal  Double                   Computed spline value
//
  LogIO os (LogOrigin("GJonesSpline", "getSplineVal()", WHERE));

  // Use GILDAS library routine, getbspl
  Int numOfknots=knots.nelements();
  Int failflag;
  Bool dum;
  Double yval;
  getbspl(&numOfknots, 
	  knots.getStorage(dum),
	  coeff.getStorage(dum),
	  &x,
	  &yval,
	  &failflag);

  return yval;
}











       
//----------------------------------------------------------------------------

} //# NAMESPACE CASA - END
