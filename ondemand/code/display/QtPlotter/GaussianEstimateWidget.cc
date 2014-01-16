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
#include "GaussianEstimateWidget.qo.h"
#include <display/QtPlotter/GaussianEstimateWidget.qo.h>
#include <display/QtPlotter/MolecularLine.h>
#include <QVector>
#include <QDebug>
#include <QDoubleValidator>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>

namespace casa {

	QColor GaussianEstimateWidget::fitCurveColor = "#CA5F00";
	void GaussianEstimateWidget::setEstimateColor( QColor estimateColor ) {
		fitCurveColor = estimateColor;
	}


	GaussianEstimateWidget::GaussianEstimateWidget(QWidget *parent)
		: QWidget(parent),  curve( NULL ), fitCurve( NULL ),
		  gaussianEstimate( 0, 0, 0, 0 ) {

		ui.setupUi(this);

		//Initialize the plot
		plot = new QwtPlot( ui.canvasHolder );
		QHBoxLayout* layout = new QHBoxLayout( ui.canvasHolder );
		layout->addWidget( plot );
		ui.canvasHolder->setLayout( layout );
		QwtPlotGrid* plotGrid = new QwtPlotGrid();
		plotGrid->attach( plot );
		plot->setCanvasBackground( Qt::white );

		//Initialize the text fields
		QDoubleValidator* validator = new QDoubleValidator( this );
		QDoubleValidator* posValidator = new QDoubleValidator( this );
		posValidator -> setBottom( 0 );
		ui.centerLineEdit->setValidator( posValidator );
		ui.fwhmLineEdit->setValidator( posValidator );
		ui.peakLineEdit->setValidator( validator );
		ui.centerLineEdit->setText( QString::number(0));
		ui.fwhmLineEdit->setText( QString::number(0));
		ui.peakLineEdit->setText( QString::number(0));
		connect( ui.centerLineEdit, SIGNAL(editingFinished()), this, SLOT(centerTextChanged()));
		connect( ui.fwhmLineEdit, SIGNAL(editingFinished()), this, SLOT(fwhmTextChanged()));
		connect( ui.peakLineEdit, SIGNAL(editingFinished()), this, SLOT(peakTextChanged()));

		//Initialize the sliders
		connect( ui.peakSlider, SIGNAL( valueChanged(int)), this, SLOT(peakSliderChanged(int)));
		connect( ui.centerSlider, SIGNAL( valueChanged(int)), this, SLOT(centerSliderChanged(int)));
		connect( ui.fwhmSlider, SIGNAL( valueChanged(int)), this, SLOT(fwhmSliderChanged(int)));

		//Initialize the "fixed" check boxes
		connect( ui.fixPeakCheckBox, SIGNAL(toggled(bool)), this, SLOT(peakFixedChanged(bool)));
		connect( ui.fixCenterCheckBox, SIGNAL(toggled(bool)), this, SLOT(centerFixedChanged(bool)));
		connect( ui.fixFWHMCheckBox, SIGNAL(toggled(bool)), this, SLOT(fwhmFixedChanged(bool)));
	}

	QwtPlotCurve* GaussianEstimateWidget::initCurve( QColor color ) {
		QwtPlotCurve* curve = new QwtPlotCurve();
		QPen pen( color );
		curve->setPen( color );
		curve->attach( plot );
		return curve;
	}

	void GaussianEstimateWidget::setTitle( const QString& titleStr ) {
		ui.estimateLabel->setText( titleStr );
	}

//------------------------------------------------------------------------------------
//                           Data
//------------------------------------------------------------------------------------

	void GaussianEstimateWidget::updateFit() {

		Vector<float> xVals(30);
		float sigma = gaussianEstimate.getFWHM();
		float center = gaussianEstimate.getCenter();
		float start = center - 2 * sigma;
		float dx = sigma / 5;
		for ( int i = 0; i < 5; i++ ) {
			float val = start + i * dx;
			xVals[i] = adjustValue( val );
		}
		dx = sigma / 10;
		start = center - sigma;
		for ( int i = 5; i < 25; i++ ) {
			float val = start + i * dx;
			xVals[i] = adjustValue( val );
		}
		dx = sigma / 5;
		start = center + sigma;
		for ( int i = 25; i < 30; i++ ) {
			float val = start + i * dx;
			xVals[i] = adjustValue( val );
		}
		gaussianEstimate.evaluate( xVals );
		Vector<float> yValues = gaussianEstimate.getYValues();
		QVector<double> yVals;
		copyVectors( yValues, yVals, false );
		QVector<double> copiedXVals;
		copyVectors( xVals, copiedXVals, false );

		if ( fitCurve == NULL ) {
			fitCurve = new QwtPlotCurve();
			fitCurve -> attach( plot );
		}
		QPen fitCurvePen( fitCurveColor );
		fitCurve -> setPen( fitCurvePen );
		fitCurve->setData( copiedXVals, yVals );
		plot->replot();
	}



