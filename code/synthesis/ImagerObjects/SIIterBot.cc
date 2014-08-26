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

#include <synthesis/ImagerObjects/SIIterBot.h>
#include <casadbus/session/DBusSession.h>
#include <casadbus/utilities/Conversion.h>

/* Include file for the lock guard */
#include <boost/thread/locks.hpp>

/* Records Interface */
#include <casa/Containers/Record.h>
#include <casadbus/types/nullptr.h>		// for casa::memory::nullptr
#include <math.h>						// For FLT_MAX

namespace casa { //# NAMESPACE CASA - BEGIN

	////////////////////////////////////
	/// SIIterBot_callback implementation ///
	////////////////////////////////////
	void SIIterBot_callback::interactionRequired( bool val ) {
		boost::lock_guard<boost::recursive_mutex> guard(mutex); 
		if ( adaptor != 0 )
			adaptor->interactionRequired(val);
	}

	void SIIterBot_callback::addHandler( SIIterBot_adaptor *adapt ) {
		boost::lock_guard<boost::recursive_mutex> guard(mutex); 
		if ( adaptor == 0 )
			adaptor = adapt;
		
	}
	void SIIterBot_callback::removeHandler( SIIterBot_adaptor *adapt ) {
		boost::lock_guard<boost::recursive_mutex> guard(mutex); 
		if ( adaptor == adapt )
			adaptor = 0;
	}

  
	////////////////////////////////////
	/// SIIterBot_state implementation ///
	////////////////////////////////////
  
	// All SIIterBot_states must have 'type' and 'name' defined.
	SIIterBot_state::SIIterBot_state( std::tr1::shared_ptr<SIIterBot_callback> cb ) :
						itsDescription("no description is currently available..."),
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
						itsSummaryMajor(IPosition(1,0)),
						callback(cb)
	{
		LogIO os( LogOrigin("SIIterBot_state",__FUNCTION__,WHERE) );
	}
    
	SIIterBot_state::~SIIterBot_state() {
	  //		fprintf( stderr, ">>>>>>\t\tSIIterBot_state::~SIIterBot_state(0x%p)\n", this );
	  //		fflush( stderr );
	}

