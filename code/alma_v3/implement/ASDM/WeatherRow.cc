
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
 * File WeatherRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <WeatherRow.h>
#include <WeatherTable.h>

#include <StationTable.h>
#include <StationRow.h>
	

using asdm::ASDM;
using asdm::WeatherRow;
using asdm::WeatherTable;

using asdm::StationTable;
using asdm::StationRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	WeatherRow::~WeatherRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	WeatherTable &WeatherRow::getTable() const {
		return table;
	}

	bool WeatherRow::isAdded() const {
		return hasBeenAdded;
	}	

	void WeatherRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a WeatherRowIDL struct.
	 */
	WeatherRowIDL *WeatherRow::toIDL() const {
		WeatherRowIDL *x = new WeatherRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		x->pressureExists = pressureExists;
		
		
			
		x->pressure = pressure.toIDLPressure();
			
		
	

	
  		
		
		x->relHumidityExists = relHumidityExists;
		
		
			
		x->relHumidity = relHumidity.toIDLHumidity();
			
		
	

	
  		
		
		x->temperatureExists = temperatureExists;
		
		
			
		x->temperature = temperature.toIDLTemperature();
			
		
	

	
  		
		
		x->windDirectionExists = windDirectionExists;
		
		
			
		x->windDirection = windDirection.toIDLAngle();
			
		
	

	
  		
		
		x->windSpeedExists = windSpeedExists;
		
		
			
		x->windSpeed = windSpeed.toIDLSpeed();
			
		
	

	
  		
		
		x->windMaxExists = windMaxExists;
		
		
			
		x->windMax = windMax.toIDLSpeed();
			
		
	

	
  		
		
		x->dewPointExists = dewPointExists;
		
		
			
		x->dewPoint = dewPoint.toIDLTemperature();
			
		
	

	
  		
		
		x->numLayerExists = numLayerExists;
		
		
			
				
		x->numLayer = numLayer;
 				
 			
		
	

	
  		
		
		x->layerHeightExists = layerHeightExists;
		
		
			
		x->layerHeight.length(layerHeight.size());
		for (unsigned int i = 0; i < layerHeight.size(); ++i) {
			
			x->layerHeight[i] = layerHeight.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		x->temperatureProfileExists = temperatureProfileExists;
		
		
			
		x->temperatureProfile.length(temperatureProfile.size());
		for (unsigned int i = 0; i < temperatureProfile.size(); ++i) {
			
			x->temperatureProfile[i] = temperatureProfile.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->cloudMonitorExists = cloudMonitorExists;
		
		
			
		x->cloudMonitor = cloudMonitor.toIDLTemperature();
			
		
	

	
  		
		
		x->numWVRExists = numWVRExists;
		
		
			
				
		x->numWVR = numWVR;
 				
 			
		
	

	
  		
		
		x->wvrTempExists = wvrTempExists;
		
		
			
		x->wvrTemp.length(wvrTemp.size());
		for (unsigned int i = 0; i < wvrTemp.size(); ++i) {
			
			x->wvrTemp[i] = wvrTemp.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->waterExists = waterExists;
		
		
			
				
		x->water = water;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->stationId = stationId.toIDLTag();
			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct WeatherRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void WeatherRow::setFromIDL (WeatherRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		pressureExists = x.pressureExists;
		if (x.pressureExists) {
		
		
			
		setPressure(Pressure (x.pressure));
			
 		
		
		}
		
	

	
		
		relHumidityExists = x.relHumidityExists;
		if (x.relHumidityExists) {
		
		
			
		setRelHumidity(Humidity (x.relHumidity));
			
 		
		
		}
		
	

	
		
		temperatureExists = x.temperatureExists;
		if (x.temperatureExists) {
		
		
			
		setTemperature(Temperature (x.temperature));
			
 		
		
		}
		
	

	
		
		windDirectionExists = x.windDirectionExists;
		if (x.windDirectionExists) {
		
		
			
		setWindDirection(Angle (x.windDirection));
			
 		
		
		}
		
	

	
		
		windSpeedExists = x.windSpeedExists;
		if (x.windSpeedExists) {
		
		
			
		setWindSpeed(Speed (x.windSpeed));
			
 		
		
		}
		
	

	
		
		windMaxExists = x.windMaxExists;
		if (x.windMaxExists) {
		
		
			
		setWindMax(Speed (x.windMax));
			
 		
		
		}
		
	

	
		
		dewPointExists = x.dewPointExists;
		if (x.dewPointExists) {
		
		
			
		setDewPoint(Temperature (x.dewPoint));
			
 		
		
		}
		
	

	
		
		numLayerExists = x.numLayerExists;
		if (x.numLayerExists) {
		
		
			
		setNumLayer(x.numLayer);
  			
 		
		
		}
		
	

	
		
		layerHeightExists = x.layerHeightExists;
		if (x.layerHeightExists) {
		
		
			
		layerHeight .clear();
		for (unsigned int i = 0; i <x.layerHeight.length(); ++i) {
			
			layerHeight.push_back(Length (x.layerHeight[i]));
			
		}
			
  		
		
		}
		
	

	
		
		temperatureProfileExists = x.temperatureProfileExists;
		if (x.temperatureProfileExists) {
		
		
			
		temperatureProfile .clear();
		for (unsigned int i = 0; i <x.temperatureProfile.length(); ++i) {
			
			temperatureProfile.push_back(Temperature (x.temperatureProfile[i]));
			
		}
			
  		
		
		}
		
	

	
		
		cloudMonitorExists = x.cloudMonitorExists;
		if (x.cloudMonitorExists) {
		
		
			
		setCloudMonitor(Temperature (x.cloudMonitor));
			
 		
		
		}
		
	

	
		
		numWVRExists = x.numWVRExists;
		if (x.numWVRExists) {
		
		
			
		setNumWVR(x.numWVR);
  			
 		
		
		}
		
	

	
		
		wvrTempExists = x.wvrTempExists;
		if (x.wvrTempExists) {
		
		
			
		wvrTemp .clear();
		for (unsigned int i = 0; i <x.wvrTemp.length(); ++i) {
			
			wvrTemp.push_back(Temperature (x.wvrTemp[i]));
			
		}
			
  		
		
		}
		
	

	
		
		waterExists = x.waterExists;
		if (x.waterExists) {
		
		
			
		setWater(x.water);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setStationId(Tag (x.stationId));
			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Weather");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string WeatherRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		if (pressureExists) {
		
		
		Parser::toXML(pressure, "pressure", buf);
		
		
		}
		
	

  	
 		
		if (relHumidityExists) {
		
		
		Parser::toXML(relHumidity, "relHumidity", buf);
		
		
		}
		
	

  	
 		
		if (temperatureExists) {
		
		
		Parser::toXML(temperature, "temperature", buf);
		
		
		}
		
	

  	
 		
		if (windDirectionExists) {
		
		
		Parser::toXML(windDirection, "windDirection", buf);
		
		
		}
		
	

  	
 		
		if (windSpeedExists) {
		
		
		Parser::toXML(windSpeed, "windSpeed", buf);
		
		
		}
		
	

  	
 		
		if (windMaxExists) {
		
		
		Parser::toXML(windMax, "windMax", buf);
		
		
		}
		
	

  	
 		
		if (dewPointExists) {
		
		
		Parser::toXML(dewPoint, "dewPoint", buf);
		
		
		}
		
	

  	
 		
		if (numLayerExists) {
		
		
		Parser::toXML(numLayer, "numLayer", buf);
		
		
		}
		
	

  	
 		
		if (layerHeightExists) {
		
		
		Parser::toXML(layerHeight, "layerHeight", buf);
		
		
		}
		
	

  	
 		
		if (temperatureProfileExists) {
		
		
		Parser::toXML(temperatureProfile, "temperatureProfile", buf);
		
		
		}
		
	

  	
 		
		if (cloudMonitorExists) {
		
		
		Parser::toXML(cloudMonitor, "cloudMonitor", buf);
		
		
		}
		
	

  	
 		
		if (numWVRExists) {
		
		
		Parser::toXML(numWVR, "numWVR", buf);
		
		
		}
		
	

  	
 		
		if (wvrTempExists) {
		
		
		Parser::toXML(wvrTemp, "wvrTemp", buf);
		
		
		}
		
	

  	
 		
		if (waterExists) {
		
		
		Parser::toXML(water, "water", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(stationId, "stationId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void WeatherRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Weather",rowDoc));
			
		
	

	
  		
        if (row.isStr("<pressure>")) {
			
	  		setPressure(Parser::getPressure("pressure","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<relHumidity>")) {
			
	  		setRelHumidity(Parser::getHumidity("relHumidity","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<temperature>")) {
			
	  		setTemperature(Parser::getTemperature("temperature","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<windDirection>")) {
			
	  		setWindDirection(Parser::getAngle("windDirection","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<windSpeed>")) {
			
	  		setWindSpeed(Parser::getSpeed("windSpeed","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<windMax>")) {
			
	  		setWindMax(Parser::getSpeed("windMax","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<dewPoint>")) {
			
	  		setDewPoint(Parser::getTemperature("dewPoint","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<numLayer>")) {
			
	  		setNumLayer(Parser::getInteger("numLayer","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<layerHeight>")) {
			
								
	  		setLayerHeight(Parser::get1DLength("layerHeight","Weather",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<temperatureProfile>")) {
			
								
	  		setTemperatureProfile(Parser::get1DTemperature("temperatureProfile","Weather",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<cloudMonitor>")) {
			
	  		setCloudMonitor(Parser::getTemperature("cloudMonitor","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<numWVR>")) {
			
	  		setNumWVR(Parser::getInteger("numWVR","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<wvrTemp>")) {
			
								
	  		setWvrTemp(Parser::get1DTemperature("wvrTemp","Weather",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<water>")) {
			
	  		setWater(Parser::getDouble("water","Weather",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setStationId(Parser::getTag("stationId","Station",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Weather");
		}
	}
	
	void WeatherRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	stationId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	


	
	
	eoss.writeBoolean(pressureExists);
	if (pressureExists) {
	
	
	
		
	pressure.toBin(eoss);
		
	

	}

	eoss.writeBoolean(relHumidityExists);
	if (relHumidityExists) {
	
	
	
		
	relHumidity.toBin(eoss);
		
	

	}

	eoss.writeBoolean(temperatureExists);
	if (temperatureExists) {
	
	
	
		
	temperature.toBin(eoss);
		
	

	}

	eoss.writeBoolean(windDirectionExists);
	if (windDirectionExists) {
	
	
	
		
	windDirection.toBin(eoss);
		
	

	}

	eoss.writeBoolean(windSpeedExists);
	if (windSpeedExists) {
	
	
	
		
	windSpeed.toBin(eoss);
		
	

	}

	eoss.writeBoolean(windMaxExists);
	if (windMaxExists) {
	
	
	
		
	windMax.toBin(eoss);
		
	

	}

	eoss.writeBoolean(dewPointExists);
	if (dewPointExists) {
	
	
	
		
	dewPoint.toBin(eoss);
		
	

	}

	eoss.writeBoolean(numLayerExists);
	if (numLayerExists) {
	
	
	
		
						
			eoss.writeInt(numLayer);
				
		
	

	}

	eoss.writeBoolean(layerHeightExists);
	if (layerHeightExists) {
	
	
	
		
	Length::toBin(layerHeight, eoss);
		
	

	}

	eoss.writeBoolean(temperatureProfileExists);
	if (temperatureProfileExists) {
	
	
	
		
	Temperature::toBin(temperatureProfile, eoss);
		
	

	}

	eoss.writeBoolean(cloudMonitorExists);
	if (cloudMonitorExists) {
	
	
	
		
	cloudMonitor.toBin(eoss);
		
	

	}

	eoss.writeBoolean(numWVRExists);
	if (numWVRExists) {
	
	
	
		
						
			eoss.writeInt(numWVR);
				
		
	

	}

	eoss.writeBoolean(wvrTempExists);
	if (wvrTempExists) {
	
	
	
		
	Temperature::toBin(wvrTemp, eoss);
		
	

	}

	eoss.writeBoolean(waterExists);
	if (waterExists) {
	
	
	
		
						
			eoss.writeDouble(water);
				
		
	

	}

	}
	
void WeatherRow::stationIdFromBin(EndianISStream& eiss) {
		
	
		
		
		stationId =  Tag::fromBin(eiss);
		
	
	
}
void WeatherRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}

void WeatherRow::pressureFromBin(EndianISStream& eiss) {
		
	pressureExists = eiss.readBoolean();
	if (pressureExists) {
		
	
		
		
		pressure =  Pressure::fromBin(eiss);
		
	

	}
	
}
void WeatherRow::relHumidityFromBin(EndianISStream& eiss) {
		
	relHumidityExists = eiss.readBoolean();
	if (relHumidityExists) {
		
	
		
		
		relHumidity =  Humidity::fromBin(eiss);
		
	

	}
	
}
void WeatherRow::temperatureFromBin(EndianISStream& eiss) {
		
	temperatureExists = eiss.readBoolean();
	if (temperatureExists) {
		
	
		
		
		temperature =  Temperature::fromBin(eiss);
		
	

	}
	
}
void WeatherRow::windDirectionFromBin(EndianISStream& eiss) {
		
	windDirectionExists = eiss.readBoolean();
	if (windDirectionExists) {
		
	
		
		
		windDirection =  Angle::fromBin(eiss);
		
	

	}
	
}
void WeatherRow::windSpeedFromBin(EndianISStream& eiss) {
		
	windSpeedExists = eiss.readBoolean();
	if (windSpeedExists) {
		
	
		
		
		windSpeed =  Speed::fromBin(eiss);
		
	

	}
	
}
void WeatherRow::windMaxFromBin(EndianISStream& eiss) {
		
	windMaxExists = eiss.readBoolean();
	if (windMaxExists) {
		
	
		
		
		windMax =  Speed::fromBin(eiss);
		
	

	}
	
}
void WeatherRow::dewPointFromBin(EndianISStream& eiss) {
		
	dewPointExists = eiss.readBoolean();
	if (dewPointExists) {
		
	
		
		
		dewPoint =  Temperature::fromBin(eiss);
		
	

	}
	
}
void WeatherRow::numLayerFromBin(EndianISStream& eiss) {
		
	numLayerExists = eiss.readBoolean();
	if (numLayerExists) {
		
	
	
		
			
		numLayer =  eiss.readInt();
			
		
	

	}
	
}
void WeatherRow::layerHeightFromBin(EndianISStream& eiss) {
		
	layerHeightExists = eiss.readBoolean();
	if (layerHeightExists) {
		
	
		
		
			
	
	layerHeight = Length::from1DBin(eiss);	
	

		
	

	}
	
}
void WeatherRow::temperatureProfileFromBin(EndianISStream& eiss) {
		
	temperatureProfileExists = eiss.readBoolean();
	if (temperatureProfileExists) {
		
	
		
		
			
	
	temperatureProfile = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void WeatherRow::cloudMonitorFromBin(EndianISStream& eiss) {
		
	cloudMonitorExists = eiss.readBoolean();
	if (cloudMonitorExists) {
		
	
		
		
		cloudMonitor =  Temperature::fromBin(eiss);
		
	

	}
	
}
void WeatherRow::numWVRFromBin(EndianISStream& eiss) {
		
	numWVRExists = eiss.readBoolean();
	if (numWVRExists) {
		
	
	
		
			
		numWVR =  eiss.readInt();
			
		
	

	}
	
}
void WeatherRow::wvrTempFromBin(EndianISStream& eiss) {
		
	wvrTempExists = eiss.readBoolean();
	if (wvrTempExists) {
		
	
		
		
			
	
	wvrTemp = Temperature::from1DBin(eiss);	
	

		
	

	}
	
}
void WeatherRow::waterFromBin(EndianISStream& eiss) {
		
	waterExists = eiss.readBoolean();
	if (waterExists) {
		
	
	
		
			
		water =  eiss.readDouble();
			
		
	

	}
	
}
	
	
	WeatherRow* WeatherRow::fromBin(EndianISStream& eiss, WeatherTable& table, const vector<string>& attributesSeq) {
		WeatherRow* row = new  WeatherRow(table);
		
		map<string, WeatherAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter != row->fromBinMethods.end()) {
				(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);			
			}
			else {
				BinaryAttributeReaderFunctor* functorP = table.getUnknownAttributeBinaryReader(attributesSeq.at(i));
				if (functorP)
					(*functorP)(eiss);
				else
					throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "WeatherTable");
			}
				
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
 	ArrayTimeInterval WeatherRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void WeatherRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "Weather");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	
	/**
	 * The attribute pressure is optional. Return true if this attribute exists.
	 * @return true if and only if the pressure attribute exists. 
	 */
	bool WeatherRow::isPressureExists() const {
		return pressureExists;
	}
	

	
 	/**
 	 * Get pressure, which is optional.
 	 * @return pressure as Pressure
 	 * @throw IllegalAccessException If pressure does not exist.
 	 */
 	Pressure WeatherRow::getPressure() const  {
		if (!pressureExists) {
			throw IllegalAccessException("pressure", "Weather");
		}
	
  		return pressure;
 	}

 	/**
 	 * Set pressure with the specified Pressure.
 	 * @param pressure The Pressure value to which pressure is to be set.
 	 
 	
 	 */
 	void WeatherRow::setPressure (Pressure pressure) {
	
 		this->pressure = pressure;
	
		pressureExists = true;
	
 	}
	
	
	/**
	 * Mark pressure, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearPressure () {
		pressureExists = false;
	}
	

	
	/**
	 * The attribute relHumidity is optional. Return true if this attribute exists.
	 * @return true if and only if the relHumidity attribute exists. 
	 */
	bool WeatherRow::isRelHumidityExists() const {
		return relHumidityExists;
	}
	

	
 	/**
 	 * Get relHumidity, which is optional.
 	 * @return relHumidity as Humidity
 	 * @throw IllegalAccessException If relHumidity does not exist.
 	 */
 	Humidity WeatherRow::getRelHumidity() const  {
		if (!relHumidityExists) {
			throw IllegalAccessException("relHumidity", "Weather");
		}
	
  		return relHumidity;
 	}

 	/**
 	 * Set relHumidity with the specified Humidity.
 	 * @param relHumidity The Humidity value to which relHumidity is to be set.
 	 
 	
 	 */
 	void WeatherRow::setRelHumidity (Humidity relHumidity) {
	
 		this->relHumidity = relHumidity;
	
		relHumidityExists = true;
	
 	}
	
	
	/**
	 * Mark relHumidity, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearRelHumidity () {
		relHumidityExists = false;
	}
	

	
	/**
	 * The attribute temperature is optional. Return true if this attribute exists.
	 * @return true if and only if the temperature attribute exists. 
	 */
	bool WeatherRow::isTemperatureExists() const {
		return temperatureExists;
	}
	

	
 	/**
 	 * Get temperature, which is optional.
 	 * @return temperature as Temperature
 	 * @throw IllegalAccessException If temperature does not exist.
 	 */
 	Temperature WeatherRow::getTemperature() const  {
		if (!temperatureExists) {
			throw IllegalAccessException("temperature", "Weather");
		}
	
  		return temperature;
 	}

 	/**
 	 * Set temperature with the specified Temperature.
 	 * @param temperature The Temperature value to which temperature is to be set.
 	 
 	
 	 */
 	void WeatherRow::setTemperature (Temperature temperature) {
	
 		this->temperature = temperature;
	
		temperatureExists = true;
	
 	}
	
	
	/**
	 * Mark temperature, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearTemperature () {
		temperatureExists = false;
	}
	

	
	/**
	 * The attribute windDirection is optional. Return true if this attribute exists.
	 * @return true if and only if the windDirection attribute exists. 
	 */
	bool WeatherRow::isWindDirectionExists() const {
		return windDirectionExists;
	}
	

	
 	/**
 	 * Get windDirection, which is optional.
 	 * @return windDirection as Angle
 	 * @throw IllegalAccessException If windDirection does not exist.
 	 */
 	Angle WeatherRow::getWindDirection() const  {
		if (!windDirectionExists) {
			throw IllegalAccessException("windDirection", "Weather");
		}
	
  		return windDirection;
 	}

 	/**
 	 * Set windDirection with the specified Angle.
 	 * @param windDirection The Angle value to which windDirection is to be set.
 	 
 	
 	 */
 	void WeatherRow::setWindDirection (Angle windDirection) {
	
 		this->windDirection = windDirection;
	
		windDirectionExists = true;
	
 	}
	
	
	/**
	 * Mark windDirection, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearWindDirection () {
		windDirectionExists = false;
	}
	

	
	/**
	 * The attribute windSpeed is optional. Return true if this attribute exists.
	 * @return true if and only if the windSpeed attribute exists. 
	 */
	bool WeatherRow::isWindSpeedExists() const {
		return windSpeedExists;
	}
	

	
 	/**
 	 * Get windSpeed, which is optional.
 	 * @return windSpeed as Speed
 	 * @throw IllegalAccessException If windSpeed does not exist.
 	 */
 	Speed WeatherRow::getWindSpeed() const  {
		if (!windSpeedExists) {
			throw IllegalAccessException("windSpeed", "Weather");
		}
	
  		return windSpeed;
 	}

 	/**
 	 * Set windSpeed with the specified Speed.
 	 * @param windSpeed The Speed value to which windSpeed is to be set.
 	 
 	
 	 */
 	void WeatherRow::setWindSpeed (Speed windSpeed) {
	
 		this->windSpeed = windSpeed;
	
		windSpeedExists = true;
	
 	}
	
	
	/**
	 * Mark windSpeed, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearWindSpeed () {
		windSpeedExists = false;
	}
	

	
	/**
	 * The attribute windMax is optional. Return true if this attribute exists.
	 * @return true if and only if the windMax attribute exists. 
	 */
	bool WeatherRow::isWindMaxExists() const {
		return windMaxExists;
	}
	

	
 	/**
 	 * Get windMax, which is optional.
 	 * @return windMax as Speed
 	 * @throw IllegalAccessException If windMax does not exist.
 	 */
 	Speed WeatherRow::getWindMax() const  {
		if (!windMaxExists) {
			throw IllegalAccessException("windMax", "Weather");
		}
	
  		return windMax;
 	}

 	/**
 	 * Set windMax with the specified Speed.
 	 * @param windMax The Speed value to which windMax is to be set.
 	 
 	
 	 */
 	void WeatherRow::setWindMax (Speed windMax) {
	
 		this->windMax = windMax;
	
		windMaxExists = true;
	
 	}
	
	
	/**
	 * Mark windMax, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearWindMax () {
		windMaxExists = false;
	}
	

	
	/**
	 * The attribute dewPoint is optional. Return true if this attribute exists.
	 * @return true if and only if the dewPoint attribute exists. 
	 */
	bool WeatherRow::isDewPointExists() const {
		return dewPointExists;
	}
	

	
 	/**
 	 * Get dewPoint, which is optional.
 	 * @return dewPoint as Temperature
 	 * @throw IllegalAccessException If dewPoint does not exist.
 	 */
 	Temperature WeatherRow::getDewPoint() const  {
		if (!dewPointExists) {
			throw IllegalAccessException("dewPoint", "Weather");
		}
	
  		return dewPoint;
 	}

 	/**
 	 * Set dewPoint with the specified Temperature.
 	 * @param dewPoint The Temperature value to which dewPoint is to be set.
 	 
 	
 	 */
 	void WeatherRow::setDewPoint (Temperature dewPoint) {
	
 		this->dewPoint = dewPoint;
	
		dewPointExists = true;
	
 	}
	
	
	/**
	 * Mark dewPoint, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearDewPoint () {
		dewPointExists = false;
	}
	

	
	/**
	 * The attribute numLayer is optional. Return true if this attribute exists.
	 * @return true if and only if the numLayer attribute exists. 
	 */
	bool WeatherRow::isNumLayerExists() const {
		return numLayerExists;
	}
	

	
 	/**
 	 * Get numLayer, which is optional.
 	 * @return numLayer as int
 	 * @throw IllegalAccessException If numLayer does not exist.
 	 */
 	int WeatherRow::getNumLayer() const  {
		if (!numLayerExists) {
			throw IllegalAccessException("numLayer", "Weather");
		}
	
  		return numLayer;
 	}

 	/**
 	 * Set numLayer with the specified int.
 	 * @param numLayer The int value to which numLayer is to be set.
 	 
 	
 	 */
 	void WeatherRow::setNumLayer (int numLayer) {
	
 		this->numLayer = numLayer;
	
		numLayerExists = true;
	
 	}
	
	
	/**
	 * Mark numLayer, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearNumLayer () {
		numLayerExists = false;
	}
	

	
	/**
	 * The attribute layerHeight is optional. Return true if this attribute exists.
	 * @return true if and only if the layerHeight attribute exists. 
	 */
	bool WeatherRow::isLayerHeightExists() const {
		return layerHeightExists;
	}
	

	
 	/**
 	 * Get layerHeight, which is optional.
 	 * @return layerHeight as vector<Length >
 	 * @throw IllegalAccessException If layerHeight does not exist.
 	 */
 	vector<Length > WeatherRow::getLayerHeight() const  {
		if (!layerHeightExists) {
			throw IllegalAccessException("layerHeight", "Weather");
		}
	
  		return layerHeight;
 	}

 	/**
 	 * Set layerHeight with the specified vector<Length >.
 	 * @param layerHeight The vector<Length > value to which layerHeight is to be set.
 	 
 	
 	 */
 	void WeatherRow::setLayerHeight (vector<Length > layerHeight) {
	
 		this->layerHeight = layerHeight;
	
		layerHeightExists = true;
	
 	}
	
	
	/**
	 * Mark layerHeight, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearLayerHeight () {
		layerHeightExists = false;
	}
	

	
	/**
	 * The attribute temperatureProfile is optional. Return true if this attribute exists.
	 * @return true if and only if the temperatureProfile attribute exists. 
	 */
	bool WeatherRow::isTemperatureProfileExists() const {
		return temperatureProfileExists;
	}
	

	
 	/**
 	 * Get temperatureProfile, which is optional.
 	 * @return temperatureProfile as vector<Temperature >
 	 * @throw IllegalAccessException If temperatureProfile does not exist.
 	 */
 	vector<Temperature > WeatherRow::getTemperatureProfile() const  {
		if (!temperatureProfileExists) {
			throw IllegalAccessException("temperatureProfile", "Weather");
		}
	
  		return temperatureProfile;
 	}

 	/**
 	 * Set temperatureProfile with the specified vector<Temperature >.
 	 * @param temperatureProfile The vector<Temperature > value to which temperatureProfile is to be set.
 	 
 	
 	 */
 	void WeatherRow::setTemperatureProfile (vector<Temperature > temperatureProfile) {
	
 		this->temperatureProfile = temperatureProfile;
	
		temperatureProfileExists = true;
	
 	}
	
	
	/**
	 * Mark temperatureProfile, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearTemperatureProfile () {
		temperatureProfileExists = false;
	}
	

	
	/**
	 * The attribute cloudMonitor is optional. Return true if this attribute exists.
	 * @return true if and only if the cloudMonitor attribute exists. 
	 */
	bool WeatherRow::isCloudMonitorExists() const {
		return cloudMonitorExists;
	}
	

	
 	/**
 	 * Get cloudMonitor, which is optional.
 	 * @return cloudMonitor as Temperature
 	 * @throw IllegalAccessException If cloudMonitor does not exist.
 	 */
 	Temperature WeatherRow::getCloudMonitor() const  {
		if (!cloudMonitorExists) {
			throw IllegalAccessException("cloudMonitor", "Weather");
		}
	
  		return cloudMonitor;
 	}

 	/**
 	 * Set cloudMonitor with the specified Temperature.
 	 * @param cloudMonitor The Temperature value to which cloudMonitor is to be set.
 	 
 	
 	 */
 	void WeatherRow::setCloudMonitor (Temperature cloudMonitor) {
	
 		this->cloudMonitor = cloudMonitor;
	
		cloudMonitorExists = true;
	
 	}
	
	
	/**
	 * Mark cloudMonitor, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearCloudMonitor () {
		cloudMonitorExists = false;
	}
	

	
	/**
	 * The attribute numWVR is optional. Return true if this attribute exists.
	 * @return true if and only if the numWVR attribute exists. 
	 */
	bool WeatherRow::isNumWVRExists() const {
		return numWVRExists;
	}
	

	
 	/**
 	 * Get numWVR, which is optional.
 	 * @return numWVR as int
 	 * @throw IllegalAccessException If numWVR does not exist.
 	 */
 	int WeatherRow::getNumWVR() const  {
		if (!numWVRExists) {
			throw IllegalAccessException("numWVR", "Weather");
		}
	
  		return numWVR;
 	}

 	/**
 	 * Set numWVR with the specified int.
 	 * @param numWVR The int value to which numWVR is to be set.
 	 
 	
 	 */
 	void WeatherRow::setNumWVR (int numWVR) {
	
 		this->numWVR = numWVR;
	
		numWVRExists = true;
	
 	}
	
	
	/**
	 * Mark numWVR, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearNumWVR () {
		numWVRExists = false;
	}
	

	
	/**
	 * The attribute wvrTemp is optional. Return true if this attribute exists.
	 * @return true if and only if the wvrTemp attribute exists. 
	 */
	bool WeatherRow::isWvrTempExists() const {
		return wvrTempExists;
	}
	

	
 	/**
 	 * Get wvrTemp, which is optional.
 	 * @return wvrTemp as vector<Temperature >
 	 * @throw IllegalAccessException If wvrTemp does not exist.
 	 */
 	vector<Temperature > WeatherRow::getWvrTemp() const  {
		if (!wvrTempExists) {
			throw IllegalAccessException("wvrTemp", "Weather");
		}
	
  		return wvrTemp;
 	}

 	/**
 	 * Set wvrTemp with the specified vector<Temperature >.
 	 * @param wvrTemp The vector<Temperature > value to which wvrTemp is to be set.
 	 
 	
 	 */
 	void WeatherRow::setWvrTemp (vector<Temperature > wvrTemp) {
	
 		this->wvrTemp = wvrTemp;
	
		wvrTempExists = true;
	
 	}
	
	
	/**
	 * Mark wvrTemp, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearWvrTemp () {
		wvrTempExists = false;
	}
	

	
	/**
	 * The attribute water is optional. Return true if this attribute exists.
	 * @return true if and only if the water attribute exists. 
	 */
	bool WeatherRow::isWaterExists() const {
		return waterExists;
	}
	

	
 	/**
 	 * Get water, which is optional.
 	 * @return water as double
 	 * @throw IllegalAccessException If water does not exist.
 	 */
 	double WeatherRow::getWater() const  {
		if (!waterExists) {
			throw IllegalAccessException("water", "Weather");
		}
	
  		return water;
 	}

 	/**
 	 * Set water with the specified double.
 	 * @param water The double value to which water is to be set.
 	 
 	
 	 */
 	void WeatherRow::setWater (double water) {
	
 		this->water = water;
	
		waterExists = true;
	
 	}
	
	
	/**
	 * Mark water, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearWater () {
		waterExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get stationId.
 	 * @return stationId as Tag
 	 */
 	Tag WeatherRow::getStationId() const {
	
  		return stationId;
 	}

 	/**
 	 * Set stationId with the specified Tag.
 	 * @param stationId The Tag value to which stationId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void WeatherRow::setStationId (Tag stationId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("stationId", "Weather");
		
  		}
  	
 		this->stationId = stationId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the Station table having Station.stationId == stationId
	 * @return a StationRow*
	 * 
	 
	 */
	 StationRow* WeatherRow::getStationUsingStationId() {
	 
	 	return table.getContainer().getStation().getRowByKey(stationId);
	 }
	 

	

	
	/**
	 * Create a WeatherRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	WeatherRow::WeatherRow (WeatherTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	
		pressureExists = false;
	

	
		relHumidityExists = false;
	

	
		temperatureExists = false;
	

	
		windDirectionExists = false;
	

	
		windSpeedExists = false;
	

	
		windMaxExists = false;
	

	
		dewPointExists = false;
	

	
		numLayerExists = false;
	

	
		layerHeightExists = false;
	

	
		temperatureProfileExists = false;
	

	
		cloudMonitorExists = false;
	

	
		numWVRExists = false;
	

	
		wvrTempExists = false;
	

	
		waterExists = false;
	

	
	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["stationId"] = &WeatherRow::stationIdFromBin; 
	 fromBinMethods["timeInterval"] = &WeatherRow::timeIntervalFromBin; 
		
	
	 fromBinMethods["pressure"] = &WeatherRow::pressureFromBin; 
	 fromBinMethods["relHumidity"] = &WeatherRow::relHumidityFromBin; 
	 fromBinMethods["temperature"] = &WeatherRow::temperatureFromBin; 
	 fromBinMethods["windDirection"] = &WeatherRow::windDirectionFromBin; 
	 fromBinMethods["windSpeed"] = &WeatherRow::windSpeedFromBin; 
	 fromBinMethods["windMax"] = &WeatherRow::windMaxFromBin; 
	 fromBinMethods["dewPoint"] = &WeatherRow::dewPointFromBin; 
	 fromBinMethods["numLayer"] = &WeatherRow::numLayerFromBin; 
	 fromBinMethods["layerHeight"] = &WeatherRow::layerHeightFromBin; 
	 fromBinMethods["temperatureProfile"] = &WeatherRow::temperatureProfileFromBin; 
	 fromBinMethods["cloudMonitor"] = &WeatherRow::cloudMonitorFromBin; 
	 fromBinMethods["numWVR"] = &WeatherRow::numWVRFromBin; 
	 fromBinMethods["wvrTemp"] = &WeatherRow::wvrTempFromBin; 
	 fromBinMethods["water"] = &WeatherRow::waterFromBin; 
	
	}
	
	WeatherRow::WeatherRow (WeatherTable &t, WeatherRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	
		pressureExists = false;
	

	
		relHumidityExists = false;
	

	
		temperatureExists = false;
	

	
		windDirectionExists = false;
	

	
		windSpeedExists = false;
	

	
		windMaxExists = false;
	

	
		dewPointExists = false;
	

	
		numLayerExists = false;
	

	
		layerHeightExists = false;
	

	
		temperatureProfileExists = false;
	

	
		cloudMonitorExists = false;
	

	
		numWVRExists = false;
	

	
		wvrTempExists = false;
	

	
		waterExists = false;
	

	
	
		
		}
		else {
	
		
			stationId = row.stationId;
		
			timeInterval = row.timeInterval;
		
		
		
		
		
		
		
		if (row.pressureExists) {
			pressure = row.pressure;		
			pressureExists = true;
		}
		else
			pressureExists = false;
		
		if (row.relHumidityExists) {
			relHumidity = row.relHumidity;		
			relHumidityExists = true;
		}
		else
			relHumidityExists = false;
		
		if (row.temperatureExists) {
			temperature = row.temperature;		
			temperatureExists = true;
		}
		else
			temperatureExists = false;
		
		if (row.windDirectionExists) {
			windDirection = row.windDirection;		
			windDirectionExists = true;
		}
		else
			windDirectionExists = false;
		
		if (row.windSpeedExists) {
			windSpeed = row.windSpeed;		
			windSpeedExists = true;
		}
		else
			windSpeedExists = false;
		
		if (row.windMaxExists) {
			windMax = row.windMax;		
			windMaxExists = true;
		}
		else
			windMaxExists = false;
		
		if (row.dewPointExists) {
			dewPoint = row.dewPoint;		
			dewPointExists = true;
		}
		else
			dewPointExists = false;
		
		if (row.numLayerExists) {
			numLayer = row.numLayer;		
			numLayerExists = true;
		}
		else
			numLayerExists = false;
		
		if (row.layerHeightExists) {
			layerHeight = row.layerHeight;		
			layerHeightExists = true;
		}
		else
			layerHeightExists = false;
		
		if (row.temperatureProfileExists) {
			temperatureProfile = row.temperatureProfile;		
			temperatureProfileExists = true;
		}
		else
			temperatureProfileExists = false;
		
		if (row.cloudMonitorExists) {
			cloudMonitor = row.cloudMonitor;		
			cloudMonitorExists = true;
		}
		else
			cloudMonitorExists = false;
		
		if (row.numWVRExists) {
			numWVR = row.numWVR;		
			numWVRExists = true;
		}
		else
			numWVRExists = false;
		
		if (row.wvrTempExists) {
			wvrTemp = row.wvrTemp;		
			wvrTempExists = true;
		}
		else
			wvrTempExists = false;
		
		if (row.waterExists) {
			water = row.water;		
			waterExists = true;
		}
		else
			waterExists = false;
		
		}
		
		 fromBinMethods["stationId"] = &WeatherRow::stationIdFromBin; 
		 fromBinMethods["timeInterval"] = &WeatherRow::timeIntervalFromBin; 
			
	
		 fromBinMethods["pressure"] = &WeatherRow::pressureFromBin; 
		 fromBinMethods["relHumidity"] = &WeatherRow::relHumidityFromBin; 
		 fromBinMethods["temperature"] = &WeatherRow::temperatureFromBin; 
		 fromBinMethods["windDirection"] = &WeatherRow::windDirectionFromBin; 
		 fromBinMethods["windSpeed"] = &WeatherRow::windSpeedFromBin; 
		 fromBinMethods["windMax"] = &WeatherRow::windMaxFromBin; 
		 fromBinMethods["dewPoint"] = &WeatherRow::dewPointFromBin; 
		 fromBinMethods["numLayer"] = &WeatherRow::numLayerFromBin; 
		 fromBinMethods["layerHeight"] = &WeatherRow::layerHeightFromBin; 
		 fromBinMethods["temperatureProfile"] = &WeatherRow::temperatureProfileFromBin; 
		 fromBinMethods["cloudMonitor"] = &WeatherRow::cloudMonitorFromBin; 
		 fromBinMethods["numWVR"] = &WeatherRow::numWVRFromBin; 
		 fromBinMethods["wvrTemp"] = &WeatherRow::wvrTempFromBin; 
		 fromBinMethods["water"] = &WeatherRow::waterFromBin; 
			
	}

	
	bool WeatherRow::compareNoAutoInc(Tag stationId, ArrayTimeInterval timeInterval) {
		bool result;
		result = true;
		
	
		
		result = result && (this->stationId == stationId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the WeatherRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool WeatherRow::equalByRequiredValue(WeatherRow* x) {
		
		return true;
	}	
	
/*
	 map<string, WeatherAttributeFromBin> WeatherRow::initFromBinMethods() {
		map<string, WeatherAttributeFromBin> result;
		
		result["stationId"] = &WeatherRow::stationIdFromBin;
		result["timeInterval"] = &WeatherRow::timeIntervalFromBin;
		
		
		result["pressure"] = &WeatherRow::pressureFromBin;
		result["relHumidity"] = &WeatherRow::relHumidityFromBin;
		result["temperature"] = &WeatherRow::temperatureFromBin;
		result["windDirection"] = &WeatherRow::windDirectionFromBin;
		result["windSpeed"] = &WeatherRow::windSpeedFromBin;
		result["windMax"] = &WeatherRow::windMaxFromBin;
		result["dewPoint"] = &WeatherRow::dewPointFromBin;
		result["numLayer"] = &WeatherRow::numLayerFromBin;
		result["layerHeight"] = &WeatherRow::layerHeightFromBin;
		result["temperatureProfile"] = &WeatherRow::temperatureProfileFromBin;
		result["cloudMonitor"] = &WeatherRow::cloudMonitorFromBin;
		result["numWVR"] = &WeatherRow::numWVRFromBin;
		result["wvrTemp"] = &WeatherRow::wvrTempFromBin;
		result["water"] = &WeatherRow::waterFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
