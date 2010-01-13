
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
 * File FocusModelRow.h
 */
 
#ifndef FocusModelRow_CLASS
#define FocusModelRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::FocusModelRowIDL;
#endif



#include <Tag.h>
using  asdm::Tag;




	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file FocusModel.h
    \brief Generated from model's revision "1.53", branch "HEAD"
*/

namespace asdm {

//class asdm::FocusModelTable;


// class asdm::AntennaRow;
class AntennaRow;

// class asdm::FocusModelRow;
class FocusModelRow;
	

class FocusModelRow;
typedef void (FocusModelRow::*FocusModelAttributeFromBin) (EndianISStream& eiss);

/**
 * The FocusModelRow class is a row of a FocusModelTable.
 * 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 */
class FocusModelRow {
friend class asdm::FocusModelTable;

public:

	virtual ~FocusModelRow();

	/**
	 * Return the table to which this row belongs.
	 */
	FocusModelTable &getTable() const;
	
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute focusModelId
	
	
	

	
 	/**
 	 * Get focusModelId.
 	 * @return focusModelId as int
 	 */
 	int getFocusModelId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute polarizationType
	
	
	

	
 	/**
 	 * Get polarizationType.
 	 * @return polarizationType as PolarizationTypeMod::PolarizationType
 	 */
 	PolarizationTypeMod::PolarizationType getPolarizationType() const;
	
 
 	
 	
 	/**
 	 * Set polarizationType with the specified PolarizationTypeMod::PolarizationType.
 	 * @param polarizationType The PolarizationTypeMod::PolarizationType value to which polarizationType is to be set.
 	 
 		
 			
 	 */
 	void setPolarizationType (PolarizationTypeMod::PolarizationType polarizationType);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute assocNature
	
	
	

	
 	/**
 	 * Get assocNature.
 	 * @return assocNature as string
 	 */
 	string getAssocNature() const;
	
 
 	
 	
 	/**
 	 * Set assocNature with the specified string.
 	 * @param assocNature The string value to which assocNature is to be set.
 	 
 		
 			
 	 */
 	void setAssocNature (string assocNature);
  		
	
	
	


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
  		
	
	
	


	
	// ===> Attribute assocFocusModelId
	
	
	

	
 	/**
 	 * Get assocFocusModelId.
 	 * @return assocFocusModelId as int
 	 */
 	int getAssocFocusModelId() const;
	
 
 	
 	
 	/**
 	 * Set assocFocusModelId with the specified int.
 	 * @param assocFocusModelId The int value to which assocFocusModelId is to be set.
 	 
 		
 			
 	 */
 	void setAssocFocusModelId (int assocFocusModelId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	

	
		
	// ===> Slice link from a row of FocusModel table to a collection of row of FocusModel table.
	
	/**
	 * Get the collection of row in the FocusModel table having focusModelId == this.focusModelId
	 * 
	 * @return a vector of FocusModelRow *
	 */
	vector <FocusModelRow *> getFocusModels();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this FocusModelRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaId
	    
	 * @param polarizationType
	    
	 * @param receiverBand
	    
	 * @param numCoeff
	    
	 * @param coeffName
	    
	 * @param coeffFormula
	    
	 * @param coeffVal
	    
	 * @param assocNature
	    
	 * @param assocFocusModelId
	    
	 */ 
	bool compareNoAutoInc(Tag antennaId, PolarizationTypeMod::PolarizationType polarizationType, ReceiverBandMod::ReceiverBand receiverBand, int numCoeff, vector<string > coeffName, vector<string > coeffFormula, vector<float > coeffVal, string assocNature, int assocFocusModelId);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param polarizationType
	    
	 * @param receiverBand
	    
	 * @param numCoeff
	    
	 * @param coeffName
	    
	 * @param coeffFormula
	    
	 * @param coeffVal
	    
	 * @param assocNature
	    
	 * @param assocFocusModelId
	    
	 */ 
	bool compareRequiredValue(PolarizationTypeMod::PolarizationType polarizationType, ReceiverBandMod::ReceiverBand receiverBand, int numCoeff, vector<string > coeffName, vector<string > coeffFormula, vector<float > coeffVal, string assocNature, int assocFocusModelId); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FocusModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(FocusModelRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	FocusModelTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a FocusModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FocusModelRow (FocusModelTable &table);

	/**
	 * Create a FocusModelRow using a copy constructor mechanism.
	 * <p>
	 * Given a FocusModelRow row and a FocusModelTable table, the method creates a new
	 * FocusModelRow owned by table. Each attribute of the created row is a copy (deep)
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
	 FocusModelRow (FocusModelTable &table, FocusModelRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute focusModelId
	
	

	int focusModelId;

	
	
 	
 	/**
 	 * Set focusModelId with the specified int value.
 	 * @param focusModelId The int value to which focusModelId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setFocusModelId (int focusModelId);
  		
	

	
	// ===> Attribute polarizationType
	
	

	PolarizationTypeMod::PolarizationType polarizationType;

	
	
 	

	
	// ===> Attribute receiverBand
	
	

	ReceiverBandMod::ReceiverBand receiverBand;

	
	
 	

	
	// ===> Attribute numCoeff
	
	

	int numCoeff;

	
	
 	

	
	// ===> Attribute coeffName
	
	

	vector<string > coeffName;

	
	
 	

	
	// ===> Attribute coeffFormula
	
	

	vector<string > coeffFormula;

	
	
 	

	
	// ===> Attribute coeffVal
	
	

	vector<float > coeffVal;

	
	
 	

	
	// ===> Attribute assocNature
	
	

	string assocNature;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	Tag antennaId;

	
	
 	

	
	// ===> Attribute assocFocusModelId
	
	

	int assocFocusModelId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		


	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, FocusModelAttributeFromBin> fromBinMethods;
void antennaIdFromBin( EndianISStream& eiss);
void focusModelIdFromBin( EndianISStream& eiss);
void polarizationTypeFromBin( EndianISStream& eiss);
void receiverBandFromBin( EndianISStream& eiss);
void numCoeffFromBin( EndianISStream& eiss);
void coeffNameFromBin( EndianISStream& eiss);
void coeffFormulaFromBin( EndianISStream& eiss);
void coeffValFromBin( EndianISStream& eiss);
void assocNatureFromBin( EndianISStream& eiss);
void assocFocusModelIdFromBin( EndianISStream& eiss);

		

#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FocusModelRowIDL struct.
	 */
	FocusModelRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FocusModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (FocusModelRowIDL x) ;
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
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the FocusModelTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static FocusModelRow* fromBin(EndianISStream& eiss, FocusModelTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* FocusModel_CLASS */
