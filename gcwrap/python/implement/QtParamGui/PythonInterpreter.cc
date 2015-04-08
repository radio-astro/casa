//# PythonInterpreter.cc: Interface for connecting and running tasks in Python.
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
#include <casaqt/QtParamGui/PythonInterpreter.h>
#include <Python.h>

namespace casa {

// PYTHONINTERPRETER DEFINITIONS //

PythonInterpreter::PythonInterpreter(PyObject* shell) : m_shell(shell) {
    m_initialized = m_shell != NULL;
    m_standalone = m_shell == NULL;
    if(m_shell != NULL) Py_DECREF(m_shell);
}

PythonInterpreter::~PythonInterpreter() {
    if(m_standalone && m_initialized) {
        // can't call Py_Finalize because destruction of modules and objects is
        // random and causes segfaults after the logger is destroyed
        // Py_Finalize();
        
        // call taskmanager::finalize because otherwise a stray process sticks
        // around indefinitely
        PyRun_String("tm.finalize()", Py_file_input, m_shell, m_shell);
    } else if(!m_standalone && m_shell != NULL) {
        PyGILState_STATE gstate;
        gstate = PyGILState_Ensure();
        Py_DECREF(m_shell);
        PyGILState_Release(gstate);
    }
}

bool PythonInterpreter::init(bool showLogger) {
    if(m_shell == NULL && m_standalone && !m_initialized) {
        try {
            String casapylib = pythonPath();
            if(casapylib.empty()) return false;
            
            // use the right python
            stringstream ss;
            ss << casapylib << "/bin";
            char* p = getenv("PATH");
            if(p) ss << ':' << p;            
            setenv("PATH", ss.str().c_str(), 1);
    
            Py_Initialize();
    
            String cp = casapyPath();
            if(!cp.empty()) {
                // set up environment for casapy
                stringstream commands;
                commands << "import sys";
                commands << "\nsys.path += ['" << cp << "', '" << casapylib;
                commands << "/lib/python2.5', '" << casapylib;
                commands << "/lib/python2.5/site-packages']\nsys.argv = [";
                if(!showLogger) commands << "'--nologger', ";
                commands << "'--noipython']\nexecfile('" << cp;
                commands << "/casapy.py')";
    
                PyRun_SimpleString(commands.str().c_str());
                
                // grab handle to interpreter
                m_shell = PyModule_GetDict(PyImport_AddModule("__main__"));           
            }
            m_initialized = Py_IsInitialized();        
        } catch(...) { m_initialized = false; }
    }
    return m_initialized;
}

void PythonInterpreter::command(String command) {
    if(m_initialized) {
        stringstream ss;
        // print then run the command
        ss << "print \"--> " << command << "\"\n" << command;
        if(m_standalone)
            PyRun_String(ss.str().c_str(), Py_file_input, m_shell, m_shell);
        else {
            PyGILState_STATE gstate;
            gstate = PyGILState_Ensure();
            PyObject_CallMethod(m_shell, "runlines", "s", ss.str().c_str());
            PyGILState_Release(gstate);
        }
    }
}

}
