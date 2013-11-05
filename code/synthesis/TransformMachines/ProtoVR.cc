// -*- C++ -*-
//# ProtoVR.cc: Implementation of the ProtoVR class
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

#include <synthesis/TransformMachines/SynthesisError.h>
#include <synthesis/TransformMachines/cDataToGridImpl.h>
#include <synthesis/TransformMachines/ProtoVR.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/MSVis/UtilJ.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <casa/OS/Timer.h>
#include <fstream>
#include <iostream>
#include <typeinfo>
#include <iomanip>
#include <synthesis/TransformMachines/FortranizedLoops.h>
#ifdef HAS_OMP
#include <omp.h>
#endif
//#include <casa/BasicMath/Functors.h>
namespace casa{

  //
  //-----------------------------------------------------------------------------------
  // Re-sample the griddedData on the VisBuffer (a.k.a gridding)
  //
  // Template instantiations for re-sampling onto a double precision
  // or single precision grid.
  //
  // void ProtoVR::copy(const VisibilityResamplerBase& other)
  // {
  //   other.copy(other
  //   SynthesisUtils::SETVEC(uvwScale_p, other.uvwScale_p);
  //   SynthesisUtils::SETVEC(offset_p, other.offset_p);
  //   SynthesisUtils::SETVEC(dphase_p, other.dphase_p);
  //   SynthesisUtils::SETVEC(chanMap_p, other.chanMap_p);
  //   SynthesisUtils::SETVEC(polMap_p, other.polMap_p);
  //   SynthesisUtils::SETVEC(spwChanFreq_p, other.spwChanFreq_p);
  //   SynthesisUtils::SETVEC(spwChanConjFreq_p, other.spwChanConjFreq_p);
  //   SynthesisUtils::SETVEC(cfMap_p, other.cfMap_p);
  //   SynthesisUtils::SETVEC(conjCFMap_p, other.conjCFMap_p);
  //   //    vbRow2CFMap_p.assign(other.vbRow2CFMap_p);
  //   convFuncStore_p = other.convFuncStore_p;
  // }

  template 
  void ProtoVR::addTo4DArray(DComplex *store,
				      const Int*  iPos, 
				      const Int* inc, 
				      Complex& nvalue, Complex& wt);
  template 
  void ProtoVR::addTo4DArray(Complex* store,
				      const Int* iPos, 
				      const Int* inc, 
				      Complex& nvalue, Complex& wt);
  template
  void ProtoVR::DataToGridImpl_p(DComplex* gridStore,  Int* gridShape /*4-elements*/,
				 VBStore& vbs, Matrix<Double>& sumwt, const Bool& dopsf,
				 Int XThGrid, Int YThGrid
			  // Int& rowBegin, Int& rowEnd,
			  // Int& startChan, Int& endChan,
			  // Int& nDataPol, Int& nDataChan,
			  // Int& vbSpw,
			  // const Bool accumCFs
					  );
  template
  void ProtoVR::DataToGridImpl_p(Complex* gridStore,  Int* gridShape /*4-elements*/,
				 VBStore& vbs,  Matrix<Double>& sumwt, const Bool& dopsf,
				 Int XThGrid, Int YThGrid
			  // Int& rowBegin, Int& rowEnd,
			  // Int& startChan, Int& endChan,
			  // Int& nDataPol, Int& nDataChan,
			  // Int& vbSpw,
			  // const Bool accumCFs
					  );

  // template 
  // void ProtoVR::DataToGrid(Array<DComplex>& griddedData, VBStore& vbs, Matrix<Double>& sumwt,
  // 		  const Bool& dopsf,Bool useConjFreqCF=False);
  // template 
  // void ProtoVR::DataToGrid(Array<Complex>& griddedData, VBStore& vbs, Matrix<Double>& sumwt,
  // 		  const Bool& dopsf,Bool useConjFreqCF=False);

