
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
 * File ReceiverRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <ReceiverRow.h>
#include <ReceiverTable.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>
	

using asdm::ASDM;
using asdm::ReceiverRow;
using asdm::ReceiverTable;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	ReceiverRow::~ReceiverRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	ReceiverTable &ReceiverRow::getTable() const {
		return table;
	}
	
	void ReceiverRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ReceiverRowIDL struct.
	 */
	ReceiverRowIDL *ReceiverRow::toIDL() const {
		ReceiverRowIDL *x = new ReceiverRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->numLo = numLo;
 				
 			
		
	

	
  		
		
		
			
				
		x->name = CORBA::string_dup(name.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->frequencyBand = frequencyBand;
 				
 			
		
	

	
  		
		
		
			
		x->freqLo.length(freqLo.size());
		for (unsigned int i = 0; i < freqLo.size(); ++i) {
			
			x->freqLo[i] = freqLo.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->receiverSideband = receiverSideband;
 				
 			
		
	

	
  		
		
		
			
		x->sidebandLo.length(sidebandLo.size());
		for (unsigned int i = 0; i < sidebandLo.size(); ++i) {
			
				
			x->sidebandLo[i] = sidebandLo.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->dewarNameExists = dewarNameExists;
		
		
			
				
		x->dewarName = CORBA::string_dup(dewarName.c_str());
				
 			
		
	

	
  		
		
		
			
		x->tDewar = tDewar.toIDLTemperature();
			
		
	

	
  		
		
		
			
		x->stabilityDuration = stabilityDuration.toIDLInterval();
			
		
	

	
  		
		
		
			
				
		x->stability = stability;
 				
 			
		
	

	
  		
		
		x->stabilityflagExists = stabilityflagExists;
		
		
			
				
		x->stabilityflag = stabilityflag;
 				
 			
		
	

	
  		
		
		
			
				
		x->receiverId = receiverId;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ReceiverRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void ReceiverRow::setFromIDL (ReceiverRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumLo(x.numLo);
  			
 		
		
	

	
		
		
			
		setName(string (x.name));
			
 		
		
	

	
		
		
			
		setFrequencyBand(x.frequencyBand);
  			
 		
		
	

	
		
		
			
		freqLo .clear();
		for (unsigned int i = 0; i <x.freqLo.length(); ++i) {
			
			freqLo.push_back(Frequency (x.freqLo[i]));
			
		}
			
  		
		
	

	
		
		
			
		setReceiverSideband(x.receiverSideband);
  			
 		
		
	

	
		
		
			
		sidebandLo .clear();
		for (unsigned int i = 0; i <x.sidebandLo.length(); ++i) {
			
			sidebandLo.push_back(x.sidebandLo[i]);
  			
		}
			
  		
		
	

	
		
		dewarNameExists = x.dewarNameExists;
		if (x.dewarNameExists) {
		
		
			
		setDewarName(string (x.dewarName));
			
 		
		
		}
		
	

	
		
		
			
		setTDewar(Temperature (x.tDewar));
			
 		
		
	

	
		
		
			
		setStabilityDuration(Interval (x.stabilityDuration));
			
 		
		
	

	
		
		
			
		setStability(x.stability);
  			
 		
		
	

	
		
		stabilityflagExists = x.stabilityflagExists;
		if (x.stabilityflagExists) {
		
		
			
		setStabilityflag(x.stabilityflag);
  			
 		
		
		}
		
	

	
		
		
			
		setReceiverId(x.receiverId);
  			
 		
		
	

	
	
		
	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Receiver");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string ReceiverRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(numLo, "numLo", buf);
		
		
	

  	
 		
		
		Parser::toXML(name, "name", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("frequencyBand", frequencyBand));
		
		
	

  	
 		
		
		Parser::toXML(freqLo, "freqLo", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverSideband", receiverSideband));
		
		
	

  	
 		
		
		Parser::toXML(sidebandLo, "sidebandLo", buf);
		
		
	

  	
 		
		if (dewarNameExists) {
		
		
		Parser::toXML(dewarName, "dewarName", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(tDewar, "tDewar", buf);
		
		
	

  	
 		
		
		Parser::toXML(stabilityDuration, "stabilityDuration", buf);
		
		
	

  	
 		
		
		Parser::toXML(stability, "stability", buf);
		
		
	

  	
 		
		if (stabilityflagExists) {
		
		
		Parser::toXML(stabilityflag, "stabilityflag", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(receiverId, "receiverId", buf);
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void ReceiverRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Receiver",rowDoc));
			
		
	

	
  		
			
	  	setNumLo(Parser::getInteger("numLo","Receiver",rowDoc));
			
		
	

	
  		
			
	  	setName(Parser::getString("name","Receiver",rowDoc));
			
		
	

	
		
		
		
		frequencyBand = EnumerationParser::getReceiverBand("frequencyBand","Receiver",rowDoc);
		
		
		
	

	
  		
			
					
	  	setFreqLo(Parser::get1DFrequency("freqLo","Receiver",rowDoc));
	  			
	  		
		
	

	
		
		
		
		receiverSideband = EnumerationParser::getReceiverSideband("receiverSideband","Receiver",rowDoc);
		
		
		
	

	
  		
			
					
	  	setSidebandLo(Parser::get1DInteger("sidebandLo","Receiver",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<dewarName>")) {
			
	  		setDewarName(Parser::getString("dewarName","Receiver",rowDoc));
			
		}
 		
	

	
  		
			
	  	setTDewar(Parser::getTemperature("tDewar","Receiver",rowDoc));
			
		
	

	
  		
			
	  	setStabilityDuration(Parser::getInterval("stabilityDuration","Receiver",rowDoc));
			
		
	

	
  		
			
	  	setStability(Parser::getDouble("stability","Receiver",rowDoc));
			
		
	

	
  		
        if (row.isStr("<stabilityflag>")) {
			
	  		setStabilityflag(Parser::getBoolean("stabilityflag","Receiver",rowDoc));
			
		}
 		
	

	
  		
			
	  	setReceiverId(Parser::getInteger("receiverId","Receiver",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Receiver");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval ReceiverRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ReceiverRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "Receiver");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get numLo.
 	 * @return numLo as int
 	 */
 	int ReceiverRow::getNumLo() const {
	
  		return numLo;
 	}

 	/**
 	 * Set numLo with the specified int.
 	 * @param numLo The int value to which numLo is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setNumLo (int numLo)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numLo = numLo;
	
 	}
	
	

	

	
 	/**
 	 * Get name.
 	 * @return name as string
 	 */
 	string ReceiverRow::getName() const {
	
  		return name;
 	}

 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setName (string name)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->name = name;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyBand.
 	 * @return frequencyBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand ReceiverRow::getFrequencyBand() const {
	
  		return frequencyBand;
 	}

 	/**
 	 * Set frequencyBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param frequencyBand The ReceiverBandMod::ReceiverBand value to which frequencyBand is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setFrequencyBand (ReceiverBandMod::ReceiverBand frequencyBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyBand = frequencyBand;
	
 	}
	
	

	

	
 	/**
 	 * Get freqLo.
 	 * @return freqLo as vector<Frequency >
 	 */
 	vector<Frequency > ReceiverRow::getFreqLo() const {
	
  		return freqLo;
 	}

 	/**
 	 * Set freqLo with the specified vector<Frequency >.
 	 * @param freqLo The vector<Frequency > value to which freqLo is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setFreqLo (vector<Frequency > freqLo)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->freqLo = freqLo;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverSideband.
 	 * @return receiverSideband as ReceiverSidebandMod::ReceiverSideband
 	 */
 	ReceiverSidebandMod::ReceiverSideband ReceiverRow::getReceiverSideband() const {
	
  		return receiverSideband;
 	}

 	/**
 	 * Set receiverSideband with the specified ReceiverSidebandMod::ReceiverSideband.
 	 * @param receiverSideband The ReceiverSidebandMod::ReceiverSideband value to which receiverSideband is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setReceiverSideband (ReceiverSidebandMod::ReceiverSideband receiverSideband)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverSideband = receiverSideband;
	
 	}
	
	

	

	
 	/**
 	 * Get sidebandLo.
 	 * @return sidebandLo as vector<int >
 	 */
 	vector<int > ReceiverRow::getSidebandLo() const {
	
  		return sidebandLo;
 	}

 	/**
 	 * Set sidebandLo with the specified vector<int >.
 	 * @param sidebandLo The vector<int > value to which sidebandLo is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setSidebandLo (vector<int > sidebandLo)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sidebandLo = sidebandLo;
	
 	}
	
	

	
	/**
	 * The attribute dewarName is optional. Return true if this attribute exists.
	 * @return true if and only if the dewarName attribute exists. 
	 */
	bool ReceiverRow::isDewarNameExists() const {
		return dewarNameExists;
	}
	

	
 	/**
 	 * Get dewarName, which is optional.
 	 * @return dewarName as string
 	 * @throw IllegalAccessException If dewarName does not exist.
 	 */
 	string ReceiverRow::getDewarName() const throw(IllegalAccessException) {
		if (!dewarNameExists) {
			throw IllegalAccessException("dewarName", "Receiver");
		}
	
  		return dewarName;
 	}

 	/**
 	 * Set dewarName with the specified string.
 	 * @param dewarName The string value to which dewarName is to be set.
 	 
 	
 	 */
 	void ReceiverRow::setDewarName (string dewarName) {
	
 		this->dewarName = dewarName;
	
		dewarNameExists = true;
	
 	}
	
	
	/**
	 * Mark dewarName, which is an optional field, as non-existent.
	 */
	void ReceiverRow::clearDewarName () {
		dewarNameExists = false;
	}
	

	

	
 	/**
 	 * Get tDewar.
 	 * @return tDewar as Temperature
 	 */
 	Temperature ReceiverRow::getTDewar() const {
	
  		return tDewar;
 	}

 	/**
 	 * Set tDewar with the specified Temperature.
 	 * @param tDewar The Temperature value to which tDewar is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setTDewar (Temperature tDewar)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->tDewar = tDewar;
	
 	}
	
	

	

	
 	/**
 	 * Get stabilityDuration.
 	 * @return stabilityDuration as Interval
 	 */
 	Interval ReceiverRow::getStabilityDuration() const {
	
  		return stabilityDuration;
 	}

 	/**
 	 * Set stabilityDuration with the specified Interval.
 	 * @param stabilityDuration The Interval value to which stabilityDuration is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setStabilityDuration (Interval stabilityDuration)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->stabilityDuration = stabilityDuration;
	
 	}
	
	

	

	
 	/**
 	 * Get stability.
 	 * @return stability as double
 	 */
 	double ReceiverRow::getStability() const {
	
  		return stability;
 	}

 	/**
 	 * Set stability with the specified double.
 	 * @param stability The double value to which stability is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setStability (double stability)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->stability = stability;
	
 	}
	
	

	
	/**
	 * The attribute stabilityflag is optional. Return true if this attribute exists.
	 * @return true if and only if the stabilityflag attribute exists. 
	 */
	bool ReceiverRow::isStabilityflagExists() const {
		return stabilityflagExists;
	}
	

	
 	/**
 	 * Get stabilityflag, which is optional.
 	 * @return stabilityflag as bool
 	 * @throw IllegalAccessException If stabilityflag does not exist.
 	 */
 	bool ReceiverRow::getStabilityflag() const throw(IllegalAccessException) {
		if (!stabilityflagExists) {
			throw IllegalAccessException("stabilityflag", "Receiver");
		}
	
  		return stabilityflag;
 	}

 	/**
 	 * Set stabilityflag with the specified bool.
 	 * @param stabilityflag The bool value to which stabilityflag is to be set.
 	 
 	
 	 */
 	void ReceiverRow::setStabilityflag (bool stabilityflag) {
	
 		this->stabilityflag = stabilityflag;
	
		stabilityflagExists = true;
	
 	}
	
	
	/**
	 * Mark stabilityflag, which is an optional field, as non-existent.
	 */
	void ReceiverRow::clearStabilityflag () {
		stabilityflagExists = false;
	}
	

	

	
 	/**
 	 * Get receiverId.
 	 * @return receiverId as int
 	 */
 	int ReceiverRow::getReceiverId() const {
	
  		return receiverId;
 	}

 	/**
 	 * Set receiverId with the specified int.
 	 * @param receiverId The int value to which receiverId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ReceiverRow::setReceiverId (int receiverId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverId", "Receiver");
		
  		}
  	
 		this->receiverId = receiverId;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag ReceiverRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ReceiverRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "Receiver");
		
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
	 SpectralWindowRow* ReceiverRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	/**
	 * Create a ReceiverRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ReceiverRow::ReceiverRow (ReceiverTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	
		dewarNameExists = false;
	

	

	

	

	
		stabilityflagExists = false;
	

	

	
	

	
	
	
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
frequencyBand = CReceiverBand::from_int(0);
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverSideband = CReceiverSideband::from_int(0);
	

	

	

	

	

	

	

	
	
	}
	
	ReceiverRow::ReceiverRow (ReceiverTable &t, ReceiverRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	
		dewarNameExists = false;
	

	

	

	

	
		stabilityflagExists = false;
	

	

	
	
		
		}
		else {
	
		
			receiverId = row.receiverId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			numLo = row.numLo;
		
			name = row.name;
		
			frequencyBand = row.frequencyBand;
		
			freqLo = row.freqLo;
		
			receiverSideband = row.receiverSideband;
		
			sidebandLo = row.sidebandLo;
		
			tDewar = row.tDewar;
		
			stabilityDuration = row.stabilityDuration;
		
			stability = row.stability;
		
		
		
		
		if (row.dewarNameExists) {
			dewarName = row.dewarName;		
			dewarNameExists = true;
		}
		else
			dewarNameExists = false;
		
		if (row.stabilityflagExists) {
			stabilityflag = row.stabilityflag;		
			stabilityflagExists = true;
		}
		else
			stabilityflagExists = false;
		
		}	
	}

	
	bool ReceiverRow::compareNoAutoInc(Tag spectralWindowId, ArrayTimeInterval timeInterval, int numLo, string name, ReceiverBandMod::ReceiverBand frequencyBand, vector<Frequency > freqLo, ReceiverSidebandMod::ReceiverSideband receiverSideband, vector<int > sidebandLo, Temperature tDewar, Interval stabilityDuration, double stability) {
		bool result;
		result = true;
		
	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->numLo == numLo);
		
		if (!result) return false;
	

	
		
		result = result && (this->name == name);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyBand == frequencyBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->freqLo == freqLo);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverSideband == receiverSideband);
		
		if (!result) return false;
	

	
		
		result = result && (this->sidebandLo == sidebandLo);
		
		if (!result) return false;
	

	
		
		result = result && (this->tDewar == tDewar);
		
		if (!result) return false;
	

	
		
		result = result && (this->stabilityDuration == stabilityDuration);
		
		if (!result) return false;
	

	
		
		result = result && (this->stability == stability);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool ReceiverRow::compareRequiredValue(int numLo, string name, ReceiverBandMod::ReceiverBand frequencyBand, vector<Frequency > freqLo, ReceiverSidebandMod::ReceiverSideband receiverSideband, vector<int > sidebandLo, Temperature tDewar, Interval stabilityDuration, double stability) {
		bool result;
		result = true;
		
	
		if (!(this->numLo == numLo)) return false;
	

	
		if (!(this->name == name)) return false;
	

	
		if (!(this->frequencyBand == frequencyBand)) return false;
	

	
		if (!(this->freqLo == freqLo)) return false;
	

	
		if (!(this->receiverSideband == receiverSideband)) return false;
	

	
		if (!(this->sidebandLo == sidebandLo)) return false;
	

	
		if (!(this->tDewar == tDewar)) return false;
	

	
		if (!(this->stabilityDuration == stabilityDuration)) return false;
	

	
		if (!(this->stability == stability)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ReceiverRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool ReceiverRow::equalByRequiredValue(ReceiverRow* x) {
		
			
		if (this->numLo != x->numLo) return false;
			
		if (this->name != x->name) return false;
			
		if (this->frequencyBand != x->frequencyBand) return false;
			
		if (this->freqLo != x->freqLo) return false;
			
		if (this->receiverSideband != x->receiverSideband) return false;
			
		if (this->sidebandLo != x->sidebandLo) return false;
			
		if (this->tDewar != x->tDewar) return false;
			
		if (this->stabilityDuration != x->stabilityDuration) return false;
			
		if (this->stability != x->stability) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
