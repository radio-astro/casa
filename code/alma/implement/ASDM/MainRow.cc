
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
 * File MainRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <MainRow.h>
#include <MainTable.h>

#include <ConfigDescriptionTable.h>
#include <ConfigDescriptionRow.h>

#include <FieldTable.h>
#include <FieldRow.h>

#include <StateTable.h>
#include <StateRow.h>

#include <ExecBlockTable.h>
#include <ExecBlockRow.h>
	

using asdm::ASDM;
using asdm::MainRow;
using asdm::MainTable;

using asdm::ConfigDescriptionTable;
using asdm::ConfigDescriptionRow;

using asdm::FieldTable;
using asdm::FieldRow;

using asdm::StateTable;
using asdm::StateRow;

using asdm::ExecBlockTable;
using asdm::ExecBlockRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	MainRow::~MainRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	MainTable &MainRow::getTable() const {
		return table;
	}
	
	void MainRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a MainRowIDL struct.
	 */
	MainRowIDL *MainRow::toIDL() const {
		MainRowIDL *x = new MainRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->time = time.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		
			
				
		x->timeSampling = timeSampling;
 				
 			
		
	

	
  		
		
		
			
		x->interval = interval.toIDLInterval();
			
		
	

	
  		
		
		
			
				
		x->numIntegration = numIntegration;
 				
 			
		
	

	
  		
		
		
			
				
		x->scanNumber = scanNumber;
 				
 			
		
	

	
  		
		
		
			
				
		x->subscanNumber = subscanNumber;
 				
 			
		
	

	
  		
		
		
			
				
		x->dataSize = dataSize;
 				
 			
		
	

	
  		
		
		
			
		x->dataOid = dataOid.toIDLEntityRef();
			
		
	

	
  		
		
		x->flagRowExists = flagRowExists;
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->configDescriptionId = configDescriptionId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->execBlockId = execBlockId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->fieldId = fieldId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
		
		x->stateId.length(stateId.size());
		for (unsigned int i = 0; i < stateId.size(); ++i) {
			
			x->stateId[i] = stateId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
		
	

	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct MainRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void MainRow::setFromIDL (MainRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTime(ArrayTime (x.time));
			
 		
		
	

	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		
			
		setTimeSampling(x.timeSampling);
  			
 		
		
	

	
		
		
			
		setInterval(Interval (x.interval));
			
 		
		
	

	
		
		
			
		setNumIntegration(x.numIntegration);
  			
 		
		
	

	
		
		
			
		setScanNumber(x.scanNumber);
  			
 		
		
	

	
		
		
			
		setSubscanNumber(x.subscanNumber);
  			
 		
		
	

	
		
		
			
		setDataSize(x.dataSize);
  			
 		
		
	

	
		
		
			
		setDataOid(EntityRef (x.dataOid));
			
 		
		
	

	
		
		flagRowExists = x.flagRowExists;
		if (x.flagRowExists) {
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setConfigDescriptionId(Tag (x.configDescriptionId));
			
 		
		
	

	
		
		
			
		setExecBlockId(Tag (x.execBlockId));
			
 		
		
	

	
		
		
			
		setFieldId(Tag (x.fieldId));
			
 		
		
	

	
		
		stateId .clear();
		for (unsigned int i = 0; i <x.stateId.length(); ++i) {
			
			stateId.push_back(Tag (x.stateId[i]));
			
		}
		
  	

	
		
	

	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Main");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string MainRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(time, "time", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("timeSampling", timeSampling));
		
		
	

  	
 		
		
		Parser::toXML(interval, "interval", buf);
		
		
	

  	
 		
		
		Parser::toXML(numIntegration, "numIntegration", buf);
		
		
	

  	
 		
		
		Parser::toXML(scanNumber, "scanNumber", buf);
		
		
	

  	
 		
		
		Parser::toXML(subscanNumber, "subscanNumber", buf);
		
		
	

  	
 		
		
		Parser::toXML(dataSize, "dataSize", buf);
		
		
	

  	
 		
		
		Parser::toXML(dataOid, "dataOid", buf);
		
		
	

  	
 		
		if (flagRowExists) {
		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(configDescriptionId, "configDescriptionId", buf);
		
		
	

  	
 		
		
		Parser::toXML(execBlockId, "execBlockId", buf);
		
		
	

  	
 		
		
		Parser::toXML(fieldId, "fieldId", buf);
		
		
	

  	
 		
		
		Parser::toXML(stateId, "stateId", buf);
		
		
	

	
		
	

	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void MainRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTime(Parser::getArrayTime("time","Main",rowDoc));
			
		
	

	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","Main",rowDoc));
			
		
	

	
		
		
		
		timeSampling = EnumerationParser::getTimeSampling("timeSampling","Main",rowDoc);
		
		
		
	

	
  		
			
	  	setInterval(Parser::getInterval("interval","Main",rowDoc));
			
		
	

	
  		
			
	  	setNumIntegration(Parser::getInteger("numIntegration","Main",rowDoc));
			
		
	

	
  		
			
	  	setScanNumber(Parser::getInteger("scanNumber","Main",rowDoc));
			
		
	

	
  		
			
	  	setSubscanNumber(Parser::getInteger("subscanNumber","Main",rowDoc));
			
		
	

	
  		
			
	  	setDataSize(Parser::getInteger("dataSize","Main",rowDoc));
			
		
	

	
  		
			
	  	setDataOid(Parser::getEntityRef("dataOid","Main",rowDoc));
			
		
	

	
  		
        if (row.isStr("<flagRow>")) {
			
	  		setFlagRow(Parser::getBoolean("flagRow","Main",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setConfigDescriptionId(Parser::getTag("configDescriptionId","ConfigDescription",rowDoc));
			
		
	

	
  		
			
	  	setExecBlockId(Parser::getTag("execBlockId","Main",rowDoc));
			
		
	

	
  		
			
	  	setFieldId(Parser::getTag("fieldId","Field",rowDoc));
			
		
	

	
  		 
  		setStateId(Parser::get1DTag("stateId","Main",rowDoc));
		
  	

	
		
	

	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Main");
		}
	}
	
	void MainRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	time.toBin(eoss);
		
	

	
	
		
	configDescriptionId.toBin(eoss);
		
	

	
	
		
	fieldId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numAntenna);
				
		
	

	
	
		
					
			eoss.writeInt(timeSampling);
				
		
	

	
	
		
	interval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numIntegration);
				
		
	

	
	
		
						
			eoss.writeInt(scanNumber);
				
		
	

	
	
		
						
			eoss.writeInt(subscanNumber);
				
		
	

	
	
		
						
			eoss.writeInt(dataSize);
				
		
	

	
	
		
	dataOid.toBin(eoss);
		
	

	
	
		
	Tag::toBin(stateId, eoss);
		
	

	
	
		
	execBlockId.toBin(eoss);
		
	


	
	
	eoss.writeBoolean(flagRowExists);
	if (flagRowExists) {
	
	
	
		
						
			eoss.writeBoolean(flagRow);
				
		
	

	}

	}
	
void MainRow::timeFromBin(EndianISStream& eiss) {
		
	
		
		
		time =  ArrayTime::fromBin(eiss);
		
	
	
}
void MainRow::configDescriptionIdFromBin(EndianISStream& eiss) {
		
	
		
		
		configDescriptionId =  Tag::fromBin(eiss);
		
	
	
}
void MainRow::fieldIdFromBin(EndianISStream& eiss) {
		
	
		
		
		fieldId =  Tag::fromBin(eiss);
		
	
	
}
void MainRow::numAntennaFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numAntenna =  eiss.readInt();
			
		
	
	
}
void MainRow::timeSamplingFromBin(EndianISStream& eiss) {
		
	
	
		
			
		timeSampling = CTimeSampling::from_int(eiss.readInt());
			
		
	
	
}
void MainRow::intervalFromBin(EndianISStream& eiss) {
		
	
		
		
		interval =  Interval::fromBin(eiss);
		
	
	
}
void MainRow::numIntegrationFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numIntegration =  eiss.readInt();
			
		
	
	
}
void MainRow::scanNumberFromBin(EndianISStream& eiss) {
		
	
	
		
			
		scanNumber =  eiss.readInt();
			
		
	
	
}
void MainRow::subscanNumberFromBin(EndianISStream& eiss) {
		
	
	
		
			
		subscanNumber =  eiss.readInt();
			
		
	
	
}
void MainRow::dataSizeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		dataSize =  eiss.readInt();
			
		
	
	
}
void MainRow::dataOidFromBin(EndianISStream& eiss) {
		
	
		
		
		dataOid =  EntityRef::fromBin(eiss);
		
	
	
}
void MainRow::stateIdFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	stateId = Tag::from1DBin(eiss);	
	

		
	
	
}
void MainRow::execBlockIdFromBin(EndianISStream& eiss) {
		
	
		
		
		execBlockId =  Tag::fromBin(eiss);
		
	
	
}

