
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




	

	
#include "CBasebandName.h"
using namespace BasebandNameMod;
	

	
#include "CNetSideband.h"
using namespace NetSidebandMod;
	

	

	

	
#include "CSidebandProcessingMode.h"
using namespace SidebandProcessingModeMod;
	

	

	
#include "CWindowFunction.h"
using namespace WindowFunctionMod;
	

	

	

	

	

	

	
#include "CCorrelationBit.h"
using namespace CorrelationBitMod;
	

	

	

	

	

	

	
#include "CFrequencyReferenceCode.h"
using namespace FrequencyReferenceCodeMod;
	

	

	

	

	

	

	

	

	
#include "CSpectralResolutionType.h"
using namespace SpectralResolutionTypeMod;
	



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
    \brief Generated from model's revision "1.52", branch "HEAD"
*/

namespace asdm {

//class asdm::SpectralWindowTable;


// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::SpectralWindowRow;
class SpectralWindowRow;

// class asdm::DopplerRow;
class DopplerRow;
	

/**
 * The SpectralWindowRow class is a row of a SpectralWindowTable.
 * 
 * Generated from model's revision "1.52", branch "HEAD"
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
	 * @throws ConversionException
	 */
	void setFromIDL (SpectralWindowRowIDL x) ;
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
	  * @table the SpectralWindowTable to which the row built by deserialization will be parented.
	  */
	 static SpectralWindowRow* fromBin(EndianISStream& eiss, SpectralWindowTable& table);	 
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute spectralWindowId
	
	
	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag getSpectralWindowId() const;
	
 
 	
 	
	
	


	
	// ===> Attribute basebandName
	
	
	

	
 	/**
 	 * Get basebandName.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 */
 	BasebandNameMod::BasebandName getBasebandName() const;
	
 
 	
 	
 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 		
 			
 	 */
 	void setBasebandName (BasebandNameMod::BasebandName basebandName);
  		
	
	
	


	
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
  		
	
	
	


	
	// ===> Attribute chanFreqStart, which is optional
	
	
	
	/**
	 * The attribute chanFreqStart is optional. Return true if this attribute exists.
	 * @return true if and only if the chanFreqStart attribute exists. 
	 */
	bool isChanFreqStartExists() const;
	

	
 	/**
 	 * Get chanFreqStart, which is optional.
 	 * @return chanFreqStart as Frequency
 	 * @throws IllegalAccessException If chanFreqStart does not exist.
 	 */
 	Frequency getChanFreqStart() const;
	
 
 	
 	
 	/**
 	 * Set chanFreqStart with the specified Frequency.
 	 * @param chanFreqStart The Frequency value to which chanFreqStart is to be set.
 	 
 		
 	 */
 	void setChanFreqStart (Frequency chanFreqStart);
		
	
	
	
	/**
	 * Mark chanFreqStart, which is an optional field, as non-existent.
	 */
	void clearChanFreqStart ();
	


	
	// ===> Attribute chanFreqStep, which is optional
	
	
	
	/**
	 * The attribute chanFreqStep is optional. Return true if this attribute exists.
	 * @return true if and only if the chanFreqStep attribute exists. 
	 */
	bool isChanFreqStepExists() const;
	

	
 	/**
 	 * Get chanFreqStep, which is optional.
 	 * @return chanFreqStep as Frequency
 	 * @throws IllegalAccessException If chanFreqStep does not exist.
 	 */
 	Frequency getChanFreqStep() const;
	
 
 	
 	
 	/**
 	 * Set chanFreqStep with the specified Frequency.
 	 * @param chanFreqStep The Frequency value to which chanFreqStep is to be set.
 	 
 		
 	 */
 	void setChanFreqStep (Frequency chanFreqStep);
		
	
	
	
	/**
	 * Mark chanFreqStep, which is an optional field, as non-existent.
	 */
	void clearChanFreqStep ();
	


	
	// ===> Attribute chanFreqArray, which is optional
	
	
	
