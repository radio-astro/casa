//# PlotMSAction.h: Actions on plotms that can be triggered.
//# Copyright (C) 2009
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


#ifndef CLIENTSCRIPT_H_
#define CLIENTSCRIPT_H_

#include <plotms/Client/Client.h>
#include <plotms/Plots/PlotMSPlotParameters.h>
namespace casa {

class PlotMSPlot;
class PlotMSApp;

/**
 * Implementation of Client built for user scripting (non-GUI) user interface.
 */

class ClientScript : public Client {
public:
	ClientScript( PlotMSApp* app,Plotter::Implementation impl = Plotter::DEFAULT);
	virtual ~ClientScript();

	//Returns whether or not the user can invoke a particular action.
	virtual bool isActionEnabled( PlotMSAction::Type type ) const;

	//Return the current client plot.
	virtual vector<PlotMSPlot*> getCurrentPlots() const;

	//Retrieve the plot load axes the user has specified.
	virtual vector<vector<PMS::Axis> > getSelectedLoadAxes() const;

	//Retrieve the release axes the user has specified.
	virtual vector<vector<PMS::Axis> > getSelectedReleaseAxes() const;

	//Retrieve flagging information specified by the client.
	virtual PlotMSFlagging getFlagging() const;
	virtual void setFlagging(PlotMSFlagging flag);

	

	//Return whether the client is interactive (a GUI) or noninteractive
	//(a script)
	virtual bool isInteractive() const;

	


	//Save the current plot to a file.
	virtual bool exportPlot(const PlotExportFormat& format, const bool async);


	//Display an error in a client dependent way.  For a GUI, this may
	//mean popping up a dialog; for a script, this may mean writing the
	//error to a log file or web log.
	virtual void showError(const String& message, const String& title, bool warning );

	//Display a message.  For a GUI client, this may mean popping up a dialog;
	//for a script client, this may mean writing the message to a log file or
	//a web log.
	virtual void showMessage(const String& message, const String& title, bool warning = false);


	//Add a plot to those displayed.
	virtual void canvasAdded( PlotCanvasPtr& /*canvas*/ ){

	}

	virtual void setOperationCompleted( bool completed );

	//The client is responsible for doing threaded operations.  For a GUI
	//client this will involve starting a background thread so that the GUI
	//is not frozen.  For a script client, this may be a no-opt as the script
	//can just wait for the operation to complete.
	virtual void doThreadedOperation( ThreadController* /*controller*/ ){

	}

	//FactoryMethod for producing a thread controller to manage a specific type
	//of threaded operation.  Examples include caching, exporting a plot, and
	//drawing.
	virtual ThreadController* getThreadController( PlotMSAction::Type type,
			PMSPTMethod postThreadMethod = NULL,
			PlotMSPlot* postThreadObject = NULL,
			int index = 0);

	virtual void plot();
	virtual void showGUI( bool show = true);

	virtual int execLoop() {
		return itsFactory_->execLoop();
	}
	virtual bool guiShown() const {
		return false;
	}
	virtual bool close() {
		return true;
	}
	virtual bool isDrawing() const{
		return false;//taskRunning;
	}
	virtual bool isClosed() const {
		return false;
	}
	virtual void gridSizeChanged( int /*rowCount*/, int /*colCount*/ );

	virtual vector<String> getFiles() const;

private:
	void initializeCurrentPlot();
	void initialize( Plotter::Implementation impl );
	PlotMSApp* plotController;
	PlotMSFlagging flagging;
	vector<PlotMSPlot*> currentPlots;

	const String SCRIPT_CLIENT;
};

} /* namespace casa */
#endif /* CLIENTSCRIPT_H_ */
