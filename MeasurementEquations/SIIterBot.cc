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

namespace casa { //# NAMESPACE CASA - BEGIN
  
  
  ////////////////////////////////////
  /// SIIterBot implementation ///
  ////////////////////////////////////
  
  // All SIIterBots must have 'type' and 'name' defined.
  SIIterBot::SIIterBot(const std::string &serviceName):
                         DBusService(serviceName),
                         itsNiter(0),
                         itsCycleNiter(0),
                         itsInteractiveNiter(0),
                         itsThreshold(0),
                         itsCycleThreshold(0.0),
                         itsInteractiveThreshold(0.0),
                         itsCycleFactor(1.0),
                         itsLoopGain(0.1),
                         itsMinPsfFraction(0.05),
                         itsMaxPsfFraction(0.8),
                         itsMaxPsfSidelobe(0.1),
                         itsStopFlag(false),
                         itsPauseFlag(false),
                         itsInteractiveMode(false),
                         itsIterDone(0),
                         itsCycleIterDone(0),
                         itsInteractiveIterDone(0),
                         itsMajorDone(0),
                         itsMaxCycleIterDone(0),
                         itsUpdatedModelFlag(false),
                         itsControllerCount(0),
                         itsSummaryMinor(IPosition(2,5,0)),
                         itsSummaryMajor(IPosition(1,0))

  {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
    std::cout << "SIIterBot Creation" << std::endl;
    
  }
    
  SIIterBot::~SIIterBot()
  {
    disconnect();
  }

  bool SIIterBot::interactiveInputRequired(Float currentPeakResidual){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex); 
   
    return (itsInteractiveMode &&
            (itsMaxCycleIterDone+itsInteractiveIterDone>=itsInteractiveNiter ||
             currentPeakResidual <= itsInteractiveThreshold ||
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
      interactionRequired(interactionPending);
    }

    /* Wait on Condition variable */
    while (interactionPending) {
      interactionCond.wait(lock);
    }

    if (updateNeeded) {
      updateNeeded = false;
      interactionRequired(false);
      pushDetails();
    }
  }

  void SIIterBot::controlUpdate(const std::map<std::string,
                                DBus::Variant>& updatedParams) {
    Record controlRecord=toRecord(updatedParams);
    setControlsFromRecord(controlRecord);
    {
      boost::lock_guard<boost::mutex> lock(interactionMutex);
      updateNeeded=true;
    }
  }

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


  bool SIIterBot::majorCycleRequired(Float currentPeakResidual){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  

    {
     /* Do a detail update if needed here */
     boost::lock_guard<boost::mutex> lock(interactionMutex);
      updateNeeded=false;
      pushDetails();
    }
    
    if (cleanComplete(currentPeakResidual))
      /* We are done cleaning, this should return true as well */
      return true;
    
    if (itsCycleIterDone >= itsCycleNiter ||
        currentPeakResidual <= itsCycleThreshold)
      return true;

    if (itsInteractiveMode &&
        (itsCycleIterDone + itsInteractiveIterDone >= itsInteractiveNiter ||
         currentPeakResidual <= itsInteractiveThreshold ||
         itsPauseFlag)) {
      return true;
    }

    /* Otherwise */
    return false;
  }

  bool SIIterBot::cleanComplete(Float currentPeakResidual){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    

    if (itsIterDone >= itsNiter || 
        currentPeakResidual <= itsThreshold ||
        itsStopFlag)
      return true;

    return false;
  }
  
