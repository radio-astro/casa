//# QtViewerPrintGui.qo.h:  Printing dialog for QtViewer
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

#ifndef QTVIEWERPRINTGUI_H
#define QTVIEWERPRINTGUI_H


#include <casa/aips.h>

#include <graphics/X11/X_enter.h>
#  include <QtCore>
#  include <QtGui>
#  include <QtXml/QDomDocument>
   //#dk Be careful to put *.ui.h within X_enter/exit bracket too,
   //#   because they'll have Qt includes.
   //#   E.g. <QApplication> needs the X11 definition of 'Display'
#  include <display/QtViewer/printControl.ui.h>
#include <graphics/X11/X_exit.h>


#include <display/QtAutoGui/QtGuiEntry.qo.h>
#include <display/QtAutoGui/QtXmlRecord.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>


namespace casa { //# NAMESPACE CASA - BEGIN


//////////////////////////  QtViewerPrintGui  /////////////////////////

// <summary>
// viewer print widget
// </summary>

class QtViewerPrintGui : public QWidget
{
    Q_OBJECT

public:

    QtViewerPrintGui(QtDisplayPanel *dp);
    ~QtViewerPrintGui();
  
    QString printFileName() const;
    QString printerName() const;
    bool load(QDomDocument &doc);
    void loadRecord(Record rec);
    static const QString printRecord;
    void adjustEpsBoundingBox( const char *from, const char *to,
			       const QSize &wcmax, const QRect &viewport );
    void printPS(QPrinter *printer, const QString printerType);
    friend ostream& operator <<(ostream &os,const QtViewerPrintGui &obj);
    friend ostream& operator <<(ostream &os,const QPrinter &printer); 
signals:

private slots:
public slots:
    void saveXPM();
    void savePS(QPixmap*, int, int);
    void print();
    void dismiss();
    void itemValueChanged(QString name, QString value, int action, bool apply);
    void ps2eps(const QString &filename, QRect rect);
    void printToFile(const QString &filename,bool isEPS);
    void selectOutputType( const QString & text );

private:
    QDomDocument m_doc;
    QVBoxLayout *vboxLayout;
    QLabel* sizeLabel;
    QString printfilename;
    QString savefiletype;
    QString printmedia;
    QString printorientation;
    int printresolution;
    float printmagnification;
    QString printepsformat;
    std::pair<int, int> printimgresolution;
    double printimgsizefactor;
    QPrinter *printer;
    QtDisplayPanel *pDP;
    
private slots:
    void dpResized(QSize panelSize, QSize canvasSize);
};

// <summary>
// viewer print widget
// </summary>

// <synopsis>
//  synopsis for the QtViewerPrintGui
// </synopsis>

//////////////////////////  QtViewerPrintCtl /////////////////////////
class QtViewerPrintCtl : public QDialog, private Ui::PrintControl
{
    Q_OBJECT
public:

    QtViewerPrintCtl(QtViewerPrintGui *parent = 0);
    ~QtViewerPrintCtl();
    QString fileName() const;
    QString printerName() const;
    QString orientation() const;
    QString paper() const;

signals:
   void printIt();
private slots:
    void checkPrinter();
public slots:

private:

};


} //# NAMESPACE CASA - END

#endif
