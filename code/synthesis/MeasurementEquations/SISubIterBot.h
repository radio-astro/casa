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

#ifndef SYNTHESIS_SISUBITERBOT
#define SYNTHESIS_SISUBITERBOT

// .casarc interface
#include <casa/System/AipsrcValue.h>

// System utilities (for profiling macros)
#include <casa/OS/HostInfo.h>
#include <sys/time.h>


// Boost Libraries for mutex and noncopyable semantics
#include <boost/thread/recursive_mutex.hpp>
#include <boost/utility.hpp>

// Include files for the DBus Service
//#include <casadbus/interfaces/SynthImager.adaptor.h>
//#include <casadbus/utilities/DBusBase.h>


/* Future Decl */
class casa::Record;

namespace casa { //# NAMESPACE CASA - BEGIN
  
  class SISubIterBot : boost::noncopyable 
 {
  public:
    SISubIterBot();
    SISubIterBot(Record& recordIn);

    virtual ~SISubIterBot();

    /* Method to call to determine if we should wait for interactive
       updates from the user true if:
         - exceeded number of interactiveNiter 
         - exceeded interactiveThreshold
         - pause has been pressed
    */
    bool interactiveInputRequired(Float currentPeakResidual);

    /* Method to call to determine if a major cycle is required, true if
         - We are done (see below)
         - exceeded number maxcycleniter
         - exceeded cycle threshold
    */
    virtual bool majorCycleRequired(Float currentPeakResidual);

    /* Method to call to determine if we should stop the clean process 
       true if
       - stop flag has been set
       - exceeded Niter
       - exceeded threshold
    */
    bool cleanComplete(Float peakResidual);

    /* Getter Methods for the control variables */
    Int getNiter();
    Int getCycleNiter();
    Int getInteractiveNiter();

    Int getMaxCycleIterDone();
    Int getIterDone();

    Float getThreshold();
    Float getCycleThreshold();
    Float getInteractiveThreshold();
   
    Float getLoopGain();
    Float getCycleFactor();

    Bool getInteractiveMode();
    Bool getPauseFlag();
    Bool getStopFlag();

    void incrementMinorCycleCount();

    /* This method resets the iteration counter for the cycle */
    void resetCycleIter();

    /* This returns the following fields:
       * Controls
       - niter
       - cycleniter
       - interactiveniter
       - threshold
       - cyclethreshold
       - interactivethreshold
       - loopgain
       - cyclefactor
       
       * Status 
       - iterdone
       - cycleiterdone;
       - interactiveiterdone;
       - nmajordone;
    */
    Record getDetailsRecord(); 
    Record serialize();

    Int getRemainingNiter();
    Int getCompletedNiter();

    /* Flag to note that the model has been updated */
    Bool getUpdatedModelFlag();
    void setUpdatedModelFlag(Bool updatedmodel);

   void addSummaryMinor(uInt decid, Float model, Float peakresidual);

    Array<Double> getSummaryMinor();
    
   /* Variables to track status inside each Deconvolver */
   Float getPeakResidual();
   Float getIntegratedFlux();
   Float getMaxPsfSidelobe();
   void setPeakResidual(Float peakResidual);
   void setIntegratedFlux(Float integratedFlux);
   void setMaxPsfSidelobe(Float maxPsfSidelobe);

  protected:
    /* As a convience the controls can also be updated from a Record.  
       The following fields are supported:
       - niter
       - cycleniter
       - interactiveniter
       - threshold
       - cyclethreshold
       - interactivethreshold
       - loopgain
       - cyclefactor
    */
    void setControlsFromRecord(Record &recordIn);



    /* These are protected in this scope, but public in the Iterbot */
    void changeNiter( Int niter );
    void changeCycleNiter( Int cycleniter );
    void changeInteractiveNiter(Int interactiveniter );

    void changeThreshold( Float threshold );
    void changeCycleThreshold( Float cyclethreshold );
    void changeInteractiveThreshold( Float cyclethreshold );

    void changeLoopGain(  Float loopgain );
    void changeCycleFactor( Float cyclefactor);

    void changeInteractiveMode(const bool& interactiveEnabled);
    void changePauseFlag(const bool& pauseEnabled);
    void changeStopFlag(const bool& stopEnabled);
    
    /* A recursive mutex which provides access control to the
       underlying state variables
    */
    boost::recursive_mutex recordMutex;

    /* These are the internal variables that we need for the control 
       note: ALL access to these should be through getter/setter mechanism
       so they can be protected by the mutex 
    */
    
    /* Control Variables */
    Int    itsNiter;
    Int    itsCycleNiter;
    Int    itsInteractiveNiter;

    Float itsThreshold;
    Float itsCycleThreshold;
    Float itsInteractiveThreshold;

    Float itsCycleFactor;
    Float itsLoopGain;
    
    Bool  itsStopFlag;
    Bool  itsPauseFlag;
    Bool  itsInteractiveMode;
    Bool  itsUpdatedModelFlag;

    /* Status Reporting Variables */
    Int   itsIterDone;
    Int   itsCycleIterDone;
    Int   itsInteractiveIterDone;

    /* This variable keeps track of the maximum number of iterations done
       during a major cycle */
    Int   itsMaxCycleIterDone;

   /* JK : TODO : These variables track the status inside each Deconvolver.
       The mergeIterBot function should reconcile these across Deconvolvers.
       Need to add get/set methods for them, and read these vars in getSubIterBot, and cleanComplete */
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
