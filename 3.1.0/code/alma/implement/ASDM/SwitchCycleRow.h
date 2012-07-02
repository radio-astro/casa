
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



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Angle.h>
using  asdm::Angle;

#include <Interval.h>
using  asdm::Interval;

#include <Tag.h>
using  asdm::Tag;

#include <Frequency.h>
using  asdm::Frequency;




	

	

	

	

	

	

	
#include "CDirectionReferenceCode.h"
using namespace DirectionReferenceCodeMod;
	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file SwitchCycle.h
    \brief Generated from model's revision "1.54", branch "HEAD"
*/

namespace asdm {

//class asdm::SwitchCycleTable;

	

class SwitchCycleRow;
typedef void (SwitchCycleRow::*SwitchCycleAttributeFromBin) (EndianISStream& eiss);

/**
 * The SwitchCycleRow class is a row of a SwitchCycleTable.
 * 
 * Generated from model's revision "1.54", branch "HEAD"
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
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
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
  		
	
	
	


	
	// ===> Attribute directionCode, which is optional
	
	
	
	/**
	 * The attribute directionCode is optional. Return true if this attribute exists.
	 * @return true if and only if the directionCode attribute exists. 
	 */
	bool isDirectionCodeExists() const;
	

	
 	/**
 	 * Get directionCode, which is optional.
 	 * @return directionCode as DirectionReferenceCodeMod::DirectionReferenceCode
 	 * @throws IllegalAccessException If directionCode does not exist.
 	 */
 	DirectionReferenceCodeMod::DirectionReferenceCode getDirectionCode() const;
	
 
 	
 	
 	/**
 	 * Set directionCode with the specified DirectionReferenceCodeMod::DirectionReferenceCode.
 	 * @param directionCode The DirectionReferenceCodeMod::DirectionReferenceCode value to which directionCode is to be set.
 	 
 		
 	 */
 	void setDirectionCode (DirectionReferenceCodeMod::DirectionReferenceCode directionCode);
		
	
	
	
	/**
	 * Mark directionCode, which is an optional field, as non-existent.
	 */
	void clearDirectionCode ();
	


	
	// ===> Attribute directionEquinox, which is optional
	
	
	
	/**
	 * The attribute directionEquinox is optional. Return true if this attribute exists.
	 * @return true if and only if the directionEquinox attribute exists. 
	 */
	bool isDirectionEquinoxExists() const;
	

	
 	/**
 	 * Get directionEquinox, which is optional.
 	 * @return directionEquinox as ArrayTime
 	 * @throws IllegalAccessException If directionEquinox does not exist.
 	 */
 	ArrayTime getDirectionEquinox() const;
	
 
 	
 	
 	/**
 	 * Set directionEquinox with the specified ArrayTime.
 	 * @param directionEquinox The ArrayTime value to which directionEquinox is to be set.
 	 
 		
 	 */
 	void setDirectionEquinox (ArrayTime directionEquinox);
		
	
	
	
	/**
	 * Mark directionEquinox, which is an optional field, as non-existent.
	 */
	void clearDirectionEquinox ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SwitchCycleRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param numStep
	    
	 * @param weightArray
	    
	 * @param dirOffsetArray
	    
	 * @param freqOffsetArray
	    
	 * @param stepDurationArray
	    
	 */ 
	bool compareNoAutoInc(int numStep, vector<float > weightArray, vector<vector<Angle > > dirOffsetArray, vector<Frequency > freqOffsetArray, vector<Interval > stepDurationArray);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param numStep
	    
	 * @param weightArray
	    
	 * @param dirOffsetArray
	    
	 * @param freqOffsetArray
	    
	 * @param stepDurationArray
	    
	 */ 
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
	 * @throws ConversionException
	 */
	void setFromIDL (SwitchCycleRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param rowDoc the XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (string rowDoc) ;	

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
	void isAdded(bool added);


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

	
	
 	

	
	// ===> Attribute directionCode, which is optional
	
	
	bool directionCodeExists;
	

	DirectionReferenceCodeMod::DirectionReferenceCode directionCode;

	
	
 	

	
	// ===> Attribute directionEquinox, which is optional
	
	
	bool directionEquinoxExists;
	

	ArrayTime directionEquinox;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, SwitchCycleAttributeFromBin> fromBinMethods;
void switchCycleIdFromBin( EndianISStream& eiss);
void numStepFromBin( EndianISStream& eiss);
void weightArrayFromBin( EndianISStream& eiss);
void dirOffsetArrayFromBin( EndianISStream& eiss);
void freqOffsetArrayFromBin( EndianISStream& eiss);
void stepDurationArrayFromBin( EndianISStream& eiss);

void directionCodeFromBin( EndianISStream& eiss);
void directionEquinoxFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the SwitchCycleTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static SwitchCycleRow* fromBin(EndianISStream& eiss, SwitchCycleTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* SwitchCycle_CLASS */
