
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
#ifndef Merger_CLASS
#define Merger_CLASS
#include <map>

#include "ASDM.h"

#include "SBSummaryTable.h"
#include "SBSummaryRow.h"

#include "ConfigDescriptionTable.h"
#include "ConfigDescriptionRow.h"

#include "FieldTable.h"
#include "FieldRow.h"

#include "StateTable.h"
#include "StateRow.h"

#include "AntennaTable.h"
#include "AntennaRow.h"

#include "DataDescriptionTable.h"
#include "DataDescriptionRow.h"

#include "SwitchCycleTable.h"
#include "SwitchCycleRow.h"

#include "SourceTable.h"
#include "SourceRow.h"

#include "FeedTable.h"
#include "FeedRow.h"

#include "SpectralWindowTable.h"
#include "SpectralWindowRow.h"

#include "FreqOffsetTable.h"
#include "FreqOffsetRow.h"

#include "PolarizationTable.h"
#include "PolarizationRow.h"

#include "ReceiverTable.h"
#include "ReceiverRow.h"

#include "BeamTable.h"
#include "BeamRow.h"

#include "DopplerTable.h"
#include "DopplerRow.h"

#include "ProcessorTable.h"
#include "ProcessorRow.h"

#include "CorrelatorModeTable.h"
#include "CorrelatorModeRow.h"

#include "CalDeviceTable.h"
#include "CalDeviceRow.h"

#include "FlagCmdTable.h"
#include "FlagCmdRow.h"

#include "FocusTable.h"
#include "FocusRow.h"

#include "HistoryTable.h"
#include "HistoryRow.h"

#include "ObservationTable.h"
#include "ObservationRow.h"

#include "PointingTable.h"
#include "PointingRow.h"

#include "SeeingTable.h"
#include "SeeingRow.h"

#include "SysCalTable.h"
#include "SysCalRow.h"

#include "TotalPowerTable.h"
#include "TotalPowerRow.h"

#include "WeatherTable.h"
#include "WeatherRow.h"

#include "WVMCalTable.h"
#include "WVMCalRow.h"

#include "EphemerisTable.h"
#include "EphemerisRow.h"

#include "ExecBlockTable.h"
#include "ExecBlockRow.h"

#include "ScanTable.h"
#include "ScanRow.h"

#include "SubscanTable.h"
#include "SubscanRow.h"

#include "MainTable.h"
#include "MainRow.h"

#include "FocusModelTable.h"
#include "FocusModelRow.h"

#include "GainTrackingTable.h"
#include "GainTrackingRow.h"

#include "PointingModelTable.h"
#include "PointingModelRow.h"

#include "CalAmpliTable.h"
#include "CalAmpliRow.h"

#include "CalDataTable.h"
#include "CalDataRow.h"

#include "CalReductionTable.h"
#include "CalReductionRow.h"

#include "CalPhaseTable.h"
#include "CalPhaseRow.h"

#include "CalSeeingTable.h"
#include "CalSeeingRow.h"

#include "CalPositionTable.h"
#include "CalPositionRow.h"

#include "CalPointingTable.h"
#include "CalPointingRow.h"

#include "CalPointingModelTable.h"
#include "CalPointingModelRow.h"

#include "CalHolographyTable.h"
#include "CalHolographyRow.h"

#include "CalAtmosphereTable.h"
#include "CalAtmosphereRow.h"

#include "CalCurveTable.h"
#include "CalCurveRow.h"

#include "StationTable.h"
#include "StationRow.h"

#include "AlmaRadiometerTable.h"
#include "AlmaRadiometerRow.h"

#include "SquareLawDetectorTable.h"
#include "SquareLawDetectorRow.h"

#include "CalFocusTable.h"
#include "CalFocusRow.h"

#include "CalDelayTable.h"
#include "CalDelayRow.h"

#include "HolographyTable.h"
#include "HolographyRow.h"

#include "CalBandpassTable.h"
#include "CalBandpassRow.h"

#include "CalFluxTable.h"
#include "CalFluxRow.h"

#include "CalFocusModelTable.h"
#include "CalFocusModelRow.h"

#include "CalGainTable.h"
#include "CalGainRow.h"

#include "CalPrimaryBeamTable.h"
#include "CalPrimaryBeamRow.h"

#include "CalWVRTable.h"
#include "CalWVRRow.h"

#include "AnnotationTable.h"
#include "AnnotationRow.h"

#include "DelayModelTable.h"
#include "DelayModelRow.h"



/*\file "Merger.h"
    \brief Generated from model's revision "1.52", branch "HEAD"
*/
using namespace std;

namespace asdm {
	class Merger {
		public :
			Merger();
			virtual ~Merger();
			
			void merge(ASDM* ds1, ASDM* ds2);
			
		private:
			ASDM* ds1;
			ASDM* ds2;
			map<string, Tag> tagTag;
			Tag getTag(const Tag& t, void (Merger::*mergeTables)());
			
			map<string, int> idId;
			int getId(const string& tableName, int id, void (Merger::*mergeTables)()); 
			

