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

#ifndef EXPORTTHREAD_H_
#define EXPORTTHREAD_H_

#include <plotms/Threads/BackgroundThread.h>
#include <graphics/GenericPlotter/PlotOptions.h>

namespace casa {

class PlotMSPlot;

/**
 * Exports a plot to a specific format in a background
 * thread.
 */

class ExportThread : public BackgroundThread {
public:
	ExportThread();

	//Store the format that will be used to export the plot.
	void setExportFormat(PlotExportFormat exportFormat );

	//Store the plot to export
	void setPlot( PlotMSPlot* plot );

	virtual ~ExportThread();
protected:
	virtual bool doWork();
	virtual void cancelWork();
private:
	PlotExportFormat format;
	PlotMSPlot* exportPlot;
	ExportThread( const ExportThread& other );
	ExportThread operator=( const ExportThread& other );
};

} /* namespace casa */
#endif /* EXPORTTHREAD_H_ */
