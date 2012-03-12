//# CasaPyInterpreter.h: Maintain and manage different flag versions.
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
//# You should have receied a copy of the GNU Library General Public License
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
//#
//# ----------------------------------------------------------------------------
//# Change Log
//# ----------------------------------------------------------------------------
//# Date         Name             Comments
//# Aug 20     Urvashi R.V.      Created this class.


#ifndef CASAPYINTERPRETER_H
#define CASAPYINTERPRETER_H

//# Includes

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/OS/Timer.h>
#include <casa/OS/File.h>

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Array.h>

#include <tableplot/TablePlot/SLog.h>
#include <Python.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Class that grabs a handle onto the currently running
// casapy Python interpreter shell.
// </summary>

// <reviewed reviewer="" date="" tests="">
// </reviewed>

// <prerequisite>
//   <li> Extending/Embedding Python with C++
//   <li> python - matplotlib/pylab
// </prerequisite>

// <etymology>
// A link between Casa and a Python-Interpreter. 
// </etymology>

// <synopsis>
// This class creates and maintains a link between C++ and the
// current instance of the python interpreter. 
// ( Wes Young wrote the first part of the constructor, that creates
//   a local python interpreter, and loads "pylab" into it. )
// 
// All python commands that TPPlotter runs, are sent into this class
// for transmission to the python interpreter.
// </synopsis>

// <motivation>
// To isolate the process of connecting between C++ and Python.
// </motivation>

// <thrown>
//    <li>
//    <li>
// </thrown>


// <todo asof="$DATE:$">
//   <li> 
// </todo>

        
class CasaPyInterpreter 
{
   public:
       // Default Constructor
       CasaPyInterpreter(Bool usegui=True);
       // Copy Constructor
       CasaPyInterpreter( CasaPyInterpreter const& );
       // Assignment
       CasaPyInterpreter& operator=(CasaPyInterpreter const&);
       // Destructor
       ~CasaPyInterpreter();
       
       // Send in a python command string.
       // Be very wary of newlines and blank spaces 
       void pyrunString( String cmd );
       // Periodically check if the internal interpreter has thrown 
       // an exception. If so, grab the message and create an AipsError
       // exception.
       void CheckPlotError( String cmd );
       // Add Gui buttons and bind them.
       // Usually called immediately after construction, and only once.
       void setupCustomGuiFeatures();

   private:

       // Exception generator.
       void CasaPyInterpreterError(String msg);
       
       // Handle to the internal python interpreter.
       PyObject *interp;

       SLog* log;
       static String clname;
};

/* Documentation for PlotterGlobals. */

// <summary>
// Description for PlotterGlobals.cc : 
// Implements the connection between GUI events and C++ callbacks.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="">
// </reviewed>

// <prerequisite>
//   <li> Extending/Embedding Python and the C/Python API
//   <li> TPGuiBinder
// </prerequisite>

// <etymology>
// Global functions to handle Gui callback binding between Python and C++.
// </etymology>

// <synopsis>

