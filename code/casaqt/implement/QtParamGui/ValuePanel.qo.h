//# ValuePanel.qo.h: Panel for inputing a data value.
//# Copyright (C) 2008
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
#ifndef VALUEPANEL_QO_H_
#define VALUEPANEL_QO_H_

#include <casaqt/QtParamGui/ValuePanel.ui.h>

#include <casa/Containers/Record.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

class RecordItemPanel;

// Abstract parent class of any panels that accept user input for values.
class ValuePanel : public QWidget {
    Q_OBJECT
    
public:
    // Constructor that takes the parameter name and a reference to the
    // record detailing its properties.
    ValuePanel(String paramName, const RecordInterface& paramRecord);
    
    // Destructor.
    virtual ~ValuePanel();
    
    
    // Abstract Methods //
    
    // Returns whether or not the currently inputted value is true or not.
    // If invalid, the String contains the reason(s), newline-separated.
    virtual pair<bool, String> valueIsValid() = 0;
    
    // Returns the type of the current value.
    virtual DataType type() = 0;
    
    // Inserts the entered value into the given record with the given name.
    virtual void getValue(String name, Record& record) = 0;
    
    // Sets the current value to the one given in the record with the given id.
    virtual void setValue(const RecordInterface& rec, RecordFieldId id) = 0;
    
    
    // Factory Methods //
    
    // Creates and returns a ValuePanel subclass instance appropriate for the
    // given parameter.  Right now, it only returns a StandardValuePanel, but
    // in the future when specialized panels are used, this will be where the
    // correct type is transparently created and returned.
    static ValuePanel* create(String paramName,
                              const RecordInterface& paramRecord);
    
protected:
    // Parameter name.
    String m_paramName;
    
    // Parameter record.
    const RecordInterface& m_paramRecord;
    
    // Returns the allowed vector in m_paramRecord in a useful format.
    vector<String> allowedVector();
};


// Standard subclass of ValuePanel that controls widget(s) to allow the user
// to input a standard value.  StandardValuePanel can handle values of all
// "normal" types defined in the DataType.  StandardValuePanel can also handle
// values of type "any" (variant) by providing a combo box to allow the user
// to switch between value types.
class StandardValuePanel : public ValuePanel, Ui::ValuePanel {
    Q_OBJECT
    
public:
    // Constructor that takes the parameter name and specification record.
    StandardValuePanel(String paramName, const RecordInterface& paramRecord);
    
    // Destructor.
    ~StandardValuePanel();
    
    
    // ValuePanel methods
    
    // See ValuePanel::valueIsValid
    pair<bool, String> valueIsValid();
    
    // See ValuePanel::type
    DataType type();
    
    // See ValuePanel::getValue
    void getValue(String name, Record& record);
    
    // See ValuePanel::setValue
    void setValue(const RecordInterface& rec, RecordFieldId id);
    
signals:
    // Emitted when the user changes the value.
    void valueChanged();
    
private:    
    // Current type.
    DataType m_type;
    
    // Whether a value is a filename that must exist for it to be valid.
    bool m_mustExist;
    
    // Array widgets (for array types only).
    vector<QWidget*> m_aWidgets;
    
    // Record item panels (for records only).
    vector<RecordItemPanel*> m_rPanels;
    
    // Allowed values, or empty if N/A.
    vector<String> m_allowed;
    
    // Holds whether the value is a variant or not.
    bool m_isVariant;
    
    // Current int bounds.  The second element holds whether it has been set.
    pair<int, bool> m_intFrom, m_intTo;
    
    // Current double bounds. The second element holds whether it has been set.
    pair<double, bool> m_doubleFrom, m_doubleTo;

    
    // Set the type of this ValuePanel to the given.  May change the displayed
    // input widgets.
    void setType(DataType type);
    
    // Limit the chooseable types to the given, for variant values.  May not
    // have a noticeable effect if the type is incompatible.
    void limitTypes(vector<String> types);
    
    // Set the lower bound for inputted int values.  May not have a
    // noticeable effect if the type is incompatible.
    void setIntRangeFrom(int from);
    
    // Set the upper bound for inputted int values.  May not have a
    // noticeable effect if the type is incompatible.
    void setIntRangeTo(int to);
    
    // Set the lower bound for inputted double values.  May not have a
    // noticeable effect if the type is incompatible.
    void setDoubleRangeFrom(double from);
    
    // Set the upper bound for inputted double values.  May not have a
    // noticeable effect if the type is incompatible.
    void setDoubleRangeTo(double to);
    
    // Set the string value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setStringValue(String val);
    
    // Set the bool value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setBoolValue(bool val);
    
    // Set the double value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setDoubleValue(double val);
    
    // Set the int value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setIntValue(int val);
    
    // Set the complex value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setComplexValue(double val1, double val2);
    
    // Set the record value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setRecordValue(const RecordInterface& record);
    
    // Set the value to the given allowed value.  May not have a noticeable
    // effect if the type is incompatible.
    void setAllowedValue(String val);
    
    // Set the string array value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setStringArrayValue(const vector<String>& array);
    
    // Set the bool array value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setBoolArrayValue(const vector<bool>& array);
    
    // Set the double array value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setDoubleArrayValue(const vector<double>& array);
    
    // Set the int array value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setIntArrayValue(const vector<int>& array);
    
    // Set the complex array value to the given.  May not have a noticeable
    // effect if the type is incompatible.
    void setComplexArrayValue(const vector<DComplex>& array);    
    
    // Returns a new array widget based on the current type.  Used for adding
    // an item to an array list.
    QWidget* arrayWidget();
    
    // Holds the types supported by the variant panel.
    static vector<DataType> supportedTypes() {
        vector<DataType> v(11);
        v[0] = TpBool; v[1] = TpArrayBool;
        v[2] = TpInt; v[3] = TpArrayInt;
        v[4] = TpDouble; v[5] = TpArrayDouble;
        v[6] = TpString; v[7] = TpArrayString;
        v[8] = TpDComplex; v[9] = TpArrayDComplex;
        v[10] = TpRecord;
        return v;
    }
    
private slots:
    // Slot for when the user chooses a different type (for variants).
    void typeChanged(QString newType);

    // Decrease array widgets by one.
    void lessArray();
    
    // Increase array widgets by one.
    void moreArray();
    
    // Decrease record items by one.
    void lessRecord();
    
    // Increase record items by one.
    void moreRecord();
    
    // Show a file dialog and put the result in the string value.
    void browse();
    
    // Connecting the various widgets to emit the valueChanged signal.
    void changedValue() { emit valueChanged(); }
};


// Panel to hold one item in a record (i.e., a key and a value).  A
// RecordItemPanel basically consists of a QLineEdit and a StandardValuePanel
// set to be a variant.
class RecordItemPanel : public QHBoxLayout {
    Q_OBJECT
    
public:
    // Default constructor.
    RecordItemPanel(String paramName);
    
    // Destructor.
    ~RecordItemPanel();
    
    // Returns the current key name entered.
    String name();
    
    // Returns the value panel used.
    StandardValuePanel* value();
    
    // Insert the key/value pair into the given record.
    void getValue(Record& record);
    
signals:
    // Emitted when the user changes the value.
    void valueChanged();
    
private:
    // Key line edit.
    QLineEdit* m_name;
    
    // Value panel.
    StandardValuePanel* m_value;
    
private slots:
    // Connecting the various widgets to emit the valueChanged signal.
    void changedValue() { emit valueChanged(); }
};

}

#endif /*VALUEPANEL_QO_H_*/
