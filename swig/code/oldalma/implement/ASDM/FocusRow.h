
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
 * File FocusRow.h
 */
 
#ifndef FocusRow_CLASS
#define FocusRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::FocusRowIDL;
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

/*\file Focus.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::FocusTable;


// class asdm::FocusModelRow;
class FocusModelRow;

// class asdm::FeedRow;
class FeedRow;

// class asdm::AntennaRow;
class AntennaRow;
	

/**
 * The FocusRow class is a row of a FocusTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class FocusRow {
friend class asdm::FocusTable;

public:

	virtual ~FocusRow();

	/**
	 * Return the table to which this row belongs.
	 */
	FocusTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FocusRowIDL struct.
	 */
	FocusRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FocusRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (FocusRowIDL x) throw(ConversionException);
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
  		
	
	
	


	
	// ===> Attribute xFocusPosition
	
	
	

	
 	/**
 	 * Get xFocusPosition.
 	 * @return xFocusPosition as Length
 	 */
 	Length getXFocusPosition() const;
	
 
 	
 	
 	/**
 	 * Set xFocusPosition with the specified Length.
 	 * @param xFocusPosition The Length value to which xFocusPosition is to be set.
 	 
 		
 			
 	 */
 	void setXFocusPosition (Length xFocusPosition);
  		
	
	
	


	
	// ===> Attribute yFocusPosition
	
	
	

	
 	/**
 	 * Get yFocusPosition.
 	 * @return yFocusPosition as Length
 	 */
 	Length getYFocusPosition() const;
	
 
 	
 	
 	/**
 	 * Set yFocusPosition with the specified Length.
 	 * @param yFocusPosition The Length value to which yFocusPosition is to be set.
 	 
 		
 			
 	 */
 	void setYFocusPosition (Length yFocusPosition);
  		
	
	
	


	
	// ===> Attribute zFocusPosition
	
	
	

	
 	/**
 	 * Get zFocusPosition.
 	 * @return zFocusPosition as Length
 	 */
 	Length getZFocusPosition() const;
	
 
 	
 	
 	/**
 	 * Set zFocusPosition with the specified Length.
 	 * @param zFocusPosition The Length value to which zFocusPosition is to be set.
 	 
 		
 			
 	 */
 	void setZFocusPosition (Length zFocusPosition);
  		
	
	
	


	
	// ===> Attribute focusTracking
	
	
	

	
 	/**
 	 * Get focusTracking.
 	 * @return focusTracking as bool
 	 */
 	bool getFocusTracking() const;
	
 
 	
 	
 	/**
 	 * Set focusTracking with the specified bool.
 	 * @param focusTracking The bool value to which focusTracking is to be set.
 	 
 		
 			
 	 */
 	void setFocusTracking (bool focusTracking);
  		
	
	
	


	
	// ===> Attribute xFocusOffset
	
	
	

	
 	/**
 	 * Get xFocusOffset.
 	 * @return xFocusOffset as Length
 	 */
 	Length getXFocusOffset() const;
	
 
 	
 	
 	/**
 	 * Set xFocusOffset with the specified Length.
 	 * @param xFocusOffset The Length value to which xFocusOffset is to be set.
 	 
 		
 			
 	 */
 	void setXFocusOffset (Length xFocusOffset);
  		
	
	
	


	
	// ===> Attribute yFocusOffset
	
	
	

	
 	/**
 	 * Get yFocusOffset.
 	 * @return yFocusOffset as Length
 	 */
 	Length getYFocusOffset() const;
	
 
 	
 	
 	/**
 	 * Set yFocusOffset with the specified Length.
 	 * @param yFocusOffset The Length value to which yFocusOffset is to be set.
 	 
 		
 			
 	 */
 	void setYFocusOffset (Length yFocusOffset);
  		
	
	
	


	
	// ===> Attribute zFocusOffset
	
	
	

	
 	/**
 	 * Get zFocusOffset.
 	 * @return zFocusOffset as Length
 	 */
 	Length getZFocusOffset() const;
	
 
 	
 	
 	/**
 	 * Set zFocusOffset with the specified Length.
 	 * @param zFocusOffset The Length value to which zFocusOffset is to be set.
 	 
 		
 			
 	 */
 	void setZFocusOffset (Length zFocusOffset);
  		
	
	
	


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
  		
	
	
	


	
	// ===> Attribute focusModelId
	
	
	

	
 	/**
 	 * Get focusModelId.
 	 * @return focusModelId as Tag
 	 */
 	Tag getFocusModelId() const;
	
 
 	
 	
 	/**
 	 * Set focusModelId with the specified Tag.
 	 * @param focusModelId The Tag value to which focusModelId is to be set.
 	 
 		
 			
 	 */
 	void setFocusModelId (Tag focusModelId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * focusModelId pointer to the row in the FocusModel table having FocusModel.focusModelId == focusModelId
	 * @return a FocusModelRow*
	 * 
	 
	 */
	 FocusModelRow* getFocusModelUsingFocusModelId();
	 

	

	

	
		
	// ===> Slice link from a row of Focus table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having feedId == this.feedId
	 * 
	 * @return a vector of FeedRow *
	 */
	vector <FeedRow *> getFeeds();
	
	

	

	

	
		
	/**
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this FocusRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag antennaId, int feedId, ArrayTimeInterval timeInterval, Tag focusModelId, Length xFocusPosition, Length yFocusPosition, Length zFocusPosition, bool focusTracking, Length xFocusOffset, Length yFocusOffset, Length zFocusOffset);
	
	

	
	bool compareRequiredValue(Tag focusModelId, Length xFocusPosition, Length yFocusPosition, Length zFocusPosition, bool focusTracking, Length xFocusOffset, Length yFocusOffset, Length zFocusOffset); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FocusRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(FocusRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	FocusTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a FocusRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FocusRow (FocusTable &table);

	/**
	 * Create a FocusRow using a copy constructor mechanism.
	 * <p>
	 * Given a FocusRow row and a FocusTable table, the method creates a new
	 * FocusRow owned by table. Each attribute of the created row is a copy (deep)
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
	 FocusRow (FocusTable &table, FocusRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute xFocusPosition
	
	

	Length xFocusPosition;

	
	
 	

	
	// ===> Attribute yFocusPosition
	
	

	Length yFocusPosition;

	
	
 	

	
	// ===> Attribute zFocusPosition
	
	

	Length zFocusPosition;

	
	
 	

	
	// ===> Attribute focusTracking
	
	

	bool focusTracking;

	
	
 	

	
	// ===> Attribute xFocusOffset
	
	

	Length xFocusOffset;

	
	
 	

	
	// ===> Attribute yFocusOffset
	
	

	Length yFocusOffset;

	
	
 	

	
	// ===> Attribute zFocusOffset
	
	

	Length zFocusOffset;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	Tag antennaId;

	
	
 	

	
	// ===> Attribute feedId
	
	

	int feedId;

	
	
 	

	
	// ===> Attribute focusModelId
	
	

	Tag focusModelId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		


	

	
		

	 

	


};

} // End namespace asdm

#endif /* Focus_CLASS */
