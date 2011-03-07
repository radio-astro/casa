//# ParamPanel.qo.h: Panel for a single parameter.
//# Copyright (C) 2005
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
#ifndef PARAMPANEL_QO_H_
#define PARAMPANEL_QO_H_

#include <casaqt/QtParamGui/QtParamGUI.qo.h>
#include <casaqt/QtParamGui/ParamPanel.ui.h>
#include <casaqt/QtParamGui/ValuePanel.qo.h>

#include <utility>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// A ParamPanel holds information for a single parameter.  It displays the
// parameter information (name, type, description, etc.) and provides a
// widget(s) for the user to input a value for that parameter.  These widget(s)
// are held in a ValuePanel.
class ParamPanel : public QWidget, Ui::ParamPanel {
    Q_OBJECT
    
public:
    // Constructor that takes the parameter name and a record describing its
    // attributes.  Allowed attributes include:
    // * allowed: a list of possible values to choose from
    // * description: a description of the parameter
    // * example: example(s) for the parameter
    // * kind: indicates whether the parameter has a special type (such as
    //         a file)
    // * limittypes: if the parameter is type "any", the potential types can
    //               be limited
    // * mustexist: if the parameter requires a non-empty value
    // * range: range of values for numeric types
    // * type (required): the type for the parameter
    // * units: what conceptual units the parameter is in
    // * value: a default value
    ParamPanel(const QtParamGUI& parent, String name,
               const RecordInterface& record, const Record& globals,
               const Record& last, const QtParamGUI::ResetMode& resetMode,
               int labelWidth = -1, const String& constraints = "",
               bool subparam = false);
    
    // Destructor.
    ~ParamPanel();
    
    // Returns the parameter's name.
    String name();
    
    // Returns the ValuePanel.
    ValuePanel* value();
    
    // Returns whether the current value given to the parameter is valid or
    // not.  If not valid, the second pair value is the reason why.
    pair<bool, String> isValid();
    
    // Put the current value into the record with the parameter name as the
    // key.
    void getValue(Record& record);
    
public slots:
    // Reset the value to the default (if any).
    void reset();
    
signals:
    // Emitted when the user changes the value in this panel.
    void valueChanged(String name);
    
private:
    // Parameter name.
    String m_name;
    
    // Parameter attributes.
    const RecordInterface& m_record;
    
    // Global settings.
    const Record& m_globals;
    
    // Last settings.
    const Record& m_last;
    
    // Reset to globals or defaults.
    const QtParamGUI::ResetMode& m_resetMode;
    
    // (Optional) label to display units.
    QLabel* m_units;
    
    // Value panel.
    ValuePanel* m_value;
    
    // Constraints dialog.
    QDialog* m_constDialog;
    
    
    // Sets the value to the default defined in the attributes. 
    void setValue();
    
private slots:
    // Show or hide the panel with the description and extra info.
    void showHideDesc();
    
    // Show/hide the constraints dialog.
    void showHideConstraints(bool checked);
    
    // When the constraints dialog is closed.
    void constraintsClosed();
    
    // Catch the value panel's valueChanged signal and re-emit it.
    void changedValue() { emit valueChanged(m_name); }
};

}

#endif /*PARAMPANEL_QO_H_*/
