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
//#include <qwt_plot_histogram.h>

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
			/*qDebug() << "Number of points in histogram is " << values.size();
			vector<Float> valuesVec;
			vector<Float> countsVec;
			values.tovector( valuesVec );
			counts.tovector( countsVec );
			for ( int i = 0; i < values.size(); i++ ){

				qDebug() << "value="<<valuesVec[i]<<" count="<<countsVec[i];
			}
			QwtPlotHistogram *histogram = new QwtPlotHistogram();
			histogram->setStyle( QwtPlotHistogram::Columns);
			histogram->setPen( QPen( Qt::black));
			histogram->setBrush( QBrush( Qt::gray ));
			QwtArray<QwtIntervalSample> samples( values.size());
			for ( int i = 0; i < samples.size(); i++ ){
				samples[i].interval= QwtFloatInterval( valuesVec[i], valuesVec[i]);
				samples[i].value = countsVec[i];
			}
			histogram->setData( QwtIntervalSeriesData( samples ));
			histogram->attach( &binPlot );*/
			QString msg( "Need to migrate to qwt6 for this to work");
			Util::showUserMessage( msg, this );
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
