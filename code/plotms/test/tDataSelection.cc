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
//#
//# $Id$

#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Plots/PlotMSOverPlot.h>

#include <stdlib.h>
#include <iostream>
#include <plotms/test/tUtil.h>
#include <casa/namespace.h>

int main(int /*argc*/, char** /*argv[]*/) {


	//Path for data
	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms");
    cout << "tDataSelection:: using data from "<<dataPath.c_str()<<endl;

    // Set up plotms object.
    PlotMSApp app(false, false );

    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSOverPlot::makeParameters(&app);

    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );

    PlotMSSelection sel = ppdata->selection();
    sel.setScan( "2,4");
    ppdata->setSelection(sel);

    app.addOverPlot( &plotParams );

    String outFile( "/tmp/plotDataSelectionTest.jpg");
    tUtil::clearFile( outFile );
    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool interactive = false;
	bool ok = app.save(format, interactive );
	cout << "tDataSelection - result of save="<<ok<<endl;
    
	bool okOutput = tUtil::checkFile( outFile, 70000, 80000, -1 );
	cout << "tDataSelection - result of first saved file check="<<okOutput<<endl;
	if ( okOutput ){
		cout << "tDataSelection Pass!"<<endl;
	}
	else {
		cout << "tDataSelection Fail!"<<endl;
	}
	return tUtil::exitMain( false );
}