			bool hasMergedSBSummary;	

			bool hasMergedConfigDescription;	

			bool hasMergedField;	

			bool hasMergedState;	

			bool hasMergedAntenna;	

			bool hasMergedDataDescription;	

			bool hasMergedSwitchCycle;	

			bool hasMergedSource;	

			bool hasMergedFeed;	

			bool hasMergedSpectralWindow;	

			bool hasMergedFreqOffset;	

			bool hasMergedPolarization;	

			bool hasMergedReceiver;	

			bool hasMergedBeam;	

			bool hasMergedDoppler;	

			bool hasMergedProcessor;	

			bool hasMergedCorrelatorMode;	

			bool hasMergedCalDevice;	

			bool hasMergedFlagCmd;	

			bool hasMergedFocus;	

			bool hasMergedHistory;	

			bool hasMergedObservation;	

			bool hasMergedPointing;	

			bool hasMergedSeeing;	

			bool hasMergedSysCal;	

			bool hasMergedTotalPower;	

			bool hasMergedWeather;	

			bool hasMergedWVMCal;	

			bool hasMergedEphemeris;	

			bool hasMergedExecBlock;	

			bool hasMergedScan;	

			bool hasMergedSubscan;	

			bool hasMergedMain;	

			bool hasMergedFocusModel;	

			bool hasMergedGainTracking;	

			bool hasMergedPointingModel;	

			bool hasMergedCalAmpli;	

			bool hasMergedCalData;	

			bool hasMergedCalReduction;	

			bool hasMergedCalPhase;	

			bool hasMergedCalSeeing;	

			bool hasMergedCalPosition;	

			bool hasMergedCalPointing;	

			bool hasMergedCalPointingModel;	

			bool hasMergedCalHolography;	

			bool hasMergedCalAtmosphere;	

			bool hasMergedCalCurve;	

			bool hasMergedStation;	

			bool hasMergedAlmaRadiometer;	

			bool hasMergedSquareLawDetector;	

			bool hasMergedCalFocus;	

			bool hasMergedCalDelay;	

			bool hasMergedHolography;	

			bool hasMergedCalBandpass;	

			bool hasMergedCalFlux;	

			bool hasMergedCalFocusModel;	

			bool hasMergedCalGain;	

			bool hasMergedCalPrimaryBeam;	

			bool hasMergedCalWVR;	

			bool hasMergedAnnotation;	

			bool hasMergedDelayModel;	
			


			void mergeSBSummary();
			void postMergeSBSummary();			

			void mergeConfigDescription();
			void postMergeConfigDescription();			

			void mergeField();
			void postMergeField();			

			void mergeState();
			void postMergeState();			

			void mergeAntenna();
			void postMergeAntenna();			

			void mergeDataDescription();
			void postMergeDataDescription();			

			void mergeSwitchCycle();
			void postMergeSwitchCycle();			

			void mergeSource();
			void postMergeSource();			

			void mergeFeed();
			void postMergeFeed();			

			void mergeSpectralWindow();
			void postMergeSpectralWindow();			

			void mergeFreqOffset();
			void postMergeFreqOffset();			

			void mergePolarization();
			void postMergePolarization();			

			void mergeReceiver();
			void postMergeReceiver();			

			void mergeBeam();
			void postMergeBeam();			

			void mergeDoppler();
			void postMergeDoppler();			

			void mergeProcessor();
			void postMergeProcessor();			

			void mergeCorrelatorMode();
			void postMergeCorrelatorMode();			

			void mergeCalDevice();
			void postMergeCalDevice();			

			void mergeFlagCmd();
			void postMergeFlagCmd();			

			void mergeFocus();
			void postMergeFocus();			

			void mergeHistory();
			void postMergeHistory();			

			void mergeObservation();
			void postMergeObservation();			

			void mergePointing();
			void postMergePointing();			

			void mergeSeeing();
			void postMergeSeeing();			

			void mergeSysCal();
			void postMergeSysCal();			

			void mergeTotalPower();
			void postMergeTotalPower();			

			void mergeWeather();
			void postMergeWeather();			

			void mergeWVMCal();
			void postMergeWVMCal();			

			void mergeEphemeris();
			void postMergeEphemeris();			

			void mergeExecBlock();
			void postMergeExecBlock();			

			void mergeScan();
			void postMergeScan();			

			void mergeSubscan();
			void postMergeSubscan();			

			void mergeMain();
			void postMergeMain();			

			void mergeFocusModel();
			void postMergeFocusModel();			

			void mergeGainTracking();
			void postMergeGainTracking();			

			void mergePointingModel();
			void postMergePointingModel();			

			void mergeCalAmpli();
			void postMergeCalAmpli();			

			void mergeCalData();
			void postMergeCalData();			

			void mergeCalReduction();
			void postMergeCalReduction();			

			void mergeCalPhase();
			void postMergeCalPhase();			

			void mergeCalSeeing();
			void postMergeCalSeeing();			

			void mergeCalPosition();
			void postMergeCalPosition();			

