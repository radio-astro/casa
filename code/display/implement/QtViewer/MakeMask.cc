//# MakeMask.cc: Qt implementation of viewer mask maker window.
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


#include <display/QtViewer/MakeMask.qo.h>
#include <display/RegionShapes/RegionShapes.h>
#include <display/QtViewer/QtDisplayData.qo.h>


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


namespace casa {
  
MakeMask::MakeMask(QtDisplayPanel* qdp) {

  qdp_ = qdp;

  regData = 0;

  activeShape = 0;

  timer = new QTimer();
  flash = false;

  cb = 1;
 
  setWindowTitle("FILEBOX");
  QVBoxLayout *layout = new QVBoxLayout;

  setLayout(layout);
  setMinimumSize(210, 180);

  tGroup = new QGroupBox;
  QGridLayout *tLayout = new QGridLayout;
  tGroup->setLayout(tLayout);

  load = new QPushButton("Load");
  tLayout->addWidget(load, 0, 0, 1, 3);
  connect(load, SIGNAL(clicked()), SLOT(loadRegionFromFile()));

  save = new QPushButton("Save");
  tLayout->addWidget(save, 0, 3, 1, 3);
  connect(save, SIGNAL(clicked()), SLOT(saveRegionToFile()));

  removeAll = new QPushButton("CleanUp");
  tLayout->addWidget(removeAll, 1, 0, 1, 2);
  connect(removeAll, SIGNAL(clicked()), SLOT(deleteAll()));

  showHide = new QPushButton("Hide");
  tLayout->addWidget(showHide, 1, 2, 1, 2);
  connect(showHide, SIGNAL(clicked()), SLOT(showHideAll()));

  color = new QComboBox;
  color->addItem("Cyan");
  color->addItem("Red");
  color->addItem("Blue");
  color->addItem("Yellow");
  color->addItem("White");
  color->addItem("Black");
  tLayout->addWidget(color, 1, 4, 1, 2);
  connect(color, SIGNAL(currentIndexChanged(const QString&)),
                 SLOT(colorAll(const QString&)));

  tLayout->addWidget(new QLabel("channels"), 2, 0, 1, 2);
  chan = new QLineEdit;
  tLayout->addWidget(chan, 2, 2, 1, 4);

  tLayout->addWidget(new QLabel("correlations"), 3, 0, 1, 2);
  corr = new QLineEdit;
  tLayout->addWidget(corr, 3, 2, 1, 4);

/*
  eGroup = new QGroupBox("Edit");
  QGridLayout *eLayout = new QGridLayout;
  eGroup->setLayout(eLayout);

  remove = new QPushButton("Delete");
  eLayout->addWidget(remove, 0, 0, 1, 1);
  connect(remove, SIGNAL(clicked()), SLOT(doIt()));

  shape = new QComboBox; 
  shape->addItem("Square");
  shape->addItem("Recangle");
  shape->addItem("Circle");
  shape->addItem("Ellipse");
  shape->addItem("Polygon");
  eLayout->addWidget(shape, 0, 1, 1, 1);
  connect(shape, SIGNAL(currentIndexChanged(const QString&)),
                 SLOT(reShape(const QString&)));

  rotateL = new QPushButton("D");
  eLayout->addWidget(rotateL, 1, 0, 1, 1);
  connect(rotateL, SIGNAL(clicked()), SLOT(doIt()));

  rotateR = new QPushButton("R");
  eLayout->addWidget(rotateR, 1, 1, 1, 1);
  connect(rotateR, SIGNAL(clicked()), SLOT(doIt()));

  left = new QPushButton("Left");
  eLayout->addWidget(left, 2, 0, 1, 1);
  connect(left, SIGNAL(clicked()), SLOT(doIt()));

  right = new QPushButton("Right");
  eLayout->addWidget(right, 2, 1, 1, 1);
  connect(right, SIGNAL(clicked()), SLOT(doIt()));

  up = new QPushButton("Up");
  eLayout->addWidget(up, 3, 0, 1, 1);
  connect(up, SIGNAL(clicked()), SLOT(doIt()));

  down = new QPushButton("Down");
  eLayout->addWidget(down, 3, 1, 1, 1);
  connect(down, SIGNAL(clicked()), SLOT(doIt()));

  wider = new QPushButton("Wider");
  eLayout->addWidget(wider, 4, 0, 1, 1);
  connect(wider, SIGNAL(clicked()), SLOT(doIt()));

  narrower = new QPushButton("Narrower");
  eLayout->addWidget(narrower, 4, 1, 1, 1);
  connect(narrower, SIGNAL(clicked()), SLOT(doIt()));

  taller = new QPushButton("Taller");
  eLayout->addWidget(taller, 5, 0, 1, 1);
  connect(taller, SIGNAL(clicked()), SLOT(doIt()));

  shorter = new QPushButton("Shorter");
  eLayout->addWidget(shorter, 5, 1, 1, 1);
  connect(shorter, SIGNAL(clicked()), SLOT(doIt()));

  layout->addWidget(eGroup);
*/
  layout->addWidget(tGroup);

  //from double click in a region
  connect(qdp_,
       SIGNAL(mouseRegionReady(Record, WorldCanvasHolder*)),
       SLOT(drawRegion(Record, WorldCanvasHolder*)));

  //also from double click a region
  connect(qdp_, SIGNAL(newRegion(String)),
                SLOT(newRegion_(String)));
  connect(qdp_,  SIGNAL(registrationChange()),
                 SLOT(deleteAll()));
  connect(qdp_, SIGNAL(activate(Record)),
                SLOT(activate(Record)) );

}
  
void MakeMask::changeAxis(String xa, String ya, String za) {
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

   cb = ccb;
   reDraw();
}

void MakeMask::activate(Record rcd) {
   cout << "activate " << rcd << endl; 

}

void MakeMask::wcChanged(const String cType,
               const Vector<Double> vx, const Vector<Double> vy) {
    cout << "wcChanged:" << cType << vx << vy << endl; 
    if (!isVisible()) 
       return;

}

void MakeMask::reShape(const QString& shape) {
   qDebug() << "reShape" << shape;
   

}

void MakeMask::loadRegionFromFile() {
   QString sName = QFileDialog::getOpenFileName(this, 
                                                tr("Open Region File"),
                                                ".",
                                                tr("Regions (*.rgn)"));
   //qDebug() << "loadRegion from" << sName;
   if (sName == "")
      return;

   String rName(sName.toStdString());
   ImageRegion* reg;
   TableRecord rec;
   try {
      //cout << "rName=" << rName << endl;
      AipsIO os(rName, ByteIO::Old);
      //os.open(rName);
      os >> rec;
      //os.close();
      //cout << "infile region record:\n" << rec << endl;
      reg = ImageRegion::fromRecord(rec, rName+".tbl");

      //WCUnion leUnion(unionRegions_p);
      //const WCRegion* newUnion = new WCUnion(reg->asWCRegion(), leUnion);
      //WCUnion* unfolded = unfoldCompositeRegionToSimpleUnion(newUnion);
      //cout << "unfolded:" << unfolded->toRecord("") << endl;
      //cout << "leUnion:" << leUnion.toRecord("") << endl;

      PtrBlock<const WCRegion* > outRegPtrs ;
      const WCRegion* rpt = &(reg->asWCRegion());
      unfoldIntoSimpleRegionPtrs(outRegPtrs, rpt);

      for (uInt m = 0; m < outRegPtrs.nelements(); m++) {
         uInt nreg=unionRegions_p.nelements();
         unionRegions_p.resize(nreg + 1, True);
         unionRegions_p[nreg] = new const ImageRegion(
             const_cast<WCRegion*>(outRegPtrs[m]));
      }
    }
    catch(...) {
      cout << "Failed to read region record" << endl;
    }
    reDraw();

}

void MakeMask::saveRegionToFile() {
   QString sName = QFileDialog::getSaveFileName(this, 
                                                tr("Save Region File"),
                                                "./",
                                                tr("Regions (*.rgn)"));
   //qDebug() << "saveRegion to" << sName;
   if (sName != ""){
      QString ext = sName.section('.', -1).toLower();
      qDebug() << ext;
      if (ext != "rgn") { 
         sName.append(".rgn");
      }
      qDebug() << sName;
    
      String regname(sName.toStdString());
      if (unionRegions_p.nelements() > 0){
         WCUnion leUnion(unionRegions_p);
         leUnion.setComment("chanExt:" +
                   chan->text().toStdString() +
                   "polExt:" +
                   corr->text().toStdString());
         ImageRegion* reg = new ImageRegion(leUnion);

         try{
            //AipsIO os(regname, ByteIO::NewNoReplace);
            AipsIO os(regname, ByteIO::New);
            os << reg->toRecord(regname+".tbl");
         }
         catch(...) {
            QMessageBox::warning(this, "FILEBOX",
               "Could not create the region file.\n"
               "Please check pathname and directory\n" 
               "permissions.");
         }
      }
   }
}

void MakeMask::showHelp() {

}

void MakeMask::addBox(RegionShape*) {
   qDebug() << "addBox";
}

void MakeMask::deleteBox(RegionShape*) {
   qDebug() << "deleteBox";
}

void MakeMask::deleteAll() {
   //qDebug() << "deleteAll";
   uInt nreg=unionRegions_p.nelements();
   for (uInt k=0; k< nreg; ++k){
      if (unionRegions_p[k] !=0){
         delete unionRegions_p[k];
      }
   }
   unionRegions_p.resize(0, True);
   reDraw();
}

void MakeMask::reDraw() {

   List<QtDisplayData*> DDs = qdp_->registeredDDs();
   ListIter<QtDisplayData*> qdds(DDs);
   if (qdds.len() == 0)
      return ;

   qdds.toEnd();
   qdds--;
   QtDisplayData* qdd = qdds.getRight();
   
   qdp_->hold();
   qdp_->panelDisplay()->removeDisplayData(*regData);

   //qDebug() << "showHide=" << showHide->text();
   if (unionRegions_p.nelements() > 0 &&
      showHide->text() == "Hide" && cb != 0) {
      WCUnion leUnion(unionRegions_p);
      ImageRegion* reg = new ImageRegion(leUnion);
      //cout << "to be save ImageRegion:\n"
      //     << reg->toRecord("arbitrary") << endl;
      regData = regionToShape(qdd, reg);

      qdp_->panelDisplay()->addDisplayData(*regData);
   }
   qdp_->release();

}

RSComposite* MakeMask::regionToShape(
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
        theShapes->setLineColor(color->currentText().toStdString());
        theShapes->setLineWidth(1);

        //regData[regName] = theShapes;
        return theShapes;
     }
     else {
        return 0;
     }
}

