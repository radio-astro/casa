
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
 * File SquareLawDetectorRow.h
 */
 
#ifndef SquareLawDetectorRow_CLASS
#define SquareLawDetectorRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::SquareLawDetectorRowIDL;
#endif






#include <Tag.h>
using  asdm::Tag;




	

	

	
#include "CDetectorBandType.h"
using namespace DetectorBandTypeMod;
	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file SquareLawDetector.h
    \brief Generated from model's revision "1.57", branch "HEAD"
*/

namespace asdm {

//class asdm::SquareLawDetectorTable;

	

class SquareLawDetectorRow;
typedef void (SquareLawDetectorRow::*SquareLawDetectorAttributeFromBin) (EndianISStream& eiss);

/**
 * The SquareLawDetectorRow class is a row of a SquareLawDetectorTable.
 * 
 * Generated from model's revision "1.57", branch "HEAD"
 *
 */
class SquareLawDetectorRow {
friend class asdm::SquareLawDetectorTable;

public:

	virtual ~SquareLawDetectorRow();

	/**
	 * Return the table to which this row belongs.
	 */
	SquareLawDetectorTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute squareLawDetectorId
	
	
	

	
 	/**
 	 * Get squareLawDetectorId.
 	 * @return squareLawDetectorId as Tag
 	 */
 	Tag getSquareLawDetectorId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute numBand
	
	
	

	
 	/**
 	 * Get numBand.
 	 * @return numBand as int
 	 */
 	int getNumBand() const;
	
 
 	
 	
 	/**
 	 * Set numBand with the specified int.
 	 * @param numBand The int value to which numBand is to be set.
 	 
 		
 			
 	 */
 	void setNumBand (int numBand);
  		
	
	
	


	
	// ===> Attribute bandType
	
	
	

	
 	/**
 	 * Get bandType.
 	 * @return bandType as DetectorBandTypeMod::DetectorBandType
 	 */
 	DetectorBandTypeMod::DetectorBandType getBandType() const;
	
 
 	
 	
 	/**
 	 * Set bandType with the specified DetectorBandTypeMod::DetectorBandType.
 	 * @param bandType The DetectorBandTypeMod::DetectorBandType value to which bandType is to be set.
 	 
 		
 			
 	 */
 	void setBandType (DetectorBandTypeMod::DetectorBandType bandType);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SquareLawDetectorRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param numBand
	    
	 * @param bandType
	    
	 */ 
	bool compareNoAutoInc(int numBand, DetectorBandTypeMod::DetectorBandType bandType);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param numBand
	    
	 * @param bandType
	    
	 */ 
	bool compareRequiredValue(int numBand, DetectorBandTypeMod::DetectorBandType bandType); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SquareLawDetectorRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(SquareLawDetectorRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SquareLawDetectorRowIDL struct.
	 */
	SquareLawDetectorRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SquareLawDetectorRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (SquareLawDetectorRowIDL x) ;
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
	SquareLawDetectorTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a SquareLawDetectorRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SquareLawDetectorRow (SquareLawDetectorTable &table);

	/**
	 * Create a SquareLawDetectorRow using a copy constructor mechanism.
	 * <p>
	 * Given a SquareLawDetectorRow row and a SquareLawDetectorTable table, the method creates a new
	 * SquareLawDetectorRow owned by table. Each attribute of the created row is a copy (deep)
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
	 SquareLawDetectorRow (SquareLawDetectorTable &table, SquareLawDetectorRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute squareLawDetectorId
	
	

	Tag squareLawDetectorId;

	
	
 	
 	/**
 	 * Set squareLawDetectorId with the specified Tag value.
 	 * @param squareLawDetectorId The Tag value to which squareLawDetectorId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSquareLawDetectorId (Tag squareLawDetectorId);
  		
	

	
	// ===> Attribute numBand
	
	

	int numBand;

	
	
 	

	
	// ===> Attribute bandType
	
	

	DetectorBandTypeMod::DetectorBandType bandType;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, SquareLawDetectorAttributeFromBin> fromBinMethods;
void squareLawDetectorIdFromBin( EndianISStream& eiss);
void numBandFromBin( EndianISStream& eiss);
void bandTypeFromBin( EndianISStream& eiss);

		
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the SquareLawDetectorTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static SquareLawDetectorRow* fromBin(EndianISStream& eiss, SquareLawDetectorTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* SquareLawDetector_CLASS */
