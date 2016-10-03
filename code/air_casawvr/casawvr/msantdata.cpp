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

  void getAntPos(const casacore::MeasurementSet &ms,
		 antpos_t &res)
  {
    const casacore::MSAntenna & antTable(ms.antenna());
    const size_t nant=antTable.nrow();
    res.resize(nant, 3);
    casacore::ROMSAntennaColumns acols(antTable);
    const casacore::ROArrayColumn<casacore::Double> &pos (acols.position());
    for(size_t i=0; i<nant; ++i)
    {
      casacore::Array<casacore::Double> a;
      pos.get(i, a,
	      casacore::True);
      for(size_t j=0; j<3; ++j)
	res(i,j)=a(casacore::IPosition(1,j));
    }
  }

  aname_t getAName(const casacore::MeasurementSet &ms)
  {
    aname_t res;
    const casacore::MSAntenna & antTable(ms.antenna());
    const size_t nant=antTable.nrow();

    casacore::ROMSAntennaColumns acols(antTable);
    const casacore::ROScalarColumn<casacore::String> &names (acols.name());
    for(size_t i=0; i<nant; ++i)
    {
      res.insert(aname_t::value_type(i, std::string(names(i))));
    }
    return res;
  }

}


