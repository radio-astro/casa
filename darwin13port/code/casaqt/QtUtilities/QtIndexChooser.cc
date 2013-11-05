//# QtIndexChooser.cc: Simple widget for choosing an index.
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
#include <casaqt/QtUtilities/QtIndexChooser.qo.h>

namespace casa {

////////////////////////////////
// QTINDEXCHOOSER DEFINITIONS //
////////////////////////////////

// Constructors/Destructors //

QtIndexChooser::QtIndexChooser(Type type, QWidget* parent) : QWidget(parent),
        itsType_(type) {
    setupUi(this);
    
    setType(itsType_, true);
    
    // Connect widgets.
    connect(nameChooser, SIGNAL(currentIndexChanged(int)),
            SLOT(indexChanged_()));
    connect(rowBox, SIGNAL(valueChanged(int)), SLOT(indexChanged_()));
    connect(colBox, SIGNAL(valueChanged(int)), SLOT(indexChanged_()));
    connect(indexBox, SIGNAL(valueChanged(int)), SLOT(indexChanged_()));
}

QtIndexChooser::~QtIndexChooser() { }


// Public Methods //

unsigned int QtIndexChooser::index() const {
    switch(itsType_) {
    case NAME:
        return itsNames_.value(nameChooser->currentText());
        
    case ROW_COL:
        return (rowBox->value() * colBox->maximum()) + colBox->value();
        
    case INDEX:
        return indexBox->value();
    }
    
    return 0;
}

void QtIndexChooser::setType(Type type, bool resizeToHint) {
    itsType_ = type;
    nameChooser->setVisible(type == NAME);
    rowColFrame->setVisible(type == ROW_COL);
    indexFrame->setVisible(type == INDEX);
    
    if(resizeToHint) {
        switch(type) {
        case NAME: resize(nameChooser->sizeHint()); break;
            
        case ROW_COL: resize(rowColFrame->sizeHint());  break;
            
        case INDEX: resize(indexFrame->sizeHint()); break;
        }
    }
}

void QtIndexChooser::setName(unsigned int /*index*/, const String& /*name*/) {
    if(itsType_ != NAME) return;
    // TODO
}

void QtIndexChooser::setNames(const vector<String>& names) {
    if(itsType_ != NAME) return;
    vector<unsigned int> v(names.size());
    for(unsigned int i = 0; i < v.size(); i++) v[i] = i;
    setNames(v, names);
}

void QtIndexChooser::setNames(const vector<unsigned int>& indices,
        const vector<String>& names) {
    if(itsType_ != NAME) return;
    itsNames_.clear();
    
    unsigned int n = min(indices.size(), names.size());
    for(unsigned int i = 0; i < n; i++)
        itsNames_.insert(names[i].c_str(), indices[i]);
    
    setupNameChooser();
}

void QtIndexChooser::setNames(const map<unsigned int, String>& names) {
    if(itsType_ != NAME) return;
    itsNames_.clear();
    
    map<unsigned int, String>::const_iterator iter;
    for(iter = names.begin(); iter != names.end(); iter++)
        itsNames_.insert(iter->second.c_str(), iter->first);
    
    setupNameChooser();
}

void QtIndexChooser::setNames(const map<String, unsigned int>& names) {
    if(itsType_ != NAME) return;
    itsNames_.clear();
    
    map<String, unsigned int>::const_iterator iter;
    for(iter = names.begin(); iter != names.end(); iter++)
        itsNames_.insert(iter->first.c_str(), iter->second);
    
    setupNameChooser();
}

void QtIndexChooser::setRowsCols(unsigned int nRows, unsigned int nCols) {
    if(itsType_ != ROW_COL) return;
    if(nRows == 0) nRows++;
    if(nCols == 0) nCols++;
    rowBox->setMaximum(nRows - 1);
    colBox->setMaximum(nCols - 1);
}

void QtIndexChooser::setMaxIndex(unsigned int maxIndex) {
    indexBox->setMaximum(maxIndex); }


// Private Methods //

void QtIndexChooser::setupNameChooser() {
    nameChooser->clear();    
    foreach(const QString& val, itsNames_.keys())
        nameChooser->addItem(val);
}

}
