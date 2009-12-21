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

#include <casa/aips.h>
#include <casa/Arrays.h>
#include <measures/Measures.h>
#include <coordinates/Coordinates.h>
#include <casa/Exceptions/Error.h>
#include <images/Regions/WCBox.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/LCRegion.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/RegionType.h>
#include <tables/Tables/TableRecord.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>


#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/QtProfile.qo.h>

#include <display/QtPlotter/QtMWCTools.qo.h>

#include <display/DisplayEvents/MWCPTRegion.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <cmath>
#include <QtGui>
#include <iostream>
#include <graphics/X11/X_exit.h>
#include <QMessageBox>

namespace casa { 

QtProfile::~QtProfile()
{
    delete pc; delete te;
    delete zoomInButton;
    delete zoomOutButton;
    delete printButton;
    delete saveButton;
    //delete printExpButton;
    //delete saveExpButton;
    delete writeButton;
}

QtProfile::QtProfile(ImageInterface<Float>* img, 
        const char *name, QWidget *parent)
        :QWidget(parent), //MWCCrosshairTool(),
         pc(0), te(0), analysis(0), 
         coordinate("world"), coordinateType(""),
         fileName(name), position(""), yUnit(""), yUnitPrefix(""), 
	 xpos(""), ypos(""),
	 cube(0), lastX(Vector<Double>()), lastY(Vector<Double>()),
         z_xval(Vector<Float>()), z_yval(Vector<Float>()),
         region("") 
{

    initPlotterResource();
    setWindowTitle(QString("Image Profile - ").append(name));
    // setWindowIcon(QIcon(":/images/nrao.png"));
    // (Wait until a casa.png is available...
    setBackgroundRole(QPalette::Dark);
 
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    zoomInButton = new QToolButton(this);
    zoomInButton->setIcon(QIcon(":/images/zoomin.png"));
    zoomInButton->setToolTip("Zoom in.  (ctrl +)");
    zoomInButton->adjustSize();
    connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));
    
    zoomOutButton = new QToolButton(this);
    zoomOutButton->setIcon(QIcon(":/images/zoomout.png"));
    zoomOutButton->setToolTip("Zoom out.  (ctrl -)");
    zoomOutButton->adjustSize();
    connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));    
    
    printButton = new QToolButton(this);
    printButton->setIcon(QIcon(":/images/print.png"));
    printButton->setToolTip("Print...");
    printButton->adjustSize();
    connect(printButton, SIGNAL(clicked()), this, SLOT(print()));

    saveButton = new QToolButton(this);
    saveButton->setIcon(QIcon(":/images/save.png"));
    saveButton->setToolTip("Save as a (standard graphics) image.");
    saveButton->adjustSize();
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));

    //printExpButton = new QToolButton(this);
    //printExpButton->setIcon(QIcon(":/images/printExp.png"));
    //printExpButton->setToolTip("Print to the default printer");
    //printExpButton->adjustSize();
    //connect(printExpButton, SIGNAL(clicked()), this, SLOT(printExp()));

    //saveExpButton = new QToolButton(this);
    //saveExpButton->setIcon(QIcon(":/images/saveExp.png"));
    //saveExpButton->setToolTip("Save to an image file.");
    //saveExpButton->adjustSize();
    //connect(saveExpButton, SIGNAL(clicked()), this, SLOT(saveExp()));

    writeButton = new QToolButton(this);
    writeButton->setIcon(QIcon(":/images/write.png"));
    writeButton->setToolTip("Save as an ascii plot file.");
    writeButton->adjustSize();
    connect(writeButton, SIGNAL(clicked()), this, SLOT(writeText()));
    
    leftButton = new QToolButton(this);
    leftButton->setIcon(QIcon(":/images/leftarrow.png"));
    leftButton->setToolTip("Scroll left  (left arrow)");
    leftButton->adjustSize();
    connect(leftButton, SIGNAL(clicked()), this, SLOT(left()));
    
    rightButton = new QToolButton(this);
    rightButton->setIcon(QIcon(":/images/rightarrow.png"));
    rightButton->setToolTip("Scroll right  (right arrow)");
    rightButton->adjustSize();
    connect(rightButton, SIGNAL(clicked()), this, SLOT(right()));    
    
    upButton = new QToolButton(this);
    upButton->setIcon(QIcon(":/images/uparrow.png"));
    upButton->setToolTip("Scroll up  (up arrow)");
    upButton->adjustSize();
    connect(upButton, SIGNAL(clicked()), this, SLOT(up()));

    downButton = new QToolButton(this);
    downButton->setIcon(QIcon(":/images/downarrow.png"));
    downButton->setToolTip("Scroll down  (down arrow)");
    downButton->adjustSize();
    connect(downButton, SIGNAL(clicked()), this, SLOT(down()));

    autoScale = new QCheckBox("auto scale", this);
    autoScale->setCheckState(Qt::Checked);
    connect(autoScale, SIGNAL(stateChanged(int)), 
            this, SLOT(setAutoScale(int)));

    buttonLayout->addWidget(zoomInButton);
    buttonLayout->addWidget(zoomOutButton);    
    buttonLayout->addWidget(leftButton);
    buttonLayout->addWidget(rightButton);    
    buttonLayout->addWidget(upButton);
    buttonLayout->addWidget(downButton);    
    buttonLayout->addItem(new QSpacerItem(40, zoomInButton->height(),
         QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    buttonLayout->addWidget(autoScale);
    buttonLayout->addWidget(writeButton);
    //buttonLayout->addWidget(printExpButton);
    //buttonLayout->addWidget(saveExpButton);
    buttonLayout->addWidget(printButton);
    buttonLayout->addWidget(saveButton);
    
    if (!layout())
       new QVBoxLayout(this);
       
    pc = new QtCanvas(this);
    QPalette pal = pc->palette();
    pal.setColor(QPalette::Background, Qt::white);
    pc->setPalette(pal);
    layout()->addWidget(pc);

    QHBoxLayout *displayLayout = new QHBoxLayout;
    chk = new QComboBox(this);
    chk->addItem("world");
    chk->addItem("pixel");
    connect(chk, SIGNAL(currentIndexChanged(const QString &)), 
            this, SLOT(changeCoordinate(const QString &)));
    
    te = new QLineEdit(this);
    te->setReadOnly(true);  
    QLabel *label = new QLabel(this);
    label->setText("<font color=\"blue\">Coordinate:</font>");
    label->setAlignment((Qt::Alignment)(Qt::AlignBottom | Qt::AlignRight));
 
    ctype = new QComboBox(this);
    frameButton_p= new QComboBox(this);

    // get reference frame info for freq axis label
    MFrequency::Types freqtype = determineRefFrame(img);

    frameType_p = String(MFrequency::showType(freqtype));
    //    ctype->addItem("true velocity ("+nativeRefFrameName+")");
    ctype->addItem("radio velocity"); 
    ctype->addItem("optical velocity");
    ctype->addItem("frequency ");
    ctype->addItem("channel");
    frameButton_p->addItem("LSRK");
    frameButton_p->addItem("BARY");
    frameButton_p->addItem("GEO");
    frameButton_p->addItem("TOPO");
    Int frameindex=frameButton_p->findText(QString(frameType_p.c_str()));
    frameButton_p->setCurrentIndex(frameindex);

    coordinateType = String(ctype->itemText(0).toStdString());

    connect(ctype, SIGNAL(currentIndexChanged(const QString &)), 
            this, SLOT(changeCoordinateType(const QString &)));
    connect(frameButton_p, SIGNAL(currentIndexChanged(const QString &)), 
            this, SLOT(changeFrame(const QString &)));
    displayLayout->addWidget(label);
    displayLayout->addWidget(chk);
    displayLayout->addWidget(te);
    displayLayout->addWidget(ctype);
    displayLayout->addWidget(frameButton_p);

    layout()->addItem(displayLayout);
    layout()->addItem(buttonLayout);
    
    zoomInButton->hide();
    zoomOutButton->hide();
    upButton->hide();
    downButton->hide();
    leftButton->hide();
    rightButton->hide();

    QSettings settings("CASA", "Viewer");
    QString pName = settings.value("Print/printer").toString();
    //printExpButton->setVisible(!pName.isNull());
    //saveExpButton->hide();
    
    connect(pc, SIGNAL(zoomChanged()), this, SLOT(updateZoomer()));
    //setCross(1); 

    analysis = new ImageAnalysis(img);

    pc->setTitle("");
    pc->setWelcome("assign a mouse button to\n"
                   "'crosshair' or 'rectangle' or 'polygon'\n"
                   "click/press+drag the assigned button on\n"
                   "the image to get a image profile");
    
    QString lbl = coordinateType.chars();
    if (lbl.contains("freq")) lbl.append(" (GHz)");
    if (lbl.contains("velo")) lbl.append(" (km/s)");
    pc->setXLabel(lbl, 12, 0.5, "Helvetica [Cronyx]");

    yUnit = QString(img->units().getName().chars());
    pc->setYLabel("("+yUnitPrefix+yUnit+")", 12, 0.5, "Helvetica [Cronyx]");

    pc->setAutoScale(autoScale->checkState());
       
}

MFrequency::Types QtProfile::determineRefFrame(ImageInterface<Float>* img, bool check_native_frame )
{ 
  MFrequency::Types freqtype;
  
  CoordinateSystem cSys=img->coordinates();
  Int specAx=cSys.findCoordinate(Coordinate::SPECTRAL);

  if ( specAx < 0 ) {
    QMessageBox::information( this, "No spectral axis...",
			      "Sorry, could not find a spectral axis for this image...",
			      QMessageBox::Ok);
    return MFrequency::DEFAULT;
  }

  SpectralCoordinate specCoor=cSys.spectralCoordinate(specAx);
  MFrequency::Types tfreqtype;
  MEpoch tepoch; 
  MPosition tposition;
  MDirection tdirection;
  specCoor.getReferenceConversion(tfreqtype, tepoch, tposition, tdirection);
  freqtype = specCoor.frequencySystem(False); // false means: get the native type
  
  if( check_native_frame && tfreqtype != freqtype ){ // there is an active conversion layer
    // ask user if he/she wants to change to native frame
    String title = "Change display reference frame?";
    String message = "Native reference frame is " + MFrequency::showType(freqtype)
      + ",\n display frame is " + MFrequency::showType(tfreqtype) + ".\n"
      + "Change display frame permanently to " + MFrequency::showType(freqtype) + "?\n"
      + "(Needs write access to image.)";
    if(QMessageBox::question(this, title.c_str(), message.c_str(),
			     QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
      // user wants to change
      try {
	// set the reference conversion to the native type, effectively switching it off
	if(!specCoor.setReferenceConversion(freqtype, tepoch, tposition, tdirection)
	   || !cSys.replaceCoordinate(specCoor, specAx) 
	   || !img->setCoordinateInfo(cSys)){
	  
	  img->coordinates().spectralCoordinate(specAx).getReferenceConversion(tfreqtype, tepoch, tposition, tdirection);
	  title = "Failure";
	  message = "casaviewer: Error setting reference frame conversion to native frame (" 
	    + MFrequency::showType(freqtype) + ")\nWill use " + MFrequency::showType(tfreqtype) + " instead";
	  QMessageBox::warning(this, title.c_str(), message.c_str(),
			       QMessageBox::Ok, QMessageBox::NoButton);
	  freqtype = tfreqtype;
	}
      } catch (AipsError x) {
	title = "Failure";
	message = "Error when trying to change display reference frame:\n" + x.getMesg();
	QMessageBox::warning(this, title.c_str(), message.c_str(),
			     QMessageBox::Ok, QMessageBox::NoButton);
	freqtype = tfreqtype;
      } 
    }
    else{ // user does not want to change
      freqtype = tfreqtype;
    }
  } // end if there is a conv layer

  return freqtype;

}

void QtProfile::zoomOut()
{
    pc->zoomOut();
}

void QtProfile::zoomIn()
{
   pc->zoomIn();
}

void QtProfile::setAutoScale(int st)
{
   //qDebug() << "auto scale state change=" << st;  
   pc->setAutoScale(st);
}

void QtProfile::left()
{
   QApplication::sendEvent(pc,
       new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, 0, 0));
}

void QtProfile::right()
{
   QApplication::sendEvent(pc,
       new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, 0, 0));
}

