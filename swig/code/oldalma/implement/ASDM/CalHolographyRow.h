
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
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalHolographyRowIDL;
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




	

	
#include "CAntennaMake.h"
using namespace AntennaMakeMod;
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	
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

/*\file CalHolography.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::CalHolographyTable;


// class asdm::CalReductionRow;
class CalReductionRow;

// class asdm::CalDataRow;
class CalDataRow;
	

/**
 * The CalHolographyRow class is a row of a CalHolographyTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class CalHolographyRow {
friend class asdm::CalHolographyTable;

public:

	virtual ~CalHolographyRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalHolographyTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalHolographyRowIDL struct.
	 */
	CalHolographyRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalHolographyRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (CalHolographyRowIDL x) throw(ConversionException);
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
  		
	
	
	


	
	// ===> Attribute numScrew
	
	
	

	
 	/**
 	 * Get numScrew.
 	 * @return numScrew as int
 	 */
 	int getNumScrew() const;
	
 
 	
 	
 	/**
 	 * Set numScrew with the specified int.
 	 * @param numScrew The int value to which numScrew is to be set.
 	 
 		
 			
 	 */
 	void setNumScrew (int numScrew);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute rawRms
	
	
	

	
 	/**
 	 * Get rawRms.
 	 * @return rawRms as Length
 	 */
 	Length getRawRms() const;
	
 
 	
 	
 	/**
 	 * Set rawRms with the specified Length.
 	 * @param rawRms The Length value to which rawRms is to be set.
 	 
 		
 			
 	 */
 	void setRawRms (Length rawRms);
  		
	
	
	


	
	// ===> Attribute weightedRms
	
	
	

	
 	/**
 	 * Get weightedRms.
 	 * @return weightedRms as Length
 	 */
 	Length getWeightedRms() const;
	
 
 	
 	
 	/**
 	 * Set weightedRms with the specified Length.
 	 * @param weightedRms The Length value to which weightedRms is to be set.
 	 
 		
 			
 	 */
 	void setWeightedRms (Length weightedRms);
  		
	
	
	


	
	// ===> Attribute screwName
	
	
	

	
 	/**
 	 * Get screwName.
 	 * @return screwName as vector<string >
 	 */
 	vector<string > getScrewName() const;
	
 
 	
 	
 	/**
 	 * Set screwName with the specified vector<string >.
 	 * @param screwName The vector<string > value to which screwName is to be set.
 	 
 		
 			
 	 */
 	void setScrewName (vector<string > screwName);
  		
	
	
	


	
	// ===> Attribute screwMotion
	
	
	

	
 	/**
 	 * Get screwMotion.
 	 * @return screwMotion as vector<Length >
 	 */
 	vector<Length > getScrewMotion() const;
	
 
 	
 	
 	/**
 	 * Set screwMotion with the specified vector<Length >.
 	 * @param screwMotion The vector<Length > value to which screwMotion is to be set.
 	 
 		
 			
 	 */
 	void setScrewMotion (vector<Length > screwMotion);
  		
	
	
	


	
	// ===> Attribute screwMotionError
	
	
	

	
 	/**
 	 * Get screwMotionError.
 	 * @return screwMotionError as vector<Length >
 	 */
 	vector<Length > getScrewMotionError() const;
	
 
 	
 	
 	/**
 	 * Set screwMotionError with the specified vector<Length >.
 	 * @param screwMotionError The vector<Length > value to which screwMotionError is to be set.
 	 
 		
 			
 	 */
 	void setScrewMotionError (vector<Length > screwMotionError);
  		
	
	
	


	
	// ===> Attribute panelModes
	
	
	

	
 	/**
 	 * Get panelModes.
 	 * @return panelModes as int
 	 */
 	int getPanelModes() const;
	
 
 	
 	
 	/**
 	 * Set panelModes with the specified int.
 	 * @param panelModes The int value to which panelModes is to be set.
 	 
 		
 			
 	 */
 	void setPanelModes (int panelModes);
  		
	
	
	


	
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
	 */ 
	bool compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, AntennaMakeMod::AntennaMake antennaMake, int numScrew, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusPosition, Length rawRms, Length weightedRms, vector<string > screwName, vector<Length > screwMotion, vector<Length > screwMotionError, int panelModes, vector<Frequency > frequencyRange, EntityRef beamMapUID, EntityRef surfaceMapUID, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ReceiverBandMod::ReceiverBand receiverBand, vector<Angle > direction, Temperature ambientTemperature);
	
	

	
	bool compareRequiredValue(AntennaMakeMod::AntennaMake antennaMake, int numScrew, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusPosition, Length rawRms, Length weightedRms, vector<string > screwName, vector<Length > screwMotion, vector<Length > screwMotionError, int panelModes, vector<Frequency > frequencyRange, EntityRef beamMapUID, EntityRef surfaceMapUID, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ReceiverBandMod::ReceiverBand receiverBand, vector<Angle > direction, Temperature ambientTemperature); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalHolographyRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalHolographyRow* x) ;

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
	void isAdded();


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

	
	
 	

	
	// ===> Attribute numScrew
	
	

	int numScrew;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute focusPosition
	
	

	vector<Length > focusPosition;

	
	
 	

	
	// ===> Attribute rawRms
	
	

	Length rawRms;

	
	
 	

	
	// ===> Attribute weightedRms
	
	

	Length weightedRms;

	
	
 	

	
	// ===> Attribute screwName
	
	

	vector<string > screwName;

	
	
 	

	
	// ===> Attribute screwMotion
	
	

	vector<Length > screwMotion;

	
	
 	

	
	// ===> Attribute screwMotionError
	
	

	vector<Length > screwMotionError;

	
	
 	

	
	// ===> Attribute panelModes
	
	

	int panelModes;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute beamMapUID
	
	

	EntityRef beamMapUID;

	
	
 	

	
	// ===> Attribute surfaceMapUID
	
	

	EntityRef surfaceMapUID;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute direction
	
	

	vector<Angle > direction;

	
	
 	

	
	// ===> Attribute ambientTemperature
	
	

	Temperature ambientTemperature;

	
	
 	

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

#endif /* CalHolography_CLASS */
