//# Copyright (C) 2005
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
#include "ThresholdingBinPlotDialog.qo.h"

#include <guitools/Histogram/BinPlotWidget.qo.h>
#include <images/Images/ImageInterface.h>
#include <QDebug>
#include <QKeyEvent>

namespace casa {

	ThresholdingBinPlotDialog::ThresholdingBinPlotDialog(QString /*yAxisUnits*/, QWidget *parent)
		: QMainWindow(parent) {
		ui.setupUi(this);
		setWindowTitle( "Graphical Collapse/Moments Threshold Specification");

		//Add the plot widget to the dialog
		QHBoxLayout* layout = new QHBoxLayout(ui.plotWidgetHolder);
		plotWidget = new BinPlotWidget( false, true, true, this );
		plotWidget->setPlotMode( 1 );
		layout->addWidget( plotWidget );
		ui.plotWidgetHolder->setLayout( layout );
		plotWidget->addZoomActions( true, ui.menuZoom );
		plotWidget->addDisplayActions( ui.menuDisplay, NULL );
		plotWidget->addPlotModeActions( ui.menuConfigure, NULL, NULL);

		connect( plotWidget, SIGNAL(postStatusMessage(const QString&)), this, SLOT(postStatusMessage(const QString&)));
		connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()));
	}

	void ThresholdingBinPlotDialog::postStatusMessage( const QString& warning ) {
		QStatusBar* statBar = statusBar();
		if ( statBar != NULL ) {
			statBar->showMessage( warning, 5000 );
		}
	}



	void ThresholdingBinPlotDialog::setImage( const std::tr1::shared_ptr<const ImageInterface<Float> > img) {
		plotWidget->setImage( img );
	}

	pair<double,double> ThresholdingBinPlotDialog::getInterval() const {
		return plotWidget->getMinMaxValues();
	}

	void ThresholdingBinPlotDialog::setInterval( double minValue, double maxValue ) {
		plotWidget->setMinMaxValues( minValue, maxValue );
	}

	void ThresholdingBinPlotDialog::accept() {
		emit accepted();
		close();
	}

	ThresholdingBinPlotDialog::~ThresholdingBinPlotDialog() {
	}

}
