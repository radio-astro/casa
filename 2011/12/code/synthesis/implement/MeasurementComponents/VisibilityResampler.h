// -*- C++ -*-
//# VisibilityResampler.h: Definition of the VisibilityResampler class
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

#ifndef SYNTHESIS_VISIBILITYRESAMPLER_H
#define SYNTHESIS_VISIBILITYRESAMPLER_H

#include <synthesis/MeasurementComponents/CFStore.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/VBStore.h>
#include <synthesis/MeasurementComponents/VisibilityResamplerBase.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <msvis/MSVis/AsynchronousTools.h>

#include <casa/aips.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>

#include <complex>

namespace casa { //# NAMESPACE CASA - BEGIN

class VisibilityResampler: public VisibilityResamplerBase
{
public:
    VisibilityResampler(): VisibilityResamplerBase() {};
    VisibilityResampler(const CFStore& cfs): VisibilityResamplerBase(cfs) {};
    VisibilityResampler(const VisibilityResampler& other):VisibilityResamplerBase()
    {copy(other);}

    //    {setConvFunc(cfs);};
    virtual ~VisibilityResampler() {};

    //    VisibilityResampler& operator=(const VisibilityResampler& other);

    void copy(const VisibilityResampler& other)
    {VisibilityResamplerBase::copy(other);}

    virtual VisibilityResamplerBase* clone() 
            {return new VisibilityResampler(*this);}

    virtual void setParams(const Vector<Double>& uvwScale, const Vector<Double>& offset,
                           const Vector<Double>& dphase)
    {
        // SynthesisUtils::SETVEC(uvwScale_p, uvwScale);
        // SynthesisUtils::SETVEC(offset_p, offset);
        // SynthesisUtils::SETVEC(dphase_p, dphase);
        uvwScale_p.reference(uvwScale);
        offset_p.reference(offset);
        dphase_p.reference(dphase);
    };

    virtual void setMaps(const Vector<Int>& chanMap, const Vector<Int>& polMap)
    {
        // SynthesisUtils::SETVEC(chanMap_p,chanMap);
        // SynthesisUtils::SETVEC(polMap_p,polMap);
        chanMap_p.reference(chanMap);
        polMap_p.reference(polMap);
    }

    virtual void setConvFunc(const CFStore& cfs) 
    {
        convFuncStore_p = cfs;
    };
    virtual void setCFMaps(const Vector<Int>& cfMap, const Vector<Int>& conjCFMap) {};
    //
    //------------------------------------------------------------------------------
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a gridding).
    //
    // In this class, these just call the private templated version.
    // The first variant grids onto a double precision grid while the
    // second one does it on a single precision grid.
    //
    virtual void DataToGrid(Array<DComplex>& griddedData, VBStore& vbs, 
                            Matrix<Double>& sumwt, const Bool& dopsf);

    virtual void DataToGrid(Array<Complex>& griddedData, VBStore& vbs, 
                            Matrix<Double>& sumwt, const Bool& dopsf);

    //
    //------------------------------------------------------------------------------
    //
    // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
    //
    virtual void GridToData(VBStore& vbs,const Array<Complex>& griddedData); 
    //    virtual void GridToData(VBStore& vbs, Array<Complex>& griddedData); 

    virtual void ComputeResiduals(VBStore& vbs);
    virtual void setMutex(async::Mutex *mu) {myMutex_p = mu;};

    // Genealogical baggage -- required for the
    // MultiThreadedVisibilityResampler -- that everyone else has to
    // carray around.
    //
    // These are no-ops for unithreaded samplers.
    //
    virtual void init(const Bool& doublePrecision) {};
    virtual void GatherGrids(Array<DComplex>& griddedData, Matrix<Double>& sumwt) {};
    virtual void GatherGrids(Array<Complex>& griddedData, Matrix<Double>& sumwt) {};
    virtual void initializePutBuffers(const Array<DComplex>& griddedData,
                                      const Matrix<Double>& sumwt) {};
    virtual void initializePutBuffers(const Array<Complex>& griddedData,
                                      const Matrix<Double>& sumwt) {};


    //
    //------------------------------------------------------------------------------
    //----------------------------Private parts-------------------------------------
    //------------------------------------------------------------------------------
    //
protected:

