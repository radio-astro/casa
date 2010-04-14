
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
 * File GainTrackingRow.h
 */
 
#ifndef GainTrackingRow_CLASS
#define GainTrackingRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::GainTrackingRowIDL;
#endif



#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;

#include <Frequency.h>
using  asdm::Frequency;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;

#include <Complex.h>
using  asdm::Complex;




	

	

	

	

	

	

	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file GainTracking.h
    \brief Generated from model's revision "1.53", branch "HEAD"
*/

namespace asdm {

//class asdm::GainTrackingTable;


// class asdm::AntennaRow;
class AntennaRow;

// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::FeedRow;
class FeedRow;
	

class GainTrackingRow;
typedef void (GainTrackingRow::*GainTrackingAttributeFromBin) (EndianISStream& eiss);

/**
 * The GainTrackingRow class is a row of a GainTrackingTable.
 * 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 */
class GainTrackingRow {
friend class asdm::GainTrackingTable;

public:

	virtual ~GainTrackingRow();

	/**
	 * Return the table to which this row belongs.
	 */
	GainTrackingTable &getTable() const;
	
	
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
  		
	
	
	


	
	// ===> Attribute attenuator
	
	
	

	
 	/**
 	 * Get attenuator.
 	 * @return attenuator as float
 	 */
 	float getAttenuator() const;
	
 
 	
 	
 	/**
 	 * Set attenuator with the specified float.
 	 * @param attenuator The float value to which attenuator is to be set.
 	 
 		
 			
 	 */
 	void setAttenuator (float attenuator);
  		
	
	
	


	
	// ===> Attribute numLO
	
	
	

	
 	/**
 	 * Get numLO.
 	 * @return numLO as int
 	 */
 	int getNumLO() const;
	
 
 	
 	
 	/**
 	 * Set numLO with the specified int.
 	 * @param numLO The int value to which numLO is to be set.
 	 
 		
 			
 	 */
 	void setNumLO (int numLO);
  		
	
	
	


	
	// ===> Attribute numReceptor
	
	
	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int getNumReceptor() const;
	
 
 	
 	
 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 		
 			
 	 */
 	void setNumReceptor (int numReceptor);
  		
	
	
	


	
	// ===> Attribute cableDelay
	
	
	

	
 	/**
 	 * Get cableDelay.
 	 * @return cableDelay as vector<double >
 	 */
 	vector<double > getCableDelay() const;
	
 
 	
 	
 	/**
 	 * Set cableDelay with the specified vector<double >.
 	 * @param cableDelay The vector<double > value to which cableDelay is to be set.
 	 
 		
 			
 	 */
 	void setCableDelay (vector<double > cableDelay);
  		
	
	
	


	
	// ===> Attribute crossPolarizationDelay
	
	
	

	
 	/**
 	 * Get crossPolarizationDelay.
 	 * @return crossPolarizationDelay as double
 	 */
 	double getCrossPolarizationDelay() const;
	
 
 	
 	
 	/**
 	 * Set crossPolarizationDelay with the specified double.
 	 * @param crossPolarizationDelay The double value to which crossPolarizationDelay is to be set.
 	 
 		
 			
 	 */
 	void setCrossPolarizationDelay (double crossPolarizationDelay);
  		
	
	
	


	
	// ===> Attribute loPropagationDelay
	
	
	

	
 	/**
 	 * Get loPropagationDelay.
 	 * @return loPropagationDelay as vector<double >
 	 */
 	vector<double > getLoPropagationDelay() const;
	
 
 	
 	
 	/**
 	 * Set loPropagationDelay with the specified vector<double >.
 	 * @param loPropagationDelay The vector<double > value to which loPropagationDelay is to be set.
 	 
 		
 			
 	 */
 	void setLoPropagationDelay (vector<double > loPropagationDelay);
  		
	
	
	


	
	// ===> Attribute polarizationTypes
	
	
	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > getPolarizationTypes() const;
	
 
 	
 	
 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 		
 			
