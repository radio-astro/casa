//# QtEditingWidget.cc: Simple editing widgets.
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
#include <casaqt/QtUtilities/QtEditingWidget.qo.h>

#include <casaqt/QtUtilities/QtFileDialog.qo.h>

namespace casa {

///////////////////////////////
// QTLABELWIDGET DEFINITIONS //
///////////////////////////////

QtLabelWidget::QtLabelWidget(const String& defaultLabel, QWidget* parent) :
        QtEditingWidget(parent), itsDefault_(defaultLabel) {
    setupUi(this);
    
    setValue(defaultLabel);
    
    // only emit change for radio buttons turned on
    connect(noneButton, SIGNAL(toggled(bool)), SLOT(labelChanged(bool)));
    connect(defaultButton, SIGNAL(toggled(bool)), SLOT(labelChanged(bool)));
    connect(customButton, SIGNAL(toggled(bool)), SLOT(labelChanged(bool)));
    
    connect(customEdit, SIGNAL(textChanged(const QString&)),
    		SLOT(labelChanged()));
}

QtLabelWidget::~QtLabelWidget() { }

String QtLabelWidget::getValue() const {
    if(noneButton->isChecked()) return "";
    else if(defaultButton->isChecked()) return itsDefault_;
    else return customEdit->text().toStdString();
}

void QtLabelWidget::setValue(const String& value) {
    blockSignals(true);
    bool changed = value != itsValue_;
    
    itsValue_ = value;
    if(value == "")               noneButton->setChecked(true);
    else if(value == itsDefault_) defaultButton->setChecked(true);
    else                          customButton->setChecked(true);
    customEdit->setText(value.c_str());
    
    blockSignals(false);
    if(changed) emit this->changed();
}

void QtLabelWidget::addRadioButtonsToGroup(QButtonGroup* group) const {
    if(group == NULL) return;
    group->addButton(noneButton);
    group->addButton(defaultButton);
    group->addButton(customButton);
}

void QtLabelWidget::labelChanged(bool check) {
    if(!check) return;
    
    if(defaultButton->isChecked()) {
        bool oldCheck = check;
        check = false;
        customEdit->setText(itsDefault_.c_str());
        check = oldCheck;
    }
    
    emit changed();
    if(getValue() != itsValue_) emit differentFromSet();
}


//////////////////////////////
// QTFILEWIDGET DEFINITIONS //
//////////////////////////////

QtFileWidget::QtFileWidget(bool directory, bool save, QWidget* parent) :
        QtEditingWidget(parent), isDirectory_(directory), isSave_(save) {
    setupUi(this);
    connect(file, SIGNAL(textChanged(const QString&)), SLOT(fileChanged()));
    connect(file, SIGNAL(editingFinished()), SLOT(trimFileName()));
    connect(FileWidget::browse, SIGNAL(clicked()), SLOT(browse()));
}

QtFileWidget::~QtFileWidget() { }

String QtFileWidget::getFile() const { return file->text().toStdString(); }
void QtFileWidget::setFile(const String& f) {
    blockSignals(true);
    String f2 = f;
    f2.trim();
    bool changed = f2 != itsFile_;
    itsFile_ = f2;
    file->setText(f2.c_str());
    blockSignals(false);
    if(changed) emit this->changed();
}

void QtFileWidget::browse() {
    QString f;
    if(isDirectory_) f = QtFileDialog::qgetExistingDir(this, "", ".");
    else if(isSave_) f = QtFileDialog::qgetAnyFile(this);
    else             f = QtFileDialog::qgetExistingFile(this);
    if(!f.isEmpty()) file->setText(f);
}

void QtFileWidget::fileChanged() {
    emit changed();
    if(getFile() != itsFile_) emit differentFromSet();
}

void QtFileWidget::trimFileName() {
    blockSignals(true);
    String f = file->text().toStdString();
    f.trim();
    bool changed = (f != itsFile_);
    if(changed) {
        itsFile_ = f;
    }
    file->setText(itsFile_.c_str());
    blockSignals(false);
    emit this->changed();
}

}