	void GaussianEstimateWidget::setRangeX( Float min, Float max ) {
		minX = min;
		maxX = max;
	}

	void GaussianEstimateWidget::setRangeY( Float min, Float max ) {
		minY = min;
		maxY = max;
	}

	void GaussianEstimateWidget::setCurveData(const Vector<float>& xValues, const Vector<float>& yValues) {

		bool reverseOrder = false;
		int count = xValues.size();
		if ( count >= 2 ) {
			if ( xValues[0] > xValues[1] ) {
				reverseOrder = true;
			}
		}
		QVector<double> xVals;
		copyVectors( xValues, xVals, reverseOrder );
		copyVectors( xValues, this->xValues, reverseOrder );

		QVector<double> yVals;
		copyVectors( yValues, yVals, reverseOrder );
		copyVectors( yValues, this->yValues, reverseOrder );

		if ( curve == NULL ) {
			curve = new QwtPlotCurve();
			curve->attach( plot );
		}

		curve->setData( xVals, yVals );
		plot->replot();
	}

	void GaussianEstimateWidget::setCurveColor( QColor color ) {
		curveColor = color;
		QPen curvePen( color );
		curve->setPen( curvePen );
	}


	void GaussianEstimateWidget::setDisplayYUnits( const QString& units ) {
		plot->setAxisTitle( QwtPlot::yLeft, units );
	}

//---------------------------------------------------------------------
//                         Estimates
//---------------------------------------------------------------------



	void GaussianEstimateWidget::setEstimate( const SpecFitGaussian& estimate ) {
		gaussianEstimate = estimate;
		updateUIBasedOnEstimate();
	}

	void GaussianEstimateWidget::updateUIBasedOnEstimate() {
		ui.fixCenterCheckBox->setChecked( gaussianEstimate.isCenterFixed());
		ui.fixPeakCheckBox->setChecked( gaussianEstimate.isPeakFixed());
		ui.fixFWHMCheckBox->setChecked( gaussianEstimate.isFwhmFixed());

		float peak   = gaussianEstimate.getPeak();
		float center = gaussianEstimate.getCenter();
		float fwhm   = gaussianEstimate.getFWHM();

		peak   = reasonablePeak( peak );
		center = reasonableCenter( center );
		fwhm   = reasonableFWHM( fwhm );

		//Note: Normally setSliderValue peak will call peakChanged. However,
		//if we are just changing units, the position of the slider may not
		//change and generate a peakChanged events.  That is why we need both
		//calls here.  The second call, peakChanged will set a new value in
		//the text field and adjust the graph.
		setSliderValuePeak( peak );
		setSliderValueCenter( center );
		setSliderValueFWHM( fwhm );
		peakChanged( peak );
		centerChanged( center );
		fwhmChanged( fwhm );
	}

	void GaussianEstimateWidget::unitsChanged( const QString& oldUnits, const QString& newUnits, SpectralCoordinate& coord) {
		Converter* converter = Converter::getConverter( oldUnits, newUnits);
		double centerVal = gaussianEstimate.getCenter();
		double fwhm = gaussianEstimate.getFWHM();
		double fwhmPoint = centerVal - fwhm;

		centerVal = converter->convert( centerVal, coord);
		fwhmPoint = converter->convert( fwhmPoint, coord);
		gaussianEstimate.setCenter( static_cast<float>(centerVal) );
		gaussianEstimate.setFWHM( static_cast<float>(qAbs(centerVal - fwhmPoint)) );
		QList<QString> keys = molecularLineMap.keys();
		for ( int i = 0; i < keys.size(); i++ ) {
			MolecularLine* molecularLine = molecularLineMap[keys[i]];
			float center = molecularLine->getCenter();
			center = converter->convert( center, coord );
			molecularLine->setCenter( center );
		}
		updateUIBasedOnEstimate();
		delete converter;
	}

