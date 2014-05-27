//#---------------------------------------------------------------------------
//# python_Plotter2.cc: python exposure of C++ Plotter2 class
//#---------------------------------------------------------------------------
//# Copyright (C) 2012
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
//# $Id: python_Plotter2.cpp 2012 2012-03-12 05:51:50Z WataruKawasaki $
//#---------------------------------------------------------------------------
#include <boost/python.hpp>

#include "Plotter2.h"

using namespace boost::python;

namespace asap {
  namespace python {
     void python_Plotter2() {
       class_<Plotter2>("Plotter2")
         .def( init <> () )
         .def("get_filename",&Plotter2::getFileName)
         .def("set_filename",&Plotter2::setFileName)
         .def("get_device",&Plotter2::getDevice)
         .def("set_device",&Plotter2::setDevice)
         .def("get_viewsurface_width",&Plotter2::getViewSurfaceWidth)
         .def("get_viewsurface_aspect",&Plotter2::getViewSurfaceAspect)
         .def("set_viewsurface",&Plotter2::setViewSurface)
         .def("add_viewport",&Plotter2::addViewport)
         .def("set_viewport",&Plotter2::setViewport)
         .def("show_viewport",&Plotter2::showViewport)
         .def("hide_viewport",&Plotter2::hideViewport)
         .def("get_hasdefaultvp",&Plotter2::getHasDefaultViewport)
         .def("get_currentvpid",&Plotter2::getCurrentViewportId)
         .def("get_vinfo",&Plotter2::getViewInfo)

         .def("set_range",&Plotter2::setRange)
         .def("set_range_x",&Plotter2::setRangeX)
         .def("set_range_y",&Plotter2::setRangeY)
         .def("get_range_x",&Plotter2::getRangeX)
         .def("get_range_y",&Plotter2::getRangeY)
         .def("set_autorange",&Plotter2::setAutoRange)
         .def("set_autorange_x",&Plotter2::setAutoRangeX)
         .def("set_autorange_y",&Plotter2::setAutoRangeY)
         .def("set_fontsize",&Plotter2::setFontSizeDef)
         .def("set_tics_x",&Plotter2::setTicksX)
         .def("set_tics_y",&Plotter2::setTicksY)
         .def("set_autotics",&Plotter2::setAutoTicks)
         .def("set_autotics_x",&Plotter2::setAutoTicksX)
         .def("set_autotics_y",&Plotter2::setAutoTicksY)
         .def("set_ninterval_x",&Plotter2::setNumIntervalX)
         .def("set_ninterval_y",&Plotter2::setNumIntervalY)
         .def("set_nlocation_x",&Plotter2::setNumLocationX)
         .def("set_nlocation_y",&Plotter2::setNumLocationY)
         .def("set_data",&Plotter2::setData)
         .def("set_line",&Plotter2::setLine)
         .def("show_line",&Plotter2::showLine)
         .def("hide_line",&Plotter2::hideLine)
         .def("set_point",&Plotter2::setPoint)
         .def("show_point",&Plotter2::showPoint)
         .def("hide_point",&Plotter2::hidePoint)
         .def("set_mask_x",&Plotter2::setMaskX)
         .def("set_arrow",&Plotter2::setArrow)
         .def("set_text",&Plotter2::setText)
         .def("set_label_x",&Plotter2::setLabelX)
         .def("set_label_y",&Plotter2::setLabelY)
         .def("set_title",&Plotter2::setTitle)
         .def("set_vpbgcolor",&Plotter2::setViewportBackgroundColor)

         .def("plot",&Plotter2::plot)
       ;
     };
  } // namespace python
} // namespace asap
