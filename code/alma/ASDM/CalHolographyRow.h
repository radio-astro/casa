
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
 * File CalHolographyRow.h
 */
 
#ifndef CalHolographyRow_CLASS
#define CalHolographyRow_CLASS

#include <vector>
#include <string>
#include <set>

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
#endif






	 
#include <ArrayTime.h>
	

	 
#include <Temperature.h>
	

	 
#include <Angle.h>
	

	 
#include <Tag.h>
	

	 
#include <Length.h>
	

	 
#include <Frequency.h>
	

	 
#include <EntityRef.h>
	




	

	
#include "CAntennaMake.h"
	

	

	

	

	

	

	

	

	
#include "CPolarizationType.h"
	

	

	
#include "CReceiverBand.h"
	

	

	

	

	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>

#include <RowTransformer.h>
//#include <TableStreamReader.h>

/*\file CalHolography.h
    \brief Generated from model's revision "1.64", branch "HEAD"
*/

namespace asdm {

//class asdm::CalHolographyTable;


// class asdm::CalReductionRow;
class CalReductionRow;

// class asdm::CalDataRow;
class CalDataRow;
	

class CalHolographyRow;
typedef void (CalHolographyRow::*CalHolographyAttributeFromBin) (EndianIStream& eis);
typedef void (CalHolographyRow::*CalHolographyAttributeFromText) (const string& s);

/**
 * The CalHolographyRow class is a row of a CalHolographyTable.
 * 
 * Generated from model's revision "1.64", branch "HEAD"
 *
 */
class CalHolographyRow {
friend class asdm::CalHolographyTable;
friend class asdm::RowTransformer<CalHolographyRow>;
//friend class asdm::TableStreamReader<CalHolographyTable, CalHolographyRow>;

public:

	virtual ~CalHolographyRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalHolographyTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
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
  		
	
	
	


	
	// ===> Attribute antennaMake
	
	
	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake getAntennaMake() const;
	
 
 	
 	
 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 		
 			