	/**
	 * The attribute chanFreqArray is optional. Return true if this attribute exists.
	 * @return true if and only if the chanFreqArray attribute exists. 
	 */
	bool isChanFreqArrayExists() const;
	

	
 	/**
 	 * Get chanFreqArray, which is optional.
 	 * @return chanFreqArray as vector<Frequency >
 	 * @throws IllegalAccessException If chanFreqArray does not exist.
 	 */
 	vector<Frequency > getChanFreqArray() const;
	
 
 	
 	
 	/**
 	 * Set chanFreqArray with the specified vector<Frequency >.
 	 * @param chanFreqArray The vector<Frequency > value to which chanFreqArray is to be set.
 	 
 		
 	 */
 	void setChanFreqArray (vector<Frequency > chanFreqArray);
		
	
	
	
	/**
	 * Mark chanFreqArray, which is an optional field, as non-existent.
	 */
	void clearChanFreqArray ();
	


	
	// ===> Attribute chanWidth, which is optional
	
	
	
	/**
	 * The attribute chanWidth is optional. Return true if this attribute exists.
	 * @return true if and only if the chanWidth attribute exists. 
	 */
	bool isChanWidthExists() const;
	

	
 	/**
 	 * Get chanWidth, which is optional.
 	 * @return chanWidth as Frequency
 	 * @throws IllegalAccessException If chanWidth does not exist.
 	 */
 	Frequency getChanWidth() const;
	
 
 	
 	
 	/**
 	 * Set chanWidth with the specified Frequency.
 	 * @param chanWidth The Frequency value to which chanWidth is to be set.
 	 
 		
 	 */
 	void setChanWidth (Frequency chanWidth);
		
	
	
	
	/**
	 * Mark chanWidth, which is an optional field, as non-existent.
	 */
	void clearChanWidth ();
	


	
	// ===> Attribute chanWidthArray, which is optional
	
	
	
	/**
	 * The attribute chanWidthArray is optional. Return true if this attribute exists.
	 * @return true if and only if the chanWidthArray attribute exists. 
	 */
	bool isChanWidthArrayExists() const;
	

	
 	/**
 	 * Get chanWidthArray, which is optional.
 	 * @return chanWidthArray as vector<Frequency >
 	 * @throws IllegalAccessException If chanWidthArray does not exist.
 	 */
 	vector<Frequency > getChanWidthArray() const;
	
 
 	
 	
 	/**
 	 * Set chanWidthArray with the specified vector<Frequency >.
 	 * @param chanWidthArray The vector<Frequency > value to which chanWidthArray is to be set.
 	 
 		
 	 */
 	void setChanWidthArray (vector<Frequency > chanWidthArray);
		
	
	
	
	/**
	 * Mark chanWidthArray, which is an optional field, as non-existent.
	 */
	void clearChanWidthArray ();
	


	
	// ===> Attribute correlationBit, which is optional
	
	
	
	/**
	 * The attribute correlationBit is optional. Return true if this attribute exists.
	 * @return true if and only if the correlationBit attribute exists. 
	 */
	bool isCorrelationBitExists() const;
	

	
 	/**
 	 * Get correlationBit, which is optional.
 	 * @return correlationBit as CorrelationBitMod::CorrelationBit
 	 * @throws IllegalAccessException If correlationBit does not exist.
 	 */
 	CorrelationBitMod::CorrelationBit getCorrelationBit() const;
	
 
 	
 	
 	/**
 	 * Set correlationBit with the specified CorrelationBitMod::CorrelationBit.
 	 * @param correlationBit The CorrelationBitMod::CorrelationBit value to which correlationBit is to be set.
 	 
 		
 	 */
 	void setCorrelationBit (CorrelationBitMod::CorrelationBit correlationBit);
		
	
	
	
	/**
	 * Mark correlationBit, which is an optional field, as non-existent.
	 */
	void clearCorrelationBit ();
	


	
	// ===> Attribute effectiveBw, which is optional
	
	
	
