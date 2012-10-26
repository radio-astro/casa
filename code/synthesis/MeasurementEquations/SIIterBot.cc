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

namespace casa { //# NAMESPACE CASA - BEGIN
  
  
  ////////////////////////////////////
  /// SIIterBot implementation ///
  ////////////////////////////////////
  
  // All SIIterBots must have 'type' and 'name' defined.
  SIIterBot::SIIterBot():Record()
  {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
    
    verifyFields();
    
  }
  
  
  SIIterBot::SIIterBot(Record &other)
  {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );
    
    assign(other);
    verifyFields();
  }
  
  SIIterBot::~SIIterBot()
  {
  }
  
  // Check that all required fields have valid values.
  Bool  SIIterBot::verifyFields()
  {
    LogIO os( LogOrigin("SISkyModel",__FUNCTION__,WHERE) );

    // Input variables

    if( ! isDefined("niter") ){ define( RecordFieldId("niter"), (Int) 0 ); }
    else {/*check datatype*/}
    
    if( ! isDefined("threshold") ){ define( RecordFieldId("threshold"), (Double) 0.0 ); }
    else 
      {/*check datatype*/ }

    if( ! isDefined("loopgain") ){ define( RecordFieldId("loopgain"), (Double) 0.1 ); }
    else 
      {/*check datatype*/}

    if( ! isDefined("maxcycleniter") ){ define( RecordFieldId("maxcycleniter"), 0  ); }
    else {/*check datatype*/}
    
    if( ! isDefined("cyclefactor") ){ define( RecordFieldId("cyclefactor"), (Double) 1.0 ); }
    else 
      {/*check datatype*/ }

    if( ! isDefined("cyclethreshold") ){ define( RecordFieldId("cyclethreshold"), (Double) 0.0 ); }
    else 
      {/*check datatype*/ }

    // Output variables

    if( ! isDefined("stop") ){ define( RecordFieldId("stop"), (Bool) False ); }
    else {/*check datatype*/}
    
    if( ! isDefined("converged") ){ define( RecordFieldId("converged"), (Bool) False ); }
    else {/*check datatype*/}
    
    if( ! isDefined("updatedmodel") ){ define( RecordFieldId("updatedmodel"), (Bool) False ); }
    else {/*check datatype*/}
    
    if( ! isDefined("niterdone") ){ define( RecordFieldId("niterdone"), (Int) 0 ); }
    else {/*check datatype*/}

    if( ! isDefined("peakresidual") ){ define( RecordFieldId("peakresidual"), (Double) 1e+20 ); }
    else {/*check datatype*/}
    
    if( ! isDefined("modelflux") ){ define( RecordFieldId("modelflux"), (Double) 0.0 ); }
    else {/*check datatype*/}
    
    if( ! isDefined("nmajordone") ){ define( RecordFieldId("nmajordone"), (Int) 0 ); }
    else {/*check datatype*/}

    if( ! isDefined("summaryminor") ){ define( RecordFieldId("summaryminor"), Array<Double>(IPosition(2,5,0)) ); }
    else {/*check datatype*/}

    if( ! isDefined("summarymajor") ){ define( RecordFieldId("summarymajor"), Array<Int>( IPosition(1,0) ) ); }
    else {/*check datatype*/}

    return True;
    
  }// end of verifyFields()
  

  void SIIterBot::incrementMajorCycleCount()
  {
    verifyFields();
    define( RecordFieldId("nmajordone") ,  isDefined("nmajordone")? 1+asInt( RecordFieldId("nmajordone") )  : 1 );


  }

  void SIIterBot::incrementMinorCycleCount()
  {
    verifyFields(); // Check Performance !
    define( RecordFieldId("niterdone") ,  isDefined("niterdone")? 1+asInt( RecordFieldId("niterdone") )  : 1 );

    /// CHECK for modified parameters here.... 

  }

  // TODO : Optimize this storage and resizing ? Or call this only now and then... ?
  void SIIterBot::addSummaryMinor(Int mapperid, Float model, Float peakresidual)
  {
    Array<Double> summaryminor = asArrayDouble( RecordFieldId("summaryminor") );
    
    IPosition shp = summaryminor.shape();
    if( shp.nelements() != 2 && shp[0] != 5 ) throw( AipsError("Internal error in shape of minor-cycle summary record") );

    Int niterdone = asInt( RecordFieldId("niterdone") );

    summaryminor.resize( IPosition( 2, 5, shp[1]+1 ) , True );
    summaryminor( IPosition(2, 0, shp[1] ) ) = asInt( RecordFieldId("niterdone") );
    summaryminor( IPosition(2, 1, shp[1] ) ) = (Double) peakresidual;
    summaryminor( IPosition(2, 2, shp[1] ) ) = (Double) model;
    summaryminor( IPosition(2, 3, shp[1] ) ) = mapperid;
    summaryminor( IPosition(2, 4, shp[1] ) ) = getCycleThreshold();

    define( RecordFieldId("summaryminor") , summaryminor );

  }// end of addSummaryMinor

  void SIIterBot::addSummaryMajor()
  {
    Array<Int> summarymajor = asArrayInt( RecordFieldId("summarymajor") );
    Int nmajordone = asInt( RecordFieldId("nmajordone") );

    IPosition shp = summarymajor.shape();
    if( shp.nelements() != 1 ) throw( AipsError("Internal error in shape of major-cycle summary record") );

    summarymajor.resize( IPosition( 1, shp[0]+1 ) , True );
    summarymajor( IPosition(1, shp[0] ) ) = asInt( RecordFieldId("niterdone") );

    define( RecordFieldId("summarymajor") , summarymajor );

  }// end of addSummaryMajor


  void SIIterBot::checkStop()
  {
    verifyFields();

    Bool niterstop = (isDefined("niterdone")&&isDefined("niter"))?  asInt( RecordFieldId("niterdone") ) >= asInt( RecordFieldId("niter") ) : False ;
    Bool thresholdstop = (isDefined("peakresidual")&&isDefined("threshold"))?  asFloat( RecordFieldId("peakresidual") ) <= asFloat( RecordFieldId("threshold") ) : False ;

    define( RecordFieldId("stop") , niterstop || thresholdstop );

    define( RecordFieldId("converged") , thresholdstop );

  }

  Bool SIIterBot::checkMinorStop( Int iters, Float currentpeakresidual )
  {
    verifyFields();
   
    Float cyclethreshold = getCycleThreshold();
    Float maxcycleniter = asFloat( RecordFieldId("maxcycleniter") );

    Int niterdone = asInt( RecordFieldId("niterdone") );
    Int niter = asInt( RecordFieldId("niter") );

    //    cout << currentpeakresidual << " : " << cyclethreshold << " : " << iters << " : " << maxcycleniter << " : " << niterdone << " : " << niter << endl;

    Bool stopnow = currentpeakresidual < cyclethreshold;
    stopnow |= niterdone>=niter;
    if(maxcycleniter>0) stopnow |= iters>maxcycleniter;

    return stopnow;

  }


  Int SIIterBot::getRemainingNiter()
  {
    verifyFields();
    return asInt( RecordFieldId("niter") ) - asInt( RecordFieldId("niterdone") );
  }

  Int SIIterBot::getCompletedNiter()
  {
    verifyFields();
    return asInt( RecordFieldId("niterdone") );
  }
  
    
  Int SIIterBot::getMaxCycleNiter()
  {
    return asInt( RecordFieldId("maxcycleniter") );
   }
  

  void SIIterBot::setPsfSidelobe(Float maxpsfsidelobe)
  {
    verifyFields();
    define( RecordFieldId("psfsidelobe") , (Double) maxpsfsidelobe );

  }

  void SIIterBot::calculateCycleThreshold()
  {
    //verifyFields();

    Float minpsffraction = asFloat( RecordFieldId("minpsffraction") );
    Float maxpsffraction = asFloat( RecordFieldId("maxpsffraction") );
    Float psfsidelobe = asFloat( RecordFieldId("psfsidelobe") );

    Float psffraction = psfsidelobe * asFloat( RecordFieldId("cyclefactor") );
    psffraction = ( psffraction < minpsffraction ) ? minpsffraction : psffraction;
    psffraction = ( psffraction > maxpsffraction ) ? maxpsffraction : psffraction;

    Float cyclethreshold = asFloat( RecordFieldId("peakresidual") ) * psffraction ;

    define( RecordFieldId("cyclethreshold"), (Double) cyclethreshold );

  }


  Float SIIterBot::getCycleThreshold()
  {
    Float userthreshold = asFloat( RecordFieldId("threshold") );
    Float cyclethreshold = asFloat( RecordFieldId("cyclethreshold") );
    return userthreshold > cyclethreshold ? userthreshold : cyclethreshold;
  }


  Float SIIterBot::getLoopGain()
  {
    //verifyFields();
    return asFloat( RecordFieldId("loopgain") );
  }


  Float SIIterBot::getPeakResidual()
  {
    return asFloat( RecordFieldId("peakresidual") );
  }

  void SIIterBot::setPeakResidual(Float peakresidual)
  {
    define( RecordFieldId("peakresidual") , (Double)peakresidual ) ;
  }

  Float SIIterBot::getModelFlux()
  {
    return asFloat( RecordFieldId("modelflux") );
  }

  void SIIterBot::setModelFlux(Float modelflux)
  {
    define( RecordFieldId("modelflux") , (Double)modelflux ) ;
  }


  Bool SIIterBot::getIsModelUpdated()
  {
    return asBool( RecordFieldId("updatedmodel") );
  }
  
  void SIIterBot::setIsModelUpdated(Bool updatedmodel)
  {
    define( RecordFieldId("updatedmodel") , updatedmodel ) ;
  }


  /////////////////////////////////////////////////////////////////////////////////////////
  //// START //// Functions for runtime parameter-modification


  void SIIterBot::changeCycleThreshold( Float cyclethreshold )
  {
    define( RecordFieldId("cyclethreshold") , cyclethreshold );
  }

  void SIIterBot::changeThreshold( Float threshold )
  {
    define( RecordFieldId("threshold") , threshold );
  }

  void SIIterBot::changeNiter( Int niter )
  {
    define( RecordFieldId("niter") , niter );
  }

  void SIIterBot::changeMaxCycleNiter( Int maxcycleniter )
  {
    define( RecordFieldId("maxcycleniter") , maxcycleniter );
  }

  void SIIterBot::changeLoopGain( Float loopgain )
  {
    define( RecordFieldId("loopgain") , loopgain );
  }

  //// END //// Functions for runtime parameter-modification
  /////////////////////////////////////////////////////////////////////////////////////////



} //# NAMESPACE CASA - END

