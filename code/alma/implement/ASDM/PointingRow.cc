
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

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <PointingModelTable.h>
#include <PointingModelRow.h>
	

using asdm::ASDM;
using asdm::PointingRow;
using asdm::PointingTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::PointingModelTable;
using asdm::PointingModelRow;


#include <Parser.h>
using asdm::Parser;
 
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
	
	void PointingRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a PointingRowIDL struct.
	 */
	PointingRowIDL *PointingRow::toIDL() const {
		PointingRowIDL *x = new PointingRowIDL ();
		
		// Fill the IDL structure.
	
		
	
		
		
			
		x->timeInterval = getTimeInterval().toIDLArrayTimeInterval();
			
		
		
	

	
		
		
			
				
		x->name = CORBA::string_dup(getName().c_str());
				
 			
		
		
	

	
		
		
			
				
		x->numPoly = getNumPoly();
 				
 			
		
		
	

	
		
		
			
		x->timeOrigin = getTimeOrigin().toIDLArrayTime();
			
		
		
	

	
		
		
			
		vector< vector<Angle> > tmpPointingDirection = getPointingDirection();
		x->pointingDirection.length(tmpPointingDirection.size());
		for (unsigned int i = 0; i < tmpPointingDirection.size(); ++i)
			x->pointingDirection[i].length(tmpPointingDirection[0].size());
		for (unsigned int i = 0; i < tmpPointingDirection.size(); ++i) {
			for (unsigned int j = 0; j < tmpPointingDirection[0].size(); ++j) {
				
				x->pointingDirection[i][j] = tmpPointingDirection[i][j].toIDLAngle();
				
		 	}
		 }
			
		
		
	

	
		
		
			
		vector< vector<Angle> > tmpTarget = getTarget();
		x->target.length(tmpTarget.size());
		for (unsigned int i = 0; i < tmpTarget.size(); ++i)
			x->target[i].length(tmpTarget[0].size());
		for (unsigned int i = 0; i < tmpTarget.size(); ++i) {
			for (unsigned int j = 0; j < tmpTarget[0].size(); ++j) {
				
				x->target[i][j] = tmpTarget[i][j].toIDLAngle();
				
		 	}
		 }
			
		
		
	

	
		
		vector< vector<Angle> > tmpOffset = getOffset();
		x->offset.length(tmpOffset.size());
		for (unsigned int i = 0; i < tmpOffset.size(); ++i)
			x->offset[i].length(tmpOffset[0].size());
		for (unsigned int i = 0; i < tmpOffset.size(); ++i) {
			for (unsigned int j = 0; j < tmpOffset[0].size(); ++j) {
				
				x->offset[i][j] = tmpOffset[i][j].toIDLAngle();
				
		 	}
		 }
		
	

	
		
		x->sourceOffsetExists = sourceOffsetExists;
  		if (sourceOffsetExists) {
  			try {
		
		
			
		vector< vector<Angle> > tmpSourceOffset = getSourceOffset();
		x->sourceOffset.length(tmpSourceOffset.size());
		for (unsigned int i = 0; i < tmpSourceOffset.size(); ++i)
			x->sourceOffset[i].length(tmpSourceOffset[0].size());
		for (unsigned int i = 0; i < tmpSourceOffset.size(); ++i) {
			for (unsigned int j = 0; j < tmpSourceOffset[0].size(); ++j) {
				
				x->sourceOffset[i][j] = tmpSourceOffset[i][j].toIDLAngle();
				
		 	}
		 }
			
		
		
			} catch (IllegalAccessException e) {
			}
		}
		
	

	
		
		
			
		vector<Angle> tmpEncoder = getEncoder();
		x->encoder.length(tmpEncoder.size());
		for (unsigned int i = 0; i < tmpEncoder.size(); ++i) {
			
			x->encoder[i] = tmpEncoder[i].toIDLAngle();
			
	 	}
			
		
		
	

	
		
		
			
				
		x->pointingTracking = getPointingTracking();
 				
 				
			
		x->phaseTrackingExists = phaseTrackingExists;
		if (phaseTrackingExists) {
			try {		
				x->phaseTracking = getPhaseTracking();
			}
 			catch (IllegalAccessException e) {
			}
		}
		
		
	

	
		
		x->overTheTopExists = overTheTopExists;
  		if (overTheTopExists) {
  			try {
		
		
			
				
		x->overTheTop = getOverTheTop();
 				
 			
		
		
			} catch (IllegalAccessException e) {
			}
		}
		
	

	
	
		
	
		
		
			
		x->antennaId = getAntennaId().toIDLTag();
			
		
		
	

	
		
		
			
				
		x->pointingModelId = getPointingModelId();
 				
 			
		
		
	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct PointingRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void PointingRow::setFromIDL (PointingRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(*(new ArrayTimeInterval (x.timeInterval)));
			
 		
		
	

	
		
		
			
		setName(*(new string (x.name)));
			
 		
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		setTimeOrigin(*(new ArrayTime (x.timeOrigin)));
			
 		
		
	

	
		
		
			
		vector< vector<Angle> > tmpPointingDirection(x.pointingDirection.length());
		for (unsigned int i = 0; i < tmpPointingDirection.size(); ++i)
			tmpPointingDirection[i] = *(new vector<Angle>(x.pointingDirection[0].length()));
		for (unsigned int i = 0; i < tmpPointingDirection.size(); ++i) {
			for (unsigned int j = 0; j < tmpPointingDirection[0].size(); ++j) {
				
				tmpPointingDirection[i][j] = *(new Angle (x.pointingDirection[i][j]));
				
  			}
		}
		setPointingDirection(tmpPointingDirection);
			
  		
		
	

	
		
		
			
		vector< vector<Angle> > tmpTarget(x.target.length());
		for (unsigned int i = 0; i < tmpTarget.size(); ++i)
			tmpTarget[i] = *(new vector<Angle>(x.target[0].length()));
		for (unsigned int i = 0; i < tmpTarget.size(); ++i) {
			for (unsigned int j = 0; j < tmpTarget[0].size(); ++j) {
				
				tmpTarget[i][j] = *(new Angle (x.target[i][j]));
				
  			}
		}
		setTarget(tmpTarget);
			
  		
		
	

	
		
		vector< vector<Angle> > tmpOffset(x.offset.length());
		for (unsigned int i = 0; i < tmpOffset.size(); ++i)
			tmpOffset[i] = *(new vector<Angle>(x.offset[0].length()));
		for (unsigned int i = 0; i < tmpOffset.size(); ++i) {
			for (unsigned int j = 0; j < tmpOffset[0].size(); ++j) {
				
				tmpOffset[i][j] = *(new Angle (x.offset[i][j]));
				
  			}
		}
		setOffset(tmpOffset);
		
	

	
		
		if (x.sourceOffsetExists) {
		
		
			
		vector< vector<Angle> > tmpSourceOffset(x.sourceOffset.length());
		for (unsigned int i = 0; i < tmpSourceOffset.size(); ++i)
			tmpSourceOffset[i] = *(new vector<Angle>(x.sourceOffset[0].length()));
		for (unsigned int i = 0; i < tmpSourceOffset.size(); ++i) {
			for (unsigned int j = 0; j < tmpSourceOffset[0].size(); ++j) {
				
				tmpSourceOffset[i][j] = *(new Angle (x.sourceOffset[i][j]));
				
  			}
		}
		setSourceOffset(tmpSourceOffset);
			
  		
		
		}
		
	

	
		
		
			
		vector<Angle> tmpEncoder(x.encoder.length());
		for (unsigned int i = 0; i < tmpEncoder.size(); ++i) {
			
			tmpEncoder[i] = *(new Angle (x.encoder[i]));
			
		}
		setEncoder(tmpEncoder);
			
  		
		
	

	
		
		
			
		setPointingTracking(x.pointingTracking);
  			
 		
		
	

	
		
		
		if (x.phaseTrackingExists) {
			setPhaseTracking(x.phaseTracking);
		}
 		
		
	

	
		
		if (x.overTheTopExists) {
		
		
			
		setOverTheTop(x.overTheTop);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(*(new Tag (x.antennaId)));
			
 		
		
	

	
		
		
			
		setPointingModelId(x.pointingModelId);
  			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Pointing");
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
		
		
	

  	
 		
		
		if (nameExists) {
			Parser::toXML(name, "name", buf);
		}
		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		
		Parser::toXML(timeOrigin, "timeOrigin", buf);
		
		
	

  	
 		
		
		
		Parser::toXML(pointingDirection, "pointingDirection", buf);
		
		
	

  	
 		
		
		
		Parser::toXML(target, "target", buf);
		
		

		Parser::toXML(offset, "offset", buf);
 		
		if (sourceOffsetExists) {
		
		
		
		Parser::toXML(sourceOffset, "sourceOffset", buf);
		
		
		}
		
	

  	
 		
		
		
		Parser::toXML(encoder, "encoder", buf);
		
		
	

  	
 		
		
		
		Parser::toXML(pointingTracking, "pointingTracking", buf);
		
		
	

  	
 		
		if (phaseTrackingExists) {
			Parser::toXML(phaseTracking, "phaseTracking", buf);
		}
		
		
	

  	
 		
		if (overTheTopExists) {
		
		
		
		Parser::toXML(overTheTop, "overTheTop", buf);
		
		
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
	void PointingRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Pointing",rowDoc));
			
		
	

	
  		
			
	       if (row.isStr("<name>")) {
			
	  		setName(Parser::getString("name","Pointing",rowDoc));
			
		}
			
		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","Pointing",rowDoc));
			
		
	

	
  		
			
	  	setTimeOrigin(Parser::getArrayTime("timeOrigin","Pointing",rowDoc));
			
		
	

	
  		
			
					
	  	setPointingDirection(Parser::get2DAngle("pointingDirection","Pointing",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setTarget(Parser::get2DAngle("target","Pointing",rowDoc));
	  			
	  		
		
	

				
	  	setOffset(Parser::get2DAngle("offset","Pointing",rowDoc));
	
        if (row.isStr("<sourceOffset>")) {
			
								
	  		setSourceOffset(Parser::get2DAngle("sourceOffset","Pointing",rowDoc));
	  			
	  		
		}
 		
	

	
  		
			
					
	  	setEncoder(Parser::get1DAngle("encoder","Pointing",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setPointingTracking(Parser::getBoolean("pointingTracking","Pointing",rowDoc));
			
		
	

	
  		
      if (row.isStr("<phaseTracking>")) {
			
	  		setPhaseTracking(Parser::getBoolean("phaseTracking","Pointing",rowDoc));
			
		}
			
		
	

	
  		
        if (row.isStr("<overTheTop>")) {
			
	  		setOverTheTop(Parser::getBoolean("overTheTop","Pointing",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setPointingModelId(Parser::getInteger("pointingModelId","Pointing",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Pointing");
		}
	}
	
	void PointingRow::toBin(EndianOSStream& eoss) {
		timeInterval.toBin(eoss);
		eoss.writeBoolean(nameExists);
		if (nameExists) {
			eoss.writeString(name);
		}
		eoss.writeInt(numPoly);
		timeOrigin.toBin(eoss);
		Angle::toBin(pointingDirection, eoss);
		Angle::toBin(target, eoss);
		Angle::toBin(offset, eoss);
		eoss.writeBoolean(sourceOffsetExists);
		if (sourceOffsetExists) {
			Angle::toBin(sourceOffset, eoss);
		}
		Angle::toBin(encoder, eoss);
		eoss.writeBoolean(pointingTracking);
		eoss.writeBoolean(phaseTrackingExists);
		if (phaseTrackingExists) {
			eoss.writeBoolean(phaseTracking);	
		}
		eoss.writeBoolean(overTheTopExists);
		if (overTheTopExists) {
			eoss.writeBoolean(overTheTop);
		}
		antennaId.toBin(eoss);
		eoss.writeInt(pointingModelId);
	}
	
	PointingRow* PointingRow::fromBin(EndianISStream& eiss, PointingTable& table) {
		PointingRow* row = new PointingRow(table);
		row->timeInterval = ArrayTimeInterval::fromBin(eiss); 
		row->nameExists = eiss.readBoolean();
		if (row->nameExists) row->name = eiss.readString();
		row->numPoly = eiss.readInt() ; 
		row->timeOrigin = ArrayTime::fromBin(eiss); 
		row->pointingDirection = Angle::from2DBin(eiss);
		row->target =  Angle::from2DBin(eiss);
		row->offset = Angle::from2DBin(eiss);
		row->sourceOffsetExists = eiss.readBoolean(); 
		if (row->sourceOffsetExists) 	row->sourceOffset = Angle::from2DBin(eiss);
		row->encoder = Angle::from1DBin(eiss);
		row->pointingTracking = eiss.readBoolean();
		row->phaseTrackingExists = eiss.readBoolean();
		if (row->phaseTrackingExists) row->phaseTracking = eiss.readBoolean();
		row->overTheTopExists = eiss.readBoolean(); 
		if (row->overTheTopExists)
			row->overTheTop = eiss.readBoolean();
		row->antennaId = Tag::fromBin(eiss); 
		row->pointingModelId = eiss.readInt(); 
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
 	void PointingRow::setTimeInterval (ArrayTimeInterval timeInterval) throw(IllegalAccessException) {
  		if (hasBeenAdded) {
			throw IllegalAccessException();
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	/**
	 * The attribute name is optional. Return true if this attribute exists.
	 * @return true if and only if the name attribute exists. 
	 */
	bool PointingRow::isNameExists() const {
		return nameExists;
	}
		

 	/**
 	 * Get name, which is optional.
 	 * @return name as string
 	 * @throw IllegalAccessException If name does not exist.
 	 */
 	string PointingRow::getName() const throw(IllegalAccessException) {
		if (!nameExists) {
			throw IllegalAccessException();
		}
	
  		return name;
 	}

 	/**
 	 * Set name with the specified string.
 	 * @param name The string value to which name is to be set.
 	
 	 */
 	void PointingRow::setName (string name) {
	
 		this->name = name;
	
		nameExists = true;
	
 	}
	
	
	/**
	 * Mark name, which is an optional field, as non-existent.
	 */
	void PointingRow::clearName () {
		nameExists = false;
	}
	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int PointingRow::getNumPoly() const {
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	
 	 */
 	void PointingRow::setNumPoly (int numPoly) {
	
 		this->numPoly = numPoly;
	
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
 	void PointingRow::setTimeOrigin (ArrayTime timeOrigin) {
	
 		this->timeOrigin = timeOrigin;
	
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
 	void PointingRow::setPointingDirection (vector<vector<Angle > > pointingDirection) {
	
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
 	void PointingRow::setTarget (vector<vector<Angle > > target) {
	
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
 	void PointingRow::setOffset (vector<vector<Angle > > offset) {
	
 		this->offset = offset;
	
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
 	vector<vector<Angle > > PointingRow::getSourceOffset() const throw(IllegalAccessException) {
		if (!sourceOffsetExists) {
			throw IllegalAccessException();
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
 	 * Get encoder.
 	 * @return encoder as vector<Angle >
 	 */
 	vector<Angle > PointingRow::getEncoder() const {
	
  		return encoder;
 	}

 	/**
 	 * Set encoder with the specified vector<Angle >.
 	 * @param encoder The vector<Angle > value to which encoder is to be set.
 	
 	 */
 	void PointingRow::setEncoder (vector<Angle > encoder) {
	
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
 	void PointingRow::setPointingTracking (bool pointingTracking) {
	
 		this->pointingTracking = pointingTracking;
	
 	}
	
	
	/**
	 * The attribute phaseTracking  is optional. Return true if this attribute exists.
	 * @return true if and only if the isPhaseTracking  attribute exists. 
	 */
	bool PointingRow::isPhaseTrackingExists() const {
		return phaseTrackingExists;
	}
	
	

	
 	/**
 	 * Get phaseTracking.
 	 * @return phaseTracking as bool
 	 */
 	bool PointingRow::getPhaseTracking() const throw(IllegalAccessException) {
	    if (!phaseTrackingExists) {
			throw IllegalAccessException();
		}
  		return phaseTracking;
 	}

 	/**
 	 * Set phaseTracking with the specified bool.
 	 * @param phaseTracking The bool value to which phaseTracking is to be set.
 	
 	 */
 	void PointingRow::setPhaseTracking (bool phaseTracking) {
	
 		this->phaseTracking = phaseTracking;
		phaseTrackingExists = true;	
 	}
	
	/**
	 * Mark phaseTracking, which is an optional field, as non-existent.
	 */
	void PointingRow::clearPhaseTracking () {
		phaseTrackingExists = false;
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
 	bool PointingRow::getOverTheTop() const throw(IllegalAccessException) {
		if (!overTheTopExists) {
			throw IllegalAccessException();
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
 	void PointingRow::setAntennaId (Tag antennaId) throw(IllegalAccessException) {
  		if (hasBeenAdded) {
			throw IllegalAccessException();
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
 	void PointingRow::setPointingModelId (int pointingModelId) {
	
 		this->pointingModelId = pointingModelId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

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
		
		nameExists = false;
		sourceOffsetExists = false;
		phaseTrackingExists = false;
		overTheTopExists = false;
	}
	
	PointingRow::PointingRow (PointingTable &t, PointingRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
		nameExists = false;
		sourceOffsetExists = false;
		phaseTrackingExists = false;
		overTheTopExists = false;
			
		}
		else {
	
		
			antennaId = row.antennaId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			pointingModelId = row.pointingModelId;
		
			numPoly = row.numPoly;
		
			timeOrigin = row.timeOrigin;
		
			pointingDirection = row.pointingDirection;
		
			target = row.target;

			offset = row.offset;
		
			encoder = row.encoder;
		
			pointingTracking = row.pointingTracking;
		
		
		if (row.nameExists) {
			name = row.name;
			nameExists = true;	
		}
		else
			nameExists = false;
		
		if (row.sourceOffsetExists) {
			sourceOffset = row.sourceOffset;		
			sourceOffsetExists = true;
		}
		else
			sourceOffsetExists = false;
			
		if (phaseTrackingExists) {
			phaseTracking = row.phaseTracking;
			phaseTrackingExists = false;
		}
		else 
			phaseTrackingExists = false;
		
		if (row.overTheTopExists) {
			overTheTop = row.overTheTop;		
			overTheTopExists = true;
		}
		else
			overTheTopExists = false;
		
		}	
	}

	
	bool PointingRow::compareNoAutoInc(Tag antennaId, ArrayTimeInterval timeInterval, int pointingModelId, int numPoly, ArrayTime timeOrigin, vector<vector<Angle > > pointingDirection, vector<vector<Angle > > target, vector<vector<Angle > > offset, vector<Angle > encoder, bool pointingTracking) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingModelId == pointingModelId);
		
		if (!result) return false;
		
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeOrigin == timeOrigin);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingDirection == pointingDirection);
		
		if (!result) return false;
	

	
		
		result = result && (this->target == target);
		
		if (!result) return false;
	

		
		result = result && (this->offset == offset);
		
		if (!result) return false;
	
		
		result = result && (this->encoder == encoder);
		
		if (!result) return false;
	

	
		
		result = result && (this->pointingTracking == pointingTracking);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool PointingRow::compareRequiredValue(int pointingModelId, int numPoly, ArrayTime timeOrigin, vector<vector<Angle > > pointingDirection, vector<vector<Angle > > target, vector<vector<Angle > > offset, vector<Angle > encoder, bool pointingTracking) {
		bool result;
		result = true;
		
	
		if (!(this->pointingModelId == pointingModelId)) return false;

	
	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->timeOrigin == timeOrigin)) return false;
	

	
		if (!(this->pointingDirection == pointingDirection)) return false;
	

	
		if (!(this->target == target)) return false;
	

		if (!(this->offset == offset)) return false;
		
	
		if (!(this->encoder == encoder)) return false;
	

	
		if (!(this->pointingTracking == pointingTracking)) return false;
	

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
		
			
		if (this->pointingModelId != x->pointingModelId) return false;
			
		if (this->name != x->name) return false;
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->timeOrigin != x->timeOrigin) return false;
			
		if (this->pointingDirection != x->pointingDirection) return false;
			
		if (this->target != x->target) return false;
			
		if (this->offset != x->offset) return false;
			
		if (this->encoder != x->encoder) return false;
			
		if (this->pointingTracking != x->pointingTracking) return false;
			
		if (this->phaseTracking != x->phaseTracking) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
