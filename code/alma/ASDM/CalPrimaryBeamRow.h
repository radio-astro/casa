
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
 * File CalPrimaryBeamRow.h
 */
 
#ifndef CalPrimaryBeamRow_CLASS
#define CalPrimaryBeamRow_CLASS

#include <vector>
#include <string>
#include <set>

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
#endif






	 
#include <ArrayTime.h>
	

	 
#include <Angle.h>
	

	 
#include <Tag.h>
	

	 
#include <Frequency.h>
	

	 
#include <EntityRef.h>
	




	

	
#include "CReceiverBand.h"
	

	

	

	
#include "CAntennaMake.h"
	

	

	

	

	
#include "CPolarizationType.h"
	

	

	

	

	

	

	

	
#include "CPrimaryBeamDescription.h"
	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>

#include <RowTransformer.h>
//#include <TableStreamReader.h>

/*\file CalPrimaryBeam.h
    \brief Generated from model's revision "1.64", branch "HEAD"
*/

namespace asdm {

//class asdm::CalPrimaryBeamTable;


// class asdm::CalReductionRow;
class CalReductionRow;

// class asdm::CalDataRow;
class CalDataRow;
	

class CalPrimaryBeamRow;
typedef void (CalPrimaryBeamRow::*CalPrimaryBeamAttributeFromBin) (EndianIStream& eis);
typedef void (CalPrimaryBeamRow::*CalPrimaryBeamAttributeFromText) (const string& s);

/**
 * The CalPrimaryBeamRow class is a row of a CalPrimaryBeamTable.
 * 
 * Generated from model's revision "1.64", branch "HEAD"
 *
 */
class CalPrimaryBeamRow {
friend class asdm::CalPrimaryBeamTable;
friend class asdm::RowTransformer<CalPrimaryBeamRow>;
//friend class asdm::TableStreamReader<CalPrimaryBeamTable, CalPrimaryBeamRow>;

public:

