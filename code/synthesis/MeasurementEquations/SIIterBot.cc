//# SIIterBot.cc: This file contains the implementation of the SIIterBot class.
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

#include <synthesis/MeasurementEquations/SIIterBot.h>

/* Include file for the lock guard */
#include <boost/thread/locks.hpp>

/* Records Interface */
#include <casa/Containers/Record.h>

#include <math.h> // For FLT_MAX

#

namespace casa { //# NAMESPACE CASA - BEGIN
  
  ////////////////////////////////////
  /// SIIterBot implementation ///
  ////////////////////////////////////
  
  // All SIIterBots must have 'type' and 'name' defined.
  SIIterBot::SIIterBot(const std::string &serviceName):
#ifdef INTERACTIVE_ITERATION
                       DBusService(serviceName),
#endif
                       itsMinPsfFraction(0.05),
                       itsMaxPsfFraction(0.8),
                       itsMaxPsfSidelobe(0.0),
                       itsPeakResidual(0.0),
                       itsControllerCount(0),
                       itsNiter(0),
                       itsCycleNiter(0),
                       itsInteractiveNiter(0),
                       itsThreshold(0),
                       itsCycleThreshold(0.0),
                       itsInteractiveThreshold(0.0),
                       itsCycleFactor(1.0),
                       itsLoopGain(0.1),
                       itsStopFlag(false),
                       itsPauseFlag(false),
                       itsInteractiveMode(false),
                       itsUpdatedModelFlag(false),
                       itsIterDone(0),
                       itsInteractiveIterDone(0),
                       itsMaxCycleIterDone(0),
                       itsMajorDone(0),
                       itsNSummaryFields(6),
                       itsSummaryMinor(IPosition(2,6,0)),
                       itsSummaryMajor(IPosition(1,0))

  {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
  }
    
  SIIterBot::~SIIterBot()
  {
#ifdef INTERACTIVE_ITERATION
    disconnect();
#endif
  }


