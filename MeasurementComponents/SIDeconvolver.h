//# SIDeconvolver.h: Definition for SIDeconvolver
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_SIDECONVOLVER_H
#define SYNTHESIS_SIDECONVOLVER_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MeasurementComponents/SkyModel.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/System/PGPlotter.h>

#include<synthesis/MeasurementEquations/SIMaskHandler.h>

namespace casa { //# NAMESPACE CASA - BEGIN

  /* Forware Declaration */
  class SISubIterBot;


class SIDeconvolver {
public:

  // Empty constructor
  SIDeconvolver();
  ~SIDeconvolver();


  // Decide how to iterate through the image axes.
  // The Mapper will call 'deconvolve' with pieces of only this shape.
  void queryDesiredShape(Bool &onechan, Bool &onepol); // , nImageFacets.

  // 
  Bool deconvolve( SISubIterBot& loopcontrols,  
		   ImageInterface<Float> &residual, 
		   ImageInterface<Float> &psf, 
		   ImageInterface<Float> &model, 
		   CountedPtr<SIMaskHandler> maskhandler, 
		   Int mapperid,
		   uInt decid);

  // eventually, send in images by reference.
  void findNextComponent( ImageInterface<Float>  &residual, 
			  ImageInterface<Float>  &psf, 
			  Float loopgain, 
			  Float &comp );
  void updateModel( ImageInterface<Float> &model, 
		    Float  &comp );
  void updateResidual( ImageInterface<Float> &residual, 
		       Float  &comp );

  void restore( ImageInterface<Float>  &image, 
		Float beam, 
		ImageInterface<Float>  &model, 
		ImageInterface<Float>  &residual, 
		ImageInterface<Float>  &weight );

  Bool checkStop( SISubIterBot &loopcontrols, Int currentiteration, Float currentresidual );

protected:

  // For debugging.
  IPosition tmpPos_p;

};

} //# NAMESPACE CASA - END

#endif
