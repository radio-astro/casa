
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
 * File SBSummaryRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <SBSummaryRow.h>
#include <SBSummaryTable.h>
	

using asdm::ASDM;
using asdm::SBSummaryRow;
using asdm::SBSummaryTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	SBSummaryRow::~SBSummaryRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	SBSummaryTable &SBSummaryRow::getTable() const {
		return table;
	}
	
	void SBSummaryRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SBSummaryRowIDL struct.
	 */
	SBSummaryRowIDL *SBSummaryRow::toIDL() const {
		SBSummaryRowIDL *x = new SBSummaryRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->sbId = sbId.toIDLEntityRef();
			
		
	

	
  		
		
		
			
		x->projectId = projectId.toIDLEntityRef();
			
		
	

	
  		
		
		
			
		x->obsUnitSetId = obsUnitSetId.toIDLEntityRef();
			
		
	

	
  		
		
		
			
				
		x->sbIntent = CORBA::string_dup(sbIntent.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->sbType = sbType;
 				
 			
		
	

	
  		
		
		
			
		x->sbDuration = sbDuration.toIDLInterval();
			
		
	

	
  		
		
		
			
				
		x->numScan = numScan;
 				
 			
		
	

	
  		
		
		
			
		x->scanIntent.length(scanIntent.size());
		for (unsigned int i = 0; i < scanIntent.size(); ++i) {
			
				
			x->scanIntent[i] = CORBA::string_dup(scanIntent.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->numberRepeats = numberRepeats;
 				
 			
		
	

	
  		
		
		
			
		x->weatherConstraint.length(weatherConstraint.size());
		for (unsigned int i = 0; i < weatherConstraint.size(); ++i) {
			
				
			x->weatherConstraint[i] = CORBA::string_dup(weatherConstraint.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->scienceGoal.length(scienceGoal.size());
		for (unsigned int i = 0; i < scienceGoal.size(); ++i) {
			
				
			x->scienceGoal[i] = CORBA::string_dup(scienceGoal.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->raCenter = raCenter;
 				
 			
		
	

	
  		
		
		
			
				
		x->decCenter = decCenter;
 				
 			
		
	

	
  		
		
		
			
				
		x->frequency = frequency;
 				
 			
		
	

	
  		
		
		
			
				
		x->frequencyBand = CORBA::string_dup(frequencyBand.c_str());
				
 			
		
	

	
  		
		
		
			
		x->observingMode.length(observingMode.size());
		for (unsigned int i = 0; i < observingMode.size(); ++i) {
			
				
			x->observingMode[i] = CORBA::string_dup(observingMode.at(i).c_str());
				
	 		
	 	}
			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SBSummaryRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void SBSummaryRow::setFromIDL (SBSummaryRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setSbId(EntityRef (x.sbId));
			
 		
		
	

	
		
		
			
		setProjectId(EntityRef (x.projectId));
			
 		
		
	

	
		
		
			
		setObsUnitSetId(EntityRef (x.obsUnitSetId));
			
 		
		
	

	
		
		
			
		setSbIntent(string (x.sbIntent));
			
 		
		
	

	
		
		
			
		setSbType(x.sbType);
  			
 		
		
	

	
		
		
			
		setSbDuration(Interval (x.sbDuration));
			
 		
		
	

	
		
		
			
		setNumScan(x.numScan);
  			
 		
		
	

	
		
		
			
		scanIntent .clear();
		for (unsigned int i = 0; i <x.scanIntent.length(); ++i) {
			
			scanIntent.push_back(string (x.scanIntent[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumberRepeats(x.numberRepeats);
  			
 		
		
	

	
		
		
			
		weatherConstraint .clear();
		for (unsigned int i = 0; i <x.weatherConstraint.length(); ++i) {
			
			weatherConstraint.push_back(string (x.weatherConstraint[i]));
			
		}
			
  		
		
	

	
		
		
			
		scienceGoal .clear();
		for (unsigned int i = 0; i <x.scienceGoal.length(); ++i) {
			
			scienceGoal.push_back(string (x.scienceGoal[i]));
			
		}
			
  		
		
	

	
		
		
			
		setRaCenter(x.raCenter);
  			
 		
		
	

	
		
		
			
		setDecCenter(x.decCenter);
  			
 		
		
	

	
		
		
			
		setFrequency(x.frequency);
  			
 		
		
	

	
		
		
			
		setFrequencyBand(string (x.frequencyBand));
			
 		
		
	

	
		
		
			
		observingMode .clear();
		for (unsigned int i = 0; i <x.observingMode.length(); ++i) {
			
			observingMode.push_back(string (x.observingMode[i]));
			
		}
			
  		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"SBSummary");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string SBSummaryRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(sbId, "sbId", buf);
		
		
	

  	
 		
		
		Parser::toXML(projectId, "projectId", buf);
		
		
	

  	
 		
		
		Parser::toXML(obsUnitSetId, "obsUnitSetId", buf);
		
		
	

  	
 		
		
		Parser::toXML(sbIntent, "sbIntent", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("sbType", sbType));
		
		
	

  	
 		
		
		Parser::toXML(sbDuration, "sbDuration", buf);
		
		
	

  	
 		
		
		Parser::toXML(numScan, "numScan", buf);
		
		
	

  	
 		
		
		Parser::toXML(scanIntent, "scanIntent", buf);
		
		
	

  	
 		
		
		Parser::toXML(numberRepeats, "numberRepeats", buf);
		
		
	

  	
 		
		
		Parser::toXML(weatherConstraint, "weatherConstraint", buf);
		
		
	

  	
 		
		
		Parser::toXML(scienceGoal, "scienceGoal", buf);
		
		
	

  	
 		
		
		Parser::toXML(raCenter, "raCenter", buf);
		
		
	

  	
 		
		
		Parser::toXML(decCenter, "decCenter", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequency, "frequency", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyBand, "frequencyBand", buf);
		
		
	

  	
 		
		
		Parser::toXML(observingMode, "observingMode", buf);
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void SBSummaryRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setSbId(Parser::getEntityRef("sbId","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setProjectId(Parser::getEntityRef("projectId","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setObsUnitSetId(Parser::getEntityRef("obsUnitSetId","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setSbIntent(Parser::getString("sbIntent","SBSummary",rowDoc));
			
		
	

	
		
		
		
		sbType = EnumerationParser::getSBType("sbType","SBSummary",rowDoc);
		
		
		
	

	
  		
			
	  	setSbDuration(Parser::getInterval("sbDuration","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setNumScan(Parser::getInteger("numScan","SBSummary",rowDoc));
			
		
	

	
  		
			
					
	  	setScanIntent(Parser::get1DString("scanIntent","SBSummary",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumberRepeats(Parser::getInteger("numberRepeats","SBSummary",rowDoc));
			
		
	

	
  		
			
					
	  	setWeatherConstraint(Parser::get1DString("weatherConstraint","SBSummary",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setScienceGoal(Parser::get1DString("scienceGoal","SBSummary",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setRaCenter(Parser::getDouble("raCenter","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setDecCenter(Parser::getDouble("decCenter","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setFrequency(Parser::getDouble("frequency","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setFrequencyBand(Parser::getString("frequencyBand","SBSummary",rowDoc));
			
		
	

	
  		
			
					
	  	setObservingMode(Parser::get1DString("observingMode","SBSummary",rowDoc));
	  			
	  		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SBSummary");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get sbId.
 	 * @return sbId as EntityRef
 	 */
 	EntityRef SBSummaryRow::getSbId() const {
	
  		return sbId;
 	}

 	/**
 	 * Set sbId with the specified EntityRef.
 	 * @param sbId The EntityRef value to which sbId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SBSummaryRow::setSbId (EntityRef sbId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("sbId", "SBSummary");
		
  		}
  	
 		this->sbId = sbId;
	
 	}
	
	

	

	
 	/**
 	 * Get projectId.
 	 * @return projectId as EntityRef
 	 */
 	EntityRef SBSummaryRow::getProjectId() const {
	
  		return projectId;
 	}

 	/**
 	 * Set projectId with the specified EntityRef.
 	 * @param projectId The EntityRef value to which projectId is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setProjectId (EntityRef projectId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->projectId = projectId;
	
 	}
	
	

	

	
 	/**
 	 * Get obsUnitSetId.
 	 * @return obsUnitSetId as EntityRef
 	 */
 	EntityRef SBSummaryRow::getObsUnitSetId() const {
	
  		return obsUnitSetId;
 	}

 	/**
 	 * Set obsUnitSetId with the specified EntityRef.
 	 * @param obsUnitSetId The EntityRef value to which obsUnitSetId is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setObsUnitSetId (EntityRef obsUnitSetId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->obsUnitSetId = obsUnitSetId;
	
 	}
	
	

	

	
 	/**
 	 * Get sbIntent.
 	 * @return sbIntent as string
 	 */
 	string SBSummaryRow::getSbIntent() const {
	
  		return sbIntent;
 	}

 	/**
 	 * Set sbIntent with the specified string.
 	 * @param sbIntent The string value to which sbIntent is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setSbIntent (string sbIntent)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sbIntent = sbIntent;
	
 	}
	
	

	

	
 	/**
 	 * Get sbType.
 	 * @return sbType as SBTypeMod::SBType
 	 */
 	SBTypeMod::SBType SBSummaryRow::getSbType() const {
	
  		return sbType;
 	}

 	/**
 	 * Set sbType with the specified SBTypeMod::SBType.
 	 * @param sbType The SBTypeMod::SBType value to which sbType is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setSbType (SBTypeMod::SBType sbType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sbType = sbType;
	
 	}
	
	

	

	
 	/**
 	 * Get sbDuration.
 	 * @return sbDuration as Interval
 	 */
 	Interval SBSummaryRow::getSbDuration() const {
	
  		return sbDuration;
 	}

 	/**
 	 * Set sbDuration with the specified Interval.
 	 * @param sbDuration The Interval value to which sbDuration is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setSbDuration (Interval sbDuration)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sbDuration = sbDuration;
	
 	}
	
	

	

	
 	/**
 	 * Get numScan.
 	 * @return numScan as int
 	 */
 	int SBSummaryRow::getNumScan() const {
	
  		return numScan;
 	}

 	/**
 	 * Set numScan with the specified int.
 	 * @param numScan The int value to which numScan is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setNumScan (int numScan)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numScan = numScan;
	
 	}
	
	

	

	
 	/**
 	 * Get scanIntent.
 	 * @return scanIntent as vector<string >
 	 */
 	vector<string > SBSummaryRow::getScanIntent() const {
	
  		return scanIntent;
 	}

 	/**
 	 * Set scanIntent with the specified vector<string >.
 	 * @param scanIntent The vector<string > value to which scanIntent is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setScanIntent (vector<string > scanIntent)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->scanIntent = scanIntent;
	
 	}
	
	

	

	
 	/**
 	 * Get numberRepeats.
 	 * @return numberRepeats as int
 	 */
 	int SBSummaryRow::getNumberRepeats() const {
	
  		return numberRepeats;
 	}

 	/**
 	 * Set numberRepeats with the specified int.
 	 * @param numberRepeats The int value to which numberRepeats is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setNumberRepeats (int numberRepeats)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numberRepeats = numberRepeats;
	
 	}
	
	

	

	
 	/**
 	 * Get weatherConstraint.
 	 * @return weatherConstraint as vector<string >
 	 */
 	vector<string > SBSummaryRow::getWeatherConstraint() const {
	
  		return weatherConstraint;
 	}

 	/**
 	 * Set weatherConstraint with the specified vector<string >.
 	 * @param weatherConstraint The vector<string > value to which weatherConstraint is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setWeatherConstraint (vector<string > weatherConstraint)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->weatherConstraint = weatherConstraint;
	
 	}
	
	

	

	
 	/**
 	 * Get scienceGoal.
 	 * @return scienceGoal as vector<string >
 	 */
 	vector<string > SBSummaryRow::getScienceGoal() const {
	
  		return scienceGoal;
 	}

 	/**
 	 * Set scienceGoal with the specified vector<string >.
 	 * @param scienceGoal The vector<string > value to which scienceGoal is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setScienceGoal (vector<string > scienceGoal)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->scienceGoal = scienceGoal;
	
 	}
	
	

	

	
 	/**
 	 * Get raCenter.
 	 * @return raCenter as double
 	 */
 	double SBSummaryRow::getRaCenter() const {
	
  		return raCenter;
 	}

 	/**
 	 * Set raCenter with the specified double.
 	 * @param raCenter The double value to which raCenter is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setRaCenter (double raCenter)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->raCenter = raCenter;
	
 	}
	
	

	

	
 	/**
 	 * Get decCenter.
 	 * @return decCenter as double
 	 */
 	double SBSummaryRow::getDecCenter() const {
	
  		return decCenter;
 	}

 	/**
 	 * Set decCenter with the specified double.
 	 * @param decCenter The double value to which decCenter is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setDecCenter (double decCenter)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->decCenter = decCenter;
	
 	}
	
	

	

	
 	/**
 	 * Get frequency.
 	 * @return frequency as double
 	 */
 	double SBSummaryRow::getFrequency() const {
	
  		return frequency;
 	}

 	/**
 	 * Set frequency with the specified double.
 	 * @param frequency The double value to which frequency is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setFrequency (double frequency)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequency = frequency;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyBand.
 	 * @return frequencyBand as string
 	 */
 	string SBSummaryRow::getFrequencyBand() const {
	
  		return frequencyBand;
 	}

 	/**
 	 * Set frequencyBand with the specified string.
 	 * @param frequencyBand The string value to which frequencyBand is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setFrequencyBand (string frequencyBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyBand = frequencyBand;
	
 	}
	
	

	

	
 	/**
 	 * Get observingMode.
 	 * @return observingMode as vector<string >
 	 */
 	vector<string > SBSummaryRow::getObservingMode() const {
	
  		return observingMode;
 	}

 	/**
 	 * Set observingMode with the specified vector<string >.
 	 * @param observingMode The vector<string > value to which observingMode is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setObservingMode (vector<string > observingMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->observingMode = observingMode;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a SBSummaryRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SBSummaryRow::SBSummaryRow (SBSummaryTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	
	
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
sbType = CSBType::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	SBSummaryRow::SBSummaryRow (SBSummaryTable &t, SBSummaryRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

			
		}
		else {
	
		
			sbId = row.sbId;
		
		
		
		
			projectId = row.projectId;
		
			obsUnitSetId = row.obsUnitSetId;
		
			sbIntent = row.sbIntent;
		
			sbType = row.sbType;
		
			sbDuration = row.sbDuration;
		
			numScan = row.numScan;
		
			scanIntent = row.scanIntent;
		
			numberRepeats = row.numberRepeats;
		
			weatherConstraint = row.weatherConstraint;
		
			scienceGoal = row.scienceGoal;
		
			raCenter = row.raCenter;
		
			decCenter = row.decCenter;
		
			frequency = row.frequency;
		
			frequencyBand = row.frequencyBand;
		
			observingMode = row.observingMode;
		
		
		
		
		}	
	}

	
	bool SBSummaryRow::compareNoAutoInc(EntityRef sbId, EntityRef projectId, EntityRef obsUnitSetId, string sbIntent, SBTypeMod::SBType sbType, Interval sbDuration, int numScan, vector<string > scanIntent, int numberRepeats, vector<string > weatherConstraint, vector<string > scienceGoal, double raCenter, double decCenter, double frequency, string frequencyBand, vector<string > observingMode) {
		bool result;
		result = true;
		
	
		
		result = result && (this->sbId == sbId);
		
		if (!result) return false;
	

	
		
		result = result && (this->projectId == projectId);
		
		if (!result) return false;
	

	
		
		result = result && (this->obsUnitSetId == obsUnitSetId);
		
		if (!result) return false;
	

	
		
		result = result && (this->sbIntent == sbIntent);
		
		if (!result) return false;
	

	
		
		result = result && (this->sbType == sbType);
		
		if (!result) return false;
	

	
		
		result = result && (this->sbDuration == sbDuration);
		
		if (!result) return false;
	

	
		
		result = result && (this->numScan == numScan);
		
		if (!result) return false;
	

	
		
		result = result && (this->scanIntent == scanIntent);
		
		if (!result) return false;
	

	
		
		result = result && (this->numberRepeats == numberRepeats);
		
		if (!result) return false;
	

	
		
		result = result && (this->weatherConstraint == weatherConstraint);
		
		if (!result) return false;
	

	
		
		result = result && (this->scienceGoal == scienceGoal);
		
		if (!result) return false;
	

	
		
		result = result && (this->raCenter == raCenter);
		
		if (!result) return false;
	

	
		
		result = result && (this->decCenter == decCenter);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequency == frequency);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyBand == frequencyBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->observingMode == observingMode);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool SBSummaryRow::compareRequiredValue(EntityRef projectId, EntityRef obsUnitSetId, string sbIntent, SBTypeMod::SBType sbType, Interval sbDuration, int numScan, vector<string > scanIntent, int numberRepeats, vector<string > weatherConstraint, vector<string > scienceGoal, double raCenter, double decCenter, double frequency, string frequencyBand, vector<string > observingMode) {
		bool result;
		result = true;
		
	
		if (!(this->projectId == projectId)) return false;
	

	
		if (!(this->obsUnitSetId == obsUnitSetId)) return false;
	

	
		if (!(this->sbIntent == sbIntent)) return false;
	

	
		if (!(this->sbType == sbType)) return false;
	

	
		if (!(this->sbDuration == sbDuration)) return false;
	

	
		if (!(this->numScan == numScan)) return false;
	

	
		if (!(this->scanIntent == scanIntent)) return false;
	

	
		if (!(this->numberRepeats == numberRepeats)) return false;
	

	
		if (!(this->weatherConstraint == weatherConstraint)) return false;
	

	
		if (!(this->scienceGoal == scienceGoal)) return false;
	

	
		if (!(this->raCenter == raCenter)) return false;
	

	
		if (!(this->decCenter == decCenter)) return false;
	

	
		if (!(this->frequency == frequency)) return false;
	

	
		if (!(this->frequencyBand == frequencyBand)) return false;
	

	
		if (!(this->observingMode == observingMode)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SBSummaryRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool SBSummaryRow::equalByRequiredValue(SBSummaryRow* x) {
		
			
		if (this->projectId != x->projectId) return false;
			
		if (this->obsUnitSetId != x->obsUnitSetId) return false;
			
		if (this->sbIntent != x->sbIntent) return false;
			
		if (this->sbType != x->sbType) return false;
			
		if (this->sbDuration != x->sbDuration) return false;
			
		if (this->numScan != x->numScan) return false;
			
		if (this->scanIntent != x->scanIntent) return false;
			
		if (this->numberRepeats != x->numberRepeats) return false;
			
		if (this->weatherConstraint != x->weatherConstraint) return false;
			
		if (this->scienceGoal != x->scienceGoal) return false;
			
		if (this->raCenter != x->raCenter) return false;
			
		if (this->decCenter != x->decCenter) return false;
			
		if (this->frequency != x->frequency) return false;
			
		if (this->frequencyBand != x->frequencyBand) return false;
			
		if (this->observingMode != x->observingMode) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
