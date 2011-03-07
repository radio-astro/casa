
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
 * File SpectralWindowRow.h
 */
 
#ifndef SpectralWindowRow_CLASS
#define SpectralWindowRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::SpectralWindowRowIDL;
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




	

	

	

	

	

	

	

	

	

	

	
#include "CNetSideband.h"
using namespace NetSidebandMod;
	

	
#include "CSidebandProcessingMode.h"
using namespace SidebandProcessingModeMod;
	

	
#include "CBasebandName.h"
using namespace BasebandNameMod;
	

	

	

	

	

	
#include "CSpectralResolutionType.h"
using namespace SpectralResolutionTypeMod;
	

	

	
#include "CWindowFunction.h"
using namespace WindowFunctionMod;
	

	

	
#include "CCorrelationBit.h"
using namespace CorrelationBitMod;
	

	



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

/*\file SpectralWindow.h
    \brief Generated from model's revision "1.46", branch "HEAD"
*/

namespace asdm {

//class asdm::SpectralWindowTable;


// class asdm::DopplerRow;
class DopplerRow;

// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::SpectralWindowRow;
class SpectralWindowRow;
	

/**
 * The SpectralWindowRow class is a row of a SpectralWindowTable.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 */
class SpectralWindowRow {
friend class asdm::SpectralWindowTable;

public:

	virtual ~SpectralWindowRow();

	/**
	 * Return the table to which this row belongs.
	 */
	SpectralWindowTable &getTable() const;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SpectralWindowRowIDL struct.
	 */
	SpectralWindowRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SpectralWindowRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void setFromIDL (SpectralWindowRowIDL x) throw(ConversionException);
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
	
	
	// ===> Attribute spectralWindowId
	
	
	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag getSpectralWindowId() const;
	
 
 	
 	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute name, which is optional
	
	
	
	/**
	 * The attribute name is optional. Return true if this attribute exists.
	 * @return true if and only if the name attribute exists. 
	 */
	bool isNameExists() const;
	

	
 	/**
 	 * Get name, which is optional.
 	 * @return name as string
 	 * @throws IllegalAccessException If name does not exist.
 	 */
 	string getName() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 	 
 		
 	 */
 	void setName (string name);
		
	
	
	
	/**
	 * Mark name, which is an optional field, as non-existent.
	 */
	void clearName ();
	


	
	// ===> Attribute refFreq
	
	
	

	
 	/**
 	 * Get refFreq.
 	 * @return refFreq as Frequency
 	 */
 	Frequency getRefFreq() const;
	
 
 	
 	
 	/**
 	 * Set refFreq with the specified Frequency.
 	 * @param refFreq The Frequency value to which refFreq is to be set.
 	 
 		
 			
 	 */
 	void setRefFreq (Frequency refFreq);
  		
	
	
	


	
	// ===> Attribute chanFreq
	
	
	

	
 	/**
 	 * Get chanFreq.
 	 * @return chanFreq as vector<Frequency >
 	 */
 	vector<Frequency > getChanFreq() const;
	
 
 	
 	
 	/**
 	 * Set chanFreq with the specified vector<Frequency >.
 	 * @param chanFreq The vector<Frequency > value to which chanFreq is to be set.
 	 
 		
 			
 	 */
 	void setChanFreq (vector<Frequency > chanFreq);
  		
	
	
	


	
	// ===> Attribute chanWidth
	
	
	

	
 	/**
 	 * Get chanWidth.
 	 * @return chanWidth as vector<Frequency >
 	 */
 	vector<Frequency > getChanWidth() const;
	
 
 	
 	
 	/**
 	 * Set chanWidth with the specified vector<Frequency >.
 	 * @param chanWidth The vector<Frequency > value to which chanWidth is to be set.
 	 
 		
 			
 	 */
 	void setChanWidth (vector<Frequency > chanWidth);
  		
	
	
	


	
	// ===> Attribute measFreqRef, which is optional
	
	
	
