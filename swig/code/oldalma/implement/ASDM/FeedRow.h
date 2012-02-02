
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
 * File FeedRow.h
 */
 
#ifndef FeedRow_CLASS
#define FeedRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::FeedRowIDL;
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




	

	

	

	

	

	

	

	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	

	

	



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

/*\file Feed.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::FeedTable;


// class asdm::AntennaRow;
class AntennaRow;

// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::ReceiverRow;
class ReceiverRow;

// class asdm::BeamRow;
class BeamRow;
	

/**
 * The FeedRow class is a row of a FeedTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class FeedRow {
friend class asdm::FeedTable;

public:

	virtual ~FeedRow();

	/**
	 * Return the table to which this row belongs.
	 */
	FeedTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FeedRowIDL struct.
	 */
	FeedRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FeedRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (FeedRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute feedId
	
	
	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int getFeedId() const;
	
 
 	
 	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numReceptor
	
	
	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int getNumReceptor() const;
	
 
 	
 	
 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 		
 			
 	 */
 	void setNumReceptor (int numReceptor);
  		
	
	
	


	
	// ===> Attribute feedNum, which is optional
	
	
	
	/**
	 * The attribute feedNum is optional. Return true if this attribute exists.
	 * @return true if and only if the feedNum attribute exists. 
	 */
	bool isFeedNumExists() const;
	

	
 	/**
 	 * Get feedNum, which is optional.
 	 * @return feedNum as int
 	 * @throws IllegalAccessException If feedNum does not exist.
 	 */
 	int getFeedNum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set feedNum with the specified int.
 	 * @param feedNum The int value to which feedNum is to be set.
 	 
 		
 	 */
 	void setFeedNum (int feedNum);
		
	
	
	
	/**
	 * Mark feedNum, which is an optional field, as non-existent.
	 */
	void clearFeedNum ();
	


	
	// ===> Attribute beamOffset
	
	
	

	
 	/**
 	 * Get beamOffset.
 	 * @return beamOffset as vector<vector<double > >
 	 */
 	vector<vector<double > > getBeamOffset() const;
	
 
 	
 	
 	/**
 	 * Set beamOffset with the specified vector<vector<double > >.
 	 * @param beamOffset The vector<vector<double > > value to which beamOffset is to be set.
 	 
 		
 			
 	 */
 	void setBeamOffset (vector<vector<double > > beamOffset);
  		
	
	
	


	
	// ===> Attribute focusReference
	
	
	

	
 	/**
 	 * Get focusReference.
 	 * @return focusReference as vector<vector<Length > >
 	 */
 	vector<vector<Length > > getFocusReference() const;
	
 
 	
 	
 	/**
 	 * Set focusReference with the specified vector<vector<Length > >.
 	 * @param focusReference The vector<vector<Length > > value to which focusReference is to be set.
 	 
 		
 			
 	 */
 	void setFocusReference (vector<vector<Length > > focusReference);
  		
	
	
	


	
	// ===> Attribute illumOffset, which is optional
	
	
	
	/**
	 * The attribute illumOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the illumOffset attribute exists. 
	 */
	bool isIllumOffsetExists() const;
	

	
 	/**
 	 * Get illumOffset, which is optional.
 	 * @return illumOffset as float
 	 * @throws IllegalAccessException If illumOffset does not exist.
 	 */
 	float getIllumOffset() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set illumOffset with the specified float.
 	 * @param illumOffset The float value to which illumOffset is to be set.
 	 
 		
 	 */
 	void setIllumOffset (float illumOffset);
		
	
	
	
	/**
	 * Mark illumOffset, which is an optional field, as non-existent.
	 */
	void clearIllumOffset ();
	


	
	// ===> Attribute illumOffsetPa, which is optional
	
	
	
	/**
	 * The attribute illumOffsetPa is optional. Return true if this attribute exists.
	 * @return true if and only if the illumOffsetPa attribute exists. 
	 */
	bool isIllumOffsetPaExists() const;
	

	
 	/**
 	 * Get illumOffsetPa, which is optional.
 	 * @return illumOffsetPa as float
 	 * @throws IllegalAccessException If illumOffsetPa does not exist.
 	 */
 	float getIllumOffsetPa() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set illumOffsetPa with the specified float.
 	 * @param illumOffsetPa The float value to which illumOffsetPa is to be set.
 	 
 		
 	 */
 	void setIllumOffsetPa (float illumOffsetPa);
		
	
	
	
	/**
	 * Mark illumOffsetPa, which is an optional field, as non-existent.
	 */
	void clearIllumOffsetPa ();
	


	
	// ===> Attribute polarizationTypes
	
	
	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > getPolarizationTypes() const;
	
 
 	
 	
 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 		
 			
 	 */
 	void setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes);
  		
	
	
	


	
	// ===> Attribute polResponse
	
	
	

	
 	/**
 	 * Get polResponse.
 	 * @return polResponse as vector<vector<Complex > >
 	 */
 	vector<vector<Complex > > getPolResponse() const;
	
 
 	
 	
 	/**
 	 * Set polResponse with the specified vector<vector<Complex > >.
 	 * @param polResponse The vector<vector<Complex > > value to which polResponse is to be set.
 	 
 		
 			
 	 */
 	void setPolResponse (vector<vector<Complex > > polResponse);
  		
	
	
	


	
	// ===> Attribute xPosition, which is optional
	
	
	
	/**
	 * The attribute xPosition is optional. Return true if this attribute exists.
	 * @return true if and only if the xPosition attribute exists. 
	 */
	bool isXPositionExists() const;
	

	
 	/**
 	 * Get xPosition, which is optional.
 	 * @return xPosition as Length
 	 * @throws IllegalAccessException If xPosition does not exist.
 	 */
 	Length getXPosition() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set xPosition with the specified Length.
 	 * @param xPosition The Length value to which xPosition is to be set.
 	 
 		
 	 */
 	void setXPosition (Length xPosition);
		
	
	
	
	/**
	 * Mark xPosition, which is an optional field, as non-existent.
	 */
	void clearXPosition ();
	


	
	// ===> Attribute yPosition, which is optional
	
	
	
	/**
	 * The attribute yPosition is optional. Return true if this attribute exists.
	 * @return true if and only if the yPosition attribute exists. 
	 */
	bool isYPositionExists() const;
	

	
 	/**
 	 * Get yPosition, which is optional.
 	 * @return yPosition as Length
 	 * @throws IllegalAccessException If yPosition does not exist.
 	 */
 	Length getYPosition() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set yPosition with the specified Length.
 	 * @param yPosition The Length value to which yPosition is to be set.
 	 
 		
 	 */
 	void setYPosition (Length yPosition);
		
	
	
	
	/**
	 * Mark yPosition, which is an optional field, as non-existent.
	 */
	void clearYPosition ();
	


	
	// ===> Attribute zPosition, which is optional
	
	
	
	/**
	 * The attribute zPosition is optional. Return true if this attribute exists.
	 * @return true if and only if the zPosition attribute exists. 
	 */
	bool isZPositionExists() const;
	

	
 	/**
 	 * Get zPosition, which is optional.
 	 * @return zPosition as Length
 	 * @throws IllegalAccessException If zPosition does not exist.
 	 */
 	Length getZPosition() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set zPosition with the specified Length.
 	 * @param zPosition The Length value to which zPosition is to be set.
 	 
 		
 	 */
 	void setZPosition (Length zPosition);
		
	
	
	
	/**
	 * Mark zPosition, which is an optional field, as non-existent.
	 */
	void clearZPosition ();
	


	
	// ===> Attribute receptorAngle
	
	
	

	
 	/**
 	 * Get receptorAngle.
 	 * @return receptorAngle as vector<Angle >
 	 */
 	vector<Angle > getReceptorAngle() const;
	
 
 	
 	
 	/**
 	 * Set receptorAngle with the specified vector<Angle >.
 	 * @param receptorAngle The vector<Angle > value to which receptorAngle is to be set.
 	 
 		
 			
 	 */
 	void setReceptorAngle (vector<Angle > receptorAngle);
  		
	
	
	


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
  		
	
	
	


	
	// ===> Attribute beamId, which is optional
	
	
	
	/**
	 * The attribute beamId is optional. Return true if this attribute exists.
	 * @return true if and only if the beamId attribute exists. 
	 */
	bool isBeamIdExists() const;
	

	
 	/**
 	 * Get beamId, which is optional.
 	 * @return beamId as vector<Tag> 
 	 * @throws IllegalAccessException If beamId does not exist.
 	 */
 	vector<Tag>  getBeamId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set beamId with the specified vector<Tag> .
 	 * @param beamId The vector<Tag>  value to which beamId is to be set.
 	 
 		
 	 */
 	void setBeamId (vector<Tag>  beamId);
		
	
	
	
	/**
	 * Mark beamId, which is an optional field, as non-existent.
	 */
	void clearBeamId ();
	


	
	// ===> Attribute receiverId
	
	
	

	
 	/**
 	 * Get receiverId.
 	 * @return receiverId as vector<int> 
 	 */
 	vector<int>  getReceiverId() const;
	
 
 	
 	
 	/**
 	 * Set receiverId with the specified vector<int> .
 	 * @param receiverId The vector<int>  value to which receiverId is to be set.
 	 
 		
 			
 	 */
 	void setReceiverId (vector<int>  receiverId);
  		
	
	
	


	
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
	 

	

	
 		
 	/**
 	 * Set receiverId[i] with the specified int.
 	 * @param i The index in receiverId where to set the int value.
 	 * @param receiverId The int value to which receiverId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void setReceiverId (int i, int receiverId); 
 			
	

	
		

	// ===> Slices link from a row of Feed table to a collection of row of Receiver table.	

	/**
	 * Append a new id to receiverId
	 * @param id the int value to be appended to receiverId
	 */
	void addReceiverId(int id); 
	
	/**
	 * Append an array of ids to receiverId
	 * @param id a vector of int containing the values to append to receiverId.
	 */ 
	void addReceiverId(vector<int> id); 


	/**
	 * Get the collection of rows in the Receiver table having receiverId == receiverId[i]
	 * @return a vector of ReceiverRow *. 
	 */	 
	const vector <ReceiverRow *> getReceivers(int i);


	/** 
	 * Get the collection of rows in the Receiver table having receiverId == receiverId[i]
	 * for any i in [O..receiverId.size()-1].
	 * @return a vector of ReceiverRow *.
	 */
	const vector <ReceiverRow *> getReceivers();
	


	

	
 		
 	/**
 	 * Set beamId[i] with the specified Tag.
 	 * @param i The index in beamId where to set the Tag value.
 	 * @param beamId The Tag value to which beamId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void setBeamId (int i, Tag beamId)  ;
 			
	

	
		 
/**
 * Append a Tag to beamId.
 * @param id the Tag to be appended to beamId
 */
 void addBeamId(Tag id); 

