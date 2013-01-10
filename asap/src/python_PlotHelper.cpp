//#---------------------------------------------------------------------------
//# python_PlotHelper.cpp: python exposure of c++ PlotHelper class
//#---------------------------------------------------------------------------
//# Author: Kanako Sugimoto, (C) 2012
//#
//# Copyright: See COPYING file that comes with this distribution
//#
//#---------------------------------------------------------------------------
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "PlotHelper.h"
//#include "ScantableWrapper.h"

using namespace boost::python;

namespace asap {
  namespace python {

void python_PlotHelper() {
  class_<PlotHelper>("plothelper")
    .def( init <> () )
    .def( init <ScantableWrapper> () )
    .def("set_scan", &PlotHelper::setScantable)
    .def("set_gridval", &PlotHelper::setGridParamVal,
         (boost::python::arg("epoch")="J2000",
          boost::python::arg("projname")="SIN"))
    .def("set_grid", &PlotHelper::setGridParam,
         (boost::python::arg("cellx")="",
          boost::python::arg("celly")="",
          boost::python::arg("center")="",
	  boost::python::arg("projname")="SIN") )
    .def("get_gpos", &PlotHelper::getGridPixel,
         (boost::python::arg("whichrow")=0) )
    //
    .def("get_gref", &PlotHelper::getGridRef)
    .def("get_gcellval", &PlotHelper::getGridCellVal)
    /** TODO
    .def("get_gcell", &PlotHelper::getGridCell)
    .def("get_gdir", &PlotHelper::getGridDirection)
    .def("get_gdirval", &PlotHelper::getGridWorld)
    **/
    ;

};

  } // python
} // asap
