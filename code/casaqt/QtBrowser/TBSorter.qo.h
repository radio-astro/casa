//# TBSorter.qo.h: Widget to configure a multi-field sort ordering.
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
#ifndef TBSORTER_H_
#define TBSORTER_H_

#include <casaqt/QtBrowser/TBSorter.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// <summary>
// Widget to configure a multi-field sort ordering.
// </summary>
//
// <synopsis>
// TBSorter allows the user to create an ordered list of fields for sorting.
// Each field in the list also has a checkbox to indicate whether the sort
// on that field should be ascending or not.  Once a sort order has been
// entered a signal is emitted; it is the parent's/caller's responsibility
// to connect the signal and implement the sort.
// </synopsis>

class TBSorter : public QDialog, Ui::Sorter {
    Q_OBJECT

public:
    // Constructor that takes the list of fields, an optional current sort
    // order to display, and an optional parent.  If parent is NULL, the
    // widget is displayed as a dialog; otherwise it is displayed in the
    // parent.
    TBSorter(vector<String>& cols, vector<pair<String, bool> >* sort = NULL,
             QWidget* parent = NULL);

    ~TBSorter();

signals:
    // sortEntered is emitted when the "Sort" button is clicked.  The sort
    // parameter contains the ordered list of fields and ascending bools.
    void sortEntered(vector<pair<String, bool> >& sort);

private:
    // The sortable fields.
    vector<String> cols;

    
    // Adds the specified field from the field list to the sort list.
    void add(int row, bool asc = true);

    // Removes the specified field from the sort list.
    void remove(int row);

    // Swaps the two indicated rows in the sort list.
    void swapRows(int r1, int r2);

private slots:
    // Slot for the "Add" button that adds the selected row in the field list
    // to the bottom of the sort list.
    void add();

    // Slot for the "Add All" button that adds all rows in the field list to
    // the bottom of the sort list.
    void addAll();

    // Slot for the "Remove" button that removes the selected row from the
    // sort list and returns it to its previous position in the field list.
    void remove();

    // Slot for the "Remove All" button that removes all the rows from the
    // sort list and returns them to the field list.
    void removeAll();

    // Slot for when the currently selected index in the sort list is changed.
    void tableIndexChanged(int row);

    // Slot for the "Move Up" button that moves the selected row in the sort
    // list up one position.
    void moveUp();

    // Slot for the "Move Down" button that moves the selected row in the sort
    // list down one position.
    void moveDown();

    // Slot for the button to accept the current sort order and emit the
    // sortEntered() signal.
    void acceptButton();
};

}

#endif /* TBSORTER_H_ */
