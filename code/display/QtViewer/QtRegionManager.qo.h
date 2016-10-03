//# QtRegionManager.qo.h: Qt implementation of viewer region manager window.
//# (This is the gui part only; it connects to region functions within
//# QtDisplayPanel).
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

#ifndef QTREGIONMANAGER_H
#define QTREGIONMANAGER_H

#include <casa/aips.h>
#include <casa/Containers/List.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/DisplayDatas/DisplayData.h>

#include <graphics/X11/X_enter.h>
#include <QtCore>
#include <QtGui>
#include <QHash>
//#dk Be careful to put *.ui.h within X_enter/exit bracket too,
//#   because they'll have Qt includes.
//#   E.g. <QApplication> needs the X11 definition of 'Display'
#include <display/QtViewer/QtRegionMgr.ui.h>
#include <graphics/X11/X_exit.h>


namespace casacore{

	class ImageRegion;
	class Record;
	class WCUnion;
	template <class T> class PtrBlock;
}

namespace casa { //# NAMESPACE CASA - BEGIN

	class QtViewer;
	class RSComposite;


	class QtRegionManager : public QWidget, protected Ui::QtRegionMgr {

		Q_OBJECT


	public:

		QtRegionManager(QtDisplayPanel* qdp, QWidget* parent=0);

		~QtRegionManager() {  }

	public slots:
		void changeAxis(casacore::String, casacore::String, casacore::String);
		void activate(casacore::Record);

	protected slots:
		//draw region on viewer
		void drawRegion(casacore::Record mousereg, WorldCanvasHolder *wch);
		// React to new region creation in display panel.
		void newRegion_(casacore::String imgFilename);

		// Load region from current displayed image
		void loadRegionFromImage();
		// Load region from ds9 or aipsbox or rgn file
		void loadRegionFromFile();

		//save region into image
		void saveRegionInImage();
		// react to SaveRgnn, save region to file
		void saveRegionInFile();
		//remove region from image
		void removeRegion();

		void toggleImageRegion();
		void zPlaneChanged();
		void currentRegionChanged(const QString &);
		void showHelp();
		void showHelpActive();

		//convert region to shape
		RSComposite *regionToShape(
		    QtDisplayData* qdd, const casacore::ImageRegion* wcreg);

		// Cleanup on destruction
		void cleanup();

		void deleteActiveBox();
		void insertActiveBox();

		// set up plane only or extending by channel and pol
		void singlePlane();
		void extendChan();
		void extendPol();

		void resetRegionExtension();

		void loadRegionsImageFromFile();
		DisplayData* getImageData(QString);
		DisplayData* getBoundingBoxData(QString);

		//delete region from image
		void deleteRegion();
		//show/hide region
		void showHideRegion();

		void flashActive();
		void exportRegions();

	public:
		bool planeAllowed(int, casacore::String&, casacore::String&);

	protected:
		void addRegionsToShape(RSComposite*& theShapes,
		                       const casacore::WCRegion*& wcreg);
		casacore::WCUnion* unfoldCompositeRegionToSimpleUnion(const casacore::WCRegion*& wcreg);
		void displaySelectedRegion();
		void showRegion(const casacore::String& regName);

		bool deleteBox(QString&, int);
		bool insertBox(QString&);
		void rotateBox(int);

		void addRegionToMenu(const QString&, const QString&);

		QtDisplayPanel* qdp_;
		QWidget* parent_;

	private:
		casacore::PtrBlock<const casacore::ImageRegion * >  unionRegions_p;
		casacore::List<RegionShape*> regShapes_p;
		void unfoldIntoSimpleRegionPtrs(casacore::PtrBlock<const casacore::WCRegion*>& outRegPtrs, const casacore::WCRegion*& wcreg);
		QHash<QString, DisplayData*> regData;
		QHash<QString, bool> regState;
		QMenu *showHideMenu;
		QMenu *deleteMenu;

		QString activeGroup;
		int activeBox;
		RegionShape* activeShape;
		QTimer* timer;
		bool flash;

		int cb;

	signals:
		void extendRegion(casacore::String, casacore::String);

	};


} //# NAMESPACE CASA - END

#endif
