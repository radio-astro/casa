//# ResidualAlgorithm.h: Parallel residual image formation
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

#ifndef SYNTHESIS_RESIDUALALGORITHM_H
#define SYNTHESIS_RESIDUALALGORITHM_H

#include <casa/BasicSL/String.h>
#include <synthesis/Parallel/Algorithm.h>
#include <synthesis/Parallel/Applicator.h>
#include <msvis/MSVis/VisSet.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <synthesis/TransformMachines/GridFT.h>
#include <synthesis/TransformMachines/FTMachine.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary> Parallel residual image formation </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto module="FTMachine">FTMachine</linkto> module
//     <li> <linkto module="Applicator">Algorithm</linkto> module
// </prerequisite>
//
// <etymology>
// From "residual image" and "parallel algorithm"
// </etymology>
//
// <synopsis> 
// This class is called by PSkyEquation to form residual image
// using parallelization within the Applicator/Algorithm framework
// </synopsis> 
//
// <example>
// <srcblock>

// </srcblock>
// </example>
//
// <motivation>
// Encapsulate the parallelized residual image formation 
// </motivation>
//
// <todo asof="01/7/20">
// <li> Does not support time-variable SkyJones
// </todo>


class ResidualAlgorithm : public Algorithm {
 public:
  // Default constructor and destructor
  ResidualAlgorithm();
  ~ResidualAlgorithm();

  // Get the input data and parameters from the controller
  void get();

  // Return the results to the controller
  void put();

  // Return the name of the algorithm
  String &name();

 private:
  // Private data
  Int model_p;
  TempImage<Complex> *cImage_p;
  String myName_p;
  String msName_p;
  Bool incremental_p;
  Matrix<Float> *weight_p;
  TempImage<Float> psf_p;
  FTMachine* ft_p;
  MeasurementSet *ms_p ; 

  // Form the residual image
  void task();

  // Local utility functions
  void initializePut(const VisBuffer& vb, Int model);
  void putResidualVis(VisBuffer& vb, Int model);

};


} //# NAMESPACE CASA - END

#endif
