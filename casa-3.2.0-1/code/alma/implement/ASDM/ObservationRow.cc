
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
 * File ObservationRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <ObservationRow.h>
#include <ObservationTable.h>
	

using asdm::ASDM;
using asdm::ObservationRow;
using asdm::ObservationTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	ObservationRow::~ObservationRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	ObservationTable &ObservationRow::getTable() const {
		return table;
	}

	bool ObservationRow::isAdded() const {
		return hasBeenAdded;
	}	

	void ObservationRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ObservationRowIDL struct.
	 */
	ObservationRowIDL *ObservationRow::toIDL() const {
		ObservationRowIDL *x = new ObservationRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->observationId = observationId.toIDLTag();
			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ObservationRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void ObservationRow::setFromIDL (ObservationRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setObservationId(Tag (x.observationId));
			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Observation");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string ObservationRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(observationId, "observationId", buf);
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void ObservationRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setObservationId(Parser::getTag("observationId","Observation",rowDoc));
			
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Observation");
		}
	}
	
	void ObservationRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	observationId.toBin(eoss);
		
	


	
	
	}
	
void ObservationRow::observationIdFromBin(EndianISStream& eiss) {
		
	
		
		
		observationId =  Tag::fromBin(eiss);
		
	
	
}

		
	
	ObservationRow* ObservationRow::fromBin(EndianISStream& eiss, ObservationTable& table, const vector<string>& attributesSeq) {
		ObservationRow* row = new  ObservationRow(table);
		
		map<string, ObservationAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "ObservationTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get observationId.
 	 * @return observationId as Tag
 	 */
 	Tag ObservationRow::getObservationId() const {
	
  		return observationId;
 	}

 	/**
 	 * Set observationId with the specified Tag.
 	 * @param observationId The Tag value to which observationId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void ObservationRow::setObservationId (Tag observationId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("observationId", "Observation");
		
  		}
  	
 		this->observationId = observationId;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a ObservationRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ObservationRow::ObservationRow (ObservationTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	
	
	
	
	

	
	
	 fromBinMethods["observationId"] = &ObservationRow::observationIdFromBin; 
		
	
	
	}
	
	ObservationRow::ObservationRow (ObservationTable &t, ObservationRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

			
		}
		else {
	
		
			observationId = row.observationId;
		
		
		
		
		
		
		
		}
		
		 fromBinMethods["observationId"] = &ObservationRow::observationIdFromBin; 
			
	
			
	}

	
	
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ObservationRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool ObservationRow::equalByRequiredValue(ObservationRow* x) {
		
		return true;
	}	
	
/*
	 map<string, ObservationAttributeFromBin> ObservationRow::initFromBinMethods() {
		map<string, ObservationAttributeFromBin> result;
		
		result["observationId"] = &ObservationRow::observationIdFromBin;
		
		
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
