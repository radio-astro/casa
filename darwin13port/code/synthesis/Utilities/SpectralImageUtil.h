//# SpectralImageUtil.h: Definitions for Stokes Image utilities
//# Copyright (C) 2013
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
#ifndef SYNTHESIS_SPECTRALIMAGEUTIL_H
#define SYNTHESIS_SPECTRALIMAGEUTIL_H

#include <images/Images/ImageInterface.h>
#include <images/Images/SubImage.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SpectralImageUtil {

 public:
  //Returns a pointer to a subimage of the requested plane
  //Note caller is responsible for deleting the subimage object
  // If writeaccess is True ...modification on the subimage will get recorded on the original image if it is writeable
  static SubImage<Float>* getChannel(ImageInterface<Float>& theIm, Int begchannel=0,  Int endchannel=-1, Bool writeAccess=False);
  //Frequency at a given pixel along the spectral axis (value returned is in Hz)
  static Double worldFreq(const CoordinateSystem& cs, Double spectralpix=0.0);

};//end of class

} //# NAMESPACE CASA - END

#endif

