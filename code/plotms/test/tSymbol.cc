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
#include <display/QtViewer/QtApp.h>

#include <iostream>
#include <msvis/MSVis/UtilJ.h>
#include <casa/namespace.h>
#include <QApplication>
#include <QDebug>

int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tSymbol using data from "<<dataPath.c_str()<<endl;

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

    PlotSymbolPtr ps = app.createSymbol ("diamond", 10, "00FF00", "mesh2", false );
      cout << "Created symbol type="<<ps->symbol()<<endl;
      PMS_PP_Display* ppdisp = plotParams.typedGroup<PMS_PP_Display>();
      if (ppdisp == NULL) {
          plotParams.setGroup<PMS_PP_Display>();
          ppdisp = plotParams.typedGroup<PMS_PP_Display>();
      }
      ppdisp->setUnflaggedSymbol(ps);


    app.addOverPlot( &plotParams );
    tUtil::updatePlot(&app);

    //Now save it and check the file size.
    String outFile( "/tmp/plotMSSymbolTest.jpg");
    tUtil::clearFile( outFile );
    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tSymbol:: Result of save="<<ok<<endl;
    
	ok = tUtil::checkFile( outFile, 190000, 205000, -1 );
	cout << "tSymbol:: Result of save file check="<<ok<<endl;

	return tUtil::exitMain( false );

}

