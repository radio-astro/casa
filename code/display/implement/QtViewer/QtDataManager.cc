//# QtDataManager.cc: Qt implementation of viewer data manager widget.
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


#include <display/QtViewer/QtDataManager.qo.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableInfo.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/File.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/Exceptions/Error.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QMessageBox>
#include <QDirModel>
#include <QHash>
#include <QSettings>
#include <QTextStream>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN



QtDataManager::QtDataManager(QtViewer* viewer,
			     const char *name,
			     QWidget *parent ) :
	       QWidget(parent),
               parent_(parent),
	       viewer_(viewer) {
  
  setWindowTitle(name);
  
  setupUi(this);
  
  toolButton_->setCheckable(true);
  toolButton_->setChecked(false);
  showTools(false);
  toolButton_->hide();	//#dk (shouldn't show until 'tools' exist).
  
  //updateButton_->setEnabled(false);	//#dk until this works.

  lelEdit_->setToolTip("Enter an image expression, such as\n"
		       "'clean.im' - 'dirty.im'.  For details, see:\n"
		       "aips2.nrao.edu/docs/notes/223/223.html");
  
  
  hideDisplayButtons();
  
  uiDataType_["Unknown"] = UNKNOWN;
  uiDataType_["Image"] = IMAGE;
  uiDataType_["Measurement Set"] = MEASUREMENT_SET;
  uiDataType_["Sky Catalog"] = SKY_CATALOG;
  uiDataType_["Directory"] = DIRECTORY;
  uiDataType_["FITS Image"] = IMAGE;
  uiDataType_["Miriad Image"] = IMAGE;
  uiDataType_["Gipsy"] = IMAGE;
  uiDataType_["Restore File"] = RESTORE;
  
  dataType_["unknown"] = UNKNOWN;
  dataType_["image"] = IMAGE;
  dataType_["ms"] = MEASUREMENT_SET;
  dataType_["skycatalog"] = SKY_CATALOG;
  dataType_["restore"] = RESTORE;
  
  uiDisplayType_["Raster Image"] = RASTER;
  uiDisplayType_["Contour Map"] = CONTOUR;
  uiDisplayType_["Vector Map"] = VECTOR;
  uiDisplayType_["Marker Map"] = MARKER;
  uiDisplayType_["Sky Catalog"] = SKY_CAT;
  uiDisplayType_["To Old Window"] = OLDPANEL;
  uiDisplayType_["To New Window"] = NEWPANEL;
  
  displayType_["raster"] = RASTER;
  displayType_["contour"] = CONTOUR;
  displayType_["vector"] = VECTOR;
  displayType_["marker"] = MARKER;
  displayType_["skycatalog"] = SKY_CAT;
  displayType_["oldpanel"] = OLDPANEL;
  displayType_["newpanel"] = NEWPANEL;
  
  leaveOpen_->setToolTip("Uncheck to close this window after "
    "data and display type selection.\n"
    "Use 'Open' button/menu on Display Panel to show it again.");
  leaveOpen_->setChecked(True);
    
  dir_.setFilter(QDir::AllDirs | //QDir::NoSymLinks |
                 QDir::Files);
  dir_.setSorting(QDir::Name);

  //#dk  QSettings are often useful, but users
  //     want to begin in their cwd in this case....
  //
  // QSettings settings("NRAO", "casa");
  // QString lastDir = settings.value("lastDir", dir_.currentPath())
  //                          .toString();
  // //cout << "lastDir=" << lastDir.toStdString() << endl;
  // dir_.cd(lastDir);
  // dirLineEdit_->setText(lastDir);
  
  dirLineEdit_->setText(dir_.currentPath());
  viewer_->selectedDMDir = dirLineEdit_->text().toStdString();
  
  
  buildDirTree();
  
  
  connect(rasterButton_,  SIGNAL(clicked()), SLOT(createButtonClicked()));
  connect(contourButton_, SIGNAL(clicked()), SLOT(createButtonClicked()));
  connect(vectorButton_,  SIGNAL(clicked()), SLOT(createButtonClicked()));
  connect(markerButton_,  SIGNAL(clicked()), SLOT(createButtonClicked()));
  connect(catalogButton_, SIGNAL(clicked()), SLOT(createButtonClicked()));
  connect(oldPanelButton_,  SIGNAL(clicked()), SLOT(restoreToOld_()));
  connect(newPanelButton_,  SIGNAL(clicked()), SLOT(restoreToNew_()));
  connect(updateButton_,  SIGNAL(clicked()), SLOT(buildDirTree()));

  //connect(registerCheck, SIGNAL(clicked()), 
  //      SLOT(registerClicked()));
  
  connect(dirLineEdit_,   SIGNAL(returnPressed()), SLOT(returnPressed()));
  
  connect(treeWidget_,    SIGNAL(itemSelectionChanged()),
			 SLOT(changeItemSelection()));
  
  connect(treeWidget_,    SIGNAL(itemClicked(QTreeWidgetItem*,int)),
			 SLOT(clickItem(QTreeWidgetItem*)));
  
  connect(treeWidget_,    SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		         SLOT(doubleClickItem()));
  
  //connect(registerCheck, SIGNAL(toggled(bool)), displayGroupBox_,
  //			 SLOT(setChecked(bool)));
  
  connect(toolButton_,    SIGNAL(toggled(bool)),  SLOT(showTools(bool)));
  
  connect(toolButton_,    SIGNAL(clicked(bool)),
	  toolGroupBox_,  SLOT(setVisible(bool)));

  connect(viewer_, SIGNAL(createDDFailed(String, String, String, String)),
		     SLOT(showDDCreateError_(String)));
  
  connect(lelEdit_, SIGNAL(gotFocus(QFocusEvent*)),  SLOT(lelGotFocus_()));



}


