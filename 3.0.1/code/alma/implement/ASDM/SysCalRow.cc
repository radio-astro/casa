
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
 * File SysCalRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <SysCalRow.h>
#include <SysCalTable.h>

#include <AntennaTable.h>
#include <AntennaRow.h>

#include <FeedTable.h>
#include <FeedRow.h>

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>
	

using asdm::ASDM;
using asdm::SysCalRow;
using asdm::SysCalTable;

using asdm::AntennaTable;
using asdm::AntennaRow;

using asdm::FeedTable;
using asdm::FeedRow;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	SysCalRow::~SysCalRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	SysCalTable &SysCalRow::getTable() const {
		return table;
	}
	
	void SysCalRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a SysCalRowIDL struct.
	 */
	SysCalRowIDL *SysCalRow::toIDL() const {
		SysCalRowIDL *x = new SysCalRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->numReceptor = numReceptor;
 				
 			
		
	

	
  		
		
		
			
				
		x->numChan = numChan;
 				
 			
		
	

	
  		
		
		x->tcalFlagExists = tcalFlagExists;
		
		
			
				
		x->tcalFlag = tcalFlag;
 				
 			
		
	

	
  		
		
		x->tcalSpectrumExists = tcalSpectrumExists;
		
		
			
		x->tcalSpectrum.length(tcalSpectrum.size());
		for (unsigned int i = 0; i < tcalSpectrum.size(); i++) {
			x->tcalSpectrum[i].length(tcalSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tcalSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tcalSpectrum.at(i).size(); j++)
					
				x->tcalSpectrum[i][j]= tcalSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->trxFlagExists = trxFlagExists;
		
		
			
				
		x->trxFlag = trxFlag;
 				
 			
		
	

	
  		
		
		x->trxSpectrumExists = trxSpectrumExists;
		
		
			
		x->trxSpectrum.length(trxSpectrum.size());
		for (unsigned int i = 0; i < trxSpectrum.size(); i++) {
			x->trxSpectrum[i].length(trxSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < trxSpectrum.size() ; i++)
			for (unsigned int j = 0; j < trxSpectrum.at(i).size(); j++)
					
				x->trxSpectrum[i][j]= trxSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->tskyFlagExists = tskyFlagExists;
		
		
			
				
		x->tskyFlag = tskyFlag;
 				
 			
		
	

	
  		
		
		x->tskySpectrumExists = tskySpectrumExists;
		
		
			
		x->tskySpectrum.length(tskySpectrum.size());
		for (unsigned int i = 0; i < tskySpectrum.size(); i++) {
			x->tskySpectrum[i].length(tskySpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tskySpectrum.size() ; i++)
			for (unsigned int j = 0; j < tskySpectrum.at(i).size(); j++)
					
				x->tskySpectrum[i][j]= tskySpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->tsysFlagExists = tsysFlagExists;
		
		
			
				
		x->tsysFlag = tsysFlag;
 				
 			
		
	

	
  		
		
		x->tsysSpectrumExists = tsysSpectrumExists;
		
		
			
		x->tsysSpectrum.length(tsysSpectrum.size());
		for (unsigned int i = 0; i < tsysSpectrum.size(); i++) {
			x->tsysSpectrum[i].length(tsysSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tsysSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tsysSpectrum.at(i).size(); j++)
					
				x->tsysSpectrum[i][j]= tsysSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->tantFlagExists = tantFlagExists;
		
		
			
				
		x->tantFlag = tantFlag;
 				
 			
		
	

	
  		
		
		x->tantSpectrumExists = tantSpectrumExists;
		
		
			
		x->tantSpectrum.length(tantSpectrum.size());
		for (unsigned int i = 0; i < tantSpectrum.size(); i++) {
			x->tantSpectrum[i].length(tantSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tantSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tantSpectrum.at(i).size(); j++)
					
						
				x->tantSpectrum[i][j] = tantSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->tantTsysFlagExists = tantTsysFlagExists;
		
		
			
				
		x->tantTsysFlag = tantTsysFlag;
 				
 			
		
	

	
  		
		
		x->tantTsysSpectrumExists = tantTsysSpectrumExists;
		
		
			
		x->tantTsysSpectrum.length(tantTsysSpectrum.size());
		for (unsigned int i = 0; i < tantTsysSpectrum.size(); i++) {
			x->tantTsysSpectrum[i].length(tantTsysSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tantTsysSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tantTsysSpectrum.at(i).size(); j++)
					
						
				x->tantTsysSpectrum[i][j] = tantTsysSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->phaseDiffFlagExists = phaseDiffFlagExists;
		
		
			
				
		x->phaseDiffFlag = phaseDiffFlag;
 				
 			
		
	

	
  		
		
		x->phaseDiffSpectrumExists = phaseDiffSpectrumExists;
		
		
			
		x->phaseDiffSpectrum.length(phaseDiffSpectrum.size());
		for (unsigned int i = 0; i < phaseDiffSpectrum.size(); i++) {
			x->phaseDiffSpectrum[i].length(phaseDiffSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < phaseDiffSpectrum.size() ; i++)
			for (unsigned int j = 0; j < phaseDiffSpectrum.at(i).size(); j++)
					
						
				x->phaseDiffSpectrum[i][j] = phaseDiffSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
	
		
	
  	
 		
		
	 	
			
		x->antennaId = antennaId.toIDLTag();
			
	 	 		
  	

	
  	
 		
		
	 	
			
				
		x->feedId = feedId;
 				
 			
	 	 		
  	

	
  	
 		
		
	 	
			
		x->spectralWindowId = spectralWindowId.toIDLTag();
			
	 	 		
  	

	
		
	

	

	

		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct SysCalRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void SysCalRow::setFromIDL (SysCalRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setNumReceptor(x.numReceptor);
  			
 		
		
	

	
		
		
			
		setNumChan(x.numChan);
  			
 		
		
	

	
		
		tcalFlagExists = x.tcalFlagExists;
		if (x.tcalFlagExists) {
		
		
			
		setTcalFlag(x.tcalFlag);
  			
 		
		
		}
		
	

	
		
		tcalSpectrumExists = x.tcalSpectrumExists;
		if (x.tcalSpectrumExists) {
		
		
			
		tcalSpectrum .clear();
		vector<Temperature> v_aux_tcalSpectrum;
		for (unsigned int i = 0; i < x.tcalSpectrum.length(); ++i) {
			v_aux_tcalSpectrum.clear();
			for (unsigned int j = 0; j < x.tcalSpectrum[0].length(); ++j) {
				
				v_aux_tcalSpectrum.push_back(Temperature (x.tcalSpectrum[i][j]));
				
  			}
  			tcalSpectrum.push_back(v_aux_tcalSpectrum);			
		}
			
  		
		
		}
		
	

	
		
		trxFlagExists = x.trxFlagExists;
		if (x.trxFlagExists) {
		
		
			
		setTrxFlag(x.trxFlag);
  			
 		
		
		}
		
	

	
		
		trxSpectrumExists = x.trxSpectrumExists;
		if (x.trxSpectrumExists) {
		
		
			
		trxSpectrum .clear();
		vector<Temperature> v_aux_trxSpectrum;
		for (unsigned int i = 0; i < x.trxSpectrum.length(); ++i) {
			v_aux_trxSpectrum.clear();
			for (unsigned int j = 0; j < x.trxSpectrum[0].length(); ++j) {
				
				v_aux_trxSpectrum.push_back(Temperature (x.trxSpectrum[i][j]));
				
  			}
  			trxSpectrum.push_back(v_aux_trxSpectrum);			
		}
			
  		
		
		}
		
	

	
		
		tskyFlagExists = x.tskyFlagExists;
		if (x.tskyFlagExists) {
		
		
			
		setTskyFlag(x.tskyFlag);
  			
 		
		
		}
		
	

	
		
		tskySpectrumExists = x.tskySpectrumExists;
		if (x.tskySpectrumExists) {
		
		
			
		tskySpectrum .clear();
		vector<Temperature> v_aux_tskySpectrum;
		for (unsigned int i = 0; i < x.tskySpectrum.length(); ++i) {
			v_aux_tskySpectrum.clear();
			for (unsigned int j = 0; j < x.tskySpectrum[0].length(); ++j) {
				
				v_aux_tskySpectrum.push_back(Temperature (x.tskySpectrum[i][j]));
				
  			}
  			tskySpectrum.push_back(v_aux_tskySpectrum);			
		}
			
  		
		
		}
		
	

	
		
		tsysFlagExists = x.tsysFlagExists;
		if (x.tsysFlagExists) {
		
		
			
		setTsysFlag(x.tsysFlag);
  			
 		
		
		}
		
	

	
		
		tsysSpectrumExists = x.tsysSpectrumExists;
		if (x.tsysSpectrumExists) {
		
		
			
		tsysSpectrum .clear();
		vector<Temperature> v_aux_tsysSpectrum;
		for (unsigned int i = 0; i < x.tsysSpectrum.length(); ++i) {
			v_aux_tsysSpectrum.clear();
			for (unsigned int j = 0; j < x.tsysSpectrum[0].length(); ++j) {
				
				v_aux_tsysSpectrum.push_back(Temperature (x.tsysSpectrum[i][j]));
				
  			}
  			tsysSpectrum.push_back(v_aux_tsysSpectrum);			
		}
			
  		
		
		}
		
	

	
		
		tantFlagExists = x.tantFlagExists;
		if (x.tantFlagExists) {
		
		
			
		setTantFlag(x.tantFlag);
  			
 		
		
		}
		
	

	
		
		tantSpectrumExists = x.tantSpectrumExists;
		if (x.tantSpectrumExists) {
		
		
			
		tantSpectrum .clear();
		vector<float> v_aux_tantSpectrum;
		for (unsigned int i = 0; i < x.tantSpectrum.length(); ++i) {
			v_aux_tantSpectrum.clear();
			for (unsigned int j = 0; j < x.tantSpectrum[0].length(); ++j) {
				
				v_aux_tantSpectrum.push_back(x.tantSpectrum[i][j]);
	  			
  			}
  			tantSpectrum.push_back(v_aux_tantSpectrum);			
		}
			
  		
		
		}
		
	

	
		
		tantTsysFlagExists = x.tantTsysFlagExists;
		if (x.tantTsysFlagExists) {
		
		
			
		setTantTsysFlag(x.tantTsysFlag);
  			
 		
		
		}
		
	

	
		
		tantTsysSpectrumExists = x.tantTsysSpectrumExists;
		if (x.tantTsysSpectrumExists) {
		
		
			
		tantTsysSpectrum .clear();
		vector<float> v_aux_tantTsysSpectrum;
		for (unsigned int i = 0; i < x.tantTsysSpectrum.length(); ++i) {
			v_aux_tantTsysSpectrum.clear();
			for (unsigned int j = 0; j < x.tantTsysSpectrum[0].length(); ++j) {
				
				v_aux_tantTsysSpectrum.push_back(x.tantTsysSpectrum[i][j]);
	  			
  			}
  			tantTsysSpectrum.push_back(v_aux_tantTsysSpectrum);			
		}
			
  		
		
		}
		
	

	
		
		phaseDiffFlagExists = x.phaseDiffFlagExists;
		if (x.phaseDiffFlagExists) {
		
		
			
		setPhaseDiffFlag(x.phaseDiffFlag);
  			
 		
		
		}
		
	

	
		
		phaseDiffSpectrumExists = x.phaseDiffSpectrumExists;
		if (x.phaseDiffSpectrumExists) {
		
		
			
		phaseDiffSpectrum .clear();
		vector<float> v_aux_phaseDiffSpectrum;
		for (unsigned int i = 0; i < x.phaseDiffSpectrum.length(); ++i) {
			v_aux_phaseDiffSpectrum.clear();
			for (unsigned int j = 0; j < x.phaseDiffSpectrum[0].length(); ++j) {
				
				v_aux_phaseDiffSpectrum.push_back(x.phaseDiffSpectrum[i][j]);
	  			
  			}
  			phaseDiffSpectrum.push_back(v_aux_phaseDiffSpectrum);			
		}
			
  		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SysCal");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string SysCalRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(numReceptor, "numReceptor", buf);
		
		
	

  	
 		
		
		Parser::toXML(numChan, "numChan", buf);
		
		
	

  	
 		
		if (tcalFlagExists) {
		
		
		Parser::toXML(tcalFlag, "tcalFlag", buf);
		
		
		}
		
	

  	
 		
		if (tcalSpectrumExists) {
		
		
		Parser::toXML(tcalSpectrum, "tcalSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (trxFlagExists) {
		
		
		Parser::toXML(trxFlag, "trxFlag", buf);
		
		
		}
		
	

  	
 		
		if (trxSpectrumExists) {
		
		
		Parser::toXML(trxSpectrum, "trxSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tskyFlagExists) {
		
		
		Parser::toXML(tskyFlag, "tskyFlag", buf);
		
		
		}
		
	

  	
 		
		if (tskySpectrumExists) {
		
		
		Parser::toXML(tskySpectrum, "tskySpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tsysFlagExists) {
		
		
		Parser::toXML(tsysFlag, "tsysFlag", buf);
		
		
		}
		
	

  	
 		
		if (tsysSpectrumExists) {
		
		
		Parser::toXML(tsysSpectrum, "tsysSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tantFlagExists) {
		
		
		Parser::toXML(tantFlag, "tantFlag", buf);
		
		
		}
		
	

  	
 		
		if (tantSpectrumExists) {
		
		
		Parser::toXML(tantSpectrum, "tantSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tantTsysFlagExists) {
		
		
		Parser::toXML(tantTsysFlag, "tantTsysFlag", buf);
		
		
		}
		
	

  	
 		
		if (tantTsysSpectrumExists) {
		
		
		Parser::toXML(tantTsysSpectrum, "tantTsysSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (phaseDiffFlagExists) {
		
		
		Parser::toXML(phaseDiffFlag, "phaseDiffFlag", buf);
		
		
		}
		
	

  	
 		
		if (phaseDiffSpectrumExists) {
		
		
		Parser::toXML(phaseDiffSpectrum, "phaseDiffSpectrum", buf);
		
		
		}
		
	

	
	
		
  	
 		
		
		Parser::toXML(antennaId, "antennaId", buf);
		
		
	

  	
 		
		
		Parser::toXML(feedId, "feedId", buf);
		
		
	

  	
 		
		
		Parser::toXML(spectralWindowId, "spectralWindowId", buf);
		
		
	

	
		
	

	

	

		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void SysCalRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","SysCal",rowDoc));
			
		
	

	
  		
			
	  	setNumReceptor(Parser::getInteger("numReceptor","SysCal",rowDoc));
			
		
	

	
  		
			
	  	setNumChan(Parser::getInteger("numChan","SysCal",rowDoc));
			
		
	

	
  		
        if (row.isStr("<tcalFlag>")) {
			
	  		setTcalFlag(Parser::getBoolean("tcalFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tcalSpectrum>")) {
			
								
	  		setTcalSpectrum(Parser::get2DTemperature("tcalSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<trxFlag>")) {
			
	  		setTrxFlag(Parser::getBoolean("trxFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<trxSpectrum>")) {
			
								
	  		setTrxSpectrum(Parser::get2DTemperature("trxSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tskyFlag>")) {
			
	  		setTskyFlag(Parser::getBoolean("tskyFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tskySpectrum>")) {
			
								
	  		setTskySpectrum(Parser::get2DTemperature("tskySpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tsysFlag>")) {
			
	  		setTsysFlag(Parser::getBoolean("tsysFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tsysSpectrum>")) {
			
								
	  		setTsysSpectrum(Parser::get2DTemperature("tsysSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tantFlag>")) {
			
	  		setTantFlag(Parser::getBoolean("tantFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tantSpectrum>")) {
			
								
	  		setTantSpectrum(Parser::get2DFloat("tantSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tantTsysFlag>")) {
			
	  		setTantTsysFlag(Parser::getBoolean("tantTsysFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tantTsysSpectrum>")) {
			
								
	  		setTantTsysSpectrum(Parser::get2DFloat("tantTsysSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<phaseDiffFlag>")) {
			
	  		setPhaseDiffFlag(Parser::getBoolean("phaseDiffFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<phaseDiffSpectrum>")) {
			
								
	  		setPhaseDiffSpectrum(Parser::get2DFloat("phaseDiffSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SysCal");
		}
	}
	
	void SysCalRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	antennaId.toBin(eoss);
		
	

	
	
		
	spectralWindowId.toBin(eoss);
		
	

	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeInt(feedId);
				
		
	

	
	
		
						
			eoss.writeInt(numReceptor);
				
		
	

	
	
		
						
			eoss.writeInt(numChan);
				
		
	


	
	
	eoss.writeBoolean(tcalFlagExists);
	if (tcalFlagExists) {
	
	
	
		
						
			eoss.writeBoolean(tcalFlag);
				
		
	

	}

	eoss.writeBoolean(tcalSpectrumExists);
	if (tcalSpectrumExists) {
	
	
	
		
	Temperature::toBin(tcalSpectrum, eoss);
		
	

	}

	eoss.writeBoolean(trxFlagExists);
	if (trxFlagExists) {
	
	
	
		
						
			eoss.writeBoolean(trxFlag);
				
		
	

	}

	eoss.writeBoolean(trxSpectrumExists);
	if (trxSpectrumExists) {
	
	
	
		
	Temperature::toBin(trxSpectrum, eoss);
		
	

	}

	eoss.writeBoolean(tskyFlagExists);
	if (tskyFlagExists) {
	
	
	
		
						
			eoss.writeBoolean(tskyFlag);
				
		
	

	}

	eoss.writeBoolean(tskySpectrumExists);
	if (tskySpectrumExists) {
	
	
	
		
	Temperature::toBin(tskySpectrum, eoss);
		
	

	}

	eoss.writeBoolean(tsysFlagExists);
	if (tsysFlagExists) {
	
	
	
		
						
			eoss.writeBoolean(tsysFlag);
				
		
	

	}

	eoss.writeBoolean(tsysSpectrumExists);
	if (tsysSpectrumExists) {
	
	
	
		
	Temperature::toBin(tsysSpectrum, eoss);
		
	

	}

	eoss.writeBoolean(tantFlagExists);
	if (tantFlagExists) {
	
	
	
		
						
			eoss.writeBoolean(tantFlag);
				
		
	

	}

	eoss.writeBoolean(tantSpectrumExists);
	if (tantSpectrumExists) {
	
	
	
		
		
			
		eoss.writeInt((int) tantSpectrum.size());
		eoss.writeInt((int) tantSpectrum.at(0).size());
		for (unsigned int i = 0; i < tantSpectrum.size(); i++) 
			for (unsigned int j = 0;  j < tantSpectrum.at(0).size(); j++) 
							 
				eoss.writeFloat(tantSpectrum.at(i).at(j));
				
	
						
		
	

	}

	eoss.writeBoolean(tantTsysFlagExists);
	if (tantTsysFlagExists) {
	
	
	
		
						
			eoss.writeBoolean(tantTsysFlag);
				
		
	

	}

	eoss.writeBoolean(tantTsysSpectrumExists);
	if (tantTsysSpectrumExists) {
	
	
	
		
		
			
		eoss.writeInt((int) tantTsysSpectrum.size());
		eoss.writeInt((int) tantTsysSpectrum.at(0).size());
		for (unsigned int i = 0; i < tantTsysSpectrum.size(); i++) 
			for (unsigned int j = 0;  j < tantTsysSpectrum.at(0).size(); j++) 
							 
				eoss.writeFloat(tantTsysSpectrum.at(i).at(j));
				
	
						
		
	

	}

	eoss.writeBoolean(phaseDiffFlagExists);
	if (phaseDiffFlagExists) {
	
	
	
		
						
			eoss.writeBoolean(phaseDiffFlag);
				
		
	

	}

	eoss.writeBoolean(phaseDiffSpectrumExists);
	if (phaseDiffSpectrumExists) {
	
	
	
		
		
			
		eoss.writeInt((int) phaseDiffSpectrum.size());
		eoss.writeInt((int) phaseDiffSpectrum.at(0).size());
		for (unsigned int i = 0; i < phaseDiffSpectrum.size(); i++) 
			for (unsigned int j = 0;  j < phaseDiffSpectrum.at(0).size(); j++) 
							 
				eoss.writeFloat(phaseDiffSpectrum.at(i).at(j));
				
	
						
		
	

	}

	}
	
void SysCalRow::antennaIdFromBin(EndianISStream& eiss) {
		
	
		
		
		antennaId =  Tag::fromBin(eiss);
		
	
	
}
void SysCalRow::spectralWindowIdFromBin(EndianISStream& eiss) {
		
	
		
		
		spectralWindowId =  Tag::fromBin(eiss);
		
	
	
}
void SysCalRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void SysCalRow::feedIdFromBin(EndianISStream& eiss) {
		
	
	
		
			
		feedId =  eiss.readInt();
			
		
	
	
}
void SysCalRow::numReceptorFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numReceptor =  eiss.readInt();
			
		
	
	
}
void SysCalRow::numChanFromBin(EndianISStream& eiss) {
		
	
	
		
			
		numChan =  eiss.readInt();
			
		
	
	
}

void SysCalRow::tcalFlagFromBin(EndianISStream& eiss) {
		
	tcalFlagExists = eiss.readBoolean();
	if (tcalFlagExists) {
		
	
	
		
			
		tcalFlag =  eiss.readBoolean();
			
		
	

	}
	
}
void SysCalRow::tcalSpectrumFromBin(EndianISStream& eiss) {
		
	tcalSpectrumExists = eiss.readBoolean();
	if (tcalSpectrumExists) {
		
	
		
		
			
	
	tcalSpectrum = Temperature::from2DBin(eiss);		
	

		
	

	}
	
}
void SysCalRow::trxFlagFromBin(EndianISStream& eiss) {
		
	trxFlagExists = eiss.readBoolean();
	if (trxFlagExists) {
		
	
	
		
			
		trxFlag =  eiss.readBoolean();
			
		
	

	}
	
}
void SysCalRow::trxSpectrumFromBin(EndianISStream& eiss) {
		
	trxSpectrumExists = eiss.readBoolean();
	if (trxSpectrumExists) {
		
	
		
		
			
	
	trxSpectrum = Temperature::from2DBin(eiss);		
	

		
	

	}
	
}
void SysCalRow::tskyFlagFromBin(EndianISStream& eiss) {
		
	tskyFlagExists = eiss.readBoolean();
	if (tskyFlagExists) {
		
	
	
		
			
		tskyFlag =  eiss.readBoolean();
			
		
	

	}
	
}
void SysCalRow::tskySpectrumFromBin(EndianISStream& eiss) {
		
	tskySpectrumExists = eiss.readBoolean();
	if (tskySpectrumExists) {
		
	
		
		
			
	
	tskySpectrum = Temperature::from2DBin(eiss);		
	

		
	

	}
	
}
void SysCalRow::tsysFlagFromBin(EndianISStream& eiss) {
		
	tsysFlagExists = eiss.readBoolean();
	if (tsysFlagExists) {
		
	
	
		
			
		tsysFlag =  eiss.readBoolean();
			
		
	

	}
	
}
void SysCalRow::tsysSpectrumFromBin(EndianISStream& eiss) {
		
	tsysSpectrumExists = eiss.readBoolean();
	if (tsysSpectrumExists) {
		
	
		
		
			
	
	tsysSpectrum = Temperature::from2DBin(eiss);		
	

		
	

	}
	
}
void SysCalRow::tantFlagFromBin(EndianISStream& eiss) {
		
	tantFlagExists = eiss.readBoolean();
	if (tantFlagExists) {
		
	
	
		
			
		tantFlag =  eiss.readBoolean();
			
		
	

	}
	
}
void SysCalRow::tantSpectrumFromBin(EndianISStream& eiss) {
		
	tantSpectrumExists = eiss.readBoolean();
	if (tantSpectrumExists) {
		
	
	
		
			
	
		tantSpectrum.clear();
		
		unsigned int tantSpectrumDim1 = eiss.readInt();
		unsigned int tantSpectrumDim2 = eiss.readInt();
		vector <float> tantSpectrumAux1;
		for (unsigned int i = 0; i < tantSpectrumDim1; i++) {
			tantSpectrumAux1.clear();
			for (unsigned int j = 0; j < tantSpectrumDim2 ; j++)			
			
			tantSpectrumAux1.push_back(eiss.readFloat());
			
			tantSpectrum.push_back(tantSpectrumAux1);
		}
	
	

		
	

	}
	
}
void SysCalRow::tantTsysFlagFromBin(EndianISStream& eiss) {
		
	tantTsysFlagExists = eiss.readBoolean();
	if (tantTsysFlagExists) {
		
	
	
		
			
		tantTsysFlag =  eiss.readBoolean();
			
		
	

	}
	
}
void SysCalRow::tantTsysSpectrumFromBin(EndianISStream& eiss) {
		
	tantTsysSpectrumExists = eiss.readBoolean();
	if (tantTsysSpectrumExists) {
		
	
	
		
			
	
		tantTsysSpectrum.clear();
		
		unsigned int tantTsysSpectrumDim1 = eiss.readInt();
		unsigned int tantTsysSpectrumDim2 = eiss.readInt();
		vector <float> tantTsysSpectrumAux1;
		for (unsigned int i = 0; i < tantTsysSpectrumDim1; i++) {
			tantTsysSpectrumAux1.clear();
			for (unsigned int j = 0; j < tantTsysSpectrumDim2 ; j++)			
			
			tantTsysSpectrumAux1.push_back(eiss.readFloat());
			
			tantTsysSpectrum.push_back(tantTsysSpectrumAux1);
		}
	
	

		
	

	}
	
}
void SysCalRow::phaseDiffFlagFromBin(EndianISStream& eiss) {
		
	phaseDiffFlagExists = eiss.readBoolean();
	if (phaseDiffFlagExists) {
		
	
	
		
			
		phaseDiffFlag =  eiss.readBoolean();
			
		
	

	}
	
}
void SysCalRow::phaseDiffSpectrumFromBin(EndianISStream& eiss) {
		
	phaseDiffSpectrumExists = eiss.readBoolean();
	if (phaseDiffSpectrumExists) {
		
	
	
		
			
	
		phaseDiffSpectrum.clear();
		
		unsigned int phaseDiffSpectrumDim1 = eiss.readInt();
		unsigned int phaseDiffSpectrumDim2 = eiss.readInt();
		vector <float> phaseDiffSpectrumAux1;
		for (unsigned int i = 0; i < phaseDiffSpectrumDim1; i++) {
			phaseDiffSpectrumAux1.clear();
			for (unsigned int j = 0; j < phaseDiffSpectrumDim2 ; j++)			
			
			phaseDiffSpectrumAux1.push_back(eiss.readFloat());
			
			phaseDiffSpectrum.push_back(phaseDiffSpectrumAux1);
		}
	
	

		
	

	}
	
}
	
	
	SysCalRow* SysCalRow::fromBin(EndianISStream& eiss, SysCalTable& table, const vector<string>& attributesSeq) {
		SysCalRow* row = new  SysCalRow(table);
		
		map<string, SysCalAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "SysCalTable");
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
 	ArrayTimeInterval SysCalRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SysCalRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "SysCal");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get numReceptor.
 	 * @return numReceptor as int
 	 */
 	int SysCalRow::getNumReceptor() const {
	
  		return numReceptor;
 	}

 	/**
 	 * Set numReceptor with the specified int.
 	 * @param numReceptor The int value to which numReceptor is to be set.
 	 
 	
 		
 	 */
 	void SysCalRow::setNumReceptor (int numReceptor)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numReceptor = numReceptor;
	
 	}
	
	

	

	
 	/**
 	 * Get numChan.
 	 * @return numChan as int
 	 */
 	int SysCalRow::getNumChan() const {
	
  		return numChan;
 	}

 	/**
 	 * Set numChan with the specified int.
 	 * @param numChan The int value to which numChan is to be set.
 	 
 	
 		
 	 */
 	void SysCalRow::setNumChan (int numChan)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->numChan = numChan;
	
 	}
	
	

	
	/**
	 * The attribute tcalFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tcalFlag attribute exists. 
	 */
	bool SysCalRow::isTcalFlagExists() const {
		return tcalFlagExists;
	}
	

	
 	/**
 	 * Get tcalFlag, which is optional.
 	 * @return tcalFlag as bool
 	 * @throw IllegalAccessException If tcalFlag does not exist.
 	 */
 	bool SysCalRow::getTcalFlag() const  {
		if (!tcalFlagExists) {
			throw IllegalAccessException("tcalFlag", "SysCal");
		}
	
  		return tcalFlag;
 	}

 	/**
 	 * Set tcalFlag with the specified bool.
 	 * @param tcalFlag The bool value to which tcalFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTcalFlag (bool tcalFlag) {
	
 		this->tcalFlag = tcalFlag;
	
		tcalFlagExists = true;
	
 	}
	
	
	/**
	 * Mark tcalFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTcalFlag () {
		tcalFlagExists = false;
	}
	

	
	/**
	 * The attribute tcalSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tcalSpectrum attribute exists. 
	 */
	bool SysCalRow::isTcalSpectrumExists() const {
		return tcalSpectrumExists;
	}
	

	
 	/**
 	 * Get tcalSpectrum, which is optional.
 	 * @return tcalSpectrum as vector<vector<Temperature > >
 	 * @throw IllegalAccessException If tcalSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > SysCalRow::getTcalSpectrum() const  {
		if (!tcalSpectrumExists) {
			throw IllegalAccessException("tcalSpectrum", "SysCal");
		}
	
  		return tcalSpectrum;
 	}

 	/**
 	 * Set tcalSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tcalSpectrum The vector<vector<Temperature > > value to which tcalSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTcalSpectrum (vector<vector<Temperature > > tcalSpectrum) {
	
 		this->tcalSpectrum = tcalSpectrum;
	
		tcalSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark tcalSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTcalSpectrum () {
		tcalSpectrumExists = false;
	}
	

	
	/**
	 * The attribute trxFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the trxFlag attribute exists. 
	 */
	bool SysCalRow::isTrxFlagExists() const {
		return trxFlagExists;
	}
	

	
 	/**
 	 * Get trxFlag, which is optional.
 	 * @return trxFlag as bool
 	 * @throw IllegalAccessException If trxFlag does not exist.
 	 */
 	bool SysCalRow::getTrxFlag() const  {
		if (!trxFlagExists) {
			throw IllegalAccessException("trxFlag", "SysCal");
		}
	
  		return trxFlag;
 	}

 	/**
 	 * Set trxFlag with the specified bool.
 	 * @param trxFlag The bool value to which trxFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTrxFlag (bool trxFlag) {
	
 		this->trxFlag = trxFlag;
	
		trxFlagExists = true;
	
 	}
	
	
	/**
	 * Mark trxFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTrxFlag () {
		trxFlagExists = false;
	}
	

	
	/**
	 * The attribute trxSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the trxSpectrum attribute exists. 
	 */
	bool SysCalRow::isTrxSpectrumExists() const {
		return trxSpectrumExists;
	}
	

	
 	/**
 	 * Get trxSpectrum, which is optional.
 	 * @return trxSpectrum as vector<vector<Temperature > >
 	 * @throw IllegalAccessException If trxSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > SysCalRow::getTrxSpectrum() const  {
		if (!trxSpectrumExists) {
			throw IllegalAccessException("trxSpectrum", "SysCal");
		}
	
  		return trxSpectrum;
 	}

 	/**
 	 * Set trxSpectrum with the specified vector<vector<Temperature > >.
 	 * @param trxSpectrum The vector<vector<Temperature > > value to which trxSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTrxSpectrum (vector<vector<Temperature > > trxSpectrum) {
	
 		this->trxSpectrum = trxSpectrum;
	
		trxSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark trxSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTrxSpectrum () {
		trxSpectrumExists = false;
	}
	

	
	/**
	 * The attribute tskyFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tskyFlag attribute exists. 
	 */
	bool SysCalRow::isTskyFlagExists() const {
		return tskyFlagExists;
	}
	

	
 	/**
 	 * Get tskyFlag, which is optional.
 	 * @return tskyFlag as bool
 	 * @throw IllegalAccessException If tskyFlag does not exist.
 	 */
 	bool SysCalRow::getTskyFlag() const  {
		if (!tskyFlagExists) {
			throw IllegalAccessException("tskyFlag", "SysCal");
		}
	
  		return tskyFlag;
 	}

 	/**
 	 * Set tskyFlag with the specified bool.
 	 * @param tskyFlag The bool value to which tskyFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTskyFlag (bool tskyFlag) {
	
 		this->tskyFlag = tskyFlag;
	
		tskyFlagExists = true;
	
 	}
	
	
	/**
	 * Mark tskyFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTskyFlag () {
		tskyFlagExists = false;
	}
	

	
	/**
	 * The attribute tskySpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tskySpectrum attribute exists. 
	 */
	bool SysCalRow::isTskySpectrumExists() const {
		return tskySpectrumExists;
	}
	

	
 	/**
 	 * Get tskySpectrum, which is optional.
 	 * @return tskySpectrum as vector<vector<Temperature > >
 	 * @throw IllegalAccessException If tskySpectrum does not exist.
 	 */
 	vector<vector<Temperature > > SysCalRow::getTskySpectrum() const  {
		if (!tskySpectrumExists) {
			throw IllegalAccessException("tskySpectrum", "SysCal");
		}
	
  		return tskySpectrum;
 	}

 	/**
 	 * Set tskySpectrum with the specified vector<vector<Temperature > >.
 	 * @param tskySpectrum The vector<vector<Temperature > > value to which tskySpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTskySpectrum (vector<vector<Temperature > > tskySpectrum) {
	
 		this->tskySpectrum = tskySpectrum;
	
		tskySpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark tskySpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTskySpectrum () {
		tskySpectrumExists = false;
	}
	

	
	/**
	 * The attribute tsysFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tsysFlag attribute exists. 
	 */
	bool SysCalRow::isTsysFlagExists() const {
		return tsysFlagExists;
	}
	

	
 	/**
 	 * Get tsysFlag, which is optional.
 	 * @return tsysFlag as bool
 	 * @throw IllegalAccessException If tsysFlag does not exist.
 	 */
 	bool SysCalRow::getTsysFlag() const  {
		if (!tsysFlagExists) {
			throw IllegalAccessException("tsysFlag", "SysCal");
		}
	
  		return tsysFlag;
 	}

 	/**
 	 * Set tsysFlag with the specified bool.
 	 * @param tsysFlag The bool value to which tsysFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTsysFlag (bool tsysFlag) {
	
 		this->tsysFlag = tsysFlag;
	
		tsysFlagExists = true;
	
 	}
	
	
	/**
	 * Mark tsysFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTsysFlag () {
		tsysFlagExists = false;
	}
	

	
	/**
	 * The attribute tsysSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tsysSpectrum attribute exists. 
	 */
	bool SysCalRow::isTsysSpectrumExists() const {
		return tsysSpectrumExists;
	}
	

	
 	/**
 	 * Get tsysSpectrum, which is optional.
 	 * @return tsysSpectrum as vector<vector<Temperature > >
 	 * @throw IllegalAccessException If tsysSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > SysCalRow::getTsysSpectrum() const  {
		if (!tsysSpectrumExists) {
			throw IllegalAccessException("tsysSpectrum", "SysCal");
		}
	
  		return tsysSpectrum;
 	}

 	/**
 	 * Set tsysSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tsysSpectrum The vector<vector<Temperature > > value to which tsysSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTsysSpectrum (vector<vector<Temperature > > tsysSpectrum) {
	
 		this->tsysSpectrum = tsysSpectrum;
	
		tsysSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark tsysSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTsysSpectrum () {
		tsysSpectrumExists = false;
	}
	

	
	/**
	 * The attribute tantFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tantFlag attribute exists. 
	 */
	bool SysCalRow::isTantFlagExists() const {
		return tantFlagExists;
	}
	

	
 	/**
 	 * Get tantFlag, which is optional.
 	 * @return tantFlag as bool
 	 * @throw IllegalAccessException If tantFlag does not exist.
 	 */
 	bool SysCalRow::getTantFlag() const  {
		if (!tantFlagExists) {
			throw IllegalAccessException("tantFlag", "SysCal");
		}
	
  		return tantFlag;
 	}

 	/**
 	 * Set tantFlag with the specified bool.
 	 * @param tantFlag The bool value to which tantFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTantFlag (bool tantFlag) {
	
 		this->tantFlag = tantFlag;
	
		tantFlagExists = true;
	
 	}
	
	
	/**
	 * Mark tantFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTantFlag () {
		tantFlagExists = false;
	}
	

	
	/**
	 * The attribute tantSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tantSpectrum attribute exists. 
	 */
	bool SysCalRow::isTantSpectrumExists() const {
		return tantSpectrumExists;
	}
	

	
 	/**
 	 * Get tantSpectrum, which is optional.
 	 * @return tantSpectrum as vector<vector<float > >
 	 * @throw IllegalAccessException If tantSpectrum does not exist.
 	 */
 	vector<vector<float > > SysCalRow::getTantSpectrum() const  {
		if (!tantSpectrumExists) {
			throw IllegalAccessException("tantSpectrum", "SysCal");
		}
	
  		return tantSpectrum;
 	}

 	/**
 	 * Set tantSpectrum with the specified vector<vector<float > >.
 	 * @param tantSpectrum The vector<vector<float > > value to which tantSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTantSpectrum (vector<vector<float > > tantSpectrum) {
	
 		this->tantSpectrum = tantSpectrum;
	
		tantSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark tantSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTantSpectrum () {
		tantSpectrumExists = false;
	}
	

	
	/**
	 * The attribute tantTsysFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tantTsysFlag attribute exists. 
	 */
	bool SysCalRow::isTantTsysFlagExists() const {
		return tantTsysFlagExists;
	}
	

	
 	/**
 	 * Get tantTsysFlag, which is optional.
 	 * @return tantTsysFlag as bool
 	 * @throw IllegalAccessException If tantTsysFlag does not exist.
 	 */
 	bool SysCalRow::getTantTsysFlag() const  {
		if (!tantTsysFlagExists) {
			throw IllegalAccessException("tantTsysFlag", "SysCal");
		}
	
  		return tantTsysFlag;
 	}

 	/**
 	 * Set tantTsysFlag with the specified bool.
 	 * @param tantTsysFlag The bool value to which tantTsysFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTantTsysFlag (bool tantTsysFlag) {
	
 		this->tantTsysFlag = tantTsysFlag;
	
		tantTsysFlagExists = true;
	
 	}
	
	
	/**
	 * Mark tantTsysFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTantTsysFlag () {
		tantTsysFlagExists = false;
	}
	

	
	/**
	 * The attribute tantTsysSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tantTsysSpectrum attribute exists. 
	 */
	bool SysCalRow::isTantTsysSpectrumExists() const {
		return tantTsysSpectrumExists;
	}
	

	
 	/**
 	 * Get tantTsysSpectrum, which is optional.
 	 * @return tantTsysSpectrum as vector<vector<float > >
 	 * @throw IllegalAccessException If tantTsysSpectrum does not exist.
 	 */
 	vector<vector<float > > SysCalRow::getTantTsysSpectrum() const  {
		if (!tantTsysSpectrumExists) {
			throw IllegalAccessException("tantTsysSpectrum", "SysCal");
		}
	
  		return tantTsysSpectrum;
 	}

 	/**
 	 * Set tantTsysSpectrum with the specified vector<vector<float > >.
 	 * @param tantTsysSpectrum The vector<vector<float > > value to which tantTsysSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTantTsysSpectrum (vector<vector<float > > tantTsysSpectrum) {
	
 		this->tantTsysSpectrum = tantTsysSpectrum;
	
		tantTsysSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark tantTsysSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTantTsysSpectrum () {
		tantTsysSpectrumExists = false;
	}
	

	
	/**
	 * The attribute phaseDiffFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseDiffFlag attribute exists. 
	 */
	bool SysCalRow::isPhaseDiffFlagExists() const {
		return phaseDiffFlagExists;
	}
	

	
 	/**
 	 * Get phaseDiffFlag, which is optional.
 	 * @return phaseDiffFlag as bool
 	 * @throw IllegalAccessException If phaseDiffFlag does not exist.
 	 */
 	bool SysCalRow::getPhaseDiffFlag() const  {
		if (!phaseDiffFlagExists) {
			throw IllegalAccessException("phaseDiffFlag", "SysCal");
		}
	
  		return phaseDiffFlag;
 	}

 	/**
 	 * Set phaseDiffFlag with the specified bool.
 	 * @param phaseDiffFlag The bool value to which phaseDiffFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setPhaseDiffFlag (bool phaseDiffFlag) {
	
 		this->phaseDiffFlag = phaseDiffFlag;
	
		phaseDiffFlagExists = true;
	
 	}
	
	
	/**
	 * Mark phaseDiffFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearPhaseDiffFlag () {
		phaseDiffFlagExists = false;
	}
	

	
	/**
	 * The attribute phaseDiffSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseDiffSpectrum attribute exists. 
	 */
	bool SysCalRow::isPhaseDiffSpectrumExists() const {
		return phaseDiffSpectrumExists;
	}
	

	
 	/**
 	 * Get phaseDiffSpectrum, which is optional.
 	 * @return phaseDiffSpectrum as vector<vector<float > >
 	 * @throw IllegalAccessException If phaseDiffSpectrum does not exist.
 	 */
 	vector<vector<float > > SysCalRow::getPhaseDiffSpectrum() const  {
		if (!phaseDiffSpectrumExists) {
			throw IllegalAccessException("phaseDiffSpectrum", "SysCal");
		}
	
  		return phaseDiffSpectrum;
 	}

 	/**
 	 * Set phaseDiffSpectrum with the specified vector<vector<float > >.
 	 * @param phaseDiffSpectrum The vector<vector<float > > value to which phaseDiffSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setPhaseDiffSpectrum (vector<vector<float > > phaseDiffSpectrum) {
	
 		this->phaseDiffSpectrum = phaseDiffSpectrum;
	
		phaseDiffSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark phaseDiffSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearPhaseDiffSpectrum () {
		phaseDiffSpectrumExists = false;
	}
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get antennaId.
 	 * @return antennaId as Tag
 	 */
 	Tag SysCalRow::getAntennaId() const {
	
  		return antennaId;
 	}

 	/**
 	 * Set antennaId with the specified Tag.
 	 * @param antennaId The Tag value to which antennaId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SysCalRow::setAntennaId (Tag antennaId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("antennaId", "SysCal");
		
  		}
  	
 		this->antennaId = antennaId;
	
 	}
	
	

	

	
 	/**
 	 * Get feedId.
 	 * @return feedId as int
 	 */
 	int SysCalRow::getFeedId() const {
	
  		return feedId;
 	}

 	/**
 	 * Set feedId with the specified int.
 	 * @param feedId The int value to which feedId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SysCalRow::setFeedId (int feedId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("feedId", "SysCal");
		
  		}
  	
 		this->feedId = feedId;
	
 	}
	
	

	

	
 	/**
 	 * Get spectralWindowId.
 	 * @return spectralWindowId as Tag
 	 */
 	Tag SysCalRow::getSpectralWindowId() const {
	
  		return spectralWindowId;
 	}

 	/**
 	 * Set spectralWindowId with the specified Tag.
 	 * @param spectralWindowId The Tag value to which spectralWindowId is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void SysCalRow::setSpectralWindowId (Tag spectralWindowId)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("spectralWindowId", "SysCal");
		
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
	 AntennaRow* SysCalRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of SysCal table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having their attribut feedId == this->feedId
	 */
	vector <FeedRow *> SysCalRow::getFeeds() {
		
			return table.getContainer().getFeed().getRowByFeedId(feedId);
		
	}
	

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* SysCalRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	/**
	 * Create a SysCalRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	SysCalRow::SysCalRow (SysCalTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	
		tcalFlagExists = false;
	

	
		tcalSpectrumExists = false;
	

	
		trxFlagExists = false;
	

	
		trxSpectrumExists = false;
	

	
		tskyFlagExists = false;
	

	
		tskySpectrumExists = false;
	

	
		tsysFlagExists = false;
	

	
		tsysSpectrumExists = false;
	

	
		tantFlagExists = false;
	

	
		tantSpectrumExists = false;
	

	
		tantTsysFlagExists = false;
	

	
		tantTsysSpectrumExists = false;
	

	
		phaseDiffFlagExists = false;
	

	
		phaseDiffSpectrumExists = false;
	

	
	

	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	 fromBinMethods["antennaId"] = &SysCalRow::antennaIdFromBin; 
	 fromBinMethods["spectralWindowId"] = &SysCalRow::spectralWindowIdFromBin; 
	 fromBinMethods["timeInterval"] = &SysCalRow::timeIntervalFromBin; 
	 fromBinMethods["feedId"] = &SysCalRow::feedIdFromBin; 
	 fromBinMethods["numReceptor"] = &SysCalRow::numReceptorFromBin; 
	 fromBinMethods["numChan"] = &SysCalRow::numChanFromBin; 
		
	
	 fromBinMethods["tcalFlag"] = &SysCalRow::tcalFlagFromBin; 
	 fromBinMethods["tcalSpectrum"] = &SysCalRow::tcalSpectrumFromBin; 
	 fromBinMethods["trxFlag"] = &SysCalRow::trxFlagFromBin; 
	 fromBinMethods["trxSpectrum"] = &SysCalRow::trxSpectrumFromBin; 
	 fromBinMethods["tskyFlag"] = &SysCalRow::tskyFlagFromBin; 
	 fromBinMethods["tskySpectrum"] = &SysCalRow::tskySpectrumFromBin; 
	 fromBinMethods["tsysFlag"] = &SysCalRow::tsysFlagFromBin; 
	 fromBinMethods["tsysSpectrum"] = &SysCalRow::tsysSpectrumFromBin; 
	 fromBinMethods["tantFlag"] = &SysCalRow::tantFlagFromBin; 
	 fromBinMethods["tantSpectrum"] = &SysCalRow::tantSpectrumFromBin; 
	 fromBinMethods["tantTsysFlag"] = &SysCalRow::tantTsysFlagFromBin; 
	 fromBinMethods["tantTsysSpectrum"] = &SysCalRow::tantTsysSpectrumFromBin; 
	 fromBinMethods["phaseDiffFlag"] = &SysCalRow::phaseDiffFlagFromBin; 
	 fromBinMethods["phaseDiffSpectrum"] = &SysCalRow::phaseDiffSpectrumFromBin; 
	
	}
	
	SysCalRow::SysCalRow (SysCalTable &t, SysCalRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	
		tcalFlagExists = false;
	

	
		tcalSpectrumExists = false;
	

	
		trxFlagExists = false;
	

	
		trxSpectrumExists = false;
	

	
		tskyFlagExists = false;
	

	
		tskySpectrumExists = false;
	

	
		tsysFlagExists = false;
	

	
		tsysSpectrumExists = false;
	

	
		tantFlagExists = false;
	

	
		tantSpectrumExists = false;
	

	
		tantTsysFlagExists = false;
	

	
		tantTsysSpectrumExists = false;
	

	
		phaseDiffFlagExists = false;
	

	
		phaseDiffSpectrumExists = false;
	

	
	

	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
			feedId = row.feedId;
		
		
		
		
			numReceptor = row.numReceptor;
		
			numChan = row.numChan;
		
		
		
		
		if (row.tcalFlagExists) {
			tcalFlag = row.tcalFlag;		
			tcalFlagExists = true;
		}
		else
			tcalFlagExists = false;
		
		if (row.tcalSpectrumExists) {
			tcalSpectrum = row.tcalSpectrum;		
			tcalSpectrumExists = true;
		}
		else
			tcalSpectrumExists = false;
		
		if (row.trxFlagExists) {
			trxFlag = row.trxFlag;		
			trxFlagExists = true;
		}
		else
			trxFlagExists = false;
		
		if (row.trxSpectrumExists) {
			trxSpectrum = row.trxSpectrum;		
			trxSpectrumExists = true;
		}
		else
			trxSpectrumExists = false;
		
		if (row.tskyFlagExists) {
			tskyFlag = row.tskyFlag;		
			tskyFlagExists = true;
		}
		else
			tskyFlagExists = false;
		
		if (row.tskySpectrumExists) {
			tskySpectrum = row.tskySpectrum;		
			tskySpectrumExists = true;
		}
		else
			tskySpectrumExists = false;
		
		if (row.tsysFlagExists) {
			tsysFlag = row.tsysFlag;		
			tsysFlagExists = true;
		}
		else
			tsysFlagExists = false;
		
		if (row.tsysSpectrumExists) {
			tsysSpectrum = row.tsysSpectrum;		
			tsysSpectrumExists = true;
		}
		else
			tsysSpectrumExists = false;
		
		if (row.tantFlagExists) {
			tantFlag = row.tantFlag;		
			tantFlagExists = true;
		}
		else
			tantFlagExists = false;
		
		if (row.tantSpectrumExists) {
			tantSpectrum = row.tantSpectrum;		
			tantSpectrumExists = true;
		}
		else
			tantSpectrumExists = false;
		
		if (row.tantTsysFlagExists) {
			tantTsysFlag = row.tantTsysFlag;		
			tantTsysFlagExists = true;
		}
		else
			tantTsysFlagExists = false;
		
		if (row.tantTsysSpectrumExists) {
			tantTsysSpectrum = row.tantTsysSpectrum;		
			tantTsysSpectrumExists = true;
		}
		else
			tantTsysSpectrumExists = false;
		
		if (row.phaseDiffFlagExists) {
			phaseDiffFlag = row.phaseDiffFlag;		
			phaseDiffFlagExists = true;
		}
		else
			phaseDiffFlagExists = false;
		
		if (row.phaseDiffSpectrumExists) {
			phaseDiffSpectrum = row.phaseDiffSpectrum;		
			phaseDiffSpectrumExists = true;
		}
		else
			phaseDiffSpectrumExists = false;
		
		}
		
		 fromBinMethods["antennaId"] = &SysCalRow::antennaIdFromBin; 
		 fromBinMethods["spectralWindowId"] = &SysCalRow::spectralWindowIdFromBin; 
		 fromBinMethods["timeInterval"] = &SysCalRow::timeIntervalFromBin; 
		 fromBinMethods["feedId"] = &SysCalRow::feedIdFromBin; 
		 fromBinMethods["numReceptor"] = &SysCalRow::numReceptorFromBin; 
		 fromBinMethods["numChan"] = &SysCalRow::numChanFromBin; 
			
	
		 fromBinMethods["tcalFlag"] = &SysCalRow::tcalFlagFromBin; 
		 fromBinMethods["tcalSpectrum"] = &SysCalRow::tcalSpectrumFromBin; 
		 fromBinMethods["trxFlag"] = &SysCalRow::trxFlagFromBin; 
		 fromBinMethods["trxSpectrum"] = &SysCalRow::trxSpectrumFromBin; 
		 fromBinMethods["tskyFlag"] = &SysCalRow::tskyFlagFromBin; 
		 fromBinMethods["tskySpectrum"] = &SysCalRow::tskySpectrumFromBin; 
		 fromBinMethods["tsysFlag"] = &SysCalRow::tsysFlagFromBin; 
		 fromBinMethods["tsysSpectrum"] = &SysCalRow::tsysSpectrumFromBin; 
		 fromBinMethods["tantFlag"] = &SysCalRow::tantFlagFromBin; 
		 fromBinMethods["tantSpectrum"] = &SysCalRow::tantSpectrumFromBin; 
		 fromBinMethods["tantTsysFlag"] = &SysCalRow::tantTsysFlagFromBin; 
		 fromBinMethods["tantTsysSpectrum"] = &SysCalRow::tantTsysSpectrumFromBin; 
		 fromBinMethods["phaseDiffFlag"] = &SysCalRow::phaseDiffFlagFromBin; 
		 fromBinMethods["phaseDiffSpectrum"] = &SysCalRow::phaseDiffSpectrumFromBin; 
			
	}

	
	bool SysCalRow::compareNoAutoInc(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId, int numReceptor, int numChan) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->feedId == feedId);
		
		if (!result) return false;
	

	
		
		result = result && (this->numReceptor == numReceptor);
		
		if (!result) return false;
	

	
		
		result = result && (this->numChan == numChan);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool SysCalRow::compareRequiredValue(int numReceptor, int numChan) {
		bool result;
		result = true;
		
	
		if (!(this->numReceptor == numReceptor)) return false;
	

	
		if (!(this->numChan == numChan)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the SysCalRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool SysCalRow::equalByRequiredValue(SysCalRow* x) {
		
			
		if (this->numReceptor != x->numReceptor) return false;
			
		if (this->numChan != x->numChan) return false;
			
		
		return true;
	}	
	
/*
	 map<string, SysCalAttributeFromBin> SysCalRow::initFromBinMethods() {
		map<string, SysCalAttributeFromBin> result;
		
		result["antennaId"] = &SysCalRow::antennaIdFromBin;
		result["spectralWindowId"] = &SysCalRow::spectralWindowIdFromBin;
		result["timeInterval"] = &SysCalRow::timeIntervalFromBin;
		result["feedId"] = &SysCalRow::feedIdFromBin;
		result["numReceptor"] = &SysCalRow::numReceptorFromBin;
		result["numChan"] = &SysCalRow::numChanFromBin;
		
		
		result["tcalFlag"] = &SysCalRow::tcalFlagFromBin;
		result["tcalSpectrum"] = &SysCalRow::tcalSpectrumFromBin;
		result["trxFlag"] = &SysCalRow::trxFlagFromBin;
		result["trxSpectrum"] = &SysCalRow::trxSpectrumFromBin;
		result["tskyFlag"] = &SysCalRow::tskyFlagFromBin;
		result["tskySpectrum"] = &SysCalRow::tskySpectrumFromBin;
		result["tsysFlag"] = &SysCalRow::tsysFlagFromBin;
		result["tsysSpectrum"] = &SysCalRow::tsysSpectrumFromBin;
		result["tantFlag"] = &SysCalRow::tantFlagFromBin;
		result["tantSpectrum"] = &SysCalRow::tantSpectrumFromBin;
		result["tantTsysFlag"] = &SysCalRow::tantTsysFlagFromBin;
		result["tantTsysSpectrum"] = &SysCalRow::tantTsysSpectrumFromBin;
		result["phaseDiffFlag"] = &SysCalRow::phaseDiffFlagFromBin;
		result["phaseDiffSpectrum"] = &SysCalRow::phaseDiffSpectrumFromBin;
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
