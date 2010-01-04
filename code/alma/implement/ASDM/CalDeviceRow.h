
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
 * File CalDeviceRow.h
 */
 
#ifndef CalDeviceRow_CLASS
#define CalDeviceRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalDeviceRowIDL;
#endif

#include <Angle.h>
#include <AngularRate.h>
#include <ArrayTime.h>
#include <ArrayTimeInterval.h>
#include <Complex.h>
#include <Entity.h>
#include <EntityId.h>
#include <EntityRef.h>
#include <Flux.h>
#include <Frequency.h>
#include <Humidity.h>
#include <Interval.h>
#include <Length.h>
#include <Pressure.h>
#include <Speed.h>
#include <Tag.h>
#include <Temperature.h>
#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>

/*
#include <Enumerations.h>
using namespace enumerations;
 */




	

	

	
#include "CCalibrationDevice.h"
using namespace CalibrationDeviceMod;
	

	

	

	

	



using asdm::Angle;
using asdm::AngularRate;
using asdm::ArrayTime;
using asdm::Complex;
using asdm::Entity;
using asdm::EntityId;
using asdm::EntityRef;
using asdm::Flux;
using asdm::Frequency;
using asdm::Humidity;
using asdm::Interval;
using asdm::Length;
using asdm::Pressure;
using asdm::Speed;
using asdm::Tag;
using asdm::Temperature;
using asdm::ConversionException;
using asdm::NoSuchRow;
using asdm::IllegalAccessException;

/*\file CalDevice.h
    \brief Generated from model's revision "1.52", branch "HEAD"
*/

namespace asdm {

//class asdm::CalDeviceTable;


// class asdm::AntennaRow;
class AntennaRow;

// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::FeedRow;
class FeedRow;
	

/**
 * The CalDeviceRow class is a row of a CalDeviceTable.
 * 
 * Generated from model's revision "1.52", branch "HEAD"
 *
 */
class CalDeviceRow {
friend class asdm::CalDeviceTable;

public:

