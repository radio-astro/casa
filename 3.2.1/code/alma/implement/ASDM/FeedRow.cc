
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

	bool FeedRow::isAdded() const {
		return hasBeenAdded;
	}	

	void FeedRow::isAdded(bool added) {
		hasBeenAdded = added;
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
									
		
			
		
	

	
  		
		
		
			
		x->receptorAngle.length(receptorAngle.size());
		for (unsigned int i = 0; i < receptorAngle.size(); ++i) {
			
			x->receptorAngle[i] = receptorAngle.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->feedNumExists = feedNumExists;
		
		
			
				
		x->feedNum = feedNum;
 				
 			
		
	

	
  		
		
		x->illumOffsetExists = illumOffsetExists;
		
		
			
		x->illumOffset.length(illumOffset.size());
		for (unsigned int i = 0; i < illumOffset.size(); ++i) {
			
			x->illumOffset[i] = illumOffset.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		x->positionExists = positionExists;
		
		
			
		x->position.length(position.size());
		for (unsigned int i = 0; i < position.size(); ++i) {
			
			x->position[i] = position.at(i).toIDLLength();
			
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
	void FeedRow::setFromIDL (FeedRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
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
			
  		
		
	

	
		
		
			
		receptorAngle .clear();
		for (unsigned int i = 0; i <x.receptorAngle.length(); ++i) {
			
			receptorAngle.push_back(Angle (x.receptorAngle[i]));
			
		}
			
  		
		
	

	
		
		feedNumExists = x.feedNumExists;
		if (x.feedNumExists) {
		
		
			
		setFeedNum(x.feedNum);
  			
 		
		
		}
		
	

	
		
		illumOffsetExists = x.illumOffsetExists;
		if (x.illumOffsetExists) {
		
		
			
		illumOffset .clear();
		for (unsigned int i = 0; i <x.illumOffset.length(); ++i) {
			
			illumOffset.push_back(Length (x.illumOffset[i]));
			
		}
			
  		
		
		}
		
	

	
		
		positionExists = x.positionExists;
		if (x.positionExists) {
		
		
			
		position .clear();
		for (unsigned int i = 0; i <x.position.length(); ++i) {
			
			position.push_back(Length (x.position[i]));
			
		}
			
  		
		
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
			throw ConversionException (err.getMessage(),"Feed");
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
		
		
	

  	
 		
		
		Parser::toXML(beamOffset, "beamOffset", buf);
		
		
	

  	
 		
		
		Parser::toXML(focusReference, "focusReference", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(polResponse, "polResponse", buf);
		
		
	

  	
 		
		
		Parser::toXML(receptorAngle, "receptorAngle", buf);
		
		
	

  	
 		
		if (feedNumExists) {
		
		
		Parser::toXML(feedNum, "feedNum", buf);
		
		
		}
		
	

  	
 		
		if (illumOffsetExists) {
		
		
		Parser::toXML(illumOffset, "illumOffset", buf);
		
		
		}
		
	

  	
 		
		if (positionExists) {
		
		
		Parser::toXML(position, "position", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
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
	void FeedRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Feed",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","Feed",rowDoc));
			
		
	

	
  		
			
					
	  	setBeamOffset(Parser::get2DDouble("beamOffset","Feed",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFocusReference(Parser::get2DLength("focusReference","Feed",rowDoc));
	  			
	  		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","Feed",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setPolResponse(Parser::get2DComplex("polResponse","Feed",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setReceptorAngle(Parser::get1DAngle("receptorAngle","Feed",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<feedNum>")) {
			
	  		setFeedNum(Parser::getInteger("feedNum","Feed",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<illumOffset>")) {
			
								
	  		setIllumOffset(Parser::get1DLength("illumOffset","Feed",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<position>")) {
			
								
	  		setPosition(Parser::get1DLength("position","Feed",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
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
	
	void FeedRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
	spectralWindowId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(feedId);
				
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) beamOffset.size());
		eoss.writeInt((int) beamOffset.at(0).size());
		for (unsigned int i = 0; i < beamOffset.size(); i++) 
			for (unsigned int j = 0;  j < beamOffset.at(0).size(); j++) 
							 
				eoss.writeDouble(beamOffset.at(i).at(j));
				
	
						
		
	

	
	
		
	Length::toBin(focusReference, eoss);
		
	

	
	
		
		
			
		eoss.writeInt((int) polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); i++)
				
			eoss.writeInt(polarizationTypes.at(i));
				
				
						
		
	

	
	
		
	Complex::toBin(polResponse, eoss);
		
	

	
	
		
	Angle::toBin(receptorAngle, eoss);
		
	

	
	
		
		
			
		eoss.writeInt((int) receiverId.size());
		for (unsigned int i = 0; i < receiverId.size(); i++)
				
			eoss.writeInt(receiverId.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(feedNumExists);
	if (feedNumExists) {
	
	
	
		
						
			eoss.writeInt(feedNum);
				
		
	

	}

	eoss.writeBoolean(illumOffsetExists);
	if (illumOffsetExists) {
	
	
	
		
	Length::toBin(illumOffset, eoss);
		
	

	}

	eoss.writeBoolean(positionExists);
	if (positionExists) {
	
	
	
		
	Length::toBin(position, eoss);
		
	

	}

	eoss.writeBoolean(beamIdExists);
	if (beamIdExists) {
	
	
	
		
	Tag::toBin(beamId, eoss);
		
	

	}

	}
	
void FeedRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void FeedRow::spectralWindowIdFromBin(EndianISStream& eiss) {
		
	
		
		
		spectralWindowId =  Tag::fromBin(eiss);
		
	
	
}
void FeedRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void FeedRow::feedIdFromBin(EndianISStream& eiss) {
		
	
	
		
			
		feedId =  eiss.readInt();
			
		
	
	
}
void FeedRow::numReceptorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	
	
}
void FeedRow::beamOffsetFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		beamOffset.clear();
		
		unsigned int beamOffsetDim1 = eiss.readInt();
		unsigned int beamOffsetDim2 = eiss.readInt();
		vector <double> beamOffsetAux1;
		for (unsigned int i = 0; i < beamOffsetDim1; i++) {
			beamOffsetAux1.clear();
			for (unsigned int j = 0; j < beamOffsetDim2 ; j++)			
			
			beamOffsetAux1.push_back(eiss.readDouble());
			
			beamOffset.push_back(beamOffsetAux1);
		}
	
	

		
	
	
}
void FeedRow::focusReferenceFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	focusReference = Length::from2DBin(eiss);		
	

		
	
	
}
void FeedRow::polarizationTypesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		polarizationTypes.clear();
		
		unsigned int polarizationTypesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypesDim1; i++)
			
			polarizationTypes.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	
	
}
void FeedRow::polResponseFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	polResponse = Complex::from2DBin(eiss);		
	

		
	
	
}
void FeedRow::receptorAngleFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	receptorAngle = Angle::from1DBin(eiss);	
	

		
	
	
}
void FeedRow::receiverIdFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		receiverId.clear();
		
		unsigned int receiverIdDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < receiverIdDim1; i++)
			
			receiverId.push_back(eiss.readInt());
			
	

		
	
	
}

void FeedRow::feedNumFromBin(EndianISStream& eiss) {
		
	feedNumExists = eiss.readBoolean();
	if (feedNumExists) {
		
	
	
		
			
		feedNum =  eiss.readInt();
			
		
	

	}
	
}
void FeedRow::illumOffsetFromBin(EndianISStream& eiss) {
		
	illumOffsetExists = eiss.readBoolean();
	if (illumOffsetExists) {
		
	
		
		
			
	
	illumOffset = Length::from1DBin(eiss);	
	

		
	

	}
	
}
void FeedRow::positionFromBin(EndianISStream& eiss) {
		
	positionExists = eiss.readBoolean();
	if (positionExists) {
		
	
		
		
			
	
	position = Length::from1DBin(eiss);	
	

		
	

	}
	
}
void FeedRow::beamIdFromBin(EndianISStream& eiss) {
		
	beamIdExists = eiss.readBoolean();
	if (beamIdExists) {
		
	
		
		
			
	
	beamId = Tag::from1DBin(eiss);	
	

		
	

	}
	
}
	
	
	FeedRow* FeedRow::fromBin(EndianISStream& eiss, FeedTable& table, const vector<string>& attributesSeq) {
		FeedRow* row = new  FeedRow(table);
		
		map<string, FeedAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "FeedTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
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
 	int FeedRow::getFeedNum() const  {
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
	 * The attribute illumOffset is optional. Return true if this attribute exists.
	 * @return true if and only if the illumOffset attribute exists. 
	 */
	bool FeedRow::isIllumOffsetExists() const {
		return illumOffsetExists;
	}
	

	
 	/**
 	 * Get illumOffset, which is optional.
 	 * @return illumOffset as vector<Length >
 	 * @throw IllegalAccessException If illumOffset does not exist.
 	 */
 	vector<Length > FeedRow::getIllumOffset() const  {
		if (!illumOffsetExists) {
			throw IllegalAccessException("illumOffset", "Feed");
		}
	
  		return illumOffset;
 	}

 	/**
 	 * Set illumOffset with the specified vector<Length >.
 	 * @param illumOffset The vector<Length > value to which illumOffset is to be set.
 	 
 	
 	 */
 	void FeedRow::setIllumOffset (vector<Length > illumOffset) {
	
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
	 * The attribute position is optional. Return true if this attribute exists.
	 * @return true if and only if the position attribute exists. 
	 */
	bool FeedRow::isPositionExists() const {
		return positionExists;
	}
	

	
 	/**
 	 * Get position, which is optional.
 	 * @return position as vector<Length >
 	 * @throw IllegalAccessException If position does not exist.
 	 */
 	vector<Length > FeedRow::getPosition() const  {
		if (!positionExists) {
			throw IllegalAccessException("position", "Feed");
		}
	
  		return position;
 	}

 	/**
 	 * Set position with the specified vector<Length >.
 	 * @param position The vector<Length > value to which position is to be set.
 	 
 	
 	 */
 	void FeedRow::setPosition (vector<Length > position) {
	
 		this->position = position;
	
		positionExists = true;
	
 	}
	
	
	/**
	 * Mark position, which is an optional field, as non-existent.
	 */
	void FeedRow::clearPosition () {
		positionExists = false;
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
 	vector<Tag>  FeedRow::getBeamId() const  {
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
 BeamRow* FeedRow::getBeamUsingBeamId(int i) {
 	return table.getContainer().getBeam().getRowByKey(beamId.at(i));
 } 
 
 /**
  * Returns the vector of BeamRow* linked to this row via the Tags stored in beamId
  *
  */
 vector<BeamRow *> FeedRow::getBeamsUsingBeamId() {
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
	

	
		positionExists = false;
	

	
	

	
		beamIdExists = false;
	

	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaId"] = &FeedRow::antennaIdFromBin; 
	 fromBinMethods["spectralWindowId"] = &FeedRow::spectralWindowIdFromBin; 
	 fromBinMethods["timeInterval"] = &FeedRow::timeIntervalFromBin; 
	 fromBinMethods["feedId"] = &FeedRow::feedIdFromBin; 
	 fromBinMethods["numReceptor"] = &FeedRow::numReceptorFromBin; 
	 fromBinMethods["beamOffset"] = &FeedRow::beamOffsetFromBin; 
	 fromBinMethods["focusReference"] = &FeedRow::focusReferenceFromBin; 
	 fromBinMethods["polarizationTypes"] = &FeedRow::polarizationTypesFromBin; 
	 fromBinMethods["polResponse"] = &FeedRow::polResponseFromBin; 
	 fromBinMethods["receptorAngle"] = &FeedRow::receptorAngleFromBin; 
	 fromBinMethods["receiverId"] = &FeedRow::receiverIdFromBin; 
		
	
	 fromBinMethods["feedNum"] = &FeedRow::feedNumFromBin; 
	 fromBinMethods["illumOffset"] = &FeedRow::illumOffsetFromBin; 
	 fromBinMethods["position"] = &FeedRow::positionFromBin; 
	 fromBinMethods["beamId"] = &FeedRow::beamIdFromBin; 
	
	}
	
	FeedRow::FeedRow (FeedTable &t, FeedRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	
		feedNumExists = false;
	

	
		illumOffsetExists = false;
	

	
		positionExists = false;
	

	
	

	
		beamIdExists = false;
	

	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
			feedId = row.feedId;
		
		
		
		
			numReceptor = row.numReceptor;
		
			beamOffset = row.beamOffset;
		
			focusReference = row.focusReference;
		
			polarizationTypes = row.polarizationTypes;
		
			polResponse = row.polResponse;
		
			receptorAngle = row.receptorAngle;
		
			receiverId = row.receiverId;
		
		
		
		
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
		
		if (row.positionExists) {
			position = row.position;		
			positionExists = true;
		}
		else
			positionExists = false;
		
		if (row.beamIdExists) {
			beamId = row.beamId;		
			beamIdExists = true;
		}
		else
			beamIdExists = false;
		
		}
		
		 fromBinMethods["antennaId"] = &FeedRow::antennaIdFromBin; 
		 fromBinMethods["spectralWindowId"] = &FeedRow::spectralWindowIdFromBin; 
		 fromBinMethods["timeInterval"] = &FeedRow::timeIntervalFromBin; 
		 fromBinMethods["feedId"] = &FeedRow::feedIdFromBin; 
		 fromBinMethods["numReceptor"] = &FeedRow::numReceptorFromBin; 
		 fromBinMethods["beamOffset"] = &FeedRow::beamOffsetFromBin; 
		 fromBinMethods["focusReference"] = &FeedRow::focusReferenceFromBin; 
		 fromBinMethods["polarizationTypes"] = &FeedRow::polarizationTypesFromBin; 
		 fromBinMethods["polResponse"] = &FeedRow::polResponseFromBin; 
		 fromBinMethods["receptorAngle"] = &FeedRow::receptorAngleFromBin; 
		 fromBinMethods["receiverId"] = &FeedRow::receiverIdFromBin; 
			
	
		 fromBinMethods["feedNum"] = &FeedRow::feedNumFromBin; 
		 fromBinMethods["illumOffset"] = &FeedRow::illumOffsetFromBin; 
		 fromBinMethods["position"] = &FeedRow::positionFromBin; 
		 fromBinMethods["beamId"] = &FeedRow::beamIdFromBin; 
			
	}

	
	bool FeedRow::compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle, vector<int>  receiverId) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
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
	

	
		
		result = result && (this->receiverId == receiverId);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool FeedRow::compareRequiredValue(int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle, vector<int>  receiverId) {
		bool result;
		result = true;
		
	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->beamOffset == beamOffset)) return false;
	

	
		if (!(this->focusReference == focusReference)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->polResponse == polResponse)) return false;
	

	
		if (!(this->receptorAngle == receptorAngle)) return false;
	

	
		if (!(this->receiverId == receiverId)) return false;
	

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
		
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->beamOffset != x->beamOffset) return false;
			
		if (this->focusReference != x->focusReference) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->polResponse != x->polResponse) return false;
			
		if (this->receptorAngle != x->receptorAngle) return false;
			
		if (this->receiverId != x->receiverId) return false;
			
		
		return true;
	}	
	
/*
	 map<string, FeedAttributeFromBin> FeedRow::initFromBinMethods() {
		map<string, FeedAttributeFromBin> result;
		
		result["antennaId"] = &FeedRow::antennaIdFromBin;
		result["spectralWindowId"] = &FeedRow::spectralWindowIdFromBin;
		result["timeInterval"] = &FeedRow::timeIntervalFromBin;
		result["feedId"] = &FeedRow::feedIdFromBin;
		result["numReceptor"] = &FeedRow::numReceptorFromBin;
		result["beamOffset"] = &FeedRow::beamOffsetFromBin;
		result["focusReference"] = &FeedRow::focusReferenceFromBin;
		result["polarizationTypes"] = &FeedRow::polarizationTypesFromBin;
		result["polResponse"] = &FeedRow::polResponseFromBin;
		result["receptorAngle"] = &FeedRow::receptorAngleFromBin;
		result["receiverId"] = &FeedRow::receiverIdFromBin;
		
		
		result["feedNum"] = &FeedRow::feedNumFromBin;
		result["illumOffset"] = &FeedRow::illumOffsetFromBin;
		result["position"] = &FeedRow::positionFromBin;
		result["beamId"] = &FeedRow::beamIdFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
