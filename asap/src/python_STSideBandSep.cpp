//#---------------------------------------------------------------------------
//# python_STSideBandSep.cpp: python exposure of c++ STSideBandSep class
//#---------------------------------------------------------------------------
//# Author: Kanako Sugimoto, (C) 2012
//#
//# Copyright: See COPYING file that comes with this distribution
//#
//#---------------------------------------------------------------------------
#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include "STSideBandSep.h"

using namespace boost::python;

namespace asap {
  namespace python {

void python_STSideBandSep() {
  class_<STSideBandSep>("SBSeparator")
    .def( init <> () )
    .def( init < const std::vector<std::string> > () )
    .def( init < const std::vector<ScantableWrapper> > () )
    .def( "set_freq", &STSideBandSep::setFrequency,
	  (boost::python::arg("frame")="") )
    .def( "set_dirtol", &STSideBandSep::setDirTolerance )
    .def( "set_shift", &STSideBandSep::setShift )
    .def( "set_limit", &STSideBandSep::setThreshold )
    .def( "solve_both", &STSideBandSep::solveBoth )
    .def( "subtract_other", &STSideBandSep::solvefromOther )
    .def( "set_lo1", &STSideBandSep::setLO1,
	  (boost::python::arg("frame")="TOPO",
	   boost::python::arg("reftime")=-1,
	   boost::python::arg("refdir")="") )
    .def( "set_lo1root", &STSideBandSep::setLO1Root )
    .def( "separate", &STSideBandSep::separate )
    //// temporal methods
    //.def( "solve_imgfreq", &STSideBandSep::solveImageFreqency )
    //.def( "_get_asistb_from_scantb", &STSideBandSep::setScanTb0 )
  ;
};

  } // python
} // asap