void QtProfile::up()
{
   QApplication::sendEvent(pc,
       new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, 0, 0));
}

void QtProfile::down()
{
   QApplication::sendEvent(pc,
       new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, 0, 0));
}

void QtProfile::print()
{
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::ScreenResolution);
    //printer.setFullPage(true);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (dlg->exec() == QDialog::Accepted) {
       QSettings settings("CASA", "Viewer");
       settings.setValue("Print/printer", printer.printerName());
       printIt(&printer);
    }
    delete dlg;
#endif
}

void QtProfile::printIt(QPrinter* printer) 
{
    QPainter painter(printer);
    QRect rect = painter.viewport();
    rect.adjust(72, 72, -72, -72);
    QPixmap *mp = pc->graph();
    QSize size = mp->size();
    size.scale(rect.size(), Qt::KeepAspectRatio);
    painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
    painter.setWindow(mp->rect());
    painter.drawPixmap(0, 0, *mp);
    painter.end();
}

void QtProfile::printExp()
{
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::ScreenResolution);
    QSettings settings("CASA", "Viewer");
    printer.setPrinterName(settings.value("Print/printer").toString());
    printIt(&printer);
#endif
}

void QtProfile::save()
{
    QString dflt = fileName + position + ".png";

    QString fn = QFileDialog::getSaveFileName(this,
       tr("Save as..."),
       QString(dflt), tr(
            "(*.png);;(*.xpm);;(*.jpg);;"
            "(*.png);;(*.ppm);;(*.jpeg)"));

    if (fn.isEmpty())
        return ;

    QString ext = fn.section('.', -1);
    if (ext == "xpm" || ext == "jpg" || ext == "png" ||
        ext == "xbm" || ext == "ppm" || ext == "jpeg")
        ;
    else 
        fn.append(".png"); 

    QFile file(fn);
    if (!file.open(QFile::WriteOnly))
        return ;

    pc->graph()->save(fn);
    return ;
}

