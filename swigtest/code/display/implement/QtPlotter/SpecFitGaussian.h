/*
 * SpecFitGaussian.h
 *
 *  Created on: Jun 9, 2012
 *      Author: slovelan
 */

#ifndef SPECFITGAUSSIAN_H_
#define SPECFITGAUSSIAN_H_

#include <display/QtPlotter/SpecFit.h>

namespace casa {

class SpecFitGaussian : public SpecFit {
public:
	SpecFitGaussian( float peak, float center, float fwhm, int index );
	void setPeak( float peak );
	void setCenter( float center );
	void setFWHM( float fwhm );
	void evaluate( Vector<Float>& xValues );
	virtual ~SpecFitGaussian();
private:
	float peak;
	float center;
	float fwhm;
};

} /* namespace casa */
#endif /* SPECFITGAUSSIAN_H_ */
