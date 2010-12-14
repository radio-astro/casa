
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
 * File CalCurveRow.h
 */
 
#ifndef CalCurveRow_CLASS
#define CalCurveRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalCurveRowIDL;
#endif






#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Tag.h>
using  asdm::Tag;

#include <Frequency.h>
using  asdm::Frequency;




	
#include "CAtmPhaseCorrection.h"
using namespace AtmPhaseCorrectionMod;
	

	
#include "CCalCurveType.h"
using namespace CalCurveTypeMod;
	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	

	

	

	

	

	

	

	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file CalCurve.h
    \brief Generated from model's revision "1.55", branch "HEAD"
*/

namespace asdm {

//class asdm::CalCurveTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

class CalCurveRow;
typedef void (CalCurveRow::*CalCurveAttributeFromBin) (EndianISStream& eiss);

/**
 * The CalCurveRow class is a row of a CalCurveTable.
 * 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 */
class CalCurveRow {
friend class asdm::CalCurveTable;

public:

	virtual ~CalCurveRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalCurveTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
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
  		
	
	
	


	
	// ===> Attribute typeCurve
	
	
	

	
 	/**
 	 * Get typeCurve.
 	 * @return typeCurve as CalCurveTypeMod::CalCurveType
 	 */
 	CalCurveTypeMod::CalCurveType getTypeCurve() const;
	
 
 	
 	
 	/**
 	 * Set typeCurve with the specified CalCurveTypeMod::CalCurveType.
 	 * @param typeCurve The CalCurveTypeMod::CalCurveType value to which typeCurve is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setTypeCurve (CalCurveTypeMod::CalCurveType typeCurve);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numAntenna
	
	
	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int getNumAntenna() const;
	
 
 	
 	
 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 		
 			
 	 */
 	void setNumAntenna (int numAntenna);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute antennaNames
	
	
	

	
 	/**
 	 * Get antennaNames.
 	 * @return antennaNames as vector<string >
 	 */
 	vector<string > getAntennaNames() const;
	
 
 	
 	
 	/**
 	 * Set antennaNames with the specified vector<string >.
 	 * @param antennaNames The vector<string > value to which antennaNames is to be set.
 	 
 		
 			
 	 */
 	void setAntennaNames (vector<string > antennaNames);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute curve
	
	
	

	
 	/**
 	 * Get curve.
 	 * @return curve as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > getCurve() const;
	
 
 	
 	
 	/**
 	 * Set curve with the specified vector<vector<vector<float > > >.
 	 * @param curve The vector<vector<vector<float > > > value to which curve is to be set.
 	 
 		
 			
 	 */
 	void setCurve (vector<vector<vector<float > > > curve);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numBaseline, which is optional
	
	
	
	/**
	 * The attribute numBaseline is optional. Return true if this attribute exists.
	 * @return true if and only if the numBaseline attribute exists. 
	 */
	bool isNumBaselineExists() const;
	

	
 	/**
 	 * Get numBaseline, which is optional.
 	 * @return numBaseline as int
 	 * @throws IllegalAccessException If numBaseline does not exist.
 	 */
 	int getNumBaseline() const;
	
 
 	
 	
 	/**
 	 * Set numBaseline with the specified int.
 	 * @param numBaseline The int value to which numBaseline is to be set.
 	 
 		
 	 */
 	void setNumBaseline (int numBaseline);
		
	
	
	
	/**
	 * Mark numBaseline, which is an optional field, as non-existent.
	 */
	void clearNumBaseline ();
	


	
	// ===> Attribute rms, which is optional
	
	
	
	/**
	 * The attribute rms is optional. Return true if this attribute exists.
	 * @return true if and only if the rms attribute exists. 
	 */
	bool isRmsExists() const;
	

	
 	/**
 	 * Get rms, which is optional.
 	 * @return rms as vector<vector<float > >
 	 * @throws IllegalAccessException If rms does not exist.
 	 */
 	vector<vector<float > > getRms() const;
	
 
 	
 	
 	/**
 	 * Set rms with the specified vector<vector<float > >.
 	 * @param rms The vector<vector<float > > value to which rms is to be set.
 	 
 		
 	 */
 	void setRms (vector<vector<float > > rms);
		
	
	
	
	/**
	 * Mark rms, which is an optional field, as non-existent.
	 */
	void clearRms ();
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this CalCurveRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param atmPhaseCorrection
	    
