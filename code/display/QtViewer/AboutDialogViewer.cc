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

#include <display/QtViewer/AboutDialogViewer.qo.h>
#include <QDebug>
#include <stdcasa/version.h>
#include <sstream>

namespace casa {

AboutDialogViewer::AboutDialogViewer(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	ui.aboutLabel->setText( getDescription());
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(close()));
}

QString AboutDialogViewer::getDescription() const {
	std::stringstream ss;

	// CASA info
	ss << "CASA Version:  ";
	VersionInfo::report( ss );
	ss << "\n\n";



	ss<<"The Viewer is a tool for visualizing images and measurement sets in raster, contour, vector, or marker form.\n";
	ss<<"Images can be blinked, and movies are available for spectral-line image cubes. A variety of image analysis tools\n";
	ss<<"such as histograms, moment calculations, two-dimensional fitting, interactive position-velocity diagrams, \n";
	ss<<"and spatial profiling are also available in the viewer.\n\n";
	ss<<"The Viewer is part of the CASA (Common Astronomy Software Applications) package.\n\n";



	QString aboutStr( ss.str().c_str() );
	return aboutStr;
}

AboutDialogViewer::~AboutDialogViewer()
{

}
}
