//# ImagerProxy.h: 
//# Copyright (C) 2006
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
//# $Id: ImagerProxy.h,v 1.2 2007/01/31 02:18:09 mmarquar Exp $
#ifndef SYNTHESIS_IMAGERPROXY_H
#define SYNTHESIS_IMAGERPROXY_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
//# Forward declarations
namespace casa { //# NAMESPACE CASA - BEGIN
  //  class String;
  class Record;
  class Imager;
  template<class T> class Vector;

class ImagerProxy 
{
 public:
  ImagerProxy();
  ImagerProxy(const String& thems, Bool compress=False);
  virtual ~ImagerProxy();
  //Bool open(String &thems, Bool compress=False);

  Bool setimage(Int nx, Int ny,
                const Record& cellx, const Record& celly,
                const String& stokes,
                Bool doShift,
                const Record& phaseCenter, 
                const Record& shiftx, const Record& shifty,
                const String& mode, Int nchan,
                Int start, Int step,
                const Record& mStart, const Record& mStep,
                const Vector<Int>& spectralwindowids, 
		Int fieldid,
                Int facets, const Record& distance,
                Float paStep, Float pbLimit);

  // Set the data selection parameters
  Bool setdata(const String& mode, const Vector<Int>& nchan, 
               const Vector<Int>& start,
               const Vector<Int>& step,
               const Vector<Int>& spectralwindowids,
               const Vector<Int>& fieldid,
               const String& msSelect="", const String& msname="");
  
  // Set the processing options
  Bool setoptions(const String& ftmachine, Int cache,
		  Int tile,
                  const String& gridfunction, 
		  const Record& mLocation,
                  Float padding, 
		  Bool usemodelcol=True, 
                  Int wprojplanes=1,
                  const String& epJTableName="",
                  Bool applyPointingOffsets=True,
                  Bool doPointingCorrection=True,
                  const String& cfCacheDirName="");
  // Weight the MeasurementSet
  Bool weight(const String& algorithm, const String& rmode,
	      const Record& noise, Double robust,
	      const Record& fieldofview, Int npixels);

  Bool filter(const String& type, const Record& bmaj,
	      const Record& bmin, const Record& bpa);

  Bool setmfcontrol(Float cyclefactor,
		    Float cyclespeedup,
		    Int stoplargenegatives, 
		    Int stoppointmode,
		    const String& scaleType,
		    Float minPB,
		    Float constPB,
		    const Vector<String>& fluxscale);
  Bool setscales(const String& scaleMethod,
		 Int inscales,
		 const Vector<Float>& userScaleSizes);
  Bool clean(const String& algorithm,
	     Int niter, 
	     Float gain,
	     const Record& threshold,//quant 
	     const Bool displayProgress, 
	     const Vector<String>& model, const Vector<Bool>& fixed,
	     const String& complist,
	     const Vector<String>& mask,
	     const Vector<String>& image,
	     const Vector<String>& residual);
  // Make plain image
  Bool makeimage(const String& type, const String& imageName);
  
 private:
  Imager* itsImager;

};

}
#endif
