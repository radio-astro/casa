
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
 * File DelayModelRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <DelayModelRow.h>
#include <DelayModelTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::DelayModelRow;
using asdm::DelayModelTable;

using asdm::AntennaTable;
using asdm::AntennaRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	DelayModelRow::~DelayModelRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	DelayModelTable &DelayModelRow::getTable() const {
		return table;
	}
	
	void DelayModelRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a DelayModelRowIDL struct.
	 */
	DelayModelRowIDL *DelayModelRow::toIDL() const {
		DelayModelRowIDL *x = new DelayModelRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
		x->timeOrigin = timeOrigin.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		
			
		x->atmDryDelay.length(atmDryDelay.size());
		for (unsigned int i = 0; i < atmDryDelay.size(); ++i) {
			
				
			x->atmDryDelay[i] = atmDryDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->atmWetDelay.length(atmWetDelay.size());
		for (unsigned int i = 0; i < atmWetDelay.size(); ++i) {
			
				
			x->atmWetDelay[i] = atmWetDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->clockDelay.length(clockDelay.size());
		for (unsigned int i = 0; i < clockDelay.size(); ++i) {
			
				
			x->clockDelay[i] = clockDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->geomDelay.length(geomDelay.size());
		for (unsigned int i = 0; i < geomDelay.size(); ++i) {
			
				
			x->geomDelay[i] = geomDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->dispDelayExists = dispDelayExists;
		
		
			
		x->dispDelay.length(dispDelay.size());
		for (unsigned int i = 0; i < dispDelay.size(); ++i) {
			
				
			x->dispDelay[i] = dispDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->groupDelayExists = groupDelayExists;
		
		
			
		x->groupDelay.length(groupDelay.size());
		for (unsigned int i = 0; i < groupDelay.size(); ++i) {
			
				
			x->groupDelay[i] = groupDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->phaseDelayExists = phaseDelayExists;
		
		
			
		x->phaseDelay.length(phaseDelay.size());
		for (unsigned int i = 0; i < phaseDelay.size(); ++i) {
			
				
			x->phaseDelay[i] = phaseDelay.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct DelayModelRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void DelayModelRow::setFromIDL (DelayModelRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setTimeOrigin(ArrayTime (x.timeOrigin));
			
 		
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		atmDryDelay .clear();
		for (unsigned int i = 0; i <x.atmDryDelay.length(); ++i) {
			
			atmDryDelay.push_back(x.atmDryDelay[i]);
  			
		}
			
  		
		
	

	
		
		
			
		atmWetDelay .clear();
		for (unsigned int i = 0; i <x.atmWetDelay.length(); ++i) {
			
			atmWetDelay.push_back(x.atmWetDelay[i]);
  			
		}
			
  		
		
	

	
		
		
			
		clockDelay .clear();
		for (unsigned int i = 0; i <x.clockDelay.length(); ++i) {
			
			clockDelay.push_back(x.clockDelay[i]);
  			
		}
			
  		
		
	

	
		
		
			
		geomDelay .clear();
		for (unsigned int i = 0; i <x.geomDelay.length(); ++i) {
			
			geomDelay.push_back(x.geomDelay[i]);
  			
		}
			
  		
		
	

	
		
		dispDelayExists = x.dispDelayExists;
		if (x.dispDelayExists) {
		
		
			
		dispDelay .clear();
		for (unsigned int i = 0; i <x.dispDelay.length(); ++i) {
			
			dispDelay.push_back(x.dispDelay[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		groupDelayExists = x.groupDelayExists;
		if (x.groupDelayExists) {
		
		
			
		groupDelay .clear();
		for (unsigned int i = 0; i <x.groupDelay.length(); ++i) {
			
			groupDelay.push_back(x.groupDelay[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		phaseDelayExists = x.phaseDelayExists;
		if (x.phaseDelayExists) {
		
		
			
		phaseDelay .clear();
		for (unsigned int i = 0; i <x.phaseDelay.length(); ++i) {
			
			phaseDelay.push_back(x.phaseDelay[i]);
  			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"DelayModel");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string DelayModelRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeOrigin, "timeOrigin", buf);
		
		
	

  	
 		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(atmDryDelay, "atmDryDelay", buf);
		
		
	

  	
 		
		
		Parser::toXML(atmWetDelay, "atmWetDelay", buf);
		
		
	

  	
 		
		
		Parser::toXML(clockDelay, "clockDelay", buf);
		
		
	

  	
 		
		
		Parser::toXML(geomDelay, "geomDelay", buf);
		
		
	

  	
 		
		if (dispDelayExists) {
		
		
		Parser::toXML(dispDelay, "dispDelay", buf);
		
		
		}
		
	

  	
 		
		if (groupDelayExists) {
		
		
		Parser::toXML(groupDelay, "groupDelay", buf);
		
		
		}
		
	

  	
 		
		if (phaseDelayExists) {
		
		
		Parser::toXML(phaseDelay, "phaseDelay", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void DelayModelRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","DelayModel",rowDoc));
			
		
	

	
  		
			
	  	setTimeOrigin(Parser::getArrayTime("timeOrigin","DelayModel",rowDoc));
			
		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","DelayModel",rowDoc));
			
		
	

	
  		
			
					
	  	setAtmDryDelay(Parser::get1DDouble("atmDryDelay","DelayModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setAtmWetDelay(Parser::get1DDouble("atmWetDelay","DelayModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setClockDelay(Parser::get1DDouble("clockDelay","DelayModel",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setGeomDelay(Parser::get1DDouble("geomDelay","DelayModel",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<dispDelay>")) {
			
								
	  		setDispDelay(Parser::get1DDouble("dispDelay","DelayModel",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<groupDelay>")) {
			
								
	  		setGroupDelay(Parser::get1DDouble("groupDelay","DelayModel",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<phaseDelay>")) {
			
								
	  		setPhaseDelay(Parser::get1DDouble("phaseDelay","DelayModel",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"DelayModel");
		}
	}
	
	void DelayModelRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
	timeOrigin.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numPoly);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) atmDryDelay.size());
		for (unsigned int i = 0; i < atmDryDelay.size(); i++)
				
			eoss.writeDouble(atmDryDelay.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) atmWetDelay.size());
		for (unsigned int i = 0; i < atmWetDelay.size(); i++)
				
			eoss.writeDouble(atmWetDelay.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) clockDelay.size());
		for (unsigned int i = 0; i < clockDelay.size(); i++)
				
			eoss.writeDouble(clockDelay.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) geomDelay.size());
		for (unsigned int i = 0; i < geomDelay.size(); i++)
				
			eoss.writeDouble(geomDelay.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(dispDelayExists);
	if (dispDelayExists) {
	
	
	
		
		
			
		eoss.writeInt((int) dispDelay.size());
		for (unsigned int i = 0; i < dispDelay.size(); i++)
				
			eoss.writeDouble(dispDelay.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(groupDelayExists);
	if (groupDelayExists) {
	
	
	
		
		
			
		eoss.writeInt((int) groupDelay.size());
		for (unsigned int i = 0; i < groupDelay.size(); i++)
				
			eoss.writeDouble(groupDelay.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(phaseDelayExists);
	if (phaseDelayExists) {
	
	
	
		
		
			
		eoss.writeInt((int) phaseDelay.size());
		for (unsigned int i = 0; i < phaseDelay.size(); i++)
				
			eoss.writeDouble(phaseDelay.at(i));
				
				
						
		
	

	}

	}
	
void DelayModelRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void DelayModelRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void DelayModelRow::timeOriginFromBin(EndianISStream& eiss) {
		
	
		
		
		timeOrigin =  ArrayTime::fromBin(eiss);
		
	
	
}
void DelayModelRow::numPolyFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numPoly =  eiss.readInt();
			
		
	
	
}
void DelayModelRow::atmDryDelayFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		atmDryDelay.clear();
		
		unsigned int atmDryDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < atmDryDelayDim1; i++)
			
			atmDryDelay.push_back(eiss.readDouble());
			
	

		
	
	
}
void DelayModelRow::atmWetDelayFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		atmWetDelay.clear();
		
		unsigned int atmWetDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < atmWetDelayDim1; i++)
			
			atmWetDelay.push_back(eiss.readDouble());
			
	

		
	
	
}
void DelayModelRow::clockDelayFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		clockDelay.clear();
		
		unsigned int clockDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < clockDelayDim1; i++)
			
			clockDelay.push_back(eiss.readDouble());
			
	

		
	
	
}
void DelayModelRow::geomDelayFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		geomDelay.clear();
		
		unsigned int geomDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < geomDelayDim1; i++)
			
			geomDelay.push_back(eiss.readDouble());
			
	

		
	
	
}

void DelayModelRow::dispDelayFromBin(EndianISStream& eiss) {
		
	dispDelayExists = eiss.readBoolean();
	if (dispDelayExists) {
		
	
	
		
			
	
		dispDelay.clear();
		
		unsigned int dispDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < dispDelayDim1; i++)
			
			dispDelay.push_back(eiss.readDouble());
			
	

		
	

	}
	
}
void DelayModelRow::groupDelayFromBin(EndianISStream& eiss) {
		
	groupDelayExists = eiss.readBoolean();
	if (groupDelayExists) {
		
	
	
		
			
	
		groupDelay.clear();
		
		unsigned int groupDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < groupDelayDim1; i++)
			
			groupDelay.push_back(eiss.readDouble());
			
	

		
	

	}
	
}
void DelayModelRow::phaseDelayFromBin(EndianISStream& eiss) {
		
	phaseDelayExists = eiss.readBoolean();
	if (phaseDelayExists) {
		
	
	
		
			
	
		phaseDelay.clear();
		
		unsigned int phaseDelayDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < phaseDelayDim1; i++)
			
			phaseDelay.push_back(eiss.readDouble());
			
	

		
	

	}
	
}
	
	
	DelayModelRow* DelayModelRow::fromBin(EndianISStream& eiss, DelayModelTable& table, const vector<string>& attributesSeq) {
		DelayModelRow* row = new  DelayModelRow(table);
		
		map<string, DelayModelAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "DelayModelTable");
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
 	ArrayTimeInterval DelayModelRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void DelayModelRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "DelayModel");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get timeOrigin.
 	 * @return timeOrigin as ArrayTime
 	 */
 	ArrayTime DelayModelRow::getTimeOrigin() const {
	
  		return timeOrigin;
 	}

 	/**
 	 * Set timeOrigin with the specified ArrayTime.
 	 * @param timeOrigin The ArrayTime value to which timeOrigin is to be set.
 	 
 	
 		
 	 */
 	void DelayModelRow::setTimeOrigin (ArrayTime timeOrigin)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->timeOrigin = timeOrigin;
	
 	}
	
	

	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int DelayModelRow::getNumPoly() const {
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 	
 		
 	 */
 	void DelayModelRow::setNumPoly (int numPoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numPoly = numPoly;
	
 	}
	
	

	

	
 	/**
 	 * Get atmDryDelay.
 	 * @return atmDryDelay as vector<double >
 	 */
 	vector<double > DelayModelRow::getAtmDryDelay() const {
	
  		return atmDryDelay;
 	}

 	/**
 	 * Set atmDryDelay with the specified vector<double >.
 	 * @param atmDryDelay The vector<double > value to which atmDryDelay is to be set.
 	 
 	
 		
 	 */
 	void DelayModelRow::setAtmDryDelay (vector<double > atmDryDelay)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmDryDelay = atmDryDelay;
	
 	}
	
	

	

	
 	/**
 	 * Get atmWetDelay.
 	 * @return atmWetDelay as vector<double >
 	 */
 	vector<double > DelayModelRow::getAtmWetDelay() const {
	
  		return atmWetDelay;
 	}

 	/**
 	 * Set atmWetDelay with the specified vector<double >.
 	 * @param atmWetDelay The vector<double > value to which atmWetDelay is to be set.
 	 
 	
 		
 	 */
 	void DelayModelRow::setAtmWetDelay (vector<double > atmWetDelay)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->atmWetDelay = atmWetDelay;
	
 	}
	
	

	

	
 	/**
 	 * Get clockDelay.
 	 * @return clockDelay as vector<double >
 	 */
 	vector<double > DelayModelRow::getClockDelay() const {
	
  		return clockDelay;
 	}

 	/**
 	 * Set clockDelay with the specified vector<double >.
 	 * @param clockDelay The vector<double > value to which clockDelay is to be set.
 	 
 	
 		
 	 */
 	void DelayModelRow::setClockDelay (vector<double > clockDelay)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->clockDelay = clockDelay;
	
 	}
	
	

	

	
 	/**
 	 * Get geomDelay.
 	 * @return geomDelay as vector<double >
 	 */
 	vector<double > DelayModelRow::getGeomDelay() const {
	
  		return geomDelay;
 	}

 	/**
 	 * Set geomDelay with the specified vector<double >.
 	 * @param geomDelay The vector<double > value to which geomDelay is to be set.
 	 
 	
 		
 	 */
 	void DelayModelRow::setGeomDelay (vector<double > geomDelay)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->geomDelay = geomDelay;
	
 	}
	
	

	
	/**
	 * The attribute dispDelay is optional. Return true if this attribute exists.
	 * @return true if and only if the dispDelay attribute exists. 
	 */
	bool DelayModelRow::isDispDelayExists() const {
		return dispDelayExists;
	}
	

	
 	/**
 	 * Get dispDelay, which is optional.
 	 * @return dispDelay as vector<double >
 	 * @throw IllegalAccessException If dispDelay does not exist.
 	 */
 	vector<double > DelayModelRow::getDispDelay() const  {
		if (!dispDelayExists) {
			throw IllegalAccessException("dispDelay", "DelayModel");
		}
	
  		return dispDelay;
 	}

 	/**
 	 * Set dispDelay with the specified vector<double >.
 	 * @param dispDelay The vector<double > value to which dispDelay is to be set.
 	 
 	
 	 */
 	void DelayModelRow::setDispDelay (vector<double > dispDelay) {
	
 		this->dispDelay = dispDelay;
	
		dispDelayExists = true;
	
 	}
	
	
	/**
	 * Mark dispDelay, which is an optional field, as non-existent.
	 */
	void DelayModelRow::clearDispDelay () {
		dispDelayExists = false;
	}
	

	
	/**
	 * The attribute groupDelay is optional. Return true if this attribute exists.
	 * @return true if and only if the groupDelay attribute exists. 
	 */
	bool DelayModelRow::isGroupDelayExists() const {
		return groupDelayExists;
	}
	

	
 	/**
 	 * Get groupDelay, which is optional.
 	 * @return groupDelay as vector<double >
 	 * @throw IllegalAccessException If groupDelay does not exist.
 	 */
 	vector<double > DelayModelRow::getGroupDelay() const  {
		if (!groupDelayExists) {
			throw IllegalAccessException("groupDelay", "DelayModel");
		}
	
  		return groupDelay;
 	}

 	/**
 	 * Set groupDelay with the specified vector<double >.
 	 * @param groupDelay The vector<double > value to which groupDelay is to be set.
 	 
 	
 	 */
 	void DelayModelRow::setGroupDelay (vector<double > groupDelay) {
	
 		this->groupDelay = groupDelay;
	
		groupDelayExists = true;
	
 	}
	
	
	/**
	 * Mark groupDelay, which is an optional field, as non-existent.
	 */
	void DelayModelRow::clearGroupDelay () {
		groupDelayExists = false;
	}
	

	
	/**
	 * The attribute phaseDelay is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseDelay attribute exists. 
	 */
	bool DelayModelRow::isPhaseDelayExists() const {
		return phaseDelayExists;
	}
	

	
 	/**
 	 * Get phaseDelay, which is optional.
 	 * @return phaseDelay as vector<double >
 	 * @throw IllegalAccessException If phaseDelay does not exist.
 	 */
 	vector<double > DelayModelRow::getPhaseDelay() const  {
		if (!phaseDelayExists) {
			throw IllegalAccessException("phaseDelay", "DelayModel");
		}
	
  		return phaseDelay;
 	}

 	/**
 	 * Set phaseDelay with the specified vector<double >.
 	 * @param phaseDelay The vector<double > value to which phaseDelay is to be set.
 	 
 	
 	 */
 	void DelayModelRow::setPhaseDelay (vector<double > phaseDelay) {
	
 		this->phaseDelay = phaseDelay;
	
		phaseDelayExists = true;
	
 	}
	
	
	/**
	 * Mark phaseDelay, which is an optional field, as non-existent.
	 */
	void DelayModelRow::clearPhaseDelay () {
		phaseDelayExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag DelayModelRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void DelayModelRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "DelayModel");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* DelayModelRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	/**
	 * Create a DelayModelRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	DelayModelRow::DelayModelRow (DelayModelTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	
		dispDelayExists = false;
	

	
		groupDelayExists = false;
	

	
		phaseDelayExists = false;
	

	
	

	
	
	
	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaId"] = &DelayModelRow::antennaIdFromBin; 
	 fromBinMethods["timeInterval"] = &DelayModelRow::timeIntervalFromBin; 
	 fromBinMethods["timeOrigin"] = &DelayModelRow::timeOriginFromBin; 
	 fromBinMethods["numPoly"] = &DelayModelRow::numPolyFromBin; 
	 fromBinMethods["atmDryDelay"] = &DelayModelRow::atmDryDelayFromBin; 
	 fromBinMethods["atmWetDelay"] = &DelayModelRow::atmWetDelayFromBin; 
	 fromBinMethods["clockDelay"] = &DelayModelRow::clockDelayFromBin; 
	 fromBinMethods["geomDelay"] = &DelayModelRow::geomDelayFromBin; 
		
	
	 fromBinMethods["dispDelay"] = &DelayModelRow::dispDelayFromBin; 
	 fromBinMethods["groupDelay"] = &DelayModelRow::groupDelayFromBin; 
	 fromBinMethods["phaseDelay"] = &DelayModelRow::phaseDelayFromBin; 
	
	}
	
	DelayModelRow::DelayModelRow (DelayModelTable &t, DelayModelRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	
		dispDelayExists = false;
	

	
		groupDelayExists = false;
	

	
		phaseDelayExists = false;
	

	
	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			timeOrigin = row.timeOrigin;
		
			numPoly = row.numPoly;
		
			atmDryDelay = row.atmDryDelay;
		
			atmWetDelay = row.atmWetDelay;
		
			clockDelay = row.clockDelay;
		
			geomDelay = row.geomDelay;
		
		
		
		
		if (row.dispDelayExists) {
			dispDelay = row.dispDelay;		
			dispDelayExists = true;
		}
		else
			dispDelayExists = false;
		
		if (row.groupDelayExists) {
			groupDelay = row.groupDelay;		
			groupDelayExists = true;
		}
		else
			groupDelayExists = false;
		
		if (row.phaseDelayExists) {
			phaseDelay = row.phaseDelay;		
			phaseDelayExists = true;
		}
		else
			phaseDelayExists = false;
		
		}
		
		 fromBinMethods["antennaId"] = &DelayModelRow::antennaIdFromBin; 
		 fromBinMethods["timeInterval"] = &DelayModelRow::timeIntervalFromBin; 
		 fromBinMethods["timeOrigin"] = &DelayModelRow::timeOriginFromBin; 
		 fromBinMethods["numPoly"] = &DelayModelRow::numPolyFromBin; 
		 fromBinMethods["atmDryDelay"] = &DelayModelRow::atmDryDelayFromBin; 
		 fromBinMethods["atmWetDelay"] = &DelayModelRow::atmWetDelayFromBin; 
		 fromBinMethods["clockDelay"] = &DelayModelRow::clockDelayFromBin; 
		 fromBinMethods["geomDelay"] = &DelayModelRow::geomDelayFromBin; 
			
	
		 fromBinMethods["dispDelay"] = &DelayModelRow::dispDelayFromBin; 
		 fromBinMethods["groupDelay"] = &DelayModelRow::groupDelayFromBin; 
		 fromBinMethods["phaseDelay"] = &DelayModelRow::phaseDelayFromBin; 
			
	}

	
	bool DelayModelRow::compareNoAutoInc(Tag antennaId, ArrayTimeInterval timeInterval, ArrayTime timeOrigin, int numPoly, vector<double > atmDryDelay, vector<double > atmWetDelay, vector<double > clockDelay, vector<double > geomDelay) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->timeOrigin == timeOrigin);
		
		if (!result) return false;
	

	
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmDryDelay == atmDryDelay);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmWetDelay == atmWetDelay);
		
		if (!result) return false;
	

	
		
		result = result && (this->clockDelay == clockDelay);
		
		if (!result) return false;
	

	
		
		result = result && (this->geomDelay == geomDelay);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool DelayModelRow::compareRequiredValue(ArrayTime timeOrigin, int numPoly, vector<double > atmDryDelay, vector<double > atmWetDelay, vector<double > clockDelay, vector<double > geomDelay) {
		bool result;
		result = true;
		
	
		if (!(this->timeOrigin == timeOrigin)) return false;
	

	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->atmDryDelay == atmDryDelay)) return false;
	

	
		if (!(this->atmWetDelay == atmWetDelay)) return false;
	

	
		if (!(this->clockDelay == clockDelay)) return false;
	

	
		if (!(this->geomDelay == geomDelay)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the DelayModelRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool DelayModelRow::equalByRequiredValue(DelayModelRow* x) {
		
			
		if (this->timeOrigin != x->timeOrigin) return false;
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->atmDryDelay != x->atmDryDelay) return false;
			
		if (this->atmWetDelay != x->atmWetDelay) return false;
			
		if (this->clockDelay != x->clockDelay) return false;
			
		if (this->geomDelay != x->geomDelay) return false;
			
		
		return true;
	}	
	
/*
	 map<string, DelayModelAttributeFromBin> DelayModelRow::initFromBinMethods() {
		map<string, DelayModelAttributeFromBin> result;
		
		result["antennaId"] = &DelayModelRow::antennaIdFromBin;
		result["timeInterval"] = &DelayModelRow::timeIntervalFromBin;
		result["timeOrigin"] = &DelayModelRow::timeOriginFromBin;
		result["numPoly"] = &DelayModelRow::numPolyFromBin;
		result["atmDryDelay"] = &DelayModelRow::atmDryDelayFromBin;
		result["atmWetDelay"] = &DelayModelRow::atmWetDelayFromBin;
		result["clockDelay"] = &DelayModelRow::clockDelayFromBin;
		result["geomDelay"] = &DelayModelRow::geomDelayFromBin;
		
		
		result["dispDelay"] = &DelayModelRow::dispDelayFromBin;
		result["groupDelay"] = &DelayModelRow::groupDelayFromBin;
		result["phaseDelay"] = &DelayModelRow::phaseDelayFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
