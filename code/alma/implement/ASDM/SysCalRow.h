
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
 * File SysCalRow.h
 */
 
#ifndef SysCalRow_CLASS
#define SysCalRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::SysCalRowIDL;
#endif



#include <Tag.h>
using  asdm::Tag;

#include <Temperature.h>
using  asdm::Temperature;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;




	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file SysCal.h
    \brief Generated from model's revision "1.53", branch "HEAD"
*/

namespace asdm {

//class asdm::SysCalTable;


// class asdm::AntennaRow;
class AntennaRow;

// class asdm::FeedRow;
class FeedRow;

// class asdm::SpectralWindowRow;
class SpectralWindowRow;
	

class SysCalRow;
typedef void (SysCalRow::*SysCalAttributeFromBin) (EndianISStream& eiss);

/**
 * The SysCalRow class is a row of a SysCalTable.
 * 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 */
class SysCalRow {
friend class asdm::SysCalTable;

public:

	virtual ~SysCalRow();

	/**
	 * Return the table to which this row belongs.
	 */
	SysCalTable &getTable() const;
	
	
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
  		
	
	
	


	
	// ===> Attribute tcalFlag, which is optional
	
	
	
	/**
	 * The attribute tcalFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tcalFlag attribute exists. 
	 */
	bool isTcalFlagExists() const;
	

	
 	/**
 	 * Get tcalFlag, which is optional.
 	 * @return tcalFlag as bool
 	 * @throws IllegalAccessException If tcalFlag does not exist.
 	 */
 	bool getTcalFlag() const;
	
 
 	
 	
 	/**
 	 * Set tcalFlag with the specified bool.
 	 * @param tcalFlag The bool value to which tcalFlag is to be set.
 	 
 		
 	 */
 	void setTcalFlag (bool tcalFlag);
		
	
	
	
	/**
	 * Mark tcalFlag, which is an optional field, as non-existent.
	 */
	void clearTcalFlag ();
	


	
	// ===> Attribute tcalSpectrum, which is optional
	
	
	
	/**
	 * The attribute tcalSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tcalSpectrum attribute exists. 
	 */
	bool isTcalSpectrumExists() const;
	

	
 	/**
 	 * Get tcalSpectrum, which is optional.
 	 * @return tcalSpectrum as vector<vector<Temperature > >
 	 * @throws IllegalAccessException If tcalSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > getTcalSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tcalSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tcalSpectrum The vector<vector<Temperature > > value to which tcalSpectrum is to be set.
 	 
 		
 	 */
 	void setTcalSpectrum (vector<vector<Temperature > > tcalSpectrum);
		
	
	
	
	/**
	 * Mark tcalSpectrum, which is an optional field, as non-existent.
	 */
	void clearTcalSpectrum ();
	


	
	// ===> Attribute trxFlag, which is optional
	
	
	
	/**
	 * The attribute trxFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the trxFlag attribute exists. 
	 */
	bool isTrxFlagExists() const;
	

	
 	/**
 	 * Get trxFlag, which is optional.
 	 * @return trxFlag as bool
 	 * @throws IllegalAccessException If trxFlag does not exist.
 	 */
 	bool getTrxFlag() const;
	
 
 	
 	
 	/**
 	 * Set trxFlag with the specified bool.
 	 * @param trxFlag The bool value to which trxFlag is to be set.
 	 
 		
 	 */
 	void setTrxFlag (bool trxFlag);
		
	
	
	
	/**
	 * Mark trxFlag, which is an optional field, as non-existent.
	 */
	void clearTrxFlag ();
	


	
	// ===> Attribute trxSpectrum, which is optional
	
	
	
	/**
	 * The attribute trxSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the trxSpectrum attribute exists. 
	 */
	bool isTrxSpectrumExists() const;
	

	
 	/**
 	 * Get trxSpectrum, which is optional.
 	 * @return trxSpectrum as vector<vector<Temperature > >
 	 * @throws IllegalAccessException If trxSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > getTrxSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set trxSpectrum with the specified vector<vector<Temperature > >.
 	 * @param trxSpectrum The vector<vector<Temperature > > value to which trxSpectrum is to be set.
 	 
 		
 	 */
 	void setTrxSpectrum (vector<vector<Temperature > > trxSpectrum);
		
	
	
	
	/**
	 * Mark trxSpectrum, which is an optional field, as non-existent.
	 */
	void clearTrxSpectrum ();
	


	
	// ===> Attribute tskyFlag, which is optional
	
	
	
