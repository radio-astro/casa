//# TBTaQL.qo.h: GUI for entering a TaQL command.
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
#include <casaqt/QtBrowser/TBTaQL.qo.h>

namespace casa {

////////////////////////
// TBTAQL DEFINITIONS //
////////////////////////

// Constructors/Destructors //

TBTaQL::TBTaQL(QWidget* parent, Commands commands) : QDialog(parent) {
    setupUi(this);
    
    if(parent == NULL) {
        connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
        connect(closeButton, SIGNAL(clicked()), this, SLOT(deleteLater()));
    } else {
        connect(closeButton, SIGNAL(clicked()), this, SLOT(doClose()));
    }
    
    if(commands.testFlag(SELECT))
        commandChooser->addItem(command(SELECT).c_str());
    else selectFrame->close();
    
    if(commands.testFlag(UPDATE))
        commandChooser->addItem(command(UPDATE).c_str());
    else updateFrame->close();

    if(commands.testFlag(INSERT))
        commandChooser->addItem(command(INSERT).c_str());
    else insertFrame->close();

    if(commands.testFlag(DELETE))
        commandChooser->addItem(command(DELETE).c_str());
    else deleteFrame->close();

    if(commands.testFlag(CALC))
        commandChooser->addItem(command(CALC).c_str());
    else calcFrame->close();
    
    if(commands.testFlag(CREATE))
        commandChooser->addItem(command(CREATE).c_str());
    else createFrame->close();
    
    connect(generateButton, SIGNAL(clicked()), this, SLOT(doGenerate()));
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(doAccept()));
    connect(browseButton, SIGNAL(clicked()), this, SLOT(doBrowse()));
    
    commandEdit->setPlainText("");
}

TBTaQL::~TBTaQL() { }

// Private Slots //

void TBTaQL::doBrowse() {
    QString f = QFileDialog::getExistingDirectory(this, tr("Find Data"));    
    if(!f.isEmpty()) locationEdit->setText(f);
}

void TBTaQL::doGenerate() {
    String cmd = qPrintable(commandChooser->currentText());
    
    stringstream ss;
    ss << cmd;
    if(cmd == command(SELECT)) {
        if(!allColumnsBox->isChecked()) {
            if(colDistinctBox->isChecked()) ss << " DISTINCT";
            ss << ' ' << qPrintable(colEdit->text());
        }
        
        ss << " FROM " << qPrintable(locationEdit->text());
        
        if(whereBox->isChecked())
            ss << " WHERE " << qPrintable(whereEdit->text());
        
        if(orderbyBox->isChecked()) {
            ss << " ORDERBY ";
            if(orderbyDistinctBox->isChecked()) ss << " DISTINCT ";
            ss << qPrintable(orderbyEdit->text());
        }
        
        if(limitBox->isChecked())
            ss << " LIMIT " << qPrintable(limitEdit->text());
        
        if(offsetBox->isChecked())
            ss << " OFFSET " << qPrintable(offsetEdit->text());
        
    // TODO
    } else if(cmd == command(UPDATE)) {
        
        
    } else if(cmd == command(INSERT)) {
        
        
    } else if(cmd == command(DELETE)) {
        
        
    } else if(cmd == command(CALC)) {
        
        
    } else if(cmd == command(CREATE)) {
        
        
    }
    commandEdit->setPlainText(ss.str().c_str());
}

void TBTaQL::doAccept() {
    String str = qPrintable(commandEdit->toPlainText());
    if(str.empty()) {
        QMessageBox::warning(this, "Error", "TaQL command cannot be empty!");
        return;
    }
    emit command(str);
    if(parent() == NULL) {
        close();
        deleteLater();
    }
}

void TBTaQL::doClose() {
    emit closeRequested();
}

}
