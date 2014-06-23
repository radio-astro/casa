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
#include <synthesis/MSVis/UtilJ.h>
#include <casa/namespace.h>
#include <QApplication>
#include <QString>
#include <QDebug>
/**
 * Tests whether multiple plots can be displayed in a grid.
 */

int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    qDebug() << "tGridPlacementMultiplePlots using data from "<<dataPath.c_str();

    // Set up plotms object
    PlotMSApp app(false, false);


    //Establish a 2x2 grid
    PlotMSParameters& params = app.getParameters();
    params.setRowCount( 2 );
    params.setColCount( 2 );



    //Make the plots
    for ( int i = 0; i < 4; i++ ){

    	// Set up parameters for the plot.
    	PlotMSPlotParameters plotParams = PlotMSOverPlot::makeParameters(&app);

    	//Plots use the same data.
    	PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    	if (ppdata == NULL) {
    		plotParams.setGroup<PMS_PP_MSData>();
    		ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    	}
    	ppdata->setFilename( dataPath );



    	//Put the plot in the correct location.
    	PMS_PP_Iteration* iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
    	if ( iterParams == NULL ){
    		plotParams.setGroup<PMS_PP_Iteration>();
    		iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
    	}
    	int rowIndex = i / 2;
    	int colIndex = i % 2;
    	iterParams->setGridRow( rowIndex );
    	iterParams->setGridCol( colIndex );

    	//Add the plot.
    	app.addOverPlot( &plotParams );
    }

    //We want to print all pages in the output.
    PlotMSExportParam& exportParams = app.getExportParameters();
    exportParams.setExportRange( PMS::PAGE_ALL );

    String outFile( "/tmp/plotMSGridPlacementMultiplePlotsTest.jpg");
    tUtil::clearFile( outFile );

    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	qDebug() << "tGridPlacementMultiplePlots:: Result of save="<<ok;
    

	ok = tUtil::checkFile( outFile, 250000, 300000, -1 );
	qDebug() << "tGridPlacementMultiplePlots:: Result of save file check="<<ok;

	tUtil::exitMain( false );
}