 	 */
 	void setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes);
  		
	
	
	


	
	// ===> Attribute receiverDelay
	
	
	

	
 	/**
 	 * Get receiverDelay.
 	 * @return receiverDelay as vector<double >
 	 */
 	vector<double > getReceiverDelay() const;
	
 
 	
 	
 	/**
 	 * Set receiverDelay with the specified vector<double >.
 	 * @param receiverDelay The vector<double > value to which receiverDelay is to be set.
 	 
 		
 			
 	 */
 	void setReceiverDelay (vector<double > receiverDelay);
  		
	
	
	


	
	// ===> Attribute delayOffset, which is optional
	
	
	
	/**
	 * The attribute delayOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the delayOffset attribute exists. 
	 */
	bool isDelayOffsetExists() const;
	

	
 	/**
 	 * Get delayOffset, which is optional.
 	 * @return delayOffset as double
 	 * @throws IllegalAccessException If delayOffset does not exist.
 	 */
 	double getDelayOffset() const;
	
 
 	
 	
 	/**
 	 * Set delayOffset with the specified double.
 	 * @param delayOffset The double value to which delayOffset is to be set.
 	 
 		
 	 */
 	void setDelayOffset (double delayOffset);
		
	
	
	
	/**
	 * Mark delayOffset, which is an optional field, as non-existent.
	 */
	void clearDelayOffset ();
	


	
	// ===> Attribute freqOffset, which is optional
	
	
	
	/**
	 * The attribute freqOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the freqOffset attribute exists. 
	 */
	bool isFreqOffsetExists() const;
	

	
 	/**
 	 * Get freqOffset, which is optional.
 	 * @return freqOffset as vector<Frequency >
 	 * @throws IllegalAccessException If freqOffset does not exist.
 	 */
 	vector<Frequency > getFreqOffset() const;
	
 
 	
 	
 	/**
 	 * Set freqOffset with the specified vector<Frequency >.
 	 * @param freqOffset The vector<Frequency > value to which freqOffset is to be set.
 	 
 		
 	 */
 	void setFreqOffset (vector<Frequency > freqOffset);
		
	
	
	
	/**
	 * Mark freqOffset, which is an optional field, as non-existent.
	 */
	void clearFreqOffset ();
	


	
	// ===> Attribute phaseOffset, which is optional
	
	
	
	/**
	 * The attribute phaseOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseOffset attribute exists. 
	 */
	bool isPhaseOffsetExists() const;
	

	
 	/**
 	 * Get phaseOffset, which is optional.
 	 * @return phaseOffset as vector<Angle >
 	 * @throws IllegalAccessException If phaseOffset does not exist.
 	 */
 	vector<Angle > getPhaseOffset() const;
	
 
 	
 	
 	/**
 	 * Set phaseOffset with the specified vector<Angle >.
 	 * @param phaseOffset The vector<Angle > value to which phaseOffset is to be set.
 	 
 		
 	 */
 	void setPhaseOffset (vector<Angle > phaseOffset);
		
	
	
	
	/**
	 * Mark phaseOffset, which is an optional field, as non-existent.
	 */
	void clearPhaseOffset ();
	


	
	// ===> Attribute samplingLevel, which is optional
	
	
	
	/**
	 * The attribute samplingLevel is optional. Return true if this attribute exists.
	 * @return true if and only if the samplingLevel attribute exists. 
	 */
	bool isSamplingLevelExists() const;
	

	
 	/**
 	 * Get samplingLevel, which is optional.
 	 * @return samplingLevel as float
 	 * @throws IllegalAccessException If samplingLevel does not exist.
 	 */
 	float getSamplingLevel() const;
	
 
 	
 	
 	/**
 	 * Set samplingLevel with the specified float.
 	 * @param samplingLevel The float value to which samplingLevel is to be set.
 	 
 		
 	 */
 	void setSamplingLevel (float samplingLevel);
		
	
	
	
	/**
	 * Mark samplingLevel, which is an optional field, as non-existent.
	 */
	void clearSamplingLevel ();
	


	
	// ===> Attribute numAttFreq, which is optional
	
	
	
