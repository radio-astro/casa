//# PlotMSSelectionWidget.qo.h: GUI widget for editing a PlotMSSelection.
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
#ifndef PLOTMSSELECTIONWIDGET_QO_H_
#define PLOTMSSELECTIONWIDGET_QO_H_

#include <casaqt/QtUtilities/QtEditingWidget.qo.h>
#include <plotms/PlotMS/PlotMSSelection.h>

#include <casa/namespace.h>

namespace casa {

// Widget for editing a PlotMSSelection object.
class PlotMSSelectionWidget : public QtEditingWidget {
    Q_OBJECT
    
public:
    // Constructor that takes an optional parent.
    PlotMSSelectionWidget(QWidget* parent = NULL);
    
    // Destructor.
    ~PlotMSSelectionWidget();
    
    
    // Gets/Sets the currently displayed value.
    // <group>
    PlotMSSelection getValue() const;
    void setValue(const PlotMSSelection& selection);
    // </group>
    
private:
    // Last set value.
    PlotMSSelection itsValue_;
    
    // Changed flag.
    bool itsFlag_;
    
    // Value widgets.
    QMap<PlotMSSelection::Field, QLineEdit*> itsValues_;
    
private slots:
    // Slot for when a GUI widget value changes.
    void selectionChanged();
};

}

#endif /* PLOTMSSELECTIONWIDGET_QO_H_ */
