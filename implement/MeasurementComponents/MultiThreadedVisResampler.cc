// -*- C++ -*-
//# MultiThreadedVisResample.cc: Implementation of the MultiThreadedVisibilityResampler class
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
#include <synthesis/Utilities/ThreadCoordinator.h>
#include <synthesis/Utilities/ThreadTimers.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/VisibilityResampler.h>
#include <synthesis/MeasurementComponents/MultiThreadedVisResampler.h>
#include <synthesis/MeasurementComponents/ResamplerWorklet.h>
#include <fstream>

namespace casa{
  // template 
  // void MultiThreadedVisibilityResampler::DataToGridImpl_p(Array<Complex>& griddedData,  
  // 						      VBStore& vbs, 
  // 						      Matrix<Double>& sumwt,
  // 						      const Bool& dopsf);
  template 
  void MultiThreadedVisibilityResampler::DataToGridImpl_p(Array<DComplex>& griddedData,  
						      VBStore& vbs, 
						      Matrix<Double>& sumwt,
						      const Bool& dopsf);

  MultiThreadedVisibilityResampler::MultiThreadedVisibilityResampler(const Bool& doublePrecision,
								     const VisibilityResampler& visResampler, 
								     const Int& n):
    resamplers_p(), doubleGriddedData_p(), singleGriddedData_p(), sumwt_p(), gridderWorklets_p(), 
    vbsVec_p(), threadClerk_p(NULL),threadStarted_p(False)
    {
      if (n < 0) nelements_p = SynthesisUtils::getenv(FTMachineNumThreadsEnvVar, n);
      if (nelements_p < 0) nelements_p = 1;
      init(doublePrecision);
      t4G_p=Timers::getTime();
      t4DG_p=Timers::getTime();
    }
  MultiThreadedVisibilityResampler::MultiThreadedVisibilityResampler(const Bool& doublePrecision,
							     const Int& n):
    resamplers_p(), doubleGriddedData_p(), singleGriddedData_p(), sumwt_p(), gridderWorklets_p(), 
    vbsVec_p(), threadClerk_p(),threadStarted_p(False)
    {
      if (n < 0) nelements_p = SynthesisUtils::getenv(FTMachineNumThreadsEnvVar, n);
      if (nelements_p < 0) nelements_p = 1;
      init(doublePrecision);
      t4G_p=Timers::getTime();
      t4DG_p=Timers::getTime();
    }

  MultiThreadedVisibilityResampler& MultiThreadedVisibilityResampler::operator=(const MultiThreadedVisibilityResampler& other)
    {
      nelements_p = other.nelements_p;
      doublePrecision_p=other.doublePrecision_p;
      SynthesisUtils::SETVEC(resamplers_p,other.resamplers_p);
      // SynthesisUtils::SETVEC(doubleGriddedData_p, other.doubleGriddedData_p);
      // SynthesisUtils::SETVEC(singleGriddedData_p,other.singleGriddedData_p);
      // SynthesisUtils::SETVEC(sumwt_p, other.sumwt_p);
      doubleGriddedData_p.reference(other.doubleGriddedData_p);
      singleGriddedData_p.reference(other.singleGriddedData_p);
      sumwt_p.reference(other.sumwt_p);
      SynthesisUtils::SETVEC(gridderWorklets_p, other.gridderWorklets_p);
      SynthesisUtils::SETVEC(vbsVec_p, other.vbsVec_p);
      threadClerk_p = other.threadClerk_p;
      threadStarted_p = other.threadStarted_p;
      t4G_p=other.t4G_p;
      t4DG_p=other.t4DG_p;
      return *this;
  }

  void MultiThreadedVisibilityResampler::init(const Bool& doublePrecision)
  {
    LogIO log_p(LogOrigin("MultiThreadedVisibilityResampler","init"));
    doublePrecision_p=doublePrecision;
    allocateBuffers();
  }

  void MultiThreadedVisibilityResampler::cleanup()
  {
    if (nelements() > 1)
      threadClerk_p->getToWork(NULL); // Signal the threads to quit
    //    delete threadClerk_p; threadClerk_p=NULL;
    vbsVec_p.resize(0);
    resamplers_p.resize(0);
    gridderWorklets_p.resize(0);
    sumwt_p.resize(0);
    doubleGriddedData_p.resize(0);
    singleGriddedData_p.resize(0);
    delete mutexForResamplers_p;
  }