  bool SIIterBot::interactiveInputRequired(){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex); 
    return (itsInteractiveMode &&
            (itsMaxCycleIterDone+itsInteractiveIterDone>=itsInteractiveNiter ||
             itsPeakResidual <= itsInteractiveThreshold ||
             itsPauseFlag));
  }

  void SIIterBot::waitForInteractiveInput() {
    /* Check that we have at least one controller */
    if (getNumberOfControllers() == 0) {
      /* Spawn a Viewer set up for interactive */
      
    }

    boost::unique_lock<boost::mutex> lock(interactionMutex);
    if(!interactionPending) {
      interactionPending = true;
      pushDetails();
#ifdef INTERACTIVE_ITERATION
      interactionRequired(interactionPending);
#endif
    }

    /* Wait on Condition variable */
    while (interactionPending) {
      interactionCond.wait(lock);
    }

    if (updateNeeded) {
      updateNeeded = false;
#ifdef INTERACTIVE_ITERATION
      interactionRequired(false);
#endif
      pushDetails();
    }
  }

  bool SIIterBot::cleanComplete(){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    

    if (itsIterDone >= itsNiter || 
        itsPeakResidual <= itsThreshold ||
        itsStopFlag)
      return true;

    return false;
  }


  Record SIIterBot::getMinorCycleControls(){
    LogIO os( LogOrigin("SIIterBot",__FUNCTION__,WHERE) );
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    


    updateCycleThreshold();

    /* Now that we have set the threshold, zero the peak residual 
       so it can be found again after the minor cycles */
    itsPeakResidual = 0;


   /* This returns a record suitable for initializing the minor cycle
       controls. */
    Record returnRecord;

    /* The minor cycle will stop based on the cycle parameters. */
    Int maxCycleIterations = itsCycleNiter;
    Float cycleThreshold     = itsCycleThreshold;
    maxCycleIterations = min(maxCycleIterations, itsNiter - itsIterDone);
    cycleThreshold = max(cycleThreshold, itsThreshold);

    if (itsInteractiveMode) {
      maxCycleIterations = min(maxCycleIterations, itsInteractiveNiter);
      cycleThreshold = max(cycleThreshold, itsInteractiveThreshold);
    }

    returnRecord.define( RecordFieldId("cycleiter"),  maxCycleIterations);
    returnRecord.define( RecordFieldId("cyclethreshold"), cycleThreshold);
    returnRecord.define( RecordFieldId("loopgain"), itsLoopGain);
    returnRecord.define( RecordFieldId("cyclefactor"), itsCycleFactor);

    return returnRecord;
  }

  void SIIterBot::mergeCycleInitializationRecord(Record& initRecord){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  
    
    itsPeakResidual = max(itsPeakResidual,
                          initRecord.asFloat(RecordFieldId("peakresidual")));
    itsMaxPsfSidelobe = 
      max(itsMaxPsfSidelobe,
          initRecord.asFloat(RecordFieldId("maxpsfsidelobe")));
  }


  void SIIterBot::mergeCycleExecutionRecord(Record& execRecord){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  

    mergeMinorCycleSummary(execRecord.asArrayDouble
                           (RecordFieldId("summaryminor")));

    itsIterDone += execRecord.asInt(RecordFieldId("iterdone"));

    itsMaxCycleIterDone =  
      max(itsMaxCycleIterDone, 
          execRecord.asInt(RecordFieldId("maxcycleiterdone")));  

    itsPeakResidual = max(itsPeakResidual,
                          execRecord.asFloat(RecordFieldId("peakresidual")));
  
    itsUpdatedModelFlag |=execRecord.asBool(RecordFieldId("updatedmodelflag"));

  }

  void SIIterBot::mergeMinorCycleSummary(const Array<Double>& summary){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  
    
    IPosition cShp = itsSummaryMinor.shape();
    IPosition nShp = summary.shape();

    if( cShp.nelements() != 2 || cShp[0] != itsNSummaryFields ||
        nShp.nelements() != 2 || nShp[0] != itsNSummaryFields ) 
      throw(AipsError("Internal error in shape of global minor-cycle summary record"));

    itsSummaryMinor.resize( IPosition( 2, itsNSummaryFields, 
                                       cShp[1]+nShp[1] ) ,True );

    for (unsigned int row = 0; row < nShp[1]; row++) {
      // iterations done
      itsSummaryMinor( IPosition(2, 0, cShp[1]+row ) ) = 
        itsIterDone + summary(IPosition(2, 0, row));  
      // peak residual
      itsSummaryMinor( IPosition(2, 1, cShp[1]+row ) ) =
        summary(IPosition(2, 1, row )); 
      // model flux
      itsSummaryMinor( IPosition(2, 2, cShp[1]+row ) ) = 
        summary(IPosition(2, 2, row )); 
      // cycle threshold
      itsSummaryMinor( IPosition(2, 3, cShp[1]+row ) ) = 
        summary(IPosition(2, 3, row )); 
      // mapper id
      itsSummaryMinor( IPosition(2, 4, cShp[1]+row ) ) =
        summary(IPosition(2, 4, row )); 
      // chunk id (channel/stokes)
      itsSummaryMinor( IPosition(2, 5, cShp[1]+row ) ) = 
        summary(IPosition(2, 5, row )); 
    }
  }
  
    #ifdef INTERACTIVE_ITERATION
  void SIIterBot::controlUpdate(const std::map<std::string,
                                DBus::Variant>& updatedParams) {
    Record controlRecord=toRecord(updatedParams);
    setControlsFromRecord(controlRecord);
    {
      boost::lock_guard<boost::mutex> lock(interactionMutex);
      updateNeeded=true;
    }
  }
    #endif

  void SIIterBot::interactionComplete() {
    changePauseFlag(false);
    itsInteractiveIterDone = 0;    
    {
      boost::lock_guard<boost::mutex> lock(interactionMutex);
      interactionPending=false;
      updateNeeded=true;
    }
    interactionCond.notify_all();
  }

#ifdef INTERACTIVE_ITERATION
  std::map<std::string,DBus::Variant> SIIterBot::getDetails(){
    return fromRecord(getDetailsRecord());
  }
