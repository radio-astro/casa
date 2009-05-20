//# QtEditingWidget.qo.h: Simple editing widgets.
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
#ifndef QTEDITINGWIDGET_QO_H_
#define QTEDITINGWIDGET_QO_H_

#include <casaqt/QtUtilities/PlotFileWidget.ui.h>
#include <casaqt/QtUtilities/PlotLabelWidget.ui.h>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

// A simple parent for editing widgets.
class QtEditingWidget : public QWidget {
    Q_OBJECT
    
public:
    // Constructor which takes an optional parent widget.
    QtEditingWidget(QWidget* parent = NULL) : QWidget(parent) { }
    
    // Destructor.
    virtual ~QtEditingWidget() { }
    
    // Adds any radio buttons in the widget to the given button group.  This
    // method should be overridden by children that have radio buttons.
    virtual void addRadioButtonsToGroup(QButtonGroup* group) const { }
    
signals:
    // This signal should be emitted whenever the user changes any value
    // settings in the GUI.
    void changed();
    
    // This signal should be emitted when the user changes any value settings
    // in the GUI AND the new value is different from the last set value.
    void differentFromSet();
};


// Widget for choosing a label.  Lets the user choose between no label, a
// default label given at construction, or a custom label they can set.
class QtLabelWidget : public QtEditingWidget, Ui::LabelWidget {
    Q_OBJECT
    
public:
    // Constructor which takes the label associated with the "default" option,
    // and an optional parent widget.
    QtLabelWidget(const String& defaultLabel, QWidget* parent = NULL);
    
    // Destructor.
    ~QtLabelWidget();
    
    // Gets/Sets the currently set label on the widget.
    // <group>
    String getValue() const;
    void setValue(const String& value);
    // </group>
    
    // Overrides PlotMSWidget::addRadioButtonsToGroup().
    void addRadioButtonsToGroup(QButtonGroup* group) const;
    
private:
    // Default label.
    String itsDefault_;

    // Last set label.
    String itsValue_;
    
private slots:
    // Slot for when the set label changes.  The "check" flag can be used to
    // avoid emitting the changed signals twice (for example, when one radio
    // button turns off when another turns on).
    void labelChanged(bool check = true);
};


// Widget for choosing a file.
class QtFileWidget : public QtEditingWidget, Ui::FileWidget {
    Q_OBJECT
    
public:
    // Constructor which takes a flag for whether the file is a directory or
    // not (for the file chooser), a flag for whether the file is for saving
    // (and thus doesn't need to exist, for the file chooser), and an optional
    // parent widget.
    QtFileWidget(bool chooseDirectory, bool saveFile, QWidget* parent= NULL);
    
    // Destructor.
    ~QtFileWidget();
    
    // Gets/Sets the currently set file.
    // <group>
    String getFile() const;
    void setFile(const String& file);
    // </group>
    
private:
    // File chooser directory flag.
    bool isDirectory_;
    
    // File chooser save file flag.
    bool isSave_;
    
    // Last set file.
    String itsFile_;
    
private slots:
    // Slot to show a file chooser and set the result in the line edit.
    void browse();

    // Slot for when the set file changes.
    void fileChanged();
};

}

#endif /* QTEDITINGWIDGET_QO_H_ */
