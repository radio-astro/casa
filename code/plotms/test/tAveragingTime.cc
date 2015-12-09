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

#include <stdlib.h>
#include <iostream>
#include <plotms/test/tUtil.h>
#include <casa/namespace.h>

int main(int /*argc*/, char** /*argv[]*/) {

    // CAS-2884 (plotms averages all channels with avgchannel = '1')

    //Path for data
    String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tAveragingChannel:: using data from "<<dataPath.c_str()<<endl;
    String exportPath = tUtil::getExportPath();
    cout << "Writing plotfiles to " << exportPath << endl;

    // Set up plotms object.
    PlotMSApp app(false, false );

    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSPlot::makeParameters(&app);

    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );

    // Set up channel vs time plot
    PMS_PP_Cache* cacheParams = plotParams.typedGroup<PMS_PP_Cache>();
    if(cacheParams == NULL) {
    	plotParams.setGroup<PMS_PP_Cache>();
    	cacheParams = plotParams.typedGroup<PMS_PP_Cache>();
    }
    PMS::Axis yAxis = PMS::CHANNEL;
    cacheParams->setYAxis(yAxis, PMS::DATA);
    
    //Add the plot
    app.addOverPlot( &plotParams );

    // Export format for all tests
    PlotExportFormat::Type type = PlotExportFormat::JPG;

    // start with no averaging
    String outFile = exportPath + "plotAveragingTimeTest1.jpg";
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;

	bool ok = app.save(format);
	cout << "tAveragingTime test 1 - result of save=" << ok << endl;
	bool okOutput = tUtil::checkFile( outFile, 300000, 335000, -1 );
	cout << "tAveragingTime test 1 - result of first saved file check="<< okOutput << endl;
    bool test1 = ok && okOutput;

    // Now turn time averaging on to 600
    PlotMSAveraging averaging = ppdata->averaging();
    averaging.setTime( true);
    averaging.setTimeValue( 600.0 );
    ppdata->setAveraging( averaging );

    app.clearPlots();
    app.addOverPlot( &plotParams );

	//Export the plot again
    String outFile2 = exportPath + "plotAveragingTimeTest2.jpg";
	PlotExportFormat format2(type, outFile2 );
	format2.resolution = PlotExportFormat::SCREEN;

	ok = app.save(format2);
	cout << "tAveragingTime test 2 - result of save=" << ok <<endl;
    // This plot file should be smaller because of averaging
	okOutput = tUtil::checkFile( outFile2, 150000, 180000, -1 );
	cout << "tAveragingTime test 2 - result of second saved file check=" << okOutput << endl;
    bool test2 = ok && okOutput;

    // clean up
    tUtil::clearFile(outFile);
    tUtil::clearFile(outFile2);
    tUtil::clearFile(exportPath);

    bool test = test1 && test2;
	bool checkGui = tUtil::exitMain( false );
    return !(test && checkGui);
}

