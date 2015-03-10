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

#include "ColorHistogram.qo.h"
#include <guitools/Histogram/BinPlotWidget.qo.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <display/DisplayCanvas/WCPowerScaleHandler.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/ColorHistogramScale.qo.h>
#include <display/QtViewer/ColorTransferWidget.qo.h>
#include <images/Images/ImageInterface.h>
#include <casa/Arrays/Array.h>
#include <QDoubleValidator>
#include <msvis/MSVis/UtilJ.h>
#include <limits>

namespace casa {

	ColorHistogram::ColorHistogram(QWidget *parent)
		: QMainWindow(parent), displayData( NULL ),
		  powerScaler( NULL), colorScale( NULL ),
		  COLOR_MAX(100), SLIDER_MIN(0), SLIDER_MAX(100) {

		ui.setupUi(this);
		setWindowTitle( "Image Color Mapping");

		maxIntensity = std::numeric_limits<double>::max();
		minIntensity = -1 * maxIntensity;

		SCALE_LIMIT = 5;
		int scaleLimitLower = -1 * SCALE_LIMIT;
		ui.powerCyclesMinLabel->setText( QString::number(scaleLimitLower)+"<<" );
		QPalette minPalette = ui.powerCyclesMinLabel->palette();
		minPalette.setColor( QPalette::Foreground, Qt::red );
		ui.powerCyclesMinLabel->setPalette( minPalette );
		ui.powerCyclesMaxLabel->setText( "<<"+QString::number(SCALE_LIMIT));
		QPalette maxPalette = ui.powerCyclesMaxLabel->palette();
		maxPalette.setColor( QPalette::Foreground, Qt::red );
		ui.powerCyclesMaxLabel->setPalette( maxPalette );
		QDoubleValidator* validator = new QDoubleValidator( scaleLimitLower, SCALE_LIMIT, 5, this );
		ui.powerCyclesLineEdit->setValidator( validator );
		ui.powerCyclesSlider->setMinimum( SLIDER_MIN );
		ui.powerCyclesSlider->setMaximum( SLIDER_MAX );
		ui.powerCyclesSlider->setTickInterval( 1 );
		connect( ui.powerCyclesSlider, SIGNAL( valueChanged(int)), this, SLOT( powerCyclesChangedSlider(int)));
		connect( ui.powerCyclesLineEdit, SIGNAL( textChanged(const QString&)), this, SLOT( powerCyclesChangedLineEdit(const QString&)));

		//Colored histogram
		ui.coloredHistogramCheckBox->setChecked( true );
		connect( ui.coloredHistogramCheckBox, SIGNAL(toggled(bool)), this, SLOT(histogramColorModeChanged(bool)));

		//Invert the color map
		connect( ui.invertMapCheckBox, SIGNAL(stateChanged(int)), this, SLOT(invertColorMap(int)));

		//Log Scale
		logScale = 0;
		/*ui.logScaleSlider->setMinimum( 0 );
		ui.logScaleSlider->setMaximum( COLOR_MAX );
		ui.logScaleSlider->setValue( logScale );
		connect( ui.logScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(logScaleChanged(int)));
		*/
		//Histogram plot
		histogram = new BinPlotWidget( false, true, false, this );
		histogram->setPlotMode( FootPrintWidget::IMAGE_MODE );
		histogram->setDisplayAxisTitles( true );
		histogram->setDisplayPlotTitle( true );
		histogram->setColorScaleMax( COLOR_MAX );
		QHBoxLayout* hBox = new QHBoxLayout();
		hBox->addWidget( histogram );
		ui.histogramHolder->setLayout( hBox );
		histogram->addZoomActions( true, ui.menuZoom );
		histogram->addDisplayActions( ui.menuDisplay, NULL );
		connect( histogram, SIGNAL(rangeChanged()), this, SLOT(resetIntensityRange()));

		//Color transfer plot
		colorTransferWidget = new ColorTransferWidget( this );
		QHBoxLayout* hBox2 = new QHBoxLayout(ui.transferWidgetHolder);
		hBox2->addWidget( colorTransferWidget );
		ui.transferWidgetHolder->setLayout( hBox2 );

		powerScaler = new WCPowerScaleHandler();
		powerScaler->setRangeMax( COLOR_MAX );

		connect( ui.okButton, SIGNAL(clicked()), this, SLOT(acceptRange()));
		connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelRange()));
	}

	void ColorHistogram::powerCyclesChangedLineEdit( const QString& str ) {
		bool validStr = false;
		float value = str.toFloat(&validStr );
		if ( validStr ) {
			resetPowerCycles( value );
		}
	}

	void ColorHistogram::powerCyclesChangedSlider(int value ) {
		casa::utilj::ThreadTimes t1;
		float valueDistance = value - SLIDER_MIN;
		float valueRange = SLIDER_MAX - SLIDER_MIN;
		float powerRange = SCALE_LIMIT * 2;
		float powerMin = -1 * SCALE_LIMIT;
		float cycleValue = powerRange * valueDistance / valueRange+powerMin;
		resetPowerCycles( cycleValue);
		casa::utilj::ThreadTimes t2;
		casa::utilj::DeltaThreadTimes dt = t2 - t1;
	}

	void ColorHistogram::resetIntensityRange() {
		std::pair<double,double> minMaxValues = histogram->getMinMaxValues();
		if ( minIntensity != minMaxValues.first || maxIntensity != minMaxValues.second ) {
			minIntensity = minMaxValues.first;
			maxIntensity = minMaxValues.second;
			resetColorLookups();
		}
	}

	void ColorHistogram::acceptRange() {
		if ( displayData != NULL ) {
			std::pair<double,double> minMaxPair = histogram->getMinMaxValues();
			float minValue = static_cast<float>(minMaxPair.first);
			float maxValue = static_cast<float>(minMaxPair.second );
			QString powerCycleText = ui.powerCyclesLineEdit->text();
			float powerCycles = powerCycleText.toFloat();
			displayData->setHistogramColorMapping( minValue, maxValue, powerCycles );
			displayData->setHistogramColorProperties( ui.invertMapCheckBox->isChecked(), /*ui.logScaleSlider->value()*/0 );

		}
	}

	void ColorHistogram::histogramColorModeChanged(bool useColors) {
		histogram->setMultiColored( useColors );
	}


	void ColorHistogram::cancelRange() {
		close();
	}

	void ColorHistogram::setDisplayData( QtDisplayData* dd ) {
		if ( displayData != NULL ) {
			disconnect( displayData, SIGNAL(colorBarChange()), this, SLOT(colorsChanged()));
		}
		displayData = dd;
		if ( displayData != NULL ) {
			connect( displayData, SIGNAL(colorBarChange()), this, SLOT( colorsChanged()));
			SHARED_PTR<ImageInterface<float> > img = displayData->imageInterface();
			histogram->setImage( img );
			colorsChanged();
		}
	}

	void ColorHistogram::invertColorMap( int /*invert*/ ) {
		updateColorMap( true);
		resetColorLookups();
	}

	void ColorHistogram::logScaleChanged ( int logScaleAmount ){
		this->logScale = logScaleAmount;
		invertColorMap( true );
	}

	void ColorHistogram::resetColorLookups() {
		//casa::utilj::ThreadTimes t1;

		//Have to change the colors used to draw the
		//histogram.
		std::vector<float> intensities = histogram->getXValues();

		//Get intensities that match the range of the color
		//Scale.
		Vector<uInt> colorValues = computeScaledIntensities( intensities );
		histogram->setColorLookups( colorValues );
		//casa::utilj::ThreadTimes t2;
		//casa::utilj::DeltaThreadTimes dt = t2 - t1;

		//For the transfer function, we only want to use intensities within
		//a given range.
		std::vector<float> colorIntensities;
		std::vector<float>::iterator intensityIter = intensities.begin();
		while ( intensityIter != intensities.end()) {
			if ( minIntensity <= (*intensityIter) && (*intensityIter)<= maxIntensity ) {
				colorIntensities.push_back( *intensityIter );
			}
			intensityIter++;
		}

		int colorValueCount = colorValues.size();
		int colorIntensityCount = colorIntensities.size();
		Vector<uInt> colorLookups( colorIntensityCount );
		int j = 0;
		for ( int i = 0; i < colorValueCount; i++ ) {
			if ( minIntensity <= intensities[i] && intensities[i]<= maxIntensity ) {
				colorLookups[j] = colorValues[i];
				j++;
			}
		}
		colorTransferWidget->setIntensities( colorIntensities );
		colorTransferWidget->setColorLookups( colorLookups );

		//casa::utilj::ThreadTimes t3;
		//casa::utilj::DeltaThreadTimes dt2 = t3 - t2;
		//qDebug() << "resetcolor lookups color transfer elapsed="<<dt2.elapsed()<<" cpu="<<dt2.cpu();

		//casa::utilj::DeltaThreadTimes dt3 = t3 - t1;
		//qDebug() << "resetcolor lookups  total elapsed="<<dt3.elapsed()<<" cpu="<<dt3.cpu();
	}

	Vector<uInt> ColorHistogram::computeScaledIntensities(const std::vector<float>& intensities ) {
		//casa::utilj::ThreadTimes t1;
		//We have to scale the intensities to the interval [0,1].
		int intensityCount = intensities.size();
		Vector<uInt> colorValues;
		if ( intensityCount > 0 ) {
			Vector<Float> scaledIntensities( intensityCount );
			for ( int i = 0; i < intensityCount; i++ ) {
				scaledIntensities[i] = intensities[i];
			}
			powerScaler->setDomainMinMax( minIntensity, maxIntensity );
			colorValues.resize( intensityCount );
			(*powerScaler)( colorValues, scaledIntensities );
		}
		//casa::utilj::ThreadTimes t3;
		//casa::utilj::DeltaThreadTimes dt2 = t3 - t1;
		//qDebug() << "computerScaledIntensities elapsed="<<dt2.elapsed()<<" cpu="<<dt2.cpu();
		return colorValues;
	}

	void ColorHistogram::updateColorMap(bool invertChanged) {
		//Update the color map
		String colorMapName = displayData->getColormap();
		bool reset = false;
		bool invertedMap = ui.invertMapCheckBox->isChecked();

		if ( colorScale != NULL ) {
			String existingMapName = colorScale->getColorMapName();
			if (existingMapName != colorMapName || invertChanged ) {
				delete colorScale;
				reset = true;
				colorScale = new ColorHistogramScale( invertedMap, logScale );
			}
		} else {
			reset = true;
			colorScale = new ColorHistogramScale( invertedMap, logScale );
		}

		if ( reset ) {
			colorScale->setColorMapName( colorMapName );
			histogram->setColorMap( colorScale );
			colorTransferWidget->setColorMap( colorScale );
		}

	}

	void ColorHistogram::colorsChanged() {

		updateColorMap();

		//Look to see if the power cycles have changed.
		Record dataOptionsRecord = displayData->getOptions();
		if ( dataOptionsRecord.isDefined( WCPowerScaleHandler::POWER_CYCLES )){
			Record powerScaleRecord = dataOptionsRecord.asRecord( WCPowerScaleHandler::POWER_CYCLES );
			float powerCycles = powerScaleRecord.asFloat( "value" );
			resetPowerCycles( powerCycles );
		}

		//Look to see if the min/max range have changed.
		if ( dataOptionsRecord.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {
			Record rangeRecord = dataOptionsRecord.asRecord( PrincipalAxesDD::HISTOGRAM_RANGE);
			Vector<float> rangeVector = rangeRecord.asArrayFloat( "value");
			if ( rangeVector.size() >= 2 ) {
				histogram->setMinMaxValues( rangeVector[0], rangeVector[1], true);
			}
		}
	}

	void ColorHistogram::resetPowerCycles( float powerCycles ) {
		float oldCycles = powerScaler->cycles();
		if ( oldCycles != powerCycles ) {
			powerScaler->setCycles( powerCycles );
			blockSignals( true );
			ui.powerCyclesLineEdit->setText( QString::number(powerCycles) );
			float powerDistance = powerCycles + SCALE_LIMIT;
			float powerRange = 2 * SCALE_LIMIT;
			float sliderRange = SLIDER_MAX - SLIDER_MIN;
			int sliderValue = SLIDER_MIN +
			                  static_cast<int>(powerDistance / powerRange * sliderRange);
			ui.powerCyclesSlider->setValue( sliderValue );
			blockSignals( false );
			resetColorLookups();
		}
	}

	ColorHistogram::~ColorHistogram() {
		delete powerScaler;
		delete colorScale;
	}
}
