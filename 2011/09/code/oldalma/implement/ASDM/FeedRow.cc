
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
 * File FeedRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <FeedRow.h>
#include <FeedTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <ReceiverTable.h>
#include <ReceiverRow.h>

#include <BeamTable.h>
#include <BeamRow.h>
	

using asdm::ASDM;
using asdm::FeedRow;
using asdm::FeedTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::ReceiverTable;
using asdm::ReceiverRow;

using asdm::BeamTable;
using asdm::BeamRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	FeedRow::~FeedRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	FeedTable &FeedRow::getTable() const {
		return table;
	}
	
	void FeedRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FeedRowIDL struct.
	 */
	FeedRowIDL *FeedRow::toIDL() const {
		FeedRowIDL *x = new FeedRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->feedId = feedId;
 				
 			
		
	

	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		x->feedNumExists = feedNumExists;
		
		
			
				
		x->feedNum = feedNum;
 				
 			
		
	

	
  		
		
		
			
		x->beamOffset.length(beamOffset.size());
		for (unsigned int i = 0; i < beamOffset.size(); i++) {
			x->beamOffset[i].length(beamOffset.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < beamOffset.size() ; i++)
			for (unsigned int j = 0; j < beamOffset.at(i).size(); j++)
					
						
				x->beamOffset[i][j] = beamOffset.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
		x->focusReference.length(focusReference.size());
		for (unsigned int i = 0; i < focusReference.size(); i++) {
			x->focusReference[i].length(focusReference.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < focusReference.size() ; i++)
			for (unsigned int j = 0; j < focusReference.at(i).size(); j++)
					
				x->focusReference[i][j]= focusReference.at(i).at(j).toIDLLength();
									
		
			
		
	

	
  		
		
		x->illumOffsetExists = illumOffsetExists;
		
		
			
				
		x->illumOffset = illumOffset;
 				
 			
		
	

	
  		
		
		x->illumOffsetPaExists = illumOffsetPaExists;
		
		
			
				
		x->illumOffsetPa = illumOffsetPa;
 				
 			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->polResponse.length(polResponse.size());
		for (unsigned int i = 0; i < polResponse.size(); i++) {
			x->polResponse[i].length(polResponse.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < polResponse.size() ; i++)
			for (unsigned int j = 0; j < polResponse.at(i).size(); j++)
					
				x->polResponse[i][j]= polResponse.at(i).at(j).toIDLComplex();
									
		
			
		
	

	
  		
		
		x->xPositionExists = xPositionExists;
		
		
			
		x->xPosition = xPosition.toIDLLength();
			
		
	

	
  		
		
		x->yPositionExists = yPositionExists;
		
		
			
		x->yPosition = yPosition.toIDLLength();
			
		
	

	
  		
		
		x->zPositionExists = zPositionExists;
		
		
			
		x->zPosition = zPosition.toIDLLength();
			
		
	

	
  		
		
		
			
		x->receptorAngle.length(receptorAngle.size());
		for (unsigned int i = 0; i < receptorAngle.size(); ++i) {
			
			x->receptorAngle[i] = receptorAngle.at(i).toIDLAngle();
			
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
 		
		x->beamIdExists = beamIdExists;
		
		
		
		x->beamId.length(beamId.size());
		for (unsigned int i = 0; i < beamId.size(); ++i) {
			
			x->beamId[i] = beamId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
  	
 		
		
		
		x->receiverId.length(receiverId.size());
		for (unsigned int i = 0; i < receiverId.size(); ++i) {
			
				
			x->receiverId[i] = receiverId.at(i);
	 			
	 		
	 	}
	 	 		
  	

	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FeedRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void FeedRow::setFromIDL (FeedRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		feedNumExists = x.feedNumExists;
		if (x.feedNumExists) {
		
		
			
		setFeedNum(x.feedNum);
  			
 		
		
		}
		
	

	
		
		
			
		beamOffset .clear();
		vector<double> v_aux_beamOffset;
		for (unsigned int i = 0; i < x.beamOffset.length(); ++i) {
			v_aux_beamOffset.clear();
			for (unsigned int j = 0; j < x.beamOffset[0].length(); ++j) {
				
				v_aux_beamOffset.push_back(x.beamOffset[i][j]);
	  			
  			}
  			beamOffset.push_back(v_aux_beamOffset);			
		}
			
  		
		
	

	
		
		
			
		focusReference .clear();
		vector<Length> v_aux_focusReference;
		for (unsigned int i = 0; i < x.focusReference.length(); ++i) {
			v_aux_focusReference.clear();
			for (unsigned int j = 0; j < x.focusReference[0].length(); ++j) {
				
				v_aux_focusReference.push_back(Length (x.focusReference[i][j]));
				
  			}
  			focusReference.push_back(v_aux_focusReference);			
		}
			
  		
		
	

	
		
		illumOffsetExists = x.illumOffsetExists;
		if (x.illumOffsetExists) {
		
		
			
		setIllumOffset(x.illumOffset);
  			
 		
		
		}
		
	

	
		
		illumOffsetPaExists = x.illumOffsetPaExists;
		if (x.illumOffsetPaExists) {
		
		
			
		setIllumOffsetPa(x.illumOffsetPa);
  			
 		
		
		}
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		polResponse .clear();
		vector<Complex> v_aux_polResponse;
		for (unsigned int i = 0; i < x.polResponse.length(); ++i) {
			v_aux_polResponse.clear();
			for (unsigned int j = 0; j < x.polResponse[0].length(); ++j) {
				
				v_aux_polResponse.push_back(Complex (x.polResponse[i][j]));
				
  			}
  			polResponse.push_back(v_aux_polResponse);			
		}
			
  		
		
	

	
		
		xPositionExists = x.xPositionExists;
		if (x.xPositionExists) {
		
		
			
		setXPosition(Length (x.xPosition));
			
 		
		
		}
		
	

	
		
		yPositionExists = x.yPositionExists;
		if (x.yPositionExists) {
		
		
			
		setYPosition(Length (x.yPosition));
			
 		
		
		}
		
	

	
		
		zPositionExists = x.zPositionExists;
		if (x.zPositionExists) {
		
		
			
		setZPosition(Length (x.zPosition));
			
 		
		
		}
		
	

	
		
		
			
		receptorAngle .clear();
		for (unsigned int i = 0; i <x.receptorAngle.length(); ++i) {
			
			receptorAngle.push_back(Angle (x.receptorAngle[i]));
			
		}
			
  		
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		beamIdExists = x.beamIdExists;
		if (x.beamIdExists) {
		
		beamId .clear();
		for (unsigned int i = 0; i <x.beamId.length(); ++i) {
			
			beamId.push_back(Tag (x.beamId[i]));
			
		}
		
		}
		
  	

	
		
		receiverId .clear();
		for (unsigned int i = 0; i <x.receiverId.length(); ++i) {
			
			receiverId.push_back(x.receiverId[i]);
  			
		}
		
  	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"Feed");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string FeedRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(feedId, "feedId", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		if (feedNumExists) {
		
		
		Parser::toXML(feedNum, "feedNum", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(beamOffset, "beamOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusReference, "focusReference", buf);
		
		
	

  	
 		
		if (illumOffsetExists) {
		
		
		Parser::toXML(illumOffset, "illumOffset", buf);
		
		
		}
		
	

  	
 		
		if (illumOffsetPaExists) {
		
		
		Parser::toXML(illumOffsetPa, "illumOffsetPa", buf);
		
		
		}
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(polResponse, "polResponse", buf);
		
		
	

  	
 		
		if (xPositionExists) {
		
		
		Parser::toXML(xPosition, "xPosition", buf);
		
		
		}
		
	

  	
 		
		if (yPositionExists) {
		
		
		Parser::toXML(yPosition, "yPosition", buf);
		
		
		}
		
	

  	
 		
		if (zPositionExists) {
		
		
		Parser::toXML(zPosition, "zPosition", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(receptorAngle, "receptorAngle", buf);
		
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		if (beamIdExists) {
		
		
		Parser::toXML(beamId, "beamId", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(receiverId, "receiverId", buf);
		
		
	

  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void FeedRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Feed",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","Feed",rowDoc));
			
		
	

	
  		
        if (row.isStr("<feedNum>")) {
			
	  		setFeedNum(Parser::getInteger("feedNum","Feed",rowDoc));
			
		}
 		
	

	
  		
			
					
	  	setBeamOffset(Parser::get2DDouble("beamOffset","Feed",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFocusReference(Parser::get2DLength("focusReference","Feed",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<illumOffset>")) {
			
	  		setIllumOffset(Parser::getFloat("illumOffset","Feed",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<illumOffsetPa>")) {
			
	  		setIllumOffsetPa(Parser::getFloat("illumOffsetPa","Feed",rowDoc));
			
		}
 		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","Feed",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setPolResponse(Parser::get2DComplex("polResponse","Feed",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<xPosition>")) {
			
	  		setXPosition(Parser::getLength("xPosition","Feed",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<yPosition>")) {
			
	  		setYPosition(Parser::getLength("yPosition","Feed",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<zPosition>")) {
			
	  		setZPosition(Parser::getLength("zPosition","Feed",rowDoc));
			
		}
 		
	

	
  		
			
					
	  	setReceptorAngle(Parser::get1DAngle("receptorAngle","Feed",rowDoc));
	  			
	  		
		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
  		if (row.isStr("<beamId>")) {
  			setBeamId(Parser::get1DTag("beamId","Feed",rowDoc));  		
  		}
  		
  	

	
  		 
  		setReceiverId(Parser::get1DInteger("receiverId","Feed",rowDoc));
		
  	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Feed");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int FeedRow::getFeedId() const {
	
  		return feedId;
 	}

 	/**
 	 * Set feedId with the specified int.
 	 * @param feedId The int value to which feedId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FeedRow::setFeedId (int feedId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("feedId", "Feed");
		
  		}
  	
 		this->feedId = feedId;
	
 	}
	
	

	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval FeedRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FeedRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "Feed");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int FeedRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void FeedRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	
	/**
	 * The attribute feedNum is optional. Return true if this attribute exists.
	 * @return true if and only if the feedNum attribute exists. 
	 */
	bool FeedRow::isFeedNumExists() const {
		return feedNumExists;
	}
	

	
 	/**
 	 * Get feedNum, which is optional.
 	 * @return feedNum as int
 	 * @throw IllegalAccessException If feedNum does not exist.
 	 */
 	int FeedRow::getFeedNum() const throw(IllegalAccessException) {
		if (!feedNumExists) {
			throw IllegalAccessException("feedNum", "Feed");
		}
	
  		return feedNum;
 	}

 	/**
 	 * Set feedNum with the specified int.
 	 * @param feedNum The int value to which feedNum is to be set.
 	 
 	
 	 */
 	void FeedRow::setFeedNum (int feedNum) {
	
 		this->feedNum = feedNum;
	
		feedNumExists = true;
	
 	}
	
	
	/**
	 * Mark feedNum, which is an optional field, as non-existent.
	 */
	void FeedRow::clearFeedNum () {
		feedNumExists = false;
	}
	

	

	
 	/**
 	 * Get beamOffset.
 	 * @return beamOffset as vector<vector<double > >
 	 */
 	vector<vector<double > > FeedRow::getBeamOffset() const {
	
  		return beamOffset;
 	}

 	/**
 	 * Set beamOffset with the specified vector<vector<double > >.
 	 * @param beamOffset The vector<vector<double > > value to which beamOffset is to be set.
 	 
 	
 		
 	 */
 	void FeedRow::setBeamOffset (vector<vector<double > > beamOffset)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->beamOffset = beamOffset;
	
 	}
	
	

	

	
 	/**
 	 * Get focusReference.
 	 * @return focusReference as vector<vector<Length > >
 	 */
 	vector<vector<Length > > FeedRow::getFocusReference() const {
	
  		return focusReference;
 	}

 	/**
 	 * Set focusReference with the specified vector<vector<Length > >.
 	 * @param focusReference The vector<vector<Length > > value to which focusReference is to be set.
 	 
 	
 		
 	 */
 	void FeedRow::setFocusReference (vector<vector<Length > > focusReference)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->focusReference = focusReference;
	
 	}
	
	

	
	/**
	 * The attribute illumOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the illumOffset attribute exists. 
	 */
	bool FeedRow::isIllumOffsetExists() const {
		return illumOffsetExists;
	}
	

	
 	/**
 	 * Get illumOffset, which is optional.
 	 * @return illumOffset as float
 	 * @throw IllegalAccessException If illumOffset does not exist.
 	 */
 	float FeedRow::getIllumOffset() const throw(IllegalAccessException) {
		if (!illumOffsetExists) {
			throw IllegalAccessException("illumOffset", "Feed");
		}
	
  		return illumOffset;
 	}

 	/**
 	 * Set illumOffset with the specified float.
 	 * @param illumOffset The float value to which illumOffset is to be set.
 	 
 	
 	 */
 	void FeedRow::setIllumOffset (float illumOffset) {
	
 		this->illumOffset = illumOffset;
	
		illumOffsetExists = true;
	
 	}
	
	
	/**
	 * Mark illumOffset, which is an optional field, as non-existent.
	 */
	void FeedRow::clearIllumOffset () {
		illumOffsetExists = false;
	}
	

	
	/**
	 * The attribute illumOffsetPa is optional. Return true if this attribute exists.
	 * @return true if and only if the illumOffsetPa attribute exists. 
	 */
	bool FeedRow::isIllumOffsetPaExists() const {
		return illumOffsetPaExists;
	}
	

	
 	/**
 	 * Get illumOffsetPa, which is optional.
 	 * @return illumOffsetPa as float
 	 * @throw IllegalAccessException If illumOffsetPa does not exist.
 	 */
 	float FeedRow::getIllumOffsetPa() const throw(IllegalAccessException) {
		if (!illumOffsetPaExists) {
			throw IllegalAccessException("illumOffsetPa", "Feed");
		}
	
  		return illumOffsetPa;
 	}

 	/**
 	 * Set illumOffsetPa with the specified float.
 	 * @param illumOffsetPa The float value to which illumOffsetPa is to be set.
 	 
 	
 	 */
 	void FeedRow::setIllumOffsetPa (float illumOffsetPa) {
	
 		this->illumOffsetPa = illumOffsetPa;
	
		illumOffsetPaExists = true;
	
 	}
	
	
	/**
	 * Mark illumOffsetPa, which is an optional field, as non-existent.
	 */
	void FeedRow::clearIllumOffsetPa () {
		illumOffsetPaExists = false;
	}
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > FeedRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void FeedRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get polResponse.
 	 * @return polResponse as vector<vector<Complex > >
 	 */
 	vector<vector<Complex > > FeedRow::getPolResponse() const {
	
  		return polResponse;
 	}

 	/**
 	 * Set polResponse with the specified vector<vector<Complex > >.
 	 * @param polResponse The vector<vector<Complex > > value to which polResponse is to be set.
 	 
 	
 		
 	 */
 	void FeedRow::setPolResponse (vector<vector<Complex > > polResponse)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polResponse = polResponse;
	
 	}
	
	

	
	/**
	 * The attribute xPosition is optional. Return true if this attribute exists.
	 * @return true if and only if the xPosition attribute exists. 
	 */
	bool FeedRow::isXPositionExists() const {
		return xPositionExists;
	}
	

	
 	/**
 	 * Get xPosition, which is optional.
 	 * @return xPosition as Length
 	 * @throw IllegalAccessException If xPosition does not exist.
 	 */
 	Length FeedRow::getXPosition() const throw(IllegalAccessException) {
		if (!xPositionExists) {
			throw IllegalAccessException("xPosition", "Feed");
		}
	
  		return xPosition;
 	}

 	/**
 	 * Set xPosition with the specified Length.
 	 * @param xPosition The Length value to which xPosition is to be set.
 	 
 	
 	 */
 	void FeedRow::setXPosition (Length xPosition) {
	
 		this->xPosition = xPosition;
	
		xPositionExists = true;
	
 	}
	
	
	/**
	 * Mark xPosition, which is an optional field, as non-existent.
	 */
	void FeedRow::clearXPosition () {
		xPositionExists = false;
	}
	

	
	/**
	 * The attribute yPosition is optional. Return true if this attribute exists.
	 * @return true if and only if the yPosition attribute exists. 
	 */
	bool FeedRow::isYPositionExists() const {
		return yPositionExists;
	}
	

	
 	/**
 	 * Get yPosition, which is optional.
 	 * @return yPosition as Length
 	 * @throw IllegalAccessException If yPosition does not exist.
 	 */
 	Length FeedRow::getYPosition() const throw(IllegalAccessException) {
		if (!yPositionExists) {
			throw IllegalAccessException("yPosition", "Feed");
		}
	
  		return yPosition;
 	}

 	/**
 	 * Set yPosition with the specified Length.
 	 * @param yPosition The Length value to which yPosition is to be set.
 	 
 	
 	 */
 	void FeedRow::setYPosition (Length yPosition) {
	
 		this->yPosition = yPosition;
	
		yPositionExists = true;
	
 	}
	
	
	/**
	 * Mark yPosition, which is an optional field, as non-existent.
	 */
	void FeedRow::clearYPosition () {
		yPositionExists = false;
	}
	

	
	/**
	 * The attribute zPosition is optional. Return true if this attribute exists.
	 * @return true if and only if the zPosition attribute exists. 
	 */
	bool FeedRow::isZPositionExists() const {
		return zPositionExists;
	}
	

	
 	/**
 	 * Get zPosition, which is optional.
 	 * @return zPosition as Length
 	 * @throw IllegalAccessException If zPosition does not exist.
 	 */
 	Length FeedRow::getZPosition() const throw(IllegalAccessException) {
		if (!zPositionExists) {
			throw IllegalAccessException("zPosition", "Feed");
		}
	
  		return zPosition;
 	}

 	/**
 	 * Set zPosition with the specified Length.
 	 * @param zPosition The Length value to which zPosition is to be set.
 	 
 	
 	 */
 	void FeedRow::setZPosition (Length zPosition) {
	
 		this->zPosition = zPosition;
	
		zPositionExists = true;
	
 	}
	
	
	/**
	 * Mark zPosition, which is an optional field, as non-existent.
	 */
	void FeedRow::clearZPosition () {
		zPositionExists = false;
	}
	

	

	
 	/**
 	 * Get receptorAngle.
 	 * @return receptorAngle as vector<Angle >
 	 */
 	vector<Angle > FeedRow::getReceptorAngle() const {
	
  		return receptorAngle;
 	}

 	/**
 	 * Set receptorAngle with the specified vector<Angle >.
 	 * @param receptorAngle The vector<Angle > value to which receptorAngle is to be set.
 	 
 	
 		
 	 */
 	void FeedRow::setReceptorAngle (vector<Angle > receptorAngle)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receptorAngle = receptorAngle;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag FeedRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FeedRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "Feed");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	
	/**
	 * The attribute beamId is optional. Return true if this attribute exists.
	 * @return true if and only if the beamId attribute exists. 
	 */
	bool FeedRow::isBeamIdExists() const {
		return beamIdExists;
	}
	

	
 	/**
 	 * Get beamId, which is optional.
 	 * @return beamId as vector<Tag> 
 	 * @throw IllegalAccessException If beamId does not exist.
 	 */
 	vector<Tag>  FeedRow::getBeamId() const throw(IllegalAccessException) {
		if (!beamIdExists) {
			throw IllegalAccessException("beamId", "Feed");
		}
	
  		return beamId;
 	}

 	/**
 	 * Set beamId with the specified vector<Tag> .
 	 * @param beamId The vector<Tag>  value to which beamId is to be set.
 	 
 	
 	 */
 	void FeedRow::setBeamId (vector<Tag>  beamId) {
	
 		this->beamId = beamId;
	
		beamIdExists = true;
	
 	}
	
	
	/**
	 * Mark beamId, which is an optional field, as non-existent.
	 */
	void FeedRow::clearBeamId () {
		beamIdExists = false;
	}
	

	

	
 	/**
 	 * Get receiverId.
 	 * @return receiverId as vector<int> 
 	 */
 	vector<int>  FeedRow::getReceiverId() const {
	
  		return receiverId;
 	}

 	/**
 	 * Set receiverId with the specified vector<int> .
 	 * @param receiverId The vector<int>  value to which receiverId is to be set.
 	 
 	
 		
 	 */
 	void FeedRow::setReceiverId (vector<int>  receiverId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->receiverId = receiverId;
	
 	}
	
	

	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag FeedRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FeedRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "Feed");
		
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
	 AntennaRow* FeedRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* FeedRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
 		
 	/**
 	 * Set receiverId[i] with the specified int.
 	 * @param i The index in receiverId where to set the int value.
 	 * @param receiverId The int value to which receiverId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void FeedRow::setReceiverId (int i, int receiverId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->receiverId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute receiverId in table FeedTable");
  		vector<int> ::iterator iter = this->receiverId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->receiverId.insert(this->receiverId.erase(iter), receiverId); 
  	}	
 			
	
	
	
		

	// ===> Slices link from a row of Feed table to a collection of row of Receiver table.	
	// vector <int> receiverId;
	
	/*
	 ** Append a new id to receiverId
	 */
	void FeedRow::addReceiverId(int id) {
		receiverId.push_back(id);
	}
	
	/*
	 ** Append an array of ids to receiverId
	 */ 
	void FeedRow::addReceiverId(vector<int> id) {
		for (unsigned int i = 0; i < id.size(); i++) 
			receiverId.push_back(id[i]);
	}
	/**
	 * Get the collection of rows in the Receiver table having receiverId == receiverId[i]
	 */	 
	const vector <ReceiverRow *> FeedRow::getReceivers(int i) {
		
			return table.getContainer().getReceiver().getRowByReceiverId(receiverId.at(i));
				
	}
	
	/** 
	 * Get the collection of pointers to rows in the Receiver table having receiverId == receiverId[i]
	 * for any i in [O..receiverId.size()-1].
	 */
	const vector <ReceiverRow *> FeedRow::getReceivers() {
		
			vector <ReceiverRow *> result;
			for (unsigned int i=0; i < receiverId.size(); i++) {
				vector <ReceiverRow *> current = table.getContainer().getReceiver().getRowByReceiverId(receiverId.at(i));
				for (unsigned int j = 0; j < current.size(); j++) 
					result.push_back(current.at(j));
			}					
			return result;
	}


	

	
 		
 	/**
 	 * Set beamId[i] with the specified Tag.
 	 * @param i The index in beamId where to set the Tag value.
 	 * @param beamId The Tag value to which beamId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void FeedRow::setBeamId (int i, Tag beamId) {
  		if ((i < 0) || (i > ((int) this->beamId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute beamId in table FeedTable");
  		vector<Tag> ::iterator iter = this->beamId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->beamId.insert(this->beamId.erase(iter), beamId); 	
  	}
 			
	
	
	
		
/**
 * Append a Tag to beamId.
 * @param id the Tag to be appended to beamId
 */
 void FeedRow::addBeamId(Tag id){
 	beamId.push_back(id);
}

/**
 * Append an array of Tag to beamId.
 * @param id an array of Tag to be appended to beamId
 */
 void FeedRow::addBeamId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		beamId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in beamId at position i.
  *
  */
 const Tag FeedRow::getBeamId(int i) {
 	return beamId.at(i);
 }
 
 /**
  * Returns the BeamRow linked to this row via the Tag stored in beamId
  * at position i.
  */
 BeamRow* FeedRow::getBeam(int i) {
 	return table.getContainer().getBeam().getRowByKey(beamId.at(i));
 } 
 
 /**
  * Returns the vector of BeamRow* linked to this row via the Tags stored in beamId
  *
  */
 vector<BeamRow *> FeedRow::getBeams() {
 	vector<BeamRow *> result;
 	for (unsigned int i = 0; i < beamId.size(); i++)
 		result.push_back(table.getContainer().getBeam().getRowByKey(beamId.at(i)));
 		
 	return result;
 }
  

	

	
	/**
	 * Create a FeedRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FeedRow::FeedRow (FeedTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	
		feedNumExists = false;
	

	

	

	
		illumOffsetExists = false;
	

	
		illumOffsetPaExists = false;
	

	

	

	
		xPositionExists = false;
	

	
		yPositionExists = false;
	

	
		zPositionExists = false;
	

	

	
	

	
		beamIdExists = false;
	

	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	FeedRow::FeedRow (FeedTable &t, FeedRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	
		feedNumExists = false;
	

	

	

	
		illumOffsetExists = false;
	

	
		illumOffsetPaExists = false;
	

	

	

	
		xPositionExists = false;
	

	
		yPositionExists = false;
	

	
		zPositionExists = false;
	

	

	
	

	
		beamIdExists = false;
	

	

	
		
		}
		else {
	
		
			feedId = row.feedId;
		
			antennaId = row.antennaId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			receiverId = row.receiverId;
		
			numReceptor = row.numReceptor;
		
			beamOffset = row.beamOffset;
		
			focusReference = row.focusReference;
		
			polarizationTypes = row.polarizationTypes;
		
			polResponse = row.polResponse;
		
			receptorAngle = row.receptorAngle;
		
		
		
		
		if (row.beamIdExists) {
			beamId = row.beamId;		
			beamIdExists = true;
		}
		else
			beamIdExists = false;
		
		if (row.feedNumExists) {
			feedNum = row.feedNum;		
			feedNumExists = true;
		}
		else
			feedNumExists = false;
		
		if (row.illumOffsetExists) {
			illumOffset = row.illumOffset;		
			illumOffsetExists = true;
		}
		else
			illumOffsetExists = false;
		
		if (row.illumOffsetPaExists) {
			illumOffsetPa = row.illumOffsetPa;		
			illumOffsetPaExists = true;
		}
		else
			illumOffsetPaExists = false;
		
		if (row.xPositionExists) {
			xPosition = row.xPosition;		
			xPositionExists = true;
		}
		else
			xPositionExists = false;
		
		if (row.yPositionExists) {
			yPosition = row.yPosition;		
			yPositionExists = true;
		}
		else
			yPositionExists = false;
		
		if (row.zPositionExists) {
			zPosition = row.zPosition;		
			zPositionExists = true;
		}
		else
			zPositionExists = false;
		
		}	
	}

	
	bool FeedRow::compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, vector<int>  receiverId, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->receiverId == receiverId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->beamOffset == beamOffset);
		
		if (!result) return false;
	

	
		
		result = result && (this->focusReference == focusReference);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->polResponse == polResponse);
		
		if (!result) return false;
	

	
		
		result = result && (this->receptorAngle == receptorAngle);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool FeedRow::compareRequiredValue(vector<int>  receiverId, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle) {
		bool result;
		result = true;
		
	
		if (!(this->receiverId == receiverId)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->beamOffset == beamOffset)) return false;
	

	
		if (!(this->focusReference == focusReference)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->polResponse == polResponse)) return false;
	

	
		if (!(this->receptorAngle == receptorAngle)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FeedRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool FeedRow::equalByRequiredValue(FeedRow* x) {
		
			
		if (this->receiverId != x->receiverId) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->beamOffset != x->beamOffset) return false;
			
		if (this->focusReference != x->focusReference) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->polResponse != x->polResponse) return false;
			
		if (this->receptorAngle != x->receptorAngle) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
