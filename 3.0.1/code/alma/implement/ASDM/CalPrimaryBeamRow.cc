
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
 * File CalPrimaryBeamRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalPrimaryBeamRow.h>
#include <CalPrimaryBeamTable.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>

#include <CalDataTable.h>
#include <CalDataRow.h>
	

using asdm::ASDM;
using asdm::CalPrimaryBeamRow;
using asdm::CalPrimaryBeamTable;

using asdm::CalReductionTable;
using asdm::CalReductionRow;

using asdm::CalDataTable;
using asdm::CalDataRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	CalPrimaryBeamRow::~CalPrimaryBeamRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalPrimaryBeamTable &CalPrimaryBeamRow::getTable() const {
		return table;
	}
	
	void CalPrimaryBeamRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalPrimaryBeamRowIDL struct.
	 */
	CalPrimaryBeamRowIDL *CalPrimaryBeamRow::toIDL() const {
		CalPrimaryBeamRowIDL *x = new CalPrimaryBeamRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->antennaName = CORBA::string_dup(antennaName.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->antennaMake = antennaMake;
 				
 			
		
	

	
  		
		
		
			
		x->frequencyRange.length(frequencyRange.size());
		for (unsigned int i = 0; i < frequencyRange.size(); ++i) {
			
			x->frequencyRange[i] = frequencyRange.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->mainBeamEfficiency.length(mainBeamEfficiency.size());
		for (unsigned int i = 0; i < mainBeamEfficiency.size(); ++i) {
			
				
			x->mainBeamEfficiency[i] = mainBeamEfficiency.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->beamMapUID = beamMapUID.toIDLEntityRef();
			
		
	

	
  		
		
		
			
				
		x->relativeAmplitudeRms = relativeAmplitudeRms;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalPrimaryBeamRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalPrimaryBeamRow::setFromIDL (CalPrimaryBeamRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAntennaName(string (x.antennaName));
			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setAntennaMake(x.antennaMake);
  			
 		
		
	

	
		
		
			
		frequencyRange .clear();
		for (unsigned int i = 0; i <x.frequencyRange.length(); ++i) {
			
			frequencyRange.push_back(Frequency (x.frequencyRange[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		mainBeamEfficiency .clear();
		for (unsigned int i = 0; i <x.mainBeamEfficiency.length(); ++i) {
			
			mainBeamEfficiency.push_back(x.mainBeamEfficiency[i]);
  			
		}
			
  		
		
	

	
		
		
			
		setBeamMapUID(EntityRef (x.beamMapUID));
			
 		
		
	

	
		
		
			
		setRelativeAmplitudeRms(x.relativeAmplitudeRms);
  			
 		
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPrimaryBeam");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalPrimaryBeamRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(antennaName, "antennaName", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("antennaMake", antennaMake));
		
		
	

  	
 		
		
		Parser::toXML(frequencyRange, "frequencyRange", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(mainBeamEfficiency, "mainBeamEfficiency", buf);
		
		
	

  	
 		
		
		Parser::toXML(beamMapUID, "beamMapUID", buf);
		
		
	

  	
 		
		
		Parser::toXML(relativeAmplitudeRms, "relativeAmplitudeRms", buf);
		
		
	

	
	
		
  	
 		
		
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
	void CalPrimaryBeamRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAntennaName(Parser::getString("antennaName","CalPrimaryBeam",rowDoc));
			
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalPrimaryBeam",rowDoc);
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalPrimaryBeam",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalPrimaryBeam",rowDoc));
			
		
	

	
		
		
		
		antennaMake = EnumerationParser::getAntennaMake("antennaMake","CalPrimaryBeam",rowDoc);
		
		
		
	

	
  		
			
					
	  	setFrequencyRange(Parser::get1DFrequency("frequencyRange","CalPrimaryBeam",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalPrimaryBeam",rowDoc));
			
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalPrimaryBeam",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setMainBeamEfficiency(Parser::get1DDouble("mainBeamEfficiency","CalPrimaryBeam",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setBeamMapUID(Parser::getEntityRef("beamMapUID","CalPrimaryBeam",rowDoc));
			
		
	

	
  		
			
	  	setRelativeAmplitudeRms(Parser::getFloat("relativeAmplitudeRms","CalPrimaryBeam",rowDoc));
			
		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalPrimaryBeam");
		}
	}
	
	void CalPrimaryBeamRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeString(antennaName);
				
		
	

	
	
		
					
			eoss.writeInt(receiverBand);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
					
			eoss.writeInt(antennaMake);
				
		
	

	
	
		
	Frequency::toBin(frequencyRange, eoss);
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); i++)
				
			eoss.writeInt(polarizationTypes.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) mainBeamEfficiency.size());
		for (unsigned int i = 0; i < mainBeamEfficiency.size(); i++)
				
			eoss.writeDouble(mainBeamEfficiency.at(i));
				
				
						
		
	

	
	
		
	beamMapUID.toBin(eoss);
		
	

	
	
		
						
			eoss.writeFloat(relativeAmplitudeRms);
				
		
	


	
	
	}
	
void CalPrimaryBeamRow::antennaNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaName =  eiss.readString();
			
		
	
	
}
void CalPrimaryBeamRow::receiverBandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		receiverBand = CReceiverBand::from_int(eiss.readInt());
			
		
	
	
}
void CalPrimaryBeamRow::calDataIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calDataId =  Tag::fromBin(eiss);
		
	
	
}
void CalPrimaryBeamRow::calReductionIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calReductionId =  Tag::fromBin(eiss);
		
	
	
}
void CalPrimaryBeamRow::startValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		startValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalPrimaryBeamRow::endValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		endValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalPrimaryBeamRow::antennaMakeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		antennaMake = CAntennaMake::from_int(eiss.readInt());
			
		
	
	
}
void CalPrimaryBeamRow::frequencyRangeFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	frequencyRange = Frequency::from1DBin(eiss);	
	

		
	
	
}
void CalPrimaryBeamRow::numReceptorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	
	
}
void CalPrimaryBeamRow::polarizationTypesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		polarizationTypes.clear();
		
		unsigned int polarizationTypesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypesDim1; i++)
			
			polarizationTypes.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	
	
}
void CalPrimaryBeamRow::mainBeamEfficiencyFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		mainBeamEfficiency.clear();
		
		unsigned int mainBeamEfficiencyDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < mainBeamEfficiencyDim1; i++)
			
			mainBeamEfficiency.push_back(eiss.readDouble());
			
	

		
	
	
}
void CalPrimaryBeamRow::beamMapUIDFromBin(EndianISStream& eiss) {
		
	
		
		
		beamMapUID =  EntityRef::fromBin(eiss);
		
	
	
}
void CalPrimaryBeamRow::relativeAmplitudeRmsFromBin(EndianISStream& eiss) {
		
	
	
		
			
		relativeAmplitudeRms =  eiss.readFloat();
			
		
	
	
}

		
	
	CalPrimaryBeamRow* CalPrimaryBeamRow::fromBin(EndianISStream& eiss, CalPrimaryBeamTable& table, const vector<string>& attributesSeq) {
		CalPrimaryBeamRow* row = new  CalPrimaryBeamRow(table);
		
		map<string, CalPrimaryBeamAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "CalPrimaryBeamTable");
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
 	string CalPrimaryBeamRow::getAntennaName() const {
	
  		return antennaName;
 	}

 	/**
 	 * Set antennaName with the specified string.
 	 * @param antennaName The string value to which antennaName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPrimaryBeamRow::setAntennaName (string antennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaName", "CalPrimaryBeam");
		
  		}
  	
 		this->antennaName = antennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalPrimaryBeamRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPrimaryBeamRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalPrimaryBeam");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalPrimaryBeamRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalPrimaryBeamRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaMake.
 	 * @return antennaMake as AntennaMakeMod::AntennaMake
 	 */
 	AntennaMakeMod::AntennaMake CalPrimaryBeamRow::getAntennaMake() const {
	
  		return antennaMake;
 	}

 	/**
 	 * Set antennaMake with the specified AntennaMakeMod::AntennaMake.
 	 * @param antennaMake The AntennaMakeMod::AntennaMake value to which antennaMake is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setAntennaMake (AntennaMakeMod::AntennaMake antennaMake)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaMake = antennaMake;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyRange.
 	 * @return frequencyRange as vector<Frequency >
 	 */
 	vector<Frequency > CalPrimaryBeamRow::getFrequencyRange() const {
	
  		return frequencyRange;
 	}

 	/**
 	 * Set frequencyRange with the specified vector<Frequency >.
 	 * @param frequencyRange The vector<Frequency > value to which frequencyRange is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setFrequencyRange (vector<Frequency > frequencyRange)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyRange = frequencyRange;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalPrimaryBeamRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalPrimaryBeamRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get mainBeamEfficiency.
 	 * @return mainBeamEfficiency as vector<double >
 	 */
 	vector<double > CalPrimaryBeamRow::getMainBeamEfficiency() const {
	
  		return mainBeamEfficiency;
 	}

 	/**
 	 * Set mainBeamEfficiency with the specified vector<double >.
 	 * @param mainBeamEfficiency The vector<double > value to which mainBeamEfficiency is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setMainBeamEfficiency (vector<double > mainBeamEfficiency)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->mainBeamEfficiency = mainBeamEfficiency;
	
 	}
	
	

	

	
 	/**
 	 * Get beamMapUID.
 	 * @return beamMapUID as EntityRef
 	 */
 	EntityRef CalPrimaryBeamRow::getBeamMapUID() const {
	
  		return beamMapUID;
 	}

 	/**
 	 * Set beamMapUID with the specified EntityRef.
 	 * @param beamMapUID The EntityRef value to which beamMapUID is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setBeamMapUID (EntityRef beamMapUID)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->beamMapUID = beamMapUID;
	
 	}
	
	

	

	
 	/**
 	 * Get relativeAmplitudeRms.
 	 * @return relativeAmplitudeRms as float
 	 */
 	float CalPrimaryBeamRow::getRelativeAmplitudeRms() const {
	
  		return relativeAmplitudeRms;
 	}

 	/**
 	 * Set relativeAmplitudeRms with the specified float.
 	 * @param relativeAmplitudeRms The float value to which relativeAmplitudeRms is to be set.
 	 
 	
 		
 	 */
 	void CalPrimaryBeamRow::setRelativeAmplitudeRms (float relativeAmplitudeRms)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->relativeAmplitudeRms = relativeAmplitudeRms;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalPrimaryBeamRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPrimaryBeamRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalPrimaryBeam");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalPrimaryBeamRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalPrimaryBeamRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalPrimaryBeam");
		
  		}
  	
 		this->calReductionId = calReductionId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the CalReduction table having CalReduction.calReductionId == calReductionId
	 * @return a CalReductionRow*
	 * 
	 
	 */
	 CalReductionRow* CalPrimaryBeamRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalPrimaryBeamRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	/**
	 * Create a CalPrimaryBeamRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalPrimaryBeamRow::CalPrimaryBeamRow (CalPrimaryBeamTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	
	

	

	
	
	
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
antennaMake = CAntennaMake::from_int(0);
	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaName"] = &CalPrimaryBeamRow::antennaNameFromBin; 
	 fromBinMethods["receiverBand"] = &CalPrimaryBeamRow::receiverBandFromBin; 
	 fromBinMethods["calDataId"] = &CalPrimaryBeamRow::calDataIdFromBin; 
	 fromBinMethods["calReductionId"] = &CalPrimaryBeamRow::calReductionIdFromBin; 
	 fromBinMethods["startValidTime"] = &CalPrimaryBeamRow::startValidTimeFromBin; 
	 fromBinMethods["endValidTime"] = &CalPrimaryBeamRow::endValidTimeFromBin; 
	 fromBinMethods["antennaMake"] = &CalPrimaryBeamRow::antennaMakeFromBin; 
	 fromBinMethods["frequencyRange"] = &CalPrimaryBeamRow::frequencyRangeFromBin; 
	 fromBinMethods["numReceptor"] = &CalPrimaryBeamRow::numReceptorFromBin; 
	 fromBinMethods["polarizationTypes"] = &CalPrimaryBeamRow::polarizationTypesFromBin; 
	 fromBinMethods["mainBeamEfficiency"] = &CalPrimaryBeamRow::mainBeamEfficiencyFromBin; 
	 fromBinMethods["beamMapUID"] = &CalPrimaryBeamRow::beamMapUIDFromBin; 
	 fromBinMethods["relativeAmplitudeRms"] = &CalPrimaryBeamRow::relativeAmplitudeRmsFromBin; 
		
	
	
	}
	
	CalPrimaryBeamRow::CalPrimaryBeamRow (CalPrimaryBeamTable &t, CalPrimaryBeamRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	
	

	
		
		}
		else {
	
		
			antennaName = row.antennaName;
		
			receiverBand = row.receiverBand;
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			antennaMake = row.antennaMake;
		
			frequencyRange = row.frequencyRange;
		
			numReceptor = row.numReceptor;
		
			polarizationTypes = row.polarizationTypes;
		
			mainBeamEfficiency = row.mainBeamEfficiency;
		
			beamMapUID = row.beamMapUID;
		
			relativeAmplitudeRms = row.relativeAmplitudeRms;
		
		
		
		
		}
		
		 fromBinMethods["antennaName"] = &CalPrimaryBeamRow::antennaNameFromBin; 
		 fromBinMethods["receiverBand"] = &CalPrimaryBeamRow::receiverBandFromBin; 
		 fromBinMethods["calDataId"] = &CalPrimaryBeamRow::calDataIdFromBin; 
		 fromBinMethods["calReductionId"] = &CalPrimaryBeamRow::calReductionIdFromBin; 
		 fromBinMethods["startValidTime"] = &CalPrimaryBeamRow::startValidTimeFromBin; 
		 fromBinMethods["endValidTime"] = &CalPrimaryBeamRow::endValidTimeFromBin; 
		 fromBinMethods["antennaMake"] = &CalPrimaryBeamRow::antennaMakeFromBin; 
		 fromBinMethods["frequencyRange"] = &CalPrimaryBeamRow::frequencyRangeFromBin; 
		 fromBinMethods["numReceptor"] = &CalPrimaryBeamRow::numReceptorFromBin; 
		 fromBinMethods["polarizationTypes"] = &CalPrimaryBeamRow::polarizationTypesFromBin; 
		 fromBinMethods["mainBeamEfficiency"] = &CalPrimaryBeamRow::mainBeamEfficiencyFromBin; 
		 fromBinMethods["beamMapUID"] = &CalPrimaryBeamRow::beamMapUIDFromBin; 
		 fromBinMethods["relativeAmplitudeRms"] = &CalPrimaryBeamRow::relativeAmplitudeRmsFromBin; 
			
	
			
	}

	
	bool CalPrimaryBeamRow::compareNoAutoInc(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, AntennaMakeMod::AntennaMake antennaMake, vector<Frequency > frequencyRange, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > mainBeamEfficiency, EntityRef beamMapUID, float relativeAmplitudeRms) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaName == antennaName);
		
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
	

	
		
		result = result && (this->antennaMake == antennaMake);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyRange == frequencyRange);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->mainBeamEfficiency == mainBeamEfficiency);
		
		if (!result) return false;
	

	
		
		result = result && (this->beamMapUID == beamMapUID);
		
		if (!result) return false;
	

	
		
		result = result && (this->relativeAmplitudeRms == relativeAmplitudeRms);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalPrimaryBeamRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, AntennaMakeMod::AntennaMake antennaMake, vector<Frequency > frequencyRange, int numReceptor, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<double > mainBeamEfficiency, EntityRef beamMapUID, float relativeAmplitudeRms) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->antennaMake == antennaMake)) return false;
	

	
		if (!(this->frequencyRange == frequencyRange)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->mainBeamEfficiency == mainBeamEfficiency)) return false;
	

	
		if (!(this->beamMapUID == beamMapUID)) return false;
	

	
		if (!(this->relativeAmplitudeRms == relativeAmplitudeRms)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalPrimaryBeamRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalPrimaryBeamRow::equalByRequiredValue(CalPrimaryBeamRow* x) {
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->antennaMake != x->antennaMake) return false;
			
		if (this->frequencyRange != x->frequencyRange) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->mainBeamEfficiency != x->mainBeamEfficiency) return false;
			
		if (this->beamMapUID != x->beamMapUID) return false;
			
		if (this->relativeAmplitudeRms != x->relativeAmplitudeRms) return false;
			
		
		return true;
	}	
	
/*
	 map<string, CalPrimaryBeamAttributeFromBin> CalPrimaryBeamRow::initFromBinMethods() {
		map<string, CalPrimaryBeamAttributeFromBin> result;
		
		result["antennaName"] = &CalPrimaryBeamRow::antennaNameFromBin;
		result["receiverBand"] = &CalPrimaryBeamRow::receiverBandFromBin;
		result["calDataId"] = &CalPrimaryBeamRow::calDataIdFromBin;
		result["calReductionId"] = &CalPrimaryBeamRow::calReductionIdFromBin;
		result["startValidTime"] = &CalPrimaryBeamRow::startValidTimeFromBin;
		result["endValidTime"] = &CalPrimaryBeamRow::endValidTimeFromBin;
		result["antennaMake"] = &CalPrimaryBeamRow::antennaMakeFromBin;
		result["frequencyRange"] = &CalPrimaryBeamRow::frequencyRangeFromBin;
		result["numReceptor"] = &CalPrimaryBeamRow::numReceptorFromBin;
		result["polarizationTypes"] = &CalPrimaryBeamRow::polarizationTypesFromBin;
		result["mainBeamEfficiency"] = &CalPrimaryBeamRow::mainBeamEfficiencyFromBin;
		result["beamMapUID"] = &CalPrimaryBeamRow::beamMapUIDFromBin;
		result["relativeAmplitudeRms"] = &CalPrimaryBeamRow::relativeAmplitudeRmsFromBin;
		
		
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