  template
  Complex ProtoVR::accumulateOnGrid(Complex* gridStore,
				    const Int* gridInc_p,
				    const Complex *cached_phaseGrad_p,
				    const Int cachedPhaseGradNX, const Int cachedPhaseGradNY,
				    const Complex* convFuncV, 
				    const Int *cfInc_p,
				    Complex nvalue,Double wVal, 
				    Int *supBLC_ptr, Int *supTRC_ptr,//Int* scaledSupport_ptr, 
				    Float* scaledSampling_ptr, 
				    Double* off_ptr, Int* convOrigin_ptr, 
				    Int* cfShape, Int* loc_ptr, Int* iGrdpos_ptr,
				    Bool finitePointingOffset,
				    Bool doPSFOnly, Bool& foundCFPeak);
  template
  Complex ProtoVR::accumulateOnGrid(DComplex* gridStore,
				    const Int* gridInc_p,
				    const Complex *cached_phaseGrad_p,
				    const Int cachedPhaseGradNX, const Int cachedPhaseGradNY,
				    const Complex* convFuncV, 
				    const Int *cfInc_p,
				    Complex nvalue,Double wVal, 
				    Int *supBLC_ptr, Int *supTRC_ptr,//Int* scaledSupport_ptr, 
				    Float* scaledSampling_ptr, 
				    Double* off_ptr, Int* convOrigin_ptr, 
				    Int* cfShape, Int* loc_ptr, Int* iGrdpos_ptr,
				    Bool finitePointingOffset,
				    Bool doPSFOnly, Bool& foundCFPeak);
  template
  void ProtoVR::cudaDataToGridImpl_p(Array<Complex>& griddedData, 
				     VBStore& vbs, Matrix<Double>& sumwt,
				     const Bool dopsf,
				     const Int* polMap_ptr, const Int *chanMap_ptr,
				     const Double *uvwScale_ptr, const Double *offset_ptr,
				     const Double *dphase_ptr, Int XThGrid, Int YThGrid);
  template 
  void ProtoVR::cudaDataToGridImpl_p(Array<DComplex>& griddedData, 
				     VBStore& vbs, Matrix<Double>& sumwt,
				     const Bool dopsf,
				     const Int* polMap_ptr, const Int *chanMap_ptr,
				     const Double *uvwScale_ptr, const Double *offset_ptr,
				     const Double *dphase_ptr, Int XThGrid, Int YThGrid);
  // template
  // void ProtoVR::accumulateFromGrid(Complex& nvalue, const DComplex* __restrict__& grid, 
  // 					  Vector<Int>& iGrdPos,
  // 					  Complex* __restrict__& convFuncV, 
  // 					  Double& wVal, Vector<Int>& scaledSupport, 
  // 					  Vector<Float>& scaledSampling, Vector<Double>& off,
  // 					  Vector<Int>& convOrigin, Vector<Int>& cfShape,
  // 					  Vector<Int>& loc, 
  // 					  Complex& phasor, 
  // 					  Double& sinDPA, Double& cosDPA,
  // 					  Bool& finitePointingOffset, 
  // 					  Matrix<Complex>& cached_phaseGrad_p,
  // 					  Bool dopsf);
  // template
  // void ProtoVR::accumulateFromGrid(Complex& nvalue, const Complex* __restrict__&  grid, 
  // 					  Vector<Int>& iGrdPos,
  // 					  Complex* __restrict__& convFuncV, 
  // 					  Double& wVal, Vector<Int>& scaledSupport, 
  // 					  Vector<Float>& scaledSampling, Vector<Double>& off,
  // 					  Vector<Int>& convOrigin, Vector<Int>& cfShape,
  // 					  Vector<Int>& loc, 
  // 					  Complex& phasor, 
  // 					  Double& sinDPA, Double& cosDPA,
  // 					  Bool& finitePointingOffset, 
  // 					  Matrix<Complex>& cached_phaseGrad_p);
  
  template 
  void ProtoVR::XInnerLoop(const Int *scaledSupport, const Float* scaledSampling,
  				  const Double* off,
  				  const Int* loc, Complex& cfArea,  
  				  const Int * __restrict__ iGrdPosPtr,
  				  Complex *__restrict__& convFuncV,
				  const Int* convOrigin,
  				  Complex& nvalue,
				  Double& wVal,
  				  Bool& finitePointingOffset,
  				  Bool& doPSFOnly,
  				  Complex* __restrict__ gridStore,
  				  Int* iloc,
  				  Complex& norm,
  				  Int* igrdpos);
  template 
  void ProtoVR::XInnerLoop(const Int *scaledSupport, const Float* scaledSampling,
  				  const Double* off,
  				  const Int* loc, Complex& cfArea,  
  				  const Int * __restrict__ iGrdPosPtr,
  				  Complex *__restrict__& convFuncV,
				  const Int* convOrigin,
				    Complex& nvalue,
				    Double& wVal,
				    Bool& finitePointingOffset,
				    Bool& doPSFOnly,
				    DComplex* __restrict__ gridStore,
				    Int* iloc,
				    Complex& norm,
				    Int* igrdpos);
  
  Complex* ProtoVR::getConvFunc_p(Int cfShape[4], VBStore& vbs,
					   Double& wVal, Int& fndx, Int& wndx,
					   Int **mNdx, Int  **conjMNdx,
					   Int& ipol, uInt& mRow)
  {
    Bool Dummy;
    Complex *tt;
    CFCStruct *tcfc;
    Int polNdx;
    Int shape[3];
    if (wVal > 0.0) polNdx=mNdx[ipol][mRow];
    else            polNdx=conjMNdx[ipol][mRow];

    tcfc=vbs.cfBSt_p.getCFB(fndx,wndx,polNdx);
    // shape[0]=(vbs.cfBSt_p.shape)[0];    shape[1]=(vbs.cfBSt_p.shape)[1];    shape[2]=(vbs.cfBSt_p.shape)[2];
    // tcfc=vbs.cfBSt_p.CFBStorage[fndx+wndx*shape[1]+polNdx*shape[2]];

    tt=tcfc->CFCStorage;
    cfShape[0]=tcfc->shape[0];
    cfShape[1]=tcfc->shape[1];

    // convFuncV = &(*cfcell->getStorage());
    // Complex *tt=convFuncV->getStorage(Dummy);
    
    //    cfShape.reference(cfcell->cfShape_p);
    
    return tt;
  };
  
