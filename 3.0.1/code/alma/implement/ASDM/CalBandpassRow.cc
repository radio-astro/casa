
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
 * File CalBandpassRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <CalBandpassRow.h>
#include <CalBandpassTable.h>

#include <CalReductionTable.h>
#include <CalReductionRow.h>

#include <CalDataTable.h>
#include <CalDataRow.h>
	

using asdm::ASDM;
using asdm::CalBandpassRow;
using asdm::CalBandpassTable;

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
	CalBandpassRow::~CalBandpassRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	CalBandpassTable &CalBandpassRow::getTable() const {
		return table;
	}
	
	void CalBandpassRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a CalBandpassRowIDL struct.
	 */
	CalBandpassRowIDL *CalBandpassRow::toIDL() const {
		CalBandpassRowIDL *x = new CalBandpassRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
				
		x->basebandName = basebandName;
 				
 			
		
	

	
  		
		
		
			
				
		x->sideband = sideband;
 				
 			
		
	

	
  		
		
		
			
				
		x->atmPhaseCorrection = atmPhaseCorrection;
 				
 			
		
	

	
  		
		
		
			
				
		x->typeCurve = typeCurve;
 				
 			
		
	

	
  		
		
		
			
				
		x->receiverBand = receiverBand;
 				
 			
		
	

	
  		
		
		
			
		x->startValidTime = startValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
		x->endValidTime = endValidTime.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
		x->antennaNames.length(antennaNames.size());
		for (unsigned int i = 0; i < antennaNames.size(); ++i) {
			
				
			x->antennaNames[i] = CORBA::string_dup(antennaNames.at(i).c_str());
				
	 		
	 	}
			
		
	

	
  		
		
		
			
				
		x->refAntennaName = CORBA::string_dup(refAntennaName.c_str());
				
 			
		
	

	
  		
		
		
			
		x->freqLimits.length(freqLimits.size());
		for (unsigned int i = 0; i < freqLimits.size(); ++i) {
			
			x->freqLimits[i] = freqLimits.at(i).toIDLFrequency();
			
	 	}
			
		
	

	
  		
		
		
			
		x->polarizationTypes.length(polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); ++i) {
			
				
			x->polarizationTypes[i] = polarizationTypes.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->curve.length(curve.size());
		for (unsigned int i = 0; i < curve.size(); i++) {
			x->curve[i].length(curve.at(i).size());
			for (unsigned int j = 0; j < curve.at(i).size(); j++) {
				x->curve[i][j].length(curve.at(i).at(j).size());
			}					 		
		}
		
		for (unsigned int i = 0; i < curve.size() ; i++)
			for (unsigned int j = 0; j < curve.at(i).size(); j++)
				for (unsigned int k = 0; k < curve.at(i).at(j).size(); k++)
					
						
					x->curve[i][j][k] = curve.at(i).at(j).at(k);
		 				
			 									
			
		
	

	
  		
		
		
			
		x->reducedChiSquared.length(reducedChiSquared.size());
		for (unsigned int i = 0; i < reducedChiSquared.size(); ++i) {
			
				
			x->reducedChiSquared[i] = reducedChiSquared.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->numBaselineExists = numBaselineExists;
		
		
			
				
		x->numBaseline = numBaseline;
 				
 			
		
	

	
  		
		
		x->rmsExists = rmsExists;
		
		
			
		x->rms.length(rms.size());
		for (unsigned int i = 0; i < rms.size(); i++) {
			x->rms[i].length(rms.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < rms.size() ; i++)
			for (unsigned int j = 0; j < rms.at(i).size(); j++)
					
						
				x->rms[i][j] = rms.at(i).at(j);
		 				
			 						
		
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->calDataId = calDataId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->calReductionId = calReductionId.toIDLTag();
			
	 	 		
  	

	
		
	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct CalBandpassRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void CalBandpassRow::setFromIDL (CalBandpassRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setBasebandName(x.basebandName);
  			
 		
		
	

	
		
		
			
		setSideband(x.sideband);
  			
 		
		
	

	
		
		
			
		setAtmPhaseCorrection(x.atmPhaseCorrection);
  			
 		
		
	

	
		
		
			
		setTypeCurve(x.typeCurve);
  			
 		
		
	

	
		
		
			
		setReceiverBand(x.receiverBand);
  			
 		
		
	

	
		
		
			
		setStartValidTime(ArrayTime (x.startValidTime));
			
 		
		
	

	
		
		
			
		setEndValidTime(ArrayTime (x.endValidTime));
			
 		
		
	

	
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
	

	
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		antennaNames .clear();
		for (unsigned int i = 0; i <x.antennaNames.length(); ++i) {
			
			antennaNames.push_back(string (x.antennaNames[i]));
			
		}
			
  		
		
	

	
		
		
			
		setRefAntennaName(string (x.refAntennaName));
			
 		
		
	

	
		
		
			
		freqLimits .clear();
		for (unsigned int i = 0; i <x.freqLimits.length(); ++i) {
			
			freqLimits.push_back(Frequency (x.freqLimits[i]));
			
		}
			
  		
		
	

	
		
		
			
		polarizationTypes .clear();
		for (unsigned int i = 0; i <x.polarizationTypes.length(); ++i) {
			
			polarizationTypes.push_back(x.polarizationTypes[i]);
  			
		}
			
  		
		
	

	
		
		
			
		curve .clear();
		vector< vector<float> > vv_aux_curve;
		vector<float> v_aux_curve;
		
		for (unsigned int i = 0; i < x.curve.length(); ++i) {
			vv_aux_curve.clear();
			for (unsigned int j = 0; j < x.curve[0].length(); ++j) {
				v_aux_curve.clear();
				for (unsigned int k = 0; k < x.curve[0][0].length(); ++k) {
					
					v_aux_curve.push_back(x.curve[i][j][k]);
		  			
		  		}
		  		vv_aux_curve.push_back(v_aux_curve);
  			}
  			curve.push_back(vv_aux_curve);
		}
			
  		
		
	

	
		
		
			
		reducedChiSquared .clear();
		for (unsigned int i = 0; i <x.reducedChiSquared.length(); ++i) {
			
			reducedChiSquared.push_back(x.reducedChiSquared[i]);
  			
		}
			
  		
		
	

	
		
		numBaselineExists = x.numBaselineExists;
		if (x.numBaselineExists) {
		
		
			
		setNumBaseline(x.numBaseline);
  			
 		
		
		}
		
	

	
		
		rmsExists = x.rmsExists;
		if (x.rmsExists) {
		
		
			
		rms .clear();
		vector<float> v_aux_rms;
		for (unsigned int i = 0; i < x.rms.length(); ++i) {
			v_aux_rms.clear();
			for (unsigned int j = 0; j < x.rms[0].length(); ++j) {
				
				v_aux_rms.push_back(x.rms[i][j]);
	  			
  			}
  			rms.push_back(v_aux_rms);			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setCalDataId(Tag (x.calDataId));
			
 		
		
	

	
		
		
			
		setCalReductionId(Tag (x.calReductionId));
			
 		
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalBandpass");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string CalBandpassRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
			buf.append(EnumerationParser::toXML("basebandName", basebandName));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("sideband", sideband));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("atmPhaseCorrection", atmPhaseCorrection));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("typeCurve", typeCurve));
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("receiverBand", receiverBand));
		
		
	

  	
 		
		
		Parser::toXML(startValidTime, "startValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(endValidTime, "endValidTime", buf);
		
		
	

  	
 		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
	

  	
 		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(antennaNames, "antennaNames", buf);
		
		
	

  	
 		
		
		Parser::toXML(refAntennaName, "refAntennaName", buf);
		
		
	

  	
 		
		
		Parser::toXML(freqLimits, "freqLimits", buf);
		
		
	

  	
 		
		
			buf.append(EnumerationParser::toXML("polarizationTypes", polarizationTypes));
		
		
	

  	
 		
		
		Parser::toXML(curve, "curve", buf);
		
		
	

  	
 		
		
		Parser::toXML(reducedChiSquared, "reducedChiSquared", buf);
		
		
	

  	
 		
		if (numBaselineExists) {
		
		
		Parser::toXML(numBaseline, "numBaseline", buf);
		
		
		}
		
	

  	
 		
		if (rmsExists) {
		
		
		Parser::toXML(rms, "rms", buf);
		
		
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
	void CalBandpassRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
		
		
		
		basebandName = EnumerationParser::getBasebandName("basebandName","CalBandpass",rowDoc);
		
		
		
	

	
		
		
		
		sideband = EnumerationParser::getNetSideband("sideband","CalBandpass",rowDoc);
		
		
		
	

	
		
		
		
		atmPhaseCorrection = EnumerationParser::getAtmPhaseCorrection("atmPhaseCorrection","CalBandpass",rowDoc);
		
		
		
	

	
		
		
		
		typeCurve = EnumerationParser::getCalCurveType("typeCurve","CalBandpass",rowDoc);
		
		
		
	

	
		
		
		
		receiverBand = EnumerationParser::getReceiverBand("receiverBand","CalBandpass",rowDoc);
		
		
		
	

	
  		
			
	  	setStartValidTime(Parser::getArrayTime("startValidTime","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setEndValidTime(Parser::getArrayTime("endValidTime","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setNumAntenna(Parser::getInteger("numAntenna","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setNumPoly(Parser::getInteger("numPoly","CalBandpass",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","CalBandpass",rowDoc));
			
		
	

	
  		
			
					
	  	setAntennaNames(Parser::get1DString("antennaNames","CalBandpass",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setRefAntennaName(Parser::getString("refAntennaName","CalBandpass",rowDoc));
			
		
	

	
  		
			
					
	  	setFreqLimits(Parser::get1DFrequency("freqLimits","CalBandpass",rowDoc));
	  			
	  		
		
	

	
		
		
		
		polarizationTypes = EnumerationParser::getPolarizationType1D("polarizationTypes","CalBandpass",rowDoc);			
		
		
		
	

	
  		
			
					
	  	setCurve(Parser::get3DFloat("curve","CalBandpass",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setReducedChiSquared(Parser::get1DDouble("reducedChiSquared","CalBandpass",rowDoc));
	  			
	  		
		
	

	
  		
        if (row.isStr("<numBaseline>")) {
			
	  		setNumBaseline(Parser::getInteger("numBaseline","CalBandpass",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<rms>")) {
			
								
	  		setRms(Parser::get2DFloat("rms","CalBandpass",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setCalDataId(Parser::getTag("calDataId","CalData",rowDoc));
			
		
	

	
  		
			
	  	setCalReductionId(Parser::getTag("calReductionId","CalReduction",rowDoc));
			
		
	

	
		
	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"CalBandpass");
		}
	}
	
	void CalBandpassRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
					
			eoss.writeInt(basebandName);
				
		
	

	
	
		
					
			eoss.writeInt(sideband);
				
		
	

	
	
		
					
			eoss.writeInt(atmPhaseCorrection);
				
		
	

	
	
		
					
			eoss.writeInt(typeCurve);
				
		
	

	
	
		
					
			eoss.writeInt(receiverBand);
				
		
	

	
	
		
	calDataId.toBin(eoss);
		
	

	
	
		
	calReductionId.toBin(eoss);
		
	

	
	
		
	startValidTime.toBin(eoss);
		
	

	
	
		
	endValidTime.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(numAntenna);
				
		
	

	
	
		
						
			eoss.writeInt(numPoly);
				
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
		
			
		eoss.writeInt((int) antennaNames.size());
		for (unsigned int i = 0; i < antennaNames.size(); i++)
				
			eoss.writeString(antennaNames.at(i));
				
				
						
		
	

	
	
		
						
			eoss.writeString(refAntennaName);
				
		
	

	
	
		
	Frequency::toBin(freqLimits, eoss);
		
	

	
	
		
		
			
		eoss.writeInt((int) polarizationTypes.size());
		for (unsigned int i = 0; i < polarizationTypes.size(); i++)
				
			eoss.writeInt(polarizationTypes.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) curve.size());
		eoss.writeInt((int) curve.at(0).size());		
		eoss.writeInt((int) curve.at(0).at(0).size());
		for (unsigned int i = 0; i < curve.size(); i++) 
			for (unsigned int j = 0;  j < curve.at(0).size(); j++)
				for (unsigned int k = 0; k <  curve.at(0).at(0).size(); k++)	
							 
					eoss.writeFloat(curve.at(i).at(j).at(k));
						
						
		
	

	
	
		
		
			
		eoss.writeInt((int) reducedChiSquared.size());
		for (unsigned int i = 0; i < reducedChiSquared.size(); i++)
				
			eoss.writeDouble(reducedChiSquared.at(i));
				
				
						
		
	


	
	
	eoss.writeBoolean(numBaselineExists);
	if (numBaselineExists) {
	
	
	
		
						
			eoss.writeInt(numBaseline);
				
		
	

	}

	eoss.writeBoolean(rmsExists);
	if (rmsExists) {
	
	
	
		
		
			
		eoss.writeInt((int) rms.size());
		eoss.writeInt((int) rms.at(0).size());
		for (unsigned int i = 0; i < rms.size(); i++) 
			for (unsigned int j = 0;  j < rms.at(0).size(); j++) 
							 
				eoss.writeFloat(rms.at(i).at(j));
				
	
						
		
	

	}

	}
	
void CalBandpassRow::basebandNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		basebandName = CBasebandName::from_int(eiss.readInt());
			
		
	
	
}
void CalBandpassRow::sidebandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		sideband = CNetSideband::from_int(eiss.readInt());
			
		
	
	
}
void CalBandpassRow::atmPhaseCorrectionFromBin(EndianISStream& eiss) {
		
	
	
		
			
		atmPhaseCorrection = CAtmPhaseCorrection::from_int(eiss.readInt());
			
		
	
	
}
void CalBandpassRow::typeCurveFromBin(EndianISStream& eiss) {
		
	
	
		
			
		typeCurve = CCalCurveType::from_int(eiss.readInt());
			
		
	
	
}
void CalBandpassRow::receiverBandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		receiverBand = CReceiverBand::from_int(eiss.readInt());
			
		
	
	
}
void CalBandpassRow::calDataIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calDataId =  Tag::fromBin(eiss);
		
	
	
}
void CalBandpassRow::calReductionIdFromBin(EndianISStream& eiss) {
		
	
		
		
		calReductionId =  Tag::fromBin(eiss);
		
	
	
}
void CalBandpassRow::startValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		startValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalBandpassRow::endValidTimeFromBin(EndianISStream& eiss) {
		
	
		
		
		endValidTime =  ArrayTime::fromBin(eiss);
		
	
	
}
void CalBandpassRow::numAntennaFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numAntenna =  eiss.readInt();
			
		
	
	
}
void CalBandpassRow::numPolyFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numPoly =  eiss.readInt();
			
		
	
	
}
void CalBandpassRow::numReceptorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	
	
}
void CalBandpassRow::antennaNamesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		antennaNames.clear();
		
		unsigned int antennaNamesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < antennaNamesDim1; i++)
			
			antennaNames.push_back(eiss.readString());
			
	

		
	
	
}
void CalBandpassRow::refAntennaNameFromBin(EndianISStream& eiss) {
		
	
	
		
			
		refAntennaName =  eiss.readString();
			
		
	
	
}
void CalBandpassRow::freqLimitsFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	freqLimits = Frequency::from1DBin(eiss);	
	

		
	
	
}
void CalBandpassRow::polarizationTypesFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		polarizationTypes.clear();
		
		unsigned int polarizationTypesDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < polarizationTypesDim1; i++)
			
			polarizationTypes.push_back(CPolarizationType::from_int(eiss.readInt()));
			
	

		
	
	
}
void CalBandpassRow::curveFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		curve.clear();
			
		unsigned int curveDim1 = eiss.readInt();
		unsigned int curveDim2 = eiss.readInt();
		unsigned int curveDim3 = eiss.readInt();
		vector <vector<float> > curveAux2;
		vector <float> curveAux1;
		for (unsigned int i = 0; i < curveDim1; i++) {
			curveAux2.clear();
			for (unsigned int j = 0; j < curveDim2 ; j++) {
				curveAux1.clear();
				for (unsigned int k = 0; k < curveDim3; k++) {
			
					curveAux1.push_back(eiss.readFloat());
			
				}
				curveAux2.push_back(curveAux1);
			}
			curve.push_back(curveAux2);
		}	
	

		
	
	
}
void CalBandpassRow::reducedChiSquaredFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		reducedChiSquared.clear();
		
		unsigned int reducedChiSquaredDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < reducedChiSquaredDim1; i++)
			
			reducedChiSquared.push_back(eiss.readDouble());
			
	

		
	
	
}

