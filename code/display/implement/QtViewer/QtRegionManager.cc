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


#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtRegionManager.qo.h>
#include <display/RegionShapes/RegionShapes.h>
#include <casa/Containers/Block.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Images/ImageRegion.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/WCUnion.h>

#include <casa/Utilities/CountedPtr.h>


namespace casa { //# NAMESPACE CASA - BEGIN


QtRegionManager::QtRegionManager(QtDisplayPanel* qdp, QWidget* parent) :
		 QWidget(parent), qdp_(qdp), parent_(parent) {
    
  // This gui receives signals from, and sends commands to qdp; qdp must be
  // valid.  parent is usually 0 though; QtRegionManager is a stand-alone
  // auxiliary window to QtDisplayPanelGui.
      

  setupUi(this);
	// Creates all the widgets (such as pathEdit_) from
	// the base class and inserts them into this widget.
  
  
  cleanup();

  updateNames();
  drawing_mode->clear();
  drawing_mode->addItem("union");

  /*
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
  */
 
     
  connect(qdp_,        SIGNAL(newRegion(String)),  SLOT(newRegion_(String)));
  /*
  connect(dismissBtn_, SIGNAL(clicked()),          SLOT(hide()));
  connect(saveRgnBtn_, SIGNAL(clicked()),          SLOT(saveRegion_()));
  connect(planeRB_,    SIGNAL(toggled(bool)),      SLOT(setExtent_()));
  connect(allChanRB_,  SIGNAL(toggled(bool)),      SLOT(setExtent_()));
  connect(allAxesRB_,  SIGNAL(toggled(bool)),      SLOT(setExtent_()));
  connect(pathEdit_,   SIGNAL(textEdited(const QString&)),
		         SLOT(pathnameChg_())); 

  */
  connect(qdp_,  SIGNAL(newRegisteredDD(QtDisplayData*)),      SLOT(updateNames()));
  connect(this, SIGNAL(destroyed()), SLOT(cleanup()));
  connect(saveInImage, SIGNAL(clicked()), SLOT(saveRegionInImage()));
  connect(saveRGNFile, SIGNAL(clicked()), SLOT(saveRegion_()));
  connect(deleteReg, SIGNAL(clicked()), SLOT(removeRegion()));
  connect(planeOnlyCB, SIGNAL(stateChanged(int)), SLOT(singlePlane()));
  connect(chanExt, SIGNAL(clicked()), SLOT(planeExtension()));
  connect(polExt, SIGNAL(clicked()), SLOT(planeExtension()));  
  connect(resetregions, SIGNAL(clicked()), SLOT(cleanup()));
  connect(loadFile, SIGNAL(clicked()), SLOT(loadRegionsFromFile()));
  connect(showOrHide, SIGNAL(clicked()), SLOT(loadRegionsFromFile()));
  
  connect(qdp_,        SIGNAL(mouseRegionReady(Record, WorldCanvasHolder*)),  SLOT(drawRegion(Record, WorldCanvasHolder*)));
  
 

}



  void QtRegionManager::drawRegion(Record mousereg, WorldCanvasHolder *wch){
    cout << "Inside drawRegion" << endl;
 
    if(this->isVisible() && mousereg.isDefined("world")){
      String type=mousereg.asString("type");
      List<QtDisplayData*> DDs = qdp_->registeredDDs();
      ListIter<QtDisplayData*> qdds(DDs);
      qdds.toEnd();
      qdds--;
      ListIter<RegionShape*> rgiter(regShapes_p);
      rgiter.toEnd();
      QtDisplayData* qdd = qdds.getRight();

      if( (qdd->imageInterface())){
	CoordinateSystem csys=(qdd->imageInterface())->coordinates();
	Int dirInd=csys.findCoordinate(Coordinate::DIRECTION);
	MDirection::Types dirType=csys.directionCoordinate(dirInd).directionType(True);
	if(type.contains("box")){	
	  Record corners = mousereg.asRecord("world");   
	  Vector<Double> blc = corners.asArrayDouble("blc");
	  Vector<Double> trc = corners.asArrayDouble("trc");
	  Vector<String> units=corners.asArrayString("units");
	  blc(0)=Quantity(blc(0), units(0)).getValue(RegionShape::UNIT);
	  blc(1)=Quantity(blc(1), units(1)).getValue(RegionShape::UNIT);
	  trc(0)=Quantity(trc(0), units(0)).getValue(RegionShape::UNIT);
	  trc(1)=Quantity(trc(1), units(1)).getValue(RegionShape::UNIT);
	  //Int zindex = mousereg.asInt("zindex");
	  RSRectangle *rect= new RSRectangle((blc(0)+trc(0))/2.0,(blc(1)+trc(1))/2.0,fabs(trc(0)-blc(0)), fabs(trc(1)-blc(1)), dirType);
	  rect->setLineColor("cyan");
	  qdp_->registerRegionShape(rect);
	  rgiter.addRight(rect);
	}
	else if(type.contains("poly")){
	  Record vertices=mousereg.asRecord("world");
	  Vector<Double> xs = vertices.asArrayDouble("x");
	  Vector<Double> ys = vertices.asArrayDouble("y");	 
	  Vector<String> units=vertices.asArrayString("units");
	  for (uInt k=0; k< xs.nelements(); ++k){
	    xs(k)=Quantity(xs(k), units(0)).getValue(RegionShape::UNIT);
	    ys(k)=Quantity(ys(k), units(1)).getValue(RegionShape::UNIT);
	    
	  }
	  RSPolygon *poly= new RSPolygon(xs,ys,dirType);
	  poly->setLineColor("cyan");
	  qdp_->registerRegionShape(poly);
	  rgiter.addRight(poly);
	}
	
      }
      
    }//Visible
  }

