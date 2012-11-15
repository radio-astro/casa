//#---------------------------------------------------------------------------
//# python_STGrid.cc: python exposure of c++ STGrid class
//#---------------------------------------------------------------------------
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include <string>
#include <vector>

#include "STGrid.h"
#include "ScantableWrapper.h"

using namespace boost::python;

namespace asap {
  namespace python {

void python_STGrid() {
  class_<STGrid>("stgrid")
    .def( init <> () )
    .def( init < const std::string > () )
    .def( init < const std::vector<std::string> > () )
    .def("_setif", &STGrid::setIF)
    .def("_setpollist", &STGrid::setPolList)
    .def("_setscanlist", &STGrid::setScanList)
    .def("_defineimage", &STGrid::defineImage,
         (boost::python::arg("nx")=-1,
          boost::python::arg("ny")=-1,
          boost::python::arg("scellx")="",
          boost::python::arg("scelly")="",
          boost::python::arg("scenter")=""))
    .def("_setfunc", &STGrid::setFunc,
         (boost::python::arg("convsupport")=-1,
          boost::python::arg("truncate")="",
          boost::python::arg("gwidth")="",
          boost::python::arg("jwidth")=""))
    .def("_grid", &STGrid::grid)
    .def("_setin", &STGrid::setFileIn)
    .def("_setfiles", &STGrid::setFileList)
    .def("_setweight", &STGrid::setWeight)
    .def("_enableclip", &STGrid::enableClip) 
    .def("_disableclip", &STGrid::disableClip) 
    .def("_save", &STGrid::saveData)
    .def("_getfunc", &STGrid::getConvFunc)
    .def("_get_resultant_map_size", &STGrid::getResultantMapSize)
    .def("_get_resultant_cell_size", &STGrid::getResultantCellSize)
    ;

  class_<STGrid2>("stgrid2")
    .def( init <> () )
    .def( init < const ScantableWrapper & > () )
    .def( init < const std::vector<ScantableWrapper> & > () )
    .def("_setif", &STGrid2::setIF)
    .def("_setpollist", &STGrid2::setPolList)
    .def("_setscanlist", &STGrid2::setScanList)
    .def("_defineimage", &STGrid2::defineImage,
         (boost::python::arg("nx")=-1,
          boost::python::arg("ny")=-1,
          boost::python::arg("scellx")="",
          boost::python::arg("scelly")="",
          boost::python::arg("scenter")=""))
    .def("_setfunc", &STGrid2::setFunc,
         (boost::python::arg("convsupport")=-1,
          boost::python::arg("truncate")="",
          boost::python::arg("gwidth")="",
          boost::python::arg("jwidth")=""))
    .def("_grid", &STGrid2::grid)
    .def("_setin", &STGrid2::setScantable)
    .def("_setfiles", &STGrid2::setScantableList)
    .def("_setweight", &STGrid2::setWeight)
    .def("_enableclip", &STGrid2::enableClip) 
    .def("_disableclip", &STGrid2::disableClip) 
    .def("_get", &STGrid2::getResultAsScantable)
    .def("_getfunc", &STGrid2::getConvFunc)
    ;
    
};

  } // python
} // asap
