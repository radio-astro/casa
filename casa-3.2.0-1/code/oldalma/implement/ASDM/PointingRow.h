
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
 * File PointingRow.h
 */
 
#ifndef PointingRow_CLASS
#define PointingRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::PointingRowIDL;
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

#include <EndianStream.h>

 
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

using asdm::EndianOSStream;
using asdm::EndianISStream;

namespace asdm {

//class asdm::PointingTable;


// class asdm::AntennaRow;
class AntennaRow;

// class asdm::PointingModelRow;
class PointingModelRow;
	

/**
 * The PointingRow class is a row of a PointingTable.
 */
class PointingRow {
friend class asdm::PointingTable;

public:

	virtual ~PointingRow();

	/**
	 * Return the table to which this row belongs.
	 */
	PointingTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a PointingRowIDL struct.
	 */
	PointingRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct PointingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (PointingRowIDL x) throw(ConversionException);
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
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @table the PointingTable to which the row built by deserialization will be parented.
	  */
	 static PointingRow* fromBin(EndianISStream& eiss, PointingTable& table);

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
 	void setTimeInterval (ArrayTimeInterval timeInterval) throw(IllegalAccessException);
  		
	
	// ===> Attribute name, which is optional
	
	
	
	/**
	 * The attribute name is optional. Return true if this attribute exists.
	 * @return true if and only if the name attribute exists. 
	 */
	bool isNameExists() const;
	
 	/**
 	 * Get name, which is optional.
 	 * @return name as string
 	 * @throws IllegalAccessException If name does not exist.
 	 */
 	string getName() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 		
 	 */
 	void setName (string name);
		
	/**
	 * Mark name, which is an optional field, as non-existent.
	 */
	void clearName ();	
	
	
	// ===> Attribute numPoly
	
	
	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int getNumPoly() const;
	
 
 	
 	
 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 		
 	 */
 	void setNumPoly (int numPoly);
		
	
	
	


	
	// ===> Attribute timeOrigin
	
	
	

	
 	/**
 	 * Get timeOrigin.
 	 * @return timeOrigin as ArrayTime
 	 */
 	ArrayTime getTimeOrigin() const;
	
 
 	
 	
 	/**
 	 * Set timeOrigin with the specified ArrayTime.
 	 * @param timeOrigin The ArrayTime value to which timeOrigin is to be set.
 		
 	 */
 	void setTimeOrigin (ArrayTime timeOrigin);
		
	
	
	


	
	// ===> Attribute pointingDirection
	
	
	

	
 	/**
 	 * Get pointingDirection.
 	 * @return pointingDirection as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getPointingDirection() const;
	
 
 	
 	
 	/**
 	 * Set pointingDirection with the specified vector<vector<Angle > >.
 	 * @param pointingDirection The vector<vector<Angle > > value to which pointingDirection is to be set.
 		
 	 */
 	void setPointingDirection (vector<vector<Angle > > pointingDirection);
		
	
	
	


	
	// ===> Attribute target
	
	
	

	
 	/**
 	 * Get target.
 	 * @return target as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getTarget() const;
	
 
 	
 	
 	/**
 	 * Set target with the specified vector<vector<Angle > >.
 	 * @param target The vector<vector<Angle > > value to which target is to be set.
 		
 	 */
 	void setTarget (vector<vector<Angle > > target);
		
	
	
	


	
	// ===> Attribute offset
	
	
	

	
 	/**
 	 * Get offset.
 	 * @return offset as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getOffset() const;
	
 
 	
 	
 	/**
 	 * Set offset with the specified vector<vector<Angle > >.
 	 * @param offset The vector<vector<Angle > > value to which offset is to be set.
 		
 	 */
 	void setOffset (vector<vector<Angle > > offset);
		

	
	// ===> Attribute sourceOffset, which is optional
	
	
	
	/**
	 * The attribute sourceOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceOffset attribute exists. 
	 */
	bool isSourceOffsetExists() const;
	

	
 	/**
 	 * Get sourceOffset, which is optional.
 	 * @return sourceOffset as vector<vector<Angle > >
 	 * @throws IllegalAccessException If sourceOffset does not exist.
 	 */
 	vector<vector<Angle > > getSourceOffset() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sourceOffset with the specified vector<vector<Angle > >.
 	 * @param sourceOffset The vector<vector<Angle > > value to which sourceOffset is to be set.
 		
 	 */
 	void setSourceOffset (vector<vector<Angle > > sourceOffset);
		
	
	
	
	/**
	 * Mark sourceOffset, which is an optional field, as non-existent.
	 */
	void clearSourceOffset ();
	


	
	// ===> Attribute encoder
	
	
	

	
 	/**
 	 * Get encoder.
 	 * @return encoder as vector<Angle >
 	 */
 	vector<Angle > getEncoder() const;
	
 
 	
 	
 	/**
 	 * Set encoder with the specified vector<Angle >.
 	 * @param encoder The vector<Angle > value to which encoder is to be set.
 		
 	 */
 	void setEncoder (vector<Angle > encoder);
		
	
	
	


	
	// ===> Attribute pointingTracking
	
	
	

	
 	/**
 	 * Get pointingTracking.
 	 * @return pointingTracking as bool
 	 */
 	bool getPointingTracking() const;
	
 
 	
 	
 	/**
 	 * Set pointingTracking with the specified bool.
 	 * @param pointingTracking The bool value to which pointingTracking is to be set.
 		
 	 */
 	void setPointingTracking (bool pointingTracking);
		
	
	// ===> Attribute phaseTracking, which is optional
	

