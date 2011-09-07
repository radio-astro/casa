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
//#include <images/Images/ImageAnalysis.h>
//#include <images/Images/PagedImage.h>
#include <lattices/Lattices/LCRegion.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/RegionType.h>
#include <tables/Tables/TableRecord.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>


#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/QtProfile.qo.h>
#include <display/QtPlotter/QtMWCTools.qo.h>
#include <imageanalysis/ImageAnalysis/ImageCollapser.h>
#include <images/Images/ImageAnalysis.h>
#include <images/Images/ImageUtilities.h>
#include <images/Images/PagedImage.h>
#include <casa/Logging/LogFilter.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
//#include <images/Images/TempImage.h>
#include <images/Images/ImageUtilities.h>
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
    delete itsLog;
}

QtProfile::QtProfile(ImageInterface<Float>* img, 
        const char *name, QWidget *parent, std::string rcstr)
        :QWidget(parent), //MWCCrosshairTool(),
         pc(0), te(0), analysis(0), image(img), over(0),
         coordinate("world"), coordinateType(""),xaxisUnit(""),ctypeUnit(""),
         fileName(name), position(""), yUnit(""), yUnitPrefix(""), 
         xpos(""), ypos(""), cube(0), npoints(0), npoints_old(0),
         lastPX(Vector<Double>()), lastPY(Vector<Double>()),
         lastWX(Vector<Double>()), lastWY(Vector<Double>()),
         z_xval(Vector<Float>()), z_yval(Vector<Float>()),
         z_eval(Vector<Float>()), region(""), rc(viewer::getrc()), rcid_(rcstr),
         itsPlotType(QtProfile::PMEAN), itsLog(new LogIO())
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

    multiProf = new QCheckBox("Overlay", this);
    multiProf->setCheckState(Qt::Checked);
    connect(multiProf, SIGNAL(stateChanged(int)), 
            this, SLOT(setMultiProfile(int)));

    relative = new QCheckBox("Relative", this);
    relative->setCheckState(Qt::Checked);
    connect(relative, SIGNAL(stateChanged(int)), 
            this, SLOT(setRelativeProfile(int)));

    autoScaleX = new QCheckBox("X auto scale", this);
    autoScaleX->setCheckState(Qt::Checked);
    connect(autoScaleX, SIGNAL(stateChanged(int)), 
            this, SLOT(setAutoScaleX(int)));

    autoScaleY = new QCheckBox("Y auto scale", this);
    autoScaleY->setCheckState(Qt::Checked);
    connect(autoScaleY, SIGNAL(stateChanged(int)), 
            this, SLOT(setAutoScaleY(int)));

    // create the combo box
    plotMode  = new QComboBox(this);
    plotMode->setToolTip("Choose the plot type.");
    errorMode = new QComboBox(this);
    errorMode->setToolTip("Choose the error type.");
    fillPlotTypes();

    connect(plotMode, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(changePlotType(const QString &)));

    connect(errorMode, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(changeErrorType(const QString &)));


    buttonLayout->addWidget(zoomInButton);
    buttonLayout->addWidget(zoomOutButton);    
    buttonLayout->addWidget(leftButton);
    buttonLayout->addWidget(rightButton);    
    buttonLayout->addWidget(upButton);
    buttonLayout->addWidget(downButton);    
    buttonLayout->addItem(new QSpacerItem(40, zoomInButton->height(),
         QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    buttonLayout->addWidget(multiProf);
    buttonLayout->addWidget(relative);
    buttonLayout->addWidget(autoScaleX);
    buttonLayout->addWidget(autoScaleY);
    //if(img->coordinates().nPixelAxes ()==4)
    //	buttonLayout->addWidget(plotError);
    //buttonLayout->addWidget(collapseImg);
    buttonLayout->addWidget(plotMode);
    buttonLayout->addWidget(errorMode);
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

    ctype->addItem("radio velocity [m/s]");
    ctype->addItem("radio velocity [km/s]");
    ctype->addItem("optical velocity [m/s]");
    ctype->addItem("optical velocity [km/s]");
    ctype->addItem("frequency [Hz]");
    ctype->addItem("frequency [MHz]");
    ctype->addItem("frequency [GHz]");
    ctype->addItem("wavelength [mm]");
    ctype->addItem("wavelength [um]");
    ctype->addItem("wavelength [nm]");
    ctype->addItem("wavelength [Angstrom]");
    ctype->addItem("air wavelength [mm]");
    ctype->addItem("air wavelength [um]");
    ctype->addItem("air wavelength [nm]");
    ctype->addItem("air wavelength [Angstrom]");
    ctype->addItem("channel");

    // read the preferred ctype from casarc
    QString pref_ctype = QString(rc.get("viewer." + rcid() + ".freqcoord.type").c_str());
    if (pref_ctype.size()>0){
      // change to the preferred ctype
    	int ctypeindex= ctype->findText(pref_ctype);
    	if (ctypeindex > -1)
    		ctype->setCurrentIndex(ctypeindex);
    }

    ctypeUnit = String(ctype->currentText().toStdString());
    getcoordTypeUnit(ctypeUnit, coordinateType, xaxisUnit);

    // make a combo box for the frequency system
    frameButton_p= new QComboBox(this);

    // get reference frame info for freq axis label
    MFrequency::Types freqtype = determineRefFrame(img);
    frameType_p = String(MFrequency::showType(freqtype));
    frameButton_p->addItem("LSRK");
    frameButton_p->addItem("BARY");
    frameButton_p->addItem("GEO");
    frameButton_p->addItem("TOPO");
    Int frameindex=frameButton_p->findText(QString(frameType_p.c_str()));
    frameButton_p->setCurrentIndex(frameindex);

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

    //image = img;
    analysis = new ImageAnalysis(img);

    pc->setTitle("");
    pc->setWelcome("assign a mouse button to\n"
                   "'crosshair' or 'rectangle' or 'polygon'\n"
                   "click/press+drag the assigned button on\n"
                   "the image to get a image profile");
    
    QString lbl = ctype->currentText();
    pc->setXLabel(lbl, 12, 0.5, "Helvetica [Cronyx]");

    yUnit = QString(img->units().getName().chars());
    pc->setYLabel("("+yUnitPrefix+yUnit+")", 12, 0.5, "Helvetica [Cronyx]");

    pc->setAutoScaleX(autoScaleX->checkState());
    pc->setAutoScaleY(autoScaleY->checkState());
       
    setMultiProfile(true);
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
   //emit add2DImage("/diska/home/mkuemmel/3Ddata/collapse_small_meas3D", "image", "raster", True);
}

void QtProfile::setMultiProfile(int st)
{
   //qDebug() << "multiple profile state change=" << st;  
   relative->setChecked(false);
   if (st) {
      relative->setEnabled(true);
   }
   else {
      relative->setEnabled(false);
   }
   if(lastPX.nelements() > 0){
      wcChanged(coordinate, lastPX, lastPY, lastWX, lastWY, UNKNPROF);
   }

}

void QtProfile::setRelativeProfile(int )
{
   //qDebug() << "relative profile state change=" << st;  
   if(lastPX.nelements() > 0){
      wcChanged(coordinate, lastPX, lastPY, lastWX, lastWY, UNKNPROF);
   }
}

void QtProfile::setPlotError(int st)
{
	//qDebug() << "plot errors=" << st;
	pc->setPlotError(st);
}

/*void QtProfile::setImgCollapse(int st){
	//qDebug() << "collapse image=" << st;
	if (st)
		doImgCollapse();
}*/

void QtProfile::setAutoScaleX(int st)
{
   //qDebug() << "auto scale state change=" << st;  
   pc->setAutoScaleX(st);
}

void QtProfile::setAutoScaleY(int st)
{
   //qDebug() << "auto scale state change=" << st;  
   pc->setAutoScaleY(st);
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
    QString fn = QFileDialog::getSaveFileName(this,
       tr("Write profile data as text"),
       QString(), tr( "(*.txt);;(*.plt)"));

    if (fn.isEmpty())
        return ;

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
    ts << "#xLabel: " << QString(ctypeUnit.chars()) << "\n";
    ts << "#yLabel: " << "(" << yUnitPrefix << yUnit << ") "<< plotMode->currentText() << "\n";
    if (z_eval.size() > 0)
        ts << "#eLabel: " << "(" << yUnitPrefix << yUnit << ") " << errorMode->currentText() << "\n";
    
    ts.setRealNumberNotation(QTextStream::ScientificNotation);

    if (z_eval.size() > 0){
    	for (uInt i = 0; i < z_xval.size(); i++) {
    		ts << z_xval(i) << "    " << z_yval(i) << "    "  << z_eval(i) << "\n";
    	}
    }
    else {
    	for (uInt i = 0; i < z_xval.size(); i++) {
    		ts << z_xval(i) << "    " << z_yval(i) << "\n";
    	}
    }

    int i = pc->getLineCount();
    for (int k = 1; k < i; k++) {
      ts << "\n";
      ts << "# " << pc->getCurveName(k) << "\n";
      CurveData data = *(pc->getCurveData(k));
      int j = data.size() / 2;
      for (int m = 0; m < j; m++) {
         ts << data[2 * m] << " " << data[2 * m + 1] << "\n";
      }
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
	//cout << "In change frame with input: " << text.toStdString() <<" coordinateType: " << coordinateType.c_str()<< endl;
	frameType_p=String(text.toStdString());
	//changeCoordinateType(QString(coordinateType.c_str()));
	changeCoordinateType(QString(ctypeUnit.c_str()));
}
void QtProfile::changeCoordinateType(const QString &text) {
    //coordinateType = String(text.toStdString());
    //qDebug() << "coordinate:" << text; 
    //Double x1, y1;
    //getProfileRange(x1, y1, coordinate.chars());
    //qDebug() << "coordinate:" << QString(coordinate.chars()) 
    //           << "profile Range:" << x1 << y1;
    xpos = "";
    ypos = "";
    position = QString("");
    te->setText(position);
    pc->clearCurve();


    ctypeUnit = String(text.toStdString());
    getcoordTypeUnit(ctypeUnit, coordinateType, xaxisUnit);

    QString lbl = text;
    pc->setXLabel(lbl, 12, 0.5, "Helvetica [Cronyx]");

    pc->setPlotSettings(QtPlotSettings());
    zoomInButton->setVisible(0);
    zoomOutButton->setVisible(0);
    upButton->setVisible(0);
    downButton->setVisible(0);
    leftButton->setVisible(0);
    rightButton->setVisible(0);

    //cout << "put to rc.viewer: " << text.toStdString() << endl;
    rc.put( "viewer." + rcid() + ".freqcoord.type", text.toStdString());

    if(lastPX.nelements() > 0){ // update display with new coord type
   	 wcChanged(coordinate, lastPX, lastPY, lastWX, lastWY, UNKNPROF);
    }

}

void QtProfile::changePlotType(const QString &text) {

	// store the plot type and set the class data
	rc.put( "viewer." + rcid() + ".plot.type", text.toStdString());
	stringToPlotType(text, itsPlotType);

	// get the coo-sys
	CoordinateSystem cSys = image->coordinates();

	switch (itsPlotType)
	{
	case QtProfile::PMEAN:
		if (npoints !=1 && errorMode->findText("rmse") < 0)
			errorMode->insertItem(1, "rmse");
		if (npoints ==1 && errorMode->findText("rmse") > -1)
			errorMode->removeItem(errorMode->findText("rmse"));
		if (cSys.qualityAxisNumber() > -1 && errorMode->findText("propagated") < 0)
			errorMode->insertItem(2, "propagated");
		break;
	case QtProfile::PMEDIAN:
		if (npoints !=1 && errorMode->findText("rmse") < 0)
			errorMode->insertItem(1, "rmse");
		if (npoints ==1 && errorMode->findText("rmse") > -1)
			errorMode->removeItem(errorMode->findText("rmse"));
		if (errorMode->findText("propagated") > -1)
			errorMode->removeItem(errorMode->findText("propagated"));
		break;
	case QtProfile::PSUM:
		if (errorMode->findText("rmse") > -1)
			errorMode->removeItem(errorMode->findText("rmse"));
		if (cSys.qualityAxisNumber() > -1 && errorMode->findText("propagated") < 0)
			errorMode->insertItem(1, "propagated");
		break;
	//case QtProfile::PVRMSE:
	//	if (errorMode->findText("rmse") > -1)
	//		errorMode->removeItem(errorMode->findText("rmse"));
	//	if (errorMode->findText("propagated") > -1)
	//		errorMode->removeItem(errorMode->findText("propagated"));
	//	break;
	}

	redraw();
}

void QtProfile::changeErrorType(const QString &text) {
	rc.put( "viewer." + rcid() + ".error.type", text.toStdString());

	stringToErrorType(text, itsErrorType);

	redraw();
}

void QtProfile::stringToPlotType(const QString &text, QtProfile::PlotType &pType){
	*itsLog << LogOrigin("QtProfile", "stringToPlotType");

	if (!text.compare(QString("mean")))
		pType = QtProfile::PMEAN;
	else if (!text.compare(QString("median")))
		pType = QtProfile::PMEDIAN;
	else if (!text.compare(QString("sum")))
		pType = QtProfile::PSUM;
	//else if (!text.compare(QString("rmse")))
	//	pType = QtProfile::PVRMSE;
	else
		*itsLog << LogIO::WARN << "The string: " << text.toStdString() << " does not correspond to a plot type!" << LogIO::POST;
}

void QtProfile::stringToErrorType(const QString &text, QtProfile::ErrorType &eType){
	*itsLog << LogOrigin("QtProfile", "stringToErrorType");

	if (!text.compare(QString("no error")))
		eType = QtProfile::PNOERROR;
	else if (!text.compare(QString("rmse")))
		eType = QtProfile::PERMSE;
	else if (!text.compare(QString("propagated")))
		eType = QtProfile::PPROPAG;
	else
		*itsLog << LogIO::WARN << "The string: " << text.toStdString() << " does not correspond to an error type!" << LogIO::POST;
}

void QtProfile::closeEvent (QCloseEvent *) {
   //qDebug() << "closeEvent";
  lastPX.resize(0);
  lastPY.resize(0);
  lastWX.resize(0);
  lastWY.resize(0);
  z_xval.resize(0);
  z_yval.resize(0);
  z_eval.resize(0);
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
	image = img;
	analysis = new ImageAnalysis(img);
	fileName = name;
	setWindowTitle(QString("Image Profile - ").append(name));

	// re-set the images
	// that are overplotted
	if (over) {
		delete over;
		over = 0;
		over = new QHash<QString, ImageAnalysis*>();
	}

	// adjust the error box
	fillPlotTypes();

	// read the preferred ctype from casarc
	QString pref_ctype = QString(rc.get("viewer." + rcid() + ".freqcoord.type").c_str());
	if (pref_ctype.size()>0){
		// change to the preferred ctype
		int ctypeindex= ctype->findText(pref_ctype);
		if (ctypeindex > -1)
			ctype->setCurrentIndex(ctypeindex);
	}

	ctypeUnit = String(ctype->currentText().toStdString());
	getcoordTypeUnit(ctypeUnit, coordinateType, xaxisUnit);

	QString lbl = ctype->currentText();
	pc->setXLabel(lbl, 12, 0.5, "Helvetica [Cronyx]");


	// get reference frame info for frequency axis label
	MFrequency::Types freqtype = determineRefFrame(img);
	frameType_p = String(MFrequency::showType(freqtype));
	Int frameindex=frameButton_p->findText(QString(frameType_p.c_str()));
	frameButton_p->setCurrentIndex(frameindex);

	yUnit = QString(img->units().getName().chars());
	yUnitPrefix = "";
	pc->setYLabel("("+yUnitPrefix+yUnit+")", 12, 0.5, "Helvetica [Cronyx]");

	xpos = "";
	ypos = "";
	lastPX.resize(0);
	lastPY.resize(0);
	lastWX.resize(0);
	lastWY.resize(0);
	position = QString("");
	te->setText(position);
	pc->clearCurve();
}

void QtProfile::wcChanged( const String c,
			   const Vector<Double> px, const Vector<Double> py,
			   const Vector<Double> wx, const Vector<Double> wy,
			   const ProfileType ptype)
{
	if (!isVisible()) return;
    if (!analysis) return;

    *itsLog << LogOrigin("QtProfile", "wcChanged");

   //cout << "profile wcChanged     cube=" << cube << endl;
    //qDebug() << "top=" << fileName;
    // QHashIterator<QString, ImageAnalysis*> j(*over);
    // while (j.hasNext()) {
    //   j.next();
    //   qDebug() << j.key() << ": " << j.value();
    // }

    if (cube == 0) {
       pc->setWelcome("No profile available "
                   "for the given data \nor\n"
                   "No profile available for the "
                   "display axes orientation");
       pc->clearCurve();
      return;
    }

    last_event_cs.resize(c.size());
    last_event_cs = c;
    last_event_px.resize(px.size());
    last_event_px = px;
    last_event_py.resize(py.size());
    last_event_py = py;
    last_event_wx.resize(wx.size());
    last_event_wx = wx;
    last_event_wy.resize(wy.size());
    last_event_wy = wy;

	 npoints = wx.size();
    if (npoints_old==0 && wx.size()>0){
   	 npoints_old = wx.size();
		 changePlotType(plotMode->currentText());
    }
    else{
   	 if (npoints==1 & npoints_old!=1){
   		 npoints_old = npoints;
   		 changePlotType(plotMode->currentText());
   	 }
   	 else if (npoints!=1 & npoints_old==1){
     		 npoints_old = npoints;
   		 changePlotType(plotMode->currentText());
   	 }
    }


    if (c != coordinate) {
   	 coordinate = c.chars();
   	 if (coordinate == "world")
   		 chk->setCurrentIndex(0);
   	 else
   		 chk->setCurrentIndex(1);
    }
       
    Int ns;
    px.shape(ns);

    Vector<Double> pxv(ns);
    Vector<Double> pyv(ns);
    Vector<Double> wxv(ns);
    Vector<Double> wyv(ns);

    if (cube == -1){
   	 pxv.assign(py);
   	 pyv.assign(px);
   	 wxv.assign(wy);
   	 wyv.assign(wx);
    }
    else{
   	 pxv.assign(px);
   	 pyv.assign(py);
   	 wxv.assign(wx);
   	 wyv.assign(wy);
    }

    if (ns < 1) return;

    switch (ptype)
    {
       case SINGPROF:
    	   pc->setTitle("Single Point Profile");
    	   region = "Point";
    	   break;
       case RECTPROF:
    	   pc->setTitle("Rectangle Region Profile");
           region = "Rect";
           break;
       case ELLPROF:
    	   pc->setTitle("Elliptical Region Profile");
           region = "Ellipse";
           break;
       case POLYPROF:
           pc->setTitle("Polygon Region Profile");
           region = "Poly";
           break;
       case UNKNPROF:
    	   break;
       default:
           pc->setTitle("");
           region = "";
    }
    pc->setWelcome("");

    if (coordinate == "world") {
       //xpos, ypos and position only used for display
       xpos = QString::number(floor(wxv[0]+0.5));
       ypos = QString::number(floor(wyv[0]+0.5));
       position = getRaDec(wxv[0], wyv[0]);
    }
    else {
       //xpos, ypos and position only used for display
       xpos = QString::number(floor(pxv[0]+0.5));
       ypos = QString::number(floor(pyv[0]+0.5));
       position = QString("[%1, %2]").arg(xpos).arg(ypos);
    }
    te->setText(position);

    //Get Profile Flux density v/s coordinateType
    Bool ok = False;
    switch (itsPlotType)
    {
    case QtProfile::PMEAN:
   	 ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
   			 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
   			 (Int)QtProfile::MEAN, 0);
   	 break;
    case QtProfile::PMEDIAN:
   	 ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
   			 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
   			 (Int)QtProfile::MEDIAN, 0);
   	 break;
    case QtProfile::PSUM:
		 ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
				 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
				 (Int)QtProfile::SUM, 0);
   	 break;
    //case QtProfile::PVRMSE:
	//	 ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
	//			 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
	//			 (Int)QtProfile::RMSE, 0);
   //	 break;
    default:
   	 ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_yval,
   			 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
   			 (Int)QtProfile::MEAN, 0);
   	 break;
    }
    if (!ok) {
   	 // change to notify user of error...
    	*itsLog << LogIO::WARN << "Can not generate the frequency profile!" << LogIO::POST;
    	return;
    }

    // get the coordinate system
    CoordinateSystem cSys = image->coordinates();
    switch (itsErrorType)
    {
    case QtProfile::PNOERROR:
   	 if (z_eval.size()> 0)
   		 z_eval.resize(0);
   	 break;
    case QtProfile::PERMSE:
   	 if (wxv.size()<2){
   		 *itsLog << LogIO::NORMAL << "Can not do the plot request, only one point!" << LogIO::POST;
      	 if (z_eval.size()> 0)
      		 z_eval.resize(0);
   	 }
   	 else {
   		 switch (itsPlotType)
   		 {
   		 case QtProfile::PSUM:
   	   		 *itsLog << LogIO::NORMAL << "Plotting RMSE as error of SUM makes no sense!" << LogIO::POST;
   	      	 if (z_eval.size()> 0)
   	      		 z_eval.resize(0);
   	   		 break;
   		 //case QtProfile::PVRMSE:
   	   //		 cout << "Plotting RMSE over RMSE makes no sense!" << endl;
   	    //  	 if (z_eval.size()> 0)
   	    //  		 z_eval.resize(0);
   	   //		 break;
   		 default:
	   		 ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_eval,
	   				 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
	   				 (Int)QtProfile::RMSE, 0);
	   		 break;
   		 }
   	 }
		 break;
    case QtProfile::PPROPAG:
		if (cSys.qualityAxisNumber() <0){
			*itsLog << LogIO::NORMAL << "Can not do the plot request, no quality axis!" << LogIO::POST;
			if (z_eval.size()> 0)
				z_eval.resize(0);
		}
		else {
			switch (itsPlotType)
			{
			case QtProfile::PMEAN:
				ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_eval,
						"world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
						(Int)QtProfile::NSQRTSUM, 1);
				break;
			case QtProfile::PMEDIAN:
				*itsLog << LogIO::NORMAL << "Can not plot the error, NO propagation for median!" << LogIO::POST;
				if (z_eval.size()> 0)
					z_eval.resize(0);
				break;
			case QtProfile::PSUM:
				ok=analysis->getFreqProfile( wxv, wyv, z_xval, z_eval,
						"world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
						(Int)QtProfile::SQRTSUM, 1);
				break;
		    default:
		    	if (z_eval.size()> 0)
		    		z_eval.resize(0);
		    	break;
			}
		}
		break;
    default:
   	 if (z_eval.size()> 0)
   		 z_eval.resize(0);
   	 break;
    }
    //cout << "Lambda values: " << z_xval << endl;
    //cout << "Y values: " << z_yval << endl;

    if ( ! ok ) {
   	 // change to notify user of error...
    	*itsLog << LogIO::WARN << "Can not generate the frequency error profile!" << LogIO::POST;
    	return;
    }


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
    while(symax < dmin && ymax != 0.0){
   	 ordersOfM += 3;
   	 symax = ymax * pow(10.,ordersOfM);
    }
    while(symax > dmax && ymax != 0.0){
   	 ordersOfM -= 3;
   	 symax = ymax * pow(10.,ordersOfM);
    }

    if(ordersOfM!=0){
   	 // correct display y axis values
   	 for (uInt i = 0; i < z_yval.size(); i++) {
   		 z_yval(i) *= pow(10.,ordersOfM);
   	 }
   	 if (z_eval.size() > 0){
   		 for (uInt i = 0; i < z_eval.size(); i++) {
   			 z_eval(i) *= pow(10.,ordersOfM);
   		 }
   	 }
    }

    if(ordersOfM!=0){
   	 // correct unit string
   	 if(yUnit.startsWith("(")||yUnit.startsWith("[")||
   			 yUnit.startsWith("\"")){
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
    
    pc->clearData();
    pc->plotPolyLine(z_xval, z_yval, z_eval, fileName);


    QHashIterator<QString, ImageAnalysis*> i(*over);
    while (i.hasNext() && multiProf->isChecked()) {
   	 i.next();
   	 //qDebug() << i.key() << ": " << i.value();
   	 QString ky = i.key();
   	 ImageAnalysis* ana = i.value();
   	 Vector<Float> xval(100);
   	 Vector<Float> yval(100);

   	 switch (itsPlotType)
   	 {
   	 case QtProfile::PMEAN:
   		 ok=ana->getFreqProfile( wxv, wyv, xval, yval,
   				 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
   				 (Int)QtProfile::MEAN, 0);
   		 break;
   	 case QtProfile::PMEDIAN:
   		 ok=ana->getFreqProfile( wxv, wyv, xval, yval,
   				 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
   				 (Int)QtProfile::MEDIAN, 0);
   		 break;
   	 case QtProfile::PSUM:
   		 ok=ana->getFreqProfile( wxv, wyv, xval, yval,
   				 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
   				 (Int)QtProfile::PSUM, 0);
   		 break;
   		 //case QtProfile::PVRMSE:
   		 //	ok=ana->getFreqProfile( wxv, wyv, xval, yval,
   		 //			"world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
   		 //			(Int)QtProfile::RMSE, 0);
   		 // break;
   	 default:
   		 ok=ana->getFreqProfile( wxv, wyv, xval, yval,
   				 "world", coordinateType, 0, 0, 0, xaxisUnit, frameType_p,
   				 (Int)QtProfile::MEAN, 0);
   		 break;
   	 }

   	 if (ok) {
   		 if(ordersOfM!=0){
   			 // correct display y axis values
   			 for (uInt i = 0; i < yval.size(); i++) {
   				 yval(i) *= pow(10.,ordersOfM);
   			 }
   		 }
   		 Vector<Float> xRel(yval.size());
   		 Vector<Float> yRel(yval.size());
   		 Int count = 0;
   		 if (relative->isChecked()) {
   			 ky = ky+ "_rel.";
   			 for (uInt i = 0; i < yval.size(); i++) {
   				 uInt k = z_yval.size() - 1;
   				 //cout << xval(i) << " - " << yval(i) << endl;
   				 //cout << z_xval(0) << " + " << z_xval(k) << endl;
   				 if (coordinateType.contains("elocity")) {
   					 if (xval(i) < z_xval(0) && xval(i) >= z_xval(k)) {
   						 for (uInt j = 0; j < k; j++) {
   							 //cout << z_xval(j) << " + "
   							 //     << z_yval(j) << endl;
   							 if (xval(i) <= z_xval(j) &&
   									 xval(i) > z_xval(j + 1)) {
   								 float s = z_xval(j + 1) - z_xval(j);
   								 if (s != 0) {
   									 xRel(count) = xval(i);
   									 yRel(count)= yval(i) -
   											 (z_yval(j) + (xval(i) - z_xval(j)) / s *
   													 (z_yval(j + 1) - z_yval(j)));
   									 count++;
   									 //yval(i) -= (z_yval(j) + (xval(i) - z_xval(j)) / s *
   									 //           (z_yval(j + 1) - z_yval(j)));

   								 }
   								 break;
   							 }
   						 }
   					 }
   				 }
   				 else {
   					 if (xval(i) >= z_xval(0) && xval(i) < z_xval(k)) {
   						 for (uInt j = 0; j < k; j++) {
   							 if (xval(i) >= z_xval(j) && xval(i) < z_xval(j + 1)) {
   								 float s = z_xval(j + 1) - z_xval(j);
   								 if (s != 0) {
   									 xRel(count) = xval(i);
   									 yRel(count)= yval(i) -
   											 (z_yval(j) + (xval(i) - z_xval(j)) / s *
   													 (z_yval(j + 1) - z_yval(j)));
   									 count++;
   									 //yval(i) -= (z_yval(j) + (xval(i) - z_xval(j)) / s *
   											 //           (z_yval(j + 1) - z_yval(j)));
   								 }
   								 break;
   							 }
   						 }
   					 }
   				 }
   			 }
   			 xRel.resize(count, True);
   			 yRel.resize(count, True);
   			 pc->addPolyLine(xRel, yRel, ky);
   		 }
   		 else {
   			 pc->addPolyLine(xval, yval, ky);
   		 }
   	 }
    }

    lastWX.assign(wxv);
    lastWY.assign(wyv);
    lastPX.assign(pxv);
    lastPY.assign(pyv);

}

void QtProfile::redraw( ) {
    wcChanged( last_event_cs, last_event_px, last_event_py, last_event_wx, last_event_wy, UNKNPROF);
}

void QtProfile::changeAxis(String xa, String ya, String za, std::vector<int> ) {
	//cout << "change axis=" << xa << " " << ya
	//     << " " << za << " cube=" << cube << endl;
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
	//if (cb != cube) {
	cube = cb;
	xpos = "";
	ypos = "";
	position = QString("");
	te->setText(position);
	if (cube == 0)
		pc->setWelcome("No profile available "
				"for the given data \nor\n"
				"No profile available for the "
				"display axes orientation"
		);
	else
		pc->setWelcome("assign a mouse button to\n"
				"'crosshair' or 'rectangle' or 'polygon'\n"
				"click/press+drag the assigned button on\n"
				"the image to get a image profile");

	pc->clearCurve();
	//}

	//cout << "cube=" << cube << endl;

}

void QtProfile::overplot(QHash<QString, ImageInterface<float>*> hash) {

	// re-set the images
	// that are overplotted
	if (over) {
		delete over;
		over = 0;
	}

	over = new QHash<QString, ImageAnalysis*>();
	QHashIterator<QString, ImageInterface<float>*> i(hash);
	while (i.hasNext()) {
		i.next();
		//qDebug() << i.key() << ": " << i.value();
		QString ky = i.key();
		ImageAnalysis* ana = new ImageAnalysis(i.value());
		(*over)[ky] = ana;
	}
}

void QtProfile::fillPlotTypes(){

	if (plotMode->count() <1 ){

		// fill the plot types
		plotMode->addItem("mean");
		plotMode->addItem("median");
		plotMode->addItem("sum");
		//plotMode->addItem("rmse");

		// read the preferred plot mode from casarc
		QString pref_plotMode = QString(rc.get("viewer." + rcid() + ".plot.type").c_str());
		if (pref_plotMode.size()>0){
			// change to the preferred plotMode
			int etypeindex= plotMode->findText(pref_plotMode);
			if (etypeindex > -1)
				plotMode->setCurrentIndex(etypeindex);
			else
				plotMode->setCurrentIndex(0);
		}
		stringToPlotType(plotMode->currentText(), itsPlotType);
	}

	// clean out the error box
	if (errorMode->count() > 0){
		if (errorMode->findText("propagated") > -1){
			errorMode->removeItem(errorMode->findText("propagated"));
		}
		if (errorMode->findText("rmse") > -1){
			errorMode->removeItem(errorMode->findText("rmse"));
		}
	}

	// get the coo-sys
	CoordinateSystem cSys = image->coordinates();

	// add the 'no error' option
	if (errorMode->findText("no error") < 0)
		errorMode->addItem("no error");

	switch (itsPlotType)
	{
	case QtProfile::PMEAN:
		if (npoints!=1)
			errorMode->addItem("rmse");
		if (cSys.qualityAxisNumber() > -1)
			errorMode->addItem("propagated");
		break;
	case QtProfile::PMEDIAN:
		if (npoints!=1)
			errorMode->addItem("rmse");
		break;
	case QtProfile::PSUM:
		if (cSys.qualityAxisNumber() > -1)
			errorMode->addItem("propagated");
		break;
	//case QtProfile::PVRMSE:
	//	break;
	}

	// read the preferred error mode from casarc
   QString pref_errMode = QString(rc.get("viewer." + rcid() + ".error.type").c_str());
   if (pref_errMode.size()>0){
     // change to the preferred plotMode
   	int e_index= errorMode->findText(pref_errMode);
   	if (e_index > -1)
   		errorMode->setCurrentIndex(e_index);
   	else
   		errorMode->setCurrentIndex(0);
   }

   stringToErrorType(errorMode->currentText(), itsErrorType);
	rc.put( "viewer." + rcid() + ".error.type", errorMode->currentText().toStdString());
}

void QtProfile::getcoordTypeUnit(String &ctypeUnitStr, String &cTypeStr, String &unitStr){
   // determine the coordinate type
   if (ctypeUnitStr.contains("air wavelength"))
   	cTypeStr = String("air wavelength");
   else if (ctypeUnitStr.contains("wavelength"))
   	cTypeStr = String("wavelength");
   else if (ctypeUnitStr.contains("radio velocity"))
   	cTypeStr = String("radio velocity");
   else if (ctypeUnitStr.contains("optical velocity"))
   	cTypeStr = String("optical velocity");
   else if (ctypeUnitStr.contains("frequency"))
   	cTypeStr = String("frequency");
   else
   	cTypeStr = String("channel");

   // determine the unit
   if (ctypeUnitStr.contains("[Hz]"))
   	unitStr = String("Hz");
   else if (ctypeUnitStr.contains("[MHz]"))
   	unitStr = String("MHz");
   else if (ctypeUnitStr.contains("[GHz]"))
   	unitStr = String("GHz");
   else if (ctypeUnitStr.contains("[m/s]"))
   	unitStr = String("m/s");
   else if (ctypeUnitStr.contains("[km/s]"))
   	unitStr = String("km/s");
   else if (ctypeUnitStr.contains("[mm]"))
   	unitStr = String("mm");
   else if (ctypeUnitStr.contains("[um]"))
   	unitStr = String("um");
   else if (ctypeUnitStr.contains("[nm]"))
   	unitStr = String("nm");
   else if (ctypeUnitStr.contains("[Angstrom]"))
   	unitStr = String("Angstrom");
   else
   	unitStr = String("");
}

/*
void QtProfile::doImgCollapse(){

	//const String ALL = RegionManager::ALL;
	const String ALL = "ALL";

	// form the output image
	const String outname = String(fileName.toStdString()) + String("_collapse");

	// get the pixel axis number of the spectral axis
	CoordinateSystem cSys = image->coordinates();
	Int specAx = cSys.findCoordinate(Coordinate::SPECTRAL);
	Vector<Int> nPixelSpec = cSys.pixelAxes(specAx);
	const uInt nAxisCollapse=nPixelSpec(0);

	cout << "Collapsing axis: " << specAx << " to image: " << outname << endl;

	// do he image collapse
	//ImageCollapser collapser("mean", image, "", "", ALL, ALL, "", nAxisCollapse, outname, True);
	//collapser.collapse(False);

	cout << "Collapsing finished!"<< endl;

	// transmit the collapsed image
	emit add2DImage(outname, "image", "raster", True);
}*/
}
