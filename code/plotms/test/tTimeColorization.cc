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
#include <plotms/test/tUtil.h>


#include <iostream>
#include <msvis/MSVis/UtilJ.h>
#include <casa/namespace.h>
#include <QApplication>

/**
 * Tests whether a elevation/amp plot can be colorized with respect to time.
 */
int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath= tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tTimeColorization using data from "<<dataPath.c_str()<<endl;

    // Set up plotms object.
    PlotMSApp app(false, false);


    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSOverPlot::makeParameters(&app);

    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );

    //Set-up a elevation x amp plot
    PMS_PP_Cache* cacheParams = plotParams.typedGroup<PMS_PP_Cache>();
    if(cacheParams == NULL) {
    	plotParams.setGroup<PMS_PP_Cache>();
    	cacheParams = plotParams.typedGroup<PMS_PP_Cache>();
    }
    PMS::Axis xAxis = PMS::EL0;
    cacheParams->setXAxis(xAxis, PMS::DATA);
    PMS::Axis yAxis = PMS::AMP;
    cacheParams->setYAxis(yAxis, PMS::DATA);

    //Set-up colorization with respect to time.
    PMS_PP_Display* displayParams = plotParams.typedGroup<PMS_PP_Display>();
    if ( displayParams == NULL ){
        plotParams.setGroup<PMS_PP_Display>();
        displayParams = plotParams.typedGroup<PMS_PP_Display>();
    }
    displayParams->setColorize( true, PMS::TIME );

    //Add the plot
    app.addOverPlot( &plotParams );

    String outFile( "/tmp/plotTimeColorizationTest.jpg");
    tUtil::clearFile( outFile );
    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tTimeColorization:: Result of save="<<ok<<endl;
    
	ok = tUtil::checkFile( outFile, 160000, 170000, -1 );
	cout << "tTimeColorization:: Result of  save file check="<<ok<<endl;
	return tUtil::exitMain( false );
}

