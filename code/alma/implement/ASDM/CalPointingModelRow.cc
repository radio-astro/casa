
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
 * File CalPointingModelRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalPointingModelRow.h>
#include <CalPointingModelTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalPointingModelRow;
using asdm::CalPointingModelTable;

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

	CalPointingModelRow::~CalPointingModelRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalPointingModelTable &CalPointingModelRow::getTable() const {
		return table;
	}
	
	void CalPointingModelRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPointingModelRowIDL struct.
	 */
	CalPointingModelRowIDL *CalPointingModelRow::toIDL() const {
		CalPointingModelRowIDL *x = new CalPointingModelRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->antennaMake = antennaMake;
 				
 			
		
	

	
  		
		
		
			
				
		x->numObs = numObs;
 				
 			
		
	

	
  		
		
		
			
				
		x->numCoeff = numCoeff;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->numFormula = numFormula;
 				
 			
		
	

	
  		
		
		
			
		x->azimuthRms = azimuthRms.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->elevationRms = elevationRms.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->skyRms = skyRms.toIDLAngle();
			
		
	

	
  		
		
		
			
		x->coeffName.length(coeffName.size());
		for (unsigned int i = 0; i < coeffName.size(); ++i) {
			
				
			x->coeffName[i] = CORBA::string_dup(coeffName.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->coeffVal.length(coeffVal.size());
		for (unsigned int i = 0; i < coeffVal.size(); ++i) {
			
				
			x->coeffVal[i] = coeffVal.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->coeffError.length(coeffError.size());
		for (unsigned int i = 0; i < coeffError.size(); ++i) {
			
				
			x->coeffError[i] = coeffError.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->coeffFixed.length(coeffFixed.size());
		for (unsigned int i = 0; i < coeffFixed.size(); ++i) {
			
				
			x->coeffFixed[i] = coeffFixed.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->coeffFormula.length(coeffFormula.size());
		for (unsigned int i = 0; i < coeffFormula.size(); ++i) {
			
				
			x->coeffFormula[i] = CORBA::string_dup(coeffFormula.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->pointingModelMode = pointingModelMode;
 				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPointingModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalPointingModelRow::setFromIDL (CalPointingModelRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setAntennaMake(x.antennaMake);
  			
 		
		
	

	
		
		
			
		setNumObs(x.numObs);
  			
 		
		
	

	
		
		
			
		setNumCoeff(x.numCoeff);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setNumFormula(x.numFormula);
  			
 		
		
	

	
		
		
			
		setAzimuthRms(Angle (x.azimuthRms));
			
 		
		
	

	
		
		
			
		setElevationRms(Angle (x.elevationRms));
			
 		
		
	

	
		
		
			
		setSkyRms(Angle (x.skyRms));
			
 		
		
	

	
		
		
			
		coeffName .clear();
		for (unsigned int i = 0; i <x.coeffName.length(); ++i) {
			
			coeffName.push_back(string (x.coeffName[i]));
			
		}
			
  		
		
	

	
		
		
			
		coeffVal .clear();
		for (unsigned int i = 0; i <x.coeffVal.length(); ++i) {
			
			coeffVal.push_back(x.coeffVal[i]);
  			
		}
			
  		
		
	

	
		
		
			
		coeffError .clear();
		for (unsigned int i = 0; i <x.coeffError.length(); ++i) {
			
			coeffError.push_back(x.coeffError[i]);
  			
		}
			
  		
		
	

	
		
		
			
		coeffFixed .clear();
		for (unsigned int i = 0; i <x.coeffFixed.length(); ++i) {
			
			coeffFixed.push_back(x.coeffFixed[i]);
  			
		}
			
  		
		
	

	
		
		
			
		coeffFormula .clear();
		for (unsigned int i = 0; i <x.coeffFormula.length(); ++i) {
			
			coeffFormula.push_back(string (x.coeffFormula[i]));
			
		}
			
  		
		
	

	
		
		
			
		setPointingModelMode(x.pointingModelMode);
  			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalPointingModel");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalPointingModelRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("antennaMake", antennaMake));
		
		
	

  	
 		
		
		Parser::toXML(numObs, "numObs", buf);
		
		
	

  	
 		
		
		Parser::toXML(numCoeff, "numCoeff", buf);
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(numFormula, "numFormula", buf);
		
		
	

  	
 		
		
		Parser::toXML(azimuthRms, "azimuthRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(elevationRms, "elevationRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(skyRms, "skyRms", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffName, "coeffName", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffVal, "coeffVal", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffError, "coeffError", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffFixed, "coeffFixed", buf);
		
		
	

  	
 		
		
		Parser::toXML(coeffFormula, "coeffFormula", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("pointingModelMode", pointingModelMode));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

	
	
		
  	
 		
		
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
	void CalPointingModelRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalPointingModel",rowDoc));
			
		
	

	
		
		
		
		antennaMake = EnumerationParser::getAntennaMake("antennaMake","CalPointingModel",rowDoc);
		
		
		
	

	
  		
			
	  	setNumObs(Parser::getInteger("numObs","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setNumCoeff(Parser::getInteger("numCoeff","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setNumFormula(Parser::getInteger("numFormula","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setAzimuthRms(Parser::getAngle("azimuthRms","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setElevationRms(Parser::getAngle("elevationRms","CalPointingModel",rowDoc));
			
		
	

	
  		
			
	  	setSkyRms(Parser::getAngle("skyRms","CalPointingModel",rowDoc));
			
		
	

	
  		
			
					
	  	setCoeffName(Parser::get1DString("coeffName","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffVal(Parser::get1DFloat("coeffVal","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffError(Parser::get1DFloat("coeffError","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffFixed(Parser::get1DBoolean("coeffFixed","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setCoeffFormula(Parser::get1DString("coeffFormula","CalPointingModel",rowDoc));
	  			
	  		
		
	

	
		
		
		
		pointingModelMode = EnumerationParser::getPointingModelMode("pointingModelMode","CalPointingModel",rowDoc);
		
		
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalPointingModel",rowDoc);
		
		
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPointingModel");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalPointingModelRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingModelRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalPointingModel");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake CalPointingModelRow::getAntennaMake() const {
	
  		return antennaMake;
 	}

 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setAntennaMake (AntennaMakeMod::AntennaMake antennaMake)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaMake = antennaMake;
	
 	}
	
	

	

	
 	/**
 	 * Get numObs.
 	 * @return numObs as int
 	 */
 	int CalPointingModelRow::getNumObs() const {
	
  		return numObs;
 	}

 	/**
 	 * Set numObs with the specified int.
 	 * @param numObs The int value to which numObs is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setNumObs (int numObs)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numObs = numObs;
	
 	}
	
	

	

	
 	/**
 	 * Get numCoeff.
 	 * @return numCoeff as int
 	 */
 	int CalPointingModelRow::getNumCoeff() const {
	
  		return numCoeff;
 	}

 	/**
 	 * Set numCoeff with the specified int.
 	 * @param numCoeff The int value to which numCoeff is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setNumCoeff (int numCoeff)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numCoeff = numCoeff;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalPointingModelRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalPointingModelRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get numFormula.
 	 * @return numFormula as int
 	 */
 	int CalPointingModelRow::getNumFormula() const {
	
  		return numFormula;
 	}

 	/**
 	 * Set numFormula with the specified int.
 	 * @param numFormula The int value to which numFormula is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setNumFormula (int numFormula)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numFormula = numFormula;
	
 	}
	
	

	

	
 	/**
 	 * Get azimuthRms.
 	 * @return azimuthRms as Angle
 	 */
 	Angle CalPointingModelRow::getAzimuthRms() const {
	
  		return azimuthRms;
 	}

 	/**
 	 * Set azimuthRms with the specified Angle.
 	 * @param azimuthRms The Angle value to which azimuthRms is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setAzimuthRms (Angle azimuthRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->azimuthRms = azimuthRms;
	
 	}
	
	

	

	
 	/**
 	 * Get elevationRms.
 	 * @return elevationRms as Angle
 	 */
 	Angle CalPointingModelRow::getElevationRms() const {
	
  		return elevationRms;
 	}

 	/**
 	 * Set elevationRms with the specified Angle.
 	 * @param elevationRms The Angle value to which elevationRms is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setElevationRms (Angle elevationRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->elevationRms = elevationRms;
	
 	}
	
	

	

	
 	/**
 	 * Get skyRms.
 	 * @return skyRms as Angle
 	 */
 	Angle CalPointingModelRow::getSkyRms() const {
	
  		return skyRms;
 	}

 	/**
 	 * Set skyRms with the specified Angle.
 	 * @param skyRms The Angle value to which skyRms is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setSkyRms (Angle skyRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->skyRms = skyRms;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffName.
 	 * @return coeffName as vector<string >
 	 */
 	vector<string > CalPointingModelRow::getCoeffName() const {
	
  		return coeffName;
 	}

 	/**
 	 * Set coeffName with the specified vector<string >.
 	 * @param coeffName The vector<string > value to which coeffName is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffName (vector<string > coeffName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffName = coeffName;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffVal.
 	 * @return coeffVal as vector<float >
 	 */
 	vector<float > CalPointingModelRow::getCoeffVal() const {
	
  		return coeffVal;
 	}

 	/**
 	 * Set coeffVal with the specified vector<float >.
 	 * @param coeffVal The vector<float > value to which coeffVal is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffVal (vector<float > coeffVal)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffVal = coeffVal;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffError.
 	 * @return coeffError as vector<float >
 	 */
 	vector<float > CalPointingModelRow::getCoeffError() const {
	
  		return coeffError;
 	}

 	/**
 	 * Set coeffError with the specified vector<float >.
 	 * @param coeffError The vector<float > value to which coeffError is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffError (vector<float > coeffError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffError = coeffError;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffFixed.
 	 * @return coeffFixed as vector<bool >
 	 */
 	vector<bool > CalPointingModelRow::getCoeffFixed() const {
	
  		return coeffFixed;
 	}

 	/**
 	 * Set coeffFixed with the specified vector<bool >.
 	 * @param coeffFixed The vector<bool > value to which coeffFixed is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffFixed (vector<bool > coeffFixed)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffFixed = coeffFixed;
	
 	}
	
	

	

	
 	/**
 	 * Get coeffFormula.
 	 * @return coeffFormula as vector<string >
 	 */
 	vector<string > CalPointingModelRow::getCoeffFormula() const {
	
  		return coeffFormula;
 	}

 	/**
 	 * Set coeffFormula with the specified vector<string >.
 	 * @param coeffFormula The vector<string > value to which coeffFormula is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setCoeffFormula (vector<string > coeffFormula)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->coeffFormula = coeffFormula;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingModelMode.
 	 * @return pointingModelMode as PointingModelModeMod::PointingModelMode
 	 */
 	PointingModelModeMod::PointingModelMode CalPointingModelRow::getPointingModelMode() const {
	
  		return pointingModelMode;
 	}

 	/**
 	 * Set pointingModelMode with the specified PointingModelModeMod::PointingModelMode.
 	 * @param pointingModelMode The PointingModelModeMod::PointingModelMode value to which pointingModelMode is to be set.
 	 
 	
 		
 	 */
 	void CalPointingModelRow::setPointingModelMode (PointingModelModeMod::PointingModelMode pointingModelMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingModelMode = pointingModelMode;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalPointingModelRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingModelRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalPointingModel");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalPointingModelRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingModelRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalPointingModel");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalPointingModelRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPointingModelRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalPointingModel");
		
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
	 CalDataRow* CalPointingModelRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalPointingModelRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalPointingModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPointingModelRow::CalPointingModelRow (CalPointingModelTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
antennaMake = CAntennaMake::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
pointingModelMode = CPointingModelMode::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	
	
	}
	
	CalPointingModelRow::CalPointingModelRow (CalPointingModelTable &t, CalPointingModelRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
			antennaName = row.antennaName;
		
			receiverBand = row.receiverBand;
		
		
		
		
			antennaMake = row.antennaMake;
		
			numObs = row.numObs;
		
			numCoeff = row.numCoeff;
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			numFormula = row.numFormula;
		
			azimuthRms = row.azimuthRms;
		
			elevationRms = row.elevationRms;
		
			skyRms = row.skyRms;
		
			coeffName = row.coeffName;
		
			coeffVal = row.coeffVal;
		
			coeffError = row.coeffError;
		
			coeffFixed = row.coeffFixed;
		
			coeffFormula = row.coeffFormula;
		
			pointingModelMode = row.pointingModelMode;
		
		
		
		
		}	
	}

	
	bool CalPointingModelRow::compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, ReceiverBandMod::ReceiverBand receiverBand, AntennaMakeMod::AntennaMake antennaMake, int numObs, int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, int numFormula, Angle azimuthRms, Angle elevationRms, Angle skyRms, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, vector<string > coeffFormula, PointingModelModeMod::PointingModelMode pointingModelMode) {
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
	

	
		
		result = result && (this->antennaMake == antennaMake);
		
		if (!result) return false;
	

	
		
		result = result && (this->numObs == numObs);
		
		if (!result) return false;
	

	
		
		result = result && (this->numCoeff == numCoeff);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->numFormula == numFormula);
		
		if (!result) return false;
	

	
		
		result = result && (this->azimuthRms == azimuthRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->elevationRms == elevationRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->skyRms == skyRms);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffName == coeffName);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffVal == coeffVal);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffError == coeffError);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffFixed == coeffFixed);
		
		if (!result) return false;
	

	
		
		result = result && (this->coeffFormula == coeffFormula);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingModelMode == pointingModelMode);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalPointingModelRow::compareRequiredValue(AntennaMakeMod::AntennaMake antennaMake, int numObs, int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, int numFormula, Angle azimuthRms, Angle elevationRms, Angle skyRms, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, vector<string > coeffFormula, PointingModelModeMod::PointingModelMode pointingModelMode) {
		bool result;
		result = true;
		
	
		if (!(this->antennaMake == antennaMake)) return false;
	

	
		if (!(this->numObs == numObs)) return false;
	

	
		if (!(this->numCoeff == numCoeff)) return false;
	

	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->numFormula == numFormula)) return false;
	

	
		if (!(this->azimuthRms == azimuthRms)) return false;
	

	
		if (!(this->elevationRms == elevationRms)) return false;
	

	
		if (!(this->skyRms == skyRms)) return false;
	

	
		if (!(this->coeffName == coeffName)) return false;
	

	
		if (!(this->coeffVal == coeffVal)) return false;
	

	
		if (!(this->coeffError == coeffError)) return false;
	

	
		if (!(this->coeffFixed == coeffFixed)) return false;
	

	
		if (!(this->coeffFormula == coeffFormula)) return false;
	

	
		if (!(this->pointingModelMode == pointingModelMode)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPointingModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalPointingModelRow::equalByRequiredValue(CalPointingModelRow* x) {
		
			
		if (this->antennaMake != x->antennaMake) return false;
			
		if (this->numObs != x->numObs) return false;
			
		if (this->numCoeff != x->numCoeff) return false;
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->numFormula != x->numFormula) return false;
			
		if (this->azimuthRms != x->azimuthRms) return false;
			
		if (this->elevationRms != x->elevationRms) return false;
			
		if (this->skyRms != x->skyRms) return false;
			
		if (this->coeffName != x->coeffName) return false;
			
		if (this->coeffVal != x->coeffVal) return false;
			
		if (this->coeffError != x->coeffError) return false;
			
		if (this->coeffFixed != x->coeffFixed) return false;
			
		if (this->coeffFormula != x->coeffFormula) return false;
			
		if (this->pointingModelMode != x->pointingModelMode) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
