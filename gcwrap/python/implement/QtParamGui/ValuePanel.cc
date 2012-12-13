//# ValuePanel.cc: Panel for inputing a data value.
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
#include <casaqt/QtParamGui/QtParamGUI.qo.h>
#include <casaqt/QtParamGui/ValuePanel.qo.h>

#include <QCheckBox>
#include <QFileDialog>

#include <limits>

#include <casa/Arrays/Vector.h>

namespace casa {

// VALUEPANEL DEFINITIONS //

ValuePanel::ValuePanel(String paramName, const RecordInterface& paramRecord) :
    QWidget(), m_paramName(paramName), m_paramRecord(paramRecord) { }

ValuePanel::~ValuePanel() { }


ValuePanel* ValuePanel::create(String pname, const RecordInterface& prec) {
    return new StandardValuePanel(pname, prec);
}

vector<String> ValuePanel::allowedVector() {
    vector<String> allowed;
    String a = QtParamGUI::ALLOWED;
    if(m_paramRecord.isDefined(a)) {
        switch(m_paramRecord.dataType(a)) {
        case TpArrayUChar: case TpArrayChar: {
            const Array<uChar>& arr = m_paramRecord.asArrayuChar(a);
            Array<uChar>::ConstIteratorSTL it = arr.begin();
            for(; it != arr.end(); it++)
                allowed.push_back(String::toString(*it));
            break; }
        case TpArrayShort: case TpArrayUShort: {
            const Array<short>& arr = m_paramRecord.asArrayShort(a);
            Array<short>::ConstIteratorSTL it = arr.begin();
            for(; it != arr.end(); it++)
                allowed.push_back(String::toString(*it));
            break; }
        case TpArrayInt: {
            const Array<int>& arr = m_paramRecord.asArrayInt(a);
            Array<int>::ConstIteratorSTL it = arr.begin();
            for(; it != arr.end(); it++)
                allowed.push_back(String::toString(*it));
            break; }
        case TpArrayUInt: {
            const Array<uInt>& arr = m_paramRecord.asArrayuInt(a);
            Array<uInt>::ConstIteratorSTL it = arr.begin();
            for(; it != arr.end(); it++)
                allowed.push_back(String::toString(*it));
            break; }
        case TpArrayFloat: {
            const Array<float>& arr = m_paramRecord.asArrayFloat(a);
            Array<float>::ConstIteratorSTL it = arr.begin();
            for(; it != arr.end(); it++)
                allowed.push_back(String::toString(*it));
            break; }
        case TpArrayDouble: {
            const Array<double>& arr = m_paramRecord.asArrayDouble(a);
            Array<double>::ConstIteratorSTL it = arr.begin();
            for(; it != arr.end(); it++)
                allowed.push_back(String::toString(*it));
            break; }
        case TpArrayComplex: {
            const Array<Complex>& arr = m_paramRecord.asArrayComplex(a);
            Array<Complex>::ConstIteratorSTL it = arr.begin();
            stringstream ss;
            for(; it != arr.end(); it++) {
                ss.str("");
                ss << '(' << (*it).real() << ", " << (*it).imag() << ')';
                allowed.push_back(ss.str());
            }
            break; }
        case TpArrayDComplex: {
            const Array<DComplex>& arr = m_paramRecord.asArrayDComplex(a);
            Array<DComplex>::ConstIteratorSTL it = arr.begin();
            stringstream ss;
            for(; it != arr.end(); it++) {
                ss.str("");
                ss << '(' << (*it).real() << ", " << (*it).imag() << ')';
                allowed.push_back(ss.str());
            }
            break; }
        case TpArrayString: {
            const Array<String>& arr = m_paramRecord.asArrayString(a);
            Array<String>::ConstIteratorSTL it = arr.begin();
            for(; it != arr.end(); it++)
                allowed.push_back(*it);
            break; }
        
        default: break;
        }
    }
    return allowed;
}


// STANDARDVALUEPANEL DEFINITIONS //

// Constructors/Destructors //

StandardValuePanel::StandardValuePanel(String paramName,
        const RecordInterface& paramRecord) :
        casa::ValuePanel(paramName, paramRecord), m_mustExist(false),
        m_isVariant(false), m_intFrom(0, false), m_intTo(0, false),
        m_doubleFrom(0, false), m_doubleTo(0, false) {
    setupUi(this);
    
    String type = paramRecord.asString(QtParamGUI::TYPE);
    type.downcase();
    DataType t = QtParamGUI::type(type);
    m_isVariant = type == QtParamGUI::ANY;
    
    typeChooser->setVisible(m_isVariant);
    allowedFrame->setVisible(false);
    arrayFrame->setVisible(false);
    boolFrame->setVisible(false);
    doubleFrame->setVisible(false);
    intFrame->setVisible(false);
    complexFrame->setVisible(false);
    stringFrame->setVisible(false);
    stringBrowse->setVisible(false);
    recordFrame->setVisible(false);
    
    if(m_isVariant) {
        vector<DataType> types = supportedTypes();
        for(unsigned int i = 0; i < types.size(); i++)
            typeChooser->addItem(QtParamGUI::type(types[i]).c_str());
        
        if(paramRecord.isDefined(QtParamGUI::LIMITTYPES)) {
            vector<String> t;
            const Array<String>& a= paramRecord.asArrayString(
                                    QtParamGUI::LIMITTYPES);
            Array<String>::ConstIteratorSTL it = a.begin();
            for(; it != a.end(); it++) t.push_back(*it);
            limitTypes(t);
        }
        typeChanged(typeChooser->currentText());
        connect(typeChooser, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(typeChanged(QString)));
        connect(typeChooser, SIGNAL(currentIndexChanged(int)),
                this, SLOT(changedValue()));
        
    } else if(paramRecord.isDefined(QtParamGUI::ALLOWED) &&
              t != TpRecord && t != TpBool && t != TpArrayBool) {
        m_allowed = allowedVector();        
        m_type = t;
        allowedFrame->setVisible(!isArray(t));
        arrayFrame->setVisible(isArray(t));

        if(!isArray(t))
            for(unsigned int i = 0; i < m_allowed.size(); i++)
                allowedChooser->addItem(m_allowed[i].c_str());
        
    } else setType(t);
    
    intSpinner->setMinimum(numeric_limits<int>::min());
    intSpinner->setMaximum(numeric_limits<int>::max());
    doubleSpinner->setMinimum(numeric_limits<double>::min());
    doubleSpinner->setMaximum(numeric_limits<double>::max());
    complexSpinner1->setMinimum(numeric_limits<double>::min());
    complexSpinner1->setMaximum(numeric_limits<double>::max());
    complexSpinner2->setMinimum(numeric_limits<double>::min());
    complexSpinner2->setMaximum(numeric_limits<double>::max());
    
    QHBoxLayout* l = new QHBoxLayout(arrayItems);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(3);
    
    QVBoxLayout* lr = new QVBoxLayout(recordItemsFrame);
    lr->setContentsMargins(0, 0, 0, 0);
    lr->setSpacing(3);
    
    if(paramRecord.isDefined(QtParamGUI::RANGE)) {
        const RecordInterface& r = paramRecord.asRecord(QtParamGUI::RANGE);
        
        if(r.isDefined(QtParamGUI::RANGE_MIN)) {
            const RecordInterface& min = r.asRecord(QtParamGUI::RANGE_MIN);            
            IPosition shape = min.shape(QtParamGUI::VALUE);
            
            bool valid = shape.size() > 0 || !isArray(t);
            for(unsigned int i = 0; i < shape.size() && valid; i++)
                if(shape[i] == 0) valid = false;

            if(valid) {
                if(t == TpInt || t == TpUInt || t == TpShort ||
                        t == TpUShort || t == TpArrayInt || t == TpArrayUInt ||
                        t == TpArrayShort || t == TpArrayUShort || t== TpOther)
                    setIntRangeFrom((int)(*(min.asArrayDouble(
                                           QtParamGUI::VALUE).begin())));
                if(t == TpDouble || t == TpFloat || t == TpArrayDouble ||
                        t == TpArrayFloat || t == TpOther)
                    setDoubleRangeFrom(*(min.asArrayDouble(
                                           QtParamGUI::VALUE).begin()));
            }
        }
        
        if(r.isDefined(QtParamGUI::RANGE_MAX)) {
            const RecordInterface& max = r.asRecord(QtParamGUI::RANGE_MAX);
            IPosition shape = max.shape(QtParamGUI::VALUE);
            
            bool valid = shape.size() > 0;
            for(unsigned int i = 0; i < shape.size() && valid; i++)
                if(shape[i] == 0) valid = false;

            if(valid) {
                t = max.dataType(QtParamGUI::VALUE);
                if(t == TpArrayDouble)
                    setDoubleRangeTo(*(max.asArrayDouble(
                                           QtParamGUI::VALUE).begin()));
                else if(t == TpArrayFloat)
                    setDoubleRangeTo(*(max.asArrayFloat(
                                           QtParamGUI::VALUE).begin()));
                else if(t == TpArrayInt)
                    setIntRangeTo(*(max.asArrayInt(
                                          QtParamGUI::VALUE).begin()));
                else if(t == TpArrayUInt)
                    setIntRangeTo((int)(*(max.asArrayuInt(
                                          QtParamGUI::VALUE).begin())));
                else if(t == TpArrayShort || t == TpArrayUShort)
                    setIntRangeTo(*(max.asArrayShort(
                                          QtParamGUI::VALUE).begin()));
            }
        }
    }
    
    if(paramRecord.isDefined(QtParamGUI::MUSTEXIST))
        m_mustExist = paramRecord.asBool(QtParamGUI::MUSTEXIST);
    
    connect(stringEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(changedValue()));
    connect(boolTrue, SIGNAL(clicked()), this, SLOT(changedValue()));
    connect(boolFalse, SIGNAL(clicked()), this, SLOT(changedValue()));
    connect(doubleSpinner, SIGNAL(valueChanged(double)),
            this, SLOT(changedValue()));
    connect(intSpinner, SIGNAL(valueChanged(int)), this, SLOT(changedValue()));
    connect(complexSpinner1, SIGNAL(valueChanged(double)),
            this, SLOT(changedValue()));
    connect(complexSpinner2, SIGNAL(valueChanged(double)),
            this, SLOT(changedValue()));
    connect(allowedChooser, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changedValue()));
    connect(stringBrowse, SIGNAL(clicked()), this, SLOT(browse()));
    connect(arrayLess, SIGNAL(clicked()), this, SLOT(lessArray()));
    connect(arrayMore, SIGNAL(clicked()), this, SLOT(moreArray()));
    connect(recordLessButton, SIGNAL(clicked()), this, SLOT(lessRecord()));
    connect(recordMoreButton, SIGNAL(clicked()), this, SLOT(moreRecord()));
}