	/**
	 * The attribute measFreqRef is optional. Return true if this attribute exists.
	 * @return true if and only if the measFreqRef attribute exists. 
	 */
	bool isMeasFreqRefExists() const;
	

	
 	/**
 	 * Get measFreqRef, which is optional.
 	 * @return measFreqRef as int
 	 * @throws IllegalAccessException If measFreqRef does not exist.
 	 */
 	int getMeasFreqRef() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set measFreqRef with the specified int.
 	 * @param measFreqRef The int value to which measFreqRef is to be set.
 	 
 		
 	 */
 	void setMeasFreqRef (int measFreqRef);
		
	
	
	
	/**
	 * Mark measFreqRef, which is an optional field, as non-existent.
	 */
	void clearMeasFreqRef ();
	


	
	// ===> Attribute effectiveBw
	
	
	

	
 	/**
 	 * Get effectiveBw.
 	 * @return effectiveBw as vector<Frequency >
 	 */
 	vector<Frequency > getEffectiveBw() const;
	
 
 	
 	
 	/**
 	 * Set effectiveBw with the specified vector<Frequency >.
 	 * @param effectiveBw The vector<Frequency > value to which effectiveBw is to be set.
 	 
 		
 			
 	 */
 	void setEffectiveBw (vector<Frequency > effectiveBw);
  		
	
	
	


	
	// ===> Attribute resolution
	
	
	

	
 	/**
 	 * Get resolution.
 	 * @return resolution as vector<Frequency >
 	 */
 	vector<Frequency > getResolution() const;
	
 
 	
 	
 	/**
 	 * Set resolution with the specified vector<Frequency >.
 	 * @param resolution The vector<Frequency > value to which resolution is to be set.
 	 
 		
 			
 	 */
 	void setResolution (vector<Frequency > resolution);
  		
	
	
	


	
	// ===> Attribute totBandwidth
	
	
	

	
 	/**
 	 * Get totBandwidth.
 	 * @return totBandwidth as Frequency
 	 */
 	Frequency getTotBandwidth() const;
	
 
 	
 	
 	/**
 	 * Set totBandwidth with the specified Frequency.
 	 * @param totBandwidth The Frequency value to which totBandwidth is to be set.
 	 
 		
 			
 	 */
 	void setTotBandwidth (Frequency totBandwidth);
  		
	
	
	


	
	// ===> Attribute netSideband
	
	
	

	
 	/**
 	 * Get netSideband.
 	 * @return netSideband as NetSidebandMod::NetSideband
 	 */
 	NetSidebandMod::NetSideband getNetSideband() const;
	
 
 	
 	
 	/**
 	 * Set netSideband with the specified NetSidebandMod::NetSideband.
 	 * @param netSideband The NetSidebandMod::NetSideband value to which netSideband is to be set.
 	 
 		
 			
 	 */
 	void setNetSideband (NetSidebandMod::NetSideband netSideband);
  		
	
	
	


	
	// ===> Attribute sidebandProcessingMode
	
	
	

	
 	/**
 	 * Get sidebandProcessingMode.
 	 * @return sidebandProcessingMode as SidebandProcessingModeMod::SidebandProcessingMode
 	 */
 	SidebandProcessingModeMod::SidebandProcessingMode getSidebandProcessingMode() const;
	
 
 	
 	
 	/**
 	 * Set sidebandProcessingMode with the specified SidebandProcessingModeMod::SidebandProcessingMode.
 	 * @param sidebandProcessingMode The SidebandProcessingModeMod::SidebandProcessingMode value to which sidebandProcessingMode is to be set.
 	 
 		
 			
 	 */
 	void setSidebandProcessingMode (SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode);
  		
	
	
	


	
	// ===> Attribute basebandName, which is optional
	
	
	
	/**
	 * The attribute basebandName is optional. Return true if this attribute exists.
	 * @return true if and only if the basebandName attribute exists. 
	 */
	bool isBasebandNameExists() const;
	

	
 	/**
 	 * Get basebandName, which is optional.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 * @throws IllegalAccessException If basebandName does not exist.
 	 */
 	BasebandNameMod::BasebandName getBasebandName() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 		
 	 */
 	void setBasebandName (BasebandNameMod::BasebandName basebandName);
		
	
	
	
	/**
	 * Mark basebandName, which is an optional field, as non-existent.
	 */
	void clearBasebandName ();
	


	
	// ===> Attribute bbcSideband, which is optional
	
	
	
