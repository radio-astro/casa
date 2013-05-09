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
#include "ColorTransferWidget.qo.h"
#include <display/QtViewer/InvisibleAxis.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>
#include <qwt_double_interval.h>
#include <QDebug>

namespace casa {

	ColorTransferWidget::ColorTransferWidget(QWidget *parent)
		: QWidget(parent), colorCurve(NULL), colorMap( NULL ) {

		ui.setupUi(this);

		colorScaleMin = 0;
		colorScaleMax = 100;

		plot = new QwtPlot( ui.plotHolderWidget );
		plot->setCanvasBackground( Qt::white );
		QHBoxLayout* layout = new QHBoxLayout();
		layout->addWidget( plot );
		ui.plotHolderWidget->setLayout( layout );
	}

	void ColorTransferWidget::setIntensities( std::vector<float>& intensityValues ) {
		int intensityCount = intensityValues.size();
		intensities.resize( intensityCount );
		for ( int i = 0; i < intensityCount; i++ ) {
			intensities[i] = intensityValues[i];
		}
	}

	void ColorTransferWidget::resetColorCurve() {
		//Add the color curve in.
		int count = colorLookups.size();
		if ( count > 0 ) {
			QVector<double> yVals( count );
			for( int i = 0; i < count; i++ ) {
				yVals[i] = colorLookups[i];
			}
			if ( colorCurve != NULL ) {
				colorCurve->detach();
				delete colorCurve;
				colorCurve = NULL;
			}
			colorCurve  = new QwtPlotCurve();
			colorCurve->setAxis( QwtPlot::xBottom, QwtPlot::yLeft);
			colorCurve->setData( intensities, yVals );
			QPen curvePen( Qt::blue );
			curvePen.setWidth( 2 );
			colorCurve->setPen(curvePen);
			colorCurve->attach(plot);
			plot->replot();
		}
	}

	void ColorTransferWidget::setColorLookups( const Vector<uInt>& lookups ) {
		int count = lookups.size();
		colorLookups.resize( count );
		for ( int i = 0; i < count; i++ ) {
			colorLookups[i] = lookups[i];
		}
		resetColorCurve();
	}


	void ColorTransferWidget::resetColorBar() {
		if ( colorMap != NULL ) {
			QwtDoubleInterval range( colorScaleMin, colorScaleMax);
			QwtScaleWidget* scale = plot->axisWidget( QwtPlot::yLeft );
			scale->setColorBarEnabled( true );
			scale->setColorMap( range, *colorMap );
			QwtScaleDraw* axisPaint = new InvisibleAxis();
			scale->setScaleDraw( axisPaint );
			plot->setAxisScale( QwtPlot::yLeft, range.minValue(), range.maxValue() );
		}
	}

	void ColorTransferWidget::setColorMap(QwtLinearColorMap* linearMap ) {
		colorMap = linearMap;
		resetColorBar();
		plot->replot();
	}

	ColorTransferWidget::~ColorTransferWidget() {
		delete colorCurve;
		delete colorMap;
	}
}
