
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
 * /////////////////////////////////////////////////////////////////
 * // WARNING!  DO NOT MODIFY THIS FILE!                          //
 * //  ---------------------------------------------------------  //
 * // | This is generated code!  Do not modify this file.       | //
 * // | Any changes will be lost when the file is re-generated. | //
 * //  ---------------------------------------------------------  //
 * /////////////////////////////////////////////////////////////////
 *
 * File CEnumerationParser.h
 */
 #include <string>
 #include <ConversionException.h>
 
 using namespace std;
 
 
 
 #include "CSBType.h"
 using namespace SBTypeMod;
 
 #include "CCorrelationMode.h"
 using namespace CorrelationModeMod;
 
 #include "CAtmPhaseCorrection.h"
 using namespace AtmPhaseCorrectionMod;
 
 #include "CSpectralResolutionType.h"
 using namespace SpectralResolutionTypeMod;
 
 #include "CCalibrationDevice.h"
 using namespace CalibrationDeviceMod;
 
 #include "CAntennaMake.h"
 using namespace AntennaMakeMod;
 
 #include "CAntennaType.h"
 using namespace AntennaTypeMod;
 
 #include "CSourceModel.h"
 using namespace SourceModelMod;
 
 #include "CPolarizationType.h"
 using namespace PolarizationTypeMod;
 
 #include "CStokesParameter.h"
 using namespace StokesParameterMod;
 
 #include "CNetSideband.h"
 using namespace NetSidebandMod;
 
 #include "CSidebandProcessingMode.h"
 using namespace SidebandProcessingModeMod;
 
 #include "CBasebandName.h"
 using namespace BasebandNameMod;
 
 #include "CWindowFunction.h"
 using namespace WindowFunctionMod;
 
 #include "CCorrelationBit.h"
 using namespace CorrelationBitMod;
 
 #include "CReceiverBand.h"
 using namespace ReceiverBandMod;
 
 #include "CReceiverSideband.h"
 using namespace ReceiverSidebandMod;
 
 #include "CProcessorType.h"
 using namespace ProcessorTypeMod;
 
 #include "CAccumMode.h"
 using namespace AccumModeMod;
 
 #include "CAxisName.h"
 using namespace AxisNameMod;
 
 #include "CFilterMode.h"
 using namespace FilterModeMod;
 
 #include "CCorrelatorName.h"
 using namespace CorrelatorNameMod;
 
 #include "CScanIntent.h"
 using namespace ScanIntentMod;
 
 #include "CSubscanIntent.h"
 using namespace SubscanIntentMod;
 
 #include "CSwitchingMode.h"
 using namespace SwitchingModeMod;
 
 #include "CCorrelatorCalibration.h"
 using namespace CorrelatorCalibrationMod;
 
 #include "CTimeSampling.h"
 using namespace TimeSamplingMod;
 
 #include "CCalType.h"
 using namespace CalTypeMod;
 
 #include "CAssociatedCalNature.h"
 using namespace AssociatedCalNatureMod;
 
 #include "CCalDataOrigin.h"
 using namespace CalDataOriginMod;
 
 #include "CInvalidatingCondition.h"
 using namespace InvalidatingConditionMod;
 
 #include "CPositionMethod.h"
 using namespace PositionMethodMod;
 
 #include "CPointingMethod.h"
 using namespace PointingMethodMod;
 
 #include "CPointingModelMode.h"
 using namespace PointingModelModeMod;
 
 #include "CSyscalMethod.h"
 using namespace SyscalMethodMod;
 
 #include "CCalCurveType.h"
 using namespace CalCurveTypeMod;
 
 #include "CStationType.h"
 using namespace StationTypeMod;
 
 #include "CDetectorBandType.h"
 using namespace DetectorBandTypeMod;
 
 #include "CFocusMethod.h"
 using namespace FocusMethodMod;
 
 #include "CHolographyChannelType.h"
 using namespace HolographyChannelTypeMod;
 
 #include "CFluxCalibrationMethod.h"
 using namespace FluxCalibrationMethodMod;
 
 #include "CWVRMethod.h"
 using namespace WVRMethodMod;
 
 #include "CCalibrationMode.h"
 using namespace CalibrationModeMod;
 
 #include "CAssociatedFieldNature.h"
 using namespace AssociatedFieldNatureMod;
 
 #include "CDataContent.h"
 using namespace DataContentMod;
 
 #include "CPrimitiveDataType.h"
 using namespace PrimitiveDataTypeMod;
 
 #include "CSchedulerMode.h"
 using namespace SchedulerModeMod;
 
 #include "CFieldCode.h"
 using namespace FieldCodeMod;
 
 #include "CProcessorSubType.h"
 using namespace ProcessorSubTypeMod;
 
 #include "CACAPolarization.h"
 using namespace ACAPolarizationMod;
 
 
 namespace asdm {
 
 	class EnumerationParser {
 	
 	static string getField(const string &xml, const string &field);
 	
 	static string substring(const string &s, int a, int b);
 	
 	static string trim(const string &s);
	
	public:
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  SBTypeMod::SBType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, SBTypeMod::SBType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<SBTypeMod::SBType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<SBTypeMod::SBType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<SBTypeMod::SBType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<SBTypeMod::SBType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<SBTypeMod::SBType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<SBTypeMod::SBType> > >& vvv_e); 

	/**
	 * Returns a SBTypeMod::SBType from a string.
	 * @param xml the string to be converted into a SBTypeMod::SBType
	 * @return a SBTypeMod::SBType.
	 */
	static SBTypeMod::SBType getSBType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<SBTypeMod::SBType> from a string.
	 * @param xml the string to be converted into a vector<SBTypeMod::SBType>
	 * @return a vector<SBTypeMod::SBType>.
	 */
	static vector<SBTypeMod::SBType> getSBType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<SBTypeMod::SBType> > from a string.
	 * @param xml the string to be converted into a vector<vector<SBTypeMod::SBType> >
	 * @return a vector<vector<SBTypeMod::SBType> >.
	 */
	static vector<vector<SBTypeMod::SBType> > getSBType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<SBTypeMod::SBType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<SBTypeMod::SBType> > >
	 * @return a vector<vector<vector<SBTypeMod::SBType> > >.
	 */
	static vector<vector<vector<SBTypeMod::SBType> > > getSBType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CorrelationModeMod::CorrelationMode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CorrelationModeMod::CorrelationMode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CorrelationModeMod::CorrelationMode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CorrelationModeMod::CorrelationMode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CorrelationModeMod::CorrelationMode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CorrelationModeMod::CorrelationMode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CorrelationModeMod::CorrelationMode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CorrelationModeMod::CorrelationMode> > >& vvv_e); 

	/**
	 * Returns a CorrelationModeMod::CorrelationMode from a string.
	 * @param xml the string to be converted into a CorrelationModeMod::CorrelationMode
	 * @return a CorrelationModeMod::CorrelationMode.
	 */
	static CorrelationModeMod::CorrelationMode getCorrelationMode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CorrelationModeMod::CorrelationMode> from a string.
	 * @param xml the string to be converted into a vector<CorrelationModeMod::CorrelationMode>
	 * @return a vector<CorrelationModeMod::CorrelationMode>.
	 */
	static vector<CorrelationModeMod::CorrelationMode> getCorrelationMode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CorrelationModeMod::CorrelationMode> > from a string.
	 * @param xml the string to be converted into a vector<vector<CorrelationModeMod::CorrelationMode> >
	 * @return a vector<vector<CorrelationModeMod::CorrelationMode> >.
	 */
	static vector<vector<CorrelationModeMod::CorrelationMode> > getCorrelationMode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CorrelationModeMod::CorrelationMode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CorrelationModeMod::CorrelationMode> > >
	 * @return a vector<vector<vector<CorrelationModeMod::CorrelationMode> > >.
	 */
	static vector<vector<vector<CorrelationModeMod::CorrelationMode> > > getCorrelationMode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  AtmPhaseCorrectionMod::AtmPhaseCorrection value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, AtmPhaseCorrectionMod::AtmPhaseCorrection e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<AtmPhaseCorrectionMod::AtmPhaseCorrection>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<AtmPhaseCorrectionMod::AtmPhaseCorrection>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> > >& vvv_e); 

	/**
	 * Returns a AtmPhaseCorrectionMod::AtmPhaseCorrection from a string.
	 * @param xml the string to be converted into a AtmPhaseCorrectionMod::AtmPhaseCorrection
	 * @return a AtmPhaseCorrectionMod::AtmPhaseCorrection.
	 */
	static AtmPhaseCorrectionMod::AtmPhaseCorrection getAtmPhaseCorrection(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> from a string.
	 * @param xml the string to be converted into a vector<AtmPhaseCorrectionMod::AtmPhaseCorrection>
	 * @return a vector<AtmPhaseCorrectionMod::AtmPhaseCorrection>.
	 */
	static vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> getAtmPhaseCorrection1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> > from a string.
	 * @param xml the string to be converted into a vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> >
	 * @return a vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> >.
	 */
	static vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> > getAtmPhaseCorrection2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> > >
	 * @return a vector<vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> > >.
	 */
	static vector<vector<vector<AtmPhaseCorrectionMod::AtmPhaseCorrection> > > getAtmPhaseCorrection3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  SpectralResolutionTypeMod::SpectralResolutionType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, SpectralResolutionTypeMod::SpectralResolutionType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<SpectralResolutionTypeMod::SpectralResolutionType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<SpectralResolutionTypeMod::SpectralResolutionType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> > >& vvv_e); 

	/**
	 * Returns a SpectralResolutionTypeMod::SpectralResolutionType from a string.
	 * @param xml the string to be converted into a SpectralResolutionTypeMod::SpectralResolutionType
	 * @return a SpectralResolutionTypeMod::SpectralResolutionType.
	 */
	static SpectralResolutionTypeMod::SpectralResolutionType getSpectralResolutionType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<SpectralResolutionTypeMod::SpectralResolutionType> from a string.
	 * @param xml the string to be converted into a vector<SpectralResolutionTypeMod::SpectralResolutionType>
	 * @return a vector<SpectralResolutionTypeMod::SpectralResolutionType>.
	 */
	static vector<SpectralResolutionTypeMod::SpectralResolutionType> getSpectralResolutionType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> > from a string.
	 * @param xml the string to be converted into a vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> >
	 * @return a vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> >.
	 */
	static vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> > getSpectralResolutionType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> > >
	 * @return a vector<vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> > >.
	 */
	static vector<vector<vector<SpectralResolutionTypeMod::SpectralResolutionType> > > getSpectralResolutionType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CalibrationDeviceMod::CalibrationDevice value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CalibrationDeviceMod::CalibrationDevice e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CalibrationDeviceMod::CalibrationDevice>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CalibrationDeviceMod::CalibrationDevice>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CalibrationDeviceMod::CalibrationDevice> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CalibrationDeviceMod::CalibrationDevice> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CalibrationDeviceMod::CalibrationDevice> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CalibrationDeviceMod::CalibrationDevice> > >& vvv_e); 

	/**
	 * Returns a CalibrationDeviceMod::CalibrationDevice from a string.
	 * @param xml the string to be converted into a CalibrationDeviceMod::CalibrationDevice
	 * @return a CalibrationDeviceMod::CalibrationDevice.
	 */
	static CalibrationDeviceMod::CalibrationDevice getCalibrationDevice(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CalibrationDeviceMod::CalibrationDevice> from a string.
	 * @param xml the string to be converted into a vector<CalibrationDeviceMod::CalibrationDevice>
	 * @return a vector<CalibrationDeviceMod::CalibrationDevice>.
	 */
	static vector<CalibrationDeviceMod::CalibrationDevice> getCalibrationDevice1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CalibrationDeviceMod::CalibrationDevice> > from a string.
	 * @param xml the string to be converted into a vector<vector<CalibrationDeviceMod::CalibrationDevice> >
	 * @return a vector<vector<CalibrationDeviceMod::CalibrationDevice> >.
	 */
	static vector<vector<CalibrationDeviceMod::CalibrationDevice> > getCalibrationDevice2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CalibrationDeviceMod::CalibrationDevice> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CalibrationDeviceMod::CalibrationDevice> > >
	 * @return a vector<vector<vector<CalibrationDeviceMod::CalibrationDevice> > >.
	 */
	static vector<vector<vector<CalibrationDeviceMod::CalibrationDevice> > > getCalibrationDevice3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  AntennaMakeMod::AntennaMake value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, AntennaMakeMod::AntennaMake e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<AntennaMakeMod::AntennaMake>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<AntennaMakeMod::AntennaMake>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<AntennaMakeMod::AntennaMake> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<AntennaMakeMod::AntennaMake> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<AntennaMakeMod::AntennaMake> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<AntennaMakeMod::AntennaMake> > >& vvv_e); 

	/**
	 * Returns a AntennaMakeMod::AntennaMake from a string.
	 * @param xml the string to be converted into a AntennaMakeMod::AntennaMake
	 * @return a AntennaMakeMod::AntennaMake.
	 */
	static AntennaMakeMod::AntennaMake getAntennaMake(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<AntennaMakeMod::AntennaMake> from a string.
	 * @param xml the string to be converted into a vector<AntennaMakeMod::AntennaMake>
	 * @return a vector<AntennaMakeMod::AntennaMake>.
	 */
	static vector<AntennaMakeMod::AntennaMake> getAntennaMake1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<AntennaMakeMod::AntennaMake> > from a string.
	 * @param xml the string to be converted into a vector<vector<AntennaMakeMod::AntennaMake> >
	 * @return a vector<vector<AntennaMakeMod::AntennaMake> >.
	 */
	static vector<vector<AntennaMakeMod::AntennaMake> > getAntennaMake2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<AntennaMakeMod::AntennaMake> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<AntennaMakeMod::AntennaMake> > >
	 * @return a vector<vector<vector<AntennaMakeMod::AntennaMake> > >.
	 */
	static vector<vector<vector<AntennaMakeMod::AntennaMake> > > getAntennaMake3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  AntennaTypeMod::AntennaType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, AntennaTypeMod::AntennaType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<AntennaTypeMod::AntennaType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<AntennaTypeMod::AntennaType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<AntennaTypeMod::AntennaType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<AntennaTypeMod::AntennaType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<AntennaTypeMod::AntennaType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<AntennaTypeMod::AntennaType> > >& vvv_e); 

	/**
	 * Returns a AntennaTypeMod::AntennaType from a string.
	 * @param xml the string to be converted into a AntennaTypeMod::AntennaType
	 * @return a AntennaTypeMod::AntennaType.
	 */
	static AntennaTypeMod::AntennaType getAntennaType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<AntennaTypeMod::AntennaType> from a string.
	 * @param xml the string to be converted into a vector<AntennaTypeMod::AntennaType>
	 * @return a vector<AntennaTypeMod::AntennaType>.
	 */
	static vector<AntennaTypeMod::AntennaType> getAntennaType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<AntennaTypeMod::AntennaType> > from a string.
	 * @param xml the string to be converted into a vector<vector<AntennaTypeMod::AntennaType> >
	 * @return a vector<vector<AntennaTypeMod::AntennaType> >.
	 */
	static vector<vector<AntennaTypeMod::AntennaType> > getAntennaType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<AntennaTypeMod::AntennaType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<AntennaTypeMod::AntennaType> > >
	 * @return a vector<vector<vector<AntennaTypeMod::AntennaType> > >.
	 */
	static vector<vector<vector<AntennaTypeMod::AntennaType> > > getAntennaType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  SourceModelMod::SourceModel value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, SourceModelMod::SourceModel e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<SourceModelMod::SourceModel>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<SourceModelMod::SourceModel>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<SourceModelMod::SourceModel> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<SourceModelMod::SourceModel> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<SourceModelMod::SourceModel> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<SourceModelMod::SourceModel> > >& vvv_e); 

	/**
	 * Returns a SourceModelMod::SourceModel from a string.
	 * @param xml the string to be converted into a SourceModelMod::SourceModel
	 * @return a SourceModelMod::SourceModel.
	 */
	static SourceModelMod::SourceModel getSourceModel(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<SourceModelMod::SourceModel> from a string.
	 * @param xml the string to be converted into a vector<SourceModelMod::SourceModel>
	 * @return a vector<SourceModelMod::SourceModel>.
	 */
	static vector<SourceModelMod::SourceModel> getSourceModel1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<SourceModelMod::SourceModel> > from a string.
	 * @param xml the string to be converted into a vector<vector<SourceModelMod::SourceModel> >
	 * @return a vector<vector<SourceModelMod::SourceModel> >.
	 */
	static vector<vector<SourceModelMod::SourceModel> > getSourceModel2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<SourceModelMod::SourceModel> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<SourceModelMod::SourceModel> > >
	 * @return a vector<vector<vector<SourceModelMod::SourceModel> > >.
	 */
	static vector<vector<vector<SourceModelMod::SourceModel> > > getSourceModel3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  PolarizationTypeMod::PolarizationType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, PolarizationTypeMod::PolarizationType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<PolarizationTypeMod::PolarizationType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<PolarizationTypeMod::PolarizationType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<PolarizationTypeMod::PolarizationType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<PolarizationTypeMod::PolarizationType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<PolarizationTypeMod::PolarizationType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<PolarizationTypeMod::PolarizationType> > >& vvv_e); 

	/**
	 * Returns a PolarizationTypeMod::PolarizationType from a string.
	 * @param xml the string to be converted into a PolarizationTypeMod::PolarizationType
	 * @return a PolarizationTypeMod::PolarizationType.
	 */
	static PolarizationTypeMod::PolarizationType getPolarizationType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<PolarizationTypeMod::PolarizationType> from a string.
	 * @param xml the string to be converted into a vector<PolarizationTypeMod::PolarizationType>
	 * @return a vector<PolarizationTypeMod::PolarizationType>.
	 */
	static vector<PolarizationTypeMod::PolarizationType> getPolarizationType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<PolarizationTypeMod::PolarizationType> > from a string.
	 * @param xml the string to be converted into a vector<vector<PolarizationTypeMod::PolarizationType> >
	 * @return a vector<vector<PolarizationTypeMod::PolarizationType> >.
	 */
	static vector<vector<PolarizationTypeMod::PolarizationType> > getPolarizationType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<PolarizationTypeMod::PolarizationType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<PolarizationTypeMod::PolarizationType> > >
	 * @return a vector<vector<vector<PolarizationTypeMod::PolarizationType> > >.
	 */
	static vector<vector<vector<PolarizationTypeMod::PolarizationType> > > getPolarizationType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  StokesParameterMod::StokesParameter value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, StokesParameterMod::StokesParameter e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<StokesParameterMod::StokesParameter>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<StokesParameterMod::StokesParameter>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<StokesParameterMod::StokesParameter> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<StokesParameterMod::StokesParameter> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<StokesParameterMod::StokesParameter> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<StokesParameterMod::StokesParameter> > >& vvv_e); 

	/**
	 * Returns a StokesParameterMod::StokesParameter from a string.
	 * @param xml the string to be converted into a StokesParameterMod::StokesParameter
	 * @return a StokesParameterMod::StokesParameter.
	 */
	static StokesParameterMod::StokesParameter getStokesParameter(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<StokesParameterMod::StokesParameter> from a string.
	 * @param xml the string to be converted into a vector<StokesParameterMod::StokesParameter>
	 * @return a vector<StokesParameterMod::StokesParameter>.
	 */
	static vector<StokesParameterMod::StokesParameter> getStokesParameter1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<StokesParameterMod::StokesParameter> > from a string.
	 * @param xml the string to be converted into a vector<vector<StokesParameterMod::StokesParameter> >
	 * @return a vector<vector<StokesParameterMod::StokesParameter> >.
	 */
	static vector<vector<StokesParameterMod::StokesParameter> > getStokesParameter2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<StokesParameterMod::StokesParameter> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<StokesParameterMod::StokesParameter> > >
	 * @return a vector<vector<vector<StokesParameterMod::StokesParameter> > >.
	 */
	static vector<vector<vector<StokesParameterMod::StokesParameter> > > getStokesParameter3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  NetSidebandMod::NetSideband value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, NetSidebandMod::NetSideband e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<NetSidebandMod::NetSideband>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<NetSidebandMod::NetSideband>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<NetSidebandMod::NetSideband> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<NetSidebandMod::NetSideband> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<NetSidebandMod::NetSideband> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<NetSidebandMod::NetSideband> > >& vvv_e); 

	/**
	 * Returns a NetSidebandMod::NetSideband from a string.
	 * @param xml the string to be converted into a NetSidebandMod::NetSideband
	 * @return a NetSidebandMod::NetSideband.
	 */
	static NetSidebandMod::NetSideband getNetSideband(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<NetSidebandMod::NetSideband> from a string.
	 * @param xml the string to be converted into a vector<NetSidebandMod::NetSideband>
	 * @return a vector<NetSidebandMod::NetSideband>.
	 */
	static vector<NetSidebandMod::NetSideband> getNetSideband1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<NetSidebandMod::NetSideband> > from a string.
	 * @param xml the string to be converted into a vector<vector<NetSidebandMod::NetSideband> >
	 * @return a vector<vector<NetSidebandMod::NetSideband> >.
	 */
	static vector<vector<NetSidebandMod::NetSideband> > getNetSideband2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<NetSidebandMod::NetSideband> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<NetSidebandMod::NetSideband> > >
	 * @return a vector<vector<vector<NetSidebandMod::NetSideband> > >.
	 */
	static vector<vector<vector<NetSidebandMod::NetSideband> > > getNetSideband3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  SidebandProcessingModeMod::SidebandProcessingMode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, SidebandProcessingModeMod::SidebandProcessingMode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<SidebandProcessingModeMod::SidebandProcessingMode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<SidebandProcessingModeMod::SidebandProcessingMode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> > >& vvv_e); 

	/**
	 * Returns a SidebandProcessingModeMod::SidebandProcessingMode from a string.
	 * @param xml the string to be converted into a SidebandProcessingModeMod::SidebandProcessingMode
	 * @return a SidebandProcessingModeMod::SidebandProcessingMode.
	 */
	static SidebandProcessingModeMod::SidebandProcessingMode getSidebandProcessingMode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<SidebandProcessingModeMod::SidebandProcessingMode> from a string.
	 * @param xml the string to be converted into a vector<SidebandProcessingModeMod::SidebandProcessingMode>
	 * @return a vector<SidebandProcessingModeMod::SidebandProcessingMode>.
	 */
	static vector<SidebandProcessingModeMod::SidebandProcessingMode> getSidebandProcessingMode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> > from a string.
	 * @param xml the string to be converted into a vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> >
	 * @return a vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> >.
	 */
	static vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> > getSidebandProcessingMode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> > >
	 * @return a vector<vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> > >.
	 */
	static vector<vector<vector<SidebandProcessingModeMod::SidebandProcessingMode> > > getSidebandProcessingMode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  BasebandNameMod::BasebandName value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, BasebandNameMod::BasebandName e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<BasebandNameMod::BasebandName>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<BasebandNameMod::BasebandName>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<BasebandNameMod::BasebandName> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<BasebandNameMod::BasebandName> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<BasebandNameMod::BasebandName> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<BasebandNameMod::BasebandName> > >& vvv_e); 

	/**
	 * Returns a BasebandNameMod::BasebandName from a string.
	 * @param xml the string to be converted into a BasebandNameMod::BasebandName
	 * @return a BasebandNameMod::BasebandName.
	 */
	static BasebandNameMod::BasebandName getBasebandName(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<BasebandNameMod::BasebandName> from a string.
	 * @param xml the string to be converted into a vector<BasebandNameMod::BasebandName>
	 * @return a vector<BasebandNameMod::BasebandName>.
	 */
	static vector<BasebandNameMod::BasebandName> getBasebandName1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<BasebandNameMod::BasebandName> > from a string.
	 * @param xml the string to be converted into a vector<vector<BasebandNameMod::BasebandName> >
	 * @return a vector<vector<BasebandNameMod::BasebandName> >.
	 */
	static vector<vector<BasebandNameMod::BasebandName> > getBasebandName2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<BasebandNameMod::BasebandName> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<BasebandNameMod::BasebandName> > >
	 * @return a vector<vector<vector<BasebandNameMod::BasebandName> > >.
	 */
	static vector<vector<vector<BasebandNameMod::BasebandName> > > getBasebandName3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  WindowFunctionMod::WindowFunction value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, WindowFunctionMod::WindowFunction e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<WindowFunctionMod::WindowFunction>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<WindowFunctionMod::WindowFunction>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<WindowFunctionMod::WindowFunction> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<WindowFunctionMod::WindowFunction> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<WindowFunctionMod::WindowFunction> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<WindowFunctionMod::WindowFunction> > >& vvv_e); 

	/**
	 * Returns a WindowFunctionMod::WindowFunction from a string.
	 * @param xml the string to be converted into a WindowFunctionMod::WindowFunction
	 * @return a WindowFunctionMod::WindowFunction.
	 */
	static WindowFunctionMod::WindowFunction getWindowFunction(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<WindowFunctionMod::WindowFunction> from a string.
	 * @param xml the string to be converted into a vector<WindowFunctionMod::WindowFunction>
	 * @return a vector<WindowFunctionMod::WindowFunction>.
	 */
	static vector<WindowFunctionMod::WindowFunction> getWindowFunction1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<WindowFunctionMod::WindowFunction> > from a string.
	 * @param xml the string to be converted into a vector<vector<WindowFunctionMod::WindowFunction> >
	 * @return a vector<vector<WindowFunctionMod::WindowFunction> >.
	 */
	static vector<vector<WindowFunctionMod::WindowFunction> > getWindowFunction2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<WindowFunctionMod::WindowFunction> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<WindowFunctionMod::WindowFunction> > >
	 * @return a vector<vector<vector<WindowFunctionMod::WindowFunction> > >.
	 */
	static vector<vector<vector<WindowFunctionMod::WindowFunction> > > getWindowFunction3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CorrelationBitMod::CorrelationBit value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CorrelationBitMod::CorrelationBit e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CorrelationBitMod::CorrelationBit>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CorrelationBitMod::CorrelationBit>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CorrelationBitMod::CorrelationBit> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CorrelationBitMod::CorrelationBit> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CorrelationBitMod::CorrelationBit> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CorrelationBitMod::CorrelationBit> > >& vvv_e); 

	/**
	 * Returns a CorrelationBitMod::CorrelationBit from a string.
	 * @param xml the string to be converted into a CorrelationBitMod::CorrelationBit
	 * @return a CorrelationBitMod::CorrelationBit.
	 */
	static CorrelationBitMod::CorrelationBit getCorrelationBit(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CorrelationBitMod::CorrelationBit> from a string.
	 * @param xml the string to be converted into a vector<CorrelationBitMod::CorrelationBit>
	 * @return a vector<CorrelationBitMod::CorrelationBit>.
	 */
	static vector<CorrelationBitMod::CorrelationBit> getCorrelationBit1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CorrelationBitMod::CorrelationBit> > from a string.
	 * @param xml the string to be converted into a vector<vector<CorrelationBitMod::CorrelationBit> >
	 * @return a vector<vector<CorrelationBitMod::CorrelationBit> >.
	 */
	static vector<vector<CorrelationBitMod::CorrelationBit> > getCorrelationBit2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CorrelationBitMod::CorrelationBit> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CorrelationBitMod::CorrelationBit> > >
	 * @return a vector<vector<vector<CorrelationBitMod::CorrelationBit> > >.
	 */
	static vector<vector<vector<CorrelationBitMod::CorrelationBit> > > getCorrelationBit3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  ReceiverBandMod::ReceiverBand value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, ReceiverBandMod::ReceiverBand e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<ReceiverBandMod::ReceiverBand>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<ReceiverBandMod::ReceiverBand>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<ReceiverBandMod::ReceiverBand> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<ReceiverBandMod::ReceiverBand> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<ReceiverBandMod::ReceiverBand> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<ReceiverBandMod::ReceiverBand> > >& vvv_e); 

	/**
	 * Returns a ReceiverBandMod::ReceiverBand from a string.
	 * @param xml the string to be converted into a ReceiverBandMod::ReceiverBand
	 * @return a ReceiverBandMod::ReceiverBand.
	 */
	static ReceiverBandMod::ReceiverBand getReceiverBand(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<ReceiverBandMod::ReceiverBand> from a string.
	 * @param xml the string to be converted into a vector<ReceiverBandMod::ReceiverBand>
	 * @return a vector<ReceiverBandMod::ReceiverBand>.
	 */
	static vector<ReceiverBandMod::ReceiverBand> getReceiverBand1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<ReceiverBandMod::ReceiverBand> > from a string.
	 * @param xml the string to be converted into a vector<vector<ReceiverBandMod::ReceiverBand> >
	 * @return a vector<vector<ReceiverBandMod::ReceiverBand> >.
	 */
	static vector<vector<ReceiverBandMod::ReceiverBand> > getReceiverBand2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<ReceiverBandMod::ReceiverBand> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<ReceiverBandMod::ReceiverBand> > >
	 * @return a vector<vector<vector<ReceiverBandMod::ReceiverBand> > >.
	 */
	static vector<vector<vector<ReceiverBandMod::ReceiverBand> > > getReceiverBand3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  ReceiverSidebandMod::ReceiverSideband value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, ReceiverSidebandMod::ReceiverSideband e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<ReceiverSidebandMod::ReceiverSideband>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<ReceiverSidebandMod::ReceiverSideband>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<ReceiverSidebandMod::ReceiverSideband> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<ReceiverSidebandMod::ReceiverSideband> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<ReceiverSidebandMod::ReceiverSideband> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<ReceiverSidebandMod::ReceiverSideband> > >& vvv_e); 

	/**
	 * Returns a ReceiverSidebandMod::ReceiverSideband from a string.
	 * @param xml the string to be converted into a ReceiverSidebandMod::ReceiverSideband
	 * @return a ReceiverSidebandMod::ReceiverSideband.
	 */
	static ReceiverSidebandMod::ReceiverSideband getReceiverSideband(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<ReceiverSidebandMod::ReceiverSideband> from a string.
	 * @param xml the string to be converted into a vector<ReceiverSidebandMod::ReceiverSideband>
	 * @return a vector<ReceiverSidebandMod::ReceiverSideband>.
	 */
	static vector<ReceiverSidebandMod::ReceiverSideband> getReceiverSideband1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<ReceiverSidebandMod::ReceiverSideband> > from a string.
	 * @param xml the string to be converted into a vector<vector<ReceiverSidebandMod::ReceiverSideband> >
	 * @return a vector<vector<ReceiverSidebandMod::ReceiverSideband> >.
	 */
	static vector<vector<ReceiverSidebandMod::ReceiverSideband> > getReceiverSideband2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<ReceiverSidebandMod::ReceiverSideband> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<ReceiverSidebandMod::ReceiverSideband> > >
	 * @return a vector<vector<vector<ReceiverSidebandMod::ReceiverSideband> > >.
	 */
	static vector<vector<vector<ReceiverSidebandMod::ReceiverSideband> > > getReceiverSideband3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  ProcessorTypeMod::ProcessorType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, ProcessorTypeMod::ProcessorType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<ProcessorTypeMod::ProcessorType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<ProcessorTypeMod::ProcessorType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<ProcessorTypeMod::ProcessorType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<ProcessorTypeMod::ProcessorType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<ProcessorTypeMod::ProcessorType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<ProcessorTypeMod::ProcessorType> > >& vvv_e); 

	/**
	 * Returns a ProcessorTypeMod::ProcessorType from a string.
	 * @param xml the string to be converted into a ProcessorTypeMod::ProcessorType
	 * @return a ProcessorTypeMod::ProcessorType.
	 */
	static ProcessorTypeMod::ProcessorType getProcessorType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<ProcessorTypeMod::ProcessorType> from a string.
	 * @param xml the string to be converted into a vector<ProcessorTypeMod::ProcessorType>
	 * @return a vector<ProcessorTypeMod::ProcessorType>.
	 */
	static vector<ProcessorTypeMod::ProcessorType> getProcessorType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<ProcessorTypeMod::ProcessorType> > from a string.
	 * @param xml the string to be converted into a vector<vector<ProcessorTypeMod::ProcessorType> >
	 * @return a vector<vector<ProcessorTypeMod::ProcessorType> >.
	 */
	static vector<vector<ProcessorTypeMod::ProcessorType> > getProcessorType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<ProcessorTypeMod::ProcessorType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<ProcessorTypeMod::ProcessorType> > >
	 * @return a vector<vector<vector<ProcessorTypeMod::ProcessorType> > >.
	 */
	static vector<vector<vector<ProcessorTypeMod::ProcessorType> > > getProcessorType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  AccumModeMod::AccumMode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, AccumModeMod::AccumMode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<AccumModeMod::AccumMode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<AccumModeMod::AccumMode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<AccumModeMod::AccumMode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<AccumModeMod::AccumMode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<AccumModeMod::AccumMode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<AccumModeMod::AccumMode> > >& vvv_e); 

	/**
	 * Returns a AccumModeMod::AccumMode from a string.
	 * @param xml the string to be converted into a AccumModeMod::AccumMode
	 * @return a AccumModeMod::AccumMode.
	 */
	static AccumModeMod::AccumMode getAccumMode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<AccumModeMod::AccumMode> from a string.
	 * @param xml the string to be converted into a vector<AccumModeMod::AccumMode>
	 * @return a vector<AccumModeMod::AccumMode>.
	 */
	static vector<AccumModeMod::AccumMode> getAccumMode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<AccumModeMod::AccumMode> > from a string.
	 * @param xml the string to be converted into a vector<vector<AccumModeMod::AccumMode> >
	 * @return a vector<vector<AccumModeMod::AccumMode> >.
	 */
	static vector<vector<AccumModeMod::AccumMode> > getAccumMode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<AccumModeMod::AccumMode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<AccumModeMod::AccumMode> > >
	 * @return a vector<vector<vector<AccumModeMod::AccumMode> > >.
	 */
	static vector<vector<vector<AccumModeMod::AccumMode> > > getAccumMode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  AxisNameMod::AxisName value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, AxisNameMod::AxisName e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<AxisNameMod::AxisName>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<AxisNameMod::AxisName>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<AxisNameMod::AxisName> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<AxisNameMod::AxisName> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<AxisNameMod::AxisName> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<AxisNameMod::AxisName> > >& vvv_e); 

	/**
	 * Returns a AxisNameMod::AxisName from a string.
	 * @param xml the string to be converted into a AxisNameMod::AxisName
	 * @return a AxisNameMod::AxisName.
	 */
	static AxisNameMod::AxisName getAxisName(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<AxisNameMod::AxisName> from a string.
	 * @param xml the string to be converted into a vector<AxisNameMod::AxisName>
	 * @return a vector<AxisNameMod::AxisName>.
	 */
	static vector<AxisNameMod::AxisName> getAxisName1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<AxisNameMod::AxisName> > from a string.
	 * @param xml the string to be converted into a vector<vector<AxisNameMod::AxisName> >
	 * @return a vector<vector<AxisNameMod::AxisName> >.
	 */
	static vector<vector<AxisNameMod::AxisName> > getAxisName2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<AxisNameMod::AxisName> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<AxisNameMod::AxisName> > >
	 * @return a vector<vector<vector<AxisNameMod::AxisName> > >.
	 */
	static vector<vector<vector<AxisNameMod::AxisName> > > getAxisName3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  FilterModeMod::FilterMode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, FilterModeMod::FilterMode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<FilterModeMod::FilterMode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<FilterModeMod::FilterMode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<FilterModeMod::FilterMode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<FilterModeMod::FilterMode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<FilterModeMod::FilterMode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<FilterModeMod::FilterMode> > >& vvv_e); 

	/**
	 * Returns a FilterModeMod::FilterMode from a string.
	 * @param xml the string to be converted into a FilterModeMod::FilterMode
	 * @return a FilterModeMod::FilterMode.
	 */
	static FilterModeMod::FilterMode getFilterMode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<FilterModeMod::FilterMode> from a string.
	 * @param xml the string to be converted into a vector<FilterModeMod::FilterMode>
	 * @return a vector<FilterModeMod::FilterMode>.
	 */
	static vector<FilterModeMod::FilterMode> getFilterMode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<FilterModeMod::FilterMode> > from a string.
	 * @param xml the string to be converted into a vector<vector<FilterModeMod::FilterMode> >
	 * @return a vector<vector<FilterModeMod::FilterMode> >.
	 */
	static vector<vector<FilterModeMod::FilterMode> > getFilterMode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<FilterModeMod::FilterMode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<FilterModeMod::FilterMode> > >
	 * @return a vector<vector<vector<FilterModeMod::FilterMode> > >.
	 */
	static vector<vector<vector<FilterModeMod::FilterMode> > > getFilterMode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CorrelatorNameMod::CorrelatorName value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CorrelatorNameMod::CorrelatorName e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CorrelatorNameMod::CorrelatorName>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CorrelatorNameMod::CorrelatorName>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CorrelatorNameMod::CorrelatorName> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CorrelatorNameMod::CorrelatorName> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CorrelatorNameMod::CorrelatorName> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CorrelatorNameMod::CorrelatorName> > >& vvv_e); 

	/**
	 * Returns a CorrelatorNameMod::CorrelatorName from a string.
	 * @param xml the string to be converted into a CorrelatorNameMod::CorrelatorName
	 * @return a CorrelatorNameMod::CorrelatorName.
	 */
	static CorrelatorNameMod::CorrelatorName getCorrelatorName(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CorrelatorNameMod::CorrelatorName> from a string.
	 * @param xml the string to be converted into a vector<CorrelatorNameMod::CorrelatorName>
	 * @return a vector<CorrelatorNameMod::CorrelatorName>.
	 */
	static vector<CorrelatorNameMod::CorrelatorName> getCorrelatorName1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CorrelatorNameMod::CorrelatorName> > from a string.
	 * @param xml the string to be converted into a vector<vector<CorrelatorNameMod::CorrelatorName> >
	 * @return a vector<vector<CorrelatorNameMod::CorrelatorName> >.
	 */
	static vector<vector<CorrelatorNameMod::CorrelatorName> > getCorrelatorName2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CorrelatorNameMod::CorrelatorName> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CorrelatorNameMod::CorrelatorName> > >
	 * @return a vector<vector<vector<CorrelatorNameMod::CorrelatorName> > >.
	 */
	static vector<vector<vector<CorrelatorNameMod::CorrelatorName> > > getCorrelatorName3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  ScanIntentMod::ScanIntent value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, ScanIntentMod::ScanIntent e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<ScanIntentMod::ScanIntent>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<ScanIntentMod::ScanIntent>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<ScanIntentMod::ScanIntent> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<ScanIntentMod::ScanIntent> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<ScanIntentMod::ScanIntent> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<ScanIntentMod::ScanIntent> > >& vvv_e); 

	/**
	 * Returns a ScanIntentMod::ScanIntent from a string.
	 * @param xml the string to be converted into a ScanIntentMod::ScanIntent
	 * @return a ScanIntentMod::ScanIntent.
	 */
	static ScanIntentMod::ScanIntent getScanIntent(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<ScanIntentMod::ScanIntent> from a string.
	 * @param xml the string to be converted into a vector<ScanIntentMod::ScanIntent>
	 * @return a vector<ScanIntentMod::ScanIntent>.
	 */
	static vector<ScanIntentMod::ScanIntent> getScanIntent1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<ScanIntentMod::ScanIntent> > from a string.
	 * @param xml the string to be converted into a vector<vector<ScanIntentMod::ScanIntent> >
	 * @return a vector<vector<ScanIntentMod::ScanIntent> >.
	 */
	static vector<vector<ScanIntentMod::ScanIntent> > getScanIntent2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<ScanIntentMod::ScanIntent> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<ScanIntentMod::ScanIntent> > >
	 * @return a vector<vector<vector<ScanIntentMod::ScanIntent> > >.
	 */
	static vector<vector<vector<ScanIntentMod::ScanIntent> > > getScanIntent3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  SubscanIntentMod::SubscanIntent value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, SubscanIntentMod::SubscanIntent e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<SubscanIntentMod::SubscanIntent>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<SubscanIntentMod::SubscanIntent>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<SubscanIntentMod::SubscanIntent> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<SubscanIntentMod::SubscanIntent> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<SubscanIntentMod::SubscanIntent> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<SubscanIntentMod::SubscanIntent> > >& vvv_e); 

	/**
	 * Returns a SubscanIntentMod::SubscanIntent from a string.
	 * @param xml the string to be converted into a SubscanIntentMod::SubscanIntent
	 * @return a SubscanIntentMod::SubscanIntent.
	 */
	static SubscanIntentMod::SubscanIntent getSubscanIntent(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<SubscanIntentMod::SubscanIntent> from a string.
	 * @param xml the string to be converted into a vector<SubscanIntentMod::SubscanIntent>
	 * @return a vector<SubscanIntentMod::SubscanIntent>.
	 */
	static vector<SubscanIntentMod::SubscanIntent> getSubscanIntent1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<SubscanIntentMod::SubscanIntent> > from a string.
	 * @param xml the string to be converted into a vector<vector<SubscanIntentMod::SubscanIntent> >
	 * @return a vector<vector<SubscanIntentMod::SubscanIntent> >.
	 */
	static vector<vector<SubscanIntentMod::SubscanIntent> > getSubscanIntent2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<SubscanIntentMod::SubscanIntent> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<SubscanIntentMod::SubscanIntent> > >
	 * @return a vector<vector<vector<SubscanIntentMod::SubscanIntent> > >.
	 */
	static vector<vector<vector<SubscanIntentMod::SubscanIntent> > > getSubscanIntent3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  SwitchingModeMod::SwitchingMode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, SwitchingModeMod::SwitchingMode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<SwitchingModeMod::SwitchingMode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<SwitchingModeMod::SwitchingMode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<SwitchingModeMod::SwitchingMode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<SwitchingModeMod::SwitchingMode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<SwitchingModeMod::SwitchingMode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<SwitchingModeMod::SwitchingMode> > >& vvv_e); 

	/**
	 * Returns a SwitchingModeMod::SwitchingMode from a string.
	 * @param xml the string to be converted into a SwitchingModeMod::SwitchingMode
	 * @return a SwitchingModeMod::SwitchingMode.
	 */
	static SwitchingModeMod::SwitchingMode getSwitchingMode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<SwitchingModeMod::SwitchingMode> from a string.
	 * @param xml the string to be converted into a vector<SwitchingModeMod::SwitchingMode>
	 * @return a vector<SwitchingModeMod::SwitchingMode>.
	 */
	static vector<SwitchingModeMod::SwitchingMode> getSwitchingMode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<SwitchingModeMod::SwitchingMode> > from a string.
	 * @param xml the string to be converted into a vector<vector<SwitchingModeMod::SwitchingMode> >
	 * @return a vector<vector<SwitchingModeMod::SwitchingMode> >.
	 */
	static vector<vector<SwitchingModeMod::SwitchingMode> > getSwitchingMode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<SwitchingModeMod::SwitchingMode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<SwitchingModeMod::SwitchingMode> > >
	 * @return a vector<vector<vector<SwitchingModeMod::SwitchingMode> > >.
	 */
	static vector<vector<vector<SwitchingModeMod::SwitchingMode> > > getSwitchingMode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CorrelatorCalibrationMod::CorrelatorCalibration value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CorrelatorCalibrationMod::CorrelatorCalibration e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CorrelatorCalibrationMod::CorrelatorCalibration>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CorrelatorCalibrationMod::CorrelatorCalibration>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> > >& vvv_e); 

	/**
	 * Returns a CorrelatorCalibrationMod::CorrelatorCalibration from a string.
	 * @param xml the string to be converted into a CorrelatorCalibrationMod::CorrelatorCalibration
	 * @return a CorrelatorCalibrationMod::CorrelatorCalibration.
	 */
	static CorrelatorCalibrationMod::CorrelatorCalibration getCorrelatorCalibration(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CorrelatorCalibrationMod::CorrelatorCalibration> from a string.
	 * @param xml the string to be converted into a vector<CorrelatorCalibrationMod::CorrelatorCalibration>
	 * @return a vector<CorrelatorCalibrationMod::CorrelatorCalibration>.
	 */
	static vector<CorrelatorCalibrationMod::CorrelatorCalibration> getCorrelatorCalibration1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> > from a string.
	 * @param xml the string to be converted into a vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> >
	 * @return a vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> >.
	 */
	static vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> > getCorrelatorCalibration2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> > >
	 * @return a vector<vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> > >.
	 */
	static vector<vector<vector<CorrelatorCalibrationMod::CorrelatorCalibration> > > getCorrelatorCalibration3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  TimeSamplingMod::TimeSampling value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, TimeSamplingMod::TimeSampling e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<TimeSamplingMod::TimeSampling>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<TimeSamplingMod::TimeSampling>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<TimeSamplingMod::TimeSampling> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<TimeSamplingMod::TimeSampling> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<TimeSamplingMod::TimeSampling> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<TimeSamplingMod::TimeSampling> > >& vvv_e); 

	/**
	 * Returns a TimeSamplingMod::TimeSampling from a string.
	 * @param xml the string to be converted into a TimeSamplingMod::TimeSampling
	 * @return a TimeSamplingMod::TimeSampling.
	 */
	static TimeSamplingMod::TimeSampling getTimeSampling(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<TimeSamplingMod::TimeSampling> from a string.
	 * @param xml the string to be converted into a vector<TimeSamplingMod::TimeSampling>
	 * @return a vector<TimeSamplingMod::TimeSampling>.
	 */
	static vector<TimeSamplingMod::TimeSampling> getTimeSampling1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<TimeSamplingMod::TimeSampling> > from a string.
	 * @param xml the string to be converted into a vector<vector<TimeSamplingMod::TimeSampling> >
	 * @return a vector<vector<TimeSamplingMod::TimeSampling> >.
	 */
	static vector<vector<TimeSamplingMod::TimeSampling> > getTimeSampling2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<TimeSamplingMod::TimeSampling> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<TimeSamplingMod::TimeSampling> > >
	 * @return a vector<vector<vector<TimeSamplingMod::TimeSampling> > >.
	 */
	static vector<vector<vector<TimeSamplingMod::TimeSampling> > > getTimeSampling3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CalTypeMod::CalType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CalTypeMod::CalType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CalTypeMod::CalType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CalTypeMod::CalType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CalTypeMod::CalType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CalTypeMod::CalType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CalTypeMod::CalType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CalTypeMod::CalType> > >& vvv_e); 

	/**
	 * Returns a CalTypeMod::CalType from a string.
	 * @param xml the string to be converted into a CalTypeMod::CalType
	 * @return a CalTypeMod::CalType.
	 */
	static CalTypeMod::CalType getCalType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CalTypeMod::CalType> from a string.
	 * @param xml the string to be converted into a vector<CalTypeMod::CalType>
	 * @return a vector<CalTypeMod::CalType>.
	 */
	static vector<CalTypeMod::CalType> getCalType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CalTypeMod::CalType> > from a string.
	 * @param xml the string to be converted into a vector<vector<CalTypeMod::CalType> >
	 * @return a vector<vector<CalTypeMod::CalType> >.
	 */
	static vector<vector<CalTypeMod::CalType> > getCalType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CalTypeMod::CalType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CalTypeMod::CalType> > >
	 * @return a vector<vector<vector<CalTypeMod::CalType> > >.
	 */
	static vector<vector<vector<CalTypeMod::CalType> > > getCalType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  AssociatedCalNatureMod::AssociatedCalNature value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, AssociatedCalNatureMod::AssociatedCalNature e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<AssociatedCalNatureMod::AssociatedCalNature>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<AssociatedCalNatureMod::AssociatedCalNature>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<AssociatedCalNatureMod::AssociatedCalNature> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<AssociatedCalNatureMod::AssociatedCalNature> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<AssociatedCalNatureMod::AssociatedCalNature> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<AssociatedCalNatureMod::AssociatedCalNature> > >& vvv_e); 

	/**
	 * Returns a AssociatedCalNatureMod::AssociatedCalNature from a string.
	 * @param xml the string to be converted into a AssociatedCalNatureMod::AssociatedCalNature
	 * @return a AssociatedCalNatureMod::AssociatedCalNature.
	 */
	static AssociatedCalNatureMod::AssociatedCalNature getAssociatedCalNature(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<AssociatedCalNatureMod::AssociatedCalNature> from a string.
	 * @param xml the string to be converted into a vector<AssociatedCalNatureMod::AssociatedCalNature>
	 * @return a vector<AssociatedCalNatureMod::AssociatedCalNature>.
	 */
	static vector<AssociatedCalNatureMod::AssociatedCalNature> getAssociatedCalNature1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<AssociatedCalNatureMod::AssociatedCalNature> > from a string.
	 * @param xml the string to be converted into a vector<vector<AssociatedCalNatureMod::AssociatedCalNature> >
	 * @return a vector<vector<AssociatedCalNatureMod::AssociatedCalNature> >.
	 */
	static vector<vector<AssociatedCalNatureMod::AssociatedCalNature> > getAssociatedCalNature2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<AssociatedCalNatureMod::AssociatedCalNature> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<AssociatedCalNatureMod::AssociatedCalNature> > >
	 * @return a vector<vector<vector<AssociatedCalNatureMod::AssociatedCalNature> > >.
	 */
	static vector<vector<vector<AssociatedCalNatureMod::AssociatedCalNature> > > getAssociatedCalNature3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CalDataOriginMod::CalDataOrigin value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CalDataOriginMod::CalDataOrigin e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CalDataOriginMod::CalDataOrigin>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CalDataOriginMod::CalDataOrigin>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CalDataOriginMod::CalDataOrigin> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CalDataOriginMod::CalDataOrigin> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CalDataOriginMod::CalDataOrigin> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CalDataOriginMod::CalDataOrigin> > >& vvv_e); 

	/**
	 * Returns a CalDataOriginMod::CalDataOrigin from a string.
	 * @param xml the string to be converted into a CalDataOriginMod::CalDataOrigin
	 * @return a CalDataOriginMod::CalDataOrigin.
	 */
	static CalDataOriginMod::CalDataOrigin getCalDataOrigin(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CalDataOriginMod::CalDataOrigin> from a string.
	 * @param xml the string to be converted into a vector<CalDataOriginMod::CalDataOrigin>
	 * @return a vector<CalDataOriginMod::CalDataOrigin>.
	 */
	static vector<CalDataOriginMod::CalDataOrigin> getCalDataOrigin1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CalDataOriginMod::CalDataOrigin> > from a string.
	 * @param xml the string to be converted into a vector<vector<CalDataOriginMod::CalDataOrigin> >
	 * @return a vector<vector<CalDataOriginMod::CalDataOrigin> >.
	 */
	static vector<vector<CalDataOriginMod::CalDataOrigin> > getCalDataOrigin2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CalDataOriginMod::CalDataOrigin> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CalDataOriginMod::CalDataOrigin> > >
	 * @return a vector<vector<vector<CalDataOriginMod::CalDataOrigin> > >.
	 */
	static vector<vector<vector<CalDataOriginMod::CalDataOrigin> > > getCalDataOrigin3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  InvalidatingConditionMod::InvalidatingCondition value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, InvalidatingConditionMod::InvalidatingCondition e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<InvalidatingConditionMod::InvalidatingCondition>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<InvalidatingConditionMod::InvalidatingCondition>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<InvalidatingConditionMod::InvalidatingCondition> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<InvalidatingConditionMod::InvalidatingCondition> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<InvalidatingConditionMod::InvalidatingCondition> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<InvalidatingConditionMod::InvalidatingCondition> > >& vvv_e); 

	/**
	 * Returns a InvalidatingConditionMod::InvalidatingCondition from a string.
	 * @param xml the string to be converted into a InvalidatingConditionMod::InvalidatingCondition
	 * @return a InvalidatingConditionMod::InvalidatingCondition.
	 */
	static InvalidatingConditionMod::InvalidatingCondition getInvalidatingCondition(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<InvalidatingConditionMod::InvalidatingCondition> from a string.
	 * @param xml the string to be converted into a vector<InvalidatingConditionMod::InvalidatingCondition>
	 * @return a vector<InvalidatingConditionMod::InvalidatingCondition>.
	 */
	static vector<InvalidatingConditionMod::InvalidatingCondition> getInvalidatingCondition1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<InvalidatingConditionMod::InvalidatingCondition> > from a string.
	 * @param xml the string to be converted into a vector<vector<InvalidatingConditionMod::InvalidatingCondition> >
	 * @return a vector<vector<InvalidatingConditionMod::InvalidatingCondition> >.
	 */
	static vector<vector<InvalidatingConditionMod::InvalidatingCondition> > getInvalidatingCondition2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<InvalidatingConditionMod::InvalidatingCondition> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<InvalidatingConditionMod::InvalidatingCondition> > >
	 * @return a vector<vector<vector<InvalidatingConditionMod::InvalidatingCondition> > >.
	 */
	static vector<vector<vector<InvalidatingConditionMod::InvalidatingCondition> > > getInvalidatingCondition3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  PositionMethodMod::PositionMethod value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, PositionMethodMod::PositionMethod e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<PositionMethodMod::PositionMethod>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<PositionMethodMod::PositionMethod>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<PositionMethodMod::PositionMethod> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<PositionMethodMod::PositionMethod> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<PositionMethodMod::PositionMethod> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<PositionMethodMod::PositionMethod> > >& vvv_e); 

	/**
	 * Returns a PositionMethodMod::PositionMethod from a string.
	 * @param xml the string to be converted into a PositionMethodMod::PositionMethod
	 * @return a PositionMethodMod::PositionMethod.
	 */
	static PositionMethodMod::PositionMethod getPositionMethod(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<PositionMethodMod::PositionMethod> from a string.
	 * @param xml the string to be converted into a vector<PositionMethodMod::PositionMethod>
	 * @return a vector<PositionMethodMod::PositionMethod>.
	 */
	static vector<PositionMethodMod::PositionMethod> getPositionMethod1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<PositionMethodMod::PositionMethod> > from a string.
	 * @param xml the string to be converted into a vector<vector<PositionMethodMod::PositionMethod> >
	 * @return a vector<vector<PositionMethodMod::PositionMethod> >.
	 */
	static vector<vector<PositionMethodMod::PositionMethod> > getPositionMethod2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<PositionMethodMod::PositionMethod> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<PositionMethodMod::PositionMethod> > >
	 * @return a vector<vector<vector<PositionMethodMod::PositionMethod> > >.
	 */
	static vector<vector<vector<PositionMethodMod::PositionMethod> > > getPositionMethod3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  PointingMethodMod::PointingMethod value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, PointingMethodMod::PointingMethod e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<PointingMethodMod::PointingMethod>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<PointingMethodMod::PointingMethod>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<PointingMethodMod::PointingMethod> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<PointingMethodMod::PointingMethod> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<PointingMethodMod::PointingMethod> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<PointingMethodMod::PointingMethod> > >& vvv_e); 

	/**
	 * Returns a PointingMethodMod::PointingMethod from a string.
	 * @param xml the string to be converted into a PointingMethodMod::PointingMethod
	 * @return a PointingMethodMod::PointingMethod.
	 */
	static PointingMethodMod::PointingMethod getPointingMethod(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<PointingMethodMod::PointingMethod> from a string.
	 * @param xml the string to be converted into a vector<PointingMethodMod::PointingMethod>
	 * @return a vector<PointingMethodMod::PointingMethod>.
	 */
	static vector<PointingMethodMod::PointingMethod> getPointingMethod1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<PointingMethodMod::PointingMethod> > from a string.
	 * @param xml the string to be converted into a vector<vector<PointingMethodMod::PointingMethod> >
	 * @return a vector<vector<PointingMethodMod::PointingMethod> >.
	 */
	static vector<vector<PointingMethodMod::PointingMethod> > getPointingMethod2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<PointingMethodMod::PointingMethod> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<PointingMethodMod::PointingMethod> > >
	 * @return a vector<vector<vector<PointingMethodMod::PointingMethod> > >.
	 */
	static vector<vector<vector<PointingMethodMod::PointingMethod> > > getPointingMethod3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  PointingModelModeMod::PointingModelMode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, PointingModelModeMod::PointingModelMode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<PointingModelModeMod::PointingModelMode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<PointingModelModeMod::PointingModelMode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<PointingModelModeMod::PointingModelMode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<PointingModelModeMod::PointingModelMode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<PointingModelModeMod::PointingModelMode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<PointingModelModeMod::PointingModelMode> > >& vvv_e); 

	/**
	 * Returns a PointingModelModeMod::PointingModelMode from a string.
	 * @param xml the string to be converted into a PointingModelModeMod::PointingModelMode
	 * @return a PointingModelModeMod::PointingModelMode.
	 */
	static PointingModelModeMod::PointingModelMode getPointingModelMode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<PointingModelModeMod::PointingModelMode> from a string.
	 * @param xml the string to be converted into a vector<PointingModelModeMod::PointingModelMode>
	 * @return a vector<PointingModelModeMod::PointingModelMode>.
	 */
	static vector<PointingModelModeMod::PointingModelMode> getPointingModelMode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<PointingModelModeMod::PointingModelMode> > from a string.
	 * @param xml the string to be converted into a vector<vector<PointingModelModeMod::PointingModelMode> >
	 * @return a vector<vector<PointingModelModeMod::PointingModelMode> >.
	 */
	static vector<vector<PointingModelModeMod::PointingModelMode> > getPointingModelMode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<PointingModelModeMod::PointingModelMode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<PointingModelModeMod::PointingModelMode> > >
	 * @return a vector<vector<vector<PointingModelModeMod::PointingModelMode> > >.
	 */
	static vector<vector<vector<PointingModelModeMod::PointingModelMode> > > getPointingModelMode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  SyscalMethodMod::SyscalMethod value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, SyscalMethodMod::SyscalMethod e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<SyscalMethodMod::SyscalMethod>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<SyscalMethodMod::SyscalMethod>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<SyscalMethodMod::SyscalMethod> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<SyscalMethodMod::SyscalMethod> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<SyscalMethodMod::SyscalMethod> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<SyscalMethodMod::SyscalMethod> > >& vvv_e); 

	/**
	 * Returns a SyscalMethodMod::SyscalMethod from a string.
	 * @param xml the string to be converted into a SyscalMethodMod::SyscalMethod
	 * @return a SyscalMethodMod::SyscalMethod.
	 */
	static SyscalMethodMod::SyscalMethod getSyscalMethod(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<SyscalMethodMod::SyscalMethod> from a string.
	 * @param xml the string to be converted into a vector<SyscalMethodMod::SyscalMethod>
	 * @return a vector<SyscalMethodMod::SyscalMethod>.
	 */
	static vector<SyscalMethodMod::SyscalMethod> getSyscalMethod1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<SyscalMethodMod::SyscalMethod> > from a string.
	 * @param xml the string to be converted into a vector<vector<SyscalMethodMod::SyscalMethod> >
	 * @return a vector<vector<SyscalMethodMod::SyscalMethod> >.
	 */
	static vector<vector<SyscalMethodMod::SyscalMethod> > getSyscalMethod2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<SyscalMethodMod::SyscalMethod> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<SyscalMethodMod::SyscalMethod> > >
	 * @return a vector<vector<vector<SyscalMethodMod::SyscalMethod> > >.
	 */
	static vector<vector<vector<SyscalMethodMod::SyscalMethod> > > getSyscalMethod3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CalCurveTypeMod::CalCurveType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CalCurveTypeMod::CalCurveType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CalCurveTypeMod::CalCurveType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CalCurveTypeMod::CalCurveType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CalCurveTypeMod::CalCurveType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CalCurveTypeMod::CalCurveType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CalCurveTypeMod::CalCurveType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CalCurveTypeMod::CalCurveType> > >& vvv_e); 

	/**
	 * Returns a CalCurveTypeMod::CalCurveType from a string.
	 * @param xml the string to be converted into a CalCurveTypeMod::CalCurveType
	 * @return a CalCurveTypeMod::CalCurveType.
	 */
	static CalCurveTypeMod::CalCurveType getCalCurveType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CalCurveTypeMod::CalCurveType> from a string.
	 * @param xml the string to be converted into a vector<CalCurveTypeMod::CalCurveType>
	 * @return a vector<CalCurveTypeMod::CalCurveType>.
	 */
	static vector<CalCurveTypeMod::CalCurveType> getCalCurveType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CalCurveTypeMod::CalCurveType> > from a string.
	 * @param xml the string to be converted into a vector<vector<CalCurveTypeMod::CalCurveType> >
	 * @return a vector<vector<CalCurveTypeMod::CalCurveType> >.
	 */
	static vector<vector<CalCurveTypeMod::CalCurveType> > getCalCurveType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CalCurveTypeMod::CalCurveType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CalCurveTypeMod::CalCurveType> > >
	 * @return a vector<vector<vector<CalCurveTypeMod::CalCurveType> > >.
	 */
	static vector<vector<vector<CalCurveTypeMod::CalCurveType> > > getCalCurveType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  StationTypeMod::StationType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, StationTypeMod::StationType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<StationTypeMod::StationType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<StationTypeMod::StationType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<StationTypeMod::StationType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<StationTypeMod::StationType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<StationTypeMod::StationType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<StationTypeMod::StationType> > >& vvv_e); 

	/**
	 * Returns a StationTypeMod::StationType from a string.
	 * @param xml the string to be converted into a StationTypeMod::StationType
	 * @return a StationTypeMod::StationType.
	 */
	static StationTypeMod::StationType getStationType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<StationTypeMod::StationType> from a string.
	 * @param xml the string to be converted into a vector<StationTypeMod::StationType>
	 * @return a vector<StationTypeMod::StationType>.
	 */
	static vector<StationTypeMod::StationType> getStationType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<StationTypeMod::StationType> > from a string.
	 * @param xml the string to be converted into a vector<vector<StationTypeMod::StationType> >
	 * @return a vector<vector<StationTypeMod::StationType> >.
	 */
	static vector<vector<StationTypeMod::StationType> > getStationType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<StationTypeMod::StationType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<StationTypeMod::StationType> > >
	 * @return a vector<vector<vector<StationTypeMod::StationType> > >.
	 */
	static vector<vector<vector<StationTypeMod::StationType> > > getStationType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  DetectorBandTypeMod::DetectorBandType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, DetectorBandTypeMod::DetectorBandType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<DetectorBandTypeMod::DetectorBandType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<DetectorBandTypeMod::DetectorBandType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<DetectorBandTypeMod::DetectorBandType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<DetectorBandTypeMod::DetectorBandType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<DetectorBandTypeMod::DetectorBandType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<DetectorBandTypeMod::DetectorBandType> > >& vvv_e); 

	/**
	 * Returns a DetectorBandTypeMod::DetectorBandType from a string.
	 * @param xml the string to be converted into a DetectorBandTypeMod::DetectorBandType
	 * @return a DetectorBandTypeMod::DetectorBandType.
	 */
	static DetectorBandTypeMod::DetectorBandType getDetectorBandType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<DetectorBandTypeMod::DetectorBandType> from a string.
	 * @param xml the string to be converted into a vector<DetectorBandTypeMod::DetectorBandType>
	 * @return a vector<DetectorBandTypeMod::DetectorBandType>.
	 */
	static vector<DetectorBandTypeMod::DetectorBandType> getDetectorBandType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<DetectorBandTypeMod::DetectorBandType> > from a string.
	 * @param xml the string to be converted into a vector<vector<DetectorBandTypeMod::DetectorBandType> >
	 * @return a vector<vector<DetectorBandTypeMod::DetectorBandType> >.
	 */
	static vector<vector<DetectorBandTypeMod::DetectorBandType> > getDetectorBandType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<DetectorBandTypeMod::DetectorBandType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<DetectorBandTypeMod::DetectorBandType> > >
	 * @return a vector<vector<vector<DetectorBandTypeMod::DetectorBandType> > >.
	 */
	static vector<vector<vector<DetectorBandTypeMod::DetectorBandType> > > getDetectorBandType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  FocusMethodMod::FocusMethod value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, FocusMethodMod::FocusMethod e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<FocusMethodMod::FocusMethod>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<FocusMethodMod::FocusMethod>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<FocusMethodMod::FocusMethod> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<FocusMethodMod::FocusMethod> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<FocusMethodMod::FocusMethod> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<FocusMethodMod::FocusMethod> > >& vvv_e); 

	/**
	 * Returns a FocusMethodMod::FocusMethod from a string.
	 * @param xml the string to be converted into a FocusMethodMod::FocusMethod
	 * @return a FocusMethodMod::FocusMethod.
	 */
	static FocusMethodMod::FocusMethod getFocusMethod(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<FocusMethodMod::FocusMethod> from a string.
	 * @param xml the string to be converted into a vector<FocusMethodMod::FocusMethod>
	 * @return a vector<FocusMethodMod::FocusMethod>.
	 */
	static vector<FocusMethodMod::FocusMethod> getFocusMethod1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<FocusMethodMod::FocusMethod> > from a string.
	 * @param xml the string to be converted into a vector<vector<FocusMethodMod::FocusMethod> >
	 * @return a vector<vector<FocusMethodMod::FocusMethod> >.
	 */
	static vector<vector<FocusMethodMod::FocusMethod> > getFocusMethod2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<FocusMethodMod::FocusMethod> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<FocusMethodMod::FocusMethod> > >
	 * @return a vector<vector<vector<FocusMethodMod::FocusMethod> > >.
	 */
	static vector<vector<vector<FocusMethodMod::FocusMethod> > > getFocusMethod3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  HolographyChannelTypeMod::HolographyChannelType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, HolographyChannelTypeMod::HolographyChannelType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<HolographyChannelTypeMod::HolographyChannelType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<HolographyChannelTypeMod::HolographyChannelType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<HolographyChannelTypeMod::HolographyChannelType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<HolographyChannelTypeMod::HolographyChannelType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<HolographyChannelTypeMod::HolographyChannelType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<HolographyChannelTypeMod::HolographyChannelType> > >& vvv_e); 

	/**
	 * Returns a HolographyChannelTypeMod::HolographyChannelType from a string.
	 * @param xml the string to be converted into a HolographyChannelTypeMod::HolographyChannelType
	 * @return a HolographyChannelTypeMod::HolographyChannelType.
	 */
	static HolographyChannelTypeMod::HolographyChannelType getHolographyChannelType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<HolographyChannelTypeMod::HolographyChannelType> from a string.
	 * @param xml the string to be converted into a vector<HolographyChannelTypeMod::HolographyChannelType>
	 * @return a vector<HolographyChannelTypeMod::HolographyChannelType>.
	 */
	static vector<HolographyChannelTypeMod::HolographyChannelType> getHolographyChannelType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<HolographyChannelTypeMod::HolographyChannelType> > from a string.
	 * @param xml the string to be converted into a vector<vector<HolographyChannelTypeMod::HolographyChannelType> >
	 * @return a vector<vector<HolographyChannelTypeMod::HolographyChannelType> >.
	 */
	static vector<vector<HolographyChannelTypeMod::HolographyChannelType> > getHolographyChannelType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<HolographyChannelTypeMod::HolographyChannelType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<HolographyChannelTypeMod::HolographyChannelType> > >
	 * @return a vector<vector<vector<HolographyChannelTypeMod::HolographyChannelType> > >.
	 */
	static vector<vector<vector<HolographyChannelTypeMod::HolographyChannelType> > > getHolographyChannelType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  FluxCalibrationMethodMod::FluxCalibrationMethod value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, FluxCalibrationMethodMod::FluxCalibrationMethod e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<FluxCalibrationMethodMod::FluxCalibrationMethod>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<FluxCalibrationMethodMod::FluxCalibrationMethod>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> > >& vvv_e); 

	/**
	 * Returns a FluxCalibrationMethodMod::FluxCalibrationMethod from a string.
	 * @param xml the string to be converted into a FluxCalibrationMethodMod::FluxCalibrationMethod
	 * @return a FluxCalibrationMethodMod::FluxCalibrationMethod.
	 */
	static FluxCalibrationMethodMod::FluxCalibrationMethod getFluxCalibrationMethod(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<FluxCalibrationMethodMod::FluxCalibrationMethod> from a string.
	 * @param xml the string to be converted into a vector<FluxCalibrationMethodMod::FluxCalibrationMethod>
	 * @return a vector<FluxCalibrationMethodMod::FluxCalibrationMethod>.
	 */
	static vector<FluxCalibrationMethodMod::FluxCalibrationMethod> getFluxCalibrationMethod1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> > from a string.
	 * @param xml the string to be converted into a vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> >
	 * @return a vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> >.
	 */
	static vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> > getFluxCalibrationMethod2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> > >
	 * @return a vector<vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> > >.
	 */
	static vector<vector<vector<FluxCalibrationMethodMod::FluxCalibrationMethod> > > getFluxCalibrationMethod3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  WVRMethodMod::WVRMethod value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, WVRMethodMod::WVRMethod e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<WVRMethodMod::WVRMethod>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<WVRMethodMod::WVRMethod>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<WVRMethodMod::WVRMethod> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<WVRMethodMod::WVRMethod> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<WVRMethodMod::WVRMethod> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<WVRMethodMod::WVRMethod> > >& vvv_e); 

	/**
	 * Returns a WVRMethodMod::WVRMethod from a string.
	 * @param xml the string to be converted into a WVRMethodMod::WVRMethod
	 * @return a WVRMethodMod::WVRMethod.
	 */
	static WVRMethodMod::WVRMethod getWVRMethod(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<WVRMethodMod::WVRMethod> from a string.
	 * @param xml the string to be converted into a vector<WVRMethodMod::WVRMethod>
	 * @return a vector<WVRMethodMod::WVRMethod>.
	 */
	static vector<WVRMethodMod::WVRMethod> getWVRMethod1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<WVRMethodMod::WVRMethod> > from a string.
	 * @param xml the string to be converted into a vector<vector<WVRMethodMod::WVRMethod> >
	 * @return a vector<vector<WVRMethodMod::WVRMethod> >.
	 */
	static vector<vector<WVRMethodMod::WVRMethod> > getWVRMethod2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<WVRMethodMod::WVRMethod> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<WVRMethodMod::WVRMethod> > >
	 * @return a vector<vector<vector<WVRMethodMod::WVRMethod> > >.
	 */
	static vector<vector<vector<WVRMethodMod::WVRMethod> > > getWVRMethod3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  CalibrationModeMod::CalibrationMode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, CalibrationModeMod::CalibrationMode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<CalibrationModeMod::CalibrationMode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<CalibrationModeMod::CalibrationMode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<CalibrationModeMod::CalibrationMode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<CalibrationModeMod::CalibrationMode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<CalibrationModeMod::CalibrationMode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<CalibrationModeMod::CalibrationMode> > >& vvv_e); 

	/**
	 * Returns a CalibrationModeMod::CalibrationMode from a string.
	 * @param xml the string to be converted into a CalibrationModeMod::CalibrationMode
	 * @return a CalibrationModeMod::CalibrationMode.
	 */
	static CalibrationModeMod::CalibrationMode getCalibrationMode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<CalibrationModeMod::CalibrationMode> from a string.
	 * @param xml the string to be converted into a vector<CalibrationModeMod::CalibrationMode>
	 * @return a vector<CalibrationModeMod::CalibrationMode>.
	 */
	static vector<CalibrationModeMod::CalibrationMode> getCalibrationMode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<CalibrationModeMod::CalibrationMode> > from a string.
	 * @param xml the string to be converted into a vector<vector<CalibrationModeMod::CalibrationMode> >
	 * @return a vector<vector<CalibrationModeMod::CalibrationMode> >.
	 */
	static vector<vector<CalibrationModeMod::CalibrationMode> > getCalibrationMode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<CalibrationModeMod::CalibrationMode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<CalibrationModeMod::CalibrationMode> > >
	 * @return a vector<vector<vector<CalibrationModeMod::CalibrationMode> > >.
	 */
	static vector<vector<vector<CalibrationModeMod::CalibrationMode> > > getCalibrationMode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  AssociatedFieldNatureMod::AssociatedFieldNature value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, AssociatedFieldNatureMod::AssociatedFieldNature e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<AssociatedFieldNatureMod::AssociatedFieldNature>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<AssociatedFieldNatureMod::AssociatedFieldNature>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> > >& vvv_e); 

	/**
	 * Returns a AssociatedFieldNatureMod::AssociatedFieldNature from a string.
	 * @param xml the string to be converted into a AssociatedFieldNatureMod::AssociatedFieldNature
	 * @return a AssociatedFieldNatureMod::AssociatedFieldNature.
	 */
	static AssociatedFieldNatureMod::AssociatedFieldNature getAssociatedFieldNature(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<AssociatedFieldNatureMod::AssociatedFieldNature> from a string.
	 * @param xml the string to be converted into a vector<AssociatedFieldNatureMod::AssociatedFieldNature>
	 * @return a vector<AssociatedFieldNatureMod::AssociatedFieldNature>.
	 */
	static vector<AssociatedFieldNatureMod::AssociatedFieldNature> getAssociatedFieldNature1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> > from a string.
	 * @param xml the string to be converted into a vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> >
	 * @return a vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> >.
	 */
	static vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> > getAssociatedFieldNature2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> > >
	 * @return a vector<vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> > >.
	 */
	static vector<vector<vector<AssociatedFieldNatureMod::AssociatedFieldNature> > > getAssociatedFieldNature3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  DataContentMod::DataContent value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, DataContentMod::DataContent e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<DataContentMod::DataContent>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<DataContentMod::DataContent>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<DataContentMod::DataContent> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<DataContentMod::DataContent> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<DataContentMod::DataContent> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<DataContentMod::DataContent> > >& vvv_e); 

	/**
	 * Returns a DataContentMod::DataContent from a string.
	 * @param xml the string to be converted into a DataContentMod::DataContent
	 * @return a DataContentMod::DataContent.
	 */
	static DataContentMod::DataContent getDataContent(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<DataContentMod::DataContent> from a string.
	 * @param xml the string to be converted into a vector<DataContentMod::DataContent>
	 * @return a vector<DataContentMod::DataContent>.
	 */
	static vector<DataContentMod::DataContent> getDataContent1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<DataContentMod::DataContent> > from a string.
	 * @param xml the string to be converted into a vector<vector<DataContentMod::DataContent> >
	 * @return a vector<vector<DataContentMod::DataContent> >.
	 */
	static vector<vector<DataContentMod::DataContent> > getDataContent2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<DataContentMod::DataContent> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<DataContentMod::DataContent> > >
	 * @return a vector<vector<vector<DataContentMod::DataContent> > >.
	 */
	static vector<vector<vector<DataContentMod::DataContent> > > getDataContent3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  PrimitiveDataTypeMod::PrimitiveDataType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, PrimitiveDataTypeMod::PrimitiveDataType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<PrimitiveDataTypeMod::PrimitiveDataType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<PrimitiveDataTypeMod::PrimitiveDataType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> > >& vvv_e); 

	/**
	 * Returns a PrimitiveDataTypeMod::PrimitiveDataType from a string.
	 * @param xml the string to be converted into a PrimitiveDataTypeMod::PrimitiveDataType
	 * @return a PrimitiveDataTypeMod::PrimitiveDataType.
	 */
	static PrimitiveDataTypeMod::PrimitiveDataType getPrimitiveDataType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<PrimitiveDataTypeMod::PrimitiveDataType> from a string.
	 * @param xml the string to be converted into a vector<PrimitiveDataTypeMod::PrimitiveDataType>
	 * @return a vector<PrimitiveDataTypeMod::PrimitiveDataType>.
	 */
	static vector<PrimitiveDataTypeMod::PrimitiveDataType> getPrimitiveDataType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> > from a string.
	 * @param xml the string to be converted into a vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> >
	 * @return a vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> >.
	 */
	static vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> > getPrimitiveDataType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> > >
	 * @return a vector<vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> > >.
	 */
	static vector<vector<vector<PrimitiveDataTypeMod::PrimitiveDataType> > > getPrimitiveDataType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  SchedulerModeMod::SchedulerMode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, SchedulerModeMod::SchedulerMode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<SchedulerModeMod::SchedulerMode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<SchedulerModeMod::SchedulerMode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<SchedulerModeMod::SchedulerMode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<SchedulerModeMod::SchedulerMode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<SchedulerModeMod::SchedulerMode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<SchedulerModeMod::SchedulerMode> > >& vvv_e); 

	/**
	 * Returns a SchedulerModeMod::SchedulerMode from a string.
	 * @param xml the string to be converted into a SchedulerModeMod::SchedulerMode
	 * @return a SchedulerModeMod::SchedulerMode.
	 */
	static SchedulerModeMod::SchedulerMode getSchedulerMode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<SchedulerModeMod::SchedulerMode> from a string.
	 * @param xml the string to be converted into a vector<SchedulerModeMod::SchedulerMode>
	 * @return a vector<SchedulerModeMod::SchedulerMode>.
	 */
	static vector<SchedulerModeMod::SchedulerMode> getSchedulerMode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<SchedulerModeMod::SchedulerMode> > from a string.
	 * @param xml the string to be converted into a vector<vector<SchedulerModeMod::SchedulerMode> >
	 * @return a vector<vector<SchedulerModeMod::SchedulerMode> >.
	 */
	static vector<vector<SchedulerModeMod::SchedulerMode> > getSchedulerMode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<SchedulerModeMod::SchedulerMode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<SchedulerModeMod::SchedulerMode> > >
	 * @return a vector<vector<vector<SchedulerModeMod::SchedulerMode> > >.
	 */
	static vector<vector<vector<SchedulerModeMod::SchedulerMode> > > getSchedulerMode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  FieldCodeMod::FieldCode value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, FieldCodeMod::FieldCode e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<FieldCodeMod::FieldCode>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<FieldCodeMod::FieldCode>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<FieldCodeMod::FieldCode> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<FieldCodeMod::FieldCode> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<FieldCodeMod::FieldCode> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<FieldCodeMod::FieldCode> > >& vvv_e); 

	/**
	 * Returns a FieldCodeMod::FieldCode from a string.
	 * @param xml the string to be converted into a FieldCodeMod::FieldCode
	 * @return a FieldCodeMod::FieldCode.
	 */
	static FieldCodeMod::FieldCode getFieldCode(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<FieldCodeMod::FieldCode> from a string.
	 * @param xml the string to be converted into a vector<FieldCodeMod::FieldCode>
	 * @return a vector<FieldCodeMod::FieldCode>.
	 */
	static vector<FieldCodeMod::FieldCode> getFieldCode1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<FieldCodeMod::FieldCode> > from a string.
	 * @param xml the string to be converted into a vector<vector<FieldCodeMod::FieldCode> >
	 * @return a vector<vector<FieldCodeMod::FieldCode> >.
	 */
	static vector<vector<FieldCodeMod::FieldCode> > getFieldCode2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<FieldCodeMod::FieldCode> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<FieldCodeMod::FieldCode> > >
	 * @return a vector<vector<vector<FieldCodeMod::FieldCode> > >.
	 */
	static vector<vector<vector<FieldCodeMod::FieldCode> > > getFieldCode3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  ProcessorSubTypeMod::ProcessorSubType value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, ProcessorSubTypeMod::ProcessorSubType e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<ProcessorSubTypeMod::ProcessorSubType>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<ProcessorSubTypeMod::ProcessorSubType>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<ProcessorSubTypeMod::ProcessorSubType> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<ProcessorSubTypeMod::ProcessorSubType> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<ProcessorSubTypeMod::ProcessorSubType> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<ProcessorSubTypeMod::ProcessorSubType> > >& vvv_e); 

	/**
	 * Returns a ProcessorSubTypeMod::ProcessorSubType from a string.
	 * @param xml the string to be converted into a ProcessorSubTypeMod::ProcessorSubType
	 * @return a ProcessorSubTypeMod::ProcessorSubType.
	 */
	static ProcessorSubTypeMod::ProcessorSubType getProcessorSubType(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<ProcessorSubTypeMod::ProcessorSubType> from a string.
	 * @param xml the string to be converted into a vector<ProcessorSubTypeMod::ProcessorSubType>
	 * @return a vector<ProcessorSubTypeMod::ProcessorSubType>.
	 */
	static vector<ProcessorSubTypeMod::ProcessorSubType> getProcessorSubType1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<ProcessorSubTypeMod::ProcessorSubType> > from a string.
	 * @param xml the string to be converted into a vector<vector<ProcessorSubTypeMod::ProcessorSubType> >
	 * @return a vector<vector<ProcessorSubTypeMod::ProcessorSubType> >.
	 */
	static vector<vector<ProcessorSubTypeMod::ProcessorSubType> > getProcessorSubType2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<ProcessorSubTypeMod::ProcessorSubType> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<ProcessorSubTypeMod::ProcessorSubType> > >
	 * @return a vector<vector<vector<ProcessorSubTypeMod::ProcessorSubType> > >.
	 */
	static vector<vector<vector<ProcessorSubTypeMod::ProcessorSubType> > > getProcessorSubType3D(const string &name, const string &tableName, const string &xmlDoc);								

	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and a content equal to 
	 * the string associated to 'e'
	 * @param elementName a string.
	 * @param e  ACAPolarizationMod::ACAPolarization value.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, ACAPolarizationMod::ACAPolarization e);
	 
	 
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'v_e' followed by sequence of strings associated to each element of 'v_e'
	 * @param elementName a string.
	 * @param v_e  a const reference to a vector<ACAPolarizationMod::ACAPolarization>.
	 * @return a string.
	 */
	 static string toXML(const string& elementName, const vector<ACAPolarizationMod::ACAPolarization>& v_e);
	
	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vv_e' followed by sequence of strings associated to each element of 'vv_e'.
	 * @param elementName a string.
	 * @param vv_e  a const reference to a vector<vector<ACAPolarizationMod::ACAPolarization> >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<ACAPolarizationMod::ACAPolarization> >& vv_e); 
	 


	/**
	 * Returns a string which represents a XML element 
	 * with name 'elementName' and of content equal to 
	 * the number of elements of 'vvv_e' followed by sequence of strings associated to each element of 'vvv_e'.
	 * @param elementName a string.
	 * @param vvv_e  a const reference to a vector<vector<vector<ACAPolarizationMod::ACAPolarization> > >.
	 * @return a string.
	 */	
	 static string toXML(const string& elementName, const vector<vector<vector<ACAPolarizationMod::ACAPolarization> > >& vvv_e); 

	/**
	 * Returns a ACAPolarizationMod::ACAPolarization from a string.
	 * @param xml the string to be converted into a ACAPolarizationMod::ACAPolarization
	 * @return a ACAPolarizationMod::ACAPolarization.
	 */
	static ACAPolarizationMod::ACAPolarization getACAPolarization(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<ACAPolarizationMod::ACAPolarization> from a string.
	 * @param xml the string to be converted into a vector<ACAPolarizationMod::ACAPolarization>
	 * @return a vector<ACAPolarizationMod::ACAPolarization>.
	 */
	static vector<ACAPolarizationMod::ACAPolarization> getACAPolarization1D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<ACAPolarizationMod::ACAPolarization> > from a string.
	 * @param xml the string to be converted into a vector<vector<ACAPolarizationMod::ACAPolarization> >
	 * @return a vector<vector<ACAPolarizationMod::ACAPolarization> >.
	 */
	static vector<vector<ACAPolarizationMod::ACAPolarization> > getACAPolarization2D(const string &name, const string &tableName, const string &xmlDoc);
	
	/**
	 * Returns a vector<vector<vector<ACAPolarizationMod::ACAPolarization> > > from a string.
	 * @param xml the string to be converted into a vector<vector<vector<ACAPolarizationMod::ACAPolarization> > >
	 * @return a vector<vector<vector<ACAPolarizationMod::ACAPolarization> > >.
	 */
	static vector<vector<vector<ACAPolarizationMod::ACAPolarization> > > getACAPolarization3D(const string &name, const string &tableName, const string &xmlDoc);								

	
	};
	
} // namespace asdm.

