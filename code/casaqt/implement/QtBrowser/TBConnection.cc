//# TBConnection.cc: Dialog for opening/connecting to a table with options.
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
//# $Id: $
#include <casaqt/QtBrowser/TBConnection.qo.h>
#include <casaqt/QtBrowser/TBMain.qo.h>
#include <casaqt/QtBrowser/TBTable.h>

namespace casa {

//////////////////////////////
// TBCONNECTION DEFINITIONS //
//////////////////////////////

// Constructors / Destructors //

TBConnection::TBConnection(TBMain* m, QWidget* p): QDialog(p), parent(m) {
    setupUi(this);
    
    // Connect widgets
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(accepted()));
    connect(locBrowse, SIGNAL(clicked()), this, SLOT(browse()));
    connect(locLocal, SIGNAL(clicked(bool)), this, SLOT(clickedLocal()));
    connect(locRemote, SIGNAL(clicked(bool)), this, SLOT(clickedRemote()));
    locEdit->installEventFilter(this);
    locHost->installEventFilter(this);
    locPort->installEventFilter(this);
    rowStart->installEventFilter(this);
    rowNum->installEventFilter(this);
    connect(driverDirect, SIGNAL(clicked(bool)), this, SLOT(clickedDirect()));
    connect(driverXML, SIGNAL(clicked(bool)), this, SLOT(clickedXML()));
    connect(driverHome, SIGNAL(clicked(bool)), this, SLOT(clickedHome()));
    connect(driverDom, SIGNAL(clicked(bool)), this, SLOT(clickedDOM()));
    connect(driverSax, SIGNAL(clicked(bool)), this, SLOT(clickedSAX()));
    
    rowStart->setText("0");
    rowNum->setText(TBConstants::itoa(
                           TBConstants::DEFAULT_SELECT_NUM).c_str());
    QIntValidator* v = new QIntValidator(rowStart);
    v->setBottom(0);
    rowStart->setValidator(v);
    v = new QIntValidator(rowNum);
    v->setBottom(1);
}

TBConnection::~TBConnection() { }

// Protected Methods //

bool TBConnection::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::FocusIn) { // "clicked" event
        if(obj == locEdit) clickedLocation();
        else if(obj == locHost) clickedHost();
        else if(obj == locPort) clickedPort();
        else if(obj == rowStart) clickedStart();
        else if(obj == rowNum) clickedNum();
    }
    return QObject::eventFilter(obj, event);
}

// Private Slots //

void TBConnection::accepted() {
    String location = qPrintable(locEdit->text());
    if(location.empty()) {
        QMessageBox::critical(this, "Open Error",
                "Location field cannot be empty.");
        return;
    }

    // start/num
    int start, num;
    String str = qPrintable(rowStart->text());
    if(TBConstants::atoi(str, &start) != 1 || start < 0) start = 0;
    str = qPrintable(rowNum->text());
    if(TBConstants::atoi(str, &num) != 1 || num < 1)
        num = TBConstants::DEFAULT_SELECT_NUM;

    // ignore local/remote option until it's implemented

    // driver
    Driver d = DIRECT;
    // if(driverXML->isChecked()) d = XML;

    Parser p = XERCES_SAX;
    if(driverHome->isChecked()) p = HOME;
    else if(driverDom->isChecked()) p = XERCES_DOM;

    DriverParams* dp = new DriverParams(d, p);
    close();
    parent->openTable(location, dp, start, num);
}

void TBConnection::browse() {
    QString file = QFileDialog::getExistingDirectory(this, tr("Find Data"),
                                parent->getLastOpenedDirectory().c_str());
    if(!file.isEmpty()) locEdit->setText(file);
    label->setText(TBConstants::OPEN_TEXT_LOCATION.c_str());
}

void TBConnection::clickedLocal() {
    label->setText(TBConstants::OPEN_TEXT_LOCAL.c_str());
}

void TBConnection::clickedRemote() {
    label->setText(TBConstants::OPEN_TEXT_REMOTE.c_str());
}

void TBConnection::clickedHost() {
    label->setText(TBConstants::OPEN_TEXT_HOST.c_str());
}

void TBConnection::clickedPort() {
    label->setText(TBConstants::OPEN_TEXT_PORT.c_str());
}

void TBConnection::clickedLocation() {
    label->setText(TBConstants::OPEN_TEXT_LOCATION.c_str());
}

void TBConnection::clickedDirect() {
    label->setText(TBConstants::OPEN_TEXT_DIRECT.c_str());
}

void TBConnection::clickedXML() {
    label->setText(TBConstants::OPEN_TEXT_XML.c_str());
}

void TBConnection::clickedHome() {
    label->setText(TBConstants::OPEN_TEXT_HOME.c_str());
}

void TBConnection::clickedDOM() {
    label->setText(TBConstants::OPEN_TEXT_DOM.c_str());
}

void TBConnection::clickedSAX() {
    label->setText(TBConstants::OPEN_TEXT_SAX.c_str());
}

void TBConnection::clickedStart() {
    label->setText(TBConstants::OPEN_TEXT_START.c_str());
}

void TBConnection::clickedNum() {
    label->setText(TBConstants::OPEN_TEXT_NUM.c_str());
}

}