  void QtRegionManager::loadRegionsFromFile(){
    QMessageBox::warning(this, "QtRegionManager",
		    "This are not supported yet\n");


  }
 
  void QtRegionManager::singlePlane(){
    if(planeOnlyCB->isChecked()){
      chanExt->setChecked(False);
      polExt->setChecked(False);
    }

      
    

  }
  

  void QtRegionManager::planeExtension(){
    if(chanExt->isChecked() || polExt->isChecked()){
      //To Be done
      chanExt->setChecked(False);
      polExt->setChecked(False);
      planeOnlyCB->setChecked(True);
      QMessageBox::warning(this, "QtRegionManager",
		    "This are not supported yet\n");
      
    }


  }

void QtRegionManager::updateNames() {
  regName->clear();
  Vector<String> regionNames=qdp_->listRegionsInImage();
  if(regionNames.nelements()==0){
    regName->addItem("No regions saved");
    return ;
  }
  for (uInt kk=0; kk < regionNames.nelements(); ++kk){
    regName->addItem(regionNames(kk).c_str());
  }



}

void QtRegionManager::removeRegion() {

  String regname((regName->currentText()).toStdString());
  
  if(regname != ""){
    qdp_->removeRegionInImage(regname);
    updateNames();
  }

}


void QtRegionManager::cleanup() {
  uInt nreg=unionRegions_p.nelements();
  for (uInt k=0; k< nreg; ++k){
    if(unionRegions_p[k] !=0){
      delete unionRegions_p[k];
    }

  }
  unionRegions_p.resize(0, True);
  ListIter<RegionShape*> rgiter(regShapes_p);
  rgiter.toEnd();
  while(!rgiter.atStart()) {
    rgiter--;
    if(qdp_->isRegistered(rgiter.getRight())){
      qdp_->unregisterRegionShape(rgiter.getRight());

    }
    rgiter.removeRight();
  }



}

void QtRegionManager::newRegion_(String imgFilename) {

  if(this->isVisible()){
    if(qdp_->hasRegion()){
      if ((drawing_mode->currentText()).contains("union")){
	
	uInt regNo=unionRegions_p.nelements();
	unionRegions_p.resize(regNo+1);
	unionRegions_p[regNo]=new const ImageRegion(qdp_->lastRegion()); 
	//cout << "regNo " << regNo << " wc " << unionRegions_p[regNo]->isWCRegion() << "   lc  " << unionRegions_p[regNo]->isLCRegion() << endl;
      }
    }
  }

  
  // React to new region creation signal from display panel.
  /*
  saveRgnBtn_->setEnabled(True);
  pathEdit_->setEnabled(True);     // Assure that these are enabled.
  
  //pathEdit_->setText(qdp_->regionPathname(imgFilename).chars());  
	// Insert region name suggestion into pathname edit box.
  pathEdit_->setText("regA");
  savedRegions_->setText(qdp_->listRegions().c_str());
  */

}
  
  
void QtRegionManager::saveRegion_() {

  if((savedName->text()) !=""){
    String regName(savedName->text().toStdString());
    if( !(qdp_->saveLastRegion(regName))) {
      QMessageBox::warning(this, "QtRegionManager",
		    "Could not create the region file.\n"
		    "Please check pathname and directory permissions\n"
		    "and be sure the file does not already exist.");
			// Could not save region.  Warn user.
    }
  }
  else{
    
     QMessageBox::warning(this, "QtRegionManager",
		    "Please enter a file name to store region in the box below button\n");

  }

 


}

void QtRegionManager::saveRegionInImage() {




    
  // Reacts to 'Save in Image'.
  
  
  if((savedName->text()) !=""){
    String regName(savedName->text().toStdString());
    if(unionRegions_p.nelements() > 0){
      WCUnion leUnion(unionRegions_p);
      qdp_->saveRegionInImage(regName, ImageRegion(leUnion));
    }
    updateNames();

    


  }
  else QMessageBox::warning(this, "QtRegionManager",
		    "Please enter a name to label region in box below the button\n");

  


}



	// Disable save button until path or region is changed.
    
  


void QtRegionManager::setExtent_() {
  
  // Reacts to extent button selection -- sets extent state in display panel.
  /*
  
  qdp_->setRegionExtent(allAxesRB_->isChecked()? 2 :
			allChanRB_->isChecked()? 1 : 0);  

  */

}



void QtRegionManager::pathnameChg_() {
  // Reacts to change in pathname (just to re-enable Save button).
  //  saveRgnBtn_->setEnabled(qdp_->hasRegion()); 

}




} //# NAMESPACE CASA - END
