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
#ifndef DISPLAY_CLEANGUI_H_
#define DISPLAY_CLEANGUI_H_

#include <QDialog>
#include <display/Clean/CleanGui.ui.h>

#if 0
#include <QProgressDialog>
#include <QFileDialog>
#include <casa/aipstype.h>
#include <display/Fit/FindSourcesDialog.qo.h>
#include <display/Fit/PixelRangeDialog.qo.h>
#include <display/Fit/ResidualHistogramDialog.qo.h>
#include <display/Fit/Fit2DLogDialog.qo.h>
#include <display/region/QtRegionSource.qo.h>
#endif

namespace casa {

	class ImagerControl;

	namespace viewer {

		class CleanGui : public QDialog, private Ui::CleanGui {
			Q_OBJECT
			public:
				CleanGui( QWidget *parent = 0 );
				virtual ~CleanGui( );

			signals:

			public slots:

			private slots:
				void selection_change( );
				void check_box_change( QTreeWidgetItem*, int );

				void play_button_event( );
				void pause_button_event( );
				void stop_button_event( );
				void refresh_button_event( );

			private:
				enum clean_state_t { UNDEFINED, PAUSED, RUNNING, STOPPED };
				// this class is not intended for copy or assignment...
				CleanGui( const CleanGui & );
				CleanGui operator=( const CleanGui & );

				// update information for the process currently selected...
				void refresh( );

				ImagerControl *ic;
				int current_process_index;
				clean_state_t current_process_state;
		};
	}
}
#endif
