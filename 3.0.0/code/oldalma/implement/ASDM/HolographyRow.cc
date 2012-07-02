
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
 * File HolographyRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <HolographyRow.h>
#include <HolographyTable.h>
	

using asdm::ASDM;
using asdm::HolographyRow;
using asdm::HolographyTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	HolographyRow::~HolographyRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	HolographyTable &HolographyRow::getTable() const {
		return table;
	}
	
	void HolographyRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a HolographyRowIDL struct.
	 */
	HolographyRowIDL *HolographyRow::toIDL() const {
		HolographyRowIDL *x = new HolographyRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->holographyId = holographyId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->numCorr = numCorr;
 				
 			
		
	

	
  		
		
		
			
		x->type.length(type.size());
		for (unsigned int i = 0; i < type.size(); ++i) {
			
				
			x->type[i] = type.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->distance = distance.toIDLLength();
			
		
	

	
  		
		
		
			
		x->focus = focus.toIDLLength();
			
		
	

	
  		
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct HolographyRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void HolographyRow::setFromIDL (HolographyRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setHolographyId(Tag (x.holographyId));
			
 		
		
	

	
		
		
			
		setNumCorr(x.numCorr);
  			
 		
		
	

	
		
		
			
		type .clear();
		for (unsigned int i = 0; i <x.type.length(); ++i) {
			
			type.push_back(x.type[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setDistance(Length (x.distance));
			
 		
		
	

	
		
		
			
		setFocus(Length (x.focus));
			
 		
		
	

	
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Holography");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string HolographyRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(holographyId, "holographyId", buf);
		
		
	

  	
 		
		
		Parser::toXML(numCorr, "numCorr", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("type", type));
		
		
	

  	
 		
		
		Parser::toXML(distance, "distance", buf);
		
		
	

  	
 		
		
		Parser::toXML(focus, "focus", buf);
		
		
	

  	
 		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void HolographyRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setHolographyId(Parser::getTag("holographyId","Holography",rowDoc));
			
		
	

	
  		
			
	  	setNumCorr(Parser::getInteger("numCorr","Holography",rowDoc));
			
		
	

	
		
		
		
		type = EnumerationParser::getHolographyChannelType1D("type","Holography",rowDoc);			
		
		
		
	

	
  		
			
	  	setDistance(Parser::getLength("distance","Holography",rowDoc));
			
		
	

	
  		
			
	  	setFocus(Parser::getLength("focus","Holography",rowDoc));
			
		
	

	
  		
			
	  	setFlagRow(Parser::getBoolean("flagRow","Holography",rowDoc));
			
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Holography");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get holographyId.
 	 * @return holographyId as Tag
 	 */
 	Tag HolographyRow::getHolographyId() const {
	
  		return holographyId;
 	}

 	/**
 	 * Set holographyId with the specified Tag.
 	 * @param holographyId The Tag value to which holographyId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void HolographyRow::setHolographyId (Tag holographyId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("holographyId", "Holography");
		
  		}
  	
 		this->holographyId = holographyId;
	
 	}
	
	

	

	
 	/**
 	 * Get numCorr.
 	 * @return numCorr as int
 	 */
 	int HolographyRow::getNumCorr() const {
	
  		return numCorr;
 	}

 	/**
 	 * Set numCorr with the specified int.
 	 * @param numCorr The int value to which numCorr is to be set.
 	 
 	
 		
 	 */
 	void HolographyRow::setNumCorr (int numCorr)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numCorr = numCorr;
	
 	}
	
	

	

	
 	/**
 	 * Get type.
 	 * @return type as vector<HolographyChannelTypeMod::HolographyChannelType >
 	 */
 	vector<HolographyChannelTypeMod::HolographyChannelType > HolographyRow::getType() const {
	
  		return type;
 	}

 	/**
 	 * Set type with the specified vector<HolographyChannelTypeMod::HolographyChannelType >.
 	 * @param type The vector<HolographyChannelTypeMod::HolographyChannelType > value to which type is to be set.
 	 
 	
 		
 	 */
 	void HolographyRow::setType (vector<HolographyChannelTypeMod::HolographyChannelType > type)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->type = type;
	
 	}
	
	

	

	
 	/**
 	 * Get distance.
 	 * @return distance as Length
 	 */
 	Length HolographyRow::getDistance() const {
	
  		return distance;
 	}

 	/**
 	 * Set distance with the specified Length.
 	 * @param distance The Length value to which distance is to be set.
 	 
 	
 		
 	 */
 	void HolographyRow::setDistance (Length distance)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->distance = distance;
	
 	}
	
	

	

	
 	/**
 	 * Get focus.
 	 * @return focus as Length
 	 */
 	Length HolographyRow::getFocus() const {
	
  		return focus;
 	}

 	/**
 	 * Set focus with the specified Length.
 	 * @param focus The Length value to which focus is to be set.
 	 
 	
 		
 	 */
 	void HolographyRow::setFocus (Length focus)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focus = focus;
	
 	}
	
	

	

	
 	/**
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool HolographyRow::getFlagRow() const {
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 		
 	 */
 	void HolographyRow::setFlagRow (bool flagRow)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagRow = flagRow;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a HolographyRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	HolographyRow::HolographyRow (HolographyTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	
	
	
	
	

	

	

	

	

	
	
	}
	
	HolographyRow::HolographyRow (HolographyTable &t, HolographyRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

			
		}
		else {
	
		
			holographyId = row.holographyId;
		
		
		
		
			numCorr = row.numCorr;
		
			type = row.type;
		
			distance = row.distance;
		
			focus = row.focus;
		
			flagRow = row.flagRow;
		
		
		
		
		}	
	}

	
	bool HolographyRow::compareNoAutoInc(int numCorr, vector<HolographyChannelTypeMod::HolographyChannelType > type, Length distance, Length focus, bool flagRow) {
		bool result;
		result = true;
		
	
		
		result = result && (this->numCorr == numCorr);
		
		if (!result) return false;
	

	
		
		result = result && (this->type == type);
		
		if (!result) return false;
	

	
		
		result = result && (this->distance == distance);
		
		if (!result) return false;
	

	
		
		result = result && (this->focus == focus);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagRow == flagRow);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool HolographyRow::compareRequiredValue(int numCorr, vector<HolographyChannelTypeMod::HolographyChannelType > type, Length distance, Length focus, bool flagRow) {
		bool result;
		result = true;
		
	
		if (!(this->numCorr == numCorr)) return false;
	

	
		if (!(this->type == type)) return false;
	

	
		if (!(this->distance == distance)) return false;
	

	
		if (!(this->focus == focus)) return false;
	

	
		if (!(this->flagRow == flagRow)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the HolographyRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool HolographyRow::equalByRequiredValue(HolographyRow* x) {
		
			
		if (this->numCorr != x->numCorr) return false;
			
		if (this->type != x->type) return false;
			
		if (this->distance != x->distance) return false;
			
		if (this->focus != x->focus) return false;
			
		if (this->flagRow != x->flagRow) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
