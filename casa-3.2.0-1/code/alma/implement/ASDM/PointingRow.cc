
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
 * File PointingRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <PointingRow.h>
#include <PointingTable.h>

#include <PointingModelTable.h>
#include <PointingModelRow.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::PointingRow;
using asdm::PointingTable;

using asdm::PointingModelTable;
using asdm::PointingModelRow;

using asdm::AntennaTable;
using asdm::AntennaRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	PointingRow::~PointingRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	PointingTable &PointingRow::getTable() const {
		return table;
	}

	bool PointingRow::isAdded() const {
		return hasBeenAdded;
	}	

	void PointingRow::isAdded(bool added) {
		hasBeenAdded = added;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a PointingRowIDL struct.
	 */
	PointingRowIDL *PointingRow::toIDL() const {
		PointingRowIDL *x = new PointingRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->numSample = numSample;
 				
 			
		
	

	
  		
		
		
			
		x->encoder.length(encoder.size());
		for (unsigned int i = 0; i < encoder.size(); i++) {
			x->encoder[i].length(encoder.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < encoder.size() ; i++)
			for (unsigned int j = 0; j < encoder.at(i).size(); j++)
					
				x->encoder[i][j]= encoder.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		
			
				
		x->pointingTracking = pointingTracking;
 				
 			
		
	

	
  		
		
		
			
				
		x->usePolynomials = usePolynomials;
 				
 			
		
	

	
  		
		
		
			
		x->timeOrigin = timeOrigin.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->numTerm = numTerm;
 				
 			
		
	

	
  		
		
		
			
		x->pointingDirection.length(pointingDirection.size());
		for (unsigned int i = 0; i < pointingDirection.size(); i++) {
			x->pointingDirection[i].length(pointingDirection.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < pointingDirection.size() ; i++)
			for (unsigned int j = 0; j < pointingDirection.at(i).size(); j++)
					
				x->pointingDirection[i][j]= pointingDirection.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		
			
		x->target.length(target.size());
		for (unsigned int i = 0; i < target.size(); i++) {
			x->target[i].length(target.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < target.size() ; i++)
			for (unsigned int j = 0; j < target.at(i).size(); j++)
					
				x->target[i][j]= target.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		
			
		x->offset.length(offset.size());
		for (unsigned int i = 0; i < offset.size(); i++) {
			x->offset[i].length(offset.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < offset.size() ; i++)
			for (unsigned int j = 0; j < offset.at(i).size(); j++)
					
				x->offset[i][j]= offset.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->overTheTopExists = overTheTopExists;
		
		
			
				
		x->overTheTop = overTheTop;
 				
 			
		
	

	
  		
		
		x->sourceOffsetExists = sourceOffsetExists;
		
		
			
		x->sourceOffset.length(sourceOffset.size());
		for (unsigned int i = 0; i < sourceOffset.size(); i++) {
			x->sourceOffset[i].length(sourceOffset.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < sourceOffset.size() ; i++)
			for (unsigned int j = 0; j < sourceOffset.at(i).size(); j++)
					
				x->sourceOffset[i][j]= sourceOffset.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->sourceOffsetReferenceCodeExists = sourceOffsetReferenceCodeExists;
		
		
			
				
		x->sourceOffsetReferenceCode = sourceOffsetReferenceCode;
 				
 			
		
	

	
  		
		
		x->sourceOffsetEquinoxExists = sourceOffsetEquinoxExists;
		
		
			
		x->sourceOffsetEquinox = sourceOffsetEquinox.toIDLArrayTime();
			
		
	

	
  		
		
		x->sampledTimeIntervalExists = sampledTimeIntervalExists;
		
		
			
		x->sampledTimeInterval.length(sampledTimeInterval.size());
		for (unsigned int i = 0; i < sampledTimeInterval.size(); ++i) {
			
			x->sampledTimeInterval[i] = sampledTimeInterval.at(i).toIDLArrayTimeInterval();
			
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
				
		x->pointingModelId = pointingModelId;
 				
 			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct PointingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void PointingRow::setFromIDL (PointingRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumSample(x.numSample);
  			
 		
		
	

	
		
		
			
		encoder .clear();
		vector<Angle> v_aux_encoder;
		for (unsigned int i = 0; i < x.encoder.length(); ++i) {
			v_aux_encoder.clear();
			for (unsigned int j = 0; j < x.encoder[0].length(); ++j) {
				
				v_aux_encoder.push_back(Angle (x.encoder[i][j]));
				
  			}
  			encoder.push_back(v_aux_encoder);			
		}
			
  		
		
	

	
		
		
			
		setPointingTracking(x.pointingTracking);
  			
 		
		
	

	
		
		
			
		setUsePolynomials(x.usePolynomials);
  			
 		
		
	

	
		
		
			
		setTimeOrigin(ArrayTime (x.timeOrigin));
			
 		
		
	

	
		
		
			
		setNumTerm(x.numTerm);
  			
 		
		
	

	
		
		
			
		pointingDirection .clear();
		vector<Angle> v_aux_pointingDirection;
		for (unsigned int i = 0; i < x.pointingDirection.length(); ++i) {
			v_aux_pointingDirection.clear();
			for (unsigned int j = 0; j < x.pointingDirection[0].length(); ++j) {
				
				v_aux_pointingDirection.push_back(Angle (x.pointingDirection[i][j]));
				
  			}
  			pointingDirection.push_back(v_aux_pointingDirection);			
		}
			
  		
		
	

	
		
		
			
		target .clear();
		vector<Angle> v_aux_target;
		for (unsigned int i = 0; i < x.target.length(); ++i) {
			v_aux_target.clear();
			for (unsigned int j = 0; j < x.target[0].length(); ++j) {
				
				v_aux_target.push_back(Angle (x.target[i][j]));
				
  			}
  			target.push_back(v_aux_target);			
		}
			
  		
		
	

	
		
		
			
		offset .clear();
		vector<Angle> v_aux_offset;
		for (unsigned int i = 0; i < x.offset.length(); ++i) {
			v_aux_offset.clear();
			for (unsigned int j = 0; j < x.offset[0].length(); ++j) {
				
				v_aux_offset.push_back(Angle (x.offset[i][j]));
				
  			}
  			offset.push_back(v_aux_offset);			
		}
			
  		
		
	

	
		
		overTheTopExists = x.overTheTopExists;
		if (x.overTheTopExists) {
		
		
			
		setOverTheTop(x.overTheTop);
  			
 		
		
		}
		
	

	
		
		sourceOffsetExists = x.sourceOffsetExists;
		if (x.sourceOffsetExists) {
		
		
			
		sourceOffset .clear();
		vector<Angle> v_aux_sourceOffset;
		for (unsigned int i = 0; i < x.sourceOffset.length(); ++i) {
			v_aux_sourceOffset.clear();
			for (unsigned int j = 0; j < x.sourceOffset[0].length(); ++j) {
				
				v_aux_sourceOffset.push_back(Angle (x.sourceOffset[i][j]));
				
  			}
  			sourceOffset.push_back(v_aux_sourceOffset);			
		}
			
  		
		
		}
		
	

	
		
		sourceOffsetReferenceCodeExists = x.sourceOffsetReferenceCodeExists;
		if (x.sourceOffsetReferenceCodeExists) {
		
		
			
		setSourceOffsetReferenceCode(x.sourceOffsetReferenceCode);
  			
 		
		
		}
		
	

	
		
		sourceOffsetEquinoxExists = x.sourceOffsetEquinoxExists;
		if (x.sourceOffsetEquinoxExists) {
		
		
			
		setSourceOffsetEquinox(ArrayTime (x.sourceOffsetEquinox));
			
 		
		
		}
		
	

	
		
		sampledTimeIntervalExists = x.sampledTimeIntervalExists;
		if (x.sampledTimeIntervalExists) {
		
		
			
		sampledTimeInterval .clear();
		for (unsigned int i = 0; i <x.sampledTimeInterval.length(); ++i) {
			
			sampledTimeInterval.push_back(ArrayTimeInterval (x.sampledTimeInterval[i]));
			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setPointingModelId(x.pointingModelId);
  			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Pointing");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string PointingRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(numSample, "numSample", buf);
		
		
	

  	
 		
		
		Parser::toXML(encoder, "encoder", buf);
		
		
	

  	
 		
		
		Parser::toXML(pointingTracking, "pointingTracking", buf);
		
		
	

  	
 		
		
		Parser::toXML(usePolynomials, "usePolynomials", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeOrigin, "timeOrigin", buf);
		
		
	

  	
 		
		
		Parser::toXML(numTerm, "numTerm", buf);
		
		
	

  	
 		
		
		Parser::toXML(pointingDirection, "pointingDirection", buf);
		
		
	

  	
 		
		
		Parser::toXML(target, "target", buf);
		
		
	

  	
 		
		
		Parser::toXML(offset, "offset", buf);
		
		
	

  	
 		
		if (overTheTopExists) {
		
		
		Parser::toXML(overTheTop, "overTheTop", buf);
		
		
		}
		
	

  	
 		
		if (sourceOffsetExists) {
		
		
		Parser::toXML(sourceOffset, "sourceOffset", buf);
		
		
		}
		
	

  	
 		
		if (sourceOffsetReferenceCodeExists) {
		
		
			buf.append(EnumerationParser::toXML("sourceOffsetReferenceCode", sourceOffsetReferenceCode));
		
		
		}
		
	

  	
 		
		if (sourceOffsetEquinoxExists) {
		
		
		Parser::toXML(sourceOffsetEquinox, "sourceOffsetEquinox", buf);
		
		
		}
		
	

  	
 		
		if (sampledTimeIntervalExists) {
		
		
		Parser::toXML(sampledTimeInterval, "sampledTimeInterval", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(pointingModelId, "pointingModelId", buf);
		
		
	

	
		
	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void PointingRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Pointing",rowDoc));
			
		
	

	
  		
			
	  	setNumSample(Parser::getInteger("numSample","Pointing",rowDoc));
			
		
	

	
  		
			
					
	  	setEncoder(Parser::get2DAngle("encoder","Pointing",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setPointingTracking(Parser::getBoolean("pointingTracking","Pointing",rowDoc));
			
		
	

	
  		
			
	  	setUsePolynomials(Parser::getBoolean("usePolynomials","Pointing",rowDoc));
			
		
	

	
  		
			
	  	setTimeOrigin(Parser::getArrayTime("timeOrigin","Pointing",rowDoc));
			
		
	

	
  		
			
	  	setNumTerm(Parser::getInteger("numTerm","Pointing",rowDoc));
			
		
	

	
  		
			
					
	  	setPointingDirection(Parser::get2DAngle("pointingDirection","Pointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setTarget(Parser::get2DAngle("target","Pointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setOffset(Parser::get2DAngle("offset","Pointing",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<overTheTop>")) {
			
	  		setOverTheTop(Parser::getBoolean("overTheTop","Pointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<sourceOffset>")) {
			
								
	  		setSourceOffset(Parser::get2DAngle("sourceOffset","Pointing",rowDoc));
	  			
	  		
		}
 		
	

	
		
	if (row.isStr("<sourceOffsetReferenceCode>")) {
		
		
		
		sourceOffsetReferenceCode = EnumerationParser::getDirectionReferenceCode("sourceOffsetReferenceCode","Pointing",rowDoc);
		
		
		
		sourceOffsetReferenceCodeExists = true;
	}
		
	

	
  		
        if (row.isStr("<sourceOffsetEquinox>")) {
			
	  		setSourceOffsetEquinox(Parser::getArrayTime("sourceOffsetEquinox","Pointing",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<sampledTimeInterval>")) {
			
								
	  		setSampledTimeInterval(Parser::get1DArrayTimeInterval("sampledTimeInterval","Pointing",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setPointingModelId(Parser::getInteger("pointingModelId","Pointing",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Pointing");
		}
	}
	
	void PointingRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numSample);
				
		
	

	
	
		
	Angle::toBin(encoder, eoss);
		
	

	
	
		
						
			eoss.writeBoolean(pointingTracking);
				
		
	

	
	
		
						
			eoss.writeBoolean(usePolynomials);
				
		
	

	
	
		
	timeOrigin.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numTerm);
				
		
	

	
	
		
	Angle::toBin(pointingDirection, eoss);
		
	

	
	
		
	Angle::toBin(target, eoss);
		
	

	
	
		
	Angle::toBin(offset, eoss);
		
	

	
	
		
						
			eoss.writeInt(pointingModelId);
				
		
	


	
	
	eoss.writeBoolean(overTheTopExists);
	if (overTheTopExists) {
	
	
	
		
						
			eoss.writeBoolean(overTheTop);
				
		
	

	}

	eoss.writeBoolean(sourceOffsetExists);
	if (sourceOffsetExists) {
	
	
	
		
	Angle::toBin(sourceOffset, eoss);
		
	

	}

	eoss.writeBoolean(sourceOffsetReferenceCodeExists);
	if (sourceOffsetReferenceCodeExists) {
	
	
	
		
					
			eoss.writeInt(sourceOffsetReferenceCode);
				
		
	

	}

	eoss.writeBoolean(sourceOffsetEquinoxExists);
	if (sourceOffsetEquinoxExists) {
	
	
	
		
	sourceOffsetEquinox.toBin(eoss);
		
	

	}

	eoss.writeBoolean(sampledTimeIntervalExists);
	if (sampledTimeIntervalExists) {
	
	
	
		
	ArrayTimeInterval::toBin(sampledTimeInterval, eoss);
		
	

	}

	}
	
void PointingRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void PointingRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void PointingRow::numSampleFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numSample =  eiss.readInt();
			
		
	
	
}
void PointingRow::encoderFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	encoder = Angle::from2DBin(eiss);		
	

		
	
	
}
void PointingRow::pointingTrackingFromBin(EndianISStream& eiss) {
		
	
	
		
			
		pointingTracking =  eiss.readBoolean();
			
		
	
	
}
void PointingRow::usePolynomialsFromBin(EndianISStream& eiss) {
		
	
	
		
			
		usePolynomials =  eiss.readBoolean();
			
		
	
	
}
void PointingRow::timeOriginFromBin(EndianISStream& eiss) {
		
	
		
		
		timeOrigin =  ArrayTime::fromBin(eiss);
		
	
	
}
void PointingRow::numTermFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numTerm =  eiss.readInt();
			
		
	
	
}
void PointingRow::pointingDirectionFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	pointingDirection = Angle::from2DBin(eiss);		
	

		
	
	
}
void PointingRow::targetFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	target = Angle::from2DBin(eiss);		
	

		
	
	
}
void PointingRow::offsetFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	offset = Angle::from2DBin(eiss);		
	

		
	
	
}
void PointingRow::pointingModelIdFromBin(EndianISStream& eiss) {
		
	
	
		
			
		pointingModelId =  eiss.readInt();
			
		
	
	
}

void PointingRow::overTheTopFromBin(EndianISStream& eiss) {
		
	overTheTopExists = eiss.readBoolean();
	if (overTheTopExists) {
		
	
	
		
			
		overTheTop =  eiss.readBoolean();
			
		
	

	}
	
}
void PointingRow::sourceOffsetFromBin(EndianISStream& eiss) {
		
	sourceOffsetExists = eiss.readBoolean();
	if (sourceOffsetExists) {
		
	
		
		
			
	
	sourceOffset = Angle::from2DBin(eiss);		
	

		
	

	}
	
}
void PointingRow::sourceOffsetReferenceCodeFromBin(EndianISStream& eiss) {
		
	sourceOffsetReferenceCodeExists = eiss.readBoolean();
	if (sourceOffsetReferenceCodeExists) {
		
	
	
		
			
		sourceOffsetReferenceCode = CDirectionReferenceCode::from_int(eiss.readInt());
			
		
	

	}
	
}
void PointingRow::sourceOffsetEquinoxFromBin(EndianISStream& eiss) {
		
	sourceOffsetEquinoxExists = eiss.readBoolean();
	if (sourceOffsetEquinoxExists) {
		
	
		
		
		sourceOffsetEquinox =  ArrayTime::fromBin(eiss);
		
	

	}
	
}
void PointingRow::sampledTimeIntervalFromBin(EndianISStream& eiss) {
		
	sampledTimeIntervalExists = eiss.readBoolean();
	if (sampledTimeIntervalExists) {
		
	
		
		
			
	
	sampledTimeInterval = ArrayTimeInterval::from1DBin(eiss);	
	

		
	

	}
	
}
	
	
	PointingRow* PointingRow::fromBin(EndianISStream& eiss, PointingTable& table, const vector<string>& attributesSeq) {
		PointingRow* row = new  PointingRow(table);
		
		map<string, PointingAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "PointingTable");
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
 	ArrayTimeInterval PointingRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void PointingRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "Pointing");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get numSample.
 	 * @return numSample as int
 	 */
 	int PointingRow::getNumSample() const {
	
  		return numSample;
 	}

 	/**
 	 * Set numSample with the specified int.
 	 * @param numSample The int value to which numSample is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setNumSample (int numSample)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numSample = numSample;
	
 	}
	
	

	

	
 	/**
 	 * Get encoder.
 	 * @return encoder as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > PointingRow::getEncoder() const {
	
  		return encoder;
 	}

 	/**
 	 * Set encoder with the specified vector<vector<Angle > >.
 	 * @param encoder The vector<vector<Angle > > value to which encoder is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setEncoder (vector<vector<Angle > > encoder)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->encoder = encoder;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingTracking.
 	 * @return pointingTracking as bool
 	 */
 	bool PointingRow::getPointingTracking() const {
	
  		return pointingTracking;
 	}

 	/**
 	 * Set pointingTracking with the specified bool.
 	 * @param pointingTracking The bool value to which pointingTracking is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setPointingTracking (bool pointingTracking)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingTracking = pointingTracking;
	
 	}
	
	

	

	
 	/**
 	 * Get usePolynomials.
 	 * @return usePolynomials as bool
 	 */
 	bool PointingRow::getUsePolynomials() const {
	
  		return usePolynomials;
 	}

 	/**
 	 * Set usePolynomials with the specified bool.
 	 * @param usePolynomials The bool value to which usePolynomials is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setUsePolynomials (bool usePolynomials)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->usePolynomials = usePolynomials;
	
 	}
	
	

	

	
 	/**
 	 * Get timeOrigin.
 	 * @return timeOrigin as ArrayTime
 	 */
 	ArrayTime PointingRow::getTimeOrigin() const {
	
  		return timeOrigin;
 	}

 	/**
 	 * Set timeOrigin with the specified ArrayTime.
 	 * @param timeOrigin The ArrayTime value to which timeOrigin is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setTimeOrigin (ArrayTime timeOrigin)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->timeOrigin = timeOrigin;
	
 	}
	
	

	

	
 	/**
 	 * Get numTerm.
 	 * @return numTerm as int
 	 */
 	int PointingRow::getNumTerm() const {
	
  		return numTerm;
 	}

 	/**
 	 * Set numTerm with the specified int.
 	 * @param numTerm The int value to which numTerm is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setNumTerm (int numTerm)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numTerm = numTerm;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingDirection.
 	 * @return pointingDirection as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > PointingRow::getPointingDirection() const {
	
  		return pointingDirection;
 	}

 	/**
 	 * Set pointingDirection with the specified vector<vector<Angle > >.
 	 * @param pointingDirection The vector<vector<Angle > > value to which pointingDirection is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setPointingDirection (vector<vector<Angle > > pointingDirection)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingDirection = pointingDirection;
	
 	}
	
	

	

	
 	/**
 	 * Get target.
 	 * @return target as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > PointingRow::getTarget() const {
	
  		return target;
 	}

 	/**
 	 * Set target with the specified vector<vector<Angle > >.
 	 * @param target The vector<vector<Angle > > value to which target is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setTarget (vector<vector<Angle > > target)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->target = target;
	
 	}
	
	

	

	
 	/**
 	 * Get offset.
 	 * @return offset as vector<vector<Angle > >
 	 */
 	vector<vector<Angle > > PointingRow::getOffset() const {
	
  		return offset;
 	}

 	/**
 	 * Set offset with the specified vector<vector<Angle > >.
 	 * @param offset The vector<vector<Angle > > value to which offset is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setOffset (vector<vector<Angle > > offset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->offset = offset;
	
 	}
	
	

	
	/**
	 * The attribute overTheTop is optional. Return true if this attribute exists.
	 * @return true if and only if the overTheTop attribute exists. 
	 */
	bool PointingRow::isOverTheTopExists() const {
		return overTheTopExists;
	}
	

	
 	/**
 	 * Get overTheTop, which is optional.
 	 * @return overTheTop as bool
 	 * @throw IllegalAccessException If overTheTop does not exist.
 	 */
 	bool PointingRow::getOverTheTop() const  {
		if (!overTheTopExists) {
			throw IllegalAccessException("overTheTop", "Pointing");
		}
	
  		return overTheTop;
 	}

 	/**
 	 * Set overTheTop with the specified bool.
 	 * @param overTheTop The bool value to which overTheTop is to be set.
 	 
 	
 	 */
 	void PointingRow::setOverTheTop (bool overTheTop) {
	
 		this->overTheTop = overTheTop;
	
		overTheTopExists = true;
	
 	}
	
	
	/**
	 * Mark overTheTop, which is an optional field, as non-existent.
	 */
	void PointingRow::clearOverTheTop () {
		overTheTopExists = false;
	}
	

	
	/**
	 * The attribute sourceOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceOffset attribute exists. 
	 */
	bool PointingRow::isSourceOffsetExists() const {
		return sourceOffsetExists;
	}
	

	
 	/**
 	 * Get sourceOffset, which is optional.
 	 * @return sourceOffset as vector<vector<Angle > >
 	 * @throw IllegalAccessException If sourceOffset does not exist.
 	 */
 	vector<vector<Angle > > PointingRow::getSourceOffset() const  {
		if (!sourceOffsetExists) {
			throw IllegalAccessException("sourceOffset", "Pointing");
		}
	
  		return sourceOffset;
 	}

 	/**
 	 * Set sourceOffset with the specified vector<vector<Angle > >.
 	 * @param sourceOffset The vector<vector<Angle > > value to which sourceOffset is to be set.
 	 
 	
 	 */
 	void PointingRow::setSourceOffset (vector<vector<Angle > > sourceOffset) {
	
 		this->sourceOffset = sourceOffset;
	
		sourceOffsetExists = true;
	
 	}
	
	
	/**
	 * Mark sourceOffset, which is an optional field, as non-existent.
	 */
	void PointingRow::clearSourceOffset () {
		sourceOffsetExists = false;
	}
	

	
	/**
	 * The attribute sourceOffsetReferenceCode is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceOffsetReferenceCode attribute exists. 
	 */
	bool PointingRow::isSourceOffsetReferenceCodeExists() const {
		return sourceOffsetReferenceCodeExists;
	}
	

	
 	/**
 	 * Get sourceOffsetReferenceCode, which is optional.
 	 * @return sourceOffsetReferenceCode as DirectionReferenceCodeMod::DirectionReferenceCode
 	 * @throw IllegalAccessException If sourceOffsetReferenceCode does not exist.
 	 */
 	DirectionReferenceCodeMod::DirectionReferenceCode PointingRow::getSourceOffsetReferenceCode() const  {
		if (!sourceOffsetReferenceCodeExists) {
			throw IllegalAccessException("sourceOffsetReferenceCode", "Pointing");
		}
	
  		return sourceOffsetReferenceCode;
 	}

 	/**
 	 * Set sourceOffsetReferenceCode with the specified DirectionReferenceCodeMod::DirectionReferenceCode.
 	 * @param sourceOffsetReferenceCode The DirectionReferenceCodeMod::DirectionReferenceCode value to which sourceOffsetReferenceCode is to be set.
 	 
 	
 	 */
 	void PointingRow::setSourceOffsetReferenceCode (DirectionReferenceCodeMod::DirectionReferenceCode sourceOffsetReferenceCode) {
	
 		this->sourceOffsetReferenceCode = sourceOffsetReferenceCode;
	
		sourceOffsetReferenceCodeExists = true;
	
 	}
	
	
	/**
	 * Mark sourceOffsetReferenceCode, which is an optional field, as non-existent.
	 */
	void PointingRow::clearSourceOffsetReferenceCode () {
		sourceOffsetReferenceCodeExists = false;
	}
	

	
	/**
	 * The attribute sourceOffsetEquinox is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceOffsetEquinox attribute exists. 
	 */
	bool PointingRow::isSourceOffsetEquinoxExists() const {
		return sourceOffsetEquinoxExists;
	}
	

	
 	/**
 	 * Get sourceOffsetEquinox, which is optional.
 	 * @return sourceOffsetEquinox as ArrayTime
 	 * @throw IllegalAccessException If sourceOffsetEquinox does not exist.
 	 */
 	ArrayTime PointingRow::getSourceOffsetEquinox() const  {
		if (!sourceOffsetEquinoxExists) {
			throw IllegalAccessException("sourceOffsetEquinox", "Pointing");
		}
	
  		return sourceOffsetEquinox;
 	}

 	/**
 	 * Set sourceOffsetEquinox with the specified ArrayTime.
 	 * @param sourceOffsetEquinox The ArrayTime value to which sourceOffsetEquinox is to be set.
 	 
 	
 	 */
 	void PointingRow::setSourceOffsetEquinox (ArrayTime sourceOffsetEquinox) {
	
 		this->sourceOffsetEquinox = sourceOffsetEquinox;
	
		sourceOffsetEquinoxExists = true;
	
 	}
	
	
	/**
	 * Mark sourceOffsetEquinox, which is an optional field, as non-existent.
	 */
	void PointingRow::clearSourceOffsetEquinox () {
		sourceOffsetEquinoxExists = false;
	}
	

	
	/**
	 * The attribute sampledTimeInterval is optional. Return true if this attribute exists.
	 * @return true if and only if the sampledTimeInterval attribute exists. 
	 */
	bool PointingRow::isSampledTimeIntervalExists() const {
		return sampledTimeIntervalExists;
	}
	

	
 	/**
 	 * Get sampledTimeInterval, which is optional.
 	 * @return sampledTimeInterval as vector<ArrayTimeInterval >
 	 * @throw IllegalAccessException If sampledTimeInterval does not exist.
 	 */
 	vector<ArrayTimeInterval > PointingRow::getSampledTimeInterval() const  {
		if (!sampledTimeIntervalExists) {
			throw IllegalAccessException("sampledTimeInterval", "Pointing");
		}
	
  		return sampledTimeInterval;
 	}

 	/**
 	 * Set sampledTimeInterval with the specified vector<ArrayTimeInterval >.
 	 * @param sampledTimeInterval The vector<ArrayTimeInterval > value to which sampledTimeInterval is to be set.
 	 
 	
 	 */
 	void PointingRow::setSampledTimeInterval (vector<ArrayTimeInterval > sampledTimeInterval) {
	
 		this->sampledTimeInterval = sampledTimeInterval;
	
		sampledTimeIntervalExists = true;
	
 	}
	
	
	/**
	 * Mark sampledTimeInterval, which is an optional field, as non-existent.
	 */
	void PointingRow::clearSampledTimeInterval () {
		sampledTimeIntervalExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag PointingRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void PointingRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "Pointing");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get pointingModelId.
 	 * @return pointingModelId as int
 	 */
 	int PointingRow::getPointingModelId() const {
	
  		return pointingModelId;
 	}

 	/**
 	 * Set pointingModelId with the specified int.
 	 * @param pointingModelId The int value to which pointingModelId is to be set.
 	 
 	
 		
 	 */
 	void PointingRow::setPointingModelId (int pointingModelId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->pointingModelId = pointingModelId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	// ===> Slice link from a row of Pointing table to a collection of row of PointingModel table.
	
	/**
	 * Get the collection of row in the PointingModel table having their attribut pointingModelId == this->pointingModelId
	 */
	vector <PointingModelRow *> PointingRow::getPointingModels() {
		
			return table.getContainer().getPointingModel().getRowByPointingModelId(pointingModelId);
		
	}
	

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* PointingRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	/**
	 * Create a PointingRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	PointingRow::PointingRow (PointingTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	
		overTheTopExists = false;
	

	
		sourceOffsetExists = false;
	

	
		sourceOffsetReferenceCodeExists = false;
	

	
		sourceOffsetEquinoxExists = false;
	

	
		sampledTimeIntervalExists = false;
	

	
	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
sourceOffsetReferenceCode = CDirectionReferenceCode::from_int(0);
	

	

	

	
	
	 fromBinMethods["antennaId"] = &PointingRow::antennaIdFromBin; 
	 fromBinMethods["timeInterval"] = &PointingRow::timeIntervalFromBin; 
	 fromBinMethods["numSample"] = &PointingRow::numSampleFromBin; 
	 fromBinMethods["encoder"] = &PointingRow::encoderFromBin; 
	 fromBinMethods["pointingTracking"] = &PointingRow::pointingTrackingFromBin; 
	 fromBinMethods["usePolynomials"] = &PointingRow::usePolynomialsFromBin; 
	 fromBinMethods["timeOrigin"] = &PointingRow::timeOriginFromBin; 
	 fromBinMethods["numTerm"] = &PointingRow::numTermFromBin; 
	 fromBinMethods["pointingDirection"] = &PointingRow::pointingDirectionFromBin; 
	 fromBinMethods["target"] = &PointingRow::targetFromBin; 
	 fromBinMethods["offset"] = &PointingRow::offsetFromBin; 
	 fromBinMethods["pointingModelId"] = &PointingRow::pointingModelIdFromBin; 
		
	
	 fromBinMethods["overTheTop"] = &PointingRow::overTheTopFromBin; 
	 fromBinMethods["sourceOffset"] = &PointingRow::sourceOffsetFromBin; 
	 fromBinMethods["sourceOffsetReferenceCode"] = &PointingRow::sourceOffsetReferenceCodeFromBin; 
	 fromBinMethods["sourceOffsetEquinox"] = &PointingRow::sourceOffsetEquinoxFromBin; 
	 fromBinMethods["sampledTimeInterval"] = &PointingRow::sampledTimeIntervalFromBin; 
	
	}
	
	PointingRow::PointingRow (PointingTable &t, PointingRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	
		overTheTopExists = false;
	

	
		sourceOffsetExists = false;
	

	
		sourceOffsetReferenceCodeExists = false;
	

	
		sourceOffsetEquinoxExists = false;
	

	
		sampledTimeIntervalExists = false;
	

	
	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			numSample = row.numSample;
		
			encoder = row.encoder;
		
			pointingTracking = row.pointingTracking;
		
			usePolynomials = row.usePolynomials;
		
			timeOrigin = row.timeOrigin;
		
			numTerm = row.numTerm;
		
			pointingDirection = row.pointingDirection;
		
			target = row.target;
		
			offset = row.offset;
		
			pointingModelId = row.pointingModelId;
		
		
		
		
		if (row.overTheTopExists) {
			overTheTop = row.overTheTop;		
			overTheTopExists = true;
		}
		else
			overTheTopExists = false;
		
		if (row.sourceOffsetExists) {
			sourceOffset = row.sourceOffset;		
			sourceOffsetExists = true;
		}
		else
			sourceOffsetExists = false;
		
		if (row.sourceOffsetReferenceCodeExists) {
			sourceOffsetReferenceCode = row.sourceOffsetReferenceCode;		
			sourceOffsetReferenceCodeExists = true;
		}
		else
			sourceOffsetReferenceCodeExists = false;
		
		if (row.sourceOffsetEquinoxExists) {
			sourceOffsetEquinox = row.sourceOffsetEquinox;		
			sourceOffsetEquinoxExists = true;
		}
		else
			sourceOffsetEquinoxExists = false;
		
		if (row.sampledTimeIntervalExists) {
			sampledTimeInterval = row.sampledTimeInterval;		
			sampledTimeIntervalExists = true;
		}
		else
			sampledTimeIntervalExists = false;
		
		}
		
		 fromBinMethods["antennaId"] = &PointingRow::antennaIdFromBin; 
		 fromBinMethods["timeInterval"] = &PointingRow::timeIntervalFromBin; 
		 fromBinMethods["numSample"] = &PointingRow::numSampleFromBin; 
		 fromBinMethods["encoder"] = &PointingRow::encoderFromBin; 
		 fromBinMethods["pointingTracking"] = &PointingRow::pointingTrackingFromBin; 
		 fromBinMethods["usePolynomials"] = &PointingRow::usePolynomialsFromBin; 
		 fromBinMethods["timeOrigin"] = &PointingRow::timeOriginFromBin; 
		 fromBinMethods["numTerm"] = &PointingRow::numTermFromBin; 
		 fromBinMethods["pointingDirection"] = &PointingRow::pointingDirectionFromBin; 
		 fromBinMethods["target"] = &PointingRow::targetFromBin; 
		 fromBinMethods["offset"] = &PointingRow::offsetFromBin; 
		 fromBinMethods["pointingModelId"] = &PointingRow::pointingModelIdFromBin; 
			
	
		 fromBinMethods["overTheTop"] = &PointingRow::overTheTopFromBin; 
		 fromBinMethods["sourceOffset"] = &PointingRow::sourceOffsetFromBin; 
		 fromBinMethods["sourceOffsetReferenceCode"] = &PointingRow::sourceOffsetReferenceCodeFromBin; 
		 fromBinMethods["sourceOffsetEquinox"] = &PointingRow::sourceOffsetEquinoxFromBin; 
		 fromBinMethods["sampledTimeInterval"] = &PointingRow::sampledTimeIntervalFromBin; 
			
	}

	
	bool PointingRow::compareNoAutoInc(Tag antennaId, ArrayTimeInterval timeInterval, int numSample, vector<vector<Angle > > encoder, bool pointingTracking, bool usePolynomials, ArrayTime timeOrigin, int numTerm, vector<vector<Angle > > pointingDirection, vector<vector<Angle > > target, vector<vector<Angle > > offset, int pointingModelId) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->numSample == numSample);
		
		if (!result) return false;
	

	
		
		result = result && (this->encoder == encoder);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingTracking == pointingTracking);
		
		if (!result) return false;
	

	
		
		result = result && (this->usePolynomials == usePolynomials);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeOrigin == timeOrigin);
		
		if (!result) return false;
	

	
		
		result = result && (this->numTerm == numTerm);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingDirection == pointingDirection);
		
		if (!result) return false;
	

	
		
		result = result && (this->target == target);
		
		if (!result) return false;
	

	
		
		result = result && (this->offset == offset);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingModelId == pointingModelId);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool PointingRow::compareRequiredValue(int numSample, vector<vector<Angle > > encoder, bool pointingTracking, bool usePolynomials, ArrayTime timeOrigin, int numTerm, vector<vector<Angle > > pointingDirection, vector<vector<Angle > > target, vector<vector<Angle > > offset, int pointingModelId) {
		bool result;
		result = true;
		
	
		if (!(this->numSample == numSample)) return false;
	

	
		if (!(this->encoder == encoder)) return false;
	

	
		if (!(this->pointingTracking == pointingTracking)) return false;
	

	
		if (!(this->usePolynomials == usePolynomials)) return false;
	

	
		if (!(this->timeOrigin == timeOrigin)) return false;
	

	
		if (!(this->numTerm == numTerm)) return false;
	

	
		if (!(this->pointingDirection == pointingDirection)) return false;
	

	
		if (!(this->target == target)) return false;
	

	
		if (!(this->offset == offset)) return false;
	

	
		if (!(this->pointingModelId == pointingModelId)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the PointingRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool PointingRow::equalByRequiredValue(PointingRow* x) {
		
			
		if (this->numSample != x->numSample) return false;
			
		if (this->encoder != x->encoder) return false;
			
		if (this->pointingTracking != x->pointingTracking) return false;
			
		if (this->usePolynomials != x->usePolynomials) return false;
			
		if (this->timeOrigin != x->timeOrigin) return false;
			
		if (this->numTerm != x->numTerm) return false;
			
		if (this->pointingDirection != x->pointingDirection) return false;
			
		if (this->target != x->target) return false;
			
		if (this->offset != x->offset) return false;
			
		if (this->pointingModelId != x->pointingModelId) return false;
			
		
		return true;
	}	
	
/*
	 map<string, PointingAttributeFromBin> PointingRow::initFromBinMethods() {
		map<string, PointingAttributeFromBin> result;
		
		result["antennaId"] = &PointingRow::antennaIdFromBin;
		result["timeInterval"] = &PointingRow::timeIntervalFromBin;
		result["numSample"] = &PointingRow::numSampleFromBin;
		result["encoder"] = &PointingRow::encoderFromBin;
		result["pointingTracking"] = &PointingRow::pointingTrackingFromBin;
		result["usePolynomials"] = &PointingRow::usePolynomialsFromBin;
		result["timeOrigin"] = &PointingRow::timeOriginFromBin;
		result["numTerm"] = &PointingRow::numTermFromBin;
		result["pointingDirection"] = &PointingRow::pointingDirectionFromBin;
		result["target"] = &PointingRow::targetFromBin;
		result["offset"] = &PointingRow::offsetFromBin;
		result["pointingModelId"] = &PointingRow::pointingModelIdFromBin;
		
		
		result["overTheTop"] = &PointingRow::overTheTopFromBin;
		result["sourceOffset"] = &PointingRow::sourceOffsetFromBin;
		result["sourceOffsetReferenceCode"] = &PointingRow::sourceOffsetReferenceCodeFromBin;
		result["sourceOffsetEquinox"] = &PointingRow::sourceOffsetEquinoxFromBin;
		result["sampledTimeInterval"] = &PointingRow::sampledTimeIntervalFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
