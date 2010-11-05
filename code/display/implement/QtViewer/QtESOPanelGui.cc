//# QtESOPanelGui.cc:	Prototype QObject for a 3D Viewer object
//# Copyright (C) 2005,2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.	See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#				Internet email: aips2-request@nrao.edu.
//#				Postal address: AIPS++ Project Office
//#			                    National Radio Astronomy Observatory
//#			                    520 Edgemont Road
//#			                    Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casa/iostream.h>

#include <display/QtViewer/QtESOPanelGui.qo.h>
#include <display/QtViewer/QtHeaderInfo.qo.h>
//#include <display/DisplayDatas/DisplayData.h>
//#include <display/QtViewer/QtViewer.qo.h>
//#include <display/QtViewer/QtDisplayPanelGui.qo.h>
//#include <display/QtViewer/QtDisplayData.qo.h>
//#include <display/QtViewer/QtMouseToolBar.qo.h>
//#include <coordinates/Coordinates/CoordinateSystem.h>
//#include <lattices/Lattices/ArrayLattice.h>
//#include <lattices/Lattices/LatticeExpr.h>
//#include <lattices/Lattices/LCBox.h>
//#include <images/Images/PagedImage.h>
//#include <images/Images/SubImage.h>
//#include <images/Regions/ImageRegion.h>
//#include <images/Regions/RegionManager.h>
//#include <images/Regions/WCBox.h>
//#include <images/Regions/WCIntersection.h>
//#include <casa/Arrays/Vector.h>
//#include <casa/Arrays/Array.h>
//#include <casa/Arrays/ArrayMath.h>
//#include <casa/iostream.h>
//#include <casa/fstream.h>
//#include <casa/sstream.h>
namespace casa {

    QtESOPanelGui::QtESOPanelGui( QtViewer *v, QWidget *parent ) : QtDisplayPanelGui( v, parent)
    {

    // set a new display title
    setWindowTitle("ESO 3D Display Panel");


    shwDtaInf_     = tlMenu_->addAction("Display Metadata");

    connect(shwDtaInf_,  SIGNAL(triggered()),  SLOT(showDataInfo()));

    }

    QtESOPanelGui::~QtESOPanelGui() { }

    void QtESOPanelGui::showDataInfo(){
    	QtHeaderInfo* hinfo = new QtHeaderInfo();
    	hinfo->show();
    	cerr << "Puuups!\n";
    }
} //# NAMESPACE CASA - END
