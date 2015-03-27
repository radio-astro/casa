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
#include <graphics/GenericPlotter/PlotOptions.h>
#include <plotms/test/tUtil.h>


#include <iostream>
#include <msvis/MSVis/UtilJ.h>
#include <casa/namespace.h>
#include <QApplication>

/**
 * Tests whether multiple plot types can be supported on the same grid.
 * Make a 2x2 grid.  Fill the (1,1) slot with an overplot,
 * the (1,2) slot with a normal plot, and then the
 * bottom row with an iteration plot that extends to the
 * next page.
 */
int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tMultiplePlotTypes using data from "<<dataPath.c_str()<<endl;

    // Set up plotms object.
    PlotMSApp app(false, false);

    //Establish a 2x2 grid
    PlotMSParameters& params = app.getParameters();
    params.setRowCount( 2 );
    params.setColCount( 2 );

    /******************************************************************
     * OVERPLOT
     */

    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSPlot::makeParameters(&app);

    // Data
    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );

    //Cache and Axes
    PMS_PP_Cache* cacheParams = plotParams.typedGroup<PMS_PP_Cache>();
    PMS_PP_Axes* axesParams = plotParams.typedGroup<PMS_PP_Axes>();
    if(cacheParams == NULL) {
    	plotParams.setGroup<PMS_PP_Cache>();
        cacheParams = plotParams.typedGroup<PMS_PP_Cache>();
    }
    if(axesParams == NULL) {
       plotParams.setGroup<PMS_PP_Axes>();
       axesParams = plotParams.typedGroup<PMS_PP_Axes>();
    }

    //The cache must have exactly as many x-axes as y-axes so we duplicate
    //the x-axis properties here.
     int yAxisCount = 2;
     PMS::Axis xAxis = PMS::TIME;
     for ( int i = 0; i < yAxisCount; i++ ){
    	 cacheParams->setXAxis(xAxis, PMS::DATA, i);
         axesParams->setXAxis( X_BOTTOM, i);
     }
     PMS::Axis yAxis1 = PMS::SCAN;
     cacheParams->setYAxis(yAxis1, PMS::DATA, 0);
     axesParams->setYAxis( Y_LEFT, 0);
     PMS::Axis yAxis2 = PMS::FIELD;
     cacheParams->setYAxis(yAxis2, PMS::DATA, 1);
     axesParams->setYAxis(Y_LEFT, 1);

     //We need to set a distinctive color for each set of data
     //so we can distinguish it.
     PMS_PP_Display* ppdisp = plotParams.typedGroup<PMS_PP_Display>();
     if (ppdisp == NULL) {
    	 plotParams.setGroup<PMS_PP_Display>();
         ppdisp = plotParams.typedGroup<PMS_PP_Display>();
     }
     PlotSymbolPtr ps = app.createSymbol ("diamond", 5, "00FF00", "mesh2", false );
     ppdisp->setUnflaggedSymbol(ps, 0);
     PlotSymbolPtr ps2 = app.createSymbol ("diamond", 5, "0000FF", "mesh2", false );
     ppdisp->setUnflaggedSymbol(ps2, 1);
     app.addOverPlot( &plotParams );

     /**
      * Normal Plot
      */
     PlotMSPlotParameters plotParams2 = PlotMSPlot::makeParameters(&app);

     PMS_PP_MSData* ppdata2 = plotParams2.typedGroup<PMS_PP_MSData>();
     if (ppdata2 == NULL) {
    	 plotParams2.setGroup<PMS_PP_MSData>();
    	 ppdata2 = plotParams2.typedGroup<PMS_PP_MSData>();
     }
     ppdata2->setFilename( dataPath );

     PMS_PP_Iteration* iterationParams2 = plotParams2.typedGroup<PMS_PP_Iteration>();
     if ( iterationParams2 == NULL ){
    	 plotParams2.setGroup<PMS_PP_Iteration>();
         iterationParams2 = plotParams2.typedGroup<PMS_PP_Iteration>();
     }
     iterationParams2->setGridRow(0);
     iterationParams2->setGridCol(1);

     app.addOverPlot( &plotParams2 );


     /**
      * Iteration Plot
      */
     PlotMSPlotParameters plotParams3 = PlotMSPlot::makeParameters(&app);

     PMS_PP_MSData* ppdata3 = plotParams3.typedGroup<PMS_PP_MSData>();
     if (ppdata3 == NULL) {
    	 plotParams3.setGroup<PMS_PP_MSData>();
         ppdata3 = plotParams3.typedGroup<PMS_PP_MSData>();
     }
     ppdata3->setFilename( dataPath );

     PMS_PP_Iteration* iterationParams3 = plotParams3.typedGroup<PMS_PP_Iteration>();
     if ( iterationParams3 == NULL ){
    	 plotParams3.setGroup<PMS_PP_Iteration>();
         iterationParams3 = plotParams3.typedGroup<PMS_PP_Iteration>();
     }
     iterationParams3->setGridRow(0);
     iterationParams3->setGridCol(1);
     iterationParams3->setIterationAxis( PMS::SCAN );

     app.addOverPlot( &plotParams3 );


    /**
     * Export
     */

    //We want to print all (2) pages in the output.
    PlotMSExportParam& exportParams = app.getExportParameters();
    exportParams.setExportRange( PMS::PAGE_ALL );

    String outFile( "/tmp/plotMSMultiplePlotTypes");
    String outFile1( outFile + "_Scan1,2.jpg");
    //Because of the iteration plot at the end, we should have two more pages.
    String outFile2( outFile + "_Scan3,4,5,6_2.jpg");
    String outFile3( outFile + "_Scan7_3.jpg");
    tUtil::clearFile( outFile1 );
    tUtil::clearFile( outFile2 );
    tUtil::clearFile( outFile3 );
    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile + ".jpg");
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tMultiplePlotTypes:: Result of save="<<ok<<endl;

	ok = tUtil::checkFile( outFile1, 210000, 224000, -1 );
	cout << "tMultiplePlotTypes:: Result of first save file check="<<ok<<endl;

	ok = tUtil::checkFile( outFile2, 210000, 225000, -1 );
	cout << "tMultiplePlotTypes:: Result of second save file check="<<ok<<endl;

	ok = tUtil::checkFile( outFile3, 54000, 58000, -1 );
	cout << "tMultiplePlotTypes:: Result of third save file check="<<ok<<endl;

	return tUtil::exitMain( false );
}

