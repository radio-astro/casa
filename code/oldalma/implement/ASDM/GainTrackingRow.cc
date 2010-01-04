
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
 * File GainTrackingRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <GainTrackingRow.h>
#include <GainTrackingTable.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <FeedTable.h>
#include <FeedRow.h>
	

using asdm::ASDM;
using asdm::GainTrackingRow;
using asdm::GainTrackingTable;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::FeedTable;
using asdm::FeedRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	GainTrackingRow::~GainTrackingRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	GainTrackingTable &GainTrackingRow::getTable() const {
		return table;
	}
	
	void GainTrackingRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a GainTrackingRowIDL struct.
	 */
	GainTrackingRowIDL *GainTrackingRow::toIDL() const {
		GainTrackingRowIDL *x = new GainTrackingRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->attenuator = attenuator;
 				
 			
		
	

	
  		
		
		x->samplingLevelExists = samplingLevelExists;
		
		
			
				
		x->samplingLevel = samplingLevel;
 				
 			
		
	

	
  		
		
		
			
		x->delayoff1 = delayoff1.toIDLInterval();
			
		
	

	
  		
		
		
			
		x->delayoff2 = delayoff2.toIDLInterval();
			
		
	

	
  		
		
		
			
		x->phaseoff1 = phaseoff1.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->phaseoff2 = phaseoff2.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->rateoff1 = rateoff1.toIDLAngularRate();
			
		
	

	
  		
		
		
			
		x->rateoff2 = rateoff2.toIDLAngularRate();
			
		
	

	
  		
		
		x->phaseRefOffsetExists = phaseRefOffsetExists;
		
		
			
		x->phaseRefOffset = phaseRefOffset.toIDLAngle();
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
				
		x->feedId = feedId;
 				
 			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct GainTrackingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void GainTrackingRow::setFromIDL (GainTrackingRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setAttenuator(x.attenuator);
  			
 		
		
	

	
		
		samplingLevelExists = x.samplingLevelExists;
		if (x.samplingLevelExists) {
		
		
			
		setSamplingLevel(x.samplingLevel);
  			
 		
		
		}
		
	

	
		
		
			
		setDelayoff1(Interval (x.delayoff1));
			
 		
		
	

	
		
		
			
		setDelayoff2(Interval (x.delayoff2));
			
 		
		
	

	
		
		
			
		setPhaseoff1(Angle (x.phaseoff1));
			
 		
		
	

	
		
		
			
		setPhaseoff2(Angle (x.phaseoff2));
			
 		
		
	

	
		
		
			
		setRateoff1(AngularRate (x.rateoff1));
			
 		
		
	

	
		
		
			
		setRateoff2(AngularRate (x.rateoff2));
			
 		
		
	

	
		
		phaseRefOffsetExists = x.phaseRefOffsetExists;
		if (x.phaseRefOffsetExists) {
		
		
			
		setPhaseRefOffset(Angle (x.phaseRefOffset));
			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"GainTracking");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string GainTrackingRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(attenuator, "attenuator", buf);
		
		
	

  	
 		
		if (samplingLevelExists) {
		
		
		Parser::toXML(samplingLevel, "samplingLevel", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(delayoff1, "delayoff1", buf);
		
		
	

  	
 		
		
		Parser::toXML(delayoff2, "delayoff2", buf);
		
		
	

  	
 		
		
		Parser::toXML(phaseoff1, "phaseoff1", buf);
		
		
	

  	
 		
		
		Parser::toXML(phaseoff2, "phaseoff2", buf);
		
		
	

  	
 		
		
		Parser::toXML(rateoff1, "rateoff1", buf);
		
		
	

  	
 		
		
		Parser::toXML(rateoff2, "rateoff2", buf);
		
		
	

  	
 		
		if (phaseRefOffsetExists) {
		
		
		Parser::toXML(phaseRefOffset, "phaseRefOffset", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(feedId, "feedId", buf);
		
		
	

  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void GainTrackingRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setAttenuator(Parser::getFloat("attenuator","GainTracking",rowDoc));
			
		
	

	
  		
        if (row.isStr("<samplingLevel>")) {
			
	  		setSamplingLevel(Parser::getFloat("samplingLevel","GainTracking",rowDoc));
			
		}
 		
	

	
  		
			
	  	setDelayoff1(Parser::getInterval("delayoff1","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setDelayoff2(Parser::getInterval("delayoff2","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setPhaseoff1(Parser::getAngle("phaseoff1","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setPhaseoff2(Parser::getAngle("phaseoff2","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setRateoff1(Parser::getAngularRate("rateoff1","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setRateoff2(Parser::getAngularRate("rateoff2","GainTracking",rowDoc));
			
		
	

	
  		
        if (row.isStr("<phaseRefOffset>")) {
			
	  		setPhaseRefOffset(Parser::getAngle("phaseRefOffset","GainTracking",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"GainTracking");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval GainTrackingRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void GainTrackingRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "GainTracking");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get attenuator.
 	 * @return attenuator as float
 	 */
 	float GainTrackingRow::getAttenuator() const {
	
  		return attenuator;
 	}

 	/**
 	 * Set attenuator with the specified float.
 	 * @param attenuator The float value to which attenuator is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setAttenuator (float attenuator)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->attenuator = attenuator;
	
 	}
	
	

	
	/**
	 * The attribute samplingLevel is optional. Return true if this attribute exists.
	 * @return true if and only if the samplingLevel attribute exists. 
	 */
	bool GainTrackingRow::isSamplingLevelExists() const {
		return samplingLevelExists;
	}
	

	
 	/**
 	 * Get samplingLevel, which is optional.
 	 * @return samplingLevel as float
 	 * @throw IllegalAccessException If samplingLevel does not exist.
 	 */
 	float GainTrackingRow::getSamplingLevel() const throw(IllegalAccessException) {
		if (!samplingLevelExists) {
			throw IllegalAccessException("samplingLevel", "GainTracking");
		}
	
  		return samplingLevel;
 	}

 	/**
 	 * Set samplingLevel with the specified float.
 	 * @param samplingLevel The float value to which samplingLevel is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setSamplingLevel (float samplingLevel) {
	
 		this->samplingLevel = samplingLevel;
	
		samplingLevelExists = true;
	
 	}
	
	
	/**
	 * Mark samplingLevel, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearSamplingLevel () {
		samplingLevelExists = false;
	}
	

	

	
 	/**
 	 * Get delayoff1.
 	 * @return delayoff1 as Interval
 	 */
 	Interval GainTrackingRow::getDelayoff1() const {
	
  		return delayoff1;
 	}

 	/**
 	 * Set delayoff1 with the specified Interval.
 	 * @param delayoff1 The Interval value to which delayoff1 is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setDelayoff1 (Interval delayoff1)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->delayoff1 = delayoff1;
	
 	}
	
	

	

	
 	/**
 	 * Get delayoff2.
 	 * @return delayoff2 as Interval
 	 */
 	Interval GainTrackingRow::getDelayoff2() const {
	
  		return delayoff2;
 	}

 	/**
 	 * Set delayoff2 with the specified Interval.
 	 * @param delayoff2 The Interval value to which delayoff2 is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setDelayoff2 (Interval delayoff2)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->delayoff2 = delayoff2;
	
 	}
	
	

	

	
 	/**
 	 * Get phaseoff1.
 	 * @return phaseoff1 as Angle
 	 */
 	Angle GainTrackingRow::getPhaseoff1() const {
	
  		return phaseoff1;
 	}

 	/**
 	 * Set phaseoff1 with the specified Angle.
 	 * @param phaseoff1 The Angle value to which phaseoff1 is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setPhaseoff1 (Angle phaseoff1)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phaseoff1 = phaseoff1;
	
 	}
	
	

	

	
 	/**
 	 * Get phaseoff2.
 	 * @return phaseoff2 as Angle
 	 */
 	Angle GainTrackingRow::getPhaseoff2() const {
	
  		return phaseoff2;
 	}

 	/**
 	 * Set phaseoff2 with the specified Angle.
 	 * @param phaseoff2 The Angle value to which phaseoff2 is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setPhaseoff2 (Angle phaseoff2)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->phaseoff2 = phaseoff2;
	
 	}
	
	

	

	
 	/**
 	 * Get rateoff1.
 	 * @return rateoff1 as AngularRate
 	 */
 	AngularRate GainTrackingRow::getRateoff1() const {
	
  		return rateoff1;
 	}

 	/**
 	 * Set rateoff1 with the specified AngularRate.
 	 * @param rateoff1 The AngularRate value to which rateoff1 is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setRateoff1 (AngularRate rateoff1)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->rateoff1 = rateoff1;
	
 	}
	
	

	

	
 	/**
 	 * Get rateoff2.
 	 * @return rateoff2 as AngularRate
 	 */
 	AngularRate GainTrackingRow::getRateoff2() const {
	
  		return rateoff2;
 	}

 	/**
 	 * Set rateoff2 with the specified AngularRate.
 	 * @param rateoff2 The AngularRate value to which rateoff2 is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setRateoff2 (AngularRate rateoff2)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->rateoff2 = rateoff2;
	
 	}
	
	

	
	/**
	 * The attribute phaseRefOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseRefOffset attribute exists. 
	 */
	bool GainTrackingRow::isPhaseRefOffsetExists() const {
		return phaseRefOffsetExists;
	}
	

	
 	/**
 	 * Get phaseRefOffset, which is optional.
 	 * @return phaseRefOffset as Angle
 	 * @throw IllegalAccessException If phaseRefOffset does not exist.
 	 */
 	Angle GainTrackingRow::getPhaseRefOffset() const throw(IllegalAccessException) {
		if (!phaseRefOffsetExists) {
			throw IllegalAccessException("phaseRefOffset", "GainTracking");
		}
	
  		return phaseRefOffset;
 	}

 	/**
 	 * Set phaseRefOffset with the specified Angle.
 	 * @param phaseRefOffset The Angle value to which phaseRefOffset is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setPhaseRefOffset (Angle phaseRefOffset) {
	
 		this->phaseRefOffset = phaseRefOffset;
	
		phaseRefOffsetExists = true;
	
 	}
	
	
	/**
	 * Mark phaseRefOffset, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearPhaseRefOffset () {
		phaseRefOffsetExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag GainTrackingRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void GainTrackingRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "GainTracking");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int GainTrackingRow::getFeedId() const {
	
  		return feedId;
 	}

 	/**
 	 * Set feedId with the specified int.
 	 * @param feedId The int value to which feedId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void GainTrackingRow::setFeedId (int feedId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("feedId", "GainTracking");
		
  		}
  	
 		this->feedId = feedId;
	
 	}
	
	

	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag GainTrackingRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void GainTrackingRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "GainTracking");
		
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
	 SpectralWindowRow* GainTrackingRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* GainTrackingRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of GainTracking table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having their attribut feedId == this->feedId
	 */
	vector <FeedRow *> GainTrackingRow::getFeeds() {
		
			return table.getContainer().getFeed().getRowByFeedId(feedId);
		
	}
	

	

	
	/**
	 * Create a GainTrackingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	GainTrackingRow::GainTrackingRow (GainTrackingTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	
		samplingLevelExists = false;
	

	

	

	

	

	

	

	
		phaseRefOffsetExists = false;
	

	
	

	

	

	
	
	
	

	

	

	

	

	

	

	

	

	
	
	}
	
	GainTrackingRow::GainTrackingRow (GainTrackingTable &t, GainTrackingRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	
		samplingLevelExists = false;
	

	

	

	

	

	

	

	
		phaseRefOffsetExists = false;
	

	
	

	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			feedId = row.feedId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			attenuator = row.attenuator;
		
			delayoff1 = row.delayoff1;
		
			delayoff2 = row.delayoff2;
		
			phaseoff1 = row.phaseoff1;
		
			phaseoff2 = row.phaseoff2;
		
			rateoff1 = row.rateoff1;
		
			rateoff2 = row.rateoff2;
		
		
		
		
		if (row.samplingLevelExists) {
			samplingLevel = row.samplingLevel;		
			samplingLevelExists = true;
		}
		else
			samplingLevelExists = false;
		
		if (row.phaseRefOffsetExists) {
			phaseRefOffset = row.phaseRefOffset;		
			phaseRefOffsetExists = true;
		}
		else
			phaseRefOffsetExists = false;
		
		}	
	}

	
	bool GainTrackingRow::compareNoAutoInc(Tag antennaId, int feedId, Tag spectralWindowId, ArrayTimeInterval timeInterval, float attenuator, Interval delayoff1, Interval delayoff2, Angle phaseoff1, Angle phaseoff2, AngularRate rateoff1, AngularRate rateoff2) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->feedId == feedId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->attenuator == attenuator);
		
		if (!result) return false;
	

	
		
		result = result && (this->delayoff1 == delayoff1);
		
		if (!result) return false;
	

	
		
		result = result && (this->delayoff2 == delayoff2);
		
		if (!result) return false;
	

	
		
		result = result && (this->phaseoff1 == phaseoff1);
		
		if (!result) return false;
	

	
		
		result = result && (this->phaseoff2 == phaseoff2);
		
		if (!result) return false;
	

	
		
		result = result && (this->rateoff1 == rateoff1);
		
		if (!result) return false;
	

	
		
		result = result && (this->rateoff2 == rateoff2);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool GainTrackingRow::compareRequiredValue(float attenuator, Interval delayoff1, Interval delayoff2, Angle phaseoff1, Angle phaseoff2, AngularRate rateoff1, AngularRate rateoff2) {
		bool result;
		result = true;
		
	
		if (!(this->attenuator == attenuator)) return false;
	

	
		if (!(this->delayoff1 == delayoff1)) return false;
	

	
		if (!(this->delayoff2 == delayoff2)) return false;
	

	
		if (!(this->phaseoff1 == phaseoff1)) return false;
	

	
		if (!(this->phaseoff2 == phaseoff2)) return false;
	

	
		if (!(this->rateoff1 == rateoff1)) return false;
	

	
		if (!(this->rateoff2 == rateoff2)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the GainTrackingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool GainTrackingRow::equalByRequiredValue(GainTrackingRow* x) {
		
			
		if (this->attenuator != x->attenuator) return false;
			
		if (this->delayoff1 != x->delayoff1) return false;
			
		if (this->delayoff2 != x->delayoff2) return false;
			
		if (this->phaseoff1 != x->phaseoff1) return false;
			
		if (this->phaseoff2 != x->phaseoff2) return false;
			
		if (this->rateoff1 != x->rateoff1) return false;
			
		if (this->rateoff2 != x->rateoff2) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
