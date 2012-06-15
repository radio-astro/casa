//# PythonInterpreter.h: Interface for connecting and running tasks in Python.
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
#ifndef PYTHONINTERPRETER_H_
#define PYTHONINTERPRETER_H_

#include <Python.h>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

// Interface for connecting to and running tasks in Python.
class PythonInterpreter {
public:
    // Constructor.  If ipythonShell is NULL, then the connection is in
    // standalone mode -- i.e., a new casapy instance will be created.
    // If ipythonShell is not NULL, it is assumed to be either an
    // IPython.ipapi.IPApi instance (accesible as _ip in casapy) or an
    // IPython.iplib.InteractiveShell object (accessible as _ip.IP in casapy).
    PythonInterpreter(PyObject* ipythonShell);
    
    // Destructor.
    ~PythonInterpreter();
    
    // Initialize the connection.  In standalone mode, creates the python
    // interpreter, runs casapy, and then connects to it.  showLogger has no
    // effect if not in standalone mode.
    bool init(bool showLogger = true);
    
    // Run the given command in python.
    void command(String command);
    
private:
    // Whether the connection is initialized.
    bool m_initialized;
    
    // Whether the connection is standalone mode or not.
    bool m_standalone;
    
    // Handle to the ipython shell or the standalone interpreter.
    PyObject* m_shell;
    
    // Returns the aips path.
    static String aipsPath() {
#ifndef AIPSROOT
        char* ap = getenv("CASAPATH");
        if(ap == NULL) return String();
        String path(ap);
        unsigned int i = path.find(' ');
        if(i >= path.size()) return String();
        else return path.substr(0, i);
#else
        return AIPSROOT;
#endif
    }
    
    // Returns the casapy python path.
    static String casapyPath() {
#ifdef AIPSROOT
#ifdef AIPSARCH
#ifdef PYTHONVER
        stringstream ss;
        ss << AIPSROOT << '/' << AIPSARCH << "/python/" << PYTHONVER;
        return ss.str();
#else
        return String();
#endif
#else
        return String();
#endif
#else
        return String();
#endif
    }
    
    // Returns the python path.
    static String pythonPath() {
#ifdef PYTHONROOT
        return String(PYTHONROOT);
#else
        return String();
#endif
    }
    
    // Returns the location of the casapy.py script.
    static String casapyLoc() {
        String p = casapyPath();
        if(!p.empty()) p += "/casapy.py";
        return p;
    }
};

}

#endif /*PYTHONINTERPRETER_H_*/
