
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
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->atmPhaseCorrection = atmPhaseCorrection;
 				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
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
			
		
	

	
  		
		
		
			
		x->apertureEfficiency.length(apertureEfficiency.size());
		for (unsigned int i = 0; i < apertureEfficiency.size(); ++i) {
			
				
			x->apertureEfficiency[i] = apertureEfficiency.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->apertureEfficiencyError.length(apertureEfficiencyError.size());
		for (unsigned int i = 0; i < apertureEfficiencyError.size(); ++i) {
			
				
			x->apertureEfficiencyError[i] = apertureEfficiencyError.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
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
	void CalAmpliRow::setFromIDL (CalAmpliRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setAtmPhaseCorrection(x.atmPhaseCorrection);
  			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
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
			
  		
		
	

	
		
		
			
		apertureEfficiency .clear();
		for (unsigned int i = 0; i <x.apertureEfficiency.length(); ++i) {
			
			apertureEfficiency.push_back(x.apertureEfficiency[i]);
  			
		}
			
  		
		
	

	
		
		
			
		apertureEfficiencyError .clear();
		for (unsigned int i = 0; i <x.apertureEfficiencyError.length(); ++i) {
			
			apertureEfficiencyError.push_back(x.apertureEfficiencyError[i]);
  			
		}
			
  		
		
	

	
		
		correctionValidityExists = x.correctionValidityExists;
		if (x.correctionValidityExists) {
		
		
			
		setCorrectionValidity(x.correctionValidity);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalAmpli");
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
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrection", atmPhaseCorrection));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(apertureEfficiency, "apertureEfficiency", buf);
		
		
	

  	
 		
		
		Parser::toXML(apertureEfficiencyError, "apertureEfficiencyError", buf);
		
		
	

  	
 		
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
	void CalAmpliRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalAmpli",rowDoc));
			
		
	

	
		
		
		
		atmPhaseCorrection = EnumerationParser::getAtmPhaseCorrection("atmPhaseCorrection","CalAmpli",rowDoc);
		
		
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalAmpli",rowDoc);
		
		
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalAmpli",rowDoc));
			
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalAmpli",rowDoc);			
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalAmpli",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalAmpli",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalAmpli",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setApertureEfficiency(Parser::get1DFloat("apertureEfficiency","CalAmpli",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setApertureEfficiencyError(Parser::get1DFloat("apertureEfficiencyError","CalAmpli",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<correctionValidity>")) {
			
	  		setCorrectionValidity(Parser::getBoolean("correctionValidity","CalAmpli",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalAmpli");
		}
	}
	
	void CalAmpliRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeString(antennaName);
				
		
	

	
	
		
					
			eoss.writeInt(atmPhaseCorrection);
				
		
	

	
	
		
					
			eoss.writeInt(receiverBand);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); i++)
				
			eoss.writeInt(polarizationTypes.at(i));
				
				
						
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
	Frequency::toBin(frequencyRange, eoss);
		
	

	
	
		
		
			
		eoss.writeInt((int) apertureEfficiency.size());
		for (unsigned int i = 0; i < apertureEfficiency.size(); i++)
				
			eoss.writeFloat(apertureEfficiency.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) apertureEfficiencyError.size());
		for (unsigned int i = 0; i < apertureEfficiencyError.size(); i++)
				
			eoss.writeFloat(apertureEfficiencyError.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(correctionValidityExists);
	if (correctionValidityExists) {
	
	
	
		
						
			eoss.writeBoolean(correctionValidity);
				
		
	

	}

	}
	
	CalAmpliRow* CalAmpliRow::fromBin(EndianISStream& eiss, CalAmpliTable& table) {
		CalAmpliRow* row = new  CalAmpliRow(table);
		
		
		
	
	
		
			
		row->antennaName =  eiss.readString();
			
		
	

	
	
		
			
		row->atmPhaseCorrection = CAtmPhaseCorrection::from_int(eiss.readInt());
			
		
	

	
	
		
			
		row->receiverBand = CReceiverBand::from_int(eiss.readInt());
			
		
	

	
		
		
		row->calDataId =  Tag::fromBin(eiss);
		
	

	
		
		
		row->calReductionId =  Tag::fromBin(eiss);
		
	

	
	
		
			
		row->numReceptor =  eiss.readInt();
			
		
	

	
	
		
			
	
		row->polarizationTypes.clear();
		
		unsigned int polarizationTypesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypesDim1; i++)
			
			row->polarizationTypes.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	

	
		
		
		row->startValidTime =  ArrayTime::fromBin(eiss);
		
	

	
		
		
		row->endValidTime =  ArrayTime::fromBin(eiss);
		
	

	
		
		
			
	
	row->frequencyRange = Frequency::from1DBin(eiss);	
	

		
	

	
	
		
			
	
		row->apertureEfficiency.clear();
		
		unsigned int apertureEfficiencyDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < apertureEfficiencyDim1; i++)
			
			row->apertureEfficiency.push_back(eiss.readFloat());
			
	

		
	

	
	
		
			
	
		row->apertureEfficiencyError.clear();
		
		unsigned int apertureEfficiencyErrorDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < apertureEfficiencyErrorDim1; i++)
			
			row->apertureEfficiencyError.push_back(eiss.readFloat());
			
	

		
	

		
		
		
	row->correctionValidityExists = eiss.readBoolean();
	if (row->correctionValidityExists) {
		
	
	
		
			
		row->correctionValidity =  eiss.readBoolean();
			
		
	

	}

		
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
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
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as AtmPhaseCorrectionMod::AtmPhaseCorrection
 	 */
 	AtmPhaseCorrectionMod::AtmPhaseCorrection CalAmpliRow::getAtmPhaseCorrection() const {
	
  		return atmPhaseCorrection;
 	}

 	/**
 	 * Set atmPhaseCorrection with the specified AtmPhaseCorrectionMod::AtmPhaseCorrection.
 	 * @param atmPhaseCorrection The AtmPhaseCorrectionMod::AtmPhaseCorrection value to which atmPhaseCorrection is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalAmpliRow::setAtmPhaseCorrection (AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("atmPhaseCorrection", "CalAmpli");
		
  		}
  	
 		this->atmPhaseCorrection = atmPhaseCorrection;
	
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
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalAmpliRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalAmpli");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
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
 	 * Get apertureEfficiency.
 	 * @return apertureEfficiency as vector<float >
 	 */
 	vector<float > CalAmpliRow::getApertureEfficiency() const {
	
  		return apertureEfficiency;
 	}

 	/**
 	 * Set apertureEfficiency with the specified vector<float >.
 	 * @param apertureEfficiency The vector<float > value to which apertureEfficiency is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setApertureEfficiency (vector<float > apertureEfficiency)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->apertureEfficiency = apertureEfficiency;
	
 	}
	
	

	

	
 	/**
 	 * Get apertureEfficiencyError.
 	 * @return apertureEfficiencyError as vector<float >
 	 */
 	vector<float > CalAmpliRow::getApertureEfficiencyError() const {
	
  		return apertureEfficiencyError;
 	}

 	/**
 	 * Set apertureEfficiencyError with the specified vector<float >.
 	 * @param apertureEfficiencyError The vector<float > value to which apertureEfficiencyError is to be set.
 	 
 	
 		
 	 */
 	void CalAmpliRow::setApertureEfficiencyError (vector<float > apertureEfficiencyError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->apertureEfficiencyError = apertureEfficiencyError;
	
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
 	bool CalAmpliRow::getCorrectionValidity() const  {
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
		
	
	

	

	

	

	

	

	

	

	

	

	
		correctionValidityExists = false;
	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
atmPhaseCorrection = CAtmPhaseCorrection::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	

	

	
	
	}
	
	CalAmpliRow::CalAmpliRow (CalAmpliTable &t, CalAmpliRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	
		correctionValidityExists = false;
	

	
	

	
		
		}
		else {
	
		
			antennaName = row.antennaName;
		
			atmPhaseCorrection = row.atmPhaseCorrection;
		
			receiverBand = row.receiverBand;
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			numReceptor = row.numReceptor;
		
			polarizationTypes = row.polarizationTypes;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			frequencyRange = row.frequencyRange;
		
			apertureEfficiency = row.apertureEfficiency;
		
			apertureEfficiencyError = row.apertureEfficiencyError;
		
		
		
		
		if (row.correctionValidityExists) {
			correctionValidity = row.correctionValidity;		
			correctionValidityExists = true;
		}
		else
			correctionValidityExists = false;
		
		}	
	}

	
	bool CalAmpliRow::compareNoAutoInc(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<float > apertureEfficiency, vector<float > apertureEfficiencyError) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmPhaseCorrection == atmPhaseCorrection);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->apertureEfficiency == apertureEfficiency);
		
		if (!result) return false;
	

	
		
		result = result && (this->apertureEfficiencyError == apertureEfficiencyError);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalAmpliRow::compareRequiredValue(int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<float > apertureEfficiency, vector<float > apertureEfficiencyError) {
		bool result;
		result = true;
		
	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->apertureEfficiency == apertureEfficiency)) return false;
	

	
		if (!(this->apertureEfficiencyError == apertureEfficiencyError)) return false;
	

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
		
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->apertureEfficiency != x->apertureEfficiency) return false;
			
		if (this->apertureEfficiencyError != x->apertureEfficiencyError) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
