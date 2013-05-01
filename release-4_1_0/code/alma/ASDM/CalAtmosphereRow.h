
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

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
#endif






	 
#include <ArrayTime.h>
	

	 
#include <Temperature.h>
	

	 
#include <Humidity.h>
	

	 
#include <Tag.h>
	

	 
#include <Length.h>
	

	 
#include <Frequency.h>
	

	 
#include <Pressure.h>
	




	
#include "CReceiverBand.h"
	

	

	
#include "CBasebandName.h"
	

	

	

	

	

	

	

	

	

	

	

	

	
#include "CPolarizationType.h"
	

	

	

	
#include "CSyscalMethod.h"
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>

#include <RowTransformer.h>
//#include <TableStreamReader.h>

/*\file CalAtmosphere.h
    \brief Generated from model's revision "1.64", branch "HEAD"
*/

namespace asdm {

//class asdm::CalAtmosphereTable;


// class asdm::CalReductionRow;
class CalReductionRow;

// class asdm::CalDataRow;
class CalDataRow;
	

class CalAtmosphereRow;
typedef void (CalAtmosphereRow::*CalAtmosphereAttributeFromBin) (EndianIStream& eis);
typedef void (CalAtmosphereRow::*CalAtmosphereAttributeFromText) (const string& s);

/**
 * The CalAtmosphereRow class is a row of a CalAtmosphereTable.
 * 
 * Generated from model's revision "1.64", branch "HEAD"
 *
 */
class CalAtmosphereRow {
friend class asdm::CalAtmosphereTable;
friend class asdm::RowTransformer<CalAtmosphereRow>;
//friend class asdm::TableStreamReader<CalAtmosphereTable, CalAtmosphereRow>;

public:

	virtual ~CalAtmosphereRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalAtmosphereTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute receiverBand
	
	
	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand getReceiverBand() const;
	
 
 	
 	
 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute basebandName
	
	
	

	
 	/**
 	 * Get basebandName.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 */
 	BasebandNameMod::BasebandName getBasebandName() const;
	
 
 	
 	
 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setBasebandName (BasebandNameMod::BasebandName basebandName);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numLoad
	
	
	

	
 	/**
 	 * Get numLoad.
 	 * @return numLoad as int
 	 */
 	int getNumLoad() const;
	
 
 	
 	
 	/**
 	 * Set numLoad with the specified int.
 	 * @param numLoad The int value to which numLoad is to be set.
 	 
 		
 			
 	 */
 	void setNumLoad (int numLoad);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute powerSkySpectrum
	
	
	

	
 	/**
 	 * Get powerSkySpectrum.
 	 * @return powerSkySpectrum as vector<vector<float > >
 	 */
 	vector<vector<float > > getPowerSkySpectrum() const;
	
 
 	
 	
 	/**
 	 * Set powerSkySpectrum with the specified vector<vector<float > >.
 	 * @param powerSkySpectrum The vector<vector<float > > value to which powerSkySpectrum is to be set.
 	 
 		
 			
 	 */
 	void setPowerSkySpectrum (vector<vector<float > > powerSkySpectrum);
  		
	
	
	


	
	// ===> Attribute powerLoadSpectrum
	
	
	

	
 	/**
 	 * Get powerLoadSpectrum.
 	 * @return powerLoadSpectrum as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > getPowerLoadSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set powerLoadSpectrum with the specified vector<vector<vector<float > > >.
 	 * @param powerLoadSpectrum The vector<vector<vector<float > > > value to which powerLoadSpectrum is to be set.
 	 
 		
 			
 	 */
 	void setPowerLoadSpectrum (vector<vector<vector<float > > > powerLoadSpectrum);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute tAtm
	
	
	

	
 	/**
 	 * Get tAtm.
 	 * @return tAtm as vector<Temperature >
 	 */
 	vector<Temperature > getTAtm() const;
	
 
 	
 	
 	/**
 	 * Set tAtm with the specified vector<Temperature >.
 	 * @param tAtm The vector<Temperature > value to which tAtm is to be set.
 	 
 		
 			
 	 */
 	void setTAtm (vector<Temperature > tAtm);
  		
	
	
	


	
	// ===> Attribute tRec
	
	
	

	
 	/**
 	 * Get tRec.
 	 * @return tRec as vector<Temperature >
 	 */
 	vector<Temperature > getTRec() const;
	
 
 	
 	
 	/**
 	 * Set tRec with the specified vector<Temperature >.
 	 * @param tRec The vector<Temperature > value to which tRec is to be set.
 	 
 		
 			
 	 */
 	void setTRec (vector<Temperature > tRec);
  		
	
	
	


	
	// ===> Attribute tSys
	
	
	

	
 	/**
 	 * Get tSys.
 	 * @return tSys as vector<Temperature >
 	 */
 	vector<Temperature > getTSys() const;
	
 
 	
 	
 	/**
 	 * Set tSys with the specified vector<Temperature >.
 	 * @param tSys The vector<Temperature > value to which tSys is to be set.
 	 
 		
 			
 	 */
 	void setTSys (vector<Temperature > tSys);
  		
	
	
	


	
	// ===> Attribute tau
	
	
	

	
 	/**
 	 * Get tau.
 	 * @return tau as vector<float >
 	 */
 	vector<float > getTau() const;
	
 
 	
 	
 	/**
 	 * Set tau with the specified vector<float >.
 	 * @param tau The vector<float > value to which tau is to be set.
 	 
 		
 			
 	 */
 	void setTau (vector<float > tau);
  		
	
	
	


	
	// ===> Attribute water
	
	
	

	
 	/**
 	 * Get water.
 	 * @return water as vector<Length >
 	 */
 	vector<Length > getWater() const;
	
 
 	
 	
 	/**
 	 * Set water with the specified vector<Length >.
 	 * @param water The vector<Length > value to which water is to be set.
 	 
 		
 			
 	 */
 	void setWater (vector<Length > water);
  		
	
	
	


	
	// ===> Attribute waterError
	
	
	

	
 	/**
 	 * Get waterError.
 	 * @return waterError as vector<Length >
 	 */
 	vector<Length > getWaterError() const;
	
 
 	
 	
 	/**
 	 * Set waterError with the specified vector<Length >.
 	 * @param waterError The vector<Length > value to which waterError is to be set.
 	 
 		
 			
 	 */
 	void setWaterError (vector<Length > waterError);
  		
	
	
	


	
	// ===> Attribute alphaSpectrum, which is optional
	
	
	