void CalBandpassRow::numBaselineFromBin(EndianISStream& eiss) {
		
	numBaselineExists = eiss.readBoolean();
	if (numBaselineExists) {
		
	
	
		
			
		numBaseline =  eiss.readInt();
			
		
	

	}
	
}
void CalBandpassRow::rmsFromBin(EndianISStream& eiss) {
		
	rmsExists = eiss.readBoolean();
	if (rmsExists) {
		
	
	
		
			
	
		rms.clear();
		
		unsigned int rmsDim1 = eiss.readInt();
		unsigned int rmsDim2 = eiss.readInt();
		vector <float> rmsAux1;
		for (unsigned int i = 0; i < rmsDim1; i++) {
			rmsAux1.clear();
			for (unsigned int j = 0; j < rmsDim2 ; j++)			
			
			rmsAux1.push_back(eiss.readFloat());
			
			rms.push_back(rmsAux1);
		}
	
	

		
	

	}
	
}
	
	
	CalBandpassRow* CalBandpassRow::fromBin(EndianISStream& eiss, CalBandpassTable& table, const vector<string>& attributesSeq) {
		CalBandpassRow* row = new  CalBandpassRow(table);
		
		map<string, CalBandpassAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "CalBandpassTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get basebandName.
 	 * @return basebandName as BasebandNameMod::BasebandName
 	 */
 	BasebandNameMod::BasebandName CalBandpassRow::getBasebandName() const {
	
  		return basebandName;
 	}

 	/**
 	 * Set basebandName with the specified BasebandNameMod::BasebandName.
 	 * @param basebandName The BasebandNameMod::BasebandName value to which basebandName is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setBasebandName (BasebandNameMod::BasebandName basebandName)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("basebandName", "CalBandpass");
		
  		}
  	
 		this->basebandName = basebandName;
	
 	}
	
	

	

	
 	/**
 	 * Get sideband.
 	 * @return sideband as NetSidebandMod::NetSideband
 	 */
 	NetSidebandMod::NetSideband CalBandpassRow::getSideband() const {
	
  		return sideband;
 	}

 	/**
 	 * Set sideband with the specified NetSidebandMod::NetSideband.
 	 * @param sideband The NetSidebandMod::NetSideband value to which sideband is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setSideband (NetSidebandMod::NetSideband sideband)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("sideband", "CalBandpass");
		
  		}
  	
 		this->sideband = sideband;
	
 	}
	
	

	

	
 	/**
 	 * Get atmPhaseCorrection.
 	 * @return atmPhaseCorrection as AtmPhaseCorrectionMod::AtmPhaseCorrection
 	 */
 	AtmPhaseCorrectionMod::AtmPhaseCorrection CalBandpassRow::getAtmPhaseCorrection() const {
	
  		return atmPhaseCorrection;
 	}

 	/**
 	 * Set atmPhaseCorrection with the specified AtmPhaseCorrectionMod::AtmPhaseCorrection.
 	 * @param atmPhaseCorrection The AtmPhaseCorrectionMod::AtmPhaseCorrection value to which atmPhaseCorrection is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setAtmPhaseCorrection (AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("atmPhaseCorrection", "CalBandpass");
		
  		}
  	
 		this->atmPhaseCorrection = atmPhaseCorrection;
	
 	}
	
	

	

	
 	/**
 	 * Get typeCurve.
 	 * @return typeCurve as CalCurveTypeMod::CalCurveType
 	 */
 	CalCurveTypeMod::CalCurveType CalBandpassRow::getTypeCurve() const {
	
  		return typeCurve;
 	}

 	/**
 	 * Set typeCurve with the specified CalCurveTypeMod::CalCurveType.
 	 * @param typeCurve The CalCurveTypeMod::CalCurveType value to which typeCurve is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setTypeCurve (CalCurveTypeMod::CalCurveType typeCurve)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("typeCurve", "CalBandpass");
		
  		}
  	
 		this->typeCurve = typeCurve;
	
 	}
	
	

	

	
 	/**
 	 * Get receiverBand.
 	 * @return receiverBand as ReceiverBandMod::ReceiverBand
 	 */
 	ReceiverBandMod::ReceiverBand CalBandpassRow::getReceiverBand() const {
	
  		return receiverBand;
 	}

 	/**
 	 * Set receiverBand with the specified ReceiverBandMod::ReceiverBand.
 	 * @param receiverBand The ReceiverBandMod::ReceiverBand value to which receiverBand is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setReceiverBand (ReceiverBandMod::ReceiverBand receiverBand)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("receiverBand", "CalBandpass");
		
  		}
  	
 		this->receiverBand = receiverBand;
	
 	}
	
	

	

	
 	/**
 	 * Get startValidTime.
 	 * @return startValidTime as ArrayTime
 	 */
 	ArrayTime CalBandpassRow::getStartValidTime() const {
	
  		return startValidTime;
 	}

 	/**
 	 * Set startValidTime with the specified ArrayTime.
 	 * @param startValidTime The ArrayTime value to which startValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setStartValidTime (ArrayTime startValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->startValidTime = startValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get endValidTime.
 	 * @return endValidTime as ArrayTime
 	 */
 	ArrayTime CalBandpassRow::getEndValidTime() const {
	
  		return endValidTime;
 	}

 	/**
 	 * Set endValidTime with the specified ArrayTime.
 	 * @param endValidTime The ArrayTime value to which endValidTime is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setEndValidTime (ArrayTime endValidTime)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->endValidTime = endValidTime;
	
 	}
	
	

	

	
 	/**
 	 * Get numAntenna.
 	 * @return numAntenna as int
 	 */
 	int CalBandpassRow::getNumAntenna() const {
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumAntenna (int numAntenna)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numAntenna = numAntenna;
	
 	}
	
	

	

	
 	/**
 	 * Get numPoly.
 	 * @return numPoly as int
 	 */
 	int CalBandpassRow::getNumPoly() const {
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumPoly (int numPoly)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numPoly = numPoly;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int CalBandpassRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get antennaNames.
 	 * @return antennaNames as vector<string >
 	 */
 	vector<string > CalBandpassRow::getAntennaNames() const {
	
  		return antennaNames;
 	}

 	/**
 	 * Set antennaNames with the specified vector<string >.
 	 * @param antennaNames The vector<string > value to which antennaNames is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setAntennaNames (vector<string > antennaNames)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->antennaNames = antennaNames;
	
 	}
	
	

	

	
 	/**
 	 * Get refAntennaName.
 	 * @return refAntennaName as string
 	 */
 	string CalBandpassRow::getRefAntennaName() const {
	
  		return refAntennaName;
 	}

 	/**
 	 * Set refAntennaName with the specified string.
 	 * @param refAntennaName The string value to which refAntennaName is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setRefAntennaName (string refAntennaName)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->refAntennaName = refAntennaName;
	
 	}
	
	

	

	
 	/**
 	 * Get freqLimits.
 	 * @return freqLimits as vector<Frequency >
 	 */
 	vector<Frequency > CalBandpassRow::getFreqLimits() const {
	
  		return freqLimits;
 	}

 	/**
 	 * Set freqLimits with the specified vector<Frequency >.
 	 * @param freqLimits The vector<Frequency > value to which freqLimits is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setFreqLimits (vector<Frequency > freqLimits)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->freqLimits = freqLimits;
	
 	}
	
	

	

	
 	/**
 	 * Get polarizationTypes.
 	 * @return polarizationTypes as vector<PolarizationTypeMod::PolarizationType >
 	 */
 	vector<PolarizationTypeMod::PolarizationType > CalBandpassRow::getPolarizationTypes() const {
	
  		return polarizationTypes;
 	}

 	/**
 	 * Set polarizationTypes with the specified vector<PolarizationTypeMod::PolarizationType >.
 	 * @param polarizationTypes The vector<PolarizationTypeMod::PolarizationType > value to which polarizationTypes is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setPolarizationTypes (vector<PolarizationTypeMod::PolarizationType > polarizationTypes)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->polarizationTypes = polarizationTypes;
	
 	}
	
	

	

	
 	/**
 	 * Get curve.
 	 * @return curve as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > CalBandpassRow::getCurve() const {
	
  		return curve;
 	}

 	/**
 	 * Set curve with the specified vector<vector<vector<float > > >.
 	 * @param curve The vector<vector<vector<float > > > value to which curve is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setCurve (vector<vector<vector<float > > > curve)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->curve = curve;
	
 	}
	
	

	

	
 	/**
 	 * Get reducedChiSquared.
 	 * @return reducedChiSquared as vector<double >
 	 */
 	vector<double > CalBandpassRow::getReducedChiSquared() const {
	
  		return reducedChiSquared;
 	}

 	/**
 	 * Set reducedChiSquared with the specified vector<double >.
 	 * @param reducedChiSquared The vector<double > value to which reducedChiSquared is to be set.
 	 
 	
 		
 	 */
 	void CalBandpassRow::setReducedChiSquared (vector<double > reducedChiSquared)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->reducedChiSquared = reducedChiSquared;
	
 	}
	
	

	
	/**
	 * The attribute numBaseline is optional. Return true if this attribute exists.
	 * @return true if and only if the numBaseline attribute exists. 
	 */
	bool CalBandpassRow::isNumBaselineExists() const {
		return numBaselineExists;
	}
	

	
 	/**
 	 * Get numBaseline, which is optional.
 	 * @return numBaseline as int
 	 * @throw IllegalAccessException If numBaseline does not exist.
 	 */
 	int CalBandpassRow::getNumBaseline() const  {
		if (!numBaselineExists) {
			throw IllegalAccessException("numBaseline", "CalBandpass");
		}
	
  		return numBaseline;
 	}

 	/**
 	 * Set numBaseline with the specified int.
 	 * @param numBaseline The int value to which numBaseline is to be set.
 	 
 	
 	 */
 	void CalBandpassRow::setNumBaseline (int numBaseline) {
	
 		this->numBaseline = numBaseline;
	
		numBaselineExists = true;
	
 	}
	
	
	/**
	 * Mark numBaseline, which is an optional field, as non-existent.
	 */
	void CalBandpassRow::clearNumBaseline () {
		numBaselineExists = false;
	}
	

	
	/**
	 * The attribute rms is optional. Return true if this attribute exists.
	 * @return true if and only if the rms attribute exists. 
	 */
	bool CalBandpassRow::isRmsExists() const {
		return rmsExists;
	}
	

	
 	/**
 	 * Get rms, which is optional.
 	 * @return rms as vector<vector<float > >
 	 * @throw IllegalAccessException If rms does not exist.
 	 */
 	vector<vector<float > > CalBandpassRow::getRms() const  {
		if (!rmsExists) {
			throw IllegalAccessException("rms", "CalBandpass");
		}
	
  		return rms;
 	}

 	/**
 	 * Set rms with the specified vector<vector<float > >.
 	 * @param rms The vector<vector<float > > value to which rms is to be set.
 	 
 	
 	 */
 	void CalBandpassRow::setRms (vector<vector<float > > rms) {
	
 		this->rms = rms;
	
		rmsExists = true;
	
 	}
	
	
	/**
	 * Mark rms, which is an optional field, as non-existent.
	 */
	void CalBandpassRow::clearRms () {
		rmsExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get calDataId.
 	 * @return calDataId as Tag
 	 */
 	Tag CalBandpassRow::getCalDataId() const {
	
  		return calDataId;
 	}

 	/**
 	 * Set calDataId with the specified Tag.
 	 * @param calDataId The Tag value to which calDataId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setCalDataId (Tag calDataId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calDataId", "CalBandpass");
		
  		}
  	
 		this->calDataId = calDataId;
	
 	}
	
	

	

	
 	/**
 	 * Get calReductionId.
 	 * @return calReductionId as Tag
 	 */
 	Tag CalBandpassRow::getCalReductionId() const {
	
  		return calReductionId;
 	}

 	/**
 	 * Set calReductionId with the specified Tag.
 	 * @param calReductionId The Tag value to which calReductionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void CalBandpassRow::setCalReductionId (Tag calReductionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("calReductionId", "CalBandpass");
		
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
	 CalReductionRow* CalBandpassRow::getCalReductionUsingCalReductionId() {
	 
	 	return table.getContainer().getCalReduction().getRowByKey(calReductionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the CalData table having CalData.calDataId == calDataId
	 * @return a CalDataRow*
	 * 
	 
	 */
	 CalDataRow* CalBandpassRow::getCalDataUsingCalDataId() {
	 
	 	return table.getContainer().getCalData().getRowByKey(calDataId);
	 }
	 

	

	
	/**
	 * Create a CalBandpassRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	CalBandpassRow::CalBandpassRow (CalBandpassTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		numBaselineExists = false;
	

	
		rmsExists = false;
	

	
	

	

	
	
	
	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
basebandName = CBasebandName::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
sideband = CNetSideband::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
atmPhaseCorrection = CAtmPhaseCorrection::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
typeCurve = CCalCurveType::from_int(0);
	

	
// This attribute is scalar and has an enumeration type. Let's initialize it to some valid value (the 1st of the enumeration).		
receiverBand = CReceiverBand::from_int(0);
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["basebandName"] = &CalBandpassRow::basebandNameFromBin; 
	 fromBinMethods["sideband"] = &CalBandpassRow::sidebandFromBin; 
	 fromBinMethods["atmPhaseCorrection"] = &CalBandpassRow::atmPhaseCorrectionFromBin; 
	 fromBinMethods["typeCurve"] = &CalBandpassRow::typeCurveFromBin; 
	 fromBinMethods["receiverBand"] = &CalBandpassRow::receiverBandFromBin; 
	 fromBinMethods["calDataId"] = &CalBandpassRow::calDataIdFromBin; 
	 fromBinMethods["calReductionId"] = &CalBandpassRow::calReductionIdFromBin; 
	 fromBinMethods["startValidTime"] = &CalBandpassRow::startValidTimeFromBin; 
	 fromBinMethods["endValidTime"] = &CalBandpassRow::endValidTimeFromBin; 
	 fromBinMethods["numAntenna"] = &CalBandpassRow::numAntennaFromBin; 
	 fromBinMethods["numPoly"] = &CalBandpassRow::numPolyFromBin; 
	 fromBinMethods["numReceptor"] = &CalBandpassRow::numReceptorFromBin; 
	 fromBinMethods["antennaNames"] = &CalBandpassRow::antennaNamesFromBin; 
	 fromBinMethods["refAntennaName"] = &CalBandpassRow::refAntennaNameFromBin; 
	 fromBinMethods["freqLimits"] = &CalBandpassRow::freqLimitsFromBin; 
	 fromBinMethods["polarizationTypes"] = &CalBandpassRow::polarizationTypesFromBin; 
	 fromBinMethods["curve"] = &CalBandpassRow::curveFromBin; 
	 fromBinMethods["reducedChiSquared"] = &CalBandpassRow::reducedChiSquaredFromBin; 
		
	
	 fromBinMethods["numBaseline"] = &CalBandpassRow::numBaselineFromBin; 
	 fromBinMethods["rms"] = &CalBandpassRow::rmsFromBin; 
	
	}
	
	CalBandpassRow::CalBandpassRow (CalBandpassTable &t, CalBandpassRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
		numBaselineExists = false;
	

	
		rmsExists = false;
	

	
	

	
		
		}
		else {
	
		
			basebandName = row.basebandName;
		
			sideband = row.sideband;
		
			atmPhaseCorrection = row.atmPhaseCorrection;
		
			typeCurve = row.typeCurve;
		
			receiverBand = row.receiverBand;
		
			calDataId = row.calDataId;
		
			calReductionId = row.calReductionId;
		
		
		
		
			startValidTime = row.startValidTime;
		
			endValidTime = row.endValidTime;
		
			numAntenna = row.numAntenna;
		
			numPoly = row.numPoly;
		
			numReceptor = row.numReceptor;
		
			antennaNames = row.antennaNames;
		
			refAntennaName = row.refAntennaName;
		
			freqLimits = row.freqLimits;
		
			polarizationTypes = row.polarizationTypes;
		
			curve = row.curve;
		
			reducedChiSquared = row.reducedChiSquared;
		
		
		
		
		if (row.numBaselineExists) {
			numBaseline = row.numBaseline;		
			numBaselineExists = true;
		}
		else
			numBaselineExists = false;
		
		if (row.rmsExists) {
			rms = row.rms;		
			rmsExists = true;
		}
		else
			rmsExists = false;
		
		}
		
		 fromBinMethods["basebandName"] = &CalBandpassRow::basebandNameFromBin; 
		 fromBinMethods["sideband"] = &CalBandpassRow::sidebandFromBin; 
		 fromBinMethods["atmPhaseCorrection"] = &CalBandpassRow::atmPhaseCorrectionFromBin; 
		 fromBinMethods["typeCurve"] = &CalBandpassRow::typeCurveFromBin; 
		 fromBinMethods["receiverBand"] = &CalBandpassRow::receiverBandFromBin; 
		 fromBinMethods["calDataId"] = &CalBandpassRow::calDataIdFromBin; 
		 fromBinMethods["calReductionId"] = &CalBandpassRow::calReductionIdFromBin; 
		 fromBinMethods["startValidTime"] = &CalBandpassRow::startValidTimeFromBin; 
		 fromBinMethods["endValidTime"] = &CalBandpassRow::endValidTimeFromBin; 
		 fromBinMethods["numAntenna"] = &CalBandpassRow::numAntennaFromBin; 
		 fromBinMethods["numPoly"] = &CalBandpassRow::numPolyFromBin; 
		 fromBinMethods["numReceptor"] = &CalBandpassRow::numReceptorFromBin; 
		 fromBinMethods["antennaNames"] = &CalBandpassRow::antennaNamesFromBin; 
		 fromBinMethods["refAntennaName"] = &CalBandpassRow::refAntennaNameFromBin; 
		 fromBinMethods["freqLimits"] = &CalBandpassRow::freqLimitsFromBin; 
		 fromBinMethods["polarizationTypes"] = &CalBandpassRow::polarizationTypesFromBin; 
		 fromBinMethods["curve"] = &CalBandpassRow::curveFromBin; 
		 fromBinMethods["reducedChiSquared"] = &CalBandpassRow::reducedChiSquaredFromBin; 
			
	
		 fromBinMethods["numBaseline"] = &CalBandpassRow::numBaselineFromBin; 
		 fromBinMethods["rms"] = &CalBandpassRow::rmsFromBin; 
			
	}

	
	bool CalBandpassRow::compareNoAutoInc(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband sideband, AtmPhaseCorrectionMod::AtmPhaseCorrection atmPhaseCorrection, CalCurveTypeMod::CalCurveType typeCurve, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<Frequency > freqLimits, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared) {
		bool result;
		result = true;
		
	
		
		result = result && (this->basebandName == basebandName);
		
		if (!result) return false;
	

	
		
		result = result && (this->sideband == sideband);
		
		if (!result) return false;
	

	
		
		result = result && (this->atmPhaseCorrection == atmPhaseCorrection);
		
		if (!result) return false;
	

	
		
		result = result && (this->typeCurve == typeCurve);
		
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
	

	
		
		result = result && (this->numAntenna == numAntenna);
		
		if (!result) return false;
	

	
		
		result = result && (this->numPoly == numPoly);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->antennaNames == antennaNames);
		
		if (!result) return false;
	

	
		
		result = result && (this->refAntennaName == refAntennaName);
		
		if (!result) return false;
	

	
		
		result = result && (this->freqLimits == freqLimits);
		
		if (!result) return false;
	

	
		
		result = result && (this->polarizationTypes == polarizationTypes);
		
		if (!result) return false;
	

	
		
		result = result && (this->curve == curve);
		
		if (!result) return false;
	

	
		
		result = result && (this->reducedChiSquared == reducedChiSquared);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool CalBandpassRow::compareRequiredValue(ArrayTime startValidTime, ArrayTime endValidTime, int numAntenna, int numPoly, int numReceptor, vector<string > antennaNames, string refAntennaName, vector<Frequency > freqLimits, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<vector<float > > > curve, vector<double > reducedChiSquared) {
		bool result;
		result = true;
		
	
		if (!(this->startValidTime == startValidTime)) return false;
	

	
		if (!(this->endValidTime == endValidTime)) return false;
	

	
		if (!(this->numAntenna == numAntenna)) return false;
	

	
		if (!(this->numPoly == numPoly)) return false;
	

	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->antennaNames == antennaNames)) return false;
	

	
		if (!(this->refAntennaName == refAntennaName)) return false;
	

	
		if (!(this->freqLimits == freqLimits)) return false;
	

	
		if (!(this->polarizationTypes == polarizationTypes)) return false;
	

	
		if (!(this->curve == curve)) return false;
	

	
		if (!(this->reducedChiSquared == reducedChiSquared)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the CalBandpassRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool CalBandpassRow::equalByRequiredValue(CalBandpassRow* x) {
		
			
		if (this->startValidTime != x->startValidTime) return false;
			
		if (this->endValidTime != x->endValidTime) return false;
			
		if (this->numAntenna != x->numAntenna) return false;
			
		if (this->numPoly != x->numPoly) return false;
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->antennaNames != x->antennaNames) return false;
			
		if (this->refAntennaName != x->refAntennaName) return false;
			
		if (this->freqLimits != x->freqLimits) return false;
			
		if (this->polarizationTypes != x->polarizationTypes) return false;
			
		if (this->curve != x->curve) return false;
			
		if (this->reducedChiSquared != x->reducedChiSquared) return false;
			
		
		return true;
	}	
	
/*
	 map<string, CalBandpassAttributeFromBin> CalBandpassRow::initFromBinMethods() {
		map<string, CalBandpassAttributeFromBin> result;
		
		result["basebandName"] = &CalBandpassRow::basebandNameFromBin;
		result["sideband"] = &CalBandpassRow::sidebandFromBin;
		result["atmPhaseCorrection"] = &CalBandpassRow::atmPhaseCorrectionFromBin;
		result["typeCurve"] = &CalBandpassRow::typeCurveFromBin;
		result["receiverBand"] = &CalBandpassRow::receiverBandFromBin;
		result["calDataId"] = &CalBandpassRow::calDataIdFromBin;
		result["calReductionId"] = &CalBandpassRow::calReductionIdFromBin;
		result["startValidTime"] = &CalBandpassRow::startValidTimeFromBin;
		result["endValidTime"] = &CalBandpassRow::endValidTimeFromBin;
		result["numAntenna"] = &CalBandpassRow::numAntennaFromBin;
		result["numPoly"] = &CalBandpassRow::numPolyFromBin;
		result["numReceptor"] = &CalBandpassRow::numReceptorFromBin;
		result["antennaNames"] = &CalBandpassRow::antennaNamesFromBin;
		result["refAntennaName"] = &CalBandpassRow::refAntennaNameFromBin;
		result["freqLimits"] = &CalBandpassRow::freqLimitsFromBin;
		result["polarizationTypes"] = &CalBandpassRow::polarizationTypesFromBin;
		result["curve"] = &CalBandpassRow::curveFromBin;
		result["reducedChiSquared"] = &CalBandpassRow::reducedChiSquaredFromBin;
		
		
		result["numBaseline"] = &CalBandpassRow::numBaselineFromBin;
		result["rms"] = &CalBandpassRow::rmsFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
