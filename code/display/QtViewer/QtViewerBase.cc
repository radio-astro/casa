//# QtViewerBase.cc: Qt implementation of main viewer supervisory object
//#                  -- Functional level.
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

#include <display/QtViewer/QtViewerBase.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtApp.h>
#include <tables/Tables/TableInfo.h>
#include <display/QtAutoGui/QtXmlRecord.h>
#include <casa/iomanip.h>

#include <graphics/X11/X_enter.h>
#  include <QFileInfo>
#  include <QFile>
#  include <QString>
#include <graphics/X11/X_exit.h>



namespace casa { //# NAMESPACE CASA - BEGIN


	QtViewerBase::QtViewerBase( bool is_server) :
		server_(is_server), qdps_(), msbtns_(),   datatypeNames_(N_DT+1),
		displaytypeNames_(N_DS+1), dataDisplaysAs_(N_DT+1) {

		// Initialize some (conceptually constant) data for datatype and
		// displaytype names, and for displaytypes which are valid for a
		// given displaytype.

		datatypeNames_[INVALID] =          "";
		datatypeNames_[IMAGE] =            "image";
		datatypeNames_[MEASUREMENT_SET] =  "ms";
		datatypeNames_[SKY_CATALOG] =      "skycatalog";
		datatypeNames_[RESTORE] =          "restore";
		datatypeNames_[LEL] =              "lel";

		displaytypeNames_[INVALID] =   "";
		displaytypeNames_[RASTER] =    "raster";
		displaytypeNames_[CONTOUR] =   "contour";
		displaytypeNames_[VECTOR] =    "vector";
		displaytypeNames_[MARKER] =    "marker";
		displaytypeNames_[SKY_CAT] =   "skycatalog";
		displaytypeNames_[OLDPANEL] =  "oldpanel";
		displaytypeNames_[NEWPANEL] =  "newpanel";

		Vector<Int> dstypes;	// (temp for the various vectors below)

		dstypes.resize(4);
		dstypes[0] = RASTER;
		dstypes[1] = CONTOUR;
		dstypes[2] = VECTOR;
		dstypes[3] = MARKER;
		dataDisplaysAs_[IMAGE] = dstypes;		// image displaytypes

		dataDisplaysAs_[LEL] = dstypes;		// lel displaytypes

		dstypes.resize(1, True);
		dataDisplaysAs_[MEASUREMENT_SET] = dstypes;	// ms displaytypes

		dstypes[0] = SKY_CAT;
		dataDisplaysAs_[SKY_CATALOG] = dstypes;	// skycatalog displaytypes

		dstypes.resize(2);
		dstypes[0] = OLDPANEL;
		dstypes[1] = NEWPANEL;
		dataDisplaysAs_[RESTORE] = dstypes;		// restore file displaytypes

	}


	QtViewerBase::~QtViewerBase() { }


	void QtViewerBase::dpCreated( QtDisplayPanelGui *newDP, QtDisplayPanel *panel ) {
		// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
		//    In the future (call sequence) newDP->displayPanel() will equal
		//    panel, but because this function is called while constructing
		//    newDP, both newDP and the panel must be passed in...
		// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
		// Only to be used by QtDisplayPanels, to inform this class of
		// their creation.
		qdps_.push_back(newDP);
		connect( newDP, SIGNAL(destroyed(QObject*)), SLOT(dpDestroyed_(QObject*)) );
		// (In case it's ever deleted).
		connect( panel, SIGNAL(dpHidden(QtDisplayPanel*)), SLOT(dpHidden_(QtDisplayPanel*)), Qt::QueuedConnection );
		// (This will happen if the newDP is closed, even if not deleted.
		// Queued connection may not be necessary, but may be cleaner:
		// dpHidden_() might lead to viewer exit).
	}


	void QtViewerBase::dpDestroyed_(QObject* dying) {
		// Connected by this class, (only) to QDPs' destroyed() signals
		// (for maintenance of the list of existing QDPs).
		// (destroyed() is emitted just _before_ the QDP is deleted).
		std::list<QtDisplayPanelGui*>::iterator iter = std::find(qdps_.begin(),qdps_.end(),static_cast<QtDisplayPanelGui*>(dying));
		if ( iter != qdps_.end( ) ) qdps_.erase(iter);
	}


