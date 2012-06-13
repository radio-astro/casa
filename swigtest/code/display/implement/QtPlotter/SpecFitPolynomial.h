/*
 * SpecFitPolynomial.h
 *
 *  Created on: Jun 9, 2012
 *      Author: slovelan
 */

#ifndef SPECFITPOLYNOMIAL_H_
#define SPECFITPOLYNOMIAL_H_

#include <display/QtPlotter/SpecFit.h>

namespace casa {

class SpecFitPolynomial : public SpecFit {
public:
	SpecFitPolynomial( Vector<Double> coefficients );
	bool isXPixels();
	void evaluate( Vector<Float>& xValues );
	virtual ~SpecFitPolynomial();
private:
	Vector<Double> coefficients;
};

} /* namespace casa */
#endif /* SPECFITPOLYNOMIAL_H_ */
