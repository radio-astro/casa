//#---------------------------------------------------------------------------
//# python_PlotHelper.cc: python exposure of c++ PlotHelper class
//#---------------------------------------------------------------------------
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include <string>
#include <vector>

#include "PlotHelper.h"
#include "ScantableWrapper.h"

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
//     .def("set_gird", &PlotHelper::setGridParam,
//          (boost::python::arg("scellx")="",
//           boost::python::arg("scelly")="",
//           boost::python::arg("scenter")=""))
    .def("get_gpos", &PlotHelper::getGridPixel,
         (boost::python::arg("whichrow")=0) )
    /** TODO
    .def("get_gdir", &PlotHelper::getGridDirection)
    .def("get_gdirval", &PlotHelper::getGridWorld)
    **/
    ;

};

  } // python
} // asap
