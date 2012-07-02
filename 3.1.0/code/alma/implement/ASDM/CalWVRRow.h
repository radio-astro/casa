
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
 * File CalWVRRow.h
 */
 
#ifndef CalWVRRow_CLASS
#define CalWVRRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalWVRRowIDL;
#endif



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Tag.h>
using  asdm::Tag;

#include <Length.h>
using  asdm::Length;

#include <Temperature.h>
using  asdm::Temperature;

#include <Frequency.h>
using  asdm::Frequency;




	

	

	
#include "CWVRMethod.h"
using namespace WVRMethodMod;
	

	

	

	

	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file CalWVR.h
    \brief Generated from model's revision "1.54", branch "HEAD"
*/

namespace asdm {

//class asdm::CalWVRTable;


// class asdm::CalDataRow;
class CalDataRow;

// class asdm::CalReductionRow;
class CalReductionRow;
	

class CalWVRRow;
typedef void (CalWVRRow::*CalWVRAttributeFromBin) (EndianISStream& eiss);

/**
 * The CalWVRRow class is a row of a CalWVRTable.
 * 
 * Generated from model's revision "1.54", branch "HEAD"
 *
 */
class CalWVRRow {
friend class asdm::CalWVRTable;

public:

	virtual ~CalWVRRow();

