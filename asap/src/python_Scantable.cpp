//#---------------------------------------------------------------------------
//# python_Scantable.cc: python exposure of c++ Scantable class
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
//# $Id: python_Scantable.cpp 2888 2013-12-26 02:21:30Z KanaSugimoto $
//#---------------------------------------------------------------------------

#include <boost/python.hpp>
#include <boost/python/args.hpp>
#include <vector>

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
    .def("getcyclenos", &ScantableWrapper::getCycleNos)
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
         (boost::python::arg("whichrow")=0, 
	  boost::python::arg("poltype")=std::string("")) )
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
    .def("_getclipmask", &ScantableWrapper::getClipMask,
	 (boost::python::arg("whichrow")=0) )
    .def("_gettsys", &ScantableWrapper::getTsys)
    .def("_gettsysspectrum", &ScantableWrapper::getTsysSpectrum )
    .def("_settsys", &ScantableWrapper::setTsys)
    .def("_getsourcename", &ScantableWrapper::getSourceName,
         (boost::python::arg("whichrow")=0) )
    .def("_getelevation", &ScantableWrapper::getElevation,
         (boost::python::arg("whichrow")=0) )
    .def("_getazimuth", &ScantableWrapper::getAzimuth,
         (boost::python::arg("whichrow")=0) )
    .def("_getparangle", &ScantableWrapper::getParAngle,
         (boost::python::arg("whichrow")=0) )
    .def("_gettime", &ScantableWrapper::getTime,
         (boost::python::arg("whichrow")=0,
          boost::python::arg("prec")=0) )
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
	 (boost::python::arg("filename")=""))
    .def("_list_header",  &ScantableWrapper::listHeader)
    .def("_getrestfreqs",  &ScantableWrapper::getRestFrequency)
    .def("_setrestfreqs",  &ScantableWrapper::setRestFrequencies)
    .def("shift_refpix", &ScantableWrapper::shift)
    .def("_setcoordinfo", &ScantableWrapper::setCoordInfo)
    .def("_getcoordinfo", &ScantableWrapper::getCoordInfo)
    .def("set_dirframe", &ScantableWrapper::setDirection,
         (boost::python::arg("refstr")="") )
    .def("_gethistory", &ScantableWrapper::getHistory)
    .def("_addhistory", &ScantableWrapper::addHistory)
    .def("drop_history", &ScantableWrapper::dropHistory)
    .def("_historylength", &ScantableWrapper::historyLength)
    .def("_getselection", &ScantableWrapper::getSelection)
    .def("_setselection", &ScantableWrapper::setSelection)
    .def("_addfit", &ScantableWrapper::addFit)
    .def("_getfit", &ScantableWrapper::getFit)
    .def("_recalcazel", &ScantableWrapper::calculateAZEL)
    .def("_setsourcetype", &ScantableWrapper::setSourceType)
    .def("_setsourcename", &ScantableWrapper::setSourceName)
    .def("_getdirectionvec", &ScantableWrapper::getDirectionVector)
    .def("_parallactify", &ScantableWrapper::parallactify)
    .def("get_coordinate", &ScantableWrapper::getCoordinate)
    .def("_get_weather", &ScantableWrapper::getWeather)
    .def("_reshape", &ScantableWrapper::reshapeSpectrum, 
	 (boost::python::arg("nmin")=-1, 
	  boost::python::arg("nmax")=-1) )
    .def("_regrid_specchan", &ScantableWrapper::regridSpecChannel, 
	 (boost::python::arg("nchan")=-1) )
    .def("_apply_bltable", &ScantableWrapper::applyBaselineTable)
    .def("_sub_baseline", &ScantableWrapper::subBaseline)
    .def("_poly_baseline", &ScantableWrapper::polyBaseline)
    .def("_auto_poly_baseline", &ScantableWrapper::autoPolyBaseline)
    .def("_chebyshev_baseline", &ScantableWrapper::chebyshevBaseline)
    .def("_auto_chebyshev_baseline", &ScantableWrapper::autoChebyshevBaseline)
    .def("_cspline_baseline", &ScantableWrapper::cubicSplineBaseline)
    .def("_auto_cspline_baseline", &ScantableWrapper::autoCubicSplineBaseline)
    .def("_sinusoid_baseline", &ScantableWrapper::sinusoidBaseline)
    .def("_auto_sinusoid_baseline", &ScantableWrapper::autoSinusoidBaseline)
    .def("get_rms", &ScantableWrapper::getRms)
    .def("format_blparams_row", &ScantableWrapper::formatBaselineParams)
    .def("format_piecewise_blparams_row", &ScantableWrapper::formatPiecewiseBaselineParams)
    .def("_is_all_chan_flagged", &ScantableWrapper::isAllChannelsFlagged,
	 (boost::python::arg("whichrow")=0) )
    .def("_fft", &ScantableWrapper::execFFT)
    //.def("_sspline_baseline", &ScantableWrapper::smoothingSplineBaseline)
    //.def("_test_cin", &ScantableWrapper::testCin)
    .def("_getmolidcol_list", &ScantableWrapper::getMoleculeIdColumnData)
    .def("_setmolidcol_list", &ScantableWrapper::setMoleculeIdColumnData)
    .def("_get_root_row_idx", &ScantableWrapper::getRootTableRowNumbers)
    .def("_calc_aic", &ScantableWrapper::calculateModelSelectionCriteria)
    .def("drop_xpol", &ScantableWrapper::dropXPol)
  ;
};

  } // python
} // asap
