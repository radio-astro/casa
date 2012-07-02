
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
 * File FocusRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <FocusRow.h>
#include <FocusTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <FocusModelTable.h>
#include <FocusModelRow.h>
	

using asdm::ASDM;
using asdm::FocusRow;
using asdm::FocusTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::FocusModelTable;
using asdm::FocusModelRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	FocusRow::~FocusRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	FocusTable &FocusRow::getTable() const {
		return table;
	}
	
	void FocusRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FocusRowIDL struct.
	 */
	FocusRowIDL *FocusRow::toIDL() const {
		FocusRowIDL *x = new FocusRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->focusTracking = focusTracking;
 				
 			
		
	

	
  		
		
		
			
		x->focusOffset.length(focusOffset.size());
		for (unsigned int i = 0; i < focusOffset.size(); ++i) {
			
			x->focusOffset[i] = focusOffset.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		x->measuredFocusPositionExists = measuredFocusPositionExists;
		
		
			
		x->measuredFocusPosition.length(measuredFocusPosition.size());
		for (unsigned int i = 0; i < measuredFocusPosition.size(); ++i) {
			
			x->measuredFocusPosition[i] = measuredFocusPosition.at(i).toIDLLength();
			
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
				
		x->focusModelId = focusModelId;
 				
 			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FocusRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void FocusRow::setFromIDL (FocusRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setFocusTracking(x.focusTracking);
  			
 		
		
	

	
		
		
			
		focusOffset .clear();
		for (unsigned int i = 0; i <x.focusOffset.length(); ++i) {
			
			focusOffset.push_back(Length (x.focusOffset[i]));
			
		}
			
  		
		
	

	
		
		measuredFocusPositionExists = x.measuredFocusPositionExists;
		if (x.measuredFocusPositionExists) {
		
		
			
		measuredFocusPosition .clear();
		for (unsigned int i = 0; i <x.measuredFocusPosition.length(); ++i) {
			
			measuredFocusPosition.push_back(Length (x.measuredFocusPosition[i]));
			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setFocusModelId(x.focusModelId);
  			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Focus");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string FocusRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusTracking, "focusTracking", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusOffset, "focusOffset", buf);
		
		
	

  	
 		
		if (measuredFocusPositionExists) {
		
		
		Parser::toXML(measuredFocusPosition, "measuredFocusPosition", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusModelId, "focusModelId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void FocusRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Focus",rowDoc));
			
		
	

	
  		
			
	  	setFocusTracking(Parser::getBoolean("focusTracking","Focus",rowDoc));
			
		
	

	
  		
			
					
	  	setFocusOffset(Parser::get1DLength("focusOffset","Focus",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<measuredFocusPosition>")) {
			
								
	  		setMeasuredFocusPosition(Parser::get1DLength("measuredFocusPosition","Focus",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFocusModelId(Parser::getInteger("focusModelId","Focus",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Focus");
		}
	}
	
	void FocusRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeBoolean(focusTracking);
				
		
	

	
	
		
	Length::toBin(focusOffset, eoss);
		
	

	
	
		
						
			eoss.writeInt(focusModelId);
				
		
	


	
	
	eoss.writeBoolean(measuredFocusPositionExists);
	if (measuredFocusPositionExists) {
	
	
	
		
	Length::toBin(measuredFocusPosition, eoss);
		
	

	}

	}
	
void FocusRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void FocusRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void FocusRow::focusTrackingFromBin(EndianISStream& eiss) {
		
	
	
		
			
		focusTracking =  eiss.readBoolean();
			
		
	
	
}
void FocusRow::focusOffsetFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	focusOffset = Length::from1DBin(eiss);	
	

		
	
	
}
void FocusRow::focusModelIdFromBin(EndianISStream& eiss) {
		
	
	
		
			
		focusModelId =  eiss.readInt();
			
		
	
	
}

void FocusRow::measuredFocusPositionFromBin(EndianISStream& eiss) {
		
	measuredFocusPositionExists = eiss.readBoolean();
	if (measuredFocusPositionExists) {
		
	
		
		
			
	
	measuredFocusPosition = Length::from1DBin(eiss);	
	

		
	

	}
	
}
	
	
	FocusRow* FocusRow::fromBin(EndianISStream& eiss, FocusTable& table, const vector<string>& attributesSeq) {
		FocusRow* row = new  FocusRow(table);
		
		map<string, FocusAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "FocusTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval FocusRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FocusRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "Focus");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get focusTracking.
 	 * @return focusTracking as bool
 	 */
 	bool FocusRow::getFocusTracking() const {
	
  		return focusTracking;
 	}

 	/**
 	 * Set focusTracking with the specified bool.
 	 * @param focusTracking The bool value to which focusTracking is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setFocusTracking (bool focusTracking)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusTracking = focusTracking;
	
 	}
	
	

	

	
 	/**
 	 * Get focusOffset.
 	 * @return focusOffset as vector<Length >
 	 */
 	vector<Length > FocusRow::getFocusOffset() const {
	
  		return focusOffset;
 	}

 	/**
 	 * Set focusOffset with the specified vector<Length >.
 	 * @param focusOffset The vector<Length > value to which focusOffset is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setFocusOffset (vector<Length > focusOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusOffset = focusOffset;
	
 	}
	
	

	
	/**
	 * The attribute measuredFocusPosition is optional. Return true if this attribute exists.
	 * @return true if and only if the measuredFocusPosition attribute exists. 
	 */
	bool FocusRow::isMeasuredFocusPositionExists() const {
		return measuredFocusPositionExists;
	}
	

	
 	/**
 	 * Get measuredFocusPosition, which is optional.
 	 * @return measuredFocusPosition as vector<Length >
 	 * @throw IllegalAccessException If measuredFocusPosition does not exist.
 	 */
 	vector<Length > FocusRow::getMeasuredFocusPosition() const  {
		if (!measuredFocusPositionExists) {
			throw IllegalAccessException("measuredFocusPosition", "Focus");
		}
	
  		return measuredFocusPosition;
 	}

 	/**
 	 * Set measuredFocusPosition with the specified vector<Length >.
 	 * @param measuredFocusPosition The vector<Length > value to which measuredFocusPosition is to be set.
 	 
 	
 	 */
 	void FocusRow::setMeasuredFocusPosition (vector<Length > measuredFocusPosition) {
	
 		this->measuredFocusPosition = measuredFocusPosition;
	
		measuredFocusPositionExists = true;
	
 	}
	
	
	/**
	 * Mark measuredFocusPosition, which is an optional field, as non-existent.
	 */
	void FocusRow::clearMeasuredFocusPosition () {
		measuredFocusPositionExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag FocusRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FocusRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "Focus");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get focusModelId.
 	 * @return focusModelId as int
 	 */
 	int FocusRow::getFocusModelId() const {
	
  		return focusModelId;
 	}

 	/**
 	 * Set focusModelId with the specified int.
 	 * @param focusModelId The int value to which focusModelId is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setFocusModelId (int focusModelId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusModelId = focusModelId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* FocusRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of Focus table to a collection of row of FocusModel table.
	
	/**
	 * Get the collection of row in the FocusModel table having their attribut focusModelId == this->focusModelId
	 */
	vector <FocusModelRow *> FocusRow::getFocusModels() {
		
			return table.getContainer().getFocusModel().getRowByFocusModelId(focusModelId);
		
	}
	

	

	
	/**
	 * Create a FocusRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FocusRow::FocusRow (FocusTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	
		measuredFocusPositionExists = false;
	

	
	

	

	
	
	
	

	

	

	

	
	
	 fromBinMethods["antennaId"] = &FocusRow::antennaIdFromBin; 
	 fromBinMethods["timeInterval"] = &FocusRow::timeIntervalFromBin; 
	 fromBinMethods["focusTracking"] = &FocusRow::focusTrackingFromBin; 
	 fromBinMethods["focusOffset"] = &FocusRow::focusOffsetFromBin; 
	 fromBinMethods["focusModelId"] = &FocusRow::focusModelIdFromBin; 
		
	
	 fromBinMethods["measuredFocusPosition"] = &FocusRow::measuredFocusPositionFromBin; 
	
	}
	
	FocusRow::FocusRow (FocusTable &t, FocusRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	
		measuredFocusPositionExists = false;
	

	
	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			focusTracking = row.focusTracking;
		
			focusOffset = row.focusOffset;
		
			focusModelId = row.focusModelId;
		
		
		
		
		if (row.measuredFocusPositionExists) {
			measuredFocusPosition = row.measuredFocusPosition;		
			measuredFocusPositionExists = true;
		}
		else
			measuredFocusPositionExists = false;
		
		}
		
		 fromBinMethods["antennaId"] = &FocusRow::antennaIdFromBin; 
		 fromBinMethods["timeInterval"] = &FocusRow::timeIntervalFromBin; 
		 fromBinMethods["focusTracking"] = &FocusRow::focusTrackingFromBin; 
		 fromBinMethods["focusOffset"] = &FocusRow::focusOffsetFromBin; 
		 fromBinMethods["focusModelId"] = &FocusRow::focusModelIdFromBin; 
			
	
		 fromBinMethods["measuredFocusPosition"] = &FocusRow::measuredFocusPositionFromBin; 
			
	}

	
	bool FocusRow::compareNoAutoInc(Tag antennaId, ArrayTimeInterval timeInterval, bool focusTracking, vector<Length > focusOffset, int focusModelId) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->focusTracking == focusTracking);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusOffset == focusOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusModelId == focusModelId);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool FocusRow::compareRequiredValue(bool focusTracking, vector<Length > focusOffset, int focusModelId) {
		bool result;
		result = true;
		
	
		if (!(this->focusTracking == focusTracking)) return false;
	

	
		if (!(this->focusOffset == focusOffset)) return false;
	

	
		if (!(this->focusModelId == focusModelId)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FocusRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool FocusRow::equalByRequiredValue(FocusRow* x) {
		
			
		if (this->focusTracking != x->focusTracking) return false;
			
		if (this->focusOffset != x->focusOffset) return false;
			
		if (this->focusModelId != x->focusModelId) return false;
			
		
		return true;
	}	
	
/*
	 map<string, FocusAttributeFromBin> FocusRow::initFromBinMethods() {
		map<string, FocusAttributeFromBin> result;
		
		result["antennaId"] = &FocusRow::antennaIdFromBin;
		result["timeInterval"] = &FocusRow::timeIntervalFromBin;
		result["focusTracking"] = &FocusRow::focusTrackingFromBin;
		result["focusOffset"] = &FocusRow::focusOffsetFromBin;
		result["focusModelId"] = &FocusRow::focusModelIdFromBin;
		
		
		result["measuredFocusPosition"] = &FocusRow::measuredFocusPositionFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
