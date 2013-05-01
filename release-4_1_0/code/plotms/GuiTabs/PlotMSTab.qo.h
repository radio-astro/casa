//# PlotMSTabs.qo.h: Tab GUI widgets.
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
#ifndef PLOTMSTABS_QO_H_
#define PLOTMSTABS_QO_H_

#include <casa/BasicSL/String.h>
#include <plotms/PlotMS/PlotMSWatchedParameters.h>

#include <QComboBox>
#include <QLabel>
#include <QList>
#include <QToolButton>
#include <QGroupBox>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations
class PlotMSApp;
class PlotMSPlotter;


// Abstract parent for any widget that goes in the tabbed side panel of the
// PlotMSPlotter.
class PlotMSTab : public QWidget, public PlotMSParametersWatcher {
    Q_OBJECT
    
public:
    // Static //
    
    // Convenience methods for setting the current value of the given QComboBox
    // to the given.
    // <group>
    static bool setChooser(QComboBox* chooser, const QString& value);
    static bool setChooser(QComboBox* chooser, const String& value) {
        QString str(value.c_str());
        return setChooser(chooser, str); }
    static bool setChooser(QComboBox* chooser, const char* value) {
        QString str(value);
        return setChooser(chooser, str); }
    // </group>
    
    
    // Non-Static //
    
    // Constructor that takes the parent plotter for this tab.
    PlotMSTab(PlotMSPlotter* parent);
    
    // Destructor.
    virtual ~PlotMSTab();
    
    
    // Returns a string to use as the tab name for this widget.
    virtual QString tabName() const = 0;
    
    // Returns all tool buttons on this tab.
    virtual QList<QToolButton*> toolButtons() const {
        return QList<QToolButton*>(); }
    
    // Sets up the tab for the given maximum width.
    virtual void setupForMaxWidth(int maxWidth) { (void)maxWidth; }
    
protected:
    // Parent.
    PlotMSApp* itsParent_;
    
    // Plotter.
    PlotMSPlotter* itsPlotter_;
    
    // Default text for labels, so that they can be easily switched to red or
    // normal depending on whether the item has changed or not.
    QMap<QLabel*, QString> itsLabelDefaults_;
    
    
    // These alter the displayed text of 
    // Some widget types have a .title not a .text, and there's no
    // Qt4 abstract class that encompasses all types that do have .text,
    // so we overload highlightWidgetText for several classes.
    // Note that the Button version is meant for QCheckBox and QRadioButton,
    // and probably isn't as useful for pushbuttons.
    // Note: was formerly named changedText(). Name and purpose was unclear.
    // <group>
    void highlightWidgetText(QLabel* label,  bool highlight);
    void highlightWidgetText(QGroupBox *box,  bool highlight);
    void highlightWidgetText(QAbstractButton *but,  bool highlight);
    // </group>
    
    
    // Returns the given text altered with HTML tags for highlighting,
    // meant to be assigned to a Qt widget's title or similar property.
    // If highlight is false, the text is returned stripped of any HTML.
    // If highlighted, the text is made red using a <font> tag.
    // <group>
    static QString highlightifyText(const QString& text, bool highlight);
    
    // Variation taking a casacore String.
    static QString highlightifyText(const String& text, bool changed)  {
        QString str(text.c_str());
        return highlightifyText(str, changed); }
        
    // Variation of highlightifyText taking plain C-style text.
    static QString highlightifyText(const char* text, bool changed)  {
        QString str(text);
        return highlightifyText(str, changed); }
        

      
    // </group>
};

}

#endif /* PLOTMSTABS_QO_H_ */
