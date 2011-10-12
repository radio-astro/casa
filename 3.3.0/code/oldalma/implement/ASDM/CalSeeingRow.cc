
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
 * File CalSeeingRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalSeeingRow.h>
#include <CalSeeingTable.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>

#include <CalDataTable.h>
#include <CalDataRow.h>
	

using asdm::ASDM;
using asdm::CalSeeingRow;
using asdm::CalSeeingTable;

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

	CalSeeingRow::~CalSeeingRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalSeeingTable &CalSeeingRow::getTable() const {
		return table;
	}
	
	void CalSeeingRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalSeeingRowIDL struct.
	 */
	CalSeeingRowIDL *CalSeeingRow::toIDL() const {
		CalSeeingRowIDL *x = new CalSeeingRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->numBaseLength = numBaseLength;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->baseLength.length(baseLength.size());
		for (unsigned int i = 0; i < baseLength.size(); ++i) {
			
			x->baseLength[i] = baseLength.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->corrPhaseRms.length(corrPhaseRms.size());
		for (unsigned int i = 0; i < corrPhaseRms.size(); ++i) {
			
			x->corrPhaseRms[i] = corrPhaseRms.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->uncorrPhaseRms.length(uncorrPhaseRms.size());
		for (unsigned int i = 0; i < uncorrPhaseRms.size(); ++i) {
			
			x->uncorrPhaseRms[i] = uncorrPhaseRms.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->seeing = seeing.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->seeingFrequency = seeingFrequency.toIDLFrequency();
			
		
	

	
  		
		
		
			
		x->seeingFreqBandwidth = seeingFreqBandwidth.toIDLFrequency();
			
		
	

	
  		
		
		x->exponentExists = exponentExists;
		
		
			
				
		x->exponent = exponent;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalSeeingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalSeeingRow::setFromIDL (CalSeeingRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setNumBaseLength(x.numBaseLength);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		baseLength .clear();
		for (unsigned int i = 0; i <x.baseLength.length(); ++i) {
			
			baseLength.push_back(Length (x.baseLength[i]));
			
		}
			
  		
		
	

	
		
		
			
		corrPhaseRms .clear();
		for (unsigned int i = 0; i <x.corrPhaseRms.length(); ++i) {
			
			corrPhaseRms.push_back(Angle (x.corrPhaseRms[i]));
			
		}
			
  		
		
	

	
		
		
			
		uncorrPhaseRms .clear();
		for (unsigned int i = 0; i <x.uncorrPhaseRms.length(); ++i) {
			
			uncorrPhaseRms.push_back(Angle (x.uncorrPhaseRms[i]));
			
		}
			
  		
		
	

	
		
		
			
		setSeeing(Angle (x.seeing));
			
 		
		
	

	
		
		
			
		setSeeingFrequency(Frequency (x.seeingFrequency));
			
 		
		
	

	
		
		
			
		setSeeingFreqBandwidth(Frequency (x.seeingFreqBandwidth));
			
 		
		
	

	
		
		exponentExists = x.exponentExists;
		if (x.exponentExists) {
		
		
			
		setExponent(x.exponent);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalSeeing");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalSeeingRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(numBaseLength, "numBaseLength", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseLength, "baseLength", buf);
		
		
	

  	
 		
		
		Parser::toXML(corrPhaseRms, "corrPhaseRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(uncorrPhaseRms, "uncorrPhaseRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(seeing, "seeing", buf);
		
		
	

  	
 		
		
		Parser::toXML(seeingFrequency, "seeingFrequency", buf);
		
		
	

  	
 		
		
		Parser::toXML(seeingFreqBandwidth, "seeingFreqBandwidth", buf);
		
		
	

  	
 		
		if (exponentExists) {
		
		
		Parser::toXML(exponent, "exponent", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
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
	void CalSeeingRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setNumBaseLength(Parser::getInteger("numBaseLength","CalSeeing",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalSeeing",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalSeeing",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalSeeing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setBaseLength(Parser::get1DLength("baseLength","CalSeeing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCorrPhaseRms(Parser::get1DAngle("corrPhaseRms","CalSeeing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setUncorrPhaseRms(Parser::get1DAngle("uncorrPhaseRms","CalSeeing",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setSeeing(Parser::getAngle("seeing","CalSeeing",rowDoc));
			
		
	

	
  		
			
	  	setSeeingFrequency(Parser::getFrequency("seeingFrequency","CalSeeing",rowDoc));
			
		
	

	
  		
			
	  	setSeeingFreqBandwidth(Parser::getFrequency("seeingFreqBandwidth","CalSeeing",rowDoc));
			
		
	

	
  		
        if (row.isStr("<exponent>")) {
			
	  		setExponent(Parser::getFloat("exponent","CalSeeing",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalSeeing");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get numBaseLength.
 	 * @return numBaseLength as int
 	 */
 	int CalSeeingRow::getNumBaseLength() const {
	
  		return numBaseLength;
 	}

 	/**
 	 * Set numBaseLength with the specified int.
 	 * @param numBaseLength The int value to which numBaseLength is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setNumBaseLength (int numBaseLength)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numBaseLength = numBaseLength;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalSeeingRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalSeeingRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalSeeingRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get baseLength.
 	 * @return baseLength as vector<Length >
 	 */
 	vector<Length > CalSeeingRow::getBaseLength() const {
	
  		return baseLength;
 	}

 	/**
 	 * Set baseLength with the specified vector<Length >.
 	 * @param baseLength The vector<Length > value to which baseLength is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setBaseLength (vector<Length > baseLength)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->baseLength = baseLength;
	
 	}
	
	

	

	
 	/**
 	 * Get corrPhaseRms.
 	 * @return corrPhaseRms as vector<Angle >
 	 */
 	vector<Angle > CalSeeingRow::getCorrPhaseRms() const {
	
  		return corrPhaseRms;
 	}

 	/**
 	 * Set corrPhaseRms with the specified vector<Angle >.
 	 * @param corrPhaseRms The vector<Angle > value to which corrPhaseRms is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setCorrPhaseRms (vector<Angle > corrPhaseRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->corrPhaseRms = corrPhaseRms;
	
 	}
	
	

	

	
 	/**
 	 * Get uncorrPhaseRms.
 	 * @return uncorrPhaseRms as vector<Angle >
 	 */
 	vector<Angle > CalSeeingRow::getUncorrPhaseRms() const {
	
  		return uncorrPhaseRms;
 	}

 	/**
 	 * Set uncorrPhaseRms with the specified vector<Angle >.
 	 * @param uncorrPhaseRms The vector<Angle > value to which uncorrPhaseRms is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setUncorrPhaseRms (vector<Angle > uncorrPhaseRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->uncorrPhaseRms = uncorrPhaseRms;
	
 	}
	
	

	

	
 	/**
 	 * Get seeing.
 	 * @return seeing as Angle
 	 */
 	Angle CalSeeingRow::getSeeing() const {
	
  		return seeing;
 	}

 	/**
 	 * Set seeing with the specified Angle.
 	 * @param seeing The Angle value to which seeing is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setSeeing (Angle seeing)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->seeing = seeing;
	
 	}
	
	

	

	
 	/**
 	 * Get seeingFrequency.
 	 * @return seeingFrequency as Frequency
 	 */
 	Frequency CalSeeingRow::getSeeingFrequency() const {
	
  		return seeingFrequency;
 	}

 	/**
 	 * Set seeingFrequency with the specified Frequency.
 	 * @param seeingFrequency The Frequency value to which seeingFrequency is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setSeeingFrequency (Frequency seeingFrequency)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->seeingFrequency = seeingFrequency;
	
 	}
	
	

	

	
 	/**
 	 * Get seeingFreqBandwidth.
 	 * @return seeingFreqBandwidth as Frequency
 	 */
 	Frequency CalSeeingRow::getSeeingFreqBandwidth() const {
	
  		return seeingFreqBandwidth;
 	}

 	/**
 	 * Set seeingFreqBandwidth with the specified Frequency.
 	 * @param seeingFreqBandwidth The Frequency value to which seeingFreqBandwidth is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setSeeingFreqBandwidth (Frequency seeingFreqBandwidth)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->seeingFreqBandwidth = seeingFreqBandwidth;
	
 	}
	
	

	
	/**
	 * The attribute exponent is optional. Return true if this attribute exists.
	 * @return true if and only if the exponent attribute exists. 
	 */
	bool CalSeeingRow::isExponentExists() const {
		return exponentExists;
	}
	

	
 	/**
 	 * Get exponent, which is optional.
 	 * @return exponent as float
 	 * @throw IllegalAccessException If exponent does not exist.
 	 */
 	float CalSeeingRow::getExponent() const throw(IllegalAccessException) {
		if (!exponentExists) {
			throw IllegalAccessException("exponent", "CalSeeing");
		}
	
  		return exponent;
 	}

 	/**
 	 * Set exponent with the specified float.
 	 * @param exponent The float value to which exponent is to be set.
 	 
 	
 	 */
 	void CalSeeingRow::setExponent (float exponent) {
	
 		this->exponent = exponent;
	
		exponentExists = true;
	
 	}
	
	
	/**
	 * Mark exponent, which is an optional field, as non-existent.
	 */
	void CalSeeingRow::clearExponent () {
		exponentExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalSeeingRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalSeeingRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalSeeing");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalSeeingRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalSeeingRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalSeeing");
		
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
	 CalReductionRow* CalSeeingRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalSeeingRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	/**
	 * Create a CalSeeingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalSeeingRow::CalSeeingRow (CalSeeingTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	
		exponentExists = false;
	

	
	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	CalSeeingRow::CalSeeingRow (CalSeeingTable &t, CalSeeingRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	
		exponentExists = false;
	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			numBaseLength = row.numBaseLength;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			frequencyRange = row.frequencyRange;
		
			baseLength = row.baseLength;
		
			corrPhaseRms = row.corrPhaseRms;
		
			uncorrPhaseRms = row.uncorrPhaseRms;
		
			seeing = row.seeing;
		
			seeingFrequency = row.seeingFrequency;
		
			seeingFreqBandwidth = row.seeingFreqBandwidth;
		
		
		
		
		if (row.exponentExists) {
			exponent = row.exponent;		
			exponentExists = true;
		}
		else
			exponentExists = false;
		
		}	
	}

	
	bool CalSeeingRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, int numBaseLength, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<Length > baseLength, vector<Angle > corrPhaseRms, vector<Angle > uncorrPhaseRms, Angle seeing, Frequency seeingFrequency, Frequency seeingFreqBandwidth) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numBaseLength == numBaseLength);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->baseLength == baseLength);
		
		if (!result) return false;
	

	
		
		result = result && (this->corrPhaseRms == corrPhaseRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->uncorrPhaseRms == uncorrPhaseRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->seeing == seeing);
		
		if (!result) return false;
	

	
		
		result = result && (this->seeingFrequency == seeingFrequency);
		
		if (!result) return false;
	

	
		
		result = result && (this->seeingFreqBandwidth == seeingFreqBandwidth);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalSeeingRow::compareRequiredValue(int numBaseLength, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<Length > baseLength, vector<Angle > corrPhaseRms, vector<Angle > uncorrPhaseRms, Angle seeing, Frequency seeingFrequency, Frequency seeingFreqBandwidth) {
		bool result;
		result = true;
		
	
		if (!(this->numBaseLength == numBaseLength)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->baseLength == baseLength)) return false;
	

	
		if (!(this->corrPhaseRms == corrPhaseRms)) return false;
	

	
		if (!(this->uncorrPhaseRms == uncorrPhaseRms)) return false;
	

	
		if (!(this->seeing == seeing)) return false;
	

	
		if (!(this->seeingFrequency == seeingFrequency)) return false;
	

	
		if (!(this->seeingFreqBandwidth == seeingFreqBandwidth)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalSeeingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalSeeingRow::equalByRequiredValue(CalSeeingRow* x) {
		
			
		if (this->numBaseLength != x->numBaseLength) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->baseLength != x->baseLength) return false;
			
		if (this->corrPhaseRms != x->corrPhaseRms) return false;
			
		if (this->uncorrPhaseRms != x->uncorrPhaseRms) return false;
			
		if (this->seeing != x->seeing) return false;
			
		if (this->seeingFrequency != x->seeingFrequency) return false;
			
		if (this->seeingFreqBandwidth != x->seeingFreqBandwidth) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
