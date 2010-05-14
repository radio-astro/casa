
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
 * File DopplerRow.h
 */
 
#ifndef DopplerRow_CLASS
#define DopplerRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::DopplerRowIDL;
#endif






	

	

	
#include "CDopplerReferenceCode.h"
using namespace DopplerReferenceCodeMod;
	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file Doppler.h
    \brief Generated from model's revision "1.54", branch "HEAD"
*/

namespace asdm {

//class asdm::DopplerTable;


// class asdm::SourceRow;
class SourceRow;
	

class DopplerRow;
typedef void (DopplerRow::*DopplerAttributeFromBin) (EndianISStream& eiss);

/**
 * The DopplerRow class is a row of a DopplerTable.
 * 
 * Generated from model's revision "1.54", branch "HEAD"
 *
 */
class DopplerRow {
friend class asdm::DopplerTable;

public:

	virtual ~DopplerRow();

	/**
	 * Return the table to which this row belongs.
	 */
	DopplerTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute dopplerId
	
	
	

	
 	/**
 	 * Get dopplerId.
 	 * @return dopplerId as int
 	 */
 	int getDopplerId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute transitionIndex
	
	
	

	
 	/**
 	 * Get transitionIndex.
 	 * @return transitionIndex as int
 	 */
 	int getTransitionIndex() const;
	
 
 	
 	
 	/**
 	 * Set transitionIndex with the specified int.
 	 * @param transitionIndex The int value to which transitionIndex is to be set.
 	 
 		
 			
 	 */
 	void setTransitionIndex (int transitionIndex);
  		
	
	
	


	
	// ===> Attribute velDef
	
	
	

	
 	/**
 	 * Get velDef.
 	 * @return velDef as DopplerReferenceCodeMod::DopplerReferenceCode
 	 */
 	DopplerReferenceCodeMod::DopplerReferenceCode getVelDef() const;
	
 
 	
 	
 	/**
 	 * Set velDef with the specified DopplerReferenceCodeMod::DopplerReferenceCode.
 	 * @param velDef The DopplerReferenceCodeMod::DopplerReferenceCode value to which velDef is to be set.
 	 
 		
 			
 	 */
 	void setVelDef (DopplerReferenceCodeMod::DopplerReferenceCode velDef);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceId
	
	
	

	
 	/**
 	 * Get sourceId.
 	 * @return sourceId as int
 	 */
 	int getSourceId() const;
	
 
 	
 	
 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSourceId (int sourceId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	// ===> Slice link from a row of Doppler table to a collection of row of Source table.
	
	/**
	 * Get the collection of row in the Source table having sourceId == this.sourceId
	 * 
	 * @return a vector of SourceRow *
	 */
	vector <SourceRow *> getSources();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this DopplerRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param sourceId
	    
	 * @param transitionIndex
	    
	 * @param velDef
	    
	 */ 
	bool compareNoAutoInc(int sourceId, int transitionIndex, DopplerReferenceCodeMod::DopplerReferenceCode velDef);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param transitionIndex
	    
	 * @param velDef
	    
	 */ 
	bool compareRequiredValue(int transitionIndex, DopplerReferenceCodeMod::DopplerReferenceCode velDef); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the DopplerRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(DopplerRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a DopplerRowIDL struct.
	 */
	DopplerRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct DopplerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (DopplerRowIDL x) ;
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
	DopplerTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a DopplerRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	DopplerRow (DopplerTable &table);

	/**
	 * Create a DopplerRow using a copy constructor mechanism.
	 * <p>
	 * Given a DopplerRow row and a DopplerTable table, the method creates a new
	 * DopplerRow owned by table. Each attribute of the created row is a copy (deep)
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
	 DopplerRow (DopplerTable &table, DopplerRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute dopplerId
	
	

	int dopplerId;

	
	
 	
 	/**
 	 * Set dopplerId with the specified int value.
 	 * @param dopplerId The int value to which dopplerId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setDopplerId (int dopplerId);
  		
	

	
	// ===> Attribute transitionIndex
	
	

	int transitionIndex;

	
	
 	

	
	// ===> Attribute velDef
	
	

	DopplerReferenceCodeMod::DopplerReferenceCode velDef;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceId
	
	

	int sourceId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		


	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, DopplerAttributeFromBin> fromBinMethods;
void dopplerIdFromBin( EndianISStream& eiss);
void sourceIdFromBin( EndianISStream& eiss);
void transitionIndexFromBin( EndianISStream& eiss);
void velDefFromBin( EndianISStream& eiss);

		
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the DopplerTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static DopplerRow* fromBin(EndianISStream& eiss, DopplerTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* Doppler_CLASS */
