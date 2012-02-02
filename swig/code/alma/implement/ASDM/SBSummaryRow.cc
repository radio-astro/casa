
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
 * File SBSummaryRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <SBSummaryRow.h>
#include <SBSummaryTable.h>
	

using asdm::ASDM;
using asdm::SBSummaryRow;
using asdm::SBSummaryTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	SBSummaryRow::~SBSummaryRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	SBSummaryTable &SBSummaryRow::getTable() const {
		return table;
	}

	bool SBSummaryRow::isAdded() const {
		return hasBeenAdded;
	}	

	void SBSummaryRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SBSummaryRowIDL struct.
	 */
	SBSummaryRowIDL *SBSummaryRow::toIDL() const {
		SBSummaryRowIDL *x = new SBSummaryRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->sBSummaryId = sBSummaryId.toIDLTag();
			
		
	

	
  		
		
		
			
		x->sbSummaryUID = sbSummaryUID.toIDLEntityRef();
			
		
	

	
  		
		
		
			
		x->projectUID = projectUID.toIDLEntityRef();
			
		
	

	
  		
		
		
			
		x->obsUnitSetId = obsUnitSetId.toIDLEntityRef();
			
		
	

	
  		
		
		
			
				
		x->frequency = frequency;
 				
 			
		
	

	
  		
		
		
			
				
		x->frequencyBand = frequencyBand;
 				
 			
		
	

	
  		
		
		
			
				
		x->sbType = sbType;
 				
 			
		
	

	
  		
		
		
			
		x->sbDuration = sbDuration.toIDLInterval();
			
		
	

	
  		
		
		
			
		x->centerDirection.length(centerDirection.size());
		for (unsigned int i = 0; i < centerDirection.size(); ++i) {
			
			x->centerDirection[i] = centerDirection.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->numObservingMode = numObservingMode;
 				
 			
		
	

	
  		
		
		
			
		x->observingMode.length(observingMode.size());
		for (unsigned int i = 0; i < observingMode.size(); ++i) {
			
				
			x->observingMode[i] = CORBA::string_dup(observingMode.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->numberRepeats = numberRepeats;
 				
 			
		
	

	
  		
		
		
			
				
		x->numScienceGoal = numScienceGoal;
 				
 			
		
	

	
  		
		
		
			
		x->scienceGoal.length(scienceGoal.size());
		for (unsigned int i = 0; i < scienceGoal.size(); ++i) {
			
				
			x->scienceGoal[i] = CORBA::string_dup(scienceGoal.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->numWeatherConstraint = numWeatherConstraint;
 				
 			
		
	

	
  		
		
		
			
		x->weatherConstraint.length(weatherConstraint.size());
		for (unsigned int i = 0; i < weatherConstraint.size(); ++i) {
			
				
			x->weatherConstraint[i] = CORBA::string_dup(weatherConstraint.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		x->centerDirectionCodeExists = centerDirectionCodeExists;
		
		
			
				
		x->centerDirectionCode = centerDirectionCode;
 				
 			
		
	

	
  		
		
		x->centerDirectionEquinoxExists = centerDirectionEquinoxExists;
		
		
			
		x->centerDirectionEquinox = centerDirectionEquinox.toIDLArrayTime();
			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SBSummaryRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void SBSummaryRow::setFromIDL (SBSummaryRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setSBSummaryId(Tag (x.sBSummaryId));
			
 		
		
	

	
		
		
			
		setSbSummaryUID(EntityRef (x.sbSummaryUID));
			
 		
		
	

	
		
		
			
		setProjectUID(EntityRef (x.projectUID));
			
 		
		
	

	
		
		
			
		setObsUnitSetId(EntityRef (x.obsUnitSetId));
			
 		
		
	

	
		
		
			
		setFrequency(x.frequency);
  			
 		
		
	

	
		
		
			
		setFrequencyBand(x.frequencyBand);
  			
 		
		
	

	
		
		
			
		setSbType(x.sbType);
  			
 		
		
	

	
		
		
			
		setSbDuration(Interval (x.sbDuration));
			
 		
		
	

	
		
		
			
		centerDirection .clear();
		for (unsigned int i = 0; i <x.centerDirection.length(); ++i) {
			
			centerDirection.push_back(Angle (x.centerDirection[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumObservingMode(x.numObservingMode);
  			
 		
		
	

	
		
		
			
		observingMode .clear();
		for (unsigned int i = 0; i <x.observingMode.length(); ++i) {
			
			observingMode.push_back(string (x.observingMode[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumberRepeats(x.numberRepeats);
  			
 		
		
	

	
		
		
			
		setNumScienceGoal(x.numScienceGoal);
  			
 		
		
	

	
		
		
			
		scienceGoal .clear();
		for (unsigned int i = 0; i <x.scienceGoal.length(); ++i) {
			
			scienceGoal.push_back(string (x.scienceGoal[i]));
			
		}
			
  		
		
	

	
		
		
			
		setNumWeatherConstraint(x.numWeatherConstraint);
  			
 		
		
	

	
		
		
			
		weatherConstraint .clear();
		for (unsigned int i = 0; i <x.weatherConstraint.length(); ++i) {
			
			weatherConstraint.push_back(string (x.weatherConstraint[i]));
			
		}
			
  		
		
	

	
		
		centerDirectionCodeExists = x.centerDirectionCodeExists;
		if (x.centerDirectionCodeExists) {
		
		
			
		setCenterDirectionCode(x.centerDirectionCode);
  			
 		
		
		}
		
	

	
		
		centerDirectionEquinoxExists = x.centerDirectionEquinoxExists;
		if (x.centerDirectionEquinoxExists) {
		
		
			
		setCenterDirectionEquinox(ArrayTime (x.centerDirectionEquinox));
			
 		
		
		}
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SBSummary");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string SBSummaryRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(sBSummaryId, "sBSummaryId", buf);
		
		
	

  	
 		
		
		Parser::toXML(sbSummaryUID, "sbSummaryUID", buf);
		
		
	

  	
 		
		
		Parser::toXML(projectUID, "projectUID", buf);
		
		
	

  	
 		
		
		Parser::toXML(obsUnitSetId, "obsUnitSetId", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequency, "frequency", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("frequencyBand", frequencyBand));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("sbType", sbType));
		
		
	

  	
 		
		
		Parser::toXML(sbDuration, "sbDuration", buf);
		
		
	

  	
 		
		
		Parser::toXML(centerDirection, "centerDirection", buf);
		
		
	

  	
 		
		
		Parser::toXML(numObservingMode, "numObservingMode", buf);
		
		
	

  	
 		
		
		Parser::toXML(observingMode, "observingMode", buf);
		
		
	

  	
 		
		
		Parser::toXML(numberRepeats, "numberRepeats", buf);
		
		
	

  	
 		
		
		Parser::toXML(numScienceGoal, "numScienceGoal", buf);
		
		
	

  	
 		
		
		Parser::toXML(scienceGoal, "scienceGoal", buf);
		
		
	

  	
 		
		
		Parser::toXML(numWeatherConstraint, "numWeatherConstraint", buf);
		
		
	

  	
 		
		
		Parser::toXML(weatherConstraint, "weatherConstraint", buf);
		
		
	

  	
 		
		if (centerDirectionCodeExists) {
		
		
			buf.append(EnumerationParser::toXML("centerDirectionCode", centerDirectionCode));
		
		
		}
		
	

  	
 		
		if (centerDirectionEquinoxExists) {
		
		
		Parser::toXML(centerDirectionEquinox, "centerDirectionEquinox", buf);
		
		
		}
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void SBSummaryRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setSBSummaryId(Parser::getTag("sBSummaryId","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setSbSummaryUID(Parser::getEntityRef("sbSummaryUID","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setProjectUID(Parser::getEntityRef("projectUID","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setObsUnitSetId(Parser::getEntityRef("obsUnitSetId","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setFrequency(Parser::getDouble("frequency","SBSummary",rowDoc));
			
		
	

	
		
		
		
		frequencyBand = EnumerationParser::getReceiverBand("frequencyBand","SBSummary",rowDoc);
		
		
		
	

	
		
		
		
		sbType = EnumerationParser::getSBType("sbType","SBSummary",rowDoc);
		
		
		
	

	
  		
			
	  	setSbDuration(Parser::getInterval("sbDuration","SBSummary",rowDoc));
			
		
	

	
  		
			
					
	  	setCenterDirection(Parser::get1DAngle("centerDirection","SBSummary",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumObservingMode(Parser::getInteger("numObservingMode","SBSummary",rowDoc));
			
		
	

	
  		
			
					
	  	setObservingMode(Parser::get1DString("observingMode","SBSummary",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumberRepeats(Parser::getInteger("numberRepeats","SBSummary",rowDoc));
			
		
	

	
  		
			
	  	setNumScienceGoal(Parser::getInteger("numScienceGoal","SBSummary",rowDoc));
			
		
	

	
  		
			
					
	  	setScienceGoal(Parser::get1DString("scienceGoal","SBSummary",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setNumWeatherConstraint(Parser::getInteger("numWeatherConstraint","SBSummary",rowDoc));
			
		
	

	
  		
			
					
	  	setWeatherConstraint(Parser::get1DString("weatherConstraint","SBSummary",rowDoc));
	  			
	  		
		
	

	
		
	if (row.isStr("<centerDirectionCode>")) {
		
		
		
		centerDirectionCode = EnumerationParser::getDirectionReferenceCode("centerDirectionCode","SBSummary",rowDoc);
		
		
		
		centerDirectionCodeExists = true;
	}
		
	

	
  		
        if (row.isStr("<centerDirectionEquinox>")) {
			
	  		setCenterDirectionEquinox(Parser::getArrayTime("centerDirectionEquinox","SBSummary",rowDoc));
			
		}
 		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SBSummary");
		}
	}
	
	void SBSummaryRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	sBSummaryId.toBin(eoss);
		
	

	
	
		
	sbSummaryUID.toBin(eoss);
		
	

	
	
		
	projectUID.toBin(eoss);
		
	

	
	
		
	obsUnitSetId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeDouble(frequency);
				
		
	

	
	
		
					
			eoss.writeInt(frequencyBand);
				
		
	

	
	
		
					
			eoss.writeInt(sbType);
				
		
	

	
	
		
	sbDuration.toBin(eoss);
		
	

	
	
		
	Angle::toBin(centerDirection, eoss);
		
	

	
	
		
						
			eoss.writeInt(numObservingMode);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) observingMode.size());
		for (unsigned int i = 0; i < observingMode.size(); i++)
				
			eoss.writeString(observingMode.at(i));
				
				
						
		
	

	
	
		
						
			eoss.writeInt(numberRepeats);
				
		
	

	
	
		
						
			eoss.writeInt(numScienceGoal);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) scienceGoal.size());
		for (unsigned int i = 0; i < scienceGoal.size(); i++)
				
			eoss.writeString(scienceGoal.at(i));
				
				
						
		
	

	
	
		
						
			eoss.writeInt(numWeatherConstraint);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) weatherConstraint.size());
		for (unsigned int i = 0; i < weatherConstraint.size(); i++)
				
			eoss.writeString(weatherConstraint.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(centerDirectionCodeExists);
	if (centerDirectionCodeExists) {
	
	
	
		
					
			eoss.writeInt(centerDirectionCode);
				
		
	

	}

	eoss.writeBoolean(centerDirectionEquinoxExists);
	if (centerDirectionEquinoxExists) {
	
	
	
		
	centerDirectionEquinox.toBin(eoss);
		
	

	}

	}
	
void SBSummaryRow::sBSummaryIdFromBin(EndianISStream& eiss) {
		
	
		
		
		sBSummaryId =  Tag::fromBin(eiss);
		
	
	
}
void SBSummaryRow::sbSummaryUIDFromBin(EndianISStream& eiss) {
		
	
		
		
		sbSummaryUID =  EntityRef::fromBin(eiss);
		
	
	
}
void SBSummaryRow::projectUIDFromBin(EndianISStream& eiss) {
		
	
		
		
		projectUID =  EntityRef::fromBin(eiss);
		
	
	
}
void SBSummaryRow::obsUnitSetIdFromBin(EndianISStream& eiss) {
		
	
		
		
		obsUnitSetId =  EntityRef::fromBin(eiss);
		
	
	
}
void SBSummaryRow::frequencyFromBin(EndianISStream& eiss) {
		
	
	
		
			
		frequency =  eiss.readDouble();
			
		
	
	
}
void SBSummaryRow::frequencyBandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		frequencyBand = CReceiverBand::from_int(eiss.readInt());
			
		
	
	
}
void SBSummaryRow::sbTypeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		sbType = CSBType::from_int(eiss.readInt());
			
		
	
	
}
void SBSummaryRow::sbDurationFromBin(EndianISStream& eiss) {
		
	
		
		
		sbDuration =  Interval::fromBin(eiss);
		
	
	
}
void SBSummaryRow::centerDirectionFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	centerDirection = Angle::from1DBin(eiss);	
	

		
	
	
}
void SBSummaryRow::numObservingModeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numObservingMode =  eiss.readInt();
			
		
	
	
}
void SBSummaryRow::observingModeFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		observingMode.clear();
		
		unsigned int observingModeDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < observingModeDim1; i++)
			
			observingMode.push_back(eiss.readString());
			
	

		
	
	
}
void SBSummaryRow::numberRepeatsFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numberRepeats =  eiss.readInt();
			
		
	
	
}
void SBSummaryRow::numScienceGoalFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numScienceGoal =  eiss.readInt();
			
		
	
	
}
void SBSummaryRow::scienceGoalFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		scienceGoal.clear();
		
		unsigned int scienceGoalDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < scienceGoalDim1; i++)
			
			scienceGoal.push_back(eiss.readString());
			
	

		
	
	
}
void SBSummaryRow::numWeatherConstraintFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numWeatherConstraint =  eiss.readInt();
			
		
	
	
}
void SBSummaryRow::weatherConstraintFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		weatherConstraint.clear();
		
		unsigned int weatherConstraintDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < weatherConstraintDim1; i++)
			
			weatherConstraint.push_back(eiss.readString());
			
	

		
	
	
}

void SBSummaryRow::centerDirectionCodeFromBin(EndianISStream& eiss) {
		
	centerDirectionCodeExists = eiss.readBoolean();
	if (centerDirectionCodeExists) {
		
	
	
		
			
		centerDirectionCode = CDirectionReferenceCode::from_int(eiss.readInt());
			
		
	

	}
	
}
void SBSummaryRow::centerDirectionEquinoxFromBin(EndianISStream& eiss) {
		
	centerDirectionEquinoxExists = eiss.readBoolean();
	if (centerDirectionEquinoxExists) {
		
	
		
		
		centerDirectionEquinox =  ArrayTime::fromBin(eiss);
		
	

	}
	
}
	
	
	SBSummaryRow* SBSummaryRow::fromBin(EndianISStream& eiss, SBSummaryTable& table, const vector<string>& attributesSeq) {
		SBSummaryRow* row = new  SBSummaryRow(table);
		
		map<string, SBSummaryAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "SBSummaryTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get sBSummaryId.
 	 * @return sBSummaryId as Tag
 	 */
 	Tag SBSummaryRow::getSBSummaryId() const {
	
  		return sBSummaryId;
 	}

 	/**
 	 * Set sBSummaryId with the specified Tag.
 	 * @param sBSummaryId The Tag value to which sBSummaryId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SBSummaryRow::setSBSummaryId (Tag sBSummaryId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("sBSummaryId", "SBSummary");
		
  		}
  	
 		this->sBSummaryId = sBSummaryId;
	
 	}
	
	

	

	
 	/**
 	 * Get sbSummaryUID.
 	 * @return sbSummaryUID as EntityRef
 	 */
 	EntityRef SBSummaryRow::getSbSummaryUID() const {
	
  		return sbSummaryUID;
 	}

 	/**
 	 * Set sbSummaryUID with the specified EntityRef.
 	 * @param sbSummaryUID The EntityRef value to which sbSummaryUID is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setSbSummaryUID (EntityRef sbSummaryUID)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sbSummaryUID = sbSummaryUID;
	
 	}
	
	

	

	
 	/**
 	 * Get projectUID.
 	 * @return projectUID as EntityRef
 	 */
 	EntityRef SBSummaryRow::getProjectUID() const {
	
  		return projectUID;
 	}

 	/**
 	 * Set projectUID with the specified EntityRef.
 	 * @param projectUID The EntityRef value to which projectUID is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setProjectUID (EntityRef projectUID)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->projectUID = projectUID;
	
 	}
	
	

	

	
 	/**
 	 * Get obsUnitSetId.
 	 * @return obsUnitSetId as EntityRef
 	 */
 	EntityRef SBSummaryRow::getObsUnitSetId() const {
	
  		return obsUnitSetId;
 	}

 	/**
 	 * Set obsUnitSetId with the specified EntityRef.
 	 * @param obsUnitSetId The EntityRef value to which obsUnitSetId is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setObsUnitSetId (EntityRef obsUnitSetId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->obsUnitSetId = obsUnitSetId;
	
 	}
	
	

	

	
 	/**
 	 * Get frequency.
 	 * @return frequency as double
 	 */
 	double SBSummaryRow::getFrequency() const {
	
  		return frequency;
 	}

 	/**
 	 * Set frequency with the specified double.
 	 * @param frequency The double value to which frequency is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setFrequency (double frequency)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequency = frequency;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyBand.
 	 * @return frequencyBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand SBSummaryRow::getFrequencyBand() const {
	
  		return frequencyBand;
 	}

 	/**
 	 * Set frequencyBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param frequencyBand The ReceiverBandMod::ReceiverBand value to which frequencyBand is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setFrequencyBand (ReceiverBandMod::ReceiverBand frequencyBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyBand = frequencyBand;
	
 	}
	
	

	

	
 	/**
 	 * Get sbType.
 	 * @return sbType as SBTypeMod::SBType
 	 */
 	SBTypeMod::SBType SBSummaryRow::getSbType() const {
	
  		return sbType;
 	}

 	/**
 	 * Set sbType with the specified SBTypeMod::SBType.
 	 * @param sbType The SBTypeMod::SBType value to which sbType is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setSbType (SBTypeMod::SBType sbType)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sbType = sbType;
	
 	}
	
	

	

	
 	/**
 	 * Get sbDuration.
 	 * @return sbDuration as Interval
 	 */
 	Interval SBSummaryRow::getSbDuration() const {
	
  		return sbDuration;
 	}

 	/**
 	 * Set sbDuration with the specified Interval.
 	 * @param sbDuration The Interval value to which sbDuration is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setSbDuration (Interval sbDuration)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sbDuration = sbDuration;
	
 	}
	
	

	

	
 	/**
 	 * Get centerDirection.
 	 * @return centerDirection as vector<Angle >
 	 */
 	vector<Angle > SBSummaryRow::getCenterDirection() const {
	
  		return centerDirection;
 	}

 	/**
 	 * Set centerDirection with the specified vector<Angle >.
 	 * @param centerDirection The vector<Angle > value to which centerDirection is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setCenterDirection (vector<Angle > centerDirection)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->centerDirection = centerDirection;
	
 	}
	
	

	

	
 	/**
 	 * Get numObservingMode.
 	 * @return numObservingMode as int
 	 */
 	int SBSummaryRow::getNumObservingMode() const {
	
  		return numObservingMode;
 	}

 	/**
 	 * Set numObservingMode with the specified int.
 	 * @param numObservingMode The int value to which numObservingMode is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setNumObservingMode (int numObservingMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numObservingMode = numObservingMode;
	
 	}
	
	

	

	
 	/**
 	 * Get observingMode.
 	 * @return observingMode as vector<string >
 	 */
 	vector<string > SBSummaryRow::getObservingMode() const {
	
  		return observingMode;
 	}

 	/**
 	 * Set observingMode with the specified vector<string >.
 	 * @param observingMode The vector<string > value to which observingMode is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setObservingMode (vector<string > observingMode)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->observingMode = observingMode;
	
 	}
	
	

	

	
 	/**
 	 * Get numberRepeats.
 	 * @return numberRepeats as int
 	 */
 	int SBSummaryRow::getNumberRepeats() const {
	
  		return numberRepeats;
 	}

 	/**
 	 * Set numberRepeats with the specified int.
 	 * @param numberRepeats The int value to which numberRepeats is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setNumberRepeats (int numberRepeats)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numberRepeats = numberRepeats;
	
 	}
	
	

	

	
 	/**
 	 * Get numScienceGoal.
 	 * @return numScienceGoal as int
 	 */
 	int SBSummaryRow::getNumScienceGoal() const {
	
  		return numScienceGoal;
 	}

 	/**
 	 * Set numScienceGoal with the specified int.
 	 * @param numScienceGoal The int value to which numScienceGoal is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setNumScienceGoal (int numScienceGoal)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numScienceGoal = numScienceGoal;
	
 	}
	
	

	

	
 	/**
 	 * Get scienceGoal.
 	 * @return scienceGoal as vector<string >
 	 */
 	vector<string > SBSummaryRow::getScienceGoal() const {
	
  		return scienceGoal;
 	}

 	/**
 	 * Set scienceGoal with the specified vector<string >.
 	 * @param scienceGoal The vector<string > value to which scienceGoal is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setScienceGoal (vector<string > scienceGoal)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->scienceGoal = scienceGoal;
	
 	}
	
	

	

	
 	/**
 	 * Get numWeatherConstraint.
 	 * @return numWeatherConstraint as int
 	 */
 	int SBSummaryRow::getNumWeatherConstraint() const {
	
  		return numWeatherConstraint;
 	}

 	/**
 	 * Set numWeatherConstraint with the specified int.
 	 * @param numWeatherConstraint The int value to which numWeatherConstraint is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setNumWeatherConstraint (int numWeatherConstraint)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numWeatherConstraint = numWeatherConstraint;
	
 	}
	
	

	

	
 	/**
 	 * Get weatherConstraint.
 	 * @return weatherConstraint as vector<string >
 	 */
 	vector<string > SBSummaryRow::getWeatherConstraint() const {
	
  		return weatherConstraint;
 	}

 	/**
 	 * Set weatherConstraint with the specified vector<string >.
 	 * @param weatherConstraint The vector<string > value to which weatherConstraint is to be set.
 	 
 	
 		
 	 */
 	void SBSummaryRow::setWeatherConstraint (vector<string > weatherConstraint)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->weatherConstraint = weatherConstraint;
	
 	}
	
	

	
	/**
	 * The attribute centerDirectionCode is optional. Return true if this attribute exists.
	 * @return true if and only if the centerDirectionCode attribute exists. 
	 */
	bool SBSummaryRow::isCenterDirectionCodeExists() const {
		return centerDirectionCodeExists;
	}
	

	
 	/**
 	 * Get centerDirectionCode, which is optional.
 	 * @return centerDirectionCode as DirectionReferenceCodeMod::DirectionReferenceCode
 	 * @throw IllegalAccessException If centerDirectionCode does not exist.
 	 */
 	DirectionReferenceCodeMod::DirectionReferenceCode SBSummaryRow::getCenterDirectionCode() const  {
		if (!centerDirectionCodeExists) {
			throw IllegalAccessException("centerDirectionCode", "SBSummary");
		}
	
  		return centerDirectionCode;
 	}

 	/**
 	 * Set centerDirectionCode with the specified DirectionReferenceCodeMod::DirectionReferenceCode.
 	 * @param centerDirectionCode The DirectionReferenceCodeMod::DirectionReferenceCode value to which centerDirectionCode is to be set.
 	 
 	
 	 */
 	void SBSummaryRow::setCenterDirectionCode (DirectionReferenceCodeMod::DirectionReferenceCode centerDirectionCode) {
	
 		this->centerDirectionCode = centerDirectionCode;
	
		centerDirectionCodeExists = true;
	
 	}
	
	
	/**
	 * Mark centerDirectionCode, which is an optional field, as non-existent.
	 */
	void SBSummaryRow::clearCenterDirectionCode () {
		centerDirectionCodeExists = false;
	}
	

	
	/**
	 * The attribute centerDirectionEquinox is optional. Return true if this attribute exists.
	 * @return true if and only if the centerDirectionEquinox attribute exists. 
	 */
	bool SBSummaryRow::isCenterDirectionEquinoxExists() const {
		return centerDirectionEquinoxExists;
	}
	

	
 	/**
 	 * Get centerDirectionEquinox, which is optional.
 	 * @return centerDirectionEquinox as ArrayTime
 	 * @throw IllegalAccessException If centerDirectionEquinox does not exist.
 	 */
 	ArrayTime SBSummaryRow::getCenterDirectionEquinox() const  {
		if (!centerDirectionEquinoxExists) {
			throw IllegalAccessException("centerDirectionEquinox", "SBSummary");
		}
	
  		return centerDirectionEquinox;
 	}

 	/**
 	 * Set centerDirectionEquinox with the specified ArrayTime.
 	 * @param centerDirectionEquinox The ArrayTime value to which centerDirectionEquinox is to be set.
 	 
 	
 	 */
 	void SBSummaryRow::setCenterDirectionEquinox (ArrayTime centerDirectionEquinox) {
	
 		this->centerDirectionEquinox = centerDirectionEquinox;
	
		centerDirectionEquinoxExists = true;
	
 	}
	
	
	/**
	 * Mark centerDirectionEquinox, which is an optional field, as non-existent.
	 */
	void SBSummaryRow::clearCenterDirectionEquinox () {
		centerDirectionEquinoxExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a SBSummaryRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SBSummaryRow::SBSummaryRow (SBSummaryTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		centerDirectionCodeExists = false;
	

	
		centerDirectionEquinoxExists = false;
	

	
	
	
	
	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
frequencyBand = CReceiverBand::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
sbType = CSBType::from_int(0);
	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
centerDirectionCode = CDirectionReferenceCode::from_int(0);
	

	

	
	
	 fromBinMethods["sBSummaryId"] = &SBSummaryRow::sBSummaryIdFromBin; 
	 fromBinMethods["sbSummaryUID"] = &SBSummaryRow::sbSummaryUIDFromBin; 
	 fromBinMethods["projectUID"] = &SBSummaryRow::projectUIDFromBin; 
	 fromBinMethods["obsUnitSetId"] = &SBSummaryRow::obsUnitSetIdFromBin; 
	 fromBinMethods["frequency"] = &SBSummaryRow::frequencyFromBin; 
	 fromBinMethods["frequencyBand"] = &SBSummaryRow::frequencyBandFromBin; 
	 fromBinMethods["sbType"] = &SBSummaryRow::sbTypeFromBin; 
	 fromBinMethods["sbDuration"] = &SBSummaryRow::sbDurationFromBin; 
	 fromBinMethods["centerDirection"] = &SBSummaryRow::centerDirectionFromBin; 
	 fromBinMethods["numObservingMode"] = &SBSummaryRow::numObservingModeFromBin; 
	 fromBinMethods["observingMode"] = &SBSummaryRow::observingModeFromBin; 
	 fromBinMethods["numberRepeats"] = &SBSummaryRow::numberRepeatsFromBin; 
	 fromBinMethods["numScienceGoal"] = &SBSummaryRow::numScienceGoalFromBin; 
	 fromBinMethods["scienceGoal"] = &SBSummaryRow::scienceGoalFromBin; 
	 fromBinMethods["numWeatherConstraint"] = &SBSummaryRow::numWeatherConstraintFromBin; 
	 fromBinMethods["weatherConstraint"] = &SBSummaryRow::weatherConstraintFromBin; 
		
	
	 fromBinMethods["centerDirectionCode"] = &SBSummaryRow::centerDirectionCodeFromBin; 
	 fromBinMethods["centerDirectionEquinox"] = &SBSummaryRow::centerDirectionEquinoxFromBin; 
	
	}
	
	SBSummaryRow::SBSummaryRow (SBSummaryTable &t, SBSummaryRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		centerDirectionCodeExists = false;
	

	
		centerDirectionEquinoxExists = false;
	

			
		}
		else {
	
		
			sBSummaryId = row.sBSummaryId;
		
		
		
		
			sbSummaryUID = row.sbSummaryUID;
		
			projectUID = row.projectUID;
		
			obsUnitSetId = row.obsUnitSetId;
		
			frequency = row.frequency;
		
			frequencyBand = row.frequencyBand;
		
			sbType = row.sbType;
		
			sbDuration = row.sbDuration;
		
			centerDirection = row.centerDirection;
		
			numObservingMode = row.numObservingMode;
		
			observingMode = row.observingMode;
		
			numberRepeats = row.numberRepeats;
		
			numScienceGoal = row.numScienceGoal;
		
			scienceGoal = row.scienceGoal;
		
			numWeatherConstraint = row.numWeatherConstraint;
		
			weatherConstraint = row.weatherConstraint;
		
		
		
		
		if (row.centerDirectionCodeExists) {
			centerDirectionCode = row.centerDirectionCode;		
			centerDirectionCodeExists = true;
		}
		else
			centerDirectionCodeExists = false;
		
		if (row.centerDirectionEquinoxExists) {
			centerDirectionEquinox = row.centerDirectionEquinox;		
			centerDirectionEquinoxExists = true;
		}
		else
			centerDirectionEquinoxExists = false;
		
		}
		
		 fromBinMethods["sBSummaryId"] = &SBSummaryRow::sBSummaryIdFromBin; 
		 fromBinMethods["sbSummaryUID"] = &SBSummaryRow::sbSummaryUIDFromBin; 
		 fromBinMethods["projectUID"] = &SBSummaryRow::projectUIDFromBin; 
		 fromBinMethods["obsUnitSetId"] = &SBSummaryRow::obsUnitSetIdFromBin; 
		 fromBinMethods["frequency"] = &SBSummaryRow::frequencyFromBin; 
		 fromBinMethods["frequencyBand"] = &SBSummaryRow::frequencyBandFromBin; 
		 fromBinMethods["sbType"] = &SBSummaryRow::sbTypeFromBin; 
		 fromBinMethods["sbDuration"] = &SBSummaryRow::sbDurationFromBin; 
		 fromBinMethods["centerDirection"] = &SBSummaryRow::centerDirectionFromBin; 
		 fromBinMethods["numObservingMode"] = &SBSummaryRow::numObservingModeFromBin; 
		 fromBinMethods["observingMode"] = &SBSummaryRow::observingModeFromBin; 
		 fromBinMethods["numberRepeats"] = &SBSummaryRow::numberRepeatsFromBin; 
		 fromBinMethods["numScienceGoal"] = &SBSummaryRow::numScienceGoalFromBin; 
		 fromBinMethods["scienceGoal"] = &SBSummaryRow::scienceGoalFromBin; 
		 fromBinMethods["numWeatherConstraint"] = &SBSummaryRow::numWeatherConstraintFromBin; 
		 fromBinMethods["weatherConstraint"] = &SBSummaryRow::weatherConstraintFromBin; 
			
	
		 fromBinMethods["centerDirectionCode"] = &SBSummaryRow::centerDirectionCodeFromBin; 
		 fromBinMethods["centerDirectionEquinox"] = &SBSummaryRow::centerDirectionEquinoxFromBin; 
			
	}

	
	bool SBSummaryRow::compareNoAutoInc(EntityRef sbSummaryUID, EntityRef projectUID, EntityRef obsUnitSetId, double frequency, ReceiverBandMod::ReceiverBand frequencyBand, SBTypeMod::SBType sbType, Interval sbDuration, vector<Angle > centerDirection, int numObservingMode, vector<string > observingMode, int numberRepeats, int numScienceGoal, vector<string > scienceGoal, int numWeatherConstraint, vector<string > weatherConstraint) {
		bool result;
		result = true;
		
	
		
		result = result && (this->sbSummaryUID == sbSummaryUID);
		
		if (!result) return false;
	

	
		
		result = result && (this->projectUID == projectUID);
		
		if (!result) return false;
	

	
		
		result = result && (this->obsUnitSetId == obsUnitSetId);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequency == frequency);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyBand == frequencyBand);
		
		if (!result) return false;
	

	
		
		result = result && (this->sbType == sbType);
		
		if (!result) return false;
	

	
		
		result = result && (this->sbDuration == sbDuration);
		
		if (!result) return false;
	

	
		
		result = result && (this->centerDirection == centerDirection);
		
		if (!result) return false;
	

	
		
		result = result && (this->numObservingMode == numObservingMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->observingMode == observingMode);
		
		if (!result) return false;
	

	
		
		result = result && (this->numberRepeats == numberRepeats);
		
		if (!result) return false;
	

	
		
		result = result && (this->numScienceGoal == numScienceGoal);
		
		if (!result) return false;
	

	
		
		result = result && (this->scienceGoal == scienceGoal);
		
		if (!result) return false;
	

	
		
		result = result && (this->numWeatherConstraint == numWeatherConstraint);
		
		if (!result) return false;
	

	
		
		result = result && (this->weatherConstraint == weatherConstraint);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool SBSummaryRow::compareRequiredValue(EntityRef sbSummaryUID, EntityRef projectUID, EntityRef obsUnitSetId, double frequency, ReceiverBandMod::ReceiverBand frequencyBand, SBTypeMod::SBType sbType, Interval sbDuration, vector<Angle > centerDirection, int numObservingMode, vector<string > observingMode, int numberRepeats, int numScienceGoal, vector<string > scienceGoal, int numWeatherConstraint, vector<string > weatherConstraint) {
		bool result;
		result = true;
		
	
		if (!(this->sbSummaryUID == sbSummaryUID)) return false;
	

	
		if (!(this->projectUID == projectUID)) return false;
	

	
		if (!(this->obsUnitSetId == obsUnitSetId)) return false;
	

	
		if (!(this->frequency == frequency)) return false;
	

	
		if (!(this->frequencyBand == frequencyBand)) return false;
	

	
		if (!(this->sbType == sbType)) return false;
	

	
		if (!(this->sbDuration == sbDuration)) return false;
	

	
		if (!(this->centerDirection == centerDirection)) return false;
	

	
		if (!(this->numObservingMode == numObservingMode)) return false;
	

	
		if (!(this->observingMode == observingMode)) return false;
	

	
		if (!(this->numberRepeats == numberRepeats)) return false;
	

	
		if (!(this->numScienceGoal == numScienceGoal)) return false;
	

	
		if (!(this->scienceGoal == scienceGoal)) return false;
	

	
		if (!(this->numWeatherConstraint == numWeatherConstraint)) return false;
	

	
		if (!(this->weatherConstraint == weatherConstraint)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SBSummaryRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool SBSummaryRow::equalByRequiredValue(SBSummaryRow* x) {
		
			
		if (this->sbSummaryUID != x->sbSummaryUID) return false;
			
		if (this->projectUID != x->projectUID) return false;
			
		if (this->obsUnitSetId != x->obsUnitSetId) return false;
			
		if (this->frequency != x->frequency) return false;
			
		if (this->frequencyBand != x->frequencyBand) return false;
			
		if (this->sbType != x->sbType) return false;
			
		if (this->sbDuration != x->sbDuration) return false;
			
		if (this->centerDirection != x->centerDirection) return false;
			
		if (this->numObservingMode != x->numObservingMode) return false;
			
		if (this->observingMode != x->observingMode) return false;
			
		if (this->numberRepeats != x->numberRepeats) return false;
			
		if (this->numScienceGoal != x->numScienceGoal) return false;
			
		if (this->scienceGoal != x->scienceGoal) return false;
			
		if (this->numWeatherConstraint != x->numWeatherConstraint) return false;
			
		if (this->weatherConstraint != x->weatherConstraint) return false;
			
		
		return true;
	}	
	
/*
	 map<string, SBSummaryAttributeFromBin> SBSummaryRow::initFromBinMethods() {
		map<string, SBSummaryAttributeFromBin> result;
		
		result["sBSummaryId"] = &SBSummaryRow::sBSummaryIdFromBin;
		result["sbSummaryUID"] = &SBSummaryRow::sbSummaryUIDFromBin;
		result["projectUID"] = &SBSummaryRow::projectUIDFromBin;
		result["obsUnitSetId"] = &SBSummaryRow::obsUnitSetIdFromBin;
		result["frequency"] = &SBSummaryRow::frequencyFromBin;
		result["frequencyBand"] = &SBSummaryRow::frequencyBandFromBin;
		result["sbType"] = &SBSummaryRow::sbTypeFromBin;
		result["sbDuration"] = &SBSummaryRow::sbDurationFromBin;
		result["centerDirection"] = &SBSummaryRow::centerDirectionFromBin;
		result["numObservingMode"] = &SBSummaryRow::numObservingModeFromBin;
		result["observingMode"] = &SBSummaryRow::observingModeFromBin;
		result["numberRepeats"] = &SBSummaryRow::numberRepeatsFromBin;
		result["numScienceGoal"] = &SBSummaryRow::numScienceGoalFromBin;
		result["scienceGoal"] = &SBSummaryRow::scienceGoalFromBin;
		result["numWeatherConstraint"] = &SBSummaryRow::numWeatherConstraintFromBin;
		result["weatherConstraint"] = &SBSummaryRow::weatherConstraintFromBin;
		
		
		result["centerDirectionCode"] = &SBSummaryRow::centerDirectionCodeFromBin;
		result["centerDirectionEquinox"] = &SBSummaryRow::centerDirectionEquinoxFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