  template <class T>
  void ProtoVR::XInnerLoop(const Int *scaledSupport, const Float* scaledSampling,
				    const Double* off,
				    const Int* loc,  Complex& cfArea,  
				    const Int * __restrict__ iGrdPosPtr,
				    Complex *__restrict__& convFuncV,
				    const Int* convOrigin,
				    Complex& nvalue,
				    Double& wVal,
				    Bool& finitePointingOffset,
				    Bool& doPSFOnly,
				    T* __restrict__ gridStore,
				    Int* iloc,
				    Complex& norm,
				    Int* igrdpos)
  {
    Complex wt;
    const Int *tt=iloc;
    Bool Dummy;
    for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++) 
      {
  	iloc[0]=(Int)((scaledSampling[0]*ix+off[0])-1)+convOrigin[0];
  	igrdpos[0]=loc[0]+ix;
	
  	{
  	  wt = getFrom4DArray((const Complex * __restrict__ &)convFuncV, 
  			      tt,cfInc_p)/cfArea;
  	  if (wVal > 0.0) {wt = conj(wt);}
	  norm += (wt);
	  // if (finitePointingOffset && !doPSFOnly) 
	  //   wt *= cached_phaseGrad_p(iloc[0]+phaseGradOrigin_l[0],
	  // 			       iloc[1]+phaseGradOrigin_l[1]);
	  
	  // The following uses raw index on the 4D grid
	  addTo4DArray(gridStore,iGrdPosPtr,gridInc_p, nvalue,wt);
  	}
      }
  }
  
  template <class T>
  Complex ProtoVR::accumulateOnGrid(T* gridStore,
				    const Int* gridInc_p,
				    const Complex *cached_phaseGrad_p,
				    const Int cachedPhaseGradNX, const Int cachedPhaseGradNY,
				    const Complex* convFuncV, 
				    const Int *cfInc_p,
				    Complex nvalue,Double wVal, 
				    Int *supBLC_ptr, Int *supTRC_ptr,//Int* scaledSupport_ptr, 
				    Float* scaledSampling_ptr, 
				    Double* off_ptr, Int* convOrigin_ptr, 
				    Int* cfShape, Int* loc_ptr, Int* iGrdpos_ptr,
				    Bool finitePointingOffset,
				    Bool doPSFOnly, Bool& foundCFPeak)
  {
    Int iloc_ptr[4]={0,0,0,0};//   for (int i=0;i<4;i++) iloc_ptr[i]=0;
    
    Complex wt, cfArea=1.0; 
    Complex norm=0.0;
    Int Nth = 1;
    
    Bool finitePointingOffset_l=finitePointingOffset;
    Bool doPSFOnly_l=doPSFOnly;
    Double wVal_l=wVal;
    Complex nvalue_l=nvalue;

    Int phaseGradOrigin_l[2]; 
    //    phaseGradOrigin_l = cached_phaseGrad_p.shape()/2;
    phaseGradOrigin_l[0] = cachedPhaseGradNX/2;
    phaseGradOrigin_l[1] = cachedPhaseGradNY/2;
    
    //    for(Int iy=-scaledSupport_ptr[1]; iy <= scaledSupport_ptr[1]; iy++) 
    for(Int iy=supBLC_ptr[1]; iy <= supTRC_ptr[1]; iy++) 
      {
	iloc_ptr[1]=(Int)((scaledSampling_ptr[1]*iy+off_ptr[1])-1)+convOrigin_ptr[1];
	iGrdpos_ptr[1]=loc_ptr[1]+iy;
	
	//	for(Int ix=-scaledSupport_ptr[0]; ix <= scaledSupport_ptr[0]; ix++) 
	for(Int ix=supBLC_ptr[0]; ix <= supTRC_ptr[0]; ix++) 
	  {
	    iloc_ptr[0]=(Int)((scaledSampling_ptr[0]*ix+off_ptr[0])-1)+convOrigin_ptr[0];
	    iGrdpos_ptr[0]=loc_ptr[0]+ix;
	    {
	      if (ix==0 and iy==0) foundCFPeak=True;
	      wt = getFrom4DArray((const Complex * __restrict__ &)convFuncV, 
				  iloc_ptr,cfInc_p)/cfArea;
	      if (wVal > 0.0) {wt = conj(wt);}
	      norm += (wt);
	      if (finitePointingOffset && !doPSFOnly) 
		wt *= cached_phaseGrad_p[iloc_ptr[0]+phaseGradOrigin_l[0]+
					 iloc_ptr[1]+phaseGradOrigin_l[1]*cachedPhaseGradNY];

	      // The following uses raw index on the 4D grid
	      addTo4DArray(gridStore,iGrdpos_ptr,gridInc_p, nvalue,wt);
	    }
	  }
      }
    return norm;
  }

