//# QtActionGroup.qo.h: Like QActionGroup but with additional functionality.
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
#ifndef QTACTIONGROUP_QO_H_
#define QTACTIONGROUP_QO_H_

#include <QAbstractButton>
#include <QAction>
#include <QList>
#include <QMultiMap>

using namespace std;

namespace casa {

// This class is used to synchronize the checked state of actions with
// checkable buttons or other actions.  This makes it so that clicking the
// synchronized button/action has the same effect as clicking the action
// itself.
class QtActionSynchronizer : public QObject {
    Q_OBJECT
    
public:
    // Constructor which takes an optional parent.
    QtActionSynchronizer(QObject* parent = NULL);
    
    // Destructor.
    virtual ~QtActionSynchronizer();
    
    // Synchronizes the given action in this group with the given button or
    // other action.
    // <group>
    virtual void synchronize(QAction* action, QAbstractButton* button);
    virtual void synchronize(QAction* action, QAction* otherAction);
    // </group>
    
    // Unsynchronizes all buttons/actions associated with the given in this
    // group.
    // <group>
    virtual void unsynchronize(QAction* action);
    virtual void unsynchronize(QAbstractButton* button);
    // </group>
    
    // Unsyncrhonizes the given action in this group with the given
    // button/action.
    // <group>
    virtual void unsynchronize(QAction* action, QAbstractButton* button);
    virtual void unsynchronize(QAction* action, QAction* otherAction);
    // </group>
    
protected:
    // Synchronized actions maps.
    // <group>
    QMultiMap<QAction*, QAbstractButton*> itsSynchedButtons_;
    QMultiMap<QAction*, QAction*> itsSynchedActions_;
    // </group>
    
    
    // Method which updates the actions/buttons associated with the given
    // action.
    virtual void actionTriggered_(QAction* action, bool checked);
    
    // Method which updates the action(s) associated with the given action or
    // button to the given checked state.
    virtual void synchronizedTriggered_(QAction* action,
            QAbstractButton* button, bool checked);
    
protected slots:
    // Slot for when an action is toggled, which calls actionTriggered_() as
    // needed.
    virtual void actionToggled(bool checked);
    
    // Slot for when an action is triggered, which calls actionTriggered_() as
    // needed.
    virtual void actionTriggered(bool checked);

    // Slot for when a synchronizing button/action is toggled, which calls
    // synchronizedTriggered_() as needed.
    virtual void synchronizedToggled(bool checked);
    
    // Slot for when a synchronizing button/action is clicked/triggered, which
    // calls synchronizedTriggered_() as needed.
    virtual void synchronizedTriggered(bool checked);
};


// This class meant to be used almost identically to an exclusive QActionGroup.
// The difference is that this class allows all QActions to be unchecked; in
// other words, either one action is checked or none are.  Some methods from
// QActionGroup have not been included, but they would be easy to include if
// desired.  This class also has the added functionality included in
// QtActionSynchronnizer.
class QtActionGroup : public QtActionSynchronizer {
    Q_OBJECT
    
public:
    // Constructor that optionally takes a parent object.
    QtActionGroup(QObject* parent = NULL);
    
    // Destructor.
    ~QtActionGroup();

    // See QActionGroup::actions().
    QList<QAction*> actions() const;
    
    // See QActionGroup::checkedAction().  Will return NULL if no action is
    // currently checked.
    QAction* checkedAction() const;

    // See QActionGroup::addAction().
    QAction* addAction(QAction* action);
    
    // See QActionGroup::removeAction().
    void removeAction(QAction* action);
    
    // See QActionGroup::isEnabled().
    bool isEnabled() const;
    
    // Overrides QtActionSynchronizer::synchronize to only allow actions that
    // are in the group.
    // <group>
    void synchronize(QAction* action, QAbstractButton* button);
    void synchronize(QAction* action, QAction* otherAction);
    // </group>
    
public slots:    
    // Sets all actions to enabled.
    void setEnabled(bool enabled);
    
    // Sets all actions to disabled.
    void setDisabled(bool disabled) { setEnabled(!disabled); }
    
signals:
    // See QActionGroup::hovered().
    void hovered(QAction* action);
    
    // See QActionGroup::triggered().
    void triggered(QAction* action);
    
    // This signals is emitted when all actions in the group have been
    // unchecked.
    void unchecked();
    
private:
    // Enabled flag.
    bool isEnabled_;
    
    // Currently checked action, or NULL if none are checked.
    QAction* itsCheckedAction_;
    
    // Actions in group.
    QList<QAction*> itsActions_;
    
    
    // Enforces mutual exclusivity and emits signals as needed.
    void action_(QAction* action, bool checked);
    
private slots:
    // Slot for QAction::hover().
    void actionHovered();
    
    // Slot for QAction::triggered(), which calls actionTriggered_ as needed.
    void actionTriggered(bool checked);
    
    // Slot for QAction::toggled(), which calls actionTriggered_ as needed.
    void actionToggled(bool checked);
};

}

#endif /* QTACTIONGROUP_QO_H_ */
