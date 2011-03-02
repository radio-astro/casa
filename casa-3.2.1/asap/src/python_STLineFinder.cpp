//#---------------------------------------------------------------------------
//# python_STLineFinder.cc: python exposure of C++ STLineFinder class
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
//# $Id: python_STLineFinder.cpp 894 2006-03-08 03:03:25Z mar637 $
//#---------------------------------------------------------------------------
#include <boost/python.hpp>

#include "STLineFinder.h"

using namespace boost::python;

namespace asap {
  namespace python {
     void python_STLineFinder() {
       class_<STLineFinder>("linefinder")
         .def( init <> () )
	 .def("setoptions",&STLineFinder::setOptions)
         .def("setscan",&STLineFinder::setScan)
         .def("findlines",&STLineFinder::findLines)
         .def("getmask",&STLineFinder::getMask)
         .def("getlineranges",&STLineFinder::getLineRanges)
         .def("getlinerangesinchannels",&STLineFinder::getLineRangesInChannels)
       ;
     };
  } // namespace python
} // namespace asap
