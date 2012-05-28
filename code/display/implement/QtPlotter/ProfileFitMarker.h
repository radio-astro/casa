/*
 * PlotMarker.h
 *   Used for displaying (center,peak) and fwhm points on the QtCanvas.
 *   Marks points used for spectral line fitting.
 *  Created on: May 23, 2012
 *      Author: slovelan
 */

#ifndef PROFILEFITMARKER_H_
#define PROFILEFITMARKER_H_

#include <QPainter>



namespace casa {

class WorldCanvasTranslator;

class ProfileFitMarker {

public:
	ProfileFitMarker(  const WorldCanvasTranslator * worldCanvasTranslator );
	void setCenterPeak( double xVal, double yVal );
	void setFWHM( double fwhm, double fwhmHeight );
	void drawMarker( QPainter& event);
	virtual ~ProfileFitMarker();

private:
	const WorldCanvasTranslator* worldCanvasTranslator;
	double center;
	double peak;
	double fwhm;
	double fwhmHeight;
	bool centerPeakSpecified;
	bool fwhmSpecified;
	//static const QString CENTER_PEAK_LABEL;
	//static const QString FWHM_LABEL;
};

} /* namespace casa */
#endif /* PROFILEFITMARKER_H_ */
