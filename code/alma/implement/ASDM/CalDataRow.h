
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
 * File CalDataRow.h
 */
 
#ifndef CalDataRow_CLASS
#define CalDataRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalDataRowIDL;
#endif






#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Tag.h>
using  asdm::Tag;

#include <EntityRef.h>
using  asdm::EntityRef;




	

	

	

	

	
#include "CCalDataOrigin.h"
using namespace CalDataOriginMod;
	

	
#include "CCalType.h"
using namespace CalTypeMod;
	

	

	

	

	
#include "CAssociatedCalNature.h"
using namespace AssociatedCalNatureMod;
	

	

	

	

	
#include "CScanIntent.h"
using namespace ScanIntentMod;
	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file CalData.h
    \brief Generated from model's revision "1.55", branch "HEAD"
*/

namespace asdm {

//class asdm::CalDataTable;

	

class CalDataRow;
typedef void (CalDataRow::*CalDataAttributeFromBin) (EndianISStream& eiss);

/**
 * The CalDataRow class is a row of a CalDataTable.
 * 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 */
class CalDataRow {
friend class asdm::CalDataTable;

public:

	virtual ~CalDataRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalDataTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute calDataId
	
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag getCalDataId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute startTimeObserved
	
	
	

	
 	/**
 	 * Get startTimeObserved.
 	 * @return startTimeObserved as ArrayTime
 	 */
 	ArrayTime getStartTimeObserved() const;
	
 
 	
 	
 	/**
 	 * Set startTimeObserved with the specified ArrayTime.
 	 * @param startTimeObserved The ArrayTime value to which startTimeObserved is to be set.
 	 
 		
 			
 	 */
 	void setStartTimeObserved (ArrayTime startTimeObserved);
  		
	
	
	


	
	// ===> Attribute endTimeObserved
	
	
	

	
 	/**
 	 * Get endTimeObserved.
 	 * @return endTimeObserved as ArrayTime
 	 */
 	ArrayTime getEndTimeObserved() const;
	
 
 	
 	
 	/**
 	 * Set endTimeObserved with the specified ArrayTime.
 	 * @param endTimeObserved The ArrayTime value to which endTimeObserved is to be set.
 	 
 		
 			
 	 */
 	void setEndTimeObserved (ArrayTime endTimeObserved);
  		
	
	
	


	
	// ===> Attribute execBlockUID
	
	
	

	
 	/**
 	 * Get execBlockUID.
 	 * @return execBlockUID as EntityRef
 	 */
 	EntityRef getExecBlockUID() const;
	
 
 	
 	
 	/**
 	 * Set execBlockUID with the specified EntityRef.
 	 * @param execBlockUID The EntityRef value to which execBlockUID is to be set.
 	 
 		
 			
 	 */
 	void setExecBlockUID (EntityRef execBlockUID);
  		
	
	
	


	
	// ===> Attribute calDataType
	
	
	

	
 	/**
 	 * Get calDataType.
 	 * @return calDataType as CalDataOriginMod::CalDataOrigin
 	 */
 	CalDataOriginMod::CalDataOrigin getCalDataType() const;
	
 
 	
 	
 	/**
 	 * Set calDataType with the specified CalDataOriginMod::CalDataOrigin.
 	 * @param calDataType The CalDataOriginMod::CalDataOrigin value to which calDataType is to be set.
 	 
 		
 			
 	 */
 	void setCalDataType (CalDataOriginMod::CalDataOrigin calDataType);
  		
	
	
	


	
	// ===> Attribute calType
	
	
	

	
 	/**
 	 * Get calType.
 	 * @return calType as CalTypeMod::CalType
 	 */
 	CalTypeMod::CalType getCalType() const;
	
 
 	
 	
 	/**
 	 * Set calType with the specified CalTypeMod::CalType.
 	 * @param calType The CalTypeMod::CalType value to which calType is to be set.
 	 
 		
 			
 	 */
 	void setCalType (CalTypeMod::CalType calType);
  		
	
	
	


	
	// ===> Attribute numScan
	
	
	

	
 	/**
 	 * Get numScan.
 	 * @return numScan as int
 	 */
 	int getNumScan() const;
	
 
 	
 	
 	/**
 	 * Set numScan with the specified int.
 	 * @param numScan The int value to which numScan is to be set.
 	 
 		
 			
 	 */
 	void setNumScan (int numScan);
  		
	
	
	


	
	// ===> Attribute scanSet
	
	
	

	
 	/**
 	 * Get scanSet.
 	 * @return scanSet as vector<int >
 	 */
 	vector<int > getScanSet() const;
	
 
 	
 	
 	/**
 	 * Set scanSet with the specified vector<int >.
 	 * @param scanSet The vector<int > value to which scanSet is to be set.
 	 
 		
 			
 	 */
 	void setScanSet (vector<int > scanSet);
  		
	
	
	


	
	// ===> Attribute assocCalDataId, which is optional
	
	
	
