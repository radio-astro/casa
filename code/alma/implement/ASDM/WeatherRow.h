
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
 * File WeatherRow.h
 */
 
#ifndef WeatherRow_CLASS
#define WeatherRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::WeatherRowIDL;
#endif






#include <Angle.h>
using  asdm::Angle;

#include <Speed.h>
using  asdm::Speed;

#include <Tag.h>
using  asdm::Tag;

#include <Temperature.h>
using  asdm::Temperature;

#include <Humidity.h>
using  asdm::Humidity;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;

#include <Pressure.h>
using  asdm::Pressure;




	

	

	

	

	

	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file Weather.h
    \brief Generated from model's revision "1.55", branch "HEAD"
*/

namespace asdm {

//class asdm::WeatherTable;


// class asdm::StationRow;
class StationRow;
	

class WeatherRow;
typedef void (WeatherRow::*WeatherAttributeFromBin) (EndianISStream& eiss);

/**
 * The WeatherRow class is a row of a WeatherTable.
 * 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 */
class WeatherRow {
friend class asdm::WeatherTable;

public:

	virtual ~WeatherRow();

	/**
	 * Return the table to which this row belongs.
	 */
	WeatherTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval getTimeInterval() const;
	
 
 	
 	
 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setTimeInterval (ArrayTimeInterval timeInterval);
  		
	
	
	


	
	// ===> Attribute pressure
	
	
	

	
 	/**
 	 * Get pressure.
 	 * @return pressure as Pressure
 	 */
 	Pressure getPressure() const;
	
 
 	
 	
 	/**
 	 * Set pressure with the specified Pressure.
 	 * @param pressure The Pressure value to which pressure is to be set.
 	 
 		
 			
 	 */
 	void setPressure (Pressure pressure);
  		
	
	
	


	
	// ===> Attribute pressureFlag
	
	
	

	
 	/**
 	 * Get pressureFlag.
 	 * @return pressureFlag as bool
 	 */
 	bool getPressureFlag() const;
	
 
 	
 	
 	/**
 	 * Set pressureFlag with the specified bool.
 	 * @param pressureFlag The bool value to which pressureFlag is to be set.
 	 
 		
 			
 	 */
 	void setPressureFlag (bool pressureFlag);
  		
	
	
	


	
	// ===> Attribute relHumidity
	
	
	

	
 	/**
 	 * Get relHumidity.
 	 * @return relHumidity as Humidity
 	 */
 	Humidity getRelHumidity() const;
	
 
 	
 	
 	/**
 	 * Set relHumidity with the specified Humidity.
 	 * @param relHumidity The Humidity value to which relHumidity is to be set.
 	 
 		
 			
 	 */
 	void setRelHumidity (Humidity relHumidity);
  		
	
	
	


	
	// ===> Attribute relHumidityFlag
	
	
	

	
 	/**
 	 * Get relHumidityFlag.
 	 * @return relHumidityFlag as bool
 	 */
 	bool getRelHumidityFlag() const;
	
 
 	
 	
 	/**
 	 * Set relHumidityFlag with the specified bool.
 	 * @param relHumidityFlag The bool value to which relHumidityFlag is to be set.
 	 
 		
 			
 	 */
 	void setRelHumidityFlag (bool relHumidityFlag);
  		
	
	
	


	
	// ===> Attribute temperature
	
	
	

	
 	/**
 	 * Get temperature.
 	 * @return temperature as Temperature
 	 */
 	Temperature getTemperature() const;
	
 
 	
 	
 	/**
 	 * Set temperature with the specified Temperature.
 	 * @param temperature The Temperature value to which temperature is to be set.
 	 
 		
 			
 	 */
 	void setTemperature (Temperature temperature);
  		
	
	
	


	
	// ===> Attribute temperatureFlag
	
	
	

	
 	/**
 	 * Get temperatureFlag.
 	 * @return temperatureFlag as bool
 	 */
 	bool getTemperatureFlag() const;
	
 
 	
 	
 	/**
 	 * Set temperatureFlag with the specified bool.
 	 * @param temperatureFlag The bool value to which temperatureFlag is to be set.
 	 
 		
 			
 	 */
 	void setTemperatureFlag (bool temperatureFlag);
  		
	
	
	


	
	// ===> Attribute windDirection
	
	
	

	
 	/**
 	 * Get windDirection.
 	 * @return windDirection as Angle
 	 */
 	Angle getWindDirection() const;
	
 
 	
 	
 	/**
 	 * Set windDirection with the specified Angle.
 	 * @param windDirection The Angle value to which windDirection is to be set.
 	 
 		
 			
 	 */
 	void setWindDirection (Angle windDirection);
  		
	
	
	


	
	// ===> Attribute windDirectionFlag
	
	
	

	
 	/**
 	 * Get windDirectionFlag.
 	 * @return windDirectionFlag as bool
 	 */
 	bool getWindDirectionFlag() const;
	
 
 	
 	
 	/**
 	 * Set windDirectionFlag with the specified bool.
 	 * @param windDirectionFlag The bool value to which windDirectionFlag is to be set.
 	 
 		
 			
 	 */
 	void setWindDirectionFlag (bool windDirectionFlag);
  		
	
	
	


	
	// ===> Attribute windSpeed
	
	
	

	
 	/**
 	 * Get windSpeed.
 	 * @return windSpeed as Speed
 	 */
 	Speed getWindSpeed() const;
	
 
 	
 	
 	/**
 	 * Set windSpeed with the specified Speed.
 	 * @param windSpeed The Speed value to which windSpeed is to be set.
 	 
 		
 			
 	 */
 	void setWindSpeed (Speed windSpeed);
  		
	
	
	


	
	// ===> Attribute windSpeedFlag
	
	
	

	
 	/**
 	 * Get windSpeedFlag.
 	 * @return windSpeedFlag as bool
 	 */
 	bool getWindSpeedFlag() const;
	
 
 	
 	
 	/**
 	 * Set windSpeedFlag with the specified bool.
 	 * @param windSpeedFlag The bool value to which windSpeedFlag is to be set.
 	 
 		
 			
 	 */
 	void setWindSpeedFlag (bool windSpeedFlag);
  		
	
	
	


	
	// ===> Attribute windMax
	
	
	

	
 	/**
 	 * Get windMax.
 	 * @return windMax as Speed
 	 */
 	Speed getWindMax() const;
	
 
 	
 	
 	/**
 	 * Set windMax with the specified Speed.
 	 * @param windMax The Speed value to which windMax is to be set.
 	 
 		
 			
 	 */
 	void setWindMax (Speed windMax);
  		
	
	
	


	
	// ===> Attribute windMaxFlag
	
	
	

	
 	/**
 	 * Get windMaxFlag.
 	 * @return windMaxFlag as bool
 	 */
 	bool getWindMaxFlag() const;
	
 
 	
 	
 	/**
 	 * Set windMaxFlag with the specified bool.
 	 * @param windMaxFlag The bool value to which windMaxFlag is to be set.
 	 
 		
 			
 	 */
 	void setWindMaxFlag (bool windMaxFlag);
  		
	
	
	


	
	// ===> Attribute dewPoint, which is optional
	
	
	
