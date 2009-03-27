
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
 * File CalHolographyRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalHolographyRow.h>
#include <CalHolographyTable.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>

#include <CalDataTable.h>
#include <CalDataRow.h>
	

using asdm::ASDM;
using asdm::CalHolographyRow;
using asdm::CalHolographyTable;

using asdm::CalReductionTable;
using asdm::CalReductionRow;

using asdm::CalDataTable;
using asdm::CalDataRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	CalHolographyRow::~CalHolographyRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalHolographyTable &CalHolographyRow::getTable() const {
		return table;
	}
	
	void CalHolographyRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalHolographyRowIDL struct.
	 */
	CalHolographyRowIDL *CalHolographyRow::toIDL() const {
		CalHolographyRowIDL *x = new CalHolographyRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->antennaMake = antennaMake;
 				
 			
		
	

	
  		
		
		
			
				
		x->numScrew = numScrew;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->focusPosition.length(focusPosition.size());
		for (unsigned int i = 0; i < focusPosition.size(); ++i) {
			
			x->focusPosition[i] = focusPosition.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->rawRms = rawRms.toIDLLength();
			
		
	

	
  		
		
		
			
		x->weightedRms = weightedRms.toIDLLength();
			
		
	

	
  		
		
		
			
		x->screwName.length(screwName.size());
		for (unsigned int i = 0; i < screwName.size(); ++i) {
			
				
			x->screwName[i] = CORBA::string_dup(screwName.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->screwMotion.length(screwMotion.size());
		for (unsigned int i = 0; i < screwMotion.size(); ++i) {
			
			x->screwMotion[i] = screwMotion.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
		x->screwMotionError.length(screwMotionError.size());
		for (unsigned int i = 0; i < screwMotionError.size(); ++i) {
			
			x->screwMotionError[i] = screwMotionError.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->panelModes = panelModes;
 				
 			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->beamMapUID = beamMapUID.toIDLEntityRef();
			
		
	

	
  		
		
		
			
		x->surfaceMapUID = surfaceMapUID.toIDLEntityRef();
			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->direction.length(direction.size());
		for (unsigned int i = 0; i < direction.size(); ++i) {
			
			x->direction[i] = direction.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->ambientTemperature = ambientTemperature.toIDLTemperature();
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalHolographyRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalHolographyRow::setFromIDL (CalHolographyRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setAntennaMake(x.antennaMake);
  			
 		
		
	

	
		
		
			
		setNumScrew(x.numScrew);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		focusPosition .clear();
		for (unsigned int i = 0; i <x.focusPosition.length(); ++i) {
			
			focusPosition.push_back(Length (x.focusPosition[i]));
			
		}
			
  		
		
	

	
		
		
			
		setRawRms(Length (x.rawRms));
			
 		
		
	

	
		
		
			
		setWeightedRms(Length (x.weightedRms));
			
 		
		
	

	
		
		
			
		screwName .clear();
		for (unsigned int i = 0; i <x.screwName.length(); ++i) {
			
			screwName.push_back(string (x.screwName[i]));
			
		}
			
  		
		
	

	
		
		
			
		screwMotion .clear();
		for (unsigned int i = 0; i <x.screwMotion.length(); ++i) {
			
			screwMotion.push_back(Length (x.screwMotion[i]));
			
		}
			
  		
		
	

	
		
		
			
		screwMotionError .clear();
		for (unsigned int i = 0; i <x.screwMotionError.length(); ++i) {
			
			screwMotionError.push_back(Length (x.screwMotionError[i]));
			
		}
			
  		
		
	

	
		
		
			
		setPanelModes(x.panelModes);
  			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		setBeamMapUID(EntityRef (x.beamMapUID));
			
 		
		
	

	
		
		
			
		setSurfaceMapUID(EntityRef (x.surfaceMapUID));
			
 		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		direction .clear();
		for (unsigned int i = 0; i <x.direction.length(); ++i) {
			
			direction.push_back(Angle (x.direction[i]));
			
		}
			
  		
		
	

	
		
		
			
		setAmbientTemperature(Temperature (x.ambientTemperature));
			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalHolography");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalHolographyRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("antennaMake", antennaMake));
		
		
	

  	
 		
		
		Parser::toXML(numScrew, "numScrew", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusPosition, "focusPosition", buf);
		
		
	

  	
 		
		
		Parser::toXML(rawRms, "rawRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(weightedRms, "weightedRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(screwName, "screwName", buf);
		
		
	

  	
 		
		
		Parser::toXML(screwMotion, "screwMotion", buf);
		
		
	

  	
 		
		
		Parser::toXML(screwMotionError, "screwMotionError", buf);
		
		
	

  	
 		
		
		Parser::toXML(panelModes, "panelModes", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(beamMapUID, "beamMapUID", buf);
		
		
	

  	
 		
		
		Parser::toXML(surfaceMapUID, "surfaceMapUID", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(direction, "direction", buf);
		
		
	

  	
 		
		
		Parser::toXML(ambientTemperature, "ambientTemperature", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalHolographyRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalHolography",rowDoc));
			
		
	

	
		
		
		
		antennaMake = EnumerationParser::getAntennaMake("antennaMake","CalHolography",rowDoc);
		
		
		
	

	
  		
			
	  	setNumScrew(Parser::getInteger("numScrew","CalHolography",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalHolography",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalHolography",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalHolography",rowDoc));
			
		
	

	
  		
			
					
	  	setFocusPosition(Parser::get1DLength("focusPosition","CalHolography",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setRawRms(Parser::getLength("rawRms","CalHolography",rowDoc));
			
		
	

	
  		
			
	  	setWeightedRms(Parser::getLength("weightedRms","CalHolography",rowDoc));
			
		
	

	
  		
			
					
	  	setScrewName(Parser::get1DString("screwName","CalHolography",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setScrewMotion(Parser::get1DLength("screwMotion","CalHolography",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setScrewMotionError(Parser::get1DLength("screwMotionError","CalHolography",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setPanelModes(Parser::getInteger("panelModes","CalHolography",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalHolography",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setBeamMapUID(Parser::getEntityRef("beamMapUID","CalHolography",rowDoc));
			
		
	

	
  		
			
	  	setSurfaceMapUID(Parser::getEntityRef("surfaceMapUID","CalHolography",rowDoc));
			
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalHolography",rowDoc);			
		
		
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalHolography",rowDoc);
		
		
		
	

	
  		
			
					
	  	setDirection(Parser::get1DAngle("direction","CalHolography",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setAmbientTemperature(Parser::getTemperature("ambientTemperature","CalHolography",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalHolography");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalHolographyRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalHolographyRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalHolography");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake CalHolographyRow::getAntennaMake() const {
	
  		return antennaMake;
 	}

 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setAntennaMake (AntennaMakeMod::AntennaMake antennaMake)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaMake = antennaMake;
	
 	}
	
	

	

	
 	/**
 	 * Get numScrew.
 	 * @return numScrew as int
 	 */
 	int CalHolographyRow::getNumScrew() const {
	
  		return numScrew;
 	}

 	/**
 	 * Set numScrew with the specified int.
 	 * @param numScrew The int value to which numScrew is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setNumScrew (int numScrew)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numScrew = numScrew;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalHolographyRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalHolographyRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalHolographyRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get focusPosition.
 	 * @return focusPosition as vector<Length >
 	 */
 	vector<Length > CalHolographyRow::getFocusPosition() const {
	
  		return focusPosition;
 	}

 	/**
 	 * Set focusPosition with the specified vector<Length >.
 	 * @param focusPosition The vector<Length > value to which focusPosition is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setFocusPosition (vector<Length > focusPosition)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusPosition = focusPosition;
	
 	}
	
	

	

	
 	/**
 	 * Get rawRms.
 	 * @return rawRms as Length
 	 */
 	Length CalHolographyRow::getRawRms() const {
	
  		return rawRms;
 	}

 	/**
 	 * Set rawRms with the specified Length.
 	 * @param rawRms The Length value to which rawRms is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setRawRms (Length rawRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->rawRms = rawRms;
	
 	}
	
	

	

	
 	/**
 	 * Get weightedRms.
 	 * @return weightedRms as Length
 	 */
 	Length CalHolographyRow::getWeightedRms() const {
	
  		return weightedRms;
 	}

 	/**
 	 * Set weightedRms with the specified Length.
 	 * @param weightedRms The Length value to which weightedRms is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setWeightedRms (Length weightedRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->weightedRms = weightedRms;
	
 	}
	
	

	

	
 	/**
 	 * Get screwName.
 	 * @return screwName as vector<string >
 	 */
 	vector<string > CalHolographyRow::getScrewName() const {
	
  		return screwName;
 	}

 	/**
 	 * Set screwName with the specified vector<string >.
 	 * @param screwName The vector<string > value to which screwName is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setScrewName (vector<string > screwName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->screwName = screwName;
	
 	}
	
	

	

	
 	/**
 	 * Get screwMotion.
 	 * @return screwMotion as vector<Length >
 	 */
 	vector<Length > CalHolographyRow::getScrewMotion() const {
	
  		return screwMotion;
 	}

 	/**
 	 * Set screwMotion with the specified vector<Length >.
 	 * @param screwMotion The vector<Length > value to which screwMotion is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setScrewMotion (vector<Length > screwMotion)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->screwMotion = screwMotion;
	
 	}
	
	

	

	
 	/**
 	 * Get screwMotionError.
 	 * @return screwMotionError as vector<Length >
 	 */
 	vector<Length > CalHolographyRow::getScrewMotionError() const {
	
  		return screwMotionError;
 	}

 	/**
 	 * Set screwMotionError with the specified vector<Length >.
 	 * @param screwMotionError The vector<Length > value to which screwMotionError is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setScrewMotionError (vector<Length > screwMotionError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->screwMotionError = screwMotionError;
	
 	}
	
	

	

	
 	/**
 	 * Get panelModes.
 	 * @return panelModes as int
 	 */
 	int CalHolographyRow::getPanelModes() const {
	
  		return panelModes;
 	}

 	/**
 	 * Set panelModes with the specified int.
 	 * @param panelModes The int value to which panelModes is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setPanelModes (int panelModes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->panelModes = panelModes;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalHolographyRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get beamMapUID.
 	 * @return beamMapUID as EntityRef
 	 */
 	EntityRef CalHolographyRow::getBeamMapUID() const {
	
  		return beamMapUID;
 	}

 	/**
 	 * Set beamMapUID with the specified EntityRef.
 	 * @param beamMapUID The EntityRef value to which beamMapUID is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setBeamMapUID (EntityRef beamMapUID)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->beamMapUID = beamMapUID;
	
 	}
	
	

	

	
 	/**
 	 * Get surfaceMapUID.
 	 * @return surfaceMapUID as EntityRef
 	 */
 	EntityRef CalHolographyRow::getSurfaceMapUID() const {
	
  		return surfaceMapUID;
 	}

 	/**
 	 * Set surfaceMapUID with the specified EntityRef.
 	 * @param surfaceMapUID The EntityRef value to which surfaceMapUID is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setSurfaceMapUID (EntityRef surfaceMapUID)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->surfaceMapUID = surfaceMapUID;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalHolographyRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalHolographyRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > CalHolographyRow::getDirection() const {
	
  		return direction;
 	}

 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setDirection (vector<Angle > direction)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->direction = direction;
	
 	}
	
	

	

	
 	/**
 	 * Get ambientTemperature.
 	 * @return ambientTemperature as Temperature
 	 */
 	Temperature CalHolographyRow::getAmbientTemperature() const {
	
  		return ambientTemperature;
 	}

 	/**
 	 * Set ambientTemperature with the specified Temperature.
 	 * @param ambientTemperature The Temperature value to which ambientTemperature is to be set.
 	 
 	
 		
 	 */
 	void CalHolographyRow::setAmbientTemperature (Temperature ambientTemperature)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->ambientTemperature = ambientTemperature;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalHolographyRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalHolographyRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalHolography");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalHolographyRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalHolographyRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalHolography");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalHolographyRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalHolographyRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	/**
	 * Create a CalHolographyRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalHolographyRow::CalHolographyRow (CalHolographyTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
antennaMake = CAntennaMake::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	
	
	}
	
	CalHolographyRow::CalHolographyRow (CalHolographyTable &t, CalHolographyRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
		
		
		
			antennaMake = row.antennaMake;
		
			numScrew = row.numScrew;
		
			numReceptor = row.numReceptor;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			focusPosition = row.focusPosition;
		
			rawRms = row.rawRms;
		
			weightedRms = row.weightedRms;
		
			screwName = row.screwName;
		
			screwMotion = row.screwMotion;
		
			screwMotionError = row.screwMotionError;
		
			panelModes = row.panelModes;
		
			frequencyRange = row.frequencyRange;
		
			beamMapUID = row.beamMapUID;
		
			surfaceMapUID = row.surfaceMapUID;
		
			polarizationTypes = row.polarizationTypes;
		
			receiverBand = row.receiverBand;
		
			direction = row.direction;
		
			ambientTemperature = row.ambientTemperature;
		
		
		
		
		}	
	}

	
	bool CalHolographyRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, AntennaMakeMod::AntennaMake antennaMake, int numScrew, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusPosition, Length rawRms, Length weightedRms, vector<string > screwName, vector<Length > screwMotion, vector<Length > screwMotionError, int panelModes, vector<Frequency > frequencyRange, EntityRef beamMapUID, EntityRef surfaceMapUID, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ReceiverBandMod::ReceiverBand receiverBand, vector<Angle > direction, Temperature ambientTemperature) {
		bool result;
		result = true;
		
	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaMake == antennaMake);
		
		if (!result) return false;
	

	
		
		result = result && (this->numScrew == numScrew);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusPosition == focusPosition);
		
		if (!result) return false;
	

	
		
		result = result && (this->rawRms == rawRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->weightedRms == weightedRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->screwName == screwName);
		
		if (!result) return false;
	

	
		
		result = result && (this->screwMotion == screwMotion);
		
		if (!result) return false;
	

	
		
		result = result && (this->screwMotionError == screwMotionError);
		
		if (!result) return false;
	

	
		
		result = result && (this->panelModes == panelModes);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->beamMapUID == beamMapUID);
		
		if (!result) return false;
	

	
		
		result = result && (this->surfaceMapUID == surfaceMapUID);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->direction == direction);
		
		if (!result) return false;
	

	
		
		result = result && (this->ambientTemperature == ambientTemperature);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalHolographyRow::compareRequiredValue(AntennaMakeMod::AntennaMake antennaMake, int numScrew, int numReceptor, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > focusPosition, Length rawRms, Length weightedRms, vector<string > screwName, vector<Length > screwMotion, vector<Length > screwMotionError, int panelModes, vector<Frequency > frequencyRange, EntityRef beamMapUID, EntityRef surfaceMapUID, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ReceiverBandMod::ReceiverBand receiverBand, vector<Angle > direction, Temperature ambientTemperature) {
		bool result;
		result = true;
		
	
		if (!(this->antennaMake == antennaMake)) return false;
	

	
		if (!(this->numScrew == numScrew)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->focusPosition == focusPosition)) return false;
	

	
		if (!(this->rawRms == rawRms)) return false;
	

	
		if (!(this->weightedRms == weightedRms)) return false;
	

	
		if (!(this->screwName == screwName)) return false;
	

	
		if (!(this->screwMotion == screwMotion)) return false;
	

	
		if (!(this->screwMotionError == screwMotionError)) return false;
	

	
		if (!(this->panelModes == panelModes)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->beamMapUID == beamMapUID)) return false;
	

	
		if (!(this->surfaceMapUID == surfaceMapUID)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->receiverBand == receiverBand)) return false;
	

	
		if (!(this->direction == direction)) return false;
	

	
		if (!(this->ambientTemperature == ambientTemperature)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalHolographyRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalHolographyRow::equalByRequiredValue(CalHolographyRow* x) {
		
			
		if (this->antennaMake != x->antennaMake) return false;
			
		if (this->numScrew != x->numScrew) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->focusPosition != x->focusPosition) return false;
			
		if (this->rawRms != x->rawRms) return false;
			
		if (this->weightedRms != x->weightedRms) return false;
			
		if (this->screwName != x->screwName) return false;
			
		if (this->screwMotion != x->screwMotion) return false;
			
		if (this->screwMotionError != x->screwMotionError) return false;
			
		if (this->panelModes != x->panelModes) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->beamMapUID != x->beamMapUID) return false;
			
		if (this->surfaceMapUID != x->surfaceMapUID) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->receiverBand != x->receiverBand) return false;
			
		if (this->direction != x->direction) return false;
			
		if (this->ambientTemperature != x->ambientTemperature) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
