//# QtParamGUI.cc: GUI for collecting user input for task parameters.
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
#include <casaqt/QtParamGui/ParamPanel.qo.h>

#include <casa/Arrays/Vector.h>

#include <QMessageBox>
#include <QDir>

namespace casa {

// QTPARAMGUI DEFINITIONS //

// Constructors/Destructors //

QtParamGUI::QtParamGUI(const Record& r, Mode m, PyObject* ipythonShell,
        const Record* globals, QWidget* parent) : QDialog(parent), m_tasks(r),
        m_resetMode(DEFAULT), m_mode(m), m_descDialog(NULL),
        m_exampleDialog(NULL), m_python(ipythonShell) {
    setupUi(this);

    connect(resetGlobals, SIGNAL(toggled(bool)), this, SLOT(resetChanged()));
    connect(resetDefaults, SIGNAL(toggled(bool)), this, SLOT(resetChanged()));
    connect(resetLast, SIGNAL(toggled(bool)), this, SLOT(resetChanged()));
    connect(lastLoadButton, SIGNAL(clicked()), this, SLOT(loadLast()));
    
    if(globals != NULL) {
        m_globals = *globals;
        m_resetMode = GLOBAL;
    }
    showHideResets(globals != NULL, false);
    
    if(r.nfields() < 2) {
        taskLabel->close();
        taskChooser->close();
        taskLine->close();
    } else {
        QStringList tasks;
        for(unsigned int i = 0; i < r.nfields(); i++)
            tasks << r.name(i).c_str();
        taskChooser->addItems(tasks);
    }
    setRecord(m_tasks, 0);
    connect(taskChooser, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(taskChosen(QString)));
    if(ipythonShell != NULL) loggerBox->setVisible(false);
    if(m != PYTHON) closeOnRunBox->setVisible(false);
    
    connect(runButton, SIGNAL(clicked()), this, SLOT(run()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
    
    setVisible(true);
    
    if(globals != NULL) resetGlobals->setChecked(true);
}

QtParamGUI::~QtParamGUI() {
    if(m_descDialog) delete m_descDialog;
    if(m_exampleDialog) delete m_exampleDialog;
}

String QtParamGUI::taskName() const { return m_taskName; }

Record QtParamGUI::taskParameters() const {
    if(m_tasks.isDefined(m_taskName) &&
       m_tasks.asRecord(m_taskName).isDefined(PARAMETERS))
        return m_tasks.asRecord(m_taskName).asRecord(PARAMETERS);
    else return Record();
}

Record QtParamGUI::taskConstraints() const {
    if(m_tasks.isDefined(m_taskName) &&
       m_tasks.asRecord(m_taskName).isDefined(CONSTRAINTS))
        return m_tasks.asRecord(m_taskName).asRecord(CONSTRAINTS);
    else return Record();
}

Record QtParamGUI::enteredParameters() const {
    Record r(Record::Variable);
    for(unsigned int i = 0; i < m_panels.size(); i++)
        m_panels[i]->getValue(r);
    return r;
}


// Private Methods //

void QtParamGUI::setRecord(const Record& record, RecordFieldId id) {
    // check for valid structure of record
    bool valid = id.byName() ? record.isDefined(id.fieldName()) :
                               (int)record.nfields() > id.fieldNumber();
    String invalidStr = "Record does not have that ID defined!";
    
    if(valid) {
        pair<bool, String> v = recordIsValid(record, id);
        valid = v.first;
        invalidStr = v.second;
    }

    if(!m_taskName.empty()) {
        String t = m_taskName;
        t += ": ";
        t += qPrintable(windowTitle());
        setWindowTitle((m_taskName + ": Input Parameters").c_str());

        nameLabel->setText(m_taskName.c_str());
    }

    if(valid) {
        const RecordInterface& r = record.asRecord(id);

        String desc;
        if(r.isDefined(DESCRIPTION)) {
            String str = r.asString(DESCRIPTION);
            String str2 = strTrim(str);
            if(!str2.empty()) {
                desc = strNLtrim(str);
                replaceSlashQuotes(desc);
            }
        }

        String sDesc;
        if(r.isDefined(SHORT_DESCRIPTION)) {
            String str = r.asString(SHORT_DESCRIPTION);
            String str2 = strTrim(str);
            if(!str2.empty()) {
                sDesc = strNLtrim(str);
                replaceSlashQuotes(sDesc);
            }
        }

        String example;
        if(r.isDefined(EXAMPLE)) {
            String str = r.asString(EXAMPLE);
            String str2 = strTrim(str);
            if(!str2.empty()) {
                example = strNLtrim(str);
                replaceSlashQuotes(example);
            }
        }

        // short description
        shortDescLabel->setVisible(!sDesc.empty());
        if(!sDesc.empty()) {
            if(sDesc.length() > MAX_SHORTDESC_LENGTH) {
                shortDescLabel->setWordWrap(true);
                shortDescLabel->setMinimumWidth(400);
            }
            shortDescLabel->setText(sDesc.c_str());
        }

        // description
        descButton->setVisible(!desc.empty());
        if(!desc.empty()) {
            connect(descButton, SIGNAL(clicked(bool)),
                    this, SLOT(showHideDesc(bool)));
            m_descDialog = new QDialog(this);
            setupDialog(m_descDialog, m_taskName, "Description", desc);
            connect(m_descDialog, SIGNAL(rejected()),
                    this, SLOT(descClosed()));
            connect(this, SIGNAL(finished(int)), m_descDialog, SLOT(close()));
        }

        // example
        exampleButton->setVisible(!example.empty());
        if(!example.empty()) {
            connect(exampleButton, SIGNAL(clicked(bool)),
                    this, SLOT(showHideExample(bool)));
            m_exampleDialog = new QDialog(this);
            setupDialog(m_exampleDialog, m_taskName, "Example", example);
            connect(m_exampleDialog, SIGNAL(rejected()),
                    this, SLOT(exampleClosed()));
            connect(this, SIGNAL(finished(int)),
                    m_exampleDialog, SLOT(close()));
        }
        
        // parameter order
        vector<String> order;
        if(r.isDefined(PARAMETER_ORDER)) {
            const Array<String>& a = r.asArrayString(PARAMETER_ORDER);
            Array<String>::ConstIteratorSTL it = a.begin();
            for(; it != a.end(); it++)
                order.push_back(*it);
        } else {
            for(unsigned int i = 0; i < r.asRecord(PARAMETERS).nfields(); i++)
                order.push_back(r.asRecord(PARAMETERS).name(i));
        }

        // remove subparameters from the order, as they will be dealt with
        // separately
        for(unsigned int i = 0; i < r.asRecord(PARAMETERS).nfields(); i++) {
            if(r.asRecord(PARAMETERS).asRecord(i).isDefined(SUBPARAMETERS)) {
                const Array<String>& a = r.asRecord(PARAMETERS).asRecord(
                                         i).asArrayString(SUBPARAMETERS);
                Array<String>::ConstIteratorSTL it = a.begin();
                for(; it != a.end(); it++) {
                    unsigned int j = 0;
                    for(; j < order.size(); j++) {
                        if(*it == order[j])
                            break;
                    }
                    if(j != order.size()) order.erase(order.begin() + j);
                }
            }
        }
        
        // check for a <taskname>.last file in the current directory
        m_last = Record();
        QFile lastFile(QDir::current().absolutePath() + QString("/") +
                       QString(m_taskName.c_str()) + QString(".last"));
        lastLoadButton->setVisible(lastFile.exists());
        lastLoadButton->setToolTip(lastFile.fileName());
        resetLast->setVisible(false);
        
        setupRecord(order);
        runButton->setEnabled(true);
        resetButton->setEnabled(true);    
    } else {
        QMessageBox::critical(this, "Parameter GUI Initialization",
                invalidStr.c_str());
        runButton->setEnabled(false);
        resetButton->setEnabled(false);
        
        exampleButton->setVisible(false);
        descButton->setVisible(false);
        shortDescLabel->setVisible(false);
        
        QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(frame->layout());
        if(layout != NULL) {
            ParamPanel* p;
            for(unsigned int i = 0; i < m_panels.size(); i++) {
                p = m_panels[i];
                layout->removeWidget(p);
                delete p;
            }
            m_panels.clear();
        }
    }
}

pair<bool, String> QtParamGUI::recordIsValid(const Record& record,
        RecordFieldId id) {
    bool valid = true;
    stringstream invalid;
    invalid << "Given record has an invalid structure:";
    
    if(record.nfields() == 0 ||
       (id.byName() ? !record.isDefined(id.fieldName()) :
                      (int)record.nfields() <= id.fieldNumber())) {
        invalid << "\n* Record has no fields!";
        return pair<bool, String>(false, invalid.str());
    }

    m_taskName = record.name(id);
    if(record.dataType(id) != TpRecord) {
        invalid << "\n* Task is not in a Record!";
        return pair<bool, String>(false, invalid.str());
    }

    const RecordInterface& taskRec = record.asRecord(id);

    // PARAMETERS
    if(taskRec.isDefined(PARAMETERS)) {
        if(taskRec.dataType(PARAMETERS) == TpRecord) {
            const RecordInterface& pRec = taskRec.asRecord(PARAMETERS);                    
            String name;
            DataType t;
            bool any;

            for(unsigned int i = 0; i < pRec.nfields(); i++) {
                name = pRec.name(i);
                any = false;
                t = TpOther;
                if(pRec.dataType(i) == TpRecord) {
                    const RecordInterface& r = pRec.asRecord(i);

                    // at least "type" must be present and valid
                    if(r.isDefined(TYPE) && r.dataType(TYPE) == TpString) {
                        t = type(r.asString(TYPE));
                        any = r.asString(TYPE) == ANY;
                        if(any) t = TpOther;
                        if(t == TpOther && !any) {
                            invalid << "\n* Parameter '" << name << "' given ";
                            invalid << "an invalid type.";
                            valid = false;
                        }                        
                    } else {
                        invalid << "\n* Parameter '" << name << "' does not ";
                        invalid << "have a type!";
                        valid = false;
                    }
                    
                    // "limittypes", if given, is a String array for an "any"
                    if(r.isDefined(LIMITTYPES)) {
                        if(r.dataType(LIMITTYPES) != TpArrayString || !any) {
                            invalid << "\n* Limittypes for '" << name << "' ";
                            invalid << "must be a string array for an 'any' ";
                            invalid << "type.";
                            valid = false;
                        }
                    }
                    
                    // "value", if given, must match the type
                    if(r.isDefined(VALUE)) {
                        if(t != TpOther) {
                            if(r.dataType(VALUE) != t) {
                                invalid << "\n* Given value for '" << name;
                                invalid << "' is not the correct type.";
                                valid = false;
                            }
                        } else if(r.isDefined(LIMITTYPES) && valid) {
                            const Array<String>& a = r.asArrayString(
                                                     LIMITTYPES);
                            Array<String>::ConstIteratorSTL it = a.begin();
                            bool found = false;
                            for(; it != a.end(); it++) {
                                if(r.dataType(VALUE) == type(*it)) {
                                    found = true;
                                    break;
                                }
                            }
                            if(!found) {
                                invalid << "\n* Given value for '" << name;
                                invalid << "' is not the correct type.";
                                valid = false;
                            }
                        }
                    }

                    // "description, if given, must be a String
                    if(r.isDefined(DESCRIPTION)) {
                        if(r.dataType(DESCRIPTION) != TpString) {
                            invalid << "\n* Description for '" << name << "' ";
                            invalid << "is not a String!";
                            valid = false;
                        }
                    }

                    // "mustexist", if given, must be a bool
                    if(r.isDefined(MUSTEXIST)) {
                        if(r.dataType(MUSTEXIST) != TpBool) {
                            invalid << "\n* Mustexist for '" << name << "' is";
                            invalid << " not a bool!";
                            valid = false;
                        }
                    }

                    // "allowed", if given, is an array of the right type with
                    // at least one element.  no "allowed" for any
                    if(r.isDefined(ALLOWED)) {
                        if(any) {
                            invalid<< "\n* Parameter '" << name << "' of type";
                            invalid << "'any' cannot have 'allowed' values.";
                            valid = false;
                        } else if(r.dataType(ALLOWED) != asArray(t)) {
                            invalid << "\n* 'Allowed' values for '" << name;
                            invalid << "' not of correct type.";
                            valid = false;
                        } else {
                            IPosition s = r.shape(ALLOWED);
                            if(s.size() == 0) {
                                invalid << "\n* 'Allowed' vector for'" << name;
                                invalid << "' is empty.";
                                valid = false;
                            } else {
                                for(unsigned int j = 0; j < s.size(); j++) {
                                    if(s[j] == 0) {
                                        invalid<< "\n* 'Allowed' vector for '";
                                        invalid << name <<"' is empty.";
                                        valid = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    // "kind", if given, is a String.  no "kind" for any
                    if(r.isDefined(KIND)) {
                        if(any) {
                            invalid<< "\n* Parameter '" << name << "' of type";
                            invalid << "'any' cannot have 'kind' defined.";
                            valid = false;
                        } else if(r.dataType(KIND) != TpString) {
                            invalid << "\n* 'Kind' for '" << name << "' is ";
                            invalid << "not a String.";
                            valid = false;
                        }
                    }

                    // "units", if given, is a String
                    if(r.isDefined(UNITS)) {
                        if(r.dataType(UNITS) != TpString) {
                            invalid << "\n* 'Units' for '" << name << "' is ";
                            invalid << "not a String.";
                            valid = false;
                        }
                    }
                    
                    // "example", if given, is a String array
                    if(r.isDefined(EXAMPLE)) {
                        if(r.dataType(EXAMPLE) != TpArrayString) {
                            invalid << "\n* 'Example' for '" << name << "' is";
                            invalid << " not a String array.";
                            valid = false;
                        }
                    }
                    
                    // "subparameters", if given, is a String array
                    if(r.isDefined(SUBPARAMETERS)) {
                        if(r.dataType(SUBPARAMETERS) != TpArrayString) {
                            invalid << "\n* 'Subparameters' for '" << name;
                            invalid << "' is not a String array.";
                            valid = false;
                        } else if(r.asArrayString(SUBPARAMETERS).nelements()
                                  == 0) {
                            invalid << "\n* 'Subparameters' for '" << name;
                            invalid << "' cannot be empty!";
                            valid = false;
                        }
                    }
                    
                    // "range", if given, has the correct structure and is for
                    // a numeric type.  if "min" or ",ax" is defined the value
                    // is a doulbe array and the value array has at least one
                    // element
                    if(r.isDefined(RANGE)) {
                        if(t != TpDouble && t != TpFloat && t != TpInt &&
                                t != TpUInt && t != TpShort && t != TpUShort &&
                                t != TpArrayDouble && t != TpArrayFloat &&
                                t != TpArrayInt && t != TpArrayUInt &&
                                t != TpArrayShort && t != TpArrayUShort) {
                            invalid << "\n* Range is given for '" << name;
                            invalid << "' which is not a numeric type.";
                            valid = false;
                        }
                        if(r.dataType(RANGE) != TpRecord) {
                            invalid << "\n* Range for '" << name << "' is not";
                            invalid << " given as a record.";
                            valid = false;
                        } else {
                            const RecordInterface& r2 = r.asRecord(RANGE);

                            if(r2.isDefined(RANGE_MIN)) {
                                if(r2.dataType(RANGE_MIN) != TpRecord) {
                                    invalid << "\n* Minimum range value for '";
                                    invalid << name << "' is not of the ";
                                    invalid << "correct type.";
                                    valid = false;
                                } else {
                                    const RecordInterface& r3 = r2.asRecord(
                                            RANGE_MIN);

                                    if(!r3.isDefined(VALUE) ||
                                       r3.dataType(VALUE) != TpArrayDouble) {
                                        invalid << "\n* Minimum range value ";
                                        invalid << "for '" << name << "' is ";
                                        invalid << "not of the correct type.";
                                        valid = false;
                                    } else {
                                        IPosition s = r3.asArrayDouble(
                                                VALUE).shape();
                                        if(s.size() == 0) {
                                            invalid << "\n* Minimum range ";
                                            invalid << "value for '" << name;
                                            invalid << "' is not defined.";
                                            valid = false;
                                        } else {
                                            for(unsigned int j = 0;
                                            j < s.size(); j++) {
                                                if(s[j] == 0) {
                                                    invalid << "\n* Minimum ";
                                                    invalid << "range value ";
                                                    invalid << "for '" << name;
                                                    invalid << "' is not ";
                                                    invalid << "defined.";
                                                    valid = false;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if(r2.isDefined(RANGE_MAX)) {
                                if(r2.dataType(RANGE_MAX) != TpRecord) {
                                    invalid << "\n* Maximum range value for '";
                                    invalid << name << "' is not of the ";
                                    invalid << "correct type.";
                                    valid = false;
                                } else {
                                    const RecordInterface& r3 = r2.asRecord(
                                            RANGE_MAX);

                                    if(!r3.isDefined(VALUE) ||
                                       r3.dataType(VALUE) != TpArrayDouble) {
                                        invalid << "\n* Maximum range value ";
                                        invalid << "for '" << name << "' is ";
                                        invalid << "not of the correct type.";
                                        valid = false;
                                    } else {
                                        IPosition s = r3.asArrayDouble(
                                                VALUE).shape();
                                        if(s.size() == 0) {
                                            invalid << "\n* Maximum range ";
                                            invalid << "value for '" << name;
                                            invalid << "' is not defined.";
                                            valid = false;
                                        } else {
                                            for(unsigned int j = 0;
                                            j < s.size(); j++) {
                                                if(s[j] == 0) {
                                                    invalid << "\n* Maximum ";
                                                    invalid << "range value ";
                                                    invalid << "for '" << name;
                                                    invalid << "' is not ";
                                                    invalid << "defined.";
                                                    valid = false;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    
                    // if value and allowed are defined, make sure value(s)
                    // in allowed array
                    if(valid && r.isDefined(VALUE) && r.isDefined(ALLOWED)) {
                        switch(t) {
                        case TpChar: case TpUChar: {
                            uChar v = r.asuChar(VALUE);
                            const Array<uChar>& a = r.asArrayuChar(ALLOWED);
                            Array<uChar>::ConstIteratorSTL it = a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpShort: case TpUShort: {
                            short v = r.asShort(VALUE);
                            const Array<short>& a = r.asArrayShort(ALLOWED);
                            Array<short>::ConstIteratorSTL it = a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpInt: {
                            int v = r.asInt(VALUE);
                            const Array<int>& a = r.asArrayInt(ALLOWED);
                            Array<int>::ConstIteratorSTL it = a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpUInt: {
                            uInt v = r.asuInt(VALUE);
                            const Array<uInt>& a = r.asArrayuInt(ALLOWED);
                            Array<uInt>::ConstIteratorSTL it = a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpFloat: {
                            float v = r.asFloat(VALUE);
                            const Array<float>& a = r.asArrayFloat(ALLOWED);
                            Array<float>::ConstIteratorSTL it = a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpDouble: {
                            double v = r.asDouble(VALUE);
                            const Array<double>& a = r.asArrayDouble(ALLOWED);
                            Array<double>::ConstIteratorSTL it = a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpComplex: {
                            Complex v = r.asComplex(VALUE);
                            const Array<Complex>& a= r.asArrayComplex(ALLOWED);
                            Array<Complex>::ConstIteratorSTL it= a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpDComplex: {
                            DComplex v = r.asDComplex(VALUE);
                            const Array<DComplex>& a = r.asArrayDComplex(
                                                       ALLOWED);
                            Array<DComplex>::ConstIteratorSTL it=a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpString: {
                            String v = r.asString(VALUE);
                            const Array<String>& a = r.asArrayString(ALLOWED);
                            Array<String>::ConstIteratorSTL it = a.begin();
                            bool found = false;
                            for(; it != a.end(); it++)
                                if(v == *it) found = true;
                            if(!found) {
                                invalid << "\n* Value given for '" << name;
                                invalid << "' is not in the array of allowed ";
                                invalid << "values.";
                                valid = false;
                            }
                            break; }
                        case TpArrayChar: case TpArrayUChar: {
                            const Array<uChar>& a = r.asArrayuChar(ALLOWED);
                            const Array<uChar>& v = r.asArrayuChar(VALUE);
                            Array<uChar>::ConstIteratorSTL it = v.begin();
                            Array<uChar>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }
                        case TpArrayShort: case TpArrayUShort: {
                            const Array<short>& a = r.asArrayShort(ALLOWED);
                            const Array<short>& v = r.asArrayShort(VALUE);
                            Array<short>::ConstIteratorSTL it = v.begin();
                            Array<short>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }
                        case TpArrayInt: {
                            const Array<int>& a = r.asArrayInt(ALLOWED);
                            const Array<int>& v = r.asArrayInt(VALUE);
                            Array<int>::ConstIteratorSTL it = v.begin();
                            Array<int>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }
                        case TpArrayUInt: {
                            const Array<uInt>& a = r.asArrayuInt(ALLOWED);
                            const Array<uInt>& v = r.asArrayuInt(VALUE);
                            Array<uInt>::ConstIteratorSTL it = v.begin();
                            Array<uInt>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }
                        case TpArrayFloat: {
                            const Array<float>& a = r.asArrayFloat(ALLOWED);
                            const Array<float>& v = r.asArrayFloat(VALUE);
                            Array<float>::ConstIteratorSTL it = v.begin();
                            Array<float>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }
                        case TpArrayDouble: {
                            const Array<double>& a = r.asArrayDouble(ALLOWED);
                            const Array<double>& v = r.asArrayDouble(VALUE);
                            Array<double>::ConstIteratorSTL it = v.begin();
                            Array<double>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }
                        case TpArrayComplex: {
                            const Array<Complex>& a= r.asArrayComplex(ALLOWED);
                            const Array<Complex>& v = r.asArrayComplex(VALUE);
                            Array<Complex>::ConstIteratorSTL it = v.begin();
                            Array<Complex>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }
                        case TpArrayDComplex: {
                            const Array<DComplex>& a = r.asArrayDComplex(
                                                       ALLOWED);
                            const Array<DComplex>& v= r.asArrayDComplex(VALUE);
                            Array<DComplex>::ConstIteratorSTL it = v.begin();
                            Array<DComplex>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }
                        case TpArrayString: {
                            const Array<String>& a = r.asArrayString(ALLOWED);
                            const Array<String>& v = r.asArrayString(VALUE);
                            Array<String>::ConstIteratorSTL it = v.begin();
                            Array<String>::ConstIteratorSTL it2;
                            bool found = false;
                            for(; it != v.end(); it++) {
                                for(it2 = a.begin(); it2 != a.end(); it2++)
                                    if(*it == *it2) found = true;
                                if(!found) {
                                    invalid << "\n* Value given for '" << name;
                                    invalid << "' is not in the array of ";
                                    invalid << "allowed values.";
                                    valid = false;
                                    break;
                                }
                            }
                            break; }

                        default: break;
                        }
                    }
                } else {
                    invalid << "\n* Parameter '" << name << "' does not ";
                    invalid << "contain a record!";
                    valid = false;
                }
            }
        } else {
            invalid << "\n* Parameters are not in a record.";
            return pair<bool, String>(false, invalid.str());
        }
    }

    // CONSTRAINTS
    // make sure parameters are valid before verifying constraints
    if(valid&& taskRec.isDefined(PARAMETERS)&& taskRec.isDefined(CONSTRAINTS)){
        if(taskRec.dataType(CONSTRAINTS) == TpRecord) {
            const RecordInterface& pRec = taskRec.asRecord(PARAMETERS);
            const RecordInterface& cRec = taskRec.asRecord(CONSTRAINTS);
            
            // make sure each entry is a record of records
            for(unsigned int i = 0; i < cRec.nfields() && valid; i++) {
                if(cRec.dataType(i) == TpRecord) {
                    const RecordInterface& r = cRec.asRecord(i);
                    
                    for(unsigned int j = 0; j < r.nfields() && valid; j++) {
                        if(r.dataType(j) == TpRecord) {
                            const RecordInterface& r2 = r.asRecord(j);
                            if(r2.isDefined(DEFAULTS)) {
                                if(!r2.dataType(DEFAULTS) == TpRecord) {
                                    invalid << "\n* Constraints must be a ";
                                    invalid << "record of records!";
                                    valid = false;
                                    break;
                                }
                                
                                const RecordInterface& r3 = r2.asRecord(
                                                            DEFAULTS);
                                for(unsigned int k = 0; k < r3.nfields(); k++){
                                    if(r3.dataType(k) != TpRecord) {
                                        invalid << "\n* Constaints must be a ";
                                        invalid << "record of records!";
                                        valid = false;
                                        break;
                                    }
                                    if(r3.asRecord(k).isDefined(VALUE)) {
                                        String name = r3.name(k);
                                        DataType t = r3.asRecord(k).dataType(
                                                     VALUE);
                                        
                                        if(pRec.isDefined(name)) {
                                            DataType t2 = type(pRec.asRecord(
                                                       name).asString(TYPE));
                                            if(t2 != TpOther) {
                                                if(t != t2) {
                                                    invalid << "\n* ";
                                                    invalid << "Constraints ";
                                                    invalid << "value given ";
                                                    invalid << "for '" << name;
                                                    invalid << "' is not of ";
                                                    invalid << "the correct ";
                                                    invalid << "type.";
                                                    valid = false;
                                                    break;
                                                }
                                            } else if(pRec.isDefined(
                                                      LIMITTYPES)) {
                                                const Array<String>& l =
                                                    pRec.asArrayString(
                                                            LIMITTYPES);
                                                bool found = false;
                                                Array<String>::ConstIteratorSTL
                                                    it = l.begin();
                                                for(; it != l.end(); it++) {
                                                    if(type(*it) == t) {
                                                        found = true;
                                                        break;
                                                    }
                                                }
                                                if(!found) {
                                                    invalid << "\n* ";
                                                    invalid << "Constraints ";
                                                    invalid << "value given ";
                                                    invalid << "for '" << name;
                                                    invalid << "' is not of ";
                                                    invalid << "the correct ";
                                                    invalid << "type.";
                                                    valid = false;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            
                        } else {
                            invalid << "\n* Constraints must be a record of ";
                            invalid << "records!";
                            valid = false;
                        }
                    }
                    
                } else {
                    invalid << "\n* Constraints must be a record of records!";
                    valid = false;
                }
            }
            
        } else {
            invalid << "\n* Constraints must be a record!";
            valid = false;
        }
    }
    
    // check that constraints and subparameters match
    if(valid && taskRec.isDefined(PARAMETERS)) {
        // if any subparameters are defined, then constraints exists
        bool subdef = false;
        const RecordInterface& pr = taskRec.asRecord(PARAMETERS);
        for(unsigned int i = 0; i < pr.nfields(); i++){
            if(pr.asRecord(i).isDefined(SUBPARAMETERS)) {
                subdef = true;
                break;
            }
        }
        if((subdef && !taskRec.isDefined(CONSTRAINTS)) ||
           (!subdef && taskRec.isDefined(CONSTRAINTS))) {
            if(!taskRec.isDefined(CONSTRAINTS)) {
                invalid << "\n* If subparameters are defined, then ";
                invalid << "constraints must also be defined and vice versa!";
                valid = false;
            }
        }
        if(valid && taskRec.isDefined(CONSTRAINTS)) {
            const RecordInterface& cr = taskRec.asRecord(CONSTRAINTS);
            // make sure that subparms in constraints match subparms in params
            for(unsigned int i = 0; i < cr.nfields(); i++) {
                const RecordInterface& r = cr.asRecord(i);
                for(unsigned int j = 0; j < r.nfields(); j++) {
                    const RecordInterface& r2 = r.asRecord(j);
                    if(r2.isDefined(DEFAULTS)) {
                        const RecordInterface& r3 = r2.asRecord(DEFAULTS);
                        for(unsigned int k = 0; k < r3.nfields(); k++) {
                            if(!pr.asRecord(cr.name(i)).isDefined(
                                    SUBPARAMETERS)) {
                                invalid << "\n* Subparameter '" << r3.name(k);
                                invalid << "' defined in constraints is not ";
                                invalid << "defined in parameters!";
                                valid = false;
                                break;
                            } else {
                                vector<String> v;
                                pr.asRecord(cr.name(i)).asArrayString(
                                        SUBPARAMETERS).tovector(v);
                                bool found = false;
                                for(unsigned int m = 0; m < v.size(); m++) {
                                    if(v[m] == r3.name(k)) {
                                        found = true;
                                        break;
                                    }
                                }
                                if(!found) {
                                    invalid<<"\n* Subparameter '"<<r3.name(k);
                                    invalid<<"' defined in constraints is not";
                                    invalid << " defined in parameters!";
                                    valid = false;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    // check that all subparameters exist exactly once in the parameter list
    if(taskRec.isDefined(PARAMETERS)) {
        const RecordInterface& pr = taskRec.asRecord(PARAMETERS);
        vector<String> subparams;
        vector<String> parents;
        for(unsigned int i = 0; i < pr.nfields(); i++) {
            if(pr.asRecord(i).isDefined(SUBPARAMETERS)) {
                parents.push_back(pr.name(i));
                const Array<String>& a = pr.asRecord(i).asArrayString(
                                         SUBPARAMETERS);
                Array<String>::ConstIteratorSTL it = a.begin();
                for(; it != a.end(); it++) subparams.push_back(*it);
            }
        }
        
        for(unsigned int i = 0; i < subparams.size(); i++) {
            // check for duplicates
            for(unsigned int j = i + 1; j < subparams.size(); j++) {
                if(subparams[i] == subparams[j]) {
                    invalid << "\n* Subparameters cannot contain duplicates!";
                    valid = false;
                }
            }
            
            // make sure subparam is defined in parameters list
            if(!pr.isDefined(subparams[i])) {
                invalid << "\n* Subparameter '" << subparams[i] << "' is ";
                invalid << "not defined in parameter list!";
                valid = false;
            }
            
            // make sure parents are not subparameters of other parents
            for(unsigned int i = 0; i < parents.size(); i++) {
                for(unsigned int j = 0; j < subparams.size(); j++) {
                    if(parents[i] == subparams[j]) {
                        invalid << "\n* Parameter '" << parents[i] << "' ";
                        invalid << "cannot be a subparameter that has ";
                        invalid << "subparameters!";
                        valid = false;
                        break;
                    }
                }
            }
        }
    }

    // DESCRIPTION
    if(taskRec.isDefined(DESCRIPTION)) {
        if(taskRec.dataType(DESCRIPTION) != TpString) {
            invalid << "\n* Example is not a String!";
            valid = false;
        }
    }

    // EXAMPLE
    if(taskRec.isDefined(EXAMPLE)) {
        if(taskRec.dataType(EXAMPLE) != TpString) {
            invalid << "\n* Example is not a String!";
            valid = false;
        }
    }

    // SHORTDESCRIPTION
    if(taskRec.isDefined(SHORT_DESCRIPTION)) {
        if(taskRec.dataType(SHORT_DESCRIPTION) != TpString) {
            invalid << "\n* Short description is not a String!";
            valid = false;
        }
    }
    
    // PARAMETERORDER
    if(valid && taskRec.isDefined(PARAMETER_ORDER)) {
        if(taskRec.dataType(PARAMETER_ORDER) == TpArrayString) {
            // copy into temp vector
            vector<String> v;
            const Array<String>& a = taskRec.asArrayString(PARAMETER_ORDER);
            Array<String>::ConstIteratorSTL it = a.begin();
            for(; it != a.end(); it++)
                v.push_back(*it);
            
            // make sure all parameters are included
            const RecordInterface& paramRec = taskRec.asRecord(PARAMETERS);
            if(v.size() != paramRec.nfields()) {
                invalid << "\n* Invalid parameter order!";
                valid = false;
            } else {
                for(unsigned int i = 0; i < v.size(); i++) {
                    if(!paramRec.isDefined(v[i])) {
                        invalid << "\n* Invalid parameter order!";
                        valid = false;
                        break;
                    }
                }
            }
        } else {
            invalid << "\n* Parameter order is not a String array!";
            valid = false;
        }
    }

    return pair<bool, String>(valid, valid ? "" : invalid.str());
}

void QtParamGUI::setupRecord(vector<String> order) {
    QVBoxLayout* layout = dynamic_cast<QVBoxLayout*>(frame->layout());
    if(layout == NULL) {
        layout = new QVBoxLayout(frame);

#if QT_VERSION >= 0x040300
        layout->setContentsMargins(0, 0, 0, 0);
#else
        layout->setMargin(0);
#endif
    } else {
        ParamPanel* p;
        for(unsigned int i = 0; i < m_panels.size(); i++) {
            p = m_panels[i];
            layout->removeWidget(p);
            delete p;
        }
        m_panels.clear();
        // remove spacer
        while(layout->count() > 0) layout->removeItem(layout->itemAt(0));
    }

    if(m_tasks.asRecord(m_taskName).isDefined(PARAMETERS)) {
        ParamPanel* p;
        const RecordInterface& params = m_tasks.asRecord(
                                        m_taskName).asRecord(PARAMETERS);
        
        // Find the maximum width for the parameter names, and use that
        QLabel label;
        QFont f = label.font();
        f.setBold(true);
        label.setFont(f);
        QFontMetrics metrics = label.fontMetrics();
        int maxWidth = -1;
        int tmp;
        for(unsigned int i = 0; i < params.nfields(); i++) {
            tmp = metrics.width(params.name(i).c_str());
            if(tmp > maxWidth) maxWidth = tmp;
        }
        
        for(unsigned int i = 0; i < order.size(); i++) {
            p = new ParamPanel(*this, order[i], params.asRecord(order[i]),
                               m_globals, m_last, m_resetMode, maxWidth,
                               constraintsString(order[i]), false);
            
            m_panels.push_back(p);
            layout->addWidget(p);
            connect(p, SIGNAL(valueChanged(String)),
                    this, SLOT(valueChanged(String)));
            
            // subparams
            if(params.asRecord(order[i]).isDefined(SUBPARAMETERS)) {
                const Array<String>& a = params.asRecord(order[
                                         i]).asArrayString(SUBPARAMETERS);
                Array<String>::ConstIteratorSTL it = a.begin();
                for(; it != a.end(); it++) {
                    p = new ParamPanel(*this, *it, params.asRecord(*it),
                                       m_globals, m_last, m_resetMode,
                                       maxWidth, constraintsString(*it), true);
                    m_panels.push_back(p);
                    layout->addWidget(p);
                    connect(p, SIGNAL(valueChanged(String)),
                            this, SLOT(valueChanged(String)));
                }
            }
        }
        layout->addStretch();
        scrollFrame->setFrameStyle(QFrame::NoFrame);
        scrollFrame->setWidget(frame);
        scrollFrame->setWidgetResizable(true);
        
        // show/hide subparameters as necessary
        if(m_tasks.asRecord(m_taskName).isDefined(CONSTRAINTS)) {
            const RecordInterface& c = m_tasks.asRecord(m_taskName).asRecord(
                                       CONSTRAINTS);
            for(unsigned int i = 0; i < c.nfields(); i++) {
                valueChanged(c.name(i));
            }
        }
    }
}

String QtParamGUI::constraintsString(const String& id) {
    if(!m_tasks.asRecord(m_taskName).isDefined(CONSTRAINTS) ||
       !m_tasks.asRecord(m_taskName).asRecord(CONSTRAINTS).isDefined(id) ||
       m_tasks.asRecord(m_taskName).asRecord(CONSTRAINTS).asRecord(
               id).nfields() == 0)
        return String();
    
    stringstream ss;
    const RecordInterface& r = m_tasks.asRecord(m_taskName).asRecord(
            CONSTRAINTS).asRecord(id);
    
    unsigned int spaces;
    unsigned int maxLength = 0;
    for(unsigned int i = 0; i < r.nfields(); i++) {
        unsigned int temp = r.name(i).size();
        if(temp == 0) temp = 2; // will be ''
        if(temp > maxLength) maxLength = temp;
    }
        
    for(unsigned int i = 0; i < r.nfields(); i++) {
        if(i > 0) ss << '\n';
        ss << (r.name(i).size() > 0 ? r.name(i) : "''") << ": ";
        unsigned int n = r.name(i).size() > 0 ? r.name(i).size() : 2;
        for(unsigned int j = 0; j < maxLength - n; j++)
            ss << ' ';
        spaces = maxLength + 2;
        
        if(!r.asRecord(i).isDefined(DEFAULTS) ||
           r.asRecord(i).asRecord(DEFAULTS).nfields() == 0) ss << "--";
        else {
            const RecordInterface& d = r.asRecord(i).asRecord(DEFAULTS);
            for(unsigned int j = 0; j < d.nfields(); j++) {
                if(j > 0) {
                    ss << '\n';
                    for(unsigned int k = 0; k < spaces; k++) ss << ' ';
                }
                ss << '(' << d.name(j) << ", ";
                
                DataType t = d.asRecord(j).dataType(VALUE);
                
                switch(t) {
                case TpBool:
                    ss << (d.asRecord(j).asBool(VALUE) ? "True" :
                                                         "False");
                    break;
                case TpChar: case TpUChar:
                    ss << d.asRecord(j).asuChar(VALUE); break;
                case TpShort: case TpUShort:
                    ss << d.asRecord(j).asShort(VALUE); break;
                case TpInt:
                    ss << d.asRecord(j).asInt(VALUE); break;
                case TpUInt:
                    ss << d.asRecord(j).asuInt(VALUE); break;
                case TpFloat:
                    ss << d.asRecord(j).asFloat(VALUE); break;
                case TpDouble:
                    ss << d.asRecord(j).asDouble(VALUE); break;
                case TpComplex: {
                    Complex c = d.asRecord(j).asComplex(VALUE);
                    ss << '(' << c.real() << ", " << c.imag();
                    ss << ')'; break; }
                case TpDComplex: {
                    DComplex c = d.asRecord(j).asDComplex(VALUE);
                    ss << '(' << c.real() << ", " << c.imag();
                    ss << ')'; break; }
                case TpString:
                    ss << d.asRecord(j).asString(VALUE);
                    if(d.asRecord(j).asString(VALUE).size() == 0)
                        ss << "''";
                    break;
                case TpRecord:
                    ss << recordToString(d.asRecord(j).asRecord(
                            VALUE), ":"); break;
                case TpArrayBool: {
                    const Array<Bool>& a = d.asRecord(
                            j).asArrayBool(VALUE);
                    Array<Bool>::ConstIteratorSTL it = a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << (*it ? "True" : "False");
                    }
                    ss << ']';
                    break; }
                case TpArrayChar: case TpArrayUChar: {
                    const Array<uChar>& a = d.asRecord(
                            j).asArrayuChar(VALUE);
                    Array<uChar>::ConstIteratorSTL it = a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << *it;
                    }
                    ss << ']';
                    break; }
                case TpArrayShort: case TpArrayUShort: {
                    const Array<Short>& a = d.asRecord(
                            j).asArrayShort(VALUE);
                    Array<Short>::ConstIteratorSTL it = a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << *it;
                    }
                    ss << ']';
                    break; }
                case TpArrayInt: {
                    const Array<Int>& a = d.asRecord(
                            j).asArrayInt(VALUE);
                    Array<Int>::ConstIteratorSTL it = a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << *it;
                    }
                    ss << ']';
                    break; }
                case TpArrayUInt: {
                    const Array<uInt>& a = d.asRecord(
                            j).asArrayuInt(VALUE);
                    Array<uInt>::ConstIteratorSTL it = a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << *it;
                    }
                    ss << ']';
                    break; }
                case TpArrayFloat: {
                    const Array<Float>& a = d.asRecord(
                            j).asArrayFloat(VALUE);
                    Array<Float>::ConstIteratorSTL it = a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << *it;
                    }
                    ss << ']';
                    break; }
                case TpArrayDouble: {
                    const Array<Double>& a = d.asRecord(
                            j).asArrayDouble(VALUE);
                    Array<Double>::ConstIteratorSTL it = a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << *it;
                    }
                    ss << ']';
                    break; }
                case TpArrayComplex: {
                    const Array<Complex>& a = d.asRecord(
                            j).asArrayComplex(VALUE);
                    Array<Complex>::ConstIteratorSTL it= a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << '(' << (*it).real() << ", ";
                        ss << (*it).imag() << ')';
                    }
                    ss << ']';
                    break; }
                case TpArrayDComplex: {
                    const Array<DComplex>& a = d.asRecord(
                            j).asArrayDComplex(VALUE);
                    Array<DComplex>::ConstIteratorSTL it=a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << '(' << (*it).real() << ", ";
                        ss << (*it).imag() << ')';
                    }
                    ss << ']';
                    break; }
                case TpArrayString: {
                    const Array<String>& a = d.asRecord(
                            j).asArrayString(VALUE);
                    Array<String>::ConstIteratorSTL it = a.begin();
                    ss << '[';
                    for(; it != a.end(); it++) {
                        if(it != a.begin()) ss << ' ';
                        ss << *it;
                        if((*it).empty()) ss << "''";
                    }
                    ss << ']';
                    break; }
                    
                default: break;
                }
                
                ss << ')';
            }
        }
    }
    
    return ss.str();
}

void QtParamGUI::showHideResets(bool globals, bool last) {
    if(!globals && !last) {
        resetFrame->setVisible(false);
    } else {
        resetFrame->setVisible(true);
        resetDefaults->setVisible(true);
        resetGlobals->setVisible(globals);
        resetLast->setVisible(last);
    }
}


// Private Slots //

void QtParamGUI::run() {
    bool valid = true;
    String validStr = "There are problems with your parameters:";
    for(unsigned int i = 0; i < m_panels.size(); i++) {
        pair<bool, String> v = m_panels[i]->isValid();
        if(!v.first) {            
            valid = false;
            validStr += '\n';
            validStr += v.second;
        }
    }

    if(!valid) {
        QMessageBox::warning(this, "Parameter Errors", validStr.c_str());
        return;
    }

    Record record = enteredParameters();

    if(m_mode == PYTHON) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        setEnabled(false);
        QCoreApplication::processEvents();
        if(m_python.init(loggerBox->isChecked()))
            m_python.command(pythonCommand(m_taskName, record));
        else
            QMessageBox::critical(this, "Python Initialization",
                    "Python interpreter failed to initialize!");
        setEnabled(true);
        QApplication::restoreOverrideCursor();
        if(closeOnRunBox->isChecked()) accept();
        else if(loggerBox->isVisible()) loggerBox->setEnabled(false);
    }
    else if(m_mode == SIGNAL) emit runRequested(m_taskName, record);
}

void QtParamGUI::reset() {
    for(unsigned int i = 0; i < m_panels.size(); i++)
        m_panels[i]->reset();
}

void QtParamGUI::cancel() {
    if(m_mode == PYTHON) reject();
    else emit cancelRequested();
}

void QtParamGUI::valueChanged(String paramName) {
    if(!m_tasks.asRecord(m_taskName).asRecord(PARAMETERS).asRecord(
               paramName).isDefined(SUBPARAMETERS)) return;
    
    ParamPanel* p = NULL;
    for(unsigned int i = 0; i < m_panels.size(); i++) {
        if(m_panels[i]->name() == paramName) {
            p = m_panels[i];
            break;
        }
    }

    vector<String> subparams;
    m_tasks.asRecord(m_taskName).asRecord(PARAMETERS).asRecord(
            paramName).asArrayString(SUBPARAMETERS).tovector(subparams);
    const RecordInterface& constraints = m_tasks.asRecord(m_taskName).asRecord(
            CONSTRAINTS).asRecord(paramName);
    
    Record val, val2;
    p->getValue(val);
    unsigned int i = 0;
    for(; i < constraints.nfields(); i++) {
        val2 = Record();
        val2.define("", constraints.name(i));
        if(valuesEqual(val, 0, val2, 0, true)) {
            if(constraints.asRecord(i).nfields() == 0 ||
               !constraints.asRecord(i).isDefined(DEFAULTS)) {
                i = constraints.nfields();
                break;
            }
            
            const RecordInterface& defs = constraints.asRecord(i).asRecord(
                                          DEFAULTS);
            for(unsigned int j = 0; j < defs.nfields(); j++) {
                p = NULL;
                // set visible
                for(unsigned int k = 0; k < m_panels.size(); k++) {
                    if(m_panels[k]->name() == defs.name(j)) {
                        p = m_panels[k];
                        break;
                    }
                }
                p->setVisible(true);
                
                // remove from subparams list
                for(unsigned int k = 0; k < subparams.size(); k++) {
                    if(subparams[k] == defs.name(j)) {
                        subparams.erase(subparams.begin() + k);
                        break;
                    }
                }
                
                // set value
                if(defs.asRecord(j).isDefined(VALUE)) {
                    const RecordInterface& r = defs.asRecord(j);
                    p->value()->setValue(r, VALUE);
                }
            }
            
            // set all left in subparams list to be invisible
            i = constraints.nfields();
            
            break;
        }
    }
    
    if(i == constraints.nfields()) {
        // hide all subparams
        for(unsigned int i = 0; i < subparams.size(); i++) {
            p = NULL;
            for(unsigned int j = 0; j < m_panels.size(); j++) {
                if(m_panels[j]->name() == subparams[i]) {
                    p = m_panels[j];
                    break;
                }
            }
            p->setVisible(false);
        }
        
    }
}

void QtParamGUI::showHideDesc(bool checked) {
    m_descDialog->setVisible(checked);
}

void QtParamGUI::showHideExample(bool checked) {
    m_exampleDialog->setVisible(checked);
}

void QtParamGUI::descClosed() { descButton->setChecked(false); }

void QtParamGUI::exampleClosed() { exampleButton->setChecked(false); }

void QtParamGUI::taskChosen(QString task) {
    String name = qPrintable(task);
    if(m_tasks.isDefined(name))
        setRecord(m_tasks, name);
}

void QtParamGUI::resetChanged() {
    if(resetGlobals->isVisible() && resetGlobals->isChecked())
        m_resetMode = GLOBAL;
    else if(resetLast->isVisible() && resetLast->isChecked())
        m_resetMode = LAST;
    else m_resetMode = DEFAULT;
}

void QtParamGUI::loadLast() {    
    QFile last(lastLoadButton->toolTip());
    stringstream invalid;
    invalid << "Could not load last parameters:";
    bool valid = readLastFile(m_last, last, invalid);

    lastLoadButton->setVisible(false);
    showHideResets(m_globals.nfields() > 0, m_last.nfields() > 0);
    
    if(valid) {
        resetLast->setChecked(true);
        reset();
    } else
        QMessageBox::critical(this, "Loading Last Parameters",
                              invalid.str().c_str());
}


// Static Members //

const String QtParamGUI::ALLOWED = "allowed";
const String QtParamGUI::ANY = "any";
const String QtParamGUI::CONSTRAINTS = "constraints";
const String QtParamGUI::DEFAULTS = "defaults";
const String QtParamGUI::DESCRIPTION = "description";
const String QtParamGUI::EXAMPLE = "example";
const String QtParamGUI::KIND = "kind";
const String QtParamGUI::KIND_FILE = "file";
const String QtParamGUI::KIND_MS = "ms";
const String QtParamGUI::KIND_TABLE = "table";
const String QtParamGUI::LIMITTYPES = "limittypes";
const String QtParamGUI::MUSTEXIST = "mustexist";
const String QtParamGUI::PARAMETERS = "parameters";
const String QtParamGUI::PARAMETER_ORDER = "parameterorder";
const String QtParamGUI::RANGE = "range";
const String QtParamGUI::RANGE_MIN = "min";
const String QtParamGUI::RANGE_MAX = "max";
const String QtParamGUI::SHORT_DESCRIPTION = "shortdescription";
const String QtParamGUI::SUBPARAMETERS = "subparameters";
const String QtParamGUI::TYPE = "type";
const String QtParamGUI::UNITS = "units";
const String QtParamGUI::VALUE = "value";

DataType QtParamGUI::type(String typeName) {
    typeName.downcase();

    // check for typeArray -- need to recapitalize the A
    unsigned int n = typeName.size();
    if(n >= 5 && typeName.from((size_t)(n - 5)) == "array") // for 64-bit
        typeName[n - 5] = 'A';

    if(typeName == type(TpBool) || typeName == "boolean") return TpBool;
    else if(typeName == type(TpChar)) return TpChar;
    else if(typeName == type(TpUChar)) return TpUChar;
    else if(typeName == type(TpShort)) return TpShort;
    else if(typeName == type(TpUShort)) return TpUShort;
    else if(typeName == type(TpInt)) return TpInt;
    else if(typeName == type(TpUInt)) return TpUInt;
    else if(typeName == type(TpFloat)) return TpFloat;
    else if(typeName == type(TpDouble)) return TpDouble;
    else if(typeName == type(TpComplex)) return TpComplex;
    else if(typeName == type(TpDComplex)) return TpDComplex;
    else if(typeName == type(TpString)) return TpString;
    else if(typeName == type(TpTable)) return TpTable;
    else if(typeName == type(TpRecord)) return TpRecord;
    else if(typeName == type(TpArrayBool)) return TpArrayBool;
    else if(typeName == type(TpArrayChar)) return TpArrayChar;
    else if(typeName == type(TpArrayUChar)) return TpArrayUChar;
    else if(typeName == type(TpArrayShort)) return TpArrayShort;
    else if(typeName == type(TpArrayUShort)) return TpArrayUShort;
    else if(typeName == type(TpArrayInt)) return TpArrayInt;
    else if(typeName == type(TpArrayUInt)) return TpArrayUInt;
    else if(typeName == type(TpArrayFloat)) return TpArrayFloat;
    else if(typeName == type(TpArrayDouble)) return TpArrayDouble;
    else if(typeName == type(TpArrayComplex)) return TpArrayComplex;
    else if(typeName == type(TpArrayDComplex)) return TpArrayDComplex;
    else if(typeName == type(TpArrayString)) return TpArrayString;

    else return TpOther;
}

String QtParamGUI::type(DataType type) {
    switch(type) {
    case TpBool: return "bool";
    case TpChar: return "char";
    case TpUChar: return "uchar";
    case TpShort: return "short";
    case TpUShort: return "ushort";
    case TpInt: return "int";
    case TpUInt: return "uint";
    case TpFloat: return "float";
    case TpDouble: return "double";
    case TpComplex: return "complex";
    case TpDComplex: return "dcomplex";
    case TpString: return "string";
    case TpTable: return "table";
    case TpArrayBool: return "boolArray";
    case TpArrayChar: return "charArray";
    case TpArrayUChar: return "ucharArray";
    case TpArrayShort: return "shortArray";
    case TpArrayUShort: return "ushortArray";
    case TpArrayInt: return "intArray";
    case TpArrayUInt: return "uintArray";
    case TpArrayFloat: return "floatArray";
    case TpArrayDouble: return "doubleArray";
    case TpArrayComplex: return "complexArray";
    case TpArrayDComplex: return "dcomplexArray";
    case TpArrayString: return "stringArray";
    case TpRecord: return "record";
    case TpOther: return "other";

    default: return "";
    }
}

String QtParamGUI::recordToString(const Record& record, String sep) {
    stringstream ss;
    DataType t;
    for(unsigned int i = 0; i < record.nfields(); i++) {
        t = record.dataType(i);        
        if(t == TpOther || t == TpTable || t == TpNumberOfTypes ||
                t == TpQuantity || t == TpArrayQuantity) continue;
        if(record.name(i).size() > 0) ss << record.name(i) << sep;
        else ss << "''" << sep;

        switch(t) {        
        case TpBool: ss << ((record.asBool(i)) ? "True" : "False"); break;
        case TpChar: case TpUChar:
            ss << '\'' << record.asuChar(i) << '\''; break;
        case TpShort: case TpUShort: ss << record.asShort(i); break;
        case TpInt: ss << record.asInt(i); break;
        case TpUInt: ss << record.asuInt(i); break;
        case TpFloat: {
            float v = record.asFloat(i);
            ss << v;
            if(v == (float)((int)v)) ss << ".0";
            break; }
        case TpDouble: {
            double v = record.asDouble(i);
            ss << v;
            if(v == (double)((int)v)) ss << ".0";
            break; }
        case TpComplex: {
            Complex c = record.asComplex(i);
            float v1 = c.real(), v2 = c.imag();
            ss << "complex(" << v1;
            if(v1 == (float)((int)v1)) ss << ".0";
            ss << ", " << v2;
            if(v2 == (float)((int)v2)) ss << ".0";
            ss << ')';
            break; }
        case TpDComplex: {
            DComplex c = record.asDComplex(i);
            double v1 = c.real(), v2 = c.imag();
            ss << "complex(" << v1;
            if(v1 == (double)((int)v1)) ss << ".0";
            ss << ", " << v2;
            if(v2 == (double)((int)v2)) ss << ".0";
            ss << ')';
            break; }
        case TpString: ss << '\'' << record.asString(i) << '\''; break;
        case TpRecord:
            ss << '{' << recordToString(record.asRecord(i), ":") << '}'; break;

        case TpArrayBool: {
            ss << '[';
            const Array<Bool> a = record.asArrayBool(i);
            Array<Bool>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            for(; i != a.end(); i++) {
                ss << ((*i) ? "True" : "False");
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayChar: case TpArrayUChar: {
            ss << '[';
            const Array<uChar> a = record.asArrayuChar(i);
            Array<uChar>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            for(; i != a.end(); i++) {
                ss << (*i);
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayShort: case TpArrayUShort: {
            ss << '[';
            const Array<Short> a = record.asArrayShort(i);
            Array<Short>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            for(; i != a.end(); i++) {
                ss << (*i);
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayInt: {
            ss << '[';
            const Array<Int> a = record.asArrayInt(i);
            Array<Int>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            for(; i != a.end(); i++) {
                ss << (*i);
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayUInt: {
            ss << '[';
            const Array<uInt> a = record.asArrayuInt(i);
            Array<uInt>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            for(; i != a.end(); i++) {
                ss << (*i);
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayFloat: {
            ss << '[';
            const Array<Float> a = record.asArrayFloat(i);
            Array<Float>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            float v;
            for(; i != a.end(); i++) {
                v = *i;
                ss << v;
                if(v == (float)((int)v)) ss << ".0";
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayDouble: {
            ss << '[';
            const Array<Double> a = record.asArrayDouble(i);
            Array<Double>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            double v;
            for(; i != a.end(); i++) {
                v = *i;
                ss << v;
                if(v == (double)((int)v)) ss << ".0";
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayComplex: {
            ss << '[';
            const Array<Complex> a = record.asArrayComplex(i);
            Array<Complex>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            float v1, v2;
            for(; i != a.end(); i++) {
                v1 = (*i).real(); v2 = (*i).imag();
                ss << "complex(" << v1;
                if(v1 == (float)((int)v1)) ss << ".0";
                ss << ", " << v2;
                if(v2 == (float)((int)v2)) ss << ".0";
                ss << ')';
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayDComplex: {
            ss << '[';
            const Array<DComplex> a = record.asArrayDComplex(i);
            Array<DComplex>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            double v1, v2;
            for(; i != a.end(); i++) {
                v1 = (*i).real(); v2 = (*i).imag();
                ss << "complex(" << v1;
                if(v1 == (double)((int)v1)) ss << ".0";
                ss << ", " << v2;
                if(v2 == (double)((int)v2)) ss << ".0";
                ss << ')';
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        case TpArrayString: {
            ss << '[';
            const Array<String> a = record.asArrayString(i);
            Array<String>::ConstIteratorSTL i = a.begin();
            unsigned int count = 0;
            for(; i != a.end(); i++) {
                ss << '\'' << (*i) << '\'';
                if(count < a.size() - 1) ss << ", ";
                count++;
            }
            ss << ']'; break; }

        default: break;        
        }

        if(i < record.nfields() - 1) ss << ", ";
    }

    return ss.str();
}

String QtParamGUI::pythonCommand(String taskName, const Record& params) {
    stringstream ss;
    ss << taskName << "(" << recordToString(params, "=") << ")";
    return ss.str();
}

void QtParamGUI::setupDialog(QDialog* d, String taskName, String name,
                             const String& value) {
    d->setWindowTitle((taskName + ": " + name).c_str());
    QVBoxLayout* l = new QVBoxLayout(d);
#if QT_VERSION >= 0x040300
    l->setContentsMargins(0, 0, 0, 0);
#else
    l->setMargin(0);
#endif
    l->setSpacing(3);

    String str = "<b>";
    str += name;
    str += "</b>:";
    l->addWidget(new QLabel(str.c_str()));

    QScrollArea* a = new QScrollArea(d);
    QLabel* label = new QLabel(("<pre>" + value + "</pre>").c_str());
    a->setWidget(label);
    l->addWidget(a);
}

void QtParamGUI::replaceSlashQuotes(String& str) {
    unsigned int i = str.find("\\'");
    while(i < str.size()) {
        str.erase(i, 1); // remove the "\"
        i = str.find("\\'", i + 1);
    }
    
    i = str.find("\\\"");
    while(i < str.size()) {
        str.erase(i, 1); // remove the "\"
        i = str.find("\\\"", i + 1);
    }
}


// Private Static //

const unsigned int QtParamGUI::MAX_SHORTDESC_LENGTH = 120;

String QtParamGUI::strTrim(const String& str) {
    unsigned int i = 0;
    char c;
    for(; i < str.size(); i++) {
        c = str[i];
        if(c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
    }
    if(i == str.size()) return String();
        
    unsigned int j = str.size() - 1;
    for(; j >= 0; j--) {
        c = str[j];
        if(c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
    }
    
    if(i == 0 && j == str.size() - 1) return str;
    else return str.substr(i, (j - i) + 1);
}

String QtParamGUI::strNLtrim(const String& str) {
    unsigned int i = 0;
    char c;
    for(; i < str.size(); i++) {
        c = str[i];
        if(c != '\n' && c != '\r') break;
    }
    if(i == str.size()) return String();
        
    unsigned int j = str.size() - 1;
    for(; j >= 0; j--) {
        c = str[j];
        if(c != '\n' && c != '\r' && c != ' ' && c != '\t') break;
    }
    
    if(i == 0 && j == str.size() - 1) return str;
    else return str.substr(i, (j - i) + 1);
}

bool QtParamGUI::valuesEqual(const RecordInterface& r1, RecordFieldId id1,
                             const RecordInterface& r2, RecordFieldId id2,
                             bool allowStrings) {
    if(id1.byName()) {
        if(!r1.isDefined(id1.fieldName())) return false;
    } else {
        if(id1.fieldNumber() < 0 ||
           (unsigned int)id1.fieldNumber() >= r1.nfields()) return false;
    }
    if(id2.byName()) {
        if(!r2.isDefined(id2.fieldName())) return false;
    } else {
        if(id2.fieldNumber() < 0 ||
           (unsigned int)id2.fieldNumber() >= r2.nfields()) return false;
    }
    
    switch(r1.dataType(id1)) {
    case TpBool:
        switch(r2.dataType(id2)) {
        case TpBool: return r1.asBool(id1) == r2.asBool(id2);
        case TpShort: return r1.asBool(id1) == (r2.asShort(id2) != 0);
        case TpInt: return r1.asBool(id1) == (r2.asInt(id2) != 0);
        case TpUInt: return r1.asBool(id1) == (r2.asuInt(id2) != 0);
        case TpString: {
            if(!allowStrings) return false;
            String v = r2.asString(id2); v.downcase();
            if(r1.asBool(id1)) return v == "1" || v == "t" || v == "true";
            else return v == "0" || v == "f" || v == "false"; }
        default: return false;
        }
        
    case TpUChar:
        switch(r2.dataType(id2)) {
        case TpUChar: return r1.asuChar(id1) == r2.asuChar(id2);
        case TpString: return r2.asString(id2).size() == 1 &&
                              r1.asuChar(id1) == r2.asString(id2)[0];
        default: return false;
        }
        
    case TpShort:
        switch(r2.dataType(id2)) {
        case TpUChar: return r1.asShort(id1) == r2.asuChar(id2);
        case TpShort: return r1.asShort(id1) == r2.asShort(id2);
        case TpInt: return r1.asShort(id1) == r2.asInt(id2);
        case TpUInt: return (uInt)r1.asShort(id1) == r2.asuInt(id2);
        case TpFloat: return r1.asShort(id1) == r2.asFloat(id2);
        case TpDouble: return r1.asShort(id1) == r2.asDouble(id2);
        case TpString: {
            if(!allowStrings) return false;
            stringstream ss; ss << r1.asShort(id1);
            return String(ss.str()) == r2.asString(id2); }
        default: return false;
        }
            
    case TpInt:
        switch(r2.dataType(id2)) {
        case TpUChar: return r1.asInt(id1) == r2.asuChar(id2);
        case TpShort: return r1.asInt(id1) == r2.asShort(id2);
        case TpInt: return r1.asInt(id1) == r2.asInt(id2);
        case TpUInt: return (double)r1.asInt(id1) == (double)r2.asuInt(id2);
        case TpFloat: return r1.asInt(id1) == r2.asFloat(id2);
        case TpDouble: return r1.asInt(id1) == r2.asDouble(id2);
        case TpString: {
            if(!allowStrings) return false;
            stringstream ss; ss << r1.asInt(id1);
            return String(ss.str()) == r2.asString(id2); }
        default: return false;
        }
        
    case TpUInt:
        switch(r2.dataType(id2)) {
        case TpUChar: return r1.asuInt(id1) == r2.asuChar(id2);
        case TpShort: return r1.asuInt(id1) == (uInt)r2.asShort(id2);
        case TpInt: return (double)r1.asuInt(id1) == (double)r2.asInt(id2);
        case TpUInt: return r1.asuInt(id1) == r2.asuInt(id2);
        case TpFloat: return r1.asuInt(id1) == r2.asFloat(id2);
        case TpDouble: return r1.asuInt(id1) == r2.asDouble(id2);
        case TpString: {
            if(!allowStrings) return false;
            stringstream ss; ss << r1.asuInt(id1);
            return String(ss.str()) == r2.asString(id2); }
        default: return false;
        }
        
    case TpFloat:
        switch(r2.dataType(id2)) {
        case TpUChar: return r1.asFloat(id1) == r2.asuChar(id2);
        case TpShort: return r1.asFloat(id1) == r2.asShort(id2);
        case TpInt: return r1.asFloat(id1) == r2.asInt(id2);
        case TpUInt: return r1.asFloat(id1) == r2.asuInt(id2);
        case TpFloat: return r1.asFloat(id1) == r2.asFloat(id2);
        case TpDouble: return r1.asFloat(id1) == r2.asDouble(id2);
        case TpString: {
            if(!allowStrings) return false;
            float f;
            if(sscanf(r2.asString(id2).c_str(), "%f", &f) != 1) return false;
            return r1.asFloat(id1) == f; }
        default: return false;
        }
        
    case TpDouble:
        switch(r2.dataType(id2)) {
        case TpUChar: return r1.asDouble(id1) == r2.asuChar(id2);
        case TpShort: return r1.asDouble(id1) == r2.asShort(id2);
        case TpInt: return r1.asDouble(id1) == r2.asInt(id2);
        case TpUInt: return r1.asDouble(id1) == r2.asuInt(id2);
        case TpFloat: return r1.asDouble(id1) == r2.asFloat(id2);
        case TpDouble: return r1.asDouble(id1) == r2.asDouble(id2);
        case TpString: {
            if(!allowStrings) return false;
            double d;
            if(sscanf(r2.asString(id2).c_str(), "%lf", &d) != 1) return false;
            return r1.asDouble(id1) == d; }
        default: return false;
        }
        
    case TpComplex:
        switch(r2.dataType(id2)) {
        case TpComplex: return r1.asComplex(id1) == r2.asComplex(id2);
        case TpDComplex: return DComplex(r1.asComplex(id1).real(),
                      r1.asComplex(id1).imag()) == r2.asDComplex(id2);
        case TpString: {
            if(!allowStrings) return false;
            float f1, f2;
            if(sscanf(r2.asString(id2).c_str(), "(%f,%f)", &f1, &f2) != 1)
                return false;
            return r1.asComplex(id1) == Complex(f1, f2); }
        default: return false;
        }
        
    case TpDComplex:
        switch(r2.dataType(id2)) {
        case TpComplex: return r1.asDComplex(id1) == DComplex(r2.asComplex(
                id2).real(), r2.asComplex(id2).imag());
        case TpDComplex: return r1.asDComplex(id1) == r2.asDComplex(id2);
        case TpString: {
            if(!allowStrings) return false;
            double d1, d2;
            if(sscanf(r2.asString(id2).c_str(), "(%lf,%lf)", &d1, &d2) != 1)
                return false;
            return r1.asDComplex(id1) == DComplex(d1, d2); }
        default: return false;
        }
        
    case TpString:
        switch(r2.dataType(id2)) {
        case TpUChar: return r1.asString(id1).size() == 1 &&
                             r1.asString(id1)[0] == r2.asuChar(id2);
        case TpString: return r1.asString(id1) == r2.asString(id2);
        default: return false;
        }
        
    case TpRecord: 
        switch(r2.dataType(id2)) {
        case TpRecord: return recordEqual(r1.asRecord(id1), r2.asRecord(id2));
        default: return false;
        }
        
    case TpArrayBool: 
        switch(r2.dataType(id2)) {
        case TpArrayBool: return arrayEqual<Bool>(r1.asArrayBool(id1),
                                                  r2.asArrayBool(id2));
        default: return false;
        }
        
    case TpArrayUChar: 
        switch(r2.dataType(id2)) {
        case TpArrayUChar: return arrayEqual<uChar>(r1.asArrayuChar(id1),
                                                    r2.asArrayuChar(id2));
        default: return false;
        }
        
    case TpArrayShort: 
        switch(r2.dataType(id2)) {
        case TpArrayShort: return arrayEqual<Short>(r1.asArrayShort(id1),
                                                    r2.asArrayShort(id2));
        default: return false;
        }
        
    case TpArrayInt: 
        switch(r2.dataType(id2)) {
        case TpArrayInt: return arrayEqual<Int>(r1.asArrayInt(id1),
                                                r2.asArrayInt(id2));
        default: return false;
        }
        
    case TpArrayUInt: 
        switch(r2.dataType(id2)) {
        case TpArrayUInt: return arrayEqual<uInt>(r1.asArrayuInt(id1),
                                                  r2.asArrayuInt(id2));
        default: return false;
        }
        
    case TpArrayFloat: 
        switch(r2.dataType(id2)) {
        case TpArrayFloat: return arrayEqual<Float>(r1.asArrayFloat(id1),
                                                    r2.asArrayFloat(id2));
        default: return false;
        }
        
    case TpArrayDouble: 
        switch(r2.dataType(id2)) {
        case TpArrayDouble: return arrayEqual<Double>(r1.asArrayDouble(id1),
                                                      r2.asArrayDouble(id2));
        default: return false;
        }
        
    case TpArrayComplex: 
        switch(r2.dataType(id2)) {
        case TpArrayComplex: return arrayEqual<Complex>(r1.asArrayComplex(id1),
                                                       r2.asArrayComplex(id2));
        default: return false;
        }
        
    case TpArrayDComplex: 
        switch(r2.dataType(id2)) {
        case TpArrayDComplex:
            return arrayEqual<DComplex>(r1.asArrayDComplex(id1),
                                        r2.asArrayDComplex(id2));
        default: return false;
        }
        
    case TpArrayString: 
        switch(r2.dataType(id2)) {
        case TpArrayString: return arrayEqual<String>(r1.asArrayString(id1),
                                                      r2.asArrayString(id2));
        default: return false;
        }
        
    default: return false;
    }
}

bool QtParamGUI::recordEqual(const RecordInterface& r1,
                             const RecordInterface& r2) {
    if(r1.nfields() != r2.nfields()) return false;
    for(unsigned int i = 0; i < r1.nfields(); i++) {
        if(r1.name(i) != r2.name(i) ||
           r1.dataType(i) != r2.dataType(i)) return false;
        
        switch(r1.dataType(i)) {
        case TpBool: return r1.asBool(i) == r2.asBool(i);
        case TpUChar: return r1.asuChar(i) == r2.asuChar(i);
        case TpShort: return r1.asShort(i) == r2.asShort(i);
        case TpInt: return r1.asInt(i) == r2.asInt(i);
        case TpUInt: return r1.asuInt(i) == r2.asuInt(i);
        case TpFloat: return r1.asFloat(i) == r2.asFloat(i);
        case TpDouble: return r1.asDouble(i) == r2.asDouble(i);
        case TpComplex: return r1.asComplex(i) == r2.asComplex(i);
        case TpDComplex: return r1.asDComplex(i) == r2.asDComplex(i);
        case TpString: return r1.asString(i) == r2.asString(i);
        case TpRecord: return recordEqual(r1.asRecord(i), r2.asRecord(i));
        case TpArrayBool: return arrayEqual<Bool>(r1.asArrayBool(i),
                                                  r2.asArrayBool(i));
        case TpArrayUChar: return arrayEqual<uChar>(r1.asArrayuChar(i),
                                                    r2.asArrayuChar(i));
        case TpArrayShort: return arrayEqual<Short>(r1.asArrayShort(i),
                                                    r2.asArrayShort(i));
        case TpArrayInt: return arrayEqual<Int>(r1.asArrayInt(i),
                                                r2.asArrayInt(i));
        case TpArrayUInt: return arrayEqual<uInt>(r1.asArrayuInt(i),
                                                  r2.asArrayuInt(i));
        case TpArrayFloat: return arrayEqual<Float>(r1.asArrayFloat(i),
                                                    r2.asArrayFloat(i));
        case TpArrayDouble: return arrayEqual<Double>(r1.asArrayDouble(i),
                                                      r2.asArrayDouble(i));
        case TpArrayComplex: return arrayEqual<Complex>(r1.asArrayComplex(i),
                                                        r2.asArrayComplex(i));
        case TpArrayDComplex:return arrayEqual<DComplex>(r1.asArrayDComplex(i),
                                                        r2.asArrayDComplex(i));
        case TpArrayString: return arrayEqual<String>(r1.asArrayString(i),
                                                      r2.asArrayString(i));
        
        default: return false;
        }
    }
    
    return true;
}

template <class T>
bool QtParamGUI::arrayEqual(const Array<T>& a1, const Array<T>& a2) {
    if(a1.shape() != a2.shape()) return false;
    
    typename Array<T>::ConstIteratorSTL it1 = a1.begin(), it2 = a2.begin();
    while(it1 != a1.end()) {
        if(*it1 != *it2) return false;
        it1++;
        it2++;
    }
    
    return true;
}

bool QtParamGUI::readLastFile(Record& r, QFile& f, stringstream& invalid) {
    if(!f.exists()) {
        invalid << "\n* File does not exist!";
        return false;
    }
    if(!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        invalid << "\n* File cannot be opened!";
        return false;
    }
    
    bool valid = true;
    while(!f.atEnd()) {
        QString line = f.readLine().trimmed();
        if(line.startsWith('#')) continue;
        
        QStringList spl = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        if(spl.size() < 3) {
            valid = false;
            invalid << "\n* Line not in [param] = [value] form!";
            break;
        }
        
        // expected format: [param] = [value]
        String param = qPrintable(spl.takeFirst());
        if(param.empty()) continue;
        spl.takeFirst();
        
        // value types: int, double, bool, string, dcomplex,
        //    int array, double array, bool array, string array, dcomplex array
        
        // check for string
        if(spl[0].startsWith('"') || spl[0].startsWith('\'')) {
            // string
            int i = line.indexOf('"'); int j = line.indexOf('\'');
            if(i == -1 || (j != -1 && j < i)) i = j;
            QString str = line.right(line.size() - i - 1);
            str.remove(str.size() - 1, 1);
            r.define(param, qPrintable(str));
            
        } else if(spl[0].startsWith('[')) {
            // array
            int i = line.indexOf('[');
            QString str = line.right(line.size() - i - 1);
            
            if(str == "]") {
                // empty list, so guess...double?
                r.define(param, Vector<Double>(vector<double>()));
                
            } else if(str.startsWith("complex(")) {
                QStringList spl2 = str.split(',');
                if(spl2.size() % 2 != 0) {
                    invalid << "\n* Improperly formed complex array!";
                    valid = false;
                    break;
                }
                for(int i = 0; i < spl2.size(); i++) {
                    spl2[i] = spl2[i].trimmed();
                    if(i == spl2.size() - 1)
                        spl2[i].remove(spl2[i].size() - 1, 1);
                    if(i % 2 == 0) {
                        // complex(
                        if(!spl2[i].startsWith("complex(") ||
                           spl2[i].size() <= 8) {
                            invalid << "\n* Improperly formed complex array!";
                            valid = false;
                            break;
                        } else spl2[i].remove(0, 8);
                    } else {
                        // )
                        if(!spl2[i].endsWith(')') || spl2[i].size() <= 1) {
                            invalid << "\n* Improperly formed complex array!";
                            valid = false;
                            break;
                        } else spl2[i].remove(spl2[i].size() - 1, 1);
                    }
                }
                
                vector<DComplex> v;                
                for(int i = 0; i < spl2.size(); i += 2)
                    v.push_back(DComplex(spl2[i].toDouble(),
                                         spl2[i + 1].toDouble()));                
                r.define(param, Vector<DComplex>(v));
            } else {                
                QStringList spl2 = str.split(',');
                for(int i = 0; i < spl2.size(); i++) {
                    spl2[i] = spl2[i].trimmed();        // get rid of spaces
                    if(i == spl2.size() - 1)
                       spl2[i].remove(spl2[i].size() - 1, 1); // and ]'s                                    
                }
                
                if(spl2[0].startsWith('"') || spl2[0].startsWith('\'')) {
                    // string vector
                    vector<String> v;
                    for(int i = 0; i < spl2.size(); i++) {
                        spl2[i].remove(0, 1);
                        spl2[i].remove(spl2[i].size() - 1, 1);
                        v.push_back(qPrintable(spl2[i]));
                    }
                    
                    r.define(param, Vector<String>(v));
                    
                } else if(str[0].isNumber() || (str[0] == '-' &&
                          str.size() >= 2 && str[1].isNumber())) {
                    if(spl2[0].contains('.')) {
                        vector<Double> v;                        
                        for(int i = 0; i < spl2.size(); i++)
                            v.push_back(spl2[i].toDouble());                        
                        r.define(param, Vector<Double>(v));
                    } else {
                        vector<Int> v;                        
                        for(int i = 0; i < spl2.size(); i++)
                            v.push_back(spl2[i].toInt());                        
                        r.define(param, Vector<Int>(v));
                    }
                } else {
                    vector<Bool> v;
                    
                    for(int i = 0; i < spl2.size(); i++) {
                        spl2[i] = spl2[i].toLower();
                        if(spl2[i] == "t" || spl2[i] == "true")
                            v.push_back(true);
                        else if(spl2[i] == "f" || spl2[i] == "false")
                            v.push_back(false);
                        else {
                            valid = false;
                            invalid<< "\n* Cannot read: "<< qPrintable(spl[i]);
                            break;
                        }
                    }
                    
                    r.define(param, Vector<Bool>(v));
                }                
            }
            
        } else if(spl[0].startsWith("complex(")) {
            int i = line.indexOf("complex(");
            QString str = line.right(line.size() - i - 8);
            str.remove(str.size() - 1, 1);
            
            QStringList spl2 = str.split(',');
            if(spl2.size() >= 2) {
                r.define(param, DComplex(spl2[0].trimmed().toDouble(),
                                         spl2[1].trimmed().toDouble()));
            } else {
                valid = false;
                invalid << "\n* Complex value incorrectly formatted.";
            }
            
        } else if(spl[0][0].isNumber() || (spl[0][0] == '-' &&
                  spl[0].size() > 1 && spl[0][1].isNumber())) {
            // double or int
            QString str = spl[0];
            if(str.contains('.')) r.define(param, str.toDouble());
            else r.define(param, str.toInt());
            
        } else {
            QString str = spl[0].toLower();
            if(str == "t" || str == "true") r.define(param, true);
            else if(str == "f" || str == "false") r.define(param, false);
            else {
                valid = false;
                invalid << "\n* Cannot read: " << qPrintable(line);
            }
        }
    }
    
    return valid;
}

/*
TpBool
TpChar
TpUChar
TpShort
TpUShort
TpInt
TpUInt
TpFloat
TpDouble
TpComplex
TpDComplex
TpString
TpRecord
TpArrayBool
TpArrayChar
TpArrayUChar
TpArrayShort
TpArrayUShort
TpArrayInt
TpArrayUInt
TpArrayFloat
TpArrayDouble
TpArrayComplex
TpArrayDComplex
TpArrayString
 */

}
