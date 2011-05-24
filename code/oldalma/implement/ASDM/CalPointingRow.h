
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
 * File CalPointingRow.h
 */
 
#ifndef CalPointingRow_CLASS
#define CalPointingRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalPointingRowIDL;
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




	

	

	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	

	

	

	

	

	
#include "CPointingMethod.h"
using namespace PointingMethodMod;
	

	

	

	

	

	

	

	
#include "CPointingModelMode.h"
using namespace PointingModelModeMod;
	

	

	

	

	



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

/*\file CalPointing.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::CalPointingTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

/**
 * The CalPointingRow class is a row of a CalPointingTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class CalPointingRow {
friend class asdm::CalPointingTable;

public:

	virtual ~CalPointingRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalPointingTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPointingRowIDL struct.
	 */
	CalPointingRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPointingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (CalPointingRowIDL x) throw(ConversionException);
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
  		
	
	
	


	
	// ===> Attribute receiverBand
	
	
	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand getReceiverBand() const;
	
 
 	
 	
 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 		
 			
 	 */
 	void setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand);
  		
	
	
	


	
	// ===> Attribute frequencyRange
	
	
	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > getFrequencyRange() const;
	
 
 	
 	
 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyRange (vector<Frequency > frequencyRange);
  		
	
	
	


	
	// ===> Attribute direction
	
	
	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > getDirection() const;
	
 
 	
 	
 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 		
 			
 	 */
 	void setDirection (vector<Angle > direction);
  		
	
	
	


	
	// ===> Attribute collOffsetRelative
	
	
	

	
 	/**
 	 * Get collOffsetRelative.
 	 * @return collOffsetRelative as vector<Angle >
 	 */
 	vector<Angle > getCollOffsetRelative() const;
	
 
 	
 	
 	/**
 	 * Set collOffsetRelative with the specified vector<Angle >.
 	 * @param collOffsetRelative The vector<Angle > value to which collOffsetRelative is to be set.
 	 
 		
 			
 	 */
 	void setCollOffsetRelative (vector<Angle > collOffsetRelative);
  		
	
	
	


	
	// ===> Attribute collOffsetAbsolute
	
	
	

	
 	/**
 	 * Get collOffsetAbsolute.
 	 * @return collOffsetAbsolute as vector<Angle >
 	 */
 	vector<Angle > getCollOffsetAbsolute() const;
	
 
 	
 	
 	/**
 	 * Set collOffsetAbsolute with the specified vector<Angle >.
 	 * @param collOffsetAbsolute The vector<Angle > value to which collOffsetAbsolute is to be set.
 	 
 		
 			
 	 */
 	void setCollOffsetAbsolute (vector<Angle > collOffsetAbsolute);
  		
	
	
	


	
	// ===> Attribute collError
	
	
	

	
 	/**
 	 * Get collError.
 	 * @return collError as vector<Angle >
 	 */
 	vector<Angle > getCollError() const;
	
 
 	
 	
 	/**
 	 * Set collError with the specified vector<Angle >.
 	 * @param collError The vector<Angle > value to which collError is to be set.
 	 
 		
 			
 	 */
 	void setCollError (vector<Angle > collError);
  		
	
	
	


	
	// ===> Attribute pointingMethod
	
	
	

	
 	/**
 	 * Get pointingMethod.
 	 * @return pointingMethod as PointingMethodMod::PointingMethod
 	 */
 	PointingMethodMod::PointingMethod getPointingMethod() const;
	
 
 	
 	
 	/**
 	 * Set pointingMethod with the specified PointingMethodMod::PointingMethod.
 	 * @param pointingMethod The PointingMethodMod::PointingMethod value to which pointingMethod is to be set.
 	 
 		
 			
 	 */
 	void setPointingMethod (PointingMethodMod::PointingMethod pointingMethod);
  		
	
	
	


	
	// ===> Attribute beamWidth, which is optional
	
	
	
	/**
	 * The attribute beamWidth is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidth attribute exists. 
	 */
	bool isBeamWidthExists() const;
	

	
 	/**
 	 * Get beamWidth, which is optional.
 	 * @return beamWidth as vector<Angle >
 	 * @throws IllegalAccessException If beamWidth does not exist.
 	 */
 	vector<Angle > getBeamWidth() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set beamWidth with the specified vector<Angle >.
 	 * @param beamWidth The vector<Angle > value to which beamWidth is to be set.
 	 
 		
 	 */
 	void setBeamWidth (vector<Angle > beamWidth);
		
	
	
	
	/**
	 * Mark beamWidth, which is an optional field, as non-existent.
	 */
	void clearBeamWidth ();
	


	
	// ===> Attribute beamWidthError, which is optional
	
	
	
	/**
	 * The attribute beamWidthError is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidthError attribute exists. 
	 */
	bool isBeamWidthErrorExists() const;
	

	
 	/**
 	 * Get beamWidthError, which is optional.
 	 * @return beamWidthError as vector<Angle >
 	 * @throws IllegalAccessException If beamWidthError does not exist.
 	 */
 	vector<Angle > getBeamWidthError() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set beamWidthError with the specified vector<Angle >.
 	 * @param beamWidthError The vector<Angle > value to which beamWidthError is to be set.
 	 
 		
 	 */
 	void setBeamWidthError (vector<Angle > beamWidthError);
		
	
	
	
	/**
	 * Mark beamWidthError, which is an optional field, as non-existent.
	 */
	void clearBeamWidthError ();
	


	
	// ===> Attribute beamPA, which is optional
	
	
	
	/**
	 * The attribute beamPA is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPA attribute exists. 
	 */
	bool isBeamPAExists() const;
	

	
 	/**
 	 * Get beamPA, which is optional.
 	 * @return beamPA as Angle
 	 * @throws IllegalAccessException If beamPA does not exist.
 	 */
 	Angle getBeamPA() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set beamPA with the specified Angle.
 	 * @param beamPA The Angle value to which beamPA is to be set.
 	 
 		
 	 */
 	void setBeamPA (Angle beamPA);
		
	
	
	
	/**
	 * Mark beamPA, which is an optional field, as non-existent.
	 */
	void clearBeamPA ();
	


	
	// ===> Attribute beamPAError, which is optional
	
	
	
	/**
	 * The attribute beamPAError is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPAError attribute exists. 
	 */
	bool isBeamPAErrorExists() const;
	

	
 	/**
 	 * Get beamPAError, which is optional.
 	 * @return beamPAError as Angle
 	 * @throws IllegalAccessException If beamPAError does not exist.
 	 */
 	Angle getBeamPAError() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set beamPAError with the specified Angle.
 	 * @param beamPAError The Angle value to which beamPAError is to be set.
 	 
 		
 	 */
 	void setBeamPAError (Angle beamPAError);
		
	
	
	
	/**
	 * Mark beamPAError, which is an optional field, as non-existent.
	 */
	void clearBeamPAError ();
	


	
	// ===> Attribute peakIntensity, which is optional
	
	
	
	/**
	 * The attribute peakIntensity is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensity attribute exists. 
	 */
	bool isPeakIntensityExists() const;
	

	
 	/**
 	 * Get peakIntensity, which is optional.
 	 * @return peakIntensity as Temperature
 	 * @throws IllegalAccessException If peakIntensity does not exist.
 	 */
 	Temperature getPeakIntensity() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set peakIntensity with the specified Temperature.
 	 * @param peakIntensity The Temperature value to which peakIntensity is to be set.
 	 
 		
 	 */
 	void setPeakIntensity (Temperature peakIntensity);
		
	
	
	
	/**
	 * Mark peakIntensity, which is an optional field, as non-existent.
	 */
	void clearPeakIntensity ();
	


	
	// ===> Attribute peakIntensityError, which is optional
	
	
	
	/**
	 * The attribute peakIntensityError is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityError attribute exists. 
	 */
	bool isPeakIntensityErrorExists() const;
	

	
 	/**
 	 * Get peakIntensityError, which is optional.
 	 * @return peakIntensityError as Temperature
 	 * @throws IllegalAccessException If peakIntensityError does not exist.
 	 */
 	Temperature getPeakIntensityError() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set peakIntensityError with the specified Temperature.
 	 * @param peakIntensityError The Temperature value to which peakIntensityError is to be set.
 	 
 		
 	 */
 	void setPeakIntensityError (Temperature peakIntensityError);
		
	
	
	
	/**
	 * Mark peakIntensityError, which is an optional field, as non-existent.
	 */
	void clearPeakIntensityError ();
	


	
	// ===> Attribute mode
	
	
	

	
 	/**
 	 * Get mode.
 	 * @return mode as PointingModelModeMod::PointingModelMode
 	 */
 	PointingModelModeMod::PointingModelMode getMode() const;
	
 
 	
 	
 	/**
 	 * Set mode with the specified PointingModelModeMod::PointingModelMode.
 	 * @param mode The PointingModelModeMod::PointingModelMode value to which mode is to be set.
 	 
 		
 			
 	 */
 	void setMode (PointingModelModeMod::PointingModelMode mode);
  		
	
	
	


	
	// ===> Attribute beamWidthWasFixed, which is optional
	
	
	
	/**
	 * The attribute beamWidthWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the beamWidthWasFixed attribute exists. 
	 */
	bool isBeamWidthWasFixedExists() const;
	

	
 	/**
 	 * Get beamWidthWasFixed, which is optional.
 	 * @return beamWidthWasFixed as vector<bool >
 	 * @throws IllegalAccessException If beamWidthWasFixed does not exist.
 	 */
 	vector<bool > getBeamWidthWasFixed() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set beamWidthWasFixed with the specified vector<bool >.
 	 * @param beamWidthWasFixed The vector<bool > value to which beamWidthWasFixed is to be set.
 	 
 		
 	 */
 	void setBeamWidthWasFixed (vector<bool > beamWidthWasFixed);
		
	
	
	
	/**
	 * Mark beamWidthWasFixed, which is an optional field, as non-existent.
	 */
	void clearBeamWidthWasFixed ();
	


	
	// ===> Attribute beamPAWasFixed, which is optional
	
	
	
	/**
	 * The attribute beamPAWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the beamPAWasFixed attribute exists. 
	 */
	bool isBeamPAWasFixedExists() const;
	

	
 	/**
 	 * Get beamPAWasFixed, which is optional.
 	 * @return beamPAWasFixed as bool
 	 * @throws IllegalAccessException If beamPAWasFixed does not exist.
 	 */
 	bool getBeamPAWasFixed() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set beamPAWasFixed with the specified bool.
 	 * @param beamPAWasFixed The bool value to which beamPAWasFixed is to be set.
 	 
 		
 	 */
 	void setBeamPAWasFixed (bool beamPAWasFixed);
		
	
	
	
	/**
	 * Mark beamPAWasFixed, which is an optional field, as non-existent.
	 */
	void clearBeamPAWasFixed ();
	


	
	// ===> Attribute peakIntensityWasFixed, which is optional
	
	
	
	/**
	 * The attribute peakIntensityWasFixed is optional. Return true if this attribute exists.
	 * @return true if and only if the peakIntensityWasFixed attribute exists. 
	 */
	bool isPeakIntensityWasFixedExists() const;
	

	
 	/**
 	 * Get peakIntensityWasFixed, which is optional.
 	 * @return peakIntensityWasFixed as bool
 	 * @throws IllegalAccessException If peakIntensityWasFixed does not exist.
 	 */
 	bool getPeakIntensityWasFixed() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set peakIntensityWasFixed with the specified bool.
 	 * @param peakIntensityWasFixed The bool value to which peakIntensityWasFixed is to be set.
 	 
 		
 	 */
 	void setPeakIntensityWasFixed (bool peakIntensityWasFixed);
		
	
	
	
	/**
	 * Mark peakIntensityWasFixed, which is an optional field, as non-existent.
	 */
	void clearPeakIntensityWasFixed ();
	


	
	// ===> Attribute ambientTemperature
	
	
	

	
 	/**
 	 * Get ambientTemperature.
 	 * @return ambientTemperature as Temperature
 	 */
 	Temperature getAmbientTemperature() const;
	
 
 	
 	
 	/**
 	 * Set ambientTemperature with the specified Temperature.
 	 * @param ambientTemperature The Temperature value to which ambientTemperature is to be set.
 	 
 		
 			
 	 */
 	void setAmbientTemperature (Temperature ambientTemperature);
  		
	
	
	


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
	 * calDataId pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* getCalDataUsingCalDataId();
	 

	

	

	
		
	/**
	 * calReductionId pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* getCalReductionUsingCalReductionId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this CalPointingRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Frequency > frequencyRange, vector<Angle > direction, vector<Angle > collOffsetRelative, vector<Angle > collOffsetAbsolute, vector<Angle > collError, PointingMethodMod::PointingMethod pointingMethod, PointingModelModeMod::PointingModelMode mode, Temperature ambientTemperature);
	
	

	
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, ReceiverBandMod::ReceiverBand receiverBand, vector<Frequency > frequencyRange, vector<Angle > direction, vector<Angle > collOffsetRelative, vector<Angle > collOffsetAbsolute, vector<Angle > collError, PointingMethodMod::PointingMethod pointingMethod, PointingModelModeMod::PointingModelMode mode, Temperature ambientTemperature); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPointingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalPointingRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	CalPointingTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalPointingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPointingRow (CalPointingTable &table);

	/**
	 * Create a CalPointingRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalPointingRow row and a CalPointingTable table, the method creates a new
	 * CalPointingRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalPointingRow (CalPointingTable &table, CalPointingRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute direction
	
	

	vector<Angle > direction;

	
	
 	

	
	// ===> Attribute collOffsetRelative
	
	

	vector<Angle > collOffsetRelative;

	
	
 	

	
	// ===> Attribute collOffsetAbsolute
	
	

	vector<Angle > collOffsetAbsolute;

	
	
 	

	
	// ===> Attribute collError
	
	

	vector<Angle > collError;

	
	
 	

	
	// ===> Attribute pointingMethod
	
	

	PointingMethodMod::PointingMethod pointingMethod;

	
	
 	

	
	// ===> Attribute beamWidth, which is optional
	
	
	bool beamWidthExists;
	

	vector<Angle > beamWidth;

	
	
 	

	
	// ===> Attribute beamWidthError, which is optional
	
	
	bool beamWidthErrorExists;
	

	vector<Angle > beamWidthError;

	
	
 	

	
	// ===> Attribute beamPA, which is optional
	
	
	bool beamPAExists;
	

	Angle beamPA;

	
	
 	

	
	// ===> Attribute beamPAError, which is optional
	
	
	bool beamPAErrorExists;
	

	Angle beamPAError;

	
	
 	

	
	// ===> Attribute peakIntensity, which is optional
	
	
	bool peakIntensityExists;
	

	Temperature peakIntensity;

	
	
 	

	
	// ===> Attribute peakIntensityError, which is optional
	
	
	bool peakIntensityErrorExists;
	

	Temperature peakIntensityError;

	
	
 	

	
	// ===> Attribute mode
	
	

	PointingModelModeMod::PointingModelMode mode;

	
	
 	

	
	// ===> Attribute beamWidthWasFixed, which is optional
	
	
	bool beamWidthWasFixedExists;
	

	vector<bool > beamWidthWasFixed;

	
	
 	

	
	// ===> Attribute beamPAWasFixed, which is optional
	
	
	bool beamPAWasFixedExists;
	

	bool beamPAWasFixed;

	
	
 	

	
	// ===> Attribute peakIntensityWasFixed, which is optional
	
	
	bool peakIntensityWasFixedExists;
	

	bool peakIntensityWasFixed;

	
	
 	

	
	// ===> Attribute ambientTemperature
	
	

	Temperature ambientTemperature;

	
	
 	

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

#endif /* CalPointing_CLASS */