//
// Description : Implements the connection between GUI events and C++ callbacks.
//   --------------------------------------------------------------------------------------
//    Flow of control is as follows.
//    
//    -> Button-press event                ( in Tkinter - matplotlib/python )
//      -> PlotFlag::flag()                ( in internal python interpreter )
//        -> PyBind::flagdata()            ( python/C++ callback )
//          -> TPGuiBinder::flagdata()     ( link to casa::application function )
//          -> TablePlot::flagData();   ( the actual flagging ! )
//
//   --------------------------------------------------------------------------------------
//
// (1) CREATE A NEW PYTHON MODULE - PyBind
// 
//    A set of global callback functions are defined. It uses the Python-C API described
//    in "http://docs.python.org/api/api.html" and "http://docs.python.org/ext/ext.html".
//    
//    These functions are initialized as a Python module (in C++), when its constructor is called.
//    -  Py_Initialize(); // Startup the internal python interpreter
//    -  initPyBind();    // constructor to create and bind this new module
//    -  Py_Run_Simple_String("import PyBind"); // import this module into python namespace.
//    The equivalent of this is done in TPPlotter::initPlot().
//
//    These functions are then accessible from within the internal python interpreter
//    created via Py_Initialize().
// 
// (2) DEFINE A C++ GUIBINDER CLASS - (example) MSPlotGuiBinder
//
//    See the definition of class TPGuiBinder in TablePlot.h.
//    A global instance of this TPGuiBinder pointer is
//    maintained, and the callbacks implemented in the PyBind module use it and call
//    its member functions. This way, TablePlot, can connect the
//    global generic PyBind callback functions to its member functions.
//
// (3) IMPLEMENT THE METHODS OF THE PYTHON MODULE - PyBind.flag(), PyBind.unflag(),...
//
//     Call member functions of TPGuiBinder from a global instance. This is needed
//     to give applications control over what functions are bound to.
//     This connects steps (1) and (2).
// 
// (4) PYTHON EVENT CAPTURE AND BINDING - PlotFlag.
//
//    The binding of the actual matplotlib-backend events to these callbacks is done in the
//    TablePlotTkAgg.py script. Buttons are added, and matplotlib callbacks are defined that
//    use the PyBind module (remember - PyBind is visible in the namespace of the internal
//    python interpreter ! ). 
//
// --------------------------------------------------------------------------------------
// Example : A simplified example based on TablePlot 
// 
//    In TablePlot.h ---> Implement a derived class of TPGuiBinder that calls the
//                      application-specific function. 
//            
//       -   class TPGuiBinder 
//   -   {
//   -      public :
//   -         TPGuiBinder( casa::TablePlot* intp ){itsTablePlot = intp;}
//   -         Bool flagdata(){ return itsTablePlot->flagData(FLAG); }
//   -      private :
//   -         casa::TablePlot* itsTablePlot;
//   -   };
//
//    In TablePlot.cc ---> A Global instance of TPGuiBinder is created.
//          
//   -   GBB = new TPGuiBinder(this);
//
//
//    In PyBind ( PlotterGlobals.cc ) ---> Define the PyBind method that calls
//                                         MSPlotGuiBinder::flagdata();
//                   GBB is the global instance of TPGuiBinder *. 
//
//   -   static PyObject *
//   -   PyBind_flagdata(PyObject *self, PyObject* args)
//   -   {
//   -      if(GBB != NULL) GBB->flagdata();
//   -      else cout << "Binder is NULL" << endl;
//   -      return Py_BuildValue("i", 1);
//   -   }
//
//    In TablePlotTkAgg.py ---> Bind the python Tk event to the PyBind callback in a
//                      python class called PlotFlag.
//
//   -    class PlotFlag:
//       -     # bind the Button "bFlag" to the command "self.flag".
//       -     self.toolbar.bFlag.config(command=self.flag);
//      -     def flag(self, *args):
//   -      self.PyBind.flagdata();
//   -
//
//  --------------------------------------------------------------------------------------
//  Again... Flow of control is as follows.
//    
//    -> Button-press event                ( in Tkinter - matplotlib/python )
//      -> PlotFlag::flag()                ( in internal python interpreter )
//        -> PyBind::flagdata()            ( python/C++ callback )
//          -> TPGuiBinder::flagdata()     ( link to casa::application function )
//          -> TablePlot::flagData();   ( the actual flagging ! )
//
// </synopsis>

// <example>
//
// <srcblock>
// 
// </srcblock>
// </example>

// <motivation>
// To generate callbacks from the matplotlib GUI to C++. 
// </motivation>

// <templating arg=T>
//    <li>
// </templating>

// <thrown>
//    <li>
//    <li>
// </thrown>


// <todo asof="$DATE:$">
//   <li> 
// </todo>

// See tables/TablePlot/PlotterGlobals.cc


} //# NAMESPACE CASA - END 

//#ifndef AIPS_NO_TEMPLATE_SRC
//#include <tableplot/TablePlot/CasaPyInterpreter.tcc>
//#endif //# AIPS_NO_TEMPLATE_SRC
#endif