	/**
	 * The attribute dewPoint is optional. Return true if this attribute exists.
	 * @return true if and only if the dewPoint attribute exists. 
	 */
	bool isDewPointExists() const;
	

	
 	/**
 	 * Get dewPoint, which is optional.
 	 * @return dewPoint as Temperature
 	 * @throws IllegalAccessException If dewPoint does not exist.
 	 */
 	Temperature getDewPoint() const;
	
 
 	
 	
 	/**
 	 * Set dewPoint with the specified Temperature.
 	 * @param dewPoint The Temperature value to which dewPoint is to be set.
 	 
 		
 	 */
 	void setDewPoint (Temperature dewPoint);
		
	
	
	
	/**
	 * Mark dewPoint, which is an optional field, as non-existent.
	 */
	void clearDewPoint ();
	


	
	// ===> Attribute dewPointFlag, which is optional
	
	
	
	/**
	 * The attribute dewPointFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the dewPointFlag attribute exists. 
	 */
	bool isDewPointFlagExists() const;
	

	
 	/**
 	 * Get dewPointFlag, which is optional.
 	 * @return dewPointFlag as bool
 	 * @throws IllegalAccessException If dewPointFlag does not exist.
 	 */
 	bool getDewPointFlag() const;
	
 
 	
 	
 	/**
 	 * Set dewPointFlag with the specified bool.
 	 * @param dewPointFlag The bool value to which dewPointFlag is to be set.
 	 
 		
 	 */
 	void setDewPointFlag (bool dewPointFlag);
		
	
	
	
	/**
	 * Mark dewPointFlag, which is an optional field, as non-existent.
	 */
	void clearDewPointFlag ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute stationId
	
	
	

	
 	/**
 	 * Get stationId.
 	 * @return stationId as Tag
 	 */
 	Tag getStationId() const;
	
 
 	
 	
 	/**
 	 * Set stationId with the specified Tag.
 	 * @param stationId The Tag value to which stationId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setStationId (Tag stationId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * stationId pointer to the row in the Station table having Station.stationId == stationId
	 * @return a StationRow*
	 * 
	 
	 */
	 StationRow* getStationUsingStationId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this WeatherRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param stationId
	    
	 * @param timeInterval
	    
	 * @param pressure
	    
	 * @param pressureFlag
	    
	 * @param relHumidity
	    
	 * @param relHumidityFlag
	    
	 * @param temperature
	    
	 * @param temperatureFlag
	    
