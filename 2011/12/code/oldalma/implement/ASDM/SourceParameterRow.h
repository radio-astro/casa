
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
 * File SourceParameterRow.h
 */
 
#ifndef SourceParameterRow_CLASS
#define SourceParameterRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::SourceParameterRowIDL;
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




	

	

	

	

	
#include "CStokesParameter.h"
using namespace StokesParameterMod;
	

	

	

	

	

	

	

	

	

	

	



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

/*\file SourceParameter.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::SourceParameterTable;


// class asdm::SourceRow;
class SourceRow;
	

/**
 * The SourceParameterRow class is a row of a SourceParameterTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class SourceParameterRow {
friend class asdm::SourceParameterTable;

public:

	virtual ~SourceParameterRow();

	/**
	 * Return the table to which this row belongs.
	 */
	SourceParameterTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SourceParameterRowIDL struct.
	 */
	SourceParameterRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SourceParameterRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (SourceParameterRowIDL x) throw(ConversionException);
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
  		
	
	
	


	
	// ===> Attribute numFreq
	
	
	

	
 	/**
 	 * Get numFreq.
 	 * @return numFreq as int
 	 */
 	int getNumFreq() const;
	
 
 	
 	
 	/**
 	 * Set numFreq with the specified int.
 	 * @param numFreq The int value to which numFreq is to be set.
 	 
 		
 			
 	 */
 	void setNumFreq (int numFreq);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numDep
	
	
	

	
 	/**
 	 * Get numDep.
 	 * @return numDep as int
 	 */
 	int getNumDep() const;
	
 
 	
 	
 	/**
 	 * Set numDep with the specified int.
 	 * @param numDep The int value to which numDep is to be set.
 	 
 		
 			
 	 */
 	void setNumDep (int numDep);
  		
	
	
	


	
	// ===> Attribute stokesParameter
	
	
	

	
 	/**
 	 * Get stokesParameter.
 	 * @return stokesParameter as vector<StokesParameterMod::StokesParameter >
 	 */
 	vector<StokesParameterMod::StokesParameter > getStokesParameter() const;
	
 
 	
 	
 	/**
 	 * Set stokesParameter with the specified vector<StokesParameterMod::StokesParameter >.
 	 * @param stokesParameter The vector<StokesParameterMod::StokesParameter > value to which stokesParameter is to be set.
 	 
 		
 			
 	 */
 	void setStokesParameter (vector<StokesParameterMod::StokesParameter > stokesParameter);
  		
	
	
	


	
	// ===> Attribute flux
	
	
	

	
 	/**
 	 * Get flux.
 	 * @return flux as vector<vector<Flux > >
 	 */
 	vector<vector<Flux > > getFlux() const;
	
 
 	
 	
 	/**
 	 * Set flux with the specified vector<vector<Flux > >.
 	 * @param flux The vector<vector<Flux > > value to which flux is to be set.
 	 
 		
 			
 	 */
 	void setFlux (vector<vector<Flux > > flux);
  		
	
	
	


	
	// ===> Attribute size, which is optional
	
	
	
	/**
	 * The attribute size is optional. Return true if this attribute exists.
	 * @return true if and only if the size attribute exists. 
	 */
	bool isSizeExists() const;
	

	
 	/**
 	 * Get size, which is optional.
 	 * @return size as vector<vector<Angle > >
 	 * @throws IllegalAccessException If size does not exist.
 	 */
 	vector<vector<Angle > > getSize() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set size with the specified vector<vector<Angle > >.
 	 * @param size The vector<vector<Angle > > value to which size is to be set.
 	 
 		
 	 */
 	void setSize (vector<vector<Angle > > size);
		
	
	
	
	/**
	 * Mark size, which is an optional field, as non-existent.
	 */
	void clearSize ();
	


	
	// ===> Attribute positionAngle, which is optional
	
	
	
	/**
	 * The attribute positionAngle is optional. Return true if this attribute exists.
	 * @return true if and only if the positionAngle attribute exists. 
	 */
	bool isPositionAngleExists() const;
	

	
 	/**
 	 * Get positionAngle, which is optional.
 	 * @return positionAngle as vector<Angle >
 	 * @throws IllegalAccessException If positionAngle does not exist.
 	 */
 	vector<Angle > getPositionAngle() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set positionAngle with the specified vector<Angle >.
 	 * @param positionAngle The vector<Angle > value to which positionAngle is to be set.
 	 
 		
 	 */
 	void setPositionAngle (vector<Angle > positionAngle);
		
	
	
	
	/**
	 * Mark positionAngle, which is an optional field, as non-existent.
	 */
	void clearPositionAngle ();
	


	
	// ===> Attribute sourceParameterId
	
	
	

	
 	/**
 	 * Get sourceParameterId.
 	 * @return sourceParameterId as int
 	 */
 	int getSourceParameterId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute frequency
	
	
	

	
 	/**
 	 * Get frequency.
 	 * @return frequency as vector<Frequency >
 	 */
 	vector<Frequency > getFrequency() const;
	
 
 	
 	
 	/**
 	 * Set frequency with the specified vector<Frequency >.
 	 * @param frequency The vector<Frequency > value to which frequency is to be set.
 	 
 		
 			
 	 */
 	void setFrequency (vector<Frequency > frequency);
  		
	
	
	


	
	// ===> Attribute frequencyInterval
	
	
	

	
 	/**
 	 * Get frequencyInterval.
 	 * @return frequencyInterval as vector<Frequency >
 	 */
 	vector<Frequency > getFrequencyInterval() const;
	
 
 	
 	
 	/**
 	 * Set frequencyInterval with the specified vector<Frequency >.
 	 * @param frequencyInterval The vector<Frequency > value to which frequencyInterval is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyInterval (vector<Frequency > frequencyInterval);
  		
	
	
	


	
	// ===> Attribute fluxErr
	
	
	

	
 	/**
 	 * Get fluxErr.
 	 * @return fluxErr as vector<vector<Flux > >
 	 */
 	vector<vector<Flux > > getFluxErr() const;
	
 
 	
 	
 	/**
 	 * Set fluxErr with the specified vector<vector<Flux > >.
 	 * @param fluxErr The vector<vector<Flux > > value to which fluxErr is to be set.
 	 
 		
 			
 	 */
 	void setFluxErr (vector<vector<Flux > > fluxErr);
  		
	
	
	


	
	// ===> Attribute sizeErr, which is optional
	
	
	
	/**
	 * The attribute sizeErr is optional. Return true if this attribute exists.
	 * @return true if and only if the sizeErr attribute exists. 
	 */
	bool isSizeErrExists() const;
	

	
 	/**
 	 * Get sizeErr, which is optional.
 	 * @return sizeErr as vector<vector<Angle > >
 	 * @throws IllegalAccessException If sizeErr does not exist.
 	 */
 	vector<vector<Angle > > getSizeErr() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sizeErr with the specified vector<vector<Angle > >.
 	 * @param sizeErr The vector<vector<Angle > > value to which sizeErr is to be set.
 	 
 		
 	 */
 	void setSizeErr (vector<vector<Angle > > sizeErr);
		
	
	
	
	/**
	 * Mark sizeErr, which is an optional field, as non-existent.
	 */
	void clearSizeErr ();
	


	
	// ===> Attribute positionAngleErr, which is optional
	
	
	
	/**
	 * The attribute positionAngleErr is optional. Return true if this attribute exists.
	 * @return true if and only if the positionAngleErr attribute exists. 
	 */
	bool isPositionAngleErrExists() const;
	

	
 	/**
 	 * Get positionAngleErr, which is optional.
 	 * @return positionAngleErr as vector<Angle >
 	 * @throws IllegalAccessException If positionAngleErr does not exist.
 	 */
 	vector<Angle > getPositionAngleErr() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set positionAngleErr with the specified vector<Angle >.
 	 * @param positionAngleErr The vector<Angle > value to which positionAngleErr is to be set.
 	 
 		
 	 */
 	void setPositionAngleErr (vector<Angle > positionAngleErr);
		
	
	
	
	/**
	 * Mark positionAngleErr, which is an optional field, as non-existent.
	 */
	void clearPositionAngleErr ();
	


	
	// ===> Attribute depSourceParameterId, which is optional
	
	
	
	/**
	 * The attribute depSourceParameterId is optional. Return true if this attribute exists.
	 * @return true if and only if the depSourceParameterId attribute exists. 
	 */
	bool isDepSourceParameterIdExists() const;
	

	
 	/**
 	 * Get depSourceParameterId, which is optional.
 	 * @return depSourceParameterId as vector<int >
 	 * @throws IllegalAccessException If depSourceParameterId does not exist.
 	 */
 	vector<int > getDepSourceParameterId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set depSourceParameterId with the specified vector<int >.
 	 * @param depSourceParameterId The vector<int > value to which depSourceParameterId is to be set.
 	 
 		
 	 */
 	void setDepSourceParameterId (vector<int > depSourceParameterId);
		
	
	
	
	/**
	 * Mark depSourceParameterId, which is an optional field, as non-existent.
	 */
	void clearDepSourceParameterId ();
	


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
	
	

	
		
	// ===> Slice link from a row of SourceParameter table to a collection of row of Source table.
	
	/**
	 * Get the collection of row in the Source table having sourceId == this.sourceId
	 * 
	 * @return a vector of SourceRow *
	 */
	vector <SourceRow *> getSources();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SourceParameterRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(int sourceId, ArrayTimeInterval timeInterval, int numFreq, int numStokes, int numDep, vector<StokesParameterMod::StokesParameter > stokesParameter, vector<vector<Flux > > flux, vector<Frequency > frequency, vector<Frequency > frequencyInterval, vector<vector<Flux > > fluxErr);
	
	

	
	bool compareRequiredValue(int numFreq, int numStokes, int numDep, vector<StokesParameterMod::StokesParameter > stokesParameter, vector<vector<Flux > > flux, vector<Frequency > frequency, vector<Frequency > frequencyInterval, vector<vector<Flux > > fluxErr); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SourceParameterRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(SourceParameterRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	SourceParameterTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a SourceParameterRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SourceParameterRow (SourceParameterTable &table);

	/**
	 * Create a SourceParameterRow using a copy constructor mechanism.
	 * <p>
	 * Given a SourceParameterRow row and a SourceParameterTable table, the method creates a new
	 * SourceParameterRow owned by table. Each attribute of the created row is a copy (deep)
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
	 SourceParameterRow (SourceParameterTable &table, SourceParameterRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute numFreq
	
	

	int numFreq;

	
	
 	

	
	// ===> Attribute numStokes
	
	

	int numStokes;

	
	
 	

	
	// ===> Attribute numDep
	
	

	int numDep;

	
	
 	

	
	// ===> Attribute stokesParameter
	
	

	vector<StokesParameterMod::StokesParameter > stokesParameter;

	
	
 	

	
	// ===> Attribute flux
	
	

	vector<vector<Flux > > flux;

	
	
 	

	
	// ===> Attribute size, which is optional
	
	
	bool sizeExists;
	

	vector<vector<Angle > > size;

	
	
 	

	
	// ===> Attribute positionAngle, which is optional
	
	
	bool positionAngleExists;
	

	vector<Angle > positionAngle;

	
	
 	

	
	// ===> Attribute sourceParameterId
	
	

	int sourceParameterId;

	
	
 	
 	/**
 	 * Set sourceParameterId with the specified int value.
 	 * @param sourceParameterId The int value to which sourceParameterId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSourceParameterId (int sourceParameterId);
  		
	

	
	// ===> Attribute frequency
	
	

	vector<Frequency > frequency;

	
	
 	

	
	// ===> Attribute frequencyInterval
	
	

	vector<Frequency > frequencyInterval;

	
	
 	

	
	// ===> Attribute fluxErr
	
	

	vector<vector<Flux > > fluxErr;

	
	
 	

	
	// ===> Attribute sizeErr, which is optional
	
	
	bool sizeErrExists;
	

	vector<vector<Angle > > sizeErr;

	
	
 	

	
	// ===> Attribute positionAngleErr, which is optional
	
	
	bool positionAngleErrExists;
	

	vector<Angle > positionAngleErr;

	
	
 	

	
	// ===> Attribute depSourceParameterId, which is optional
	
	
	bool depSourceParameterIdExists;
	

	vector<int > depSourceParameterId;

	
	
 	

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

#endif /* SourceParameter_CLASS */
