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
    .def( init < const std::vector<std::string> > () )
    .def("_setif", &STGrid::setIF)
    .def("_setpollist", &STGrid::setPolList)
    .def("_setscanlist", &STGrid::setScanList)
    .def("_defineimage", &STGrid::defineImage)
    .def("_setfunc", &STGrid::setFunc)
    .def("_grid", &STGrid::grid)
    .def("_setin", &STGrid::setFileIn)
    .def("_setfiles", &STGrid::setFileList)
    .def("_setweight", &STGrid::setWeight)
    .def("_enableclip", &STGrid::enableClip) 
    .def("_disableclip", &STGrid::disableClip) 
    .def("_save", &STGrid::saveData)
    ;
};

  } // python
} // asap