void QtProfile::saveExp()
{
    //qDebug() << "fileName:" << fileName;
    QFile file(fileName.append(position).append(".png"));
    if (!file.open(QFile::WriteOnly))
        return ;
    //qDebug() << "open ok";

    pc->graph()->save(fileName, "PNG");
    //qDebug() << "save ok";
    return ;
}

void QtProfile::writeText()
{
    //qDebug() << "fileName:" << fileName;
    QString fn = QFileDialog::getSaveFileName(this,
       tr("Write profile data as text"),
       QString(), tr( "(*.txt);;(*.plt)"));

    if (fn.isEmpty())
        return ;
    fn = fn.section('/', -1);


    QString ext = fn.section('.', -1);
    if (ext != "txt" && ext != "plt")
        fn.append(".txt"); 

    QFile file(fn);
    if (!file.open(QFile::WriteOnly | QIODevice::Text))
        return ;
    QTextStream ts(&file);
    
    ts << "#title: Image profile - " << fileName << " " 
       << region << "(" << position << ")\n";
    ts << "#coordintate: " << QString(coordinate.chars()) << "\n";
    ts << "#xLabel: " << QString(coordinateType.chars()) << "\n";
    ts << "#yLabel: " << "(" << yUnit << ")\n";
    
    ts.setRealNumberNotation(QTextStream::ScientificNotation);

    for (uInt i = 0; i < z_xval.size(); i++) {
      ts << z_xval(i) << "    " << z_yval(i) << "\n";
    }
   
    return ;
}

