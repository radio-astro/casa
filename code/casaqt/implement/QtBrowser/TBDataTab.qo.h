//# TBDataTab.qo.h: Widget used to display table data.
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
#ifndef TBDATATAB_H_
#define TBDATATAB_H_

#include <casaqt/QtBrowser/TBDataTab.ui.h>

#include <vector>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBTable;
class QCloseableWidget;
class TBFilterRuleSequence;
class TBFormat;
class QFontColor;
class TBTableTabs;
class ProgressHelper;
class TBArrayPanel;
class TBData;

// <summary>
// Subclass of QTableWidgetItem that allows for custom sorting.
// <summary>
//
// <synopsis>
// TBDataItem takes a value and a type.  The object saves the value in two
// ways using Qt::DisplayRole and Qt::UserRole.  The value is stored as a
// string using QtDisplayRole, and this is what is actually shown in the
// QTableWidget.  The data stored with Qt::UserRole, however, may be of a
// different type, such as an integer.  This storage is used when sorting
// so that, for example, numbers can be sorted numerically rather than
// lexiographically.  Array types are sorted using the first element for
// one-dimensional arrays and the first dimension for other arrays.
// </synopsis>

class TBDataItem : public QTableWidgetItem {
public:
    // Constructor to take the String value and the type.
    //TBDataItem(String value, String type);
    
    TBDataItem(TBData* data);

    ~TBDataItem();

    
    // Override QTableWidgetItem's less-than operator.  Returns true if the
    // value of this TBDataItem is less than that of the given TBDataItem,
    // false otherwise.  If other is not of type TBDataItem, the behavior is
    // undefined.  Also, since this operator is only used for custom sorting,
    // the other TBDataItem is assumed to have the same type as this one.
    virtual bool operator<(const QTableWidgetItem& other) const;
    
private:
    // Holds the data type.
    String type;
    
    // Is true if the value stored for Qt::UserRole is an array dimension,
    // false otherwise.
    bool arrayDimension;
    
    TBData* tdata;

    
    // Set the Qt::UserRole and, if text is true, the Qt::DisplayRole values.
    void set(String value, bool text);
};

// <summary>
// Widget used to display table data.
// <summary>
//
// <synopsis>
// TBDataTab is the tab responsible for displaying table data in a
// QTableWidget.  It also provides functionality for changing page and number
// of rows loaded, catching and passing on edit events, and handling
// double-click events.
// </synopsis>

class TBDataTab : public QWidget, Ui::DataTab {
    Q_OBJECT

public:
    // Constructor that takes the parent TBTableTabs.
    TBDataTab(TBTableTabs* tt);

    ~TBDataTab();

    
    // Returns the QTableWidget that displays the data.
    QTableWidget* getTableWidget();
    
    // Returns the QBrush that is used to draw the background of any selected
    // cells in the QTableWidget.
    QBrush& getSelectedCellBackground();

    // Retruns the QBrush that is used to draw the background of any unselected
    // cells in the QTableWidget.
    QBrush& getUnselectedCellBackground();
    
    // Returns the current row filter, or NULL if there is none.
    TBFilterRuleSequence* getFilter();
    
    // Sets the current filter to the given rules, but does NOT apply it.
    void setFilter(TBFilterRuleSequence* rule);

    // Returns the current format for the field at the given index, or NULL if
    // there is none.
    TBFormat* formatAt(int index);
    
    // Returns the current sort on the displayed data.  Each pair in the vector
    // is the name of the field with a bool indicating whether the sort is
    // ascending or not.
    vector<pair<String, bool> >* getSortFields();
    
    
    // Updates the QTableWidget with new data that has been loaded into the
    // table backend.  If a ProgressHelper is provided, it will be updated
    // periodically with progress information.
    void updateTable(ProgressHelper* pp = NULL);

    // Sets the given cell in the QTableWidget to the new value, but does NOT
    // update the backend.  If format is true, then any existing format will
    // be applied to the new value.
    void setData(int row, int col, TBData* newVal, bool format = true);

    // Loads the given page into the table backend, which then updates the
    // display.  If the page is valid, the result of TBTableTabs::loadRows()
    // is returned; otherwise false is returned.
    bool loadPage(int p);

    // Refreshes the table backend using TBTable::refresh(), then updates the
    // displayed data using setData().
    void refresh(int row, int col, bool format = true);

    // Sorts the displayed data with the given order.  Each pair in the vector
    // is the name of the field with a bool indicating whether the sort is
    // ascending or not.
    void sortBy(vector<pair<String, bool> >& s);
    
    // Sets the sort indicator on the QTableWidget for the given column with an
    // ascending arrow if asc is true or a descending arrow otherwise.
    void setSortIndicator(int index, bool asc);

    // Clears the sort indicator on the QTableWidget.
    void clearSortIndicator();
    
    // Selects and highlights the given row in the table widget.
    void highlight(int row);
    
    // Returns the visible index for the row with the given logical index.
    // Rows may have differing visible and logical indices when a sort has
    // been entered.
    int visibleIndex(int logicalIndex);
    
    // Returns the logical index for the row with the given visible index.
    // Rows may have differing visible and logical indices when a sort has
    // been entered.
    int logicalIndex(int visibleIndex);
    
    // Hides (or shows) the column with the given index.  Whether the column
    // is shown or hidden depends on the "hidden" parameter.
    void hideColumn(int index, bool hidden);
    
    // Opens a format dialog for the field with the given index.  Applies the
    // format that the user enters.
    void formatField(int index);

public slots:
    // Shows the given widget in the side panel.  isArray should be true if
    // the widget is a TBViewArray, false otherwise.
    void showWidgetInSplitter(QWidget* widget, bool isArray = false);

