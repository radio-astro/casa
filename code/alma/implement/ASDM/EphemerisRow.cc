
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
 * File EphemerisRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <EphemerisRow.h>
#include <EphemerisTable.h>
	

using asdm::ASDM;
using asdm::EphemerisRow;
using asdm::EphemerisTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	EphemerisRow::~EphemerisRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	EphemerisTable &EphemerisRow::getTable() const {
		return table;
	}
	
	void EphemerisRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a EphemerisRowIDL struct.
	 */
	EphemerisRowIDL *EphemerisRow::toIDL() const {
		EphemerisRowIDL *x = new EphemerisRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->ephemerisId = ephemerisId.toIDLTag();
			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct EphemerisRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void EphemerisRow::setFromIDL (EphemerisRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setEphemerisId(Tag (x.ephemerisId));
			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Ephemeris");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string EphemerisRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(ephemerisId, "ephemerisId", buf);
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void EphemerisRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setEphemerisId(Parser::getTag("ephemerisId","Ephemeris",rowDoc));
			
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Ephemeris");
		}
	}
	
	void EphemerisRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	ephemerisId.toBin(eoss);
		
	


	
	
	}
	
	EphemerisRow* EphemerisRow::fromBin(EndianISStream& eiss, EphemerisTable& table) {
		EphemerisRow* row = new  EphemerisRow(table);
		
		
		
	
		
		
		row->ephemerisId =  Tag::fromBin(eiss);
		
	

		
		
		
		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get ephemerisId.
 	 * @return ephemerisId as Tag
 	 */
 	Tag EphemerisRow::getEphemerisId() const {
	
  		return ephemerisId;
 	}

 	/**
 	 * Set ephemerisId with the specified Tag.
 	 * @param ephemerisId The Tag value to which ephemerisId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void EphemerisRow::setEphemerisId (Tag ephemerisId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("ephemerisId", "Ephemeris");
		
  		}
  	
 		this->ephemerisId = ephemerisId;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a EphemerisRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	EphemerisRow::EphemerisRow (EphemerisTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	
	
	
	
	
	
	}
	
	EphemerisRow::EphemerisRow (EphemerisTable &t, EphemerisRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

			
		}
		else {
	
		
			ephemerisId = row.ephemerisId;
		
		
		
		
		
		
		
		}	
	}

	
	
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the EphemerisRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool EphemerisRow::equalByRequiredValue(EphemerisRow* x) {
		
		return true;
	}	
	

} // End namespace asdm
 