void ProtoVR::cachePhaseGrad_g(Complex *cached_phaseGrad_p, Int phaseGradNX, Int phaseGradNY,
			       Double* cached_PointingOffset_p, Double* pointingOffset,
			       Int cfShape[4], Int convOrigin[4])
{
  if (
      ((fabs(pointingOffset[0]-cached_PointingOffset_p[0])) > 1e-6) ||
      ((fabs(pointingOffset[1]-cached_PointingOffset_p[1])) > 1e-6) ||
      (phaseGradNX < cfShape[0]) || (phaseGradNY < cfShape[1])
      )
      {
	cerr << "Computing phase gradiant for pointing offset " 
	     << "[" << pointingOffset[0] << "," << pointingOffset[1] << "] ["
	     << cfShape[0] << "," << cfShape[1] << "]" << endl;
	
	Int nx=cfShape[0], ny=cfShape[1];
	Double grad;
	Complex phx,phy;
	
	cerr << "Resize cached_phaseGrad_p !!!" << endl;
	//	cached_phaseGrad_p.resize(nx,ny);
	cached_PointingOffset_p[0] = pointingOffset[0];
	cached_PointingOffset_p[1] = pointingOffset[1];
	
	for(Int ix=0;ix<nx;ix++)
	  {
	    grad = (ix-convOrigin[0])*pointingOffset[0];
	    Double sx,cx;
	    sincos(grad,&sx,&cx);
	    //	    phx = Complex(cos(grad),sin(grad));
	    phx = Complex(cx,sx);
	    for(Int iy=0;iy<ny;iy++)
	      {
		grad = (iy-convOrigin[1])*pointingOffset[1];
		Double sy,cy;
		sincos(grad,&sy,&cy);
		//		phy = Complex(cos(grad),sin(grad));
		phy = Complex(cy,sy);
		cached_phaseGrad_p[ix+iy*phaseGradNY]=phx*phy;
	      }
	  }
      }
}


  void ProtoVR::DataToGrid(Array<DComplex>& griddedData, VBStore& vbs, Matrix<Double>& sumwt,
			   const Bool& dopsf,Bool /*useConjFreqCF*/)
    {
      // Complex tt;
      // Bool isGridSinglePrecision=(typeid(griddedData[0]) == typeid(tt));


      Bool Dummy;
      DComplex *gridStore=griddedData.getStorage(Dummy);
      Vector<Int> gridV=griddedData.shape().asVector();
      IPosition shp=vbs.BLCXi.shape();
      Int *gridShape = gridV.getStorage(Dummy),NBlocks=shp(0)*shp(1), k=0, threadID=0;
      Int Nth=1;

      Matrix<Int> gridCoords(NBlocks,2);
      for (Int i=0;i<shp(0);i++)
	for (Int j=0;j<shp(1);j++)
	{
	  gridCoords(k,0)=i;
	  gridCoords(k,1)=j;
	  k++;
	}
      Block<Matrix<Double> > sumWt_th(NBlocks);
      for (Int i=0; i < NBlocks; i++)
	{
	  sumWt_th[i].resize(sumwt.shape());
	  sumWt_th[i].set(0.0);
	}
      //      Timer timer;

#ifdef HAS_OMP
      Nth=min(max(1,NBlocks),omp_get_max_threads()-2);
#endif
      Nth=16;

//      timer.mark();
      //
      // Loop over all the grid partitions
      //

      Int *polMap_ptr=polMap_p.getStorage(Dummy),
	*chanMap_ptr = chanMap_p.getStorage(Dummy);
      Double *uvwScale_ptr=uvwScale_p.getStorage(Dummy), 
	*offset_ptr=offset_p.getStorage(Dummy), 
	*dphase_ptr=dphase_p.getStorage(Dummy);
      
      Bool sumWtBool;

#pragma omp parallel shared(gridCoords,polMap_ptr,chanMap_ptr, uvwScale_ptr, offset_ptr, dphase_ptr, sumWt_th) private(sumWtBool) num_threads(Nth)
      {
	//	Matrix<Double> tmpsumwt(sumwt.shape());
#pragma omp for nowait schedule(static,16)
      for (Int i=0;i<NBlocks;i++)
	{
	  //tmpsumwt=0.0;

	  //DataToGridImpl_p(gridStore, gridShape, vbs, tmpSumWt,dopsf,i,j);

	  cDataToGridImpl_p(gridStore, gridShape, &vbs, &(sumWt_th[i]), dopsf, 
			    polMap_ptr, chanMap_ptr, uvwScale_ptr, offset_ptr,
			    dphase_ptr, gridCoords(i,0), gridCoords(i,1));
	  // if (isGridSinglePrecision)
	  //   complexGridder_ptr((Complex *)gridStore, gridShape, &vbs, &tmpSumWt, dopsf, 
	  // 			 polMap_ptr, chanMap_ptr, uvwScale_ptr, offset_ptr,
	  // 			 dphase_ptr, gridCoords(i,0), gridCoords(i,1));
	  // else

	  // dcomplexGridder_ptr((DComplex *)gridStore, gridShape, &vbs, &sumwt, dopsf, 
	  // 			  polMap_ptr, chanMap_ptr, uvwScale_ptr, offset_ptr,
	  // 			  dphase_ptr, gridCoords(i,0), gridCoords(i,1));
			    
	  //	    DataToGridImpl_p(gridStore, gridShape, vbs, tmpSumWt,dopsf,gridCoords(i,0), gridCoords(i,1));

// #ifdef HAS_OMP
// 	    threadID=omp_get_thread_num();
// #endif
	  //cerr << "Thread ID(DC) = " << threadID << " SumWT = " << tmpSumWt << " " << sumwt << endl;
	  //sumwt += tmpsumwt;
	}
      }
      for (Int i=0; i < NBlocks; i++) sumwt=sumwt+sumWt_th[i];
      //      cerr << "Timer: " << timer.all() << endl;
    }

  void ProtoVR::DataToGrid(Array<Complex>& griddedData, VBStore& vbs, Matrix<Double>& sumwt,
			   const Bool& dopsf,Bool /* useConjFreqCF*/)
    {
      Bool Dummy;
      Complex *gridStore=griddedData.getStorage(Dummy);
      Vector<Int> gridV=griddedData.shape().asVector();
      IPosition shp=vbs.BLCXi.shape();
      Int *gridShape = gridV.getStorage(Dummy),NBlocks=shp(0)*shp(1), k=0, threadID=0;
      Int Nth=1;
      Matrix<Int> gridCoords(NBlocks,2);
      for (Int i=0;i<shp(0);i++)
	for (Int j=0;j<shp(1);j++)
	{
	  gridCoords(k,0)=i;
	  gridCoords(k,1)=j;
	  k++;
	}
      Block<Matrix<Double> > sumWt_th(NBlocks);
      for (Int i=0; i < NBlocks; i++)
	{
	  sumWt_th[i].resize(sumwt.shape());
	  sumWt_th[i].set(0.0);
	}
      //      Timer timer;

#ifdef HAS_OMP
      Nth=min(max(1,NBlocks),omp_get_max_threads()-2);
#endif
      Nth=16;

//      timer.mark();
      //
      // Loop over all the grid partitions
      //

      Int *polMap_ptr=polMap_p.getStorage(Dummy),
	*chanMap_ptr = chanMap_p.getStorage(Dummy);
      Double *uvwScale_ptr=uvwScale_p.getStorage(Dummy), 
	*offset_ptr=offset_p.getStorage(Dummy), 
	*dphase_ptr=dphase_p.getStorage(Dummy);
      
      Bool sumWtBool;

#pragma omp parallel shared(gridCoords,polMap_ptr,chanMap_ptr, uvwScale_ptr, offset_ptr, dphase_ptr, sumWt_th) private(sumWtBool) num_threads(Nth)
      {
	//Matrix<Double> tmpSumWt(sumwt.shape());
#pragma omp for nowait schedule(static,16)
      for (Int i=0;i<NBlocks;i++)
	  {
	    //tmpSumWt=0.0;
	    //DataToGridImpl_p(gridStore, gridShape, vbs, tmpSumWt,dopsf,i,j);

	    cDataToGridImpl_p(gridStore, gridShape, &vbs, &sumWt_th[i], dopsf, 
	    		      polMap_ptr, chanMap_ptr, uvwScale_ptr, offset_ptr,
	    		      dphase_ptr, gridCoords(i,0), gridCoords(i,1));
	    // complexGridder_ptr(gridStore, gridShape, &vbs, &tmpSumWt, dopsf, 
	    // 		      polMap_ptr, chanMap_ptr, uvwScale_ptr, offset_ptr,
	    // 		      dphase_ptr, gridCoords(i,0), gridCoords(i,1));

	    //	    DataToGridImpl_p(gridStore, gridShape, vbs, tmpSumWt,dopsf,gridCoords(i,0), gridCoords(i,1));

// #ifdef HAS_OMP
// 	    threadID=omp_get_thread_num();
// #endif
	    //cerr << "Thread ID(C) = " << threadID << " SumWT = " << tmpSumWt << " " << sumwt << endl;
	    //sumwt += tmpSumWt;
	  }
      }
      for (Int i=0; i < NBlocks; i++) sumwt=sumwt+sumWt_th[i];
      //      cerr << "Timer: " << timer.all() << endl;
    }

  Bool ProtoVR::computeSupport(const VBStore& vbs, const Int& XThGrid, const Int& YThGrid,
			       const Int support[2], const Float sampling[2],
			       const Double pos[2], const Int loc[3],
			       Float iblc[2], Float itrc[2])
  {
    //    Int sup[2] = {support[0]*sampling[0], support[1]*sampling[1]};
    Int sup[2] = {support[0], support[1]};
    Int blc[2] = {vbs.BLCXi(XThGrid, YThGrid), vbs.BLCYi(XThGrid, YThGrid)};
    Int trc[2] = {vbs.TRCXi(XThGrid, YThGrid), vbs.TRCYi(XThGrid, YThGrid)};

    Float vblc[2]={pos[0]-sup[0],pos[1]-sup[1]}, vtrc[2]={pos[0]+sup[0],pos[1]+sup[1]};
    if (SynthesisUtils::checkIntersection(blc,trc,vblc,vtrc))
      {
	SynthesisUtils::calcIntersection(blc,trc,vblc,vtrc,iblc,itrc);
	return True;
      }
    return False;
  }

