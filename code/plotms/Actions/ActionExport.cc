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
	itsType_= PLOT_EXPORT;
	interactive = false;
}

void ActionExport::setExportFormat( const PlotExportFormat& exportFormat ){
	format = exportFormat;
}

void ActionExport::setInteractive( bool interactive ){
	this->interactive = interactive;
}

bool ActionExport::loadParameters(){
	bool parametersLoaded = false;
	if ( client != NULL ){
		plots = client->getCurrentPlots();
		interactive = client->isInteractive();
		if ( plots.size() > 0  ){
			if ( !format.location.empty() ){
				parametersLoaded = true;
			}
		}
	}
    return parametersLoaded;
}

bool ActionExport::exportText( PlotMSApp* plotms ){
	Record rec;
	CountedPtr<PlotMSAction> action = ActionFactory::getAction( SEL_INFO, client );
	bool ok = action->doActionWithResponse(plotms, rec);
	if(rec.nfields() < 1) return ok;

	// Write record data to file
	ofstream csv_file;
	csv_file.open(format.location.c_str());
	Record firstRecord = rec.subRecord(0);
	String xunit = "";
	String yunit = "";
	const String X_AXIS_UNITS = "xaxis";
	const String Y_AXIS_UNITS = "yaxis";
	if ( firstRecord.isDefined( X_AXIS_UNITS )){
		xunit = firstRecord.asString(X_AXIS_UNITS);
	}
	if ( firstRecord.isDefined( Y_AXIS_UNITS)){
		yunit = rec.subRecord(0).asString(Y_AXIS_UNITS);
	}
	csv_file << "# x y chan scan field ant1 ant2 ant1name "
			<< "ant2name time freq spw corr offset currchunk irel"
			<< endl;
	if ( xunit.length() > 0 || yunit.length() > 0 ){
		csv_file << "# " << xunit << " " << yunit
			<< " None None None None None None None "
			<< "MJD(seconds) GHz None None None None None"
			<< endl;
	}

	for(uInt n = 0; n < firstRecord.nfields(); ++n) {
		Record r = firstRecord.subRecord(n);
		csv_file << "# From plot " << n << endl;
		if ( r.isDefined( X_AXIS_UNITS ) ){
			r.removeField( X_AXIS_UNITS );
		}
		if  ( r.isDefined( Y_AXIS_UNITS ) ){
			r.removeField( Y_AXIS_UNITS );
		}

		for(uInt _field = 0; _field < r.nfields(); ++_field) {
			ostringstream fs;
			fs << _field;
			String field_str = fs.str();
			Record fieldRecord = r.subRecord( field_str );
			Double x = fieldRecord.asDouble("x");
			Double y = fieldRecord.asDouble("y");
			Int chan = fieldRecord.asInt("chan");
			Int scan = fieldRecord.asInt("scan");
			Int field = fieldRecord.asInt("field");
			Int ant1 = fieldRecord.asInt("ant1");
			Int ant2 = fieldRecord.asInt("ant2");
			String ant1name =fieldRecord.asString("ant1name");
			String ant2name =fieldRecord.asString("ant2name");

			//String time = r.subRecord(field_str).asString("time");
			Double time = fieldRecord.asDouble("time");
			Int spw = fieldRecord.asInt("spw");
			Double freq = fieldRecord.asDouble("freq");
			String corr = fieldRecord.asString("corr");
			Int offset = fieldRecord.asInt("offset");
			Int currchunk = fieldRecord.asInt("currchunk");
			Int irel = fieldRecord.asInt("irel");

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

PlotExportFormat ActionExport::adjustFormat( PlotExportFormat::Type t){
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
	return exportFormat;
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

	if ( t == PlotExportFormat::TEXT ){
		ok = exportText(plotms);
	}
	else {

		PlotExportFormat exportFormat = adjustFormat( t );
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
		exportThread->setPlots( plots );
		setUpClientCommunication( exportThread, -1 );

		ok = initiateWork( exportThread );
		if ( threadController == NULL ){
			delete exportThread;
		}
	}
	return ok;
}



ActionExport::~ActionExport() {
}

} /* namespace casa */
