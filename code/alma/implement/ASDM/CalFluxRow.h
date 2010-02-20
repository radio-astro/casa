
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
 * File CalFluxRow.h
 */
 
#ifndef CalFluxRow_CLASS
#define CalFluxRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalFluxRowIDL;
#endif



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;

#include <Frequency.h>
using  asdm::Frequency;




	

	

	

	

	

	

	
#include "CFluxCalibrationMethod.h"
using namespace FluxCalibrationMethodMod;
	

	

	

	
#include "CStokesParameter.h"
using namespace StokesParameterMod;
	

	

	
#include "CDirectionReferenceCode.h"
using namespace DirectionReferenceCodeMod;
	

	

	

	

	

	

	
#include "CSourceModel.h"
using namespace SourceModelMod;
	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file CalFlux.h
    \brief Generated from model's revision "1.53", branch "HEAD"
*/

namespace asdm {

//class asdm::CalFluxTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

class CalFluxRow;
typedef void (CalFluxRow::*CalFluxAttributeFromBin) (EndianISStream& eiss);

/**
 * The CalFluxRow class is a row of a CalFluxTable.
 * 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 */
class CalFluxRow {
friend class asdm::CalFluxTable;

public:

	virtual ~CalFluxRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalFluxTable &getTable() const;
	
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceName
	
	
	

	
 	/**
 	 * Get sourceName.
 	 * @return sourceName as string
 	 */
 	string getSourceName() const;
	
 
 	
 	
 	/**
 	 * Set sourceName with the specified string.
 	 * @param sourceName The string value to which sourceName is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSourceName (string sourceName);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numFrequencyRanges
	
	
	

	
 	/**
 	 * Get numFrequencyRanges.
 	 * @return numFrequencyRanges as int
 	 */
 	int getNumFrequencyRanges() const;
	
 
 	
 	
 	/**
 	 * Set numFrequencyRanges with the specified int.
 	 * @param numFrequencyRanges The int value to which numFrequencyRanges is to be set.
 	 
 		
 			
 	 */
 	void setNumFrequencyRanges (int numFrequencyRanges);
  		
	
	
	


	
	// ===> Attribute numStokes
	
	
	

	
 	/**
 	 * Get numStokes.
 	 * @return numStokes as int
 	 */
 	int getNumStokes() const;
	
 
 	
 	
 	/**
 	 * Set numStokes with the specified int.
 	 * @param numStokes The int value to which numStokes is to be set.
 	 
 		
 			
 	 */
 	void setNumStokes (int numStokes);
  		
	
	
	


	
	// ===> Attribute frequencyRanges
	
	
	

	
 	/**
 	 * Get frequencyRanges.
 	 * @return frequencyRanges as vector<vector<Frequency > >
 	 */
 	vector<vector<Frequency > > getFrequencyRanges() const;
	
 
 	
 	
 	/**
 	 * Set frequencyRanges with the specified vector<vector<Frequency > >.
 	 * @param frequencyRanges The vector<vector<Frequency > > value to which frequencyRanges is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyRanges (vector<vector<Frequency > > frequencyRanges);
  		
	
	
	


	
	// ===> Attribute fluxMethod
	
	
	

	
 	/**
 	 * Get fluxMethod.
 	 * @return fluxMethod as FluxCalibrationMethodMod::FluxCalibrationMethod
 	 */
 	FluxCalibrationMethodMod::FluxCalibrationMethod getFluxMethod() const;
	
 
 	
 	
 	/**
 	 * Set fluxMethod with the specified FluxCalibrationMethodMod::FluxCalibrationMethod.
 	 * @param fluxMethod The FluxCalibrationMethodMod::FluxCalibrationMethod value to which fluxMethod is to be set.
 	 
 		
 			
 	 */
 	void setFluxMethod (FluxCalibrationMethodMod::FluxCalibrationMethod fluxMethod);
  		
	
	
	


	
	// ===> Attribute flux
	
	
	

	
 	/**
 	 * Get flux.
 	 * @return flux as vector<vector<double > >
 	 */
 	vector<vector<double > > getFlux() const;
	
 
 	
 	
 	/**
 	 * Set flux with the specified vector<vector<double > >.
 	 * @param flux The vector<vector<double > > value to which flux is to be set.
 	 
 		
 			
 	 */
 	void setFlux (vector<vector<double > > flux);
  		
	
	
	


	
	// ===> Attribute fluxError
	
	
	

	
 	/**
 	 * Get fluxError.
 	 * @return fluxError as vector<vector<double > >
 	 */
 	vector<vector<double > > getFluxError() const;
	
 
 	
 	
 	/**
 	 * Set fluxError with the specified vector<vector<double > >.
 	 * @param fluxError The vector<vector<double > > value to which fluxError is to be set.
 	 
 		
 			
 	 */
 	void setFluxError (vector<vector<double > > fluxError);
  		
	
	
	


	
	// ===> Attribute stokes
	
	
	

	
 	/**
 	 * Get stokes.
 	 * @return stokes as vector<StokesParameterMod::StokesParameter >
 	 */
 	vector<StokesParameterMod::StokesParameter > getStokes() const;
	
 
 	
 	
 	/**
 	 * Set stokes with the specified vector<StokesParameterMod::StokesParameter >.
 	 * @param stokes The vector<StokesParameterMod::StokesParameter > value to which stokes is to be set.
 	 
 		
 			
 	 */
 	void setStokes (vector<StokesParameterMod::StokesParameter > stokes);
  		
	
	
	


	
	// ===> Attribute direction, which is optional
	
	
	