	 * @param typeCurve
	    
	 * @param receiverBand
	    
	 * @param calDataId
	    
	 * @param calReductionId
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param frequencyRange
	    
	 * @param numAntenna
	    
	 * @param numPoly
	    
	 * @param numReceptor
	    
	 * @param antennaNames
	    
	 * @param refAntennaName
	    
	 * @param polarizationTypes
	    
	 * @param curve
	    
	 * @param reducedChiSquared
	    
	 */ 
	bool compareNoAutoInc(AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param frequencyRange
	    
	 * @param numAntenna
	    
	 * @param numPoly
	    
	 * @param numReceptor
	    
	 * @param antennaNames
	    
	 * @param refAntennaName
	    
	 * @param polarizationTypes
	    
	 * @param curve
	    
	 * @param reducedChiSquared
	    
	 */ 
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, vector<Frequency > frequencyRange, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalCurveRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalCurveRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalCurveRowIDL struct.
	 */
	CalCurveRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalCurveRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (CalCurveRowIDL x) ;
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
	CalCurveTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a CalCurveRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalCurveRow (CalCurveTable &table);

	/**
	 * Create a CalCurveRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalCurveRow row and a CalCurveTable table, the method creates a new
	 * CalCurveRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalCurveRow (CalCurveTable &table, CalCurveRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute atmPhaseCorrection
	
	

	AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection;

	
	
 	

	
	// ===> Attribute typeCurve
	
	

	CalCurveTypeMod::CalCurveType typeCurve;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<Frequency > frequencyRange;

	
	
 	

	
	// ===> Attribute numAntenna
	
	

	int numAntenna;

	
	
 	

	
	// ===> Attribute numPoly
	
	

	int numPoly;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute antennaNames
	
	

	vector<string > antennaNames;

	
	
 	

	
	// ===> Attribute refAntennaName
	
	

	string refAntennaName;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute curve
	
	

	vector<vector<vector<float > > > curve;

	
	
 	

	
	// ===> Attribute reducedChiSquared
	
	

	vector<double > reducedChiSquared;

	
	
 	

	
	// ===> Attribute numBaseline, which is optional
	
	
	bool numBaselineExists;
	

	int numBaseline;

	
	
 	

	
	// ===> Attribute rms, which is optional
	
	
	bool rmsExists;
	

	vector<vector<float > > rms;

	
	
 	

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
	map<string, CalCurveAttributeFromBin> fromBinMethods;
void atmPhaseCorrectionFromBin( EndianISStream& eiss);
void typeCurveFromBin( EndianISStream& eiss);
void receiverBandFromBin( EndianISStream& eiss);
void calDataIdFromBin( EndianISStream& eiss);
void calReductionIdFromBin( EndianISStream& eiss);
void startValidTimeFromBin( EndianISStream& eiss);
void endValidTimeFromBin( EndianISStream& eiss);
void frequencyRangeFromBin( EndianISStream& eiss);
void numAntennaFromBin( EndianISStream& eiss);
void numPolyFromBin( EndianISStream& eiss);
void numReceptorFromBin( EndianISStream& eiss);
void antennaNamesFromBin( EndianISStream& eiss);
void refAntennaNameFromBin( EndianISStream& eiss);
void polarizationTypesFromBin( EndianISStream& eiss);
void curveFromBin( EndianISStream& eiss);
void reducedChiSquaredFromBin( EndianISStream& eiss);

void numBaselineFromBin( EndianISStream& eiss);
void rmsFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the CalCurveTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalCurveRow* fromBin(EndianISStream& eiss, CalCurveTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* CalCurve_CLASS */
