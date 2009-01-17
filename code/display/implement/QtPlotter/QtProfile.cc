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
#include <images/Images/WCBox.h>
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
         coordinate("world"), coordinateType("velocity"),
         fileName(name), position(""), xpos(""), ypos(""),
         cube(0)
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
    label->setAlignment(Qt::AlignBottom | Qt::AlignRight);
    ctype = new QComboBox(this);
    ctype->addItem("velocity");
    ctype->addItem("frequency");
    ctype->addItem("pixel");
    connect(ctype, SIGNAL(currentIndexChanged(const QString &)), 
            this, SLOT(changeCoordinateType(const QString &)));
    displayLayout->addWidget(label);
    displayLayout->addWidget(chk);
    displayLayout->addWidget(te);
    displayLayout->addWidget(ctype);

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
    pc->setYLabel("Flux Density (mJy)", 10, 0.5, "Helvetica [Cronyx]");
    pc->setXLabel(QString(coordinateType.chars()), 10, 0.5, "Helvetica [Cronyx]");
    pc->setAutoScale(autoScale->checkState());
       
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


    char* t = "plt";
    QString ext = fn.section('.', -1);
    if (ext == "txt" || ext == "plt")
        t = (char*)ext.toLocal8Bit().constData();
    else 
        fn.append(".plt"); 

    QFile file(fn);
    if (!file.open(QFile::WriteOnly | QIODevice::Text))
        return ;
    QTextStream ts(&file);
    
    Vector<Double> xy(2);
    bool xv = 0, yv = 0; 
    double x = xpos.toDouble(&xv);
    double y = ypos.toDouble(&yv);
    if (!xv || !yv) 
       return;
    xy[0]=x; xy[1]=y;
    Vector<Float> z_xval;
    Vector<Float> z_yval;
    Bool ok = False;
    ok=analysis->getFreqProfile(xy, z_xval, z_yval, 
                          coordinate, coordinateType);

    ts << "#title: Image profile - " << fileName << " " << position << "\n";
    ts << "#coordintate: " << QString(coordinate.chars()) << "\n";
    ts << "#xLabel: " << QString(coordinateType.chars()) << "\n";
    ts << "#yLabel: " << "Flux Density (mJy)\n";
    
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
    xpos = "";
    ypos = "";
    position = QString("");
    te->setText(position);
    pc->clearCurve(0);
    //cout << "changeCoordinate: coordinate=" 
    //       << coordinate << endl;
    emit coordinateChange(coordinate);
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
    if (text.contains("freq")) lbl.append(" (GHz)");
    if (text.contains("velo")) lbl.append(" (km/s)");
    pc->setXLabel(lbl, 10, 0.5, "Helvetica [Cronyx]");
    pc->setPlotSettings(QtPlotSettings());
    zoomInButton->setVisible(0);
    zoomOutButton->setVisible(0);
    upButton->setVisible(0);
    downButton->setVisible(0);
    leftButton->setVisible(0);
    rightButton->setVisible(0);

}
/*
void QtProfile::crosshairReady(const String& evtype) {
   //qDebug() << "profile: " << evtype.c_str() << x << y;	
   pc->setTitle("");
   pc->setWelcome("");
   if (!analysis) return;
   //qDebug() << "analysis != 0";
   Double x, y;
   if (coordinate == "world") {
      getWorld(x, y);	
      position = getRaDec(x, y);
   }
   else {
      getLin(x, y);	
      position = QString("[%1, %2]").arg(x).arg(y);
   }
 
   xpos = QString::number(x);
   ypos = QString::number(y);

   te->setText(position);
   Vector<Double> xy(2);
   xy[0]=x; xy[1]=y;
   Vector<Float> z_xval;
   Vector<Float> z_yval;
   //Get Profile Flux density v/s velocity
   Bool ok = False;
   ok=analysis->getFreqProfile(xy, z_xval, z_yval, coordinate, coordinateType);
   //qDebug() << "ok" << ok;
   pc->plotPolyLine(z_xval, z_yval);
}
*/
void QtProfile::closeEvent (QCloseEvent* event) {
   //qDebug() << "closeEvent";
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

    pc->setYLabel("Flux Density (mJy)", 10, 0.5, "Helvetica [Cronyx]");
    pc->setXLabel(QString(coordinateType.chars()), 10, 0.5, "Helvetica [Cronyx]");
       
    xpos = "";
    ypos = "";
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
    //cout << "ns=" << ns << endl;  
  
    Vector<Double> xv(ns);
    Vector<Double> yv(ns);
    if (cube == -1)
        for (Int i = 0; i < ns; i++) {
            xv(i) = y(i);
            yv(i) = x(i); 
        }
    else 
        for (Int i = 0; i < ns; i++) {
            xv(i) = x(i);
            yv(i) = y(i); 
        }

    if (ns < 1) return;
    if (ns == 1) {
        pc->setTitle("Single Point Profile");
    }
    else if (ns == 2) {
        pc->setTitle("Rectangle Region Profile");
    }
    else {
        pc->setTitle("Polygon Region Profile");
    }
    pc->setWelcome("");

    //xpos, ypos and position only used for display
    xpos = QString::number(xv[0]);
    ypos = QString::number(yv[0]);
    //qDebug() << c.chars() << "xpos:" << xpos << "ypos:" << ypos;

    if (coordinate == "world") {
       position = getRaDec(xv[0], yv[0]);
    }
    else {
       position = QString("[%1, %2]").arg(xpos).arg(ypos);
    }
    //qDebug() << "position:" << position;
    te->setText(position);

    Vector<Float> z_xval;
    Vector<Float> z_yval;
    //Get Profile Flux density v/s velocity
    Bool ok = False;
    ok=analysis->getFreqProfile(xv, yv, z_xval, z_yval, 
                coordinate, coordinateType);
    pc->plotPolyLine(z_xval, z_yval);

}

void QtProfile::changeAxis(String xa, String ya, String za) {
   //cout << "change axis=" << xa << " " << ya 
   //     << " " << za << endl;
   int cb = 0;
   if (xa.contains("Decl") && ya.contains("Right"))
       cb = -1;
   if (xa.contains("Right") && ya.contains("Decl"))
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
