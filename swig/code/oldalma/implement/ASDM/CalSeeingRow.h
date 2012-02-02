
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
 * File CalSeeingRow.h
 */
 
#ifndef CalSeeingRow_CLASS
#define CalSeeingRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalSeeingRowIDL;
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

/*\file CalSeeing.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::CalSeeingTable;


// class asdm::CalReductionRow;
class CalReductionRow;

// class asdm::CalDataRow;
class CalDataRow;
	

/**
 * The CalSeeingRow class is a row of a CalSeeingTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class CalSeeingRow {
friend class asdm::CalSeeingTable;

public:

	virtual ~CalSeeingRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalSeeingTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalSeeingRowIDL struct.
	 */
	CalSeeingRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalSeeingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (CalSeeingRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute numBaseLength
	
	
	

	
 	/**
 	 * Get numBaseLength.
 	 * @return numBaseLength as int
 	 */
 	int getNumBaseLength() const;
	
 
 	
 	
 	/**
 	 * Set numBaseLength with the specified int.
 	 * @param numBaseLength The int value to which numBaseLength is to be set.
 	 
 		
 			
 	 */
 	void setNumBaseLength (int numBaseLength);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute baseLength
	
	
	

	
 	/**
 	 * Get baseLength.
 	 * @return baseLength as vector<Length >
 	 */
 	vector<Length > getBaseLength() const;
	
 
 	
 	
 	/**
 	 * Set baseLength with the specified vector<Length >.
 	 * @param baseLength The vector<Length > value to which baseLength is to be set.
 	 
 		
 			
 	 */
 	void setBaseLength (vector<Length > baseLength);
  		
	
	
	


	
	// ===> Attribute corrPhaseRms
	
	
	

	
 	/**
 	 * Get corrPhaseRms.
 	 * @return corrPhaseRms as vector<Angle >
 	 */
 	vector<Angle > getCorrPhaseRms() const;
	
 
 	
 	
 	/**
 	 * Set corrPhaseRms with the specified vector<Angle >.
 	 * @param corrPhaseRms The vector<Angle > value to which corrPhaseRms is to be set.
 	 
 		
 			
 	 */
 	void setCorrPhaseRms (vector<Angle > corrPhaseRms);
  		
	
	
	


	
	// ===> Attribute uncorrPhaseRms
	
	
	

	
 	/**
 	 * Get uncorrPhaseRms.
 	 * @return uncorrPhaseRms as vector<Angle >
 	 */
 	vector<Angle > getUncorrPhaseRms() const;
	
 
 	
 	
 	/**
 	 * Set uncorrPhaseRms with the specified vector<Angle >.
 	 * @param uncorrPhaseRms The vector<Angle > value to which uncorrPhaseRms is to be set.
 	 
 		
 			
 	 */
 	void setUncorrPhaseRms (vector<Angle > uncorrPhaseRms);
  		
	
	
	


	
	// ===> Attribute seeing
	
	
	

	
 	/**
 	 * Get seeing.
 	 * @return seeing as Angle
 	 */
 	Angle getSeeing() const;
	
 
 	
 	
 	/**
 	 * Set seeing with the specified Angle.
 	 * @param seeing The Angle value to which seeing is to be set.
 	 
 		
 			
 	 */
 	void setSeeing (Angle seeing);
  		
	
	
	


	
	// ===> Attribute seeingFrequency
	
	
	

	
 	/**
 	 * Get seeingFrequency.
 	 * @return seeingFrequency as Frequency
 	 */
 	Frequency getSeeingFrequency() const;
	
 
 	
 	
 	/**
 	 * Set seeingFrequency with the specified Frequency.
 	 * @param seeingFrequency The Frequency value to which seeingFrequency is to be set.
 	 
 		
 			
 	 */
 	void setSeeingFrequency (Frequency seeingFrequency);
  		
	
	
	


	
	// ===> Attribute seeingFreqBandwidth
	
	
	

	
 	/**
 	 * Get seeingFreqBandwidth.
 	 * @return seeingFreqBandwidth as Frequency
 	 */
 	Frequency getSeeingFreqBandwidth() const;
	
 
 	
 	
 	/**
 	 * Set seeingFreqBandwidth with the specified Frequency.
 	 * @param seeingFreqBandwidth The Frequency value to which seeingFreqBandwidth is to be set.
 	 
 		
 			
 	 */
 	void setSeeingFreqBandwidth (Frequency seeingFreqBandwidth);
  		
	
	
	


	
	// ===> Attribute exponent, which is optional
	
	
	
	/**
	 * The attribute exponent is optional. Return true if this attribute exists.
	 * @return true if and only if the exponent attribute exists. 
	 */
	bool isExponentExists() const;
	

	
 	/**
 	 * Get exponent, which is optional.
 	 * @return exponent as float
 	 * @throws IllegalAccessException If exponent does not exist.
 	 */
 	float getExponent() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set exponent with the specified float.
 	 * @param exponent The float value to which exponent is to be set.
 	 
 		
 	 */
 	void setExponent (float exponent);
		
	
	
	
	/**
	 * Mark exponent, which is an optional field, as non-existent.
	 */
	void clearExponent ();
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this CalSeeingRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag calDataId, Tag calReductionId, int numBaseLength, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<Length > baseLength, vector<Angle > corrPhaseRms, vector<Angle > uncorrPhaseRms, Angle seeing, Frequency seeingFrequency, Frequency seeingFreqBandwidth);
	
	

	
	bool compareRequiredValue(int numBaseLength, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<Length > baseLength, vector<Angle > corrPhaseRms, vector<Angle > uncorrPhaseRms, Angle seeing, Frequency seeingFrequency, Frequency seeingFreqBandwidth); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalSeeingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalSeeingRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	CalSeeingTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalSeeingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalSeeingRow (CalSeeingTable &table);

	/**
	 * Create a CalSeeingRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalSeeingRow row and a CalSeeingTable table, the method creates a new
	 * CalSeeingRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalSeeingRow (CalSeeingTable &table, CalSeeingRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute numBaseLength
	
	

	int numBaseLength;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute baseLength
	
	

	vector<Length > baseLength;

	
	
 	

	
	// ===> Attribute corrPhaseRms
	
	

	vector<Angle > corrPhaseRms;

	
	
 	

	
	// ===> Attribute uncorrPhaseRms
	
	

	vector<Angle > uncorrPhaseRms;

	
	
 	

	
	// ===> Attribute seeing
	
	

	Angle seeing;

	
	
 	

	
	// ===> Attribute seeingFrequency
	
	

	Frequency seeingFrequency;

	
	
 	

	
	// ===> Attribute seeingFreqBandwidth
	
	

	Frequency seeingFreqBandwidth;

	
	
 	

	
	// ===> Attribute exponent, which is optional
	
	
	bool exponentExists;
	

	float exponent;

	
	
 	

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

#endif /* CalSeeing_CLASS */
