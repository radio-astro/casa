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


	//Path for data
	String dataPath = "/lustre/bkent/evlapipeline/helpdesk/13A-398.sb17165245.eb19445220.56369.115810324074.ms";
    cout << "tAveraging:: using data from "<<dataPath.c_str()<<endl;

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


    //Use corrected data
    PMS_PP_Cache* cacheParams = plotParams.typedGroup<PMS_PP_Cache>();
    if ( cacheParams == NULL ){
    	plotParams.setGroup<PMS_PP_Cache>();
    	cacheParams = plotParams.typedGroup<PMS_PP_Cache>();
    }
    cacheParams->setYAxis (PMS::AMP, PMS::CORRECTED );

    //Turn channel and time averaging on.
    PlotMSAveraging averaging = ppdata->averaging();
    //averaging.setChannel( true);
    //averaging.setChannelValue( 65 );
    averaging.setTime( true );
    averaging.setTimeValue( 200 );
    ppdata->setAveraging( averaging );

    //Make some selections.
    PlotMSSelection sel = ppdata->selection();
    sel.setScan( "31");
    //Note:  the :3 selects on channel 3 so we can decrease the data size and not
    //average channels which doesn't seem to be the problem.
    sel.setSpw( "10:3");
    sel.setAntenna( "22&23");
    sel.setCorr( "RR");
    sel.setField( "2" );
    ppdata->setSelection(sel);

    //Colorize by spw
    PMS_PP_Display* displayParams = plotParams.typedGroup<PMS_PP_Display>();
    if ( displayParams == NULL ){
    	plotParams.setGroup<PMS_PP_Display>();
    	displayParams = plotParams.typedGroup<PMS_PP_Display>();
    }
    displayParams->setColorize( true, PMS::SPW );

    //Add the plot
    app.addOverPlot( &plotParams );

    //Export
    String outFile( "/tmp/plotAveragingTest.jpg");
    tUtil::clearFile( outFile );
    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tAveraging - result of save="<<ok<<endl;
    
	bool okOutput = tUtil::checkFile( outFile, 44000, 45000, -1 );
	cout << "tAveraging - result of first saved file check="<<okOutput<<endl;

	//Now turn time averaging on
	/*averaging.setTime( true );
	averaging.setTimeValue( 200 );
	ppdata->setAveraging( averaging);


	//Export the plot again
	String outFile2( "/tmp/plotAveragingTest2.jpg");
	tUtil::clearFile( outFile2 );
	PlotExportFormat::Type type2 = PlotExportFormat::JPG;
    PlotExportFormat format2(type2, outFile2 );
    format2.resolution = PlotExportFormat::SCREEN;
    bool ok2 = app.save(format2);
	cout << "tAveraging - result of save="<<ok2<<endl;

	bool okOutput2 = tUtil::checkFile( outFile2, 166000, 170000, -1 );
	cout << "tAveraging - result of second saved file check="<<okOutput2<<endl;
	if ( okOutput2  && okOutput){
		cout << "tAveraging Pass!"<<endl;
	}
	else {
		cout << "tAveraging Fail!"<<endl;
	}*/


	return tUtil::exitMain( false );
}