/**
 * Append a vector of Tag to beamId.
 * @param id an array of Tag to be appended to beamId
 */
 void addBeamId(const vector<Tag> & id); 
 

 /**
  * Returns the Tag stored in beamId at position i.
  * @param i the position in beamId where the Tag is retrieved.
  * @return the Tag stored at position i in beamId.
  */
 const Tag getBeamId(int i);
 
 /**
  * Returns the BeamRow linked to this row via the tag stored in beamId
  * at position i.
  * @param i the position in beamId.
  * @return a pointer on a BeamRow whose key (a Tag) is equal to the Tag stored at position
  * i in the beamId. 
  */
 BeamRow* getBeam(int i); 
 
 /**
  * Returns the vector of BeamRow* linked to this row via the Tags stored in beamId
  * @return an array of pointers on BeamRow.
  */
 vector<BeamRow *> getBeams(); 
  

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this FeedRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, vector<int>  receiverId, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle);
	
	

	
	bool compareRequiredValue(vector<int>  receiverId, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FeedRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(FeedRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	FeedTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a FeedRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FeedRow (FeedTable &table);

	/**
	 * Create a FeedRow using a copy constructor mechanism.
	 * <p>
	 * Given a FeedRow row and a FeedTable table, the method creates a new
	 * FeedRow owned by table. Each attribute of the created row is a copy (deep)
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
	 FeedRow (FeedTable &table, FeedRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute feedId
	
	

	int feedId;

	
	
 	
 	/**
 	 * Set feedId with the specified int value.
 	 * @param feedId The int value to which feedId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setFeedId (int feedId);
  		
	

	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute feedNum, which is optional
	
	
	bool feedNumExists;
	

	int feedNum;

	
	
 	

	
	// ===> Attribute beamOffset
	
	

	vector<vector<double > > beamOffset;

	
	
 	

	
	// ===> Attribute focusReference
	
	

	vector<vector<Length > > focusReference;

	
	
 	

	
	// ===> Attribute illumOffset, which is optional
	
	
	bool illumOffsetExists;
	

	float illumOffset;

	
	
 	

	
	// ===> Attribute illumOffsetPa, which is optional
	
	
	bool illumOffsetPaExists;
	

	float illumOffsetPa;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute polResponse
	
	

	vector<vector<Complex > > polResponse;

	
	
 	

	
	// ===> Attribute xPosition, which is optional
	
	
	bool xPositionExists;
	

	Length xPosition;

	
	
 	

	
	// ===> Attribute yPosition, which is optional
	
	
	bool yPositionExists;
	

	Length yPosition;

	
	
 	

	
	// ===> Attribute zPosition, which is optional
	
	
	bool zPositionExists;
	

	Length zPosition;

	
	
 	

	
	// ===> Attribute receptorAngle
	
	

	vector<Angle > receptorAngle;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	Tag antennaId;

	
	
 	

	
	// ===> Attribute beamId, which is optional
	
	
	bool beamIdExists;
	

	vector<Tag>  beamId;

	
	
 	

	
	// ===> Attribute receiverId
	
	

	vector<int>  receiverId;

	
	
 	

	
	// ===> Attribute spectralWindowId
	
	

	Tag spectralWindowId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	

	
		
	

	

	
		


	


};

} // End namespace asdm

#endif /* Feed_CLASS */
