
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
 * File WVMCalRow.h
 */
 
#ifndef WVMCalRow_CLASS
#define WVMCalRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::WVMCalRowIDL;
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

/*\file WVMCal.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::WVMCalTable;


// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::AntennaRow;
class AntennaRow;
	

/**
 * The WVMCalRow class is a row of a WVMCalTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class WVMCalRow {
friend class asdm::WVMCalTable;

public:

	virtual ~WVMCalRow();

	/**
	 * Return the table to which this row belongs.
	 */
	WVMCalTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a WVMCalRowIDL struct.
	 */
	WVMCalRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct WVMCalRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (WVMCalRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute operationMode, which is optional
	
	
	
	/**
	 * The attribute operationMode is optional. Return true if this attribute exists.
	 * @return true if and only if the operationMode attribute exists. 
	 */
	bool isOperationModeExists() const;
	

	
 	/**
 	 * Get operationMode, which is optional.
 	 * @return operationMode as string
 	 * @throws IllegalAccessException If operationMode does not exist.
 	 */
 	string getOperationMode() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set operationMode with the specified string.
 	 * @param operationMode The string value to which operationMode is to be set.
 	 
 		
 	 */
 	void setOperationMode (string operationMode);
		
	
	
	
	/**
	 * Mark operationMode, which is an optional field, as non-existent.
	 */
	void clearOperationMode ();
	


	
	// ===> Attribute numPoly
	
	
	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int getNumPoly() const;
	
 
 	
 	
 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 		
 			
 	 */
 	void setNumPoly (int numPoly);
  		
	
	
	


	
	// ===> Attribute freqOrigin
	
	
	

	
 	/**
 	 * Get freqOrigin.
 	 * @return freqOrigin as Frequency
 	 */
 	Frequency getFreqOrigin() const;
	
 
 	
 	
 	/**
 	 * Set freqOrigin with the specified Frequency.
 	 * @param freqOrigin The Frequency value to which freqOrigin is to be set.
 	 
 		
 			
 	 */
 	void setFreqOrigin (Frequency freqOrigin);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute pathCoeff
	
	
	

	
 	/**
 	 * Get pathCoeff.
 	 * @return pathCoeff as vector<double >
 	 */
 	vector<double > getPathCoeff() const;
	
 
 	
 	
 	/**
 	 * Set pathCoeff with the specified vector<double >.
 	 * @param pathCoeff The vector<double > value to which pathCoeff is to be set.
 	 
 		
 			
 	 */
 	void setPathCoeff (vector<double > pathCoeff);
  		
	
	
	


	
	// ===> Attribute calibrationMode
	
	
	

	
 	/**
 	 * Get calibrationMode.
 	 * @return calibrationMode as string
 	 */
 	string getCalibrationMode() const;
	
 
 	
 	
 	/**
 	 * Set calibrationMode with the specified string.
 	 * @param calibrationMode The string value to which calibrationMode is to be set.
 	 
 		
 			
 	 */
 	void setCalibrationMode (string calibrationMode);
  		
	
	
	


	
	// ===> Attribute wvrefModel, which is optional
	
	
	
	/**
	 * The attribute wvrefModel is optional. Return true if this attribute exists.
	 * @return true if and only if the wvrefModel attribute exists. 
	 */
	bool isWvrefModelExists() const;
	

	
 	/**
 	 * Get wvrefModel, which is optional.
 	 * @return wvrefModel as float
 	 * @throws IllegalAccessException If wvrefModel does not exist.
 	 */
 	float getWvrefModel() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set wvrefModel with the specified float.
 	 * @param wvrefModel The float value to which wvrefModel is to be set.
 	 
 		
 	 */
 	void setWvrefModel (float wvrefModel);
		
	
	
	
	/**
	 * Mark wvrefModel, which is an optional field, as non-existent.
	 */
	void clearWvrefModel ();
	


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
	 * spectralWindowId pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* getSpectralWindowUsingSpectralWindowId();
	 

	

	

	
		
	/**
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this WVMCalRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int numPoly, Frequency freqOrigin, vector<double > pathCoeff, string calibrationMode);
	
	

	
	bool compareRequiredValue(int numPoly, Frequency freqOrigin, vector<double > pathCoeff, string calibrationMode); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the WVMCalRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(WVMCalRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	WVMCalTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a WVMCalRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	WVMCalRow (WVMCalTable &table);

	/**
	 * Create a WVMCalRow using a copy constructor mechanism.
	 * <p>
	 * Given a WVMCalRow row and a WVMCalTable table, the method creates a new
	 * WVMCalRow owned by table. Each attribute of the created row is a copy (deep)
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
	 WVMCalRow (WVMCalTable &table, WVMCalRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute operationMode, which is optional
	
	
	bool operationModeExists;
	

	string operationMode;

	
	
 	

	
	// ===> Attribute numPoly
	
	

	int numPoly;

	
	
 	

	
	// ===> Attribute freqOrigin
	
	

	Frequency freqOrigin;

	
	
 	

	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute pathCoeff
	
	

	vector<double > pathCoeff;

	
	
 	

	
	// ===> Attribute calibrationMode
	
	

	string calibrationMode;

	
	
 	

	
	// ===> Attribute wvrefModel, which is optional
	
	
	bool wvrefModelExists;
	

	float wvrefModel;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	Tag antennaId;

	
	
 	

	
	// ===> Attribute spectralWindowId
	
	

	Tag spectralWindowId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	


};

} // End namespace asdm

#endif /* WVMCal_CLASS */