	/**
	 * The attribute alphaSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the alphaSpectrum attribute exists. 
	 */
	bool isAlphaSpectrumExists() const;
	

	
 	/**
 	 * Get alphaSpectrum, which is optional.
 	 * @return alphaSpectrum as vector<vector<float > >
 	 * @throws IllegalAccessException If alphaSpectrum does not exist.
 	 */
 	vector<vector<float > > getAlphaSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set alphaSpectrum with the specified vector<vector<float > >.
 	 * @param alphaSpectrum The vector<vector<float > > value to which alphaSpectrum is to be set.
 	 
 		
 	 */
 	void setAlphaSpectrum (vector<vector<float > > alphaSpectrum);
		
	
	
	
	/**
	 * Mark alphaSpectrum, which is an optional field, as non-existent.
	 */
	void clearAlphaSpectrum ();
	


	
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
 	vector<float > getForwardEfficiency() const;
	
 
 	
 	
 	/**
 	 * Set forwardEfficiency with the specified vector<float >.
 	 * @param forwardEfficiency The vector<float > value to which forwardEfficiency is to be set.
 	 
 		
 	 */
 	void setForwardEfficiency (vector<float > forwardEfficiency);
		
	
	
	
	/**
	 * Mark forwardEfficiency, which is an optional field, as non-existent.
	 */
	void clearForwardEfficiency ();
	


	
	// ===> Attribute forwardEfficiencyError, which is optional
	
	
	
	/**
	 * The attribute forwardEfficiencyError is optional. Return true if this attribute exists.
	 * @return true if and only if the forwardEfficiencyError attribute exists. 
	 */
	bool isForwardEfficiencyErrorExists() const;
	

	
 	/**
 	 * Get forwardEfficiencyError, which is optional.
 	 * @return forwardEfficiencyError as vector<double >
 	 * @throws IllegalAccessException If forwardEfficiencyError does not exist.
 	 */
 	vector<double > getForwardEfficiencyError() const;
	
 
 	
 	
 	/**
 	 * Set forwardEfficiencyError with the specified vector<double >.
 	 * @param forwardEfficiencyError The vector<double > value to which forwardEfficiencyError is to be set.
 	 
 		
 	 */
 	void setForwardEfficiencyError (vector<double > forwardEfficiencyError);
		
	
	
	
	/**
	 * Mark forwardEfficiencyError, which is an optional field, as non-existent.
	 */
	void clearForwardEfficiencyError ();
	


	
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
 	vector<float > getSbGain() const;
	
 
 	
 	
 	/**
 	 * Set sbGain with the specified vector<float >.
 	 * @param sbGain The vector<float > value to which sbGain is to be set.
 	 
 		
 	 */
 	void setSbGain (vector<float > sbGain);
		
	
	
	
	/**
	 * Mark sbGain, which is an optional field, as non-existent.
	 */
	void clearSbGain ();
	


	
	// ===> Attribute sbGainError, which is optional
	
	
	
