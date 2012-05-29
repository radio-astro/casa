/*
 * SpecFitMonitor.h
 *
 *  Created on: May 16, 2012
 *      Author: slovelan
 */

#ifndef SPECFITMONITOR_H_
#define SPECFITMONITOR_H_

#include <QString>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>

namespace casa {

template <class T> class ImageInterface;

class SpecFitMonitor {
public:
	SpecFitMonitor();
	virtual QString getFileName() const = 0;
	virtual Vector<Float> getXValues() const = 0;
	virtual Vector<Float> getYValues() const = 0;
	virtual Vector<Float>getZValues() const = 0;
	virtual QString getYUnit() const = 0;
	virtual QString getYUnitPrefix() const = 0;
	virtual String getXAxisUnit() const = 0;
	virtual void postStatus( String str ) = 0;
	virtual void plotMainCurve() = 0;
	virtual const ImageInterface<Float>* getImage() const = 0;
	virtual const String getPixelBox() const = 0;
	virtual ~SpecFitMonitor();
};

} /* namespace casa */
#endif /* SPECFITMONITOR_H_ */