	/**
	 * The attribute tskyFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tskyFlag attribute exists. 
	 */
	bool isTskyFlagExists() const;
	

	
 	/**
 	 * Get tskyFlag, which is optional.
 	 * @return tskyFlag as bool
 	 * @throws IllegalAccessException If tskyFlag does not exist.
 	 */
 	bool getTskyFlag() const;
	
 
 	
 	
 	/**
 	 * Set tskyFlag with the specified bool.
 	 * @param tskyFlag The bool value to which tskyFlag is to be set.
 	 
 		
 	 */
 	void setTskyFlag (bool tskyFlag);
		
	
	
	
	/**
	 * Mark tskyFlag, which is an optional field, as non-existent.
	 */
	void clearTskyFlag ();
	


	
	// ===> Attribute tskySpectrum, which is optional
	
	
	
	/**
	 * The attribute tskySpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tskySpectrum attribute exists. 
	 */
	bool isTskySpectrumExists() const;
	

	
 	/**
 	 * Get tskySpectrum, which is optional.
 	 * @return tskySpectrum as vector<vector<Temperature > >
 	 * @throws IllegalAccessException If tskySpectrum does not exist.
 	 */
 	vector<vector<Temperature > > getTskySpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tskySpectrum with the specified vector<vector<Temperature > >.
 	 * @param tskySpectrum The vector<vector<Temperature > > value to which tskySpectrum is to be set.
 	 
 		
 	 */
 	void setTskySpectrum (vector<vector<Temperature > > tskySpectrum);
		
	
	
	
	/**
	 * Mark tskySpectrum, which is an optional field, as non-existent.
	 */
	void clearTskySpectrum ();
	


	
	// ===> Attribute tsysFlag, which is optional
	
	
	
	/**
	 * The attribute tsysFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tsysFlag attribute exists. 
	 */
	bool isTsysFlagExists() const;
	

	
 	/**
 	 * Get tsysFlag, which is optional.
 	 * @return tsysFlag as bool
 	 * @throws IllegalAccessException If tsysFlag does not exist.
 	 */
 	bool getTsysFlag() const;
	
 
 	
 	
 	/**
 	 * Set tsysFlag with the specified bool.
 	 * @param tsysFlag The bool value to which tsysFlag is to be set.
 	 
 		
 	 */
 	void setTsysFlag (bool tsysFlag);
		
	
	
	
	/**
	 * Mark tsysFlag, which is an optional field, as non-existent.
	 */
	void clearTsysFlag ();
	


	
	// ===> Attribute tsysSpectrum, which is optional
	
	
	
	/**
	 * The attribute tsysSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tsysSpectrum attribute exists. 
	 */
	bool isTsysSpectrumExists() const;
	

	
 	/**
 	 * Get tsysSpectrum, which is optional.
 	 * @return tsysSpectrum as vector<vector<Temperature > >
 	 * @throws IllegalAccessException If tsysSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > getTsysSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tsysSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tsysSpectrum The vector<vector<Temperature > > value to which tsysSpectrum is to be set.
 	 
 		
 	 */
 	void setTsysSpectrum (vector<vector<Temperature > > tsysSpectrum);
		
	
	
	
	/**
	 * Mark tsysSpectrum, which is an optional field, as non-existent.
	 */
	void clearTsysSpectrum ();
	


	
	// ===> Attribute tantFlag, which is optional
	
	
	
	/**
	 * The attribute tantFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tantFlag attribute exists. 
	 */
	bool isTantFlagExists() const;
	

	
 	/**
 	 * Get tantFlag, which is optional.
 	 * @return tantFlag as bool
 	 * @throws IllegalAccessException If tantFlag does not exist.
 	 */
 	bool getTantFlag() const;
	
 
 	
 	
 	/**
 	 * Set tantFlag with the specified bool.
 	 * @param tantFlag The bool value to which tantFlag is to be set.
 	 
 		
 	 */
 	void setTantFlag (bool tantFlag);
		
	
	
	
	/**
	 * Mark tantFlag, which is an optional field, as non-existent.
	 */
	void clearTantFlag ();
	


	
	// ===> Attribute tantSpectrum, which is optional
	
	
	
	/**
	 * The attribute tantSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tantSpectrum attribute exists. 
	 */
	bool isTantSpectrumExists() const;
	

	
 	/**
 	 * Get tantSpectrum, which is optional.
 	 * @return tantSpectrum as vector<vector<float > >
 	 * @throws IllegalAccessException If tantSpectrum does not exist.
 	 */
 	vector<vector<float > > getTantSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tantSpectrum with the specified vector<vector<float > >.
 	 * @param tantSpectrum The vector<vector<float > > value to which tantSpectrum is to be set.
 	 
 		
 	 */
 	void setTantSpectrum (vector<vector<float > > tantSpectrum);
		
	
	
	
	/**
	 * Mark tantSpectrum, which is an optional field, as non-existent.
	 */
	void clearTantSpectrum ();
	


	
	// ===> Attribute tantTsysFlag, which is optional
	
	
	