    // Clears whatever widget (if any) is currently being displayed in the side
    // panel.
    void clearWidgetInSplitter();

    // Applies the given format to the field at the given index.
    void applyFormat(int index, TBFormat* format);

    // Clears the format on the field at the given index.
    void clearFormat(int index);

    // Clears all formats on all fields.
    void clearAllFormats();

signals:
    // This signal is emitted when the user has edited a cell.  The parameter
    // describe which cell was edited and its new value.
    void dataChanged(int row, int col, String newVal);

    // This signal is emitted when the side panel is closed.  The QWidget
    // points to the widget that was just closed.
    void rightWidgetClosed(QWidget* which);

    // This signal is emitted when the user right-clicks on an index and
    // selects the "Follow subtable index reference" command.  The String
    // indicates which subtable was chosen, and the index indicates the chosen
    // index which is the value of the cell that was right-clicked on.
    void followReferenceRequested(String subtable, int index);
    
    // This signal is emitted when a sort is applied to the table.
    void sortEntered();
    
    // This signal is emitted when a sort is cleared from the table.
    void sortCleared();
    
    // This signal is emitted when a column is shown or hidden.
    void columnHidden(int index, bool hidden);

protected:
    // Catches the right-click event; if the right click is on a table cell and
    // the displayed data could be an index, show the subtable index reference
    // menu.  Also show action for copying to the clipboard.
    void contextMenuEvent(QContextMenuEvent* event);
    
private:
    // Useful pointers to table backend.
    // <group>
    TBTableTabs* ttabs;
    TBTable* table;
    // </group>
    
    // The currently loaded page.
    int page;

    // How many rows per page are loaded at once.
    int loadRows;

    // Side panel.
    QCloseableWidget* rightWidget;

    // List of cells that are highlighted (the row and column of the currently
    // selected cell).
    vector<QTableWidgetItem*> highlightedCells;

    // Brushes used to draw the backgrounds of unselected and selected cells in
    // the QTableWidget, respectively.
    // <group>
    QBrush unselectedBackground;
    QBrush selectedBackground;
    // </group>
    
    // Current row filter, or NULL if there is none.
    TBFilterRuleSequence* filter;

    // Current field formats.
    vector<TBFormat*> formats;

    // "Cleared" format for cell text.
    QFontColor* defaultFormat;

    // Indicates whether the side panel is currently open with an array or not.
    bool arrayOpened;

    // Points to the current array panel, or NULL if there is none.
    TBArrayPanel* arrayPanel;

    // Keeps track of the un-sorted order of the rows.
    // Doing rowItems[logicalRow]->row() returns the visualRow.
    vector<QTableWidgetItem*> rowItems;

    // Keeps track of the un-sorted order of the rows.
    // Doing rowIndices[visualRow] returns the logicalRow.
    vector<int> rowIndices;

    // The current sort order.
    vector<pair<String, bool> > currSort;
    
    // Used for handling right-clicks on headers.  Keeps track of the last
    // right-clicked header column number.
    int clickedHeader;

    
    // Update widgets that keep track of what page the browser is on.
    void updatePageTrackers();

    // Sorts on the given row.  If asc is true, sorts in ascending order,
    // otherwise sorts in descending order.
    void sort(int col, bool asc);
    
    // Applies the current filter to the displayed rows.
    void applyFilter();

private slots:
    // Slot for page back button.  Goes back a page.
    void pageBack();

    // Slot for page forward button.  Goes forward a page.
    void pageForward();
    
    // Slot to go to first page.
    void pageFirst();
    
    // Slot to go to last page.
    void pageLast();

    // Slot for page go button.  Goes to the page specified in the page line
    // edit.
    void pageGo();

    // Slot for QTableWidget::cellChanged().  Catches changes from the
    // QTableWidget and emits signals as needed.
    void notifyDataChanged(int row, int col);

    // Slot for QTableWidget::currentCellChanged().  Updates the display to
    // highlight the row and column of the selected cell.
    void cellClicked(int row, int col);

    // Slot for the menu command chosen from the reference menu shown on a
    // right click.  Catches which subtable the user followed a reference and
    // emits signals as needed.
    void referenceMenuClicked(QAction* which);

    // Slot for QTableWidget::cellDoubleClicked().  Catches a double-click
    // event on a cell in the QTableWidget.  If the value is a table type,
    // opens the indicated table in a new tab.  If the value is an array type,
    // opens the array in the side panel.  Otherwise, if the table is currently
    // editable, enters into editing mode on that cell.
    void doubleClicked(int row, int col);
    
    // Displays the tooltip for the last clicked header in a dialog.
    void displayFieldInfo();
    
    // Displays statistics (min, max, mean) for the last clicked header in a
    // dialog.
    void displayFieldStatistics();
    
    // Hides the column that was just right-clicked.
    void hideColumn();
    
    // Formats the field that was just right-clicked.
    void formatField();
    
    // Slot for when a column header is moved.  Enables the "restore columns"
    // button.
    void headerMoved();
    
    // Slot for the restore columns button.  Moves all columns back to their
    // original locations.
    void restoreColumns();

    // Slot for the resize headers button.  Resizes the row and column headers
    // to fit displayed contents.  See QTableWidget::resizeRowsToContents() and
    // QTableWidget::resizeColumnsToContents().
    void resizeHeaders();

    // Slot for QHeaderView::sectionClicked().  Catches a sorting click event
    // on a column header and sorts accordingly.
    void headerClicked(int index);

    // Slot for the clear sort button.  Clears the sort on the table by moving
    // the rows back to their original order.
    void clearSort();

    // Slot for copying the currently selected text into the system clipboard.
    void copyData();
};

}

#endif /* TBDATATAB_H_ */
