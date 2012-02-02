
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
 * File ProcessorRow.h
 */
 
#ifndef ProcessorRow_CLASS
#define ProcessorRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::ProcessorRowIDL;
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




	

	
#include "CProcessorType.h"
using namespace ProcessorTypeMod;
	

	

	



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

/*\file Processor.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::ProcessorTable;


// class asdm::CorrelatorModeRow;
class CorrelatorModeRow;
	

/**
 * The ProcessorRow class is a row of a ProcessorTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class ProcessorRow {
friend class asdm::ProcessorTable;

public:

	virtual ~ProcessorRow();

	/**
	 * Return the table to which this row belongs.
	 */
	ProcessorTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ProcessorRowIDL struct.
	 */
	ProcessorRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ProcessorRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (ProcessorRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute processorId
	
	
	

	
 	/**
 	 * Get processorId.
 	 * @return processorId as Tag
 	 */
 	Tag getProcessorId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute type
	
	
	

	
 	/**
 	 * Get type.
 	 * @return type as ProcessorTypeMod::ProcessorType
 	 */
 	ProcessorTypeMod::ProcessorType getType() const;
	
 
 	
 	
 	/**
 	 * Set type with the specified ProcessorTypeMod::ProcessorType.
 	 * @param type The ProcessorTypeMod::ProcessorType value to which type is to be set.
 	 
 		
 			
 	 */
 	void setType (ProcessorTypeMod::ProcessorType type);
  		
	
	
	


	
	// ===> Attribute subType
	
	
	

	
 	/**
 	 * Get subType.
 	 * @return subType as string
 	 */
 	string getSubType() const;
	
 
 	
 	
 	/**
 	 * Set subType with the specified string.
 	 * @param subType The string value to which subType is to be set.
 	 
 		
 			
 	 */
 	void setSubType (string subType);
  		
	
	
	


	
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
 	bool getFlagRow() const throw(IllegalAccessException);
	
 
 	
 	
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
	
	
	// ===> Attribute almaCorrelatorModeId
	
	
	

	
 	/**
 	 * Get almaCorrelatorModeId.
 	 * @return almaCorrelatorModeId as Tag
 	 */
 	Tag getAlmaCorrelatorModeId() const;
	
 
 	
 	
 	/**
 	 * Set almaCorrelatorModeId with the specified Tag.
 	 * @param almaCorrelatorModeId The Tag value to which almaCorrelatorModeId is to be set.
 	 
 		
 			
 	 */
 	void setAlmaCorrelatorModeId (Tag almaCorrelatorModeId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * almaCorrelatorModeId pointer to the row in the CorrelatorMode table having CorrelatorMode.almaCorrelatorModeId == almaCorrelatorModeId
	 * @return a CorrelatorModeRow*
	 * 
	 
	 */
	 CorrelatorModeRow* getCorrelatorModeUsingAlmaCorrelatorModeId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this ProcessorRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag almaCorrelatorModeId, ProcessorTypeMod::ProcessorType type, string subType);
	
	

	
	bool compareRequiredValue(Tag almaCorrelatorModeId, ProcessorTypeMod::ProcessorType type, string subType); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ProcessorRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(ProcessorRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	ProcessorTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a ProcessorRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ProcessorRow (ProcessorTable &table);

	/**
	 * Create a ProcessorRow using a copy constructor mechanism.
	 * <p>
	 * Given a ProcessorRow row and a ProcessorTable table, the method creates a new
	 * ProcessorRow owned by table. Each attribute of the created row is a copy (deep)
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
	 ProcessorRow (ProcessorTable &table, ProcessorRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute processorId
	
	

	Tag processorId;

	
	
 	
 	/**
 	 * Set processorId with the specified Tag value.
 	 * @param processorId The Tag value to which processorId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setProcessorId (Tag processorId);
  		
	

	
	// ===> Attribute type
	
	

	ProcessorTypeMod::ProcessorType type;

	
	
 	

	
	// ===> Attribute subType
	
	

	string subType;

	
	
 	

	
	// ===> Attribute flagRow, which is optional
	
	
	bool flagRowExists;
	

	bool flagRow;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute almaCorrelatorModeId
	
	

	Tag almaCorrelatorModeId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	


};

} // End namespace asdm

#endif /* Processor_CLASS */
