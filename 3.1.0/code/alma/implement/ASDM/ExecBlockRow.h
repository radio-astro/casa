
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
 * File ExecBlockRow.h
 */
 
#ifndef ExecBlockRow_CLASS
#define ExecBlockRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::ExecBlockRowIDL;
#endif



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;

#include <Length.h>
using  asdm::Length;

#include <EntityRef.h>
using  asdm::EntityRef;




	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file ExecBlock.h
    \brief Generated from model's revision "1.54", branch "HEAD"
*/

namespace asdm {

//class asdm::ExecBlockTable;


// class asdm::AntennaRow;
class AntennaRow;

// class asdm::SBSummaryRow;
class SBSummaryRow;
	

class ExecBlockRow;
typedef void (ExecBlockRow::*ExecBlockAttributeFromBin) (EndianISStream& eiss);

/**
 * The ExecBlockRow class is a row of a ExecBlockTable.
 * 
 * Generated from model's revision "1.54", branch "HEAD"
 *
 */
class ExecBlockRow {
friend class asdm::ExecBlockTable;

public:

	virtual ~ExecBlockRow();

	/**
	 * Return the table to which this row belongs.
	 */
	ExecBlockTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute execBlockId
	
	
	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag getExecBlockId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute startTime
	
	
	

	
 	/**
 	 * Get startTime.
 	 * @return startTime as ArrayTime
 	 */
 	ArrayTime getStartTime() const;
	
 
 	
 	
 	/**
 	 * Set startTime with the specified ArrayTime.
 	 * @param startTime The ArrayTime value to which startTime is to be set.
 	 
 		
 			
 	 */
 	void setStartTime (ArrayTime startTime);
  		
	
	
	


	
	// ===> Attribute endTime
	
	
	

	
 	/**
 	 * Get endTime.
 	 * @return endTime as ArrayTime
 	 */
 	ArrayTime getEndTime() const;
	
 
 	
 	
 	/**
 	 * Set endTime with the specified ArrayTime.
 	 * @param endTime The ArrayTime value to which endTime is to be set.
 	 
 		
 			
 	 */
 	void setEndTime (ArrayTime endTime);
  		
	
	
	


	
	// ===> Attribute execBlockNum
	
	
	

	
 	/**
 	 * Get execBlockNum.
 	 * @return execBlockNum as int
 	 */
 	int getExecBlockNum() const;
	
 
 	
 	
 	/**
 	 * Set execBlockNum with the specified int.
 	 * @param execBlockNum The int value to which execBlockNum is to be set.
 	 
 		
 			
 	 */
 	void setExecBlockNum (int execBlockNum);
  		
	
	
	


	
	// ===> Attribute execBlockUID
	
	
	

	
 	/**
 	 * Get execBlockUID.
 	 * @return execBlockUID as EntityRef
 	 */
 	EntityRef getExecBlockUID() const;
	
 
 	
 	
 	/**
 	 * Set execBlockUID with the specified EntityRef.
 	 * @param execBlockUID The EntityRef value to which execBlockUID is to be set.
 	 
 		
 			
 	 */
 	void setExecBlockUID (EntityRef execBlockUID);
  		
	
	
	


	
	// ===> Attribute projectId
	
	
	

	
 	/**
 	 * Get projectId.
 	 * @return projectId as EntityRef
 	 */
 	EntityRef getProjectId() const;
	
 
 	
 	
 	/**
 	 * Set projectId with the specified EntityRef.
 	 * @param projectId The EntityRef value to which projectId is to be set.
 	 
 		
 			
 	 */
 	void setProjectId (EntityRef projectId);
  		
	
	
	


	
	// ===> Attribute configName
	
	
	

	
 	/**
 	 * Get configName.
 	 * @return configName as string
 	 */
 	string getConfigName() const;
	
 
 	
 	
 	/**
 	 * Set configName with the specified string.
 	 * @param configName The string value to which configName is to be set.
 	 
 		
 			
 	 */
 	void setConfigName (string configName);
  		
	
	
	


	
	// ===> Attribute telescopeName
	
	
	

	
 	/**
 	 * Get telescopeName.
 	 * @return telescopeName as string
 	 */
 	string getTelescopeName() const;
	
 
 	
 	
 	/**
 	 * Set telescopeName with the specified string.
 	 * @param telescopeName The string value to which telescopeName is to be set.
 	 
 		
 			
 	 */
 	void setTelescopeName (string telescopeName);
  		
	
	
	


	
	// ===> Attribute observerName
	
	
	

	
 	/**
 	 * Get observerName.
 	 * @return observerName as string
 	 */
 	string getObserverName() const;
	
 
 	
 	
 	/**
 	 * Set observerName with the specified string.
 	 * @param observerName The string value to which observerName is to be set.
 	 
 		
 			
 	 */
 	void setObserverName (string observerName);
  		
	
	
	


	
	// ===> Attribute observingLog
	
	
	

	
 	/**
 	 * Get observingLog.
 	 * @return observingLog as string
 	 */
 	string getObservingLog() const;
	
 
 	
 	
 	/**
 	 * Set observingLog with the specified string.
 	 * @param observingLog The string value to which observingLog is to be set.
 	 
 		
 			
 	 */
 	void setObservingLog (string observingLog);
  		
	
	
	


	
	// ===> Attribute sessionReference
	
	
	

	
 	/**
 	 * Get sessionReference.
 	 * @return sessionReference as string
 	 */
 	string getSessionReference() const;
	
 
 	
 	
 	/**
 	 * Set sessionReference with the specified string.
 	 * @param sessionReference The string value to which sessionReference is to be set.
 	 
 		
 			
 	 */
 	void setSessionReference (string sessionReference);
  		
	
	
	


	
	// ===> Attribute sbSummary
	
	
	

	
 	/**
 	 * Get sbSummary.
 	 * @return sbSummary as EntityRef
 	 */
 	EntityRef getSbSummary() const;
	
 
 	
 	
 	/**
 	 * Set sbSummary with the specified EntityRef.
 	 * @param sbSummary The EntityRef value to which sbSummary is to be set.
 	 
 		
 			
 	 */
 	void setSbSummary (EntityRef sbSummary);
  		
	
	
	


	
	// ===> Attribute schedulerMode
	
	
	

	
 	/**
 	 * Get schedulerMode.
 	 * @return schedulerMode as string
 	 */
 	string getSchedulerMode() const;
	
 
 	
 	
 	/**
 	 * Set schedulerMode with the specified string.
 	 * @param schedulerMode The string value to which schedulerMode is to be set.
 	 
 		
 			
 	 */
 	void setSchedulerMode (string schedulerMode);
  		
	
	
	


	
	// ===> Attribute baseRangeMin
	
	
	

	
 	/**
 	 * Get baseRangeMin.
 	 * @return baseRangeMin as Length
 	 */
 	Length getBaseRangeMin() const;
	
 
 	
 	
 	/**
 	 * Set baseRangeMin with the specified Length.
 	 * @param baseRangeMin The Length value to which baseRangeMin is to be set.
 	 
 		
 			
 	 */
 	void setBaseRangeMin (Length baseRangeMin);
  		
	
	
	


	
	// ===> Attribute baseRangeMax
	
	
	

	
 	/**
 	 * Get baseRangeMax.
 	 * @return baseRangeMax as Length
 	 */
 	Length getBaseRangeMax() const;
	
 
 	
 	
 	/**
 	 * Set baseRangeMax with the specified Length.
 	 * @param baseRangeMax The Length value to which baseRangeMax is to be set.
 	 
 		
 			
 	 */
 	void setBaseRangeMax (Length baseRangeMax);
  		
	
	
	


	
	// ===> Attribute baseRmsMinor
	
	
	

	
 	/**
 	 * Get baseRmsMinor.
 	 * @return baseRmsMinor as Length
 	 */
 	Length getBaseRmsMinor() const;
	
 
 	
 	
 	/**
 	 * Set baseRmsMinor with the specified Length.
 	 * @param baseRmsMinor The Length value to which baseRmsMinor is to be set.
 	 
 		
 			
 	 */
 	void setBaseRmsMinor (Length baseRmsMinor);
  		
	
	
	


	
	// ===> Attribute baseRmsMajor
	
	
	

	
 	/**
 	 * Get baseRmsMajor.
 	 * @return baseRmsMajor as Length
 	 */
 	Length getBaseRmsMajor() const;
	
 
 	
 	
 	/**
 	 * Set baseRmsMajor with the specified Length.
 	 * @param baseRmsMajor The Length value to which baseRmsMajor is to be set.
 	 
 		
 			
 	 */
 	void setBaseRmsMajor (Length baseRmsMajor);
  		
	
	
	


	
	// ===> Attribute basePa
	
	
	

	
 	/**
 	 * Get basePa.
 	 * @return basePa as Angle
 	 */
 	Angle getBasePa() const;
	
 
 	
 	
 	/**
 	 * Set basePa with the specified Angle.
 	 * @param basePa The Angle value to which basePa is to be set.
 	 
 		
 			
 	 */
 	void setBasePa (Angle basePa);
  		
	
	
	


	
	// ===> Attribute siteAltitude
	
	
	

	
 	/**
 	 * Get siteAltitude.
 	 * @return siteAltitude as Length
 	 */
 	Length getSiteAltitude() const;
	
 
 	
 	
 	/**
 	 * Set siteAltitude with the specified Length.
 	 * @param siteAltitude The Length value to which siteAltitude is to be set.
 	 
 		
 			
 	 */
 	void setSiteAltitude (Length siteAltitude);
  		
	
	
	


	
	// ===> Attribute siteLongitude
	
	
	

	
 	/**
 	 * Get siteLongitude.
 	 * @return siteLongitude as Angle
 	 */
 	Angle getSiteLongitude() const;
	
 
 	
 	
 	/**
 	 * Set siteLongitude with the specified Angle.
 	 * @param siteLongitude The Angle value to which siteLongitude is to be set.
 	 
 		
 			
 	 */
 	void setSiteLongitude (Angle siteLongitude);
  		
	
	
	


	
	// ===> Attribute siteLatitude
	
	
	

	
 	/**
 	 * Get siteLatitude.
 	 * @return siteLatitude as Angle
 	 */
 	Angle getSiteLatitude() const;
	
 
 	
 	
 	/**
 	 * Set siteLatitude with the specified Angle.
 	 * @param siteLatitude The Angle value to which siteLatitude is to be set.
 	 
 		
 			
 	 */
 	void setSiteLatitude (Angle siteLatitude);
  		
	
	
	


	
	// ===> Attribute aborted
	
	
	

	
 	/**
 	 * Get aborted.
 	 * @return aborted as bool
 	 */
 	bool getAborted() const;
	
 
 	
 	
 	/**
 	 * Set aborted with the specified bool.
 	 * @param aborted The bool value to which aborted is to be set.
 	 
 		
 			
 	 */
 	void setAborted (bool aborted);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute releaseDate, which is optional
	
	
	