	virtual ~CalDeviceRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalDeviceTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalDeviceRowIDL struct.
	 */
	CalDeviceRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalDeviceRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (CalDeviceRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (string rowDoc) ;
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @table the CalDeviceTable to which the row built by deserialization will be parented.
	  */
	 static CalDeviceRow* fromBin(EndianISStream& eiss, CalDeviceTable& table);	 
	
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
  		
	
	
	


	
	// ===> Attribute numCalload
	
	
	

	
 	/**
 	 * Get numCalload.
 	 * @return numCalload as int
 	 */
 	int getNumCalload() const;
	
 
 	
 	
 	/**
 	 * Set numCalload with the specified int.
 	 * @param numCalload The int value to which numCalload is to be set.
 	 
 		
 			
 	 */
 	void setNumCalload (int numCalload);
  		
	
	
	


	
	// ===> Attribute calLoadNames
	
	
	

	
 	/**
 	 * Get calLoadNames.
 	 * @return calLoadNames as vector<CalibrationDeviceMod::CalibrationDevice >
 	 */
 	vector<CalibrationDeviceMod::CalibrationDevice > getCalLoadNames() const;
	
 
 	
 	
 	/**
 	 * Set calLoadNames with the specified vector<CalibrationDeviceMod::CalibrationDevice >.
 	 * @param calLoadNames The vector<CalibrationDeviceMod::CalibrationDevice > value to which calLoadNames is to be set.
 	 
 		
 			
 	 */
 	void setCalLoadNames (vector<CalibrationDeviceMod::CalibrationDevice > calLoadNames);
  		
	
	
	


	
	// ===> Attribute numReceptor, which is optional
	
	
	
	/**
	 * The attribute numReceptor is optional. Return true if this attribute exists.
	 * @return true if and only if the numReceptor attribute exists. 
	 */
	bool isNumReceptorExists() const;
	

	
 	/**
 	 * Get numReceptor, which is optional.
 	 * @return numReceptor as int
 	 * @throws IllegalAccessException If numReceptor does not exist.
 	 */
 	int getNumReceptor() const;
	
 
 	
 	
 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 		
 	 */
 	void setNumReceptor (int numReceptor);
		
	
	
	
	/**
	 * Mark numReceptor, which is an optional field, as non-existent.
	 */
	void clearNumReceptor ();
	


	
	// ===> Attribute calEff, which is optional
	
	
	
	/**
	 * The attribute calEff is optional. Return true if this attribute exists.
	 * @return true if and only if the calEff attribute exists. 
	 */
	bool isCalEffExists() const;
	

	
 	/**
 	 * Get calEff, which is optional.
 	 * @return calEff as vector<vector<float > >
 	 * @throws IllegalAccessException If calEff does not exist.
 	 */
 	vector<vector<float > > getCalEff() const;
	
 
 	
 	
 	/**
 	 * Set calEff with the specified vector<vector<float > >.
 	 * @param calEff The vector<vector<float > > value to which calEff is to be set.
 	 
 		
 	 */
 	void setCalEff (vector<vector<float > > calEff);
		
	
	
	
	/**
	 * Mark calEff, which is an optional field, as non-existent.
	 */
	void clearCalEff ();
	


	
	// ===> Attribute noiseCal, which is optional
	
	
	
	/**
	 * The attribute noiseCal is optional. Return true if this attribute exists.
	 * @return true if and only if the noiseCal attribute exists. 
	 */
	bool isNoiseCalExists() const;
	

	
 	/**
 	 * Get noiseCal, which is optional.
 	 * @return noiseCal as vector<double >
 	 * @throws IllegalAccessException If noiseCal does not exist.
 	 */
 	vector<double > getNoiseCal() const;
	
 
 	
 	
 	/**
 	 * Set noiseCal with the specified vector<double >.
 	 * @param noiseCal The vector<double > value to which noiseCal is to be set.
 	 
 		
 	 */
 	void setNoiseCal (vector<double > noiseCal);
		
	
	
	
	/**
	 * Mark noiseCal, which is an optional field, as non-existent.
	 */
	void clearNoiseCal ();
	


	
	// ===> Attribute temperatureLoad, which is optional
	
	
	
	/**
	 * The attribute temperatureLoad is optional. Return true if this attribute exists.
	 * @return true if and only if the temperatureLoad attribute exists. 
	 */
	bool isTemperatureLoadExists() const;
	

	
 	/**
 	 * Get temperatureLoad, which is optional.
 	 * @return temperatureLoad as vector<Temperature >
 	 * @throws IllegalAccessException If temperatureLoad does not exist.
 	 */
 	vector<Temperature > getTemperatureLoad() const;
	
 
 	
 	
 	/**
 	 * Set temperatureLoad with the specified vector<Temperature >.
 	 * @param temperatureLoad The vector<Temperature > value to which temperatureLoad is to be set.
 	 
 		
 	 */
 	void setTemperatureLoad (vector<Temperature > temperatureLoad);
		
	
	
	
	/**
	 * Mark temperatureLoad, which is an optional field, as non-existent.
	 */
	void clearTemperatureLoad ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag getAntennaId() const;
	
 
 	
 	
 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setAntennaId (Tag antennaId);
  		
	
	
	


	
	// ===> Attribute feedId
	
	
	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int getFeedId() const;
	
 
 	
 	
 	/**
 	 * Set feedId with the specified int.
 	 * @param feedId The int value to which feedId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setFeedId (int feedId);
  		
	
	
	


	
	// ===> Attribute spectralWindowId
	
	
	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag getSpectralWindowId() const;
	
 
 	
 	
 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSpectralWindowId (Tag spectralWindowId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	

	
		
	/**
	 * spectralWindowId pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* getSpectralWindowUsingSpectralWindowId();
	 

	

	

	
		
	// ===> Slice link from a row of CalDevice table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having feedId == this.feedId
	 * 
	 * @return a vector of FeedRow *
	 */
	vector <FeedRow *> getFeeds();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this CalDeviceRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, int numCalload, vector<CalibrationDeviceMod::CalibrationDevice > calLoadNames);
	
	

	
	bool compareRequiredValue(int numCalload, vector<CalibrationDeviceMod::CalibrationDevice > calLoadNames); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalDeviceRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalDeviceRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	CalDeviceTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalDeviceRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalDeviceRow (CalDeviceTable &table);

	/**
	 * Create a CalDeviceRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalDeviceRow row and a CalDeviceTable table, the method creates a new
	 * CalDeviceRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalDeviceRow (CalDeviceTable &table, CalDeviceRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute numCalload
	
	

	int numCalload;

	
	
 	

	
	// ===> Attribute calLoadNames
	
	

	vector<CalibrationDeviceMod::CalibrationDevice > calLoadNames;

	
	
 	

	
	// ===> Attribute numReceptor, which is optional
	
	
	bool numReceptorExists;
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute calEff, which is optional
	
	
	bool calEffExists;
	

	vector<vector<float > > calEff;

	
	
 	

	
	// ===> Attribute noiseCal, which is optional
	
	
	bool noiseCalExists;
	

	vector<double > noiseCal;

	
	
 	

	
	// ===> Attribute temperatureLoad, which is optional
	
	
	bool temperatureLoadExists;
	

	vector<Temperature > temperatureLoad;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	Tag antennaId;

	
	
 	

	
	// ===> Attribute feedId
	
	

	int feedId;

	
	
 	

	
	// ===> Attribute spectralWindowId
	
	

	Tag spectralWindowId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	

	
		


	


};

} // End namespace asdm

#endif /* CalDevice_CLASS */
