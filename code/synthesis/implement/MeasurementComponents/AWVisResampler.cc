// -*- C++ -*-
//# AWVisResampler.cc: Implementation of the AWVisResampler class
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
#include <synthesis/MeasurementComponents/AWVisResampler.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <fstream>
//#include <casa/BasicMath/Functors.h>
namespace casa{
  //
  //-----------------------------------------------------------------------------------
  // Re-sample the griddedData on the VisBuffer (a.k.a gridding)
  //
  // Template instantiations for re-sampling onto a double precision
  // or single precision grid.
  //
  template
  void AWVisResampler::DataToGridImpl_p(Array<DComplex>& grid, VBStore& vbs, 
					Matrix<Double>& sumwt,const Bool& dopsf,
					CFStore& cfs) __restrict__;
  template
  void AWVisResampler::DataToGridImpl_p(Array<Complex>& grid, VBStore& vbs, 
					Matrix<Double>& sumwt,const Bool& dopsf,
					CFStore& cfs) __restrict__;

  //
  //-----------------------------------------------------------------------------------
  //
  // AWVisResampler& AWVisResampler::operator=(const AWVisResampler& other)
  // {
  //   SETVEC(uvwScale_p, other.uvwScale_p);
  //   SETVEC(offset_p, other.offset_p);
  //   SETVEC(dphase_p, other.dphase_p);
  //   SETVEC(chanMap_p, other.chanMap_p);
  //   SETVEC(polMap_p, other.polMap_p);

  //   convFuncStore_p = other.convFuncStore_p;

  //   return *this;
  // }
  //
  //-----------------------------------------------------------------------------------
  // Template implementation for DataToGrid
  //
  template <class T>
  void AWVisResampler::DataToGridImpl_p(Array<T>& grid,  VBStore& vbs, 
					Matrix<Double>& sumwt,const Bool& dopsf,
					CFStore& cfs)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny, nw;
    Int achan, apol, rbeg, rend, PolnPlane, ConjPlane;
    
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(3), off(3), iloc(4),tiloc(4),scaledSampling(2), scaledSupport(2);
    Vector<Double> pos(2);

    //    IPosition grdpos(4);
    Vector<Int> igrdpos(4);

    Double norm=0.0;
    Complex phasor, nvalue, wt;
    Vector<Int> cfShape=convFuncStore_p.data->shape().asVector();
    Vector<Int> convOrigin = (cfShape-1)/2;
    Double sinDPA=0.0, cosDPA=1.0;
    Double cfScale,lambda, cfRefFreq = convFuncStore_p.coordSys.
      spectralCoordinate(convFuncStore_p.coordSys.findCoordinate(Coordinate::SPECTRAL))
      .referenceValue()(0);

    // static Double tt=-1;
    // if (vbs.freq[0] != tt) {tt=vbs.freq[0];cerr << tt << endl;}

    rbeg = 0;       rend = vbs.nRow_p;
    rbeg = vbs.beginRow_p;
    rend = vbs.endRow_p;
    
    nx = grid.shape()[0]; ny = grid.shape()[1]; nw = cfShape[2];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
    support(0) = convFuncStore_p.xSupport[0];
    support(1) = convFuncStore_p.ySupport[0];

    Bool Dummy, gDummy;
    T* __restrict__ gridStore = grid.getStorage(gDummy);
    const Int * __restrict__ iPosPtr = igrdpos.getStorage(Dummy);
    const Complex* __restrict__ convFuncV=cfs.data->getStorage(Dummy);
      
    Double *freq=vbs.freq_p.getStorage(Dummy);
    Bool *rowFlag=vbs.rowFlag_p.getStorage(Dummy);

    // Matrix<Float>&  imagingWeight=vbs.imagingWeight_p;
    // Matrix<Double>& uvw=vbs.uvw_p;
    // Cube<Complex>&  visCube=vbs.visCube_p;
    // Cube<Bool>&     flagCube=vbs.flagCube_p;

    //    Vector<Int> gridInc, cfInc;
    //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan); 

