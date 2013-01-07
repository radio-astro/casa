//# SIISubterBot.cc: This file contains the implementation of the SISubIterBot class.
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

#include <synthesis/MeasurementEquations/SISubIterBot.h>

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
                                itsSummaryMinor(IPosition(2,6,0)),
				itsNSummaryFields(6),
				itsDeconvolverID(0) // Needs to be filled in.
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
                                               itsSummaryMinor(IPosition(2,6,0)),
					       itsNSummaryFields(6),
					       itsDeconvolverID(0) // Needs to be filled in.
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

  /// UUU Is any mapper ready - any one has reached the iter limit ?
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

    /// UUU is THIS mapper ready..
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

  Float SISubIterBot::getPeakResidual()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsPeakResidual;
  }

  void SISubIterBot::setPeakResidual(Float peakResidual)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsPeakResidual = peakResidual;
  }

  Float SISubIterBot::getIntegratedFlux()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsIntegratedFlux;
  }

  void SISubIterBot::setIntegratedFlux(Float integratedFlux)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsIntegratedFlux = integratedFlux;
  }

  Float SISubIterBot::getMaxPsfSidelobe()
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    return itsMaxPsfSidelobe;
  }

  void SISubIterBot::setMaxPsfSidelobe(Float maxPsfSidelobe)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    itsMaxPsfSidelobe = maxPsfSidelobe;
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

  void SISubIterBot::addSummaryMinor(uInt decid, Float model, Float peakresidual)
  {
    boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    
    LogIO os( LogOrigin("SISubIterBot", __FUNCTION__ ,WHERE) );

    IPosition shp = itsSummaryMinor.shape();
    if( shp.nelements() != 2 && shp[0] != itsNSummaryFields ) 
      throw(AipsError("Internal error in shape of minor-cycle summary record"));

     // Note : itsNSummaryFields is hard-coded to 6 in the SISubIterBot constructors.
     itsSummaryMinor.resize( IPosition( 2, itsNSummaryFields, shp[1]+1 ) , True );
     // iterations done
     itsSummaryMinor( IPosition(2, 0, shp[1] ) ) = itsIterDone;
     // peak residual
     itsSummaryMinor( IPosition(2, 1, shp[1] ) ) = (Double) peakresidual;
     // model flux
     itsSummaryMinor( IPosition(2, 2, shp[1] ) ) = (Double) model;
     // cycle threshold
     itsSummaryMinor( IPosition(2, 3, shp[1] ) ) = getCycleThreshold();
     // mapper id
     itsSummaryMinor( IPosition(2, 4, shp[1] ) ) = itsDeconvolverID; // TODO : Get current deconvolver tool ID.
     // chunk id (channel/stokes)
     itsSummaryMinor( IPosition(2, 5, shp[1] ) ) = decid;

  }// end of addSummaryMinor

  Array<Double> SISubIterBot::getSummaryMinor() {
    return itsSummaryMinor;
  }

 
} //# NAMESPACE CASA - END

