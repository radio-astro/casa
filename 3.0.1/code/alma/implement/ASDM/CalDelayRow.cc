
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
 * File CalDelayRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalDelayRow.h>
#include <CalDelayTable.h>

#include <CalDataTable.h>
#include <CalDataRow.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>
	

using asdm::ASDM;
using asdm::CalDelayRow;
using asdm::CalDelayTable;

using asdm::CalDataTable;
using asdm::CalDataRow;

using asdm::CalReductionTable;
using asdm::CalReductionRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	CalDelayRow::~CalDelayRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalDelayTable &CalDelayRow::getTable() const {
		return table;
	}
	
	void CalDelayRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalDelayRowIDL struct.
	 */
	CalDelayRowIDL *CalDelayRow::toIDL() const {
		CalDelayRowIDL *x = new CalDelayRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->atmPhaseCorrection = atmPhaseCorrection;
 				
 			
		
	

	
  		
		
		
			
				
		x->basebandName = basebandName;
 				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->refAntennaName = CORBA::string_dup(refAntennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
		x->delayError.length(delayError.size());
		for (unsigned int i = 0; i < delayError.size(); ++i) {
			
				
			x->delayError[i] = delayError.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->delayOffset.length(delayOffset.size());
		for (unsigned int i = 0; i < delayOffset.size(); ++i) {
			
				
			x->delayOffset[i] = delayOffset.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->reducedChiSquared.length(reducedChiSquared.size());
		for (unsigned int i = 0; i < reducedChiSquared.size(); ++i) {
			
				
			x->reducedChiSquared[i] = reducedChiSquared.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->crossDelayOffsetExists = crossDelayOffsetExists;
		
		
			
				
		x->crossDelayOffset = crossDelayOffset;
 				
 			
		
	

	
  		
		
		x->crossDelayOffsetErrorExists = crossDelayOffsetErrorExists;
		
		
			
				
		x->crossDelayOffsetError = crossDelayOffsetError;
 				
 			
		
	

	
  		
		
		x->numSidebandExists = numSidebandExists;
		
		
			
				
		x->numSideband = numSideband;
 				
 			
		
	

	
  		
		
		x->refFreqExists = refFreqExists;
		
		
			
		x->refFreq.length(refFreq.size());
		for (unsigned int i = 0; i < refFreq.size(); ++i) {
			
			x->refFreq[i] = refFreq.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		x->refFreqPhaseExists = refFreqPhaseExists;
		
		
			
		x->refFreqPhase.length(refFreqPhase.size());
		for (unsigned int i = 0; i < refFreqPhase.size(); ++i) {
			
			x->refFreqPhase[i] = refFreqPhase.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->sidebandsExists = sidebandsExists;
		
		
			
		x->sidebands.length(sidebands.size());
		for (unsigned int i = 0; i < sidebands.size(); ++i) {
			
				
			x->sidebands[i] = sidebands.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalDelayRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalDelayRow::setFromIDL (CalDelayRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setAtmPhaseCorrection(x.atmPhaseCorrection);
  			
 		
		
	

	
		
		
			
		setBasebandName(x.basebandName);
  			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setRefAntennaName(string (x.refAntennaName));
			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		delayError .clear();
		for (unsigned int i = 0; i <x.delayError.length(); ++i) {
			
			delayError.push_back(x.delayError[i]);
  			
		}
			
  		
		
	

	
		
		
			
		delayOffset .clear();
		for (unsigned int i = 0; i <x.delayOffset.length(); ++i) {
			
			delayOffset.push_back(x.delayOffset[i]);
  			
		}
			
  		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		reducedChiSquared .clear();
		for (unsigned int i = 0; i <x.reducedChiSquared.length(); ++i) {
			
			reducedChiSquared.push_back(x.reducedChiSquared[i]);
  			
		}
			
  		
		
	

	
		
		crossDelayOffsetExists = x.crossDelayOffsetExists;
		if (x.crossDelayOffsetExists) {
		
		
			
		setCrossDelayOffset(x.crossDelayOffset);
  			
 		
		
		}
		
	

	
		
		crossDelayOffsetErrorExists = x.crossDelayOffsetErrorExists;
		if (x.crossDelayOffsetErrorExists) {
		
		
			
		setCrossDelayOffsetError(x.crossDelayOffsetError);
  			
 		
		
		}
		
	

	
		
		numSidebandExists = x.numSidebandExists;
		if (x.numSidebandExists) {
		
		
			
		setNumSideband(x.numSideband);
  			
 		
		
		}
		
	

	
		
		refFreqExists = x.refFreqExists;
		if (x.refFreqExists) {
		
		
			
		refFreq .clear();
		for (unsigned int i = 0; i <x.refFreq.length(); ++i) {
			
			refFreq.push_back(Frequency (x.refFreq[i]));
			
		}
			
  		
		
		}
		
	

	
		
		refFreqPhaseExists = x.refFreqPhaseExists;
		if (x.refFreqPhaseExists) {
		
		
			
		refFreqPhase .clear();
		for (unsigned int i = 0; i <x.refFreqPhase.length(); ++i) {
			
			refFreqPhase.push_back(Angle (x.refFreqPhase[i]));
			
		}
			
  		
		
		}
		
	

	
		
		sidebandsExists = x.sidebandsExists;
		if (x.sidebandsExists) {
		
		
			
		sidebands .clear();
		for (unsigned int i = 0; i <x.sidebands.length(); ++i) {
			
			sidebands.push_back(x.sidebands[i]);
  			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalDelay");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalDelayRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrection", atmPhaseCorrection));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("basebandName", basebandName));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(refAntennaName, "refAntennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(delayError, "delayError", buf);
		
		
	

  	
 		
		
		Parser::toXML(delayOffset, "delayOffset", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(reducedChiSquared, "reducedChiSquared", buf);
		
		
	

  	
 		
		if (crossDelayOffsetExists) {
		
		
		Parser::toXML(crossDelayOffset, "crossDelayOffset", buf);
		
		
		}
		
	

  	
 		
		if (crossDelayOffsetErrorExists) {
		
		
		Parser::toXML(crossDelayOffsetError, "crossDelayOffsetError", buf);
		
		
		}
		
	

  	
 		
		if (numSidebandExists) {
		
		
		Parser::toXML(numSideband, "numSideband", buf);
		
		
		}
		
	

  	
 		
		if (refFreqExists) {
		
		
		Parser::toXML(refFreq, "refFreq", buf);
		
		
		}
		
	

  	
 		
		if (refFreqPhaseExists) {
		
		
		Parser::toXML(refFreqPhase, "refFreqPhase", buf);
		
		
		}
		
	

  	
 		
		if (sidebandsExists) {
		
		
			buf.append(EnumerationParser::toXML("sidebands", sidebands));
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(calDataId, "calDataId", buf);
		
		
	

  	
 		
		
		Parser::toXML(calReductionId, "calReductionId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void CalDelayRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalDelay",rowDoc));
			
		
	

	
		
		
		
		atmPhaseCorrection = EnumerationParser::getAtmPhaseCorrection("atmPhaseCorrection","CalDelay",rowDoc);
		
		
		
	

	
		
		
		
		basebandName = EnumerationParser::getBasebandName("basebandName","CalDelay",rowDoc);
		
		
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalDelay",rowDoc);
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalDelay",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalDelay",rowDoc));
			
		
	

	
  		
			
	  	setRefAntennaName(Parser::getString("refAntennaName","CalDelay",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalDelay",rowDoc));
			
		
	

	
  		
			
					
	  	setDelayError(Parser::get1DDouble("delayError","CalDelay",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setDelayOffset(Parser::get1DDouble("delayOffset","CalDelay",rowDoc));
	  			
	  		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalDelay",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setReducedChiSquared(Parser::get1DDouble("reducedChiSquared","CalDelay",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<crossDelayOffset>")) {
			
	  		setCrossDelayOffset(Parser::getDouble("crossDelayOffset","CalDelay",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<crossDelayOffsetError>")) {
			
	  		setCrossDelayOffsetError(Parser::getDouble("crossDelayOffsetError","CalDelay",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<numSideband>")) {
			
	  		setNumSideband(Parser::getInteger("numSideband","CalDelay",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<refFreq>")) {
			
								
	  		setRefFreq(Parser::get1DFrequency("refFreq","CalDelay",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<refFreqPhase>")) {
			
								
	  		setRefFreqPhase(Parser::get1DAngle("refFreqPhase","CalDelay",rowDoc));
	  			
	  		
		}
 		
	

	
		
	if (row.isStr("<sidebands>")) {
		
		
		
		sidebands = EnumerationParser::getReceiverSideband1D("sidebands","CalDelay",rowDoc);			
		
		
		
		sidebandsExists = true;
	}
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalDelay");
		}
	}
	
	void CalDelayRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeString(antennaName);
				
		
	

	
	
		
					
			eoss.writeInt(atmPhaseCorrection);
				
		
	

	
	
		
					
			eoss.writeInt(basebandName);
				
		
	

	
	
		
					
			eoss.writeInt(receiverBand);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(refAntennaName);
				
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) delayError.size());
		for (unsigned int i = 0; i < delayError.size(); i++)
				
			eoss.writeDouble(delayError.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) delayOffset.size());
		for (unsigned int i = 0; i < delayOffset.size(); i++)
				
			eoss.writeDouble(delayOffset.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); i++)
				
			eoss.writeInt(polarizationTypes.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) reducedChiSquared.size());
		for (unsigned int i = 0; i < reducedChiSquared.size(); i++)
				
			eoss.writeDouble(reducedChiSquared.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(crossDelayOffsetExists);
	if (crossDelayOffsetExists) {
	
	
	
		
						
			eoss.writeDouble(crossDelayOffset);
				
		
	

	}

	eoss.writeBoolean(crossDelayOffsetErrorExists);
	if (crossDelayOffsetErrorExists) {
	
	
	
		
						
			eoss.writeDouble(crossDelayOffsetError);
				
		
	

	}

	eoss.writeBoolean(numSidebandExists);
	if (numSidebandExists) {
	
	
	
		
						
			eoss.writeInt(numSideband);
				
		
	

	}

	eoss.writeBoolean(refFreqExists);
	if (refFreqExists) {
	
	
	
		
	Frequency::toBin(refFreq, eoss);
		
	

	}

	eoss.writeBoolean(refFreqPhaseExists);
	if (refFreqPhaseExists) {
	
	
	
		
	Angle::toBin(refFreqPhase, eoss);
		
	

	}

	eoss.writeBoolean(sidebandsExists);
	if (sidebandsExists) {
	
	
	
		
		
			
		eoss.writeInt((int) sidebands.size());
		for (unsigned int i = 0; i < sidebands.size(); i++)
				
			eoss.writeInt(sidebands.at(i));
				
				
						
		
	

	}

	}
	
void CalDelayRow::antennaNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaName =  eiss.readString();
			
		
	
	
}
void CalDelayRow::atmPhaseCorrectionFromBin(EndianISStream& eiss) {
		
	
	
		
			
		atmPhaseCorrection = CAtmPhaseCorrection::from_int(eiss.readInt());
			
		
	
	
}
void CalDelayRow::basebandNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		basebandName = CBasebandName::from_int(eiss.readInt());
			
		
	
	
}
void CalDelayRow::receiverBandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		receiverBand = CReceiverBand::from_int(eiss.readInt());
			
		
	
	
}
void CalDelayRow::calDataIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calDataId =  Tag::fromBin(eiss);
		
	
	
}
void CalDelayRow::calReductionIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calReductionId =  Tag::fromBin(eiss);
		
	
	
}
void CalDelayRow::startValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		startValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalDelayRow::endValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		endValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalDelayRow::refAntennaNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		refAntennaName =  eiss.readString();
			
		
	
	
}
void CalDelayRow::numReceptorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	
	
}
void CalDelayRow::delayErrorFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		delayError.clear();
		
		unsigned int delayErrorDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < delayErrorDim1; i++)
			
			delayError.push_back(eiss.readDouble());
			
	

		
	
	
}
void CalDelayRow::delayOffsetFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		delayOffset.clear();
		
		unsigned int delayOffsetDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < delayOffsetDim1; i++)
			
			delayOffset.push_back(eiss.readDouble());
			
	

		
	
	
}
void CalDelayRow::polarizationTypesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		polarizationTypes.clear();
		
		unsigned int polarizationTypesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypesDim1; i++)
			
			polarizationTypes.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	
	
}
void CalDelayRow::reducedChiSquaredFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		reducedChiSquared.clear();
		
		unsigned int reducedChiSquaredDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < reducedChiSquaredDim1; i++)
			
			reducedChiSquared.push_back(eiss.readDouble());
			
	

		
	
	
}