	/**
	 * The attribute bbcSideband is optional. Return true if this attribute exists.
	 * @return true if and only if the bbcSideband attribute exists. 
	 */
	bool isBbcSidebandExists() const;
	

	
 	/**
 	 * Get bbcSideband, which is optional.
 	 * @return bbcSideband as int
 	 * @throws IllegalAccessException If bbcSideband does not exist.
 	 */
 	int getBbcSideband() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set bbcSideband with the specified int.
 	 * @param bbcSideband The int value to which bbcSideband is to be set.
 	 
 		
 	 */
 	void setBbcSideband (int bbcSideband);
		
	
	
	
	/**
	 * Mark bbcSideband, which is an optional field, as non-existent.
	 */
	void clearBbcSideband ();
	


	
	// ===> Attribute ifConvChain, which is optional
	
	
	
	/**
	 * The attribute ifConvChain is optional. Return true if this attribute exists.
	 * @return true if and only if the ifConvChain attribute exists. 
	 */
	bool isIfConvChainExists() const;
	

	
 	/**
 	 * Get ifConvChain, which is optional.
 	 * @return ifConvChain as int
 	 * @throws IllegalAccessException If ifConvChain does not exist.
 	 */
 	int getIfConvChain() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set ifConvChain with the specified int.
 	 * @param ifConvChain The int value to which ifConvChain is to be set.
 	 
 		
 	 */
 	void setIfConvChain (int ifConvChain);
		
	
	
	
	/**
	 * Mark ifConvChain, which is an optional field, as non-existent.
	 */
	void clearIfConvChain ();
	


	
	// ===> Attribute freqGroup, which is optional
	
	
	
	/**
	 * The attribute freqGroup is optional. Return true if this attribute exists.
	 * @return true if and only if the freqGroup attribute exists. 
	 */
	bool isFreqGroupExists() const;
	

	
 	/**
 	 * Get freqGroup, which is optional.
 	 * @return freqGroup as int
 	 * @throws IllegalAccessException If freqGroup does not exist.
 	 */
 	int getFreqGroup() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set freqGroup with the specified int.
 	 * @param freqGroup The int value to which freqGroup is to be set.
 	 
 		
 	 */
 	void setFreqGroup (int freqGroup);
		
	
	
	
	/**
	 * Mark freqGroup, which is an optional field, as non-existent.
	 */
	void clearFreqGroup ();
	


	
	// ===> Attribute freqGroupName, which is optional
	
	
	
	/**
	 * The attribute freqGroupName is optional. Return true if this attribute exists.
	 * @return true if and only if the freqGroupName attribute exists. 
	 */
	bool isFreqGroupNameExists() const;
	

	
 	/**
 	 * Get freqGroupName, which is optional.
 	 * @return freqGroupName as string
 	 * @throws IllegalAccessException If freqGroupName does not exist.
 	 */
 	string getFreqGroupName() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set freqGroupName with the specified string.
 	 * @param freqGroupName The string value to which freqGroupName is to be set.
 	 
 		
 	 */
 	void setFreqGroupName (string freqGroupName);
		
	
	
	
	/**
	 * Mark freqGroupName, which is an optional field, as non-existent.
	 */
	void clearFreqGroupName ();
	


	
	// ===> Attribute assocNature, which is optional
	
	
	
