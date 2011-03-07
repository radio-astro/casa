
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
 * File CalFocusRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalFocusRow.h>
#include <CalFocusTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalFocusRow;
using asdm::CalFocusTable;

using asdm::CalDataTable;
using asdm::CalDataRow;

using asdm::CalReductionTable;
using asdm::CalReductionRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	CalFocusRow::~CalFocusRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalFocusTable &CalFocusRow::getTable() const {
		return table;
	}
	
	void CalFocusRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalFocusRowIDL struct.
	 */
	CalFocusRowIDL *CalFocusRow::toIDL() const {
		CalFocusRowIDL *x = new CalFocusRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->offset.length(offset.size());
		for (unsigned int i = 0; i < offset.size(); ++i) {
			
			x->offset[i] = offset.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->error.length(error.size());
		for (unsigned int i = 0; i < error.size(); ++i) {
			
			x->error[i] = error.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->focusMethod = focusMethod;
 				
 			
		
	

	
  		
		
		
			
		x->pointingDirection.length(pointingDirection.size());
		for (unsigned int i = 0; i < pointingDirection.size(); ++i) {
			
			x->pointingDirection[i] = pointingDirection.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->wasFixed.length(wasFixed.size());
		for (unsigned int i = 0; i < wasFixed.size(); ++i) {
			
				
			x->wasFixed[i] = wasFixed.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->ambientTemperature = ambientTemperature.toIDLTemperature();
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalFocusRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalFocusRow::setFromIDL (CalFocusRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		offset .clear();
		for (unsigned int i = 0; i <x.offset.length(); ++i) {
			
			offset.push_back(Length (x.offset[i]));
			
		}
			
  		
		
	

	
		
		
			
		error .clear();
		for (unsigned int i = 0; i <x.error.length(); ++i) {
			
			error.push_back(Length (x.error[i]));
			
		}
			
  		
		
	

	
		
		
			
		setFocusMethod(x.focusMethod);
  			
 		
		
	

	
		
		
			
		pointingDirection .clear();
		for (unsigned int i = 0; i <x.pointingDirection.length(); ++i) {
			
			pointingDirection.push_back(Angle (x.pointingDirection[i]));
			
		}
			
  		
		
	

	
		
		
			
		wasFixed .clear();
		for (unsigned int i = 0; i <x.wasFixed.length(); ++i) {
			
			wasFixed.push_back(x.wasFixed[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setAmbientTemperature(Temperature (x.ambientTemperature));
			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalFocus");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalFocusRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(offset, "offset", buf);
		
		
	

  	
 		
		
		Parser::toXML(error, "error", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("focusMethod", focusMethod));
		
		
	

  	
 		
		
		Parser::toXML(pointingDirection, "pointingDirection", buf);
		
		
	

  	
 		
		
		Parser::toXML(wasFixed, "wasFixed", buf);
		
		
	

  	
 		
		
		Parser::toXML(ambientTemperature, "ambientTemperature", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalFocusRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalFocus",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalFocus",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalFocus",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalFocus",rowDoc);
		
		
		
	

	
  		
			
					
	  	setOffset(Parser::get1DLength("offset","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setError(Parser::get1DLength("error","CalFocus",rowDoc));
	  			
	  		
		
	

	
		
		
		
		focusMethod = EnumerationParser::getFocusMethod("focusMethod","CalFocus",rowDoc);
		
		
		
	

	
  		
			
					
	  	setPointingDirection(Parser::get1DAngle("pointingDirection","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setWasFixed(Parser::get1DBoolean("wasFixed","CalFocus",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setAmbientTemperature(Parser::getTemperature("ambientTemperature","CalFocus",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalFocus");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalFocusRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalFocus");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalFocusRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalFocusRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalFocusRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalFocusRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get offset.
 	 * @return offset as vector<Length >
 	 */
 	vector<Length > CalFocusRow::getOffset() const {
	
  		return offset;
 	}

 	/**
 	 * Set offset with the specified vector<Length >.
 	 * @param offset The vector<Length > value to which offset is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setOffset (vector<Length > offset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->offset = offset;
	
 	}
	
	

	

	
 	/**
 	 * Get error.
 	 * @return error as vector<Length >
 	 */
 	vector<Length > CalFocusRow::getError() const {
	
  		return error;
 	}

 	/**
 	 * Set error with the specified vector<Length >.
 	 * @param error The vector<Length > value to which error is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setError (vector<Length > error)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->error = error;
	
 	}
	
	

	

	
 	/**
 	 * Get focusMethod.
 	 * @return focusMethod as FocusMethodMod::FocusMethod
 	 */
 	FocusMethodMod::FocusMethod CalFocusRow::getFocusMethod() const {
	
  		return focusMethod;
 	}

 	/**
 	 * Set focusMethod with the specified FocusMethodMod::FocusMethod.
 	 * @param focusMethod The FocusMethodMod::FocusMethod value to which focusMethod is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setFocusMethod (FocusMethodMod::FocusMethod focusMethod)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusMethod = focusMethod;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingDirection.
 	 * @return pointingDirection as vector<Angle >
 	 */
 	vector<Angle > CalFocusRow::getPointingDirection() const {
	
  		return pointingDirection;
 	}

 	/**
 	 * Set pointingDirection with the specified vector<Angle >.
 	 * @param pointingDirection The vector<Angle > value to which pointingDirection is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setPointingDirection (vector<Angle > pointingDirection)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingDirection = pointingDirection;
	
 	}
	
	

	

	
 	/**
 	 * Get wasFixed.
 	 * @return wasFixed as vector<bool >
 	 */
 	vector<bool > CalFocusRow::getWasFixed() const {
	
  		return wasFixed;
 	}

 	/**
 	 * Set wasFixed with the specified vector<bool >.
 	 * @param wasFixed The vector<bool > value to which wasFixed is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setWasFixed (vector<bool > wasFixed)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->wasFixed = wasFixed;
	
 	}
	
	

	

	
 	/**
 	 * Get ambientTemperature.
 	 * @return ambientTemperature as Temperature
 	 */
 	Temperature CalFocusRow::getAmbientTemperature() const {
	
  		return ambientTemperature;
 	}

 	/**
 	 * Set ambientTemperature with the specified Temperature.
 	 * @param ambientTemperature The Temperature value to which ambientTemperature is to be set.
 	 
 	
 		
 	 */
 	void CalFocusRow::setAmbientTemperature (Temperature ambientTemperature)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->ambientTemperature = ambientTemperature;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalFocusRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalFocus");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalFocusRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalFocus");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalFocusRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalFocusRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalFocusRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalFocusRow::CalFocusRow (CalFocusTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
focusMethod = CFocusMethod::from_int(0);
	

	

	

	
	
	}
	
	CalFocusRow::CalFocusRow (CalFocusTable &t, CalFocusRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
		
		
		
			frequencyRange = row.frequencyRange;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			receiverBand = row.receiverBand;
		
			offset = row.offset;
		
			error = row.error;
		
			focusMethod = row.focusMethod;
		
			pointingDirection = row.pointingDirection;
		
			wasFixed = row.wasFixed;
		
			ambientTemperature = row.ambientTemperature;
		
		
		
		
		}	
	}

	
	bool CalFocusRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, vector<Frequency > frequencyRange, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Length > offset, vector<Length > error, FocusMethodMod::FocusMethod focusMethod, vector<Angle > pointingDirection, vector<bool > wasFixed, Temperature ambientTemperature) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->offset == offset);
		
		if (!result) return false;
	

	
		
		result = result && (this->error == error);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusMethod == focusMethod);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingDirection == pointingDirection);
		
		if (!result) return false;
	

	
		
		result = result && (this->wasFixed == wasFixed);
		
		if (!result) return false;
	

	
		
		result = result && (this->ambientTemperature == ambientTemperature);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalFocusRow::compareRequiredValue(vector<Frequency > frequencyRange, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Length > offset, vector<Length > error, FocusMethodMod::FocusMethod focusMethod, vector<Angle > pointingDirection, vector<bool > wasFixed, Temperature ambientTemperature) {
		bool result;
		result = true;
		
	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

	
		if (!(this->offset == offset)) return false;
	

	
		if (!(this->error == error)) return false;
	

	
		if (!(this->focusMethod == focusMethod)) return false;
	

	
		if (!(this->pointingDirection == pointingDirection)) return false;
	

	
		if (!(this->wasFixed == wasFixed)) return false;
	

	
		if (!(this->ambientTemperature == ambientTemperature)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalFocusRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalFocusRow::equalByRequiredValue(CalFocusRow* x) {
		
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		if (this->offset != x->offset) return false;
			
		if (this->error != x->error) return false;
			
		if (this->focusMethod != x->focusMethod) return false;
			
		if (this->pointingDirection != x->pointingDirection) return false;
			
		if (this->wasFixed != x->wasFixed) return false;
			
		if (this->ambientTemperature != x->ambientTemperature) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
