//# FlagReport.h: This file contains the interface definition of the FlagReport class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef SYNTHESIS_SILOOPCONTROLLER
#define SYNTHESIS_SILOOPCONTROLLER

// .casarc interface
#include <casa/System/AipsrcValue.h>

// Records interface
#include <casa/Containers/Record.h>

// System utilities (for profiling macros)
#include <casa/OS/HostInfo.h>
#include <sys/time.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class SIIterBot : public Record
  {
  public:
    SIIterBot();
    SIIterBot(Record &other);
    ~SIIterBot();
    
    Bool verifyFields();

    void incrementMajorCycleCount();
    void incrementMinorCycleCount();
    //    void updatePeakResidual( Float newpeakresidual );
    void checkStop();
    Bool checkMinorStop( Int iters , Float currentpeakresidual );

    void calculateCycleThreshold();
    Float getCycleThreshold();

    Float getLoopGain();
    Int getRemainingNiter();
    Int getCompletedNiter();

    Int getMaxCycleNiter();

    Float getPeakResidual();
    void setPeakResidual(Float peakresidual);

    Float getModelFlux();
    void setModelFlux(Float modelflux);

    void setPsfSidelobe( Float maxpsfsidelobe );

    Bool getIsModelUpdated();
    void setIsModelUpdated(Bool updatedmodel);

    void addSummaryMinor(Int mapperid, Float model, Float peakresidual);
    void addSummaryMajor();

    //// START /// Functions for runtime parameter modification
    void changeCycleThreshold( Float cyclethreshold );
    void changeThreshold( Float threshold );
    void changeNiter( Int niter );
    void changeMaxCycleNiter( Int maxcycleniter );
    void changeLoopGain(  Float loopgain );
    //// END /// Functions for runtime parameter modification


  protected:

    
    
  private:
  };
  
  
  
  
} //# NAMESPACE CASA - END

#endif /* FLAGDATAHANDLER_H_ */
