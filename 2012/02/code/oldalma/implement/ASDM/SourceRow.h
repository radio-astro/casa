
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
 * File SourceRow.h
 */
 
#ifndef SourceRow_CLASS
#define SourceRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::SourceRowIDL;
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




	

	

	

	

	

	

	

	

	

	

	

	

	

	
#include "CSourceModel.h"
using namespace SourceModelMod;
	

	

	



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

/*\file Source.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::SourceTable;


// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::SourceParameterRow;
class SourceParameterRow;
	

/**
 * The SourceRow class is a row of a SourceTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class SourceRow {
friend class asdm::SourceTable;

public:

	virtual ~SourceRow();

	/**
	 * Return the table to which this row belongs.
	 */
	SourceTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SourceRowIDL struct.
	 */
	SourceRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SourceRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (SourceRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute sourceId
	
	
	

	
 	/**
 	 * Get sourceId.
 	 * @return sourceId as int
 	 */
 	int getSourceId() const;
	
 
 	
 	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numLines
	
	
	

	
 	/**
 	 * Get numLines.
 	 * @return numLines as int
 	 */
 	int getNumLines() const;
	
 
 	
 	
 	/**
 	 * Set numLines with the specified int.
 	 * @param numLines The int value to which numLines is to be set.
 	 
 		
 			
 	 */
 	void setNumLines (int numLines);
  		
	
	
	


	
	// ===> Attribute sourceName
	
	
	

	
 	/**
 	 * Get sourceName.
 	 * @return sourceName as string
 	 */
 	string getSourceName() const;
	
 
 	
 	
 	/**
 	 * Set sourceName with the specified string.
 	 * @param sourceName The string value to which sourceName is to be set.
 	 
 		
 			
 	 */
 	void setSourceName (string sourceName);
  		
	
	
	


	
	// ===> Attribute catalog, which is optional
	
	
	
	/**
	 * The attribute catalog is optional. Return true if this attribute exists.
	 * @return true if and only if the catalog attribute exists. 
	 */
	bool isCatalogExists() const;
	

	
 	/**
 	 * Get catalog, which is optional.
 	 * @return catalog as string
 	 * @throws IllegalAccessException If catalog does not exist.
 	 */
 	string getCatalog() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set catalog with the specified string.
 	 * @param catalog The string value to which catalog is to be set.
 	 
 		
 	 */
 	void setCatalog (string catalog);
		
	
	
	
	/**
	 * Mark catalog, which is an optional field, as non-existent.
	 */
	void clearCatalog ();
	


	
	// ===> Attribute calibrationGroup, which is optional
	
	
	
	/**
	 * The attribute calibrationGroup is optional. Return true if this attribute exists.
	 * @return true if and only if the calibrationGroup attribute exists. 
	 */
	bool isCalibrationGroupExists() const;
	

	
 	/**
 	 * Get calibrationGroup, which is optional.
 	 * @return calibrationGroup as int
 	 * @throws IllegalAccessException If calibrationGroup does not exist.
 	 */
 	int getCalibrationGroup() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set calibrationGroup with the specified int.
 	 * @param calibrationGroup The int value to which calibrationGroup is to be set.
 	 
 		
 	 */
 	void setCalibrationGroup (int calibrationGroup);
		
	
	
	
	/**
	 * Mark calibrationGroup, which is an optional field, as non-existent.
	 */
	void clearCalibrationGroup ();
	


	
	// ===> Attribute code
	
	
	

	
 	/**
 	 * Get code.
 	 * @return code as string
 	 */
 	string getCode() const;
	
 
 	
 	
 	/**
 	 * Set code with the specified string.
 	 * @param code The string value to which code is to be set.
 	 
 		
 			
 	 */
 	void setCode (string code);
  		
	
	
	


	
	// ===> Attribute direction
	
	
	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > getDirection() const;
	
 
 	
 	
 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 		
 			
 	 */
 	void setDirection (vector<Angle > direction);
  		
	
	
	


	
	// ===> Attribute position, which is optional
	
	
	
	/**
	 * The attribute position is optional. Return true if this attribute exists.
	 * @return true if and only if the position attribute exists. 
	 */
	bool isPositionExists() const;
	

	
 	/**
 	 * Get position, which is optional.
 	 * @return position as vector<Length >
 	 * @throws IllegalAccessException If position does not exist.
 	 */
 	vector<Length > getPosition() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set position with the specified vector<Length >.
 	 * @param position The vector<Length > value to which position is to be set.
 	 
 		
 	 */
 	void setPosition (vector<Length > position);
		
	
	
	
	/**
	 * Mark position, which is an optional field, as non-existent.
	 */
	void clearPosition ();
	


	
	// ===> Attribute properMotion
	
	
	

	
 	/**
 	 * Get properMotion.
 	 * @return properMotion as vector<AngularRate >
 	 */
 	vector<AngularRate > getProperMotion() const;
	
 
 	
 	
 	/**
 	 * Set properMotion with the specified vector<AngularRate >.
 	 * @param properMotion The vector<AngularRate > value to which properMotion is to be set.
 	 
 		
 			
 	 */
 	void setProperMotion (vector<AngularRate > properMotion);
  		
	
	
	


	
	// ===> Attribute transition, which is optional
	
	
	
	/**
	 * The attribute transition is optional. Return true if this attribute exists.
	 * @return true if and only if the transition attribute exists. 
	 */
	bool isTransitionExists() const;
	

	
 	/**
 	 * Get transition, which is optional.
 	 * @return transition as vector<string >
 	 * @throws IllegalAccessException If transition does not exist.
 	 */
 	vector<string > getTransition() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set transition with the specified vector<string >.
 	 * @param transition The vector<string > value to which transition is to be set.
 	 
 		
 	 */
 	void setTransition (vector<string > transition);
		
	
	
	
	/**
	 * Mark transition, which is an optional field, as non-existent.
	 */
	void clearTransition ();
	


	
	// ===> Attribute restFrequency, which is optional
	
	
	
	/**
	 * The attribute restFrequency is optional. Return true if this attribute exists.
	 * @return true if and only if the restFrequency attribute exists. 
	 */
	bool isRestFrequencyExists() const;
	

	
 	/**
 	 * Get restFrequency, which is optional.
 	 * @return restFrequency as vector<Frequency >
 	 * @throws IllegalAccessException If restFrequency does not exist.
 	 */
 	vector<Frequency > getRestFrequency() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set restFrequency with the specified vector<Frequency >.
 	 * @param restFrequency The vector<Frequency > value to which restFrequency is to be set.
 	 
 		
 	 */
 	void setRestFrequency (vector<Frequency > restFrequency);
		
	
	
	
	/**
	 * Mark restFrequency, which is an optional field, as non-existent.
	 */
	void clearRestFrequency ();
	


	
	// ===> Attribute sysVel, which is optional
	
	
	
	/**
	 * The attribute sysVel is optional. Return true if this attribute exists.
	 * @return true if and only if the sysVel attribute exists. 
	 */
	bool isSysVelExists() const;
	

	
 	/**
 	 * Get sysVel, which is optional.
 	 * @return sysVel as vector<Speed >
 	 * @throws IllegalAccessException If sysVel does not exist.
 	 */
 	vector<Speed > getSysVel() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sysVel with the specified vector<Speed >.
 	 * @param sysVel The vector<Speed > value to which sysVel is to be set.
 	 
 		
 	 */
 	void setSysVel (vector<Speed > sysVel);
		
	
	
	
	/**
	 * Mark sysVel, which is an optional field, as non-existent.
	 */
	void clearSysVel ();
	


	
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
 	SourceModelMod::SourceModel getSourceModel() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sourceModel with the specified SourceModelMod::SourceModel.
 	 * @param sourceModel The SourceModelMod::SourceModel value to which sourceModel is to be set.
 	 
 		
 	 */
 	void setSourceModel (SourceModelMod::SourceModel sourceModel);
		
	
	
	
	/**
	 * Mark sourceModel, which is an optional field, as non-existent.
	 */
	void clearSourceModel ();
	


	
	// ===> Attribute deltaVel, which is optional
	
	
	
	/**
	 * The attribute deltaVel is optional. Return true if this attribute exists.
	 * @return true if and only if the deltaVel attribute exists. 
	 */
	bool isDeltaVelExists() const;
	

	
 	/**
 	 * Get deltaVel, which is optional.
 	 * @return deltaVel as Speed
 	 * @throws IllegalAccessException If deltaVel does not exist.
 	 */
 	Speed getDeltaVel() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set deltaVel with the specified Speed.
 	 * @param deltaVel The Speed value to which deltaVel is to be set.
 	 
 		
 	 */
 	void setDeltaVel (Speed deltaVel);
		
	
	
	
	/**
	 * Mark deltaVel, which is an optional field, as non-existent.
	 */
	void clearDeltaVel ();
	


	
	// ===> Attribute rangeVel, which is optional
	
	
	
	/**
	 * The attribute rangeVel is optional. Return true if this attribute exists.
	 * @return true if and only if the rangeVel attribute exists. 
	 */
	bool isRangeVelExists() const;
	

	
 	/**
 	 * Get rangeVel, which is optional.
 	 * @return rangeVel as vector<Speed >
 	 * @throws IllegalAccessException If rangeVel does not exist.
 	 */
 	vector<Speed > getRangeVel() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set rangeVel with the specified vector<Speed >.
 	 * @param rangeVel The vector<Speed > value to which rangeVel is to be set.
 	 
 		
 	 */
 	void setRangeVel (vector<Speed > rangeVel);
		
	
	
	
	/**
	 * Mark rangeVel, which is an optional field, as non-existent.
	 */
	void clearRangeVel ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceParameterId, which is optional
	
	
	
	/**
	 * The attribute sourceParameterId is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceParameterId attribute exists. 
	 */
	bool isSourceParameterIdExists() const;
	

	
 	/**
 	 * Get sourceParameterId, which is optional.
 	 * @return sourceParameterId as int
 	 * @throws IllegalAccessException If sourceParameterId does not exist.
 	 */
 	int getSourceParameterId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sourceParameterId with the specified int.
 	 * @param sourceParameterId The int value to which sourceParameterId is to be set.
 	 
 		
 	 */
 	void setSourceParameterId (int sourceParameterId);
		
	
	
	
	/**
	 * Mark sourceParameterId, which is an optional field, as non-existent.
	 */
	void clearSourceParameterId ();
	


	
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
	 

	

	

	
		
	// ===> Slice link from a row of Source table to a collection of row of SourceParameter table.
	
	/**
	 * Get the collection of row in the SourceParameter table having sourceParameterId == this.sourceParameterId
	 * 
	 * @return a vector of SourceParameterRow *
	 */
	vector <SourceParameterRow *> getSourceParameters();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SourceRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag spectralWindowId, ArrayTimeInterval timeInterval, int numLines, string sourceName, string code, vector<Angle > direction, vector<AngularRate > properMotion);
	
	

	
	bool compareRequiredValue(int numLines, string sourceName, string code, vector<Angle > direction, vector<AngularRate > properMotion); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SourceRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(SourceRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	SourceTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a SourceRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SourceRow (SourceTable &table);

	/**
	 * Create a SourceRow using a copy constructor mechanism.
	 * <p>
	 * Given a SourceRow row and a SourceTable table, the method creates a new
	 * SourceRow owned by table. Each attribute of the created row is a copy (deep)
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
	 SourceRow (SourceTable &table, SourceRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceId
	
	

	int sourceId;

	
	
 	
 	/**
 	 * Set sourceId with the specified int value.
 	 * @param sourceId The int value to which sourceId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSourceId (int sourceId);
  		
	

	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute numLines
	
	

	int numLines;

	
	
 	

	
	// ===> Attribute sourceName
	
	

	string sourceName;

	
	
 	

	
	// ===> Attribute catalog, which is optional
	
	
	bool catalogExists;
	

	string catalog;

	
	
 	

	
	// ===> Attribute calibrationGroup, which is optional
	
	
	bool calibrationGroupExists;
	

	int calibrationGroup;

	
	
 	

	
	// ===> Attribute code
	
	

	string code;

	
	
 	

	
	// ===> Attribute direction
	
	

	vector<Angle > direction;

	
	
 	

	
	// ===> Attribute position, which is optional
	
	
	bool positionExists;
	

	vector<Length > position;

	
	
 	

	
	// ===> Attribute properMotion
	
	

	vector<AngularRate > properMotion;

	
	
 	

	
	// ===> Attribute transition, which is optional
	
	
	bool transitionExists;
	

	vector<string > transition;

	
	
 	

	
	// ===> Attribute restFrequency, which is optional
	
	
	bool restFrequencyExists;
	

	vector<Frequency > restFrequency;

	
	
 	

	
	// ===> Attribute sysVel, which is optional
	
	
	bool sysVelExists;
	

	vector<Speed > sysVel;

	
	
 	

	
	// ===> Attribute sourceModel, which is optional
	
	
	bool sourceModelExists;
	

	SourceModelMod::SourceModel sourceModel;

	
	
 	

	
	// ===> Attribute deltaVel, which is optional
	
	
	bool deltaVelExists;
	

	Speed deltaVel;

	
	
 	

	
	// ===> Attribute rangeVel, which is optional
	
	
	bool rangeVelExists;
	

	vector<Speed > rangeVel;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sourceParameterId, which is optional
	
	
	bool sourceParameterIdExists;
	

	int sourceParameterId;

	
	
 	

	
	// ===> Attribute spectralWindowId
	
	

	Tag spectralWindowId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		


	


};

} // End namespace asdm

#endif /* Source_CLASS */
