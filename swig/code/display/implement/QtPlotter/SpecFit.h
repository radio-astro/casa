/*
 * SpecFit.h
 *
 *  Created on: Jun 9, 2012
 *      Author: slovelan
 */

#ifndef SPECFIT_H_
#define SPECFIT_H_

#include <casa/Arrays/Vector.h>
#include <QString>

namespace casa {

class SpecFit {
public:
	SpecFit( QString suffix );
	QString getSuffix();
	virtual bool isXPixels();
	virtual void evaluate( Vector<Float>& xValues ) = 0;
	Vector<Float> getXValues() const;
	void setXValues( Vector<Float>& xVals );
	Vector<Float> getYValues() const;
	void setCurveName( QString curveName );
	QString getCurveName() const;
	bool isSpecFitFor( int pixelX, int pixelY ) const;
	void setFitCenter( int pixelX, int pixelY );
	virtual ~SpecFit();

protected:
	Vector<Float> xValues;
	Vector<Float> yValues;

private:
	QString suffix;
	QString curveName;
	int centerX;
	int centerY;

};


} /* namespace casa */
#endif /* SPECFIT_H_ */
