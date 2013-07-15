//# SlicerGen.qo.h: GUI which provides a Slicer object
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
//# $Id$


#ifndef DISPLAY_SLICERGEN_H_
#define DISPLAY_SLICERGEN_H_

#include <display/QtViewer/SlicerGen.ui.h>
#include <casa/Arrays/Vector.h>

namespace casa {
	namespace viewer {

		class SlicerGen : public QObject, private Ui::SlicerGen {
			Q_OBJECT
		public:
			SlicerGen( );
			void initialize( QFrame * );
			void disable( );
			void enable( const std::string &path );
			bool sliceReady( ) const;
			std::string getSliceRep( ) const;

        signals:
            void stateChange(bool);

		private:
			Vector<int> shape;
			std::vector<QSpinBox*> startgui;
			void load_startgui( );
			std::vector<QSpinBox*> sizegui;
			void load_sizegui( );
			typedef std::map<QSpinBox*,int> spin_map_t;
			spin_map_t axis_map;

            // when setting up slicer don't generate events...
            bool drop_events;

		private slots:
			void adjust_position(int);
			void adjust_size(int);

		};
	}
}

#endif
