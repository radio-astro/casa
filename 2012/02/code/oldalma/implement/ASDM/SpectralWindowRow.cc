
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
 * File SpectralWindowRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <SpectralWindowRow.h>
#include <SpectralWindowTable.h>

#include <DopplerTable.h>
#include <DopplerRow.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>
	

using asdm::ASDM;
using asdm::SpectralWindowRow;
using asdm::SpectralWindowTable;

using asdm::DopplerTable;
using asdm::DopplerRow;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	SpectralWindowRow::~SpectralWindowRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	SpectralWindowTable &SpectralWindowRow::getTable() const {
		return table;
	}
	
	void SpectralWindowRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SpectralWindowRowIDL struct.
	 */
	SpectralWindowRowIDL *SpectralWindowRow::toIDL() const {
		SpectralWindowRowIDL *x = new SpectralWindowRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->numChan = numChan;
 				
 			
		
	

	
  		
		
		x->nameExists = nameExists;
		
		
			
				
		x->name = CORBA::string_dup(name.c_str());
				
 			
		
	

	
  		
		
		
			
		x->refFreq = refFreq.toIDLFrequency();
			
		
	

	
  		
		
		
			
		x->chanFreq.length(chanFreq.size());
		for (unsigned int i = 0; i < chanFreq.size(); ++i) {
			
			x->chanFreq[i] = chanFreq.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->chanWidth.length(chanWidth.size());
		for (unsigned int i = 0; i < chanWidth.size(); ++i) {
			
			x->chanWidth[i] = chanWidth.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		x->measFreqRefExists = measFreqRefExists;
		
		
			
				
		x->measFreqRef = measFreqRef;
 				
 			
		
	

	
  		
		
		
			
		x->effectiveBw.length(effectiveBw.size());
		for (unsigned int i = 0; i < effectiveBw.size(); ++i) {
			
			x->effectiveBw[i] = effectiveBw.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->resolution.length(resolution.size());
		for (unsigned int i = 0; i < resolution.size(); ++i) {
			
			x->resolution[i] = resolution.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->totBandwidth = totBandwidth.toIDLFrequency();
			
		
	

	
  		
		
		
			
				
		x->netSideband = netSideband;
 				
 			
		
	

	
  		
		
		
			
				
		x->sidebandProcessingMode = sidebandProcessingMode;
 				
 			
		
	

	
  		
		
		x->basebandNameExists = basebandNameExists;
		
		
			
				
		x->basebandName = basebandName;
 				
 			
		
	

	
  		
		
		x->bbcSidebandExists = bbcSidebandExists;
		
		
			
				
		x->bbcSideband = bbcSideband;
 				
 			
		
	

	
  		
		
		x->ifConvChainExists = ifConvChainExists;
		
		
			
				
		x->ifConvChain = ifConvChain;
 				
 			
		
	

	
  		
		
		x->freqGroupExists = freqGroupExists;
		
		
			
				
		x->freqGroup = freqGroup;
 				
 			
		
	

	
  		
		
		x->freqGroupNameExists = freqGroupNameExists;
		
		
			
				
		x->freqGroupName = CORBA::string_dup(freqGroupName.c_str());
				
 			
		
	

	
  		
		
		x->assocNatureExists = assocNatureExists;
		
		
			
		x->assocNature.length(assocNature.size());
		for (unsigned int i = 0; i < assocNature.size(); ++i) {
			
				
			x->assocNature[i] = assocNature.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->quantization = quantization;
 				
 			
		
	

	
  		
		
		
			
				
		x->windowFunction = windowFunction;
 				
 			
		
	

	
  		
		
		
			
				
		x->oversampling = oversampling;
 				
 			
		
	

	
  		
		
		
			
				
		x->correlationBit = correlationBit;
 				
 			
		
	

	
  		
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
	
  	
 		
 		
		x->assocSpectralWindowIdExists = assocSpectralWindowIdExists;
		
		
		
		x->assocSpectralWindowId.length(assocSpectralWindowId.size());
		for (unsigned int i = 0; i < assocSpectralWindowId.size(); ++i) {
			
			x->assocSpectralWindowId[i] = assocSpectralWindowId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
 		
		x->dopplerIdExists = dopplerIdExists;
		
		
	 	
			
				
		x->dopplerId = dopplerId;
 				
 			
	 	 		
  	

	
  	
 		
 		
		x->imageSpectralWindowIdExists = imageSpectralWindowIdExists;
		
		
	 	
			
		x->imageSpectralWindowId = imageSpectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SpectralWindowRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void SpectralWindowRow::setFromIDL (SpectralWindowRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
		
			
		setNumChan(x.numChan);
  			
 		
		
	

	
		
		nameExists = x.nameExists;
		if (x.nameExists) {
		
		
			
		setName(string (x.name));
			
 		
		
		}
		
	

	
		
		
			
		setRefFreq(Frequency (x.refFreq));
			
 		
		
	

	
		
		
			
		chanFreq .clear();
		for (unsigned int i = 0; i <x.chanFreq.length(); ++i) {
			
			chanFreq.push_back(Frequency (x.chanFreq[i]));
			
		}
			
  		
		
	

	
		
		
			
		chanWidth .clear();
		for (unsigned int i = 0; i <x.chanWidth.length(); ++i) {
			
			chanWidth.push_back(Frequency (x.chanWidth[i]));
			
		}
			
  		
		
	

	
		
		measFreqRefExists = x.measFreqRefExists;
		if (x.measFreqRefExists) {
		
		
			
		setMeasFreqRef(x.measFreqRef);
  			
 		
		
		}
		
	

	
		
		
			
		effectiveBw .clear();
		for (unsigned int i = 0; i <x.effectiveBw.length(); ++i) {
			
			effectiveBw.push_back(Frequency (x.effectiveBw[i]));
			
		}
			
  		
		
	

	
		
		
			
		resolution .clear();
		for (unsigned int i = 0; i <x.resolution.length(); ++i) {
			
			resolution.push_back(Frequency (x.resolution[i]));
			
		}
			
  		
		
	

	
		
		
			
		setTotBandwidth(Frequency (x.totBandwidth));
			
 		
		
	

	
		
		
			
		setNetSideband(x.netSideband);
  			
 		
		
	

	
		
		
			
		setSidebandProcessingMode(x.sidebandProcessingMode);
  			
 		
		
	

	
		
		basebandNameExists = x.basebandNameExists;
		if (x.basebandNameExists) {
		
		
			
		setBasebandName(x.basebandName);
  			
 		
		
		}
		
	

	
		
		bbcSidebandExists = x.bbcSidebandExists;
		if (x.bbcSidebandExists) {
		
		
			
		setBbcSideband(x.bbcSideband);
  			
 		
		
		}
		
	

	
		
		ifConvChainExists = x.ifConvChainExists;
		if (x.ifConvChainExists) {
		
		
			
		setIfConvChain(x.ifConvChain);
  			
 		
		
		}
		
	

	
		
		freqGroupExists = x.freqGroupExists;
		if (x.freqGroupExists) {
		
		
			
		setFreqGroup(x.freqGroup);
  			
 		
		
		}
		
	

	
		
		freqGroupNameExists = x.freqGroupNameExists;
		if (x.freqGroupNameExists) {
		
		
			
		setFreqGroupName(string (x.freqGroupName));
			
 		
		
		}
		
	

	
		
		assocNatureExists = x.assocNatureExists;
		if (x.assocNatureExists) {
		
		
			
		assocNature .clear();
		for (unsigned int i = 0; i <x.assocNature.length(); ++i) {
			
			assocNature.push_back(x.assocNature[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		
			
		setQuantization(x.quantization);
  			
 		
		
	

	
		
		
			
		setWindowFunction(x.windowFunction);
  			
 		
		
	

	
		
		
			
		setOversampling(x.oversampling);
  			
 		
		
	

	
		
		
			
		setCorrelationBit(x.correlationBit);
  			
 		
		
	

	
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
	

	
	
		
	
		
		assocSpectralWindowIdExists = x.assocSpectralWindowIdExists;
		if (x.assocSpectralWindowIdExists) {
		
		assocSpectralWindowId .clear();
		for (unsigned int i = 0; i <x.assocSpectralWindowId.length(); ++i) {
			
			assocSpectralWindowId.push_back(Tag (x.assocSpectralWindowId[i]));
			
		}
		
		}
		
  	

	
		
		dopplerIdExists = x.dopplerIdExists;
		if (x.dopplerIdExists) {
		
		
			
		setDopplerId(x.dopplerId);
  			
 		
		
		}
		
	

	
		
		imageSpectralWindowIdExists = x.imageSpectralWindowIdExists;
		if (x.imageSpectralWindowIdExists) {
		
		
			
		setImageSpectralWindowId(Tag (x.imageSpectralWindowId));
			
 		
		
		}
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"SpectralWindow");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string SpectralWindowRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

  	
 		
		
		Parser::toXML(numChan, "numChan", buf);
		
		
	

  	
 		
		if (nameExists) {
		
		
		Parser::toXML(name, "name", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(refFreq, "refFreq", buf);
		
		
	

  	
 		
		
		Parser::toXML(chanFreq, "chanFreq", buf);
		
		
	

  	
 		
		
		Parser::toXML(chanWidth, "chanWidth", buf);
		
		
	

  	
 		
		if (measFreqRefExists) {
		
		
		Parser::toXML(measFreqRef, "measFreqRef", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(effectiveBw, "effectiveBw", buf);
		
		
	

  	
 		
		
		Parser::toXML(resolution, "resolution", buf);
		
		
	

  	
 		
		
		Parser::toXML(totBandwidth, "totBandwidth", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("netSideband", netSideband));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("sidebandProcessingMode", sidebandProcessingMode));
		
		
	

  	
 		
		if (basebandNameExists) {
		
		
			buf.append(EnumerationParser::toXML("basebandName", basebandName));
		
		
		}
		
	

  	
 		
		if (bbcSidebandExists) {
		
		
		Parser::toXML(bbcSideband, "bbcSideband", buf);
		
		
		}
		
	

  	
 		
		if (ifConvChainExists) {
		
		
		Parser::toXML(ifConvChain, "ifConvChain", buf);
		
		
		}
		
	

  	
 		
		if (freqGroupExists) {
		
		
		Parser::toXML(freqGroup, "freqGroup", buf);
		
		
		}
		
	

  	
 		
		if (freqGroupNameExists) {
		
		
		Parser::toXML(freqGroupName, "freqGroupName", buf);
		
		
		}
		
	

  	
 		
		if (assocNatureExists) {
		
		
			buf.append(EnumerationParser::toXML("assocNature", assocNature));
		
		
		}
		
	

  	
 		
		
		Parser::toXML(quantization, "quantization", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("windowFunction", windowFunction));
		
		
	

  	
 		
		
		Parser::toXML(oversampling, "oversampling", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("correlationBit", correlationBit));
		
		
	

  	
 		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
	

	
	
		
  	
 		
		if (assocSpectralWindowIdExists) {
		
		
		Parser::toXML(assocSpectralWindowId, "assocSpectralWindowId", buf);
		
		
		}
		
	

  	
 		
		if (dopplerIdExists) {
		
		
		Parser::toXML(dopplerId, "dopplerId", buf);
		
		
		}
		
	

  	
 		
		if (imageSpectralWindowIdExists) {
		
		
		Parser::toXML(imageSpectralWindowId, "imageSpectralWindowId", buf);
		
		
		}
		
	

	
		
	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void SpectralWindowRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
  		
			
	  	setNumChan(Parser::getInteger("numChan","SpectralWindow",rowDoc));
			
		
	

	
  		
        if (row.isStr("<name>")) {
			
	  		setName(Parser::getString("name","SpectralWindow",rowDoc));
			
		}
 		
	

	
  		
			
	  	setRefFreq(Parser::getFrequency("refFreq","SpectralWindow",rowDoc));
			
		
	

	
  		
			
					
	  	setChanFreq(Parser::get1DFrequency("chanFreq","SpectralWindow",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setChanWidth(Parser::get1DFrequency("chanWidth","SpectralWindow",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<measFreqRef>")) {
			
	  		setMeasFreqRef(Parser::getInteger("measFreqRef","SpectralWindow",rowDoc));
			
		}
 		
	

	
  		
			
					
	  	setEffectiveBw(Parser::get1DFrequency("effectiveBw","SpectralWindow",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setResolution(Parser::get1DFrequency("resolution","SpectralWindow",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setTotBandwidth(Parser::getFrequency("totBandwidth","SpectralWindow",rowDoc));
			
		
	

	
		
		
		
		netSideband = EnumerationParser::getNetSideband("netSideband","SpectralWindow",rowDoc);
		
		
		
	

	
		
		
		
		sidebandProcessingMode = EnumerationParser::getSidebandProcessingMode("sidebandProcessingMode","SpectralWindow",rowDoc);
		
		
		
	

	
		
	if (row.isStr("<basebandName>")) {
		
		
		
		basebandName = EnumerationParser::getBasebandName("basebandName","SpectralWindow",rowDoc);
		
		
		
		basebandNameExists = true;
	}
		
	

	
  		
        if (row.isStr("<bbcSideband>")) {
			
	  		setBbcSideband(Parser::getInteger("bbcSideband","SpectralWindow",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<ifConvChain>")) {
			
	  		setIfConvChain(Parser::getInteger("ifConvChain","SpectralWindow",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<freqGroup>")) {
			
	  		setFreqGroup(Parser::getInteger("freqGroup","SpectralWindow",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<freqGroupName>")) {
			
	  		setFreqGroupName(Parser::getString("freqGroupName","SpectralWindow",rowDoc));
			
		}
 		
	

	
		
	if (row.isStr("<assocNature>")) {
		
		
		
		assocNature = EnumerationParser::getSpectralResolutionType1D("assocNature","SpectralWindow",rowDoc);			
		
		
		
		assocNatureExists = true;
	}
		
	

	
  		
			
	  	setQuantization(Parser::getBoolean("quantization","SpectralWindow",rowDoc));
			
		
	

	
		
		
		
		windowFunction = EnumerationParser::getWindowFunction("windowFunction","SpectralWindow",rowDoc);
		
		
		
	

	
  		
			
	  	setOversampling(Parser::getBoolean("oversampling","SpectralWindow",rowDoc));
			
		
	

	
		
		
		
		correlationBit = EnumerationParser::getCorrelationBit("correlationBit","SpectralWindow",rowDoc);
		
		
		
	

	
  		
			
	  	setFlagRow(Parser::getBoolean("flagRow","SpectralWindow",rowDoc));
			
		
	

	
	
		
	
  		
  		if (row.isStr("<assocSpectralWindowId>")) {
  			setAssocSpectralWindowId(Parser::get1DTag("assocSpectralWindowId","SpectralWindow",rowDoc));  		
  		}
  		
  	

	
  		
        if (row.isStr("<dopplerId>")) {
			
	  		setDopplerId(Parser::getInteger("dopplerId","SpectralWindow",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<imageSpectralWindowId>")) {
			
	  		setImageSpectralWindowId(Parser::getTag("imageSpectralWindowId","SpectralWindow",rowDoc));
			
		}
 		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SpectralWindow");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag SpectralWindowRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SpectralWindowRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "SpectralWindow");
		
  		}
  	
 		this->spectralWindowId = spectralWindowId;
	
 	}
	
	

	

	
 	/**
 	 * Get numChan.
 	 * @return numChan as int
 	 */
 	int SpectralWindowRow::getNumChan() const {
	
  		return numChan;
 	}

 	/**
 	 * Set numChan with the specified int.
 	 * @param numChan The int value to which numChan is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setNumChan (int numChan)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numChan = numChan;
	
 	}
	
	

	
	/**
	 * The attribute name is optional. Return true if this attribute exists.
	 * @return true if and only if the name attribute exists. 
	 */
	bool SpectralWindowRow::isNameExists() const {
		return nameExists;
	}
	

	
 	/**
 	 * Get name, which is optional.
 	 * @return name as string
 	 * @throw IllegalAccessException If name does not exist.
 	 */
 	string SpectralWindowRow::getName() const throw(IllegalAccessException) {
		if (!nameExists) {
			throw IllegalAccessException("name", "SpectralWindow");
		}
	
  		return name;
 	}

 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setName (string name) {
	
 		this->name = name;
	
		nameExists = true;
	
 	}
	
	
	/**
	 * Mark name, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearName () {
		nameExists = false;
	}
	

	

	
 	/**
 	 * Get refFreq.
 	 * @return refFreq as Frequency
 	 */
 	Frequency SpectralWindowRow::getRefFreq() const {
	
  		return refFreq;
 	}

 	/**
 	 * Set refFreq with the specified Frequency.
 	 * @param refFreq The Frequency value to which refFreq is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setRefFreq (Frequency refFreq)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refFreq = refFreq;
	
 	}
	
	

	

	
 	/**
 	 * Get chanFreq.
 	 * @return chanFreq as vector<Frequency >
 	 */
 	vector<Frequency > SpectralWindowRow::getChanFreq() const {
	
  		return chanFreq;
 	}

 	/**
 	 * Set chanFreq with the specified vector<Frequency >.
 	 * @param chanFreq The vector<Frequency > value to which chanFreq is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setChanFreq (vector<Frequency > chanFreq)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->chanFreq = chanFreq;
	
 	}
	
	

	

	
 	/**
 	 * Get chanWidth.
 	 * @return chanWidth as vector<Frequency >
 	 */
 	vector<Frequency > SpectralWindowRow::getChanWidth() const {
	
  		return chanWidth;
 	}

 	/**
 	 * Set chanWidth with the specified vector<Frequency >.
 	 * @param chanWidth The vector<Frequency > value to which chanWidth is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setChanWidth (vector<Frequency > chanWidth)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->chanWidth = chanWidth;
	
 	}
	
	

	
	/**
	 * The attribute measFreqRef is optional. Return true if this attribute exists.
	 * @return true if and only if the measFreqRef attribute exists. 
	 */
	bool SpectralWindowRow::isMeasFreqRefExists() const {
		return measFreqRefExists;
	}
	

	
 	/**
 	 * Get measFreqRef, which is optional.
 	 * @return measFreqRef as int
 	 * @throw IllegalAccessException If measFreqRef does not exist.
 	 */
 	int SpectralWindowRow::getMeasFreqRef() const throw(IllegalAccessException) {
		if (!measFreqRefExists) {
			throw IllegalAccessException("measFreqRef", "SpectralWindow");
		}
	
  		return measFreqRef;
 	}

 	/**
 	 * Set measFreqRef with the specified int.
 	 * @param measFreqRef The int value to which measFreqRef is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setMeasFreqRef (int measFreqRef) {
	
 		this->measFreqRef = measFreqRef;
	
		measFreqRefExists = true;
	
 	}
	
	
	/**
	 * Mark measFreqRef, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearMeasFreqRef () {
		measFreqRefExists = false;
	}
	

	

	
 	/**
 	 * Get effectiveBw.
 	 * @return effectiveBw as vector<Frequency >
 	 */
 	vector<Frequency > SpectralWindowRow::getEffectiveBw() const {
	
  		return effectiveBw;
 	}

 	/**
 	 * Set effectiveBw with the specified vector<Frequency >.
 	 * @param effectiveBw The vector<Frequency > value to which effectiveBw is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setEffectiveBw (vector<Frequency > effectiveBw)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->effectiveBw = effectiveBw;
	
 	}
	
	

	

	
 	/**
 	 * Get resolution.
 	 * @return resolution as vector<Frequency >
 	 */
 	vector<Frequency > SpectralWindowRow::getResolution() const {
	
  		return resolution;
 	}

 	/**
 	 * Set resolution with the specified vector<Frequency >.
 	 * @param resolution The vector<Frequency > value to which resolution is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setResolution (vector<Frequency > resolution)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->resolution = resolution;
	
 	}
	
	

	

	
 	/**
 	 * Get totBandwidth.
 	 * @return totBandwidth as Frequency
 	 */
 	Frequency SpectralWindowRow::getTotBandwidth() const {
	
  		return totBandwidth;
 	}

 	/**
 	 * Set totBandwidth with the specified Frequency.
 	 * @param totBandwidth The Frequency value to which totBandwidth is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setTotBandwidth (Frequency totBandwidth)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->totBandwidth = totBandwidth;
	
 	}
	
	

	

	
 	/**
 	 * Get netSideband.
 	 * @return netSideband as NetSidebandMod::NetSideband
 	 */
 	NetSidebandMod::NetSideband SpectralWindowRow::getNetSideband() const {
	
  		return netSideband;
 	}

 	/**
 	 * Set netSideband with the specified NetSidebandMod::NetSideband.
 	 * @param netSideband The NetSidebandMod::NetSideband value to which netSideband is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setNetSideband (NetSidebandMod::NetSideband netSideband)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->netSideband = netSideband;
	
 	}
	
	

	

	
 	/**
 	 * Get sidebandProcessingMode.
 	 * @return sidebandProcessingMode as SidebandProcessingModeMod::SidebandProcessingMode
 	 */
 	SidebandProcessingModeMod::SidebandProcessingMode SpectralWindowRow::getSidebandProcessingMode() const {
	
  		return sidebandProcessingMode;
 	}

 	/**
 	 * Set sidebandProcessingMode with the specified SidebandProcessingModeMod::SidebandProcessingMode.
 	 * @param sidebandProcessingMode The SidebandProcessingModeMod::SidebandProcessingMode value to which sidebandProcessingMode is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setSidebandProcessingMode (SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sidebandProcessingMode = sidebandProcessingMode;
	
 	}
	
	

	
	/**
	 * The attribute basebandName is optional. Return true if this attribute exists.
	 * @return true if and only if the basebandName attribute exists. 
	 */
	bool SpectralWindowRow::isBasebandNameExists() const {
		return basebandNameExists;
	}
	

	
 	/**
 	 * Get basebandName, which is optional.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 * @throw IllegalAccessException If basebandName does not exist.
 	 */
 	BasebandNameMod::BasebandName SpectralWindowRow::getBasebandName() const throw(IllegalAccessException) {
		if (!basebandNameExists) {
			throw IllegalAccessException("basebandName", "SpectralWindow");
		}
	
  		return basebandName;
 	}

 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setBasebandName (BasebandNameMod::BasebandName basebandName) {
	
 		this->basebandName = basebandName;
	
		basebandNameExists = true;
	
 	}
	
	
	/**
	 * Mark basebandName, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearBasebandName () {
		basebandNameExists = false;
	}
	

	
	/**
	 * The attribute bbcSideband is optional. Return true if this attribute exists.
	 * @return true if and only if the bbcSideband attribute exists. 
	 */
	bool SpectralWindowRow::isBbcSidebandExists() const {
		return bbcSidebandExists;
	}
	

	
 	/**
 	 * Get bbcSideband, which is optional.
 	 * @return bbcSideband as int
 	 * @throw IllegalAccessException If bbcSideband does not exist.
 	 */
 	int SpectralWindowRow::getBbcSideband() const throw(IllegalAccessException) {
		if (!bbcSidebandExists) {
			throw IllegalAccessException("bbcSideband", "SpectralWindow");
		}
	
  		return bbcSideband;
 	}

 	/**
 	 * Set bbcSideband with the specified int.
 	 * @param bbcSideband The int value to which bbcSideband is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setBbcSideband (int bbcSideband) {
	
 		this->bbcSideband = bbcSideband;
	
		bbcSidebandExists = true;
	
 	}
	
	
	/**
	 * Mark bbcSideband, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearBbcSideband () {
		bbcSidebandExists = false;
	}
	

	
	/**
	 * The attribute ifConvChain is optional. Return true if this attribute exists.
	 * @return true if and only if the ifConvChain attribute exists. 
	 */
	bool SpectralWindowRow::isIfConvChainExists() const {
		return ifConvChainExists;
	}
	

	
 	/**
 	 * Get ifConvChain, which is optional.
 	 * @return ifConvChain as int
 	 * @throw IllegalAccessException If ifConvChain does not exist.
 	 */
 	int SpectralWindowRow::getIfConvChain() const throw(IllegalAccessException) {
		if (!ifConvChainExists) {
			throw IllegalAccessException("ifConvChain", "SpectralWindow");
		}
	
  		return ifConvChain;
 	}

 	/**
 	 * Set ifConvChain with the specified int.
 	 * @param ifConvChain The int value to which ifConvChain is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setIfConvChain (int ifConvChain) {
	
 		this->ifConvChain = ifConvChain;
	
		ifConvChainExists = true;
	
 	}
	
	
	/**
	 * Mark ifConvChain, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearIfConvChain () {
		ifConvChainExists = false;
	}
	

	
	/**
	 * The attribute freqGroup is optional. Return true if this attribute exists.
	 * @return true if and only if the freqGroup attribute exists. 
	 */
	bool SpectralWindowRow::isFreqGroupExists() const {
		return freqGroupExists;
	}
	

	
 	/**
 	 * Get freqGroup, which is optional.
 	 * @return freqGroup as int
 	 * @throw IllegalAccessException If freqGroup does not exist.
 	 */
 	int SpectralWindowRow::getFreqGroup() const throw(IllegalAccessException) {
		if (!freqGroupExists) {
			throw IllegalAccessException("freqGroup", "SpectralWindow");
		}
	
  		return freqGroup;
 	}

 	/**
 	 * Set freqGroup with the specified int.
 	 * @param freqGroup The int value to which freqGroup is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setFreqGroup (int freqGroup) {
	
 		this->freqGroup = freqGroup;
	
		freqGroupExists = true;
	
 	}
	
	
	/**
	 * Mark freqGroup, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearFreqGroup () {
		freqGroupExists = false;
	}
	

	
	/**
	 * The attribute freqGroupName is optional. Return true if this attribute exists.
	 * @return true if and only if the freqGroupName attribute exists. 
	 */
	bool SpectralWindowRow::isFreqGroupNameExists() const {
		return freqGroupNameExists;
	}
	

	
 	/**
 	 * Get freqGroupName, which is optional.
 	 * @return freqGroupName as string
 	 * @throw IllegalAccessException If freqGroupName does not exist.
 	 */
 	string SpectralWindowRow::getFreqGroupName() const throw(IllegalAccessException) {
		if (!freqGroupNameExists) {
			throw IllegalAccessException("freqGroupName", "SpectralWindow");
		}
	
  		return freqGroupName;
 	}

 	/**
 	 * Set freqGroupName with the specified string.
 	 * @param freqGroupName The string value to which freqGroupName is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setFreqGroupName (string freqGroupName) {
	
 		this->freqGroupName = freqGroupName;
	
		freqGroupNameExists = true;
	
 	}
	
	
	/**
	 * Mark freqGroupName, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearFreqGroupName () {
		freqGroupNameExists = false;
	}
	

	
	/**
	 * The attribute assocNature is optional. Return true if this attribute exists.
	 * @return true if and only if the assocNature attribute exists. 
	 */
	bool SpectralWindowRow::isAssocNatureExists() const {
		return assocNatureExists;
	}
	

	
 	/**
 	 * Get assocNature, which is optional.
 	 * @return assocNature as vector<SpectralResolutionTypeMod::SpectralResolutionType >
 	 * @throw IllegalAccessException If assocNature does not exist.
 	 */
 	vector<SpectralResolutionTypeMod::SpectralResolutionType > SpectralWindowRow::getAssocNature() const throw(IllegalAccessException) {
		if (!assocNatureExists) {
			throw IllegalAccessException("assocNature", "SpectralWindow");
		}
	
  		return assocNature;
 	}

 	/**
 	 * Set assocNature with the specified vector<SpectralResolutionTypeMod::SpectralResolutionType >.
 	 * @param assocNature The vector<SpectralResolutionTypeMod::SpectralResolutionType > value to which assocNature is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setAssocNature (vector<SpectralResolutionTypeMod::SpectralResolutionType > assocNature) {
	
 		this->assocNature = assocNature;
	
		assocNatureExists = true;
	
 	}
	
	
	/**
	 * Mark assocNature, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearAssocNature () {
		assocNatureExists = false;
	}
	

	

	
 	/**
 	 * Get quantization.
 	 * @return quantization as bool
 	 */
 	bool SpectralWindowRow::getQuantization() const {
	
  		return quantization;
 	}

 	/**
 	 * Set quantization with the specified bool.
 	 * @param quantization The bool value to which quantization is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setQuantization (bool quantization)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->quantization = quantization;
	
 	}
	
	

	

	
 	/**
 	 * Get windowFunction.
 	 * @return windowFunction as WindowFunctionMod::WindowFunction
 	 */
 	WindowFunctionMod::WindowFunction SpectralWindowRow::getWindowFunction() const {
	
  		return windowFunction;
 	}

 	/**
 	 * Set windowFunction with the specified WindowFunctionMod::WindowFunction.
 	 * @param windowFunction The WindowFunctionMod::WindowFunction value to which windowFunction is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setWindowFunction (WindowFunctionMod::WindowFunction windowFunction)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->windowFunction = windowFunction;
	
 	}
	
	

	

	
 	/**
 	 * Get oversampling.
 	 * @return oversampling as bool
 	 */
 	bool SpectralWindowRow::getOversampling() const {
	
  		return oversampling;
 	}

 	/**
 	 * Set oversampling with the specified bool.
 	 * @param oversampling The bool value to which oversampling is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setOversampling (bool oversampling)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->oversampling = oversampling;
	
 	}
	
	

	

	
 	/**
 	 * Get correlationBit.
 	 * @return correlationBit as CorrelationBitMod::CorrelationBit
 	 */
 	CorrelationBitMod::CorrelationBit SpectralWindowRow::getCorrelationBit() const {
	
  		return correlationBit;
 	}

 	/**
 	 * Set correlationBit with the specified CorrelationBitMod::CorrelationBit.
 	 * @param correlationBit The CorrelationBitMod::CorrelationBit value to which correlationBit is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setCorrelationBit (CorrelationBitMod::CorrelationBit correlationBit)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->correlationBit = correlationBit;
	
 	}
	
	

	

	
 	/**
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool SpectralWindowRow::getFlagRow() const {
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 		
 	 */
 	void SpectralWindowRow::setFlagRow (bool flagRow)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagRow = flagRow;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	/**
	 * The attribute assocSpectralWindowId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocSpectralWindowId attribute exists. 
	 */
	bool SpectralWindowRow::isAssocSpectralWindowIdExists() const {
		return assocSpectralWindowIdExists;
	}
	

	
 	/**
 	 * Get assocSpectralWindowId, which is optional.
 	 * @return assocSpectralWindowId as vector<Tag> 
 	 * @throw IllegalAccessException If assocSpectralWindowId does not exist.
 	 */
 	vector<Tag>  SpectralWindowRow::getAssocSpectralWindowId() const throw(IllegalAccessException) {
		if (!assocSpectralWindowIdExists) {
			throw IllegalAccessException("assocSpectralWindowId", "SpectralWindow");
		}
	
  		return assocSpectralWindowId;
 	}

 	/**
 	 * Set assocSpectralWindowId with the specified vector<Tag> .
 	 * @param assocSpectralWindowId The vector<Tag>  value to which assocSpectralWindowId is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setAssocSpectralWindowId (vector<Tag>  assocSpectralWindowId) {
	
 		this->assocSpectralWindowId = assocSpectralWindowId;
	
		assocSpectralWindowIdExists = true;
	
 	}
	
	
	/**
	 * Mark assocSpectralWindowId, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearAssocSpectralWindowId () {
		assocSpectralWindowIdExists = false;
	}
	

	
	/**
	 * The attribute dopplerId is optional. Return true if this attribute exists.
	 * @return true if and only if the dopplerId attribute exists. 
	 */
	bool SpectralWindowRow::isDopplerIdExists() const {
		return dopplerIdExists;
	}
	

	
 	/**
 	 * Get dopplerId, which is optional.
 	 * @return dopplerId as int
 	 * @throw IllegalAccessException If dopplerId does not exist.
 	 */
 	int SpectralWindowRow::getDopplerId() const throw(IllegalAccessException) {
		if (!dopplerIdExists) {
			throw IllegalAccessException("dopplerId", "SpectralWindow");
		}
	
  		return dopplerId;
 	}

 	/**
 	 * Set dopplerId with the specified int.
 	 * @param dopplerId The int value to which dopplerId is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setDopplerId (int dopplerId) {
	
 		this->dopplerId = dopplerId;
	
		dopplerIdExists = true;
	
 	}
	
	
	/**
	 * Mark dopplerId, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearDopplerId () {
		dopplerIdExists = false;
	}
	

	
	/**
	 * The attribute imageSpectralWindowId is optional. Return true if this attribute exists.
	 * @return true if and only if the imageSpectralWindowId attribute exists. 
	 */
	bool SpectralWindowRow::isImageSpectralWindowIdExists() const {
		return imageSpectralWindowIdExists;
	}
	

	
 	/**
 	 * Get imageSpectralWindowId, which is optional.
 	 * @return imageSpectralWindowId as Tag
 	 * @throw IllegalAccessException If imageSpectralWindowId does not exist.
 	 */
 	Tag SpectralWindowRow::getImageSpectralWindowId() const throw(IllegalAccessException) {
		if (!imageSpectralWindowIdExists) {
			throw IllegalAccessException("imageSpectralWindowId", "SpectralWindow");
		}
	
  		return imageSpectralWindowId;
 	}

 	/**
 	 * Set imageSpectralWindowId with the specified Tag.
 	 * @param imageSpectralWindowId The Tag value to which imageSpectralWindowId is to be set.
 	 
 	
 	 */
 	void SpectralWindowRow::setImageSpectralWindowId (Tag imageSpectralWindowId) {
	
 		this->imageSpectralWindowId = imageSpectralWindowId;
	
		imageSpectralWindowIdExists = true;
	
 	}
	
	
	/**
	 * Mark imageSpectralWindowId, which is an optional field, as non-existent.
	 */
	void SpectralWindowRow::clearImageSpectralWindowId () {
		imageSpectralWindowIdExists = false;
	}
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	// ===> Slice link from a row of SpectralWindow table to a collection of row of Doppler table.
	
	/**
	 * Get the collection of row in the Doppler table having their attribut dopplerId == this->dopplerId
	 */
	vector <DopplerRow *> SpectralWindowRow::getDopplers() {
		
			if (dopplerIdExists) {
				return table.getContainer().getDoppler().getRowByDopplerId(dopplerId);
			}
			else 
				throw IllegalAccessException();
		
	}
	

	

	
 		
 	/**
 	 * Set assocSpectralWindowId[i] with the specified Tag.
 	 * @param i The index in assocSpectralWindowId where to set the Tag value.
 	 * @param assocSpectralWindowId The Tag value to which assocSpectralWindowId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void SpectralWindowRow::setAssocSpectralWindowId (int i, Tag assocSpectralWindowId) {
  		if ((i < 0) || (i > ((int) this->assocSpectralWindowId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute assocSpectralWindowId in table SpectralWindowTable");
  		vector<Tag> ::iterator iter = this->assocSpectralWindowId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->assocSpectralWindowId.insert(this->assocSpectralWindowId.erase(iter), assocSpectralWindowId); 	
  	}
 			
	
	
	
		
/**
 * Append a Tag to assocSpectralWindowId.
 * @param id the Tag to be appended to assocSpectralWindowId
 */
 void SpectralWindowRow::addAssocSpectralWindowId(Tag id){
 	assocSpectralWindowId.push_back(id);
}

/**
 * Append an array of Tag to assocSpectralWindowId.
 * @param id an array of Tag to be appended to assocSpectralWindowId
 */
 void SpectralWindowRow::addAssocSpectralWindowId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		assocSpectralWindowId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in assocSpectralWindowId at position i.
  *
  */
 const Tag SpectralWindowRow::getAssocSpectralWindowId(int i) {
 	return assocSpectralWindowId.at(i);
 }
 
 /**
  * Returns the SpectralWindowRow linked to this row via the Tag stored in assocSpectralWindowId
  * at position i.
  */
 SpectralWindowRow* SpectralWindowRow::getSpectralWindow(int i) {
 	return table.getContainer().getSpectralWindow().getRowByKey(assocSpectralWindowId.at(i));
 } 
 
 /**
  * Returns the vector of SpectralWindowRow* linked to this row via the Tags stored in assocSpectralWindowId
  *
  */
 vector<SpectralWindowRow *> SpectralWindowRow::getSpectralWindows() {
 	vector<SpectralWindowRow *> result;
 	for (unsigned int i = 0; i < assocSpectralWindowId.size(); i++)
 		result.push_back(table.getContainer().getSpectralWindow().getRowByKey(assocSpectralWindowId.at(i)));
 		
 	return result;
 }
  

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.imageSpectralWindowId == imageSpectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 * throws IllegalAccessException
	 
	 */
	 SpectralWindowRow* SpectralWindowRow::getSpectralWindowUsingImageSpectralWindowId() {
	 
	 	if (!imageSpectralWindowIdExists)
	 		throw IllegalAccessException();	 		 
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(imageSpectralWindowId);
	 }
	 

	

	
	/**
	 * Create a SpectralWindowRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SpectralWindowRow::SpectralWindowRow (SpectralWindowTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	
		nameExists = false;
	

	

	

	

	
		measFreqRefExists = false;
	

	

	

	

	

	

	
		basebandNameExists = false;
	

	
		bbcSidebandExists = false;
	

	
		ifConvChainExists = false;
	

	
		freqGroupExists = false;
	

	
		freqGroupNameExists = false;
	

	
		assocNatureExists = false;
	

	

	

	

	

	

	
	
		assocSpectralWindowIdExists = false;
	

	
		dopplerIdExists = false;
	

	
		imageSpectralWindowIdExists = false;
	

	
	
	
	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
netSideband = CNetSideband::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
sidebandProcessingMode = CSidebandProcessingMode::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
basebandName = CBasebandName::from_int(0);
	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
windowFunction = CWindowFunction::from_int(0);
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
correlationBit = CCorrelationBit::from_int(0);
	

	
	
	}
	
	SpectralWindowRow::SpectralWindowRow (SpectralWindowTable &t, SpectralWindowRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	
		nameExists = false;
	

	

	

	

	
		measFreqRefExists = false;
	

	

	

	

	

	

	
		basebandNameExists = false;
	

	
		bbcSidebandExists = false;
	

	
		ifConvChainExists = false;
	

	
		freqGroupExists = false;
	

	
		freqGroupNameExists = false;
	

	
		assocNatureExists = false;
	

	

	

	

	

	

	
	
		assocSpectralWindowIdExists = false;
	

	
		dopplerIdExists = false;
	

	
		imageSpectralWindowIdExists = false;
	
		
		}
		else {
	
		
			spectralWindowId = row.spectralWindowId;
		
		
		
		
			numChan = row.numChan;
		
			refFreq = row.refFreq;
		
			chanFreq = row.chanFreq;
		
			chanWidth = row.chanWidth;
		
			effectiveBw = row.effectiveBw;
		
			resolution = row.resolution;
		
			totBandwidth = row.totBandwidth;
		
			netSideband = row.netSideband;
		
			sidebandProcessingMode = row.sidebandProcessingMode;
		
			quantization = row.quantization;
		
			windowFunction = row.windowFunction;
		
			oversampling = row.oversampling;
		
			correlationBit = row.correlationBit;
		
			flagRow = row.flagRow;
		
		
		
		
		if (row.assocSpectralWindowIdExists) {
			assocSpectralWindowId = row.assocSpectralWindowId;		
			assocSpectralWindowIdExists = true;
		}
		else
			assocSpectralWindowIdExists = false;
		
		if (row.dopplerIdExists) {
			dopplerId = row.dopplerId;		
			dopplerIdExists = true;
		}
		else
			dopplerIdExists = false;
		
		if (row.imageSpectralWindowIdExists) {
			imageSpectralWindowId = row.imageSpectralWindowId;		
			imageSpectralWindowIdExists = true;
		}
		else
			imageSpectralWindowIdExists = false;
		
		if (row.nameExists) {
			name = row.name;		
			nameExists = true;
		}
		else
			nameExists = false;
		
		if (row.measFreqRefExists) {
			measFreqRef = row.measFreqRef;		
			measFreqRefExists = true;
		}
		else
			measFreqRefExists = false;
		
		if (row.basebandNameExists) {
			basebandName = row.basebandName;		
			basebandNameExists = true;
		}
		else
			basebandNameExists = false;
		
		if (row.bbcSidebandExists) {
			bbcSideband = row.bbcSideband;		
			bbcSidebandExists = true;
		}
		else
			bbcSidebandExists = false;
		
		if (row.ifConvChainExists) {
			ifConvChain = row.ifConvChain;		
			ifConvChainExists = true;
		}
		else
			ifConvChainExists = false;
		
		if (row.freqGroupExists) {
			freqGroup = row.freqGroup;		
			freqGroupExists = true;
		}
		else
			freqGroupExists = false;
		
		if (row.freqGroupNameExists) {
			freqGroupName = row.freqGroupName;		
			freqGroupNameExists = true;
		}
		else
			freqGroupNameExists = false;
		
		if (row.assocNatureExists) {
			assocNature = row.assocNature;		
			assocNatureExists = true;
		}
		else
			assocNatureExists = false;
		
		}	
	}

	
	bool SpectralWindowRow::compareNoAutoInc(int numChan, Frequency refFreq, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<Frequency > effectiveBw, vector<Frequency > resolution, Frequency totBandwidth, NetSidebandMod::NetSideband netSideband, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, bool quantization, WindowFunctionMod::WindowFunction windowFunction, bool oversampling, CorrelationBitMod::CorrelationBit correlationBit, bool flagRow) {
		bool result;
		result = true;
		
	
		
		result = result && (this->numChan == numChan);
		
		if (!result) return false;
	

	
		
		result = result && (this->refFreq == refFreq);
		
		if (!result) return false;
	

	
		
		result = result && (this->chanFreq == chanFreq);
		
		if (!result) return false;
	

	
		
		result = result && (this->chanWidth == chanWidth);
		
		if (!result) return false;
	

	
		
		result = result && (this->effectiveBw == effectiveBw);
		
		if (!result) return false;
	

	
		
		result = result && (this->resolution == resolution);
		
		if (!result) return false;
	

	
		
		result = result && (this->totBandwidth == totBandwidth);
		
		if (!result) return false;
	

	
		
		result = result && (this->netSideband == netSideband);
		
		if (!result) return false;
	

	
		
		result = result && (this->sidebandProcessingMode == sidebandProcessingMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->quantization == quantization);
		
		if (!result) return false;
	

	
		
		result = result && (this->windowFunction == windowFunction);
		
		if (!result) return false;
	

	
		
		result = result && (this->oversampling == oversampling);
		
		if (!result) return false;
	

	
		
		result = result && (this->correlationBit == correlationBit);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagRow == flagRow);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool SpectralWindowRow::compareRequiredValue(int numChan, Frequency refFreq, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<Frequency > effectiveBw, vector<Frequency > resolution, Frequency totBandwidth, NetSidebandMod::NetSideband netSideband, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, bool quantization, WindowFunctionMod::WindowFunction windowFunction, bool oversampling, CorrelationBitMod::CorrelationBit correlationBit, bool flagRow) {
		bool result;
		result = true;
		
	
		if (!(this->numChan == numChan)) return false;
	

	
		if (!(this->refFreq == refFreq)) return false;
	

	
		if (!(this->chanFreq == chanFreq)) return false;
	

	
		if (!(this->chanWidth == chanWidth)) return false;
	

	
		if (!(this->effectiveBw == effectiveBw)) return false;
	

	
		if (!(this->resolution == resolution)) return false;
	

	
		if (!(this->totBandwidth == totBandwidth)) return false;
	

	
		if (!(this->netSideband == netSideband)) return false;
	

	
		if (!(this->sidebandProcessingMode == sidebandProcessingMode)) return false;
	

	
		if (!(this->quantization == quantization)) return false;
	

	
		if (!(this->windowFunction == windowFunction)) return false;
	

	
		if (!(this->oversampling == oversampling)) return false;
	

	
		if (!(this->correlationBit == correlationBit)) return false;
	

	
		if (!(this->flagRow == flagRow)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SpectralWindowRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool SpectralWindowRow::equalByRequiredValue(SpectralWindowRow* x) {
		
			
		if (this->numChan != x->numChan) return false;
			
		if (this->refFreq != x->refFreq) return false;
			
		if (this->chanFreq != x->chanFreq) return false;
			
		if (this->chanWidth != x->chanWidth) return false;
			
		if (this->effectiveBw != x->effectiveBw) return false;
			
		if (this->resolution != x->resolution) return false;
			
		if (this->totBandwidth != x->totBandwidth) return false;
			
		if (this->netSideband != x->netSideband) return false;
			
		if (this->sidebandProcessingMode != x->sidebandProcessingMode) return false;
			
		if (this->quantization != x->quantization) return false;
			
		if (this->windowFunction != x->windowFunction) return false;
			
		if (this->oversampling != x->oversampling) return false;
			
		if (this->correlationBit != x->correlationBit) return false;
			
		if (this->flagRow != x->flagRow) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
