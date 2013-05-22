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

#include "CanvasModeRangeSelection.h"
#include <display/QtPlotter/QtCanvas.qo.h>
#include <QMouseEvent>
namespace casa {

	CanvasModeRangeSelection::CanvasModeRangeSelection():
		CanvasMode(Qt::SplitHCursor) {
	}

	bool CanvasModeRangeSelection::isMode( ModeIndex mode ) const {
		bool matchingMode = false;
		if ( mode == MODE_RANGESELECTION ) {
			matchingMode = true;
		}
		return matchingMode;
	}


	void CanvasModeRangeSelection::mousePressEventSpecific( QMouseEvent* event ) {
		receiver->startRangeX( event );
	}

	void CanvasModeRangeSelection::mouseMoveEvent( QMouseEvent* event ) {
		receiver->updateRangeX( event );
	}

	void CanvasModeRangeSelection::mouseReleaseEventSpecific( QMouseEvent* event ) {
		receiver->endRangeX( event );
	}

	QString CanvasModeRangeSelection::toString() const {
		return "Range Selection";
	}
	CanvasModeRangeSelection::~CanvasModeRangeSelection() {
	}

} /* namespace casa */