	/**
	 * The attribute direction is optional. Return true if this attribute exists.
	 * @return true if and only if the direction attribute exists. 
	 */
	bool isDirectionExists() const;
	

	
 	/**
 	 * Get direction, which is optional.
 	 * @return direction as vector<Angle >
 	 * @throws IllegalAccessException If direction does not exist.
 	 */
 	vector<Angle > getDirection() const;
	
 
 	
 	
 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 		
 	 */
 	void setDirection (vector<Angle > direction);
		
	
	
	
	/**
	 * Mark direction, which is an optional field, as non-existent.
	 */
	void clearDirection ();
	


	
	// ===> Attribute directionCode, which is optional
	
	
	
	/**
	 * The attribute directionCode is optional. Return true if this attribute exists.
	 * @return true if and only if the directionCode attribute exists. 
	 */
	bool isDirectionCodeExists() const;
	

	
 	/**
 	 * Get directionCode, which is optional.
 	 * @return directionCode as DirectionReferenceCodeMod::DirectionReferenceCode
 	 * @throws IllegalAccessException If directionCode does not exist.
 	 */
 	DirectionReferenceCodeMod::DirectionReferenceCode getDirectionCode() const;
	
 
 	
 	
 	/**
 	 * Set directionCode with the specified DirectionReferenceCodeMod::DirectionReferenceCode.
 	 * @param directionCode The DirectionReferenceCodeMod::DirectionReferenceCode value to which directionCode is to be set.
 	 
 		
 	 */
 	void setDirectionCode (DirectionReferenceCodeMod::DirectionReferenceCode directionCode);
		
	
	
	
	/**
	 * Mark directionCode, which is an optional field, as non-existent.
	 */
	void clearDirectionCode ();
	


	
	// ===> Attribute directionEquinox, which is optional
	
	
	
	/**
	 * The attribute directionEquinox is optional. Return true if this attribute exists.
	 * @return true if and only if the directionEquinox attribute exists. 
	 */
	bool isDirectionEquinoxExists() const;
	

	
 	/**
 	 * Get directionEquinox, which is optional.
 	 * @return directionEquinox as Angle
 	 * @throws IllegalAccessException If directionEquinox does not exist.
 	 */
 	Angle getDirectionEquinox() const;
	
 
 	
 	
 	/**
 	 * Set directionEquinox with the specified Angle.
 	 * @param directionEquinox The Angle value to which directionEquinox is to be set.
 	 
 		
 	 */
 	void setDirectionEquinox (Angle directionEquinox);
		
	
	
	
	/**
	 * Mark directionEquinox, which is an optional field, as non-existent.
	 */
	void clearDirectionEquinox ();
	


	
	// ===> Attribute PA, which is optional
	
	
	
	/**
	 * The attribute PA is optional. Return true if this attribute exists.
	 * @return true if and only if the PA attribute exists. 
	 */
	bool isPAExists() const;
	

	
 	/**
 	 * Get PA, which is optional.
 	 * @return PA as vector<vector<Angle > >
 	 * @throws IllegalAccessException If PA does not exist.
 	 */
 	vector<vector<Angle > > getPA() const;
	
 
 	
 	
 	/**
 	 * Set PA with the specified vector<vector<Angle > >.
 	 * @param PA The vector<vector<Angle > > value to which PA is to be set.
 	 
 		
 	 */
 	void setPA (vector<vector<Angle > > PA);
		
	
	
	
	/**
	 * Mark PA, which is an optional field, as non-existent.
	 */
	void clearPA ();
	


	
	// ===> Attribute PAError, which is optional
	
	
	
