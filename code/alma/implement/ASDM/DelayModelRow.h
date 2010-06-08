
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
 * File DelayModelRow.h
 */
 
#ifndef DelayModelRow_CLASS
#define DelayModelRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::DelayModelRowIDL;
#endif



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Tag.h>
using  asdm::Tag;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;




	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file DelayModel.h
    \brief Generated from model's revision "1.54", branch "HEAD"
*/

namespace asdm {

//class asdm::DelayModelTable;


// class asdm::AntennaRow;
class AntennaRow;
	

class DelayModelRow;
typedef void (DelayModelRow::*DelayModelAttributeFromBin) (EndianISStream& eiss);

/**
 * The DelayModelRow class is a row of a DelayModelTable.
 * 
 * Generated from model's revision "1.54", branch "HEAD"
 *
 */
class DelayModelRow {
friend class asdm::DelayModelTable;

public:

	virtual ~DelayModelRow();

	/**
	 * Return the table to which this row belongs.
	 */
	DelayModelTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
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
  		
	
	
	


	
	// ===> Attribute timeOrigin
	
	
	

	
 	/**
 	 * Get timeOrigin.
 	 * @return timeOrigin as ArrayTime
 	 */
 	ArrayTime getTimeOrigin() const;
	
 
 	
 	
 	/**
 	 * Set timeOrigin with the specified ArrayTime.
 	 * @param timeOrigin The ArrayTime value to which timeOrigin is to be set.
 	 
 		
 			
 	 */
 	void setTimeOrigin (ArrayTime timeOrigin);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute atmDryDelay
	
	
	

	
 	/**
 	 * Get atmDryDelay.
 	 * @return atmDryDelay as vector<double >
 	 */
 	vector<double > getAtmDryDelay() const;
	
 
 	
 	
 	/**
 	 * Set atmDryDelay with the specified vector<double >.
 	 * @param atmDryDelay The vector<double > value to which atmDryDelay is to be set.
 	 
 		
 			
 	 */
 	void setAtmDryDelay (vector<double > atmDryDelay);
  		
	
	
	


	
	// ===> Attribute atmWetDelay
	
	
	

	
 	/**
 	 * Get atmWetDelay.
 	 * @return atmWetDelay as vector<double >
 	 */
 	vector<double > getAtmWetDelay() const;
	
 
 	
 	
 	/**
 	 * Set atmWetDelay with the specified vector<double >.
 	 * @param atmWetDelay The vector<double > value to which atmWetDelay is to be set.
 	 
 		
 			
 	 */
 	void setAtmWetDelay (vector<double > atmWetDelay);
  		
	
	
	


	
	// ===> Attribute clockDelay
	
	
	

	
 	/**
 	 * Get clockDelay.
 	 * @return clockDelay as vector<double >
 	 */
 	vector<double > getClockDelay() const;
	
 
 	
 	
 	/**
 	 * Set clockDelay with the specified vector<double >.
 	 * @param clockDelay The vector<double > value to which clockDelay is to be set.
 	 
 		
 			
 	 */
 	void setClockDelay (vector<double > clockDelay);
  		
	
	
	


	
	// ===> Attribute geomDelay
	
	
	

	
 	/**
 	 * Get geomDelay.
 	 * @return geomDelay as vector<double >
 	 */
 	vector<double > getGeomDelay() const;
	
 
 	
 	
 	/**
 	 * Set geomDelay with the specified vector<double >.
 	 * @param geomDelay The vector<double > value to which geomDelay is to be set.
 	 
 		
 			
 	 */
 	void setGeomDelay (vector<double > geomDelay);
  		
	
	
	


	
	// ===> Attribute dispDelay, which is optional
	
	
	
	/**
	 * The attribute dispDelay is optional. Return true if this attribute exists.
	 * @return true if and only if the dispDelay attribute exists. 
	 */
	bool isDispDelayExists() const;
	

	
 	/**
 	 * Get dispDelay, which is optional.
 	 * @return dispDelay as vector<double >
 	 * @throws IllegalAccessException If dispDelay does not exist.
 	 */
 	vector<double > getDispDelay() const;
	
 
 	
 	
 	/**
 	 * Set dispDelay with the specified vector<double >.
 	 * @param dispDelay The vector<double > value to which dispDelay is to be set.
 	 
 		
 	 */
 	void setDispDelay (vector<double > dispDelay);
		
	
	
	
	/**
	 * Mark dispDelay, which is an optional field, as non-existent.
	 */
	void clearDispDelay ();
	


	
	// ===> Attribute groupDelay, which is optional
	
	
	
