
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
 * File StationRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <StationRow.h>
#include <StationTable.h>
	

using asdm::ASDM;
using asdm::StationRow;
using asdm::StationTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	StationRow::~StationRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	StationTable &StationRow::getTable() const {
		return table;
	}
	
	void StationRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a StationRowIDL struct.
	 */
	StationRowIDL *StationRow::toIDL() const {
		StationRowIDL *x = new StationRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->stationId = stationId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->name = CORBA::string_dup(name.c_str());
				
 			
		
	

	
  		
		
		
			
		x->position.length(position.size());
		for (unsigned int i = 0; i < position.size(); ++i) {
			
			x->position[i] = position.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->type = type;
 				
 			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct StationRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void StationRow::setFromIDL (StationRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setStationId(Tag (x.stationId));
			
 		
		
	

	
		
		
			
		setName(string (x.name));
			
 		
		
	

	
		
		
			
		position .clear();
		for (unsigned int i = 0; i <x.position.length(); ++i) {
			
			position.push_back(Length (x.position[i]));
			
		}
			
  		
		
	

	
		
		
			
		setType(x.type);
  			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Station");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string StationRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(stationId, "stationId", buf);
		
		
	

  	
 		
		
		Parser::toXML(name, "name", buf);
		
		
	

  	
 		
		
		Parser::toXML(position, "position", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("type", type));
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void StationRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setStationId(Parser::getTag("stationId","Station",rowDoc));
			
		
	

	
  		
			
	  	setName(Parser::getString("name","Station",rowDoc));
			
		
	

	
  		
			
					
	  	setPosition(Parser::get1DLength("position","Station",rowDoc));
	  			
	  		
		
	

	
		
		
		
		type = EnumerationParser::getStationType("type","Station",rowDoc);
		
		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Station");
		}
	}
	
	void StationRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	stationId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(name);
				
		
	

	
	
		
	Length::toBin(position, eoss);
		
	

	
	
		
					
			eoss.writeInt(type);
				
		
	


	
	
	}
	
	StationRow* StationRow::fromBin(EndianISStream& eiss, StationTable& table) {
		StationRow* row = new  StationRow(table);
		
		
		
	
		
		
		row->stationId =  Tag::fromBin(eiss);
		
	

	
	
		
			
		row->name =  eiss.readString();
			
		
	

	
		
		
			
	
	row->position = Length::from1DBin(eiss);	
	

		
	

	
	
		
			
		row->type = CStationType::from_int(eiss.readInt());
			
		
	

		
		
		
		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get stationId.
 	 * @return stationId as Tag
 	 */
 	Tag StationRow::getStationId() const {
	
  		return stationId;
 	}

 	/**
 	 * Set stationId with the specified Tag.
 	 * @param stationId The Tag value to which stationId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void StationRow::setStationId (Tag stationId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("stationId", "Station");
		
  		}
  	
 		this->stationId = stationId;
	
 	}
	
	

	

	
 	/**
 	 * Get name.
 	 * @return name as string
 	 */
 	string StationRow::getName() const {
	
  		return name;
 	}

 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 	 
 	
 		
 	 */
 	void StationRow::setName (string name)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->name = name;
	
 	}
	
	

	

	
 	/**
 	 * Get position.
 	 * @return position as vector<Length >
 	 */
 	vector<Length > StationRow::getPosition() const {
	
  		return position;
 	}

 	/**
 	 * Set position with the specified vector<Length >.
 	 * @param position The vector<Length > value to which position is to be set.
 	 
 	
 		
 	 */
 	void StationRow::setPosition (vector<Length > position)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->position = position;
	
 	}
	
	

	

	
 	/**
 	 * Get type.
 	 * @return type as StationTypeMod::StationType
 	 */
 	StationTypeMod::StationType StationRow::getType() const {
	
  		return type;
 	}

 	/**
 	 * Set type with the specified StationTypeMod::StationType.
 	 * @param type The StationTypeMod::StationType value to which type is to be set.
 	 
 	
 		
 	 */
 	void StationRow::setType (StationTypeMod::StationType type)  {
  	
  	
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
	 * Create a StationRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	StationRow::StationRow (StationTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	
	
	
	
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
type = CStationType::from_int(0);
	
	
	}
	
	StationRow::StationRow (StationTable &t, StationRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

			
		}
		else {
	
		
			stationId = row.stationId;
		
		
		
		
			name = row.name;
		
			position = row.position;
		
			type = row.type;
		
		
		
		
		}	
	}

	
	bool StationRow::compareNoAutoInc(string name, vector<Length > position, StationTypeMod::StationType type) {
		bool result;
		result = true;
		
	
		
		result = result && (this->name == name);
		
		if (!result) return false;
	

	
		
		result = result && (this->position == position);
		
		if (!result) return false;
	

	
		
		result = result && (this->type == type);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool StationRow::compareRequiredValue(string name, vector<Length > position, StationTypeMod::StationType type) {
		bool result;
		result = true;
		
	
		if (!(this->name == name)) return false;
	

	
		if (!(this->position == position)) return false;
	

	
		if (!(this->type == type)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the StationRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool StationRow::equalByRequiredValue(StationRow* x) {
		
			
		if (this->name != x->name) return false;
			
		if (this->position != x->position) return false;
			
		if (this->type != x->type) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