	void QtViewerBase::dpHidden_(QtDisplayPanel* /*qdp*/) {
		// Connected by this class, (only) to QDPs' dpHidden() signals
		// (for checking on open QDPs).
		//
		// If the dp's window (i.e. the QDPGui) is iconified or the user changes
		// 'virtual desktops', the qdp _does_ receive a QHideEvent, triggering
		// this routine, but qdp->isVisible() _will remain True_ (see isVisible()
		// doc).  This is somewhat counterintuitive, but useful.  In this context,
		// dpHidden_ (hideEvent received) with !qdp->isVisible() indicates that a
		// qdp's window has been _closed_ (not just minimized, or otherwise
		// obscured).
		//
		// NB:  There is no closeEvent per se.  A hideEvent where isVisible()
		// is False is probably the closest thing there is to a 'closeEvent',
		// although it can also occur if the widget or parent is hidden
		// programmatically (i.e., this will probably all break the minute
		// someone wants to explicitly hide() a QDP, without 'closing' it....
		// Also note: qdp->isVisible() is False just after construction,
		// until qdp (or its parent window) is 'shown'.).
		//
		// Also note that we test !qdp->isVisible(), not dp->isHidden(); they
		// are not equivalent.  The former, but not the latter, will also be true
		// if an enclosing widget (typically QDPG) has just been closed; we want
		// to react to that as well in this context.
		//
		// Also: even 'closed' does not _necessarily_ mean the panel is
		// being deleted, or that it will not be reactivated (shown) later.
		//
		//
		// --> This routine will result in viewer 'quit' (exit of Qt loop)
		// if no QDPs are left open. <--
		//
		//
		// Even that does not necessarily mean the program will exit, however.
		// casaviewer usually does exit; but in, e.g. interactive clean, the
		// Qt loop may be restarted (and a 'closed' but undeleted display panel
		// may be shown again).



		// Quit when the last display panel is closed.

		if ( nOpenDPs() == 0 && ! server_ ) quit();	// ('open' tested via isVisible() here too...)
		// 'quit' closes (hides) all viewer windows, which results in
		// an an exit from the Qt loop, deactivating all Gui response.
		// It does not in itself delete objects or exit the process,
		// although the driver program might well do that.  (Also, some
		// of the display panels may have WA_DeleteOnClose set, which
		// would  cause their deletion -- see, e.g., QtViewer::createDPG()).
		// Alternatively, the loop might be restarted later with
		// existing widgets intact (e.g. in interactive clean).

// <drs> - why would this be done... removing display data because the user hides the viewer?
//else if(!qdp->isVisible()) qdp->unregisterAll();
		// Remove DDs from a closed panel.
		// Note: This is _not_ done if we are 'quitting', which may
		// mean we will want to restart the Qt loop later (with the
		// same panel in the same state...).
	}


	std::list<QtDisplayPanelGui*> QtViewerBase::openDPs() {
		std::list<QtDisplayPanelGui*> result;
		// The list of QtDisplayPanels that are not closed.
		for ( std::list<QtDisplayPanelGui*>::iterator iter = qdps_.begin( ); iter != qdps_.end( ); ++iter ) {
			if ( (*iter)->isVisible( ) ) result.push_back(*iter);
		}
		return result;
	}


	Int QtViewerBase::nOpenDPs() {
		Int result = 0;
		// The number of open QtDisplayPanels.
		for ( std::list<QtDisplayPanelGui*>::iterator iter = qdps_.begin( ); iter != qdps_.end( ); ++iter ) {
			if ( (*iter)->isVisible( ) ) ++result;
		}
		return result;
	}


	void QtViewerBase::hold() {
		// Hold of (canvas-draw) refresh of all QDPs.  (NB: does not
		// concern enabling of Qt Widgets).  Call to hold() must be matched to
		// later call of release().  It is sometimes efficient to wait until
		// several operations are complete and then redraw everything just once.
		for ( std::list<QtDisplayPanelGui*>::iterator iter = qdps_.begin( ); iter != qdps_.end( ); ++iter ) {
			(*iter)->displayPanel( )->hold( );
		}
	}


