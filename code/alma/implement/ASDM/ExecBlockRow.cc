
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

#include <SBSummaryTable.h>
#include <SBSummaryRow.h>
	

using asdm::ASDM;
using asdm::ExecBlockRow;
using asdm::ExecBlockTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::SBSummaryTable;
using asdm::SBSummaryRow;


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

	bool ExecBlockRow::isAdded() const {
		return hasBeenAdded;
	}	

	void ExecBlockRow::isAdded(bool added) {
		hasBeenAdded = added;
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
			
		
	

	
  		
		
		
			
		x->startTime = startTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endTime = endTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->execBlockNum = execBlockNum;
 				
 			
		
	

	
  		
		
		
			
		x->execBlockUID = execBlockUID.toIDLEntityRef();
			
		
	

	
  		
		
		
			
		x->projectId = projectId.toIDLEntityRef();
			
		
	

	
  		
		
		
			
				
		x->configName = CORBA::string_dup(configName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->telescopeName = CORBA::string_dup(telescopeName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->observerName = CORBA::string_dup(observerName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->observingLog = CORBA::string_dup(observingLog.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->sessionReference = CORBA::string_dup(sessionReference.c_str());
				
 			
		
	

	
  		
		
		
			
		x->sbSummary = sbSummary.toIDLEntityRef();
			
		
	

	
  		
		
		
			
				
		x->schedulerMode = CORBA::string_dup(schedulerMode.c_str());
				
 			
		
	

	
  		
		
		
			
		x->baseRangeMin = baseRangeMin.toIDLLength();
			
		
	

	
  		
		
		
			
		x->baseRangeMax = baseRangeMax.toIDLLength();
			
		
	

	
  		
		
		
			
		x->baseRmsMinor = baseRmsMinor.toIDLLength();
			
		
	

	
  		
		
		
			
		x->baseRmsMajor = baseRmsMajor.toIDLLength();
			
		
	

	
  		
		
		
			
		x->basePa = basePa.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->siteAltitude = siteAltitude.toIDLLength();
			
		
	

	
  		
		
		
			
		x->siteLongitude = siteLongitude.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->siteLatitude = siteLatitude.toIDLAngle();
			
		
	

	
  		
		
		
			
				
		x->aborted = aborted;
 				
 			
		
	

	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		x->releaseDateExists = releaseDateExists;
		
		
			
		x->releaseDate = releaseDate.toIDLArrayTime();
			
		
	

	
  		
		
		x->flagRowExists = flagRowExists;
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
	
  	
 		
		
		
		x->antennaId.length(antennaId.size());
		for (unsigned int i = 0; i < antennaId.size(); ++i) {
			
			x->antennaId[i] = antennaId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
		
	 	
			
		x->sBSummaryId = sBSummaryId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ExecBlockRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void ExecBlockRow::setFromIDL (ExecBlockRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setExecBlockId(Tag (x.execBlockId));
			
 		
		
	

	
		
		
			
		setStartTime(ArrayTime (x.startTime));
			
 		
		
	

	
		
		
			
		setEndTime(ArrayTime (x.endTime));
			
 		
		
	

	
		
		
			
		setExecBlockNum(x.execBlockNum);
  			
 		
		
	

	
		
		
			
		setExecBlockUID(EntityRef (x.execBlockUID));
			
 		
		
	

	
		
		
			
		setProjectId(EntityRef (x.projectId));
			
 		
		
	

	
		
		
			
		setConfigName(string (x.configName));
			
 		
		
	

	
		
		
			
		setTelescopeName(string (x.telescopeName));
			
 		
		
	

	
		
		
			
		setObserverName(string (x.observerName));
			
 		
		
	

	
		
		
			
		setObservingLog(string (x.observingLog));
			
 		
		
	

	
		
		
			
		setSessionReference(string (x.sessionReference));
			
 		
		
	

	
		
		
			
		setSbSummary(EntityRef (x.sbSummary));
			
 		
		
	

	
		
		
			
		setSchedulerMode(string (x.schedulerMode));
			
 		
		
	

	
		
		
			
		setBaseRangeMin(Length (x.baseRangeMin));
			
 		
		
	

	
		
		
			
		setBaseRangeMax(Length (x.baseRangeMax));
			
 		
		
	

	
		
		
			
		setBaseRmsMinor(Length (x.baseRmsMinor));
			
 		
		
	

	
		
		
			
		setBaseRmsMajor(Length (x.baseRmsMajor));
			
 		
		
	

	
		
		
			
		setBasePa(Angle (x.basePa));
			
 		
		
	

	
		
		
			
		setSiteAltitude(Length (x.siteAltitude));
			
 		
		
	

	
		
		
			
		setSiteLongitude(Angle (x.siteLongitude));
			
 		
		
	

	
		
		
			
		setSiteLatitude(Angle (x.siteLatitude));
			
 		
		
	

	
		
		
			
		setAborted(x.aborted);
  			
 		
		
	

	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		releaseDateExists = x.releaseDateExists;
		if (x.releaseDateExists) {
		
		
			
		setReleaseDate(ArrayTime (x.releaseDate));
			
 		
		
		}
		
	

	
		
		flagRowExists = x.flagRowExists;
		if (x.flagRowExists) {
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
		}
		
	

	
	
		
	
		
		antennaId .clear();
		for (unsigned int i = 0; i <x.antennaId.length(); ++i) {
			
			antennaId.push_back(Tag (x.antennaId[i]));
			
		}
		
  	

	
		
		
			
		setSBSummaryId(Tag (x.sBSummaryId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"ExecBlock");
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
		
		
	

  	
 		
		
		Parser::toXML(startTime, "startTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endTime, "endTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(execBlockNum, "execBlockNum", buf);
		
		
	

  	
 		
		
		Parser::toXML(execBlockUID, "execBlockUID", buf);
		
		
	

  	
 		
		
		Parser::toXML(projectId, "projectId", buf);
		
		
	

  	
 		
		
		Parser::toXML(configName, "configName", buf);
		
		
	

  	
 		
		
		Parser::toXML(telescopeName, "telescopeName", buf);
		
		
	

  	
 		
		
		Parser::toXML(observerName, "observerName", buf);
		
		
	

  	
 		
		
		Parser::toXML(observingLog, "observingLog", buf);
		
		
	

  	
 		
		
		Parser::toXML(sessionReference, "sessionReference", buf);
		
		
	

  	
 		
		
		Parser::toXML(sbSummary, "sbSummary", buf);
		
		
	

  	
 		
		
		Parser::toXML(schedulerMode, "schedulerMode", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseRangeMin, "baseRangeMin", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseRangeMax, "baseRangeMax", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseRmsMinor, "baseRmsMinor", buf);
		
		
	

  	
 		
		
		Parser::toXML(baseRmsMajor, "baseRmsMajor", buf);
		
		
	

  	
 		
		
		Parser::toXML(basePa, "basePa", buf);
		
		
	

  	
 		
		
		Parser::toXML(siteAltitude, "siteAltitude", buf);
		
		
	

  	
 		
		
		Parser::toXML(siteLongitude, "siteLongitude", buf);
		
		
	

  	
 		
		
		Parser::toXML(siteLatitude, "siteLatitude", buf);
		
		
	

  	
 		
		
		Parser::toXML(aborted, "aborted", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		if (releaseDateExists) {
		
		
		Parser::toXML(releaseDate, "releaseDate", buf);
		
		
		}
		
	

  	
 		
		if (flagRowExists) {
		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(sBSummaryId, "sBSummaryId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void ExecBlockRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setExecBlockId(Parser::getTag("execBlockId","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setStartTime(Parser::getArrayTime("startTime","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setEndTime(Parser::getArrayTime("endTime","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setExecBlockNum(Parser::getInteger("execBlockNum","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setExecBlockUID(Parser::getEntityRef("execBlockUID","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setProjectId(Parser::getEntityRef("projectId","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setConfigName(Parser::getString("configName","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setTelescopeName(Parser::getString("telescopeName","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setObserverName(Parser::getString("observerName","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setObservingLog(Parser::getString("observingLog","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSessionReference(Parser::getString("sessionReference","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSbSummary(Parser::getEntityRef("sbSummary","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSchedulerMode(Parser::getString("schedulerMode","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBaseRangeMin(Parser::getLength("baseRangeMin","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBaseRangeMax(Parser::getLength("baseRangeMax","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBaseRmsMinor(Parser::getLength("baseRmsMinor","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBaseRmsMajor(Parser::getLength("baseRmsMajor","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setBasePa(Parser::getAngle("basePa","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSiteAltitude(Parser::getLength("siteAltitude","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSiteLongitude(Parser::getAngle("siteLongitude","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setSiteLatitude(Parser::getAngle("siteLatitude","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setAborted(Parser::getBoolean("aborted","ExecBlock",rowDoc));
			
		
	

	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","ExecBlock",rowDoc));
			
		
	

	
  		
        if (row.isStr("<releaseDate>")) {
			
	  		setReleaseDate(Parser::getArrayTime("releaseDate","ExecBlock",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<flagRow>")) {
			
	  		setFlagRow(Parser::getBoolean("flagRow","ExecBlock",rowDoc));
			
		}
 		
	

	
	
		
	
  		 
  		setAntennaId(Parser::get1DTag("antennaId","ExecBlock",rowDoc));
		
  	

	
  		
			
	  	setSBSummaryId(Parser::getTag("sBSummaryId","ExecBlock",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"ExecBlock");
		}
	}
	
	void ExecBlockRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	execBlockId.toBin(eoss);
		
	

	
	
		
	startTime.toBin(eoss);
		
	

	
	
		
	endTime.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(execBlockNum);
				
		
	

	
	
		
	execBlockUID.toBin(eoss);
		
	

	
	
		
	projectId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(configName);
				
		
	

	
	
		
						
			eoss.writeString(telescopeName);
				
		
	

	
	
		
						
			eoss.writeString(observerName);
				
		
	

	
	
		
						
			eoss.writeString(observingLog);
				
		
	

	
	
		
						
			eoss.writeString(sessionReference);
				
		
	

	
	
		
	sbSummary.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(schedulerMode);
				
		
	

	
	
		
	baseRangeMin.toBin(eoss);
		
	

	
	
		
	baseRangeMax.toBin(eoss);
		
	

	
	
		
	baseRmsMinor.toBin(eoss);
		
	

	
	
		
	baseRmsMajor.toBin(eoss);
		
	

	
	
		
	basePa.toBin(eoss);
		
	

	
	
		
	siteAltitude.toBin(eoss);
		
	

	
	
		
	siteLongitude.toBin(eoss);
		
	

	
	
		
	siteLatitude.toBin(eoss);
		
	

	
	
		
						
			eoss.writeBoolean(aborted);
				
		
	

	
	
		
						
			eoss.writeInt(numAntenna);
				
		
	

	
	
		
	Tag::toBin(antennaId, eoss);
		
	

	
	
		
	sBSummaryId.toBin(eoss);
		
	


	
	
	eoss.writeBoolean(releaseDateExists);
	if (releaseDateExists) {
	
	
	
		
	releaseDate.toBin(eoss);
		
	

	}

	eoss.writeBoolean(flagRowExists);
	if (flagRowExists) {
	
	
	
		
						
			eoss.writeBoolean(flagRow);
				
		
	

	}

	}
	
void ExecBlockRow::execBlockIdFromBin(EndianISStream& eiss) {
		
	
		
		
		execBlockId =  Tag::fromBin(eiss);
		
	
	
}
void ExecBlockRow::startTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		startTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void ExecBlockRow::endTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		endTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void ExecBlockRow::execBlockNumFromBin(EndianISStream& eiss) {
		
	
	
		
			
		execBlockNum =  eiss.readInt();
			
		
	
	
}
void ExecBlockRow::execBlockUIDFromBin(EndianISStream& eiss) {
		
	
		
		
		execBlockUID =  EntityRef::fromBin(eiss);
		
	
	
}
void ExecBlockRow::projectIdFromBin(EndianISStream& eiss) {
		
	
		
		
		projectId =  EntityRef::fromBin(eiss);
		
	
	
}
void ExecBlockRow::configNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		configName =  eiss.readString();
			
		
	
	
}
void ExecBlockRow::telescopeNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		telescopeName =  eiss.readString();
			
		
	
	
}
void ExecBlockRow::observerNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		observerName =  eiss.readString();
			
		
	
	
}
void ExecBlockRow::observingLogFromBin(EndianISStream& eiss) {
		
	
	
		
			
		observingLog =  eiss.readString();
			
		
	
	
}
void ExecBlockRow::sessionReferenceFromBin(EndianISStream& eiss) {
		
	
	
		
			
		sessionReference =  eiss.readString();
			
		
	
	
}
void ExecBlockRow::sbSummaryFromBin(EndianISStream& eiss) {
		
	
		
		
		sbSummary =  EntityRef::fromBin(eiss);
		
	
	
}
void ExecBlockRow::schedulerModeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		schedulerMode =  eiss.readString();
			
		
	
	
}
void ExecBlockRow::baseRangeMinFromBin(EndianISStream& eiss) {
		
	
		
		
		baseRangeMin =  Length::fromBin(eiss);
		
	
	
}
void ExecBlockRow::baseRangeMaxFromBin(EndianISStream& eiss) {
		
	
		
		
		baseRangeMax =  Length::fromBin(eiss);
		
	
	
}
void ExecBlockRow::baseRmsMinorFromBin(EndianISStream& eiss) {
		
	
		
		
		baseRmsMinor =  Length::fromBin(eiss);
		
	
	
}
void ExecBlockRow::baseRmsMajorFromBin(EndianISStream& eiss) {
		
	
		
		
		baseRmsMajor =  Length::fromBin(eiss);
		
	
	
}
void ExecBlockRow::basePaFromBin(EndianISStream& eiss) {
		
	
		
		
		basePa =  Angle::fromBin(eiss);
		
	
	
}
void ExecBlockRow::siteAltitudeFromBin(EndianISStream& eiss) {
		
	
		
		
		siteAltitude =  Length::fromBin(eiss);
		
	
	
}
void ExecBlockRow::siteLongitudeFromBin(EndianISStream& eiss) {
		
	
		
		
		siteLongitude =  Angle::fromBin(eiss);
		
	
	
}
void ExecBlockRow::siteLatitudeFromBin(EndianISStream& eiss) {
		
	
		
		
		siteLatitude =  Angle::fromBin(eiss);
		
	
	
}
void ExecBlockRow::abortedFromBin(EndianISStream& eiss) {
		
	
	
		
			
		aborted =  eiss.readBoolean();
			
		
	
	
}
void ExecBlockRow::numAntennaFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numAntenna =  eiss.readInt();
			
		
	
	
}
void ExecBlockRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	antennaId = Tag::from1DBin(eiss);	
	

		
	
	
}
void ExecBlockRow::sBSummaryIdFromBin(EndianISStream& eiss) {
		
	
		
		
		sBSummaryId =  Tag::fromBin(eiss);
		
	
	
}

void ExecBlockRow::releaseDateFromBin(EndianISStream& eiss) {
		
	releaseDateExists = eiss.readBoolean();
	if (releaseDateExists) {
		
	
		
		
		releaseDate =  ArrayTime::fromBin(eiss);
		
	

	}
	
}
void ExecBlockRow::flagRowFromBin(EndianISStream& eiss) {
		
	flagRowExists = eiss.readBoolean();
	if (flagRowExists) {
		
	
	
		
			
		flagRow =  eiss.readBoolean();
			
		
	

	}
	
}
	
	
	ExecBlockRow* ExecBlockRow::fromBin(EndianISStream& eiss, ExecBlockTable& table, const vector<string>& attributesSeq) {
		ExecBlockRow* row = new  ExecBlockRow(table);
		
		map<string, ExecBlockAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "ExecBlockTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
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
 	 * Get startTime.
 	 * @return startTime as ArrayTime
 	 */
 	ArrayTime ExecBlockRow::getStartTime() const {
	
  		return startTime;
 	}

 	/**
 	 * Set startTime with the specified ArrayTime.
 	 * @param startTime The ArrayTime value to which startTime is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setStartTime (ArrayTime startTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startTime = startTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endTime.
 	 * @return endTime as ArrayTime
 	 */
 	ArrayTime ExecBlockRow::getEndTime() const {
	
  		return endTime;
 	}

 	/**
 	 * Set endTime with the specified ArrayTime.
 	 * @param endTime The ArrayTime value to which endTime is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setEndTime (ArrayTime endTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endTime = endTime;
	
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
 	 * @return observingLog as string
 	 */
 	string ExecBlockRow::getObservingLog() const {
	
  		return observingLog;
 	}

 	/**
 	 * Set observingLog with the specified string.
 	 * @param observingLog The string value to which observingLog is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setObservingLog (string observingLog)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->observingLog = observingLog;
	
 	}
	
	

	

	
 	/**
 	 * Get sessionReference.
 	 * @return sessionReference as string
 	 */
 	string ExecBlockRow::getSessionReference() const {
	
  		return sessionReference;
 	}

 	/**
 	 * Set sessionReference with the specified string.
 	 * @param sessionReference The string value to which sessionReference is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setSessionReference (string sessionReference)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sessionReference = sessionReference;
	
 	}
	
	

	

	
 	/**
 	 * Get sbSummary.
 	 * @return sbSummary as EntityRef
 	 */
 	EntityRef ExecBlockRow::getSbSummary() const {
	
  		return sbSummary;
 	}

 	/**
 	 * Set sbSummary with the specified EntityRef.
 	 * @param sbSummary The EntityRef value to which sbSummary is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setSbSummary (EntityRef sbSummary)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sbSummary = sbSummary;
	
 	}
	
	

	

	
 	/**
 	 * Get schedulerMode.
 	 * @return schedulerMode as string
 	 */
 	string ExecBlockRow::getSchedulerMode() const {
	
  		return schedulerMode;
 	}

 	/**
 	 * Set schedulerMode with the specified string.
 	 * @param schedulerMode The string value to which schedulerMode is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setSchedulerMode (string schedulerMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->schedulerMode = schedulerMode;
	
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
 	ArrayTime ExecBlockRow::getReleaseDate() const  {
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
	 * The attribute flagRow is optional. Return true if this attribute exists.
	 * @return true if and only if the flagRow attribute exists. 
	 */
	bool ExecBlockRow::isFlagRowExists() const {
		return flagRowExists;
	}
	

	
 	/**
 	 * Get flagRow, which is optional.
 	 * @return flagRow as bool
 	 * @throw IllegalAccessException If flagRow does not exist.
 	 */
 	bool ExecBlockRow::getFlagRow() const  {
		if (!flagRowExists) {
			throw IllegalAccessException("flagRow", "ExecBlock");
		}
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 	 */
 	void ExecBlockRow::setFlagRow (bool flagRow) {
	
 		this->flagRow = flagRow;
	
		flagRowExists = true;
	
 	}
	
	
	/**
	 * Mark flagRow, which is an optional field, as non-existent.
	 */
	void ExecBlockRow::clearFlagRow () {
		flagRowExists = false;
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
	
	

	

	
 	/**
 	 * Get sBSummaryId.
 	 * @return sBSummaryId as Tag
 	 */
 	Tag ExecBlockRow::getSBSummaryId() const {
	
  		return sBSummaryId;
 	}

 	/**
 	 * Set sBSummaryId with the specified Tag.
 	 * @param sBSummaryId The Tag value to which sBSummaryId is to be set.
 	 
 	
 		
 	 */
 	void ExecBlockRow::setSBSummaryId (Tag sBSummaryId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sBSummaryId = sBSummaryId;
	
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
	 * Returns the pointer to the row in the SBSummary table having SBSummary.sBSummaryId == sBSummaryId
	 * @return a SBSummaryRow*
	 * 
	 
	 */
	 SBSummaryRow* ExecBlockRow::getSBSummaryUsingSBSummaryId() {
	 
	 	return table.getContainer().getSBSummary().getRowByKey(sBSummaryId);
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
	

	
		flagRowExists = false;
	

	
	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["execBlockId"] = &ExecBlockRow::execBlockIdFromBin; 
	 fromBinMethods["startTime"] = &ExecBlockRow::startTimeFromBin; 
	 fromBinMethods["endTime"] = &ExecBlockRow::endTimeFromBin; 
	 fromBinMethods["execBlockNum"] = &ExecBlockRow::execBlockNumFromBin; 
	 fromBinMethods["execBlockUID"] = &ExecBlockRow::execBlockUIDFromBin; 
	 fromBinMethods["projectId"] = &ExecBlockRow::projectIdFromBin; 
	 fromBinMethods["configName"] = &ExecBlockRow::configNameFromBin; 
	 fromBinMethods["telescopeName"] = &ExecBlockRow::telescopeNameFromBin; 
	 fromBinMethods["observerName"] = &ExecBlockRow::observerNameFromBin; 
	 fromBinMethods["observingLog"] = &ExecBlockRow::observingLogFromBin; 
	 fromBinMethods["sessionReference"] = &ExecBlockRow::sessionReferenceFromBin; 
	 fromBinMethods["sbSummary"] = &ExecBlockRow::sbSummaryFromBin; 
	 fromBinMethods["schedulerMode"] = &ExecBlockRow::schedulerModeFromBin; 
	 fromBinMethods["baseRangeMin"] = &ExecBlockRow::baseRangeMinFromBin; 
	 fromBinMethods["baseRangeMax"] = &ExecBlockRow::baseRangeMaxFromBin; 
	 fromBinMethods["baseRmsMinor"] = &ExecBlockRow::baseRmsMinorFromBin; 
	 fromBinMethods["baseRmsMajor"] = &ExecBlockRow::baseRmsMajorFromBin; 
	 fromBinMethods["basePa"] = &ExecBlockRow::basePaFromBin; 
	 fromBinMethods["siteAltitude"] = &ExecBlockRow::siteAltitudeFromBin; 
	 fromBinMethods["siteLongitude"] = &ExecBlockRow::siteLongitudeFromBin; 
	 fromBinMethods["siteLatitude"] = &ExecBlockRow::siteLatitudeFromBin; 
	 fromBinMethods["aborted"] = &ExecBlockRow::abortedFromBin; 
	 fromBinMethods["numAntenna"] = &ExecBlockRow::numAntennaFromBin; 
	 fromBinMethods["antennaId"] = &ExecBlockRow::antennaIdFromBin; 
	 fromBinMethods["sBSummaryId"] = &ExecBlockRow::sBSummaryIdFromBin; 
		
	
	 fromBinMethods["releaseDate"] = &ExecBlockRow::releaseDateFromBin; 
	 fromBinMethods["flagRow"] = &ExecBlockRow::flagRowFromBin; 
	
	}
	
	ExecBlockRow::ExecBlockRow (ExecBlockTable &t, ExecBlockRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		releaseDateExists = false;
	

	
		flagRowExists = false;
	

	
	

	
		
		}
		else {
	
		
			execBlockId = row.execBlockId;
		
		
		
		
			startTime = row.startTime;
		
			endTime = row.endTime;
		
			execBlockNum = row.execBlockNum;
		
			execBlockUID = row.execBlockUID;
		
			projectId = row.projectId;
		
			configName = row.configName;
		
			telescopeName = row.telescopeName;
		
			observerName = row.observerName;
		
			observingLog = row.observingLog;
		
			sessionReference = row.sessionReference;
		
			sbSummary = row.sbSummary;
		
			schedulerMode = row.schedulerMode;
		
			baseRangeMin = row.baseRangeMin;
		
			baseRangeMax = row.baseRangeMax;
		
			baseRmsMinor = row.baseRmsMinor;
		
			baseRmsMajor = row.baseRmsMajor;
		
			basePa = row.basePa;
		
			siteAltitude = row.siteAltitude;
		
			siteLongitude = row.siteLongitude;
		
			siteLatitude = row.siteLatitude;
		
			aborted = row.aborted;
		
			numAntenna = row.numAntenna;
		
			antennaId = row.antennaId;
		
			sBSummaryId = row.sBSummaryId;
		
		
		
		
		if (row.releaseDateExists) {
			releaseDate = row.releaseDate;		
			releaseDateExists = true;
		}
		else
			releaseDateExists = false;
		
		if (row.flagRowExists) {
			flagRow = row.flagRow;		
			flagRowExists = true;
		}
		else
			flagRowExists = false;
		
		}
		
		 fromBinMethods["execBlockId"] = &ExecBlockRow::execBlockIdFromBin; 
		 fromBinMethods["startTime"] = &ExecBlockRow::startTimeFromBin; 
		 fromBinMethods["endTime"] = &ExecBlockRow::endTimeFromBin; 
		 fromBinMethods["execBlockNum"] = &ExecBlockRow::execBlockNumFromBin; 
		 fromBinMethods["execBlockUID"] = &ExecBlockRow::execBlockUIDFromBin; 
		 fromBinMethods["projectId"] = &ExecBlockRow::projectIdFromBin; 
		 fromBinMethods["configName"] = &ExecBlockRow::configNameFromBin; 
		 fromBinMethods["telescopeName"] = &ExecBlockRow::telescopeNameFromBin; 
		 fromBinMethods["observerName"] = &ExecBlockRow::observerNameFromBin; 
		 fromBinMethods["observingLog"] = &ExecBlockRow::observingLogFromBin; 
		 fromBinMethods["sessionReference"] = &ExecBlockRow::sessionReferenceFromBin; 
		 fromBinMethods["sbSummary"] = &ExecBlockRow::sbSummaryFromBin; 
		 fromBinMethods["schedulerMode"] = &ExecBlockRow::schedulerModeFromBin; 
		 fromBinMethods["baseRangeMin"] = &ExecBlockRow::baseRangeMinFromBin; 
		 fromBinMethods["baseRangeMax"] = &ExecBlockRow::baseRangeMaxFromBin; 
		 fromBinMethods["baseRmsMinor"] = &ExecBlockRow::baseRmsMinorFromBin; 
		 fromBinMethods["baseRmsMajor"] = &ExecBlockRow::baseRmsMajorFromBin; 
		 fromBinMethods["basePa"] = &ExecBlockRow::basePaFromBin; 
		 fromBinMethods["siteAltitude"] = &ExecBlockRow::siteAltitudeFromBin; 
		 fromBinMethods["siteLongitude"] = &ExecBlockRow::siteLongitudeFromBin; 
		 fromBinMethods["siteLatitude"] = &ExecBlockRow::siteLatitudeFromBin; 
		 fromBinMethods["aborted"] = &ExecBlockRow::abortedFromBin; 
		 fromBinMethods["numAntenna"] = &ExecBlockRow::numAntennaFromBin; 
		 fromBinMethods["antennaId"] = &ExecBlockRow::antennaIdFromBin; 
		 fromBinMethods["sBSummaryId"] = &ExecBlockRow::sBSummaryIdFromBin; 
			
	
		 fromBinMethods["releaseDate"] = &ExecBlockRow::releaseDateFromBin; 
		 fromBinMethods["flagRow"] = &ExecBlockRow::flagRowFromBin; 
			
	}

	
	bool ExecBlockRow::compareNoAutoInc(ArrayTime startTime, ArrayTime endTime, int execBlockNum, EntityRef execBlockUID, EntityRef projectId, string configName, string telescopeName, string observerName, string observingLog, string sessionReference, EntityRef sbSummary, string schedulerMode, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, Length siteAltitude, Angle siteLongitude, Angle siteLatitude, bool aborted, int numAntenna, vector<Tag>  antennaId, Tag sBSummaryId) {
		bool result;
		result = true;
		
	
		
		result = result && (this->startTime == startTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endTime == endTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->execBlockNum == execBlockNum);
		
		if (!result) return false;
	

	
		
		result = result && (this->execBlockUID == execBlockUID);
		
		if (!result) return false;
	

	
		
		result = result && (this->projectId == projectId);
		
		if (!result) return false;
	

	
		
		result = result && (this->configName == configName);
		
		if (!result) return false;
	

	
		
		result = result && (this->telescopeName == telescopeName);
		
		if (!result) return false;
	

	
		
		result = result && (this->observerName == observerName);
		
		if (!result) return false;
	

	
		
		result = result && (this->observingLog == observingLog);
		
		if (!result) return false;
	

	
		
		result = result && (this->sessionReference == sessionReference);
		
		if (!result) return false;
	

	
		
		result = result && (this->sbSummary == sbSummary);
		
		if (!result) return false;
	

	
		
		result = result && (this->schedulerMode == schedulerMode);
		
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
	

	
		
		result = result && (this->siteAltitude == siteAltitude);
		
		if (!result) return false;
	

	
		
		result = result && (this->siteLongitude == siteLongitude);
		
		if (!result) return false;
	

	
		
		result = result && (this->siteLatitude == siteLatitude);
		
		if (!result) return false;
	

	
		
		result = result && (this->aborted == aborted);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->sBSummaryId == sBSummaryId);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool ExecBlockRow::compareRequiredValue(ArrayTime startTime, ArrayTime endTime, int execBlockNum, EntityRef execBlockUID, EntityRef projectId, string configName, string telescopeName, string observerName, string observingLog, string sessionReference, EntityRef sbSummary, string schedulerMode, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, Length siteAltitude, Angle siteLongitude, Angle siteLatitude, bool aborted, int numAntenna, vector<Tag>  antennaId, Tag sBSummaryId) {
		bool result;
		result = true;
		
	
		if (!(this->startTime == startTime)) return false;
	

	
		if (!(this->endTime == endTime)) return false;
	

	
		if (!(this->execBlockNum == execBlockNum)) return false;
	

	
		if (!(this->execBlockUID == execBlockUID)) return false;
	

	
		if (!(this->projectId == projectId)) return false;
	

	
		if (!(this->configName == configName)) return false;
	

	
		if (!(this->telescopeName == telescopeName)) return false;
	

	
		if (!(this->observerName == observerName)) return false;
	

	
		if (!(this->observingLog == observingLog)) return false;
	

	
		if (!(this->sessionReference == sessionReference)) return false;
	

	
		if (!(this->sbSummary == sbSummary)) return false;
	

	
		if (!(this->schedulerMode == schedulerMode)) return false;
	

	
		if (!(this->baseRangeMin == baseRangeMin)) return false;
	

	
		if (!(this->baseRangeMax == baseRangeMax)) return false;
	

	
		if (!(this->baseRmsMinor == baseRmsMinor)) return false;
	

	
		if (!(this->baseRmsMajor == baseRmsMajor)) return false;
	

	
		if (!(this->basePa == basePa)) return false;
	

	
		if (!(this->siteAltitude == siteAltitude)) return false;
	

	
		if (!(this->siteLongitude == siteLongitude)) return false;
	

	
		if (!(this->siteLatitude == siteLatitude)) return false;
	

	
		if (!(this->aborted == aborted)) return false;
	

	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->antennaId == antennaId)) return false;
	

	
		if (!(this->sBSummaryId == sBSummaryId)) return false;
	

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
		
			
		if (this->startTime != x->startTime) return false;
			
		if (this->endTime != x->endTime) return false;
			
		if (this->execBlockNum != x->execBlockNum) return false;
			
		if (this->execBlockUID != x->execBlockUID) return false;
			
		if (this->projectId != x->projectId) return false;
			
		if (this->configName != x->configName) return false;
			
		if (this->telescopeName != x->telescopeName) return false;
			
		if (this->observerName != x->observerName) return false;
			
		if (this->observingLog != x->observingLog) return false;
			
		if (this->sessionReference != x->sessionReference) return false;
			
		if (this->sbSummary != x->sbSummary) return false;
			
		if (this->schedulerMode != x->schedulerMode) return false;
			
		if (this->baseRangeMin != x->baseRangeMin) return false;
			
		if (this->baseRangeMax != x->baseRangeMax) return false;
			
		if (this->baseRmsMinor != x->baseRmsMinor) return false;
			
		if (this->baseRmsMajor != x->baseRmsMajor) return false;
			
		if (this->basePa != x->basePa) return false;
			
		if (this->siteAltitude != x->siteAltitude) return false;
			
		if (this->siteLongitude != x->siteLongitude) return false;
			
		if (this->siteLatitude != x->siteLatitude) return false;
			
		if (this->aborted != x->aborted) return false;
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->antennaId != x->antennaId) return false;
			
		if (this->sBSummaryId != x->sBSummaryId) return false;
			
		
		return true;
	}	
	
/*
	 map<string, ExecBlockAttributeFromBin> ExecBlockRow::initFromBinMethods() {
		map<string, ExecBlockAttributeFromBin> result;
		
		result["execBlockId"] = &ExecBlockRow::execBlockIdFromBin;
		result["startTime"] = &ExecBlockRow::startTimeFromBin;
		result["endTime"] = &ExecBlockRow::endTimeFromBin;
		result["execBlockNum"] = &ExecBlockRow::execBlockNumFromBin;
		result["execBlockUID"] = &ExecBlockRow::execBlockUIDFromBin;
		result["projectId"] = &ExecBlockRow::projectIdFromBin;
		result["configName"] = &ExecBlockRow::configNameFromBin;
		result["telescopeName"] = &ExecBlockRow::telescopeNameFromBin;
		result["observerName"] = &ExecBlockRow::observerNameFromBin;
		result["observingLog"] = &ExecBlockRow::observingLogFromBin;
		result["sessionReference"] = &ExecBlockRow::sessionReferenceFromBin;
		result["sbSummary"] = &ExecBlockRow::sbSummaryFromBin;
		result["schedulerMode"] = &ExecBlockRow::schedulerModeFromBin;
		result["baseRangeMin"] = &ExecBlockRow::baseRangeMinFromBin;
		result["baseRangeMax"] = &ExecBlockRow::baseRangeMaxFromBin;
		result["baseRmsMinor"] = &ExecBlockRow::baseRmsMinorFromBin;
		result["baseRmsMajor"] = &ExecBlockRow::baseRmsMajorFromBin;
		result["basePa"] = &ExecBlockRow::basePaFromBin;
		result["siteAltitude"] = &ExecBlockRow::siteAltitudeFromBin;
		result["siteLongitude"] = &ExecBlockRow::siteLongitudeFromBin;
		result["siteLatitude"] = &ExecBlockRow::siteLatitudeFromBin;
		result["aborted"] = &ExecBlockRow::abortedFromBin;
		result["numAntenna"] = &ExecBlockRow::numAntennaFromBin;
		result["antennaId"] = &ExecBlockRow::antennaIdFromBin;
		result["sBSummaryId"] = &ExecBlockRow::sBSummaryIdFromBin;
		
		
		result["releaseDate"] = &ExecBlockRow::releaseDateFromBin;
		result["flagRow"] = &ExecBlockRow::flagRowFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
