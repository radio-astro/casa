//# QtRegionManager.cc: Qt implementation of viewer region manager window.
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


#include <display/QtViewer/QtRegionManager.qo.h>


namespace casa { //# NAMESPACE CASA - BEGIN


QtRegionManager::QtRegionManager(QtDisplayPanel* qdp, QWidget* parent) :
		 QWidget(parent), qdp_(qdp), parent_(parent) {
    
  // This gui receives signals from, and sends commands to qdp; qdp must be
  // valid.  parent is usually 0 though; QtRegionManager is a stand-alone
  // auxiliary window to QtDisplayPanelGui.
      

  setupUi(this);
	// Creates all the widgets (such as pathEdit_) from
	// the base class and inserts them into this widget.
  
  
  pathEdit_->setEnabled(False);		// (won't be enabled until
  saveRgnBtn_->setEnabled(False);	// a region has been created).
  
  pathEdit_->setToolTip("Region Creation Steps:\n"
    "  * Move to the desired image plane in the display panel, and\n"
    "    set desired region extent button above (for non-displayed axes).\n"
    "    (Note: you MUST set extent button BEFORE using the mouse).\n"
    "  * Select a 2-D region for the displayed plane with one of\n"
    "    the region mouse tools (rectangle or polygon).\n"
    "  * Modify the offered pathname for the region (if desired)\n"
    "    (the extension '.rgn' is recommended).\n"
    "  * Press 'Save Last Region'.");
  
  leaveOpenCB_->setToolTip("If unchecked, the window will close after "
    "each region is saved.\n"
    "Select 'Region Manager' in the 'Tools' menu to show it again.");
  
    
  connect(qdp_,        SIGNAL(newRegion(String)),  SLOT(newRegion_(String)));
  connect(dismissBtn_, SIGNAL(clicked()),          SLOT(hide()));
  connect(saveRgnBtn_, SIGNAL(clicked()),          SLOT(saveRegion_()));
  connect(planeRB_,    SIGNAL(toggled(bool)),      SLOT(setExtent_()));
  connect(allChanRB_,  SIGNAL(toggled(bool)),      SLOT(setExtent_()));
  connect(allAxesRB_,  SIGNAL(toggled(bool)),      SLOT(setExtent_()));
  connect(pathEdit_,   SIGNAL(textEdited(const QString&)),
		         SLOT(pathnameChg_()));  }



void QtRegionManager::newRegion_(String imgFilename) {
  
  // React to new region creation signal from display panel.
  
  saveRgnBtn_->setEnabled(True);
  pathEdit_->setEnabled(True);     // Assure that these are enabled.
  
  pathEdit_->setText(qdp_->regionPathname(imgFilename).chars());  }
	// Insert region name suggestion into pathname edit box.
  

  
  
void QtRegionManager::saveRegion_() {
  
  // Reacts to 'Save Last Region' button.
  
  if(qdp_->saveLastRegion(pathEdit_->text().toStdString())) {
    if(!leaveOpenCB_->isChecked()) hide();  }
	// Save succeeded: dismiss gui unless 'Leave Open' is checked.
    
  else QMessageBox::warning(this, "QtRegionManager",
		    "Could not create the region file.\n"
		    "Please check pathname and directory permissions\n"
		    "and be sure the file does not already exist.");
			// Could not save region.  Warn user.

  saveRgnBtn_->setEnabled(False);  }
	// Disable save button until path or region is changed.
    
  


void QtRegionManager::setExtent_() {
  
  // Reacts to extent button selection -- sets extent state in display panel.
  
  qdp_->setRegionExtent(allAxesRB_->isChecked()? 2 :
			allChanRB_->isChecked()? 1 : 0);  }



void QtRegionManager::pathnameChg_() {
  // Reacts to change in pathname (just to re-enable Save button).
  saveRgnBtn_->setEnabled(qdp_->hasRegion());  }




} //# NAMESPACE CASA - END
