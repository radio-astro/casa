
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
 * File ExecBlockRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <ExecBlockRow.h>
#include <ExecBlockTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::ExecBlockRow;
using asdm::ExecBlockTable;

using asdm::AntennaTable;
using asdm::AntennaRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	ExecBlockRow::~ExecBlockRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	ExecBlockTable &ExecBlockRow::getTable() const {
		return table;
	}
	
	void ExecBlockRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ExecBlockRowIDL struct.
	 */
	ExecBlockRowIDL *ExecBlockRow::toIDL() const {
		ExecBlockRowIDL *x = new ExecBlockRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->execBlockId = execBlockId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->execBlockNum = execBlockNum;
 				
 			
		
	

	
  		
		
		
			
				
		x->telescopeName = CORBA::string_dup(telescopeName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->configName = CORBA::string_dup(configName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		
			
		x->baseRangeMin = baseRangeMin.toIDLLength();
			
		
	

	
  		
		
		
			
		x->baseRangeMax = baseRangeMax.toIDLLength();
			
		
	

	
  		
		
		
			
		x->baseRmsMinor = baseRmsMinor.toIDLLength();
			
		
	

	
  		
		
		
			
		x->baseRmsMajor = baseRmsMajor.toIDLLength();
			
		
	

	
  		
		
		
			
		x->basePa = basePa.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->observerName = CORBA::string_dup(observerName.c_str());
				
 			
		
	

	
  		
		
		
			
		x->observingLog.length(observingLog.size());
		for (unsigned int i = 0; i < observingLog.size(); ++i) {
			
				
			x->observingLog[i] = CORBA::string_dup(observingLog.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->schedulerMode.length(schedulerMode.size());
		for (unsigned int i = 0; i < schedulerMode.size(); ++i) {
			
				
			x->schedulerMode[i] = CORBA::string_dup(schedulerMode.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->projectId = projectId.toIDLEntityRef();
			
		
	

	
  		
		
		
			
		x->siteLongitude = siteLongitude.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->siteLatitude = siteLatitude.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->siteAltitude = siteAltitude.toIDLLength();
			
		
	

	
  		
		
		x->releaseDateExists = releaseDateExists;
		
		
			
		x->releaseDate = releaseDate.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
  		
		
		
			
		x->execBlockUID = execBlockUID.toIDLEntityRef();
			
		
	

	
  		
		
		
			
				
		x->aborted = aborted;
 				
 			
		
	

	
	
		
	
  	
 		
		
		
		x->antennaId.length(antennaId.size());
		for (unsigned int i = 0; i < antennaId.size(); ++i) {
			
			x->antennaId[i] = antennaId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ExecBlockRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void ExecBlockRow::setFromIDL (ExecBlockRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setExecBlockId(Tag (x.execBlockId));
			
 		
		
	

	
		
		
			
		setExecBlockNum(x.execBlockNum);
  			
 		
		
	

	
		
		
			
		setTelescopeName(string (x.telescopeName));
			
 		
		
	

	
		
		
			
		setConfigName(string (x.configName));
			
 		
		
	

	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		
			
		setBaseRangeMin(Length (x.baseRangeMin));
			
 		
		
	

	
		
		
			
		setBaseRangeMax(Length (x.baseRangeMax));
			
 		
		
	

	
		
		
			
		setBaseRmsMinor(Length (x.baseRmsMinor));
			
 		
		
	

	
		
		
			
		setBaseRmsMajor(Length (x.baseRmsMajor));
			
 		
		
	

	
		
		
			
		setBasePa(Angle (x.basePa));
			
 		
		
	

	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setObserverName(string (x.observerName));
			
 		
		
	

	
		
		
			
		observingLog .clear();
		for (unsigned int i = 0; i <x.observingLog.length(); ++i) {
			
			observingLog.push_back(string (x.observingLog[i]));
			
		}
			
  		
		
	

	
		
		
			
		schedulerMode .clear();
		for (unsigned int i = 0; i <x.schedulerMode.length(); ++i) {
			
			schedulerMode.push_back(string (x.schedulerMode[i]));
			
		}
			
  		
		
	

	
		
		
			
		setProjectId(EntityRef (x.projectId));
			
 		
		
	

	
		
		
			
		setSiteLongitude(Angle (x.siteLongitude));
			
 		
		
	

	
		
		
			
		setSiteLatitude(Angle (x.siteLatitude));
			
 		
		
	

	
		
		
			
		setSiteAltitude(Length (x.siteAltitude));
			
 		
		
	

	
		
		releaseDateExists = x.releaseDateExists;
		if (x.releaseDateExists) {
		
		
			
		setReleaseDate(ArrayTime (x.releaseDate));
			
 		
		
		}
		
	

	
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
	

	
		
		
			
		setExecBlockUID(EntityRef (x.execBlockUID));
			
 		
		
	

	
		
		
			
		setAborted(x.aborted);
  			
 		
		
	

	
	
		
	
		
		antennaId .clear();
		for (unsigned int i = 0; i <x.antennaId.length(); ++i) {
			
			antennaId.push_back(Tag (x.antennaId[i]));
			
		}
		
  	

	
		
	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"ExecBlock");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string ExecBlockRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(execBlockId, "execBlockId", buf);
		
		
	

  	
 		
		
		Parser::toXML(execBlockNum, "execBlockNum", buf);
		
		
	

  	
 		
		
		Parser::toXML(telescopeName, "telescopeName", buf);
		
		
	

  	
 		
		
		Parser::toXML(configName, "configName", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseRangeMin, "baseRangeMin", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseRangeMax, "baseRangeMax", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseRmsMinor, "baseRmsMinor", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseRmsMajor, "baseRmsMajor", buf);
		
		
	

  	
 		
		
		Parser::toXML(basePa, "basePa", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(observerName, "observerName", buf);
		
		
	

  	
 		
		
		Parser::toXML(observingLog, "observingLog", buf);
		
		
	

  	
 		
		
		Parser::toXML(schedulerMode, "schedulerMode", buf);
		
		
	

  	
 		
		
		Parser::toXML(projectId, "projectId", buf);
		
		
	

  	
 		
		
		Parser::toXML(siteLongitude, "siteLongitude", buf);
		
		
	

  	
 		
		
		Parser::toXML(siteLatitude, "siteLatitude", buf);
		
		
	

  	
 		
		
		Parser::toXML(siteAltitude, "siteAltitude", buf);
		
		
	

  	
 		
		if (releaseDateExists) {
		
		
		Parser::toXML(releaseDate, "releaseDate", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
	

  	
 		
		
		Parser::toXML(execBlockUID, "execBlockUID", buf);
		
		
	

  	
 		
		
		Parser::toXML(aborted, "aborted", buf);
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void ExecBlockRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setExecBlockId(Parser::getTag("execBlockId","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setExecBlockNum(Parser::getInteger("execBlockNum","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setTelescopeName(Parser::getString("telescopeName","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setConfigName(Parser::getString("configName","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBaseRangeMin(Parser::getLength("baseRangeMin","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBaseRangeMax(Parser::getLength("baseRangeMax","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBaseRmsMinor(Parser::getLength("baseRmsMinor","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBaseRmsMajor(Parser::getLength("baseRmsMajor","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBasePa(Parser::getAngle("basePa","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setObserverName(Parser::getString("observerName","ExecBlock",rowDoc));
			
		
	

	
  		
			
					
	  	setObservingLog(Parser::get1DString("observingLog","ExecBlock",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setSchedulerMode(Parser::get1DString("schedulerMode","ExecBlock",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setProjectId(Parser::getEntityRef("projectId","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSiteLongitude(Parser::getAngle("siteLongitude","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSiteLatitude(Parser::getAngle("siteLatitude","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSiteAltitude(Parser::getLength("siteAltitude","ExecBlock",rowDoc));
			
		
	

	
  		
        if (row.isStr("<releaseDate>")) {
			
	  		setReleaseDate(Parser::getArrayTime("releaseDate","ExecBlock",rowDoc));
			
		}
 		
	

	
  		
			
	  	setFlagRow(Parser::getBoolean("flagRow","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setExecBlockUID(Parser::getEntityRef("execBlockUID","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setAborted(Parser::getBoolean("aborted","ExecBlock",rowDoc));
			
		
	

	
	
		
	
  		 
  		setAntennaId(Parser::get1DTag("antennaId","ExecBlock",rowDoc));
		
  	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"ExecBlock");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag ExecBlockRow::getExecBlockId() const {
	
  		return execBlockId;
 	}

 	/**
 	 * Set execBlockId with the specified Tag.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ExecBlockRow::setExecBlockId (Tag execBlockId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("execBlockId", "ExecBlock");
		
  		}
  	
 		this->execBlockId = execBlockId;
	
 	}
	
	

	

	
 	/**
 	 * Get execBlockNum.
 	 * @return execBlockNum as int
 	 */
 	int ExecBlockRow::getExecBlockNum() const {
	
  		return execBlockNum;
 	}

 	/**
 	 * Set execBlockNum with the specified int.
 	 * @param execBlockNum The int value to which execBlockNum is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setExecBlockNum (int execBlockNum)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->execBlockNum = execBlockNum;
	
 	}
	
	

	

	
 	/**
 	 * Get telescopeName.
 	 * @return telescopeName as string
 	 */
 	string ExecBlockRow::getTelescopeName() const {
	
  		return telescopeName;
 	}

 	/**
 	 * Set telescopeName with the specified string.
 	 * @param telescopeName The string value to which telescopeName is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setTelescopeName (string telescopeName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->telescopeName = telescopeName;
	
 	}
	
	

	

	
 	/**
 	 * Get configName.
 	 * @return configName as string
 	 */
 	string ExecBlockRow::getConfigName() const {
	
  		return configName;
 	}

 	/**
 	 * Set configName with the specified string.
 	 * @param configName The string value to which configName is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setConfigName (string configName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->configName = configName;
	
 	}
	
	

	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int ExecBlockRow::getNumAntenna() const {
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setNumAntenna (int numAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAntenna = numAntenna;
	
 	}
	
	

	

	
 	/**
 	 * Get baseRangeMin.
 	 * @return baseRangeMin as Length
 	 */
 	Length ExecBlockRow::getBaseRangeMin() const {
	
  		return baseRangeMin;
 	}

 	/**
 	 * Set baseRangeMin with the specified Length.
 	 * @param baseRangeMin The Length value to which baseRangeMin is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setBaseRangeMin (Length baseRangeMin)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->baseRangeMin = baseRangeMin;
	
 	}
	
	

	

	
 	/**
 	 * Get baseRangeMax.
 	 * @return baseRangeMax as Length
 	 */
 	Length ExecBlockRow::getBaseRangeMax() const {
	
  		return baseRangeMax;
 	}

 	/**
 	 * Set baseRangeMax with the specified Length.
 	 * @param baseRangeMax The Length value to which baseRangeMax is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setBaseRangeMax (Length baseRangeMax)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->baseRangeMax = baseRangeMax;
	
 	}
	
	

	

	
 	/**
 	 * Get baseRmsMinor.
 	 * @return baseRmsMinor as Length
 	 */
 	Length ExecBlockRow::getBaseRmsMinor() const {
	
  		return baseRmsMinor;
 	}

 	/**
 	 * Set baseRmsMinor with the specified Length.
 	 * @param baseRmsMinor The Length value to which baseRmsMinor is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setBaseRmsMinor (Length baseRmsMinor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->baseRmsMinor = baseRmsMinor;
	
 	}
	
	

	

	
 	/**
 	 * Get baseRmsMajor.
 	 * @return baseRmsMajor as Length
 	 */
 	Length ExecBlockRow::getBaseRmsMajor() const {
	
  		return baseRmsMajor;
 	}

 	/**
 	 * Set baseRmsMajor with the specified Length.
 	 * @param baseRmsMajor The Length value to which baseRmsMajor is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setBaseRmsMajor (Length baseRmsMajor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->baseRmsMajor = baseRmsMajor;
	
 	}
	
	

	

	
 	/**
 	 * Get basePa.
 	 * @return basePa as Angle
 	 */
 	Angle ExecBlockRow::getBasePa() const {
	
  		return basePa;
 	}

 	/**
 	 * Set basePa with the specified Angle.
 	 * @param basePa The Angle value to which basePa is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setBasePa (Angle basePa)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->basePa = basePa;
	
 	}
	
	

	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval ExecBlockRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get observerName.
 	 * @return observerName as string
 	 */
 	string ExecBlockRow::getObserverName() const {
	
  		return observerName;
 	}

 	/**
 	 * Set observerName with the specified string.
 	 * @param observerName The string value to which observerName is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setObserverName (string observerName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->observerName = observerName;
	
 	}
	
	

	

	
 	/**
 	 * Get observingLog.
 	 * @return observingLog as vector<string >
 	 */
 	vector<string > ExecBlockRow::getObservingLog() const {
	
  		return observingLog;
 	}

 	/**
 	 * Set observingLog with the specified vector<string >.
 	 * @param observingLog The vector<string > value to which observingLog is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setObservingLog (vector<string > observingLog)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->observingLog = observingLog;
	
 	}
	
	

	

	
 	/**
 	 * Get schedulerMode.
 	 * @return schedulerMode as vector<string >
 	 */
 	vector<string > ExecBlockRow::getSchedulerMode() const {
	
  		return schedulerMode;
 	}

 	/**
 	 * Set schedulerMode with the specified vector<string >.
 	 * @param schedulerMode The vector<string > value to which schedulerMode is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setSchedulerMode (vector<string > schedulerMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->schedulerMode = schedulerMode;
	
 	}
	
	

	

	
 	/**
 	 * Get projectId.
 	 * @return projectId as EntityRef
 	 */
 	EntityRef ExecBlockRow::getProjectId() const {
	
  		return projectId;
 	}

 	/**
 	 * Set projectId with the specified EntityRef.
 	 * @param projectId The EntityRef value to which projectId is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setProjectId (EntityRef projectId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->projectId = projectId;
	
 	}
	
	

	

	
 	/**
 	 * Get siteLongitude.
 	 * @return siteLongitude as Angle
 	 */
 	Angle ExecBlockRow::getSiteLongitude() const {
	
  		return siteLongitude;
 	}

 	/**
 	 * Set siteLongitude with the specified Angle.
 	 * @param siteLongitude The Angle value to which siteLongitude is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setSiteLongitude (Angle siteLongitude)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->siteLongitude = siteLongitude;
	
 	}
	
	

	

	
 	/**
 	 * Get siteLatitude.
 	 * @return siteLatitude as Angle
 	 */
 	Angle ExecBlockRow::getSiteLatitude() const {
	
  		return siteLatitude;
 	}

 	/**
 	 * Set siteLatitude with the specified Angle.
 	 * @param siteLatitude The Angle value to which siteLatitude is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setSiteLatitude (Angle siteLatitude)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->siteLatitude = siteLatitude;
	
 	}
	
	

	

	
 	/**
 	 * Get siteAltitude.
 	 * @return siteAltitude as Length
 	 */
 	Length ExecBlockRow::getSiteAltitude() const {
	
  		return siteAltitude;
 	}

 	/**
 	 * Set siteAltitude with the specified Length.
 	 * @param siteAltitude The Length value to which siteAltitude is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setSiteAltitude (Length siteAltitude)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->siteAltitude = siteAltitude;
	
 	}
	
	

	
	/**
	 * The attribute releaseDate is optional. Return true if this attribute exists.
	 * @return true if and only if the releaseDate attribute exists. 
	 */
	bool ExecBlockRow::isReleaseDateExists() const {
		return releaseDateExists;
	}
	

	
 	/**
 	 * Get releaseDate, which is optional.
 	 * @return releaseDate as ArrayTime
 	 * @throw IllegalAccessException If releaseDate does not exist.
 	 */
 	ArrayTime ExecBlockRow::getReleaseDate() const throw(IllegalAccessException) {
		if (!releaseDateExists) {
			throw IllegalAccessException("releaseDate", "ExecBlock");
		}
	
  		return releaseDate;
 	}

 	/**
 	 * Set releaseDate with the specified ArrayTime.
 	 * @param releaseDate The ArrayTime value to which releaseDate is to be set.
 	 
 	
 	 */
 	void ExecBlockRow::setReleaseDate (ArrayTime releaseDate) {
	
 		this->releaseDate = releaseDate;
	
		releaseDateExists = true;
	
 	}
	
	
	/**
	 * Mark releaseDate, which is an optional field, as non-existent.
	 */
	void ExecBlockRow::clearReleaseDate () {
		releaseDateExists = false;
	}
	

	

	
 	/**
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool ExecBlockRow::getFlagRow() const {
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setFlagRow (bool flagRow)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagRow = flagRow;
	
 	}
	
	

	

	
 	/**
 	 * Get execBlockUID.
 	 * @return execBlockUID as EntityRef
 	 */
 	EntityRef ExecBlockRow::getExecBlockUID() const {
	
  		return execBlockUID;
 	}

 	/**
 	 * Set execBlockUID with the specified EntityRef.
 	 * @param execBlockUID The EntityRef value to which execBlockUID is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setExecBlockUID (EntityRef execBlockUID)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->execBlockUID = execBlockUID;
	
 	}
	
	

	

	
 	/**
 	 * Get aborted.
 	 * @return aborted as bool
 	 */
 	bool ExecBlockRow::getAborted() const {
	
  		return aborted;
 	}

 	/**
 	 * Set aborted with the specified bool.
 	 * @param aborted The bool value to which aborted is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setAborted (bool aborted)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->aborted = aborted;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as vector<Tag> 
 	 */
 	vector<Tag>  ExecBlockRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified vector<Tag> .
 	 * @param antennaId The vector<Tag>  value to which antennaId is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setAntennaId (vector<Tag>  antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set antennaId[i] with the specified Tag.
 	 * @param i The index in antennaId where to set the Tag value.
 	 * @param antennaId The Tag value to which antennaId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void ExecBlockRow::setAntennaId (int i, Tag antennaId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->antennaId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute antennaId in table ExecBlockTable");
  		vector<Tag> ::iterator iter = this->antennaId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->antennaId.insert(this->antennaId.erase(iter), antennaId); 
  	}	
 			
	
	
	
		
/**
 * Append a Tag to antennaId.
 * @param id the Tag to be appended to antennaId
 */
 void ExecBlockRow::addAntennaId(Tag id){
 	antennaId.push_back(id);
}

/**
 * Append an array of Tag to antennaId.
 * @param id an array of Tag to be appended to antennaId
 */
 void ExecBlockRow::addAntennaId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		antennaId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in antennaId at position i.
  *
  */
 const Tag ExecBlockRow::getAntennaId(int i) {
 	return antennaId.at(i);
 }
 
 /**
  * Returns the AntennaRow linked to this row via the Tag stored in antennaId
  * at position i.
  */
 AntennaRow* ExecBlockRow::getAntenna(int i) {
 	return table.getContainer().getAntenna().getRowByKey(antennaId.at(i));
 } 
 
 /**
  * Returns the vector of AntennaRow* linked to this row via the Tags stored in antennaId
  *
  */
 vector<AntennaRow *> ExecBlockRow::getAntennas() {
 	vector<AntennaRow *> result;
 	for (unsigned int i = 0; i < antennaId.size(); i++)
 		result.push_back(table.getContainer().getAntenna().getRowByKey(antennaId.at(i)));
 		
 	return result;
 }
  

	

	
	/**
	 * Create a ExecBlockRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ExecBlockRow::ExecBlockRow (ExecBlockTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		releaseDateExists = false;
	

	

	

	

	
	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	ExecBlockRow::ExecBlockRow (ExecBlockTable &t, ExecBlockRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		releaseDateExists = false;
	

	

	

	

	
	
		
		}
		else {
	
		
			execBlockId = row.execBlockId;
		
		
		
		
			antennaId = row.antennaId;
		
			execBlockNum = row.execBlockNum;
		
			telescopeName = row.telescopeName;
		
			configName = row.configName;
		
			numAntenna = row.numAntenna;
		
			baseRangeMin = row.baseRangeMin;
		
			baseRangeMax = row.baseRangeMax;
		
			baseRmsMinor = row.baseRmsMinor;
		
			baseRmsMajor = row.baseRmsMajor;
		
			basePa = row.basePa;
		
			timeInterval = row.timeInterval;
		
			observerName = row.observerName;
		
			observingLog = row.observingLog;
		
			schedulerMode = row.schedulerMode;
		
			projectId = row.projectId;
		
			siteLongitude = row.siteLongitude;
		
			siteLatitude = row.siteLatitude;
		
			siteAltitude = row.siteAltitude;
		
			flagRow = row.flagRow;
		
			execBlockUID = row.execBlockUID;
		
			aborted = row.aborted;
		
		
		
		
		if (row.releaseDateExists) {
			releaseDate = row.releaseDate;		
			releaseDateExists = true;
		}
		else
			releaseDateExists = false;
		
		}	
	}

	
	bool ExecBlockRow::compareNoAutoInc(vector<Tag>  antennaId, int execBlockNum, string telescopeName, string configName, int numAntenna, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, ArrayTimeInterval timeInterval, string observerName, vector<string > observingLog, vector<string > schedulerMode, EntityRef projectId, Angle siteLongitude, Angle siteLatitude, Length siteAltitude, bool flagRow, EntityRef execBlockUID, bool aborted) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->execBlockNum == execBlockNum);
		
		if (!result) return false;
	

	
		
		result = result && (this->telescopeName == telescopeName);
		
		if (!result) return false;
	

	
		
		result = result && (this->configName == configName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->baseRangeMin == baseRangeMin);
		
		if (!result) return false;
	

	
		
		result = result && (this->baseRangeMax == baseRangeMax);
		
		if (!result) return false;
	

	
		
		result = result && (this->baseRmsMinor == baseRmsMinor);
		
		if (!result) return false;
	

	
		
		result = result && (this->baseRmsMajor == baseRmsMajor);
		
		if (!result) return false;
	

	
		
		result = result && (this->basePa == basePa);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->observerName == observerName);
		
		if (!result) return false;
	

	
		
		result = result && (this->observingLog == observingLog);
		
		if (!result) return false;
	

	
		
		result = result && (this->schedulerMode == schedulerMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->projectId == projectId);
		
		if (!result) return false;
	

	
		
		result = result && (this->siteLongitude == siteLongitude);
		
		if (!result) return false;
	

	
		
		result = result && (this->siteLatitude == siteLatitude);
		
		if (!result) return false;
	

	
		
		result = result && (this->siteAltitude == siteAltitude);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagRow == flagRow);
		
		if (!result) return false;
	

	
		
		result = result && (this->execBlockUID == execBlockUID);
		
		if (!result) return false;
	

	
		
		result = result && (this->aborted == aborted);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool ExecBlockRow::compareRequiredValue(vector<Tag>  antennaId, int execBlockNum, string telescopeName, string configName, int numAntenna, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, ArrayTimeInterval timeInterval, string observerName, vector<string > observingLog, vector<string > schedulerMode, EntityRef projectId, Angle siteLongitude, Angle siteLatitude, Length siteAltitude, bool flagRow, EntityRef execBlockUID, bool aborted) {
		bool result;
		result = true;
		
	
		if (!(this->antennaId == antennaId)) return false;
	

	
		if (!(this->execBlockNum == execBlockNum)) return false;
	

	
		if (!(this->telescopeName == telescopeName)) return false;
	

	
		if (!(this->configName == configName)) return false;
	

	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->baseRangeMin == baseRangeMin)) return false;
	

	
		if (!(this->baseRangeMax == baseRangeMax)) return false;
	

	
		if (!(this->baseRmsMinor == baseRmsMinor)) return false;
	

	
		if (!(this->baseRmsMajor == baseRmsMajor)) return false;
	

	
		if (!(this->basePa == basePa)) return false;
	

	
		if (!(this->timeInterval == timeInterval)) return false;
	

	
		if (!(this->observerName == observerName)) return false;
	

	
		if (!(this->observingLog == observingLog)) return false;
	

	
		if (!(this->schedulerMode == schedulerMode)) return false;
	

	
		if (!(this->projectId == projectId)) return false;
	

	
		if (!(this->siteLongitude == siteLongitude)) return false;
	

	
		if (!(this->siteLatitude == siteLatitude)) return false;
	

	
		if (!(this->siteAltitude == siteAltitude)) return false;
	

	
		if (!(this->flagRow == flagRow)) return false;
	

	
		if (!(this->execBlockUID == execBlockUID)) return false;
	

	
		if (!(this->aborted == aborted)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ExecBlockRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool ExecBlockRow::equalByRequiredValue(ExecBlockRow* x) {
		
			
		if (this->antennaId != x->antennaId) return false;
			
		if (this->execBlockNum != x->execBlockNum) return false;
			
		if (this->telescopeName != x->telescopeName) return false;
			
		if (this->configName != x->configName) return false;
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->baseRangeMin != x->baseRangeMin) return false;
			
		if (this->baseRangeMax != x->baseRangeMax) return false;
			
		if (this->baseRmsMinor != x->baseRmsMinor) return false;
			
		if (this->baseRmsMajor != x->baseRmsMajor) return false;
			
		if (this->basePa != x->basePa) return false;
			
		if (this->timeInterval != x->timeInterval) return false;
			
		if (this->observerName != x->observerName) return false;
			
		if (this->observingLog != x->observingLog) return false;
			
		if (this->schedulerMode != x->schedulerMode) return false;
			
		if (this->projectId != x->projectId) return false;
			
		if (this->siteLongitude != x->siteLongitude) return false;
			
		if (this->siteLatitude != x->siteLatitude) return false;
			
		if (this->siteAltitude != x->siteAltitude) return false;
			
		if (this->flagRow != x->flagRow) return false;
			
		if (this->execBlockUID != x->execBlockUID) return false;
			
		if (this->aborted != x->aborted) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
