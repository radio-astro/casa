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
#include "CanvasMode.h"
#include <display/QtPlotter/QtCanvas.qo.h>

namespace casa {

	void CanvasMode::setReceiver( QtCanvas* canvas ) {
		receiver = canvas;
	}

	bool CanvasMode::isMode( ModeIndex /*mode*/ ) const {
		return false;
	}

	CanvasMode::CanvasMode(Qt::CursorShape modeCursor):
		receiver(NULL), cursor( modeCursor)  {
	}
	void CanvasMode::mousePressEvent( QMouseEvent* event ) {
		if ( receiver != NULL ) {
			receiver->setCursor( cursor );
			mousePressEventSpecific( event );
		}
	}

	void CanvasMode::mouseReleaseEvent( QMouseEvent* event ) {
		if ( receiver != NULL ) {
			mouseReleaseEventSpecific( event );
			receiver->unsetCursor();
		}
	}

	CanvasMode::~CanvasMode() {
	}

} /* namespace casa */
