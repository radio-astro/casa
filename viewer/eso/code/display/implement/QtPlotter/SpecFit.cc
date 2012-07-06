/*
 * SpecFit.cc
 *
 *  Created on: Jun 9, 2012
 *      Author: slovelan
 */

#include "SpecFit.h"

namespace casa {

SpecFit::SpecFit( QString fileSuffix) : suffix( fileSuffix ){
}



QString SpecFit::getSuffix(){
	return suffix;
}

bool SpecFit::isXPixels(){
	return false;
}

bool SpecFit::isSpecFitFor( int pixelX, int pixelY ) const{
	bool correctCenter = false;
	if ( centerX == pixelX && centerY == pixelY ){
		correctCenter = true;
	}
	return correctCenter;
}
void SpecFit::setFitCenter( int pixelX, int pixelY ){
	centerX = pixelX;
	centerY = pixelY;
}

void SpecFit::setXValues(Vector<Float>& xVals) {
	xValues = xVals;
}

Vector<Float> SpecFit::getXValues() const {
	return xValues;
}

Vector<Float> SpecFit::getYValues() const {
	return yValues;
}

void SpecFit::setCurveName( QString cName ){
	curveName = cName;
}

QString SpecFit::getCurveName() const {
	return curveName;
}

SpecFit::~SpecFit() {
}

} /* namespace casa */