void MakeMask::addRegionsToShape(RSComposite*& theShapes, 
                                        const WCRegion*& wcreg){
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
      MDirection::Types dirType=coords->
             directionCoordinate(dirInd).directionType(True);
      //Assuming x, y axes are dirInd and dirInd+1
      Vector<Double> blc(2);
      Vector<Double> trc(2);
      QuantumHolder h;
      for (Int j=dirInd; j <= dirInd+1; ++j){
        const RecordInterface& subRec0=blcrec.asRecord(j);
        const RecordInterface& subRec1=trcrec.asRecord(j);
        String error;
        if (!h.fromRecord(error, subRec0)) {
           throw (AipsError 
          ("WCBox::fromRecord - could not recover blc because "+error));
        }

        blc(j-dirInd)=h.asQuantumDouble().getValue(RegionShape::UNIT);
        if (!h.fromRecord(error, subRec1)) {
           throw (AipsError 
            ("WCBox::fromRecord - could not recover trc because "+error));
        }
        trc(j-dirInd)=h.asQuantumDouble().getValue(RegionShape::UNIT);
      }
      RSRectangle *rect = (cb == -1) ?
         new RSRectangle(
             (blc(1)+trc(1))/2.0,(blc(0)+trc(0))/2.0,
              fabs(trc(1)-blc(1)), fabs(trc(0)-blc(0)), dirType) :
         new RSRectangle(
             (blc(0)+trc(0))/2.0,(blc(1)+trc(1))/2.0,
              fabs(trc(0)-blc(0)), fabs(trc(1)-blc(1)), dirType);
      rect->setLineColor(color->currentText().toStdString());
      theShapes->addShape(rect);

    }
    else if((wcreg->type())== "WCPolygon"){
      TableRecord polyrec=wcreg->toRecord("");
      CoordinateSystem *coords;
      coords=CoordinateSystem::restore(polyrec,"coordinates");
      //cout << "coords polyg " << coords->nCoordinates() << endl;

      Int dirInd=coords->findCoordinate(Coordinate::DIRECTION);
      MDirection::Types dirType=coords->
          directionCoordinate(dirInd).directionType(True);
      Vector<Double> x;
      Vector<Double> y;
      const RecordInterface& subRecord0 = polyrec.asRecord("x");
      String error;
      QuantumHolder h;
      if (!h.fromRecord(error, subRecord0)) {
        throw (AipsError 
       ("WCPolygon::fromRecord - could not recover X Quantum vector because "
        +error));
      }
      x = h.asQuantumVectorDouble().getValue(RegionShape::UNIT);
      const RecordInterface& subRecord1 = polyrec.asRecord("y");
      if (!h.fromRecord(error, subRecord1)) {
        throw (AipsError 
       ("WCPolygon::fromRecord - could not recover Y Quantum vector because "
        +error));
      }
      y = h.asQuantumVectorDouble().getValue(RegionShape::UNIT);

      RSPolygon *poly= (cb == -1) ?
           new RSPolygon(y,x,dirType) : new RSPolygon(x,y,dirType);
      poly->setLineColor(color->currentText().toStdString());
      theShapes->addShape(poly);
    }
    else if((wcreg->type()) == "WCUnion" ||(wcreg->type()) == "WCIntersection"){
      PtrBlock<const WCRegion*> regPtrs=
        (static_cast<const WCCompound* >(wcreg))->regions();
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

void MakeMask::showHideAll() {
   //qDebug() << "showHide";
   if (showHide->text() == "Hide") {
      showHide->setText("Show");
   }
   else { 
      showHide->setText("Hide");
   }
   reDraw();
}

void MakeMask::colorAll(const QString& clr) {
   //qDebug() << "colorAll" << clr;
   reDraw();
}

void MakeMask::rotateBox(int cb) {
}

void MakeMask::newRegion_(String imgFilename) {
   //cout << "newRegion_" << imgFilename << endl;
   
   if (this->isVisible()){
      if (qdp_->hasRegion()){
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

   reDraw();
}

void MakeMask::drawRegion(Record mousereg, 
                          WorldCanvasHolder *wch){
}

WCUnion* MakeMask::unfoldCompositeRegionToSimpleUnion(const WCRegion*& wcreg){
    PtrBlock<const WCRegion* > outRegPtrs ;
    unfoldIntoSimpleRegionPtrs(outRegPtrs, wcreg);
    WCUnion* outputUnion = new WCUnion(True, outRegPtrs);
    return outputUnion;
}

void MakeMask::unfoldIntoSimpleRegionPtrs(PtrBlock<const WCRegion*>& outRegPtrs,
                                          const WCRegion*& wcreg){
   if ((wcreg->type()) == "WCBox"){
      uInt nreg=outRegPtrs.nelements();
      outRegPtrs.resize(nreg+1);
      outRegPtrs[nreg]=new WCBox(static_cast<const WCBox & >(*wcreg));
   }
   else if((wcreg->type()) == "WCPolygon"){
      uInt nreg=outRegPtrs.nelements();
      outRegPtrs.resize(nreg+1);
      outRegPtrs[nreg]=new WCPolygon(static_cast<const WCPolygon & >(*wcreg));
   }
   else if((wcreg->type()) == "WCUnion" ||
           (wcreg->type()) == "WCIntersection" ){
      PtrBlock<const WCRegion*> regPtrs =
           (static_cast<const WCCompound* >(wcreg))->regions();
      //cout << "number of wcregions " << regPtrs.nelements() << endl;
      for (uInt j=0; j < regPtrs.nelements(); ++j){
         unfoldIntoSimpleRegionPtrs(outRegPtrs, regPtrs[j]);
      }
   }
}

void MakeMask::doIt() {
  QPushButton* action = dynamic_cast<QPushButton*>(sender()); 
  if (action == 0) {
    return; 
  }
  qDebug() << action->text() << "clicked";

}

  
}
