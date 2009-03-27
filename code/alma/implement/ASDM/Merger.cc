
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * Warning!
 *  -------------------------------------------------------------------- 
 * | This is generated code!  Do not modify this file.                  |
 * | If you do, all changes will be lost when the file is re-generated. |
 *  --------------------------------------------------------------------
 *
 * File Merger.h
 */
#include <Merger.h>

using namespace std;

namespace asdm {

	Merger::Merger() {
		this->ds1 = (ASDM *) 0;
		this->ds2 = (ASDM *) 0;
		

		Merger::mergeSBSummaryPtr = &Merger::mergeSBSummary;

		Merger::mergeConfigDescriptionPtr = &Merger::mergeConfigDescription;

		Merger::mergeFieldPtr = &Merger::mergeField;

		Merger::mergeStatePtr = &Merger::mergeState;

		Merger::mergeAntennaPtr = &Merger::mergeAntenna;

		Merger::mergeDataDescriptionPtr = &Merger::mergeDataDescription;

		Merger::mergeSwitchCyclePtr = &Merger::mergeSwitchCycle;

		Merger::mergeSourcePtr = &Merger::mergeSource;

		Merger::mergeFeedPtr = &Merger::mergeFeed;

		Merger::mergeSourceParameterPtr = &Merger::mergeSourceParameter;

		Merger::mergeSpectralWindowPtr = &Merger::mergeSpectralWindow;

		Merger::mergeFreqOffsetPtr = &Merger::mergeFreqOffset;

		Merger::mergePolarizationPtr = &Merger::mergePolarization;

		Merger::mergeReceiverPtr = &Merger::mergeReceiver;

		Merger::mergeBeamPtr = &Merger::mergeBeam;

		Merger::mergeDopplerPtr = &Merger::mergeDoppler;

		Merger::mergeProcessorPtr = &Merger::mergeProcessor;

		Merger::mergeCorrelatorModePtr = &Merger::mergeCorrelatorMode;

		Merger::mergeCalDevicePtr = &Merger::mergeCalDevice;

		Merger::mergeFlagCmdPtr = &Merger::mergeFlagCmd;

		Merger::mergeFocusPtr = &Merger::mergeFocus;

		Merger::mergeHistoryPtr = &Merger::mergeHistory;

		Merger::mergeObservationPtr = &Merger::mergeObservation;

		Merger::mergePointingPtr = &Merger::mergePointing;

		Merger::mergeSeeingPtr = &Merger::mergeSeeing;

		Merger::mergeSysCalPtr = &Merger::mergeSysCal;

		Merger::mergeTotalPowerPtr = &Merger::mergeTotalPower;

		Merger::mergeWeatherPtr = &Merger::mergeWeather;

		Merger::mergeWVMCalPtr = &Merger::mergeWVMCal;

		Merger::mergeEphemerisPtr = &Merger::mergeEphemeris;

		Merger::mergeExecBlockPtr = &Merger::mergeExecBlock;

		Merger::mergeScanPtr = &Merger::mergeScan;

		Merger::mergeSubscanPtr = &Merger::mergeSubscan;

		Merger::mergeMainPtr = &Merger::mergeMain;

		Merger::mergeFocusModelPtr = &Merger::mergeFocusModel;

		Merger::mergeGainTrackingPtr = &Merger::mergeGainTracking;

		Merger::mergePointingModelPtr = &Merger::mergePointingModel;

		Merger::mergeCalAmpliPtr = &Merger::mergeCalAmpli;

		Merger::mergeCalDataPtr = &Merger::mergeCalData;

		Merger::mergeCalReductionPtr = &Merger::mergeCalReduction;

		Merger::mergeCalPhasePtr = &Merger::mergeCalPhase;

		Merger::mergeCalSeeingPtr = &Merger::mergeCalSeeing;

		Merger::mergeCalPositionPtr = &Merger::mergeCalPosition;

		Merger::mergeCalPointingPtr = &Merger::mergeCalPointing;

		Merger::mergeCalPointingModelPtr = &Merger::mergeCalPointingModel;

		Merger::mergeCalHolographyPtr = &Merger::mergeCalHolography;

		Merger::mergeCalAtmospherePtr = &Merger::mergeCalAtmosphere;

		Merger::mergeCalCurvePtr = &Merger::mergeCalCurve;

		Merger::mergeStationPtr = &Merger::mergeStation;

		Merger::mergeAlmaRadiometerPtr = &Merger::mergeAlmaRadiometer;

		Merger::mergeSquareLawDetectorPtr = &Merger::mergeSquareLawDetector;

		Merger::mergeCalFocusPtr = &Merger::mergeCalFocus;

		Merger::mergeCalDelayPtr = &Merger::mergeCalDelay;

		Merger::mergeHolographyPtr = &Merger::mergeHolography;

		Merger::mergeCalBandpassPtr = &Merger::mergeCalBandpass;

		Merger::mergeCalFluxPtr = &Merger::mergeCalFlux;

		Merger::mergeCalFocusModelPtr = &Merger::mergeCalFocusModel;

		Merger::mergeCalGainPtr = &Merger::mergeCalGain;

		Merger::mergeCalPrimaryBeamPtr = &Merger::mergeCalPrimaryBeam;

		Merger::mergeCalWVRPtr = &Merger::mergeCalWVR;

		Merger::mergeAnnotationPtr = &Merger::mergeAnnotation;

	}
	
	Merger::~Merger() {

	}
	
