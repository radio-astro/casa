//# PlotMSAxisWidget.cc: Widget for choosing a single axis.
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
#include <plotms/Gui/PlotMSAxisWidget.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotRangeWidget.qo.h>
#include <plotms/GuiTabs/PlotMSTab.qo.h>

namespace casa {

//////////////////////////////////
// PLOTMSAXISWIDGET DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

PlotMSAxisWidget::PlotMSAxisWidget(PMS::Axis defaultAxis, int attachAxes,
        QWidget* parent) : QtEditingWidget(parent) {
    setupUi(this);
    
    // Setup axes choices.
    const vector<String>& axes = PMS::axesStrings();
    String def = PMS::axis(defaultAxis);
    for(unsigned int i = 0; i < axes.size(); i++) {
        chooser->addItem(axes[i].c_str());
        if(axes[i] == def) chooser->setCurrentIndex(i);
    }
    
    // Setup data column choices.
    const vector<String>& data = PMS::dataColumnStrings();
    def = PMS::dataColumn(PMS::DEFAULT_DATACOLUMN);
    for(unsigned int i = 0; i < data.size(); i++) {
        dataChooser->addItem(data[i].c_str());
        if(data[i] == def) dataChooser->setCurrentIndex(i);
    }
    
    // Setup attach axes.
    attachBottom->setVisible(attachAxes & X_BOTTOM);
    attachTop->setVisible(attachAxes & X_TOP);
    attachLeft->setVisible(attachAxes & Y_LEFT);
    attachRight->setVisible(attachAxes & Y_RIGHT);
    
    // Setup range widget.
    itsRangeWidget_ = new PlotRangeWidget(true);
    QtUtilities::putInFrame(rangeFrame, itsRangeWidget_);
    
    axisChanged(chooser->currentText());
    
    // Connect widgets.
    connect(chooser, SIGNAL(currentIndexChanged(const QString&)),
            SLOT(axisChanged(const QString&)));
    
    connect(chooser, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
    connect(dataChooser, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()));
    connect(attachBottom, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(attachTop, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(attachLeft, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(attachRight, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(itsRangeWidget_, SIGNAL(changed()), SIGNAL(changed()));
}

PlotMSAxisWidget::~PlotMSAxisWidget() { }


// Public Methods //

PMS::Axis PlotMSAxisWidget::axis() const {
    return PMS::axis(chooser->currentText().toStdString()); }
PMS::DataColumn PlotMSAxisWidget::data() const {
    return PMS::dataColumn(dataChooser->currentText().toStdString()); }
PlotAxis PlotMSAxisWidget::attachAxis() const {
    if(attachTop->isChecked()) return X_TOP;
    else if(attachLeft->isChecked()) return Y_LEFT;
    else if(attachRight->isChecked()) return Y_RIGHT;
    else return X_BOTTOM;
}
bool PlotMSAxisWidget::rangeCustom() const {
    return itsRangeWidget_->isCustom(); }
prange_t PlotMSAxisWidget::range() const {
    return itsRangeWidget_->getRange(); }

void PlotMSAxisWidget::setValue(PMS::Axis axis, PMS::DataColumn data,
        PlotAxis attachAxis, bool rangeCustom, prange_t range){
    PlotMSTab::setChooser(chooser, PMS::axis(axis));
    PlotMSTab::setChooser(dataChooser, PMS::dataColumn(data));
    
    switch(attachAxis) {
    case X_BOTTOM: attachBottom->setChecked(true); break;
    case X_TOP:    attachTop->setChecked(true);    break;
    case Y_LEFT:   attachLeft->setChecked(true);   break;
    case Y_RIGHT:  attachRight->setChecked(true);  break;
    default: break;
    }
    
    itsRangeWidget_->setRange(PMS::axisType(axis) == PMS::TTIME, rangeCustom,
            range);
}

void PlotMSAxisWidget::setInCache(bool isInCache) {
    inCache->setChecked(isInCache); }


// Private Slots //

void PlotMSAxisWidget::axisChanged(const QString& value) {
    dataFrame->setVisible(PMS::axisIsData(PMS::axis(value.toStdString())));
}

}
