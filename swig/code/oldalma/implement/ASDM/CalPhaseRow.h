
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
 * File CalPhaseRow.h
 */
 
#ifndef CalPhaseRow_CLASS
#define CalPhaseRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalPhaseRowIDL;
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




	
#include "CBasebandName.h"
using namespace BasebandNameMod;
	

	

	

	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	

	
#include "CAtmPhaseCorrection.h"
using namespace AtmPhaseCorrectionMod;
	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	

	

	

	

	

	

	

	

	



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

/*\file CalPhase.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::CalPhaseTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

/**
 * The CalPhaseRow class is a row of a CalPhaseTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class CalPhaseRow {
friend class asdm::CalPhaseTable;

public:

	virtual ~CalPhaseRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalPhaseTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPhaseRowIDL struct.
	 */
	CalPhaseRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPhaseRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (CalPhaseRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute basebandName
	
	
	

	
 	/**
 	 * Get basebandName.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 */
 	BasebandNameMod::BasebandName getBasebandName() const;
	
 
 	
 	
 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setBasebandName (BasebandNameMod::BasebandName basebandName);
  		
	
	
	


	
	// ===> Attribute numBaseline
	
	
	

	
 	/**
 	 * Get numBaseline.
 	 * @return numBaseline as int
 	 */
 	int getNumBaseline() const;
	
 
 	
 	
 	/**
 	 * Set numBaseline with the specified int.
 	 * @param numBaseline The int value to which numBaseline is to be set.
 	 
 		
 			
 	 */
 	void setNumBaseline (int numBaseline);
  		
	
	
	


	
	// ===> Attribute numAPC
	
	
	

	
 	/**
 	 * Get numAPC.
 	 * @return numAPC as int
 	 */
 	int getNumAPC() const;
	
 
 	
 	
 	/**
 	 * Set numAPC with the specified int.
 	 * @param numAPC The int value to which numAPC is to be set.
 	 
 		
 			
 	 */
 	void setNumAPC (int numAPC);
  		
	
	
	


	
	// ===> Attribute numReceptor
	
	
	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int getNumReceptor() const;
	
 
 	
 	
 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 		
 			
 	 */
 	void setNumReceptor (int numReceptor);
  		
	
	
	


	
	// ===> Attribute receiverBand
	
	
	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand getReceiverBand() const;
	
 
 	
 	
 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 		
 			
 	 */
 	void setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand);
  		
	
	
	


	
	// ===> Attribute antennaNames
	
	
	

	
 	/**
 	 * Get antennaNames.
 	 * @return antennaNames as vector<vector<string > >
 	 */
 	vector<vector<string > > getAntennaNames() const;
	
 
 	
 	
 	/**
 	 * Set antennaNames with the specified vector<vector<string > >.
 	 * @param antennaNames The vector<vector<string > > value to which antennaNames is to be set.
 	 
 		
 			
 	 */
 	void setAntennaNames (vector<vector<string > > antennaNames);
  		
	
	
	


	
	// ===> Attribute atmPhaseCorrections
	
	
	

	
 	/**
 	 * Get atmPhaseCorrections.
 	 * @return atmPhaseCorrections as vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >
 	 */
 	vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > getAtmPhaseCorrections() const;
	
 
 	
 	
 	/**
 	 * Set atmPhaseCorrections with the specified vector<AtmPhaseCorrectionMod::AtmPhaseCorrection >.
 	 * @param atmPhaseCorrections The vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > value to which atmPhaseCorrections is to be set.
 	 
 		
 			
 	 */
 	void setAtmPhaseCorrections (vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections);
  		
	
	
	


	
	// ===> Attribute polarizationTypes
	
	
	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > getPolarizationTypes() const;
	
 
 	
 	
 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 		
 			
 	 */
 	void setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute frequencyRange
	
	
	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > getFrequencyRange() const;
	
 
 	
 	
 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyRange (vector<Frequency > frequencyRange);
  		
	
	
	


	
	// ===> Attribute ampli
	
	
	

	
 	/**
 	 * Get ampli.
 	 * @return ampli as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > getAmpli() const;
	
 
 	
 	
 	/**
 	 * Set ampli with the specified vector<vector<vector<float > > >.
 	 * @param ampli The vector<vector<vector<float > > > value to which ampli is to be set.
 	 
 		
 			
 	 */
 	void setAmpli (vector<vector<vector<float > > > ampli);
  		
	
	
	


	
	// ===> Attribute phase
	
	
	

	
 	/**
 	 * Get phase.
 	 * @return phase as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > getPhase() const;
	
 
 	
 	
 	/**
 	 * Set phase with the specified vector<vector<vector<float > > >.
 	 * @param phase The vector<vector<vector<float > > > value to which phase is to be set.
 	 
 		
 			
 	 */
 	void setPhase (vector<vector<vector<float > > > phase);
  		
	
	
	


	
	// ===> Attribute phaseRms
	
	
	

	
 	/**
 	 * Get phaseRms.
 	 * @return phaseRms as vector<vector<vector<Angle > > >
 	 */
 	vector<vector<vector<Angle > > > getPhaseRms() const;
	
 
 	
 	
 	/**
 	 * Set phaseRms with the specified vector<vector<vector<Angle > > >.
 	 * @param phaseRms The vector<vector<vector<Angle > > > value to which phaseRms is to be set.
 	 
 		
 			
 	 */
 	void setPhaseRms (vector<vector<vector<Angle > > > phaseRms);
  		
	
	
	


	
	// ===> Attribute statPhaseRms
	
	
	

	
 	/**
 	 * Get statPhaseRms.
 	 * @return statPhaseRms as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getStatPhaseRms() const;
	
 
 	
 	
 	/**
 	 * Set statPhaseRms with the specified vector<vector<Angle > >.
 	 * @param statPhaseRms The vector<vector<Angle > > value to which statPhaseRms is to be set.
 	 
 		
 			
 	 */
 	void setStatPhaseRms (vector<vector<Angle > > statPhaseRms);
  		
	
	
	


	
	// ===> Attribute correctionValidity, which is optional
	
	
	
	/**
	 * The attribute correctionValidity is optional. Return true if this attribute exists.
	 * @return true if and only if the correctionValidity attribute exists. 
	 */
	bool isCorrectionValidityExists() const;
	

	
 	/**
 	 * Get correctionValidity, which is optional.
 	 * @return correctionValidity as vector<bool >
 	 * @throws IllegalAccessException If correctionValidity does not exist.
 	 */
 	vector<bool > getCorrectionValidity() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set correctionValidity with the specified vector<bool >.
 	 * @param correctionValidity The vector<bool > value to which correctionValidity is to be set.
 	 
 		
 	 */
 	void setCorrectionValidity (vector<bool > correctionValidity);
		
	
	
	
	/**
	 * Mark correctionValidity, which is an optional field, as non-existent.
	 */
	void clearCorrectionValidity ();
	


	
	// ===> Attribute decorrelationFactor
	
	
	

	
 	/**
 	 * Get decorrelationFactor.
 	 * @return decorrelationFactor as vector<vector<float > >
 	 */
 	vector<vector<float > > getDecorrelationFactor() const;
	
 
 	
 	
 	/**
 	 * Set decorrelationFactor with the specified vector<vector<float > >.
 	 * @param decorrelationFactor The vector<vector<float > > value to which decorrelationFactor is to be set.
 	 
 		
 			
 	 */
 	void setDecorrelationFactor (vector<vector<float > > decorrelationFactor);
  		
	
	
	


	
	// ===> Attribute baselineLengths
	
	
	

	
 	/**
 	 * Get baselineLengths.
 	 * @return baselineLengths as vector<Length >
 	 */
 	vector<Length > getBaselineLengths() const;
	
 
 	
 	
 	/**
 	 * Set baselineLengths with the specified vector<Length >.
 	 * @param baselineLengths The vector<Length > value to which baselineLengths is to be set.
 	 
 		
 			
 	 */
 	void setBaselineLengths (vector<Length > baselineLengths);
  		
	
	
	


	
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
	 * Compare each mandatory attribute except the autoincrementable one of this CalPhaseRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag calDataId, Tag calReductionId, BasebandNameMod::BasebandName basebandName, int numBaseline, int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<vector<string > > antennaNames, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<vector<vector<float > > > ampli, vector<vector<vector<float > > > phase, vector<vector<vector<Angle > > > phaseRms, vector<vector<Angle > > statPhaseRms, vector<vector<float > > decorrelationFactor, vector<Length > baselineLengths, vector<Angle > direction);
	
	

	
	bool compareRequiredValue(int numBaseline, int numAPC, int numReceptor, ReceiverBandMod::ReceiverBand receiverBand, vector<vector<string > > antennaNames, vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, vector<vector<vector<float > > > ampli, vector<vector<vector<float > > > phase, vector<vector<vector<Angle > > > phaseRms, vector<vector<Angle > > statPhaseRms, vector<vector<float > > decorrelationFactor, vector<Length > baselineLengths, vector<Angle > direction); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPhaseRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalPhaseRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	CalPhaseTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a CalPhaseRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPhaseRow (CalPhaseTable &table);

	/**
	 * Create a CalPhaseRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalPhaseRow row and a CalPhaseTable table, the method creates a new
	 * CalPhaseRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalPhaseRow (CalPhaseTable &table, CalPhaseRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute basebandName
	
	

	BasebandNameMod::BasebandName basebandName;

	
	
 	

	
	// ===> Attribute numBaseline
	
	

	int numBaseline;

	
	
 	

	
	// ===> Attribute numAPC
	
	

	int numAPC;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute antennaNames
	
	

	vector<vector<string > > antennaNames;

	
	
 	

	
	// ===> Attribute atmPhaseCorrections
	
	

	vector<AtmPhaseCorrectionMod::AtmPhaseCorrection > atmPhaseCorrections;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute ampli
	
	

	vector<vector<vector<float > > > ampli;

	
	
 	

	
	// ===> Attribute phase
	
	

	vector<vector<vector<float > > > phase;

	
	
 	

	
	// ===> Attribute phaseRms
	
	

	vector<vector<vector<Angle > > > phaseRms;

	
	
 	

	
	// ===> Attribute statPhaseRms
	
	

	vector<vector<Angle > > statPhaseRms;

	
	
 	

	
	// ===> Attribute correctionValidity, which is optional
	
	
	bool correctionValidityExists;
	

	vector<bool > correctionValidity;

	
	
 	

	
	// ===> Attribute decorrelationFactor
	
	

	vector<vector<float > > decorrelationFactor;

	
	
 	

	
	// ===> Attribute baselineLengths
	
	

	vector<Length > baselineLengths;

	
	
 	

	
	// ===> Attribute direction
	
	

	vector<Angle > direction;

	
	
 	

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

#endif /* CalPhase_CLASS */
