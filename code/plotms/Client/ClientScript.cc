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
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <casaqt/PlotterImplementations/PlotterImplementations.h>
#include <QDebug>
#include <QWidget>
#include <QHBoxLayout>
namespace casa {

ClientScript::ClientScript(PlotMSApp* controllerApp, Plotter::Implementation impl) :
	SCRIPT_CLIENT( "PlotMS Script Client"){
	plotController = controllerApp;
	initialize( impl );
}

bool ClientScript::isActionEnabled( PlotMSAction::Type /*type*/ ) const {
	//This methods is called at present for GUI actions like hover.
	return false;
}

void ClientScript::plot() {
	if ( currentPlots.size() == 0 ){
		initializeCurrentPlot();
	}


	//This is present because it forces a hidden widget to update.
	//Without it, sometimes the exported plot was not appearing correctly,
	//i.e., a missing y-axis.
	itsPlotter_->updateScriptGui();
}

void ClientScript::initializeCurrentPlot() {
	if ( currentPlots.size() == 0){
		PlotMSPlotManager& manager = plotController->getPlotManager();
		int plotCount = manager.numPlots();
		for ( int i = 0; i < plotCount; i++ ){
			currentPlots.push_back(manager.plot( i ));
		}
	}
}

vector<PlotMSPlot*> ClientScript::getCurrentPlots() const {
	return currentPlots;
}

bool ClientScript::isInteractive() const {
	return false;
}

void ClientScript::setOperationCompleted( bool completed ){
	plotController->setOperationCompleted( completed );
}


//Retrieve flagging information specified by the client.
PlotMSFlagging ClientScript::getFlagging() const {
	return flagging;
}

void ClientScript::setFlagging(PlotMSFlagging flag) {
	flagging = flag;
}

bool ClientScript::exportPlot(const PlotExportFormat& format,
		 const bool /*async*/){

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
	return result;
}

//Retrieve the plot load axes the user has specified.
vector<vector<PMS::Axis> > ClientScript::getSelectedLoadAxes() const {
	vector<vector<PMS::Axis> > axes;
	return axes;
}

//Retrieve the release axes the user has specified.
vector<vector<PMS::Axis> > ClientScript::getSelectedReleaseAxes() const {
	vector<vector<PMS::Axis> > axes;
	return axes;
}


ThreadController* ClientScript::getThreadController( PlotMSAction::Type /*type*/,
			PMSPTMethod /*postThreadMethod*/,
			PlotMSPlot* /*postThreadObject*/,
			int /*index*/ ) {
	return NULL;
}

void ClientScript::showGUI( bool /*show*/ ) {
	//itsPlotter_->showGUI(true);
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