  void MultiThreadedVisibilityResampler::allocateBuffers()
  {
    LogIO log_p(LogOrigin("MultiThreadedVisibilityResampler","allocateBuffers"));
    Int totalMem=0;
    if (nelements() > 1)
      {
	log_p << "Allocating buffers per thread.  No. of threads = " << nelements() << endl;
	mutexForResamplers_p = new async::Mutex;

	if (doublePrecision_p)  doubleGriddedData_p.resize(nelements());
	else                    singleGriddedData_p.resize(nelements());

	resamplers_p.resize(nelements());
	sumwt_p.resize(nelements());
	gridderWorklets_p.resize(nelements());
	vbsVec_p.resize(nelements());
	if (threadClerk_p.null()) threadClerk_p = new ThreadCoordinator<Int>(nelements());
	for (Int i=0;i<nelements();i++)
	  {
	    resamplers_p[i] = new VisibilityResampler();
	    resamplers_p[i]->setMutex(mutexForResamplers_p);
	    gridderWorklets_p[i] = new ResamplerWorklet();
	    if (doublePrecision_p) doubleGriddedData_p[i] = new Array<DComplex>;
	    else	           singleGriddedData_p[i] = new Array<Complex>;
	    sumwt_p[i] = new Matrix<Double>;
	    if (!threadStarted_p)
	      {
		if (doublePrecision_p) totalMem += (*doubleGriddedData_p[i]).size()*sizeof(DComplex);
		else totalMem += (*singleGriddedData_p[i]).size()*sizeof(Complex);
		totalMem += (*sumwt_p[i]).size()*sizeof(Double);
		// if (doublePrecision_p)
		//   (*gridderWorklets_p[i]).init(i, threadClerk_p,
		// 			       &(*resamplers_p[i]),
		// 			       &vbsVec_p[i], &(*doubleGriddedData_p[i]),
		// 			       &(*sumwt_p[i]));
		// else
		//   (*gridderWorklets_p[i]).init(i, threadClerk_p,
		// 			       &(*resamplers_p[i]),
		// 			       &vbsVec_p[i], &(*singleGriddedData_p[i]),
		// 			       &(*sumwt_p[i]));
		  
		(*gridderWorklets_p[i]).initThread(i, threadClerk_p, &(*resamplers_p[i]));
		(*gridderWorklets_p[i]).startThread();
	      }
	  }
	threadStarted_p=True;
      }
    else
      {
	resamplers_p.resize(1);
	resamplers_p[0] = new VisibilityResampler();
	vbsVec_p.resize(1);
      }
    if (totalMem > 0)
      log_p << "Total memory used in buffers for multi-threading: " << totalMem/(1024*1024) << " MB" << LogIO::POST;
  }

  void MultiThreadedVisibilityResampler::setParams(const Vector<Double>& uvwScale, 
						   const Vector<Double>& offset,
						   const Vector<Double>& dphase)
  {for (Int i=0;i < nelements(); i++) resamplers_p[i]->setParams(uvwScale, offset, dphase);}

  void MultiThreadedVisibilityResampler::setMaps(const Vector<Int>& chanMap, 
					     const Vector<Int>& polMap)
  {for (Int i=0;i < nelements(); i++) resamplers_p[i]->setMaps(chanMap, polMap);};

  void MultiThreadedVisibilityResampler::setConvFunc(const CFStore& cfs)
  {for (Int i=0;i < nelements(); i++) resamplers_p[i]->setConvFunc(cfs);};
  
  void MultiThreadedVisibilityResampler::scatter(Vector<VBStore>& vbStores,const VBStore& vbs)
  {
    Int nRows=vbs.nRow_p, nr,br=0;
    nr=(Int)(nRows/nelements()+0.5);

    for(Int i=0; i < nelements(); i++)
      {
	vbStores[i].reference(vbs);
	vbStores[i].beginRow_p = min(br,nRows); 
	vbStores[i].endRow_p   = min(br+nr,nRows);
	br = vbStores[i].endRow_p+1;
      }
    // cerr << "Total number of rows = " << nRows << endl;
    // for(Int i=0; i < nelements(); i++)
    //   cerr << i << " " << vbStores[i].endRow_p -vbStores[i].beginRow_p << endl;
  }

