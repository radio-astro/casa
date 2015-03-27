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
    // File size should be the same for first 3 cases
    int noAveragingSizeMin = 263000;
    int noAveragingSizeMax = 266000;

    // start with no averaging
    String outFile( "/tmp/plotAveragingChannelTest1.jpg");
    tUtil::clearFile( outFile );
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tAveragingChannel test 1 - result of save=" << ok << endl;
    
	bool okOutput = tUtil::checkFile( outFile, noAveragingSizeMin, noAveragingSizeMax, -1 );
	cout << "tAveragingChannel test 1 - result of first saved file check="<<okOutput<<endl;

    // Now turn channel averaging on to 1 (should be same as none)
    PlotMSAveraging averaging = ppdata->averaging();
    averaging.setChannel( true);
    averaging.setChannelValue( 1 );
    ppdata->setAveraging( averaging );

    app.clearPlots();
    app.addOverPlot( &plotParams );

	//Export the plot again
	String outFile2( "/tmp/plotAveragingChannelTest2.jpg");
	tUtil::clearFile( outFile2 );
	PlotExportFormat format2(type, outFile2 );
	format2.resolution = PlotExportFormat::SCREEN;
	bool ok2 = app.save(format2);
	cout << "tAveragingChannel test 2 - result of save=" << ok2 <<endl;
	bool okOutput2 = tUtil::checkFile( outFile2, noAveragingSizeMin, noAveragingSizeMax, -1 );
	cout << "tAveragingChannel test 2 - result of third saved file check="<<okOutput2<<endl;

    // Now turn channel averaging on to 3 for "normal" channel averaging
    averaging.setChannel( true);
    averaging.setChannelValue( 3.0 );
    ppdata->setAveraging( averaging );

    app.clearPlots();
    app.addOverPlot( &plotParams );

	//Export the plot again
	String outFile3( "/tmp/plotAveragingChannelTest3.jpg");
	tUtil::clearFile( outFile3 );
	PlotExportFormat format3(type, outFile3 );
	format3.resolution = PlotExportFormat::SCREEN;
	bool ok3 = app.save(format3);
	cout << "tAveragingChannel test 3 - result of save=" << ok3 <<endl;
        // This plot file should be smaller because of averaging
	bool okOutput3 = tUtil::checkFile( outFile3, 180000, 185000, -1 );
	cout << "tAveragingChannel test 3 - result of third saved file check="<<okOutput3<<endl;

	if ( okOutput  && okOutput2 && okOutput3 ){
		cout << "tAveragingChannel Pass!"<<endl;
	}
	else {
		cout << "tAveragingChannel Fail!"<<endl;
	}


	return tUtil::exitMain( false );
}