void CalDelayRow::crossDelayOffsetFromBin(EndianISStream& eiss) {
		
	crossDelayOffsetExists = eiss.readBoolean();
	if (crossDelayOffsetExists) {
		
	
	
		
			
		crossDelayOffset =  eiss.readDouble();
			
		
	

	}
	
}
void CalDelayRow::crossDelayOffsetErrorFromBin(EndianISStream& eiss) {
		
	crossDelayOffsetErrorExists = eiss.readBoolean();
	if (crossDelayOffsetErrorExists) {
		
	
	
		
			
		crossDelayOffsetError =  eiss.readDouble();
			
		
	

	}
	
}
void CalDelayRow::numSidebandFromBin(EndianISStream& eiss) {
		
	numSidebandExists = eiss.readBoolean();
	if (numSidebandExists) {
		
	
	
		
			
		numSideband =  eiss.readInt();
			
		
	

	}
	
}
void CalDelayRow::refFreqFromBin(EndianISStream& eiss) {
		
	refFreqExists = eiss.readBoolean();
	if (refFreqExists) {
		
	
		
		
			
	
	refFreq = Frequency::from1DBin(eiss);	
	

		
	

	}
	
}
void CalDelayRow::refFreqPhaseFromBin(EndianISStream& eiss) {
		
	refFreqPhaseExists = eiss.readBoolean();
	if (refFreqPhaseExists) {
		
	
		
		
			
	
	refFreqPhase = Angle::from1DBin(eiss);	
	

		
	

	}
	
}
void CalDelayRow::sidebandsFromBin(EndianISStream& eiss) {
		
	sidebandsExists = eiss.readBoolean();
	if (sidebandsExists) {
		
	
	
		
			
	
		sidebands.clear();
		
		unsigned int sidebandsDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < sidebandsDim1; i++)
			
			sidebands.push_back(CReceiverSideband::from_int(eiss.readInt()));
			
	

		
	

	}
	
}
	
	
	CalDelayRow* CalDelayRow::fromBin(EndianISStream& eiss, CalDelayTable& table, const vector<string>& attributesSeq) {
		CalDelayRow* row = new  CalDelayRow(table);
		
		map<string, CalDelayAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "CalDelayTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaName.
 	 * @return antennaName as string
 	 */
 	string CalDelayRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalDelay");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as AtmPhaseCorrectionMod::AtmPhaseCorrection
 	 */
 	AtmPhaseCorrectionMod::AtmPhaseCorrection CalDelayRow::getAtmPhaseCorrection() const {
	
  		return atmPhaseCorrection;
 	}

 	/**
 	 * Set atmPhaseCorrection with the specified AtmPhaseCorrectionMod::AtmPhaseCorrection.
 	 * @param atmPhaseCorrection The AtmPhaseCorrectionMod::AtmPhaseCorrection value to which atmPhaseCorrection is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setAtmPhaseCorrection (AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("atmPhaseCorrection", "CalDelay");
		
  		}
  	
 		this->atmPhaseCorrection = atmPhaseCorrection;
	
 	}
	
	

	

	
 	/**
 	 * Get basebandName.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 */
 	BasebandNameMod::BasebandName CalDelayRow::getBasebandName() const {
	
  		return basebandName;
 	}

 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setBasebandName (BasebandNameMod::BasebandName basebandName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("basebandName", "CalDelay");
		
  		}
  	
 		this->basebandName = basebandName;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalDelayRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalDelay");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalDelayRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalDelayRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get refAntennaName.
 	 * @return refAntennaName as string
 	 */
 	string CalDelayRow::getRefAntennaName() const {
	
  		return refAntennaName;
 	}

 	/**
 	 * Set refAntennaName with the specified string.
 	 * @param refAntennaName The string value to which refAntennaName is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setRefAntennaName (string refAntennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refAntennaName = refAntennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalDelayRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get delayError.
 	 * @return delayError as vector<double >
 	 */
 	vector<double > CalDelayRow::getDelayError() const {
	
  		return delayError;
 	}

 	/**
 	 * Set delayError with the specified vector<double >.
 	 * @param delayError The vector<double > value to which delayError is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setDelayError (vector<double > delayError)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->delayError = delayError;
	
 	}
	
	

	

	
 	/**
 	 * Get delayOffset.
 	 * @return delayOffset as vector<double >
 	 */
 	vector<double > CalDelayRow::getDelayOffset() const {
	
  		return delayOffset;
 	}

 	/**
 	 * Set delayOffset with the specified vector<double >.
 	 * @param delayOffset The vector<double > value to which delayOffset is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setDelayOffset (vector<double > delayOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->delayOffset = delayOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalDelayRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get reducedChiSquared.
 	 * @return reducedChiSquared as vector<double >
 	 */
 	vector<double > CalDelayRow::getReducedChiSquared() const {
	
  		return reducedChiSquared;
 	}

 	/**
 	 * Set reducedChiSquared with the specified vector<double >.
 	 * @param reducedChiSquared The vector<double > value to which reducedChiSquared is to be set.
 	 
 	
 		
 	 */
 	void CalDelayRow::setReducedChiSquared (vector<double > reducedChiSquared)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->reducedChiSquared = reducedChiSquared;
	
 	}
	
	

	
	/**
	 * The attribute crossDelayOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the crossDelayOffset attribute exists. 
	 */
	bool CalDelayRow::isCrossDelayOffsetExists() const {
		return crossDelayOffsetExists;
	}
	

	
 	/**
 	 * Get crossDelayOffset, which is optional.
 	 * @return crossDelayOffset as double
 	 * @throw IllegalAccessException If crossDelayOffset does not exist.
 	 */
 	double CalDelayRow::getCrossDelayOffset() const  {
		if (!crossDelayOffsetExists) {
			throw IllegalAccessException("crossDelayOffset", "CalDelay");
		}
	
  		return crossDelayOffset;
 	}

 	/**
 	 * Set crossDelayOffset with the specified double.
 	 * @param crossDelayOffset The double value to which crossDelayOffset is to be set.
 	 
 	
 	 */
 	void CalDelayRow::setCrossDelayOffset (double crossDelayOffset) {
	
 		this->crossDelayOffset = crossDelayOffset;
	
		crossDelayOffsetExists = true;
	
 	}
	
	
	/**
	 * Mark crossDelayOffset, which is an optional field, as non-existent.
	 */
	void CalDelayRow::clearCrossDelayOffset () {
		crossDelayOffsetExists = false;
	}
	

	
	/**
	 * The attribute crossDelayOffsetError is optional. Return true if this attribute exists.
	 * @return true if and only if the crossDelayOffsetError attribute exists. 
	 */
	bool CalDelayRow::isCrossDelayOffsetErrorExists() const {
		return crossDelayOffsetErrorExists;
	}
	

	
 	/**
 	 * Get crossDelayOffsetError, which is optional.
 	 * @return crossDelayOffsetError as double
 	 * @throw IllegalAccessException If crossDelayOffsetError does not exist.
 	 */
 	double CalDelayRow::getCrossDelayOffsetError() const  {
		if (!crossDelayOffsetErrorExists) {
			throw IllegalAccessException("crossDelayOffsetError", "CalDelay");
		}
	
  		return crossDelayOffsetError;
 	}

 	/**
 	 * Set crossDelayOffsetError with the specified double.
 	 * @param crossDelayOffsetError The double value to which crossDelayOffsetError is to be set.
 	 
 	
 	 */
 	void CalDelayRow::setCrossDelayOffsetError (double crossDelayOffsetError) {
	
 		this->crossDelayOffsetError = crossDelayOffsetError;
	
		crossDelayOffsetErrorExists = true;
	
 	}
	
	
	/**
	 * Mark crossDelayOffsetError, which is an optional field, as non-existent.
	 */
	void CalDelayRow::clearCrossDelayOffsetError () {
		crossDelayOffsetErrorExists = false;
	}
	

	
	/**
	 * The attribute numSideband is optional. Return true if this attribute exists.
	 * @return true if and only if the numSideband attribute exists. 
	 */
	bool CalDelayRow::isNumSidebandExists() const {
		return numSidebandExists;
	}
	

	
 	/**
 	 * Get numSideband, which is optional.
 	 * @return numSideband as int
 	 * @throw IllegalAccessException If numSideband does not exist.
 	 */
 	int CalDelayRow::getNumSideband() const  {
		if (!numSidebandExists) {
			throw IllegalAccessException("numSideband", "CalDelay");
		}
	
  		return numSideband;
 	}

 	/**
 	 * Set numSideband with the specified int.
 	 * @param numSideband The int value to which numSideband is to be set.
 	 
 	
 	 */
 	void CalDelayRow::setNumSideband (int numSideband) {
	
 		this->numSideband = numSideband;
	
		numSidebandExists = true;
	
 	}
	
	
	/**
	 * Mark numSideband, which is an optional field, as non-existent.
	 */
	void CalDelayRow::clearNumSideband () {
		numSidebandExists = false;
	}
	

	
	/**
	 * The attribute refFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the refFreq attribute exists. 
	 */
	bool CalDelayRow::isRefFreqExists() const {
		return refFreqExists;
	}
	

	
 	/**
 	 * Get refFreq, which is optional.
 	 * @return refFreq as vector<Frequency >
 	 * @throw IllegalAccessException If refFreq does not exist.
 	 */
 	vector<Frequency > CalDelayRow::getRefFreq() const  {
		if (!refFreqExists) {
			throw IllegalAccessException("refFreq", "CalDelay");
		}
	
  		return refFreq;
 	}

 	/**
 	 * Set refFreq with the specified vector<Frequency >.
 	 * @param refFreq The vector<Frequency > value to which refFreq is to be set.
 	 
 	
 	 */
 	void CalDelayRow::setRefFreq (vector<Frequency > refFreq) {
	
 		this->refFreq = refFreq;
	
		refFreqExists = true;
	
 	}
	
	
	/**
	 * Mark refFreq, which is an optional field, as non-existent.
	 */
	void CalDelayRow::clearRefFreq () {
		refFreqExists = false;
	}
	

	
	/**
	 * The attribute refFreqPhase is optional. Return true if this attribute exists.
	 * @return true if and only if the refFreqPhase attribute exists. 
	 */
	bool CalDelayRow::isRefFreqPhaseExists() const {
		return refFreqPhaseExists;
	}
	

	
 	/**
 	 * Get refFreqPhase, which is optional.
 	 * @return refFreqPhase as vector<Angle >
 	 * @throw IllegalAccessException If refFreqPhase does not exist.
 	 */
 	vector<Angle > CalDelayRow::getRefFreqPhase() const  {
		if (!refFreqPhaseExists) {
			throw IllegalAccessException("refFreqPhase", "CalDelay");
		}
	
  		return refFreqPhase;
 	}

 	/**
 	 * Set refFreqPhase with the specified vector<Angle >.
 	 * @param refFreqPhase The vector<Angle > value to which refFreqPhase is to be set.
 	 
 	
 	 */
 	void CalDelayRow::setRefFreqPhase (vector<Angle > refFreqPhase) {
	
 		this->refFreqPhase = refFreqPhase;
	
		refFreqPhaseExists = true;
	
 	}
	
	
	/**
	 * Mark refFreqPhase, which is an optional field, as non-existent.
	 */
	void CalDelayRow::clearRefFreqPhase () {
		refFreqPhaseExists = false;
	}
	

	
	/**
	 * The attribute sidebands is optional. Return true if this attribute exists.
	 * @return true if and only if the sidebands attribute exists. 
	 */
	bool CalDelayRow::isSidebandsExists() const {
		return sidebandsExists;
	}
	

	
 	/**
 	 * Get sidebands, which is optional.
 	 * @return sidebands as vector<ReceiverSidebandMod::ReceiverSideband >
 	 * @throw IllegalAccessException If sidebands does not exist.
 	 */
 	vector<ReceiverSidebandMod::ReceiverSideband > CalDelayRow::getSidebands() const  {
		if (!sidebandsExists) {
			throw IllegalAccessException("sidebands", "CalDelay");
		}
	
  		return sidebands;
 	}

 	/**
 	 * Set sidebands with the specified vector<ReceiverSidebandMod::ReceiverSideband >.
 	 * @param sidebands The vector<ReceiverSidebandMod::ReceiverSideband > value to which sidebands is to be set.
 	 
 	
 	 */
 	void CalDelayRow::setSidebands (vector<ReceiverSidebandMod::ReceiverSideband > sidebands) {
	
 		this->sidebands = sidebands;
	
		sidebandsExists = true;
	
 	}
	
	
	/**
	 * Mark sidebands, which is an optional field, as non-existent.
	 */
	void CalDelayRow::clearSidebands () {
		sidebandsExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalDelayRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalDelay");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalDelayRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalDelayRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalDelay");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalDelayRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalDelayRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	/**
	 * Create a CalDelayRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalDelayRow::CalDelayRow (CalDelayTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	
		crossDelayOffsetExists = false;
	

	
		crossDelayOffsetErrorExists = false;
	

	
		numSidebandExists = false;
	

	
		refFreqExists = false;
	

	
		refFreqPhaseExists = false;
	

	
		sidebandsExists = false;
	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
atmPhaseCorrection = CAtmPhaseCorrection::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
basebandName = CBasebandName::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaName"] = &CalDelayRow::antennaNameFromBin; 
	 fromBinMethods["atmPhaseCorrection"] = &CalDelayRow::atmPhaseCorrectionFromBin; 
	 fromBinMethods["basebandName"] = &CalDelayRow::basebandNameFromBin; 
	 fromBinMethods["receiverBand"] = &CalDelayRow::receiverBandFromBin; 
	 fromBinMethods["calDataId"] = &CalDelayRow::calDataIdFromBin; 
	 fromBinMethods["calReductionId"] = &CalDelayRow::calReductionIdFromBin; 
	 fromBinMethods["startValidTime"] = &CalDelayRow::startValidTimeFromBin; 
	 fromBinMethods["endValidTime"] = &CalDelayRow::endValidTimeFromBin; 
	 fromBinMethods["refAntennaName"] = &CalDelayRow::refAntennaNameFromBin; 
	 fromBinMethods["numReceptor"] = &CalDelayRow::numReceptorFromBin; 
	 fromBinMethods["delayError"] = &CalDelayRow::delayErrorFromBin; 
	 fromBinMethods["delayOffset"] = &CalDelayRow::delayOffsetFromBin; 
	 fromBinMethods["polarizationTypes"] = &CalDelayRow::polarizationTypesFromBin; 
	 fromBinMethods["reducedChiSquared"] = &CalDelayRow::reducedChiSquaredFromBin; 
		
	
	 fromBinMethods["crossDelayOffset"] = &CalDelayRow::crossDelayOffsetFromBin; 
	 fromBinMethods["crossDelayOffsetError"] = &CalDelayRow::crossDelayOffsetErrorFromBin; 
	 fromBinMethods["numSideband"] = &CalDelayRow::numSidebandFromBin; 
	 fromBinMethods["refFreq"] = &CalDelayRow::refFreqFromBin; 
	 fromBinMethods["refFreqPhase"] = &CalDelayRow::refFreqPhaseFromBin; 
	 fromBinMethods["sidebands"] = &CalDelayRow::sidebandsFromBin; 
	
	}
	
	CalDelayRow::CalDelayRow (CalDelayTable &t, CalDelayRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	
		crossDelayOffsetExists = false;
	

	
		crossDelayOffsetErrorExists = false;
	

	
		numSidebandExists = false;
	

	
		refFreqExists = false;
	

	
		refFreqPhaseExists = false;
	

	
		sidebandsExists = false;
	

	
	

	
		
		}
		else {
	
		
			antennaName = row.antennaName;
		
			atmPhaseCorrection = row.atmPhaseCorrection;
		
			basebandName = row.basebandName;
		
			receiverBand = row.receiverBand;
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			refAntennaName = row.refAntennaName;
		
			numReceptor = row.numReceptor;
		
			delayError = row.delayError;
		
			delayOffset = row.delayOffset;
		
			polarizationTypes = row.polarizationTypes;
		
			reducedChiSquared = row.reducedChiSquared;
		
		
		
		
		if (row.crossDelayOffsetExists) {
			crossDelayOffset = row.crossDelayOffset;		
			crossDelayOffsetExists = true;
		}
		else
			crossDelayOffsetExists = false;
		
		if (row.crossDelayOffsetErrorExists) {
			crossDelayOffsetError = row.crossDelayOffsetError;		
			crossDelayOffsetErrorExists = true;
		}
		else
			crossDelayOffsetErrorExists = false;
		
		if (row.numSidebandExists) {
			numSideband = row.numSideband;		
			numSidebandExists = true;
		}
		else
			numSidebandExists = false;
		
		if (row.refFreqExists) {
			refFreq = row.refFreq;		
			refFreqExists = true;
		}
		else
			refFreqExists = false;
		
		if (row.refFreqPhaseExists) {
			refFreqPhase = row.refFreqPhase;		
			refFreqPhaseExists = true;
		}
		else
			refFreqPhaseExists = false;
		
		if (row.sidebandsExists) {
			sidebands = row.sidebands;		
			sidebandsExists = true;
		}
		else
			sidebandsExists = false;
		
		}
		
		 fromBinMethods["antennaName"] = &CalDelayRow::antennaNameFromBin; 
		 fromBinMethods["atmPhaseCorrection"] = &CalDelayRow::atmPhaseCorrectionFromBin; 
		 fromBinMethods["basebandName"] = &CalDelayRow::basebandNameFromBin; 
		 fromBinMethods["receiverBand"] = &CalDelayRow::receiverBandFromBin; 
		 fromBinMethods["calDataId"] = &CalDelayRow::calDataIdFromBin; 
		 fromBinMethods["calReductionId"] = &CalDelayRow::calReductionIdFromBin; 
		 fromBinMethods["startValidTime"] = &CalDelayRow::startValidTimeFromBin; 
		 fromBinMethods["endValidTime"] = &CalDelayRow::endValidTimeFromBin; 
		 fromBinMethods["refAntennaName"] = &CalDelayRow::refAntennaNameFromBin; 
		 fromBinMethods["numReceptor"] = &CalDelayRow::numReceptorFromBin; 
		 fromBinMethods["delayError"] = &CalDelayRow::delayErrorFromBin; 
		 fromBinMethods["delayOffset"] = &CalDelayRow::delayOffsetFromBin; 
		 fromBinMethods["polarizationTypes"] = &CalDelayRow::polarizationTypesFromBin; 
		 fromBinMethods["reducedChiSquared"] = &CalDelayRow::reducedChiSquaredFromBin; 
			
	
		 fromBinMethods["crossDelayOffset"] = &CalDelayRow::crossDelayOffsetFromBin; 
		 fromBinMethods["crossDelayOffsetError"] = &CalDelayRow::crossDelayOffsetErrorFromBin; 
		 fromBinMethods["numSideband"] = &CalDelayRow::numSidebandFromBin; 
		 fromBinMethods["refFreq"] = &CalDelayRow::refFreqFromBin; 
		 fromBinMethods["refFreqPhase"] = &CalDelayRow::refFreqPhaseFromBin; 
		 fromBinMethods["sidebands"] = &CalDelayRow::sidebandsFromBin; 
			
	}

	
	bool CalDelayRow::compareNoAutoInc(string antennaName, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, BasebandNameMod::BasebandName basebandName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, string refAntennaName, int numReceptor, vector<double > delayError, vector<double > delayOffset, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > reducedChiSquared) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaName == antennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmPhaseCorrection == atmPhaseCorrection);
		
		if (!result) return false;
	

	
		
		result = result && (this->basebandName == basebandName);
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverBand == receiverBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->calDataId == calDataId);
		
		if (!result) return false;
	

	
		
		result = result && (this->calReductionId == calReductionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->startValidTime == startValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->endValidTime == endValidTime);
		
		if (!result) return false;
	

	
		
		result = result && (this->refAntennaName == refAntennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->delayError == delayError);
		
		if (!result) return false;
	

	
		
		result = result && (this->delayOffset == delayOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->reducedChiSquared == reducedChiSquared);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalDelayRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, string refAntennaName, int numReceptor, vector<double > delayError, vector<double > delayOffset, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > reducedChiSquared) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->refAntennaName == refAntennaName)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->delayError == delayError)) return false;
	

	
		if (!(this->delayOffset == delayOffset)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->reducedChiSquared == reducedChiSquared)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalDelayRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalDelayRow::equalByRequiredValue(CalDelayRow* x) {
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->refAntennaName != x->refAntennaName) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->delayError != x->delayError) return false;
			
		if (this->delayOffset != x->delayOffset) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->reducedChiSquared != x->reducedChiSquared) return false;
			
		
		return true;
	}	
	
/*
	 map<string, CalDelayAttributeFromBin> CalDelayRow::initFromBinMethods() {
		map<string, CalDelayAttributeFromBin> result;
		
		result["antennaName"] = &CalDelayRow::antennaNameFromBin;
		result["atmPhaseCorrection"] = &CalDelayRow::atmPhaseCorrectionFromBin;
		result["basebandName"] = &CalDelayRow::basebandNameFromBin;
		result["receiverBand"] = &CalDelayRow::receiverBandFromBin;
		result["calDataId"] = &CalDelayRow::calDataIdFromBin;
		result["calReductionId"] = &CalDelayRow::calReductionIdFromBin;
		result["startValidTime"] = &CalDelayRow::startValidTimeFromBin;
		result["endValidTime"] = &CalDelayRow::endValidTimeFromBin;
		result["refAntennaName"] = &CalDelayRow::refAntennaNameFromBin;
		result["numReceptor"] = &CalDelayRow::numReceptorFromBin;
		result["delayError"] = &CalDelayRow::delayErrorFromBin;
		result["delayOffset"] = &CalDelayRow::delayOffsetFromBin;
		result["polarizationTypes"] = &CalDelayRow::polarizationTypesFromBin;
		result["reducedChiSquared"] = &CalDelayRow::reducedChiSquaredFromBin;
		
		
		result["crossDelayOffset"] = &CalDelayRow::crossDelayOffsetFromBin;
		result["crossDelayOffsetError"] = &CalDelayRow::crossDelayOffsetErrorFromBin;
		result["numSideband"] = &CalDelayRow::numSidebandFromBin;
		result["refFreq"] = &CalDelayRow::refFreqFromBin;
		result["refFreqPhase"] = &CalDelayRow::refFreqPhaseFromBin;
		result["sidebands"] = &CalDelayRow::sidebandsFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
