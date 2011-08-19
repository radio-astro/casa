
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
 * File StateRow.h
 */
 
#ifndef StateRow_CLASS
#define StateRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::StateRowIDL;
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

/*\file State.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::StateTable;

	

/**
 * The StateRow class is a row of a StateTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class StateRow {
friend class asdm::StateTable;

public:

	virtual ~StateRow();

	/**
	 * Return the table to which this row belongs.
	 */
	StateTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a StateRowIDL struct.
	 */
	StateRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct StateRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (StateRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute stateId
	
	
	

	
 	/**
 	 * Get stateId.
 	 * @return stateId as Tag
 	 */
 	Tag getStateId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute sig
	
	
	

	
 	/**
 	 * Get sig.
 	 * @return sig as bool
 	 */
 	bool getSig() const;
	
 
 	
 	
 	/**
 	 * Set sig with the specified bool.
 	 * @param sig The bool value to which sig is to be set.
 	 
 		
 			
 	 */
 	void setSig (bool sig);
  		
	
	
	


	
	// ===> Attribute ref
	
	
	

	
 	/**
 	 * Get ref.
 	 * @return ref as bool
 	 */
 	bool getRef() const;
	
 
 	
 	
 	/**
 	 * Set ref with the specified bool.
 	 * @param ref The bool value to which ref is to be set.
 	 
 		
 			
 	 */
 	void setRef (bool ref);
  		
	
	
	


	
	// ===> Attribute weight, which is optional
	
	
	
	/**
	 * The attribute weight is optional. Return true if this attribute exists.
	 * @return true if and only if the weight attribute exists. 
	 */
	bool isWeightExists() const;
	

	
 	/**
 	 * Get weight, which is optional.
 	 * @return weight as float
 	 * @throws IllegalAccessException If weight does not exist.
 	 */
 	float getWeight() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set weight with the specified float.
 	 * @param weight The float value to which weight is to be set.
 	 
 		
 	 */
 	void setWeight (float weight);
		
	
	
	
	/**
	 * Mark weight, which is an optional field, as non-existent.
	 */
	void clearWeight ();
	


	
	// ===> Attribute onSky
	
	
	

	
 	/**
 	 * Get onSky.
 	 * @return onSky as bool
 	 */
 	bool getOnSky() const;
	
 
 	
 	
 	/**
 	 * Set onSky with the specified bool.
 	 * @param onSky The bool value to which onSky is to be set.
 	 
 		
 			
 	 */
 	void setOnSky (bool onSky);
  		
	
	
	


	
	// ===> Attribute calDeviceName
	
	
	

	
 	/**
 	 * Get calDeviceName.
 	 * @return calDeviceName as CalibrationDeviceMod::CalibrationDevice
 	 */
 	CalibrationDeviceMod::CalibrationDevice getCalDeviceName() const;
	
 
 	
 	
 	/**
 	 * Set calDeviceName with the specified CalibrationDeviceMod::CalibrationDevice.
 	 * @param calDeviceName The CalibrationDeviceMod::CalibrationDevice value to which calDeviceName is to be set.
 	 
 		
 			
 	 */
 	void setCalDeviceName (CalibrationDeviceMod::CalibrationDevice calDeviceName);
  		
	
	
	


	
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
	 * Compare each mandatory attribute except the autoincrementable one of this StateRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(bool sig, bool ref, bool onSky, CalibrationDeviceMod::CalibrationDevice calDeviceName, bool flagRow);
	
	

	
	bool compareRequiredValue(bool sig, bool ref, bool onSky, CalibrationDeviceMod::CalibrationDevice calDeviceName, bool flagRow); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the StateRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(StateRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	StateTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a StateRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	StateRow (StateTable &table);

	/**
	 * Create a StateRow using a copy constructor mechanism.
	 * <p>
	 * Given a StateRow row and a StateTable table, the method creates a new
	 * StateRow owned by table. Each attribute of the created row is a copy (deep)
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
	 StateRow (StateTable &table, StateRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute stateId
	
	

	Tag stateId;

	
	
 	
 	/**
 	 * Set stateId with the specified Tag value.
 	 * @param stateId The Tag value to which stateId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setStateId (Tag stateId);
  		
	

	
	// ===> Attribute sig
	
	

	bool sig;

	
	
 	

	
	// ===> Attribute ref
	
	

	bool ref;

	
	
 	

	
	// ===> Attribute weight, which is optional
	
	
	bool weightExists;
	

	float weight;

	
	
 	

	
	// ===> Attribute onSky
	
	

	bool onSky;

	
	
 	

	
	// ===> Attribute calDeviceName
	
	

	CalibrationDeviceMod::CalibrationDevice calDeviceName;

	
	
 	

	
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

#endif /* State_CLASS */