	/**
	 * The attribute assocNature is optional. Return true if this attribute exists.
	 * @return true if and only if the assocNature attribute exists. 
	 */
	bool isAssocNatureExists() const;
	

	
 	/**
 	 * Get assocNature, which is optional.
 	 * @return assocNature as vector<SpectralResolutionTypeMod::SpectralResolutionType >
 	 * @throws IllegalAccessException If assocNature does not exist.
 	 */
 	vector<SpectralResolutionTypeMod::SpectralResolutionType > getAssocNature() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set assocNature with the specified vector<SpectralResolutionTypeMod::SpectralResolutionType >.
 	 * @param assocNature The vector<SpectralResolutionTypeMod::SpectralResolutionType > value to which assocNature is to be set.
 	 
 		
 	 */
 	void setAssocNature (vector<SpectralResolutionTypeMod::SpectralResolutionType > assocNature);
		
	
	
	
	/**
	 * Mark assocNature, which is an optional field, as non-existent.
	 */
	void clearAssocNature ();
	


	
	// ===> Attribute quantization
	
	
	

	
 	/**
 	 * Get quantization.
 	 * @return quantization as bool
 	 */
 	bool getQuantization() const;
	
 
 	
 	
 	/**
 	 * Set quantization with the specified bool.
 	 * @param quantization The bool value to which quantization is to be set.
 	 
 		
 			
 	 */
 	void setQuantization (bool quantization);
  		
	
	
	


	
	// ===> Attribute windowFunction
	
	
	

	
 	/**
 	 * Get windowFunction.
 	 * @return windowFunction as WindowFunctionMod::WindowFunction
 	 */
 	WindowFunctionMod::WindowFunction getWindowFunction() const;
	
 
 	
 	
 	/**
 	 * Set windowFunction with the specified WindowFunctionMod::WindowFunction.
 	 * @param windowFunction The WindowFunctionMod::WindowFunction value to which windowFunction is to be set.
 	 
 		
 			
 	 */
 	void setWindowFunction (WindowFunctionMod::WindowFunction windowFunction);
  		
	
	
	


	
	// ===> Attribute oversampling
	
	
	

	
 	/**
 	 * Get oversampling.
 	 * @return oversampling as bool
 	 */
 	bool getOversampling() const;
	
 
 	
 	
 	/**
 	 * Set oversampling with the specified bool.
 	 * @param oversampling The bool value to which oversampling is to be set.
 	 
 		
 			
 	 */
 	void setOversampling (bool oversampling);
  		
	
	
	


	
	// ===> Attribute correlationBit
	
	
	

	
 	/**
 	 * Get correlationBit.
 	 * @return correlationBit as CorrelationBitMod::CorrelationBit
 	 */
 	CorrelationBitMod::CorrelationBit getCorrelationBit() const;
	
 
 	
 	
 	/**
 	 * Set correlationBit with the specified CorrelationBitMod::CorrelationBit.
 	 * @param correlationBit The CorrelationBitMod::CorrelationBit value to which correlationBit is to be set.
 	 
 		
 			
 	 */
 	void setCorrelationBit (CorrelationBitMod::CorrelationBit correlationBit);
  		
	
	
	


	
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
	
	
	// ===> Attribute assocSpectralWindowId, which is optional
	
	
	
	/**
	 * The attribute assocSpectralWindowId is optional. Return true if this attribute exists.
	 * @return true if and only if the assocSpectralWindowId attribute exists. 
	 */
	bool isAssocSpectralWindowIdExists() const;
	

	
 	/**
 	 * Get assocSpectralWindowId, which is optional.
 	 * @return assocSpectralWindowId as vector<Tag> 
 	 * @throws IllegalAccessException If assocSpectralWindowId does not exist.
 	 */
 	vector<Tag>  getAssocSpectralWindowId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set assocSpectralWindowId with the specified vector<Tag> .
 	 * @param assocSpectralWindowId The vector<Tag>  value to which assocSpectralWindowId is to be set.
 	 
 		
 	 */
 	void setAssocSpectralWindowId (vector<Tag>  assocSpectralWindowId);
		
	
	
	
	/**
	 * Mark assocSpectralWindowId, which is an optional field, as non-existent.
	 */
	void clearAssocSpectralWindowId ();
	


	
	// ===> Attribute dopplerId, which is optional
	
	
	
	/**
	 * The attribute dopplerId is optional. Return true if this attribute exists.
	 * @return true if and only if the dopplerId attribute exists. 
	 */
	bool isDopplerIdExists() const;
	

	
 	/**
 	 * Get dopplerId, which is optional.
 	 * @return dopplerId as int
 	 * @throws IllegalAccessException If dopplerId does not exist.
 	 */
 	int getDopplerId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set dopplerId with the specified int.
 	 * @param dopplerId The int value to which dopplerId is to be set.
 	 
 		
 	 */
 	void setDopplerId (int dopplerId);
		
	
	
	
	/**
	 * Mark dopplerId, which is an optional field, as non-existent.
	 */
	void clearDopplerId ();
	


	
	// ===> Attribute imageSpectralWindowId, which is optional
	
	
	
