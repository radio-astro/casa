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
  
  
  SISubIterBot::SISubIterBot(): itsNiter(0),
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
                                itsCycleIterDone(0),
                                itsInteractiveIterDone(0),
                                itsMaxCycleIterDone(0),
                                itsSummaryMinor(IPosition(2,5,0))
  {}


  SISubIterBot::SISubIterBot(Record& recordIn):itsNiter(0),
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
                                               itsCycleIterDone(0),
                                               itsInteractiveIterDone(0),
                                               itsMaxCycleIterDone(0),
                                               itsSummaryMinor(IPosition(2,5,0))
  {
    LogIO os( LogOrigin("SISubIterBot",__FUNCTION__,WHERE) );
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  

    setControlsFromRecord(recordIn);

    if (recordIn.isDefined("updatedmodelflag"))
      recordIn.get("updatedmodelflag", itsUpdatedModelFlag);
    if (recordIn.isDefined("stopflag"))
      recordIn.get("stopflag",itsStopFlag);
    if (recordIn.isDefined("pauseflag"))
      recordIn.get("pauseflag",itsPauseFlag);
    if (recordIn.isDefined("interactive"))
      recordIn.get("interactive",itsInteractiveMode);
    if (recordIn.isDefined("maxcycleiterdone"))
      recordIn.get("maxcycleiterdone",itsMaxCycleIterDone);
    if (recordIn.isDefined("iterdone"))
      recordIn.get("iterdone",itsIterDone);
    if (recordIn.isDefined("cycleiterdone"))
      recordIn.get("cycleiterdone",itsCycleIterDone);
    if (recordIn.isDefined("interactiveiterdone"))
      recordIn.get("interactiveiterdone",itsInteractiveIterDone);
    if (recordIn.isDefined("summaryminor"))
      recordIn.get("summaryminor",itsSummaryMinor);
  }

  SISubIterBot::~SISubIterBot(){}

  bool SISubIterBot::interactiveInputRequired(Float currentPeakResidual){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex); 
    return (itsInteractiveMode &&
            (itsMaxCycleIterDone+itsInteractiveIterDone>=itsInteractiveNiter ||
             currentPeakResidual <= itsInteractiveThreshold ||
             itsPauseFlag));
  }

  bool SISubIterBot::majorCycleRequired(Float currentPeakResidual){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  
    
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

  bool SISubIterBot::cleanComplete(Float currentPeakResidual){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    

    if (itsIterDone >= itsNiter || 
        currentPeakResidual <= itsThreshold ||
        itsStopFlag)
      return true;

    return false;
  }

  void SISubIterBot::changeNiter( Int niter )
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsNiter = niter;
  }

  void SISubIterBot::changeCycleNiter( Int cycleniter )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
     if (cycleniter < 0)
       itsCycleNiter = itsNiter;
     else
       itsCycleNiter = cycleniter;
  }

  void SISubIterBot::changeInteractiveNiter( Int interactiveNiter )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
     itsInteractiveNiter = interactiveNiter;
  }

  void SISubIterBot::changeThreshold( Float threshold )
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsThreshold = threshold;
  }

  void SISubIterBot::changeCycleThreshold( Float cyclethreshold )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
     itsCycleThreshold = cyclethreshold;
  }

  void SISubIterBot::changeInteractiveThreshold( Float interactivethreshold )
  {
     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
     itsInteractiveThreshold = interactivethreshold;
  }

  void SISubIterBot::changeLoopGain( Float loopgain )
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    itsLoopGain = loopgain;
  }

  void SISubIterBot::changeCycleFactor(Float cyclefactor)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsCycleFactor = cyclefactor;
  }

  void SISubIterBot::changeInteractiveMode(const bool& interactiveEnabled)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsInteractiveMode = interactiveEnabled;
  }

  void SISubIterBot::changePauseFlag(const bool& pauseEnabled)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsPauseFlag = pauseEnabled;
  }
  
  void SISubIterBot::changeStopFlag(const bool& stopEnabled)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsStopFlag = stopEnabled;
  }

  Int SISubIterBot::getNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsNiter;
  }

  Int SISubIterBot::getCycleNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsCycleNiter;
  }

  Int SISubIterBot::getInteractiveNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsInteractiveNiter;
  }

  Int SISubIterBot::getMaxCycleIterDone()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsMaxCycleIterDone;
  }

  Int SISubIterBot::getRemainingNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsNiter - itsIterDone;
  }

  Int SISubIterBot::getCompletedNiter()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsIterDone;
  }

  Float SISubIterBot::getThreshold()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    return itsThreshold;
  }

  Float SISubIterBot::getCycleThreshold()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return max(itsCycleThreshold,itsThreshold);
  }

  Float SISubIterBot::getInteractiveThreshold()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsInteractiveThreshold;
  }

  Float SISubIterBot::getLoopGain()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsLoopGain;
  }

  Float SISubIterBot::getCycleFactor()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsCycleFactor;
  }

  Bool SISubIterBot::getInteractiveMode()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsInteractiveMode;
  }

  Bool SISubIterBot::getPauseFlag()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsPauseFlag;
  }

  Bool SISubIterBot::getStopFlag()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsStopFlag;
  }

  Bool SISubIterBot::getUpdatedModelFlag()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    return itsUpdatedModelFlag;
  }
  
  void SISubIterBot::setUpdatedModelFlag(Bool updatedmodel)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsUpdatedModelFlag = updatedmodel;
  }

  void SISubIterBot::incrementMinorCycleCount()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    itsIterDone++;
    itsCycleIterDone++;
  }

  Record SISubIterBot::getDetailsRecord() {
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

    /* Status Reporting Variables */
    returnRecord.define( RecordFieldId("iterdone"),  itsIterDone);
    returnRecord.define( RecordFieldId("cycleiterdone"),  itsCycleIterDone);
    returnRecord.define( RecordFieldId("interactiveiterdone"), 
                         itsInteractiveIterDone + itsCycleIterDone);
    
    return returnRecord;
  }

  Record SISubIterBot::serialize() {
    LogIO os( LogOrigin("SISubIterBot",__FUNCTION__,WHERE) );
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    Record returnRecord = getDetailsRecord();

    returnRecord.define(RecordFieldId("updatedmodelflag"), itsUpdatedModelFlag);
    returnRecord.define(RecordFieldId("stopflag"),itsStopFlag);
    returnRecord.define(RecordFieldId("pauseflag"),itsPauseFlag);
    returnRecord.define(RecordFieldId("interactive"),itsInteractiveMode);
    returnRecord.define(RecordFieldId("maxcycleiterdone"),itsMaxCycleIterDone);
    returnRecord.define(RecordFieldId("summaryminor"), itsSummaryMinor);

    return returnRecord;
  }

  void SISubIterBot::setControlsFromRecord(Record &recordIn) {
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

  void SISubIterBot::resetCycleIter(){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsMaxCycleIterDone = max(itsCycleIterDone, itsMaxCycleIterDone);
    itsCycleIterDone = 0;
  }

  void SISubIterBot::addSummaryMinor(Int mapperid, Float model, Float peakresidual)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    
    LogIO os( LogOrigin("SISubIterBot", __FUNCTION__ ,WHERE) );


    IPosition shp = itsSummaryMinor.shape();
    if( shp.nelements() != 2 && shp[0] != 5 ) 
      throw(AipsError("Internal error in shape of minor-cycle summary record"));

     itsSummaryMinor.resize( IPosition( 2, 5, shp[1]+1 ) , True );
     itsSummaryMinor( IPosition(2, 0, shp[1] ) ) = itsIterDone;
     itsSummaryMinor( IPosition(2, 1, shp[1] ) ) = (Double) peakresidual;
     itsSummaryMinor( IPosition(2, 2, shp[1] ) ) = (Double) model;
     itsSummaryMinor( IPosition(2, 3, shp[1] ) ) = mapperid;
     itsSummaryMinor( IPosition(2, 4, shp[1] ) ) = getCycleThreshold();

     shp = itsSummaryMinor.shape();

  }// end of addSummaryMinor

  Array<Double> SISubIterBot::getSummaryMinor() {
    return itsSummaryMinor;
  }

  ////////////////////////////////////
  /// SIIterBot implementation ///
  ////////////////////////////////////
  
  // All SIIterBots must have 'type' and 'name' defined.
  SIIterBot::SIIterBot(const std::string &serviceName):
                         DBusService(serviceName),
                         itsMinPsfFraction(0.05),
                         itsMaxPsfFraction(0.8),
                         itsMaxPsfSidelobe(0.1),
                         itsControllerCount(0),
                         itsMajorDone(0),
                         itsSummaryMajor(IPosition(1,0))

  {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
  }
    
  SIIterBot::~SIIterBot()
  {
    disconnect();
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

  bool SIIterBot::majorCycleRequired(Float currentPeakResidual){
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  

    if (updateNeeded) {
     /* Do a detail update if needed here */
     boost::lock_guard<boost::mutex> lock(interactionMutex);
      updateNeeded=false;
      pushDetails();
    }
    
    return SISubIterBot::majorCycleRequired(currentPeakResidual);
  }

  void SIIterBot::mergeSubIterBot(SISubIterBot& subIterBot) {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  
    LogIO os( LogOrigin("SISubIterBot",__FUNCTION__,WHERE) );


    /* Need to merge in the minor cycle summary */
    IPosition shp = itsSummaryMinor.shape();
    if( shp.nelements() != 2 && shp[0] != 5 ) 
      throw(AipsError("Internal error in shape of minor-cycle summary record"));
    Array<Double> subSummary = subIterBot.getSummaryMinor();
    IPosition subIterShp = subSummary.shape();


    itsSummaryMinor.resize( IPosition( 2, 5, shp[1]+subIterShp[1] ) ,True );

    for (unsigned int row = 0; row < subIterShp[1]; row++) {
      itsSummaryMinor( IPosition(2, 0, shp[1]+row ) ) = 
        itsIterDone + subSummary(IPosition(2, 0,row))  ;
      itsSummaryMinor( IPosition(2, 1, shp[1]+row ) ) = 
        subSummary(IPosition(2, 1, row ));
      itsSummaryMinor( IPosition(2, 2, shp[1]+row ) ) = 
        subSummary(IPosition(2, 2, row ));
      itsSummaryMinor( IPosition(2, 3, shp[1]+row ) ) = 
        subSummary(IPosition(2, 3, row ));
      itsSummaryMinor( IPosition(2, 4, shp[1]+row ) ) = 
        subSummary(IPosition(2, 4, row ));
    }

    /* Now do the rest of the values */
    itsMaxCycleIterDone = max(itsMaxCycleIterDone, 
                              subIterBot.getMaxCycleIterDone());
    itsIterDone += subIterBot.getCompletedNiter();

    if (subIterBot.getUpdatedModelFlag()) {
      setUpdatedModelFlag(true);
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



  /* ------------ End of runtime parameter getters -------- */

  void SIIterBot::incrementMajorCycleCount()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    itsMajorDone++;

    /* Interactive iteractions update */
    itsInteractiveIterDone += max(itsMaxCycleIterDone, itsCycleIterDone);

    itsMaxCycleIterDone = 0;
    itsCycleIterDone = 0;
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

  Record SIIterBot::getSubIterBotRecord() {
    LogIO os( LogOrigin("SIIterBot",__FUNCTION__,WHERE) );
    /* This returns a record suitable for initializing a subIterBot. 
       In particular it has no iterations or summary */
    Record returnRecord = serialize();
    Array<Double> tmpSummary(IPosition(2,5,0));

    returnRecord.define( RecordFieldId("iterdone"),  0);
    returnRecord.define( RecordFieldId("summaryminor"), tmpSummary);
    returnRecord.define( RecordFieldId("updatedmodelflag"), false);

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
  
  void SIIterBot::updateCycleThreshold(Float PeakResidual) {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    
    Float psffraction = itsMaxPsfSidelobe * itsCycleFactor;
    
    psffraction = max(psffraction, itsMinPsfFraction);
    psffraction = min(psffraction, itsMaxPsfFraction);
    
    itsCycleThreshold = PeakResidual * psffraction;
    pushDetails();
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

  Record SIIterBot::getDetailsRecord(){
    Record returnRecord = SISubIterBot::getDetailsRecord();
    returnRecord.define( RecordFieldId("nmajordone"),  itsMajorDone);
    returnRecord.define( RecordFieldId("maxpsfsidelobe"), itsMaxPsfSidelobe);
    returnRecord.define( RecordFieldId("maxpsffraction"), itsMaxPsfFraction);
    returnRecord.define( RecordFieldId("minpsffraction"), itsMinPsfFraction);

    return returnRecord;
  }

  /* Methods that just delegate to the SubIterBot class */
  void SIIterBot::changeNiter( Int niter ){
    SISubIterBot::changeNiter(niter);
  }

  void SIIterBot::changeCycleNiter( Int cycleniter){
    SISubIterBot::changeCycleNiter(cycleniter);
  }

  void SIIterBot::changeInteractiveNiter(Int interactiveniter ){
    SISubIterBot::changeInteractiveNiter(interactiveniter);
  }

  
  void SIIterBot::changeThreshold( Float threshold){
    SISubIterBot::changeThreshold(threshold);
  }

  void SIIterBot::changeCycleThreshold( Float cyclethreshold){
    SISubIterBot::changeCycleThreshold(cyclethreshold);
  }

  void SIIterBot::changeInteractiveThreshold( Float cyclethreshold){
    SISubIterBot::changeInteractiveThreshold(cyclethreshold);
  }

  void SIIterBot::changeLoopGain( Float loopgain){
    SISubIterBot::changeLoopGain(loopgain);
  }

  void SIIterBot::changeCycleFactor( Float cyclefactor){
    SISubIterBot::changeCycleFactor(cyclefactor);
  }

  void SIIterBot::changeInteractiveMode(const bool& interactiveEnabled){
    SISubIterBot::changeInteractiveMode(interactiveEnabled);
  }

  void SIIterBot::changePauseFlag(const bool& pauseEnabled){
    SISubIterBot::changePauseFlag(pauseEnabled);
  }

  void SIIterBot::changeStopFlag(const bool& stopEnabled){
    SISubIterBot::changeStopFlag(stopEnabled);
  }

  void SIIterBot::setControlsFromRecord(Record &recordIn){
    SISubIterBot::setControlsFromRecord(recordIn);
  }






} //# NAMESPACE CASA - END