	/**
	 * The attribute groupDelay is optional. Return true if this attribute exists.
	 * @return true if and only if the groupDelay attribute exists. 
	 */
	bool isGroupDelayExists() const;
	

	
 	/**
 	 * Get groupDelay, which is optional.
 	 * @return groupDelay as vector<double >
 	 * @throws IllegalAccessException If groupDelay does not exist.
 	 */
 	vector<double > getGroupDelay() const;
	
 
 	
 	
 	/**
 	 * Set groupDelay with the specified vector<double >.
 	 * @param groupDelay The vector<double > value to which groupDelay is to be set.
 	 
 		
 	 */
 	void setGroupDelay (vector<double > groupDelay);
		
	
	
	
	/**
	 * Mark groupDelay, which is an optional field, as non-existent.
	 */
	void clearGroupDelay ();
	


	
	// ===> Attribute phaseDelay, which is optional
	
	
	
	/**
	 * The attribute phaseDelay is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseDelay attribute exists. 
	 */
	bool isPhaseDelayExists() const;
	

	
 	/**
 	 * Get phaseDelay, which is optional.
 	 * @return phaseDelay as vector<double >
 	 * @throws IllegalAccessException If phaseDelay does not exist.
 	 */
 	vector<double > getPhaseDelay() const;
	
 
 	
 	
 	/**
 	 * Set phaseDelay with the specified vector<double >.
 	 * @param phaseDelay The vector<double > value to which phaseDelay is to be set.
 	 
 		
 	 */
 	void setPhaseDelay (vector<double > phaseDelay);
		
	
	
	
	/**
	 * Mark phaseDelay, which is an optional field, as non-existent.
	 */
	void clearPhaseDelay ();
	


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
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this DelayModelRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaId
	    
	 * @param timeInterval
	    
	 * @param timeOrigin
	    
	 * @param numPoly
	    
	 * @param atmDryDelay
	    
	 * @param atmWetDelay
	    
	 * @param clockDelay
	    
	 * @param geomDelay
	    
	 */ 
	bool compareNoAutoInc(Tag antennaId, ArrayTimeInterval timeInterval, ArrayTime timeOrigin, int numPoly, vector<double > atmDryDelay, vector<double > atmWetDelay, vector<double > clockDelay, vector<double > geomDelay);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param timeOrigin
	    
	 * @param numPoly
	    
	 * @param atmDryDelay
	    
	 * @param atmWetDelay
	    
	 * @param clockDelay
	    
	 * @param geomDelay
	    
	 */ 
	bool compareRequiredValue(ArrayTime timeOrigin, int numPoly, vector<double > atmDryDelay, vector<double > atmWetDelay, vector<double > clockDelay, vector<double > geomDelay); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the DelayModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(DelayModelRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a DelayModelRowIDL struct.
	 */
	DelayModelRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct DelayModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (DelayModelRowIDL x) ;
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
	DelayModelTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a DelayModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	DelayModelRow (DelayModelTable &table);

	/**
	 * Create a DelayModelRow using a copy constructor mechanism.
	 * <p>
	 * Given a DelayModelRow row and a DelayModelTable table, the method creates a new
	 * DelayModelRow owned by table. Each attribute of the created row is a copy (deep)
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
	 DelayModelRow (DelayModelTable &table, DelayModelRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute timeOrigin
	
	

	ArrayTime timeOrigin;

	
	
 	

	
	// ===> Attribute numPoly
	
	

	int numPoly;

	
	
 	

	
	// ===> Attribute atmDryDelay
	
	

	vector<double > atmDryDelay;

	
	
 	

	
	// ===> Attribute atmWetDelay
	
	

	vector<double > atmWetDelay;

	
	
 	

	
	// ===> Attribute clockDelay
	
	

	vector<double > clockDelay;

	
	
 	

	
	// ===> Attribute geomDelay
	
	

	vector<double > geomDelay;

	
	
 	

	
	// ===> Attribute dispDelay, which is optional
	
	
	bool dispDelayExists;
	

	vector<double > dispDelay;

	
	
 	

	
	// ===> Attribute groupDelay, which is optional
	
	
	bool groupDelayExists;
	

	vector<double > groupDelay;

	
	
 	

	
	// ===> Attribute phaseDelay, which is optional
	
	
	bool phaseDelayExists;
	

	vector<double > phaseDelay;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	Tag antennaId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, DelayModelAttributeFromBin> fromBinMethods;
void antennaIdFromBin( EndianISStream& eiss);
void timeIntervalFromBin( EndianISStream& eiss);
void timeOriginFromBin( EndianISStream& eiss);
void numPolyFromBin( EndianISStream& eiss);
void atmDryDelayFromBin( EndianISStream& eiss);
void atmWetDelayFromBin( EndianISStream& eiss);
void clockDelayFromBin( EndianISStream& eiss);
void geomDelayFromBin( EndianISStream& eiss);

void dispDelayFromBin( EndianISStream& eiss);
void groupDelayFromBin( EndianISStream& eiss);
void phaseDelayFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the DelayModelTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static DelayModelRow* fromBin(EndianISStream& eiss, DelayModelTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* DelayModel_CLASS */