	/**
	 * The attribute imageSpectralWindowId is optional. Return true if this attribute exists.
	 * @return true if and only if the imageSpectralWindowId attribute exists. 
	 */
	bool isImageSpectralWindowIdExists() const;
	

	
 	/**
 	 * Get imageSpectralWindowId, which is optional.
 	 * @return imageSpectralWindowId as Tag
 	 * @throws IllegalAccessException If imageSpectralWindowId does not exist.
 	 */
 	Tag getImageSpectralWindowId() const throw(IllegalAccessException);
	
 
 	
 	
 	/**
 	 * Set imageSpectralWindowId with the specified Tag.
 	 * @param imageSpectralWindowId The Tag value to which imageSpectralWindowId is to be set.
 	 
 		
 	 */
 	void setImageSpectralWindowId (Tag imageSpectralWindowId);
		
	
	
	
	/**
	 * Mark imageSpectralWindowId, which is an optional field, as non-existent.
	 */
	void clearImageSpectralWindowId ();
	


	///////////
	// Links //
	///////////
	
	

	
		
	// ===> Slice link from a row of SpectralWindow table to a collection of row of Doppler table.
	
	/**
	 * Get the collection of row in the Doppler table having dopplerId == this.dopplerId
	 * 
	 * @return a vector of DopplerRow *
	 */
	vector <DopplerRow *> getDopplers();
	
	

	

	
 		
 	/**
 	 * Set assocSpectralWindowId[i] with the specified Tag.
 	 * @param i The index in assocSpectralWindowId where to set the Tag value.
 	 * @param assocSpectralWindowId The Tag value to which assocSpectralWindowId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void setAssocSpectralWindowId (int i, Tag assocSpectralWindowId)  ;
 			
	

	
		 
/**
 * Append a Tag to assocSpectralWindowId.
 * @param id the Tag to be appended to assocSpectralWindowId
 */
 void addAssocSpectralWindowId(Tag id); 

/**
 * Append a vector of Tag to assocSpectralWindowId.
 * @param id an array of Tag to be appended to assocSpectralWindowId
 */
 void addAssocSpectralWindowId(const vector<Tag> & id); 
 

 /**
  * Returns the Tag stored in assocSpectralWindowId at position i.
  * @param i the position in assocSpectralWindowId where the Tag is retrieved.
  * @return the Tag stored at position i in assocSpectralWindowId.
  */
 const Tag getAssocSpectralWindowId(int i);
 
 /**
  * Returns the SpectralWindowRow linked to this row via the tag stored in assocSpectralWindowId
  * at position i.
  * @param i the position in assocSpectralWindowId.
  * @return a pointer on a SpectralWindowRow whose key (a Tag) is equal to the Tag stored at position
  * i in the assocSpectralWindowId. 
  */
 SpectralWindowRow* getSpectralWindow(int i); 
 
 /**
  * Returns the vector of SpectralWindowRow* linked to this row via the Tags stored in assocSpectralWindowId
  * @return an array of pointers on SpectralWindowRow.
  */
 vector<SpectralWindowRow *> getSpectralWindows(); 
  

	

	

	
		
	/**
	 * imageSpectralWindowId pointer to the row in the SpectralWindow table having SpectralWindow.imageSpectralWindowId == imageSpectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 * throws IllegalAccessException
	 
	 */
	 SpectralWindowRow* getSpectralWindowUsingImageSpectralWindowId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SpectralWindowRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(int numChan, Frequency refFreq, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<Frequency > effectiveBw, vector<Frequency > resolution, Frequency totBandwidth, NetSidebandMod::NetSideband netSideband, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, bool quantization, WindowFunctionMod::WindowFunction windowFunction, bool oversampling, CorrelationBitMod::CorrelationBit correlationBit, bool flagRow);
	
	

	
	bool compareRequiredValue(int numChan, Frequency refFreq, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<Frequency > effectiveBw, vector<Frequency > resolution, Frequency totBandwidth, NetSidebandMod::NetSideband netSideband, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, bool quantization, WindowFunctionMod::WindowFunction windowFunction, bool oversampling, CorrelationBitMod::CorrelationBit correlationBit, bool flagRow); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SpectralWindowRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(SpectralWindowRow* x) ;

private:
	/**
	 * The table to which this row belongs.
	 */
	SpectralWindowTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a SpectralWindowRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SpectralWindowRow (SpectralWindowTable &table);