  void MultiThreadedVisibilityResampler::GatherGrids(Array<DComplex>& griddedData,
						 Matrix<Double>& sumwt)
  {
    LogIO log_p(LogOrigin("MultiThreadedVisibilityResampler","GatherGrids"));
    if (nelements() > 1)
      {
	//	log_p << "Deleting thread clerk" << LogIO::POST;
	//	delete threadClerk_p; threadClerk_p=NULL;
	log_p << "Gathering grids..." << LogIO::POST;
	// cerr << "Gridded data shape = " << griddedData.shape() << " " << sumwt.shape() << endl;
	// for (Int i=0;i<nelements();i++)
	//   cerr << "Gridded buffer shape = " 
	//        << (*doubleGriddedData_p[i]).shape() << " " 
	//        << (*sumwt_p[i]).shape() << endl;
	for(Int i=0;i<nelements(); i++)
	  {
	    griddedData += *(doubleGriddedData_p[i]);
	    sumwt += *(sumwt_p[i]);
	  }
      }
    log_p << "Gridder timing: " 
	  << "Setup = " << tSetupG.formatAverage().c_str() << " " 
	  << "SendData = " << tSendDataG.formatAverage().c_str() << " " 
	  << "WaitForWork = " << tWaitForWorkG.formatAverage().c_str() 
	  << "Outside = " << tOutsideG.formatAverage().c_str() 
	  << LogIO::POST;
    log_p << "DGridder timing: "
	  << "Setup = " << tSetupDG.formatAverage().c_str() << " " 
	  << "SendData = " << tSendDataDG.formatAverage().c_str() << " " 
	  << "WaitForWork = " << tWaitForWorkDG.formatAverage().c_str() 
	  << "Outside = " << tOutsideDG.formatAverage().c_str() 
	  << LogIO::POST;
  }

