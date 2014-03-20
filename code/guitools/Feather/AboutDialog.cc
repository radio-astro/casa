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
#include <guitools/Feather/AboutDialog.qo.h>
#include <QDebug>
#include <stdcasa/version.h>
#include <sstream>

namespace casa {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	ui.aboutLabel->setText( getDescription());
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(close()));
}

QString AboutDialog::getDescription() const {
	std::stringstream ss;
	ss<<"Feather is a tool for combining two images with different spatial resolutions using their Fourier transforms.\n";
	ss<<"Typically one image is a high-resolution (interferometric) image and the other is a lower resolution image.\n";
	ss<<"Feather is part of the CASA (Common Astronomy Software Applications) package.\n\n";

	// CASA info
	ss << "CASA Version:  ";
	VersionInfo::report( ss );
	ss << "\n\n";

	QString aboutStr( ss.str().c_str() );
	return aboutStr;
}

AboutDialog::~AboutDialog()
{

}
}