	/**
	 * Create a SpectralWindowRow using a copy constructor mechanism.
	 * <p>
	 * Given a SpectralWindowRow row and a SpectralWindowTable table, the method creates a new
	 * SpectralWindowRow owned by table. Each attribute of the created row is a copy (deep)
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
	 SpectralWindowRow (SpectralWindowTable &table, SpectralWindowRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute spectralWindowId
	
	

	Tag spectralWindowId;

	
	
 	
 	/**
 	 * Set spectralWindowId with the specified Tag value.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
		
 		
			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setSpectralWindowId (Tag spectralWindowId);
  		
	

	
	// ===> Attribute numChan
	
	

	int numChan;

	
	
 	

	
	// ===> Attribute name, which is optional
	
	
	bool nameExists;
	

	string name;

	
	
 	

	
	// ===> Attribute refFreq
	
	

	Frequency refFreq;

	
	
 	

	
	// ===> Attribute chanFreq
	
	

	vector<Frequency > chanFreq;

	
	
 	

	
	// ===> Attribute chanWidth
	
	

	vector<Frequency > chanWidth;

	
	
 	

	
	// ===> Attribute measFreqRef, which is optional
	
	
	bool measFreqRefExists;
	

	int measFreqRef;

	
	
 	

	
	// ===> Attribute effectiveBw
	
	

	vector<Frequency > effectiveBw;

	
	
 	

	
	// ===> Attribute resolution
	
	

	vector<Frequency > resolution;

	
	
 	

	
	// ===> Attribute totBandwidth
	
	

	Frequency totBandwidth;

	
	
 	

	
	// ===> Attribute netSideband
	
	

	NetSidebandMod::NetSideband netSideband;

	
	
 	

	
	// ===> Attribute sidebandProcessingMode
	
	

	SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode;

	
	
 	

	
	// ===> Attribute basebandName, which is optional
	
	
	bool basebandNameExists;
	

	BasebandNameMod::BasebandName basebandName;

	
	
 	

	
	// ===> Attribute bbcSideband, which is optional
	
	
	bool bbcSidebandExists;
	

	int bbcSideband;

	
	
 	

	
	// ===> Attribute ifConvChain, which is optional
	
	
	bool ifConvChainExists;
	

	int ifConvChain;

	
	
 	

	
	// ===> Attribute freqGroup, which is optional
	
	
	bool freqGroupExists;
	

	int freqGroup;

	
	
 	

	
	// ===> Attribute freqGroupName, which is optional
	
	
	bool freqGroupNameExists;
	

	string freqGroupName;

	
	
 	

	
	// ===> Attribute assocNature, which is optional
	
	
	bool assocNatureExists;
	

	vector<SpectralResolutionTypeMod::SpectralResolutionType > assocNature;

	
	
 	

	
	// ===> Attribute quantization
	
	

	bool quantization;

	
	
 	

	
	// ===> Attribute windowFunction
	
	

	WindowFunctionMod::WindowFunction windowFunction;

	
	
 	

	
	// ===> Attribute oversampling
	
	

	bool oversampling;

	
	
 	

	
	// ===> Attribute correlationBit
	
	

	CorrelationBitMod::CorrelationBit correlationBit;

	
	
 	

	
	// ===> Attribute flagRow
	
	

	bool flagRow;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute assocSpectralWindowId, which is optional
	
	
	bool assocSpectralWindowIdExists;
	

	vector<Tag>  assocSpectralWindowId;

	
	
 	

	
	// ===> Attribute dopplerId, which is optional
	
	
	bool dopplerIdExists;
	

	int dopplerId;

	
	
 	

	
	// ===> Attribute imageSpectralWindowId, which is optional
	
	
	bool imageSpectralWindowIdExists;
	

	Tag imageSpectralWindowId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		


	

	
		


	

	
		

	 

	


};

} // End namespace asdm

#endif /* SpectralWindow_CLASS */
