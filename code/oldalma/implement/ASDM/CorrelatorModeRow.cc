
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
 * File CorrelatorModeRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CorrelatorModeRow.h>
#include <CorrelatorModeTable.h>
	

using asdm::ASDM;
using asdm::CorrelatorModeRow;
using asdm::CorrelatorModeTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	CorrelatorModeRow::~CorrelatorModeRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CorrelatorModeTable &CorrelatorModeRow::getTable() const {
		return table;
	}
	
	void CorrelatorModeRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CorrelatorModeRowIDL struct.
	 */
	CorrelatorModeRowIDL *CorrelatorModeRow::toIDL() const {
		CorrelatorModeRowIDL *x = new CorrelatorModeRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->almaCorrelatorModeId = almaCorrelatorModeId.toIDLTag();
			
		
	

	
  		
		
		
			
				
		x->numBaseband = numBaseband;
 				
 			
		
	

	
  		
		
		
			
		x->basebandNames.length(basebandNames.size());
		for (unsigned int i = 0; i < basebandNames.size(); ++i) {
			
				
			x->basebandNames[i] = basebandNames.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->basebandConfig.length(basebandConfig.size());
		for (unsigned int i = 0; i < basebandConfig.size(); ++i) {
			
				
			x->basebandConfig[i] = basebandConfig.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->accumMode = accumMode;
 				
 			
		
	

	
  		
		
		
			
				
		x->binMode = binMode;
 				
 			
		
	

	
  		
		
		
			
		x->axesOrderArray.length(axesOrderArray.size());
		for (unsigned int i = 0; i < axesOrderArray.size(); ++i) {
			
				
			x->axesOrderArray[i] = axesOrderArray.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->filterMode.length(filterMode.size());
		for (unsigned int i = 0; i < filterMode.size(); ++i) {
			
				
			x->filterMode[i] = filterMode.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->correlatorName = correlatorName;
 				
 			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CorrelatorModeRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CorrelatorModeRow::setFromIDL (CorrelatorModeRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAlmaCorrelatorModeId(Tag (x.almaCorrelatorModeId));
			
 		
		
	

	
		
		
			
		setNumBaseband(x.numBaseband);
  			
 		
		
	

	
		
		
			
		basebandNames .clear();
		for (unsigned int i = 0; i <x.basebandNames.length(); ++i) {
			
			basebandNames.push_back(x.basebandNames[i]);
  			
		}
			
  		
		
	

	
		
		
			
		basebandConfig .clear();
		for (unsigned int i = 0; i <x.basebandConfig.length(); ++i) {
			
			basebandConfig.push_back(x.basebandConfig[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setAccumMode(x.accumMode);
  			
 		
		
	

	
		
		
			
		setBinMode(x.binMode);
  			
 		
		
	

	
		
		
			
		axesOrderArray .clear();
		for (unsigned int i = 0; i <x.axesOrderArray.length(); ++i) {
			
			axesOrderArray.push_back(x.axesOrderArray[i]);
  			
		}
			
  		
		
	

	
		
		
			
		filterMode .clear();
		for (unsigned int i = 0; i <x.filterMode.length(); ++i) {
			
			filterMode.push_back(x.filterMode[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setCorrelatorName(x.correlatorName);
  			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CorrelatorMode");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CorrelatorModeRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(almaCorrelatorModeId, "almaCorrelatorModeId", buf);
		
		
	

  	
 		
		
		Parser::toXML(numBaseband, "numBaseband", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("basebandNames", basebandNames));
		
		
	

  	
 		
		
		Parser::toXML(basebandConfig, "basebandConfig", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("accumMode", accumMode));
		
		
	

  	
 		
		
		Parser::toXML(binMode, "binMode", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("axesOrderArray", axesOrderArray));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("filterMode", filterMode));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("correlatorName", correlatorName));
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CorrelatorModeRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAlmaCorrelatorModeId(Parser::getTag("almaCorrelatorModeId","CorrelatorMode",rowDoc));
			
		
	

	
  		
			
	  	setNumBaseband(Parser::getInteger("numBaseband","CorrelatorMode",rowDoc));
			
		
	

	
		
		
		
		basebandNames = EnumerationParser::getBasebandName1D("basebandNames","CorrelatorMode",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setBasebandConfig(Parser::get1DInteger("basebandConfig","CorrelatorMode",rowDoc));
	  			
	  		
		
	

	
		
		
		
		accumMode = EnumerationParser::getAccumMode("accumMode","CorrelatorMode",rowDoc);
		
		
		
	

	
  		
			
	  	setBinMode(Parser::getInteger("binMode","CorrelatorMode",rowDoc));
			
		
	

	
		
		
		
		axesOrderArray = EnumerationParser::getAxisName1D("axesOrderArray","CorrelatorMode",rowDoc);			
		
		
		
	

	
		
		
		
		filterMode = EnumerationParser::getFilterMode1D("filterMode","CorrelatorMode",rowDoc);			
		
		
		
	

	
		
		
		
		correlatorName = EnumerationParser::getCorrelatorName("correlatorName","CorrelatorMode",rowDoc);
		
		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CorrelatorMode");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get almaCorrelatorModeId.
 	 * @return almaCorrelatorModeId as Tag
 	 */
 	Tag CorrelatorModeRow::getAlmaCorrelatorModeId() const {
	
  		return almaCorrelatorModeId;
 	}

 	/**
 	 * Set almaCorrelatorModeId with the specified Tag.
 	 * @param almaCorrelatorModeId The Tag value to which almaCorrelatorModeId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CorrelatorModeRow::setAlmaCorrelatorModeId (Tag almaCorrelatorModeId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("almaCorrelatorModeId", "CorrelatorMode");
		
  		}
  	
 		this->almaCorrelatorModeId = almaCorrelatorModeId;
	
 	}
	
	

	

	
 	/**
 	 * Get numBaseband.
 	 * @return numBaseband as int
 	 */
 	int CorrelatorModeRow::getNumBaseband() const {
	
  		return numBaseband;
 	}

 	/**
 	 * Set numBaseband with the specified int.
 	 * @param numBaseband The int value to which numBaseband is to be set.
 	 
 	
 		
 	 */
 	void CorrelatorModeRow::setNumBaseband (int numBaseband)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numBaseband = numBaseband;
	
 	}
	
	

	

	
 	/**
 	 * Get basebandNames.
 	 * @return basebandNames as vector<BasebandNameMod::BasebandName >
 	 */
 	vector<BasebandNameMod::BasebandName > CorrelatorModeRow::getBasebandNames() const {
	
  		return basebandNames;
 	}

 	/**
 	 * Set basebandNames with the specified vector<BasebandNameMod::BasebandName >.
 	 * @param basebandNames The vector<BasebandNameMod::BasebandName > value to which basebandNames is to be set.
 	 
 	
 		
 	 */
 	void CorrelatorModeRow::setBasebandNames (vector<BasebandNameMod::BasebandName > basebandNames)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->basebandNames = basebandNames;
	
 	}
	
	

	

	
 	/**
 	 * Get basebandConfig.
 	 * @return basebandConfig as vector<int >
 	 */
 	vector<int > CorrelatorModeRow::getBasebandConfig() const {
	
  		return basebandConfig;
 	}

 	/**
 	 * Set basebandConfig with the specified vector<int >.
 	 * @param basebandConfig The vector<int > value to which basebandConfig is to be set.
 	 
 	
 		
 	 */
 	void CorrelatorModeRow::setBasebandConfig (vector<int > basebandConfig)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->basebandConfig = basebandConfig;
	
 	}
	
	

	

	
 	/**
 	 * Get accumMode.
 	 * @return accumMode as AccumModeMod::AccumMode
 	 */
 	AccumModeMod::AccumMode CorrelatorModeRow::getAccumMode() const {
	
  		return accumMode;
 	}

 	/**
 	 * Set accumMode with the specified AccumModeMod::AccumMode.
 	 * @param accumMode The AccumModeMod::AccumMode value to which accumMode is to be set.
 	 
 	
 		
 	 */
 	void CorrelatorModeRow::setAccumMode (AccumModeMod::AccumMode accumMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->accumMode = accumMode;
	
 	}
	
	

	

	
 	/**
 	 * Get binMode.
 	 * @return binMode as int
 	 */
 	int CorrelatorModeRow::getBinMode() const {
	
  		return binMode;
 	}

 	/**
 	 * Set binMode with the specified int.
 	 * @param binMode The int value to which binMode is to be set.
 	 
 	
 		
 	 */
 	void CorrelatorModeRow::setBinMode (int binMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->binMode = binMode;
	
 	}
	
	

	

	
 	/**
 	 * Get axesOrderArray.
 	 * @return axesOrderArray as vector<AxisNameMod::AxisName >
 	 */
 	vector<AxisNameMod::AxisName > CorrelatorModeRow::getAxesOrderArray() const {
	
  		return axesOrderArray;
 	}

 	/**
 	 * Set axesOrderArray with the specified vector<AxisNameMod::AxisName >.
 	 * @param axesOrderArray The vector<AxisNameMod::AxisName > value to which axesOrderArray is to be set.
 	 
 	
 		
 	 */
 	void CorrelatorModeRow::setAxesOrderArray (vector<AxisNameMod::AxisName > axesOrderArray)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->axesOrderArray = axesOrderArray;
	
 	}
	
	

	

	
 	/**
 	 * Get filterMode.
 	 * @return filterMode as vector<FilterModeMod::FilterMode >
 	 */
 	vector<FilterModeMod::FilterMode > CorrelatorModeRow::getFilterMode() const {
	
  		return filterMode;
 	}

 	/**
 	 * Set filterMode with the specified vector<FilterModeMod::FilterMode >.
 	 * @param filterMode The vector<FilterModeMod::FilterMode > value to which filterMode is to be set.
 	 
 	
 		
 	 */
 	void CorrelatorModeRow::setFilterMode (vector<FilterModeMod::FilterMode > filterMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->filterMode = filterMode;
	
 	}
	
	

	

	
 	/**
 	 * Get correlatorName.
 	 * @return correlatorName as CorrelatorNameMod::CorrelatorName
 	 */
 	CorrelatorNameMod::CorrelatorName CorrelatorModeRow::getCorrelatorName() const {
	
  		return correlatorName;
 	}

 	/**
 	 * Set correlatorName with the specified CorrelatorNameMod::CorrelatorName.
 	 * @param correlatorName The CorrelatorNameMod::CorrelatorName value to which correlatorName is to be set.
 	 
 	
 		
 	 */
 	void CorrelatorModeRow::setCorrelatorName (CorrelatorNameMod::CorrelatorName correlatorName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->correlatorName = correlatorName;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a CorrelatorModeRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CorrelatorModeRow::CorrelatorModeRow (CorrelatorModeTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	
	
	
	
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
accumMode = CAccumMode::from_int(0);
	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
correlatorName = CCorrelatorName::from_int(0);
	
	
	}
	
	CorrelatorModeRow::CorrelatorModeRow (CorrelatorModeTable &t, CorrelatorModeRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

			
		}
		else {
	
		
			almaCorrelatorModeId = row.almaCorrelatorModeId;
		
		
		
		
			numBaseband = row.numBaseband;
		
			basebandNames = row.basebandNames;
		
			basebandConfig = row.basebandConfig;
		
			accumMode = row.accumMode;
		
			binMode = row.binMode;
		
			axesOrderArray = row.axesOrderArray;
		
			filterMode = row.filterMode;
		
			correlatorName = row.correlatorName;
		
		
		
		
		}	
	}

	
	bool CorrelatorModeRow::compareNoAutoInc(int numBaseband, vector<BasebandNameMod::BasebandName > basebandNames, vector<int > basebandConfig, AccumModeMod::AccumMode accumMode, int binMode, vector<AxisNameMod::AxisName > axesOrderArray, vector<FilterModeMod::FilterMode > filterMode, CorrelatorNameMod::CorrelatorName correlatorName) {
		bool result;
		result = true;
		
	
		
		result = result && (this->numBaseband == numBaseband);
		
		if (!result) return false;
	

	
		
		result = result && (this->basebandNames == basebandNames);
		
		if (!result) return false;
	

	
		
		result = result && (this->basebandConfig == basebandConfig);
		
		if (!result) return false;
	

	
		
		result = result && (this->accumMode == accumMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->binMode == binMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->axesOrderArray == axesOrderArray);
		
		if (!result) return false;
	

	
		
		result = result && (this->filterMode == filterMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->correlatorName == correlatorName);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CorrelatorModeRow::compareRequiredValue(int numBaseband, vector<BasebandNameMod::BasebandName > basebandNames, vector<int > basebandConfig, AccumModeMod::AccumMode accumMode, int binMode, vector<AxisNameMod::AxisName > axesOrderArray, vector<FilterModeMod::FilterMode > filterMode, CorrelatorNameMod::CorrelatorName correlatorName) {
		bool result;
		result = true;
		
	
		if (!(this->numBaseband == numBaseband)) return false;
	

	
		if (!(this->basebandNames == basebandNames)) return false;
	

	
		if (!(this->basebandConfig == basebandConfig)) return false;
	

	
		if (!(this->accumMode == accumMode)) return false;
	

	
		if (!(this->binMode == binMode)) return false;
	

	
		if (!(this->axesOrderArray == axesOrderArray)) return false;
	

	
		if (!(this->filterMode == filterMode)) return false;
	

	
		if (!(this->correlatorName == correlatorName)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CorrelatorModeRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CorrelatorModeRow::equalByRequiredValue(CorrelatorModeRow* x) {
		
			
		if (this->numBaseband != x->numBaseband) return false;
			
		if (this->basebandNames != x->basebandNames) return false;
			
		if (this->basebandConfig != x->basebandConfig) return false;
			
		if (this->accumMode != x->accumMode) return false;
			
		if (this->binMode != x->binMode) return false;
			
		if (this->axesOrderArray != x->axesOrderArray) return false;
			
		if (this->filterMode != x->filterMode) return false;
			
		if (this->correlatorName != x->correlatorName) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