	/**
	 * The attribute sbGainError is optional. Return true if this attribute exists.
	 * @return true if and only if the sbGainError attribute exists. 
	 */
	bool isSbGainErrorExists() const;
	

	
 	/**
 	 * Get sbGainError, which is optional.
 	 * @return sbGainError as vector<float >
 	 * @throws IllegalAccessException If sbGainError does not exist.
 	 */
 	vector<float > getSbGainError() const;
	
 
 	
 	
 	/**
 	 * Set sbGainError with the specified vector<float >.
 	 * @param sbGainError The vector<float > value to which sbGainError is to be set.
 	 
 		
 	 */
 	void setSbGainError (vector<float > sbGainError);
		
	
	
	
	/**
	 * Mark sbGainError, which is an optional field, as non-existent.
	 */
	void clearSbGainError ();
	


	
	// ===> Attribute sbGainSpectrum, which is optional
	
	
	
	/**
	 * The attribute sbGainSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the sbGainSpectrum attribute exists. 
	 */
	bool isSbGainSpectrumExists() const;
	

	
 	/**
 	 * Get sbGainSpectrum, which is optional.
 	 * @return sbGainSpectrum as vector<vector<float > >
 	 * @throws IllegalAccessException If sbGainSpectrum does not exist.
 	 */
 	vector<vector<float > > getSbGainSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set sbGainSpectrum with the specified vector<vector<float > >.
 	 * @param sbGainSpectrum The vector<vector<float > > value to which sbGainSpectrum is to be set.
 	 
 		
 	 */
 	void setSbGainSpectrum (vector<vector<float > > sbGainSpectrum);
		
	
	
	
	/**
	 * Mark sbGainSpectrum, which is an optional field, as non-existent.
	 */
	void clearSbGainSpectrum ();
	


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
	 * calReductionId pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* getCalReductionUsingCalReductionId();
	 

	

	

	
		
	/**
	 * calDataId pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* getCalDataUsingCalDataId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this CalAtmosphereRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaName
	    
	 * @param receiverBand
	    
	 * @param basebandName
	    
	 * @param calDataId
	    
	 * @param calReductionId
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param numFreq
	    
	 * @param numLoad
	    
	 * @param numReceptor
	    
	 * @param forwardEffSpectrum
	    
	 * @param frequencyRange
	    
	 * @param groundPressure
	    
	 * @param groundRelHumidity
	    
	 * @param frequencySpectrum
	    
	 * @param groundTemperature
	    
	 * @param polarizationTypes
	    
	 * @param powerSkySpectrum
	    
	 * @param powerLoadSpectrum
	    
	 * @param syscalType
	    
	 * @param tAtmSpectrum
	    
	 * @param tRecSpectrum
	    
	 * @param tSysSpectrum
	    
	 * @param tauSpectrum
	    
	 * @param tAtm
	    
	 * @param tRec
	    
	 * @param tSys
	    
	 * @param tau
	    
	 * @param water
	    
	 * @param waterError
	    
	 */ 
	bool compareNoAutoInc(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, BasebandNameMod::BasebandName basebandName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numFreq, int numLoad, int numReceptor, vector<vector<float > > forwardEffSpectrum, vector<Frequency > frequencyRange, Pressure groundPressure, Humidity groundRelHumidity, vector<Frequency > frequencySpectrum, Temperature groundTemperature, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<float > > powerSkySpectrum, vector<vector<vector<float > > > powerLoadSpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tAtmSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tSysSpectrum, vector<vector<float > > tauSpectrum, vector<Temperature > tAtm, vector<Temperature > tRec, vector<Temperature > tSys, vector<float > tau, vector<Length > water, vector<Length > waterError);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param numFreq
	    
	 * @param numLoad
	    