    typedef enum {Grid, Degrid} GridOrDegrid;
    async::Mutex *myMutex_p;
    // Vector<Double> uvwScale_p, offset_p, dphase_p;
    // Vector<Int> chanMap_p, polMap_p;
    // CFStore convFuncStore_p;
    //    Int inc0_p, inc1_p, inc2_p, inc3_p;
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a de-gridding).
    //
//    template <class T>
//    void DataToGridImpl_p(Array<T>& griddedData, VBStore& vb,
//                          const Bool& dopsf, Matrix<Double>& sumwt);

    // Grid/Degrid inner loop operations.  These are designed to inline nicely with
    // the logic in gridDegrid (see below). The paraeters are:
    //
    // grid - Pointer to an element of the grid (const for degrid)
    // weight - Reference to the weight to be applied to the current position.  Its float size
    //          should match up with the complex's type (e.g. DComplex <--> Double).
    // value - Reference to the value to be put into or built from the grid.  It should
    //         have the same sized complex as the grid element and will be const for
    //         gridding.

    static inline void
    gridDegridInnerOperation (DComplex * grid, const Double & weight, const DComplex & value)
    {
        * grid += weight * value; // Gridding
    }

    static inline void
    gridDegridInnerOperation (Complex * grid, const Float & weight, const Complex & value)
    {
        * grid += weight * value; // Gridding
    }

    static inline void
    gridDegridInnerOperation (const Complex * grid, const Float &  weight, Complex & value)
    {
        value += (* grid) * weight; // Degridding
    }

    static inline void
    gridDegridInnerOperation (const DComplex * grid, const Double &  weight, DComplex & value)
    {
        value += (* grid) * weight; // Degridding
    }

    // The templated method that will perform

    template <typename G, typename GP, typename Val>
    void gridDegrid (GridOrDegrid gridOrDegrid,
                     G & grid,
                     VBStore & vbs,
                     Bool dopsf = True,
                     Matrix<Double> * sumwt = NULL);


    // void sgrid(Vector<Double>& pos, Vector<Int>& loc, Vector<Int>& off, 
    // 	       Complex& phasor, const Int& irow, const Matrix<Double>& uvw, 
    // 	       const Double& dphase, const Double& freq, 
    // 	       const Vector<Double>& scale, const Vector<Double>& offset,
    // 	       const Vector<Float>& sampling);

    /*
    void sgrid(Int& ndim, Double* pos, Int* loc, Int* off, 
    	       Complex& phasor, const Int& irow, const Double* uvw, 
    	       const Double& dphase, const Double& freq, 
    	       const Double* scale, const Double* offset,
    	       const Float* sampling);

    inline Bool onGrid (const Int& nx, const Int& ny, 
			const Vector<Int>& __restrict__ loc, 
			const Vector<Int>& __restrict__ support) __restrict__ 
    {
      return (((loc(0)-support[0]) >= 0 ) && ((loc(0)+support[0]) < nx) &&
	      ((loc(1)-support[1]) >= 0 ) && ((loc(1)+support[1]) < ny));
    };
    inline Bool onGrid (const Int& nx, const Int& ny, 
			const Int& loc0, const Int& loc1, 
			const Int& support) __restrict__ 
    {
      return (((loc0-support) >= 0 ) && ((loc0+support) < nx) &&
	      ((loc1-support) >= 0 ) && ((loc1+support) < ny));
    };

    // Array assignment operator in CASACore requires lhs.nelements()
    // == 0 or lhs.nelements()=rhs.nelements()
    // template <class T>
    // inline void SETVEC(Vector<T>& lhs, const Vector<T>& rhs)
    // {lhs.resize(rhs.shape()); lhs = rhs;};


    //
    // Internal methods to address a 4D array.  These should ulimately
    // moved to a Array4D class in CASACore
    //

    // This is called less frequently.  Currently once per VisBuffer
    inline void cacheAxisIncrements(Int& n0, Int& n1, Int& n2, Int& n3)
    {inc0_p=1, inc1_p=inc0_p*n0, inc2_p=inc1_p*n1, inc3_p=inc2_p*n2;(void)n3;}


    // The following two methods are called in the innermost loop.
    inline Complex getFrom4DArray(const Complex* __restrict__ store,
				  const Int* __restrict__ iPos) __restrict__ 
    {return store[iPos[0] + iPos[1]*inc1_p + iPos[2]*inc2_p +iPos[3]*inc3_p];};

    template <class T>
    void addTo4DArray(T* __restrict__ store,
		      const Int* __restrict__ iPos, 
		      Complex& nvalue, Double& wt) __restrict__ 
    {store[iPos[0] + iPos[1]*inc1_p + iPos[2]*inc2_p +iPos[3]*inc3_p] += (nvalue*wt);}
     */
};


