//#---------------------------------------------------------------------------
//# python_Scantable.cc: python exposure of c++ Scantable class
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
//# $Id: python_Scantable.cpp 1700 2010-02-16 05:21:26Z WataruKawasaki $
//#---------------------------------------------------------------------------
#include <vector>

#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "ScantableWrapper.h"

using namespace boost::python;

namespace asap {
  namespace python {

void python_Scantable() {
  class_<ScantableWrapper>("Scantable")
    //.def( init <> () )
    .def( init < int > () )
    .def( init < const std::string&, int > () )
    .def( init < const ScantableWrapper& > () )
    .def("_copy", &ScantableWrapper::copy)
    .def("_assign", &ScantableWrapper::assign)
    .def("getif", &ScantableWrapper::getIF)
    .def("getifnos", &ScantableWrapper::getIFNos)
    .def("getbeam", &ScantableWrapper::getBeam)
    .def("getbeamnos", &ScantableWrapper::getBeamNos)
    .def("getpol", &ScantableWrapper::getPol)
    .def("getpolnos", &ScantableWrapper::getPolNos)
    .def("getscan", &ScantableWrapper::getScan)
    .def("getscannos", &ScantableWrapper::getScanNos)
    .def("getcycle", &ScantableWrapper::getCycle)
    .def("getmolnos", &ScantableWrapper::getMolNos)
    .def("nif", &ScantableWrapper::nif,
         (boost::python::arg("scanno")=-1) )
    .def("nbeam", &ScantableWrapper::nbeam,
         (boost::python::arg("scanno")=-1) )
    .def("npol", &ScantableWrapper::npol,
         (boost::python::arg("scanno")=-1) )
    .def("nchan", &ScantableWrapper::nchan,
         (boost::python::arg("ifno")=-1) )
    .def("ncycle", &ScantableWrapper::ncycle,
         (boost::python::arg("scanno")=-1) )
    .def("nscan", &ScantableWrapper::nscan)
    .def("nrow", &ScantableWrapper::nrow)
    .def("get_fluxunit", &ScantableWrapper::getFluxUnit)
    .def("set_fluxunit", &ScantableWrapper::setFluxUnit)
    .def("_setInstrument", &ScantableWrapper::setInstrument)
    .def("_setfeedtype", &ScantableWrapper::setFeedType)
    .def("_getspectrum", &ScantableWrapper::getSpectrum,
         (arg("whichrow")=0, arg("poltype")=std::string("")) )
    .def("poltype", &ScantableWrapper::getPolType )
    .def("get_column_names", &ScantableWrapper::columnNames)
    .def("_getpollabel", &ScantableWrapper::getPolarizationLabel)
    .def("_setspectrum",&ScantableWrapper::setSpectrum,
         (boost::python::arg("whichrow")=0) )
    .def("_getabcissa", &ScantableWrapper::getAbcissa,
         (boost::python::arg("whichrow")=0) )
    .def("_getabcissalabel", &ScantableWrapper::getAbcissaLabel,
         (boost::python::arg("whichrow")=0) )
    .def("_getmask", &ScantableWrapper::getMask,
         (boost::python::arg("whichrow")=0) )
    .def("_gettsys", &ScantableWrapper::getTsys)
    .def("_getsourcename", &ScantableWrapper::getSourceName,
         (boost::python::arg("whichrow")=0) )
    .def("_getelevation", &ScantableWrapper::getElevation,
         (boost::python::arg("whichrow")=0) )
    .def("_getazimuth", &ScantableWrapper::getAzimuth,
         (boost::python::arg("whichrow")=0) )
    .def("_getparangle", &ScantableWrapper::getParAngle,
         (boost::python::arg("whichrow")=0) )
    .def("_gettime", &ScantableWrapper::getTime,
         (boost::python::arg("whichrow")=0) )
    .def("_getinttime", &ScantableWrapper::getIntTime,
         (boost::python::arg("whichrow")=0) )
    .def("_getdirection", &ScantableWrapper::getDirectionString,
         (boost::python::arg("whichrow")=0) )
    .def("get_antennaname", &ScantableWrapper::getAntennaName)
    .def("_flag", &ScantableWrapper::flag)
    .def("_flag_row", &ScantableWrapper::flagRow)
    .def("_getflagrow", &ScantableWrapper::getFlagRow,
	 (boost::python::arg("whichrow")=0) )
    .def("_clip", &ScantableWrapper::clip,
	 (boost::python::arg("clipoutside")=true,
	  boost::python::arg("unflag")=false) )
    .def("_save",  &ScantableWrapper::makePersistent)
    .def("_summary",  &ScantableWrapper::summary,
         (boost::python::arg("verbose")=true) )
    //.def("_getrestfreqs",  &ScantableWrapper::getRestFrequencies)
    .def("_getrestfreqs",  &ScantableWrapper::getRestFrequency)
    .def("_setrestfreqs",  &ScantableWrapper::setRestFrequencies)
    .def("shift_refpix", &ScantableWrapper::shift)
    .def("_setcoordinfo", &ScantableWrapper::setCoordInfo)
    .def("_getcoordinfo", &ScantableWrapper::getCoordInfo)
    .def("set_dirframe", &ScantableWrapper::setDirection,
         (boost::python::arg("refstr")="") )
    .def("_gethistory", &ScantableWrapper::getHistory)
    .def("_addhistory", &ScantableWrapper::addHistory)
    .def("_getselection", &ScantableWrapper::getSelection)
    .def("_setselection", &ScantableWrapper::setSelection)
    .def("_addfit", &ScantableWrapper::addFit)
    .def("_getfit", &ScantableWrapper::getFit)
    .def("_recalcazel", &ScantableWrapper::calculateAZEL)
    .def("_setsourcetype", &ScantableWrapper::setSourceType)
    .def("_getdirectionvec", &ScantableWrapper::getDirectionVector)
    .def("_reshape", &ScantableWrapper::reshapeSpectrum, 
	 (boost::python::arg("nmin")=-1, 
	  boost::python::arg("nmax")=-1) )
  ;
};

  } // python
} // asap