 	 */
 	void setAntennaMake (AntennaMakeMod::AntennaMake antennaMake);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute ambientTemperature
	
	
	

	
 	/**
 	 * Get ambientTemperature.
 	 * @return ambientTemperature as Temperature
 	 */
 	Temperature getAmbientTemperature() const;
	
 
 	
 	
 	/**
 	 * Set ambientTemperature with the specified Temperature.
 	 * @param ambientTemperature The Temperature value to which ambientTemperature is to be set.
 	 
 		
 			
 	 */
 	void setAmbientTemperature (Temperature ambientTemperature);
  		
	
	
	


	
	// ===> Attribute focusPosition
	
	
	

	
 	/**
 	 * Get focusPosition.
 	 * @return focusPosition as vector<Length >
 	 */
 	vector<Length > getFocusPosition() const;
	
 
 	
 	
 	/**
 	 * Set focusPosition with the specified vector<Length >.
 	 * @param focusPosition The vector<Length > value to which focusPosition is to be set.
 	 
 		
 			
 	 */
 	void setFocusPosition (vector<Length > focusPosition);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute illuminationTaper
	
	
	

	
 	/**
 	 * Get illuminationTaper.
 	 * @return illuminationTaper as double
 	 */
 	double getIlluminationTaper() const;
	
 
 	
 	
 	/**
 	 * Set illuminationTaper with the specified double.
 	 * @param illuminationTaper The double value to which illuminationTaper is to be set.
 	 
 		
 			
 	 */
 	void setIlluminationTaper (double illuminationTaper);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numPanelModes
	
	
	

	
 	/**
 	 * Get numPanelModes.
 	 * @return numPanelModes as int
 	 */
 	int getNumPanelModes() const;
	
 
 	
 	
 	/**
 	 * Set numPanelModes with the specified int.
 	 * @param numPanelModes The int value to which numPanelModes is to be set.
 	 
 		
 			
 	 */
 	void setNumPanelModes (int numPanelModes);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute beamMapUID
	
	
	

	
 	/**
 	 * Get beamMapUID.
 	 * @return beamMapUID as EntityRef
 	 */
 	EntityRef getBeamMapUID() const;
	
 
 	
 	
 	/**
 	 * Set beamMapUID with the specified EntityRef.
 	 * @param beamMapUID The EntityRef value to which beamMapUID is to be set.
 	 
 		
 			
 	 */
 	void setBeamMapUID (EntityRef beamMapUID);
  		
	
	
	


	
	// ===> Attribute rawRMS
	
	
	

	
 	/**
 	 * Get rawRMS.
 	 * @return rawRMS as Length
 	 */
 	Length getRawRMS() const;
	
 
 	
 	
 	/**
 	 * Set rawRMS with the specified Length.
 	 * @param rawRMS The Length value to which rawRMS is to be set.
 	 
 		
 			
 	 */
 	void setRawRMS (Length rawRMS);
  		
	
	
	


	
	// ===> Attribute weightedRMS
	
	
	

	
 	/**
 	 * Get weightedRMS.
 	 * @return weightedRMS as Length
 	 */
 	Length getWeightedRMS() const;
	
 
 	
 	
 	/**
 	 * Set weightedRMS with the specified Length.
 	 * @param weightedRMS The Length value to which weightedRMS is to be set.
 	 
 		
 			
 	 */
 	void setWeightedRMS (Length weightedRMS);
  		
	
	
	


	
	// ===> Attribute surfaceMapUID
	
	
	

	
 	/**
 	 * Get surfaceMapUID.
 	 * @return surfaceMapUID as EntityRef
 	 */
 	EntityRef getSurfaceMapUID() const;
	
 
 	
 	
 	/**
 	 * Set surfaceMapUID with the specified EntityRef.
 	 * @param surfaceMapUID The EntityRef value to which surfaceMapUID is to be set.
 	 
 		
 			
 	 */
 	void setSurfaceMapUID (EntityRef surfaceMapUID);
  		
	
	
	


	
	// ===> Attribute direction
	
	
	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > getDirection() const;
	
 
 	
 	
 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 		
 			
 	 */
 	void setDirection (vector<Angle > direction);
  		
	
	
	


	
	// ===> Attribute numScrew, which is optional
	
	
	
	/**
	 * The attribute numScrew is optional. Return true if this attribute exists.
	 * @return true if and only if the numScrew attribute exists. 
	 */
	bool isNumScrewExists() const;
	

	
 	/**
 	 * Get numScrew, which is optional.
 	 * @return numScrew as int
 	 * @throws IllegalAccessException If numScrew does not exist.
 	 */
 	int getNumScrew() const;
	
 
 	
 	
 	/**
 	 * Set numScrew with the specified int.
 	 * @param numScrew The int value to which numScrew is to be set.
 	 
 		
 	 */
 	void setNumScrew (int numScrew);
		
	
	
	
	/**
	 * Mark numScrew, which is an optional field, as non-existent.
	 */
	void clearNumScrew ();
	


	
	// ===> Attribute screwName, which is optional
	
	
	
	/**
	 * The attribute screwName is optional. Return true if this attribute exists.
	 * @return true if and only if the screwName attribute exists. 
	 */
	bool isScrewNameExists() const;
	

	
 	/**
 	 * Get screwName, which is optional.
 	 * @return screwName as vector<string >
 	 * @throws IllegalAccessException If screwName does not exist.
 	 */
 	vector<string > getScrewName() const;
	
 
 	
 	
 	/**
 	 * Set screwName with the specified vector<string >.
 	 * @param screwName The vector<string > value to which screwName is to be set.
 	 
 		
 	 */
 	void setScrewName (vector<string > screwName);
		
	
	
	
	/**
	 * Mark screwName, which is an optional field, as non-existent.
	 */
	void clearScrewName ();
	


	
	// ===> Attribute screwMotion, which is optional
	
	
	
