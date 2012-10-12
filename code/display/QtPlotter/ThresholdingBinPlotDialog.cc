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

#include <images/Images/ImageHistograms.h>
#include <images/Images/ImageInterface.h>
#include <display/QtPlotter/Util.h>
#include <QDebug>
#include <qwt_plot_curve.h>

namespace casa {

ThresholdingBinPlotDialog::ThresholdingBinPlotDialog(QWidget *parent)
    : QDialog(parent), histogramMaker( NULL ),  binPlot( this )
{
	ui.setupUi(this);

	//Add the plot to the dialog
	QHBoxLayout* layout = new QHBoxLayout(ui.plotWidget);
	layout->addWidget( &binPlot );
	ui.plotWidget->setLayout( layout );

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void ThresholdingBinPlotDialog::setImage( ImageInterface<Float>* img ){
	if ( img != NULL ){
		//TODO:  Logging when we construct this?
		if ( histogramMaker == NULL ){
			histogramMaker = new ImageHistograms<Float>( *img );
		}
		else {
			histogramMaker->setNewImage( *img );
		}
		Array<Float> values;
		Array<Float> counts;
		bool success = histogramMaker->getHistograms( values, counts );
		if ( success ){
			//put the data into the qwt plot
			qDebug() << "Number of points in histogram is " << values.size();
			vector<float> xVector;
			vector<float> yVector;
			values.tovector( xVector );
			counts.tovector( yVector );
			for ( int i = 0; i < static_cast<int>(xVector.size()); i++ ){
				QVector<double> xValues(2);
				QVector<double> yValues(2);
				xValues[0] = xVector[i];
				xValues[1] = xVector[i];
				yValues[0] = 0;
				yValues[1] = yVector[i];
				QwtPlotCurve* curve  = new QwtPlotCurve();
				curve->setData( xValues, yValues );
				curve->attach(&binPlot);
			}
			binPlot.replot();
		}
		else {
			QString msg( "Could not make a histogram from the image.");
			Util::showUserMessage( msg, this );
		}
	}
}

ThresholdingBinPlotDialog::~ThresholdingBinPlotDialog()
{
	delete histogramMaker;

}
}
