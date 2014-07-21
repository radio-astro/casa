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

#ifndef CLIENT_H_
#define CLIENT_H_

#include <plotms/Actions/PlotMSAction.h>
#include <plotms/PlotMS/PlotMSFlagging.h>
#include <graphics/GenericPlotter/PlotOptions.h>
#include <plotms/PlotMS/PlotMSLabelFormat.h>

namespace casa {

class PlotMSPlot;

/**
 * Abstraction for the interface between the client (possibly a GUI) and the
 * model (plot engine) that is performing the work.
 */

class Client {
public:
	//Returns whether or not the user can invoke a particular action.
	virtual bool isActionEnabled( PlotMSAction::Type type ) const = 0;

	//Return the current client plot.
	virtual vector<PlotMSPlot*> getCurrentPlots() const = 0;

	//Retrieve the plot load axes the user has specified.
	virtual vector<vector<PMS::Axis> > getSelectedLoadAxes() const = 0;

	//Retrieve the release axes the user has specified.
	virtual vector<vector<PMS::Axis> > getSelectedReleaseAxes() const = 0;



	//Retrieve flagging information specified by the client.
	virtual PlotMSFlagging getFlagging() const = 0;

	//In the case of a grid of plots whether to use common x- or y- axes.
	virtual void setCommonAxes( bool commonX, bool commonY );
	virtual bool isCommonAxisX() const;
	virtual bool isCommonAxisY() const;

	virtual void setAxisLocation( PlotAxis locationX, PlotAxis locationY );
	virtual PlotAxis getAxisLocationX() const;
	virtual PlotAxis getAxisLocationY() const;

	//Flagging
	virtual void setFlagging(PlotMSFlagging flag) = 0;

	//Return whether the client is interactive (a GUI) or noninteractive
	//(a script)
	virtual bool isInteractive() const = 0;

	virtual bool exportToFormat(const PlotExportFormat& format);

	//Save the current plot to a file.
	virtual bool exportPlot(const PlotExportFormat& format,
			const bool async) = 0;

	//Display an error in a client dependent way.  For a GUI, this may
	//mean popping up a dialog; for a script, this may mean writing the
	//error to a log file or web log.
	virtual void showError(const String& message,
			const String& title, bool isWarning) = 0;

	//Display a message.  For a GUI client, this may mean popping up a dialog;
	//for a script client, this may mean writing the message to a log file or
	//a web log.
	virtual void showMessage(const String& message, const String& title, bool warning = false) = 0;


	virtual void setCanvasCachedAxesStackImageSize(int width, int height );

	//Get the abstraction that holds the plots.
	virtual PlotterPtr getPlotter();

	//Use to enable annotations in the GUI client
	virtual void setAnnotationModeActive( PlotMSAction::Type /*type*/, bool /*active*/ ){

	}

	//Ask the client to make a deep copy of the given plot symbol.  The client
	//(and only the client) should have access to the PlotFactory that makes
	//the plot components.
	virtual PlotSymbolPtr createSymbol( const PlotSymbolPtr& copy );
	//Ask the client to make a symbol with the given specifications for insertion
	//into the plot.
	virtual PlotSymbolPtr createSymbol(const String& descriptor, Int size,
			const String& color, const String& fillPattern, bool outline);

	//Logging is client (implementation) specific so that client needs to
	//be asked for the logger.
	virtual PlotLoggerPtr getLogger();

	//Add a plot to those displayed.
	virtual void canvasAdded( PlotCanvasPtr& canvas ) = 0;

	//Is the plot visible?
	virtual bool isVisible(PlotCanvasPtr& canvas );

	//The client is responsible for doing threaded operations.  For a GUI
	//client this will involve starting a background thread so that the GUI
	//is not frozen.  For a script client, this may be a no-opt as the script
	//can just wait for the operation to complete.
	virtual void doThreadedOperation( ThreadController* controller ) = 0;

	//FactoryMethod for producing a thread controller to manage a specific type
	//of threaded operation.  Examples include caching, exporting a plot, and
	//drawing.
	virtual ThreadController* getThreadController( PlotMSAction::Type type,
			PMSPTMethod postThreadMethod = NULL,
			PlotMSPlot* postThreadObject = NULL,
			int index = 0) = 0;



	virtual vector<PlotCanvasPtr> currentCanvases();

	//Keeps resizes from flooding event loop (dragging).
	virtual void holdDrawing();
	virtual void releaseDrawing();
	virtual bool allDrawingHeld() const;

	virtual void setOperationCompleted( bool completed );
	virtual void plot() = 0;
	virtual void showGUI( bool show = true) = 0;
	virtual int showAndExec(bool show = true);
	virtual int execLoop() = 0;
	virtual bool guiShown() const = 0;
	virtual bool close() = 0;
	virtual bool isDrawing() const = 0;
	virtual bool isClosed() const = 0;
	virtual PlotFactoryPtr getPlotFactory();
	virtual void gridSizeChanged( int /*rowCount*/, int /*colCount*/ )=0;


protected:
	// Plotter.
	PlotterPtr itsPlotter_;
	// Plot factory.
	PlotFactoryPtr itsFactory_;

	Client(){};
	virtual ~Client(){};
};

}
#endif /* CLIENTINTERFACE_H_ */