StandardValuePanel::~StandardValuePanel() { }


// Public Methods //

pair<bool, String> StandardValuePanel::valueIsValid() {
    bool valid = true;
    String invalid;
    if(m_type == TpString) {
        if(m_mustExist) {
            // make sure the given file exists
            QString s = stringEdit->text();
            if(s.isEmpty()) {
                valid = false;
                invalid += m_paramName + ": no file entered!";
            } else if(!QFile::exists(s)) {
                valid = false;
                invalid += m_paramName + ": File '" + qPrintable(s) + "' does"
                          " not exist!";
            }
        }
    } else if(m_type == TpRecord) {
        vector<String> names;
        String name;
        bool found = false;
        pair<bool, String> recursiveResult;        
        for(unsigned int i = 0; i < m_rPanels.size() && !found; i++) {
            // check for unique name
            name = m_rPanels[i]->name();
            for(unsigned int j = 0; j < names.size() && !found; j++) {
                if(name == names[j]) {
                    found = true;
                    valid = false;
                    if(!invalid.empty()) invalid += '\n';
                    invalid += m_paramName + ": record must have unique keys!";
                }
            }
            if(!found) {
                names.push_back(name);
                // if the type is a record, check that one as well
                if(m_rPanels[i]->value()->type() == TpRecord) {
                    recursiveResult = m_rPanels[i]->value()->
                                      valueIsValid();
                    if(!recursiveResult.first) {
                        valid = false;
                        if(!invalid.empty()) invalid += '\n';
                        invalid += recursiveResult.second;
                    }
                }                    
            }
        }
    }
    return pair<bool, String>(valid, invalid);
}

