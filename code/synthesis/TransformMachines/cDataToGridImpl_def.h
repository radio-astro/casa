#ifndef SYNTHESIS_CDATATOGRIDIMPLDEF
#define SYNTHESIS_CDATATOGRIDIMPLDEF
#include <casa/Arrays/Matrix.h>
#include <synthesis/TransformMachines/VBStore.h>

  //
  //----------------------------------------------------------------------
  //

  // template
  // void cuDataToGridImpl_p(Complex* gridStore, Int* gridShape, VBStore* vbs,
  // 				     Matrix<Double>* sumwt, 
  // 				     const Bool dopsf,
  // 				     const Int* polMap_ptr, const Int *chanMap_ptr,
  // 				     const Double *uvwScale_ptr, const Double *offset_ptr,
  // 				     const Double *dphase_ptr,
  // 				     Int XThGrid=0, Int YThGrid=0
  // 				     );
  // template
  // void cuDataToGridImpl_p(DComplex* gridStore, Int* gridShape, VBStore* vbs,
  // 				     Matrix<Double>* sumwt, 
  // 				     const Bool dopsf ,
  // 				     const Int* polMap_ptr, const Int *chanMap_ptr,
  // 				     const Double *uvwScale_ptr, const Double *offset_ptr,
  // 				     const Double *dphase_ptr,
  // 				     Int XThGrid=0, Int YThGrid=0
  // 				     );

  template
  void cDataToGridImpl_p(Complex* gridStore, Int* gridShape, VBStore* vbs,
			 Matrix<Double>* sumwt, const Bool dopsf,
			 const Int* polMap_ptr, const Int *chanMap_ptr,
			 const Double *uvwScale_ptr, const Double *offset_ptr,
			 const Double *dphase_ptr,
			 Int XThGrid=0, Int YThGrid=0);
  template
  void cDataToGridImpl_p(DComplex* gridStore, Int* gridShape, VBStore* vbs,
			 Matrix<Double>* sumwt, const Bool dopsf,
			 const Int* polMap_ptr, const Int *chanMap_ptr,
			 const Double *uvwScale_ptr, const Double *offset_ptr,
			 const Double *dphase_ptr,Int XThGrid=0, Int YThGrid=0);
  

  template
  Complex caccumulateOnGrid(Complex* gridStore, const Int* gridInc_p, const Complex *cached_phaseGrad_p,
			    const Int cachedPhaseGradNX, const Int cachedPhaseGradNY,
			    const Complex* convFuncV, const Int *cfInc_p,Complex nvalue,
			    Double wVal, Int *supBLC_ptr, Int *supTRC_ptr,
			    Float* scaledSampling_ptr, Double* off_ptr, Int* convOrigin_ptr, 
			    Int* cfShape, Int* loc_ptr, Int* iGrdpos_ptr, Bool finitePointingOffset,
			    Bool doPSFOnly, Bool& foundCFPeak);

  template
  Complex caccumulateOnGrid(DComplex* gridStore, const Int* gridInc_p, const Complex *cached_phaseGrad_p,
			    const Int cachedPhaseGradNX, const Int cachedPhaseGradNY,
			    const Complex* convFuncV, const Int *cfInc_p,Complex nvalue,
			    Double wVal, Int *supBLC_ptr, Int *supTRC_ptr,
			    Float* scaledSampling_ptr, Double* off_ptr, Int* convOrigin_ptr, 
			    Int* cfShape, Int* loc_ptr, Int* iGrdpos_ptr, Bool finitePointingOffset,
			    Bool doPSFOnly, Bool& foundCFPeak);
  //
  //----------------------------------------------------------------------
  //

  template
  void addTo4DArray(Complex *store, const Int *iPos, const Int* inc, 
		    Complex& nvalue, Complex& wt);
  template
  void addTo4DArray(DComplex *store, const Int *iPos, const Int* inc, 
		    Complex& nvalue, Complex& wt);

#endif // 
