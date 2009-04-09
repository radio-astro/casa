//# PlotMSAveragingWidget.qo.h: GUI widget for editing PlotMSAveraging.
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
#ifndef PLOTMSAVERAGINGWIDGET_QO_H_
#define PLOTMSAVERAGINGWIDGET_QO_H_

#include <plotms/Gui/PlotMSAveragingWidget.ui.h>

#include <casaqt/QtUtilities/QtEditingWidget.qo.h>
#include <plotms/PlotMS/PlotMSAveraging.h>

#include <casa/namespace.h>

namespace casa {

/*
    void changed();

    void differentFromSet();
 */

// GUI widget for editing a PlotMSAveraging object.
class PlotMSAveragingWidget : public QtEditingWidget, Ui::AveragingWidget {
    Q_OBJECT
    
public:
    // Constructor.
    PlotMSAveragingWidget(QWidget* parent = NULL);
    
    // Destructor.
    ~PlotMSAveragingWidget();
    
    
    // Gets/Sets the currently displayed value.
    // <group>
    PlotMSAveraging getValue() const;
    void setValue(const PlotMSAveraging& averaging);
    // </group>
    
private:
    // Last set value.
    PlotMSAveraging itsValue_;
    
    // Changed flag.
    bool itsFlag_;
    
    // Value widgets.
    // <group>
    QMap<PlotMSAveraging::Field, QCheckBox*> itsFlags_;
    QMap<PlotMSAveraging::Field, QLineEdit*> itsValues_;
    // </group>
    
private slots:
    // Slot for when a GUI widget value changes.
    void averagingChanged();
};

}

#endif /* PLOTMSAVERAGINGWIDGET_QO_H_ */
