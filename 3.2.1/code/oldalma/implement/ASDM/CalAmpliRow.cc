
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
 * File CalAmpliRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalAmpliRow.h>
#include <CalAmpliTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalAmpliRow;
using asdm::CalAmpliTable;

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

	CalAmpliRow::~CalAmpliRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalAmpliTable &CalAmpliRow::getTable() const {
		return table;
	}
	
	void CalAmpliRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalAmpliRowIDL struct.
	 */
	CalAmpliRowIDL *CalAmpliRow::toIDL() const {
		CalAmpliRowIDL *x = new CalAmpliRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->numAPC = numAPC;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
		x->atmPhaseCorrections.length(atmPhaseCorrections.size());
		for (unsigned int i = 0; i < atmPhaseCorrections.size(); ++i) {
			
				
			x->atmPhaseCorrections[i] = atmPhaseCorrections.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		x->apertureEfficiencyExists = apertureEfficiencyExists;
		
		
			
		x->apertureEfficiency.length(apertureEfficiency.size());
		for (unsigned int i = 0; i < apertureEfficiency.size(); i++) {
			x->apertureEfficiency[i].length(apertureEfficiency.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < apertureEfficiency.size() ; i++)
			for (unsigned int j = 0; j < apertureEfficiency.at(i).size(); j++)
					
						
				x->apertureEfficiency[i][j] = apertureEfficiency.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->apertureEfficiencyErrorExists = apertureEfficiencyErrorExists;
		
		
			
		x->apertureEfficiencyError.length(apertureEfficiencyError.size());
		for (unsigned int i = 0; i < apertureEfficiencyError.size(); i++) {
			x->apertureEfficiencyError[i].length(apertureEfficiencyError.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < apertureEfficiencyError.size() ; i++)
			for (unsigned int j = 0; j < apertureEfficiencyError.at(i).size(); j++)
					
						
				x->apertureEfficiencyError[i][j] = apertureEfficiencyError.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->correctionValidityExists = correctionValidityExists;
		
		
			
				
		x->correctionValidity = correctionValidity;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalAmpliRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalAmpliRow::setFromIDL (CalAmpliRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setNumAPC(x.numAPC);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		atmPhaseCorrections .clear();
		for (unsigned int i = 0; i <x.atmPhaseCorrections.length(); ++i) {
			
			atmPhaseCorrections.push_back(x.atmPhaseCorrections[i]);
  			
		}
			
  		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		apertureEfficiencyExists = x.apertureEfficiencyExists;
		if (x.apertureEfficiencyExists) {
		
		
			
		apertureEfficiency .clear();
		vector<float> v_aux_apertureEfficiency;
		for (unsigned int i = 0; i < x.apertureEfficiency.length(); ++i) {
			v_aux_apertureEfficiency.clear();
			for (unsigned int j = 0; j < x.apertureEfficiency[0].length(); ++j) {
				
				v_aux_apertureEfficiency.push_back(x.apertureEfficiency[i][j]);
	  			
  			}
  			apertureEfficiency.push_back(v_aux_apertureEfficiency);			
		}
			
  		
		
		}
		
	

	
		
		apertureEfficiencyErrorExists = x.apertureEfficiencyErrorExists;
		if (x.apertureEfficiencyErrorExists) {
		
		
			
		apertureEfficiencyError .clear();
		vector<float> v_aux_apertureEfficiencyError;
		for (unsigned int i = 0; i < x.apertureEfficiencyError.length(); ++i) {
			v_aux_apertureEfficiencyError.clear();
			for (unsigned int j = 0; j < x.apertureEfficiencyError[0].length(); ++j) {
				
				v_aux_apertureEfficiencyError.push_back(x.apertureEfficiencyError[i][j]);
	  			
  			}
  			apertureEfficiencyError.push_back(v_aux_apertureEfficiencyError);			
		}
			
  		
		
		}
		
	

	
		
		correctionValidityExists = x.correctionValidityExists;
		if (x.correctionValidityExists) {
		
		
			
		setCorrectionValidity(x.correctionValidity);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalAmpli");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalAmpliRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(numAPC, "numAPC", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrections", atmPhaseCorrections));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		if (apertureEfficiencyExists) {
		
		
		Parser::toXML(apertureEfficiency, "apertureEfficiency", buf);
		
		
		}
		
	

  	
 		
		if (apertureEfficiencyErrorExists) {
		
		
		Parser::toXML(apertureEfficiencyError, "apertureEfficiencyError", buf);
		
		
		}
		
	

  	
 		
		if (correctionValidityExists) {
		
		
		Parser::toXML(correctionValidity, "correctionValidity", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
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
	void CalAmpliRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setNumAPC(Parser::getInteger("numAPC","CalAmpli",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalAmpli",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalAmpli",rowDoc);
		
		
		
	

	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalAmpli",rowDoc));
			
		
	

	
		
		
		
		atmPhaseCorrections = EnumerationParser::getAtmPhaseCorrection1D("atmPhaseCorrections","CalAmpli",rowDoc);			
		
		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalAmpli",rowDoc);			
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalAmpli",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalAmpli",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalAmpli",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<apertureEfficiency>")) {
			
								
	  		setApertureEfficiency(Parser::get2DFloat("apertureEfficiency","CalAmpli",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<apertureEfficiencyError>")) {
			
								
	  		setApertureEfficiencyError(Parser::get2DFloat("apertureEfficiencyError","CalAmpli",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<correctionValidity>")) {
			
	  		setCorrectionValidity(Parser::getBoolean("correctionValidity","CalAmpli",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalAmpli");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get numAPC.
 	 * @return numAPC as int
 	 */
 	int CalAmpliRow::getNumAPC() const {
	
  		return numAPC;
 	}

 	/**
 	 * Set numAPC with the specified int.
 	 * @param numAPC The int value to which numAPC is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setNumAPC (int numAPC)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAPC = numAPC;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalAmpliRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalAmpliRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalAmpliRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalAmpliRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalAmpli");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrections.
 	 * @return atmPhaseCorrections as vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >
 	 */
 	vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > CalAmpliRow::getAtmPhaseCorrections() const {
	
  		return atmPhaseCorrections;
 	}

 	/**
 	 * Set atmPhaseCorrections with the specified vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >.
 	 * @param atmPhaseCorrections The vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > value to which atmPhaseCorrections is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setAtmPhaseCorrections (vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmPhaseCorrections = atmPhaseCorrections;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalAmpliRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalAmpliRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalAmpliRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalAmpliRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	
	/**
	 * The attribute apertureEfficiency is optional. Return true if this attribute exists.
	 * @return true if and only if the apertureEfficiency attribute exists. 
	 */
	bool CalAmpliRow::isApertureEfficiencyExists() const {
		return apertureEfficiencyExists;
	}
	

	
 	/**
 	 * Get apertureEfficiency, which is optional.
 	 * @return apertureEfficiency as vector<vector<float > >
 	 * @throw IllegalAccessException If apertureEfficiency does not exist.
 	 */
 	vector<vector<float > > CalAmpliRow::getApertureEfficiency() const throw(IllegalAccessException) {
		if (!apertureEfficiencyExists) {
			throw IllegalAccessException("apertureEfficiency", "CalAmpli");
		}
	
  		return apertureEfficiency;
 	}

 	/**
 	 * Set apertureEfficiency with the specified vector<vector<float > >.
 	 * @param apertureEfficiency The vector<vector<float > > value to which apertureEfficiency is to be set.
 	 
 	
 	 */
 	void CalAmpliRow::setApertureEfficiency (vector<vector<float > > apertureEfficiency) {
	
 		this->apertureEfficiency = apertureEfficiency;
	
		apertureEfficiencyExists = true;
	
 	}
	
	
	/**
	 * Mark apertureEfficiency, which is an optional field, as non-existent.
	 */
	void CalAmpliRow::clearApertureEfficiency () {
		apertureEfficiencyExists = false;
	}
	

	
	/**
	 * The attribute apertureEfficiencyError is optional. Return true if this attribute exists.
	 * @return true if and only if the apertureEfficiencyError attribute exists. 
	 */
	bool CalAmpliRow::isApertureEfficiencyErrorExists() const {
		return apertureEfficiencyErrorExists;
	}
	

	
 	/**
 	 * Get apertureEfficiencyError, which is optional.
 	 * @return apertureEfficiencyError as vector<vector<float > >
 	 * @throw IllegalAccessException If apertureEfficiencyError does not exist.
 	 */
 	vector<vector<float > > CalAmpliRow::getApertureEfficiencyError() const throw(IllegalAccessException) {
		if (!apertureEfficiencyErrorExists) {
			throw IllegalAccessException("apertureEfficiencyError", "CalAmpli");
		}
	
  		return apertureEfficiencyError;
 	}

 	/**
 	 * Set apertureEfficiencyError with the specified vector<vector<float > >.
 	 * @param apertureEfficiencyError The vector<vector<float > > value to which apertureEfficiencyError is to be set.
 	 
 	
 	 */
 	void CalAmpliRow::setApertureEfficiencyError (vector<vector<float > > apertureEfficiencyError) {
	
 		this->apertureEfficiencyError = apertureEfficiencyError;
	
		apertureEfficiencyErrorExists = true;
	
 	}
	
	
	/**
	 * Mark apertureEfficiencyError, which is an optional field, as non-existent.
	 */
	void CalAmpliRow::clearApertureEfficiencyError () {
		apertureEfficiencyErrorExists = false;
	}
	

	
	/**
	 * The attribute correctionValidity is optional. Return true if this attribute exists.
	 * @return true if and only if the correctionValidity attribute exists. 
	 */
	bool CalAmpliRow::isCorrectionValidityExists() const {
		return correctionValidityExists;
	}
	

	
 	/**
 	 * Get correctionValidity, which is optional.
 	 * @return correctionValidity as bool
 	 * @throw IllegalAccessException If correctionValidity does not exist.
 	 */
 	bool CalAmpliRow::getCorrectionValidity() const throw(IllegalAccessException) {
		if (!correctionValidityExists) {
			throw IllegalAccessException("correctionValidity", "CalAmpli");
		}
	
  		return correctionValidity;
 	}

 	/**
 	 * Set correctionValidity with the specified bool.
 	 * @param correctionValidity The bool value to which correctionValidity is to be set.
 	 
 	
 	 */
 	void CalAmpliRow::setCorrectionValidity (bool correctionValidity) {
	
 		this->correctionValidity = correctionValidity;
	
		correctionValidityExists = true;
	
 	}
	
	
	/**
	 * Mark correctionValidity, which is an optional field, as non-existent.
	 */
	void CalAmpliRow::clearCorrectionValidity () {
		correctionValidityExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalAmpliRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalAmpliRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalAmpli");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalAmpliRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalAmpliRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalAmpli");
		
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
	 CalDataRow* CalAmpliRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalAmpliRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalAmpliRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalAmpliRow::CalAmpliRow (CalAmpliTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	
		apertureEfficiencyExists = false;
	

	
		apertureEfficiencyErrorExists = false;
	

	
		correctionValidityExists = false;
	

	
	

	

	
	
	
	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	

	

	

	
	
	}
	
	CalAmpliRow::CalAmpliRow (CalAmpliTable &t, CalAmpliRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	
		apertureEfficiencyExists = false;
	

	
		apertureEfficiencyErrorExists = false;
	

	
		correctionValidityExists = false;
	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
		
		
		
			numAPC = row.numAPC;
		
			numReceptor = row.numReceptor;
		
			receiverBand = row.receiverBand;
		
			atmPhaseCorrections = row.atmPhaseCorrections;
		
			polarizationTypes = row.polarizationTypes;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			frequencyRange = row.frequencyRange;
		
		
		
		
		if (row.apertureEfficiencyExists) {
			apertureEfficiency = row.apertureEfficiency;		
			apertureEfficiencyExists = true;
		}
		else
			apertureEfficiencyExists = false;
		
		if (row.apertureEfficiencyErrorExists) {
			apertureEfficiencyError = row.apertureEfficiencyError;		
			apertureEfficiencyErrorExists = true;
		}
		else
			apertureEfficiencyErrorExists = false;
		
		if (row.correctionValidityExists) {
			correctionValidity = row.correctionValidity;		
			correctionValidityExists = true;
		}
		else
			correctionValidityExists = false;
		
		}	
	}

	
	bool CalAmpliRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numAPC == numAPC);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmPhaseCorrections == atmPhaseCorrections);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalAmpliRow::compareRequiredValue(int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange) {
		bool result;
		result = true;
		
	
		if (!(this->numAPC == numAPC)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

	
		if (!(this->atmPhaseCorrections == atmPhaseCorrections)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalAmpliRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalAmpliRow::equalByRequiredValue(CalAmpliRow* x) {
		
			
		if (this->numAPC != x->numAPC) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		if (this->atmPhaseCorrections != x->atmPhaseCorrections) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