	 * @param windDirection
	    
	 * @param windDirectionFlag
	    
	 * @param windSpeed
	    
	 * @param windSpeedFlag
	    
	 * @param windMax
	    
	 * @param windMaxFlag
	    
	 */ 
	bool compareNoAutoInc(Tag stationId, ArrayTimeInterval timeInterval, Pressure pressure, bool pressureFlag, Humidity relHumidity, bool relHumidityFlag, Temperature temperature, bool temperatureFlag, Angle windDirection, bool windDirectionFlag, Speed windSpeed, bool windSpeedFlag, Speed windMax, bool windMaxFlag);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param pressure
	    
	 * @param pressureFlag
	    
	 * @param relHumidity
	    
	 * @param relHumidityFlag
	    
	 * @param temperature
	    
	 * @param temperatureFlag
	    
	 * @param windDirection
	    
	 * @param windDirectionFlag
	    
	 * @param windSpeed
	    
	 * @param windSpeedFlag
	    
	 * @param windMax
	    
	 * @param windMaxFlag
	    
	 */ 
	bool compareRequiredValue(Pressure pressure, bool pressureFlag, Humidity relHumidity, bool relHumidityFlag, Temperature temperature, bool temperatureFlag, Angle windDirection, bool windDirectionFlag, Speed windSpeed, bool windSpeedFlag, Speed windMax, bool windMaxFlag); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the WeatherRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(WeatherRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a WeatherRowIDL struct.
	 */
	WeatherRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct WeatherRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (WeatherRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param rowDoc the XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (string rowDoc) ;	

private:
	/**
	 * The table to which this row belongs.
	 */
	WeatherTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a WeatherRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	WeatherRow (WeatherTable &table);

	/**
	 * Create a WeatherRow using a copy constructor mechanism.
	 * <p>
	 * Given a WeatherRow row and a WeatherTable table, the method creates a new
	 * WeatherRow owned by table. Each attribute of the created row is a copy (deep)
	 * of the corresponding attribute of row. The method does not add the created
	 * row to its table, its simply parents it to table, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a row with default values for its attributes. 
	 *
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 * @param row  The row which is to be copied.
	 */
	 WeatherRow (WeatherTable &table, WeatherRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute pressure
	
	

	Pressure pressure;

	
	
 	

	
	// ===> Attribute pressureFlag
	
	

	bool pressureFlag;

	
	
 	

	
	// ===> Attribute relHumidity
	
	

	Humidity relHumidity;

	
	
 	

	
	// ===> Attribute relHumidityFlag
	
	

	bool relHumidityFlag;

	
	
 	

	
	// ===> Attribute temperature
	
	

	Temperature temperature;

	
	
 	

	
	// ===> Attribute temperatureFlag
	
	

	bool temperatureFlag;

	
	
 	

	
	// ===> Attribute windDirection
	
	

	Angle windDirection;

	
	
 	

	
	// ===> Attribute windDirectionFlag
	
	

	bool windDirectionFlag;

	
	
 	

	
	// ===> Attribute windSpeed
	
	

	Speed windSpeed;

	
	
 	

	
	// ===> Attribute windSpeedFlag
	
	

	bool windSpeedFlag;

	
	
 	

	
	// ===> Attribute windMax
	
	

	Speed windMax;

	
	
 	

	
	// ===> Attribute windMaxFlag
	
	

	bool windMaxFlag;

	
	
 	

	
	// ===> Attribute dewPoint, which is optional
	
	
	bool dewPointExists;
	

	Temperature dewPoint;

	
	
 	

	
	// ===> Attribute dewPointFlag, which is optional
	
	
	bool dewPointFlagExists;
	

	bool dewPointFlag;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute stationId
	
	

	Tag stationId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, WeatherAttributeFromBin> fromBinMethods;
void stationIdFromBin( EndianISStream& eiss);
void timeIntervalFromBin( EndianISStream& eiss);
void pressureFromBin( EndianISStream& eiss);
void pressureFlagFromBin( EndianISStream& eiss);
void relHumidityFromBin( EndianISStream& eiss);
void relHumidityFlagFromBin( EndianISStream& eiss);
void temperatureFromBin( EndianISStream& eiss);
void temperatureFlagFromBin( EndianISStream& eiss);
void windDirectionFromBin( EndianISStream& eiss);
void windDirectionFlagFromBin( EndianISStream& eiss);
void windSpeedFromBin( EndianISStream& eiss);
void windSpeedFlagFromBin( EndianISStream& eiss);
void windMaxFromBin( EndianISStream& eiss);
void windMaxFlagFromBin( EndianISStream& eiss);

void dewPointFromBin( EndianISStream& eiss);
void dewPointFlagFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the WeatherTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static WeatherRow* fromBin(EndianISStream& eiss, WeatherTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* Weather_CLASS */
