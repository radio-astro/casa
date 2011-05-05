// -*- C++ -*-
//# VisibilityResampler.cc: Implementation of the VisibilityResampler class
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

#include <synthesis/MeasurementComponents/SynthesisError.h>
#include <synthesis/MeasurementComponents/VisibilityResampler.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <msvis/MSVis/AsynchronousTools.h>
#include <fstream>

namespace casa{
  //
  //-----------------------------------------------------------------------------------
  //
  // VisibilityResampler& VisibilityResampler::operator=(const VisibilityResampler& other)
  // {
  //   SynthesisUtils::SETVEC(uvwScale_p, other.uvwScale_p);
  //   SynthesisUtils::SETVEC(offset_p, other.offset_p);
  //   SynthesisUtils::SETVEC(dphase_p, other.dphase_p);
  //   SynthesisUtils::SETVEC(chanMap_p, other.chanMap_p);
  //   SynthesisUtils::SETVEC(polMap_p, other.polMap_p);

  //   convFuncStore_p = other.convFuncStore_p;
  //   myMutex_p = other.myMutex_p;
  //   return *this;
  // }
  //
  //-----------------------------------------------------------------------------------
  // Re-sample the griddedData on the VisBuffer (a.k.a gridding)
  //
  // Template instantiations for re-sampling onto a double precision
  // or single precision grid.
  //
  template
  void VisibilityResampler::DataToGridImpl_p(Array<DComplex>& grid, VBStore& vbs, 
					     const Bool& dopsf,  Matrix<Double>& sumwt) __restrict__;
  template
  void VisibilityResampler::DataToGridImpl_p(Array<Complex>& grid, VBStore& vbs, 
  					     const Bool& dopsf,  Matrix<Double>& sumwt) __restrict__;

  // template void VisibilityResampler::addTo4DArray(DComplex* store,const Int* iPos, Complex& val, Double& wt) __restrict__;
  // template void VisibilityResampler::addTo4DArray(Complex* store,const Int* iPos, Complex& val, Double& wt) __restrict__;
  //
  //-----------------------------------------------------------------------------------
  // Template implementation for DataToGrid
  //
  template <class T>
  void VisibilityResampler::DataToGridImpl_p(Array<T>& grid,  VBStore& vbs, const Bool& dopsf,
					     Matrix<Double>& sumwt)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny;
    Int achan, apol, rbeg, rend;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(2), off(2), iloc(2);
    Vector<Double> pos(2);

    //    IPosition grdpos(4);
    Vector<Int> igrdpos(4);

    Double norm=0, wt, imgWt;
    Complex phasor, nvalue;

    rbeg = vbs.beginRow_p;
    rend = vbs.endRow_p;
    //    cerr << rbeg << " " << rend << " " << vbs.nRow() << endl;
    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
    support(0) = convFuncStore_p.xSupport[0];
    support(1) = convFuncStore_p.ySupport[0];

    Bool Dummy, gDummy;
    T __restrict__ *gridStore = grid.getStorage(gDummy);
    Int * __restrict__ iPosPtr = igrdpos.getStorage(Dummy);
    const Int * __restrict__ iPosPtrConst = iPosPtr;
    Double *__restrict__ convFunc=(*(convFuncStore_p.rdata)).getStorage(Dummy);
    Double * __restrict__ freq=vbs.freq_p.getStorage(Dummy);
    Bool * __restrict__ rowFlag=vbs.rowFlag_p.getStorage(Dummy);

    Float * __restrict__ imagingWeight = vbs.imagingWeight_p.getStorage(Dummy);
    Double * __restrict__ uvw = vbs.uvw_p.getStorage(Dummy);
    Bool * __restrict__ flagCube = vbs.flagCube_p.getStorage(Dummy);
    Complex * __restrict__ visCube = vbs.visCube_p.getStorage(Dummy);
    Double * __restrict__ scale = uvwScale_p.getStorage(Dummy);
    Double * __restrict__ offset = offset_p.getStorage(Dummy);
    Float * __restrict__ samplingPtr = sampling.getStorage(Dummy);
    Double * __restrict__ posPtr=pos.getStorage(Dummy);
    Int * __restrict__ locPtr=loc.getStorage(Dummy);
    Int * __restrict__ offPtr=off.getStorage(Dummy);
    Double * __restrict__ sumwtPtr = sumwt.getStorage(Dummy);
    Int * __restrict__ ilocPtr=iloc.getStorage(Dummy);
    Int * __restrict__ supportPtr = support.getStorage(Dummy);
    Int nDim = vbs.uvw_p.shape()[0];

