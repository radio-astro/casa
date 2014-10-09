/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version September 2010. 
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2
   
   \file mswvrdata.cpp

*/

#include "msantdata.hpp"
#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <casacore/ms/MeasurementSets/MSAntenna.h>
#include <casacore/ms/MeasurementSets/MSColumns.h>

namespace LibAIR2 {

  void getAntPos(const casa::MeasurementSet &ms,
		 antpos_t &res)
  {
    const casa::MSAntenna & antTable(ms.antenna());
    const size_t nant=antTable.nrow();
    res.resize(nant, 3);
    casa::ROMSAntennaColumns acols(antTable);
    const casa::ROArrayColumn<casa::Double> &pos (acols.position());
    for(size_t i=0; i<nant; ++i)
    {
      casa::Array<casa::Double> a;
      pos.get(i, a,
	      casa::True);
      for(size_t j=0; j<3; ++j)
	res(i,j)=a(casa::IPosition(1,j));
    }
  }

  aname_t getAName(const casa::MeasurementSet &ms)
  {
    aname_t res;
    const casa::MSAntenna & antTable(ms.antenna());
    const size_t nant=antTable.nrow();

    casa::ROMSAntennaColumns acols(antTable);
    const casa::ROScalarColumn<casa::String> &names (acols.name());
    for(size_t i=0; i<nant; ++i)
    {
      res.insert(aname_t::value_type(i, std::string(names(i))));
    }
    return res;
  }

}


