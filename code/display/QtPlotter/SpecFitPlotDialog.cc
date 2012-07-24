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
#include "SpecFitPlotDialog.qo.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

namespace casa {

SpecFitPlotDialog::SpecFitPlotDialog(QWidget *parent)
    : QDialog(parent), summaryPlot( NULL )
{
	ui.setupUi(this);

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void SpecFitPlotDialog::setFitCount( int count ){
	fitCount = count;
}

void SpecFitPlotDialog::setData( Vector<Float> xValues, Vector<Float> yValues ){
	toDoubleVectors( xValues, yValues, xVals, yVals );
	addPlots();
}

void SpecFitPlotDialog::toDoubleVectors( const Vector<Float>& sourceXValues, const Vector<Float>& sourceYValues,
		QVector<double>& destXValues, QVector<double>& destYValues ){
	destXValues.clear();
	destYValues.clear();
	int count = qMin( sourceXValues.size(), sourceYValues.size());
	for ( int i = 0; i < count; i++ ){
		destXValues.append( sourceXValues[i]);
		destYValues.append( sourceYValues[i]);
	}
}



void SpecFitPlotDialog::addPlots(){
	if ( summaryPlot == NULL ){
		summaryPlot = new QwtPlot( ui.summaryFitHolder );
		summaryPlot->setTitle( "Summation Fit");
		QwtPlotCurve* curve = new QwtPlotCurve();
		curve->setData( xVals, yVals );
		curve->attach( summaryPlot );
	}
}

SpecFitPlotDialog::~SpecFitPlotDialog()
{
	delete summaryPlot;
	while( ! plots.isEmpty()){
		QwtPlot* plot = plots.takeFirst();
		delete plot;
	}
}
}
