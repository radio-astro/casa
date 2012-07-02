
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
 * File PolarizationRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <PolarizationRow.h>
#include <PolarizationTable.h>
	

using asdm::ASDM;
using asdm::PolarizationRow;
using asdm::PolarizationTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	PolarizationRow::~PolarizationRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	PolarizationTable &PolarizationRow::getTable() const {
		return table;
	}
	
	void PolarizationRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a PolarizationRowIDL struct.
	 */
	PolarizationRowIDL *PolarizationRow::toIDL() const {
		PolarizationRowIDL *x = new PolarizationRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->polarizationId = polarizationId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->numCorr = numCorr;
 				
 			
		
	

	
  		
		
		
			
		x->corrType.length(corrType.size());
		for (unsigned int i = 0; i < corrType.size(); ++i) {
			
				
			x->corrType[i] = corrType.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->corrProduct.length(corrProduct.size());
		for (unsigned int i = 0; i < corrProduct.size(); i++) {
			x->corrProduct[i].length(corrProduct.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < corrProduct.size() ; i++)
			for (unsigned int j = 0; j < corrProduct.at(i).size(); j++)
					
						
				x->corrProduct[i][j] = corrProduct.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->flagRowExists = flagRowExists;
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct PolarizationRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void PolarizationRow::setFromIDL (PolarizationRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setPolarizationId(Tag (x.polarizationId));
			
 		
		
	

	
		
		
			
		setNumCorr(x.numCorr);
  			
 		
		
	

	
		
		
			
		corrType .clear();
		for (unsigned int i = 0; i <x.corrType.length(); ++i) {
			
			corrType.push_back(x.corrType[i]);
  			
		}
			
  		
		
	

	
		
		
			
		corrProduct .clear();
		vector<PolarizationType> v_aux_corrProduct;
		for (unsigned int i = 0; i < x.corrProduct.length(); ++i) {
			v_aux_corrProduct.clear();
			for (unsigned int j = 0; j < x.corrProduct[0].length(); ++j) {
				
				v_aux_corrProduct.push_back(x.corrProduct[i][j]);
	  			
  			}
  			corrProduct.push_back(v_aux_corrProduct);			
		}
			
  		
		
	

	
		
		flagRowExists = x.flagRowExists;
		if (x.flagRowExists) {
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
		}
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Polarization");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string PolarizationRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(polarizationId, "polarizationId", buf);
		
		
	

  	
 		
		
		Parser::toXML(numCorr, "numCorr", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("corrType", corrType));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("corrProduct", corrProduct));
		
		
	

  	
 		
		if (flagRowExists) {
		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
		}
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void PolarizationRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setPolarizationId(Parser::getTag("polarizationId","Polarization",rowDoc));
			
		
	

	
  		
			
	  	setNumCorr(Parser::getInteger("numCorr","Polarization",rowDoc));
			
		
	

	
		
		
		
		corrType = EnumerationParser::getStokesParameter1D("corrType","Polarization",rowDoc);			
		
		
		
	

	
		
		
		
		corrProduct = EnumerationParser::getPolarizationType2D("corrProduct","Polarization",rowDoc);			
		
		
		
	

	
  		
        if (row.isStr("<flagRow>")) {
			
	  		setFlagRow(Parser::getBoolean("flagRow","Polarization",rowDoc));
			
		}
 		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Polarization");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get polarizationId.
 	 * @return polarizationId as Tag
 	 */
 	Tag PolarizationRow::getPolarizationId() const {
	
  		return polarizationId;
 	}

 	/**
 	 * Set polarizationId with the specified Tag.
 	 * @param polarizationId The Tag value to which polarizationId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void PolarizationRow::setPolarizationId (Tag polarizationId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("polarizationId", "Polarization");
		
  		}
  	
 		this->polarizationId = polarizationId;
	
 	}
	
	

	

	
 	/**
 	 * Get numCorr.
 	 * @return numCorr as int
 	 */
 	int PolarizationRow::getNumCorr() const {
	
  		return numCorr;
 	}

 	/**
 	 * Set numCorr with the specified int.
 	 * @param numCorr The int value to which numCorr is to be set.
 	 
 	
 		
 	 */
 	void PolarizationRow::setNumCorr (int numCorr)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numCorr = numCorr;
	
 	}
	
	

	

	
 	/**
 	 * Get corrType.
 	 * @return corrType as vector<StokesParameterMod::StokesParameter >
 	 */
 	vector<StokesParameterMod::StokesParameter > PolarizationRow::getCorrType() const {
	
  		return corrType;
 	}

 	/**
 	 * Set corrType with the specified vector<StokesParameterMod::StokesParameter >.
 	 * @param corrType The vector<StokesParameterMod::StokesParameter > value to which corrType is to be set.
 	 
 	
 		
 	 */
 	void PolarizationRow::setCorrType (vector<StokesParameterMod::StokesParameter > corrType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->corrType = corrType;
	
 	}
	
	

	

	
 	/**
 	 * Get corrProduct.
 	 * @return corrProduct as vector<vector<PolarizationTypeMod::PolarizationType > >
 	 */
 	vector<vector<PolarizationTypeMod::PolarizationType > > PolarizationRow::getCorrProduct() const {
	
  		return corrProduct;
 	}

 	/**
 	 * Set corrProduct with the specified vector<vector<PolarizationTypeMod::PolarizationType > >.
 	 * @param corrProduct The vector<vector<PolarizationTypeMod::PolarizationType > > value to which corrProduct is to be set.
 	 
 	
 		
 	 */
 	void PolarizationRow::setCorrProduct (vector<vector<PolarizationTypeMod::PolarizationType > > corrProduct)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->corrProduct = corrProduct;
	
 	}
	
	

	
	/**
	 * The attribute flagRow is optional. Return true if this attribute exists.
	 * @return true if and only if the flagRow attribute exists. 
	 */
	bool PolarizationRow::isFlagRowExists() const {
		return flagRowExists;
	}
	

	
 	/**
 	 * Get flagRow, which is optional.
 	 * @return flagRow as bool
 	 * @throw IllegalAccessException If flagRow does not exist.
 	 */
 	bool PolarizationRow::getFlagRow() const throw(IllegalAccessException) {
		if (!flagRowExists) {
			throw IllegalAccessException("flagRow", "Polarization");
		}
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 	 */
 	void PolarizationRow::setFlagRow (bool flagRow) {
	
 		this->flagRow = flagRow;
	
		flagRowExists = true;
	
 	}
	
	
	/**
	 * Mark flagRow, which is an optional field, as non-existent.
	 */
	void PolarizationRow::clearFlagRow () {
		flagRowExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a PolarizationRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	PolarizationRow::PolarizationRow (PolarizationTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	
		flagRowExists = false;
	

	
	
	
	
	

	

	

	

	
	
	}
	
	PolarizationRow::PolarizationRow (PolarizationTable &t, PolarizationRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	
		flagRowExists = false;
	

			
		}
		else {
	
		
			polarizationId = row.polarizationId;
		
		
		
		
			numCorr = row.numCorr;
		
			corrType = row.corrType;
		
			corrProduct = row.corrProduct;
		
		
		
		
		if (row.flagRowExists) {
			flagRow = row.flagRow;		
			flagRowExists = true;
		}
		else
			flagRowExists = false;
		
		}	
	}

	
	bool PolarizationRow::compareNoAutoInc(int numCorr, vector<StokesParameterMod::StokesParameter > corrType, vector<vector<PolarizationTypeMod::PolarizationType > > corrProduct) {
		bool result;
		result = true;
		
	
		
		result = result && (this->numCorr == numCorr);
		
		if (!result) return false;
	

	
		
		result = result && (this->corrType == corrType);
		
		if (!result) return false;
	

	
		
		result = result && (this->corrProduct == corrProduct);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool PolarizationRow::compareRequiredValue(int numCorr, vector<StokesParameterMod::StokesParameter > corrType, vector<vector<PolarizationTypeMod::PolarizationType > > corrProduct) {
		bool result;
		result = true;
		
	
		if (!(this->numCorr == numCorr)) return false;
	

	
		if (!(this->corrType == corrType)) return false;
	

	
		if (!(this->corrProduct == corrProduct)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the PolarizationRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool PolarizationRow::equalByRequiredValue(PolarizationRow* x) {
		
			
		if (this->numCorr != x->numCorr) return false;
			
		if (this->corrType != x->corrType) return false;
			
		if (this->corrProduct != x->corrProduct) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
