//#---------------------------------------------------------------------------
//# python_asap.cc: python module for single dish package asap
//#---------------------------------------------------------------------------
//# Copyright (C) 2004-2012
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
//# $Id: python_asap.cpp 3035 2015-04-01 09:46:13Z KanaSugimoto $
//#---------------------------------------------------------------------------

#include <boost/python.hpp>
#include <boost/python/exception_translator.hpp>

#include <string>
#include <vector>

#include <casa/aips.h>
#include <casa/Exceptions/Error.h>
#include "ScantableWrapper.h"

#include <casacore/python/Converters/PycExcp.h>
#include <casacore/python/Converters/PycBasicData.h>
#include <casacore/python/Converters/PycValueHolder.h>
#include <casacore/python/Converters/PycRecord.h>


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
#ifdef ENABLE_PLOTTER2
  asap::python::python_Plotter2();
#endif //ENABLE_PLOTTER2
  asap::python::python_Scantable();
  asap::python::python_STFiller();
  asap::python::python_Filler();
  asap::python::python_MSFiller();
  asap::python::python_STSelector();
  asap::python::python_STMath();
  asap::python::python_Fitter();
  asap::python::python_STLineFinder();
  asap::python::python_STFitEntry();
  asap::python::python_STWriter();
  asap::python::python_MSWriter();
  asap::python::python_LineCatalog();
  asap::python::python_LogSink();
  asap::python::python_STCoordinate();
  asap::python::python_STAtmosphere();
  asap::python::python_SrcType();
  asap::python::python_STGrid();
  asap::python::python_Iterator();
  asap::python::python_EdgeMarker();
  asap::python::python_PlotHelper();
  asap::python::python_STSideBandSep();
  asap::python::python_CalibrationManager();

  casacore::python::register_convert_excp();
  casacore::python::register_convert_basicdata();
  casacore::python::register_convert_std_vector<asap::ScantableWrapper>();
  casacore::python::register_convert_std_vector<int>();
  casacore::python::register_convert_std_vector<uint>();
  casacore::python::register_convert_std_vector<float>();
  casacore::python::register_convert_std_vector<double>();
  casacore::python::register_convert_std_vector<std::string>();
  casacore::python::register_convert_std_vector<bool>();
  casacore::python::register_convert_casa_valueholder();
  casacore::python::register_convert_casa_record();

}