template <class T>
void ProtoVR::DataToGridImpl_p(T* gridStore,  Int* gridShape /*4-elements*/,
			       VBStore& vbs, Matrix<Double>& sumwt, const Bool& dopsf,
			       Int XThGrid, Int YThGrid)
{
  LogIO log_l(LogOrigin("ProtoVR[R&D]","DataToGridImpl_p"));

  Int nGridPol, nGridChan, nx, ny, nw, nCFFreq;
  Int targetIMChan, targetIMPol, rbeg, rend;
  Int startChan, endChan;
  Bool accumCFs;

  Float sampling[2],scaledSampling[2];
  Int support[2],loc[3], iloc[4],tiloc[4],scaledSupport[2];
  Double pos[2], off[3];
  Int igrdpos[4];
  
  Complex phasor=Complex(1.0,0.0), nvalue, wt;
  Complex norm;
  Int cfShape[4];
  Bool Dummy;
  Bool * flagCube_ptr=vbs.flagCube_p.getStorage(Dummy);
  Bool * rowFlag_ptr = vbs.rowFlag_p.getStorage(Dummy);
  Float * imgWts_ptr = vbs.imagingWeight_p.getStorage(Dummy);
  Complex * visCube_ptr = vbs.visCube_p.getStorage(Dummy);
  Double *sumWt_ptr=sumwt.getStorage(Dummy);

  //  Vector<Double> pointingOffset(cfb.getPointingOffset());
  Double *pointingOffset_ptr=vbs.cfBSt_p.pointingOffset,
    *cached_PointingOffset_ptr=cached_PointingOffset_p.getStorage(Dummy);

  Int vbSpw=vbs.spwID_p;
    

  for (Int ii=0;ii<4;ii++)
    cfShape[ii]=vbRow2CFBMap_p(0)->getStorage()(0,0,0)->getStorage()->shape()(ii);
  Int convOrigin[4]; 
  convOrigin[0]= (cfShape[0])/2;
  convOrigin[1]= (cfShape[1])/2;
  convOrigin[2]= (cfShape[2])/2;
  convOrigin[3]= (cfShape[3])/2;
  
  // rbeg = rowBegin;
  // rend = rowEnd;
  rbeg = vbs.beginRow_p;
  rend = vbs.endRow_p;
  
  nx=gridShape[0]; ny=gridShape[1];
  nGridPol=gridShape[2]; nGridChan=gridShape[3];
  Bool gDummy;
  
  Double *freq=vbs.freq_p.getStorage(Dummy);
  
  cacheAxisIncrements(gridShape, gridInc_p);

  // cerr << "Gridshape = " << gridShape[0] << " " << gridShape[1] << " " << gridShape[2] << " " << gridShape[3] << " "
  //      << gridInc_p[0] << " " << gridInc_p[1] << " " << gridInc_p[2] << " " << gridInc_p[3] << " " << endl;

  nCFFreq = vbs.cfBSt_p.shape[0]; // shape[0]: nChan, shape[1]: nW, shape[2]: nPol
  nw = vbs.cfBSt_p.shape[1];

  iloc[0]=iloc[1]=iloc[2]=iloc[3]=0;
  Int nDataChan=vbs.nDataChan_p,
    nDataPol = vbs.nDataPol_p;
  accumCFs=vbs.accumCFs_p;
  if (accumCFs)
    {
      startChan = vbs.startChan_p;
      endChan = vbs.endChan_p;
    }
  else 
    {
      startChan = 0;
      endChan = vbs.nDataChan_p;
    }


  //  cerr << "ProtoVR: " << rbeg << " " << rend << " " << startChan << " " << endChan << " " << nDataChan << " " << nDataPol << endl;
  
  Bool finitePointingOffsets= (
			      (fabs(pointingOffset_ptr[0])>0) ||  
			      (fabs(pointingOffset_ptr[1])>0)
			      );
  for(Int irow=rbeg; irow< rend; irow++)
    {   
      if(!(*(rowFlag_ptr+irow)))
	{   
	  for(Int ichan=startChan; ichan< endChan; ichan++)
	    {
	      if (*(imgWts_ptr + ichan+irow*nDataChan)!=0.0) 
		{  
		  targetIMChan=chanMap_p[ichan];
		  
		  if((targetIMChan>=0) && (targetIMChan<nGridChan)) 
		    {
		      Double dataWVal = vbs.vb_p->uvw()(irow)(2);
		      
		      Int wndx = (int)(sqrt(vbs.cfBSt_p.wIncr*abs(dataWVal*freq[ichan]/C::c)));
		      
		      Int cfFreqNdx;
		      if (vbs.conjBeams_p) cfFreqNdx = vbs.cfBSt_p.conjFreqNdxMap[vbSpw][ichan];
		      else cfFreqNdx = vbs.cfBSt_p.freqNdxMap[vbSpw][ichan];
		      
		      Float s;
		      s=vbs.cfBSt_p.CFBStorage->sampling;
		      support[0]=vbs.cfBSt_p.CFBStorage->xSupport;
		      support[1]=vbs.cfBSt_p.CFBStorage->ySupport;
		      
		      sampling[0] = sampling[1] = SynthesisUtils::nint(s);
		      
		      const Double *uvw_ptr=vbs.uvw_p.getStorage(Dummy),
			*uvwScale_ptr=uvwScale_p.getStorage(Dummy),
			*offset_ptr=offset_p.getStorage(Dummy);;
		      
		      sgrid(pos,loc,off, phasor, irow, vbs.uvw_p, dphase_p[irow], freq[ichan], 
			    uvwScale_ptr, offset_ptr, sampling);
		      
		      Float cfblc[2], cftrc[2];
		      //		    pos[0]=1024.1;pos[1]=1025.6;
		      Bool onMyGrid=
			computeSupport(vbs, XThGrid, YThGrid, support, sampling, pos, loc,cfblc,cftrc);
		      // if (inMyGrid)
		      //   cerr << SynthesisUtils::nint((iblc[0]-loc[0])/sampling[0]) << " " 
		      // 	   << SynthesisUtils::nint((iblc[1]-loc[1])/sampling[1]) << " " 
		      // 	   << SynthesisUtils::nint((itrc[0]-loc[0])/sampling[0]) << " " 
		      // 	   << SynthesisUtils::nint((itrc[1]-loc[1])/sampling[1]) 
		      // 	   << endl;
		      
		      // 		    if (onGrid(nx, ny, nw, loc, support)) 
		      if (onMyGrid)
			{
			  
			  Int iblc[2], itrc[2];
			  
			  iblc[0]=SynthesisUtils::nint((cfblc[0]-pos[0]));///sampling[0]);
			  iblc[1]=SynthesisUtils::nint((cfblc[1]-pos[1]));///sampling[1]);
			  itrc[0]=SynthesisUtils::nint((cftrc[0]-pos[0]));///sampling[0]);
			  itrc[1]=SynthesisUtils::nint((cftrc[1]-pos[1]));///sampling[1]); 
			  
			  Int dx=abs(itrc[0]-iblc[0])+1, dy=abs(itrc[1]-iblc[1])+1;
			  Float cfFractioanlArea = (dx*dy)/(float)square(abs(support[1]+support[0])+1);
			  
			  // if ((irow < 3) && (ichan == 2))
			  //   {
			  //     // cerr << irow << "#[" << XThGrid << "," << YThGrid << "] " 
			  //     // 	 << (cfblc[0]) << " " << (cftrc[0]) << " " 
			  //     // 	 << (cfblc[1]) << " " << (cftrc[1]) << " " 
			  //     // 	 << dx << " " << dy << " " << (dx*dy) << " " << cfFractionalArea
			  //     // 	 << endl;
			  //     // cerr << irow << "#[" << XThGrid << "," << YThGrid << "] " 
			  //     // 	 << (pos[0]) << " " << (pos[1]) << endl;
			      
			  //     cerr << irow << " [" << XThGrid << "," << YThGrid << "] " 
			  // 	   << iblc[0] << " " << itrc[0] << " " 
			  // 	   << iblc[1] << " " << itrc[1] << " " << rend
			  // 	   << endl;
			  //   }
			  
			  // Loop over all image-plane polarization planes.
			  for(Int ipol=0; ipol< nDataPol; ipol++) 
			    { 
			      if((!(*(flagCube_ptr + ipol + ichan*nDataPol + irow*nDataPol*nDataChan))))
				{  
				  targetIMPol=polMap_p(ipol);
				  if ((targetIMPol>=0) && (targetIMPol<nGridPol)) 
				    {
				      igrdpos[2]=targetIMPol; igrdpos[3]=targetIMChan;
				      
				      // if(accumCFs)     allPolNChanDone_l(ipol,ichan,irow)=True;
				      if(dopsf) nvalue=Complex(*(imgWts_ptr + ichan + irow*nDataChan));
				      else      nvalue= *(imgWts_ptr+ichan+irow*nDataChan)*
						  (*(visCube_ptr+ipol+ichan*nDataPol+irow*nDataChan*nDataPol)*phasor);
				      
				      norm = 0.0;
				      // for (uInt mRow=0;mRow<conjMNdx[ipol].nelements(); mRow++) 
				      // for (uInt mRow=0;mRow<vbs.cfBSt_p.conjMuellerElementsIndex[ipol].nelements(); mRow++) 
				      Bool foundCFPeak=False;
				      for (uInt mRow=0;mRow<vbs.cfBSt_p.nMueller; mRow++) 
					{
					  Complex* convFuncV;
					  // CUWORK:  Essentially CFC.getCellPtr(FNDX, WNDX, POLNDX)
					  // CUWORK: CFC wrapper
					  convFuncV=getConvFunc_p(cfShape, vbs, dataWVal, cfFreqNdx, wndx, 
								  vbs.cfBSt_p.muellerElementsIndex,
								  vbs.cfBSt_p.conjMuellerElementsIndex, ipol,  mRow);
					  
					  convOrigin[0]=cfShape[0]/2;
					  convOrigin[1]=cfShape[1]/2;
					  convOrigin[2]=cfShape[2]/2;
					  convOrigin[3]=cfShape[3]/2;
					  Bool psfOnly=((dopsf==True) && (accumCFs==False));
					  // // CUWORK: Convert to a global function with native types
					  Int cachedPhaseGradNX=cached_phaseGrad_p.shape()[0],
					    cachedPhaseGradNY=cached_phaseGrad_p.shape()[1];
					  Complex *cached_PhaseGrad_ptr=cached_phaseGrad_p.getStorage(Dummy);
					  
					  if (finitePointingOffsets && !psfOnly)
					    cachePhaseGrad_g(cached_PhaseGrad_ptr, cachedPhaseGradNX, cachedPhaseGradNY,	
							     cached_PointingOffset_ptr, pointingOffset_ptr, cfShape, convOrigin);//, cfRefFreq);//, vbs.imRefFreq());
					  
					  cacheAxisIncrements(cfShape, cfInc_p);
					  //cerr << gridShape[0] << " " << gridShape[1] << " " << gridInc_p[0] << " " << gridInc_p[0] << endl;
					  norm += accumulateOnGrid(gridStore, gridInc_p, cached_PhaseGrad_ptr, 
								   cachedPhaseGradNX, cachedPhaseGradNY,
								   convFuncV, cfInc_p, nvalue,dataWVal,
								   iblc,itrc,/*support,*/ sampling, off, 
								   convOrigin, cfShape, loc, igrdpos,
								   finitePointingOffsets,psfOnly,foundCFPeak);
					}
				      
				      //sumwt(targetIMPol,targetIMChan) += vbs.imagingWeight_p(ichan, irow);//*abs(norm);
				      //cerr << sumwt << " " << targetIMPol << " " << targetIMChan << " " << vbs.imagingWeight_p(ichan, irow) << " " << abs(norm) << endl;
				      // Int dx=abs(itrc[0]-iblc[0]+1), dy=abs(itrc[1]-iblc[1]+1);
				      // Float cfPixArea = (float)square(abs(support[0]-support[1]+1));
				      
				      // Accumulate data weight only when the CF peak was used. This can also be done via
				      // fractional area of the CF used, and probably should be done that for high accuracy 
				      // using the norm of the CF.
				      //if (foundCFPeak) 
				      *(sumWt_ptr+targetIMPol+targetIMChan*nGridPol)+= *(imgWts_ptr+ichan+irow*nDataChan)*abs(norm);
				    }
				}
			    } // End poln-loop
			}
		    }
		}
	    } // End chan-loop
	}
    } // End row-loop
  //exit(0);
}