			void mergeCalPointing();
			void postMergeCalPointing();			

			void mergeCalPointingModel();
			void postMergeCalPointingModel();			

			void mergeCalHolography();
			void postMergeCalHolography();			

			void mergeCalAtmosphere();
			void postMergeCalAtmosphere();			

			void mergeCalCurve();
			void postMergeCalCurve();			

			void mergeStation();
			void postMergeStation();			

			void mergeAlmaRadiometer();
			void postMergeAlmaRadiometer();			

			void mergeSquareLawDetector();
			void postMergeSquareLawDetector();			

			void mergeCalFocus();
			void postMergeCalFocus();			

			void mergeCalDelay();
			void postMergeCalDelay();			

			void mergeHolography();
			void postMergeHolography();			

			void mergeCalBandpass();
			void postMergeCalBandpass();			

			void mergeCalFlux();
			void postMergeCalFlux();			

			void mergeCalFocusModel();
			void postMergeCalFocusModel();			

			void mergeCalGain();
			void postMergeCalGain();			

			void mergeCalPrimaryBeam();
			void postMergeCalPrimaryBeam();			

			void mergeCalWVR();
			void postMergeCalWVR();			

			void mergeAnnotation();
			void postMergeAnnotation();			

			void mergeDelayModel();
			void postMergeDelayModel();			



		void (Merger::*mergeSBSummaryPtr) () ;

		void (Merger::*mergeConfigDescriptionPtr) () ;

		void (Merger::*mergeFieldPtr) () ;

		void (Merger::*mergeStatePtr) () ;

		void (Merger::*mergeAntennaPtr) () ;

		void (Merger::*mergeDataDescriptionPtr) () ;

		void (Merger::*mergeSwitchCyclePtr) () ;

		void (Merger::*mergeSourcePtr) () ;

		void (Merger::*mergeFeedPtr) () ;

		void (Merger::*mergeSpectralWindowPtr) () ;

		void (Merger::*mergeFreqOffsetPtr) () ;

		void (Merger::*mergePolarizationPtr) () ;

		void (Merger::*mergeReceiverPtr) () ;

		void (Merger::*mergeBeamPtr) () ;

		void (Merger::*mergeDopplerPtr) () ;

		void (Merger::*mergeProcessorPtr) () ;

		void (Merger::*mergeCorrelatorModePtr) () ;

		void (Merger::*mergeCalDevicePtr) () ;

		void (Merger::*mergeFlagCmdPtr) () ;

		void (Merger::*mergeFocusPtr) () ;

		void (Merger::*mergeHistoryPtr) () ;

		void (Merger::*mergeObservationPtr) () ;

		void (Merger::*mergePointingPtr) () ;

		void (Merger::*mergeSeeingPtr) () ;

		void (Merger::*mergeSysCalPtr) () ;

		void (Merger::*mergeTotalPowerPtr) () ;

		void (Merger::*mergeWeatherPtr) () ;

		void (Merger::*mergeWVMCalPtr) () ;

		void (Merger::*mergeEphemerisPtr) () ;

		void (Merger::*mergeExecBlockPtr) () ;

		void (Merger::*mergeScanPtr) () ;

		void (Merger::*mergeSubscanPtr) () ;

		void (Merger::*mergeMainPtr) () ;

		void (Merger::*mergeFocusModelPtr) () ;

		void (Merger::*mergeGainTrackingPtr) () ;

		void (Merger::*mergePointingModelPtr) () ;

		void (Merger::*mergeCalAmpliPtr) () ;

		void (Merger::*mergeCalDataPtr) () ;

		void (Merger::*mergeCalReductionPtr) () ;

		void (Merger::*mergeCalPhasePtr) () ;

		void (Merger::*mergeCalSeeingPtr) () ;

		void (Merger::*mergeCalPositionPtr) () ;

		void (Merger::*mergeCalPointingPtr) () ;

		void (Merger::*mergeCalPointingModelPtr) () ;

		void (Merger::*mergeCalHolographyPtr) () ;

		void (Merger::*mergeCalAtmospherePtr) () ;

		void (Merger::*mergeCalCurvePtr) () ;

		void (Merger::*mergeStationPtr) () ;

		void (Merger::*mergeAlmaRadiometerPtr) () ;

		void (Merger::*mergeSquareLawDetectorPtr) () ;

		void (Merger::*mergeCalFocusPtr) () ;

		void (Merger::*mergeCalDelayPtr) () ;

		void (Merger::*mergeHolographyPtr) () ;

		void (Merger::*mergeCalBandpassPtr) () ;

		void (Merger::*mergeCalFluxPtr) () ;

		void (Merger::*mergeCalFocusModelPtr) () ;

		void (Merger::*mergeCalGainPtr) () ;

		void (Merger::*mergeCalPrimaryBeamPtr) () ;

		void (Merger::*mergeCalWVRPtr) () ;

		void (Merger::*mergeAnnotationPtr) () ;

		void (Merger::*mergeDelayModelPtr) () ;

	};
} // End namespace asdm

#endif  /* Merger_CLASS */
