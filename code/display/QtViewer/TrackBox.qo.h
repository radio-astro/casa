//# TrackBox.qo.h: boxes used for cursor tracking for display data.
//# Copyright (C) 2013
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
//# $Id: QtDisplayPanelGui.qo.h,v 1.7 2006/10/10 21:42:05 dking Exp $

#ifndef QTVIEWER_TRACKBOX_H_
#define QTVIEWER_TRACKBOX_H_
#include <QGroupBox>
#include <QTextEdit>
#include <string>

namespace casa { //# NAMESPACE CASA - BEGIN

    class QtDisplayData;

    // <summary>
    // Helper class for QtDisplayPanelGui, for display of tracking information.
    // </summary>

    // <synopsis>
    // TrackBox is the widget for the position the tracking information of
    // a single QtDisplayData within a QtDisplayPanelGui.  trkgWidget_ will
    // show a TrackBox for each registered QDD capable of displaying tracking
    // information, in registration order.  TrackBox is simply a QGroupBox with
    // a QTextEdit inside it.  The QGroupBox displays the QDD's name and has
    // a checkbox that can be used to hide the tracking text area to save
    // space.  TrackBox is intended to be used exclusively by QtDisplayPanelGui.
    // </synopsis>

	class TrackBox : public QGroupBox {

		Q_OBJECT;

	public:

		TrackBox(QtDisplayData* qdd, QWidget* parent=0);
        virtual ~TrackBox( );

        void setText( std::string trkgString);
		void clear() {
			trkgEdit_->clear();
		}
		QtDisplayData* dd() {
			return qdd_;
		}
		std::string name() {
			return objectName().toStdString();
		}

        static int heightOpen( ) { return 75; }
        static int heightClosed( ) { return 25; }
        static int heightHeader( ) { return 30; }

    signals:
        void visibilityChange( bool visible, QtDisplayData * );

    private slots:
        void visibility_event( bool visible );

	protected:

		// Attempts automatic adjustment of tracking display height
		// according to contents.
		void setTrackingHeight_();

		QtDisplayData* qdd_;	// (the QDD whose tracking info it displays).
		QTextEdit*    trkgEdit_;	// (the box's tracking info display area).

	private:
		TrackBox() {  }		// (not intended for use)

	};

}
#endif
