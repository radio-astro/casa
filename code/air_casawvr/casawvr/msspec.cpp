/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version June 2010. 
   
   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file msspec.cpp
   
*/

#include "msspec.hpp"

#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/tables/Tables/Table.h>
#include <casacore/ms/MeasurementSets/MSColumns.h>

namespace LibAIR {

  void loadSpec(const casa::MeasurementSet &ms,
		MSSpec &s)
  {
    casa::MSSpectralWindow specTable(ms.spectralWindow());
    // Number of spectral windows in this measurement set
    const size_t nspw=specTable.nrow();

    // Number of channels 
    casa::ROScalarColumn<casa::Int> nc
      (specTable,
       casa::MSSpectralWindow::columnName(casa::MSSpectralWindow::NUM_CHAN));

    // Frequencies of the channels
    casa::ROArrayColumn<casa::Double> 
      chfreq(specTable,
	     casa::MSSpectralWindow::columnName(casa::MSSpectralWindow::CHAN_FREQ));

    s.spws.resize(nspw);
    for (size_t spw=0; spw<nspw; ++spw)
    {
      casa::Array<casa::Double> freq;
      chfreq.get(spw, freq, casa::True);
      s.spws[spw].chf.resize(nc(spw));
      for(size_t i=0; i< static_cast<size_t>(nc(spw)); ++i)
      {
	s.spws[spw].chf[i]=freq(casa::IPosition(1,i));
      }
    }
  }

  std::ostream & 
  operator<<(std::ostream &os,
	     const MSSpec &s)
  {
    os<<"This MS has "<<s.spws.size()<<" spectral windows"
      <<std::endl;
    for (size_t spw=0; spw<s.spws.size(); ++spw)
    {
      os<<"SPW "<<spw<<" has "<<s.spws[spw].chf.size()<<" channels ";
      if (s.spws[spw].chf.size()==1)
      {
	os<<"at frequency "<<s.spws[spw].chf[0];
      }
      else
      {
	os<<"starting at frequency "<<s.spws[spw].chf[0]
	  <<" and with last at frequency "<<s.spws[spw].chf[s.spws[spw].chf.size()-1];
      }
      os<<std::endl;
    }
    return os;
  }

  std::map<size_t, size_t>
  SPWDataDescMap(const casa::MeasurementSet &ms)
  {
    std::map<size_t, size_t> res;
    const casa::MSDataDescription dd(ms.dataDescription());
    const size_t n=dd.nrow();

    casa::ROScalarColumn<casa::Int> 
      spwid(dd,
	    casa::MSDataDescription::columnName(casa::MSDataDescriptionEnums::SPECTRAL_WINDOW_ID));


    for(size_t i=0; i<n; ++i)
    {
      int spw;
      spwid.get(i, spw);
      res.insert(std::pair<size_t, size_t>(spw, i));
    }
    return res;
  }

  std::map<size_t, size_t>
  DataDescSPWMap(const casa::MeasurementSet &ms)
  {
    std::map<size_t, size_t> res;
    const casa::MSDataDescription dd(ms.dataDescription());
    const size_t n=dd.nrow();

    casa::ROScalarColumn<casa::Int> 
      spwid(dd,
	    casa::MSDataDescription::columnName(casa::MSDataDescriptionEnums::SPECTRAL_WINDOW_ID));


    for(size_t i=0; i<n; ++i)
    {
      int spw;
      spwid.get(i, spw);
      res.insert(std::pair<size_t, size_t>(i, spw));
    }
    return res;
  }

  void dataSPWs(const casa::MeasurementSet &ms,
		std::vector<size_t> &spw,
		const std::vector<size_t> &sortedI)
  {
    std::map<size_t, size_t> map=DataDescSPWMap(ms);
    const casa::ROMSMainColumns cols(ms);
    const casa::ROScalarColumn<casa::Int> &dd=cols.dataDescId();
    const size_t nrows=dd.nrow();          
    spw.resize(nrows);
    for(size_t ii=0; ii<nrows; ++ii)
    {
      size_t i = sortedI[ii];

      spw[i]=map[dd(i)];
    }
  }


}



