
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

/*\file ExecBlock.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::ExecBlockTable;


// class asdm::AntennaRow;
class AntennaRow;
	

/**
 * The ExecBlockRow class is a row of a ExecBlockTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
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
	 */
	void setFromIDL (ExecBlockRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute execBlockId
	
	
	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag getExecBlockId() const;
	
 
 	
 	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute timeInterval
	
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval getTimeInterval() const;
	
 
 	
 	
 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 		
 			
 	 */
 	void setTimeInterval (ArrayTimeInterval timeInterval);
  		
	
	
	


	
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
 	 * @return observingLog as vector<string >
 	 */
 	vector<string > getObservingLog() const;
	
 
 	
 	
 	/**
 	 * Set observingLog with the specified vector<string >.
 	 * @param observingLog The vector<string > value to which observingLog is to be set.
 	 
 		
 			
 	 */
 	void setObservingLog (vector<string > observingLog);
  		
	
	
	


	
	// ===> Attribute schedulerMode
	
	
	

	
 	/**
 	 * Get schedulerMode.
 	 * @return schedulerMode as vector<string >
 	 */
 	vector<string > getSchedulerMode() const;
	
 
 	
 	
 	/**
 	 * Set schedulerMode with the specified vector<string >.
 	 * @param schedulerMode The vector<string > value to which schedulerMode is to be set.
 	 
 		
 			
 	 */
 	void setSchedulerMode (vector<string > schedulerMode);
  		
	
	
	


	
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
 	ArrayTime getReleaseDate() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set releaseDate with the specified ArrayTime.
 	 * @param releaseDate The ArrayTime value to which releaseDate is to be set.
 	 
 		
 	 */
 	void setReleaseDate (ArrayTime releaseDate);
		
	
	
	
	/**
	 * Mark releaseDate, which is an optional field, as non-existent.
	 */
	void clearReleaseDate ();
	


	
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
	 * Compare each mandatory attribute except the autoincrementable one of this ExecBlockRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(vector<Tag>  antennaId, int execBlockNum, string telescopeName, string configName, int numAntenna, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, ArrayTimeInterval timeInterval, string observerName, vector<string > observingLog, vector<string > schedulerMode, EntityRef projectId, Angle siteLongitude, Angle siteLatitude, Length siteAltitude, bool flagRow, EntityRef execBlockUID, bool aborted);
	
	

	
	bool compareRequiredValue(vector<Tag>  antennaId, int execBlockNum, string telescopeName, string configName, int numAntenna, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, ArrayTimeInterval timeInterval, string observerName, vector<string > observingLog, vector<string > schedulerMode, EntityRef projectId, Angle siteLongitude, Angle siteLatitude, Length siteAltitude, bool flagRow, EntityRef execBlockUID, bool aborted); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ExecBlockRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(ExecBlockRow* x) ;

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
	void isAdded();


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
  		
	

	
	// ===> Attribute execBlockNum
	
	

	int execBlockNum;

	
	
 	

	
	// ===> Attribute telescopeName
	
	

	string telescopeName;

	
	
 	

	
	// ===> Attribute configName
	
	

	string configName;

	
	
 	

	
	// ===> Attribute numAntenna
	
	

	int numAntenna;

	
	
 	

	
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

	
	
 	

	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute observerName
	
	

	string observerName;

	
	
 	

	
	// ===> Attribute observingLog
	
	

	vector<string > observingLog;

	
	
 	

	
	// ===> Attribute schedulerMode
	
	

	vector<string > schedulerMode;

	
	
 	

	
	// ===> Attribute projectId
	
	

	EntityRef projectId;

	
	
 	

	
	// ===> Attribute siteLongitude
	
	

	Angle siteLongitude;

	
	
 	

	
	// ===> Attribute siteLatitude
	
	

	Angle siteLatitude;

	
	
 	

	
	// ===> Attribute siteAltitude
	
	

	Length siteAltitude;

	
	
 	

	
	// ===> Attribute releaseDate, which is optional
	
	
	bool releaseDateExists;
	

	ArrayTime releaseDate;

	
	
 	

	
	// ===> Attribute flagRow
	
	

	bool flagRow;

	
	
 	

	
	// ===> Attribute execBlockUID
	
	

	EntityRef execBlockUID;

	
	
 	

	
	// ===> Attribute aborted
	
	

	bool aborted;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	vector<Tag>  antennaId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		


	


};

} // End namespace asdm

#endif /* ExecBlock_CLASS */