	/**
	 * The attribute numAttFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the numAttFreq attribute exists. 
	 */
	bool isNumAttFreqExists() const;
	

	
 	/**
 	 * Get numAttFreq, which is optional.
 	 * @return numAttFreq as int
 	 * @throws IllegalAccessException If numAttFreq does not exist.
 	 */
 	int getNumAttFreq() const;
	
 
 	
 	
 	/**
 	 * Set numAttFreq with the specified int.
 	 * @param numAttFreq The int value to which numAttFreq is to be set.
 	 
 		
 	 */
 	void setNumAttFreq (int numAttFreq);
		
	
	
	
	/**
	 * Mark numAttFreq, which is an optional field, as non-existent.
	 */
	void clearNumAttFreq ();
	


	
	// ===> Attribute attFreq, which is optional
	
	
	
	/**
	 * The attribute attFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the attFreq attribute exists. 
	 */
	bool isAttFreqExists() const;
	

	
 	/**
 	 * Get attFreq, which is optional.
 	 * @return attFreq as vector<double >
 	 * @throws IllegalAccessException If attFreq does not exist.
 	 */
 	vector<double > getAttFreq() const;
	
 
 	
 	
 	/**
 	 * Set attFreq with the specified vector<double >.
 	 * @param attFreq The vector<double > value to which attFreq is to be set.
 	 
 		
 	 */
 	void setAttFreq (vector<double > attFreq);
		
	
	
	
	/**
	 * Mark attFreq, which is an optional field, as non-existent.
	 */
	void clearAttFreq ();
	


	
	// ===> Attribute attSpectrum, which is optional
	
	
	
	/**
	 * The attribute attSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the attSpectrum attribute exists. 
	 */
	bool isAttSpectrumExists() const;
	

	
 	/**
 	 * Get attSpectrum, which is optional.
 	 * @return attSpectrum as vector<Complex >
 	 * @throws IllegalAccessException If attSpectrum does not exist.
 	 */
 	vector<Complex > getAttSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set attSpectrum with the specified vector<Complex >.
 	 * @param attSpectrum The vector<Complex > value to which attSpectrum is to be set.
 	 
 		
 	 */
 	void setAttSpectrum (vector<Complex > attSpectrum);
		
	
	
	
	/**
	 * Mark attSpectrum, which is an optional field, as non-existent.
	 */
	void clearAttSpectrum ();
	


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
  		
	
	
	


	
	// ===> Attribute feedId
	
	
	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int getFeedId() const;
	
 
 	
 	
 	/**
 	 * Set feedId with the specified int.
 	 * @param feedId The int value to which feedId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setFeedId (int feedId);
  		
	
	
	


	
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
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	

	
		
	/**
	 * spectralWindowId pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* getSpectralWindowUsingSpectralWindowId();
	 

	

	

	
		
	// ===> Slice link from a row of GainTracking table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having feedId == this.feedId
	 * 
	 * @return a vector of FeedRow *
	 */
	vector <FeedRow *> getFeeds();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this GainTrackingRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaId
	    
	 * @param spectralWindowId
	    
	 * @param timeInterval
	    
	 * @param feedId
	    
	 * @param attenuator
	    
	 * @param numLO
	    
	 * @param numReceptor
	    
	 * @param cableDelay
	    
	 * @param crossPolarizationDelay
	    
	 * @param loPropagationDelay
	    
	 * @param polarizationTypes
	    
	 * @param receiverDelay
	    
