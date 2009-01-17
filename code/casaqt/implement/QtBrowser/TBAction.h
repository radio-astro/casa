//# TBAction.h: Edit actions that can be done, undone, and redone.
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
#ifndef TBACTION_H_
#define TBACTION_H_

#include <casaqt/QtBrowser/TBConstants.h>

#include <casa/BasicSL/String.h>

#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBTableTabs;
class TBViewArray;
class TBData;

// <summary>
// Abstract parent class of any action that can be performed.
// <summary>
//
// <synopsis>
// Any editing change to the underlying table should be encapsulated in a
// TBAction.  A TBAction subclass needs to be able to perform the designated
// action as well as knowing how to undo it.  The specific implementation of
// performing the action is left up to the subclasses.
// </synopsis>

class TBAction {
public:
    // Constructor that takes a pointer to the origin object.  The origin is
    // used to, for example, remove actions associated with objects that have
    // been closed or otherwise made unavailable.
    TBAction(void* origin);

    virtual ~TBAction();

    
    // Returns a pointer to the origin object.
    void* getOrigin();

    
    // doAction() must be implemented by any subclass.  This method performs
    // the action, updating both the backend and the GUI as necessary.
    virtual Result doAction() = 0;

    // undoAction() must be implemented by any subclass.  This method undoes
    // the action, updating both the backend and the GUI as necessary.
    virtual Result undoAction() = 0;

    // name() must be implemented by any subclass.  This method returns the
    // name of the action.  The name should be human-readable and does not have
    // to be any specific format.
    virtual String name() = 0;

    // isAssociatedWith() must be implemented by any subclass.  This method
    // returns true if this action is associated with the given origin object,
    // false otherwise.  If an action is associated with an object that is
    // then closed (like a table or an array) then the action may be removed
    // from the performed/undone queues.
    virtual bool isAssociatedWith(void* origin) = 0;

protected:
    // Pointer to origin object.
    void* origin;
};

// <summary>
// Contains an list of performed actions and a list of undone actions.
// <summary>
//
// <synopsis>
// A TBActionList keep track of performed and undone actions and provides
// methods to add and move actions between the two lists.  The lists have a
// maximum length defined by TBConstants::MAX_ACTION_BUFFER; once this limit
// has been reached, old actions are discarded.
// </synopsis>

class TBActionList {
public:
    // Default Constructor to initialize the empty lists.
    TBActionList();

    ~TBActionList();

    
    // Returns true if the performed list is empty, false otherwise.
    bool isEmpty();

    // Returns true if the undone list is empty, false otherwise.
    bool undoneIsEmpty();

    // Returns the size of the performed list.
    int size();

    // Returns the size of the undone list.
    int undoneSize();

    // Returns the name() value of the last performed action, or blank if
    // there is none.
    String lastActionName();

    // Returns the name() value of the last undone action, or blank if there
    // is none.
    String lastUndoneActionName();

    // Returns the performed action at the designated index, or NULL if the
    // index is invalid.
    TBAction* at(unsigned int i);

    // Returns the undone action at the designated index, or NULL if the index
    // is invalid.
    TBAction* undoneAt(unsigned int i);
    
    
    // Adds the given TBAction to the performed list and calls the action's
    // doAction() method.
    Result doAction(TBAction* action);

    // Moves the latest performed action to the undone list and calls the
    // action's undoAction() method.
    Result undoAction();

    // Moves the latest undone action to the performed list and calls the
    // action's doAction() method.
    Result redoAction();

    // Removes the given action from the performed list, but does not delete
    // it.  Returns true if the remove was successful, false otherwise.
    bool remove(TBAction* a);

    // Removes the given action from the undone list, but does not delete it.
    // Returns true if the remove was successful, false otherwise.
    bool removeUndone(TBAction* a);
    
private:
    // Performed actions list
    vector<TBAction*> actions;

    // Undone actions list
    vector<TBAction*> undone;
};

/* Specific Actions */

// <summary>
// TBAction for when non-array data in the table is edited.
// <summary>
//
// <synopsis>
// A TBEditDataAction keeps track of the table, row, column, new value, and old
// value.  When this action is performed, the underlying table is updated to
// the new value at the given row and column; when this action is undone, the
// underlying table is updated to the old value.
// </synopsis>

class TBEditDataAction : public TBAction {
public:    
    TBEditDataAction(TBTableTabs* tt, int row, int col, TBData* newVal);

    virtual ~TBEditDataAction();

    
    // Implements TBAction::doAction().
    // Updates the table by calling TBTable::editData() followed by a
    // TBDataTab::setData() if the edit is successful.  Returns the result
    // of the TBTable::editData() call.
    Result doAction();

    // Implements TBAction::undoAction().
    // Updates the table by calling TBTable::editData() followed by a
    // TBDataTab::setData() if the edit is successful.  Returns the result
    // of the TBTable::editData() call.
    Result undoAction();

    // Implements TBAction::name().
    // Returns "edit [table name]([row],[col])".
    String name();

    // Implements TBAction::isAssociatedWith().
    // Returns true if o is equal to the TBTableTabs object given in the
    // constructor, false otherwise.
    bool isAssociatedWith(void* o);
    
private:
    // Origin table.
    TBTableTabs* tt;

    // Row of the edit data.
    int row;

    // Column of the edit data.
    int col;
    
    // New value.
    TBData* newVal;

    // Old value.
    TBData* oldVal;
    
    // This action's name.
    String actionName;

    
    // Updates the underlying table with the given value.
    Result update(TBData* val);
};

// <summary>
// TBAction for when array data in the table is edited.
// <summary>
//
// <synopsis>
// A TBEditArrayDataAction keeps track of the table, row, column, array
// coordinates, new value, and old value.  When this action is performed, the
// underlying table is updated to the new value at the given row, column, and
// coordinates; when this action is undone, the underlying table is updated to
// the old value.
// </synopsis>

class TBEditArrayDataAction : public TBAction {
public:
    TBEditArrayDataAction(TBTableTabs* tt, TBViewArray* array, int row,
        int col, vector<int> coord, TBData* newVal);

    virtual ~TBEditArrayDataAction();

    
    // Implements TBAction::doAction().
    // Updates the table by calling TBTable::editArrayData() followed by a
    // TBViewArray::setDataAt() if the edit is successful.  Returns the result
    // of the TBTable::editArrayData().
    Result doAction();

    // Implements TBAction::undoAction().
    // Updates the table by calling TBTable::editArrayData() followed by a
    // TBViewArray::setDataAt() if the edit is successful.  Returns the result
    // of the TBTable::editArrayData().
    Result undoAction();

    // Implements TBAction::name().
    // Returns "edit [table name]([row],[col])[coords]".
    String name();

    // Implements TBAction::isAssociatedWith().
    // Returns true if o is the TBTableTabs object or the TBViewArray object
    // given in the constructor, false otherwise.
    bool isAssociatedWith(void* o);

private:
    // Origin table.
    TBTableTabs* tt;

    // Row of the edit data.
    int row;

    // Column of the edit data.
    int col;

    // Array coordinates of the edit data.
    vector<int> coords;;
    
    // New value.
    TBData* newVal;
    
    // Old value.
    TBData* oldVal;

    // This action's name.
    String actionName;

    // Indicates whether the array is one-dimensional or not.
    bool oneDim;

    
    // Updates the underlying table with the given value.
    Result update(TBData* val);
};

}

#endif /* TBACTION_H_ */
