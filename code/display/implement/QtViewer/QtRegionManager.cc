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
#include <casa/Containers/RecordField.h>
#include <casa/Quanta/QuantumHolder.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/WCUnion.h>
#include <images/Regions/WCRegion.h>
#include <images/Regions/WCCompound.h>
#include <images/Regions/WCBox.h>
#include <images/Regions/WCPolygon.h>
#include <display/Display/DParameterChoice.h>
#include <display/Display/Attribute.h>
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
  
  
  //showHideMenu = new QMenu(this);
  //showHideButton->setMenu(showHideMenu);
  //deleteMenu = new QMenu(this);
  //deleteButton->setMenu(deleteMenu);

  cleanup();

  //loadRegionFromImage();
  //drawing_mode->clear();
  //drawing_mode->addItem("union");

 
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
  connect(qdp_, SIGNAL(animatorChange()),  
                SLOT(zPlaneChanged()) );
  connect(qdp_, SIGNAL(activate(Record)),  
                SLOT(activate(Record)) );
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
  //connect(chanExt, SIGNAL(clicked()), 
  //                 SLOT(extendChan()));
  //connect(polExt, SIGNAL(clicked()), 
  //                SLOT(extendPol()));  
  connect(helpButton, SIGNAL(clicked()),
                      SLOT(showHelp()));
  connect(helpAct, SIGNAL(clicked()),
                      SLOT(showHelpActive()));
  connect(resetregions, SIGNAL(clicked()), 
                        SLOT(cleanup()));
  connect(loadFile, SIGNAL(clicked()), 
                    SLOT(loadRegionFromFile()));
  connect(showOrHide, SIGNAL(clicked()), 
                      SLOT(toggleImageRegion()));
  connect(regName, 
          SIGNAL(currentIndexChanged(const QString &)), 
          SLOT(currentRegionChanged(const QString &)));

  connect(deleteButton,
          SIGNAL(clicked()),
          SLOT(deleteActiveBox()));
  connect(insertButton,
          SIGNAL(clicked()),
          SLOT(insertActiveBox()));

  connect(exportButton,
          SIGNAL(clicked()),
          SLOT(exportRegions()));

  planeOnlyCB->setChecked(True); 
  planeOnlyCB->setChecked(False); 
  planeOnlyCB->hide();
  chanExt->setEnabled(True);
  polExt->setEnabled(True);
  lineEdit->setReadOnly(true);

  deleteButton->setEnabled(false);
  insertButton->setEnabled(false);
 
  activeGroup = "";
  activeBox = -1;
  activeShape = 0;
  timer = new QTimer(this);
  flash = false;
  connect(timer,
          SIGNAL(timeout()),
          SLOT(flashActive()));

  cb = 1;
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
        if (qdds.len() == 0)
           return;
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


  WCUnion* QtRegionManager::unfoldCompositeRegionToSimpleUnion(
            const WCRegion*& wcreg){
    PtrBlock<const WCRegion* > outRegPtrs ;
    unfoldIntoSimpleRegionPtrs(outRegPtrs, wcreg);
    WCUnion* outputUnion = new WCUnion(True, outRegPtrs);
    return outputUnion;
  }

  void QtRegionManager::unfoldIntoSimpleRegionPtrs(PtrBlock<const WCRegion*>& outRegPtrs, const WCRegion*& wcreg){
     if((wcreg->type()) == "WCBox"){
       uInt nreg=outRegPtrs.nelements();
       outRegPtrs.resize(nreg+1);
       outRegPtrs[nreg]=new WCBox(static_cast<const WCBox & >(*wcreg));
     }
     else if((wcreg->type()) == "WCPolygon"){
       uInt nreg=outRegPtrs.nelements();
       outRegPtrs.resize(nreg+1);
       outRegPtrs[nreg]=new WCPolygon(static_cast<const WCPolygon & >(*wcreg));
     }
     else if((wcreg->type()) == "WCUnion" ||(wcreg->type()) == "WCIntersection" ){
       PtrBlock<const WCRegion*> regPtrs=(static_cast<const WCCompound* >(wcreg))->regions();
      //cout << "number of wcregions " << regPtrs.nelements() << endl;
      for (uInt j=0; j < regPtrs.nelements(); ++j){
	unfoldIntoSimpleRegionPtrs(outRegPtrs, regPtrs[j]);
      }

     }

  }

  void QtRegionManager::addRegionsToShape(RSComposite*& theShapes, const WCRegion*& wcreg){
    //cout << "WCRegion->comment=" 
    //     << wcreg->comment() << endl; 
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
    String rest = wcreg->comment();
    String chan = rest.before("polExt:");
    chan = chan.after("chanExt:");
    String pola = rest.after("polExt:");
    //cout << "chanExt=" << chan << endl;
    //cout << "polExt=" << pola << endl;
    Attribute oldState("chan", chan);
    Attribute newState("pola", pola);
    theShapes->setRestriction(oldState);
    theShapes->setRestriction(newState);

  }
  void QtRegionManager::drawRegion(
      Record mousereg, WorldCanvasHolder *wch){
    //cout << "drawRegion ..." << endl;
 
    if(this->isVisible() && mousereg.isDefined("world")){
      String type=mousereg.asString("type");
      List<QtDisplayData*> DDs = qdp_->registeredDDs();
      ListIter<QtDisplayData*> qdds(DDs);
      if (qdds.len() == 0)
         return;
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
          if (qdds.len() == 0)
             return;
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
            } else {                                                         
               IPosition fixedPos(ndim); 
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
       if (qdds.len() == 0)
          return;
       qdds.toEnd();
       qdds--;
       QtDisplayData* qdd = qdds.getRight();
       
       sName = "File: " + sName;
       regName->addItem(sName);

       //addRegionToMenu(sName, "File");

       //cout << "------imageRegion\n" 
       //     << reg->toRecord("ab") << endl;
       regData[sName] = regionToShape(qdd, reg);
       regState[sName] = false;
       currentRegionChanged(sName);

       savedName->setText("");
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

  void QtRegionManager::zPlaneChanged(){

    //DisplayData *dd = getImageData(regName->currentText());
    //DisplayData *dd = getBoundingBoxData(
    //                  regName->currentText());
   
    int zIndex = 0;
    List<QtDisplayData*> DDs = qdp_->registeredDDs();
    ListIter<QtDisplayData*> qdds(DDs);
    if (qdds.len() > 0) {
       qdds.toEnd();
       qdds--;
       QtDisplayData* qdd = qdds.getRight();
       zIndex = qdd->dd()->activeZIndex();
    }
    //cout << "zIndex=" << zIndex << endl;

    //cout << "regData count=" << regData.count() << endl;
    QList<QString> kys = regData.keys();
    for (int i = 0; i < kys.size(); ++i) {

    QString sName = kys.at(i);
    //cout << "toggle region=" << sName.toStdString() 
    //     << endl;
    if (sName == "")
       continue;

    DisplayData *dd = regData[sName];
    //cout << "DisplayData dd=" << dd 
    //     << "\n"  
    //     << ((RegionShape*)dd)->toRecord() << endl;
  
    if (!dd) 
       continue;

    String chan;
    String pola;
    AttributeBuffer* buf = dd->restrictionBuffer();
    if (!(buf && buf->getValue("chan", chan)))
       chan = "";
    if (!(buf && buf->getValue("pola", pola)))
       pola = "";
    //cout << "chan=" << chan << " pola=" << pola << endl;

    bool allow = planeAllowed(zIndex, chan, pola);
    //cout << "allow=" << allow << endl;

    if (!allow) {
       qdp_->hold();
       qdp_->panelDisplay()->removeDisplayData(*dd);
       qdp_->release();
       if (sName==regName->currentText())
          showOrHide->setEnabled(false);
       continue;
    }
       
    if (sName==regName->currentText())
       showOrHide->setEnabled(true);

    if (regState[sName] == false) {
       //qdp_->registerRegionShape((RegionShape*)dd);
       qdp_->hold();
       qdp_->panelDisplay()->removeDisplayData(*dd);
       qdp_->release();
       //cout << sName.toStdString() 
       //     << " was not shown, is shown now" << endl;
    } 
    else {
       //qdp_->registerRegionShape((RegionShape*)dd);
       qdp_->hold();
       qdp_->panelDisplay()->addDisplayData(*dd);
       qdp_->release();
       //cout << sName.toStdString() 
       //     << " was shown, is still shown" << endl;
    }

    }

  }
  void QtRegionManager::toggleImageRegion(){

    //DisplayData *dd = getImageData(regName->currentText());
    //DisplayData *dd = getBoundingBoxData(
    //                  regName->currentText());
   
    int zIndex = 0;
    List<QtDisplayData*> DDs = qdp_->registeredDDs();
    ListIter<QtDisplayData*> qdds(DDs);
    if (qdds.len() > 0) {
       qdds.toEnd();
       qdds--;
       QtDisplayData* qdd = qdds.getRight();
       zIndex = qdd->dd()->activeZIndex();
    }
    //cout << "zIndex=" << zIndex << endl;

    QString sName = regName->currentText();
    //cout << "toggle region=" << sName.toStdString() 
    //     << endl;
    if (sName == "")
       return;

    //cout << "regData count=" << regData.count() << endl;
    DisplayData *dd = regData[regName->currentText()];
    //cout << "DisplayData dd=" << dd 
    //     << "\n"  
    //     << ((RegionShape*)dd)->toRecord() << endl;
  
    if (!dd) 
       return;

    String chan;
    String pola;
    AttributeBuffer* buf = dd->restrictionBuffer();
    if (!(buf && buf->getValue("chan", chan)))
       chan = "";
    if (!(buf && buf->getValue("pola", pola)))
       pola = "";
    //cout << "chan=" << chan << " pola=" << pola << endl;

       
    bool allow = planeAllowed(zIndex, chan, pola);
    //cout << "allow=" << allow << endl;

    if (!allow) {
       showOrHide->setEnabled(false);
       qdp_->hold();
       qdp_->panelDisplay()->removeDisplayData(*dd);
       qdp_->release();
       regState[sName] = false;
       showOrHide->setEnabled(false);
       return;
    }

    showOrHide->setEnabled(true);
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
    }
  }
  
  void QtRegionManager::extendChan(){
      chan_sel->setEnabled(True);
      chan_sel->setText("");
      //planeOnlyCB->setChecked(False);
  }

  void QtRegionManager::extendPol(){
      pol_sel->setEnabled(True);
      pol_sel->setText("");
      //planeOnlyCB->setChecked(False);
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
    if (qdds.len() == 0)
       return;
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
       
          //cout << "======imageRegion\n" 
          //     << reg->toRecord("ab") << endl;
          QString sName = regionNames(kk).c_str();
          sName = "Image: " + sName;
          regName->addItem(sName);

          //addRegionToMenu(sName, "Image");
           
          regData[sName] = regionToShape(qdd, reg);
          regState[sName] = false;
          currentRegionChanged(sName);
       }
    }
  }

}

