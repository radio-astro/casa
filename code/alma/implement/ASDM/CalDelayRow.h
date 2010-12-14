
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
 * File CalDelayRow.h
 */
 
#ifndef CalDelayRow_CLASS
#define CalDelayRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalDelayRowIDL;
#endif






#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;

#include <Frequency.h>
using  asdm::Frequency;




	

	
#include "CAtmPhaseCorrection.h"
using namespace AtmPhaseCorrectionMod;
	

	
#include "CBasebandName.h"
using namespace BasebandNameMod;
	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	

	

	

	

	

	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	

	

	

	

	
#include "CReceiverSideband.h"
using namespace ReceiverSidebandMod;
	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file CalDelay.h
    \brief Generated from model's revision "1.55", branch "HEAD"
*/

namespace asdm {

//class asdm::CalDelayTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

class CalDelayRow;
typedef void (CalDelayRow::*CalDelayAttributeFromBin) (EndianISStream& eiss);

/**
 * The CalDelayRow class is a row of a CalDelayTable.
 * 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 */
class CalDelayRow {
friend class asdm::CalDelayTable;

public:

	virtual ~CalDelayRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalDelayTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
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
  		
	
	
	


	
	// ===> Attribute atmPhaseCorrection
	
	
	

	
 	/**
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as AtmPhaseCorrectionMod::AtmPhaseCorrection
 	 */
 	AtmPhaseCorrectionMod::AtmPhaseCorrection getAtmPhaseCorrection() const;
	
 
 	
 	
 	/**
 	 * Set atmPhaseCorrection with the specified AtmPhaseCorrectionMod::AtmPhaseCorrection.
 	 * @param atmPhaseCorrection The AtmPhaseCorrectionMod::AtmPhaseCorrection value to which atmPhaseCorrection is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setAtmPhaseCorrection (AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute refAntennaName
	
	
	

	
 	/**
 	 * Get refAntennaName.
 	 * @return refAntennaName as string
 	 */
 	string getRefAntennaName() const;
	
 
 	
 	
 	/**
 	 * Set refAntennaName with the specified string.
 	 * @param refAntennaName The string value to which refAntennaName is to be set.
 	 
 		
 			
 	 */
 	void setRefAntennaName (string refAntennaName);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute delayError
	
	
	

	
 	/**
 	 * Get delayError.
 	 * @return delayError as vector<double >
 	 */
 	vector<double > getDelayError() const;
	
 
 	
 	
 	/**
 	 * Set delayError with the specified vector<double >.
 	 * @param delayError The vector<double > value to which delayError is to be set.
 	 
 		
 			
 	 */
 	void setDelayError (vector<double > delayError);
  		
	
	
	


	
	// ===> Attribute delayOffset
	
	
	

	
 	/**
 	 * Get delayOffset.
 	 * @return delayOffset as vector<double >
 	 */
 	vector<double > getDelayOffset() const;
	
 
 	
 	
 	/**
 	 * Set delayOffset with the specified vector<double >.
 	 * @param delayOffset The vector<double > value to which delayOffset is to be set.
 	 
 		
 			
 	 */
 	void setDelayOffset (vector<double > delayOffset);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute reducedChiSquared
	
	
	

	
 	/**
 	 * Get reducedChiSquared.
 	 * @return reducedChiSquared as vector<double >
 	 */
 	vector<double > getReducedChiSquared() const;
	
 
 	
 	
 	/**
 	 * Set reducedChiSquared with the specified vector<double >.
 	 * @param reducedChiSquared The vector<double > value to which reducedChiSquared is to be set.
 	 
 		
 			
 	 */
 	void setReducedChiSquared (vector<double > reducedChiSquared);
  		
	
	
	


	
	// ===> Attribute crossDelayOffset, which is optional
	
	
	
	/**
	 * The attribute crossDelayOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the crossDelayOffset attribute exists. 
	 */
	bool isCrossDelayOffsetExists() const;
	

	
 	/**
 	 * Get crossDelayOffset, which is optional.
 	 * @return crossDelayOffset as double
 	 * @throws IllegalAccessException If crossDelayOffset does not exist.
 	 */
 	double getCrossDelayOffset() const;
	
 
 	
 	
 	/**
 	 * Set crossDelayOffset with the specified double.
 	 * @param crossDelayOffset The double value to which crossDelayOffset is to be set.
 	 
 		
 	 */
 	void setCrossDelayOffset (double crossDelayOffset);
		
	
	
	
	/**
	 * Mark crossDelayOffset, which is an optional field, as non-existent.
	 */
	void clearCrossDelayOffset ();
	


	
	// ===> Attribute crossDelayOffsetError, which is optional
	
	
	
