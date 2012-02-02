
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
 * File CalDeviceRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalDeviceRow.h>
#include <CalDeviceTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <FeedTable.h>
#include <FeedRow.h>
	

using asdm::ASDM;
using asdm::CalDeviceRow;
using asdm::CalDeviceTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::FeedTable;
using asdm::FeedRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	CalDeviceRow::~CalDeviceRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalDeviceTable &CalDeviceRow::getTable() const {
		return table;
	}
	
	void CalDeviceRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalDeviceRowIDL struct.
	 */
	CalDeviceRowIDL *CalDeviceRow::toIDL() const {
		CalDeviceRowIDL *x = new CalDeviceRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->numCalload = numCalload;
 				
 			
		
	

	
  		
		
		x->noiseCalExists = noiseCalExists;
		
		
			
		x->noiseCal.length(noiseCal.size());
		for (unsigned int i = 0; i < noiseCal.size(); ++i) {
			
				
			x->noiseCal[i] = noiseCal.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->temperatureLoadExists = temperatureLoadExists;
		
		
			
		x->temperatureLoad.length(temperatureLoad.size());
		for (unsigned int i = 0; i < temperatureLoad.size(); ++i) {
			
			x->temperatureLoad[i] = temperatureLoad.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->calEffExists = calEffExists;
		
		
			
		x->calEff.length(calEff.size());
		for (unsigned int i = 0; i < calEff.size(); i++) {
			x->calEff[i].length(calEff.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < calEff.size() ; i++)
			for (unsigned int j = 0; j < calEff.at(i).size(); j++)
					
						
				x->calEff[i][j] = calEff.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->calLoadName.length(calLoadName.size());
		for (unsigned int i = 0; i < calLoadName.size(); ++i) {
			
				
			x->calLoadName[i] = calLoadName.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
				
		x->feedId = feedId;
 				
 			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalDeviceRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalDeviceRow::setFromIDL (CalDeviceRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumCalload(x.numCalload);
  			
 		
		
	

	
		
		noiseCalExists = x.noiseCalExists;
		if (x.noiseCalExists) {
		
		
			
		noiseCal .clear();
		for (unsigned int i = 0; i <x.noiseCal.length(); ++i) {
			
			noiseCal.push_back(x.noiseCal[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		temperatureLoadExists = x.temperatureLoadExists;
		if (x.temperatureLoadExists) {
		
		
			
		temperatureLoad .clear();
		for (unsigned int i = 0; i <x.temperatureLoad.length(); ++i) {
			
			temperatureLoad.push_back(Temperature (x.temperatureLoad[i]));
			
		}
			
  		
		
		}
		
	

	
		
		calEffExists = x.calEffExists;
		if (x.calEffExists) {
		
		
			
		calEff .clear();
		vector<float> v_aux_calEff;
		for (unsigned int i = 0; i < x.calEff.length(); ++i) {
			v_aux_calEff.clear();
			for (unsigned int j = 0; j < x.calEff[0].length(); ++j) {
				
				v_aux_calEff.push_back(x.calEff[i][j]);
	  			
  			}
  			calEff.push_back(v_aux_calEff);			
		}
			
  		
		
		}
		
	

	
		
		
			
		calLoadName .clear();
		for (unsigned int i = 0; i <x.calLoadName.length(); ++i) {
			
			calLoadName.push_back(x.calLoadName[i]);
  			
		}
			
  		
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"CalDevice");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalDeviceRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(numCalload, "numCalload", buf);
		
		
	

  	
 		
		if (noiseCalExists) {
		
		
		Parser::toXML(noiseCal, "noiseCal", buf);
		
		
		}
		
	

  	
 		
		if (temperatureLoadExists) {
		
		
		Parser::toXML(temperatureLoad, "temperatureLoad", buf);
		
		
		}
		
	

  	
 		
		if (calEffExists) {
		
		
		Parser::toXML(calEff, "calEff", buf);
		
		
		}
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("calLoadName", calLoadName));
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(feedId, "feedId", buf);
		
		
	

  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CalDeviceRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","CalDevice",rowDoc));
			
		
	

	
  		
			
	  	setNumCalload(Parser::getInteger("numCalload","CalDevice",rowDoc));
			
		
	

	
  		
        if (row.isStr("<noiseCal>")) {
			
								
	  		setNoiseCal(Parser::get1DDouble("noiseCal","CalDevice",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<temperatureLoad>")) {
			
								
	  		setTemperatureLoad(Parser::get1DTemperature("temperatureLoad","CalDevice",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<calEff>")) {
			
								
	  		setCalEff(Parser::get2DFloat("calEff","CalDevice",rowDoc));
	  			
	  		
		}
 		
	

	
		
		
		
		calLoadName = EnumerationParser::getCalibrationDevice1D("calLoadName","CalDevice",rowDoc);			
		
		
		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalDevice");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval CalDeviceRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDeviceRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "CalDevice");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get numCalload.
 	 * @return numCalload as int
 	 */
 	int CalDeviceRow::getNumCalload() const {
	
  		return numCalload;
 	}

 	/**
 	 * Set numCalload with the specified int.
 	 * @param numCalload The int value to which numCalload is to be set.
 	 
 	
 		
 	 */
 	void CalDeviceRow::setNumCalload (int numCalload)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numCalload = numCalload;
	
 	}
	
	

	
	/**
	 * The attribute noiseCal is optional. Return true if this attribute exists.
	 * @return true if and only if the noiseCal attribute exists. 
	 */
	bool CalDeviceRow::isNoiseCalExists() const {
		return noiseCalExists;
	}
	

	
 	/**
 	 * Get noiseCal, which is optional.
 	 * @return noiseCal as vector<double >
 	 * @throw IllegalAccessException If noiseCal does not exist.
 	 */
 	vector<double > CalDeviceRow::getNoiseCal() const throw(IllegalAccessException) {
		if (!noiseCalExists) {
			throw IllegalAccessException("noiseCal", "CalDevice");
		}
	
  		return noiseCal;
 	}

 	/**
 	 * Set noiseCal with the specified vector<double >.
 	 * @param noiseCal The vector<double > value to which noiseCal is to be set.
 	 
 	
 	 */
 	void CalDeviceRow::setNoiseCal (vector<double > noiseCal) {
	
 		this->noiseCal = noiseCal;
	
		noiseCalExists = true;
	
 	}
	
	
	/**
	 * Mark noiseCal, which is an optional field, as non-existent.
	 */
	void CalDeviceRow::clearNoiseCal () {
		noiseCalExists = false;
	}
	

	
	/**
	 * The attribute temperatureLoad is optional. Return true if this attribute exists.
	 * @return true if and only if the temperatureLoad attribute exists. 
	 */
	bool CalDeviceRow::isTemperatureLoadExists() const {
		return temperatureLoadExists;
	}
	

	
 	/**
 	 * Get temperatureLoad, which is optional.
 	 * @return temperatureLoad as vector<Temperature >
 	 * @throw IllegalAccessException If temperatureLoad does not exist.
 	 */
 	vector<Temperature > CalDeviceRow::getTemperatureLoad() const throw(IllegalAccessException) {
		if (!temperatureLoadExists) {
			throw IllegalAccessException("temperatureLoad", "CalDevice");
		}
	
  		return temperatureLoad;
 	}

 	/**
 	 * Set temperatureLoad with the specified vector<Temperature >.
 	 * @param temperatureLoad The vector<Temperature > value to which temperatureLoad is to be set.
 	 
 	
 	 */
 	void CalDeviceRow::setTemperatureLoad (vector<Temperature > temperatureLoad) {
	
 		this->temperatureLoad = temperatureLoad;
	
		temperatureLoadExists = true;
	
 	}
	
	
	/**
	 * Mark temperatureLoad, which is an optional field, as non-existent.
	 */
	void CalDeviceRow::clearTemperatureLoad () {
		temperatureLoadExists = false;
	}
	

	
	/**
	 * The attribute calEff is optional. Return true if this attribute exists.
	 * @return true if and only if the calEff attribute exists. 
	 */
	bool CalDeviceRow::isCalEffExists() const {
		return calEffExists;
	}
	

	
 	/**
 	 * Get calEff, which is optional.
 	 * @return calEff as vector<vector<float > >
 	 * @throw IllegalAccessException If calEff does not exist.
 	 */
 	vector<vector<float > > CalDeviceRow::getCalEff() const throw(IllegalAccessException) {
		if (!calEffExists) {
			throw IllegalAccessException("calEff", "CalDevice");
		}
	
  		return calEff;
 	}

 	/**
 	 * Set calEff with the specified vector<vector<float > >.
 	 * @param calEff The vector<vector<float > > value to which calEff is to be set.
 	 
 	
 	 */
 	void CalDeviceRow::setCalEff (vector<vector<float > > calEff) {
	
 		this->calEff = calEff;
	
		calEffExists = true;
	
 	}
	
	
	/**
	 * Mark calEff, which is an optional field, as non-existent.
	 */
	void CalDeviceRow::clearCalEff () {
		calEffExists = false;
	}
	

	

	
 	/**
 	 * Get calLoadName.
 	 * @return calLoadName as vector<CalibrationDeviceMod::CalibrationDevice >
 	 */
 	vector<CalibrationDeviceMod::CalibrationDevice > CalDeviceRow::getCalLoadName() const {
	
  		return calLoadName;
 	}

 	/**
 	 * Set calLoadName with the specified vector<CalibrationDeviceMod::CalibrationDevice >.
 	 * @param calLoadName The vector<CalibrationDeviceMod::CalibrationDevice > value to which calLoadName is to be set.
 	 
 	
 		
 	 */
 	void CalDeviceRow::setCalLoadName (vector<CalibrationDeviceMod::CalibrationDevice > calLoadName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->calLoadName = calLoadName;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag CalDeviceRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDeviceRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "CalDevice");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int CalDeviceRow::getFeedId() const {
	
  		return feedId;
 	}

 	/**
 	 * Set feedId with the specified int.
 	 * @param feedId The int value to which feedId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDeviceRow::setFeedId (int feedId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("feedId", "CalDevice");
		
  		}
  	
 		this->feedId = feedId;
	
 	}
	
	

	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag CalDeviceRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDeviceRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "CalDevice");
		
  		}
  	
 		this->spectralWindowId = spectralWindowId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* CalDeviceRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* CalDeviceRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of CalDevice table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having their attribut feedId == this->feedId
	 */
	vector <FeedRow *> CalDeviceRow::getFeeds() {
		
			return table.getContainer().getFeed().getRowByFeedId(feedId);
		
	}
	

	

	
	/**
	 * Create a CalDeviceRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalDeviceRow::CalDeviceRow (CalDeviceTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	
		noiseCalExists = false;
	

	
		temperatureLoadExists = false;
	

	
		calEffExists = false;
	

	

	
	

	

	

	
	
	
	

	

	

	

	

	
	
	}
	
	CalDeviceRow::CalDeviceRow (CalDeviceTable &t, CalDeviceRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	
		noiseCalExists = false;
	

	
		temperatureLoadExists = false;
	

	
		calEffExists = false;
	

	

	
	

	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			feedId = row.feedId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			numCalload = row.numCalload;
		
			calLoadName = row.calLoadName;
		
		
		
		
		if (row.noiseCalExists) {
			noiseCal = row.noiseCal;		
			noiseCalExists = true;
		}
		else
			noiseCalExists = false;
		
		if (row.temperatureLoadExists) {
			temperatureLoad = row.temperatureLoad;		
			temperatureLoadExists = true;
		}
		else
			temperatureLoadExists = false;
		
		if (row.calEffExists) {
			calEff = row.calEff;		
			calEffExists = true;
		}
		else
			calEffExists = false;
		
		}	
	}

	
	bool CalDeviceRow::compareNoAutoInc(Tag antennaId, int feedId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int numCalload, vector<CalibrationDeviceMod::CalibrationDevice > calLoadName) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->feedId == feedId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->numCalload == numCalload);
		
		if (!result) return false;
	

	
		
		result = result && (this->calLoadName == calLoadName);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalDeviceRow::compareRequiredValue(int numCalload, vector<CalibrationDeviceMod::CalibrationDevice > calLoadName) {
		bool result;
		result = true;
		
	
		if (!(this->numCalload == numCalload)) return false;
	

	
		if (!(this->calLoadName == calLoadName)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalDeviceRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalDeviceRow::equalByRequiredValue(CalDeviceRow* x) {
		
			
		if (this->numCalload != x->numCalload) return false;
			
		if (this->calLoadName != x->calLoadName) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