	 */ 
	bool compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, float attenuator, int numLO, int numReceptor, vector<double > cableDelay, double crossPolarizationDelay, vector<double > loPropagationDelay, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > receiverDelay);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param attenuator
	    
	 * @param numLO
	    
	 * @param numReceptor
	    
	 * @param cableDelay
	    
	 * @param crossPolarizationDelay
	    
	 * @param loPropagationDelay
	    
	 * @param polarizationTypes
	    
	 * @param receiverDelay
	    
	 */ 
	bool compareRequiredValue(float attenuator, int numLO, int numReceptor, vector<double > cableDelay, double crossPolarizationDelay, vector<double > loPropagationDelay, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > receiverDelay); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the GainTrackingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(GainTrackingRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a GainTrackingRowIDL struct.
	 */
	GainTrackingRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct GainTrackingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (GainTrackingRowIDL x) ;
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
	GainTrackingTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a GainTrackingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	GainTrackingRow (GainTrackingTable &table);

	/**
	 * Create a GainTrackingRow using a copy constructor mechanism.
	 * <p>
	 * Given a GainTrackingRow row and a GainTrackingTable table, the method creates a new
	 * GainTrackingRow owned by table. Each attribute of the created row is a copy (deep)
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
	 GainTrackingRow (GainTrackingTable &table, GainTrackingRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute attenuator
	
	

	float attenuator;

	
	
 	

	
	// ===> Attribute numLO
	
	

	int numLO;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute cableDelay
	
	

	vector<double > cableDelay;

	
	
 	

	
	// ===> Attribute crossPolarizationDelay
	
	

	double crossPolarizationDelay;

	
	
 	

	
	// ===> Attribute loPropagationDelay
	
	

	vector<double > loPropagationDelay;

	
	
 	

	
	// ===> Attribute polarizationTypes
	
	

	vector<PolarizationTypeMod::PolarizationType > polarizationTypes;

	
	
 	

	
	// ===> Attribute receiverDelay
	
	

	vector<double > receiverDelay;

	
	
 	

	
	// ===> Attribute delayOffset, which is optional
	
	
	bool delayOffsetExists;
	

	double delayOffset;

	
	
 	

	
	// ===> Attribute freqOffset, which is optional
	
	
	bool freqOffsetExists;
	

	vector<Frequency > freqOffset;

	
	
 	

	
	// ===> Attribute phaseOffset, which is optional
	
	
	bool phaseOffsetExists;
	

	vector<Angle > phaseOffset;

	
	
 	

	
	// ===> Attribute samplingLevel, which is optional
	
	
	bool samplingLevelExists;
	

	float samplingLevel;

	
	
 	

	
	// ===> Attribute numAttFreq, which is optional
	
	
	bool numAttFreqExists;
	

	int numAttFreq;

	
	
 	

	
	// ===> Attribute attFreq, which is optional
	
	
	bool attFreqExists;
	

	vector<double > attFreq;

	
	
 	

	
	// ===> Attribute attSpectrum, which is optional
	
	
	bool attSpectrumExists;
	

	vector<Complex > attSpectrum;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute antennaId
	
	

	Tag antennaId;

	
	
 	

	
	// ===> Attribute feedId
	
	

	int feedId;

	
	
 	

	
	// ===> Attribute spectralWindowId
	
	

	Tag spectralWindowId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
		

	 

	

	
		


	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, GainTrackingAttributeFromBin> fromBinMethods;
void antennaIdFromBin( EndianISStream& eiss);
void spectralWindowIdFromBin( EndianISStream& eiss);
void timeIntervalFromBin( EndianISStream& eiss);
void feedIdFromBin( EndianISStream& eiss);
void attenuatorFromBin( EndianISStream& eiss);
void numLOFromBin( EndianISStream& eiss);
void numReceptorFromBin( EndianISStream& eiss);
void cableDelayFromBin( EndianISStream& eiss);
void crossPolarizationDelayFromBin( EndianISStream& eiss);
void loPropagationDelayFromBin( EndianISStream& eiss);
void polarizationTypesFromBin( EndianISStream& eiss);
void receiverDelayFromBin( EndianISStream& eiss);

void delayOffsetFromBin( EndianISStream& eiss);
void freqOffsetFromBin( EndianISStream& eiss);
void phaseOffsetFromBin( EndianISStream& eiss);
void samplingLevelFromBin( EndianISStream& eiss);
void numAttFreqFromBin( EndianISStream& eiss);
void attFreqFromBin( EndianISStream& eiss);
void attSpectrumFromBin( EndianISStream& eiss);
	
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the GainTrackingTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static GainTrackingRow* fromBin(EndianISStream& eiss, GainTrackingTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* GainTracking_CLASS */
