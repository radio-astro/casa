
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
 * File CalGainRow.h
 */
 
#ifndef CalGainRow_CLASS
#define CalGainRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalGainRowIDL;
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

/*\file CalGain.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::CalGainTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

/**
 * The CalGainRow class is a row of a CalGainTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class CalGainRow {
friend class asdm::CalGainTable;

public:

	virtual ~CalGainRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalGainTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalGainRowIDL struct.
	 */
	CalGainRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalGainRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (CalGainRowIDL x) throw(ConversionException);
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
  		
	
	
	


	
	// ===> Attribute gain
	
	
	

	
 	/**
 	 * Get gain.
 	 * @return gain as vector<vector<float > >
 	 */
 	vector<vector<float > > getGain() const;
	
 
 	
 	
 	/**
 	 * Set gain with the specified vector<vector<float > >.
 	 * @param gain The vector<vector<float > > value to which gain is to be set.
 	 
 		
 			
 	 */
 	void setGain (vector<vector<float > > gain);
  		
	
	
	


	
	// ===> Attribute gainValid
	
	
	

	
 	/**
 	 * Get gainValid.
 	 * @return gainValid as vector<bool >
 	 */
 	vector<bool > getGainValid() const;
	
 
 	
 	
 	/**
 	 * Set gainValid with the specified vector<bool >.
 	 * @param gainValid The vector<bool > value to which gainValid is to be set.
 	 
 		
 			
 	 */
 	void setGainValid (vector<bool > gainValid);
  		
	
	
	


	
	// ===> Attribute fit
	
	
	

	
 	/**
 	 * Get fit.
 	 * @return fit as vector<vector<float > >
 	 */
 	vector<vector<float > > getFit() const;
	
 
 	
 	
 	/**
 	 * Set fit with the specified vector<vector<float > >.
 	 * @param fit The vector<vector<float > > value to which fit is to be set.
 	 
 		
 			
 	 */
 	void setFit (vector<vector<float > > fit);
  		
	
	
	


	
	// ===> Attribute fitWeight
	
	
	

	
 	/**
 	 * Get fitWeight.
 	 * @return fitWeight as vector<float >
 	 */
 	vector<float > getFitWeight() const;
	
 
 	
 	
 	/**
 	 * Set fitWeight with the specified vector<float >.
 	 * @param fitWeight The vector<float > value to which fitWeight is to be set.
 	 
 		
 			
 	 */
 	void setFitWeight (vector<float > fitWeight);
  		
	
	
	


	
	// ===> Attribute totalGainValid
	
	
	

	
 	/**
 	 * Get totalGainValid.
 	 * @return totalGainValid as bool
 	 */
 	bool getTotalGainValid() const;
	
 
 	
 	
 	/**
 	 * Set totalGainValid with the specified bool.
 	 * @param totalGainValid The bool value to which totalGainValid is to be set.
 	 
 		
 			
 	 */
 	void setTotalGainValid (bool totalGainValid);
  		
	
	
	


	
	// ===> Attribute totalFit
	
	
	

	
 	/**
 	 * Get totalFit.
 	 * @return totalFit as float
 	 */
 	float getTotalFit() const;
	
 
 	
 	
 	/**
 	 * Set totalFit with the specified float.
 	 * @param totalFit The float value to which totalFit is to be set.
 	 
 		
 			
 	 */
 	void setTotalFit (float totalFit);
  		
	
	
	


	
	// ===> Attribute totalFitWeight
	
	
	

	
 	/**
 	 * Get totalFitWeight.
 	 * @return totalFitWeight as float
 	 */
 	float getTotalFitWeight() const;
	
 
 	
 	
 	/**
 	 * Set totalFitWeight with the specified float.
 	 * @param totalFitWeight The float value to which totalFitWeight is to be set.
 	 
 		
 			
 	 */
 	void setTotalFitWeight (float totalFitWeight);
  		
	
	
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this CalGainRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<vector<float > > gain, vector<bool > gainValid, vector<vector<float > > fit, vector<float > fitWeight, bool totalGainValid, float totalFit, float totalFitWeight);
	
	

	
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, vector<vector<float > > gain, vector<bool > gainValid, vector<vector<float > > fit, vector<float > fitWeight, bool totalGainValid, float totalFit, float totalFitWeight); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalGainRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalGainRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	CalGainTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalGainRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalGainRow (CalGainTable &table);

	/**
	 * Create a CalGainRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalGainRow row and a CalGainTable table, the method creates a new
	 * CalGainRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalGainRow (CalGainTable &table, CalGainRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute gain
	
	

	vector<vector<float > > gain;

	
	
 	

	
	// ===> Attribute gainValid
	
	

	vector<bool > gainValid;

	
	
 	

	
	// ===> Attribute fit
	
	

	vector<vector<float > > fit;

	
	
 	

	
	// ===> Attribute fitWeight
	
	

	vector<float > fitWeight;

	
	
 	

	
	// ===> Attribute totalGainValid
	
	

	bool totalGainValid;

	
	
 	

	
	// ===> Attribute totalFit
	
	

	float totalFit;

	
	
 	

	
	// ===> Attribute totalFitWeight
	
	

	float totalFitWeight;

	
	
 	

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

#endif /* CalGain_CLASS */
