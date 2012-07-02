//# PlotMSAveragingWidget.cc: GUI widget for editing PlotMSAveraging.
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
#include <plotms/Gui/PlotMSAveragingWidget.qo.h>

#include <casaqt/QtUtilities/QtButtonGroup.qo.h>

#include <QDoubleValidator>
#include <QSet>

namespace casa {

///////////////////////////////////////
// PLOTMSAVERAGINGWIDGET DEFINITIONS //
///////////////////////////////////////

PlotMSAveragingWidget::PlotMSAveragingWidget(QWidget* parent) :
        QtEditingWidget(parent), itsFlag_(true) {
    // Set up GUI.
    setupUi(this);
    
    // Set up maps.
    itsFlags_[PlotMSAveraging::CHANNEL] = channel;
    itsFlags_[PlotMSAveraging::TIME] = time;
    itsFlags_[PlotMSAveraging::SCAN] = scan;
    itsFlags_[PlotMSAveraging::FIELD] = field;
    itsFlags_[PlotMSAveraging::BASELINE] = baseline;
    itsFlags_[PlotMSAveraging::ANTENNA] = antenna;
    itsFlags_[PlotMSAveraging::SPW] = spw;
    
    itsValues_[PlotMSAveraging::CHANNEL] = channelValue;
    itsValues_[PlotMSAveraging::TIME] = timeValue;
    
    // Set up mutually exclusive groups.
    QtButtonGroup* buttonGroup;
    QSet<PlotMSAveraging::Field> seenFields;
    foreach(PlotMSAveraging::Field f, itsFlags_.keys()) {
        if(seenFields.contains(f)) continue;
        const vector<PlotMSAveraging::Field>& group =
            PlotMSAveraging::fieldMutuallyExclusiveGroup(f);
        if(group.size() > 0) {
            buttonGroup = new QtButtonGroup(this);
            buttonGroup->addButton(itsFlags_[f]);
            seenFields.insert(f);
            
            for(unsigned int i = 0; i < group.size(); i++) {
                if(seenFields.contains(group[i])) continue;
                buttonGroup->addButton(itsFlags_[group[i]]);
                seenFields.insert(group[i]);
            }
        }
    }
    
    // Set value.
    setValue(itsValue_);
    
    // Connect widgets.
    foreach(QCheckBox* w, itsFlags_)
        connect(w, SIGNAL(toggled(bool)), SLOT(averagingChanged()));
    
    foreach(QLineEdit* w, itsValues_) {
        w->setValidator(new QDoubleValidator(w));
        connect(w, SIGNAL(textChanged(const QString&)),
                SLOT(averagingChanged()));
    }
}

PlotMSAveragingWidget::~PlotMSAveragingWidget() { }


PlotMSAveraging PlotMSAveragingWidget::getValue() const {
    PlotMSAveraging val;
    
    const vector<PlotMSAveraging::Field>& f = PlotMSAveraging::fields();
    double d;
    bool ok;
    for(unsigned int i = 0; i < f.size(); i++) {
        if(!itsFlags_.contains(f[i])) continue;
        val.setFlag(f[i], itsFlags_.value(f[i])->isChecked());
        if(PlotMSAveraging::fieldHasValue(f[i])) {
            if(itsValues_.contains(f[i])) {
                d = itsValues_.value(f[i])->text().toDouble(&ok);
                if(ok) val.setValue(f[i], d);
            }
        }
    }
    
    return val;
}

void PlotMSAveragingWidget::setValue(const PlotMSAveraging& val) {
    itsValue_ = val;
    bool oldFlag = itsFlag_;
    itsFlag_ = false;
    
    const vector<PlotMSAveraging::Field>& f = PlotMSAveraging::fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        if(!itsFlags_.contains(f[i])) continue;
        itsFlags_.value(f[i])->setChecked(val.getFlag(f[i]));
        if(itsValues_.contains(f[i]) && PlotMSAveraging::fieldHasValue(f[i]))
            itsValues_.value(f[i])->setText(
                    QString::number(val.getValue(f[i])));
    }
    
    itsFlag_ = oldFlag;
}


void PlotMSAveragingWidget::averagingChanged() {
    if(!itsFlag_) return;
    emit changed();
    PlotMSAveraging val = getValue();
    if(val != itsValue_) emit differentFromSet();
}

}