QtDataManager::~QtDataManager(){
}


void QtDataManager::doubleClickItem(){
  //QMessageBox::warning(this, tr("QtDataManager"), tr("double"));
}


void QtDataManager::clickItem(QTreeWidgetItem* item){
  if(item!=0 && item->text(1)=="Directory") updateDirectory(item->text(0));
}


void QtDataManager::updateDirectory(QString str){
  QDir saved = dir_;
  if(!dir_.cd(str)) {
    QMessageBox::warning(this, tr("QtDataManager"),
    tr("No such directory:\n %1").arg(str));
    dir_ = saved;  }
  if(dir_.entryList().size() == 0) {
    QMessageBox::warning(this, tr("QtDataManager"),
    tr("Could not enter the directory:\n %1").arg(dir_.path()));
    dir_ = saved;  }
  dir_.makeAbsolute();
  dirLineEdit_->setText(dir_.cleanPath(dir_.path()));
  viewer_->selectedDMDir = dirLineEdit_->text().toStdString();
  buildDirTree();
}


void QtDataManager::buildDirTree() {
   
  treeWidget_->clear();
  hideDisplayButtons();
  lelEdit_->deactivate();
  QStringList lbl;
  lbl << "Name" << "Type";
  treeWidget_->setColumnCount(2);
  treeWidget_->setHeaderLabels(lbl);

  QTreeWidgetItem *dirItem;
  dir_.makeAbsolute();
  QStringList entryList = dir_.entryList();
  
  for (int i = 0; i < entryList.size(); i++) {
    QString it = entryList.at(i);
    if (it.compare(".") > 0) {
      QString path = dir_.path() + "/" +  entryList.at(i);
      QString type = viewer_->fileType(path.toStdString()).chars();
      int dType = uiDataType_[type];
      //cout << "path=" << path.toStdString()
      //     << "type=" << type.toStdString() << " dType:"<<dType<<endl

      if (dType!=UNKNOWN) {
        dirItem = new QTreeWidgetItem(treeWidget_);
        dirItem->setText(0, it);
        dirItem->setText(1, type);
        dirItem->setTextColor(1, getDirColor(dType));  }  }  }

	    
  // QSettings settings("NRAO", "casa");
  // //cout << "dir_.path()=" << dir_.path().toStdString() << endl;
  // settings.setValue("lastDir", dir_.path());

  treeWidget_->resizeColumnToContents(0);
  // treeWidget_->setColumnWidth(0, 200);   // (only suppotted in Qt 4.2+)

}


void QtDataManager::changeItemSelection(){
  QList<QTreeWidgetItem *> lst = treeWidget_->selectedItems();
  if (!lst.empty()) {
      lelEdit_->deactivate();
      QTreeWidgetItem *item = (QTreeWidgetItem*)(lst.at(0));
      showDisplayButtons(uiDataType_[item->text(1)]);
      
  }
}



void QtDataManager::showDisplayButtons(int ddtp) {
  hideDisplayButtons();
  switch (ddtp) {
     case IMAGE :
        rasterButton_->show();
        contourButton_->show();
        vectorButton_->show();
        markerButton_->show();
        break;      
     case MEASUREMENT_SET :
        rasterButton_->show();        
        break;
     case SKY_CATALOG:        
        catalogButton_->show();
        break;
     case RESTORE:        
        oldPanelButton_->show();
        newPanelButton_->show();
        break;
  }
}



QColor QtDataManager::getDirColor(int ddtp) {
  QColor clr;
  switch (ddtp) {
     case IMAGE:            clr = Qt::darkGreen;        break;
     case MEASUREMENT_SET:  clr = Qt::darkBlue;         break;
     case SKY_CATALOG:      clr = Qt::darkCyan;         break;
     case RESTORE:          clr = QColor(255,43,45);    break;
     case DIRECTORY:        clr = Qt::black;            break;
     case UNKNOWN: default: clr = Qt::darkMagenta;  }
     
  return clr;
}


