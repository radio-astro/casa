//# PlotMSAxisWidget.qo.h: Widget for choosing a single axis.
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
#ifndef PLOTMSAXISWIDGET_QO_H_
#define PLOTMSAXISWIDGET_QO_H_

#include <plotms/Gui/PlotMSAxisWidget.ui.h>

#include <casaqt/QtUtilities/QtEditingWidget.qo.h>
#include <plotms/PlotMS/PlotMSConstants.h>
#include <casaqt/QtUtilities/QtUtilities.h>
#include <casa/namespace.h>

namespace casa {

//# Forward declarations.
class PlotRangeWidget;


// Widget for choosing a single axis.
class PlotMSAxisWidget : public QtEditingWidget, Ui::AxisWidget {
    Q_OBJECT
    
public:
    // Constructor which takes optional parent widget.
    PlotMSAxisWidget(PMS::Axis defaultAxis, int attachAxes,
            QWidget* parent = NULL);
    
    // Destructor.
    ~PlotMSAxisWidget();
    
    // Returns the labels used in the widget.
    // <group>
    QLabel* dataLabel() { return AxisWidget::dataLabel; }
    QLabel* attachLabel() { return AxisWidget::attachLabel; }
    QLabel* rangeLabel() { return AxisWidget::rangeLabel; }
    QLabel* axisLabel(){
    	return AxisWidget::axisLabel;
    }
    // </group>
    
    // Gets the current set values in the widget.
    // <group>
    PMS::Axis axis() const;
    PMS::DataColumn data() const;
    PlotAxis attachAxis() const;
    bool matchesData(const PlotMSAxisWidget* other ) const;
    bool rangeCustom() const;
    prange_t range() const;
    // </group>

    // set default custom range according to chosen MS
    void setRange(bool isDate, double from, double to);
    
    // Sets the displayed value to the given.
    void setValue(PMS::Axis axis, PMS::DataColumn data, PlotAxis attachAxis,
            bool rangeCustom, prange_t range);
    
    // Sets the "in cache" checkbox to the given.
    void setInCache(bool inCache);
    void insertLabelDefaults( QMap<QLabel*,QString>& map );

    //Returns an identifier for this axis data.
    QString getIdentifier() const;

signals:
	void axisIdentifierChanged(PlotMSAxisWidget*);
	void axisChanged();

private:
    void initPlotAxis(int attachAxis);
    void setAttachAxis(PlotAxis attachAxis );

    // Widget for the range.
    PlotRangeWidget* itsRangeWidget_;

    
private slots:
    // Slot for when the axis value changed.
    void axisChanged(const QString& value);
    void axisDataChanged();

};

}

#endif /* PLOTMSAXISWIDGET_QO_H_ */
