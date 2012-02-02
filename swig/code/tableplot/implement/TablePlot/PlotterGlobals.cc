//# PlotterGlobals.cc: Class for GUI binding
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
//#
//#
//# PlotterGlobals Change Log
//# =======================
//# Data   Name      Description
//# May 2007    Urvashi R.V.    Created to allow GUI events (button presses) 
//#                             to be bound to C++ call-backs.
//#

//# Includes

#include <Python.h>
#include <iostream>
#include <casa/aipstype.h>
#include <casa/Arrays/Vector.h>

#include <tableplot/TablePlot/TablePlot.h>
#include <tableplot/TablePlot/SLog.h>

#define LOG0 0

namespace casa {

// <summary>
// Description : Implements the connection between GUI events and C++ callbacks.
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
//   --------------------------------------------------------------------------------------
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


// Instantiated in TablePlot.cc
extern casa::TPGuiBinder* GBB;
        
// Python-binding globals
PyObject *plotx_p,*ploty_p;
void initPyBind(void);
static PyObject* PyBind_readXdata(PyObject *self, PyObject* args);
static PyObject* PyBind_readYdata(PyObject *self, PyObject* args);
static PyObject* PyBind_markregion(PyObject *self, PyObject* args);
static PyObject* PyBind_flagdata(PyObject *self, PyObject* args);
static PyObject* PyBind_unflagdata(PyObject *self, PyObject* args);
static PyObject* PyBind_locatedata(PyObject *self, PyObject* args);
static PyObject* PyBind_iterplotnext(PyObject *self, PyObject* args);
static PyObject* PyBind_iterplotstop(PyObject *self, PyObject* args);
static PyObject* PyBind_clearplot(PyObject *self, PyObject* args);
static PyObject* PyBind_quit(PyObject *self, PyObject* args);


// Declare methods 
static PyMethodDef PyBind_methods[] = {

   {"readXdata",   PyBind_readXdata, METH_NOARGS,
    "Return x data."},
   {"readYdata",   PyBind_readYdata, METH_NOARGS,
    "Return y data."},
   {"markregion",   PyBind_markregion, METH_VARARGS,
    "Mark a region."},
   {"flagdata",   PyBind_flagdata, METH_NOARGS,
    "Flag data."},
   {"unflagdata",   PyBind_unflagdata, METH_NOARGS,
    "Unflag data."},
   {"locatedata",   PyBind_locatedata, METH_NOARGS,
    "Locate data."},
   {"iterplotnext",PyBind_iterplotnext, METH_NOARGS,
    "Iteration Plot : Next."},
   {"iterplotstop",PyBind_iterplotstop, METH_NOARGS,
    "Iteration Plot : Stop."},
   {"clearplot",PyBind_clearplot, METH_NOARGS,
    "Clear Plot"},
   {"quit",PyBind_quit, METH_VARARGS,
    "Quit"},
   {NULL,      NULL}      // sentinel 
};

// Initialize the PythonBinder
void
initPyBind(void)
{
   PyImport_AddModule("PyBind");
   Py_InitModule("PyBind", PyBind_methods);
}

// Send out a Python list to python 
// Used to memory-map PyArrayObjects in TPPlotter, to the python namespace.
static PyObject *
PyBind_readXdata(PyObject *self, PyObject* args)
{
   return plotx_p;
}
static PyObject *
PyBind_readYdata(PyObject *self, PyObject* args)
{
   return ploty_p;
}
///*********** Mark Region ************/
static PyObject *
PyBind_markregion(PyObject *self, PyObject* list)
{
   int len;
   double xmin,xmax,ymin,ymax;
   int panelnumber, nrows,ncols;

   PyObject *python_list;
   PyObject *elt_list;
   PyArg_ParseTuple (list,"O!", &PyList_Type,&python_list) ; 
   
   /* Number of regions */
   len = PyList_Size(python_list);

   if(len>0);
   
   Vector<double> gvec(4);
   for(int i=0;i<len;i++)
   {
      elt_list = PyList_GetItem(python_list,i);

      /* This is sent in to markRegions() */
      xmin = PyFloat_AsDouble(PyList_GetItem(elt_list,0));
      ymin = PyFloat_AsDouble(PyList_GetItem(elt_list,1));
      xmax = PyFloat_AsDouble(PyList_GetItem(elt_list,2));
      ymax = PyFloat_AsDouble(PyList_GetItem(elt_list,3));
      panelnumber = (int)PyFloat_AsDouble(PyList_GetItem(elt_list,4));
      nrows = (int)PyFloat_AsDouble(PyList_GetItem(elt_list,5));
      ncols = (int)PyFloat_AsDouble(PyList_GetItem(elt_list,6));

      gvec(0)=xmin; gvec(1)=xmax; gvec(2)=ymin; gvec(3)=ymax;
   
#if LOG0
      ostringstream os;
      os << "Recorded : [" << nrows << "," << ncols << "," 
         << panelnumber << "] : "  << gvec;
      SLog::slog()->out(os, "PyBind_markregion", "PlotGlobals",
                        LogMessage::DEBUGGING);
#endif
      if(GBB != NULL) 
         GBB->markregion(nrows,ncols,panelnumber,gvec);
      else 
         SLog::slog()->out("Binder is NULL", "PyBind_markregion", 
             "PlotGlobals", LogMessage::WARN);
   }

   Py_INCREF(python_list);
   return python_list;

}
///*********** Flag Data ************/
static PyObject *
PyBind_flagdata(PyObject *self, PyObject* args)
{
   if(GBB != NULL) GBB->flagdata();
   else 
      SLog::slog()->out("Binder is NULL", "PyBind_flagdata", 
             "PlotGlobals", LogMessage::WARN);
    
   return Py_BuildValue("i", 1);
}
///*********** Unflag Data ************/
static PyObject *
PyBind_unflagdata(PyObject *self, PyObject* args)
{
   if(GBB != NULL) GBB->unflagdata();
   else
      SLog::slog()->out("Binder is NULL", "PyBind_unflagdata", 
             "PlotGlobals", LogMessage::WARN);
   return Py_BuildValue("i", 1);
}
///*********** Locate Data ************/
static PyObject *
PyBind_locatedata(PyObject *self, PyObject* args)
{
   if(GBB != NULL) GBB->locatedata();
   else
      SLog::slog()->out("Binder is NULL", "PyBind_locatedata", 
             "PlotGlobals", LogMessage::WARN);
   return Py_BuildValue("i", 1);
}
///*********** IterPlotNext ************/
static PyObject *
PyBind_iterplotnext(PyObject *self, PyObject* args)
{
   if(GBB != NULL) GBB->iterplotnext();
   else
     SLog::slog()->out("Binder is NULL", "PyBind_iterplotnext", 
             "PlotGlobals", LogMessage::WARN);
   return Py_BuildValue("i", 1);
}
///*********** IterPlotStop ************/
static PyObject *
PyBind_iterplotstop(PyObject *self, PyObject* args)
{
   if(GBB != NULL) GBB->iterplotstop();
   else
     SLog::slog()->out("Binder is NULL", "PyBind_iterplotstop", 
             "PlotGlobals", LogMessage::WARN);
   return Py_BuildValue("i", 1);
}
///*********** ClearPlot ************/
static PyObject *
PyBind_clearplot(PyObject *self, PyObject* args)
{
   if(GBB != NULL) GBB->clearplot();
   else 
     SLog::slog()->out("Binder is NULL", "PyBind_clearplot", 
             "PlotGlobals", LogMessage::WARN);
   return Py_BuildValue("i", 1);
}
///*********** Quit ************/
static PyObject *
PyBind_quit(PyObject *self, PyObject* args)
{
        int closewin;
   PyArg_ParseTuple (args,"i", &closewin) ; 
        
        //cout << "from PyBind_quit() : closewin : " << closewin << endl;
        
   if(GBB != NULL) GBB->quit(closewin);
   else
     SLog::slog()->out("Binder is NULL", "PyBind_quit", 
             "PlotGlobals", LogMessage::WARN);
   return Py_BuildValue("i", 1);
}

};

