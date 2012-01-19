
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
 * File ConfigDescriptionRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <ConfigDescriptionRow.h>
#include <ConfigDescriptionTable.h>

#include <ProcessorTable.h>
#include <ProcessorRow.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <DataDescriptionTable.h>
#include <DataDescriptionRow.h>

#include <SwitchCycleTable.h>
#include <SwitchCycleRow.h>

#include <FeedTable.h>
#include <FeedRow.h>

#include <ConfigDescriptionTable.h>
#include <ConfigDescriptionRow.h>
	

using asdm::ASDM;
using asdm::ConfigDescriptionRow;
using asdm::ConfigDescriptionTable;

using asdm::ProcessorTable;
using asdm::ProcessorRow;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::DataDescriptionTable;
using asdm::DataDescriptionRow;

using asdm::SwitchCycleTable;
using asdm::SwitchCycleRow;

using asdm::FeedTable;
using asdm::FeedRow;

using asdm::ConfigDescriptionTable;
using asdm::ConfigDescriptionRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	ConfigDescriptionRow::~ConfigDescriptionRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	ConfigDescriptionTable &ConfigDescriptionRow::getTable() const {
		return table;
	}
	
	void ConfigDescriptionRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ConfigDescriptionRowIDL struct.
	 */
	ConfigDescriptionRowIDL *ConfigDescriptionRow::toIDL() const {
		ConfigDescriptionRowIDL *x = new ConfigDescriptionRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		
			
				
		x->numFeed = numFeed;
 				
 			
		
	

	
  		
		
		
			
		x->numSubBand.length(numSubBand.size());
		for (unsigned int i = 0; i < numSubBand.size(); ++i) {
			
				
			x->numSubBand[i] = numSubBand.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->phasedArrayListExists = phasedArrayListExists;
		
		
			
		x->phasedArrayList.length(phasedArrayList.size());
		for (unsigned int i = 0; i < phasedArrayList.size(); ++i) {
			
				
			x->phasedArrayList[i] = phasedArrayList.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->correlationMode = correlationMode;
 				
 			
		
	

	
  		
		
		x->flagAntExists = flagAntExists;
		
		
			
		x->flagAnt.length(flagAnt.size());
		for (unsigned int i = 0; i < flagAnt.size(); ++i) {
			
				
			x->flagAnt[i] = flagAnt.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->configDescriptionId = configDescriptionId.toIDLTag();
			
		
	

	
  		
		
		
			
		x->atmPhaseCorrection.length(atmPhaseCorrection.size());
		for (unsigned int i = 0; i < atmPhaseCorrection.size(); ++i) {
			
				
			x->atmPhaseCorrection[i] = atmPhaseCorrection.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->assocNatureExists = assocNatureExists;
		
		
			
		x->assocNature.length(assocNature.size());
		for (unsigned int i = 0; i < assocNature.size(); ++i) {
			
				
			x->assocNature[i] = assocNature.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
		
		x->antennaId.length(antennaId.size());
		for (unsigned int i = 0; i < antennaId.size(); ++i) {
			
			x->antennaId[i] = antennaId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
 		
		x->assocConfigDescriptionIdExists = assocConfigDescriptionIdExists;
		
		
		
		x->assocConfigDescriptionId.length(assocConfigDescriptionId.size());
		for (unsigned int i = 0; i < assocConfigDescriptionId.size(); ++i) {
			
			x->assocConfigDescriptionId[i] = assocConfigDescriptionId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
		
		
		x->dataDescriptionId.length(dataDescriptionId.size());
		for (unsigned int i = 0; i < dataDescriptionId.size(); ++i) {
			
			x->dataDescriptionId[i] = dataDescriptionId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
		
		
		x->feedId.length(feedId.size());
		for (unsigned int i = 0; i < feedId.size(); ++i) {
			
				
			x->feedId[i] = feedId.at(i);
	 			
	 		
	 	}
	 	 		
  	

	
  	
 		
		
	 	
			
		x->processorId = processorId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
		
		x->switchCycleId.length(switchCycleId.size());
		for (unsigned int i = 0; i < switchCycleId.size(); ++i) {
			
			x->switchCycleId[i] = switchCycleId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
		
	

	

	

	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ConfigDescriptionRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void ConfigDescriptionRow::setFromIDL (ConfigDescriptionRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		
			
		setNumFeed(x.numFeed);
  			
 		
		
	

	
		
		
			
		numSubBand .clear();
		for (unsigned int i = 0; i <x.numSubBand.length(); ++i) {
			
			numSubBand.push_back(x.numSubBand[i]);
  			
		}
			
  		
		
	

	
		
		phasedArrayListExists = x.phasedArrayListExists;
		if (x.phasedArrayListExists) {
		
		
			
		phasedArrayList .clear();
		for (unsigned int i = 0; i <x.phasedArrayList.length(); ++i) {
			
			phasedArrayList.push_back(x.phasedArrayList[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		
			
		setCorrelationMode(x.correlationMode);
  			
 		
		
	

	
		
		flagAntExists = x.flagAntExists;
		if (x.flagAntExists) {
		
		
			
		flagAnt .clear();
		for (unsigned int i = 0; i <x.flagAnt.length(); ++i) {
			
			flagAnt.push_back(x.flagAnt[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		
			
		setConfigDescriptionId(Tag (x.configDescriptionId));
			
 		
		
	

	
		
		
			
		atmPhaseCorrection .clear();
		for (unsigned int i = 0; i <x.atmPhaseCorrection.length(); ++i) {
			
			atmPhaseCorrection.push_back(x.atmPhaseCorrection[i]);
  			
		}
			
  		
		
	

	
		
		assocNatureExists = x.assocNatureExists;
		if (x.assocNatureExists) {
		
		
			
		assocNature .clear();
		for (unsigned int i = 0; i <x.assocNature.length(); ++i) {
			
			assocNature.push_back(x.assocNature[i]);
  			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		antennaId .clear();
		for (unsigned int i = 0; i <x.antennaId.length(); ++i) {
			
			antennaId.push_back(Tag (x.antennaId[i]));
			
		}
		
  	

	
		
		assocConfigDescriptionIdExists = x.assocConfigDescriptionIdExists;
		if (x.assocConfigDescriptionIdExists) {
		
		assocConfigDescriptionId .clear();
		for (unsigned int i = 0; i <x.assocConfigDescriptionId.length(); ++i) {
			
			assocConfigDescriptionId.push_back(Tag (x.assocConfigDescriptionId[i]));
			
		}
		
		}
		
  	

	
		
		dataDescriptionId .clear();
		for (unsigned int i = 0; i <x.dataDescriptionId.length(); ++i) {
			
			dataDescriptionId.push_back(Tag (x.dataDescriptionId[i]));
			
		}
		
  	

	
		
		feedId .clear();
		for (unsigned int i = 0; i <x.feedId.length(); ++i) {
			
			feedId.push_back(x.feedId[i]);
  			
		}
		
  	

	
		
		
			
		setProcessorId(Tag (x.processorId));
			
 		
		
	

	
		
		switchCycleId .clear();
		for (unsigned int i = 0; i <x.switchCycleId.length(); ++i) {
			
			switchCycleId.push_back(Tag (x.switchCycleId[i]));
			
		}
		
  	

	
		
	

	

	

	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"ConfigDescription");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string ConfigDescriptionRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		
		Parser::toXML(numFeed, "numFeed", buf);
		
		
	

  	
 		
		
		Parser::toXML(numSubBand, "numSubBand", buf);
		
		
	

  	
 		
		if (phasedArrayListExists) {
		
		
		Parser::toXML(phasedArrayList, "phasedArrayList", buf);
		
		
		}
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("correlationMode", correlationMode));
		
		
	

  	
 		
		if (flagAntExists) {
		
		
		Parser::toXML(flagAnt, "flagAnt", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(configDescriptionId, "configDescriptionId", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrection", atmPhaseCorrection));
		
		
	

  	
 		
		if (assocNatureExists) {
		
		
			buf.append(EnumerationParser::toXML("assocNature", assocNature));
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		if (assocConfigDescriptionIdExists) {
		
		
		Parser::toXML(assocConfigDescriptionId, "assocConfigDescriptionId", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(dataDescriptionId, "dataDescriptionId", buf);
		
		
	

  	
 		
		
		Parser::toXML(feedId, "feedId", buf);
		
		
	

  	
 		
		
		Parser::toXML(processorId, "processorId", buf);
		
		
	

  	
 		
		
		Parser::toXML(switchCycleId, "switchCycleId", buf);
		
		
	

	
		
	

	

	

	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void ConfigDescriptionRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","ConfigDescription",rowDoc));
			
		
	

	
  		
			
	  	setNumFeed(Parser::getInteger("numFeed","ConfigDescription",rowDoc));
			
		
	

	
  		
			
					
	  	setNumSubBand(Parser::get1DInteger("numSubBand","ConfigDescription",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<phasedArrayList>")) {
			
								
	  		setPhasedArrayList(Parser::get1DInteger("phasedArrayList","ConfigDescription",rowDoc));
	  			
	  		
		}
 		
	

	
		
		
		
		correlationMode = EnumerationParser::getCorrelationMode("correlationMode","ConfigDescription",rowDoc);
		
		
		
	

	
  		
        if (row.isStr("<flagAnt>")) {
			
								
	  		setFlagAnt(Parser::get1DBoolean("flagAnt","ConfigDescription",rowDoc));
	  			
	  		
		}
 		
	

	
  		
			
	  	setConfigDescriptionId(Parser::getTag("configDescriptionId","ConfigDescription",rowDoc));
			
		
	

	
		
		
		
		atmPhaseCorrection = EnumerationParser::getAtmPhaseCorrection1D("atmPhaseCorrection","ConfigDescription",rowDoc);			
		
		
		
	

	
		
	if (row.isStr("<assocNature>")) {
		
		
		
		assocNature = EnumerationParser::getSpectralResolutionType1D("assocNature","ConfigDescription",rowDoc);			
		
		
		
		assocNatureExists = true;
	}
		
	

	
	
		
	
  		 
  		setAntennaId(Parser::get1DTag("antennaId","ConfigDescription",rowDoc));
		
  	

	
  		
  		if (row.isStr("<assocConfigDescriptionId>")) {
  			setAssocConfigDescriptionId(Parser::get1DTag("assocConfigDescriptionId","ConfigDescription",rowDoc));  		
  		}
  		
  	

	
  		 
  		setDataDescriptionId(Parser::get1DTag("dataDescriptionId","ConfigDescription",rowDoc));
		
  	

	
  		 
  		setFeedId(Parser::get1DInteger("feedId","ConfigDescription",rowDoc));
		
  	

	
  		
			
	  	setProcessorId(Parser::getTag("processorId","ConfigDescription",rowDoc));
			
		
	

	
  		 
  		setSwitchCycleId(Parser::get1DTag("switchCycleId","ConfigDescription",rowDoc));
		
  	

	
		
	

	

	

	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"ConfigDescription");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int ConfigDescriptionRow::getNumAntenna() const {
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setNumAntenna (int numAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAntenna = numAntenna;
	
 	}
	
	

	

	
 	/**
 	 * Get numFeed.
 	 * @return numFeed as int
 	 */
 	int ConfigDescriptionRow::getNumFeed() const {
	
  		return numFeed;
 	}

 	/**
 	 * Set numFeed with the specified int.
 	 * @param numFeed The int value to which numFeed is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setNumFeed (int numFeed)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numFeed = numFeed;
	
 	}
	
	

	

	
 	/**
 	 * Get numSubBand.
 	 * @return numSubBand as vector<int >
 	 */
 	vector<int > ConfigDescriptionRow::getNumSubBand() const {
	
  		return numSubBand;
 	}

 	/**
 	 * Set numSubBand with the specified vector<int >.
 	 * @param numSubBand The vector<int > value to which numSubBand is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setNumSubBand (vector<int > numSubBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numSubBand = numSubBand;
	
 	}
	
	

	
	/**
	 * The attribute phasedArrayList is optional. Return true if this attribute exists.
	 * @return true if and only if the phasedArrayList attribute exists. 
	 */
	bool ConfigDescriptionRow::isPhasedArrayListExists() const {
		return phasedArrayListExists;
	}
	

	
 	/**
 	 * Get phasedArrayList, which is optional.
 	 * @return phasedArrayList as vector<int >
 	 * @throw IllegalAccessException If phasedArrayList does not exist.
 	 */
 	vector<int > ConfigDescriptionRow::getPhasedArrayList() const throw(IllegalAccessException) {
		if (!phasedArrayListExists) {
			throw IllegalAccessException("phasedArrayList", "ConfigDescription");
		}
	
  		return phasedArrayList;
 	}

 	/**
 	 * Set phasedArrayList with the specified vector<int >.
 	 * @param phasedArrayList The vector<int > value to which phasedArrayList is to be set.
 	 
 	
 	 */
 	void ConfigDescriptionRow::setPhasedArrayList (vector<int > phasedArrayList) {
	
 		this->phasedArrayList = phasedArrayList;
	
		phasedArrayListExists = true;
	
 	}
	
	
	/**
	 * Mark phasedArrayList, which is an optional field, as non-existent.
	 */
	void ConfigDescriptionRow::clearPhasedArrayList () {
		phasedArrayListExists = false;
	}
	

	

	
 	/**
 	 * Get correlationMode.
 	 * @return correlationMode as CorrelationModeMod::CorrelationMode
 	 */
 	CorrelationModeMod::CorrelationMode ConfigDescriptionRow::getCorrelationMode() const {
	
  		return correlationMode;
 	}

 	/**
 	 * Set correlationMode with the specified CorrelationModeMod::CorrelationMode.
 	 * @param correlationMode The CorrelationModeMod::CorrelationMode value to which correlationMode is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setCorrelationMode (CorrelationModeMod::CorrelationMode correlationMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->correlationMode = correlationMode;
	
 	}
	
	

	
	/**
	 * The attribute flagAnt is optional. Return true if this attribute exists.
	 * @return true if and only if the flagAnt attribute exists. 
	 */
	bool ConfigDescriptionRow::isFlagAntExists() const {
		return flagAntExists;
	}
	

	
 	/**
 	 * Get flagAnt, which is optional.
 	 * @return flagAnt as vector<bool >
 	 * @throw IllegalAccessException If flagAnt does not exist.
 	 */
 	vector<bool > ConfigDescriptionRow::getFlagAnt() const throw(IllegalAccessException) {
		if (!flagAntExists) {
			throw IllegalAccessException("flagAnt", "ConfigDescription");
		}
	
  		return flagAnt;
 	}

 	/**
 	 * Set flagAnt with the specified vector<bool >.
 	 * @param flagAnt The vector<bool > value to which flagAnt is to be set.
 	 
 	
 	 */
 	void ConfigDescriptionRow::setFlagAnt (vector<bool > flagAnt) {
	
 		this->flagAnt = flagAnt;
	
		flagAntExists = true;
	
 	}
	
	
	/**
	 * Mark flagAnt, which is an optional field, as non-existent.
	 */
	void ConfigDescriptionRow::clearFlagAnt () {
		flagAntExists = false;
	}
	

	

	
 	/**
 	 * Get configDescriptionId.
 	 * @return configDescriptionId as Tag
 	 */
 	Tag ConfigDescriptionRow::getConfigDescriptionId() const {
	
  		return configDescriptionId;
 	}

 	/**
 	 * Set configDescriptionId with the specified Tag.
 	 * @param configDescriptionId The Tag value to which configDescriptionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ConfigDescriptionRow::setConfigDescriptionId (Tag configDescriptionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("configDescriptionId", "ConfigDescription");
		
  		}
  	
 		this->configDescriptionId = configDescriptionId;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >
 	 */
 	vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > ConfigDescriptionRow::getAtmPhaseCorrection() const {
	
  		return atmPhaseCorrection;
 	}

 	/**
 	 * Set atmPhaseCorrection with the specified vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >.
 	 * @param atmPhaseCorrection The vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > value to which atmPhaseCorrection is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setAtmPhaseCorrection (vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrection)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmPhaseCorrection = atmPhaseCorrection;
	
 	}
	
	

	
	/**
	 * The attribute assocNature is optional. Return true if this attribute exists.
	 * @return true if and only if the assocNature attribute exists. 
	 */
	bool ConfigDescriptionRow::isAssocNatureExists() const {
		return assocNatureExists;
	}
	

	
 	/**
 	 * Get assocNature, which is optional.
 	 * @return assocNature as vector<SpectralResolutionTypeMod::SpectralResolutionType >
 	 * @throw IllegalAccessException If assocNature does not exist.
 	 */
 	vector<SpectralResolutionTypeMod::SpectralResolutionType > ConfigDescriptionRow::getAssocNature() const throw(IllegalAccessException) {
		if (!assocNatureExists) {
			throw IllegalAccessException("assocNature", "ConfigDescription");
		}
	
  		return assocNature;
 	}

 	/**
 	 * Set assocNature with the specified vector<SpectralResolutionTypeMod::SpectralResolutionType >.
 	 * @param assocNature The vector<SpectralResolutionTypeMod::SpectralResolutionType > value to which assocNature is to be set.
 	 
 	
 	 */
 	void ConfigDescriptionRow::setAssocNature (vector<SpectralResolutionTypeMod::SpectralResolutionType > assocNature) {
	
 		this->assocNature = assocNature;
	
		assocNatureExists = true;
	
 	}
	
	
	/**
	 * Mark assocNature, which is an optional field, as non-existent.
	 */
	void ConfigDescriptionRow::clearAssocNature () {
		assocNatureExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as vector<Tag> 
 	 */
 	vector<Tag>  ConfigDescriptionRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified vector<Tag> .
 	 * @param antennaId The vector<Tag>  value to which antennaId is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setAntennaId (vector<Tag>  antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	
	/**
	 * The attribute assocConfigDescriptionId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocConfigDescriptionId attribute exists. 
	 */
	bool ConfigDescriptionRow::isAssocConfigDescriptionIdExists() const {
		return assocConfigDescriptionIdExists;
	}
	

	
 	/**
 	 * Get assocConfigDescriptionId, which is optional.
 	 * @return assocConfigDescriptionId as vector<Tag> 
 	 * @throw IllegalAccessException If assocConfigDescriptionId does not exist.
 	 */
 	vector<Tag>  ConfigDescriptionRow::getAssocConfigDescriptionId() const throw(IllegalAccessException) {
		if (!assocConfigDescriptionIdExists) {
			throw IllegalAccessException("assocConfigDescriptionId", "ConfigDescription");
		}
	
  		return assocConfigDescriptionId;
 	}

 	/**
 	 * Set assocConfigDescriptionId with the specified vector<Tag> .
 	 * @param assocConfigDescriptionId The vector<Tag>  value to which assocConfigDescriptionId is to be set.
 	 
 	
 	 */
 	void ConfigDescriptionRow::setAssocConfigDescriptionId (vector<Tag>  assocConfigDescriptionId) {
	
 		this->assocConfigDescriptionId = assocConfigDescriptionId;
	
		assocConfigDescriptionIdExists = true;
	
 	}
	
	
	/**
	 * Mark assocConfigDescriptionId, which is an optional field, as non-existent.
	 */
	void ConfigDescriptionRow::clearAssocConfigDescriptionId () {
		assocConfigDescriptionIdExists = false;
	}
	

	

	
 	/**
 	 * Get dataDescriptionId.
 	 * @return dataDescriptionId as vector<Tag> 
 	 */
 	vector<Tag>  ConfigDescriptionRow::getDataDescriptionId() const {
	
  		return dataDescriptionId;
 	}

 	/**
 	 * Set dataDescriptionId with the specified vector<Tag> .
 	 * @param dataDescriptionId The vector<Tag>  value to which dataDescriptionId is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setDataDescriptionId (vector<Tag>  dataDescriptionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->dataDescriptionId = dataDescriptionId;
	
 	}
	
	

	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as vector<int> 
 	 */
 	vector<int>  ConfigDescriptionRow::getFeedId() const {
	
  		return feedId;
 	}

 	/**
 	 * Set feedId with the specified vector<int> .
 	 * @param feedId The vector<int>  value to which feedId is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setFeedId (vector<int>  feedId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->feedId = feedId;
	
 	}
	
	

	

	
 	/**
 	 * Get processorId.
 	 * @return processorId as Tag
 	 */
 	Tag ConfigDescriptionRow::getProcessorId() const {
	
  		return processorId;
 	}

 	/**
 	 * Set processorId with the specified Tag.
 	 * @param processorId The Tag value to which processorId is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setProcessorId (Tag processorId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->processorId = processorId;
	
 	}
	
	

	

	
 	/**
 	 * Get switchCycleId.
 	 * @return switchCycleId as vector<Tag> 
 	 */
 	vector<Tag>  ConfigDescriptionRow::getSwitchCycleId() const {
	
  		return switchCycleId;
 	}

 	/**
 	 * Set switchCycleId with the specified vector<Tag> .
 	 * @param switchCycleId The vector<Tag>  value to which switchCycleId is to be set.
 	 
 	
 		
 	 */
 	void ConfigDescriptionRow::setSwitchCycleId (vector<Tag>  switchCycleId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->switchCycleId = switchCycleId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the Processor table having Processor.processorId == processorId
	 * @return a ProcessorRow*
	 * 
	 
	 */
	 ProcessorRow* ConfigDescriptionRow::getProcessorUsingProcessorId() {
	 
	 	return table.getContainer().getProcessor().getRowByKey(processorId);
	 }
	 

	

	
 		
 	/**
 	 * Set antennaId[i] with the specified Tag.
 	 * @param i The index in antennaId where to set the Tag value.
 	 * @param antennaId The Tag value to which antennaId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void ConfigDescriptionRow::setAntennaId (int i, Tag antennaId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->antennaId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute antennaId in table ConfigDescriptionTable");
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
 void ConfigDescriptionRow::addAntennaId(Tag id){
 	antennaId.push_back(id);
}

/**
 * Append an array of Tag to antennaId.
 * @param id an array of Tag to be appended to antennaId
 */
 void ConfigDescriptionRow::addAntennaId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		antennaId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in antennaId at position i.
  *
  */
 const Tag ConfigDescriptionRow::getAntennaId(int i) {
 	return antennaId.at(i);
 }
 
 /**
  * Returns the AntennaRow linked to this row via the Tag stored in antennaId
  * at position i.
  */
 AntennaRow* ConfigDescriptionRow::getAntenna(int i) {
 	return table.getContainer().getAntenna().getRowByKey(antennaId.at(i));
 } 
 
 /**
  * Returns the vector of AntennaRow* linked to this row via the Tags stored in antennaId
  *
  */
 vector<AntennaRow *> ConfigDescriptionRow::getAntennas() {
 	vector<AntennaRow *> result;
 	for (unsigned int i = 0; i < antennaId.size(); i++)
 		result.push_back(table.getContainer().getAntenna().getRowByKey(antennaId.at(i)));
 		
 	return result;
 }
  

	

	
 		
 	/**
 	 * Set dataDescriptionId[i] with the specified Tag.
 	 * @param i The index in dataDescriptionId where to set the Tag value.
 	 * @param dataDescriptionId The Tag value to which dataDescriptionId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void ConfigDescriptionRow::setDataDescriptionId (int i, Tag dataDescriptionId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->dataDescriptionId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute dataDescriptionId in table ConfigDescriptionTable");
  		vector<Tag> ::iterator iter = this->dataDescriptionId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->dataDescriptionId.insert(this->dataDescriptionId.erase(iter), dataDescriptionId); 
  	}	
 			
	
	
	
		
/**
 * Append a Tag to dataDescriptionId.
 * @param id the Tag to be appended to dataDescriptionId
 */
 void ConfigDescriptionRow::addDataDescriptionId(Tag id){
 	dataDescriptionId.push_back(id);
}

/**
 * Append an array of Tag to dataDescriptionId.
 * @param id an array of Tag to be appended to dataDescriptionId
 */
 void ConfigDescriptionRow::addDataDescriptionId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		dataDescriptionId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in dataDescriptionId at position i.
  *
  */
 const Tag ConfigDescriptionRow::getDataDescriptionId(int i) {
 	return dataDescriptionId.at(i);
 }
 
 /**
  * Returns the DataDescriptionRow linked to this row via the Tag stored in dataDescriptionId
  * at position i.
  */
 DataDescriptionRow* ConfigDescriptionRow::getDataDescription(int i) {
 	return table.getContainer().getDataDescription().getRowByKey(dataDescriptionId.at(i));
 } 
 
 /**
  * Returns the vector of DataDescriptionRow* linked to this row via the Tags stored in dataDescriptionId
  *
  */
 vector<DataDescriptionRow *> ConfigDescriptionRow::getDataDescriptions() {
 	vector<DataDescriptionRow *> result;
 	for (unsigned int i = 0; i < dataDescriptionId.size(); i++)
 		result.push_back(table.getContainer().getDataDescription().getRowByKey(dataDescriptionId.at(i)));
 		
 	return result;
 }
  

	

	
 		
 	/**
 	 * Set switchCycleId[i] with the specified Tag.
 	 * @param i The index in switchCycleId where to set the Tag value.
 	 * @param switchCycleId The Tag value to which switchCycleId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void ConfigDescriptionRow::setSwitchCycleId (int i, Tag switchCycleId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->switchCycleId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute switchCycleId in table ConfigDescriptionTable");
  		vector<Tag> ::iterator iter = this->switchCycleId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->switchCycleId.insert(this->switchCycleId.erase(iter), switchCycleId); 
  	}	
 			
	
	
	
		
/**
 * Append a Tag to switchCycleId.
 * @param id the Tag to be appended to switchCycleId
 */
 void ConfigDescriptionRow::addSwitchCycleId(Tag id){
 	switchCycleId.push_back(id);
}

/**
 * Append an array of Tag to switchCycleId.
 * @param id an array of Tag to be appended to switchCycleId
 */
 void ConfigDescriptionRow::addSwitchCycleId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		switchCycleId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in switchCycleId at position i.
  *
  */
 const Tag ConfigDescriptionRow::getSwitchCycleId(int i) {
 	return switchCycleId.at(i);
 }
 
 /**
  * Returns the SwitchCycleRow linked to this row via the Tag stored in switchCycleId
  * at position i.
  */
 SwitchCycleRow* ConfigDescriptionRow::getSwitchCycle(int i) {
 	return table.getContainer().getSwitchCycle().getRowByKey(switchCycleId.at(i));
 } 
 
 /**
  * Returns the vector of SwitchCycleRow* linked to this row via the Tags stored in switchCycleId
  *
  */
 vector<SwitchCycleRow *> ConfigDescriptionRow::getSwitchCycles() {
 	vector<SwitchCycleRow *> result;
 	for (unsigned int i = 0; i < switchCycleId.size(); i++)
 		result.push_back(table.getContainer().getSwitchCycle().getRowByKey(switchCycleId.at(i)));
 		
 	return result;
 }
  

	

	
 		
 	/**
 	 * Set feedId[i] with the specified int.
 	 * @param i The index in feedId where to set the int value.
 	 * @param feedId The int value to which feedId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void ConfigDescriptionRow::setFeedId (int i, int feedId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->feedId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute feedId in table ConfigDescriptionTable");
  		vector<int> ::iterator iter = this->feedId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->feedId.insert(this->feedId.erase(iter), feedId); 
  	}	
 			
	
	
	
		

	// ===> Slices link from a row of ConfigDescription table to a collection of row of Feed table.	
	// vector <int> feedId;
	
	/*
	 ** Append a new id to feedId
	 */
	void ConfigDescriptionRow::addFeedId(int id) {
		feedId.push_back(id);
	}
	
	/*
	 ** Append an array of ids to feedId
	 */ 
	void ConfigDescriptionRow::addFeedId(vector<int> id) {
		for (unsigned int i = 0; i < id.size(); i++) 
			feedId.push_back(id[i]);
	}
	/**
	 * Get the collection of rows in the Feed table having feedId == feedId[i]
	 */	 
	const vector <FeedRow *> ConfigDescriptionRow::getFeeds(int i) {
		
			return table.getContainer().getFeed().getRowByFeedId(feedId.at(i));
				
	}
	
	/** 
	 * Get the collection of pointers to rows in the Feed table having feedId == feedId[i]
	 * for any i in [O..feedId.size()-1].
	 */
	const vector <FeedRow *> ConfigDescriptionRow::getFeeds() {
		
			vector <FeedRow *> result;
			for (unsigned int i=0; i < feedId.size(); i++) {
				vector <FeedRow *> current = table.getContainer().getFeed().getRowByFeedId(feedId.at(i));
				for (unsigned int j = 0; j < current.size(); j++) 
					result.push_back(current.at(j));
			}					
			return result;
	}


	

	
 		
 	/**
 	 * Set assocConfigDescriptionId[i] with the specified Tag.
 	 * @param i The index in assocConfigDescriptionId where to set the Tag value.
 	 * @param assocConfigDescriptionId The Tag value to which assocConfigDescriptionId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void ConfigDescriptionRow::setAssocConfigDescriptionId (int i, Tag assocConfigDescriptionId) {
  		if ((i < 0) || (i > ((int) this->assocConfigDescriptionId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute assocConfigDescriptionId in table ConfigDescriptionTable");
  		vector<Tag> ::iterator iter = this->assocConfigDescriptionId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->assocConfigDescriptionId.insert(this->assocConfigDescriptionId.erase(iter), assocConfigDescriptionId); 	
  	}
 			
	
	
	
		
/**
 * Append a Tag to assocConfigDescriptionId.
 * @param id the Tag to be appended to assocConfigDescriptionId
 */
 void ConfigDescriptionRow::addAssocConfigDescriptionId(Tag id){
 	assocConfigDescriptionId.push_back(id);
}

/**
 * Append an array of Tag to assocConfigDescriptionId.
 * @param id an array of Tag to be appended to assocConfigDescriptionId
 */
 void ConfigDescriptionRow::addAssocConfigDescriptionId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		assocConfigDescriptionId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in assocConfigDescriptionId at position i.
  *
  */
 const Tag ConfigDescriptionRow::getAssocConfigDescriptionId(int i) {
 	return assocConfigDescriptionId.at(i);
 }
 
 /**
  * Returns the ConfigDescriptionRow linked to this row via the Tag stored in assocConfigDescriptionId
  * at position i.
  */
 ConfigDescriptionRow* ConfigDescriptionRow::getConfigDescription(int i) {
 	return table.getContainer().getConfigDescription().getRowByKey(assocConfigDescriptionId.at(i));
 } 
 
 /**
  * Returns the vector of ConfigDescriptionRow* linked to this row via the Tags stored in assocConfigDescriptionId
  *
  */
 vector<ConfigDescriptionRow *> ConfigDescriptionRow::getConfigDescriptions() {
 	vector<ConfigDescriptionRow *> result;
 	for (unsigned int i = 0; i < assocConfigDescriptionId.size(); i++)
 		result.push_back(table.getContainer().getConfigDescription().getRowByKey(assocConfigDescriptionId.at(i)));
 		
 	return result;
 }
  

	

	
	/**
	 * Create a ConfigDescriptionRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ConfigDescriptionRow::ConfigDescriptionRow (ConfigDescriptionTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	
		phasedArrayListExists = false;
	

	

	
		flagAntExists = false;
	

	

	

	
		assocNatureExists = false;
	

	
	

	
		assocConfigDescriptionIdExists = false;
	

	

	

	

	

	
	
	
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
correlationMode = CCorrelationMode::from_int(0);
	

	

	

	

	
	
	}
	
	ConfigDescriptionRow::ConfigDescriptionRow (ConfigDescriptionTable &t, ConfigDescriptionRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	
		phasedArrayListExists = false;
	

	

	
		flagAntExists = false;
	

	

	

	
		assocNatureExists = false;
	

	
	

	
		assocConfigDescriptionIdExists = false;
	

	

	

	

	
		
		}
		else {
	
		
			configDescriptionId = row.configDescriptionId;
		
		
		
		
			antennaId = row.antennaId;
		
			dataDescriptionId = row.dataDescriptionId;
		
			feedId = row.feedId;
		
			processorId = row.processorId;
		
			switchCycleId = row.switchCycleId;
		
			numAntenna = row.numAntenna;
		
			numFeed = row.numFeed;
		
			numSubBand = row.numSubBand;
		
			correlationMode = row.correlationMode;
		
			atmPhaseCorrection = row.atmPhaseCorrection;
		
		
		
		
		if (row.assocConfigDescriptionIdExists) {
			assocConfigDescriptionId = row.assocConfigDescriptionId;		
			assocConfigDescriptionIdExists = true;
		}
		else
			assocConfigDescriptionIdExists = false;
		
		if (row.phasedArrayListExists) {
			phasedArrayList = row.phasedArrayList;		
			phasedArrayListExists = true;
		}
		else
			phasedArrayListExists = false;
		
		if (row.flagAntExists) {
			flagAnt = row.flagAnt;		
			flagAntExists = true;
		}
		else
			flagAntExists = false;
		
		if (row.assocNatureExists) {
			assocNature = row.assocNature;		
			assocNatureExists = true;
		}
		else
			assocNatureExists = false;
		
		}	
	}

	
	bool ConfigDescriptionRow::compareNoAutoInc(vector<Tag>  antennaId, vector<Tag>  dataDescriptionId, vector<int>  feedId, Tag processorId, vector<Tag>  switchCycleId, int numAntenna, int numFeed, vector<int > numSubBand, CorrelationModeMod::CorrelationMode correlationMode, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrection) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->dataDescriptionId == dataDescriptionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->feedId == feedId);
		
		if (!result) return false;
	

	
		
		result = result && (this->processorId == processorId);
		
		if (!result) return false;
	

	
		
		result = result && (this->switchCycleId == switchCycleId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->numFeed == numFeed);
		
		if (!result) return false;
	

	
		
		result = result && (this->numSubBand == numSubBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->correlationMode == correlationMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmPhaseCorrection == atmPhaseCorrection);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool ConfigDescriptionRow::compareRequiredValue(vector<Tag>  antennaId, vector<Tag>  dataDescriptionId, vector<int>  feedId, Tag processorId, vector<Tag>  switchCycleId, int numAntenna, int numFeed, vector<int > numSubBand, CorrelationModeMod::CorrelationMode correlationMode, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrection) {
		bool result;
		result = true;
		
	
		if (!(this->antennaId == antennaId)) return false;
	

	
		if (!(this->dataDescriptionId == dataDescriptionId)) return false;
	

	
		if (!(this->feedId == feedId)) return false;
	

	
		if (!(this->processorId == processorId)) return false;
	

	
		if (!(this->switchCycleId == switchCycleId)) return false;
	

	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->numFeed == numFeed)) return false;
	

	
		if (!(this->numSubBand == numSubBand)) return false;
	

	
		if (!(this->correlationMode == correlationMode)) return false;
	

	
		if (!(this->atmPhaseCorrection == atmPhaseCorrection)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ConfigDescriptionRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool ConfigDescriptionRow::equalByRequiredValue(ConfigDescriptionRow* x) {
		
			
		if (this->antennaId != x->antennaId) return false;
			
		if (this->dataDescriptionId != x->dataDescriptionId) return false;
			
		if (this->feedId != x->feedId) return false;
			
		if (this->processorId != x->processorId) return false;
			
		if (this->switchCycleId != x->switchCycleId) return false;
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->numFeed != x->numFeed) return false;
			
		if (this->numSubBand != x->numSubBand) return false;
			
		if (this->correlationMode != x->correlationMode) return false;
			
		if (this->atmPhaseCorrection != x->atmPhaseCorrection) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
