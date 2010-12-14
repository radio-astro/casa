
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
 * File AlmaRadiometerRow.h
 */
 
#ifndef AlmaRadiometerRow_CLASS
#define AlmaRadiometerRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::AlmaRadiometerRowIDL;
#endif






#include <Tag.h>
using  asdm::Tag;




	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file AlmaRadiometer.h
    \brief Generated from model's revision "1.55", branch "HEAD"
*/

namespace asdm {

//class asdm::AlmaRadiometerTable;


// class asdm::SpectralWindowRow;
class SpectralWindowRow;
	

class AlmaRadiometerRow;
typedef void (AlmaRadiometerRow::*AlmaRadiometerAttributeFromBin) (EndianISStream& eiss);

/**
 * The AlmaRadiometerRow class is a row of a AlmaRadiometerTable.
 * 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 */
class AlmaRadiometerRow {
friend class asdm::AlmaRadiometerTable;

public:

	virtual ~AlmaRadiometerRow();

	/**
	 * Return the table to which this row belongs.
	 */
	AlmaRadiometerTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute almaRadiometerId
	
	
	

	
 	/**
 	 * Get almaRadiometerId.
 	 * @return almaRadiometerId as Tag
 	 */
 	Tag getAlmaRadiometerId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute numAntenna, which is optional
	
	
	
	/**
	 * The attribute numAntenna is optional. Return true if this attribute exists.
	 * @return true if and only if the numAntenna attribute exists. 
	 */
	bool isNumAntennaExists() const;
	

	
 	/**
 	 * Get numAntenna, which is optional.
 	 * @return numAntenna as int
 	 * @throws IllegalAccessException If numAntenna does not exist.
 	 */
 	int getNumAntenna() const;
	
 
 	
 	
 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 		
 	 */
 	void setNumAntenna (int numAntenna);
		
	
	
	
	/**
	 * Mark numAntenna, which is an optional field, as non-existent.
	 */
	void clearNumAntenna ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute spectralWindowId, which is optional
	
	
	
	/**
	 * The attribute spectralWindowId is optional. Return true if this attribute exists.
	 * @return true if and only if the spectralWindowId attribute exists. 
	 */
	bool isSpectralWindowIdExists() const;
	

	
 	/**
 	 * Get spectralWindowId, which is optional.
 	 * @return spectralWindowId as vector<Tag> 
 	 * @throws IllegalAccessException If spectralWindowId does not exist.
 	 */
 	vector<Tag>  getSpectralWindowId() const;
	
 
 	
 	
 	/**
 	 * Set spectralWindowId with the specified vector<Tag> .
 	 * @param spectralWindowId The vector<Tag>  value to which spectralWindowId is to be set.
 	 
 		
 	 */
 	void setSpectralWindowId (vector<Tag>  spectralWindowId);
		
	
	
	
	/**
	 * Mark spectralWindowId, which is an optional field, as non-existent.
	 */
	void clearSpectralWindowId ();
	


	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set spectralWindowId[i] with the specified Tag.
 	 * @param i The index in spectralWindowId where to set the Tag value.
 	 * @param spectralWindowId The Tag value to which spectralWindowId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void setSpectralWindowId (int i, Tag spectralWindowId)  ;
 			
	

	
		 
/**
 * Append a Tag to spectralWindowId.
 * @param id the Tag to be appended to spectralWindowId
 */
 void addSpectralWindowId(Tag id); 

/**
 * Append a vector of Tag to spectralWindowId.
 * @param id an array of Tag to be appended to spectralWindowId
 */
 void addSpectralWindowId(const vector<Tag> & id); 
 

 /**
  * Returns the Tag stored in spectralWindowId at position i.
  * @param i the position in spectralWindowId where the Tag is retrieved.
  * @return the Tag stored at position i in spectralWindowId.
  */
 const Tag getSpectralWindowId(int i);
 
 /**
  * Returns the SpectralWindowRow linked to this row via the tag stored in spectralWindowId
  * at position i.
  * @param i the position in spectralWindowId.
  * @return a pointer on a SpectralWindowRow whose key (a Tag) is equal to the Tag stored at position
  * i in the spectralWindowId. 
  */
 SpectralWindowRow* getSpectralWindowUsingSpectralWindowId(int i); 
 
 /**
  * Returns the vector of SpectralWindowRow* linked to this row via the Tags stored in spectralWindowId
  * @return an array of pointers on SpectralWindowRow.
  */
 vector<SpectralWindowRow *> getSpectralWindowsUsingSpectralWindowId(); 
  

	

	
	
	

		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the AlmaRadiometerRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(AlmaRadiometerRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a AlmaRadiometerRowIDL struct.
	 */
	AlmaRadiometerRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct AlmaRadiometerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (AlmaRadiometerRowIDL x) ;
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
	AlmaRadiometerTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a AlmaRadiometerRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	AlmaRadiometerRow (AlmaRadiometerTable &table);

	/**
	 * Create a AlmaRadiometerRow using a copy constructor mechanism.
	 * <p>
	 * Given a AlmaRadiometerRow row and a AlmaRadiometerTable table, the method creates a new
	 * AlmaRadiometerRow owned by table. Each attribute of the created row is a copy (deep)
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
	 AlmaRadiometerRow (AlmaRadiometerTable &table, AlmaRadiometerRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute almaRadiometerId
	
	

	Tag almaRadiometerId;

	
	
 	
 	/**
 	 * Set almaRadiometerId with the specified Tag value.
 	 * @param almaRadiometerId The Tag value to which almaRadiometerId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setAlmaRadiometerId (Tag almaRadiometerId);
  		
	

	
	// ===> Attribute numAntenna, which is optional
	
	
	bool numAntennaExists;
	

	int numAntenna;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute spectralWindowId, which is optional
	
	
	bool spectralWindowIdExists;
	

	vector<Tag>  spectralWindowId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		


	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, AlmaRadiometerAttributeFromBin> fromBinMethods;
void almaRadiometerIdFromBin( EndianISStream& eiss);

void numAntennaFromBin( EndianISStream& eiss);
void spectralWindowIdFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the AlmaRadiometerTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static AlmaRadiometerRow* fromBin(EndianISStream& eiss, AlmaRadiometerTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* AlmaRadiometer_CLASS */
