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
#include <plotms/PlotMS/PlotMSExportParam.h>
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
    PlotMSApp(bool connectToDBus = false, bool userGui = true);
    
    // Constructor which takes the given parameters.  If connectToDBus is true,
    // then the application registers itself with CASA's DBus server using the
    // PlotMSDBusApp::dbusName() with the current process ID.
    PlotMSApp(const PlotMSParameters& params, bool connectToDBus = false, bool userGui = true);
    
    // Destructor
    ~PlotMSApp();
  
    
    // Plotter Methods //

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
    
    //Remove existing plots.
    virtual void clearPlots();

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

    //Enables/disables hover settings on the canvases based
    //on GUI settings.
    void resetHover();
    
    // Parameter Methods //
    
    // Gets/Sets the parameters for this PlotMS.
    // <group>
    virtual PlotMSParameters& getParameters();
    void setParameters(const PlotMSParameters& params);
    // </group>
    
    // Gets/Sets the export parameters for this PlotMS.
    // <group>
    virtual PlotMSExportParam& getExportParameters();
    void setExportParameters(const PlotMSExportParam& params);
    PlotExportFormat getExportFormat();
    void setExportFormat( const PlotExportFormat format );
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
    

    // See PlotMSPlotManager::addOverPlot();
    PlotMSPlot* addOverPlot(const PlotMSPlotParameters* p = NULL);
    
    virtual bool isDrawing() const;
    bool isClosed() const;
    // Set whether the latest plot update was successful completed.  This
    // may not be the case if an invalid selection was made.
    void setOperationCompleted( bool completed );


    // save plot  to file using specified format.
    bool save(const PlotExportFormat& format);

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
    //Returns whether or not the latest plot update completed successfully.
    //For example, if an invalid selection was made, the return value may be false.
    bool isOperationCompleted() const;
    PlotterPtr getPlotter();
public:
    // To allow normal error/warning/info popups, which block execution,
    // or, if not, prevent blocking by writing to the Logger and posting 
    // text to a status bar (or other visible gui element TBD)
    // (public for now, while it's experimental)
	bool its_want_avoid_popups;
	bool updateCachePlot( PlotMSPlot* plot, void (*f)(void*, bool), bool setupPlot);
	void setCommonAxes(bool commonX, bool commonY );
	bool isCommonAxisX() const;
	bool isCommonAxisY() const;
	void setAxisLocation( PlotAxis locationX, PlotAxis locationY );
	PlotAxis getAxisLocationX() const;
	PlotAxis getAxisLocationY() const;
	vector<String> getFiles() const;


private:
    // Plotter, Script or GUI
    Client* itsPlotter_;
    bool isGUI_;

    // Current parameters.
    PlotMSParameters itsParameters_;
    PlotMSExportParam itsExportParameters_;
    PlotExportFormat itsExportFormat;
    
    // Logger.
    PlotLoggerPtr itsLogger_;
    
    // Plot manager.
    PlotMSPlotManager itsPlotManager_;
    
    // DBus application, or NULL if one is not needed.
    PlotMSDBusApp* itsDBus_;

	//Whether the most recent plot updated was successfully
	//completed.  A null selection, for example, could result
	//in an unsuccessful update.
    bool operationCompleted;

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

}

#endif /* PLOTMS_H_ */
