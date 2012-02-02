
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
	
		
	
		
		
			
		x->time = getTime().toIDLArrayTime();
			
		
		
	

	
		
		
			
				
		x->scanNumber = getScanNumber();
 				
 			
		
		
	

	
		
		
			
				
		x->subscanNumber = getSubscanNumber();
 				
 			
		
		
	

	
		
		
			
				
		x->integrationNumber = getIntegrationNumber();
 				
 			
		
		
	

	
		
		
			
		vector< vector<Length> > tmpUvw = getUvw();
		x->uvw.length(tmpUvw.size());
		for (unsigned int i = 0; i < tmpUvw.size(); ++i)
			x->uvw[i].length(tmpUvw[0].size());
		for (unsigned int i = 0; i < tmpUvw.size(); ++i) {
			for (unsigned int j = 0; j < tmpUvw[0].size(); ++j) {
				
				x->uvw[i][j] = tmpUvw[i][j].toIDLLength();
				
		 	}
		 }
			
		
		
	

	
		
		
			
		vector< vector<Interval> > tmpExposure = getExposure();
		x->exposure.length(tmpExposure.size());
		for (unsigned int i = 0; i < tmpExposure.size(); ++i)
			x->exposure[i].length(tmpExposure[0].size());
		for (unsigned int i = 0; i < tmpExposure.size(); ++i) {
			for (unsigned int j = 0; j < tmpExposure[0].size(); ++j) {
				
				x->exposure[i][j] = tmpExposure[i][j].toIDLInterval();
				
		 	}
		 }
			
		
		
	

	
		
		
			
		vector< vector<ArrayTime> > tmpTimeCentroid = getTimeCentroid();
		x->timeCentroid.length(tmpTimeCentroid.size());
		for (unsigned int i = 0; i < tmpTimeCentroid.size(); ++i)
			x->timeCentroid[i].length(tmpTimeCentroid[0].size());
		for (unsigned int i = 0; i < tmpTimeCentroid.size(); ++i) {
			for (unsigned int j = 0; j < tmpTimeCentroid[0].size(); ++j) {
				
				x->timeCentroid[i][j] = tmpTimeCentroid[i][j].toIDLArrayTime();
				
		 	}
		 }
			
		
		
	

	
		
		
			
		vector< vector< vector<float> > > tmpFloatData = getFloatData();
		x->floatData.length(tmpFloatData.size());
		for (unsigned int i = 0; i < tmpFloatData.size(); ++i) {
			x->floatData[i].length(tmpFloatData[0].size());
			for (unsigned int j = 0; j < tmpFloatData[0][0].size(); ++j)
				x->floatData[i][j].length(tmpFloatData[0][0].size());
		}
		for (unsigned int i = 0; i < tmpFloatData.size(); ++i) {
			for (unsigned int j = 0; j < tmpFloatData[0].size(); ++j) {
				for (unsigned int k = 0; k < tmpFloatData[0][0].size(); ++k) {
					
						
					x->floatData[i][j][k] = tmpFloatData[i][j][k];
		 				
			 		
				}
			}
		}
			
		
		
	

	
		
		
			
		vector<int> tmpFlagAnt = getFlagAnt();
		x->flagAnt.length(tmpFlagAnt.size());
		for (unsigned int i = 0; i < tmpFlagAnt.size(); ++i) {
			
				
			x->flagAnt[i] = tmpFlagAnt[i];
	 			
	 		
	 	}
			
		
		
	

	
		
		
			
		vector< vector<int> > tmpFlagPol = getFlagPol();
		x->flagPol.length(tmpFlagPol.size());
		for (unsigned int i = 0; i < tmpFlagPol.size(); ++i)
			x->flagPol[i].length(tmpFlagPol[0].size());
		for (unsigned int i = 0; i < tmpFlagPol.size(); ++i) {
			for (unsigned int j = 0; j < tmpFlagPol[0].size(); ++j) {
				
					
				x->flagPol[i][j] = tmpFlagPol[i][j];
	 				
		 		
		 	}
		 }
			
		
		
	

	
		
		
			
				
		x->flagRow = getFlagRow();
 				
 			
		
		
	

	
		
		
			
		x->interval = getInterval().toIDLInterval();
			
		
		
	

	
		
		x->subintegrationNumberExists = subintegrationNumberExists;
  		if (subintegrationNumberExists) {
  			try {
		
		
			
				
		x->subintegrationNumber = getSubintegrationNumber();
 				
 			
		
		
			} catch (IllegalAccessException e) {
			}
		}
		
	

	
	
		
	
		
		
			
		x->configDescriptionId = getConfigDescriptionId().toIDLTag();
			
		
		
	

	
		
		
			
		x->execBlockId = getExecBlockId().toIDLTag();
			
		
		
	

	
		
		
			
		x->fieldId = getFieldId().toIDLTag();
			
		
		
	

	
  		
  	

	
		
	

	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct TotalPowerRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void TotalPowerRow::setFromIDL (TotalPowerRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTime(*(new ArrayTime (x.time)));
			
 		
		
	

	
		
		
			
		setScanNumber(x.scanNumber);
  			
 		
		
	

	
		
		
			
		setSubscanNumber(x.subscanNumber);
  			
 		
		
	

	
		
		
			
		setIntegrationNumber(x.integrationNumber);
  			
 		
		
	

	
		
		
			
		vector< vector<Length> > tmpUvw(x.uvw.length());
		for (unsigned int i = 0; i < tmpUvw.size(); ++i)
			tmpUvw[i] = *(new vector<Length>(x.uvw[0].length()));
		for (unsigned int i = 0; i < tmpUvw.size(); ++i) {
			for (unsigned int j = 0; j < tmpUvw[0].size(); ++j) {
				
				tmpUvw[i][j] = *(new Length (x.uvw[i][j]));
				
  			}
		}
		setUvw(tmpUvw);
			
  		
		
	

	
		
		
			
		vector< vector<Interval> > tmpExposure(x.exposure.length());
		for (unsigned int i = 0; i < tmpExposure.size(); ++i)
			tmpExposure[i] = *(new vector<Interval>(x.exposure[0].length()));
		for (unsigned int i = 0; i < tmpExposure.size(); ++i) {
			for (unsigned int j = 0; j < tmpExposure[0].size(); ++j) {
				
				tmpExposure[i][j] = *(new Interval (x.exposure[i][j]));
				
  			}
		}
		setExposure(tmpExposure);
			
  		
		
	

	
		
		
			
		vector< vector<ArrayTime> > tmpTimeCentroid(x.timeCentroid.length());
		for (unsigned int i = 0; i < tmpTimeCentroid.size(); ++i)
			tmpTimeCentroid[i] = *(new vector<ArrayTime>(x.timeCentroid[0].length()));
		for (unsigned int i = 0; i < tmpTimeCentroid.size(); ++i) {
			for (unsigned int j = 0; j < tmpTimeCentroid[0].size(); ++j) {
				
				tmpTimeCentroid[i][j] = *(new ArrayTime (x.timeCentroid[i][j]));
				
  			}
		}
		setTimeCentroid(tmpTimeCentroid);
			
  		
		
	

	
		
		
			
		vector< vector< vector<float> > > tmpFloatData(x.floatData.length());
		for (unsigned int i = 0; i < tmpFloatData.size(); ++i) {
			tmpFloatData[i] = *(new vector< vector<float> > (x.floatData[0].length()));
			for (unsigned int j = 0; j < tmpFloatData[0].size(); ++j)
				tmpFloatData[i][j] = *(new vector<float>(x.floatData[0][0].length()));
		}
		for (unsigned int i = 0; i < tmpFloatData.size(); ++i) {
			for (unsigned int j = 0; j < tmpFloatData[0].size(); ++j) {
				for (unsigned int k = 0; k < tmpFloatData[0][0].size(); ++k) {
					
					tmpFloatData[i][j][k] = x.floatData[i][j][k];
		  			
		  		}
  			}
		}
		setFloatData(tmpFloatData);
			
  		
		
	

	
		
		
			
		vector<int> tmpFlagAnt(x.flagAnt.length());
		for (unsigned int i = 0; i < tmpFlagAnt.size(); ++i) {
			
			tmpFlagAnt[i] = x.flagAnt[i];
  			
		}
		setFlagAnt(tmpFlagAnt);
			
  		
		
	

	
		
		
			
		vector< vector<int> > tmpFlagPol(x.flagPol.length());
		for (unsigned int i = 0; i < tmpFlagPol.size(); ++i)
			tmpFlagPol[i] = *(new vector<int>(x.flagPol[0].length()));
		for (unsigned int i = 0; i < tmpFlagPol.size(); ++i) {
			for (unsigned int j = 0; j < tmpFlagPol[0].size(); ++j) {
				
				tmpFlagPol[i][j] = x.flagPol[i][j];
	  			
  			}
		}
		setFlagPol(tmpFlagPol);
			
  		
		
	

	
		
		
			
		setFlagRow(x.flagRow);
  			
 		
		
	

	
		
		
			
		setInterval(*(new Interval (x.interval)));
			
 		
		
	

	
		
		if (x.subintegrationNumberExists) {
		
		
			
		setSubintegrationNumber(x.subintegrationNumber);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setConfigDescriptionId(*(new Tag (x.configDescriptionId)));
			
 		
		
	

	
		
		
			
		setExecBlockId(*(new Tag (x.execBlockId)));
			
 		
		
	

	
		
		
			
		setFieldId(*(new Tag (x.fieldId)));
			
 		
		
	

	
		
		vector<Tag> tmpStateId(x.stateId.length());
		for (unsigned int i = 0; i < tmpStateId.size(); ++i) {
			
			tmpStateId[i] = *(new Tag (x.stateId[i]));
			
		}
		setStateId(tmpStateId);		
		
  	

	
		
	

	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"TotalPower");
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
	void TotalPowerRow::setFromXML (string rowDoc) throw(ConversionException) {
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
		eoss.writeInt(scanNumber);
		eoss.writeInt(subscanNumber);
		eoss.writeInt(integrationNumber);
		Length::toBin(uvw, eoss);
		Interval::toBin(exposure, eoss);
		ArrayTime::toBin(timeCentroid, eoss);
		
		eoss.writeInt(floatData.size());
		eoss.writeInt(floatData[0].size());
		eoss.writeInt(floatData[0][0].size());
		for (unsigned int i = 0; i < floatData.size(); i ++)
			for (unsigned int j = 0; j < floatData[0].size(); j++)
				for (unsigned int k = 0; k < floatData[0][0].size(); k++)
					eoss.writeFloat(floatData[i][j][k]);
		
		eoss.writeInt(flagAnt.size());
		for (unsigned int i = 0; i < flagAnt.size(); i++)
			eoss.writeInt(flagAnt[i]);
			
		eoss.writeInt(flagPol.size());
		eoss.writeInt(flagPol[0].size());		
		for (unsigned int i = 0; i < flagPol.size(); i++)
			for (unsigned int j = 0; j < flagPol[0].size(); j++)
				eoss.writeInt(flagPol[i][j]);
			
		eoss.writeBoolean(flagRow);
		interval.toBin(eoss);
		
		eoss.writeBoolean(subintegrationNumberExists);
		if (subintegrationNumberExists)
			eoss.writeInt(subintegrationNumber);
		
		configDescriptionId.toBin(eoss);
		execBlockId.toBin(eoss);
		fieldId.toBin(eoss);
		Tag::toBin(stateId, eoss);
	}
	
	TotalPowerRow* TotalPowerRow::fromBin(EndianISStream& eiss, TotalPowerTable& table) {
		TotalPowerRow* row = new TotalPowerRow(table);
		
		row->time = ArrayTime::fromBin(eiss); 
		row->scanNumber = eiss.readInt(); 
		row->subscanNumber = eiss.readInt(); 
		row->integrationNumber = eiss.readInt();  
		row->uvw = Length::from2DBin(eiss); 
		row->exposure = Interval::from2DBin(eiss); 
		row->timeCentroid = ArrayTime::from2DBin(eiss); 
		
		int dim1 = 0;
		int dim2 = 0;
		int dim3 = 0;
		
		dim1 = eiss.readInt();
		dim2 = eiss.readInt(); 
		dim3 = eiss.readInt(); 
		
		vector<vector<float> > faux1;
		vector<float> faux2;
		for (int i = 0; i < dim1; i ++) {
			faux1.clear();
			for (int j = 0; j < dim2; j++) {
				faux2.clear();
				for (int k = 0; k < dim3; k++)
					faux2.push_back(eiss.readFloat());
				faux1.push_back(faux2);
			}
			row->floatData.push_back(faux1);
		}
		
		dim1 = eiss.readInt();
		for (int i = 0; i < dim1; i++)
			row->flagAnt.push_back(eiss.readInt());
		
		dim1 = eiss.readInt();
		dim2 = eiss.readInt();
		vector<int> iaux;
		for (int i = 0; i < dim1; i++) {
			iaux.clear();
			for (int j = 0;  j < dim2; j++)
				iaux.push_back(eiss.readInt());
			row->flagPol.push_back(iaux);
		}
						
		row->flagRow = eiss.readBoolean(); 
		row->interval = Interval::fromBin(eiss); 
		
		row->subintegrationNumberExists = eiss.readBoolean();
		if (row->subintegrationNumberExists) row->subintegrationNumber = eiss.readInt();
		
		row->configDescriptionId = Tag::fromBin(eiss);
		row->execBlockId = Tag::fromBin(eiss);
		row->fieldId = Tag::fromBin(eiss);
		row->stateId = Tag::from1DBin(eiss);
		
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
 	void TotalPowerRow::setTime (ArrayTime time) throw(IllegalAccessException) {
  		if (hasBeenAdded) {
			throw IllegalAccessException();
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
 	void TotalPowerRow::setScanNumber (int scanNumber) {
	
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
 	void TotalPowerRow::setSubscanNumber (int subscanNumber) {
	
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
 	void TotalPowerRow::setIntegrationNumber (int integrationNumber) {
	
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
 	void TotalPowerRow::setUvw (vector<vector<Length > > uvw) {
	
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
 	void TotalPowerRow::setExposure (vector<vector<Interval > > exposure) {
	
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
 	void TotalPowerRow::setTimeCentroid (vector<vector<ArrayTime > > timeCentroid) {
	
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
 	void TotalPowerRow::setFloatData (vector<vector<vector<float > > > floatData) {
	
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
 	void TotalPowerRow::setFlagAnt (vector<int > flagAnt) {
	
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
 	void TotalPowerRow::setFlagPol (vector<vector<int > > flagPol) {
	
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
 	void TotalPowerRow::setFlagRow (bool flagRow) {
	
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
 	void TotalPowerRow::setInterval (Interval interval) {
	
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
 	int TotalPowerRow::getSubintegrationNumber() const throw(IllegalAccessException) {
		if (!subintegrationNumberExists) {
			throw IllegalAccessException();
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
 	void TotalPowerRow::setConfigDescriptionId (Tag configDescriptionId) throw(IllegalAccessException) {
  		if (hasBeenAdded) {
			throw IllegalAccessException();
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
 	void TotalPowerRow::setExecBlockId (Tag execBlockId) {
	
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
 	void TotalPowerRow::setFieldId (Tag fieldId) throw(IllegalAccessException) {
  		if (hasBeenAdded) {
			throw IllegalAccessException();
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
 	void TotalPowerRow::setStateId (vector<Tag>  stateId) {
	
 		this->stateId = stateId;
	
 	}
	
	

	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set stateId[i] with the specified Tag.
 	 * @param i The index in stateId where to set the Tag value.
 	 * @param stateId The Tag value to which stateId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void TotalPowerRow::setStateId (int i, Tag stateId) throw(OutOfBoundsException) {
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
	 

	

	
	
		
		
			
	// ===> One to one link from a row of TotalPower table to a row of ExecBlock table.
	
	/**
	 * Get the row in table ExecBlock by traversing the defined link to that table.
	 * @return A row in ExecBlock table.
	 * @throws NoSuchRow if there is no such row in table ExecBlock.
	 */
	ExecBlockRow *TotalPowerRow::getExecBlock() const throw(NoSuchRow) {
		return table.getContainer().getExecBlock().getRowByKey( execBlockId );
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
	

	
	

	

	

	

	}
	
	TotalPowerRow::TotalPowerRow (TotalPowerTable &t, TotalPowerRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	
		subintegrationNumberExists = false;
	

	
	

	

	

	
		
		}
		else {
	
		
			configDescriptionId = row.configDescriptionId;
		
			fieldId = row.fieldId;
		
			time = row.time;
		
		
		
		
			execBlockId = row.execBlockId;
		
			stateId = row.stateId;
		
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
		
		
		
		
		if (row.subintegrationNumberExists) {
			subintegrationNumber = row.subintegrationNumber;		
			subintegrationNumberExists = true;
		}
		else
			subintegrationNumberExists = false;
		
		}	
	}

	
	bool TotalPowerRow::compareNoAutoInc(Tag configDescriptionId, Tag fieldId, ArrayTime time, Tag execBlockId, vector<Tag>  stateId, int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval) {
		bool result;
		result = true;
		
	
		
		result = result && (this->configDescriptionId == configDescriptionId);
		
		if (!result) return false;
	

	
		
		result = result && (this->fieldId == fieldId);
		
		if (!result) return false;
	

	
		
		result = result && (this->time == time);
		
		if (!result) return false;
	

	
		
		result = result && (this->execBlockId == execBlockId);
		
		if (!result) return false;
	

	
		
		result = result && (this->stateId == stateId);
		
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
	

		return result;
	}	
	
	
	
	bool TotalPowerRow::compareRequiredValue(Tag execBlockId, vector<Tag>  stateId, int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval) {
		bool result;
		result = true;
		
	
		if (!(this->execBlockId == execBlockId)) return false;
	

	
		if (!(this->stateId == stateId)) return false;
	

	
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
		
			
		if (this->execBlockId != x->execBlockId) return false;
			
		if (this->stateId != x->stateId) return false;
			
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
			
		
		return true;
	}	
	

} // End namespace asdm
 
