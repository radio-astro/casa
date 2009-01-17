//#---------------------------------------------------------------------------
//# python_asap.cc: python module for single dish package asap
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
//# $Id: python_asap.cpp 1126 2006-08-10 04:16:19Z mar637 $
//#---------------------------------------------------------------------------
#include <string>
#include <vector>

#include <boost/python.hpp>
#include <boost/python/exception_translator.hpp>

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include "ScantableWrapper.h"


#include "pyconversions.h"
#include "python_asap.h"

namespace asap {
  namespace python {
void translate_ex(const casa::AipsError& e)
{
  // Use the Python 'C' API to set up an exception object
  PyErr_SetString(PyExc_RuntimeError, e.what());
}

  }
}
using namespace boost::python;

BOOST_PYTHON_MODULE(_asap) {
  asap::python::python_Scantable();
  asap::python::python_STFiller();
  asap::python::python_STSelector();
  asap::python::python_STMath();
  asap::python::python_Fitter();
  asap::python::python_STLineFinder();
  asap::python::python_STFitEntry();
  asap::python::python_STWriter();
  asap::python::python_LineCatalog();

  asap::python::python_Logger();
  register_exception_translator<casa::AipsError>(&asap::python::translate_ex);

  //std_vector_to_tuple <  > ();
  from_python_sequence < std::vector< asap::ScantableWrapper >,
    variable_capacity_policy > ();

  std_vector_to_tuple < int > ();
  from_python_sequence < std::vector < int >,
    variable_capacity_policy > ();
  std_vector_to_tuple < uint > ();
  from_python_sequence < std::vector < uint >,
    variable_capacity_policy > ();
  std_vector_to_tuple < float > ();
  from_python_sequence < std::vector < float >,
    variable_capacity_policy > ();
  std_vector_to_tuple < double > ();
  from_python_sequence < std::vector < double >,
    variable_capacity_policy > ();
  std_vector_to_tuple < std::string > ();
  from_python_sequence < std::vector < std::string >,
    variable_capacity_policy > ();
  std_vector_to_tuple < bool> ();
  from_python_sequence < std::vector < bool >,
    variable_capacity_policy > ();
}
