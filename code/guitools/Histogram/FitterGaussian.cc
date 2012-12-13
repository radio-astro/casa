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

#include "FitterGaussian.h"
#include <scimath/Functionals/Gaussian1D.h>

namespace casa {

FitterGaussian::FitterGaussian() {
	peakSpecified = false;
	centerSpecified = false;
	fwhmSpecified = false;
}

void FitterGaussian::setPeak( double peakValue ){
	peak = peakValue;
	peakSpecified = true;
}
void FitterGaussian::setCenter( double centerValue ){
	center = centerValue;
	centerSpecified = true;
}
void FitterGaussian::setFWHM( double fwhmValue ){
	fwhm = fwhmValue;
	fwhmSpecified = true;
}

double FitterGaussian::getPeak() const {
	return peak;
}

double FitterGaussian::getCenter() const {
	return center;
}

double FitterGaussian::getFWHM() const {
	return fwhm;
}

int FitterGaussian::getPeakIndex() const {
	int maxIndex = -1;
	double maxValue = std::numeric_limits<float>::min();
	int dataCount = yValues.size();
	for ( int i = 0; i < dataCount; i++ ){
		if ( yValues[i] > maxValue ){
			maxValue = yValues[i];
			maxIndex = i;
		}
	}
	return maxIndex;
}

bool FitterGaussian::estimateFWHM(){
	bool estimated = true;
	if ( !fwhmSpecified ){
		//Find where we are just less than half the peak
		//from the center.
		int peakIndex = getPeakIndex();
		bool foundFwhm = false;
		if ( peakIndex >= 0 ){
			float targetValue = yValues[peakIndex] / 2;
			int startIndex = peakIndex - 1;
			int endIndex = peakIndex + 1;
			int dataCount = xValues.size();
			while ( startIndex >= 0 || endIndex < dataCount ){
				if ( startIndex >= 0 && yValues[startIndex] < targetValue  ){
					fwhm = 2 * qAbs( xValues[startIndex] - xValues[peakIndex]);
					foundFwhm = true;
					break;
				}
				if ( endIndex < dataCount && yValues[endIndex] < targetValue ){
					fwhm = 2 * qAbs( xValues[endIndex] - xValues[peakIndex]);
					foundFwhm = true;
					break;
				}
				startIndex--;
				endIndex++;
			}
		}
		if ( !foundFwhm ){
			estimated = false;
			errorMsg = "Could not estimate FWHM from the data.";
		}
	}
	fwhmSpecified = true;
	return estimated;
}

bool FitterGaussian::estimateCenterPeak(){
	bool estimated = true;
	if ( !centerSpecified || ! peakSpecified ){
		int maxIndex = getPeakIndex();
		if ( maxIndex >= 0 ){
			if ( !centerSpecified ){
				center = xValues[maxIndex];
			}
			if ( !peakSpecified ){
				peak = yValues[maxIndex];
			}
		}
		else {
			errorMsg = "Please check for valid histogram data.";
			estimated = false;
		}
	}
	centerSpecified = true;
	peakSpecified = true;
	return estimated;
}

bool FitterGaussian::doFit(){
	//Model the fit so that we can give it to the canvas for
	//drawing.
	bool fitSuccessful = true;
	fitValues.resize( xValues.size());
	fitSuccessful = estimateCenterPeak();
	if ( fitSuccessful ){
		fitSuccessful = estimateFWHM();
		if ( fitSuccessful ){
			if ( fwhm <= 0 ){
				fitSuccessful = false;
				errorMsg = "The full width at half maximum value must be positive.";
			}
			else {
				Gaussian1D<Float> gaussFit(peak, center, fwhm );
				for( int i = 0; i < static_cast<int>(xValues.size()); i++ ){
					fitValues[i] = gaussFit.eval(&xValues[i]);
				}
				dataFitted = true;
			}
		}
	}
	return fitSuccessful;
}

void FitterGaussian::clearFit(){
	errorMsg="";
	dataFitted = false;
	centerSpecified = false;
	peakSpecified = false;
	fwhmSpecified = false;
}

void FitterGaussian::toAscii( QTextStream& stream ) const {
	const QString END_LINE( "\n" );
	stream << "Gaussian Fit" << END_LINE;
	stream << "Center: "<< center << END_LINE;
	stream << "Peak: "<< peak << END_LINE;
	stream << "FWHM: "<< fwhm << END_LINE << END_LINE;
	Fitter::toAscii( stream );
}

FitterGaussian::~FitterGaussian() {
}

} /* namespace casa */