DataType StandardValuePanel::type() { return m_type; }

void StandardValuePanel::getValue(String name, Record& record) {
    bool a = m_allowed.size() > 0;
    switch(m_type) {
    case TpBool: record.define(name, boolTrue->isChecked()); break;
    case TpChar: {
        String str = qPrintable(a ? allowedChooser->currentText() :
                                    stringEdit->text());
        if(str.size() > 0) record.define(name, (char)str[0]); break; }
    case TpUChar: {
        String str = qPrintable(a ? allowedChooser->currentText() :
                                    stringEdit->text());
        if(str.size() > 0) record.define(name, (uChar)str[0]); break; }
    case TpShort: {
        if(!a) record.define(name, (short)intSpinner->value());
        else {
            String str = qPrintable(allowedChooser->currentText());
            short v;
            sscanf(str.c_str(), "%hd", &v);
            record.define(name, v);
        } break; }
    case TpUShort: {
        if(!a) record.define(name, (unsigned short)intSpinner->value());
        else {
            String str = qPrintable(allowedChooser->currentText());
            unsigned short v;
            sscanf(str.c_str(), "%hu", &v);
            record.define(name, v);
        } break; }
    case TpInt: {
        if(!a) record.define(name, intSpinner->value());
        else {
            String str = qPrintable(allowedChooser->currentText());
            int v;
            sscanf(str.c_str(), "%d", &v);
            record.define(name, v);
        } break; }
    case TpUInt: {
        if(!a) record.define(name, (unsigned int)intSpinner->value());
        else {
            String str = qPrintable(allowedChooser->currentText());
            unsigned int v;
            sscanf(str.c_str(), "%u", &v);
            record.define(name, v);
        } break; }
    case TpFloat: {
        if(!a) record.define(name, (float)doubleSpinner->value());
        else {
            String str = qPrintable(allowedChooser->currentText());
            float v;
            sscanf(str.c_str(), "%f", &v);
            record.define(name, v);
        } break; }
    case TpDouble: {
        if(!a) record.define(name, doubleSpinner->value());
        else {
            String str = qPrintable(allowedChooser->currentText());
            double v;
            sscanf(str.c_str(), "%lf", &v);
            record.define(name, v);
        } break; }
    case TpComplex: {
        if(!a) record.define(name, Complex((float)complexSpinner1->value(),
                                           (float)complexSpinner2->value()));
        else {
            String str = qPrintable(allowedChooser->currentText());
            float v1, v2;
            sscanf(str.c_str(), "(%f, %f)", &v1, &v2);
            record.define(name, Complex(v1, v2));
        } break; }
    case TpDComplex: {
        if(!a) record.define(name, DComplex(complexSpinner1->value(),
                                            complexSpinner2->value()));
        else {
            String str = qPrintable(allowedChooser->currentText());
            double v1, v2;
            sscanf(str.c_str(), "(%lf, %lf)", &v1, &v2);
            record.define(name, DComplex(v1, v2));
        } break; }
    case TpString: {
        if(!a) record.define(name, qPrintable(stringEdit->text()));
        else record.define(name, qPrintable(allowedChooser->currentText()));
        break; }
    case TpRecord: {
        Record r(Record::Variable);
        for(unsigned int i = 0; i < m_rPanels.size(); i++)
            m_rPanels[i]->getValue(r);
        record.defineRecord(name, r);
        break; }
    
    case TpArrayBool: {
        vector<Bool> v;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++)
            v.push_back(((QCheckBox*)m_aWidgets[i])->isChecked());
        record.define(name, Vector<Bool>(v));
        break; }
    
    case TpArrayChar: case TpArrayUChar: {
        vector<uChar> v;
        String str;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            str = qPrintable(a ? ((QComboBox*)m_aWidgets[i])->currentText() :
                                 ((QLineEdit*)m_aWidgets[i])->text());
            if(str.size() > 0) v.push_back((uChar)str[0]);
        }
        record.define(name, Vector<uChar>(v));
        break; }
    
    case TpArrayShort: case TpArrayUShort: {
        vector<Short> v;
        short s;
        String str;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            if(!a) s = (short)((QSpinBox*)m_aWidgets[i])->value();
            else {
                str = qPrintable(((QComboBox*)m_aWidgets[i])->currentText());
                sscanf(str.c_str(), "%hd", &s);
            }
            v.push_back(s);
        }
        record.define(name, Vector<Short>(v));
        break; }
    
    case TpArrayInt: {
        vector<Int> v;
        int s;
        String str;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            if(!a) s = ((QSpinBox*)m_aWidgets[i])->value();
            else {
                str = qPrintable(((QComboBox*)m_aWidgets[i])->currentText());
                sscanf(str.c_str(), "%d", &s);
            }
            v.push_back(s);
        }
        record.define(name, Vector<Int>(v));
        break; }
    
    case TpArrayUInt: {
        vector<uInt> v;
        unsigned int s;
        String str;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            if(!a) s = (unsigned int)((QSpinBox*)m_aWidgets[i])->value();
            else {
                str = qPrintable(((QComboBox*)m_aWidgets[i])->currentText());
                sscanf(str.c_str(), "%u", &s);
            }
            v.push_back(s);
        }
        record.define(name, Vector<uInt>(v));
        break; }
    
    case TpArrayFloat: {
        vector<Float> v;
        float s;
        String str;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            if(!a) s = (float)((QDoubleSpinBox*)m_aWidgets[i])->value();
            else {
                str = qPrintable(((QComboBox*)m_aWidgets[i])->currentText());
                sscanf(str.c_str(), "%f", &s);
            }
            v.push_back(s);
        }
        record.define(name, Vector<Float>(v));
        break; }
    
    case TpArrayDouble: {
        vector<Double> v;
        double s;
        String str;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            if(!a) s = ((QDoubleSpinBox*)m_aWidgets[i])->value();
            else {
                str = qPrintable(((QComboBox*)m_aWidgets[i])->currentText());
                sscanf(str.c_str(), "%lf", &s);
            }
            v.push_back(s);
        }
        record.define(name, Vector<Double>(v));
        break; }
    
    case TpArrayComplex: {
        vector<Complex> v;
        float v1, v2;
        String str;
        QHBoxLayout* l;
        QWidget* w1, *w2;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            if(!a) {
                l = (QHBoxLayout*)((QFrame*)m_aWidgets[i])->layout();
                if(l->count() < 5) continue; // shouldn't happen
                w1 = l->itemAt(1)->widget();
                w2 = l->itemAt(3)->widget();
                if(w1 == NULL || w2 == NULL) continue; // shouldn't happen
                v1 = (float)dynamic_cast<QDoubleSpinBox*>(w1)->value();
                v2 = (float)dynamic_cast<QDoubleSpinBox*>(w2)->value();
                
            } else {
                str = qPrintable(((QComboBox*)m_aWidgets[i])->currentText());
                sscanf(str.c_str(), "(%f, %f)", &v1, &v2);
            }
            v.push_back(Complex(v1, v2));
        }
        record.define(name, Vector<Complex>(v));
        break; }
    
    case TpArrayDComplex: {
        vector<DComplex> v;
        double v1, v2;
        String str;
        QHBoxLayout* l;
        QWidget* w1, *w2;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            if(!a) {
                l = (QHBoxLayout*)((QFrame*)m_aWidgets[i])->layout();
                if(l->count() < 5) continue; // shouldn't happen
                w1 = l->itemAt(1)->widget();
                w2 = l->itemAt(3)->widget();
                if(w1 == NULL || w2 == NULL) continue; // shouldn't happen
                v1 = dynamic_cast<QDoubleSpinBox*>(w1)->value();
                v2 = dynamic_cast<QDoubleSpinBox*>(w2)->value();
                
            } else {
                str = qPrintable(((QComboBox*)m_aWidgets[i])->currentText());
                sscanf(str.c_str(), "(%lf, %lf)", &v1, &v2);
            }
            v.push_back(DComplex(v1, v2));
        }
        record.define(name, Vector<DComplex>(v));
        break; }
    
    case TpArrayString: {
        vector<String> v;
        String str;
        for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
            str = qPrintable(a ? ((QComboBox*)m_aWidgets[i])->currentText():
                                 ((QLineEdit*)m_aWidgets[i])->text());
            v.push_back(str);
        }
        record.define(name, Vector<String>(v));
        break; }
    
    default: break;
    }
}

