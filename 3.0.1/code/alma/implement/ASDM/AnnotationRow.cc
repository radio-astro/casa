
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
 * File AnnotationRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <AnnotationRow.h>
#include <AnnotationTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::AnnotationRow;
using asdm::AnnotationTable;

using asdm::AntennaTable;
using asdm::AntennaRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	AnnotationRow::~AnnotationRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	AnnotationTable &AnnotationRow::getTable() const {
		return table;
	}
	
	void AnnotationRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a AnnotationRowIDL struct.
	 */
	AnnotationRowIDL *AnnotationRow::toIDL() const {
		AnnotationRowIDL *x = new AnnotationRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->annotationId = annotationId.toIDLTag();
			
		
	

	
  		
		
		
			
		x->time = time.toIDLArrayTime();
			
		
	

	
  		
		
		
			
				
		x->issue = CORBA::string_dup(issue.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->details = CORBA::string_dup(details.c_str());
				
 			
		
	

	
  		
		
		x->numAntennaExists = numAntennaExists;
		
		
			
				
		x->numAntenna = numAntenna;
 				
 			
		
	

	
  		
		
		x->basebandNameExists = basebandNameExists;
		
		
			
		x->basebandName.length(basebandName.size());
		for (unsigned int i = 0; i < basebandName.size(); ++i) {
			
				
			x->basebandName[i] = basebandName.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->numBasebandExists = numBasebandExists;
		
		
			
				
		x->numBaseband = numBaseband;
 				
 			
		
	

	
  		
		
		x->intervalExists = intervalExists;
		
		
			
		x->interval = interval.toIDLInterval();
			
		
	

	
  		
		
		x->dValueExists = dValueExists;
		
		
			
				
		x->dValue = dValue;
 				
 			
		
	

	
  		
		
		x->vdValueExists = vdValueExists;
		
		
			
		x->vdValue.length(vdValue.size());
		for (unsigned int i = 0; i < vdValue.size(); ++i) {
			
				
			x->vdValue[i] = vdValue.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->vvdValuesExists = vvdValuesExists;
		
		
			
		x->vvdValues.length(vvdValues.size());
		for (unsigned int i = 0; i < vvdValues.size(); i++) {
			x->vvdValues[i].length(vvdValues.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < vvdValues.size() ; i++)
			for (unsigned int j = 0; j < vvdValues.at(i).size(); j++)
					
						
				x->vvdValues[i][j] = vvdValues.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->llValueExists = llValueExists;
		
		
			
				
		x->llValue = llValue;
 				
 			
		
	

	
  		
		
		x->vllValueExists = vllValueExists;
		
		
			
		x->vllValue.length(vllValue.size());
		for (unsigned int i = 0; i < vllValue.size(); ++i) {
			
				
			x->vllValue[i] = vllValue.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->vvllValueExists = vvllValueExists;
		
		
			
		x->vvllValue.length(vvllValue.size());
		for (unsigned int i = 0; i < vvllValue.size(); i++) {
			x->vvllValue[i].length(vvllValue.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < vvllValue.size() ; i++)
			for (unsigned int j = 0; j < vvllValue.at(i).size(); j++)
					
						
				x->vvllValue[i][j] = vvllValue.at(i).at(j);
		 				
			 						
		
			
		
	

	
	
		
	
  	
 		
 		
		x->antennaIdExists = antennaIdExists;
		
		
		
		x->antennaId.length(antennaId.size());
		for (unsigned int i = 0; i < antennaId.size(); ++i) {
			
			x->antennaId[i] = antennaId.at(i).toIDLTag();
			
	 	}
	 	 		
  	

	
		
	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct AnnotationRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void AnnotationRow::setFromIDL (AnnotationRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setAnnotationId(Tag (x.annotationId));
			
 		
		
	

	
		
		
			
		setTime(ArrayTime (x.time));
			
 		
		
	

	
		
		
			
		setIssue(string (x.issue));
			
 		
		
	

	
		
		
			
		setDetails(string (x.details));
			
 		
		
	

	
		
		numAntennaExists = x.numAntennaExists;
		if (x.numAntennaExists) {
		
		
			
		setNumAntenna(x.numAntenna);
  			
 		
		
		}
		
	

	
		
		basebandNameExists = x.basebandNameExists;
		if (x.basebandNameExists) {
		
		
			
		basebandName .clear();
		for (unsigned int i = 0; i <x.basebandName.length(); ++i) {
			
			basebandName.push_back(x.basebandName[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		numBasebandExists = x.numBasebandExists;
		if (x.numBasebandExists) {
		
		
			
		setNumBaseband(x.numBaseband);
  			
 		
		
		}
		
	

	
		
		intervalExists = x.intervalExists;
		if (x.intervalExists) {
		
		
			
		setInterval(Interval (x.interval));
			
 		
		
		}
		
	

	
		
		dValueExists = x.dValueExists;
		if (x.dValueExists) {
		
		
			
		setDValue(x.dValue);
  			
 		
		
		}
		
	

	
		
		vdValueExists = x.vdValueExists;
		if (x.vdValueExists) {
		
		
			
		vdValue .clear();
		for (unsigned int i = 0; i <x.vdValue.length(); ++i) {
			
			vdValue.push_back(x.vdValue[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		vvdValuesExists = x.vvdValuesExists;
		if (x.vvdValuesExists) {
		
		
			
		vvdValues .clear();
		vector<double> v_aux_vvdValues;
		for (unsigned int i = 0; i < x.vvdValues.length(); ++i) {
			v_aux_vvdValues.clear();
			for (unsigned int j = 0; j < x.vvdValues[0].length(); ++j) {
				
				v_aux_vvdValues.push_back(x.vvdValues[i][j]);
	  			
  			}
  			vvdValues.push_back(v_aux_vvdValues);			
		}
			
  		
		
		}
		
	

	
		
		llValueExists = x.llValueExists;
		if (x.llValueExists) {
		
		
			
		setLlValue(x.llValue);
  			
 		
		
		}
		
	

	
		
		vllValueExists = x.vllValueExists;
		if (x.vllValueExists) {
		
		
			
		vllValue .clear();
		for (unsigned int i = 0; i <x.vllValue.length(); ++i) {
			
			vllValue.push_back(x.vllValue[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		vvllValueExists = x.vvllValueExists;
		if (x.vvllValueExists) {
		
		
			
		vvllValue .clear();
		vector<long long> v_aux_vvllValue;
		for (unsigned int i = 0; i < x.vvllValue.length(); ++i) {
			v_aux_vvllValue.clear();
			for (unsigned int j = 0; j < x.vvllValue[0].length(); ++j) {
				
				v_aux_vvllValue.push_back(x.vvllValue[i][j]);
	  			
  			}
  			vvllValue.push_back(v_aux_vvllValue);			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		antennaIdExists = x.antennaIdExists;
		if (x.antennaIdExists) {
		
		antennaId .clear();
		for (unsigned int i = 0; i <x.antennaId.length(); ++i) {
			
			antennaId.push_back(Tag (x.antennaId[i]));
			
		}
		
		}
		
  	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Annotation");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string AnnotationRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(annotationId, "annotationId", buf);
		
		
	

  	
 		
		
		Parser::toXML(time, "time", buf);
		
		
	

  	
 		
		
		Parser::toXML(issue, "issue", buf);
		
		
	

  	
 		
		
		Parser::toXML(details, "details", buf);
		
		
	

  	
 		
		if (numAntennaExists) {
		
		
		Parser::toXML(numAntenna, "numAntenna", buf);
		
		
		}
		
	

  	
 		
		if (basebandNameExists) {
		
		
			buf.append(EnumerationParser::toXML("basebandName", basebandName));
		
		
		}
		
	

  	
 		
		if (numBasebandExists) {
		
		
		Parser::toXML(numBaseband, "numBaseband", buf);
		
		
		}
		
	

  	
 		
		if (intervalExists) {
		
		
		Parser::toXML(interval, "interval", buf);
		
		
		}
		
	

  	
 		
		if (dValueExists) {
		
		
		Parser::toXML(dValue, "dValue", buf);
		
		
		}
		
	

  	
 		
		if (vdValueExists) {
		
		
		Parser::toXML(vdValue, "vdValue", buf);
		
		
		}
		
	

  	
 		
		if (vvdValuesExists) {
		
		
		Parser::toXML(vvdValues, "vvdValues", buf);
		
		
		}
		
	

  	
 		
		if (llValueExists) {
		
		
		Parser::toXML(llValue, "llValue", buf);
		
		
		}
		
	

  	
 		
		if (vllValueExists) {
		
		
		Parser::toXML(vllValue, "vllValue", buf);
		
		
		}
		
	

  	
 		
		if (vvllValueExists) {
		
		
		Parser::toXML(vvllValue, "vvllValue", buf);
		
		
		}
		
	

	
	
		
  	
 		
		if (antennaIdExists) {
		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
		}
		
	

	
		
	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void AnnotationRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setAnnotationId(Parser::getTag("annotationId","Annotation",rowDoc));
			
		
	

	
  		
			
	  	setTime(Parser::getArrayTime("time","Annotation",rowDoc));
			
		
	

	
  		
			
	  	setIssue(Parser::getString("issue","Annotation",rowDoc));
			
		
	

	
  		
			
	  	setDetails(Parser::getString("details","Annotation",rowDoc));
			
		
	

	
  		
        if (row.isStr("<numAntenna>")) {
			
	  		setNumAntenna(Parser::getInteger("numAntenna","Annotation",rowDoc));
			
		}
 		
	

	
		
	if (row.isStr("<basebandName>")) {
		
		
		
		basebandName = EnumerationParser::getBasebandName1D("basebandName","Annotation",rowDoc);			
		
		
		
		basebandNameExists = true;
	}
		
	

	
  		
        if (row.isStr("<numBaseband>")) {
			
	  		setNumBaseband(Parser::getInteger("numBaseband","Annotation",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<interval>")) {
			
	  		setInterval(Parser::getInterval("interval","Annotation",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<dValue>")) {
			
	  		setDValue(Parser::getDouble("dValue","Annotation",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<vdValue>")) {
			
								
	  		setVdValue(Parser::get1DDouble("vdValue","Annotation",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<vvdValues>")) {
			
								
	  		setVvdValues(Parser::get2DDouble("vvdValues","Annotation",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<llValue>")) {
			
	  		setLlValue(Parser::getLong("llValue","Annotation",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<vllValue>")) {
			
								
	  		setVllValue(Parser::get1DLong("vllValue","Annotation",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<vvllValue>")) {
			
								
	  		setVvllValue(Parser::get2DLong("vvllValue","Annotation",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
  		if (row.isStr("<antennaId>")) {
  			setAntennaId(Parser::get1DTag("antennaId","Annotation",rowDoc));  		
  		}
  		
  	

	
		
	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"Annotation");
		}
	}
	
	void AnnotationRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	annotationId.toBin(eoss);
		
	

	
	
		
	time.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(issue);
				
		
	

	
	
		
						
			eoss.writeString(details);
				
		
	


	
	
	eoss.writeBoolean(numAntennaExists);
	if (numAntennaExists) {
	
	
	
		
						
			eoss.writeInt(numAntenna);
				
		
	

	}

	eoss.writeBoolean(basebandNameExists);
	if (basebandNameExists) {
	
	
	
		
		
			
		eoss.writeInt((int) basebandName.size());
		for (unsigned int i = 0; i < basebandName.size(); i++)
				
			eoss.writeInt(basebandName.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(numBasebandExists);
	if (numBasebandExists) {
	
	
	
		
						
			eoss.writeInt(numBaseband);
				
		
	

	}

	eoss.writeBoolean(intervalExists);
	if (intervalExists) {
	
	
	
		
	interval.toBin(eoss);
		
	

	}

	eoss.writeBoolean(dValueExists);
	if (dValueExists) {
	
	
	
		
						
			eoss.writeDouble(dValue);
				
		
	

	}

	eoss.writeBoolean(vdValueExists);
	if (vdValueExists) {
	
	
	
		
		
			
		eoss.writeInt((int) vdValue.size());
		for (unsigned int i = 0; i < vdValue.size(); i++)
				
			eoss.writeDouble(vdValue.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(vvdValuesExists);
	if (vvdValuesExists) {
	
	
	
		
		
			
		eoss.writeInt((int) vvdValues.size());
		eoss.writeInt((int) vvdValues.at(0).size());
		for (unsigned int i = 0; i < vvdValues.size(); i++) 
			for (unsigned int j = 0;  j < vvdValues.at(0).size(); j++) 
							 
				eoss.writeDouble(vvdValues.at(i).at(j));
				
	
						
		
	

	}

	eoss.writeBoolean(llValueExists);
	if (llValueExists) {
	
	
	
		
						
			eoss.writeLong(llValue);
				
		
	

	}

	eoss.writeBoolean(vllValueExists);
	if (vllValueExists) {
	
	
	
		
		
			
		eoss.writeInt((int) vllValue.size());
		for (unsigned int i = 0; i < vllValue.size(); i++)
				
			eoss.writeLong(vllValue.at(i));
				
				
						
		
	

	}

	eoss.writeBoolean(vvllValueExists);
	if (vvllValueExists) {
	
	
	
		
		
			
		eoss.writeInt((int) vvllValue.size());
		eoss.writeInt((int) vvllValue.at(0).size());
		for (unsigned int i = 0; i < vvllValue.size(); i++) 
			for (unsigned int j = 0;  j < vvllValue.at(0).size(); j++) 
							 
				eoss.writeLong(vvllValue.at(i).at(j));
				
	
						
		
	

	}

	eoss.writeBoolean(antennaIdExists);
	if (antennaIdExists) {
	
	
	
		
	Tag::toBin(antennaId, eoss);
		
	

	}

	}
	
void AnnotationRow::annotationIdFromBin(EndianISStream& eiss) {
		
	
		
		
		annotationId =  Tag::fromBin(eiss);
		
	
	
}
void AnnotationRow::timeFromBin(EndianISStream& eiss) {
		
	
		
		
		time =  ArrayTime::fromBin(eiss);
		
	
	
}
void AnnotationRow::issueFromBin(EndianISStream& eiss) {
		
	
	
		
			
		issue =  eiss.readString();
			
		
	
	
}
void AnnotationRow::detailsFromBin(EndianISStream& eiss) {
		
	
	
		
			
		details =  eiss.readString();
			
		
	
	
}

void AnnotationRow::numAntennaFromBin(EndianISStream& eiss) {
		
	numAntennaExists = eiss.readBoolean();
	if (numAntennaExists) {
		
	
	
		
			
		numAntenna =  eiss.readInt();
			
		
	

	}
	
}
void AnnotationRow::basebandNameFromBin(EndianISStream& eiss) {
		
	basebandNameExists = eiss.readBoolean();
	if (basebandNameExists) {
		
	
	
		
			
	
		basebandName.clear();
		
		unsigned int basebandNameDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < basebandNameDim1; i++)
			
			basebandName.push_back(CBasebandName::from_int(eiss.readInt()));
			
	

		
	

	}
	
}
void AnnotationRow::numBasebandFromBin(EndianISStream& eiss) {
		
	numBasebandExists = eiss.readBoolean();
	if (numBasebandExists) {
		
	
	
		
			
		numBaseband =  eiss.readInt();
			
		
	

	}
	
}
void AnnotationRow::intervalFromBin(EndianISStream& eiss) {
		
	intervalExists = eiss.readBoolean();
	if (intervalExists) {
		
	
		
		
		interval =  Interval::fromBin(eiss);
		
	

	}
	
}
void AnnotationRow::dValueFromBin(EndianISStream& eiss) {
		
	dValueExists = eiss.readBoolean();
	if (dValueExists) {
		
	
	
		
			
		dValue =  eiss.readDouble();
			
		
	

	}
	
}
void AnnotationRow::vdValueFromBin(EndianISStream& eiss) {
		
	vdValueExists = eiss.readBoolean();
	if (vdValueExists) {
		
	
	
		
			
	
		vdValue.clear();
		
		unsigned int vdValueDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < vdValueDim1; i++)
			
			vdValue.push_back(eiss.readDouble());
			
	

		
	

	}
	
}
void AnnotationRow::vvdValuesFromBin(EndianISStream& eiss) {
		
	vvdValuesExists = eiss.readBoolean();
	if (vvdValuesExists) {
		
	
	
		
			
	
		vvdValues.clear();
		
		unsigned int vvdValuesDim1 = eiss.readInt();
		unsigned int vvdValuesDim2 = eiss.readInt();
		vector <double> vvdValuesAux1;
		for (unsigned int i = 0; i < vvdValuesDim1; i++) {
			vvdValuesAux1.clear();
			for (unsigned int j = 0; j < vvdValuesDim2 ; j++)			
			
			vvdValuesAux1.push_back(eiss.readDouble());
			
			vvdValues.push_back(vvdValuesAux1);
		}
	
	

		
	

	}
	
}
void AnnotationRow::llValueFromBin(EndianISStream& eiss) {
		
	llValueExists = eiss.readBoolean();
	if (llValueExists) {
		
	
	
		
			
		llValue =  eiss.readLong();
			
		
	

	}
	
}
void AnnotationRow::vllValueFromBin(EndianISStream& eiss) {
		
	vllValueExists = eiss.readBoolean();
	if (vllValueExists) {
		
	
	
		
			
	
		vllValue.clear();
		
		unsigned int vllValueDim1 = eiss.readInt();
		for (unsigned int  i = 0 ; i < vllValueDim1; i++)
			
			vllValue.push_back(eiss.readLong());
			
	

		
	

	}
	
}
void AnnotationRow::vvllValueFromBin(EndianISStream& eiss) {
		
	vvllValueExists = eiss.readBoolean();
	if (vvllValueExists) {
		
	
	
		
			
	
		vvllValue.clear();
		
		unsigned int vvllValueDim1 = eiss.readInt();
		unsigned int vvllValueDim2 = eiss.readInt();
		vector <long long> vvllValueAux1;
		for (unsigned int i = 0; i < vvllValueDim1; i++) {
			vvllValueAux1.clear();
			for (unsigned int j = 0; j < vvllValueDim2 ; j++)			
			
			vvllValueAux1.push_back(eiss.readLong());
			
			vvllValue.push_back(vvllValueAux1);
		}
	
	

		
	

	}
	
}
void AnnotationRow::antennaIdFromBin(EndianISStream& eiss) {
		
	antennaIdExists = eiss.readBoolean();
	if (antennaIdExists) {
		
	
		
		
			
	
	antennaId = Tag::from1DBin(eiss);	
	

		
	

	}
	
}
	
	
	AnnotationRow* AnnotationRow::fromBin(EndianISStream& eiss, AnnotationTable& table, const vector<string>& attributesSeq) {
		AnnotationRow* row = new  AnnotationRow(table);
		
		map<string, AnnotationAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "AnnotationTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get annotationId.
 	 * @return annotationId as Tag
 	 */
 	Tag AnnotationRow::getAnnotationId() const {
	
  		return annotationId;
 	}

 	/**
 	 * Set annotationId with the specified Tag.
 	 * @param annotationId The Tag value to which annotationId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void AnnotationRow::setAnnotationId (Tag annotationId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("annotationId", "Annotation");
		
  		}
  	
 		this->annotationId = annotationId;
	
 	}
	
	

	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime AnnotationRow::getTime() const {
	
  		return time;
 	}

 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 	
 		
 	 */
 	void AnnotationRow::setTime (ArrayTime time)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->time = time;
	
 	}
	
	

	

	
 	/**
 	 * Get issue.
 	 * @return issue as string
 	 */
 	string AnnotationRow::getIssue() const {
	
  		return issue;
 	}

 	/**
 	 * Set issue with the specified string.
 	 * @param issue The string value to which issue is to be set.
 	 
 	
 		
 	 */
 	void AnnotationRow::setIssue (string issue)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->issue = issue;
	
 	}
	
	

	

	
 	/**
 	 * Get details.
 	 * @return details as string
 	 */
 	string AnnotationRow::getDetails() const {
	
  		return details;
 	}

 	/**
 	 * Set details with the specified string.
 	 * @param details The string value to which details is to be set.
 	 
 	
 		
 	 */
 	void AnnotationRow::setDetails (string details)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->details = details;
	
 	}
	
	

	
	/**
	 * The attribute numAntenna is optional. Return true if this attribute exists.
	 * @return true if and only if the numAntenna attribute exists. 
	 */
	bool AnnotationRow::isNumAntennaExists() const {
		return numAntennaExists;
	}
	

	
 	/**
 	 * Get numAntenna, which is optional.
 	 * @return numAntenna as int
 	 * @throw IllegalAccessException If numAntenna does not exist.
 	 */
 	int AnnotationRow::getNumAntenna() const  {
		if (!numAntennaExists) {
			throw IllegalAccessException("numAntenna", "Annotation");
		}
	
  		return numAntenna;
 	}

 	/**
 	 * Set numAntenna with the specified int.
 	 * @param numAntenna The int value to which numAntenna is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setNumAntenna (int numAntenna) {
	
 		this->numAntenna = numAntenna;
	
		numAntennaExists = true;
	
 	}
	
	
	/**
	 * Mark numAntenna, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearNumAntenna () {
		numAntennaExists = false;
	}
	

	
	/**
	 * The attribute basebandName is optional. Return true if this attribute exists.
	 * @return true if and only if the basebandName attribute exists. 
	 */
	bool AnnotationRow::isBasebandNameExists() const {
		return basebandNameExists;
	}
	

	
 	/**
 	 * Get basebandName, which is optional.
 	 * @return basebandName as vector<BasebandNameMod::BasebandName >
 	 * @throw IllegalAccessException If basebandName does not exist.
 	 */
 	vector<BasebandNameMod::BasebandName > AnnotationRow::getBasebandName() const  {
		if (!basebandNameExists) {
			throw IllegalAccessException("basebandName", "Annotation");
		}
	
  		return basebandName;
 	}

 	/**
 	 * Set basebandName with the specified vector<BasebandNameMod::BasebandName >.
 	 * @param basebandName The vector<BasebandNameMod::BasebandName > value to which basebandName is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setBasebandName (vector<BasebandNameMod::BasebandName > basebandName) {
	
 		this->basebandName = basebandName;
	
		basebandNameExists = true;
	
 	}
	
	
	/**
	 * Mark basebandName, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearBasebandName () {
		basebandNameExists = false;
	}
	

	
	/**
	 * The attribute numBaseband is optional. Return true if this attribute exists.
	 * @return true if and only if the numBaseband attribute exists. 
	 */
	bool AnnotationRow::isNumBasebandExists() const {
		return numBasebandExists;
	}
	

	
 	/**
 	 * Get numBaseband, which is optional.
 	 * @return numBaseband as int
 	 * @throw IllegalAccessException If numBaseband does not exist.
 	 */
 	int AnnotationRow::getNumBaseband() const  {
		if (!numBasebandExists) {
			throw IllegalAccessException("numBaseband", "Annotation");
		}
	
  		return numBaseband;
 	}

 	/**
 	 * Set numBaseband with the specified int.
 	 * @param numBaseband The int value to which numBaseband is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setNumBaseband (int numBaseband) {
	
 		this->numBaseband = numBaseband;
	
		numBasebandExists = true;
	
 	}
	
	
	/**
	 * Mark numBaseband, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearNumBaseband () {
		numBasebandExists = false;
	}
	

	
	/**
	 * The attribute interval is optional. Return true if this attribute exists.
	 * @return true if and only if the interval attribute exists. 
	 */
	bool AnnotationRow::isIntervalExists() const {
		return intervalExists;
	}
	

	
 	/**
 	 * Get interval, which is optional.
 	 * @return interval as Interval
 	 * @throw IllegalAccessException If interval does not exist.
 	 */
 	Interval AnnotationRow::getInterval() const  {
		if (!intervalExists) {
			throw IllegalAccessException("interval", "Annotation");
		}
	
  		return interval;
 	}

 	/**
 	 * Set interval with the specified Interval.
 	 * @param interval The Interval value to which interval is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setInterval (Interval interval) {
	
 		this->interval = interval;
	
		intervalExists = true;
	
 	}
	
	
	/**
	 * Mark interval, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearInterval () {
		intervalExists = false;
	}
	

	
	/**
	 * The attribute dValue is optional. Return true if this attribute exists.
	 * @return true if and only if the dValue attribute exists. 
	 */
	bool AnnotationRow::isDValueExists() const {
		return dValueExists;
	}
	

	
 	/**
 	 * Get dValue, which is optional.
 	 * @return dValue as double
 	 * @throw IllegalAccessException If dValue does not exist.
 	 */
 	double AnnotationRow::getDValue() const  {
		if (!dValueExists) {
			throw IllegalAccessException("dValue", "Annotation");
		}
	
  		return dValue;
 	}

 	/**
 	 * Set dValue with the specified double.
 	 * @param dValue The double value to which dValue is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setDValue (double dValue) {
	
 		this->dValue = dValue;
	
		dValueExists = true;
	
 	}
	
	
	/**
	 * Mark dValue, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearDValue () {
		dValueExists = false;
	}
	

	
	/**
	 * The attribute vdValue is optional. Return true if this attribute exists.
	 * @return true if and only if the vdValue attribute exists. 
	 */
	bool AnnotationRow::isVdValueExists() const {
		return vdValueExists;
	}
	

	
 	/**
 	 * Get vdValue, which is optional.
 	 * @return vdValue as vector<double >
 	 * @throw IllegalAccessException If vdValue does not exist.
 	 */
 	vector<double > AnnotationRow::getVdValue() const  {
		if (!vdValueExists) {
			throw IllegalAccessException("vdValue", "Annotation");
		}
	
  		return vdValue;
 	}

 	/**
 	 * Set vdValue with the specified vector<double >.
 	 * @param vdValue The vector<double > value to which vdValue is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setVdValue (vector<double > vdValue) {
	
 		this->vdValue = vdValue;
	
		vdValueExists = true;
	
 	}
	
	
	/**
	 * Mark vdValue, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearVdValue () {
		vdValueExists = false;
	}
	

	
	/**
	 * The attribute vvdValues is optional. Return true if this attribute exists.
	 * @return true if and only if the vvdValues attribute exists. 
	 */
	bool AnnotationRow::isVvdValuesExists() const {
		return vvdValuesExists;
	}
	

	
 	/**
 	 * Get vvdValues, which is optional.
 	 * @return vvdValues as vector<vector<double > >
 	 * @throw IllegalAccessException If vvdValues does not exist.
 	 */
 	vector<vector<double > > AnnotationRow::getVvdValues() const  {
		if (!vvdValuesExists) {
			throw IllegalAccessException("vvdValues", "Annotation");
		}
	
  		return vvdValues;
 	}

 	/**
 	 * Set vvdValues with the specified vector<vector<double > >.
 	 * @param vvdValues The vector<vector<double > > value to which vvdValues is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setVvdValues (vector<vector<double > > vvdValues) {
	
 		this->vvdValues = vvdValues;
	
		vvdValuesExists = true;
	
 	}
	
	
	/**
	 * Mark vvdValues, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearVvdValues () {
		vvdValuesExists = false;
	}
	

	
	/**
	 * The attribute llValue is optional. Return true if this attribute exists.
	 * @return true if and only if the llValue attribute exists. 
	 */
	bool AnnotationRow::isLlValueExists() const {
		return llValueExists;
	}
	

	
 	/**
 	 * Get llValue, which is optional.
 	 * @return llValue as long long
 	 * @throw IllegalAccessException If llValue does not exist.
 	 */
 	long long AnnotationRow::getLlValue() const  {
		if (!llValueExists) {
			throw IllegalAccessException("llValue", "Annotation");
		}
	
  		return llValue;
 	}

 	/**
 	 * Set llValue with the specified long long.
 	 * @param llValue The long long value to which llValue is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setLlValue (long long llValue) {
	
 		this->llValue = llValue;
	
		llValueExists = true;
	
 	}
	
	
	/**
	 * Mark llValue, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearLlValue () {
		llValueExists = false;
	}
	

	
	/**
	 * The attribute vllValue is optional. Return true if this attribute exists.
	 * @return true if and only if the vllValue attribute exists. 
	 */
	bool AnnotationRow::isVllValueExists() const {
		return vllValueExists;
	}
	

	
 	/**
 	 * Get vllValue, which is optional.
 	 * @return vllValue as vector<long long >
 	 * @throw IllegalAccessException If vllValue does not exist.
 	 */
 	vector<long long > AnnotationRow::getVllValue() const  {
		if (!vllValueExists) {
			throw IllegalAccessException("vllValue", "Annotation");
		}
	
  		return vllValue;
 	}

 	/**
 	 * Set vllValue with the specified vector<long long >.
 	 * @param vllValue The vector<long long > value to which vllValue is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setVllValue (vector<long long > vllValue) {
	
 		this->vllValue = vllValue;
	
		vllValueExists = true;
	
 	}
	
	
	/**
	 * Mark vllValue, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearVllValue () {
		vllValueExists = false;
	}
	

	
	/**
	 * The attribute vvllValue is optional. Return true if this attribute exists.
	 * @return true if and only if the vvllValue attribute exists. 
	 */
	bool AnnotationRow::isVvllValueExists() const {
		return vvllValueExists;
	}
	

	
 	/**
 	 * Get vvllValue, which is optional.
 	 * @return vvllValue as vector<vector<long long > >
 	 * @throw IllegalAccessException If vvllValue does not exist.
 	 */
 	vector<vector<long long > > AnnotationRow::getVvllValue() const  {
		if (!vvllValueExists) {
			throw IllegalAccessException("vvllValue", "Annotation");
		}
	
  		return vvllValue;
 	}

 	/**
 	 * Set vvllValue with the specified vector<vector<long long > >.
 	 * @param vvllValue The vector<vector<long long > > value to which vvllValue is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setVvllValue (vector<vector<long long > > vvllValue) {
	
 		this->vvllValue = vvllValue;
	
		vvllValueExists = true;
	
 	}
	
	
	/**
	 * Mark vvllValue, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearVvllValue () {
		vvllValueExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	/**
	 * The attribute antennaId is optional. Return true if this attribute exists.
	 * @return true if and only if the antennaId attribute exists. 
	 */
	bool AnnotationRow::isAntennaIdExists() const {
		return antennaIdExists;
	}
	

	
 	/**
 	 * Get antennaId, which is optional.
 	 * @return antennaId as vector<Tag> 
 	 * @throw IllegalAccessException If antennaId does not exist.
 	 */
 	vector<Tag>  AnnotationRow::getAntennaId() const  {
		if (!antennaIdExists) {
			throw IllegalAccessException("antennaId", "Annotation");
		}
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified vector<Tag> .
 	 * @param antennaId The vector<Tag>  value to which antennaId is to be set.
 	 
 	
 	 */
 	void AnnotationRow::setAntennaId (vector<Tag>  antennaId) {
	
 		this->antennaId = antennaId;
	
		antennaIdExists = true;
	
 	}
	
	
	/**
	 * Mark antennaId, which is an optional field, as non-existent.
	 */
	void AnnotationRow::clearAntennaId () {
		antennaIdExists = false;
	}
	

	///////////
	// Links //
	///////////
	
	
 		
 	/**
 	 * Set antennaId[i] with the specified Tag.
 	 * @param i The index in antennaId where to set the Tag value.
 	 * @param antennaId The Tag value to which antennaId[i] is to be set. 
 	 * @throws OutOfBoundsException
  	 */
  	void AnnotationRow::setAntennaId (int i, Tag antennaId) {
  		if ((i < 0) || (i > ((int) this->antennaId.size())))
  			throw OutOfBoundsException("Index out of bounds during a set operation on attribute antennaId in table AnnotationTable");
  		vector<Tag> ::iterator iter = this->antennaId.begin();
  		int j = 0;
  		while (j < i) {
  			j++; iter++;
  		}
  		this->antennaId.insert(this->antennaId.erase(iter), antennaId); 	
  	}
 			
	
	
	
		
/**
 * Append a Tag to antennaId.
 * @param id the Tag to be appended to antennaId
 */
 void AnnotationRow::addAntennaId(Tag id){
 	antennaId.push_back(id);
}

/**
 * Append an array of Tag to antennaId.
 * @param id an array of Tag to be appended to antennaId
 */
 void AnnotationRow::addAntennaId(const vector<Tag> & id) {
 	for (unsigned int i=0; i < id.size(); i++)
 		antennaId.push_back(id.at(i));
 }
 

 /**
  * Returns the Tag stored in antennaId at position i.
  *
  */
 const Tag AnnotationRow::getAntennaId(int i) {
 	return antennaId.at(i);
 }
 
 /**
  * Returns the AntennaRow linked to this row via the Tag stored in antennaId
  * at position i.
  */
 AntennaRow* AnnotationRow::getAntenna(int i) {
 	return table.getContainer().getAntenna().getRowByKey(antennaId.at(i));
 } 
 
 /**
  * Returns the vector of AntennaRow* linked to this row via the Tags stored in antennaId
  *
  */
 vector<AntennaRow *> AnnotationRow::getAntennas() {
 	vector<AntennaRow *> result;
 	for (unsigned int i = 0; i < antennaId.size(); i++)
 		result.push_back(table.getContainer().getAntenna().getRowByKey(antennaId.at(i)));
 		
 	return result;
 }
  

	

	
	/**
	 * Create a AnnotationRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	AnnotationRow::AnnotationRow (AnnotationTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	
		numAntennaExists = false;
	

	
		basebandNameExists = false;
	

	
		numBasebandExists = false;
	

	
		intervalExists = false;
	

	
		dValueExists = false;
	

	
		vdValueExists = false;
	

	
		vvdValuesExists = false;
	

	
		llValueExists = false;
	

	
		vllValueExists = false;
	

	
		vvllValueExists = false;
	

	
	
		antennaIdExists = false;
	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["annotationId"] = &AnnotationRow::annotationIdFromBin; 
	 fromBinMethods["time"] = &AnnotationRow::timeFromBin; 
	 fromBinMethods["issue"] = &AnnotationRow::issueFromBin; 
	 fromBinMethods["details"] = &AnnotationRow::detailsFromBin; 
		
	
	 fromBinMethods["numAntenna"] = &AnnotationRow::numAntennaFromBin; 
	 fromBinMethods["basebandName"] = &AnnotationRow::basebandNameFromBin; 
	 fromBinMethods["numBaseband"] = &AnnotationRow::numBasebandFromBin; 
	 fromBinMethods["interval"] = &AnnotationRow::intervalFromBin; 
	 fromBinMethods["dValue"] = &AnnotationRow::dValueFromBin; 
	 fromBinMethods["vdValue"] = &AnnotationRow::vdValueFromBin; 
	 fromBinMethods["vvdValues"] = &AnnotationRow::vvdValuesFromBin; 
	 fromBinMethods["llValue"] = &AnnotationRow::llValueFromBin; 
	 fromBinMethods["vllValue"] = &AnnotationRow::vllValueFromBin; 
	 fromBinMethods["vvllValue"] = &AnnotationRow::vvllValueFromBin; 
	 fromBinMethods["antennaId"] = &AnnotationRow::antennaIdFromBin; 
	
	}
	
	AnnotationRow::AnnotationRow (AnnotationTable &t, AnnotationRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	
		numAntennaExists = false;
	

	
		basebandNameExists = false;
	

	
		numBasebandExists = false;
	

	
		intervalExists = false;
	

	
		dValueExists = false;
	

	
		vdValueExists = false;
	

	
		vvdValuesExists = false;
	

	
		llValueExists = false;
	

	
		vllValueExists = false;
	

	
		vvllValueExists = false;
	

	
	
		antennaIdExists = false;
	
		
		}
		else {
	
		
			annotationId = row.annotationId;
		
		
		
		
			time = row.time;
		
			issue = row.issue;
		
			details = row.details;
		
		
		
		
		if (row.numAntennaExists) {
			numAntenna = row.numAntenna;		
			numAntennaExists = true;
		}
		else
			numAntennaExists = false;
		
		if (row.basebandNameExists) {
			basebandName = row.basebandName;		
			basebandNameExists = true;
		}
		else
			basebandNameExists = false;
		
		if (row.numBasebandExists) {
			numBaseband = row.numBaseband;		
			numBasebandExists = true;
		}
		else
			numBasebandExists = false;
		
		if (row.intervalExists) {
			interval = row.interval;		
			intervalExists = true;
		}
		else
			intervalExists = false;
		
		if (row.dValueExists) {
			dValue = row.dValue;		
			dValueExists = true;
		}
		else
			dValueExists = false;
		
		if (row.vdValueExists) {
			vdValue = row.vdValue;		
			vdValueExists = true;
		}
		else
			vdValueExists = false;
		
		if (row.vvdValuesExists) {
			vvdValues = row.vvdValues;		
			vvdValuesExists = true;
		}
		else
			vvdValuesExists = false;
		
		if (row.llValueExists) {
			llValue = row.llValue;		
			llValueExists = true;
		}
		else
			llValueExists = false;
		
		if (row.vllValueExists) {
			vllValue = row.vllValue;		
			vllValueExists = true;
		}
		else
			vllValueExists = false;
		
		if (row.vvllValueExists) {
			vvllValue = row.vvllValue;		
			vvllValueExists = true;
		}
		else
			vvllValueExists = false;
		
		if (row.antennaIdExists) {
			antennaId = row.antennaId;		
			antennaIdExists = true;
		}
		else
			antennaIdExists = false;
		
		}
		
		 fromBinMethods["annotationId"] = &AnnotationRow::annotationIdFromBin; 
		 fromBinMethods["time"] = &AnnotationRow::timeFromBin; 
		 fromBinMethods["issue"] = &AnnotationRow::issueFromBin; 
		 fromBinMethods["details"] = &AnnotationRow::detailsFromBin; 
			
	
		 fromBinMethods["numAntenna"] = &AnnotationRow::numAntennaFromBin; 
		 fromBinMethods["basebandName"] = &AnnotationRow::basebandNameFromBin; 
		 fromBinMethods["numBaseband"] = &AnnotationRow::numBasebandFromBin; 
		 fromBinMethods["interval"] = &AnnotationRow::intervalFromBin; 
		 fromBinMethods["dValue"] = &AnnotationRow::dValueFromBin; 
		 fromBinMethods["vdValue"] = &AnnotationRow::vdValueFromBin; 
		 fromBinMethods["vvdValues"] = &AnnotationRow::vvdValuesFromBin; 
		 fromBinMethods["llValue"] = &AnnotationRow::llValueFromBin; 
		 fromBinMethods["vllValue"] = &AnnotationRow::vllValueFromBin; 
		 fromBinMethods["vvllValue"] = &AnnotationRow::vvllValueFromBin; 
		 fromBinMethods["antennaId"] = &AnnotationRow::antennaIdFromBin; 
			
	}

	
	bool AnnotationRow::compareNoAutoInc(ArrayTime time, string issue, string details) {
		bool result;
		result = true;
		
	
		
		result = result && (this->time == time);
		
		if (!result) return false;
	

	
		
		result = result && (this->issue == issue);
		
		if (!result) return false;
	

	
		
		result = result && (this->details == details);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool AnnotationRow::compareRequiredValue(ArrayTime time, string issue, string details) {
		bool result;
		result = true;
		
	
		if (!(this->time == time)) return false;
	

	
		if (!(this->issue == issue)) return false;
	

	
		if (!(this->details == details)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the AnnotationRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool AnnotationRow::equalByRequiredValue(AnnotationRow* x) {
		
			
		if (this->time != x->time) return false;
			
		if (this->issue != x->issue) return false;
			
		if (this->details != x->details) return false;
			
		
		return true;
	}	
	
/*
	 map<string, AnnotationAttributeFromBin> AnnotationRow::initFromBinMethods() {
		map<string, AnnotationAttributeFromBin> result;
		
		result["annotationId"] = &AnnotationRow::annotationIdFromBin;
		result["time"] = &AnnotationRow::timeFromBin;
		result["issue"] = &AnnotationRow::issueFromBin;
		result["details"] = &AnnotationRow::detailsFromBin;
		
		
		result["numAntenna"] = &AnnotationRow::numAntennaFromBin;
		result["basebandName"] = &AnnotationRow::basebandNameFromBin;
		result["numBaseband"] = &AnnotationRow::numBasebandFromBin;
		result["interval"] = &AnnotationRow::intervalFromBin;
		result["dValue"] = &AnnotationRow::dValueFromBin;
		result["vdValue"] = &AnnotationRow::vdValueFromBin;
		result["vvdValues"] = &AnnotationRow::vvdValuesFromBin;
		result["llValue"] = &AnnotationRow::llValueFromBin;
		result["vllValue"] = &AnnotationRow::vllValueFromBin;
		result["vvllValue"] = &AnnotationRow::vvllValueFromBin;
		result["antennaId"] = &AnnotationRow::antennaIdFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
