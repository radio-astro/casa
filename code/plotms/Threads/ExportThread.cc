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

#include "ExportThread.h"
#include <plotms/Plots/PlotMSPlot.h>
#include <QDebug>
namespace casa {

ExportThread::ExportThread()
: format( PlotExportFormat::JPG, "" ) {
}

void ExportThread::setExportFormat(PlotExportFormat exportFormat ){
	format = exportFormat;
}

void ExportThread::setPlots( vector<PlotMSPlot*> plots ){
	exportPlots = plots;
}

bool ExportThread::doWork(){
	bool result = true;
	int count = exportPlots.size();
	//for ( int i = 0; i < count; i++ ){
	if ( count > 0 ){
		result = exportPlots[0]->exportToFormat( format );
	}
	//}
	return result;
}

void ExportThread::cancelWork(){
	int count = exportPlots.size();
	//for ( int i = 0; i < count; i++ ){
	if ( count > 0 ){
		exportPlots[0]->exportToFormatCancel();
	}
	//}
}

ExportThread::~ExportThread() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