	virtual ~CalPrimaryBeamRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalPrimaryBeamTable &getTable() const;
	
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
  		
	
	
	


	
	// ===> Attribute numSubband
	
	
	

	
 	/**
 	 * Get numSubband.
 	 * @return numSubband as int
 	 */
 	int getNumSubband() const;
	
 
 	
 	
 	/**
 	 * Set numSubband with the specified int.
 	 * @param numSubband The int value to which numSubband is to be set.
 	 
 		
 			
 	 */
 	void setNumSubband (int numSubband);
  		
	
	
	


	
	// ===> Attribute frequencyRange
	
	
	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<vector<Frequency > >
 	 */
 	vector<vector<Frequency > > getFrequencyRange() const;
	
 
 	
 	
 	/**
 	 * Set frequencyRange with the specified vector<vector<Frequency > >.
 	 * @param frequencyRange The vector<vector<Frequency > > value to which frequencyRange is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyRange (vector<vector<Frequency > > frequencyRange);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute mainBeamEfficiency
	
	
	

	
 	/**
 	 * Get mainBeamEfficiency.
 	 * @return mainBeamEfficiency as vector<double >
 	 */
 	vector<double > getMainBeamEfficiency() const;
	
 
 	
 	
 	/**
 	 * Set mainBeamEfficiency with the specified vector<double >.
 	 * @param mainBeamEfficiency The vector<double > value to which mainBeamEfficiency is to be set.
 	 
 		
 			
 	 */
 	void setMainBeamEfficiency (vector<double > mainBeamEfficiency);
  		
	
	
	


	
	// ===> Attribute beamDescriptionUID
	
	
	

	
 	/**
 	 * Get beamDescriptionUID.
 	 * @return beamDescriptionUID as EntityRef
 	 */
 	EntityRef getBeamDescriptionUID() const;
	
 
 	
 	
 	/**
 	 * Set beamDescriptionUID with the specified EntityRef.
 	 * @param beamDescriptionUID The EntityRef value to which beamDescriptionUID is to be set.
 	 
 		
 			
 	 */
 	void setBeamDescriptionUID (EntityRef beamDescriptionUID);
  		
	
	
	


	
	// ===> Attribute relativeAmplitudeRms
	
	
	

	
 	/**
 	 * Get relativeAmplitudeRms.
 	 * @return relativeAmplitudeRms as float
 	 */
 	float getRelativeAmplitudeRms() const;
	
 
 	
 	
 	/**
 	 * Set relativeAmplitudeRms with the specified float.
 	 * @param relativeAmplitudeRms The float value to which relativeAmplitudeRms is to be set.
 	 
 		
 			
 	 */
 	void setRelativeAmplitudeRms (float relativeAmplitudeRms);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute minValidDirection
	
	
	

	
 	/**
 	 * Get minValidDirection.
 	 * @return minValidDirection as vector<Angle >
 	 */
 	vector<Angle > getMinValidDirection() const;
	
 
 	
 	
 	/**
 	 * Set minValidDirection with the specified vector<Angle >.
 	 * @param minValidDirection The vector<Angle > value to which minValidDirection is to be set.
 	 
 		
 			
 	 */
 	void setMinValidDirection (vector<Angle > minValidDirection);
  		
	
	
	


	
	// ===> Attribute maxValidDirection
	
	
	

	
 	/**
 	 * Get maxValidDirection.
 	 * @return maxValidDirection as vector<Angle >
 	 */
 	vector<Angle > getMaxValidDirection() const;
	
 
 	
 	
 	/**
 	 * Set maxValidDirection with the specified vector<Angle >.
 	 * @param maxValidDirection The vector<Angle > value to which maxValidDirection is to be set.
 	 
 		
 			
 	 */
 	void setMaxValidDirection (vector<Angle > maxValidDirection);
  		
	
	
	


	
	// ===> Attribute descriptionType
	
	
	

	
 	/**
 	 * Get descriptionType.
 	 * @return descriptionType as PrimaryBeamDescriptionMod::PrimaryBeamDescription
 	 */
 	PrimaryBeamDescriptionMod::PrimaryBeamDescription getDescriptionType() const;
	
 
 	
 	
 	/**
 	 * Set descriptionType with the specified PrimaryBeamDescriptionMod::PrimaryBeamDescription.
 	 * @param descriptionType The PrimaryBeamDescriptionMod::PrimaryBeamDescription value to which descriptionType is to be set.
 	 
 		
 			
 	 */
 	void setDescriptionType (PrimaryBeamDescriptionMod::PrimaryBeamDescription descriptionType);
  		
	
	
	


	
	// ===> Attribute imageChannelNumber
	
	
	

	
 	/**
 	 * Get imageChannelNumber.
 	 * @return imageChannelNumber as vector<int >
 	 */
 	vector<int > getImageChannelNumber() const;
	
 
 	
 	
 	/**
 	 * Set imageChannelNumber with the specified vector<int >.
 	 * @param imageChannelNumber The vector<int > value to which imageChannelNumber is to be set.
 	 
 		
 			
 	 */
 	void setImageChannelNumber (vector<int > imageChannelNumber);
  		
	
	
	


	
	// ===> Attribute imageNominalFrequency
	
	
	

	
 	/**
 	 * Get imageNominalFrequency.
 	 * @return imageNominalFrequency as vector<Frequency >
 	 */
 	vector<Frequency > getImageNominalFrequency() const;
	
 
 	
 	
 	/**
 	 * Set imageNominalFrequency with the specified vector<Frequency >.
 	 * @param imageNominalFrequency The vector<Frequency > value to which imageNominalFrequency is to be set.
 	 
 		
 			
 	 */
 	void setImageNominalFrequency (vector<Frequency > imageNominalFrequency);
  		
	
	
	


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
	 * calReductionId pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* getCalReductionUsingCalReductionId();
	 

	

	

	
		
	/**
	 * calDataId pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* getCalDataUsingCalDataId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this CalPrimaryBeamRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaName
	    
	 * @param receiverBand
	    
	 * @param calDataId
	    
	 * @param calReductionId
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param antennaMake
	    
	 * @param numSubband
	    
	 * @param frequencyRange
	    
	 * @param numReceptor
	    
	 * @param polarizationTypes
	    
	 * @param mainBeamEfficiency
	    
	 * @param beamDescriptionUID
	    
	 * @param relativeAmplitudeRms
	    
	 * @param direction
	    
	 * @param minValidDirection
	    
	 * @param maxValidDirection
	    
	 * @param descriptionType
	    
	 * @param imageChannelNumber
	    
	 * @param imageNominalFrequency
	    
	 */ 
	bool compareNoAutoInc(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, AntennaMakeMod::AntennaMake antennaMake, int numSubband, vector<vector<Frequency > > frequencyRange, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > mainBeamEfficiency, EntityRef beamDescriptionUID, float relativeAmplitudeRms, vector<Angle > direction, vector<Angle > minValidDirection, vector<Angle > maxValidDirection, PrimaryBeamDescriptionMod::PrimaryBeamDescription descriptionType, vector<int > imageChannelNumber, vector<Frequency > imageNominalFrequency);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param antennaMake
	    
