//# SDMaskHandler.h: Definition for SDMaskHandler
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

#ifndef SYNTHESIS_SDMASKHANDLER_H
#define SYNTHESIS_SDMASKHANDLER_H

#include <ms/MeasurementSets/MeasurementSet.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <images/Images/TempImage.h>
#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>

#include<synthesis/ImagerObjects/SIImageStore.h>
#include<synthesis/ImagerObjects/SIImageStoreMultiTerm.h>
#include <synthesis/ImagerObjects/InteractiveMasking.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SDMaskHandler 
{
public:

  // Empty constructor
  SDMaskHandler();
  ~SDMaskHandler();

  void resetMask(CountedPtr<SIImageStore> imstore);

  void makeMask();

  int makeInteractiveMask(CountedPtr<SIImageStore>& imstore,
			  Int& niter, Int& ncycles, 
			  String& threshold);

  // Return a reference to an imageinterface for the mask.
  void makeAutoMask(CountedPtr<SIImageStore> imstore);

  void makePBMask(CountedPtr<SIImageStore> imstore, Float weightlimit);
protected:
  InteractiveMasking *interactiveMasker_p;
};

} //# NAMESPACE CASA - END


#endif