void StandardValuePanel::setValue(const RecordInterface& r, RecordFieldId id) {
    DataType t = r.dataType(id);

    if(t == TpBool) setBoolValue(r.asBool(id));
    else if(t == TpFloat) setDoubleValue(r.asFloat(id));
    else if(t==TpDouble) setDoubleValue(r.asDouble(id));
    else if(t == TpShort || t == TpUShort) setIntValue(r.asShort(id));
    else if(t == TpInt) setIntValue(r.asInt(id));
    else if(t == TpUInt) setIntValue(r.asuInt(id));
    else if(t == TpComplex) {
        Complex c = r.asComplex(id);
        setComplexValue(c.real(), c.imag());
    } else if(t == TpDComplex) {
        DComplex c = r.asDComplex(id);
        setComplexValue(c.real(), c.imag());
    } else if(t == TpChar || t == TpUChar)
        setStringValue(String(r.asuChar(id)));
    else if(t == TpString) setStringValue(r.asString(id));
    else if(t == TpRecord) setRecordValue(r.asRecord(id));
    else if(t == TpArrayBool) {
        const Array<Bool>& array = r.asArrayBool(id);
        vector<bool> vector;
        Array<Bool>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(*it);
        setBoolArrayValue(vector);
    } else if(t == TpArrayChar || t == TpArrayUChar) {
        const Array<uChar>& array = r.asArrayuChar(id);
        vector<String> vector;
        Array<uChar>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(String(*it));
        setStringArrayValue(vector);
    } else if(t == TpArrayShort || t == TpArrayUShort) {
        const Array<short>& array = r.asArrayShort(id);
        vector<int> vector;
        Array<short>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(*it);
        setIntArrayValue(vector);
    } else if(t == TpArrayInt) {
        const Array<Int>& array = r.asArrayInt(id);
        vector<int> vector;
        Array<Int>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(*it);
        setIntArrayValue(vector);
    } else if(t == TpArrayUInt) {
        const Array<uInt>& array = r.asArrayuInt(id);
        vector<int> vector;
        Array<uInt>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(static_cast<int>(*it));
        setIntArrayValue(vector);
    } else if(t == TpArrayFloat) {
        const Array<Float>& array = r.asArrayFloat(id);
        vector<double> vector;
        Array<Float>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(*it);
        setDoubleArrayValue(vector);
    } else if(t == TpArrayDouble) {
        const Array<Double>& array = r.asArrayDouble(id);
        
        // special case, since when resetting to last value "[]" always
        // turns into empty double array
        if(array.nelements() == 0) {
            switch(m_type) {
            case TpArrayBool:
                setBoolArrayValue(vector<bool>()); break;
            case TpArrayChar: case TpArrayUChar: case TpArrayString:
                setStringArrayValue(vector<String>()); break;
            case TpArrayShort: case TpArrayUShort: case TpArrayInt:
            case TpArrayUInt:
                setIntArrayValue(vector<int>()); break;
            case TpArrayFloat: case TpArrayDouble:
                setDoubleArrayValue(vector<double>()); break;
            case TpArrayComplex: case TpArrayDComplex:
                setComplexArrayValue(vector<DComplex>()); break;
            default: break;
            }
        } else {        
            vector<double> vector;
            Array<Double>::ConstIteratorSTL it = array.begin();
            for(; it != array.end(); it++)
                vector.push_back(*it);
            setDoubleArrayValue(vector);
        }
    } else if(t == TpArrayComplex) {
        const Array<Complex>& array = r.asArrayComplex(id);
        vector<DComplex> vector;
        Array<Complex>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(DComplex((*it).real(), (*it).imag()));
        setComplexArrayValue(vector);
    } else if(t == TpArrayDComplex) {
        const Array<DComplex>& array = r.asArrayDComplex(id);
        vector<DComplex> vector;
        Array<DComplex>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(*it);
        setComplexArrayValue(vector);
    } else if(t == TpArrayString) {
        const Array<String>& array = r.asArrayString(id);
        vector<String> vector;
        Array<String>::ConstIteratorSTL it = array.begin();
        for(; it != array.end(); it++)
            vector.push_back(*it);
        setStringArrayValue(vector);
    }
}


