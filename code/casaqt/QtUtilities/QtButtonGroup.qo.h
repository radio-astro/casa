//# QtButtonGroup.qo.h: Like QButtonGroup but with additional functionality.
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
#ifndef QTBUTTONGROUP_QO_H_
#define QTBUTTONGROUP_QO_H_

#include <QAbstractButton>

namespace casa {

// This class meant to be used almost identically to an exclusive QButtonGroup.
// The difference is that this class allows all QAbstractButtons to be
// unchecked; in other words, either one button is checked or none are.  Some
// methods from QButtonGroup have not been included, but they would be easy to
// include if desired.
class QtButtonGroup : public QObject {
    Q_OBJECT
    
public:
    // Constructor that takes optional parent object.
    QtButtonGroup(QObject* parent);
    
    // Destructor.
    ~QtButtonGroup();
    
    
    // See QButtonGroup::buttons().
    QList<QAbstractButton*> buttons() const;
    
    // See QButtonGroup::checkedButton().  Will return NULL if no button is
    // currently checked.
    QAbstractButton* checkedButton() const;
    
    // See QButtonGroup::addButton().
    void addButton(QAbstractButton* button);
    
    // See QButtonGroup::removeButton().
    void removeButton(QAbstractButton* button);
    
    // Returns whether this group is enabled or not.
    bool isEnabled() const;
    
public slots:
    // Sets all buttons to enabled.
    void setEnabled(bool enabled);

    // Sets all buttons to disabled.
    void setDisabled(bool disabled) { setEnabled(!disabled); }
    
signals:
    // See QButtonGroup::buttonClicked().
    void buttonClicked(QAbstractButton* button);

    // See QButtonGroup::buttonPressed().
    void buttonPressed(QAbstractButton* button);

    // See QButtonGroup::buttonReleased().
    void buttonReleased(QAbstractButton* button);
    
    // This signals is emitted when all buttons in the group have been
    // unchecked.
    void unchecked();
    
private:
    // Enabled flag.
    bool isEnabled_;
    
    // Currently checked button, or NULL if none are checked.
    QAbstractButton* itsCheckedButton_;
    
    // Buttons in group.
    QList<QAbstractButton*> itsButtons_;
    
    
    // Method for when a button is the group is checked or toggled.
    void toggled_(QAbstractButton* button, bool checked);
    
private slots:
    // Slot for QAbstractButton::clicked(), which calls toggled_() as needed.
    void clicked(bool checked);
    
    // Slot for QAbstractButton::toggled(), which calls toggled_() as needed.
    void toggled(bool checked);
    
    // Slot for QAbstractButton::pressed().
    void pressed();
    
    // Slot for QAbstractButton::released().
    void released();
};

}

#endif /* QTBUTTONGROUP_QO_H_ */
