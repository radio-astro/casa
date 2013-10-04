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

#ifndef FITTERGAUSSIAN_H_
#define FITTERGAUSSIAN_H_

#include <guitools/Histogram/Fitter.h>

namespace casa {

/**
 * Manages Gaussian initial guesses and fits Gaussian curves to
 * the histogram.
 */
class FitterGaussian : public Fitter {
public:
	FitterGaussian();
	virtual QString getSolutionStatistics() const;
	void setPeak( double peakValue );
	void setCenter( double centerValue );
	void setFWHM( double fwhmValue );

	virtual bool doFit();
	virtual void toAscii( QTextStream& stream ) const;
	virtual ~FitterGaussian();
	virtual void clearFit();

private:
	double getPeak() const;
	double getCenter() const;
	double getFWHM() const;
	bool estimateCenterPeak();
	bool estimateFWHM();
	int getPeakIndex() const;
	double peak;
	double center;
	double fwhm;
	bool peakSpecified;
	bool centerSpecified;
	bool fwhmSpecified;
	float solutionPeak;
	float solutionCenter;
	float solutionFWHM;

};

} /* namespace casa */
#endif /* FITTERGAUSSIAN_H_ */
