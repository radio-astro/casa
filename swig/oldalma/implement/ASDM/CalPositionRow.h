
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
 * File CalPositionRow.h
 */
 
#ifndef CalPositionRow_CLASS
#define CalPositionRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalPositionRowIDL;
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




	

	

	

	

	

	

	

	

	

	

	

	
#include "CPositionMethod.h"
using namespace PositionMethodMod;
	

	

	

	

	



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

/*\file CalPosition.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::CalPositionTable;


// class asdm::CalReductionRow;
class CalReductionRow;

// class asdm::CalDataRow;
class CalDataRow;
	

/**
 * The CalPositionRow class is a row of a CalPositionTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class CalPositionRow {
friend class asdm::CalPositionTable;

public:

	virtual ~CalPositionRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalPositionTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPositionRowIDL struct.
	 */
	CalPositionRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPositionRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (CalPositionRowIDL x) throw(ConversionException);
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
	 */
	void setFromXML (string rowDoc) throw(ConversionException);
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string getAntennaName() const;
	
 
 	
 	
 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setAntennaName (string antennaName);
  		
	
	
	


	
	// ===> Attribute numAntenna
	
	
	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int getNumAntenna() const;
	
 
 	
 	
 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 		
 			
 	 */
 	void setNumAntenna (int numAntenna);
  		
	
	
	


	
	// ===> Attribute startValidTime
	
	
	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime getStartValidTime() const;
	
 
 	
 	
 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 		
 			
 	 */
 	void setStartValidTime (ArrayTime startValidTime);
  		
	
	
	


	
	// ===> Attribute endValidTime
	
	
	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime getEndValidTime() const;
	
 
 	
 	
 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 		
 			
 	 */
 	void setEndValidTime (ArrayTime endValidTime);
  		
	
	
	


	
	// ===> Attribute positionOffset
	
	
	

	
 	/**
 	 * Get positionOffset.
 	 * @return positionOffset as vector<Length >
 	 */
 	vector<Length > getPositionOffset() const;
	
 
 	
 	
 	/**
 	 * Set positionOffset with the specified vector<Length >.
 	 * @param positionOffset The vector<Length > value to which positionOffset is to be set.
 	 
 		
 			
 	 */
 	void setPositionOffset (vector<Length > positionOffset);
  		
	
	
	


	
	// ===> Attribute positionErr
	
	
	

	
 	/**
 	 * Get positionErr.
 	 * @return positionErr as vector<Length >
 	 */
 	vector<Length > getPositionErr() const;
	
 
 	
 	
 	/**
 	 * Set positionErr with the specified vector<Length >.
 	 * @param positionErr The vector<Length > value to which positionErr is to be set.
 	 
 		
 			
 	 */
 	void setPositionErr (vector<Length > positionErr);
  		
	
	
	


	
	// ===> Attribute delayRms
	
	
	

	
 	/**
 	 * Get delayRms.
 	 * @return delayRms as Interval
 	 */
 	Interval getDelayRms() const;
	
 
 	
 	
 	/**
 	 * Set delayRms with the specified Interval.
 	 * @param delayRms The Interval value to which delayRms is to be set.
 	 
 		
 			
 	 */
 	void setDelayRms (Interval delayRms);
  		
	
	
	


	
	// ===> Attribute phaseRms
	
	
	

	
 	/**
 	 * Get phaseRms.
 	 * @return phaseRms as Angle
 	 */
 	Angle getPhaseRms() const;
	
 
 	
 	
 	/**
 	 * Set phaseRms with the specified Angle.
 	 * @param phaseRms The Angle value to which phaseRms is to be set.
 	 
 		
 			
 	 */
 	void setPhaseRms (Angle phaseRms);
  		
	
	
	


	
	// ===> Attribute axesOffset
	
	
	

	
 	/**
 	 * Get axesOffset.
 	 * @return axesOffset as Length
 	 */
 	Length getAxesOffset() const;
	
 
 	
 	
 	/**
 	 * Set axesOffset with the specified Length.
 	 * @param axesOffset The Length value to which axesOffset is to be set.
 	 
 		
 			
 	 */
 	void setAxesOffset (Length axesOffset);
  		
	
	
	


	
	// ===> Attribute axesOffsetFixed
	
	
	

	
 	/**
 	 * Get axesOffsetFixed.
 	 * @return axesOffsetFixed as bool
 	 */
 	bool getAxesOffsetFixed() const;
	
 
 	
 	
 	/**
 	 * Set axesOffsetFixed with the specified bool.
 	 * @param axesOffsetFixed The bool value to which axesOffsetFixed is to be set.
 	 
 		
 			
 	 */
 	void setAxesOffsetFixed (bool axesOffsetFixed);
  		
	
	
	


	
	// ===> Attribute axesOffsetErr
	
	
	

	
 	/**
 	 * Get axesOffsetErr.
 	 * @return axesOffsetErr as Length
 	 */
 	Length getAxesOffsetErr() const;
	
 
 	
 	
 	/**
 	 * Set axesOffsetErr with the specified Length.
 	 * @param axesOffsetErr The Length value to which axesOffsetErr is to be set.
 	 
 		
 			
 	 */
 	void setAxesOffsetErr (Length axesOffsetErr);
  		
	
	
	


	
	// ===> Attribute positionMethod
	
	
	

	
 	/**
 	 * Get positionMethod.
 	 * @return positionMethod as PositionMethodMod::PositionMethod
 	 */
 	PositionMethodMod::PositionMethod getPositionMethod() const;
	
 
 	
 	
 	/**
 	 * Set positionMethod with the specified PositionMethodMod::PositionMethod.
 	 * @param positionMethod The PositionMethodMod::PositionMethod value to which positionMethod is to be set.
 	 
 		
 			
 	 */
 	void setPositionMethod (PositionMethodMod::PositionMethod positionMethod);
  		
	
	
	


	
	// ===> Attribute refAntennaNames
	
	
	

	
 	/**
 	 * Get refAntennaNames.
 	 * @return refAntennaNames as vector<string >
 	 */
 	vector<string > getRefAntennaNames() const;
	
 
 	
 	
 	/**
 	 * Set refAntennaNames with the specified vector<string >.
 	 * @param refAntennaNames The vector<string > value to which refAntennaNames is to be set.
 	 
 		
 			
 	 */
 	void setRefAntennaNames (vector<string > refAntennaNames);
  		
	
	
	


	
	// ===> Attribute stationName
	
	
	

	
 	/**
 	 * Get stationName.
 	 * @return stationName as string
 	 */
 	string getStationName() const;
	
 
 	
 	
 	/**
 	 * Set stationName with the specified string.
 	 * @param stationName The string value to which stationName is to be set.
 	 
 		
 			
 	 */
 	void setStationName (string stationName);
  		
	
	
	


	
	// ===> Attribute antennaPosition
	
	
	

	
 	/**
 	 * Get antennaPosition.
 	 * @return antennaPosition as vector<Length >
 	 */
 	vector<Length > getAntennaPosition() const;
	
 
 	
 	
 	/**
 	 * Set antennaPosition with the specified vector<Length >.
 	 * @param antennaPosition The vector<Length > value to which antennaPosition is to be set.
 	 
 		
 			
 	 */
 	void setAntennaPosition (vector<Length > antennaPosition);
  		
	
	
	


	
	// ===> Attribute stationPosition
	
	
	

	
 	/**
 	 * Get stationPosition.
 	 * @return stationPosition as vector<Length >
 	 */
 	vector<Length > getStationPosition() const;
	
 
 	
 	
 	/**
 	 * Set stationPosition with the specified vector<Length >.
 	 * @param stationPosition The vector<Length > value to which stationPosition is to be set.
 	 
 		
 			
 	 */
 	void setStationPosition (vector<Length > stationPosition);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute calDataId
	
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag getCalDataId() const;
	
 
 	
 	
 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setCalDataId (Tag calDataId);
  		
	
	
	


	
	// ===> Attribute calReductionId
	
	
	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag getCalReductionId() const;
	
 
 	
 	
 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setCalReductionId (Tag calReductionId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * calReductionId pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* getCalReductionUsingCalReductionId();
	 

	

	

	
		
	/**
	 * calDataId pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* getCalDataUsingCalDataId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this CalPositionRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, int numAntenna, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > positionOffset, vector<Length > positionErr, Interval delayRms, Angle phaseRms, Length axesOffset, bool axesOffsetFixed, Length axesOffsetErr, PositionMethodMod::PositionMethod positionMethod, vector<string > refAntennaNames, string stationName, vector<Length > antennaPosition, vector<Length > stationPosition);
	
	

	
	bool compareRequiredValue(int numAntenna, ArrayTime startValidTime, ArrayTime endValidTime, vector<Length > positionOffset, vector<Length > positionErr, Interval delayRms, Angle phaseRms, Length axesOffset, bool axesOffsetFixed, Length axesOffsetErr, PositionMethodMod::PositionMethod positionMethod, vector<string > refAntennaNames, string stationName, vector<Length > antennaPosition, vector<Length > stationPosition); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPositionRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalPositionRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	CalPositionTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalPositionRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPositionRow (CalPositionTable &table);

	/**
	 * Create a CalPositionRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalPositionRow row and a CalPositionTable table, the method creates a new
	 * CalPositionRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalPositionRow (CalPositionTable &table, CalPositionRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute numAntenna
	
	

	int numAntenna;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute positionOffset
	
	

	vector<Length > positionOffset;

	
	
 	

	
	// ===> Attribute positionErr
	
	

	vector<Length > positionErr;

	
	
 	

	
	// ===> Attribute delayRms
	
	

	Interval delayRms;

	
	
 	

	
	// ===> Attribute phaseRms
	
	

	Angle phaseRms;

	
	
 	

	
	// ===> Attribute axesOffset
	
	

	Length axesOffset;

	
	
 	

	
	// ===> Attribute axesOffsetFixed
	
	

	bool axesOffsetFixed;

	
	
 	

	
	// ===> Attribute axesOffsetErr
	
	

	Length axesOffsetErr;

	
	
 	

	
	// ===> Attribute positionMethod
	
	

	PositionMethodMod::PositionMethod positionMethod;

	
	
 	

	
	// ===> Attribute refAntennaNames
	
	

	vector<string > refAntennaNames;

	
	
 	

	
	// ===> Attribute stationName
	
	

	string stationName;

	
	
 	

	
	// ===> Attribute antennaPosition
	
	

	vector<Length > antennaPosition;

	
	
 	

	
	// ===> Attribute stationPosition
	
	

	vector<Length > stationPosition;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute calDataId
	
	

	Tag calDataId;

	
	
 	

	
	// ===> Attribute calReductionId
	
	

	Tag calReductionId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	


};

} // End namespace asdm

#endif /* CalPosition_CLASS */
