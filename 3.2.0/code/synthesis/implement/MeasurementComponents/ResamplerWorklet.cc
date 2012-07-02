// -*- C++ -*-
//# ResamplerWorklet.cc: Implementation of the ResamplerWorklet class
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
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/VisibilityResampler.h>
#include <synthesis/MeasurementComponents/ResamplerWorklet.h>
#include <synthesis/Utilities/ThreadCoordinator.h>
#include <fstream>
#include <sys/syscall.h>

namespace casa{
  ResamplerWorklet::ResamplerWorklet() {}
  ResamplerWorklet& ResamplerWorklet::operator=(const ResamplerWorklet& other)
  {
    myResampler_p = other.myResampler_p;
    myVBStore_p = other.myVBStore_p;
    myGriddedDataDouble_p = other.myGriddedDataDouble_p;
    myGriddedDataSingle_p = other.myGriddedDataSingle_p;
    mySumWt_p = other.mySumWt_p;
    myThreadClerk_p = other.myThreadClerk_p;
    myID_p = other.myID_p;
    myPID_p = other.myPID_p;
    myTID_p = other.myTID_p;
    mySkyFTGrid_p = other.mySkyFTGrid_p;
    return *this;
  }
  void ResamplerWorklet::init(Int& id, 
			    ThreadCoordinator<Bool>* threadClerk,
			    VisibilityResampler* resampler, VBStore* vbs,
			    Array<DComplex>* griddedData, Matrix<Double>* sumwt,
			    Array<Complex>* skyFTGrid)
    {
      LogIO log_p(LogOrigin("ResamplerWorklet","init (double precision)"));

      setID(id);
      myThreadClerk_p = threadClerk;
      myResampler_p = resampler;
      myVBStore_p = vbs;
      myGriddedDataDouble_p = griddedData;
      mySumWt_p = sumwt;
      mySkyFTGrid_p = skyFTGrid;
      //      log_p << "Gridder worklet # " << id << " ready." << LogIO::POST;
    };
  void ResamplerWorklet::init(Int& id, 
			    ThreadCoordinator<Bool>* threadClerk,
			    VisibilityResampler* resampler, VBStore* vbs,
			    Array<Complex>* griddedData, Matrix<Double>* sumwt,
			    Array<Complex>* skyFTGrid)
    {
      LogIO log_p(LogOrigin("ResamplerWorklet","init (single precision)"));

      setID(id);
      myThreadClerk_p = threadClerk;
      myResampler_p = resampler;
      myVBStore_p = vbs;
      myGriddedDataSingle_p = griddedData;
      mySumWt_p = sumwt;
      mySkyFTGrid_p = skyFTGrid;
      //      log_p << "Gridder worklet # " << id << " ready." << LogIO::POST;
    };

  void ResamplerWorklet::setPID()
  {
    LogIO log_p(LogOrigin("ResamplerWorklet","setPID"));
    myTID_p = gettid_p (); myPID_p = getpid ();
    log_p << "Worklet # " << myID_p << " Activated. " 
    	  << "PID = " << myPID_p << " TID = " << myTID_p
    	  << LogIO::POST;
  }
  void* ResamplerWorklet::run()
  {
    LogIO log_p(LogOrigin("ResamplerWorklet","run"));

    setPID();

    while(True)
      {
	Bool *doDataToGrid;
	doDataToGrid = myThreadClerk_p->waitForWork(this);
	if (doDataToGrid == NULL) break;
	if (*doDataToGrid==True)
	  myResampler_p->DataToGrid(*myGriddedDataDouble_p, *myVBStore_p, 
				    *mySumWt_p, myVBStore_p->dopsf_p);
	else 
	  myResampler_p->GridToData(*myVBStore_p, *mySkyFTGrid_p);
      }
    log_p << "Alveda from Workelet # " << myID_p << LogIO::POST;
    return NULL;
  }
};