	/**
	 * The attribute assocCalDataId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocCalDataId attribute exists. 
	 */
	bool isAssocCalDataIdExists() const;
	

	
 	/**
 	 * Get assocCalDataId, which is optional.
 	 * @return assocCalDataId as Tag
 	 * @throws IllegalAccessException If assocCalDataId does not exist.
 	 */
 	Tag getAssocCalDataId() const;
	
 
 	
 	
 	/**
 	 * Set assocCalDataId with the specified Tag.
 	 * @param assocCalDataId The Tag value to which assocCalDataId is to be set.
 	 
 		
 	 */
 	void setAssocCalDataId (Tag assocCalDataId);
		
	
	
	
	/**
	 * Mark assocCalDataId, which is an optional field, as non-existent.
	 */
	void clearAssocCalDataId ();
	


	
	// ===> Attribute assocCalNature, which is optional
	
	
	
	/**
	 * The attribute assocCalNature is optional. Return true if this attribute exists.
	 * @return true if and only if the assocCalNature attribute exists. 
	 */
	bool isAssocCalNatureExists() const;
	

	
 	/**
 	 * Get assocCalNature, which is optional.
 	 * @return assocCalNature as AssociatedCalNatureMod::AssociatedCalNature
 	 * @throws IllegalAccessException If assocCalNature does not exist.
 	 */
 	AssociatedCalNatureMod::AssociatedCalNature getAssocCalNature() const;
	
 
 	
 	
 	/**
 	 * Set assocCalNature with the specified AssociatedCalNatureMod::AssociatedCalNature.
 	 * @param assocCalNature The AssociatedCalNatureMod::AssociatedCalNature value to which assocCalNature is to be set.
 	 
 		
 	 */
 	void setAssocCalNature (AssociatedCalNatureMod::AssociatedCalNature assocCalNature);
		
	
	
	
	/**
	 * Mark assocCalNature, which is an optional field, as non-existent.
	 */
	void clearAssocCalNature ();
	


	
	// ===> Attribute fieldName, which is optional
	
	
	
	/**
	 * The attribute fieldName is optional. Return true if this attribute exists.
	 * @return true if and only if the fieldName attribute exists. 
	 */
	bool isFieldNameExists() const;
	

	
 	/**
 	 * Get fieldName, which is optional.
 	 * @return fieldName as vector<string >
 	 * @throws IllegalAccessException If fieldName does not exist.
 	 */
 	vector<string > getFieldName() const;
	
 
 	
 	
 	/**
 	 * Set fieldName with the specified vector<string >.
 	 * @param fieldName The vector<string > value to which fieldName is to be set.
 	 
 		
 	 */
 	void setFieldName (vector<string > fieldName);
		
	
	
	
	/**
	 * Mark fieldName, which is an optional field, as non-existent.
	 */
	void clearFieldName ();
	


	
	// ===> Attribute sourceName, which is optional
	
	
	
	/**
	 * The attribute sourceName is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceName attribute exists. 
	 */
	bool isSourceNameExists() const;
	

	
 	/**
 	 * Get sourceName, which is optional.
 	 * @return sourceName as vector<string >
 	 * @throws IllegalAccessException If sourceName does not exist.
 	 */
 	vector<string > getSourceName() const;
	
 
 	
 	
 	/**
 	 * Set sourceName with the specified vector<string >.
 	 * @param sourceName The vector<string > value to which sourceName is to be set.
 	 
 		
 	 */
 	void setSourceName (vector<string > sourceName);
		
	
	
	
	/**
	 * Mark sourceName, which is an optional field, as non-existent.
	 */
	void clearSourceName ();
	


	
	// ===> Attribute sourceCode, which is optional
	
	
	
	/**
	 * The attribute sourceCode is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceCode attribute exists. 
	 */
	bool isSourceCodeExists() const;
	

	
 	/**
 	 * Get sourceCode, which is optional.
 	 * @return sourceCode as vector<string >
 	 * @throws IllegalAccessException If sourceCode does not exist.
 	 */
 	vector<string > getSourceCode() const;
	
 
 	
 	
 	/**
 	 * Set sourceCode with the specified vector<string >.
 	 * @param sourceCode The vector<string > value to which sourceCode is to be set.
 	 
 		
 	 */
 	void setSourceCode (vector<string > sourceCode);
		
	
	
	
	/**
	 * Mark sourceCode, which is an optional field, as non-existent.
	 */
	void clearSourceCode ();
	


	
	// ===> Attribute scanIntent, which is optional
	
	
	