void QtDataManager::hideDisplayButtons(){
  rasterButton_->hide();
  contourButton_->hide();
  vectorButton_->hide();
  markerButton_->hide();
  catalogButton_->hide();
  oldPanelButton_->hide();
  newPanelButton_->hide();
}


void QtDataManager::returnPressed(){
  QString str = dirLineEdit_->text();
  updateDirectory(str);
}



void QtDataManager::createButtonClicked() {
  
  QPushButton* button = dynamic_cast<QPushButton*>(sender());
  
  if(viewer_==0 || button==0) return;
  
  String path, datatype, displaytype;
  
  displaytype = (displayType_.key(uiDisplayType_[button->text()]))
		.toStdString();
  
  if(lelEdit_->isActive()) {
    
    // Display LEL expression.
    
    path = lelEdit_->text().trimmed().toStdString();
    datatype = "lel";  } 
  

  
  else if (treeWidget_->currentItem() > 0) {
  
    // Display selected file.
  
    path = (dir_.path() + "/" + treeWidget_->currentItem()->text(0))
	   .toStdString();

    datatype = dataType_.key(uiDataType_[treeWidget_->currentItem()->text(1)])
		        .toStdString();  }

  if(path=="" || datatype=="" || displaytype=="") return;
    

  viewer_->createDD(path, datatype, displaytype);
  
    
  if(!leaveOpen_->isChecked()) close();  // (will hide dialog for now).

}


//<drs> Duplicate code for this functionality in QtDBusViewerAdaptor::restore(...)
//      should try to find a way to make this available from QtViewer
void QtDataManager::restoreToOld_() {
  // Restore viewer state to existing panel.
  // Use the first empty panel, or if none, the first panel.
  
  List<QtDisplayPanel*> DPs = viewer_->openDPs();  
  ListIter<QtDisplayPanel*> dps(DPs);
  
  for(dps.toStart(); !dps.atEnd(); dps++) {
    QtDisplayPanel* dp = dps.getRight();
    if(dp->registeredDDs().len()==0) {
      restoreTo_(dp);		// restore to first empty panel, if any...
      return;  }  }
  
  dps.toStart();
  if(!dps.atEnd()) {
    QtDisplayPanel* dp = dps.getRight();
    restoreTo_(dp);		// ...else, restore to first panel, if any...
    return;  }
     
  restoreToNew_();  }		// ...else, restore to a new panel.




//<drs> Duplicate code for this functionality in QtDBusViewerAdaptor::restore(...)
//      should try to find a way to make this available from QtViewer
void QtDataManager::restoreToNew_() {
  // Create new display panel, restore viewer state to it.
  viewer_->createDPG();
  
  List<QtDisplayPanel*> DPs = viewer_->openDPs();  
  if(DPs.len()>0) {				// (Safety: should be True)
    ListIter<QtDisplayPanel*> dps(DPs);
    dps.toEnd();
    dps--;					// Newly-created dp should be
    QtDisplayPanel* dp = dps.getRight();	// the last one on the list.
    
    restoreTo_(dp);  }  }


  
    
//<drs> Duplicate code for this functionality in QtDBusViewerAdaptor::restore(...)
//      should try to find a way to make this available from QtViewer
void QtDataManager::restoreTo_(QtDisplayPanel* dp) {
  // Restore viewer state to given panel.
  
  // Collect the name of the selected restore file.
  String filename = ( dir_.path() + "/" +
	              treeWidget_->currentItem()->text(0) ).toStdString();

  Bool ok = dp->restorePanelState(filename);
  
  if(ok && !leaveOpen_->isChecked()) close();  }
	// (will hide open dialog for now, if requested).

    
  
  
  
  
  
void QtDataManager::showTools(bool show) {
   if (show) {
      //resize(QSize(537, 386).expandedTo(minimumSizeHint()));
      toolButton_->setText(QApplication::translate("QtDataManager",
						  "Hide Tools"));
   }
   else {
      //resize(QSize(537, 260).expandedTo(minimumSizeHint()));
      toolButton_->setText(QApplication::translate("QtDataManager",
						  "Show Tools"));
   }
   
   toolGroupBox_->setVisible(show);      
}



void QtDataManager::lelGotFocus_() {
  treeWidget_->clearSelection();
  showDisplayButtons(IMAGE);  }
  


void QtDataManager::showDDCreateError_(String errMsg) {
  // For now, just send to cerr.  (To do: put this on a status line).
  cerr<<endl<<errMsg<<endl;  }


} //# NAMESPACE CASA - END
