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


bool exportPlot(PlotMSApp& app, String outFile, PlotExportFormat::Type type,
                String strType, int minSize, int maxSize ){
	tUtil::clearFile( outFile+"_Scan1,2,3,4" );
	tUtil::clearFile( outFile+"_Scan5,6,7_2" );

	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;

	bool ok = app.save(format);
	cout << "tExportRange:: "<<strType.c_str()<<" Result of save="<<ok<<endl;
	bool okOutput1 = tUtil::checkFile( outFile+"_Scan1,2,3,4", minSize, maxSize, -1 );
	cout << "tExportRange:: "<<strType.c_str()<<" Result of first save file check=" << okOutput1 << endl;
    bool okOutput2 = tUtil::checkFile( outFile+"_Scan5,6,7_2", minSize, maxSize, -1 );
	cout << "tExportRange:: "<<strType.c_str()<<" Result of second save file check=" << okOutput2 << endl;
    bool test = ok && okOutput1 && okOutput2;

    return test;
}

bool exportAsJPG( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangeJPGTest");
    PlotExportFormat::Type type = PlotExportFormat::JPG;
    return exportPlot( app, outFile, type, "JPG", 100000, 130000);
}

bool exportAsPNG( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangePNGTest");
    PlotExportFormat::Type type = PlotExportFormat::PNG;
    return exportPlot(  app, outFile, type, "PNG", 30000, 35000 );

}

bool exportAsPS( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangePSTest");
    PlotExportFormat::Type type = PlotExportFormat::PS;
    return exportPlot(  app, outFile, type, "PS", 800000, 1150000);

}

bool exportAsPDF( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangePDFTest");
    PlotExportFormat::Type type = PlotExportFormat::PDF;
    return exportPlot(  app, outFile, type, "PDF", 65000, 90000);

}

bool exportAsText( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangeTextTest");
    PlotExportFormat::Type type = PlotExportFormat::TEXT;
    return exportPlot(  app, outFile, type, "TEXT", 20000, 30000 );

}


/**
 * Tests whether an iteration plot, with two pages, can be exported
 * in a single pass.
 */

int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tExportRange using data from "<<dataPath.c_str()<<endl;

    // Set up plotms object.
    PlotMSApp app(false, false);

    //Make a 2x2 grid
    PlotMSParameters& overallParams = app.getParameters();
    overallParams.setRowCount( 2 );
    overallParams.setColCount( 2 );

    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSPlot::makeParameters(&app);

    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );

    //We are going to iterate over scan, using a shared axis and global
    //scale with a 2 x 2 grid for the iteration.
    PMS_PP_Iteration* iterationParams = plotParams.typedGroup<PMS_PP_Iteration>();
    if ( iterationParams == NULL ){
    	plotParams.setGroup<PMS_PP_Iteration>();
    	iterationParams = plotParams.typedGroup<PMS_PP_Iteration>();
    }
    iterationParams->setIterationAxis( PMS::SCAN );
    iterationParams->setGlobalScaleX( true );
    iterationParams->setGlobalScaleY( true );

    iterationParams->setCommonAxisX( true );
    iterationParams->setCommonAxisY( true );

    //We want to print all (2) pages in the output.
    PlotMSExportParam& exportParams = app.getExportParameters();
    exportParams.setExportRange( PMS::PAGE_ALL );

    //Make the plot.
    app.addOverPlot( &plotParams );

    bool jpg_test = exportAsJPG( app );
    bool png_test = exportAsPNG( app );

    //Too much data to export as text.
    //exportAsText( app );
    bool pdf_test = exportAsPDF( app );
    bool ps_test = exportAsPS( app );
    bool test = jpg_test && png_test && pdf_test && ps_test;

    bool checkGui = tUtil::exitMain( false );
    return !(test && checkGui);
}

