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

#include "Fitter.h"

namespace casa {

Fitter::Fitter() {
	// TODO Auto-generated constructor stub

}

void Fitter::setData( Vector<Float> dataValuesX, Vector<Float> dataValuesY ){
	clearFit();
	fitValues.resize(0);
	xValues = dataValuesX;
	yValues = dataValuesY;
}

Vector<Float> Fitter::getFitValues(){
	return fitValues;
}

QString Fitter::getErrorMessage() const {
	return errorMsg;
}

QString Fitter::getStatusMessage() const {
	return statusMsg;
}

float Fitter::getMean() const {
	int count = xValues.size();
	float totalCount = 0;
	float weightedMean = 0;
	if ( count > 0 ){
		float sum = 0;
		for ( int i = 0; i < count; i++ ){
			sum = sum + xValues[i] * yValues[i];
			totalCount = totalCount + yValues[i];
		}
		if ( totalCount > 0 ){
			weightedMean = sum / totalCount;
		}
	}
	return weightedMean;
}

Fitter::~Fitter() {
}

} /* namespace casa */
