//# PlotMS.h: Main controller for plotms.
//# Copyright (C) 2008
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
//# $Id: $
#ifndef PLOTMS_H_
#define PLOTMS_H_

#include <plotms/PlotMS/PlotMSParameters.h>
#include <plotms/Plots/PlotMSPlotManager.h>

namespace casa {

// Version definitions.
// <group>
#define PLOTMS_VERSION 0x002250;
#define PLOTMS_VERSION_STR "2.25";
// </group>


//# Forward declarations.
class PlotMSDBusApp;
class PlotMSPlotter;


// Controller class for plotms.  Handles interactions between the UI and plots.
class PlotMS : public PlotMSParametersWatcher {
public:    
    // Default constructor that uses default options.  If connectToDBus is
    // true, then the application registers itself with CASA's DBus server
    // using the PlotMSDBusApp::dbusName() with the current process ID.
    PlotMS(bool connectToDBus = false);
    
    // Constructor which takes the given parameters.  If connectToDBus is true,
    // then the application registers itself with CASA's DBus server using the
    // PlotMSDBusApp::dbusName() with the current process ID.
    PlotMS(const PlotMSParameters& params, bool connectToDBus = false);
    
    // Destructor
    ~PlotMS();
  
    
    // Plotter Methods //
    
    // Returns the PlotMSPlotter associated with this PlotMS. 
    PlotMSPlotter* getPlotter();
    
    // See PlotMSPlotter::showGUI().
    void showGUI(bool show = true);
    
    // See PlotMSPlotter::guiShown().
    bool guiShown() const;
    
    // See PlotMSPlotter::execLoop().
    int execLoop();
    
    // See PlotMSPlotter::showAndExec().
    int showAndExec(bool show= true);
    
    // See PlotMSPlotter::close().
    void close();
    
    // See PlotMSPlotter::showError().
    // <group>
    void showError(const String& message, const String& title = "PlotMS Error",
            bool isWarning = false);
    void showWarning(const String& message,
            const String& title = "PlotMS Warning");
    // </group>
    
    // See PlotMSPlotter::showMessage().
    void showMessage(const String& message,
            const String& title = "PlotMS Message");
    
    
    // Parameter Methods //
    
    // Gets/Sets the parameters for this PlotMS.
    // <group>
    PlotMSParameters& getParameters();
    void setParameters(const PlotMSParameters& params);
    // </group>
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
                int updateFlag);
    
    
    // Logger Methods //
    
    // Gets the logger associated with this PlotMS.
    PlotLoggerPtr getLogger();
    
    
    // Plot Management Methods //
    
    // Returns the PlotMSPlotManager associated with this PlotMS.
    PlotMSPlotManager& getPlotManager();
    
    // See PlotMSPlotManager::addSinglePlot().
    PlotMSSinglePlot* addSinglePlot(const PlotMSPlotParameters* p = NULL);
    
    // See PlotMSPlotManager::addMultiPlot();
    PlotMSMultiPlot* addMultiPlot(const PlotMSPlotParameters* p = NULL);
    
    bool isDrawing() const;
    bool isClosed() const;

    // save plot  to file using specified format. If interactive, pop up confirm window, if not, no confirm windowl
    bool save(const PlotExportFormat& format, const bool interactive);

public:
    // To allow normal error/warning/info popups, which block execution,
    // or, if not, prevent blocking by writing to the Logger and posting 
    // text to a status bar (or other visible gui element TBD)
    // (public for now, while it's experimental)
	bool its_want_avoid_popups;

private:
    // Plotter GUI.
    PlotMSPlotter* itsPlotter_;
    
    // Current parameters.
    PlotMSParameters itsParameters_;
    
    // Logger.
    PlotLoggerPtr itsLogger_;
    
    // Plot manager.
    PlotMSPlotManager itsPlotManager_;
    
    // DBus application, or NULL if one is not needed.
    PlotMSDBusApp* itsDBus_;

    
    // Initializes a new PlotMS object, to be called from constructor.
    void initialize(bool connectToDBus);    
    
    // Disable copy constructor and operator for now.
    // <group>
    PlotMS(const PlotMS& copy);
    PlotMS& operator=(const PlotMS& copy);
    // </group>
    
};

/*
// Temporary class for polarization selection, until I understand it better.
// (Not currently used.)
class PlotMSPolSelection {
public:
    PlotMSPolSelection(const String& polselstr = DEFAULT_POLSELSTR);
    
    ~PlotMSPolSelection();
    
    void setPolSelection(const String& polselstr);
    
    const String& polselstr() const;    
    unsigned int npols() const;    
    int pol_selection(unsigned int index) const;
    
    int operator()(unsigned int index) const { return pol_selection(index); }
    
private:
    String itsPolSelStr_;
    vector<int> itsPolSel_;
    
    static const String DEFAULT_POLSELSTR;
};
 */

// Rob's AxisUnit stuff, not currently used.
/*
enum AxisUnitEnum {Angle,         // Not Dimensionless because it can
                              // include degrees, mas, radians, etc.
           Dimensionless,     // Things that you'd have to stretch to
                              // tack a unit onto.
           FluxDensity,       // (m, u)Jy
           Freq,          // VisSet.h typedefs Frequency to Double.
           Intensity,         // Jy/beam, Jy/sq. arcsec, etc.
           Time,
           Velocity,
                   Wavelengths};      // (u, v, w) distances.

class AxisUnit
{
public:
  AxisUnitEnum operator[](const String& quant);

  // You want the AxisUnit, you go through this.  Call like AxisUnit::lookup().['time'].
  static AxisUnit& lookup();
  
private:
  // Default constructor.  Because it is private, instantiation of AxisUnits is suppressed.
  AxisUnit();

  // There can only be one AxisUnit, so there is no copy constructor or
  // assignment operator.

  // Destructor
  ~AxisUnit() {}

  typedef std::map<String, AxisUnitEnum> S2UMap;
  S2UMap quant2u;
};
  
class AxisUnitException: public AipsError {
public:
  //
  // Creates an AxisUnitException and initializes the error message from
  // the parameter
  // <group>
  AxisUnitException(const Char *str) : AipsError(str, AipsError::INVALID_ARGUMENT) {}
  AxisUnitException(const String &str) : AipsError(str,
                           AipsError::INVALID_ARGUMENT) {}
  
  // </group>

  // Destructor which does nothing.
  ~AxisUnitException() throw() {}
};
 */

}

#endif /* PLOTMS_H_ */
