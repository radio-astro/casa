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

#include "ActionExport.h"
#include <plotms/Client/Client.h>
#include <plotms/Actions/ActionFactory.h>
#include <plotms/Threads/ThreadController.h>
#include <plotms/Threads/ExportThread.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <iomanip>
#include <fstream>

#include <QDebug>

namespace casa {

ActionExport::ActionExport( Client* client )
: PlotMSAction( client ), format( PlotExportFormat::JPG, ""){
	plot = NULL;
	itsType_= PLOT_EXPORT;
	interactive = false;
}

void ActionExport::setExportFormat( const PlotExportFormat& format ){
	this->format = format;
}

void ActionExport::setInteractive( bool interactive ){
	this->interactive = interactive;
}

bool ActionExport::loadParameters(){
	bool parametersLoaded = false;
	if ( client != NULL ){
		plot = client->getCurrentPlot();
		//The format may already be set, in which case we would not
		//want to reset it.
		if ( format.location.length() == 0 ){
			format = client->getPlotExportFormat();
			interactive = client->isInteractive();
		}
		if ( plot != NULL  ){
			if ( !format.location.empty() ){
				parametersLoaded = true;
			}
		}
	}
	return parametersLoaded;
}

bool ActionExport::doActionSpecific(PlotMSApp* plotms){
	bool ok = true;
	String form = PlotExportFormat::exportFormat( format.type );
	PlotExportFormat::Type t = PlotExportFormat::exportFormat(form, &ok);
	if(!ok) {
		t = PlotExportFormat::typeForExtension(format.location, &ok);
		if(!ok) {
			itsDoActionResult_ = "Invalid format extension for filename '"+
					format.location + "'!";
			return ok;
		}
	}

	if(t == PlotExportFormat::TEXT) {
		Record rec;
		CountedPtr<PlotMSAction> action = ActionFactory::getAction( SEL_INFO, client );
		//PlotMSAction action(PlotMSAction::SEL_INFO);
		ok = action->doActionWithResponse(plotms, rec);
		if(rec.nfields() < 1) return ok;
		// Write record data to file
		ofstream csv_file;
		csv_file.open(format.location.c_str());
		String xunit = rec.subRecord(0).asString("xaxis");
		String yunit = rec.subRecord(0).asString("yaxis");
		csv_file << "# x y chan scan field ant1 ant2 ant1name "
				<< "ant2name time freq spw corr offset currchunk irel"
				<< endl;
		csv_file << "# " << xunit << " " << yunit
				<< " None None None None None None None "
				<< "MJD(seconds) GHz None None None None None"
				<< endl;
		for(uInt n = 0; n < rec.nfields(); ++n) {
			Record r = rec.subRecord(n);
			csv_file << "# From plot " << n << endl;
			r.removeField("xaxis");
			r.removeField("yaxis");
			for(uInt _field = 0; _field < r.nfields(); ++_field) {
				ostringstream fs;
				fs << _field;
				String field_str = fs.str();
				Double x = r.subRecord(field_str).asDouble("x");
				Double y = r.subRecord(field_str).asDouble("y");
				Int chan = r.subRecord(field_str).asInt("chan");
				Int scan = r.subRecord(field_str).asInt("scan");
				Int field = r.subRecord(field_str).asInt("field");
				Int ant1 = r.subRecord(field_str).asInt("ant1");
				Int ant2 = r.subRecord(field_str).asInt("ant2");
				String ant1name =
						r.subRecord(field_str).asString("ant1name");
				String ant2name =
						r.subRecord(field_str).asString("ant2name");
				//String time = r.subRecord(field_str).asString("time");
				Double time = r.subRecord(field_str).asDouble("time");
				Int spw = r.subRecord(field_str).asInt("spw");
				Double freq = r.subRecord(field_str).asDouble("freq");
				String corr = r.subRecord(field_str).asString("corr");
				Int offset = r.subRecord(field_str).asInt("offset");
				Int currchunk = r.subRecord(field_str).asInt("currchunk");
				Int irel = r.subRecord(field_str).asInt("irel");
				int precision = csv_file.precision();
				if(xunit == "Time") {
					csv_file << std::setprecision(3) << std::fixed
							<< x << " ";
					csv_file.unsetf(ios_base::fixed);
					csv_file.precision(precision);
				} else if(xunit == "Frequency") {
					csv_file << std::setprecision(9) << std::fixed
							<< x << " ";
					csv_file.unsetf(ios_base::fixed);
					csv_file.precision(precision);
				} else {
					csv_file << x << " ";
				}
				if(yunit == "Time") {
					csv_file << std::setprecision(3) << std::fixed
							<< y << " ";
					csv_file.unsetf(ios_base::fixed);
					csv_file.precision(precision);
				} else if(yunit == "Frequency") {
					csv_file << std::setprecision(9) << std::fixed
							<< y << " ";
					csv_file.unsetf(ios_base::fixed);
					csv_file.precision(precision);
				} else {
					csv_file << y << " ";
				}
				csv_file << chan << " " << scan << " " << field << " "
						<< ant1 << " " << ant2 << " " << ant1name << " "
						<< ant2name << " ";
				csv_file << std::setprecision(3) << std::fixed
						<< time << " ";
				csv_file << std::setprecision(9) << std::fixed
						<< freq << " ";
				csv_file.unsetf(ios_base::fixed);
				csv_file.precision(precision);
				csv_file << spw << " " << corr << " " << offset << " "
						<< currchunk << " " << irel << endl;
			}
		}
		csv_file.close();
		return ok;
	}

	PlotExportFormat exportFormat(t, format.location);
	exportFormat.resolution = format.resolution;

	exportFormat.dpi = format.dpi;
	if(exportFormat.dpi <= 0){
		exportFormat.dpi = -1;
	}
	exportFormat.width = format.width;
	if(exportFormat.width <= 0){
		exportFormat.width = -1;
	}
	exportFormat.height = format.height;
	if(exportFormat.height <= 0){
		exportFormat.height = -1;
	}

    // TODO !export fix
    // Quick hack for screen resolution images.  Taking a screenshot without
    // drawing the items is basically impossible in the non-main (GUI) thread,
    // so for now just turn on high resolution so that it has to draw each
    // items.  This isn't ideal because it is slow, but for now it's better to
    // have something that works and is slow than something that doesn't work.
    if((exportFormat.type == PlotExportFormat::JPG ||
       exportFormat.type == PlotExportFormat::PNG) &&
       exportFormat.resolution == PlotExportFormat::SCREEN) {
    	cout << "NOTICE: Exporting to images in screen resolution is currently"
    	     << " not working.  Switching to high resolution (which is slower,"
    	     << " but works)." << endl;
    	exportFormat.resolution = PlotExportFormat::HIGH;
    }

	ExportThread* exportThread = new ExportThread();
	exportThread->setExportFormat( exportFormat );
	exportThread->setPlot( plot );
	setUpClientCommunication( exportThread );
	ok = initiateWork( exportThread );
	if ( threadController == NULL ){
		delete exportThread;
	}

	return ok;
}



ActionExport::~ActionExport() {
}

} /* namespace casa */
