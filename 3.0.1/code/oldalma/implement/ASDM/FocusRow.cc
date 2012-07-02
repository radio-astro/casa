
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

#include <FocusModelTable.h>
#include <FocusModelRow.h>

#include <FeedTable.h>
#include <FeedRow.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::FocusRow;
using asdm::FocusTable;

using asdm::FocusModelTable;
using asdm::FocusModelRow;

using asdm::FeedTable;
using asdm::FeedRow;

using asdm::AntennaTable;
using asdm::AntennaRow;


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
			
		
	

	
  		
		
		
			
		x->xFocusPosition = xFocusPosition.toIDLLength();
			
		
	

	
  		
		
		
			
		x->yFocusPosition = yFocusPosition.toIDLLength();
			
		
	

	
  		
		
		
			
		x->zFocusPosition = zFocusPosition.toIDLLength();
			
		
	

	
  		
		
		
			
				
		x->focusTracking = focusTracking;
 				
 			
		
	

	
  		
		
		
			
		x->xFocusOffset = xFocusOffset.toIDLLength();
			
		
	

	
  		
		
		
			
		x->yFocusOffset = yFocusOffset.toIDLLength();
			
		
	

	
  		
		
		
			
		x->zFocusOffset = zFocusOffset.toIDLLength();
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
				
		x->feedId = feedId;
 				
 			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->focusModelId = focusModelId.toIDLTag();
			
	 	 		
  	

	
		
	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FocusRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void FocusRow::setFromIDL (FocusRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setXFocusPosition(Length (x.xFocusPosition));
			
 		
		
	

	
		
		
			
		setYFocusPosition(Length (x.yFocusPosition));
			
 		
		
	

	
		
		
			
		setZFocusPosition(Length (x.zFocusPosition));
			
 		
		
	

	
		
		
			
		setFocusTracking(x.focusTracking);
  			
 		
		
	

	
		
		
			
		setXFocusOffset(Length (x.xFocusOffset));
			
 		
		
	

	
		
		
			
		setYFocusOffset(Length (x.yFocusOffset));
			
 		
		
	

	
		
		
			
		setZFocusOffset(Length (x.zFocusOffset));
			
 		
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setFocusModelId(Tag (x.focusModelId));
			
 		
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Focus");
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
		
		
	

  	
 		
		
		Parser::toXML(xFocusPosition, "xFocusPosition", buf);
		
		
	

  	
 		
		
		Parser::toXML(yFocusPosition, "yFocusPosition", buf);
		
		
	

  	
 		
		
		Parser::toXML(zFocusPosition, "zFocusPosition", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusTracking, "focusTracking", buf);
		
		
	

  	
 		
		
		Parser::toXML(xFocusOffset, "xFocusOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(yFocusOffset, "yFocusOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(zFocusOffset, "zFocusOffset", buf);
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(feedId, "feedId", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusModelId, "focusModelId", buf);
		
		
	

	
		
	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void FocusRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Focus",rowDoc));
			
		
	

	
  		
			
	  	setXFocusPosition(Parser::getLength("xFocusPosition","Focus",rowDoc));
			
		
	

	
  		
			
	  	setYFocusPosition(Parser::getLength("yFocusPosition","Focus",rowDoc));
			
		
	

	
  		
			
	  	setZFocusPosition(Parser::getLength("zFocusPosition","Focus",rowDoc));
			
		
	

	
  		
			
	  	setFocusTracking(Parser::getBoolean("focusTracking","Focus",rowDoc));
			
		
	

	
  		
			
	  	setXFocusOffset(Parser::getLength("xFocusOffset","Focus",rowDoc));
			
		
	

	
  		
			
	  	setYFocusOffset(Parser::getLength("yFocusOffset","Focus",rowDoc));
			
		
	

	
  		
			
	  	setZFocusOffset(Parser::getLength("zFocusOffset","Focus",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setFocusModelId(Parser::getTag("focusModelId","Focus",rowDoc));
			
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Focus");
		}
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
 	 * Get xFocusPosition.
 	 * @return xFocusPosition as Length
 	 */
 	Length FocusRow::getXFocusPosition() const {
	
  		return xFocusPosition;
 	}

 	/**
 	 * Set xFocusPosition with the specified Length.
 	 * @param xFocusPosition The Length value to which xFocusPosition is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setXFocusPosition (Length xFocusPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->xFocusPosition = xFocusPosition;
	
 	}
	
	

	

	
 	/**
 	 * Get yFocusPosition.
 	 * @return yFocusPosition as Length
 	 */
 	Length FocusRow::getYFocusPosition() const {
	
  		return yFocusPosition;
 	}

 	/**
 	 * Set yFocusPosition with the specified Length.
 	 * @param yFocusPosition The Length value to which yFocusPosition is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setYFocusPosition (Length yFocusPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->yFocusPosition = yFocusPosition;
	
 	}
	
	

	

	
 	/**
 	 * Get zFocusPosition.
 	 * @return zFocusPosition as Length
 	 */
 	Length FocusRow::getZFocusPosition() const {
	
  		return zFocusPosition;
 	}

 	/**
 	 * Set zFocusPosition with the specified Length.
 	 * @param zFocusPosition The Length value to which zFocusPosition is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setZFocusPosition (Length zFocusPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->zFocusPosition = zFocusPosition;
	
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
 	 * Get xFocusOffset.
 	 * @return xFocusOffset as Length
 	 */
 	Length FocusRow::getXFocusOffset() const {
	
  		return xFocusOffset;
 	}

 	/**
 	 * Set xFocusOffset with the specified Length.
 	 * @param xFocusOffset The Length value to which xFocusOffset is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setXFocusOffset (Length xFocusOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->xFocusOffset = xFocusOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get yFocusOffset.
 	 * @return yFocusOffset as Length
 	 */
 	Length FocusRow::getYFocusOffset() const {
	
  		return yFocusOffset;
 	}

 	/**
 	 * Set yFocusOffset with the specified Length.
 	 * @param yFocusOffset The Length value to which yFocusOffset is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setYFocusOffset (Length yFocusOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->yFocusOffset = yFocusOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get zFocusOffset.
 	 * @return zFocusOffset as Length
 	 */
 	Length FocusRow::getZFocusOffset() const {
	
  		return zFocusOffset;
 	}

 	/**
 	 * Set zFocusOffset with the specified Length.
 	 * @param zFocusOffset The Length value to which zFocusOffset is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setZFocusOffset (Length zFocusOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->zFocusOffset = zFocusOffset;
	
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
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int FocusRow::getFeedId() const {
	
  		return feedId;
 	}

 	/**
 	 * Set feedId with the specified int.
 	 * @param feedId The int value to which feedId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FocusRow::setFeedId (int feedId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("feedId", "Focus");
		
  		}
  	
 		this->feedId = feedId;
	
 	}
	
	

	

	
 	/**
 	 * Get focusModelId.
 	 * @return focusModelId as Tag
 	 */
 	Tag FocusRow::getFocusModelId() const {
	
  		return focusModelId;
 	}

 	/**
 	 * Set focusModelId with the specified Tag.
 	 * @param focusModelId The Tag value to which focusModelId is to be set.
 	 
 	
 		
 	 */
 	void FocusRow::setFocusModelId (Tag focusModelId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusModelId = focusModelId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the FocusModel table having FocusModel.focusModelId == focusModelId
	 * @return a FocusModelRow*
	 * 
	 
	 */
	 FocusModelRow* FocusRow::getFocusModelUsingFocusModelId() {
	 
	 	return table.getContainer().getFocusModel().getRowByKey(focusModelId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of Focus table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having their attribut feedId == this->feedId
	 */
	vector <FeedRow *> FocusRow::getFeeds() {
		
			return table.getContainer().getFeed().getRowByFeedId(feedId);
		
	}
	

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* FocusRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
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
		
	
	

	

	

	

	

	

	

	

	
	

	

	

	
	
	
	

	

	

	

	

	

	

	
	
	}
	
	FocusRow::FocusRow (FocusTable &t, FocusRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	
	

	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			feedId = row.feedId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			focusModelId = row.focusModelId;
		
			xFocusPosition = row.xFocusPosition;
		
			yFocusPosition = row.yFocusPosition;
		
			zFocusPosition = row.zFocusPosition;
		
			focusTracking = row.focusTracking;
		
			xFocusOffset = row.xFocusOffset;
		
			yFocusOffset = row.yFocusOffset;
		
			zFocusOffset = row.zFocusOffset;
		
		
		
		
		}	
	}

	
	bool FocusRow::compareNoAutoInc(Tag antennaId, int feedId, ArrayTimeInterval timeInterval, Tag focusModelId, Length xFocusPosition, Length yFocusPosition, Length zFocusPosition, bool focusTracking, Length xFocusOffset, Length yFocusOffset, Length zFocusOffset) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->feedId == feedId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->focusModelId == focusModelId);
		
		if (!result) return false;
	

	
		
		result = result && (this->xFocusPosition == xFocusPosition);
		
		if (!result) return false;
	

	
		
		result = result && (this->yFocusPosition == yFocusPosition);
		
		if (!result) return false;
	

	
		
		result = result && (this->zFocusPosition == zFocusPosition);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusTracking == focusTracking);
		
		if (!result) return false;
	

	
		
		result = result && (this->xFocusOffset == xFocusOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->yFocusOffset == yFocusOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->zFocusOffset == zFocusOffset);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool FocusRow::compareRequiredValue(Tag focusModelId, Length xFocusPosition, Length yFocusPosition, Length zFocusPosition, bool focusTracking, Length xFocusOffset, Length yFocusOffset, Length zFocusOffset) {
		bool result;
		result = true;
		
	
		if (!(this->focusModelId == focusModelId)) return false;
	

	
		if (!(this->xFocusPosition == xFocusPosition)) return false;
	

	
		if (!(this->yFocusPosition == yFocusPosition)) return false;
	

	
		if (!(this->zFocusPosition == zFocusPosition)) return false;
	

	
		if (!(this->focusTracking == focusTracking)) return false;
	

	
		if (!(this->xFocusOffset == xFocusOffset)) return false;
	

	
		if (!(this->yFocusOffset == yFocusOffset)) return false;
	

	
		if (!(this->zFocusOffset == zFocusOffset)) return false;
	

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
		
			
		if (this->focusModelId != x->focusModelId) return false;
			
		if (this->xFocusPosition != x->xFocusPosition) return false;
			
		if (this->yFocusPosition != x->yFocusPosition) return false;
			
		if (this->zFocusPosition != x->zFocusPosition) return false;
			
		if (this->focusTracking != x->focusTracking) return false;
			
		if (this->xFocusOffset != x->xFocusOffset) return false;
			
		if (this->yFocusOffset != x->yFocusOffset) return false;
			
		if (this->zFocusOffset != x->zFocusOffset) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
