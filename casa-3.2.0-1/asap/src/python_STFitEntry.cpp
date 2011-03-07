//#---------------------------------------------------------------------------
//# python_STFitEntry: python exposure of c++ STFitEntry class
//#---------------------------------------------------------------------------
//# Copyright (C) 2006
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
//# $Id: python_STFitEntry.cpp 1933 2010-09-17 08:55:41Z WataruKawasaki $
//#---------------------------------------------------------------------------
#include <boost/python.hpp>

#include "STFitEntry.h"

using namespace boost::python;

namespace asap {
  namespace python {

    void python_STFitEntry() {
      class_<STFitEntry>("fitentry")
        .def( init <> () )
        .def( init < const STFitEntry& > () )
        .def("getfixedparameters", &STFitEntry::getParmasks)
        .def("getparameters", &STFitEntry::getParameters)
        .def("geterrors", &STFitEntry::getErrors)
        .def("getfunctions", &STFitEntry::getFunctions)
	.def("getcomponents", &STFitEntry::getComponents)
	.def("getframeinfo", &STFitEntry::getFrameinfo)
        .def("setfixedparameters", &STFitEntry::setParmasks)
        .def("setparameters", &STFitEntry::setParameters)
        .def("seterrors", &STFitEntry::setErrors)
        .def("setfunctions", &STFitEntry::setFunctions)
	.def("setcomponents", &STFitEntry::setComponents)
	.def("setframeinfo", &STFitEntry::setFrameinfo)
      ;
    };

  } //namespace python
} // namespace asap
