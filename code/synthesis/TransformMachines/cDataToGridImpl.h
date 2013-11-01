#ifndef SYNTHESIS_CDATATOGRIDIMPL
#define SYNTHESIS_CDATATOGRIDIMPL

#include <casa/Arrays/Matrix.h>
#include <synthesis/TransformMachines/VBStore.h>

namespace casa { //# NAMESPACE CASA - BEGIN


  typedef void (*ComplexGridder)(Complex* gridStore, Int* gridShape, VBStore* vbs,
				  Matrix<Double>* sumwt, const Bool dopsf,
				  const Int* polMap_ptr, const Int *chanMap_ptr,
				  const Double *uvwScale_ptr, const Double *offset_ptr,
				  const Double *dphase_ptr, Int XThGrid, Int YThGrid);
  typedef void (*DComplexGridder)(DComplex* gridStore, Int* gridShape, VBStore* vbs,
				  Matrix<Double>* sumwt, const Bool dopsf,
				  const Int* polMap_ptr, const Int *chanMap_ptr,
				  const Double *uvwScale_ptr, const Double *offset_ptr,
				  const Double *dphase_ptr, Int XThGrid, Int YThGrid);

  // template <class T>
  // void cuDataToGridImpl_p(T* gridStore, Int* gridShape, VBStore* vbs,
  // 				     Matrix<Double>* sumwt, 
  // 				     const Bool dopsf ,
  // 				     const Int* polMap_ptr, const Int *chanMap_ptr,
  // 				     const Double *uvwScale_ptr, const Double *offset_ptr,
  // 				     const Double *dphase_ptr, Int XThGrid=0, Int YThGrid=0
  // 				     );

  template <class T>
  void cDataToGridImpl_p(T* gridStore, Int* gridShape, VBStore* vbs,
  			 Matrix<Double>* sumwt, const Bool dopsf,
  			 const Int* polMap_ptr, const Int *chanMap_ptr,
  			 const Double *uvwScale_ptr, const Double *offset_ptr,
  			 const Double *dphase_ptr, Int XThGrid=0, Int YThGrid=0);

  void csgrid(Double pos[2], Int loc[3], Double off[3], Complex& phasor, 
	      const Int& irow, const Matrix<Double>& uvw, const Double& dphase, 
	      const Double& freq, const Double* scale, const Double* offset,
	      const Float sampling[2]);

  Bool ccomputeSupport(const VBStore* vbs, const Int& XThGrid, const Int& YThGrid,
		       const Int support[2], const Float sampling[2],
		       const Double pos[2], const Int loc[3],
		       Float iblc[2], Float itrc[2]);

  Complex* cgetConvFunc_p(Int cfShape[4], VBStore* vbs, Double& wVal, Int& fndx, 
			  Int& wndx, Int **mNdx, Int  **conjMNdx,Int& ipol, uInt& mRow);


  void ccachePhaseGrad_g(Complex *cached_phaseGrad_p, Int phaseGradNX, Int phaseGradNY,
			 Double* cached_PointingOffset_p, Double* pointingOffset,
			 Int cfShape[4], Int convOrigin[4]);

  template <class T>
  Complex caccumulateOnGrid(T* gridStore, const Int* gridInc_p, const Complex *cached_phaseGrad_p,
			    const Int cachedPhaseGradNX, const Int cachedPhaseGradNY,
			    const Complex* convFuncV, const Int *cfInc_p,Complex nvalue,
			    Double wVal, Int *supBLC_ptr, Int *supTRC_ptr,
			    Float* scaledSampling_ptr, Double* off_ptr, Int* convOrigin_ptr, 
			    Int* cfShape, Int* loc_ptr, Int* iGrdpos_ptr, Bool finitePointingOffset,
			    Bool doPSFOnly, Bool& foundCFPeak);
  //
  //----------------------------------------------------------------------
  //
  inline Complex getFrom4DArray(const Complex * store,
				const Int* iPos, const Int inc[4])
  {return *(store+(iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]));};
  //
  //----------------------------------------------------------------------
  //
  inline void cacheAxisIncrements(const Int n[4], Int inc[4])
  {inc[0]=1; inc[1]=inc[0]*n[0]; inc[2]=inc[1]*n[1]; inc[3]=inc[2]*n[2];(void)n[3];}
  //
  //----------------------------------------------------------------------
  //
  template <class T>
  void addTo4DArray(T *store, const Int *iPos, const Int* inc, 
		    Complex& nvalue, Complex& wt)
  {store[iPos[0] + iPos[1]*inc[1] + iPos[2]*inc[2] +iPos[3]*inc[3]] += (nvalue*wt);}
  //
  //----------------------------------------------------------------------
  //

};
#endif // 

