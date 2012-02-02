
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
 * File CalDelayRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalDelayRow.h>
#include <CalDelayTable.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>

#include <CalDataTable.h>
#include <CalDataRow.h>
	

using asdm::ASDM;
using asdm::CalDelayRow;
using asdm::CalDelayTable;

using asdm::CalReductionTable;
using asdm::CalReductionRow;

using asdm::CalDataTable;
using asdm::CalDataRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	CalDelayRow::~CalDelayRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalDelayTable &CalDelayRow::getTable() const {
		return table;
	}
	
	void CalDelayRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalDelayRowIDL struct.
	 */
	CalDelayRowIDL *CalDelayRow::toIDL() const {
		CalDelayRowIDL *x = new CalDelayRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->basebandName = basebandName;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
				
		x->refAntennaName = CORBA::string_dup(refAntennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->delayOffset.length(delayOffset.size());
		for (unsigned int i = 0; i < delayOffset.size(); ++i) {
			
				
			x->delayOffset[i] = delayOffset.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->delayError.length(delayError.size());
		for (unsigned int i = 0; i < delayError.size(); ++i) {
			
				
			x->delayError[i] = delayError.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->crossDelayOffset = crossDelayOffset;
 				
 			
		
	

	
  		
		
		
			
				
		x->crossDelayOffsetError = crossDelayOffsetError;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalDelayRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalDelayRow::setFromIDL (CalDelayRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setBasebandName(x.basebandName);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		setRefAntennaName(string (x.refAntennaName));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		delayOffset .clear();
		for (unsigned int i = 0; i <x.delayOffset.length(); ++i) {
			
			delayOffset.push_back(x.delayOffset[i]);
  			
		}
			
  		
		
	

	
		
		
			
		delayError .clear();
		for (unsigned int i = 0; i <x.delayError.length(); ++i) {
			
			delayError.push_back(x.delayError[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setCrossDelayOffset(x.crossDelayOffset);
  			
 		
		
	

	
		
		
			
		setCrossDelayOffsetError(x.crossDelayOffsetError);
  			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalDelay");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalDelayRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("basebandName", basebandName));
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(refAntennaName, "refAntennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(delayOffset, "delayOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(delayError, "delayError", buf);
		
		
	

  	
 		
		
		Parser::toXML(crossDelayOffset, "crossDelayOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(crossDelayOffsetError, "crossDelayOffsetError", buf);
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(calDataId, "calDataId", buf);
		
		
	

  	
 		
		
		Parser::toXML(calReductionId, "calReductionId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CalDelayRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalDelay",rowDoc));
			
		
	

	
		
		
		
		basebandName = EnumerationParser::getBasebandName("basebandName","CalDelay",rowDoc);
		
		
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalDelay",rowDoc));
			
		
	

	
  		
			
	  	setRefAntennaName(Parser::getString("refAntennaName","CalDelay",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalDelay",rowDoc);
		
		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalDelay",rowDoc);			
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalDelay",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalDelay",rowDoc));
			
		
	

	
  		
			
					
	  	setDelayOffset(Parser::get1DDouble("delayOffset","CalDelay",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setDelayError(Parser::get1DDouble("delayError","CalDelay",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setCrossDelayOffset(Parser::getDouble("crossDelayOffset","CalDelay",rowDoc));
			
		
	

	
  		
			
	  	setCrossDelayOffsetError(Parser::getDouble("crossDelayOffsetError","CalDelay",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalDelay");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalDelayRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalDelay");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get basebandName.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 */
 	BasebandNameMod::BasebandName CalDelayRow::getBasebandName() const {
	
  		return basebandName;
 	}

 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setBasebandName (BasebandNameMod::BasebandName basebandName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("basebandName", "CalDelay");
		
  		}
  	
 		this->basebandName = basebandName;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalDelayRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get refAntennaName.
 	 * @return refAntennaName as string
 	 */
 	string CalDelayRow::getRefAntennaName() const {
	
  		return refAntennaName;
 	}

 	/**
 	 * Set refAntennaName with the specified string.
 	 * @param refAntennaName The string value to which refAntennaName is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setRefAntennaName (string refAntennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refAntennaName = refAntennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalDelayRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalDelayRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalDelayRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalDelayRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get delayOffset.
 	 * @return delayOffset as vector<double >
 	 */
 	vector<double > CalDelayRow::getDelayOffset() const {
	
  		return delayOffset;
 	}

 	/**
 	 * Set delayOffset with the specified vector<double >.
 	 * @param delayOffset The vector<double > value to which delayOffset is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setDelayOffset (vector<double > delayOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->delayOffset = delayOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get delayError.
 	 * @return delayError as vector<double >
 	 */
 	vector<double > CalDelayRow::getDelayError() const {
	
  		return delayError;
 	}

 	/**
 	 * Set delayError with the specified vector<double >.
 	 * @param delayError The vector<double > value to which delayError is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setDelayError (vector<double > delayError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->delayError = delayError;
	
 	}
	
	

	

	
 	/**
 	 * Get crossDelayOffset.
 	 * @return crossDelayOffset as double
 	 */
 	double CalDelayRow::getCrossDelayOffset() const {
	
  		return crossDelayOffset;
 	}

 	/**
 	 * Set crossDelayOffset with the specified double.
 	 * @param crossDelayOffset The double value to which crossDelayOffset is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setCrossDelayOffset (double crossDelayOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->crossDelayOffset = crossDelayOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get crossDelayOffsetError.
 	 * @return crossDelayOffsetError as double
 	 */
 	double CalDelayRow::getCrossDelayOffsetError() const {
	
  		return crossDelayOffsetError;
 	}

 	/**
 	 * Set crossDelayOffsetError with the specified double.
 	 * @param crossDelayOffsetError The double value to which crossDelayOffsetError is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setCrossDelayOffsetError (double crossDelayOffsetError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->crossDelayOffsetError = crossDelayOffsetError;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalDelayRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalDelay");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalDelayRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalDelay");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalDelayRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalDelayRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	/**
	 * Create a CalDelayRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalDelayRow::CalDelayRow (CalDelayTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
basebandName = CBasebandName::from_int(0);
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	

	
	
	}
	
	CalDelayRow::CalDelayRow (CalDelayTable &t, CalDelayRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
			basebandName = row.basebandName;
		
		
		
		
			numReceptor = row.numReceptor;
		
			refAntennaName = row.refAntennaName;
		
			receiverBand = row.receiverBand;
		
			polarizationTypes = row.polarizationTypes;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			delayOffset = row.delayOffset;
		
			delayError = row.delayError;
		
			crossDelayOffset = row.crossDelayOffset;
		
			crossDelayOffsetError = row.crossDelayOffsetError;
		
		
		
		
		}	
	}

	
	bool CalDelayRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, BasebandNameMod::BasebandName basebandName, int numReceptor, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<double > delayOffset, vector<double > delayError, double crossDelayOffset, double crossDelayOffsetError) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->basebandName == basebandName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->refAntennaName == refAntennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->delayOffset == delayOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->delayError == delayError);
		
		if (!result) return false;
	

	
		
		result = result && (this->crossDelayOffset == crossDelayOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->crossDelayOffsetError == crossDelayOffsetError);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalDelayRow::compareRequiredValue(int numReceptor, string refAntennaName, ReceiverBandMod::ReceiverBand receiverBand, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<double > delayOffset, vector<double > delayError, double crossDelayOffset, double crossDelayOffsetError) {
		bool result;
		result = true;
		
	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->refAntennaName == refAntennaName)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->delayOffset == delayOffset)) return false;
	

	
		if (!(this->delayError == delayError)) return false;
	

	
		if (!(this->crossDelayOffset == crossDelayOffset)) return false;
	

	
		if (!(this->crossDelayOffsetError == crossDelayOffsetError)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalDelayRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalDelayRow::equalByRequiredValue(CalDelayRow* x) {
		
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->refAntennaName != x->refAntennaName) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->delayOffset != x->delayOffset) return false;
			
		if (this->delayError != x->delayError) return false;
			
		if (this->crossDelayOffset != x->crossDelayOffset) return false;
			
		if (this->crossDelayOffsetError != x->crossDelayOffsetError) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