	/**
	 * The attribute PAError is optional. Return true if this attribute exists.
	 * @return true if and only if the PAError attribute exists. 
	 */
	bool isPAErrorExists() const;
	

	
 	/**
 	 * Get PAError, which is optional.
 	 * @return PAError as vector<vector<Angle > >
 	 * @throws IllegalAccessException If PAError does not exist.
 	 */
 	vector<vector<Angle > > getPAError() const;
	
 
 	
 	
 	/**
 	 * Set PAError with the specified vector<vector<Angle > >.
 	 * @param PAError The vector<vector<Angle > > value to which PAError is to be set.
 	 
 		
 	 */
 	void setPAError (vector<vector<Angle > > PAError);
		
	
	
	
	/**
	 * Mark PAError, which is an optional field, as non-existent.
	 */
	void clearPAError ();
	


	
	// ===> Attribute size, which is optional
	
	
	
	/**
	 * The attribute size is optional. Return true if this attribute exists.
	 * @return true if and only if the size attribute exists. 
	 */
	bool isSizeExists() const;
	

	
 	/**
 	 * Get size, which is optional.
 	 * @return size as vector<vector<vector<Angle > > >
 	 * @throws IllegalAccessException If size does not exist.
 	 */
 	vector<vector<vector<Angle > > > getSize() const;
	
 
 	
 	
 	/**
 	 * Set size with the specified vector<vector<vector<Angle > > >.
 	 * @param size The vector<vector<vector<Angle > > > value to which size is to be set.
 	 
 		
 	 */
 	void setSize (vector<vector<vector<Angle > > > size);
		
	
	
	
	/**
	 * Mark size, which is an optional field, as non-existent.
	 */
	void clearSize ();
	


	
	// ===> Attribute sizeError, which is optional
	
	
	
	/**
	 * The attribute sizeError is optional. Return true if this attribute exists.
	 * @return true if and only if the sizeError attribute exists. 
	 */
	bool isSizeErrorExists() const;
	

	
 	/**
 	 * Get sizeError, which is optional.
 	 * @return sizeError as vector<vector<vector<Angle > > >
 	 * @throws IllegalAccessException If sizeError does not exist.
 	 */
 	vector<vector<vector<Angle > > > getSizeError() const;
	
 
 	
 	
 	/**
 	 * Set sizeError with the specified vector<vector<vector<Angle > > >.
 	 * @param sizeError The vector<vector<vector<Angle > > > value to which sizeError is to be set.
 	 
 		
 	 */
 	void setSizeError (vector<vector<vector<Angle > > > sizeError);
		
	
	
	
	/**
	 * Mark sizeError, which is an optional field, as non-existent.
	 */
	void clearSizeError ();
	


	
	// ===> Attribute sourceModel, which is optional
	
	
	
	/**
	 * The attribute sourceModel is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceModel attribute exists. 
	 */
	bool isSourceModelExists() const;
	

	
 	/**
 	 * Get sourceModel, which is optional.
 	 * @return sourceModel as SourceModelMod::SourceModel
 	 * @throws IllegalAccessException If sourceModel does not exist.
 	 */
 	SourceModelMod::SourceModel getSourceModel() const;
	
 
 	
 	
 	/**
 	 * Set sourceModel with the specified SourceModelMod::SourceModel.
 	 * @param sourceModel The SourceModelMod::SourceModel value to which sourceModel is to be set.
 	 
 		
 	 */
 	void setSourceModel (SourceModelMod::SourceModel sourceModel);
		
	
	
	
	/**
	 * Mark sourceModel, which is an optional field, as non-existent.
	 */
	void clearSourceModel ();
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this CalFluxRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param sourceName
	    
	 * @param calDataId
	    
	 * @param calReductionId
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param numFrequencyRanges
	    
	 * @param numStokes
	    
	 * @param frequencyRanges
	    
	 * @param fluxMethod
	    
	 * @param flux
	    
	 * @param fluxError
	    
	 * @param stokes
	    
	 */ 
	bool compareNoAutoInc(string sourceName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numFrequencyRanges, int numStokes, vector<vector<Frequency > > frequencyRanges, FluxCalibrationMethodMod::FluxCalibrationMethod fluxMethod, vector<vector<double > > flux, vector<vector<double > > fluxError, vector<StokesParameterMod::StokesParameter > stokes);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param numFrequencyRanges
	    
