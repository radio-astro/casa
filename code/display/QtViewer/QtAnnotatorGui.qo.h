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

#ifndef QTANNOTATORGUIQO_H
#define QTANNOTATORGUIQO_H

#include <casa/aips.h>
#include <display/QtAutoGui/QtAutoGui.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/DisplayShapes/Annotations.h>
#include <casa/Containers/Record.h>


#include <graphics/X11/X_enter.h>
#  include <QDir>
#  include <QColor>
#  include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <display/QtViewer/QtAnnotatorGui.ui.h>
#include <graphics/X11/X_exit.h>


 
namespace casa { 

class QtViewer;
class QtMarkerGui;

// <summary>
// Annotating widget for a display panel.
// </summary>

// <synopsis>
// </synopsis>


class QtAnnotatorGui: public QWidget, private Ui::Annotating {
    Q_OBJECT
public:
    QtAnnotatorGui(QtDisplayPanel *parent = 0) ;
    ~QtAnnotatorGui();

    static const QString toolshorthands[12];
    static const QString basebitmaps[12];
    static const QString markerbitmaps[14];
    static const QString helptext[12];
    static const QString canvastype[3];

    static const int dlkeys[4];

private:
    QButtonGroup *bGroup;
    QButtonGroup *mGroup;

    void setup();
    QToolButton *button[12];
    QToolButton *marker[14];
    int activeTool;
    int activeMarker;
    int activeKey;
    QString canvasType;

    QtMarkerGui *gridLayout2;
    bool moveMarker;
    QtDisplayPanel* pDP;
    Annotations* annot;

private slots:
    void mouseReleaseEvent(QMouseEvent *event);
    void buttonClicked(QAbstractButton* button);
    void markerClicked(QAbstractButton* button);
    void buttonClicked(int button);
    void canvasLockChanged(const QString &text);
    void canvasLockChanged(int idx);

public:
    void markerReleased(QMouseEvent *event);
    QButtonGroup* mgroup();

public slots:
    void confirmToolKitChange(const QString, const Qt::MouseButton, const bool);
signals:
    void toolKitChange(const QString, const Qt::MouseButton, const bool);
    void setMarker(const QString);
    // Display::K_Pointer_Button1,  -- LeftButton  -- 1
    // Display::K_Pointer_Button2,  -- MidButton  -- 2
    // Display::K_Pointer_Button3;  -- RightButton  -- 3
};

class QtMarkerGui: public QWidget {
    Q_OBJECT
public:
    QtMarkerGui(QtAnnotatorGui *parent = 0, Qt::WFlags f = Qt::Popup) ;
    ~QtMarkerGui() ;
    virtual void mouseReleaseEvent ( QMouseEvent * event );
private:
    QtAnnotatorGui* parent;

};
 
}

#endif

