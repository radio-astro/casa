//#---------------------------------------------------------------------------
//# SDMathWrapper.h: Wrapper classes to use CountedPtr
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
//# $Id:
//#---------------------------------------------------------------------------
#ifndef SDMATHWRAPPER_H
#define SDMATHWRAPPER_H

#include <vector>
#include <string>

//#include <boost/python/tuple.hpp>

#include <casa/Utilities/CountedPtr.h>

#include <singledish/SDMemTableWrapper.h>
#include <singledish/SDMath.h>

namespace asap {

namespace SDMathWrapper {


  static SDMath sdmath;
  // Quotient
  SDMemTableWrapper quotient(const SDMemTableWrapper& on,
                             const SDMemTableWrapper& off,
                             casa::Bool preserveContinuum);

  // Simple binary operations
  SDMemTableWrapper binaryOperate(const SDMemTableWrapper& left,
				  const SDMemTableWrapper& right,
				  const std::string& op, bool doTSys);
  
  // Multiply
  void scaleInSitu(SDMemTableWrapper& in, float factor, bool doAll, 
		   bool doTSys);
  SDMemTableWrapper scale(const SDMemTableWrapper& in, float factor, 
			  bool all, bool doTSys);
  
  // Add
  void addInSitu(SDMemTableWrapper& in, float offset, bool doAall);
  SDMemTableWrapper add(const SDMemTableWrapper& in, float offset, bool all);

// Smooth
  void smoothInSitu(SDMemTableWrapper& in, const std::string& kernel,
		     float width, bool doAll);
  SDMemTableWrapper smooth(const SDMemTableWrapper& in,
			   const std::string& kernel, float width, bool doAll);

// Bin up
  void binInSitu(SDMemTableWrapper& in, int width);
  SDMemTableWrapper bin(const SDMemTableWrapper& in, int width);

// Resample 
  void resampleInSitu(SDMemTableWrapper& in, const std::string& method, 
		      float width);
  SDMemTableWrapper resample(const SDMemTableWrapper& in, 
			     const std::string& method, 
                             float width);

// Convert brightness between Jy and K
  void convertFluxInSitu(SDMemTableWrapper& in, float D, float eta, 
			 float JyPerK, bool doAll);
  SDMemTableWrapper convertFlux(const SDMemTableWrapper& in, float D, 
				float eta, float JyPerK, bool doAll);

// Apply gain elevation correction
  void gainElevationInSitu (SDMemTableWrapper& in, const std::vector<float>& coeffs,
                            const std::string& fileName, 
                            const std::string& method, bool doAll);
  SDMemTableWrapper gainElevation(const SDMemTableWrapper& in, const std::vector<float>& coeffs,
                                  const std::string& fileName, 
                                  const std::string& method, bool doAll);

// Apply frequency alignment
  void frequencyAlignmentInSitu (SDMemTableWrapper& in, const std::string& refTime,
                                 const std::string& method, bool perFreqID);
  SDMemTableWrapper frequencyAlignment(const SDMemTableWrapper& in, const std::string& refTime,
                                       const std::string& method, bool perFreqID);

// XY Phase rotation
  void rotateXYPhaseInSitu(SDMemTableWrapper& in, float angle, bool doAll);

// LinPol Phase rotation
  void rotateLinPolPhaseInSitu(SDMemTableWrapper& in, float angle, bool doAll);

// Apply opacity correction
  void opacityInSitu(SDMemTableWrapper& in, float tau, bool doAll);
  SDMemTableWrapper opacity(const SDMemTableWrapper& in, float tau, 
			    bool doAll);

// Average in time
//  SDMemTableWrapper average(boost::python::tuple tpl,
  SDMemTableWrapper average(const std::vector<casa::CountedPtr<SDMemTable> >& tpl,
			    const std::vector<bool>& mask,
			    bool scanAv, const std::string& wt);

// Average polarizations
  void averagePolInSitu(SDMemTableWrapper& in,  const std::vector<bool>& mask,
                        const std::string& weightStr);
  SDMemTableWrapper averagePol(const SDMemTableWrapper& in,
			       const std::vector<bool>& mask,
                               const std::string& weightStr);

  // Frequency Switching
  void SDMathWrapper::frequencySwitchInSitu(SDMemTableWrapper& in);

  SDMemTableWrapper 
  SDMathWrapper::frequencySwitch(const SDMemTableWrapper& in);

  // Statistics
  std::vector<float> statistic(const SDMemTableWrapper& in,
                               const std::vector<bool>& mask, 
                               const std::string& which, int row);

};

} // namespace
#endif