/*
 * gridDegrid performs the actual grid or degrid operation.  A separate instantiation is used for
 * each grid/degrid direction as well as for the type of the grid.  Two of the template parameters take
 * "const" prefixes depending on the direction.  It is important that the right const/"nonconst" prefix
 * be "used" since that is what selects the inner loop method.
 *
 * The inner loop method is the overloaded method gridDegridInnerOperation.  This family of methods
 * contain the appropriate operation that is performed in the innermost loop of the grid or degrid.
 * This is simply a statement of the form "(* gridPointer) += weight * value" for gridding and
 * "value += weight * (* gridPointer) for degridding.  Overloading was chosen so that there would be
 * no conditional statements in the inner loop.  There are some direction-dependent logic in the gridDegrid
 * routine but these are outside of the inner loop and thus do not seriously impact performance.
 *
 * Template parameters:
 *
 * GridType - Array<SomeComplex> where SomeComplex is either Complex or DComplex
 * GridPointer - Pointer to an element of the grid (e.g., SomeComplex *).  It is important
 *               that a "const" prefix be provided for degridding.
 * ValueType - This is the type of the value that is being put into the grid or extracted
 *             from it.  This must have a "const" prefix for gridding and not be const for
 *             degridding.
 *
 * Examples:
 *
 * For the body of a gridder (e.g., inside method DataToGrid):
 *
 * gridDegrid<Array<Complex>, Complex *, const Complex> (Grid, griddedData, vbs, dopsf, & sumwt);
 *
 * For the body of a degridder method (e.g., inside method GridToData):
 *
 * gridDegrid<const Array<Complex>, const Complex *, Complex> (Degrid, grid, vbs);
 *
 *
 */



