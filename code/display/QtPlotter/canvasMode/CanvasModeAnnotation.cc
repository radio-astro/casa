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

#include "CanvasModeAnnotation.h"
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/annotations/AnnotationText.h>
#include <QMouseEvent>

namespace casa {

	CanvasModeAnnotation::CanvasModeAnnotation()
		:CanvasMode(Qt::CrossCursor) {
		dragEnabled = false;
	}

	bool CanvasModeAnnotation::isMode( ModeIndex mode ) const {
		bool matchingMode = false;
		if ( mode == MODE_ANNOTATION ) {
			matchingMode = true;
		}
		return matchingMode;
	}

	void CanvasModeAnnotation::mousePressEventSpecific( QMouseEvent* event ) {
		receiver->selectAnnotation( event );
		dragEnabled = true;
		if ( receiver->isAnnotation( event ) ) {
			Annotation* currentAnnotation = receiver->getSelectedAnnotation();
			currentAnnotation->registerMousePosition( event->pos().x(), event->pos().y() );
		} else {
			if ( receiver->selectedAnnotation != NULL ) {
				delete receiver->selectedAnnotation;
			}
			receiver->selectedAnnotation = new AnnotationText( receiver );
			receiver->selectedAnnotation->move( event->pos().x(), event->pos().y() );
		}
	}

	void CanvasModeAnnotation::mouseMoveEvent( QMouseEvent* event ) {
		if ( dragEnabled ) {
			if ( receiver->isAnnotation( event ) ) {
				Annotation* currentAnnotation = receiver->getSelectedAnnotation();
				currentAnnotation->mousePositionMoved( event->pos().x(), event->pos().y() );
				receiver->update();
			} else if ( receiver->isAnnotationActive() ) {
				Annotation* activeAnnotation = receiver->getActiveAnnotation();
				activeAnnotation->setDimensionsPosition( event->pos().x(), event->pos().y() );
				receiver->update();
			}
		} else {
			if ( receiver->isAnnotation( event ) ) {
				Annotation* currentAnnotation = receiver->getSelectedAnnotation();
				currentAnnotation->registerMousePosition( event->pos().x(), event->pos().y() );
				receiver->update();
			}
		}
	}

	void CanvasModeAnnotation::mouseReleaseEventSpecific( QMouseEvent* event ) {
		if ( receiver->isAnnotationActive()) {
			receiver->storeActiveAnnotation( event );
		} else {
			receiver->selectAnnotation(event, false );
		}
		dragEnabled = false;
	}

	QString CanvasModeAnnotation::toString() const{
		return "Annotation";
	}

	CanvasModeAnnotation::~CanvasModeAnnotation() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
