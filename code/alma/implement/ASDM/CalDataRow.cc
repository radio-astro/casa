
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

	bool CalDataRow::isAdded() const {
		return hasBeenAdded;
	}	

	void CalDataRow::isAdded(bool added) {
		hasBeenAdded = added;
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
			
		
	

	
  		
		
		
			
		x->startTimeObserved = startTimeObserved.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endTimeObserved = endTimeObserved.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->execBlockUID = execBlockUID.toIDLEntityRef();
			
		
	

	
  		
		
		
			
				
		x->calDataType = calDataType;
 				
 			
		
	

	
  		
		
		
			
				
		x->calType = calType;
 				
 			
		
	

	
  		
		
		
			
				
		x->numScan = numScan;
 				
 			
		
	

	
  		
		
		
			
		x->scanSet.length(scanSet.size());
		for (unsigned int i = 0; i < scanSet.size(); ++i) {
			
				
			x->scanSet[i] = scanSet.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->assocCalDataIdExists = assocCalDataIdExists;
		
		
			
		x->assocCalDataId = assocCalDataId.toIDLTag();
			
		
	

	
  		
		
		x->assocCalNatureExists = assocCalNatureExists;
		
		
			
				
		x->assocCalNature = assocCalNature;
 				
 			
		
	

	
  		
		
		x->fieldNameExists = fieldNameExists;
		
		
			
		x->fieldName.length(fieldName.size());
		for (unsigned int i = 0; i < fieldName.size(); ++i) {
			
				
			x->fieldName[i] = CORBA::string_dup(fieldName.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
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
			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalDataRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalDataRow::setFromIDL (CalDataRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setStartTimeObserved(ArrayTime (x.startTimeObserved));
			
 		
		
	

	
		
		
			
		setEndTimeObserved(ArrayTime (x.endTimeObserved));
			
 		
		
	

	
		
		
			
		setExecBlockUID(EntityRef (x.execBlockUID));
			
 		
		
	

	
		
		
			
		setCalDataType(x.calDataType);
  			
 		
		
	

	
		
		
			
		setCalType(x.calType);
  			
 		
		
	

	
		
		
			
		setNumScan(x.numScan);
  			
 		
		
	

	
		
		
			
		scanSet .clear();
		for (unsigned int i = 0; i <x.scanSet.length(); ++i) {
			
			scanSet.push_back(x.scanSet[i]);
  			
		}
			
  		
		
	

	
		
		assocCalDataIdExists = x.assocCalDataIdExists;
		if (x.assocCalDataIdExists) {
		
		
			
		setAssocCalDataId(Tag (x.assocCalDataId));
			
 		
		
		}
		
	

	
		
		assocCalNatureExists = x.assocCalNatureExists;
		if (x.assocCalNatureExists) {
		
		
			
		setAssocCalNature(x.assocCalNature);
  			
 		
		
		}
		
	

	
		
		fieldNameExists = x.fieldNameExists;
		if (x.fieldNameExists) {
		
		
			
		fieldName .clear();
		for (unsigned int i = 0; i <x.fieldName.length(); ++i) {
			
			fieldName.push_back(string (x.fieldName[i]));
			
		}
			
  		
		
		}
		
	

	
		
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
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalData");
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
		
		
	

  	
 		
		
		Parser::toXML(startTimeObserved, "startTimeObserved", buf);
		
		
	

  	
 		
		
		Parser::toXML(endTimeObserved, "endTimeObserved", buf);
		
		
	

  	
 		
		
		Parser::toXML(execBlockUID, "execBlockUID", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("calDataType", calDataType));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("calType", calType));
		
		
	

  	
 		
		
		Parser::toXML(numScan, "numScan", buf);
		
		
	

  	
 		
		
		Parser::toXML(scanSet, "scanSet", buf);
		
		
	

  	
 		
		if (assocCalDataIdExists) {
		
		
		Parser::toXML(assocCalDataId, "assocCalDataId", buf);
		
		
		}
		
	

  	
 		
		if (assocCalNatureExists) {
		
		
			buf.append(EnumerationParser::toXML("assocCalNature", assocCalNature));
		
		
		}
		
	

  	
 		
		if (fieldNameExists) {
		
		
		Parser::toXML(fieldName, "fieldName", buf);
		
		
		}
		
	

  	
 		
		if (sourceNameExists) {
		
		
		Parser::toXML(sourceName, "sourceName", buf);
		
		
		}
		
	

  	
 		
		if (sourceCodeExists) {
		
		
		Parser::toXML(sourceCode, "sourceCode", buf);
		
		
		}
		
	

  	
 		
		if (scanIntentExists) {
		
		
			buf.append(EnumerationParser::toXML("scanIntent", scanIntent));
		
		
		}
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CalDataRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setStartTimeObserved(Parser::getArrayTime("startTimeObserved","CalData",rowDoc));
			
		
	

	
  		
			
	  	setEndTimeObserved(Parser::getArrayTime("endTimeObserved","CalData",rowDoc));
			
		
	

	
  		
			
	  	setExecBlockUID(Parser::getEntityRef("execBlockUID","CalData",rowDoc));
			
		
	

	
		
		
		
		calDataType = EnumerationParser::getCalDataOrigin("calDataType","CalData",rowDoc);
		
		
		
	

	
		
		
		
		calType = EnumerationParser::getCalType("calType","CalData",rowDoc);
		
		
		
	

	
  		
			
	  	setNumScan(Parser::getInteger("numScan","CalData",rowDoc));
			
		
	

	
  		
			
					
	  	setScanSet(Parser::get1DInteger("scanSet","CalData",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<assocCalDataId>")) {
			
	  		setAssocCalDataId(Parser::getTag("assocCalDataId","CalData",rowDoc));
			
		}
 		
	

	
		
	if (row.isStr("<assocCalNature>")) {
		
		
		
		assocCalNature = EnumerationParser::getAssociatedCalNature("assocCalNature","CalData",rowDoc);
		
		
		
		assocCalNatureExists = true;
	}
		
	

	
  		
        if (row.isStr("<fieldName>")) {
			
								
	  		setFieldName(Parser::get1DString("fieldName","CalData",rowDoc));
	  			
	  		
		}
 		
	

	
  		
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
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalData");
		}
	}
	
	void CalDataRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	startTimeObserved.toBin(eoss);
		
	

	
	
		
	endTimeObserved.toBin(eoss);
		
	

	
	
		
	execBlockUID.toBin(eoss);
		
	

	
	
		
					
			eoss.writeInt(calDataType);
				
		
	

	
	
		
					
			eoss.writeInt(calType);
				
		
	

	
	
		
						
			eoss.writeInt(numScan);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) scanSet.size());
		for (unsigned int i = 0; i < scanSet.size(); i++)
				
			eoss.writeInt(scanSet.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(assocCalDataIdExists);
	if (assocCalDataIdExists) {
	
	
	
		
	assocCalDataId.toBin(eoss);
		
	

	}

	eoss.writeBoolean(assocCalNatureExists);
	if (assocCalNatureExists) {
	
	
	
		
					
			eoss.writeInt(assocCalNature);
				
		
	

	}

	eoss.writeBoolean(fieldNameExists);
	if (fieldNameExists) {
	
	
	
		
		
			
		eoss.writeInt((int) fieldName.size());
		for (unsigned int i = 0; i < fieldName.size(); i++)
				
			eoss.writeString(fieldName.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(sourceNameExists);
	if (sourceNameExists) {
	
	
	
		
		
			
		eoss.writeInt((int) sourceName.size());
		for (unsigned int i = 0; i < sourceName.size(); i++)
				
			eoss.writeString(sourceName.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(sourceCodeExists);
	if (sourceCodeExists) {
	
	
	
		
		
			
		eoss.writeInt((int) sourceCode.size());
		for (unsigned int i = 0; i < sourceCode.size(); i++)
				
			eoss.writeString(sourceCode.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(scanIntentExists);
	if (scanIntentExists) {
	
	
	
		
		
			
		eoss.writeInt((int) scanIntent.size());
		for (unsigned int i = 0; i < scanIntent.size(); i++)
				
			eoss.writeInt(scanIntent.at(i));
				
				
						
		
	

	}

	}
	
void CalDataRow::calDataIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calDataId =  Tag::fromBin(eiss);
		
	
	
}
void CalDataRow::startTimeObservedFromBin(EndianISStream& eiss) {
		
	
		
		
		startTimeObserved =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalDataRow::endTimeObservedFromBin(EndianISStream& eiss) {
		
	
		
		
		endTimeObserved =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalDataRow::execBlockUIDFromBin(EndianISStream& eiss) {
		
	
		
		
		execBlockUID =  EntityRef::fromBin(eiss);
		
	
	
}
void CalDataRow::calDataTypeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		calDataType = CCalDataOrigin::from_int(eiss.readInt());
			
		
	
	
}
void CalDataRow::calTypeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		calType = CCalType::from_int(eiss.readInt());
			
		
	
	
}
void CalDataRow::numScanFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numScan =  eiss.readInt();
			
		
	
	
}
void CalDataRow::scanSetFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		scanSet.clear();
		
		unsigned int scanSetDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < scanSetDim1; i++)
			
			scanSet.push_back(eiss.readInt());
			
	

		
	
	
}

void CalDataRow::assocCalDataIdFromBin(EndianISStream& eiss) {
		
	assocCalDataIdExists = eiss.readBoolean();
	if (assocCalDataIdExists) {
		
	
		
		
		assocCalDataId =  Tag::fromBin(eiss);
		
	

	}
	
}
void CalDataRow::assocCalNatureFromBin(EndianISStream& eiss) {
		
	assocCalNatureExists = eiss.readBoolean();
	if (assocCalNatureExists) {
		
	
	
		
			
		assocCalNature = CAssociatedCalNature::from_int(eiss.readInt());
			
		
	

	}
	
}
void CalDataRow::fieldNameFromBin(EndianISStream& eiss) {
		
	fieldNameExists = eiss.readBoolean();
	if (fieldNameExists) {
		
	
	
		
			
	
		fieldName.clear();
		
		unsigned int fieldNameDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < fieldNameDim1; i++)
			
			fieldName.push_back(eiss.readString());
			
	

		
	

	}
	
}
void CalDataRow::sourceNameFromBin(EndianISStream& eiss) {
		
	sourceNameExists = eiss.readBoolean();
	if (sourceNameExists) {
		
	
	
		
			
	
		sourceName.clear();
		
		unsigned int sourceNameDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < sourceNameDim1; i++)
			
			sourceName.push_back(eiss.readString());
			
	

		
	

	}
	
}
void CalDataRow::sourceCodeFromBin(EndianISStream& eiss) {
		
	sourceCodeExists = eiss.readBoolean();
	if (sourceCodeExists) {
		
	
	
		
			
	
		sourceCode.clear();
		
		unsigned int sourceCodeDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < sourceCodeDim1; i++)
			
			sourceCode.push_back(eiss.readString());
			
	

		
	

	}
	
}
void CalDataRow::scanIntentFromBin(EndianISStream& eiss) {
		
	scanIntentExists = eiss.readBoolean();
	if (scanIntentExists) {
		
	
	
		
			
	
		scanIntent.clear();
		
		unsigned int scanIntentDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < scanIntentDim1; i++)
			
			scanIntent.push_back(CScanIntent::from_int(eiss.readInt()));
			
	

		
	

	}
	
}
	
	
	CalDataRow* CalDataRow::fromBin(EndianISStream& eiss, CalDataTable& table, const vector<string>& attributesSeq) {
		CalDataRow* row = new  CalDataRow(table);
		
		map<string, CalDataAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "CalDataTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
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
 	 * Get execBlockUID.
 	 * @return execBlockUID as EntityRef
 	 */
 	EntityRef CalDataRow::getExecBlockUID() const {
	
  		return execBlockUID;
 	}

 	/**
 	 * Set execBlockUID with the specified EntityRef.
 	 * @param execBlockUID The EntityRef value to which execBlockUID is to be set.
 	 
 	
 		
 	 */
 	void CalDataRow::setExecBlockUID (EntityRef execBlockUID)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->execBlockUID = execBlockUID;
	
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
 	Tag CalDataRow::getAssocCalDataId() const  {
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
 	AssociatedCalNatureMod::AssociatedCalNature CalDataRow::getAssocCalNature() const  {
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
	 * The attribute fieldName is optional. Return true if this attribute exists.
	 * @return true if and only if the fieldName attribute exists. 
	 */
	bool CalDataRow::isFieldNameExists() const {
		return fieldNameExists;
	}
	

	
 	/**
 	 * Get fieldName, which is optional.
 	 * @return fieldName as vector<string >
 	 * @throw IllegalAccessException If fieldName does not exist.
 	 */
 	vector<string > CalDataRow::getFieldName() const  {
		if (!fieldNameExists) {
			throw IllegalAccessException("fieldName", "CalData");
		}
	
  		return fieldName;
 	}

 	/**
 	 * Set fieldName with the specified vector<string >.
 	 * @param fieldName The vector<string > value to which fieldName is to be set.
 	 
 	
 	 */
 	void CalDataRow::setFieldName (vector<string > fieldName) {
	
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
 	vector<string > CalDataRow::getSourceName() const  {
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
 	vector<string > CalDataRow::getSourceCode() const  {
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
 	vector<ScanIntentMod::ScanIntent > CalDataRow::getScanIntent() const  {
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
		
	
	

	

	

	

	

	

	

	

	
		assocCalDataIdExists = false;
	

	
		assocCalNatureExists = false;
	

	
		fieldNameExists = false;
	

	
		sourceNameExists = false;
	

	
		sourceCodeExists = false;
	

	
		scanIntentExists = false;
	

	
	
	
	
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
calDataType = CCalDataOrigin::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
calType = CCalType::from_int(0);
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
assocCalNature = CAssociatedCalNature::from_int(0);
	

	

	

	

	

	
	
	 fromBinMethods["calDataId"] = &CalDataRow::calDataIdFromBin; 
	 fromBinMethods["startTimeObserved"] = &CalDataRow::startTimeObservedFromBin; 
	 fromBinMethods["endTimeObserved"] = &CalDataRow::endTimeObservedFromBin; 
	 fromBinMethods["execBlockUID"] = &CalDataRow::execBlockUIDFromBin; 
	 fromBinMethods["calDataType"] = &CalDataRow::calDataTypeFromBin; 
	 fromBinMethods["calType"] = &CalDataRow::calTypeFromBin; 
	 fromBinMethods["numScan"] = &CalDataRow::numScanFromBin; 
	 fromBinMethods["scanSet"] = &CalDataRow::scanSetFromBin; 
		
	
	 fromBinMethods["assocCalDataId"] = &CalDataRow::assocCalDataIdFromBin; 
	 fromBinMethods["assocCalNature"] = &CalDataRow::assocCalNatureFromBin; 
	 fromBinMethods["fieldName"] = &CalDataRow::fieldNameFromBin; 
	 fromBinMethods["sourceName"] = &CalDataRow::sourceNameFromBin; 
	 fromBinMethods["sourceCode"] = &CalDataRow::sourceCodeFromBin; 
	 fromBinMethods["scanIntent"] = &CalDataRow::scanIntentFromBin; 
	
	}
	
	CalDataRow::CalDataRow (CalDataTable &t, CalDataRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	
		assocCalDataIdExists = false;
	

	
		assocCalNatureExists = false;
	

	
		fieldNameExists = false;
	

	
		sourceNameExists = false;
	

	
		sourceCodeExists = false;
	

	
		scanIntentExists = false;
	

			
		}
		else {
	
		
			calDataId = row.calDataId;
		
		
		
		
			startTimeObserved = row.startTimeObserved;
		
			endTimeObserved = row.endTimeObserved;
		
			execBlockUID = row.execBlockUID;
		
			calDataType = row.calDataType;
		
			calType = row.calType;
		
			numScan = row.numScan;
		
			scanSet = row.scanSet;
		
		
		
		
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
		
		if (row.fieldNameExists) {
			fieldName = row.fieldName;		
			fieldNameExists = true;
		}
		else
			fieldNameExists = false;
		
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
		
		}
		
		 fromBinMethods["calDataId"] = &CalDataRow::calDataIdFromBin; 
		 fromBinMethods["startTimeObserved"] = &CalDataRow::startTimeObservedFromBin; 
		 fromBinMethods["endTimeObserved"] = &CalDataRow::endTimeObservedFromBin; 
		 fromBinMethods["execBlockUID"] = &CalDataRow::execBlockUIDFromBin; 
		 fromBinMethods["calDataType"] = &CalDataRow::calDataTypeFromBin; 
		 fromBinMethods["calType"] = &CalDataRow::calTypeFromBin; 
		 fromBinMethods["numScan"] = &CalDataRow::numScanFromBin; 
		 fromBinMethods["scanSet"] = &CalDataRow::scanSetFromBin; 
			
	
		 fromBinMethods["assocCalDataId"] = &CalDataRow::assocCalDataIdFromBin; 
		 fromBinMethods["assocCalNature"] = &CalDataRow::assocCalNatureFromBin; 
		 fromBinMethods["fieldName"] = &CalDataRow::fieldNameFromBin; 
		 fromBinMethods["sourceName"] = &CalDataRow::sourceNameFromBin; 
		 fromBinMethods["sourceCode"] = &CalDataRow::sourceCodeFromBin; 
		 fromBinMethods["scanIntent"] = &CalDataRow::scanIntentFromBin; 
			
	}

	
	bool CalDataRow::compareNoAutoInc(ArrayTime startTimeObserved, ArrayTime endTimeObserved, EntityRef execBlockUID, CalDataOriginMod::CalDataOrigin calDataType, CalTypeMod::CalType calType, int numScan, vector<int > scanSet) {
		bool result;
		result = true;
		
	
		
		result = result && (this->startTimeObserved == startTimeObserved);
		
		if (!result) return false;
	

	
		
		result = result && (this->endTimeObserved == endTimeObserved);
		
		if (!result) return false;
	

	
		
		result = result && (this->execBlockUID == execBlockUID);
		
		if (!result) return false;
	

	
		
		result = result && (this->calDataType == calDataType);
		
		if (!result) return false;
	

	
		
		result = result && (this->calType == calType);
		
		if (!result) return false;
	

	
		
		result = result && (this->numScan == numScan);
		
		if (!result) return false;
	

	
		
		result = result && (this->scanSet == scanSet);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalDataRow::compareRequiredValue(ArrayTime startTimeObserved, ArrayTime endTimeObserved, EntityRef execBlockUID, CalDataOriginMod::CalDataOrigin calDataType, CalTypeMod::CalType calType, int numScan, vector<int > scanSet) {
		bool result;
		result = true;
		
	
		if (!(this->startTimeObserved == startTimeObserved)) return false;
	

	
		if (!(this->endTimeObserved == endTimeObserved)) return false;
	

	
		if (!(this->execBlockUID == execBlockUID)) return false;
	

	
		if (!(this->calDataType == calDataType)) return false;
	

	
		if (!(this->calType == calType)) return false;
	

	
		if (!(this->numScan == numScan)) return false;
	

	
		if (!(this->scanSet == scanSet)) return false;
	

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
		
			
		if (this->startTimeObserved != x->startTimeObserved) return false;
			
		if (this->endTimeObserved != x->endTimeObserved) return false;
			
		if (this->execBlockUID != x->execBlockUID) return false;
			
		if (this->calDataType != x->calDataType) return false;
			
		if (this->calType != x->calType) return false;
			
		if (this->numScan != x->numScan) return false;
			
		if (this->scanSet != x->scanSet) return false;
			
		
		return true;
	}	
	
/*
	 map<string, CalDataAttributeFromBin> CalDataRow::initFromBinMethods() {
		map<string, CalDataAttributeFromBin> result;
		
		result["calDataId"] = &CalDataRow::calDataIdFromBin;
		result["startTimeObserved"] = &CalDataRow::startTimeObservedFromBin;
		result["endTimeObserved"] = &CalDataRow::endTimeObservedFromBin;
		result["execBlockUID"] = &CalDataRow::execBlockUIDFromBin;
		result["calDataType"] = &CalDataRow::calDataTypeFromBin;
		result["calType"] = &CalDataRow::calTypeFromBin;
		result["numScan"] = &CalDataRow::numScanFromBin;
		result["scanSet"] = &CalDataRow::scanSetFromBin;
		
		
		result["assocCalDataId"] = &CalDataRow::assocCalDataIdFromBin;
		result["assocCalNature"] = &CalDataRow::assocCalNatureFromBin;
		result["fieldName"] = &CalDataRow::fieldNameFromBin;
		result["sourceName"] = &CalDataRow::sourceNameFromBin;
		result["sourceCode"] = &CalDataRow::sourceCodeFromBin;
		result["scanIntent"] = &CalDataRow::scanIntentFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
