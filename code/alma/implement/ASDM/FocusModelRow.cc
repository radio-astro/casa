
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
 * File FocusModelRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <FocusModelRow.h>
#include <FocusModelTable.h>
	

using asdm::ASDM;
using asdm::FocusModelRow;
using asdm::FocusModelTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	FocusModelRow::~FocusModelRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	FocusModelTable &FocusModelRow::getTable() const {
		return table;
	}
	
	void FocusModelRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FocusModelRowIDL struct.
	 */
	FocusModelRowIDL *FocusModelRow::toIDL() const {
		FocusModelRowIDL *x = new FocusModelRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->focusModelId = focusModelId.toIDLTag();
			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FocusModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void FocusModelRow::setFromIDL (FocusModelRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setFocusModelId(Tag (x.focusModelId));
			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"FocusModel");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string FocusModelRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(focusModelId, "focusModelId", buf);
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void FocusModelRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setFocusModelId(Parser::getTag("focusModelId","FocusModel",rowDoc));
			
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"FocusModel");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get focusModelId.
 	 * @return focusModelId as Tag
 	 */
 	Tag FocusModelRow::getFocusModelId() const {
	
  		return focusModelId;
 	}

 	/**
 	 * Set focusModelId with the specified Tag.
 	 * @param focusModelId The Tag value to which focusModelId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FocusModelRow::setFocusModelId (Tag focusModelId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("focusModelId", "FocusModel");
		
  		}
  	
 		this->focusModelId = focusModelId;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a FocusModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FocusModelRow::FocusModelRow (FocusModelTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	
	
	
	
	
	
	}
	
	FocusModelRow::FocusModelRow (FocusModelTable &t, FocusModelRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

			
		}
		else {
	
		
			focusModelId = row.focusModelId;
		
		
		
		
		
		
		
		}	
	}

	
	
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FocusModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool FocusModelRow::equalByRequiredValue(FocusModelRow* x) {
		
		return true;
	}	
	

} // End namespace asdm
 
