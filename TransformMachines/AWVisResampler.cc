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

#include <synthesis/TransformMachines/SynthesisError.h>
#include <synthesis/TransformMachines/AWVisResampler.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/SynthesisMath.h>
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
  template
  void AWVisResampler::DataToGridImpl_p(Array<DComplex>& grid, VBStore& vbs, 
  					Matrix<Double>& sumwt,const Bool& dopsf,
  					Bool useConjFreqCF); // __restrict__;
  template
  void AWVisResampler::DataToGridImpl_p(Array<Complex>& grid, VBStore& vbs, 
					Matrix<Double>& sumwt,const Bool& dopsf,
					Bool useConjFreqCF); // __restrict__;

  template
  Complex AWVisResampler::accumulateOnGrid(Array<DComplex>& grid, Complex* __restrict__& convFuncV, 
					  Complex& nvalue, Double& wVal, 
					  Vector<Int>& scaledSupport, Vector<Float>& scaledSampling, 
					  Vector<Double>& off, Vector<Int>& convOrigin, 
					  Vector<Int>& /*cfShape*/, Vector<Int>& loc, Vector<Int>& igrdpos,
					   Double& /*sinDPA*/, Double& /*cosDPA*/,
					  Bool& pointingOffset, Bool dopsf);
  template
  Complex AWVisResampler::accumulateOnGrid(Array<Complex>& grid, Complex* __restrict__& convFuncV, 
					  Complex& nvalue, Double& wVal, 
					  Vector<Int>& scaledSupport, Vector<Float>& scaledSampling, 
					  Vector<Double>& off, Vector<Int>& convOrigin, 
					  Vector<Int>& /*cfShape*/, Vector<Int>& loc, Vector<Int>& igrdpos,
					   Double& /*sinDPA*/, Double& /*cosDPA*/,
					  Bool& pointingOffset, Bool dopsf);
  // template
  // void AWVisResampler::accumulateFromGrid(Complex& nvalue, const DComplex* __restrict__& grid, 
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
  template
  void AWVisResampler::accumulateFromGrid(Complex& nvalue, const Complex* __restrict__&  grid, 
					  Vector<Int>& iGrdPos,
					  Complex* __restrict__& convFuncV, 
					  Double& wVal, Vector<Int>& scaledSupport, 
					  Vector<Float>& scaledSampling, Vector<Double>& off,
					  Vector<Int>& convOrigin, Vector<Int>& cfShape,
					  Vector<Int>& loc, 
					  Complex& phasor, 
					  Double& sinDPA, Double& cosDPA,
					  Bool& finitePointingOffset, 
					  Matrix<Complex>& cached_phaseGrad_p);
  
  template 
  void AWVisResampler::XInnerLoop(const Int *scaledSupport, const Float* scaledSampling,
  				  const Double* off,
  				  const Int* loc, Complex& cfArea,  
  				  const Int * __restrict__ iGrdPosPtr,
  				  Complex *__restrict__& convFuncV,
				  const Int* convOrigin,
  				  Complex& nvalue,
				  Double& wVal,
  				  Bool& /*finitePointingOffset*/,
  				  Bool& /*doPSFOnly*/,
  				  Complex* __restrict__ gridStore,
  				  Int* iloc,
  				  Complex& norm,
  				  Int* igrdpos);
  template 
  void AWVisResampler::XInnerLoop(const Int *scaledSupport, const Float* scaledSampling,
  				  const Double* off,
  				  const Int* loc, Complex& cfArea,  
  				  const Int * __restrict__ iGrdPosPtr,
  				  Complex *__restrict__& convFuncV,
				  const Int* convOrigin,
  				  Complex& nvalue,
				  Double& wVal,
  				  Bool& /*finitePointingOffset*/,
  				  Bool& /*doPSFOnly*/,
  				  DComplex* __restrict__ gridStore,
  				  Int* iloc,
  				  Complex& norm,
  				  Int* igrdpos);

  Complex* AWVisResampler::getConvFunc_p(Vector<Int>& cfShape, CFBuffer& cfb,
					 Double& wVal, Int& fndx, Int& wndx,
					 PolMapType& mNdx, PolMapType& conjMNdx,
					 Int& ipol, uInt& mRow)
  {
    Bool Dummy;
    Array<Complex> *convFuncV;
    CFCell *cfcell;
    //
    // Since we conjugate the CF depending on the sign of the w-value,
    // pick the appropriate element of the Mueller Matrix (see note on
    // this for details). Without reading the note/understanding,
    // fiddle with this logic at your own risk (can easily lead to a
    // lot of grief. --Sanjay).
    //
    timer_p.mark();

    if (wVal > 0.0) 
      {
	cfcell=&(*(cfb.getCFCellPtr(fndx,wndx,mNdx[ipol][mRow])));

	//	convFuncV=&(*(cfb.getCFCellPtr(fndx,wndx,mNdx[ipol][mRow])->getStorage()));//->getStorage(Dummy);
	//	cerr << "Indexes+: " << fndx << " " << wndx << " " << mNdx[ipol][mRow] << " " << ipol << " " << mRow << " " << convFuncV->shape().asVector() << endl;
      }
    else
      {
	cfcell=&(*(cfb.getCFCellPtr(fndx,wndx,conjMNdx[ipol][mRow])));

	//	convFuncV=&(*(cfb.getCFCellPtr(fndx,wndx,conjMNdx[ipol][mRow])->getStorage()));//->getStorage(Dummy);
	//	cerr << "Indexes-: " << fndx << " " << wndx << " " << conjMNdx[ipol][mRow] << " " << ipol << " " << mRow << " " << convFuncV->shape().asVector() << endl;
      }
    convFuncV = &(*cfcell->getStorage());
    Complex *tt=convFuncV->getStorage(Dummy);
    //    cfShape = convFuncV->shape().asVector();

    cfShape.reference(cfcell->cfShape_p);
    //    cfShape.assign(cfcell->cfShape_p);
    runTimeG1_p += timer_p.real();


    return tt;
  };

  template <class T>
  void AWVisResampler::XInnerLoop(const Int *scaledSupport, const Float* scaledSampling,
  				  const Double* off,
  				  const Int* loc,  Complex& cfArea,  
  				  const Int * __restrict__ iGrdPosPtr,
  				  Complex *__restrict__& convFuncV,
				  const Int* convOrigin,
  				  Complex& nvalue,
				  Double& wVal,
  				  Bool& /*finitePointingOffset*/,
  				  Bool& /*doPSFOnly*/,
  				  T* __restrict__ gridStore,
  				  Int* iloc,
  				  Complex& norm,
  				  Int* igrdpos)
  {
    Complex wt;
    const Int *tt=iloc;
    Bool Dummy;
    const Int *cfInc_ptr=cfInc_p.getStorage(Dummy);
    for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++) 
      {
  	iloc[0]=(Int)((scaledSampling[0]*ix+off[0])-1)+convOrigin[0];
  	igrdpos[0]=loc[0]+ix;

  	{
  	  wt = getFrom4DArray((const Complex * __restrict__ &)convFuncV, 
  			      tt,cfInc_ptr)/cfArea;
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
  Complex AWVisResampler::accumulateOnGrid(Array<T>& grid,Complex* __restrict__& convFuncV, 
					   Complex& nvalue,Double& wVal, 
					   Vector<Int>& scaledSupport, Vector<Float>& scaledSampling, 
					   Vector<Double>& off, Vector<Int>& convOrigin, 
					   Vector<Int>& /*cfShape*/, Vector<Int>& loc, Vector<Int>& igrdpos,
					   Double& /*sinDPA*/, Double& /*cosDPA*/,
					   Bool& finitePointingOffset,
					   Bool doPSFOnly)
  {
    Vector<Int> iloc(4,0), tiloc(4);
    Bool Dummy;
    Complex wt, cfArea=1.0; 
    Complex norm=0.0;
    const Int * __restrict__ iGrdPosPtr = igrdpos.getStorage(Dummy);
    T* __restrict__ gridStore = grid.getStorage(Dummy);
    Int Nth = 1;
#ifdef HAS_OMP
    Nth=max(omp_get_max_threads()-2,1);
#endif


    const Int* scaledSupport_ptr=scaledSupport.getStorage(Dummy);
    const Float *scaledSampling_ptr=scaledSampling.getStorage(Dummy);
    const Double *off_ptr=off.getStorage(Dummy);
    const Int *loc_ptr = loc.getStorage(Dummy);
    const Int* convOrigin_ptr=convOrigin.getStorage(Dummy);
    Int *iloc_ptr=iloc.getStorage(Dummy);
    Int *igrdpos_ptr=igrdpos.getStorage(Dummy);

    Bool finitePointingOffset_l=finitePointingOffset;
    Bool doPSFOnly_l=doPSFOnly;
    Double wVal_l=wVal;
    Complex nvalue_l=nvalue;
    Complex *convFuncV_l=convFuncV;
    //
    // !!!!! Compute cfArea for high precision work
    //

     // cfArea = getCFArea(convFuncV, wVal, scaledSupport, scaledSampling,
     // 		       off, convOrigin, cfShape, sinDPA,cosDPA);


    // cerr << "Cfarea = " << cfArea << endl;
     IPosition phaseGradOrigin_l; 
     phaseGradOrigin_l = cached_phaseGrad_p.shape()/2;

// #pragma omp parallel default(none) \
//   shared(gridStore) \
//   firstprivate(scaledSupport_ptr,scaledSampling_ptr,off_ptr,loc_ptr,cfArea,iGrdPosPtr, \
// 	       convFuncV_l, convOrigin_ptr, nvalue_l, wVal_l, finitePointingOffset_l, doPSFOnly_l, \
// 	       iloc_ptr, norm,igrdpos_ptr) num_threads(Nth)
     {
// #pragma omp for
    for(Int iy=-scaledSupport[1]; iy <= scaledSupport[1]; iy++) 
      {
	iloc_ptr[1]=(Int)((scaledSampling[1]*iy+off[1])-1)+convOrigin[1];
	igrdpos[1]=loc[1]+iy;
	XInnerLoop(scaledSupport_ptr, scaledSampling_ptr,
		   off_ptr,
		   loc_ptr, cfArea,  
		   iGrdPosPtr,
		   convFuncV_l,
		   convOrigin_ptr,
		   nvalue_l,
		   wVal_l,
		   finitePointingOffset_l,
		   doPSFOnly_l,
		   gridStore,
		   iloc_ptr,
		   norm,
		   igrdpos_ptr);

	// for(Int ix=-scaledSupport[0]; ix <= scaledSupport[0]; ix++) 
	//   {
	//     iloc[0]=(Int)((scaledSampling[0]*ix+off[0])-1)+convOrigin[0];
	//     igrdpos[0]=loc[0]+ix;
	//     //
	//     // reindex() is for historical reasons and does three
	//     // operations: (1) rotate the co-ord. sys. using
	//     // sin/cosDPA, (2) add convOrigin to iloc and return the
	//     // result in tiloc and add convOrigin to tiloc, and (3)
	//     // return True if tiloc lines with in the cfShape.
	//     //
	//     //	    if (reindex(iloc,tiloc,sinDPA, cosDPA, convOrigin, cfShape))
	//       {
	// 	wt = getFrom4DArray((const Complex * __restrict__ &)convFuncV, 
	// 			    iloc,cfInc_p)/cfArea;
	// 	if (wVal > 0.0) {wt = conj(wt);}
	// 	norm += (wt);
	// 	if (finitePointingOffset && !doPSFOnly) 
	// 	  wt *= cached_phaseGrad_p(iloc[0]+phaseGradOrigin_l[0],
	// 				   iloc[1]+phaseGradOrigin_l[1]);
	// 	// The following uses raw index on the 4D grid
	// 	addTo4DArray(gridStore,iGrdPosPtr,gridInc_p, nvalue,wt);
	//       }
	//   }
      }
     }
    return norm;
  }
  // Moved the accumulateFromGrid() method to file to play with
  // multi-threading it to not clutter this file.  Both versions
  // (threaded and non-threaded) are in this file.
#include "accumulateFromGrid.cc"
  //
  //-----------------------------------------------------------------------------------
  //
  void AWVisResampler::cachePhaseGrad_p(const Vector<Double>& pointingOffset,
					const Vector<Int>&cfShape,
					const Vector<Int>& convOrigin,
					const Double& /*cfRefFreq*/,
                                        const Double& /*imRefFreq*/)
  {
    LogIO log_l(LogOrigin("AWVisResampler","cachePhaseGrad[R&D]"));
    //cout << "# " << cfRefFreq << " " << imRefFreq << endl;
    if (
    	(sum(fabs(pointingOffset-cached_PointingOffset_p)) > 1e-6) ||
    	(cached_phaseGrad_p.shape()[0] < cfShape[0])              ||
    	(cached_phaseGrad_p.shape()[1] < cfShape[1])
    	)
      {
	log_l << "Computing phase gradiant for pointing offset " 
	      << pointingOffset << cfShape 
	      <<LogIO::POST;
	Int nx=cfShape(0), ny=cfShape(1);
	Double grad;
	Complex phx,phy;

	cached_phaseGrad_p.resize(nx,ny);
	cached_PointingOffset_p = pointingOffset;
	
	for(Int ix=0;ix<nx;ix++)
	  {
	    grad = (ix-convOrigin[0])*pointingOffset[0];
	    Double sx,cx;
	    SINCOS(grad,sx,cx);
	    //	    phx = Complex(cos(grad),sin(grad));
	    phx = Complex(cx,sx);
	    for(Int iy=0;iy<ny;iy++)
	      {
		grad = (iy-convOrigin[1])*pointingOffset[1];
		Double sy,cy;
		SINCOS(grad,sy,cy);
		//		phy = Complex(cos(grad),sin(grad));
		phy = Complex(cy,sy);
		cached_phaseGrad_p(ix,iy)=phx*phy;
	      }
	  }
      }
  }
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
					Bool /*useConjFreqCF*/)
  {
    LogIO log_l(LogOrigin("AWVisResampler[R&D]","DataToGridImpl_p"));
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny, nw, nCFFreq;
    Int targetIMChan, targetIMPol, rbeg, rend;//, PolnPlane, ConjPlane;
    Int startChan, endChan;
    
    Vector<Float> sampling(2),scaledSampling(2);
    Vector<Int> support(2),loc(3), iloc(4),tiloc(4),scaledSupport(2);
    Vector<Double> pos(2), off(3);
    Vector<Int> igrdpos(4);

    Complex phasor, nvalue, wt;
    Complex norm;
    Vector<Int> cfShape;
    cfShape=vbRow2CFBMap_p(0)->getStorage()(0,0,0)->getStorage()->shape().asVector();

    Vector<Int> convOrigin = (cfShape)/2;
    Double sinDPA=0.0, cosDPA=1.0, cfRefFreq;
    //    Double cfScale=1.0;

    timer_p.mark();
    rbeg = 0;       rend = vbs.nRow_p;
    rbeg = vbs.beginRow_p;
    rend = vbs.endRow_p;
    
    nx = grid.shape()[0]; ny = grid.shape()[1]; 
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    Bool Dummy, gDummy, 
      accumCFs=((vbs.uvw_p.nelements() == 0) && dopsf);

    T* __restrict__ gridStore = grid.getStorage(gDummy);
      
    Double *freq=vbs.freq_p.getStorage(Dummy);

    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);

    Bool * __restrict__ flagCube_ptr=vbs.flagCube_p.getStorage(Dummy);
    Bool * __restrict__ rowFlag_ptr = vbs.rowFlag_p.getStorage(Dummy);;
    Float * __restrict__ imgWts_ptr = vbs.imagingWeight_p.getStorage(Dummy);
    Complex * __restrict__ visCube_ptr = vbs.visCube_p.getStorage(Dummy);

    Vector<Double> wVals, fVals; PolMapType mVals, mNdx, conjMVals, conjMNdx;
    Double fIncr, wIncr;
    CFBuffer& cfb = *vbRow2CFBMap_p(0);
    // CFBStruct cfbst;
    // cfb.getAsStruct(cfbst);
    // for(int ii=0;ii<vbs.cfBSt_p.shape[0];ii++)
    //   for(int jj=0;jj<vbs.cfBSt_p.shape[1];jj++)
    // 	for(int kk=0;kk<vbs.cfBSt_p.shape[2];kk++)
    // 	  {
    // 	    CFCStruct cfcst=vbs.cfBSt_p.getCFB(ii,jj,kk);
    // 	    cerr << "[" << ii << "," << jj << "," << kk << "]:" 
    // 		 << cfcst.sampling << " "
    // 		 << cfcst.xSupport << " "
    // 		 << cfcst.ySupport 
    // 		 << endl;
    // 	  }



    cfb.getCoordList(fVals,wVals,mNdx, mVals, conjMNdx, conjMVals, fIncr, wIncr);
    Vector<Double> pointingOffset(cfb.getPointingOffset());
    runTimeG1_p += timer_p.real();

    nw = wVals.nelements();
    nCFFreq = fVals.nelements()-1;
    iloc = 0;

    // timer.mark();
    IPosition shp=vbs.flagCube_p.shape();
    Cube<Bool> allPolNChanDone_l(shp(0),shp(1),1);
   if (accumCFs)
     {
       for (Int ipol=0;ipol<nDataPol;ipol++)
         {
           if (polMap_p(ipol) < 0)
             {
               for (Int ichan=0;ichan<nDataChan;ichan++)
                 //for (Int irow=rbeg;irow<rend;irow++)
                   allPolNChanDone_l(ipol,ichan,0)=True;
             }
         }

	startChan = vbs.startChan_p;
	endChan = vbs.endChan_p;
     }
    else 
      {
	startChan = 0;
	endChan = nDataChan;
      }

   Bool finitePointingOffsets=(
			      (fabs(pointingOffset(0))>0) ||  
			      (fabs(pointingOffset(1))>0)
			      );
   Bool isGridSinglePrecision=(typeid(gridStore[0]) == typeid(wt));

   //   Double conjRefFreq = vbs.imRefFreq();
   Int vbSpw = (vbs.vb_p)->spectralWindow();

    for(Int irow=rbeg; irow< rend; irow++){   
      //      if ((vbs.uvw_p.nelements() == 0)) 
      //if (accumCFs) if (allTrue(allPolNChanDone_l)) break;
      
      if(!(*(rowFlag_ptr+irow)))
	{   
	  for(Int ichan=startChan; ichan< endChan; ichan++)
	    {
	      if (*(imgWts_ptr + ichan+irow*nDataChan)!=0.0) 
		{  
		  targetIMChan=chanMap_p[ichan];
		  
		  if((targetIMChan>=0) && (targetIMChan<nGridChan)) 
		    {
		      timer_p.mark();
		      Double dataWVal = vbs.vb_p->uvw()(irow)(2);
		      Int wndx = cfb.nearestWNdx(abs(dataWVal)*freq[ichan]/C::c);
		      // Double conjFreq=sqrt(2*conjRefFreq*conjRefFreq - freq[ichan]*freq[ichan]);
		      // Int fndx = cfb.nearestFreqNdx(freq[ichan]),
		      // 	conjFNdx =cfb.nearestFreqNdx(conjFreq);

		      // Int fndx=cfb.nearestFreqNdx(vbSpw,ichan), conjFNdx=cfb.nearestFreqNdx(vbSpw,ichan,True);
		      Int cfFreqNdx;
		      if (vbs.conjBeams_p) cfFreqNdx = cfb.nearestFreqNdx(vbSpw,ichan,True);// Get the conj. freq. index
		      else  cfFreqNdx = cfb.nearestFreqNdx(vbSpw,ichan);

		      //		      cerr << "G: " << cfFreqNdx << " " << wndx << " " << ichan << " " << vbSpw << " " << freq[ichan] << endl;

		      runTimeG3_p += timer_p.real();
		      
		      Float s;
		      //
		      //------------------------------------------------------------------------------
		      //
		      // Using the int-index version for Freq, W and Muellerelements
		      //	      cfb.getParams(cfRefFreq, s, support(0), support(1),0,wndx,0);
		      //
		      //------------------------------------------------------------------------------
		      //
		      // if (vbs.conjBeams_p)
		      // 	{
		      // 	  timer_p.mark();
		      // 	  cfb.getParams(cfRefFreq, s, support(0), support(1),conjFNdx,wndx,0);
		      // 	  runTimeG4_p += timer_p.real();
		      // 	}
		      // else
		      // 	cfb.getParams(cfRefFreq, s, support(0), support(1),fndx,wndx,0);

		      timer_p.mark();
		      cfb.getParams(cfRefFreq, s, support(0), support(1),cfFreqNdx,wndx,0);
		      runTimeG4_p += timer_p.real();

		      sampling(0) = sampling(1) = SynthesisUtils::nint(s);
		      // sampling[0] = SynthesisUtils::nint(sampling[0]*cfScale);
		      // sampling[1] = SynthesisUtils::nint(sampling[1]*cfScale);
		      // support[0]  = SynthesisUtils::nint(support[0]/cfScale);
		      // support[1]  = SynthesisUtils::nint(support[1]/cfScale);

		      timer_p.mark();
		      sgrid(pos,loc,off, phasor, irow, vbs.uvw_p, dphase_p[irow], freq[ichan], 
			    uvwScale_p, offset_p, sampling);
		      runTimeG5_p += timer_p.real();
		      
		      if (onGrid(nx, ny, nw, loc, support)) 
			{
			  // Loop over all image-plane polarization planes.
			  for(Int ipol=0; ipol< nDataPol; ipol++) 
			    { 
			      if((!(*(flagCube_ptr + ipol + ichan*nDataPol + irow*nDataPol*nDataChan))))
				{  
				  targetIMPol=polMap_p(ipol);
				  if ((targetIMPol>=0) && (targetIMPol<nGridPol)) 
				    {
				      igrdpos[2]=targetIMPol; igrdpos[3]=targetIMChan;
				      
				      if(accumCFs)     allPolNChanDone_l(ipol,ichan,0)=True;
				      
				      // ConjPlane = cfMap_p[ipol];
				      // PolnPlane = conjCFMap_p[ipol];
				      
				      if(dopsf) nvalue=Complex(*(imgWts_ptr + ichan + irow*nDataChan));
				      else      nvalue= *(imgWts_ptr+ichan+irow*nDataChan)*
						  (*(visCube_ptr+ipol+ichan*nDataPol+irow*nDataChan*nDataPol)*phasor);
				      
				      norm = 0.0;
				      // Loop over all relevant elements of the Mueller matrix for the polarization
				      // ipol.
				      for (uInt mRow=0;mRow<conjMNdx[ipol].nelements(); mRow++) 
					{
					  Complex* convFuncV;
					  timer_p.mark();
					  // if (vbs.conjBeams_p) convFuncV=getConvFunc_p(cfShape, cfb, dataWVal, conjFNdx, 
					  // 					       wndx, mNdx, conjMNdx, ipol,  mRow);
					  // else                 convFuncV=getConvFunc_p(cfShape, cfb, dataWVal, fndx, 
					  // 					       wndx, mNdx, conjMNdx, ipol,  mRow);

					  convFuncV=getConvFunc_p(cfShape, cfb, dataWVal, cfFreqNdx,
								  wndx, mNdx, conjMNdx, ipol,  mRow);
					  
					  runTimeG6_p += timer_p.real();
					  // support.assign(scaledSupport);
					  // sampling.assign(scaledSampling);
					  
					  convOrigin=cfShape/2;
					  Bool psfOnly=((dopsf==True) && (accumCFs==False));
					  if (finitePointingOffsets && !psfOnly)
					    cachePhaseGrad_p(pointingOffset, cfShape, convOrigin, cfRefFreq, vbs.imRefFreq());
					  
					  cacheAxisIncrements(cfShape, cfInc_p);
					  
					  // accumulateOnGrid() is a local C++ method with the inner loops.  The include
					  // file (FortanizedLoopsToGrid.cc) has the interface code to call the inner 
					  // loops re-written in FORTRAN (in synthesis/fortran/faccumulateOnGrid.f)

timer_p.mark();
					  // norm += accumulateOnGrid(grid,convFuncV,nvalue,dataWVal,
					  // 			   support,sampling,
					  // 			   off, convOrigin, cfShape, loc, igrdpos,
					  // 			   sinDPA, cosDPA,finitePointingOffsets,psfOnly);
// cerr << vbs.vb_p->spectralWindow() << " " << vbs.vb_p->rowIds()(irow) << " " << irow << " " << ichan << " " << ipol << " " << mRow << endl;
#include <synthesis/TransformMachines/FortranizedLoopsToGrid.cc>
runTimeG7_p += timer_p.real();
					}
				      //				      cerr << "Norm = " << norm << endl;
				      sumwt(targetIMPol,targetIMChan) += vbs.imagingWeight_p(ichan, irow)*abs(norm);
				      //		      *(sumWt_ptr+apol+achan*nGridChan)+= *(imgWts_ptr+ichan+irow*nDataChan);
				    }
				}
			    } // End poln-loop
			}
		    }
		}
	    } // End chan-loop
	}
    } // End row-loop
    // exit(0);
    runTimeG_p = timer_p.real() + runTimeG1_p + runTimeG2_p + runTimeG3_p + runTimeG4_p + runTimeG5_p + runTimeG6_p + runTimeG7_p;
    T *tt=(T *)gridStore;
    grid.putStorage(tt,gDummy);
  }
  //
  //-----------------------------------------------------------------------------------
  // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
  //
  void AWVisResampler::GridToData(VBStore& vbs, const Array<Complex>& grid)
  {
    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny,nw, nCFFreq;
    Int achan, apol, rbeg, rend;//, PolnPlane, ConjPlane;
    Vector<Float> sampling(2);//scaledSampling(2);
    Vector<Int> support(2),loc(3), iloc(4),tiloc(4);// scaledSupport(2);
    Vector<Double> pos(2), off(3);
    
    IPosition grdpos(4);
    
    Vector<Complex> norm(4,0.0);
    Complex phasor, nvalue, wt;
    Vector<Int> cfShape=vbRow2CFBMap_p(0)->getStorage()(0,0,0)->getStorage()->shape().asVector();
    Vector<Double> pointingOffset((*vbRow2CFBMap_p(0)).getPointingOffset());
    
    //    Vector<Int> convOrigin = (cfShape-1)/2;
    Vector<Int> convOrigin = (cfShape)/2;
    Double sinDPA=0.0, cosDPA=1.0, cfRefFreq;//cfScale=1.0
    //    Int wndx = 0, fndx=0;
    
    rbeg=0;
    rend=vbs.nRow_p;
    rbeg = vbs.beginRow_p;
    rend = vbs.endRow_p;
    nx       = grid.shape()[0]; ny        = grid.shape()[1];
    nw       = cfShape[2];
    nGridPol = grid.shape()[2]; nGridChan = grid.shape()[3];
    
    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];
    
    //
    // The following code reduces most array accesses to the simplest
    // possible to improve performance.  However this made no
    // difference in the run-time performance compared to Vector,
    // Matrix and Cube indexing.
    //
    Bool Dummy;
    const Complex* __restrict__ gridStore = grid.getStorage(Dummy);
    (void)gridStore;
    Vector<Int> igrdpos(4);
    Double *freq=vbs.freq_p.getStorage(Dummy);
    Bool *rowFlag=vbs.rowFlag_p.getStorage(Dummy);
    
    Matrix<Double>& uvw=vbs.uvw_p;
    Cube<Complex>&  visCube=vbs.visCube_p;
    Cube<Bool>&     flagCube=vbs.flagCube_p;
    
    Vector<Int> gridInc, cfInc;
    
    cacheAxisIncrements(grid.shape().asVector(), gridInc_p);
    cacheAxisIncrements(cfShape, cfInc_p);
    // Initialize the co-ordinates used for reading the CF values The
    // CFs are 4D arrays, with the last two axis degenerate (of length
    // 1).  The last two axis were formerly the W-, and
    // Polarization-axis.
    iloc = 0;
    Bool finitePointingOffset=(
			       (fabs(pointingOffset(0))>0) ||  
			       (fabs(pointingOffset(1))>0)
			       );
    Int vbSpw = (vbs.vb_p)->spectralWindow();

    for(Int irow=rbeg; irow<rend; irow++) {
      if(!rowFlag[irow]) {
	CFBuffer& cfb = *vbRow2CFBMap_p(irow);
	Vector<Double> wVals, fVals; PolMapType mVals, mNdx, conjMVals, conjMNdx;
	Double fIncr, wIncr;
	cfb.getCoordList(fVals,wVals,mNdx, mVals, conjMNdx, conjMVals, fIncr, wIncr);
	nw = wVals.nelements();
	nCFFreq = fVals.nelements()-1;
	
	for (Int ichan=0; ichan < nDataChan; ichan++) {
	  achan=chanMap_p[ichan];
	  
	  if((achan>=0) && (achan<nGridChan)) {
	    //	    lambda = C::c/freq[ichan];
	    Double dataWVal = (vbs.vb_p->uvw()(irow)(2));
	    Int wndx = cfb.nearestWNdx(abs(dataWVal)*freq[ichan]/C::c);
	    //Int fndx = cfb.nearestFreqNdx(freq[ichan]);
	    Int fndx = cfb.nearestFreqNdx(vbSpw,ichan);

	    //	    cerr << "DG: " << fndx << " " << wndx << " " << ichan << " " << vbSpw << " " << freq[ichan] << endl;
	    
	    //	    cerr << "Grid: " << ichan << " " << freq[ichan] << " " << fndx << endl;
	    
	    // if (nw > 1) wndx=SynthesisUtils::nint((dataWVal*freq[ichan]/C::c)/wIncr-1);
	    // if (nCFFreq > 0) fndx = SynthesisUtils::nint((freq[ichan])/fIncr-1);
	    Float s;
	    // CoordinateSystem cs; 
	    // cfb.getParams(cs,s,support(0),support(1),0,wndx,0);
	    cfb.getParams(cfRefFreq,s,support(0),support(1),fndx,wndx,0);
	    sampling(0) = sampling(1) = SynthesisUtils::nint(s);
	    
	    //cfScale = cfRefFreq/freq[ichan];
	    
	    // sampling[0] = SynthesisUtils::nint(sampling[0]*cfScale);
	    // sampling[1] = SynthesisUtils::nint(sampling[1]*cfScale);
	    // support[0]  = SynthesisUtils::nint(support[0]/cfScale);
	    // support[1]  = SynthesisUtils::nint(support[1]/cfScale);
	    
	    sgrid(pos,loc,off,phasor,irow,uvw,dphase_p[irow],freq[ichan],
		  uvwScale_p,offset_p,sampling);
	    
	    //	    iloc[2]=max(0, min(nw, loc[2]));
	    
	    Bool isOnGrid;
	    if ((isOnGrid=onGrid(nx, ny, nw, loc, support))) {
	      for(Int ipol=0; ipol < nDataPol; ipol++) {
		
		if(!flagCube(ipol,ichan,irow)) { 
		  apol=polMap_p[ipol];
		  
		  if((apol>=0) && (apol<nGridPol)) {
		    igrdpos[2]=apol; igrdpos[3]=achan;
		    nvalue=0.0;      norm(apol)=0.0;
		    
		    // ConjPlane = cfMap_p(ipol);
		    // PolnPlane = conjCFMap_p(ipol);
		    
		    // With VBRow2CFMap in use, CF for each pol. plane is a separate 2D Array.  
		    //		    iloc[3]=0;
		    for (uInt mRow=0; mRow<conjMNdx[ipol].nelements(); mRow++)
		      {
			//
			// Get the pointer to the storage for the CF
			// indexed by the Freq, W-term and Mueller
			// Element.
			//
			Complex*  convFuncV;
			convFuncV = getConvFunc_p(cfShape, cfb, dataWVal, fndx, wndx, mNdx,
						  conjMNdx, ipol, mRow);
			//
			// Compute the incrmenets and center pixel for the current CF
			//
			cacheAxisIncrements(cfShape, cfInc_p);
			convOrigin = (cfShape)/2;
			if (finitePointingOffset)
			  cachePhaseGrad_p(pointingOffset, cfShape, convOrigin, cfRefFreq, vbs.imRefFreq());
			
			//
			// ALERT: The -1 in the expression for iloc
			// was determined by comparing with a working
			// old code (fpbmos.f).
			//
			// Complex tt=0.0;
			// int nn=0;


			// accumulateFromGrid() is a local C++ method with the inner loops.  The include
			// file (FortanizedLoopsFromGrid.cc) has the interface code to call the inner 
			// loops re-written in FORTRAN (in synthesis/fortran/faccumulateOnGrid.f)

			// accumulateFromGrid(nvalue, gridStore, igrdpos, convFuncV, dataWVal,
			// 		   scaledSupport, scaledSampling, off, convOrigin, 
			// 		   cfShape, loc, phasor, sinDPA, cosDPA, 
			// 		   finitePointingOffset, cached_phaseGrad_p);
// Timer timer;
#include <synthesis/TransformMachines/FortranizedLoopsFromGrid.cc>
// runTimeDG_p += timer.real();

			 // //--------------------------------------------------------------------------------
			 // IPosition phaseGradOrigin_l = cached_phaseGrad_p.shape()/2;
			 // for(Int iy=-support[1]; iy <= support[1]; iy++) 
			 //   {
			 //     //			    iloc(1)=(Int)(scaledSampling[1]*iy+off[1]-1);//+convOrigin[1];
			 //     iloc(1)=(sampling[1]*iy+off[1]);//+convOrigin[1];
			 //     igrdpos[1]=loc[1]+iy;
			 //     for(Int ix=-support[0]; ix <= support[0]; ix++) 
			 //       {
			 // 	 //				iloc(0)=(Int)(scaledSampling[0]*ix+off[0]-1);//+convOrigin[0];
			 // 	 iloc(0)=(sampling[0]*ix+off[0]);//+convOrigin[0];
			 // 	 igrdpos[0]=loc[0]+ix;
			 // 	 tiloc=iloc;
			 // 	 if (reindex(iloc,tiloc,sinDPA, cosDPA, convOrigin, cfShape))
			 // 	   {
			 // 	     wt=getFrom4DArray((const Complex * __restrict__ &)convFuncV,
			 // 			       tiloc,cfInc_p);
			 // 	     if (dataWVal > 0.0) wt = conj(wt);
			 // 	     norm(apol)+=(wt);
			 // 	     if (finitePointingOffset) 
			 // 	       {
			 // 		 wt *= (cached_phaseGrad_p(iloc[0]+phaseGradOrigin_l(0),  iloc[1]+phaseGradOrigin_l(1)));
			 // 		 // ttt += (cached_phaseGrad_p(iloc[0]+phaseGradOrigin_l(0), iloc[1]+phaseGradOrigin_l(1)));
			 // 		 // cerr << "## " << (cached_phaseGrad_p(iloc[0]+phaseGradOrigin_l(0), iloc[1]+phaseGradOrigin_l(1))) << " "
			 // 		 //      << iloc << " " << phaseGradOrigin_l << endl;
			 // 	       }
			 // 	     // nvalue+=wt*grid(grdpos);
			 // 	     // The following uses raw index on the 4D grid
			 // 	     // nvalue+=wt*getFrom4DArray(gridStore,iPosPtr,gridInc);
			 // 	     nvalue+=wt*getFrom4DArray(gridStore,igrdpos,gridInc_p);
			 // 	   }
			 //       }
			 //   }
			 // //--------------------------------------------------------------------------------

			 //		    visCube(ipol,ichan,irow)=(nvalue*conj(phasor))/norm(apol);
		      }
		    visCube(ipol,ichan,irow)=nvalue; // Goes with FortranizedLoopsFromGrid.cc
		    //visCube(ipol,ichan,irow)=nvalue*conj(phasor)/norm(apol); // Goes with C++ loops
		    // cerr << ipol << " " << ichan << " " << irow << " " << nvalue << " " << norm(apol) << " " << pointingOffset 
		    // 	 << " " << qualifier_p << " " << ttt << " " << scaledSupport << endl;
		}
	      }
	    }
	  }
	}
      }
      //	junk++;
    }
  } // End row-loop
    // cerr << endl;
    // if (junk==20) exit(0);
}
//
//-----------------------------------------------------------------------------------
//
void AWVisResampler::sgrid(Vector<Double>& pos, Vector<Int>& loc, 
			   Vector<Double>& off, Complex& phasor, 
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
      pos[idim]=scale[idim]*uvw_l(idim)*freq/C::c+(offset[idim]);
      loc[idim]=SynthesisUtils::nint(pos[idim]);
      //	off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]+1);
      off[idim]=SynthesisUtils::nint((loc[idim]-pos[idim])*sampling[idim]);
    }
  
  if (dphase != 0.0)
    {
      phase=-2.0*C::pi*dphase*freq/C::c;
      Double sp,cp;
      SINCOS(phase,sp,cp);
      //      phasor=Complex(cos(phase), sin(phase));
      phasor=Complex(cp,sp);
    }
  else
    phasor=Complex(1.0);
  // cerr << "### " << pos[0] << " " << offset[0] << " " << loc[0] << " " << off[0] << " " << uvw_l(0) << endl;
  // exit(0);
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

Complex AWVisResampler::getCFArea(Complex* __restrict__& convFuncV, 
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
		  wt = getFrom4DArray((const Complex * __restrict__ &)convFuncV, 
				      tiloc,cfInc_p);
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
};// end namespace casa
