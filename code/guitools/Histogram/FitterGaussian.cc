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
//#include <scimath/Functionals/Gaussian1D.h>
#include <scimath/Fitting/FitGaussian.h>
#include <QDebug>

#include <iostream>
using namespace std;
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

				FitGaussian<Float> fitgauss;
				fitgauss.setDimensions(1);
				fitgauss.setNumGaussians(1);

				//Initialize the x-data values
				Matrix<Float> components;
				int xCount = xValues.size();
				components.resize(xCount,1);
				for ( int i = 0; i < xCount; i++ ){
					components(i,0) = xValues[i];
				}

				//Initialize the estimate
				Matrix<Float> estimate;
				estimate.resize(1, 3);
				estimate(0,0) = peak;
				estimate(0,1) = center;
				estimate(0,2) = fwhm;
				fitgauss.setFirstEstimate(estimate);

				try {
					Matrix<Float> solution = fitgauss.fit(components, yValues, rmsError );
					solutionPeak = solution( 0, 0 );
					solutionCenter = solution( 0, 1 );
					solutionFWHM= solution( 0, 2 );
					solutionChiSquared = fitgauss.chisquared();
					solutionRMS = fitgauss.RMS();
					solutionConverged = fitgauss.converged();
					Gaussian1D<Float> gaussFit(solutionPeak, solutionCenter, solutionFWHM );
					for( int i = 0; i < static_cast<int>(xValues.size()); i++ ){
							fitValues[i] = gaussFit.eval(&xValues[i]);
					}
					dataFitted = true;
				}
				catch (AipsError& err) {
				    qDebug() << "ERROR: " << err.what();
				    fitSuccessful = false;
				}
			}
		}
	}
	return fitSuccessful;
}

QString FitterGaussian::getSolutionStatistics() const {
	QString stats;
	if ( solutionConverged ){
		stats.append( "Fit converged to a value.\n");
	}
	else {
		stats.append( "Fit did not converge.\n Result generated the smallest RMS.\n");
	}
	int fieldWidth = -14;
	QChar fillChar(' ');
	QString centerStr("%0");
	centerStr = centerStr.arg( "Center:", fieldWidth, fillChar);
	stats.append( centerStr + QString::number(solutionCenter)+"\n");
	QString peakStr("%0");
	peakStr = peakStr.arg( "Peak:", fieldWidth, fillChar);
	stats.append( peakStr + QString::number(solutionPeak)+"\n");
	QString fwhmStr("%0");
	fwhmStr = fwhmStr.arg( "FWHM:", fieldWidth, fillChar );
	stats.append( fwhmStr + QString::number(solutionFWHM)+"\n");
	QString chiSquaredStr("%0");
	chiSquaredStr = chiSquaredStr.arg( "Chi-squared:", fieldWidth, fillChar);
	stats.append( chiSquaredStr + QString::number(solutionChiSquared)+"\n");
	QString rmsStr("%0");
	rmsStr = rmsStr.arg( "RMS:", fieldWidth, fillChar );
	stats.append( rmsStr + QString::number(solutionRMS));
	return stats;
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
