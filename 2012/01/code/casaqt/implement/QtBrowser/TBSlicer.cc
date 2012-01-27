//# TBSlicer.cc: Widget to display and change the current array slice.
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
#include <casaqt/QtBrowser/TBSlicer.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>

namespace casa {

////////////////////////////
// TBSPLICER DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

TBSlicer::TBSlicer(vector<int> d): QWidget(), spinners(), values(),
                                          oldR(-1), oldC(-1){
    setupUi(this);
    hboxLayout->removeWidget(spinBox);
    spinBox->close();
    
    // Add a spinner for each dimension followed by a stretcher
    for(unsigned int i = 0; i < d.size(); i++) {
        QSpinBox* spinner = new QSpinBox(groupBox);
        spinner->setMaximum(d.at(i) - 1);
        hboxLayout->addWidget(spinner);
        spinners.push_back(spinner);
        values.push_back(0);
    }
    hboxLayout->addStretch();

    // Set up slicer
    shouldEmit = false;
    rowsSpinBox->setMaximum(d.size() - 1);
    colsSpinBox->setMaximum(d.size() - 1);
    connect(rowsSpinBox, SIGNAL(valueChanged(int)),
    		this, SLOT(rowAxisChanged(int)));
    connect(colsSpinBox, SIGNAL(valueChanged(int)),
    		this, SLOT(colAxisChanged(int)));
    rowsSpinBox->setValue(d.size() - 2);
    colsSpinBox->setValue(d.size() - 1);    
    shouldEmit = true;
    
    // Connect spinners
    for(unsigned int i = 0; i < spinners.size(); i++)
        connect(spinners.at(i), SIGNAL(valueChanged(int)),
        		this, SLOT(valueChanged()));
}

TBSlicer::~TBSlicer() { }

// Private Methods //

void TBSlicer::emitSliceChanged() {
    if(shouldEmit && oldR >= 0 && oldC >= 0) {
        vector<int> v(values);

        v[oldR] = TBConstants::SLICER_ROW_AXIS;
        v[oldC] = TBConstants::SLICER_COL_AXIS;
        
        emit sliceChanged(v);
    }
}

// Private Slots //

void TBSlicer::valueChanged() {
    bool changed = false;
    for(unsigned int i = 0; i < spinners.size(); i++) {
        int n = spinners.at(i)->value();
        if(n != values.at(i)) {
            changed = true;
            values[i] = n;
        }
    }
    
    if(changed) emitSliceChanged();
}

void TBSlicer::rowAxisChanged(int which) {
    if(which == oldR) return;

    shouldEmit = false;
    if(which == oldC) {
        // skip over unavailable axes
        if(which == oldR - 1 && which > 0)
            rowsSpinBox->setValue(which - 1);
        else if(which == oldR + 1 && which < rowsSpinBox->maximum())
            rowsSpinBox->setValue(which + 1);
        else
            rowsSpinBox->setValue(oldR);
            
        return;
    }
    shouldEmit = true;
    
    if(oldR >= 0) spinners.at(oldR)->setEnabled(true);
    spinners.at(which)->setEnabled(false);
    oldR = which;

    emitSliceChanged();
}

void TBSlicer::colAxisChanged(int which) {
    if(which == oldC) return;

    shouldEmit = false;
    if(which == oldR) {
        // skip over unavilable axes
        if(which == oldC - 1 && which > 0)
            colsSpinBox->setValue(which - 1);
        else if(which == oldC + 1 && which < colsSpinBox->maximum())
            colsSpinBox->setValue(which + 1);
        else
            colsSpinBox->setValue(oldC);
            
        return;
    }
    shouldEmit = true;

    if(oldC >= 0) spinners.at(oldC)->setEnabled(true);
    spinners.at(which)->setEnabled(false);
    oldC = which;
    emitSliceChanged();
}

}