// Private Methods //

void StandardValuePanel::setType(DataType type) {
    if(m_isVariant) {
        int i;
        for(i = 0; i < typeChooser->count(); i++) {
            if(String(qPrintable(typeChooser->itemText(i))) ==
               QtParamGUI::type(type)) {
                typeChooser->blockSignals(true);
                typeChooser->setCurrentIndex(i);
                typeChooser->blockSignals(false);
                break;
            }
        }
        if(i == typeChooser->count()) // shouldn't happen
            return;
    }
    
    m_type = type;
    
    bool b = type == TpBool;
    bool d = type == TpFloat || type == TpDouble;
    bool i = type == TpShort || type == TpInt;
    bool ui = type == TpUShort || type == TpUInt;
    bool c = type == TpComplex || type == TpDComplex;
    bool a = type == TpArrayBool || type == TpArrayChar || type == TpArrayUChar
             || type == TpArrayShort || type == TpArrayUShort ||
             type == TpArrayInt || type == TpArrayUInt || type == TpArrayFloat
             || type == TpArrayDouble || type == TpArrayComplex ||
             type == TpArrayDComplex || type == TpArrayString;
    bool r = type == TpRecord;
    bool s = !b && !d && !i && !c && !a && !r;
    
    boolFrame->setVisible(b);
    doubleFrame->setVisible(d);
    intFrame->setVisible(i || ui);
    complexFrame->setVisible(c);
    stringFrame->setVisible(s);
    arrayFrame->setVisible(a);
    recordFrame->setVisible(r);
    
    if(!m_intFrom.second && (ui || i)) {
        if(ui) intSpinner->setMinimum(0);
        else intSpinner->setMinimum(numeric_limits<int>::min());
    }
    
    if(s) {
        stringBrowse->setVisible(false);
        if(m_paramRecord.isDefined(QtParamGUI::KIND)) {
            String s = m_paramRecord.asString(QtParamGUI::KIND);
            s.downcase();
            if(s == QtParamGUI::KIND_MS || s == QtParamGUI::KIND_FILE ||
               s == QtParamGUI::KIND_TABLE)
                stringBrowse->setVisible(true);
        }
    }
}

