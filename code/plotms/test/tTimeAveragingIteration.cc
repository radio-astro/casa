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
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/test/tUtil.h>


#include <iostream>
#include <msvis/MSVis/UtilJ.h>
#include <casa/namespace.h>
#include <QApplication>

/**
 * Tests whether a elevation/amp plot can be iterated with respect to time.
 * Here we are averaging time.
 */
int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath= tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tTimeAveragingIteration using data from "<<dataPath.c_str()<<endl;

    // Set up plotms object.
    PlotMSApp app(false, false);


    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSPlot::makeParameters(&app);

    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );

    //Set time averaging of 100000 seconds.
    PlotMSAveraging averaging;
    averaging.setTime( true );
    averaging.setTimeValue( 3600 );
    ppdata->setAveraging( averaging );

    //Make a 2x2 grid
    PlotMSParameters& overallParams = app.getParameters();
    overallParams.setRowCount( 1 );
    overallParams.setColCount( 2 );

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

    //Set-up iteration with respect to time.
    PMS_PP_Iteration* iterationParams = plotParams.typedGroup<PMS_PP_Iteration>();
    if ( iterationParams == NULL ){
        plotParams.setGroup<PMS_PP_Iteration>();
        iterationParams = plotParams.typedGroup<PMS_PP_Iteration>();
    }
    iterationParams->setIterationAxis( PMS::TIME );

    //Add the plot
    app.addOverPlot( &plotParams );

    String outFile( "/tmp/plotTimeAveragingIterationTest");
    String outFile1( outFile + "_Time09:21:44.9994 - 09:50:44.9995,09:50:44.9995 - 10:46:15.0001.jpg");
    tUtil::clearFile( outFile1 );
    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile + ".jpg" );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tTimeAveragingIteration:: Result of save="<<ok<<endl;
    
	ok = tUtil::checkFile( outFile1, 100000, 102000, -1 );
	cout << "tTimeAveragingIteration:: Result of  save file check="<<ok<<endl;
	return tUtil::exitMain( false );
}