void QtProfile::updateZoomer()
{
    if (pc->getCurZoom() > 0)
    {
        zoomOutButton->setEnabled(true);
        zoomOutButton->show();
    }
    if (pc->getCurZoom() == 0)
    {
        zoomOutButton->setEnabled(false);
    }
    if (pc->getCurZoom() < pc->getZoomStackSize() - 1)
    {
        zoomInButton->setEnabled(true);
        zoomInButton->show();  
    } 
    if (pc->getCurZoom() == pc->getZoomStackSize() - 1)
    {
        zoomInButton->setEnabled(false);
    }   
    upButton->setVisible(1);
    downButton->setVisible(1);
    leftButton->setVisible(1);
    rightButton->setVisible(1);
}
 
void QtProfile::changeCoordinate(const QString &text) {

  coordinate = String(text.toStdString());

  //Double x1, y1;
  //getProfileRange(x1, y1, coordinate.chars());
  //qDebug() << "coordinate:" << QString(coordinate.chars()) 
  //           << "profile Range:" << x1 << y1;

  emit coordinateChange(coordinate);
}

void QtProfile::changeFrame(const QString &text) {
  frameType_p=String(text.toStdString());
  changeCoordinateType(QString(coordinateType.c_str()));
}
void QtProfile::changeCoordinateType(const QString &text) {
    coordinateType = String(text.toStdString());
    //qDebug() << "coordinate:" << text; 
    //Double x1, y1;
    //getProfileRange(x1, y1, coordinate.chars());
    //qDebug() << "coordinate:" << QString(coordinate.chars()) 
    //           << "profile Range:" << x1 << y1;
    xpos = "";
    ypos = "";
    position = QString("");
    te->setText(position);
    pc->clearCurve(0);

    QString lbl = text;
    if (text.contains("freq")) lbl.append(" (GHz) " + QString(frameType_p.c_str()));
    if (text.contains("velo")) lbl.append(" (km/s) " + QString(frameType_p.c_str()));
    pc->setXLabel(lbl, 12, 0.5, "Helvetica [Cronyx]");

    pc->setPlotSettings(QtPlotSettings());
    zoomInButton->setVisible(0);
    zoomOutButton->setVisible(0);
    upButton->setVisible(0);
    downButton->setVisible(0);
    leftButton->setVisible(0);
    rightButton->setVisible(0);

    if(lastX.nelements() > 0){ // update display with new coord type
      wcChanged(coordinate, lastX, lastY);
    }

}

