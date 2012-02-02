
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
 * File CalAtmosphereRow.h
 */
 
#ifndef CalAtmosphereRow_CLASS
#define CalAtmosphereRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalAtmosphereRowIDL;
#endif

#include <Angle.h>
#include <AngularRate.h>
#include <ArrayTime.h>
#include <ArrayTimeInterval.h>
#include <Complex.h>
#include <Entity.h>
#include <EntityId.h>
#include <EntityRef.h>
#include <Flux.h>
#include <Frequency.h>
#include <Humidity.h>
#include <Interval.h>
#include <Length.h>
#include <Pressure.h>
#include <Speed.h>
#include <Tag.h>
#include <Temperature.h>
#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>

/*
#include <Enumerations.h>
using namespace enumerations;
 */




	

	

	

	

	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	
#include "CSyscalMethod.h"
using namespace SyscalMethodMod;
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	



using asdm::Angle;
using asdm::AngularRate;
using asdm::ArrayTime;
using asdm::Complex;
using asdm::Entity;
using asdm::EntityId;
using asdm::EntityRef;
using asdm::Flux;
using asdm::Frequency;
using asdm::Humidity;
using asdm::Interval;
using asdm::Length;
using asdm::Pressure;
using asdm::Speed;
using asdm::Tag;
using asdm::Temperature;
using asdm::ConversionException;
using asdm::NoSuchRow;
using asdm::IllegalAccessException;

