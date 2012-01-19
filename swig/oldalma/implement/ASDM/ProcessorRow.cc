
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
 * File ProcessorRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <ProcessorRow.h>
#include <ProcessorTable.h>

#include <CorrelatorModeTable.h>
#include <CorrelatorModeRow.h>
	

using asdm::ASDM;
using asdm::ProcessorRow;
using asdm::ProcessorTable;

using asdm::CorrelatorModeTable;
using asdm::CorrelatorModeRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	ProcessorRow::~ProcessorRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	ProcessorTable &ProcessorRow::getTable() const {
		return table;
	}
	
	void ProcessorRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ProcessorRowIDL struct.
	 */
	ProcessorRowIDL *ProcessorRow::toIDL() const {
		ProcessorRowIDL *x = new ProcessorRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->processorId = processorId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->type = type;
 				
 			
		
	

	
  		
		
		
			
				
		x->subType = CORBA::string_dup(subType.c_str());
				
 			
		
	

	
  		
		
		x->flagRowExists = flagRowExists;
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->almaCorrelatorModeId = almaCorrelatorModeId.toIDLTag();
			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ProcessorRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void ProcessorRow::setFromIDL (ProcessorRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setProcessorId(Tag (x.processorId));
			
 		
		
	

	
		
		
			
		setType(x.type);
  			
 		
		
	

	
		
		
			
		setSubType(string (x.subType));
			
 		
		
	

	
		
		flagRowExists = x.flagRowExists;
		if (x.flagRowExists) {
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setAlmaCorrelatorModeId(Tag (x.almaCorrelatorModeId));
			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Processor");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string ProcessorRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(processorId, "processorId", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("type", type));
		
		
	

  	
 		
		
		Parser::toXML(subType, "subType", buf);
		
		
	

  	
 		
		if (flagRowExists) {
		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(almaCorrelatorModeId, "almaCorrelatorModeId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void ProcessorRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setProcessorId(Parser::getTag("processorId","Processor",rowDoc));
			
		
	

	
		
		
		
		type = EnumerationParser::getProcessorType("type","Processor",rowDoc);
		
		
		
	

	
  		
			
	  	setSubType(Parser::getString("subType","Processor",rowDoc));
			
		
	

	
  		
        if (row.isStr("<flagRow>")) {
			
	  		setFlagRow(Parser::getBoolean("flagRow","Processor",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setAlmaCorrelatorModeId(Parser::getTag("almaCorrelatorModeId","Processor",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Processor");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get processorId.
 	 * @return processorId as Tag
 	 */
 	Tag ProcessorRow::getProcessorId() const {
	
  		return processorId;
 	}

 	/**
 	 * Set processorId with the specified Tag.
 	 * @param processorId The Tag value to which processorId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ProcessorRow::setProcessorId (Tag processorId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("processorId", "Processor");
		
  		}
  	
 		this->processorId = processorId;
	
 	}
	
	

	

	
 	/**
 	 * Get type.
 	 * @return type as ProcessorTypeMod::ProcessorType
 	 */
 	ProcessorTypeMod::ProcessorType ProcessorRow::getType() const {
	
  		return type;
 	}

 	/**
 	 * Set type with the specified ProcessorTypeMod::ProcessorType.
 	 * @param type The ProcessorTypeMod::ProcessorType value to which type is to be set.
 	 
 	
 		
 	 */
 	void ProcessorRow::setType (ProcessorTypeMod::ProcessorType type)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->type = type;
	
 	}
	
	

	

	
 	/**
 	 * Get subType.
 	 * @return subType as string
 	 */
 	string ProcessorRow::getSubType() const {
	
  		return subType;
 	}

 	/**
 	 * Set subType with the specified string.
 	 * @param subType The string value to which subType is to be set.
 	 
 	
 		
 	 */
 	void ProcessorRow::setSubType (string subType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->subType = subType;
	
 	}
	
	

	
	/**
	 * The attribute flagRow is optional. Return true if this attribute exists.
	 * @return true if and only if the flagRow attribute exists. 
	 */
	bool ProcessorRow::isFlagRowExists() const {
		return flagRowExists;
	}
	

	
 	/**
 	 * Get flagRow, which is optional.
 	 * @return flagRow as bool
 	 * @throw IllegalAccessException If flagRow does not exist.
 	 */
 	bool ProcessorRow::getFlagRow() const throw(IllegalAccessException) {
		if (!flagRowExists) {
			throw IllegalAccessException("flagRow", "Processor");
		}
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 	 */
 	void ProcessorRow::setFlagRow (bool flagRow) {
	
 		this->flagRow = flagRow;
	
		flagRowExists = true;
	
 	}
	
	
	/**
	 * Mark flagRow, which is an optional field, as non-existent.
	 */
	void ProcessorRow::clearFlagRow () {
		flagRowExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get almaCorrelatorModeId.
 	 * @return almaCorrelatorModeId as Tag
 	 */
 	Tag ProcessorRow::getAlmaCorrelatorModeId() const {
	
  		return almaCorrelatorModeId;
 	}

 	/**
 	 * Set almaCorrelatorModeId with the specified Tag.
 	 * @param almaCorrelatorModeId The Tag value to which almaCorrelatorModeId is to be set.
 	 
 	
 		
 	 */
 	void ProcessorRow::setAlmaCorrelatorModeId (Tag almaCorrelatorModeId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->almaCorrelatorModeId = almaCorrelatorModeId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CorrelatorMode table having CorrelatorMode.almaCorrelatorModeId == almaCorrelatorModeId
	 * @return a CorrelatorModeRow*
	 * 
	 
	 */
	 CorrelatorModeRow* ProcessorRow::getCorrelatorModeUsingAlmaCorrelatorModeId() {
	 
	 	return table.getContainer().getCorrelatorMode().getRowByKey(almaCorrelatorModeId);
	 }
	 

	

	
	/**
	 * Create a ProcessorRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ProcessorRow::ProcessorRow (ProcessorTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	
		flagRowExists = false;
	

	
	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
type = CProcessorType::from_int(0);
	

	

	
	
	}
	
	ProcessorRow::ProcessorRow (ProcessorTable &t, ProcessorRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	
		flagRowExists = false;
	

	
	
		
		}
		else {
	
		
			processorId = row.processorId;
		
		
		
		
			almaCorrelatorModeId = row.almaCorrelatorModeId;
		
			type = row.type;
		
			subType = row.subType;
		
		
		
		
		if (row.flagRowExists) {
			flagRow = row.flagRow;		
			flagRowExists = true;
		}
		else
			flagRowExists = false;
		
		}	
	}

	
	bool ProcessorRow::compareNoAutoInc(Tag almaCorrelatorModeId, ProcessorTypeMod::ProcessorType type, string subType) {
		bool result;
		result = true;
		
	
		
		result = result && (this->almaCorrelatorModeId == almaCorrelatorModeId);
		
		if (!result) return false;
	

	
		
		result = result && (this->type == type);
		
		if (!result) return false;
	

	
		
		result = result && (this->subType == subType);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool ProcessorRow::compareRequiredValue(Tag almaCorrelatorModeId, ProcessorTypeMod::ProcessorType type, string subType) {
		bool result;
		result = true;
		
	
		if (!(this->almaCorrelatorModeId == almaCorrelatorModeId)) return false;
	

	
		if (!(this->type == type)) return false;
	

	
		if (!(this->subType == subType)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ProcessorRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool ProcessorRow::equalByRequiredValue(ProcessorRow* x) {
		
			
		if (this->almaCorrelatorModeId != x->almaCorrelatorModeId) return false;
			
		if (this->type != x->type) return false;
			
		if (this->subType != x->subType) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
