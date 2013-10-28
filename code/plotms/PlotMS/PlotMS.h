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
#include <plotms/PlotMS/PlotMSFlagging.h>
#include <plotms/PlotMS/PlotEngine.h>
#include <plotms/Actions/PlotMSAction.h>

namespace casa {

// Version definitions.
// <group>
#define PLOTMS_VERSION 0x002250;
#define PLOTMS_VERSION_STR "2.25";
// </group>


//# Forward declarations.
class PlotMSDBusApp;
class Client;


// Controller class for plotms.  Handles interactions between the UI and plots.
class PlotMSApp : public PlotMSParametersWatcher, public PlotEngine {
public:    
    // Default constructor that uses default options.  If connectToDBus is
    // true, then the application registers itself with CASA's DBus server
    // using the PlotMSDBusApp::dbusName() with the current process ID.
    PlotMSApp(bool connectToDBus = false, bool userGui = true );
    
    // Constructor which takes the given parameters.  If connectToDBus is true,
    // then the application registers itself with CASA's DBus server using the
    // PlotMSDBusApp::dbusName() with the current process ID.
    PlotMSApp(const PlotMSParameters& params, bool connectToDBus = false, bool userGui = true);
    
    // Destructor
    ~PlotMSApp();
  
    
    // Plotter Methods //
    
    // Returns the PlotMSPlotter associated with this PlotMS. 
    //PlotMSPlotter* getPlotter();
    

    // See PlotMSPlotter::showGUI().
    virtual void showGUI(bool show = true);
    
    // See PlotMSPlotter::guiShown().
    virtual bool guiShown() const;
    
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
    
    // Enable/disable annotations in the client
    void setAnnotationModeActive( PlotMSAction::Type type, bool active );
    
    // Parameter Methods //
    
    // Gets/Sets the parameters for this PlotMS.
    // <group>
    virtual PlotMSParameters& getParameters();
    void setParameters(const PlotMSParameters& params);
    // </group>
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
                int updateFlag);
    virtual PlotSymbolPtr createSymbol (const String& descriptor,
    		Int size, const String& color,
        	const String& fillPattern, bool outline );
    PlotSymbolPtr createSymbol( const PlotSymbolPtr& copy );
    // Logger Methods //
    
    // Gets the logger associated with this PlotMS.
    virtual PlotLoggerPtr getLogger();
    
    
    // Plot Management Methods //
    
    // Returns the PlotMSPlotManager associated with this PlotMS.
    virtual PlotMSPlotManager& getPlotManager();
    
    // See PlotMSPlotManager::addSinglePlot().
   // virtual PlotMSPlot* addSinglePlot(const PlotMSPlotParameters* p = NULL);
    
    // See PlotMSPlotManager::addMultiPlot();
    //PlotMSPlot* addMultiPlot(const PlotMSPlotParameters* p = NULL);

    // See PlotMSPlotManager::addIterPlot();
    //PlotMSPlot* addIterPlot(const PlotMSPlotParameters* p = NULL);

    // See PlotMSPlotManager::addOverPlot();
    virtual PlotMSOverPlot* addOverPlot(const PlotMSPlotParameters* p = NULL);
    
    virtual bool isDrawing() const;
    bool isClosed() const;

    // save plot  to file using specified format. If interactive, pop up confirm window, if not, no confirm windowl
    bool save(const PlotExportFormat& format, const bool interactive);

    /**
     * PlotEngine methods
     */
    virtual PlotFactoryPtr getPlotFactory();
    virtual void quitApplication();
    virtual PlotMSFlagging getFlagging() const;
    virtual void setFlagging(PlotMSFlagging flag);
    void canvasAdded( PlotCanvasPtr canvas );
    bool isVisible(PlotCanvasPtr& canvas );
    bool exportToFormat(const PlotExportFormat& format);
    virtual Record locateInfo( Bool& success, String& errorMessage );
    PlotterPtr getPlotter();
public:
    // To allow normal error/warning/info popups, which block execution,
    // or, if not, prevent blocking by writing to the Logger and posting 
    // text to a status bar (or other visible gui element TBD)
    // (public for now, while it's experimental)
	bool its_want_avoid_popups;
	bool updateCachePlot( PlotMSPlot* plot, void (*f)(void*, bool), bool setupPlot);
	void setCommonAxes(bool commonX, bool commonY );
private:
    // Plotter GUI.
    //PlotMSPlotter* itsPlotter_;
    Client* itsPlotter_;

    // Current parameters.
    PlotMSParameters itsParameters_;
    
    // Logger.
    PlotLoggerPtr itsLogger_;
    
    // Plot manager.
    PlotMSPlotManager itsPlotManager_;
    
    // DBus application, or NULL if one is not needed.
    PlotMSDBusApp* itsDBus_;

    
    // Initializes a new PlotMS object, to be called from constructor.
    void initialize(bool connectToDBus, bool userGui );
    
    // Disable copy constructor and operator for now.
    // <group>
    PlotMSApp(const PlotMSApp& copy);
    PlotMSApp& operator=(const PlotMSApp& copy);
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
