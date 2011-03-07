
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
 * File AntennaRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <AntennaRow.h>
#include <AntennaTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <StationTable.h>
#include <StationRow.h>
	

using asdm::ASDM;
using asdm::AntennaRow;
using asdm::AntennaTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::StationTable;
using asdm::StationRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	AntennaRow::~AntennaRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	AntennaTable &AntennaRow::getTable() const {
		return table;
	}

	bool AntennaRow::isAdded() const {
		return hasBeenAdded;
	}	

	void AntennaRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a AntennaRowIDL struct.
	 */
	AntennaRowIDL *AntennaRow::toIDL() const {
		AntennaRowIDL *x = new AntennaRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->antennaId = antennaId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->name = CORBA::string_dup(name.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->antennaMake = antennaMake;
 				
 			
		
	

	
  		
		
		
			
				
		x->antennaType = antennaType;
 				
 			
		
	

	
  		
		
		
			
		x->dishDiameter = dishDiameter.toIDLLength();
			
		
	

	
  		
		
		
			
		x->position.length(position.size());
		for (unsigned int i = 0; i < position.size(); ++i) {
			
			x->position[i] = position.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->offset.length(offset.size());
		for (unsigned int i = 0; i < offset.size(); ++i) {
			
			x->offset[i] = offset.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->time = time.toIDLArrayTime();
			
		
	

	
	
		
	
  	
 		
 		
		x->assocAntennaIdExists = assocAntennaIdExists;
		
		
	 	
			
		x->assocAntennaId = assocAntennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->stationId = stationId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct AntennaRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void AntennaRow::setFromIDL (AntennaRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setName(string (x.name));
			
 		
		
	

	
		
		
			
		setAntennaMake(x.antennaMake);
  			
 		
		
	

	
		
		
			
		setAntennaType(x.antennaType);
  			
 		
		
	

	
		
		
			
		setDishDiameter(Length (x.dishDiameter));
			
 		
		
	

	
		
		
			
		position .clear();
		for (unsigned int i = 0; i <x.position.length(); ++i) {
			
			position.push_back(Length (x.position[i]));
			
		}
			
  		
		
	

	
		
		
			
		offset .clear();
		for (unsigned int i = 0; i <x.offset.length(); ++i) {
			
			offset.push_back(Length (x.offset[i]));
			
		}
			
  		
		
	

	
		
		
			
		setTime(ArrayTime (x.time));
			
 		
		
	

	
	
		
	
		
		assocAntennaIdExists = x.assocAntennaIdExists;
		if (x.assocAntennaIdExists) {
		
		
			
		setAssocAntennaId(Tag (x.assocAntennaId));
			
 		
		
		}
		
	

	
		
		
			
		setStationId(Tag (x.stationId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Antenna");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string AntennaRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(name, "name", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("antennaMake", antennaMake));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("antennaType", antennaType));
		
		
	

  	
 		
		
		Parser::toXML(dishDiameter, "dishDiameter", buf);
		
		
	

  	
 		
		
		Parser::toXML(position, "position", buf);
		
		
	

  	
 		
		
		Parser::toXML(offset, "offset", buf);
		
		
	

  	
 		
		
		Parser::toXML(time, "time", buf);
		
		
	

	
	
		
  	
 		
		if (assocAntennaIdExists) {
		
		
		Parser::toXML(assocAntennaId, "assocAntennaId", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(stationId, "stationId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void AntennaRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setName(Parser::getString("name","Antenna",rowDoc));
			
		
	

	
		
		
		
		antennaMake = EnumerationParser::getAntennaMake("antennaMake","Antenna",rowDoc);
		
		
		
	

	
		
		
		
		antennaType = EnumerationParser::getAntennaType("antennaType","Antenna",rowDoc);
		
		
		
	

	
  		
			
	  	setDishDiameter(Parser::getLength("dishDiameter","Antenna",rowDoc));
			
		
	

	
  		
			
					
	  	setPosition(Parser::get1DLength("position","Antenna",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setOffset(Parser::get1DLength("offset","Antenna",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setTime(Parser::getArrayTime("time","Antenna",rowDoc));
			
		
	

	
	
		
	
  		
        if (row.isStr("<assocAntennaId>")) {
			
	  		setAssocAntennaId(Parser::getTag("assocAntennaId","Antenna",rowDoc));
			
		}
 		
	

	
  		
			
	  	setStationId(Parser::getTag("stationId","Antenna",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Antenna");
		}
	}
	
	void AntennaRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(name);
				
		
	

	
	
		
					
			eoss.writeInt(antennaMake);
				
		
	

	
	
		
					
			eoss.writeInt(antennaType);
				
		
	

	
	
		
	dishDiameter.toBin(eoss);
		
	

	
	
		
	Length::toBin(position, eoss);
		
	

	
	
		
	Length::toBin(offset, eoss);
		
	

	
	
		
	time.toBin(eoss);
		
	

	
	
		
	stationId.toBin(eoss);
		
	


	
	
	eoss.writeBoolean(assocAntennaIdExists);
	if (assocAntennaIdExists) {
	
	
	
		
	assocAntennaId.toBin(eoss);
		
	

	}

	}
	
void AntennaRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void AntennaRow::nameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		name =  eiss.readString();
			
		
	
	
}
void AntennaRow::antennaMakeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaMake = CAntennaMake::from_int(eiss.readInt());
			
		
	
	
}
void AntennaRow::antennaTypeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaType = CAntennaType::from_int(eiss.readInt());
			
		
	
	
}
void AntennaRow::dishDiameterFromBin(EndianISStream& eiss) {
		
	
		
		
		dishDiameter =  Length::fromBin(eiss);
		
	
	
}
void AntennaRow::positionFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	position = Length::from1DBin(eiss);	
	

		
	
	
}
void AntennaRow::offsetFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	offset = Length::from1DBin(eiss);	
	

		
	
	
}
void AntennaRow::timeFromBin(EndianISStream& eiss) {
		
	
		
		
		time =  ArrayTime::fromBin(eiss);
		
	
	
}
void AntennaRow::stationIdFromBin(EndianISStream& eiss) {
		
	
		
		
		stationId =  Tag::fromBin(eiss);
		
	
	
}

void AntennaRow::assocAntennaIdFromBin(EndianISStream& eiss) {
		
	assocAntennaIdExists = eiss.readBoolean();
	if (assocAntennaIdExists) {
		
	
		
		
		assocAntennaId =  Tag::fromBin(eiss);
		
	

	}
	
}
	
	
	AntennaRow* AntennaRow::fromBin(EndianISStream& eiss, AntennaTable& table, const vector<string>& attributesSeq) {
		AntennaRow* row = new  AntennaRow(table);
		
		map<string, AntennaAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "AntennaTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag AntennaRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void AntennaRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "Antenna");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get name.
 	 * @return name as string
 	 */
 	string AntennaRow::getName() const {
	
  		return name;
 	}

 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setName (string name)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->name = name;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake AntennaRow::getAntennaMake() const {
	
  		return antennaMake;
 	}

 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setAntennaMake (AntennaMakeMod::AntennaMake antennaMake)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaMake = antennaMake;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaType.
 	 * @return antennaType as AntennaTypeMod::AntennaType
 	 */
 	AntennaTypeMod::AntennaType AntennaRow::getAntennaType() const {
	
  		return antennaType;
 	}

 	/**
 	 * Set antennaType with the specified AntennaTypeMod::AntennaType.
 	 * @param antennaType The AntennaTypeMod::AntennaType value to which antennaType is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setAntennaType (AntennaTypeMod::AntennaType antennaType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaType = antennaType;
	
 	}
	
	

	

	
 	/**
 	 * Get dishDiameter.
 	 * @return dishDiameter as Length
 	 */
 	Length AntennaRow::getDishDiameter() const {
	
  		return dishDiameter;
 	}

 	/**
 	 * Set dishDiameter with the specified Length.
 	 * @param dishDiameter The Length value to which dishDiameter is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setDishDiameter (Length dishDiameter)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->dishDiameter = dishDiameter;
	
 	}
	
	

	

	
 	/**
 	 * Get position.
 	 * @return position as vector<Length >
 	 */
 	vector<Length > AntennaRow::getPosition() const {
	
  		return position;
 	}

 	/**
 	 * Set position with the specified vector<Length >.
 	 * @param position The vector<Length > value to which position is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setPosition (vector<Length > position)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->position = position;
	
 	}
	
	

	

	
 	/**
 	 * Get offset.
 	 * @return offset as vector<Length >
 	 */
 	vector<Length > AntennaRow::getOffset() const {
	
  		return offset;
 	}

 	/**
 	 * Set offset with the specified vector<Length >.
 	 * @param offset The vector<Length > value to which offset is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setOffset (vector<Length > offset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->offset = offset;
	
 	}
	
	

	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime AntennaRow::getTime() const {
	
  		return time;
 	}

 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setTime (ArrayTime time)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->time = time;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	/**
	 * The attribute assocAntennaId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocAntennaId attribute exists. 
	 */
	bool AntennaRow::isAssocAntennaIdExists() const {
		return assocAntennaIdExists;
	}
	

	
 	/**
 	 * Get assocAntennaId, which is optional.
 	 * @return assocAntennaId as Tag
 	 * @throw IllegalAccessException If assocAntennaId does not exist.
 	 */
 	Tag AntennaRow::getAssocAntennaId() const  {
		if (!assocAntennaIdExists) {
			throw IllegalAccessException("assocAntennaId", "Antenna");
		}
	
  		return assocAntennaId;
 	}

 	/**
 	 * Set assocAntennaId with the specified Tag.
 	 * @param assocAntennaId The Tag value to which assocAntennaId is to be set.
 	 
 	
 	 */
 	void AntennaRow::setAssocAntennaId (Tag assocAntennaId) {
	
 		this->assocAntennaId = assocAntennaId;
	
		assocAntennaIdExists = true;
	
 	}
	
	
	/**
	 * Mark assocAntennaId, which is an optional field, as non-existent.
	 */
	void AntennaRow::clearAssocAntennaId () {
		assocAntennaIdExists = false;
	}
	

	

	
 	/**
 	 * Get stationId.
 	 * @return stationId as Tag
 	 */
 	Tag AntennaRow::getStationId() const {
	
  		return stationId;
 	}

 	/**
 	 * Set stationId with the specified Tag.
 	 * @param stationId The Tag value to which stationId is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setStationId (Tag stationId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->stationId = stationId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
		
		
		
			
	// ===> Optional link from a row of Antenna table to a row of Antenna table.

	/**
	 * The link to table Antenna is optional. Return true if this link exists.
	 * @return true if and only if the Antenna link exists. 
	 */
	bool AntennaRow::isAssociatedAntennaExists() const {
		return assocAntennaIdExists;
	}

	/**
	 * Get the optional row in table Antenna by traversing the defined link to that table.
	 * @return A row in Antenna table.
	 * @throws NoSuchRow if there is no such row in table Antenna or the link does not exist.
	 */
	AntennaRow *AntennaRow::getAssociatedAntenna() const {

		if (!assocAntennaIdExists) {
			throw NoSuchRow("Antenna","Antenna",true);
		}

		return table.getContainer().getAntenna().getRowByKey( antennaId );
	}
	
	/**
	 * Set the values of the link attributes needed to link this row to a row in table Antenna.
	 */
	void AntennaRow::setAssociatedAntennaLink(Tag assocAntennaId) {

		this->assocAntennaId = assocAntennaId;
		assocAntennaIdExists = true;

	}



		
		
	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Station table having Station.stationId == stationId
	 * @return a StationRow*
	 * 
	 
	 */
	 StationRow* AntennaRow::getStationUsingStationId() {
	 
	 	return table.getContainer().getStation().getRowByKey(stationId);
	 }
	 

	

	
	/**
	 * Create a AntennaRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	AntennaRow::AntennaRow (AntennaTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	
	
		assocAntennaIdExists = false;
	

	

	
	
	
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
antennaMake = CAntennaMake::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
antennaType = CAntennaType::from_int(0);
	

	

	

	

	

	
	
	 fromBinMethods["antennaId"] = &AntennaRow::antennaIdFromBin; 
	 fromBinMethods["name"] = &AntennaRow::nameFromBin; 
	 fromBinMethods["antennaMake"] = &AntennaRow::antennaMakeFromBin; 
	 fromBinMethods["antennaType"] = &AntennaRow::antennaTypeFromBin; 
	 fromBinMethods["dishDiameter"] = &AntennaRow::dishDiameterFromBin; 
	 fromBinMethods["position"] = &AntennaRow::positionFromBin; 
	 fromBinMethods["offset"] = &AntennaRow::offsetFromBin; 
	 fromBinMethods["time"] = &AntennaRow::timeFromBin; 
	 fromBinMethods["stationId"] = &AntennaRow::stationIdFromBin; 
		
	
	 fromBinMethods["assocAntennaId"] = &AntennaRow::assocAntennaIdFromBin; 
	
	}
	
	AntennaRow::AntennaRow (AntennaTable &t, AntennaRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	
	
		assocAntennaIdExists = false;
	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
		
		
		
			name = row.name;
		
			antennaMake = row.antennaMake;
		
			antennaType = row.antennaType;
		
			dishDiameter = row.dishDiameter;
		
			position = row.position;
		
			offset = row.offset;
		
			time = row.time;
		
			stationId = row.stationId;
		
		
		
		
		if (row.assocAntennaIdExists) {
			assocAntennaId = row.assocAntennaId;		
			assocAntennaIdExists = true;
		}
		else
			assocAntennaIdExists = false;
		
		}
		
		 fromBinMethods["antennaId"] = &AntennaRow::antennaIdFromBin; 
		 fromBinMethods["name"] = &AntennaRow::nameFromBin; 
		 fromBinMethods["antennaMake"] = &AntennaRow::antennaMakeFromBin; 
		 fromBinMethods["antennaType"] = &AntennaRow::antennaTypeFromBin; 
		 fromBinMethods["dishDiameter"] = &AntennaRow::dishDiameterFromBin; 
		 fromBinMethods["position"] = &AntennaRow::positionFromBin; 
		 fromBinMethods["offset"] = &AntennaRow::offsetFromBin; 
		 fromBinMethods["time"] = &AntennaRow::timeFromBin; 
		 fromBinMethods["stationId"] = &AntennaRow::stationIdFromBin; 
			
	
		 fromBinMethods["assocAntennaId"] = &AntennaRow::assocAntennaIdFromBin; 
			
	}

	
	bool AntennaRow::compareNoAutoInc(string name, AntennaMakeMod::AntennaMake antennaMake, AntennaTypeMod::AntennaType antennaType, Length dishDiameter, vector<Length > position, vector<Length > offset, ArrayTime time, Tag stationId) {
		bool result;
		result = true;
		
	
		
		result = result && (this->name == name);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaMake == antennaMake);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaType == antennaType);
		
		if (!result) return false;
	

	
		
		result = result && (this->dishDiameter == dishDiameter);
		
		if (!result) return false;
	

	
		
		result = result && (this->position == position);
		
		if (!result) return false;
	

	
		
		result = result && (this->offset == offset);
		
		if (!result) return false;
	

	
		
		result = result && (this->time == time);
		
		if (!result) return false;
	

	
		
		result = result && (this->stationId == stationId);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool AntennaRow::compareRequiredValue(string name, AntennaMakeMod::AntennaMake antennaMake, AntennaTypeMod::AntennaType antennaType, Length dishDiameter, vector<Length > position, vector<Length > offset, ArrayTime time, Tag stationId) {
		bool result;
		result = true;
		
	
		if (!(this->name == name)) return false;
	

	
		if (!(this->antennaMake == antennaMake)) return false;
	

	
		if (!(this->antennaType == antennaType)) return false;
	

	
		if (!(this->dishDiameter == dishDiameter)) return false;
	

	
		if (!(this->position == position)) return false;
	

	
		if (!(this->offset == offset)) return false;
	

	
		if (!(this->time == time)) return false;
	

	
		if (!(this->stationId == stationId)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the AntennaRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool AntennaRow::equalByRequiredValue(AntennaRow* x) {
		
			
		if (this->name != x->name) return false;
			
		if (this->antennaMake != x->antennaMake) return false;
			
		if (this->antennaType != x->antennaType) return false;
			
		if (this->dishDiameter != x->dishDiameter) return false;
			
		if (this->position != x->position) return false;
			
		if (this->offset != x->offset) return false;
			
		if (this->time != x->time) return false;
			
		if (this->stationId != x->stationId) return false;
			
		
		return true;
	}	
	
/*
	 map<string, AntennaAttributeFromBin> AntennaRow::initFromBinMethods() {
		map<string, AntennaAttributeFromBin> result;
		
		result["antennaId"] = &AntennaRow::antennaIdFromBin;
		result["name"] = &AntennaRow::nameFromBin;
		result["antennaMake"] = &AntennaRow::antennaMakeFromBin;
		result["antennaType"] = &AntennaRow::antennaTypeFromBin;
		result["dishDiameter"] = &AntennaRow::dishDiameterFromBin;
		result["position"] = &AntennaRow::positionFromBin;
		result["offset"] = &AntennaRow::offsetFromBin;
		result["time"] = &AntennaRow::timeFromBin;
		result["stationId"] = &AntennaRow::stationIdFromBin;
		
		
		result["assocAntennaId"] = &AntennaRow::assocAntennaIdFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
