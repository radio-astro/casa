//
// C++ Implementation: python_LineCatalog
//
// Description: This file is the boost::python wrapper for asap::LineCatalog
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <boost/python.hpp>
#include <string>

#include "LineCatalog.h"

using namespace boost::python;

namespace asap {
  namespace python {

    void python_LineCatalog() {
      class_<LineCatalog>("linecatalog")
        .def( init < const std::string& > () )
        .def("summary", &LineCatalog::summary)
        .def("get_name", &LineCatalog::getName)
        .def("get_frequency", &LineCatalog::getFrequency)
        .def("set_frequency_limits", &LineCatalog::setFrequencyLimits)
        .def("set_strength_limits", &LineCatalog::setStrengthLimits)
        .def("set_name", &LineCatalog::setPattern)
        .def("save", &LineCatalog::save)
        .def("reset", &LineCatalog::reset)
        .def("nrow", &LineCatalog::nrow)
     ;
    };
  }
}
