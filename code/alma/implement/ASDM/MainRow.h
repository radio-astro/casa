
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
 * File MainRow.h
 */
 
#ifndef MainRow_CLASS
#define MainRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::MainRowIDL;
#endif






#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Interval.h>
using  asdm::Interval;

#include <Tag.h>
using  asdm::Tag;

#include <EntityRef.h>
using  asdm::EntityRef;




	

	

	
#include "CTimeSampling.h"
using namespace TimeSamplingMod;
	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file Main.h
    \brief Generated from model's revision "1.55", branch "HEAD"
*/

namespace asdm {

//class asdm::MainTable;


// class asdm::ConfigDescriptionRow;
class ConfigDescriptionRow;

// class asdm::FieldRow;
class FieldRow;

// class asdm::StateRow;
class StateRow;

// class asdm::ExecBlockRow;
class ExecBlockRow;
	

class MainRow;
typedef void (MainRow::*MainAttributeFromBin) (EndianISStream& eiss);

/**
 * The MainRow class is a row of a MainTable.
 * 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 */
class MainRow {
friend class asdm::MainTable;

public:

	virtual ~MainRow();

	/**
	 * Return the table to which this row belongs.
	 */
	MainTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute time
	
	
	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime getTime() const;
	
 
 	
 	
 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setTime (ArrayTime time);
  		
	
	
	


	
	// ===> Attribute numAntenna
	
	
	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int getNumAntenna() const;
	
 
 	
 	
 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 		
 			
 	 */
 	void setNumAntenna (int numAntenna);
  		
	
	
	


	
	// ===> Attribute timeSampling
	
	
	

	
 	/**
 	 * Get timeSampling.
 	 * @return timeSampling as TimeSamplingMod::TimeSampling
 	 */
 	TimeSamplingMod::TimeSampling getTimeSampling() const;
	
 
 	
 	
 	/**
 	 * Set timeSampling with the specified TimeSamplingMod::TimeSampling.
 	 * @param timeSampling The TimeSamplingMod::TimeSampling value to which timeSampling is to be set.
 	 
 		
 			
 	 */
 	void setTimeSampling (TimeSamplingMod::TimeSampling timeSampling);
  		
	
	
	


	
	// ===> Attribute interval
	
	
	

	
 	/**
 	 * Get interval.
 	 * @return interval as Interval
 	 */
 	Interval getInterval() const;
	
 
 	
 	
 	/**
 	 * Set interval with the specified Interval.
 	 * @param interval The Interval value to which interval is to be set.
 	 
 		
 			
 	 */
 	void setInterval (Interval interval);
  		
	
	
	


	
	// ===> Attribute numIntegration
	
	
	

	
 	/**
 	 * Get numIntegration.
 	 * @return numIntegration as int
 	 */
 	int getNumIntegration() const;
	
 
 	
 	
 	/**
 	 * Set numIntegration with the specified int.
 	 * @param numIntegration The int value to which numIntegration is to be set.
 	 
 		
 			
 	 */
 	void setNumIntegration (int numIntegration);
  		
	
	
	


	
	// ===> Attribute scanNumber
	
	
	

	
 	/**
 	 * Get scanNumber.
 	 * @return scanNumber as int
 	 */
 	int getScanNumber() const;
	
 
 	
 	
 	/**
 	 * Set scanNumber with the specified int.
 	 * @param scanNumber The int value to which scanNumber is to be set.
 	 
 		
 			
 	 */
 	void setScanNumber (int scanNumber);
  		
	
	
	


	
	// ===> Attribute subscanNumber
	
	
	

	
 	/**
 	 * Get subscanNumber.
 	 * @return subscanNumber as int
 	 */
 	int getSubscanNumber() const;
	
 
 	
 	
 	/**
 	 * Set subscanNumber with the specified int.
 	 * @param subscanNumber The int value to which subscanNumber is to be set.
 	 
 		
 			
 	 */
 	void setSubscanNumber (int subscanNumber);
  		
	
	
	


	
	// ===> Attribute dataSize
	
	
	

	
 	/**
 	 * Get dataSize.
 	 * @return dataSize as int
 	 */
 	int getDataSize() const;
	
 
 	
 	
 	/**
 	 * Set dataSize with the specified int.
 	 * @param dataSize The int value to which dataSize is to be set.
 	 
 		
 			
 	 */
 	void setDataSize (int dataSize);
  		
	
	
	


	
	// ===> Attribute dataOid
	
	
	

	
 	/**
 	 * Get dataOid.
 	 * @return dataOid as EntityRef
 	 */
 	EntityRef getDataOid() const;
	
 
 	
 	
 	/**
 	 * Set dataOid with the specified EntityRef.
 	 * @param dataOid The EntityRef value to which dataOid is to be set.
 	 
 		
 			
 	 */
 	void setDataOid (EntityRef dataOid);
  		
	
	
	


	
	// ===> Attribute flagRow, which is optional
	
	
	
