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

#include "ActionSummary.h"
#include <plotms/Client/Client.h>
#include <plotms/PlotMS/PlotMSLabelFormat.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameters.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSSummary.h>
#include <casa/Logging/LogFilter.h>

namespace casa {

ActionSummary::ActionSummary( Client* client )
	:PlotMSAction( client ){
	plot = NULL;
	itsType_=MS_SUMMARY;
}

bool ActionSummary::loadParameters(){
	bool parametersLoaded = false;
	if ( client != NULL ){
		plot = client->getCurrentPlot();
		if ( plot != NULL ){
			parametersLoaded = true;
		}
	}
	return parametersLoaded;
}

bool ActionSummary::doActionSpecific(PlotMSApp* plotms) {
	bool success = false, reenableGlobal = false;
	try {
		// Get MS.
		MeasurementSet ms;

		// Check if MS has already been opened.

	    // Check if filename has been set but not plotted.
		PlotMSPlotParameters currentlySet = client->getPlotParameters();
		String filename = PMS_PP_RETCALL(currentlySet, PMS_PP_MSData,
							 filename, "");

		// If not, exit.
		if(filename.empty()) {
			itsDoActionResult_ = "MS has not been opened/set yet!";
			return false;
		}

		ms= MeasurementSet(filename, TableLock(TableLock::AutoLocking),
					   Table::Old);

		// Set up MSSummary object.
		MSSummary mss(ms);

		// Set up log objects.
		LogSink sink(LogFilter(plotms->getLogger()->filterMinPriority()));
		if(!plotms->getLogger()->usingGlobalSink()) {
			LogSinkInterface* ic = plotms->getLogger()->localSinkCopy();
		    sink.localSink(ic);

		    // Temporarily disable global log sink if we're not using it, since
		    // MSSummary posts to both (how annoying).
		    PlotLogger::disableGlobalSink();
		    reenableGlobal = true;
		}
		LogIO log(LogOrigin(PMS::LOG_ORIGIN,PMS::LOG_ORIGIN_SUMMARY),sink);

		// Log summary of the appropriate type and verbosity.
		bool vb = client->isMSSummaryVerbose();
		switch( client->getMSSummaryType()) {
		        case PMS::S_ALL:          mss.list(log, vb); break;
		        case PMS::S_WHERE:        mss.listWhere(log, vb); break;
		        case PMS::S_WHAT:         mss.listWhat(log, vb); break;
		        case PMS::S_HOW:          mss.listHow(log, vb); break;
		        case PMS::S_MAIN:         mss.listMain(log, vb); break;
		        case PMS::S_TABLES:       mss.listTables(log, vb); break;
		        case PMS::S_ANTENNA:      mss.listAntenna(log, vb); break;
		        case PMS::S_FEED:         mss.listFeed(log, vb); break;
		        case PMS::S_FIELD:        mss.listField(log, vb); break;
		        case PMS::S_OBSERVATION:  mss.listObservation(log, vb); break;
		        case PMS::S_HISTORY:      mss.listHistory(log); break;
		        case PMS::S_POLARIZATION: mss.listPolarization(log, vb); break;
		        case PMS::S_SOURCE:       mss.listSource(log, vb); break;
		        case PMS::S_SPW:          mss.listSpectralWindow(log, vb); break;
		        case PMS::S_SPW_POL:      mss.listSpectralAndPolInfo(log,vb);break;
		        case PMS::S_SYSCAL:       mss.listSysCal(log, vb); break;
		        case PMS::S_WEATHER:      mss.listWeather(log, vb); break;
		  }
		  success = true;

	}
	catch(AipsError x) {
		itsDoActionResult_ = x.getMesg();
	}
	// Cleanup.
	if(reenableGlobal) PlotLogger::enableGlobalSink();
	return success;
}

ActionSummary::~ActionSummary() {
}

} /* namespace casa */
