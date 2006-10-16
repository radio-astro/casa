//# WBSkyEquation.h: WBSkyEquation definition
//# Copyright (C) 1996,1997,1998,1999,2000,2002,2003
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
//# $Id$


#ifndef SYNTHESIS_WBSKYEQUATION_H
#define SYNTHESIS_WBSKYEQUATION_H
#include <synthesis/MeasurementComponents/WFGridFT.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <images/Images/ImageRegion.h>
#include <synthesis/MeasurementComponents/WBCleanImageSkyModel.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WBSkyEquation : public SkyEquation {
public:

  WBSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, ComponentFTMachine& cft);

  void makeApproxPSF(Int model, ImageInterface<Float>& psf);
  void wbfullGradientsChiSquared(Bool incremental);
  void gradientsChiSquared(Bool incremental, Bool commitModel=False);
  void initializePut(const VisBuffer& vb, Int model, Bool doPSF);
  void finalizePut(const VisBuffer& vb, Int model, Bool doPSF);
  void initializeGet(const VisBuffer& vb, Int row, Int model,Bool incremental); 
  virtual VisBuffer& get(VisBuffer& result, Int model,Bool incremental);
  void predict(Bool incremental);
  //// Do something about this !!!
virtual VisBuffer& get(VisBuffer& vb, const ComponentList& components){};
 private:

  
  WBCleanImageSkyModel *itsSM; // =sm_
  LogIO os;
  Int ntaylor_p;
  Int adbg;

};

} //# NAMESPACE CASA - END

#endif