//
//-----------------------------------------------------------------------------------
// Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
//
void ProtoVR::GridToData(VBStore& vbs, const Array<Complex>& grid)
{
}
//
//-----------------------------------------------------------------------------------
//
void ProtoVR::sgrid(Double pos[2], Int loc[3], 
		    Double off[3], Complex& phasor, 
		    const Int& irow, const Matrix<Double>& uvw, 
		    const Double& dphase, const Double& freq, 
		    const Double* scale, 
		    const Double* offset,
		    const Float sampling[2])
{
  Double phase;
  //Vector<Double> uvw_l(3,0); // This allows gridding of weights
  Double uvw_l[3]={0.0,0.0,0.0}; // This allows gridding of weights
  Bool dd;
  const Double *uvw_ptr=uvw.getStorage(dd);
  // centered on the uv-origin
  //  if (uvw.nelements() > 0) for(Int i=0;i<3;i++) uvw_l[i]=uvw(i,irow);
  if (uvw.nelements() > 0) for(Int i=0;i<3;i++) uvw_l[i]=uvw_ptr[i+irow*3];
  
  pos[2]=sqrt(abs(scale[2]*uvw_l[2]*freq/C::c))+offset[2];
  loc[2]=SynthesisUtils::nint(pos[2]);
  pos[2]=0.0;loc[2]=0;
  off[2]=0.0;
  
  for(Int idim=0;idim<2;idim++)
    {
      pos[idim]=scale[idim]*uvw_l[idim]*freq/C::c+(offset[idim]);
      loc[idim]=SynthesisUtils::nint(pos[idim]);
      //	off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]+1);
      off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]);
    }
  
  // if (dphase != 0.0)
  //   {
  //     phase=-2.0*C::pi*dphase*freq/C::c;
  //     Double sp,cp;
  //     sincos(phase,&sp,&cp);
  //     //      phasor=Complex(cos(phase), sin(phase));
  //     phasor=Complex(cp,sp);
  //   }
  // else
  //   phasor=Complex(1.0);
  // cerr << "### " << pos[0] << " " << offset[0] << " " << loc[0] << " " << off[0] << " " << uvw_l[0] << endl;
  // exit(0);
}
//
//-----------------------------------------------------------------------------------
//
Bool ProtoVR::reindex(const Vector<Int>& in, Vector<Int>& out,
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
  if (!onGrid)
    cerr << "CF index out of range: " << out << " " << size << endl;
  return onGrid;
}


