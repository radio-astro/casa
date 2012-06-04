/*
 * ProfileTaskMonitor.h
 *
 *  Created on: May 16, 2012
 *      Author: slovelan
 */

#ifndef PROFILETASKMONITOR_H_
#define PROFILETASKMONITOR_H_

#include <QString>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>

namespace casa {

template <class T> class ImageInterface;

class ProfileTaskMonitor {
public:
	ProfileTaskMonitor();
	virtual QString getFileName() const = 0;
	virtual QString getImagePath() const =0;
	virtual Vector<Float> getXValues() const = 0;
	virtual Vector<Float> getYValues() const = 0;
	virtual Vector<Float>getZValues() const = 0;
	virtual QString getYUnit() const = 0;
	virtual QString getYUnitPrefix() const = 0;
	virtual String getXAxisUnit() const = 0;
	virtual void postStatus( String str ) = 0;
	virtual void plotMainCurve() = 0;
	virtual const ImageInterface<Float>* getImage() const = 0;
	virtual const void getPixelBounds( Vector<double>& xPixels, Vector<double>& yPixels) const = 0;

	//Specific to Moments
	virtual void persist( const QString& key, const QString& value ) = 0;
	virtual QString read( const QString & key ) const = 0;
	virtual void imageCollapsed(String path, String dataType, String displayType, Bool autoRegister, Bool tmpData, ImageInterface<Float>* img = NULL)=0;



	virtual ~ProfileTaskMonitor();
};

} /* namespace casa */
#endif /* PROFILETASKMONITOR_H_ */
