
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
 * File SourceRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <SourceRow.h>
#include <SourceTable.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <SourceParameterTable.h>
#include <SourceParameterRow.h>
	

using asdm::ASDM;
using asdm::SourceRow;
using asdm::SourceTable;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::SourceParameterTable;
using asdm::SourceParameterRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	SourceRow::~SourceRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	SourceTable &SourceRow::getTable() const {
		return table;
	}
	
	void SourceRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SourceRowIDL struct.
	 */
	SourceRowIDL *SourceRow::toIDL() const {
		SourceRowIDL *x = new SourceRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->sourceId = sourceId;
 				
 			
		
	

	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->numLines = numLines;
 				
 			
		
	

	
  		
		
		
			
				
		x->sourceName = CORBA::string_dup(sourceName.c_str());
				
 			
		
	

	
  		
		
		x->catalogExists = catalogExists;
		
		
			
				
		x->catalog = CORBA::string_dup(catalog.c_str());
				
 			
		
	

	
  		
		
		x->calibrationGroupExists = calibrationGroupExists;
		
		
			
				
		x->calibrationGroup = calibrationGroup;
 				
 			
		
	

	
  		
		
		
			
				
		x->code = CORBA::string_dup(code.c_str());
				
 			
		
	

	
  		
		
		
			
		x->direction.length(direction.size());
		for (unsigned int i = 0; i < direction.size(); ++i) {
			
			x->direction[i] = direction.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->positionExists = positionExists;
		
		
			
		x->position.length(position.size());
		for (unsigned int i = 0; i < position.size(); ++i) {
			
			x->position[i] = position.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->properMotion.length(properMotion.size());
		for (unsigned int i = 0; i < properMotion.size(); ++i) {
			
			x->properMotion[i] = properMotion.at(i).toIDLAngularRate();
			
	 	}
			
		
	

	
  		
		
		x->transitionExists = transitionExists;
		
		
			
		x->transition.length(transition.size());
		for (unsigned int i = 0; i < transition.size(); ++i) {
			
				
			x->transition[i] = CORBA::string_dup(transition.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		x->restFrequencyExists = restFrequencyExists;
		
		
			
		x->restFrequency.length(restFrequency.size());
		for (unsigned int i = 0; i < restFrequency.size(); ++i) {
			
			x->restFrequency[i] = restFrequency.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		x->sysVelExists = sysVelExists;
		
		
			
		x->sysVel.length(sysVel.size());
		for (unsigned int i = 0; i < sysVel.size(); ++i) {
			
			x->sysVel[i] = sysVel.at(i).toIDLSpeed();
			
	 	}
			
		
	

	
  		
		
		x->sourceModelExists = sourceModelExists;
		
		
			
				
		x->sourceModel = sourceModel;
 				
 			
		
	

	
  		
		
		x->deltaVelExists = deltaVelExists;
		
		
			
		x->deltaVel = deltaVel.toIDLSpeed();
			
		
	

	
  		
		
		x->rangeVelExists = rangeVelExists;
		
		
			
		x->rangeVel.length(rangeVel.size());
		for (unsigned int i = 0; i < rangeVel.size(); ++i) {
			
			x->rangeVel[i] = rangeVel.at(i).toIDLSpeed();
			
	 	}
			
		
	

	
	
		
	
  	
 		
 		
		x->sourceParameterIdExists = sourceParameterIdExists;
		
		
	 	
			
				
		x->sourceParameterId = sourceParameterId;
 				
 			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SourceRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void SourceRow::setFromIDL (SourceRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setSourceId(x.sourceId);
  			
 		
		
	

	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumLines(x.numLines);
  			
 		
		
	

	
		
		
			
		setSourceName(string (x.sourceName));
			
 		
		
	

	
		
		catalogExists = x.catalogExists;
		if (x.catalogExists) {
		
		
			
		setCatalog(string (x.catalog));
			
 		
		
		}
		
	

	
		
		calibrationGroupExists = x.calibrationGroupExists;
		if (x.calibrationGroupExists) {
		
		
			
		setCalibrationGroup(x.calibrationGroup);
  			
 		
		
		}
		
	

	
		
		
			
		setCode(string (x.code));
			
 		
		
	

	
		
		
			
		direction .clear();
		for (unsigned int i = 0; i <x.direction.length(); ++i) {
			
			direction.push_back(Angle (x.direction[i]));
			
		}
			
  		
		
	

	
		
		positionExists = x.positionExists;
		if (x.positionExists) {
		
		
			
		position .clear();
		for (unsigned int i = 0; i <x.position.length(); ++i) {
			
			position.push_back(Length (x.position[i]));
			
		}
			
  		
		
		}
		
	

	
		
		
			
		properMotion .clear();
		for (unsigned int i = 0; i <x.properMotion.length(); ++i) {
			
			properMotion.push_back(AngularRate (x.properMotion[i]));
			
		}
			
  		
		
	

	
		
		transitionExists = x.transitionExists;
		if (x.transitionExists) {
		
		
			
		transition .clear();
		for (unsigned int i = 0; i <x.transition.length(); ++i) {
			
			transition.push_back(string (x.transition[i]));
			
		}
			
  		
		
		}
		
	

	
		
		restFrequencyExists = x.restFrequencyExists;
		if (x.restFrequencyExists) {
		
		
			
		restFrequency .clear();
		for (unsigned int i = 0; i <x.restFrequency.length(); ++i) {
			
			restFrequency.push_back(Frequency (x.restFrequency[i]));
			
		}
			
  		
		
		}
		
	

	
		
		sysVelExists = x.sysVelExists;
		if (x.sysVelExists) {
		
		
			
		sysVel .clear();
		for (unsigned int i = 0; i <x.sysVel.length(); ++i) {
			
			sysVel.push_back(Speed (x.sysVel[i]));
			
		}
			
  		
		
		}
		
	

	
		
		sourceModelExists = x.sourceModelExists;
		if (x.sourceModelExists) {
		
		
			
		setSourceModel(x.sourceModel);
  			
 		
		
		}
		
	

	
		
		deltaVelExists = x.deltaVelExists;
		if (x.deltaVelExists) {
		
		
			
		setDeltaVel(Speed (x.deltaVel));
			
 		
		
		}
		
	

	
		
		rangeVelExists = x.rangeVelExists;
		if (x.rangeVelExists) {
		
		
			
		rangeVel .clear();
		for (unsigned int i = 0; i <x.rangeVel.length(); ++i) {
			
			rangeVel.push_back(Speed (x.rangeVel[i]));
			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		sourceParameterIdExists = x.sourceParameterIdExists;
		if (x.sourceParameterIdExists) {
		
		
			
		setSourceParameterId(x.sourceParameterId);
  			
 		
		
		}
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Source");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string SourceRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(sourceId, "sourceId", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(numLines, "numLines", buf);
		
		
	

  	
 		
		
		Parser::toXML(sourceName, "sourceName", buf);
		
		
	

  	
 		
		if (catalogExists) {
		
		
		Parser::toXML(catalog, "catalog", buf);
		
		
		}
		
	

  	
 		
		if (calibrationGroupExists) {
		
		
		Parser::toXML(calibrationGroup, "calibrationGroup", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(code, "code", buf);
		
		
	

  	
 		
		
		Parser::toXML(direction, "direction", buf);
		
		
	

  	
 		
		if (positionExists) {
		
		
		Parser::toXML(position, "position", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(properMotion, "properMotion", buf);
		
		
	

  	
 		
		if (transitionExists) {
		
		
		Parser::toXML(transition, "transition", buf);
		
		
		}
		
	

  	
 		
		if (restFrequencyExists) {
		
		
		Parser::toXML(restFrequency, "restFrequency", buf);
		
		
		}
		
	

  	
 		
		if (sysVelExists) {
		
		
		Parser::toXML(sysVel, "sysVel", buf);
		
		
		}
		
	

  	
 		
		if (sourceModelExists) {
		
		
			buf.append(EnumerationParser::toXML("sourceModel", sourceModel));
		
		
		}
		
	

  	
 		
		if (deltaVelExists) {
		
		
		Parser::toXML(deltaVel, "deltaVel", buf);
		
		
		}
		
	

  	
 		
		if (rangeVelExists) {
		
		
		Parser::toXML(rangeVel, "rangeVel", buf);
		
		
		}
		
	

	
	
		
  	
 		
		if (sourceParameterIdExists) {
		
		
		Parser::toXML(sourceParameterId, "sourceParameterId", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void SourceRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setSourceId(Parser::getInteger("sourceId","Source",rowDoc));
			
		
	

	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Source",rowDoc));
			
		
	

	
  		
			
	  	setNumLines(Parser::getInteger("numLines","Source",rowDoc));
			
		
	

	
  		
			
	  	setSourceName(Parser::getString("sourceName","Source",rowDoc));
			
		
	

	
  		
        if (row.isStr("<catalog>")) {
			
	  		setCatalog(Parser::getString("catalog","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<calibrationGroup>")) {
			
	  		setCalibrationGroup(Parser::getInteger("calibrationGroup","Source",rowDoc));
			
		}
 		
	

	
  		
			
	  	setCode(Parser::getString("code","Source",rowDoc));
			
		
	

	
  		
			
					
	  	setDirection(Parser::get1DAngle("direction","Source",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<position>")) {
			
								
	  		setPosition(Parser::get1DLength("position","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
			
					
	  	setProperMotion(Parser::get1DAngularRate("properMotion","Source",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<transition>")) {
			
								
	  		setTransition(Parser::get1DString("transition","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<restFrequency>")) {
			
								
	  		setRestFrequency(Parser::get1DFrequency("restFrequency","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<sysVel>")) {
			
								
	  		setSysVel(Parser::get1DSpeed("sysVel","Source",rowDoc));
	  			
	  		
		}
 		
	

	
		
	if (row.isStr("<sourceModel>")) {
		
		
		
		sourceModel = EnumerationParser::getSourceModel("sourceModel","Source",rowDoc);
		
		
		
		sourceModelExists = true;
	}
		
	

	
  		
        if (row.isStr("<deltaVel>")) {
			
	  		setDeltaVel(Parser::getSpeed("deltaVel","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<rangeVel>")) {
			
								
	  		setRangeVel(Parser::get1DSpeed("rangeVel","Source",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
        if (row.isStr("<sourceParameterId>")) {
			
	  		setSourceParameterId(Parser::getInteger("sourceParameterId","Source",rowDoc));
			
		}
 		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Source");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get sourceId.
 	 * @return sourceId as int
 	 */
 	int SourceRow::getSourceId() const {
	
  		return sourceId;
 	}

 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceRow::setSourceId (int sourceId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("sourceId", "Source");
		
  		}
  	
 		this->sourceId = sourceId;
	
 	}
	
	

	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval SourceRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "Source");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get numLines.
 	 * @return numLines as int
 	 */
 	int SourceRow::getNumLines() const {
	
  		return numLines;
 	}

 	/**
 	 * Set numLines with the specified int.
 	 * @param numLines The int value to which numLines is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setNumLines (int numLines)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numLines = numLines;
	
 	}
	
	

	

	
 	/**
 	 * Get sourceName.
 	 * @return sourceName as string
 	 */
 	string SourceRow::getSourceName() const {
	
  		return sourceName;
 	}

 	/**
 	 * Set sourceName with the specified string.
 	 * @param sourceName The string value to which sourceName is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setSourceName (string sourceName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sourceName = sourceName;
	
 	}
	
	

	
	/**
	 * The attribute catalog is optional. Return true if this attribute exists.
	 * @return true if and only if the catalog attribute exists. 
	 */
	bool SourceRow::isCatalogExists() const {
		return catalogExists;
	}
	

	
 	/**
 	 * Get catalog, which is optional.
 	 * @return catalog as string
 	 * @throw IllegalAccessException If catalog does not exist.
 	 */
 	string SourceRow::getCatalog() const throw(IllegalAccessException) {
		if (!catalogExists) {
			throw IllegalAccessException("catalog", "Source");
		}
	
  		return catalog;
 	}

 	/**
 	 * Set catalog with the specified string.
 	 * @param catalog The string value to which catalog is to be set.
 	 
 	
 	 */
 	void SourceRow::setCatalog (string catalog) {
	
 		this->catalog = catalog;
	
		catalogExists = true;
	
 	}
	
	
	/**
	 * Mark catalog, which is an optional field, as non-existent.
	 */
	void SourceRow::clearCatalog () {
		catalogExists = false;
	}
	

	
	/**
	 * The attribute calibrationGroup is optional. Return true if this attribute exists.
	 * @return true if and only if the calibrationGroup attribute exists. 
	 */
	bool SourceRow::isCalibrationGroupExists() const {
		return calibrationGroupExists;
	}
	

	
 	/**
 	 * Get calibrationGroup, which is optional.
 	 * @return calibrationGroup as int
 	 * @throw IllegalAccessException If calibrationGroup does not exist.
 	 */
 	int SourceRow::getCalibrationGroup() const throw(IllegalAccessException) {
		if (!calibrationGroupExists) {
			throw IllegalAccessException("calibrationGroup", "Source");
		}
	
  		return calibrationGroup;
 	}

 	/**
 	 * Set calibrationGroup with the specified int.
 	 * @param calibrationGroup The int value to which calibrationGroup is to be set.
 	 
 	
 	 */
 	void SourceRow::setCalibrationGroup (int calibrationGroup) {
	
 		this->calibrationGroup = calibrationGroup;
	
		calibrationGroupExists = true;
	
 	}
	
	
	/**
	 * Mark calibrationGroup, which is an optional field, as non-existent.
	 */
	void SourceRow::clearCalibrationGroup () {
		calibrationGroupExists = false;
	}
	

	

	
 	/**
 	 * Get code.
 	 * @return code as string
 	 */
 	string SourceRow::getCode() const {
	
  		return code;
 	}

 	/**
 	 * Set code with the specified string.
 	 * @param code The string value to which code is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setCode (string code)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->code = code;
	
 	}
	
	

	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > SourceRow::getDirection() const {
	
  		return direction;
 	}

 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setDirection (vector<Angle > direction)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->direction = direction;
	
 	}
	
	

	
	/**
	 * The attribute position is optional. Return true if this attribute exists.
	 * @return true if and only if the position attribute exists. 
	 */
	bool SourceRow::isPositionExists() const {
		return positionExists;
	}
	

	
 	/**
 	 * Get position, which is optional.
 	 * @return position as vector<Length >
 	 * @throw IllegalAccessException If position does not exist.
 	 */
 	vector<Length > SourceRow::getPosition() const throw(IllegalAccessException) {
		if (!positionExists) {
			throw IllegalAccessException("position", "Source");
		}
	
  		return position;
 	}

 	/**
 	 * Set position with the specified vector<Length >.
 	 * @param position The vector<Length > value to which position is to be set.
 	 
 	
 	 */
 	void SourceRow::setPosition (vector<Length > position) {
	
 		this->position = position;
	
		positionExists = true;
	
 	}
	
	
	/**
	 * Mark position, which is an optional field, as non-existent.
	 */
	void SourceRow::clearPosition () {
		positionExists = false;
	}
	

	

	
 	/**
 	 * Get properMotion.
 	 * @return properMotion as vector<AngularRate >
 	 */
 	vector<AngularRate > SourceRow::getProperMotion() const {
	
  		return properMotion;
 	}

 	/**
 	 * Set properMotion with the specified vector<AngularRate >.
 	 * @param properMotion The vector<AngularRate > value to which properMotion is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setProperMotion (vector<AngularRate > properMotion)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->properMotion = properMotion;
	
 	}
	
	

	
	/**
	 * The attribute transition is optional. Return true if this attribute exists.
	 * @return true if and only if the transition attribute exists. 
	 */
	bool SourceRow::isTransitionExists() const {
		return transitionExists;
	}
	

	
 	/**
 	 * Get transition, which is optional.
 	 * @return transition as vector<string >
 	 * @throw IllegalAccessException If transition does not exist.
 	 */
 	vector<string > SourceRow::getTransition() const throw(IllegalAccessException) {
		if (!transitionExists) {
			throw IllegalAccessException("transition", "Source");
		}
	
  		return transition;
 	}

 	/**
 	 * Set transition with the specified vector<string >.
 	 * @param transition The vector<string > value to which transition is to be set.
 	 
 	
 	 */
 	void SourceRow::setTransition (vector<string > transition) {
	
 		this->transition = transition;
	
		transitionExists = true;
	
 	}
	
	
	/**
	 * Mark transition, which is an optional field, as non-existent.
	 */
	void SourceRow::clearTransition () {
		transitionExists = false;
	}
	

	
	/**
	 * The attribute restFrequency is optional. Return true if this attribute exists.
	 * @return true if and only if the restFrequency attribute exists. 
	 */
	bool SourceRow::isRestFrequencyExists() const {
		return restFrequencyExists;
	}
	

	
 	/**
 	 * Get restFrequency, which is optional.
 	 * @return restFrequency as vector<Frequency >
 	 * @throw IllegalAccessException If restFrequency does not exist.
 	 */
 	vector<Frequency > SourceRow::getRestFrequency() const throw(IllegalAccessException) {
		if (!restFrequencyExists) {
			throw IllegalAccessException("restFrequency", "Source");
		}
	
  		return restFrequency;
 	}

 	/**
 	 * Set restFrequency with the specified vector<Frequency >.
 	 * @param restFrequency The vector<Frequency > value to which restFrequency is to be set.
 	 
 	
 	 */
 	void SourceRow::setRestFrequency (vector<Frequency > restFrequency) {
	
 		this->restFrequency = restFrequency;
	
		restFrequencyExists = true;
	
 	}
	
	
	/**
	 * Mark restFrequency, which is an optional field, as non-existent.
	 */
	void SourceRow::clearRestFrequency () {
		restFrequencyExists = false;
	}
	

	
	/**
	 * The attribute sysVel is optional. Return true if this attribute exists.
	 * @return true if and only if the sysVel attribute exists. 
	 */
	bool SourceRow::isSysVelExists() const {
		return sysVelExists;
	}
	

	
 	/**
 	 * Get sysVel, which is optional.
 	 * @return sysVel as vector<Speed >
 	 * @throw IllegalAccessException If sysVel does not exist.
 	 */
 	vector<Speed > SourceRow::getSysVel() const throw(IllegalAccessException) {
		if (!sysVelExists) {
			throw IllegalAccessException("sysVel", "Source");
		}
	
  		return sysVel;
 	}

 	/**
 	 * Set sysVel with the specified vector<Speed >.
 	 * @param sysVel The vector<Speed > value to which sysVel is to be set.
 	 
 	
 	 */
 	void SourceRow::setSysVel (vector<Speed > sysVel) {
	
 		this->sysVel = sysVel;
	
		sysVelExists = true;
	
 	}
	
	
	/**
	 * Mark sysVel, which is an optional field, as non-existent.
	 */
	void SourceRow::clearSysVel () {
		sysVelExists = false;
	}
	

	
	/**
	 * The attribute sourceModel is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceModel attribute exists. 
	 */
	bool SourceRow::isSourceModelExists() const {
		return sourceModelExists;
	}
	

	
 	/**
 	 * Get sourceModel, which is optional.
 	 * @return sourceModel as SourceModelMod::SourceModel
 	 * @throw IllegalAccessException If sourceModel does not exist.
 	 */
 	SourceModelMod::SourceModel SourceRow::getSourceModel() const throw(IllegalAccessException) {
		if (!sourceModelExists) {
			throw IllegalAccessException("sourceModel", "Source");
		}
	
  		return sourceModel;
 	}

 	/**
 	 * Set sourceModel with the specified SourceModelMod::SourceModel.
 	 * @param sourceModel The SourceModelMod::SourceModel value to which sourceModel is to be set.
 	 
 	
 	 */
 	void SourceRow::setSourceModel (SourceModelMod::SourceModel sourceModel) {
	
 		this->sourceModel = sourceModel;
	
		sourceModelExists = true;
	
 	}
	
	
	/**
	 * Mark sourceModel, which is an optional field, as non-existent.
	 */
	void SourceRow::clearSourceModel () {
		sourceModelExists = false;
	}
	

	
	/**
	 * The attribute deltaVel is optional. Return true if this attribute exists.
	 * @return true if and only if the deltaVel attribute exists. 
	 */
	bool SourceRow::isDeltaVelExists() const {
		return deltaVelExists;
	}
	

	
 	/**
 	 * Get deltaVel, which is optional.
 	 * @return deltaVel as Speed
 	 * @throw IllegalAccessException If deltaVel does not exist.
 	 */
 	Speed SourceRow::getDeltaVel() const throw(IllegalAccessException) {
		if (!deltaVelExists) {
			throw IllegalAccessException("deltaVel", "Source");
		}
	
  		return deltaVel;
 	}

 	/**
 	 * Set deltaVel with the specified Speed.
 	 * @param deltaVel The Speed value to which deltaVel is to be set.
 	 
 	
 	 */
 	void SourceRow::setDeltaVel (Speed deltaVel) {
	
 		this->deltaVel = deltaVel;
	
		deltaVelExists = true;
	
 	}
	
	
	/**
	 * Mark deltaVel, which is an optional field, as non-existent.
	 */
	void SourceRow::clearDeltaVel () {
		deltaVelExists = false;
	}
	

	
	/**
	 * The attribute rangeVel is optional. Return true if this attribute exists.
	 * @return true if and only if the rangeVel attribute exists. 
	 */
	bool SourceRow::isRangeVelExists() const {
		return rangeVelExists;
	}
	

	
 	/**
 	 * Get rangeVel, which is optional.
 	 * @return rangeVel as vector<Speed >
 	 * @throw IllegalAccessException If rangeVel does not exist.
 	 */
 	vector<Speed > SourceRow::getRangeVel() const throw(IllegalAccessException) {
		if (!rangeVelExists) {
			throw IllegalAccessException("rangeVel", "Source");
		}
	
  		return rangeVel;
 	}

 	/**
 	 * Set rangeVel with the specified vector<Speed >.
 	 * @param rangeVel The vector<Speed > value to which rangeVel is to be set.
 	 
 	
 	 */
 	void SourceRow::setRangeVel (vector<Speed > rangeVel) {
	
 		this->rangeVel = rangeVel;
	
		rangeVelExists = true;
	
 	}
	
	
	/**
	 * Mark rangeVel, which is an optional field, as non-existent.
	 */
	void SourceRow::clearRangeVel () {
		rangeVelExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	/**
	 * The attribute sourceParameterId is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceParameterId attribute exists. 
	 */
	bool SourceRow::isSourceParameterIdExists() const {
		return sourceParameterIdExists;
	}
	

	
 	/**
 	 * Get sourceParameterId, which is optional.
 	 * @return sourceParameterId as int
 	 * @throw IllegalAccessException If sourceParameterId does not exist.
 	 */
 	int SourceRow::getSourceParameterId() const throw(IllegalAccessException) {
		if (!sourceParameterIdExists) {
			throw IllegalAccessException("sourceParameterId", "Source");
		}
	
  		return sourceParameterId;
 	}

 	/**
 	 * Set sourceParameterId with the specified int.
 	 * @param sourceParameterId The int value to which sourceParameterId is to be set.
 	 
 	
 	 */
 	void SourceRow::setSourceParameterId (int sourceParameterId) {
	
 		this->sourceParameterId = sourceParameterId;
	
		sourceParameterIdExists = true;
	
 	}
	
	
	/**
	 * Mark sourceParameterId, which is an optional field, as non-existent.
	 */
	void SourceRow::clearSourceParameterId () {
		sourceParameterIdExists = false;
	}
	

	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag SourceRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "Source");
		
  		}
  	
 		this->spectralWindowId = spectralWindowId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* SourceRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of Source table to a collection of row of SourceParameter table.
	
	/**
	 * Get the collection of row in the SourceParameter table having their attribut sourceParameterId == this->sourceParameterId
	 */
	vector <SourceParameterRow *> SourceRow::getSourceParameters() {
		
			if (sourceParameterIdExists) {
				return table.getContainer().getSourceParameter().getRowBySourceParameterId(sourceParameterId);
			}
			else 
				throw IllegalAccessException();
		
	}
	

	

	
	/**
	 * Create a SourceRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SourceRow::SourceRow (SourceTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	
		catalogExists = false;
	

	
		calibrationGroupExists = false;
	

	

	

	
		positionExists = false;
	

	

	
		transitionExists = false;
	

	
		restFrequencyExists = false;
	

	
		sysVelExists = false;
	

	
		sourceModelExists = false;
	

	
		deltaVelExists = false;
	

	
		rangeVelExists = false;
	

	
	
		sourceParameterIdExists = false;
	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
sourceModel = CSourceModel::from_int(0);
	

	

	
	
	}
	
	SourceRow::SourceRow (SourceTable &t, SourceRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	
		catalogExists = false;
	

	
		calibrationGroupExists = false;
	

	

	

	
		positionExists = false;
	

	

	
		transitionExists = false;
	

	
		restFrequencyExists = false;
	

	
		sysVelExists = false;
	

	
		sourceModelExists = false;
	

	
		deltaVelExists = false;
	

	
		rangeVelExists = false;
	

	
	
		sourceParameterIdExists = false;
	

	
		
		}
		else {
	
		
			sourceId = row.sourceId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			numLines = row.numLines;
		
			sourceName = row.sourceName;
		
			code = row.code;
		
			direction = row.direction;
		
			properMotion = row.properMotion;
		
		
		
		
		if (row.sourceParameterIdExists) {
			sourceParameterId = row.sourceParameterId;		
			sourceParameterIdExists = true;
		}
		else
			sourceParameterIdExists = false;
		
		if (row.catalogExists) {
			catalog = row.catalog;		
			catalogExists = true;
		}
		else
			catalogExists = false;
		
		if (row.calibrationGroupExists) {
			calibrationGroup = row.calibrationGroup;		
			calibrationGroupExists = true;
		}
		else
			calibrationGroupExists = false;
		
		if (row.positionExists) {
			position = row.position;		
			positionExists = true;
		}
		else
			positionExists = false;
		
		if (row.transitionExists) {
			transition = row.transition;		
			transitionExists = true;
		}
		else
			transitionExists = false;
		
		if (row.restFrequencyExists) {
			restFrequency = row.restFrequency;		
			restFrequencyExists = true;
		}
		else
			restFrequencyExists = false;
		
		if (row.sysVelExists) {
			sysVel = row.sysVel;		
			sysVelExists = true;
		}
		else
			sysVelExists = false;
		
		if (row.sourceModelExists) {
			sourceModel = row.sourceModel;		
			sourceModelExists = true;
		}
		else
			sourceModelExists = false;
		
		if (row.deltaVelExists) {
			deltaVel = row.deltaVel;		
			deltaVelExists = true;
		}
		else
			deltaVelExists = false;
		
		if (row.rangeVelExists) {
			rangeVel = row.rangeVel;		
			rangeVelExists = true;
		}
		else
			rangeVelExists = false;
		
		}	
	}

	
	bool SourceRow::compareNoAutoInc(Tag spectralWindowId, ArrayTimeInterval timeInterval, int numLines, string sourceName, string code, vector<Angle > direction, vector<AngularRate > properMotion) {
		bool result;
		result = true;
		
	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->numLines == numLines);
		
		if (!result) return false;
	

	
		
		result = result && (this->sourceName == sourceName);
		
		if (!result) return false;
	

	
		
		result = result && (this->code == code);
		
		if (!result) return false;
	

	
		
		result = result && (this->direction == direction);
		
		if (!result) return false;
	

	
		
		result = result && (this->properMotion == properMotion);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool SourceRow::compareRequiredValue(int numLines, string sourceName, string code, vector<Angle > direction, vector<AngularRate > properMotion) {
		bool result;
		result = true;
		
	
		if (!(this->numLines == numLines)) return false;
	

	
		if (!(this->sourceName == sourceName)) return false;
	

	
		if (!(this->code == code)) return false;
	

	
		if (!(this->direction == direction)) return false;
	

	
		if (!(this->properMotion == properMotion)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SourceRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool SourceRow::equalByRequiredValue(SourceRow* x) {
		
			
		if (this->numLines != x->numLines) return false;
			
		if (this->sourceName != x->sourceName) return false;
			
		if (this->code != x->code) return false;
			
		if (this->direction != x->direction) return false;
			
		if (this->properMotion != x->properMotion) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