	bool SIIterBot_state::interactiveInputRequired( ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex); 
		return ( itsInteractiveMode &&
				 ( itsMaxCycleIterDone+itsInteractiveIterDone>=itsInteractiveNiter ||
				   itsPeakResidual <= itsInteractiveThreshold ||
				   itsPauseFlag ) );
	}

	void SIIterBot_state::waitForInteractiveInput() {
		/* Check that we have at least one controller */
		if (getNumberOfControllers() == 0) {
			/* Spawn a Viewer set up for interactive */
		}
		cout << "UU : setup interaction" << endl;
		boost::unique_lock<boost::mutex> lock(interactionMutex);
		if(!interactionPending) {
			interactionPending = true;
			pushDetails( );
			if ( memory::nullptr.check(callback) == false )
				callback->interactionRequired(interactionPending);
		}
		cout << "UU : about to wait" << endl;
		/* Wait on Condition variable */
		while (interactionPending) {
			interactionCond.wait(lock);
		}

		cout << "UU : returned from wait" << endl;
		if (updateNeeded) {
			updateNeeded = false;
			if ( memory::nullptr.check(callback) == false )
				callback->interactionRequired(false);
			pushDetails();
		}
	}

	int SIIterBot_state::cleanComplete(){
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    

		//		printOut("FromcleanComplete ", False);

		if ( itsMajorDone==0 && itsIterDone==0 ) return false;

		LogIO os( LogOrigin("SIIterBot_state",__FUNCTION__,WHERE) );

		int stopCode=0;

		if ( itsIterDone >= itsNiter || 
		     itsPeakResidual <= itsThreshold ||
		     itsStopFlag )
		  {
		    //		    os << "Reached global stopping criteria : ";

		    if( itsIterDone >= itsNiter ) { stopCode=1; }
		      //  os << "Numer of iterations. "; // (" << itsIterDone << ") >= limit (" << itsNiter << ")" ;
		    if( itsPeakResidual <= itsThreshold ) {stopCode=2; }
		      //os << "Peak residual (" << itsPeakResidual << ") <= threshold(" << itsThreshold << ")";
		    if( itsStopFlag ) {stopCode=3;}
		      //os << "Forced stop. ";
		    //		    os << LogIO::POST;

		    //return true;
		  }
		
		//		os << "Peak residual : " << itsPeakResidual << " and " << itsIterDone << " iterations."<< LogIO::POST;
		//		return false;
		return stopCode;
	}


	Record SIIterBot_state::getMinorCycleControls(){
		LogIO os( LogOrigin("SIIterBot_state",__FUNCTION__,WHERE) );
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
		/*
		if (itsInteractiveMode) {
			maxCycleIterations = min(maxCycleIterations, itsInteractiveNiter);
			cycleThreshold = max(cycleThreshold, itsInteractiveThreshold);
		}
		*/
		returnRecord.define( RecordFieldId("cycleniter"),  maxCycleIterations);
		returnRecord.define( RecordFieldId("cyclethreshold"), cycleThreshold);
		returnRecord.define( RecordFieldId("loopgain"), itsLoopGain);

		return returnRecord;
	}

	void SIIterBot_state::mergeCycleInitializationRecord(Record& initRecord){
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  
    
		itsPeakResidual = max( itsPeakResidual,
							   initRecord.asFloat(RecordFieldId("peakresidual")) );
		itsMaxPsfSidelobe =  max( itsMaxPsfSidelobe, initRecord.asFloat(RecordFieldId("maxpsfsidelobe")) );

	}


	void SIIterBot_state::mergeCycleExecutionRecord( Record& execRecord ){
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  

		LogIO os( LogOrigin("SIIterBot_state",__FUNCTION__,WHERE) );

		mergeMinorCycleSummary( execRecord.asArrayDouble( RecordFieldId("summaryminor")) );

		itsIterDone += execRecord.asInt(RecordFieldId("iterdone"));

		itsMaxCycleIterDone = max( itsMaxCycleIterDone, execRecord.asInt(RecordFieldId("maxcycleiterdone")) );

		itsPeakResidual = max( itsPeakResidual, execRecord.asFloat(RecordFieldId("peakresidual")) );
  
		itsUpdatedModelFlag |=execRecord.asBool( RecordFieldId("updatedmodelflag") );

		os << "Completed " << itsIterDone << " iterations." << LogIO::POST;
		//with peak residual "<< itsPeakResidual << LogIO::POST;
	}

	void SIIterBot_state::mergeMinorCycleSummary( const Array<Double>& summary ){
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);  
    
		IPosition cShp = itsSummaryMinor.shape();
		IPosition nShp = summary.shape();

		if( cShp.nelements() != 2 || cShp[0] != itsNSummaryFields ||
			nShp.nelements() != 2 || nShp[0] != itsNSummaryFields ) 
			throw(AipsError("Internal error in shape of global minor-cycle summary record"));

		itsSummaryMinor.resize( IPosition( 2, itsNSummaryFields, cShp[1]+nShp[1] ) ,True );

		for (unsigned int row = 0; row < nShp[1]; row++) {
			// iterations done
			itsSummaryMinor( IPosition(2,0,cShp[1]+row) ) = itsIterDone + summary(IPosition(2,0,row));  
			// peak residual
			itsSummaryMinor( IPosition(2,1,cShp[1]+row) ) = summary(IPosition(2,1,row)); 
			// model flux
			itsSummaryMinor( IPosition(2,2,cShp[1]+row) ) = summary(IPosition(2,2,row));
			// cycle threshold
			itsSummaryMinor( IPosition(2,3,cShp[1]+row) ) = summary(IPosition(2,3,row)); 
			// mapper id
			itsSummaryMinor( IPosition(2,4,cShp[1]+row) ) = summary(IPosition(2,4,row)); 
			// chunk id (channel/stokes)
			itsSummaryMinor( IPosition(2,5,cShp[1]+row) ) = summary(IPosition(2,5,row)); 
		}
	}
  
#ifdef INTERACTIVE_ITERATION
	void SIIterBot_state::controlUpdate( const std::map<std::string,DBus::Variant>& updatedParams ) {
		Record controlRecord=dbus::toRecord(updatedParams);
		setControlsFromRecord(controlRecord);
		{
			boost::lock_guard<boost::mutex> lock(interactionMutex);
			updateNeeded=true;
		}
	}
