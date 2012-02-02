//# QtRegionDock.qo.h: dockable Qt implementation of viewer region management
//# Copyright (C) 2011
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

#ifndef REGION_QTREGIONDOCK_H_
#define REGION_QTREGIONDOCK_H_
#include <iostream>
#include <display/region/QtRegionDock.ui.h>
#include <imageanalysis/Annotations/AnnRegion.h>
#include <imageanalysis/Annotations/RegionTextList.h>

namespace casa {
    namespace viewer {

	class QtRegionState;

	class QtRegionDock : public QDockWidget, protected Ui::QtRegionDock {
	    Q_OBJECT
	    public:

		QtRegionDock( QWidget* parent=0 );
		~QtRegionDock();

		void addRegion(QtRegionState*,int index = -1);
		int indexOf(QtRegionState*) const;
		void removeRegion(QtRegionState*);
		void selectRegion(QtRegionState*);

		/* QStackedWidget *regionStack( ) { return regions; } */

		/* void showStats( const QString &stats ); */

	    signals:
		void deleteRegion(QtRegionState*);
		void outputRegions( std::list<QtRegionState*>, RegionTextList & );

	    private slots:
		void stack_changed(int);
		void change_stack(int);
		void delete_current_region(bool);
		void output_region_event(const QString &what, const QString &where);
	};
    }
}

#endif
