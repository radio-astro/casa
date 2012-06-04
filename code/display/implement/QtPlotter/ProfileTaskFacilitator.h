/*
 * ProfileTaskFacilitator.h
 *
 *  Created on: May 16, 2012
 *      Author: slovelan
 *
 *  This abstract class is an interface that specifies the basic functionality that
 *  spectral line moments/fitting should implement.  It is expected that it will be subclassed
 *  by implementation classes to provide specialized behavior.
 *
 */

#ifndef PROFILE_TASK_FACILITATOR_H_
#define PROFILE_TASK_FACILITATOR_H_

#include <QString>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>

class QDoubleValidator;

namespace casa {

class QtCanvas;
class ProfileTaskMonitor;
class LogIO;

template <class T> class ImageInterface;

class ProfileTaskFacilitator {
public:
	ProfileTaskFacilitator();
	virtual ~ProfileTaskFacilitator();

	virtual void setUnits( QString units ) = 0;
	virtual void setRange(float start, float end )=0;
	virtual void reset() = 0;

	virtual void setCanvas( QtCanvas* pixelCanvas );
	virtual void setTaskMonitor( ProfileTaskMonitor* monitor );
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

	bool isOptical();
	void setOptical( bool optical );

	virtual void clear();
	void setCollapseVals(const Vector<Float> &spcVals);

protected:
	bool isValidChannelRangeValue( QString str, const QString& endStr );
	void findChannelRange( float startVal, float endVal,
		const Vector<Float>& specValues, Int& channelStartIndex, Int& channelEndIndex );
	QtCanvas* pixelCanvas;
	ProfileTaskMonitor* taskMonitor;
	LogIO* logger;

private:
	bool optical;
	QDoubleValidator *validator;
};

} /* namespace casa */
#endif /* SPECFITTER_H_ */
