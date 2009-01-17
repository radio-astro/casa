//# paramgui_python.cc: Python module for accessing parameter GUI.
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
#include <Python.h>
#include <casaqt/QtParamGui/QtParamGUI.qo.h>
#include <xmlcasa/xerces/stdcasaXMLUtil.h>
#include <xmlcasa/StdCasa/CasacSupport.h>
#include <xmlcasa/conversions_python.h>
#include <fstream>
#include <QDir>

extern "C" {
  void paramgui_runTask_priv(const char* taskName, PyObject* ipythonShell);
  void paramgui_runAll_priv(PyObject* ipythonShell);
}

class PGHelper {
public:
    PGHelper() : argc(0), argv(NULL), app(new QApplication(argc, argv)) { }
    
    ~PGHelper() {
        app->quit();
        // TODO why segfault?
        // delete app;
    }
    
private:
    int argc;
    char** argv;
    
public:
    QApplication* app;
    Record globalParams;
};

static PGHelper helper;

void paramgui_runTask_priv(const char* taskName, PyObject* ipythonShell) {
    String path = getenv("CASAPATH");
    path = path.substr(0, path.find(' ')) + "/share/xml/" + taskName + ".xml";
  
    fstream fin;
    fin.open(path.c_str(), ios::in);
    bool valid = fin.is_open();
    fin.close();
  
    if(valid) {      
        casac::stdcasaXMLUtil xmlUtils;
        casac::record r;
        xmlUtils.readXMLFile(r, path);
        Record* rec = toRecord(r);
        QtParamGUI pg(*rec, QtParamGUI::PYTHON, ipythonShell,
                      helper.globalParams.nfields() == 0 ?
                              NULL : &helper.globalParams);
        delete rec;
        pg.exec();
    }
}

static PyObject* paramgui_runTask_(PyObject*, PyObject* args, PyObject* kws) {
    const char* taskName;
    PyObject* ipython;

    static char* kwlist[] = {"taskName", "ipShell", NULL};
  
    if(!PyArg_ParseTupleAndKeywords(args, kws, "sO", kwlist,
                                    &taskName, &ipython)) return NULL;
    
    paramgui_runTask_priv(taskName, ipython);
    Py_INCREF(Py_None);
    return Py_None;
}

void paramgui_runAll_priv(PyObject* ipythonShell) {
    String path = getenv("CASAPATH");
    unsigned int i = path.find(' ');
    path = path.substr(0, i) += "/share/xml/";
    
    casac::stdcasaXMLUtil xmlUtils;
    casac::record r;
    QDir dir(path.c_str());
    QStringList files = dir.entryList(QStringList("*.xml"),
            QDir::Files);
    for(int i = 0; i < files.size(); i++) {
        String s = path + qPrintable(files[i]);
        xmlUtils.readXMLFile(r, s);
    }
    Record* rec = toRecord(r);
    QtParamGUI pg(*rec, QtParamGUI::PYTHON, ipythonShell,
                  helper.globalParams.nfields() == 0 ?
                          NULL : &helper.globalParams);
    delete rec;
    pg.exec();
}

static PyObject* paramgui_runAll_(PyObject*, PyObject* args, PyObject* kws) {
    PyObject* ipython;

    static char* kwlist[] = {"ipShell", NULL};
  
    if(!PyArg_ParseTupleAndKeywords(args, kws, "O", kwlist, &ipython))
            return NULL;
    
    paramgui_runAll_priv(ipython);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* paramgui_setGlobals_(PyObject*,PyObject* args,PyObject* kws) {
    PyObject* obj(NULL);
    static char* kwlist[] = {"globals", NULL};
    
    if(!PyArg_ParseTupleAndKeywords(args, kws, "O", kwlist, &obj)) return NULL;
    
    if(!PyDict_Check(obj)) {
        PyErr_SetString(PyExc_TypeError, "Globals must be a dict.");
        return NULL;
    }
    
    casac::variant v = casac::pyobj2variant(obj);
    
    Record* rec = toRecord(v.asRecord());
    helper.globalParams = *rec;
    delete rec;
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef ParamguiMethods[] = {
  { "runTask", (PyCFunction)paramgui_runTask_, METH_VARARGS | METH_KEYWORDS,
    PyDoc_STR("runTask(taskName[, ipShell]) -> None\n"
              "Run a parameter GUI for the given task using the given "
              "IPython shell.  The second argument is not necessary if "
              "the shell has already been set using setShell().\n"
              "Examples: runTask('almasimmos') or runTask('almasimmos', "
              "_ip) or runTask(taskName='almasimmos') or "
              "runTask(taskName='almasimmos', ipShell=_ip).") },
  { "runAll", (PyCFunction)paramgui_runAll_, METH_VARARGS | METH_KEYWORDS,
    PyDoc_STR("runAll([ipShell]) -> None\n"
              "Run a parameter GUI for all tasks using the given "
              "IPython shell.  The argument is not necessary if the shell "
              "has already been set using setShell().\nExamples: "
              "runAll() or runAll(_ip) or runAll(ipShell=_ip).") },
  { "setGlobals",(PyCFunction)paramgui_setGlobals_,METH_VARARGS| METH_KEYWORDS,
    PyDoc_STR("setGlobals(globals) -> None\n"
              "Sets the global parameters (that override the defaults) to "
              "the given.\nExample: setGlobals(sys._getframe(0).f_globals) "
              "or setGlobals(globals=sys._getframe(0).f_globals)") },
  { NULL, NULL, 0, NULL }
};

PyDoc_STRVAR(module_doc, "Module for showing a parameter GUI for a task.");

PyMODINIT_FUNC initparamgui() {
    PyObject *m;
    m = Py_InitModule3("paramgui", ParamguiMethods, module_doc);
}