void MainRow::flagRowFromBin(EndianISStream& eiss) {
		
	flagRowExists = eiss.readBoolean();
	if (flagRowExists) {
		
	
	
		
			
		flagRow =  eiss.readBoolean();
			
		
	

	}
	
}
	
	
	MainRow* MainRow::fromBin(EndianISStream& eiss, MainTable& table, const vector<string>& attributesSeq) {
		MainRow* row = new  MainRow(table);
		
		map<string, MainAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "MainTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime MainRow::getTime() const {
	
  		return time;
 	}

 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void MainRow::setTime (ArrayTime time)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("time", "Main");
		
  		}
  	
 		this->time = time;
	
 	}
	
	

	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int MainRow::getNumAntenna() const {
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setNumAntenna (int numAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAntenna = numAntenna;
	
 	}
	
	

	

	
 	/**
 	 * Get timeSampling.
 	 * @return timeSampling as TimeSamplingMod::TimeSampling
 	 */
 	TimeSamplingMod::TimeSampling MainRow::getTimeSampling() const {
	
  		return timeSampling;
 	}

 	/**
 	 * Set timeSampling with the specified TimeSamplingMod::TimeSampling.
 	 * @param timeSampling The TimeSamplingMod::TimeSampling value to which timeSampling is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setTimeSampling (TimeSamplingMod::TimeSampling timeSampling)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->timeSampling = timeSampling;
	
 	}
	
	

	

	
 	/**
 	 * Get interval.
 	 * @return interval as Interval
 	 */
 	Interval MainRow::getInterval() const {
	
  		return interval;
 	}

 	/**
 	 * Set interval with the specified Interval.
 	 * @param interval The Interval value to which interval is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setInterval (Interval interval)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->interval = interval;
	
 	}
	
	

	

	
 	/**
 	 * Get numIntegration.
 	 * @return numIntegration as int
 	 */
 	int MainRow::getNumIntegration() const {
	
  		return numIntegration;
 	}

 	/**
 	 * Set numIntegration with the specified int.
 	 * @param numIntegration The int value to which numIntegration is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setNumIntegration (int numIntegration)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numIntegration = numIntegration;
	
 	}
	
	

	

	
 	/**
 	 * Get scanNumber.
 	 * @return scanNumber as int
 	 */
 	int MainRow::getScanNumber() const {
	
  		return scanNumber;
 	}

 	/**
 	 * Set scanNumber with the specified int.
 	 * @param scanNumber The int value to which scanNumber is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setScanNumber (int scanNumber)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->scanNumber = scanNumber;
	
 	}
	
	

	

	
 	/**
 	 * Get subscanNumber.
 	 * @return subscanNumber as int
 	 */
 	int MainRow::getSubscanNumber() const {
	
  		return subscanNumber;
 	}

 	/**
 	 * Set subscanNumber with the specified int.
 	 * @param subscanNumber The int value to which subscanNumber is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setSubscanNumber (int subscanNumber)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->subscanNumber = subscanNumber;
	
 	}
	
	

	

	
 	/**
 	 * Get dataSize.
 	 * @return dataSize as int
 	 */
 	int MainRow::getDataSize() const {
	
  		return dataSize;
 	}

 	/**
 	 * Set dataSize with the specified int.
 	 * @param dataSize The int value to which dataSize is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setDataSize (int dataSize)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->dataSize = dataSize;
	
 	}
	
	

	

	
 	/**
 	 * Get dataOid.
 	 * @return dataOid as EntityRef
 	 */
 	EntityRef MainRow::getDataOid() const {
	
  		return dataOid;
 	}

 	/**
 	 * Set dataOid with the specified EntityRef.
 	 * @param dataOid The EntityRef value to which dataOid is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setDataOid (EntityRef dataOid)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->dataOid = dataOid;
	
 	}
	
	

	
	/**
	 * The attribute flagRow is optional. Return true if this attribute exists.
	 * @return true if and only if the flagRow attribute exists. 
	 */
	bool MainRow::isFlagRowExists() const {
		return flagRowExists;
	}
	

	
 	/**
 	 * Get flagRow, which is optional.
 	 * @return flagRow as bool
 	 * @throw IllegalAccessException If flagRow does not exist.
 	 */
 	bool MainRow::getFlagRow() const  {
		if (!flagRowExists) {
			throw IllegalAccessException("flagRow", "Main");
		}
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 	 */
 	void MainRow::setFlagRow (bool flagRow) {
	
 		this->flagRow = flagRow;
	
		flagRowExists = true;
	
 	}
	
	
	/**
	 * Mark flagRow, which is an optional field, as non-existent.
	 */
	void MainRow::clearFlagRow () {
		flagRowExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get configDescriptionId.
 	 * @return configDescriptionId as Tag
 	 */
 	Tag MainRow::getConfigDescriptionId() const {
	
  		return configDescriptionId;
 	}

 	/**
 	 * Set configDescriptionId with the specified Tag.
 	 * @param configDescriptionId The Tag value to which configDescriptionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void MainRow::setConfigDescriptionId (Tag configDescriptionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("configDescriptionId", "Main");
		
  		}
  	
 		this->configDescriptionId = configDescriptionId;
	
 	}
	
	

	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag MainRow::getExecBlockId() const {
	
  		return execBlockId;
 	}

 	/**
 	 * Set execBlockId with the specified Tag.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setExecBlockId (Tag execBlockId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->execBlockId = execBlockId;
	
 	}
	
	

	

	
 	/**
 	 * Get fieldId.
 	 * @return fieldId as Tag
 	 */
 	Tag MainRow::getFieldId() const {
	
  		return fieldId;
 	}

 	/**
 	 * Set fieldId with the specified Tag.
 	 * @param fieldId The Tag value to which fieldId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void MainRow::setFieldId (Tag fieldId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("fieldId", "Main");
		
  		}
  	
 		this->fieldId = fieldId;
	
 	}
	
	

	

	
 	/**
 	 * Get stateId.
 	 * @return stateId as vector<Tag> 
 	 */
 	vector<Tag>  MainRow::getStateId() const {
	
  		return stateId;
 	}

 	/**
 	 * Set stateId with the specified vector<Tag> .
 	 * @param stateId The vector<Tag>  value to which stateId is to be set.
 	 
 	
 		
 	 */
 	void MainRow::setStateId (vector<Tag>  stateId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->stateId = stateId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the ConfigDescription table having ConfigDescription.configDescriptionId == configDescriptionId
	 * @return a ConfigDescriptionRow*
	 * 
	 
	 */
	 ConfigDescriptionRow* MainRow::getConfigDescriptionUsingConfigDescriptionId() {
	 
	 	return table.getContainer().getConfigDescription().getRowByKey(configDescriptionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Field table having Field.fieldId == fieldId
	 * @return a FieldRow*
	 * 
	 
	 */
	 FieldRow* MainRow::getFieldUsingFieldId() {
	 
	 	return table.getContainer().getField().getRowByKey(fieldId);
	 }
	 

	

	
 		
 	/**
 	 * Set stateId[i] with the specified Tag.
 	 * @param i The index in stateId where to set the Tag value.
 	 * @param stateId The Tag value to which stateId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void MainRow::setStateId (int i, Tag stateId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->stateId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute stateId in table MainTable");
  		vector<Tag> ::iterator iter = this->stateId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->stateId.insert(this->stateId.erase(iter), stateId); 
  	}	
 			
	
	
	
		
/**
 * Append a Tag to stateId.
 * @param id the Tag to be appended to stateId
 */
 void MainRow::addStateId(Tag id){
 	stateId.push_back(id);
}

/**
 * Append an array of Tag to stateId.
 * @param id an array of Tag to be appended to stateId
 */
 void MainRow::addStateId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		stateId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in stateId at position i.
  *
  */
 const Tag MainRow::getStateId(int i) {
 	return stateId.at(i);
 }
 
 /**
  * Returns the StateRow linked to this row via the Tag stored in stateId
  * at position i.
  */
 StateRow* MainRow::getState(int i) {
 	return table.getContainer().getState().getRowByKey(stateId.at(i));
 } 
 
 /**
  * Returns the vector of StateRow* linked to this row via the Tags stored in stateId
  *
  */
 vector<StateRow *> MainRow::getStates() {
 	vector<StateRow *> result;
 	for (unsigned int i = 0; i < stateId.size(); i++)
 		result.push_back(table.getContainer().getState().getRowByKey(stateId.at(i)));
 		
 	return result;
 }
  

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the ExecBlock table having ExecBlock.execBlockId == execBlockId
	 * @return a ExecBlockRow*
	 * 
	 
	 */
	 ExecBlockRow* MainRow::getExecBlockUsingExecBlockId() {
	 
	 	return table.getContainer().getExecBlock().getRowByKey(execBlockId);
	 }
	 

	

	
	/**
	 * Create a MainRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	MainRow::MainRow (MainTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	
		flagRowExists = false;
	

	
	

	

	

	

	
	
	
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
timeSampling = CTimeSampling::from_int(0);
	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["time"] = &MainRow::timeFromBin; 
	 fromBinMethods["configDescriptionId"] = &MainRow::configDescriptionIdFromBin; 
	 fromBinMethods["fieldId"] = &MainRow::fieldIdFromBin; 
	 fromBinMethods["numAntenna"] = &MainRow::numAntennaFromBin; 
	 fromBinMethods["timeSampling"] = &MainRow::timeSamplingFromBin; 
	 fromBinMethods["interval"] = &MainRow::intervalFromBin; 
	 fromBinMethods["numIntegration"] = &MainRow::numIntegrationFromBin; 
	 fromBinMethods["scanNumber"] = &MainRow::scanNumberFromBin; 
	 fromBinMethods["subscanNumber"] = &MainRow::subscanNumberFromBin; 
	 fromBinMethods["dataSize"] = &MainRow::dataSizeFromBin; 
	 fromBinMethods["dataOid"] = &MainRow::dataOidFromBin; 
	 fromBinMethods["stateId"] = &MainRow::stateIdFromBin; 
	 fromBinMethods["execBlockId"] = &MainRow::execBlockIdFromBin; 
		
	
	 fromBinMethods["flagRow"] = &MainRow::flagRowFromBin; 
	
	}
	
	MainRow::MainRow (MainTable &t, MainRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	
		flagRowExists = false;
	

	
	

	

	

	
		
		}
		else {
	
		
			time = row.time;
		
			configDescriptionId = row.configDescriptionId;
		
			fieldId = row.fieldId;
		
		
		
		
			numAntenna = row.numAntenna;
		
			timeSampling = row.timeSampling;
		
			interval = row.interval;
		
			numIntegration = row.numIntegration;
		
			scanNumber = row.scanNumber;
		
			subscanNumber = row.subscanNumber;
		
			dataSize = row.dataSize;
		
			dataOid = row.dataOid;
		
			stateId = row.stateId;
		
			execBlockId = row.execBlockId;
		
		
		
		
		if (row.flagRowExists) {
			flagRow = row.flagRow;		
			flagRowExists = true;
		}
		else
			flagRowExists = false;
		
		}
		
		 fromBinMethods["time"] = &MainRow::timeFromBin; 
		 fromBinMethods["configDescriptionId"] = &MainRow::configDescriptionIdFromBin; 
		 fromBinMethods["fieldId"] = &MainRow::fieldIdFromBin; 
		 fromBinMethods["numAntenna"] = &MainRow::numAntennaFromBin; 
		 fromBinMethods["timeSampling"] = &MainRow::timeSamplingFromBin; 
		 fromBinMethods["interval"] = &MainRow::intervalFromBin; 
		 fromBinMethods["numIntegration"] = &MainRow::numIntegrationFromBin; 
		 fromBinMethods["scanNumber"] = &MainRow::scanNumberFromBin; 
		 fromBinMethods["subscanNumber"] = &MainRow::subscanNumberFromBin; 
		 fromBinMethods["dataSize"] = &MainRow::dataSizeFromBin; 
		 fromBinMethods["dataOid"] = &MainRow::dataOidFromBin; 
		 fromBinMethods["stateId"] = &MainRow::stateIdFromBin; 
		 fromBinMethods["execBlockId"] = &MainRow::execBlockIdFromBin; 
			
	
		 fromBinMethods["flagRow"] = &MainRow::flagRowFromBin; 
			
	}

	
	bool MainRow::compareNoAutoInc(ArrayTime time, Tag configDescriptionId, Tag fieldId, int numAntenna, TimeSamplingMod::TimeSampling timeSampling, Interval interval, int numIntegration, int scanNumber, int subscanNumber, int dataSize, EntityRef dataOid, vector<Tag>  stateId, Tag execBlockId) {
		bool result;
		result = true;
		
	
		
		result = result && (this->time == time);
		
		if (!result) return false;
	

	
		
		result = result && (this->configDescriptionId == configDescriptionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->fieldId == fieldId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeSampling == timeSampling);
		
		if (!result) return false;
	

	
		
		result = result && (this->interval == interval);
		
		if (!result) return false;
	

	
		
		result = result && (this->numIntegration == numIntegration);
		
		if (!result) return false;
	

	
		
		result = result && (this->scanNumber == scanNumber);
		
		if (!result) return false;
	

	
		
		result = result && (this->subscanNumber == subscanNumber);
		
		if (!result) return false;
	

	
		
		result = result && (this->dataSize == dataSize);
		
		if (!result) return false;
	

	
		
		result = result && (this->dataOid == dataOid);
		
		if (!result) return false;
	

	
		
		result = result && (this->stateId == stateId);
		
		if (!result) return false;
	

	
		
		result = result && (this->execBlockId == execBlockId);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool MainRow::compareRequiredValue(int numAntenna, TimeSamplingMod::TimeSampling timeSampling, Interval interval, int numIntegration, int scanNumber, int subscanNumber, int dataSize, EntityRef dataOid, vector<Tag>  stateId, Tag execBlockId) {
		bool result;
		result = true;
		
	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->timeSampling == timeSampling)) return false;
	

	
		if (!(this->interval == interval)) return false;
	

	
		if (!(this->numIntegration == numIntegration)) return false;
	

	
		if (!(this->scanNumber == scanNumber)) return false;
	

	
		if (!(this->subscanNumber == subscanNumber)) return false;
	

	
		if (!(this->dataSize == dataSize)) return false;
	

	
		if (!(this->dataOid == dataOid)) return false;
	

	
		if (!(this->stateId == stateId)) return false;
	

	
		if (!(this->execBlockId == execBlockId)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the MainRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool MainRow::equalByRequiredValue(MainRow* x) {
		
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->timeSampling != x->timeSampling) return false;
			
		if (this->interval != x->interval) return false;
			
		if (this->numIntegration != x->numIntegration) return false;
			
		if (this->scanNumber != x->scanNumber) return false;
			
		if (this->subscanNumber != x->subscanNumber) return false;
			
		if (this->dataSize != x->dataSize) return false;
			
		if (this->dataOid != x->dataOid) return false;
			
		if (this->stateId != x->stateId) return false;
			
		if (this->execBlockId != x->execBlockId) return false;
			
		
		return true;
	}	
	
/*
	 map<string, MainAttributeFromBin> MainRow::initFromBinMethods() {
		map<string, MainAttributeFromBin> result;
		
		result["time"] = &MainRow::timeFromBin;
		result["configDescriptionId"] = &MainRow::configDescriptionIdFromBin;
		result["fieldId"] = &MainRow::fieldIdFromBin;
		result["numAntenna"] = &MainRow::numAntennaFromBin;
		result["timeSampling"] = &MainRow::timeSamplingFromBin;
		result["interval"] = &MainRow::intervalFromBin;
		result["numIntegration"] = &MainRow::numIntegrationFromBin;
		result["scanNumber"] = &MainRow::scanNumberFromBin;
		result["subscanNumber"] = &MainRow::subscanNumberFromBin;
		result["dataSize"] = &MainRow::dataSizeFromBin;
		result["dataOid"] = &MainRow::dataOidFromBin;
		result["stateId"] = &MainRow::stateIdFromBin;
		result["execBlockId"] = &MainRow::execBlockIdFromBin;
		
		
		result["flagRow"] = &MainRow::flagRowFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
