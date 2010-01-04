
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
 * File PointingModelRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <PointingModelRow.h>
#include <PointingModelTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::PointingModelRow;
using asdm::PointingModelTable;

using asdm::AntennaTable;
using asdm::AntennaRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	PointingModelRow::~PointingModelRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	PointingModelTable &PointingModelRow::getTable() const {
		return table;
	}
	
	void PointingModelRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a PointingModelRowIDL struct.
	 */
	PointingModelRowIDL *PointingModelRow::toIDL() const {
		PointingModelRowIDL *x = new PointingModelRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->pointingModelId = pointingModelId;
 				
 			
		
	

	
  		
		
		
			
				
		x->numCoeff = numCoeff;
 				
 			
		
	

	
  		
		
		
			
		x->coeffName.length(coeffName.size());
		for (unsigned int i = 0; i < coeffName.size(); ++i) {
			
				
			x->coeffName[i] = CORBA::string_dup(coeffName.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->coeffVal.length(coeffVal.size());
		for (unsigned int i = 0; i < coeffVal.size(); ++i) {
			
				
			x->coeffVal[i] = coeffVal.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->numFormulaExists = numFormulaExists;
		
		
			
				
		x->numFormula = numFormula;
 				
 			
		
	

	
  		
		
		x->coeffFormulaExists = coeffFormulaExists;
		
		
			
		x->coeffFormula.length(coeffFormula.size());
		for (unsigned int i = 0; i < coeffFormula.size(); ++i) {
			
				
			x->coeffFormula[i] = CORBA::string_dup(coeffFormula.at(i).c_str());
				
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct PointingModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void PointingModelRow::setFromIDL (PointingModelRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setPointingModelId(x.pointingModelId);
  			
 		
		
	

	
		
		
			
		setNumCoeff(x.numCoeff);
  			
 		
		
	

	
		
		
			
		coeffName .clear();
		for (unsigned int i = 0; i <x.coeffName.length(); ++i) {
			
			coeffName.push_back(string (x.coeffName[i]));
			
		}
			
  		
		
	

	
		
		
			
		coeffVal .clear();
		for (unsigned int i = 0; i <x.coeffVal.length(); ++i) {
			
			coeffVal.push_back(x.coeffVal[i]);
  			
		}
			
  		
		
	

	
		
		numFormulaExists = x.numFormulaExists;
		if (x.numFormulaExists) {
		
		
			
		setNumFormula(x.numFormula);
  			
 		
		
		}
		
	

	
		
		coeffFormulaExists = x.coeffFormulaExists;
		if (x.coeffFormulaExists) {
		
		
			
		coeffFormula .clear();
		for (unsigned int i = 0; i <x.coeffFormula.length(); ++i) {
			
			coeffFormula.push_back(string (x.coeffFormula[i]));
			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"PointingModel");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string PointingModelRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(pointingModelId, "pointingModelId", buf);
		
		
	

  	
 		
		
		Parser::toXML(numCoeff, "numCoeff", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffName, "coeffName", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffVal, "coeffVal", buf);
		
		
	

  	
 		
		if (numFormulaExists) {
		
		
		Parser::toXML(numFormula, "numFormula", buf);
		
		
		}
		
	

  	
 		
		if (coeffFormulaExists) {
		
		
		Parser::toXML(coeffFormula, "coeffFormula", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void PointingModelRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setPointingModelId(Parser::getInteger("pointingModelId","PointingModel",rowDoc));
			
		
	

	
  		
			
	  	setNumCoeff(Parser::getInteger("numCoeff","PointingModel",rowDoc));
			
		
	

	
  		
			
					
	  	setCoeffName(Parser::get1DString("coeffName","PointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffVal(Parser::get1DFloat("coeffVal","PointingModel",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<numFormula>")) {
			
	  		setNumFormula(Parser::getInteger("numFormula","PointingModel",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<coeffFormula>")) {
			
								
	  		setCoeffFormula(Parser::get1DString("coeffFormula","PointingModel",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"PointingModel");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get pointingModelId.
 	 * @return pointingModelId as int
 	 */
 	int PointingModelRow::getPointingModelId() const {
	
  		return pointingModelId;
 	}

 	/**
 	 * Set pointingModelId with the specified int.
 	 * @param pointingModelId The int value to which pointingModelId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void PointingModelRow::setPointingModelId (int pointingModelId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("pointingModelId", "PointingModel");
		
  		}
  	
 		this->pointingModelId = pointingModelId;
	
 	}
	
	

	

	
 	/**
 	 * Get numCoeff.
 	 * @return numCoeff as int
 	 */
 	int PointingModelRow::getNumCoeff() const {
	
  		return numCoeff;
 	}

 	/**
 	 * Set numCoeff with the specified int.
 	 * @param numCoeff The int value to which numCoeff is to be set.
 	 
 	
 		
 	 */
 	void PointingModelRow::setNumCoeff (int numCoeff)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numCoeff = numCoeff;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffName.
 	 * @return coeffName as vector<string >
 	 */
 	vector<string > PointingModelRow::getCoeffName() const {
	
  		return coeffName;
 	}

 	/**
 	 * Set coeffName with the specified vector<string >.
 	 * @param coeffName The vector<string > value to which coeffName is to be set.
 	 
 	
 		
 	 */
 	void PointingModelRow::setCoeffName (vector<string > coeffName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffName = coeffName;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffVal.
 	 * @return coeffVal as vector<float >
 	 */
 	vector<float > PointingModelRow::getCoeffVal() const {
	
  		return coeffVal;
 	}

 	/**
 	 * Set coeffVal with the specified vector<float >.
 	 * @param coeffVal The vector<float > value to which coeffVal is to be set.
 	 
 	
 		
 	 */
 	void PointingModelRow::setCoeffVal (vector<float > coeffVal)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffVal = coeffVal;
	
 	}
	
	

	
	/**
	 * The attribute numFormula is optional. Return true if this attribute exists.
	 * @return true if and only if the numFormula attribute exists. 
	 */
	bool PointingModelRow::isNumFormulaExists() const {
		return numFormulaExists;
	}
	

	
 	/**
 	 * Get numFormula, which is optional.
 	 * @return numFormula as int
 	 * @throw IllegalAccessException If numFormula does not exist.
 	 */
 	int PointingModelRow::getNumFormula() const throw(IllegalAccessException) {
		if (!numFormulaExists) {
			throw IllegalAccessException("numFormula", "PointingModel");
		}
	
  		return numFormula;
 	}

 	/**
 	 * Set numFormula with the specified int.
 	 * @param numFormula The int value to which numFormula is to be set.
 	 
 	
 	 */
 	void PointingModelRow::setNumFormula (int numFormula) {
	
 		this->numFormula = numFormula;
	
		numFormulaExists = true;
	
 	}
	
	
	/**
	 * Mark numFormula, which is an optional field, as non-existent.
	 */
	void PointingModelRow::clearNumFormula () {
		numFormulaExists = false;
	}
	

	
	/**
	 * The attribute coeffFormula is optional. Return true if this attribute exists.
	 * @return true if and only if the coeffFormula attribute exists. 
	 */
	bool PointingModelRow::isCoeffFormulaExists() const {
		return coeffFormulaExists;
	}
	

	
 	/**
 	 * Get coeffFormula, which is optional.
 	 * @return coeffFormula as vector<string >
 	 * @throw IllegalAccessException If coeffFormula does not exist.
 	 */
 	vector<string > PointingModelRow::getCoeffFormula() const throw(IllegalAccessException) {
		if (!coeffFormulaExists) {
			throw IllegalAccessException("coeffFormula", "PointingModel");
		}
	
  		return coeffFormula;
 	}

 	/**
 	 * Set coeffFormula with the specified vector<string >.
 	 * @param coeffFormula The vector<string > value to which coeffFormula is to be set.
 	 
 	
 	 */
 	void PointingModelRow::setCoeffFormula (vector<string > coeffFormula) {
	
 		this->coeffFormula = coeffFormula;
	
		coeffFormulaExists = true;
	
 	}
	
	
	/**
	 * Mark coeffFormula, which is an optional field, as non-existent.
	 */
	void PointingModelRow::clearCoeffFormula () {
		coeffFormulaExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag PointingModelRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void PointingModelRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "PointingModel");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* PointingModelRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	/**
	 * Create a PointingModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	PointingModelRow::PointingModelRow (PointingModelTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	
		numFormulaExists = false;
	

	
		coeffFormulaExists = false;
	

	
	

	
	
	
	

	

	

	

	

	
	
	}
	
	PointingModelRow::PointingModelRow (PointingModelTable &t, PointingModelRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	
		numFormulaExists = false;
	

	
		coeffFormulaExists = false;
	

	
	
		
		}
		else {
	
		
			pointingModelId = row.pointingModelId;
		
			antennaId = row.antennaId;
		
		
		
		
			numCoeff = row.numCoeff;
		
			coeffName = row.coeffName;
		
			coeffVal = row.coeffVal;
		
		
		
		
		if (row.numFormulaExists) {
			numFormula = row.numFormula;		
			numFormulaExists = true;
		}
		else
			numFormulaExists = false;
		
		if (row.coeffFormulaExists) {
			coeffFormula = row.coeffFormula;		
			coeffFormulaExists = true;
		}
		else
			coeffFormulaExists = false;
		
		}	
	}

	
	bool PointingModelRow::compareNoAutoInc(Tag antennaId, int numCoeff, vector<string > coeffName, vector<float > coeffVal) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numCoeff == numCoeff);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffName == coeffName);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffVal == coeffVal);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool PointingModelRow::compareRequiredValue(int numCoeff, vector<string > coeffName, vector<float > coeffVal) {
		bool result;
		result = true;
		
	
		if (!(this->numCoeff == numCoeff)) return false;
	

	
		if (!(this->coeffName == coeffName)) return false;
	

	
		if (!(this->coeffVal == coeffVal)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the PointingModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool PointingModelRow::equalByRequiredValue(PointingModelRow* x) {
		
			
		if (this->numCoeff != x->numCoeff) return false;
			
		if (this->coeffName != x->coeffName) return false;
			
		if (this->coeffVal != x->coeffVal) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