#endif

	void SIIterBot_state::interactionComplete() {
	  cout << "UU : Interaction completed" << endl;
		changePauseFlag(false);
		itsInteractiveIterDone = 0;    
		{
			boost::lock_guard<boost::mutex> lock(interactionMutex);
			interactionPending=false;
			updateNeeded=true;
		}
		interactionCond.notify_all();
	}

	std::string SIIterBot_state::getDescription( ) {
		boost::lock_guard<boost::recursive_mutex> guard(descriptionMutex);
		return itsDescription;
	}

	void SIIterBot_state::setDescription( const std::string &value ) {
		boost::lock_guard<boost::recursive_mutex> guard(descriptionMutex);    
		itsDescription = value;
	}

#ifdef INTERACTIVE_ITERATION
	std::map<std::string,DBus::Variant> SIIterBot_state::getDetails( ) {
		return dbus::fromRecord(getDetailsRecord());
	}
#endif

	void SIIterBot_state::pushDetails() {
#ifdef INTERACTIVE_ITERATION
/*FIXME    detailUpdate(fromRecord(getDetailsRecord())); */
#endif
	}

	void SIIterBot_state::pushSummary( ) {
		//boost::lock_guard<boost::recursive_mutex> guard(countMutex);
		std::cout << __FUNCTION__ << "executing" << std::endl;
		//    summaryUpdate();
	}

	DBus::Variant SIIterBot_state::getSummary( ) {
		std::cout << __FUNCTION__ << " executing" << std::endl;
		return DBus::Variant( );
	}

	int SIIterBot_state::getNumberOfControllers( ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		return itsControllerCount;
	}

	bool SIIterBot_state::incrementController( ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsControllerCount++;
		return true;
	}

	bool SIIterBot_state::decrementController( ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsControllerCount--;
		return true;
	} 

  /* ------------ End of runtime parameter getters -------- */

	void SIIterBot_state::incrementMajorCycleCount( ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
		itsMajorDone++;

		/* Interactive iteractions update */ 
		itsInteractiveIterDone += itsMaxCycleIterDone;

		/* Get ready to do the minor cycle */
		itsPeakResidual = 0;
		itsMaxPsfSidelobe = 0;
		itsMaxCycleIterDone = 0;
	}

	Int SIIterBot_state::getMajorCycleCount( ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
		return itsMajorDone;
	}
  
	Record SIIterBot_state::getSummaryRecord( ) {
		LogIO os( LogOrigin("SIIterBot_state",__FUNCTION__,WHERE) );
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
		Record returnRecord = getDetailsRecord();
		returnRecord.define( RecordFieldId("summaryminor"), itsSummaryMinor);
		returnRecord.define( RecordFieldId("summarymajor"), itsSummaryMajor);
		return returnRecord;
	}

  // TODO : Optimize this storage and resizing ? Or call this only now and then... ?

	void SIIterBot_state::addSummaryMajor( ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);

		IPosition shp = itsSummaryMajor.shape();
		if( shp.nelements() != 1 ) 
			throw(AipsError("Internal error in shape of major-cycle summary record"));

		itsSummaryMajor.resize( IPosition( 1, shp[0]+1 ) , True );
		itsSummaryMajor( IPosition(1, shp[0] ) ) = itsIterDone;
	}
  
	void SIIterBot_state::updateCycleThreshold( ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
    
		Float psffraction = itsMaxPsfSidelobe * itsCycleFactor;
    
		psffraction = max(psffraction, itsMinPsfFraction);
		psffraction = min(psffraction, itsMaxPsfFraction);
    
		itsCycleThreshold = itsPeakResidual * psffraction;
		pushDetails();
	}

//   Float SIIterBot_state::getMaxPsfSidelobe()
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     return itsMaxPsfSidelobe;
//   }

//   void SIIterBot_state::setMaxPsfSidelobe(Float maxSidelobe)
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     itsMaxPsfSidelobe = maxSidelobe;
//   }

//   Float SIIterBot_state::getMaxPsfFraction()
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     return itsMaxPsfFraction;
//   }

//   void SIIterBot_state::setMaxPsfFraction(Float maxPsfFraction)
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     itsMaxPsfFraction = maxPsfFraction;
//   }

//   Float SIIterBot_state::getMinPsfFraction()
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     return itsMinPsfFraction;
//   }

