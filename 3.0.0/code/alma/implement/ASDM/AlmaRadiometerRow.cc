
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

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>
	

using asdm::ASDM;
using asdm::AlmaRadiometerRow;
using asdm::AlmaRadiometerTable;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;


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
	
		
	
  		
		
		
			
		x->almaRadiometerId = almaRadiometerId.toIDLTag();
			
		
	

	
  		
		
		x->numAntennaExists = numAntennaExists;
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
	
		
	
  	
 		
 		
		x->spectralWindowIdExists = spectralWindowIdExists;
		
		
		
		x->spectralWindowId.length(spectralWindowId.size());
		for (unsigned int i = 0; i < spectralWindowId.size(); ++i) {
			
			x->spectralWindowId[i] = spectralWindowId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct AlmaRadiometerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void AlmaRadiometerRow::setFromIDL (AlmaRadiometerRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAlmaRadiometerId(Tag (x.almaRadiometerId));
			
 		
		
	

	
		
		numAntennaExists = x.numAntennaExists;
		if (x.numAntennaExists) {
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
		}
		
	

	
	
		
	
		
		spectralWindowIdExists = x.spectralWindowIdExists;
		if (x.spectralWindowIdExists) {
		
		spectralWindowId .clear();
		for (unsigned int i = 0; i <x.spectralWindowId.length(); ++i) {
			
			spectralWindowId.push_back(Tag (x.spectralWindowId[i]));
			
		}
		
		}
		
  	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"AlmaRadiometer");
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
		
	
		
  	
 		
		
		Parser::toXML(almaRadiometerId, "almaRadiometerId", buf);
		
		
	

  	
 		
		if (numAntennaExists) {
		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
		}
		
	

	
	
		
  	
 		
		if (spectralWindowIdExists) {
		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
		}
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void AlmaRadiometerRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAlmaRadiometerId(Parser::getTag("almaRadiometerId","AlmaRadiometer",rowDoc));
			
		
	

	
  		
        if (row.isStr("<numAntenna>")) {
			
	  		setNumAntenna(Parser::getInteger("numAntenna","AlmaRadiometer",rowDoc));
			
		}
 		
	

	
	
		
	
  		
  		if (row.isStr("<spectralWindowId>")) {
  			setSpectralWindowId(Parser::get1DTag("spectralWindowId","AlmaRadiometer",rowDoc));  		
  		}
  		
  	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"AlmaRadiometer");
		}
	}
	
	void AlmaRadiometerRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	almaRadiometerId.toBin(eoss);
		
	


	
	
	eoss.writeBoolean(numAntennaExists);
	if (numAntennaExists) {
	
	
	
		
						
			eoss.writeInt(numAntenna);
				
		
	

	}

	eoss.writeBoolean(spectralWindowIdExists);
	if (spectralWindowIdExists) {
	
	
	
		
	Tag::toBin(spectralWindowId, eoss);
		
	

	}

	}
	
	AlmaRadiometerRow* AlmaRadiometerRow::fromBin(EndianISStream& eiss, AlmaRadiometerTable& table) {
		AlmaRadiometerRow* row = new  AlmaRadiometerRow(table);
		
		
		
	
		
		
		row->almaRadiometerId =  Tag::fromBin(eiss);
		
	

		
		
		
	row->numAntennaExists = eiss.readBoolean();
	if (row->numAntennaExists) {
		
	
	
		
			
		row->numAntenna =  eiss.readInt();
			
		
	

	}

	row->spectralWindowIdExists = eiss.readBoolean();
	if (row->spectralWindowIdExists) {
		
	
		
		
			
	
	row->spectralWindowId = Tag::from1DBin(eiss);	
	

		
	

	}

		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get almaRadiometerId.
 	 * @return almaRadiometerId as Tag
 	 */
 	Tag AlmaRadiometerRow::getAlmaRadiometerId() const {
	
  		return almaRadiometerId;
 	}

 	/**
 	 * Set almaRadiometerId with the specified Tag.
 	 * @param almaRadiometerId The Tag value to which almaRadiometerId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void AlmaRadiometerRow::setAlmaRadiometerId (Tag almaRadiometerId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("almaRadiometerId", "AlmaRadiometer");
		
  		}
  	
 		this->almaRadiometerId = almaRadiometerId;
	
 	}
	
	

	
	/**
	 * The attribute numAntenna is optional. Return true if this attribute exists.
	 * @return true if and only if the numAntenna attribute exists. 
	 */
	bool AlmaRadiometerRow::isNumAntennaExists() const {
		return numAntennaExists;
	}
	

	
 	/**
 	 * Get numAntenna, which is optional.
 	 * @return numAntenna as int
 	 * @throw IllegalAccessException If numAntenna does not exist.
 	 */
 	int AlmaRadiometerRow::getNumAntenna() const  {
		if (!numAntennaExists) {
			throw IllegalAccessException("numAntenna", "AlmaRadiometer");
		}
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 	 */
 	void AlmaRadiometerRow::setNumAntenna (int numAntenna) {
	
 		this->numAntenna = numAntenna;
	
		numAntennaExists = true;
	
 	}
	
	
	/**
	 * Mark numAntenna, which is an optional field, as non-existent.
	 */
	void AlmaRadiometerRow::clearNumAntenna () {
		numAntennaExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	/**
	 * The attribute spectralWindowId is optional. Return true if this attribute exists.
	 * @return true if and only if the spectralWindowId attribute exists. 
	 */
	bool AlmaRadiometerRow::isSpectralWindowIdExists() const {
		return spectralWindowIdExists;
	}
	

	
 	/**
 	 * Get spectralWindowId, which is optional.
 	 * @return spectralWindowId as vector<Tag> 
 	 * @throw IllegalAccessException If spectralWindowId does not exist.
 	 */
 	vector<Tag>  AlmaRadiometerRow::getSpectralWindowId() const  {
		if (!spectralWindowIdExists) {
			throw IllegalAccessException("spectralWindowId", "AlmaRadiometer");
		}
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified vector<Tag> .
 	 * @param spectralWindowId The vector<Tag>  value to which spectralWindowId is to be set.
 	 
 	
 	 */
 	void AlmaRadiometerRow::setSpectralWindowId (vector<Tag>  spectralWindowId) {
	
 		this->spectralWindowId = spectralWindowId;
	
		spectralWindowIdExists = true;
	
 	}
	
	
	/**
	 * Mark spectralWindowId, which is an optional field, as non-existent.
	 */
	void AlmaRadiometerRow::clearSpectralWindowId () {
		spectralWindowIdExists = false;
	}
	

	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set spectralWindowId[i] with the specified Tag.
 	 * @param i The index in spectralWindowId where to set the Tag value.
 	 * @param spectralWindowId The Tag value to which spectralWindowId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void AlmaRadiometerRow::setSpectralWindowId (int i, Tag spectralWindowId) {
  		if ((i < 0) || (i > ((int) this->spectralWindowId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute spectralWindowId in table AlmaRadiometerTable");
  		vector<Tag> ::iterator iter = this->spectralWindowId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->spectralWindowId.insert(this->spectralWindowId.erase(iter), spectralWindowId); 	
  	}
 			
	
	
	
		
/**
 * Append a Tag to spectralWindowId.
 * @param id the Tag to be appended to spectralWindowId
 */
 void AlmaRadiometerRow::addSpectralWindowId(Tag id){
 	spectralWindowId.push_back(id);
}

/**
 * Append an array of Tag to spectralWindowId.
 * @param id an array of Tag to be appended to spectralWindowId
 */
 void AlmaRadiometerRow::addSpectralWindowId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		spectralWindowId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in spectralWindowId at position i.
  *
  */
 const Tag AlmaRadiometerRow::getSpectralWindowId(int i) {
 	return spectralWindowId.at(i);
 }
 
 /**
  * Returns the SpectralWindowRow linked to this row via the Tag stored in spectralWindowId
  * at position i.
  */
 SpectralWindowRow* AlmaRadiometerRow::getSpectralWindow(int i) {
 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId.at(i));
 } 
 
 /**
  * Returns the vector of SpectralWindowRow* linked to this row via the Tags stored in spectralWindowId
  *
  */
 vector<SpectralWindowRow *> AlmaRadiometerRow::getSpectralWindows() {
 	vector<SpectralWindowRow *> result;
 	for (unsigned int i = 0; i < spectralWindowId.size(); i++)
 		result.push_back(table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId.at(i)));
 		
 	return result;
 }
  

	

	
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
		
	
	

	
		numAntennaExists = false;
	

	
	
		spectralWindowIdExists = false;
	

	
	
	
	

	
	
	}
	
	AlmaRadiometerRow::AlmaRadiometerRow (AlmaRadiometerTable &t, AlmaRadiometerRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	
		numAntennaExists = false;
	

	
	
		spectralWindowIdExists = false;
	
		
		}
		else {
	
		
			almaRadiometerId = row.almaRadiometerId;
		
		
		
		
		
		
		
		if (row.numAntennaExists) {
			numAntenna = row.numAntenna;		
			numAntennaExists = true;
		}
		else
			numAntennaExists = false;
		
		if (row.spectralWindowIdExists) {
			spectralWindowId = row.spectralWindowId;		
			spectralWindowIdExists = true;
		}
		else
			spectralWindowIdExists = false;
		
		}	
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
		
		return true;
	}	
	

} // End namespace asdm
 
