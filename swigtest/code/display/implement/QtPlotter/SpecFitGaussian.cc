/*
 * SpecFitGaussian.cc
 *
 *  Created on: Jun 9, 2012
 *      Author: slovelan
 */

#include "SpecFitGaussian.h"
#include <scimath/Functionals/Gaussian1D.h>

namespace casa {

SpecFitGaussian::SpecFitGaussian( float peakVal, float centerVal, float fwhmVal, int index) :
		SpecFit("_Gaussian_FIT_"+QString::number(index)) {
	peak = peakVal;
	center = centerVal;
	fwhm = fwhmVal;
}

void SpecFitGaussian::setPeak( float peakVal ){
	peak = peakVal;
}
void SpecFitGaussian::setCenter( float centerVal ){
	center = centerVal;
}

void SpecFitGaussian::setFWHM( float fwhmVal ){
	fwhm = fwhmVal;
}

void SpecFitGaussian::evaluate(Vector<Float>& xVals ){
	//Model the fit so that we can give it to the canvas for
	//drawing.
	xValues = xVals;
	yValues.resize( xValues.size());
	Gaussian1D<Float> gaussFit(peak, center, fwhm );
	for( int i = 0; i < static_cast<int>(xValues.size()); i++ ){
		yValues[i] = gaussFit.eval(&xValues[i]);
	}

}

SpecFitGaussian::~SpecFitGaussian() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