void QtProfile::closeEvent (QCloseEvent* event) {
   //qDebug() << "closeEvent";
  lastX.resize(0);
  lastY.resize(0);
  z_xval.resize(0);
  z_yval.resize(0);
  emit hideProfile();
}

QString QtProfile::getRaDec(double x, double y) {
   QString raDec = "";
   int sign = (y > 0) ? 1 : -1;
   const double a = 572.95779513082; 
   double rah, ram, decd, decm;
   double ras = x * 24 * a;
   double decs = sign * y * 360 * a;

   if (ras > 86400) ras = 0;
   if (decs > 1296000) decs = 0;
 
   int h, m, d, c;
   rah = ras / 3600;
   h = (int)floor(rah);
   ram = (rah - h) * 60;
   m = (int)floor(ram);
   ras = (ram - m) * 60; 
   ras = (int)(1000 * ras) / 1000.; 

   decd = decs / 3600;
   d = (int)floor(decd);
   decm = (decd - d) * 60;
   c = (int)floor(decm);
   decs = (decm - c) * 60;
   decs = (int)(1000 * decs) / 1000.;
  
   raDec.append((h < 10) ? "0" : "").append(QString().setNum(h)).append(":")
        .append((m < 10) ? "0" : "").append(QString().setNum(m)).append(":")
        .append((ras < 10) ? "0" : "").append(QString().setNum(ras))
        .append((sign > 0) ? "+" : "-")
        .append((d < 10) ? "0" : "").append(QString().setNum(d)).append("d")
        .append((c < 10) ? "0" : "").append(QString().setNum(c)).append("m")
        .append((decs < 10) ? "0" : "").append(QString().setNum(decs));
 
   return raDec;
}

void QtProfile::resetProfile(ImageInterface<Float>* img, const char *name)
{
    analysis = new ImageAnalysis(img);
    setWindowTitle(QString("Image Profile - ").append(name));

    // reset the combo box for selecting the coordinate type
    ctype->clear();
    // get reference frame info for freq axis label
    MFrequency::Types freqtype = determineRefFrame(img);

    QString nativeRefFrameName = QString(MFrequency::showType(freqtype).c_str());
    //    ctype->addItem("true velocity ("+nativeRefFrameName+")");
    ctype->addItem("radio velocity ("+nativeRefFrameName+")"); 
    ctype->addItem("optical velocity ("+nativeRefFrameName+")");
    ctype->addItem("frequency ("+nativeRefFrameName+")");
    ctype->addItem("channel");    

    coordinateType = String(ctype->itemText(0).toStdString());

    QString lbl = coordinateType.chars();
    if (lbl.contains("freq")) lbl.append(" (GHz)");
    if (lbl.contains("velo")) lbl.append(" (km/s)");
    pc->setXLabel(lbl, 12, 0.5, "Helvetica [Cronyx]");

    yUnit = QString(img->units().getName().chars());
    yUnitPrefix = "";
    pc->setYLabel("("+yUnitPrefix+yUnit+")", 12, 0.5, "Helvetica [Cronyx]");

    xpos = "";
    ypos = "";
    lastX.resize(0);
    lastY.resize(0);
    position = QString("");
    te->setText(position);
    pc->clearCurve(0);
}