	/**
	 * The attribute screwMotion is optional. Return true if this attribute exists.
	 * @return true if and only if the screwMotion attribute exists. 
	 */
	bool isScrewMotionExists() const;
	

	
 	/**
 	 * Get screwMotion, which is optional.
 	 * @return screwMotion as vector<Length >
 	 * @throws IllegalAccessException If screwMotion does not exist.
 	 */
 	vector<Length > getScrewMotion() const;
	
 
 	
 	
 	/**
 	 * Set screwMotion with the specified vector<Length >.
 	 * @param screwMotion The vector<Length > value to which screwMotion is to be set.
 	 
 		
 	 */
 	void setScrewMotion (vector<Length > screwMotion);
		
	
	
	
	/**
	 * Mark screwMotion, which is an optional field, as non-existent.
	 */
	void clearScrewMotion ();
	


	
	// ===> Attribute screwMotionError, which is optional
	
	
	
	/**
	 * The attribute screwMotionError is optional. Return true if this attribute exists.
	 * @return true if and only if the screwMotionError attribute exists. 
	 */
	bool isScrewMotionErrorExists() const;
	

	
 	/**
 	 * Get screwMotionError, which is optional.
 	 * @return screwMotionError as vector<Length >
 	 * @throws IllegalAccessException If screwMotionError does not exist.
 	 */
 	vector<Length > getScrewMotionError() const;
	
 
 	
 	
 	/**
 	 * Set screwMotionError with the specified vector<Length >.
 	 * @param screwMotionError The vector<Length > value to which screwMotionError is to be set.
 	 
 		
 	 */
 	void setScrewMotionError (vector<Length > screwMotionError);
		
	
	
	
	/**
	 * Mark screwMotionError, which is an optional field, as non-existent.
	 */
	void clearScrewMotionError ();
	


	
	// ===> Attribute gravCorrection, which is optional
	
	
	
	/**
	 * The attribute gravCorrection is optional. Return true if this attribute exists.
	 * @return true if and only if the gravCorrection attribute exists. 
	 */
	bool isGravCorrectionExists() const;
	

	
 	/**
 	 * Get gravCorrection, which is optional.
 	 * @return gravCorrection as bool
 	 * @throws IllegalAccessException If gravCorrection does not exist.
 	 */
 	bool getGravCorrection() const;
	
 
 	
 	
 	/**
 	 * Set gravCorrection with the specified bool.
 	 * @param gravCorrection The bool value to which gravCorrection is to be set.
 	 
 		
 	 */
 	void setGravCorrection (bool gravCorrection);
		
	
	
	
	/**
	 * Mark gravCorrection, which is an optional field, as non-existent.
	 */
	void clearGravCorrection ();
	


	
	// ===> Attribute gravOptRange, which is optional
	
	
	
	/**
	 * The attribute gravOptRange is optional. Return true if this attribute exists.
	 * @return true if and only if the gravOptRange attribute exists. 
	 */
	bool isGravOptRangeExists() const;
	

	
 	/**
 	 * Get gravOptRange, which is optional.
 	 * @return gravOptRange as vector<Angle >
 	 * @throws IllegalAccessException If gravOptRange does not exist.
 	 */
 	vector<Angle > getGravOptRange() const;
	
 
 	
 	
 	/**
 	 * Set gravOptRange with the specified vector<Angle >.
 	 * @param gravOptRange The vector<Angle > value to which gravOptRange is to be set.
 	 
 		
 	 */
 	void setGravOptRange (vector<Angle > gravOptRange);
		
	
	
	
	/**
	 * Mark gravOptRange, which is an optional field, as non-existent.
	 */
	void clearGravOptRange ();
	


	
	// ===> Attribute tempCorrection, which is optional
	
	
	
