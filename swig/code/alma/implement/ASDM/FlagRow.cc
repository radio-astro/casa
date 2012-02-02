
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
 * File FlagRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <FlagRow.h>
#include <FlagTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>
	

using asdm::ASDM;
using asdm::FlagRow;
using asdm::FlagTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	FlagRow::~FlagRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	FlagTable &FlagRow::getTable() const {
		return table;
	}

	bool FlagRow::isAdded() const {
		return hasBeenAdded;
	}	

	void FlagRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FlagRowIDL struct.
	 */
	FlagRowIDL *FlagRow::toIDL() const {
		FlagRowIDL *x = new FlagRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->flagId = flagId.toIDLTag();
			
		
	

	
  		
		
		
			
		x->startTime = startTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endTime = endTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->reason = CORBA::string_dup(reason.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		x->numPolarizationTypeExists = numPolarizationTypeExists;
		
		
			
				
		x->numPolarizationType = numPolarizationType;
 				
 			
		
	

	
  		
		
		x->numSpectralWindowExists = numSpectralWindowExists;
		
		
			
				
		x->numSpectralWindow = numSpectralWindow;
 				
 			
		
	

	
  		
		
		x->numPairedAntennaExists = numPairedAntennaExists;
		
		
			
				
		x->numPairedAntenna = numPairedAntenna;
 				
 			
		
	

	
  		
		
		x->polarizationTypeExists = polarizationTypeExists;
		
		
			
		x->polarizationType.length(polarizationType.size());
		for (unsigned int i = 0; i < polarizationType.size(); ++i) {
			
				
			x->polarizationType[i] = polarizationType.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
		
		x->antennaId.length(antennaId.size());
		for (unsigned int i = 0; i < antennaId.size(); ++i) {
			
			x->antennaId[i] = antennaId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
 		
		x->pairedAntennaIdExists = pairedAntennaIdExists;
		
		
		
		x->pairedAntennaId.length(pairedAntennaId.size());
		for (unsigned int i = 0; i < pairedAntennaId.size(); ++i) {
			
			x->pairedAntennaId[i] = pairedAntennaId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
 		
		x->spectralWindowIdExists = spectralWindowIdExists;
		
		
		
		x->spectralWindowId.length(spectralWindowId.size());
		for (unsigned int i = 0; i < spectralWindowId.size(); ++i) {
			
			x->spectralWindowId[i] = spectralWindowId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
		
	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FlagRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void FlagRow::setFromIDL (FlagRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setFlagId(Tag (x.flagId));
			
 		
		
	

	
		
		
			
		setStartTime(ArrayTime (x.startTime));
			
 		
		
	

	
		
		
			
		setEndTime(ArrayTime (x.endTime));
			
 		
		
	

	
		
		
			
		setReason(string (x.reason));
			
 		
		
	

	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		numPolarizationTypeExists = x.numPolarizationTypeExists;
		if (x.numPolarizationTypeExists) {
		
		
			
		setNumPolarizationType(x.numPolarizationType);
  			
 		
		
		}
		
	

	
		
		numSpectralWindowExists = x.numSpectralWindowExists;
		if (x.numSpectralWindowExists) {
		
		
			
		setNumSpectralWindow(x.numSpectralWindow);
  			
 		
		
		}
		
	

	
		
		numPairedAntennaExists = x.numPairedAntennaExists;
		if (x.numPairedAntennaExists) {
		
		
			
		setNumPairedAntenna(x.numPairedAntenna);
  			
 		
		
		}
		
	

	
		
		polarizationTypeExists = x.polarizationTypeExists;
		if (x.polarizationTypeExists) {
		
		
			
		polarizationType .clear();
		for (unsigned int i = 0; i <x.polarizationType.length(); ++i) {
			
			polarizationType.push_back(x.polarizationType[i]);
  			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		antennaId .clear();
		for (unsigned int i = 0; i <x.antennaId.length(); ++i) {
			
			antennaId.push_back(Tag (x.antennaId[i]));
			
		}
		
  	

	
		
		pairedAntennaIdExists = x.pairedAntennaIdExists;
		if (x.pairedAntennaIdExists) {
		
		pairedAntennaId .clear();
		for (unsigned int i = 0; i <x.pairedAntennaId.length(); ++i) {
			
			pairedAntennaId.push_back(Tag (x.pairedAntennaId[i]));
			
		}
		
		}
		
  	

	
		
		spectralWindowIdExists = x.spectralWindowIdExists;
		if (x.spectralWindowIdExists) {
		
		spectralWindowId .clear();
		for (unsigned int i = 0; i <x.spectralWindowId.length(); ++i) {
			
			spectralWindowId.push_back(Tag (x.spectralWindowId[i]));
			
		}
		
		}
		
  	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Flag");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string FlagRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(flagId, "flagId", buf);
		
		
	

  	
 		
		
		Parser::toXML(startTime, "startTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endTime, "endTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(reason, "reason", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		if (numPolarizationTypeExists) {
		
		
		Parser::toXML(numPolarizationType, "numPolarizationType", buf);
		
		
		}
		
	

  	
 		
		if (numSpectralWindowExists) {
		
		
		Parser::toXML(numSpectralWindow, "numSpectralWindow", buf);
		
		
		}
		
	

  	
 		
		if (numPairedAntennaExists) {
		
		
		Parser::toXML(numPairedAntenna, "numPairedAntenna", buf);
		
		
		}
		
	

  	
 		
		if (polarizationTypeExists) {
		
		
			buf.append(EnumerationParser::toXML("polarizationType", polarizationType));
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		if (pairedAntennaIdExists) {
		
		
		Parser::toXML(pairedAntennaId, "pairedAntennaId", buf);
		
		
		}
		
	

  	
 		
		if (spectralWindowIdExists) {
		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
		}
		
	

	
		
	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void FlagRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setFlagId(Parser::getTag("flagId","Flag",rowDoc));
			
		
	

	
  		
			
	  	setStartTime(Parser::getArrayTime("startTime","Flag",rowDoc));
			
		
	

	
  		
			
	  	setEndTime(Parser::getArrayTime("endTime","Flag",rowDoc));
			
		
	

	
  		
			
	  	setReason(Parser::getString("reason","Flag",rowDoc));
			
		
	

	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","Flag",rowDoc));
			
		
	

	
  		
        if (row.isStr("<numPolarizationType>")) {
			
	  		setNumPolarizationType(Parser::getInteger("numPolarizationType","Flag",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<numSpectralWindow>")) {
			
	  		setNumSpectralWindow(Parser::getInteger("numSpectralWindow","Flag",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<numPairedAntenna>")) {
			
	  		setNumPairedAntenna(Parser::getInteger("numPairedAntenna","Flag",rowDoc));
			
		}
 		
	

	
		
	if (row.isStr("<polarizationType>")) {
		
		
		
		polarizationType = EnumerationParser::getPolarizationType1D("polarizationType","Flag",rowDoc);			
		
		
		
		polarizationTypeExists = true;
	}
		
	

	
	
		
	
  		 
  		setAntennaId(Parser::get1DTag("antennaId","Flag",rowDoc));
		
  	

	
  		
  		if (row.isStr("<pairedAntennaId>")) {
  			setPairedAntennaId(Parser::get1DTag("pairedAntennaId","Flag",rowDoc));  		
  		}
  		
  	

	
  		
  		if (row.isStr("<spectralWindowId>")) {
  			setSpectralWindowId(Parser::get1DTag("spectralWindowId","Flag",rowDoc));  		
  		}
  		
  	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Flag");
		}
	}
	
	void FlagRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	flagId.toBin(eoss);
		
	

	
	
		
	startTime.toBin(eoss);
		
	

	
	
		
	endTime.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(reason);
				
		
	

	
	
		
						
			eoss.writeInt(numAntenna);
				
		
	

	
	
		
	Tag::toBin(antennaId, eoss);
		
	


	
	
	eoss.writeBoolean(numPolarizationTypeExists);
	if (numPolarizationTypeExists) {
	
	
	
		
						
			eoss.writeInt(numPolarizationType);
				
		
	

	}

	eoss.writeBoolean(numSpectralWindowExists);
	if (numSpectralWindowExists) {
	
	
	
		
						
			eoss.writeInt(numSpectralWindow);
				
		
	

	}

	eoss.writeBoolean(numPairedAntennaExists);
	if (numPairedAntennaExists) {
	
	
	
		
						
			eoss.writeInt(numPairedAntenna);
				
		
	

	}

	eoss.writeBoolean(polarizationTypeExists);
	if (polarizationTypeExists) {
	
	
	
		
		
			
		eoss.writeInt((int) polarizationType.size());
		for (unsigned int i = 0; i < polarizationType.size(); i++)
				
			eoss.writeInt(polarizationType.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(pairedAntennaIdExists);
	if (pairedAntennaIdExists) {
	
	
	
		
	Tag::toBin(pairedAntennaId, eoss);
		
	

	}

	eoss.writeBoolean(spectralWindowIdExists);
	if (spectralWindowIdExists) {
	
	
	
		
	Tag::toBin(spectralWindowId, eoss);
		
	

	}

	}
	
void FlagRow::flagIdFromBin(EndianISStream& eiss) {
		
	
		
		
		flagId =  Tag::fromBin(eiss);
		
	
	
}
void FlagRow::startTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		startTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void FlagRow::endTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		endTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void FlagRow::reasonFromBin(EndianISStream& eiss) {
		
	
	
		
			
		reason =  eiss.readString();
			
		
	
	
}
void FlagRow::numAntennaFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numAntenna =  eiss.readInt();
			
		
	
	
}
void FlagRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	antennaId = Tag::from1DBin(eiss);	
	

		
	
	
}

void FlagRow::numPolarizationTypeFromBin(EndianISStream& eiss) {
		
	numPolarizationTypeExists = eiss.readBoolean();
	if (numPolarizationTypeExists) {
		
	
	
		
			
		numPolarizationType =  eiss.readInt();
			
		
	

	}
	
}
void FlagRow::numSpectralWindowFromBin(EndianISStream& eiss) {
		
	numSpectralWindowExists = eiss.readBoolean();
	if (numSpectralWindowExists) {
		
	
	
		
			
		numSpectralWindow =  eiss.readInt();
			
		
	

	}
	
}
void FlagRow::numPairedAntennaFromBin(EndianISStream& eiss) {
		
	numPairedAntennaExists = eiss.readBoolean();
	if (numPairedAntennaExists) {
		
	
	
		
			
		numPairedAntenna =  eiss.readInt();
			
		
	

	}
	
}
void FlagRow::polarizationTypeFromBin(EndianISStream& eiss) {
		
	polarizationTypeExists = eiss.readBoolean();
	if (polarizationTypeExists) {
		
	
	
		
			
	
		polarizationType.clear();
		
		unsigned int polarizationTypeDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypeDim1; i++)
			
			polarizationType.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	

	}
	
}
void FlagRow::pairedAntennaIdFromBin(EndianISStream& eiss) {
		
	pairedAntennaIdExists = eiss.readBoolean();
	if (pairedAntennaIdExists) {
		
	
		
		
			
	
	pairedAntennaId = Tag::from1DBin(eiss);	
	

		
	

	}
	
}
void FlagRow::spectralWindowIdFromBin(EndianISStream& eiss) {
		
	spectralWindowIdExists = eiss.readBoolean();
	if (spectralWindowIdExists) {
		
	
		
		
			
	
	spectralWindowId = Tag::from1DBin(eiss);	
	

		
	

	}
	
}
	
	
	FlagRow* FlagRow::fromBin(EndianISStream& eiss, FlagTable& table, const vector<string>& attributesSeq) {
		FlagRow* row = new  FlagRow(table);
		
		map<string, FlagAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "FlagTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get flagId.
 	 * @return flagId as Tag
 	 */
 	Tag FlagRow::getFlagId() const {
	
  		return flagId;
 	}

 	/**
 	 * Set flagId with the specified Tag.
 	 * @param flagId The Tag value to which flagId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FlagRow::setFlagId (Tag flagId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("flagId", "Flag");
		
  		}
  	
 		this->flagId = flagId;
	
 	}
	
	

	

	
 	/**
 	 * Get startTime.
 	 * @return startTime as ArrayTime
 	 */
 	ArrayTime FlagRow::getStartTime() const {
	
  		return startTime;
 	}

 	/**
 	 * Set startTime with the specified ArrayTime.
 	 * @param startTime The ArrayTime value to which startTime is to be set.
 	 
 	
 		
 	 */
 	void FlagRow::setStartTime (ArrayTime startTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startTime = startTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endTime.
 	 * @return endTime as ArrayTime
 	 */
 	ArrayTime FlagRow::getEndTime() const {
	
  		return endTime;
 	}

 	/**
 	 * Set endTime with the specified ArrayTime.
 	 * @param endTime The ArrayTime value to which endTime is to be set.
 	 
 	
 		
 	 */
 	void FlagRow::setEndTime (ArrayTime endTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endTime = endTime;
	
 	}
	
	

	

	
 	/**
 	 * Get reason.
 	 * @return reason as string
 	 */
 	string FlagRow::getReason() const {
	
  		return reason;
 	}

 	/**
 	 * Set reason with the specified string.
 	 * @param reason The string value to which reason is to be set.
 	 
 	
 		
 	 */
 	void FlagRow::setReason (string reason)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->reason = reason;
	
 	}
	
	

	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int FlagRow::getNumAntenna() const {
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 		
 	 */
 	void FlagRow::setNumAntenna (int numAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAntenna = numAntenna;
	
 	}
	
	

	
	/**
	 * The attribute numPolarizationType is optional. Return true if this attribute exists.
	 * @return true if and only if the numPolarizationType attribute exists. 
	 */
	bool FlagRow::isNumPolarizationTypeExists() const {
		return numPolarizationTypeExists;
	}
	

	
 	/**
 	 * Get numPolarizationType, which is optional.
 	 * @return numPolarizationType as int
 	 * @throw IllegalAccessException If numPolarizationType does not exist.
 	 */
 	int FlagRow::getNumPolarizationType() const  {
		if (!numPolarizationTypeExists) {
			throw IllegalAccessException("numPolarizationType", "Flag");
		}
	
  		return numPolarizationType;
 	}

 	/**
 	 * Set numPolarizationType with the specified int.
 	 * @param numPolarizationType The int value to which numPolarizationType is to be set.
 	 
 	
 	 */
 	void FlagRow::setNumPolarizationType (int numPolarizationType) {
	
 		this->numPolarizationType = numPolarizationType;
	
		numPolarizationTypeExists = true;
	
 	}
	
	
	/**
	 * Mark numPolarizationType, which is an optional field, as non-existent.
	 */
	void FlagRow::clearNumPolarizationType () {
		numPolarizationTypeExists = false;
	}
	

	
	/**
	 * The attribute numSpectralWindow is optional. Return true if this attribute exists.
	 * @return true if and only if the numSpectralWindow attribute exists. 
	 */
	bool FlagRow::isNumSpectralWindowExists() const {
		return numSpectralWindowExists;
	}
	

	
 	/**
 	 * Get numSpectralWindow, which is optional.
 	 * @return numSpectralWindow as int
 	 * @throw IllegalAccessException If numSpectralWindow does not exist.
 	 */
 	int FlagRow::getNumSpectralWindow() const  {
		if (!numSpectralWindowExists) {
			throw IllegalAccessException("numSpectralWindow", "Flag");
		}
	
  		return numSpectralWindow;
 	}

 	/**
 	 * Set numSpectralWindow with the specified int.
 	 * @param numSpectralWindow The int value to which numSpectralWindow is to be set.
 	 
 	
 	 */
 	void FlagRow::setNumSpectralWindow (int numSpectralWindow) {
	
 		this->numSpectralWindow = numSpectralWindow;
	
		numSpectralWindowExists = true;
	
 	}
	
	
	/**
	 * Mark numSpectralWindow, which is an optional field, as non-existent.
	 */
	void FlagRow::clearNumSpectralWindow () {
		numSpectralWindowExists = false;
	}
	

	
	/**
	 * The attribute numPairedAntenna is optional. Return true if this attribute exists.
	 * @return true if and only if the numPairedAntenna attribute exists. 
	 */
	bool FlagRow::isNumPairedAntennaExists() const {
		return numPairedAntennaExists;
	}
	

	
 	/**
 	 * Get numPairedAntenna, which is optional.
 	 * @return numPairedAntenna as int
 	 * @throw IllegalAccessException If numPairedAntenna does not exist.
 	 */
 	int FlagRow::getNumPairedAntenna() const  {
		if (!numPairedAntennaExists) {
			throw IllegalAccessException("numPairedAntenna", "Flag");
		}
	
  		return numPairedAntenna;
 	}

 	/**
 	 * Set numPairedAntenna with the specified int.
 	 * @param numPairedAntenna The int value to which numPairedAntenna is to be set.
 	 
 	
 	 */
 	void FlagRow::setNumPairedAntenna (int numPairedAntenna) {
	
 		this->numPairedAntenna = numPairedAntenna;
	
		numPairedAntennaExists = true;
	
 	}
	
	
	/**
	 * Mark numPairedAntenna, which is an optional field, as non-existent.
	 */
	void FlagRow::clearNumPairedAntenna () {
		numPairedAntennaExists = false;
	}
	

	
	/**
	 * The attribute polarizationType is optional. Return true if this attribute exists.
	 * @return true if and only if the polarizationType attribute exists. 
	 */
	bool FlagRow::isPolarizationTypeExists() const {
		return polarizationTypeExists;
	}
	

	
 	/**
 	 * Get polarizationType, which is optional.
 	 * @return polarizationType as vector<PolarizationTypeMod::PolarizationType >
 	 * @throw IllegalAccessException If polarizationType does not exist.
 	 */
 	vector<PolarizationTypeMod::PolarizationType > FlagRow::getPolarizationType() const  {
		if (!polarizationTypeExists) {
			throw IllegalAccessException("polarizationType", "Flag");
		}
	
  		return polarizationType;
 	}

 	/**
 	 * Set polarizationType with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationType The vector<PolarizationTypeMod::PolarizationType > value to which polarizationType is to be set.
 	 
 	
 	 */
 	void FlagRow::setPolarizationType (vector<PolarizationTypeMod::PolarizationType > polarizationType) {
	
 		this->polarizationType = polarizationType;
	
		polarizationTypeExists = true;
	
 	}
	
	
	/**
	 * Mark polarizationType, which is an optional field, as non-existent.
	 */
	void FlagRow::clearPolarizationType () {
		polarizationTypeExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as vector<Tag> 
 	 */
 	vector<Tag>  FlagRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified vector<Tag> .
 	 * @param antennaId The vector<Tag>  value to which antennaId is to be set.
 	 
 	
 		
 	 */
 	void FlagRow::setAntennaId (vector<Tag>  antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	
	/**
	 * The attribute pairedAntennaId is optional. Return true if this attribute exists.
	 * @return true if and only if the pairedAntennaId attribute exists. 
	 */
	bool FlagRow::isPairedAntennaIdExists() const {
		return pairedAntennaIdExists;
	}
	

	
 	/**
 	 * Get pairedAntennaId, which is optional.
 	 * @return pairedAntennaId as vector<Tag> 
 	 * @throw IllegalAccessException If pairedAntennaId does not exist.
 	 */
 	vector<Tag>  FlagRow::getPairedAntennaId() const  {
		if (!pairedAntennaIdExists) {
			throw IllegalAccessException("pairedAntennaId", "Flag");
		}
	
  		return pairedAntennaId;
 	}

 	/**
 	 * Set pairedAntennaId with the specified vector<Tag> .
 	 * @param pairedAntennaId The vector<Tag>  value to which pairedAntennaId is to be set.
 	 
 	
 	 */
 	void FlagRow::setPairedAntennaId (vector<Tag>  pairedAntennaId) {
	
 		this->pairedAntennaId = pairedAntennaId;
	
		pairedAntennaIdExists = true;
	
 	}
	
	
	/**
	 * Mark pairedAntennaId, which is an optional field, as non-existent.
	 */
	void FlagRow::clearPairedAntennaId () {
		pairedAntennaIdExists = false;
	}
	

	
	/**
	 * The attribute spectralWindowId is optional. Return true if this attribute exists.
	 * @return true if and only if the spectralWindowId attribute exists. 
	 */
	bool FlagRow::isSpectralWindowIdExists() const {
		return spectralWindowIdExists;
	}
	

	
 	/**
 	 * Get spectralWindowId, which is optional.
 	 * @return spectralWindowId as vector<Tag> 
 	 * @throw IllegalAccessException If spectralWindowId does not exist.
 	 */
 	vector<Tag>  FlagRow::getSpectralWindowId() const  {
		if (!spectralWindowIdExists) {
			throw IllegalAccessException("spectralWindowId", "Flag");
		}
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified vector<Tag> .
 	 * @param spectralWindowId The vector<Tag>  value to which spectralWindowId is to be set.
 	 
 	
 	 */
 	void FlagRow::setSpectralWindowId (vector<Tag>  spectralWindowId) {
	
 		this->spectralWindowId = spectralWindowId;
	
		spectralWindowIdExists = true;
	
 	}
	
	
	/**
	 * Mark spectralWindowId, which is an optional field, as non-existent.
	 */
	void FlagRow::clearSpectralWindowId () {
		spectralWindowIdExists = false;
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
  	void FlagRow::setAntennaId (int i, Tag antennaId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->antennaId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute antennaId in table FlagTable");
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
 void FlagRow::addAntennaId(Tag id){
 	antennaId.push_back(id);
}

/**
 * Append an array of Tag to antennaId.
 * @param id an array of Tag to be appended to antennaId
 */
 void FlagRow::addAntennaId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		antennaId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in antennaId at position i.
  *
  */
 const Tag FlagRow::getAntennaId(int i) {
 	return antennaId.at(i);
 }
 
 /**
  * Returns the AntennaRow linked to this row via the Tag stored in antennaId
  * at position i.
  */
 AntennaRow* FlagRow::getAntennaUsingAntennaId(int i) {
 	return table.getContainer().getAntenna().getRowByKey(antennaId.at(i));
 } 
 
 /**
  * Returns the vector of AntennaRow* linked to this row via the Tags stored in antennaId
  *
  */
 vector<AntennaRow *> FlagRow::getAntennasUsingAntennaId() {
 	vector<AntennaRow *> result;
 	for (unsigned int i = 0; i < antennaId.size(); i++)
 		result.push_back(table.getContainer().getAntenna().getRowByKey(antennaId.at(i)));
 		
 	return result;
 }
  

	

	
 		
 	/**
 	 * Set pairedAntennaId[i] with the specified Tag.
 	 * @param i The index in pairedAntennaId where to set the Tag value.
 	 * @param pairedAntennaId The Tag value to which pairedAntennaId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void FlagRow::setPairedAntennaId (int i, Tag pairedAntennaId) {
  		if ((i < 0) || (i > ((int) this->pairedAntennaId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute pairedAntennaId in table FlagTable");
  		vector<Tag> ::iterator iter = this->pairedAntennaId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->pairedAntennaId.insert(this->pairedAntennaId.erase(iter), pairedAntennaId); 	
  	}
 			
	
	
	
		
/**
 * Append a Tag to pairedAntennaId.
 * @param id the Tag to be appended to pairedAntennaId
 */
 void FlagRow::addPairedAntennaId(Tag id){
 	pairedAntennaId.push_back(id);
}

/**
 * Append an array of Tag to pairedAntennaId.
 * @param id an array of Tag to be appended to pairedAntennaId
 */
 void FlagRow::addPairedAntennaId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		pairedAntennaId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in pairedAntennaId at position i.
  *
  */
 const Tag FlagRow::getPairedAntennaId(int i) {
 	return pairedAntennaId.at(i);
 }
 
 /**
  * Returns the AntennaRow linked to this row via the Tag stored in pairedAntennaId
  * at position i.
  */
 AntennaRow* FlagRow::getAntennaUsingPairedAntennaId(int i) {
 	return table.getContainer().getAntenna().getRowByKey(pairedAntennaId.at(i));
 } 
 
 /**
  * Returns the vector of AntennaRow* linked to this row via the Tags stored in pairedAntennaId
  *
  */
 vector<AntennaRow *> FlagRow::getAntennasUsingPairedAntennaId() {
 	vector<AntennaRow *> result;
 	for (unsigned int i = 0; i < pairedAntennaId.size(); i++)
 		result.push_back(table.getContainer().getAntenna().getRowByKey(pairedAntennaId.at(i)));
 		
 	return result;
 }
  

	

	
 		
 	/**
 	 * Set spectralWindowId[i] with the specified Tag.
 	 * @param i The index in spectralWindowId where to set the Tag value.
 	 * @param spectralWindowId The Tag value to which spectralWindowId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void FlagRow::setSpectralWindowId (int i, Tag spectralWindowId) {
  		if ((i < 0) || (i > ((int) this->spectralWindowId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute spectralWindowId in table FlagTable");
  		vector<Tag> ::iterator iter = this->spectralWindowId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->spectralWindowId.insert(this->spectralWindowId.erase(iter), spectralWindowId); 	
  	}
 			
	
	
	
		
/**
 * Append a Tag to spectralWindowId.
 * @param id the Tag to be appended to spectralWindowId
 */
 void FlagRow::addSpectralWindowId(Tag id){
 	spectralWindowId.push_back(id);
}

/**
 * Append an array of Tag to spectralWindowId.
 * @param id an array of Tag to be appended to spectralWindowId
 */
 void FlagRow::addSpectralWindowId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		spectralWindowId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in spectralWindowId at position i.
  *
  */
 const Tag FlagRow::getSpectralWindowId(int i) {
 	return spectralWindowId.at(i);
 }
 
 /**
  * Returns the SpectralWindowRow linked to this row via the Tag stored in spectralWindowId
  * at position i.
  */
 SpectralWindowRow* FlagRow::getSpectralWindowUsingSpectralWindowId(int i) {
 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId.at(i));
 } 
 
 /**
  * Returns the vector of SpectralWindowRow* linked to this row via the Tags stored in spectralWindowId
  *
  */
 vector<SpectralWindowRow *> FlagRow::getSpectralWindowsUsingSpectralWindowId() {
 	vector<SpectralWindowRow *> result;
 	for (unsigned int i = 0; i < spectralWindowId.size(); i++)
 		result.push_back(table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId.at(i)));
 		
 	return result;
 }
  

	

	
	/**
	 * Create a FlagRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FlagRow::FlagRow (FlagTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	
		numPolarizationTypeExists = false;
	

	
		numSpectralWindowExists = false;
	

	
		numPairedAntennaExists = false;
	

	
		polarizationTypeExists = false;
	

	
	

	
		pairedAntennaIdExists = false;
	

	
		spectralWindowIdExists = false;
	

	
	
	
	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["flagId"] = &FlagRow::flagIdFromBin; 
	 fromBinMethods["startTime"] = &FlagRow::startTimeFromBin; 
	 fromBinMethods["endTime"] = &FlagRow::endTimeFromBin; 
	 fromBinMethods["reason"] = &FlagRow::reasonFromBin; 
	 fromBinMethods["numAntenna"] = &FlagRow::numAntennaFromBin; 
	 fromBinMethods["antennaId"] = &FlagRow::antennaIdFromBin; 
		
	
	 fromBinMethods["numPolarizationType"] = &FlagRow::numPolarizationTypeFromBin; 
	 fromBinMethods["numSpectralWindow"] = &FlagRow::numSpectralWindowFromBin; 
	 fromBinMethods["numPairedAntenna"] = &FlagRow::numPairedAntennaFromBin; 
	 fromBinMethods["polarizationType"] = &FlagRow::polarizationTypeFromBin; 
	 fromBinMethods["pairedAntennaId"] = &FlagRow::pairedAntennaIdFromBin; 
	 fromBinMethods["spectralWindowId"] = &FlagRow::spectralWindowIdFromBin; 
	
	}
	
	FlagRow::FlagRow (FlagTable &t, FlagRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	
		numPolarizationTypeExists = false;
	

	
		numSpectralWindowExists = false;
	

	
		numPairedAntennaExists = false;
	

	
		polarizationTypeExists = false;
	

	
	

	
		pairedAntennaIdExists = false;
	

	
		spectralWindowIdExists = false;
	
		
		}
		else {
	
		
			flagId = row.flagId;
		
		
		
		
			startTime = row.startTime;
		
			endTime = row.endTime;
		
			reason = row.reason;
		
			numAntenna = row.numAntenna;
		
			antennaId = row.antennaId;
		
		
		
		
		if (row.numPolarizationTypeExists) {
			numPolarizationType = row.numPolarizationType;		
			numPolarizationTypeExists = true;
		}
		else
			numPolarizationTypeExists = false;
		
		if (row.numSpectralWindowExists) {
			numSpectralWindow = row.numSpectralWindow;		
			numSpectralWindowExists = true;
		}
		else
			numSpectralWindowExists = false;
		
		if (row.numPairedAntennaExists) {
			numPairedAntenna = row.numPairedAntenna;		
			numPairedAntennaExists = true;
		}
		else
			numPairedAntennaExists = false;
		
		if (row.polarizationTypeExists) {
			polarizationType = row.polarizationType;		
			polarizationTypeExists = true;
		}
		else
			polarizationTypeExists = false;
		
		if (row.pairedAntennaIdExists) {
			pairedAntennaId = row.pairedAntennaId;		
			pairedAntennaIdExists = true;
		}
		else
			pairedAntennaIdExists = false;
		
		if (row.spectralWindowIdExists) {
			spectralWindowId = row.spectralWindowId;		
			spectralWindowIdExists = true;
		}
		else
			spectralWindowIdExists = false;
		
		}
		
		 fromBinMethods["flagId"] = &FlagRow::flagIdFromBin; 
		 fromBinMethods["startTime"] = &FlagRow::startTimeFromBin; 
		 fromBinMethods["endTime"] = &FlagRow::endTimeFromBin; 
		 fromBinMethods["reason"] = &FlagRow::reasonFromBin; 
		 fromBinMethods["numAntenna"] = &FlagRow::numAntennaFromBin; 
		 fromBinMethods["antennaId"] = &FlagRow::antennaIdFromBin; 
			
	
		 fromBinMethods["numPolarizationType"] = &FlagRow::numPolarizationTypeFromBin; 
		 fromBinMethods["numSpectralWindow"] = &FlagRow::numSpectralWindowFromBin; 
		 fromBinMethods["numPairedAntenna"] = &FlagRow::numPairedAntennaFromBin; 
		 fromBinMethods["polarizationType"] = &FlagRow::polarizationTypeFromBin; 
		 fromBinMethods["pairedAntennaId"] = &FlagRow::pairedAntennaIdFromBin; 
		 fromBinMethods["spectralWindowId"] = &FlagRow::spectralWindowIdFromBin; 
			
	}

	
	bool FlagRow::compareNoAutoInc(ArrayTime startTime, ArrayTime endTime, string reason, int numAntenna, vector<Tag>  antennaId) {
		bool result;
		result = true;
		
	
		
		result = result && (this->startTime == startTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endTime == endTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->reason == reason);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool FlagRow::compareRequiredValue(ArrayTime startTime, ArrayTime endTime, string reason, int numAntenna, vector<Tag>  antennaId) {
		bool result;
		result = true;
		
	
		if (!(this->startTime == startTime)) return false;
	

	
		if (!(this->endTime == endTime)) return false;
	

	
		if (!(this->reason == reason)) return false;
	

	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->antennaId == antennaId)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FlagRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool FlagRow::equalByRequiredValue(FlagRow* x) {
		
			
		if (this->startTime != x->startTime) return false;
			
		if (this->endTime != x->endTime) return false;
			
		if (this->reason != x->reason) return false;
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->antennaId != x->antennaId) return false;
			
		
		return true;
	}	
	
/*
	 map<string, FlagAttributeFromBin> FlagRow::initFromBinMethods() {
		map<string, FlagAttributeFromBin> result;
		
		result["flagId"] = &FlagRow::flagIdFromBin;
		result["startTime"] = &FlagRow::startTimeFromBin;
		result["endTime"] = &FlagRow::endTimeFromBin;
		result["reason"] = &FlagRow::reasonFromBin;
		result["numAntenna"] = &FlagRow::numAntennaFromBin;
		result["antennaId"] = &FlagRow::antennaIdFromBin;
		
		
		result["numPolarizationType"] = &FlagRow::numPolarizationTypeFromBin;
		result["numSpectralWindow"] = &FlagRow::numSpectralWindowFromBin;
		result["numPairedAntenna"] = &FlagRow::numPairedAntennaFromBin;
		result["polarizationType"] = &FlagRow::polarizationTypeFromBin;
		result["pairedAntennaId"] = &FlagRow::pairedAntennaIdFromBin;
		result["spectralWindowId"] = &FlagRow::spectralWindowIdFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
