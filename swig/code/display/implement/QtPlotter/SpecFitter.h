/*
 * SpecFitter.h
 *
 *  Created on: May 16, 2012
 *      Author: slovelan
 *
 *  This abstract class is an interface that specifies the basic functionality that
 *  spectral line profiling should implement.  It is expected that it will be subclassed
 *  by implementation classes to provide specialized spectral line fitting behavior.
 *
 */

#ifndef SPECFITTER_H_
#define SPECFITTER_H_

#include <QString>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
namespace casa {

class QtCanvas;
class SpecFitMonitor;
class LogIO;
template <class T> class ImageInterface;

class SpecFitter {
public:
	SpecFitter();
	virtual ~SpecFitter();

	virtual void specLineFit() = 0;
	virtual void setUnits( QString units ) = 0;
	virtual void setRange(float start, float end )=0;
	virtual void resetSpectralFitter() = 0;

	virtual void setCanvas( QtCanvas* pixelCanvas );
	virtual void setSpecFitMonitor( SpecFitMonitor* monitor );
	virtual void setLogger( LogIO* log );
	virtual void plotMainCurve();

	virtual QString getFileName();
	virtual void logWarning(String msg );
	virtual void postStatus( String status );
	virtual Vector<Float> getXValues() const;
	virtual Vector<Float> getYValues() const;
	virtual Vector<Float> getZValues() const;
	virtual QString getYUnit() const;
	virtual QString getYUnitPrefix() const;
	virtual String getXAxisUnit() const;
	virtual const ImageInterface<Float>* getImage() const;
	virtual const String getPixelBox() const;

protected:
	QtCanvas* pixelCanvas;
	SpecFitMonitor* specFitMonitor;
	LogIO* logger;
};

} /* namespace casa */
#endif /* SPECFITTER_H_ */
