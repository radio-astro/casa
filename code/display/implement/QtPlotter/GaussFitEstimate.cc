/*
 * GaussFitEstimate.cc
 *
 *  Created on: Apr 27, 2012
 *      Author: slovelan
 */

#include "GaussFitEstimate.h"

namespace casa {

int GaussFitEstimate::estimateNumber = 0;

GaussFitEstimate::GaussFitEstimate() :
	PEAK_STR("p"),
	CENTER_STR("c"),
	BEAM_WIDTH_STR("f"){
	identifier = estimateNumber;
	estimateNumber++;

}

void GaussFitEstimate::setPeak( float pValue ){
	peakValue = pValue;
}
float GaussFitEstimate::getPeak() const {
	return peakValue;
}
void GaussFitEstimate::setCenter( float cValue ){
	centerValue = cValue;
}
float GaussFitEstimate::getCenter() const {
	return centerValue;
}

void GaussFitEstimate::setBeamWidth( float bValue ){
	beamWidth = bValue;
}
float GaussFitEstimate::getBeamWidth() const {
	return beamWidth;
}

void GaussFitEstimate::setPeakFixed( bool fixed ){
	peakFixed = fixed;
}

float GaussFitEstimate::getPeakFixed() const {
	return peakFixed;
}

void GaussFitEstimate::setCenterFixed( bool fixed ){
	centerFixed = fixed;
}

float GaussFitEstimate::getCenterFixed() const {
	return centerFixed;
}

void GaussFitEstimate::setBeamWidthFixed( bool fixed ) {
	beamWidthFixed = fixed;
}

float GaussFitEstimate::getBeamWidthFixed() const {
	return beamWidthFixed;
}

QString GaussFitEstimate::getFixedString() const {
	QString fixedStr;
	if ( centerFixed ){
		fixedStr.append( CENTER_STR );
	}
	if ( peakFixed ){
		fixedStr.append( PEAK_STR );
	}
	if ( beamWidthFixed ){
		fixedStr.append( BEAM_WIDTH_STR );
	}
	return fixedStr;
}


GaussFitEstimate::~GaussFitEstimate() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
