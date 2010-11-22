//#---------------------------------------------------------------------------
//# python_Fitter.cc: python exposure of c++ Fitter class
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id: python_Fitter.cpp 1391 2007-07-30 01:59:36Z MalteMarquarding $
//#---------------------------------------------------------------------------
#include <boost/python.hpp>

#include "STFitter.h"

using namespace boost::python;

namespace asap {
  namespace python {

    void python_Fitter() {
      class_<Fitter>("fitter")
        .def( init <> () )
        .def("setexpression", &Fitter::setExpression)
        .def("setdata", &Fitter::setData)
        .def("getresidual", &Fitter::getResidual)
        .def("getfit", &Fitter::getFit)
        .def("getfixedparameters", &Fitter::getFixedParameters)
        .def("setfixedparameters", &Fitter::setFixedParameters)
        .def("getparameters", &Fitter::getParameters)
        .def("setparameters", &Fitter::setParameters)
        .def("getestimate", &Fitter::getEstimate)
        .def("estimate", &Fitter::computeEstimate)
        .def("geterrors", &Fitter::getErrors)
        .def("getchi2", &Fitter::getChisquared)
        .def("reset", &Fitter::reset)
        .def("fit", &Fitter::fit)
        .def("lfit", &Fitter::lfit)
        .def("evaluate", &Fitter::evaluate)
      ;
    };

  } //namespace python
} // namespace asap
