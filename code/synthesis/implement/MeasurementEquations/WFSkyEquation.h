//# WFSkyEquation.h: WFSkyEquation definition
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


#ifndef SYNTHESIS_WFSKYEQUATION_H
#define SYNTHESIS_WFSKYEQUATION_H
#include <synthesis/MeasurementComponents/WFGridFT.h>
#include <synthesis/MeasurementEquations/SkyEquation.h>
#include <images/Images/ImageRegion.h>
#include <synthesis/MeasurementComponents/WFCleanImageSkyModel.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class WFSkyEquation : public SkyEquation {
public:

  WFSkyEquation(SkyModel& sm, VisSet& vs, FTMachine& ft, ComponentFTMachine& cft);


  //Make all the facets PSFs in one pass through the visibilities

  virtual void makeMultiApproxPSF(PtrBlock<TempImage<Float> * >& psfMulti,
				       Int nmodels) ;
  
  
  //  virtual void predict(Bool incremental=False);
  virtual void gradientsChiSquared(Bool incremental, Bool hasModel=True);

  
  virtual void initializeMultiGet(const VisBuffer& vb, Int row, Int nmodel,
				  Bool incremental);

  virtual VisBuffer& multiGet(VisBuffer& vb, 
			      Int nmodels, Bool incremental);   

  virtual void initializeMultiPut(const VisBuffer &vb, Int model); 

  virtual void multiPut(const VisBuffer& vb, Int nmodel, Bool dopsf=False); 

  virtual void finalizeMultiPut(const VisBuffer& vb, Int Model);  

  virtual void multiPredict(Bool incremental);

 private:

  WFGridFT *itsFT; // =ft_
  WFGridFT *itsIFT; // = ift_
  
  WFCleanImageSkyModel *itsSM; // =sm_


};


} //# NAMESPACE CASA - END

#endif