	/**
	 * The attribute phaseTracking is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseTracking attribute exists. 
	 */
	bool isPhaseTrackingExists() const;
	
	
	

	
 	/**
 	 * Get phaseTracking, which is optional.
 	 * @return phaseTracking as bool
 	 * @throws IllegalAccessException If phaseTracking does not exist.
 	 */
 	bool getPhaseTracking() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set phaseTracking with the specified bool.
 	 * @param phaseTracking The bool value to which phaseTracking is to be set.
 		
 	 */
 	void setPhaseTracking (bool phaseTracking);
		
	
	
	
	/**
	 * Mark phaseTracking, which is an optional field, as non-existent.
	 */
	void clearPhaseTracking ();	


	
	// ===> Attribute overTheTop, which is optional
	
	
	
	/**
	 * The attribute overTheTop is optional. Return true if this attribute exists.
	 * @return true if and only if the overTheTop attribute exists. 
	 */
	bool isOverTheTopExists() const;
	

	
 	/**
 	 * Get overTheTop, which is optional.
 	 * @return overTheTop as bool
 	 * @throws IllegalAccessException If overTheTop does not exist.
 	 */
 	bool getOverTheTop() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set overTheTop with the specified bool.
 	 * @param overTheTop The bool value to which overTheTop is to be set.
 		
 	 */
 	void setOverTheTop (bool overTheTop);
		
	
	
	
	/**
	 * Mark overTheTop, which is an optional field, as non-existent.
	 */
	void clearOverTheTop ();
	


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
 	void setAntennaId (Tag antennaId) throw(IllegalAccessException);
  		
	
	
	


	
	// ===> Attribute pointingModelId
	
	
	

	
 	/**
 	 * Get pointingModelId.
 	 * @return pointingModelId as int
 	 */
 	int getPointingModelId() const;
	
 
 	
 	
 	/**
 	 * Set pointingModelId with the specified int.
 	 * @param pointingModelId The int value to which pointingModelId is to be set.
 		
 	 */
 	void setPointingModelId (int pointingModelId);
		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	

	
		
	// ===> Slice link from a row of Pointing table to a collection of row of PointingModel table.
	
	/**
	 * Get the collection of row in the PointingModel table having pointingModelId == this.pointingModelId
	 * 
	 * @return a vector of PointingModelRow *
	 */
	vector <PointingModelRow *> getPointingModels();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this PointingRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag antennaId, ArrayTimeInterval timeInterval, int pointingModelId, int numPoly, ArrayTime timeOrigin, vector<vector<Angle > > pointingDirection, vector<vector<Angle > > target, vector<vector<Angle > > offset, vector<Angle > encoder, bool pointingTracking);
	

	
	bool compareRequiredValue(int pointingModelId, int numPoly, ArrayTime timeOrigin, vector<vector<Angle > > pointingDirection, vector<vector<Angle > > target, vector<vector<Angle > > offset, vector<Angle > encoder, bool pointingTracking); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the PointingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(PointingRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	PointingTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a PointingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	PointingRow (PointingTable &table);

	/**
	 * Create a PointingRow using a copy constructor mechanism.
	 * <p>
	 * Given a PointingRow row and a PointingTable table, the method creates a new
	 * PointingRow owned by table. Each attribute of the created row is a copy (deep)
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
	 PointingRow (PointingTable &table, PointingRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	
	ArrayTimeInterval timeInterval;
	
	
 	

	
	// ===> Attribute name, which is optional
	
	
	bool nameExists;
	
	string name;
	
	
 	

	
	// ===> Attribute numPoly
	
	
	int numPoly;
	
	
 	

	
	// ===> Attribute timeOrigin
	
	
	ArrayTime timeOrigin;
	
	
 	

	
	// ===> Attribute pointingDirection
	
	
	vector<vector<Angle > > pointingDirection;
	
	
 	

	
	// ===> Attribute target
	
	
	vector<vector<Angle > > target;
	
	
 	

	
	// ===> Attribute offset
	
	
	vector<vector<Angle > > offset;
	
	
 	

	
	// ===> Attribute sourceOffset, which is optional
	
	
	bool sourceOffsetExists;
	
	vector<vector<Angle > > sourceOffset;
	
	
 	

	
	// ===> Attribute encoder
	
	
	vector<Angle > encoder;
	
	
 	

	
	// ===> Attribute pointingTracking
	
	
	bool pointingTracking;
	
	
	// ===> Attribute phaseTracking, which is optional
	
	
	bool phaseTrackingExists;
	
	bool phaseTracking;
	
	
	
	// ===> Attribute overTheTop, which is optional
	
	
	bool overTheTopExists;
	
	bool overTheTop;
	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	
	Tag antennaId;
	
	
 	

	
	// ===> Attribute pointingModelId
	
	
	int pointingModelId;
	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		


	


};

} // End namespace asdm

#endif /* Pointing_CLASS */
