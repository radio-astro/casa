//# MakeApproxPSFAlgorithm.h: parallelized version of approximate PSF formation
//# Copyright (C) 1996,1997,1998,1999,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef SYNTHESIS_MAKEAPPROXPSFALGORITHM_H
#define SYNTHESIS_MAKEAPPROXPSFALGORITHM_H

#include <casa/BasicSL/String.h>
#include <synthesis/Parallel/Algorithm.h>
#include <synthesis/Parallel/Applicator.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <synthesis/MeasurementComponents/GridFT.h>
#include <synthesis/MeasurementComponents/FTMachine.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Parallelized version of approximate PSF formation </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="FTMachine">FTMachine</linkto> module
//   <li> <linkto module="Algorithm">Algorithm</linkto> module
// </prerequisite>
//
// <etymology>
// Parallelized version of approximate PSF formation
// </etymology>
//
// <synopsis> 
// Parallelized version of approximate PSF formation. Inherits
// from Algorithm to fit within the AIPS++ parallelization 
// framework.
// </synopsis> 
//
// <example>
// <srcblock>

// </srcblock>
// </example>
//
// <motivation>
// Parallel PSF formation is one part of parallelized wide-field imaging.
// </motivation>
//
// <todo asof="02/02/05">
// <li> Full I/O profiling and temp image management.
// </todo>

class MakeApproxPSFAlgorithm : public Algorithm {
public:
  // Default constructor and destructor
  MakeApproxPSFAlgorithm();
  ~MakeApproxPSFAlgorithm();

  // Get the input data and parameters from the controller
  void get();

  // Return the results to the controller
  void put();

  // Return the name of the algorithm
  String &name();

 private:
  // Local copies of the data and input parameters
  TempImage<Complex> *cImage_p;
  String myName_p;
  String msFileName_p;

  TempImage<Float> *psf_p;
  Matrix<Float> weight_p;
  Vector<Float> beam_p; 
  Array<Float> *beamArray_p;
  FTMachine* ft_p;

  // Form and fit the PSF/beam
  void task();
};



} //# NAMESPACE CASA - END

#endif
