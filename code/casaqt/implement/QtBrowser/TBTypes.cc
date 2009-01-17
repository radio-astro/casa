//# TBTypes.cc: (Not finished) Widget for entering a value based on its type.
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
#include <casaqt/QtBrowser/TBTypes.qo.h>
#include <casaqt/QtBrowser/TBConstants.h>

namespace casa {

/////////////////////////
// TBTYPES DEFINITIONS //
/////////////////////////

// Constructors/Destructors //

TBTypes::TBTypes(String t, bool ia) : QWidget(), arrayTypes(NULL),
									  recordTypes(NULL) {
    setupUi(this);
    types = TBConstants::nonArrayTypes();
    if(ia) {
        setupArray();
        setupRecord();
    }

    setType(t);
}

TBTypes::~TBTypes() {
    delete types;
    if(arrayTypes != NULL) delete arrayTypes;
    if(recordTypes != NULL) delete recordTypes;
}

// Public Methods //

void TBTypes::setType(String t) {
    type = String(t);

    bool lineClose = true, textClose = true, complexClose = true,
         recordClose = true, arrayClose = true, array = false;
    if(TBConstants::typeIsArray(type)) {
        arrayClose = false;
        array = true;
    } else if(type == TBConstants::TYPE_STRING) {
        textClose = false;
    } else if(type == TBConstants::TYPE_COMPLEX ||
    		  type == TBConstants::TYPE_DCOMPLEX) {
        complexClose = false;
    } else if(type == TBConstants::TYPE_RECORD) {
        recordClose = false;
    } else {
        lineClose = false;
    }

    lineFrame->setHidden(lineClose);
    textFrame->setHidden(textClose);
    complexFrame->setHidden(complexClose);
    recordFrame->setHidden(recordClose);
    arrayFrame->setHidden(arrayClose);

    if(array) {
        String at = TBConstants::arrayType(type);
        arrayTypes->setType(at);
        arrayTable->clear();
        arrayTable->setRowCount(1);
        arrayTable->setColumnCount(1);
        arrayTable->setCurrentCell(0, 0);
        QTableWidgetItem* item = arrayTable->currentItem();
        item->setFlags(item->flags() & (~Qt::ItemIsEditable));
    }
}

// Private Methods //

void TBTypes::setupArray() {
    QGridLayout* l = new QGridLayout(arrayValueFrame);
#if QT_VERSION >= 0x040300
    l->setHorizontalSpacing(0);
    l->setVerticalSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
#else
    l->setSpacing(0);
    l->setMargin(0);
#endif
    arrayTypes = new TBTypes(types->at(0), false);
    l->addWidget(arrayTypes, 0, 0, 1, 1);
    connect(arrayDimSetButton, SIGNAL(clicked()), this, SLOT(arrayDimSet()));
    connect(arrayValSetButton, SIGNAL(clicked()), this, SLOT(arrayValSet()));
}

void TBTypes::setupRecord() {
    for(unsigned int i = 0; i < types->size(); i++) {
        String t = types->at(i);
        String name = TBConstants::typeName(t);
        recordTypeChooser->addItem((name + " (" + t + ")").c_str());
    }
    QGridLayout* l = new QGridLayout(recordValueFrame);
#if QT_VERSION >= 0x040300
    l->setHorizontalSpacing(0);
    l->setVerticalSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
#else
    l->setSpacing(0);
    l->setMargin(0);
#endif
    recordTypes = new TBTypes(types->at(0), false);
    l->addWidget(recordTypes, 0, 0, 1, 1);
    connect(recordTypeChooser, SIGNAL(currentIndexChanged(int)),
            this, SLOT(recordChooserChanged(int)));
}

// Private Slots //

void TBTypes::recordChooserChanged(int index) {
    if(index < 0) return;
    recordTypes->setType(types->at(index));
}

void TBTypes::arrayDimSet() {
    int rows = arrayVectorBox->isChecked() ? 1 : arrayRowSpinner->value();
    int cols = arrayColSpinner->value();
    arrayTable->setRowCount(rows);
    arrayTable->setColumnCount(cols);
    for(int i = 0; i < arrayTable->rowCount(); i++) {
        for(int j = 0; j < arrayTable->columnCount(); j++) {
            QTableWidgetItem* item = arrayTable->item(i, j);
            item->setFlags(item->flags() & (~Qt::ItemIsEditable));
        }
    }
}

void TBTypes::arrayValSet() {
    // not implemented
}

}
