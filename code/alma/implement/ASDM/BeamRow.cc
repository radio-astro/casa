
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
 * File BeamRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <BeamRow.h>
#include <BeamTable.h>
	

using asdm::ASDM;
using asdm::BeamRow;
using asdm::BeamTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	BeamRow::~BeamRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	BeamTable &BeamRow::getTable() const {
		return table;
	}
	
	void BeamRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a BeamRowIDL struct.
	 */
	BeamRowIDL *BeamRow::toIDL() const {
		BeamRowIDL *x = new BeamRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->beamId = beamId.toIDLTag();
			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct BeamRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void BeamRow::setFromIDL (BeamRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setBeamId(Tag (x.beamId));
			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Beam");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string BeamRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(beamId, "beamId", buf);
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void BeamRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setBeamId(Parser::getTag("beamId","Beam",rowDoc));
			
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Beam");
		}
	}
	
	void BeamRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	beamId.toBin(eoss);
		
	


	
	
	}
	
	BeamRow* BeamRow::fromBin(EndianISStream& eiss, BeamTable& table) {
		BeamRow* row = new  BeamRow(table);
		
		
		
	
		
		
		row->beamId =  Tag::fromBin(eiss);
		
	

		
		
		
		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get beamId.
 	 * @return beamId as Tag
 	 */
 	Tag BeamRow::getBeamId() const {
	
  		return beamId;
 	}

 	/**
 	 * Set beamId with the specified Tag.
 	 * @param beamId The Tag value to which beamId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void BeamRow::setBeamId (Tag beamId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("beamId", "Beam");
		
  		}
  	
 		this->beamId = beamId;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a BeamRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	BeamRow::BeamRow (BeamTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	
	
	
	
	
	
	}
	
	BeamRow::BeamRow (BeamTable &t, BeamRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

			
		}
		else {
	
		
			beamId = row.beamId;
		
		
		
		
		
		
		
		}	
	}

	
	
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the BeamRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool BeamRow::equalByRequiredValue(BeamRow* x) {
		
		return true;
	}	
	

} // End namespace asdm
 
