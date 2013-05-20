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

#include "CanvasModeFactory.h"
#include <display/QtPlotter/canvasMode/CanvasMode.h>
#include <display/QtPlotter/canvasMode/CanvasModeAnnotation.h>
#include <display/QtPlotter/canvasMode/CanvasModeChannel.h>
#include <display/QtPlotter/canvasMode/CanvasModeContextMenu.h>
#include <display/QtPlotter/canvasMode/CanvasModeRangeSelection.h>
#include <display/QtPlotter/canvasMode/CanvasModeZoom.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <QMouseEvent>
#include <QDebug>
namespace casa {


	CanvasModeFactory::CanvasModeFactory(QtCanvas* receiver ) {
		canvasModes.append( new CanvasModeAnnotation() );
		canvasModes.append( new CanvasModeChannel() );
		canvasModes.append( new CanvasModeContextMenu() );
		canvasModes.append( new CanvasModeRangeSelection() );
		canvasModes.append( new CanvasModeZoom() );
		for ( int i = 0; i < canvasModes.size(); i++ ){
			canvasModes[i]->setReceiver( receiver );
		}
	}

	CanvasMode* CanvasModeFactory::getModeForEvent( QMouseEvent* event ) {
#if defined(__APPLE__)
		const Qt::KeyboardModifier control_modifier = Qt::MetaModifier;
#else
		const Qt::KeyboardModifier control_modifier = Qt::ControlModifier;
#endif
		CanvasMode* canvasMode = NULL;
		if (event->button() == Qt::LeftButton) {
			if ( event->modifiers() == Qt::ShiftModifier ) {
				canvasMode = canvasModes[CanvasMode::MODE_RANGESELECTION];
			} else if ( event->modifiers() == control_modifier ) {
				canvasMode = canvasModes[CanvasMode::MODE_ANNOTATION];
			} else {
				canvasMode = canvasModes[CanvasMode::MODE_ZOOM];
			}
		} else if ( event->button() == Qt::RightButton ) {
			if ( event->modifiers() == Qt::ShiftModifier ) {

			} else if ( event->modifiers() == control_modifier ) {
				canvasMode = canvasModes[CanvasMode::MODE_CHANNEL];
			} else {
				canvasMode = canvasModes[CanvasMode::MODE_CONTEXTMENU];
			}
		} else {
			qDebug() << "Unrecognized mode button"<<event->button();
		}
		return canvasMode;
	}

	CanvasMode* CanvasModeFactory::getMode( CanvasMode::ModeIndex mode ) {
		return canvasModes[mode];
	}

	CanvasModeFactory::~CanvasModeFactory() {
		while( !canvasModes.isEmpty() ) {
			CanvasMode* mode = canvasModes.takeFirst();
			delete mode;
		}
	}

} /* namespace casa */
