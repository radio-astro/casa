
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
 * File CalPointingRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalPointingRow.h>
#include <CalPointingTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalPointingRow;
using asdm::CalPointingTable;

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

	CalPointingRow::~CalPointingRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalPointingTable &CalPointingRow::getTable() const {
		return table;
	}
	
	void CalPointingRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPointingRowIDL struct.
	 */
	CalPointingRowIDL *CalPointingRow::toIDL() const {
		CalPointingRowIDL *x = new CalPointingRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->direction.length(direction.size());
		for (unsigned int i = 0; i < direction.size(); ++i) {
			
			x->direction[i] = direction.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->collOffsetRelative.length(collOffsetRelative.size());
		for (unsigned int i = 0; i < collOffsetRelative.size(); ++i) {
			
			x->collOffsetRelative[i] = collOffsetRelative.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->collOffsetAbsolute.length(collOffsetAbsolute.size());
		for (unsigned int i = 0; i < collOffsetAbsolute.size(); ++i) {
			
			x->collOffsetAbsolute[i] = collOffsetAbsolute.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->collError.length(collError.size());
		for (unsigned int i = 0; i < collError.size(); ++i) {
			
			x->collError[i] = collError.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->pointingMethod = pointingMethod;
 				
 			
		
	

	
  		
		
		x->beamWidthExists = beamWidthExists;
		
		
			
		x->beamWidth.length(beamWidth.size());
		for (unsigned int i = 0; i < beamWidth.size(); ++i) {
			
			x->beamWidth[i] = beamWidth.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->beamWidthErrorExists = beamWidthErrorExists;
		
		
			
		x->beamWidthError.length(beamWidthError.size());
		for (unsigned int i = 0; i < beamWidthError.size(); ++i) {
			
			x->beamWidthError[i] = beamWidthError.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->beamPAExists = beamPAExists;
		
		
			
		x->beamPA = beamPA.toIDLAngle();
			
		
	

	
  		
		
		x->beamPAErrorExists = beamPAErrorExists;
		
		
			
		x->beamPAError = beamPAError.toIDLAngle();
			
		
	

	
  		
		
		x->peakIntensityExists = peakIntensityExists;
		
		
			
		x->peakIntensity = peakIntensity.toIDLTemperature();
			
		
	

	
  		
		
		x->peakIntensityErrorExists = peakIntensityErrorExists;
		
		
			
		x->peakIntensityError = peakIntensityError.toIDLTemperature();
			
		
	

	
  		
		
		
			
				
		x->mode = mode;
 				
 			
		
	

	
  		
		
		x->beamWidthWasFixedExists = beamWidthWasFixedExists;
		
		
			
		x->beamWidthWasFixed.length(beamWidthWasFixed.size());
		for (unsigned int i = 0; i < beamWidthWasFixed.size(); ++i) {
			
				
			x->beamWidthWasFixed[i] = beamWidthWasFixed.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->beamPAWasFixedExists = beamPAWasFixedExists;
		
		
			
				
		x->beamPAWasFixed = beamPAWasFixed;
 				
 			
		
	

	
  		
		
		x->peakIntensityWasFixedExists = peakIntensityWasFixedExists;
		
		
			
				
		x->peakIntensityWasFixed = peakIntensityWasFixed;
 				
 			
		
	

	
  		
		
		
			
		x->ambientTemperature = ambientTemperature.toIDLTemperature();
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPointingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalPointingRow::setFromIDL (CalPointingRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		direction .clear();
		for (unsigned int i = 0; i <x.direction.length(); ++i) {
			
			direction.push_back(Angle (x.direction[i]));
			
		}
			
  		
		
	

	
		
		
			
		collOffsetRelative .clear();
		for (unsigned int i = 0; i <x.collOffsetRelative.length(); ++i) {
			
			collOffsetRelative.push_back(Angle (x.collOffsetRelative[i]));
			
		}
			
  		
		
	

	
		
		
			
		collOffsetAbsolute .clear();
		for (unsigned int i = 0; i <x.collOffsetAbsolute.length(); ++i) {
			
			collOffsetAbsolute.push_back(Angle (x.collOffsetAbsolute[i]));
			
		}
			
  		
		
	

	
		
		
			
		collError .clear();
		for (unsigned int i = 0; i <x.collError.length(); ++i) {
			
			collError.push_back(Angle (x.collError[i]));
			
		}
			
  		
		
	

	
		
		
			
		setPointingMethod(x.pointingMethod);
  			
 		
		
	

	
		
		beamWidthExists = x.beamWidthExists;
		if (x.beamWidthExists) {
		
		
			
		beamWidth .clear();
		for (unsigned int i = 0; i <x.beamWidth.length(); ++i) {
			
			beamWidth.push_back(Angle (x.beamWidth[i]));
			
		}
			
  		
		
		}
		
	

	
		
		beamWidthErrorExists = x.beamWidthErrorExists;
		if (x.beamWidthErrorExists) {
		
		
			
		beamWidthError .clear();
		for (unsigned int i = 0; i <x.beamWidthError.length(); ++i) {
			
			beamWidthError.push_back(Angle (x.beamWidthError[i]));
			
		}
			
  		
		
		}
		
	

	
		
		beamPAExists = x.beamPAExists;
		if (x.beamPAExists) {
		
		
			
		setBeamPA(Angle (x.beamPA));
			
 		
		
		}
		
	

	
		
		beamPAErrorExists = x.beamPAErrorExists;
		if (x.beamPAErrorExists) {
		
		
			
		setBeamPAError(Angle (x.beamPAError));
			
 		
		
		}
		
	

	
		
		peakIntensityExists = x.peakIntensityExists;
		if (x.peakIntensityExists) {
		
		
			
		setPeakIntensity(Temperature (x.peakIntensity));
			
 		
		
		}
		
	

	
		
		peakIntensityErrorExists = x.peakIntensityErrorExists;
		if (x.peakIntensityErrorExists) {
		
		
			
		setPeakIntensityError(Temperature (x.peakIntensityError));
			
 		
		
		}
		
	

	
		
		
			
		setMode(x.mode);
  			
 		
		
	

	
		
		beamWidthWasFixedExists = x.beamWidthWasFixedExists;
		if (x.beamWidthWasFixedExists) {
		
		
			
		beamWidthWasFixed .clear();
		for (unsigned int i = 0; i <x.beamWidthWasFixed.length(); ++i) {
			
			beamWidthWasFixed.push_back(x.beamWidthWasFixed[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		beamPAWasFixedExists = x.beamPAWasFixedExists;
		if (x.beamPAWasFixedExists) {
		
		
			
		setBeamPAWasFixed(x.beamPAWasFixed);
  			
 		
		
		}
		
	

	
		
		peakIntensityWasFixedExists = x.peakIntensityWasFixedExists;
		if (x.peakIntensityWasFixedExists) {
		
		
			
		setPeakIntensityWasFixed(x.peakIntensityWasFixed);
  			
 		
		
		}
		
	

	
		
		
			
		setAmbientTemperature(Temperature (x.ambientTemperature));
			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalPointing");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalPointingRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(direction, "direction", buf);
		
		
	

  	
 		
		
		Parser::toXML(collOffsetRelative, "collOffsetRelative", buf);
		
		
	

  	
 		
		
		Parser::toXML(collOffsetAbsolute, "collOffsetAbsolute", buf);
		
		
	

  	
 		
		
		Parser::toXML(collError, "collError", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("pointingMethod", pointingMethod));
		
		
	

  	
 		
		if (beamWidthExists) {
		
		
		Parser::toXML(beamWidth, "beamWidth", buf);
		
		
		}
		
	

  	
 		
		if (beamWidthErrorExists) {
		
		
		Parser::toXML(beamWidthError, "beamWidthError", buf);
		
		
		}
		
	

  	
 		
		if (beamPAExists) {
		
		
		Parser::toXML(beamPA, "beamPA", buf);
		
		
		}
		
	

  	
 		
		if (beamPAErrorExists) {
		
		
		Parser::toXML(beamPAError, "beamPAError", buf);
		
		
		}
		
	

  	
 		
		if (peakIntensityExists) {
		
		
		Parser::toXML(peakIntensity, "peakIntensity", buf);
		
		
		}
		
	

  	
 		
		if (peakIntensityErrorExists) {
		
		
		Parser::toXML(peakIntensityError, "peakIntensityError", buf);
		
		
		}
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("mode", mode));
		
		
	

  	
 		
		if (beamWidthWasFixedExists) {
		
		
		Parser::toXML(beamWidthWasFixed, "beamWidthWasFixed", buf);
		
		
		}
		
	

  	
 		
		if (beamPAWasFixedExists) {
		
		
		Parser::toXML(beamPAWasFixed, "beamPAWasFixed", buf);
		
		
		}
		
	

  	
 		
		if (peakIntensityWasFixedExists) {
		
		
		Parser::toXML(peakIntensityWasFixed, "peakIntensityWasFixed", buf);
		
		
		}
		
	

  	
 		
		
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
	void CalPointingRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalPointing",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalPointing",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalPointing",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalPointing",rowDoc);
		
		
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setDirection(Parser::get1DAngle("direction","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCollOffsetRelative(Parser::get1DAngle("collOffsetRelative","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCollOffsetAbsolute(Parser::get1DAngle("collOffsetAbsolute","CalPointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCollError(Parser::get1DAngle("collError","CalPointing",rowDoc));
	  			
	  		
		
	

	
		
		
		
		pointingMethod = EnumerationParser::getPointingMethod("pointingMethod","CalPointing",rowDoc);
		
		
		
	

	
  		
        if (row.isStr("<beamWidth>")) {
			
								
	  		setBeamWidth(Parser::get1DAngle("beamWidth","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<beamWidthError>")) {
			
								
	  		setBeamWidthError(Parser::get1DAngle("beamWidthError","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<beamPA>")) {
			
	  		setBeamPA(Parser::getAngle("beamPA","CalPointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<beamPAError>")) {
			
	  		setBeamPAError(Parser::getAngle("beamPAError","CalPointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<peakIntensity>")) {
			
	  		setPeakIntensity(Parser::getTemperature("peakIntensity","CalPointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<peakIntensityError>")) {
			
	  		setPeakIntensityError(Parser::getTemperature("peakIntensityError","CalPointing",rowDoc));
			
		}
 		
	

	
		
		
		
		mode = EnumerationParser::getPointingModelMode("mode","CalPointing",rowDoc);
		
		
		
	

	
  		
        if (row.isStr("<beamWidthWasFixed>")) {
			
								
	  		setBeamWidthWasFixed(Parser::get1DBoolean("beamWidthWasFixed","CalPointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<beamPAWasFixed>")) {
			
	  		setBeamPAWasFixed(Parser::getBoolean("beamPAWasFixed","CalPointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<peakIntensityWasFixed>")) {
			
	  		setPeakIntensityWasFixed(Parser::getBoolean("peakIntensityWasFixed","CalPointing",rowDoc));
			
		}
 		
	

	
  		
			
	  	setAmbientTemperature(Parser::getTemperature("ambientTemperature","CalPointing",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPointing");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalPointingRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalPointing");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalPointingRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalPointingRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalPointingRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalPointingRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > CalPointingRow::getDirection() const {
	
  		return direction;
 	}

 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setDirection (vector<Angle > direction)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->direction = direction;
	
 	}
	
	

	

	
 	/**
 	 * Get collOffsetRelative.
 	 * @return collOffsetRelative as vector<Angle >
 	 */
 	vector<Angle > CalPointingRow::getCollOffsetRelative() const {
	
  		return collOffsetRelative;
 	}

 	/**
 	 * Set collOffsetRelative with the specified vector<Angle >.
 	 * @param collOffsetRelative The vector<Angle > value to which collOffsetRelative is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setCollOffsetRelative (vector<Angle > collOffsetRelative)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->collOffsetRelative = collOffsetRelative;
	
 	}
	
	

	

	
 	/**
 	 * Get collOffsetAbsolute.
 	 * @return collOffsetAbsolute as vector<Angle >
 	 */
 	vector<Angle > CalPointingRow::getCollOffsetAbsolute() const {
	
  		return collOffsetAbsolute;
 	}

 	/**
 	 * Set collOffsetAbsolute with the specified vector<Angle >.
 	 * @param collOffsetAbsolute The vector<Angle > value to which collOffsetAbsolute is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setCollOffsetAbsolute (vector<Angle > collOffsetAbsolute)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->collOffsetAbsolute = collOffsetAbsolute;
	
 	}
	
	

	

	
 	/**
 	 * Get collError.
 	 * @return collError as vector<Angle >
 	 */
 	vector<Angle > CalPointingRow::getCollError() const {
	
  		return collError;
 	}

 	/**
 	 * Set collError with the specified vector<Angle >.
 	 * @param collError The vector<Angle > value to which collError is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setCollError (vector<Angle > collError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->collError = collError;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingMethod.
 	 * @return pointingMethod as PointingMethodMod::PointingMethod
 	 */
 	PointingMethodMod::PointingMethod CalPointingRow::getPointingMethod() const {
	
  		return pointingMethod;
 	}

 	/**
 	 * Set pointingMethod with the specified PointingMethodMod::PointingMethod.
 	 * @param pointingMethod The PointingMethodMod::PointingMethod value to which pointingMethod is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setPointingMethod (PointingMethodMod::PointingMethod pointingMethod)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingMethod = pointingMethod;
	
 	}
	
	

	
	/**
	 * The attribute beamWidth is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidth attribute exists. 
	 */
	bool CalPointingRow::isBeamWidthExists() const {
		return beamWidthExists;
	}
	

	
 	/**
 	 * Get beamWidth, which is optional.
 	 * @return beamWidth as vector<Angle >
 	 * @throw IllegalAccessException If beamWidth does not exist.
 	 */
 	vector<Angle > CalPointingRow::getBeamWidth() const throw(IllegalAccessException) {
		if (!beamWidthExists) {
			throw IllegalAccessException("beamWidth", "CalPointing");
		}
	
  		return beamWidth;
 	}

 	/**
 	 * Set beamWidth with the specified vector<Angle >.
 	 * @param beamWidth The vector<Angle > value to which beamWidth is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamWidth (vector<Angle > beamWidth) {
	
 		this->beamWidth = beamWidth;
	
		beamWidthExists = true;
	
 	}
	
	
	/**
	 * Mark beamWidth, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamWidth () {
		beamWidthExists = false;
	}
	

	
	/**
	 * The attribute beamWidthError is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidthError attribute exists. 
	 */
	bool CalPointingRow::isBeamWidthErrorExists() const {
		return beamWidthErrorExists;
	}
	

	
 	/**
 	 * Get beamWidthError, which is optional.
 	 * @return beamWidthError as vector<Angle >
 	 * @throw IllegalAccessException If beamWidthError does not exist.
 	 */
 	vector<Angle > CalPointingRow::getBeamWidthError() const throw(IllegalAccessException) {
		if (!beamWidthErrorExists) {
			throw IllegalAccessException("beamWidthError", "CalPointing");
		}
	
  		return beamWidthError;
 	}

 	/**
 	 * Set beamWidthError with the specified vector<Angle >.
 	 * @param beamWidthError The vector<Angle > value to which beamWidthError is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamWidthError (vector<Angle > beamWidthError) {
	
 		this->beamWidthError = beamWidthError;
	
		beamWidthErrorExists = true;
	
 	}
	
	
	/**
	 * Mark beamWidthError, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamWidthError () {
		beamWidthErrorExists = false;
	}
	

	
	/**
	 * The attribute beamPA is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPA attribute exists. 
	 */
	bool CalPointingRow::isBeamPAExists() const {
		return beamPAExists;
	}
	

	
 	/**
 	 * Get beamPA, which is optional.
 	 * @return beamPA as Angle
 	 * @throw IllegalAccessException If beamPA does not exist.
 	 */
 	Angle CalPointingRow::getBeamPA() const throw(IllegalAccessException) {
		if (!beamPAExists) {
			throw IllegalAccessException("beamPA", "CalPointing");
		}
	
  		return beamPA;
 	}

 	/**
 	 * Set beamPA with the specified Angle.
 	 * @param beamPA The Angle value to which beamPA is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamPA (Angle beamPA) {
	
 		this->beamPA = beamPA;
	
		beamPAExists = true;
	
 	}
	
	
	/**
	 * Mark beamPA, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamPA () {
		beamPAExists = false;
	}
	

	
	/**
	 * The attribute beamPAError is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPAError attribute exists. 
	 */
	bool CalPointingRow::isBeamPAErrorExists() const {
		return beamPAErrorExists;
	}
	

	
 	/**
 	 * Get beamPAError, which is optional.
 	 * @return beamPAError as Angle
 	 * @throw IllegalAccessException If beamPAError does not exist.
 	 */
 	Angle CalPointingRow::getBeamPAError() const throw(IllegalAccessException) {
		if (!beamPAErrorExists) {
			throw IllegalAccessException("beamPAError", "CalPointing");
		}
	
  		return beamPAError;
 	}

 	/**
 	 * Set beamPAError with the specified Angle.
 	 * @param beamPAError The Angle value to which beamPAError is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamPAError (Angle beamPAError) {
	
 		this->beamPAError = beamPAError;
	
		beamPAErrorExists = true;
	
 	}
	
	
	/**
	 * Mark beamPAError, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamPAError () {
		beamPAErrorExists = false;
	}
	

	
	/**
	 * The attribute peakIntensity is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensity attribute exists. 
	 */
	bool CalPointingRow::isPeakIntensityExists() const {
		return peakIntensityExists;
	}
	

	
 	/**
 	 * Get peakIntensity, which is optional.
 	 * @return peakIntensity as Temperature
 	 * @throw IllegalAccessException If peakIntensity does not exist.
 	 */
 	Temperature CalPointingRow::getPeakIntensity() const throw(IllegalAccessException) {
		if (!peakIntensityExists) {
			throw IllegalAccessException("peakIntensity", "CalPointing");
		}
	
  		return peakIntensity;
 	}

 	/**
 	 * Set peakIntensity with the specified Temperature.
 	 * @param peakIntensity The Temperature value to which peakIntensity is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setPeakIntensity (Temperature peakIntensity) {
	
 		this->peakIntensity = peakIntensity;
	
		peakIntensityExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensity, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearPeakIntensity () {
		peakIntensityExists = false;
	}
	

	
	/**
	 * The attribute peakIntensityError is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityError attribute exists. 
	 */
	bool CalPointingRow::isPeakIntensityErrorExists() const {
		return peakIntensityErrorExists;
	}
	

	
 	/**
 	 * Get peakIntensityError, which is optional.
 	 * @return peakIntensityError as Temperature
 	 * @throw IllegalAccessException If peakIntensityError does not exist.
 	 */
 	Temperature CalPointingRow::getPeakIntensityError() const throw(IllegalAccessException) {
		if (!peakIntensityErrorExists) {
			throw IllegalAccessException("peakIntensityError", "CalPointing");
		}
	
  		return peakIntensityError;
 	}

 	/**
 	 * Set peakIntensityError with the specified Temperature.
 	 * @param peakIntensityError The Temperature value to which peakIntensityError is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setPeakIntensityError (Temperature peakIntensityError) {
	
 		this->peakIntensityError = peakIntensityError;
	
		peakIntensityErrorExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensityError, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearPeakIntensityError () {
		peakIntensityErrorExists = false;
	}
	

	

	
 	/**
 	 * Get mode.
 	 * @return mode as PointingModelModeMod::PointingModelMode
 	 */
 	PointingModelModeMod::PointingModelMode CalPointingRow::getMode() const {
	
  		return mode;
 	}

 	/**
 	 * Set mode with the specified PointingModelModeMod::PointingModelMode.
 	 * @param mode The PointingModelModeMod::PointingModelMode value to which mode is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setMode (PointingModelModeMod::PointingModelMode mode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->mode = mode;
	
 	}
	
	

	
	/**
	 * The attribute beamWidthWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidthWasFixed attribute exists. 
	 */
	bool CalPointingRow::isBeamWidthWasFixedExists() const {
		return beamWidthWasFixedExists;
	}
	

	
 	/**
 	 * Get beamWidthWasFixed, which is optional.
 	 * @return beamWidthWasFixed as vector<bool >
 	 * @throw IllegalAccessException If beamWidthWasFixed does not exist.
 	 */
 	vector<bool > CalPointingRow::getBeamWidthWasFixed() const throw(IllegalAccessException) {
		if (!beamWidthWasFixedExists) {
			throw IllegalAccessException("beamWidthWasFixed", "CalPointing");
		}
	
  		return beamWidthWasFixed;
 	}

 	/**
 	 * Set beamWidthWasFixed with the specified vector<bool >.
 	 * @param beamWidthWasFixed The vector<bool > value to which beamWidthWasFixed is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamWidthWasFixed (vector<bool > beamWidthWasFixed) {
	
 		this->beamWidthWasFixed = beamWidthWasFixed;
	
		beamWidthWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark beamWidthWasFixed, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamWidthWasFixed () {
		beamWidthWasFixedExists = false;
	}
	

	
	/**
	 * The attribute beamPAWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPAWasFixed attribute exists. 
	 */
	bool CalPointingRow::isBeamPAWasFixedExists() const {
		return beamPAWasFixedExists;
	}
	

	
 	/**
 	 * Get beamPAWasFixed, which is optional.
 	 * @return beamPAWasFixed as bool
 	 * @throw IllegalAccessException If beamPAWasFixed does not exist.
 	 */
 	bool CalPointingRow::getBeamPAWasFixed() const throw(IllegalAccessException) {
		if (!beamPAWasFixedExists) {
			throw IllegalAccessException("beamPAWasFixed", "CalPointing");
		}
	
  		return beamPAWasFixed;
 	}

 	/**
 	 * Set beamPAWasFixed with the specified bool.
 	 * @param beamPAWasFixed The bool value to which beamPAWasFixed is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setBeamPAWasFixed (bool beamPAWasFixed) {
	
 		this->beamPAWasFixed = beamPAWasFixed;
	
		beamPAWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark beamPAWasFixed, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearBeamPAWasFixed () {
		beamPAWasFixedExists = false;
	}
	

	
	/**
	 * The attribute peakIntensityWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityWasFixed attribute exists. 
	 */
	bool CalPointingRow::isPeakIntensityWasFixedExists() const {
		return peakIntensityWasFixedExists;
	}
	

	
 	/**
 	 * Get peakIntensityWasFixed, which is optional.
 	 * @return peakIntensityWasFixed as bool
 	 * @throw IllegalAccessException If peakIntensityWasFixed does not exist.
 	 */
 	bool CalPointingRow::getPeakIntensityWasFixed() const throw(IllegalAccessException) {
		if (!peakIntensityWasFixedExists) {
			throw IllegalAccessException("peakIntensityWasFixed", "CalPointing");
		}
	
  		return peakIntensityWasFixed;
 	}

 	/**
 	 * Set peakIntensityWasFixed with the specified bool.
 	 * @param peakIntensityWasFixed The bool value to which peakIntensityWasFixed is to be set.
 	 
 	
 	 */
 	void CalPointingRow::setPeakIntensityWasFixed (bool peakIntensityWasFixed) {
	
 		this->peakIntensityWasFixed = peakIntensityWasFixed;
	
		peakIntensityWasFixedExists = true;
	
 	}
	
	
	/**
	 * Mark peakIntensityWasFixed, which is an optional field, as non-existent.
	 */
	void CalPointingRow::clearPeakIntensityWasFixed () {
		peakIntensityWasFixedExists = false;
	}
	

	

	
 	/**
 	 * Get ambientTemperature.
 	 * @return ambientTemperature as Temperature
 	 */
 	Temperature CalPointingRow::getAmbientTemperature() const {
	
  		return ambientTemperature;
 	}

 	/**
 	 * Set ambientTemperature with the specified Temperature.
 	 * @param ambientTemperature The Temperature value to which ambientTemperature is to be set.
 	 
 	
 		
 	 */
 	void CalPointingRow::setAmbientTemperature (Temperature ambientTemperature)  {
  	
  	
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
 	Tag CalPointingRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalPointing");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalPointingRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalPointing");
		
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
	 CalDataRow* CalPointingRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalPointingRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalPointingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPointingRow::CalPointingRow (CalPointingTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	
		beamWidthExists = false;
	

	
		beamWidthErrorExists = false;
	

	
		beamPAExists = false;
	

	
		beamPAErrorExists = false;
	

	
		peakIntensityExists = false;
	

	
		peakIntensityErrorExists = false;
	

	

	
		beamWidthWasFixedExists = false;
	

	
		beamPAWasFixedExists = false;
	

	
		peakIntensityWasFixedExists = false;
	

	

	
	

	

	
	
	
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
pointingMethod = CPointingMethod::from_int(0);
	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
mode = CPointingModelMode::from_int(0);
	

	

	

	

	
	
	}
	
	CalPointingRow::CalPointingRow (CalPointingTable &t, CalPointingRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	
		beamWidthExists = false;
	

	
		beamWidthErrorExists = false;
	

	
		beamPAExists = false;
	

	
		beamPAErrorExists = false;
	

	
		peakIntensityExists = false;
	

	
		peakIntensityErrorExists = false;
	

	

	
		beamWidthWasFixedExists = false;
	

	
		beamPAWasFixedExists = false;
	

	
		peakIntensityWasFixedExists = false;
	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
		
		
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			receiverBand = row.receiverBand;
		
			frequencyRange = row.frequencyRange;
		
			direction = row.direction;
		
			collOffsetRelative = row.collOffsetRelative;
		
			collOffsetAbsolute = row.collOffsetAbsolute;
		
			collError = row.collError;
		
			pointingMethod = row.pointingMethod;
		
			mode = row.mode;
		
			ambientTemperature = row.ambientTemperature;
		
		
		
		
		if (row.beamWidthExists) {
			beamWidth = row.beamWidth;		
			beamWidthExists = true;
		}
		else
			beamWidthExists = false;
		
		if (row.beamWidthErrorExists) {
			beamWidthError = row.beamWidthError;		
			beamWidthErrorExists = true;
		}
		else
			beamWidthErrorExists = false;
		
		if (row.beamPAExists) {
			beamPA = row.beamPA;		
			beamPAExists = true;
		}
		else
			beamPAExists = false;
		
		if (row.beamPAErrorExists) {
			beamPAError = row.beamPAError;		
			beamPAErrorExists = true;
		}
		else
			beamPAErrorExists = false;
		
		if (row.peakIntensityExists) {
			peakIntensity = row.peakIntensity;		
			peakIntensityExists = true;
		}
		else
			peakIntensityExists = false;
		
		if (row.peakIntensityErrorExists) {
			peakIntensityError = row.peakIntensityError;		
			peakIntensityErrorExists = true;
		}
		else
			peakIntensityErrorExists = false;
		
		if (row.beamWidthWasFixedExists) {
			beamWidthWasFixed = row.beamWidthWasFixed;		
			beamWidthWasFixedExists = true;
		}
		else
			beamWidthWasFixedExists = false;
		
		if (row.beamPAWasFixedExists) {
			beamPAWasFixed = row.beamPAWasFixed;		
			beamPAWasFixedExists = true;
		}
		else
			beamPAWasFixedExists = false;
		
		if (row.peakIntensityWasFixedExists) {
			peakIntensityWasFixed = row.peakIntensityWasFixed;		
			peakIntensityWasFixedExists = true;
		}
		else
			peakIntensityWasFixedExists = false;
		
		}	
	}

	
	bool CalPointingRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Frequency > frequencyRange, vector<Angle > direction, vector<Angle > collOffsetRelative, vector<Angle > collOffsetAbsolute, vector<Angle > collError, PointingMethodMod::PointingMethod pointingMethod, PointingModelModeMod::PointingModelMode mode, Temperature ambientTemperature) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->direction == direction);
		
		if (!result) return false;
	

	
		
		result = result && (this->collOffsetRelative == collOffsetRelative);
		
		if (!result) return false;
	

	
		
		result = result && (this->collOffsetAbsolute == collOffsetAbsolute);
		
		if (!result) return false;
	

	
		
		result = result && (this->collError == collError);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingMethod == pointingMethod);
		
		if (!result) return false;
	

	
		
		result = result && (this->mode == mode);
		
		if (!result) return false;
	

	
		
		result = result && (this->ambientTemperature == ambientTemperature);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalPointingRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Frequency > frequencyRange, vector<Angle > direction, vector<Angle > collOffsetRelative, vector<Angle > collOffsetAbsolute, vector<Angle > collError, PointingMethodMod::PointingMethod pointingMethod, PointingModelModeMod::PointingModelMode mode, Temperature ambientTemperature) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->direction == direction)) return false;
	

	
		if (!(this->collOffsetRelative == collOffsetRelative)) return false;
	

	
		if (!(this->collOffsetAbsolute == collOffsetAbsolute)) return false;
	

	
		if (!(this->collError == collError)) return false;
	

	
		if (!(this->pointingMethod == pointingMethod)) return false;
	

	
		if (!(this->mode == mode)) return false;
	

	
		if (!(this->ambientTemperature == ambientTemperature)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPointingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalPointingRow::equalByRequiredValue(CalPointingRow* x) {
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->direction != x->direction) return false;
			
		if (this->collOffsetRelative != x->collOffsetRelative) return false;
			
		if (this->collOffsetAbsolute != x->collOffsetAbsolute) return false;
			
		if (this->collError != x->collError) return false;
			
		if (this->pointingMethod != x->pointingMethod) return false;
			
		if (this->mode != x->mode) return false;
			
		if (this->ambientTemperature != x->ambientTemperature) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
