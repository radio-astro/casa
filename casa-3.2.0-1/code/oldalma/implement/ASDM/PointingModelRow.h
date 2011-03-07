
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
 * File PointingModelRow.h
 */
 
#ifndef PointingModelRow_CLASS
#define PointingModelRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::PointingModelRowIDL;
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

/*\file PointingModel.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::PointingModelTable;


// class asdm::AntennaRow;
class AntennaRow;
	

/**
 * The PointingModelRow class is a row of a PointingModelTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class PointingModelRow {
friend class asdm::PointingModelTable;

public:

	virtual ~PointingModelRow();

	/**
	 * Return the table to which this row belongs.
	 */
	PointingModelTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a PointingModelRowIDL struct.
	 */
	PointingModelRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct PointingModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (PointingModelRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute pointingModelId
	
	
	

	
 	/**
 	 * Get pointingModelId.
 	 * @return pointingModelId as int
 	 */
 	int getPointingModelId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute numCoeff
	
	
	

	
 	/**
 	 * Get numCoeff.
 	 * @return numCoeff as int
 	 */
 	int getNumCoeff() const;
	
 
 	
 	
 	/**
 	 * Set numCoeff with the specified int.
 	 * @param numCoeff The int value to which numCoeff is to be set.
 	 
 		
 			
 	 */
 	void setNumCoeff (int numCoeff);
  		
	
	
	


	
	// ===> Attribute coeffName
	
	
	

	
 	/**
 	 * Get coeffName.
 	 * @return coeffName as vector<string >
 	 */
 	vector<string > getCoeffName() const;
	
 
 	
 	
 	/**
 	 * Set coeffName with the specified vector<string >.
 	 * @param coeffName The vector<string > value to which coeffName is to be set.
 	 
 		
 			
 	 */
 	void setCoeffName (vector<string > coeffName);
  		
	
	
	


	
	// ===> Attribute coeffVal
	
	
	

	
 	/**
 	 * Get coeffVal.
 	 * @return coeffVal as vector<float >
 	 */
 	vector<float > getCoeffVal() const;
	
 
 	
 	
 	/**
 	 * Set coeffVal with the specified vector<float >.
 	 * @param coeffVal The vector<float > value to which coeffVal is to be set.
 	 
 		
 			
 	 */
 	void setCoeffVal (vector<float > coeffVal);
  		
	
	
	


	
	// ===> Attribute numFormula, which is optional
	
	
	
	/**
	 * The attribute numFormula is optional. Return true if this attribute exists.
	 * @return true if and only if the numFormula attribute exists. 
	 */
	bool isNumFormulaExists() const;
	

	
 	/**
 	 * Get numFormula, which is optional.
 	 * @return numFormula as int
 	 * @throws IllegalAccessException If numFormula does not exist.
 	 */
 	int getNumFormula() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set numFormula with the specified int.
 	 * @param numFormula The int value to which numFormula is to be set.
 	 
 		
 	 */
 	void setNumFormula (int numFormula);
		
	
	
	
	/**
	 * Mark numFormula, which is an optional field, as non-existent.
	 */
	void clearNumFormula ();
	


	
	// ===> Attribute coeffFormula, which is optional
	
	
	
	/**
	 * The attribute coeffFormula is optional. Return true if this attribute exists.
	 * @return true if and only if the coeffFormula attribute exists. 
	 */
	bool isCoeffFormulaExists() const;
	

	
 	/**
 	 * Get coeffFormula, which is optional.
 	 * @return coeffFormula as vector<string >
 	 * @throws IllegalAccessException If coeffFormula does not exist.
 	 */
 	vector<string > getCoeffFormula() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set coeffFormula with the specified vector<string >.
 	 * @param coeffFormula The vector<string > value to which coeffFormula is to be set.
 	 
 		
 	 */
 	void setCoeffFormula (vector<string > coeffFormula);
		
	
	
	
	/**
	 * Mark coeffFormula, which is an optional field, as non-existent.
	 */
	void clearCoeffFormula ();
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this PointingModelRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag antennaId, int numCoeff, vector<string > coeffName, vector<float > coeffVal);
	
	

	
	bool compareRequiredValue(int numCoeff, vector<string > coeffName, vector<float > coeffVal); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the PointingModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(PointingModelRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	PointingModelTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a PointingModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	PointingModelRow (PointingModelTable &table);

	/**
	 * Create a PointingModelRow using a copy constructor mechanism.
	 * <p>
	 * Given a PointingModelRow row and a PointingModelTable table, the method creates a new
	 * PointingModelRow owned by table. Each attribute of the created row is a copy (deep)
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
	 PointingModelRow (PointingModelTable &table, PointingModelRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute pointingModelId
	
	

	int pointingModelId;

	
	
 	
 	/**
 	 * Set pointingModelId with the specified int value.
 	 * @param pointingModelId The int value to which pointingModelId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setPointingModelId (int pointingModelId);
  		
	

	
	// ===> Attribute numCoeff
	
	

	int numCoeff;

	
	
 	

	
	// ===> Attribute coeffName
	
	

	vector<string > coeffName;

	
	
 	

	
	// ===> Attribute coeffVal
	
	

	vector<float > coeffVal;

	
	
 	

	
	// ===> Attribute numFormula, which is optional
	
	
	bool numFormulaExists;
	

	int numFormula;

	
	
 	

	
	// ===> Attribute coeffFormula, which is optional
	
	
	bool coeffFormulaExists;
	

	vector<string > coeffFormula;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	Tag antennaId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	


};

} // End namespace asdm

#endif /* PointingModel_CLASS */
