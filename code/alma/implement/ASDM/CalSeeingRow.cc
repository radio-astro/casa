
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

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalSeeingRow;
using asdm::CalSeeingTable;

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
	
		
	
  		
		
		
			
				
		x->atmPhaseCorrection = atmPhaseCorrection;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->integrationTime = integrationTime.toIDLInterval();
			
		
	

	
  		
		
		
			
				
		x->numBaseLengths = numBaseLengths;
 				
 			
		
	

	
  		
		
		
			
		x->baselineLengths.length(baselineLengths.size());
		for (unsigned int i = 0; i < baselineLengths.size(); ++i) {
			
			x->baselineLengths[i] = baselineLengths.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->phaseRMS.length(phaseRMS.size());
		for (unsigned int i = 0; i < phaseRMS.size(); ++i) {
			
			x->phaseRMS[i] = phaseRMS.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->seeing = seeing.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->seeingError = seeingError.toIDLAngle();
			
		
	

	
  		
		
		x->exponentExists = exponentExists;
		
		
			
				
		x->exponent = exponent;
 				
 			
		
	

	
  		
		
		x->outerScaleExists = outerScaleExists;
		
		
			
		x->outerScale = outerScale.toIDLLength();
			
		
	

	
  		
		
		x->outerScaleRMSExists = outerScaleRMSExists;
		
		
			
		x->outerScaleRMS = outerScaleRMS.toIDLAngle();
			
		
	

	
	
		
	
  	
 		
		
	 	
			
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
	void CalSeeingRow::setFromIDL (CalSeeingRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAtmPhaseCorrection(x.atmPhaseCorrection);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		setIntegrationTime(Interval (x.integrationTime));
			
 		
		
	

	
		
		
			
		setNumBaseLengths(x.numBaseLengths);
  			
 		
		
	

	
		
		
			
		baselineLengths .clear();
		for (unsigned int i = 0; i <x.baselineLengths.length(); ++i) {
			
			baselineLengths.push_back(Length (x.baselineLengths[i]));
			
		}
			
  		
		
	

	
		
		
			
		phaseRMS .clear();
		for (unsigned int i = 0; i <x.phaseRMS.length(); ++i) {
			
			phaseRMS.push_back(Angle (x.phaseRMS[i]));
			
		}
			
  		
		
	

	
		
		
			
		setSeeing(Angle (x.seeing));
			
 		
		
	

	
		
		
			
		setSeeingError(Angle (x.seeingError));
			
 		
		
	

	
		
		exponentExists = x.exponentExists;
		if (x.exponentExists) {
		
		
			
		setExponent(x.exponent);
  			
 		
		
		}
		
	

	
		
		outerScaleExists = x.outerScaleExists;
		if (x.outerScaleExists) {
		
		
			
		setOuterScale(Length (x.outerScale));
			
 		
		
		}
		
	

	
		
		outerScaleRMSExists = x.outerScaleRMSExists;
		if (x.outerScaleRMSExists) {
		
		
			
		setOuterScaleRMS(Angle (x.outerScaleRMS));
			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalSeeing");
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
		
	
		
  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrection", atmPhaseCorrection));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(integrationTime, "integrationTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(numBaseLengths, "numBaseLengths", buf);
		
		
	

  	
 		
		
		Parser::toXML(baselineLengths, "baselineLengths", buf);
		
		
	

  	
 		
		
		Parser::toXML(phaseRMS, "phaseRMS", buf);
		
		
	

  	
 		
		
		Parser::toXML(seeing, "seeing", buf);
		
		
	

  	
 		
		
		Parser::toXML(seeingError, "seeingError", buf);
		
		
	

  	
 		
		if (exponentExists) {
		
		
		Parser::toXML(exponent, "exponent", buf);
		
		
		}
		
	

  	
 		
		if (outerScaleExists) {
		
		
		Parser::toXML(outerScale, "outerScale", buf);
		
		
		}
		
	

  	
 		
		if (outerScaleRMSExists) {
		
		
		Parser::toXML(outerScaleRMS, "outerScaleRMS", buf);
		
		
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
	void CalSeeingRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
		
		
		
		atmPhaseCorrection = EnumerationParser::getAtmPhaseCorrection("atmPhaseCorrection","CalSeeing",rowDoc);
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalSeeing",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalSeeing",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalSeeing",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setIntegrationTime(Parser::getInterval("integrationTime","CalSeeing",rowDoc));
			
		
	

	
  		
			
	  	setNumBaseLengths(Parser::getInteger("numBaseLengths","CalSeeing",rowDoc));
			
		
	

	
  		
			
					
	  	setBaselineLengths(Parser::get1DLength("baselineLengths","CalSeeing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPhaseRMS(Parser::get1DAngle("phaseRMS","CalSeeing",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setSeeing(Parser::getAngle("seeing","CalSeeing",rowDoc));
			
		
	

	
  		
			
	  	setSeeingError(Parser::getAngle("seeingError","CalSeeing",rowDoc));
			
		
	

	
  		
        if (row.isStr("<exponent>")) {
			
	  		setExponent(Parser::getFloat("exponent","CalSeeing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<outerScale>")) {
			
	  		setOuterScale(Parser::getLength("outerScale","CalSeeing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<outerScaleRMS>")) {
			
	  		setOuterScaleRMS(Parser::getAngle("outerScaleRMS","CalSeeing",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalSeeing");
		}
	}
	
	void CalSeeingRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
					
			eoss.writeInt(atmPhaseCorrection);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
	Frequency::toBin(frequencyRange, eoss);
		
	

	
	
		
	integrationTime.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numBaseLengths);
				
		
	

	
	
		
	Length::toBin(baselineLengths, eoss);
		
	

	
	
		
	Angle::toBin(phaseRMS, eoss);
		
	

	
	
		
	seeing.toBin(eoss);
		
	

	
	
		
	seeingError.toBin(eoss);
		
	


	
	
	eoss.writeBoolean(exponentExists);
	if (exponentExists) {
	
	
	
		
						
			eoss.writeFloat(exponent);
				
		
	

	}

	eoss.writeBoolean(outerScaleExists);
	if (outerScaleExists) {
	
	
	
		
	outerScale.toBin(eoss);
		
	

	}

	eoss.writeBoolean(outerScaleRMSExists);
	if (outerScaleRMSExists) {
	
	
	
		
	outerScaleRMS.toBin(eoss);
		
	

	}

	}
	
	CalSeeingRow* CalSeeingRow::fromBin(EndianISStream& eiss, CalSeeingTable& table) {
		CalSeeingRow* row = new  CalSeeingRow(table);
		
		
		
	
	
		
			
		row->atmPhaseCorrection = CAtmPhaseCorrection::from_int(eiss.readInt());
			
		
	

	
		
		
		row->calDataId =  Tag::fromBin(eiss);
		
	

	
		
		
		row->calReductionId =  Tag::fromBin(eiss);
		
	

	
		
		
		row->startValidTime =  ArrayTime::fromBin(eiss);
		
	

	
		
		
		row->endValidTime =  ArrayTime::fromBin(eiss);
		
	

	
		
		
			
	
	row->frequencyRange = Frequency::from1DBin(eiss);	
	

		
	

	
		
		
		row->integrationTime =  Interval::fromBin(eiss);
		
	

	
	
		
			
		row->numBaseLengths =  eiss.readInt();
			
		
	

	
		
		
			
	
	row->baselineLengths = Length::from1DBin(eiss);	
	

		
	

	
		
		
			
	
	row->phaseRMS = Angle::from1DBin(eiss);	
	

		
	

	
		
		
		row->seeing =  Angle::fromBin(eiss);
		
	

	
		
		
		row->seeingError =  Angle::fromBin(eiss);
		
	

		
		
		
	row->exponentExists = eiss.readBoolean();
	if (row->exponentExists) {
		
	
	
		
			
		row->exponent =  eiss.readFloat();
			
		
	

	}

	row->outerScaleExists = eiss.readBoolean();
	if (row->outerScaleExists) {
		
	
		
		
		row->outerScale =  Length::fromBin(eiss);
		
	

	}

	row->outerScaleRMSExists = eiss.readBoolean();
	if (row->outerScaleRMSExists) {
		
	
		
		
		row->outerScaleRMS =  Angle::fromBin(eiss);
		
	

	}

		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as AtmPhaseCorrectionMod::AtmPhaseCorrection
 	 */
 	AtmPhaseCorrectionMod::AtmPhaseCorrection CalSeeingRow::getAtmPhaseCorrection() const {
	
  		return atmPhaseCorrection;
 	}

 	/**
 	 * Set atmPhaseCorrection with the specified AtmPhaseCorrectionMod::AtmPhaseCorrection.
 	 * @param atmPhaseCorrection The AtmPhaseCorrectionMod::AtmPhaseCorrection value to which atmPhaseCorrection is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalSeeingRow::setAtmPhaseCorrection (AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("atmPhaseCorrection", "CalSeeing");
		
  		}
  	
 		this->atmPhaseCorrection = atmPhaseCorrection;
	
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
 	 * Get integrationTime.
 	 * @return integrationTime as Interval
 	 */
 	Interval CalSeeingRow::getIntegrationTime() const {
	
  		return integrationTime;
 	}

 	/**
 	 * Set integrationTime with the specified Interval.
 	 * @param integrationTime The Interval value to which integrationTime is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setIntegrationTime (Interval integrationTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->integrationTime = integrationTime;
	
 	}
	
	

	

	
 	/**
 	 * Get numBaseLengths.
 	 * @return numBaseLengths as int
 	 */
 	int CalSeeingRow::getNumBaseLengths() const {
	
  		return numBaseLengths;
 	}

 	/**
 	 * Set numBaseLengths with the specified int.
 	 * @param numBaseLengths The int value to which numBaseLengths is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setNumBaseLengths (int numBaseLengths)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numBaseLengths = numBaseLengths;
	
 	}
	
	

	

	
 	/**
 	 * Get baselineLengths.
 	 * @return baselineLengths as vector<Length >
 	 */
 	vector<Length > CalSeeingRow::getBaselineLengths() const {
	
  		return baselineLengths;
 	}

 	/**
 	 * Set baselineLengths with the specified vector<Length >.
 	 * @param baselineLengths The vector<Length > value to which baselineLengths is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setBaselineLengths (vector<Length > baselineLengths)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->baselineLengths = baselineLengths;
	
 	}
	
	

	

	
 	/**
 	 * Get phaseRMS.
 	 * @return phaseRMS as vector<Angle >
 	 */
 	vector<Angle > CalSeeingRow::getPhaseRMS() const {
	
  		return phaseRMS;
 	}

 	/**
 	 * Set phaseRMS with the specified vector<Angle >.
 	 * @param phaseRMS The vector<Angle > value to which phaseRMS is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setPhaseRMS (vector<Angle > phaseRMS)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phaseRMS = phaseRMS;
	
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
 	 * Get seeingError.
 	 * @return seeingError as Angle
 	 */
 	Angle CalSeeingRow::getSeeingError() const {
	
  		return seeingError;
 	}

 	/**
 	 * Set seeingError with the specified Angle.
 	 * @param seeingError The Angle value to which seeingError is to be set.
 	 
 	
 		
 	 */
 	void CalSeeingRow::setSeeingError (Angle seeingError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->seeingError = seeingError;
	
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
 	float CalSeeingRow::getExponent() const  {
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
	

	
	/**
	 * The attribute outerScale is optional. Return true if this attribute exists.
	 * @return true if and only if the outerScale attribute exists. 
	 */
	bool CalSeeingRow::isOuterScaleExists() const {
		return outerScaleExists;
	}
	

	
 	/**
 	 * Get outerScale, which is optional.
 	 * @return outerScale as Length
 	 * @throw IllegalAccessException If outerScale does not exist.
 	 */
 	Length CalSeeingRow::getOuterScale() const  {
		if (!outerScaleExists) {
			throw IllegalAccessException("outerScale", "CalSeeing");
		}
	
  		return outerScale;
 	}

 	/**
 	 * Set outerScale with the specified Length.
 	 * @param outerScale The Length value to which outerScale is to be set.
 	 
 	
 	 */
 	void CalSeeingRow::setOuterScale (Length outerScale) {
	
 		this->outerScale = outerScale;
	
		outerScaleExists = true;
	
 	}
	
	
	/**
	 * Mark outerScale, which is an optional field, as non-existent.
	 */
	void CalSeeingRow::clearOuterScale () {
		outerScaleExists = false;
	}
	

	
	/**
	 * The attribute outerScaleRMS is optional. Return true if this attribute exists.
	 * @return true if and only if the outerScaleRMS attribute exists. 
	 */
	bool CalSeeingRow::isOuterScaleRMSExists() const {
		return outerScaleRMSExists;
	}
	

	
 	/**
 	 * Get outerScaleRMS, which is optional.
 	 * @return outerScaleRMS as Angle
 	 * @throw IllegalAccessException If outerScaleRMS does not exist.
 	 */
 	Angle CalSeeingRow::getOuterScaleRMS() const  {
		if (!outerScaleRMSExists) {
			throw IllegalAccessException("outerScaleRMS", "CalSeeing");
		}
	
  		return outerScaleRMS;
 	}

 	/**
 	 * Set outerScaleRMS with the specified Angle.
 	 * @param outerScaleRMS The Angle value to which outerScaleRMS is to be set.
 	 
 	
 	 */
 	void CalSeeingRow::setOuterScaleRMS (Angle outerScaleRMS) {
	
 		this->outerScaleRMS = outerScaleRMS;
	
		outerScaleRMSExists = true;
	
 	}
	
	
	/**
	 * Mark outerScaleRMS, which is an optional field, as non-existent.
	 */
	void CalSeeingRow::clearOuterScaleRMS () {
		outerScaleRMSExists = false;
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
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalSeeingRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalSeeingRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
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
	

	
		outerScaleExists = false;
	

	
		outerScaleRMSExists = false;
	

	
	

	

	
	
	
	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
atmPhaseCorrection = CAtmPhaseCorrection::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	CalSeeingRow::CalSeeingRow (CalSeeingTable &t, CalSeeingRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	
		exponentExists = false;
	

	
		outerScaleExists = false;
	

	
		outerScaleRMSExists = false;
	

	
	

	
		
		}
		else {
	
		
			atmPhaseCorrection = row.atmPhaseCorrection;
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			frequencyRange = row.frequencyRange;
		
			integrationTime = row.integrationTime;
		
			numBaseLengths = row.numBaseLengths;
		
			baselineLengths = row.baselineLengths;
		
			phaseRMS = row.phaseRMS;
		
			seeing = row.seeing;
		
			seeingError = row.seeingError;
		
		
		
		
		if (row.exponentExists) {
			exponent = row.exponent;		
			exponentExists = true;
		}
		else
			exponentExists = false;
		
		if (row.outerScaleExists) {
			outerScale = row.outerScale;		
			outerScaleExists = true;
		}
		else
			outerScaleExists = false;
		
		if (row.outerScaleRMSExists) {
			outerScaleRMS = row.outerScaleRMS;		
			outerScaleRMSExists = true;
		}
		else
			outerScaleRMSExists = false;
		
		}	
	}

	
	bool CalSeeingRow::compareNoAutoInc(AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, Interval integrationTime, int numBaseLengths, vector<Length > baselineLengths, vector<Angle > phaseRMS, Angle seeing, Angle seeingError) {
		bool result;
		result = true;
		
	
		
		result = result && (this->atmPhaseCorrection == atmPhaseCorrection);
		
		if (!result) return false;
	

	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->integrationTime == integrationTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->numBaseLengths == numBaseLengths);
		
		if (!result) return false;
	

	
		
		result = result && (this->baselineLengths == baselineLengths);
		
		if (!result) return false;
	

	
		
		result = result && (this->phaseRMS == phaseRMS);
		
		if (!result) return false;
	

	
		
		result = result && (this->seeing == seeing);
		
		if (!result) return false;
	

	
		
		result = result && (this->seeingError == seeingError);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalSeeingRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, Interval integrationTime, int numBaseLengths, vector<Length > baselineLengths, vector<Angle > phaseRMS, Angle seeing, Angle seeingError) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->integrationTime == integrationTime)) return false;
	

	
		if (!(this->numBaseLengths == numBaseLengths)) return false;
	

	
		if (!(this->baselineLengths == baselineLengths)) return false;
	

	
		if (!(this->phaseRMS == phaseRMS)) return false;
	

	
		if (!(this->seeing == seeing)) return false;
	

	
		if (!(this->seeingError == seeingError)) return false;
	

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
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->integrationTime != x->integrationTime) return false;
			
		if (this->numBaseLengths != x->numBaseLengths) return false;
			
		if (this->baselineLengths != x->baselineLengths) return false;
			
		if (this->phaseRMS != x->phaseRMS) return false;
			
		if (this->seeing != x->seeing) return false;
			
		if (this->seeingError != x->seeingError) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
