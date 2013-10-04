//# FlagAgentDisplay.h: This file contains the interface definition of the FlagAgentDisplay class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef FlagAgentDisplay_H_
#define FlagAgentDisplay_H_

#include <flagging/Flagging/FlagAgentBase.h>

#include <casadbus/viewer/ViewerProxy.h>
#include <casadbus/plotserver/PlotServerProxy.h>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>

//#include <ms/MeasurementSets/MSColumns.h>

#include <flagging/Flagging/LFPlotServerProxy.h>


namespace casa { //# NAMESPACE CASA - BEGIN
  
  class FlagAgentDisplay : public FlagAgentBase {
    
  public:
    
    FlagAgentDisplay(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube = false);
    ~FlagAgentDisplay();

    // Make plots and either display or write to a file
    Bool displayReports(FlagReport &combinedReport);
    
    // Get a report/summary
    FlagReport getReport();
    
  protected:
    
    // Compute flags for a given (time,freq) map
    bool computeAntennaPairFlags(const vi::VisBuffer2 &visBuffer, VisMapper &visibilities,FlagMapper &flag,Int antenna1,Int antenna2,vector<uInt> &rows);

    void preProcessBuffer(const vi::VisBuffer2 &visBuffer);
    
    // Choose how to step through the baselines in the current chunk
    void iterateAntennaPairsInteractive(antennaPairMap *antennaPairMap_ptr);
    
    // Parse configuration parameters
    void setAgentParameters(Record config);

  private:
    
    Bool setDataLayout();
    Bool setReportLayout();
    Bool buildDataPlotWindow();
    Bool buildReportPlotWindow();


    void getChunkInfo(const vi::VisBuffer2 &visBuffer);
    Bool skipBaseline(std::pair<Int,Int> antennaPair);
 
    void getUserInput();
    void getReportUserInput();
    
    void DisplayRaster(Int xdim, Int ydim, Vector<Float> &data, uInt frame);
    void DisplayLine(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold,  uInt frame);
    void DisplayScatter(Int xdim, Vector<Double> &xdata, Vector<Float> &ydata, String label, String color, Bool hold,  uInt frame);
    void DisplayLineScatterError(FlagPlotServerProxy *&plotter, String &plottype, Vector<Float> &xdata, Vector<Float> &ydata, String &errortype, Vector<Float> &error, String label, String color, uInt frame);

    // Plotter members
    FlagPlotServerProxy *dataplotter_p;  
    FlagPlotServerProxy *reportplotter_p;
    Vector<dbus::variant> panels_p, report_panels_p;
    char *dock_xml_p, *report_dock_xml_p;
    Bool showBandpass_p;

    // Control parameters
    Bool stopAndExit_p;
    Bool pause_p;
    Bool dataDisplay_p, reportDisplay_p; // show per chunk plots and/or end-of-MS plots
    Bool reportReturn_p;
    String reportFormat_p;
    
    // visBuffer state variables
    Int fieldId_p;
    String fieldName_p;
    Int scanStart_p;
    Int scanEnd_p;
    Int spwId_p;
    String antenna1_p;
    String antenna2_p;
    
    uInt nPolarizations_p;
    Vector<Double> freqList_p;
    
    
    // GUI parameters
    String userChoice_p;
    String userFixA1_p, userFixA2_p;
    
    Int skipScan_p, skipSpw_p, skipField_p;

    Vector<String> plotColours_p;
    
    
  };
  
  
} //# NAMESPACE CASA - END

#endif /* FLAGAGENTDISPLAY_H_ */

