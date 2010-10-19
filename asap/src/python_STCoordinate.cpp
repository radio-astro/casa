//
// C++ Implementation: python_STCoordinate
//
// Description: This file is the boost::python wrapper for asap::STCoordinate
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <boost/python.hpp>

#include "STCoordinate.h"

using namespace boost::python;

namespace asap {
  namespace python {
    void python_STCoordinate() {
      class_<STCoordinate>("coordinate")
        .def( init < const STCoordinate& > () )
        .def("get_reference_pixel", &STCoordinate::getReferencePixel)
        .def("get_reference_value", &STCoordinate::getReferenceValue)
        .def("get_increment", &STCoordinate::getIncrement)
        .def("to_frequency", &STCoordinate::toFrequency)
        .def("to_velocity", &STCoordinate::toVelocity)
        .def("to_pixel", &STCoordinate::toPixel)
      ;
    };
  }
}
