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

#ifndef CANVASMODE_H_
#define CANVASMODE_H_

#include <QCursor>

class QMouseEvent;

namespace casa {

	class QtCanvas;

	class CanvasMode {

	public:
		enum ModeIndex {MODE_ANNOTATION, MODE_CHANNEL, MODE_CONTEXTMENU, MODE_RANGESELECTION, MODE_ZOOM };
		void setReceiver( QtCanvas* receiver );
		virtual bool isMode( ModeIndex mode ) const;
		void mousePressEvent( QMouseEvent* event );
		virtual void mousePressEventSpecific( QMouseEvent* event ) = 0;
		virtual void mouseMoveEvent( QMouseEvent* event ) = 0;
		void mouseReleaseEvent( QMouseEvent* event );
		virtual void mouseReleaseEventSpecific( QMouseEvent* event ) = 0;
		virtual QString toString() const = 0;
		virtual ~CanvasMode();

	protected:
		CanvasMode( Qt::CursorShape modeCursor );
		QtCanvas* receiver;
		Qt::CursorShape cursor;
	};

} /* namespace casa */
#endif /* CANVASMODE_H_ */
