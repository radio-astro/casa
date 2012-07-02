
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

/*\file SysCal.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::SysCalTable;


// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::FeedRow;
class FeedRow;

// class asdm::AntennaRow;
class AntennaRow;
	

/**
 * The SysCalRow class is a row of a SysCalTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
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
	 */
	void setFromIDL (SysCalRowIDL x) throw(ConversionException);
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
  		
	
	
	


	
	// ===> Attribute numLoad, which is optional
	
	
	
	/**
	 * The attribute numLoad is optional. Return true if this attribute exists.
	 * @return true if and only if the numLoad attribute exists. 
	 */
	bool isNumLoadExists() const;
	

	
 	/**
 	 * Get numLoad, which is optional.
 	 * @return numLoad as int
 	 * @throws IllegalAccessException If numLoad does not exist.
 	 */
 	int getNumLoad() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set numLoad with the specified int.
 	 * @param numLoad The int value to which numLoad is to be set.
 	 
 		
 	 */
 	void setNumLoad (int numLoad);
		
	
	
	
	/**
	 * Mark numLoad, which is an optional field, as non-existent.
	 */
	void clearNumLoad ();
	


	
	// ===> Attribute calLoad, which is optional
	
	
	
	/**
	 * The attribute calLoad is optional. Return true if this attribute exists.
	 * @return true if and only if the calLoad attribute exists. 
	 */
	bool isCalLoadExists() const;
	

	
 	/**
 	 * Get calLoad, which is optional.
 	 * @return calLoad as vector<int >
 	 * @throws IllegalAccessException If calLoad does not exist.
 	 */
 	vector<int > getCalLoad() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set calLoad with the specified vector<int >.
 	 * @param calLoad The vector<int > value to which calLoad is to be set.
 	 
 		
 	 */
 	void setCalLoad (vector<int > calLoad);
		
	
	
	
	/**
	 * Mark calLoad, which is an optional field, as non-existent.
	 */
	void clearCalLoad ();
	


	
	// ===> Attribute feff, which is optional
	
	
	
	/**
	 * The attribute feff is optional. Return true if this attribute exists.
	 * @return true if and only if the feff attribute exists. 
	 */
	bool isFeffExists() const;
	

	
 	/**
 	 * Get feff, which is optional.
 	 * @return feff as vector<float >
 	 * @throws IllegalAccessException If feff does not exist.
 	 */
 	vector<float > getFeff() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set feff with the specified vector<float >.
 	 * @param feff The vector<float > value to which feff is to be set.
 	 
 		
 	 */
 	void setFeff (vector<float > feff);
		
	
	
	
	/**
	 * Mark feff, which is an optional field, as non-existent.
	 */
	void clearFeff ();
	


	
	// ===> Attribute aeff, which is optional
	
	
	
	/**
	 * The attribute aeff is optional. Return true if this attribute exists.
	 * @return true if and only if the aeff attribute exists. 
	 */
	bool isAeffExists() const;
	

	
 	/**
 	 * Get aeff, which is optional.
 	 * @return aeff as vector<float >
 	 * @throws IllegalAccessException If aeff does not exist.
 	 */
 	vector<float > getAeff() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set aeff with the specified vector<float >.
 	 * @param aeff The vector<float > value to which aeff is to be set.
 	 
 		
 	 */
 	void setAeff (vector<float > aeff);
		
	
	
	
	/**
	 * Mark aeff, which is an optional field, as non-existent.
	 */
	void clearAeff ();
	


	
	// ===> Attribute phaseDiff, which is optional
	
	
	
	/**
	 * The attribute phaseDiff is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseDiff attribute exists. 
	 */
	bool isPhaseDiffExists() const;
	

	
 	/**
 	 * Get phaseDiff, which is optional.
 	 * @return phaseDiff as Angle
 	 * @throws IllegalAccessException If phaseDiff does not exist.
 	 */
 	Angle getPhaseDiff() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set phaseDiff with the specified Angle.
 	 * @param phaseDiff The Angle value to which phaseDiff is to be set.
 	 
 		
 	 */
 	void setPhaseDiff (Angle phaseDiff);
		
	
	
	
	/**
	 * Mark phaseDiff, which is an optional field, as non-existent.
	 */
	void clearPhaseDiff ();
	


	
	// ===> Attribute sbgain, which is optional
	
	
	
	/**
	 * The attribute sbgain is optional. Return true if this attribute exists.
	 * @return true if and only if the sbgain attribute exists. 
	 */
	bool isSbgainExists() const;
	

	
 	/**
 	 * Get sbgain, which is optional.
 	 * @return sbgain as float
 	 * @throws IllegalAccessException If sbgain does not exist.
 	 */
 	float getSbgain() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sbgain with the specified float.
 	 * @param sbgain The float value to which sbgain is to be set.
 	 
 		
 	 */
 	void setSbgain (float sbgain);
		
	
	
	
	/**
	 * Mark sbgain, which is an optional field, as non-existent.
	 */
	void clearSbgain ();
	


	
	// ===> Attribute tau, which is optional
	
	
	
	/**
	 * The attribute tau is optional. Return true if this attribute exists.
	 * @return true if and only if the tau attribute exists. 
	 */
	bool isTauExists() const;
	

	
 	/**
 	 * Get tau, which is optional.
 	 * @return tau as vector<Temperature >
 	 * @throws IllegalAccessException If tau does not exist.
 	 */
 	vector<Temperature > getTau() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tau with the specified vector<Temperature >.
 	 * @param tau The vector<Temperature > value to which tau is to be set.
 	 
 		
 	 */
 	void setTau (vector<Temperature > tau);
		
	
	
	
	/**
	 * Mark tau, which is an optional field, as non-existent.
	 */
	void clearTau ();
	


	
	// ===> Attribute tcal, which is optional
	
	
	
	/**
	 * The attribute tcal is optional. Return true if this attribute exists.
	 * @return true if and only if the tcal attribute exists. 
	 */
	bool isTcalExists() const;
	

	
 	/**
 	 * Get tcal, which is optional.
 	 * @return tcal as vector<Temperature >
 	 * @throws IllegalAccessException If tcal does not exist.
 	 */
 	vector<Temperature > getTcal() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tcal with the specified vector<Temperature >.
 	 * @param tcal The vector<Temperature > value to which tcal is to be set.
 	 
 		
 	 */
 	void setTcal (vector<Temperature > tcal);
		
	
	
	
	/**
	 * Mark tcal, which is an optional field, as non-existent.
	 */
	void clearTcal ();
	


	
	// ===> Attribute trx, which is optional
	
	
	
	/**
	 * The attribute trx is optional. Return true if this attribute exists.
	 * @return true if and only if the trx attribute exists. 
	 */
	bool isTrxExists() const;
	

	
 	/**
 	 * Get trx, which is optional.
 	 * @return trx as vector<Temperature >
 	 * @throws IllegalAccessException If trx does not exist.
 	 */
 	vector<Temperature > getTrx() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set trx with the specified vector<Temperature >.
 	 * @param trx The vector<Temperature > value to which trx is to be set.
 	 
 		
 	 */
 	void setTrx (vector<Temperature > trx);
		
	
	
	
	/**
	 * Mark trx, which is an optional field, as non-existent.
	 */
	void clearTrx ();
	


	
	// ===> Attribute tsky, which is optional
	
	
	
	/**
	 * The attribute tsky is optional. Return true if this attribute exists.
	 * @return true if and only if the tsky attribute exists. 
	 */
	bool isTskyExists() const;
	

	
 	/**
 	 * Get tsky, which is optional.
 	 * @return tsky as vector<Temperature >
 	 * @throws IllegalAccessException If tsky does not exist.
 	 */
 	vector<Temperature > getTsky() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tsky with the specified vector<Temperature >.
 	 * @param tsky The vector<Temperature > value to which tsky is to be set.
 	 
 		
 	 */
 	void setTsky (vector<Temperature > tsky);
		
	
	
	
	/**
	 * Mark tsky, which is an optional field, as non-existent.
	 */
	void clearTsky ();
	


	
	// ===> Attribute tsys, which is optional
	
	
	
	/**
	 * The attribute tsys is optional. Return true if this attribute exists.
	 * @return true if and only if the tsys attribute exists. 
	 */
	bool isTsysExists() const;
	

	
 	/**
 	 * Get tsys, which is optional.
 	 * @return tsys as vector<Temperature >
 	 * @throws IllegalAccessException If tsys does not exist.
 	 */
 	vector<Temperature > getTsys() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tsys with the specified vector<Temperature >.
 	 * @param tsys The vector<Temperature > value to which tsys is to be set.
 	 
 		
 	 */
 	void setTsys (vector<Temperature > tsys);
		
	
	
	
	/**
	 * Mark tsys, which is an optional field, as non-existent.
	 */
	void clearTsys ();
	


	
	// ===> Attribute tant, which is optional
	
	
	
	/**
	 * The attribute tant is optional. Return true if this attribute exists.
	 * @return true if and only if the tant attribute exists. 
	 */
	bool isTantExists() const;
	

	
 	/**
 	 * Get tant, which is optional.
 	 * @return tant as vector<Temperature >
 	 * @throws IllegalAccessException If tant does not exist.
 	 */
 	vector<Temperature > getTant() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tant with the specified vector<Temperature >.
 	 * @param tant The vector<Temperature > value to which tant is to be set.
 	 
 		
 	 */
 	void setTant (vector<Temperature > tant);
		
	
	
	
	/**
	 * Mark tant, which is an optional field, as non-existent.
	 */
	void clearTant ();
	


	
	// ===> Attribute tantTsys, which is optional
	
	
	
	/**
	 * The attribute tantTsys is optional. Return true if this attribute exists.
	 * @return true if and only if the tantTsys attribute exists. 
	 */
	bool isTantTsysExists() const;
	

	
 	/**
 	 * Get tantTsys, which is optional.
 	 * @return tantTsys as vector<float >
 	 * @throws IllegalAccessException If tantTsys does not exist.
 	 */
 	vector<float > getTantTsys() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tantTsys with the specified vector<float >.
 	 * @param tantTsys The vector<float > value to which tantTsys is to be set.
 	 
 		
 	 */
 	void setTantTsys (vector<float > tantTsys);
		
	
	
	
	/**
	 * Mark tantTsys, which is an optional field, as non-existent.
	 */
	void clearTantTsys ();
	


	
	// ===> Attribute pwvPath, which is optional
	
	
	
	/**
	 * The attribute pwvPath is optional. Return true if this attribute exists.
	 * @return true if and only if the pwvPath attribute exists. 
	 */
	bool isPwvPathExists() const;
	

	
 	/**
 	 * Get pwvPath, which is optional.
 	 * @return pwvPath as vector<Length >
 	 * @throws IllegalAccessException If pwvPath does not exist.
 	 */
 	vector<Length > getPwvPath() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set pwvPath with the specified vector<Length >.
 	 * @param pwvPath The vector<Length > value to which pwvPath is to be set.
 	 
 		
 	 */
 	void setPwvPath (vector<Length > pwvPath);
		
	
	
	
	/**
	 * Mark pwvPath, which is an optional field, as non-existent.
	 */
	void clearPwvPath ();
	


	
	// ===> Attribute dpwvPath, which is optional
	
	
	
	/**
	 * The attribute dpwvPath is optional. Return true if this attribute exists.
	 * @return true if and only if the dpwvPath attribute exists. 
	 */
	bool isDpwvPathExists() const;
	

	
 	/**
 	 * Get dpwvPath, which is optional.
 	 * @return dpwvPath as vector<float >
 	 * @throws IllegalAccessException If dpwvPath does not exist.
 	 */
 	vector<float > getDpwvPath() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set dpwvPath with the specified vector<float >.
 	 * @param dpwvPath The vector<float > value to which dpwvPath is to be set.
 	 
 		
 	 */
 	void setDpwvPath (vector<float > dpwvPath);
		
	
	
	
	/**
	 * Mark dpwvPath, which is an optional field, as non-existent.
	 */
	void clearDpwvPath ();
	


	
	// ===> Attribute feffSpectrum, which is optional
	
	
	
	/**
	 * The attribute feffSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the feffSpectrum attribute exists. 
	 */
	bool isFeffSpectrumExists() const;
	

	
 	/**
 	 * Get feffSpectrum, which is optional.
 	 * @return feffSpectrum as vector<vector<float > >
 	 * @throws IllegalAccessException If feffSpectrum does not exist.
 	 */
 	vector<vector<float > > getFeffSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set feffSpectrum with the specified vector<vector<float > >.
 	 * @param feffSpectrum The vector<vector<float > > value to which feffSpectrum is to be set.
 	 
 		
 	 */
 	void setFeffSpectrum (vector<vector<float > > feffSpectrum);
		
	
	
	
	/**
	 * Mark feffSpectrum, which is an optional field, as non-existent.
	 */
	void clearFeffSpectrum ();
	


	
	// ===> Attribute sbgainSpectrum, which is optional
	
	
	
	/**
	 * The attribute sbgainSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the sbgainSpectrum attribute exists. 
	 */
	bool isSbgainSpectrumExists() const;
	

	
 	/**
 	 * Get sbgainSpectrum, which is optional.
 	 * @return sbgainSpectrum as vector<vector<float > >
 	 * @throws IllegalAccessException If sbgainSpectrum does not exist.
 	 */
 	vector<vector<float > > getSbgainSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sbgainSpectrum with the specified vector<vector<float > >.
 	 * @param sbgainSpectrum The vector<vector<float > > value to which sbgainSpectrum is to be set.
 	 
 		
 	 */
 	void setSbgainSpectrum (vector<vector<float > > sbgainSpectrum);
		
	
	
	
	/**
	 * Mark sbgainSpectrum, which is an optional field, as non-existent.
	 */
	void clearSbgainSpectrum ();
	


	
	// ===> Attribute tauSpectrum, which is optional
	
	
	
	/**
	 * The attribute tauSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tauSpectrum attribute exists. 
	 */
	bool isTauSpectrumExists() const;
	

	
 	/**
 	 * Get tauSpectrum, which is optional.
 	 * @return tauSpectrum as vector<vector<Temperature > >
 	 * @throws IllegalAccessException If tauSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > getTauSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tauSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tauSpectrum The vector<vector<Temperature > > value to which tauSpectrum is to be set.
 	 
 		
 	 */
 	void setTauSpectrum (vector<vector<Temperature > > tauSpectrum);
		
	
	
	
	/**
	 * Mark tauSpectrum, which is an optional field, as non-existent.
	 */
	void clearTauSpectrum ();
	


	
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
 	vector<vector<Temperature > > getTcalSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tcalSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tcalSpectrum The vector<vector<Temperature > > value to which tcalSpectrum is to be set.
 	 
 		
 	 */
 	void setTcalSpectrum (vector<vector<Temperature > > tcalSpectrum);
		
	
	
	
	/**
	 * Mark tcalSpectrum, which is an optional field, as non-existent.
	 */
	void clearTcalSpectrum ();
	


	
	// ===> Attribute trxSpectrum, which is optional
	
	
	
	/**
	 * The attribute trxSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the trxSpectrum attribute exists. 
	 */
	bool isTrxSpectrumExists() const;
	

	
 	/**
 	 * Get trxSpectrum, which is optional.
 	 * @return trxSpectrum as vector<Temperature >
 	 * @throws IllegalAccessException If trxSpectrum does not exist.
 	 */
 	vector<Temperature > getTrxSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set trxSpectrum with the specified vector<Temperature >.
 	 * @param trxSpectrum The vector<Temperature > value to which trxSpectrum is to be set.
 	 
 		
 	 */
 	void setTrxSpectrum (vector<Temperature > trxSpectrum);
		
	
	
	
	/**
	 * Mark trxSpectrum, which is an optional field, as non-existent.
	 */
	void clearTrxSpectrum ();
	


	
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
 	vector<vector<Temperature > > getTskySpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tskySpectrum with the specified vector<vector<Temperature > >.
 	 * @param tskySpectrum The vector<vector<Temperature > > value to which tskySpectrum is to be set.
 	 
 		
 	 */
 	void setTskySpectrum (vector<vector<Temperature > > tskySpectrum);
		
	
	
	
	/**
	 * Mark tskySpectrum, which is an optional field, as non-existent.
	 */
	void clearTskySpectrum ();
	


	
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
 	vector<vector<Temperature > > getTsysSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tsysSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tsysSpectrum The vector<vector<Temperature > > value to which tsysSpectrum is to be set.
 	 
 		
 	 */
 	void setTsysSpectrum (vector<vector<Temperature > > tsysSpectrum);
		
	
	
	
	/**
	 * Mark tsysSpectrum, which is an optional field, as non-existent.
	 */
	void clearTsysSpectrum ();
	


	
	// ===> Attribute tantSpectrum, which is optional
	
	
	
	/**
	 * The attribute tantSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tantSpectrum attribute exists. 
	 */
	bool isTantSpectrumExists() const;
	

	
 	/**
 	 * Get tantSpectrum, which is optional.
 	 * @return tantSpectrum as vector<vector<Temperature > >
 	 * @throws IllegalAccessException If tantSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > getTantSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tantSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tantSpectrum The vector<vector<Temperature > > value to which tantSpectrum is to be set.
 	 
 		
 	 */
 	void setTantSpectrum (vector<vector<Temperature > > tantSpectrum);
		
	
	
	
	/**
	 * Mark tantSpectrum, which is an optional field, as non-existent.
	 */
	void clearTantSpectrum ();
	


	
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
 	vector<vector<float > > getTantTsysSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tantTsysSpectrum with the specified vector<vector<float > >.
 	 * @param tantTsysSpectrum The vector<vector<float > > value to which tantTsysSpectrum is to be set.
 	 
 		
 	 */
 	void setTantTsysSpectrum (vector<vector<float > > tantTsysSpectrum);
		
	
	
	
	/**
	 * Mark tantTsysSpectrum, which is an optional field, as non-existent.
	 */
	void clearTantTsysSpectrum ();
	


	
	// ===> Attribute pwvPathSpectrum, which is optional
	
	
	
	/**
	 * The attribute pwvPathSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the pwvPathSpectrum attribute exists. 
	 */
	bool isPwvPathSpectrumExists() const;
	

	
 	/**
 	 * Get pwvPathSpectrum, which is optional.
 	 * @return pwvPathSpectrum as vector<Length >
 	 * @throws IllegalAccessException If pwvPathSpectrum does not exist.
 	 */
 	vector<Length > getPwvPathSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set pwvPathSpectrum with the specified vector<Length >.
 	 * @param pwvPathSpectrum The vector<Length > value to which pwvPathSpectrum is to be set.
 	 
 		
 	 */
 	void setPwvPathSpectrum (vector<Length > pwvPathSpectrum);
		
	
	
	
	/**
	 * Mark pwvPathSpectrum, which is an optional field, as non-existent.
	 */
	void clearPwvPathSpectrum ();
	


	
	// ===> Attribute dpwvPathSpectrum, which is optional
	
	
	
	/**
	 * The attribute dpwvPathSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the dpwvPathSpectrum attribute exists. 
	 */
	bool isDpwvPathSpectrumExists() const;
	

	
 	/**
 	 * Get dpwvPathSpectrum, which is optional.
 	 * @return dpwvPathSpectrum as vector<float >
 	 * @throws IllegalAccessException If dpwvPathSpectrum does not exist.
 	 */
 	vector<float > getDpwvPathSpectrum() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set dpwvPathSpectrum with the specified vector<float >.
 	 * @param dpwvPathSpectrum The vector<float > value to which dpwvPathSpectrum is to be set.
 	 
 		
 	 */
 	void setDpwvPathSpectrum (vector<float > dpwvPathSpectrum);
		
	
	
	
	/**
	 * Mark dpwvPathSpectrum, which is an optional field, as non-existent.
	 */
	void clearDpwvPathSpectrum ();
	


	
	// ===> Attribute numPoly, which is optional
	
	
	
	/**
	 * The attribute numPoly is optional. Return true if this attribute exists.
	 * @return true if and only if the numPoly attribute exists. 
	 */
	bool isNumPolyExists() const;
	

	
 	/**
 	 * Get numPoly, which is optional.
 	 * @return numPoly as int
 	 * @throws IllegalAccessException If numPoly does not exist.
 	 */
 	int getNumPoly() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 		
 	 */
 	void setNumPoly (int numPoly);
		
	
	
	
	/**
	 * Mark numPoly, which is an optional field, as non-existent.
	 */
	void clearNumPoly ();
	


	
	// ===> Attribute numPolyFreq, which is optional
	
	
	
	/**
	 * The attribute numPolyFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the numPolyFreq attribute exists. 
	 */
	bool isNumPolyFreqExists() const;
	

	
 	/**
 	 * Get numPolyFreq, which is optional.
 	 * @return numPolyFreq as int
 	 * @throws IllegalAccessException If numPolyFreq does not exist.
 	 */
 	int getNumPolyFreq() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set numPolyFreq with the specified int.
 	 * @param numPolyFreq The int value to which numPolyFreq is to be set.
 	 
 		
 	 */
 	void setNumPolyFreq (int numPolyFreq);
		
	
	
	
	/**
	 * Mark numPolyFreq, which is an optional field, as non-existent.
	 */
	void clearNumPolyFreq ();
	


	
	// ===> Attribute timeOrigin, which is optional
	
	
	
	/**
	 * The attribute timeOrigin is optional. Return true if this attribute exists.
	 * @return true if and only if the timeOrigin attribute exists. 
	 */
	bool isTimeOriginExists() const;
	

	
 	/**
 	 * Get timeOrigin, which is optional.
 	 * @return timeOrigin as ArrayTime
 	 * @throws IllegalAccessException If timeOrigin does not exist.
 	 */
 	ArrayTime getTimeOrigin() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set timeOrigin with the specified ArrayTime.
 	 * @param timeOrigin The ArrayTime value to which timeOrigin is to be set.
 	 
 		
 	 */
 	void setTimeOrigin (ArrayTime timeOrigin);
		
	
	
	
	/**
	 * Mark timeOrigin, which is an optional field, as non-existent.
	 */
	void clearTimeOrigin ();
	


	
	// ===> Attribute freqOrigin, which is optional
	
	
	
	/**
	 * The attribute freqOrigin is optional. Return true if this attribute exists.
	 * @return true if and only if the freqOrigin attribute exists. 
	 */
	bool isFreqOriginExists() const;
	

	
 	/**
 	 * Get freqOrigin, which is optional.
 	 * @return freqOrigin as Frequency
 	 * @throws IllegalAccessException If freqOrigin does not exist.
 	 */
 	Frequency getFreqOrigin() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set freqOrigin with the specified Frequency.
 	 * @param freqOrigin The Frequency value to which freqOrigin is to be set.
 	 
 		
 	 */
 	void setFreqOrigin (Frequency freqOrigin);
		
	
	
	
	/**
	 * Mark freqOrigin, which is an optional field, as non-existent.
	 */
	void clearFreqOrigin ();
	


	
	// ===> Attribute phaseCurve, which is optional
	
	
	
	/**
	 * The attribute phaseCurve is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseCurve attribute exists. 
	 */
	bool isPhaseCurveExists() const;
	

	
 	/**
 	 * Get phaseCurve, which is optional.
 	 * @return phaseCurve as vector<vector<Angle > >
 	 * @throws IllegalAccessException If phaseCurve does not exist.
 	 */
 	vector<vector<Angle > > getPhaseCurve() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set phaseCurve with the specified vector<vector<Angle > >.
 	 * @param phaseCurve The vector<vector<Angle > > value to which phaseCurve is to be set.
 	 
 		
 	 */
 	void setPhaseCurve (vector<vector<Angle > > phaseCurve);
		
	
	
	
	/**
	 * Mark phaseCurve, which is an optional field, as non-existent.
	 */
	void clearPhaseCurve ();
	


	
	// ===> Attribute delayCurve, which is optional
	
	
	
	/**
	 * The attribute delayCurve is optional. Return true if this attribute exists.
	 * @return true if and only if the delayCurve attribute exists. 
	 */
	bool isDelayCurveExists() const;
	

	
 	/**
 	 * Get delayCurve, which is optional.
 	 * @return delayCurve as vector<vector<Interval > >
 	 * @throws IllegalAccessException If delayCurve does not exist.
 	 */
 	vector<vector<Interval > > getDelayCurve() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set delayCurve with the specified vector<vector<Interval > >.
 	 * @param delayCurve The vector<vector<Interval > > value to which delayCurve is to be set.
 	 
 		
 	 */
 	void setDelayCurve (vector<vector<Interval > > delayCurve);
		
	
	
	
	/**
	 * Mark delayCurve, which is an optional field, as non-existent.
	 */
	void clearDelayCurve ();
	


	
	// ===> Attribute ampliCurve, which is optional
	
	
	
	/**
	 * The attribute ampliCurve is optional. Return true if this attribute exists.
	 * @return true if and only if the ampliCurve attribute exists. 
	 */
	bool isAmpliCurveExists() const;
	

	
 	/**
 	 * Get ampliCurve, which is optional.
 	 * @return ampliCurve as vector<vector<float > >
 	 * @throws IllegalAccessException If ampliCurve does not exist.
 	 */
 	vector<vector<float > > getAmpliCurve() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set ampliCurve with the specified vector<vector<float > >.
 	 * @param ampliCurve The vector<vector<float > > value to which ampliCurve is to be set.
 	 
 		
 	 */
 	void setAmpliCurve (vector<vector<float > > ampliCurve);
		
	
	
	
	/**
	 * Mark ampliCurve, which is an optional field, as non-existent.
	 */
	void clearAmpliCurve ();
	


	
	// ===> Attribute bandpassCurve, which is optional
	
	
	
	/**
	 * The attribute bandpassCurve is optional. Return true if this attribute exists.
	 * @return true if and only if the bandpassCurve attribute exists. 
	 */
	bool isBandpassCurveExists() const;
	

	
 	/**
 	 * Get bandpassCurve, which is optional.
 	 * @return bandpassCurve as vector<vector<float > >
 	 * @throws IllegalAccessException If bandpassCurve does not exist.
 	 */
 	vector<vector<float > > getBandpassCurve() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set bandpassCurve with the specified vector<vector<float > >.
 	 * @param bandpassCurve The vector<vector<float > > value to which bandpassCurve is to be set.
 	 
 		
 	 */
 	void setBandpassCurve (vector<vector<float > > bandpassCurve);
		
	
	
	
	/**
	 * Mark bandpassCurve, which is an optional field, as non-existent.
	 */
	void clearBandpassCurve ();
	


	
	// ===> Attribute phasediffFlag, which is optional
	
	
	
	/**
	 * The attribute phasediffFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the phasediffFlag attribute exists. 
	 */
	bool isPhasediffFlagExists() const;
	

	
 	/**
 	 * Get phasediffFlag, which is optional.
 	 * @return phasediffFlag as bool
 	 * @throws IllegalAccessException If phasediffFlag does not exist.
 	 */
 	bool getPhasediffFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set phasediffFlag with the specified bool.
 	 * @param phasediffFlag The bool value to which phasediffFlag is to be set.
 	 
 		
 	 */
 	void setPhasediffFlag (bool phasediffFlag);
		
	
	
	
	/**
	 * Mark phasediffFlag, which is an optional field, as non-existent.
	 */
	void clearPhasediffFlag ();
	


	
	// ===> Attribute sbgainFlag, which is optional
	
	
	
	/**
	 * The attribute sbgainFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the sbgainFlag attribute exists. 
	 */
	bool isSbgainFlagExists() const;
	

	
 	/**
 	 * Get sbgainFlag, which is optional.
 	 * @return sbgainFlag as bool
 	 * @throws IllegalAccessException If sbgainFlag does not exist.
 	 */
 	bool getSbgainFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set sbgainFlag with the specified bool.
 	 * @param sbgainFlag The bool value to which sbgainFlag is to be set.
 	 
 		
 	 */
 	void setSbgainFlag (bool sbgainFlag);
		
	
	
	
	/**
	 * Mark sbgainFlag, which is an optional field, as non-existent.
	 */
	void clearSbgainFlag ();
	


	
	// ===> Attribute tauFlag, which is optional
	
	
	
	/**
	 * The attribute tauFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tauFlag attribute exists. 
	 */
	bool isTauFlagExists() const;
	

	
 	/**
 	 * Get tauFlag, which is optional.
 	 * @return tauFlag as bool
 	 * @throws IllegalAccessException If tauFlag does not exist.
 	 */
 	bool getTauFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tauFlag with the specified bool.
 	 * @param tauFlag The bool value to which tauFlag is to be set.
 	 
 		
 	 */
 	void setTauFlag (bool tauFlag);
		
	
	
	
	/**
	 * Mark tauFlag, which is an optional field, as non-existent.
	 */
	void clearTauFlag ();
	


	
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
 	bool getTcalFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tcalFlag with the specified bool.
 	 * @param tcalFlag The bool value to which tcalFlag is to be set.
 	 
 		
 	 */
 	void setTcalFlag (bool tcalFlag);
		
	
	
	
	/**
	 * Mark tcalFlag, which is an optional field, as non-existent.
	 */
	void clearTcalFlag ();
	


	
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
 	bool getTrxFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set trxFlag with the specified bool.
 	 * @param trxFlag The bool value to which trxFlag is to be set.
 	 
 		
 	 */
 	void setTrxFlag (bool trxFlag);
		
	
	
	
	/**
	 * Mark trxFlag, which is an optional field, as non-existent.
	 */
	void clearTrxFlag ();
	


	
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
 	bool getTskyFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tskyFlag with the specified bool.
 	 * @param tskyFlag The bool value to which tskyFlag is to be set.
 	 
 		
 	 */
 	void setTskyFlag (bool tskyFlag);
		
	
	
	
	/**
	 * Mark tskyFlag, which is an optional field, as non-existent.
	 */
	void clearTskyFlag ();
	


	
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
 	bool getTsysFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tsysFlag with the specified bool.
 	 * @param tsysFlag The bool value to which tsysFlag is to be set.
 	 
 		
 	 */
 	void setTsysFlag (bool tsysFlag);
		
	
	
	
	/**
	 * Mark tsysFlag, which is an optional field, as non-existent.
	 */
	void clearTsysFlag ();
	


	
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
 	bool getTantFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tantFlag with the specified bool.
 	 * @param tantFlag The bool value to which tantFlag is to be set.
 	 
 		
 	 */
 	void setTantFlag (bool tantFlag);
		
	
	
	
	/**
	 * Mark tantFlag, which is an optional field, as non-existent.
	 */
	void clearTantFlag ();
	


	
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
 	bool getTantTsysFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set tantTsysFlag with the specified bool.
 	 * @param tantTsysFlag The bool value to which tantTsysFlag is to be set.
 	 
 		
 	 */
 	void setTantTsysFlag (bool tantTsysFlag);
		
	
	
	
	/**
	 * Mark tantTsysFlag, which is an optional field, as non-existent.
	 */
	void clearTantTsysFlag ();
	


	
	// ===> Attribute pwvPathFlag, which is optional
	
	
	
	/**
	 * The attribute pwvPathFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the pwvPathFlag attribute exists. 
	 */
	bool isPwvPathFlagExists() const;
	

	
 	/**
 	 * Get pwvPathFlag, which is optional.
 	 * @return pwvPathFlag as bool
 	 * @throws IllegalAccessException If pwvPathFlag does not exist.
 	 */
 	bool getPwvPathFlag() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set pwvPathFlag with the specified bool.
 	 * @param pwvPathFlag The bool value to which pwvPathFlag is to be set.
 	 
 		
 	 */
 	void setPwvPathFlag (bool pwvPathFlag);
		
	
	
	
	/**
	 * Mark pwvPathFlag, which is an optional field, as non-existent.
	 */
	void clearPwvPathFlag ();
	


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
	 * spectralWindowId pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* getSpectralWindowUsingSpectralWindowId();
	 

	

	

	
		
	// ===> Slice link from a row of SysCal table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having feedId == this.feedId
	 * 
	 * @return a vector of FeedRow *
	 */
	vector <FeedRow *> getFeeds();
	
	

	

	

	
		
	/**
	 * antennaId pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* getAntennaUsingAntennaId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SysCalRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(Tag antennaId, int feedId, Tag spectralWindowId, ArrayTimeInterval timeInterval);
	
	

		 
	
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

	
	
 	

	
	// ===> Attribute numLoad, which is optional
	
	
	bool numLoadExists;
	

	int numLoad;

	
	
 	

	
	// ===> Attribute calLoad, which is optional
	
	
	bool calLoadExists;
	

	vector<int > calLoad;

	
	
 	

	
	// ===> Attribute feff, which is optional
	
	
	bool feffExists;
	

	vector<float > feff;

	
	
 	

	
	// ===> Attribute aeff, which is optional
	
	
	bool aeffExists;
	

	vector<float > aeff;

	
	
 	

	
	// ===> Attribute phaseDiff, which is optional
	
	
	bool phaseDiffExists;
	

	Angle phaseDiff;

	
	
 	

	
	// ===> Attribute sbgain, which is optional
	
	
	bool sbgainExists;
	

	float sbgain;

	
	
 	

	
	// ===> Attribute tau, which is optional
	
	
	bool tauExists;
	

	vector<Temperature > tau;

	
	
 	

	
	// ===> Attribute tcal, which is optional
	
	
	bool tcalExists;
	

	vector<Temperature > tcal;

	
	
 	

	
	// ===> Attribute trx, which is optional
	
	
	bool trxExists;
	

	vector<Temperature > trx;

	
	
 	

	
	// ===> Attribute tsky, which is optional
	
	
	bool tskyExists;
	

	vector<Temperature > tsky;

	
	
 	

	
	// ===> Attribute tsys, which is optional
	
	
	bool tsysExists;
	

	vector<Temperature > tsys;

	
	
 	

	
	// ===> Attribute tant, which is optional
	
	
	bool tantExists;
	

	vector<Temperature > tant;

	
	
 	

	
	// ===> Attribute tantTsys, which is optional
	
	
	bool tantTsysExists;
	

	vector<float > tantTsys;

	
	
 	

	
	// ===> Attribute pwvPath, which is optional
	
	
	bool pwvPathExists;
	

	vector<Length > pwvPath;

	
	
 	

	
	// ===> Attribute dpwvPath, which is optional
	
	
	bool dpwvPathExists;
	

	vector<float > dpwvPath;

	
	
 	

	
	// ===> Attribute feffSpectrum, which is optional
	
	
	bool feffSpectrumExists;
	

	vector<vector<float > > feffSpectrum;

	
	
 	

	
	// ===> Attribute sbgainSpectrum, which is optional
	
	
	bool sbgainSpectrumExists;
	

	vector<vector<float > > sbgainSpectrum;

	
	
 	

	
	// ===> Attribute tauSpectrum, which is optional
	
	
	bool tauSpectrumExists;
	

	vector<vector<Temperature > > tauSpectrum;

	
	
 	

	
	// ===> Attribute tcalSpectrum, which is optional
	
	
	bool tcalSpectrumExists;
	

	vector<vector<Temperature > > tcalSpectrum;

	
	
 	

	
	// ===> Attribute trxSpectrum, which is optional
	
	
	bool trxSpectrumExists;
	

	vector<Temperature > trxSpectrum;

	
	
 	

	
	// ===> Attribute tskySpectrum, which is optional
	
	
	bool tskySpectrumExists;
	

	vector<vector<Temperature > > tskySpectrum;

	
	
 	

	
	// ===> Attribute tsysSpectrum, which is optional
	
	
	bool tsysSpectrumExists;
	

	vector<vector<Temperature > > tsysSpectrum;

	
	
 	

	
	// ===> Attribute tantSpectrum, which is optional
	
	
	bool tantSpectrumExists;
	

	vector<vector<Temperature > > tantSpectrum;

	
	
 	

	
	// ===> Attribute tantTsysSpectrum, which is optional
	
	
	bool tantTsysSpectrumExists;
	

	vector<vector<float > > tantTsysSpectrum;

	
	
 	

	
	// ===> Attribute pwvPathSpectrum, which is optional
	
	
	bool pwvPathSpectrumExists;
	

	vector<Length > pwvPathSpectrum;

	
	
 	

	
	// ===> Attribute dpwvPathSpectrum, which is optional
	
	
	bool dpwvPathSpectrumExists;
	

	vector<float > dpwvPathSpectrum;

	
	
 	

	
	// ===> Attribute numPoly, which is optional
	
	
	bool numPolyExists;
	

	int numPoly;

	
	
 	

	
	// ===> Attribute numPolyFreq, which is optional
	
	
	bool numPolyFreqExists;
	

	int numPolyFreq;

	
	
 	

	
	// ===> Attribute timeOrigin, which is optional
	
	
	bool timeOriginExists;
	

	ArrayTime timeOrigin;

	
	
 	

	
	// ===> Attribute freqOrigin, which is optional
	
	
	bool freqOriginExists;
	

	Frequency freqOrigin;

	
	
 	

	
	// ===> Attribute phaseCurve, which is optional
	
	
	bool phaseCurveExists;
	

	vector<vector<Angle > > phaseCurve;

	
	
 	

	
	// ===> Attribute delayCurve, which is optional
	
	
	bool delayCurveExists;
	

	vector<vector<Interval > > delayCurve;

	
	
 	

	
	// ===> Attribute ampliCurve, which is optional
	
	
	bool ampliCurveExists;
	

	vector<vector<float > > ampliCurve;

	
	
 	

	
	// ===> Attribute bandpassCurve, which is optional
	
	
	bool bandpassCurveExists;
	

	vector<vector<float > > bandpassCurve;

	
	
 	

	
	// ===> Attribute phasediffFlag, which is optional
	
	
	bool phasediffFlagExists;
	

	bool phasediffFlag;

	
	
 	

	
	// ===> Attribute sbgainFlag, which is optional
	
	
	bool sbgainFlagExists;
	

	bool sbgainFlag;

	
	
 	

	
	// ===> Attribute tauFlag, which is optional
	
	
	bool tauFlagExists;
	

	bool tauFlag;

	
	
 	

	
	// ===> Attribute tcalFlag, which is optional
	
	
	bool tcalFlagExists;
	

	bool tcalFlag;

	
	
 	

	
	// ===> Attribute trxFlag, which is optional
	
	
	bool trxFlagExists;
	

	bool trxFlag;

	
	
 	

	
	// ===> Attribute tskyFlag, which is optional
	
	
	bool tskyFlagExists;
	

	bool tskyFlag;

	
	
 	

	
	// ===> Attribute tsysFlag, which is optional
	
	
	bool tsysFlagExists;
	

	bool tsysFlag;

	
	
 	

	
	// ===> Attribute tantFlag, which is optional
	
	
	bool tantFlagExists;
	

	bool tantFlag;

	
	
 	

	
	// ===> Attribute tantTsysFlag, which is optional
	
	
	bool tantTsysFlagExists;
	

	bool tantTsysFlag;

	
	
 	

	
	// ===> Attribute pwvPathFlag, which is optional
	
	
	bool pwvPathFlagExists;
	

	bool pwvPathFlag;

	
	
 	

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
	
	
		

	 

	

	
		


	

	
		

	 

	


};

} // End namespace asdm

#endif /* SysCal_CLASS */
