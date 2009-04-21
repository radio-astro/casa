//# TBFileDialog.cc: Subclass of QFileDialog with additional functionality.
//# Copyright (C) 2009
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
#include <casaqt/QtBrowser/TBFileDialog.qo.h>

#include <QLayout>

namespace casa {

//////////////////////////////
// TBFILEDIALOG DEFINITIONS //
//////////////////////////////

// Static //

QString TBFileDialog::getExistingTable(QWidget* parent, const QString& caption,
        const QString& directory) {
    TBFileDialog chooser(parent, caption, directory);
    chooser.setModal(true);
    chooser.setAcceptMode(AcceptOpen);
    chooser.setFileMode(DirectoryOnly);
    
    if(chooser.exec()) {    
        QStringList files = chooser.selectedFiles();
        if(files.size() > 0) return files[0];
    }
    
    return QString();
}


// Non-Static //

TBFileDialog::TBFileDialog(QWidget* parent, Qt::WindowFlags flags) :
        QFileDialog(parent, flags) {
    initialize();
}

TBFileDialog::TBFileDialog(QWidget* parent, const QString& caption,
        const QString& directory, const QString& filter) : QFileDialog(parent,
        caption, directory, filter) {
    initialize();
}

TBFileDialog::~TBFileDialog() { }

void TBFileDialog::initialize() {
    // NOTICE: This is only valid as long as QFileDialog continues to use a
    // QGridLayout.  I don't really like adding the label this way, but think
    // it's the best of the current options unless I've missed something..    
    QGridLayout* gl = dynamic_cast<QGridLayout*>(layout());
    if(gl == NULL) return;
    
    chosenLabel = new QLabel();
    chosenLabel->setWordWrap(true);
    gl->addWidget(new QLabel("Selected:"), gl->rowCount(), 0);
    gl->addWidget(chosenLabel, gl->rowCount() -1, 1, 1, gl->columnCount() -1);
    
    timeout();
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), SLOT(timeout()));
    timer->start(250);
}

void TBFileDialog::timeout() {
    chosenLabel->setText(selectedFiles().join(", ").replace("/", "/ "));
}

}
