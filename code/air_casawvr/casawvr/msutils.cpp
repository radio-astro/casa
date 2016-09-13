/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file msutils.cpp

*/

#include <stdexcept>
#include "msutils.hpp"

#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/ms/MeasurementSets/MSFieldColumns.h>
#include <casacore/ms/MeasurementSets/MSField.h>
#include <casacore/ms/MeasurementSets/MSColumns.h>

namespace LibAIR2 {

  /** Channel frequencies for spectral window spw     
   */
  void spwChannelFreq(const casa::MeasurementSet &ms,
		      size_t spw,
		      std::vector<double> &fres)
  {
    casa::MSSpectralWindow specTable(ms.spectralWindow());

    // Number of channels 
    casa::ROScalarColumn<casa::Int> nc
      (specTable,
       casa::MSSpectralWindow::columnName(casa::MSSpectralWindow::NUM_CHAN));

    // Frequencies of the channels
    casa::ROArrayColumn<casa::Double> 
      chfreq(specTable,
	     casa::MSSpectralWindow::columnName(casa::MSSpectralWindow::CHAN_FREQ));

    fres.resize(nc(spw));
    casa::Array<casa::Double> freq;
    chfreq.get(spw, freq, casa::True);
    
    for(size_t i=0; i< static_cast<size_t>(nc(spw)); ++i)
    {
      fres[i]=freq(casa::IPosition(1,i));
    }
  }

  void fieldIDs(const casa::MeasurementSet &ms,
		std::vector<double> &time,
		std::vector<int> &fieldID,
		std::vector<int> &sourceID,
		const std::vector<size_t> &sortedI)
  {
    const casa::ROMSMainColumns cols(ms);
    const casa::ROScalarColumn<casa::Int> &f= cols.fieldId();
    const casa::ROScalarColumn<casa::Double> &t= cols.time();

    std::map<size_t, size_t> srcmap=getFieldSrcMap(ms);

    const size_t nrows=f.nrow();    

    if(sortedI.size()!= nrows){
      throw std::runtime_error("Time-sorted row vector must have same size as MS.");
    }

    time.resize(nrows);
    fieldID.resize(nrows);
    sourceID.resize(nrows);
    for(size_t ii=0; ii<nrows; ++ii)
    {
      size_t i = sortedI[ii];

      time[ii]=t(i);
      fieldID[ii]=f(i);
      if (srcmap.count(f(i)) == 0)
      {
	throw std::runtime_error("Encountered data without associated source");
      }
      /**checking if compiler version has map::at*/
#if __GNUC__ <= 4 and __GNUC_MINOR__ < 1
      sourceID[ii]=srcmap[(f(i))];
#else
      sourceID[ii]=srcmap.at(f(i));
#endif

    }
  }


  std::ostream & operator<<(std::ostream &o, 
			    const StateIntentMap &t)
  {
    o<<"     Map between State_ID and Scan Intents "<<std::endl
     <<"----------------------------------------------"<<std::endl;
    for(StateIntentMap::const_iterator i=t.begin(); 
	i!= t.end(); 
	++i)
    {
      o<<i->first<<": "<<i->second
       <<std::endl;
    }
    return o;
  }


  void scanIntents(const casa::MeasurementSet &ms,
		   StateIntentMap &mi)
  {
    mi.clear();
    casa::MSState state(ms.state());
    const casa::ROScalarColumn<casa::String> modes(state,
						   casa::MSState::columnName(casa::MSState::OBS_MODE));
    for(size_t i=0; i<state.nrow(); ++i)
    {
      casa::String t;
      modes.get(i,t);
      mi.insert(std::pair<size_t, std::string>(i, t));
    }
  }

  std::set<size_t> skyStateIDs(const casa::MeasurementSet &ms)
  {
    std::set<size_t> res;
    StateIntentMap mi;
    scanIntents(ms, mi);
    for(StateIntentMap::const_iterator i=mi.begin();
	i!=mi.end();
	++i)
    {
      if(i->second.find("ON_SOURCE") != i->second.npos &&
	 i->second.find("CALIBRATE_ATMOSPHERE") == i->second.npos)
	res.insert(i->first);
    }
    return res;
  }

  field_t getFieldNames(const casa::MeasurementSet &ms)
  {
    field_t res;
    const casa::MSField & fieldT(ms.field());
    const size_t nfields=fieldT.nrow();

    casa::ROMSFieldColumns fcols(fieldT);
    const casa::ROScalarColumn<casa::String> &names (fcols.name());
    for(size_t i=0; i<nfields; ++i)
    {
      res.insert(field_t::value_type(i, std::string(names(i))));
    }
    return res;
  }

  field_t getSourceNames(const casa::MeasurementSet &ms)
  {
    field_t res;
    const casa::MSSource & srcTab(ms.source());
    const size_t nsource=srcTab.nrow();

    casa::ROMSSourceColumns scols(srcTab);
    const casa::ROScalarColumn<casa::String> &names (scols.name());
    const casa::ROScalarColumn<casa::Int> & ids (scols.sourceId());
    for(size_t i=0; i<nsource; ++i)
    {
      res.insert(field_t::value_type(ids(i), std::string(names(i))));
    }
    return res;
    
  }

  std::set<size_t> getSrcFields(const casa::MeasurementSet &ms,
				const std::string &source)
  {
    std::set<size_t> res;
    const casa::MSField & fieldT(ms.field());
    const size_t nfields=fieldT.nrow();

    field_t sources=getSourceNames(ms);

    casa::ROMSFieldColumns fcols(fieldT);
    const casa::ROScalarColumn<casa::Int> &fieldsrc (fcols.sourceId());
    for(size_t i=0; i<nfields; ++i)
    {
      try {
	if(sources.at(fieldsrc(i))==source)
	  res.insert(i);
      }
      catch (const std::out_of_range &e)
      {
	std::cout<<"ERROR: Could not find source name for source ID " <<fieldsrc(i)<<std::endl
		 <<"       This could be related to CSV-910"<<std::endl
		 <<"       Continuing with processing but the statistics might be affected by this"<<std::endl
		 <<std::endl;
      }
    }
    return res;
  }

  std::map<size_t, size_t> getFieldSrcMap(const casa::MeasurementSet &ms)
  {
    std::map<size_t, size_t> res;
    const casa::MSField & fieldT(ms.field());
    const size_t nfields=fieldT.nrow();
    casa::ROMSFieldColumns fcols(fieldT);
    const casa::ROScalarColumn<casa::Int> &fieldsrc (fcols.sourceId());
    for(size_t i=0; i<nfields; ++i)
    {
      res.insert(std::pair<size_t, size_t>(i, fieldsrc(i)));
    }
    return res;
  }
}