	/**
	 * The attribute effectiveBw is optional. Return true if this attribute exists.
	 * @return true if and only if the effectiveBw attribute exists. 
	 */
	bool isEffectiveBwExists() const;
	

	
 	/**
 	 * Get effectiveBw, which is optional.
 	 * @return effectiveBw as Frequency
 	 * @throws IllegalAccessException If effectiveBw does not exist.
 	 */
 	Frequency getEffectiveBw() const;
	
 
 	
 	
 	/**
 	 * Set effectiveBw with the specified Frequency.
 	 * @param effectiveBw The Frequency value to which effectiveBw is to be set.
 	 
 		
 	 */
 	void setEffectiveBw (Frequency effectiveBw);
		
	
	
	
	/**
	 * Mark effectiveBw, which is an optional field, as non-existent.
	 */
	void clearEffectiveBw ();
	


	
	// ===> Attribute effectiveBwArray, which is optional
	
	
	
	/**
	 * The attribute effectiveBwArray is optional. Return true if this attribute exists.
	 * @return true if and only if the effectiveBwArray attribute exists. 
	 */
	bool isEffectiveBwArrayExists() const;
	

	
 	/**
 	 * Get effectiveBwArray, which is optional.
 	 * @return effectiveBwArray as vector<Frequency >
 	 * @throws IllegalAccessException If effectiveBwArray does not exist.
 	 */
 	vector<Frequency > getEffectiveBwArray() const;
	
 
 	
 	
 	/**
 	 * Set effectiveBwArray with the specified vector<Frequency >.
 	 * @param effectiveBwArray The vector<Frequency > value to which effectiveBwArray is to be set.
 	 
 		
 	 */
 	void setEffectiveBwArray (vector<Frequency > effectiveBwArray);
		
	
	
	
	/**
	 * Mark effectiveBwArray, which is an optional field, as non-existent.
	 */
	void clearEffectiveBwArray ();
	


	
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
 	int getFreqGroup() const;
	
 
 	
 	
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
 	string getFreqGroupName() const;
	
 
 	
 	
 	/**
 	 * Set freqGroupName with the specified string.
 	 * @param freqGroupName The string value to which freqGroupName is to be set.
 	 
 		
 	 */
 	void setFreqGroupName (string freqGroupName);
		
	
	
	
	/**
	 * Mark freqGroupName, which is an optional field, as non-existent.
	 */
	void clearFreqGroupName ();
	


	
	// ===> Attribute lineArray, which is optional
	
	
	
	/**
	 * The attribute lineArray is optional. Return true if this attribute exists.
	 * @return true if and only if the lineArray attribute exists. 
	 */
	bool isLineArrayExists() const;
	

	
 	/**
 	 * Get lineArray, which is optional.
 	 * @return lineArray as vector<bool >
 	 * @throws IllegalAccessException If lineArray does not exist.
 	 */
 	vector<bool > getLineArray() const;
	
 
 	
 	
 	/**
 	 * Set lineArray with the specified vector<bool >.
 	 * @param lineArray The vector<bool > value to which lineArray is to be set.
 	 
 		
 	 */
 	void setLineArray (vector<bool > lineArray);
		
	
	
	
	/**
	 * Mark lineArray, which is an optional field, as non-existent.
	 */
	void clearLineArray ();
	


	
	// ===> Attribute measFreqRef, which is optional
	
	
	
