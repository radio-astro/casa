
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
	
		
	
  		
		
		
			
				
		x->receiverId = receiverId;
 				
 			
		
	

	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->name = CORBA::string_dup(name.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numLO = numLO;
 				
 			
		
	

	
  		
		
		
			
				
		x->frequencyBand = frequencyBand;
 				
 			
		
	

	
  		
		
		
			
		x->freqLO.length(freqLO.size());
		for (unsigned int i = 0; i < freqLO.size(); ++i) {
			
			x->freqLO[i] = freqLO.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->receiverSideband = receiverSideband;
 				
 			
		
	

	
  		
		
		
			
		x->sidebandLO.length(sidebandLO.size());
		for (unsigned int i = 0; i < sidebandLO.size(); ++i) {
			
				
			x->sidebandLO[i] = sidebandLO.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ReceiverRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void ReceiverRow::setFromIDL (ReceiverRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setReceiverId(x.receiverId);
  			
 		
		
	

	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setName(string (x.name));
			
 		
		
	

	
		
		
			
		setNumLO(x.numLO);
  			
 		
		
	

	
		
		
			
		setFrequencyBand(x.frequencyBand);
  			
 		
		
	

	
		
		
			
		freqLO .clear();
		for (unsigned int i = 0; i <x.freqLO.length(); ++i) {
			
			freqLO.push_back(Frequency (x.freqLO[i]));
			
		}
			
  		
		
	

	
		
		
			
		setReceiverSideband(x.receiverSideband);
  			
 		
		
	

	
		
		
			
		sidebandLO .clear();
		for (unsigned int i = 0; i <x.sidebandLO.length(); ++i) {
			
			sidebandLO.push_back(x.sidebandLO[i]);
  			
		}
			
  		
		
	

	
	
		
	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Receiver");
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
		
	
		
  	
 		
		
		Parser::toXML(receiverId, "receiverId", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(name, "name", buf);
		
		
	

  	
 		
		
		Parser::toXML(numLO, "numLO", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("frequencyBand", frequencyBand));
		
		
	

  	
 		
		
		Parser::toXML(freqLO, "freqLO", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverSideband", receiverSideband));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("sidebandLO", sidebandLO));
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void ReceiverRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setReceiverId(Parser::getInteger("receiverId","Receiver",rowDoc));
			
		
	

	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Receiver",rowDoc));
			
		
	

	
  		
			
	  	setName(Parser::getString("name","Receiver",rowDoc));
			
		
	

	
  		
			
	  	setNumLO(Parser::getInteger("numLO","Receiver",rowDoc));
			
		
	

	
		
		
		
		frequencyBand = EnumerationParser::getReceiverBand("frequencyBand","Receiver",rowDoc);
		
		
		
	

	
  		
			
					
	  	setFreqLO(Parser::get1DFrequency("freqLO","Receiver",rowDoc));
	  			
	  		
		
	

	
		
		
		
		receiverSideband = EnumerationParser::getReceiverSideband("receiverSideband","Receiver",rowDoc);
		
		
		
	

	
		
		
		
		sidebandLO = EnumerationParser::getNetSideband1D("sidebandLO","Receiver",rowDoc);			
		
		
		
	

	
	
		
	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Receiver");
		}
	}
	
	void ReceiverRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeInt(receiverId);
				
		
	

	
	
		
	spectralWindowId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(name);
				
		
	

	
	
		
						
			eoss.writeInt(numLO);
				
		
	

	
	
		
					
			eoss.writeInt(frequencyBand);
				
		
	

	
	
		
	Frequency::toBin(freqLO, eoss);
		
	

	
	
		
					
			eoss.writeInt(receiverSideband);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) sidebandLO.size());
		for (unsigned int i = 0; i < sidebandLO.size(); i++)
				
			eoss.writeInt(sidebandLO.at(i));
				
				
						
		
	


	
	
	}
	
	ReceiverRow* ReceiverRow::fromBin(EndianISStream& eiss, ReceiverTable& table) {
		ReceiverRow* row = new  ReceiverRow(table);
		
		
		
	
	
		
			
		row->receiverId =  eiss.readInt();
			
		
	

	
		
		
		row->spectralWindowId =  Tag::fromBin(eiss);
		
	

	
		
		
		row->timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	

	
	
		
			
		row->name =  eiss.readString();
			
		
	

	
	
		
			
		row->numLO =  eiss.readInt();
			
		
	

	
	
		
			
		row->frequencyBand = CReceiverBand::from_int(eiss.readInt());
			
		
	

	
		
		
			
	
	row->freqLO = Frequency::from1DBin(eiss);	
	

		
	

	
	
		
			
		row->receiverSideband = CReceiverSideband::from_int(eiss.readInt());
			
		
	

	
	
		
			
	
		row->sidebandLO.clear();
		
		unsigned int sidebandLODim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < sidebandLODim1; i++)
			
			row->sidebandLO.push_back(CNetSideband::from_int(eiss.readInt()));
			
	

		
	

		
		
		
		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
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
 	 * Get numLO.
 	 * @return numLO as int
 	 */
 	int ReceiverRow::getNumLO() const {
	
  		return numLO;
 	}

 	/**
 	 * Set numLO with the specified int.
 	 * @param numLO The int value to which numLO is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setNumLO (int numLO)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numLO = numLO;
	
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
 	 * Get freqLO.
 	 * @return freqLO as vector<Frequency >
 	 */
 	vector<Frequency > ReceiverRow::getFreqLO() const {
	
  		return freqLO;
 	}

 	/**
 	 * Set freqLO with the specified vector<Frequency >.
 	 * @param freqLO The vector<Frequency > value to which freqLO is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setFreqLO (vector<Frequency > freqLO)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->freqLO = freqLO;
	
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
 	 * Get sidebandLO.
 	 * @return sidebandLO as vector<NetSidebandMod::NetSideband >
 	 */
 	vector<NetSidebandMod::NetSideband > ReceiverRow::getSidebandLO() const {
	
  		return sidebandLO;
 	}

 	/**
 	 * Set sidebandLO with the specified vector<NetSidebandMod::NetSideband >.
 	 * @param sidebandLO The vector<NetSidebandMod::NetSideband > value to which sidebandLO is to be set.
 	 
 	
 		
 	 */
 	void ReceiverRow::setSidebandLO (vector<NetSidebandMod::NetSideband > sidebandLO)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sidebandLO = sidebandLO;
	
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
		
	
	

	

	

	

	

	

	

	

	
	

	
	
	
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
frequencyBand = CReceiverBand::from_int(0);
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverSideband = CReceiverSideband::from_int(0);
	

	
	
	}
	
	ReceiverRow::ReceiverRow (ReceiverTable &t, ReceiverRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	
	
		
		}
		else {
	
		
			receiverId = row.receiverId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			name = row.name;
		
			numLO = row.numLO;
		
			frequencyBand = row.frequencyBand;
		
			freqLO = row.freqLO;
		
			receiverSideband = row.receiverSideband;
		
			sidebandLO = row.sidebandLO;
		
		
		
		
		}	
	}

	
	bool ReceiverRow::compareNoAutoInc(Tag spectralWindowId, ArrayTimeInterval timeInterval, string name, int numLO, ReceiverBandMod::ReceiverBand frequencyBand, vector<Frequency > freqLO, ReceiverSidebandMod::ReceiverSideband receiverSideband, vector<NetSidebandMod::NetSideband > sidebandLO) {
		bool result;
		result = true;
		
	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->name == name);
		
		if (!result) return false;
	

	
		
		result = result && (this->numLO == numLO);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyBand == frequencyBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->freqLO == freqLO);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverSideband == receiverSideband);
		
		if (!result) return false;
	

	
		
		result = result && (this->sidebandLO == sidebandLO);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool ReceiverRow::compareRequiredValue(string name, int numLO, ReceiverBandMod::ReceiverBand frequencyBand, vector<Frequency > freqLO, ReceiverSidebandMod::ReceiverSideband receiverSideband, vector<NetSidebandMod::NetSideband > sidebandLO) {
		bool result;
		result = true;
		
	
		if (!(this->name == name)) return false;
	

	
		if (!(this->numLO == numLO)) return false;
	

	
		if (!(this->frequencyBand == frequencyBand)) return false;
	

	
		if (!(this->freqLO == freqLO)) return false;
	

	
		if (!(this->receiverSideband == receiverSideband)) return false;
	

	
		if (!(this->sidebandLO == sidebandLO)) return false;
	

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
		
			
		if (this->name != x->name) return false;
			
		if (this->numLO != x->numLO) return false;
			
		if (this->frequencyBand != x->frequencyBand) return false;
			
		if (this->freqLO != x->freqLO) return false;
			
		if (this->receiverSideband != x->receiverSideband) return false;
			
		if (this->sidebandLO != x->sidebandLO) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