	/**
	 * The attribute tantTsysFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tantTsysFlag attribute exists. 
	 */
	bool isTantTsysFlagExists() const;
	

	
 	/**
 	 * Get tantTsysFlag, which is optional.
 	 * @return tantTsysFlag as bool
 	 * @throws IllegalAccessException If tantTsysFlag does not exist.
 	 */
 	bool getTantTsysFlag() const;
	
 
 	
 	
 	/**
 	 * Set tantTsysFlag with the specified bool.
 	 * @param tantTsysFlag The bool value to which tantTsysFlag is to be set.
 	 
 		
 	 */
 	void setTantTsysFlag (bool tantTsysFlag);
		
	
	
	
	/**
	 * Mark tantTsysFlag, which is an optional field, as non-existent.
	 */
	void clearTantTsysFlag ();
	


	
	// ===> Attribute tantTsysSpectrum, which is optional
	
	
	
	/**
	 * The attribute tantTsysSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tantTsysSpectrum attribute exists. 
	 */
	bool isTantTsysSpectrumExists() const;
	

	
 	/**
 	 * Get tantTsysSpectrum, which is optional.
 	 * @return tantTsysSpectrum as vector<vector<float > >
 	 * @throws IllegalAccessException If tantTsysSpectrum does not exist.
 	 */
 	vector<vector<float > > getTantTsysSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set tantTsysSpectrum with the specified vector<vector<float > >.
 	 * @param tantTsysSpectrum The vector<vector<float > > value to which tantTsysSpectrum is to be set.
 	 
 		
 	 */
 	void setTantTsysSpectrum (vector<vector<float > > tantTsysSpectrum);
		
	
	
	
	/**
	 * Mark tantTsysSpectrum, which is an optional field, as non-existent.
	 */
	void clearTantTsysSpectrum ();
	


	
	// ===> Attribute phaseDiffFlag, which is optional
	
	
	
	/**
	 * The attribute phaseDiffFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseDiffFlag attribute exists. 
	 */
	bool isPhaseDiffFlagExists() const;
	

	
 	/**
 	 * Get phaseDiffFlag, which is optional.
 	 * @return phaseDiffFlag as bool
 	 * @throws IllegalAccessException If phaseDiffFlag does not exist.
 	 */
 	bool getPhaseDiffFlag() const;
	
 
 	
 	
 	/**
 	 * Set phaseDiffFlag with the specified bool.
 	 * @param phaseDiffFlag The bool value to which phaseDiffFlag is to be set.
 	 
 		
 	 */
 	void setPhaseDiffFlag (bool phaseDiffFlag);
		
	
	
	
	/**
	 * Mark phaseDiffFlag, which is an optional field, as non-existent.
	 */
	void clearPhaseDiffFlag ();
	


	
	// ===> Attribute phaseDiffSpectrum, which is optional
	
	
	
	/**
	 * The attribute phaseDiffSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseDiffSpectrum attribute exists. 
	 */
	bool isPhaseDiffSpectrumExists() const;
	

	
 	/**
 	 * Get phaseDiffSpectrum, which is optional.
 	 * @return phaseDiffSpectrum as vector<vector<float > >
 	 * @throws IllegalAccessException If phaseDiffSpectrum does not exist.
 	 */
 	vector<vector<float > > getPhaseDiffSpectrum() const;
	
 
 	
 	
 	/**
 	 * Set phaseDiffSpectrum with the specified vector<vector<float > >.
 	 * @param phaseDiffSpectrum The vector<vector<float > > value to which phaseDiffSpectrum is to be set.
 	 
 		
 	 */
 	void setPhaseDiffSpectrum (vector<vector<float > > phaseDiffSpectrum);
		
	
	
	
	/**
	 * Mark phaseDiffSpectrum, which is an optional field, as non-existent.
	 */
	void clearPhaseDiffSpectrum ();
	


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
	 

	

	

	
		
	// ===> Slice link from a row of SysCal table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having feedId == this.feedId
	 * 
	 * @return a vector of FeedRow *
	 */
	vector <FeedRow *> getFeeds();
	
	

	

	

	
		
	/**
	 * spectralWindowId pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* getSpectralWindowUsingSpectralWindowId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SysCalRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param antennaId
	    
	 * @param spectralWindowId
	    
	 * @param timeInterval
	    
	 * @param feedId
	    
	 * @param numReceptor
	    
	 * @param numChan
	    
	 */ 
	bool compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, int numReceptor, int numChan);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param numReceptor
	    
