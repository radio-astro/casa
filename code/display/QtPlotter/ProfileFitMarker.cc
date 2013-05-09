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


#include "ProfileFitMarker.h"
#include <display/QtPlotter/WorldCanvasTranslator.h>
#include <QPaintEvent>

namespace casa {

	ProfileFitMarker::ProfileFitMarker( const WorldCanvasTranslator* wct) :
		worldCanvasTranslator( wct) {
		centerPeakSpecified = false;
		fwhmSpecified = false;
	}

	void ProfileFitMarker::setCenterPeak( double xVal, double yVal ) {
		center = xVal;
		peak = yVal;
		centerPeakSpecified = true;
	}

	void ProfileFitMarker::setFWHM( double fwhmVal, double fwhmHeightVal  ) {
		fwhm = fwhmVal;
		fwhmHeight = fwhmHeightVal;
		fwhmSpecified = true;
	}

	void ProfileFitMarker::drawMarker( QPainter& painter) {

		//We can't do anything unless the center/peak is specified.
		if ( centerPeakSpecified ) {
			//Set-up the pen
			const QPen& pen = painter.pen();
			QPen penCopy( pen );
			const int POINT_SIZE = 5;
			const int FWHM_SIZE = 3;
			penCopy.setColor( Qt::black );
			penCopy.setWidth( POINT_SIZE );
			painter.setPen(penCopy);

			//Plot the center/peak
			//Calculate the pixel values for the points we need to plot
			int pixelCenter = worldCanvasTranslator -> getPixelX( center );
			int pixelPeak = worldCanvasTranslator -> getPixelY( peak );

			//Mark a point at the (center,peak)
			painter.drawPoint( pixelCenter, pixelPeak );

			if (fwhmSpecified ) {
				penCopy.setWidth( FWHM_SIZE );
				painter.setPen( penCopy );

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
