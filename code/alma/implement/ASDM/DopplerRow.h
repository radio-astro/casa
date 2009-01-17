
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
 * File DopplerRow.h
 */
 
#ifndef DopplerRow_CLASS
#define DopplerRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::DopplerRowIDL;
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

/*\file Doppler.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::DopplerTable;


// class asdm::SourceRow;
class SourceRow;
	

/**
 * The DopplerRow class is a row of a DopplerTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class DopplerRow {
friend class asdm::DopplerTable;

public:

	virtual ~DopplerRow();

	/**
	 * Return the table to which this row belongs.
	 */
	DopplerTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a DopplerRowIDL struct.
	 */
	DopplerRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct DopplerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (DopplerRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute dopplerId
	
	
	

	
 	/**
 	 * Get dopplerId.
 	 * @return dopplerId as int
 	 */
 	int getDopplerId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute transitionIndex
	
	
	

	
 	/**
 	 * Get transitionIndex.
 	 * @return transitionIndex as int
 	 */
 	int getTransitionIndex() const;
	
 
 	
 	
 	/**
 	 * Set transitionIndex with the specified int.
 	 * @param transitionIndex The int value to which transitionIndex is to be set.
 	 
 		
 			
 	 */
 	void setTransitionIndex (int transitionIndex);
  		
	
	
	


	
	// ===> Attribute velDef
	
	
	

	
 	/**
 	 * Get velDef.
 	 * @return velDef as Speed
 	 */
 	Speed getVelDef() const;
	
 
 	
 	
 	/**
 	 * Set velDef with the specified Speed.
 	 * @param velDef The Speed value to which velDef is to be set.
 	 
 		
 			
 	 */
 	void setVelDef (Speed velDef);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceId
	
	
	

	
 	/**
 	 * Get sourceId.
 	 * @return sourceId as int
 	 */
 	int getSourceId() const;
	
 
 	
 	
 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSourceId (int sourceId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	// ===> Slice link from a row of Doppler table to a collection of row of Source table.
	
	/**
	 * Get the collection of row in the Source table having sourceId == this.sourceId
	 * 
	 * @return a vector of SourceRow *
	 */
	vector <SourceRow *> getSources();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this DopplerRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(int sourceId, int transitionIndex, Speed velDef);
	
	

	
	bool compareRequiredValue(int transitionIndex, Speed velDef); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the DopplerRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(DopplerRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	DopplerTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a DopplerRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	DopplerRow (DopplerTable &table);

	/**
	 * Create a DopplerRow using a copy constructor mechanism.
	 * <p>
	 * Given a DopplerRow row and a DopplerTable table, the method creates a new
	 * DopplerRow owned by table. Each attribute of the created row is a copy (deep)
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
	 DopplerRow (DopplerTable &table, DopplerRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute dopplerId
	
	

	int dopplerId;

	
	
 	
 	/**
 	 * Set dopplerId with the specified int value.
 	 * @param dopplerId The int value to which dopplerId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setDopplerId (int dopplerId);
  		
	

	
	// ===> Attribute transitionIndex
	
	

	int transitionIndex;

	
	
 	

	
	// ===> Attribute velDef
	
	

	Speed velDef;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceId
	
	

	int sourceId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		


	


};

} // End namespace asdm

#endif /* Doppler_CLASS */
