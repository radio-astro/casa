//# PlotMSFlaggingWidget.qo.h: GUI widget for editing PlotMSFlagging.
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
#ifndef PLOTMSFLAGGINGWIDGET_QO_H_
#define PLOTMSFLAGGINGWIDGET_QO_H_

#include <plotms/Gui/PlotMSFlaggingWidget.ui.h>

#include <casaqt/QtUtilities/QtEditingWidget.qo.h>
#include <plotms/Gui/PlotMSSelectionWidget.qo.h>
#include <plotms/PlotMS/PlotMSFlagging.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations.
class PlotMSPlot;


// GUI widget for editing a PlotMSFlagging object.
class PlotMSFlaggingWidget : public QtEditingWidget, Ui::FlaggingWidget {
    Q_OBJECT
    
public:
    // Constructor that takes an optional parent widget.
    PlotMSFlaggingWidget(QWidget* parent = NULL);
    
    // Destructor.
    ~PlotMSFlaggingWidget();
    
    
    // Gets the currently set value, using the given PlotMSPlot to get the MS
    // objects from if not NULL.  If the plot is NULL, then the MS objects must
    // be set using PlotMSFlagging::setMS() before being used.
    PlotMSFlagging getValue(PlotMSPlot* plot = NULL) const;
    
    // Sets the currently displayed value to the given.  If MS objects are set
    // in the given value, they are ignored.
    void setValue(const PlotMSFlagging& flagging);
    
private:
    // Last set value.
    PlotMSFlagging itsValue_;
    
    // Changed flag.
    bool itsFlag_;
    
    // Value widgets.
    // <group>
    QMap<PlotMSFlagging::Field, QAbstractButton*> itsFlags_;
    QMap<PlotMSFlagging::Field, QLineEdit*> itsValues_;
    QMap<PlotMSFlagging::Field, PlotMSSelectionWidget*> itsSelectionValues_;
    // </group>
    
private slots:
    // Slot for when a GUI widget value changes.
    void flaggingChanged();
};

}

#endif /* PLOTMSFLAGGINGWIDGET_QO_H_ */