	/**
	 * The attribute measFreqRef is optional. Return true if this attribute exists.
	 * @return true if and only if the measFreqRef attribute exists. 
	 */
	bool isMeasFreqRefExists() const;
	

	
 	/**
 	 * Get measFreqRef, which is optional.
 	 * @return measFreqRef as FrequencyReferenceCodeMod::FrequencyReferenceCode
 	 * @throws IllegalAccessException If measFreqRef does not exist.
 	 */
 	FrequencyReferenceCodeMod::FrequencyReferenceCode getMeasFreqRef() const;
	
 
 	
 	
 	/**
 	 * Set measFreqRef with the specified FrequencyReferenceCodeMod::FrequencyReferenceCode.
 	 * @param measFreqRef The FrequencyReferenceCodeMod::FrequencyReferenceCode value to which measFreqRef is to be set.
 	 
 		
 	 */
 	void setMeasFreqRef (FrequencyReferenceCodeMod::FrequencyReferenceCode measFreqRef);
		
	
	
	
	/**
	 * Mark measFreqRef, which is an optional field, as non-existent.
	 */
	void clearMeasFreqRef ();
	


	
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
 	string getName() const;
	
 
 	
 	
 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 	 
 		
 	 */
 	void setName (string name);
		
	
	
	
	/**
	 * Mark name, which is an optional field, as non-existent.
	 */
	void clearName ();
	


	
	// ===> Attribute oversampling, which is optional
	
	
	
	/**
	 * The attribute oversampling is optional. Return true if this attribute exists.
	 * @return true if and only if the oversampling attribute exists. 
	 */
	bool isOversamplingExists() const;
	

	
 	/**
 	 * Get oversampling, which is optional.
 	 * @return oversampling as bool
 	 * @throws IllegalAccessException If oversampling does not exist.
 	 */
 	bool getOversampling() const;
	
 
 	
 	
 	/**
 	 * Set oversampling with the specified bool.
 	 * @param oversampling The bool value to which oversampling is to be set.
 	 
 		
 	 */
 	void setOversampling (bool oversampling);
		
	
	
	
	/**
	 * Mark oversampling, which is an optional field, as non-existent.
	 */
	void clearOversampling ();
	


	
	// ===> Attribute quantization, which is optional
	
	
	
	/**
	 * The attribute quantization is optional. Return true if this attribute exists.
	 * @return true if and only if the quantization attribute exists. 
	 */
	bool isQuantizationExists() const;
	

	
 	/**
 	 * Get quantization, which is optional.
 	 * @return quantization as bool
 	 * @throws IllegalAccessException If quantization does not exist.
 	 */
 	bool getQuantization() const;
	
 
 	
 	
 	/**
 	 * Set quantization with the specified bool.
 	 * @param quantization The bool value to which quantization is to be set.
 	 
 		
 	 */
 	void setQuantization (bool quantization);
		
	
	
	
	/**
	 * Mark quantization, which is an optional field, as non-existent.
	 */
	void clearQuantization ();
	


	
	// ===> Attribute refChan, which is optional
	
	
	
	/**
	 * The attribute refChan is optional. Return true if this attribute exists.
	 * @return true if and only if the refChan attribute exists. 
	 */
	bool isRefChanExists() const;
	

	
 	/**
 	 * Get refChan, which is optional.
 	 * @return refChan as double
 	 * @throws IllegalAccessException If refChan does not exist.
 	 */
 	double getRefChan() const;
	
 
 	
 	
 	/**
 	 * Set refChan with the specified double.
 	 * @param refChan The double value to which refChan is to be set.
 	 
 		
 	 */
 	void setRefChan (double refChan);
		
	
	
	
	/**
	 * Mark refChan, which is an optional field, as non-existent.
	 */
	void clearRefChan ();
	


	
	// ===> Attribute resolution, which is optional
	
	
	
	/**
	 * The attribute resolution is optional. Return true if this attribute exists.
	 * @return true if and only if the resolution attribute exists. 
	 */
	bool isResolutionExists() const;
	

	
 	/**
 	 * Get resolution, which is optional.
 	 * @return resolution as Frequency
 	 * @throws IllegalAccessException If resolution does not exist.
 	 */
 	Frequency getResolution() const;
	
 
 	
 	
 	/**
 	 * Set resolution with the specified Frequency.
 	 * @param resolution The Frequency value to which resolution is to be set.
 	 
 		
 	 */
 	void setResolution (Frequency resolution);
		
	
	
	
	/**
	 * Mark resolution, which is an optional field, as non-existent.
	 */
	void clearResolution ();
	


	
	// ===> Attribute resolutionArray, which is optional
	
	
	