/*\file CalAtmosphere.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::CalAtmosphereTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

/**
 * The CalAtmosphereRow class is a row of a CalAtmosphereTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class CalAtmosphereRow {
friend class asdm::CalAtmosphereTable;

public:

	virtual ~CalAtmosphereRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalAtmosphereTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalAtmosphereRowIDL struct.
	 */
	CalAtmosphereRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalAtmosphereRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (CalAtmosphereRowIDL x) throw(ConversionException);
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void setFromXML (string rowDoc) throw(ConversionException);
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string getAntennaName() const;
	
 
 	
 	
 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setAntennaName (string antennaName);
  		
	
	
	


	
	// ===> Attribute numReceptor
	
	
	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int getNumReceptor() const;
	
 
 	
 	
 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 		
 			
 	 */
 	void setNumReceptor (int numReceptor);
  		
	
	
	


	
	// ===> Attribute numFreq
	
	
	

	
 	/**
 	 * Get numFreq.
 	 * @return numFreq as int
 	 */
 	int getNumFreq() const;
	
 
 	
 	
 	/**
 	 * Set numFreq with the specified int.
 	 * @param numFreq The int value to which numFreq is to be set.
 	 
 		
 			
 	 */
 	void setNumFreq (int numFreq);
  		
	
	
	


	
	// ===> Attribute endValidTime
	
	
	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime getEndValidTime() const;
	
 
 	
 	
 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 		
 			
 	 */
 	void setEndValidTime (ArrayTime endValidTime);
  		
	
	
	


	
	// ===> Attribute startValidTime
	
	
	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime getStartValidTime() const;
	
 
 	
 	
 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 		
 			
 	 */
 	void setStartValidTime (ArrayTime startValidTime);
  		
	
	
	


	
	// ===> Attribute polarizationTypes
	
	
	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > getPolarizationTypes() const;
	
 
 	
 	
 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 		
 			
 	 */
 	void setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes);
  		
	
	
	


	
	// ===> Attribute frequencyRange
	
	
	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > getFrequencyRange() const;
	
 
 	
 	
 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyRange (vector<Frequency > frequencyRange);
  		
	
	
	


	
	// ===> Attribute frequencySpectrum
	
	
	

	
 	/**
 	 * Get frequencySpectrum.
 	 * @return frequencySpectrum as vector<Frequency >
 	 */
 	vector<Frequency > getFrequencySpectrum() const;
	
 
 	
 	
 	/**
 	 * Set frequencySpectrum with the specified vector<Frequency >.
 	 * @param frequencySpectrum The vector<Frequency > value to which frequencySpectrum is to be set.
 	 
 		
 			
 	 */
 	void setFrequencySpectrum (vector<Frequency > frequencySpectrum);
  		
	
	
	


	
	// ===> Attribute syscalType
	
	
	

	
 	/**
 	 * Get syscalType.
 	 * @return syscalType as SyscalMethodMod::SyscalMethod
 	 */
 	SyscalMethodMod::SyscalMethod getSyscalType() const;
	
 
 	
 	
 	/**
 	 * Set syscalType with the specified SyscalMethodMod::SyscalMethod.
 	 * @param syscalType The SyscalMethodMod::SyscalMethod value to which syscalType is to be set.
 	 
 		
 			
 	 */
 	void setSyscalType (SyscalMethodMod::SyscalMethod syscalType);
  		
	
	
	


	
	// ===> Attribute tSysSpectrum
	
	
	

	
 	/**
 	 * Get tSysSpectrum.
 	 * @return tSysSpectrum as vector<vector<Temperature > >
 	 */
 	vector<vector<Temperature > > getTSysSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tSysSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tSysSpectrum The vector<vector<Temperature > > value to which tSysSpectrum is to be set.
 	 
 		
 			
 	 */
 	void setTSysSpectrum (vector<vector<Temperature > > tSysSpectrum);
  		
	
	
	


	
	// ===> Attribute tRecSpectrum
	
	
	

	
 	/**
 	 * Get tRecSpectrum.
 	 * @return tRecSpectrum as vector<vector<Temperature > >
 	 */
 	vector<vector<Temperature > > getTRecSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tRecSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tRecSpectrum The vector<vector<Temperature > > value to which tRecSpectrum is to be set.
 	 
 		
 			
 	 */
 	void setTRecSpectrum (vector<vector<Temperature > > tRecSpectrum);
  		
	
	
	


	
	// ===> Attribute tAtmSpectrum
	
	
	

	
 	/**
 	 * Get tAtmSpectrum.
 	 * @return tAtmSpectrum as vector<vector<Temperature > >
 	 */
 	vector<vector<Temperature > > getTAtmSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tAtmSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tAtmSpectrum The vector<vector<Temperature > > value to which tAtmSpectrum is to be set.
 	 
 		
 			
 	 */
 	void setTAtmSpectrum (vector<vector<Temperature > > tAtmSpectrum);
  		
	
	
	


	
	// ===> Attribute tauSpectrum
	
	
	

	
 	/**
 	 * Get tauSpectrum.
 	 * @return tauSpectrum as vector<vector<float > >
 	 */
 	vector<vector<float > > getTauSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tauSpectrum with the specified vector<vector<float > >.
 	 * @param tauSpectrum The vector<vector<float > > value to which tauSpectrum is to be set.
 	 
 		
 			
 	 */
 	void setTauSpectrum (vector<vector<float > > tauSpectrum);
  		
	
	
	


	
	// ===> Attribute sbGainSpectrum
	
	
	

	
 	/**
 	 * Get sbGainSpectrum.
 	 * @return sbGainSpectrum as vector<vector<float > >
 	 */
 	vector<vector<float > > getSbGainSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set sbGainSpectrum with the specified vector<vector<float > >.
 	 * @param sbGainSpectrum The vector<vector<float > > value to which sbGainSpectrum is to be set.
 	 
 		
 			
 	 */
 	void setSbGainSpectrum (vector<vector<float > > sbGainSpectrum);
  		
	
	
	


	
	// ===> Attribute forwardEffSpectrum
	
	
	

	
 	/**
 	 * Get forwardEffSpectrum.
 	 * @return forwardEffSpectrum as vector<vector<float > >
 	 */
 	vector<vector<float > > getForwardEffSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set forwardEffSpectrum with the specified vector<vector<float > >.
 	 * @param forwardEffSpectrum The vector<vector<float > > value to which forwardEffSpectrum is to be set.
 	 
 		
 			
 	 */
 	void setForwardEffSpectrum (vector<vector<float > > forwardEffSpectrum);
  		
	
	
	


	
	// ===> Attribute tSys, which is optional
	
	
	
	/**
	 * The attribute tSys is optional. Return true if this attribute exists.
	 * @return true if and only if the tSys attribute exists. 
	 */
	bool isTSysExists() const;
	

	
 	/**
 	 * Get tSys, which is optional.
 	 * @return tSys as vector<Temperature >
 	 * @throws IllegalAccessException If tSys does not exist.
 	 */
 	vector<Temperature > getTSys() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tSys with the specified vector<Temperature >.
 	 * @param tSys The vector<Temperature > value to which tSys is to be set.
 	 
 		
 	 */
 	void setTSys (vector<Temperature > tSys);
		
	
	
	
	/**
	 * Mark tSys, which is an optional field, as non-existent.
	 */
	void clearTSys ();
	


	
	// ===> Attribute tRec, which is optional
	
	
	
	/**
	 * The attribute tRec is optional. Return true if this attribute exists.
	 * @return true if and only if the tRec attribute exists. 
	 */
	bool isTRecExists() const;
	

	
 	/**
 	 * Get tRec, which is optional.
 	 * @return tRec as vector<Temperature >
 	 * @throws IllegalAccessException If tRec does not exist.
 	 */
 	vector<Temperature > getTRec() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tRec with the specified vector<Temperature >.
 	 * @param tRec The vector<Temperature > value to which tRec is to be set.
 	 
 		
 	 */
 	void setTRec (vector<Temperature > tRec);
		
	
	
	
	/**
	 * Mark tRec, which is an optional field, as non-existent.
	 */
	void clearTRec ();
	


	
	// ===> Attribute tAtm, which is optional
	
	
	
	/**
	 * The attribute tAtm is optional. Return true if this attribute exists.
	 * @return true if and only if the tAtm attribute exists. 
	 */
	bool isTAtmExists() const;
	

	
 	/**
 	 * Get tAtm, which is optional.
 	 * @return tAtm as vector<Temperature >
 	 * @throws IllegalAccessException If tAtm does not exist.
 	 */
 	vector<Temperature > getTAtm() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tAtm with the specified vector<Temperature >.
 	 * @param tAtm The vector<Temperature > value to which tAtm is to be set.
 	 
 		
 	 */
 	void setTAtm (vector<Temperature > tAtm);
		
	
	
	
	/**
	 * Mark tAtm, which is an optional field, as non-existent.
	 */
	void clearTAtm ();
	


	
	// ===> Attribute sbGain, which is optional
	
	
	
	/**
	 * The attribute sbGain is optional. Return true if this attribute exists.
	 * @return true if and only if the sbGain attribute exists. 
	 */
	bool isSbGainExists() const;
	

	
 	/**
 	 * Get sbGain, which is optional.
 	 * @return sbGain as vector<float >
 	 * @throws IllegalAccessException If sbGain does not exist.
 	 */
 	vector<float > getSbGain() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sbGain with the specified vector<float >.
 	 * @param sbGain The vector<float > value to which sbGain is to be set.
 	 
 		
 	 */
 	void setSbGain (vector<float > sbGain);
		
	
	
	
	/**
	 * Mark sbGain, which is an optional field, as non-existent.
	 */
	void clearSbGain ();
	


	
	// ===> Attribute water, which is optional
	
	
	
	/**
	 * The attribute water is optional. Return true if this attribute exists.
	 * @return true if and only if the water attribute exists. 
	 */
	bool isWaterExists() const;
	

	
 	/**
 	 * Get water, which is optional.
 	 * @return water as vector<Length >
 	 * @throws IllegalAccessException If water does not exist.
 	 */
 	vector<Length > getWater() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set water with the specified vector<Length >.
 	 * @param water The vector<Length > value to which water is to be set.
 	 
 		
 	 */
 	void setWater (vector<Length > water);
		
	
	
	
	/**
	 * Mark water, which is an optional field, as non-existent.
	 */
	void clearWater ();
	


	
	// ===> Attribute forwardEfficiency, which is optional
	
	
	
	/**
	 * The attribute forwardEfficiency is optional. Return true if this attribute exists.
	 * @return true if and only if the forwardEfficiency attribute exists. 
	 */
	bool isForwardEfficiencyExists() const;
	

	
 	/**
 	 * Get forwardEfficiency, which is optional.
 	 * @return forwardEfficiency as vector<float >
 	 * @throws IllegalAccessException If forwardEfficiency does not exist.
 	 */
 	vector<float > getForwardEfficiency() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set forwardEfficiency with the specified vector<float >.
 	 * @param forwardEfficiency The vector<float > value to which forwardEfficiency is to be set.
 	 
 		
 	 */
 	void setForwardEfficiency (vector<float > forwardEfficiency);
		
	
	
	
	/**
	 * Mark forwardEfficiency, which is an optional field, as non-existent.
	 */
	void clearForwardEfficiency ();
	


	
	// ===> Attribute tau, which is optional
	
	
	
	/**
	 * The attribute tau is optional. Return true if this attribute exists.
	 * @return true if and only if the tau attribute exists. 
	 */
	bool isTauExists() const;
	

	
 	/**
 	 * Get tau, which is optional.
 	 * @return tau as vector<float >
 	 * @throws IllegalAccessException If tau does not exist.
 	 */
 	vector<float > getTau() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tau with the specified vector<float >.
 	 * @param tau The vector<float > value to which tau is to be set.
 	 
 		
 	 */
 	void setTau (vector<float > tau);
		
	
	
	
	/**
	 * Mark tau, which is an optional field, as non-existent.
	 */
	void clearTau ();
	


	
	// ===> Attribute groundPressure
	
	
	

	
 	/**
 	 * Get groundPressure.
 	 * @return groundPressure as Pressure
 	 */
 	Pressure getGroundPressure() const;
	
 
 	
 	
 	/**
 	 * Set groundPressure with the specified Pressure.
 	 * @param groundPressure The Pressure value to which groundPressure is to be set.
 	 
 		
 			
 	 */
 	void setGroundPressure (Pressure groundPressure);
  		
	
	
	


	
	// ===> Attribute groundTemperature
	
	
	

	
 	/**
 	 * Get groundTemperature.
 	 * @return groundTemperature as Temperature
 	 */
 	Temperature getGroundTemperature() const;
	
 
 	
 	
 	/**
 	 * Set groundTemperature with the specified Temperature.
 	 * @param groundTemperature The Temperature value to which groundTemperature is to be set.
 	 
 		
 			
 	 */
 	void setGroundTemperature (Temperature groundTemperature);
  		
	
	
	


	
	// ===> Attribute groundRelHumidity
	
	
	

	
 	/**
 	 * Get groundRelHumidity.
 	 * @return groundRelHumidity as Humidity
 	 */
 	Humidity getGroundRelHumidity() const;
	
 
 	
 	
 	/**
 	 * Set groundRelHumidity with the specified Humidity.
 	 * @param groundRelHumidity The Humidity value to which groundRelHumidity is to be set.
 	 
 		
 			
 	 */
 	void setGroundRelHumidity (Humidity groundRelHumidity);
  		
	
	
	


	
	// ===> Attribute subType
	
	
	

	
 	/**
 	 * Get subType.
 	 * @return subType as string
 	 */
 	string getSubType() const;
	
 
 	
 	
 	/**
 	 * Set subType with the specified string.
 	 * @param subType The string value to which subType is to be set.
 	 
 		
 			
 	 */
 	void setSubType (string subType);
  		
	
	
	


	
	// ===> Attribute receiverBand
	
	
	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand getReceiverBand() const;
	
 
 	
 	
 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 		
 			
 	 */
 	void setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute calDataId
	
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag getCalDataId() const;
	
 
 	
 	
 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setCalDataId (Tag calDataId);
  		
	
	
	


	
	// ===> Attribute calReductionId
	
	
	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag getCalReductionId() const;
	
 
 	
 	
 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setCalReductionId (Tag calReductionId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * calDataId pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* getCalDataUsingCalDataId();
	 

	

	

	
		
	/**
	 * calReductionId pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* getCalReductionUsingCalReductionId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this CalAtmosphereRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, int numReceptor, int numFreq, ArrayTime endValidTime, ArrayTime startValidTime, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<Frequency > frequencyRange, vector<Frequency > frequencySpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tSysSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tAtmSpectrum, vector<vector<float > > tauSpectrum, vector<vector<float > > sbGainSpectrum, vector<vector<float > > forwardEffSpectrum, Pressure groundPressure, Temperature groundTemperature, Humidity groundRelHumidity, string subType, ReceiverBandMod::ReceiverBand receiverBand);
	
	

	
	bool compareRequiredValue(int numReceptor, int numFreq, ArrayTime endValidTime, ArrayTime startValidTime, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<Frequency > frequencyRange, vector<Frequency > frequencySpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tSysSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tAtmSpectrum, vector<vector<float > > tauSpectrum, vector<vector<float > > sbGainSpectrum, vector<vector<float > > forwardEffSpectrum, Pressure groundPressure, Temperature groundTemperature, Humidity groundRelHumidity, string subType, ReceiverBandMod::ReceiverBand receiverBand); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalAtmosphereRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalAtmosphereRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	CalAtmosphereTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalAtmosphereRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalAtmosphereRow (CalAtmosphereTable &table);

	/**
	 * Create a CalAtmosphereRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalAtmosphereRow row and a CalAtmosphereTable table, the method creates a new
	 * CalAtmosphereRow owned by table. Each attribute of the created row is a copy (deep)
	 * of the corresponding attribute of row. The method does not add the created
	 * row to its table, its simply parents it to table, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a row with default values for its attributes. 
	 *
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 * @param row  The row which is to be copied.
	 */
	 CalAtmosphereRow (CalAtmosphereTable &table, CalAtmosphereRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute numFreq
	
	

	int numFreq;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute frequencySpectrum
	
	

	vector<Frequency > frequencySpectrum;

	
	
 	

	
	// ===> Attribute syscalType
	
	

	SyscalMethodMod::SyscalMethod syscalType;

	
	
 	

	
	// ===> Attribute tSysSpectrum
	
	

	vector<vector<Temperature > > tSysSpectrum;

	
	
 	

	
	// ===> Attribute tRecSpectrum
	
	

	vector<vector<Temperature > > tRecSpectrum;

	
	
 	

	
	// ===> Attribute tAtmSpectrum
	
	

	vector<vector<Temperature > > tAtmSpectrum;

	
	
 	

	
	// ===> Attribute tauSpectrum
	
	

	vector<vector<float > > tauSpectrum;

	
	
 	

	
	// ===> Attribute sbGainSpectrum
	
	

	vector<vector<float > > sbGainSpectrum;

	
	
 	

	
	// ===> Attribute forwardEffSpectrum
	
	

	vector<vector<float > > forwardEffSpectrum;

	
	
 	

	
	// ===> Attribute tSys, which is optional
	
	
	bool tSysExists;
	

	vector<Temperature > tSys;

	
	
 	

	
	// ===> Attribute tRec, which is optional
	
	
	bool tRecExists;
	

	vector<Temperature > tRec;

	
	
 	

	
	// ===> Attribute tAtm, which is optional
	
	
	bool tAtmExists;
	

	vector<Temperature > tAtm;

	
	
 	

	
	// ===> Attribute sbGain, which is optional
	
	
	bool sbGainExists;
	

	vector<float > sbGain;

	
	
 	

	
	// ===> Attribute water, which is optional
	
	
	bool waterExists;
	

	vector<Length > water;

	
	
 	

	
	// ===> Attribute forwardEfficiency, which is optional
	
	
	bool forwardEfficiencyExists;
	

	vector<float > forwardEfficiency;

	
	
 	

	
	// ===> Attribute tau, which is optional
	
	
	bool tauExists;
	

	vector<float > tau;

	
	
 	

	
	// ===> Attribute groundPressure
	
	

	Pressure groundPressure;

	
	
 	

	
	// ===> Attribute groundTemperature
	
	

	Temperature groundTemperature;

	
	
 	

	
	// ===> Attribute groundRelHumidity
	
	

	Humidity groundRelHumidity;

	
	
 	

	
	// ===> Attribute subType
	
	

	string subType;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute calDataId
	
	

	Tag calDataId;

	
	
 	

	
	// ===> Attribute calReductionId
	
	

	Tag calReductionId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	


};

} // End namespace asdm

#endif /* CalAtmosphere_CLASS */
