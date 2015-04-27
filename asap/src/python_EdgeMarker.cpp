//#---------------------------------------------------------------------------
//# python_EdgeMarker.cc: python exposure of c++ EdgeMarkerWrapper class
//#---------------------------------------------------------------------------
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include <string>
#include <vector>

#include "EdgeMarkerWrapper.h"
#include "ScantableWrapper.h"

using namespace boost::python;

namespace asap {
  namespace python {

void python_EdgeMarker() {
  class_<EdgeMarkerWrapper>("_edgemarker")
    .def( init <> () )
    .def( init < const bool > () )
    .def("_setdata", &EdgeMarkerWrapper::setdata)
    .def("_setoption", &EdgeMarkerWrapper::setoption)
    .def("_get", &EdgeMarkerWrapper::get)
    .def("_examine", &EdgeMarkerWrapper::examine)
    .def("_detect", &EdgeMarkerWrapper::detect)
//     .def("_reset", &EdgeMarkerWrapper::reset)
    .def("_mark", &EdgeMarkerWrapper::mark)
    ;
    
};

  } // python
} // asap
