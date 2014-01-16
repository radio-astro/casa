//# QtViewerBase.qo.h: Qt implementation of main viewer supervisory object.
//#                 -- Functional level.
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

#ifndef QTVIEWERBASE_H
#define QTVIEWERBASE_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <display/QtViewer/QtMouseToolState.qo.h>
#include <casa/Arrays/Vector.h>

#include <graphics/X11/X_enter.h>
#  include <QObject>
#  include <QtXml/QDomDocument>
#include <graphics/X11/X_exit.h>


// <summary>
// Qt implementation of main viewer supervisory object -- Functional level.
// </summary>

// <synopsis>
// The viewer is structured with a functional layer and a gui layer.
// In principle the former can operate without the latter.  This class
// manages functional objects associated with the viewer, in particular
// the list of user-created DDs.
// </synopsis>

namespace casa { //# NAMESPACE CASA - BEGIN

	class String;
	class QtDisplayData;
	class QtDisplayPanel;
	class QtDisplayPanelGui;

	class QtViewerBase : public QObject {

		Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

	public:

		QtViewerBase( bool is_server=false );
		~QtViewerBase();

		bool server( ) const {
			return server_;
		}

		// Return the common QtMouseToolState that all mouse tool users
		// should share.
		virtual QtMouseToolState* mouseBtns() {
			return &msbtns_;
		}
		virtual const QtMouseToolState* mouseBtns() const {
			return &msbtns_;
		}

		// The list of QtDisplayPanels that are not closed.
		virtual std::list<QtDisplayPanelGui*> openDPs();

		// The number of open QtDisplayPanels.  (More efficient than the
		// equivalent openDPs().len(), if the number is all that's needed).
		virtual Int nOpenDPs();

		// Only to be used by QtDisplayPanels, to inform this class of
		// their creation.  (C++ should allow individual methods to be
		// 'friend'ed to outside classes...).
		virtual void dpCreated( QtDisplayPanelGui *newDP, QtDisplayPanel *panel );

		// function to aid typing files of interest to the viewer.
		// Moved from QtDataManager to be available for non-gui use.
		// Returns user interface Strings like "Measurement Set".
		static String fileType(const String pathname);

		// similar to above; returns internal DD datatype names like "ms"
		// (or else 'nonexistent', 'unknown').
		static String filetype(const String pathname);

		// Does the given file pathname hold a readable file with valid ID and
		// form as a viewer restore xml document?  (If so, contents are set
		// onto restoredoc).
		static Bool isRestoreFile(String filename, QDomDocument& restoredoc);

		// Does the given String have valid ID and form as viewer restore xml?
		// (If so, contents are set onto restoredoc).
		static Bool isRestoreString(String xmlState, QDomDocument& restoredoc);

		static const String cvRestoreID;		//# internal identifier,
		static const String cvRestoreFileExt;		//# default file extension,
		//# for save-restore

		// Utility routines to convert between Vector<Float> and String.
		//<group>
		static String toString(Vector<Float> values);
		static Vector<Float> toVectorF(String values, Bool* ok=0);
		static String toString(Vector<Double> values);
		static Vector<Double> toVectorD(String values, Bool* ok=0);
		//</group>


		// Returns True iff datatype is a vaild viewer datatype and
		// displaytype is valid for the datatype.  If the former is true
		// but the latter isn't, displaytype is [re]set to the default
		// displaytype for the datatype.
		Bool dataDisplaysAs(String datatype, String& displaytype);

		// public (const) data.

		// viewer datatypess.
		static const Int IMAGE=1, MEASUREMENT_SET=2, SKY_CATALOG=3,
		                 RESTORE=4, LEL=5,                           N_DT=5;

		// viewer displaytypes.
		static const Int RASTER=1, CONTOUR=2, VECTOR=3, MARKER=4,
		                 SKY_CAT=5, OLDPANEL=6, NEWPANEL=7,          N_DS=7;

		// (for invalid datatype or displaytype).
		static const Int INVALID=0;


		bool exiting( ) {
			return qtviewer_app_exit;
		}

	public slots:

		// Hold/release of (canvas-draw) refresh of all QDPs.  (NB: does not
		// concern enabling of Qt Widgets).  Call to hold() must be matched to
		// later call of release(); they can be nested.  It is sometimes
		// efficient to wait until several changes are complete and then
		// redraw everything just once.
		//<group>
		virtual void hold();
		virtual void release();
		//</group>

		// Closes all open panels, which will exit the Qt loop.  Note that the
		// loop can be restarted (and is, in interactive clean, e.g.), with
		// existing widgets intact.  This does not in itself delete objects
		// or exit the process, although the driver program might do that.
		// Also, some of the panels may have WA_DeleteOnClose set, which
		// would cause their deletion (see, e.g., QtViewer::createDPG()).
		virtual void quit();

	protected slots:

		// Connected by this class, (only) to QDPs' destroyed() signals
		// (for maintenance of the list of existing QDPs).
		virtual void dpDestroyed_(QObject*);

		// Connected by this class, (only) to QDPs' dpHidden() signals
		// (for checking on open QDPs -- will quit if none are left open).
		virtual void dpHidden_(QtDisplayPanel*);



	protected:

		bool server_;

		// Existing QtDisplayPanels (whether 'closed' or not, as long as
		// not deleted).  Unlike QDDs, QtViewerBase does not create or delete
		// QDPs, it just tries to keep track of them.  Note that by default,
		// 'closed' DPs are simply not 'visible' (see QWidget::isVisible()), though
		// they are not deleted unless their owner does it.  An owner can also
		// revive (show) a 'closed' QDP.  The publicly-available list openDPs()
		// shows only those which are not closed.  Minimized or covered
		// DPs are still considered 'open'.
		std::list<QtDisplayPanelGui*> qdps_;


		// This should be the only place this object is ever created....
		// Holds mouse button assignment for the mouse tools, which is to
		// be the same on all mouse toolbars / display panels.
		QtMouseToolState msbtns_;

		// Translates IMAGE, RASTER, etc. into the names used internally
		// (e.g. "image", "raster").
		Vector<String> datatypeNames_, displaytypeNames_;

		// e.g. dataDisplaysAs_[IMAGE] will be {RASTER, CONTOUR, VECTOR, MARKER}
		// dataDisplaysAs_[datatype][0] will be the default displaytype for
		// that datatype.
		Vector<Vector<Int> > dataDisplaysAs_;

	private:
		static bool qtviewer_app_exit;

	};



} //# NAMESPACE CASA - END

#endif