	SpecFitGaussian GaussianEstimateWidget::getEstimate() {
		return gaussianEstimate;
	}

//---------------------------------------------------------------------
//                           Peak
//---------------------------------------------------------------------

	void GaussianEstimateWidget::setSliderValuePeak( float value ) {
		int peakSliderValue = reverseScaleY( value );
		ui.peakSlider->setValue( peakSliderValue );
	}

	void GaussianEstimateWidget::peakFixedChanged( bool checked ) {
		gaussianEstimate.setPeakFixed( checked );
	}

	void GaussianEstimateWidget::peakTextChanged() {
		QString peakStr = ui.peakLineEdit->text();
		if ( peakStr.length() > 0 ) {
			float peakValue = static_cast<float> (peakStr.toDouble());
			int sliderValue = reverseScaleY( peakValue );
			ui.peakSlider->setValue( sliderValue );
		}
	}

	void GaussianEstimateWidget::peakSliderChanged( int value ) {
		float peakValue = scaleY( value );
		peakChanged( peakValue );
	}

	void GaussianEstimateWidget::peakChanged( float value ) {
		ui.peakLineEdit->setText( QString::number( value ));
		gaussianEstimate.setPeak( value );
		updateFit();
	}

	float GaussianEstimateWidget::reasonablePeak( float value ) const {
		float val = value;
		if ( val < minY || val > maxY ) {
			val = maxY;
		}
		return val;
	}


//----------------------------------------------------------------------
//                         Center
//----------------------------------------------------------------------

	void GaussianEstimateWidget::setSliderValueCenter( float value ) {
		int centerSliderValue = reverseScaleX( value );
		ui.centerSlider->setValue( centerSliderValue );
	}

	float GaussianEstimateWidget::reasonableCenter( float value ) const {
		float val = value;
		if ( val < minX || val > maxX ) {
			int maxYIndex = 0;
			for ( int i = 0; i < static_cast<int>(yValues.size()); i++ ) {
				if ( yValues[i] == maxY ) {
					maxYIndex = i;
					break;
				}
			}
			if ( maxYIndex < static_cast<int>(xValues.size()) ) {
				val = xValues[maxYIndex];
			}
		}
		return val;
	}

	void GaussianEstimateWidget::centerFixedChanged( bool checked ) {
		gaussianEstimate.setCenterFixed( checked );
	}

	void GaussianEstimateWidget::centerTextChanged() {
		QString centerStr = ui.centerLineEdit->text();
		if ( centerStr.length() > 0 ) {
			float centerValue = static_cast<float> (centerStr.toDouble());
			int sliderValue = reverseScaleX( centerValue );
			ui.centerSlider->setValue( sliderValue );
		}
	}

	void GaussianEstimateWidget::centerSliderChanged( int value ) {
		float centerValue = scaleX( value );
		centerChanged( centerValue );
	}

	void GaussianEstimateWidget::centerChanged( float value ) {
		ui.centerLineEdit->setText( QString::number( value ));
		gaussianEstimate.setCenter( value );
		updateFit();
	}




//-----------------------------------------------------------------------
//                              FWHM
//-----------------------------------------------------------------------

	void GaussianEstimateWidget::setSliderValueFWHM( float value ) {
		float range = getFwhmRange();
		int fwhmValue = reverseScale( value, 0, range );
		ui.fwhmSlider->setValue( fwhmValue );
	}

	float GaussianEstimateWidget::reasonableFWHM( float value ) const {
		float val = value;
		float maxFWHM = this->getFwhmRange();
		if ( val < 0 || val > maxFWHM ) {
			val = maxFWHM / 2;
		}
		return val;
	}

	float GaussianEstimateWidget::getFwhmRange() const {
		float fwhmRange = (maxX - minX) / 10;
		return fwhmRange;
	}

	void GaussianEstimateWidget::fwhmTextChanged() {
		QString fwhmStr = ui.fwhmLineEdit->text();
		if ( fwhmStr.length() > 0 ) {
			float fwhmValue = static_cast<float> (fwhmStr.toDouble());
			int sliderValue = reverseScale( fwhmValue, 0, getFwhmRange() );
			ui.fwhmSlider->setValue( sliderValue );
		}
	}

