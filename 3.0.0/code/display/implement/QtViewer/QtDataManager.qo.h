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

#ifndef QTDATAMANAGER_H_
#define QTDATAMANAGER_H_
	// (Trailing underscore is not a typo -- do not remove it;
	// QtDataManager.ui.h uses the QTDATAMANAGER_H symbol).


#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <display/QtViewer/QtLELEdit.qo.h>

#include <graphics/X11/X_enter.h>
#  include <QDir>
#  include <QColor>
#  include <QHash>
#  include <QWidget>
   //#dk Be careful to put *.ui.h within X_enter/exit bracket too,
   //#   because they'll have Qt includes.
   //#   E.g. <QApplication> needs the X11 definition of 'Display'
#  include <display/QtViewer/QtDataManager.ui.h>
#include <graphics/X11/X_exit.h>

 
namespace casa { //# NAMESPACE CASA - BEGIN

class QtViewer;
class QtDisplayPanel;


class QtDataManager : public QWidget, private Ui::QtDataManager {
   
   Q_OBJECT

 public:
  
  QtDataManager(QtViewer* viewer=0, const char* name=0,
		QWidget* parent=0 );
  ~QtDataManager();
  
  String path() const { return dir_.path().toStdString();  }
  
  void updateDirectory(const QString);
 
 signals:
 
  void tableReadErrorSignal(String msg);
  

 protected:

  void showDisplayButtons(int);
  void hideDisplayButtons();
  QColor getDirColor(int);

  enum DATATYPE {UNKNOWN, IMAGE, MEASUREMENT_SET, SKY_CATALOG, RESTORE,
                 DIRECTORY};
  enum DISPLAYTYPE {RASTER, CONTOUR, VECTOR, MARKER, SKY_CAT,
                    NEWPANEL, OLDPANEL};

  QHash<QString, int> dataType_;
  QHash<QString, int> uiDataType_;
  QHash<QString, int> displayType_;
  QHash<QString, int> uiDisplayType_;
 
 
 protected slots:
  
  void createButtonClicked();
  void showTools(bool show);
  void doubleClickItem();
  void clickItem(QTreeWidgetItem* item);
  void changeItemSelection();
  void returnPressed();
  void buildDirTree();
  void lelGotFocus_();
  
  // Restore viewer state to existing panel.
  // Use the first empty panel, or if none, the first panel.
  void restoreToOld_();
  
  // Create new display panel, restore viewer state to it.
  void restoreToNew_();

  // Restore viewer state to given panel.
  void restoreTo_(QtDisplayPanel* dp);
  
  
  void showDDCreateError_(String);

 
 private:
  
  QWidget *parent_;
  QtViewer* viewer_;
  QDir dir_;  
  
};



} //# NAMESPACE CASA - END

#endif
