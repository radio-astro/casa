//# QtDataManager.qo.h: Qt implementation of viewer data manager widget.
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

#ifndef QTEXPORTMANAGER_H_
#define QTEXPORTMANAGER_H_
// (Trailing underscore is not a typo -- do not remove it;
// QtDataManager.ui.h uses the QTDATAMANAGER_H symbol).


#include <casa/aips.h>
#include <casa/BasicSL/String.h>
//#include <display/QtViewer/QtLELEdit.qo.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
//#dk Be careful to put *.ui.h within X_enter/exit bracket too,
//#   because they'll have Qt includes.
//#   E.g. <QApplication> needs the X11 definition of 'Display'
#include <display/QtViewer/QtExportManager.ui.h>
//#include <display/QtViewer/QtDataMgrMsSelect.ui.h>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN

	class QtDisplayPanelGui;
	class QtDisplayPanel;
	class QtDisplayData;
	template <class T> class ImageInterface;

	class QtExportManager : public QWidget, private Ui::QtExportManager {

		Q_OBJECT

	public:

		QtExportManager(QtDisplayPanelGui* panel=0, const char* name=0,
		                QWidget* parent=0 );
		~QtExportManager();

	public slots:
		void updateEM(QtDisplayData* qdd=0, Bool autoRegister=True);

	protected:
		void fillFormatBox(int type);
		QColor getDirColor(int);
		QString getOutFileName(QString &filter);
		QString getOutDirName(QString &);
		QString getOutDirNameOLD(QString &filter);

		enum DATATYPE { UNKNOWN, IMAGE, MEASUREMENT_SET, SKY_CATALOG, RESTORE,
		                DIRECTORY, QUALIMG, CASAREGION, DS9REGION
		              };
		enum DISPLAYTYPE { RASTER, CONTOUR, VECTOR, MARKER, SKY_CAT,
		                   NEWPANEL, OLDPANEL
		                 };

		QHash<int, String>  dataType_;
		QHash<QString, int> uiDataType_;
		QHash<int, String>  displayType_;
		QHash<QString, int> uiDisplayType_;
		QHash<int, QtDisplayData*> qddList_;
		QVector<int>        exportTypes_;

	protected slots:
		void changeItemSelection();
		void exportData();
		void buildFileList();
		void browseOutFilePath();
		void browseFileSelection();
		void browseFileSelectionOLD();

		void showDDCreateError_(String);


	private:
		void expImageInterfaceToFITS(ImageInterface<Float>* img, String outFile);
		void getSectralCoordFlags(const ImageInterface<Float>* img, Bool &preferVelocity, Bool &opticalVelocity,
		                          Bool &preferWavelength, Bool &preferAirWavelength);
		void expImageInterfaceToCASA(ImageInterface<Float>* img, String outFile);
		//void expImageInterfaceToMIRIAD(ImageInterface<Float>* img, String outFile);
		void messageFromEM(QString &msg);

		QWidget *parent_;
		QtDisplayPanelGui* panel_;
		QDir dir_;
		QTreeWidgetItem *selItem_;
		int saveType_;
	};



} //# NAMESPACE CASA - END

#endif
