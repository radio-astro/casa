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

#include <synthesis/MeasurementEquations/SIMinorCycleController.h>

/* Include file for the lock guard */
//#include <boost/thread/locks.hpp>

/* Records Interface */
#include <casa/Containers/Record.h>

#include <math.h> // For FLT_MAX

namespace casa { //# NAMESPACE CASA - BEGIN
  
  
  SIMinorCycleController::SIMinorCycleController(): 
                                itsCycleNiter(0),
                                itsCycleThreshold(0.0),
                                itsCycleFactor(1.0),
                                itsLoopGain(0.1),
                                itsUpdatedModelFlag(false),
                                itsIterDone(0),
                                itsCycleIterDone(0),
                                itsTotalIterDone(0),
                                itsIntegratedFlux(0),
                                itsMaxCycleIterDone(0),
                                itsPeakResidual(0),
                                itsMaxPsfSidelobe(0),
                                itsSummaryMinor(IPosition(2,6,0)),
				itsNSummaryFields(6),
				itsDeconvolverID(0) 
  {}



  SIMinorCycleController::~SIMinorCycleController(){}


  bool SIMinorCycleController::majorCycleRequired(Float currentPeakResidual){
    if (itsCycleIterDone >= itsCycleNiter ||
        currentPeakResidual <= itsCycleThreshold)
      return true;

    /* Otherwise */
    return false;
  }


  Float SIMinorCycleController::getLoopGain()
  {
    return itsLoopGain;
  }
  
  Float SIMinorCycleController::getCycleFactor()
  {
    return itsCycleFactor;
  }

  
  void SIMinorCycleController::setUpdatedModelFlag(Bool updatedmodel)
  {
    itsUpdatedModelFlag = updatedmodel;
  }

  void SIMinorCycleController::incrementMinorCycleCount()
  {
    itsIterDone++;
    itsCycleIterDone++;
  }

  Float SIMinorCycleController::getPeakResidual()
  {
    return itsPeakResidual;
  }

  void SIMinorCycleController::setPeakResidual(Float peakResidual)
  {
    itsPeakResidual = peakResidual;
  }

  Float SIMinorCycleController::getIntegratedFlux()
  {
    return itsIntegratedFlux;
  }

  void SIMinorCycleController::addIntegratedFlux(Float integratedFlux)
  {
    itsIntegratedFlux += integratedFlux;
  }

  Float SIMinorCycleController::getMaxPsfSidelobe()
  {
    return itsMaxPsfSidelobe;
  }

  void SIMinorCycleController::setMaxPsfSidelobe(Float maxPsfSidelobe)
  {
    itsMaxPsfSidelobe = maxPsfSidelobe;
  }

  Record SIMinorCycleController::getCycleExecutionRecord() {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
    //boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    Record returnRecord;

    returnRecord.define( RecordFieldId("iterdone"),  itsIterDone);
    returnRecord.define(RecordFieldId("peakresidual"), itsPeakResidual);
    returnRecord.define(RecordFieldId("updatedmodelflag"), 
                        itsUpdatedModelFlag);
    returnRecord.define(RecordFieldId("summaryminor"), itsSummaryMinor);
    returnRecord.define(RecordFieldId("updatedmodelflag"),
                        itsUpdatedModelFlag);
    returnRecord.define(RecordFieldId("maxcycleiterdone"),
                        itsMaxCycleIterDone);
    return returnRecord;
  }

  Record SIMinorCycleController::getCycleInitializationRecord() {
    LogIO os( LogOrigin("SIMinorCycleController",__FUNCTION__,WHERE) );

    Record returnRecord;

    /* Control Variables */
    returnRecord.define(RecordFieldId("peakresidual"), itsPeakResidual);
    returnRecord.define(RecordFieldId("maxpsfsidelobe"), itsMaxPsfSidelobe);

    return returnRecord;
  }

  void SIMinorCycleController::setCycleControls(Record &recordIn) {
    LogIO os( LogOrigin("SIMinorCycleController",__FUNCTION__,WHERE) );

    if (recordIn.isDefined("cycleniter"))
      recordIn.get(RecordFieldId("cycleniter"), itsCycleNiter);

    if (recordIn.isDefined("cyclethreshold")) 
      recordIn.get(RecordFieldId("cyclethreshold"),itsCycleThreshold);

    if (recordIn.isDefined("loopgain")) 
      recordIn.get(RecordFieldId("loopgain"), itsLoopGain);

    if (recordIn.isDefined("cyclefactor"))
      recordIn.get(RecordFieldId("cyclefactor"), itsCycleFactor);

    /* Reset the counters for the new cycle */
    itsMaxCycleIterDone = 0;
    itsCycleIterDone = 0;
    itsUpdatedModelFlag = false;
  }

  void SIMinorCycleController::resetCycleIter(){
    itsMaxCycleIterDone = max(itsCycleIterDone, itsMaxCycleIterDone);
    itsCycleIterDone = 0;
  }

  void SIMinorCycleController::addSummaryMinor(uInt decid, Float model, Float peakresidual)
  {
    //boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
    
    LogIO os( LogOrigin("SIMinorCycleController", __FUNCTION__ ,WHERE) );

    IPosition shp = itsSummaryMinor.shape();
    if( shp.nelements() != 2 && shp[0] != itsNSummaryFields ) 
      throw(AipsError("Internal error in shape of minor-cycle summary record"));

     // Note : itsNSummaryFields is hard-coded to 6 in the SIMinorCycleController constructors.
     itsSummaryMinor.resize( IPosition( 2, itsNSummaryFields, shp[1]+1 ) , True );
     // iterations done
     itsSummaryMinor( IPosition(2, 0, shp[1] ) ) = itsIterDone;
     // peak residual
     itsSummaryMinor( IPosition(2, 1, shp[1] ) ) = (Double) peakresidual;
     // model flux
     itsSummaryMinor( IPosition(2, 2, shp[1] ) ) = (Double) model;
     // cycle threshold
     itsSummaryMinor( IPosition(2, 3, shp[1] ) ) = itsCycleThreshold;
     // mapper id
     itsSummaryMinor( IPosition(2, 4, shp[1] ) ) = itsDeconvolverID; // TODO : Get current deconvolver tool ID.
     // chunk id (channel/stokes)
     itsSummaryMinor( IPosition(2, 5, shp[1] ) ) = decid;

  }// end of addSummaryMinor
 
} //# NAMESPACE CASA - END