	/**
	 * The attribute crossDelayOffsetError is optional. Return true if this attribute exists.
	 * @return true if and only if the crossDelayOffsetError attribute exists. 
	 */
	bool isCrossDelayOffsetErrorExists() const;
	

	
 	/**
 	 * Get crossDelayOffsetError, which is optional.
 	 * @return crossDelayOffsetError as double
 	 * @throws IllegalAccessException If crossDelayOffsetError does not exist.
 	 */
 	double getCrossDelayOffsetError() const;
	
 
 	
 	
 	/**
 	 * Set crossDelayOffsetError with the specified double.
 	 * @param crossDelayOffsetError The double value to which crossDelayOffsetError is to be set.
 	 
 		
 	 */
 	void setCrossDelayOffsetError (double crossDelayOffsetError);
		
	
	
	
	/**
	 * Mark crossDelayOffsetError, which is an optional field, as non-existent.
	 */
	void clearCrossDelayOffsetError ();
	


	
	// ===> Attribute numSideband, which is optional
	
	
	
	/**
	 * The attribute numSideband is optional. Return true if this attribute exists.
	 * @return true if and only if the numSideband attribute exists. 
	 */
	bool isNumSidebandExists() const;
	

	
 	/**
 	 * Get numSideband, which is optional.
 	 * @return numSideband as int
 	 * @throws IllegalAccessException If numSideband does not exist.
 	 */
 	int getNumSideband() const;
	
 
 	
 	
 	/**
 	 * Set numSideband with the specified int.
 	 * @param numSideband The int value to which numSideband is to be set.
 	 
 		
 	 */
 	void setNumSideband (int numSideband);
		
	
	
	
	/**
	 * Mark numSideband, which is an optional field, as non-existent.
	 */
	void clearNumSideband ();
	


	
	// ===> Attribute refFreq, which is optional
	
	
	
	/**
	 * The attribute refFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the refFreq attribute exists. 
	 */
	bool isRefFreqExists() const;
	

	
 	/**
 	 * Get refFreq, which is optional.
 	 * @return refFreq as vector<Frequency >
 	 * @throws IllegalAccessException If refFreq does not exist.
 	 */
 	vector<Frequency > getRefFreq() const;
	
 
 	
 	
 	/**
 	 * Set refFreq with the specified vector<Frequency >.
 	 * @param refFreq The vector<Frequency > value to which refFreq is to be set.
 	 
 		
 	 */
 	void setRefFreq (vector<Frequency > refFreq);
		
	
	
	
	/**
	 * Mark refFreq, which is an optional field, as non-existent.
	 */
	void clearRefFreq ();
	


	
	// ===> Attribute refFreqPhase, which is optional
	
	
	
	/**
	 * The attribute refFreqPhase is optional. Return true if this attribute exists.
	 * @return true if and only if the refFreqPhase attribute exists. 
	 */
	bool isRefFreqPhaseExists() const;
	

	
 	/**
 	 * Get refFreqPhase, which is optional.
 	 * @return refFreqPhase as vector<Angle >
 	 * @throws IllegalAccessException If refFreqPhase does not exist.
 	 */
 	vector<Angle > getRefFreqPhase() const;
	
 
 	
 	
 	/**
 	 * Set refFreqPhase with the specified vector<Angle >.
 	 * @param refFreqPhase The vector<Angle > value to which refFreqPhase is to be set.
 	 
 		
 	 */
 	void setRefFreqPhase (vector<Angle > refFreqPhase);
		
	
	
	
	/**
	 * Mark refFreqPhase, which is an optional field, as non-existent.
	 */
	void clearRefFreqPhase ();
	


	
	// ===> Attribute sidebands, which is optional
	
	
	
	/**
	 * The attribute sidebands is optional. Return true if this attribute exists.
	 * @return true if and only if the sidebands attribute exists. 
	 */
	bool isSidebandsExists() const;
	

	
 	/**
 	 * Get sidebands, which is optional.
 	 * @return sidebands as vector<ReceiverSidebandMod::ReceiverSideband >
 	 * @throws IllegalAccessException If sidebands does not exist.
 	 */
 	vector<ReceiverSidebandMod::ReceiverSideband > getSidebands() const;
	
 
 	
 	
 	/**
 	 * Set sidebands with the specified vector<ReceiverSidebandMod::ReceiverSideband >.
 	 * @param sidebands The vector<ReceiverSidebandMod::ReceiverSideband > value to which sidebands is to be set.
 	 
 		
 	 */
 	void setSidebands (vector<ReceiverSidebandMod::ReceiverSideband > sidebands);
		
	
	
	
	/**
	 * Mark sidebands, which is an optional field, as non-existent.
	 */
	void clearSidebands ();
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this CalDelayRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaName
	    