void StandardValuePanel::limitTypes(vector<String> types) {
    QStringList t;
    vector<DataType> st = supportedTypes();
    for(unsigned int i = 0; i < types.size(); i++)
        for(unsigned int j = 0; j < st.size(); j++)
            if(types[i] == QtParamGUI::type(st[j]))
                t << types[i].c_str();
    
    if(!t.isEmpty()) {
        // clear out old values
        while(typeChooser->count() > 0) typeChooser->removeItem(0);
        typeChooser->addItems(t);
    }
}

void StandardValuePanel::setIntRangeFrom(int from) {
    intSpinner->setMinimum(from);
    m_intFrom = pair<int, bool>(from, true);
}
void StandardValuePanel::setIntRangeTo(int to) {
    intSpinner->setMaximum(to);
    m_intTo = pair<int, bool>(to, true);
}

void StandardValuePanel::setDoubleRangeFrom(double f) {
    doubleSpinner->setMinimum(f);
    m_doubleFrom = pair<double, bool>(f, true);
}

void StandardValuePanel::setDoubleRangeTo(double to) {
    doubleSpinner->setMaximum(to);
    m_doubleTo = pair<double, bool>(to, true);
}

void StandardValuePanel::setStringValue(String val) {
    if(m_allowed.size() == 0) stringEdit->setText(val.c_str());
    else setAllowedValue(val);
}

void StandardValuePanel::setBoolValue(bool val) {
    if(val) boolTrue->setChecked(true);
    else boolFalse->setChecked(true);
}

void StandardValuePanel::setDoubleValue(double val) {
    if(m_allowed.size() == 0) doubleSpinner->setValue(val);
    else setAllowedValue(String::toString(val));
}

void StandardValuePanel::setIntValue(int val) {
    if(m_allowed.size() == 0) intSpinner->setValue(val);
    else setAllowedValue(String::toString(val));
}

void StandardValuePanel::setComplexValue(double val1, double val2) {
    if(m_allowed.size() == 0) {
        complexSpinner1->setValue(val1);
        complexSpinner2->setValue(val2);
    } else {
        stringstream ss;
        ss << '(' << val1 << ", " << val2 << ')';
        setAllowedValue(ss.str());
    }
}

void StandardValuePanel::setRecordValue(const RecordInterface& record) {
    // clear old items
    while(m_rPanels.size() > 0) lessRecord();
    while(m_rPanels.size() < record.nfields()) moreRecord();

    // add new items
    DataType t;
    StandardValuePanel* p;
    for(unsigned int i = 0; i < record.nfields(); i++) {
        t = record.dataType(i);
        p = m_rPanels[i]->value();

        switch(t) {
        case TpBool:
            p->setType(t);
            p->setBoolValue(record.asBool(i));
            break;

        case TpInt:
            p->setType(t);
            p->setIntValue(record.asInt(i));
            break;

        case TpDouble:
            p->setType(t);
            p->setDoubleValue(record.asDouble(i));
            break;

        case TpDComplex: {
            p->setType(t);
            DComplex c = record.asDComplex(i);
            p->setComplexValue(c.real(), c.imag());
            break; }

        case TpString:
            p->setType(t);
            p->setStringValue(record.asString(i));
            break;

        case TpRecord:
            p->setType(t);
            p->setRecordValue(record.asRecord(i));
            break;

        case TpArrayBool: {
            const Array<Bool>& a = record.asArrayBool(i);
            vector<bool> v;
            Array<Bool>::ConstIteratorSTL it = a.begin();
            for(; it != a.end(); it++)
                v.push_back(*it);
            p->setType(t);
            p->setBoolArrayValue(v);
            break; }

        case TpArrayInt: {
            const Array<Int>& a = record.asArrayInt(i);
            vector<int> v;
            Array<Int>::ConstIteratorSTL it = a.begin();
            for(; it != a.end(); it++)
                v.push_back(*it);
            p->setType(t);
            p->setIntArrayValue(v);
            break; }

        case TpArrayDouble: {
            const Array<Double>& a = record.asArrayDouble(i);
            vector<double> v;
            Array<Double>::ConstIteratorSTL it = a.begin();
            for(; it != a.end(); it++)
                v.push_back(*it);
            p->setType(t);
            p->setDoubleArrayValue(v);
            break; }

        case TpArrayDComplex: {
            const Array<DComplex>& a = record.asArrayDComplex(i);
            vector<DComplex> v;
            Array<DComplex>::ConstIteratorSTL it = a.begin();
            for(; it != a.end(); it++)
                v.push_back(*it);
            p->setType(t);
            p->setComplexArrayValue(v);
            break; }

        case TpArrayString: {
            const Array<String>& a = record.asArrayString(i);
            vector<String> v;
            Array<String>::ConstIteratorSTL it = a.begin();
            for(; it != a.end(); it++)
                v.push_back(*it);
            p->setType(t);
            p->setStringArrayValue(v);
            break; }

        default: break;
        }
    }
}

void StandardValuePanel::setAllowedValue(String val) {
    for(int i = 0; i < allowedChooser->count(); i++) {
        if(String(qPrintable(allowedChooser->itemText(i))) == val) {
            allowedChooser->setCurrentIndex(i);
            break;
        }
    }
}

