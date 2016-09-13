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

#include <flagging/Flagging/FlagPlotServerProxy.h>


namespace casa { //# NAMESPACE CASA - BEGIN
  
  class FlagAgentDisplay : public FlagAgentBase {
    
  public:
    
    FlagAgentDisplay(FlagDataHandler *dh, casacore::Record config, casacore::Bool writePrivateFlagCube = false);
    ~FlagAgentDisplay();

    // Make plots and either display or write to a file
    casacore::Bool displayReports(FlagReport &combinedReport);
    
    // Get a report/summary
    FlagReport getReport();
    
  protected:
    
    // Compute flags for a given (time,freq) map
    bool computeAntennaPairFlags(const vi::VisBuffer2 &visBuffer, VisMapper &visibilities,FlagMapper &flag,casacore::Int antenna1,casacore::Int antenna2,vector<casacore::uInt> &rows);

    void preProcessBuffer(const vi::VisBuffer2 &visBuffer);
    
    // Choose how to step through the baselines in the current chunk
    void iterateAntennaPairsInteractive(antennaPairMap *antennaPairMap_ptr);
    
    // Parse configuration parameters
    void setAgentParameters(casacore::Record config);

  private:
    
    casacore::Bool setDataLayout();
    casacore::Bool setReportLayout();
    casacore::Bool buildDataPlotWindow();
    casacore::Bool buildReportPlotWindow();


    void getChunkInfo(const vi::VisBuffer2 &visBuffer);
    casacore::Bool skipBaseline(std::pair<casacore::Int,casacore::Int> antennaPair);
 
    void getUserInput();
    void getReportUserInput();
    
    void DisplayRaster(casacore::Int xdim, casacore::Int ydim, casacore::Vector<casacore::Float> &data, casacore::uInt frame);
    void DisplayLine(casacore::Int xdim, casacore::Vector<casacore::Double> &xdata, casacore::Vector<casacore::Float> &ydata, casacore::String label, casacore::String color, casacore::Bool hold,  casacore::uInt frame);
    void DisplayScatter(casacore::Int xdim, casacore::Vector<casacore::Double> &xdata, casacore::Vector<casacore::Float> &ydata, casacore::String label, casacore::String color, casacore::Bool hold,  casacore::uInt frame);
    void DisplayLineScatterError(FlagPlotServerProxy *&plotter, casacore::String &plottype, casacore::Vector<casacore::Float> &xdata, casacore::Vector<casacore::Float> &ydata, casacore::String &errortype, casacore::Vector<casacore::Float> &error, casacore::String label, casacore::String color, casacore::uInt frame);

    // Plotter members
    FlagPlotServerProxy *dataplotter_p;  
    FlagPlotServerProxy *reportplotter_p;

    // GUI parameters
    casacore::String userChoice_p;
    casacore::String userFixA1_p, userFixA2_p;

    casacore::Int skipScan_p, skipSpw_p, skipField_p;

    // Control parameters
    casacore::Bool pause_p;

    // visBuffer state variables
    casacore::Int fieldId_p;
    casacore::String fieldName_p;
    casacore::Int scanStart_p;
    casacore::Int scanEnd_p;
    casacore::Int spwId_p;
    casacore::uInt nPolarizations_p;
    casacore::Vector<casacore::Double> freqList_p;
    casacore::String antenna1_p;
    casacore::String antenna2_p;

    casacore::Bool dataDisplay_p, reportDisplay_p; // show per chunk plots and/or end-of-casacore::MS plots
    casacore::String reportFormat_p;
    casacore::Bool stopAndExit_p;
    casacore::Bool reportReturn_p;
    casacore::Bool showBandpass_p;

    casacore::Vector<dbus::variant> panels_p, report_panels_p;
    const char *dock_xml_p, *report_dock_xml_p;
//    casacore::Bool showBandpass_p;

    // Control parameters
//    casacore::Bool stopAndExit_p;
//    casacore::Bool pause_p;
//    casacore::Bool dataDisplay_p, reportDisplay_p; // show per chunk plots and/or end-of-casacore::MS plots
//    casacore::Bool reportReturn_p;
//    casacore::String reportFormat_p;
    
    // visBuffer state variables
//    casacore::Int fieldId_p;
//    casacore::String fieldName_p;
//    casacore::Int scanStart_p;
//    casacore::Int scanEnd_p;
//    casacore::Int spwId_p;
//    casacore::String antenna1_p;
//    casacore::String antenna2_p;
    
//    casacore::uInt nPolarizations_p;
//    casacore::Vector<casacore::Double> freqList_p;
    
    // GUI parameters
//    casacore::String userChoice_p;
//    casacore::String userFixA1_p, userFixA2_p;
    
//    casacore::Int skipScan_p, skipSpw_p, skipField_p;

    casacore::Vector<casacore::String> plotColours_p;
    
    
  };
  
  
} //# NAMESPACE CASA - END

#endif /* FLAGAGENTDISPLAY_H_ */

