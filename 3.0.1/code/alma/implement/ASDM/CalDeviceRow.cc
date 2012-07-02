
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
 				
 			
		
	

	
  		
		
		
			
		x->calLoadNames.length(calLoadNames.size());
		for (unsigned int i = 0; i < calLoadNames.size(); ++i) {
			
				
			x->calLoadNames[i] = calLoadNames.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->numReceptorExists = numReceptorExists;
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		x->calEffExists = calEffExists;
		
		
			
		x->calEff.length(calEff.size());
		for (unsigned int i = 0; i < calEff.size(); i++) {
			x->calEff[i].length(calEff.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < calEff.size() ; i++)
			for (unsigned int j = 0; j < calEff.at(i).size(); j++)
					
						
				x->calEff[i][j] = calEff.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
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
	void CalDeviceRow::setFromIDL (CalDeviceRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumCalload(x.numCalload);
  			
 		
		
	

	
		
		
			
		calLoadNames .clear();
		for (unsigned int i = 0; i <x.calLoadNames.length(); ++i) {
			
			calLoadNames.push_back(x.calLoadNames[i]);
  			
		}
			
  		
		
	

	
		
		numReceptorExists = x.numReceptorExists;
		if (x.numReceptorExists) {
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
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
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalDevice");
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
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("calLoadNames", calLoadNames));
		
		
	

  	
 		
		if (numReceptorExists) {
		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
		}
		
	

  	
 		
		if (calEffExists) {
		
		
		Parser::toXML(calEff, "calEff", buf);
		
		
		}
		
	

  	
 		
		if (noiseCalExists) {
		
		
		Parser::toXML(noiseCal, "noiseCal", buf);
		
		
		}
		
	

  	
 		
		if (temperatureLoadExists) {
		
		
		Parser::toXML(temperatureLoad, "temperatureLoad", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
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
	void CalDeviceRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","CalDevice",rowDoc));
			
		
	

	
  		
			
	  	setNumCalload(Parser::getInteger("numCalload","CalDevice",rowDoc));
			
		
	

	
		
		
		
		calLoadNames = EnumerationParser::getCalibrationDevice1D("calLoadNames","CalDevice",rowDoc);			
		
		
		
	

	
  		
        if (row.isStr("<numReceptor>")) {
			
	  		setNumReceptor(Parser::getInteger("numReceptor","CalDevice",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<calEff>")) {
			
								
	  		setCalEff(Parser::get2DFloat("calEff","CalDevice",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<noiseCal>")) {
			
								
	  		setNoiseCal(Parser::get1DDouble("noiseCal","CalDevice",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<temperatureLoad>")) {
			
								
	  		setTemperatureLoad(Parser::get1DTemperature("temperatureLoad","CalDevice",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalDevice");
		}
	}
	
	void CalDeviceRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
	spectralWindowId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(feedId);
				
		
	

	
	
		
						
			eoss.writeInt(numCalload);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) calLoadNames.size());
		for (unsigned int i = 0; i < calLoadNames.size(); i++)
				
			eoss.writeInt(calLoadNames.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(numReceptorExists);
	if (numReceptorExists) {
	
	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	}

	eoss.writeBoolean(calEffExists);
	if (calEffExists) {
	
	
	
		
		
			
		eoss.writeInt((int) calEff.size());
		eoss.writeInt((int) calEff.at(0).size());
		for (unsigned int i = 0; i < calEff.size(); i++) 
			for (unsigned int j = 0;  j < calEff.at(0).size(); j++) 
							 
				eoss.writeFloat(calEff.at(i).at(j));
				
	
						
		
	

	}

	eoss.writeBoolean(noiseCalExists);
	if (noiseCalExists) {
	
	
	
		
		
			
		eoss.writeInt((int) noiseCal.size());
		for (unsigned int i = 0; i < noiseCal.size(); i++)
				
			eoss.writeDouble(noiseCal.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(temperatureLoadExists);
	if (temperatureLoadExists) {
	
	
	
		
	Temperature::toBin(temperatureLoad, eoss);
		
	

	}

	}
	
void CalDeviceRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void CalDeviceRow::spectralWindowIdFromBin(EndianISStream& eiss) {
		
	
		
		
		spectralWindowId =  Tag::fromBin(eiss);
		
	
	
}
void CalDeviceRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void CalDeviceRow::feedIdFromBin(EndianISStream& eiss) {
		
	
	
		
			
		feedId =  eiss.readInt();
			
		
	
	
}
void CalDeviceRow::numCalloadFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numCalload =  eiss.readInt();
			
		
	
	
}
void CalDeviceRow::calLoadNamesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		calLoadNames.clear();
		
		unsigned int calLoadNamesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < calLoadNamesDim1; i++)
			
			calLoadNames.push_back(CCalibrationDevice::from_int(eiss.readInt()));
			
	

		
	
	
}

void CalDeviceRow::numReceptorFromBin(EndianISStream& eiss) {
		
	numReceptorExists = eiss.readBoolean();
	if (numReceptorExists) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	

	}
	
}
void CalDeviceRow::calEffFromBin(EndianISStream& eiss) {
		
	calEffExists = eiss.readBoolean();
	if (calEffExists) {
		
	
	
		
			
	
		calEff.clear();
		
		unsigned int calEffDim1 = eiss.readInt();
		unsigned int calEffDim2 = eiss.readInt();
		vector <float> calEffAux1;
		for (unsigned int i = 0; i < calEffDim1; i++) {
			calEffAux1.clear();
			for (unsigned int j = 0; j < calEffDim2 ; j++)			
			
			calEffAux1.push_back(eiss.readFloat());
			
			calEff.push_back(calEffAux1);
		}
	
	

		
	

	}
	
}
void CalDeviceRow::noiseCalFromBin(EndianISStream& eiss) {
		
	noiseCalExists = eiss.readBoolean();
	if (noiseCalExists) {
		
	
	
		
			
	
		noiseCal.clear();
		
		unsigned int noiseCalDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < noiseCalDim1; i++)
			
			noiseCal.push_back(eiss.readDouble());
			
	

		
	

	}
	
}
void CalDeviceRow::temperatureLoadFromBin(EndianISStream& eiss) {
		
	temperatureLoadExists = eiss.readBoolean();
	if (temperatureLoadExists) {
		
	
		
		
			
	
	temperatureLoad = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
	
	
	CalDeviceRow* CalDeviceRow::fromBin(EndianISStream& eiss, CalDeviceTable& table, const vector<string>& attributesSeq) {
		CalDeviceRow* row = new  CalDeviceRow(table);
		
		map<string, CalDeviceAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "CalDeviceTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
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
 	 * Get calLoadNames.
 	 * @return calLoadNames as vector<CalibrationDeviceMod::CalibrationDevice >
 	 */
 	vector<CalibrationDeviceMod::CalibrationDevice > CalDeviceRow::getCalLoadNames() const {
	
  		return calLoadNames;
 	}

 	/**
 	 * Set calLoadNames with the specified vector<CalibrationDeviceMod::CalibrationDevice >.
 	 * @param calLoadNames The vector<CalibrationDeviceMod::CalibrationDevice > value to which calLoadNames is to be set.
 	 
 	
 		
 	 */
 	void CalDeviceRow::setCalLoadNames (vector<CalibrationDeviceMod::CalibrationDevice > calLoadNames)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->calLoadNames = calLoadNames;
	
 	}
	
	

	
	/**
	 * The attribute numReceptor is optional. Return true if this attribute exists.
	 * @return true if and only if the numReceptor attribute exists. 
	 */
	bool CalDeviceRow::isNumReceptorExists() const {
		return numReceptorExists;
	}
	

	
 	/**
 	 * Get numReceptor, which is optional.
 	 * @return numReceptor as int
 	 * @throw IllegalAccessException If numReceptor does not exist.
 	 */
 	int CalDeviceRow::getNumReceptor() const  {
		if (!numReceptorExists) {
			throw IllegalAccessException("numReceptor", "CalDevice");
		}
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 	 */
 	void CalDeviceRow::setNumReceptor (int numReceptor) {
	
 		this->numReceptor = numReceptor;
	
		numReceptorExists = true;
	
 	}
	
	
	/**
	 * Mark numReceptor, which is an optional field, as non-existent.
	 */
	void CalDeviceRow::clearNumReceptor () {
		numReceptorExists = false;
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
 	vector<vector<float > > CalDeviceRow::getCalEff() const  {
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
 	vector<double > CalDeviceRow::getNoiseCal() const  {
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
 	vector<Temperature > CalDeviceRow::getTemperatureLoad() const  {
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
		
	
	

	

	

	
		numReceptorExists = false;
	

	
		calEffExists = false;
	

	
		noiseCalExists = false;
	

	
		temperatureLoadExists = false;
	

	
	

	

	

	
	
	
	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaId"] = &CalDeviceRow::antennaIdFromBin; 
	 fromBinMethods["spectralWindowId"] = &CalDeviceRow::spectralWindowIdFromBin; 
	 fromBinMethods["timeInterval"] = &CalDeviceRow::timeIntervalFromBin; 
	 fromBinMethods["feedId"] = &CalDeviceRow::feedIdFromBin; 
	 fromBinMethods["numCalload"] = &CalDeviceRow::numCalloadFromBin; 
	 fromBinMethods["calLoadNames"] = &CalDeviceRow::calLoadNamesFromBin; 
		
	
	 fromBinMethods["numReceptor"] = &CalDeviceRow::numReceptorFromBin; 
	 fromBinMethods["calEff"] = &CalDeviceRow::calEffFromBin; 
	 fromBinMethods["noiseCal"] = &CalDeviceRow::noiseCalFromBin; 
	 fromBinMethods["temperatureLoad"] = &CalDeviceRow::temperatureLoadFromBin; 
	
	}
	
	CalDeviceRow::CalDeviceRow (CalDeviceTable &t, CalDeviceRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	
		numReceptorExists = false;
	

	
		calEffExists = false;
	

	
		noiseCalExists = false;
	

	
		temperatureLoadExists = false;
	

	
	

	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
			feedId = row.feedId;
		
		
		
		
			numCalload = row.numCalload;
		
			calLoadNames = row.calLoadNames;
		
		
		
		
		if (row.numReceptorExists) {
			numReceptor = row.numReceptor;		
			numReceptorExists = true;
		}
		else
			numReceptorExists = false;
		
		if (row.calEffExists) {
			calEff = row.calEff;		
			calEffExists = true;
		}
		else
			calEffExists = false;
		
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
		
		}
		
		 fromBinMethods["antennaId"] = &CalDeviceRow::antennaIdFromBin; 
		 fromBinMethods["spectralWindowId"] = &CalDeviceRow::spectralWindowIdFromBin; 
		 fromBinMethods["timeInterval"] = &CalDeviceRow::timeIntervalFromBin; 
		 fromBinMethods["feedId"] = &CalDeviceRow::feedIdFromBin; 
		 fromBinMethods["numCalload"] = &CalDeviceRow::numCalloadFromBin; 
		 fromBinMethods["calLoadNames"] = &CalDeviceRow::calLoadNamesFromBin; 
			
	
		 fromBinMethods["numReceptor"] = &CalDeviceRow::numReceptorFromBin; 
		 fromBinMethods["calEff"] = &CalDeviceRow::calEffFromBin; 
		 fromBinMethods["noiseCal"] = &CalDeviceRow::noiseCalFromBin; 
		 fromBinMethods["temperatureLoad"] = &CalDeviceRow::temperatureLoadFromBin; 
			
	}

	
	bool CalDeviceRow::compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, int numCalload, vector<CalibrationDeviceMod::CalibrationDevice > calLoadNames) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->feedId == feedId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numCalload == numCalload);
		
		if (!result) return false;
	

	
		
		result = result && (this->calLoadNames == calLoadNames);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalDeviceRow::compareRequiredValue(int numCalload, vector<CalibrationDeviceMod::CalibrationDevice > calLoadNames) {
		bool result;
		result = true;
		
	
		if (!(this->numCalload == numCalload)) return false;
	

	
		if (!(this->calLoadNames == calLoadNames)) return false;
	

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
			
		if (this->calLoadNames != x->calLoadNames) return false;
			
		
		return true;
	}	
	
/*
	 map<string, CalDeviceAttributeFromBin> CalDeviceRow::initFromBinMethods() {
		map<string, CalDeviceAttributeFromBin> result;
		
		result["antennaId"] = &CalDeviceRow::antennaIdFromBin;
		result["spectralWindowId"] = &CalDeviceRow::spectralWindowIdFromBin;
		result["timeInterval"] = &CalDeviceRow::timeIntervalFromBin;
		result["feedId"] = &CalDeviceRow::feedIdFromBin;
		result["numCalload"] = &CalDeviceRow::numCalloadFromBin;
		result["calLoadNames"] = &CalDeviceRow::calLoadNamesFromBin;
		
		
		result["numReceptor"] = &CalDeviceRow::numReceptorFromBin;
		result["calEff"] = &CalDeviceRow::calEffFromBin;
		result["noiseCal"] = &CalDeviceRow::noiseCalFromBin;
		result["temperatureLoad"] = &CalDeviceRow::temperatureLoadFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
