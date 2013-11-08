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
#ifndef GAUSSIANESTIMATEWIDGET_QO_H
#define GAUSSIANESTIMATEWIDGET_QO_H

#include <QWidget>
#include <casa/Arrays/Vector.h>
#include <display/QtPlotter/GaussianEstimateWidget.ui.h>
#include <display/QtPlotter/SpecFitGaussian.h>
#include <display/QtPlotter/conversion/Converter.h>

class QwtPlot;
class QwtPlotCurve;

namespace casa {

	class MolecularLine;

	class GaussianEstimateWidget : public QWidget {
		Q_OBJECT

	public:
		static void setEstimateColor( QColor estimateColor );
		GaussianEstimateWidget(QWidget *parent = 0);
		void setCurveData(const Vector<float>& xValues, const Vector<float>& yValues);
		void setCurveColor( QColor color );
		void setTitle( const QString& titleStr );
		void setRangeX( Float xValue, Float yValue );
		void setRangeY( Float xValue, Float yValue );
		void setDisplayYUnits( const QString& units );
		void molecularLineChanged( float peak, float center, const QString& label,
		                           const QString& chemicalName, const QString& resolvedQNs, const QString& frequencyUnits );
		SpecFitGaussian getEstimate();
		void setEstimate( const SpecFitGaussian& estimate );
		void unitsChanged( const QString& oldUnits, const QString& newUnits);
		void setSliderValueFWHM( float value );
		void updateUIBasedOnEstimate();
		void clearMolecularLines();
		~GaussianEstimateWidget();

	signals:
		void coordinatedValuesChanged(float);

	private slots:
		void peakSliderChanged( int value );
		void centerSliderChanged( int value );
		void fwhmSliderChanged( int value );
		void peakTextChanged();
		void centerTextChanged();
		void fwhmTextChanged();
		void peakFixedChanged( bool fixed );
		void centerFixedChanged( bool fixed );
		void fwhmFixedChanged( bool fixed );

	private:
		float scale( int value, Float min, Float max ) const;
		float scaleY( int value ) const;
		float scaleX( int value ) const;
		int reverseScale( float value, Float min, Float max ) const;
		int reverseScaleY( float value ) const;
		int reverseScaleX( float value ) const;
		float getFwhmRange() const;
		void peakChanged( float value );
		void centerChanged( float value );
		void fwhmChanged( float value );
		void updateFit();
		void copyVectors( const Vector<float>& values, QVector<double>& vals, bool reverseOrder );
		void copyVectors( const Vector<float>& sourceValues, Vector<float>& destinationValues, bool reverseOrder );
		float adjustValue( float val ) const;
		void setSliderValuePeak( float value );
		void setSliderValueCenter( float value );
		float reasonableFWHM( float value ) const;
		float reasonableCenter( float value ) const;
		float reasonablePeak( float value ) const;

		QwtPlotCurve* initCurve( QColor color );
		void clearCurve( QwtPlotCurve*& curve );

		QwtPlot* plot;
		QwtPlotCurve* curve;
		QwtPlotCurve* fitCurve;
		Ui::GaussianEstimateWidgetClass ui;
		SpecFitGaussian gaussianEstimate;
		Vector<float> xValues;
		Vector<float> yValues;
		Float minX;
		Float maxX;
		Float minY;
		Float maxY;
		QColor curveColor;
		static QColor fitCurveColor;
		QMap<QString,MolecularLine*> molecularLineMap;
	};
}

#endif // GAUSSIANESTIMATEWIDGET_QO_H
