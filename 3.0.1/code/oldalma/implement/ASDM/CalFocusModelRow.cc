
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
 * File CalFocusModelRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalFocusModelRow.h>
#include <CalFocusModelTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalFocusModelRow;
using asdm::CalFocusModelTable;

using asdm::CalDataTable;
using asdm::CalDataRow;

using asdm::CalReductionTable;
using asdm::CalReductionRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	CalFocusModelRow::~CalFocusModelRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalFocusModelTable &CalFocusModelRow::getTable() const {
		return table;
	}
	
	void CalFocusModelRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalFocusModelRowIDL struct.
	 */
	CalFocusModelRowIDL *CalFocusModelRow::toIDL() const {
		CalFocusModelRowIDL *x = new CalFocusModelRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numCoeff = numCoeff;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->focusRMS.length(focusRMS.size());
		for (unsigned int i = 0; i < focusRMS.size(); ++i) {
			
			x->focusRMS[i] = focusRMS.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->coeffName.length(coeffName.size());
		for (unsigned int i = 0; i < coeffName.size(); i++) {
			x->coeffName[i].length(coeffName.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < coeffName.size() ; i++)
			for (unsigned int j = 0; j < coeffName.at(i).size(); j++)
					
						
				x->coeffName[i][j] = CORBA::string_dup(coeffName.at(i).at(j).c_str());
						
			 						
		
			
		
	

	
  		
		
		
			
		x->coeffFormula.length(coeffFormula.size());
		for (unsigned int i = 0; i < coeffFormula.size(); i++) {
			x->coeffFormula[i].length(coeffFormula.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < coeffFormula.size() ; i++)
			for (unsigned int j = 0; j < coeffFormula.at(i).size(); j++)
					
						
				x->coeffFormula[i][j] = CORBA::string_dup(coeffFormula.at(i).at(j).c_str());
						
			 						
		
			
		
	

	
  		
		
		
			
		x->coeffValue.length(coeffValue.size());
		for (unsigned int i = 0; i < coeffValue.size(); i++) {
			x->coeffValue[i].length(coeffValue.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < coeffValue.size() ; i++)
			for (unsigned int j = 0; j < coeffValue.at(i).size(); j++)
					
						
				x->coeffValue[i][j] = coeffValue.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->coeffError.length(coeffError.size());
		for (unsigned int i = 0; i < coeffError.size(); i++) {
			x->coeffError[i].length(coeffError.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < coeffError.size() ; i++)
			for (unsigned int j = 0; j < coeffError.at(i).size(); j++)
					
						
				x->coeffError[i][j] = coeffError.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->coeffFixed.length(coeffFixed.size());
		for (unsigned int i = 0; i < coeffFixed.size(); i++) {
			x->coeffFixed[i].length(coeffFixed.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < coeffFixed.size() ; i++)
			for (unsigned int j = 0; j < coeffFixed.at(i).size(); j++)
					
						
				x->coeffFixed[i][j] = coeffFixed.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
				
		x->focusModel = CORBA::string_dup(focusModel.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numSourceObs = numSourceObs;
 				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
				
		x->antennaMake = antennaMake;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalFocusModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalFocusModelRow::setFromIDL (CalFocusModelRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setNumCoeff(x.numCoeff);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		focusRMS .clear();
		for (unsigned int i = 0; i <x.focusRMS.length(); ++i) {
			
			focusRMS.push_back(Length (x.focusRMS[i]));
			
		}
			
  		
		
	

	
		
		
			
		coeffName .clear();
		vector<string> v_aux_coeffName;
		for (unsigned int i = 0; i < x.coeffName.length(); ++i) {
			v_aux_coeffName.clear();
			for (unsigned int j = 0; j < x.coeffName[0].length(); ++j) {
				
				v_aux_coeffName.push_back(string (x.coeffName[i][j]));
				
  			}
  			coeffName.push_back(v_aux_coeffName);			
		}
			
  		
		
	

	
		
		
			
		coeffFormula .clear();
		vector<string> v_aux_coeffFormula;
		for (unsigned int i = 0; i < x.coeffFormula.length(); ++i) {
			v_aux_coeffFormula.clear();
			for (unsigned int j = 0; j < x.coeffFormula[0].length(); ++j) {
				
				v_aux_coeffFormula.push_back(string (x.coeffFormula[i][j]));
				
  			}
  			coeffFormula.push_back(v_aux_coeffFormula);			
		}
			
  		
		
	

	
		
		
			
		coeffValue .clear();
		vector<float> v_aux_coeffValue;
		for (unsigned int i = 0; i < x.coeffValue.length(); ++i) {
			v_aux_coeffValue.clear();
			for (unsigned int j = 0; j < x.coeffValue[0].length(); ++j) {
				
				v_aux_coeffValue.push_back(x.coeffValue[i][j]);
	  			
  			}
  			coeffValue.push_back(v_aux_coeffValue);			
		}
			
  		
		
	

	
		
		
			
		coeffError .clear();
		vector<float> v_aux_coeffError;
		for (unsigned int i = 0; i < x.coeffError.length(); ++i) {
			v_aux_coeffError.clear();
			for (unsigned int j = 0; j < x.coeffError[0].length(); ++j) {
				
				v_aux_coeffError.push_back(x.coeffError[i][j]);
	  			
  			}
  			coeffError.push_back(v_aux_coeffError);			
		}
			
  		
		
	

	
		
		
			
		coeffFixed .clear();
		vector<bool> v_aux_coeffFixed;
		for (unsigned int i = 0; i < x.coeffFixed.length(); ++i) {
			v_aux_coeffFixed.clear();
			for (unsigned int j = 0; j < x.coeffFixed[0].length(); ++j) {
				
				v_aux_coeffFixed.push_back(x.coeffFixed[i][j]);
	  			
  			}
  			coeffFixed.push_back(v_aux_coeffFixed);			
		}
			
  		
		
	

	
		
		
			
		setFocusModel(string (x.focusModel));
			
 		
		
	

	
		
		
			
		setNumSourceObs(x.numSourceObs);
  			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setAntennaMake(x.antennaMake);
  			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalFocusModel");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalFocusModelRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(numCoeff, "numCoeff", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusRMS, "focusRMS", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffName, "coeffName", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffFormula, "coeffFormula", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffValue, "coeffValue", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffError, "coeffError", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffFixed, "coeffFixed", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusModel, "focusModel", buf);
		
		
	

  	
 		
		
		Parser::toXML(numSourceObs, "numSourceObs", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("antennaMake", antennaMake));
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(calDataId, "calDataId", buf);
		
		
	

  	
 		
		
		Parser::toXML(calReductionId, "calReductionId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CalFocusModelRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalFocusModel",rowDoc));
			
		
	

	
  		
			
	  	setNumCoeff(Parser::getInteger("numCoeff","CalFocusModel",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalFocusModel",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalFocusModel",rowDoc));
			
		
	

	
  		
			
					
	  	setFocusRMS(Parser::get1DLength("focusRMS","CalFocusModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffName(Parser::get2DString("coeffName","CalFocusModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffFormula(Parser::get2DString("coeffFormula","CalFocusModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffValue(Parser::get2DFloat("coeffValue","CalFocusModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffError(Parser::get2DFloat("coeffError","CalFocusModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffFixed(Parser::get2DBoolean("coeffFixed","CalFocusModel",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setFocusModel(Parser::getString("focusModel","CalFocusModel",rowDoc));
			
		
	

	
  		
			
	  	setNumSourceObs(Parser::getInteger("numSourceObs","CalFocusModel",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalFocusModel",rowDoc);
		
		
		
	

	
		
		
		
		antennaMake = EnumerationParser::getAntennaMake("antennaMake","CalFocusModel",rowDoc);
		
		
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalFocusModel");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalFocusModelRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusModelRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalFocusModel");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get numCoeff.
 	 * @return numCoeff as int
 	 */
 	int CalFocusModelRow::getNumCoeff() const {
	
  		return numCoeff;
 	}

 	/**
 	 * Set numCoeff with the specified int.
 	 * @param numCoeff The int value to which numCoeff is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setNumCoeff (int numCoeff)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numCoeff = numCoeff;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalFocusModelRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalFocusModelRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get focusRMS.
 	 * @return focusRMS as vector<Length >
 	 */
 	vector<Length > CalFocusModelRow::getFocusRMS() const {
	
  		return focusRMS;
 	}

 	/**
 	 * Set focusRMS with the specified vector<Length >.
 	 * @param focusRMS The vector<Length > value to which focusRMS is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setFocusRMS (vector<Length > focusRMS)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusRMS = focusRMS;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffName.
 	 * @return coeffName as vector<vector<string > >
 	 */
 	vector<vector<string > > CalFocusModelRow::getCoeffName() const {
	
  		return coeffName;
 	}

 	/**
 	 * Set coeffName with the specified vector<vector<string > >.
 	 * @param coeffName The vector<vector<string > > value to which coeffName is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setCoeffName (vector<vector<string > > coeffName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffName = coeffName;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffFormula.
 	 * @return coeffFormula as vector<vector<string > >
 	 */
 	vector<vector<string > > CalFocusModelRow::getCoeffFormula() const {
	
  		return coeffFormula;
 	}

 	/**
 	 * Set coeffFormula with the specified vector<vector<string > >.
 	 * @param coeffFormula The vector<vector<string > > value to which coeffFormula is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setCoeffFormula (vector<vector<string > > coeffFormula)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffFormula = coeffFormula;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffValue.
 	 * @return coeffValue as vector<vector<float > >
 	 */
 	vector<vector<float > > CalFocusModelRow::getCoeffValue() const {
	
  		return coeffValue;
 	}

 	/**
 	 * Set coeffValue with the specified vector<vector<float > >.
 	 * @param coeffValue The vector<vector<float > > value to which coeffValue is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setCoeffValue (vector<vector<float > > coeffValue)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffValue = coeffValue;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffError.
 	 * @return coeffError as vector<vector<float > >
 	 */
 	vector<vector<float > > CalFocusModelRow::getCoeffError() const {
	
  		return coeffError;
 	}

 	/**
 	 * Set coeffError with the specified vector<vector<float > >.
 	 * @param coeffError The vector<vector<float > > value to which coeffError is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setCoeffError (vector<vector<float > > coeffError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffError = coeffError;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffFixed.
 	 * @return coeffFixed as vector<vector<bool > >
 	 */
 	vector<vector<bool > > CalFocusModelRow::getCoeffFixed() const {
	
  		return coeffFixed;
 	}

 	/**
 	 * Set coeffFixed with the specified vector<vector<bool > >.
 	 * @param coeffFixed The vector<vector<bool > > value to which coeffFixed is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setCoeffFixed (vector<vector<bool > > coeffFixed)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffFixed = coeffFixed;
	
 	}
	
	

	

	
 	/**
 	 * Get focusModel.
 	 * @return focusModel as string
 	 */
 	string CalFocusModelRow::getFocusModel() const {
	
  		return focusModel;
 	}

 	/**
 	 * Set focusModel with the specified string.
 	 * @param focusModel The string value to which focusModel is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setFocusModel (string focusModel)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusModel = focusModel;
	
 	}
	
	

	

	
 	/**
 	 * Get numSourceObs.
 	 * @return numSourceObs as int
 	 */
 	int CalFocusModelRow::getNumSourceObs() const {
	
  		return numSourceObs;
 	}

 	/**
 	 * Set numSourceObs with the specified int.
 	 * @param numSourceObs The int value to which numSourceObs is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setNumSourceObs (int numSourceObs)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numSourceObs = numSourceObs;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalFocusModelRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusModelRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalFocusModel");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake CalFocusModelRow::getAntennaMake() const {
	
  		return antennaMake;
 	}

 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 	
 		
 	 */
 	void CalFocusModelRow::setAntennaMake (AntennaMakeMod::AntennaMake antennaMake)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaMake = antennaMake;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalFocusModelRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusModelRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalFocusModel");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalFocusModelRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalFocusModelRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalFocusModel");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalFocusModelRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalFocusModelRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalFocusModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalFocusModelRow::CalFocusModelRow (CalFocusModelTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
antennaMake = CAntennaMake::from_int(0);
	
	
	}
	
	CalFocusModelRow::CalFocusModelRow (CalFocusModelTable &t, CalFocusModelRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
			receiverBand = row.receiverBand;
		
		
		
		
			numCoeff = row.numCoeff;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			focusRMS = row.focusRMS;
		
			coeffName = row.coeffName;
		
			coeffFormula = row.coeffFormula;
		
			coeffValue = row.coeffValue;
		
			coeffError = row.coeffError;
		
			coeffFixed = row.coeffFixed;
		
			focusModel = row.focusModel;
		
			numSourceObs = row.numSourceObs;
		
			antennaMake = row.antennaMake;
		
		
		
		
		}	
	}

	
	bool CalFocusModelRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, ReceiverBandMod::ReceiverBand receiverBand, int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusRMS, vector<vector<string > > coeffName, vector<vector<string > > coeffFormula, vector<vector<float > > coeffValue, vector<vector<float > > coeffError, vector<vector<bool > > coeffFixed, string focusModel, int numSourceObs, AntennaMakeMod::AntennaMake antennaMake) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->numCoeff == numCoeff);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusRMS == focusRMS);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffName == coeffName);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffFormula == coeffFormula);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffValue == coeffValue);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffError == coeffError);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffFixed == coeffFixed);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusModel == focusModel);
		
		if (!result) return false;
	

	
		
		result = result && (this->numSourceObs == numSourceObs);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaMake == antennaMake);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalFocusModelRow::compareRequiredValue(int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusRMS, vector<vector<string > > coeffName, vector<vector<string > > coeffFormula, vector<vector<float > > coeffValue, vector<vector<float > > coeffError, vector<vector<bool > > coeffFixed, string focusModel, int numSourceObs, AntennaMakeMod::AntennaMake antennaMake) {
		bool result;
		result = true;
		
	
		if (!(this->numCoeff == numCoeff)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->focusRMS == focusRMS)) return false;
	

	
		if (!(this->coeffName == coeffName)) return false;
	

	
		if (!(this->coeffFormula == coeffFormula)) return false;
	

	
		if (!(this->coeffValue == coeffValue)) return false;
	

	
		if (!(this->coeffError == coeffError)) return false;
	

	
		if (!(this->coeffFixed == coeffFixed)) return false;
	

	
		if (!(this->focusModel == focusModel)) return false;
	

	
		if (!(this->numSourceObs == numSourceObs)) return false;
	

	
		if (!(this->antennaMake == antennaMake)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalFocusModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalFocusModelRow::equalByRequiredValue(CalFocusModelRow* x) {
		
			
		if (this->numCoeff != x->numCoeff) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->focusRMS != x->focusRMS) return false;
			
		if (this->coeffName != x->coeffName) return false;
			
		if (this->coeffFormula != x->coeffFormula) return false;
			
		if (this->coeffValue != x->coeffValue) return false;
			
		if (this->coeffError != x->coeffError) return false;
			
		if (this->coeffFixed != x->coeffFixed) return false;
			
		if (this->focusModel != x->focusModel) return false;
			
		if (this->numSourceObs != x->numSourceObs) return false;
			
		if (this->antennaMake != x->antennaMake) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
