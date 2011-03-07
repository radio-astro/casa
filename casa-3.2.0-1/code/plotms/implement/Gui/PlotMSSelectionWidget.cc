//# PlotMSSelectionWidget.cc: GUI widget for editing a PlotMSSelection.
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
#include <plotms/Gui/PlotMSSelectionWidget.qo.h>

#include <QLabel>

namespace casa {

PlotMSSelectionWidget::PlotMSSelectionWidget(QWidget* parent) :
        QtEditingWidget(parent), itsFlag_(true) {    
    QGridLayout* l = new QGridLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(3);
    
    vector<PlotMSSelection::Field> fields = PlotMSSelection::fields();
    QLabel* label; QLineEdit* val;
    for(unsigned int i = 0; i < fields.size(); i++) {
        label = new QLabel(QString(PlotMSSelection::field(fields[i]).c_str()));
        label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        l->addWidget(label, i, 0);        
        val = new QLineEdit();       
        l->addWidget(val, i, 1);
        itsValues_.insert(fields[i], val);
        
        connect(val, SIGNAL(textChanged(const QString&)),
                SLOT(selectionChanged()));
    }
    
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

PlotMSSelectionWidget::~PlotMSSelectionWidget() { }


PlotMSSelection PlotMSSelectionWidget::getValue() const {
    PlotMSSelection sel;

    QMapIterator<PlotMSSelection::Field, QLineEdit*> i(itsValues_);
    while(i.hasNext()) {
         i.next();
         sel.setValue(i.key(), i.value()->text().toStdString());
     }
    
    return sel;
}

void PlotMSSelectionWidget::setValue(const PlotMSSelection& selection) {
    itsValue_ = selection;
    bool oldFlag = itsFlag_;
    itsFlag_ = false;
    
    QMapIterator<PlotMSSelection::Field, QLineEdit*> i(itsValues_);
    while(i.hasNext()) {
         i.next();
         i.value()->setText(selection.getValue(i.key()).c_str());
    }
    
    itsFlag_ = oldFlag;
}


void PlotMSSelectionWidget::selectionChanged() {
    if(!itsFlag_) return;
    emit changed();
    PlotMSSelection val = getValue();
    if(val != itsValue_) emit differentFromSet();
}

}