	 * @param atmPhaseCorrection
	    
	 * @param basebandName
	    
	 * @param receiverBand
	    
	 * @param calDataId
	    
	 * @param calReductionId
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param refAntennaName
	    
	 * @param numReceptor
	    
	 * @param delayError
	    
	 * @param delayOffset
	    
	 * @param polarizationTypes
	    
	 * @param reducedChiSquared
	    
	 */ 
	bool compareNoAutoInc(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, BasebandNameMod::BasebandName basebandName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, string refAntennaName, int numReceptor, vector<double > delayError, vector<double > delayOffset, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > reducedChiSquared);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param refAntennaName
	    
	 * @param numReceptor
	    
	 * @param delayError
	    
	 * @param delayOffset
	    
	 * @param polarizationTypes
	    
	 * @param reducedChiSquared
	    
	 */ 
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, string refAntennaName, int numReceptor, vector<double > delayError, vector<double > delayOffset, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > reducedChiSquared); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalDelayRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalDelayRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalDelayRowIDL struct.
	 */
	CalDelayRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalDelayRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (CalDelayRowIDL x) ;
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
	CalDelayTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a CalDelayRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalDelayRow (CalDelayTable &table);

	/**
	 * Create a CalDelayRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalDelayRow row and a CalDelayTable table, the method creates a new
	 * CalDelayRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalDelayRow (CalDelayTable &table, CalDelayRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute atmPhaseCorrection
	
	

	AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection;

	
	
 	

	
	// ===> Attribute basebandName
	
	

	BasebandNameMod::BasebandName basebandName;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute refAntennaName
	
	

	string refAntennaName;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute delayError
	
	

	vector<double > delayError;

	
	
 	

	
	// ===> Attribute delayOffset
	
	

	vector<double > delayOffset;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute reducedChiSquared
	
	

	vector<double > reducedChiSquared;

	
	
 	

	
	// ===> Attribute crossDelayOffset, which is optional
	
	
	bool crossDelayOffsetExists;
	

	double crossDelayOffset;

	
	
 	

	
	// ===> Attribute crossDelayOffsetError, which is optional
	
	
	bool crossDelayOffsetErrorExists;
	

	double crossDelayOffsetError;

	
	
 	

	
	// ===> Attribute numSideband, which is optional
	
	
	bool numSidebandExists;
	

	int numSideband;

	
	
 	

	
	// ===> Attribute refFreq, which is optional
	
	
	bool refFreqExists;
	

	vector<Frequency > refFreq;

	
	
 	

	
	// ===> Attribute refFreqPhase, which is optional
	
	
	bool refFreqPhaseExists;
	

	vector<Angle > refFreqPhase;

	
	
 	

	
	// ===> Attribute sidebands, which is optional
	
	
	bool sidebandsExists;
	

	vector<ReceiverSidebandMod::ReceiverSideband > sidebands;

	
	
 	

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
	map<string, CalDelayAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianISStream& eiss);
void atmPhaseCorrectionFromBin( EndianISStream& eiss);
void basebandNameFromBin( EndianISStream& eiss);
void receiverBandFromBin( EndianISStream& eiss);
void calDataIdFromBin( EndianISStream& eiss);
void calReductionIdFromBin( EndianISStream& eiss);
void startValidTimeFromBin( EndianISStream& eiss);
void endValidTimeFromBin( EndianISStream& eiss);
void refAntennaNameFromBin( EndianISStream& eiss);
void numReceptorFromBin( EndianISStream& eiss);
void delayErrorFromBin( EndianISStream& eiss);
void delayOffsetFromBin( EndianISStream& eiss);
void polarizationTypesFromBin( EndianISStream& eiss);
void reducedChiSquaredFromBin( EndianISStream& eiss);

void crossDelayOffsetFromBin( EndianISStream& eiss);
void crossDelayOffsetErrorFromBin( EndianISStream& eiss);
void numSidebandFromBin( EndianISStream& eiss);
void refFreqFromBin( EndianISStream& eiss);
void refFreqPhaseFromBin( EndianISStream& eiss);
void sidebandsFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the CalDelayTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalDelayRow* fromBin(EndianISStream& eiss, CalDelayTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* CalDelay_CLASS */
