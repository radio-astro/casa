
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
			
		
	

	
  		
		
		
			
		x->distance = distance.toIDLLength();
			
		
	

	
  		
		
		
			
		x->focus = focus.toIDLLength();
			
		
	

	
  		
		
		
			
				
		x->numCorr = numCorr;
 				
 			
		
	

	
  		
		
		
			
		x->type.length(type.size());
		for (unsigned int i = 0; i < type.size(); ++i) {
			
				
			x->type[i] = type.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct HolographyRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void HolographyRow::setFromIDL (HolographyRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setHolographyId(Tag (x.holographyId));
			
 		
		
	

	
		
		
			
		setDistance(Length (x.distance));
			
 		
		
	

	
		
		
			
		setFocus(Length (x.focus));
			
 		
		
	

	
		
		
			
		setNumCorr(x.numCorr);
  			
 		
		
	

	
		
		
			
		type .clear();
		for (unsigned int i = 0; i <x.type.length(); ++i) {
			
			type.push_back(x.type[i]);
  			
		}
			
  		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Holography");
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
		
		
	

  	
 		
		
		Parser::toXML(distance, "distance", buf);
		
		
	

  	
 		
		
		Parser::toXML(focus, "focus", buf);
		
		
	

  	
 		
		
		Parser::toXML(numCorr, "numCorr", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("type", type));
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void HolographyRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setHolographyId(Parser::getTag("holographyId","Holography",rowDoc));
			
		
	

	
  		
			
	  	setDistance(Parser::getLength("distance","Holography",rowDoc));
			
		
	

	
  		
			
	  	setFocus(Parser::getLength("focus","Holography",rowDoc));
			
		
	

	
  		
			
	  	setNumCorr(Parser::getInteger("numCorr","Holography",rowDoc));
			
		
	

	
		
		
		
		type = EnumerationParser::getHolographyChannelType1D("type","Holography",rowDoc);			
		
		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Holography");
		}
	}
	
	void HolographyRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	holographyId.toBin(eoss);
		
	

	
	
		
	distance.toBin(eoss);
		
	

	
	
		
	focus.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numCorr);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) type.size());
		for (unsigned int i = 0; i < type.size(); i++)
				
			eoss.writeInt(type.at(i));
				
				
						
		
	


	
	
	}
	
	HolographyRow* HolographyRow::fromBin(EndianISStream& eiss, HolographyTable& table) {
		HolographyRow* row = new  HolographyRow(table);
		
		
		
	
		
		
		row->holographyId =  Tag::fromBin(eiss);
		
	

	
		
		
		row->distance =  Length::fromBin(eiss);
		
	

	
		
		
		row->focus =  Length::fromBin(eiss);
		
	

	
	
		
			
		row->numCorr =  eiss.readInt();
			
		
	

	
	
		
			
	
		row->type.clear();
		
		unsigned int typeDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < typeDim1; i++)
			
			row->type.push_back(CHolographyChannelType::from_int(eiss.readInt()));
			
	

		
	

		
		
		
		
		return row;
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
		
		
		
		
			distance = row.distance;
		
			focus = row.focus;
		
			numCorr = row.numCorr;
		
			type = row.type;
		
		
		
		
		}	
	}

	
	bool HolographyRow::compareNoAutoInc(Length distance, Length focus, int numCorr, vector<HolographyChannelTypeMod::HolographyChannelType > type) {
		bool result;
		result = true;
		
	
		
		result = result && (this->distance == distance);
		
		if (!result) return false;
	

	
		
		result = result && (this->focus == focus);
		
		if (!result) return false;
	

	
		
		result = result && (this->numCorr == numCorr);
		
		if (!result) return false;
	

	
		
		result = result && (this->type == type);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool HolographyRow::compareRequiredValue(Length distance, Length focus, int numCorr, vector<HolographyChannelTypeMod::HolographyChannelType > type) {
		bool result;
		result = true;
		
	
		if (!(this->distance == distance)) return false;
	

	
		if (!(this->focus == focus)) return false;
	

	
		if (!(this->numCorr == numCorr)) return false;
	

	
		if (!(this->type == type)) return false;
	

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
		
			
		if (this->distance != x->distance) return false;
			
		if (this->focus != x->focus) return false;
			
		if (this->numCorr != x->numCorr) return false;
			
		if (this->type != x->type) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
