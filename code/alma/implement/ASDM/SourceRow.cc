
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
 * File SourceRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <SourceRow.h>
#include <SourceTable.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>
	

using asdm::ASDM;
using asdm::SourceRow;
using asdm::SourceTable;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	SourceRow::~SourceRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	SourceTable &SourceRow::getTable() const {
		return table;
	}
	
	void SourceRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SourceRowIDL struct.
	 */
	SourceRowIDL *SourceRow::toIDL() const {
		SourceRowIDL *x = new SourceRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->sourceId = sourceId;
 				
 			
		
	

	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->code = CORBA::string_dup(code.c_str());
				
 			
		
	

	
  		
		
		
			
		x->direction.length(direction.size());
		for (unsigned int i = 0; i < direction.size(); ++i) {
			
			x->direction[i] = direction.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		
			
		x->properMotion.length(properMotion.size());
		for (unsigned int i = 0; i < properMotion.size(); ++i) {
			
			x->properMotion[i] = properMotion.at(i).toIDLAngularRate();
			
	 	}
			
		
	

	
  		
		
		
			
				
		x->sourceName = CORBA::string_dup(sourceName.c_str());
				
 			
		
	

	
  		
		
		x->directionCodeExists = directionCodeExists;
		
		
			
				
		x->directionCode = directionCode;
 				
 			
		
	

	
  		
		
		x->directionEquinoxExists = directionEquinoxExists;
		
		
			
		x->directionEquinox = directionEquinox.toIDLArrayTime();
			
		
	

	
  		
		
		x->calibrationGroupExists = calibrationGroupExists;
		
		
			
				
		x->calibrationGroup = calibrationGroup;
 				
 			
		
	

	
  		
		
		x->catalogExists = catalogExists;
		
		
			
				
		x->catalog = CORBA::string_dup(catalog.c_str());
				
 			
		
	

	
  		
		
		x->deltaVelExists = deltaVelExists;
		
		
			
		x->deltaVel = deltaVel.toIDLSpeed();
			
		
	

	
  		
		
		x->positionExists = positionExists;
		
		
			
		x->position.length(position.size());
		for (unsigned int i = 0; i < position.size(); ++i) {
			
			x->position[i] = position.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		x->numLinesExists = numLinesExists;
		
		
			
				
		x->numLines = numLines;
 				
 			
		
	

	
  		
		
		x->transitionExists = transitionExists;
		
		
			
		x->transition.length(transition.size());
		for (unsigned int i = 0; i < transition.size(); ++i) {
			
				
			x->transition[i] = CORBA::string_dup(transition.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		x->restFrequencyExists = restFrequencyExists;
		
		
			
		x->restFrequency.length(restFrequency.size());
		for (unsigned int i = 0; i < restFrequency.size(); ++i) {
			
			x->restFrequency[i] = restFrequency.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		x->sysVelExists = sysVelExists;
		
		
			
		x->sysVel.length(sysVel.size());
		for (unsigned int i = 0; i < sysVel.size(); ++i) {
			
			x->sysVel[i] = sysVel.at(i).toIDLSpeed();
			
	 	}
			
		
	

	
  		
		
		x->rangeVelExists = rangeVelExists;
		
		
			
		x->rangeVel.length(rangeVel.size());
		for (unsigned int i = 0; i < rangeVel.size(); ++i) {
			
			x->rangeVel[i] = rangeVel.at(i).toIDLSpeed();
			
	 	}
			
		
	

	
  		
		
		x->sourceModelExists = sourceModelExists;
		
		
			
				
		x->sourceModel = sourceModel;
 				
 			
		
	

	
  		
		
		x->frequencyRefCodeExists = frequencyRefCodeExists;
		
		
			
				
		x->frequencyRefCode = frequencyRefCode;
 				
 			
		
	

	
  		
		
		x->numFreqExists = numFreqExists;
		
		
			
				
		x->numFreq = numFreq;
 				
 			
		
	

	
  		
		
		x->numStokesExists = numStokesExists;
		
		
			
				
		x->numStokes = numStokes;
 				
 			
		
	

	
  		
		
		x->frequencyExists = frequencyExists;
		
		
			
		x->frequency.length(frequency.size());
		for (unsigned int i = 0; i < frequency.size(); ++i) {
			
			x->frequency[i] = frequency.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		x->frequencyIntervalExists = frequencyIntervalExists;
		
		
			
		x->frequencyInterval.length(frequencyInterval.size());
		for (unsigned int i = 0; i < frequencyInterval.size(); ++i) {
			
			x->frequencyInterval[i] = frequencyInterval.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		x->stokesParameterExists = stokesParameterExists;
		
		
			
		x->stokesParameter.length(stokesParameter.size());
		for (unsigned int i = 0; i < stokesParameter.size(); ++i) {
			
				
			x->stokesParameter[i] = stokesParameter.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->fluxExists = fluxExists;
		
		
			
		x->flux.length(flux.size());
		for (unsigned int i = 0; i < flux.size(); i++) {
			x->flux[i].length(flux.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < flux.size() ; i++)
			for (unsigned int j = 0; j < flux.at(i).size(); j++)
					
				x->flux[i][j]= flux.at(i).at(j).toIDLFlux();
									
		
			
		
	

	
  		
		
		x->fluxErrExists = fluxErrExists;
		
		
			
		x->fluxErr.length(fluxErr.size());
		for (unsigned int i = 0; i < fluxErr.size(); i++) {
			x->fluxErr[i].length(fluxErr.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < fluxErr.size() ; i++)
			for (unsigned int j = 0; j < fluxErr.at(i).size(); j++)
					
				x->fluxErr[i][j]= fluxErr.at(i).at(j).toIDLFlux();
									
		
			
		
	

	
  		
		
		x->positionAngleExists = positionAngleExists;
		
		
			
		x->positionAngle.length(positionAngle.size());
		for (unsigned int i = 0; i < positionAngle.size(); ++i) {
			
			x->positionAngle[i] = positionAngle.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->positionAngleErrExists = positionAngleErrExists;
		
		
			
		x->positionAngleErr.length(positionAngleErr.size());
		for (unsigned int i = 0; i < positionAngleErr.size(); ++i) {
			
			x->positionAngleErr[i] = positionAngleErr.at(i).toIDLAngle();
			
	 	}
			
		
	

	
  		
		
		x->sizeExists = sizeExists;
		
		
			
		x->size.length(size.size());
		for (unsigned int i = 0; i < size.size(); i++) {
			x->size[i].length(size.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < size.size() ; i++)
			for (unsigned int j = 0; j < size.at(i).size(); j++)
					
				x->size[i][j]= size.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->sizeErrExists = sizeErrExists;
		
		
			
		x->sizeErr.length(sizeErr.size());
		for (unsigned int i = 0; i < sizeErr.size(); i++) {
			x->sizeErr[i].length(sizeErr.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < sizeErr.size() ; i++)
			for (unsigned int j = 0; j < sizeErr.at(i).size(); j++)
					
				x->sizeErr[i][j]= sizeErr.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SourceRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void SourceRow::setFromIDL (SourceRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setSourceId(x.sourceId);
  			
 		
		
	

	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setCode(string (x.code));
			
 		
		
	

	
		
		
			
		direction .clear();
		for (unsigned int i = 0; i <x.direction.length(); ++i) {
			
			direction.push_back(Angle (x.direction[i]));
			
		}
			
  		
		
	

	
		
		
			
		properMotion .clear();
		for (unsigned int i = 0; i <x.properMotion.length(); ++i) {
			
			properMotion.push_back(AngularRate (x.properMotion[i]));
			
		}
			
  		
		
	

	
		
		
			
		setSourceName(string (x.sourceName));
			
 		
		
	

	
		
		directionCodeExists = x.directionCodeExists;
		if (x.directionCodeExists) {
		
		
			
		setDirectionCode(x.directionCode);
  			
 		
		
		}
		
	

	
		
		directionEquinoxExists = x.directionEquinoxExists;
		if (x.directionEquinoxExists) {
		
		
			
		setDirectionEquinox(ArrayTime (x.directionEquinox));
			
 		
		
		}
		
	

	
		
		calibrationGroupExists = x.calibrationGroupExists;
		if (x.calibrationGroupExists) {
		
		
			
		setCalibrationGroup(x.calibrationGroup);
  			
 		
		
		}
		
	

	
		
		catalogExists = x.catalogExists;
		if (x.catalogExists) {
		
		
			
		setCatalog(string (x.catalog));
			
 		
		
		}
		
	

	
		
		deltaVelExists = x.deltaVelExists;
		if (x.deltaVelExists) {
		
		
			
		setDeltaVel(Speed (x.deltaVel));
			
 		
		
		}
		
	

	
		
		positionExists = x.positionExists;
		if (x.positionExists) {
		
		
			
		position .clear();
		for (unsigned int i = 0; i <x.position.length(); ++i) {
			
			position.push_back(Length (x.position[i]));
			
		}
			
  		
		
		}
		
	

	
		
		numLinesExists = x.numLinesExists;
		if (x.numLinesExists) {
		
		
			
		setNumLines(x.numLines);
  			
 		
		
		}
		
	

	
		
		transitionExists = x.transitionExists;
		if (x.transitionExists) {
		
		
			
		transition .clear();
		for (unsigned int i = 0; i <x.transition.length(); ++i) {
			
			transition.push_back(string (x.transition[i]));
			
		}
			
  		
		
		}
		
	

	
		
		restFrequencyExists = x.restFrequencyExists;
		if (x.restFrequencyExists) {
		
		
			
		restFrequency .clear();
		for (unsigned int i = 0; i <x.restFrequency.length(); ++i) {
			
			restFrequency.push_back(Frequency (x.restFrequency[i]));
			
		}
			
  		
		
		}
		
	

	
		
		sysVelExists = x.sysVelExists;
		if (x.sysVelExists) {
		
		
			
		sysVel .clear();
		for (unsigned int i = 0; i <x.sysVel.length(); ++i) {
			
			sysVel.push_back(Speed (x.sysVel[i]));
			
		}
			
  		
		
		}
		
	

	
		
		rangeVelExists = x.rangeVelExists;
		if (x.rangeVelExists) {
		
		
			
		rangeVel .clear();
		for (unsigned int i = 0; i <x.rangeVel.length(); ++i) {
			
			rangeVel.push_back(Speed (x.rangeVel[i]));
			
		}
			
  		
		
		}
		
	

	
		
		sourceModelExists = x.sourceModelExists;
		if (x.sourceModelExists) {
		
		
			
		setSourceModel(x.sourceModel);
  			
 		
		
		}
		
	

	
		
		frequencyRefCodeExists = x.frequencyRefCodeExists;
		if (x.frequencyRefCodeExists) {
		
		
			
		setFrequencyRefCode(x.frequencyRefCode);
  			
 		
		
		}
		
	

	
		
		numFreqExists = x.numFreqExists;
		if (x.numFreqExists) {
		
		
			
		setNumFreq(x.numFreq);
  			
 		
		
		}
		
	

	
		
		numStokesExists = x.numStokesExists;
		if (x.numStokesExists) {
		
		
			
		setNumStokes(x.numStokes);
  			
 		
		
		}
		
	

	
		
		frequencyExists = x.frequencyExists;
		if (x.frequencyExists) {
		
		
			
		frequency .clear();
		for (unsigned int i = 0; i <x.frequency.length(); ++i) {
			
			frequency.push_back(Frequency (x.frequency[i]));
			
		}
			
  		
		
		}
		
	

	
		
		frequencyIntervalExists = x.frequencyIntervalExists;
		if (x.frequencyIntervalExists) {
		
		
			
		frequencyInterval .clear();
		for (unsigned int i = 0; i <x.frequencyInterval.length(); ++i) {
			
			frequencyInterval.push_back(Frequency (x.frequencyInterval[i]));
			
		}
			
  		
		
		}
		
	

	
		
		stokesParameterExists = x.stokesParameterExists;
		if (x.stokesParameterExists) {
		
		
			
		stokesParameter .clear();
		for (unsigned int i = 0; i <x.stokesParameter.length(); ++i) {
			
			stokesParameter.push_back(x.stokesParameter[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		fluxExists = x.fluxExists;
		if (x.fluxExists) {
		
		
			
		flux .clear();
		vector<Flux> v_aux_flux;
		for (unsigned int i = 0; i < x.flux.length(); ++i) {
			v_aux_flux.clear();
			for (unsigned int j = 0; j < x.flux[0].length(); ++j) {
				
				v_aux_flux.push_back(Flux (x.flux[i][j]));
				
  			}
  			flux.push_back(v_aux_flux);			
		}
			
  		
		
		}
		
	

	
		
		fluxErrExists = x.fluxErrExists;
		if (x.fluxErrExists) {
		
		
			
		fluxErr .clear();
		vector<Flux> v_aux_fluxErr;
		for (unsigned int i = 0; i < x.fluxErr.length(); ++i) {
			v_aux_fluxErr.clear();
			for (unsigned int j = 0; j < x.fluxErr[0].length(); ++j) {
				
				v_aux_fluxErr.push_back(Flux (x.fluxErr[i][j]));
				
  			}
  			fluxErr.push_back(v_aux_fluxErr);			
		}
			
  		
		
		}
		
	

	
		
		positionAngleExists = x.positionAngleExists;
		if (x.positionAngleExists) {
		
		
			
		positionAngle .clear();
		for (unsigned int i = 0; i <x.positionAngle.length(); ++i) {
			
			positionAngle.push_back(Angle (x.positionAngle[i]));
			
		}
			
  		
		
		}
		
	

	
		
		positionAngleErrExists = x.positionAngleErrExists;
		if (x.positionAngleErrExists) {
		
		
			
		positionAngleErr .clear();
		for (unsigned int i = 0; i <x.positionAngleErr.length(); ++i) {
			
			positionAngleErr.push_back(Angle (x.positionAngleErr[i]));
			
		}
			
  		
		
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
		
	

	
	
		
	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Source");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string SourceRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(sourceId, "sourceId", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(code, "code", buf);
		
		
	

  	
 		
		
		Parser::toXML(direction, "direction", buf);
		
		
	

  	
 		
		
		Parser::toXML(properMotion, "properMotion", buf);
		
		
	

  	
 		
		
		Parser::toXML(sourceName, "sourceName", buf);
		
		
	

  	
 		
		if (directionCodeExists) {
		
		
			buf.append(EnumerationParser::toXML("directionCode", directionCode));
		
		
		}
		
	

  	
 		
		if (directionEquinoxExists) {
		
		
		Parser::toXML(directionEquinox, "directionEquinox", buf);
		
		
		}
		
	

  	
 		
		if (calibrationGroupExists) {
		
		
		Parser::toXML(calibrationGroup, "calibrationGroup", buf);
		
		
		}
		
	

  	
 		
		if (catalogExists) {
		
		
		Parser::toXML(catalog, "catalog", buf);
		
		
		}
		
	

  	
 		
		if (deltaVelExists) {
		
		
		Parser::toXML(deltaVel, "deltaVel", buf);
		
		
		}
		
	

  	
 		
		if (positionExists) {
		
		
		Parser::toXML(position, "position", buf);
		
		
		}
		
	

  	
 		
		if (numLinesExists) {
		
		
		Parser::toXML(numLines, "numLines", buf);
		
		
		}
		
	

  	
 		
		if (transitionExists) {
		
		
		Parser::toXML(transition, "transition", buf);
		
		
		}
		
	

  	
 		
		if (restFrequencyExists) {
		
		
		Parser::toXML(restFrequency, "restFrequency", buf);
		
		
		}
		
	

  	
 		
		if (sysVelExists) {
		
		
		Parser::toXML(sysVel, "sysVel", buf);
		
		
		}
		
	

  	
 		
		if (rangeVelExists) {
		
		
		Parser::toXML(rangeVel, "rangeVel", buf);
		
		
		}
		
	

  	
 		
		if (sourceModelExists) {
		
		
			buf.append(EnumerationParser::toXML("sourceModel", sourceModel));
		
		
		}
		
	

  	
 		
		if (frequencyRefCodeExists) {
		
		
			buf.append(EnumerationParser::toXML("frequencyRefCode", frequencyRefCode));
		
		
		}
		
	

  	
 		
		if (numFreqExists) {
		
		
		Parser::toXML(numFreq, "numFreq", buf);
		
		
		}
		
	

  	
 		
		if (numStokesExists) {
		
		
		Parser::toXML(numStokes, "numStokes", buf);
		
		
		}
		
	

  	
 		
		if (frequencyExists) {
		
		
		Parser::toXML(frequency, "frequency", buf);
		
		
		}
		
	

  	
 		
		if (frequencyIntervalExists) {
		
		
		Parser::toXML(frequencyInterval, "frequencyInterval", buf);
		
		
		}
		
	

  	
 		
		if (stokesParameterExists) {
		
		
			buf.append(EnumerationParser::toXML("stokesParameter", stokesParameter));
		
		
		}
		
	

  	
 		
		if (fluxExists) {
		
		
		Parser::toXML(flux, "flux", buf);
		
		
		}
		
	

  	
 		
		if (fluxErrExists) {
		
		
		Parser::toXML(fluxErr, "fluxErr", buf);
		
		
		}
		
	

  	
 		
		if (positionAngleExists) {
		
		
		Parser::toXML(positionAngle, "positionAngle", buf);
		
		
		}
		
	

  	
 		
		if (positionAngleErrExists) {
		
		
		Parser::toXML(positionAngleErr, "positionAngleErr", buf);
		
		
		}
		
	

  	
 		
		if (sizeExists) {
		
		
		Parser::toXML(size, "size", buf);
		
		
		}
		
	

  	
 		
		if (sizeErrExists) {
		
		
		Parser::toXML(sizeErr, "sizeErr", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void SourceRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setSourceId(Parser::getInteger("sourceId","Source",rowDoc));
			
		
	

	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","Source",rowDoc));
			
		
	

	
  		
			
	  	setCode(Parser::getString("code","Source",rowDoc));
			
		
	

	
  		
			
					
	  	setDirection(Parser::get1DAngle("direction","Source",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setProperMotion(Parser::get1DAngularRate("properMotion","Source",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setSourceName(Parser::getString("sourceName","Source",rowDoc));
			
		
	

	
		
	if (row.isStr("<directionCode>")) {
		
		
		
		directionCode = EnumerationParser::getDirectionReferenceCode("directionCode","Source",rowDoc);
		
		
		
		directionCodeExists = true;
	}
		
	

	
  		
        if (row.isStr("<directionEquinox>")) {
			
	  		setDirectionEquinox(Parser::getArrayTime("directionEquinox","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<calibrationGroup>")) {
			
	  		setCalibrationGroup(Parser::getInteger("calibrationGroup","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<catalog>")) {
			
	  		setCatalog(Parser::getString("catalog","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<deltaVel>")) {
			
	  		setDeltaVel(Parser::getSpeed("deltaVel","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<position>")) {
			
								
	  		setPosition(Parser::get1DLength("position","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<numLines>")) {
			
	  		setNumLines(Parser::getInteger("numLines","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<transition>")) {
			
								
	  		setTransition(Parser::get1DString("transition","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<restFrequency>")) {
			
								
	  		setRestFrequency(Parser::get1DFrequency("restFrequency","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<sysVel>")) {
			
								
	  		setSysVel(Parser::get1DSpeed("sysVel","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<rangeVel>")) {
			
								
	  		setRangeVel(Parser::get1DSpeed("rangeVel","Source",rowDoc));
	  			
	  		
		}
 		
	

	
		
	if (row.isStr("<sourceModel>")) {
		
		
		
		sourceModel = EnumerationParser::getSourceModel("sourceModel","Source",rowDoc);
		
		
		
		sourceModelExists = true;
	}
		
	

	
		
	if (row.isStr("<frequencyRefCode>")) {
		
		
		
		frequencyRefCode = EnumerationParser::getFrequencyReferenceCode("frequencyRefCode","Source",rowDoc);
		
		
		
		frequencyRefCodeExists = true;
	}
		
	

	
  		
        if (row.isStr("<numFreq>")) {
			
	  		setNumFreq(Parser::getInteger("numFreq","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<numStokes>")) {
			
	  		setNumStokes(Parser::getInteger("numStokes","Source",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<frequency>")) {
			
								
	  		setFrequency(Parser::get1DFrequency("frequency","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<frequencyInterval>")) {
			
								
	  		setFrequencyInterval(Parser::get1DFrequency("frequencyInterval","Source",rowDoc));
	  			
	  		
		}
 		
	

	
		
	if (row.isStr("<stokesParameter>")) {
		
		
		
		stokesParameter = EnumerationParser::getStokesParameter1D("stokesParameter","Source",rowDoc);			
		
		
		
		stokesParameterExists = true;
	}
		
	

	
  		
        if (row.isStr("<flux>")) {
			
								
	  		setFlux(Parser::get2DFlux("flux","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<fluxErr>")) {
			
								
	  		setFluxErr(Parser::get2DFlux("fluxErr","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<positionAngle>")) {
			
								
	  		setPositionAngle(Parser::get1DAngle("positionAngle","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<positionAngleErr>")) {
			
								
	  		setPositionAngleErr(Parser::get1DAngle("positionAngleErr","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<size>")) {
			
								
	  		setSize(Parser::get2DAngle("size","Source",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<sizeErr>")) {
			
								
	  		setSizeErr(Parser::get2DAngle("sizeErr","Source",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Source");
		}
	}
	
	void SourceRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
						
			eoss.writeInt(sourceId);
				
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
	spectralWindowId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(code);
				
		
	

	
	
		
	Angle::toBin(direction, eoss);
		
	

	
	
		
	AngularRate::toBin(properMotion, eoss);
		
	

	
	
		
						
			eoss.writeString(sourceName);
				
		
	


	
	
	eoss.writeBoolean(directionCodeExists);
	if (directionCodeExists) {
	
	
	
		
					
			eoss.writeInt(directionCode);
				
		
	

	}

	eoss.writeBoolean(directionEquinoxExists);
	if (directionEquinoxExists) {
	
	
	
		
	directionEquinox.toBin(eoss);
		
	

	}

	eoss.writeBoolean(calibrationGroupExists);
	if (calibrationGroupExists) {
	
	
	
		
						
			eoss.writeInt(calibrationGroup);
				
		
	

	}

	eoss.writeBoolean(catalogExists);
	if (catalogExists) {
	
	
	
		
						
			eoss.writeString(catalog);
				
		
	

	}

	eoss.writeBoolean(deltaVelExists);
	if (deltaVelExists) {
	
	
	
		
	deltaVel.toBin(eoss);
		
	

	}

	eoss.writeBoolean(positionExists);
	if (positionExists) {
	
	
	
		
	Length::toBin(position, eoss);
		
	

	}

	eoss.writeBoolean(numLinesExists);
	if (numLinesExists) {
	
	
	
		
						
			eoss.writeInt(numLines);
				
		
	

	}

	eoss.writeBoolean(transitionExists);
	if (transitionExists) {
	
	
	
		
		
			
		eoss.writeInt((int) transition.size());
		for (unsigned int i = 0; i < transition.size(); i++)
				
			eoss.writeString(transition.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(restFrequencyExists);
	if (restFrequencyExists) {
	
	
	
		
	Frequency::toBin(restFrequency, eoss);
		
	

	}

	eoss.writeBoolean(sysVelExists);
	if (sysVelExists) {
	
	
	
		
	Speed::toBin(sysVel, eoss);
		
	

	}

	eoss.writeBoolean(rangeVelExists);
	if (rangeVelExists) {
	
	
	
		
	Speed::toBin(rangeVel, eoss);
		
	

	}

	eoss.writeBoolean(sourceModelExists);
	if (sourceModelExists) {
	
	
	
		
					
			eoss.writeInt(sourceModel);
				
		
	

	}

	eoss.writeBoolean(frequencyRefCodeExists);
	if (frequencyRefCodeExists) {
	
	
	
		
					
			eoss.writeInt(frequencyRefCode);
				
		
	

	}

	eoss.writeBoolean(numFreqExists);
	if (numFreqExists) {
	
	
	
		
						
			eoss.writeInt(numFreq);
				
		
	

	}

	eoss.writeBoolean(numStokesExists);
	if (numStokesExists) {
	
	
	
		
						
			eoss.writeInt(numStokes);
				
		
	

	}

	eoss.writeBoolean(frequencyExists);
	if (frequencyExists) {
	
	
	
		
	Frequency::toBin(frequency, eoss);
		
	

	}

	eoss.writeBoolean(frequencyIntervalExists);
	if (frequencyIntervalExists) {
	
	
	
		
	Frequency::toBin(frequencyInterval, eoss);
		
	

	}

	eoss.writeBoolean(stokesParameterExists);
	if (stokesParameterExists) {
	
	
	
		
		
			
		eoss.writeInt((int) stokesParameter.size());
		for (unsigned int i = 0; i < stokesParameter.size(); i++)
				
			eoss.writeInt(stokesParameter.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(fluxExists);
	if (fluxExists) {
	
	
	
		
	Flux::toBin(flux, eoss);
		
	

	}

	eoss.writeBoolean(fluxErrExists);
	if (fluxErrExists) {
	
	
	
		
	Flux::toBin(fluxErr, eoss);
		
	

	}

	eoss.writeBoolean(positionAngleExists);
	if (positionAngleExists) {
	
	
	
		
	Angle::toBin(positionAngle, eoss);
		
	

	}

	eoss.writeBoolean(positionAngleErrExists);
	if (positionAngleErrExists) {
	
	
	
		
	Angle::toBin(positionAngleErr, eoss);
		
	

	}

	eoss.writeBoolean(sizeExists);
	if (sizeExists) {
	
	
	
		
	Angle::toBin(size, eoss);
		
	

	}

	eoss.writeBoolean(sizeErrExists);
	if (sizeErrExists) {
	
	
	
		
	Angle::toBin(sizeErr, eoss);
		
	

	}

	}
	
void SourceRow::sourceIdFromBin(EndianISStream& eiss) {
		
	
	
		
			
		sourceId =  eiss.readInt();
			
		
	
	
}
void SourceRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void SourceRow::spectralWindowIdFromBin(EndianISStream& eiss) {
		
	
		
		
		spectralWindowId =  Tag::fromBin(eiss);
		
	
	
}
void SourceRow::codeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		code =  eiss.readString();
			
		
	
	
}
void SourceRow::directionFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	direction = Angle::from1DBin(eiss);	
	

		
	
	
}
void SourceRow::properMotionFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	properMotion = AngularRate::from1DBin(eiss);	
	

		
	
	
}
void SourceRow::sourceNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		sourceName =  eiss.readString();
			
		
	
	
}

void SourceRow::directionCodeFromBin(EndianISStream& eiss) {
		
	directionCodeExists = eiss.readBoolean();
	if (directionCodeExists) {
		
	
	
		
			
		directionCode = CDirectionReferenceCode::from_int(eiss.readInt());
			
		
	

	}
	
}
void SourceRow::directionEquinoxFromBin(EndianISStream& eiss) {
		
	directionEquinoxExists = eiss.readBoolean();
	if (directionEquinoxExists) {
		
	
		
		
		directionEquinox =  ArrayTime::fromBin(eiss);
		
	

	}
	
}
void SourceRow::calibrationGroupFromBin(EndianISStream& eiss) {
		
	calibrationGroupExists = eiss.readBoolean();
	if (calibrationGroupExists) {
		
	
	
		
			
		calibrationGroup =  eiss.readInt();
			
		
	

	}
	
}
void SourceRow::catalogFromBin(EndianISStream& eiss) {
		
	catalogExists = eiss.readBoolean();
	if (catalogExists) {
		
	
	
		
			
		catalog =  eiss.readString();
			
		
	

	}
	
}
void SourceRow::deltaVelFromBin(EndianISStream& eiss) {
		
	deltaVelExists = eiss.readBoolean();
	if (deltaVelExists) {
		
	
		
		
		deltaVel =  Speed::fromBin(eiss);
		
	

	}
	
}
void SourceRow::positionFromBin(EndianISStream& eiss) {
		
	positionExists = eiss.readBoolean();
	if (positionExists) {
		
	
		
		
			
	
	position = Length::from1DBin(eiss);	
	

		
	

	}
	
}
void SourceRow::numLinesFromBin(EndianISStream& eiss) {
		
	numLinesExists = eiss.readBoolean();
	if (numLinesExists) {
		
	
	
		
			
		numLines =  eiss.readInt();
			
		
	

	}
	
}
void SourceRow::transitionFromBin(EndianISStream& eiss) {
		
	transitionExists = eiss.readBoolean();
	if (transitionExists) {
		
	
	
		
			
	
		transition.clear();
		
		unsigned int transitionDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < transitionDim1; i++)
			
			transition.push_back(eiss.readString());
			
	

		
	

	}
	
}
void SourceRow::restFrequencyFromBin(EndianISStream& eiss) {
		
	restFrequencyExists = eiss.readBoolean();
	if (restFrequencyExists) {
		
	
		
		
			
	
	restFrequency = Frequency::from1DBin(eiss);	
	

		
	

	}
	
}
void SourceRow::sysVelFromBin(EndianISStream& eiss) {
		
	sysVelExists = eiss.readBoolean();
	if (sysVelExists) {
		
	
		
		
			
	
	sysVel = Speed::from1DBin(eiss);	
	

		
	

	}
	
}
void SourceRow::rangeVelFromBin(EndianISStream& eiss) {
		
	rangeVelExists = eiss.readBoolean();
	if (rangeVelExists) {
		
	
		
		
			
	
	rangeVel = Speed::from1DBin(eiss);	
	

		
	

	}
	
}
void SourceRow::sourceModelFromBin(EndianISStream& eiss) {
		
	sourceModelExists = eiss.readBoolean();
	if (sourceModelExists) {
		
	
	
		
			
		sourceModel = CSourceModel::from_int(eiss.readInt());
			
		
	

	}
	
}
void SourceRow::frequencyRefCodeFromBin(EndianISStream& eiss) {
		
	frequencyRefCodeExists = eiss.readBoolean();
	if (frequencyRefCodeExists) {
		
	
	
		
			
		frequencyRefCode = CFrequencyReferenceCode::from_int(eiss.readInt());
			
		
	

	}
	
}
void SourceRow::numFreqFromBin(EndianISStream& eiss) {
		
	numFreqExists = eiss.readBoolean();
	if (numFreqExists) {
		
	
	
		
			
		numFreq =  eiss.readInt();
			
		
	

	}
	
}
void SourceRow::numStokesFromBin(EndianISStream& eiss) {
		
	numStokesExists = eiss.readBoolean();
	if (numStokesExists) {
		
	
	
		
			
		numStokes =  eiss.readInt();
			
		
	

	}
	
}
void SourceRow::frequencyFromBin(EndianISStream& eiss) {
		
	frequencyExists = eiss.readBoolean();
	if (frequencyExists) {
		
	
		
		
			
	
	frequency = Frequency::from1DBin(eiss);	
	

		
	

	}
	
}
void SourceRow::frequencyIntervalFromBin(EndianISStream& eiss) {
		
	frequencyIntervalExists = eiss.readBoolean();
	if (frequencyIntervalExists) {
		
	
		
		
			
	
	frequencyInterval = Frequency::from1DBin(eiss);	
	

		
	

	}
	
}
void SourceRow::stokesParameterFromBin(EndianISStream& eiss) {
		
	stokesParameterExists = eiss.readBoolean();
	if (stokesParameterExists) {
		
	
	
		
			
	
		stokesParameter.clear();
		
		unsigned int stokesParameterDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < stokesParameterDim1; i++)
			
			stokesParameter.push_back(CStokesParameter::from_int(eiss.readInt()));
			
	

		
	

	}
	
}
void SourceRow::fluxFromBin(EndianISStream& eiss) {
		
	fluxExists = eiss.readBoolean();
	if (fluxExists) {
		
	
		
		
			
	
	flux = Flux::from2DBin(eiss);		
	

		
	

	}
	
}
void SourceRow::fluxErrFromBin(EndianISStream& eiss) {
		
	fluxErrExists = eiss.readBoolean();
	if (fluxErrExists) {
		
	
		
		
			
	
	fluxErr = Flux::from2DBin(eiss);		
	

		
	

	}
	
}
void SourceRow::positionAngleFromBin(EndianISStream& eiss) {
		
	positionAngleExists = eiss.readBoolean();
	if (positionAngleExists) {
		
	
		
		
			
	
	positionAngle = Angle::from1DBin(eiss);	
	

		
	

	}
	
}
void SourceRow::positionAngleErrFromBin(EndianISStream& eiss) {
		
	positionAngleErrExists = eiss.readBoolean();
	if (positionAngleErrExists) {
		
	
		
		
			
	
	positionAngleErr = Angle::from1DBin(eiss);	
	

		
	

	}
	
}
void SourceRow::sizeFromBin(EndianISStream& eiss) {
		
	sizeExists = eiss.readBoolean();
	if (sizeExists) {
		
	
		
		
			
	
	size = Angle::from2DBin(eiss);		
	

		
	

	}
	
}
void SourceRow::sizeErrFromBin(EndianISStream& eiss) {
		
	sizeErrExists = eiss.readBoolean();
	if (sizeErrExists) {
		
	
		
		
			
	
	sizeErr = Angle::from2DBin(eiss);		
	

		
	

	}
	
}
	
	
	SourceRow* SourceRow::fromBin(EndianISStream& eiss, SourceTable& table, const vector<string>& attributesSeq) {
		SourceRow* row = new  SourceRow(table);
		
		map<string, SourceAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "SourceTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get sourceId.
 	 * @return sourceId as int
 	 */
 	int SourceRow::getSourceId() const {
	
  		return sourceId;
 	}

 	/**
 	 * Set sourceId with the specified int.
 	 * @param sourceId The int value to which sourceId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceRow::setSourceId (int sourceId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("sourceId", "Source");
		
  		}
  	
 		this->sourceId = sourceId;
	
 	}
	
	

	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval SourceRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "Source");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get code.
 	 * @return code as string
 	 */
 	string SourceRow::getCode() const {
	
  		return code;
 	}

 	/**
 	 * Set code with the specified string.
 	 * @param code The string value to which code is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setCode (string code)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->code = code;
	
 	}
	
	

	

	
 	/**
 	 * Get direction.
 	 * @return direction as vector<Angle >
 	 */
 	vector<Angle > SourceRow::getDirection() const {
	
  		return direction;
 	}

 	/**
 	 * Set direction with the specified vector<Angle >.
 	 * @param direction The vector<Angle > value to which direction is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setDirection (vector<Angle > direction)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->direction = direction;
	
 	}
	
	

	

	
 	/**
 	 * Get properMotion.
 	 * @return properMotion as vector<AngularRate >
 	 */
 	vector<AngularRate > SourceRow::getProperMotion() const {
	
  		return properMotion;
 	}

 	/**
 	 * Set properMotion with the specified vector<AngularRate >.
 	 * @param properMotion The vector<AngularRate > value to which properMotion is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setProperMotion (vector<AngularRate > properMotion)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->properMotion = properMotion;
	
 	}
	
	

	

	
 	/**
 	 * Get sourceName.
 	 * @return sourceName as string
 	 */
 	string SourceRow::getSourceName() const {
	
  		return sourceName;
 	}

 	/**
 	 * Set sourceName with the specified string.
 	 * @param sourceName The string value to which sourceName is to be set.
 	 
 	
 		
 	 */
 	void SourceRow::setSourceName (string sourceName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->sourceName = sourceName;
	
 	}
	
	

	
	/**
	 * The attribute directionCode is optional. Return true if this attribute exists.
	 * @return true if and only if the directionCode attribute exists. 
	 */
	bool SourceRow::isDirectionCodeExists() const {
		return directionCodeExists;
	}
	

	
 	/**
 	 * Get directionCode, which is optional.
 	 * @return directionCode as DirectionReferenceCodeMod::DirectionReferenceCode
 	 * @throw IllegalAccessException If directionCode does not exist.
 	 */
 	DirectionReferenceCodeMod::DirectionReferenceCode SourceRow::getDirectionCode() const  {
		if (!directionCodeExists) {
			throw IllegalAccessException("directionCode", "Source");
		}
	
  		return directionCode;
 	}

 	/**
 	 * Set directionCode with the specified DirectionReferenceCodeMod::DirectionReferenceCode.
 	 * @param directionCode The DirectionReferenceCodeMod::DirectionReferenceCode value to which directionCode is to be set.
 	 
 	
 	 */
 	void SourceRow::setDirectionCode (DirectionReferenceCodeMod::DirectionReferenceCode directionCode) {
	
 		this->directionCode = directionCode;
	
		directionCodeExists = true;
	
 	}
	
	
	/**
	 * Mark directionCode, which is an optional field, as non-existent.
	 */
	void SourceRow::clearDirectionCode () {
		directionCodeExists = false;
	}
	

	
	/**
	 * The attribute directionEquinox is optional. Return true if this attribute exists.
	 * @return true if and only if the directionEquinox attribute exists. 
	 */
	bool SourceRow::isDirectionEquinoxExists() const {
		return directionEquinoxExists;
	}
	

	
 	/**
 	 * Get directionEquinox, which is optional.
 	 * @return directionEquinox as ArrayTime
 	 * @throw IllegalAccessException If directionEquinox does not exist.
 	 */
 	ArrayTime SourceRow::getDirectionEquinox() const  {
		if (!directionEquinoxExists) {
			throw IllegalAccessException("directionEquinox", "Source");
		}
	
  		return directionEquinox;
 	}

 	/**
 	 * Set directionEquinox with the specified ArrayTime.
 	 * @param directionEquinox The ArrayTime value to which directionEquinox is to be set.
 	 
 	
 	 */
 	void SourceRow::setDirectionEquinox (ArrayTime directionEquinox) {
	
 		this->directionEquinox = directionEquinox;
	
		directionEquinoxExists = true;
	
 	}
	
	
	/**
	 * Mark directionEquinox, which is an optional field, as non-existent.
	 */
	void SourceRow::clearDirectionEquinox () {
		directionEquinoxExists = false;
	}
	

	
	/**
	 * The attribute calibrationGroup is optional. Return true if this attribute exists.
	 * @return true if and only if the calibrationGroup attribute exists. 
	 */
	bool SourceRow::isCalibrationGroupExists() const {
		return calibrationGroupExists;
	}
	

	
 	/**
 	 * Get calibrationGroup, which is optional.
 	 * @return calibrationGroup as int
 	 * @throw IllegalAccessException If calibrationGroup does not exist.
 	 */
 	int SourceRow::getCalibrationGroup() const  {
		if (!calibrationGroupExists) {
			throw IllegalAccessException("calibrationGroup", "Source");
		}
	
  		return calibrationGroup;
 	}

 	/**
 	 * Set calibrationGroup with the specified int.
 	 * @param calibrationGroup The int value to which calibrationGroup is to be set.
 	 
 	
 	 */
 	void SourceRow::setCalibrationGroup (int calibrationGroup) {
	
 		this->calibrationGroup = calibrationGroup;
	
		calibrationGroupExists = true;
	
 	}
	
	
	/**
	 * Mark calibrationGroup, which is an optional field, as non-existent.
	 */
	void SourceRow::clearCalibrationGroup () {
		calibrationGroupExists = false;
	}
	

	
	/**
	 * The attribute catalog is optional. Return true if this attribute exists.
	 * @return true if and only if the catalog attribute exists. 
	 */
	bool SourceRow::isCatalogExists() const {
		return catalogExists;
	}
	

	
 	/**
 	 * Get catalog, which is optional.
 	 * @return catalog as string
 	 * @throw IllegalAccessException If catalog does not exist.
 	 */
 	string SourceRow::getCatalog() const  {
		if (!catalogExists) {
			throw IllegalAccessException("catalog", "Source");
		}
	
  		return catalog;
 	}

 	/**
 	 * Set catalog with the specified string.
 	 * @param catalog The string value to which catalog is to be set.
 	 
 	
 	 */
 	void SourceRow::setCatalog (string catalog) {
	
 		this->catalog = catalog;
	
		catalogExists = true;
	
 	}
	
	
	/**
	 * Mark catalog, which is an optional field, as non-existent.
	 */
	void SourceRow::clearCatalog () {
		catalogExists = false;
	}
	

	
	/**
	 * The attribute deltaVel is optional. Return true if this attribute exists.
	 * @return true if and only if the deltaVel attribute exists. 
	 */
	bool SourceRow::isDeltaVelExists() const {
		return deltaVelExists;
	}
	

	
 	/**
 	 * Get deltaVel, which is optional.
 	 * @return deltaVel as Speed
 	 * @throw IllegalAccessException If deltaVel does not exist.
 	 */
 	Speed SourceRow::getDeltaVel() const  {
		if (!deltaVelExists) {
			throw IllegalAccessException("deltaVel", "Source");
		}
	
  		return deltaVel;
 	}

 	/**
 	 * Set deltaVel with the specified Speed.
 	 * @param deltaVel The Speed value to which deltaVel is to be set.
 	 
 	
 	 */
 	void SourceRow::setDeltaVel (Speed deltaVel) {
	
 		this->deltaVel = deltaVel;
	
		deltaVelExists = true;
	
 	}
	
	
	/**
	 * Mark deltaVel, which is an optional field, as non-existent.
	 */
	void SourceRow::clearDeltaVel () {
		deltaVelExists = false;
	}
	

	
	/**
	 * The attribute position is optional. Return true if this attribute exists.
	 * @return true if and only if the position attribute exists. 
	 */
	bool SourceRow::isPositionExists() const {
		return positionExists;
	}
	

	
 	/**
 	 * Get position, which is optional.
 	 * @return position as vector<Length >
 	 * @throw IllegalAccessException If position does not exist.
 	 */
 	vector<Length > SourceRow::getPosition() const  {
		if (!positionExists) {
			throw IllegalAccessException("position", "Source");
		}
	
  		return position;
 	}

 	/**
 	 * Set position with the specified vector<Length >.
 	 * @param position The vector<Length > value to which position is to be set.
 	 
 	
 	 */
 	void SourceRow::setPosition (vector<Length > position) {
	
 		this->position = position;
	
		positionExists = true;
	
 	}
	
	
	/**
	 * Mark position, which is an optional field, as non-existent.
	 */
	void SourceRow::clearPosition () {
		positionExists = false;
	}
	

	
	/**
	 * The attribute numLines is optional. Return true if this attribute exists.
	 * @return true if and only if the numLines attribute exists. 
	 */
	bool SourceRow::isNumLinesExists() const {
		return numLinesExists;
	}
	

	
 	/**
 	 * Get numLines, which is optional.
 	 * @return numLines as int
 	 * @throw IllegalAccessException If numLines does not exist.
 	 */
 	int SourceRow::getNumLines() const  {
		if (!numLinesExists) {
			throw IllegalAccessException("numLines", "Source");
		}
	
  		return numLines;
 	}

 	/**
 	 * Set numLines with the specified int.
 	 * @param numLines The int value to which numLines is to be set.
 	 
 	
 	 */
 	void SourceRow::setNumLines (int numLines) {
	
 		this->numLines = numLines;
	
		numLinesExists = true;
	
 	}
	
	
	/**
	 * Mark numLines, which is an optional field, as non-existent.
	 */
	void SourceRow::clearNumLines () {
		numLinesExists = false;
	}
	

	
	/**
	 * The attribute transition is optional. Return true if this attribute exists.
	 * @return true if and only if the transition attribute exists. 
	 */
	bool SourceRow::isTransitionExists() const {
		return transitionExists;
	}
	

	
 	/**
 	 * Get transition, which is optional.
 	 * @return transition as vector<string >
 	 * @throw IllegalAccessException If transition does not exist.
 	 */
 	vector<string > SourceRow::getTransition() const  {
		if (!transitionExists) {
			throw IllegalAccessException("transition", "Source");
		}
	
  		return transition;
 	}

 	/**
 	 * Set transition with the specified vector<string >.
 	 * @param transition The vector<string > value to which transition is to be set.
 	 
 	
 	 */
 	void SourceRow::setTransition (vector<string > transition) {
	
 		this->transition = transition;
	
		transitionExists = true;
	
 	}
	
	
	/**
	 * Mark transition, which is an optional field, as non-existent.
	 */
	void SourceRow::clearTransition () {
		transitionExists = false;
	}
	

	
	/**
	 * The attribute restFrequency is optional. Return true if this attribute exists.
	 * @return true if and only if the restFrequency attribute exists. 
	 */
	bool SourceRow::isRestFrequencyExists() const {
		return restFrequencyExists;
	}
	

	
 	/**
 	 * Get restFrequency, which is optional.
 	 * @return restFrequency as vector<Frequency >
 	 * @throw IllegalAccessException If restFrequency does not exist.
 	 */
 	vector<Frequency > SourceRow::getRestFrequency() const  {
		if (!restFrequencyExists) {
			throw IllegalAccessException("restFrequency", "Source");
		}
	
  		return restFrequency;
 	}

 	/**
 	 * Set restFrequency with the specified vector<Frequency >.
 	 * @param restFrequency The vector<Frequency > value to which restFrequency is to be set.
 	 
 	
 	 */
 	void SourceRow::setRestFrequency (vector<Frequency > restFrequency) {
	
 		this->restFrequency = restFrequency;
	
		restFrequencyExists = true;
	
 	}
	
	
	/**
	 * Mark restFrequency, which is an optional field, as non-existent.
	 */
	void SourceRow::clearRestFrequency () {
		restFrequencyExists = false;
	}
	

	
	/**
	 * The attribute sysVel is optional. Return true if this attribute exists.
	 * @return true if and only if the sysVel attribute exists. 
	 */
	bool SourceRow::isSysVelExists() const {
		return sysVelExists;
	}
	

	
 	/**
 	 * Get sysVel, which is optional.
 	 * @return sysVel as vector<Speed >
 	 * @throw IllegalAccessException If sysVel does not exist.
 	 */
 	vector<Speed > SourceRow::getSysVel() const  {
		if (!sysVelExists) {
			throw IllegalAccessException("sysVel", "Source");
		}
	
  		return sysVel;
 	}

 	/**
 	 * Set sysVel with the specified vector<Speed >.
 	 * @param sysVel The vector<Speed > value to which sysVel is to be set.
 	 
 	
 	 */
 	void SourceRow::setSysVel (vector<Speed > sysVel) {
	
 		this->sysVel = sysVel;
	
		sysVelExists = true;
	
 	}
	
	
	/**
	 * Mark sysVel, which is an optional field, as non-existent.
	 */
	void SourceRow::clearSysVel () {
		sysVelExists = false;
	}
	

	
	/**
	 * The attribute rangeVel is optional. Return true if this attribute exists.
	 * @return true if and only if the rangeVel attribute exists. 
	 */
	bool SourceRow::isRangeVelExists() const {
		return rangeVelExists;
	}
	

	
 	/**
 	 * Get rangeVel, which is optional.
 	 * @return rangeVel as vector<Speed >
 	 * @throw IllegalAccessException If rangeVel does not exist.
 	 */
 	vector<Speed > SourceRow::getRangeVel() const  {
		if (!rangeVelExists) {
			throw IllegalAccessException("rangeVel", "Source");
		}
	
  		return rangeVel;
 	}

 	/**
 	 * Set rangeVel with the specified vector<Speed >.
 	 * @param rangeVel The vector<Speed > value to which rangeVel is to be set.
 	 
 	
 	 */
 	void SourceRow::setRangeVel (vector<Speed > rangeVel) {
	
 		this->rangeVel = rangeVel;
	
		rangeVelExists = true;
	
 	}
	
	
	/**
	 * Mark rangeVel, which is an optional field, as non-existent.
	 */
	void SourceRow::clearRangeVel () {
		rangeVelExists = false;
	}
	

	
	/**
	 * The attribute sourceModel is optional. Return true if this attribute exists.
	 * @return true if and only if the sourceModel attribute exists. 
	 */
	bool SourceRow::isSourceModelExists() const {
		return sourceModelExists;
	}
	

	
 	/**
 	 * Get sourceModel, which is optional.
 	 * @return sourceModel as SourceModelMod::SourceModel
 	 * @throw IllegalAccessException If sourceModel does not exist.
 	 */
 	SourceModelMod::SourceModel SourceRow::getSourceModel() const  {
		if (!sourceModelExists) {
			throw IllegalAccessException("sourceModel", "Source");
		}
	
  		return sourceModel;
 	}

 	/**
 	 * Set sourceModel with the specified SourceModelMod::SourceModel.
 	 * @param sourceModel The SourceModelMod::SourceModel value to which sourceModel is to be set.
 	 
 	
 	 */
 	void SourceRow::setSourceModel (SourceModelMod::SourceModel sourceModel) {
	
 		this->sourceModel = sourceModel;
	
		sourceModelExists = true;
	
 	}
	
	
	/**
	 * Mark sourceModel, which is an optional field, as non-existent.
	 */
	void SourceRow::clearSourceModel () {
		sourceModelExists = false;
	}
	

	
	/**
	 * The attribute frequencyRefCode is optional. Return true if this attribute exists.
	 * @return true if and only if the frequencyRefCode attribute exists. 
	 */
	bool SourceRow::isFrequencyRefCodeExists() const {
		return frequencyRefCodeExists;
	}
	

	
 	/**
 	 * Get frequencyRefCode, which is optional.
 	 * @return frequencyRefCode as FrequencyReferenceCodeMod::FrequencyReferenceCode
 	 * @throw IllegalAccessException If frequencyRefCode does not exist.
 	 */
 	FrequencyReferenceCodeMod::FrequencyReferenceCode SourceRow::getFrequencyRefCode() const  {
		if (!frequencyRefCodeExists) {
			throw IllegalAccessException("frequencyRefCode", "Source");
		}
	
  		return frequencyRefCode;
 	}

 	/**
 	 * Set frequencyRefCode with the specified FrequencyReferenceCodeMod::FrequencyReferenceCode.
 	 * @param frequencyRefCode The FrequencyReferenceCodeMod::FrequencyReferenceCode value to which frequencyRefCode is to be set.
 	 
 	
 	 */
 	void SourceRow::setFrequencyRefCode (FrequencyReferenceCodeMod::FrequencyReferenceCode frequencyRefCode) {
	
 		this->frequencyRefCode = frequencyRefCode;
	
		frequencyRefCodeExists = true;
	
 	}
	
	
	/**
	 * Mark frequencyRefCode, which is an optional field, as non-existent.
	 */
	void SourceRow::clearFrequencyRefCode () {
		frequencyRefCodeExists = false;
	}
	

	
	/**
	 * The attribute numFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the numFreq attribute exists. 
	 */
	bool SourceRow::isNumFreqExists() const {
		return numFreqExists;
	}
	

	
 	/**
 	 * Get numFreq, which is optional.
 	 * @return numFreq as int
 	 * @throw IllegalAccessException If numFreq does not exist.
 	 */
 	int SourceRow::getNumFreq() const  {
		if (!numFreqExists) {
			throw IllegalAccessException("numFreq", "Source");
		}
	
  		return numFreq;
 	}

 	/**
 	 * Set numFreq with the specified int.
 	 * @param numFreq The int value to which numFreq is to be set.
 	 
 	
 	 */
 	void SourceRow::setNumFreq (int numFreq) {
	
 		this->numFreq = numFreq;
	
		numFreqExists = true;
	
 	}
	
	
	/**
	 * Mark numFreq, which is an optional field, as non-existent.
	 */
	void SourceRow::clearNumFreq () {
		numFreqExists = false;
	}
	

	
	/**
	 * The attribute numStokes is optional. Return true if this attribute exists.
	 * @return true if and only if the numStokes attribute exists. 
	 */
	bool SourceRow::isNumStokesExists() const {
		return numStokesExists;
	}
	

	
 	/**
 	 * Get numStokes, which is optional.
 	 * @return numStokes as int
 	 * @throw IllegalAccessException If numStokes does not exist.
 	 */
 	int SourceRow::getNumStokes() const  {
		if (!numStokesExists) {
			throw IllegalAccessException("numStokes", "Source");
		}
	
  		return numStokes;
 	}

 	/**
 	 * Set numStokes with the specified int.
 	 * @param numStokes The int value to which numStokes is to be set.
 	 
 	
 	 */
 	void SourceRow::setNumStokes (int numStokes) {
	
 		this->numStokes = numStokes;
	
		numStokesExists = true;
	
 	}
	
	
	/**
	 * Mark numStokes, which is an optional field, as non-existent.
	 */
	void SourceRow::clearNumStokes () {
		numStokesExists = false;
	}
	

	
	/**
	 * The attribute frequency is optional. Return true if this attribute exists.
	 * @return true if and only if the frequency attribute exists. 
	 */
	bool SourceRow::isFrequencyExists() const {
		return frequencyExists;
	}
	

	
 	/**
 	 * Get frequency, which is optional.
 	 * @return frequency as vector<Frequency >
 	 * @throw IllegalAccessException If frequency does not exist.
 	 */
 	vector<Frequency > SourceRow::getFrequency() const  {
		if (!frequencyExists) {
			throw IllegalAccessException("frequency", "Source");
		}
	
  		return frequency;
 	}

 	/**
 	 * Set frequency with the specified vector<Frequency >.
 	 * @param frequency The vector<Frequency > value to which frequency is to be set.
 	 
 	
 	 */
 	void SourceRow::setFrequency (vector<Frequency > frequency) {
	
 		this->frequency = frequency;
	
		frequencyExists = true;
	
 	}
	
	
	/**
	 * Mark frequency, which is an optional field, as non-existent.
	 */
	void SourceRow::clearFrequency () {
		frequencyExists = false;
	}
	

	
	/**
	 * The attribute frequencyInterval is optional. Return true if this attribute exists.
	 * @return true if and only if the frequencyInterval attribute exists. 
	 */
	bool SourceRow::isFrequencyIntervalExists() const {
		return frequencyIntervalExists;
	}
	

	
 	/**
 	 * Get frequencyInterval, which is optional.
 	 * @return frequencyInterval as vector<Frequency >
 	 * @throw IllegalAccessException If frequencyInterval does not exist.
 	 */
 	vector<Frequency > SourceRow::getFrequencyInterval() const  {
		if (!frequencyIntervalExists) {
			throw IllegalAccessException("frequencyInterval", "Source");
		}
	
  		return frequencyInterval;
 	}

 	/**
 	 * Set frequencyInterval with the specified vector<Frequency >.
 	 * @param frequencyInterval The vector<Frequency > value to which frequencyInterval is to be set.
 	 
 	
 	 */
 	void SourceRow::setFrequencyInterval (vector<Frequency > frequencyInterval) {
	
 		this->frequencyInterval = frequencyInterval;
	
		frequencyIntervalExists = true;
	
 	}
	
	
	/**
	 * Mark frequencyInterval, which is an optional field, as non-existent.
	 */
	void SourceRow::clearFrequencyInterval () {
		frequencyIntervalExists = false;
	}
	

	
	/**
	 * The attribute stokesParameter is optional. Return true if this attribute exists.
	 * @return true if and only if the stokesParameter attribute exists. 
	 */
	bool SourceRow::isStokesParameterExists() const {
		return stokesParameterExists;
	}
	

	
 	/**
 	 * Get stokesParameter, which is optional.
 	 * @return stokesParameter as vector<StokesParameterMod::StokesParameter >
 	 * @throw IllegalAccessException If stokesParameter does not exist.
 	 */
 	vector<StokesParameterMod::StokesParameter > SourceRow::getStokesParameter() const  {
		if (!stokesParameterExists) {
			throw IllegalAccessException("stokesParameter", "Source");
		}
	
  		return stokesParameter;
 	}

 	/**
 	 * Set stokesParameter with the specified vector<StokesParameterMod::StokesParameter >.
 	 * @param stokesParameter The vector<StokesParameterMod::StokesParameter > value to which stokesParameter is to be set.
 	 
 	
 	 */
 	void SourceRow::setStokesParameter (vector<StokesParameterMod::StokesParameter > stokesParameter) {
	
 		this->stokesParameter = stokesParameter;
	
		stokesParameterExists = true;
	
 	}
	
	
	/**
	 * Mark stokesParameter, which is an optional field, as non-existent.
	 */
	void SourceRow::clearStokesParameter () {
		stokesParameterExists = false;
	}
	

	
	/**
	 * The attribute flux is optional. Return true if this attribute exists.
	 * @return true if and only if the flux attribute exists. 
	 */
	bool SourceRow::isFluxExists() const {
		return fluxExists;
	}
	

	
 	/**
 	 * Get flux, which is optional.
 	 * @return flux as vector<vector<Flux > >
 	 * @throw IllegalAccessException If flux does not exist.
 	 */
 	vector<vector<Flux > > SourceRow::getFlux() const  {
		if (!fluxExists) {
			throw IllegalAccessException("flux", "Source");
		}
	
  		return flux;
 	}

 	/**
 	 * Set flux with the specified vector<vector<Flux > >.
 	 * @param flux The vector<vector<Flux > > value to which flux is to be set.
 	 
 	
 	 */
 	void SourceRow::setFlux (vector<vector<Flux > > flux) {
	
 		this->flux = flux;
	
		fluxExists = true;
	
 	}
	
	
	/**
	 * Mark flux, which is an optional field, as non-existent.
	 */
	void SourceRow::clearFlux () {
		fluxExists = false;
	}
	

	
	/**
	 * The attribute fluxErr is optional. Return true if this attribute exists.
	 * @return true if and only if the fluxErr attribute exists. 
	 */
	bool SourceRow::isFluxErrExists() const {
		return fluxErrExists;
	}
	

	
 	/**
 	 * Get fluxErr, which is optional.
 	 * @return fluxErr as vector<vector<Flux > >
 	 * @throw IllegalAccessException If fluxErr does not exist.
 	 */
 	vector<vector<Flux > > SourceRow::getFluxErr() const  {
		if (!fluxErrExists) {
			throw IllegalAccessException("fluxErr", "Source");
		}
	
  		return fluxErr;
 	}

 	/**
 	 * Set fluxErr with the specified vector<vector<Flux > >.
 	 * @param fluxErr The vector<vector<Flux > > value to which fluxErr is to be set.
 	 
 	
 	 */
 	void SourceRow::setFluxErr (vector<vector<Flux > > fluxErr) {
	
 		this->fluxErr = fluxErr;
	
		fluxErrExists = true;
	
 	}
	
	
	/**
	 * Mark fluxErr, which is an optional field, as non-existent.
	 */
	void SourceRow::clearFluxErr () {
		fluxErrExists = false;
	}
	

	
	/**
	 * The attribute positionAngle is optional. Return true if this attribute exists.
	 * @return true if and only if the positionAngle attribute exists. 
	 */
	bool SourceRow::isPositionAngleExists() const {
		return positionAngleExists;
	}
	

	
 	/**
 	 * Get positionAngle, which is optional.
 	 * @return positionAngle as vector<Angle >
 	 * @throw IllegalAccessException If positionAngle does not exist.
 	 */
 	vector<Angle > SourceRow::getPositionAngle() const  {
		if (!positionAngleExists) {
			throw IllegalAccessException("positionAngle", "Source");
		}
	
  		return positionAngle;
 	}

 	/**
 	 * Set positionAngle with the specified vector<Angle >.
 	 * @param positionAngle The vector<Angle > value to which positionAngle is to be set.
 	 
 	
 	 */
 	void SourceRow::setPositionAngle (vector<Angle > positionAngle) {
	
 		this->positionAngle = positionAngle;
	
		positionAngleExists = true;
	
 	}
	
	
	/**
	 * Mark positionAngle, which is an optional field, as non-existent.
	 */
	void SourceRow::clearPositionAngle () {
		positionAngleExists = false;
	}
	

	
	/**
	 * The attribute positionAngleErr is optional. Return true if this attribute exists.
	 * @return true if and only if the positionAngleErr attribute exists. 
	 */
	bool SourceRow::isPositionAngleErrExists() const {
		return positionAngleErrExists;
	}
	

	
 	/**
 	 * Get positionAngleErr, which is optional.
 	 * @return positionAngleErr as vector<Angle >
 	 * @throw IllegalAccessException If positionAngleErr does not exist.
 	 */
 	vector<Angle > SourceRow::getPositionAngleErr() const  {
		if (!positionAngleErrExists) {
			throw IllegalAccessException("positionAngleErr", "Source");
		}
	
  		return positionAngleErr;
 	}

 	/**
 	 * Set positionAngleErr with the specified vector<Angle >.
 	 * @param positionAngleErr The vector<Angle > value to which positionAngleErr is to be set.
 	 
 	
 	 */
 	void SourceRow::setPositionAngleErr (vector<Angle > positionAngleErr) {
	
 		this->positionAngleErr = positionAngleErr;
	
		positionAngleErrExists = true;
	
 	}
	
	
	/**
	 * Mark positionAngleErr, which is an optional field, as non-existent.
	 */
	void SourceRow::clearPositionAngleErr () {
		positionAngleErrExists = false;
	}
	

	
	/**
	 * The attribute size is optional. Return true if this attribute exists.
	 * @return true if and only if the size attribute exists. 
	 */
	bool SourceRow::isSizeExists() const {
		return sizeExists;
	}
	

	
 	/**
 	 * Get size, which is optional.
 	 * @return size as vector<vector<Angle > >
 	 * @throw IllegalAccessException If size does not exist.
 	 */
 	vector<vector<Angle > > SourceRow::getSize() const  {
		if (!sizeExists) {
			throw IllegalAccessException("size", "Source");
		}
	
  		return size;
 	}

 	/**
 	 * Set size with the specified vector<vector<Angle > >.
 	 * @param size The vector<vector<Angle > > value to which size is to be set.
 	 
 	
 	 */
 	void SourceRow::setSize (vector<vector<Angle > > size) {
	
 		this->size = size;
	
		sizeExists = true;
	
 	}
	
	
	/**
	 * Mark size, which is an optional field, as non-existent.
	 */
	void SourceRow::clearSize () {
		sizeExists = false;
	}
	

	
	/**
	 * The attribute sizeErr is optional. Return true if this attribute exists.
	 * @return true if and only if the sizeErr attribute exists. 
	 */
	bool SourceRow::isSizeErrExists() const {
		return sizeErrExists;
	}
	

	
 	/**
 	 * Get sizeErr, which is optional.
 	 * @return sizeErr as vector<vector<Angle > >
 	 * @throw IllegalAccessException If sizeErr does not exist.
 	 */
 	vector<vector<Angle > > SourceRow::getSizeErr() const  {
		if (!sizeErrExists) {
			throw IllegalAccessException("sizeErr", "Source");
		}
	
  		return sizeErr;
 	}

 	/**
 	 * Set sizeErr with the specified vector<vector<Angle > >.
 	 * @param sizeErr The vector<vector<Angle > > value to which sizeErr is to be set.
 	 
 	
 	 */
 	void SourceRow::setSizeErr (vector<vector<Angle > > sizeErr) {
	
 		this->sizeErr = sizeErr;
	
		sizeErrExists = true;
	
 	}
	
	
	/**
	 * Mark sizeErr, which is an optional field, as non-existent.
	 */
	void SourceRow::clearSizeErr () {
		sizeErrExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag SourceRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SourceRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "Source");
		
  		}
  	
 		this->spectralWindowId = spectralWindowId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* SourceRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	/**
	 * Create a SourceRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SourceRow::SourceRow (SourceTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	
		directionCodeExists = false;
	

	
		directionEquinoxExists = false;
	

	
		calibrationGroupExists = false;
	

	
		catalogExists = false;
	

	
		deltaVelExists = false;
	

	
		positionExists = false;
	

	
		numLinesExists = false;
	

	
		transitionExists = false;
	

	
		restFrequencyExists = false;
	

	
		sysVelExists = false;
	

	
		rangeVelExists = false;
	

	
		sourceModelExists = false;
	

	
		frequencyRefCodeExists = false;
	

	
		numFreqExists = false;
	

	
		numStokesExists = false;
	

	
		frequencyExists = false;
	

	
		frequencyIntervalExists = false;
	

	
		stokesParameterExists = false;
	

	
		fluxExists = false;
	

	
		fluxErrExists = false;
	

	
		positionAngleExists = false;
	

	
		positionAngleErrExists = false;
	

	
		sizeExists = false;
	

	
		sizeErrExists = false;
	

	
	

	
	
	
	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
directionCode = CDirectionReferenceCode::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
sourceModel = CSourceModel::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
frequencyRefCode = CFrequencyReferenceCode::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["sourceId"] = &SourceRow::sourceIdFromBin; 
	 fromBinMethods["timeInterval"] = &SourceRow::timeIntervalFromBin; 
	 fromBinMethods["spectralWindowId"] = &SourceRow::spectralWindowIdFromBin; 
	 fromBinMethods["code"] = &SourceRow::codeFromBin; 
	 fromBinMethods["direction"] = &SourceRow::directionFromBin; 
	 fromBinMethods["properMotion"] = &SourceRow::properMotionFromBin; 
	 fromBinMethods["sourceName"] = &SourceRow::sourceNameFromBin; 
		
	
	 fromBinMethods["directionCode"] = &SourceRow::directionCodeFromBin; 
	 fromBinMethods["directionEquinox"] = &SourceRow::directionEquinoxFromBin; 
	 fromBinMethods["calibrationGroup"] = &SourceRow::calibrationGroupFromBin; 
	 fromBinMethods["catalog"] = &SourceRow::catalogFromBin; 
	 fromBinMethods["deltaVel"] = &SourceRow::deltaVelFromBin; 
	 fromBinMethods["position"] = &SourceRow::positionFromBin; 
	 fromBinMethods["numLines"] = &SourceRow::numLinesFromBin; 
	 fromBinMethods["transition"] = &SourceRow::transitionFromBin; 
	 fromBinMethods["restFrequency"] = &SourceRow::restFrequencyFromBin; 
	 fromBinMethods["sysVel"] = &SourceRow::sysVelFromBin; 
	 fromBinMethods["rangeVel"] = &SourceRow::rangeVelFromBin; 
	 fromBinMethods["sourceModel"] = &SourceRow::sourceModelFromBin; 
	 fromBinMethods["frequencyRefCode"] = &SourceRow::frequencyRefCodeFromBin; 
	 fromBinMethods["numFreq"] = &SourceRow::numFreqFromBin; 
	 fromBinMethods["numStokes"] = &SourceRow::numStokesFromBin; 
	 fromBinMethods["frequency"] = &SourceRow::frequencyFromBin; 
	 fromBinMethods["frequencyInterval"] = &SourceRow::frequencyIntervalFromBin; 
	 fromBinMethods["stokesParameter"] = &SourceRow::stokesParameterFromBin; 
	 fromBinMethods["flux"] = &SourceRow::fluxFromBin; 
	 fromBinMethods["fluxErr"] = &SourceRow::fluxErrFromBin; 
	 fromBinMethods["positionAngle"] = &SourceRow::positionAngleFromBin; 
	 fromBinMethods["positionAngleErr"] = &SourceRow::positionAngleErrFromBin; 
	 fromBinMethods["size"] = &SourceRow::sizeFromBin; 
	 fromBinMethods["sizeErr"] = &SourceRow::sizeErrFromBin; 
	
	}
	
	SourceRow::SourceRow (SourceTable &t, SourceRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	
		directionCodeExists = false;
	

	
		directionEquinoxExists = false;
	

	
		calibrationGroupExists = false;
	

	
		catalogExists = false;
	

	
		deltaVelExists = false;
	

	
		positionExists = false;
	

	
		numLinesExists = false;
	

	
		transitionExists = false;
	

	
		restFrequencyExists = false;
	

	
		sysVelExists = false;
	

	
		rangeVelExists = false;
	

	
		sourceModelExists = false;
	

	
		frequencyRefCodeExists = false;
	

	
		numFreqExists = false;
	

	
		numStokesExists = false;
	

	
		frequencyExists = false;
	

	
		frequencyIntervalExists = false;
	

	
		stokesParameterExists = false;
	

	
		fluxExists = false;
	

	
		fluxErrExists = false;
	

	
		positionAngleExists = false;
	

	
		positionAngleErrExists = false;
	

	
		sizeExists = false;
	

	
		sizeErrExists = false;
	

	
	
		
		}
		else {
	
		
			sourceId = row.sourceId;
		
			timeInterval = row.timeInterval;
		
			spectralWindowId = row.spectralWindowId;
		
		
		
		
			code = row.code;
		
			direction = row.direction;
		
			properMotion = row.properMotion;
		
			sourceName = row.sourceName;
		
		
		
		
		if (row.directionCodeExists) {
			directionCode = row.directionCode;		
			directionCodeExists = true;
		}
		else
			directionCodeExists = false;
		
		if (row.directionEquinoxExists) {
			directionEquinox = row.directionEquinox;		
			directionEquinoxExists = true;
		}
		else
			directionEquinoxExists = false;
		
		if (row.calibrationGroupExists) {
			calibrationGroup = row.calibrationGroup;		
			calibrationGroupExists = true;
		}
		else
			calibrationGroupExists = false;
		
		if (row.catalogExists) {
			catalog = row.catalog;		
			catalogExists = true;
		}
		else
			catalogExists = false;
		
		if (row.deltaVelExists) {
			deltaVel = row.deltaVel;		
			deltaVelExists = true;
		}
		else
			deltaVelExists = false;
		
		if (row.positionExists) {
			position = row.position;		
			positionExists = true;
		}
		else
			positionExists = false;
		
		if (row.numLinesExists) {
			numLines = row.numLines;		
			numLinesExists = true;
		}
		else
			numLinesExists = false;
		
		if (row.transitionExists) {
			transition = row.transition;		
			transitionExists = true;
		}
		else
			transitionExists = false;
		
		if (row.restFrequencyExists) {
			restFrequency = row.restFrequency;		
			restFrequencyExists = true;
		}
		else
			restFrequencyExists = false;
		
		if (row.sysVelExists) {
			sysVel = row.sysVel;		
			sysVelExists = true;
		}
		else
			sysVelExists = false;
		
		if (row.rangeVelExists) {
			rangeVel = row.rangeVel;		
			rangeVelExists = true;
		}
		else
			rangeVelExists = false;
		
		if (row.sourceModelExists) {
			sourceModel = row.sourceModel;		
			sourceModelExists = true;
		}
		else
			sourceModelExists = false;
		
		if (row.frequencyRefCodeExists) {
			frequencyRefCode = row.frequencyRefCode;		
			frequencyRefCodeExists = true;
		}
		else
			frequencyRefCodeExists = false;
		
		if (row.numFreqExists) {
			numFreq = row.numFreq;		
			numFreqExists = true;
		}
		else
			numFreqExists = false;
		
		if (row.numStokesExists) {
			numStokes = row.numStokes;		
			numStokesExists = true;
		}
		else
			numStokesExists = false;
		
		if (row.frequencyExists) {
			frequency = row.frequency;		
			frequencyExists = true;
		}
		else
			frequencyExists = false;
		
		if (row.frequencyIntervalExists) {
			frequencyInterval = row.frequencyInterval;		
			frequencyIntervalExists = true;
		}
		else
			frequencyIntervalExists = false;
		
		if (row.stokesParameterExists) {
			stokesParameter = row.stokesParameter;		
			stokesParameterExists = true;
		}
		else
			stokesParameterExists = false;
		
		if (row.fluxExists) {
			flux = row.flux;		
			fluxExists = true;
		}
		else
			fluxExists = false;
		
		if (row.fluxErrExists) {
			fluxErr = row.fluxErr;		
			fluxErrExists = true;
		}
		else
			fluxErrExists = false;
		
		if (row.positionAngleExists) {
			positionAngle = row.positionAngle;		
			positionAngleExists = true;
		}
		else
			positionAngleExists = false;
		
		if (row.positionAngleErrExists) {
			positionAngleErr = row.positionAngleErr;		
			positionAngleErrExists = true;
		}
		else
			positionAngleErrExists = false;
		
		if (row.sizeExists) {
			size = row.size;		
			sizeExists = true;
		}
		else
			sizeExists = false;
		
		if (row.sizeErrExists) {
			sizeErr = row.sizeErr;		
			sizeErrExists = true;
		}
		else
			sizeErrExists = false;
		
		}
		
		 fromBinMethods["sourceId"] = &SourceRow::sourceIdFromBin; 
		 fromBinMethods["timeInterval"] = &SourceRow::timeIntervalFromBin; 
		 fromBinMethods["spectralWindowId"] = &SourceRow::spectralWindowIdFromBin; 
		 fromBinMethods["code"] = &SourceRow::codeFromBin; 
		 fromBinMethods["direction"] = &SourceRow::directionFromBin; 
		 fromBinMethods["properMotion"] = &SourceRow::properMotionFromBin; 
		 fromBinMethods["sourceName"] = &SourceRow::sourceNameFromBin; 
			
	
		 fromBinMethods["directionCode"] = &SourceRow::directionCodeFromBin; 
		 fromBinMethods["directionEquinox"] = &SourceRow::directionEquinoxFromBin; 
		 fromBinMethods["calibrationGroup"] = &SourceRow::calibrationGroupFromBin; 
		 fromBinMethods["catalog"] = &SourceRow::catalogFromBin; 
		 fromBinMethods["deltaVel"] = &SourceRow::deltaVelFromBin; 
		 fromBinMethods["position"] = &SourceRow::positionFromBin; 
		 fromBinMethods["numLines"] = &SourceRow::numLinesFromBin; 
		 fromBinMethods["transition"] = &SourceRow::transitionFromBin; 
		 fromBinMethods["restFrequency"] = &SourceRow::restFrequencyFromBin; 
		 fromBinMethods["sysVel"] = &SourceRow::sysVelFromBin; 
		 fromBinMethods["rangeVel"] = &SourceRow::rangeVelFromBin; 
		 fromBinMethods["sourceModel"] = &SourceRow::sourceModelFromBin; 
		 fromBinMethods["frequencyRefCode"] = &SourceRow::frequencyRefCodeFromBin; 
		 fromBinMethods["numFreq"] = &SourceRow::numFreqFromBin; 
		 fromBinMethods["numStokes"] = &SourceRow::numStokesFromBin; 
		 fromBinMethods["frequency"] = &SourceRow::frequencyFromBin; 
		 fromBinMethods["frequencyInterval"] = &SourceRow::frequencyIntervalFromBin; 
		 fromBinMethods["stokesParameter"] = &SourceRow::stokesParameterFromBin; 
		 fromBinMethods["flux"] = &SourceRow::fluxFromBin; 
		 fromBinMethods["fluxErr"] = &SourceRow::fluxErrFromBin; 
		 fromBinMethods["positionAngle"] = &SourceRow::positionAngleFromBin; 
		 fromBinMethods["positionAngleErr"] = &SourceRow::positionAngleErrFromBin; 
		 fromBinMethods["size"] = &SourceRow::sizeFromBin; 
		 fromBinMethods["sizeErr"] = &SourceRow::sizeErrFromBin; 
			
	}

	
	bool SourceRow::compareNoAutoInc(ArrayTimeInterval timeInterval, Tag spectralWindowId, string code, vector<Angle > direction, vector<AngularRate > properMotion, string sourceName) {
		bool result;
		result = true;
		
	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->code == code);
		
		if (!result) return false;
	

	
		
		result = result && (this->direction == direction);
		
		if (!result) return false;
	

	
		
		result = result && (this->properMotion == properMotion);
		
		if (!result) return false;
	

	
		
		result = result && (this->sourceName == sourceName);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool SourceRow::compareRequiredValue(string code, vector<Angle > direction, vector<AngularRate > properMotion, string sourceName) {
		bool result;
		result = true;
		
	
		if (!(this->code == code)) return false;
	

	
		if (!(this->direction == direction)) return false;
	

	
		if (!(this->properMotion == properMotion)) return false;
	

	
		if (!(this->sourceName == sourceName)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SourceRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool SourceRow::equalByRequiredValue(SourceRow* x) {
		
			
		if (this->code != x->code) return false;
			
		if (this->direction != x->direction) return false;
			
		if (this->properMotion != x->properMotion) return false;
			
		if (this->sourceName != x->sourceName) return false;
			
		
		return true;
	}	
	
/*
	 map<string, SourceAttributeFromBin> SourceRow::initFromBinMethods() {
		map<string, SourceAttributeFromBin> result;
		
		result["sourceId"] = &SourceRow::sourceIdFromBin;
		result["timeInterval"] = &SourceRow::timeIntervalFromBin;
		result["spectralWindowId"] = &SourceRow::spectralWindowIdFromBin;
		result["code"] = &SourceRow::codeFromBin;
		result["direction"] = &SourceRow::directionFromBin;
		result["properMotion"] = &SourceRow::properMotionFromBin;
		result["sourceName"] = &SourceRow::sourceNameFromBin;
		
		
		result["directionCode"] = &SourceRow::directionCodeFromBin;
		result["directionEquinox"] = &SourceRow::directionEquinoxFromBin;
		result["calibrationGroup"] = &SourceRow::calibrationGroupFromBin;
		result["catalog"] = &SourceRow::catalogFromBin;
		result["deltaVel"] = &SourceRow::deltaVelFromBin;
		result["position"] = &SourceRow::positionFromBin;
		result["numLines"] = &SourceRow::numLinesFromBin;
		result["transition"] = &SourceRow::transitionFromBin;
		result["restFrequency"] = &SourceRow::restFrequencyFromBin;
		result["sysVel"] = &SourceRow::sysVelFromBin;
		result["rangeVel"] = &SourceRow::rangeVelFromBin;
		result["sourceModel"] = &SourceRow::sourceModelFromBin;
		result["frequencyRefCode"] = &SourceRow::frequencyRefCodeFromBin;
		result["numFreq"] = &SourceRow::numFreqFromBin;
		result["numStokes"] = &SourceRow::numStokesFromBin;
		result["frequency"] = &SourceRow::frequencyFromBin;
		result["frequencyInterval"] = &SourceRow::frequencyIntervalFromBin;
		result["stokesParameter"] = &SourceRow::stokesParameterFromBin;
		result["flux"] = &SourceRow::fluxFromBin;
		result["fluxErr"] = &SourceRow::fluxErrFromBin;
		result["positionAngle"] = &SourceRow::positionAngleFromBin;
		result["positionAngleErr"] = &SourceRow::positionAngleErrFromBin;
		result["size"] = &SourceRow::sizeFromBin;
		result["sizeErr"] = &SourceRow::sizeErrFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