	 * @param numReceptor
	    
	 * @param forwardEffSpectrum
	    
	 * @param frequencyRange
	    
	 * @param groundPressure
	    
	 * @param groundRelHumidity
	    
	 * @param frequencySpectrum
	    
	 * @param groundTemperature
	    
	 * @param polarizationTypes
	    
	 * @param powerSkySpectrum
	    
	 * @param powerLoadSpectrum
	    
	 * @param syscalType
	    
	 * @param tAtmSpectrum
	    
	 * @param tRecSpectrum
	    
	 * @param tSysSpectrum
	    
	 * @param tauSpectrum
	    
	 * @param tAtm
	    
	 * @param tRec
	    
	 * @param tSys
	    
	 * @param tau
	    
	 * @param water
	    
	 * @param waterError
	    
	 */ 
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, int numFreq, int numLoad, int numReceptor, vector<vector<float > > forwardEffSpectrum, vector<Frequency > frequencyRange, Pressure groundPressure, Humidity groundRelHumidity, vector<Frequency > frequencySpectrum, Temperature groundTemperature, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<float > > powerSkySpectrum, vector<vector<vector<float > > > powerLoadSpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tAtmSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tSysSpectrum, vector<vector<float > > tauSpectrum, vector<Temperature > tAtm, vector<Temperature > tRec, vector<Temperature > tSys, vector<float > tau, vector<Length > water, vector<Length > waterError); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalAtmosphereRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalAtmosphereRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalAtmosphereRowIDL struct.
	 */
	asdmIDL::CalAtmosphereRowIDL *toIDL() const;
	
	/**
	 * Define the content of a CalAtmosphereRowIDL struct from the values
	 * found in this row.
	 *
	 * @param x a reference to the CalAtmosphereRowIDL struct to be set.
	 *
	 */
	 void toIDL(asdmIDL::CalAtmosphereRowIDL& x) const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalAtmosphereRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (asdmIDL::CalAtmosphereRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	std::string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param rowDoc the XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (std::string rowDoc) ;

	/// @cond DISPLAY_PRIVATE	
	////////////////////////////////////////////////////////////
	// binary-deserialization material from an EndianIStream  //
	////////////////////////////////////////////////////////////

	std::map<std::string, CalAtmosphereAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianIStream& eis);
void receiverBandFromBin( EndianIStream& eis);
void basebandNameFromBin( EndianIStream& eis);
void calDataIdFromBin( EndianIStream& eis);
void calReductionIdFromBin( EndianIStream& eis);
void startValidTimeFromBin( EndianIStream& eis);
void endValidTimeFromBin( EndianIStream& eis);
void numFreqFromBin( EndianIStream& eis);
void numLoadFromBin( EndianIStream& eis);
void numReceptorFromBin( EndianIStream& eis);
void forwardEffSpectrumFromBin( EndianIStream& eis);
void frequencyRangeFromBin( EndianIStream& eis);
void groundPressureFromBin( EndianIStream& eis);
void groundRelHumidityFromBin( EndianIStream& eis);
void frequencySpectrumFromBin( EndianIStream& eis);
void groundTemperatureFromBin( EndianIStream& eis);
void polarizationTypesFromBin( EndianIStream& eis);
void powerSkySpectrumFromBin( EndianIStream& eis);
void powerLoadSpectrumFromBin( EndianIStream& eis);
void syscalTypeFromBin( EndianIStream& eis);
void tAtmSpectrumFromBin( EndianIStream& eis);
void tRecSpectrumFromBin( EndianIStream& eis);
void tSysSpectrumFromBin( EndianIStream& eis);
void tauSpectrumFromBin( EndianIStream& eis);
void tAtmFromBin( EndianIStream& eis);
void tRecFromBin( EndianIStream& eis);
void tSysFromBin( EndianIStream& eis);
void tauFromBin( EndianIStream& eis);
void waterFromBin( EndianIStream& eis);
void waterErrorFromBin( EndianIStream& eis);

void alphaSpectrumFromBin( EndianIStream& eis);
void forwardEfficiencyFromBin( EndianIStream& eis);
void forwardEfficiencyErrorFromBin( EndianIStream& eis);
void sbGainFromBin( EndianIStream& eis);
void sbGainErrorFromBin( EndianIStream& eis);
void sbGainSpectrumFromBin( EndianIStream& eis);


