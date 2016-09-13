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
#ifndef QTVIEWER_CURSORTRACKINGHOLDER_H_
#define QTVIEWER_CURSORTRACKINGHOLDER_H_

#include <QtGui/QWidget>
#include <display/QtViewer/InActiveDock.qo.h>
#include <display/QtViewer/CursorTrackingHolder.ui.h>
#include <vector>

namespace casa {

    class QtDisplayPanelGui;
    class QtDisplayData;
    class PrincipalAxesDD;
    class Record;
    class TrackBox;

	/**
	 * Dock widget which manages the cursor tracking dialogs...
	 */
    class CursorTrackingHolder : public InActiveDock, protected Ui::CursorTrackingHolder {
		Q_OBJECT

	public:
		CursorTrackingHolder( QtDisplayPanelGui *panel, QWidget *parent = 0);
		~CursorTrackingHolder( );

        void arrangeTrackBoxes( );
        TrackBox *addTrackBox( QtDisplayData *, int position = -1 );
        void removeTrackBox( QtDisplayData * );
        void display( const Record &trackingRec );
        void cursorUpdate( const std::vector<double> &, QtDisplayData * );

        QSize sizeHint( ) const { return find_size( ); }
        QSize minimumSizeHint( ) const { return find_size( ); }

		void dismiss( );

		//Overriden because if the cursor tracker was not shown, images were loaded,
		//and then the cursor tracker was shown, it was not resizing properly.
		void setVisible( bool visible );

	protected:
		void closeEvent ( QCloseEvent * event );

    private slots:
        void handle_folding( bool, QtDisplayData * );
		void handle_visibility(bool);

    private:
        QSize find_size( ) const;
        void update_size( );
        QtDisplayPanelGui *panel_;

		bool dismissed;
	};
}
#endif
