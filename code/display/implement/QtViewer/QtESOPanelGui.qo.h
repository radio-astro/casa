//# QtESOPanelGui.qo.h:  interactive clean display panel
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
//# $Id$

#ifndef QtESOPANELGUI_H_
#define QtESOPANELGUI_H_

//#include <casa/aips.h>
//#include <casa/BasicSL/String.h>
//#include <casa/Containers/Record.h>
//#include <casa/Arrays/Vector.h>
//#include <coordinates/Coordinates/CoordinateSystem.h>
//#include <coordinates/Coordinates/DirectionCoordinate.h>

//#include <graphics/X11/X_enter.h>
//#  include <QObject>
//#  include <QGroupBox>
//#  include <QRadioButton>
//#  include <QPushButton>
//#  include <QHBoxLayout>
//#  include <QVBoxLayout>
//#  include <QLineEdit>
//#include <graphics/X11/X_exit.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>


namespace casa {

    //class QtViewer;
    //class QtDisplayPanel;
    //class QtDisplayData;
    //class ImageRegion;
    //class WorldCanvasHolder;

    // <synopsis>
    // Sandbox to test out some things towards te ESO 3D Viewer
    // </synopsis>
    class QtESOPanelGui: public QtDisplayPanelGui {

	Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
			//# implement/.../*.h files; also, makefile must include
			//# name of this file in 'mocs' section.
  
    public: 
 
	QtESOPanelGui( QtViewer *v, QWidget *parent=0 );
  
	~QtESOPanelGui();

	public slots:
	  virtual void showDataInfo();

    protected:
		QAction *shwDtaInf_;
};


} //# NAMESPACE CASA - END

#endif