	 /**
	  * Deserialize a stream of bytes read from an EndianIStream to build a PointingRow.
	  * @param eiss the EndianIStream to be read.
	  * @param table the CalAtmosphereTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalAtmosphereRow* fromBin(EndianIStream& eis, CalAtmosphereTable& table, const std::vector<std::string>& attributesSeq);	 
 
 	 /**
 	  * Parses a string t and assign the result of the parsing to the attribute of name attributeName.
 	  *
 	  * @param attributeName the name of the attribute whose value is going to be defined.
 	  * @param t the string to be parsed into a value given to the attribute of name attributeName.
 	  */
 	 void fromText(const std::string& attributeName, const std::string&  t);
     /// @endcond			

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
	void isAdded(bool added);


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
	
	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute basebandName
	
	

	BasebandNameMod::BasebandName basebandName;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute numFreq
	
	

	int numFreq;

	
	
 	

	
	// ===> Attribute numLoad
	
	

	int numLoad;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute forwardEffSpectrum
	
	

	vector<vector<float > > forwardEffSpectrum;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute groundPressure
	
	

	Pressure groundPressure;

	
	
 	

	
	// ===> Attribute groundRelHumidity
	
	

	Humidity groundRelHumidity;

	
	
 	

	
	// ===> Attribute frequencySpectrum
	
	

	vector<Frequency > frequencySpectrum;

	
	
 	

	
	// ===> Attribute groundTemperature
	
	

	Temperature groundTemperature;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute powerSkySpectrum
	
	

	vector<vector<float > > powerSkySpectrum;

	
	
 	

	
	// ===> Attribute powerLoadSpectrum
	
	

	vector<vector<vector<float > > > powerLoadSpectrum;

	
	
 	

	
	// ===> Attribute syscalType
	
	

	SyscalMethodMod::SyscalMethod syscalType;

	
	
 	

	
	// ===> Attribute tAtmSpectrum
	
	

	vector<vector<Temperature > > tAtmSpectrum;

	
	
 	

	
	// ===> Attribute tRecSpectrum
	
	

	vector<vector<Temperature > > tRecSpectrum;

	
	
 	

	
	// ===> Attribute tSysSpectrum
	
	

	vector<vector<Temperature > > tSysSpectrum;

	
	
 	

	
	// ===> Attribute tauSpectrum
	
	

	vector<vector<float > > tauSpectrum;

	
	
 	

	
	// ===> Attribute tAtm
	
	

	vector<Temperature > tAtm;

	
	
 	

	
	// ===> Attribute tRec
	
	

	vector<Temperature > tRec;

	
	
 	

	
	// ===> Attribute tSys
	
	

	vector<Temperature > tSys;

	
	
 	

	
	// ===> Attribute tau
	
	

	vector<float > tau;

	
	
 	

	
	// ===> Attribute water
	
	

	vector<Length > water;

	
	
 	

	
	// ===> Attribute waterError
	
	

	vector<Length > waterError;

	
	
 	

	
	// ===> Attribute alphaSpectrum, which is optional
	
	
	bool alphaSpectrumExists;
	

	vector<vector<float > > alphaSpectrum;

	
	
 	

	
	// ===> Attribute forwardEfficiency, which is optional
	
	
	bool forwardEfficiencyExists;
	

	vector<float > forwardEfficiency;

	
	
 	

	
	// ===> Attribute forwardEfficiencyError, which is optional
	
	
	bool forwardEfficiencyErrorExists;
	

	vector<double > forwardEfficiencyError;

	
	
 	

	
	// ===> Attribute sbGain, which is optional
	
	
	bool sbGainExists;
	

	vector<float > sbGain;

	
	
 	

	
	// ===> Attribute sbGainError, which is optional
	
	
	bool sbGainErrorExists;
	

	vector<float > sbGainError;

	
	
 	

	
	// ===> Attribute sbGainSpectrum, which is optional
	
	
	bool sbGainSpectrumExists;
	

