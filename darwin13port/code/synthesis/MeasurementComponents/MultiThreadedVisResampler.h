// -*- C++ -*-
//# MultiThreadedVisResampler.h: Definition of the MultiThreadedVisibilityResampler class
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

#ifndef SYNTHESIS_MULTITHREADEDVISIBILITYRESAMPLER_H
#define SYNTHESIS_MULTITHREADEDVISIBILITYRESAMPLER_H

#include <synthesis/TransformMachines/VisibilityResamplerBase.h>
#include <synthesis/MeasurementComponents/ResamplerWorklet.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/TransformMachines/VBStore.h>
#include <synthesis/MeasurementComponents/MThWorkIDEnum.h>
#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogMessage.h>
#define DEFAULTNOOFCORES -1
#define FTMachineNumThreadsEnvVar "ftmachine_num_threads"
namespace casa { //# NAMESPACE CASA - BEGIN
  class MultiThreadedVisibilityResampler: public VisibilityResamplerBase
  {
  public: 
    //    enum VBLoader {NOONE=-1, DATATOGRID=0, GRIDTODATA, RESIDUALCALC};
     MultiThreadedVisibilityResampler():
      resamplers_p(), doubleGriddedData_p(), singleGriddedData_p(), 
      sumwt_p(), gridderWorklets_p(), vbsVec_p(), visResamplerCtor_p(), 
      whoLoadedVB_p(MThWorkID::NOONE), currentVBS_p(0)
    {
      nelements_p = SynthesisUtils::getenv(FTMachineNumThreadsEnvVar, 1);
      if (nelements_p < 0) nelements_p = 1;
      //      nelements_p=DEFAULTNOOFCORES;
    };

    MultiThreadedVisibilityResampler(const Bool& doublePrecision, 
				     const Int& n=DEFAULTNOOFCORES);

    MultiThreadedVisibilityResampler(const Bool& doublePrecision, 
				     CountedPtr<VisibilityResamplerBase>& visResampler, 
				     const Int& n=DEFAULTNOOFCORES);
    virtual ~MultiThreadedVisibilityResampler()
    {
      cleanup();
      nelements_p=0;
      currentVBS_p=0;
    }

    MultiThreadedVisibilityResampler(const MultiThreadedVisibilityResampler& other) : VisibilityResamplerBase() {copy(other);};

    // This version will make a clone with shared data buffers (the
    // complex grids and the sum-of-weights arrays).
    MultiThreadedVisibilityResampler* clone()
    {
      //Allocate a new instance, and copy the internals.
      MultiThreadedVisibilityResampler *clonedCopy;
      clonedCopy = new MultiThreadedVisibilityResampler(*this);
      // Now reset the data buffers with independent buffers (arrays of size 0)
      clonedCopy->allocateDataBuffers();
      return clonedCopy;
    };

    MultiThreadedVisibilityResampler& operator=(const MultiThreadedVisibilityResampler& other);

    void cleanup();
    void releaseBuffers();

    void copy(const MultiThreadedVisibilityResampler& other);

    virtual Int nelements() {return nelements_p;};
    virtual void setParams(const Vector<Double>& uvwScale, const Vector<Double>& offset,
			   const Vector<Double>& dphase);

    virtual void setMaps(const Vector<Int>& chanMap, const Vector<Int>& polMap);
    virtual void setCFMaps(const Vector<Int>& cfMap, const Vector<Int>& conjCFMap);
    virtual void setFreqMaps(const Matrix<Double>& spwChanFreqs, const Matrix<Double>& spwChanConjFreqs);

    virtual void setConvFunc(const CFStore& cfs);
    void init(const Bool& doublePrecision);
    //
    //------------------------------------------------------------------------------
    //
    // Re-sample the griddedData on the VisBuffer (a.k.a gridding).
    //
    // In this class, these just call the private templated version.
    // The first variant grids onto a double precision grid while the
    // second one does it on a single precision grid.
    //
    virtual void DataToGrid(Array<DComplex>& griddedData,  
			    VBStore& vbs, 
			    Matrix<Double>& sumwt,
			    const Bool& dopsf,Bool /*useConjFreqCF*/=False)
    {DataToGridImpl_p(griddedData, vbs, sumwt,dopsf);}

    virtual void DataToGrid(Array<Complex>& griddedData, 
			    VBStore& vbs, 
    			    Matrix<Double>& sumwt,
			    const Bool& dopsf,Bool /*useConjFreqCF*/=False)
    {
      DataToGridImpl_p(griddedData, vbs, sumwt,dopsf);
    }

    //
    // This is the gather operation on griddedData for nelements > 1
    //
    void GatherGrids(Array<DComplex>& griddedData, Matrix<Double>& sumwt);
    void GatherGrids(Array<Complex>& griddedData, Matrix<Double>& sumwt);
    void initializePutBuffers(const Array<DComplex>& griddedData,
			      const Matrix<Double>& sumwt);
    void initializePutBuffers(const Array<Complex>& griddedData,
			      const Matrix<Double>& sumwt);
    void initializeDataBuffers(VBStore& /*vbs*/) {};

    //
    //------------------------------------------------------------------------------
    //
    // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
    //
    virtual void GridToData(VBStore& vbs, const Array<Complex>& griddedData); 
    //    virtual void GridToData(VBStore& vbs, Array<Complex>& griddedData); 

    virtual void ComputeResiduals(VBStore& vbs);
    //
    //------------------------------------------------------------------------------
    //----------------------------Private parts-------------------------------------
    //------------------------------------------------------------------------------
    //
  private:
    template <class T>
    void DataToGridImpl_p(Array<T>& griddedData, VBStore& vb,  
			  Matrix<Double>& sumwt,const Bool& dopsf,Bool useConjFreqCF=False);
    // template <class T>
    // void GatherGrids_p(const Array<T>& griddedData, 
    // 		       const Matrix<Double>& sumwt);

    // template <class T>
    // void initializeBuffers(const Array<T>& griddedData,
    // 			   const Matrix<Double>& sumwt);

    void allocateBuffers(Bool newDataBuffers=True);
    void makeInfrastructureContainers();
    Double allocateDataBuffers();
    void startThreads();
    void scatter(Matrix<VBStore>& vbsStores,const VBStore& vbs);

    Int nelements_p;
    Bool doublePrecision_p;
    Vector<CountedPtr<VisibilityResamplerBase> > resamplers_p;
    Vector<CountedPtr<Array<DComplex> > > doubleGriddedData_p;
    Vector<CountedPtr<Array<Complex> > > singleGriddedData_p;
    Vector<CountedPtr<Matrix<Double> > > sumwt_p;
    Vector<CountedPtr<ResamplerWorklet> > gridderWorklets_p;
    //    Vector<VBStore> vbsVec_p;
    Matrix<VBStore> vbsVec_p;

    CountedPtr<ThreadCoordinator<Int> > threadClerk_p;
    Bool threadStarted_p;
    casa::utilj::DeltaThreadTimes tSetupG, tSendDataG, tWaitForWorkG, tOutsideG;
    casa::utilj::DeltaThreadTimes tSetupDG, tSendDataDG, tWaitForWorkDG, tOutsideDG;
    casa::utilj::ThreadTimes t4G_p,t4DG_p;
    //    async::Mutex *mutexForResamplers_p;
    CountedPtr<VisibilityResamplerBase> visResamplerCtor_p;
    Int whoLoadedVB_p;
    Int currentVBS_p;
 };
}; //# NAMESPACE CASA - END

#endif // 
