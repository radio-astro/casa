//# PlotRangeWidget.qo.h: Widget for plot axes ranges.
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
#ifndef PLOTRANGEWIDGET_QO_H
#define PLOTRANGEWIDGET_QO_H

#include <plotms/Gui/PlotRangeWidget.ui.h>

#include <casaqt/QtUtilities/QtEditingWidget.qo.h>
#include <graphics/GenericPlotter/PlotOptions.h>

#include <utility>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// Widget for selecting a plot axis range.  Lets the user choose between an
// automatic range or a custom range they can set.  Ranges can either be normal
// doubles, or two dates which are converted to/from doubles as needed using
// PMS::dateDouble().
class PlotRangeWidget : public QtEditingWidget, Ui::RangeWidget {
    Q_OBJECT
    
public:
    // Constructor which takes an optional parent widget.
    PlotRangeWidget(bool customOnTwoLines = false, QWidget* parent = NULL);
    
    // Destructor.
    ~PlotRangeWidget();
    
    
    // Gets/Sets whether or not the displayed range widgets are for dates or
    // not.
    // <group>
    bool isDate() const;
    void setIsDate(bool isDate = true);
    // </group>
    
    // Gets whether the currently set date is custom or not.
    bool isCustom() const;
    
    // Gets/Sets the currently set range.
    prange_t getRange() const;
    void getRange(double& from, double& to) {
        prange_t r = getRange();
        from = r.first;
        to = r.second;
    }
    void setRange(bool isDate, bool isCustom, double from, double to);
    void setRange(bool isDate, bool isCustom, prange_t range) {
        setRange(isDate, isCustom, range.first, range.second); }
    // </group>
    
    // Overrides PlotMSWidget::addRadioButtonsToGroup().
    void addRadioButtonsToGroup(QButtonGroup* group) const;
    
private:
    // Whether last set range was custom or not.
    bool isCustom_;
    
    // Last set range.
    double from_, to_;
    
private slots:
    // Slot for when the set range changes.
    void rangeChanged();
};

}

#endif /* PLOTRANGEWIDGET_QO_H */
