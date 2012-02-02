
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
 * File AlmaRadiometerRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <AlmaRadiometerRow.h>
#include <AlmaRadiometerTable.h>
	

using asdm::ASDM;
using asdm::AlmaRadiometerRow;
using asdm::AlmaRadiometerTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	AlmaRadiometerRow::~AlmaRadiometerRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	AlmaRadiometerTable &AlmaRadiometerRow::getTable() const {
		return table;
	}
	
	void AlmaRadiometerRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a AlmaRadiometerRowIDL struct.
	 */
	AlmaRadiometerRowIDL *AlmaRadiometerRow::toIDL() const {
		AlmaRadiometerRowIDL *x = new AlmaRadiometerRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->modeId = modeId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->numBand = numBand;
 				
 			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct AlmaRadiometerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void AlmaRadiometerRow::setFromIDL (AlmaRadiometerRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setModeId(Tag (x.modeId));
			
 		
		
	

	
		
		
			
		setNumBand(x.numBand);
  			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"AlmaRadiometer");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string AlmaRadiometerRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(modeId, "modeId", buf);
		
		
	

  	
 		
		
		Parser::toXML(numBand, "numBand", buf);
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void AlmaRadiometerRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setModeId(Parser::getTag("modeId","AlmaRadiometer",rowDoc));
			
		
	

	
  		
			
	  	setNumBand(Parser::getInteger("numBand","AlmaRadiometer",rowDoc));
			
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"AlmaRadiometer");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get modeId.
 	 * @return modeId as Tag
 	 */
 	Tag AlmaRadiometerRow::getModeId() const {
	
  		return modeId;
 	}

 	/**
 	 * Set modeId with the specified Tag.
 	 * @param modeId The Tag value to which modeId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void AlmaRadiometerRow::setModeId (Tag modeId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("modeId", "AlmaRadiometer");
		
  		}
  	
 		this->modeId = modeId;
	
 	}
	
	

	

	
 	/**
 	 * Get numBand.
 	 * @return numBand as int
 	 */
 	int AlmaRadiometerRow::getNumBand() const {
	
  		return numBand;
 	}

 	/**
 	 * Set numBand with the specified int.
 	 * @param numBand The int value to which numBand is to be set.
 	 
 	
 		
 	 */
 	void AlmaRadiometerRow::setNumBand (int numBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numBand = numBand;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a AlmaRadiometerRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	AlmaRadiometerRow::AlmaRadiometerRow (AlmaRadiometerTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	
	
	
	
	

	
	
	}
	
	AlmaRadiometerRow::AlmaRadiometerRow (AlmaRadiometerTable &t, AlmaRadiometerRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

			
		}
		else {
	
		
			modeId = row.modeId;
		
		
		
		
			numBand = row.numBand;
		
		
		
		
		}	
	}

	
	bool AlmaRadiometerRow::compareNoAutoInc(int numBand) {
		bool result;
		result = true;
		
	
		
		result = result && (this->numBand == numBand);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool AlmaRadiometerRow::compareRequiredValue(int numBand) {
		bool result;
		result = true;
		
	
		if (!(this->numBand == numBand)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the AlmaRadiometerRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool AlmaRadiometerRow::equalByRequiredValue(AlmaRadiometerRow* x) {
		
			
		if (this->numBand != x->numBand) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
