//# QtActionGroup.cc: Like QActionGroup but with additional functionality.
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
#include <casaqt/QtUtilities/QtActionGroup.qo.h>

#include <iostream>

namespace casa {

//////////////////////////////////////
// QTACTIONSYNCRHONIZER DEFINITIONS //
//////////////////////////////////////

// Constructors/Destructors //

QtActionSynchronizer::QtActionSynchronizer(QObject* parent): QObject(parent){ }

QtActionSynchronizer::~QtActionSynchronizer() { }


// Public Methods //

void QtActionSynchronizer::synchronize(QAction* act, QAbstractButton* button) {
    if(act == NULL || button == NULL ||
       itsSynchedButtons_.contains(act, button)) return;
    
    unsynchronize(button);
    
    itsSynchedButtons_.insert(act, button);
    button->setChecked(act->isChecked());
    connect(act, SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
    connect(act, SIGNAL(triggered(bool)), SLOT(actionTriggered(bool)));
    connect(button, SIGNAL(toggled(bool)), SLOT(synchronizedToggled(bool)));
    connect(button, SIGNAL(clicked(bool)), SLOT(synchronizedTriggered(bool)));
}

void QtActionSynchronizer::synchronize(QAction* action, QAction* otherAction) {
    if(action== NULL || otherAction == NULL ||
       itsSynchedActions_.contains(action, otherAction)) return;
    
    unsynchronize(otherAction);
    
    itsSynchedActions_.insert(action, otherAction);
    otherAction->setChecked(action->isChecked());
    connect(action, SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
    connect(action, SIGNAL(triggered(bool)), SLOT(actionTriggered(bool)));
    connect(otherAction, SIGNAL(toggled(bool)),
            SLOT(synchronizedToggled(bool)));
    connect(otherAction, SIGNAL(triggered(bool)),
            SLOT(synchronizedTriggered(bool)));
}

void QtActionSynchronizer::unsynchronize(QAction* action) {
    if(action == NULL) return;
    
    disconnect(action, SIGNAL(toggled(bool)), this, SLOT(actionToggled(bool)));
    disconnect(action, SIGNAL(triggered(bool)), this,
            SLOT(actionTriggered(bool)));
    
    if(itsSynchedButtons_.contains(action)) {
        foreach(QAbstractButton* b, itsSynchedButtons_.values(action)) {
            disconnect(b, SIGNAL(toggled(bool)), this,
                    SLOT(synchronizedToggled(bool)));
            disconnect(b, SIGNAL(clicked(bool)), this,
                    SLOT(synchronizedTriggered(bool)));
        }
        itsSynchedButtons_.remove(action);
    }
    
    if(itsSynchedActions_.contains(action)) {
        foreach(QAction* a, itsSynchedActions_.values(action)) {
            disconnect(a, SIGNAL(toggled(bool)), this,
                    SLOT(synchronizedToggled(bool)));
            disconnect(a, SIGNAL(triggered(bool)), this,
                    SLOT(synchronizedTriggered(bool)));
        }
        itsSynchedActions_.remove(action);
    }
    
    if(itsSynchedActions_.values().contains(action)) {
        disconnect(action, SIGNAL(toggled(bool)), this,
                SLOT(synchronizedToggled(bool)));
        disconnect(action, SIGNAL(triggered(bool)), this,
                SLOT(synchronizedTriggered(bool)));
        foreach(QAction* a, itsSynchedActions_.keys())
            itsSynchedActions_.remove(a, action);
    }
}

void QtActionSynchronizer::unsynchronize(QAbstractButton* button) {
    if(button == NULL || !itsSynchedButtons_.values().contains(button)) return;
    disconnect(button, SIGNAL(toggled(bool)), this,
            SLOT(synchronizedToggled(bool)));
    disconnect(button, SIGNAL(clicked(bool)), this,
            SLOT(synchronizedTriggered(bool)));
    foreach(QAction* a, itsSynchedButtons_.keys()) {
        itsSynchedButtons_.remove(a, button);
        if(!itsSynchedActions_.contains(a) && !itsSynchedButtons_.contains(a)){
            disconnect(a, SIGNAL(toggled(bool)), this,
                    SLOT(actionToggled(bool)));
            disconnect(a, SIGNAL(triggered(bool)), this,
                    SLOT(actionTriggered(bool)));
        }
    }
}

void QtActionSynchronizer::unsynchronize(QAction* a, QAbstractButton* button) {
    if(a == NULL || button == NULL || !itsSynchedButtons_.contains(a, button))
        return;
    
    disconnect(button, SIGNAL(toggled(bool)), this,
            SLOT(synchronizedToggled(bool)));
    disconnect(button, SIGNAL(clicked(bool)), this,
            SLOT(synchronizedTtriggered(bool)));
    itsSynchedButtons_.remove(a, button);
    
    if(!itsSynchedActions_.contains(a) && !itsSynchedButtons_.contains(a)) {
        disconnect(a, SIGNAL(toggled(bool)), this, SLOT(actionToggled(bool)));
        disconnect(a, SIGNAL(triggered(bool)), this,
                SLOT(actionTriggered(bool)));
    }
}

void QtActionSynchronizer::unsynchronize(QAction* act, QAction* otherAction) {
    if(act == NULL || otherAction == NULL ||
       !itsSynchedActions_.contains(act, otherAction)) return;
    disconnect(otherAction, SIGNAL(toggled(bool)), this,
            SLOT(synchronizedToggled(bool)));
    disconnect(otherAction, SIGNAL(triggered(bool)), this,
            SLOT(synchronizedTriggered(bool)));
    itsSynchedActions_.remove(act, otherAction);
    
    if(!itsSynchedActions_.contains(act) && !itsSynchedButtons_.contains(act)){
        disconnect(act, SIGNAL(toggled(bool)), this,SLOT(actionToggled(bool)));
        disconnect(act, SIGNAL(triggered(bool)), this,
                SLOT(actionTriggered(bool)));
    }
}


// Protected Methods //

void QtActionSynchronizer::actionTriggered_(QAction* action, bool checked) {
    if(action == NULL) return;
    
    // Update synchronized buttons/actions.
    if(itsSynchedButtons_.contains(action))
        foreach(QAbstractButton* b, itsSynchedButtons_.values(action))
            b->setChecked(checked);
    if(itsSynchedActions_.contains(action))
        foreach(QAction* a, itsSynchedActions_.values(action))
            a->setChecked(checked);
}

void QtActionSynchronizer::synchronizedTriggered_(QAction* action,
        QAbstractButton* button, bool checked) {
    if(action == NULL && button == NULL) return;
    
    // Update synchronized action.  If action/button is checkable, update the
    // checked state; otherwise just trigger the action.
    if(action != NULL) {
        foreach(QAction* act, itsSynchedActions_.keys()) {
            if(itsSynchedActions_.contains(act, action)) {
                if(!action->isCheckable() || act->isChecked() != checked)
                    act->trigger();
            }
        }
    } else {
        foreach(QAction* act, itsSynchedButtons_.keys()) {
            if(itsSynchedButtons_.contains(act, button)) {
                if(!button->isCheckable() || act->isChecked() != checked)
                    act->trigger();
            }
        }
    }
}


// Protected Slots //

void QtActionSynchronizer::actionToggled(bool checked) {
    QAction* action = dynamic_cast<QAction*>(sender());
    if(action != NULL) actionTriggered_(action, checked);
}

void QtActionSynchronizer::actionTriggered(bool checked) {
    QAction* action = dynamic_cast<QAction*>(sender());
    
    // If action is checkable, ignore because toggled signal will also be sent
    // and processed.
    if(action != NULL && !action->isCheckable())
        actionTriggered_(action, checked);
}

void QtActionSynchronizer::synchronizedToggled(bool checked) {
    QAction* action = dynamic_cast<QAction*>(sender());
    QAbstractButton* button = action == NULL ?
            dynamic_cast<QAbstractButton*>(sender()) : NULL;
    if(action != NULL || button != NULL)
        synchronizedTriggered_(action, button, checked);
}

void QtActionSynchronizer::synchronizedTriggered(bool checked) {
    QAction* action = dynamic_cast<QAction*>(sender());
    QAbstractButton* button = action == NULL ?
            dynamic_cast<QAbstractButton*>(sender()) : NULL;
    if(action == NULL && button == NULL) return;
    
    // If action/button is checkable, ignore because toggled signal will also
    // be sent and processed.
    if((action != NULL && !action->isCheckable()) || !button->isCheckable())
        synchronizedTriggered_(action, button, checked);
}


///////////////////////////////
// QTACTIONGROUP DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

QtActionGroup::QtActionGroup(QObject* parent) : QtActionSynchronizer(parent),
        isEnabled_(true), itsCheckedAction_(NULL) { }

QtActionGroup::~QtActionGroup() { }


// Public Methods //

QList<QAction*> QtActionGroup::actions() const { return itsActions_; }
QAction* QtActionGroup::checkedAction() const { return itsCheckedAction_; }

QAction* QtActionGroup::addAction(QAction* action) {
    if(action != NULL && !itsActions_.contains(action)) {
        unsynchronize(action);
        
        itsActions_.append(action);
        action->setEnabled(isEnabled_);
        connect(action, SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
        connect(action, SIGNAL(triggered(bool)), SLOT(actionTriggered(bool)));
        connect(action, SIGNAL(hovered()), SLOT(actionHovered()));
        
        if(action->isChecked()) {
            // Replace the currently checked action, if there is one.
            QAction* old = itsCheckedAction_;
            itsCheckedAction_ = action;
            if(old != NULL) old->setChecked(false);
        }
    }
    
    return action;
}

void QtActionGroup::removeAction(QAction* action) {
    if(action != NULL && itsActions_.contains(action)) {
        itsActions_.removeAll(action);
        disconnect(action, SIGNAL(toggled(bool)),
                this, SLOT(actionToggled(bool)));
        disconnect(action, SIGNAL(triggered(bool)),
                this, SLOT(actionTriggered(bool)));
        disconnect(action, SIGNAL(hovered()), this, SLOT(actionHovered()));
        
        unsynchronize(action);
        
        if(itsCheckedAction_ == action) {
            itsCheckedAction_ = NULL;
            emit unchecked();
        }
    }
}

bool QtActionGroup::isEnabled() const { return isEnabled_; }

void QtActionGroup::synchronize(QAction* action, QAbstractButton* button) {
    if(itsActions_.contains(action))
        QtActionSynchronizer::synchronize(action, button);
}

void QtActionGroup::synchronize(QAction* action, QAction* otherAction) {
    if(itsActions_.contains(action))
        QtActionSynchronizer::synchronize(action, otherAction);
}


// Public Slots //

void QtActionGroup::setEnabled(bool enabled) {
    isEnabled_ = enabled;
    foreach(QAction* action, itsActions_)
        action->setEnabled(enabled);
}


// Private Methods //

void QtActionGroup::action_(QAction* action, bool checked) {    
    bool allUnchecked = false;
    if(checked) {
        // Enforce mutual exclusivity if checked.
        if(itsCheckedAction_ != action) {
            QAction* old = itsCheckedAction_;
            itsCheckedAction_ = action;
            if(old != NULL) old->setChecked(false);
        }
        
    } else {
        // Check if all actions are unchecked.
        allUnchecked = true;
        foreach(QAction* a, itsActions_) {
            if(a->isChecked()) {
                allUnchecked = false;
                break;
            }
        }
        if(allUnchecked) itsCheckedAction_ = NULL;
    }
    
    emit triggered(action);
    if(allUnchecked) emit unchecked();
}


// Private Slots //

void QtActionGroup::actionHovered() {
    QAction* action = dynamic_cast<QAction*>(sender());
    if(action != NULL) emit hovered(action);
}

void QtActionGroup::actionTriggered(bool checked) {
    QAction* action = dynamic_cast<QAction*>(sender());
    // Only process if not checkable, because otherwise the toggled signal will
    // also be sent and used.
    if(action != NULL && !action->isCheckable()) action_(action, checked);
}

void QtActionGroup::actionToggled(bool checked) {
    QAction* action = dynamic_cast<QAction*>(sender());
    if(action != NULL) action_(action, checked);
}

}