#endif

  void SIIterBot::pushDetails() {
    #ifdef INTERACTIVE_ITERATION
    detailUpdate(fromRecord(getDetailsRecord()));
    #endif
  }

  void SIIterBot::pushSummary(){
    //boost::lock_guard<boost::recursive_mutex> guard(countMutex);
    std::cout << __FUNCTION__ << "executing" << std::endl;
    //    summaryUpdate();
  }

  DBus::Variant SIIterBot::getSummary() {
    std::cout << __FUNCTION__ << " executing" << std::endl;
  }

  int SIIterBot::getNumberOfControllers(){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsControllerCount;
  }

  bool SIIterBot::incrementController() {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsControllerCount++;
    return true;
  }

  bool SIIterBot::decrementController() {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsControllerCount--;
    return true;
  } 

  /* ------------ End of runtime parameter getters -------- */

  void SIIterBot::incrementMajorCycleCount()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    itsMajorDone++;

    /* Interactive iteractions update */ 
    itsInteractiveIterDone += itsMaxCycleIterDone;

    /* Get ready to do the minor cycle */
    itsPeakResidual = 0;
    itsMaxPsfSidelobe = 0;
    itsMaxCycleIterDone = 0;
  }

  Int SIIterBot::getMajorCycleCount()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    return itsMajorDone;
  }
  
  Record SIIterBot::getSummaryRecord() {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    Record returnRecord = getDetailsRecord();
    returnRecord.define( RecordFieldId("summaryminor"), itsSummaryMinor);
    returnRecord.define( RecordFieldId("summarymajor"), itsSummaryMajor);

    return returnRecord;
  }

  // TODO : Optimize this storage and resizing ? Or call this only now and then... ?

  void SIIterBot::addSummaryMajor()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);

     IPosition shp = itsSummaryMajor.shape();
     if( shp.nelements() != 1 ) 
       throw(AipsError("Internal error in shape of major-cycle summary record"));

     itsSummaryMajor.resize( IPosition( 1, shp[0]+1 ) , True );
     itsSummaryMajor( IPosition(1, shp[0] ) ) = itsIterDone;
  }// end of addSummaryMajor
  
  void SIIterBot::updateCycleThreshold() {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    
    Float psffraction = itsMaxPsfSidelobe * itsCycleFactor;
    
    psffraction = max(psffraction, itsMinPsfFraction);
    psffraction = min(psffraction, itsMaxPsfFraction);
    
    itsCycleThreshold = itsPeakResidual * psffraction;
    pushDetails();
  }

//   Float SIIterBot::getMaxPsfSidelobe()
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     return itsMaxPsfSidelobe;
//   }

//   void SIIterBot::setMaxPsfSidelobe(Float maxSidelobe)
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     itsMaxPsfSidelobe = maxSidelobe;
//   }

//   Float SIIterBot::getMaxPsfFraction()
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     return itsMaxPsfFraction;
//   }

//   void SIIterBot::setMaxPsfFraction(Float maxPsfFraction)
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     itsMaxPsfFraction = maxPsfFraction;
//   }

//   Float SIIterBot::getMinPsfFraction()
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     return itsMinPsfFraction;
//   }

