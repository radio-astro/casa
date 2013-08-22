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


#include "ClientScript.h"
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <plotms/Actions/ActionExport.h>
//#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <casaqt/PlotterImplementations/PlotterImplementations.h>
#include <QDebug>
#include <QWidget>
#include <QHBoxLayout>
namespace casa {

ClientScript::ClientScript(PlotMSApp* controllerApp, Plotter::Implementation impl) :
	SCRIPT_CLIENT( "PlotMS Script Client"){
	plotController = controllerApp;
	initialize( impl );
	currentPlot = NULL;
	//taskRunning = false;
}

bool ClientScript::isActionEnabled( PlotMSAction::Type /*type*/ ) const {
	//This methods is called at present for GUI actions like hover.
	return false;
}

void ClientScript::plot() {
	if ( currentPlot == NULL ){
		initializeCurrentPlot();
	}
	if ( currentPlot != NULL ){
		 // Tell the plot to redraw itself because of the cache.
		PlotMSPlotParameters& params = currentPlot->parameters();
		currentPlot->parametersHaveChanged(params,PlotMSWatchedParameters::ALL_UPDATE_FLAGS());
	}

}

void ClientScript::initializeCurrentPlot() {
	if ( currentPlot == NULL ){
		PlotMSPlotManager& manager = plotController->getPlotManager();
		int plotCount = manager.numPlots();
		if ( plotCount > 0 ){
			currentPlot = manager.plot( 0 );
		}
		else {
			qDebug() << "Plot manager did not have any plots";
		}
	}
}

PlotMSPlot* ClientScript::getCurrentPlot() const {
	return currentPlot;
}

bool ClientScript::isInteractive() const {
	return false;
}

bool ClientScript::isMSSummaryVerbose() const {
	return false;
}

PMS::SummaryType ClientScript::getMSSummaryType() const{
	return PMS::S_ALL;
}

//Retrieve flagging information specified by the client.
PlotMSFlagging ClientScript::getFlagging() const {
	return flagging;
}

void ClientScript::setFlagging(PlotMSFlagging flag) {
	flagging = flag;
}

bool ClientScript::exportPlot(const PlotExportFormat& format,
			const bool /*interactive*/, const bool /*async*/){
	//taskRunning = true;

	//Make sure there is data in the plot.
	plot();

	//Create an export action to do the work.
	ActionExport action( this );
	action.setUseThreading( false );
	action.setExportFormat( format );
	action.setInteractive( false );

	//Do the export
	bool result = action.doAction(plotController);
	if ( !result ){
		showError( action.doActionResult(), "Export Failed!", false );
	}
	//taskRunning = false;
	return result;
}


ThreadController* ClientScript::getThreadController( PlotMSAction::Type /*type*/,
			PMSPTMethod /*postThreadMethod*/, PMSPTObject /*postThreadObject*/ ) {
	return NULL;
}

void ClientScript::showGUI( bool show ) {
	itsPlotter_->showGUI(show);
}

void ClientScript::showMessage(const String& message, const String& title) {
	PlotLoggerPtr infoLog = itsPlotter_->logger();
	infoLog->postMessage(SCRIPT_CLIENT, title, message,PlotLogger::MSG_INFO);
}

void ClientScript::showError(const String& message, const String& title, bool warning){
	PlotLoggerPtr infoLog = itsPlotter_->logger();
	if ( warning ){
		infoLog->postMessage(SCRIPT_CLIENT, title, message,PlotLogger::MSG_WARN);
	}
	else {
		infoLog->postMessage(SCRIPT_CLIENT, title, message,PlotLogger::MSG_ERROR);
	}
}


void ClientScript::initialize(Plotter::Implementation impl){
	// Try to initialize plotter, and throw error on failure.
	itsFactory_ = plotterImplementation(impl);

	if( itsFactory_.null() ) {
		String error = "Invalid plotter implementation.";
		if(impl == Plotter::QWT){
			error += "  Make sure that you have qwt installed and the "
	                     "AIPS_HAS_QWT compiler flag turned on!";

	    }
		throw AipsError(error);
	}

	// Set up plotter.
	itsPlotter_ = itsFactory_->plotter("PlotMS", false, false,
	    		plotController->getParameters().logEvents(), false);

}

ClientScript::~ClientScript() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
