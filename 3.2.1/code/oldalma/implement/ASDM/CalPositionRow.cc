
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
 * File CalPositionRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalPositionRow.h>
#include <CalPositionTable.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>

#include <CalDataTable.h>
#include <CalDataRow.h>
	

using asdm::ASDM;
using asdm::CalPositionRow;
using asdm::CalPositionTable;

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

	CalPositionRow::~CalPositionRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalPositionTable &CalPositionRow::getTable() const {
		return table;
	}
	
	void CalPositionRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPositionRowIDL struct.
	 */
	CalPositionRowIDL *CalPositionRow::toIDL() const {
		CalPositionRowIDL *x = new CalPositionRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->positionOffset.length(positionOffset.size());
		for (unsigned int i = 0; i < positionOffset.size(); ++i) {
			
			x->positionOffset[i] = positionOffset.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->positionErr.length(positionErr.size());
		for (unsigned int i = 0; i < positionErr.size(); ++i) {
			
			x->positionErr[i] = positionErr.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->delayRms = delayRms.toIDLInterval();
			
		
	

	
  		
		
		
			
		x->phaseRms = phaseRms.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->axesOffset = axesOffset.toIDLLength();
			
		
	

	
  		
		
		
			
				
		x->axesOffsetFixed = axesOffsetFixed;
 				
 			
		
	

	
  		
		
		
			
		x->axesOffsetErr = axesOffsetErr.toIDLLength();
			
		
	

	
  		
		
		
			
				
		x->positionMethod = positionMethod;
 				
 			
		
	

	
  		
		
		
			
		x->refAntennaNames.length(refAntennaNames.size());
		for (unsigned int i = 0; i < refAntennaNames.size(); ++i) {
			
				
			x->refAntennaNames[i] = CORBA::string_dup(refAntennaNames.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->stationName = CORBA::string_dup(stationName.c_str());
				
 			
		
	

	
  		
		
		
			
		x->antennaPosition.length(antennaPosition.size());
		for (unsigned int i = 0; i < antennaPosition.size(); ++i) {
			
			x->antennaPosition[i] = antennaPosition.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->stationPosition.length(stationPosition.size());
		for (unsigned int i = 0; i < stationPosition.size(); ++i) {
			
			x->stationPosition[i] = stationPosition.at(i).toIDLLength();
			
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPositionRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalPositionRow::setFromIDL (CalPositionRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		positionOffset .clear();
		for (unsigned int i = 0; i <x.positionOffset.length(); ++i) {
			
			positionOffset.push_back(Length (x.positionOffset[i]));
			
		}
			
  		
		
	

	
		
		
			
		positionErr .clear();
		for (unsigned int i = 0; i <x.positionErr.length(); ++i) {
			
			positionErr.push_back(Length (x.positionErr[i]));
			
		}
			
  		
		
	

	
		
		
			
		setDelayRms(Interval (x.delayRms));
			
 		
		
	

	
		
		
			
		setPhaseRms(Angle (x.phaseRms));
			
 		
		
	

	
		
		
			
		setAxesOffset(Length (x.axesOffset));
			
 		
		
	

	
		
		
			
		setAxesOffsetFixed(x.axesOffsetFixed);
  			
 		
		
	

	
		
		
			
		setAxesOffsetErr(Length (x.axesOffsetErr));
			
 		
		
	

	
		
		
			
		setPositionMethod(x.positionMethod);
  			
 		
		
	

	
		
		
			
		refAntennaNames .clear();
		for (unsigned int i = 0; i <x.refAntennaNames.length(); ++i) {
			
			refAntennaNames.push_back(string (x.refAntennaNames[i]));
			
		}
			
  		
		
	

	
		
		
			
		setStationName(string (x.stationName));
			
 		
		
	

	
		
		
			
		antennaPosition .clear();
		for (unsigned int i = 0; i <x.antennaPosition.length(); ++i) {
			
			antennaPosition.push_back(Length (x.antennaPosition[i]));
			
		}
			
  		
		
	

	
		
		
			
		stationPosition .clear();
		for (unsigned int i = 0; i <x.stationPosition.length(); ++i) {
			
			stationPosition.push_back(Length (x.stationPosition[i]));
			
		}
			
  		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalPosition");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalPositionRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(positionOffset, "positionOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(positionErr, "positionErr", buf);
		
		
	

  	
 		
		
		Parser::toXML(delayRms, "delayRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(phaseRms, "phaseRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(axesOffset, "axesOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(axesOffsetFixed, "axesOffsetFixed", buf);
		
		
	

  	
 		
		
		Parser::toXML(axesOffsetErr, "axesOffsetErr", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("positionMethod", positionMethod));
		
		
	

  	
 		
		
		Parser::toXML(refAntennaNames, "refAntennaNames", buf);
		
		
	

  	
 		
		
		Parser::toXML(stationName, "stationName", buf);
		
		
	

  	
 		
		
		Parser::toXML(antennaPosition, "antennaPosition", buf);
		
		
	

  	
 		
		
		Parser::toXML(stationPosition, "stationPosition", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalPositionRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalPosition",rowDoc));
			
		
	

	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","CalPosition",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalPosition",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalPosition",rowDoc));
			
		
	

	
  		
			
					
	  	setPositionOffset(Parser::get1DLength("positionOffset","CalPosition",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setPositionErr(Parser::get1DLength("positionErr","CalPosition",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setDelayRms(Parser::getInterval("delayRms","CalPosition",rowDoc));
			
		
	

	
  		
			
	  	setPhaseRms(Parser::getAngle("phaseRms","CalPosition",rowDoc));
			
		
	

	
  		
			
	  	setAxesOffset(Parser::getLength("axesOffset","CalPosition",rowDoc));
			
		
	

	
  		
			
	  	setAxesOffsetFixed(Parser::getBoolean("axesOffsetFixed","CalPosition",rowDoc));
			
		
	

	
  		
			
	  	setAxesOffsetErr(Parser::getLength("axesOffsetErr","CalPosition",rowDoc));
			
		
	

	
		
		
		
		positionMethod = EnumerationParser::getPositionMethod("positionMethod","CalPosition",rowDoc);
		
		
		
	

	
  		
			
					
	  	setRefAntennaNames(Parser::get1DString("refAntennaNames","CalPosition",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setStationName(Parser::getString("stationName","CalPosition",rowDoc));
			
		
	

	
  		
			
					
	  	setAntennaPosition(Parser::get1DLength("antennaPosition","CalPosition",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setStationPosition(Parser::get1DLength("stationPosition","CalPosition",rowDoc));
	  			
	  		
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPosition");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalPositionRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPositionRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalPosition");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int CalPositionRow::getNumAntenna() const {
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setNumAntenna (int numAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAntenna = numAntenna;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalPositionRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalPositionRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get positionOffset.
 	 * @return positionOffset as vector<Length >
 	 */
 	vector<Length > CalPositionRow::getPositionOffset() const {
	
  		return positionOffset;
 	}

 	/**
 	 * Set positionOffset with the specified vector<Length >.
 	 * @param positionOffset The vector<Length > value to which positionOffset is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setPositionOffset (vector<Length > positionOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->positionOffset = positionOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get positionErr.
 	 * @return positionErr as vector<Length >
 	 */
 	vector<Length > CalPositionRow::getPositionErr() const {
	
  		return positionErr;
 	}

 	/**
 	 * Set positionErr with the specified vector<Length >.
 	 * @param positionErr The vector<Length > value to which positionErr is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setPositionErr (vector<Length > positionErr)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->positionErr = positionErr;
	
 	}
	
	

	

	
 	/**
 	 * Get delayRms.
 	 * @return delayRms as Interval
 	 */
 	Interval CalPositionRow::getDelayRms() const {
	
  		return delayRms;
 	}

 	/**
 	 * Set delayRms with the specified Interval.
 	 * @param delayRms The Interval value to which delayRms is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setDelayRms (Interval delayRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->delayRms = delayRms;
	
 	}
	
	

	

	
 	/**
 	 * Get phaseRms.
 	 * @return phaseRms as Angle
 	 */
 	Angle CalPositionRow::getPhaseRms() const {
	
  		return phaseRms;
 	}

 	/**
 	 * Set phaseRms with the specified Angle.
 	 * @param phaseRms The Angle value to which phaseRms is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setPhaseRms (Angle phaseRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phaseRms = phaseRms;
	
 	}
	
	

	

	
 	/**
 	 * Get axesOffset.
 	 * @return axesOffset as Length
 	 */
 	Length CalPositionRow::getAxesOffset() const {
	
  		return axesOffset;
 	}

 	/**
 	 * Set axesOffset with the specified Length.
 	 * @param axesOffset The Length value to which axesOffset is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setAxesOffset (Length axesOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->axesOffset = axesOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get axesOffsetFixed.
 	 * @return axesOffsetFixed as bool
 	 */
 	bool CalPositionRow::getAxesOffsetFixed() const {
	
  		return axesOffsetFixed;
 	}

 	/**
 	 * Set axesOffsetFixed with the specified bool.
 	 * @param axesOffsetFixed The bool value to which axesOffsetFixed is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setAxesOffsetFixed (bool axesOffsetFixed)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->axesOffsetFixed = axesOffsetFixed;
	
 	}
	
	

	

	
 	/**
 	 * Get axesOffsetErr.
 	 * @return axesOffsetErr as Length
 	 */
 	Length CalPositionRow::getAxesOffsetErr() const {
	
  		return axesOffsetErr;
 	}

 	/**
 	 * Set axesOffsetErr with the specified Length.
 	 * @param axesOffsetErr The Length value to which axesOffsetErr is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setAxesOffsetErr (Length axesOffsetErr)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->axesOffsetErr = axesOffsetErr;
	
 	}
	
	

	

	
 	/**
 	 * Get positionMethod.
 	 * @return positionMethod as PositionMethodMod::PositionMethod
 	 */
 	PositionMethodMod::PositionMethod CalPositionRow::getPositionMethod() const {
	
  		return positionMethod;
 	}

 	/**
 	 * Set positionMethod with the specified PositionMethodMod::PositionMethod.
 	 * @param positionMethod The PositionMethodMod::PositionMethod value to which positionMethod is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setPositionMethod (PositionMethodMod::PositionMethod positionMethod)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->positionMethod = positionMethod;
	
 	}
	
	

	

	
 	/**
 	 * Get refAntennaNames.
 	 * @return refAntennaNames as vector<string >
 	 */
 	vector<string > CalPositionRow::getRefAntennaNames() const {
	
  		return refAntennaNames;
 	}

 	/**
 	 * Set refAntennaNames with the specified vector<string >.
 	 * @param refAntennaNames The vector<string > value to which refAntennaNames is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setRefAntennaNames (vector<string > refAntennaNames)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refAntennaNames = refAntennaNames;
	
 	}
	
	

	

	
 	/**
 	 * Get stationName.
 	 * @return stationName as string
 	 */
 	string CalPositionRow::getStationName() const {
	
  		return stationName;
 	}

 	/**
 	 * Set stationName with the specified string.
 	 * @param stationName The string value to which stationName is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setStationName (string stationName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->stationName = stationName;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaPosition.
 	 * @return antennaPosition as vector<Length >
 	 */
 	vector<Length > CalPositionRow::getAntennaPosition() const {
	
  		return antennaPosition;
 	}

 	/**
 	 * Set antennaPosition with the specified vector<Length >.
 	 * @param antennaPosition The vector<Length > value to which antennaPosition is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setAntennaPosition (vector<Length > antennaPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaPosition = antennaPosition;
	
 	}
	
	

	

	
 	/**
 	 * Get stationPosition.
 	 * @return stationPosition as vector<Length >
 	 */
 	vector<Length > CalPositionRow::getStationPosition() const {
	
  		return stationPosition;
 	}

 	/**
 	 * Set stationPosition with the specified vector<Length >.
 	 * @param stationPosition The vector<Length > value to which stationPosition is to be set.
 	 
 	
 		
 	 */
 	void CalPositionRow::setStationPosition (vector<Length > stationPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->stationPosition = stationPosition;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalPositionRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPositionRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalPosition");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalPositionRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPositionRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalPosition");
		
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
	 CalReductionRow* CalPositionRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalPositionRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	/**
	 * Create a CalPositionRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPositionRow::CalPositionRow (CalPositionTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
positionMethod = CPositionMethod::from_int(0);
	

	

	

	

	
	
	}
	
	CalPositionRow::CalPositionRow (CalPositionTable &t, CalPositionRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
		
		
		
			numAntenna = row.numAntenna;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			positionOffset = row.positionOffset;
		
			positionErr = row.positionErr;
		
			delayRms = row.delayRms;
		
			phaseRms = row.phaseRms;
		
			axesOffset = row.axesOffset;
		
			axesOffsetFixed = row.axesOffsetFixed;
		
			axesOffsetErr = row.axesOffsetErr;
		
			positionMethod = row.positionMethod;
		
			refAntennaNames = row.refAntennaNames;
		
			stationName = row.stationName;
		
			antennaPosition = row.antennaPosition;
		
			stationPosition = row.stationPosition;
		
		
		
		
		}	
	}

	
	bool CalPositionRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, int numAntenna, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > positionOffset, vector<Length > positionErr, Interval delayRms, Angle phaseRms, Length axesOffset, bool axesOffsetFixed, Length axesOffsetErr, PositionMethodMod::PositionMethod positionMethod, vector<string > refAntennaNames, string stationName, vector<Length > antennaPosition, vector<Length > stationPosition) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->positionOffset == positionOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->positionErr == positionErr);
		
		if (!result) return false;
	

	
		
		result = result && (this->delayRms == delayRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->phaseRms == phaseRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->axesOffset == axesOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->axesOffsetFixed == axesOffsetFixed);
		
		if (!result) return false;
	

	
		
		result = result && (this->axesOffsetErr == axesOffsetErr);
		
		if (!result) return false;
	

	
		
		result = result && (this->positionMethod == positionMethod);
		
		if (!result) return false;
	

	
		
		result = result && (this->refAntennaNames == refAntennaNames);
		
		if (!result) return false;
	

	
		
		result = result && (this->stationName == stationName);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaPosition == antennaPosition);
		
		if (!result) return false;
	

	
		
		result = result && (this->stationPosition == stationPosition);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalPositionRow::compareRequiredValue(int numAntenna, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > positionOffset, vector<Length > positionErr, Interval delayRms, Angle phaseRms, Length axesOffset, bool axesOffsetFixed, Length axesOffsetErr, PositionMethodMod::PositionMethod positionMethod, vector<string > refAntennaNames, string stationName, vector<Length > antennaPosition, vector<Length > stationPosition) {
		bool result;
		result = true;
		
	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->positionOffset == positionOffset)) return false;
	

	
		if (!(this->positionErr == positionErr)) return false;
	

	
		if (!(this->delayRms == delayRms)) return false;
	

	
		if (!(this->phaseRms == phaseRms)) return false;
	

	
		if (!(this->axesOffset == axesOffset)) return false;
	

	
		if (!(this->axesOffsetFixed == axesOffsetFixed)) return false;
	

	
		if (!(this->axesOffsetErr == axesOffsetErr)) return false;
	

	
		if (!(this->positionMethod == positionMethod)) return false;
	

	
		if (!(this->refAntennaNames == refAntennaNames)) return false;
	

	
		if (!(this->stationName == stationName)) return false;
	

	
		if (!(this->antennaPosition == antennaPosition)) return false;
	

	
		if (!(this->stationPosition == stationPosition)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPositionRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalPositionRow::equalByRequiredValue(CalPositionRow* x) {
		
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->positionOffset != x->positionOffset) return false;
			
		if (this->positionErr != x->positionErr) return false;
			
		if (this->delayRms != x->delayRms) return false;
			
		if (this->phaseRms != x->phaseRms) return false;
			
		if (this->axesOffset != x->axesOffset) return false;
			
		if (this->axesOffsetFixed != x->axesOffsetFixed) return false;
			
		if (this->axesOffsetErr != x->axesOffsetErr) return false;
			
		if (this->positionMethod != x->positionMethod) return false;
			
		if (this->refAntennaNames != x->refAntennaNames) return false;
			
		if (this->stationName != x->stationName) return false;
			
		if (this->antennaPosition != x->antennaPosition) return false;
			
		if (this->stationPosition != x->stationPosition) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
