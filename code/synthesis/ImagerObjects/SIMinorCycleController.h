//# SISubterBot.h: This file contains the interface definition SISubIterBot class
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

#ifndef SYNTHESIS_MINORCYCLECONTROLLER
#define SYNTHESIS_MINORCYCLECONTROLLER

// .casarc interface
#include <casa/System/AipsrcValue.h>

// System utilities (for profiling macros)
#include <casa/OS/HostInfo.h>
#include <sys/time.h>


// Boost Libraries for mutex and noncopyable semantics
//#include <boost/thread/recursive_mutex.hpp>
#include <boost/utility.hpp>

// Include files for the DBus Service
//#include <casadbus/interfaces/SynthImager.adaptor.h>

/* Future Decl */
class casa::Record;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class SIMinorCycleController : boost::noncopyable 
 {
  public:
    SIMinorCycleController();

    virtual ~SIMinorCycleController();

    /* Method to call to determine if a major cycle is required, true if
         - exceeded number maxcycleniter
         - exceeded cycle threshold
    */
    virtual bool majorCycleRequired(Float currentPeakResidual);

    /* This method sets the following controls for the Minor Cycle Controller:
       - niter
       - cycleniter
       - interactiveniter?
       - threshold
       - cyclethreshold
       - interactive theshold?
       - loop gain
       - cycle factor
    */
    void setCycleControls(Record& );

    /* Get Cycle Execution Data:
       Returns information about the set of cycles just executed:
       - iterdone
       - maxcycleiterdone
       - updated model flag
       - peak residual
       - minor cycle summary
    */
    Record getCycleExecutionRecord();


    /* Get Initialization Data
       Returns information at the beginning of a cycle:
       - peak residual
       - maxPSFSidelobe
    */
    Record getCycleInitializationRecord();

    /* Getter Methods for the control variables */
    Float getLoopGain(); 

    void incrementMinorCycleCount(Int itersDonePerStep=1);

    Int getIterDone();
    Int getCycleNiter();
    Float getCycleThreshold();

    /* This method resets the iteration counter for the cycle */
    void resetCycleIter();

    /* Flag to note that the model has been updated */
    void setUpdatedModelFlag(Bool updatedmodel);

   void addSummaryMinor(uInt deconvolverid, uInt subimageid, Float model, Float peakresidual);
    
    /* Variables to track status inside each Deconvolver */
    Float getPeakResidual();
    Float getIntegratedFlux();
    Float getMaxPsfSidelobe();
    
    void setPeakResidual(Float peakResidual);
    void addIntegratedFlux(Float integratedFlux);
    void setMaxPsfSidelobe(Float maxPsfSidelobe);

 protected:
    /* Control Variables */
    Int    itsCycleNiter;
    Float itsCycleThreshold;
    Float itsLoopGain;
    
    Bool  itsUpdatedModelFlag;

    /* Status Reporting Variables */
    Int   itsIterDone; // Iterations done during this set of cycles
    Int   itsCycleIterDone; // Iterations done on this subimage
   Int   itsIterDiff; // Difference between latest and previous count, to catch zero increments.

    /* Useful only for reporting */
    Int   itsTotalIterDone; // Total iterations done by deconvolver

    /* This variable keeps track of the maximum number of iterations done
       during a major cycle */
    Int   itsMaxCycleIterDone;
    

   Float itsPeakResidual;
   Float itsIntegratedFlux;
   Float itsMaxPsfSidelobe;
   

    /* Summary Variable */
    Array<Double> itsSummaryMinor;
    Int itsNSummaryFields;
    Int itsDeconvolverID;    /* An ID per Deconvolver. Used only for the summary */
  };

    
} //# NAMESPACE CASA - END

#endif /* SISUBITERBOT_H_ */