	/**
	 * The attribute resolutionArray is optional. Return true if this attribute exists.
	 * @return true if and only if the resolutionArray attribute exists. 
	 */
	bool isResolutionArrayExists() const;
	

	
 	/**
 	 * Get resolutionArray, which is optional.
 	 * @return resolutionArray as vector<Frequency >
 	 * @throws IllegalAccessException If resolutionArray does not exist.
 	 */
 	vector<Frequency > getResolutionArray() const;
	
 
 	
 	
 	/**
 	 * Set resolutionArray with the specified vector<Frequency >.
 	 * @param resolutionArray The vector<Frequency > value to which resolutionArray is to be set.
 	 
 		
 	 */
 	void setResolutionArray (vector<Frequency > resolutionArray);
		
	
	
	
	/**
	 * Mark resolutionArray, which is an optional field, as non-existent.
	 */
	void clearResolutionArray ();
	


	
	// ===> Attribute numAssocValues, which is optional
	
	
	
	/**
	 * The attribute numAssocValues is optional. Return true if this attribute exists.
	 * @return true if and only if the numAssocValues attribute exists. 
	 */
	bool isNumAssocValuesExists() const;
	

	
 	/**
 	 * Get numAssocValues, which is optional.
 	 * @return numAssocValues as int
 	 * @throws IllegalAccessException If numAssocValues does not exist.
 	 */
 	int getNumAssocValues() const;
	
 
 	
 	
 	/**
 	 * Set numAssocValues with the specified int.
 	 * @param numAssocValues The int value to which numAssocValues is to be set.
 	 
 		
 	 */
 	void setNumAssocValues (int numAssocValues);
		
	
	
	
	/**
	 * Mark numAssocValues, which is an optional field, as non-existent.
	 */
	void clearNumAssocValues ();
	


	
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
 	vector<SpectralResolutionTypeMod::SpectralResolutionType > getAssocNature() const;
	
 
 	
 	
 	/**
 	 * Set assocNature with the specified vector<SpectralResolutionTypeMod::SpectralResolutionType >.
 	 * @param assocNature The vector<SpectralResolutionTypeMod::SpectralResolutionType > value to which assocNature is to be set.
 	 
 		
 	 */
 	void setAssocNature (vector<SpectralResolutionTypeMod::SpectralResolutionType > assocNature);
		
	
	
	
	/**
	 * Mark assocNature, which is an optional field, as non-existent.
	 */
	void clearAssocNature ();
	


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
 	vector<Tag>  getAssocSpectralWindowId() const;
	
 
 	
 	
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
 	int getDopplerId() const;
	
 
 	
 	
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
 	Tag getImageSpectralWindowId() const;
	
 
 	
 	
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
	 

	

	

	
		
	// ===> Slice link from a row of SpectralWindow table to a collection of row of Doppler table.
	
	/**
	 * Get the collection of row in the Doppler table having dopplerId == this.dopplerId
	 * 
	 * @return a vector of DopplerRow *
	 */
	vector <DopplerRow *> getDopplers();
	
	

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this SpectralWindowRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 */ 
	bool compareNoAutoInc(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband netSideband, int numChan, Frequency refFreq, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, Frequency totBandwidth, WindowFunctionMod::WindowFunction windowFunction);
	
	

	
	bool compareRequiredValue(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband netSideband, int numChan, Frequency refFreq, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, Frequency totBandwidth, WindowFunctionMod::WindowFunction windowFunction); 
		 
	
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
  		
	

	
	// ===> Attribute basebandName
	
	

