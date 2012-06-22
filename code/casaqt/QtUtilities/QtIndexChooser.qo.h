//# QtIndexChooser.qo.h: Simple widget for choosing an index.
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
#ifndef QTINDEXCHOOSER_QO_H_
#define QTINDEXCHOOSER_QO_H_

#include <casaqt/QtUtilities/QtIndexChooser.ui.h>

#include <casa/BasicSL/String.h>

#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// Widget that allows the user to pick an index, with a number of choices on
// how the index can be chosen.  (See Type enum.)
class QtIndexChooser : public QWidget, Ui::IndexChooser {
    Q_OBJECT
    
public:
    // Static //
    
    // The type of the chooser; i.e., how the user picks the index.
    enum Type {
        NAME,    // Display a chooser that shows names associated with indices.
        ROW_COL, // Display a row and column spinner box.
        INDEX    // Display an index spinner box.
    };
    
    
    // Non-Static //
    
    // Constructor which takes the type and an optional parent widget.
    QtIndexChooser(Type type, QWidget* parent = NULL);
    
    // Destructor.
    ~QtIndexChooser();
    
    
    // Returns the currently set index on the widget.
    unsigned int index() const;
    
    // Sets the type to the given, resizing if indicated.
    void setType(Type type, bool resizeToHint = true);
    
    // Only valid if type is NAME.  Sets the name for the given index and makes
    // it available in the GUI.
    void setName(unsigned int index, const String& name);
    
    // Only valid if type is NAME.  Sets the available names to the given.
    // <group>
    void setNames(const vector<String>& names);
    void setNames(const vector<unsigned int>& indices,
            const vector<String>& names);
    void setNames(const map<unsigned int, String>& names);
    void setNames(const map<String, unsigned int>& names);
    // </group>
    
    // Only valid if type is ROW_COL.  Sets the number of rows and columns.
    void setRowsCols(unsigned int nRows, unsigned int nCols);
    
    // Only valid if type is INDEX.  Sets the maximum index (inclusive).
    void setMaxIndex(unsigned int maxIndex);
    
signals:
    // Emitted when the index changes, and contains the new index value.
    void indexChanged(unsigned int newIndex);
    
private:
    // Type.
    Type itsType_;
    
    // Name/Index map.
    QMap<QString, unsigned int> itsNames_;
    
private:
    // Sets up the name chooser for the currently set name/index mappings.
    void setupNameChooser();
    
private slots:    
    // Slot for when the index changes.
    void indexChanged_() { emit indexChanged(index()); }
};

}

#endif /* QTINDEXCHOOSER_QO_H_ */
