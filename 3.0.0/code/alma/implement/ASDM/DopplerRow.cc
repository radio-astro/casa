
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
 * File DopplerRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <DopplerRow.h>
#include <DopplerTable.h>

#include <SourceTable.h>
#include <SourceRow.h>
	

using asdm::ASDM;
using asdm::DopplerRow;
using asdm::DopplerTable;

using asdm::SourceTable;
using asdm::SourceRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	DopplerRow::~DopplerRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	DopplerTable &DopplerRow::getTable() const {
		return table;
	}
	
	void DopplerRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a DopplerRowIDL struct.
	 */
	DopplerRowIDL *DopplerRow::toIDL() const {
		DopplerRowIDL *x = new DopplerRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->dopplerId = dopplerId;
 				
 			
		
	

	
  		
		
		
			
				
		x->transitionIndex = transitionIndex;
 				
 			
		
	

	
  		
		
		
			
				
		x->velDef = velDef;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
				
		x->sourceId = sourceId;
 				
 			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct DopplerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void DopplerRow::setFromIDL (DopplerRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setDopplerId(x.dopplerId);
  			
 		
		
	

	
		
		
			
		setTransitionIndex(x.transitionIndex);
  			
 		
		
	

	
		
		
			
		setVelDef(x.velDef);
  			
 		
		
	

	
	
		
	
		
		
			
		setSourceId(x.sourceId);
  			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Doppler");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string DopplerRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(dopplerId, "dopplerId", buf);
		
		
	

  	
 		
		
		Parser::toXML(transitionIndex, "transitionIndex", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("velDef", velDef));
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(sourceId, "sourceId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void DopplerRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setDopplerId(Parser::getInteger("dopplerId","Doppler",rowDoc));
			
		
	

	
  		
			
	  	setTransitionIndex(Parser::getInteger("transitionIndex","Doppler",rowDoc));
			
		
	

	
		
		
		
		velDef = EnumerationParser::getDopplerReferenceCode("velDef","Doppler",rowDoc);
		
		
		
	

	
	
		
	
  		
			
	  	setSourceId(Parser::getInteger("sourceId","Source",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Doppler");
		}
	}
	
	void DopplerRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeInt(dopplerId);
				
		
	

	
	
		
						
			eoss.writeInt(sourceId);
				
		
	

	
	
		
						
			eoss.writeInt(transitionIndex);
				
		
	

	
	
		
					
			eoss.writeInt(velDef);
				
		
	


	
	
	}
	
	DopplerRow* DopplerRow::fromBin(EndianISStream& eiss, DopplerTable& table) {
		DopplerRow* row = new  DopplerRow(table);
		
		
		
	
	
		
			
		row->dopplerId =  eiss.readInt();
			
		
	

	
	
		
			
		row->sourceId =  eiss.readInt();
			
		
	

	
	
		
			
		row->transitionIndex =  eiss.readInt();
			
		
	

	
	
		
			
		row->velDef = CDopplerReferenceCode::from_int(eiss.readInt());
			
		
	

		
		
		
		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get dopplerId.
 	 * @return dopplerId as int
 	 */
 	int DopplerRow::getDopplerId() const {
	
  		return dopplerId;
 	}

 	/**
 	 * Set dopplerId with the specified int.
 	 * @param dopplerId The int value to which dopplerId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void DopplerRow::setDopplerId (int dopplerId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("dopplerId", "Doppler");
		
  		}
  	
 		this->dopplerId = dopplerId;
	
 	}
	
	

	

	
 	/**
 	 * Get transitionIndex.
 	 * @return transitionIndex as int
 	 */
 	int DopplerRow::getTransitionIndex() const {
	
  		return transitionIndex;
 	}

 	/**
 	 * Set transitionIndex with the specified int.
 	 * @param transitionIndex The int value to which transitionIndex is to be set.
 	 
 	
 		
 	 */
 	void DopplerRow::setTransitionIndex (int transitionIndex)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->transitionIndex = transitionIndex;
	
 	}
	
	

	

	
 	/**
 	 * Get velDef.
 	 * @return velDef as DopplerReferenceCodeMod::DopplerReferenceCode
 	 */
 	DopplerReferenceCodeMod::DopplerReferenceCode DopplerRow::getVelDef() const {
	
  		return velDef;
 	}

 	/**
 	 * Set velDef with the specified DopplerReferenceCodeMod::DopplerReferenceCode.
 	 * @param velDef The DopplerReferenceCodeMod::DopplerReferenceCode value to which velDef is to be set.
 	 
 	
 		
 	 */
 	void DopplerRow::setVelDef (DopplerReferenceCodeMod::DopplerReferenceCode velDef)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->velDef = velDef;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get sourceId.
 	 * @return sourceId as int
 	 */
 	int DopplerRow::getSourceId() const {
	
  		return sourceId;
 	}

 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void DopplerRow::setSourceId (int sourceId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("sourceId", "Doppler");
		
  		}
  	
 		this->sourceId = sourceId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	// ===> Slice link from a row of Doppler table to a collection of row of Source table.
	
	/**
	 * Get the collection of row in the Source table having their attribut sourceId == this->sourceId
	 */
	vector <SourceRow *> DopplerRow::getSources() {
		
			return table.getContainer().getSource().getRowBySourceId(sourceId);
		
	}
	

	

	
	/**
	 * Create a DopplerRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	DopplerRow::DopplerRow (DopplerTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	
	

	
	
	
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
velDef = CDopplerReferenceCode::from_int(0);
	
	
	}
	
	DopplerRow::DopplerRow (DopplerTable &t, DopplerRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	
	
		
		}
		else {
	
		
			dopplerId = row.dopplerId;
		
			sourceId = row.sourceId;
		
		
		
		
			transitionIndex = row.transitionIndex;
		
			velDef = row.velDef;
		
		
		
		
		}	
	}

	
	bool DopplerRow::compareNoAutoInc(int sourceId, int transitionIndex, DopplerReferenceCodeMod::DopplerReferenceCode velDef) {
		bool result;
		result = true;
		
	
		
		result = result && (this->sourceId == sourceId);
		
		if (!result) return false;
	

	
		
		result = result && (this->transitionIndex == transitionIndex);
		
		if (!result) return false;
	

	
		
		result = result && (this->velDef == velDef);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool DopplerRow::compareRequiredValue(int transitionIndex, DopplerReferenceCodeMod::DopplerReferenceCode velDef) {
		bool result;
		result = true;
		
	
		if (!(this->transitionIndex == transitionIndex)) return false;
	

	
		if (!(this->velDef == velDef)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the DopplerRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool DopplerRow::equalByRequiredValue(DopplerRow* x) {
		
			
		if (this->transitionIndex != x->transitionIndex) return false;
			
		if (this->velDef != x->velDef) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
