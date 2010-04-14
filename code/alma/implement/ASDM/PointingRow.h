
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



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;




	

	

	

	

	

	

	

	

	

	

	

	

	
#include "CDirectionReferenceCode.h"
using namespace DirectionReferenceCodeMod;
	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file Pointing.h
    \brief Generated from model's revision "1.53", branch "HEAD"
*/

namespace asdm {

//class asdm::PointingTable;


// class asdm::PointingModelRow;
class PointingModelRow;

// class asdm::AntennaRow;
class AntennaRow;
	

class PointingRow;
typedef void (PointingRow::*PointingAttributeFromBin) (EndianISStream& eiss);

/**
 * The PointingRow class is a row of a PointingTable.
 * 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 */
class PointingRow {
friend class asdm::PointingTable;

public:

	virtual ~PointingRow();

	/**
	 * Return the table to which this row belongs.
	 */
	PointingTable &getTable() const;
	
	
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
  		
	
	
	


	
	// ===> Attribute numSample
	
	
	

	
 	/**
 	 * Get numSample.
 	 * @return numSample as int
 	 */
 	int getNumSample() const;
	
 
 	
 	
 	/**
 	 * Set numSample with the specified int.
 	 * @param numSample The int value to which numSample is to be set.
 	 
 		
 			
 	 */
 	void setNumSample (int numSample);
  		
	
	
	


	
	// ===> Attribute encoder
	
	
	

	
 	/**
 	 * Get encoder.
 	 * @return encoder as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getEncoder() const;
	
 
 	
 	
 	/**
 	 * Set encoder with the specified vector<vector<Angle > >.
 	 * @param encoder The vector<vector<Angle > > value to which encoder is to be set.
 	 
 		
 			
 	 */
 	void setEncoder (vector<vector<Angle > > encoder);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute usePolynomials
	
	
	

	
 	/**
 	 * Get usePolynomials.
 	 * @return usePolynomials as bool
 	 */
 	bool getUsePolynomials() const;
	
 
 	
 	
 	/**
 	 * Set usePolynomials with the specified bool.
 	 * @param usePolynomials The bool value to which usePolynomials is to be set.
 	 
 		
 			
 	 */
 	void setUsePolynomials (bool usePolynomials);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numTerm
	
	
	

	
 	/**
 	 * Get numTerm.
 	 * @return numTerm as int
 	 */
 	int getNumTerm() const;
	
 
 	
 	
 	/**
 	 * Set numTerm with the specified int.
 	 * @param numTerm The int value to which numTerm is to be set.
 	 
 		
 			
 	 */
 	void setNumTerm (int numTerm);
  		
	
	
	


	
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
 	bool getOverTheTop() const;
	
 
 	
 	
 	/**
 	 * Set overTheTop with the specified bool.
 	 * @param overTheTop The bool value to which overTheTop is to be set.
 	 
 		
 	 */
 	void setOverTheTop (bool overTheTop);
		
	
	
	
	/**
	 * Mark overTheTop, which is an optional field, as non-existent.
	 */
	void clearOverTheTop ();
	


	
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
 	vector<vector<Angle > > getSourceOffset() const;
	
 
 	
 	
 	/**
 	 * Set sourceOffset with the specified vector<vector<Angle > >.
 	 * @param sourceOffset The vector<vector<Angle > > value to which sourceOffset is to be set.
 	 
 		
 	 */
 	void setSourceOffset (vector<vector<Angle > > sourceOffset);
		
	
	
	
	/**
	 * Mark sourceOffset, which is an optional field, as non-existent.
	 */
	void clearSourceOffset ();
	


	
	// ===> Attribute sourceOffsetReferenceCode, which is optional
	
	
	