	/**
	 * The attribute releaseDate is optional. Return true if this attribute exists.
	 * @return true if and only if the releaseDate attribute exists. 
	 */
	bool isReleaseDateExists() const;
	

	
 	/**
 	 * Get releaseDate, which is optional.
 	 * @return releaseDate as ArrayTime
 	 * @throws IllegalAccessException If releaseDate does not exist.
 	 */
 	ArrayTime getReleaseDate() const;
	
 
 	
 	
 	/**
 	 * Set releaseDate with the specified ArrayTime.
 	 * @param releaseDate The ArrayTime value to which releaseDate is to be set.
 	 
 		
 	 */
 	void setReleaseDate (ArrayTime releaseDate);
		
	
	
	
	/**
	 * Mark releaseDate, which is an optional field, as non-existent.
	 */
	void clearReleaseDate ();
	


	
	// ===> Attribute flagRow, which is optional
	
	
	
	/**
	 * The attribute flagRow is optional. Return true if this attribute exists.
	 * @return true if and only if the flagRow attribute exists. 
	 */
	bool isFlagRowExists() const;
	

	
 	/**
 	 * Get flagRow, which is optional.
 	 * @return flagRow as bool
 	 * @throws IllegalAccessException If flagRow does not exist.
 	 */
 	bool getFlagRow() const;
	
 
 	
 	
 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 		
 	 */
 	void setFlagRow (bool flagRow);
		
	
	
	
	/**
	 * Mark flagRow, which is an optional field, as non-existent.
	 */
	void clearFlagRow ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as vector<Tag> 
 	 */
 	vector<Tag>  getAntennaId() const;
	
 
 	
 	
 	/**
 	 * Set antennaId with the specified vector<Tag> .
 	 * @param antennaId The vector<Tag>  value to which antennaId is to be set.
 	 
 		
 			
 	 */
 	void setAntennaId (vector<Tag>  antennaId);
  		
	
	
	


	
	// ===> Attribute sBSummaryId
	
	
	

	
 	/**
 	 * Get sBSummaryId.
 	 * @return sBSummaryId as Tag
 	 */
 	Tag getSBSummaryId() const;
	
 
 	
 	
 	/**
 	 * Set sBSummaryId with the specified Tag.
 	 * @param sBSummaryId The Tag value to which sBSummaryId is to be set.
 	 
 		
 			
 	 */
 	void setSBSummaryId (Tag sBSummaryId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set antennaId[i] with the specified Tag.
 	 * @param i The index in antennaId where to set the Tag value.
 	 * @param antennaId The Tag value to which antennaId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void setAntennaId (int i, Tag antennaId); 
 			
	

	
		 
/**
 * Append a Tag to antennaId.
 * @param id the Tag to be appended to antennaId
 */
 void addAntennaId(Tag id); 

/**
 * Append a vector of Tag to antennaId.
 * @param id an array of Tag to be appended to antennaId
 */
 void addAntennaId(const vector<Tag> & id); 
 

 /**
  * Returns the Tag stored in antennaId at position i.
  * @param i the position in antennaId where the Tag is retrieved.
  * @return the Tag stored at position i in antennaId.
  */
 const Tag getAntennaId(int i);
 
 /**
  * Returns the AntennaRow linked to this row via the tag stored in antennaId
  * at position i.
  * @param i the position in antennaId.
  * @return a pointer on a AntennaRow whose key (a Tag) is equal to the Tag stored at position
  * i in the antennaId. 
  */
 AntennaRow* getAntenna(int i); 
 
 /**
  * Returns the vector of AntennaRow* linked to this row via the Tags stored in antennaId
  * @return an array of pointers on AntennaRow.
  */
 vector<AntennaRow *> getAntennas(); 
  

	

	

	
		
	/**
	 * sBSummaryId pointer to the row in the SBSummary table having SBSummary.sBSummaryId == sBSummaryId
	 * @return a SBSummaryRow*
	 * 
	 
	 */
	 SBSummaryRow* getSBSummaryUsingSBSummaryId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this ExecBlockRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startTime
	    
	 * @param endTime
	    
	 * @param execBlockNum
	    
	 * @param execBlockUID
	    
	 * @param projectId
	    
	 * @param configName
	    
	 * @param telescopeName
	    
	 * @param observerName
	    
	 * @param observingLog
	    
	 * @param sessionReference
	    
	 * @param sbSummary
	    
	 * @param schedulerMode
	    
	 * @param baseRangeMin
	    
	 * @param baseRangeMax
	    
	 * @param baseRmsMinor
	    
	 * @param baseRmsMajor
	    
	 * @param basePa
	    
	 * @param siteAltitude
	    
	 * @param siteLongitude
	    
	 * @param siteLatitude
	    
	 * @param aborted
	    
	 * @param numAntenna
	    
	 * @param antennaId
	    
	 * @param sBSummaryId
	    
	 */ 
	bool compareNoAutoInc(ArrayTime startTime, ArrayTime endTime, int execBlockNum, EntityRef execBlockUID, EntityRef projectId, string configName, string telescopeName, string observerName, string observingLog, string sessionReference, EntityRef sbSummary, string schedulerMode, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, Length siteAltitude, Angle siteLongitude, Angle siteLatitude, bool aborted, int numAntenna, vector<Tag>  antennaId, Tag sBSummaryId);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param startTime
	    
	 * @param endTime
	    
	 * @param execBlockNum
	    
	 * @param execBlockUID
	    
	 * @param projectId
	    
	 * @param configName
	    
	 * @param telescopeName
	    
	 * @param observerName
	    
	 * @param observingLog
	    
	 * @param sessionReference
	    
	 * @param sbSummary
	    
	 * @param schedulerMode
	    
	 * @param baseRangeMin
	    
	 * @param baseRangeMax
	    
	 * @param baseRmsMinor
	    
	 * @param baseRmsMajor
	    
	 * @param basePa
	    
	 * @param siteAltitude
	    
	 * @param siteLongitude
	    
	 * @param siteLatitude
	    
	 * @param aborted
	    
	 * @param numAntenna
	    
	 * @param antennaId
	    
	 * @param sBSummaryId
	    
	 */ 
	bool compareRequiredValue(ArrayTime startTime, ArrayTime endTime, int execBlockNum, EntityRef execBlockUID, EntityRef projectId, string configName, string telescopeName, string observerName, string observingLog, string sessionReference, EntityRef sbSummary, string schedulerMode, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, Length siteAltitude, Angle siteLongitude, Angle siteLatitude, bool aborted, int numAntenna, vector<Tag>  antennaId, Tag sBSummaryId); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ExecBlockRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(ExecBlockRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ExecBlockRowIDL struct.
	 */
	ExecBlockRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ExecBlockRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (ExecBlockRowIDL x) ;
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
	ExecBlockTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a ExecBlockRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ExecBlockRow (ExecBlockTable &table);

	/**
	 * Create a ExecBlockRow using a copy constructor mechanism.
	 * <p>
	 * Given a ExecBlockRow row and a ExecBlockTable table, the method creates a new
	 * ExecBlockRow owned by table. Each attribute of the created row is a copy (deep)
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
	 ExecBlockRow (ExecBlockTable &table, ExecBlockRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute execBlockId
	
	

	Tag execBlockId;

	
	
 	
 	/**
 	 * Set execBlockId with the specified Tag value.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setExecBlockId (Tag execBlockId);
  		
	

	
	// ===> Attribute startTime
	
	

	ArrayTime startTime;

	
	
 	

	
	// ===> Attribute endTime
	
	

	ArrayTime endTime;

	
	
 	

	
	// ===> Attribute execBlockNum
	
	

	int execBlockNum;

	
	
 	

	
	// ===> Attribute execBlockUID
	
	

	EntityRef execBlockUID;

	
	
 	

	
	// ===> Attribute projectId
	
	

	EntityRef projectId;

	
	
 	

	
	// ===> Attribute configName
	
	

	string configName;

	
	
 	

	
	// ===> Attribute telescopeName
	
	

	string telescopeName;

	
	
 	

	
	// ===> Attribute observerName
	
	

	string observerName;

	
	
 	

	
	// ===> Attribute observingLog
	
	

	string observingLog;

	
	
 	

	
	// ===> Attribute sessionReference
	
	

	string sessionReference;

	
	
 	

	
	// ===> Attribute sbSummary
	
	

	EntityRef sbSummary;

	
	
 	

	
	// ===> Attribute schedulerMode
	
	

	string schedulerMode;

	
	
 	

	
	// ===> Attribute baseRangeMin
	
	

	Length baseRangeMin;

	
	
 	

	
	// ===> Attribute baseRangeMax
	
	

	Length baseRangeMax;

	
	
 	

	
	// ===> Attribute baseRmsMinor
	
	

	Length baseRmsMinor;

	
	
 	

	
	// ===> Attribute baseRmsMajor
	
	

	Length baseRmsMajor;

	
	
 	

	
	// ===> Attribute basePa
	
	

	Angle basePa;

	
	
 	

	
	// ===> Attribute siteAltitude
	
	

	Length siteAltitude;

	
	
 	

	
	// ===> Attribute siteLongitude
	
	

	Angle siteLongitude;

	
	
 	

	
	// ===> Attribute siteLatitude
	
	

	Angle siteLatitude;

	
	
 	

	
	// ===> Attribute aborted
	
	

	bool aborted;

	
	
 	

	
	// ===> Attribute numAntenna
	
	

	int numAntenna;

	
	
 	

	
	// ===> Attribute releaseDate, which is optional
	
	
	bool releaseDateExists;
	

	ArrayTime releaseDate;

	
	
 	

	
	// ===> Attribute flagRow, which is optional
	
	
	bool flagRowExists;
	

	bool flagRow;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	vector<Tag>  antennaId;

	
	
 	

	
	// ===> Attribute sBSummaryId
	
	

	Tag sBSummaryId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		


	

	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, ExecBlockAttributeFromBin> fromBinMethods;
void execBlockIdFromBin( EndianISStream& eiss);
void startTimeFromBin( EndianISStream& eiss);
void endTimeFromBin( EndianISStream& eiss);
void execBlockNumFromBin( EndianISStream& eiss);
void execBlockUIDFromBin( EndianISStream& eiss);
void projectIdFromBin( EndianISStream& eiss);
void configNameFromBin( EndianISStream& eiss);
void telescopeNameFromBin( EndianISStream& eiss);
void observerNameFromBin( EndianISStream& eiss);
void observingLogFromBin( EndianISStream& eiss);
void sessionReferenceFromBin( EndianISStream& eiss);
void sbSummaryFromBin( EndianISStream& eiss);
void schedulerModeFromBin( EndianISStream& eiss);
void baseRangeMinFromBin( EndianISStream& eiss);
void baseRangeMaxFromBin( EndianISStream& eiss);
void baseRmsMinorFromBin( EndianISStream& eiss);
void baseRmsMajorFromBin( EndianISStream& eiss);
void basePaFromBin( EndianISStream& eiss);
void siteAltitudeFromBin( EndianISStream& eiss);
void siteLongitudeFromBin( EndianISStream& eiss);
void siteLatitudeFromBin( EndianISStream& eiss);
void abortedFromBin( EndianISStream& eiss);
void numAntennaFromBin( EndianISStream& eiss);
void antennaIdFromBin( EndianISStream& eiss);
void sBSummaryIdFromBin( EndianISStream& eiss);

void releaseDateFromBin( EndianISStream& eiss);
void flagRowFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the ExecBlockTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static ExecBlockRow* fromBin(EndianISStream& eiss, ExecBlockTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* ExecBlock_CLASS */
