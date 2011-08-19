// -*- C++ -*-
//# AzElAperture.h: Definition of the AzElAperture class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
//
#ifndef SYNTHESIS_AZELAPERTURE_H
#define SYNTHESIS_AZELAPERTURE_H

#include <images/Images/ImageInterface.h>
#include <synthesis/MeasurementComponents/ATerm.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
//
//---------------------------------------------------------------------
//---------------------------------------------------------------------
// TEMPS The following #defines should REALLLLAY GO!
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//
#define CONVSIZE (1024*2)
#define CONVWTSIZEFACTOR sqrt(2.0)
#define OVERSAMPLING 10
#define THRESHOLD 1E-3

namespace casa { //# NAMESPACE CASA - BEGIN
  template<class T> class ImageInterface;
  template<class T> class Matrix;
  class VisBuffer;
  class AzElAperture : public ATerm
  {
  public:
    AzElAperture(): ATerm(){};
    ~AzElAperture() {};
    virtual void rotate(const VisBuffer& vb, CFStore& cfs) 
    {
      LogIO log_l(LogOrigin("AzElAperture", "rotate"));
      Double actualPA = getPA(vb), currentCFPA = cfs.pa.getValue("rad");
      CFStore tmp;
      tmp.set(cfs);
      (*tmp.data).assign(*cfs.data);

      if (fabs(actualPA-currentCFPA) > 0.1)
	{
	  SynthesisUtils::rotateComplexArray(log_l, *tmp.data, cfs.coordSys,
					     *cfs.data,
					     currentCFPA-actualPA,
					     "LINEAR");
	  cfs.pa=Quantity(actualPA, "rad");
	}
    };
  };
};
#endif
