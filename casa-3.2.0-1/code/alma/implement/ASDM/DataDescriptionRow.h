
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
 * File DataDescriptionRow.h
 */
 
#ifndef DataDescriptionRow_CLASS
#define DataDescriptionRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::DataDescriptionRowIDL;
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


namespace asdm {

//class asdm::DataDescriptionTable;


// class asdm::PolarizationRow;
class PolarizationRow;

// class asdm::HolographyRow;
class HolographyRow;

// class asdm::SpectralWindowRow;
class SpectralWindowRow;
	

/**
 * The DataDescriptionRow class is a row of a DataDescriptionTable.
 */
class DataDescriptionRow {
friend class asdm::DataDescriptionTable;

public:

	virtual ~DataDescriptionRow();

	/**
	 * Return the table to which this row belongs.
	 */
	DataDescriptionTable &getTable() const;
	
	/**
	 * Has this row been added to its table?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;

#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a DataDescriptionRowIDL struct.
	 */
	DataDescriptionRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct DataDescriptionRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (DataDescriptionRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute dataDescriptionId
	
	
	

	
 	/**
 	 * Get dataDescriptionId.
 	 * @return dataDescriptionId as Tag
 	 */
 	Tag getDataDescriptionId() const;
	
 

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute polOrHoloId
	
	
	

	
 	/**
 	 * Get polOrHoloId.
 	 * @return polOrHoloId as Tag
 	 */
 	Tag getPolOrHoloId() const;
	
 
 	
 	
 	/**
 	 * Set polOrHoloId with the specified Tag.
 	 * @param polOrHoloId The Tag value to which polOrHoloId is to be set.
 		
 	 */
 	void setPolOrHoloId (Tag polOrHoloId);
		
	
	
	


	
	// ===> Attribute spectralWindowId
	
	
	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag getSpectralWindowId() const;
	
 
 	
 	
 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 		
 	 */
 	void setSpectralWindowId (Tag spectralWindowId);
		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * Returns pointer to the row in the Polarization table having Polarization.polarizationId == polOrHoloId
	 * @return a PolarizationRow* or 0 if polOrHoloId is an Holography Tag.
	 * 	 
	 */
	 PolarizationRow* getPolarizationUsingPolOrHoloId();
	 
	 /**
	 * Returns pointer to the row in the Holograpy table having Holography.holographyId == polOrHoloId
	 * @return a HolographyRow* or 0 if polOrHoloId is an Polarization Tag.
	 * 	 
	 */
	 HolographyRow* getHolographyUsingPolOrHoloId();
	 
	/**
	 * Returns a pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 */
	 SpectralWindowRow* getSpectralWindowUsingSpectralWindowId();
	 
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this DataDescriptionRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag polOrHoloId, Tag spectralWindowId);
	

	bool compareRequiredValue(Tag polOrHoloId, Tag spectralWindowId); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the DataDescriptionRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(DataDescriptionRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	DataDescriptionTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a DataDescriptionRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	DataDescriptionRow (DataDescriptionTable &table);

	/**
	 * Create a DataDescriptionRow using a copy constructor mechanism.
	 * <p>
	 * Given a DataDescriptionRow row and a DataDescriptionTable table, the method creates a new
	 * DataDescriptionRow owned by table. Each attribute of the created row is a copy (deep)
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
	 DataDescriptionRow (DataDescriptionTable &table, DataDescriptionRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute dataDescriptionId
	
	
	Tag dataDescriptionId;
	
	
 	
 	/**
 	 * Set dataDescriptionId with the specified Tag value.
 	 * @param dataDescriptionId The Tag value to which dataDescriptionId is to be set.
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 */
 	void setDataDescriptionId (Tag dataDescriptionId) throw(IllegalAccessException);
  		
		
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute polOrHoloId
	
	
	Tag polOrHoloId;
	
	
 	

	
	// ===> Attribute spectralWindowId
	
	
	Tag spectralWindowId;
	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	


};

} // End namespace asdm

#endif /* DataDescription_CLASS */
