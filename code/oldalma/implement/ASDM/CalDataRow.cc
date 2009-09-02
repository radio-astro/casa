
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
 * File CalDataRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalDataRow.h>
#include <CalDataTable.h>
	

using asdm::ASDM;
using asdm::CalDataRow;
using asdm::CalDataTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	CalDataRow::~CalDataRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalDataTable &CalDataRow::getTable() const {
		return table;
	}
	
	void CalDataRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalDataRowIDL struct.
	 */
	CalDataRowIDL *CalDataRow::toIDL() const {
		CalDataRowIDL *x = new CalDataRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->calDataId = calDataId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->numScan = numScan;
 				
 			
		
	

	
  		
		
		x->frequencyGroupExists = frequencyGroupExists;
		
		
			
				
		x->frequencyGroup = frequencyGroup;
 				
 			
		
	

	
  		
		
		
			
		x->scanSet.length(scanSet.size());
		for (unsigned int i = 0; i < scanSet.size(); ++i) {
			
				
			x->scanSet[i] = scanSet.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->calType = calType;
 				
 			
		
	

	
  		
		
		x->freqGroupNameExists = freqGroupNameExists;
		
		
			
				
		x->freqGroupName = CORBA::string_dup(freqGroupName.c_str());
				
 			
		
	

	
  		
		
		x->fieldNameExists = fieldNameExists;
		
		
			
				
		x->fieldName = CORBA::string_dup(fieldName.c_str());
				
 			
		
	

	
  		
		
		x->fieldCodeExists = fieldCodeExists;
		
		
			
		x->fieldCode.length(fieldCode.size());
		for (unsigned int i = 0; i < fieldCode.size(); ++i) {
			
				
			x->fieldCode[i] = CORBA::string_dup(fieldCode.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->startTimeObserved = startTimeObserved.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endTimeObserved = endTimeObserved.toIDLArrayTime();
			
		
	

	
  		
		
		x->sourceNameExists = sourceNameExists;
		
		
			
		x->sourceName.length(sourceName.size());
		for (unsigned int i = 0; i < sourceName.size(); ++i) {
			
				
			x->sourceName[i] = CORBA::string_dup(sourceName.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		x->sourceCodeExists = sourceCodeExists;
		
		
			
		x->sourceCode.length(sourceCode.size());
		for (unsigned int i = 0; i < sourceCode.size(); ++i) {
			
				
			x->sourceCode[i] = CORBA::string_dup(sourceCode.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		x->scanIntentExists = scanIntentExists;
		
		
			
		x->scanIntent.length(scanIntent.size());
		for (unsigned int i = 0; i < scanIntent.size(); ++i) {
			
				
			x->scanIntent[i] = scanIntent.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->assocCalDataIdExists = assocCalDataIdExists;
		
		
			
		x->assocCalDataId = assocCalDataId.toIDLTag();
			
		
	

	
  		
		
		x->assocCalNatureExists = assocCalNatureExists;
		
		
			
				
		x->assocCalNature = assocCalNature;
 				
 			
		
	

	
  		
		
		
			
				
		x->calDataType = calDataType;
 				
 			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalDataRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalDataRow::setFromIDL (CalDataRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setNumScan(x.numScan);
  			
 		
		
	

	
		
		frequencyGroupExists = x.frequencyGroupExists;
		if (x.frequencyGroupExists) {
		
		
			
		setFrequencyGroup(x.frequencyGroup);
  			
 		
		
		}
		
	

	
		
		
			
		scanSet .clear();
		for (unsigned int i = 0; i <x.scanSet.length(); ++i) {
			
			scanSet.push_back(x.scanSet[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setCalType(x.calType);
  			
 		
		
	

	
		
		freqGroupNameExists = x.freqGroupNameExists;
		if (x.freqGroupNameExists) {
		
		
			
		setFreqGroupName(string (x.freqGroupName));
			
 		
		
		}
		
	

	
		
		fieldNameExists = x.fieldNameExists;
		if (x.fieldNameExists) {
		
		
			
		setFieldName(string (x.fieldName));
			
 		
		
		}
		
	

	
		
		fieldCodeExists = x.fieldCodeExists;
		if (x.fieldCodeExists) {
		
		
			
		fieldCode .clear();
		for (unsigned int i = 0; i <x.fieldCode.length(); ++i) {
			
			fieldCode.push_back(string (x.fieldCode[i]));
			
		}
			
  		
		
		}
		
	

	
		
		
			
		setStartTimeObserved(ArrayTime (x.startTimeObserved));
			
 		
		
	

	
		
		
			
		setEndTimeObserved(ArrayTime (x.endTimeObserved));
			
 		
		
	

	
		
		sourceNameExists = x.sourceNameExists;
		if (x.sourceNameExists) {
		
		
			
		sourceName .clear();
		for (unsigned int i = 0; i <x.sourceName.length(); ++i) {
			
			sourceName.push_back(string (x.sourceName[i]));
			
		}
			
  		
		
		}
		
	

	
		
		sourceCodeExists = x.sourceCodeExists;
		if (x.sourceCodeExists) {
		
		
			
		sourceCode .clear();
		for (unsigned int i = 0; i <x.sourceCode.length(); ++i) {
			
			sourceCode.push_back(string (x.sourceCode[i]));
			
		}
			
  		
		
		}
		
	

	
		
		scanIntentExists = x.scanIntentExists;
		if (x.scanIntentExists) {
		
		
			
		scanIntent .clear();
		for (unsigned int i = 0; i <x.scanIntent.length(); ++i) {
			
			scanIntent.push_back(x.scanIntent[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		assocCalDataIdExists = x.assocCalDataIdExists;
		if (x.assocCalDataIdExists) {
		
		
			
		setAssocCalDataId(Tag (x.assocCalDataId));
			
 		
		
		}
		
	

	
		
		assocCalNatureExists = x.assocCalNatureExists;
		if (x.assocCalNatureExists) {
		
		
			
		setAssocCalNature(x.assocCalNature);
  			
 		
		
		}
		
	

	
		
		
			
		setCalDataType(x.calDataType);
  			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalData");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalDataRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(calDataId, "calDataId", buf);
		
		
	

  	
 		
		
		Parser::toXML(numScan, "numScan", buf);
		
		
	

  	
 		
		if (frequencyGroupExists) {
		
		
		Parser::toXML(frequencyGroup, "frequencyGroup", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(scanSet, "scanSet", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("calType", calType));
		
		
	

  	
 		
		if (freqGroupNameExists) {
		
		
		Parser::toXML(freqGroupName, "freqGroupName", buf);
		
		
		}
		
	

  	
 		
		if (fieldNameExists) {
		
		
		Parser::toXML(fieldName, "fieldName", buf);
		
		
		}
		
	

  	
 		
		if (fieldCodeExists) {
		
		
		Parser::toXML(fieldCode, "fieldCode", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(startTimeObserved, "startTimeObserved", buf);
		
		
	

  	
 		
		
		Parser::toXML(endTimeObserved, "endTimeObserved", buf);
		
		
	

  	
 		
		if (sourceNameExists) {
		
		
		Parser::toXML(sourceName, "sourceName", buf);
		
		
		}
		
	

  	
 		
		if (sourceCodeExists) {
		
		
		Parser::toXML(sourceCode, "sourceCode", buf);
		
		
		}
		
	

  	
 		
		if (scanIntentExists) {
		
		
			buf.append(EnumerationParser::toXML("scanIntent", scanIntent));
		
		
		}
		
	

  	
 		
		if (assocCalDataIdExists) {
		
		
		Parser::toXML(assocCalDataId, "assocCalDataId", buf);
		
		
		}
		
	

  	
 		
		if (assocCalNatureExists) {
		
		
			buf.append(EnumerationParser::toXML("assocCalNature", assocCalNature));
		
		
		}
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("calDataType", calDataType));
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CalDataRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setNumScan(Parser::getInteger("numScan","CalData",rowDoc));
			
		
	

	
  		
        if (row.isStr("<frequencyGroup>")) {
			
	  		setFrequencyGroup(Parser::getInteger("frequencyGroup","CalData",rowDoc));
			
		}
 		
	

	
  		
			
					
	  	setScanSet(Parser::get1DInteger("scanSet","CalData",rowDoc));
	  			
	  		
		
	

	
		
		
		
		calType = EnumerationParser::getCalType("calType","CalData",rowDoc);
		
		
		
	

	
  		
        if (row.isStr("<freqGroupName>")) {
			
	  		setFreqGroupName(Parser::getString("freqGroupName","CalData",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<fieldName>")) {
			
	  		setFieldName(Parser::getString("fieldName","CalData",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<fieldCode>")) {
			
								
	  		setFieldCode(Parser::get1DString("fieldCode","CalData",rowDoc));
	  			
	  		
		}
 		
	

	
  		
			
	  	setStartTimeObserved(Parser::getArrayTime("startTimeObserved","CalData",rowDoc));
			
		
	

	
  		
			
	  	setEndTimeObserved(Parser::getArrayTime("endTimeObserved","CalData",rowDoc));
			
		
	

	
  		
        if (row.isStr("<sourceName>")) {
			
								
	  		setSourceName(Parser::get1DString("sourceName","CalData",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<sourceCode>")) {
			
								
	  		setSourceCode(Parser::get1DString("sourceCode","CalData",rowDoc));
	  			
	  		
		}
 		
	

	
		
	if (row.isStr("<scanIntent>")) {
		
		
		
		scanIntent = EnumerationParser::getScanIntent1D("scanIntent","CalData",rowDoc);			
		
		
		
		scanIntentExists = true;
	}
		
	

	
  		
        if (row.isStr("<assocCalDataId>")) {
			
	  		setAssocCalDataId(Parser::getTag("assocCalDataId","CalData",rowDoc));
			
		}
 		
	

	
		
	if (row.isStr("<assocCalNature>")) {
		
		
		
		assocCalNature = EnumerationParser::getAssociatedCalNature("assocCalNature","CalData",rowDoc);
		
		
		
		assocCalNatureExists = true;
	}
		
	

	
		
		
		
		calDataType = EnumerationParser::getCalDataOrigin("calDataType","CalData",rowDoc);
		
		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalData");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalDataRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDataRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalData");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get numScan.
 	 * @return numScan as int
 	 */
 	int CalDataRow::getNumScan() const {
	
  		return numScan;
 	}

 	/**
 	 * Set numScan with the specified int.
 	 * @param numScan The int value to which numScan is to be set.
 	 
 	
 		
 	 */
 	void CalDataRow::setNumScan (int numScan)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numScan = numScan;
	
 	}
	
	

	
	/**
	 * The attribute frequencyGroup is optional. Return true if this attribute exists.
	 * @return true if and only if the frequencyGroup attribute exists. 
	 */
	bool CalDataRow::isFrequencyGroupExists() const {
		return frequencyGroupExists;
	}
	

	
 	/**
 	 * Get frequencyGroup, which is optional.
 	 * @return frequencyGroup as int
 	 * @throw IllegalAccessException If frequencyGroup does not exist.
 	 */
 	int CalDataRow::getFrequencyGroup() const throw(IllegalAccessException) {
		if (!frequencyGroupExists) {
			throw IllegalAccessException("frequencyGroup", "CalData");
		}
	
  		return frequencyGroup;
 	}

 	/**
 	 * Set frequencyGroup with the specified int.
 	 * @param frequencyGroup The int value to which frequencyGroup is to be set.
 	 
 	
 	 */
 	void CalDataRow::setFrequencyGroup (int frequencyGroup) {
	
 		this->frequencyGroup = frequencyGroup;
	
		frequencyGroupExists = true;
	
 	}
	
	
	/**
	 * Mark frequencyGroup, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearFrequencyGroup () {
		frequencyGroupExists = false;
	}
	

	

	
 	/**
 	 * Get scanSet.
 	 * @return scanSet as vector<int >
 	 */
 	vector<int > CalDataRow::getScanSet() const {
	
  		return scanSet;
 	}

 	/**
 	 * Set scanSet with the specified vector<int >.
 	 * @param scanSet The vector<int > value to which scanSet is to be set.
 	 
 	
 		
 	 */
 	void CalDataRow::setScanSet (vector<int > scanSet)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->scanSet = scanSet;
	
 	}
	
	

	

	
 	/**
 	 * Get calType.
 	 * @return calType as CalTypeMod::CalType
 	 */
 	CalTypeMod::CalType CalDataRow::getCalType() const {
	
  		return calType;
 	}

 	/**
 	 * Set calType with the specified CalTypeMod::CalType.
 	 * @param calType The CalTypeMod::CalType value to which calType is to be set.
 	 
 	
 		
 	 */
 	void CalDataRow::setCalType (CalTypeMod::CalType calType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->calType = calType;
	
 	}
	
	

	
	/**
	 * The attribute freqGroupName is optional. Return true if this attribute exists.
	 * @return true if and only if the freqGroupName attribute exists. 
	 */
	bool CalDataRow::isFreqGroupNameExists() const {
		return freqGroupNameExists;
	}
	

	
 	/**
 	 * Get freqGroupName, which is optional.
 	 * @return freqGroupName as string
 	 * @throw IllegalAccessException If freqGroupName does not exist.
 	 */
 	string CalDataRow::getFreqGroupName() const throw(IllegalAccessException) {
		if (!freqGroupNameExists) {
			throw IllegalAccessException("freqGroupName", "CalData");
		}
	
  		return freqGroupName;
 	}

 	/**
 	 * Set freqGroupName with the specified string.
 	 * @param freqGroupName The string value to which freqGroupName is to be set.
 	 
 	
 	 */
 	void CalDataRow::setFreqGroupName (string freqGroupName) {
	
 		this->freqGroupName = freqGroupName;
	
		freqGroupNameExists = true;
	
 	}
	
	
	/**
	 * Mark freqGroupName, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearFreqGroupName () {
		freqGroupNameExists = false;
	}
	

	
	/**
	 * The attribute fieldName is optional. Return true if this attribute exists.
	 * @return true if and only if the fieldName attribute exists. 
	 */
	bool CalDataRow::isFieldNameExists() const {
		return fieldNameExists;
	}
	

	
 	/**
 	 * Get fieldName, which is optional.
 	 * @return fieldName as string
 	 * @throw IllegalAccessException If fieldName does not exist.
 	 */
 	string CalDataRow::getFieldName() const throw(IllegalAccessException) {
		if (!fieldNameExists) {
			throw IllegalAccessException("fieldName", "CalData");
		}
	
  		return fieldName;
 	}

 	/**
 	 * Set fieldName with the specified string.
 	 * @param fieldName The string value to which fieldName is to be set.
 	 
 	
 	 */
 	void CalDataRow::setFieldName (string fieldName) {
	
 		this->fieldName = fieldName;
	
		fieldNameExists = true;
	
 	}
	
	
	/**
	 * Mark fieldName, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearFieldName () {
		fieldNameExists = false;
	}
	

	
	/**
	 * The attribute fieldCode is optional. Return true if this attribute exists.
	 * @return true if and only if the fieldCode attribute exists. 
	 */
	bool CalDataRow::isFieldCodeExists() const {
		return fieldCodeExists;
	}
	

	
 	/**
 	 * Get fieldCode, which is optional.
 	 * @return fieldCode as vector<string >
 	 * @throw IllegalAccessException If fieldCode does not exist.
 	 */
 	vector<string > CalDataRow::getFieldCode() const throw(IllegalAccessException) {
		if (!fieldCodeExists) {
			throw IllegalAccessException("fieldCode", "CalData");
		}
	
  		return fieldCode;
 	}

 	/**
 	 * Set fieldCode with the specified vector<string >.
 	 * @param fieldCode The vector<string > value to which fieldCode is to be set.
 	 
 	
 	 */
 	void CalDataRow::setFieldCode (vector<string > fieldCode) {
	
 		this->fieldCode = fieldCode;
	
		fieldCodeExists = true;
	
 	}
	
	
	/**
	 * Mark fieldCode, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearFieldCode () {
		fieldCodeExists = false;
	}
	

	

	
 	/**
 	 * Get startTimeObserved.
 	 * @return startTimeObserved as ArrayTime
 	 */
 	ArrayTime CalDataRow::getStartTimeObserved() const {
	
  		return startTimeObserved;
 	}

 	/**
 	 * Set startTimeObserved with the specified ArrayTime.
 	 * @param startTimeObserved The ArrayTime value to which startTimeObserved is to be set.
 	 
 	
 		
 	 */
 	void CalDataRow::setStartTimeObserved (ArrayTime startTimeObserved)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startTimeObserved = startTimeObserved;
	
 	}
	
	

	

	
 	/**
 	 * Get endTimeObserved.
 	 * @return endTimeObserved as ArrayTime
 	 */
 	ArrayTime CalDataRow::getEndTimeObserved() const {
	
  		return endTimeObserved;
 	}

 	/**
 	 * Set endTimeObserved with the specified ArrayTime.
 	 * @param endTimeObserved The ArrayTime value to which endTimeObserved is to be set.
 	 
 	
 		
 	 */
 	void CalDataRow::setEndTimeObserved (ArrayTime endTimeObserved)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endTimeObserved = endTimeObserved;
	
 	}
	
	

	
	/**
	 * The attribute sourceName is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceName attribute exists. 
	 */
	bool CalDataRow::isSourceNameExists() const {
		return sourceNameExists;
	}
	

	
 	/**
 	 * Get sourceName, which is optional.
 	 * @return sourceName as vector<string >
 	 * @throw IllegalAccessException If sourceName does not exist.
 	 */
 	vector<string > CalDataRow::getSourceName() const throw(IllegalAccessException) {
		if (!sourceNameExists) {
			throw IllegalAccessException("sourceName", "CalData");
		}
	
  		return sourceName;
 	}

 	/**
 	 * Set sourceName with the specified vector<string >.
 	 * @param sourceName The vector<string > value to which sourceName is to be set.
 	 
 	
 	 */
 	void CalDataRow::setSourceName (vector<string > sourceName) {
	
 		this->sourceName = sourceName;
	
		sourceNameExists = true;
	
 	}
	
	
	/**
	 * Mark sourceName, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearSourceName () {
		sourceNameExists = false;
	}
	

	
	/**
	 * The attribute sourceCode is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceCode attribute exists. 
	 */
	bool CalDataRow::isSourceCodeExists() const {
		return sourceCodeExists;
	}
	

	
 	/**
 	 * Get sourceCode, which is optional.
 	 * @return sourceCode as vector<string >
 	 * @throw IllegalAccessException If sourceCode does not exist.
 	 */
 	vector<string > CalDataRow::getSourceCode() const throw(IllegalAccessException) {
		if (!sourceCodeExists) {
			throw IllegalAccessException("sourceCode", "CalData");
		}
	
  		return sourceCode;
 	}

 	/**
 	 * Set sourceCode with the specified vector<string >.
 	 * @param sourceCode The vector<string > value to which sourceCode is to be set.
 	 
 	
 	 */
 	void CalDataRow::setSourceCode (vector<string > sourceCode) {
	
 		this->sourceCode = sourceCode;
	
		sourceCodeExists = true;
	
 	}
	
	
	/**
	 * Mark sourceCode, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearSourceCode () {
		sourceCodeExists = false;
	}
	

	
	/**
	 * The attribute scanIntent is optional. Return true if this attribute exists.
	 * @return true if and only if the scanIntent attribute exists. 
	 */
	bool CalDataRow::isScanIntentExists() const {
		return scanIntentExists;
	}
	

	
 	/**
 	 * Get scanIntent, which is optional.
 	 * @return scanIntent as vector<ScanIntentMod::ScanIntent >
 	 * @throw IllegalAccessException If scanIntent does not exist.
 	 */
 	vector<ScanIntentMod::ScanIntent > CalDataRow::getScanIntent() const throw(IllegalAccessException) {
		if (!scanIntentExists) {
			throw IllegalAccessException("scanIntent", "CalData");
		}
	
  		return scanIntent;
 	}

 	/**
 	 * Set scanIntent with the specified vector<ScanIntentMod::ScanIntent >.
 	 * @param scanIntent The vector<ScanIntentMod::ScanIntent > value to which scanIntent is to be set.
 	 
 	
 	 */
 	void CalDataRow::setScanIntent (vector<ScanIntentMod::ScanIntent > scanIntent) {
	
 		this->scanIntent = scanIntent;
	
		scanIntentExists = true;
	
 	}
	
	
	/**
	 * Mark scanIntent, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearScanIntent () {
		scanIntentExists = false;
	}
	

	
	/**
	 * The attribute assocCalDataId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocCalDataId attribute exists. 
	 */
	bool CalDataRow::isAssocCalDataIdExists() const {
		return assocCalDataIdExists;
	}
	

	
 	/**
 	 * Get assocCalDataId, which is optional.
 	 * @return assocCalDataId as Tag
 	 * @throw IllegalAccessException If assocCalDataId does not exist.
 	 */
 	Tag CalDataRow::getAssocCalDataId() const throw(IllegalAccessException) {
		if (!assocCalDataIdExists) {
			throw IllegalAccessException("assocCalDataId", "CalData");
		}
	
  		return assocCalDataId;
 	}

 	/**
 	 * Set assocCalDataId with the specified Tag.
 	 * @param assocCalDataId The Tag value to which assocCalDataId is to be set.
 	 
 	
 	 */
 	void CalDataRow::setAssocCalDataId (Tag assocCalDataId) {
	
 		this->assocCalDataId = assocCalDataId;
	
		assocCalDataIdExists = true;
	
 	}
	
	
	/**
	 * Mark assocCalDataId, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearAssocCalDataId () {
		assocCalDataIdExists = false;
	}
	

	
	/**
	 * The attribute assocCalNature is optional. Return true if this attribute exists.
	 * @return true if and only if the assocCalNature attribute exists. 
	 */
	bool CalDataRow::isAssocCalNatureExists() const {
		return assocCalNatureExists;
	}
	

	
 	/**
 	 * Get assocCalNature, which is optional.
 	 * @return assocCalNature as AssociatedCalNatureMod::AssociatedCalNature
 	 * @throw IllegalAccessException If assocCalNature does not exist.
 	 */
 	AssociatedCalNatureMod::AssociatedCalNature CalDataRow::getAssocCalNature() const throw(IllegalAccessException) {
		if (!assocCalNatureExists) {
			throw IllegalAccessException("assocCalNature", "CalData");
		}
	
  		return assocCalNature;
 	}

 	/**
 	 * Set assocCalNature with the specified AssociatedCalNatureMod::AssociatedCalNature.
 	 * @param assocCalNature The AssociatedCalNatureMod::AssociatedCalNature value to which assocCalNature is to be set.
 	 
 	
 	 */
 	void CalDataRow::setAssocCalNature (AssociatedCalNatureMod::AssociatedCalNature assocCalNature) {
	
 		this->assocCalNature = assocCalNature;
	
		assocCalNatureExists = true;
	
 	}
	
	
	/**
	 * Mark assocCalNature, which is an optional field, as non-existent.
	 */
	void CalDataRow::clearAssocCalNature () {
		assocCalNatureExists = false;
	}
	

	

	
 	/**
 	 * Get calDataType.
 	 * @return calDataType as CalDataOriginMod::CalDataOrigin
 	 */
 	CalDataOriginMod::CalDataOrigin CalDataRow::getCalDataType() const {
	
  		return calDataType;
 	}

 	/**
 	 * Set calDataType with the specified CalDataOriginMod::CalDataOrigin.
 	 * @param calDataType The CalDataOriginMod::CalDataOrigin value to which calDataType is to be set.
 	 
 	
 		
 	 */
 	void CalDataRow::setCalDataType (CalDataOriginMod::CalDataOrigin calDataType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->calDataType = calDataType;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a CalDataRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalDataRow::CalDataRow (CalDataTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	
		frequencyGroupExists = false;
	

	

	

	
		freqGroupNameExists = false;
	

	
		fieldNameExists = false;
	

	
		fieldCodeExists = false;
	

	

	

	
		sourceNameExists = false;
	

	
		sourceCodeExists = false;
	

	
		scanIntentExists = false;
	

	
		assocCalDataIdExists = false;
	

	
		assocCalNatureExists = false;
	

	

	
	
	
	
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
calType = CCalType::from_int(0);
	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
assocCalNature = CAssociatedCalNature::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
calDataType = CCalDataOrigin::from_int(0);
	
	
	}
	
	CalDataRow::CalDataRow (CalDataTable &t, CalDataRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	
		frequencyGroupExists = false;
	

	

	

	
		freqGroupNameExists = false;
	

	
		fieldNameExists = false;
	

	
		fieldCodeExists = false;
	

	

	

	
		sourceNameExists = false;
	

	
		sourceCodeExists = false;
	

	
		scanIntentExists = false;
	

	
		assocCalDataIdExists = false;
	

	
		assocCalNatureExists = false;
	

	

			
		}
		else {
	
		
			calDataId = row.calDataId;
		
		
		
		
			numScan = row.numScan;
		
			scanSet = row.scanSet;
		
			calType = row.calType;
		
			startTimeObserved = row.startTimeObserved;
		
			endTimeObserved = row.endTimeObserved;
		
			calDataType = row.calDataType;
		
		
		
		
		if (row.frequencyGroupExists) {
			frequencyGroup = row.frequencyGroup;		
			frequencyGroupExists = true;
		}
		else
			frequencyGroupExists = false;
		
		if (row.freqGroupNameExists) {
			freqGroupName = row.freqGroupName;		
			freqGroupNameExists = true;
		}
		else
			freqGroupNameExists = false;
		
		if (row.fieldNameExists) {
			fieldName = row.fieldName;		
			fieldNameExists = true;
		}
		else
			fieldNameExists = false;
		
		if (row.fieldCodeExists) {
			fieldCode = row.fieldCode;		
			fieldCodeExists = true;
		}
		else
			fieldCodeExists = false;
		
		if (row.sourceNameExists) {
			sourceName = row.sourceName;		
			sourceNameExists = true;
		}
		else
			sourceNameExists = false;
		
		if (row.sourceCodeExists) {
			sourceCode = row.sourceCode;		
			sourceCodeExists = true;
		}
		else
			sourceCodeExists = false;
		
		if (row.scanIntentExists) {
			scanIntent = row.scanIntent;		
			scanIntentExists = true;
		}
		else
			scanIntentExists = false;
		
		if (row.assocCalDataIdExists) {
			assocCalDataId = row.assocCalDataId;		
			assocCalDataIdExists = true;
		}
		else
			assocCalDataIdExists = false;
		
		if (row.assocCalNatureExists) {
			assocCalNature = row.assocCalNature;		
			assocCalNatureExists = true;
		}
		else
			assocCalNatureExists = false;
		
		}	
	}

	
	bool CalDataRow::compareNoAutoInc(int numScan, vector<int > scanSet, CalTypeMod::CalType calType, ArrayTime startTimeObserved, ArrayTime endTimeObserved, CalDataOriginMod::CalDataOrigin calDataType) {
		bool result;
		result = true;
		
	
		
		result = result && (this->numScan == numScan);
		
		if (!result) return false;
	

	
		
		result = result && (this->scanSet == scanSet);
		
		if (!result) return false;
	

	
		
		result = result && (this->calType == calType);
		
		if (!result) return false;
	

	
		
		result = result && (this->startTimeObserved == startTimeObserved);
		
		if (!result) return false;
	

	
		
		result = result && (this->endTimeObserved == endTimeObserved);
		
		if (!result) return false;
	

	
		
		result = result && (this->calDataType == calDataType);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalDataRow::compareRequiredValue(int numScan, vector<int > scanSet, CalTypeMod::CalType calType, ArrayTime startTimeObserved, ArrayTime endTimeObserved, CalDataOriginMod::CalDataOrigin calDataType) {
		bool result;
		result = true;
		
	
		if (!(this->numScan == numScan)) return false;
	

	
		if (!(this->scanSet == scanSet)) return false;
	

	
		if (!(this->calType == calType)) return false;
	

	
		if (!(this->startTimeObserved == startTimeObserved)) return false;
	

	
		if (!(this->endTimeObserved == endTimeObserved)) return false;
	

	
		if (!(this->calDataType == calDataType)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalDataRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalDataRow::equalByRequiredValue(CalDataRow* x) {
		
			
		if (this->numScan != x->numScan) return false;
			
		if (this->scanSet != x->scanSet) return false;
			
		if (this->calType != x->calType) return false;
			
		if (this->startTimeObserved != x->startTimeObserved) return false;
			
		if (this->endTimeObserved != x->endTimeObserved) return false;
			
		if (this->calDataType != x->calDataType) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
