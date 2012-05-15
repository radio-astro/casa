/*
 * GaussFitEstimate.h
 *
 *  Created on: Apr 27, 2012
 *      Author: slovelan
 */

#ifndef GAUSSFITESTIMATE_H_APRIL
#define GAUSSFITESTIMATE_H_APRIL

#include <QString>

namespace casa {

/**
 * Represents an initial Gaussian spectral line
 * fitting estimate.
 */
class GaussFitEstimate {
public:
	GaussFitEstimate();
	void setPeak( float pValue );
	float getPeak() const;
	void setCenter( float cValue );
	float getCenter() const;
	void setBeamWidth( float bValue );
	float getBeamWidth() const;
	void setPeakFixed( bool fixed );
	float getPeakFixed() const;
	void setCenterFixed( bool fixed );
	float getCenterFixed() const;
	void setBeamWidthFixed( bool fixed );
	float getBeamWidthFixed() const;
	/**
	 * Returns a string indicating the parameters that should be
	 * held constant during the fit. 'p' for peak, 'c' for center
	 * and 'f' for fwhm.  For example, fc means to hold the fwhm
	 * and the center constant during the fit.
	 */
	QString getFixedString() const;

	virtual ~GaussFitEstimate();
private:
	static int estimateNumber;
	int identifier;
	float peakValue;
	float centerValue;
	float beamWidth;
	bool peakFixed;
	bool centerFixed;
	bool beamWidthFixed;
	const QString PEAK_STR;
	const QString CENTER_STR;
	const QString BEAM_WIDTH_STR;

};

} /* namespace casa */
#endif /* GAUSSFITESTIMATE_H_ */