	/**
	 * Return the table to which this row belongs.
	 */
	CalWVRTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
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
  		
	
	
	


	
	// ===> Attribute wvrMethod
	
	
	

	
 	/**
 	 * Get wvrMethod.
 	 * @return wvrMethod as WVRMethodMod::WVRMethod
 	 */
 	WVRMethodMod::WVRMethod getWvrMethod() const;
	
 
 	
 	
 	/**
 	 * Set wvrMethod with the specified WVRMethodMod::WVRMethod.
 	 * @param wvrMethod The WVRMethodMod::WVRMethod value to which wvrMethod is to be set.
 	 
 		
 			
 	 */
 	void setWvrMethod (WVRMethodMod::WVRMethod wvrMethod);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute numInputAntennas
	
	
	

	
 	/**
 	 * Get numInputAntennas.
 	 * @return numInputAntennas as int
 	 */
 	int getNumInputAntennas() const;
	
 
 	
 	
 	/**
 	 * Set numInputAntennas with the specified int.
 	 * @param numInputAntennas The int value to which numInputAntennas is to be set.
 	 
 		
 			
 	 */
 	void setNumInputAntennas (int numInputAntennas);
  		
	
	
	


	
	// ===> Attribute inputAntennaNames
	
	
	

	
 	/**
 	 * Get inputAntennaNames.
 	 * @return inputAntennaNames as vector<string >
 	 */
 	vector<string > getInputAntennaNames() const;
	
 
 	
 	
 	/**
 	 * Set inputAntennaNames with the specified vector<string >.
 	 * @param inputAntennaNames The vector<string > value to which inputAntennaNames is to be set.
 	 
 		
 			
 	 */
 	void setInputAntennaNames (vector<string > inputAntennaNames);
  		
	
	
	


	
	// ===> Attribute numChan
	
	
	

	
 	/**
 	 * Get numChan.
 	 * @return numChan as int
 	 */
 	int getNumChan() const;
	
 
 	
 	
 	/**
 	 * Set numChan with the specified int.
 	 * @param numChan The int value to which numChan is to be set.
 	 
 		
 			
 	 */
 	void setNumChan (int numChan);
  		
	
	
	


	
	// ===> Attribute chanFreq
	
	
	

	
 	/**
 	 * Get chanFreq.
 	 * @return chanFreq as vector<Frequency >
 	 */
 	vector<Frequency > getChanFreq() const;
	
 
 	
 	
 	/**
 	 * Set chanFreq with the specified vector<Frequency >.
 	 * @param chanFreq The vector<Frequency > value to which chanFreq is to be set.
 	 
 		
 			
 	 */
 	void setChanFreq (vector<Frequency > chanFreq);
  		
	
	
	


	
	// ===> Attribute chanWidth
	
	
	

	
 	/**
 	 * Get chanWidth.
 	 * @return chanWidth as vector<Frequency >
 	 */
 	vector<Frequency > getChanWidth() const;
	
 
 	
 	
 	/**
 	 * Set chanWidth with the specified vector<Frequency >.
 	 * @param chanWidth The vector<Frequency > value to which chanWidth is to be set.
 	 
 		
 			
 	 */
 	void setChanWidth (vector<Frequency > chanWidth);
  		
	
	
	


	
	// ===> Attribute refTemp
	
	
	

	
 	/**
 	 * Get refTemp.
 	 * @return refTemp as vector<vector<Temperature > >
 	 */
 	vector<vector<Temperature > > getRefTemp() const;
	
 
 	
 	
 	/**
 	 * Set refTemp with the specified vector<vector<Temperature > >.
 	 * @param refTemp The vector<vector<Temperature > > value to which refTemp is to be set.
 	 
 		
 			
 	 */
 	void setRefTemp (vector<vector<Temperature > > refTemp);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute pathCoeff
	
	
	

	
 	/**
 	 * Get pathCoeff.
 	 * @return pathCoeff as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > getPathCoeff() const;
	
 
 	
 	
 	/**
 	 * Set pathCoeff with the specified vector<vector<vector<float > > >.
 	 * @param pathCoeff The vector<vector<vector<float > > > value to which pathCoeff is to be set.
 	 
 		
 			
 	 */
 	void setPathCoeff (vector<vector<vector<float > > > pathCoeff);
  		
	
	
	


	
	// ===> Attribute polyFreqLimits
	
	
	

	
 	/**
 	 * Get polyFreqLimits.
 	 * @return polyFreqLimits as vector<Frequency >
 	 */
 	vector<Frequency > getPolyFreqLimits() const;
	
 
 	
 	
 	/**
 	 * Set polyFreqLimits with the specified vector<Frequency >.
 	 * @param polyFreqLimits The vector<Frequency > value to which polyFreqLimits is to be set.
 	 
 		
 			
 	 */
 	void setPolyFreqLimits (vector<Frequency > polyFreqLimits);
  		
	
	
	


	
	// ===> Attribute wetPath
	
	
	

	
 	/**
 	 * Get wetPath.
 	 * @return wetPath as vector<float >
 	 */
 	vector<float > getWetPath() const;
	
 
 	
 	
 	/**
 	 * Set wetPath with the specified vector<float >.
 	 * @param wetPath The vector<float > value to which wetPath is to be set.
 	 
 		
 			
 	 */
 	void setWetPath (vector<float > wetPath);
  		
	
	
	


	
	// ===> Attribute dryPath
	
	
	

	
 	/**
 	 * Get dryPath.
 	 * @return dryPath as vector<float >
 	 */
 	vector<float > getDryPath() const;
	
 
 	
 	
 	/**
 	 * Set dryPath with the specified vector<float >.
 	 * @param dryPath The vector<float > value to which dryPath is to be set.
 	 
 		
 			
 	 */
 	void setDryPath (vector<float > dryPath);
  		
	
	
	


	
	// ===> Attribute water
	
	
	

	
 	/**
 	 * Get water.
 	 * @return water as Length
 	 */
 	Length getWater() const;
	
 
 	
 	
 	/**
 	 * Set water with the specified Length.
 	 * @param water The Length value to which water is to be set.
 	 
 		
 			
 	 */
 	void setWater (Length water);
  		
	
	
	


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
	 * Compare each mandatory attribute except the autoincrementable one of this CalWVRRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaName
	    
	 * @param calDataId
	    
	 * @param calReductionId
	    
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param wvrMethod
	    
	 * @param numInputAntennas
	    
	 * @param inputAntennaNames
	    
	 * @param numChan
	    
	 * @param chanFreq
	    
	 * @param chanWidth
	    
	 * @param refTemp
	    
	 * @param numPoly
	    
	 * @param pathCoeff
	    
	 * @param polyFreqLimits
	    
	 * @param wetPath
	    
	 * @param dryPath
	    
	 * @param water
	    
