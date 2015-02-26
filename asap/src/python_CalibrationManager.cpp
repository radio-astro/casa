//#---------------------------------------------------------------------------
//# python_CalibrationManager.cc: python exposure of c++ calibration classes
//#---------------------------------------------------------------------------
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include <string>
#include <vector>

#include "CalibrationManager.h"
#include "ScantableWrapper.h"

using namespace boost::python;

namespace asap {
  namespace python {

void python_CalibrationManager() {
  class_<CalibrationManager>("_calmanager")
    .def( init <> () )
    .def("set_data", &CalibrationManager::setScantable)
    .def("set_data", &CalibrationManager::setScantableByName)
    .def("add_applytable", &CalibrationManager::addApplyTable)
    .def("add_skytable", &CalibrationManager::addSkyTable)
    .def("add_tsystable", &CalibrationManager::addTsysTable)
    .def("set_calmode", &CalibrationManager::setMode)
    .def("set_time_interpolation", &CalibrationManager::setTimeInterpolation,
         (boost::python::arg("interp"),
          boost::python::arg("order")=-1))
    .def("set_freq_interpolation", &CalibrationManager::setFrequencyInterpolation,
         (boost::python::arg("interp"),
          boost::python::arg("order")=-1))
    .def("set_calibration_options", &CalibrationManager::setCalibrationOptions)
    .def("reset", &CalibrationManager::reset)
    .def("set_tsys_spw", &CalibrationManager::setTsysSpw)
    .def("set_tsys_spw_withrange", &CalibrationManager::setTsysSpwWithRange,
         (boost::python::arg("spwlist"),
	  boost::python::arg("average")=false))
    .def("set_tsys_transfer", &CalibrationManager::setTsysTransfer)
    .def("calibrate", &CalibrationManager::calibrate)
    .def("apply", &CalibrationManager::apply,
         (boost::python::arg("insitu")=false,
          boost::python::arg("filltsys")=true))
    .def("save_caltable", &CalibrationManager::saveCaltable)
    .def("split", &CalibrationManager::split)
    ;
    
};

  } // python
} // asap
