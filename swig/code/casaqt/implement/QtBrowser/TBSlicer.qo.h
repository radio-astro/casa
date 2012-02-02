//# TBSlicer.qo.h: Widget to display and change the current array slice.
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
#ifndef TBSLICER_H_
#define TBSLICER_H_

#include <casaqt/QtBrowser/TBSlicer.ui.h>

#include <vector>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// <summary>
// Widget to display and change the current array slice.
// </summary>
//
// <synopsis>
// A TBSlicer has two parts: the bottom part allows the user to chooser which
// dimension is viewed along the two axes, and the top part chooses which
// "slice" along those two axes to view.  When the user changes the slice,
// a signal is emitted; the parent/caller is responsible for connecting and
// processing the signal.
// </synopsis>

class TBSlicer : public QWidget, Ui_Slicer {
    Q_OBJECT

public:
    // Constructor that takes the shape of the array.
    TBSlicer(vector<int> d);

    ~TBSlicer();

signals:
    // This signal is emitted when the user changes the slice.  The slice
    // parameter contains non-negative values along all dimensions EXCEPT
    // for the two dimensions that are mapped to the X- and Y-axes.  The
    // dimension mapped to the X-axis has the value
	// TBConstants::SLICER_ROW_AXIS in the vector while the dimension mapped
	// to the Y-axis has the value TBConstants::SLICER_COL_AXIS in the vector.
	// For example, if the array was 4x4x4, a slice of [SLICER_ROW_AXIS 1
	// SLICER_COL_AXIS] would mean to display the first dimension along the
	// X-axis and the third dimension along the Y-axis and to use 1 as the
	// index for the second dimension.
    void sliceChanged(vector<int> slice);
    
private:
    // Current spinners.
    vector<QSpinBox*> spinners;

    // Current slice values.
    vector<int> values;

    // Holds the old row index.
    int oldR;

    // Holds the old column index.
    int oldC;

    // Flag to indicate whether GUI-generated events are "genuine."
    bool shouldEmit;

    
    // Collects the slice and emits the sliceChanged() signal.
    void emitSliceChanged();

private slots:
    // Slot for when one of the spinners changes values.  Updates the current
	// slice and calls emitSliceChanged() if the slice has changed.
    void valueChanged();

    // Slot for when the row axis dimension is changed.  Updates the slicer
    // accordingly.
    void rowAxisChanged(int newRow);

    // Slot for when the column axis dimension is changed.  Updates the slicer
    // accordingly.
    void colAxisChanged(int newCol);
};

}

#endif /* TBSLICER_H_ */