	/**
	 * The attribute flagRow is optional. Return true if this attribute exists.
	 * @return true if and only if the flagRow attribute exists. 
	 */
	bool isFlagRowExists() const;
	

	
 	/**
 	 * Get flagRow, which is optional.
 	 * @return flagRow as bool
 	 * @throws IllegalAccessException If flagRow does not exist.
 	 */
 	bool getFlagRow() const;
	
 
 	
 	
 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 		
 	 */
 	void setFlagRow (bool flagRow);
		
	
	
	
	/**
	 * Mark flagRow, which is an optional field, as non-existent.
	 */
	void clearFlagRow ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute configDescriptionId
	
	
	

	
 	/**
 	 * Get configDescriptionId.
 	 * @return configDescriptionId as Tag
 	 */
 	Tag getConfigDescriptionId() const;
	
 
 	
 	
 	/**
 	 * Set configDescriptionId with the specified Tag.
 	 * @param configDescriptionId The Tag value to which configDescriptionId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setConfigDescriptionId (Tag configDescriptionId);
  		
	
	
	


	
	// ===> Attribute execBlockId
	
	
	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag getExecBlockId() const;
	
 
 	
 	
 	/**
 	 * Set execBlockId with the specified Tag.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
 	 
 		
 			
 	 */
 	void setExecBlockId (Tag execBlockId);
  		
	
	
	


	
	// ===> Attribute fieldId
	
	
	

	
 	/**
 	 * Get fieldId.
 	 * @return fieldId as Tag
 	 */
 	Tag getFieldId() const;
	
 
 	
 	
 	/**
 	 * Set fieldId with the specified Tag.
 	 * @param fieldId The Tag value to which fieldId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setFieldId (Tag fieldId);
  		
	
	
	


	
	// ===> Attribute stateId
	
	
	

	
 	/**
 	 * Get stateId.
 	 * @return stateId as vector<Tag> 
 	 */
 	vector<Tag>  getStateId() const;
	
 
 	
 	
 	/**
 	 * Set stateId with the specified vector<Tag> .
 	 * @param stateId The vector<Tag>  value to which stateId is to be set.
 	 
 		
 			
 	 */
 	void setStateId (vector<Tag>  stateId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * configDescriptionId pointer to the row in the ConfigDescription table having ConfigDescription.configDescriptionId == configDescriptionId
	 * @return a ConfigDescriptionRow*
	 * 
	 
	 */
	 ConfigDescriptionRow* getConfigDescriptionUsingConfigDescriptionId();
	 

	

	

	
		
	/**
	 * fieldId pointer to the row in the Field table having Field.fieldId == fieldId
	 * @return a FieldRow*
	 * 
	 
	 */
	 FieldRow* getFieldUsingFieldId();
	 

	

	
 		
 	/**
 	 * Set stateId[i] with the specified Tag.
 	 * @param i The index in stateId where to set the Tag value.
 	 * @param stateId The Tag value to which stateId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void setStateId (int i, Tag stateId); 
 			
	

	
		 
/**
 * Append a Tag to stateId.
 * @param id the Tag to be appended to stateId
 */
 void addStateId(Tag id); 

/**
 * Append a vector of Tag to stateId.
 * @param id an array of Tag to be appended to stateId
 */
 void addStateId(const vector<Tag> & id); 
 

 /**
  * Returns the Tag stored in stateId at position i.
  * @param i the position in stateId where the Tag is retrieved.
  * @return the Tag stored at position i in stateId.
  */
 const Tag getStateId(int i);
 
 /**
  * Returns the StateRow linked to this row via the tag stored in stateId
  * at position i.
  * @param i the position in stateId.
  * @return a pointer on a StateRow whose key (a Tag) is equal to the Tag stored at position
  * i in the stateId. 
  */
 StateRow* getStateUsingStateId(int i); 
 
 /**
  * Returns the vector of StateRow* linked to this row via the Tags stored in stateId
  * @return an array of pointers on StateRow.
  */
 vector<StateRow *> getStatesUsingStateId(); 
  

	

	

	
		
	/**
	 * execBlockId pointer to the row in the ExecBlock table having ExecBlock.execBlockId == execBlockId
	 * @return a ExecBlockRow*
	 * 
	 
	 */
	 ExecBlockRow* getExecBlockUsingExecBlockId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this MainRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param time
	    
	 * @param configDescriptionId
	    
	 * @param fieldId
	    
	 * @param numAntenna
	    
	 * @param timeSampling
	    
	 * @param interval
	    
	 * @param numIntegration
	    
	 * @param scanNumber
	    
	 * @param subscanNumber
	    
	 * @param dataSize
	    
	 * @param dataOid
	    
	 * @param stateId
	    
	 * @param execBlockId
	    
	 */ 
	bool compareNoAutoInc(ArrayTime time, Tag configDescriptionId, Tag fieldId, int numAntenna, TimeSamplingMod::TimeSampling timeSampling, Interval interval, int numIntegration, int scanNumber, int subscanNumber, int dataSize, EntityRef dataOid, vector<Tag>  stateId, Tag execBlockId);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param numAntenna
	    
	 * @param timeSampling
	    
	 * @param interval
	    
	 * @param numIntegration
	    
	 * @param scanNumber
	    
	 * @param subscanNumber
	    
	 * @param dataSize
	    
	 * @param dataOid
	    
	 * @param stateId
	    
	 * @param execBlockId
	    
	 */ 
	bool compareRequiredValue(int numAntenna, TimeSamplingMod::TimeSampling timeSampling, Interval interval, int numIntegration, int scanNumber, int subscanNumber, int dataSize, EntityRef dataOid, vector<Tag>  stateId, Tag execBlockId); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the MainRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(MainRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a MainRowIDL struct.
	 */
	MainRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct MainRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (MainRowIDL x) ;
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
	MainTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a MainRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	MainRow (MainTable &table);

	/**
	 * Create a MainRow using a copy constructor mechanism.
	 * <p>
	 * Given a MainRow row and a MainTable table, the method creates a new
	 * MainRow owned by table. Each attribute of the created row is a copy (deep)
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
	 MainRow (MainTable &table, MainRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute time
	
	

	ArrayTime time;

	
	
 	

	
	// ===> Attribute numAntenna
	
	

	int numAntenna;

	
	
 	

	
	// ===> Attribute timeSampling
	
	

	TimeSamplingMod::TimeSampling timeSampling;

	
	
 	

	
	// ===> Attribute interval
	
	

	Interval interval;

	
	
 	

	
	// ===> Attribute numIntegration
	
	

	int numIntegration;

	
	
 	

	
	// ===> Attribute scanNumber
	
	

	int scanNumber;

	
	
 	

	
	// ===> Attribute subscanNumber
	
	

	int subscanNumber;

	
	
 	

	
	// ===> Attribute dataSize
	
	

	int dataSize;

	
	
 	

	
	// ===> Attribute dataOid
	
	

	EntityRef dataOid;

	
	
 	

	
	// ===> Attribute flagRow, which is optional
	
	
	bool flagRowExists;
	

	bool flagRow;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute configDescriptionId
	
	

	Tag configDescriptionId;

	
	
 	

	
	// ===> Attribute execBlockId
	
	

	Tag execBlockId;

	
	
 	

	
	// ===> Attribute fieldId
	
	

	Tag fieldId;

	
	
 	

	
	// ===> Attribute stateId
	
	

	vector<Tag>  stateId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	

	
		


	

	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, MainAttributeFromBin> fromBinMethods;
void timeFromBin( EndianISStream& eiss);
void configDescriptionIdFromBin( EndianISStream& eiss);
void fieldIdFromBin( EndianISStream& eiss);
void numAntennaFromBin( EndianISStream& eiss);
void timeSamplingFromBin( EndianISStream& eiss);
void intervalFromBin( EndianISStream& eiss);
void numIntegrationFromBin( EndianISStream& eiss);
void scanNumberFromBin( EndianISStream& eiss);
void subscanNumberFromBin( EndianISStream& eiss);
void dataSizeFromBin( EndianISStream& eiss);
void dataOidFromBin( EndianISStream& eiss);
void stateIdFromBin( EndianISStream& eiss);
void execBlockIdFromBin( EndianISStream& eiss);

void flagRowFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the MainTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static MainRow* fromBin(EndianISStream& eiss, MainTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* Main_CLASS */
