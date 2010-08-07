//#---------------------------------------------------------------------------
//# python_Logger.cc: python exposure of c++ LogSink
//#---------------------------------------------------------------------------
//# Copyright (C) 2010
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
//# $Id:
//#---------------------------------------------------------------------------
#include <boost/python.hpp>

#include "AsapLogSink.h"

using namespace boost::python;

namespace asap {
  namespace python {
    void python_LogSink() {
      class_<AsapLogSink>("LogSink")
        .def( init <> () )
	.def("post", &AsapLogSink::postMessage,
         (boost::python::arg("location")="",
         boost::python::arg("priority")="INFO"))
	.def("pop", &AsapLogSink::popMessages)
      ;
      def("set_global_sink", &setAsapSink);
    };

  } //namespace python
} // namespace asap
