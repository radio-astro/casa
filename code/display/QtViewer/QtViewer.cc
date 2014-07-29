//# QtViewer.cc: Qt implementation of main viewer supervisory object
//#		 -- Gui level.
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
//# $Id$
#include <display/QtViewer/QtDBusViewerAdaptor.qo.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtCleanPanelGui.qo.h>


extern int qInitResources_QtViewer();

namespace casa { //# NAMESPACE CASA - BEGIN


	QString QtViewer::name_;

	const QString &QtViewer::name( ) {
		return name_;
	}

	QtViewer::QtViewer( const std::list<std::string> &args, bool is_server, const char *dbus_name ) :
		QtViewerBase(is_server), dbus_(NULL), args_(args), is_server_(is_server) {

		name_ = (is_server_ ? "view_server" : "viewer");
		dbus_name_ = (dbus_name ? dbus_name : 0);

		qInitResources_QtViewer();
		// Makes QtViewer icons, etc. available via Qt resource system.
		//
		// You would normally use this macro for the purpose instead:
		//
		//   Q_INIT_RESOURCE(QtViewer);
		//
		// It translates as:
		//
		//   extern int qInitResources_QtViewer();
		//   qInitResources_QtViewer();
		//
		// It doesn't work here because it makes the linker looks for
		//   casa::qInitResources_QtViewer()     :-)   dk

		if ( is_server_ ) {
			dbus_ = new QtDBusViewerAdaptor(this);
			dbus_->connectToDBus(dbus_name_);
		} else {
			dbus_ = 0;
		}
	}


	QtViewer::~QtViewer() {
		// wonder if we need to delete dbus adaptor...
	}

	QtDisplayPanelGui *QtViewer::createDPG() {
		// Create a main display panel Gui.
		//
		QtDisplayPanelGui* dpg = new QtDisplayPanelGui(this,0,"dpg",args_);
		// don't need to worry about QtCleanPanelGui objs because they
		// do not need to support cursor tracking and the P/V tool...
		// although, there is nothing preventing QtCleanPanelGui objs
		// being added to this list...
		panels.push_back(dpg);

		// Previously casaviewer.cc created a QtDisplayPanelGui directly,
		// now it uses this function to ensure consistent behavior with
		// creation of QtDisplayPanelGui objects...
		if ( is_server_ ) {

			dpg->setAttribute(Qt::WA_DeleteOnClose);
			// Deletes this window (only) when user closes it.
			// (Note that 'closed' Qt windows are not deleted by default --
			// not unless this attribute is explicitly set).
			//
			// Noone will hold the dpg pointer (! -- at least in the current
			// revision).  Essentially, the gui user manages dpg's storage....
			//
			// Note, however: DPGs do not have to be created via this routine.
			// QtClean, e.g., constructs its own dpg directly, which does
			// not delete on close, and which is re-opened on successive
			// restarts of the Qt event loop.  In that case, QtClean manages
			// its own dpg storage.

			dpg->show();
		}

		return dpg;
	}

	QtDisplayPanelGui *QtViewer::createDPG(const QtDisplayPanelGui *other) {
		// Create a main display panel Gui.
		//
		QtDisplayPanelGui* dpg = new QtDisplayPanelGui(other,0,"dpg",args_);
		// don't need to worry about QtCleanPanelGui objs because they
		// do not need to support cursor tracking and the P/V tool...
		// although, there is nothing preventing QtCleanPanelGui objs
		// being added to this list...
		panels.push_back(dpg);

		// Previously casaviewer.cc created a QtDisplayPanelGui directly,
		// now it uses this function to ensure consistent behavior with
		// creation of QtDisplayPanelGui objects...
		if ( is_server_ ) {

			dpg->setAttribute(Qt::WA_DeleteOnClose);
			// Deletes this window (only) when user closes it.
			// (Note that 'closed' Qt windows are not deleted by default --
			// not unless this attribute is explicitly set).
			//
			// Noone will hold the dpg pointer (! -- at least in the current
			// revision).  Essentially, the gui user manages dpg's storage....
			//
			// Note, however: DPGs do not have to be created via this routine.
			// QtClean, e.g., constructs its own dpg directly, which does
			// not delete on close, and which is re-opened on successive
			// restarts of the Qt event loop.  In that case, QtClean manages
			// its own dpg storage.

			dpg->show();
		}

		return dpg;
	}

	void QtViewer::dpgDeleted( QtDisplayPanelGui *dpg ) {
		panel_list_t::iterator iter = std::find(panels.begin( ), panels.end( ), dpg );
		if ( iter != panels.end( ) ) panels.erase(iter);
	}

	QtCleanPanelGui *QtViewer::createInteractiveCleanGui( ) {
		QtCleanPanelGui* cpg = new QtCleanPanelGui(this,0,args_);
		return cpg;
	}

	void QtViewer::activate( bool state ) {
		for ( panel_list_t::iterator iter = panels.begin( ); iter != panels.end( ); ++iter )
			(*iter)->activate( state );
	}

	void QtViewer::quit() {
		QtViewerBase::quit();
	}


} //# NAMESPACE CASA - END