// void lineCFArea(const Int& th,
// 		  const Double& sinDPA,
// 		  const Double& cosDPA,
// 		  const Complex*__restrict__& convFuncV,
// 		  const Vector<Int>& cfShape,
// 		  const Vector<Int>& convOrigin,
// 		  const Int& cfInc,
// 		  Vector<Int>& iloc,
// 		  Vector<Int>& tiloc,
// 		  const Int* supportPtr,
// 		  const Float* samplingPtr,
// 		  const Double* offPtr,
// 		  Complex *cfAreaArrPtr)
// {
//   cfAreaArrPtr[th]=0.0;
//   for(Int ix=-supportPtr[0]; ix <= supportPtr[0]; ix++) 
//     {
// 	iloc[0]=(Int)((samplingPtr[0]*ix+offPtr[0])-1);//+convOrigin[0];
// 	tiloc=iloc;
// 	if (reindex(iloc,tiloc,sinDPA, cosDPA, 
// 		    convOrigin, cfShape))
// 	  {
// 	    wt = getFrom4DArray((const Complex * __restrict__ &)convFuncV, 
// 				tiloc,cfInc);
// 	    if (dataWVal > 0.0) wt = conj(wt);
// 	    cfAreaArrPtr[th] += wt;
// 	  }
//     }
// }