	void GaussianEstimateWidget::fwhmSliderChanged( int value ) {
		float fwhmValue = scale( value, 0, getFwhmRange() );
		fwhmChanged( fwhmValue );
	}

	void GaussianEstimateWidget::fwhmChanged( float value ) {
		ui.fwhmLineEdit->setText( QString::number( value ));
		gaussianEstimate.setFWHM( value );
		emit coordinatedValuesChanged( value );
		updateFit();
	}

	void GaussianEstimateWidget::fwhmFixedChanged( bool checked ) {
		gaussianEstimate.setFwhmFixed( checked );
	}

//--------------------------------------------------------------------
//                    Molecular Lines
//--------------------------------------------------------------------

	void GaussianEstimateWidget::molecularLineChanged( float peak, float center,
	        const QString& label, const QString& chemicalName, const QString& resolvedQNs,
	        const QString& frequencyUnits ) {

		//Update the sliders which should update everything else.
		setSliderValuePeak( peak );
		setSliderValueCenter( center );
		QString key = label + QString::number(center);
		if ( ! molecularLineMap.contains( key )) {
			MolecularLine* molecularLine = new MolecularLine(center, peak, label,
			        chemicalName, resolvedQNs, frequencyUnits, center );
			molecularLine->attach( plot );
			plot->replot();
			molecularLineMap.insert( key, molecularLine );
		}
	}

	void GaussianEstimateWidget::clearMolecularLines() {
		QList<QString> keys = molecularLineMap.keys();
		for ( int i = 0; i < keys.size(); i++ ) {
			MolecularLine* molecularLine = molecularLineMap[ keys[i]];
			molecularLineMap.remove( keys[i]);
			molecularLine->detach( );
			delete molecularLine;
		}
		plot->replot();
	}

//---------------------------------------------------------------------
//                    Scaling values
//---------------------------------------------------------------------

	float GaussianEstimateWidget::scaleY( int value ) const {
		return scale( value, minY, maxY );
	}

	int GaussianEstimateWidget::reverseScaleY( float value ) const {
		return reverseScale( value, minY, maxY );
	}

	float GaussianEstimateWidget::scaleX( int value ) const {
		return scale( value, minX, maxX );
	}

	int GaussianEstimateWidget::reverseScaleX( float value ) const {
		return reverseScale( value, minX, maxX );
	}

	float GaussianEstimateWidget::scale( int value, Float min, Float max ) const {
		return min + value * ( max - min ) / 100;
	}

	int GaussianEstimateWidget::reverseScale( float value, Float min, Float max ) const {
		int reversedValue = static_cast<int>(( value - min) * 100 / (max - min));
		return reversedValue;
	}

//--------------------------------------------------------------------------------
//                             Utility
//--------------------------------------------------------------------------------

	void GaussianEstimateWidget::copyVectors( const Vector<float>& values, QVector<double>& vals, bool reverseOrder ) {
		int count = static_cast<int>( values.size());
		for ( int i = 0; i < count; i++ ) {
			int counter = i;
			if ( reverseOrder ) {
				counter = count - 1 - i;
			}
			vals.append( values[counter]);
		}
	}

	void GaussianEstimateWidget::copyVectors( const Vector<float>& sourceValues,
	        Vector<float>& destinationValues, bool reverseOrder ) {
		int count = static_cast<int>( sourceValues.size());
		destinationValues.resize( count );
		for ( int i = 0; i < count; i++ ) {
			int counter = i;
			if ( reverseOrder ) {
				counter = count - 1 - i;
			}
			destinationValues[i] = sourceValues[counter];
		}
	}

	float GaussianEstimateWidget::adjustValue( float val ) const {
		float value = val;
		if ( value < minX ) {
			value = minX;
		} else if ( value > maxX ) {
			value = maxX;
		}
		return value;
	}

	void GaussianEstimateWidget::clearCurve( QwtPlotCurve*& curve ) {
		if ( curve != NULL ) {
			curve->detach();
			delete curve;
			curve = NULL;
		}
	}

	GaussianEstimateWidget::~GaussianEstimateWidget() {
		clearMolecularLines();
		clearCurve( curve );
		clearCurve( fitCurve );
		delete plot;
	}
}