void QtRegionManager::addRegionToMenu(
      const QString& sName, const QString& source) {
   QAction *action = new QAction(source + ": " + sName, showHideMenu);
   action->setCheckable(True);
   action->setChecked(True);
   showHideMenu->addAction(action);
   connect(action, SIGNAL(triggered()), SLOT(showHideRegion()));

   action = new QAction(source + ": " + sName, deleteMenu);
   action->setCheckable(False);
   deleteMenu->addAction(action);
   connect(action, SIGNAL(triggered()), SLOT(deleteRegion()));
}


void QtRegionManager::removeRegion() {

  QString sName = regName->currentText();
  //qDebug() << "sName=" << sName;
  QString bName = sName.section(" ", -1);
  //qDebug() << "bName=" << bName;

  String regname(bName.toStdString());
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
  insertButton->setEnabled(false);
  deleteButton->setEnabled(false);
}

void QtRegionManager::newRegion_(String imgFilename) {
  //cout << "newRegion..." << endl;

  if(this->isVisible()){
    if(qdp_->hasRegion()){
      //if ((drawing_mode->currentText()).contains("union")){
	
	uInt regNo=unionRegions_p.nelements();
	unionRegions_p.resize(regNo+1);
	unionRegions_p[regNo]=new const 
              ImageRegion(qdp_->lastRegion()); 
	//cout << "regNo=" << regNo << " WCRegion=" 
        //     << unionRegions_p[regNo]->isWCRegion() 
        //     << "   LCRegion=" 
        //     << unionRegions_p[regNo]->isLCRegion() 
        //     << endl;

      //}

      /*
      ListIter<RegionShape*> rgiter(regShapes_p);
      rgiter.toEnd();
      while(!rgiter.atStart()) {
          rgiter--;
          if(qdp_->isRegistered(rgiter.getRight())){
            qdp_->unregisterRegionShape(rgiter.getRight());
          }
          rgiter.removeRight();
      }
      */
      insertButton->setEnabled(activeBox > -1);
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
      leUnion.setComment("chanExt:" + 
                   chan_sel->text().toStdString() +
                   "polExt:" +
                   pol_sel->text().toStdString());
      ImageRegion* reg = new ImageRegion(leUnion);

      try{
         AipsIO os(regname, ByteIO::NewNoReplace);
         os << reg->toRecord(regname+".tbl");  
         cleanup();
         savedName->setText("");
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
  if (qdds.len() == 0)
     return;
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
     leUnion.setComment("chanExt:" + 
                  chan_sel->text().toStdString() +
                  "polExt:" +
                  pol_sel->text().toStdString());
     ImageRegion* reg = new ImageRegion(leUnion);
     //cout << "to be save ImageRegion:\n"  
     //     << reg->toRecord("arbitrary") << endl;
     qdp_->saveRegionInImage(regname, *reg);
    
     sName = "Image: " + sName;
     regName->addItem(sName);

     //addRegionToMenu(sName, "Image");

     regData[sName] = regionToShape(qdd, reg);
     regState[sName] = false;
     currentRegionChanged(sName);
     cleanup();

     savedName->setText("");

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

  List<QtDisplayData*> DDs = qdp_->registeredDDs();
  ListIter<QtDisplayData*> qdds(DDs);
  if (qdds.len() == 0)
     return;

  qdds.toEnd();
  qdds--;
  QtDisplayData* qdd = qdds.getRight();

  int zIdx = qdd->dd()->activeZIndex();
  //cout << "zIndex=" << zIdx << endl;


  //DisplayData *dd = getImageData(i);
  //DisplayData *dd = getBoundingBoxData(i);
  DisplayData *dd = regData[i];

  if (!dd) 
     return;

  //cout << "add DisplayData " << dd 
  //     << "\n"
  //     << ((RegionShape*)dd)->toRecord() << endl;

  String chan;
  String pola;
  AttributeBuffer* buf = dd->restrictionBuffer();
  if (!(buf && buf->getValue("chan", chan)))
     chan = "";
  if (!(buf && buf->getValue("pola", pola)))
     pola = "";
  //cout << "chan=" << chan << " pola=" << pola << endl;
  chan_sel->setText(chan.c_str());
  pol_sel->setText(pola.c_str());

  if (planeAllowed(zIdx, chan, pola)) {

     if (regState[i] == true) {
        showOrHide->setText("Hide");
        showOrHide->setEnabled(true);
        return;
     }
     
     //this activates the selected image region 
     //if the region is not in the region list
     //create the dd and add (name, dd) to the hash
     //show it by put it on pd directly

     //qdp_->registerRegionShape((RegionShape*)dd);
     qdp_->hold();
     qdp_->panelDisplay()->addDisplayData(*dd);
     qdp_->release();

     showOrHide->setText("Hide");
     showOrHide->setEnabled(true);
     regState[i] = true;
 }
  else {
     //qdp_->unregisterRegionShape((RegionShape*)dd);
     qdp_->hold();
     qdp_->panelDisplay()->removeDisplayData(*dd);
     qdp_->release();

     showOrHide->setText("Show");
     showOrHide->setEnabled(false);
     regState[i] = false;
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
  if (qdds.len() == 0)
     return 0;
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
  if (qdds.len() == 0)
     return 0;
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

bool QtRegionManager::planeAllowed(
         int idx, String& xa, String& ya) {
   xa.gsub(" ", "");
   ya.gsub(" ", "");
   if (xa.length() == 0)
      return true;
   String w[10];
   Int nw = split(xa, w, 10, ',');
   for (Int k = 0; k < nw; k++) {
      String x[2];
      Int nx = split(w[k], x, 2, '~');
      if (nx == 2 && 
          idx >= atoi(x[0].c_str()) && 
          idx <= atoi(x[1].c_str())) {
         return true;
      }
      if (nx == 1 && atoi(x[0].c_str()) == idx) {
         return true;
      }
   }
   return false;
}

void QtRegionManager::showHelp() {
    QMessageBox::information(this, "QtRegionManager",
      "Steps to create an image region (group):\n"
      "  1. display the image;\n"
      "  2. assign a region tool (rectangle or polygon) "
            "to a mouse button;\n"
      "  3. draw a region on the displayed image;\n"
      "  4. adjust the size or shape by dragging "
            "the corner handle;\n"
      "  5. ajust the position by dragging the inside;\n"
      "  6. record the region by double clicking inside;\n"
      "  7. repeat 3~6 as needed;\n"
      "  8. set the region extend channels and "
            "polarizations, example:1,5~10;\n"
      "  9. set the region (group) name to be saved;\n"
      " 10. save the region in the image or to a file.");

}

void QtRegionManager::showHelpActive() {
    QMessageBox::information(this, "QtRegionManager",
      "Steps to manipulate an image region:\n"
      "  1. assign a region tool (rectangle or polygon) "
            "to a mouse button;\n"
      "  2. show the image region;\n"
      "  3. click inside a region. the regions in the\n"
      "     group are marked red. the active region is flashing;\n"
      "  4. draw new regions with mouse. click 'insert'\n"
      "     to add the new regions into the group.\n"
      "  5. click 'delete' to remove the active region\n"
      "     from the group.\n"
      "  6. use 3~5 for moving or resizing a region.\n");

}

void QtRegionManager::changeAxis(
         String xa, String ya, String za) {
   //cout << "change axis=" << xa << " " << ya
   //     << " " << za << endl;
   int ccb = 0;
   if (xa.contains("Decl") && ya.contains("Right"))
       ccb = -1;
   if (xa.contains("Right") && ya.contains("Decl"))
       ccb = 1;
   if (xa.contains("atitu") && ya.contains("ongitu"))
       ccb = -1;
   if (xa.contains("ongitu") && ya.contains("atitu"))
       ccb = 1;

   if (cb != ccb && ccb != 0) {
      rotateBox(ccb);
   }
   cb = ccb;
   if (cb == 0) {
     //qDebug() << "turn off boxes";
   }
}

void QtRegionManager::exportRegions() {
   QList<QString> kys = regData.keys();
   for (int k = 0; k < kys.size(); ++k) {
      QString sName = kys.at(k);
      if (sName == "")
         continue;
   
      if ((actGrp->isChecked() || actOne->isChecked()) && 
          sName != activeGroup)
         continue;

      DisplayData *dd = regData[sName];
     
      if (!dd) 
         continue;
   
      Record ddRec = ((RegionShape*)dd)->toRecord();
      //cout << sName.toStdString() << " region shape="
      //     << "\n"  << ddRec << endl;

      if (!ddRec.asBool("isworld") ||
          !ddRec.asString("type").contains("composite") ||
          !ddRec.asBool("dependentchildren") ||
          !ddRec.isDefined("children"))
         continue;
        
      RecordFieldPtr<Record> children(ddRec, "children");
      Record& reg = *children;
      Int sz = reg.size();
      Int i = 0;
      cout << "Image Region === " << sName.toStdString() << endl; 
      for (; i < sz; i++) {
         if (actOne->isChecked() && activeBox != i)
            continue;

         RecordFieldPtr<Record> nth(reg, i);
         Record& sub = *nth;
         Vector<Double> xs;
         Vector<Double> ys;
         if (sub.asString("type").contains("polygon")){
	    xs = sub.asArrayDouble("xcoords");
	    ys = sub.asArrayDouble("ycoords");	 
            cout << i << " polygon\n"
                 << "   " << xs << "\n   " << ys << endl;
         }
         if (sub.asString("type").contains("rectangle")){
	    xs = sub.asArrayDouble("coordinates");
            cout << i << " rectangle\n"
                 << "   " << xs << endl;
         }
      }
   }
}

void QtRegionManager::rotateBox(int cb) {
   //qDebug() << "rotate cb=" << cb ;
   QList<QString> kys = regData.keys();
   for (int k = 0; k < kys.size(); ++k) {
      QString sName = kys.at(k);
      if (sName == "")
         continue;
   
      DisplayData *dd = regData[sName];
     
      if (!dd) 
         continue;
   
      qdp_->hold();
      qdp_->panelDisplay()->removeDisplayData(*dd);
      qdp_->release();

      Record ddRec = ((RegionShape*)dd)->toRecord();
      //cout << sName.toStdString() << " region shape="
      //     << "\n"  << ddRec << endl;

      if (!ddRec.asBool("isworld") ||
          !ddRec.asString("type").contains("composite") ||
          !ddRec.asBool("dependentchildren") ||
          !ddRec.isDefined("children"))
         continue;
        
      RecordFieldPtr<Record> children(ddRec, "children");
      Record& reg = *children;
      Int sz = reg.size();
      Int i = 0;
      for (; i < sz; i++) {
         RecordFieldPtr<Record> nth(reg, i);
         Record& sub = *nth;
         Vector<Double> xs;
         Vector<Double> ys;
         if (sub.asString("type").contains("polygon")){
	    xs = sub.asArrayDouble("xcoords");
	    ys = sub.asArrayDouble("ycoords");	 
            sub.define("xcoords", ys);
            sub.define("ycoords", xs);
         }
         if (sub.asString("type").contains("rectangle")){
	    xs = sub.asArrayDouble("coordinates");
	    ys = xs;	 
            ys(0) = xs(1);
            ys(1) = xs(0); 
            ys(2) = xs(3);
            ys(3) = xs(2); 
            sub.define("coordinates", ys);
         }
      }
      //cout << sName.toStdString() << " rotated region shape="
      //     << "\n"  << ddRec << endl;
      RegionShape* ee = 0;
      String err = "";
      ee = RegionShape::shapeFromRecord(ddRec, err);
      if (err == "") {
         ee->setLineColor("cyan");
         regData[sName] = ee;
         qdp_->hold();
         qdp_->panelDisplay()->addDisplayData(*ee);
         qdp_->release();
      }
   }
}

void QtRegionManager::deleteRegion() {
   QAction* action = dynamic_cast<QAction*>(sender());
   if (action == 0) 
      return;

   QString bName = action->text();
   QString sName = bName.section(" ", -1);
   //qDebug() << "delete ---" << bName;
   //qDebug() << "remove nenu---" << sName;

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

     deleteMenu->removeAction(action);
     
     QList<QAction *> acts = showHideMenu->actions();
     //qDebug() << "remove show/hide menu---" << bName;
     for (int i = 0; i < acts.size(); i++) {
        if (acts.at(i)->text() == bName) {
          showHideMenu->removeAction(acts.at(i));
        }
     }

   }


}

void QtRegionManager::showHideRegion() {
   QAction* action = dynamic_cast<QAction*>(sender());
   if (action == 0) 
      return;

   qDebug() << "show hide-----------" << action->text();
}

void QtRegionManager::insertActiveBox() {
   QString sName = lineEdit->text();
   if (sName == "")
      return;
    
   //bool ok = 
   insertBox(activeGroup);
}

bool QtRegionManager::insertBox(QString& group) {
  //qDebug() << "insert group:" << group;
  if (group == "") 
     return false;

  List<QtDisplayData*> DDs = qdp_->registeredDDs();
  ListIter<QtDisplayData*> qdds(DDs);
  if (qdds.len() == 0)
     return false;
  qdds.toEnd();
  qdds--;
  QtDisplayData* qdd = qdds.getRight();
  
  QString grp = group;
  QString stype = grp.section(":", 0, 0);
  QString sName = grp.section(":", 1, 1);
  sName = sName.trimmed();
  //qDebug() << "stype:" << stype << "sName:" << sName;

  //remove from the screen
  DisplayData *dd = regData[group];
  if (!dd) 
     return false;
  if (regState[group] != true)
     return false;
  if (dd) {
     //qDebug() << "remove dd";
     qdp_->hold();
     qdp_->panelDisplay()->removeDisplayData(*dd);
     qdp_->release();
     dd = 0;
  }


  if (stype == "Image") {
     String regname(sName.toStdString());
     if (qdd->imageInterface() &&
         (qdd->imageInterface())->hasRegion(regname)) {
        //delete region in image
     }

     ImageRegion* prev = (qdd->imageInterface())
         ->getImageRegionPtr(regname);

     if (unionRegions_p.nelements() > 0){
        WCUnion leUnion(unionRegions_p);
        leUnion.setComment("chanExt:" + 
                  chan_sel->text().toStdString() +
                  "polExt:" +
                  pol_sel->text().toStdString());

        const WCRegion* newUnion = new WCUnion(prev->asWCRegion(), leUnion); 
        WCUnion* unfolded = unfoldCompositeRegionToSimpleUnion(newUnion);
        //cout << "regions:" << unfolded->toRecord("") << endl;

        ImageRegion* newReg = new ImageRegion(*unfolded);
        //cout << "new to be saved ImageRegion:\n"  
        //     << newReg->toRecord("arbitrary") << endl;

        qdp_->saveRegionInImage(regname, *newReg);
    
        regData[group] = regionToShape(qdd, newReg);
        regState[group] = false;
        currentRegionChanged(group);
        cleanup();
     }
     
     activeGroup = "";
     activeBox = -1;
     lineEdit->setText("");
     deleteButton->setEnabled(false);
     insertButton->setEnabled(false);
     if (activeShape) {
        qdp_->hold();
        qdp_->panelDisplay()->removeDisplayData(*activeShape);
        qdp_->release();
        activeShape = 0;
     }
     timer->stop();
     return true;
  }
  if(stype == "File"){
    String rName(sName.toStdString());
    ImageRegion* prev = 0;
    TableRecord rec;
    bool ok = true;
    try{
       //cout << "rName=" << rName << endl;
       AipsIO os(rName, ByteIO::Old);
       //os.open(rName);
       os >> rec;  
       //cout << "infile region record:\n" << rec << endl;
       prev = ImageRegion::fromRecord(rec, rName+".tbl");
    }
    catch(...) {
       ok = false;
    }
 
    if (!ok || !prev)
       return false;

    if(unionRegions_p.nelements() > 0){
      WCUnion leUnion(unionRegions_p);
      leUnion.setComment("chanExt:" + 
                   chan_sel->text().toStdString() +
                   "polExt:" +
                   pol_sel->text().toStdString());

      const WCRegion* newUnion = new WCUnion(prev->asWCRegion(), leUnion); 
      WCUnion* unfolded = unfoldCompositeRegionToSimpleUnion(newUnion);
      //cout << "regions:" << unfolded->toRecord("") << endl;

      ImageRegion* newReg = new ImageRegion(*unfolded);
      //cout << "new to be saved ImageRegion:\n"  
      //     << newReg->toRecord("arbitrary") << endl;

      try{
         AipsIO os(rName, ByteIO::Update);
         os << newReg->toRecord(rName+".tbl");  

         regData[group] = regionToShape(qdd, newReg);
         regState[group] = false;
         currentRegionChanged(group);

         cleanup();
      }
      catch(...) { 
         return false;
      }

    }
    activeGroup = "";
    activeBox = -1;
    lineEdit->setText("");
    deleteButton->setEnabled(false);
    insertButton->setEnabled(false);
    if (activeShape) {
       qdp_->hold();
       qdp_->panelDisplay()->removeDisplayData(*activeShape);
       qdp_->release();
       activeShape = 0;
    }
    timer->stop();
    return true;
  }

  return false;
}
void QtRegionManager::deleteActiveBox() {
   QString sName = lineEdit->text();
   if (sName == "")
      return;
    
   //bool ok = 
   deleteBox(activeGroup, activeBox);
}

bool QtRegionManager::deleteBox(QString& group, int comp) {
  //qDebug() << "delete group:" << group << " comp:" << comp; 
  if (comp < 0 || group == "")
     return false;

  List<QtDisplayData*> DDs = qdp_->registeredDDs();
  ListIter<QtDisplayData*> qdds(DDs);
  if (qdds.len() == 0)
     return false;
  
  qdds.toEnd();
  qdds--;
  QtDisplayData* qdd = qdds.getRight();
 
  QString grp = group;
  QString stype = grp.section(":", 0, 0);
  QString sName = grp.section(":", 1, 1);
  sName = sName.trimmed();
  QString ttl = lineEdit->text().section(":", -1);
  //qDebug() << "stype:" << stype << "sName:" 
  //         << sName << "ttl:" << ttl;

  //remove from the screen
  DisplayData *dd = regData[group];
  if (!dd) 
     return false;
  if (regState[group] != true)
     return false;

  if (dd) {
     //qDebug() << "remove dd";
     qdp_->hold();
     qdp_->panelDisplay()->removeDisplayData(*dd);
     qdp_->release();
     dd = 0;
  }

  //QMessageBox::warning(this, "QtRegionManager",
  // 		    "watch this region\n");

  if (stype == "Image") {
     String regname(sName.toStdString());
     if (qdd->imageInterface() &&
         (qdd->imageInterface())->hasRegion(regname)) {
        //delete region in image
     }

     ImageRegion* prev = (qdd->imageInterface())
         ->getImageRegionPtr(regname);

     const WCRegion* wcr = prev->asWCRegionPtr();
     PtrBlock<const WCRegion*> regPtrs=
          (static_cast<const WCCompound* >(wcr))->regions();
     regPtrs.remove(comp);
     WCUnion nu(True, regPtrs);
     ImageRegion newReg(nu);
     qdp_->saveRegionInImage(regname, newReg);
  

     if (ttl == "1") {
        //no more left, delete this group
        //cout << "remove region " << regname << endl;  
        if (regname != ""){
           qdp_->removeRegionInImage(regname);
        }

        regState.remove(group);
        regData.remove(group);
        int k = regName->findText(group);
        if (k > -1)
           regName->removeItem(k);
     }
     else {
        regData[group] = regionToShape(qdd, &newReg);
        regState[group] = false;
        currentRegionChanged(group);
        cleanup();
     }
  }
  if (stype == "File") {
    String rName(sName.toStdString());
    ImageRegion* prev = 0;
    TableRecord rec;
    bool ok = true;
    try{
       //cout << "rName=" << rName << endl;
       AipsIO os(rName, ByteIO::Old);
       //os.open(rName);
       os >> rec;  
       //cout << "infile region record:\n" << rec << endl;
       prev = ImageRegion::fromRecord(rec, rName+".tbl");
    }
    catch(...) {
       ok = false;
    }
 
    if (!ok || !prev)
       return false;

    const WCRegion* wcr = prev->asWCRegionPtr();
    PtrBlock<const WCRegion*> regPtrs=
          (static_cast<const WCCompound* >(wcr))->regions();
    regPtrs.remove(comp);
    WCUnion nu(True, regPtrs);
    ImageRegion newReg(nu);

    try{
       AipsIO os(rName, ByteIO::Update);
       os << newReg.toRecord(rName+".tbl");  

       regData[group] = regionToShape(qdd, &newReg);
       regState[group] = false;
       currentRegionChanged(group);

       cleanup();
    }
    catch(...) { 
       return false;
    }

    if (ttl == "1") {
       //no more left, delete this group
       //delete file ?

       regState.remove(group);
       regData.remove(group);
       int k = regName->findText(group);
       if (k > -1)
          regName->removeItem(k);
    }
    else {
       regData[group] = regionToShape(qdd, &newReg);
       regState[group] = false;
       currentRegionChanged(group);
       cleanup();
    }
  }

  activeGroup = "";
  activeBox = -1;
  lineEdit->setText("");
  deleteButton->setEnabled(false);
  insertButton->setEnabled(false);
  if (activeShape) {
     qdp_->hold();
     qdp_->panelDisplay()->removeDisplayData(*activeShape);
     qdp_->release();
     activeShape = 0;
  }
  timer->stop();
   
  return true;
}

void QtRegionManager::flashActive() {
   if (activeGroup == "" || activeBox < 0 || !activeShape)
      return;
   flash = !flash;
   String clr = (flash) ? "green" : "black";
   qdp_->hold();
   qdp_->panelDisplay()->removeDisplayData(*activeShape);
   activeShape->setLineColor(clr);
   qdp_->panelDisplay()->addDisplayData(*activeShape);
   qdp_->release();
}

void QtRegionManager::activate(Record rec) {
   //cout  << "activate " << rec;
   
   int zIndex = 0;
   Vector<Double> wx(2);
   wx  = -1000;

   String tool = rec.asString("tool");
   if (this->isVisible() && rec.isDefined("world")){
      Record world = rec.asRecord("world");
      Vector<Double> wld = world.asArrayDouble("wld");
      Vector<String> units = world.asArrayString("units");
   
      List<QtDisplayData*> DDs = qdp_->registeredDDs();
      ListIter<QtDisplayData*> qdds(DDs);
      if (qdds.len() > 0) {
         qdds.toEnd();
         qdds--;
         QtDisplayData* qdd = qdds.getRight();
         zIndex = qdd->dd()->activeZIndex();
   
         if ((qdd->imageInterface())){
            CoordinateSystem csys=(qdd->imageInterface())->coordinates();
            //Int dirInd=csys.findCoordinate(Coordinate::DIRECTION);
            //MDirection::Types dirType=csys.directionCoordinate(dirInd)
            //                      .directionType(True);
            wx(0) = Quantity(wld(0), units(0)).getValue(RegionShape::UNIT);
   	    wx(1) = Quantity(wld(1), units(1)).getValue(RegionShape::UNIT);
         }
      }
   }
   //cout << "zIndex=" << zIndex << endl;
   //cout << "wx=" << wx << endl;
 
   if (wx(0) == -1000 && wx(0) == -1000)
      return;

   //convert to linear to compare
   //
   //WorldCanvas* wc = 0;
   //ListIter<WorldCanvas* >* wcs = qdp_->panelDisplay()->wcs();
   //wcs->toStart();
   //if (!wcs->atEnd()) { 
   //  wc = wcs->getRight();
   //}
   //if (wc == 0)
   //   return;
   //Vector<Double> lin(2);
   //wc->worldToLin(lin, wx);
   //cout << "lin=" << lin << endl;


   //cout << "regData count=" << regData.count() << endl;
   QList<QString> kys = regData.keys();
   for (int k = 0; k < kys.size(); ++k) {
      bool active = false; 
      QString sName = kys.at(k);
      //cout << "toggle region=" << sName.toStdString() 
      //     << endl;
      if (sName == "")
         continue;
   
      DisplayData *dd = regData[sName];
     
      if (!dd) 
         continue;
   
      String chan;
      String pola;
      AttributeBuffer* buf = dd->restrictionBuffer();
      if (!(buf && buf->getValue("chan", chan)))
         chan = "";
      if (!(buf && buf->getValue("pola", pola)))
         pola = "";
      //cout << "chan=" << chan << " pola=" << pola << endl;
   
      bool allow = planeAllowed(zIndex, chan, pola);
      //cout << "allow=" << allow << endl;
   
      if (!allow)
         continue;
      
      if (regState[sName] != true)
         continue;

      //we only need to care about this dd;
      Record ddRec = ((RegionShape*)dd)->toRecord();
      //cout << "DisplayData dd=" << dd 
      //     << "\n"  << ddRec << endl;

      if (!ddRec.asBool("isworld") ||
          !ddRec.asString("type").contains("composite") ||
          !ddRec.asBool("dependentchildren") ||
          !ddRec.isDefined("children"))
         continue;
        
      RecordFieldPtr<Record> children(ddRec, "children");
      Record& reg = *children;
      //cout << reg ;
      Int sz = reg.size();
      //cout << "sz=" << sz << endl;
         
      Int i = 0;
      for (i = 0; i < sz; i++) {
         //lineEdit->setText("");
         RecordFieldPtr<Record> nth(reg, i);
         Record& sub = *nth;
         Vector<Double> xs;
         Vector<Double> ys;
        
         if (sub.asString("type").contains("polygon") &&
             tool.contains("olygon")){
            //cout << "polygon--------" << endl;
	    xs = sub.asArrayDouble("xcoords");
	    ys = sub.asArrayDouble("ycoords");	 
            Double xd = -10000;
            Double yd = -10000;
            Double xc = -xd;
            Double yc = -yd;
            for (uInt j = 0; j < xs.nelements(); j++) {
               xd = fmax(xd, xs(j));
               yd = fmax(yd, ys(j));
               xc = fmin(xc, xs(j));
               yc = fmin(yc, ys(j));
            }
            if (xc <= wx(0) && wx(0) <= xd &&
                yc <= wx(1) && wx(1) <= yd) {
               active = true;
            }
         }
         if (sub.asString("type").contains("rectangle") &&
             tool.contains("ectangle")){
            //cout << "rectangle--------" << endl;
	    xs = sub.asArrayDouble("coordinates");
            xs.resize(4, true);
            Double xd = xs(2) / 2.0;
            Double yd = xs(3) / 2.0;
            Double xc = xs(0);
            Double yc = xs(1);
            xs.resize(2);
	    ys = xs;	 
            xs(0) = xc - xd;
            ys(0) = yc - yd; 
            xs(1) = xc + xd;
            ys(1) = yc + yd; 
            if (xs(0) <= wx(0) && wx(0) <= xs(1) &&
                ys(0) <= wx(1) && wx(1) <= ys(1)) {
                active = true;
            }
         }

         if (active) {
            //qDebug() << "activeGroup:" << activeGroup 
            //         << " activeBox:" << activeBox; 
            if (activeGroup != "" && activeBox > -1) {

                timer->stop();
                if (activeShape) {
                   qdp_->hold();
                   qdp_->panelDisplay()->removeDisplayData(*activeShape);
                   qdp_->release();
                   activeShape = 0;
                }

                DisplayData *dd = regData[activeGroup];
     
                if (activeGroup!=sName && dd && regState[activeGroup]) {
                   qdp_->hold();
                   qdp_->panelDisplay()->removeDisplayData(*dd);
                   qdp_->release();
                   
                   Record ddRec = ((RegionShape*)dd)->toRecord();
                   if (ddRec.asBool("isworld") && 
                       ddRec.asString("type").contains("composite") &&
                       ddRec.asBool("dependentchildren")) {
        
                      RegionShape* ee = 0;
                      String err = "";
                      ee = RegionShape::shapeFromRecord(ddRec, err);
                      if (err == "") {
                         ee->setLineColor("cyan");
                         regData[activeGroup] = ee;
                         qdp_->hold();
                         qdp_->panelDisplay()->addDisplayData(*ee);
                         qdp_->release();
                      }
                   }
                }
             }
             activeGroup = "";
             activeBox = -1;

            //QMessageBox::warning(this, "QtRegionManager",
            // 		    "after remove\n");
                         

            //cout << "sub=" << sub << endl;
            String err = "";
            activeShape = RegionShape::shapeFromRecord(sub, err);
            if (err != "") {
               activeShape = 0;
            }
            if (activeShape) {
               timer->start(1000);
            }


            //unfortunately, RegionShape composite does not 
            //handle individual color for each box
            //
            //sub.define("line_color", "red");
            //RecordFieldPtr<String> clr (sub, "line_color");
            //*clr = "red";
            RecordFieldPtr<String> clr (ddRec, "line_color");
            *clr = "red";

            //unfortunately, this will get overwritten when
            //reconstruct the RegionShape
            //
            //ddRec.define("active", i);

            //cout << xs << endl;
            //cout << ys << endl;
            lineEdit->setText(QString(" %1::%2:%3")
                     .arg(sName).arg(i + 1).arg(sz)); 
            break;
         }
         
         //convert to linear and compare vertices
         //Vector<Double> wy(2);
         //for (Int j = 0; j < xs.nelements(); j++) {
         //   wy(0) = xs(j);
         //   wy(1) = ys(j);
         //   wc->worldToLin(wx, wy);
         //   cout << "wy=" << wy << endl;
         //   cout << "wx=" << wx << endl;
         //   if (fabs(wx(0) - lin(0)) < 5 && fabs(wx(1) - lin(1)) < 5) {
         //      cout << "activate --------" << endl;
         //   }
         //}

         //directly compare by world point by point is difficult!
         //Vector<Double> wy(2);
         //for (uInt j = 0; j < xs.nelements(); j++) {
         //   if (fabs(wx(0) - xs(j)) < 0.05 && 
         //       fabs(wx(1) - xs(j)) < 0.05) {
         //      cout << "activate --------" << endl;
         //   }
         //}
      }
      if (active) { 

         RegionShape* ee = 0;
         String err = "";
         ee = RegionShape::shapeFromRecord(ddRec, err);
         //cout << "ee=" << ee <<  endl;
         if (err == "") {
            regData[sName] = ee;
            qdp_->hold();
            qdp_->panelDisplay()->removeDisplayData(*dd);

            //already marked red by change the record
            //ee->setLineColor("red");
            //cout << "changed color ee=" << ee 
            //     << "\n"  << ((RegionShape*)ee)->toRecord() << endl;

            qdp_->panelDisplay()->addDisplayData(*ee);
            qdp_->release();
         }
         else {
            cout << "Error convert from record to shape: "
                 << err << endl;
         }
         activeGroup = sName;
         activeBox = i;
         deleteButton->setEnabled(true);
         insertButton->setEnabled(unionRegions_p.nelements() > 0);
         break;
      }
   }
}

} //# NAMESPACE CASA - END