Complex ProtoVR::getCFArea(Complex* __restrict__& convFuncV, 
			   Double& wVal, 
			   Vector<Int>& scaledSupport, 
			   Vector<Float>& scaledSampling,
			   Vector<Double>& off,
			   Vector<Int>& convOrigin, 
			   Vector<Int>& cfShape,
			   Double& sinDPA, 
			   Double& cosDPA)
{
  Vector<Int> iloc(4,0),tiloc(4);
  Complex cfArea=0, wt;
  Bool dummy;
  Int *supportPtr=scaledSupport.getStorage(dummy);
  Double *offPtr=off.getStorage(dummy);
  Float *samplingPtr=scaledSampling.getStorage(dummy);
  Int Nth=1;
  Vector<Complex> cfAreaArr(Nth);
  Complex *cfAreaArrPtr=cfAreaArr.getStorage(dummy);
  
  for(Int iy=-supportPtr[1]; iy <= supportPtr[1]; iy++) 
    {
      iloc(1)=(Int)((samplingPtr[1]*iy+offPtr[1])-1);//+convOrigin[1];
      for (Int th=0;th<Nth;th++)
	{
	  cfAreaArr[th]=0.0;
	  for(Int ix=-supportPtr[0]; ix <= supportPtr[0]; ix++) 
	    {
	      iloc[0]=(Int)((samplingPtr[0]*ix+offPtr[0])-1);//+convOrigin[0];
	      tiloc=iloc;
	      if (reindex(iloc,tiloc,sinDPA, cosDPA, 
			  convOrigin, cfShape))
		{
		  Bool dummy;
		  Int *tiloc_ptr=tiloc.getStorage(dummy);
		  wt = getFrom4DArray(convFuncV, tiloc_ptr,cfInc_p);
		  if (wVal > 0.0) wt = conj(wt);
		  cfAreaArrPtr[th] += wt;
		}
	    }
	}
      cfArea += sum(cfAreaArr);
    }
  //    cerr << "cfArea: " << scaledSupport << " " << scaledSampling << " " << cfShape << " " << convOrigin << " " << cfArea << endl;
  return cfArea;
}

void ProtoVR::initializeDataBuffers(VBStore& vbs)
{
  LogIO log_l(LogOrigin("ProtoVR[R&D]","initializeDataBuffers"));
  // log_l << "********Send data buffers to device***********" << LogIO::WARN;
  
  //
  // Symmetrize the VB for better load-balanced computing
  //
  Int N=vbs.nRow()/2;
  IPosition shp(vbs.visCube_p.shape());
  
  for (Int irow=N;irow<vbs.nRow();irow++)
    {
      vbs.uvw_p(0,irow)=-vbs.uvw_p(0,irow);
      vbs.uvw_p(1,irow)=-vbs.uvw_p(1,irow);
      vbs.uvw_p(2,irow)=-vbs.uvw_p(2,irow);
      
      for (Int ichan=0;ichan<shp(1);ichan++)
	for (Int ipol=0;ipol<shp(0);ipol++)
	  {
	    vbs.visCube_p(ipol,ichan,irow) = conj(vbs.visCube_p(ipol,ichan,irow));
	  }
    }
  // for (Int irow=0;irow<vbs.nRow();irow++)
  //   {
  //     cerr << irow << " " << vbs.uvw_p(0,irow) << " " << vbs.uvw_p(1,irow) << " " << vbs.uvw_p(2,irow) << endl;
  //   }
  // exit(0);
}
template <class T>
void ProtoVR::cudaDataToGridImpl_p(Array<T>& griddedData,
				   VBStore& vbs, Matrix<Double>& sumwt,
				   const Bool dopsf,
				   const Int* polMap_ptr, const Int *chanMap_ptr,
				   const Double *uvwScale_ptr, const Double *offset_ptr,
				   const Double *dphase_ptr, Int XThGrid, Int YThGrid)
{
}

void ProtoVR::GatherGrids(Array<DComplex>& griddedData, Matrix<Double>& sumwt) {cerr << "Empty GatherGrids(DComplex)" << endl;};
void ProtoVR::GatherGrids(Array<Complex>& griddedData, Matrix<Double>& sumwt) {cerr << "Empy GatherGrids(Complex)" << endl;};
};// end namespace casa