	/**
	 * The attribute tempCorrection is optional. Return true if this attribute exists.
	 * @return true if and only if the tempCorrection attribute exists. 
	 */
	bool isTempCorrectionExists() const;
	

	
 	/**
 	 * Get tempCorrection, which is optional.
 	 * @return tempCorrection as bool
 	 * @throws IllegalAccessException If tempCorrection does not exist.
 	 */
 	bool getTempCorrection() const;
	
 
 	
 	
 	/**
 	 * Set tempCorrection with the specified bool.
 	 * @param tempCorrection The bool value to which tempCorrection is to be set.
 	 
 		
 	 */
 	void setTempCorrection (bool tempCorrection);
		
	
	
	
	/**
	 * Mark tempCorrection, which is an optional field, as non-existent.
	 */
	void clearTempCorrection ();
	


	
	// ===> Attribute tempOptRange, which is optional
	
	
	
	/**
	 * The attribute tempOptRange is optional. Return true if this attribute exists.
	 * @return true if and only if the tempOptRange attribute exists. 
	 */
	bool isTempOptRangeExists() const;
	

	
 	/**
 	 * Get tempOptRange, which is optional.
 	 * @return tempOptRange as vector<Temperature >
 	 * @throws IllegalAccessException If tempOptRange does not exist.
 	 */
 	vector<Temperature > getTempOptRange() const;
	
 
 	
 	
 	/**
 	 * Set tempOptRange with the specified vector<Temperature >.
 	 * @param tempOptRange The vector<Temperature > value to which tempOptRange is to be set.
 	 
 		
 	 */
 	void setTempOptRange (vector<Temperature > tempOptRange);
		
	
	
	
	/**
	 * Mark tempOptRange, which is an optional field, as non-existent.
	 */
	void clearTempOptRange ();
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this CalHolographyRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaName
	    
	 * @param calDataId
	    
	 * @param calReductionId
	    
	 * @param antennaMake
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param ambientTemperature
	    
	 * @param focusPosition
	    
	 * @param frequencyRange
	    
	 * @param illuminationTaper
	    
	 * @param numReceptor
	    
	 * @param polarizationTypes
	    
	 * @param numPanelModes
	    
	 * @param receiverBand
	    
	 * @param beamMapUID
	    
	 * @param rawRMS
	    
	 * @param weightedRMS
	    
	 * @param surfaceMapUID
	    
	 * @param direction
	    
	 */ 
	bool compareNoAutoInc(string antennaName, Tag calDataId, Tag calReductionId, AntennaMakeMod::AntennaMake antennaMake, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, vector<Length > focusPosition, vector<Frequency > frequencyRange, double illuminationTaper, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, int numPanelModes, ReceiverBandMod::ReceiverBand receiverBand, EntityRef beamMapUID, Length rawRMS, Length weightedRMS, EntityRef surfaceMapUID, vector<Angle > direction);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaMake
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param ambientTemperature
	    
	 * @param focusPosition
	    
	 * @param frequencyRange
	    
	 * @param illuminationTaper
	    
	 * @param numReceptor
	    
	 * @param polarizationTypes
	    
	 * @param numPanelModes
	    
	 * @param receiverBand
	    
	 * @param beamMapUID
	    
	 * @param rawRMS
	    
	 * @param weightedRMS
	    
	 * @param surfaceMapUID
	    
	 * @param direction
	    
	 */ 
	bool compareRequiredValue(AntennaMakeMod::AntennaMake antennaMake, ArrayTime startValidTime, ArrayTime endValidTime, Temperature ambientTemperature, vector<Length > focusPosition, vector<Frequency > frequencyRange, double illuminationTaper, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, int numPanelModes, ReceiverBandMod::ReceiverBand receiverBand, EntityRef beamMapUID, Length rawRMS, Length weightedRMS, EntityRef surfaceMapUID, vector<Angle > direction); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalHolographyRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalHolographyRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalHolographyRowIDL struct.
	 */
	asdmIDL::CalHolographyRowIDL *toIDL() const;
	
	/**
	 * Define the content of a CalHolographyRowIDL struct from the values
	 * found in this row.
	 *
	 * @param x a reference to the CalHolographyRowIDL struct to be set.
	 *
	 */
	 void toIDL(asdmIDL::CalHolographyRowIDL& x) const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalHolographyRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (asdmIDL::CalHolographyRowIDL x) ;
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

