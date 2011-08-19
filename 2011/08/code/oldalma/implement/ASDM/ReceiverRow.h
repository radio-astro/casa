
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
 * File ReceiverRow.h
 */
 
#ifndef ReceiverRow_CLASS
#define ReceiverRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::ReceiverRowIDL;
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




	

	

	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	

	
#include "CReceiverSideband.h"
using namespace ReceiverSidebandMod;
	

	

	

	

	

	

	

	



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

/*\file Receiver.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::ReceiverTable;


// class asdm::SpectralWindowRow;
class SpectralWindowRow;
	

/**
 * The ReceiverRow class is a row of a ReceiverTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class ReceiverRow {
friend class asdm::ReceiverTable;

public:

	virtual ~ReceiverRow();

	/**
	 * Return the table to which this row belongs.
	 */
	ReceiverTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a ReceiverRowIDL struct.
	 */
	ReceiverRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct ReceiverRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (ReceiverRowIDL x) throw(ConversionException);
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
  		
	
	
	


	
	// ===> Attribute numLo
	
	
	

	
 	/**
 	 * Get numLo.
 	 * @return numLo as int
 	 */
 	int getNumLo() const;
	
 
 	
 	
 	/**
 	 * Set numLo with the specified int.
 	 * @param numLo The int value to which numLo is to be set.
 	 
 		
 			
 	 */
 	void setNumLo (int numLo);
  		
	
	
	


	
	// ===> Attribute name
	
	
	

	
 	/**
 	 * Get name.
 	 * @return name as string
 	 */
 	string getName() const;
	
 
 	
 	
 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 	 
 		
 			
 	 */
 	void setName (string name);
  		
	
	
	


	
	// ===> Attribute frequencyBand
	
	
	

	
 	/**
 	 * Get frequencyBand.
 	 * @return frequencyBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand getFrequencyBand() const;
	
 
 	
 	
 	/**
 	 * Set frequencyBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param frequencyBand The ReceiverBandMod::ReceiverBand value to which frequencyBand is to be set.
 	 
 		
 			
 	 */
 	void setFrequencyBand (ReceiverBandMod::ReceiverBand frequencyBand);
  		
	
	
	


	
	// ===> Attribute freqLo
	
	
	

	
 	/**
 	 * Get freqLo.
 	 * @return freqLo as vector<Frequency >
 	 */
 	vector<Frequency > getFreqLo() const;
	
 
 	
 	
 	/**
 	 * Set freqLo with the specified vector<Frequency >.
 	 * @param freqLo The vector<Frequency > value to which freqLo is to be set.
 	 
 		
 			
 	 */
 	void setFreqLo (vector<Frequency > freqLo);
  		
	
	
	


	
	// ===> Attribute receiverSideband
	
	
	

	
 	/**
 	 * Get receiverSideband.
 	 * @return receiverSideband as ReceiverSidebandMod::ReceiverSideband
 	 */
 	ReceiverSidebandMod::ReceiverSideband getReceiverSideband() const;
	
 
 	
 	
 	/**
 	 * Set receiverSideband with the specified ReceiverSidebandMod::ReceiverSideband.
 	 * @param receiverSideband The ReceiverSidebandMod::ReceiverSideband value to which receiverSideband is to be set.
 	 
 		
 			
 	 */
 	void setReceiverSideband (ReceiverSidebandMod::ReceiverSideband receiverSideband);
  		
	
	
	


	
	// ===> Attribute sidebandLo
	
	
	

	
 	/**
 	 * Get sidebandLo.
 	 * @return sidebandLo as vector<int >
 	 */
 	vector<int > getSidebandLo() const;
	
 
 	
 	
 	/**
 	 * Set sidebandLo with the specified vector<int >.
 	 * @param sidebandLo The vector<int > value to which sidebandLo is to be set.
 	 
 		
 			
 	 */
 	void setSidebandLo (vector<int > sidebandLo);
  		
	
	
	


	
	// ===> Attribute dewarName, which is optional
	
	
	
	/**
	 * The attribute dewarName is optional. Return true if this attribute exists.
	 * @return true if and only if the dewarName attribute exists. 
	 */
	bool isDewarNameExists() const;
	

	
 	/**
 	 * Get dewarName, which is optional.
 	 * @return dewarName as string
 	 * @throws IllegalAccessException If dewarName does not exist.
 	 */
 	string getDewarName() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set dewarName with the specified string.
 	 * @param dewarName The string value to which dewarName is to be set.
 	 
 		
 	 */
 	void setDewarName (string dewarName);
		
	
	
	
	/**
	 * Mark dewarName, which is an optional field, as non-existent.
	 */
	void clearDewarName ();
	


	
	// ===> Attribute tDewar
	
	
	

	
 	/**
 	 * Get tDewar.
 	 * @return tDewar as Temperature
 	 */
 	Temperature getTDewar() const;
	
 
 	
 	
 	/**
 	 * Set tDewar with the specified Temperature.
 	 * @param tDewar The Temperature value to which tDewar is to be set.
 	 
 		
 			
 	 */
 	void setTDewar (Temperature tDewar);
  		
	
	
	


	
	// ===> Attribute stabilityDuration
	
	
	

	
 	/**
 	 * Get stabilityDuration.
 	 * @return stabilityDuration as Interval
 	 */
 	Interval getStabilityDuration() const;
	
 
 	
 	
 	/**
 	 * Set stabilityDuration with the specified Interval.
 	 * @param stabilityDuration The Interval value to which stabilityDuration is to be set.
 	 
 		
 			
 	 */
 	void setStabilityDuration (Interval stabilityDuration);
  		
	
	
	


	
	// ===> Attribute stability
	
	
	

	
 	/**
 	 * Get stability.
 	 * @return stability as double
 	 */
 	double getStability() const;
	
 
 	
 	
 	/**
 	 * Set stability with the specified double.
 	 * @param stability The double value to which stability is to be set.
 	 
 		
 			
 	 */
 	void setStability (double stability);
  		
	
	
	


	
	// ===> Attribute stabilityflag, which is optional
	
	
	
	/**
	 * The attribute stabilityflag is optional. Return true if this attribute exists.
	 * @return true if and only if the stabilityflag attribute exists. 
	 */
	bool isStabilityflagExists() const;
	

	
 	/**
 	 * Get stabilityflag, which is optional.
 	 * @return stabilityflag as bool
 	 * @throws IllegalAccessException If stabilityflag does not exist.
 	 */
 	bool getStabilityflag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set stabilityflag with the specified bool.
 	 * @param stabilityflag The bool value to which stabilityflag is to be set.
 	 
 		
 	 */
 	void setStabilityflag (bool stabilityflag);
		
	
	
	
	/**
	 * Mark stabilityflag, which is an optional field, as non-existent.
	 */
	void clearStabilityflag ();
	


	
	// ===> Attribute receiverId
	
	
	

	
 	/**
 	 * Get receiverId.
 	 * @return receiverId as int
 	 */
 	int getReceiverId() const;
	
 
 	
 	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute spectralWindowId
	
	
	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag getSpectralWindowId() const;
	
 
 	
 	
 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSpectralWindowId (Tag spectralWindowId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * spectralWindowId pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* getSpectralWindowUsingSpectralWindowId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this ReceiverRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag spectralWindowId, ArrayTimeInterval timeInterval, int numLo, string name, ReceiverBandMod::ReceiverBand frequencyBand, vector<Frequency > freqLo, ReceiverSidebandMod::ReceiverSideband receiverSideband, vector<int > sidebandLo, Temperature tDewar, Interval stabilityDuration, double stability);
	
	

	
	bool compareRequiredValue(int numLo, string name, ReceiverBandMod::ReceiverBand frequencyBand, vector<Frequency > freqLo, ReceiverSidebandMod::ReceiverSideband receiverSideband, vector<int > sidebandLo, Temperature tDewar, Interval stabilityDuration, double stability); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the ReceiverRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(ReceiverRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	ReceiverTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a ReceiverRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	ReceiverRow (ReceiverTable &table);

	/**
	 * Create a ReceiverRow using a copy constructor mechanism.
	 * <p>
	 * Given a ReceiverRow row and a ReceiverTable table, the method creates a new
	 * ReceiverRow owned by table. Each attribute of the created row is a copy (deep)
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
	 ReceiverRow (ReceiverTable &table, ReceiverRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute numLo
	
	

	int numLo;

	
	
 	

	
	// ===> Attribute name
	
	

	string name;

	
	
 	

	
	// ===> Attribute frequencyBand
	
	

	ReceiverBandMod::ReceiverBand frequencyBand;

	
	
 	

	
	// ===> Attribute freqLo
	
	

	vector<Frequency > freqLo;

	
	
 	

	
	// ===> Attribute receiverSideband
	
	

	ReceiverSidebandMod::ReceiverSideband receiverSideband;

	
	
 	

	
	// ===> Attribute sidebandLo
	
	

	vector<int > sidebandLo;

	
	
 	

	
	// ===> Attribute dewarName, which is optional
	
	
	bool dewarNameExists;
	

	string dewarName;

	
	
 	

	
	// ===> Attribute tDewar
	
	

	Temperature tDewar;

	
	
 	

	
	// ===> Attribute stabilityDuration
	
	

	Interval stabilityDuration;

	
	
 	

	
	// ===> Attribute stability
	
	

	double stability;

	
	
 	

	
	// ===> Attribute stabilityflag, which is optional
	
	
	bool stabilityflagExists;
	

	bool stabilityflag;

	
	
 	

	
	// ===> Attribute receiverId
	
	

	int receiverId;

	
	
 	
 	/**
 	 * Set receiverId with the specified int value.
 	 * @param receiverId The int value to which receiverId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setReceiverId (int receiverId);
  		
	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute spectralWindowId
	
	

	Tag spectralWindowId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	


};

} // End namespace asdm

#endif /* Receiver_CLASS */