//   void SIIterBot::setMinPsfFraction(Float minPsfFraction)
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     itsMinPsfFraction = minPsfFraction;
//   }

  Record SIIterBot::getDetailsRecord(){
    LogIO os( LogOrigin("SIIterBot",__FUNCTION__,WHERE) );
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    Record returnRecord;

    /* Control Variables */
    returnRecord.define(RecordFieldId("niter"), itsNiter);
    returnRecord.define(RecordFieldId("cycleniter"), itsCycleNiter);
    returnRecord.define(RecordFieldId("interactiveniter"),itsInteractiveNiter);

    returnRecord.define( RecordFieldId("threshold"),  itsThreshold);    
    returnRecord.define( RecordFieldId("cyclethreshold"),itsCycleThreshold);
    returnRecord.define( RecordFieldId("interactivethreshold"),
                         itsInteractiveThreshold);  

    returnRecord.define( RecordFieldId("loopgain"),  itsLoopGain);
    returnRecord.define( RecordFieldId("cyclefactor"), itsCycleFactor);

    /* Status Reporting Variables */
    returnRecord.define( RecordFieldId("iterdone"),  itsIterDone);
    returnRecord.define( RecordFieldId("cycleiterdone"),  itsMaxCycleIterDone);
    returnRecord.define( RecordFieldId("interactiveiterdone"), 
                         itsInteractiveIterDone + itsMaxCycleIterDone);
    
     returnRecord.define( RecordFieldId("nmajordone"),  itsMajorDone);
     returnRecord.define( RecordFieldId("maxpsfsidelobe"), itsMaxPsfSidelobe);
     returnRecord.define( RecordFieldId("maxpsffraction"), itsMaxPsfFraction);
     returnRecord.define( RecordFieldId("minpsffraction"), itsMinPsfFraction);

     return returnRecord;
  }

  void SIIterBot::changeNiter( Int niter )
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsNiter = niter;
  }

  void SIIterBot::changeCycleNiter( Int cycleniter )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
     if (cycleniter <= 0)  
       itsCycleNiter = itsNiter;
     else
       itsCycleNiter = cycleniter;
  }

  void SIIterBot::changeInteractiveNiter( Int interactiveNiter )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
     itsInteractiveNiter = interactiveNiter;
  }

  void SIIterBot::changeThreshold( Float threshold )
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsThreshold = threshold;
  }

  void SIIterBot::changeCycleThreshold( Float cyclethreshold )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
     itsCycleThreshold = cyclethreshold;
  }

  void SIIterBot::changeInteractiveThreshold( Float interactivethreshold )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
     itsInteractiveThreshold = interactivethreshold;
  }

  void SIIterBot::changeLoopGain( Float loopgain )
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    itsLoopGain = loopgain;
  }

  void SIIterBot::changeCycleFactor(Float cyclefactor)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsCycleFactor = cyclefactor;
  }

  void SIIterBot::changeInteractiveMode(const bool& interactiveEnabled){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsInteractiveMode = interactiveEnabled;
  }

  void SIIterBot::changePauseFlag(const bool& pauseEnabled){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsPauseFlag = pauseEnabled;
  }

  void SIIterBot::changeStopFlag(const bool& stopEnabled){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsStopFlag = stopEnabled;
  }

  void SIIterBot::setControlsFromRecord(Record &recordIn){
    LogIO os( LogOrigin("SIIterBot",__FUNCTION__,WHERE) );
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);

    /* Note it is important that niter get set first as we catch
       negative values in the cycleniter, and set it equal to niter */
    if (recordIn.isDefined("niter"))
      changeNiter(recordIn.asInt( RecordFieldId("niter")));

    if (recordIn.isDefined("cycleniter"))
      changeCycleNiter(recordIn.asInt( RecordFieldId("cycleniter")));

    if (recordIn.isDefined("interactiveniter"))
      changeInteractiveNiter(recordIn.
                             asInt(RecordFieldId("interactiveniter")));
    
    if (recordIn.isDefined("threshold")) 
      changeThreshold(recordIn.asFloat( RecordFieldId("threshold")));
        
    if (recordIn.isDefined("cyclethreshold")) 
      changeCycleThreshold(recordIn.asFloat( RecordFieldId("cyclethreshold")));
    
    if (recordIn.isDefined("interactivethreshold")) 
      changeInteractiveThreshold(recordIn.asFloat
                                 (RecordFieldId("interactivethreshold")));

    if (recordIn.isDefined("loopgain")) 
      changeLoopGain(recordIn.asDouble( RecordFieldId("loopgain")));;

    if (recordIn.isDefined("cyclefactor"))
      changeCycleFactor(recordIn.asFloat( RecordFieldId("cyclefactor")));

    if (recordIn.isDefined("interactive"))
      changeInteractiveMode(recordIn.asBool(RecordFieldId("interactive")));
   }

} //# NAMESPACE CASA - END

