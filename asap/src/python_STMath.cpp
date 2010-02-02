//#---------------------------------------------------------------------------
//# python_STMathWrapper.cc: python exposure of c++ STMath class
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
//# $Id: python_STMath.cpp 1677 2010-01-27 07:29:48Z TakeshiNakazato $
//#---------------------------------------------------------------------------
#include <vector>
#include <boost/python.hpp>

#include "STMathWrapper.h"

using namespace boost::python;

namespace asap {
  namespace python {
    void python_STMath() {
      class_<STMathWrapper>("stmath")
        .def( init < > () )
        .def( init < bool > () )
        .def("_insitu", &STMathWrapper::insitu)
        .def("_setinsitu", &STMathWrapper::setInsitu)
        .def("_average", &STMathWrapper::average)
        .def("_averagechannel", &STMathWrapper::averageChannel)
        .def("_averagepol", &STMathWrapper::averagePolarisations)
        .def("_averagebeams", &STMathWrapper::averageBeams)
        .def("_unaryop", &STMathWrapper::unaryOperate)
        .def("_arrayop", &STMathWrapper::arrayOperate)
        //.def("_array2dop", &STMathWrapper::array2dOperate)
        .def("_binaryop", &STMathWrapper::binaryOperate)
        .def("_auto_quotient", &STMathWrapper::autoQuotient)
        .def("_quotient", &STMathWrapper::quotient)
        .def("_dototalpower", &STMathWrapper::dototalpower)
        .def("_dosigref", &STMathWrapper::dosigref)
        .def("_donod", &STMathWrapper::donod)
        .def("_dofs", &STMathWrapper::dofs)
        .def("_stats", &STMathWrapper::statistic)
        .def("_minmaxchan", &STMathWrapper::minMaxChan)
        .def("_freqswitch", &STMathWrapper::freqSwitch)
        .def("_bin", &STMathWrapper::bin)
        .def("_resample", &STMathWrapper::resample)
        .def("_smooth", &STMathWrapper::smooth)
        .def("_gainel", &STMathWrapper::gainElevation)
        .def("_convertflux", &STMathWrapper::convertFlux)
        .def("_convertpol", &STMathWrapper::convertPolarisation)
        .def("_opacity", &STMathWrapper::opacity)
        .def("_merge", &STMathWrapper::merge)
        .def("_rotate_xyphase", &STMathWrapper::rotateXYPhase)
        .def("_rotate_linpolphase", &STMathWrapper::rotateLinPolPhase)
        .def("_invert_phase", &STMathWrapper::invertPhase)
        .def("_swap_linears", &STMathWrapper::swapPolarisations)
        .def("_freq_align", &STMathWrapper::frequencyAlign)
        .def("_mx_extract", &STMathWrapper::mxExtract)
        .def("_lag_flag", &STMathWrapper::lagFlag)
	// testing average spectra with different channel/resolution
        .def("_new_average", &STMathWrapper::new_average)
        // cwcal
        .def("cwcal", &STMathWrapper::cwcal)
        .def("almacal", &STMathWrapper::almacal)
          ;
    };

  } //namespace python
} // namespace asap

