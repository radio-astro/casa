//# IterationManager.qo.h: manage iteration of QtDisplayData lattices
//# with surrounding Gui functionality
//# Copyright (C) 2012
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

#ifndef QTITERATIONMANAGER_H
#define QTITERATIONMANAGER_H
#include <vector>
#include <QObject>

namespace casa {
    class IPosition;
    class QtDisplayData;
    class QtDisplayPanelGui;
    namespace viewer {
	class IterationClient;
	class IterationManager : public QObject {
	  Q_OBJECT;	//# Allows slot/signal definition.  Must only occur in
			//# implement/.../*.h files; also, makefile must include
			//# name of this file in 'mocs' section.

	    public:
		IterationManager( QtDisplayPanelGui *gui );
		~IterationManager( );

		void registerClient( IterationClient *c );
		void unregisterClient( IterationClient *c );

		std::vector<std::string> worldAxisNames( ) const;
		std::vector<std::string> worldAxisUnits( ) const;

		void locateFrame( unsigned int movie_axis, IPosition &start, IterationClient * );

	    private:
		bool refresh_needed;
		std::list<IterationClient*> clients;
		QtDisplayPanelGui *dpg;
	};
    }
}

#endif
