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
#include <plotms/Plots/PlotMSPlotManager.h>
#include <plotms/test/tUtil.h>


#include <iostream>
#include <msvis/MSVis/UtilJ.h>
#include <casa/namespace.h>
#include <QApplication>
#include <QDebug>

/**
 * Tests whether plots can be placed in a 2x3 page layout.  Then it changes
 * the grid size to 1x1 and generates a plot.  This tests whether
 * the grid size  can be dynamically changed in scripting mode.
 */

int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    qDebug() << "tGridPlacementMultipleRuns using data from "<<dataPath.c_str();

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


    //Put the plot in the first slot
    PMS_PP_Iteration* iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
    if ( iterParams == NULL ){
    	plotParams.setGroup<PMS_PP_Iteration>();
    	iterParams = plotParams.typedGroup<PMS_PP_Iteration>();
    }
    iterParams->setGridRow( 0 );
    iterParams->setGridCol( 0 );


    //Make the plot.
    app.addOverPlot( &plotParams );


    //Make a second plot
    PlotMSPlotParameters plotParams2 = PlotMSOverPlot::makeParameters(&app);

    PMS_PP_MSData* ppdata2 = plotParams2.typedGroup<PMS_PP_MSData>();
	if (ppdata2 == NULL) {
		plotParams2.setGroup<PMS_PP_MSData>();
		ppdata2 = plotParams2.typedGroup<PMS_PP_MSData>();
	}
	ppdata2->setFilename( dataPath );
	PMS_PP_Iteration* iterParams2 = plotParams2.typedGroup<PMS_PP_Iteration>();
	if ( iterParams2 == NULL ){
		plotParams2.setGroup<PMS_PP_Iteration>();
		iterParams2 = plotParams2.typedGroup<PMS_PP_Iteration>();
	}
	iterParams2->setGridRow( 0 );
	iterParams2->setGridCol( 1 );
	app.addOverPlot( &plotParams2 );

	 //Make a third plot
	 PlotMSPlotParameters plotParams3 = PlotMSOverPlot::makeParameters(&app);

	 PMS_PP_MSData* ppdata3 = plotParams3.typedGroup<PMS_PP_MSData>();
	 if (ppdata3 == NULL) {
		plotParams3.setGroup<PMS_PP_MSData>();
		ppdata3 = plotParams3.typedGroup<PMS_PP_MSData>();
	}
	ppdata3->setFilename( dataPath );
	PMS_PP_Iteration* iterParams3 = plotParams3.typedGroup<PMS_PP_Iteration>();
	if ( iterParams3 == NULL ){
		plotParams3.setGroup<PMS_PP_Iteration>();
		iterParams3 = plotParams3.typedGroup<PMS_PP_Iteration>();
	}
	iterParams3->setGridRow( 0 );
	iterParams3->setGridCol( 2 );
	app.addOverPlot( &plotParams3 );

	 //Make a fourth plot
	PlotMSPlotParameters plotParams4 = PlotMSOverPlot::makeParameters(&app);

	PMS_PP_MSData* ppdata4 = plotParams4.typedGroup<PMS_PP_MSData>();
	if (ppdata4 == NULL) {
		plotParams4.setGroup<PMS_PP_MSData>();
		ppdata4 = plotParams4.typedGroup<PMS_PP_MSData>();
	}
	ppdata4->setFilename( dataPath );
	PMS_PP_Iteration* iterParams4 = plotParams4.typedGroup<PMS_PP_Iteration>();
	if ( iterParams4 == NULL ){
		plotParams4.setGroup<PMS_PP_Iteration>();
		iterParams4 = plotParams3.typedGroup<PMS_PP_Iteration>();
	}
	iterParams4->setGridRow( 1 );
	iterParams4->setGridCol( 0 );
	app.addOverPlot( &plotParams4 );

	 //Make a fifth plot
	PlotMSPlotParameters plotParams5 = PlotMSOverPlot::makeParameters(&app);

	PMS_PP_MSData* ppdata5 = plotParams5.typedGroup<PMS_PP_MSData>();
	if (ppdata5 == NULL) {
		plotParams5.setGroup<PMS_PP_MSData>();
		ppdata5 = plotParams5.typedGroup<PMS_PP_MSData>();
	}
	ppdata5->setFilename( dataPath );
	PMS_PP_Iteration* iterParams5 = plotParams5.typedGroup<PMS_PP_Iteration>();
	if ( iterParams5 == NULL ){
		plotParams5.setGroup<PMS_PP_Iteration>();
		iterParams5 = plotParams5.typedGroup<PMS_PP_Iteration>();
	}
	iterParams5->setGridRow( 1 );
	iterParams5->setGridCol( 0 );
	app.addOverPlot( &plotParams5 );

	 //Make a sixth plot
	PlotMSPlotParameters plotParams6 = PlotMSOverPlot::makeParameters(&app);

	PMS_PP_MSData* ppdata6 = plotParams6.typedGroup<PMS_PP_MSData>();
	if (ppdata6 == NULL) {
		plotParams6.setGroup<PMS_PP_MSData>();
		ppdata6 = plotParams6.typedGroup<PMS_PP_MSData>();
	}
	ppdata6->setFilename( dataPath );
	PMS_PP_Iteration* iterParams6 = plotParams6.typedGroup<PMS_PP_Iteration>();
	if ( iterParams6 == NULL ){
		plotParams6.setGroup<PMS_PP_Iteration>();
		iterParams6 = plotParams6.typedGroup<PMS_PP_Iteration>();
	}
	iterParams6->setGridRow( 1 );
	iterParams6->setGridCol( 0 );
	app.addOverPlot( &plotParams6 );

    //We want to print all pages in the output.
    PlotMSExportParam& exportParams = app.getExportParameters();
    exportParams.setExportRange( PMS::PAGE_ALL );

    String outFile( "/tmp/plotMSGridPlacementMultipleRuns1Test.jpg");
    tUtil::clearFile( outFile );

    PlotExportFormat::Type type2 = PlotExportFormat::JPG;
	PlotExportFormat format(type2, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	qDebug() << "tGridPlacementMultipleRuns 1:: Result of save="<<ok;
    
	ok = tUtil::checkFile( outFile, 311000, 312000, -1 );
	qDebug() << "tGridPlacementMultipleRuns 1:: Result of save file check="<<ok;

	 //Now neck down the grid size
	 params.setGridSize( 1, 1);
	 app.setParameters( params );

	 //Make a third plot
	 PlotMSPlotParameters plotParams7 = PlotMSOverPlot::makeParameters(&app);

	 PMS_PP_MSData* ppdata7 = plotParams7.typedGroup<PMS_PP_MSData>();
	 if (ppdata7 == NULL) {
	 		plotParams7.setGroup<PMS_PP_MSData>();
	 		ppdata7 = plotParams7.typedGroup<PMS_PP_MSData>();
	 }
	 ppdata7->setFilename( dataPath );
	 PMS_PP_Iteration* iterParams7 = plotParams7.typedGroup<PMS_PP_Iteration>();
	 if ( iterParams7 == NULL ){
	 	plotParams7.setGroup<PMS_PP_Iteration>();
	 	iterParams7 = plotParams7.typedGroup<PMS_PP_Iteration>();
	 }
	 iterParams7->setGridRow( 0 );
	 iterParams7->setGridCol( 0 );
	 app.addOverPlot( &plotParams7 );

	 //Export the second version.
	 String outFile2( "/tmp/plotMSGridPlacementMultipleRuns2Test.jpg");
	 tUtil::clearFile( outFile2 );
	 PlotExportFormat format2(type2, outFile2 );
	 format2.resolution = PlotExportFormat::SCREEN;
	 ok = app.save(format2);
	 qDebug() << "tGridPlacementMultipleRuns 2:: Result of save="<<ok;

	ok = tUtil::checkFile( outFile2, 231000, 232000, -1 );
	qDebug() << "tGridPlacementMultipleRuns 2:: Result of save file check="<<ok;

	tUtil::exitMain( false );
}