	void Merger::merge(ASDM* ds1, ASDM* ds2) {
		this->ds1 = ds1;
		this->ds2 = ds2;


		hasMergedSBSummary = false;

		hasMergedConfigDescription = false;

		hasMergedField = false;

		hasMergedState = false;

		hasMergedAntenna = false;

		hasMergedDataDescription = false;

		hasMergedSwitchCycle = false;

		hasMergedSource = false;

		hasMergedFeed = false;

		hasMergedSourceParameter = false;

		hasMergedSpectralWindow = false;

		hasMergedFreqOffset = false;

		hasMergedPolarization = false;

		hasMergedReceiver = false;

		hasMergedBeam = false;

		hasMergedDoppler = false;

		hasMergedProcessor = false;

		hasMergedCorrelatorMode = false;

		hasMergedCalDevice = false;

		hasMergedFlagCmd = false;

		hasMergedFocus = false;

		hasMergedHistory = false;

		hasMergedObservation = false;

		hasMergedPointing = false;

		hasMergedSeeing = false;

		hasMergedSysCal = false;

		hasMergedTotalPower = false;

		hasMergedWeather = false;

		hasMergedWVMCal = false;

		hasMergedEphemeris = false;

		hasMergedExecBlock = false;

		hasMergedScan = false;

		hasMergedSubscan = false;

		hasMergedMain = false;

		hasMergedFocusModel = false;

		hasMergedGainTracking = false;

		hasMergedPointingModel = false;

		hasMergedCalAmpli = false;

		hasMergedCalData = false;

		hasMergedCalReduction = false;

		hasMergedCalPhase = false;

		hasMergedCalSeeing = false;

		hasMergedCalPosition = false;

		hasMergedCalPointing = false;

		hasMergedCalPointingModel = false;

		hasMergedCalHolography = false;

		hasMergedCalAtmosphere = false;

		hasMergedCalCurve = false;

		hasMergedStation = false;

		hasMergedAlmaRadiometer = false;

		hasMergedSquareLawDetector = false;

		hasMergedCalFocus = false;

		hasMergedCalDelay = false;

		hasMergedHolography = false;

		hasMergedCalBandpass = false;

		hasMergedCalFlux = false;

		hasMergedCalFocusModel = false;

		hasMergedCalGain = false;

		hasMergedCalPrimaryBeam = false;

		hasMergedCalWVR = false;

		hasMergedAnnotation = false;


		mergeSBSummary( );

		mergeConfigDescription( );

		mergeField( );

		mergeState( );

		mergeAntenna( );

		mergeDataDescription( );

		mergeSwitchCycle( );

		mergeSource( );

		mergeFeed( );

		mergeSourceParameter( );

		mergeSpectralWindow( );

		mergeFreqOffset( );

		mergePolarization( );

		mergeReceiver( );

		mergeBeam( );

		mergeDoppler( );

		mergeProcessor( );

		mergeCorrelatorMode( );

		mergeCalDevice( );

		mergeFlagCmd( );

		mergeFocus( );

		mergeHistory( );

		mergeObservation( );

		mergePointing( );

		mergeSeeing( );

		mergeSysCal( );

		mergeTotalPower( );

		mergeWeather( );

		mergeWVMCal( );

		mergeEphemeris( );

		mergeExecBlock( );

		mergeScan( );

		mergeSubscan( );

		mergeMain( );

		mergeFocusModel( );

		mergeGainTracking( );

		mergePointingModel( );

		mergeCalAmpli( );

		mergeCalData( );

		mergeCalReduction( );

		mergeCalPhase( );

		mergeCalSeeing( );

		mergeCalPosition( );

		mergeCalPointing( );

		mergeCalPointingModel( );

		mergeCalHolography( );

		mergeCalAtmosphere( );

		mergeCalCurve( );

		mergeStation( );

		mergeAlmaRadiometer( );

		mergeSquareLawDetector( );

		mergeCalFocus( );

		mergeCalDelay( );

		mergeHolography( );

		mergeCalBandpass( );

		mergeCalFlux( );

		mergeCalFocusModel( );

		mergeCalGain( );

		mergeCalPrimaryBeam( );

		mergeCalWVR( );

		mergeAnnotation( );


		postMergeSBSummary( );

		postMergeConfigDescription( );

		postMergeField( );

		postMergeState( );

		postMergeAntenna( );

		postMergeDataDescription( );

		postMergeSwitchCycle( );

		postMergeSource( );

		postMergeFeed( );

		postMergeSourceParameter( );

		postMergeSpectralWindow( );

		postMergeFreqOffset( );

		postMergePolarization( );

		postMergeReceiver( );

		postMergeBeam( );

		postMergeDoppler( );

		postMergeProcessor( );

		postMergeCorrelatorMode( );

		postMergeCalDevice( );

		postMergeFlagCmd( );

		postMergeFocus( );

		postMergeHistory( );

		postMergeObservation( );

		postMergePointing( );

		postMergeSeeing( );

		postMergeSysCal( );

		postMergeTotalPower( );

		postMergeWeather( );

		postMergeWVMCal( );

		postMergeEphemeris( );

		postMergeExecBlock( );

		postMergeScan( );

		postMergeSubscan( );

		postMergeMain( );

		postMergeFocusModel( );

		postMergeGainTracking( );

		postMergePointingModel( );

		postMergeCalAmpli( );

		postMergeCalData( );

		postMergeCalReduction( );

		postMergeCalPhase( );

		postMergeCalSeeing( );

		postMergeCalPosition( );

		postMergeCalPointing( );

		postMergeCalPointingModel( );

		postMergeCalHolography( );

		postMergeCalAtmosphere( );

		postMergeCalCurve( );

		postMergeStation( );

		postMergeAlmaRadiometer( );

		postMergeSquareLawDetector( );

		postMergeCalFocus( );

		postMergeCalDelay( );

		postMergeHolography( );

		postMergeCalBandpass( );

		postMergeCalFlux( );

		postMergeCalFocusModel( );

		postMergeCalGain( );

		postMergeCalPrimaryBeam( );

		postMergeCalWVR( );

		postMergeAnnotation( );
			
	}
	
	

	void Merger::mergeSBSummary() {
		cout << "Entering Merger::mergeSBSummary" << endl;
		if (hasMergedSBSummary) return;
	
		hasMergedSBSummary = true;
		cout << "Exiting Merger::mergeSBSummary" << endl;
	}
	
	void Merger::postMergeSBSummary() {
		cout << "Entering Merger::postMergeSBSummary" << endl;
	
		cout << "Exiting Merger::postMergeSBSummary" << endl;
	}			

	void Merger::mergeConfigDescription() {
		cout << "Entering Merger::mergeConfigDescription" << endl;
		if (hasMergedConfigDescription) return;
	
		vector <ConfigDescriptionRow *> rows2 = ds2->getConfigDescription().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			ConfigDescriptionRow * row = ds1->getConfigDescription().newRow(rows2.at(i));
		
			
				
			vector<Tag> antennaId2 = rows2.at(i)->getAntennaId();
			vector<Tag> antennaId1;
			for (unsigned int j = 0; j < antennaId2.size(); j++)
				
				antennaId1.push_back(getTag(antennaId2.at(j), mergeAntennaPtr));
				
			row->setAntennaId(	antennaId1);
			
		
			
				
			vector<Tag> dataDescriptionId2 = rows2.at(i)->getDataDescriptionId();
			vector<Tag> dataDescriptionId1;
			for (unsigned int j = 0; j < dataDescriptionId2.size(); j++)
				
				dataDescriptionId1.push_back(getTag(dataDescriptionId2.at(j), mergeDataDescriptionPtr));
				
			row->setDataDescriptionId(	dataDescriptionId1);
			
		
			
				
			vector<int> feedId2 = rows2.at(i)->getFeedId();
			vector<int> feedId1;
			for (unsigned int j = 0; j < feedId2.size(); j++)
				
				feedId1.push_back(getId("Feed", feedId2.at(j), mergeFeedPtr));
				
			row->setFeedId(	feedId1);
			
		
			
				
				
			Tag processorIdTag = getTag(row->getProcessorId(), mergeProcessorPtr);
			row->setProcessorId(processorIdTag);
				
			
		
			
				
			vector<Tag> switchCycleId2 = rows2.at(i)->getSwitchCycleId();
			vector<Tag> switchCycleId1;
			for (unsigned int j = 0; j < switchCycleId2.size(); j++)
				
				switchCycleId1.push_back(getTag(switchCycleId2.at(j), mergeSwitchCyclePtr));
				
			row->setSwitchCycleId(	switchCycleId1);
			
		
			ConfigDescriptionRow * retRow = ds1->getConfigDescription().add(row);
		
			
			tagTag.insert(make_pair(rows2.at(i)->getConfigDescriptionId().toString(), retRow->getConfigDescriptionId()));
			
		
		}
	