  void MultiThreadedVisibilityResampler::GatherGrids(Array<Complex>& griddedData,
						 Matrix<Double>& sumwt)
  {
    if (nelements() > 1)
      {
	for(Int i=0;i<nelements(); i++)
	  {
	    griddedData += *singleGriddedData_p[i];
	    sumwt += *sumwt_p[i];
	  }
      }
  }
  void MultiThreadedVisibilityResampler::initializePutBuffers(const Array<DComplex>& griddedData,
							  const Matrix<Double>& sumwt)
  {
    LogIO log_p(LogOrigin("MultiThreadedVisibilityResampler","initializeBuffers"));
    if ((nelements() > 1))
      {
    	//	if (threadClerk_p) {delete threadClerk_p; threadClerk_p=NULL;}
    	Double totalMem=0;
    	// if (!threadClerk_p) threadClerk_p = new ThreadCoordinator<Int>(nelements());
    	for(Int i=0; i<nelements(); i++)
    	  {
    	    // Resize and copy The following commented code attempts
	    // to defeat the problem of false sharing, should it be
	    // the bottlenec.
	    // {
	    //   Vector<Array<DComplex>* > tmp(5);
	    //   for (Int j=0;j<5;j++) 
	    // 	{
	    // 	  (tmp[j]) = new Array<DComplex>;
	    // 	  (tmp[j])->assign(griddedData);
	    // 	}
	    //   (*doubleGriddedData_p[i]).assign(griddedData);
	    //   for (Int j=0;j<5;j++) delete tmp[j];
	    // }

	    // The following code relies on system memalloc
	    (*doubleGriddedData_p[i]).assign(griddedData);
    	    (*sumwt_p[i]).assign(sumwt);
	    if (!threadStarted_p)
	      {
		totalMem += (*doubleGriddedData_p[i]).size()*sizeof(DComplex);
		totalMem += (*sumwt_p[i]).size()*sizeof(Double);
		// (*gridderWorklets_p[i]).init(i, threadClerk_p,
		// 			     &(*resamplers_p[i]),
		// 			     &vbsVec_p[i], &(*doubleGriddedData_p[i]),
		// 			     &(*sumwt_p[i]));
		// (*gridderWorklets_p[i]).startThread();
	      }
    	  }
    	if (!threadStarted_p) 
	  log_p << "Total memory used in buffers:" << totalMem/(1024*1024) << " MB" << LogIO::POST;
	// for (Int i=0;i<nelements(); i++)
	//   log_p << "Activating worklet " 
	// 	<< "# "     << (*gridderWorklets_p[i]).getID()  << ". " 
	// 	<< "PID = " << (*gridderWorklets_p[i]).getPID() << " "
	// 	<< "TID = " << (*gridderWorklets_p[i]).getTID() 
	// 	<< LogIO::POST;
	//	threadStarted_p = True;
      }
  }
  void MultiThreadedVisibilityResampler::initializePutBuffers(const Array<Complex>& griddedData,
							  const Matrix<Double>& sumwt)
  {
    if (nelements() > 1)
      {
	for(Int i=0; i<nelements(); i++)
	  {
	    (*singleGriddedData_p[i]).assign(griddedData);
	    (*sumwt_p[i]).assign(sumwt);
	  }
      }
  }
  //
  //------------------------------------------------------------------------------
  //
  // Re-sample the griddedData on the VisBuffer (a.k.a gridding).
  //
  // Make the following four methods via templated implementation.

  
  template <class T>
  void MultiThreadedVisibilityResampler::DataToGridImpl_p(Array<T>& griddedData,  
						      VBStore& vbs, 
						      Matrix<Double>& sumwt,
						      const Bool& dopsf)
  {
    //    LogIO log_p(LogOrigin("MultiThreadedVisibilityResampler", "DataToGridImpl_p"));
    scatter(vbsVec_p,vbs);

    if (nelements() == 1)
	resamplers_p[0]->DataToGrid(griddedData, vbsVec_p[0], sumwt, dopsf);
    else
      {
	//	Bool workRequestDataToGrid=True;
	Int workRequestDataToGrid=1;
	Timers t1=Timers::getTime();
	for(Int i=0; i < nelements(); i++) 
	  {
	    vbsVec_p[i].dopsf_p = dopsf;
	    if (doublePrecision_p) (*gridderWorklets_p[i]).initToSky(&vbsVec_p[i], &(*doubleGriddedData_p[i]),
								     &(*sumwt_p[i]));
	    else                    (*gridderWorklets_p[i]).initToSky(&vbsVec_p[i], &(*singleGriddedData_p[i]),
								      &(*sumwt_p[i]));
	  }

	Timers t2=Timers::getTime();
	//	threadClerk_p->getToWork(&workRequestDataToGrid);
	threadClerk_p->giveWorkToWorkers(&workRequestDataToGrid);
	Timers t3=Timers::getTime();
	threadClerk_p->waitForWorkersToFinishTask();
	Timers t4=Timers::getTime();

	tSetupG       += t2-t1;
	tSendDataG    += t3-t2;
	tWaitForWorkG += t4-t3;
	tOutsideG     += t1-t4G_p;
      }
    t4G_p = Timers::getTime();
  }
  //
  //------------------------------------------------------------------------------
  //
  // Re-sample VisBuffer to a regular grid (griddedData) (a.k.a. de-gridding)
  // Still single threaded...
  void MultiThreadedVisibilityResampler::GridToData(VBStore& vbs, Array<Complex>& griddedData) 
  {
    //    LogIO log_p(LogOrigin("MultiThreadedVisibilityResampler", "GridToData"));
    scatter(vbsVec_p,vbs);

    if (nelements() == 1)
      resamplers_p[0]->GridToData(vbsVec_p[0],griddedData);
    else
      {
	//    	Bool workRequestDataToGrid=False;
    	Int workRequestDataToGrid=0;
	Timers t1=Timers::getTime();
	for(Int i=0; i < nelements(); i++) 
	  {
	    (*gridderWorklets_p[i]).initToVis(&vbsVec_p[i],&griddedData);
	    // Need to break-up the init into things that required to
	    // be init'ed for gridding and de-gridding.
	    // (*gridderWorklets_p[i]).init(i, threadClerk_p,
	    // 				 &(*resamplers_p[i]),
	    // 				 &vbsVec_p[i], &(*doubleGriddedData_p[i]),
	    // 				 &(*sumwt_p[i]),&griddedData);
	  }
	Timers t2=Timers::getTime();
	//	threadClerk_p->getToWork(&workRequestDataToGrid);
	threadClerk_p->giveWorkToWorkers(&workRequestDataToGrid);
	Timers t3=Timers::getTime();
	threadClerk_p->waitForWorkersToFinishTask();
	Timers t4=Timers::getTime();
	tSetupDG += t2-t1;
	tSendDataDG += t3-t2;
	tWaitForWorkDG += t4-t3;
	tOutsideDG += t1-t4DG_p;
      }
    t4DG_p = Timers::getTime();
  }
  //
  //------------------------------------------------------------------------------
  //
  void MultiThreadedVisibilityResampler::ComputeResiduals(VBStore& vbs)
  {
    scatter(vbsVec_p,vbs);
    if (nelements() == 1)
      resamplers_p[0]->ComputeResiduals(vbsVec_p[0]);
    else
      {
	Int workRequested=2;
	for (Int i=0; i<nelements(); i++)
	  (*gridderWorklets_p[i]).initToVis(&vbsVec_p[i],NULL);
	threadClerk_p->giveWorkToWorkers(&workRequested);
	threadClerk_p->waitForWorkersToFinishTask();
      }
  }

};