void StandardValuePanel::setStringArrayValue(const vector<String>& array) {
    if(m_type != TpArrayString && m_type != TpArrayChar &&
            m_type != TpArrayUChar) return;
    
    // resize array
    while(array.size() < m_aWidgets.size()) lessArray();
    while(array.size() > m_aWidgets.size()) moreArray();

    bool a = m_allowed.size() > 0;
    String s;
    for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
        s = array[i];
        if(!a) ((QLineEdit*)m_aWidgets[i])->setText(s.c_str());
        else {
            QComboBox* b = (QComboBox*)m_aWidgets[i];
            for(int i = 0; i < b->count(); i++) {
                if(String(qPrintable(b->itemText(i))) == s) {
                    b->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void StandardValuePanel::setBoolArrayValue(const vector<bool>& array) {
    if(m_type != TpArrayBool) return;
    
    // resize array
    while(array.size() < m_aWidgets.size()) lessArray();
    while(array.size() > m_aWidgets.size()) moreArray();

    bool a = m_allowed.size() > 0;
    bool v;
    for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
        v = array[i];
        if(!a) ((QCheckBox*)m_aWidgets[i])->setChecked(v);
        else {
            QComboBox* b = (QComboBox*)m_aWidgets[i];
            for(int i = 0; i < b->count(); i++) {
                if(String(qPrintable(b->itemText(i))) == String::toString(v)) {
                    b->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void StandardValuePanel::setDoubleArrayValue(const vector<double>& array) {
    if(m_type != TpArrayDouble && m_type != TpArrayFloat) return;
    
    // resize array
    while(array.size() < m_aWidgets.size()) lessArray();
    while(array.size() > m_aWidgets.size()) moreArray();

    bool a = m_allowed.size() > 0;
    double v;
    for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
        v = array[i];
        if(!a) ((QDoubleSpinBox*)m_aWidgets[i])->setValue(v);
        else {
            QComboBox* b = (QComboBox*)m_aWidgets[i];
            for(int i = 0; i < b->count(); i++) {
                if(String(qPrintable(b->itemText(i))) == String::toString(v)) {
                    b->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void StandardValuePanel::setIntArrayValue(const vector<int>& array) {
    if(m_type != TpArrayInt && m_type != TpArrayUInt &&
            m_type != TpArrayShort && m_type != TpArrayUShort) return;
    
    // resize array
    while(array.size() < m_aWidgets.size()) lessArray();
    while(array.size() > m_aWidgets.size()) moreArray();

    bool a = m_allowed.size() > 0;
    int v;
    for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
        v = array[i];
        if(!a) ((QSpinBox*)m_aWidgets[i])->setValue(v);
        else {
            QComboBox* b = (QComboBox*)m_aWidgets[i];
            for(int i = 0; i < b->count(); i++) {
                if(String(qPrintable(b->itemText(i))) == String::toString(v)) {
                    b->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void StandardValuePanel::setComplexArrayValue(const vector<DComplex>& array) {
    if(m_type != TpArrayComplex && m_type != TpArrayDComplex) return;
    
    // resize array
    while(array.size() < m_aWidgets.size()) lessArray();
    while(array.size() > m_aWidgets.size()) moreArray();

    bool a = m_allowed.size() > 0;
    DComplex v;
    stringstream ss;
    for(unsigned int i = 0; i < m_aWidgets.size(); i++) {
        v = array[i];
        if(!a) {
            QHBoxLayout* l = (QHBoxLayout*)((QFrame*)m_aWidgets[i])->layout();
            if(l->count() < 5) continue; // shouldn't happen

            QWidget* w1 = l->itemAt(1)->widget(),
            *w2 = l->itemAt(3)->widget();
            if(w1 != NULL && w2 != NULL) {
                QDoubleSpinBox* b1 = dynamic_cast<QDoubleSpinBox*>(w1),
                *b2 = dynamic_cast<QDoubleSpinBox*>(w2);
                if(b1 != NULL && b2 != NULL) {
                    b1->setValue(v.real());
                    b2->setValue(v.imag());
                }
            }

        } else {
            QComboBox* b = (QComboBox*)m_aWidgets[i];
            ss.str("(");
            ss << v.real() << ", " << v.imag() << ")";
            for(int i = 0; i < b->count(); i++) {
                if(String(qPrintable(b->itemText(i))) == String(ss.str())) {
                    b->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

QWidget* StandardValuePanel::arrayWidget() {
    if(m_allowed.size() > 0) {
        QComboBox* b = new QComboBox();
        for(unsigned int i = 0; i < m_allowed.size(); i++)
            b->addItem(m_allowed[i].c_str());
        connect(b, SIGNAL(currentIndexChanged(int)),
                this, SLOT(changedValue()));
        return b;
    } else if(m_type == TpArrayBool) {
        QCheckBox* b = new QCheckBox("");
        connect(b, SIGNAL(clicked()), this, SLOT(changedValue()));
        return b;
    } else if(m_type == TpArrayChar || m_type == TpArrayUChar) {
        QLineEdit* e = new QLineEdit();
        e->setMaximumWidth(30);
        e->setMaxLength(1);
        connect(e, SIGNAL(textChanged(const QString&)),
                this, SLOT(changedValue()));
        return e;
    } else if(m_type == TpArrayShort || m_type == TpArrayUShort ||
              m_type == TpArrayInt || m_type == TpArrayUInt) {
        QSpinBox* s = new QSpinBox();
        
        if(m_intFrom.second) s->setMinimum(m_intFrom.first);
        else if(m_type == TpArrayShort || m_type == TpArrayInt)
            s->setMinimum(numeric_limits<int>::min());
        else s->setMinimum(0);
        
        if(m_intTo.second) s->setMaximum(m_intTo.first);
        else s->setMaximum(numeric_limits<int>::max());
        
        connect(s, SIGNAL(valueChanged(int)), this, SLOT(changedValue()));
        return s;
    } else if(m_type == TpArrayFloat || m_type == TpArrayDouble) {
        QDoubleSpinBox* s = new QDoubleSpinBox();
        
        if(m_doubleFrom.second) s->setMinimum(m_doubleFrom.first);
        else s->setMinimum(numeric_limits<double>::min());
        
        if(m_doubleTo.second) s->setMaximum(m_doubleTo.first);
        else s->setMaximum(numeric_limits<double>::max());
        
        connect(s, SIGNAL(valueChanged(double)), this, SLOT(changedValue()));
        return s;
    } else if(m_type == TpArrayComplex || m_type == TpArrayDComplex) {
        QFrame* f = new QFrame();
        f->setFrameStyle(QFrame::NoFrame);
        QHBoxLayout* l = new QHBoxLayout(f);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);
        
        l->addWidget(new QLabel("("));
        QDoubleSpinBox* b1 = new QDoubleSpinBox(), *b2 = new QDoubleSpinBox();
        b1->setMinimum(numeric_limits<double>::min());
        b2->setMinimum(numeric_limits<double>::min());
        b1->setMaximum(numeric_limits<double>::max());
        b2->setMaximum(numeric_limits<double>::max());
        l->addWidget(b1);
        l->addWidget(new QLabel(","));
        l->addWidget(b2);
        l->addWidget(new QLabel(")"));
        connect(b1, SIGNAL(valueChanged(double)), this, SLOT(changedValue()));
        connect(b2, SIGNAL(valueChanged(double)), this, SLOT(changedValue()));

        return f;
    } else if(m_type == TpArrayString) {
        QLineEdit* e = new QLineEdit();
        e->setMinimumWidth(100);
        connect(e, SIGNAL(textChanged(const QString&)),
                this, SLOT(changedValue()));
        return e;
    } else return NULL;
}


// Private Slots //

void StandardValuePanel::typeChanged(QString newType) {
    setType(QtParamGUI::type(qPrintable(newType)));
}

void StandardValuePanel::lessArray() {
    if(m_aWidgets.size() > 0) {
        QWidget* w = m_aWidgets[m_aWidgets.size() - 1];
        arrayItems->layout()->removeWidget(w);
        delete w;
        m_aWidgets.erase(m_aWidgets.begin() + (m_aWidgets.size() - 1));
        
        if(m_aWidgets.size() == 0) arrayLess->setEnabled(false);
        changedValue();
    }
}

void StandardValuePanel::moreArray() {
    QWidget* w = arrayWidget();
    if(w == NULL) return;
    arrayItems->layout()->addWidget(w);
    m_aWidgets.push_back(w);
    arrayLess->setEnabled(true);
    changedValue();
}

void StandardValuePanel::lessRecord() {
    if(m_rPanels.size() > 0) {
        RecordItemPanel* p = m_rPanels[m_rPanels.size() - 1];
        recordItemsFrame->layout()->removeItem(p);
        delete p;
        m_rPanels.erase(m_rPanels.begin() + (m_rPanels.size() - 1));
        
        if(m_rPanels.size() == 0) recordLessButton->setEnabled(false);
        changedValue();
    }
}

void StandardValuePanel::moreRecord() {
    RecordItemPanel* p = new RecordItemPanel(m_paramName);
    ((QBoxLayout*)recordItemsFrame->layout())->addLayout(p);
    m_rPanels.push_back(p);
    recordLessButton->setEnabled(true);
    connect(p, SIGNAL(valueChanged()), this, SLOT(changedValue()));
    changedValue();
}

void StandardValuePanel::browse() {
    QFileDialog dialog(qobject_cast<QWidget*>(parent()));
    bool dir = false;
    if(m_paramRecord.isDefined(QtParamGUI::KIND)) {
        String s = m_paramRecord.asString(QtParamGUI::KIND);
        s.downcase();
        dir = (s == QtParamGUI::KIND_MS || s == QtParamGUI::KIND_TABLE);
    }
    dialog.setFileMode(dir? QFileDialog::Directory: QFileDialog::ExistingFile);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    if(dialog.exec()) {
        QStringList files = dialog.selectedFiles();
        if(files.size() > 0) {
            QString file = files[0];
            if(!file.isEmpty()) stringEdit->setText(file);
        }
    }
}


// RECORDPANEL DEFINITIONS //

RecordItemPanel::RecordItemPanel(String paramName) {
    setContentsMargins(0, 0, 0, 0);
    setSpacing(3);
    
    m_name = new QLineEdit();
    addWidget(m_name);
    
    // all the value panel needs to know is the type
    Record pRec(Record::Variable);
    pRec.define(QtParamGUI::TYPE, QtParamGUI::ANY);
    
    m_value = new StandardValuePanel(paramName, pRec);
    addWidget(m_value);
    
    connect(m_value, SIGNAL(valueChanged()), this, SLOT(changedValue()));
    connect(m_name, SIGNAL(textChanged(const QString&)),
            this, SLOT(changedValue()));
}

RecordItemPanel::~RecordItemPanel() { }


String RecordItemPanel::name() { return String(qPrintable(m_name->text())); }

StandardValuePanel* RecordItemPanel::value() { return m_value; }

void RecordItemPanel::getValue(Record& record) {
    String name = qPrintable(m_name->text());
    if(!record.isDefined(name)) m_value->getValue(name, record);
}

}