void QtProfile::wcChanged(const String c,
    const Vector<Double> x, const Vector<Double> y) 
{
    if (!isVisible()) return;
    if (!analysis) return;
    if (cube == 0) return;

    //cout << "wcChanged: coordType=" << c 
    //     << " coordinate=" << coordinate << endl;

    if (c != coordinate) {
       coordinate = c.chars();
       if (coordinate == "world")
          chk->setCurrentIndex(0);
       else
          chk->setCurrentIndex(1);
    }
       
    Int ns;
    x.shape(ns);
    //cout << "ns =" << ns << endl;  
    //cout << "cube =" << cube << endl;  
  
    Vector<Double> xv(ns);
    Vector<Double> yv(ns);
    if (cube == -1){
	for (Int i = 0; i < ns; i++) {
	    xv(i) = y(i);
	    yv(i) = x(i); 
        }
    }
    else{
        for (Int i = 0; i < ns; i++) {
            xv(i) = x(i);
            yv(i) = y(i); 
        }
    }

    if (ns < 1) return;
    if (ns == 1) {
        pc->setTitle("Single Point Profile");
        region = "Point";
    }
    else if (ns == 2) {
        pc->setTitle("Rectangle Region Profile");
        region = "Rect";
    }
    else {
        pc->setTitle("Polygon Region Profile");
        region = "Poly";
    }
    pc->setWelcome("");

    //xpos, ypos and position only used for display
    xpos = QString::number(floor(xv[0]+0.5));
    ypos = QString::number(floor(yv[0]+0.5));
    //qDebug() << c.chars() << "xpos:" 
    //           << xpos << "ypos:" << ypos;

    if (coordinate == "world") {
       position = getRaDec(xv[0], yv[0]);
    }
    else {
       position = QString("[%1, %2]").arg(xpos).arg(ypos);
    }
    //qDebug() << "position:" << position;
    te->setText(position);

    //Get Profile Flux density v/s velocity
    Bool ok = False;
    ok=analysis->getFreqProfile(xv, yv, z_xval, z_yval, 
				coordinate, coordinateType, 0, 0, 0, "", frameType_p);

    // scale for better display
    // max absolute display numbers should be between 0.1 and 100.0
    Double dmin = 0.1;
    Double dmax = 100.;
    Double ymin = min(z_yval);  	
    Double ymax = max(z_yval);
    ymax = max(abs(ymin), ymax);
    Double symax;
    Int ordersOfM = 0;

    symax = ymax;
    while(symax < dmin){
	ordersOfM += 3;
	symax = ymax * pow(10.,ordersOfM);
    }
    while(symax > dmax){
	ordersOfM -= 3;
	symax = ymax * pow(10.,ordersOfM);
    }

    if(ordersOfM!=0){
	// correct display y axis values
        for (uInt i = 0; i < z_yval.size(); i++) {
            z_yval(i) *= pow(10.,ordersOfM);
        }
	// correct unit string 
	if(yUnit.startsWith("(")||yUnit.startsWith("[")||yUnit.startsWith("\"")){
	    // express factor as number
	    ostringstream oss;
	    oss << -ordersOfM;
	    yUnitPrefix = "10E"+QString(oss.str().c_str())+" ";
	}
	else{
	    // express factor as character
	    switch(-ordersOfM){ // note the sign!
	    case -9:
		yUnitPrefix = "p";
		break;
	    case -6:
		yUnitPrefix = "u";
		break;
	    case -3:
		yUnitPrefix = "m";
		break;
	    case 3:
		yUnitPrefix = "k";
		break;
	    case 6:
		yUnitPrefix = "M";
		break;
	    case 9:
		yUnitPrefix = "M";
		break;
	    default:
		ostringstream oss;
		oss << -ordersOfM;
		yUnitPrefix = "10E"+QString(oss.str().c_str())+" ";
		break;
	    }
	}
    }
    else{ // no correction
	yUnitPrefix = "";
    }
	
    pc->setYLabel("("+yUnitPrefix+yUnit+")", 12, 0.5, "Helvetica [Cronyx]");


    // plot the graph 
    pc->plotPolyLine(z_xval, z_yval);

    lastX.assign(xv);
    lastY.assign(yv);

}

void QtProfile::changeAxis(String xa, String ya, String za) {
   //cout << "change axis=" << xa << " " << ya 
   //     << " " << za << endl;
   int cb = 0;
   if (xa.contains("Decl") && ya.contains("Right"))
       cb = -1;
   if (xa.contains("Right") && ya.contains("Decl"))
       cb = 1;
   if (xa.contains("atitu") && ya.contains("ongitu"))
       cb = -1;
   if (xa.contains("ongitu") && ya.contains("atitu"))
       cb = 1;
   if (!za.contains("Freq"))
       cb = 0;
   if (cb != cube) {
       cube = cb;
       xpos = "";
       ypos = "";
       position = QString("");
       te->setText(position);
       if (cube == 0)
            pc->setWelcome("No profile available\n"
                   "for the given data cube");
       else 
            pc->setWelcome("assign a mouse button to\n"
                   "'crosshair' or 'rectangle' or 'polygon'\n"
                   "click/press+drag the assigned button on\n"
                   "the image to get a image profile");

       pc->clearCurve(0);
   }
     
   //cout << "cube=" << cube << endl;

}

}

