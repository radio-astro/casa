
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
	
	void AntennaRow::isAdded() {
		hasBeenAdded = true;
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
 				
 			
		
	

	
  		
		
		
			
		x->xPosition = xPosition.toIDLLength();
			
		
	

	
  		
		
		
			
		x->yPosition = yPosition.toIDLLength();
			
		
	

	
  		
		
		
			
		x->zPosition = zPosition.toIDLLength();
			
		
	

	
  		
		
		
			
		x->time = time.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->xOffset = xOffset.toIDLLength();
			
		
	

	
  		
		
		
			
		x->yOffset = yOffset.toIDLLength();
			
		
	

	
  		
		
		
			
		x->zOffset = zOffset.toIDLLength();
			
		
	

	
  		
		
		
			
		x->dishDiameter = dishDiameter.toIDLLength();
			
		
	

	
  		
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
	
  	
 		
 		
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
	void AntennaRow::setFromIDL (AntennaRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setName(string (x.name));
			
 		
		
	

	
		
		
			
		setAntennaMake(x.antennaMake);
  			
 		
		
	

	
		
		
			
		setAntennaType(x.antennaType);
  			
 		
		
	

	
		
		
			
		setXPosition(Length (x.xPosition));
			
 		
		
	

	
		
		
			
		setYPosition(Length (x.yPosition));
			
 		
		
	

	
		
		
			
		setZPosition(Length (x.zPosition));
			
 		
		
	

	
		
		
			
		setTime(ArrayTime (x.time));
			
 		
		
	

	
		
		
			
		setXOffset(Length (x.xOffset));
			
 		
		
	

	
		
		
			
		setYOffset(Length (x.yOffset));
			
 		
		
	

	
		
		
			
		setZOffset(Length (x.zOffset));
			
 		
		
	

	
		
		
			
		setDishDiameter(Length (x.dishDiameter));
			
 		
		
	

	
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
	

	
	
		
	
		
		assocAntennaIdExists = x.assocAntennaIdExists;
		if (x.assocAntennaIdExists) {
		
		
			
		setAssocAntennaId(Tag (x.assocAntennaId));
			
 		
		
		}
		
	

	
		
		
			
		setStationId(Tag (x.stationId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Antenna");
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
		
		
	

  	
 		
		
		Parser::toXML(xPosition, "xPosition", buf);
		
		
	

  	
 		
		
		Parser::toXML(yPosition, "yPosition", buf);
		
		
	

  	
 		
		
		Parser::toXML(zPosition, "zPosition", buf);
		
		
	

  	
 		
		
		Parser::toXML(time, "time", buf);
		
		
	

  	
 		
		
		Parser::toXML(xOffset, "xOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(yOffset, "yOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(zOffset, "zOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(dishDiameter, "dishDiameter", buf);
		
		
	

  	
 		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
	

	
	
		
  	
 		
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
	void AntennaRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setName(Parser::getString("name","Antenna",rowDoc));
			
		
	

	
		
		
		
		antennaMake = EnumerationParser::getAntennaMake("antennaMake","Antenna",rowDoc);
		
		
		
	

	
		
		
		
		antennaType = EnumerationParser::getAntennaType("antennaType","Antenna",rowDoc);
		
		
		
	

	
  		
			
	  	setXPosition(Parser::getLength("xPosition","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setYPosition(Parser::getLength("yPosition","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setZPosition(Parser::getLength("zPosition","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setTime(Parser::getArrayTime("time","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setXOffset(Parser::getLength("xOffset","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setYOffset(Parser::getLength("yOffset","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setZOffset(Parser::getLength("zOffset","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setDishDiameter(Parser::getLength("dishDiameter","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFlagRow(Parser::getBoolean("flagRow","Antenna",rowDoc));
			
		
	

	
	
		
	
  		
        if (row.isStr("<assocAntennaId>")) {
			
	  		setAssocAntennaId(Parser::getTag("assocAntennaId","Antenna",rowDoc));
			
		}
 		
	

	
  		
			
	  	setStationId(Parser::getTag("stationId","Antenna",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Antenna");
		}
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
 	 * Get xPosition.
 	 * @return xPosition as Length
 	 */
 	Length AntennaRow::getXPosition() const {
	
  		return xPosition;
 	}

 	/**
 	 * Set xPosition with the specified Length.
 	 * @param xPosition The Length value to which xPosition is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setXPosition (Length xPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->xPosition = xPosition;
	
 	}
	
	

	

	
 	/**
 	 * Get yPosition.
 	 * @return yPosition as Length
 	 */
 	Length AntennaRow::getYPosition() const {
	
  		return yPosition;
 	}

 	/**
 	 * Set yPosition with the specified Length.
 	 * @param yPosition The Length value to which yPosition is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setYPosition (Length yPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->yPosition = yPosition;
	
 	}
	
	

	

	
 	/**
 	 * Get zPosition.
 	 * @return zPosition as Length
 	 */
 	Length AntennaRow::getZPosition() const {
	
  		return zPosition;
 	}

 	/**
 	 * Set zPosition with the specified Length.
 	 * @param zPosition The Length value to which zPosition is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setZPosition (Length zPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->zPosition = zPosition;
	
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
	
	

	

	
 	/**
 	 * Get xOffset.
 	 * @return xOffset as Length
 	 */
 	Length AntennaRow::getXOffset() const {
	
  		return xOffset;
 	}

 	/**
 	 * Set xOffset with the specified Length.
 	 * @param xOffset The Length value to which xOffset is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setXOffset (Length xOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->xOffset = xOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get yOffset.
 	 * @return yOffset as Length
 	 */
 	Length AntennaRow::getYOffset() const {
	
  		return yOffset;
 	}

 	/**
 	 * Set yOffset with the specified Length.
 	 * @param yOffset The Length value to which yOffset is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setYOffset (Length yOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->yOffset = yOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get zOffset.
 	 * @return zOffset as Length
 	 */
 	Length AntennaRow::getZOffset() const {
	
  		return zOffset;
 	}

 	/**
 	 * Set zOffset with the specified Length.
 	 * @param zOffset The Length value to which zOffset is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setZOffset (Length zOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->zOffset = zOffset;
	
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
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool AntennaRow::getFlagRow() const {
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 		
 	 */
 	void AntennaRow::setFlagRow (bool flagRow)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagRow = flagRow;
	
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
 	Tag AntennaRow::getAssocAntennaId() const throw(IllegalAccessException) {
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
	AntennaRow *AntennaRow::getAssociatedAntenna() const throw(NoSuchRow) {

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
	

	

	

	

	

	

	

	

	

	
	
	}
	
	AntennaRow::AntennaRow (AntennaTable &t, AntennaRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
		assocAntennaIdExists = false;
	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
		
		
		
			stationId = row.stationId;
		
			name = row.name;
		
			antennaMake = row.antennaMake;
		
			antennaType = row.antennaType;
		
			xPosition = row.xPosition;
		
			yPosition = row.yPosition;
		
			zPosition = row.zPosition;
		
			time = row.time;
		
			xOffset = row.xOffset;
		
			yOffset = row.yOffset;
		
			zOffset = row.zOffset;
		
			dishDiameter = row.dishDiameter;
		
			flagRow = row.flagRow;
		
		
		
		
		if (row.assocAntennaIdExists) {
			assocAntennaId = row.assocAntennaId;		
			assocAntennaIdExists = true;
		}
		else
			assocAntennaIdExists = false;
		
		}	
	}

	
	bool AntennaRow::compareNoAutoInc(Tag stationId, string name, AntennaMakeMod::AntennaMake antennaMake, AntennaTypeMod::AntennaType antennaType, Length xPosition, Length yPosition, Length zPosition, ArrayTime time, Length xOffset, Length yOffset, Length zOffset, Length dishDiameter, bool flagRow) {
		bool result;
		result = true;
		
	
		
		result = result && (this->stationId == stationId);
		
		if (!result) return false;
	

	
		
		result = result && (this->name == name);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaMake == antennaMake);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaType == antennaType);
		
		if (!result) return false;
	

	
		
		result = result && (this->xPosition == xPosition);
		
		if (!result) return false;
	

	
		
		result = result && (this->yPosition == yPosition);
		
		if (!result) return false;
	

	
		
		result = result && (this->zPosition == zPosition);
		
		if (!result) return false;
	

	
		
		result = result && (this->time == time);
		
		if (!result) return false;
	

	
		
		result = result && (this->xOffset == xOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->yOffset == yOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->zOffset == zOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->dishDiameter == dishDiameter);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagRow == flagRow);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool AntennaRow::compareRequiredValue(Tag stationId, string name, AntennaMakeMod::AntennaMake antennaMake, AntennaTypeMod::AntennaType antennaType, Length xPosition, Length yPosition, Length zPosition, ArrayTime time, Length xOffset, Length yOffset, Length zOffset, Length dishDiameter, bool flagRow) {
		bool result;
		result = true;
		
	
		if (!(this->stationId == stationId)) return false;
	

	
		if (!(this->name == name)) return false;
	

	
		if (!(this->antennaMake == antennaMake)) return false;
	

	
		if (!(this->antennaType == antennaType)) return false;
	

	
		if (!(this->xPosition == xPosition)) return false;
	

	
		if (!(this->yPosition == yPosition)) return false;
	

	
		if (!(this->zPosition == zPosition)) return false;
	

	
		if (!(this->time == time)) return false;
	

	
		if (!(this->xOffset == xOffset)) return false;
	

	
		if (!(this->yOffset == yOffset)) return false;
	

	
		if (!(this->zOffset == zOffset)) return false;
	

	
		if (!(this->dishDiameter == dishDiameter)) return false;
	

	
		if (!(this->flagRow == flagRow)) return false;
	

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
		
			
		if (this->stationId != x->stationId) return false;
			
		if (this->name != x->name) return false;
			
		if (this->antennaMake != x->antennaMake) return false;
			
		if (this->antennaType != x->antennaType) return false;
			
		if (this->xPosition != x->xPosition) return false;
			
		if (this->yPosition != x->yPosition) return false;
			
		if (this->zPosition != x->zPosition) return false;
			
		if (this->time != x->time) return false;
			
		if (this->xOffset != x->xOffset) return false;
			
		if (this->yOffset != x->yOffset) return false;
			
		if (this->zOffset != x->zOffset) return false;
			
		if (this->dishDiameter != x->dishDiameter) return false;
			
		if (this->flagRow != x->flagRow) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
