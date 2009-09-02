
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
 * File TotalPowerRow.h
 */
 
#ifndef TotalPowerRow_CLASS
#define TotalPowerRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::TotalPowerRowIDL;
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


namespace asdm {

//class asdm::TotalPowerTable;


// class asdm::StateRow;
class StateRow;

// class asdm::FieldRow;
class FieldRow;

// class asdm::ConfigDescriptionRow;
class ConfigDescriptionRow;

// class asdm::ExecBlockRow;
class ExecBlockRow;
	

/**
 * The TotalPowerRow class is a row of a TotalPowerTable.
 */
class TotalPowerRow {
friend class asdm::TotalPowerTable;

public:

	virtual ~TotalPowerRow();

	/**
	 * Return the table to which this row belongs.
	 */
	TotalPowerTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a TotalPowerRowIDL struct.
	 */
	TotalPowerRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct TotalPowerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (TotalPowerRowIDL x) throw(ConversionException);
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
	
   /**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a TotalPowerRow.
	  * @param eiss the EndianISStream to be read.
	  * @table the TotalPowerTable to which the row built by deserialization will be parented.
	  */
	 static TotalPowerRow* fromBin(EndianISStream& eiss, TotalPowerTable& table);
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute time
	
	
	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime getTime() const;
	
 
 	
 	
 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 */
 	void setTime (ArrayTime& time) throw(IllegalAccessException);
  		
	
	
	


	
	// ===> Attribute scanNumber
	
	
	

	
 	/**
 	 * Get scanNumber.
 	 * @return scanNumber as int
 	 */
 	int getScanNumber() const;
	
 
 	
 	
 	/**
 	 * Set scanNumber with the specified int.
 	 * @param scanNumber The int value to which scanNumber is to be set.
 		
 	 */
 	void setScanNumber (int scanNumber);
		
	
	
	


	
	// ===> Attribute subscanNumber
	
	
	

	
 	/**
 	 * Get subscanNumber.
 	 * @return subscanNumber as int
 	 */
 	int getSubscanNumber() const;
	
 
 	
 	
 	/**
 	 * Set subscanNumber with the specified int.
 	 * @param subscanNumber The int value to which subscanNumber is to be set.
 		
 	 */
 	void setSubscanNumber (int subscanNumber);
		
	
	
	


	
	// ===> Attribute integrationNumber
	
	
	

	
 	/**
 	 * Get integrationNumber.
 	 * @return integrationNumber as int
 	 */
 	int getIntegrationNumber() const;
	
 
 	
 	
 	/**
 	 * Set integrationNumber with the specified int.
 	 * @param integrationNumber The int value to which integrationNumber is to be set.
 		
 	 */
 	void setIntegrationNumber (int integrationNumber);
		
	
	
	


	
	// ===> Attribute uvw
	
	
	

	
 	/**
 	 * Get uvw.
 	 * @return uvw as vector<vector<Length > >
 	 */
 	vector<vector<Length > > getUvw() const;
	
 
 	
 	
 	/**
 	 * Set uvw with the specified vector<vector<Length > >.
 	 * @param uvw The vector<vector<Length > > value to which uvw is to be set.
 		
 	 */
 	void setUvw (vector<vector<Length > >& uvw);
		
	
	
	


	
	// ===> Attribute exposure
	
	
	

	
 	/**
 	 * Get exposure.
 	 * @return exposure as vector<vector<Interval > >
 	 */
 	vector<vector<Interval > > getExposure() const;
	
 
 	
 	
 	/**
 	 * Set exposure with the specified vector<vector<Interval > >.
 	 * @param exposure The vector<vector<Interval > > value to which exposure is to be set.
 		
 	 */
 	void setExposure (vector<vector<Interval > >& exposure);
		
	
	
	


	
	// ===> Attribute timeCentroid
	
	
	

	
 	/**
 	 * Get timeCentroid.
 	 * @return timeCentroid as vector<vector<ArrayTime > >
 	 */
 	vector<vector<ArrayTime > > getTimeCentroid() const;
	
 
 	
 	
 	/**
 	 * Set timeCentroid with the specified vector<vector<ArrayTime > >.
 	 * @param timeCentroid The vector<vector<ArrayTime > > value to which timeCentroid is to be set.
 		
 	 */
 	void setTimeCentroid (vector<vector<ArrayTime > >& timeCentroid);
		
	
	
	


	
	// ===> Attribute floatData
	
	
	

	
 	/**
 	 * Get floatData.
 	 * @return floatData as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > getFloatData() const;
	
 
 	
 	
 	/**
 	 * Set floatData with the specified vector<vector<vector<float > > >.
 	 * @param floatData The vector<vector<vector<float > > > value to which floatData is to be set.
 		
 	 */
 	void setFloatData (vector<vector<vector<float > > >& floatData);
		
	
	
	


	
	// ===> Attribute flagAnt
	
	
	

	
 	/**
 	 * Get flagAnt.
 	 * @return flagAnt as vector<int >
 	 */
 	vector<int > getFlagAnt() const;
	
 
 	
 	
 	/**
 	 * Set flagAnt with the specified vector<int >.
 	 * @param flagAnt The vector<int > value to which flagAnt is to be set.
 		
 	 */
 	void setFlagAnt (vector<int >& flagAnt);
		
	
	
	


	
	// ===> Attribute flagPol
	
	
	

	
 	/**
 	 * Get flagPol.
 	 * @return flagPol as vector<vector<int > >
 	 */
 	vector<vector<int > > getFlagPol() const;
	
 
 	
 	
 	/**
 	 * Set flagPol with the specified vector<vector<int > >.
 	 * @param flagPol The vector<vector<int > > value to which flagPol is to be set.
 		
 	 */
 	void setFlagPol (vector<vector<int > >& flagPol);
		
	
	
	


	
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
		
	
	
	


	
	// ===> Attribute interval
	
	
	

	
 	/**
 	 * Get interval.
 	 * @return interval as Interval
 	 */
 	Interval getInterval() const;
	
 
 	
 	
 	/**
 	 * Set interval with the specified Interval.
 	 * @param interval The Interval value to which interval is to be set.
 		
 	 */
 	void setInterval (Interval& interval);
		
	
	
	


	
	// ===> Attribute subintegrationNumber, which is optional
	
	
	
