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
#include <QDebug>

/**
 * Tests whether a plot can be placed in the second row, second column
 * of a 2x3 page layout.  The uneven number of rows and columns are important
 * to make sure placement is not switched or confused.
 */

int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    qDebug() << "tGridPlacement using data from "<<dataPath.c_str();

    // Set up plotms object
    PlotMSApp app(false, false);


    //Establish a 2x3 grid
    PlotMSParameters& params = app.getParameters();
    params.setRowCount( 2 );
    params.setColCount( 3 );

    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSOverPlot::makeParameters(&app);

    // Put the data into the plot.
    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );


    //Put the plot in the 2nd row, 2nd col
    PMS_PP_Iteration* iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
    if ( iterParams == NULL ){
    	plotParams.setGroup<PMS_PP_Iteration>();
    	iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
    }
    iterParams->setGridRow( 1 );
    iterParams->setGridCol( 1 );


    //Make the plot.
    app.addOverPlot( &plotParams );


    //QApplication::exec();

    //We want to print all pages in the output.
    PlotMSExportParam& exportParams = app.getExportParameters();
    exportParams.setExportRange( PMS::PAGE_ALL );




    String outFile( "/tmp/plotMSGridPlacementTest.jpg");
    tUtil::clearFile( outFile );

    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	qDebug() << "tGridPlacement:: Result of save="<<ok;
    

	ok = tUtil::checkFile( outFile, 60000, 80000, -1 );
	qDebug() << "tGridPlacement:: Result of save file check="<<ok;

	tUtil::exitMain( false );
}

