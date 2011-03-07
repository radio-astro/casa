
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
 * File ScanRow.h
 */
 
#ifndef ScanRow_CLASS
#define ScanRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::ScanRowIDL;
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




	

	

	

	

	

	

	
#include "CScanIntent.h"
using namespace ScanIntentMod;
	

	

	

	



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

/*\file Scan.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::ScanTable;


// class asdm::ExecBlockRow;
class ExecBlockRow;
	

/**
 * The ScanRow class is a row of a ScanTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class ScanRow {
friend class asdm::ScanTable;

public:

	virtual ~ScanRow();

	/**
	 * Return the table to which this row belongs.
	 */
	ScanTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ScanRowIDL struct.
	 */
	ScanRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ScanRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (ScanRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute scanNumber
	
	
	

	
 	/**
 	 * Get scanNumber.
 	 * @return scanNumber as int
 	 */
 	int getScanNumber() const;
	
 
 	
 	
 	/**
 	 * Set scanNumber with the specified int.
 	 * @param scanNumber The int value to which scanNumber is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setScanNumber (int scanNumber);
  		
	
	
	


	
	// ===> Attribute startTime
	
	
	

	
 	/**
 	 * Get startTime.
 	 * @return startTime as ArrayTime
 	 */
 	ArrayTime getStartTime() const;
	
 
 	
 	
 	/**
 	 * Set startTime with the specified ArrayTime.
 	 * @param startTime The ArrayTime value to which startTime is to be set.
 	 
 		
 			
 	 */
 	void setStartTime (ArrayTime startTime);
  		
	
	
	


	
	// ===> Attribute endTime
	
	
	

	
 	/**
 	 * Get endTime.
 	 * @return endTime as ArrayTime
 	 */
 	ArrayTime getEndTime() const;
	
 
 	
 	
 	/**
 	 * Set endTime with the specified ArrayTime.
 	 * @param endTime The ArrayTime value to which endTime is to be set.
 	 
 		
 			
 	 */
 	void setEndTime (ArrayTime endTime);
  		
	
	
	


	
	// ===> Attribute numSubScan
	
	
	

	
 	/**
 	 * Get numSubScan.
 	 * @return numSubScan as int
 	 */
 	int getNumSubScan() const;
	
 
 	
 	
 	/**
 	 * Set numSubScan with the specified int.
 	 * @param numSubScan The int value to which numSubScan is to be set.
 	 
 		
 			
 	 */
 	void setNumSubScan (int numSubScan);
  		
	
	
	


	
	// ===> Attribute numIntent
	
	
	

	
 	/**
 	 * Get numIntent.
 	 * @return numIntent as int
 	 */
 	int getNumIntent() const;
	
 
 	
 	
 	/**
 	 * Set numIntent with the specified int.
 	 * @param numIntent The int value to which numIntent is to be set.
 	 
 		
 			
 	 */
 	void setNumIntent (int numIntent);
  		
	
	
	


	
	// ===> Attribute numField, which is optional
	
	
	
	/**
	 * The attribute numField is optional. Return true if this attribute exists.
	 * @return true if and only if the numField attribute exists. 
	 */
	bool isNumFieldExists() const;
	

	
 	/**
 	 * Get numField, which is optional.
 	 * @return numField as int
 	 * @throws IllegalAccessException If numField does not exist.
 	 */
 	int getNumField() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set numField with the specified int.
 	 * @param numField The int value to which numField is to be set.
 	 
 		
 	 */
 	void setNumField (int numField);
		
	
	
	
	/**
	 * Mark numField, which is an optional field, as non-existent.
	 */
	void clearNumField ();
	


	
	// ===> Attribute scanIntent
	
	
	

	
 	/**
 	 * Get scanIntent.
 	 * @return scanIntent as vector<ScanIntentMod::ScanIntent >
 	 */
 	vector<ScanIntentMod::ScanIntent > getScanIntent() const;
	
 
 	
 	
 	/**
 	 * Set scanIntent with the specified vector<ScanIntentMod::ScanIntent >.
 	 * @param scanIntent The vector<ScanIntentMod::ScanIntent > value to which scanIntent is to be set.
 	 
 		
 			
 	 */
 	void setScanIntent (vector<ScanIntentMod::ScanIntent > scanIntent);
  		
	
	
	


	
	// ===> Attribute sourceName, which is optional
	
	
	
	/**
	 * The attribute sourceName is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceName attribute exists. 
	 */
	bool isSourceNameExists() const;
	

	
 	/**
 	 * Get sourceName, which is optional.
 	 * @return sourceName as string
 	 * @throws IllegalAccessException If sourceName does not exist.
 	 */
 	string getSourceName() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sourceName with the specified string.
 	 * @param sourceName The string value to which sourceName is to be set.
 	 
 		
 	 */
 	void setSourceName (string sourceName);
		
	
	
	
	/**
	 * Mark sourceName, which is an optional field, as non-existent.
	 */
	void clearSourceName ();
	


	
	// ===> Attribute fieldName, which is optional
	
	
	
	/**
	 * The attribute fieldName is optional. Return true if this attribute exists.
	 * @return true if and only if the fieldName attribute exists. 
	 */
	bool isFieldNameExists() const;
	

	
 	/**
 	 * Get fieldName, which is optional.
 	 * @return fieldName as vector<string >
 	 * @throws IllegalAccessException If fieldName does not exist.
 	 */
 	vector<string > getFieldName() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set fieldName with the specified vector<string >.
 	 * @param fieldName The vector<string > value to which fieldName is to be set.
 	 
 		
 	 */
 	void setFieldName (vector<string > fieldName);
		
	
	
	
	/**
	 * Mark fieldName, which is an optional field, as non-existent.
	 */
	void clearFieldName ();
	


	
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
	
	
	// ===> Attribute execBlockId
	
	
	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag getExecBlockId() const;
	
 
 	
 	
 	/**
 	 * Set execBlockId with the specified Tag.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setExecBlockId (Tag execBlockId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * execBlockId pointer to the row in the ExecBlock table having ExecBlock.execBlockId == execBlockId
	 * @return a ExecBlockRow*
	 * 
	 
	 */
	 ExecBlockRow* getExecBlockUsingExecBlockId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this ScanRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag execBlockId, int scanNumber, ArrayTime startTime, ArrayTime endTime, int numSubScan, int numIntent, vector<ScanIntentMod::ScanIntent > scanIntent, bool flagRow);
	
	

	
	bool compareRequiredValue(ArrayTime startTime, ArrayTime endTime, int numSubScan, int numIntent, vector<ScanIntentMod::ScanIntent > scanIntent, bool flagRow); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ScanRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(ScanRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	ScanTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a ScanRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ScanRow (ScanTable &table);

	/**
	 * Create a ScanRow using a copy constructor mechanism.
	 * <p>
	 * Given a ScanRow row and a ScanTable table, the method creates a new
	 * ScanRow owned by table. Each attribute of the created row is a copy (deep)
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
	 ScanRow (ScanTable &table, ScanRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute scanNumber
	
	

	int scanNumber;

	
	
 	

	
	// ===> Attribute startTime
	
	

	ArrayTime startTime;

	
	
 	

	
	// ===> Attribute endTime
	
	

	ArrayTime endTime;

	
	
 	

	
	// ===> Attribute numSubScan
	
	

	int numSubScan;

	
	
 	

	
	// ===> Attribute numIntent
	
	

	int numIntent;

	
	
 	

	
	// ===> Attribute numField, which is optional
	
	
	bool numFieldExists;
	

	int numField;

	
	
 	

	
	// ===> Attribute scanIntent
	
	

	vector<ScanIntentMod::ScanIntent > scanIntent;

	
	
 	

	
	// ===> Attribute sourceName, which is optional
	
	
	bool sourceNameExists;
	

	string sourceName;

	
	
 	

	
	// ===> Attribute fieldName, which is optional
	
	
	bool fieldNameExists;
	

	vector<string > fieldName;

	
	
 	

	
	// ===> Attribute flagRow
	
	

	bool flagRow;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute execBlockId
	
	

	Tag execBlockId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	


};

} // End namespace asdm

#endif /* Scan_CLASS */