	/**
	 * The attribute subintegrationNumber is optional. Return true if this attribute exists.
	 * @return true if and only if the subintegrationNumber attribute exists. 
	 */
	bool isSubintegrationNumberExists() const;
	

	
 	/**
 	 * Get subintegrationNumber, which is optional.
 	 * @return subintegrationNumber as int
 	 * @throws IllegalAccessException If subintegrationNumber does not exist.
 	 */
 	int getSubintegrationNumber() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set subintegrationNumber with the specified int.
 	 * @param subintegrationNumber The int value to which subintegrationNumber is to be set.
 		
 	 */
 	void setSubintegrationNumber (int subintegrationNumber);
		
	
	
	
	/**
	 * Mark subintegrationNumber, which is an optional field, as non-existent.
	 */
	void clearSubintegrationNumber ();
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute configDescriptionId
	
	
	

	
 	/**
 	 * Get configDescriptionId.
 	 * @return configDescriptionId as Tag
 	 */
 	Tag getConfigDescriptionId() const;
	
 
 	
 	
 	/**
 	 * Set configDescriptionId with the specified Tag.
 	 * @param configDescriptionId The Tag value to which configDescriptionId is to be set.
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 */
 	void setConfigDescriptionId (Tag& configDescriptionId) throw(IllegalAccessException);
  		
	
	
	


	
	// ===> Attribute execBlockId
	
	
	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag getExecBlockId() const;
	
 
 	
 	
 	/**
 	 * Set execBlockId with the specified Tag.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
 		
 	 */
 	void setExecBlockId (Tag& execBlockId);
		
	
	
	


	
	// ===> Attribute fieldId
	
	
	

	
 	/**
 	 * Get fieldId.
 	 * @return fieldId as Tag
 	 */
 	Tag getFieldId() const;
	
 
 	
 	
 	/**
 	 * Set fieldId with the specified Tag.
 	 * @param fieldId The Tag value to which fieldId is to be set.
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 */
 	void setFieldId (Tag& fieldId) throw(IllegalAccessException);
  		
	
	
	


	
	// ===> Attribute stateId
	
	
	

	
 	/**
 	 * Get stateId.
 	 * @return stateId as vector<Tag> 
 	 */
 	vector<Tag>  getStateId() const;
	
 
 	
 	
 	/**
 	 * Set stateId with the specified vector<Tag> .
 	 * @param stateId The vector<Tag>  value to which stateId is to be set.
 		
 	 */
 	void setStateId (vector<Tag>&  stateId);
		
	
	
	


	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set stateId[i] with the specified Tag.
 	 * @param i The index in stateId where to set the Tag value.
 	 * @param stateId The Tag value to which stateId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void setStateId (int i, Tag stateId) throw(OutOfBoundsException) ;
 			
	

	
		 
/**
 * Append a Tag to stateId.
 * @param id the Tag to be appended to stateId
 */
 void addStateId(Tag id); 

/**
 * Append a vector of Tag to stateId.
 * @param id an array of Tag to be appended to stateId
 */
 void addStateId(const vector<Tag> & id); 
 

