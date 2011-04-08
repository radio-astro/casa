
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
 * File HolographyRow.h
 */
 
#ifndef HolographyRow_CLASS
#define HolographyRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::HolographyRowIDL;
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




	

	

	
#include "CHolographyChannelType.h"
using namespace HolographyChannelTypeMod;
	

	

	

	



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

/*\file Holography.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::HolographyTable;

	

/**
 * The HolographyRow class is a row of a HolographyTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class HolographyRow {
friend class asdm::HolographyTable;

public:

	virtual ~HolographyRow();

	/**
	 * Return the table to which this row belongs.
	 */
	HolographyTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a HolographyRowIDL struct.
	 */
	HolographyRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct HolographyRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (HolographyRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute holographyId
	
	
	

	
 	/**
 	 * Get holographyId.
 	 * @return holographyId as Tag
 	 */
 	Tag getHolographyId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute numCorr
	
	
	

	
 	/**
 	 * Get numCorr.
 	 * @return numCorr as int
 	 */
 	int getNumCorr() const;
	
 
 	
 	
 	/**
 	 * Set numCorr with the specified int.
 	 * @param numCorr The int value to which numCorr is to be set.
 	 
 		
 			
 	 */
 	void setNumCorr (int numCorr);
  		
	
	
	


	
	// ===> Attribute type
	
	
	

	
 	/**
 	 * Get type.
 	 * @return type as vector<HolographyChannelTypeMod::HolographyChannelType >
 	 */
 	vector<HolographyChannelTypeMod::HolographyChannelType > getType() const;
	
 
 	
 	
 	/**
 	 * Set type with the specified vector<HolographyChannelTypeMod::HolographyChannelType >.
 	 * @param type The vector<HolographyChannelTypeMod::HolographyChannelType > value to which type is to be set.
 	 
 		
 			
 	 */
 	void setType (vector<HolographyChannelTypeMod::HolographyChannelType > type);
  		
	
	
	


	
	// ===> Attribute distance
	
	
	

	
 	/**
 	 * Get distance.
 	 * @return distance as Length
 	 */
 	Length getDistance() const;
	
 
 	
 	
 	/**
 	 * Set distance with the specified Length.
 	 * @param distance The Length value to which distance is to be set.
 	 
 		
 			
 	 */
 	void setDistance (Length distance);
  		
	
	
	


	
	// ===> Attribute focus
	
	
	

	
 	/**
 	 * Get focus.
 	 * @return focus as Length
 	 */
 	Length getFocus() const;
	
 
 	
 	
 	/**
 	 * Set focus with the specified Length.
 	 * @param focus The Length value to which focus is to be set.
 	 
 		
 			
 	 */
 	void setFocus (Length focus);
  		
	
	
	


	
	// ===> Attribute flagRow
	
	
	

	
 	/**
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool getFlagRow() const;
	
 
 	
 	
 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 		
 			
 	 */
 	void setFlagRow (bool flagRow);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this HolographyRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(int numCorr, vector<HolographyChannelTypeMod::HolographyChannelType > type, Length distance, Length focus, bool flagRow);
	
	

	
	bool compareRequiredValue(int numCorr, vector<HolographyChannelTypeMod::HolographyChannelType > type, Length distance, Length focus, bool flagRow); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the HolographyRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(HolographyRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	HolographyTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a HolographyRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	HolographyRow (HolographyTable &table);

	/**
	 * Create a HolographyRow using a copy constructor mechanism.
	 * <p>
	 * Given a HolographyRow row and a HolographyTable table, the method creates a new
	 * HolographyRow owned by table. Each attribute of the created row is a copy (deep)
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
	 HolographyRow (HolographyTable &table, HolographyRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute holographyId
	
	

	Tag holographyId;

	
	
 	
 	/**
 	 * Set holographyId with the specified Tag value.
 	 * @param holographyId The Tag value to which holographyId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setHolographyId (Tag holographyId);
  		
	

	
	// ===> Attribute numCorr
	
	

	int numCorr;

	
	
 	

	
	// ===> Attribute type
	
	

	vector<HolographyChannelTypeMod::HolographyChannelType > type;

	
	
 	

	
	// ===> Attribute distance
	
	

	Length distance;

	
	
 	

	
	// ===> Attribute focus
	
	

	Length focus;

	
	
 	

	
	// ===> Attribute flagRow
	
	

	bool flagRow;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	

};

} // End namespace asdm

#endif /* Holography_CLASS */
