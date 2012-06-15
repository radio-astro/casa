#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#add_definitions( -DWITHOUT_ACS -pg )
#set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg" )
#set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg" )
#set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -pg" )

add_definitions( -DWITHOUT_ACS )
if( APPLE AND
    NOT CMAKE_CXX_FLAGS MATCHES x86_64 AND
    NOT _gcc_machine MATCHES x86_64 AND
    NOT _gcc_machine MATCHES 11.0 )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i586" )
endif()

casa_add_library( alma_v3
  ASDM/AlmaRadiometerRow.cc
  ASDM/AlmaRadiometerTable.cc
  ASDM/Angle.cc
  ASDM/AngularRate.cc
  ASDM/AnnotationRow.cc
  ASDM/AnnotationTable.cc
  ASDM/AntennaRow.cc
  ASDM/AntennaTable.cc
  ASDM/ArrayTime.cc
  ASDM/ArrayTimeInterval.cc
  ASDM/ASDM.cc
  ASDM/Base64.cc
  ASDMBinaries/BaselineMetadata.cc
  ASDMBinaries/BaselinesSet.cc
  ASDMBinaries/DataDescriptionsSet.cc
  ASDMBinaries/Error.cc
  ASDMBinaries/Integration.cc
  ASDMBinaries/SDMBinData.cc
  ASDMBinaries/SDMDataObject.cc
  ASDMBinaries/SDMDataObjectParser.cc
  ASDMBinaries/SDMDataObjectReader.cc
  ASDMBinaries/SDMDataObjectStreamReader.cc
  ASDMBinaries/SDMDataObjectWriter.cc
  ASDMBinaries/SwitchCyclesList.cc
  ASDM/BooleanWrapper.cc
  ASDM/ByteWrapper.cc
  ASDM/CalAmpliRow.cc
  ASDM/CalAmpliTable.cc
  ASDM/CalAtmosphereRow.cc
  ASDM/CalAtmosphereTable.cc
  ASDM/CalBandpassRow.cc
  ASDM/CalBandpassTable.cc
  ASDM/CalCurveRow.cc
  ASDM/CalCurveTable.cc
  ASDM/CalDataRow.cc
  ASDM/CalDataTable.cc
  ASDM/CalDelayRow.cc
  ASDM/CalDelayTable.cc
  ASDM/CalDeviceRow.cc
  ASDM/CalDeviceTable.cc
  ASDM/CalFluxRow.cc
  ASDM/CalFluxTable.cc
  ASDM/CalFocusModelRow.cc
  ASDM/CalFocusModelTable.cc
  ASDM/CalFocusRow.cc
  ASDM/CalFocusTable.cc
  ASDM/CalGainRow.cc
  ASDM/CalGainTable.cc
  ASDM/CalHolographyRow.cc
  ASDM/CalHolographyTable.cc
  ASDM/CalPhaseRow.cc
  ASDM/CalPhaseTable.cc
  ASDM/CalPointingModelRow.cc
  ASDM/CalPointingModelTable.cc
  ASDM/CalPointingRow.cc
  ASDM/CalPointingTable.cc
  ASDM/CalPositionRow.cc
  ASDM/CalPositionTable.cc
  ASDM/CalPrimaryBeamRow.cc
  ASDM/CalPrimaryBeamTable.cc
  ASDM/CalReductionRow.cc
  ASDM/CalReductionTable.cc
  ASDM/CalSeeingRow.cc
  ASDM/CalSeeingTable.cc
  ASDM/CalWVRRow.cc
  ASDM/CalWVRTable.cc
  ASDM/CharacterWrapper.cc
  ASDM/ComplexWrapper.cc
  ASDM/ConfigDescriptionRow.cc
  ASDM/ConfigDescriptionTable.cc
  ASDM/ConversionException.cc
  ASDM/CorrelatorModeRow.cc
  ASDM/CorrelatorModeTable.cc
  ASDM/DataDescriptionRow.cc
  ASDM/DataDescriptionTable.cc
  ASDM/DelayModelRow.cc
  ASDM/DelayModelTable.cc
  ASDM/DopplerRow.cc
  ASDM/DopplerTable.cc
  ASDM/DoubleWrapper.cc
  ASDM/DuplicateKey.cc
  ASDM/EndianStream.cc
  ASDM/Entity.cc
  ASDM/EntityId.cc
  ASDM/EntityRef.cc
  ASDM/EnumerationParser.cc
  ASDM/EphemerisRow.cc
  ASDM/EphemerisTable.cc
  ASDM/ExecBlockRow.cc
  ASDM/ExecBlockTable.cc
  ASDM/FeedRow.cc
  ASDM/FeedTable.cc
  ASDM/FieldRow.cc
  ASDM/FieldTable.cc
  ASDM/FlagRow.cc
  ASDM/FlagTable.cc
  ASDM/FlagCmdRow.cc
  ASDM/FlagCmdTable.cc
  ASDM/FloatWrapper.cc
  ASDM/Flux.cc
  ASDM/FocusModelRow.cc
  ASDM/FocusModelTable.cc
  ASDM/FocusRow.cc
  ASDM/FocusTable.cc
  ASDM/FreqOffsetRow.cc
  ASDM/FreqOffsetTable.cc
  ASDM/Frequency.cc
  ASDM/GainTrackingRow.cc
  ASDM/GainTrackingTable.cc
  ASDM/HistoryRow.cc
  ASDM/HistoryTable.cc
  ASDM/HolographyRow.cc
  ASDM/HolographyTable.cc
  ASDM/Humidity.cc
  ASDM/IllegalAccessException.cc
  ASDM/IntegerWrapper.cc
  ASDM/Interval.cc
  ASDM/InvalidArgumentException.cc
  ASDM/InvalidDataException.cc
  ASDM/Length.cc
  ASDM/LongWrapper.cc
  ASDM/MainRow.cc
  ASDM/MainTable.cc
  ASDM/Merger.cc
  ASDM/Misc.cc
  ASDM/NoSuchRow.cc
  ASDM/NumberFormatException.cc
  ASDM/ObservationRow.cc
  ASDM/ObservationTable.cc
  ASDM/OutOfBoundsException.cc
  ASDM/Parser.cc
  ASDM/PartId.cc
  ASDM/PointingModelRow.cc
  ASDM/PointingModelTable.cc
  ASDM/PointingRow.cc
  ASDM/PointingTable.cc
  ASDM/PolarizationRow.cc
  ASDM/PolarizationTable.cc
  ASDM/Pressure.cc
  ASDM/ProcessorRow.cc
  ASDM/ProcessorTable.cc
  ASDM/ReceiverRow.cc
  ASDM/ReceiverTable.cc
  ASDM/SBSummaryRow.cc
  ASDM/SBSummaryTable.cc
  ASDM/ScaleTable.cc
  ASDM/ScaleTable.h
  ASDM/ScaleRow.cc
  ASDM/ScaleRow.h
  ASDM/ScanRow.cc
  ASDM/ScanTable.cc
  ASDM/SeeingRow.cc
  ASDM/SeeingTable.cc
  ASDM/ShortWrapper.cc
  ASDM/SourceRow.cc
  ASDM/SourceTable.cc
  ASDM/SpectralWindowRow.cc
  ASDM/SpectralWindowTable.cc
  ASDM/Speed.cc
  ASDM/SquareLawDetectorRow.cc
  ASDM/SquareLawDetectorTable.cc
  ASDM/StateRow.cc
  ASDM/StateTable.cc
  ASDM/StationRow.cc
  ASDM/StationTable.cc
  ASDM/StringTokenizer.cc
  ASDM/SubscanRow.cc
  ASDM/SubscanTable.cc
  ASDM/SwitchCycleRow.cc
  ASDM/SwitchCycleTable.cc
  ASDM/SysCalRow.cc
  ASDM/SysCalTable.cc
  ASDM/SysPowerRow.cc
  ASDM/SysPowerTable.cc
  ASDM/Tag.cc
  ASDM/TagFormatException.cc
  ASDM/TagType.cc
  ASDM/Temperature.cc
  ASDM/TotalPowerRow.cc
  ASDM/TotalPowerTable.cc
  ASDM/UniquenessViolationException.cc
  ASDM/WeatherRow.cc
  ASDM/WeatherTable.cc
  ASDM/WVMCalRow.cc
  ASDM/WVMCalTable.cc
  Enumerations/CACAPolarization.cc
  Enumerations/CAccumMode.cc
  Enumerations/CAntennaMake.cc
  Enumerations/CAntennaMotionPattern.cc
  Enumerations/CAntennaType.cc
  Enumerations/CAssociatedCalNature.cc
  Enumerations/CAssociatedFieldNature.cc
  Enumerations/CAtmPhaseCorrection.cc
  Enumerations/CAxisName.cc
  Enumerations/CBasebandName.cc
  Enumerations/CBaselineReferenceCode.cc
  Enumerations/CCalCurveType.cc
  Enumerations/CCalDataOrigin.cc
  Enumerations/CCalibrationDevice.cc
  Enumerations/CCalibrationFunction.cc
  Enumerations/CCalibrationMode.cc
  Enumerations/CCalibrationSet.cc
  Enumerations/CCalType.cc
  Enumerations/CCorrelationBit.cc
  Enumerations/CCorrelationMode.cc
  Enumerations/CCorrelatorCalibration.cc
  Enumerations/CCorrelatorName.cc
  Enumerations/CCorrelatorType.cc
  Enumerations/CDataContent.cc
  Enumerations/CDataScale.cc
  Enumerations/CDetectorBandType.cc
  Enumerations/CDirectionReferenceCode.cc
  Enumerations/CDopplerReferenceCode.cc
  Enumerations/CDopplerTrackingMode.cc
  Enumerations/CFieldCode.cc
  Enumerations/CFilterMode.cc
  Enumerations/CFluxCalibrationMethod.cc
  Enumerations/CFocusMethod.cc
  Enumerations/CFrequencyReferenceCode.cc
  Enumerations/CHolographyChannelType.cc
  Enumerations/CInvalidatingCondition.cc
  Enumerations/CNetSideband.cc
  Enumerations/CPointingMethod.cc
  Enumerations/CPointingModelMode.cc
  Enumerations/CPolarizationType.cc
  Enumerations/CPositionMethod.cc
  Enumerations/CPositionReferenceCode.cc
  Enumerations/CPrimaryBeamDescription.cc
  Enumerations/CPrimitiveDataType.cc
  Enumerations/CProcessorSubType.cc
  Enumerations/CProcessorType.cc
  Enumerations/CRadialVelocityReferenceCode.cc
  Enumerations/CReceiverBand.cc
  Enumerations/CReceiverSideband.cc
  Enumerations/CSBType.cc
  Enumerations/CScanIntent.cc
  Enumerations/CSchedulerMode.cc
  Enumerations/CSidebandProcessingMode.cc
  Enumerations/CSourceModel.cc
  Enumerations/CSpectralResolutionType.cc
  Enumerations/CStationType.cc
  Enumerations/CStokesParameter.cc
  Enumerations/CSubscanIntent.cc
  Enumerations/CSwitchingMode.cc
  Enumerations/CSyscalMethod.cc
  Enumerations/CTimeSampling.cc
  Enumerations/CTimeScale.cc
  Enumerations/CWeightType.cc
  Enumerations/CWindowFunction.cc
  Enumerations/CWVRMethod.cc
  Enumtcl/ACAPolarization.cc
  Enumtcl/AccumMode.cc
  Enumtcl/AntennaMake.cc
  Enumtcl/AntennaMotionPattern.cc
  Enumtcl/AntennaType.cc
  Enumtcl/AssociatedCalNature.cc
  Enumtcl/AssociatedFieldNature.cc
  Enumtcl/AtmPhaseCorrection.cc
  Enumtcl/AxisName.cc
  Enumtcl/BasebandName.cc
  Enumtcl/BaselineReferenceCode.cc
  Enumtcl/CalCurveType.cc
  Enumtcl/CalDataOrigin.cc
  Enumtcl/CalibrationDevice.cc
  Enumtcl/CalibrationFunction.cc
  Enumtcl/CalibrationMode.cc
  Enumtcl/CalibrationSet.cc
  Enumtcl/CalType.cc
  Enumtcl/CorrelationBit.cc
  Enumtcl/CorrelationMode.cc
  Enumtcl/CorrelatorCalibration.cc
  Enumtcl/CorrelatorName.cc
  Enumtcl/CorrelatorType.cc
  Enumtcl/DataContent.cc
  Enumtcl/DataScale.cc
  Enumtcl/DetectorBandType.cc
  Enumtcl/DirectionReferenceCode.cc
  Enumtcl/DopplerReferenceCode.cc
  Enumtcl/DopplerTrackingMode.cc
  Enumtcl/FieldCode.cc
  Enumtcl/FilterMode.cc
  Enumtcl/FluxCalibrationMethod.cc
  Enumtcl/FocusMethod.cc
  Enumtcl/FrequencyReferenceCode.cc
  Enumtcl/HolographyChannelType.cc
  Enumtcl/InvalidatingCondition.cc
  Enumtcl/NetSideband.cc
  Enumtcl/PointingMethod.cc
  Enumtcl/PointingModelMode.cc
  Enumtcl/PolarizationType.cc
  Enumtcl/PositionMethod.cc
  Enumtcl/PositionReferenceCode.cc
  Enumtcl/PrimaryBeamDescription.cc
  Enumtcl/PrimitiveDataType.cc
  Enumtcl/ProcessorSubType.cc
  Enumtcl/ProcessorType.cc
  Enumtcl/ReceiverBand.cc
  Enumtcl/ReceiverSideband.cc
  Enumtcl/SBType.cc
  Enumtcl/ScanIntent.cc
  Enumtcl/SchedulerMode.cc
  Enumtcl/SidebandProcessingMode.cc
  Enumtcl/SourceModel.cc
  Enumtcl/SpectralResolutionType.cc
  Enumtcl/StationType.cc
  Enumtcl/StokesParameter.cc
  Enumtcl/SubscanIntent.cc
  Enumtcl/SwitchingMode.cc
  Enumtcl/SyscalMethod.cc
  Enumtcl/TimeSampling.cc
  Enumtcl/TimeScale.cc
  Enumtcl/WindowFunction.cc
  Enumtcl/WeightType.cc
  Enumtcl/WVRMethod.cc
  MS2ASDM/MS2ASDM.cc
  )

