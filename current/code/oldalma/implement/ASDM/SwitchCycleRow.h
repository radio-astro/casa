
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
 * File SwitchCycleRow.h
 */
 
#ifndef SwitchCycleRow_CLASS
#define SwitchCycleRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::SwitchCycleRowIDL;
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

/*\file SwitchCycle.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::SwitchCycleTable;

	

/**
 * The SwitchCycleRow class is a row of a SwitchCycleTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class SwitchCycleRow {
friend class asdm::SwitchCycleTable;

public:

	virtual ~SwitchCycleRow();

	/**
	 * Return the table to which this row belongs.
	 */
	SwitchCycleTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SwitchCycleRowIDL struct.
	 */
	SwitchCycleRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SwitchCycleRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (SwitchCycleRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute switchCycleId
	
	
	

	
 	/**
 	 * Get switchCycleId.
 	 * @return switchCycleId as Tag
 	 */
 	Tag getSwitchCycleId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute numStep
	
	
	

	
 	/**
 	 * Get numStep.
 	 * @return numStep as int
 	 */
 	int getNumStep() const;
	
 
 	
 	
 	/**
 	 * Set numStep with the specified int.
 	 * @param numStep The int value to which numStep is to be set.
 	 
 		
 			
 	 */
 	void setNumStep (int numStep);
  		
	
	
	


	
	// ===> Attribute weightArray
	
	
	

	
 	/**
 	 * Get weightArray.
 	 * @return weightArray as vector<float >
 	 */
 	vector<float > getWeightArray() const;
	
 
 	
 	
 	/**
 	 * Set weightArray with the specified vector<float >.
 	 * @param weightArray The vector<float > value to which weightArray is to be set.
 	 
 		
 			
 	 */
 	void setWeightArray (vector<float > weightArray);
  		
	
	
	


	
	// ===> Attribute dirOffsetArray
	
	
	

	
 	/**
 	 * Get dirOffsetArray.
 	 * @return dirOffsetArray as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getDirOffsetArray() const;
	
 
 	
 	
 	/**
 	 * Set dirOffsetArray with the specified vector<vector<Angle > >.
 	 * @param dirOffsetArray The vector<vector<Angle > > value to which dirOffsetArray is to be set.
 	 
 		
 			
 	 */
 	void setDirOffsetArray (vector<vector<Angle > > dirOffsetArray);
  		
	
	
	


	
	// ===> Attribute freqOffsetArray
	
	
	

	
 	/**
 	 * Get freqOffsetArray.
 	 * @return freqOffsetArray as vector<Frequency >
 	 */
 	vector<Frequency > getFreqOffsetArray() const;
	
 
 	
 	
 	/**
 	 * Set freqOffsetArray with the specified vector<Frequency >.
 	 * @param freqOffsetArray The vector<Frequency > value to which freqOffsetArray is to be set.
 	 
 		
 			
 	 */
 	void setFreqOffsetArray (vector<Frequency > freqOffsetArray);
  		
	
	
	


	
	// ===> Attribute stepDurationArray
	
	
	

	
 	/**
 	 * Get stepDurationArray.
 	 * @return stepDurationArray as vector<Interval >
 	 */
 	vector<Interval > getStepDurationArray() const;
	
 
 	
 	
 	/**
 	 * Set stepDurationArray with the specified vector<Interval >.
 	 * @param stepDurationArray The vector<Interval > value to which stepDurationArray is to be set.
 	 
 		
 			
 	 */
 	void setStepDurationArray (vector<Interval > stepDurationArray);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SwitchCycleRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(int numStep, vector<float > weightArray, vector<vector<Angle > > dirOffsetArray, vector<Frequency > freqOffsetArray, vector<Interval > stepDurationArray);
	
	

	
	bool compareRequiredValue(int numStep, vector<float > weightArray, vector<vector<Angle > > dirOffsetArray, vector<Frequency > freqOffsetArray, vector<Interval > stepDurationArray); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SwitchCycleRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(SwitchCycleRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	SwitchCycleTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a SwitchCycleRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SwitchCycleRow (SwitchCycleTable &table);

	/**
	 * Create a SwitchCycleRow using a copy constructor mechanism.
	 * <p>
	 * Given a SwitchCycleRow row and a SwitchCycleTable table, the method creates a new
	 * SwitchCycleRow owned by table. Each attribute of the created row is a copy (deep)
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
	 SwitchCycleRow (SwitchCycleTable &table, SwitchCycleRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute switchCycleId
	
	

	Tag switchCycleId;

	
	
 	
 	/**
 	 * Set switchCycleId with the specified Tag value.
 	 * @param switchCycleId The Tag value to which switchCycleId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSwitchCycleId (Tag switchCycleId);
  		
	

	
	// ===> Attribute numStep
	
	

	int numStep;

	
	
 	

	
	// ===> Attribute weightArray
	
	

	vector<float > weightArray;

	
	
 	

	
	// ===> Attribute dirOffsetArray
	
	

	vector<vector<Angle > > dirOffsetArray;

	
	
 	

	
	// ===> Attribute freqOffsetArray
	
	

	vector<Frequency > freqOffsetArray;

	
	
 	

	
	// ===> Attribute stepDurationArray
	
	

	vector<Interval > stepDurationArray;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	

};

} // End namespace asdm

#endif /* SwitchCycle_CLASS */