		hasMergedConfigDescription = true;
		cout << "Exiting Merger::mergeConfigDescription" << endl;
	}
	
	void Merger::postMergeConfigDescription() {
		cout << "Entering Merger::postMergeConfigDescription" << endl;
	
		vector <ConfigDescriptionRow *> rows1 = ds1->getConfigDescription().get();
		ConfigDescriptionRow *row1 = 0;
		for (unsigned int i = 0; i < rows1.size(); i++) {
			row1 = rows1.at(i);
		
			
			if (row1->isAssocConfigDescriptionIdExists()) {
				
				
				vector<Tag> assocConfigDescriptionId1 = row1->getAssocConfigDescriptionId();
				vector<Tag> assocConfigDescriptionId1_new;
				for (unsigned int j = 0; j < assocConfigDescriptionId1.size(); j++) {
					
					assocConfigDescriptionId1_new.push_back(getTag( assocConfigDescriptionId1.at(j), 0));
					
				}
				row1->setAssocConfigDescriptionId(	assocConfigDescriptionId1_new);
				
			}
			
		
		}
	
		cout << "Exiting Merger::postMergeConfigDescription" << endl;
	}			

	void Merger::mergeField() {
		cout << "Entering Merger::mergeField" << endl;
		if (hasMergedField) return;
	
		hasMergedField = true;
		cout << "Exiting Merger::mergeField" << endl;
	}
	
	void Merger::postMergeField() {
		cout << "Entering Merger::postMergeField" << endl;
	
		vector <FieldRow *> rows1 = ds1->getField().get();
		FieldRow *row1 = 0;
		for (unsigned int i = 0; i < rows1.size(); i++) {
			row1 = rows1.at(i);
		
			
			if (row1->isAssocFieldIdExists()) {
				
				
				vector<Tag> assocFieldId1 = row1->getAssocFieldId();
				vector<Tag> assocFieldId1_new;
				for (unsigned int j = 0; j < assocFieldId1.size(); j++) {
					
					assocFieldId1_new.push_back(getTag( assocFieldId1.at(j), 0));
					
				}
				row1->setAssocFieldId(	assocFieldId1_new);
				
			}
			
		
			
			if (row1->isEphemerisIdExists()) {
				
				
				
					
			row1->setEphemerisId(getTag(row1->getEphemerisId(), 0));
					
				
			}
			
		
			
			if (row1->isSourceIdExists()) {
				
				
				
					
			row1->setSourceId(getId("Source", row1->getSourceId(), 0));
					
				
			}
			
		
		}
	
		cout << "Exiting Merger::postMergeField" << endl;
	}			

	void Merger::mergeState() {
		cout << "Entering Merger::mergeState" << endl;
		if (hasMergedState) return;
	
		hasMergedState = true;
		cout << "Exiting Merger::mergeState" << endl;
	}
	
	void Merger::postMergeState() {
		cout << "Entering Merger::postMergeState" << endl;
	
		cout << "Exiting Merger::postMergeState" << endl;
	}			

	void Merger::mergeAntenna() {
		cout << "Entering Merger::mergeAntenna" << endl;
		if (hasMergedAntenna) return;
	
		vector <AntennaRow *> rows2 = ds2->getAntenna().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			AntennaRow * row = ds1->getAntenna().newRow(rows2.at(i));
		
			
				
				
			Tag stationIdTag = getTag(row->getStationId(), mergeStationPtr);
			row->setStationId(stationIdTag);
				
			
		
			AntennaRow * retRow = ds1->getAntenna().add(row);
		
			
			tagTag.insert(make_pair(rows2.at(i)->getAntennaId().toString(), retRow->getAntennaId()));
			
		
		}
	
		hasMergedAntenna = true;
		cout << "Exiting Merger::mergeAntenna" << endl;
	}
	
	void Merger::postMergeAntenna() {
		cout << "Entering Merger::postMergeAntenna" << endl;
	
		vector <AntennaRow *> rows1 = ds1->getAntenna().get();
		AntennaRow *row1 = 0;
		for (unsigned int i = 0; i < rows1.size(); i++) {
			row1 = rows1.at(i);
		
			
			if (row1->isAssocAntennaIdExists()) {
				
				
				
					
			row1->setAssocAntennaId(getTag(row1->getAssocAntennaId(), 0));
					
				
			}
			
		
		}
	
		cout << "Exiting Merger::postMergeAntenna" << endl;
	}			

	void Merger::mergeDataDescription() {
		cout << "Entering Merger::mergeDataDescription" << endl;
		if (hasMergedDataDescription) return;
	
		vector <DataDescriptionRow *> rows2 = ds2->getDataDescription().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			DataDescriptionRow * row = ds1->getDataDescription().newRow(rows2.at(i));
		
			
				
				
			Tag polOrHoloIdTag = getTag(row->getPolOrHoloId(), mergePolarizationPtr);
			row->setPolOrHoloId(polOrHoloIdTag);
				
			
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			DataDescriptionRow * retRow = ds1->getDataDescription().add(row);
		
			
			tagTag.insert(make_pair(rows2.at(i)->getDataDescriptionId().toString(), retRow->getDataDescriptionId()));
			
		
		}
	
		hasMergedDataDescription = true;
		cout << "Exiting Merger::mergeDataDescription" << endl;
	}
	
	void Merger::postMergeDataDescription() {
		cout << "Entering Merger::postMergeDataDescription" << endl;
	
		cout << "Exiting Merger::postMergeDataDescription" << endl;
	}			

	void Merger::mergeSwitchCycle() {
		cout << "Entering Merger::mergeSwitchCycle" << endl;
		if (hasMergedSwitchCycle) return;
	
		hasMergedSwitchCycle = true;
		cout << "Exiting Merger::mergeSwitchCycle" << endl;
	}
	
	void Merger::postMergeSwitchCycle() {
		cout << "Entering Merger::postMergeSwitchCycle" << endl;
	
		cout << "Exiting Merger::postMergeSwitchCycle" << endl;
	}			

	void Merger::mergeSource() {
		cout << "Entering Merger::mergeSource" << endl;
		if (hasMergedSource) return;
	
		vector <SourceRow *> rows2 = ds2->getSource().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			SourceRow * row = ds1->getSource().newRow(rows2.at(i));
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			SourceRow * retRow = ds1->getSource().add(row);
		
			
			idId.insert(make_pair("Source_"+Integer::toString(rows2.at(i)->getSourceId()), retRow->getSourceId()));
			
		
		}
	
		hasMergedSource = true;
		cout << "Exiting Merger::mergeSource" << endl;
	}
	
	void Merger::postMergeSource() {
		cout << "Entering Merger::postMergeSource" << endl;
	
		vector <SourceRow *> rows1 = ds1->getSource().get();
		SourceRow *row1 = 0;
		for (unsigned int i = 0; i < rows1.size(); i++) {
			row1 = rows1.at(i);
		
			
			if (row1->isSourceParameterIdExists()) {
				
				
				
					
			row1->setSourceParameterId(getId("SourceParameter", row1->getSourceParameterId(), 0));
					
				
			}
			
		
		}
	
		cout << "Exiting Merger::postMergeSource" << endl;
	}			

	void Merger::mergeFeed() {
		cout << "Entering Merger::mergeFeed" << endl;
		if (hasMergedFeed) return;
	
		vector <FeedRow *> rows2 = ds2->getFeed().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			FeedRow * row = ds1->getFeed().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			
				
			vector<int> receiverId2 = rows2.at(i)->getReceiverId();
			vector<int> receiverId1;
			for (unsigned int j = 0; j < receiverId2.size(); j++)
				
				receiverId1.push_back(getId("Receiver", receiverId2.at(j), mergeReceiverPtr));
				
			row->setReceiverId(	receiverId1);
			
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			FeedRow * retRow = ds1->getFeed().add(row);
		
			
			idId.insert(make_pair("Feed_"+Integer::toString(rows2.at(i)->getFeedId()), retRow->getFeedId()));
			
		
		}
	
		hasMergedFeed = true;
		cout << "Exiting Merger::mergeFeed" << endl;
	}
	
	void Merger::postMergeFeed() {
		cout << "Entering Merger::postMergeFeed" << endl;
	
		vector <FeedRow *> rows1 = ds1->getFeed().get();
		FeedRow *row1 = 0;
		for (unsigned int i = 0; i < rows1.size(); i++) {
			row1 = rows1.at(i);
		
			
			if (row1->isBeamIdExists()) {
				
				
				vector<Tag> beamId1 = row1->getBeamId();
				vector<Tag> beamId1_new;
				for (unsigned int j = 0; j < beamId1.size(); j++) {
					
					beamId1_new.push_back(getTag( beamId1.at(j), 0));
					
				}
				row1->setBeamId(	beamId1_new);
				
			}
			
		
		}
	
		cout << "Exiting Merger::postMergeFeed" << endl;
	}			

	void Merger::mergeSourceParameter() {
		cout << "Entering Merger::mergeSourceParameter" << endl;
		if (hasMergedSourceParameter) return;
	
		vector <SourceParameterRow *> rows2 = ds2->getSourceParameter().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			SourceParameterRow * row = ds1->getSourceParameter().newRow(rows2.at(i));
		
			
				
				
			row->setSourceId(getId("Source", row->getSourceId(), mergeSourcePtr));
				
			
		
			SourceParameterRow * retRow = ds1->getSourceParameter().add(row);
		
			
			idId.insert(make_pair("SourceParameter_"+Integer::toString(rows2.at(i)->getSourceParameterId()), retRow->getSourceParameterId()));
			
		
		}
	
		hasMergedSourceParameter = true;
		cout << "Exiting Merger::mergeSourceParameter" << endl;
	}
	
	void Merger::postMergeSourceParameter() {
		cout << "Entering Merger::postMergeSourceParameter" << endl;
	
		cout << "Exiting Merger::postMergeSourceParameter" << endl;
	}			

	void Merger::mergeSpectralWindow() {
		cout << "Entering Merger::mergeSpectralWindow" << endl;
		if (hasMergedSpectralWindow) return;
	
		hasMergedSpectralWindow = true;
		cout << "Exiting Merger::mergeSpectralWindow" << endl;
	}
	
	void Merger::postMergeSpectralWindow() {
		cout << "Entering Merger::postMergeSpectralWindow" << endl;
	
		vector <SpectralWindowRow *> rows1 = ds1->getSpectralWindow().get();
		SpectralWindowRow *row1 = 0;
		for (unsigned int i = 0; i < rows1.size(); i++) {
			row1 = rows1.at(i);
		
			
			if (row1->isAssocSpectralWindowIdExists()) {
				
				
				vector<Tag> assocSpectralWindowId1 = row1->getAssocSpectralWindowId();
				vector<Tag> assocSpectralWindowId1_new;
				for (unsigned int j = 0; j < assocSpectralWindowId1.size(); j++) {
					
					assocSpectralWindowId1_new.push_back(getTag( assocSpectralWindowId1.at(j), 0));
					
				}
				row1->setAssocSpectralWindowId(	assocSpectralWindowId1_new);
				
			}
			
		
			
			if (row1->isDopplerIdExists()) {
				
				
				
					
			row1->setDopplerId(getId("Doppler", row1->getDopplerId(), 0));
					
				
			}
			
		
			
			if (row1->isImageSpectralWindowIdExists()) {
				
				
				
					
			row1->setImageSpectralWindowId(getTag(row1->getImageSpectralWindowId(), 0));
					
				
			}
			
		
		}
	
		cout << "Exiting Merger::postMergeSpectralWindow" << endl;
	}			

	void Merger::mergeFreqOffset() {
		cout << "Entering Merger::mergeFreqOffset" << endl;
		if (hasMergedFreqOffset) return;
	
		vector <FreqOffsetRow *> rows2 = ds2->getFreqOffset().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			FreqOffsetRow * row = ds1->getFreqOffset().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			
				
				
			row->setFeedId(getId("Feed", row->getFeedId(), mergeFeedPtr));
				
			
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			FreqOffsetRow * retRow = ds1->getFreqOffset().add(row);
		
		}
	
		hasMergedFreqOffset = true;
		cout << "Exiting Merger::mergeFreqOffset" << endl;
	}
	
	void Merger::postMergeFreqOffset() {
		cout << "Entering Merger::postMergeFreqOffset" << endl;
	
		cout << "Exiting Merger::postMergeFreqOffset" << endl;
	}			

	void Merger::mergePolarization() {
		cout << "Entering Merger::mergePolarization" << endl;
		if (hasMergedPolarization) return;
	
		hasMergedPolarization = true;
		cout << "Exiting Merger::mergePolarization" << endl;
	}
	
	void Merger::postMergePolarization() {
		cout << "Entering Merger::postMergePolarization" << endl;
	
		cout << "Exiting Merger::postMergePolarization" << endl;
	}			

	void Merger::mergeReceiver() {
		cout << "Entering Merger::mergeReceiver" << endl;
		if (hasMergedReceiver) return;
	
		vector <ReceiverRow *> rows2 = ds2->getReceiver().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			ReceiverRow * row = ds1->getReceiver().newRow(rows2.at(i));
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			ReceiverRow * retRow = ds1->getReceiver().add(row);
		
			
			idId.insert(make_pair("Receiver_"+Integer::toString(rows2.at(i)->getReceiverId()), retRow->getReceiverId()));
			
		
		}
	
		hasMergedReceiver = true;
		cout << "Exiting Merger::mergeReceiver" << endl;
	}
	
	void Merger::postMergeReceiver() {
		cout << "Entering Merger::postMergeReceiver" << endl;
	
		cout << "Exiting Merger::postMergeReceiver" << endl;
	}			

	void Merger::mergeBeam() {
		cout << "Entering Merger::mergeBeam" << endl;
		if (hasMergedBeam) return;
	
		hasMergedBeam = true;
		cout << "Exiting Merger::mergeBeam" << endl;
	}
	
	void Merger::postMergeBeam() {
		cout << "Entering Merger::postMergeBeam" << endl;
	
		cout << "Exiting Merger::postMergeBeam" << endl;
	}			

	void Merger::mergeDoppler() {
		cout << "Entering Merger::mergeDoppler" << endl;
		if (hasMergedDoppler) return;
	
		vector <DopplerRow *> rows2 = ds2->getDoppler().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			DopplerRow * row = ds1->getDoppler().newRow(rows2.at(i));
		
			
				
				
			row->setSourceId(getId("Source", row->getSourceId(), mergeSourcePtr));
				
			
		
			DopplerRow * retRow = ds1->getDoppler().add(row);
		
			
			idId.insert(make_pair("Doppler_"+Integer::toString(rows2.at(i)->getDopplerId()), retRow->getDopplerId()));
			
		
		}
	
		hasMergedDoppler = true;
		cout << "Exiting Merger::mergeDoppler" << endl;
	}
	
	void Merger::postMergeDoppler() {
		cout << "Entering Merger::postMergeDoppler" << endl;
	
		cout << "Exiting Merger::postMergeDoppler" << endl;
	}			

	void Merger::mergeProcessor() {
		cout << "Entering Merger::mergeProcessor" << endl;
		if (hasMergedProcessor) return;
	
		vector <ProcessorRow *> rows2 = ds2->getProcessor().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			ProcessorRow * row = ds1->getProcessor().newRow(rows2.at(i));
		
			
				
				
			Tag almaCorrelatorModeIdTag = getTag(row->getAlmaCorrelatorModeId(), mergeCorrelatorModePtr);
			row->setAlmaCorrelatorModeId(almaCorrelatorModeIdTag);
				
			
		
			ProcessorRow * retRow = ds1->getProcessor().add(row);
		
			
			tagTag.insert(make_pair(rows2.at(i)->getProcessorId().toString(), retRow->getProcessorId()));
			
		
		}
	
		hasMergedProcessor = true;
		cout << "Exiting Merger::mergeProcessor" << endl;
	}
	
	void Merger::postMergeProcessor() {
		cout << "Entering Merger::postMergeProcessor" << endl;
	
		cout << "Exiting Merger::postMergeProcessor" << endl;
	}			

	void Merger::mergeCorrelatorMode() {
		cout << "Entering Merger::mergeCorrelatorMode" << endl;
		if (hasMergedCorrelatorMode) return;
	
		hasMergedCorrelatorMode = true;
		cout << "Exiting Merger::mergeCorrelatorMode" << endl;
	}
	
	void Merger::postMergeCorrelatorMode() {
		cout << "Entering Merger::postMergeCorrelatorMode" << endl;
	
		cout << "Exiting Merger::postMergeCorrelatorMode" << endl;
	}			

	void Merger::mergeCalDevice() {
		cout << "Entering Merger::mergeCalDevice" << endl;
		if (hasMergedCalDevice) return;
	
		vector <CalDeviceRow *> rows2 = ds2->getCalDevice().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalDeviceRow * row = ds1->getCalDevice().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			
				
				
			row->setFeedId(getId("Feed", row->getFeedId(), mergeFeedPtr));
				
			
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			CalDeviceRow * retRow = ds1->getCalDevice().add(row);
		
		}
	
		hasMergedCalDevice = true;
		cout << "Exiting Merger::mergeCalDevice" << endl;
	}
	
	void Merger::postMergeCalDevice() {
		cout << "Entering Merger::postMergeCalDevice" << endl;
	
		cout << "Exiting Merger::postMergeCalDevice" << endl;
	}			

	void Merger::mergeFlagCmd() {
		cout << "Entering Merger::mergeFlagCmd" << endl;
		if (hasMergedFlagCmd) return;
	
		hasMergedFlagCmd = true;
		cout << "Exiting Merger::mergeFlagCmd" << endl;
	}
	
	void Merger::postMergeFlagCmd() {
		cout << "Entering Merger::postMergeFlagCmd" << endl;
	
		cout << "Exiting Merger::postMergeFlagCmd" << endl;
	}			

	void Merger::mergeFocus() {
		cout << "Entering Merger::mergeFocus" << endl;
		if (hasMergedFocus) return;
	
		vector <FocusRow *> rows2 = ds2->getFocus().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			FocusRow * row = ds1->getFocus().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			
				
				
			row->setFeedId(getId("Feed", row->getFeedId(), mergeFeedPtr));
				
			
		
			
				
				
			Tag focusModelIdTag = getTag(row->getFocusModelId(), mergeFocusModelPtr);
			row->setFocusModelId(focusModelIdTag);
				
			
		
			FocusRow * retRow = ds1->getFocus().add(row);
		
		}
	
		hasMergedFocus = true;
		cout << "Exiting Merger::mergeFocus" << endl;
	}
	
	void Merger::postMergeFocus() {
		cout << "Entering Merger::postMergeFocus" << endl;
	
		cout << "Exiting Merger::postMergeFocus" << endl;
	}			

	void Merger::mergeHistory() {
		cout << "Entering Merger::mergeHistory" << endl;
		if (hasMergedHistory) return;
	
		vector <HistoryRow *> rows2 = ds2->getHistory().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			HistoryRow * row = ds1->getHistory().newRow(rows2.at(i));
		
			
				
				
			Tag execBlockIdTag = getTag(row->getExecBlockId(), mergeExecBlockPtr);
			row->setExecBlockId(execBlockIdTag);
				
			
		
			HistoryRow * retRow = ds1->getHistory().add(row);
		
		}
	
		hasMergedHistory = true;
		cout << "Exiting Merger::mergeHistory" << endl;
	}
	
	void Merger::postMergeHistory() {
		cout << "Entering Merger::postMergeHistory" << endl;
	
		cout << "Exiting Merger::postMergeHistory" << endl;
	}			

	void Merger::mergeObservation() {
		cout << "Entering Merger::mergeObservation" << endl;
		if (hasMergedObservation) return;
	
		hasMergedObservation = true;
		cout << "Exiting Merger::mergeObservation" << endl;
	}
	
	void Merger::postMergeObservation() {
		cout << "Entering Merger::postMergeObservation" << endl;
	
		cout << "Exiting Merger::postMergeObservation" << endl;
	}			

	void Merger::mergePointing() {
		cout << "Entering Merger::mergePointing" << endl;
		if (hasMergedPointing) return;
	
		vector <PointingRow *> rows2 = ds2->getPointing().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			PointingRow * row = ds1->getPointing().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			
				
				
			row->setPointingModelId(getId("PointingModel", row->getPointingModelId(), mergePointingModelPtr));
				
			
		
			PointingRow * retRow = ds1->getPointing().add(row);
		
		}
	
		hasMergedPointing = true;
		cout << "Exiting Merger::mergePointing" << endl;
	}
	
	void Merger::postMergePointing() {
		cout << "Entering Merger::postMergePointing" << endl;
	
		cout << "Exiting Merger::postMergePointing" << endl;
	}			

	void Merger::mergeSeeing() {
		cout << "Entering Merger::mergeSeeing" << endl;
		if (hasMergedSeeing) return;
	
		hasMergedSeeing = true;
		cout << "Exiting Merger::mergeSeeing" << endl;
	}
	
	void Merger::postMergeSeeing() {
		cout << "Entering Merger::postMergeSeeing" << endl;
	
		cout << "Exiting Merger::postMergeSeeing" << endl;
	}			

	void Merger::mergeSysCal() {
		cout << "Entering Merger::mergeSysCal" << endl;
		if (hasMergedSysCal) return;
	
		vector <SysCalRow *> rows2 = ds2->getSysCal().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			SysCalRow * row = ds1->getSysCal().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			
				
				
			row->setFeedId(getId("Feed", row->getFeedId(), mergeFeedPtr));
				
			
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			SysCalRow * retRow = ds1->getSysCal().add(row);
		
		}
	
		hasMergedSysCal = true;
		cout << "Exiting Merger::mergeSysCal" << endl;
	}
	
	void Merger::postMergeSysCal() {
		cout << "Entering Merger::postMergeSysCal" << endl;
	
		cout << "Exiting Merger::postMergeSysCal" << endl;
	}			

	void Merger::mergeTotalPower() {
		cout << "Entering Merger::mergeTotalPower" << endl;
		if (hasMergedTotalPower) return;
	
		vector <TotalPowerRow *> rows2 = ds2->getTotalPower().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			TotalPowerRow * row = ds1->getTotalPower().newRow(rows2.at(i));
		
			
				
				
			Tag configDescriptionIdTag = getTag(row->getConfigDescriptionId(), mergeConfigDescriptionPtr);
			row->setConfigDescriptionId(configDescriptionIdTag);
				
			
		
			
				
				
			Tag execBlockIdTag = getTag(row->getExecBlockId(), mergeExecBlockPtr);
			row->setExecBlockId(execBlockIdTag);
				
			
		
			
				
				
			Tag fieldIdTag = getTag(row->getFieldId(), mergeFieldPtr);
			row->setFieldId(fieldIdTag);
				
			
		
			
				
			vector<Tag> stateId2 = rows2.at(i)->getStateId();
			vector<Tag> stateId1;
			for (unsigned int j = 0; j < stateId2.size(); j++)
				
				stateId1.push_back(getTag(stateId2.at(j), mergeStatePtr));
				
			row->setStateId(	stateId1);
			
		
			TotalPowerRow * retRow = ds1->getTotalPower().add(row);
		
		}
	
		hasMergedTotalPower = true;
		cout << "Exiting Merger::mergeTotalPower" << endl;
	}
	
	void Merger::postMergeTotalPower() {
		cout << "Entering Merger::postMergeTotalPower" << endl;
	
		cout << "Exiting Merger::postMergeTotalPower" << endl;
	}			

	void Merger::mergeWeather() {
		cout << "Entering Merger::mergeWeather" << endl;
		if (hasMergedWeather) return;
	
		vector <WeatherRow *> rows2 = ds2->getWeather().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			WeatherRow * row = ds1->getWeather().newRow(rows2.at(i));
		
			
				
				
			Tag stationIdTag = getTag(row->getStationId(), mergeStationPtr);
			row->setStationId(stationIdTag);
				
			
		
			WeatherRow * retRow = ds1->getWeather().add(row);
		
		}
	
		hasMergedWeather = true;
		cout << "Exiting Merger::mergeWeather" << endl;
	}
	
	void Merger::postMergeWeather() {
		cout << "Entering Merger::postMergeWeather" << endl;
	
		cout << "Exiting Merger::postMergeWeather" << endl;
	}			

	void Merger::mergeWVMCal() {
		cout << "Entering Merger::mergeWVMCal" << endl;
		if (hasMergedWVMCal) return;
	
		vector <WVMCalRow *> rows2 = ds2->getWVMCal().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			WVMCalRow * row = ds1->getWVMCal().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			WVMCalRow * retRow = ds1->getWVMCal().add(row);
		
		}
	
		hasMergedWVMCal = true;
		cout << "Exiting Merger::mergeWVMCal" << endl;
	}
	
	void Merger::postMergeWVMCal() {
		cout << "Entering Merger::postMergeWVMCal" << endl;
	
		cout << "Exiting Merger::postMergeWVMCal" << endl;
	}			

	void Merger::mergeEphemeris() {
		cout << "Entering Merger::mergeEphemeris" << endl;
		if (hasMergedEphemeris) return;
	
		hasMergedEphemeris = true;
		cout << "Exiting Merger::mergeEphemeris" << endl;
	}
	
	void Merger::postMergeEphemeris() {
		cout << "Entering Merger::postMergeEphemeris" << endl;
	
		cout << "Exiting Merger::postMergeEphemeris" << endl;
	}			

	void Merger::mergeExecBlock() {
		cout << "Entering Merger::mergeExecBlock" << endl;
		if (hasMergedExecBlock) return;
	
		vector <ExecBlockRow *> rows2 = ds2->getExecBlock().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			ExecBlockRow * row = ds1->getExecBlock().newRow(rows2.at(i));
		
			
				
			vector<Tag> antennaId2 = rows2.at(i)->getAntennaId();
			vector<Tag> antennaId1;
			for (unsigned int j = 0; j < antennaId2.size(); j++)
				
				antennaId1.push_back(getTag(antennaId2.at(j), mergeAntennaPtr));
				
			row->setAntennaId(	antennaId1);
			
		
			ExecBlockRow * retRow = ds1->getExecBlock().add(row);
		
			
			tagTag.insert(make_pair(rows2.at(i)->getExecBlockId().toString(), retRow->getExecBlockId()));
			
		
		}
	
		hasMergedExecBlock = true;
		cout << "Exiting Merger::mergeExecBlock" << endl;
	}
	
	void Merger::postMergeExecBlock() {
		cout << "Entering Merger::postMergeExecBlock" << endl;
	
		cout << "Exiting Merger::postMergeExecBlock" << endl;
	}			

	void Merger::mergeScan() {
		cout << "Entering Merger::mergeScan" << endl;
		if (hasMergedScan) return;
	
		vector <ScanRow *> rows2 = ds2->getScan().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			ScanRow * row = ds1->getScan().newRow(rows2.at(i));
		
			
				
				
			Tag execBlockIdTag = getTag(row->getExecBlockId(), mergeExecBlockPtr);
			row->setExecBlockId(execBlockIdTag);
				
			
		
			ScanRow * retRow = ds1->getScan().add(row);
		
		}
	
		hasMergedScan = true;
		cout << "Exiting Merger::mergeScan" << endl;
	}
	
	void Merger::postMergeScan() {
		cout << "Entering Merger::postMergeScan" << endl;
	
		cout << "Exiting Merger::postMergeScan" << endl;
	}			

	void Merger::mergeSubscan() {
		cout << "Entering Merger::mergeSubscan" << endl;
		if (hasMergedSubscan) return;
	
		vector <SubscanRow *> rows2 = ds2->getSubscan().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			SubscanRow * row = ds1->getSubscan().newRow(rows2.at(i));
		
			
				
				
			Tag execBlockIdTag = getTag(row->getExecBlockId(), mergeExecBlockPtr);
			row->setExecBlockId(execBlockIdTag);
				
			
		
			
				
				
			
		
			SubscanRow * retRow = ds1->getSubscan().add(row);
		
		}
	
		hasMergedSubscan = true;
		cout << "Exiting Merger::mergeSubscan" << endl;
	}
	
	void Merger::postMergeSubscan() {
		cout << "Entering Merger::postMergeSubscan" << endl;
	
		cout << "Exiting Merger::postMergeSubscan" << endl;
	}			

	void Merger::mergeMain() {
		cout << "Entering Merger::mergeMain" << endl;
		if (hasMergedMain) return;
	
		vector <MainRow *> rows2 = ds2->getMain().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			MainRow * row = ds1->getMain().newRow(rows2.at(i));
		
			
				
				
			Tag configDescriptionIdTag = getTag(row->getConfigDescriptionId(), mergeConfigDescriptionPtr);
			row->setConfigDescriptionId(configDescriptionIdTag);
				
			
		
			
				
				
			Tag execBlockIdTag = getTag(row->getExecBlockId(), mergeExecBlockPtr);
			row->setExecBlockId(execBlockIdTag);
				
			
		
			
				
				
			Tag fieldIdTag = getTag(row->getFieldId(), mergeFieldPtr);
			row->setFieldId(fieldIdTag);
				
			
		
			
				
			vector<Tag> stateId2 = rows2.at(i)->getStateId();
			vector<Tag> stateId1;
			for (unsigned int j = 0; j < stateId2.size(); j++)
				
				stateId1.push_back(getTag(stateId2.at(j), mergeStatePtr));
				
			row->setStateId(	stateId1);
			
		
			MainRow * retRow = ds1->getMain().add(row);
		
		}
	
		hasMergedMain = true;
		cout << "Exiting Merger::mergeMain" << endl;
	}
	
	void Merger::postMergeMain() {
		cout << "Entering Merger::postMergeMain" << endl;
	
		cout << "Exiting Merger::postMergeMain" << endl;
	}			

	void Merger::mergeFocusModel() {
		cout << "Entering Merger::mergeFocusModel" << endl;
		if (hasMergedFocusModel) return;
	
		hasMergedFocusModel = true;
		cout << "Exiting Merger::mergeFocusModel" << endl;
	}
	
	void Merger::postMergeFocusModel() {
		cout << "Entering Merger::postMergeFocusModel" << endl;
	
		cout << "Exiting Merger::postMergeFocusModel" << endl;
	}			

	void Merger::mergeGainTracking() {
		cout << "Entering Merger::mergeGainTracking" << endl;
		if (hasMergedGainTracking) return;
	
		vector <GainTrackingRow *> rows2 = ds2->getGainTracking().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			GainTrackingRow * row = ds1->getGainTracking().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			
				
				
			row->setFeedId(getId("Feed", row->getFeedId(), mergeFeedPtr));
				
			
		
			
				
				
			Tag spectralWindowIdTag = getTag(row->getSpectralWindowId(), mergeSpectralWindowPtr);
			row->setSpectralWindowId(spectralWindowIdTag);
				
			
		
			GainTrackingRow * retRow = ds1->getGainTracking().add(row);
		
		}
	
		hasMergedGainTracking = true;
		cout << "Exiting Merger::mergeGainTracking" << endl;
	}
	
	void Merger::postMergeGainTracking() {
		cout << "Entering Merger::postMergeGainTracking" << endl;
	
		cout << "Exiting Merger::postMergeGainTracking" << endl;
	}			

	void Merger::mergePointingModel() {
		cout << "Entering Merger::mergePointingModel" << endl;
		if (hasMergedPointingModel) return;
	
		vector <PointingModelRow *> rows2 = ds2->getPointingModel().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			PointingModelRow * row = ds1->getPointingModel().newRow(rows2.at(i));
		
			
				
				
			Tag antennaIdTag = getTag(row->getAntennaId(), mergeAntennaPtr);
			row->setAntennaId(antennaIdTag);
				
			
		
			PointingModelRow * retRow = ds1->getPointingModel().add(row);
		
			
			idId.insert(make_pair("PointingModel_"+Integer::toString(rows2.at(i)->getPointingModelId()), retRow->getPointingModelId()));
			
		
		}
	
		hasMergedPointingModel = true;
		cout << "Exiting Merger::mergePointingModel" << endl;
	}
	
	void Merger::postMergePointingModel() {
		cout << "Entering Merger::postMergePointingModel" << endl;
	
		cout << "Exiting Merger::postMergePointingModel" << endl;
	}			

	void Merger::mergeCalAmpli() {
		cout << "Entering Merger::mergeCalAmpli" << endl;
		if (hasMergedCalAmpli) return;
	
		vector <CalAmpliRow *> rows2 = ds2->getCalAmpli().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalAmpliRow * row = ds1->getCalAmpli().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalAmpliRow * retRow = ds1->getCalAmpli().add(row);
		
		}
	
		hasMergedCalAmpli = true;
		cout << "Exiting Merger::mergeCalAmpli" << endl;
	}
	
	void Merger::postMergeCalAmpli() {
		cout << "Entering Merger::postMergeCalAmpli" << endl;
	
		cout << "Exiting Merger::postMergeCalAmpli" << endl;
	}			

	void Merger::mergeCalData() {
		cout << "Entering Merger::mergeCalData" << endl;
		if (hasMergedCalData) return;
	
		hasMergedCalData = true;
		cout << "Exiting Merger::mergeCalData" << endl;
	}
	
	void Merger::postMergeCalData() {
		cout << "Entering Merger::postMergeCalData" << endl;
	
		cout << "Exiting Merger::postMergeCalData" << endl;
	}			

	void Merger::mergeCalReduction() {
		cout << "Entering Merger::mergeCalReduction" << endl;
		if (hasMergedCalReduction) return;
	
		hasMergedCalReduction = true;
		cout << "Exiting Merger::mergeCalReduction" << endl;
	}
	
	void Merger::postMergeCalReduction() {
		cout << "Entering Merger::postMergeCalReduction" << endl;
	
		cout << "Exiting Merger::postMergeCalReduction" << endl;
	}			

	void Merger::mergeCalPhase() {
		cout << "Entering Merger::mergeCalPhase" << endl;
		if (hasMergedCalPhase) return;
	
		vector <CalPhaseRow *> rows2 = ds2->getCalPhase().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalPhaseRow * row = ds1->getCalPhase().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalPhaseRow * retRow = ds1->getCalPhase().add(row);
		
		}
	
		hasMergedCalPhase = true;
		cout << "Exiting Merger::mergeCalPhase" << endl;
	}
	
	void Merger::postMergeCalPhase() {
		cout << "Entering Merger::postMergeCalPhase" << endl;
	
		cout << "Exiting Merger::postMergeCalPhase" << endl;
	}			

	void Merger::mergeCalSeeing() {
		cout << "Entering Merger::mergeCalSeeing" << endl;
		if (hasMergedCalSeeing) return;
	
		vector <CalSeeingRow *> rows2 = ds2->getCalSeeing().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalSeeingRow * row = ds1->getCalSeeing().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalSeeingRow * retRow = ds1->getCalSeeing().add(row);
		
		}
	
		hasMergedCalSeeing = true;
		cout << "Exiting Merger::mergeCalSeeing" << endl;
	}
	
	void Merger::postMergeCalSeeing() {
		cout << "Entering Merger::postMergeCalSeeing" << endl;
	
		cout << "Exiting Merger::postMergeCalSeeing" << endl;
	}			

	void Merger::mergeCalPosition() {
		cout << "Entering Merger::mergeCalPosition" << endl;
		if (hasMergedCalPosition) return;
	
		vector <CalPositionRow *> rows2 = ds2->getCalPosition().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalPositionRow * row = ds1->getCalPosition().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalPositionRow * retRow = ds1->getCalPosition().add(row);
		
		}
	
		hasMergedCalPosition = true;
		cout << "Exiting Merger::mergeCalPosition" << endl;
	}
	
	void Merger::postMergeCalPosition() {
		cout << "Entering Merger::postMergeCalPosition" << endl;
	
		cout << "Exiting Merger::postMergeCalPosition" << endl;
	}			

	void Merger::mergeCalPointing() {
		cout << "Entering Merger::mergeCalPointing" << endl;
		if (hasMergedCalPointing) return;
	
		vector <CalPointingRow *> rows2 = ds2->getCalPointing().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalPointingRow * row = ds1->getCalPointing().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalPointingRow * retRow = ds1->getCalPointing().add(row);
		
		}
	
		hasMergedCalPointing = true;
		cout << "Exiting Merger::mergeCalPointing" << endl;
	}
	
	void Merger::postMergeCalPointing() {
		cout << "Entering Merger::postMergeCalPointing" << endl;
	
		cout << "Exiting Merger::postMergeCalPointing" << endl;
	}			

	void Merger::mergeCalPointingModel() {
		cout << "Entering Merger::mergeCalPointingModel" << endl;
		if (hasMergedCalPointingModel) return;
	
		vector <CalPointingModelRow *> rows2 = ds2->getCalPointingModel().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalPointingModelRow * row = ds1->getCalPointingModel().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalPointingModelRow * retRow = ds1->getCalPointingModel().add(row);
		
		}
	
		hasMergedCalPointingModel = true;
		cout << "Exiting Merger::mergeCalPointingModel" << endl;
	}
	
	void Merger::postMergeCalPointingModel() {
		cout << "Entering Merger::postMergeCalPointingModel" << endl;
	
		cout << "Exiting Merger::postMergeCalPointingModel" << endl;
	}			

	void Merger::mergeCalHolography() {
		cout << "Entering Merger::mergeCalHolography" << endl;
		if (hasMergedCalHolography) return;
	
		vector <CalHolographyRow *> rows2 = ds2->getCalHolography().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalHolographyRow * row = ds1->getCalHolography().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalHolographyRow * retRow = ds1->getCalHolography().add(row);
		
		}
	
		hasMergedCalHolography = true;
		cout << "Exiting Merger::mergeCalHolography" << endl;
	}
	
	void Merger::postMergeCalHolography() {
		cout << "Entering Merger::postMergeCalHolography" << endl;
	
		cout << "Exiting Merger::postMergeCalHolography" << endl;
	}			

	void Merger::mergeCalAtmosphere() {
		cout << "Entering Merger::mergeCalAtmosphere" << endl;
		if (hasMergedCalAtmosphere) return;
	
		vector <CalAtmosphereRow *> rows2 = ds2->getCalAtmosphere().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalAtmosphereRow * row = ds1->getCalAtmosphere().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalAtmosphereRow * retRow = ds1->getCalAtmosphere().add(row);
		
		}
	
		hasMergedCalAtmosphere = true;
		cout << "Exiting Merger::mergeCalAtmosphere" << endl;
	}
	
	void Merger::postMergeCalAtmosphere() {
		cout << "Entering Merger::postMergeCalAtmosphere" << endl;
	
		cout << "Exiting Merger::postMergeCalAtmosphere" << endl;
	}			

	void Merger::mergeCalCurve() {
		cout << "Entering Merger::mergeCalCurve" << endl;
		if (hasMergedCalCurve) return;
	
		vector <CalCurveRow *> rows2 = ds2->getCalCurve().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalCurveRow * row = ds1->getCalCurve().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalCurveRow * retRow = ds1->getCalCurve().add(row);
		
		}
	
		hasMergedCalCurve = true;
		cout << "Exiting Merger::mergeCalCurve" << endl;
	}
	
	void Merger::postMergeCalCurve() {
		cout << "Entering Merger::postMergeCalCurve" << endl;
	
		cout << "Exiting Merger::postMergeCalCurve" << endl;
	}			

	void Merger::mergeStation() {
		cout << "Entering Merger::mergeStation" << endl;
		if (hasMergedStation) return;
	
		hasMergedStation = true;
		cout << "Exiting Merger::mergeStation" << endl;
	}
	
	void Merger::postMergeStation() {
		cout << "Entering Merger::postMergeStation" << endl;
	
		cout << "Exiting Merger::postMergeStation" << endl;
	}			

	void Merger::mergeAlmaRadiometer() {
		cout << "Entering Merger::mergeAlmaRadiometer" << endl;
		if (hasMergedAlmaRadiometer) return;
	
		hasMergedAlmaRadiometer = true;
		cout << "Exiting Merger::mergeAlmaRadiometer" << endl;
	}
	
	void Merger::postMergeAlmaRadiometer() {
		cout << "Entering Merger::postMergeAlmaRadiometer" << endl;
	
		cout << "Exiting Merger::postMergeAlmaRadiometer" << endl;
	}			

	void Merger::mergeSquareLawDetector() {
		cout << "Entering Merger::mergeSquareLawDetector" << endl;
		if (hasMergedSquareLawDetector) return;
	
		hasMergedSquareLawDetector = true;
		cout << "Exiting Merger::mergeSquareLawDetector" << endl;
	}
	
	void Merger::postMergeSquareLawDetector() {
		cout << "Entering Merger::postMergeSquareLawDetector" << endl;
	
		cout << "Exiting Merger::postMergeSquareLawDetector" << endl;
	}			

	void Merger::mergeCalFocus() {
		cout << "Entering Merger::mergeCalFocus" << endl;
		if (hasMergedCalFocus) return;
	
		vector <CalFocusRow *> rows2 = ds2->getCalFocus().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalFocusRow * row = ds1->getCalFocus().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalFocusRow * retRow = ds1->getCalFocus().add(row);
		
		}
	
		hasMergedCalFocus = true;
		cout << "Exiting Merger::mergeCalFocus" << endl;
	}
	
	void Merger::postMergeCalFocus() {
		cout << "Entering Merger::postMergeCalFocus" << endl;
	
		cout << "Exiting Merger::postMergeCalFocus" << endl;
	}			

	void Merger::mergeCalDelay() {
		cout << "Entering Merger::mergeCalDelay" << endl;
		if (hasMergedCalDelay) return;
	
		vector <CalDelayRow *> rows2 = ds2->getCalDelay().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalDelayRow * row = ds1->getCalDelay().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalDelayRow * retRow = ds1->getCalDelay().add(row);
		
		}
	
		hasMergedCalDelay = true;
		cout << "Exiting Merger::mergeCalDelay" << endl;
	}
	
	void Merger::postMergeCalDelay() {
		cout << "Entering Merger::postMergeCalDelay" << endl;
	
		cout << "Exiting Merger::postMergeCalDelay" << endl;
	}			

	void Merger::mergeHolography() {
		cout << "Entering Merger::mergeHolography" << endl;
		if (hasMergedHolography) return;
	
		hasMergedHolography = true;
		cout << "Exiting Merger::mergeHolography" << endl;
	}
	
	void Merger::postMergeHolography() {
		cout << "Entering Merger::postMergeHolography" << endl;
	
		cout << "Exiting Merger::postMergeHolography" << endl;
	}			

	void Merger::mergeCalBandpass() {
		cout << "Entering Merger::mergeCalBandpass" << endl;
		if (hasMergedCalBandpass) return;
	
		vector <CalBandpassRow *> rows2 = ds2->getCalBandpass().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalBandpassRow * row = ds1->getCalBandpass().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalBandpassRow * retRow = ds1->getCalBandpass().add(row);
		
		}
	
		hasMergedCalBandpass = true;
		cout << "Exiting Merger::mergeCalBandpass" << endl;
	}
	
	void Merger::postMergeCalBandpass() {
		cout << "Entering Merger::postMergeCalBandpass" << endl;
	
		cout << "Exiting Merger::postMergeCalBandpass" << endl;
	}			

	void Merger::mergeCalFlux() {
		cout << "Entering Merger::mergeCalFlux" << endl;
		if (hasMergedCalFlux) return;
	
		vector <CalFluxRow *> rows2 = ds2->getCalFlux().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalFluxRow * row = ds1->getCalFlux().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalFluxRow * retRow = ds1->getCalFlux().add(row);
		
		}
	
		hasMergedCalFlux = true;
		cout << "Exiting Merger::mergeCalFlux" << endl;
	}
	
	void Merger::postMergeCalFlux() {
		cout << "Entering Merger::postMergeCalFlux" << endl;
	
		cout << "Exiting Merger::postMergeCalFlux" << endl;
	}			

	void Merger::mergeCalFocusModel() {
		cout << "Entering Merger::mergeCalFocusModel" << endl;
		if (hasMergedCalFocusModel) return;
	
		vector <CalFocusModelRow *> rows2 = ds2->getCalFocusModel().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalFocusModelRow * row = ds1->getCalFocusModel().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalFocusModelRow * retRow = ds1->getCalFocusModel().add(row);
		
		}
	
		hasMergedCalFocusModel = true;
		cout << "Exiting Merger::mergeCalFocusModel" << endl;
	}
	
	void Merger::postMergeCalFocusModel() {
		cout << "Entering Merger::postMergeCalFocusModel" << endl;
	
		cout << "Exiting Merger::postMergeCalFocusModel" << endl;
	}			

	void Merger::mergeCalGain() {
		cout << "Entering Merger::mergeCalGain" << endl;
		if (hasMergedCalGain) return;
	
		vector <CalGainRow *> rows2 = ds2->getCalGain().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalGainRow * row = ds1->getCalGain().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalGainRow * retRow = ds1->getCalGain().add(row);
		
		}
	
		hasMergedCalGain = true;
		cout << "Exiting Merger::mergeCalGain" << endl;
	}
	
	void Merger::postMergeCalGain() {
		cout << "Entering Merger::postMergeCalGain" << endl;
	
		cout << "Exiting Merger::postMergeCalGain" << endl;
	}			

	void Merger::mergeCalPrimaryBeam() {
		cout << "Entering Merger::mergeCalPrimaryBeam" << endl;
		if (hasMergedCalPrimaryBeam) return;
	
		vector <CalPrimaryBeamRow *> rows2 = ds2->getCalPrimaryBeam().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalPrimaryBeamRow * row = ds1->getCalPrimaryBeam().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalPrimaryBeamRow * retRow = ds1->getCalPrimaryBeam().add(row);
		
		}
	
		hasMergedCalPrimaryBeam = true;
		cout << "Exiting Merger::mergeCalPrimaryBeam" << endl;
	}
	
	void Merger::postMergeCalPrimaryBeam() {
		cout << "Entering Merger::postMergeCalPrimaryBeam" << endl;
	
		cout << "Exiting Merger::postMergeCalPrimaryBeam" << endl;
	}			

	void Merger::mergeCalWVR() {
		cout << "Entering Merger::mergeCalWVR" << endl;
		if (hasMergedCalWVR) return;
	
		vector <CalWVRRow *> rows2 = ds2->getCalWVR().get();
		for (unsigned int i = 0; i < rows2.size(); i++) {
			CalWVRRow * row = ds1->getCalWVR().newRow(rows2.at(i));
		
			
				
				
			Tag calDataIdTag = getTag(row->getCalDataId(), mergeCalDataPtr);
			row->setCalDataId(calDataIdTag);
				
			
		
			
				
				
			Tag calReductionIdTag = getTag(row->getCalReductionId(), mergeCalReductionPtr);
			row->setCalReductionId(calReductionIdTag);
				
			
		
			CalWVRRow * retRow = ds1->getCalWVR().add(row);
		
		}
	
		hasMergedCalWVR = true;
		cout << "Exiting Merger::mergeCalWVR" << endl;
	}
	
	void Merger::postMergeCalWVR() {
		cout << "Entering Merger::postMergeCalWVR" << endl;
	
		cout << "Exiting Merger::postMergeCalWVR" << endl;
	}			

	void Merger::mergeAnnotation() {
		cout << "Entering Merger::mergeAnnotation" << endl;
		if (hasMergedAnnotation) return;
	
		hasMergedAnnotation = true;
		cout << "Exiting Merger::mergeAnnotation" << endl;
	}
	
	void Merger::postMergeAnnotation() {
		cout << "Entering Merger::postMergeAnnotation" << endl;
	
		vector <AnnotationRow *> rows1 = ds1->getAnnotation().get();
		AnnotationRow *row1 = 0;
		for (unsigned int i = 0; i < rows1.size(); i++) {
			row1 = rows1.at(i);
		
			
			if (row1->isAntennaIdExists()) {
				
				
				vector<Tag> antennaId1 = row1->getAntennaId();
				vector<Tag> antennaId1_new;
				for (unsigned int j = 0; j < antennaId1.size(); j++) {
					
					antennaId1_new.push_back(getTag( antennaId1.at(j), 0));
					
				}
				row1->setAntennaId(	antennaId1_new);
				
			}
			
		
		}
	
		cout << "Exiting Merger::postMergeAnnotation" << endl;
	}			


	Tag Merger::getTag(const Tag& t, void (Merger::*mergeTables)()) {
		if (mergeTables != 0) (this->*mergeTables)();
		
		
		
		map<string, Tag>::iterator iter = tagTag.find(t.toString());
		if (iter!=tagTag.end()) return iter->second;
		
		// Should throw an exception here.
		cout << "Could not find the mapped Tag for " << t.toString() << endl;
		return Tag();
	}
	
	int Merger::getId(const string& tableName, int id, void (Merger::*mergeTables)()) {
		if (mergeTables != 0) (this->*mergeTables)();
		
		map<string, int>::iterator iter = idId.find(tableName+"_"+Integer::toString(id));
		if (iter != idId.end()) return iter->second;
		
		// Should throw an exception here.
		cout << "Could not find the mapped Id for " << id << endl;
		return -1;
	}

} // End namespace asdm