template <typename GridType, typename GridPointer, typename Value>
void VisibilityResampler::gridDegrid (GridOrDegrid gridOrDegrid,
                                      GridType & grid,
                                      VBStore & vbs,
                                      Bool dopsf,
                                      Matrix<Double> * sumwt)
{
    typedef typename GridType::value_type GridComplex; // Underlying elements
    typedef typename GridComplex::value_type Real;

    Bool visCubeDelete = False;
    Complex * __restrict__ visCube = vbs.visCube_p.getStorage(visCubeDelete);

    Int nDataChan, nDataPol, nGridPol, nGridChan, nx, ny;
    Int achan, apol, rbeg, rend;
    Vector<Float> sampling(2);
    Vector<Int> support(2),loc(2), off(2), iloc(2);
    Vector<Double> pos(2);

    Vector<Int> gridPosition(4);

    Double imgWt;
    GridComplex nvalue;
    Complex phasor;

    rbeg = vbs.beginRow_p;
    rend = vbs.endRow_p;
    //    cerr << rbeg << " " << rend << " " << vbs.nRow() << endl;
    nx = grid.shape()[0];
    ny = grid.shape()[1];
    nGridPol = grid.shape()[2];
    nGridChan = grid.shape()[3];

    nDataPol  = vbs.flagCube_p.shape()[0];
    nDataChan = vbs.flagCube_p.shape()[1];

    sampling[0] = sampling[1] = convFuncStore_p.sampling[0];
    support(0) = convFuncStore_p.xSupport[0];
    support(1) = convFuncStore_p.ySupport[0];

    Bool Dummy = False;
    Bool gridDelete = False;
    //const Int * __restrict__ iPosPtr = gridPosition.getStorage(Dummy);
    GridPointer gridStore = grid.getStorage(gridDelete);
    Double *__restrict__ convFunc=(*(convFuncStore_p.rdata)).getStorage(Dummy);
    Double * __restrict__ freq=vbs.freq_p.getStorage(Dummy);
    Bool * __restrict__ rowFlag=vbs.rowFlag_p.getStorage(Dummy);

    Float * __restrict__ imagingWeight = vbs.imagingWeight_p.getStorage(Dummy);
    Double * __restrict__ uvw = vbs.uvw_p.getStorage(Dummy);
    Bool * __restrict__ flagCube = vbs.flagCube_p.getStorage(Dummy);
    Double * __restrict__ scale = uvwScale_p.getStorage(Dummy);
    Double * __restrict__ offset = offset_p.getStorage(Dummy);
    Float * __restrict__ samplingPtr = sampling.getStorage(Dummy);
    Double * __restrict__ posPtr=pos.getStorage(Dummy);
    Int * __restrict__ locPtr=loc.getStorage(Dummy);
    Int * __restrict__ offPtr=off.getStorage(Dummy);
    //Double * __restrict__ sumwtPtr = (sumwt != NULL) ? sumwt->getStorage(Dummy) : NULL;
    Int nDim = vbs.uvw_p.shape()[0];

    // Precompute the grid increments for indexing into the 4D array.

    Vector<Int> gridShape = grid.shape().asVector();
    Vector<Int> gridIncrements (4);

    gridIncrements[0] = 1;
    for (int i = 1; i < 4; i++){
        gridIncrements [i] = gridIncrements[i-1] * gridShape[i-1];
    }

    // Setup a vector to hold the X and Y functions and provide pointers to access them quickly.
    // They are functions of the channel so the actual filling of the arrays must occur side the
    // channel loop.  The current computation model assumes that convolution weight can be computed
    // as the produce the the X component and the Y component each determined independently.

    Vector<Real> convolutionLookupX (2 * support[0] + 1, 0.0);
    Vector<Real> convolutionLookupY (2 * support[1] + 1, 0.0);
    const Real * const pConvolutionLookupY0 = convolutionLookupY.getStorage (Dummy);
    const Real * const pConvolutionLookupX0 = convolutionLookupX.getStorage (Dummy);
    const Real * const pConvolutionLookupXEnd = pConvolutionLookupX0 + convolutionLookupX.size();

    for(Int irow=rbeg; irow < rend; irow++){          // For all rows

        if(rowFlag[irow])
            continue; // skip if the row is not flagged

        for(Int ichan=0; ichan< nDataChan; ichan++){ // For all channels

            if (gridOrDegrid == Grid && imagingWeight[ichan+irow*nDataChan]==0.0)
                continue;  // Skip if weights are zero

            achan=chanMap_p[ichan];

            if((achan<0) || (achan>=nGridChan))
                continue; // skip if selected channels are invalid

            sgrid(nDim,posPtr,locPtr,offPtr, phasor, irow, uvw,dphase_p[irow], freq[ichan],
                  scale, offset, samplingPtr);

            if (! onGrid(nx, ny, loc, support))
                continue;  // skip if the data co-ords. are outside the grid

            // Compute the two convolution function vectors.  Also sum up the vector
            // elements to use in computing the normalization factor.

            Double convolutionSumX = 0;
            for (int ix = - support [0], ii = 0; ix <= support [0]; ix ++, ii++){
                Int iConv = abs(int(sampling[0] * ix + off[0]));
                convolutionLookupX [ii] = convFunc[iConv];
                convolutionSumX += convFunc[iConv];
            }

            Double convolutionSumY= 0;
            for (int iy = - support [1], ii = 0; iy <= support [1]; iy ++, ii++){
                Int iConv = abs(int(sampling[1] * iy + off[1]));
                convolutionLookupY [ii] = convFunc[iConv];
                convolutionSumY += convFunc[iConv];
            }

            // Compute the normalization factor as the product of the sums of each
            // axis's convolution vector (sum (sum (conv[y]conv[x], x=-X to X), y=-Y to Y))
            // separates to sum (conv[x], x=-X to X) * sum (conv[y], y=-Y to Y)

            Real norm = convolutionSumX * convolutionSumY;

            for(Int ipol=0; ipol< nDataPol; ipol++) { // For all polarizations

                if((flagCube[ipol+ichan*nDataPol+irow*nDataChan*nDataPol]))
                    continue; //skip if data is flagged

                apol=polMap_p(ipol);
                if ((apol<0) || (apol>=nGridPol))
                    continue; // skip if polarization is out of range

                gridPosition[2]=apol;
                gridPosition[3]=achan;

                // For gridding, extract and weight the appropriate element of the visibility cube
                // and put it in "nvalue". For degridding, "nvalue" will be calculated so initialize
                // it to zero.

                if (gridOrDegrid == Grid){

                    imgWt=imagingWeight[ichan+irow*nDataChan];

                    if (dopsf){
                        nvalue=Complex(imgWt);
                    }
                    else{
                        nvalue=imgWt*(visCube[ipol+ichan*nDataPol+irow*nDataPol*nDataChan]*phasor);
                    }
                }
                else { // Degrid

                    nvalue = 0;

                }

                // The "Inner Loops" and its initialization starts here
                // ====================================================

                // Define the indices for the four different axes

                const Int X = 0;
                const Int Y = 1;
                const Int Z1 = 2; // polarization
                const Int Z2 = 3; // channel

                Int gridZ1 = gridPosition [Z1]; // Third grid coordinate, Z1
                Int gridZ2 = gridPosition [Z2]; // Fourth grid coordinate, Z2

                GridPointer gridStoreZ1Z2 =
                        gridStore + gridZ1 * gridIncrements [Z1] + gridZ2 * gridIncrements [Z2];
                // Position of origin of xy plane specified by Z1, Z2

                GridPointer gridStoreYZ1Z2 = gridStoreZ1Z2 + gridIncrements [Y] * (loc[Y] - support[Y] - 1);
                // Position of origin of lower left corner of rectangle
                // of XY plane used in convolution

                // The inner loop is spreading "nvalue" over a small 2D rectangle of the plane
                // appropriate for the polarization and channel (i.e., a 2D slice of the grid hypercube).
                // The 2D convolution rectangle is approximately centered around the point being gridded and
                // ranges from y +/- Support[Y] and x +/- Support[X] where the support values are
                // positive integers.

                const Int yIncrement = gridIncrements [Y]; // increment to move to next row
                const Int x0 = (loc[X] - support[X]) - 1; // x-coord of first point in conv subgrid
                const Int yMax = support[Y];
                const Int nY = yMax * 2 + 1;              // Number of grid points in the Y axis
                const Real * pConvolutionLookupY = pConvolutionLookupY0;
                                                          // Point to the first element of the Y
                                                          // convolution lookup vector.

                Value & value = nvalue;  // Reference the value using the type specified by the
                                         // template parameter.  "value" has the same type as
                                         // "nvalue" but will be also be "const" in gridding
                                         // operations.  N.B.: This alias is necessary to ensure that
                                         // the proper overloaded grid inner operation method
                                         // is selected by the compiler.

                // Initialization done, enter the "Inner Loops"

                for(Int iy=0; iy < nY ; iy ++)
                {
                    const Real convFuncY = * pConvolutionLookupY ++;

                    gridStoreYZ1Z2 += yIncrement;
                    GridPointer gridStoreXYZ1Z2 = gridStoreYZ1Z2 + x0;

                    for (const Real * pConvolutionLookupX = pConvolutionLookupX0;
                         pConvolutionLookupX != pConvolutionLookupXEnd;
                         pConvolutionLookupX ++){

                        Real weight = (* pConvolutionLookupX) * convFuncY;

                        gridStoreXYZ1Z2 += 1;
                        gridDegridInnerOperation (gridStoreXYZ1Z2, weight, value);

                    }
                }

                // The "Inner Loops" are completed!

                if (gridOrDegrid == Grid){
                    (* sumwt) (apol,achan) += imgWt*norm;
                }
                else{ // Degrid
                    visCube[ipol+ichan*nDataPol+irow*nDataChan*nDataPol] = Complex (nvalue)*conj(phasor)/norm;
                }

            } // end polarization
        } // end channel
    } // end row

    Complex * tmp2 = (Complex *) visCube;
    vbs.visCube_p.putStorage (tmp2,visCubeDelete);
}


}; //# NAMESPACE CASA - END

#endif // 


