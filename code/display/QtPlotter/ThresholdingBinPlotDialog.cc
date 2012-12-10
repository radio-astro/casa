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
    : QDialog(parent){
	ui.setupUi(this);
	setWindowTitle( "Graphical Collapse/Moments Threshold Specification");

	//Add the plot widget to the dialog
	QHBoxLayout* layout = new QHBoxLayout(ui.plotWidgetHolder);
	plotWidget = new BinPlotWidget( false, true, false, this );
	plotWidget->setPlotMode( 1 );
	layout->addWidget( plotWidget );
	ui.plotWidgetHolder->setLayout( layout );

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

void ThresholdingBinPlotDialog::keyPressEvent( QKeyEvent* event ){
	int keyCode = event->key();
	//This was written here because pressing a return on a line edit inside
	//the dialog was closing the dialog.
	if ( keyCode != Qt::Key_Return ){
		QDialog::keyPressEvent( event );
	}
}


void ThresholdingBinPlotDialog::setImage( ImageInterface<Float>* img ){
	plotWidget->setImage( img );
}

pair<double,double> ThresholdingBinPlotDialog::getInterval() const {
	return plotWidget->getMinMaxValues();
}

void ThresholdingBinPlotDialog::setInterval( double minValue, double maxValue ){
	plotWidget->setMinMaxValues( minValue, maxValue );
}

ThresholdingBinPlotDialog::~ThresholdingBinPlotDialog(){
}

}
