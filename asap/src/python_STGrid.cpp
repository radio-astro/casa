//#---------------------------------------------------------------------------
//# python_STGrid.cc: python exposure of c++ STGrid class
//#---------------------------------------------------------------------------
#include <string>
#include <vector>

#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "STGrid.h"
//#include "STGridWrapper.h"

using namespace boost::python;

namespace asap {
  namespace python {

void python_STGrid() {
  //class_<STGridWrapper>("stgrid")
  class_<STGrid>("stgrid")
    .def( init <> () )
    .def( init < const std::string > () )
    .def("_setif", &STGrid::setIF)
    .def("_setpollist", &STGrid::setPolList)
    .def("_defineimage", &STGrid::defineImage)
    .def("_setoption", &STGrid::setOption)
    .def("_grid", &STGrid::grid)
    .def("_setin", &STGrid::setFileIn)
    .def("_setweight", &STGrid::setWeight)
    .def("_save", &STGrid::saveData)
    ;
};

  } // python
} // asap