    // Cache increment values for adding to grid in gridInc.  This is
    // supplied to addTo4DArray later.
    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);
    // Cache the CF related increments intenrally in
    // VisibilityResamplerBase for use in getFrom4DArray later.
    cacheAxisIncrements(cfShape, cfInc_p);
    //    IsNaN<Complex> testNaN;

    Bool * __restrict__ flagCube_ptr=vbs.flagCube_p.getStorage(Dummy);
    Bool * __restrict__ rowFlag_ptr = vbs.rowFlag_p.getStorage(Dummy);;
    Float * __restrict__ imgWts_ptr = vbs.imagingWeight_p.getStorage(Dummy);
    Complex * __restrict__ visCube_ptr = vbs.visCube_p.getStorage(Dummy);
    Double * __restrict__ sumWt_ptr = sumwt.getStorage(Dummy);

    // {
    //   IPosition tt(4);
    //   for(tt(0)=0;tt(0)<grid.shape()(0);tt(0)++)
    // 	for(tt(1)=0;tt(1)<grid.shape()(1);tt(1)++)
    // 	  for(tt(2)=0;tt(2)<grid.shape()(2);tt(2)++)
    // 	    for(tt(3)=0;tt(3)<grid.shape()(3);tt(3)++)
    // 	      grid(tt)*=1.0;
    // }

    for(Int irow=rbeg; irow< rend; irow++){   
      //      if(!rowFlag[irow]){   
      if(!(*(rowFlag_ptr+irow))){   
	for(Int ichan=0; ichan< nDataChan; ichan++){
	  //	  if (imagingWeight(ichan,irow)!=0.0) {  
	  if (*(imgWts_ptr + ichan+irow*nDataChan)!=0.0) {  
	    achan=chanMap_p[ichan];
	    
	    if((achan>=0) && (achan<nGridChan)) {
	      lambda = C::c/freq[ichan];
	      cfScale = cfRefFreq/freq[ichan];

	      scaledSampling[0] = SynthesisUtils::nint(sampling[0]*cfScale);
	      scaledSampling[1] = SynthesisUtils::nint(sampling[1]*cfScale);
	      scaledSupport[0]  = SynthesisUtils::nint(support[0]/cfScale);
	      scaledSupport[1]  = SynthesisUtils::nint(support[1]/cfScale);


	      sgrid(pos,loc,off, phasor, irow, 
		    vbs.uvw_p, dphase_p[irow], freq[ichan], 
		    uvwScale_p, offset_p, sampling);

	      iloc[2]=max(0, min(nw-1, loc[2]));

	      if (onGrid(nx, ny, nw, loc, support)) { 

		for(Int ipol=0; ipol< nDataPol; ipol++) { 
		  //		  if((!flagCube(ipol,ichan,irow))){  
		  if((!(*(flagCube_ptr + ipol + ichan*nDataPol + irow*nDataPol*nDataChan)))){  
		    apol=polMap_p(ipol);
		    if ((apol>=0) && (apol<nGridPol)) {
		      igrdpos[2]=apol; igrdpos[3]=achan;
		      
		      norm=0.0;
		      ConjPlane = cfMap_p[ipol];
		      PolnPlane = conjCFMap_p[ipol];

		      iloc[3]=PolnPlane;

		      // if(dopsf)  nvalue=Complex(imagingWeight(ichan,irow));
		      // else	 nvalue=imagingWeight(ichan,irow)*
		      // 		   (visCube(ipol,ichan,irow)*phasor);
		      // if ((imagingWeight(ichan,irow) > 0.0) || (visCube(ipol,ichan,irow) > 0.0))
		      // 	cerr << imagingWeight(ichan,irow) << " " << visCube(ipol,ichan,irow)
		      // 	     << " " << irow << " " << ichan << " " << ipol << endl;
		      if(dopsf)  nvalue=Complex(*(imgWts_ptr + ichan + irow*nDataChan));
		      else	 nvalue= *(imgWts_ptr+ichan+irow*nDataChan)*
		      		   (*(visCube_ptr+ipol+ichan*nDataPol+irow*nDataChan*nDataPol)*phasor);
		      for(Int iy=-scaledSupport[1]; iy <= scaledSupport[1]; iy++) 
			{
			  iloc(1)=(Int)(scaledSampling[1]*iy+off[1]);
			  igrdpos[1]=loc[1]+iy;
			  for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++) 
			    {
			      iloc[0]=(Int)(scaledSampling[0]*ix+off[0]);
			      tiloc=iloc;
			      if (reindex(iloc,tiloc,sinDPA, cosDPA, 
			      		  convOrigin, cfShape))
				{
				  wt = getFrom4DArray(convFuncV, tiloc,cfInc_p);
				  igrdpos[0]=loc[0]+ix;
				  //				  grid(igrdpos) += nvalue*wt;

				  // The following uses raw index on the 4D grid
				  addTo4DArray(gridStore,iPosPtr,gridInc_p, nvalue,wt);
				  //				  norm+=real(wt);
				}
			    }
			}
		      //		      sumwt(apol,achan)+=imagingWeight(ichan,irow);//*norm;
		      *(sumWt_ptr+apol+achan*nGridChan)+= *(imgWts_ptr+ichan+irow*nDataChan);
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
  void AWVisResampler::GridToData(VBStore& vbs, const Array<Complex>& grid)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny,nw;
    Int achan, apol, rbeg, rend, PolnPlane, ConjPlane;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(3), off(3), iloc(4),tiloc(4), scaledSampling(2), scaledSupport(2);
    Vector<Double> pos(2);

    IPosition grdpos(4);

    Vector<Complex> norm(4,0.0);
    Complex phasor, nvalue, wt;
    Vector<Int> cfShape(convFuncStore_p.data->shape().asVector());
    //    Vector<Int> convOrigin = (cfShape-1)/2;
    Vector<Int> convOrigin = (cfShape-1)/2;
    Double sinDPA=0.0, cosDPA=1.0;
    Double cfScale,lambda, cfRefFreq = convFuncStore_p.coordSys.
      spectralCoordinate(convFuncStore_p.coordSys.findCoordinate(Coordinate::SPECTRAL))
      .referenceValue()(0);

    rbeg=0;
    rend=vbs.nRow_p;
    rbeg = vbs.beginRow_p;
    rend = vbs.endRow_p;
    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nw       = cfShape[2];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
    support(0) = convFuncStore_p.xSupport[0];
    support(1) = convFuncStore_p.ySupport[0];
    //
    // The following code reduces most array accesses to the simplest
    // possible to improve performance.  However this made no
    // difference in the run-time performance compared to Vector,
    // Matrix and Cube indexing.
    //
    Bool Dummy;
    const Complex *gridStore = grid.getStorage(Dummy);
    Vector<Int> igrdpos(4);
    const Int *iPosPtr = igrdpos.getStorage(Dummy);
    Complex *convFunc=(*(convFuncStore_p.data)).getStorage(Dummy);
    const Complex* __restrict__ convFuncV=convFuncStore_p.data->getStorage(Dummy);
    Double *freq=vbs.freq_p.getStorage(Dummy);
    Bool *rowFlag=vbs.rowFlag_p.getStorage(Dummy);

    Matrix<Float>&  imagingWeight=vbs.imagingWeight_p;
    Matrix<Double>& uvw=vbs.uvw_p;
    Cube<Complex>&  visCube=vbs.visCube_p;
    Cube<Bool>&     flagCube=vbs.flagCube_p;

    Vector<Int> gridInc, cfInc;

    //    cacheAxisIncrements(nx,ny,nGridPol, nGridChan);
    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);
    cacheAxisIncrements(cfShape, cfInc_p);

    for(Int irow=rbeg; irow<rend; irow++) {
      if(!rowFlag[irow]) {

	for (Int ichan=0; ichan < nDataChan; ichan++) {
	  achan=chanMap_p[ichan];

	  if((achan>=0) && (achan<nGridChan)) {

	    lambda = C::c/freq[ichan];
	    cfScale = cfRefFreq/freq[ichan];

	    scaledSampling[0] = SynthesisUtils::nint(sampling[0]*cfScale);
	    scaledSampling[1] = SynthesisUtils::nint(sampling[1]*cfScale);
	    scaledSupport[0]  = SynthesisUtils::nint(support[0]/cfScale);
	    scaledSupport[1]  = SynthesisUtils::nint(support[1]/cfScale);


	    sgrid(pos,loc,off,phasor,irow,uvw,dphase_p[irow],freq[ichan],
		  uvwScale_p,offset_p,sampling);

	    iloc[2]=max(0, min(nw-1, loc[2]));

	    if (onGrid(nx, ny, nw, loc, support)) {
	      for(Int ipol=0; ipol < nDataPol; ipol++) {

		if(!flagCube(ipol,ichan,irow)) { 
		  apol=polMap_p[ipol];
		  
		  if((apol>=0) && (apol<nGridPol)) {
		    igrdpos[2]=apol; igrdpos[3]=achan;
		    nvalue=0.0;      norm(apol)=0.0;

		    ConjPlane = cfMap_p(ipol);
		    PolnPlane = conjCFMap_p(ipol);

		    iloc[3]=PolnPlane;

		    for(Int iy=-scaledSupport[1]; iy <= scaledSupport[1]; iy++) 
		      {
			iloc(1)=(Int)(scaledSampling[1]*iy+off[1]);
			igrdpos[1]=loc[1]+iy;
			
			for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++) 
			  {
			    iloc(0)=(Int)(scaledSampling[0]*ix+off[0]);
			    igrdpos[0]=loc[0]+ix;
			    tiloc=iloc;
			    if (reindex(iloc,tiloc,sinDPA, cosDPA, 
			    		convOrigin, cfShape))
			      {
				wt=getFrom4DArray(convFuncV,tiloc,cfInc_p);
				norm(apol)+=(wt);
				//			    nvalue+=wt*grid(grdpos);
				// The following uses raw index on the 4D grid
				//				nvalue+=wt*getFrom4DArray(gridStore,iPosPtr,gridInc);
				nvalue+=wt*getFrom4DArray(gridStore,igrdpos,gridInc_p);
			      }
			  }
		      }
		    visCube(ipol,ichan,irow)=(nvalue*conj(phasor))/norm(apol);
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
  //
  //-----------------------------------------------------------------------------------
  //
  void AWVisResampler::sgrid(Vector<Double>& pos, Vector<Int>& loc, 
			     Vector<Int>& off, Complex& phasor, 
			     const Int& irow, const Matrix<Double>& uvw, 
			     const Double& dphase, const Double& freq, 
			     const Vector<Double>& scale, 
			     const Vector<Double>& offset,
			     const Vector<Float>& sampling)
  {
    Double phase;
    Vector<Double> uvw_l(3,0); // This allows gridding of weights
			       // centered on the uv-origin
    if (uvw.nelements() > 0) for(Int i=0;i<3;i++) uvw_l[i]=uvw(i,irow);

    pos(2)=sqrt(abs(scale[2]*uvw_l(2)*freq/C::c))+offset[2];
    loc(2)=SynthesisUtils::nint(pos[2]);
    off(2)=0;

    for(Int idim=0;idim<2;idim++)
      {
	pos[idim]=scale[idim]*uvw_l(idim)*freq/C::c+offset[idim];
	loc[idim]=SynthesisUtils::nint(pos[idim]);
	off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]);
      }

    if (dphase != 0.0)
      {
	phase=-2.0*C::pi*dphase*freq/C::c;
	phasor=Complex(cos(phase), sin(phase));
      }
    else
      phasor=Complex(1.0);
  }
  //
  //-----------------------------------------------------------------------------------
  //
  Bool AWVisResampler::reindex(const Vector<Int>& in, Vector<Int>& out,
			       const Double& sinDPA, const Double& cosDPA,
			       const Vector<Int>& Origin, const Vector<Int>& size)
  {
    
    Bool onGrid=False;
    Int ix=in[0], iy=in[1];
    if (sinDPA != 0.0)
      {
	ix = SynthesisUtils::nint(cosDPA*in[0] + sinDPA*in[1]);
	iy = SynthesisUtils::nint(-sinDPA*in[0] + cosDPA*in[1]);
      }
    out[0]=ix+Origin[0];
    out[1]=iy+Origin[1];

    onGrid = ((out[0] >= 0) && (out[0] < size[0]) &&
	      (out[1] >= 0) && (out[1] < size[1]));
    return onGrid;
  }

  template 
  void AWVisResampler::addTo4DArray(DComplex* __restrict__ & store,
				    const Int* __restrict__ & iPos, 
				    const Vector<Int>& inc, 
  				    Complex& nvalue, Complex& wt) __restrict__ ;
  template 
  void AWVisResampler::addTo4DArray(Complex* __restrict__ & store,
				    const Int* __restrict__ & iPos, 
				    const Vector<Int>& inc, 
  				    Complex& nvalue, Complex& wt) __restrict__;
};// end namespace casa
