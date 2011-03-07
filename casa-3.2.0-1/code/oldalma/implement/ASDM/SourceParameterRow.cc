
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
 * File SourceParameterRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <SourceParameterRow.h>
#include <SourceParameterTable.h>

#include <SourceTable.h>
#include <SourceRow.h>
	

using asdm::ASDM;
using asdm::SourceParameterRow;
using asdm::SourceParameterTable;

using asdm::SourceTable;
using asdm::SourceRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {

	SourceParameterRow::~SourceParameterRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	SourceParameterTable &SourceParameterRow::getTable() const {
		return table;
	}
	
	void SourceParameterRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SourceParameterRowIDL struct.
	 */
	SourceParameterRowIDL *SourceParameterRow::toIDL() const {
		SourceParameterRowIDL *x = new SourceParameterRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->numFreq = numFreq;
 				
 			
		
	

	
  		
		
		
			
				
		x->numStokes = numStokes;
 				
 			
		
	

	
  		
		
		
			
				
		x->numDep = numDep;
 				
 			
		
	

	
  		
		
		
			
		x->stokesParameter.length(stokesParameter.size());
		for (unsigned int i = 0; i < stokesParameter.size(); ++i) {
			
				
			x->stokesParameter[i] = stokesParameter.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->flux.length(flux.size());
		for (unsigned int i = 0; i < flux.size(); i++) {
			x->flux[i].length(flux.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < flux.size() ; i++)
			for (unsigned int j = 0; j < flux.at(i).size(); j++)
					
				x->flux[i][j]= flux.at(i).at(j).toIDLFlux();
									
		
			
		
	

	
  		
		
		x->sizeExists = sizeExists;
		
		
			
		x->size.length(size.size());
		for (unsigned int i = 0; i < size.size(); i++) {
			x->size[i].length(size.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < size.size() ; i++)
			for (unsigned int j = 0; j < size.at(i).size(); j++)
					
				x->size[i][j]= size.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->positionAngleExists = positionAngleExists;
		
		
			
		x->positionAngle.length(positionAngle.size());
		for (unsigned int i = 0; i < positionAngle.size(); ++i) {
			
			x->positionAngle[i] = positionAngle.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->sourceParameterId = sourceParameterId;
 				
 			
		
	

	
  		
		
		
			
		x->frequency.length(frequency.size());
		for (unsigned int i = 0; i < frequency.size(); ++i) {
			
			x->frequency[i] = frequency.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->frequencyInterval.length(frequencyInterval.size());
		for (unsigned int i = 0; i < frequencyInterval.size(); ++i) {
			
			x->frequencyInterval[i] = frequencyInterval.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->fluxErr.length(fluxErr.size());
		for (unsigned int i = 0; i < fluxErr.size(); i++) {
			x->fluxErr[i].length(fluxErr.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < fluxErr.size() ; i++)
			for (unsigned int j = 0; j < fluxErr.at(i).size(); j++)
					
				x->fluxErr[i][j]= fluxErr.at(i).at(j).toIDLFlux();
									
		
			
		
	

	
  		
		
		x->sizeErrExists = sizeErrExists;
		
		
			
		x->sizeErr.length(sizeErr.size());
		for (unsigned int i = 0; i < sizeErr.size(); i++) {
			x->sizeErr[i].length(sizeErr.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < sizeErr.size() ; i++)
			for (unsigned int j = 0; j < sizeErr.at(i).size(); j++)
					
				x->sizeErr[i][j]= sizeErr.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->positionAngleErrExists = positionAngleErrExists;
		
		
			
		x->positionAngleErr.length(positionAngleErr.size());
		for (unsigned int i = 0; i < positionAngleErr.size(); ++i) {
			
			x->positionAngleErr[i] = positionAngleErr.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->depSourceParameterIdExists = depSourceParameterIdExists;
		
		
			
		x->depSourceParameterId.length(depSourceParameterId.size());
		for (unsigned int i = 0; i < depSourceParameterId.size(); ++i) {
			
				
			x->depSourceParameterId[i] = depSourceParameterId.at(i);
	 			
	 		
	 	}
			
		
	

	
	
		
	
  	
 		
		
	 	
			
				
		x->sourceId = sourceId;
 				
 			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SourceParameterRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void SourceParameterRow::setFromIDL (SourceParameterRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumFreq(x.numFreq);
  			
 		
		
	

	
		
		
			
		setNumStokes(x.numStokes);
  			
 		
		
	

	
		
		
			
		setNumDep(x.numDep);
  			
 		
		
	

	
		
		
			
		stokesParameter .clear();
		for (unsigned int i = 0; i <x.stokesParameter.length(); ++i) {
			
			stokesParameter.push_back(x.stokesParameter[i]);
  			
		}
			
  		
		
	

	
		
		
			
		flux .clear();
		vector<Flux> v_aux_flux;
		for (unsigned int i = 0; i < x.flux.length(); ++i) {
			v_aux_flux.clear();
			for (unsigned int j = 0; j < x.flux[0].length(); ++j) {
				
				v_aux_flux.push_back(Flux (x.flux[i][j]));
				
  			}
  			flux.push_back(v_aux_flux);			
		}
			
  		
		
	

	
		
		sizeExists = x.sizeExists;
		if (x.sizeExists) {
		
		
			
		size .clear();
		vector<Angle> v_aux_size;
		for (unsigned int i = 0; i < x.size.length(); ++i) {
			v_aux_size.clear();
			for (unsigned int j = 0; j < x.size[0].length(); ++j) {
				
				v_aux_size.push_back(Angle (x.size[i][j]));
				
  			}
  			size.push_back(v_aux_size);			
		}
			
  		
		
		}
		
	

	
		
		positionAngleExists = x.positionAngleExists;
		if (x.positionAngleExists) {
		
		
			
		positionAngle .clear();
		for (unsigned int i = 0; i <x.positionAngle.length(); ++i) {
			
			positionAngle.push_back(Angle (x.positionAngle[i]));
			
		}
			
  		
		
		}
		
	

	
		
		
			
		setSourceParameterId(x.sourceParameterId);
  			
 		
		
	

	
		
		
			
		frequency .clear();
		for (unsigned int i = 0; i <x.frequency.length(); ++i) {
			
			frequency.push_back(Frequency (x.frequency[i]));
			
		}
			
  		
		
	

	
		
		
			
		frequencyInterval .clear();
		for (unsigned int i = 0; i <x.frequencyInterval.length(); ++i) {
			
			frequencyInterval.push_back(Frequency (x.frequencyInterval[i]));
			
		}
			
  		
		
	

	
		
		
			
		fluxErr .clear();
		vector<Flux> v_aux_fluxErr;
		for (unsigned int i = 0; i < x.fluxErr.length(); ++i) {
			v_aux_fluxErr.clear();
			for (unsigned int j = 0; j < x.fluxErr[0].length(); ++j) {
				
				v_aux_fluxErr.push_back(Flux (x.fluxErr[i][j]));
				
  			}
  			fluxErr.push_back(v_aux_fluxErr);			
		}
			
  		
		
	

	
		
		sizeErrExists = x.sizeErrExists;
		if (x.sizeErrExists) {
		
		
			
		sizeErr .clear();
		vector<Angle> v_aux_sizeErr;
		for (unsigned int i = 0; i < x.sizeErr.length(); ++i) {
			v_aux_sizeErr.clear();
			for (unsigned int j = 0; j < x.sizeErr[0].length(); ++j) {
				
				v_aux_sizeErr.push_back(Angle (x.sizeErr[i][j]));
				
  			}
  			sizeErr.push_back(v_aux_sizeErr);			
		}
			
  		
		
		}
		
	

	
		
		positionAngleErrExists = x.positionAngleErrExists;
		if (x.positionAngleErrExists) {
		
		
			
		positionAngleErr .clear();
		for (unsigned int i = 0; i <x.positionAngleErr.length(); ++i) {
			
			positionAngleErr.push_back(Angle (x.positionAngleErr[i]));
			
		}
			
  		
		
		}
		
	

	
		
		depSourceParameterIdExists = x.depSourceParameterIdExists;
		if (x.depSourceParameterIdExists) {
		
		
			
		depSourceParameterId .clear();
		for (unsigned int i = 0; i <x.depSourceParameterId.length(); ++i) {
			
			depSourceParameterId.push_back(x.depSourceParameterId[i]);
  			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setSourceId(x.sourceId);
  			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"SourceParameter");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string SourceParameterRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(numFreq, "numFreq", buf);
		
		
	

  	
 		
		
		Parser::toXML(numStokes, "numStokes", buf);
		
		
	

  	
 		
		
		Parser::toXML(numDep, "numDep", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("stokesParameter", stokesParameter));
		
		
	

  	
 		
		
		Parser::toXML(flux, "flux", buf);
		
		
	

  	
 		
		if (sizeExists) {
		
		
		Parser::toXML(size, "size", buf);
		
		
		}
		
	

  	
 		
		if (positionAngleExists) {
		
		
		Parser::toXML(positionAngle, "positionAngle", buf);
		
		
		}
		
	

  	
 		
		
		Parser::toXML(sourceParameterId, "sourceParameterId", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequency, "frequency", buf);
		
		
	

  	
 		
		
		Parser::toXML(frequencyInterval, "frequencyInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(fluxErr, "fluxErr", buf);
		
		
	

  	
 		
		if (sizeErrExists) {
		
		
		Parser::toXML(sizeErr, "sizeErr", buf);
		
		
		}
		
	

  	
 		
		if (positionAngleErrExists) {
		
		
		Parser::toXML(positionAngleErr, "positionAngleErr", buf);
		
		
		}
		
	

  	
 		
		if (depSourceParameterIdExists) {
		
		
		Parser::toXML(depSourceParameterId, "depSourceParameterId", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(sourceId, "sourceId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void SourceParameterRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","SourceParameter",rowDoc));
			
		
	

	
  		
			
	  	setNumFreq(Parser::getInteger("numFreq","SourceParameter",rowDoc));
			
		
	

	
  		
			
	  	setNumStokes(Parser::getInteger("numStokes","SourceParameter",rowDoc));
			
		
	

	
  		
			
	  	setNumDep(Parser::getInteger("numDep","SourceParameter",rowDoc));
			
		
	

	
		
		
		
		stokesParameter = EnumerationParser::getStokesParameter1D("stokesParameter","SourceParameter",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setFlux(Parser::get2DFlux("flux","SourceParameter",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<size>")) {
			
								
	  		setSize(Parser::get2DAngle("size","SourceParameter",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<positionAngle>")) {
			
								
	  		setPositionAngle(Parser::get1DAngle("positionAngle","SourceParameter",rowDoc));
	  			
	  		
		}
 		
	

	
  		
			
	  	setSourceParameterId(Parser::getInteger("sourceParameterId","SourceParameter",rowDoc));
			
		
	

	
  		
			
					
	  	setFrequency(Parser::get1DFrequency("frequency","SourceParameter",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFrequencyInterval(Parser::get1DFrequency("frequencyInterval","SourceParameter",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFluxErr(Parser::get2DFlux("fluxErr","SourceParameter",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<sizeErr>")) {
			
								
	  		setSizeErr(Parser::get2DAngle("sizeErr","SourceParameter",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<positionAngleErr>")) {
			
								
	  		setPositionAngleErr(Parser::get1DAngle("positionAngleErr","SourceParameter",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<depSourceParameterId>")) {
			
								
	  		setDepSourceParameterId(Parser::get1DInteger("depSourceParameterId","SourceParameter",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setSourceId(Parser::getInteger("sourceId","Source",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SourceParameter");
		}
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval SourceParameterRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceParameterRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "SourceParameter");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get numFreq.
 	 * @return numFreq as int
 	 */
 	int SourceParameterRow::getNumFreq() const {
	
  		return numFreq;
 	}

 	/**
 	 * Set numFreq with the specified int.
 	 * @param numFreq The int value to which numFreq is to be set.
 	 
 	
 		
 	 */
 	void SourceParameterRow::setNumFreq (int numFreq)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numFreq = numFreq;
	
 	}
	
	

	

	
 	/**
 	 * Get numStokes.
 	 * @return numStokes as int
 	 */
 	int SourceParameterRow::getNumStokes() const {
	
  		return numStokes;
 	}

 	/**
 	 * Set numStokes with the specified int.
 	 * @param numStokes The int value to which numStokes is to be set.
 	 
 	
 		
 	 */
 	void SourceParameterRow::setNumStokes (int numStokes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numStokes = numStokes;
	
 	}
	
	

	

	
 	/**
 	 * Get numDep.
 	 * @return numDep as int
 	 */
 	int SourceParameterRow::getNumDep() const {
	
  		return numDep;
 	}

 	/**
 	 * Set numDep with the specified int.
 	 * @param numDep The int value to which numDep is to be set.
 	 
 	
 		
 	 */
 	void SourceParameterRow::setNumDep (int numDep)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numDep = numDep;
	
 	}
	
	

	

	
 	/**
 	 * Get stokesParameter.
 	 * @return stokesParameter as vector<StokesParameterMod::StokesParameter >
 	 */
 	vector<StokesParameterMod::StokesParameter > SourceParameterRow::getStokesParameter() const {
	
  		return stokesParameter;
 	}

 	/**
 	 * Set stokesParameter with the specified vector<StokesParameterMod::StokesParameter >.
 	 * @param stokesParameter The vector<StokesParameterMod::StokesParameter > value to which stokesParameter is to be set.
 	 
 	
 		
 	 */
 	void SourceParameterRow::setStokesParameter (vector<StokesParameterMod::StokesParameter > stokesParameter)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->stokesParameter = stokesParameter;
	
 	}
	
	

	

	
 	/**
 	 * Get flux.
 	 * @return flux as vector<vector<Flux > >
 	 */
 	vector<vector<Flux > > SourceParameterRow::getFlux() const {
	
  		return flux;
 	}

 	/**
 	 * Set flux with the specified vector<vector<Flux > >.
 	 * @param flux The vector<vector<Flux > > value to which flux is to be set.
 	 
 	
 		
 	 */
 	void SourceParameterRow::setFlux (vector<vector<Flux > > flux)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flux = flux;
	
 	}
	
	

	
	/**
	 * The attribute size is optional. Return true if this attribute exists.
	 * @return true if and only if the size attribute exists. 
	 */
	bool SourceParameterRow::isSizeExists() const {
		return sizeExists;
	}
	

	
 	/**
 	 * Get size, which is optional.
 	 * @return size as vector<vector<Angle > >
 	 * @throw IllegalAccessException If size does not exist.
 	 */
 	vector<vector<Angle > > SourceParameterRow::getSize() const throw(IllegalAccessException) {
		if (!sizeExists) {
			throw IllegalAccessException("size", "SourceParameter");
		}
	
  		return size;
 	}

 	/**
 	 * Set size with the specified vector<vector<Angle > >.
 	 * @param size The vector<vector<Angle > > value to which size is to be set.
 	 
 	
 	 */
 	void SourceParameterRow::setSize (vector<vector<Angle > > size) {
	
 		this->size = size;
	
		sizeExists = true;
	
 	}
	
	
	/**
	 * Mark size, which is an optional field, as non-existent.
	 */
	void SourceParameterRow::clearSize () {
		sizeExists = false;
	}
	

	
	/**
	 * The attribute positionAngle is optional. Return true if this attribute exists.
	 * @return true if and only if the positionAngle attribute exists. 
	 */
	bool SourceParameterRow::isPositionAngleExists() const {
		return positionAngleExists;
	}
	

	
 	/**
 	 * Get positionAngle, which is optional.
 	 * @return positionAngle as vector<Angle >
 	 * @throw IllegalAccessException If positionAngle does not exist.
 	 */
 	vector<Angle > SourceParameterRow::getPositionAngle() const throw(IllegalAccessException) {
		if (!positionAngleExists) {
			throw IllegalAccessException("positionAngle", "SourceParameter");
		}
	
  		return positionAngle;
 	}

 	/**
 	 * Set positionAngle with the specified vector<Angle >.
 	 * @param positionAngle The vector<Angle > value to which positionAngle is to be set.
 	 
 	
 	 */
 	void SourceParameterRow::setPositionAngle (vector<Angle > positionAngle) {
	
 		this->positionAngle = positionAngle;
	
		positionAngleExists = true;
	
 	}
	
	
	/**
	 * Mark positionAngle, which is an optional field, as non-existent.
	 */
	void SourceParameterRow::clearPositionAngle () {
		positionAngleExists = false;
	}
	

	

	
 	/**
 	 * Get sourceParameterId.
 	 * @return sourceParameterId as int
 	 */
 	int SourceParameterRow::getSourceParameterId() const {
	
  		return sourceParameterId;
 	}

 	/**
 	 * Set sourceParameterId with the specified int.
 	 * @param sourceParameterId The int value to which sourceParameterId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceParameterRow::setSourceParameterId (int sourceParameterId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("sourceParameterId", "SourceParameter");
		
  		}
  	
 		this->sourceParameterId = sourceParameterId;
	
 	}
	
	

	

	
 	/**
 	 * Get frequency.
 	 * @return frequency as vector<Frequency >
 	 */
 	vector<Frequency > SourceParameterRow::getFrequency() const {
	
  		return frequency;
 	}

 	/**
 	 * Set frequency with the specified vector<Frequency >.
 	 * @param frequency The vector<Frequency > value to which frequency is to be set.
 	 
 	
 		
 	 */
 	void SourceParameterRow::setFrequency (vector<Frequency > frequency)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequency = frequency;
	
 	}
	
	

	

	
 	/**
 	 * Get frequencyInterval.
 	 * @return frequencyInterval as vector<Frequency >
 	 */
 	vector<Frequency > SourceParameterRow::getFrequencyInterval() const {
	
  		return frequencyInterval;
 	}

 	/**
 	 * Set frequencyInterval with the specified vector<Frequency >.
 	 * @param frequencyInterval The vector<Frequency > value to which frequencyInterval is to be set.
 	 
 	
 		
 	 */
 	void SourceParameterRow::setFrequencyInterval (vector<Frequency > frequencyInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->frequencyInterval = frequencyInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get fluxErr.
 	 * @return fluxErr as vector<vector<Flux > >
 	 */
 	vector<vector<Flux > > SourceParameterRow::getFluxErr() const {
	
  		return fluxErr;
 	}

 	/**
 	 * Set fluxErr with the specified vector<vector<Flux > >.
 	 * @param fluxErr The vector<vector<Flux > > value to which fluxErr is to be set.
 	 
 	
 		
 	 */
 	void SourceParameterRow::setFluxErr (vector<vector<Flux > > fluxErr)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->fluxErr = fluxErr;
	
 	}
	
	

	
	/**
	 * The attribute sizeErr is optional. Return true if this attribute exists.
	 * @return true if and only if the sizeErr attribute exists. 
	 */
	bool SourceParameterRow::isSizeErrExists() const {
		return sizeErrExists;
	}
	

	
 	/**
 	 * Get sizeErr, which is optional.
 	 * @return sizeErr as vector<vector<Angle > >
 	 * @throw IllegalAccessException If sizeErr does not exist.
 	 */
 	vector<vector<Angle > > SourceParameterRow::getSizeErr() const throw(IllegalAccessException) {
		if (!sizeErrExists) {
			throw IllegalAccessException("sizeErr", "SourceParameter");
		}
	
  		return sizeErr;
 	}

 	/**
 	 * Set sizeErr with the specified vector<vector<Angle > >.
 	 * @param sizeErr The vector<vector<Angle > > value to which sizeErr is to be set.
 	 
 	
 	 */
 	void SourceParameterRow::setSizeErr (vector<vector<Angle > > sizeErr) {
	
 		this->sizeErr = sizeErr;
	
		sizeErrExists = true;
	
 	}
	
	
	/**
	 * Mark sizeErr, which is an optional field, as non-existent.
	 */
	void SourceParameterRow::clearSizeErr () {
		sizeErrExists = false;
	}
	

	
	/**
	 * The attribute positionAngleErr is optional. Return true if this attribute exists.
	 * @return true if and only if the positionAngleErr attribute exists. 
	 */
	bool SourceParameterRow::isPositionAngleErrExists() const {
		return positionAngleErrExists;
	}
	

	
 	/**
 	 * Get positionAngleErr, which is optional.
 	 * @return positionAngleErr as vector<Angle >
 	 * @throw IllegalAccessException If positionAngleErr does not exist.
 	 */
 	vector<Angle > SourceParameterRow::getPositionAngleErr() const throw(IllegalAccessException) {
		if (!positionAngleErrExists) {
			throw IllegalAccessException("positionAngleErr", "SourceParameter");
		}
	
  		return positionAngleErr;
 	}

 	/**
 	 * Set positionAngleErr with the specified vector<Angle >.
 	 * @param positionAngleErr The vector<Angle > value to which positionAngleErr is to be set.
 	 
 	
 	 */
 	void SourceParameterRow::setPositionAngleErr (vector<Angle > positionAngleErr) {
	
 		this->positionAngleErr = positionAngleErr;
	
		positionAngleErrExists = true;
	
 	}
	
	
	/**
	 * Mark positionAngleErr, which is an optional field, as non-existent.
	 */
	void SourceParameterRow::clearPositionAngleErr () {
		positionAngleErrExists = false;
	}
	

	
	/**
	 * The attribute depSourceParameterId is optional. Return true if this attribute exists.
	 * @return true if and only if the depSourceParameterId attribute exists. 
	 */
	bool SourceParameterRow::isDepSourceParameterIdExists() const {
		return depSourceParameterIdExists;
	}
	

	
 	/**
 	 * Get depSourceParameterId, which is optional.
 	 * @return depSourceParameterId as vector<int >
 	 * @throw IllegalAccessException If depSourceParameterId does not exist.
 	 */
 	vector<int > SourceParameterRow::getDepSourceParameterId() const throw(IllegalAccessException) {
		if (!depSourceParameterIdExists) {
			throw IllegalAccessException("depSourceParameterId", "SourceParameter");
		}
	
  		return depSourceParameterId;
 	}

 	/**
 	 * Set depSourceParameterId with the specified vector<int >.
 	 * @param depSourceParameterId The vector<int > value to which depSourceParameterId is to be set.
 	 
 	
 	 */
 	void SourceParameterRow::setDepSourceParameterId (vector<int > depSourceParameterId) {
	
 		this->depSourceParameterId = depSourceParameterId;
	
		depSourceParameterIdExists = true;
	
 	}
	
	
	/**
	 * Mark depSourceParameterId, which is an optional field, as non-existent.
	 */
	void SourceParameterRow::clearDepSourceParameterId () {
		depSourceParameterIdExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get sourceId.
 	 * @return sourceId as int
 	 */
 	int SourceParameterRow::getSourceId() const {
	
  		return sourceId;
 	}

 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceParameterRow::setSourceId (int sourceId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("sourceId", "SourceParameter");
		
  		}
  	
 		this->sourceId = sourceId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	// ===> Slice link from a row of SourceParameter table to a collection of row of Source table.
	
	/**
	 * Get the collection of row in the Source table having their attribut sourceId == this->sourceId
	 */
	vector <SourceRow *> SourceParameterRow::getSources() {
		
			return table.getContainer().getSource().getRowBySourceId(sourceId);
		
	}
	

	

	
	/**
	 * Create a SourceParameterRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SourceParameterRow::SourceParameterRow (SourceParameterTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	
		sizeExists = false;
	

	
		positionAngleExists = false;
	

	

	

	

	

	
		sizeErrExists = false;
	

	
		positionAngleErrExists = false;
	

	
		depSourceParameterIdExists = false;
	

	
	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	SourceParameterRow::SourceParameterRow (SourceParameterTable &t, SourceParameterRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	
		sizeExists = false;
	

	
		positionAngleExists = false;
	

	

	

	

	

	
		sizeErrExists = false;
	

	
		positionAngleErrExists = false;
	

	
		depSourceParameterIdExists = false;
	

	
	
		
		}
		else {
	
		
			sourceParameterId = row.sourceParameterId;
		
			sourceId = row.sourceId;
		
			timeInterval = row.timeInterval;
		
		
		
		
			numFreq = row.numFreq;
		
			numStokes = row.numStokes;
		
			numDep = row.numDep;
		
			stokesParameter = row.stokesParameter;
		
			flux = row.flux;
		
			frequency = row.frequency;
		
			frequencyInterval = row.frequencyInterval;
		
			fluxErr = row.fluxErr;
		
		
		
		
		if (row.sizeExists) {
			size = row.size;		
			sizeExists = true;
		}
		else
			sizeExists = false;
		
		if (row.positionAngleExists) {
			positionAngle = row.positionAngle;		
			positionAngleExists = true;
		}
		else
			positionAngleExists = false;
		
		if (row.sizeErrExists) {
			sizeErr = row.sizeErr;		
			sizeErrExists = true;
		}
		else
			sizeErrExists = false;
		
		if (row.positionAngleErrExists) {
			positionAngleErr = row.positionAngleErr;		
			positionAngleErrExists = true;
		}
		else
			positionAngleErrExists = false;
		
		if (row.depSourceParameterIdExists) {
			depSourceParameterId = row.depSourceParameterId;		
			depSourceParameterIdExists = true;
		}
		else
			depSourceParameterIdExists = false;
		
		}	
	}

	
	bool SourceParameterRow::compareNoAutoInc(int sourceId, ArrayTimeInterval timeInterval, int numFreq, int numStokes, int numDep, vector<StokesParameterMod::StokesParameter > stokesParameter, vector<vector<Flux > > flux, vector<Frequency > frequency, vector<Frequency > frequencyInterval, vector<vector<Flux > > fluxErr) {
		bool result;
		result = true;
		
	
		
		result = result && (this->sourceId == sourceId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->numFreq == numFreq);
		
		if (!result) return false;
	

	
		
		result = result && (this->numStokes == numStokes);
		
		if (!result) return false;
	

	
		
		result = result && (this->numDep == numDep);
		
		if (!result) return false;
	

	
		
		result = result && (this->stokesParameter == stokesParameter);
		
		if (!result) return false;
	

	
		
		result = result && (this->flux == flux);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequency == frequency);
		
		if (!result) return false;
	

	
		
		result = result && (this->frequencyInterval == frequencyInterval);
		
		if (!result) return false;
	

	
		
		result = result && (this->fluxErr == fluxErr);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool SourceParameterRow::compareRequiredValue(int numFreq, int numStokes, int numDep, vector<StokesParameterMod::StokesParameter > stokesParameter, vector<vector<Flux > > flux, vector<Frequency > frequency, vector<Frequency > frequencyInterval, vector<vector<Flux > > fluxErr) {
		bool result;
		result = true;
		
	
		if (!(this->numFreq == numFreq)) return false;
	

	
		if (!(this->numStokes == numStokes)) return false;
	

	
		if (!(this->numDep == numDep)) return false;
	

	
		if (!(this->stokesParameter == stokesParameter)) return false;
	

	
		if (!(this->flux == flux)) return false;
	

	
		if (!(this->frequency == frequency)) return false;
	

	
		if (!(this->frequencyInterval == frequencyInterval)) return false;
	

	
		if (!(this->fluxErr == fluxErr)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SourceParameterRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool SourceParameterRow::equalByRequiredValue(SourceParameterRow* x) {
		
			
		if (this->numFreq != x->numFreq) return false;
			
		if (this->numStokes != x->numStokes) return false;
			
		if (this->numDep != x->numDep) return false;
			
		if (this->stokesParameter != x->stokesParameter) return false;
			
		if (this->flux != x->flux) return false;
			
		if (this->frequency != x->frequency) return false;
			
		if (this->frequencyInterval != x->frequencyInterval) return false;
			
		if (this->fluxErr != x->fluxErr) return false;
			
		
		return true;
	}	
	

} // End namespace asdm
 
