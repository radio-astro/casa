/*
 * PlotMarker.cc
 *
 *  Created on: May 23, 2012
 *      Author: slovelan
 */

#include "ProfileFitMarker.h"
#include <display/QtPlotter/WorldCanvasTranslator.h>
#include <QPaintEvent>
#include <QDebug>

namespace casa {

//const QString ProfileFitMarker::CENTER_PEAK_LABEL = "(C,P)";
//const QString ProfileFitMarker::FWHM_LABEL = "FWHM";

ProfileFitMarker::ProfileFitMarker( const WorldCanvasTranslator* wct) :
		worldCanvasTranslator( wct){
	centerPeakSpecified = false;
	fwhmSpecified = false;
}

void ProfileFitMarker::setCenterPeak( double xVal, double yVal ){
	center = xVal;
	peak = yVal;
	centerPeakSpecified = true;
}

void ProfileFitMarker::setFWHM( double fwhmVal, double fwhmHeightVal  ){
	fwhm = fwhmVal;
	fwhmHeight = fwhmHeightVal;
	fwhmSpecified = true;
}

void ProfileFitMarker::drawMarker( QPainter& painter){

	//We can't do anything unless the center/peak is specified.
	if ( centerPeakSpecified ){
		//Set-up the pen
		const QPen& pen = painter.pen();
		QPen penCopy( pen );
		penCopy.setWidth( 5 );
		painter.setPen(penCopy);

		//Plot the center/peak
		//Calculate the pixel values for the points we need to plot
		int pixelCenter = worldCanvasTranslator -> getPixelX( center );
		int pixelPeak = worldCanvasTranslator -> getPixelY( peak );

		//Mark a point at the (center,peak)
		painter.drawPoint( pixelCenter, pixelPeak );

		if (fwhmSpecified ){
			//Draw a line to show the fwhm
			int pixelfwhmHeight = worldCanvasTranslator -> getPixelY( fwhmHeight );
			double halfWidth = fwhm / 2;
			double fwhmXFirst = center - halfWidth;
			double fwhmXSecond = center + halfWidth;
			int pixelfwhmXFirst = worldCanvasTranslator -> getPixelX( fwhmXFirst );
			int pixelfwhmXSecond = worldCanvasTranslator -> getPixelX( fwhmXSecond );
			painter.drawLine( pixelfwhmXFirst, pixelfwhmHeight, pixelfwhmXSecond, pixelfwhmHeight );
		}
		painter.setPen( pen );
	}
}

ProfileFitMarker::~ProfileFitMarker() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
