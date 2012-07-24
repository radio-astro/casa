#include "SpecFitPlotDialog.qo.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

namespace casa {

SpecFitPlotDialog::SpecFitPlotDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}
void SpecFitPlotDialog::setData( Vector<Float> xValues, Vector<Float> yValues ){
	toDoubleVectors( xValues, yValues, xVals, yVals );

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

void SpecFitPlotDialog::addPlot( const Vector<Float>& xValues,
		const Vector<Float>& yValues, QColor /*color*/ ){


		QwtPlot* plot = new QwtPlot( this );

		QwtPlotCurve* curve = new QwtPlotCurve();
		QVector<double> xVals;
		QVector<double> yVals;
		toDoubleVectors( xValues, yValues, xVals, yVals );
		curve->setData( xVals, yVals );
		curve->attach( plot );
		plots.append( plot );
}

SpecFitPlotDialog::~SpecFitPlotDialog()
{

}
}
