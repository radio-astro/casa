
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
 * File GainTrackingRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <GainTrackingRow.h>
#include <GainTrackingTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <FeedTable.h>
#include <FeedRow.h>
	

using asdm::ASDM;
using asdm::GainTrackingRow;
using asdm::GainTrackingTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::FeedTable;
using asdm::FeedRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	GainTrackingRow::~GainTrackingRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	GainTrackingTable &GainTrackingRow::getTable() const {
		return table;
	}
	
	void GainTrackingRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a GainTrackingRowIDL struct.
	 */
	GainTrackingRowIDL *GainTrackingRow::toIDL() const {
		GainTrackingRowIDL *x = new GainTrackingRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->attenuator = attenuator;
 				
 			
		
	

	
  		
		
		
			
				
		x->numLO = numLO;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
		x->cableDelay.length(cableDelay.size());
		for (unsigned int i = 0; i < cableDelay.size(); ++i) {
			
				
			x->cableDelay[i] = cableDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->crossPolarizationDelay = crossPolarizationDelay;
 				
 			
		
	

	
  		
		
		
			
		x->loPropagationDelay.length(loPropagationDelay.size());
		for (unsigned int i = 0; i < loPropagationDelay.size(); ++i) {
			
				
			x->loPropagationDelay[i] = loPropagationDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->receiverDelay.length(receiverDelay.size());
		for (unsigned int i = 0; i < receiverDelay.size(); ++i) {
			
				
			x->receiverDelay[i] = receiverDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->delayOffsetExists = delayOffsetExists;
		
		
			
				
		x->delayOffset = delayOffset;
 				
 			
		
	

	
  		
		
		x->freqOffsetExists = freqOffsetExists;
		
		
			
		x->freqOffset.length(freqOffset.size());
		for (unsigned int i = 0; i < freqOffset.size(); ++i) {
			
			x->freqOffset[i] = freqOffset.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		x->phaseOffsetExists = phaseOffsetExists;
		
		
			
		x->phaseOffset.length(phaseOffset.size());
		for (unsigned int i = 0; i < phaseOffset.size(); ++i) {
			
			x->phaseOffset[i] = phaseOffset.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->samplingLevelExists = samplingLevelExists;
		
		
			
				
		x->samplingLevel = samplingLevel;
 				
 			
		
	

	
  		
		
		x->numAttFreqExists = numAttFreqExists;
		
		
			
				
		x->numAttFreq = numAttFreq;
 				
 			
		
	

	
  		
		
		x->attFreqExists = attFreqExists;
		
		
			
		x->attFreq.length(attFreq.size());
		for (unsigned int i = 0; i < attFreq.size(); ++i) {
			
				
			x->attFreq[i] = attFreq.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->attSpectrumExists = attSpectrumExists;
		
		
			
		x->attSpectrum.length(attSpectrum.size());
		for (unsigned int i = 0; i < attSpectrum.size(); ++i) {
			
			x->attSpectrum[i] = attSpectrum.at(i).toIDLComplex();
			
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
				
		x->feedId = feedId;
 				
 			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct GainTrackingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void GainTrackingRow::setFromIDL (GainTrackingRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setAttenuator(x.attenuator);
  			
 		
		
	

	
		
		
			
		setNumLO(x.numLO);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		cableDelay .clear();
		for (unsigned int i = 0; i <x.cableDelay.length(); ++i) {
			
			cableDelay.push_back(x.cableDelay[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setCrossPolarizationDelay(x.crossPolarizationDelay);
  			
 		
		
	

	
		
		
			
		loPropagationDelay .clear();
		for (unsigned int i = 0; i <x.loPropagationDelay.length(); ++i) {
			
			loPropagationDelay.push_back(x.loPropagationDelay[i]);
  			
		}
			
  		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		receiverDelay .clear();
		for (unsigned int i = 0; i <x.receiverDelay.length(); ++i) {
			
			receiverDelay.push_back(x.receiverDelay[i]);
  			
		}
			
  		
		
	

	
		
		delayOffsetExists = x.delayOffsetExists;
		if (x.delayOffsetExists) {
		
		
			
		setDelayOffset(x.delayOffset);
  			
 		
		
		}
		
	

	
		
		freqOffsetExists = x.freqOffsetExists;
		if (x.freqOffsetExists) {
		
		
			
		freqOffset .clear();
		for (unsigned int i = 0; i <x.freqOffset.length(); ++i) {
			
			freqOffset.push_back(Frequency (x.freqOffset[i]));
			
		}
			
  		
		
		}
		
	

	
		
		phaseOffsetExists = x.phaseOffsetExists;
		if (x.phaseOffsetExists) {
		
		
			
		phaseOffset .clear();
		for (unsigned int i = 0; i <x.phaseOffset.length(); ++i) {
			
			phaseOffset.push_back(Angle (x.phaseOffset[i]));
			
		}
			
  		
		
		}
		
	

	
		
		samplingLevelExists = x.samplingLevelExists;
		if (x.samplingLevelExists) {
		
		
			
		setSamplingLevel(x.samplingLevel);
  			
 		
		
		}
		
	

	
		
		numAttFreqExists = x.numAttFreqExists;
		if (x.numAttFreqExists) {
		
		
			
		setNumAttFreq(x.numAttFreq);
  			
 		
		
		}
		
	

	
		
		attFreqExists = x.attFreqExists;
		if (x.attFreqExists) {
		
		
			
		attFreq .clear();
		for (unsigned int i = 0; i <x.attFreq.length(); ++i) {
			
			attFreq.push_back(x.attFreq[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		attSpectrumExists = x.attSpectrumExists;
		if (x.attSpectrumExists) {
		
		
			
		attSpectrum .clear();
		for (unsigned int i = 0; i <x.attSpectrum.length(); ++i) {
			
			attSpectrum.push_back(Complex (x.attSpectrum[i]));
			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"GainTracking");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string GainTrackingRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(attenuator, "attenuator", buf);
		
		
	

  	
 		
		
		Parser::toXML(numLO, "numLO", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(cableDelay, "cableDelay", buf);
		
		
	

  	
 		
		
		Parser::toXML(crossPolarizationDelay, "crossPolarizationDelay", buf);
		
		
	

  	
 		
		
		Parser::toXML(loPropagationDelay, "loPropagationDelay", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(receiverDelay, "receiverDelay", buf);
		
		
	

  	
 		
		if (delayOffsetExists) {
		
		
		Parser::toXML(delayOffset, "delayOffset", buf);
		
		
		}
		
	

  	
 		
		if (freqOffsetExists) {
		
		
		Parser::toXML(freqOffset, "freqOffset", buf);
		
		
		}
		
	

  	
 		
		if (phaseOffsetExists) {
		
		
		Parser::toXML(phaseOffset, "phaseOffset", buf);
		
		
		}
		
	

  	
 		
		if (samplingLevelExists) {
		
		
		Parser::toXML(samplingLevel, "samplingLevel", buf);
		
		
		}
		
	

  	
 		
		if (numAttFreqExists) {
		
		
		Parser::toXML(numAttFreq, "numAttFreq", buf);
		
		
		}
		
	

  	
 		
		if (attFreqExists) {
		
		
		Parser::toXML(attFreq, "attFreq", buf);
		
		
		}
		
	

  	
 		
		if (attSpectrumExists) {
		
		
		Parser::toXML(attSpectrum, "attSpectrum", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(feedId, "feedId", buf);
		
		
	

  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void GainTrackingRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setAttenuator(Parser::getFloat("attenuator","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setNumLO(Parser::getInteger("numLO","GainTracking",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","GainTracking",rowDoc));
			
		
	

	
  		
			
					
	  	setCableDelay(Parser::get1DDouble("cableDelay","GainTracking",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setCrossPolarizationDelay(Parser::getDouble("crossPolarizationDelay","GainTracking",rowDoc));
			
		
	

	
  		
			
					
	  	setLoPropagationDelay(Parser::get1DDouble("loPropagationDelay","GainTracking",rowDoc));
	  			
	  		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","GainTracking",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setReceiverDelay(Parser::get1DDouble("receiverDelay","GainTracking",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<delayOffset>")) {
			
	  		setDelayOffset(Parser::getDouble("delayOffset","GainTracking",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<freqOffset>")) {
			
								
	  		setFreqOffset(Parser::get1DFrequency("freqOffset","GainTracking",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<phaseOffset>")) {
			
								
	  		setPhaseOffset(Parser::get1DAngle("phaseOffset","GainTracking",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<samplingLevel>")) {
			
	  		setSamplingLevel(Parser::getFloat("samplingLevel","GainTracking",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<numAttFreq>")) {
			
	  		setNumAttFreq(Parser::getInteger("numAttFreq","GainTracking",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<attFreq>")) {
			
								
	  		setAttFreq(Parser::get1DDouble("attFreq","GainTracking",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<attSpectrum>")) {
			
								
	  		setAttSpectrum(Parser::get1DComplex("attSpectrum","GainTracking",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"GainTracking");
		}
	}
	
	void GainTrackingRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
	spectralWindowId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(feedId);
				
		
	

	
	
		
						
			eoss.writeFloat(attenuator);
				
		
	

	
	
		
						
			eoss.writeInt(numLO);
				
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) cableDelay.size());
		for (unsigned int i = 0; i < cableDelay.size(); i++)
				
			eoss.writeDouble(cableDelay.at(i));
				
				
						
		
	

	
	
		
						
			eoss.writeDouble(crossPolarizationDelay);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) loPropagationDelay.size());
		for (unsigned int i = 0; i < loPropagationDelay.size(); i++)
				
			eoss.writeDouble(loPropagationDelay.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); i++)
				
			eoss.writeInt(polarizationTypes.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) receiverDelay.size());
		for (unsigned int i = 0; i < receiverDelay.size(); i++)
				
			eoss.writeDouble(receiverDelay.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(delayOffsetExists);
	if (delayOffsetExists) {
	
	
	
		
						
			eoss.writeDouble(delayOffset);
				
		
	

	}

	eoss.writeBoolean(freqOffsetExists);
	if (freqOffsetExists) {
	
	
	
		
	Frequency::toBin(freqOffset, eoss);
		
	

	}

	eoss.writeBoolean(phaseOffsetExists);
	if (phaseOffsetExists) {
	
	
	
		
	Angle::toBin(phaseOffset, eoss);
		
	

	}

	eoss.writeBoolean(samplingLevelExists);
	if (samplingLevelExists) {
	
	
	
		
						
			eoss.writeFloat(samplingLevel);
				
		
	

	}

	eoss.writeBoolean(numAttFreqExists);
	if (numAttFreqExists) {
	
	
	
		
						
			eoss.writeInt(numAttFreq);
				
		
	

	}

	eoss.writeBoolean(attFreqExists);
	if (attFreqExists) {
	
	
	
		
		
			
		eoss.writeInt((int) attFreq.size());
		for (unsigned int i = 0; i < attFreq.size(); i++)
				
			eoss.writeDouble(attFreq.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(attSpectrumExists);
	if (attSpectrumExists) {
	
	
	
		
	Complex::toBin(attSpectrum, eoss);
		
	

	}

	}
	
void GainTrackingRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void GainTrackingRow::spectralWindowIdFromBin(EndianISStream& eiss) {
		
	
		
		
		spectralWindowId =  Tag::fromBin(eiss);
		
	
	
}
void GainTrackingRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void GainTrackingRow::feedIdFromBin(EndianISStream& eiss) {
		
	
	
		
			
		feedId =  eiss.readInt();
			
		
	
	
}
void GainTrackingRow::attenuatorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		attenuator =  eiss.readFloat();
			
		
	
	
}
void GainTrackingRow::numLOFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numLO =  eiss.readInt();
			
		
	
	
}
void GainTrackingRow::numReceptorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	
	
}
void GainTrackingRow::cableDelayFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		cableDelay.clear();
		
		unsigned int cableDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < cableDelayDim1; i++)
			
			cableDelay.push_back(eiss.readDouble());
			
	

		
	
	
}
void GainTrackingRow::crossPolarizationDelayFromBin(EndianISStream& eiss) {
		
	
	
		
			
		crossPolarizationDelay =  eiss.readDouble();
			
		
	
	
}
void GainTrackingRow::loPropagationDelayFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		loPropagationDelay.clear();
		
		unsigned int loPropagationDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < loPropagationDelayDim1; i++)
			
			loPropagationDelay.push_back(eiss.readDouble());
			
	

		
	
	
}
void GainTrackingRow::polarizationTypesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		polarizationTypes.clear();
		
		unsigned int polarizationTypesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypesDim1; i++)
			
			polarizationTypes.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	
	
}
void GainTrackingRow::receiverDelayFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		receiverDelay.clear();
		
		unsigned int receiverDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < receiverDelayDim1; i++)
			
			receiverDelay.push_back(eiss.readDouble());
			
	

		
	
	
}

void GainTrackingRow::delayOffsetFromBin(EndianISStream& eiss) {
		
	delayOffsetExists = eiss.readBoolean();
	if (delayOffsetExists) {
		
	
	
		
			
		delayOffset =  eiss.readDouble();
			
		
	

	}
	
}
void GainTrackingRow::freqOffsetFromBin(EndianISStream& eiss) {
		
	freqOffsetExists = eiss.readBoolean();
	if (freqOffsetExists) {
		
	
		
		
			
	
	freqOffset = Frequency::from1DBin(eiss);	
	

		
	

	}
	
}
void GainTrackingRow::phaseOffsetFromBin(EndianISStream& eiss) {
		
	phaseOffsetExists = eiss.readBoolean();
	if (phaseOffsetExists) {
		
	
		
		
			
	
	phaseOffset = Angle::from1DBin(eiss);	
	

		
	

	}
	
}
void GainTrackingRow::samplingLevelFromBin(EndianISStream& eiss) {
		
	samplingLevelExists = eiss.readBoolean();
	if (samplingLevelExists) {
		
	
	
		
			
		samplingLevel =  eiss.readFloat();
			
		
	

	}
	
}
void GainTrackingRow::numAttFreqFromBin(EndianISStream& eiss) {
		
	numAttFreqExists = eiss.readBoolean();
	if (numAttFreqExists) {
		
	
	
		
			
		numAttFreq =  eiss.readInt();
			
		
	

	}
	
}
void GainTrackingRow::attFreqFromBin(EndianISStream& eiss) {
		
	attFreqExists = eiss.readBoolean();
	if (attFreqExists) {
		
	
	
		
			
	
		attFreq.clear();
		
		unsigned int attFreqDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < attFreqDim1; i++)
			
			attFreq.push_back(eiss.readDouble());
			
	

		
	

	}
	
}
void GainTrackingRow::attSpectrumFromBin(EndianISStream& eiss) {
		
	attSpectrumExists = eiss.readBoolean();
	if (attSpectrumExists) {
		
	
		
		
			
	
	attSpectrum = Complex::from1DBin(eiss);	
	

		
	

	}
	
}
	
	
	GainTrackingRow* GainTrackingRow::fromBin(EndianISStream& eiss, GainTrackingTable& table, const vector<string>& attributesSeq) {
		GainTrackingRow* row = new  GainTrackingRow(table);
		
		map<string, GainTrackingAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "GainTrackingTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval GainTrackingRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void GainTrackingRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "GainTracking");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get attenuator.
 	 * @return attenuator as float
 	 */
 	float GainTrackingRow::getAttenuator() const {
	
  		return attenuator;
 	}

 	/**
 	 * Set attenuator with the specified float.
 	 * @param attenuator The float value to which attenuator is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setAttenuator (float attenuator)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->attenuator = attenuator;
	
 	}
	
	

	

	
 	/**
 	 * Get numLO.
 	 * @return numLO as int
 	 */
 	int GainTrackingRow::getNumLO() const {
	
  		return numLO;
 	}

 	/**
 	 * Set numLO with the specified int.
 	 * @param numLO The int value to which numLO is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setNumLO (int numLO)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numLO = numLO;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int GainTrackingRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get cableDelay.
 	 * @return cableDelay as vector<double >
 	 */
 	vector<double > GainTrackingRow::getCableDelay() const {
	
  		return cableDelay;
 	}

 	/**
 	 * Set cableDelay with the specified vector<double >.
 	 * @param cableDelay The vector<double > value to which cableDelay is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setCableDelay (vector<double > cableDelay)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->cableDelay = cableDelay;
	
 	}
	
	

	

	
 	/**
 	 * Get crossPolarizationDelay.
 	 * @return crossPolarizationDelay as double
 	 */
 	double GainTrackingRow::getCrossPolarizationDelay() const {
	
  		return crossPolarizationDelay;
 	}

 	/**
 	 * Set crossPolarizationDelay with the specified double.
 	 * @param crossPolarizationDelay The double value to which crossPolarizationDelay is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setCrossPolarizationDelay (double crossPolarizationDelay)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->crossPolarizationDelay = crossPolarizationDelay;
	
 	}
	
	

	

	
 	/**
 	 * Get loPropagationDelay.
 	 * @return loPropagationDelay as vector<double >
 	 */
 	vector<double > GainTrackingRow::getLoPropagationDelay() const {
	
  		return loPropagationDelay;
 	}

 	/**
 	 * Set loPropagationDelay with the specified vector<double >.
 	 * @param loPropagationDelay The vector<double > value to which loPropagationDelay is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setLoPropagationDelay (vector<double > loPropagationDelay)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->loPropagationDelay = loPropagationDelay;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > GainTrackingRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverDelay.
 	 * @return receiverDelay as vector<double >
 	 */
 	vector<double > GainTrackingRow::getReceiverDelay() const {
	
  		return receiverDelay;
 	}

 	/**
 	 * Set receiverDelay with the specified vector<double >.
 	 * @param receiverDelay The vector<double > value to which receiverDelay is to be set.
 	 
 	
 		
 	 */
 	void GainTrackingRow::setReceiverDelay (vector<double > receiverDelay)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverDelay = receiverDelay;
	
 	}
	
	

	
	/**
	 * The attribute delayOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the delayOffset attribute exists. 
	 */
	bool GainTrackingRow::isDelayOffsetExists() const {
		return delayOffsetExists;
	}
	

	
 	/**
 	 * Get delayOffset, which is optional.
 	 * @return delayOffset as double
 	 * @throw IllegalAccessException If delayOffset does not exist.
 	 */
 	double GainTrackingRow::getDelayOffset() const  {
		if (!delayOffsetExists) {
			throw IllegalAccessException("delayOffset", "GainTracking");
		}
	
  		return delayOffset;
 	}

 	/**
 	 * Set delayOffset with the specified double.
 	 * @param delayOffset The double value to which delayOffset is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setDelayOffset (double delayOffset) {
	
 		this->delayOffset = delayOffset;
	
		delayOffsetExists = true;
	
 	}
	
	
	/**
	 * Mark delayOffset, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearDelayOffset () {
		delayOffsetExists = false;
	}
	

	
	/**
	 * The attribute freqOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the freqOffset attribute exists. 
	 */
	bool GainTrackingRow::isFreqOffsetExists() const {
		return freqOffsetExists;
	}
	

	
 	/**
 	 * Get freqOffset, which is optional.
 	 * @return freqOffset as vector<Frequency >
 	 * @throw IllegalAccessException If freqOffset does not exist.
 	 */
 	vector<Frequency > GainTrackingRow::getFreqOffset() const  {
		if (!freqOffsetExists) {
			throw IllegalAccessException("freqOffset", "GainTracking");
		}
	
  		return freqOffset;
 	}

 	/**
 	 * Set freqOffset with the specified vector<Frequency >.
 	 * @param freqOffset The vector<Frequency > value to which freqOffset is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setFreqOffset (vector<Frequency > freqOffset) {
	
 		this->freqOffset = freqOffset;
	
		freqOffsetExists = true;
	
 	}
	
	
	/**
	 * Mark freqOffset, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearFreqOffset () {
		freqOffsetExists = false;
	}
	

	
	/**
	 * The attribute phaseOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseOffset attribute exists. 
	 */
	bool GainTrackingRow::isPhaseOffsetExists() const {
		return phaseOffsetExists;
	}
	

	
 	/**
 	 * Get phaseOffset, which is optional.
 	 * @return phaseOffset as vector<Angle >
 	 * @throw IllegalAccessException If phaseOffset does not exist.
 	 */
 	vector<Angle > GainTrackingRow::getPhaseOffset() const  {
		if (!phaseOffsetExists) {
			throw IllegalAccessException("phaseOffset", "GainTracking");
		}
	
  		return phaseOffset;
 	}

 	/**
 	 * Set phaseOffset with the specified vector<Angle >.
 	 * @param phaseOffset The vector<Angle > value to which phaseOffset is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setPhaseOffset (vector<Angle > phaseOffset) {
	
 		this->phaseOffset = phaseOffset;
	
		phaseOffsetExists = true;
	
 	}
	
	
	/**
	 * Mark phaseOffset, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearPhaseOffset () {
		phaseOffsetExists = false;
	}
	

	
	/**
	 * The attribute samplingLevel is optional. Return true if this attribute exists.
	 * @return true if and only if the samplingLevel attribute exists. 
	 */
	bool GainTrackingRow::isSamplingLevelExists() const {
		return samplingLevelExists;
	}
	

	
 	/**
 	 * Get samplingLevel, which is optional.
 	 * @return samplingLevel as float
 	 * @throw IllegalAccessException If samplingLevel does not exist.
 	 */
 	float GainTrackingRow::getSamplingLevel() const  {
		if (!samplingLevelExists) {
			throw IllegalAccessException("samplingLevel", "GainTracking");
		}
	
  		return samplingLevel;
 	}

 	/**
 	 * Set samplingLevel with the specified float.
 	 * @param samplingLevel The float value to which samplingLevel is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setSamplingLevel (float samplingLevel) {
	
 		this->samplingLevel = samplingLevel;
	
		samplingLevelExists = true;
	
 	}
	
	
	/**
	 * Mark samplingLevel, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearSamplingLevel () {
		samplingLevelExists = false;
	}
	

	
	/**
	 * The attribute numAttFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the numAttFreq attribute exists. 
	 */
	bool GainTrackingRow::isNumAttFreqExists() const {
		return numAttFreqExists;
	}
	

	
 	/**
 	 * Get numAttFreq, which is optional.
 	 * @return numAttFreq as int
 	 * @throw IllegalAccessException If numAttFreq does not exist.
 	 */
 	int GainTrackingRow::getNumAttFreq() const  {
		if (!numAttFreqExists) {
			throw IllegalAccessException("numAttFreq", "GainTracking");
		}
	
  		return numAttFreq;
 	}

 	/**
 	 * Set numAttFreq with the specified int.
 	 * @param numAttFreq The int value to which numAttFreq is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setNumAttFreq (int numAttFreq) {
	
 		this->numAttFreq = numAttFreq;
	
		numAttFreqExists = true;
	
 	}
	
	
	/**
	 * Mark numAttFreq, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearNumAttFreq () {
		numAttFreqExists = false;
	}
	

	
	/**
	 * The attribute attFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the attFreq attribute exists. 
	 */
	bool GainTrackingRow::isAttFreqExists() const {
		return attFreqExists;
	}
	

	
 	/**
 	 * Get attFreq, which is optional.
 	 * @return attFreq as vector<double >
 	 * @throw IllegalAccessException If attFreq does not exist.
 	 */
 	vector<double > GainTrackingRow::getAttFreq() const  {
		if (!attFreqExists) {
			throw IllegalAccessException("attFreq", "GainTracking");
		}
	
  		return attFreq;
 	}

 	/**
 	 * Set attFreq with the specified vector<double >.
 	 * @param attFreq The vector<double > value to which attFreq is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setAttFreq (vector<double > attFreq) {
	
 		this->attFreq = attFreq;
	
		attFreqExists = true;
	
 	}
	
	
	/**
	 * Mark attFreq, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearAttFreq () {
		attFreqExists = false;
	}
	

	
	/**
	 * The attribute attSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the attSpectrum attribute exists. 
	 */
	bool GainTrackingRow::isAttSpectrumExists() const {
		return attSpectrumExists;
	}
	

	
 	/**
 	 * Get attSpectrum, which is optional.
 	 * @return attSpectrum as vector<Complex >
 	 * @throw IllegalAccessException If attSpectrum does not exist.
 	 */
 	vector<Complex > GainTrackingRow::getAttSpectrum() const  {
		if (!attSpectrumExists) {
			throw IllegalAccessException("attSpectrum", "GainTracking");
		}
	
  		return attSpectrum;
 	}

 	/**
 	 * Set attSpectrum with the specified vector<Complex >.
 	 * @param attSpectrum The vector<Complex > value to which attSpectrum is to be set.
 	 
 	
 	 */
 	void GainTrackingRow::setAttSpectrum (vector<Complex > attSpectrum) {
	
 		this->attSpectrum = attSpectrum;
	
		attSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark attSpectrum, which is an optional field, as non-existent.
	 */
	void GainTrackingRow::clearAttSpectrum () {
		attSpectrumExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag GainTrackingRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void GainTrackingRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "GainTracking");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int GainTrackingRow::getFeedId() const {
	
  		return feedId;
 	}

 	/**
 	 * Set feedId with the specified int.
 	 * @param feedId The int value to which feedId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void GainTrackingRow::setFeedId (int feedId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("feedId", "GainTracking");
		
  		}
  	
 		this->feedId = feedId;
	
 	}
	
	

	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag GainTrackingRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void GainTrackingRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "GainTracking");
		
  		}
  	
 		this->spectralWindowId = spectralWindowId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* GainTrackingRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* GainTrackingRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of GainTracking table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having their attribut feedId == this->feedId
	 */
	vector <FeedRow *> GainTrackingRow::getFeeds() {
		
			return table.getContainer().getFeed().getRowByFeedId(feedId);
		
	}
	

	

	
	/**
	 * Create a GainTrackingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	GainTrackingRow::GainTrackingRow (GainTrackingTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	
		delayOffsetExists = false;
	

	
		freqOffsetExists = false;
	

	
		phaseOffsetExists = false;
	

	
		samplingLevelExists = false;
	

	
		numAttFreqExists = false;
	

	
		attFreqExists = false;
	

	
		attSpectrumExists = false;
	

	
	

	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaId"] = &GainTrackingRow::antennaIdFromBin; 
	 fromBinMethods["spectralWindowId"] = &GainTrackingRow::spectralWindowIdFromBin; 
	 fromBinMethods["timeInterval"] = &GainTrackingRow::timeIntervalFromBin; 
	 fromBinMethods["feedId"] = &GainTrackingRow::feedIdFromBin; 
	 fromBinMethods["attenuator"] = &GainTrackingRow::attenuatorFromBin; 
	 fromBinMethods["numLO"] = &GainTrackingRow::numLOFromBin; 
	 fromBinMethods["numReceptor"] = &GainTrackingRow::numReceptorFromBin; 
	 fromBinMethods["cableDelay"] = &GainTrackingRow::cableDelayFromBin; 
	 fromBinMethods["crossPolarizationDelay"] = &GainTrackingRow::crossPolarizationDelayFromBin; 
	 fromBinMethods["loPropagationDelay"] = &GainTrackingRow::loPropagationDelayFromBin; 
	 fromBinMethods["polarizationTypes"] = &GainTrackingRow::polarizationTypesFromBin; 
	 fromBinMethods["receiverDelay"] = &GainTrackingRow::receiverDelayFromBin; 
		
	
	 fromBinMethods["delayOffset"] = &GainTrackingRow::delayOffsetFromBin; 
	 fromBinMethods["freqOffset"] = &GainTrackingRow::freqOffsetFromBin; 
	 fromBinMethods["phaseOffset"] = &GainTrackingRow::phaseOffsetFromBin; 
	 fromBinMethods["samplingLevel"] = &GainTrackingRow::samplingLevelFromBin; 
	 fromBinMethods["numAttFreq"] = &GainTrackingRow::numAttFreqFromBin; 
	 fromBinMethods["attFreq"] = &GainTrackingRow::attFreqFromBin; 
	 fromBinMethods["attSpectrum"] = &GainTrackingRow::attSpectrumFromBin; 
	
	}
	
	GainTrackingRow::GainTrackingRow (GainTrackingTable &t, GainTrackingRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	
		delayOffsetExists = false;
	

	
		freqOffsetExists = false;
	

	
		phaseOffsetExists = false;
	

	
		samplingLevelExists = false;
	

	
		numAttFreqExists = false;
	

	
		attFreqExists = false;
	

	
		attSpectrumExists = false;
	

	
	

	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
			feedId = row.feedId;
		
		
		
		
			attenuator = row.attenuator;
		
			numLO = row.numLO;
		
			numReceptor = row.numReceptor;
		
			cableDelay = row.cableDelay;
		
			crossPolarizationDelay = row.crossPolarizationDelay;
		
			loPropagationDelay = row.loPropagationDelay;
		
			polarizationTypes = row.polarizationTypes;
		
			receiverDelay = row.receiverDelay;
		
		
		
		
		if (row.delayOffsetExists) {
			delayOffset = row.delayOffset;		
			delayOffsetExists = true;
		}
		else
			delayOffsetExists = false;
		
		if (row.freqOffsetExists) {
			freqOffset = row.freqOffset;		
			freqOffsetExists = true;
		}
		else
			freqOffsetExists = false;
		
		if (row.phaseOffsetExists) {
			phaseOffset = row.phaseOffset;		
			phaseOffsetExists = true;
		}
		else
			phaseOffsetExists = false;
		
		if (row.samplingLevelExists) {
			samplingLevel = row.samplingLevel;		
			samplingLevelExists = true;
		}
		else
			samplingLevelExists = false;
		
		if (row.numAttFreqExists) {
			numAttFreq = row.numAttFreq;		
			numAttFreqExists = true;
		}
		else
			numAttFreqExists = false;
		
		if (row.attFreqExists) {
			attFreq = row.attFreq;		
			attFreqExists = true;
		}
		else
			attFreqExists = false;
		
		if (row.attSpectrumExists) {
			attSpectrum = row.attSpectrum;		
			attSpectrumExists = true;
		}
		else
			attSpectrumExists = false;
		
		}
		
		 fromBinMethods["antennaId"] = &GainTrackingRow::antennaIdFromBin; 
		 fromBinMethods["spectralWindowId"] = &GainTrackingRow::spectralWindowIdFromBin; 
		 fromBinMethods["timeInterval"] = &GainTrackingRow::timeIntervalFromBin; 
		 fromBinMethods["feedId"] = &GainTrackingRow::feedIdFromBin; 
		 fromBinMethods["attenuator"] = &GainTrackingRow::attenuatorFromBin; 
		 fromBinMethods["numLO"] = &GainTrackingRow::numLOFromBin; 
		 fromBinMethods["numReceptor"] = &GainTrackingRow::numReceptorFromBin; 
		 fromBinMethods["cableDelay"] = &GainTrackingRow::cableDelayFromBin; 
		 fromBinMethods["crossPolarizationDelay"] = &GainTrackingRow::crossPolarizationDelayFromBin; 
		 fromBinMethods["loPropagationDelay"] = &GainTrackingRow::loPropagationDelayFromBin; 
		 fromBinMethods["polarizationTypes"] = &GainTrackingRow::polarizationTypesFromBin; 
		 fromBinMethods["receiverDelay"] = &GainTrackingRow::receiverDelayFromBin; 
			
	
		 fromBinMethods["delayOffset"] = &GainTrackingRow::delayOffsetFromBin; 
		 fromBinMethods["freqOffset"] = &GainTrackingRow::freqOffsetFromBin; 
		 fromBinMethods["phaseOffset"] = &GainTrackingRow::phaseOffsetFromBin; 
		 fromBinMethods["samplingLevel"] = &GainTrackingRow::samplingLevelFromBin; 
		 fromBinMethods["numAttFreq"] = &GainTrackingRow::numAttFreqFromBin; 
		 fromBinMethods["attFreq"] = &GainTrackingRow::attFreqFromBin; 
		 fromBinMethods["attSpectrum"] = &GainTrackingRow::attSpectrumFromBin; 
			
	}

	
	bool GainTrackingRow::compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, float attenuator, int numLO, int numReceptor, vector<double > cableDelay, double crossPolarizationDelay, vector<double > loPropagationDelay, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > receiverDelay) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->feedId == feedId);
		
		if (!result) return false;
	

	
		
		result = result && (this->attenuator == attenuator);
		
		if (!result) return false;
	

	
		
		result = result && (this->numLO == numLO);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->cableDelay == cableDelay);
		
		if (!result) return false;
	

	
		
		result = result && (this->crossPolarizationDelay == crossPolarizationDelay);
		
		if (!result) return false;
	

	
		
		result = result && (this->loPropagationDelay == loPropagationDelay);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverDelay == receiverDelay);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool GainTrackingRow::compareRequiredValue(float attenuator, int numLO, int numReceptor, vector<double > cableDelay, double crossPolarizationDelay, vector<double > loPropagationDelay, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > receiverDelay) {
		bool result;
		result = true;
		
	
		if (!(this->attenuator == attenuator)) return false;
	

	
		if (!(this->numLO == numLO)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->cableDelay == cableDelay)) return false;
	

	
		if (!(this->crossPolarizationDelay == crossPolarizationDelay)) return false;
	

	
		if (!(this->loPropagationDelay == loPropagationDelay)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->receiverDelay == receiverDelay)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the GainTrackingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool GainTrackingRow::equalByRequiredValue(GainTrackingRow* x) {
		
			
		if (this->attenuator != x->attenuator) return false;
			
		if (this->numLO != x->numLO) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->cableDelay != x->cableDelay) return false;
			
		if (this->crossPolarizationDelay != x->crossPolarizationDelay) return false;
			
		if (this->loPropagationDelay != x->loPropagationDelay) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->receiverDelay != x->receiverDelay) return false;
			
		
		return true;
	}	
	
/*
	 map<string, GainTrackingAttributeFromBin> GainTrackingRow::initFromBinMethods() {
		map<string, GainTrackingAttributeFromBin> result;
		
		result["antennaId"] = &GainTrackingRow::antennaIdFromBin;
		result["spectralWindowId"] = &GainTrackingRow::spectralWindowIdFromBin;
		result["timeInterval"] = &GainTrackingRow::timeIntervalFromBin;
		result["feedId"] = &GainTrackingRow::feedIdFromBin;
		result["attenuator"] = &GainTrackingRow::attenuatorFromBin;
		result["numLO"] = &GainTrackingRow::numLOFromBin;
		result["numReceptor"] = &GainTrackingRow::numReceptorFromBin;
		result["cableDelay"] = &GainTrackingRow::cableDelayFromBin;
		result["crossPolarizationDelay"] = &GainTrackingRow::crossPolarizationDelayFromBin;
		result["loPropagationDelay"] = &GainTrackingRow::loPropagationDelayFromBin;
		result["polarizationTypes"] = &GainTrackingRow::polarizationTypesFromBin;
		result["receiverDelay"] = &GainTrackingRow::receiverDelayFromBin;
		
		
		result["delayOffset"] = &GainTrackingRow::delayOffsetFromBin;
		result["freqOffset"] = &GainTrackingRow::freqOffsetFromBin;
		result["phaseOffset"] = &GainTrackingRow::phaseOffsetFromBin;
		result["samplingLevel"] = &GainTrackingRow::samplingLevelFromBin;
		result["numAttFreq"] = &GainTrackingRow::numAttFreqFromBin;
		result["attFreq"] = &GainTrackingRow::attFreqFromBin;
		result["attSpectrum"] = &GainTrackingRow::attSpectrumFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