	 * @param numStokes
	    
	 * @param frequencyRanges
	    
	 * @param fluxMethod
	    
	 * @param flux
	    
	 * @param fluxError
	    
	 * @param stokes
	    
	 */ 
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, int numFrequencyRanges, int numStokes, vector<vector<Frequency > > frequencyRanges, FluxCalibrationMethodMod::FluxCalibrationMethod fluxMethod, vector<vector<double > > flux, vector<vector<double > > fluxError, vector<StokesParameterMod::StokesParameter > stokes); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalFluxRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalFluxRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalFluxRowIDL struct.
	 */
	CalFluxRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalFluxRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (CalFluxRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param rowDoc the XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (string rowDoc) ;	

private:
	/**
	 * The table to which this row belongs.
	 */
	CalFluxTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalFluxRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalFluxRow (CalFluxTable &table);

	/**
	 * Create a CalFluxRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalFluxRow row and a CalFluxTable table, the method creates a new
	 * CalFluxRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalFluxRow (CalFluxTable &table, CalFluxRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceName
	
	

	string sourceName;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute numFrequencyRanges
	
	

	int numFrequencyRanges;

	
	
 	

	
	// ===> Attribute numStokes
	
	

	int numStokes;

	
	
 	

	
	// ===> Attribute frequencyRanges
	
	

	vector<vector<Frequency > > frequencyRanges;

	
	
 	

	
	// ===> Attribute fluxMethod
	
	

	FluxCalibrationMethodMod::FluxCalibrationMethod fluxMethod;

	
	
 	

	
	// ===> Attribute flux
	
	

	vector<vector<double > > flux;

	
	
 	

	
	// ===> Attribute fluxError
	
	

	vector<vector<double > > fluxError;

	
	
 	

	
	// ===> Attribute stokes
	
	

	vector<StokesParameterMod::StokesParameter > stokes;

	
	
 	

	
	// ===> Attribute direction, which is optional
	
	
	bool directionExists;
	

	vector<Angle > direction;

	
	
 	

	
	// ===> Attribute directionCode, which is optional
	
	
	bool directionCodeExists;
	

	DirectionReferenceCodeMod::DirectionReferenceCode directionCode;

	
	
 	

	
	// ===> Attribute directionEquinox, which is optional
	
	
	bool directionEquinoxExists;
	

	Angle directionEquinox;

	
	
 	

	
	// ===> Attribute PA, which is optional
	
	
	bool PAExists;
	

	vector<vector<Angle > > PA;

	
	
 	

	
	// ===> Attribute PAError, which is optional
	
	
	bool PAErrorExists;
	

	vector<vector<Angle > > PAError;

	
	
 	

	
	// ===> Attribute size, which is optional
	
	
	bool sizeExists;
	

	vector<vector<vector<Angle > > > size;

	
	
 	

	
	// ===> Attribute sizeError, which is optional
	
	
	bool sizeErrorExists;
	

	vector<vector<vector<Angle > > > sizeError;

	
	
 	

	
	// ===> Attribute sourceModel, which is optional
	
	
	bool sourceModelExists;
	

	SourceModelMod::SourceModel sourceModel;

	
	
 	

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
	
	
		

	 

	

	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, CalFluxAttributeFromBin> fromBinMethods;
void sourceNameFromBin( EndianISStream& eiss);
void calDataIdFromBin( EndianISStream& eiss);
void calReductionIdFromBin( EndianISStream& eiss);
void startValidTimeFromBin( EndianISStream& eiss);
void endValidTimeFromBin( EndianISStream& eiss);
void numFrequencyRangesFromBin( EndianISStream& eiss);
void numStokesFromBin( EndianISStream& eiss);
void frequencyRangesFromBin( EndianISStream& eiss);
void fluxMethodFromBin( EndianISStream& eiss);
void fluxFromBin( EndianISStream& eiss);
void fluxErrorFromBin( EndianISStream& eiss);
void stokesFromBin( EndianISStream& eiss);

void directionFromBin( EndianISStream& eiss);
void directionCodeFromBin( EndianISStream& eiss);
void directionEquinoxFromBin( EndianISStream& eiss);
void PAFromBin( EndianISStream& eiss);
void PAErrorFromBin( EndianISStream& eiss);
void sizeFromBin( EndianISStream& eiss);
void sizeErrorFromBin( EndianISStream& eiss);
void sourceModelFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the CalFluxTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalFluxRow* fromBin(EndianISStream& eiss, CalFluxTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* CalFlux_CLASS */
