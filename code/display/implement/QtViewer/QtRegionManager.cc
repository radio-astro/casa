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
#include <casa/Quanta/QuantumHolder.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/WCUnion.h>
#include <display/Display/DParameterChoice.h>
#include <casa/IO/AipsIO.h>
#include <images/Images/PagedImage.h>

#include <tables/Tables/TableRecord.h>


#include <display/DisplayDatas/LatticeAsRaster.h>
#include <casa/Utilities/CountedPtr.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//template <class T> class ImageInterface;

QtRegionManager::QtRegionManager(
   QtDisplayPanel* qdp, QWidget* parent) : 
    QWidget(parent), qdp_(qdp), parent_(parent), 
     regData(), regState() {
    
  // This gui receives signals from, and sends commands 
  // to qdp; qdp must be valid.  

  setupUi(this);
  // Creates all the widgets (such as pathEdit_) from
  // the base class and inserts them into this widget.
  
  
  cleanup();

  //loadRegionFromImage();
  drawing_mode->clear();
  drawing_mode->addItem("union");

 
  connect(qdp_,  SIGNAL(registrationChange()), 
                 SLOT(loadRegionFromImage()));
  //from double click in a region 
  connect(qdp_,   
       SIGNAL(mouseRegionReady(Record, WorldCanvasHolder*)),
       SLOT(drawRegion(Record, WorldCanvasHolder*)));
  //also from double click a region 
  connect(qdp_, SIGNAL(newRegion(String)),  
                SLOT(newRegion_(String)));
  connect(this, SIGNAL(extendRegion(String, String)),
          qdp_, SLOT(extendRegion(String, String)));
  connect(chan_sel, SIGNAL(editingFinished()), 
                    SLOT(resetRegionExtension()));
  connect(pol_sel, SIGNAL(editingFinished()), 
                    SLOT(resetRegionExtension()));
  connect(this, SIGNAL(destroyed()), SLOT(cleanup()));
  connect(this, SIGNAL(destroyed()), SLOT(cleanup()));
  connect(saveInImage, SIGNAL(clicked()), 
                       SLOT(saveRegionInImage()));
  connect(saveRGNFile, SIGNAL(clicked()), 
                       SLOT(saveRegionInFile()));
  connect(deleteReg, SIGNAL(clicked()), 
                     SLOT(removeRegion()));
  //connect(planeOnlyCB, SIGNAL(stateChanged(int)), 
  //                   SLOT(singlePlane()));
  connect(chanExt, SIGNAL(clicked()), 
                   SLOT(extendChan()));
  connect(polExt, SIGNAL(clicked()), 
                  SLOT(extendPol()));  
  connect(resetregions, SIGNAL(clicked()), 
                        SLOT(cleanup()));
  connect(loadFile, SIGNAL(clicked()), 
                    SLOT(loadRegionFromFile()));
  connect(showOrHide, SIGNAL(clicked()), 
                      SLOT(toggleImageRegion()));
  connect(regName, 
          SIGNAL(currentIndexChanged(const QString &)), 
          SLOT(currentRegionChanged(const QString &)));

  planeOnlyCB->setChecked(False); 
  planeOnlyCB->hide();
}
  void QtRegionManager::displaySelectedRegion(){

    String theName(regName->currentText().toStdString());
    /////read region name and call showregion with name
    cout << "regname " << theName << endl;
    showRegion(theName);

  }

  void QtRegionManager::showRegion(const String& regName){

    ImageRegion imreg=qdp_->getRegion(regName);
    if(imreg.isWCRegion()){

      const WCRegion* wcreg=imreg.asWCRegionPtr();
      CoordinateSystem csys;
      {
	List<QtDisplayData*> DDs = qdp_->registeredDDs();
	ListIter<QtDisplayData*> qdds(DDs);
	qdds.toEnd();
	qdds--;
	QtDisplayData* qdd = qdds.getRight();
	csys=(qdd->imageInterface())->coordinates();
      }
      //cout << "before showreg csys" << endl;
      Int dirInd=csys.findCoordinate(Coordinate::DIRECTION);
      MDirection::Types dirType=csys.directionCoordinate(dirInd).directionType(True);
      RSComposite *theShapes= new RSComposite(dirType);
      //cout << "calling add" <<endl;
      addRegionsToShape(theShapes, wcreg);
      theShapes->setLineColor("cyan");
      ListIter<RegionShape*> rgiter(regShapes_p);
      rgiter.toEnd();
      qdp_->registerRegionShape(theShapes);
      rgiter.addRight(theShapes);
    }

  }

  void QtRegionManager::addRegionsToShape(RSComposite*& theShapes, const WCRegion*& wcreg){
    if((wcreg->type()) == "WCBox"){
      TableRecord boxrec=wcreg->toRecord("");
      const RecordInterface& blcrec=boxrec.asRecord("blc");
      const RecordInterface& trcrec=boxrec.asRecord("trc");
      CoordinateSystem *coords;
      coords=CoordinateSystem::restore(boxrec,"coordinates");
      //cout << "coords rect " << coords->nCoordinates() << endl;
      Int dirInd=coords->findCoordinate(Coordinate::DIRECTION);
      MDirection::Types dirType=coords->directionCoordinate(dirInd).directionType(True);
      //Assuming x, y axes are dirInd and dirInd+1
      Vector<Double> blc(2);
      Vector<Double> trc(2);
      QuantumHolder h;
      for (Int j=dirInd; j <= dirInd+1; ++j){
	const RecordInterface& subRec0=blcrec.asRecord(j);
	const RecordInterface& subRec1=trcrec.asRecord(j);
	String error;
	if (!h.fromRecord(error, subRec0)) {
           throw (AipsError ("WCBox::fromRecord - could not recover blc because "+error));
	}

	blc(j-dirInd)=h.asQuantumDouble().getValue(RegionShape::UNIT);
	if (!h.fromRecord(error, subRec1)) {
           throw (AipsError ("WCBox::fromRecord - could not recover trc because "+error));
	}
	trc(j-dirInd)=h.asQuantumDouble().getValue(RegionShape::UNIT);
      }
      RSRectangle *rect= new RSRectangle((blc(0)+trc(0))/2.0,(blc(1)+trc(1))/2.0,fabs(trc(0)-blc(0)), fabs(trc(1)-blc(1)), dirType);
      rect->setLineColor("cyan");
      //Doing it manually as Composites does not seem to be getting registered
      //This is not a workaround because it add extra dd
      //to the panel 
      ///////////////////////////
      //qdp_->registerRegionShape(rect);
      //ListIter<RegionShape*> rgiter(regShapes_p);
      //rgiter.toEnd();
      //rgiter.addRight(rect);
      ///////////////////////////

      theShapes->addShape(rect);

    }
    else if((wcreg->type())== "WCPolygon"){
      TableRecord polyrec=wcreg->toRecord("");
      CoordinateSystem *coords;
      coords=CoordinateSystem::restore(polyrec,"coordinates");
      //cout << "coords polyg " << coords->nCoordinates() << endl;

      Int dirInd=coords->findCoordinate(Coordinate::DIRECTION);
      MDirection::Types dirType=coords->directionCoordinate(dirInd).directionType(True);
      Vector<Double> x;
      Vector<Double> y;
      const RecordInterface& subRecord0 = polyrec.asRecord("x");
      String error;
      QuantumHolder h;
      if (!h.fromRecord(error, subRecord0)) {
	throw (AipsError ("WCPolygon::fromRecord - could not recover X Quantum vector because "+error));
      }
      x = h.asQuantumVectorDouble().getValue(RegionShape::UNIT);
      const RecordInterface& subRecord1 = polyrec.asRecord("y");
      if (!h.fromRecord(error, subRecord1)) {
	throw (AipsError ("WCPolygon::fromRecord - could not recover Y Quantum vector because "+error));
      }
      y = h.asQuantumVectorDouble().getValue(RegionShape::UNIT);
      
      RSPolygon *poly= new RSPolygon(x,y,dirType);
      poly->setLineColor("cyan");
      //Doing it manually as Composites does not seem to be getting registered
      //This is not a workaround because it add extra dd
      //to the panel 
      ///////////////
      //qdp_->registerRegionShape(poly);
      //ListIter<RegionShape*> rgiter(regShapes_p);
      //rgiter.toEnd();
      //rgiter.addRight(poly);
      /////////////
      theShapes->addShape(poly);
    }
    else if((wcreg->type()) == "WCUnion" ||(wcreg->type()) == "WCIntersection" ){
      PtrBlock<const WCRegion*> regPtrs=(static_cast<const WCCompound* >(wcreg))->regions();
      //cout << "number of wcregions " << regPtrs.nelements() << endl;
      for (uInt j=0; j < regPtrs.nelements(); ++j){
	addRegionsToShape(theShapes, regPtrs[j]);
      }

    }


  }
  void QtRegionManager::drawRegion(
      Record mousereg, WorldCanvasHolder *wch){
    //cout << "drawRegion ..." << endl;
 
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

  void QtRegionManager::loadRegionsImageFromFile(){
    QMessageBox::warning(this, "QtRegionManager",
		    "Load region From file\n");

    if ((savedName->text()) !=""){
       QString sName = savedName->text();
       String rName(sName.toStdString());
       ImageRegion* reg = 0;
       TableRecord rec;
       try{
          //cout << "rName=" << rName << endl;
          AipsIO os(rName, ByteIO::Old);
          //os.open(rName);
          os >> rec;  
          //cout << "infile region record:\n" << rec << endl;
          reg = ImageRegion::fromRecord(rec, rName+".tbl");

          List<QtDisplayData*> DDs = qdp_->registeredDDs();
          ListIter<QtDisplayData*> qdds(DDs);
          qdds.toEnd();
          qdds--;
          QtDisplayData* qdd = qdds.getRight();

          if (qdd->imageInterface()){

            ImageInterface<Float>* pImage = 0;
            pImage = new SubImage<Float>(
               *(qdd->imageInterface()), *reg, False);
     
            DisplayData *dd = 0;
            if (!pImage)
               return;
            uInt ndim = pImage->ndim(); 
            if (ndim < 2) {
               throw(AipsError(String(
                 "Image has less than two dimensions"))); 
            } else if (ndim == 2) {
              dd = (DisplayData *)(
                   new LatticeAsRaster<Float>(pImage, 0, 1));
            } else {                                                         IPosition fixedPos(ndim); 
               fixedPos = 0;
               dd = (DisplayData *)(
                   new LatticeAsRaster<Float>(
                       pImage, 0, 1, 2, fixedPos));
            }
            //cout << "dd=" << dd <<  endl;
            regData[sName] = dd;
            regName->addItem(sName);
            currentRegionChanged(sName);
          }
          return ;
       }
       catch(...) { 
          cout << "Failed to read region record" << endl;
          return ;  
       }
       return ;  
    }
  }

  void QtRegionManager::loadRegionFromFile(){
    //QMessageBox::warning(this, "QtRegionManager",
    //		    "Load region From file\n");

    if ((savedName->text()) == "")
       return;
   
    QString sName = savedName->text();
    if (regData.contains(sName))
       return;

    String rName(sName.toStdString());
    ImageRegion* reg;
    TableRecord rec;
    try{
       //cout << "rName=" << rName << endl;
       AipsIO os(rName, ByteIO::Old);
       //os.open(rName);
       os >> rec;  
       //cout << "infile region record:\n" << rec << endl;
       reg = ImageRegion::fromRecord(rec, rName+".tbl");

       List<QtDisplayData*> DDs = qdp_->registeredDDs();
       ListIter<QtDisplayData*> qdds(DDs);
       qdds.toEnd();
       qdds--;
       QtDisplayData* qdd = qdds.getRight();

       regName->addItem(sName);
       regData[sName] = regionToShape(qdd, reg);
       regState[sName] = false;
       currentRegionChanged(sName);

       //RegionShape* dd = 0;
       //String err = "";
       //dd = RegionShape::shapeFromRecord(rec, err);
       //cout << "dd=" << dd <<  endl;
       //if (err == "") {
       //   regData[sName] = dd;
       //   regName->addItem(sName);
       //   currentRegionChanged(sName);
       //}
       //else {
       //  cout << "Error convert from record to shape: "
       //       << err << endl;
       //}
       
    }
    catch(...) { 
       cout << "Failed to read region record" << endl;
    }
  }

  void QtRegionManager::toggleImageRegion(){

    //DisplayData *dd = getImageData(regName->currentText());
    //DisplayData *dd = getBoundingBoxData(
    //                  regName->currentText());

    QString sName = regName->currentText();
    //cout << "toggle region=" << sName.toStdString() 
    //     << endl;
    if (sName == "")
       return;

    DisplayData *dd = regData[regName->currentText()];
    //cout << "DisplayData dd=" << dd 
    //     << "\n"  
    //     << ((RegionShape*)dd)->toRecord() << endl;
    if (!dd) 
       return;
       
    //cout << "regData count=" << regData.count() << endl;
    if (showOrHide->text()=="Show") {
       //qdp_->registerRegionShape((RegionShape*)dd);
       qdp_->hold();
       qdp_->panelDisplay()->addDisplayData(*dd);
       qdp_->release();
       showOrHide->setText("Hide");
       regState[sName] = true;
       //cout << sName.toStdString() 
       //     << " was hidden, is shown now" << endl;
    } 
    else {
       //qdp_->unregisterRegionShape((RegionShape*)dd);
       qdp_->hold();
       qdp_->panelDisplay()->removeDisplayData(*dd);
       qdp_->release();
       showOrHide->setText("Show");
       regState[sName] = false;
       //cout << sName.toStdString() 
       //     << " was shown, is hidden now" << endl;
    }

  }

 
  void QtRegionManager::singlePlane(){
    //cout << "planeOnly checked="
    //     << planeOnlyCB->isChecked() << endl;
    if(planeOnlyCB->isChecked()){
      chan_sel->setText("");
      pol_sel->setText("");
      chan_sel->setEnabled(False);
      pol_sel->setEnabled(False);
      chanExt->setChecked(False);
      polExt->setChecked(False);
    }
  }
  
  void QtRegionManager::extendChan(){
    //cout << "extendChan checked=" 
    //     << chanExt->isChecked() << endl;
    if(chanExt->isChecked()){
      chan_sel->setEnabled(True);
      chan_sel->setText("");
      //planeOnlyCB->setChecked(False);
    }
    else {
      chan_sel->setEnabled(False);
      chan_sel->setText("");
    }
  }

  void QtRegionManager::extendPol(){
    //cout << "extendPol checked=" 
    //     << polExt->isChecked() << endl;
    if(polExt->isChecked()){
      pol_sel->setEnabled(True);
      pol_sel->setText("");
      //planeOnlyCB->setChecked(False);
    }
    else {
      pol_sel->setEnabled(False);
      pol_sel->setText("");
    }
  }

void QtRegionManager::loadRegionFromImage() {
  regName->clear();
  regData.clear();
  regState.clear();
  cleanup();
  Vector<String> regionNames=qdp_->listRegionsInImage();
  if(regionNames.nelements()==0){
    regName->addItem("No regions saved");
    showOrHide->setDisabled(true);
    deleteReg->setDisabled(true);
  }
  else {
    showOrHide->setEnabled(true);
    deleteReg->setEnabled(true);

    List<QtDisplayData*> DDs = qdp_->registeredDDs();
    ListIter<QtDisplayData*> qdds(DDs);
    qdds.toEnd();
    qdds--;
    QtDisplayData* qdd = qdds.getRight();

    for (uInt kk=0; kk < regionNames.nelements(); ++kk){
       if (qdd->imageInterface()){
          //may not be necessary to check because
          //regionNames list already does
          bool inImage = (qdd->imageInterface())
                 ->hasRegion(regionNames(kk));
          if (!inImage)
             continue;

          ImageRegion* reg = (qdd->imageInterface())
                    ->getImageRegionPtr(regionNames[kk]);
       
          QString sName = regionNames(kk).c_str();
          regName->addItem(sName);
          regData[sName] = regionToShape(qdd, reg);
          regState[sName] = false;
          currentRegionChanged(sName);
       }
    }
  }

}

void QtRegionManager::removeRegion() {

  QString sName = regName->currentText();

  String regname(sName.toStdString());
  //cout << "remove region " << regname << endl;  
  if(regname != ""){
    qdp_->removeRegionInImage(regname);
  }

  DisplayData *dd = regData[sName];
  if (dd) {
     //qdp_->registerRegionShape((RegionShape*)dd);
     qdp_->hold();
     qdp_->panelDisplay()->removeDisplayData(*dd);
     qdp_->release();
         
     regState.remove(sName);
     regData.remove(sName);
     regName->removeItem(regName->currentIndex());
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
  //cout << "newRegion..." << endl;

  if(this->isVisible()){
    if(qdp_->hasRegion()){
      if ((drawing_mode->currentText()).contains("union")){
	
	uInt regNo=unionRegions_p.nelements();
	unionRegions_p.resize(regNo+1);
	unionRegions_p[regNo]=new const 
              ImageRegion(qdp_->lastRegion()); 
	//cout << "regNo=" << regNo << " WCRegion=" 
        //     << unionRegions_p[regNo]->isWCRegion() 
        //     << "   LCRegion=" 
        //     << unionRegions_p[regNo]->isLCRegion() 
        //     << endl;

      }
    }
  }
  //for (uInt i = 0; i < unionRegions_p.nelements(); i++) {
  //   if (unionRegions_p[i] != 0){
  //       cout << "to be save ImageRegion: " << i << "\n" 
  //            <<unionRegions_p[i]->toRecord("arbitrary") 
  //            << endl;
  //   }
  //}

}
  
  
void QtRegionManager::saveRegionInFile() {

  //cout << "extend chan=" 
  //     << chan_sel->text().toStdString() << endl;
  //cout << "extend pol=" 
  //     << pol_sel->text().toStdString() << endl;

  QString sName = savedName->text();
  if(sName != ""){
    String regname(sName.toStdString());
    if(unionRegions_p.nelements() > 0){
      WCUnion leUnion(unionRegions_p);
      ImageRegion* reg = new ImageRegion(leUnion);

      try{
         AipsIO os(regname, ByteIO::NewNoReplace);
         os << reg->toRecord(regname+".tbl");  
         cleanup();
      }
      catch(...) { 
         QMessageBox::warning(this, "QtRegionManager",
            "Could not create the region file.\n"
            "Please check pathname and directory\n"
            "permissions and be sure the file \n"
            "does not already exist.");
      }
    }
  }
  else {
     QMessageBox::warning(this, "QtRegionManager",
	 "Please enter a name to label region in box"
         " below the button\n");
  }

}

void QtRegionManager::saveRegionInImage() {

  // Reacts to 'Save in Image'.

  List<QtDisplayData*> DDs = qdp_->registeredDDs();
  ListIter<QtDisplayData*> qdds(DDs);
  qdds.toEnd();
  qdds--;
  QtDisplayData* qdd = qdds.getRight();
  
  QString sName = savedName->text();
  if (sName == "") {
     QMessageBox::warning(this, "QtRegionManager",
	 "Please enter a name to label region in box"
         " below the button\n");
     return;
  }

  String regname(sName.toStdString());
  if (qdd->imageInterface() &&
      (qdd->imageInterface())->hasRegion(regname)) {
     QMessageBox::warning(this, "QtRegionManager",
	 "Please enter a different name to label region.\n"
         "There is already a saved region with that name.");
     return;

  }



  if (unionRegions_p.nelements() > 0){
     WCUnion leUnion(unionRegions_p);
     ImageRegion* reg = new ImageRegion(leUnion);
     //cout << "to be save ImageRegion:\n"  
     //     << reg->toRecord("arbitrary") << endl;
     qdp_->saveRegionInImage(regname, *reg);
    
     regName->addItem(sName);
     regData[sName] = regionToShape(qdd, reg);
     regState[sName] = false;
     currentRegionChanged(sName);
     cleanup();

  }

}


void QtRegionManager::currentRegionChanged(const QString &i) {
  //cout << "currentRegionChanged " 
  //     << i.toStdString() << endl; 

  if (regState.count() > 0) {
     deleteReg->setEnabled(true);
     int noreg = regName->findText("No regions saved");
     regName->removeItem(noreg);
  }

  int id = max(regName->findText(i), 0);
  regName->setCurrentIndex(id);

  if (regState[i] == true) {
     showOrHide->setText("Hide");
     showOrHide->setEnabled(true);
     return;
  }
     
  //this activates the selected image region 
  //if the region is not in the region list
  //create the dd and add (name, dd) to the hash
  //show it by put it on pd directly

  //DisplayData *dd = getImageData(i);
  //DisplayData *dd = getBoundingBoxData(i);
  DisplayData *dd = regData[i];
  if (dd) {
     //cout << "add DisplayData " << dd 
     //     << "\n"
     //     << ((RegionShape*)dd)->toRecord() << endl;

     //qdp_->registerRegionShape((RegionShape*)dd);
     qdp_->hold();
     qdp_->panelDisplay()->addDisplayData(*dd);
     qdp_->release();

     showOrHide->setText("Hide");
     showOrHide->setEnabled(true);
     regState[i] = true;
     //QMessageBox::warning(this, "wait",
     // 		    "wait---------\n");
  }
}

DisplayData* QtRegionManager::getImageData(QString regName){
  /*
  QString to std::string:
  std::string((const char*)qstr);

  std::string to QString:
  QString(str.c_str());
  */
  //String regname((const char*)(regName.data())); 

  String regname(regName.toStdString()); 
  //cout << "get region=" << regname << endl;

  if (regname=="")
     return 0;

  if (regData.contains(regName)) {
     return regData.value(regName);
  }

  List<QtDisplayData*> DDs = qdp_->registeredDDs();
  ListIter<QtDisplayData*> qdds(DDs);
  qdds.toEnd();
  qdds--;
  QtDisplayData* qdd = qdds.getRight();

  if (qdd->imageInterface()){
     bool inImage = (qdd->imageInterface())
              ->hasRegion(regname);
     //cout << "hasRegion " << regname << " "  
     //     << (qdd->imageInterface())->hasRegion(regname) 
     //     << endl;
     if (!inImage)
        return 0;

     ImageRegion* reg = (qdd->imageInterface())
                         ->getImageRegionPtr(regname);

     ImageInterface<Float>* pImage = 0;
     pImage = new SubImage<Float>(*(qdd->imageInterface()), 
                  *reg, False);
     
     DisplayData *dd = 0;
     uInt ndim = pImage->ndim(); 
     if (ndim < 2) {
        throw(AipsError(String(
              "Image has less than two dimensions"))); 
     } else if (ndim == 2) {
        dd = (DisplayData *)(
              new LatticeAsRaster<Float>(pImage, 0, 1));
     } else {                                                         IPosition fixedPos(ndim); 
        fixedPos = 0;
        dd = (DisplayData *)(
             new LatticeAsRaster<Float>(
                 pImage, 0 , 1, 2, fixedPos));
     }
     regData[regName] = dd;
     return dd;
  } 
  return 0;

}


DisplayData* 
QtRegionManager::getBoundingBoxData(QString regName){

  String regname(regName.toStdString()); 
  cout << "get region=" << regname << endl;

  if (regname=="")
     return 0;

  if (regData.contains(regName)) {
     return regData.value(regName);
  }

  List<QtDisplayData*> DDs = qdp_->registeredDDs();
  ListIter<QtDisplayData*> qdds(DDs);
  qdds.toEnd();
  qdds--;
  QtDisplayData* qdd = qdds.getRight();

  if (qdd->imageInterface()){
     bool inImage = (qdd->imageInterface())
              ->hasRegion(regname);
     //cout << "hasRegion " << regname << " "  
     //     << (qdd->imageInterface())->hasRegion(regname) 
     //     << endl;
     if (!inImage)
        return 0;

     ImageRegion* reg = (qdd->imageInterface())
                         ->getImageRegionPtr(regname);
     if (reg && reg->isWCRegion()){

        const WCRegion* wcreg = reg->asWCRegionPtr();
        CoordinateSystem csys;
	csys = (qdd->imageInterface())->coordinates();
        //cout << "before showreg csys" << endl;
        Int dirInd =
              csys.findCoordinate(Coordinate::DIRECTION);
        MDirection::Types dirType = csys.
           directionCoordinate(dirInd).directionType(True);
        RSComposite *theShapes= new RSComposite(dirType);
        //cout << "calling add" <<endl;
        addRegionsToShape(theShapes, wcreg);
        theShapes->setLineColor("cyan");
        theShapes->setLineWidth(1);

        //regData[regName] = theShapes;
        return theShapes;
     }

  } 
  return 0;

}

RSComposite* QtRegionManager::regionToShape(
     QtDisplayData* qdd, const ImageRegion* reg) {

     if (reg && reg->isWCRegion()){

        const WCRegion* wcreg = reg->asWCRegionPtr();
        CoordinateSystem csys;
	csys = (qdd->imageInterface())->coordinates();
        //cout << "before showreg csys" << endl;
        Int dirInd =
              csys.findCoordinate(Coordinate::DIRECTION);
        MDirection::Types dirType = csys.
           directionCoordinate(dirInd).directionType(True);
        RSComposite *theShapes= new RSComposite(dirType);
        //cout << "calling add" <<endl;
        addRegionsToShape(theShapes, wcreg);
        theShapes->setLineColor("cyan");
        theShapes->setLineWidth(1);

        //regData[regName] = theShapes;
        return theShapes;
     }
     else {
        return 0;
     }
}

void QtRegionManager::resetRegionExtension() {
      emit extendRegion(chan_sel->text().toStdString(),
                        pol_sel->text().toStdString());
}

void QtRegionManager::changeAxis(
         String xa, String ya, String za) {
   //cout << "change axis=" << xa << " " << ya
   //     << " " << za << endl;
   //int cb = 0;
   //if (xa.contains("Decl") && ya.contains("Right"))
   //    cb = -1;
   //if (xa.contains("Right") && ya.contains("Decl"))
   //    cb = 1;
   //if (!za.contains("Freq"))
   //    cb = 0;
}


} //# NAMESPACE CASA - END
