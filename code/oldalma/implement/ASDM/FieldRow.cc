
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
 * File FieldRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <FieldRow.h>
#include <FieldTable.h>

#include <FieldTable.h>
#include <FieldRow.h>

#include <EphemerisTable.h>
#include <EphemerisRow.h>

#include <SourceTable.h>
#include <SourceRow.h>
	

using asdm::ASDM;
using asdm::FieldRow;
using asdm::FieldTable;

using asdm::FieldTable;
using asdm::FieldRow;

using asdm::EphemerisTable;
using asdm::EphemerisRow;

using asdm::SourceTable;
using asdm::SourceRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	FieldRow::~FieldRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	FieldTable &FieldRow::getTable() const {
		return table;
	}
	
	void FieldRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FieldRowIDL struct.
	 */
	FieldRowIDL *FieldRow::toIDL() const {
		FieldRowIDL *x = new FieldRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->fieldId = fieldId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->fieldName = CORBA::string_dup(fieldName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->code = CORBA::string_dup(code.c_str());
				
 			
		
	

	
  		
		
		
			
		x->time = time.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		
			
		x->delayDir.length(delayDir.size());
		for (unsigned int i = 0; i < delayDir.size(); i++) {
			x->delayDir[i].length(delayDir.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < delayDir.size() ; i++)
			for (unsigned int j = 0; j < delayDir.at(i).size(); j++)
					
				x->delayDir[i][j]= delayDir.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		
			
		x->phaseDir.length(phaseDir.size());
		for (unsigned int i = 0; i < phaseDir.size(); i++) {
			x->phaseDir[i].length(phaseDir.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < phaseDir.size() ; i++)
			for (unsigned int j = 0; j < phaseDir.at(i).size(); j++)
					
				x->phaseDir[i][j]= phaseDir.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		
			
		x->referenceDir.length(referenceDir.size());
		for (unsigned int i = 0; i < referenceDir.size(); i++) {
			x->referenceDir[i].length(referenceDir.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < referenceDir.size() ; i++)
			for (unsigned int j = 0; j < referenceDir.at(i).size(); j++)
					
				x->referenceDir[i][j]= referenceDir.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->assocNatureExists = assocNatureExists;
		
		
			
				
		x->assocNature = CORBA::string_dup(assocNature.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
	
  	
 		
 		
		x->assocFieldIdExists = assocFieldIdExists;
		
		
		
		x->assocFieldId.length(assocFieldId.size());
		for (unsigned int i = 0; i < assocFieldId.size(); ++i) {
			
			x->assocFieldId[i] = assocFieldId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
 		
		x->ephemerisIdExists = ephemerisIdExists;
		
		
	 	
			
		x->ephemerisId = ephemerisId.toIDLTag();
			
	 	 		
  	

	
  	
 		
 		
		x->sourceIdExists = sourceIdExists;
		
		
	 	
			
				
		x->sourceId = sourceId;
 				
 			
	 	 		
  	

	
		
	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FieldRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void FieldRow::setFromIDL (FieldRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setFieldId(Tag (x.fieldId));
			
 		
		
	

	
		
		
			
		setFieldName(string (x.fieldName));
			
 		
		
	

	
		
		
			
		setCode(string (x.code));
			
 		
		
	

	
		
		
			
		setTime(ArrayTime (x.time));
			
 		
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		delayDir .clear();
		vector<Angle> v_aux_delayDir;
		for (unsigned int i = 0; i < x.delayDir.length(); ++i) {
			v_aux_delayDir.clear();
			for (unsigned int j = 0; j < x.delayDir[0].length(); ++j) {
				
				v_aux_delayDir.push_back(Angle (x.delayDir[i][j]));
				
  			}
  			delayDir.push_back(v_aux_delayDir);			
		}
			
  		
		
	

	
		
		
			
		phaseDir .clear();
		vector<Angle> v_aux_phaseDir;
		for (unsigned int i = 0; i < x.phaseDir.length(); ++i) {
			v_aux_phaseDir.clear();
			for (unsigned int j = 0; j < x.phaseDir[0].length(); ++j) {
				
				v_aux_phaseDir.push_back(Angle (x.phaseDir[i][j]));
				
  			}
  			phaseDir.push_back(v_aux_phaseDir);			
		}
			
  		
		
	

	
		
		
			
		referenceDir .clear();
		vector<Angle> v_aux_referenceDir;
		for (unsigned int i = 0; i < x.referenceDir.length(); ++i) {
			v_aux_referenceDir.clear();
			for (unsigned int j = 0; j < x.referenceDir[0].length(); ++j) {
				
				v_aux_referenceDir.push_back(Angle (x.referenceDir[i][j]));
				
  			}
  			referenceDir.push_back(v_aux_referenceDir);			
		}
			
  		
		
	

	
		
		assocNatureExists = x.assocNatureExists;
		if (x.assocNatureExists) {
		
		
			
		setAssocNature(string (x.assocNature));
			
 		
		
		}
		
	

	
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
	

	
	
		
	
		
		assocFieldIdExists = x.assocFieldIdExists;
		if (x.assocFieldIdExists) {
		
		assocFieldId .clear();
		for (unsigned int i = 0; i <x.assocFieldId.length(); ++i) {
			
			assocFieldId.push_back(Tag (x.assocFieldId[i]));
			
		}
		
		}
		
  	

	
		
		ephemerisIdExists = x.ephemerisIdExists;
		if (x.ephemerisIdExists) {
		
		
			
		setEphemerisId(Tag (x.ephemerisId));
			
 		
		
		}
		
	

	
		
		sourceIdExists = x.sourceIdExists;
		if (x.sourceIdExists) {
		
		
			
		setSourceId(x.sourceId);
  			
 		
		
		}
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Field");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string FieldRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(fieldId, "fieldId", buf);
		
		
	

  	
 		
		
		Parser::toXML(fieldName, "fieldName", buf);
		
		
	

  	
 		
		
		Parser::toXML(code, "code", buf);
		
		
	

  	
 		
		
		Parser::toXML(time, "time", buf);
		
		
	

  	
 		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(delayDir, "delayDir", buf);
		
		
	

  	
 		
		
		Parser::toXML(phaseDir, "phaseDir", buf);
		
		
	

  	
 		
		
		Parser::toXML(referenceDir, "referenceDir", buf);
		
		
	

  	
 		
		if (assocNatureExists) {
		
		
		Parser::toXML(assocNature, "assocNature", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
	

	
	
		
  	
 		
		if (assocFieldIdExists) {
		
		
		Parser::toXML(assocFieldId, "assocFieldId", buf);
		
		
		}
		
	

  	
 		
		if (ephemerisIdExists) {
		
		
		Parser::toXML(ephemerisId, "ephemerisId", buf);
		
		
		}
		
	

  	
 		
		if (sourceIdExists) {
		
		
		Parser::toXML(sourceId, "sourceId", buf);
		
		
		}
		
	

	
		
	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void FieldRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setFieldId(Parser::getTag("fieldId","Field",rowDoc));
			
		
	

	
  		
			
	  	setFieldName(Parser::getString("fieldName","Field",rowDoc));
			
		
	

	
  		
			
	  	setCode(Parser::getString("code","Field",rowDoc));
			
		
	

	
  		
			
	  	setTime(Parser::getArrayTime("time","Field",rowDoc));
			
		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","Field",rowDoc));
			
		
	

	
  		
			
					
	  	setDelayDir(Parser::get2DAngle("delayDir","Field",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPhaseDir(Parser::get2DAngle("phaseDir","Field",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setReferenceDir(Parser::get2DAngle("referenceDir","Field",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<assocNature>")) {
			
	  		setAssocNature(Parser::getString("assocNature","Field",rowDoc));
			
		}
 		
	

	
  		
			
	  	setFlagRow(Parser::getBoolean("flagRow","Field",rowDoc));
			
		
	

	
	
		
	
  		
  		if (row.isStr("<assocFieldId>")) {
  			setAssocFieldId(Parser::get1DTag("assocFieldId","Field",rowDoc));  		
  		}
  		
  	

	
  		
        if (row.isStr("<ephemerisId>")) {
			
	  		setEphemerisId(Parser::getTag("ephemerisId","Field",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<sourceId>")) {
			
	  		setSourceId(Parser::getInteger("sourceId","Field",rowDoc));
			
		}
 		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Field");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get fieldId.
 	 * @return fieldId as Tag
 	 */
 	Tag FieldRow::getFieldId() const {
	
  		return fieldId;
 	}

 	/**
 	 * Set fieldId with the specified Tag.
 	 * @param fieldId The Tag value to which fieldId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FieldRow::setFieldId (Tag fieldId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("fieldId", "Field");
		
  		}
  	
 		this->fieldId = fieldId;
	
 	}
	
	

	

	
 	/**
 	 * Get fieldName.
 	 * @return fieldName as string
 	 */
 	string FieldRow::getFieldName() const {
	
  		return fieldName;
 	}

 	/**
 	 * Set fieldName with the specified string.
 	 * @param fieldName The string value to which fieldName is to be set.
 	 
 	
 		
 	 */
 	void FieldRow::setFieldName (string fieldName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->fieldName = fieldName;
	
 	}
	
	

	

	
 	/**
 	 * Get code.
 	 * @return code as string
 	 */
 	string FieldRow::getCode() const {
	
  		return code;
 	}

 	/**
 	 * Set code with the specified string.
 	 * @param code The string value to which code is to be set.
 	 
 	
 		
 	 */
 	void FieldRow::setCode (string code)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->code = code;
	
 	}
	
	

	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime FieldRow::getTime() const {
	
  		return time;
 	}

 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 	
 		
 	 */
 	void FieldRow::setTime (ArrayTime time)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->time = time;
	
 	}
	
	

	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int FieldRow::getNumPoly() const {
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 	
 		
 	 */
 	void FieldRow::setNumPoly (int numPoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numPoly = numPoly;
	
 	}
	
	

	

	
 	/**
 	 * Get delayDir.
 	 * @return delayDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > FieldRow::getDelayDir() const {
	
  		return delayDir;
 	}

 	/**
 	 * Set delayDir with the specified vector<vector<Angle > >.
 	 * @param delayDir The vector<vector<Angle > > value to which delayDir is to be set.
 	 
 	
 		
 	 */
 	void FieldRow::setDelayDir (vector<vector<Angle > > delayDir)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->delayDir = delayDir;
	
 	}
	
	

	

	
 	/**
 	 * Get phaseDir.
 	 * @return phaseDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > FieldRow::getPhaseDir() const {
	
  		return phaseDir;
 	}

 	/**
 	 * Set phaseDir with the specified vector<vector<Angle > >.
 	 * @param phaseDir The vector<vector<Angle > > value to which phaseDir is to be set.
 	 
 	
 		
 	 */
 	void FieldRow::setPhaseDir (vector<vector<Angle > > phaseDir)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phaseDir = phaseDir;
	
 	}
	
	

	

	
 	/**
 	 * Get referenceDir.
 	 * @return referenceDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > FieldRow::getReferenceDir() const {
	
  		return referenceDir;
 	}

 	/**
 	 * Set referenceDir with the specified vector<vector<Angle > >.
 	 * @param referenceDir The vector<vector<Angle > > value to which referenceDir is to be set.
 	 
 	
 		
 	 */
 	void FieldRow::setReferenceDir (vector<vector<Angle > > referenceDir)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->referenceDir = referenceDir;
	
 	}
	
	

	
	/**
	 * The attribute assocNature is optional. Return true if this attribute exists.
	 * @return true if and only if the assocNature attribute exists. 
	 */
	bool FieldRow::isAssocNatureExists() const {
		return assocNatureExists;
	}
	

	
 	/**
 	 * Get assocNature, which is optional.
 	 * @return assocNature as string
 	 * @throw IllegalAccessException If assocNature does not exist.
 	 */
 	string FieldRow::getAssocNature() const throw(IllegalAccessException) {
		if (!assocNatureExists) {
			throw IllegalAccessException("assocNature", "Field");
		}
	
  		return assocNature;
 	}

 	/**
 	 * Set assocNature with the specified string.
 	 * @param assocNature The string value to which assocNature is to be set.
 	 
 	
 	 */
 	void FieldRow::setAssocNature (string assocNature) {
	
 		this->assocNature = assocNature;
	
		assocNatureExists = true;
	
 	}
	
	
	/**
	 * Mark assocNature, which is an optional field, as non-existent.
	 */
	void FieldRow::clearAssocNature () {
		assocNatureExists = false;
	}
	

	

	
 	/**
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool FieldRow::getFlagRow() const {
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 		
 	 */
 	void FieldRow::setFlagRow (bool flagRow)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagRow = flagRow;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	/**
	 * The attribute assocFieldId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocFieldId attribute exists. 
	 */
	bool FieldRow::isAssocFieldIdExists() const {
		return assocFieldIdExists;
	}
	

	
 	/**
 	 * Get assocFieldId, which is optional.
 	 * @return assocFieldId as vector<Tag> 
 	 * @throw IllegalAccessException If assocFieldId does not exist.
 	 */
 	vector<Tag>  FieldRow::getAssocFieldId() const throw(IllegalAccessException) {
		if (!assocFieldIdExists) {
			throw IllegalAccessException("assocFieldId", "Field");
		}
	
  		return assocFieldId;
 	}

 	/**
 	 * Set assocFieldId with the specified vector<Tag> .
 	 * @param assocFieldId The vector<Tag>  value to which assocFieldId is to be set.
 	 
 	
 	 */
 	void FieldRow::setAssocFieldId (vector<Tag>  assocFieldId) {
	
 		this->assocFieldId = assocFieldId;
	
		assocFieldIdExists = true;
	
 	}
	
	
	/**
	 * Mark assocFieldId, which is an optional field, as non-existent.
	 */
	void FieldRow::clearAssocFieldId () {
		assocFieldIdExists = false;
	}
	

	
	/**
	 * The attribute ephemerisId is optional. Return true if this attribute exists.
	 * @return true if and only if the ephemerisId attribute exists. 
	 */
	bool FieldRow::isEphemerisIdExists() const {
		return ephemerisIdExists;
	}
	

	
 	/**
 	 * Get ephemerisId, which is optional.
 	 * @return ephemerisId as Tag
 	 * @throw IllegalAccessException If ephemerisId does not exist.
 	 */
 	Tag FieldRow::getEphemerisId() const throw(IllegalAccessException) {
		if (!ephemerisIdExists) {
			throw IllegalAccessException("ephemerisId", "Field");
		}
	
  		return ephemerisId;
 	}

 	/**
 	 * Set ephemerisId with the specified Tag.
 	 * @param ephemerisId The Tag value to which ephemerisId is to be set.
 	 
 	
 	 */
 	void FieldRow::setEphemerisId (Tag ephemerisId) {
	
 		this->ephemerisId = ephemerisId;
	
		ephemerisIdExists = true;
	
 	}
	
	
	/**
	 * Mark ephemerisId, which is an optional field, as non-existent.
	 */
	void FieldRow::clearEphemerisId () {
		ephemerisIdExists = false;
	}
	

	
	/**
	 * The attribute sourceId is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceId attribute exists. 
	 */
	bool FieldRow::isSourceIdExists() const {
		return sourceIdExists;
	}
	

	
 	/**
 	 * Get sourceId, which is optional.
 	 * @return sourceId as int
 	 * @throw IllegalAccessException If sourceId does not exist.
 	 */
 	int FieldRow::getSourceId() const throw(IllegalAccessException) {
		if (!sourceIdExists) {
			throw IllegalAccessException("sourceId", "Field");
		}
	
  		return sourceId;
 	}

 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 	
 	 */
 	void FieldRow::setSourceId (int sourceId) {
	
 		this->sourceId = sourceId;
	
		sourceIdExists = true;
	
 	}
	
	
	/**
	 * Mark sourceId, which is an optional field, as non-existent.
	 */
	void FieldRow::clearSourceId () {
		sourceIdExists = false;
	}
	

	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set assocFieldId[i] with the specified Tag.
 	 * @param i The index in assocFieldId where to set the Tag value.
 	 * @param assocFieldId The Tag value to which assocFieldId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void FieldRow::setAssocFieldId (int i, Tag assocFieldId) {
  		if ((i < 0) || (i > ((int) this->assocFieldId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute assocFieldId in table FieldTable");
  		vector<Tag> ::iterator iter = this->assocFieldId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->assocFieldId.insert(this->assocFieldId.erase(iter), assocFieldId); 	
  	}
 			
	
	
	
		
/**
 * Append a Tag to assocFieldId.
 * @param id the Tag to be appended to assocFieldId
 */
 void FieldRow::addAssocFieldId(Tag id){
 	assocFieldId.push_back(id);
}

/**
 * Append an array of Tag to assocFieldId.
 * @param id an array of Tag to be appended to assocFieldId
 */
 void FieldRow::addAssocFieldId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		assocFieldId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in assocFieldId at position i.
  *
  */
 const Tag FieldRow::getAssocFieldId(int i) {
 	return assocFieldId.at(i);
 }
 
 /**
  * Returns the FieldRow linked to this row via the Tag stored in assocFieldId
  * at position i.
  */
 FieldRow* FieldRow::getField(int i) {
 	return table.getContainer().getField().getRowByKey(assocFieldId.at(i));
 } 
 
 /**
  * Returns the vector of FieldRow* linked to this row via the Tags stored in assocFieldId
  *
  */
 vector<FieldRow *> FieldRow::getFields() {
 	vector<FieldRow *> result;
 	for (unsigned int i = 0; i < assocFieldId.size(); i++)
 		result.push_back(table.getContainer().getField().getRowByKey(assocFieldId.at(i)));
 		
 	return result;
 }
  

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Ephemeris table having Ephemeris.ephemerisId == ephemerisId
	 * @return a EphemerisRow*
	 * 
	 
	 * throws IllegalAccessException
	 
	 */
	 EphemerisRow* FieldRow::getEphemerisUsingEphemerisId() {
	 
	 	if (!ephemerisIdExists)
	 		throw IllegalAccessException();	 		 
	 
	 	return table.getContainer().getEphemeris().getRowByKey(ephemerisId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of Field table to a collection of row of Source table.
	
	/**
	 * Get the collection of row in the Source table having their attribut sourceId == this->sourceId
	 */
	vector <SourceRow *> FieldRow::getSources() {
		
			if (sourceIdExists) {
				return table.getContainer().getSource().getRowBySourceId(sourceId);
			}
			else 
				throw IllegalAccessException();
		
	}
	

	

	
	/**
	 * Create a FieldRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FieldRow::FieldRow (FieldTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	
		assocNatureExists = false;
	

	

	
	
		assocFieldIdExists = false;
	

	
		ephemerisIdExists = false;
	

	
		sourceIdExists = false;
	

	
	
	
	

	

	

	

	

	

	

	

	

	
	
	}
	
	FieldRow::FieldRow (FieldTable &t, FieldRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	
		assocNatureExists = false;
	

	

	
	
		assocFieldIdExists = false;
	

	
		ephemerisIdExists = false;
	

	
		sourceIdExists = false;
	
		
		}
		else {
	
		
			fieldId = row.fieldId;
		
		
		
		
			fieldName = row.fieldName;
		
			code = row.code;
		
			time = row.time;
		
			numPoly = row.numPoly;
		
			delayDir = row.delayDir;
		
			phaseDir = row.phaseDir;
		
			referenceDir = row.referenceDir;
		
			flagRow = row.flagRow;
		
		
		
		
		if (row.assocFieldIdExists) {
			assocFieldId = row.assocFieldId;		
			assocFieldIdExists = true;
		}
		else
			assocFieldIdExists = false;
		
		if (row.ephemerisIdExists) {
			ephemerisId = row.ephemerisId;		
			ephemerisIdExists = true;
		}
		else
			ephemerisIdExists = false;
		
		if (row.sourceIdExists) {
			sourceId = row.sourceId;		
			sourceIdExists = true;
		}
		else
			sourceIdExists = false;
		
		if (row.assocNatureExists) {
			assocNature = row.assocNature;		
			assocNatureExists = true;
		}
		else
			assocNatureExists = false;
		
		}	
	}

	
	bool FieldRow::compareNoAutoInc(string fieldName, string code, ArrayTime time, int numPoly, vector<vector<Angle > > delayDir, vector<vector<Angle > > phaseDir, vector<vector<Angle > > referenceDir, bool flagRow) {
		bool result;
		result = true;
		
	
		
		result = result && (this->fieldName == fieldName);
		
		if (!result) return false;
	

	
		
		result = result && (this->code == code);
		
		if (!result) return false;
	

	
		
		result = result && (this->time == time);
		
		if (!result) return false;
	

	
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->delayDir == delayDir);
		
		if (!result) return false;
	

	
		
		result = result && (this->phaseDir == phaseDir);
		
		if (!result) return false;
	

	
		
		result = result && (this->referenceDir == referenceDir);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagRow == flagRow);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool FieldRow::compareRequiredValue(string fieldName, string code, ArrayTime time, int numPoly, vector<vector<Angle > > delayDir, vector<vector<Angle > > phaseDir, vector<vector<Angle > > referenceDir, bool flagRow) {
		bool result;
		result = true;
		
	
		if (!(this->fieldName == fieldName)) return false;
	

	
		if (!(this->code == code)) return false;
	

	
		if (!(this->time == time)) return false;
	

	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->delayDir == delayDir)) return false;
	

	
		if (!(this->phaseDir == phaseDir)) return false;
	

	
		if (!(this->referenceDir == referenceDir)) return false;
	

	
		if (!(this->flagRow == flagRow)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FieldRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool FieldRow::equalByRequiredValue(FieldRow* x) {
		
			
		if (this->fieldName != x->fieldName) return false;
			
		if (this->code != x->code) return false;
			
		if (this->time != x->time) return false;
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->delayDir != x->delayDir) return false;
			
		if (this->phaseDir != x->phaseDir) return false;
			
		if (this->referenceDir != x->referenceDir) return false;
			
		if (this->flagRow != x->flagRow) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
