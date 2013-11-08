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

#include "CanvasModeZoom.h"
#include <display/QtPlotter/QtCanvas.qo.h>
#include <QMouseEvent>
namespace casa {

	CanvasModeZoom::CanvasModeZoom():CanvasMode(Qt::CrossCursor) {

	}

	bool CanvasModeZoom::isMode( ModeIndex mode ) const {
		bool matchingMode = false;
		if ( mode == MODE_ZOOM ) {
			matchingMode = true;
		}
		return matchingMode;
	}

	void CanvasModeZoom::mousePressEventSpecific( QMouseEvent* event) {
		receiver->startZoomRect( event );
		receiver->setCursor( cursor );
	}

	void CanvasModeZoom::mouseMoveEvent( QMouseEvent* event ) {
		receiver->updateZoomRect( event );
	}

	void CanvasModeZoom::mouseReleaseEventSpecific( QMouseEvent* event ) {
		receiver->endZoomRect( event );
	}

	QString CanvasModeZoom::toString() const {
		return "Zoom";
	}

	CanvasModeZoom::~CanvasModeZoom() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
