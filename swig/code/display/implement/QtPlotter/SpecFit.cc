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