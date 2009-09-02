
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
 * File ScanRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <ScanRow.h>
#include <ScanTable.h>

#include <ExecBlockTable.h>
#include <ExecBlockRow.h>
	

using asdm::ASDM;
using asdm::ScanRow;
using asdm::ScanTable;

using asdm::ExecBlockTable;
using asdm::ExecBlockRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	ScanRow::~ScanRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	ScanTable &ScanRow::getTable() const {
		return table;
	}
	
	void ScanRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ScanRowIDL struct.
	 */
	ScanRowIDL *ScanRow::toIDL() const {
		ScanRowIDL *x = new ScanRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->scanNumber = scanNumber;
 				
 			
		
	

	
  		
		
		
			
		x->startTime = startTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endTime = endTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->numSubScan = numSubScan;
 				
 			
		
	

	
  		
		
		
			
				
		x->numIntent = numIntent;
 				
 			
		
	

	
  		
		
		x->numFieldExists = numFieldExists;
		
		
			
				
		x->numField = numField;
 				
 			
		
	

	
  		
		
		
			
		x->scanIntent.length(scanIntent.size());
		for (unsigned int i = 0; i < scanIntent.size(); ++i) {
			
				
			x->scanIntent[i] = scanIntent.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->sourceNameExists = sourceNameExists;
		
		
			
				
		x->sourceName = CORBA::string_dup(sourceName.c_str());
				
 			
		
	

	
  		
		
		x->fieldNameExists = fieldNameExists;
		
		
			
		x->fieldName.length(fieldName.size());
		for (unsigned int i = 0; i < fieldName.size(); ++i) {
			
				
			x->fieldName[i] = CORBA::string_dup(fieldName.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->execBlockId = execBlockId.toIDLTag();
			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ScanRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void ScanRow::setFromIDL (ScanRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setScanNumber(x.scanNumber);
  			
 		
		
	

	
		
		
			
		setStartTime(ArrayTime (x.startTime));
			
 		
		
	

	
		
		
			
		setEndTime(ArrayTime (x.endTime));
			
 		
		
	

	
		
		
			
		setNumSubScan(x.numSubScan);
  			
 		
		
	

	
		
		
			
		setNumIntent(x.numIntent);
  			
 		
		
	

	
		
		numFieldExists = x.numFieldExists;
		if (x.numFieldExists) {
		
		
			
		setNumField(x.numField);
  			
 		
		
		}
		
	

	
		
		
			
		scanIntent .clear();
		for (unsigned int i = 0; i <x.scanIntent.length(); ++i) {
			
			scanIntent.push_back(x.scanIntent[i]);
  			
		}
			
  		
		
	

	
		
		sourceNameExists = x.sourceNameExists;
		if (x.sourceNameExists) {
		
		
			
		setSourceName(string (x.sourceName));
			
 		
		
		}
		
	

	
		
		fieldNameExists = x.fieldNameExists;
		if (x.fieldNameExists) {
		
		
			
		fieldName .clear();
		for (unsigned int i = 0; i <x.fieldName.length(); ++i) {
			
			fieldName.push_back(string (x.fieldName[i]));
			
		}
			
  		
		
		}
		
	

	
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
	

	
	
		
	
		
		
			
		setExecBlockId(Tag (x.execBlockId));
			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Scan");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string ScanRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(scanNumber, "scanNumber", buf);
		
		
	

  	
 		
		
		Parser::toXML(startTime, "startTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endTime, "endTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(numSubScan, "numSubScan", buf);
		
		
	

  	
 		
		
		Parser::toXML(numIntent, "numIntent", buf);
		
		
	

  	
 		
		if (numFieldExists) {
		
		
		Parser::toXML(numField, "numField", buf);
		
		
		}
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("scanIntent", scanIntent));
		
		
	

  	
 		
		if (sourceNameExists) {
		
		
		Parser::toXML(sourceName, "sourceName", buf);
		
		
		}
		
	

  	
 		
		if (fieldNameExists) {
		
		
		Parser::toXML(fieldName, "fieldName", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(execBlockId, "execBlockId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void ScanRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setScanNumber(Parser::getInteger("scanNumber","Scan",rowDoc));
			
		
	

	
  		
			
	  	setStartTime(Parser::getArrayTime("startTime","Scan",rowDoc));
			
		
	

	
  		
			
	  	setEndTime(Parser::getArrayTime("endTime","Scan",rowDoc));
			
		
	

	
  		
			
	  	setNumSubScan(Parser::getInteger("numSubScan","Scan",rowDoc));
			
		
	

	
  		
			
	  	setNumIntent(Parser::getInteger("numIntent","Scan",rowDoc));
			
		
	

	
  		
        if (row.isStr("<numField>")) {
			
	  		setNumField(Parser::getInteger("numField","Scan",rowDoc));
			
		}
 		
	

	
		
		
		
		scanIntent = EnumerationParser::getScanIntent1D("scanIntent","Scan",rowDoc);			
		
		
		
	

	
  		
        if (row.isStr("<sourceName>")) {
			
	  		setSourceName(Parser::getString("sourceName","Scan",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<fieldName>")) {
			
								
	  		setFieldName(Parser::get1DString("fieldName","Scan",rowDoc));
	  			
	  		
		}
 		
	

	
  		
			
	  	setFlagRow(Parser::getBoolean("flagRow","Scan",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setExecBlockId(Parser::getTag("execBlockId","ExecBlock",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Scan");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get scanNumber.
 	 * @return scanNumber as int
 	 */
 	int ScanRow::getScanNumber() const {
	
  		return scanNumber;
 	}

 	/**
 	 * Set scanNumber with the specified int.
 	 * @param scanNumber The int value to which scanNumber is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ScanRow::setScanNumber (int scanNumber)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("scanNumber", "Scan");
		
  		}
  	
 		this->scanNumber = scanNumber;
	
 	}
	
	

	

	
 	/**
 	 * Get startTime.
 	 * @return startTime as ArrayTime
 	 */
 	ArrayTime ScanRow::getStartTime() const {
	
  		return startTime;
 	}

 	/**
 	 * Set startTime with the specified ArrayTime.
 	 * @param startTime The ArrayTime value to which startTime is to be set.
 	 
 	
 		
 	 */
 	void ScanRow::setStartTime (ArrayTime startTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startTime = startTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endTime.
 	 * @return endTime as ArrayTime
 	 */
 	ArrayTime ScanRow::getEndTime() const {
	
  		return endTime;
 	}

 	/**
 	 * Set endTime with the specified ArrayTime.
 	 * @param endTime The ArrayTime value to which endTime is to be set.
 	 
 	
 		
 	 */
 	void ScanRow::setEndTime (ArrayTime endTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endTime = endTime;
	
 	}
	
	

	

	
 	/**
 	 * Get numSubScan.
 	 * @return numSubScan as int
 	 */
 	int ScanRow::getNumSubScan() const {
	
  		return numSubScan;
 	}

 	/**
 	 * Set numSubScan with the specified int.
 	 * @param numSubScan The int value to which numSubScan is to be set.
 	 
 	
 		
 	 */
 	void ScanRow::setNumSubScan (int numSubScan)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numSubScan = numSubScan;
	
 	}
	
	

	

	
 	/**
 	 * Get numIntent.
 	 * @return numIntent as int
 	 */
 	int ScanRow::getNumIntent() const {
	
  		return numIntent;
 	}

 	/**
 	 * Set numIntent with the specified int.
 	 * @param numIntent The int value to which numIntent is to be set.
 	 
 	
 		
 	 */
 	void ScanRow::setNumIntent (int numIntent)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numIntent = numIntent;
	
 	}
	
	

	
	/**
	 * The attribute numField is optional. Return true if this attribute exists.
	 * @return true if and only if the numField attribute exists. 
	 */
	bool ScanRow::isNumFieldExists() const {
		return numFieldExists;
	}
	

	
 	/**
 	 * Get numField, which is optional.
 	 * @return numField as int
 	 * @throw IllegalAccessException If numField does not exist.
 	 */
 	int ScanRow::getNumField() const throw(IllegalAccessException) {
		if (!numFieldExists) {
			throw IllegalAccessException("numField", "Scan");
		}
	
  		return numField;
 	}

 	/**
 	 * Set numField with the specified int.
 	 * @param numField The int value to which numField is to be set.
 	 
 	
 	 */
 	void ScanRow::setNumField (int numField) {
	
 		this->numField = numField;
	
		numFieldExists = true;
	
 	}
	
	
	/**
	 * Mark numField, which is an optional field, as non-existent.
	 */
	void ScanRow::clearNumField () {
		numFieldExists = false;
	}
	

	

	
 	/**
 	 * Get scanIntent.
 	 * @return scanIntent as vector<ScanIntentMod::ScanIntent >
 	 */
 	vector<ScanIntentMod::ScanIntent > ScanRow::getScanIntent() const {
	
  		return scanIntent;
 	}

 	/**
 	 * Set scanIntent with the specified vector<ScanIntentMod::ScanIntent >.
 	 * @param scanIntent The vector<ScanIntentMod::ScanIntent > value to which scanIntent is to be set.
 	 
 	
 		
 	 */
 	void ScanRow::setScanIntent (vector<ScanIntentMod::ScanIntent > scanIntent)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->scanIntent = scanIntent;
	
 	}
	
	

	
	/**
	 * The attribute sourceName is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceName attribute exists. 
	 */
	bool ScanRow::isSourceNameExists() const {
		return sourceNameExists;
	}
	

	
 	/**
 	 * Get sourceName, which is optional.
 	 * @return sourceName as string
 	 * @throw IllegalAccessException If sourceName does not exist.
 	 */
 	string ScanRow::getSourceName() const throw(IllegalAccessException) {
		if (!sourceNameExists) {
			throw IllegalAccessException("sourceName", "Scan");
		}
	
  		return sourceName;
 	}

 	/**
 	 * Set sourceName with the specified string.
 	 * @param sourceName The string value to which sourceName is to be set.
 	 
 	
 	 */
 	void ScanRow::setSourceName (string sourceName) {
	
 		this->sourceName = sourceName;
	
		sourceNameExists = true;
	
 	}
	
	
	/**
	 * Mark sourceName, which is an optional field, as non-existent.
	 */
	void ScanRow::clearSourceName () {
		sourceNameExists = false;
	}
	

	
	/**
	 * The attribute fieldName is optional. Return true if this attribute exists.
	 * @return true if and only if the fieldName attribute exists. 
	 */
	bool ScanRow::isFieldNameExists() const {
		return fieldNameExists;
	}
	

	
 	/**
 	 * Get fieldName, which is optional.
 	 * @return fieldName as vector<string >
 	 * @throw IllegalAccessException If fieldName does not exist.
 	 */
 	vector<string > ScanRow::getFieldName() const throw(IllegalAccessException) {
		if (!fieldNameExists) {
			throw IllegalAccessException("fieldName", "Scan");
		}
	
  		return fieldName;
 	}

 	/**
 	 * Set fieldName with the specified vector<string >.
 	 * @param fieldName The vector<string > value to which fieldName is to be set.
 	 
 	
 	 */
 	void ScanRow::setFieldName (vector<string > fieldName) {
	
 		this->fieldName = fieldName;
	
		fieldNameExists = true;
	
 	}
	
	
	/**
	 * Mark fieldName, which is an optional field, as non-existent.
	 */
	void ScanRow::clearFieldName () {
		fieldNameExists = false;
	}
	

	

	
 	/**
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool ScanRow::getFlagRow() const {
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 		
 	 */
 	void ScanRow::setFlagRow (bool flagRow)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagRow = flagRow;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag ScanRow::getExecBlockId() const {
	
  		return execBlockId;
 	}

 	/**
 	 * Set execBlockId with the specified Tag.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ScanRow::setExecBlockId (Tag execBlockId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("execBlockId", "Scan");
		
  		}
  	
 		this->execBlockId = execBlockId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the ExecBlock table having ExecBlock.execBlockId == execBlockId
	 * @return a ExecBlockRow*
	 * 
	 
	 */
	 ExecBlockRow* ScanRow::getExecBlockUsingExecBlockId() {
	 
	 	return table.getContainer().getExecBlock().getRowByKey(execBlockId);
	 }
	 

	

	
	/**
	 * Create a ScanRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ScanRow::ScanRow (ScanTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	
		numFieldExists = false;
	

	

	
		sourceNameExists = false;
	

	
		fieldNameExists = false;
	

	

	
	

	
	
	
	

	

	

	

	

	

	

	

	

	
	
	}
	
	ScanRow::ScanRow (ScanTable &t, ScanRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	
		numFieldExists = false;
	

	

	
		sourceNameExists = false;
	

	
		fieldNameExists = false;
	

	

	
	
		
		}
		else {
	
		
			execBlockId = row.execBlockId;
		
			scanNumber = row.scanNumber;
		
		
		
		
			startTime = row.startTime;
		
			endTime = row.endTime;
		
			numSubScan = row.numSubScan;
		
			numIntent = row.numIntent;
		
			scanIntent = row.scanIntent;
		
			flagRow = row.flagRow;
		
		
		
		
		if (row.numFieldExists) {
			numField = row.numField;		
			numFieldExists = true;
		}
		else
			numFieldExists = false;
		
		if (row.sourceNameExists) {
			sourceName = row.sourceName;		
			sourceNameExists = true;
		}
		else
			sourceNameExists = false;
		
		if (row.fieldNameExists) {
			fieldName = row.fieldName;		
			fieldNameExists = true;
		}
		else
			fieldNameExists = false;
		
		}	
	}

	
	bool ScanRow::compareNoAutoInc(Tag execBlockId, int scanNumber, ArrayTime startTime, ArrayTime endTime, int numSubScan, int numIntent, vector<ScanIntentMod::ScanIntent > scanIntent, bool flagRow) {
		bool result;
		result = true;
		
	
		
		result = result && (this->execBlockId == execBlockId);
		
		if (!result) return false;
	

	
		
		result = result && (this->scanNumber == scanNumber);
		
		if (!result) return false;
	

	
		
		result = result && (this->startTime == startTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endTime == endTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->numSubScan == numSubScan);
		
		if (!result) return false;
	

	
		
		result = result && (this->numIntent == numIntent);
		
		if (!result) return false;
	

	
		
		result = result && (this->scanIntent == scanIntent);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagRow == flagRow);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool ScanRow::compareRequiredValue(ArrayTime startTime, ArrayTime endTime, int numSubScan, int numIntent, vector<ScanIntentMod::ScanIntent > scanIntent, bool flagRow) {
		bool result;
		result = true;
		
	
		if (!(this->startTime == startTime)) return false;
	

	
		if (!(this->endTime == endTime)) return false;
	

	
		if (!(this->numSubScan == numSubScan)) return false;
	

	
		if (!(this->numIntent == numIntent)) return false;
	

	
		if (!(this->scanIntent == scanIntent)) return false;
	

	
		if (!(this->flagRow == flagRow)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ScanRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool ScanRow::equalByRequiredValue(ScanRow* x) {
		
			
		if (this->startTime != x->startTime) return false;
			
		if (this->endTime != x->endTime) return false;
			
		if (this->numSubScan != x->numSubScan) return false;
			
		if (this->numIntent != x->numIntent) return false;
			
		if (this->scanIntent != x->scanIntent) return false;
			
		if (this->flagRow != x->flagRow) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