  void SIIterBot::changeNiter( Int niter )
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsNiter = niter;
  }

  void SIIterBot::changeCycleNiter( Int cycleniter )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
     if (cycleniter < 0)
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

  void SIIterBot::changeInteractiveMode(const bool& interactiveEnabled)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsInteractiveMode = interactiveEnabled;
  }

  void SIIterBot::changePauseFlag(const bool& pauseEnabled)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsPauseFlag = pauseEnabled;
  }
  
  void SIIterBot::changeStopFlag(const bool& stopEnabled)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsStopFlag = stopEnabled;
  }

  std::map<std::string,DBus::Variant> SIIterBot::getDetails(){
    return fromRecord(getDetailsRecord());
  }

  void SIIterBot::pushDetails() {
    detailUpdate(fromRecord(getDetailsRecord()));
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


  void SIIterBot::setControlsFromRecord(Record &recordIn) {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
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

  /* ------------ Getters for Control Variables ---------------- */
  Int SIIterBot::getNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsNiter;
  }

  Int SIIterBot::getCycleNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsCycleNiter;
  }

  Int SIIterBot::getInteractiveNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsInteractiveNiter;
  }

  Float SIIterBot::getThreshold()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    return itsThreshold;
  }

  Float SIIterBot::getCycleThreshold()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return max(itsCycleThreshold,itsThreshold);
  }

  Float SIIterBot::getInteractiveThreshold()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsInteractiveThreshold;
  }

  Float SIIterBot::getLoopGain()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsLoopGain;
  }

  Float SIIterBot::getCycleFactor()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsCycleFactor;
  }

  Bool SIIterBot::getInteractiveMode()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsInteractiveMode;
  }

  Bool SIIterBot::getPauseFlag()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsPauseFlag;
  }

  Bool SIIterBot::getStopFlag()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsStopFlag;
  }
  /* ------------ End of runtime parameter getters -------- */
  Int SIIterBot::getRemainingNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsNiter - itsIterDone;
  }

  Int SIIterBot::getCompletedNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsIterDone;
  }

  void SIIterBot::incrementMajorCycleCount()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    itsMajorDone++;

    /* Interactive iteractions update */
    itsInteractiveIterDone += max(itsMaxCycleIterDone, itsCycleIterDone);

    itsMaxCycleIterDone = 0;
    itsCycleIterDone = 0;

  }

  void SIIterBot::incrementMinorCycleCount()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    itsIterDone++;
    itsCycleIterDone++;
  }

  Int SIIterBot::getMajorCycleCount()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    return itsMajorDone;
  }

  Record SIIterBot::getDetailsRecord() {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
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

    returnRecord.define( RecordFieldId("maxpsfsidelobe"), itsMaxPsfSidelobe);
    returnRecord.define( RecordFieldId("maxpsffraction"), itsMaxPsfFraction);
    returnRecord.define( RecordFieldId("minpsffraction"), itsMinPsfFraction);

    /* Status Reporting Variables */
    returnRecord.define( RecordFieldId("nmajordone"),  itsMajorDone);
    returnRecord.define( RecordFieldId("iterdone"),  itsIterDone);
    returnRecord.define( RecordFieldId("cycleiterdone"),  itsCycleIterDone);
    returnRecord.define( RecordFieldId("interactiveiterdone"), 
                         itsInteractiveIterDone + itsCycleIterDone);
    
    return returnRecord;
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
  void SIIterBot::addSummaryMinor(Int mapperid, Float model, Float peakresidual)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    
    IPosition shp = itsSummaryMinor.shape();
    if( shp.nelements() != 2 && shp[0] != 5 ) 
      throw(AipsError("Internal error in shape of minor-cycle summary record"));

     itsSummaryMinor.resize( IPosition( 2, 5, shp[1]+1 ) , True );
     itsSummaryMinor( IPosition(2, 0, shp[1] ) ) = itsIterDone;
     itsSummaryMinor( IPosition(2, 1, shp[1] ) ) = (Double) peakresidual;
     itsSummaryMinor( IPosition(2, 2, shp[1] ) ) = (Double) model;
     itsSummaryMinor( IPosition(2, 3, shp[1] ) ) = mapperid;
     itsSummaryMinor( IPosition(2, 4, shp[1] ) ) = getCycleThreshold();
  }// end of addSummaryMinor

  void SIIterBot::addSummaryMajor()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);

     IPosition shp = itsSummaryMajor.shape();
     if( shp.nelements() != 1 ) 
       throw(AipsError("Internal error in shape of major-cycle summary record"));

     itsSummaryMajor.resize( IPosition( 1, shp[0]+1 ) , True );
     itsSummaryMajor( IPosition(1, shp[0] ) ) = itsIterDone;
  }// end of addSummaryMajor
  
  void SIIterBot::updateCycleThreshold(Float PeakResidual) {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    
    Float psffraction = itsMaxPsfSidelobe * itsCycleFactor;
    
    psffraction = max(psffraction, itsMinPsfFraction);
    psffraction = min(psffraction, itsMaxPsfFraction);
    
    itsCycleThreshold = PeakResidual * psffraction;
    pushDetails();
  }

  void SIIterBot::resetCycleIter(){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsMaxCycleIterDone = max(itsCycleIterDone, itsMaxCycleIterDone);
    itsCycleIterDone = 0;
  }

  Float SIIterBot::getMaxPsfSidelobe()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsMaxPsfSidelobe;
  }

  void SIIterBot::setMaxPsfSidelobe(Float maxSidelobe)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsMaxPsfSidelobe = maxSidelobe;
  }

  Float SIIterBot::getMaxPsfFraction()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsMaxPsfFraction;
  }

  void SIIterBot::setMaxPsfFraction(Float maxPsfFraction)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsMaxPsfFraction = maxPsfFraction;
  }

  Float SIIterBot::getMinPsfFraction()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsMinPsfFraction;
  }

  void SIIterBot::setMinPsfFraction(Float minPsfFraction)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsMinPsfFraction = minPsfFraction;
  }

  Bool SIIterBot::getUpdatedModelFlag()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    return itsUpdatedModelFlag;
  }
  
  void SIIterBot::setUpdatedModelFlag(Bool updatedmodel)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsUpdatedModelFlag = updatedmodel;
  }

} //# NAMESPACE CASA - END

