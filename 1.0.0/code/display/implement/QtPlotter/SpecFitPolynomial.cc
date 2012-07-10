/*
 * SpecFitPolynomial.cc
 *
 *  Created on: Jun 9, 2012
 *      Author: slovelan
 */

#include "SpecFitPolynomial.h"

namespace casa {

SpecFitPolynomial::SpecFitPolynomial( Vector<Double> coeffs) :
		SpecFit("_Polynomial_FIT"){
	coefficients = coeffs;
}

bool SpecFitPolynomial::isXPixels(){
	return true;
}

void SpecFitPolynomial::evaluate( Vector<Float>& xVals ){
	//Evaluate the polynomial
	yValues.resize(xVals.size());
	int polyOrder = coefficients.size();
	for( int i = 0; i < static_cast<int>(xVals.size()); i++ ){
		yValues[i] = 0;
		for ( int j = 0; j < polyOrder; j++ ){
			yValues[i] = yValues[i] + coefficients[j] * casa::pow( xVals[i], j);
		}
	}
}

SpecFitPolynomial::~SpecFitPolynomial() {
}

} /* namespace casa */
