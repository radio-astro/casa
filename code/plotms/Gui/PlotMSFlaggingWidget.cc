//# PlotMSFlaggingWidget.cc: GUI widget for editing PlotMSFlagging.
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
#include <plotms/Gui/PlotMSFlaggingWidget.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSSelectionWidget.qo.h>
#include <plotms/Plots/PlotMSPlot.h>

#include <QDoubleValidator>

namespace casa {

//////////////////////////////////////
// PLOTMSFLAGGINGWIDGET DEFINITIONS //
//////////////////////////////////////

PlotMSFlaggingWidget::PlotMSFlaggingWidget(QWidget* parent) :
        QtEditingWidget(parent), itsFlag_(true) {
    // Set up GUI.
    setupUi(this);
    PlotMSSelectionWidget* selWidget = new PlotMSSelectionWidget();
    QtUtilities::putInFrame(alternateSelectionFrame, selWidget);
    
    QtUtilities::putInScrollArea(extendFrame);
    extendToAlternateFrame->hide();
    
    // Set up maps.
    itsFlags_[PlotMSFlagging::EXTEND] = extend;
    itsFlags_[PlotMSFlagging::CORR] = correlation;
    itsFlags_[PlotMSFlagging::CORR_ALL] = correlationAll;
    itsFlags_[PlotMSFlagging::CORR_POLN_DEP] = correlationPolnDep;
    itsFlags_[PlotMSFlagging::CHANNEL] = channel;
    itsFlags_[PlotMSFlagging::SPW] = spw;
    itsFlags_[PlotMSFlagging::ANTENNA] = antenna;
    itsFlags_[PlotMSFlagging::ANTENNA_ANTENNA] = antennaAntenna;
    itsFlags_[PlotMSFlagging::ANTENNA_BASELINES] = antennaBaselines;
    itsFlags_[PlotMSFlagging::TIME] = time;
    itsFlags_[PlotMSFlagging::SCANS] = scans;
    itsFlags_[PlotMSFlagging::FIELD] = field;
    itsFlags_[PlotMSFlagging::SEL_SELECTED] = extendToSelected;
    itsFlags_[PlotMSFlagging::SEL_ALTERNATE] = extendToAlternate;
    
    itsValues_[PlotMSFlagging::ANTENNA_ANTENNA] = antennaAntennaValue;
    itsSelectionValues_[PlotMSFlagging::SEL_ALTERNATE] = selWidget;
    
    // Set value.
    setValue(itsValue_);
    
    // Connect widgets.
    foreach(QAbstractButton* w, itsFlags_)
        connect(w, SIGNAL(toggled(bool)), SLOT(flaggingChanged()));
    
    foreach(QLineEdit* w, itsValues_) {
        w->setValidator(new QDoubleValidator(w));
        connect(w, SIGNAL(textChanged(const QString&)),
                SLOT(flaggingChanged()));
    }
    
    foreach(PlotMSSelectionWidget* w, itsSelectionValues_)
        connect(w, SIGNAL(changed()), SLOT(flaggingChanged()));
}

PlotMSFlaggingWidget::~PlotMSFlaggingWidget() { }


PlotMSFlagging PlotMSFlaggingWidget::getValue(PlotMSPlot* /*plot*/) const {
    PlotMSFlagging val;

    const vector<PlotMSFlagging::Field>& f = PlotMSFlagging::fields();
    double d;
    PlotMSSelection s;
    bool ok;
    for(unsigned int i = 0; i < f.size(); i++) {
        if(!itsFlags_.contains(f[i])) continue;
        val.setFlag(f[i], itsFlags_.value(f[i])->isChecked());
        if(PlotMSFlagging::fieldHasValue(f[i])) {
            if(itsValues_.contains(f[i])) {
                d = itsValues_.value(f[i])->text().toDouble(&ok);
                if(ok) val.setValue(f[i], d);
            }
        }
        if(PlotMSFlagging::fieldHasSelectionValue(f[i])) {
            if(itsSelectionValues_.contains(f[i])) {
                s = itsSelectionValues_.value(f[i])->getValue();
                val.setSelectionValue(f[i], s);
            }
        }
    }
    
    return val;
}

void PlotMSFlaggingWidget::setValue(const PlotMSFlagging& val) {
    itsValue_ = val;
    bool oldFlag = itsFlag_;
    itsFlag_ = false;
    
    const vector<PlotMSFlagging::Field>& f = PlotMSFlagging::fields();
    for(unsigned int i = 0; i < f.size(); i++) {
        if(!itsFlags_.contains(f[i])) continue;
        itsFlags_.value(f[i])->setChecked(val.getFlag(f[i]));
        if(itsValues_.contains(f[i]) && PlotMSFlagging::fieldHasValue(f[i]))
            itsValues_.value(f[i])->setText(
                    QString::number(val.getValue(f[i])));
        if(itsSelectionValues_.contains(f[i]) &&
           PlotMSFlagging::fieldHasSelectionValue(f[i]))
            itsSelectionValues_.value(f[i])->setValue(
                    val.getSelectionValue(f[i]));
    }
    
    itsFlag_ = oldFlag;
}


void PlotMSFlaggingWidget::flaggingChanged() {
    if(!itsFlag_) return;
    emit changed();
    PlotMSFlagging val = getValue();
    if(val != itsValue_) emit differentFromSet();
}

}
