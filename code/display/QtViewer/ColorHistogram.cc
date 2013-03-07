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
#include <display/QtViewer/ColorHistogramScale.h>
#include <images/Images/ImageInterface.h>
#include <casa/Arrays/Array.h>

namespace casa {

ColorHistogram::ColorHistogram(QWidget *parent)
    : QDialog(parent), displayData( NULL ),
      powerScaler( NULL), colorScale( NULL ), COLOR_MAX(100){
	ui.setupUi(this);
	setWindowTitle( "Image Color Mapping");

	histogram = new BinPlotWidget( false, true, true, true, this );
	histogram->setPlotMode( BinPlotWidget::IMAGE_MODE );
	histogram->setDisplayAxisTitles( true );
	histogram->setDisplayPlotTitle( true );
	histogram->setColorScaleMax( COLOR_MAX );
	QHBoxLayout* hBox = new QHBoxLayout();
	hBox->addWidget( histogram );
	ui.histogramHolder->setLayout( hBox );

	powerScaler = new WCPowerScaleHandler();
	powerScaler->setRangeMax( COLOR_MAX );

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(acceptRange()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(cancelRange()));
}


void ColorHistogram::acceptRange(){
	if ( displayData != NULL ){
		std::pair<double,double> minMaxPair = histogram->getMinMaxValues();
		float minValue = static_cast<float>(minMaxPair.first);
		float maxValue = static_cast<float>(minMaxPair.second );
		displayData->setHistogramColorRange( minValue, maxValue );
	}
	close();
}


void ColorHistogram::cancelRange(){
	close();
}

void ColorHistogram::setDisplayData( QtDisplayData* dd ){
	if ( displayData != NULL ){
		disconnect( displayData, SIGNAL(colorBarChange()), this, SLOT(colorsChanged()));
	}
	displayData = dd;
	if ( displayData != NULL ){
		connect( displayData, SIGNAL(colorBarChange()), this, SLOT( colorsChanged()));
		histogram->setColorBarVisible( true );
		ImageInterface<float>* img = displayData->imageInterface();
		histogram->setImage( img );
		colorsChanged();
	}
}

void ColorHistogram::resetColorLookups(){
	//Have to change the colors used to draw the
	//histogram.
	std::vector<float> intensities = histogram->getXValues();
	//Get intensities that match the range of the color
	//Scale.
	Vector<uInt> colorValues = computeScaledIntensities( intensities );
	histogram->setColorLookups( colorValues );
}

Vector<uInt> ColorHistogram::computeScaledIntensities(const std::vector<float>& intensities ){

	//We have to scale the intensities to the interval [0,1].
	float minValue = std::numeric_limits<float>::max();
	float maxValue = std::numeric_limits<float>::min();
	int intensityCount = intensities.size();
	Vector<uInt> colorValues;
	if ( intensityCount > 0 ){
		Vector<Float> scaledIntensities( intensityCount );
		for ( int i = 0; i < intensityCount; i++ ){
			if ( intensities[i] < minValue ){
				minValue = intensities[i];
			}
			if ( intensities[i] > maxValue ){
				maxValue = intensities[i];
			}
			scaledIntensities[i] = intensities[i];
		}

		powerScaler->setDomainMinMax( minValue, maxValue );

		//IPosition colorValueSizing( intensityCount );
		colorValues.resize( intensityCount );
		(*powerScaler)( colorValues, scaledIntensities );
	}
	return colorValues;
}

void ColorHistogram::colorsChanged(){

	//Update the color map
	String colorMapName = displayData->getColormap();
	bool reset = false;
	if ( colorScale != NULL ){
		String existingMapName = colorScale->getColorMapName();
		if (existingMapName != colorMapName ){
			delete colorScale;
			reset = true;
			colorScale = new ColorHistogramScale();
		}
	}
	else {
		reset = true;
		colorScale = new ColorHistogramScale();
	}

	if ( reset ){
		colorScale->setColorMapName( colorMapName );
		histogram->setColorMap( colorScale );
	}

	//Look to see if the power cycles have changed.
	Record dataOptionsRecord = displayData->getOptions();
	Record powerScaleRecord = dataOptionsRecord.asRecord( WCPowerScaleHandler::POWER_CYCLES );
	float powerCycles = powerScaleRecord.asFloat( "value" );
	float oldCycles = powerScaler->cycles();
	if ( oldCycles != powerCycles ){
		powerScaler->setCycles( powerCycles );
		reset = true;
	}
	if ( reset ){
		resetColorLookups();
	}

	//Look to see if the min/max range have changed.
	if ( dataOptionsRecord.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)){
		Record rangeRecord = dataOptionsRecord.asRecord( PrincipalAxesDD::HISTOGRAM_RANGE);
		Vector<float> rangeVector = rangeRecord.asArrayFloat( "value");
		if ( rangeVector.size() >= 2 ){
			histogram->setMinMaxValues( rangeVector[0], rangeVector[1], true);
		}
	}
}

ColorHistogram::~ColorHistogram(){
	delete powerScaler;
	delete colorScale;
}
}
