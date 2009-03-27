//# TBViewArray.qo.h: Widget for viewing array data in TBArray format.
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
#ifndef TBVIEWARRAY_H_
#define TBVIEWARRAY_H_

#include <casaqt/QtBrowser/TBViewArray.ui.h>
#include <casaqt/QtBrowser/TBArrayPanel.ui.h>
#include <casaqt/QtBrowser/TBBrowser.qo.h>

#include <vector>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBTableTabs;
class TBArray;
class TBTable;
class TBSlicer;
class TBFormat;
class QFontColor;
class QCloseableWidget;
class TBArrayData;
class TBData;
class TBDataRecord;

// <summary>
// Widget for viewing array data in TBArray format.
// </summary>
//
// <synopsis>
// A TBViewArray displays potentially multi-dimensional array data in a
// QTableWidget.  If the array has dimensionality greater than two, a
// TBSlicer is used to control the array slice.
// </synopsis>

class TBViewArray : public QWidget, Ui::ViewArray {
    Q_OBJECT

public:    
    // Constructor which takes the table parent, the "indices" where this array
    // is located, the array to view, the location in the table (if applicable,
    // and whether this array should be editable or not.  The top of the
    // array view will have a label that says "[table name][first, second] =
    // [type] array of size [size]."  For keyword arrays, row and col are
    // irrelevant and editable should be false.
    TBViewArray(TBTableTabs* tt, String first, String second, TBArrayData* arr,
                int row, int col, bool editable);

    ~TBViewArray();
    
    
    // Returns the array that is being displayed.
    TBArrayData* getArrayData();    
    
    // Sets whether the arrays being viewed should release their data when
    // closed or not.
    void setShouldRelease(bool b);
        

    // Returns the data at the given coordinates, or NULL if the coordinates
    // are invalid.    
    TBData* dataAt(vector<int> d);

    // Sets the data at the given coordinates to the given value WITHOUT
    // updating the table backend.  If format is true, then any current
    // format is applied to the new value.    
    void setDataAt(vector<int> d, TBData& newVal, bool format = true);

    // Applies the given format to the array cells.
    void applyFormat(TBFormat* f);

    // Clears the current format from the array cells and applies the given
    // QFontColor (which should be the default table cell font and color).
    void clearFormat(QFontColor* f);
    
private:
    // Table backend.
    // <group>
    TBTableTabs* tTabs;
    TBTable* t;
    // </group>
    
    // Array being displayed.    
    TBArrayData* array;

    // Flag to indicate whether GUI-generated events are "genuine."
    bool update;
    
    // Slicer for arrays with dimensionality greater than two.
    TBSlicer* slicer;
    
    // Current slice for arrays with dimensionality greater than two.
    vector<int> currentSlice;
    
    // Indicates whether this array is allowed to be edited.  Data arrays
    // should be true while keyword arrays should be false.
    bool editable;
    
    // Background for unselected cells.
    QBrush unselectedBackground;
    
    // Background for selected cells.
    QBrush selectedBackground;
    
    // List of cells that are on the same row or column as the currently
    // selected cell.
    vector<QTableWidgetItem*> selectedCells;
    
    // Current format.
    TBFormat* format;
    
    // Indicates whether the underlying array data should be released when
    // the view is closed or not.
    bool shouldRelease;
    
    // Row of data array.
    int row;
    
    // Column of data array.
    int col;

    
    // Sets up the GUI components with the given parameters for the label.
    void setup(String first, String second);

    // Returns the array-relevant coordinates corresponding to the given
    // indices. 
    vector<int> currentCell(int row, int col);
    
    // Relabels the table headers to be 0- rather than 1-based.
    void relabelHeaders();

private slots:
    // Slot for when the user changes data in the array.  If the edit is
    // valid, a TBEditArrayDataAction is generated and sent to the browser
    // for execution.
    void dataChanged(int row, int col);

    // Slot for when the slicer changes (for arrays with dimensionality
    // greater than two).
    void sliceChanged(vector<int> newSlice);

    // Slot for when an array cell is clicked.  Updates cells in the same
    // row or column with a "selected" background.
    void cellClicked(int row, int col);

    // Slot for when an array cell is double-clicked.  If the array is
    // editable and the table is currently in editing mode, the user is then
    // allowed to edit the cell data.
    void cellDoubleClicked(int row, int col);
};

// <summary>
// Panel that can hold multiple TBViewArray widgets.
// </summary>
//
// <synopsis>
// TBArrayPanel is the widget that is actually shown in the side panel and
// consists of one or more TBViewArray widgets.  When the user double-clicks
// on another array, it is added to the TBArrayPanel.  When the panel is
// closed, it closes all the TBViewArray widgets as well.
// </synopsis>

class TBArrayPanel : public QWidget, Ui::ArrayPanel {
    Q_OBJECT
    
public:
    // Constructor that takes the table backend.
    TBArrayPanel(TBTableTabs* tt);

    ~TBArrayPanel();

    
    // Adds the given TBViewArray widget to this panel and returns whether it
    // succeeded or not.  If the given array is already being displayed (see
    // TBArray::sameLocationAs(), false is returned.
    bool addArray(TBViewArray* array, int colIndex);
    
    // Calls setShouldRelease on all TBViewArrays in this panel.
    void setShouldRelease(bool b);
    
    // Applies the given format to any TBViewArray with the given index.
    void applyFormat(TBFormat* format, int colIndex);

public slots:
    // Removes any actions in the browser that are associated with any
    // of the arrays in this panel.
    void removeActionsAssociatedWithArrays();

signals:
    // This signal is emitted when the user presses "close" on all the
    // currently opened arrays in this panel.  The caller should then close
    // the panel itself.
    void allArraysClosed();
    
private:
    // Table backend.
    TBTableTabs* ttabs;
    
    // List of opened arrays.
    vector<TBViewArray*> arrays;
    
    // List of wrapper widgets.
    vector<QCloseableWidget*> widgets;
    
    // Array indices.
    vector<int> indices;
    
    // Splitter to hold the opened arrays.
    QSplitter splitter;
    
    
    // Removes any actions in the browser that are associate with the given
    // array in this panel.
    void removeActionsAssociatedWithArray(TBViewArray* array);
    
private slots:
    // Slot for when the user closes an individual array.
    void closeRequested(QWidget* widget);
};


// <summary>
// Widget for viewing record data.
// </summary>
//
// <synopsis>
// A TBViewRecord displays data in a TBDataRecord format, which uses an
// underlying Record object.  The record is displayed in a table, and the
// values can also be another table (for arrays or sub-records).
// </synopsis>

class TBViewRecord : public QWidget, Ui::ViewArray {
    Q_OBJECT

public:
    // Constructor which takes the table parent, the record to display, and the
    // "indices" to display in the label.
    TBViewRecord(TBTableTabs* tt, TBDataRecord* r, String first,
                 String second = "");
    
    ~TBViewRecord();
    
private:
    // Table parent.
    TBTableTabs* tt;
    
    // Displayed record.
    Record& record;
    
    // Fills the given table with the given parameters.
    void fill(QTableWidget& table, Record& r, String first, String second);
};

}

#endif /* TBVIEWARRAY_H_ */
