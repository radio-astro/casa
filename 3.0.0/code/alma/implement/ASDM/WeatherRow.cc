
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
	
	void WeatherRow::isAdded() {
		hasBeenAdded = true;
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
			
		
	

	
  		
		
		
			
		x->pressure = pressure.toIDLPressure();
			
		
	

	
  		
		
		
			
				
		x->pressureFlag = pressureFlag;
 				
 			
		
	

	
  		
		
		
			
		x->relHumidity = relHumidity.toIDLHumidity();
			
		
	

	
  		
		
		
			
				
		x->relHumidityFlag = relHumidityFlag;
 				
 			
		
	

	
  		
		
		
			
		x->temperature = temperature.toIDLTemperature();
			
		
	

	
  		
		
		
			
				
		x->temperatureFlag = temperatureFlag;
 				
 			
		
	

	
  		
		
		
			
		x->windDirection = windDirection.toIDLAngle();
			
		
	

	
  		
		
		
			
				
		x->windDirectionFlag = windDirectionFlag;
 				
 			
		
	

	
  		
		
		
			
		x->windSpeed = windSpeed.toIDLSpeed();
			
		
	

	
  		
		
		
			
				
		x->windSpeedFlag = windSpeedFlag;
 				
 			
		
	

	
  		
		
		
			
		x->windMax = windMax.toIDLSpeed();
			
		
	

	
  		
		
		
			
				
		x->windMaxFlag = windMaxFlag;
 				
 			
		
	

	
  		
		
		x->dewPointExists = dewPointExists;
		
		
			
		x->dewPoint = dewPoint.toIDLTemperature();
			
		
	

	
  		
		
		x->dewPointFlagExists = dewPointFlagExists;
		
		
			
				
		x->dewPointFlag = dewPointFlag;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
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
			
 		
		
	

	
		
		
			
		setPressure(Pressure (x.pressure));
			
 		
		
	

	
		
		
			
		setPressureFlag(x.pressureFlag);
  			
 		
		
	

	
		
		
			
		setRelHumidity(Humidity (x.relHumidity));
			
 		
		
	

	
		
		
			
		setRelHumidityFlag(x.relHumidityFlag);
  			
 		
		
	

	
		
		
			
		setTemperature(Temperature (x.temperature));
			
 		
		
	

	
		
		
			
		setTemperatureFlag(x.temperatureFlag);
  			
 		
		
	

	
		
		
			
		setWindDirection(Angle (x.windDirection));
			
 		
		
	

	
		
		
			
		setWindDirectionFlag(x.windDirectionFlag);
  			
 		
		
	

	
		
		
			
		setWindSpeed(Speed (x.windSpeed));
			
 		
		
	

	
		
		
			
		setWindSpeedFlag(x.windSpeedFlag);
  			
 		
		
	

	
		
		
			
		setWindMax(Speed (x.windMax));
			
 		
		
	

	
		
		
			
		setWindMaxFlag(x.windMaxFlag);
  			
 		
		
	

	
		
		dewPointExists = x.dewPointExists;
		if (x.dewPointExists) {
		
		
			
		setDewPoint(Temperature (x.dewPoint));
			
 		
		
		}
		
	

	
		
		dewPointFlagExists = x.dewPointFlagExists;
		if (x.dewPointFlagExists) {
		
		
			
		setDewPointFlag(x.dewPointFlag);
  			
 		
		
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
		
		
	

  	
 		
		
		Parser::toXML(pressure, "pressure", buf);
		
		
	

  	
 		
		
		Parser::toXML(pressureFlag, "pressureFlag", buf);
		
		
	

  	
 		
		
		Parser::toXML(relHumidity, "relHumidity", buf);
		
		
	

  	
 		
		
		Parser::toXML(relHumidityFlag, "relHumidityFlag", buf);
		
		
	

  	
 		
		
		Parser::toXML(temperature, "temperature", buf);
		
		
	

  	
 		
		
		Parser::toXML(temperatureFlag, "temperatureFlag", buf);
		
		
	

  	
 		
		
		Parser::toXML(windDirection, "windDirection", buf);
		
		
	

  	
 		
		
		Parser::toXML(windDirectionFlag, "windDirectionFlag", buf);
		
		
	

  	
 		
		
		Parser::toXML(windSpeed, "windSpeed", buf);
		
		
	

  	
 		
		
		Parser::toXML(windSpeedFlag, "windSpeedFlag", buf);
		
		
	

  	
 		
		
		Parser::toXML(windMax, "windMax", buf);
		
		
	

  	
 		
		
		Parser::toXML(windMaxFlag, "windMaxFlag", buf);
		
		
	

  	
 		
		if (dewPointExists) {
		
		
		Parser::toXML(dewPoint, "dewPoint", buf);
		
		
		}
		
	

  	
 		
		if (dewPointFlagExists) {
		
		
		Parser::toXML(dewPointFlag, "dewPointFlag", buf);
		
		
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
			
		
	

	
  		
			
	  	setPressure(Parser::getPressure("pressure","Weather",rowDoc));
			
		
	

	
  		
			
	  	setPressureFlag(Parser::getBoolean("pressureFlag","Weather",rowDoc));
			
		
	

	
  		
			
	  	setRelHumidity(Parser::getHumidity("relHumidity","Weather",rowDoc));
			
		
	

	
  		
			
	  	setRelHumidityFlag(Parser::getBoolean("relHumidityFlag","Weather",rowDoc));
			
		
	

	
  		
			
	  	setTemperature(Parser::getTemperature("temperature","Weather",rowDoc));
			
		
	

	
  		
			
	  	setTemperatureFlag(Parser::getBoolean("temperatureFlag","Weather",rowDoc));
			
		
	

	
  		
			
	  	setWindDirection(Parser::getAngle("windDirection","Weather",rowDoc));
			
		
	

	
  		
			
	  	setWindDirectionFlag(Parser::getBoolean("windDirectionFlag","Weather",rowDoc));
			
		
	

	
  		
			
	  	setWindSpeed(Parser::getSpeed("windSpeed","Weather",rowDoc));
			
		
	

	
  		
			
	  	setWindSpeedFlag(Parser::getBoolean("windSpeedFlag","Weather",rowDoc));
			
		
	

	
  		
			
	  	setWindMax(Parser::getSpeed("windMax","Weather",rowDoc));
			
		
	

	
  		
			
	  	setWindMaxFlag(Parser::getBoolean("windMaxFlag","Weather",rowDoc));
			
		
	

	
  		
        if (row.isStr("<dewPoint>")) {
			
	  		setDewPoint(Parser::getTemperature("dewPoint","Weather",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<dewPointFlag>")) {
			
	  		setDewPointFlag(Parser::getBoolean("dewPointFlag","Weather",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setStationId(Parser::getTag("stationId","Station",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Weather");
		}
	}
	
	void WeatherRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	stationId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
	pressure.toBin(eoss);
		
	

	
	
		
						
			eoss.writeBoolean(pressureFlag);
				
		
	

	
	
		
	relHumidity.toBin(eoss);
		
	

	
	
		
						
			eoss.writeBoolean(relHumidityFlag);
				
		
	

	
	
		
	temperature.toBin(eoss);
		
	

	
	
		
						
			eoss.writeBoolean(temperatureFlag);
				
		
	

	
	
		
	windDirection.toBin(eoss);
		
	

	
	
		
						
			eoss.writeBoolean(windDirectionFlag);
				
		
	

	
	
		
	windSpeed.toBin(eoss);
		
	

	
	
		
						
			eoss.writeBoolean(windSpeedFlag);
				
		
	

	
	
		
	windMax.toBin(eoss);
		
	

	
	
		
						
			eoss.writeBoolean(windMaxFlag);
				
		
	


	
	
	eoss.writeBoolean(dewPointExists);
	if (dewPointExists) {
	
	
	
		
	dewPoint.toBin(eoss);
		
	

	}

	eoss.writeBoolean(dewPointFlagExists);
	if (dewPointFlagExists) {
	
	
	
		
						
			eoss.writeBoolean(dewPointFlag);
				
		
	

	}

	}
	
	WeatherRow* WeatherRow::fromBin(EndianISStream& eiss, WeatherTable& table) {
		WeatherRow* row = new  WeatherRow(table);
		
		
		
	
		
		
		row->stationId =  Tag::fromBin(eiss);
		
	

	
		
		
		row->timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	

	
		
		
		row->pressure =  Pressure::fromBin(eiss);
		
	

	
	
		
			
		row->pressureFlag =  eiss.readBoolean();
			
		
	

	
		
		
		row->relHumidity =  Humidity::fromBin(eiss);
		
	

	
	
		
			
		row->relHumidityFlag =  eiss.readBoolean();
			
		
	

	
		
		
		row->temperature =  Temperature::fromBin(eiss);
		
	

	
	
		
			
		row->temperatureFlag =  eiss.readBoolean();
			
		
	

	
		
		
		row->windDirection =  Angle::fromBin(eiss);
		
	

	
	
		
			
		row->windDirectionFlag =  eiss.readBoolean();
			
		
	

	
		
		
		row->windSpeed =  Speed::fromBin(eiss);
		
	

	
	
		
			
		row->windSpeedFlag =  eiss.readBoolean();
			
		
	

	
		
		
		row->windMax =  Speed::fromBin(eiss);
		
	

	
	
		
			
		row->windMaxFlag =  eiss.readBoolean();
			
		
	

		
		
		
	row->dewPointExists = eiss.readBoolean();
	if (row->dewPointExists) {
		
	
		
		
		row->dewPoint =  Temperature::fromBin(eiss);
		
	

	}

	row->dewPointFlagExists = eiss.readBoolean();
	if (row->dewPointFlagExists) {
		
	
	
		
			
		row->dewPointFlag =  eiss.readBoolean();
			
		
	

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
 	 * Get pressure.
 	 * @return pressure as Pressure
 	 */
 	Pressure WeatherRow::getPressure() const {
	
  		return pressure;
 	}

 	/**
 	 * Set pressure with the specified Pressure.
 	 * @param pressure The Pressure value to which pressure is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setPressure (Pressure pressure)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pressure = pressure;
	
 	}
	
	

	

	
 	/**
 	 * Get pressureFlag.
 	 * @return pressureFlag as bool
 	 */
 	bool WeatherRow::getPressureFlag() const {
	
  		return pressureFlag;
 	}

 	/**
 	 * Set pressureFlag with the specified bool.
 	 * @param pressureFlag The bool value to which pressureFlag is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setPressureFlag (bool pressureFlag)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pressureFlag = pressureFlag;
	
 	}
	
	

	

	
 	/**
 	 * Get relHumidity.
 	 * @return relHumidity as Humidity
 	 */
 	Humidity WeatherRow::getRelHumidity() const {
	
  		return relHumidity;
 	}

 	/**
 	 * Set relHumidity with the specified Humidity.
 	 * @param relHumidity The Humidity value to which relHumidity is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setRelHumidity (Humidity relHumidity)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->relHumidity = relHumidity;
	
 	}
	
	

	

	
 	/**
 	 * Get relHumidityFlag.
 	 * @return relHumidityFlag as bool
 	 */
 	bool WeatherRow::getRelHumidityFlag() const {
	
  		return relHumidityFlag;
 	}

 	/**
 	 * Set relHumidityFlag with the specified bool.
 	 * @param relHumidityFlag The bool value to which relHumidityFlag is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setRelHumidityFlag (bool relHumidityFlag)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->relHumidityFlag = relHumidityFlag;
	
 	}
	
	

	

	
 	/**
 	 * Get temperature.
 	 * @return temperature as Temperature
 	 */
 	Temperature WeatherRow::getTemperature() const {
	
  		return temperature;
 	}

 	/**
 	 * Set temperature with the specified Temperature.
 	 * @param temperature The Temperature value to which temperature is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setTemperature (Temperature temperature)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->temperature = temperature;
	
 	}
	
	

	

	
 	/**
 	 * Get temperatureFlag.
 	 * @return temperatureFlag as bool
 	 */
 	bool WeatherRow::getTemperatureFlag() const {
	
  		return temperatureFlag;
 	}

 	/**
 	 * Set temperatureFlag with the specified bool.
 	 * @param temperatureFlag The bool value to which temperatureFlag is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setTemperatureFlag (bool temperatureFlag)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->temperatureFlag = temperatureFlag;
	
 	}
	
	

	

	
 	/**
 	 * Get windDirection.
 	 * @return windDirection as Angle
 	 */
 	Angle WeatherRow::getWindDirection() const {
	
  		return windDirection;
 	}

 	/**
 	 * Set windDirection with the specified Angle.
 	 * @param windDirection The Angle value to which windDirection is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setWindDirection (Angle windDirection)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->windDirection = windDirection;
	
 	}
	
	

	

	
 	/**
 	 * Get windDirectionFlag.
 	 * @return windDirectionFlag as bool
 	 */
 	bool WeatherRow::getWindDirectionFlag() const {
	
  		return windDirectionFlag;
 	}

 	/**
 	 * Set windDirectionFlag with the specified bool.
 	 * @param windDirectionFlag The bool value to which windDirectionFlag is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setWindDirectionFlag (bool windDirectionFlag)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->windDirectionFlag = windDirectionFlag;
	
 	}
	
	

	

	
 	/**
 	 * Get windSpeed.
 	 * @return windSpeed as Speed
 	 */
 	Speed WeatherRow::getWindSpeed() const {
	
  		return windSpeed;
 	}

 	/**
 	 * Set windSpeed with the specified Speed.
 	 * @param windSpeed The Speed value to which windSpeed is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setWindSpeed (Speed windSpeed)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->windSpeed = windSpeed;
	
 	}
	
	

	

	
 	/**
 	 * Get windSpeedFlag.
 	 * @return windSpeedFlag as bool
 	 */
 	bool WeatherRow::getWindSpeedFlag() const {
	
  		return windSpeedFlag;
 	}

 	/**
 	 * Set windSpeedFlag with the specified bool.
 	 * @param windSpeedFlag The bool value to which windSpeedFlag is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setWindSpeedFlag (bool windSpeedFlag)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->windSpeedFlag = windSpeedFlag;
	
 	}
	
	

	

	
 	/**
 	 * Get windMax.
 	 * @return windMax as Speed
 	 */
 	Speed WeatherRow::getWindMax() const {
	
  		return windMax;
 	}

 	/**
 	 * Set windMax with the specified Speed.
 	 * @param windMax The Speed value to which windMax is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setWindMax (Speed windMax)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->windMax = windMax;
	
 	}
	
	

	

	
 	/**
 	 * Get windMaxFlag.
 	 * @return windMaxFlag as bool
 	 */
 	bool WeatherRow::getWindMaxFlag() const {
	
  		return windMaxFlag;
 	}

 	/**
 	 * Set windMaxFlag with the specified bool.
 	 * @param windMaxFlag The bool value to which windMaxFlag is to be set.
 	 
 	
 		
 	 */
 	void WeatherRow::setWindMaxFlag (bool windMaxFlag)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->windMaxFlag = windMaxFlag;
	
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
	 * The attribute dewPointFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the dewPointFlag attribute exists. 
	 */
	bool WeatherRow::isDewPointFlagExists() const {
		return dewPointFlagExists;
	}
	

	
 	/**
 	 * Get dewPointFlag, which is optional.
 	 * @return dewPointFlag as bool
 	 * @throw IllegalAccessException If dewPointFlag does not exist.
 	 */
 	bool WeatherRow::getDewPointFlag() const  {
		if (!dewPointFlagExists) {
			throw IllegalAccessException("dewPointFlag", "Weather");
		}
	
  		return dewPointFlag;
 	}

 	/**
 	 * Set dewPointFlag with the specified bool.
 	 * @param dewPointFlag The bool value to which dewPointFlag is to be set.
 	 
 	
 	 */
 	void WeatherRow::setDewPointFlag (bool dewPointFlag) {
	
 		this->dewPointFlag = dewPointFlag;
	
		dewPointFlagExists = true;
	
 	}
	
	
	/**
	 * Mark dewPointFlag, which is an optional field, as non-existent.
	 */
	void WeatherRow::clearDewPointFlag () {
		dewPointFlagExists = false;
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
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	
		dewPointExists = false;
	

	
		dewPointFlagExists = false;
	

	
	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	WeatherRow::WeatherRow (WeatherTable &t, WeatherRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	
		dewPointExists = false;
	

	
		dewPointFlagExists = false;
	

	
	
		
		}
		else {
	
		
			stationId = row.stationId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			pressure = row.pressure;
		
			pressureFlag = row.pressureFlag;
		
			relHumidity = row.relHumidity;
		
			relHumidityFlag = row.relHumidityFlag;
		
			temperature = row.temperature;
		
			temperatureFlag = row.temperatureFlag;
		
			windDirection = row.windDirection;
		
			windDirectionFlag = row.windDirectionFlag;
		
			windSpeed = row.windSpeed;
		
			windSpeedFlag = row.windSpeedFlag;
		
			windMax = row.windMax;
		
			windMaxFlag = row.windMaxFlag;
		
		
		
		
		if (row.dewPointExists) {
			dewPoint = row.dewPoint;		
			dewPointExists = true;
		}
		else
			dewPointExists = false;
		
		if (row.dewPointFlagExists) {
			dewPointFlag = row.dewPointFlag;		
			dewPointFlagExists = true;
		}
		else
			dewPointFlagExists = false;
		
		}	
	}

	
	bool WeatherRow::compareNoAutoInc(Tag stationId, ArrayTimeInterval timeInterval, Pressure pressure, bool pressureFlag, Humidity relHumidity, bool relHumidityFlag, Temperature temperature, bool temperatureFlag, Angle windDirection, bool windDirectionFlag, Speed windSpeed, bool windSpeedFlag, Speed windMax, bool windMaxFlag) {
		bool result;
		result = true;
		
	
		
		result = result && (this->stationId == stationId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->pressure == pressure);
		
		if (!result) return false;
	

	
		
		result = result && (this->pressureFlag == pressureFlag);
		
		if (!result) return false;
	

	
		
		result = result && (this->relHumidity == relHumidity);
		
		if (!result) return false;
	

	
		
		result = result && (this->relHumidityFlag == relHumidityFlag);
		
		if (!result) return false;
	

	
		
		result = result && (this->temperature == temperature);
		
		if (!result) return false;
	

	
		
		result = result && (this->temperatureFlag == temperatureFlag);
		
		if (!result) return false;
	

	
		
		result = result && (this->windDirection == windDirection);
		
		if (!result) return false;
	

	
		
		result = result && (this->windDirectionFlag == windDirectionFlag);
		
		if (!result) return false;
	

	
		
		result = result && (this->windSpeed == windSpeed);
		
		if (!result) return false;
	

	
		
		result = result && (this->windSpeedFlag == windSpeedFlag);
		
		if (!result) return false;
	

	
		
		result = result && (this->windMax == windMax);
		
		if (!result) return false;
	

	
		
		result = result && (this->windMaxFlag == windMaxFlag);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool WeatherRow::compareRequiredValue(Pressure pressure, bool pressureFlag, Humidity relHumidity, bool relHumidityFlag, Temperature temperature, bool temperatureFlag, Angle windDirection, bool windDirectionFlag, Speed windSpeed, bool windSpeedFlag, Speed windMax, bool windMaxFlag) {
		bool result;
		result = true;
		
	
		if (!(this->pressure == pressure)) return false;
	

	
		if (!(this->pressureFlag == pressureFlag)) return false;
	

	
		if (!(this->relHumidity == relHumidity)) return false;
	

	
		if (!(this->relHumidityFlag == relHumidityFlag)) return false;
	

	
		if (!(this->temperature == temperature)) return false;
	

	
		if (!(this->temperatureFlag == temperatureFlag)) return false;
	

	
		if (!(this->windDirection == windDirection)) return false;
	

	
		if (!(this->windDirectionFlag == windDirectionFlag)) return false;
	

	
		if (!(this->windSpeed == windSpeed)) return false;
	

	
		if (!(this->windSpeedFlag == windSpeedFlag)) return false;
	

	
		if (!(this->windMax == windMax)) return false;
	

	
		if (!(this->windMaxFlag == windMaxFlag)) return false;
	

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
		
			
		if (this->pressure != x->pressure) return false;
			
		if (this->pressureFlag != x->pressureFlag) return false;
			
		if (this->relHumidity != x->relHumidity) return false;
			
		if (this->relHumidityFlag != x->relHumidityFlag) return false;
			
		if (this->temperature != x->temperature) return false;
			
		if (this->temperatureFlag != x->temperatureFlag) return false;
			
		if (this->windDirection != x->windDirection) return false;
			
		if (this->windDirectionFlag != x->windDirectionFlag) return false;
			
		if (this->windSpeed != x->windSpeed) return false;
			
		if (this->windSpeedFlag != x->windSpeedFlag) return false;
			
		if (this->windMax != x->windMax) return false;
			
		if (this->windMaxFlag != x->windMaxFlag) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