	void QtViewerBase::release() {
		for ( std::list<QtDisplayPanelGui*>::iterator iter = qdps_.begin( ); iter != qdps_.end( ); ++iter ) {
			(*iter)->displayPanel( )->release( );
		}
	}


	String QtViewerBase::fileType(const String pathname) {
		// (static) function to aid typing files of interest to the viewer.
		// Moved from QtDataManager to be available for non-gui use.

		QDomDocument restoredoc;
		if(isRestoreFile(pathname, restoredoc)) return "Restore File";

		QString pathName = pathname.chars();
		QFileInfo fileInfo(pathName);


		if (!fileInfo.isFile() && !fileInfo.isDir()) {
			// try to cut any possible FITS extension specified
			// as part of the input
			if ( ! (int) pathname.compare(pathname.length()-1, 1, "]", 1) &&
			        (int) pathname.rfind("[", pathname.length()) > -1 ) {
				pathName = String(pathname, 0, pathname.rfind("[", pathname.length())).chars();
				fileInfo = QFileInfo(pathName);
			}
		}

		if (fileInfo.isFile()) {

			QFile file(pathName);
			if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				char buf[10240];
				qint64 lineLength = file.readLine(buf, sizeof(buf));
				if (lineLength > 0) {
					QString line(buf);
					if ( line.startsWith("#CRTF") ) {
						return "CASA Region File";
					}
					if ( line.startsWith("# Region file format: DS9") ) {
						return "DS9 Region File";
					}
					if ( line.remove(' ').contains("SIMPLE=T") ) {
						return "FITS Image";
					}
				}
			}

			if ( pathName.endsWith(".crtf") )
				return "CASA Region File";
			if ( pathName.endsWith(".reg") )
				return "DS9 Region File";

		} else if (fileInfo.isDir()) {	// Directory

			QFileInfo tab(pathName + "/table.dat");

			if (tab.isFile ())  {

				try {				// Table

					TableInfo tblinfo = TableInfo(pathName.toStdString()+"/table.info");

					QString result = tblinfo.type().chars();
					if (result =="IERS" || result =="IERSe" || result =="Skycatalog") {

						return "Sky Catalog";
					}

					if (result.simplified()=="")  return "Table";

					return result.toStdString();

				} catch (...)  {
					return "Bad Table";
				}

			} else {				// Non-Table Directory

				QFileInfo hd(pathName,  "header");
				QFileInfo imt(pathName +  "/image" );

				if (hd.isFile() && imt.exists())  return "Miriad Image";

				QFileInfo vis(pathName + "/visdata" );

				if (hd.isFile() && vis.exists())  return "Miriad Vis";


				return "Directory";
			}
		}


		if (fileInfo.isSymLink())  return "SymLink";

		if (!fileInfo.exists())  return "Invalid";

		return  "Unknown";

	}



	String QtViewerBase::filetype(const String pathname) {
		// similar to above; returns internal DD datatype names.

		String fTyp = fileType(pathname);

		if(fTyp=="Image" || fTyp=="FITS Image" ||
		        fTyp=="Miriad Image" || fTyp=="Gipsy") return "image";
		if(fTyp=="Measurement Set")               return "ms";
		if(fTyp=="Sky Catalog")                   return "skycatalog";
		if(fTyp=="Restore File")                  return "restore";

		if(fTyp=="Invalid")                       return "nonexistent";

		return "unknown";
	}



