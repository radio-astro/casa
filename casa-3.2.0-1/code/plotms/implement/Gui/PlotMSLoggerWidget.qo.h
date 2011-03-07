//# PlotMSLoggerWidget.qo.h: Widget to set log events and priority filter.
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
#ifndef PLOTMSLOGGERWIDGET_QO_H_
#define PLOTMSLOGGERWIDGET_QO_H_

#include <casa/Logging/LogMessage.h>
#include <casaqt/QtUtilities/QtEditingWidget.qo.h>

#include <QComboBox>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward declarations.
class QtComboCheckBox;


// Widget to set log events and priority filter.
class PlotMSLoggerWidget : public QtEditingWidget {
    Q_OBJECT
    
public:
    // Constructor which takes the maximum width for the widget and an optional
    // parent.  MUST be created after plotms' extended log types have already
    // been registered with PlotLogger.
    PlotMSLoggerWidget(const String& filename, int events,
            LogMessage::Priority priority, int maxWidth,
            bool editFilename = true, bool editPriority = true,
            QWidget* parent = NULL);
    
    // Destructor.
    ~PlotMSLoggerWidget();
    
    // Gets/Sets the currently set filename.
    // <group>
    String filename() const;
    void setFilename(const String& filename);
    // </group>
    
    // Gets/Sets the currently set events flag.
    // <group>
    int events() const;
    void setEvents(int events);
    // </group>
    
    // Gets/Sets the currently set priority.
    // <group>
    LogMessage::Priority priority() const;
    void setPriority(LogMessage::Priority priority);
    // </group>
    
private:
    // Last set filename.
    String itsFilename_;
    
    // Widget for editing filename.
    QtFileWidget* itsFilenameWidget_;
    
    // Last set events flag.
    int itsEvents_;
    
    // Combo check box for events.
    QtComboCheckBox* itsEventsBox_;
    
    // Event values for the combo box indices.
    QList<int> itsEventsIndexValues_;
    
    // Last set priority.
    LogMessage::Priority itsPriority_;
    
    // Combo box.
    QComboBox* itsPriorityBox_;
    
private slots:
    // Slot for when the widget changes.
    void widgetChanged();
};

}

#endif /* PLOTMSLOGGERWIDGET_QO_H_ */