casa_add_executable( alma_v3 asdm2MS_v3 
  apps/asdm2MS_v3/asdm2MS_v3.cc
  apps/asdm2MS_v3/ASDM2MSFiller.cc
  apps/asdm2MS_v3/ASDMTableBase.cc
  apps/asdm2MS_v3/ASDMTables.cc
  apps/asdm2MS_v3/ASDMVerbatimFiller.cc
  apps/asdm2MS_v3/Name2Table.cc
  apps/asdm2MS_v3/UvwCoords.cc
  )

casa_add_executable( alma_v3 asdm2MS_xl 
  apps/asdm2MS_xl/asdm2MS_xl.cc
  apps/asdm2MS_xl/ASDM2MSFiller.cc
  apps/asdm2MS_xl/ASDMTableBase.cc
  apps/asdm2MS_xl/ASDMTables.cc
  apps/asdm2MS_xl/ASDMVerbatimFiller.cc
  apps/asdm2MS_xl/Name2Table.cc
  apps/asdm2MS_xl/UvwCoords.cc
  )

casa_add_executable( alma_v3 MS2asdm_v3
  apps/MS2asdm_v3/MS2asdm_v3.cc 
  )

casa_add_executable( alma_v3 asdmSummary
  apps/asdmSummary/asdmSummary.cc 
  )

casa_add_assay(alma_v3 ASDMBinaries/test/tReadBigBDF.cc)
casa_add_assay(alma_v3 ASDMBinaries/test/tReadAllBDFs.cc)
casa_add_assay(alma_v3 ASDM/test/tTableStreamReader.cc)
#casa_add_assay(alma_v3 ASDM/test/tThreadedTableStreamReader.cc)