	vector<vector<float > > sbGainSpectrum;

	
	
 	

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
	
	
		

	 

	

	
		

	 

	

	
/*
	////////////////////////////////////////////////////////////
	// binary-deserialization material from an EndianIStream  //
	////////////////////////////////////////////////////////////
	std::map<std::string, CalAtmosphereAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianIStream& eis);
void receiverBandFromBin( EndianIStream& eis);
void basebandNameFromBin( EndianIStream& eis);
void calDataIdFromBin( EndianIStream& eis);
void calReductionIdFromBin( EndianIStream& eis);
void startValidTimeFromBin( EndianIStream& eis);
void endValidTimeFromBin( EndianIStream& eis);
void numFreqFromBin( EndianIStream& eis);
void numLoadFromBin( EndianIStream& eis);
void numReceptorFromBin( EndianIStream& eis);
void forwardEffSpectrumFromBin( EndianIStream& eis);
void frequencyRangeFromBin( EndianIStream& eis);
void groundPressureFromBin( EndianIStream& eis);
void groundRelHumidityFromBin( EndianIStream& eis);
void frequencySpectrumFromBin( EndianIStream& eis);
void groundTemperatureFromBin( EndianIStream& eis);
void polarizationTypesFromBin( EndianIStream& eis);
void powerSkySpectrumFromBin( EndianIStream& eis);
void powerLoadSpectrumFromBin( EndianIStream& eis);
void syscalTypeFromBin( EndianIStream& eis);
void tAtmSpectrumFromBin( EndianIStream& eis);
void tRecSpectrumFromBin( EndianIStream& eis);
void tSysSpectrumFromBin( EndianIStream& eis);
void tauSpectrumFromBin( EndianIStream& eis);
void tAtmFromBin( EndianIStream& eis);
void tRecFromBin( EndianIStream& eis);
void tSysFromBin( EndianIStream& eis);
void tauFromBin( EndianIStream& eis);
void waterFromBin( EndianIStream& eis);
void waterErrorFromBin( EndianIStream& eis);

void alphaSpectrumFromBin( EndianIStream& eis);
void forwardEfficiencyFromBin( EndianIStream& eis);
void forwardEfficiencyErrorFromBin( EndianIStream& eis);
void sbGainFromBin( EndianIStream& eis);
void sbGainErrorFromBin( EndianIStream& eis);
void sbGainSpectrumFromBin( EndianIStream& eis);

*/
	
	///////////////////////////////////
	// text-deserialization material //
	///////////////////////////////////
	std::map<std::string, CalAtmosphereAttributeFromText> fromTextMethods;
	
void antennaNameFromText (const string & s);
	
	
void receiverBandFromText (const string & s);
	
	
void basebandNameFromText (const string & s);
	
	
void calDataIdFromText (const string & s);
	
	
void calReductionIdFromText (const string & s);
	
	
void startValidTimeFromText (const string & s);
	
	
void endValidTimeFromText (const string & s);
	
	
void numFreqFromText (const string & s);
	
	
void numLoadFromText (const string & s);
	
	
void numReceptorFromText (const string & s);
	
	
void forwardEffSpectrumFromText (const string & s);
	
	
void frequencyRangeFromText (const string & s);
	
	
void groundPressureFromText (const string & s);
	
	
void groundRelHumidityFromText (const string & s);
	
	
void frequencySpectrumFromText (const string & s);
	
	
void groundTemperatureFromText (const string & s);
	
	
void polarizationTypesFromText (const string & s);
	
	
void powerSkySpectrumFromText (const string & s);
	
	
void powerLoadSpectrumFromText (const string & s);
	
	
void syscalTypeFromText (const string & s);
	
	
void tAtmSpectrumFromText (const string & s);
	
	
void tRecSpectrumFromText (const string & s);
	
	
void tSysSpectrumFromText (const string & s);
	
	
void tauSpectrumFromText (const string & s);
	
	
void tAtmFromText (const string & s);
	
	
void tRecFromText (const string & s);
	
	
void tSysFromText (const string & s);
	
	
void tauFromText (const string & s);
	
	
void waterFromText (const string & s);
	
	
void waterErrorFromText (const string & s);
	

	
void alphaSpectrumFromText (const string & s);
	
	
void forwardEfficiencyFromText (const string & s);
	
	
void forwardEfficiencyErrorFromText (const string & s);
	
	
void sbGainFromText (const string & s);
	
	
void sbGainErrorFromText (const string & s);
	
	
void sbGainSpectrumFromText (const string & s);
	
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianIStream to build a PointingRow.
	  * @param eiss the EndianIStream to be read.
	  * @param table the CalAtmosphereTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.

	 static CalAtmosphereRow* fromBin(EndianIStream& eis, CalAtmosphereTable& table, const std::vector<std::string>& attributesSeq);	 
		*/
};

} // End namespace asdm

#endif /* CalAtmosphere_CLASS */