	 * @param numChan
	    
	 */ 
	bool compareRequiredValue(int numReceptor, int numChan); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SysCalRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(SysCalRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	SysCalTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a SysCalRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SysCalRow (SysCalTable &table);

	/**
	 * Create a SysCalRow using a copy constructor mechanism.
	 * <p>
	 * Given a SysCalRow row and a SysCalTable table, the method creates a new
	 * SysCalRow owned by table. Each attribute of the created row is a copy (deep)
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
	 SysCalRow (SysCalTable &table, SysCalRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute numReceptor
	
	

	int numReceptor;

	
	
 	

	
	// ===> Attribute numChan
	
	

	int numChan;

	
	
 	

	
	// ===> Attribute tcalFlag, which is optional
	
	
	bool tcalFlagExists;
	

	bool tcalFlag;

	
	
 	

	
	// ===> Attribute tcalSpectrum, which is optional
	
	
	bool tcalSpectrumExists;
	

	vector<vector<Temperature > > tcalSpectrum;

	
	
 	

	
	// ===> Attribute trxFlag, which is optional
	
	
	bool trxFlagExists;
	

	bool trxFlag;

	
	
 	

	
	// ===> Attribute trxSpectrum, which is optional
	
	
	bool trxSpectrumExists;
	

	vector<vector<Temperature > > trxSpectrum;

	
	
 	

	
	// ===> Attribute tskyFlag, which is optional
	
	
	bool tskyFlagExists;
	

	bool tskyFlag;

	
	
 	

	
	// ===> Attribute tskySpectrum, which is optional
	
	
	bool tskySpectrumExists;
	

	vector<vector<Temperature > > tskySpectrum;

	
	
 	

	
	// ===> Attribute tsysFlag, which is optional
	
	
	bool tsysFlagExists;
	

	bool tsysFlag;

	
	
 	

	
	// ===> Attribute tsysSpectrum, which is optional
	
	
	bool tsysSpectrumExists;
	

	vector<vector<Temperature > > tsysSpectrum;

	
	
 	

	
	// ===> Attribute tantFlag, which is optional
	
	
	bool tantFlagExists;
	

	bool tantFlag;

	
	
 	

	
	// ===> Attribute tantSpectrum, which is optional
	
	
	bool tantSpectrumExists;
	

	vector<vector<float > > tantSpectrum;

	
	
 	

	
	// ===> Attribute tantTsysFlag, which is optional
	
	
	bool tantTsysFlagExists;
	

	bool tantTsysFlag;

	
	
 	

	
	// ===> Attribute tantTsysSpectrum, which is optional
	
	
	bool tantTsysSpectrumExists;
	

	vector<vector<float > > tantTsysSpectrum;

	
	
 	

	
	// ===> Attribute phaseDiffFlag, which is optional
	
	
	bool phaseDiffFlagExists;
	

	bool phaseDiffFlag;

	
	
 	

	
	// ===> Attribute phaseDiffSpectrum, which is optional
	
	
	bool phaseDiffSpectrumExists;
	

	vector<vector<float > > phaseDiffSpectrum;

	
	
 	

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
	map<string, SysCalAttributeFromBin> fromBinMethods;
void antennaIdFromBin( EndianISStream& eiss);
void spectralWindowIdFromBin( EndianISStream& eiss);
void timeIntervalFromBin( EndianISStream& eiss);
void feedIdFromBin( EndianISStream& eiss);
void numReceptorFromBin( EndianISStream& eiss);
void numChanFromBin( EndianISStream& eiss);

void tcalFlagFromBin( EndianISStream& eiss);
void tcalSpectrumFromBin( EndianISStream& eiss);
void trxFlagFromBin( EndianISStream& eiss);
void trxSpectrumFromBin( EndianISStream& eiss);
void tskyFlagFromBin( EndianISStream& eiss);
void tskySpectrumFromBin( EndianISStream& eiss);
void tsysFlagFromBin( EndianISStream& eiss);
void tsysSpectrumFromBin( EndianISStream& eiss);
void tantFlagFromBin( EndianISStream& eiss);
void tantSpectrumFromBin( EndianISStream& eiss);
void tantTsysFlagFromBin( EndianISStream& eiss);
void tantTsysSpectrumFromBin( EndianISStream& eiss);
void phaseDiffFlagFromBin( EndianISStream& eiss);
void phaseDiffSpectrumFromBin( EndianISStream& eiss);
	

#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SysCalRowIDL struct.
	 */
	SysCalRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SysCalRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (SysCalRowIDL x) ;
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
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the SysCalTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static SysCalRow* fromBin(EndianISStream& eiss, SysCalTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* SysCal_CLASS */