	 * @param numSubband
	    
	 * @param frequencyRange
	    
	 * @param numReceptor
	    
	 * @param polarizationTypes
	    
	 * @param mainBeamEfficiency
	    
	 * @param beamDescriptionUID
	    
	 * @param relativeAmplitudeRms
	    
	 * @param direction
	    
	 * @param minValidDirection
	    
	 * @param maxValidDirection
	    
	 * @param descriptionType
	    
	 * @param imageChannelNumber
	    
	 * @param imageNominalFrequency
	    
	 */ 
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, AntennaMakeMod::AntennaMake antennaMake, int numSubband, vector<vector<Frequency > > frequencyRange, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > mainBeamEfficiency, EntityRef beamDescriptionUID, float relativeAmplitudeRms, vector<Angle > direction, vector<Angle > minValidDirection, vector<Angle > maxValidDirection, PrimaryBeamDescriptionMod::PrimaryBeamDescription descriptionType, vector<int > imageChannelNumber, vector<Frequency > imageNominalFrequency); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPrimaryBeamRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalPrimaryBeamRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPrimaryBeamRowIDL struct.
	 */
	asdmIDL::CalPrimaryBeamRowIDL *toIDL() const;
	
	/**
	 * Define the content of a CalPrimaryBeamRowIDL struct from the values
	 * found in this row.
	 *
	 * @param x a reference to the CalPrimaryBeamRowIDL struct to be set.
	 *
	 */
	 void toIDL(asdmIDL::CalPrimaryBeamRowIDL& x) const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPrimaryBeamRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (asdmIDL::CalPrimaryBeamRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	std::string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param rowDoc the XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (std::string rowDoc) ;

	/// @cond DISPLAY_PRIVATE	
	////////////////////////////////////////////////////////////
	// binary-deserialization material from an EndianIStream  //
	////////////////////////////////////////////////////////////

	std::map<std::string, CalPrimaryBeamAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianIStream& eis);
void receiverBandFromBin( EndianIStream& eis);
void calDataIdFromBin( EndianIStream& eis);
void calReductionIdFromBin( EndianIStream& eis);
void startValidTimeFromBin( EndianIStream& eis);
void endValidTimeFromBin( EndianIStream& eis);
void antennaMakeFromBin( EndianIStream& eis);
void numSubbandFromBin( EndianIStream& eis);
void frequencyRangeFromBin( EndianIStream& eis);
void numReceptorFromBin( EndianIStream& eis);
void polarizationTypesFromBin( EndianIStream& eis);
void mainBeamEfficiencyFromBin( EndianIStream& eis);
void beamDescriptionUIDFromBin( EndianIStream& eis);
void relativeAmplitudeRmsFromBin( EndianIStream& eis);
void directionFromBin( EndianIStream& eis);
void minValidDirectionFromBin( EndianIStream& eis);
void maxValidDirectionFromBin( EndianIStream& eis);
void descriptionTypeFromBin( EndianIStream& eis);
void imageChannelNumberFromBin( EndianIStream& eis);
void imageNominalFrequencyFromBin( EndianIStream& eis);

	

	 /**
	  * Deserialize a stream of bytes read from an EndianIStream to build a PointingRow.
	  * @param eiss the EndianIStream to be read.
	  * @param table the CalPrimaryBeamTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalPrimaryBeamRow* fromBin(EndianIStream& eis, CalPrimaryBeamTable& table, const std::vector<std::string>& attributesSeq);	 
 
 	 /**
 	  * Parses a string t and assign the result of the parsing to the attribute of name attributeName.
 	  *
 	  * @param attributeName the name of the attribute whose value is going to be defined.
 	  * @param t the string to be parsed into a value given to the attribute of name attributeName.
 	  */
 	 void fromText(const std::string& attributeName, const std::string&  t);
     /// @endcond			

private:
	/**
	 * The table to which this row belongs.
	 */
	CalPrimaryBeamTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a CalPrimaryBeamRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPrimaryBeamRow (CalPrimaryBeamTable &table);

