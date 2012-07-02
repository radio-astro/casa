
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

#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>

#include <FeedTable.h>
#include <FeedRow.h>

#include <AntennaTable.h>
#include <AntennaRow.h>
	

using asdm::ASDM;
using asdm::SysCalRow;
using asdm::SysCalTable;

using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;

using asdm::FeedTable;
using asdm::FeedRow;

using asdm::AntennaTable;
using asdm::AntennaRow;


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
			
		
	

	
  		
		
		x->numLoadExists = numLoadExists;
		
		
			
				
		x->numLoad = numLoad;
 				
 			
		
	

	
  		
		
		x->calLoadExists = calLoadExists;
		
		
			
		x->calLoad.length(calLoad.size());
		for (unsigned int i = 0; i < calLoad.size(); ++i) {
			
				
			x->calLoad[i] = calLoad.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->feffExists = feffExists;
		
		
			
		x->feff.length(feff.size());
		for (unsigned int i = 0; i < feff.size(); ++i) {
			
				
			x->feff[i] = feff.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->aeffExists = aeffExists;
		
		
			
		x->aeff.length(aeff.size());
		for (unsigned int i = 0; i < aeff.size(); ++i) {
			
				
			x->aeff[i] = aeff.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->phaseDiffExists = phaseDiffExists;
		
		
			
		x->phaseDiff = phaseDiff.toIDLAngle();
			
		
	

	
  		
		
		x->sbgainExists = sbgainExists;
		
		
			
				
		x->sbgain = sbgain;
 				
 			
		
	

	
  		
		
		x->tauExists = tauExists;
		
		
			
		x->tau.length(tau.size());
		for (unsigned int i = 0; i < tau.size(); ++i) {
			
			x->tau[i] = tau.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->tcalExists = tcalExists;
		
		
			
		x->tcal.length(tcal.size());
		for (unsigned int i = 0; i < tcal.size(); ++i) {
			
			x->tcal[i] = tcal.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->trxExists = trxExists;
		
		
			
		x->trx.length(trx.size());
		for (unsigned int i = 0; i < trx.size(); ++i) {
			
			x->trx[i] = trx.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->tskyExists = tskyExists;
		
		
			
		x->tsky.length(tsky.size());
		for (unsigned int i = 0; i < tsky.size(); ++i) {
			
			x->tsky[i] = tsky.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->tsysExists = tsysExists;
		
		
			
		x->tsys.length(tsys.size());
		for (unsigned int i = 0; i < tsys.size(); ++i) {
			
			x->tsys[i] = tsys.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->tantExists = tantExists;
		
		
			
		x->tant.length(tant.size());
		for (unsigned int i = 0; i < tant.size(); ++i) {
			
			x->tant[i] = tant.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->tantTsysExists = tantTsysExists;
		
		
			
		x->tantTsys.length(tantTsys.size());
		for (unsigned int i = 0; i < tantTsys.size(); ++i) {
			
				
			x->tantTsys[i] = tantTsys.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->pwvPathExists = pwvPathExists;
		
		
			
		x->pwvPath.length(pwvPath.size());
		for (unsigned int i = 0; i < pwvPath.size(); ++i) {
			
			x->pwvPath[i] = pwvPath.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		x->dpwvPathExists = dpwvPathExists;
		
		
			
		x->dpwvPath.length(dpwvPath.size());
		for (unsigned int i = 0; i < dpwvPath.size(); ++i) {
			
				
			x->dpwvPath[i] = dpwvPath.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->feffSpectrumExists = feffSpectrumExists;
		
		
			
		x->feffSpectrum.length(feffSpectrum.size());
		for (unsigned int i = 0; i < feffSpectrum.size(); i++) {
			x->feffSpectrum[i].length(feffSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < feffSpectrum.size() ; i++)
			for (unsigned int j = 0; j < feffSpectrum.at(i).size(); j++)
					
						
				x->feffSpectrum[i][j] = feffSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->sbgainSpectrumExists = sbgainSpectrumExists;
		
		
			
		x->sbgainSpectrum.length(sbgainSpectrum.size());
		for (unsigned int i = 0; i < sbgainSpectrum.size(); i++) {
			x->sbgainSpectrum[i].length(sbgainSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < sbgainSpectrum.size() ; i++)
			for (unsigned int j = 0; j < sbgainSpectrum.at(i).size(); j++)
					
						
				x->sbgainSpectrum[i][j] = sbgainSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->tauSpectrumExists = tauSpectrumExists;
		
		
			
		x->tauSpectrum.length(tauSpectrum.size());
		for (unsigned int i = 0; i < tauSpectrum.size(); i++) {
			x->tauSpectrum[i].length(tauSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tauSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tauSpectrum.at(i).size(); j++)
					
				x->tauSpectrum[i][j]= tauSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->tcalSpectrumExists = tcalSpectrumExists;
		
		
			
		x->tcalSpectrum.length(tcalSpectrum.size());
		for (unsigned int i = 0; i < tcalSpectrum.size(); i++) {
			x->tcalSpectrum[i].length(tcalSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tcalSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tcalSpectrum.at(i).size(); j++)
					
				x->tcalSpectrum[i][j]= tcalSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->trxSpectrumExists = trxSpectrumExists;
		
		
			
		x->trxSpectrum.length(trxSpectrum.size());
		for (unsigned int i = 0; i < trxSpectrum.size(); ++i) {
			
			x->trxSpectrum[i] = trxSpectrum.at(i).toIDLTemperature();
			
	 	}
			
		
	

	
  		
		
		x->tskySpectrumExists = tskySpectrumExists;
		
		
			
		x->tskySpectrum.length(tskySpectrum.size());
		for (unsigned int i = 0; i < tskySpectrum.size(); i++) {
			x->tskySpectrum[i].length(tskySpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tskySpectrum.size() ; i++)
			for (unsigned int j = 0; j < tskySpectrum.at(i).size(); j++)
					
				x->tskySpectrum[i][j]= tskySpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->tsysSpectrumExists = tsysSpectrumExists;
		
		
			
		x->tsysSpectrum.length(tsysSpectrum.size());
		for (unsigned int i = 0; i < tsysSpectrum.size(); i++) {
			x->tsysSpectrum[i].length(tsysSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tsysSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tsysSpectrum.at(i).size(); j++)
					
				x->tsysSpectrum[i][j]= tsysSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->tantSpectrumExists = tantSpectrumExists;
		
		
			
		x->tantSpectrum.length(tantSpectrum.size());
		for (unsigned int i = 0; i < tantSpectrum.size(); i++) {
			x->tantSpectrum[i].length(tantSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tantSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tantSpectrum.at(i).size(); j++)
					
				x->tantSpectrum[i][j]= tantSpectrum.at(i).at(j).toIDLTemperature();
									
		
			
		
	

	
  		
		
		x->tantTsysSpectrumExists = tantTsysSpectrumExists;
		
		
			
		x->tantTsysSpectrum.length(tantTsysSpectrum.size());
		for (unsigned int i = 0; i < tantTsysSpectrum.size(); i++) {
			x->tantTsysSpectrum[i].length(tantTsysSpectrum.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < tantTsysSpectrum.size() ; i++)
			for (unsigned int j = 0; j < tantTsysSpectrum.at(i).size(); j++)
					
						
				x->tantTsysSpectrum[i][j] = tantTsysSpectrum.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->pwvPathSpectrumExists = pwvPathSpectrumExists;
		
		
			
		x->pwvPathSpectrum.length(pwvPathSpectrum.size());
		for (unsigned int i = 0; i < pwvPathSpectrum.size(); ++i) {
			
			x->pwvPathSpectrum[i] = pwvPathSpectrum.at(i).toIDLLength();
			
	 	}
			
		
	

	
  		
		
		x->dpwvPathSpectrumExists = dpwvPathSpectrumExists;
		
		
			
		x->dpwvPathSpectrum.length(dpwvPathSpectrum.size());
		for (unsigned int i = 0; i < dpwvPathSpectrum.size(); ++i) {
			
				
			x->dpwvPathSpectrum[i] = dpwvPathSpectrum.at(i);
	 			
	 		
	 	}
			
		
	

	
  		
		
		x->numPolyExists = numPolyExists;
		
		
			
				
		x->numPoly = numPoly;
 				
 			
		
	

	
  		
		
		x->numPolyFreqExists = numPolyFreqExists;
		
		
			
				
		x->numPolyFreq = numPolyFreq;
 				
 			
		
	

	
  		
		
		x->timeOriginExists = timeOriginExists;
		
		
			
		x->timeOrigin = timeOrigin.toIDLArrayTime();
			
		
	

	
  		
		
		x->freqOriginExists = freqOriginExists;
		
		
			
		x->freqOrigin = freqOrigin.toIDLFrequency();
			
		
	

	
  		
		
		x->phaseCurveExists = phaseCurveExists;
		
		
			
		x->phaseCurve.length(phaseCurve.size());
		for (unsigned int i = 0; i < phaseCurve.size(); i++) {
			x->phaseCurve[i].length(phaseCurve.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < phaseCurve.size() ; i++)
			for (unsigned int j = 0; j < phaseCurve.at(i).size(); j++)
					
				x->phaseCurve[i][j]= phaseCurve.at(i).at(j).toIDLAngle();
									
		
			
		
	

	
  		
		
		x->delayCurveExists = delayCurveExists;
		
		
			
		x->delayCurve.length(delayCurve.size());
		for (unsigned int i = 0; i < delayCurve.size(); i++) {
			x->delayCurve[i].length(delayCurve.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < delayCurve.size() ; i++)
			for (unsigned int j = 0; j < delayCurve.at(i).size(); j++)
					
				x->delayCurve[i][j]= delayCurve.at(i).at(j).toIDLInterval();
									
		
			
		
	

	
  		
		
		x->ampliCurveExists = ampliCurveExists;
		
		
			
		x->ampliCurve.length(ampliCurve.size());
		for (unsigned int i = 0; i < ampliCurve.size(); i++) {
			x->ampliCurve[i].length(ampliCurve.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < ampliCurve.size() ; i++)
			for (unsigned int j = 0; j < ampliCurve.at(i).size(); j++)
					
						
				x->ampliCurve[i][j] = ampliCurve.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->bandpassCurveExists = bandpassCurveExists;
		
		
			
		x->bandpassCurve.length(bandpassCurve.size());
		for (unsigned int i = 0; i < bandpassCurve.size(); i++) {
			x->bandpassCurve[i].length(bandpassCurve.at(i).size());			 		
		}
		
		for (unsigned int i = 0; i < bandpassCurve.size() ; i++)
			for (unsigned int j = 0; j < bandpassCurve.at(i).size(); j++)
					
						
				x->bandpassCurve[i][j] = bandpassCurve.at(i).at(j);
		 				
			 						
		
			
		
	

	
  		
		
		x->phasediffFlagExists = phasediffFlagExists;
		
		
			
				
		x->phasediffFlag = phasediffFlag;
 				
 			
		
	

	
  		
		
		x->sbgainFlagExists = sbgainFlagExists;
		
		
			
				
		x->sbgainFlag = sbgainFlag;
 				
 			
		
	

	
  		
		
		x->tauFlagExists = tauFlagExists;
		
		
			
				
		x->tauFlag = tauFlag;
 				
 			
		
	

	
  		
		
		x->tcalFlagExists = tcalFlagExists;
		
		
			
				
		x->tcalFlag = tcalFlag;
 				
 			
		
	

	
  		
		
		x->trxFlagExists = trxFlagExists;
		
		
			
				
		x->trxFlag = trxFlag;
 				
 			
		
	

	
  		
		
		x->tskyFlagExists = tskyFlagExists;
		
		
			
				
		x->tskyFlag = tskyFlag;
 				
 			
		
	

	
  		
		
		x->tsysFlagExists = tsysFlagExists;
		
		
			
				
		x->tsysFlag = tsysFlag;
 				
 			
		
	

	
  		
		
		x->tantFlagExists = tantFlagExists;
		
		
			
				
		x->tantFlag = tantFlag;
 				
 			
		
	

	
  		
		
		x->tantTsysFlagExists = tantTsysFlagExists;
		
		
			
				
		x->tantTsysFlag = tantTsysFlag;
 				
 			
		
	

	
  		
		
		x->pwvPathFlagExists = pwvPathFlagExists;
		
		
			
				
		x->pwvPathFlag = pwvPathFlag;
 				
 			
		
	

	
	
		
	
  	
 		
		
	 	
			
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
	void SysCalRow::setFromIDL (SysCalRowIDL x) throw(ConversionException) {
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		numLoadExists = x.numLoadExists;
		if (x.numLoadExists) {
		
		
			
		setNumLoad(x.numLoad);
  			
 		
		
		}
		
	

	
		
		calLoadExists = x.calLoadExists;
		if (x.calLoadExists) {
		
		
			
		calLoad .clear();
		for (unsigned int i = 0; i <x.calLoad.length(); ++i) {
			
			calLoad.push_back(x.calLoad[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		feffExists = x.feffExists;
		if (x.feffExists) {
		
		
			
		feff .clear();
		for (unsigned int i = 0; i <x.feff.length(); ++i) {
			
			feff.push_back(x.feff[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		aeffExists = x.aeffExists;
		if (x.aeffExists) {
		
		
			
		aeff .clear();
		for (unsigned int i = 0; i <x.aeff.length(); ++i) {
			
			aeff.push_back(x.aeff[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		phaseDiffExists = x.phaseDiffExists;
		if (x.phaseDiffExists) {
		
		
			
		setPhaseDiff(Angle (x.phaseDiff));
			
 		
		
		}
		
	

	
		
		sbgainExists = x.sbgainExists;
		if (x.sbgainExists) {
		
		
			
		setSbgain(x.sbgain);
  			
 		
		
		}
		
	

	
		
		tauExists = x.tauExists;
		if (x.tauExists) {
		
		
			
		tau .clear();
		for (unsigned int i = 0; i <x.tau.length(); ++i) {
			
			tau.push_back(Temperature (x.tau[i]));
			
		}
			
  		
		
		}
		
	

	
		
		tcalExists = x.tcalExists;
		if (x.tcalExists) {
		
		
			
		tcal .clear();
		for (unsigned int i = 0; i <x.tcal.length(); ++i) {
			
			tcal.push_back(Temperature (x.tcal[i]));
			
		}
			
  		
		
		}
		
	

	
		
		trxExists = x.trxExists;
		if (x.trxExists) {
		
		
			
		trx .clear();
		for (unsigned int i = 0; i <x.trx.length(); ++i) {
			
			trx.push_back(Temperature (x.trx[i]));
			
		}
			
  		
		
		}
		
	

	
		
		tskyExists = x.tskyExists;
		if (x.tskyExists) {
		
		
			
		tsky .clear();
		for (unsigned int i = 0; i <x.tsky.length(); ++i) {
			
			tsky.push_back(Temperature (x.tsky[i]));
			
		}
			
  		
		
		}
		
	

	
		
		tsysExists = x.tsysExists;
		if (x.tsysExists) {
		
		
			
		tsys .clear();
		for (unsigned int i = 0; i <x.tsys.length(); ++i) {
			
			tsys.push_back(Temperature (x.tsys[i]));
			
		}
			
  		
		
		}
		
	

	
		
		tantExists = x.tantExists;
		if (x.tantExists) {
		
		
			
		tant .clear();
		for (unsigned int i = 0; i <x.tant.length(); ++i) {
			
			tant.push_back(Temperature (x.tant[i]));
			
		}
			
  		
		
		}
		
	

	
		
		tantTsysExists = x.tantTsysExists;
		if (x.tantTsysExists) {
		
		
			
		tantTsys .clear();
		for (unsigned int i = 0; i <x.tantTsys.length(); ++i) {
			
			tantTsys.push_back(x.tantTsys[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		pwvPathExists = x.pwvPathExists;
		if (x.pwvPathExists) {
		
		
			
		pwvPath .clear();
		for (unsigned int i = 0; i <x.pwvPath.length(); ++i) {
			
			pwvPath.push_back(Length (x.pwvPath[i]));
			
		}
			
  		
		
		}
		
	

	
		
		dpwvPathExists = x.dpwvPathExists;
		if (x.dpwvPathExists) {
		
		
			
		dpwvPath .clear();
		for (unsigned int i = 0; i <x.dpwvPath.length(); ++i) {
			
			dpwvPath.push_back(x.dpwvPath[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		feffSpectrumExists = x.feffSpectrumExists;
		if (x.feffSpectrumExists) {
		
		
			
		feffSpectrum .clear();
		vector<float> v_aux_feffSpectrum;
		for (unsigned int i = 0; i < x.feffSpectrum.length(); ++i) {
			v_aux_feffSpectrum.clear();
			for (unsigned int j = 0; j < x.feffSpectrum[0].length(); ++j) {
				
				v_aux_feffSpectrum.push_back(x.feffSpectrum[i][j]);
	  			
  			}
  			feffSpectrum.push_back(v_aux_feffSpectrum);			
		}
			
  		
		
		}
		
	

	
		
		sbgainSpectrumExists = x.sbgainSpectrumExists;
		if (x.sbgainSpectrumExists) {
		
		
			
		sbgainSpectrum .clear();
		vector<float> v_aux_sbgainSpectrum;
		for (unsigned int i = 0; i < x.sbgainSpectrum.length(); ++i) {
			v_aux_sbgainSpectrum.clear();
			for (unsigned int j = 0; j < x.sbgainSpectrum[0].length(); ++j) {
				
				v_aux_sbgainSpectrum.push_back(x.sbgainSpectrum[i][j]);
	  			
  			}
  			sbgainSpectrum.push_back(v_aux_sbgainSpectrum);			
		}
			
  		
		
		}
		
	

	
		
		tauSpectrumExists = x.tauSpectrumExists;
		if (x.tauSpectrumExists) {
		
		
			
		tauSpectrum .clear();
		vector<Temperature> v_aux_tauSpectrum;
		for (unsigned int i = 0; i < x.tauSpectrum.length(); ++i) {
			v_aux_tauSpectrum.clear();
			for (unsigned int j = 0; j < x.tauSpectrum[0].length(); ++j) {
				
				v_aux_tauSpectrum.push_back(Temperature (x.tauSpectrum[i][j]));
				
  			}
  			tauSpectrum.push_back(v_aux_tauSpectrum);			
		}
			
  		
		
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
		
	

	
		
		trxSpectrumExists = x.trxSpectrumExists;
		if (x.trxSpectrumExists) {
		
		
			
		trxSpectrum .clear();
		for (unsigned int i = 0; i <x.trxSpectrum.length(); ++i) {
			
			trxSpectrum.push_back(Temperature (x.trxSpectrum[i]));
			
		}
			
  		
		
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
		
	

	
		
		tantSpectrumExists = x.tantSpectrumExists;
		if (x.tantSpectrumExists) {
		
		
			
		tantSpectrum .clear();
		vector<Temperature> v_aux_tantSpectrum;
		for (unsigned int i = 0; i < x.tantSpectrum.length(); ++i) {
			v_aux_tantSpectrum.clear();
			for (unsigned int j = 0; j < x.tantSpectrum[0].length(); ++j) {
				
				v_aux_tantSpectrum.push_back(Temperature (x.tantSpectrum[i][j]));
				
  			}
  			tantSpectrum.push_back(v_aux_tantSpectrum);			
		}
			
  		
		
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
		
	

	
		
		pwvPathSpectrumExists = x.pwvPathSpectrumExists;
		if (x.pwvPathSpectrumExists) {
		
		
			
		pwvPathSpectrum .clear();
		for (unsigned int i = 0; i <x.pwvPathSpectrum.length(); ++i) {
			
			pwvPathSpectrum.push_back(Length (x.pwvPathSpectrum[i]));
			
		}
			
  		
		
		}
		
	

	
		
		dpwvPathSpectrumExists = x.dpwvPathSpectrumExists;
		if (x.dpwvPathSpectrumExists) {
		
		
			
		dpwvPathSpectrum .clear();
		for (unsigned int i = 0; i <x.dpwvPathSpectrum.length(); ++i) {
			
			dpwvPathSpectrum.push_back(x.dpwvPathSpectrum[i]);
  			
		}
			
  		
		
		}
		
	

	
		
		numPolyExists = x.numPolyExists;
		if (x.numPolyExists) {
		
		
			
		setNumPoly(x.numPoly);
  			
 		
		
		}
		
	

	
		
		numPolyFreqExists = x.numPolyFreqExists;
		if (x.numPolyFreqExists) {
		
		
			
		setNumPolyFreq(x.numPolyFreq);
  			
 		
		
		}
		
	

	
		
		timeOriginExists = x.timeOriginExists;
		if (x.timeOriginExists) {
		
		
			
		setTimeOrigin(ArrayTime (x.timeOrigin));
			
 		
		
		}
		
	

	
		
		freqOriginExists = x.freqOriginExists;
		if (x.freqOriginExists) {
		
		
			
		setFreqOrigin(Frequency (x.freqOrigin));
			
 		
		
		}
		
	

	
		
		phaseCurveExists = x.phaseCurveExists;
		if (x.phaseCurveExists) {
		
		
			
		phaseCurve .clear();
		vector<Angle> v_aux_phaseCurve;
		for (unsigned int i = 0; i < x.phaseCurve.length(); ++i) {
			v_aux_phaseCurve.clear();
			for (unsigned int j = 0; j < x.phaseCurve[0].length(); ++j) {
				
				v_aux_phaseCurve.push_back(Angle (x.phaseCurve[i][j]));
				
  			}
  			phaseCurve.push_back(v_aux_phaseCurve);			
		}
			
  		
		
		}
		
	

	
		
		delayCurveExists = x.delayCurveExists;
		if (x.delayCurveExists) {
		
		
			
		delayCurve .clear();
		vector<Interval> v_aux_delayCurve;
		for (unsigned int i = 0; i < x.delayCurve.length(); ++i) {
			v_aux_delayCurve.clear();
			for (unsigned int j = 0; j < x.delayCurve[0].length(); ++j) {
				
				v_aux_delayCurve.push_back(Interval (x.delayCurve[i][j]));
				
  			}
  			delayCurve.push_back(v_aux_delayCurve);			
		}
			
  		
		
		}
		
	

	
		
		ampliCurveExists = x.ampliCurveExists;
		if (x.ampliCurveExists) {
		
		
			
		ampliCurve .clear();
		vector<float> v_aux_ampliCurve;
		for (unsigned int i = 0; i < x.ampliCurve.length(); ++i) {
			v_aux_ampliCurve.clear();
			for (unsigned int j = 0; j < x.ampliCurve[0].length(); ++j) {
				
				v_aux_ampliCurve.push_back(x.ampliCurve[i][j]);
	  			
  			}
  			ampliCurve.push_back(v_aux_ampliCurve);			
		}
			
  		
		
		}
		
	

	
		
		bandpassCurveExists = x.bandpassCurveExists;
		if (x.bandpassCurveExists) {
		
		
			
		bandpassCurve .clear();
		vector<float> v_aux_bandpassCurve;
		for (unsigned int i = 0; i < x.bandpassCurve.length(); ++i) {
			v_aux_bandpassCurve.clear();
			for (unsigned int j = 0; j < x.bandpassCurve[0].length(); ++j) {
				
				v_aux_bandpassCurve.push_back(x.bandpassCurve[i][j]);
	  			
  			}
  			bandpassCurve.push_back(v_aux_bandpassCurve);			
		}
			
  		
		
		}
		
	

	
		
		phasediffFlagExists = x.phasediffFlagExists;
		if (x.phasediffFlagExists) {
		
		
			
		setPhasediffFlag(x.phasediffFlag);
  			
 		
		
		}
		
	

	
		
		sbgainFlagExists = x.sbgainFlagExists;
		if (x.sbgainFlagExists) {
		
		
			
		setSbgainFlag(x.sbgainFlag);
  			
 		
		
		}
		
	

	
		
		tauFlagExists = x.tauFlagExists;
		if (x.tauFlagExists) {
		
		
			
		setTauFlag(x.tauFlag);
  			
 		
		
		}
		
	

	
		
		tcalFlagExists = x.tcalFlagExists;
		if (x.tcalFlagExists) {
		
		
			
		setTcalFlag(x.tcalFlag);
  			
 		
		
		}
		
	

	
		
		trxFlagExists = x.trxFlagExists;
		if (x.trxFlagExists) {
		
		
			
		setTrxFlag(x.trxFlag);
  			
 		
		
		}
		
	

	
		
		tskyFlagExists = x.tskyFlagExists;
		if (x.tskyFlagExists) {
		
		
			
		setTskyFlag(x.tskyFlag);
  			
 		
		
		}
		
	

	
		
		tsysFlagExists = x.tsysFlagExists;
		if (x.tsysFlagExists) {
		
		
			
		setTsysFlag(x.tsysFlag);
  			
 		
		
		}
		
	

	
		
		tantFlagExists = x.tantFlagExists;
		if (x.tantFlagExists) {
		
		
			
		setTantFlag(x.tantFlag);
  			
 		
		
		}
		
	

	
		
		tantTsysFlagExists = x.tantTsysFlagExists;
		if (x.tantTsysFlagExists) {
		
		
			
		setTantTsysFlag(x.tantTsysFlag);
  			
 		
		
		}
		
	

	
		
		pwvPathFlagExists = x.pwvPathFlagExists;
		if (x.pwvPathFlagExists) {
		
		
			
		setPwvPathFlag(x.pwvPathFlag);
  			
 		
		
		}
		
	

	
	
		
	
		
		
			
		setAntennaId(Tag (x.antennaId));
			
 		
		
	

	
		
		
			
		setFeedId(x.feedId);
  			
 		
		
	

	
		
		
			
		setSpectralWindowId(Tag (x.spectralWindowId));
			
 		
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw new ConversionException (err.getMessage(),"SysCal");
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
		
		
	

  	
 		
		if (numLoadExists) {
		
		
		Parser::toXML(numLoad, "numLoad", buf);
		
		
		}
		
	

  	
 		
		if (calLoadExists) {
		
		
		Parser::toXML(calLoad, "calLoad", buf);
		
		
		}
		
	

  	
 		
		if (feffExists) {
		
		
		Parser::toXML(feff, "feff", buf);
		
		
		}
		
	

  	
 		
		if (aeffExists) {
		
		
		Parser::toXML(aeff, "aeff", buf);
		
		
		}
		
	

  	
 		
		if (phaseDiffExists) {
		
		
		Parser::toXML(phaseDiff, "phaseDiff", buf);
		
		
		}
		
	

  	
 		
		if (sbgainExists) {
		
		
		Parser::toXML(sbgain, "sbgain", buf);
		
		
		}
		
	

  	
 		
		if (tauExists) {
		
		
		Parser::toXML(tau, "tau", buf);
		
		
		}
		
	

  	
 		
		if (tcalExists) {
		
		
		Parser::toXML(tcal, "tcal", buf);
		
		
		}
		
	

  	
 		
		if (trxExists) {
		
		
		Parser::toXML(trx, "trx", buf);
		
		
		}
		
	

  	
 		
		if (tskyExists) {
		
		
		Parser::toXML(tsky, "tsky", buf);
		
		
		}
		
	

  	
 		
		if (tsysExists) {
		
		
		Parser::toXML(tsys, "tsys", buf);
		
		
		}
		
	

  	
 		
		if (tantExists) {
		
		
		Parser::toXML(tant, "tant", buf);
		
		
		}
		
	

  	
 		
		if (tantTsysExists) {
		
		
		Parser::toXML(tantTsys, "tantTsys", buf);
		
		
		}
		
	

  	
 		
		if (pwvPathExists) {
		
		
		Parser::toXML(pwvPath, "pwvPath", buf);
		
		
		}
		
	

  	
 		
		if (dpwvPathExists) {
		
		
		Parser::toXML(dpwvPath, "dpwvPath", buf);
		
		
		}
		
	

  	
 		
		if (feffSpectrumExists) {
		
		
		Parser::toXML(feffSpectrum, "feffSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (sbgainSpectrumExists) {
		
		
		Parser::toXML(sbgainSpectrum, "sbgainSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tauSpectrumExists) {
		
		
		Parser::toXML(tauSpectrum, "tauSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tcalSpectrumExists) {
		
		
		Parser::toXML(tcalSpectrum, "tcalSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (trxSpectrumExists) {
		
		
		Parser::toXML(trxSpectrum, "trxSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tskySpectrumExists) {
		
		
		Parser::toXML(tskySpectrum, "tskySpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tsysSpectrumExists) {
		
		
		Parser::toXML(tsysSpectrum, "tsysSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tantSpectrumExists) {
		
		
		Parser::toXML(tantSpectrum, "tantSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (tantTsysSpectrumExists) {
		
		
		Parser::toXML(tantTsysSpectrum, "tantTsysSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (pwvPathSpectrumExists) {
		
		
		Parser::toXML(pwvPathSpectrum, "pwvPathSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (dpwvPathSpectrumExists) {
		
		
		Parser::toXML(dpwvPathSpectrum, "dpwvPathSpectrum", buf);
		
		
		}
		
	

  	
 		
		if (numPolyExists) {
		
		
		Parser::toXML(numPoly, "numPoly", buf);
		
		
		}
		
	

  	
 		
		if (numPolyFreqExists) {
		
		
		Parser::toXML(numPolyFreq, "numPolyFreq", buf);
		
		
		}
		
	

  	
 		
		if (timeOriginExists) {
		
		
		Parser::toXML(timeOrigin, "timeOrigin", buf);
		
		
		}
		
	

  	
 		
		if (freqOriginExists) {
		
		
		Parser::toXML(freqOrigin, "freqOrigin", buf);
		
		
		}
		
	

  	
 		
		if (phaseCurveExists) {
		
		
		Parser::toXML(phaseCurve, "phaseCurve", buf);
		
		
		}
		
	

  	
 		
		if (delayCurveExists) {
		
		
		Parser::toXML(delayCurve, "delayCurve", buf);
		
		
		}
		
	

  	
 		
		if (ampliCurveExists) {
		
		
		Parser::toXML(ampliCurve, "ampliCurve", buf);
		
		
		}
		
	

  	
 		
		if (bandpassCurveExists) {
		
		
		Parser::toXML(bandpassCurve, "bandpassCurve", buf);
		
		
		}
		
	

  	
 		
		if (phasediffFlagExists) {
		
		
		Parser::toXML(phasediffFlag, "phasediffFlag", buf);
		
		
		}
		
	

  	
 		
		if (sbgainFlagExists) {
		
		
		Parser::toXML(sbgainFlag, "sbgainFlag", buf);
		
		
		}
		
	

  	
 		
		if (tauFlagExists) {
		
		
		Parser::toXML(tauFlag, "tauFlag", buf);
		
		
		}
		
	

  	
 		
		if (tcalFlagExists) {
		
		
		Parser::toXML(tcalFlag, "tcalFlag", buf);
		
		
		}
		
	

  	
 		
		if (trxFlagExists) {
		
		
		Parser::toXML(trxFlag, "trxFlag", buf);
		
		
		}
		
	

  	
 		
		if (tskyFlagExists) {
		
		
		Parser::toXML(tskyFlag, "tskyFlag", buf);
		
		
		}
		
	

  	
 		
		if (tsysFlagExists) {
		
		
		Parser::toXML(tsysFlag, "tsysFlag", buf);
		
		
		}
		
	

  	
 		
		if (tantFlagExists) {
		
		
		Parser::toXML(tantFlag, "tantFlag", buf);
		
		
		}
		
	

  	
 		
		if (tantTsysFlagExists) {
		
		
		Parser::toXML(tantTsysFlag, "tantTsysFlag", buf);
		
		
		}
		
	

  	
 		
		if (pwvPathFlagExists) {
		
		
		Parser::toXML(pwvPathFlag, "pwvPathFlag", buf);
		
		
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
	void SysCalRow::setFromXML (string rowDoc) throw(ConversionException) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","SysCal",rowDoc));
			
		
	

	
  		
        if (row.isStr("<numLoad>")) {
			
	  		setNumLoad(Parser::getInteger("numLoad","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<calLoad>")) {
			
								
	  		setCalLoad(Parser::get1DInteger("calLoad","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<feff>")) {
			
								
	  		setFeff(Parser::get1DFloat("feff","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<aeff>")) {
			
								
	  		setAeff(Parser::get1DFloat("aeff","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<phaseDiff>")) {
			
	  		setPhaseDiff(Parser::getAngle("phaseDiff","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<sbgain>")) {
			
	  		setSbgain(Parser::getFloat("sbgain","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tau>")) {
			
								
	  		setTau(Parser::get1DTemperature("tau","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tcal>")) {
			
								
	  		setTcal(Parser::get1DTemperature("tcal","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<trx>")) {
			
								
	  		setTrx(Parser::get1DTemperature("trx","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tsky>")) {
			
								
	  		setTsky(Parser::get1DTemperature("tsky","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tsys>")) {
			
								
	  		setTsys(Parser::get1DTemperature("tsys","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tant>")) {
			
								
	  		setTant(Parser::get1DTemperature("tant","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tantTsys>")) {
			
								
	  		setTantTsys(Parser::get1DFloat("tantTsys","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<pwvPath>")) {
			
								
	  		setPwvPath(Parser::get1DLength("pwvPath","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<dpwvPath>")) {
			
								
	  		setDpwvPath(Parser::get1DFloat("dpwvPath","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<feffSpectrum>")) {
			
								
	  		setFeffSpectrum(Parser::get2DFloat("feffSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<sbgainSpectrum>")) {
			
								
	  		setSbgainSpectrum(Parser::get2DFloat("sbgainSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tauSpectrum>")) {
			
								
	  		setTauSpectrum(Parser::get2DTemperature("tauSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tcalSpectrum>")) {
			
								
	  		setTcalSpectrum(Parser::get2DTemperature("tcalSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<trxSpectrum>")) {
			
								
	  		setTrxSpectrum(Parser::get1DTemperature("trxSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tskySpectrum>")) {
			
								
	  		setTskySpectrum(Parser::get2DTemperature("tskySpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tsysSpectrum>")) {
			
								
	  		setTsysSpectrum(Parser::get2DTemperature("tsysSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tantSpectrum>")) {
			
								
	  		setTantSpectrum(Parser::get2DTemperature("tantSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<tantTsysSpectrum>")) {
			
								
	  		setTantTsysSpectrum(Parser::get2DFloat("tantTsysSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<pwvPathSpectrum>")) {
			
								
	  		setPwvPathSpectrum(Parser::get1DLength("pwvPathSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<dpwvPathSpectrum>")) {
			
								
	  		setDpwvPathSpectrum(Parser::get1DFloat("dpwvPathSpectrum","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<numPoly>")) {
			
	  		setNumPoly(Parser::getInteger("numPoly","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<numPolyFreq>")) {
			
	  		setNumPolyFreq(Parser::getInteger("numPolyFreq","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<timeOrigin>")) {
			
	  		setTimeOrigin(Parser::getArrayTime("timeOrigin","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<freqOrigin>")) {
			
	  		setFreqOrigin(Parser::getFrequency("freqOrigin","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<phaseCurve>")) {
			
								
	  		setPhaseCurve(Parser::get2DAngle("phaseCurve","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<delayCurve>")) {
			
								
	  		setDelayCurve(Parser::get2DInterval("delayCurve","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<ampliCurve>")) {
			
								
	  		setAmpliCurve(Parser::get2DFloat("ampliCurve","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<bandpassCurve>")) {
			
								
	  		setBandpassCurve(Parser::get2DFloat("bandpassCurve","SysCal",rowDoc));
	  			
	  		
		}
 		
	

	
  		
        if (row.isStr("<phasediffFlag>")) {
			
	  		setPhasediffFlag(Parser::getBoolean("phasediffFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<sbgainFlag>")) {
			
	  		setSbgainFlag(Parser::getBoolean("sbgainFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tauFlag>")) {
			
	  		setTauFlag(Parser::getBoolean("tauFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tcalFlag>")) {
			
	  		setTcalFlag(Parser::getBoolean("tcalFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<trxFlag>")) {
			
	  		setTrxFlag(Parser::getBoolean("trxFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tskyFlag>")) {
			
	  		setTskyFlag(Parser::getBoolean("tskyFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tsysFlag>")) {
			
	  		setTsysFlag(Parser::getBoolean("tsysFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tantFlag>")) {
			
	  		setTantFlag(Parser::getBoolean("tantFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<tantTsysFlag>")) {
			
	  		setTantTsysFlag(Parser::getBoolean("tantTsysFlag","SysCal",rowDoc));
			
		}
 		
	

	
  		
        if (row.isStr("<pwvPathFlag>")) {
			
	  		setPwvPathFlag(Parser::getBoolean("pwvPathFlag","SysCal",rowDoc));
			
		}
 		
	

	
	
		
	
  		
			
	  	setAntennaId(Parser::getTag("antennaId","Antenna",rowDoc));
			
		
	

	
  		
			
	  	setFeedId(Parser::getInteger("feedId","Feed",rowDoc));
			
		
	

	
  		
			
	  	setSpectralWindowId(Parser::getTag("spectralWindowId","SpectralWindow",rowDoc));
			
		
	

	
		
	

	

	

		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"SysCal");
		}
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
	 * The attribute numLoad is optional. Return true if this attribute exists.
	 * @return true if and only if the numLoad attribute exists. 
	 */
	bool SysCalRow::isNumLoadExists() const {
		return numLoadExists;
	}
	

	
 	/**
 	 * Get numLoad, which is optional.
 	 * @return numLoad as int
 	 * @throw IllegalAccessException If numLoad does not exist.
 	 */
 	int SysCalRow::getNumLoad() const throw(IllegalAccessException) {
		if (!numLoadExists) {
			throw IllegalAccessException("numLoad", "SysCal");
		}
	
  		return numLoad;
 	}

 	/**
 	 * Set numLoad with the specified int.
 	 * @param numLoad The int value to which numLoad is to be set.
 	 
 	
 	 */
 	void SysCalRow::setNumLoad (int numLoad) {
	
 		this->numLoad = numLoad;
	
		numLoadExists = true;
	
 	}
	
	
	/**
	 * Mark numLoad, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearNumLoad () {
		numLoadExists = false;
	}
	

	
	/**
	 * The attribute calLoad is optional. Return true if this attribute exists.
	 * @return true if and only if the calLoad attribute exists. 
	 */
	bool SysCalRow::isCalLoadExists() const {
		return calLoadExists;
	}
	

	
 	/**
 	 * Get calLoad, which is optional.
 	 * @return calLoad as vector<int >
 	 * @throw IllegalAccessException If calLoad does not exist.
 	 */
 	vector<int > SysCalRow::getCalLoad() const throw(IllegalAccessException) {
		if (!calLoadExists) {
			throw IllegalAccessException("calLoad", "SysCal");
		}
	
  		return calLoad;
 	}

 	/**
 	 * Set calLoad with the specified vector<int >.
 	 * @param calLoad The vector<int > value to which calLoad is to be set.
 	 
 	
 	 */
 	void SysCalRow::setCalLoad (vector<int > calLoad) {
	
 		this->calLoad = calLoad;
	
		calLoadExists = true;
	
 	}
	
	
	/**
	 * Mark calLoad, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearCalLoad () {
		calLoadExists = false;
	}
	

	
	/**
	 * The attribute feff is optional. Return true if this attribute exists.
	 * @return true if and only if the feff attribute exists. 
	 */
	bool SysCalRow::isFeffExists() const {
		return feffExists;
	}
	

	
 	/**
 	 * Get feff, which is optional.
 	 * @return feff as vector<float >
 	 * @throw IllegalAccessException If feff does not exist.
 	 */
 	vector<float > SysCalRow::getFeff() const throw(IllegalAccessException) {
		if (!feffExists) {
			throw IllegalAccessException("feff", "SysCal");
		}
	
  		return feff;
 	}

 	/**
 	 * Set feff with the specified vector<float >.
 	 * @param feff The vector<float > value to which feff is to be set.
 	 
 	
 	 */
 	void SysCalRow::setFeff (vector<float > feff) {
	
 		this->feff = feff;
	
		feffExists = true;
	
 	}
	
	
	/**
	 * Mark feff, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearFeff () {
		feffExists = false;
	}
	

	
	/**
	 * The attribute aeff is optional. Return true if this attribute exists.
	 * @return true if and only if the aeff attribute exists. 
	 */
	bool SysCalRow::isAeffExists() const {
		return aeffExists;
	}
	

	
 	/**
 	 * Get aeff, which is optional.
 	 * @return aeff as vector<float >
 	 * @throw IllegalAccessException If aeff does not exist.
 	 */
 	vector<float > SysCalRow::getAeff() const throw(IllegalAccessException) {
		if (!aeffExists) {
			throw IllegalAccessException("aeff", "SysCal");
		}
	
  		return aeff;
 	}

 	/**
 	 * Set aeff with the specified vector<float >.
 	 * @param aeff The vector<float > value to which aeff is to be set.
 	 
 	
 	 */
 	void SysCalRow::setAeff (vector<float > aeff) {
	
 		this->aeff = aeff;
	
		aeffExists = true;
	
 	}
	
	
	/**
	 * Mark aeff, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearAeff () {
		aeffExists = false;
	}
	

	
	/**
	 * The attribute phaseDiff is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseDiff attribute exists. 
	 */
	bool SysCalRow::isPhaseDiffExists() const {
		return phaseDiffExists;
	}
	

	
 	/**
 	 * Get phaseDiff, which is optional.
 	 * @return phaseDiff as Angle
 	 * @throw IllegalAccessException If phaseDiff does not exist.
 	 */
 	Angle SysCalRow::getPhaseDiff() const throw(IllegalAccessException) {
		if (!phaseDiffExists) {
			throw IllegalAccessException("phaseDiff", "SysCal");
		}
	
  		return phaseDiff;
 	}

 	/**
 	 * Set phaseDiff with the specified Angle.
 	 * @param phaseDiff The Angle value to which phaseDiff is to be set.
 	 
 	
 	 */
 	void SysCalRow::setPhaseDiff (Angle phaseDiff) {
	
 		this->phaseDiff = phaseDiff;
	
		phaseDiffExists = true;
	
 	}
	
	
	/**
	 * Mark phaseDiff, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearPhaseDiff () {
		phaseDiffExists = false;
	}
	

	
	/**
	 * The attribute sbgain is optional. Return true if this attribute exists.
	 * @return true if and only if the sbgain attribute exists. 
	 */
	bool SysCalRow::isSbgainExists() const {
		return sbgainExists;
	}
	

	
 	/**
 	 * Get sbgain, which is optional.
 	 * @return sbgain as float
 	 * @throw IllegalAccessException If sbgain does not exist.
 	 */
 	float SysCalRow::getSbgain() const throw(IllegalAccessException) {
		if (!sbgainExists) {
			throw IllegalAccessException("sbgain", "SysCal");
		}
	
  		return sbgain;
 	}

 	/**
 	 * Set sbgain with the specified float.
 	 * @param sbgain The float value to which sbgain is to be set.
 	 
 	
 	 */
 	void SysCalRow::setSbgain (float sbgain) {
	
 		this->sbgain = sbgain;
	
		sbgainExists = true;
	
 	}
	
	
	/**
	 * Mark sbgain, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearSbgain () {
		sbgainExists = false;
	}
	

	
	/**
	 * The attribute tau is optional. Return true if this attribute exists.
	 * @return true if and only if the tau attribute exists. 
	 */
	bool SysCalRow::isTauExists() const {
		return tauExists;
	}
	

	
 	/**
 	 * Get tau, which is optional.
 	 * @return tau as vector<Temperature >
 	 * @throw IllegalAccessException If tau does not exist.
 	 */
 	vector<Temperature > SysCalRow::getTau() const throw(IllegalAccessException) {
		if (!tauExists) {
			throw IllegalAccessException("tau", "SysCal");
		}
	
  		return tau;
 	}

 	/**
 	 * Set tau with the specified vector<Temperature >.
 	 * @param tau The vector<Temperature > value to which tau is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTau (vector<Temperature > tau) {
	
 		this->tau = tau;
	
		tauExists = true;
	
 	}
	
	
	/**
	 * Mark tau, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTau () {
		tauExists = false;
	}
	

	
	/**
	 * The attribute tcal is optional. Return true if this attribute exists.
	 * @return true if and only if the tcal attribute exists. 
	 */
	bool SysCalRow::isTcalExists() const {
		return tcalExists;
	}
	

	
 	/**
 	 * Get tcal, which is optional.
 	 * @return tcal as vector<Temperature >
 	 * @throw IllegalAccessException If tcal does not exist.
 	 */
 	vector<Temperature > SysCalRow::getTcal() const throw(IllegalAccessException) {
		if (!tcalExists) {
			throw IllegalAccessException("tcal", "SysCal");
		}
	
  		return tcal;
 	}

 	/**
 	 * Set tcal with the specified vector<Temperature >.
 	 * @param tcal The vector<Temperature > value to which tcal is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTcal (vector<Temperature > tcal) {
	
 		this->tcal = tcal;
	
		tcalExists = true;
	
 	}
	
	
	/**
	 * Mark tcal, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTcal () {
		tcalExists = false;
	}
	

	
	/**
	 * The attribute trx is optional. Return true if this attribute exists.
	 * @return true if and only if the trx attribute exists. 
	 */
	bool SysCalRow::isTrxExists() const {
		return trxExists;
	}
	

	
 	/**
 	 * Get trx, which is optional.
 	 * @return trx as vector<Temperature >
 	 * @throw IllegalAccessException If trx does not exist.
 	 */
 	vector<Temperature > SysCalRow::getTrx() const throw(IllegalAccessException) {
		if (!trxExists) {
			throw IllegalAccessException("trx", "SysCal");
		}
	
  		return trx;
 	}

 	/**
 	 * Set trx with the specified vector<Temperature >.
 	 * @param trx The vector<Temperature > value to which trx is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTrx (vector<Temperature > trx) {
	
 		this->trx = trx;
	
		trxExists = true;
	
 	}
	
	
	/**
	 * Mark trx, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTrx () {
		trxExists = false;
	}
	

	
	/**
	 * The attribute tsky is optional. Return true if this attribute exists.
	 * @return true if and only if the tsky attribute exists. 
	 */
	bool SysCalRow::isTskyExists() const {
		return tskyExists;
	}
	

	
 	/**
 	 * Get tsky, which is optional.
 	 * @return tsky as vector<Temperature >
 	 * @throw IllegalAccessException If tsky does not exist.
 	 */
 	vector<Temperature > SysCalRow::getTsky() const throw(IllegalAccessException) {
		if (!tskyExists) {
			throw IllegalAccessException("tsky", "SysCal");
		}
	
  		return tsky;
 	}

 	/**
 	 * Set tsky with the specified vector<Temperature >.
 	 * @param tsky The vector<Temperature > value to which tsky is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTsky (vector<Temperature > tsky) {
	
 		this->tsky = tsky;
	
		tskyExists = true;
	
 	}
	
	
	/**
	 * Mark tsky, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTsky () {
		tskyExists = false;
	}
	

	
	/**
	 * The attribute tsys is optional. Return true if this attribute exists.
	 * @return true if and only if the tsys attribute exists. 
	 */
	bool SysCalRow::isTsysExists() const {
		return tsysExists;
	}
	

	
 	/**
 	 * Get tsys, which is optional.
 	 * @return tsys as vector<Temperature >
 	 * @throw IllegalAccessException If tsys does not exist.
 	 */
 	vector<Temperature > SysCalRow::getTsys() const throw(IllegalAccessException) {
		if (!tsysExists) {
			throw IllegalAccessException("tsys", "SysCal");
		}
	
  		return tsys;
 	}

 	/**
 	 * Set tsys with the specified vector<Temperature >.
 	 * @param tsys The vector<Temperature > value to which tsys is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTsys (vector<Temperature > tsys) {
	
 		this->tsys = tsys;
	
		tsysExists = true;
	
 	}
	
	
	/**
	 * Mark tsys, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTsys () {
		tsysExists = false;
	}
	

	
	/**
	 * The attribute tant is optional. Return true if this attribute exists.
	 * @return true if and only if the tant attribute exists. 
	 */
	bool SysCalRow::isTantExists() const {
		return tantExists;
	}
	

	
 	/**
 	 * Get tant, which is optional.
 	 * @return tant as vector<Temperature >
 	 * @throw IllegalAccessException If tant does not exist.
 	 */
 	vector<Temperature > SysCalRow::getTant() const throw(IllegalAccessException) {
		if (!tantExists) {
			throw IllegalAccessException("tant", "SysCal");
		}
	
  		return tant;
 	}

 	/**
 	 * Set tant with the specified vector<Temperature >.
 	 * @param tant The vector<Temperature > value to which tant is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTant (vector<Temperature > tant) {
	
 		this->tant = tant;
	
		tantExists = true;
	
 	}
	
	
	/**
	 * Mark tant, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTant () {
		tantExists = false;
	}
	

	
	/**
	 * The attribute tantTsys is optional. Return true if this attribute exists.
	 * @return true if and only if the tantTsys attribute exists. 
	 */
	bool SysCalRow::isTantTsysExists() const {
		return tantTsysExists;
	}
	

	
 	/**
 	 * Get tantTsys, which is optional.
 	 * @return tantTsys as vector<float >
 	 * @throw IllegalAccessException If tantTsys does not exist.
 	 */
 	vector<float > SysCalRow::getTantTsys() const throw(IllegalAccessException) {
		if (!tantTsysExists) {
			throw IllegalAccessException("tantTsys", "SysCal");
		}
	
  		return tantTsys;
 	}

 	/**
 	 * Set tantTsys with the specified vector<float >.
 	 * @param tantTsys The vector<float > value to which tantTsys is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTantTsys (vector<float > tantTsys) {
	
 		this->tantTsys = tantTsys;
	
		tantTsysExists = true;
	
 	}
	
	
	/**
	 * Mark tantTsys, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTantTsys () {
		tantTsysExists = false;
	}
	

	
	/**
	 * The attribute pwvPath is optional. Return true if this attribute exists.
	 * @return true if and only if the pwvPath attribute exists. 
	 */
	bool SysCalRow::isPwvPathExists() const {
		return pwvPathExists;
	}
	

	
 	/**
 	 * Get pwvPath, which is optional.
 	 * @return pwvPath as vector<Length >
 	 * @throw IllegalAccessException If pwvPath does not exist.
 	 */
 	vector<Length > SysCalRow::getPwvPath() const throw(IllegalAccessException) {
		if (!pwvPathExists) {
			throw IllegalAccessException("pwvPath", "SysCal");
		}
	
  		return pwvPath;
 	}

 	/**
 	 * Set pwvPath with the specified vector<Length >.
 	 * @param pwvPath The vector<Length > value to which pwvPath is to be set.
 	 
 	
 	 */
 	void SysCalRow::setPwvPath (vector<Length > pwvPath) {
	
 		this->pwvPath = pwvPath;
	
		pwvPathExists = true;
	
 	}
	
	
	/**
	 * Mark pwvPath, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearPwvPath () {
		pwvPathExists = false;
	}
	

	
	/**
	 * The attribute dpwvPath is optional. Return true if this attribute exists.
	 * @return true if and only if the dpwvPath attribute exists. 
	 */
	bool SysCalRow::isDpwvPathExists() const {
		return dpwvPathExists;
	}
	

	
 	/**
 	 * Get dpwvPath, which is optional.
 	 * @return dpwvPath as vector<float >
 	 * @throw IllegalAccessException If dpwvPath does not exist.
 	 */
 	vector<float > SysCalRow::getDpwvPath() const throw(IllegalAccessException) {
		if (!dpwvPathExists) {
			throw IllegalAccessException("dpwvPath", "SysCal");
		}
	
  		return dpwvPath;
 	}

 	/**
 	 * Set dpwvPath with the specified vector<float >.
 	 * @param dpwvPath The vector<float > value to which dpwvPath is to be set.
 	 
 	
 	 */
 	void SysCalRow::setDpwvPath (vector<float > dpwvPath) {
	
 		this->dpwvPath = dpwvPath;
	
		dpwvPathExists = true;
	
 	}
	
	
	/**
	 * Mark dpwvPath, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearDpwvPath () {
		dpwvPathExists = false;
	}
	

	
	/**
	 * The attribute feffSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the feffSpectrum attribute exists. 
	 */
	bool SysCalRow::isFeffSpectrumExists() const {
		return feffSpectrumExists;
	}
	

	
 	/**
 	 * Get feffSpectrum, which is optional.
 	 * @return feffSpectrum as vector<vector<float > >
 	 * @throw IllegalAccessException If feffSpectrum does not exist.
 	 */
 	vector<vector<float > > SysCalRow::getFeffSpectrum() const throw(IllegalAccessException) {
		if (!feffSpectrumExists) {
			throw IllegalAccessException("feffSpectrum", "SysCal");
		}
	
  		return feffSpectrum;
 	}

 	/**
 	 * Set feffSpectrum with the specified vector<vector<float > >.
 	 * @param feffSpectrum The vector<vector<float > > value to which feffSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setFeffSpectrum (vector<vector<float > > feffSpectrum) {
	
 		this->feffSpectrum = feffSpectrum;
	
		feffSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark feffSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearFeffSpectrum () {
		feffSpectrumExists = false;
	}
	

	
	/**
	 * The attribute sbgainSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the sbgainSpectrum attribute exists. 
	 */
	bool SysCalRow::isSbgainSpectrumExists() const {
		return sbgainSpectrumExists;
	}
	

	
 	/**
 	 * Get sbgainSpectrum, which is optional.
 	 * @return sbgainSpectrum as vector<vector<float > >
 	 * @throw IllegalAccessException If sbgainSpectrum does not exist.
 	 */
 	vector<vector<float > > SysCalRow::getSbgainSpectrum() const throw(IllegalAccessException) {
		if (!sbgainSpectrumExists) {
			throw IllegalAccessException("sbgainSpectrum", "SysCal");
		}
	
  		return sbgainSpectrum;
 	}

 	/**
 	 * Set sbgainSpectrum with the specified vector<vector<float > >.
 	 * @param sbgainSpectrum The vector<vector<float > > value to which sbgainSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setSbgainSpectrum (vector<vector<float > > sbgainSpectrum) {
	
 		this->sbgainSpectrum = sbgainSpectrum;
	
		sbgainSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark sbgainSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearSbgainSpectrum () {
		sbgainSpectrumExists = false;
	}
	

	
	/**
	 * The attribute tauSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tauSpectrum attribute exists. 
	 */
	bool SysCalRow::isTauSpectrumExists() const {
		return tauSpectrumExists;
	}
	

	
 	/**
 	 * Get tauSpectrum, which is optional.
 	 * @return tauSpectrum as vector<vector<Temperature > >
 	 * @throw IllegalAccessException If tauSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > SysCalRow::getTauSpectrum() const throw(IllegalAccessException) {
		if (!tauSpectrumExists) {
			throw IllegalAccessException("tauSpectrum", "SysCal");
		}
	
  		return tauSpectrum;
 	}

 	/**
 	 * Set tauSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tauSpectrum The vector<vector<Temperature > > value to which tauSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTauSpectrum (vector<vector<Temperature > > tauSpectrum) {
	
 		this->tauSpectrum = tauSpectrum;
	
		tauSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark tauSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTauSpectrum () {
		tauSpectrumExists = false;
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
 	vector<vector<Temperature > > SysCalRow::getTcalSpectrum() const throw(IllegalAccessException) {
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
	 * The attribute trxSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the trxSpectrum attribute exists. 
	 */
	bool SysCalRow::isTrxSpectrumExists() const {
		return trxSpectrumExists;
	}
	

	
 	/**
 	 * Get trxSpectrum, which is optional.
 	 * @return trxSpectrum as vector<Temperature >
 	 * @throw IllegalAccessException If trxSpectrum does not exist.
 	 */
 	vector<Temperature > SysCalRow::getTrxSpectrum() const throw(IllegalAccessException) {
		if (!trxSpectrumExists) {
			throw IllegalAccessException("trxSpectrum", "SysCal");
		}
	
  		return trxSpectrum;
 	}

 	/**
 	 * Set trxSpectrum with the specified vector<Temperature >.
 	 * @param trxSpectrum The vector<Temperature > value to which trxSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTrxSpectrum (vector<Temperature > trxSpectrum) {
	
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
 	vector<vector<Temperature > > SysCalRow::getTskySpectrum() const throw(IllegalAccessException) {
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
 	vector<vector<Temperature > > SysCalRow::getTsysSpectrum() const throw(IllegalAccessException) {
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
	 * The attribute tantSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the tantSpectrum attribute exists. 
	 */
	bool SysCalRow::isTantSpectrumExists() const {
		return tantSpectrumExists;
	}
	

	
 	/**
 	 * Get tantSpectrum, which is optional.
 	 * @return tantSpectrum as vector<vector<Temperature > >
 	 * @throw IllegalAccessException If tantSpectrum does not exist.
 	 */
 	vector<vector<Temperature > > SysCalRow::getTantSpectrum() const throw(IllegalAccessException) {
		if (!tantSpectrumExists) {
			throw IllegalAccessException("tantSpectrum", "SysCal");
		}
	
  		return tantSpectrum;
 	}

 	/**
 	 * Set tantSpectrum with the specified vector<vector<Temperature > >.
 	 * @param tantSpectrum The vector<vector<Temperature > > value to which tantSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTantSpectrum (vector<vector<Temperature > > tantSpectrum) {
	
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
 	vector<vector<float > > SysCalRow::getTantTsysSpectrum() const throw(IllegalAccessException) {
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
	 * The attribute pwvPathSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the pwvPathSpectrum attribute exists. 
	 */
	bool SysCalRow::isPwvPathSpectrumExists() const {
		return pwvPathSpectrumExists;
	}
	

	
 	/**
 	 * Get pwvPathSpectrum, which is optional.
 	 * @return pwvPathSpectrum as vector<Length >
 	 * @throw IllegalAccessException If pwvPathSpectrum does not exist.
 	 */
 	vector<Length > SysCalRow::getPwvPathSpectrum() const throw(IllegalAccessException) {
		if (!pwvPathSpectrumExists) {
			throw IllegalAccessException("pwvPathSpectrum", "SysCal");
		}
	
  		return pwvPathSpectrum;
 	}

 	/**
 	 * Set pwvPathSpectrum with the specified vector<Length >.
 	 * @param pwvPathSpectrum The vector<Length > value to which pwvPathSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setPwvPathSpectrum (vector<Length > pwvPathSpectrum) {
	
 		this->pwvPathSpectrum = pwvPathSpectrum;
	
		pwvPathSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark pwvPathSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearPwvPathSpectrum () {
		pwvPathSpectrumExists = false;
	}
	

	
	/**
	 * The attribute dpwvPathSpectrum is optional. Return true if this attribute exists.
	 * @return true if and only if the dpwvPathSpectrum attribute exists. 
	 */
	bool SysCalRow::isDpwvPathSpectrumExists() const {
		return dpwvPathSpectrumExists;
	}
	

	
 	/**
 	 * Get dpwvPathSpectrum, which is optional.
 	 * @return dpwvPathSpectrum as vector<float >
 	 * @throw IllegalAccessException If dpwvPathSpectrum does not exist.
 	 */
 	vector<float > SysCalRow::getDpwvPathSpectrum() const throw(IllegalAccessException) {
		if (!dpwvPathSpectrumExists) {
			throw IllegalAccessException("dpwvPathSpectrum", "SysCal");
		}
	
  		return dpwvPathSpectrum;
 	}

 	/**
 	 * Set dpwvPathSpectrum with the specified vector<float >.
 	 * @param dpwvPathSpectrum The vector<float > value to which dpwvPathSpectrum is to be set.
 	 
 	
 	 */
 	void SysCalRow::setDpwvPathSpectrum (vector<float > dpwvPathSpectrum) {
	
 		this->dpwvPathSpectrum = dpwvPathSpectrum;
	
		dpwvPathSpectrumExists = true;
	
 	}
	
	
	/**
	 * Mark dpwvPathSpectrum, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearDpwvPathSpectrum () {
		dpwvPathSpectrumExists = false;
	}
	

	
	/**
	 * The attribute numPoly is optional. Return true if this attribute exists.
	 * @return true if and only if the numPoly attribute exists. 
	 */
	bool SysCalRow::isNumPolyExists() const {
		return numPolyExists;
	}
	

	
 	/**
 	 * Get numPoly, which is optional.
 	 * @return numPoly as int
 	 * @throw IllegalAccessException If numPoly does not exist.
 	 */
 	int SysCalRow::getNumPoly() const throw(IllegalAccessException) {
		if (!numPolyExists) {
			throw IllegalAccessException("numPoly", "SysCal");
		}
	
  		return numPoly;
 	}

 	/**
 	 * Set numPoly with the specified int.
 	 * @param numPoly The int value to which numPoly is to be set.
 	 
 	
 	 */
 	void SysCalRow::setNumPoly (int numPoly) {
	
 		this->numPoly = numPoly;
	
		numPolyExists = true;
	
 	}
	
	
	/**
	 * Mark numPoly, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearNumPoly () {
		numPolyExists = false;
	}
	

	
	/**
	 * The attribute numPolyFreq is optional. Return true if this attribute exists.
	 * @return true if and only if the numPolyFreq attribute exists. 
	 */
	bool SysCalRow::isNumPolyFreqExists() const {
		return numPolyFreqExists;
	}
	

	
 	/**
 	 * Get numPolyFreq, which is optional.
 	 * @return numPolyFreq as int
 	 * @throw IllegalAccessException If numPolyFreq does not exist.
 	 */
 	int SysCalRow::getNumPolyFreq() const throw(IllegalAccessException) {
		if (!numPolyFreqExists) {
			throw IllegalAccessException("numPolyFreq", "SysCal");
		}
	
  		return numPolyFreq;
 	}

 	/**
 	 * Set numPolyFreq with the specified int.
 	 * @param numPolyFreq The int value to which numPolyFreq is to be set.
 	 
 	
 	 */
 	void SysCalRow::setNumPolyFreq (int numPolyFreq) {
	
 		this->numPolyFreq = numPolyFreq;
	
		numPolyFreqExists = true;
	
 	}
	
	
	/**
	 * Mark numPolyFreq, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearNumPolyFreq () {
		numPolyFreqExists = false;
	}
	

	
	/**
	 * The attribute timeOrigin is optional. Return true if this attribute exists.
	 * @return true if and only if the timeOrigin attribute exists. 
	 */
	bool SysCalRow::isTimeOriginExists() const {
		return timeOriginExists;
	}
	

	
 	/**
 	 * Get timeOrigin, which is optional.
 	 * @return timeOrigin as ArrayTime
 	 * @throw IllegalAccessException If timeOrigin does not exist.
 	 */
 	ArrayTime SysCalRow::getTimeOrigin() const throw(IllegalAccessException) {
		if (!timeOriginExists) {
			throw IllegalAccessException("timeOrigin", "SysCal");
		}
	
  		return timeOrigin;
 	}

 	/**
 	 * Set timeOrigin with the specified ArrayTime.
 	 * @param timeOrigin The ArrayTime value to which timeOrigin is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTimeOrigin (ArrayTime timeOrigin) {
	
 		this->timeOrigin = timeOrigin;
	
		timeOriginExists = true;
	
 	}
	
	
	/**
	 * Mark timeOrigin, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTimeOrigin () {
		timeOriginExists = false;
	}
	

	
	/**
	 * The attribute freqOrigin is optional. Return true if this attribute exists.
	 * @return true if and only if the freqOrigin attribute exists. 
	 */
	bool SysCalRow::isFreqOriginExists() const {
		return freqOriginExists;
	}
	

	
 	/**
 	 * Get freqOrigin, which is optional.
 	 * @return freqOrigin as Frequency
 	 * @throw IllegalAccessException If freqOrigin does not exist.
 	 */
 	Frequency SysCalRow::getFreqOrigin() const throw(IllegalAccessException) {
		if (!freqOriginExists) {
			throw IllegalAccessException("freqOrigin", "SysCal");
		}
	
  		return freqOrigin;
 	}

 	/**
 	 * Set freqOrigin with the specified Frequency.
 	 * @param freqOrigin The Frequency value to which freqOrigin is to be set.
 	 
 	
 	 */
 	void SysCalRow::setFreqOrigin (Frequency freqOrigin) {
	
 		this->freqOrigin = freqOrigin;
	
		freqOriginExists = true;
	
 	}
	
	
	/**
	 * Mark freqOrigin, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearFreqOrigin () {
		freqOriginExists = false;
	}
	

	
	/**
	 * The attribute phaseCurve is optional. Return true if this attribute exists.
	 * @return true if and only if the phaseCurve attribute exists. 
	 */
	bool SysCalRow::isPhaseCurveExists() const {
		return phaseCurveExists;
	}
	

	
 	/**
 	 * Get phaseCurve, which is optional.
 	 * @return phaseCurve as vector<vector<Angle > >
 	 * @throw IllegalAccessException If phaseCurve does not exist.
 	 */
 	vector<vector<Angle > > SysCalRow::getPhaseCurve() const throw(IllegalAccessException) {
		if (!phaseCurveExists) {
			throw IllegalAccessException("phaseCurve", "SysCal");
		}
	
  		return phaseCurve;
 	}

 	/**
 	 * Set phaseCurve with the specified vector<vector<Angle > >.
 	 * @param phaseCurve The vector<vector<Angle > > value to which phaseCurve is to be set.
 	 
 	
 	 */
 	void SysCalRow::setPhaseCurve (vector<vector<Angle > > phaseCurve) {
	
 		this->phaseCurve = phaseCurve;
	
		phaseCurveExists = true;
	
 	}
	
	
	/**
	 * Mark phaseCurve, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearPhaseCurve () {
		phaseCurveExists = false;
	}
	

	
	/**
	 * The attribute delayCurve is optional. Return true if this attribute exists.
	 * @return true if and only if the delayCurve attribute exists. 
	 */
	bool SysCalRow::isDelayCurveExists() const {
		return delayCurveExists;
	}
	

	
 	/**
 	 * Get delayCurve, which is optional.
 	 * @return delayCurve as vector<vector<Interval > >
 	 * @throw IllegalAccessException If delayCurve does not exist.
 	 */
 	vector<vector<Interval > > SysCalRow::getDelayCurve() const throw(IllegalAccessException) {
		if (!delayCurveExists) {
			throw IllegalAccessException("delayCurve", "SysCal");
		}
	
  		return delayCurve;
 	}

 	/**
 	 * Set delayCurve with the specified vector<vector<Interval > >.
 	 * @param delayCurve The vector<vector<Interval > > value to which delayCurve is to be set.
 	 
 	
 	 */
 	void SysCalRow::setDelayCurve (vector<vector<Interval > > delayCurve) {
	
 		this->delayCurve = delayCurve;
	
		delayCurveExists = true;
	
 	}
	
	
	/**
	 * Mark delayCurve, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearDelayCurve () {
		delayCurveExists = false;
	}
	

	
	/**
	 * The attribute ampliCurve is optional. Return true if this attribute exists.
	 * @return true if and only if the ampliCurve attribute exists. 
	 */
	bool SysCalRow::isAmpliCurveExists() const {
		return ampliCurveExists;
	}
	

	
 	/**
 	 * Get ampliCurve, which is optional.
 	 * @return ampliCurve as vector<vector<float > >
 	 * @throw IllegalAccessException If ampliCurve does not exist.
 	 */
 	vector<vector<float > > SysCalRow::getAmpliCurve() const throw(IllegalAccessException) {
		if (!ampliCurveExists) {
			throw IllegalAccessException("ampliCurve", "SysCal");
		}
	
  		return ampliCurve;
 	}

 	/**
 	 * Set ampliCurve with the specified vector<vector<float > >.
 	 * @param ampliCurve The vector<vector<float > > value to which ampliCurve is to be set.
 	 
 	
 	 */
 	void SysCalRow::setAmpliCurve (vector<vector<float > > ampliCurve) {
	
 		this->ampliCurve = ampliCurve;
	
		ampliCurveExists = true;
	
 	}
	
	
	/**
	 * Mark ampliCurve, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearAmpliCurve () {
		ampliCurveExists = false;
	}
	

	
	/**
	 * The attribute bandpassCurve is optional. Return true if this attribute exists.
	 * @return true if and only if the bandpassCurve attribute exists. 
	 */
	bool SysCalRow::isBandpassCurveExists() const {
		return bandpassCurveExists;
	}
	

	
 	/**
 	 * Get bandpassCurve, which is optional.
 	 * @return bandpassCurve as vector<vector<float > >
 	 * @throw IllegalAccessException If bandpassCurve does not exist.
 	 */
 	vector<vector<float > > SysCalRow::getBandpassCurve() const throw(IllegalAccessException) {
		if (!bandpassCurveExists) {
			throw IllegalAccessException("bandpassCurve", "SysCal");
		}
	
  		return bandpassCurve;
 	}

 	/**
 	 * Set bandpassCurve with the specified vector<vector<float > >.
 	 * @param bandpassCurve The vector<vector<float > > value to which bandpassCurve is to be set.
 	 
 	
 	 */
 	void SysCalRow::setBandpassCurve (vector<vector<float > > bandpassCurve) {
	
 		this->bandpassCurve = bandpassCurve;
	
		bandpassCurveExists = true;
	
 	}
	
	
	/**
	 * Mark bandpassCurve, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearBandpassCurve () {
		bandpassCurveExists = false;
	}
	

	
	/**
	 * The attribute phasediffFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the phasediffFlag attribute exists. 
	 */
	bool SysCalRow::isPhasediffFlagExists() const {
		return phasediffFlagExists;
	}
	

	
 	/**
 	 * Get phasediffFlag, which is optional.
 	 * @return phasediffFlag as bool
 	 * @throw IllegalAccessException If phasediffFlag does not exist.
 	 */
 	bool SysCalRow::getPhasediffFlag() const throw(IllegalAccessException) {
		if (!phasediffFlagExists) {
			throw IllegalAccessException("phasediffFlag", "SysCal");
		}
	
  		return phasediffFlag;
 	}

 	/**
 	 * Set phasediffFlag with the specified bool.
 	 * @param phasediffFlag The bool value to which phasediffFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setPhasediffFlag (bool phasediffFlag) {
	
 		this->phasediffFlag = phasediffFlag;
	
		phasediffFlagExists = true;
	
 	}
	
	
	/**
	 * Mark phasediffFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearPhasediffFlag () {
		phasediffFlagExists = false;
	}
	

	
	/**
	 * The attribute sbgainFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the sbgainFlag attribute exists. 
	 */
	bool SysCalRow::isSbgainFlagExists() const {
		return sbgainFlagExists;
	}
	

	
 	/**
 	 * Get sbgainFlag, which is optional.
 	 * @return sbgainFlag as bool
 	 * @throw IllegalAccessException If sbgainFlag does not exist.
 	 */
 	bool SysCalRow::getSbgainFlag() const throw(IllegalAccessException) {
		if (!sbgainFlagExists) {
			throw IllegalAccessException("sbgainFlag", "SysCal");
		}
	
  		return sbgainFlag;
 	}

 	/**
 	 * Set sbgainFlag with the specified bool.
 	 * @param sbgainFlag The bool value to which sbgainFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setSbgainFlag (bool sbgainFlag) {
	
 		this->sbgainFlag = sbgainFlag;
	
		sbgainFlagExists = true;
	
 	}
	
	
	/**
	 * Mark sbgainFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearSbgainFlag () {
		sbgainFlagExists = false;
	}
	

	
	/**
	 * The attribute tauFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the tauFlag attribute exists. 
	 */
	bool SysCalRow::isTauFlagExists() const {
		return tauFlagExists;
	}
	

	
 	/**
 	 * Get tauFlag, which is optional.
 	 * @return tauFlag as bool
 	 * @throw IllegalAccessException If tauFlag does not exist.
 	 */
 	bool SysCalRow::getTauFlag() const throw(IllegalAccessException) {
		if (!tauFlagExists) {
			throw IllegalAccessException("tauFlag", "SysCal");
		}
	
  		return tauFlag;
 	}

 	/**
 	 * Set tauFlag with the specified bool.
 	 * @param tauFlag The bool value to which tauFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setTauFlag (bool tauFlag) {
	
 		this->tauFlag = tauFlag;
	
		tauFlagExists = true;
	
 	}
	
	
	/**
	 * Mark tauFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearTauFlag () {
		tauFlagExists = false;
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
 	bool SysCalRow::getTcalFlag() const throw(IllegalAccessException) {
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
 	bool SysCalRow::getTrxFlag() const throw(IllegalAccessException) {
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
 	bool SysCalRow::getTskyFlag() const throw(IllegalAccessException) {
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
 	bool SysCalRow::getTsysFlag() const throw(IllegalAccessException) {
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
 	bool SysCalRow::getTantFlag() const throw(IllegalAccessException) {
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
 	bool SysCalRow::getTantTsysFlag() const throw(IllegalAccessException) {
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
	 * The attribute pwvPathFlag is optional. Return true if this attribute exists.
	 * @return true if and only if the pwvPathFlag attribute exists. 
	 */
	bool SysCalRow::isPwvPathFlagExists() const {
		return pwvPathFlagExists;
	}
	

	
 	/**
 	 * Get pwvPathFlag, which is optional.
 	 * @return pwvPathFlag as bool
 	 * @throw IllegalAccessException If pwvPathFlag does not exist.
 	 */
 	bool SysCalRow::getPwvPathFlag() const throw(IllegalAccessException) {
		if (!pwvPathFlagExists) {
			throw IllegalAccessException("pwvPathFlag", "SysCal");
		}
	
  		return pwvPathFlag;
 	}

 	/**
 	 * Set pwvPathFlag with the specified bool.
 	 * @param pwvPathFlag The bool value to which pwvPathFlag is to be set.
 	 
 	
 	 */
 	void SysCalRow::setPwvPathFlag (bool pwvPathFlag) {
	
 		this->pwvPathFlag = pwvPathFlag;
	
		pwvPathFlagExists = true;
	
 	}
	
	
	/**
	 * Mark pwvPathFlag, which is an optional field, as non-existent.
	 */
	void SysCalRow::clearPwvPathFlag () {
		pwvPathFlagExists = false;
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
	 * Returns the pointer to the row in the SpectralWindow table having SpectralWindow.spectralWindowId == spectralWindowId
	 * @return a SpectralWindowRow*
	 * 
	 
	 */
	 SpectralWindowRow* SysCalRow::getSpectralWindowUsingSpectralWindowId() {
	 
	 	return table.getContainer().getSpectralWindow().getRowByKey(spectralWindowId);
	 }
	 

	

	
	
	
		

	// ===> Slice link from a row of SysCal table to a collection of row of Feed table.
	
	/**
	 * Get the collection of row in the Feed table having their attribut feedId == this->feedId
	 */
	vector <FeedRow *> SysCalRow::getFeeds() {
		
			return table.getContainer().getFeed().getRowByFeedId(feedId);
		
	}
	

	

	
	
	
		

	/**
	 * Returns the pointer to the row in the Antenna table having Antenna.antennaId == antennaId
	 * @return a AntennaRow*
	 * 
	 
	 */
	 AntennaRow* SysCalRow::getAntennaUsingAntennaId() {
	 
	 	return table.getContainer().getAntenna().getRowByKey(antennaId);
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
		
	
	

	
		numLoadExists = false;
	

	
		calLoadExists = false;
	

	
		feffExists = false;
	

	
		aeffExists = false;
	

	
		phaseDiffExists = false;
	

	
		sbgainExists = false;
	

	
		tauExists = false;
	

	
		tcalExists = false;
	

	
		trxExists = false;
	

	
		tskyExists = false;
	

	
		tsysExists = false;
	

	
		tantExists = false;
	

	
		tantTsysExists = false;
	

	
		pwvPathExists = false;
	

	
		dpwvPathExists = false;
	

	
		feffSpectrumExists = false;
	

	
		sbgainSpectrumExists = false;
	

	
		tauSpectrumExists = false;
	

	
		tcalSpectrumExists = false;
	

	
		trxSpectrumExists = false;
	

	
		tskySpectrumExists = false;
	

	
		tsysSpectrumExists = false;
	

	
		tantSpectrumExists = false;
	

	
		tantTsysSpectrumExists = false;
	

	
		pwvPathSpectrumExists = false;
	

	
		dpwvPathSpectrumExists = false;
	

	
		numPolyExists = false;
	

	
		numPolyFreqExists = false;
	

	
		timeOriginExists = false;
	

	
		freqOriginExists = false;
	

	
		phaseCurveExists = false;
	

	
		delayCurveExists = false;
	

	
		ampliCurveExists = false;
	

	
		bandpassCurveExists = false;
	

	
		phasediffFlagExists = false;
	

	
		sbgainFlagExists = false;
	

	
		tauFlagExists = false;
	

	
		tcalFlagExists = false;
	

	
		trxFlagExists = false;
	

	
		tskyFlagExists = false;
	

	
		tsysFlagExists = false;
	

	
		tantFlagExists = false;
	

	
		tantTsysFlagExists = false;
	

	
		pwvPathFlagExists = false;
	

	
	

	

	

	
	
	
	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	

	
	
	}
	
	SysCalRow::SysCalRow (SysCalTable &t, SysCalRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	
		numLoadExists = false;
	

	
		calLoadExists = false;
	

	
		feffExists = false;
	

	
		aeffExists = false;
	

	
		phaseDiffExists = false;
	

	
		sbgainExists = false;
	

	
		tauExists = false;
	

	
		tcalExists = false;
	

	
		trxExists = false;
	

	
		tskyExists = false;
	

	
		tsysExists = false;
	

	
		tantExists = false;
	

	
		tantTsysExists = false;
	

	
		pwvPathExists = false;
	

	
		dpwvPathExists = false;
	

	
		feffSpectrumExists = false;
	

	
		sbgainSpectrumExists = false;
	

	
		tauSpectrumExists = false;
	

	
		tcalSpectrumExists = false;
	

	
		trxSpectrumExists = false;
	

	
		tskySpectrumExists = false;
	

	
		tsysSpectrumExists = false;
	

	
		tantSpectrumExists = false;
	

	
		tantTsysSpectrumExists = false;
	

	
		pwvPathSpectrumExists = false;
	

	
		dpwvPathSpectrumExists = false;
	

	
		numPolyExists = false;
	

	
		numPolyFreqExists = false;
	

	
		timeOriginExists = false;
	

	
		freqOriginExists = false;
	

	
		phaseCurveExists = false;
	

	
		delayCurveExists = false;
	

	
		ampliCurveExists = false;
	

	
		bandpassCurveExists = false;
	

	
		phasediffFlagExists = false;
	

	
		sbgainFlagExists = false;
	

	
		tauFlagExists = false;
	

	
		tcalFlagExists = false;
	

	
		trxFlagExists = false;
	

	
		tskyFlagExists = false;
	

	
		tsysFlagExists = false;
	

	
		tantFlagExists = false;
	

	
		tantTsysFlagExists = false;
	

	
		pwvPathFlagExists = false;
	

	
	

	

	
		
		}
		else {
	
		
			antennaId = row.antennaId;
		
			feedId = row.feedId;
		
			spectralWindowId = row.spectralWindowId;
		
			timeInterval = row.timeInterval;
		
		
		
		
		
		
		
		if (row.numLoadExists) {
			numLoad = row.numLoad;		
			numLoadExists = true;
		}
		else
			numLoadExists = false;
		
		if (row.calLoadExists) {
			calLoad = row.calLoad;		
			calLoadExists = true;
		}
		else
			calLoadExists = false;
		
		if (row.feffExists) {
			feff = row.feff;		
			feffExists = true;
		}
		else
			feffExists = false;
		
		if (row.aeffExists) {
			aeff = row.aeff;		
			aeffExists = true;
		}
		else
			aeffExists = false;
		
		if (row.phaseDiffExists) {
			phaseDiff = row.phaseDiff;		
			phaseDiffExists = true;
		}
		else
			phaseDiffExists = false;
		
		if (row.sbgainExists) {
			sbgain = row.sbgain;		
			sbgainExists = true;
		}
		else
			sbgainExists = false;
		
		if (row.tauExists) {
			tau = row.tau;		
			tauExists = true;
		}
		else
			tauExists = false;
		
		if (row.tcalExists) {
			tcal = row.tcal;		
			tcalExists = true;
		}
		else
			tcalExists = false;
		
		if (row.trxExists) {
			trx = row.trx;		
			trxExists = true;
		}
		else
			trxExists = false;
		
		if (row.tskyExists) {
			tsky = row.tsky;		
			tskyExists = true;
		}
		else
			tskyExists = false;
		
		if (row.tsysExists) {
			tsys = row.tsys;		
			tsysExists = true;
		}
		else
			tsysExists = false;
		
		if (row.tantExists) {
			tant = row.tant;		
			tantExists = true;
		}
		else
			tantExists = false;
		
		if (row.tantTsysExists) {
			tantTsys = row.tantTsys;		
			tantTsysExists = true;
		}
		else
			tantTsysExists = false;
		
		if (row.pwvPathExists) {
			pwvPath = row.pwvPath;		
			pwvPathExists = true;
		}
		else
			pwvPathExists = false;
		
		if (row.dpwvPathExists) {
			dpwvPath = row.dpwvPath;		
			dpwvPathExists = true;
		}
		else
			dpwvPathExists = false;
		
		if (row.feffSpectrumExists) {
			feffSpectrum = row.feffSpectrum;		
			feffSpectrumExists = true;
		}
		else
			feffSpectrumExists = false;
		
		if (row.sbgainSpectrumExists) {
			sbgainSpectrum = row.sbgainSpectrum;		
			sbgainSpectrumExists = true;
		}
		else
			sbgainSpectrumExists = false;
		
		if (row.tauSpectrumExists) {
			tauSpectrum = row.tauSpectrum;		
			tauSpectrumExists = true;
		}
		else
			tauSpectrumExists = false;
		
		if (row.tcalSpectrumExists) {
			tcalSpectrum = row.tcalSpectrum;		
			tcalSpectrumExists = true;
		}
		else
			tcalSpectrumExists = false;
		
		if (row.trxSpectrumExists) {
			trxSpectrum = row.trxSpectrum;		
			trxSpectrumExists = true;
		}
		else
			trxSpectrumExists = false;
		
		if (row.tskySpectrumExists) {
			tskySpectrum = row.tskySpectrum;		
			tskySpectrumExists = true;
		}
		else
			tskySpectrumExists = false;
		
		if (row.tsysSpectrumExists) {
			tsysSpectrum = row.tsysSpectrum;		
			tsysSpectrumExists = true;
		}
		else
			tsysSpectrumExists = false;
		
		if (row.tantSpectrumExists) {
			tantSpectrum = row.tantSpectrum;		
			tantSpectrumExists = true;
		}
		else
			tantSpectrumExists = false;
		
		if (row.tantTsysSpectrumExists) {
			tantTsysSpectrum = row.tantTsysSpectrum;		
			tantTsysSpectrumExists = true;
		}
		else
			tantTsysSpectrumExists = false;
		
		if (row.pwvPathSpectrumExists) {
			pwvPathSpectrum = row.pwvPathSpectrum;		
			pwvPathSpectrumExists = true;
		}
		else
			pwvPathSpectrumExists = false;
		
		if (row.dpwvPathSpectrumExists) {
			dpwvPathSpectrum = row.dpwvPathSpectrum;		
			dpwvPathSpectrumExists = true;
		}
		else
			dpwvPathSpectrumExists = false;
		
		if (row.numPolyExists) {
			numPoly = row.numPoly;		
			numPolyExists = true;
		}
		else
			numPolyExists = false;
		
		if (row.numPolyFreqExists) {
			numPolyFreq = row.numPolyFreq;		
			numPolyFreqExists = true;
		}
		else
			numPolyFreqExists = false;
		
		if (row.timeOriginExists) {
			timeOrigin = row.timeOrigin;		
			timeOriginExists = true;
		}
		else
			timeOriginExists = false;
		
		if (row.freqOriginExists) {
			freqOrigin = row.freqOrigin;		
			freqOriginExists = true;
		}
		else
			freqOriginExists = false;
		
		if (row.phaseCurveExists) {
			phaseCurve = row.phaseCurve;		
			phaseCurveExists = true;
		}
		else
			phaseCurveExists = false;
		
		if (row.delayCurveExists) {
			delayCurve = row.delayCurve;		
			delayCurveExists = true;
		}
		else
			delayCurveExists = false;
		
		if (row.ampliCurveExists) {
			ampliCurve = row.ampliCurve;		
			ampliCurveExists = true;
		}
		else
			ampliCurveExists = false;
		
		if (row.bandpassCurveExists) {
			bandpassCurve = row.bandpassCurve;		
			bandpassCurveExists = true;
		}
		else
			bandpassCurveExists = false;
		
		if (row.phasediffFlagExists) {
			phasediffFlag = row.phasediffFlag;		
			phasediffFlagExists = true;
		}
		else
			phasediffFlagExists = false;
		
		if (row.sbgainFlagExists) {
			sbgainFlag = row.sbgainFlag;		
			sbgainFlagExists = true;
		}
		else
			sbgainFlagExists = false;
		
		if (row.tauFlagExists) {
			tauFlag = row.tauFlag;		
			tauFlagExists = true;
		}
		else
			tauFlagExists = false;
		
		if (row.tcalFlagExists) {
			tcalFlag = row.tcalFlag;		
			tcalFlagExists = true;
		}
		else
			tcalFlagExists = false;
		
		if (row.trxFlagExists) {
			trxFlag = row.trxFlag;		
			trxFlagExists = true;
		}
		else
			trxFlagExists = false;
		
		if (row.tskyFlagExists) {
			tskyFlag = row.tskyFlag;		
			tskyFlagExists = true;
		}
		else
			tskyFlagExists = false;
		
		if (row.tsysFlagExists) {
			tsysFlag = row.tsysFlag;		
			tsysFlagExists = true;
		}
		else
			tsysFlagExists = false;
		
		if (row.tantFlagExists) {
			tantFlag = row.tantFlag;		
			tantFlagExists = true;
		}
		else
			tantFlagExists = false;
		
		if (row.tantTsysFlagExists) {
			tantTsysFlag = row.tantTsysFlag;		
			tantTsysFlagExists = true;
		}
		else
			tantTsysFlagExists = false;
		
		if (row.pwvPathFlagExists) {
			pwvPathFlag = row.pwvPathFlag;		
			pwvPathFlagExists = true;
		}
		else
			pwvPathFlagExists = false;
		
		}	
	}

	
	bool SysCalRow::compareNoAutoInc(Tag antennaId, int feedId, Tag spectralWindowId, ArrayTimeInterval timeInterval) {
		bool result;
		result = true;
		
	
		
		result = result && (this->antennaId == antennaId);
		
		if (!result) return false;
	

	
		
		result = result && (this->feedId == feedId);
		
		if (!result) return false;
	

	
		
		result = result && (this->spectralWindowId == spectralWindowId);
		
		if (!result) return false;
	

	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

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
		
		return true;
	}	
	

} // End namespace asdm
 