	std::map<std::string, CalHolographyAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianIStream& eis);
void calDataIdFromBin( EndianIStream& eis);
void calReductionIdFromBin( EndianIStream& eis);
void antennaMakeFromBin( EndianIStream& eis);
void startValidTimeFromBin( EndianIStream& eis);
void endValidTimeFromBin( EndianIStream& eis);
void ambientTemperatureFromBin( EndianIStream& eis);
void focusPositionFromBin( EndianIStream& eis);
void frequencyRangeFromBin( EndianIStream& eis);
void illuminationTaperFromBin( EndianIStream& eis);
void numReceptorFromBin( EndianIStream& eis);
void polarizationTypesFromBin( EndianIStream& eis);
void numPanelModesFromBin( EndianIStream& eis);
void receiverBandFromBin( EndianIStream& eis);
void beamMapUIDFromBin( EndianIStream& eis);
void rawRMSFromBin( EndianIStream& eis);
void weightedRMSFromBin( EndianIStream& eis);
void surfaceMapUIDFromBin( EndianIStream& eis);
void directionFromBin( EndianIStream& eis);

void numScrewFromBin( EndianIStream& eis);
void screwNameFromBin( EndianIStream& eis);
void screwMotionFromBin( EndianIStream& eis);
void screwMotionErrorFromBin( EndianIStream& eis);
void gravCorrectionFromBin( EndianIStream& eis);
void gravOptRangeFromBin( EndianIStream& eis);
void tempCorrectionFromBin( EndianIStream& eis);
void tempOptRangeFromBin( EndianIStream& eis);


	 /**
	  * Deserialize a stream of bytes read from an EndianIStream to build a PointingRow.
	  * @param eiss the EndianIStream to be read.
	  * @param table the CalHolographyTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalHolographyRow* fromBin(EndianIStream& eis, CalHolographyTable& table, const std::vector<std::string>& attributesSeq);	 
 
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
	CalHolographyTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a CalHolographyRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalHolographyRow (CalHolographyTable &table);

	/**
	 * Create a CalHolographyRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalHolographyRow row and a CalHolographyTable table, the method creates a new
	 * CalHolographyRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalHolographyRow (CalHolographyTable &table, CalHolographyRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute antennaMake
	
	

	AntennaMakeMod::AntennaMake antennaMake;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute ambientTemperature
	
	

	Temperature ambientTemperature;

	
	
 	

	
	// ===> Attribute focusPosition
	
	

	vector<Length > focusPosition;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute illuminationTaper
	
	

	double illuminationTaper;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute numPanelModes
	
	

	int numPanelModes;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute beamMapUID
	
	

	EntityRef beamMapUID;

	
	
 	

	
	// ===> Attribute rawRMS
	
	

	Length rawRMS;

	
	
 	

	
	// ===> Attribute weightedRMS
	
	

	Length weightedRMS;

	
	
 	

	
	// ===> Attribute surfaceMapUID
	
	

	EntityRef surfaceMapUID;

	
	
 	

	
	// ===> Attribute direction
	
	

	vector<Angle > direction;

	
	
 	

	
	// ===> Attribute numScrew, which is optional
	
	
	bool numScrewExists;
	

	int numScrew;

	
	
 	

	
	// ===> Attribute screwName, which is optional
	
	
	bool screwNameExists;
	

	vector<string > screwName;

	
	
 	

	
	// ===> Attribute screwMotion, which is optional
	
	
	bool screwMotionExists;
	

	vector<Length > screwMotion;

	
	
 	

	
	// ===> Attribute screwMotionError, which is optional
	
	
	bool screwMotionErrorExists;
	

	vector<Length > screwMotionError;

	
	
 	

	
	// ===> Attribute gravCorrection, which is optional
	
	
	bool gravCorrectionExists;
	

	bool gravCorrection;

	
	
 	

	
	// ===> Attribute gravOptRange, which is optional
	
	
	bool gravOptRangeExists;
	

	vector<Angle > gravOptRange;

	
	
 	

	
	// ===> Attribute tempCorrection, which is optional
	
	
	bool tempCorrectionExists;
	

	bool tempCorrection;

	
	
 	

	
	// ===> Attribute tempOptRange, which is optional
	
	
	bool tempOptRangeExists;
	

	vector<Temperature > tempOptRange;

	
	
 	

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
	std::map<std::string, CalHolographyAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianIStream& eis);
void calDataIdFromBin( EndianIStream& eis);
void calReductionIdFromBin( EndianIStream& eis);
void antennaMakeFromBin( EndianIStream& eis);
void startValidTimeFromBin( EndianIStream& eis);
void endValidTimeFromBin( EndianIStream& eis);
void ambientTemperatureFromBin( EndianIStream& eis);
void focusPositionFromBin( EndianIStream& eis);
void frequencyRangeFromBin( EndianIStream& eis);
void illuminationTaperFromBin( EndianIStream& eis);
void numReceptorFromBin( EndianIStream& eis);
void polarizationTypesFromBin( EndianIStream& eis);
void numPanelModesFromBin( EndianIStream& eis);
void receiverBandFromBin( EndianIStream& eis);
void beamMapUIDFromBin( EndianIStream& eis);
void rawRMSFromBin( EndianIStream& eis);
void weightedRMSFromBin( EndianIStream& eis);
void surfaceMapUIDFromBin( EndianIStream& eis);
void directionFromBin( EndianIStream& eis);

void numScrewFromBin( EndianIStream& eis);
void screwNameFromBin( EndianIStream& eis);
void screwMotionFromBin( EndianIStream& eis);
void screwMotionErrorFromBin( EndianIStream& eis);
void gravCorrectionFromBin( EndianIStream& eis);
void gravOptRangeFromBin( EndianIStream& eis);
void tempCorrectionFromBin( EndianIStream& eis);
void tempOptRangeFromBin( EndianIStream& eis);

*/
	
