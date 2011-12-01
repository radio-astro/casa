//# QtButtonGroup.cc: Like QButtonGroup but with additional functionality.
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
#include <casaqt/QtUtilities/QtButtonGroup.qo.h>

namespace casa {

///////////////////////////////
// QTBUTTONGROUP DEFINITIONS //
///////////////////////////////

QtButtonGroup::QtButtonGroup(QObject* parent) : QObject(parent),
        isEnabled_(true), itsCheckedButton_(NULL) { }

QtButtonGroup::~QtButtonGroup() { }


QList<QAbstractButton*> QtButtonGroup::buttons() const { return itsButtons_; }
QAbstractButton* QtButtonGroup::checkedButton() const {
    return itsCheckedButton_; }

void QtButtonGroup::addButton(QAbstractButton* button) {
    if(button != NULL && !itsButtons_.contains(button)) {
        itsButtons_.append(button);
        button->setEnabled(isEnabled_);
        connect(button, SIGNAL(clicked(bool)), SLOT(clicked(bool)));
        connect(button, SIGNAL(toggled(bool)), SLOT(toggled(bool)));
        connect(button, SIGNAL(pressed()), SLOT(pressed()));
        connect(button, SIGNAL(released()), SLOT(released()));
        
        if(button->isChecked()) {
            // Replace the currently checked button, if there is one.
            QAbstractButton* old = itsCheckedButton_;
            itsCheckedButton_ = button;
            if(old != NULL) old->setChecked(false);
        }
    }
}

void QtButtonGroup::removeButton(QAbstractButton* button) {
    if(button != NULL && itsButtons_.contains(button)) {
        itsButtons_.removeAll(button);
        disconnect(button, SIGNAL(clicked(bool)), this, SLOT(clicked(bool)));
        disconnect(button, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
        disconnect(button, SIGNAL(pressed()), this, SLOT(pressed()));
        disconnect(button, SIGNAL(released()), this, SLOT(released()));
        
        if(itsCheckedButton_ == button) {
            itsCheckedButton_ = NULL;
            emit unchecked();
        }
    }
}


bool QtButtonGroup::isEnabled() const { return isEnabled_; }


void QtButtonGroup::setEnabled(bool enabled) {
    isEnabled_ = enabled;
    foreach(QAbstractButton* button, itsButtons_)
        button->setEnabled(enabled);
}


void QtButtonGroup::toggled_(QAbstractButton* button, bool checked) {
    bool allUnchecked = false;
    if(checked) {
        // Enforce mutual exclusivity if checked.
        if(itsCheckedButton_ != button) {
            QAbstractButton* old = itsCheckedButton_;
            itsCheckedButton_ = button;
            if(old != NULL) old->setChecked(false);
        }
        
    } else {
        // Check if all actions are unchecked.
        allUnchecked = true;
        foreach(QAbstractButton* button, itsButtons_) {
            if(button->isChecked()) {
                allUnchecked = false;
                break;
            }
        }
        if(allUnchecked) itsCheckedButton_ = NULL;
    }
    
    emit buttonClicked(button);
    if(allUnchecked) emit unchecked();
}


void QtButtonGroup::clicked(bool checked) {
    QAbstractButton* button = dynamic_cast<QAbstractButton*>(sender());
    // Only process if not checkable, because otherwise the toggled signal will
    // also be sent and used.
    if(button != NULL && !button->isCheckable()) toggled_(button, checked);
}

void QtButtonGroup::toggled(bool checked) {
    QAbstractButton* button = dynamic_cast<QAbstractButton*>(sender());
    if(button != NULL) toggled_(button, checked);
}

void QtButtonGroup::pressed() {
    QAbstractButton* button = dynamic_cast<QAbstractButton*>(sender());
    if(button != NULL) emit buttonPressed(button);
}

void QtButtonGroup::released() {
    QAbstractButton* button = dynamic_cast<QAbstractButton*>(sender());
    if(button != NULL) emit buttonReleased(button);
}

}
