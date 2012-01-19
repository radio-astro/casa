
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
 * File CalPointingModelRow.h
 */
 
#ifndef CalPointingModelRow_CLASS
#define CalPointingModelRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalPointingModelRowIDL;
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




	

	
#include "CAntennaMake.h"
using namespace AntennaMakeMod;
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
#include "CPointingModelMode.h"
using namespace PointingModelModeMod;
	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	



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

/*\file CalPointingModel.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::CalPointingModelTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

/**
 * The CalPointingModelRow class is a row of a CalPointingModelTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class CalPointingModelRow {
friend class asdm::CalPointingModelTable;

public:

	virtual ~CalPointingModelRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalPointingModelTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPointingModelRowIDL struct.
	 */
	CalPointingModelRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPointingModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (CalPointingModelRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute antennaName
	
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string getAntennaName() const;
	
 
 	
 	
 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setAntennaName (string antennaName);
  		
	
	
	


	
	// ===> Attribute antennaMake
	
	
	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake getAntennaMake() const;
	
 
 	
 	
 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 		
 			
 	 */
 	void setAntennaMake (AntennaMakeMod::AntennaMake antennaMake);
  		
	
	
	


	
	// ===> Attribute numObs
	
	
	

	
 	/**
 	 * Get numObs.
 	 * @return numObs as int
 	 */
 	int getNumObs() const;
	
 
 	
 	
 	/**
 	 * Set numObs with the specified int.
 	 * @param numObs The int value to which numObs is to be set.
 	 
 		
 			
 	 */
 	void setNumObs (int numObs);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numFormula
	
	
	

	
 	/**
 	 * Get numFormula.
 	 * @return numFormula as int
 	 */
 	int getNumFormula() const;
	
 
 	
 	
 	/**
 	 * Set numFormula with the specified int.
 	 * @param numFormula The int value to which numFormula is to be set.
 	 
 		
 			
 	 */
 	void setNumFormula (int numFormula);
  		
	
	
	


	
	// ===> Attribute azimuthRms
	
	
	

	
 	/**
 	 * Get azimuthRms.
 	 * @return azimuthRms as Angle
 	 */
 	Angle getAzimuthRms() const;
	
 
 	
 	
 	/**
 	 * Set azimuthRms with the specified Angle.
 	 * @param azimuthRms The Angle value to which azimuthRms is to be set.
 	 
 		
 			
 	 */
 	void setAzimuthRms (Angle azimuthRms);
  		
	
	
	


	
	// ===> Attribute elevationRms
	
	
	

	
 	/**
 	 * Get elevationRms.
 	 * @return elevationRms as Angle
 	 */
 	Angle getElevationRms() const;
	
 
 	
 	
 	/**
 	 * Set elevationRms with the specified Angle.
 	 * @param elevationRms The Angle value to which elevationRms is to be set.
 	 
 		
 			
 	 */
 	void setElevationRms (Angle elevationRms);
  		
	
	
	


	
	// ===> Attribute skyRms
	
	
	

	
 	/**
 	 * Get skyRms.
 	 * @return skyRms as Angle
 	 */
 	Angle getSkyRms() const;
	
 
 	
 	
 	/**
 	 * Set skyRms with the specified Angle.
 	 * @param skyRms The Angle value to which skyRms is to be set.
 	 
 		
 			
 	 */
 	void setSkyRms (Angle skyRms);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute coeffError
	
	
	

	
 	/**
 	 * Get coeffError.
 	 * @return coeffError as vector<float >
 	 */
 	vector<float > getCoeffError() const;
	
 
 	
 	
 	/**
 	 * Set coeffError with the specified vector<float >.
 	 * @param coeffError The vector<float > value to which coeffError is to be set.
 	 
 		
 			
 	 */
 	void setCoeffError (vector<float > coeffError);
  		
	
	
	


	
	// ===> Attribute coeffFixed
	
	
	

	
 	/**
 	 * Get coeffFixed.
 	 * @return coeffFixed as vector<bool >
 	 */
 	vector<bool > getCoeffFixed() const;
	
 
 	
 	
 	/**
 	 * Set coeffFixed with the specified vector<bool >.
 	 * @param coeffFixed The vector<bool > value to which coeffFixed is to be set.
 	 
 		
 			
 	 */
 	void setCoeffFixed (vector<bool > coeffFixed);
  		
	
	
	


	
	// ===> Attribute coeffFormula
	
	
	

	
 	/**
 	 * Get coeffFormula.
 	 * @return coeffFormula as vector<string >
 	 */
 	vector<string > getCoeffFormula() const;
	
 
 	
 	
 	/**
 	 * Set coeffFormula with the specified vector<string >.
 	 * @param coeffFormula The vector<string > value to which coeffFormula is to be set.
 	 
 		
 			
 	 */
 	void setCoeffFormula (vector<string > coeffFormula);
  		
	
	
	


	
	// ===> Attribute pointingModelMode
	
	
	

	
 	/**
 	 * Get pointingModelMode.
 	 * @return pointingModelMode as PointingModelModeMod::PointingModelMode
 	 */
 	PointingModelModeMod::PointingModelMode getPointingModelMode() const;
	
 
 	
 	
 	/**
 	 * Set pointingModelMode with the specified PointingModelModeMod::PointingModelMode.
 	 * @param pointingModelMode The PointingModelModeMod::PointingModelMode value to which pointingModelMode is to be set.
 	 
 		
 			
 	 */
 	void setPointingModelMode (PointingModelModeMod::PointingModelMode pointingModelMode);
  		
	
	
	


	
	// ===> Attribute receiverBand
	
	
	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand getReceiverBand() const;
	
 
 	
 	
 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand);
  		
	
	
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this CalPointingModelRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag calDataId, Tag calReductionId, string antennaName, ReceiverBandMod::ReceiverBand receiverBand, AntennaMakeMod::AntennaMake antennaMake, int numObs, int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, int numFormula, Angle azimuthRms, Angle elevationRms, Angle skyRms, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, vector<string > coeffFormula, PointingModelModeMod::PointingModelMode pointingModelMode);
	
	

	
	bool compareRequiredValue(AntennaMakeMod::AntennaMake antennaMake, int numObs, int numCoeff, ArrayTime startValidTime, ArrayTime endValidTime, int numFormula, Angle azimuthRms, Angle elevationRms, Angle skyRms, vector<string > coeffName, vector<float > coeffVal, vector<float > coeffError, vector<bool > coeffFixed, vector<string > coeffFormula, PointingModelModeMod::PointingModelMode pointingModelMode); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPointingModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalPointingModelRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	CalPointingModelTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalPointingModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPointingModelRow (CalPointingModelTable &table);

	/**
	 * Create a CalPointingModelRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalPointingModelRow row and a CalPointingModelTable table, the method creates a new
	 * CalPointingModelRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalPointingModelRow (CalPointingModelTable &table, CalPointingModelRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute antennaMake
	
	

	AntennaMakeMod::AntennaMake antennaMake;

	
	
 	

	
	// ===> Attribute numObs
	
	

	int numObs;

	
	
 	

	
	// ===> Attribute numCoeff
	
	

	int numCoeff;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute numFormula
	
	

	int numFormula;

	
	
 	

	
	// ===> Attribute azimuthRms
	
	

	Angle azimuthRms;

	
	
 	

	
	// ===> Attribute elevationRms
	
	

	Angle elevationRms;

	
	
 	

	
	// ===> Attribute skyRms
	
	

	Angle skyRms;

	
	
 	

	
	// ===> Attribute coeffName
	
	

	vector<string > coeffName;

	
	
 	

	
	// ===> Attribute coeffVal
	
	

	vector<float > coeffVal;

	
	
 	

	
	// ===> Attribute coeffError
	
	

	vector<float > coeffError;

	
	
 	

	
	// ===> Attribute coeffFixed
	
	

	vector<bool > coeffFixed;

	
	
 	

	
	// ===> Attribute coeffFormula
	
	

	vector<string > coeffFormula;

	
	
 	

	
	// ===> Attribute pointingModelMode
	
	

	PointingModelModeMod::PointingModelMode pointingModelMode;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

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

#endif /* CalPointingModel_CLASS */
