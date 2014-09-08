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
 * Tests whether we can overplot field and scan for the test data with a legend
 * showing the two sets of data.
 */
int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tLegend using data from "<<dataPath.c_str()<<endl;

    // Set up plotms object.
    PlotMSApp app(false, false);


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

     //Turn on the legend.
     PMS_PP_Canvas* canvasParams = plotParams.typedGroup<PMS_PP_Canvas>();
     if(canvasParams == NULL) {
    	 plotParams.setGroup<PMS_PP_Canvas>();
         canvasParams = plotParams.typedGroup<PMS_PP_Canvas>();
     }
     canvasParams->showLegend(true);
     qDebug() << "tLegend setShowLegend";
     const PlotCanvas::LegendPosition position = PlotCanvas::INT_URIGHT;
     canvasParams->setLegendPosition( position );
     qDebug() << "tLegend setLegendPosition";

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

    String outFile( "/tmp/plotLegendTest.jpg");
    tUtil::clearFile( outFile );
    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tLegend:: Result of save="<<ok<<endl;
    
	ok = tUtil::checkFile( outFile, 70000, 75000, -1 );
	cout << "tLegend:: Result of save file check="<<ok<<endl;
	return tUtil::exitMain( false );
}

