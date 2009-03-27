//# CleanImageSkyModel.cc: Implementation of CleanImageSkyModel classes
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2003
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

#include <casa/Arrays/ArrayMath.h>
#include <synthesis/MeasurementComponents/CleanImageSkyModel.h>
#include <components/ComponentModels/SkyComponent.h>
#include <components/ComponentModels/ComponentList.h>
#include <casa/OS/File.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>

#include <tables/Tables/TableLock.h>

#include <casa/sstream.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>

namespace casa { //# NAMESPACE CASA - BEGIN

CleanImageSkyModel::CleanImageSkyModel() : ImageSkyModel()
{
}

Bool CleanImageSkyModel::addMask(Int thismodel, ImageInterface<Float>& mask)
{
  LogIO os(LogOrigin("CleanImageSkyModel", "addMask"));
  if(thismodel>=nmodels_p||thismodel<0) {
    os << LogIO::SEVERE << "Illegal model slot" << thismodel << LogIO::POST;
    return False;
  }
  if(Int(mask_p.nelements())<=thismodel) mask_p.resize(thismodel+1);
  mask_p[thismodel] = &mask;
  AlwaysAssert(mask_p[thismodel], AipsError);
  return True;
}
  
CleanImageSkyModel::CleanImageSkyModel(const CleanImageSkyModel& other) {
  operator=(other);
};

CleanImageSkyModel::~CleanImageSkyModel() {
};

CleanImageSkyModel& CleanImageSkyModel::operator=(const CleanImageSkyModel& other) {
  if(this!=&other) {
    for (Int thismodel=0;thismodel<nmodels_p;thismodel++) {
      mask_p[thismodel]=other.mask_p[thismodel];
      fluxmask_p[thismodel]=other.fluxmask_p[thismodel];
    }
  };
  return *this;
}

Bool CleanImageSkyModel::add(ComponentList& compList)
{
  return ImageSkyModel::add(compList);
}
 
Int CleanImageSkyModel::add(ImageInterface<Float>& image, const Int maxNumXfr)
{
  Int index=ImageSkyModel::add(image, maxNumXfr);

  mask_p.resize(nmodels_p); 
  fluxmask_p.resize(nmodels_p);
  
  mask_p[index]=0;
  fluxmask_p[index]=0;

  return index;
}

Bool CleanImageSkyModel::hasMask(Int model) 
{
  if(mask_p.nelements()==0) return False;
  return (mask_p[model]);
}

ImageInterface<Float>& CleanImageSkyModel::mask(Int model) 
{
  AlwaysAssert(nmodels_p>0, AipsError);
  AlwaysAssert((model>-1)&&(model<nmodels_p), AipsError);
  AlwaysAssert(mask_p[model], AipsError);
  return *mask_p[model];
};

Bool CleanImageSkyModel::hasFluxMask(Int model) 
{
  if(fluxmask_p.nelements()==0) return False;
  return (fluxmask_p[model]);
}

ImageInterface<Float>& CleanImageSkyModel::fluxMask(Int model) 
{
  AlwaysAssert(nmodels_p>0, AipsError);
  AlwaysAssert((model>-1)&&(model<nmodels_p), AipsError);
  AlwaysAssert(fluxmask_p[model], AipsError);
  return *fluxmask_p[model];
};

Bool CleanImageSkyModel::addFluxMask(Int thismodel, ImageInterface<Float>& fluxMask)
{
  LogIO os(LogOrigin("CleanImageSkyModel", "add"));
  if(thismodel>=nmodels_p||thismodel<0) {
    os << LogIO::SEVERE << "Illegal model slot" << thismodel << LogIO::POST;
    return False;
  }
  if(Int(fluxmask_p.nelements())<=thismodel) fluxmask_p.resize(thismodel+1);
  fluxmask_p[thismodel] = &fluxMask;
  AlwaysAssert(fluxmask_p[thismodel], AipsError);
  return True;
}



} //# NAMESPACE CASA - END