	/**
	 * Create a CalPrimaryBeamRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalPrimaryBeamRow row and a CalPrimaryBeamTable table, the method creates a new
	 * CalPrimaryBeamRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalPrimaryBeamRow (CalPrimaryBeamTable &table, CalPrimaryBeamRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute antennaMake
	
	

	AntennaMakeMod::AntennaMake antennaMake;

	
	
 	

	
	// ===> Attribute numSubband
	
	

	int numSubband;

	
	
 	

	
	// ===> Attribute frequencyRange
	
	

	vector<vector<Frequency > > frequencyRange;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute mainBeamEfficiency
	
	

	vector<double > mainBeamEfficiency;

	
	
 	

	
	// ===> Attribute beamDescriptionUID
	
	

	EntityRef beamDescriptionUID;

	
	
 	

	
	// ===> Attribute relativeAmplitudeRms
	
	

	float relativeAmplitudeRms;

	
	
 	

	
	// ===> Attribute direction
	
	

	vector<Angle > direction;

	
	
 	

	
	// ===> Attribute minValidDirection
	
	

	vector<Angle > minValidDirection;

	
	
 	

	
	// ===> Attribute maxValidDirection
	
	

	vector<Angle > maxValidDirection;

	
	
 	

	
	// ===> Attribute descriptionType
	
	

	PrimaryBeamDescriptionMod::PrimaryBeamDescription descriptionType;

	
	
 	

	
	// ===> Attribute imageChannelNumber
	
	

	vector<int > imageChannelNumber;

	
	
 	

	
	// ===> Attribute imageNominalFrequency
	
	

	vector<Frequency > imageNominalFrequency;

	
	
 	

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
	
	
		

	 

	

	
		

	 

	

	
/*
	////////////////////////////////////////////////////////////
	// binary-deserialization material from an EndianIStream  //
	////////////////////////////////////////////////////////////
	std::map<std::string, CalPrimaryBeamAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianIStream& eis);
void receiverBandFromBin( EndianIStream& eis);
void calDataIdFromBin( EndianIStream& eis);
void calReductionIdFromBin( EndianIStream& eis);
void startValidTimeFromBin( EndianIStream& eis);
void endValidTimeFromBin( EndianIStream& eis);
void antennaMakeFromBin( EndianIStream& eis);
void numSubbandFromBin( EndianIStream& eis);
void frequencyRangeFromBin( EndianIStream& eis);
void numReceptorFromBin( EndianIStream& eis);
void polarizationTypesFromBin( EndianIStream& eis);
void mainBeamEfficiencyFromBin( EndianIStream& eis);
void beamDescriptionUIDFromBin( EndianIStream& eis);
void relativeAmplitudeRmsFromBin( EndianIStream& eis);
void directionFromBin( EndianIStream& eis);
void minValidDirectionFromBin( EndianIStream& eis);
void maxValidDirectionFromBin( EndianIStream& eis);
void descriptionTypeFromBin( EndianIStream& eis);
void imageChannelNumberFromBin( EndianIStream& eis);
void imageNominalFrequencyFromBin( EndianIStream& eis);

	
*/
	
	///////////////////////////////////
	// text-deserialization material //
	///////////////////////////////////
	std::map<std::string, CalPrimaryBeamAttributeFromText> fromTextMethods;
	
void antennaNameFromText (const string & s);
	
	
void receiverBandFromText (const string & s);
	
	
void calDataIdFromText (const string & s);
	
	
void calReductionIdFromText (const string & s);
	
	
void startValidTimeFromText (const string & s);
	
	
void endValidTimeFromText (const string & s);
	
	
void antennaMakeFromText (const string & s);
	
	
void numSubbandFromText (const string & s);
	
	
void frequencyRangeFromText (const string & s);
	
	
void numReceptorFromText (const string & s);
	
	
void polarizationTypesFromText (const string & s);
	
	
void mainBeamEfficiencyFromText (const string & s);
	
	
	
void relativeAmplitudeRmsFromText (const string & s);
	
	
void directionFromText (const string & s);
	
	
void minValidDirectionFromText (const string & s);
	
	
void maxValidDirectionFromText (const string & s);
	
	
void descriptionTypeFromText (const string & s);
	
	
void imageChannelNumberFromText (const string & s);
	
	
void imageNominalFrequencyFromText (const string & s);
	

		
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianIStream to build a PointingRow.
	  * @param eiss the EndianIStream to be read.
	  * @param table the CalPrimaryBeamTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.

	 static CalPrimaryBeamRow* fromBin(EndianIStream& eis, CalPrimaryBeamTable& table, const std::vector<std::string>& attributesSeq);	 
		*/
};

} // End namespace asdm

#endif /* CalPrimaryBeam_CLASS */