	 */ 
	bool compareNoAutoInc(string antennaName, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startValidTime
	    
	 * @param endValidTime
	    
	 * @param wvrMethod
	    
	 * @param numInputAntennas
	    
	 * @param inputAntennaNames
	    
	 * @param numChan
	    
	 * @param chanFreq
	    
	 * @param chanWidth
	    
	 * @param refTemp
	    
	 * @param numPoly
	    
	 * @param pathCoeff
	    
	 * @param polyFreqLimits
	    
	 * @param wetPath
	    
	 * @param dryPath
	    
	 * @param water
	    
	 */ 
	bool compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, WVRMethodMod::WVRMethod wvrMethod, int numInputAntennas, vector<string > inputAntennaNames, int numChan, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<vector<Temperature > > refTemp, int numPoly, vector<vector<vector<float > > > pathCoeff, vector<Frequency > polyFreqLimits, vector<float > wetPath, vector<float > dryPath, Length water); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalWVRRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(CalWVRRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalWVRRowIDL struct.
	 */
	CalWVRRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalWVRRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (CalWVRRowIDL x) ;
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
	CalWVRTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a CalWVRRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalWVRRow (CalWVRTable &table);

	/**
	 * Create a CalWVRRow using a copy constructor mechanism.
	 * <p>
	 * Given a CalWVRRow row and a CalWVRTable table, the method creates a new
	 * CalWVRRow owned by table. Each attribute of the created row is a copy (deep)
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
	 CalWVRRow (CalWVRTable &table, CalWVRRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute startValidTime
	
	

	ArrayTime startValidTime;

	
	
 	

	
	// ===> Attribute endValidTime
	
	

	ArrayTime endValidTime;

	
	
 	

	
	// ===> Attribute wvrMethod
	
	

	WVRMethodMod::WVRMethod wvrMethod;

	
	
 	

	
	// ===> Attribute antennaName
	
	

	string antennaName;

	
	
 	

	
	// ===> Attribute numInputAntennas
	
	

	int numInputAntennas;

	
	
 	

	
	// ===> Attribute inputAntennaNames
	
	

	vector<string > inputAntennaNames;

	
	
 	

	
	// ===> Attribute numChan
	
	

	int numChan;

	
	
 	

	
	// ===> Attribute chanFreq
	
	

	vector<Frequency > chanFreq;

	
	
 	

	
	// ===> Attribute chanWidth
	
	

	vector<Frequency > chanWidth;

	
	
 	

	
	// ===> Attribute refTemp
	
	

	vector<vector<Temperature > > refTemp;

	
	
 	

	
	// ===> Attribute numPoly
	
	

	int numPoly;

	
	
 	

	
	// ===> Attribute pathCoeff
	
	

	vector<vector<vector<float > > > pathCoeff;

	
	
 	

	
	// ===> Attribute polyFreqLimits
	
	

	vector<Frequency > polyFreqLimits;

	
	
 	

	
	// ===> Attribute wetPath
	
	

	vector<float > wetPath;

	
	
 	

	
	// ===> Attribute dryPath
	
	

	vector<float > dryPath;

	
	
 	

	
	// ===> Attribute water
	
	

	Length water;

	
	
 	

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
	map<string, CalWVRAttributeFromBin> fromBinMethods;
void antennaNameFromBin( EndianISStream& eiss);
void calDataIdFromBin( EndianISStream& eiss);
void calReductionIdFromBin( EndianISStream& eiss);
void startValidTimeFromBin( EndianISStream& eiss);
void endValidTimeFromBin( EndianISStream& eiss);
void wvrMethodFromBin( EndianISStream& eiss);
void numInputAntennasFromBin( EndianISStream& eiss);
void inputAntennaNamesFromBin( EndianISStream& eiss);
void numChanFromBin( EndianISStream& eiss);
void chanFreqFromBin( EndianISStream& eiss);
void chanWidthFromBin( EndianISStream& eiss);
void refTempFromBin( EndianISStream& eiss);
void numPolyFromBin( EndianISStream& eiss);
void pathCoeffFromBin( EndianISStream& eiss);
void polyFreqLimitsFromBin( EndianISStream& eiss);
void wetPathFromBin( EndianISStream& eiss);
void dryPathFromBin( EndianISStream& eiss);
void waterFromBin( EndianISStream& eiss);

		
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the CalWVRTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static CalWVRRow* fromBin(EndianISStream& eiss, CalWVRTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* CalWVR_CLASS */
