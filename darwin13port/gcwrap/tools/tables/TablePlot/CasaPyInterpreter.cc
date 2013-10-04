//# CasaPyInterpreter.cc: Manage flag versions
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000,2001,2002,2003
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
//# $Id$



//# Includes

#include <cmath>
#include <casa/Exceptions/Error.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/stdio.h>

#include <casa/Exceptions.h>

#include <casa/Utilities/DataType.h>
#include <casa/System/ProgressMeter.h>

#include <tools/tables/TablePlot/CasaPyInterpreter.h>
#include <numpy/arrayobject.h>

namespace casa { //# NAMESPACE CASA - BEGIN

extern void initPyBind(void);

#define LOG0 0

String CasaPyInterpreter::clname = "CasaPyInterpreter";
/*********************************************************************************/
CasaPyInterpreter::CasaPyInterpreter(Bool usegui)
{
   String fnname= "CasaPyInterpreter"; 
   log = SLog::slog();   
   // Start connecting to the currently running python interpreter
   interp = PyModule_GetDict(PyImport_AddModule("__main__"));
   if(!PyDict_GetItemString(interp, "__builtins__"))
   {
      PyObject *builtinMod = PyImport_ImportModule("__builtin__");
      log->out("Loading builtins", fnname, clname, LogMessage::DEBUGGING);
      if(!builtinMod || 
         PyDict_SetItemString(interp, "__builtins__", builtinMod))
      {
         log->out("Fail: load builtins", fnname, clname, LogMessage::DEBUGGING);
      }
      PyObject *plotlib = PyImport_ImportModule("pylab");
      if(PyDict_SetItemString(interp, "pl", plotlib))
         log->out("Fail: load pl", fnname, clname, LogMessage::DEBUGGING);
         Py_DECREF(builtinMod);
         Py_DECREF(plotlib);
      }
    if(!PyDict_GetItemString(interp, "pl")){
      PyObject *plotlib = PyImport_ImportModule("pylab");
      if(PyDict_SetItemString(interp, "pl", plotlib))
         log->out("Fail: load pl", fnname, clname, LogMessage::DEBUGGING);
         Py_DECREF(plotlib);
    }
      // Connected to the currently running python interpreter

      // Initialize/import the Numeric::arrayobject module
      import_array();

#if LOG0
      pyrunString("print dir()\n" ); 
      pyrunString("print 'Interpreter created from TPPlotter !'\n" );
#endif
        
      if(usegui) 
         pyrunString("pl.ion()\n" );
      else 
         pyrunString("pl.ioff()\n");
        
      // Initialize the C++ - Python binder defined in PlotterGlobals.cc
      initPyBind();
        
      // Import the C++ - Python binder into the interpreter.
      pyrunString("import PyBind\n" );
        
      // Import the backend fudge code into the interpreter.
      // TablePlotTkAgg.py in code/xmlcasa/scripts  implements PlotFlag
#if LOG0 
      log->out("importing backend", fnname, clname, 
               LogMessage::DEBUGGING);
#endif 
      pyrunString("from TablePlotTkAgg import PlotFlag\n" );
        
        // Initialize PlotFlag.
#if LOG0 
      log->out("start backend", fnname, clname, 
               LogMessage::DEBUGGING);
#endif 
      pyrunString("pf = PlotFlag(PyBind);\n" );
        
      //os.DebugMessage( "get curr fig manager " );
      //pyrunString("figman = pl.get_current_fig_manager();\n" );
      //pyrunString("pf.setup_custom_features(figman);\n\n" );
        
#if LOG0 
      pyrunString("print dir()\n" );
#endif
        
        
      // Import modules that will be used for time-formatting.
      pyrunString("import time\n" );
      pyrunString("import datetime\n" );
      pyrunString("from matplotlib.ticker import MaxNLocator\n");
      pyrunString("from pylab import figure, show\n");
}

/*********************************************************************************/
CasaPyInterpreter::~CasaPyInterpreter()
{
   // TODO : perhaps this cleanup should come here, 
   //instead of being done inside TPLP...
   //pyrunString("del pf\n" ); 
   //pyrunString("del PlotFlag\n" ); 
   //pyrunString("del PyBind\n" ); 
}
/*********************************************************************************/
void CasaPyInterpreter::pyrunString( String cmd )
{
    PyRun_String( cmd.c_str(), Py_file_input, interp, interp );
    CheckPlotError(cmd);
}
/*********************************************************************************/
void CasaPyInterpreter::setupCustomGuiFeatures()
{
    // Get a handle to the current figure manager, and if required,
    // setup custom GUI features ( add buttons and bind them to callbacks )
    pyrunString("figman=pl.get_current_fig_manager();\n\n" );
    pyrunString(
       "if(pf.newtoolbar is True):pf.setup_custom_features(figman);\n\n" );
}

/*********************************************************************************/
void CasaPyInterpreter::CheckPlotError( String cmd )
{
   PyObject *exc=NULL;
   exc = PyErr_Occurred();
   if(exc!=NULL) 
   {
      PyErr_Print();
      CasaPyInterpreterError(String("From Python : SEVERE : \n")+ 
             cmd + String(" : "));
   }
}

/*********************************************************************************/
/*********************************************************************************/
void CasaPyInterpreter::CasaPyInterpreterError(String msg)
{
   throw AipsError("CasaPyInterpreter: " + msg);
}
/*********************************************************************************/

} //# NAMESPACE CASA - END 