//   void SIIterBot_state::setMinPsfFraction(Float minPsfFraction)
//   {
//     boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
//     itsMinPsfFraction = minPsfFraction;
//   }

	Record SIIterBot_state::getDetailsRecord(){
		LogIO os( LogOrigin("SIIterBot_state",__FUNCTION__,WHERE) );
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
		Record returnRecord;

		/* Control Variables */
		returnRecord.define(RecordFieldId("niter"), itsNiter);
		returnRecord.define(RecordFieldId("cycleniter"), itsCycleNiter);
		returnRecord.define(RecordFieldId("interactiveniter"),itsInteractiveNiter);

		returnRecord.define( RecordFieldId("threshold"),  itsThreshold);    
		returnRecord.define( RecordFieldId("cyclethreshold"),itsCycleThreshold);
		returnRecord.define( RecordFieldId("interactivethreshold"), itsInteractiveThreshold);  

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
		returnRecord.define( RecordFieldId("interactivemode"), itsInteractiveMode);

		/* report clean's state */
		returnRecord.define( RecordFieldId("cleanstate"), itsStopFlag ? "stopped" :
		                                                  itsPauseFlag ? "paused" : "running" );
		return returnRecord;
	}

	void SIIterBot_state::changeNiter( Int niter ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsNiter = niter;
		//cout << "UUU : change niter : " << niter << endl;
	}

	void SIIterBot_state::changeCycleNiter( Int cycleniter ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
		if (cycleniter <= 0)  
			itsCycleNiter = itsNiter;
		else
			itsCycleNiter = cycleniter;
	}

	void SIIterBot_state::changeInteractiveNiter( Int interactiveNiter ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsInteractiveNiter = interactiveNiter;
	}

	void SIIterBot_state::changeThreshold( Float threshold ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsThreshold = threshold;
	}

	void SIIterBot_state::changeCycleThreshold( Float cyclethreshold ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
		itsCycleThreshold = cyclethreshold;
	}

	void SIIterBot_state::changeInteractiveThreshold( Float interactivethreshold ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
		itsInteractiveThreshold = interactivethreshold;
	}

	void SIIterBot_state::changeLoopGain( Float loopgain ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);
		itsLoopGain = loopgain;
	}

	void SIIterBot_state::changeCycleFactor( Float cyclefactor ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsCycleFactor = cyclefactor;
	}

	void SIIterBot_state::changeInteractiveMode( const bool& interactiveEnabled ) {
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsInteractiveMode = interactiveEnabled;
	}

	void SIIterBot_state::changePauseFlag( const bool& pauseEnabled ){
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsPauseFlag = pauseEnabled;
		cout << "UUU : changed pause flag to " << pauseEnabled << endl;
	}

	void SIIterBot_state::changeStopFlag(const bool& stopEnabled){
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);    
		itsStopFlag = stopEnabled;
	}

	void SIIterBot_state::setControlsFromRecord( Record &recordIn ) {
		LogIO os( LogOrigin("SIIterBot_state",__FUNCTION__,WHERE) );
		boost::lock_guard<boost::recursive_mutex> guard(recordMutex);

		/* Note it is important that niter get set first as we catch
		   negative values in the cycleniter, and set it equal to niter */
		if (recordIn.isDefined("niter"))
			changeNiter(recordIn.asInt( RecordFieldId("niter")));

		if (recordIn.isDefined("cycleniter"))
			changeCycleNiter(recordIn.asInt( RecordFieldId("cycleniter")));

		if (recordIn.isDefined("interactiveniter"))
			changeInteractiveNiter( recordIn.asInt(RecordFieldId("interactiveniter")) );
    
		if (recordIn.isDefined("threshold")) 
		  {
		    // Threshold can be a variant, either Float or String(with units).
		    Float fthresh=0.0;
		    // If a number, treat it as a number in units of Jy.
		    if( recordIn.dataType("threshold") == TpFloat || 
			recordIn.dataType("threshold") == TpDouble || 
			recordIn.dataType("threshold") == TpInt )
		      { fthresh = recordIn.asFloat( RecordFieldId("threshold")); }
		    // If a string, try to convert to a Quantity
		    else if( recordIn.dataType("threshold") == TpString )
		      {
			Quantity thresh; 
			// If it cannot be converted to a Quantity.... complain, and use zero.
			if( ! casa::Quantity::read( thresh, recordIn.asString( RecordFieldId("threshold") ) ) )
			  {os << LogIO::WARN << "Cannot parse threshold value. Setting to zero." << LogIO::POST;  
			    fthresh=0.0;}
			// If converted to Quantity, get value in Jy. 
			// ( Note : This does not check for wrong units, e.g. if the user says '100m' ! )
			else { fthresh = thresh.getValue(Unit("Jy")); }
		      }
		    // If neither valid datatype, print a warning and use zero.
		    else {os << LogIO::WARN << "Threshold is neither a number nor a string Quantity. Setting to zero." << LogIO::POST;
		      fthresh=0.0; }

		    // Set the threshold as a float. This is the data format used everywhere internally. 
		    changeThreshold( fthresh );
		  }
		
		if (recordIn.isDefined("cyclethreshold")) 
			changeCycleThreshold(recordIn.asFloat( RecordFieldId("cyclethreshold")));
    
		if (recordIn.isDefined("interactivethreshold")) 
			changeInteractiveThreshold(recordIn.asFloat(RecordFieldId("interactivethreshold")));

		if (recordIn.isDefined("loopgain")) 
			changeLoopGain(recordIn.asDouble( RecordFieldId("loopgain")));;

		if (recordIn.isDefined("cyclefactor"))
			changeCycleFactor(recordIn.asFloat( RecordFieldId("cyclefactor")));

		if (recordIn.isDefined("interactive"))
			changeInteractiveMode(recordIn.asBool(RecordFieldId("interactive")));

		printOut("After Setting : ", False);

	}

	/* Print out contents of the IterBot. For debugging. */
	void SIIterBot_state::printOut( String prefix, Bool verbose ) {
		if( verbose == True ) {
			cout << prefix << " : " 
				 << " ItsNiter=" << itsNiter
				 << " itsCycleNiter=" << itsCycleNiter
				 << " itsInteractiveNiter=" << itsInteractiveNiter
				 << " itsThreshold=" << itsThreshold
				 << " itsCycleThreshold=" << itsCycleThreshold
				 << " itsInteractiveThreshold=" << itsInteractiveThreshold
				 << " itsCycleFactor=" << itsCycleFactor
				 << " itsLoopGain=" << itsLoopGain
				 << " itsStopFlag=" << itsStopFlag
				 << " itsPauseFlag=" << itsPauseFlag
				 << " itsInteractiveMode=" << itsInteractiveMode
				 << " itsUpdatedModelFlag=" << itsUpdatedModelFlag
				 << " itsPeakResidual=" << itsPeakResidual
				 << " itsMaxPsfSidelobe=" << itsMaxPsfSidelobe
				 << " itsMinPsfFraction=" << itsMinPsfFraction
				 << " itsMaxPsfFraction=" << itsMaxPsfFraction
				 << " itsIterdone=" << itsIterDone
				 << " itsInteractiveIterDone=" << itsInteractiveIterDone
				 << " itsMaxCycleIterDone=" << itsMaxCycleIterDone
				 << " itsMajorDone=" << itsMajorDone
				 << endl;
		} else {
			cout << prefix << " : " 
				 << " ItsNiter=" << itsNiter
				 << " itsCycleNiter=" << itsCycleNiter
				 << " itsThreshold=" << itsThreshold
				 << " itsCycleThreshold=" << itsCycleThreshold
				 << " itsStopFlag=" << itsStopFlag
				 << " itsPeakResidual=" << itsPeakResidual
				 << " itsIterdone=" << itsIterDone
				 << endl;
		}

	}

	SIIterBot_adaptor::SIIterBot_adaptor( std::tr1::shared_ptr<SIIterBot_state> s, const std::string &bus_name, const std::string &object_path) :
#ifdef INTERACTIVE_ITERATION
				dbus::address(bus_name),
				DBus::ObjectAdaptor( DBusSession::instance().connection( ), object_path ),
#endif
				state(s) {
#ifdef INTERACTIVE_ITERATION
		state->acceptCallbacks(this);
#endif
	}

	SIIterBot_adaptor::~SIIterBot_adaptor() {
		fprintf( stderr, ">>>>>>\t\tSIIterBot_adaptor::~SIIterBot_adaptor(0x%p)\n", this );
		fflush( stderr );
#ifdef INTERACTIVE_ITERATION
		state->denyCallbacks(this);
		disconnect();
		
#endif
	}

} //# NAMESPACE CASA - END