	/**
	 * The attribute sourceOffsetReferenceCode is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceOffsetReferenceCode attribute exists. 
	 */
	bool isSourceOffsetReferenceCodeExists() const;
	

	
 	/**
 	 * Get sourceOffsetReferenceCode, which is optional.
 	 * @return sourceOffsetReferenceCode as DirectionReferenceCodeMod::DirectionReferenceCode
 	 * @throws IllegalAccessException If sourceOffsetReferenceCode does not exist.
 	 */
 	DirectionReferenceCodeMod::DirectionReferenceCode getSourceOffsetReferenceCode() const;
	
 
 	
 	
 	/**
 	 * Set sourceOffsetReferenceCode with the specified DirectionReferenceCodeMod::DirectionReferenceCode.
 	 * @param sourceOffsetReferenceCode The DirectionReferenceCodeMod::DirectionReferenceCode value to which sourceOffsetReferenceCode is to be set.
 	 
 		
 	 */
 	void setSourceOffsetReferenceCode (DirectionReferenceCodeMod::DirectionReferenceCode sourceOffsetReferenceCode);
		
	
	
	
	/**
	 * Mark sourceOffsetReferenceCode, which is an optional field, as non-existent.
	 */
	void clearSourceOffsetReferenceCode ();
	


	
	// ===> Attribute sourceOffsetEquinox, which is optional
	
	
	
	/**
	 * The attribute sourceOffsetEquinox is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceOffsetEquinox attribute exists. 
	 */
	bool isSourceOffsetEquinoxExists() const;
	

	
 	/**
 	 * Get sourceOffsetEquinox, which is optional.
 	 * @return sourceOffsetEquinox as ArrayTime
 	 * @throws IllegalAccessException If sourceOffsetEquinox does not exist.
 	 */
 	ArrayTime getSourceOffsetEquinox() const;
	
 
 	
 	
 	/**
 	 * Set sourceOffsetEquinox with the specified ArrayTime.
 	 * @param sourceOffsetEquinox The ArrayTime value to which sourceOffsetEquinox is to be set.
 	 
 		
 	 */
 	void setSourceOffsetEquinox (ArrayTime sourceOffsetEquinox);
		
	
	
	
	/**
	 * Mark sourceOffsetEquinox, which is an optional field, as non-existent.
	 */
	void clearSourceOffsetEquinox ();
	


	
	// ===> Attribute sampledTimeInterval, which is optional
	
	
	
	/**
	 * The attribute sampledTimeInterval is optional. Return true if this attribute exists.
	 * @return true if and only if the sampledTimeInterval attribute exists. 
	 */
	bool isSampledTimeIntervalExists() const;
	

	
 	/**
 	 * Get sampledTimeInterval, which is optional.
 	 * @return sampledTimeInterval as vector<ArrayTimeInterval >
 	 * @throws IllegalAccessException If sampledTimeInterval does not exist.
 	 */
 	vector<ArrayTimeInterval > getSampledTimeInterval() const;
	
 
 	
 	
 	/**
 	 * Set sampledTimeInterval with the specified vector<ArrayTimeInterval >.
 	 * @param sampledTimeInterval The vector<ArrayTimeInterval > value to which sampledTimeInterval is to be set.
 	 
 		
 	 */
 	void setSampledTimeInterval (vector<ArrayTimeInterval > sampledTimeInterval);
		
	
	
	
	/**
	 * Mark sampledTimeInterval, which is an optional field, as non-existent.
	 */
	void clearSampledTimeInterval ();
	


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
	
	

	
		
	// ===> Slice link from a row of Pointing table to a collection of row of PointingModel table.
	
	/**
	 * Get the collection of row in the PointingModel table having pointingModelId == this.pointingModelId
	 * 
	 * @return a vector of PointingModelRow *
	 */
	vector <PointingModelRow *> getPointingModels();
	
	

	

	

	
		
	/**
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this PointingRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaId
	    
	 * @param timeInterval
	    
	 * @param numSample
	    
	 * @param encoder
	    
	 * @param pointingTracking
	    
	 * @param usePolynomials
	    
	 * @param timeOrigin
	    
	 * @param numTerm
	    
	 * @param pointingDirection
	    
	 * @param target
	    
	 * @param offset
	    
	 * @param pointingModelId
	    
	 */ 
	bool compareNoAutoInc(Tag antennaId, ArrayTimeInterval timeInterval, int numSample, vector<vector<Angle > > encoder, bool pointingTracking, bool usePolynomials, ArrayTime timeOrigin, int numTerm, vector<vector<Angle > > pointingDirection, vector<vector<Angle > > target, vector<vector<Angle > > offset, int pointingModelId);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param numSample
	    