	///////////////////////////////////
	// text-deserialization material //
	///////////////////////////////////
	std::map<std::string, CalHolographyAttributeFromText> fromTextMethods;
	
void antennaNameFromText (const string & s);
	
	
void calDataIdFromText (const string & s);
	
	
void calReductionIdFromText (const string & s);
	
	
void antennaMakeFromText (const string & s);
	
	
void startValidTimeFromText (const string & s);
	
	
void endValidTimeFromText (const string & s);
	
	
void ambientTemperatureFromText (const string & s);
	
	
void focusPositionFromText (const string & s);
	
	
void frequencyRangeFromText (const string & s);
	
	
void illuminationTaperFromText (const string & s);
	
	
void numReceptorFromText (const string & s);
	
	
void polarizationTypesFromText (const string & s);
	
	
void numPanelModesFromText (const string & s);
	
	
void receiverBandFromText (const string & s);
	
	
	
void rawRMSFromText (const string & s);
	
	
void weightedRMSFromText (const string & s);
	
	
	
void directionFromText (const string & s);
	

	
void numScrewFromText (const string & s);
	
	
void screwNameFromText (const string & s);
	
	
void screwMotionFromText (const string & s);
	
	
void screwMotionErrorFromText (const string & s);
	
	
void gravCorrectionFromText (const string & s);
	
	
void gravOptRangeFromText (const string & s);
	
	
void tempCorrectionFromText (const string & s);
	
	
void tempOptRangeFromText (const string & s);
	
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianIStream to build a PointingRow.
	  * @param eiss the EndianIStream to be read.
	  * @param table the CalHolographyTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.

	 static CalHolographyRow* fromBin(EndianIStream& eis, CalHolographyTable& table, const std::vector<std::string>& attributesSeq);	 
		*/
};

} // End namespace asdm

#endif /* CalHolography_CLASS */
