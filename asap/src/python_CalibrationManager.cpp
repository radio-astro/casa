//#---------------------------------------------------------------------------
//# python_CalibrationManager.cc: python exposure of c++ calibration classes
//#---------------------------------------------------------------------------
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include <string>
#include <vector>

#include "CalibrationManagerWrapper.h"
#include "ScantableWrapper.h"

using namespace boost::python;

namespace asap {
  namespace python {

void python_CalibrationManager() {
  class_<CalibrationManagerWrapper>("_calmanager")
    .def( init <> () )
    .def("set_data", &CalibrationManagerWrapper::setScantable)
    .def("set_data", &CalibrationManagerWrapper::setScantableByName)
    .def("add_applytable", &CalibrationManagerWrapper::addApplyTable)
    .def("add_skytable", &CalibrationManagerWrapper::addSkyTable)
    .def("add_tsystable", &CalibrationManagerWrapper::addTsysTable)
    .def("set_calmode", &CalibrationManagerWrapper::setMode)
    .def("set_time_interpolation", &CalibrationManagerWrapper::setTimeInterpolation,
         (boost::python::arg("interp"),
          boost::python::arg("order")=-1))
    .def("set_freq_interpolation", &CalibrationManagerWrapper::setFrequencyInterpolation,
         (boost::python::arg("interp"),
          boost::python::arg("order")=-1))
    .def("set_calibration_options", &CalibrationManagerWrapper::setCalibrationOptions)
    .def("reset", &CalibrationManagerWrapper::reset)
    .def("set_tsys_spw", &CalibrationManagerWrapper::setTsysSpw)
    .def("set_tsys_spw_withrange", &CalibrationManagerWrapper::setTsysSpwWithRange,
         (boost::python::arg("spwlist"),
	  boost::python::arg("average")=false))
    .def("set_tsys_transfer", &CalibrationManagerWrapper::setTsysTransfer)
    .def("calibrate", &CalibrationManagerWrapper::calibrate)
    .def("apply", &CalibrationManagerWrapper::apply,
         (boost::python::arg("insitu")=false,
          boost::python::arg("filltsys")=true))
    .def("save_caltable", &CalibrationManagerWrapper::saveCaltable)
    .def("split", &CalibrationManagerWrapper::split)
    ;
    
};

  } // python
} // asap
