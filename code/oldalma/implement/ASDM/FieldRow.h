
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
 * File FieldRow.h
 */
 
#ifndef FieldRow_CLASS
#define FieldRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::FieldRowIDL;
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

/*\file Field.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::FieldTable;


// class asdm::FieldRow;
class FieldRow;

// class asdm::EphemerisRow;
class EphemerisRow;

// class asdm::SourceRow;
class SourceRow;
	

/**
 * The FieldRow class is a row of a FieldTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class FieldRow {
friend class asdm::FieldTable;

public:

	virtual ~FieldRow();

	/**
	 * Return the table to which this row belongs.
	 */
	FieldTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FieldRowIDL struct.
	 */
	FieldRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FieldRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (FieldRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute fieldId
	
	
	

	
 	/**
 	 * Get fieldId.
 	 * @return fieldId as Tag
 	 */
 	Tag getFieldId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute fieldName
	
	
	

	
 	/**
 	 * Get fieldName.
 	 * @return fieldName as string
 	 */
 	string getFieldName() const;
	
 
 	
 	
 	/**
 	 * Set fieldName with the specified string.
 	 * @param fieldName The string value to which fieldName is to be set.
 	 
 		
 			
 	 */
 	void setFieldName (string fieldName);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute time
	
	
	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime getTime() const;
	
 
 	
 	
 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 		
 			
 	 */
 	void setTime (ArrayTime time);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute delayDir
	
	
	

	
 	/**
 	 * Get delayDir.
 	 * @return delayDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getDelayDir() const;
	
 
 	
 	
 	/**
 	 * Set delayDir with the specified vector<vector<Angle > >.
 	 * @param delayDir The vector<vector<Angle > > value to which delayDir is to be set.
 	 
 		
 			
 	 */
 	void setDelayDir (vector<vector<Angle > > delayDir);
  		
	
	
	


	
	// ===> Attribute phaseDir
	
	
	

	
 	/**
 	 * Get phaseDir.
 	 * @return phaseDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getPhaseDir() const;
	
 
 	
 	
 	/**
 	 * Set phaseDir with the specified vector<vector<Angle > >.
 	 * @param phaseDir The vector<vector<Angle > > value to which phaseDir is to be set.
 	 
 		
 			
 	 */
 	void setPhaseDir (vector<vector<Angle > > phaseDir);
  		
	
	
	


	
	// ===> Attribute referenceDir
	
	
	

	
 	/**
 	 * Get referenceDir.
 	 * @return referenceDir as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > getReferenceDir() const;
	
 
 	
 	
 	/**
 	 * Set referenceDir with the specified vector<vector<Angle > >.
 	 * @param referenceDir The vector<vector<Angle > > value to which referenceDir is to be set.
 	 
 		
 			
 	 */
 	void setReferenceDir (vector<vector<Angle > > referenceDir);
  		
	
	
	


	
	// ===> Attribute assocNature, which is optional
	
	
	
	/**
	 * The attribute assocNature is optional. Return true if this attribute exists.
	 * @return true if and only if the assocNature attribute exists. 
	 */
	bool isAssocNatureExists() const;
	

	
 	/**
 	 * Get assocNature, which is optional.
 	 * @return assocNature as string
 	 * @throws IllegalAccessException If assocNature does not exist.
 	 */
 	string getAssocNature() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set assocNature with the specified string.
 	 * @param assocNature The string value to which assocNature is to be set.
 	 
 		
 	 */
 	void setAssocNature (string assocNature);
		
	
	
	
	/**
	 * Mark assocNature, which is an optional field, as non-existent.
	 */
	void clearAssocNature ();
	


	
	// ===> Attribute flagRow
	
	
	

	
 	/**
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool getFlagRow() const;
	
 
 	
 	
 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 		
 			
 	 */
 	void setFlagRow (bool flagRow);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute assocFieldId, which is optional
	
	
	
	/**
	 * The attribute assocFieldId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocFieldId attribute exists. 
	 */
	bool isAssocFieldIdExists() const;
	

	
 	/**
 	 * Get assocFieldId, which is optional.
 	 * @return assocFieldId as vector<Tag> 
 	 * @throws IllegalAccessException If assocFieldId does not exist.
 	 */
 	vector<Tag>  getAssocFieldId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set assocFieldId with the specified vector<Tag> .
 	 * @param assocFieldId The vector<Tag>  value to which assocFieldId is to be set.
 	 
 		
 	 */
 	void setAssocFieldId (vector<Tag>  assocFieldId);
		
	
	
	
	/**
	 * Mark assocFieldId, which is an optional field, as non-existent.
	 */
	void clearAssocFieldId ();
	


	
	// ===> Attribute ephemerisId, which is optional
	
	
	
	/**
	 * The attribute ephemerisId is optional. Return true if this attribute exists.
	 * @return true if and only if the ephemerisId attribute exists. 
	 */
	bool isEphemerisIdExists() const;
	

	
 	/**
 	 * Get ephemerisId, which is optional.
 	 * @return ephemerisId as Tag
 	 * @throws IllegalAccessException If ephemerisId does not exist.
 	 */
 	Tag getEphemerisId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set ephemerisId with the specified Tag.
 	 * @param ephemerisId The Tag value to which ephemerisId is to be set.
 	 
 		
 	 */
 	void setEphemerisId (Tag ephemerisId);
		
	
	
	
	/**
	 * Mark ephemerisId, which is an optional field, as non-existent.
	 */
	void clearEphemerisId ();
	


	
	// ===> Attribute sourceId, which is optional
	
	
	
	/**
	 * The attribute sourceId is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceId attribute exists. 
	 */
	bool isSourceIdExists() const;
	

	
 	/**
 	 * Get sourceId, which is optional.
 	 * @return sourceId as int
 	 * @throws IllegalAccessException If sourceId does not exist.
 	 */
 	int getSourceId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 		
 	 */
 	void setSourceId (int sourceId);
		
	
	
	
	/**
	 * Mark sourceId, which is an optional field, as non-existent.
	 */
	void clearSourceId ();
	


	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set assocFieldId[i] with the specified Tag.
 	 * @param i The index in assocFieldId where to set the Tag value.
 	 * @param assocFieldId The Tag value to which assocFieldId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void setAssocFieldId (int i, Tag assocFieldId)  ;
 			
	

	
		 
/**
 * Append a Tag to assocFieldId.
 * @param id the Tag to be appended to assocFieldId
 */
 void addAssocFieldId(Tag id); 

/**
 * Append a vector of Tag to assocFieldId.
 * @param id an array of Tag to be appended to assocFieldId
 */
 void addAssocFieldId(const vector<Tag> & id); 
 

 /**
  * Returns the Tag stored in assocFieldId at position i.
  * @param i the position in assocFieldId where the Tag is retrieved.
  * @return the Tag stored at position i in assocFieldId.
  */
 const Tag getAssocFieldId(int i);
 
 /**
  * Returns the FieldRow linked to this row via the tag stored in assocFieldId
  * at position i.
  * @param i the position in assocFieldId.
  * @return a pointer on a FieldRow whose key (a Tag) is equal to the Tag stored at position
  * i in the assocFieldId. 
  */
 FieldRow* getField(int i); 
 
 /**
  * Returns the vector of FieldRow* linked to this row via the Tags stored in assocFieldId
  * @return an array of pointers on FieldRow.
  */
 vector<FieldRow *> getFields(); 
  

	

	

	
		
	/**
	 * ephemerisId pointer to the row in the Ephemeris table having Ephemeris.ephemerisId == ephemerisId
	 * @return a EphemerisRow*
	 * 
	 
	 * throws IllegalAccessException
	 
	 */
	 EphemerisRow* getEphemerisUsingEphemerisId();
	 

	

	

	
		
	// ===> Slice link from a row of Field table to a collection of row of Source table.
	
	/**
	 * Get the collection of row in the Source table having sourceId == this.sourceId
	 * 
	 * @return a vector of SourceRow *
	 */
	vector <SourceRow *> getSources();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this FieldRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(string fieldName, string code, ArrayTime time, int numPoly, vector<vector<Angle > > delayDir, vector<vector<Angle > > phaseDir, vector<vector<Angle > > referenceDir, bool flagRow);
	
	

	
	bool compareRequiredValue(string fieldName, string code, ArrayTime time, int numPoly, vector<vector<Angle > > delayDir, vector<vector<Angle > > phaseDir, vector<vector<Angle > > referenceDir, bool flagRow); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FieldRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(FieldRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	FieldTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a FieldRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FieldRow (FieldTable &table);

	/**
	 * Create a FieldRow using a copy constructor mechanism.
	 * <p>
	 * Given a FieldRow row and a FieldTable table, the method creates a new
	 * FieldRow owned by table. Each attribute of the created row is a copy (deep)
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
	 FieldRow (FieldTable &table, FieldRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute fieldId
	
	

	Tag fieldId;

	
	
 	
 	/**
 	 * Set fieldId with the specified Tag value.
 	 * @param fieldId The Tag value to which fieldId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setFieldId (Tag fieldId);
  		
	

	
	// ===> Attribute fieldName
	
	

	string fieldName;

	
	
 	

	
	// ===> Attribute code
	
	

	string code;

	
	
 	

	
	// ===> Attribute time
	
	

	ArrayTime time;

	
	
 	

	
	// ===> Attribute numPoly
	
	

	int numPoly;

	
	
 	

	
	// ===> Attribute delayDir
	
	

	vector<vector<Angle > > delayDir;

	
	
 	

	
	// ===> Attribute phaseDir
	
	

	vector<vector<Angle > > phaseDir;

	
	
 	

	
	// ===> Attribute referenceDir
	
	

	vector<vector<Angle > > referenceDir;

	
	
 	

	
	// ===> Attribute assocNature, which is optional
	
	
	bool assocNatureExists;
	

	string assocNature;

	
	
 	

	
	// ===> Attribute flagRow
	
	

	bool flagRow;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute assocFieldId, which is optional
	
	
	bool assocFieldIdExists;
	

	vector<Tag>  assocFieldId;

	
	
 	

	
	// ===> Attribute ephemerisId, which is optional
	
	
	bool ephemerisIdExists;
	

	Tag ephemerisId;

	
	
 	

	
	// ===> Attribute sourceId, which is optional
	
	
	bool sourceIdExists;
	

	int sourceId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		


	

	
		

	 

	

	
		


	


};

} // End namespace asdm

#endif /* Field_CLASS */