	/**
	 * The attribute scanIntent is optional. Return true if this attribute exists.
	 * @return true if and only if the scanIntent attribute exists. 
	 */
	bool isScanIntentExists() const;
	

	
 	/**
 	 * Get scanIntent, which is optional.
 	 * @return scanIntent as vector<ScanIntentMod::ScanIntent >
 	 * @throws IllegalAccessException If scanIntent does not exist.
 	 */
 	vector<ScanIntentMod::ScanIntent > getScanIntent() const;
	
 
 	
 	
 	/**
 	 * Set scanIntent with the specified vector<ScanIntentMod::ScanIntent >.
 	 * @param scanIntent The vector<ScanIntentMod::ScanIntent > value to which scanIntent is to be set.
 	 
 		
 	 */
 	void setScanIntent (vector<ScanIntentMod::ScanIntent > scanIntent);
		
	
	
	
	/**
	 * Mark scanIntent, which is an optional field, as non-existent.
	 */
	void clearScanIntent ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this CalDataRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startTimeObserved
	    
	 * @param endTimeObserved
	    
	 * @param execBlockUID
	    
	 * @param calDataType
	    
	 * @param calType
	    
	 * @param numScan
	    
	 * @param scanSet
	    
	 */ 
	bool compareNoAutoInc(ArrayTime startTimeObserved, ArrayTime endTimeObserved, EntityRef execBlockUID, CalDataOriginMod::CalDataOrigin calDataType, CalTypeMod::CalType calType, int numScan, vector<int > scanSet);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startTimeObserved
	    
	 * @param endTimeObserved
	    
	 * @param execBlockUID
	    
	 * @param calDataType
	    
	 * @param calType
	    
	 * @param numScan
	    
	 * @param scanSet
	    
	 */ 
	bool compareRequiredValue(ArrayTime startTimeObserved, ArrayTime endTimeObserved, EntityRef execBlockUID, CalDataOriginMod::CalDataOrigin calDataType, CalTypeMod::CalType calType, int numScan, vector<int > scanSet); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalDataRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalDataRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalDataRowIDL struct.
	 */
	CalDataRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalDataRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (CalDataRowIDL x) ;
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
	CalDataTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a CalDataRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalDataRow (CalDataTable &table);

	/**
	 * Create a CalDataRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalDataRow row and a CalDataTable table, the method creates a new
	 * CalDataRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalDataRow (CalDataTable &table, CalDataRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute calDataId
	
	

	Tag calDataId;

	
	
 	
 	/**
 	 * Set calDataId with the specified Tag value.
 	 * @param calDataId The Tag value to which calDataId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setCalDataId (Tag calDataId);
  		
	

	
	// ===> Attribute startTimeObserved
	
	

	ArrayTime startTimeObserved;

	
	
 	

	
	// ===> Attribute endTimeObserved
	
	

	ArrayTime endTimeObserved;

	
	
 	

	
	// ===> Attribute execBlockUID
	
	

	EntityRef execBlockUID;

	
	
 	

	
	// ===> Attribute calDataType
	
	

	CalDataOriginMod::CalDataOrigin calDataType;

	
	
 	

	
	// ===> Attribute calType
	
	

	CalTypeMod::CalType calType;

	
	
 	

	
	// ===> Attribute numScan
	
	

	int numScan;

	
	
 	

	
	// ===> Attribute scanSet
	
	

	vector<int > scanSet;

	
	
 	

	
	// ===> Attribute assocCalDataId, which is optional
	
	
	bool assocCalDataIdExists;
	

	Tag assocCalDataId;

	
	
 	

	
	// ===> Attribute assocCalNature, which is optional
	
	
	bool assocCalNatureExists;
	

	AssociatedCalNatureMod::AssociatedCalNature assocCalNature;

	
	
 	

	
	// ===> Attribute fieldName, which is optional
	
	
	bool fieldNameExists;
	

	vector<string > fieldName;

	
	
 	

	
	// ===> Attribute sourceName, which is optional
	
	
	bool sourceNameExists;
	

	vector<string > sourceName;

	
	
 	

	
	// ===> Attribute sourceCode, which is optional
	
	
	bool sourceCodeExists;
	

	vector<string > sourceCode;

	
	
 	

	
	// ===> Attribute scanIntent, which is optional
	
	
	bool scanIntentExists;
	

	vector<ScanIntentMod::ScanIntent > scanIntent;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, CalDataAttributeFromBin> fromBinMethods;
void calDataIdFromBin( EndianISStream& eiss);
void startTimeObservedFromBin( EndianISStream& eiss);
void endTimeObservedFromBin( EndianISStream& eiss);
void execBlockUIDFromBin( EndianISStream& eiss);
void calDataTypeFromBin( EndianISStream& eiss);
void calTypeFromBin( EndianISStream& eiss);
void numScanFromBin( EndianISStream& eiss);
void scanSetFromBin( EndianISStream& eiss);

void assocCalDataIdFromBin( EndianISStream& eiss);
void assocCalNatureFromBin( EndianISStream& eiss);
void fieldNameFromBin( EndianISStream& eiss);
void sourceNameFromBin( EndianISStream& eiss);
void sourceCodeFromBin( EndianISStream& eiss);
void scanIntentFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the CalDataTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalDataRow* fromBin(EndianISStream& eiss, CalDataTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* CalData_CLASS */
