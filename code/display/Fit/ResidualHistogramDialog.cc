//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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
#include "ResidualHistogramDialog.qo.h"
#include <guitools/Histogram/BinPlotWidget.qo.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>

namespace casa {

	ResidualHistogramDialog::ResidualHistogramDialog(QWidget *parent)
		: QDialog(parent), residualImage( ) {
		ui.setupUi(this);
		setWindowTitle( "Fit 2D Residual Image Histogram");

		//Add the plot widget to the dialog
		QHBoxLayout* layout = new QHBoxLayout(ui.histogramHolder);
		plotWidget = new BinPlotWidget( false, false, false, this );
		plotWidget->setPlotMode( 1 );
		layout->addWidget( plotWidget );
		ui.histogramHolder->setLayout( layout );

		connect( ui.closeButton, SIGNAL(clicked()), this, SLOT(close()));

	}
	bool ResidualHistogramDialog::setImage( const String& imagePath ) {
		bool success = true;
		residualImage = ImageTask::shCImFloat();

		try {
			residualImage.reset(new PagedImage<Float> (imagePath));
			plotWidget->setImage( residualImage );
		} catch( AipsError& error ) {
			success = false;

		}
		return success;
	}

	ResidualHistogramDialog::~ResidualHistogramDialog() {
	}
}
