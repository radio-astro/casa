//# QtParamGUI.qo.h: GUI for collecting user input for task parameters.
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
#ifndef QTPARAMGUI_QO_H_
#define QTPARAMGUI_QO_H_

#include <Python.h>

#include <casaqt/QtParamGui/PythonInterpreter.h>
#include <casaqt/QtParamGui/QtParamGUI.ui.h>

#include <casa/Containers/Record.h>

#include <utility>

#include <QDialog>
#include <QFile>

#include <casa/namespace.h>
using namespace std;

namespace casa {

class ParamPanel;

class QtParamGUI : public QDialog, Ui::ParamInput {
    Q_OBJECT
    
public:
    // STATIC //
    
    // The running mode.
    enum Mode {
        PYTHON, SIGNAL
    };
    
    // Possible reset values.
    enum ResetMode {
        DEFAULT, GLOBAL, LAST
    };
    
    // String constants used in parsing the parameter records.
    // <group>
    static const String ALLOWED;
    static const String ANY;
    static const String CONSTRAINTS;
    static const String DEFAULTS;
    static const String DESCRIPTION;
    static const String EXAMPLE;
    static const String KIND;
    static const String KIND_FILE;
    static const String KIND_MS;
    static const String KIND_TABLE;
    static const String LIMITTYPES;
    static const String MUSTEXIST;
    static const String PARAMETERS;
    static const String PARAMETER_ORDER;
    static const String RANGE;
    static const String RANGE_MIN;
    static const String RANGE_MAX;
    static const String SHORT_DESCRIPTION;
    static const String SUBPARAMETERS;
    static const String TYPE;
    static const String UNITS;
    static const String VALUE;
    // </group>
    
    // Returns the type for the given type name.
    static DataType type(String typeName);
    
    // Returns the name for the given type.
    static String type(DataType type);
    
    // Converts the given record into a Python-like string representation.
    static String recordToString(const Record& record, String separator);
    
    // Converts the task name and parameter record into a Python command.
    static String pythonCommand(String taskName, const Record& params);
    
    // Sets up the given dialog to have the given name and given text.
    static void setupDialog(QDialog* dialog, String taskName, String textName,
                            const String& text);
    
    // Replaces all instances of \' and \" in the given string with ' and ",
    // respectively.
    static void replaceSlashQuotes(String& string);

    
    // NON-STATIC //
    
    // Constructor that takes the tasksRecord and builds the GUI.  Uses the
    // given mode and passes ipythonShell to the PythonInterpreter (see
    // PythonInterpreter constructor).  If the given globals record is
    // valid, any relevant values are used.  If the given parent is NULL, the
    // GUI is in dialog form whereas if a parent is given, it can be
    // embedded in another widget.
    // If the tasksRecord contains more than one entry, a QComboBox is
    // displayed at the top of the widget to choose between the different
    // possible tasks.
    QtParamGUI(const Record& tasksRecord, Mode mode = PYTHON,
               PyObject* ipythonShell = NULL, const Record* globals = NULL,
               QWidget* parent = NULL);
    
    // Destructor.
    ~QtParamGUI();
    
    // Returns the name of the currently displayed task.
    String taskName() const;
    
    // Returns a copy of the parameter attributes record for this task.
    Record taskParameters() const;
    
    // Returns a copy of the constraints record for this task.
    Record taskConstraints() const;
    
    // Returns the currently entered parameter names/values in a Record.
    Record enteredParameters() const;
    
signals:
    // In SIGNAL mode, this is emitted when the user clicks "run".  The name
    // of the task and a copy of the entered parameter names/values are given.
    // It is the responsibility of the parent/caller to close the QtParamGUI
    // widget as desired.
    void runRequested(String taskName, Record parameters);
    
    // In SIGNAL mode, this is emitted when the user clicks "cancel".
    // It is the responsibility of the parent/caller to close the QtParamGUI
    // widget as desired.
    void cancelRequested();
    
private:
    // All tasks and their parameters/attributes.
    Record m_tasks;
    
    // Global parameter values.
    Record m_globals;
    
    // Last parameter values.
    Record m_last;
    
    // When the user clicks "reset", indicates whether to reset to globals
    // or defaults or last.
    ResetMode m_resetMode;
    
    // Mode.
    Mode m_mode;
    
    // Description dialog to show/hide.
    QDialog* m_descDialog;
    
    // Example dialog to show/hide.
    QDialog* m_exampleDialog;
    
    // Constraints dialog to show/hide.
    // QDialog* m_constDialog;
    
    // Python interpreter.  Never used in SIGNAL mode.
    PythonInterpreter m_python;
    
    // Parameter panels for currently displayed task.
    vector<ParamPanel*> m_panels;
    
    // Currently displayed task.
    String m_taskName;

    
    // Sets the tasks record to the given, and then displays the task with
    // the given RecordFieldId.
    void setRecord(const Record& record, RecordFieldId id = 0);
    
    // Checks the value in the record at the given RecordFieldId for
    // validity.  If invalid, the second value in the pair holds the reason(s)
    // why.
    pair<bool, String> recordIsValid(const Record& record, RecordFieldId id);
    
    // Once the displayed task has been chosen, set up the panels.  If an
    // order is given, use that instead of the record's natural order.
    void setupRecord(vector<String> order);
    
    // Returns the text for the constraints dialog for the given param panel.
    String constraintsString(const String& id);
    
    // Show/hide the frame and radio buttons for the different reset modes.
    void showHideResets(bool globals, bool last);
    
    
    // STATIC
    
    // After this length, move the short description to two lines instead
    // of one.
    static const unsigned int MAX_SHORTDESC_LENGTH;
    
    // Trims the given String of leading and trailing whitespace, where
    // whitespace is ' ', '\t', '\n', and '\r'.
    static String strTrim(const String& str);
    
    // Trims the given String of leading newlines and trailing whitespace,
    // where newlines are '\n' or '\r' and whitespace is ' ', '\t', '\n', or
    // '\r'.
    static String strNLtrim(const String& str);
    
    // Returns true if the value at r1[id1] is equal to the value at
    // r2[id2].  If allowStrings is false, String values can be compared
    // to non-String values.
    static bool valuesEqual(const RecordInterface& r1, RecordFieldId id1,
                            const RecordInterface& r2, RecordFieldId id2,
                            bool allowStrings = false);
    
    // Returns true if the two records are equal or not.
    static bool recordEqual(const RecordInterface& r1,
                            const RecordInterface& r2);
    
    // Returns true if the two arrays are equals or not.
    template <class T>
    static bool arrayEqual(const Array<T>& a1, const Array<T>& a2);
    
    // Reads the python commands from the given file into the given record.
    // The stringstream is used to note any errors during read.
    static bool readLastFile(Record& rec, QFile& file, stringstream& ss);
    
private slots:
    // When the user clicks "run".
    void run();
    
    // When the user clicks "reset".
    void reset();
    
    // When the user clicks "cancel".
    void cancel();
    
    // When the value for the given parameter changes.  If constraints are
    // given, check against those.
    void valueChanged(String paramName);
    
    // Show/hide the description dialog.
    void showHideDesc(bool checked);
    
    // Show/hide the example dialog.
    void showHideExample(bool checked);
    
    // When the description dialog is closed.
    void descClosed();
    
    // When the example dialog is closed.
    void exampleClosed();
    
    // When the user chooses a different task.
    void taskChosen(QString task);
    
    // When the user changes the reset option.
    void resetChanged();
    
    // When the user loads parameters from a .last file.
    void loadLast();
};

}

#endif /*QTPARAMGUI_QO_H_*/
