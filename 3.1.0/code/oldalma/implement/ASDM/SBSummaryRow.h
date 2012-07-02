
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
 * File SBSummaryRow.h
 */
 
#ifndef SBSummaryRow_CLASS
#define SBSummaryRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::SBSummaryRowIDL;
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




	

	

	

	

	
#include "CSBType.h"
using namespace SBTypeMod;
	

	

	

	

	

	

	

	

	

	

	

	



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

/*\file SBSummary.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::SBSummaryTable;

	

/**
 * The SBSummaryRow class is a row of a SBSummaryTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class SBSummaryRow {
friend class asdm::SBSummaryTable;

public:

	virtual ~SBSummaryRow();

	/**
	 * Return the table to which this row belongs.
	 */
	SBSummaryTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SBSummaryRowIDL struct.
	 */
	SBSummaryRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SBSummaryRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (SBSummaryRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute sbId
	
	
	

	
 	/**
 	 * Get sbId.
 	 * @return sbId as EntityRef
 	 */
 	EntityRef getSbId() const;
	
 
 	
 	
 	/**
 	 * Set sbId with the specified EntityRef.
 	 * @param sbId The EntityRef value to which sbId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSbId (EntityRef sbId);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute obsUnitSetId
	
	
	

	
 	/**
 	 * Get obsUnitSetId.
 	 * @return obsUnitSetId as EntityRef
 	 */
 	EntityRef getObsUnitSetId() const;
	
 
 	
 	
 	/**
 	 * Set obsUnitSetId with the specified EntityRef.
 	 * @param obsUnitSetId The EntityRef value to which obsUnitSetId is to be set.
 	 
 		
 			
 	 */
 	void setObsUnitSetId (EntityRef obsUnitSetId);
  		
	
	
	


	
	// ===> Attribute sbIntent
	
	
	

	
 	/**
 	 * Get sbIntent.
 	 * @return sbIntent as string
 	 */
 	string getSbIntent() const;
	
 
 	
 	
 	/**
 	 * Set sbIntent with the specified string.
 	 * @param sbIntent The string value to which sbIntent is to be set.
 	 
 		
 			
 	 */
 	void setSbIntent (string sbIntent);
  		
	
	
	


	
	// ===> Attribute sbType
	
	
	

	
 	/**
 	 * Get sbType.
 	 * @return sbType as SBTypeMod::SBType
 	 */
 	SBTypeMod::SBType getSbType() const;
	
 
 	
 	
 	/**
 	 * Set sbType with the specified SBTypeMod::SBType.
 	 * @param sbType The SBTypeMod::SBType value to which sbType is to be set.
 	 
 		
 			
 	 */
 	void setSbType (SBTypeMod::SBType sbType);
  		
	
	
	


	
	// ===> Attribute sbDuration
	
	
	

	
 	/**
 	 * Get sbDuration.
 	 * @return sbDuration as Interval
 	 */
 	Interval getSbDuration() const;
	
 
 	
 	
 	/**
 	 * Set sbDuration with the specified Interval.
 	 * @param sbDuration The Interval value to which sbDuration is to be set.
 	 
 		
 			
 	 */
 	void setSbDuration (Interval sbDuration);
  		
	
	
	


	
	// ===> Attribute numScan
	
	
	

	
 	/**
 	 * Get numScan.
 	 * @return numScan as int
 	 */
 	int getNumScan() const;
	
 
 	
 	
 	/**
 	 * Set numScan with the specified int.
 	 * @param numScan The int value to which numScan is to be set.
 	 
 		
 			
 	 */
 	void setNumScan (int numScan);
  		
	
	
	


	
	// ===> Attribute scanIntent
	
	
	

	
 	/**
 	 * Get scanIntent.
 	 * @return scanIntent as vector<string >
 	 */
 	vector<string > getScanIntent() const;
	
 
 	
 	
 	/**
 	 * Set scanIntent with the specified vector<string >.
 	 * @param scanIntent The vector<string > value to which scanIntent is to be set.
 	 
 		
 			
 	 */
 	void setScanIntent (vector<string > scanIntent);
  		
	
	
	


	
	// ===> Attribute numberRepeats
	
	
	

	
 	/**
 	 * Get numberRepeats.
 	 * @return numberRepeats as int
 	 */
 	int getNumberRepeats() const;
	
 
 	
 	
 	/**
 	 * Set numberRepeats with the specified int.
 	 * @param numberRepeats The int value to which numberRepeats is to be set.
 	 
 		
 			
 	 */
 	void setNumberRepeats (int numberRepeats);
  		
	
	
	


	
	// ===> Attribute weatherConstraint
	
	
	

	
 	/**
 	 * Get weatherConstraint.
 	 * @return weatherConstraint as vector<string >
 	 */
 	vector<string > getWeatherConstraint() const;
	
 
 	
 	
 	/**
 	 * Set weatherConstraint with the specified vector<string >.
 	 * @param weatherConstraint The vector<string > value to which weatherConstraint is to be set.
 	 
 		
 			
 	 */
 	void setWeatherConstraint (vector<string > weatherConstraint);
  		
	
	
	


	
	// ===> Attribute scienceGoal
	
	
	

	
 	/**
 	 * Get scienceGoal.
 	 * @return scienceGoal as vector<string >
 	 */
 	vector<string > getScienceGoal() const;
	
 
 	
 	
 	/**
 	 * Set scienceGoal with the specified vector<string >.
 	 * @param scienceGoal The vector<string > value to which scienceGoal is to be set.
 	 
 		
 			
 	 */
 	void setScienceGoal (vector<string > scienceGoal);
  		
	
	
	


	
	// ===> Attribute raCenter
	
	
	

	
 	/**
 	 * Get raCenter.
 	 * @return raCenter as double
 	 */
 	double getRaCenter() const;
	
 
 	
 	
 	/**
 	 * Set raCenter with the specified double.
 	 * @param raCenter The double value to which raCenter is to be set.
 	 
 		
 			
 	 */
 	void setRaCenter (double raCenter);
  		
	
	
	


	
	// ===> Attribute decCenter
	
	
	

	
 	/**
 	 * Get decCenter.
 	 * @return decCenter as double
 	 */
 	double getDecCenter() const;
	
 
 	
 	
 	/**
 	 * Set decCenter with the specified double.
 	 * @param decCenter The double value to which decCenter is to be set.
 	 
 		
 			
 	 */
 	void setDecCenter (double decCenter);
  		
	
	
	


	
	// ===> Attribute frequency
	
	
	

	
 	/**
 	 * Get frequency.
 	 * @return frequency as double
 	 */
 	double getFrequency() const;
	
 
 	
 	
 	/**
 	 * Set frequency with the specified double.
 	 * @param frequency The double value to which frequency is to be set.
 	 
 		
 			
 	 */
 	void setFrequency (double frequency);
  		
	
	
	


	
	// ===> Attribute frequencyBand
	
	
	

	
 	/**
 	 * Get frequencyBand.
 	 * @return frequencyBand as string
 	 */
 	string getFrequencyBand() const;
	
 
 	
 	
 	/**
 	 * Set frequencyBand with the specified string.
 	 * @param frequencyBand The string value to which frequencyBand is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyBand (string frequencyBand);
  		
	
	
	


	
	// ===> Attribute observingMode
	
	
	

	
 	/**
 	 * Get observingMode.
 	 * @return observingMode as vector<string >
 	 */
 	vector<string > getObservingMode() const;
	
 
 	
 	
 	/**
 	 * Set observingMode with the specified vector<string >.
 	 * @param observingMode The vector<string > value to which observingMode is to be set.
 	 
 		
 			
 	 */
 	void setObservingMode (vector<string > observingMode);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SBSummaryRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(EntityRef sbId, EntityRef projectId, EntityRef obsUnitSetId, string sbIntent, SBTypeMod::SBType sbType, Interval sbDuration, int numScan, vector<string > scanIntent, int numberRepeats, vector<string > weatherConstraint, vector<string > scienceGoal, double raCenter, double decCenter, double frequency, string frequencyBand, vector<string > observingMode);
	
	

	
	bool compareRequiredValue(EntityRef projectId, EntityRef obsUnitSetId, string sbIntent, SBTypeMod::SBType sbType, Interval sbDuration, int numScan, vector<string > scanIntent, int numberRepeats, vector<string > weatherConstraint, vector<string > scienceGoal, double raCenter, double decCenter, double frequency, string frequencyBand, vector<string > observingMode); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SBSummaryRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(SBSummaryRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	SBSummaryTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a SBSummaryRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SBSummaryRow (SBSummaryTable &table);

	/**
	 * Create a SBSummaryRow using a copy constructor mechanism.
	 * <p>
	 * Given a SBSummaryRow row and a SBSummaryTable table, the method creates a new
	 * SBSummaryRow owned by table. Each attribute of the created row is a copy (deep)
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
	 SBSummaryRow (SBSummaryTable &table, SBSummaryRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute sbId
	
	

	EntityRef sbId;

	
	
 	

	
	// ===> Attribute projectId
	
	

	EntityRef projectId;

	
	
 	

	
	// ===> Attribute obsUnitSetId
	
	

	EntityRef obsUnitSetId;

	
	
 	

	
	// ===> Attribute sbIntent
	
	

	string sbIntent;

	
	
 	

	
	// ===> Attribute sbType
	
	

	SBTypeMod::SBType sbType;

	
	
 	

	
	// ===> Attribute sbDuration
	
	

	Interval sbDuration;

	
	
 	

	
	// ===> Attribute numScan
	
	

	int numScan;

	
	
 	

	
	// ===> Attribute scanIntent
	
	

	vector<string > scanIntent;

	
	
 	

	
	// ===> Attribute numberRepeats
	
	

	int numberRepeats;

	
	
 	

	
	// ===> Attribute weatherConstraint
	
	

	vector<string > weatherConstraint;

	
	
 	

	
	// ===> Attribute scienceGoal
	
	

	vector<string > scienceGoal;

	
	
 	

	
	// ===> Attribute raCenter
	
	

	double raCenter;

	
	
 	

	
	// ===> Attribute decCenter
	
	

	double decCenter;

	
	
 	

	
	// ===> Attribute frequency
	
	

	double frequency;

	
	
 	

	
	// ===> Attribute frequencyBand
	
	

	string frequencyBand;

	
	
 	

	
	// ===> Attribute observingMode
	
	

	vector<string > observingMode;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	

};

} // End namespace asdm

#endif /* SBSummary_CLASS */
