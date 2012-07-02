
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
 * File TotalPowerRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <TotalPowerRow.h>
#include <TotalPowerTable.h>

#include <StateTable.h>
#include <StateRow.h>

#include <FieldTable.h>
#include <FieldRow.h>

#include <ConfigDescriptionTable.h>
#include <ConfigDescriptionRow.h>

#include <ExecBlockTable.h>
#include <ExecBlockRow.h>
	

using asdm::ASDM;
using asdm::TotalPowerRow;
using asdm::TotalPowerTable;

using asdm::StateTable;
using asdm::StateRow;

using asdm::FieldTable;
using asdm::FieldRow;

using asdm::ConfigDescriptionTable;
using asdm::ConfigDescriptionRow;

using asdm::ExecBlockTable;
using asdm::ExecBlockRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	TotalPowerRow::~TotalPowerRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	TotalPowerTable &TotalPowerRow::getTable() const {
		return table;
	}
	
	void TotalPowerRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a TotalPowerRowIDL struct.
	 */
	TotalPowerRowIDL *TotalPowerRow::toIDL() const {
		TotalPowerRowIDL *x = new TotalPowerRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->time = time.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->scanNumber = scanNumber;
 				
 			
		
	

	
  		
		
		
			
				
		x->subscanNumber = subscanNumber;
 				
 			
		
	

	
  		
		
		
			
				
		x->integrationNumber = integrationNumber;
 				
 			
		
	

	
  		
		
		
			
		x->uvw.length(uvw.size());
		for (unsigned int i = 0; i < uvw.size(); i++) {
			x->uvw[i].length(uvw.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < uvw.size() ; i++)
			for (unsigned int j = 0; j < uvw.at(i).size(); j++)
					
				x->uvw[i][j]= uvw.at(i).at(j).toIDLLength();
									
		
			
		
	

	
  		
		
		
			
		x->exposure.length(exposure.size());
		for (unsigned int i = 0; i < exposure.size(); i++) {
			x->exposure[i].length(exposure.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < exposure.size() ; i++)
			for (unsigned int j = 0; j < exposure.at(i).size(); j++)
					
				x->exposure[i][j]= exposure.at(i).at(j).toIDLInterval();
									
		
			
		
	

	
  		
		
		
			
		x->timeCentroid.length(timeCentroid.size());
		for (unsigned int i = 0; i < timeCentroid.size(); i++) {
			x->timeCentroid[i].length(timeCentroid.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < timeCentroid.size() ; i++)
			for (unsigned int j = 0; j < timeCentroid.at(i).size(); j++)
					
				x->timeCentroid[i][j]= timeCentroid.at(i).at(j).toIDLArrayTime();
									
		
			
		
	

	
  		
		
		
			
		x->floatData.length(floatData.size());
		for (unsigned int i = 0; i < floatData.size(); i++) {
			x->floatData[i].length(floatData.at(i).size());
			for (unsigned int j = 0; j < floatData.at(i).size(); j++) {
				x->floatData[i][j].length(floatData.at(i).at(j).size());
			}					 		
		}
		
		for (unsigned int i = 0; i < floatData.size() ; i++)
			for (unsigned int j = 0; j < floatData.at(i).size(); j++)
				for (unsigned int k = 0; k < floatData.at(i).at(j).size(); k++)
					
						
					x->floatData[i][j][k] = floatData.at(i).at(j).at(k);
		 				
			 									
			
		
	

	
  		
		
		
			
		x->flagAnt.length(flagAnt.size());
		for (unsigned int i = 0; i < flagAnt.size(); ++i) {
			
				
			x->flagAnt[i] = flagAnt.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		
			
		x->flagPol.length(flagPol.size());
		for (unsigned int i = 0; i < flagPol.size(); i++) {
			x->flagPol[i].length(flagPol.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < flagPol.size() ; i++)
			for (unsigned int j = 0; j < flagPol.at(i).size(); j++)
					
						
				x->flagPol[i][j] = flagPol.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		
			
				
		x->flagRow = flagRow;
 				
 			
		
	

	
  		
		
		
			
		x->interval = interval.toIDLInterval();
			
		
	

	
  		
		
		x->subintegrationNumberExists = subintegrationNumberExists;
		
		
			
				
		x->subintegrationNumber = subintegrationNumber;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->configDescriptionId = configDescriptionId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->execBlockId = execBlockId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->fieldId = fieldId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
		
		x->stateId.length(stateId.size());
		for (unsigned int i = 0; i < stateId.size(); ++i) {
			
			x->stateId[i] = stateId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
		
	

	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct TotalPowerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void TotalPowerRow::setFromIDL (TotalPowerRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTime(ArrayTime (x.time));
			
 		
		
	

	
		
		
			
		setScanNumber(x.scanNumber);
  			
 		
		
	

	
		
		
			
		setSubscanNumber(x.subscanNumber);
  			
 		
		
	

	
		
		
			
		setIntegrationNumber(x.integrationNumber);
  			
 		
		
	

	
		
		
			
		uvw .clear();
		vector<Length> v_aux_uvw;
		for (unsigned int i = 0; i < x.uvw.length(); ++i) {
			v_aux_uvw.clear();
			for (unsigned int j = 0; j < x.uvw[0].length(); ++j) {
				
				v_aux_uvw.push_back(Length (x.uvw[i][j]));
				
  			}
  			uvw.push_back(v_aux_uvw);			
		}
			
  		
		
	

	
		
		
			
		exposure .clear();
		vector<Interval> v_aux_exposure;
		for (unsigned int i = 0; i < x.exposure.length(); ++i) {
			v_aux_exposure.clear();
			for (unsigned int j = 0; j < x.exposure[0].length(); ++j) {
				
				v_aux_exposure.push_back(Interval (x.exposure[i][j]));
				
  			}
  			exposure.push_back(v_aux_exposure);			
		}
			
  		
		
	

	
		
		
			
		timeCentroid .clear();
		vector<ArrayTime> v_aux_timeCentroid;
		for (unsigned int i = 0; i < x.timeCentroid.length(); ++i) {
			v_aux_timeCentroid.clear();
			for (unsigned int j = 0; j < x.timeCentroid[0].length(); ++j) {
				
				v_aux_timeCentroid.push_back(ArrayTime (x.timeCentroid[i][j]));
				
  			}
  			timeCentroid.push_back(v_aux_timeCentroid);			
		}
			
  		
		
	

	
		
		
			
		floatData .clear();
		vector< vector<float> > vv_aux_floatData;
		vector<float> v_aux_floatData;
		
		for (unsigned int i = 0; i < x.floatData.length(); ++i) {
			vv_aux_floatData.clear();
			for (unsigned int j = 0; j < x.floatData[0].length(); ++j) {
				v_aux_floatData.clear();
				for (unsigned int k = 0; k < x.floatData[0][0].length(); ++k) {
					
					v_aux_floatData.push_back(x.floatData[i][j][k]);
		  			
		  		}
		  		vv_aux_floatData.push_back(v_aux_floatData);
  			}
  			floatData.push_back(vv_aux_floatData);
		}
			
  		
		
	

	
		
		
			
		flagAnt .clear();
		for (unsigned int i = 0; i <x.flagAnt.length(); ++i) {
			
			flagAnt.push_back(x.flagAnt[i]);
  			
		}
			
  		
		
	

	
		
		
			
		flagPol .clear();
		vector<int> v_aux_flagPol;
		for (unsigned int i = 0; i < x.flagPol.length(); ++i) {
			v_aux_flagPol.clear();
			for (unsigned int j = 0; j < x.flagPol[0].length(); ++j) {
				
				v_aux_flagPol.push_back(x.flagPol[i][j]);
	  			
  			}
  			flagPol.push_back(v_aux_flagPol);			
		}
			
  		
		
	

	
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
	

	
		
		
			
		setInterval(Interval (x.interval));
			
 		
		
	

	
		
		subintegrationNumberExists = x.subintegrationNumberExists;
		if (x.subintegrationNumberExists) {
		
		
			
		setSubintegrationNumber(x.subintegrationNumber);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setConfigDescriptionId(Tag (x.configDescriptionId));
			
 		
		
	

	
		
		
			
		setExecBlockId(Tag (x.execBlockId));
			
 		
		
	

	
		
		
			
		setFieldId(Tag (x.fieldId));
			
 		
		
	

	
		
		stateId .clear();
		for (unsigned int i = 0; i <x.stateId.length(); ++i) {
			
			stateId.push_back(Tag (x.stateId[i]));
			
		}
		
  	

	
		
	

	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"TotalPower");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string TotalPowerRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(time, "time", buf);
		
		
	

  	
 		
		
		Parser::toXML(scanNumber, "scanNumber", buf);
		
		
	

  	
 		
		
		Parser::toXML(subscanNumber, "subscanNumber", buf);
		
		
	

  	
 		
		
		Parser::toXML(integrationNumber, "integrationNumber", buf);
		
		
	

  	
 		
		
		Parser::toXML(uvw, "uvw", buf);
		
		
	

  	
 		
		
		Parser::toXML(exposure, "exposure", buf);
		
		
	

  	
 		
		
		Parser::toXML(timeCentroid, "timeCentroid", buf);
		
		
	

  	
 		
		
		Parser::toXML(floatData, "floatData", buf);
		
		
	

  	
 		
		
		Parser::toXML(flagAnt, "flagAnt", buf);
		
		
	

  	
 		
		
		Parser::toXML(flagPol, "flagPol", buf);
		
		
	

  	
 		
		
		Parser::toXML(flagRow, "flagRow", buf);
		
		
	

  	
 		
		
		Parser::toXML(interval, "interval", buf);
		
		
	

  	
 		
		if (subintegrationNumberExists) {
		
		
		Parser::toXML(subintegrationNumber, "subintegrationNumber", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(configDescriptionId, "configDescriptionId", buf);
		
		
	

  	
 		
		
		Parser::toXML(execBlockId, "execBlockId", buf);
		
		
	

  	
 		
		
		Parser::toXML(fieldId, "fieldId", buf);
		
		
	

  	
 		
		
		Parser::toXML(stateId, "stateId", buf);
		
		
	

	
		
	

	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void TotalPowerRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTime(Parser::getArrayTime("time","TotalPower",rowDoc));
			
		
	

	
  		
			
	  	setScanNumber(Parser::getInteger("scanNumber","TotalPower",rowDoc));
			
		
	

	
  		
			
	  	setSubscanNumber(Parser::getInteger("subscanNumber","TotalPower",rowDoc));
			
		
	

	
  		
			
	  	setIntegrationNumber(Parser::getInteger("integrationNumber","TotalPower",rowDoc));
			
		
	

	
  		
			
					
	  	setUvw(Parser::get2DLength("uvw","TotalPower",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setExposure(Parser::get2DInterval("exposure","TotalPower",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setTimeCentroid(Parser::get2DArrayTime("timeCentroid","TotalPower",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFloatData(Parser::get3DFloat("floatData","TotalPower",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFlagAnt(Parser::get1DInteger("flagAnt","TotalPower",rowDoc));
	  			
	  		
		
	

	
  		
			
					
	  	setFlagPol(Parser::get2DInteger("flagPol","TotalPower",rowDoc));
	  			
	  		
		
	

	
  		
			
	  	setFlagRow(Parser::getBoolean("flagRow","TotalPower",rowDoc));
			
		
	

	
  		
			
	  	setInterval(Parser::getInterval("interval","TotalPower",rowDoc));
			
		
	

	
  		
        if (row.isStr("<subintegrationNumber>")) {
			
	  		setSubintegrationNumber(Parser::getInteger("subintegrationNumber","TotalPower",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setConfigDescriptionId(Parser::getTag("configDescriptionId","Processor",rowDoc));
			
		
	

	
  		
			
	  	setExecBlockId(Parser::getTag("execBlockId","TotalPower",rowDoc));
			
		
	

	
  		
			
	  	setFieldId(Parser::getTag("fieldId","Field",rowDoc));
			
		
	

	
  		 
  		setStateId(Parser::get1DTag("stateId","TotalPower",rowDoc));
		
  	

	
		
	

	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"TotalPower");
		}
	}
	
	void TotalPowerRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	time.toBin(eoss);
		
	

	
	
		
	configDescriptionId.toBin(eoss);
		
	

	
	
		
	fieldId.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(scanNumber);
				
		
	

	
	
		
						
			eoss.writeInt(subscanNumber);
				
		
	

	
	
		
						
			eoss.writeInt(integrationNumber);
				
		
	

	
	
		
	Length::toBin(uvw, eoss);
		
	

	
	
		
	Interval::toBin(exposure, eoss);
		
	

	
	
		
	ArrayTime::toBin(timeCentroid, eoss);
		
	

	
	
		
		
			
		eoss.writeInt((int) floatData.size());
		eoss.writeInt((int) floatData.at(0).size());		
		eoss.writeInt((int) floatData.at(0).at(0).size());
		for (unsigned int i = 0; i < floatData.size(); i++) 
			for (unsigned int j = 0;  j < floatData.at(0).size(); j++)
				for (unsigned int k = 0; k <  floatData.at(0).at(0).size(); k++)	
							 
					eoss.writeFloat(floatData.at(i).at(j).at(k));
						
						
		
	

	
	
		
		
			
		eoss.writeInt((int) flagAnt.size());
		for (unsigned int i = 0; i < flagAnt.size(); i++)
				
			eoss.writeInt(flagAnt.at(i));
				
				
						
		
	

	
	
		
		
			
		eoss.writeInt((int) flagPol.size());
		eoss.writeInt((int) flagPol.at(0).size());
		for (unsigned int i = 0; i < flagPol.size(); i++) 
			for (unsigned int j = 0;  j < flagPol.at(0).size(); j++) 
							 
				eoss.writeInt(flagPol.at(i).at(j));
				
	
						
		
	

	
	
		
						
			eoss.writeBoolean(flagRow);
				
		
	

	
	
		
	interval.toBin(eoss);
		
	

	
	
		
	Tag::toBin(stateId, eoss);
		
	

	
	
		
	execBlockId.toBin(eoss);
		
	


	
	
	eoss.writeBoolean(subintegrationNumberExists);
	if (subintegrationNumberExists) {
	
	
	
		
						
			eoss.writeInt(subintegrationNumber);
				
		
	

	}

	}
	
void TotalPowerRow::timeFromBin(EndianISStream& eiss) {
		
	
		
		
		time =  ArrayTime::fromBin(eiss);
		
	
	
}
void TotalPowerRow::configDescriptionIdFromBin(EndianISStream& eiss) {
		
	
		
		
		configDescriptionId =  Tag::fromBin(eiss);
		
	
	
}
void TotalPowerRow::fieldIdFromBin(EndianISStream& eiss) {
		
	
		
		
		fieldId =  Tag::fromBin(eiss);
		
	
	
}
void TotalPowerRow::scanNumberFromBin(EndianISStream& eiss) {
		
	
	
		
			
		scanNumber =  eiss.readInt();
			
		
	
	
}
void TotalPowerRow::subscanNumberFromBin(EndianISStream& eiss) {
		
	
	
		
			
		subscanNumber =  eiss.readInt();
			
		
	
	
}
void TotalPowerRow::integrationNumberFromBin(EndianISStream& eiss) {
		
	
	
		
			
		integrationNumber =  eiss.readInt();
			
		
	
	
}
void TotalPowerRow::uvwFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	uvw = Length::from2DBin(eiss);		
	

		
	
	
}
void TotalPowerRow::exposureFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	exposure = Interval::from2DBin(eiss);		
	

		
	
	
}
void TotalPowerRow::timeCentroidFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	timeCentroid = ArrayTime::from2DBin(eiss);		
	

		
	
	
}
void TotalPowerRow::floatDataFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		floatData.clear();
			
		unsigned int floatDataDim1 = eiss.readInt();
		unsigned int floatDataDim2 = eiss.readInt();
		unsigned int floatDataDim3 = eiss.readInt();
		vector <vector<float> > floatDataAux2;
		vector <float> floatDataAux1;
		for (unsigned int i = 0; i < floatDataDim1; i++) {
			floatDataAux2.clear();
			for (unsigned int j = 0; j < floatDataDim2 ; j++) {
				floatDataAux1.clear();
				for (unsigned int k = 0; k < floatDataDim3; k++) {
			
					floatDataAux1.push_back(eiss.readFloat());
			
				}
				floatDataAux2.push_back(floatDataAux1);
			}
			floatData.push_back(floatDataAux2);
		}	
	

		
	
	
}
void TotalPowerRow::flagAntFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		flagAnt.clear();
		
		unsigned int flagAntDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < flagAntDim1; i++)
			
			flagAnt.push_back(eiss.readInt());
			
	

		
	
	
}
void TotalPowerRow::flagPolFromBin(EndianISStream& eiss) {
		
	
	
		
			
	
		flagPol.clear();
		
		unsigned int flagPolDim1 = eiss.readInt();
		unsigned int flagPolDim2 = eiss.readInt();
		vector <int> flagPolAux1;
		for (unsigned int i = 0; i < flagPolDim1; i++) {
			flagPolAux1.clear();
			for (unsigned int j = 0; j < flagPolDim2 ; j++)			
			
			flagPolAux1.push_back(eiss.readInt());
			
			flagPol.push_back(flagPolAux1);
		}
	
	

		
	
	
}
void TotalPowerRow::flagRowFromBin(EndianISStream& eiss) {
		
	
	
		
			
		flagRow =  eiss.readBoolean();
			
		
	
	
}
void TotalPowerRow::intervalFromBin(EndianISStream& eiss) {
		
	
		
		
		interval =  Interval::fromBin(eiss);
		
	
	
}
void TotalPowerRow::stateIdFromBin(EndianISStream& eiss) {
		
	
		
		
			
	
	stateId = Tag::from1DBin(eiss);	
	

		
	
	
}
void TotalPowerRow::execBlockIdFromBin(EndianISStream& eiss) {
		
	
		
		
		execBlockId =  Tag::fromBin(eiss);
		
	
	
}

void TotalPowerRow::subintegrationNumberFromBin(EndianISStream& eiss) {
		
	subintegrationNumberExists = eiss.readBoolean();
	if (subintegrationNumberExists) {
		
	
	
		
			
		subintegrationNumber =  eiss.readInt();
			
		
	

	}
	
}
	
	
	TotalPowerRow* TotalPowerRow::fromBin(EndianISStream& eiss, TotalPowerTable& table, const vector<string>& attributesSeq) {
		TotalPowerRow* row = new  TotalPowerRow(table);
		
		map<string, TotalPowerAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "TotalPowerTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime TotalPowerRow::getTime() const {
	
  		return time;
 	}

 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void TotalPowerRow::setTime (ArrayTime time)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("time", "TotalPower");
		
  		}
  	
 		this->time = time;
	
 	}
	
	

	

	
 	/**
 	 * Get scanNumber.
 	 * @return scanNumber as int
 	 */
 	int TotalPowerRow::getScanNumber() const {
	
  		return scanNumber;
 	}

 	/**
 	 * Set scanNumber with the specified int.
 	 * @param scanNumber The int value to which scanNumber is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setScanNumber (int scanNumber)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->scanNumber = scanNumber;
	
 	}
	
	

	

	
 	/**
 	 * Get subscanNumber.
 	 * @return subscanNumber as int
 	 */
 	int TotalPowerRow::getSubscanNumber() const {
	
  		return subscanNumber;
 	}

 	/**
 	 * Set subscanNumber with the specified int.
 	 * @param subscanNumber The int value to which subscanNumber is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setSubscanNumber (int subscanNumber)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->subscanNumber = subscanNumber;
	
 	}
	
	

	

	
 	/**
 	 * Get integrationNumber.
 	 * @return integrationNumber as int
 	 */
 	int TotalPowerRow::getIntegrationNumber() const {
	
  		return integrationNumber;
 	}

 	/**
 	 * Set integrationNumber with the specified int.
 	 * @param integrationNumber The int value to which integrationNumber is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setIntegrationNumber (int integrationNumber)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->integrationNumber = integrationNumber;
	
 	}
	
	

	

	
 	/**
 	 * Get uvw.
 	 * @return uvw as vector<vector<Length > >
 	 */
 	vector<vector<Length > > TotalPowerRow::getUvw() const {
	
  		return uvw;
 	}

 	/**
 	 * Set uvw with the specified vector<vector<Length > >.
 	 * @param uvw The vector<vector<Length > > value to which uvw is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setUvw (vector<vector<Length > > uvw)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->uvw = uvw;
	
 	}
	
	

	

	
 	/**
 	 * Get exposure.
 	 * @return exposure as vector<vector<Interval > >
 	 */
 	vector<vector<Interval > > TotalPowerRow::getExposure() const {
	
  		return exposure;
 	}

 	/**
 	 * Set exposure with the specified vector<vector<Interval > >.
 	 * @param exposure The vector<vector<Interval > > value to which exposure is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setExposure (vector<vector<Interval > > exposure)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->exposure = exposure;
	
 	}
	
	

	

	
 	/**
 	 * Get timeCentroid.
 	 * @return timeCentroid as vector<vector<ArrayTime > >
 	 */
 	vector<vector<ArrayTime > > TotalPowerRow::getTimeCentroid() const {
	
  		return timeCentroid;
 	}

 	/**
 	 * Set timeCentroid with the specified vector<vector<ArrayTime > >.
 	 * @param timeCentroid The vector<vector<ArrayTime > > value to which timeCentroid is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setTimeCentroid (vector<vector<ArrayTime > > timeCentroid)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->timeCentroid = timeCentroid;
	
 	}
	
	

	

	
 	/**
 	 * Get floatData.
 	 * @return floatData as vector<vector<vector<float > > >
 	 */
 	vector<vector<vector<float > > > TotalPowerRow::getFloatData() const {
	
  		return floatData;
 	}

 	/**
 	 * Set floatData with the specified vector<vector<vector<float > > >.
 	 * @param floatData The vector<vector<vector<float > > > value to which floatData is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setFloatData (vector<vector<vector<float > > > floatData)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->floatData = floatData;
	
 	}
	
	

	

	
 	/**
 	 * Get flagAnt.
 	 * @return flagAnt as vector<int >
 	 */
 	vector<int > TotalPowerRow::getFlagAnt() const {
	
  		return flagAnt;
 	}

 	/**
 	 * Set flagAnt with the specified vector<int >.
 	 * @param flagAnt The vector<int > value to which flagAnt is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setFlagAnt (vector<int > flagAnt)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagAnt = flagAnt;
	
 	}
	
	

	

	
 	/**
 	 * Get flagPol.
 	 * @return flagPol as vector<vector<int > >
 	 */
 	vector<vector<int > > TotalPowerRow::getFlagPol() const {
	
  		return flagPol;
 	}

 	/**
 	 * Set flagPol with the specified vector<vector<int > >.
 	 * @param flagPol The vector<vector<int > > value to which flagPol is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setFlagPol (vector<vector<int > > flagPol)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagPol = flagPol;
	
 	}
	
	

	

	
 	/**
 	 * Get flagRow.
 	 * @return flagRow as bool
 	 */
 	bool TotalPowerRow::getFlagRow() const {
	
  		return flagRow;
 	}

 	/**
 	 * Set flagRow with the specified bool.
 	 * @param flagRow The bool value to which flagRow is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setFlagRow (bool flagRow)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->flagRow = flagRow;
	
 	}
	
	

	

	
 	/**
 	 * Get interval.
 	 * @return interval as Interval
 	 */
 	Interval TotalPowerRow::getInterval() const {
	
  		return interval;
 	}

 	/**
 	 * Set interval with the specified Interval.
 	 * @param interval The Interval value to which interval is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setInterval (Interval interval)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->interval = interval;
	
 	}
	
	

	
	/**
	 * The attribute subintegrationNumber is optional. Return true if this attribute exists.
	 * @return true if and only if the subintegrationNumber attribute exists. 
	 */
	bool TotalPowerRow::isSubintegrationNumberExists() const {
		return subintegrationNumberExists;
	}
	

	
 	/**
 	 * Get subintegrationNumber, which is optional.
 	 * @return subintegrationNumber as int
 	 * @throw IllegalAccessException If subintegrationNumber does not exist.
 	 */
 	int TotalPowerRow::getSubintegrationNumber() const  {
		if (!subintegrationNumberExists) {
			throw IllegalAccessException("subintegrationNumber", "TotalPower");
		}
	
  		return subintegrationNumber;
 	}

 	/**
 	 * Set subintegrationNumber with the specified int.
 	 * @param subintegrationNumber The int value to which subintegrationNumber is to be set.
 	 
 	
 	 */
 	void TotalPowerRow::setSubintegrationNumber (int subintegrationNumber) {
	
 		this->subintegrationNumber = subintegrationNumber;
	
		subintegrationNumberExists = true;
	
 	}
	
	
	/**
	 * Mark subintegrationNumber, which is an optional field, as non-existent.
	 */
	void TotalPowerRow::clearSubintegrationNumber () {
		subintegrationNumberExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get configDescriptionId.
 	 * @return configDescriptionId as Tag
 	 */
 	Tag TotalPowerRow::getConfigDescriptionId() const {
	
  		return configDescriptionId;
 	}

 	/**
 	 * Set configDescriptionId with the specified Tag.
 	 * @param configDescriptionId The Tag value to which configDescriptionId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void TotalPowerRow::setConfigDescriptionId (Tag configDescriptionId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("configDescriptionId", "TotalPower");
		
  		}
  	
 		this->configDescriptionId = configDescriptionId;
	
 	}
	
	

	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag TotalPowerRow::getExecBlockId() const {
	
  		return execBlockId;
 	}

 	/**
 	 * Set execBlockId with the specified Tag.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setExecBlockId (Tag execBlockId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->execBlockId = execBlockId;
	
 	}
	
	

	

	
 	/**
 	 * Get fieldId.
 	 * @return fieldId as Tag
 	 */
 	Tag TotalPowerRow::getFieldId() const {
	
  		return fieldId;
 	}

 	/**
 	 * Set fieldId with the specified Tag.
 	 * @param fieldId The Tag value to which fieldId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void TotalPowerRow::setFieldId (Tag fieldId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("fieldId", "TotalPower");
		
  		}
  	
 		this->fieldId = fieldId;
	
 	}
	
	

	

	
 	/**
 	 * Get stateId.
 	 * @return stateId as vector<Tag> 
 	 */
 	vector<Tag>  TotalPowerRow::getStateId() const {
	
  		return stateId;
 	}

 	/**
 	 * Set stateId with the specified vector<Tag> .
 	 * @param stateId The vector<Tag>  value to which stateId is to be set.
 	 
 	
 		
 	 */
 	void TotalPowerRow::setStateId (vector<Tag>  stateId)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->stateId = stateId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set stateId[i] with the specified Tag.
 	 * @param i The index in stateId where to set the Tag value.
 	 * @param stateId The Tag value to which stateId[i] is to be set. 
	 		
 	 * @throws IndexOutOfBoundsException
  	 */
  	void TotalPowerRow::setStateId (int i, Tag stateId)  {
  	  	if (hasBeenAdded) {
  	  		
  		}
  		if ((i < 0) || (i > ((int) this->stateId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute stateId in table TotalPowerTable");
  		vector<Tag> ::iterator iter = this->stateId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->stateId.insert(this->stateId.erase(iter), stateId); 
  	}	
 			
	
	
	
		
/**
 * Append a Tag to stateId.
 * @param id the Tag to be appended to stateId
 */
 void TotalPowerRow::addStateId(Tag id){
 	stateId.push_back(id);
}

/**
 * Append an array of Tag to stateId.
 * @param id an array of Tag to be appended to stateId
 */
 void TotalPowerRow::addStateId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		stateId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in stateId at position i.
  *
  */
 const Tag TotalPowerRow::getStateId(int i) {
 	return stateId.at(i);
 }
 
 /**
  * Returns the StateRow linked to this row via the Tag stored in stateId
  * at position i.
  */
 StateRow* TotalPowerRow::getState(int i) {
 	return table.getContainer().getState().getRowByKey(stateId.at(i));
 } 
 
 /**
  * Returns the vector of StateRow* linked to this row via the Tags stored in stateId
  *
  */
 vector<StateRow *> TotalPowerRow::getStates() {
 	vector<StateRow *> result;
 	for (unsigned int i = 0; i < stateId.size(); i++)
 		result.push_back(table.getContainer().getState().getRowByKey(stateId.at(i)));
 		
 	return result;
 }
  

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Field table having Field.fieldId == fieldId
	 * @return a FieldRow*
	 * 
	 
	 */
	 FieldRow* TotalPowerRow::getFieldUsingFieldId() {
	 
	 	return table.getContainer().getField().getRowByKey(fieldId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the ConfigDescription table having ConfigDescription.configDescriptionId == configDescriptionId
	 * @return a ConfigDescriptionRow*
	 * 
	 
	 */
	 ConfigDescriptionRow* TotalPowerRow::getConfigDescriptionUsingConfigDescriptionId() {
	 
	 	return table.getContainer().getConfigDescription().getRowByKey(configDescriptionId);
	 }
	 

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the ExecBlock table having ExecBlock.execBlockId == execBlockId
	 * @return a ExecBlockRow*
	 * 
	 
	 */
	 ExecBlockRow* TotalPowerRow::getExecBlockUsingExecBlockId() {
	 
	 	return table.getContainer().getExecBlock().getRowByKey(execBlockId);
	 }
	 

	

	
	/**
	 * Create a TotalPowerRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	TotalPowerRow::TotalPowerRow (TotalPowerTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	

	

	

	

	

	
		subintegrationNumberExists = false;
	

	
	

	

	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["time"] = &TotalPowerRow::timeFromBin; 
	 fromBinMethods["configDescriptionId"] = &TotalPowerRow::configDescriptionIdFromBin; 
	 fromBinMethods["fieldId"] = &TotalPowerRow::fieldIdFromBin; 
	 fromBinMethods["scanNumber"] = &TotalPowerRow::scanNumberFromBin; 
	 fromBinMethods["subscanNumber"] = &TotalPowerRow::subscanNumberFromBin; 
	 fromBinMethods["integrationNumber"] = &TotalPowerRow::integrationNumberFromBin; 
	 fromBinMethods["uvw"] = &TotalPowerRow::uvwFromBin; 
	 fromBinMethods["exposure"] = &TotalPowerRow::exposureFromBin; 
	 fromBinMethods["timeCentroid"] = &TotalPowerRow::timeCentroidFromBin; 
	 fromBinMethods["floatData"] = &TotalPowerRow::floatDataFromBin; 
	 fromBinMethods["flagAnt"] = &TotalPowerRow::flagAntFromBin; 
	 fromBinMethods["flagPol"] = &TotalPowerRow::flagPolFromBin; 
	 fromBinMethods["flagRow"] = &TotalPowerRow::flagRowFromBin; 
	 fromBinMethods["interval"] = &TotalPowerRow::intervalFromBin; 
	 fromBinMethods["stateId"] = &TotalPowerRow::stateIdFromBin; 
	 fromBinMethods["execBlockId"] = &TotalPowerRow::execBlockIdFromBin; 
		
	
	 fromBinMethods["subintegrationNumber"] = &TotalPowerRow::subintegrationNumberFromBin; 
	
	}
	
	TotalPowerRow::TotalPowerRow (TotalPowerTable &t, TotalPowerRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	
		subintegrationNumberExists = false;
	

	
	

	

	

	
		
		}
		else {
	
		
			time = row.time;
		
			configDescriptionId = row.configDescriptionId;
		
			fieldId = row.fieldId;
		
		
		
		
			scanNumber = row.scanNumber;
		
			subscanNumber = row.subscanNumber;
		
			integrationNumber = row.integrationNumber;
		
			uvw = row.uvw;
		
			exposure = row.exposure;
		
			timeCentroid = row.timeCentroid;
		
			floatData = row.floatData;
		
			flagAnt = row.flagAnt;
		
			flagPol = row.flagPol;
		
			flagRow = row.flagRow;
		
			interval = row.interval;
		
			stateId = row.stateId;
		
			execBlockId = row.execBlockId;
		
		
		
		
		if (row.subintegrationNumberExists) {
			subintegrationNumber = row.subintegrationNumber;		
			subintegrationNumberExists = true;
		}
		else
			subintegrationNumberExists = false;
		
		}
		
		 fromBinMethods["time"] = &TotalPowerRow::timeFromBin; 
		 fromBinMethods["configDescriptionId"] = &TotalPowerRow::configDescriptionIdFromBin; 
		 fromBinMethods["fieldId"] = &TotalPowerRow::fieldIdFromBin; 
		 fromBinMethods["scanNumber"] = &TotalPowerRow::scanNumberFromBin; 
		 fromBinMethods["subscanNumber"] = &TotalPowerRow::subscanNumberFromBin; 
		 fromBinMethods["integrationNumber"] = &TotalPowerRow::integrationNumberFromBin; 
		 fromBinMethods["uvw"] = &TotalPowerRow::uvwFromBin; 
		 fromBinMethods["exposure"] = &TotalPowerRow::exposureFromBin; 
		 fromBinMethods["timeCentroid"] = &TotalPowerRow::timeCentroidFromBin; 
		 fromBinMethods["floatData"] = &TotalPowerRow::floatDataFromBin; 
		 fromBinMethods["flagAnt"] = &TotalPowerRow::flagAntFromBin; 
		 fromBinMethods["flagPol"] = &TotalPowerRow::flagPolFromBin; 
		 fromBinMethods["flagRow"] = &TotalPowerRow::flagRowFromBin; 
		 fromBinMethods["interval"] = &TotalPowerRow::intervalFromBin; 
		 fromBinMethods["stateId"] = &TotalPowerRow::stateIdFromBin; 
		 fromBinMethods["execBlockId"] = &TotalPowerRow::execBlockIdFromBin; 
			
	
		 fromBinMethods["subintegrationNumber"] = &TotalPowerRow::subintegrationNumberFromBin; 
			
	}

	
	bool TotalPowerRow::compareNoAutoInc(ArrayTime time, Tag configDescriptionId, Tag fieldId, int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval, vector<Tag>  stateId, Tag execBlockId) {
		bool result;
		result = true;
		
	
		
		result = result && (this->time == time);
		
		if (!result) return false;
	

	
		
		result = result && (this->configDescriptionId == configDescriptionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->fieldId == fieldId);
		
		if (!result) return false;
	

	
		
		result = result && (this->scanNumber == scanNumber);
		
		if (!result) return false;
	

	
		
		result = result && (this->subscanNumber == subscanNumber);
		
		if (!result) return false;
	

	
		
		result = result && (this->integrationNumber == integrationNumber);
		
		if (!result) return false;
	

	
		
		result = result && (this->uvw == uvw);
		
		if (!result) return false;
	

	
		
		result = result && (this->exposure == exposure);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeCentroid == timeCentroid);
		
		if (!result) return false;
	

	
		
		result = result && (this->floatData == floatData);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagAnt == flagAnt);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagPol == flagPol);
		
		if (!result) return false;
	

	
		
		result = result && (this->flagRow == flagRow);
		
		if (!result) return false;
	

	
		
		result = result && (this->interval == interval);
		
		if (!result) return false;
	

	
		
		result = result && (this->stateId == stateId);
		
		if (!result) return false;
	

	
		
		result = result && (this->execBlockId == execBlockId);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool TotalPowerRow::compareRequiredValue(int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval, vector<Tag>  stateId, Tag execBlockId) {
		bool result;
		result = true;
		
	
		if (!(this->scanNumber == scanNumber)) return false;
	

	
		if (!(this->subscanNumber == subscanNumber)) return false;
	

	
		if (!(this->integrationNumber == integrationNumber)) return false;
	

	
		if (!(this->uvw == uvw)) return false;
	

	
		if (!(this->exposure == exposure)) return false;
	

	
		if (!(this->timeCentroid == timeCentroid)) return false;
	

	
		if (!(this->floatData == floatData)) return false;
	

	
		if (!(this->flagAnt == flagAnt)) return false;
	

	
		if (!(this->flagPol == flagPol)) return false;
	

	
		if (!(this->flagRow == flagRow)) return false;
	

	
		if (!(this->interval == interval)) return false;
	

	
		if (!(this->stateId == stateId)) return false;
	

	
		if (!(this->execBlockId == execBlockId)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the TotalPowerRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool TotalPowerRow::equalByRequiredValue(TotalPowerRow* x) {
		
			
		if (this->scanNumber != x->scanNumber) return false;
			
		if (this->subscanNumber != x->subscanNumber) return false;
			
		if (this->integrationNumber != x->integrationNumber) return false;
			
		if (this->uvw != x->uvw) return false;
			
		if (this->exposure != x->exposure) return false;
			
		if (this->timeCentroid != x->timeCentroid) return false;
			
		if (this->floatData != x->floatData) return false;
			
		if (this->flagAnt != x->flagAnt) return false;
			
		if (this->flagPol != x->flagPol) return false;
			
		if (this->flagRow != x->flagRow) return false;
			
		if (this->interval != x->interval) return false;
			
		if (this->stateId != x->stateId) return false;
			
		if (this->execBlockId != x->execBlockId) return false;
			
		
		return true;
	}	
	
/*
	 map<string, TotalPowerAttributeFromBin> TotalPowerRow::initFromBinMethods() {
		map<string, TotalPowerAttributeFromBin> result;
		
		result["time"] = &TotalPowerRow::timeFromBin;
		result["configDescriptionId"] = &TotalPowerRow::configDescriptionIdFromBin;
		result["fieldId"] = &TotalPowerRow::fieldIdFromBin;
		result["scanNumber"] = &TotalPowerRow::scanNumberFromBin;
		result["subscanNumber"] = &TotalPowerRow::subscanNumberFromBin;
		result["integrationNumber"] = &TotalPowerRow::integrationNumberFromBin;
		result["uvw"] = &TotalPowerRow::uvwFromBin;
		result["exposure"] = &TotalPowerRow::exposureFromBin;
		result["timeCentroid"] = &TotalPowerRow::timeCentroidFromBin;
		result["floatData"] = &TotalPowerRow::floatDataFromBin;
		result["flagAnt"] = &TotalPowerRow::flagAntFromBin;
		result["flagPol"] = &TotalPowerRow::flagPolFromBin;
		result["flagRow"] = &TotalPowerRow::flagRowFromBin;
		result["interval"] = &TotalPowerRow::intervalFromBin;
		result["stateId"] = &TotalPowerRow::stateIdFromBin;
		result["execBlockId"] = &TotalPowerRow::execBlockIdFromBin;
		
		
		result["subintegrationNumber"] = &TotalPowerRow::subintegrationNumberFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