// SAVE-RESTORE METHODS




	Bool QtViewerBase::isRestoreFile(String filename, QDomDocument& restoredoc) {
		// Does the given file pathname hold a readable file with valid ID and
		// form as viewer restore xml?  (If so, contents are set onto restoredoc).

		QFile fl(filename.chars());
		QFileInfo fi(filename.chars());
		if(!fl.exists() || !fi.isFile()) return False;

		if(!fl.open(QIODevice::ReadOnly | QIODevice::Text)) return False;

		if(!restoredoc.setContent(&fl)) return False;

		QDomElement restoreElem = restoredoc.firstChildElement(cvRestoreID.chars());
		if(restoreElem.isNull()) return False;

		return True;
	}



	Bool QtViewerBase::isRestoreString(String xmlState,
	                                   QDomDocument& restoredoc) {
		// Does the given String have valid ID and form as viewer restore xml?
		// (If so, contents are set onto restoredoc).

		QtXmlRecord xr;
		if(!restoredoc.setContent(QString(xmlState.chars()))) return False;

		QDomElement restoreElem = restoredoc.firstChildElement(cvRestoreID.chars());
		if(restoreElem.isNull()) return False;

		return True;
	}


	const String QtViewerBase::cvRestoreID = "casaviewer-restore";
	const String QtViewerBase::cvRestoreFileExt = "rstr";
	// (constants used by save-restore).





// Utility routines to convert between Vector<Float> or Vector<Double>
// and String.


	String QtViewerBase::toString(Vector<Float> values) {
		ostringstream os;
		os << values;
		return String(os);
	}


	String QtViewerBase::toString(Vector<Double> values) {
		ostringstream os;
		os << setprecision(10) << values;
		return String(os);
	}


	Vector<Float> QtViewerBase::toVectorF(String values, Bool* ok) {
		// (If supplied, ok will return True iff values is a set of numeric values
		// (only), separated by (arbitrary) strings of the characters ", []").

		Bool oK = True;
		Vector<Float> vs;

		QRegExp rx("[,\\s\\[\\]]");
		QStringList vals = QString(values.chars()).
		                   split(rx, QString::SkipEmptyParts);
		// separate on commas, whitespace or brackets.
		Int sz = vals.size();
		vs.resize(sz);
		for(Int i=0; i<sz; i++) {
			Bool valid = True;
			vs[i] = vals.at(i).toFloat(&valid);
			oK = oK && valid;
		}	// Check that each value is numeric.

		if(ok!=0) *ok = oK;

		return vs;
	}



	Vector<Double> QtViewerBase::toVectorD(String values, Bool* ok) {

		Bool oK = True;
		Vector<Double> vs;

		QRegExp rx("[,\\s\\[\\]]");
		QStringList vals = QString(values.chars()).
		                   split(rx, QString::SkipEmptyParts);
		// separate on commas, whitespace or brackets.
		Int sz = vals.size();
		vs.resize(sz);
		for(Int i=0; i<sz; i++) {
			Bool valid = True;
			vs[i] = vals.at(i).toDouble(&valid);
			oK = oK && valid;
		}

		if(ok!=0) *ok = oK;

		return vs;
	}





	Bool QtViewerBase::dataDisplaysAs(String datatype, String& displaytype) {
		// Returns True iff datatype is a vaild viewer datatype and
		// displaytype is valid for the datatype.  If the former is true
		// but the latter isn't, displaytype is [re]set to the default
		// displaytype for the datatype.  (You can pass "" in for displaytype
		// to retrieve the default display type for datatype).

		for(Int dt=1; dt<=N_DT; dt++) if(datatypeNames_(dt)==datatype) {

				Vector<Int>& dstypes = dataDisplaysAs_[dt];
				Int ndst = dstypes.size();

				for(Int dst=0; dst<ndst; dst++) {

					if(displaytypeNames_[dstypes[dst]]==displaytype)  return True;
				}

				displaytype = displaytypeNames_[dstypes[0]];
				// displaytype was not valid for the datatype.
				// insert default display type into displaytype.
				return False;
			}

		return False;
	}



	bool QtViewerBase::qtviewer_app_exit = false;
	void QtViewerBase::quit() {
		// Close all open panels, which will exit Qt loop.  (Note that the
		// loop might be restarted (and is, in interactive clean, e.g.),
		// with existing widgets intact).  This does not in itself delete
		// objects or exit the process, although the driver program might
		// do that.  Also, some of the panels may have WA_DeleteOnClose set,
		// which would cause their deletion (see, e.g., QtViewer::createDPG()).
		qtviewer_app_exit = true;
		QtApp::app()->closeAllWindows();
	}


} //# NAMESPACE CASA - END
