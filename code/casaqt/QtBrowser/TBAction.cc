//# TBAction.cc: Edit actions that can be done, undone, and redone.
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
#include <casaqt/QtBrowser/TBAction.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBTableTabs.qo.h>
#include <casaqt/QtBrowser/TBViewArray.qo.h>
#include <casaqt/QtBrowser/TBDataTab.qo.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/TBData.h>

namespace casa {

///////////////////////////
// TBACTION DEFINITIONS //
///////////////////////////

// Constructors/Destructors //

TBAction::TBAction(void* o) : origin(o) { }

TBAction::~TBAction() { }

// Accessors/Mutators //

void* TBAction::getOrigin() {
    return origin;
}

///////////////////////////////
// TBACTIONLIST DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

TBActionList::TBActionList(): actions(), undone() { }

TBActionList::~TBActionList() {
    for(unsigned int i = 0; i < actions.size(); i++)
        delete actions.at(i);
        
    for(unsigned int i = 0; i < undone.size(); i++)
        delete undone.at(i);
}

// Accessors/Mutators //

bool TBActionList::isEmpty() { return actions.size() == 0; }

bool TBActionList::undoneIsEmpty() { return undone.size() == 0; }

int TBActionList::size() { return actions.size(); }

int TBActionList::undoneSize() { return undone.size(); }

String TBActionList::lastActionName() {
    if(actions.size() > 0) return (actions.at(actions.size() - 1))->name();
    else return "";
}

String TBActionList::lastUndoneActionName() {
    if(undone.size() > 0) return (undone.at(undone.size() - 1))->name();
    else return "";
}

TBAction* TBActionList::at(unsigned int i) {
    if(i < actions.size()) {
        return actions.at(i);
    } else return NULL;
}

TBAction* TBActionList::undoneAt(unsigned int i) {
    if(i < undone.size()) {
        return undone.at(i);
    } else return NULL;
}

// Public Methods //

Result TBActionList::doAction(TBAction* action) {
    Result r = action->doAction();
    if(r.valid) {
        actions.push_back(action);
        
        // Check the size for old actions to delete
        if(actions.size() > TBConstants::MAX_ACTION_BUFFER) {
            delete actions.at(0);
            actions.erase(actions.begin());
        }

        if(undone.size() > 0) {
            // Clear undone actions
            for(unsigned int i = 0; i < undone.size(); i++)
                delete undone.at(i);
            undone.clear();
        }
    }
    return r;
}

Result TBActionList::undoAction() {
    if(actions.size() > 0) {
        TBAction* a = actions.at(actions.size() - 1);
        Result r = a->undoAction();
        if(r.valid) {
            actions.pop_back();
            undone.push_back(a);
            
            // Check the size for old actions to delete
            if(undone.size() > TBConstants::MAX_ACTION_BUFFER) {
                delete undone.at(0);
                undone.erase(undone.begin());
            }
        }
        return r;
    }
    return Result("No actions to undo.", false);
}

Result TBActionList::redoAction() {
    if(undone.size() > 0) {
        TBAction* a = undone.at(undone.size() - 1);
        Result r = a->doAction();
        if(r.valid) {
            undone.pop_back();
            actions.push_back(a);
            
            // Check the size for old actions to delete
            if(actions.size() > TBConstants::MAX_ACTION_BUFFER) {
                delete actions.at(0);
                actions.erase(actions.begin());
            }
        }
        return r;
    }
    return Result("No actions to redo.", false);
}

bool TBActionList::remove(TBAction* a) {
    for(unsigned int i = 0; i < actions.size(); i++) {
        if(actions.at(i) == a) {
            actions.erase(actions.begin() + i);
            return true;
        }
    }
    return false;
}

bool TBActionList::removeUndone(TBAction* a) {
    for(unsigned int i = 0; i < undone.size(); i++) {
        if(undone.at(i) == a) {
            undone.erase(undone.begin() + i);
            return true;
        }
    }
    return false;
}


///////////////////////////////////
// TBEDITDATAACTION DEFINITIONS //
///////////////////////////////////

// Constructors/Destructors //

TBEditDataAction::TBEditDataAction(TBTableTabs* t, int r, int c, TBData* d) :
                        TBAction(t), tt(t), row(r), col(c),
                        newVal(d) {
    oldVal = TBData::create(*tt->getTable()->dataAt(r, c));
    actionName = String("edit " + tt->getName() + "(" + TBConstants::itoa(r) +
                        "," + TBConstants::itoa(c) + ")");
}

TBEditDataAction::~TBEditDataAction() {
    delete oldVal;
    delete newVal;
}

// Public Methods //

Result TBEditDataAction::doAction() { return update(newVal); }

Result TBEditDataAction::undoAction() { return update(oldVal); }

String TBEditDataAction::name() { return actionName; }

bool TBEditDataAction::isAssociatedWith(void* o) { return origin == o; }

// Private Methods //

Result TBEditDataAction::update(TBData* val) {
    Result r = tt->getTable()->editData(row, col, val);
    if(r.valid) {
        tt->getDataTab()->setData(row, col, val);
    }
    return r;
    
}


////////////////////////////////////////
// TBEDITARRAYDATAACTION DEFINITIONS //
////////////////////////////////////////

// Constructors/Destructors //
            
TBEditArrayDataAction::TBEditArrayDataAction(TBTableTabs* t, TBViewArray* a,
            int r, int c, vector<int> d, TBData* nv) :
            TBAction(a), tt(t), row(r), col(c), coords(d), newVal(nv),
            oneDim(a->getArrayData()->isOneDimensional()) {
    oldVal = TBData::create(*a->getArrayData()->dataAt(d));
    
    actionName = String("edit " + tt->getName() + "(" + TBConstants::itoa(r) +
                        "," + TBConstants::itoa(c) + ")[");
    for(unsigned int i = 0; i < d.size(); i++) {
        actionName += TBConstants::itoa(d.at(i));
        if(i < d.size() - 1) actionName += " ";
    }
    actionName += "]";
}

TBEditArrayDataAction::~TBEditArrayDataAction() {
    delete newVal;
    delete oldVal;
}

// Public Methods //

Result TBEditArrayDataAction::doAction() { return update(newVal); }

Result TBEditArrayDataAction::undoAction() { return update(oldVal); }

String TBEditArrayDataAction::name() { return actionName; }

bool TBEditArrayDataAction::isAssociatedWith(void* o) {
    return origin == o || tt == o;
}

// Private Methods //

Result TBEditArrayDataAction::update(TBData* val) {
    Result r = tt->getTable()->editArrayData(row, col, coords, val, oneDim);
    if(r.valid) {
        ((TBViewArray*)origin)->setDataAt(coords, *val);
        if(oneDim) tt->getDataTab()->refresh(row, col, true);
    }
    return r;
}

}