    //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);
    cacheAxisIncrements(grid.shape().asVector());

    for(Int irow=rbeg; irow < rend; irow++){          // For all rows
      
      if(!rowFlag[irow]){                        // If the row is not flagged
	
	for(Int ichan=0; ichan< nDataChan; ichan++){ // For all channels
	  
	  //	  if (vbs.imagingWeight(ichan,irow)!=0.0) {  // If weights are not zero
	  if (imagingWeight[ichan+irow*nDataChan]!=0.0) {  // If weights are not zero
	    achan=chanMap_p(ichan);
	    
	    if((achan>=0) && (achan<nGridChan)) {   // If selected channels are valid
	      
	      // sgrid(pos,loc,off, phasor, irow, 
	      // 	    vbs.uvw,dphase_p[irow], vbs.freq[ichan], 
	      // 	    uvwScale_p, offset_p, sampling);
	      sgrid(nDim,posPtr,locPtr,offPtr, phasor, irow, 
		    uvw,dphase_p[irow], freq[ichan], 
		    scale, offset, samplingPtr);

	      if (onGrid(nx, ny, loc, support)) {   // If the data co-ords. are with-in the grid
		
		for(Int ipol=0; ipol< nDataPol; ipol++) { // For all polarizations
		  // if((!vbs.flagCube(ipol,ichan,irow))){   // If the pol. & chan. specific
		  if((!flagCube[ipol+ichan*nDataPol+irow*nDataChan*nDataPol])){
		    apol=polMap_p(ipol);
		    if ((apol>=0) && (apol<nGridPol)) {
		      //	      igrdpos(2)=apol; igrdpos(3)=achan;
		      iPosPtr[2]=apol; iPosPtr[3]=achan;
		      norm=0.0;

		      imgWt=imagingWeight[ichan+irow*nDataChan];
		      if(dopsf)  nvalue=Complex(imgWt);
		      else	 nvalue=imgWt*
				   // (vbs.visCube(ipol,ichan,irow)*phasor);
				   (visCube[ipol+ichan*nDataPol+irow*nDataPol*nDataChan]*phasor);

		      for(Int iy=-supportPtr[1]; iy <= supportPtr[1]; iy++) 
			{
			  ilocPtr[1]=abs((int)(samplingPtr[1]*iy+offPtr[1]));
			  //			  igrdpos(1)=loc(1)+iy;
			  iPosPtr[1]=locPtr[1]+iy;
			  //		  wt = convFunc[ilocPtr[1]];
			  for(Int ix=-supportPtr[0]; ix <= supportPtr[0]; ix++) 
			    {
			      ilocPtr[0]=abs((int)(samplingPtr[0]*ix+offPtr[0]));
			      wt = convFunc[iloc[0]]*convFunc[iloc[1]];
			      //wt *= convFunc[ilocPtr[0]];

			      //igrdpos(0)=loc(0)+ix;
			      iPosPtr[0]=locPtr[0]+ix;


			      // grid(grdpos) += nvalue*wt;

			      // The following uses raw index on the 4D grid
			      addTo4DArray(gridStore,iPosPtr,nvalue,wt);
			      norm+=wt;
			    }
			}
		      //		      sumwtPtr[apol+achan*nGridPol]+=imgWt*norm;
		      sumwt(apol,achan)+=imgWt*norm;
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    T *tt=(T *)gridStore;
    grid.putStorage(tt,gDummy);
  }
  //
  //-----------------------------------------------------------------------------------
  // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
  //
  void VisibilityResampler::GridToData(VBStore& vbs, const Array<Complex>& grid)
  //  void VisibilityResampler::GridToData(VBStore& vbs, Array<Complex>& grid)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny;
    Int achan, apol, rbeg, rend;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(2), off(2), iloc(2);
    Vector<Double> pos(2);

    IPosition grdpos(4);

    Double norm=0, wt;
    Complex phasor, nvalue;

    // rbeg=0;
    // rend=vbs.nRow_p;
    rbeg = vbs.beginRow_p;
    rend = vbs.endRow_p;
    //    cerr << rbeg << " " << rend << " " << vbs.nRow() << endl;
    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
    support(0) = convFuncStore_p.xSupport[0];
    support(1) = convFuncStore_p.ySupport[0];

    Bool Dummy,vbcDummy;
    const Complex *__restrict__ gridStore = grid.getStorage(Dummy);
    Vector<Int> igrdpos(4);
    Int *__restrict__ iPosPtr = igrdpos.getStorage(Dummy);
    const Int *__restrict__ iPosPtrConst = iPosPtr;
    Double *__restrict__ convFunc=(*(convFuncStore_p.rdata)).getStorage(Dummy);
    Double *__restrict__ freq=vbs.freq_p.getStorage(Dummy);
    Bool *__restrict__ rowFlag=vbs.rowFlag_p.getStorage(Dummy);

    Float * __restrict__ imagingWeight = vbs.imagingWeight_p.getStorage(Dummy);
    Double * __restrict__ uvw = vbs.uvw_p.getStorage(Dummy);
    Bool * __restrict__ flagCube = vbs.flagCube_p.getStorage(Dummy);
    Complex * __restrict__ visCube = vbs.visCube_p.getStorage(vbcDummy);
    Double * __restrict__ scale = uvwScale_p.getStorage(Dummy);
    Double * __restrict__ offset = offset_p.getStorage(Dummy);
    Int * __restrict__ supportPtr = support.getStorage(Dummy);
    Float * __restrict__ samplingPtr = sampling.getStorage(Dummy);
    Double * __restrict__ posPtr=pos.getStorage(Dummy);
    Int * __restrict__ locPtr=loc.getStorage(Dummy);
    Int * __restrict__ offPtr=off.getStorage(Dummy);
    Int * __restrict__ ilocPtr=iloc.getStorage(Dummy);
    Int nDim = vbs.uvw_p.shape()(0);

    //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);
    cacheAxisIncrements(grid.shape().asVector());

    for(Int irow=rbeg; irow < rend; irow++) {
      if(!rowFlag[irow]) {

	for (Int ichan=0; ichan < nDataChan; ichan++) {
	  achan=chanMap_p(ichan);

	  if((achan>=0) && (achan<nGridChan)) {
	    // sgrid(pos,loc,off,phasor,irow,vbs.uvw,
	    // 	  dphase_p[irow],vbs.freq[ichan],
	    // 	  uvwScale_p,offset_p,sampling);
	    sgrid(nDim,posPtr,locPtr,offPtr,phasor,irow,uvw,
		  dphase_p[irow],freq[ichan],
		  scale,offset,samplingPtr);

	    if (onGrid(nx, ny, loc, support)) {
	      for(Int ipol=0; ipol < nDataPol; ipol++) {

		if(!flagCube[ipol+ichan*nDataPol+irow*nDataChan*nDataPol]) { 
		  apol=polMap_p(ipol);
		  
		  if((apol>=0) && (apol<nGridPol)) {
		    //		    igrdpos(2)=apol; igrdpos(3)=achan;
		    iPosPtr[2]=apol; iPosPtr[3]=achan;
		    nvalue=0.0;
		    norm=0.0;

		    for(Int iy=-supportPtr[1]; iy <= supportPtr[1]; iy++) 
		      {
			ilocPtr[1]=abs((Int)(samplingPtr[1]*iy+offPtr[1]));
			//			igrdpos(1)=loc(1)+iy;
			iPosPtr[1]=locPtr[1]+iy;
			//			wt = convFunc[ilocPtr[1]];
			for(Int ix=-supportPtr[0]; ix <= supportPtr[0]; ix++) 
			  {
			    ilocPtr[0]=abs((Int)(samplingPtr[0]*ix+offPtr[0]));
			    //			    igrdpos(0)=loc(0)+ix;
			    iPosPtr[0]=locPtr[0]+ix;
			    
			    wt=convFunc[ilocPtr[0]]*convFunc[ilocPtr[1]];
			    norm+=wt;
			    //			    nvalue+=wt*grid(grdpos);
			    // The following uses raw index on the 4D grid
			    nvalue+=wt*getFrom4DArray(gridStore,iPosPtrConst);
			  }
		      }
		    visCube[ipol+ichan*nDataPol+irow*nDataChan*nDataPol]=(nvalue*conj(phasor))/norm;
		  }
		}
	      }
	    }
	  }
	}
      }
    }
    Complex *tt=(Complex *) visCube;
    vbs.visCube_p.putStorage(tt,vbcDummy);
  }
  //
  //-----------------------------------------------------------------------------------
  //
  // void VisibilityResampler::sgrid(Int& uvwDim,Double* __restrict__ pos, 
  // 				  Int* __restrict__ loc, 
  // 				  Int* __restrict__ off, 
  // 				  Complex& phasor, const Int& irow,
  // 				  // const Matrix<Double>& __restrict__ uvw, 
  // 				  const Double* __restrict__ uvw, 
  // 				  const Double& __restrict__ dphase, 
  // 				  const Double& __restrict__ freq, 
  // 				  const Double* __restrict__ scale, 
  // 				  const Double* __restrict__ offset,
  // 				  const Float* __restrict__ sampling) __restrict__ 
  // 				  // const Vector<Double>& __restrict__ scale, 
  // 				  // const Vector<Double>& __restrict__ offset,
  // 				  // const Vector<Float>& __restrict__ sampling) __restrict__ 
  // {
  //   Double phase;
  //   //    Int ndim=pos.shape()(0);
  //   Int ndim=2;

  //   for(Int idim=0;idim<ndim;idim++)
  //     {
  // 	pos[idim]=scale[idim]*uvw[idim+irow*uvwDim]*freq/C::c+offset[idim];
  // 	loc[idim]=(Int)std::floor(pos[idim]+0.5);
  // 	off[idim]=(Int)std::floor(((loc[idim]-pos[idim])*sampling[idim])+0.5);
  //     }

  //   if (dphase != 0.0)
  //     {
  // 	phase=-2.0*C::pi*dphase*freq/C::c;
  // 	phasor=Complex(cos(phase), sin(phase));
  //     }
  //   else
  //     phasor=Complex(1.0);
  // }
  void VisibilityResampler::ComputeResiduals(VBStore& vbs)
  {
    Int rbeg = vbs.beginRow_p, rend = vbs.endRow_p;
    IPosition vbDataShape=vbs.modelCube_p.shape();
    IPosition start(vbDataShape), last(vbDataShape);
    start=0; start(2)=rbeg;
    last(2)=rend; //last=last-1;

    if (!vbs.useCorrected_p)
      {
	for(uInt ichan = start(0); ichan < last(0); ichan++)
	  for(uInt ipol = start(1); ipol < last(1); ipol++)
	    for(uInt irow = start(2); irow < last(2); irow++)
	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.visCube_p(ichan,ipol,irow);
      }
    else
      {
	for(uInt ichan = start(0); ichan < last(0); ichan++)
	  for(uInt ipol = start(1); ipol < last(1); ipol++)
	    for(uInt irow = start(2); irow < last(2); irow++)
	      vbs.modelCube_p(ichan,ipol,irow) = vbs.modelCube_p(ichan,ipol,irow) - vbs.correctedCube_p(ichan,ipol,irow);
      }
      

    // Slicer mySlice(start,last,Slicer::endIsLast);
    // Cube<Complex> slicedModel, slicedData, slicedCorrected;
    // if (!vbs.useCorrected_p) 
    //   {
    // 	{
    // 	  async::MutexLocker tt(*myMutex_p);
    // 	  slicedModel = Cube<Complex>(vbs.modelCube_p(mySlice));
    // 	  slicedData = Cube<Complex>(vbs.visCube_p(mySlice));
    // 	}
    // 	slicedModel -= slicedData;
    //   }
    // else
    //   {
    // 	{
    // 	  async::MutexLocker tt(*myMutex_p);
    // 	  slicedModel = Cube<Complex>(vbs.modelCube_p(mySlice));
    // 	  slicedCorrected = Cube<Complex>(vbs.correctedCube_p(mySlice));
    // 	}
    // 	slicedModel -= slicedCorrected;
    //   }
  }
};// end namespace casa