 /**
  * Returns the Tag stored in stateId at position i.
  * @param i the position in stateId where the Tag is retrieved.
  * @return the Tag stored at position i in stateId.
  */
 const Tag getStateId(int i);
 
 /**
  * Returns the StateRow linked to this row via the tag stored in stateId
  * at position i.
  * @param i the position in stateId.
  * @return a pointer on a StateRow whose key (a Tag) is equal to the Tag stored at position
  * i in the stateId. 
  */
 StateRow* getState(int i); 
 
 /**
  * Returns the vector of StateRow* linked to this row via the Tags stored in stateId
  * @return an array of pointers on StateRow.
  */
 vector<StateRow *> getStates(); 
  

	

	

	
		
	/**
	 * fieldId pointer to the row in the Field table having Field.fieldId == fieldId
	 * @return a FieldRow*
	 * 
	 
	 */
	 FieldRow* getFieldUsingFieldId();
	 

	

	

	
		
	/**
	 * configDescriptionId pointer to the row in the ConfigDescription table having ConfigDescription.configDescriptionId == configDescriptionId
	 * @return a ConfigDescriptionRow*
	 * 
	 
	 */
	 ConfigDescriptionRow* getConfigDescriptionUsingConfigDescriptionId();
	 

	

	

	
		
			
	// ===> One to one link from a row of TotalPower table to a row of ExecBlock table.
	
	/**
	 * Get the row in table ExecBlock by traversing the defined link to that table.
	 * @return A row in ExecBlock table.
	 * @throws NoSuchRow if there is no such row in table ExecBlock.
	 */
	ExecBlockRow *getExecBlock() const throw(NoSuchRow);
	

		
		
		
	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this TotalPowerRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag configDescriptionId, Tag fieldId, ArrayTime time, Tag execBlockId, vector<Tag>  stateId, int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval);
	
	

	
	bool compareRequiredValue(Tag execBlockId, vector<Tag>  stateId, int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the TotalPowerRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(TotalPowerRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	TotalPowerTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a TotalPowerRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	TotalPowerRow (TotalPowerTable &table);

	/**
	 * Create a TotalPowerRow using a copy constructor mechanism.
	 * <p>
	 * Given a TotalPowerRow row and a TotalPowerTable table, the method creates a new
	 * TotalPowerRow owned by table. Each attribute of the created row is a copy (deep)
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
	 TotalPowerRow (TotalPowerTable &table, TotalPowerRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute time
	
	
	ArrayTime time;
	
	
 	

	
	// ===> Attribute scanNumber
	
	
	int scanNumber;
	
	
 	

	
	// ===> Attribute subscanNumber
	
	
	int subscanNumber;
	
	
 	

	
	// ===> Attribute integrationNumber
	
	
	int integrationNumber;
	
	
 	

	
	// ===> Attribute uvw
	
	
	vector<vector<Length > > uvw;
	
	
 	

	
	// ===> Attribute exposure
	
	
	vector<vector<Interval > > exposure;
	
	
 	

	
	// ===> Attribute timeCentroid
	
	
	vector<vector<ArrayTime > > timeCentroid;
	
	
 	

	
	// ===> Attribute floatData
	
	
	vector<vector<vector<float > > > floatData;
	
	
 	

	
	// ===> Attribute flagAnt
	
	
	vector<int > flagAnt;
	
	
 	

	
	// ===> Attribute flagPol
	
	
	vector<vector<int > > flagPol;
	
	
 	

	
	// ===> Attribute flagRow
	
	
	bool flagRow;
	
	
 	

	
	// ===> Attribute interval
	
	
	Interval interval;
	
	
 	

	
	// ===> Attribute subintegrationNumber, which is optional
	
	
	bool subintegrationNumberExists;
	
	int subintegrationNumber;
	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute configDescriptionId
	
	
	Tag configDescriptionId;
	
	
 	

	
	// ===> Attribute execBlockId
	
	
	Tag execBlockId;
	
	
 	

	
	// ===> Attribute fieldId
	
	
	Tag fieldId;
	
	
 	

	
	// ===> Attribute stateId
	
	
	vector<Tag>  stateId;
	
	
 	

	///////////
	// Links //
	///////////
	
	
		


	

	
		

	 

	

	
		

	 

	

		
		
	


};

} // End namespace asdm

#endif /* TotalPower_CLASS */