	BasebandNameMod::BasebandName basebandName;

	
	
 	

	
	// ===> Attribute netSideband
	
	

	NetSidebandMod::NetSideband netSideband;

	
	
 	

	
	// ===> Attribute numChan
	
	

	int numChan;

	
	
 	

	
	// ===> Attribute refFreq
	
	

	Frequency refFreq;

	
	
 	

	
	// ===> Attribute sidebandProcessingMode
	
	

	SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode;

	
	
 	

	
	// ===> Attribute totBandwidth
	
	

	Frequency totBandwidth;

	
	
 	

	
	// ===> Attribute windowFunction
	
	

	WindowFunctionMod::WindowFunction windowFunction;

	
	
 	

	
	// ===> Attribute chanFreqStart, which is optional
	
	
	bool chanFreqStartExists;
	

	Frequency chanFreqStart;

	
	
 	

	
	// ===> Attribute chanFreqStep, which is optional
	
	
	bool chanFreqStepExists;
	

	Frequency chanFreqStep;

	
	
 	

	
	// ===> Attribute chanFreqArray, which is optional
	
	
	bool chanFreqArrayExists;
	

	vector<Frequency > chanFreqArray;

	
	
 	

	
	// ===> Attribute chanWidth, which is optional
	
	
	bool chanWidthExists;
	

	Frequency chanWidth;

	
	
 	

	
	// ===> Attribute chanWidthArray, which is optional
	
	
	bool chanWidthArrayExists;
	

	vector<Frequency > chanWidthArray;

	
	
 	

	
	// ===> Attribute correlationBit, which is optional
	
	
	bool correlationBitExists;
	

	CorrelationBitMod::CorrelationBit correlationBit;

	
	
 	

	
	// ===> Attribute effectiveBw, which is optional
	
	
	bool effectiveBwExists;
	

	Frequency effectiveBw;

	
	
 	

	
	// ===> Attribute effectiveBwArray, which is optional
	
	
	bool effectiveBwArrayExists;
	

	vector<Frequency > effectiveBwArray;

	
	
 	

	
	// ===> Attribute freqGroup, which is optional
	
	
	bool freqGroupExists;
	

	int freqGroup;

	
	
 	

	
	// ===> Attribute freqGroupName, which is optional
	
	
	bool freqGroupNameExists;
	

	string freqGroupName;

	
	
 	

	
	// ===> Attribute lineArray, which is optional
	
	
	bool lineArrayExists;
	

	vector<bool > lineArray;

	
	
 	

	
	// ===> Attribute measFreqRef, which is optional
	
	
	bool measFreqRefExists;
	

	FrequencyReferenceCodeMod::FrequencyReferenceCode measFreqRef;

	
	
 	

	
	// ===> Attribute name, which is optional
	
	
	bool nameExists;
	

	string name;

	
	
 	

	
	// ===> Attribute oversampling, which is optional
	
	
	bool oversamplingExists;
	

	bool oversampling;

	
	
 	

	
	// ===> Attribute quantization, which is optional
	
	
	bool quantizationExists;
	

	bool quantization;

	
	
 	

	
	// ===> Attribute refChan, which is optional
	
	
	bool refChanExists;
	

	double refChan;

	
	
 	

	
	// ===> Attribute resolution, which is optional
	
	
	bool resolutionExists;
	

	Frequency resolution;

	
	
 	

	
	// ===> Attribute resolutionArray, which is optional
	
	
	bool resolutionArrayExists;
	

	vector<Frequency > resolutionArray;

	
	
 	

	
	// ===> Attribute numAssocValues, which is optional
	
	
	bool numAssocValuesExists;
	

	int numAssocValues;

	
	
 	

	
	// ===> Attribute assocNature, which is optional
	
	
	bool assocNatureExists;
	

	vector<SpectralResolutionTypeMod::SpectralResolutionType > assocNature;

	
	
 	

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
