
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
 * File WVMCalRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <WVMCalRow.h>
#include <WVMCalTable.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::WVMCalRow;
using asdm::WVMCalTable;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::AntennaTable;
using asdm::AntennaRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	WVMCalRow::~WVMCalRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	WVMCalTable &WVMCalRow::getTable() const {
		return table;
	}
	
	void WVMCalRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a WVMCalRowIDL struct.
	 */
	WVMCalRowIDL *WVMCalRow::toIDL() const {
		WVMCalRowIDL *x = new WVMCalRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		x->operationModeExists = operationModeExists;
		
		
			
				
		x->operationMode = CORBA::string_dup(operationMode.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		
			
		x->freqOrigin = freqOrigin.toIDLFrequency();
			
		
	

	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
		x->pathCoeff.length(pathCoeff.size());
		for (unsigned int i = 0; i < pathCoeff.size(); ++i) {
			
				
			x->pathCoeff[i] = pathCoeff.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->calibrationMode = CORBA::string_dup(calibrationMode.c_str());
				
 			
		
	

	
  		
		
		x->wvrefModelExists = wvrefModelExists;
		
		
			
				
		x->wvrefModel = wvrefModel;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct WVMCalRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void WVMCalRow::setFromIDL (WVMCalRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		operationModeExists = x.operationModeExists;
		if (x.operationModeExists) {
		
		
			
		setOperationMode(string (x.operationMode));
			
 		
		
		}
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		setFreqOrigin(Frequency (x.freqOrigin));
			
 		
		
	

	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		pathCoeff .clear();
		for (unsigned int i = 0; i <x.pathCoeff.length(); ++i) {
			
			pathCoeff.push_back(x.pathCoeff[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setCalibrationMode(string (x.calibrationMode));
			
 		
		
	

	
		
		wvrefModelExists = x.wvrefModelExists;
		if (x.wvrefModelExists) {
		
		
			
		setWvrefModel(x.wvrefModel);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"WVMCal");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string WVMCalRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		if (operationModeExists) {
		
		
		Parser::toXML(operationMode, "operationMode", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(freqOrigin, "freqOrigin", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(pathCoeff, "pathCoeff", buf);
		
		
	

  	
 		
		
		Parser::toXML(calibrationMode, "calibrationMode", buf);
		
		
	

  	
 		
		if (wvrefModelExists) {
		
		
		Parser::toXML(wvrefModel, "wvrefModel", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void WVMCalRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
        if (row.isStr("<operationMode>")) {
			
	  		setOperationMode(Parser::getString("operationMode","WVMCal",rowDoc));
			
		}
 		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","WVMCal",rowDoc));
			
		
	

	
  		
			
	  	setFreqOrigin(Parser::getFrequency("freqOrigin","WVMCal",rowDoc));
			
		
	

	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","WVMCal",rowDoc));
			
		
	

	
  		
			
					
	  	setPathCoeff(Parser::get1DDouble("pathCoeff","WVMCal",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setCalibrationMode(Parser::getString("calibrationMode","WVMCal",rowDoc));
			
		
	

	
  		
        if (row.isStr("<wvrefModel>")) {
			
	  		setWvrefModel(Parser::getFloat("wvrefModel","WVMCal",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"WVMCal");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	/**
	 * The attribute operationMode is optional. Return true if this attribute exists.
	 * @return true if and only if the operationMode attribute exists. 
	 */
	bool WVMCalRow::isOperationModeExists() const {
		return operationModeExists;
	}
	

	
 	/**
 	 * Get operationMode, which is optional.
 	 * @return operationMode as string
 	 * @throw IllegalAccessException If operationMode does not exist.
 	 */
 	string WVMCalRow::getOperationMode() const throw(IllegalAccessException) {
		if (!operationModeExists) {
			throw IllegalAccessException("operationMode", "WVMCal");
		}
	
  		return operationMode;
 	}

 	/**
 	 * Set operationMode with the specified string.
 	 * @param operationMode The string value to which operationMode is to be set.
 	 
 	
 	 */
 	void WVMCalRow::setOperationMode (string operationMode) {
	
 		this->operationMode = operationMode;
	
		operationModeExists = true;
	
 	}
	
	
	/**
	 * Mark operationMode, which is an optional field, as non-existent.
	 */
	void WVMCalRow::clearOperationMode () {
		operationModeExists = false;
	}
	

	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int WVMCalRow::getNumPoly() const {
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setNumPoly (int numPoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numPoly = numPoly;
	
 	}
	
	

	

	
 	/**
 	 * Get freqOrigin.
 	 * @return freqOrigin as Frequency
 	 */
 	Frequency WVMCalRow::getFreqOrigin() const {
	
  		return freqOrigin;
 	}

 	/**
 	 * Set freqOrigin with the specified Frequency.
 	 * @param freqOrigin The Frequency value to which freqOrigin is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setFreqOrigin (Frequency freqOrigin)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->freqOrigin = freqOrigin;
	
 	}
	
	

	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval WVMCalRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void WVMCalRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "WVMCal");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get pathCoeff.
 	 * @return pathCoeff as vector<double >
 	 */
 	vector<double > WVMCalRow::getPathCoeff() const {
	
  		return pathCoeff;
 	}

 	/**
 	 * Set pathCoeff with the specified vector<double >.
 	 * @param pathCoeff The vector<double > value to which pathCoeff is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setPathCoeff (vector<double > pathCoeff)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pathCoeff = pathCoeff;
	
 	}
	
	

	

	
 	/**
 	 * Get calibrationMode.
 	 * @return calibrationMode as string
 	 */
 	string WVMCalRow::getCalibrationMode() const {
	
  		return calibrationMode;
 	}

 	/**
 	 * Set calibrationMode with the specified string.
 	 * @param calibrationMode The string value to which calibrationMode is to be set.
 	 
 	
 		
 	 */
 	void WVMCalRow::setCalibrationMode (string calibrationMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->calibrationMode = calibrationMode;
	
 	}
	
	

	
	/**
	 * The attribute wvrefModel is optional. Return true if this attribute exists.
	 * @return true if and only if the wvrefModel attribute exists. 
	 */
	bool WVMCalRow::isWvrefModelExists() const {
		return wvrefModelExists;
	}
	

	
 	/**
 	 * Get wvrefModel, which is optional.
 	 * @return wvrefModel as float
 	 * @throw IllegalAccessException If wvrefModel does not exist.
 	 */
 	float WVMCalRow::getWvrefModel() const throw(IllegalAccessException) {
		if (!wvrefModelExists) {
			throw IllegalAccessException("wvrefModel", "WVMCal");
		}
	
  		return wvrefModel;
 	}

 	/**
 	 * Set wvrefModel with the specified float.
 	 * @param wvrefModel The float value to which wvrefModel is to be set.
 	 
 	
 	 */
 	void WVMCalRow::setWvrefModel (float wvrefModel) {
	
 		this->wvrefModel = wvrefModel;
	
		wvrefModelExists = true;
	
 	}
	
	
	/**
	 * Mark wvrefModel, which is an optional field, as non-existent.
	 */
	void WVMCalRow::clearWvrefModel () {
		wvrefModelExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag WVMCalRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void WVMCalRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "WVMCal");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag WVMCalRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void WVMCalRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "WVMCal");
		
  		}
  	
 		this->spectralWindowId = spectralWindowId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* WVMCalRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* WVMCalRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	/**
	 * Create a WVMCalRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	WVMCalRow::WVMCalRow (WVMCalTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	
		operationModeExists = false;
	

	

	

	

	

	

	
		wvrefModelExists = false;
	

	
	

	

	
	
	
	

	

	

	

	

	

	
	
	}
	
	WVMCalRow::WVMCalRow (WVMCalTable &t, WVMCalRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	
		operationModeExists = false;
	

	

	

	

	

	

	
		wvrefModelExists = false;
	

	
	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			numPoly = row.numPoly;
		
			freqOrigin = row.freqOrigin;
		
			pathCoeff = row.pathCoeff;
		
			calibrationMode = row.calibrationMode;
		
		
		
		
		if (row.operationModeExists) {
			operationMode = row.operationMode;		
			operationModeExists = true;
		}
		else
			operationModeExists = false;
		
		if (row.wvrefModelExists) {
			wvrefModel = row.wvrefModel;		
			wvrefModelExists = true;
		}
		else
			wvrefModelExists = false;
		
		}	
	}

	
	bool WVMCalRow::compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int numPoly, Frequency freqOrigin, vector<double > pathCoeff, string calibrationMode) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->freqOrigin == freqOrigin);
		
		if (!result) return false;
	

	
		
		result = result && (this->pathCoeff == pathCoeff);
		
		if (!result) return false;
	

	
		
		result = result && (this->calibrationMode == calibrationMode);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool WVMCalRow::compareRequiredValue(int numPoly, Frequency freqOrigin, vector<double > pathCoeff, string calibrationMode) {
		bool result;
		result = true;
		
	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->freqOrigin == freqOrigin)) return false;
	

	
		if (!(this->pathCoeff == pathCoeff)) return false;
	

	
		if (!(this->calibrationMode == calibrationMode)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the WVMCalRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool WVMCalRow::equalByRequiredValue(WVMCalRow* x) {
		
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->freqOrigin != x->freqOrigin) return false;
			
		if (this->pathCoeff != x->pathCoeff) return false;
			
		if (this->calibrationMode != x->calibrationMode) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