	 * @param encoder
	    
	 * @param pointingTracking
	    
	 * @param usePolynomials
	    
	 * @param timeOrigin
	    
	 * @param numTerm
	    
	 * @param pointingDirection
	    
	 * @param target
	    
	 * @param offset
	    
	 * @param pointingModelId
	    
	 */ 
	bool compareRequiredValue(int numSample, vector<vector<Angle > > encoder, bool pointingTracking, bool usePolynomials, ArrayTime timeOrigin, int numTerm, vector<vector<Angle > > pointingDirection, vector<vector<Angle > > target, vector<vector<Angle > > offset, int pointingModelId); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the PointingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(PointingRow* x) ;
	
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
	 * @throws ConversionException
	 */
	void setFromIDL (PointingRowIDL x) ;
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

	
	
 	

	
	// ===> Attribute numSample
	
	

	int numSample;

	
	
 	

	
	// ===> Attribute encoder
	
	

	vector<vector<Angle > > encoder;

	
	
 	

	
	// ===> Attribute pointingTracking
	
	

	bool pointingTracking;

	
	
 	

	
	// ===> Attribute usePolynomials
	
	

	bool usePolynomials;

	
	
 	

	
	// ===> Attribute timeOrigin
	
	

	ArrayTime timeOrigin;

	
	
 	

	
	// ===> Attribute numTerm
	
	

	int numTerm;

	
	
 	

	
	// ===> Attribute pointingDirection
	
	

	vector<vector<Angle > > pointingDirection;

	
	
 	

	
	// ===> Attribute target
	
	

	vector<vector<Angle > > target;

	
	
 	

	
	// ===> Attribute offset
	
	

	vector<vector<Angle > > offset;

	
	
 	

	
	// ===> Attribute overTheTop, which is optional
	
	
	bool overTheTopExists;
	

	bool overTheTop;

	
	
 	

	
	// ===> Attribute sourceOffset, which is optional
	
	
	bool sourceOffsetExists;
	

	vector<vector<Angle > > sourceOffset;

	
	
 	

	
	// ===> Attribute sourceOffsetReferenceCode, which is optional
	
	
	bool sourceOffsetReferenceCodeExists;
	

	DirectionReferenceCodeMod::DirectionReferenceCode sourceOffsetReferenceCode;

	
	
 	

	
	// ===> Attribute sourceOffsetEquinox, which is optional
	
	
	bool sourceOffsetEquinoxExists;
	

	ArrayTime sourceOffsetEquinox;

	
	
 	

	
	// ===> Attribute sampledTimeInterval, which is optional
	
	
	bool sampledTimeIntervalExists;
	

	vector<ArrayTimeInterval > sampledTimeInterval;

	
	
 	

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
	
	
		


	

	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, PointingAttributeFromBin> fromBinMethods;
void antennaIdFromBin( EndianISStream& eiss);
void timeIntervalFromBin( EndianISStream& eiss);
void numSampleFromBin( EndianISStream& eiss);
void encoderFromBin( EndianISStream& eiss);
void pointingTrackingFromBin( EndianISStream& eiss);
void usePolynomialsFromBin( EndianISStream& eiss);
void timeOriginFromBin( EndianISStream& eiss);
void numTermFromBin( EndianISStream& eiss);
void pointingDirectionFromBin( EndianISStream& eiss);
void targetFromBin( EndianISStream& eiss);
void offsetFromBin( EndianISStream& eiss);
void pointingModelIdFromBin( EndianISStream& eiss);

void overTheTopFromBin( EndianISStream& eiss);
void sourceOffsetFromBin( EndianISStream& eiss);
void sourceOffsetReferenceCodeFromBin( EndianISStream& eiss);
void sourceOffsetEquinoxFromBin( EndianISStream& eiss);
void sampledTimeIntervalFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the PointingTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static PointingRow* fromBin(EndianISStream& eiss, PointingTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* Pointing_CLASS */
